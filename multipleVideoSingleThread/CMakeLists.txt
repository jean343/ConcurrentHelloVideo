cmake_minimum_required(VERSION 2.8)
project( multipleVideoSingleThread )


add_executable( multipleVideoSingleThread multipleVideoSingleThread.cpp )
target_link_libraries( multipleVideoSingleThread X11 pthread /opt/vc/src/hello_pi/libs/ilclient/libilclient.a boost_system /opt/vc/lib/libbcm_host.so /opt/vc/lib/libopenmaxil.so /opt/vc/lib/libvcos.so  /opt/vc/lib/libGLESv2.so /opt/vc/lib/libEGL.so ${Boost_LIBRARIES} )