#ifndef PS2_IRXIMG_HDD_BOOTSTRAP_ARGS_H
#define PS2_IRXIMG_HDD_BOOTSTRAP_ARGS_H

/* Index of the first explicit target argument, or argc to use elf_path.ini.
 * Some HDD ELF browsers pass both partition metadata and an IOP reset flag. */
int ps2_irximg_hdd_target_index(int argc, const char *const argv[]);

#endif
