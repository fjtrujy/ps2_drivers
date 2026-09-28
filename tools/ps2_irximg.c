#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IRXIMG_MAGIC "PS2IRX01"
#define IRXIMG_MAGIC_SIZE 8u
#define IRXIMG_VERSION 1u
#define IRXIMG_HEADER_SIZE 44u
#define IRXIMG_ENTRY_SIZE 20u
#define IRXIMG_ALIGNMENT 16u
#define IRXIMG_MAX_ENTRIES 128u
#define IRXIMG_MAX_FIELD 128u
#define IRXIMG_PATH_MAX 1024u

struct manifest_entry {
    uint32_t id;
    char filename[IRXIMG_MAX_FIELD];
    char symbol[IRXIMG_MAX_FIELD];
    char name[IRXIMG_MAX_FIELD];
};

struct manifest {
    struct manifest_entry entries[IRXIMG_MAX_ENTRIES];
    size_t count;
    uint32_t crc32;
};

struct payload {
    unsigned char *data;
    uint32_t size;
    uint32_t crc32;
    uint32_t offset;
};

static uint32_t crc32_update(uint32_t crc, const unsigned char *data, size_t size)
{
    size_t i;

    crc = ~crc;
    for (i = 0; i < size; ++i) {
        unsigned int bit;

        crc ^= data[i];
        for (bit = 0; bit < 8; ++bit) {
            uint32_t mask = (uint32_t)(-(int32_t)(crc & 1u));
            crc = (crc >> 1) ^ (0xedb88320u & mask);
        }
    }

    return ~crc;
}

static uint32_t crc32_bytes(const unsigned char *data, size_t size)
{
    return crc32_update(0u, data, size);
}

static uint32_t align_up_u32(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1u) & ~(alignment - 1u);
}

static void write_u32_le(unsigned char *dst, uint32_t value)
{
    dst[0] = (unsigned char)(value & 0xffu);
    dst[1] = (unsigned char)((value >> 8) & 0xffu);
    dst[2] = (unsigned char)((value >> 16) & 0xffu);
    dst[3] = (unsigned char)((value >> 24) & 0xffu);
}

static uint32_t read_u32_le(const unsigned char *src)
{
    return ((uint32_t)src[0]) |
           ((uint32_t)src[1] << 8) |
           ((uint32_t)src[2] << 16) |
           ((uint32_t)src[3] << 24);
}

static int parse_u32(const char *text, uint32_t *value)
{
    char *end = NULL;
    unsigned long parsed;

    errno = 0;
    parsed = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed > UINT32_MAX)
        return -1;

    *value = (uint32_t)parsed;
    return 0;
}

static char *trim_line(char *line)
{
    size_t len;
    char *start = line;

    while (*start == ' ' || *start == '\t')
        ++start;

    len = strlen(start);
    while (len > 0 &&
           (start[len - 1] == '\n' || start[len - 1] == '\r' ||
            start[len - 1] == ' ' || start[len - 1] == '\t')) {
        start[--len] = '\0';
    }

    return start;
}

static int copy_field(char *dst, size_t dst_size, const char *src, const char *field_name, size_t line_number)
{
    size_t len = strlen(src);

    if (len == 0 || len >= dst_size) {
        fprintf(stderr, "manifest:%zu: invalid %s field\n", line_number, field_name);
        return -1;
    }

    memcpy(dst, src, len + 1);
    return 0;
}

static uint32_t manifest_logical_crc(const struct manifest *manifest)
{
    uint32_t crc = 0u;
    size_t i;

    for (i = 0; i < manifest->count; ++i) {
        const struct manifest_entry *entry = &manifest->entries[i];
        unsigned char id_bytes[4];

        write_u32_le(id_bytes, entry->id);
        crc = crc32_update(crc, id_bytes, sizeof(id_bytes));
        crc = crc32_update(crc, (const unsigned char *)entry->filename, strlen(entry->filename) + 1u);
        crc = crc32_update(crc, (const unsigned char *)entry->symbol, strlen(entry->symbol) + 1u);
        crc = crc32_update(crc, (const unsigned char *)entry->name, strlen(entry->name) + 1u);
    }

    return crc;
}

