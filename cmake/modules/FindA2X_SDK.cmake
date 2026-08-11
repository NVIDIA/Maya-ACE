include(FindPackageHandleStandardArgs)

find_path(A2X_SDK_INCLUDE_DIRS audio2face/audio2face.h
    HINTS ${A2X_SDK_ROOT}
    PATH_SUFFIXES include)

find_path(A2X_SDK_BIN_DIR audio2x.dll
    HINTS ${A2X_SDK_ROOT}
    PATH_SUFFIXES bin
    )

find_path(A2X_SDK_LIBRARIES audio2x.lib
    HINTS ${A2X_SDK_ROOT}
    PATH_SUFFIXES lib lib64
    )

find_package_handle_standard_args(A2X_SDK DEFAULT_MSG
                                  A2X_SDK_INCLUDE_DIRS
                                  A2X_SDK_BIN_DIR
                                  A2X_SDK_LIBRARIES)

if(A2X_SDK_FOUND)
    # get A2X_SDK version (potential future use)
    file(READ "${A2X_SDK_ROOT}/VERSION.md" A2X_SDK_VERSION_CONTENTS)
    string(REGEX MATCH "(([0-9]+)\.([0-9]+)\.([0-9]+)(\.([0-9]+))?)"
                 A2X_SDK_VERSION "${A2X_SDK_VERSION_CONTENTS}")

    # Assemble A2X_SDK version
    if(NOT A2X_SDK_VERSION)
        set(A2X_SDK_VERSION "?")
    endif()
    message(STATUS "Found A2X_SDK: v${A2X_SDK_VERSION}  (include: ${A2X_SDK_INCLUDE_DIRS}, library: ${A2X_SDK_LIBRARIES}, bin: ${A2X_SDK_BIN_DIR})")

    mark_as_advanced(A2X_SDK_ROOT
                     A2X_SDK_INCLUDE_DIRS
                     A2X_SDK_LIBRARIES
                     A2X_SDK_BIN_DIR)
endif()
