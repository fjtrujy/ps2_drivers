#include <limits.h>
#ifdef __PS2__
#include <malloc.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ps2_drivers_img.h>
#include <ps2_irx_image_format.h>

#include "ps2_drivers_img_internal.h"
#include "ps2_irx_ids.h"

#define PS2_DRIVER_REQ_KNOWN_MASK ((1u << 20) - 1u)

struct ps2_irx_image_entry {
    uint32_t id;
    uint32_t offset;
    uint32_t size;
    uint32_t crc32;
    uint32_t flags;
};

struct ps2_irx_staged_module {
    uint32_t id;
    void *data;
    uint32_t size;
};

static struct ps2_irx_staged_module *g_staged_modules;
static size_t g_staged_capacity;
static size_t g_staged_count;
static size_t g_staged_bytes;
static char *g_image_path;

static uint32_t read_u32_le(const unsigned char *src)
{
    return ((uint32_t)src[0]) |
           ((uint32_t)src[1] << 8) |
           ((uint32_t)src[2] << 16) |
           ((uint32_t)src[3] << 24);
}

static uint32_t crc32_bytes(const unsigned char *data, size_t size)
{
    uint32_t crc = ~0u;
    size_t i;

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

static void *alloc_aligned(size_t alignment, size_t size)
{
#ifdef __PS2__
    return memalign(alignment, size);
#else
    void *data = NULL;

    if (posix_memalign(&data, alignment, size) != 0)
        return NULL;

    return data;
#endif
}

static void mark_module(unsigned char requested[IRXIMG_MAX_ENTRIES + 1u], uint32_t id)
{
    if (id <= IRXIMG_MAX_ENTRIES)
        requested[id] = 1u;
}

static void expand_requirements(
    uint32_t requirements,
    unsigned char requested[IRXIMG_MAX_ENTRIES + 1u])
{
    if (requirements & PS2_DRIVER_REQ_SIO2MAN)
        mark_module(requested, PS2_IRX_ID_SIO2MAN);

    if (requirements & PS2_DRIVER_REQ_FILEXIO) {
        mark_module(requested, PS2_IRX_ID_IOMANX);
        mark_module(requested, PS2_IRX_ID_FILEXIO);
    }

    if (requirements & PS2_DRIVER_REQ_MEMCARD) {
        mark_module(requested, PS2_IRX_ID_SIO2MAN);
        mark_module(requested, PS2_IRX_ID_MCMAN);
        mark_module(requested, PS2_IRX_ID_MCSERV);
    }

    if (requirements & PS2_DRIVER_REQ_BDM) {
        mark_module(requested, PS2_IRX_ID_BDM);
        mark_module(requested, PS2_IRX_ID_BDMFS_FATFS);
    }

    if (requirements & PS2_DRIVER_REQ_USBD)
        mark_module(requested, PS2_IRX_ID_USBD);

    if (requirements & PS2_DRIVER_REQ_USB) {
        mark_module(requested, PS2_IRX_ID_BDM);
        mark_module(requested, PS2_IRX_ID_BDMFS_FATFS);
        mark_module(requested, PS2_IRX_ID_USBD);
        mark_module(requested, PS2_IRX_ID_USBMASS_BD);
    }

    if (requirements & PS2_DRIVER_REQ_MX4SIO) {
        mark_module(requested, PS2_IRX_ID_SIO2MAN);
        mark_module(requested, PS2_IRX_ID_BDM);
        mark_module(requested, PS2_IRX_ID_BDMFS_FATFS);
        mark_module(requested, PS2_IRX_ID_MX4SIO_BD);
    }

    if (requirements & PS2_DRIVER_REQ_CDFS)
        mark_module(requested, PS2_IRX_ID_CDFS);

    if (requirements & PS2_DRIVER_REQ_DEV9)
        mark_module(requested, PS2_IRX_ID_PS2DEV9);

    if (requirements & PS2_DRIVER_REQ_HDD) {
        mark_module(requested, PS2_IRX_ID_IOMANX);
        mark_module(requested, PS2_IRX_ID_FILEXIO);
        mark_module(requested, PS2_IRX_ID_BDM);
        mark_module(requested, PS2_IRX_ID_BDMFS_FATFS);
        mark_module(requested, PS2_IRX_ID_PS2DEV9);
        mark_module(requested, PS2_IRX_ID_PS2ATAD);
        mark_module(requested, PS2_IRX_ID_PS2HDD);
        mark_module(requested, PS2_IRX_ID_PS2FS);
    }

    if (requirements & PS2_DRIVER_REQ_JOYSTICK) {
        mark_module(requested, PS2_IRX_ID_SIO2MAN);
        mark_module(requested, PS2_IRX_ID_MTAPMAN);
        mark_module(requested, PS2_IRX_ID_PADMAN);
    }

    if (requirements & PS2_DRIVER_REQ_AUDIO) {
        mark_module(requested, PS2_IRX_ID_LIBSD);
        mark_module(requested, PS2_IRX_ID_AUDSRV);
    }

    if (requirements & PS2_DRIVER_REQ_POWEROFF)
        mark_module(requested, PS2_IRX_ID_POWEROFF);

    if (requirements & PS2_DRIVER_REQ_MOUSE) {
        mark_module(requested, PS2_IRX_ID_USBD);
        mark_module(requested, PS2_IRX_ID_PS2MOUSE);
    }

    if (requirements & PS2_DRIVER_REQ_KEYBOARD) {
        mark_module(requested, PS2_IRX_ID_USBD);
        mark_module(requested, PS2_IRX_ID_PS2KBD);
    }

    if (requirements & PS2_DRIVER_REQ_CAMERA) {
        mark_module(requested, PS2_IRX_ID_USBD);
        mark_module(requested, PS2_IRX_ID_PS2CAM);
    }

    if (requirements & PS2_DRIVER_REQ_NETMAN)
        mark_module(requested, PS2_IRX_ID_NETMAN);

    if (requirements & PS2_DRIVER_REQ_SMAP)
        mark_module(requested, PS2_IRX_ID_SMAP);

    if (requirements & PS2_DRIVER_REQ_EEIP) {
        mark_module(requested, PS2_IRX_ID_PS2DEV9);
        mark_module(requested, PS2_IRX_ID_NETMAN);
        mark_module(requested, PS2_IRX_ID_SMAP);
    }

    if (requirements & PS2_DRIVER_REQ_IOPIP) {
        mark_module(requested, PS2_IRX_ID_PS2DEV9);
        mark_module(requested, PS2_IRX_ID_NETMAN);
        mark_module(requested, PS2_IRX_ID_SMAP);
        mark_module(requested, PS2_IRX_ID_PS2IP_NM);
        mark_module(requested, PS2_IRX_ID_PS2IPS);
    }
}

static int parse_image_entries(
    FILE *file,
    uint32_t file_size,
    struct ps2_irx_image_entry **entries_out,
    uint32_t *entry_count_out)
{
    unsigned char header[IRXIMG_HEADER_SIZE];
    unsigned char *table = NULL;
    struct ps2_irx_image_entry *entries = NULL;
    uint32_t version;
    uint32_t header_size;
    uint32_t entry_count;
    uint32_t entry_size;
    uint32_t entry_table_offset;
    uint32_t payload_offset;
    uint32_t declared_file_size;
    uint32_t flags;
    uint32_t i;

    if (fseek(file, 0, SEEK_SET) != 0 ||
        fread(header, 1, sizeof(header), file) != sizeof(header))
        return PS2_DRIVERS_IMG_ERR_IO;

    if (memcmp(header, IRXIMG_MAGIC, IRXIMG_MAGIC_SIZE) != 0)
        return PS2_DRIVERS_IMG_ERR_FORMAT;

    version = read_u32_le(header + 8u);
    header_size = read_u32_le(header + 12u);
    entry_count = read_u32_le(header + 16u);
    entry_size = read_u32_le(header + 20u);
    entry_table_offset = read_u32_le(header + 24u);
    payload_offset = read_u32_le(header + 28u);
    declared_file_size = read_u32_le(header + 32u);
    flags = read_u32_le(header + 36u);

    if (version != IRXIMG_VERSION ||
        header_size != IRXIMG_HEADER_SIZE ||
        entry_size != IRXIMG_ENTRY_SIZE ||
        entry_table_offset != IRXIMG_HEADER_SIZE ||
        entry_count == 0u ||
        entry_count > IRXIMG_MAX_ENTRIES ||
        flags != 0u ||
        declared_file_size != file_size)
        return PS2_DRIVERS_IMG_ERR_FORMAT;

    if (entry_count > (UINT32_MAX - entry_table_offset) / entry_size ||
        entry_table_offset + entry_count * entry_size > file_size ||
        payload_offset < entry_table_offset + entry_count * entry_size ||
        payload_offset > file_size ||
        (payload_offset & (IRXIMG_ALIGNMENT - 1u)) != 0u)
        return PS2_DRIVERS_IMG_ERR_FORMAT;

    table = (unsigned char *)malloc((size_t)entry_count * IRXIMG_ENTRY_SIZE);
    entries = (struct ps2_irx_image_entry *)calloc(entry_count, sizeof(*entries));
    if (table == NULL || entries == NULL) {
        free(table);
        free(entries);
        return PS2_DRIVERS_IMG_ERR_MEMORY;
    }

    if (fseek(file, (long)entry_table_offset, SEEK_SET) != 0 ||
        fread(table, IRXIMG_ENTRY_SIZE, entry_count, file) != entry_count) {
        free(table);
        free(entries);
        return PS2_DRIVERS_IMG_ERR_IO;
    }

    for (i = 0; i < entry_count; ++i) {
        const unsigned char *raw = table + i * IRXIMG_ENTRY_SIZE;
        uint32_t j;

        entries[i].id = read_u32_le(raw + 0u);
        entries[i].offset = read_u32_le(raw + 4u);
        entries[i].size = read_u32_le(raw + 8u);
        entries[i].crc32 = read_u32_le(raw + 12u);
        entries[i].flags = read_u32_le(raw + 16u);

        if (entries[i].id == 0u ||
            entries[i].size == 0u ||
            entries[i].flags != 0u ||
            entries[i].offset < payload_offset ||
            entries[i].offset > file_size ||
            (entries[i].offset & (IRXIMG_ALIGNMENT - 1u)) != 0u ||
            entries[i].size > file_size - entries[i].offset) {
            free(table);
            free(entries);
            return PS2_DRIVERS_IMG_ERR_FORMAT;
        }

        for (j = 0; j < i; ++j) {
            if (entries[j].id == entries[i].id) {
                free(table);
                free(entries);
                return PS2_DRIVERS_IMG_ERR_FORMAT;
            }
        }
    }

    free(table);
    *entries_out = entries;
    *entry_count_out = entry_count;
    return PS2_DRIVERS_IMG_OK;
}

static const struct ps2_irx_image_entry *find_entry(
    const struct ps2_irx_image_entry *entries,
    uint32_t entry_count,
    uint32_t id)
{
    uint32_t i;

    for (i = 0; i < entry_count; ++i) {
        if (entries[i].id == id)
            return &entries[i];
    }

    return NULL;
}

static void free_staged_array(
    struct ps2_irx_staged_module *modules,
    size_t capacity)
{
    size_t i;

    if (modules == NULL)
        return;

    for (i = 0; i < capacity; ++i)
        free(modules[i].data);

    free(modules);
}

static int stage_from_path(const char *image_path, uint32_t driver_requirements)
{
    unsigned char requested[IRXIMG_MAX_ENTRIES + 1u] = {0};
    struct ps2_irx_image_entry *entries = NULL;
    struct ps2_irx_staged_module *staged = NULL;
    uint32_t entry_count = 0u;
    uint32_t file_size;
    long file_size_long;
    size_t requested_count = 0u;
    size_t staged_index = 0u;
    size_t staged_bytes = 0u;
    uint32_t id;
    int result;
    FILE *file;

    if (image_path == NULL ||
        driver_requirements == 0u ||
        (driver_requirements & ~PS2_DRIVER_REQ_KNOWN_MASK) != 0u)
        return PS2_DRIVERS_IMG_ERR_ARGUMENT;

    if (g_staged_modules != NULL || g_staged_count != 0u)
        return PS2_DRIVERS_IMG_ERR_ALREADY_STAGED;

    expand_requirements(driver_requirements, requested);

    for (id = 1u; id <= IRXIMG_MAX_ENTRIES; ++id) {
        if (requested[id])
            ++requested_count;
    }

    if (requested_count == 0u)
        return PS2_DRIVERS_IMG_ERR_ARGUMENT;

    file = fopen(image_path, "rb");
    if (file == NULL)
        return PS2_DRIVERS_IMG_ERR_OPEN;

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return PS2_DRIVERS_IMG_ERR_IO;
    }

    file_size_long = ftell(file);
    if (file_size_long < (long)IRXIMG_HEADER_SIZE ||
        (unsigned long)file_size_long > UINT32_MAX) {
        fclose(file);
        return PS2_DRIVERS_IMG_ERR_FORMAT;
    }
    file_size = (uint32_t)file_size_long;

    result = parse_image_entries(file, file_size, &entries, &entry_count);
    if (result != PS2_DRIVERS_IMG_OK) {
        fclose(file);
        return result;
    }

    staged = (struct ps2_irx_staged_module *)calloc(requested_count, sizeof(*staged));
    if (staged == NULL) {
        free(entries);
        fclose(file);
        return PS2_DRIVERS_IMG_ERR_MEMORY;
    }

    for (id = 1u; id <= IRXIMG_MAX_ENTRIES; ++id) {
        const struct ps2_irx_image_entry *entry;
        void *data;

        if (!requested[id])
            continue;

        entry = find_entry(entries, entry_count, id);
        if (entry == NULL) {
            result = PS2_DRIVERS_IMG_ERR_MISSING_MODULE;
            goto fail;
        }

        data = alloc_aligned(IRXIMG_ALIGNMENT, entry->size);
        if (data == NULL) {
            result = PS2_DRIVERS_IMG_ERR_MEMORY;
            goto fail;
        }

        if (fseek(file, (long)entry->offset, SEEK_SET) != 0 ||
            fread(data, 1, entry->size, file) != entry->size) {
            free(data);
            result = PS2_DRIVERS_IMG_ERR_IO;
            goto fail;
        }

        if (crc32_bytes((const unsigned char *)data, entry->size) != entry->crc32) {
            free(data);
            result = PS2_DRIVERS_IMG_ERR_CRC;
            goto fail;
        }

        staged[staged_index].id = id;
        staged[staged_index].data = data;
        staged[staged_index].size = entry->size;
        staged_bytes += entry->size;
        ++staged_index;
    }

    free(entries);
    fclose(file);

    g_staged_modules = staged;
    g_staged_capacity = requested_count;
    g_staged_count = requested_count;
    g_staged_bytes = staged_bytes;
    return PS2_DRIVERS_IMG_OK;

fail:
    free(entries);
    fclose(file);
    free_staged_array(staged, requested_count);
    return result;
}

