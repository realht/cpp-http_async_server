cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

# обратите внимание на аргумент TARGETS у команды conan_basic_setup
include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

get_property(importTargets DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY IMPORTED_TARGETS)
message(STATUS "${importTargets}") 

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(GameLib STATIC 
    src/app.h
    src/app.cpp
    src/app_addition_struct.h
    src/app_model.h
    src/app_model.cpp
    src/collision_detector.h
    src/collision_detector.cpp
    src/db_connector.h
    src/database.h
    src/database.cpp
    src/extra_data.h
    src/extra_data.cpp
    src/geom.h
    src/json_loader.h
    src/json_loader.cpp
    src/loot_generator.h
    src/loot_generator.cpp
    src/model.h
    src/model.cpp
    src/model_serialization.h
    src/model_serialization.cpp
    src/player_tokens.h
    src/serializator.h
    src/serializator.cpp
    src/tagged.h
    src/tagged_uuid.h
    src/tagged_uuid.cpp
)

target_include_directories(GameLib PUBLIC CONAN_PKG::boost)
target_link_libraries(GameLib PUBLIC CONAN_PKG::boost Threads::Threads CONAN_PKG::libpq CONAN_PKG::libpqxx)

add_executable(game_server
    src/main.cpp
    src/api_handler.h
    src/api_handler.cpp
    src/api_handler_static_name.h
    src/http_server.cpp
    src/http_server.h
    src/logger.h
    src/logger.cpp
    src/logging_request_handler.h
    src/parse_command_line.h
    src/request_handler.cpp
    src/request_handler.h
    src/ticker.h
)

add_executable(game_server_tests
    tests/model_tests.cpp
    tests/loot_generator_tests.cpp
)

add_executable(collision_detection_tests
    tests/collision-detector-tests.cpp
)

add_executable(state_serialization_tests
    tests/state-serialization-tests.cpp
)

target_link_libraries(game_server GameLib)
target_link_libraries(game_server_tests CONAN_PKG::catch2 GameLib) 
target_link_libraries(collision_detection_tests CONAN_PKG::catch2 GameLib) 
target_link_libraries(state_serialization_tests CONAN_PKG::catch2 GameLib) 