add_executable(broker
    main.cpp
    src/storage/persistent_log.cpp
)

target_include_directories(broker
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(broker
    PRIVATE
        shared
        Boost::boost
)