static int load_manifest(const char *path, struct manifest *manifest)
{
    FILE *file;
    char line[512];
    size_t line_number = 0;

    memset(manifest, 0, sizeof(*manifest));

    file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "failed to open manifest '%s': %s\n", path, strerror(errno));
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char *fields[4];
        char *cursor;
        char *token;
        size_t field_count = 0;
        size_t i;
        struct manifest_entry *entry;

        ++line_number;
        cursor = trim_line(line);
        if (*cursor == '\0' || *cursor == '#')
            continue;

        token = strtok(cursor, "|");
        while (token != NULL && field_count < 4u) {
            fields[field_count++] = token;
            token = strtok(NULL, "|");
        }

        if (field_count != 4u || token != NULL) {
            fprintf(stderr, "manifest:%zu: expected 4 pipe-delimited fields\n", line_number);
            fclose(file);
            return -1;
        }

        if (manifest->count >= IRXIMG_MAX_ENTRIES) {
            fprintf(stderr, "manifest has more than %u entries\n", IRXIMG_MAX_ENTRIES);
            fclose(file);
            return -1;
        }

        entry = &manifest->entries[manifest->count];
        if (parse_u32(fields[0], &entry->id) != 0 || entry->id == 0u) {
            fprintf(stderr, "manifest:%zu: invalid non-zero module id '%s'\n", line_number, fields[0]);
            fclose(file);
            return -1;
        }

        for (i = 0; i < manifest->count; ++i) {
            if (manifest->entries[i].id == entry->id) {
                fprintf(stderr, "manifest:%zu: duplicate module id %u\n", line_number, entry->id);
                fclose(file);
                return -1;
            }
        }

        if (copy_field(entry->filename, sizeof(entry->filename), fields[1], "filename", line_number) != 0 ||
            copy_field(entry->symbol, sizeof(entry->symbol), fields[2], "symbol", line_number) != 0 ||
            copy_field(entry->name, sizeof(entry->name), fields[3], "name", line_number) != 0) {
            fclose(file);
            return -1;
        }

        ++manifest->count;
    }

    if (ferror(file)) {
        fprintf(stderr, "failed while reading manifest '%s'\n", path);
        fclose(file);
        return -1;
    }

    fclose(file);

    if (manifest->count == 0u) {
        fprintf(stderr, "manifest '%s' contains no IRX entries\n", path);
        return -1;
    }

    manifest->crc32 = manifest_logical_crc(manifest);
    return 0;
}

static int read_file(const char *path, unsigned char **data_out, uint32_t *size_out)
{
    FILE *file;
    long size_long;
    unsigned char *data;

    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "failed to open '%s': %s\n", path, strerror(errno));
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "failed to seek '%s'\n", path);
        fclose(file);
        return -1;
    }

    size_long = ftell(file);
    if (size_long <= 0 || (unsigned long)size_long > UINT32_MAX) {
        fprintf(stderr, "invalid file size for '%s'\n", path);
        fclose(file);
        return -1;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "failed to rewind '%s'\n", path);
        fclose(file);
        return -1;
    }

    data = (unsigned char *)malloc((size_t)size_long);
    if (data == NULL) {
        fprintf(stderr, "out of memory while reading '%s'\n", path);
        fclose(file);
        return -1;
    }

    if (fread(data, 1, (size_t)size_long, file) != (size_t)size_long) {
        fprintf(stderr, "short read from '%s'\n", path);
        free(data);
        fclose(file);
        return -1;
    }

    fclose(file);
    *data_out = data;
    *size_out = (uint32_t)size_long;
    return 0;
}

