# CMakeList.txt : CMake project for IconHider, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("IconHider")

# Add source to this project's executable.
add_executable (IconHider "IconHider.cpp" "IconHider.h" bt.rc)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET IconHider PROPERTY CXX_STANDARD 20)
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Копируем bat-файл в директорию билда после сборки
add_custom_command(TARGET IconHider POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/IconHiderAddStartup.bat 
                              $<TARGET_FILE_DIR:IconHider>
)

# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0")

# TODO: Add tests and install targets if needed.
