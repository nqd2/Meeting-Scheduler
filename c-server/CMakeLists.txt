cmake_minimum_required(VERSION 3.16)
project(c-server C)

set(CMAKE_C_STANDARD 23)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOC REQUIRED libmongoc-1.0)
pkg_check_modules(BSON REQUIRED libbson-1.0)

include_directories(
        ${MONGOC_INCLUDE_DIRS}
        ${BSON_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}
)

link_directories(${MONGOC_LIBRARY_DIRS} ${BSON_LIBRARY_DIRS})

add_executable(c-server
        main.c
        db/db_core.c
        db/db_env.c
        db/db_user.c
        db/db_slot.c
        db/db_meeting.c
        db/db_utils.c
        net/net_server.c
        protocol/protocol_parser.c
        protocol/protocol_response.c
        session/session_manager.c
        handler/handler.c
        handler/handler_auth.c
        handler/handler_slot.c
        handler/handler_meeting.c
)

target_link_libraries(c-server ${MONGOC_LIBRARIES} ${BSON_LIBRARIES})

target_compile_options(c-server PRIVATE ${MONGOC_CFLAGS_OTHER} ${BSON_CFLAGS_OTHER} -Wall -Wextra)