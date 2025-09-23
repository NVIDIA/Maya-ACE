include(FindPackageHandleStandardArgs)

find_path(TENSORRT_INCLUDE_DIR NvInfer.h
    HINTS ${TENSORRT_ROOT_DIR}
    PATH_SUFFIXES include)

find_path(TENSORRT_BIN_DIR trtexec.exe
    HINTS ${TENSORRT_ROOT_DIR}
    PATH_SUFFIXES bin)

find_path(TENSORRT_LIB_DIR nvinfer_10.dll
    HINTS ${TENSORRT_ROOT_DIR}
    PATH_SUFFIXES lib lib64)

find_package_handle_standard_args(TensorRT DEFAULT_MSG
                                  TENSORRT_INCLUDE_DIR
                                  TENSORRT_BIN_DIR
                                  TENSORRT_LIB_DIR)

if(TENSORRT_FOUND)
    # get TensorRT version
    file(READ ${TENSORRT_INCLUDE_DIR}/NvInferVersion.h TENSORRT_HEADER_CONTENTS)
    string(REGEX MATCH "define NV_TENSORRT_MAJOR * +([0-9]+)"
                 TENSORRT_VERSION_MAJOR "${TENSORRT_HEADER_CONTENTS}")
    string(REGEX REPLACE "define NV_TENSORRT_MAJOR * +([0-9]+)" "\\1"
                 TENSORRT_VERSION_MAJOR "${TENSORRT_VERSION_MAJOR}")
    string(REGEX MATCH "define NV_TENSORRT_MINOR * +([0-9]+)"
                 TENSORRT_VERSION_MINOR "${TENSORRT_HEADER_CONTENTS}")
    string(REGEX REPLACE "define NV_TENSORRT_MINOR * +([0-9]+)" "\\1"
                 TENSORRT_VERSION_MINOR "${TENSORRT_VERSION_MINOR}")
    string(REGEX MATCH "define NV_TENSORRT_PATCH * +([0-9]+)"
                 TENSORRT_VERSION_PATCH "${TENSORRT_HEADER_CONTENTS}")
    string(REGEX REPLACE "define NV_TENSORRT_PATCH * +([0-9]+)" "\\1"
                 TENSORRT_VERSION_PATCH "${TENSORRT_VERSION_PATCH}")
    string(REGEX MATCH "define NV_TENSORRT_BUILD * +([0-9]+)"
                 TENSORRT_VERSION_BUILD "${TENSORRT_HEADER_CONTENTS}")
    string(REGEX REPLACE "define NV_TENSORRT_BUILD * +([0-9]+)" "\\1"
                 TENSORRT_VERSION_BUILD "${TENSORRT_VERSION_BUILD}")

    # Fallback. get TRT ENTERPRISE version from header
    if(NOT TENSORRT_VERSION_MAJOR)
        string(REGEX MATCH "define TRT_MAJOR_ENTERPRISE * +([0-9]+)"
                    TENSORRT_VERSION_MAJOR "${TENSORRT_HEADER_CONTENTS}")
        string(REGEX REPLACE "define TRT_MAJOR_ENTERPRISE * +([0-9]+)" "\\1"
                    TENSORRT_VERSION_MAJOR "${TENSORRT_VERSION_MAJOR}")
        string(REGEX MATCH "define TRT_MINOR_ENTERPRISE * +([0-9]+)"
                    TENSORRT_VERSION_MINOR "${TENSORRT_HEADER_CONTENTS}")
        string(REGEX REPLACE "define TRT_MINOR_ENTERPRISE * +([0-9]+)" "\\1"
                    TENSORRT_VERSION_MINOR "${TENSORRT_VERSION_MINOR}")
        string(REGEX MATCH "define TRT_PATCH_ENTERPRISE * +([0-9]+)"
                    TENSORRT_VERSION_PATCH "${TENSORRT_HEADER_CONTENTS}")
        string(REGEX REPLACE "define TRT_PATCH_ENTERPRISE * +([0-9]+)" "\\1"
                    TENSORRT_VERSION_PATCH "${TENSORRT_VERSION_PATCH}")
        string(REGEX MATCH "define TRT_BUILD_ENTERPRISE * +([0-9]+)"
                    TENSORRT_VERSION_BUILD "${TENSORRT_HEADER_CONTENTS}")
        string(REGEX REPLACE "define TRT_BUILD_ENTERPRISE * +([0-9]+)" "\\1"
                    TENSORRT_VERSION_BUILD "${TENSORRT_VERSION_BUILD}")
    endif()

    # Assemble TensorRT version
    if(NOT TENSORRT_VERSION_MAJOR)
        set(TENSORRT_VERSION "?")
    else()
        set(TENSORRT_VERSION
        "${TENSORRT_VERSION_MAJOR}.${TENSORRT_VERSION_MINOR}.${TENSORRT_VERSION_PATCH}.${TENSORRT_VERSION_BUILD}")
    endif()
    message(STATUS "Found TensorRT: v${TENSORRT_VERSION}  (include: ${TENSORRT_INCLUDE_DIR}, library: ${TENSORRT_LIB_DIR}, bin: ${TENSORRT_BIN_DIR})")
endif()

unset(TENSORRT_HEADER_CONTENTS)
unset(TENSORRT_INCLUDE_DIR)
unset(TENSORRT_VERSION_MAJOR)
unset(TENSORRT_VERSION_MINOR)
unset(TENSORRT_VERSION_PATCH)
