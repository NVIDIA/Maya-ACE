# Maya DevKit paths and availability checking
set(MAYA2024_ROOT "${PROJECT_SOURCE_DIR}/deps/devkitBase2024")
set(MAYA2024_INCLUDE_DIRS "${MAYA2024_ROOT}/include")
set(MAYA2024_LIBRARIES "${MAYA2024_ROOT}/lib")

set(MAYA2025_ROOT "${PROJECT_SOURCE_DIR}/deps/devkitBase2025")
set(MAYA2025_INCLUDE_DIRS "${MAYA2025_ROOT}/include")
set(MAYA2025_LIBRARIES "${MAYA2025_ROOT}/lib")

# Maya 2022 is not heavily tested, you may need to make your own adjustments for compatibility.
set(MAYA2022_ROOT "${PROJECT_SOURCE_DIR}/deps/devkitBase2022")
set(MAYA2022_INCLUDE_DIRS "${MAYA2022_ROOT}/include")
set(MAYA2022_LIBRARIES "${MAYA2022_ROOT}/lib")

set(MAYA2026_ROOT "${PROJECT_SOURCE_DIR}/deps/devkitBase2026")
set(MAYA2026_INCLUDE_DIRS "${MAYA2026_ROOT}/include")
set(MAYA2026_LIBRARIES "${MAYA2026_ROOT}/lib")

# Check which Maya versions are actually available
set(MAYA_VERSIONS 2022 2024 2025 2026)
set(MAYA_FOUND_ANY FALSE)

foreach(year IN LISTS MAYA_VERSIONS)
    # Check if both include directory and library directory exist
    if(EXISTS "${MAYA${year}_INCLUDE_DIRS}" AND EXISTS "${MAYA${year}_LIBRARIES}")
        # Also check for essential Maya headers to confirm it's a valid installation
        if(EXISTS "${MAYA${year}_INCLUDE_DIRS}/maya/MFn.h")
            set(MAYA${year}_FOUND TRUE)
            set(MAYA_FOUND_ANY TRUE)
            message(STATUS "Found Maya ${year} DevKit at: ${MAYA${year}_ROOT}")
        else()
            set(MAYA${year}_FOUND FALSE)
            message(STATUS "Maya ${year} DevKit directory exists but missing essential headers: ${MAYA${year}_ROOT}")
        endif()
    else()
        set(MAYA${year}_FOUND FALSE)
        message(STATUS "Maya ${year} DevKit not found at: ${MAYA${year}_ROOT}")
    endif()
endforeach()

# Set the overall MAYA_FOUND variable
set(MAYA_FOUND ${MAYA_FOUND_ANY})

if(MAYA_FOUND)
    message(STATUS "Maya DevKit found for one or more versions")
else()
    if(Maya_FIND_REQUIRED)
        message(FATAL_ERROR "Maya DevKit not found. Please ensure Maya DevKit is properly installed in the deps directory.")
    else()
        message(WARNING "Maya DevKit not found. Maya-dependent targets will be skipped.")
    endif()
endif()
