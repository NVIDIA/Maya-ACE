set(TARGET_NAME tests_a2x_local)

file(GLOB SRC "*.cpp")
# Add the required source files from maya_ace common
set(SRC
 ${SRC} 
 ${PROJECT_SOURCE_DIR}/source/maya_ace/common/a2x_models.cpp
)
add_executable(${TARGET_NAME} ${SRC})

target_include_directories(
    ${TARGET_NAME}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/source
    ${PROJECT_SOURCE_DIR}/source/maya_ace
    ${A2X_SDK_INCLUDE_DIRS}
    ${TENSORRT_INCLUDE_DIRS}
    ${RAPIDJSON_INCLUDE_DIRS}
)

target_link_directories(
    ${TARGET_NAME}
    PRIVATE
    ${A2X_SDK_LIBRARIES}
)

add_dependencies(${TARGET_NAME}
    maya_ace_2024
)

target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
    GTest::gtest
    GTest::gmock
    "audio2x"
)
