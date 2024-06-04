aux_source_directory("${CMAKE_CURRENT_SOURCE_DIR}" SRC)

add_library(${PROJECT_NAME}-lib ${SRC})

target_include_directories(${PROJECT_NAME}-lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if (CMAKE_SYSTEM_NAME MATCHES Linux)
    target_link_libraries(${PROJECT_NAME}-lib PRIVATE udev mount)
endif (CMAKE_SYSTEM_NAME MATCHES Linux)

set_target_properties(${PROJECT_NAME}-lib PROPERTIES 
                        OUTPUT_NAME ${PROJECT_NAME}
                        PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/hostid.h")

install(TARGETS ${PROJECT_NAME}-lib
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)