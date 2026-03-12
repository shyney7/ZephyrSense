# build_web.cmake — Build-time script for CesiumJS web assets
#
# Invoked via: cmake -DWEB_DIR=<path> -DOUTPUT_QRC=<path> -P build_web.cmake
#
# In CI ($ENV{CI} set), skips npm and expects dist/ to be pre-built.
# Locally, runs npm install + npm run build, then generates a .qrc file
# listing all files in dist/ for AUTORCC embedding.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED WEB_DIR)
    message(FATAL_ERROR "WEB_DIR not defined. Pass -DWEB_DIR=<path>")
endif()
if(NOT DEFINED OUTPUT_QRC)
    message(FATAL_ERROR "OUTPUT_QRC not defined. Pass -DOUTPUT_QRC=<path>")
endif()

set(WEB_DIST_DIR "${WEB_DIR}/dist")

# ── npm build (skipped in CI) ────────────────────────────────────────────────
if(NOT DEFINED ENV{CI})
    find_program(NPM_EXECUTABLE NAMES npm.cmd npm)
    if(NOT NPM_EXECUTABLE)
        message(FATAL_ERROR
            "npm not found. Install Node.js or ensure npm is on PATH.")
    endif()

    # CMAKE_HOST_WIN32 = build host platform (correct for -P scripts)
    # WIN32 = target platform (may not be set in script mode)
    if(CMAKE_HOST_WIN32)
        set(NPM_CMD cmd /c "${NPM_EXECUTABLE}")
    else()
        set(NPM_CMD "${NPM_EXECUTABLE}")
    endif()

    message(STATUS "[ZephyrSense] Running npm install...")
    execute_process(
        COMMAND ${NPM_CMD} install
        WORKING_DIRECTORY "${WEB_DIR}"
        RESULT_VARIABLE _result
    )
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "npm install failed (exit code ${_result})")
    endif()

    message(STATUS "[ZephyrSense] Running npm run build...")
    execute_process(
        COMMAND ${NPM_CMD} run build
        WORKING_DIRECTORY "${WEB_DIR}"
        RESULT_VARIABLE _result
    )
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "npm run build failed (exit code ${_result})")
    endif()
else()
    message(STATUS "[ZephyrSense] CI detected — skipping npm (web assets pre-built)")
endif()

# ── Verify dist output ───────────────────────────────────────────────────────
if(NOT EXISTS "${WEB_DIST_DIR}/index.html")
    message(FATAL_ERROR
        "web/dist/index.html not found. "
        "Run 'cd web && npm install && npm run build' first.")
endif()

# ── Generate .qrc ────────────────────────────────────────────────────────────
file(GLOB_RECURSE dist_files
    RELATIVE "${WEB_DIST_DIR}"
    "${WEB_DIST_DIR}/*"
)
list(LENGTH dist_files file_count)

# Normalize backslashes to forward slashes for rcc compatibility (Windows paths)
string(REPLACE "\\" "/" WEB_DIST_DIR_FWD "${WEB_DIST_DIR}")

set(qrc "<!DOCTYPE RCC><RCC version=\"1.0\">\n<qresource prefix=\"/web\">\n")
foreach(f IN LISTS dist_files)
    string(REPLACE "\\" "/" f_fwd "${f}")
    string(APPEND qrc
        "    <file alias=\"${f_fwd}\">${WEB_DIST_DIR_FWD}/${f_fwd}</file>\n")
endforeach()
string(APPEND qrc "</qresource>\n</RCC>\n")

# Always write — Ninja needs OUTPUT mtime updated to stop re-scheduling this command.
# rcc is fast; spurious rcc reruns are cheaper than an infinite rebuild loop.
file(WRITE "${OUTPUT_QRC}" "${qrc}")
message(STATUS "[ZephyrSense] Generated web_resources.qrc (${file_count} files)")
