add_executable(quickstart-example main.cpp)
target_link_libraries(quickstart-example small)

add_executable(default_inline_storage-example default_inline_storage.cpp)
target_link_libraries(default_inline_storage-example small)

add_executable(unicode_strings-example unicode_strings.cpp)
target_link_libraries(unicode_strings-example small)

add_executable(associative-example associative.cpp)
target_link_libraries(associative-example small)