static int write_zeros(FILE *file, uint32_t count)
{
    static const unsigned char zeros[64] = {0};

    while (count > 0u) {
        uint32_t chunk = count > sizeof(zeros) ? (uint32_t)sizeof(zeros) : count;

        if (fwrite(zeros, 1, chunk, file) != chunk)
            return -1;
        count -= chunk;
    }

    return 0;
}

static int build_payload_path(char *path, size_t path_size, const char *directory, const char *filename)
{
    int written;
    size_t dir_len = strlen(directory);
    const char *separator = (dir_len > 0u && directory[dir_len - 1u] == '/') ? "" : "/";

    written = snprintf(path, path_size, "%s%s%s", directory, separator, filename);
    if (written < 0 || (size_t)written >= path_size) {
        fprintf(stderr, "IRX path is too long: %s/%s\n", directory, filename);
        return -1;
    }

    return 0;
}

static void free_payloads(struct payload *payloads, size_t count)
{
    size_t i;

    for (i = 0; i < count; ++i)
        free(payloads[i].data);
}

static int pack_image(const char *manifest_path, const char *irx_directory, const char *output_path)
{
    struct manifest manifest;
    struct payload payloads[IRXIMG_MAX_ENTRIES];
    uint32_t entry_table_offset = IRXIMG_HEADER_SIZE;
    uint32_t payload_offset;
    uint32_t cursor;
    uint32_t file_size;
    size_t i;
    FILE *output;
    unsigned char header[IRXIMG_HEADER_SIZE];

    memset(payloads, 0, sizeof(payloads));

    if (load_manifest(manifest_path, &manifest) != 0)
        return 1;

    if (manifest.count > (UINT32_MAX - IRXIMG_HEADER_SIZE) / IRXIMG_ENTRY_SIZE) {
        fprintf(stderr, "manifest entry table is too large\n");
        return 1;
    }

    payload_offset = align_up_u32(
        IRXIMG_HEADER_SIZE + (uint32_t)manifest.count * IRXIMG_ENTRY_SIZE,
        IRXIMG_ALIGNMENT);
    cursor = payload_offset;

    for (i = 0; i < manifest.count; ++i) {
        char path[IRXIMG_PATH_MAX];

        if (build_payload_path(path, sizeof(path), irx_directory, manifest.entries[i].filename) != 0) {
            free_payloads(payloads, manifest.count);
            return 1;
        }

        if (read_file(path, &payloads[i].data, &payloads[i].size) != 0) {
            free_payloads(payloads, manifest.count);
            return 1;
        }

        payloads[i].crc32 = crc32_bytes(payloads[i].data, payloads[i].size);
        payloads[i].offset = cursor;

        if (payloads[i].size > UINT32_MAX - cursor) {
            fprintf(stderr, "image is too large\n");
            free_payloads(payloads, manifest.count);
            return 1;
        }

        cursor = align_up_u32(cursor + payloads[i].size, IRXIMG_ALIGNMENT);
    }

    file_size = cursor;
    output = fopen(output_path, "wb");
    if (output == NULL) {
        fprintf(stderr, "failed to create '%s': %s\n", output_path, strerror(errno));
        free_payloads(payloads, manifest.count);
        return 1;
    }

    memset(header, 0, sizeof(header));
    memcpy(header, IRXIMG_MAGIC, IRXIMG_MAGIC_SIZE);
    write_u32_le(header + 8u, IRXIMG_VERSION);
    write_u32_le(header + 12u, IRXIMG_HEADER_SIZE);
    write_u32_le(header + 16u, (uint32_t)manifest.count);
    write_u32_le(header + 20u, IRXIMG_ENTRY_SIZE);
    write_u32_le(header + 24u, entry_table_offset);
    write_u32_le(header + 28u, payload_offset);
    write_u32_le(header + 32u, file_size);
    write_u32_le(header + 36u, 0u);
    write_u32_le(header + 40u, manifest.crc32);

    if (fwrite(header, 1, sizeof(header), output) != sizeof(header)) {
        fprintf(stderr, "failed to write image header\n");
        fclose(output);
        free_payloads(payloads, manifest.count);
        return 1;
    }

    for (i = 0; i < manifest.count; ++i) {
        unsigned char entry[IRXIMG_ENTRY_SIZE];

        memset(entry, 0, sizeof(entry));
        write_u32_le(entry + 0u, manifest.entries[i].id);
        write_u32_le(entry + 4u, payloads[i].offset);
        write_u32_le(entry + 8u, payloads[i].size);
        write_u32_le(entry + 12u, payloads[i].crc32);
        write_u32_le(entry + 16u, 0u);

        if (fwrite(entry, 1, sizeof(entry), output) != sizeof(entry)) {
            fprintf(stderr, "failed to write image entry %zu\n", i);
            fclose(output);
            free_payloads(payloads, manifest.count);
            return 1;
        }
    }

    if (payload_offset < IRXIMG_HEADER_SIZE + (uint32_t)manifest.count * IRXIMG_ENTRY_SIZE ||
        write_zeros(
            output,
            payload_offset - (IRXIMG_HEADER_SIZE + (uint32_t)manifest.count * IRXIMG_ENTRY_SIZE)) != 0) {
        fprintf(stderr, "failed to write image table padding\n");
        fclose(output);
        free_payloads(payloads, manifest.count);
        return 1;
    }

    for (i = 0; i < manifest.count; ++i) {
        uint32_t end = payloads[i].offset + payloads[i].size;
        uint32_t aligned_end = align_up_u32(end, IRXIMG_ALIGNMENT);

        if (fwrite(payloads[i].data, 1, payloads[i].size, output) != payloads[i].size ||
            write_zeros(output, aligned_end - end) != 0) {
            fprintf(stderr, "failed to write payload '%s'\n", manifest.entries[i].filename);
            fclose(output);
            free_payloads(payloads, manifest.count);
            return 1;
        }
    }

    if (fclose(output) != 0) {
        fprintf(stderr, "failed to finalize '%s'\n", output_path);
        free_payloads(payloads, manifest.count);
        return 1;
    }

    free_payloads(payloads, manifest.count);
    printf("packed %zu IRX modules into %s (%u bytes)\n", manifest.count, output_path, file_size);
    return 0;
}

