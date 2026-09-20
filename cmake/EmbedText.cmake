# Generates C++ string literals from text files in INPUT_DIR.
# Expected -D variables:
#   INPUT_DIR, EXTENSIONS (comma-separated, e.g. obj or vert,frag),
#   OUT_H, OUT_CPP, HEADER_NAME (included by the .cpp)

if(NOT DEFINED INPUT_DIR OR NOT DEFINED EXTENSIONS OR NOT DEFINED OUT_H OR NOT DEFINED OUT_CPP OR NOT DEFINED HEADER_NAME)
    message(FATAL_ERROR "EmbedText.cmake requires INPUT_DIR, EXTENSIONS, OUT_H, OUT_CPP, and HEADER_NAME")
endif()

string(REPLACE "," ";" EXT_LIST "${EXTENSIONS}")
set(TEXT_FILES)
foreach(EXT ${EXT_LIST})
    file(GLOB FOUND "${INPUT_DIR}/*.${EXT}")
    list(APPEND TEXT_FILES ${FOUND})
endforeach()
list(SORT TEXT_FILES)

if(NOT TEXT_FILES)
    message(FATAL_ERROR "No files with extensions [${EXTENSIONS}] found in ${INPUT_DIR}")
endif()

get_filename_component(OUT_DIR "${OUT_CPP}" DIRECTORY)
file(MAKE_DIRECTORY "${OUT_DIR}")

set(H_CONTENT "#pragma once\n\nnamespace kerf {\nnamespace embedded {\n\n")
set(CPP_CONTENT "#include \"${HEADER_NAME}\"\n\nnamespace kerf {\nnamespace embedded {\n\n")

foreach(TEXT_FILE ${TEXT_FILES})
    get_filename_component(FNAME "${TEXT_FILE}" NAME)
    string(REPLACE "." "_" SYMBOL "${FNAME}")
    file(READ "${TEXT_FILE}" SOURCE)
    string(FIND "${SOURCE}" ")kerfshd\"" DELIM_HIT)
    if(NOT DELIM_HIT EQUAL -1)
        message(FATAL_ERROR "${TEXT_FILE} contains the embed delimiter )kerfshd\"")
    endif()
    string(APPEND H_CONTENT "extern const char ${SYMBOL}[];\n")
    string(APPEND CPP_CONTENT "const char ${SYMBOL}[] = R\"kerfshd(${SOURCE})kerfshd\";\n\n")
endforeach()

string(APPEND H_CONTENT "\n}\n}\n")
string(APPEND CPP_CONTENT "}\n}\n")

file(WRITE "${OUT_H}" "${H_CONTENT}")
file(WRITE "${OUT_CPP}" "${CPP_CONTENT}")
