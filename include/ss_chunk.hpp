#pragma once

#include "ss_value.hpp"
#include "ss_opcodes.hpp"

namespace swiftscript {

struct Chunk;

// Upvalue descriptor for compilation
struct UpvalueInfo {
    uint16_t index;    // Index in enclosing scope (local or upvalue)
    bool is_local;     // true = captures local, false = captures upvalue
};

struct FunctionPrototype {
    std::string name;
    std::vector<std::string> params;
    std::vector<std::string> param_labels;
    struct ParamDefaultValue {
        bool has_default{false};
        Value value;
        std::optional<std::string> string_value;
    };
    std::vector<ParamDefaultValue> param_defaults;
    std::shared_ptr<Chunk> chunk;
    std::vector<UpvalueInfo> upvalues;  // Captured variables info
    bool is_initializer{false};
    bool is_override{false};
};

// Protocol method requirement
struct ProtocolMethodReq {
    std::string name;
    std::vector<std::string> param_names;
    bool is_mutating{false};
};

// Protocol property requirement
struct ProtocolPropertyReq {
    std::string name;
    bool has_getter{true};
    bool has_setter{false};
};

// Protocol definition
struct Protocol {
    std::string name;
    std::vector<ProtocolMethodReq> method_requirements;
    std::vector<ProtocolPropertyReq> property_requirements;
    std::vector<std::string> inherited_protocols;
};

    // Bytecode chunk
    struct Chunk {
        std::vector<uint8_t> code;
        std::vector<uint32_t> lines;
        std::vector<Value> constants;
        std::vector<std::string> strings;
        std::vector<FunctionPrototype> functions;
        std::vector<std::shared_ptr<Protocol>> protocols;

        void write(uint8_t byte, uint32_t line);
        void write_op(OpCode op, uint32_t line);

        size_t add_constant(Value value);
        size_t add_string(const std::string& str);
        size_t add_function(FunctionPrototype proto);
        size_t add_protocol(std::shared_ptr<Protocol> protocol);

        size_t emit_jump(OpCode op, uint32_t line);
        void patch_jump(size_t offset);

        void disassemble(const std::string& name) const;
        size_t disassemble_instruction(size_t offset) const;

    private:
        size_t simple_instruction(const char* name, size_t offset) const;
        size_t constant_instruction(const char* name, size_t offset) const;
        size_t string_instruction(const char* name, size_t offset) const;
        size_t short_instruction(const char* name, size_t offset) const;
        size_t jump_instruction(const char* name, int sign, size_t offset) const;
        size_t property_instruction(const char* name, size_t offset) const;
    };

} // namespace swiftscript
