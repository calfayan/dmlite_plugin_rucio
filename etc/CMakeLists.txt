cmake_minimum_required (VERSION 2.6)

# Configuration files
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/rucio.conf.in
                ${CMAKE_CURRENT_BINARY_DIR}/rucio.conf
                @ONLY)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/ca.crt.in
                ${CMAKE_CURRENT_BINARY_DIR}/ca.crt
                @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/rucio.conf
         DESTINATION etc/dmlite.conf.d)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ca.crt
         DESTINATION etc/rucio.conf.d)
