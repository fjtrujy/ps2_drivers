#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <delaythread.h>
#include <iopcontrol.h>
#include <kernel.h>
#include <sbv_patches.h>
#include <sifrpc.h>
#include <timer.h>

#include <ps2_cacheio_driver.h>
#include <ps2_drivers_img.h>
#include <ps2_fileXio_driver.h>
#include <ps2_usb_driver.h>

#define TEST_BLOCK_SIZE (64u * 1024u)
#define DEFAULT_TEST_ITERATIONS 4096u
#define DEFAULT_TEST_PATH "mass:/NJEMU-58704ed5-MVS/cache/mslug3_cache/crom"

static int report_fd = -1;

static void report(const char *format, ...)
{
    char buffer[512];
    va_list args;
    int length;

    va_start(args, format);
    length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length < 0)
        return;

    printf("%s", buffer);
    if (report_fd >= 0) {
        size_t bytes = (size_t)length;
        if (bytes >= sizeof(buffer))
            bytes = sizeof(buffer) - 1;
        write(report_fd, buffer, bytes);
    }
}

static void reset_iop(void)
{
    SifInitRpc(0);

    while (!SifIopReset(NULL, 0)) {
    }
    while (!SifIopSync()) {
    }

    SifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
}

static int wait_for_mass(void)
{
    struct stat stat_buffer;
    unsigned int attempt;

    for (attempt = 0; attempt < 1000u; ++attempt) {
        if (stat("mass:/", &stat_buffer) == 0)
            return 1;
        DelayThread(10000);
    }

    return 0;
}

static uint32_t next_random(uint32_t *state)
{
    *state = (*state * 1664525u) + 1013904223u;
    return *state;
}

static ssize_t read_posix_block(int fd, void *buffer)
{
    return read(fd, buffer, TEST_BLOCK_SIZE);
}

static void diagnose_posix_block(int fd, uint64_t block_offset, void *buffer)
{
    unsigned int sector;

    if (lseek(fd, (off_t)block_offset, SEEK_SET) < 0) {
        report("POSIX diagnostic seek failed at offset 0x%llx: errno=%d\n",
            (unsigned long long)block_offset, errno);
        return;
    }

    for (sector = 0; sector < TEST_BLOCK_SIZE / 512u; ++sector) {
        ssize_t bytes_read = read(fd, buffer, 512);
        if (bytes_read != 512) {
            report("POSIX 512-byte diagnostic failed at +0x%x: bytes=%ld errno=%d\n",
                sector * 512u, (long)bytes_read, errno);
            return;
        }
    }

    report("POSIX 512-byte diagnostic passed for entire failed block\n");
}

static uint32_t crc32_block(const unsigned char *data)
{
    uint32_t crc = ~0u;
    unsigned int i;

    for (i = 0; i < TEST_BLOCK_SIZE; ++i) {
        unsigned int bit;

        crc ^= data[i];
        for (bit = 0; bit < 8u; ++bit) {
            uint32_t mask = (uint32_t)(-(int32_t)(crc & 1u));
            crc = (crc >> 1) ^ (0xedb88320u & mask);
        }
    }

    return ~crc;
}

