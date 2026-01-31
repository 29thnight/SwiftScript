// OpCode handlers implementation
#pragma once
#include <stdexcept>

namespace swiftscript {

    // Opcodes
    enum class OpCode : uint8_t {
        // Constants & stack
        OP_CONSTANT,
        OP_STRING,
        OP_NIL,
        OP_TRUE,
        OP_FALSE,
        OP_POP,
        OP_DUP,  // Duplicate top of stack

        // Arithmetic
        OP_ADD,
        OP_SUBTRACT,
        OP_MULTIPLY,
        OP_DIVIDE,
        OP_MODULO,
        OP_NEGATE,

        // Bitwise operations
        OP_BITWISE_NOT,
        OP_BITWISE_AND,
        OP_BITWISE_OR,
        OP_BITWISE_XOR,
        OP_LEFT_SHIFT,
        OP_RIGHT_SHIFT,

        // Comparison
        OP_EQUAL,
        OP_NOT_EQUAL,
        OP_LESS,
        OP_GREATER,
        OP_LESS_EQUAL,
        OP_GREATER_EQUAL,

        // Logic
        OP_NOT,
        OP_AND,
        OP_OR,

        // Variables
        OP_GET_GLOBAL,
        OP_SET_GLOBAL,
        OP_GET_LOCAL,
        OP_SET_LOCAL,

        // Control flow
        OP_JUMP,
        OP_JUMP_IF_FALSE,
        OP_JUMP_IF_NIL,
        OP_LOOP,

        // Functions / Classes
        OP_FUNCTION,
        OP_CLOSURE,            // Create closure with upvalues
        OP_CLASS,              // Create class object
        OP_METHOD,             // Attach method to class on stack
        OP_DEFINE_PROPERTY,    // Register stored property metadata on class
        OP_DEFINE_COMPUTED_PROPERTY, // Register computed property with getter/setter
        OP_INHERIT,            // Link subclass to superclass
        OP_CALL,
        OP_CALL_NAMED,
        OP_RETURN,

        // Upvalues (for closures)
        OP_GET_UPVALUE,        // Get captured variable
        OP_SET_UPVALUE,        // Set captured variable
        OP_CLOSE_UPVALUE,      // Close upvalue when variable goes out of scope

        // Objects & members
        OP_GET_PROPERTY,
        OP_SET_PROPERTY,
        OP_SUPER,
        OP_OPTIONAL_CHAIN,

        // Optional handling
        OP_UNWRAP,
        OP_NIL_COALESCE,

        // Range operators
        OP_RANGE_INCLUSIVE,    // ... operator
        OP_RANGE_EXCLUSIVE,    // ..< operator

        // Collection operations
        OP_ARRAY,              // Create array from N elements on stack
        OP_DICT,               // Create dict from N key-value pairs on stack
        OP_GET_SUBSCRIPT,      // array[index] or dict[key]
        OP_SET_SUBSCRIPT,      // array[index] = value or dict[key] = value

        // Tuple operations
        OP_TUPLE,              // Create tuple from N elements on stack
        OP_GET_TUPLE_INDEX,    // tuple.0, tuple.1 (index access)
        OP_GET_TUPLE_LABEL,    // tuple.x, tuple.y (label access)

        // Struct operations
        OP_STRUCT,             // Create struct type object
        OP_STRUCT_METHOD,      // Attach method to struct (with mutating flag)
        OP_COPY_VALUE,         // Deep copy struct instance for value semantics

        // Property observers
        OP_DEFINE_PROPERTY_WITH_OBSERVERS, // Define property with willSet/didSet observers

        // Enum operations
        OP_ENUM,               // Create enum type object
        OP_ENUM_CASE,          // Define enum case
        OP_MATCH_ENUM_CASE,    // Check enum case name against a value
        OP_GET_ASSOCIATED,     // Get associated value by index

        // Protocol operations
        OP_PROTOCOL,           // Create protocol object
        OP_DEFINE_GLOBAL,      // Define global variable

        // Type operations
        OP_TYPE_CHECK,         // is operator: check if value is of type
        OP_TYPE_CAST,          // as operator: cast value to type
        OP_TYPE_CAST_OPTIONAL, // as? operator: optional cast
        OP_TYPE_CAST_FORCED,   // as! operator: forced cast

        // Error handling
        OP_THROW,              // throw error

        // Misc
        OP_READ_LINE,
        OP_PRINT,
        OP_HALT,
    };
} // namespace swiftscript
