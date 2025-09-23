set(TARGET_NAME tests_maya_plugin)

file(GLOB SRC "*.cpp")

# Check if Maya SDK is available (using MAYA2024 variables like in maya_ace)
if(DEFINED MAYA2024_INCLUDE_DIRS)
    message(STATUS "Building maya_utils tests with Maya SDK support")
    # When building with Maya SDK, also include the actual maya_utils source
    list(APPEND SRC "${PROJECT_SOURCE_DIR}/source/maya_ace/common/maya_utils.cpp")
endif()

add_executable(${TARGET_NAME} ${SRC})

# Common includes
target_include_directories(
    ${TARGET_NAME}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/source
)

if(DEFINED MAYA2024_INCLUDE_DIRS)
    target_include_directories(
        ${TARGET_NAME}
        PRIVATE
        ${MAYA2024_INCLUDE_DIRS}
    )
    target_link_directories(
        ${TARGET_NAME}
        PRIVATE
        ${MAYA2024_LIBRARIES}
    )
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
        "Foundation"
        "OpenMaya"
        GTest::gtest
        GTest::gmock
    )
    # Set RPATH for finding Maya libraries at runtime (Linux/macOS)
    if(NOT WIN32)
        set_target_properties(${TARGET_NAME} PROPERTIES
            INSTALL_RPATH "${MAYA2024_LIBRARY_DIR}"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    endif()
else()
    message(STATUS "Building maya_utils tests in mock mode (no Maya SDK)")
    # For testing without Maya SDK, we'll use mock implementations
    target_compile_definitions(${TARGET_NAME} PRIVATE MAYA_MOCK_MODE)
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
        GTest::gtest
        GTest::gmock
    )
endif()
