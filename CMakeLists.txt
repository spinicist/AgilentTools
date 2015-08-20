cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(AgilentTools CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
aux_source_directory(. SRC_LIST)
include(CheckCXXCompilerFlag)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

include_directories(Source)

add_library(agilent Source/fid.cpp Source/fidFile.cpp
                    Source/fdf.cpp Source/fdfFile.cpp
                    Source/procpar.cpp Source/util.cpp )
add_library(nifti   Source/niiNifti.cpp Source/niiHeader.cpp
                    Source/niiInternal.cpp Source/niiExtension.cpp
                    Source/niiZipFile.cpp
                    Source/niiInternal-inl.h Source/niiNifti-inl.h
                    Source/niiEnum.h Source/niiExtensionCodes.h )

set(PROGRAMS procparse fdf2nii fid2nii )

foreach(PROGRAM ${PROGRAMS})
    add_executable(${PROGRAM} Source/${PROGRAM}.cpp)
    target_link_libraries(${PROGRAM} agilent nifti z )
endforeach(PROGRAM)


set(SCRIPTS_DIR Scripts)
set(PY_TOOLS Source/fdf2nii.py)

install(TARGETS ${PROGRAMS} RUNTIME DESTINATION bin)
install(FILES ${PY_TOOLS} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE DESTINATION bin)
