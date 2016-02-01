cmake_minimum_required(VERSION 2.8)
project( opengl )

FIND_PACKAGE( Boost REQUIRED COMPONENTS thread )

add_executable( opengl opengl.cpp triangle.cpp )
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries( opengl X11 /opt/vc/src/hello_pi/libs/ilclient/libilclient.a boost_system /opt/vc/lib/libbcm_host.so /opt/vc/lib/libopenmaxil.so /opt/vc/lib/libvcos.so  /opt/vc/lib/libGLESv2.so /opt/vc/lib/libEGL.so ${Boost_LIBRARIES} )