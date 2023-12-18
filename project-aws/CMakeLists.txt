# CMakeList.txt : CMake project for project-aws, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("project-aws")

# Add source to this project's executable.
add_executable (project-aws "project-aws.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET project-aws PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
set(SERVICE_COMPONENTS s3-crt logs sqs iam s3 sts)

find_package(AWSSDK REQUIRED COMPONENTS ${SERVICE_COMPONENTS})
if(MSVC AND BUILD_SHARED_LIBS)
    set(CMAKE_BUILD_TYPE Debug) # Explicitly setting CMAKE_BUILD_TYPE is necessary in Windows to copy DLLs.

    list(APPEND SERVICE_LIST ${SERVICE_COMPONENTS})

    AWSSDK_CPY_DYN_LIBS(SERVICE_LIST "" ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif()

target_link_libraries(project-aws ${AWSSDK_LINK_LIBRARIES})