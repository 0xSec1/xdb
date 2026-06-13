add_executable(tests tests.cpp)
add_subdirectory("target")
target_link_libraries(tests PRIVATE xdb::libxdb Catch2::Catch2WithMain)
