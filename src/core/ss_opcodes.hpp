// OpCode handlers implementation
#pragma once
#include <stdexcept>

namespace swive {

    // Opcodes
    enum class OpCode : uint8_t {
#define X(name) name,
#include "ss_opcodes.def"
#undef X
    };
} // namespace swive
