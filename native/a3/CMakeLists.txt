cmake_minimum_required(VERSION 3.20)
project(roomtone_native_a3 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
enable_testing()
add_executable(roomtone_a3_dsp_tests tests/dsp_tests.cpp)
add_test(NAME native_dsp_safety COMMAND roomtone_a3_dsp_tests)
add_executable(roomtone_a3_handoff_tests tests/handoff_tests.cpp)
add_test(NAME native_offline_session_handoff COMMAND roomtone_a3_handoff_tests)

# Windows-only experimental executable; existing browser code remains unchanged.
if(WIN32)
  include(FetchContent)
  # Immutable upstream Git commit: miniaudio 0.11.25.
  FetchContent_Declare(miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_TAG 9634bedb5b5a2ca38c1ee7108a9358a4e233f14d
    GIT_SHALLOW FALSE)
  FetchContent_GetProperties(miniaudio)
  if(NOT miniaudio_POPULATED)
    FetchContent_Populate(miniaudio)
  endif()
  add_executable(roomtone_a3_native src/main.cpp)
  target_include_directories(roomtone_a3_native PRIVATE ${miniaudio_SOURCE_DIR})
  target_link_libraries(roomtone_a3_native PRIVATE ole32 uuid comdlg32)
  target_compile_definitions(roomtone_a3_native PRIVATE NOMINMAX MA_NO_ENCODING)
endif()