static int compare_buffers(const unsigned char *expected, const unsigned char *actual,
    uint64_t offset)
{
    unsigned int i;

    if (memcmp(expected, actual, TEST_BLOCK_SIZE) == 0)
        return 0;

    for (i = 0; i < TEST_BLOCK_SIZE; ++i) {
        if (expected[i] != actual[i]) {
            report("mismatch at file offset 0x%08x%08x + 0x%x: posix=%02x cacheio=%02x\n",
                (unsigned int)(offset >> 32), (unsigned int)offset, i,
                expected[i], actual[i]);
            break;
        }
    }

    return -1;
}

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : DEFAULT_TEST_PATH;
    unsigned int iterations = argc > 2 ? (unsigned int)strtoul(argv[2], NULL, 0)
                                        : DEFAULT_TEST_ITERATIONS;
    unsigned char *posix_buffer = NULL;
    unsigned char *cacheio_buffer = NULL;
    uint32_t *block_crc = NULL;
    uint64_t file_size;
    uint64_t block_count;
    uint32_t random_state = 0x4e4a454du;
    int cacheio_handle = -1;
    int fd = -1;
    unsigned int i;
    int result;
    u64 posix_ticks = 0, cacheio_ticks = 0;

    printf("cacheio compare: path=%s iterations=%u\n", path, iterations);

    result = ps2_drivers_img_stage_default(
        PS2_DRIVER_REQ_FILEXIO | PS2_DRIVER_REQ_USB | PS2_DRIVER_REQ_CACHEIO);
    if (result != PS2_DRIVERS_IMG_OK) {
        printf("staging failed: %s (%d)\n", ps2_drivers_img_error_string(result), result);
        return 1;
    }

    printf("staged modules=%u bytes=%u\n",
        (unsigned int)ps2_drivers_img_staged_modules(),
        (unsigned int)ps2_drivers_img_staged_bytes());

    reset_iop();

    if (init_fileXio_driver() < 0) {
        printf("fileXio init failed\n");
        return 2;
    }

    report_fd = open("host:cacheio_compare_result.txt",
        O_WRONLY | O_CREAT | O_TRUNC, 0666);
    report("cacheio compare: path=%s iterations=%u\n", path, iterations);
    report("fileXio initialized\n");

    if (init_usb_driver(true) < 0) {
        report("USB init failed\n");
        result = 3;
        goto cleanup;
    }
    report("USB stack initialized; waiting for mass:\n");

    if (!wait_for_mass()) {
        report("mass: did not become ready within 10 seconds\n");
        result = 4;
        goto cleanup;
    }
    report("mass: ready\n");

    if (init_cacheio_driver(false) != CACHEIO_INIT_STATUS_OK) {
        report("cacheio init failed\n");
        result = 5;
        goto cleanup;
    }
    report("cacheio initialized\n");

    report("opening POSIX file\n");
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        report("POSIX open failed: %d\n", errno);
        result = 6;
        goto cleanup;
    }
    report("POSIX file opened\n");

    {
        report("seeking POSIX file end\n");
        off_t end = lseek(fd, 0, SEEK_END);
        if (end <= 0) {
            report("failed to determine file size\n");
            result = 7;
            goto cleanup;
        }
        file_size = (uint64_t)end;
        report("POSIX file size resolved\n");
    }

    block_count = file_size / TEST_BLOCK_SIZE;
    if (block_count == 0) {
        report("file is smaller than one 64 KiB block\n");
        result = 8;
        goto cleanup;
    }

    report("opening cacheio handle\n");
    cacheio_handle = cacheioOpen(path);
    if (cacheio_handle < 0) {
        report("cacheioOpen failed: %d\n", cacheio_handle);
        result = 9;
        goto cleanup;
    }
    report("cacheio handle opened\n");

    posix_buffer = memalign(64, TEST_BLOCK_SIZE);
    cacheio_buffer = memalign(64, TEST_BLOCK_SIZE);
    if (block_count > SIZE_MAX / sizeof(*block_crc)) {
        report("block count is too large for CRC table\n");
        result = 10;
        goto cleanup;
    }
    block_crc = malloc((size_t)block_count * sizeof(*block_crc));
    if (posix_buffer == NULL || cacheio_buffer == NULL || block_crc == NULL) {
        report("buffer allocation failed\n");
        result = 10;
        goto cleanup;
    }

    report("file_size=%llu blocks=%llu\n",
        (unsigned long long)file_size, (unsigned long long)block_count);

    if (block_count > 70u) {
        report("diagnosing known failing block 70 before full comparison\n");
        diagnose_posix_block(fd, 70u * TEST_BLOCK_SIZE, posix_buffer);
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        report("failed to rewind POSIX file\n");
        result = 11;
        goto cleanup;
    }

    report("starting full sequential byte-for-byte comparison\n");
    for (i = 0; i < block_count; ++i) {
        uint64_t offset = (uint64_t)i * TEST_BLOCK_SIZE;
        int bytes;

        {
            ssize_t bytes_read = read_posix_block(fd, posix_buffer);
            if (bytes_read != (ssize_t)TEST_BLOCK_SIZE) {
                report("POSIX sequential read failed at block %u: bytes=%ld errno=%d\n",
                    i, (long)bytes_read, errno);
                diagnose_posix_block(fd, offset, posix_buffer);
                result = 11;
                goto cleanup;
            }
        }

        memset(cacheio_buffer, 0xa5, TEST_BLOCK_SIZE);
        bytes = cacheioReadAt(cacheio_handle, offset, cacheio_buffer, TEST_BLOCK_SIZE);
        if (bytes != (int)TEST_BLOCK_SIZE) {
            report("cacheioReadAt failed during sequential comparison at block %u: %d\n",
                i, bytes);
            result = 12;
            goto cleanup;
        }

        if (compare_buffers(posix_buffer, cacheio_buffer, offset) < 0) {
            report("FAILED after %u successful sequential comparisons\n", i);
            result = 13;
            goto cleanup;
        }

        block_crc[i] = crc32_block(posix_buffer);
        if (((i + 1u) & 127u) == 0u)
            report("byte-verified %u/%llu blocks\n",
                i + 1u, (unsigned long long)block_count);
    }

    report("PASS: all %llu blocks matched byte-for-byte\n",
        (unsigned long long)block_count);
    report("starting %u random cacheio offset checks\n", iterations);

    for (i = 0; i < iterations; ++i) {
        uint64_t block = next_random(&random_state) % block_count;
        uint64_t offset = block * TEST_BLOCK_SIZE;
        int bytes;
        uint32_t actual_crc;
        u64 start;
        ssize_t posix_bytes;

        memset(cacheio_buffer, 0xa5, TEST_BLOCK_SIZE);
        /* Alternate order to avoid always giving one path a warm device cache. */
        if ((i & 1u) == 0) {
            start = GetTimerSystemTime();
            if (lseek(fd, (off_t)offset, SEEK_SET) != (off_t)offset) {
                result = 16;
                goto cleanup;
            }
            posix_bytes = read_posix_block(fd, posix_buffer);
            posix_ticks += GetTimerSystemTime() - start;
        }
        start = GetTimerSystemTime();
        bytes = cacheioReadAt(cacheio_handle, offset, cacheio_buffer, TEST_BLOCK_SIZE);
        cacheio_ticks += GetTimerSystemTime() - start;
        if ((i & 1u) != 0) {
            start = GetTimerSystemTime();
            if (lseek(fd, (off_t)offset, SEEK_SET) != (off_t)offset) {
                result = 16;
                goto cleanup;
            }
            posix_bytes = read_posix_block(fd, posix_buffer);
            posix_ticks += GetTimerSystemTime() - start;
        }
        if (posix_bytes != (ssize_t)TEST_BLOCK_SIZE) {
            report("random POSIX read failed at iteration %u: bytes=%ld errno=%d\n",
                i, (long)posix_bytes, errno);
            result = 16;
            goto cleanup;
        }
        if (bytes != (int)TEST_BLOCK_SIZE) {
            report("random cacheioReadAt failed at iteration %u block %llu: %d\n",
                i, (unsigned long long)block, bytes);
            result = 14;
            goto cleanup;
        }

        if (compare_buffers(posix_buffer, cacheio_buffer, offset) < 0) {
            result = 13;
            goto cleanup;
        }
        actual_crc = crc32_block(cacheio_buffer);
        if (actual_crc != block_crc[block]) {
            report("random CRC mismatch at iteration %u block %llu: expected=%08x actual=%08x\n",
                i, (unsigned long long)block, block_crc[block], actual_crc);
            result = 15;
            goto cleanup;
        }

        if (((i + 1u) & 255u) == 0u)
            report("random-verified %u/%u blocks\n", i + 1u, iterations);
    }

    report("PASS: %u random cacheio reads matched sequential reference CRCs\n",
        iterations);
    report("PASS: %u random reads also matched POSIX byte-for-byte\n", iterations);
    report("random I/O ticks: posix_seek_read=%llu cacheio_read_at=%llu ticks_per_second=%u\n",
        (unsigned long long)posix_ticks, (unsigned long long)cacheio_ticks,
        (unsigned int)kBUSCLK);
    result = 0;

cleanup:
    report("RESULT: %s code=%d\n", result == 0 ? "PASS" : "FAIL", result);
    free(block_crc);
    free(cacheio_buffer);
    free(posix_buffer);
    if (cacheio_handle >= 0)
        cacheioClose(cacheio_handle);
    if (fd >= 0)
        close(fd);
    if (report_fd >= 0) {
        close(report_fd);
        report_fd = -1;
    }
    deinit_cacheio_driver(false);
    deinit_usb_driver(true);
    deinit_fileXio_driver();
    return result;
}
