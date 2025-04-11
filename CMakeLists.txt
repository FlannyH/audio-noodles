# CMakeList.txt : CMake project for AudioNoodles, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Update submodules as needed
find_package(Git QUIET)
if(GIT_FOUND)
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule init")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule init
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
  endif()
else()
  message(FATAL_ERROR "Failed to find Git!")
endif()

project ("AudioNoodles")

# Add source to this project's executable.
add_executable (AudioNoodles 
  "source/audio_noodle.cpp"
  "source/log.cpp"
  "source/log.hpp"
  "source/adsr.cpp"
  "source/adsr.hpp"
  "source/midi.cpp"
  "source/midi.hpp"
  "source/input.cpp"
  "source/input.hpp"
  "source/mixer.cpp"
  "source/mixer.hpp"
  "source/track.cpp"
  "source/track.hpp"
  "source/common.hpp"
  "source/session.cpp"
  "source/session.hpp"
  "source/processor.cpp"
  "source/processor.hpp"
  "source/ui/scene.cpp"
  "source/ui/scene.hpp"
  "source/ui/panel.cpp"
  "source/ui/panel.hpp"
  "source/ui/components.hpp"
  "source/ui/value_system.hpp"
  "source/graphics/device.cpp"
  "source/graphics/device.hpp"
  "source/graphics/renderer.cpp"
  "source/graphics/renderer.hpp"
  "source/graphics/resource.hpp"
  "source/graphics/transform.cpp"
  "source/graphics/transform.hpp"
  "source/graphics/opengl/device_opengl.cpp"
  "source/graphics/opengl/device_opengl.hpp"
  "source/processors/wav_osc.cpp"
  "source/processors/wav_osc.hpp"
)

# Set debug working directory
set_target_properties(
    AudioNoodles PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET AudioNoodles PROPERTY CXX_STANDARD 20)
endif()

# todo: only compile on windows and linux on x86/64 CPUs
add_subdirectory(external/glfw)
add_subdirectory(external/glbinding)
add_subdirectory(external/glm)
add_subdirectory(external/portaudio)
add_subdirectory(external/rtmidi)
target_link_libraries(AudioNoodles 
  glfw
  glbinding
  glm::glm
  portaudio
  rtmidi
)
target_include_directories(AudioNoodles PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/external/glm/
  ${CMAKE_CURRENT_SOURCE_DIR}/external/stb/
  ${CMAKE_CURRENT_SOURCE_DIR}/external/portaudio/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/external/rtmidi/
)

# Copy runtime files to build output
file(COPY assets/ DESTINATION ${CMAKE_BINARY_DIR}/assets)
