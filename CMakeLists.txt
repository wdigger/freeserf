cmake_minimum_required(VERSION 3.16)
project(freeserf VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 11)

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_mixer CONFIG)
option(ENABLE_SDL3_MIXER "Enable SDL3_mixer support (music/sound effects)"
       ${SDL3_mixer_FOUND})

add_compile_definitions(
  "PACKAGE_BUGREPORT=\"https://github.com/freeserf/freeserf/issues\"")

include(CheckIncludeFile)
include(TestBigEndian)
foreach(header byteswap.h endian.h stdint.h unistd.h sys/endian.h)
  string(TOUPPER "HAVE_${header}" var)
  string(REGEX REPLACE "[./]" "_" var "${var}")
  check_include_file(${header} ${var})
  if(${var})
    add_compile_definitions(${var}=1)
  endif()
endforeach()
test_big_endian(WORDS_BIGENDIAN)
if(WORDS_BIGENDIAN)
  add_compile_definitions(WORDS_BIGENDIAN=1)
endif()

set(SOURCES
  src/freeserf.cc src/data.cc src/mission.c
  src/ai.c src/ai_road.c src/ai_manage.c src/ai_want.c src/ai_rate.c
  src/game.c src/serf.c src/flag.c src/building.c src/random.c
  src/pathfinder.cc src/gfx.cc src/viewport.cc src/minimap.cc
  src/interface.cc src/gui.cc src/popup.cc src/game-init.cc src/extro.cc
  src/notification.cc src/panel.cc src/map.c src/player.c
  src/video-sdl.cc src/video.cc src/audio.cc src/savegame.c src/list.c
  src/log.c src/version.c src/data-source-dos.cc src/tpwm.cc
  src/event_loop.cc src/event_loop-sdl.cc src/sfx2wav.cc src/xmi2mid.cc
  src/data-source.cc)

if(ENABLE_SDL3_MIXER)
  list(APPEND SOURCES src/audio-sdlmixer.cc)
else()
  list(APPEND SOURCES src/audio-dummy.cc)
endif()

# Version from git, shown in the log and the game start box.
find_package(Git QUIET)
set(VCS_VERSION "${PROJECT_VERSION}")
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
  execute_process(COMMAND ${GIT_EXECUTABLE} describe --always --tags --dirty
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  OUTPUT_VARIABLE VCS_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
file(WRITE ${CMAKE_BINARY_DIR}/version-vcs.h.tmp
     "#ifndef VERSION_VCS_H\n#define VERSION_VCS_H\n\n"
     "#define VERSION_VCS \"${VCS_VERSION}\"\n\n#endif /* VERSION_VCS_H */\n")
configure_file(${CMAKE_BINARY_DIR}/version-vcs.h.tmp
               ${CMAKE_BINARY_DIR}/version-vcs.h COPYONLY)

add_executable(freeserf ${SOURCES})
target_include_directories(freeserf PRIVATE ${CMAKE_SOURCE_DIR}
                           ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR})
target_link_libraries(freeserf PRIVATE SDL3::SDL3)
if(ENABLE_SDL3_MIXER)
  target_link_libraries(freeserf PRIVATE SDL3_mixer::SDL3_mixer)
endif()
if(NOT MSVC)
  target_link_libraries(freeserf PRIVATE m)
  # Warnings of the debug build (--enable-debug of the former autotools).
  target_compile_options(freeserf PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-pedantic -Wall>
    $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-Wall>)
endif()
