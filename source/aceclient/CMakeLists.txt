
set(TARGET_NAME aceclient)

file(GLOB SRC "*.cpp")
add_library(${TARGET_NAME} STATIC ${SRC})

target_include_directories(
    ${TARGET_NAME}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/source
    ${ABSL_INCLUDE_DIRS}
    ${AUDIOFILE_INCLUDE_DIRS}
    ${CARES_INCLUDE_DIRS}
    ${GRPC_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${RE2_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

target_link_directories(
    ${TARGET_NAME}
    PRIVATE
    ${ABSL_LIBRARIES}
    ${CARES_LIBRARIES}
    ${GRPC_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${RE2_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
    "ace-grpc-cpp"
)