int ps2_drivers_img_stage(const char *image_path, uint32_t driver_requirements)
{
    char *image_path_copy;
    size_t image_path_size;
    int result;

    if (image_path == NULL ||
        driver_requirements == 0u ||
        (driver_requirements & ~PS2_DRIVER_REQ_KNOWN_MASK) != 0u)
        return PS2_DRIVERS_IMG_ERR_ARGUMENT;

    if (g_staged_modules != NULL || g_staged_count != 0u)
        return PS2_DRIVERS_IMG_ERR_ALREADY_STAGED;

    image_path_size = strlen(image_path) + 1u;
    image_path_copy = (char *)malloc(image_path_size);
    if (image_path_copy == NULL)
        return PS2_DRIVERS_IMG_ERR_MEMORY;

    memcpy(image_path_copy, image_path, image_path_size);

    result = stage_from_path(image_path, driver_requirements);
    if (result != PS2_DRIVERS_IMG_OK) {
        free(image_path_copy);
        return result;
    }

    free(g_image_path);
    g_image_path = image_path_copy;
    return PS2_DRIVERS_IMG_OK;
}

int ps2_drivers_img_restage(uint32_t driver_requirements)
{
    if (driver_requirements == 0u ||
        (driver_requirements & ~PS2_DRIVER_REQ_KNOWN_MASK) != 0u)
        return PS2_DRIVERS_IMG_ERR_ARGUMENT;

    if (g_image_path == NULL)
        return PS2_DRIVERS_IMG_ERR_NO_SOURCE;

    return stage_from_path(g_image_path, driver_requirements);
}

