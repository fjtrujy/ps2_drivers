# Samples CMakeLists.txt
# This file includes all sample projects

message(STATUS "Building samples...")

# Add each sample subdirectory
add_subdirectory(alldrivers_sample)
add_subdirectory(alldrivers_verbose_sample)
# basic_elf_loader excluded - requires libloadfile which is not available
# add_subdirectory(basic_elf_loader)
add_subdirectory(filesystem_sample)
add_subdirectory(hdd_sample)
add_subdirectory(listdir_sample)
add_subdirectory(mtap_sample)
add_subdirectory(poweroff_sample)
add_subdirectory(showiopinfo_sample)
add_subdirectory(wav_sample)
