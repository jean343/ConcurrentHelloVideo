cmake_minimum_required(VERSION 2.8)

include_directories("/opt/vc/include/interface/vcos/pthreads/;/opt/vc/include/interface/vmcs_host/linux")
include_directories(/opt/vc/include /opt/vc/src/hello_pi/libs/ilclient/ ${Boost_INCLUDE_DIR})

add_subdirectory(multipleVideoSingleThread)
add_subdirectory(video)
add_subdirectory(videoPerf)