void ps2_drivers_img_discard_staged(void)
{
    free_staged_array(g_staged_modules, g_staged_capacity);
    g_staged_modules = NULL;
    g_staged_capacity = 0u;
    g_staged_count = 0u;
    g_staged_bytes = 0u;
}

void ps2_drivers_img_forget_source(void)
{
    free(g_image_path);
    g_image_path = NULL;
}

size_t ps2_drivers_img_staged_bytes(void)
{
    return g_staged_bytes;
}

size_t ps2_drivers_img_staged_modules(void)
{
    return g_staged_count;
}

int ps2_drivers_img_get_staged(uint32_t module_id, void **data, unsigned int *size)
{
    size_t i;

    if (data == NULL || size == NULL)
        return -1;

    for (i = 0; i < g_staged_capacity; ++i) {
        if (g_staged_modules[i].id == module_id && g_staged_modules[i].data != NULL) {
            *data = g_staged_modules[i].data;
            *size = g_staged_modules[i].size;
            return 0;
        }
    }

    return -1;
}

void ps2_drivers_img_release_staged(uint32_t module_id)
{
    size_t i;

    for (i = 0; i < g_staged_capacity; ++i) {
        if (g_staged_modules[i].id == module_id && g_staged_modules[i].data != NULL) {
            free(g_staged_modules[i].data);
            g_staged_modules[i].data = NULL;

            if (g_staged_bytes >= g_staged_modules[i].size)
                g_staged_bytes -= g_staged_modules[i].size;
            else
                g_staged_bytes = 0u;

            g_staged_modules[i].size = 0u;
            if (g_staged_count > 0u)
                --g_staged_count;
            break;
        }
    }

    if (g_staged_count == 0u && g_staged_modules != NULL) {
        free(g_staged_modules);
        g_staged_modules = NULL;
        g_staged_capacity = 0u;
        g_staged_bytes = 0u;
    }
}