static int validate_image_internal(
    const char *manifest_path,
    const char *image_path,
    int inspect)
{
    struct manifest manifest;
    unsigned char *image = NULL;
    uint32_t image_size = 0u;
    uint32_t version;
    uint32_t header_size;
    uint32_t entry_count;
    uint32_t entry_size;
    uint32_t entry_table_offset;
    uint32_t payload_offset;
    uint32_t declared_file_size;
    uint32_t flags;
    uint32_t manifest_crc;
    uint32_t previous_end = 0u;
    size_t i;

    if (load_manifest(manifest_path, &manifest) != 0)
        return 1;

    if (read_file(image_path, &image, &image_size) != 0)
        return 1;

    if (image_size < IRXIMG_HEADER_SIZE) {
        fprintf(stderr, "image is smaller than the v1 header\n");
        free(image);
        return 1;
    }

    if (memcmp(image, IRXIMG_MAGIC, IRXIMG_MAGIC_SIZE) != 0) {
        fprintf(stderr, "invalid image magic\n");
        free(image);
        return 1;
    }

    version = read_u32_le(image + 8u);
    header_size = read_u32_le(image + 12u);
    entry_count = read_u32_le(image + 16u);
    entry_size = read_u32_le(image + 20u);
    entry_table_offset = read_u32_le(image + 24u);
    payload_offset = read_u32_le(image + 28u);
    declared_file_size = read_u32_le(image + 32u);
    flags = read_u32_le(image + 36u);
    manifest_crc = read_u32_le(image + 40u);

    if (version != IRXIMG_VERSION ||
        header_size != IRXIMG_HEADER_SIZE ||
        entry_size != IRXIMG_ENTRY_SIZE ||
        entry_table_offset != IRXIMG_HEADER_SIZE ||
        flags != 0u) {
        fprintf(stderr, "unsupported or malformed image header\n");
        free(image);
        return 1;
    }

    if (entry_count != manifest.count) {
        fprintf(stderr, "image entry count %u does not match manifest count %zu\n", entry_count, manifest.count);
        free(image);
        return 1;
    }

    if (declared_file_size != image_size) {
        fprintf(stderr, "image declares %u bytes but file contains %u bytes\n", declared_file_size, image_size);
        free(image);
        return 1;
    }

    if (manifest_crc != manifest.crc32) {
        fprintf(stderr, "image manifest CRC does not match the canonical manifest\n");
        free(image);
        return 1;
    }

    if (entry_count > (UINT32_MAX - entry_table_offset) / entry_size ||
        entry_table_offset + entry_count * entry_size > image_size ||
        payload_offset < entry_table_offset + entry_count * entry_size ||
        (payload_offset & (IRXIMG_ALIGNMENT - 1u)) != 0u) {
        fprintf(stderr, "invalid entry table or payload offset\n");
        free(image);
        return 1;
    }

    if (inspect) {
        printf("magic=%s version=%u entries=%u file_size=%u manifest_crc32=%08x\n",
               IRXIMG_MAGIC, version, entry_count, image_size, manifest_crc);
        printf("%-4s %-18s %-10s %-10s %-10s\n", "id", "name", "offset", "size", "crc32");
    }

    previous_end = payload_offset;
    for (i = 0; i < manifest.count; ++i) {
        const unsigned char *entry = image + entry_table_offset + (uint32_t)i * entry_size;
        uint32_t id = read_u32_le(entry + 0u);
        uint32_t offset = read_u32_le(entry + 4u);
        uint32_t size = read_u32_le(entry + 8u);
        uint32_t crc = read_u32_le(entry + 12u);
        uint32_t entry_flags = read_u32_le(entry + 16u);
        uint32_t actual_crc;

        if (id != manifest.entries[i].id) {
            fprintf(stderr, "entry %zu has id %u, expected %u\n", i, id, manifest.entries[i].id);
            free(image);
            return 1;
        }

        if (size == 0u || entry_flags != 0u ||
            offset < payload_offset ||
            (offset & (IRXIMG_ALIGNMENT - 1u)) != 0u ||
            offset < previous_end ||
            size > image_size - offset) {
            fprintf(stderr, "entry %zu has invalid bounds or flags\n", i);
            free(image);
            return 1;
        }

        actual_crc = crc32_bytes(image + offset, size);
        if (actual_crc != crc) {
            fprintf(stderr, "entry '%s' CRC mismatch\n", manifest.entries[i].name);
            free(image);
            return 1;
        }

        previous_end = offset + size;

        if (inspect) {
            printf("%-4u %-18s %-10u %-10u %08x\n",
                   id, manifest.entries[i].name, offset, size, crc);
        }
    }

    free(image);
    return 0;
}

static void print_usage(const char *program)
{
    fprintf(stderr,
            "usage:\n"
            "  %s pack <manifest> <irx-directory> <output.irximg>\n"
            "  %s validate <manifest> <image.irximg>\n"
            "  %s inspect <manifest> <image.irximg>\n",
            program, program, program);
}

int main(int argc, char **argv)
{
    if (argc == 5 && strcmp(argv[1], "pack") == 0)
        return pack_image(argv[2], argv[3], argv[4]);

    if (argc == 4 && strcmp(argv[1], "validate") == 0) {
        int result = validate_image_internal(argv[2], argv[3], 0);
        if (result == 0)
            printf("validated %s\n", argv[3]);
        return result;
    }

    if (argc == 4 && strcmp(argv[1], "inspect") == 0)
        return validate_image_internal(argv[2], argv[3], 1);

    print_usage(argv[0]);
    return 2;
}
