# Embeds a binary file as unsigned char data[] + data_len.
# Expected -D variables: INPUT_FILE, OUT_H, OUT_CPP, HEADER_NAME

if(NOT DEFINED INPUT_FILE OR NOT DEFINED OUT_H OR NOT DEFINED OUT_CPP OR NOT DEFINED HEADER_NAME)
    message(FATAL_ERROR "EmbedBinary.cmake requires INPUT_FILE, OUT_H, OUT_CPP, and HEADER_NAME")
endif()

if(NOT EXISTS "${INPUT_FILE}")
    message(FATAL_ERROR "EmbedBinary.cmake: ${INPUT_FILE} does not exist")
endif()

get_filename_component(FNAME "${INPUT_FILE}" NAME)
string(REPLACE "." "_" SYMBOL "${FNAME}")

file(READ "${INPUT_FILE}" HEX HEX)
string(LENGTH "${HEX}" HEX_LEN)
if(HEX_LEN EQUAL 0)
    message(FATAL_ERROR "EmbedBinary.cmake: ${INPUT_FILE} is empty")
endif()
math(EXPR BYTE_LEN "${HEX_LEN} / 2")

string(REGEX REPLACE "([0-9A-Fa-f][0-9A-Fa-f])" "0x\\1," BYTES "${HEX}")

get_filename_component(OUT_DIR "${OUT_CPP}" DIRECTORY)
file(MAKE_DIRECTORY "${OUT_DIR}")

file(WRITE "${OUT_H}"
"#pragma once

namespace kerf {
namespace embedded {

extern const unsigned char ${SYMBOL}[${BYTE_LEN}];
extern const unsigned int ${SYMBOL}_len;

}
}
")

file(WRITE "${OUT_CPP}"
"#include \"${HEADER_NAME}\"

namespace kerf {
namespace embedded {

const unsigned char ${SYMBOL}[${BYTE_LEN}] = { ${BYTES} };
const unsigned int ${SYMBOL}_len = ${BYTE_LEN};

}
}
")
