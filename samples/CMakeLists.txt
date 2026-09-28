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
add_subdirectory(irximg_sample)
add_subdirectory(listdir_sample)
add_subdirectory(mtap_sample)
add_subdirectory(network_ee_sample)
add_subdirectory(network_iop_sample)
add_subdirectory(select_test_ee)
add_subdirectory(select_test_iop)
add_subdirectory(tcp_burst_client_ee)
add_subdirectory(tcp_server_ee)
add_subdirectory(udp_echo_server_ee)
add_subdirectory(poweroff_sample)
add_subdirectory(showiopinfo_sample)
add_subdirectory(wav_sample)
