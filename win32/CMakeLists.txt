cmake_minimum_required(VERSION 3.16)

# LWCloneU2 – Windows-Teil / Windows part
#   ledwiz.dll  – Ersatz für die originale LED-Wiz-DLL (asynchrones USB-I/O)
#   testapp     – kleines Testprogramm, schaltet alle Ausgänge ein
#   lwcconfig   – Kommandozeilen-Tool: LED-Wiz-ID ändern, Bandbreite messen
#
# Baut mit MSVC, MinGW-w64 oder LLVM-MinGW (clang), 32 und 64 Bit.
# Works with MSVC, MinGW-w64 or LLVM-MinGW (clang), 32 and 64 bit.

project(lwcloneu2_win32 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
    message(FATAL_ERROR "Dieser Teil ist nur für Windows. / This part is Windows only.")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Ausgabe: bin/x86 bzw. bin/x64 neben dem Quellbaum
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(LWC_ARCH x64)
else()
    set(LWC_ARCH x86)
endif()
set(LWC_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${LWC_ARCH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LWC_BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LWC_BIN_DIR})
foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${cfg} CFG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG} ${LWC_BIN_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG} ${LWC_BIN_DIR})
endforeach()

add_compile_definitions(WIN32 _WINDOWS UNICODE=0)

if(MSVC)
    add_compile_options(/W3)
    # Statische CRT → keine vcredist-Abhängigkeit für die DLL
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    add_compile_options(-Wall -Wno-unused-variable -Wno-unused-but-set-variable)
    # Keine Abhängigkeit von libstdc++/libgcc/libwinpthread-DLLs
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

# ---------------------------------------------------------------- ledwiz.dll
add_library(ledwiz SHARED
    driver/src/ledwiz.cpp
    driver/src/usbdev.cpp
    driver/src/ledwiz.def
    driver/include/ledwiz.h
    driver/src/usbdev.h
)
target_include_directories(ledwiz PUBLIC driver/include)
target_link_libraries(ledwiz PRIVATE hid setupapi)
set_target_properties(ledwiz PROPERTIES PREFIX "" OUTPUT_NAME "ledwiz")
if(NOT MSVC)
    # Exportnamen exakt wie in der .def (keine Dekoration), auch bei 32 Bit
    target_link_options(ledwiz PRIVATE -Wl,--kill-at)
endif()

# ------------------------------------------------------------------ testapp
add_executable(testapp testapp/src/main.cpp)
target_include_directories(testapp PRIVATE driver/include)
add_dependencies(testapp ledwiz)

# ---------------------------------------------------------------- lwcconfig
add_executable(lwcconfig lwcconfig/src/main.cpp)
target_include_directories(lwcconfig PRIVATE driver/include)
add_dependencies(lwcconfig ledwiz)
