cmake_minimum_required(VERSION 2.8)
project( videoPerf )

FIND_PACKAGE( Boost REQUIRED COMPONENTS thread )

add_executable( videoPerf videoPerf.cpp )
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries( videoPerf X11 /opt/vc/src/hello_pi/libs/ilclient/libilclient.a boost_system /opt/vc/lib/libbcm_host.so /opt/vc/lib/libopenmaxil.so /opt/vc/lib/libvcos.so  /opt/vc/lib/libGLESv2.so /opt/vc/lib/libEGL.so ${Boost_LIBRARIES} )