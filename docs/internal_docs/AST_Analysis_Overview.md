# Swive AST ºÐ¼® Ã¼°è

## ?? °³¿ä

Swive´Â Swift-like ¹®¹ýÀ» »ç¿ëÇÏ´Â ½ºÅ©¸³Æ® ¾ð¾î·Î, ¼Ò½º ÄÚµå¸¦ ´ÙÀ½°ú °°Àº ÆÄÀÌÇÁ¶óÀÎÀ» ÅëÇØ Ã³¸®ÇÕ´Ï´Ù:

```
Source Code ¡æ Lexer ¡æ Tokens ¡æ Parser ¡æ AST ¡æ Type Checker ¡æ Compiler ¡æ Bytecode ¡æ VM
```

---

## ?? 1´Ü°è: ¾îÈÖ ºÐ¼® (Lexical Analysis)

### Lexer (`ss_lexer.hpp`, `ss_lexer.cpp`)

¼Ò½º ÄÚµå¸¦ ÅäÅ«(Token) ½ºÆ®¸²À¸·Î º¯È¯ÇÕ´Ï´Ù.

```cpp
class Lexer {
    Token next_token();
    std::vector<Token> tokenize_all();
};
```

#### Áö¿øÇÏ´Â ÅäÅ« Å¸ÀÔ (`ss_token.hpp`)

| Ä«Å×°í¸® | ÅäÅ« Å¸ÀÔ |
|---------|----------|
| ¸®ÅÍ·² | `Integer`, `Float`, `String`, `True`, `False`, `Null` |
| Å°¿öµå | `Func`, `Class`, `Struct`, `Enum`, `Protocol`, `Extension`, `Var`, `Let`, `If`, `Else`, `While`, `For`, `Return`, `Switch`, `Case`, ... |
| ¿¬»êÀÚ | `Plus`, `Minus`, `Star`, `Slash`, `Equal`, `EqualEqual`, `Less`, `Greater`, ... |
| ±¸ºÐÀÚ | `LeftParen`, `RightParen`, `LeftBrace`, `RightBrace`, `LeftBracket`, `RightBracket`, `Comma`, `Colon`, `Semicolon` |
| Æ¯¼ö | `InterpolatedStringStart`, `InterpolationStart`, `InterpolationEnd` |

---

## ?? 2´Ü°è: ±¸¹® ºÐ¼® (Parsing)

### Parser (`ss_parser.hpp`, `ss_parser.cpp`)

ÅäÅ« ½ºÆ®¸²À» AST(Abstract Syntax Tree)·Î º¯È¯ÇÕ´Ï´Ù.

```cpp
class Parser {
    std::vector<StmtPtr> parse();
};
```

### AST ±¸Á¶ (`ss_ast.hpp`)

#### Ç¥Çö½Ä (Expressions)

¸ðµç Ç¥Çö½ÄÀº `Expr` ±âº» Å¬·¡½º¸¦ »ó¼ÓÇÕ´Ï´Ù:

```cpp
enum class ExprKind {
    Literal,           // 42, "hello", true
    InterpolatedString,// "Hello, \(name)!"
    Identifier,        // myVariable
    Unary,             // -x, !flag
    Binary,            // a + b, x == y
    Assign,            // x = 5
    Call,              // foo(arg1, arg2)
    Member,            // obj.property
    Super,             // super.method()
    ForceUnwrap,       // optional!
    OptionalChain,     // obj?.property
    NilCoalesce,       // a ?? b
    Range,             // 1...10, 0..<5
    Ternary,           // condition ? a : b
    ArrayLiteral,      // [1, 2, 3]
    DictLiteral,       // ["key": value]
    Subscript,         // array[0]
    Closure,           // { (x) in x * 2 }
    TypeCast,          // expr as Type, expr as? Type
    TypeCheck,         // expr is Type
    TupleLiteral,      // (1, "hello")
    TupleMember,       // tuple.0, tuple.x
};
```

##### ÁÖ¿ä Ç¥Çö½Ä ±¸Á¶Ã¼

```cpp
// ¸®ÅÍ·² Ç¥Çö½Ä
struct LiteralExpr : Expr {
    Value value;
    std::optional<std::string> string_value;
};

// ÀÌÇ× ¿¬»ê Ç¥Çö½Ä
struct BinaryExpr : Expr {
    TokenType op;
    ExprPtr left;
    ExprPtr right;
};

// ÇÔ¼ö È£Ãâ Ç¥Çö½Ä
struct CallExpr : Expr {
    ExprPtr callee;
    std::vector<ExprPtr> arguments;
    std::vector<std::string> argument_names;  // Named parameters
};

// Å¬·ÎÀú Ç¥Çö½Ä
struct ClosureExpr : Expr {
    std::vector<std::pair<std::string, TypeAnnotation>> params;
    std::optional<TypeAnnotation> return_type;
    std::vector<StmtPtr> body;
};
```

#### ¹®Àå (Statements)

¸ðµç ¹®ÀåÀº `Stmt` ±âº» Å¬·¡½º¸¦ »ó¼ÓÇÕ´Ï´Ù:

```cpp
enum class StmtKind {
    Expression,        // Ç¥Çö½Ä ¹®Àå
    Print,             // print(expr)
    Block,             // { ... }
    VarDecl,           // var x = 5, let y: Int = 10
    TupleDestructuring,// let (a, b) = tuple
    ClassDecl,         // class MyClass { ... }
    StructDecl,        // struct Point { ... }
    EnumDecl,          // enum Direction { ... }
    ProtocolDecl,      // protocol Drawable { ... }
    ExtensionDecl,     // extension String { ... }
    If,                // if condition { ... }
    IfLet,             // if let x = optional { ... }
    GuardLet,          // guard let x = optional else { ... }
    While,             // while condition { ... }
    RepeatWhile,       // repeat { ... } while condition
    ForIn,             // for x in collection { ... }
    Break,             // break
    Continue,          // continue
    Switch,            // switch value { case ... }
    Return,            // return value
    FuncDecl,          // func name() { ... }
    Import,            // import "module.ss"
};
```

##### ÁÖ¿ä ¹®Àå ±¸Á¶Ã¼

```cpp
// º¯¼ö ¼±¾ð
struct VarDeclStmt : Stmt {
    std::string name;
    std::optional<TypeAnnotation> type_annotation;
    ExprPtr initializer;
    bool is_let{false};      // let vs var
    bool is_static{false};   // static property
    bool is_lazy{false};     // lazy initialization
    bool is_computed{false}; // computed property
    std::unique_ptr<BlockStmt> getter_body;
    std::unique_ptr<BlockStmt> setter_body;
    std::unique_ptr<BlockStmt> will_set_body;  // Property observer
    std::unique_ptr<BlockStmt> did_set_body;   // Property observer
    AccessLevel access_level{AccessLevel::Internal};
};

// ÇÔ¼ö ¼±¾ð
struct FuncDeclStmt : Stmt {
    std::string name;
    std::vector<std::string> generic_params;
    std::vector<GenericConstraint> generic_constraints;
    std::vector<ParamDecl> params;
    std::unique_ptr<BlockStmt> body;
    std::optional<TypeAnnotation> return_type;
    bool is_override{false};
    bool is_static{false};
    std::optional<TypeAnnotation> expected_error_type;
    AccessLevel access_level{AccessLevel::Internal};
};

// Å¬·¡½º ¼±¾ð
struct ClassDeclStmt : Stmt {
    std::string name;
    std::vector<std::string> generic_params;
    std::optional<std::string> superclass_name;
    std::vector<std::string> protocol_conformances;
    std::vector<std::unique_ptr<FuncDeclStmt>> methods;
    std::vector<std::unique_ptr<VarDeclStmt>> properties;
    std::unique_ptr<BlockStmt> deinit_body;
    AccessLevel access_level{AccessLevel::Internal};
};

// Struct ¼±¾ð
struct StructDeclStmt : Stmt {
    std::string name;
    std::vector<std::string> generic_params;
    std::vector<std::string> protocol_conformances;
    std::vector<std::unique_ptr<VarDeclStmt>> properties;
    std::vector<std::unique_ptr<StructMethodDecl>> methods;
    std::vector<std::unique_ptr<FuncDeclStmt>> initializers;
};

// Enum ¼±¾ð
struct EnumDeclStmt : Stmt {
    std::string name;
    std::vector<EnumCaseDecl> cases;
    std::optional<TypeAnnotation> raw_type;
    std::vector<std::unique_ptr<StructMethodDecl>> methods;
};
```

#### Å¸ÀÔ ¾î³ëÅ×ÀÌ¼Ç

```cpp
struct TypeAnnotation {
    std::string name;
    bool is_optional{false};      // Int?
    bool is_function_type{false}; // (Int) -> String
    bool is_tuple_type{false};    // (Int, String)
    std::vector<TypeAnnotation> param_types;
    std::shared_ptr<TypeAnnotation> return_type;
    std::vector<TypeAnnotation> generic_args;  // Array<Int>
    std::vector<TupleTypeElement> tuple_elements;
};
```

---

## ? 3´Ü°è: Å¸ÀÔ °Ë»ç (Type Checking)

### TypeChecker (`ss_type_checker.hpp`, `ss_type_checker.cpp`)

AST¸¦ ¼øÈ¸ÇÏ¸ç Å¸ÀÔ Á¤ÇÕ¼ºÀ» °ËÁõÇÕ´Ï´Ù.

```cpp
class TypeChecker {
    void check(const std::vector<StmtPtr>& program);
};
```

#### Å¸ÀÔ Á¤º¸ ±¸Á¶

```cpp
struct TypeInfo {
    std::string name;
    bool is_optional;
    TypeKind kind;  // Builtin, User, Protocol, Function, GenericParameter, Tuple, Unknown
    std::vector<TypeInfo> param_types;
    std::shared_ptr<TypeInfo> return_type;
    std::vector<TupleElementInfo> tuple_elements;
};
```

#### ÁÖ¿ä °Ë»ç Ç×¸ñ

1. **º¯¼ö Å¸ÀÔ °Ë»ç**: ¼±¾ðµÈ Å¸ÀÔ°ú ÇÒ´ç °ªÀÇ Å¸ÀÔ ÀÏÄ¡ È®ÀÎ
2. **ÇÔ¼ö È£Ãâ °Ë»ç**: ÀÎÀÚ Å¸ÀÔ°ú ¸Å°³º¯¼ö Å¸ÀÔ ÀÏÄ¡ È®ÀÎ
3. **¸Þ¼­µå Á¢±Ù °Ë»ç**: Á¢±Ù Á¦¾îÀÚ (public, private, internal, fileprivate)
4. **ÇÁ·ÎÅäÄÝ ÁØ¼ö °Ë»ç**: ÇÊ¼ö ¸Þ¼­µå/ÇÁ·ÎÆÛÆ¼ ±¸Çö ¿©ºÎ
5. **Á¦³×¸¯ Å¸ÀÔ Æ¯¼öÈ­**: `Array<Int>`, `Box<String>` µîÀÇ Å¸ÀÔ Ãß·Ð

---

## ?? 4´Ü°è: ÄÄÆÄÀÏ (Compilation)

### Compiler (`ss_compiler.hpp`, `ss_compiler.cpp`)

AST¸¦ ¹ÙÀÌÆ®ÄÚµå·Î º¯È¯ÇÕ´Ï´Ù.

```cpp
class Compiler {
    Assembly compile(const std::vector<StmtPtr>& program);
};
```

#### ÄÄÆÄÀÏ °úÁ¤

1. **Á¦³×¸¯ Æ¯¼öÈ­**: Á¦³×¸¯ Å¸ÀÔ/ÇÔ¼ö¸¦ ±¸Ã¼Àû Å¸ÀÔÀ¸·Î ÀÎ½ºÅÏ½ºÈ­
2. **Å¸ÀÔ Ã¼Å© ½ÇÇà**: `TypeChecker::check()` È£Ãâ
3. **¹®Àå ÄÄÆÄÀÏ**: °¢ ¹®ÀåÀ» ¼øÈ¸ÇÏ¸ç ¹ÙÀÌÆ®ÄÚµå »ý¼º
4. **¸ÞÅ¸µ¥ÀÌÅÍ »ý¼º**: Å¸ÀÔ/¸Þ¼­µå/ÇÊµå Á¤ÀÇ Å×ÀÌºí ±¸¼º

#### Visitor ÆÐÅÏ

```cpp
// Statement visitors
void visit(VarDeclStmt* stmt);
void visit(FuncDeclStmt* stmt);
void visit(ClassDeclStmt* stmt);
void visit(StructDeclStmt* stmt);
void visit(EnumDeclStmt* stmt);
void visit(IfStmt* stmt);
void visit(WhileStmt* stmt);
void visit(ForInStmt* stmt);
void visit(ReturnStmt* stmt);
// ...

// Expression visitors
void visit(LiteralExpr* expr);
void visit(BinaryExpr* expr);
void visit(CallExpr* expr);
void visit(MemberExpr* expr);
void visit(ClosureExpr* expr);
// ...
```

#### ½ºÄÚÇÁ °ü¸®

```cpp
struct Local {
    std::string name;
    int depth;              // ½ºÄÚÇÁ ±íÀÌ
    bool is_optional;
    bool is_captured;       // Å¬·ÎÀú¿¡ Ä¸Ã³ ¿©ºÎ
    std::string type_name;
};

std::vector<Local> locals_;
int scope_depth_{0};
```

---

## ?? 5´Ü°è: Ãâ·Â (Assembly)

### Assembly ±¸Á¶ (`ss_chunk.hpp`)

ÄÄÆÄÀÏµÈ ¹ÙÀÌÆ®ÄÚµå¿Í ¸ÞÅ¸µ¥ÀÌÅÍ¸¦ ´ã´Â ÄÁÅ×ÀÌ³ÊÀÔ´Ï´Ù.

```cpp
struct Assembly {
    AssemblyManifest manifest;
    
    // ¸ÞÅ¸µ¥ÀÌÅÍ Å×ÀÌºí
    std::vector<std::string> string_table;      // ¹®ÀÚ¿­ »ó¼ö Ç®
    std::vector<Value> global_constant_pool;    // Àü¿ª »ó¼ö Ç®
    std::vector<TypeDef> type_definitions;      // Å¸ÀÔ Á¤ÀÇ
    std::vector<MethodDef> method_definitions;  // ¸Þ¼­µå Á¤ÀÇ
    std::vector<FieldDef> field_definitions;    // ÇÊµå Á¤ÀÇ
    std::vector<PropertyDef> property_definitions;
    std::vector<uint8_t> signature_blob;        // ¸Þ¼­µå ½Ã±×´ÏÃ³
    
    // ¹ÙÀÌÆ®ÄÚµå
    std::vector<MethodBody> method_bodies;      // ¸Þ¼­µå º»¹®
    std::vector<FunctionPrototype> function_prototypes;
    std::vector<Protocol> protocols;
};
```

---

## ??? 6´Ü°è: ½ÇÇà (Virtual Machine)

### VM (`ss_vm.hpp`, `ss_vm.cpp`)

¹ÙÀÌÆ®ÄÚµå¸¦ ÇØ¼®ÇÏ°í ½ÇÇàÇÕ´Ï´Ù.

```cpp
class VM {
    Value interpret(const std::string& source);
    Value execute(const Assembly& chunk);
    Value run();
};
```

#### OPCODE Ã³¸®

```cpp
// OPCODE ÇÚµé·¯ Å×ÀÌºí (ss_vm_opcodes.inl¿¡¼­ »ý¼º)
const std::array<OpHandlerFunc, 256> g_opcode_handlers = make_handler_table();

Value VM::run() {
    while(true) {
        OpCode op = static_cast<OpCode>(read_byte());
        auto handler = g_opcode_handlers[static_cast<uint8_t>(op)];
        handler(*this);
        // ...
    }
}
```

---

## ?? ÀüÃ¼ Èå¸§ ¿ä¾à

```
¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
¦¢                        Source Code                               ¦¢
¦¢  let x: Int = 42                                                ¦¢
¦¢  func greet(name: String) -> String { return "Hello, \(name)" } ¦¢
¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
                              ¦¢
                              ¡å
¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
¦¢                         Lexer                                    ¦¢
¦¢  [Let] [Identifier:"x"] [Colon] [Identifier:"Int"] [Equal] ...  ¦¢
¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
                              ¦¢
                              ¡å
¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
¦¢                         Parser                                   ¦¢
¦¢  VarDeclStmt { name: "x", type: "Int", init: LiteralExpr(42) }  ¦¢
¦¢  FuncDeclStmt { name: "greet", params: [...], body: [...] }     ¦¢
¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
                              ¦¢
                              ¡å
¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
¦¢                       Type Checker                               ¦¢
¦¢  - Verify Int == Int ?                                          ¦¢
¦¢  - Verify String param type ?                                   ¦¢
¦¢  - Verify return type matches ?                                 ¦¢
¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
                              ¦¢
                              ¡å
¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
¦¢                        Compiler                                  ¦¢
¦¢  OP_CONSTANT 42                                                 ¦¢
¦¢  OP_DEFINE_GLOBAL "x"                                           ¦¢
¦¢  OP_CLOSURE <function_index>                                    ¦¢
¦¢  ...                                                            ¦¢
¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
                              ¦¢
                              ¡å
¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
¦¢                     Virtual Machine                              ¦¢
¦¢  Execute bytecode, manage stack, handle objects                 ¦¢
¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
```

---

## ?? °ü·Ã ÆÄÀÏ ±¸Á¶

```
src/common/
¦§¦¡¦¡ ss_lexer.hpp/.cpp      # ¾îÈÖ ºÐ¼®±â
¦§¦¡¦¡ ss_token.hpp/.cpp      # ÅäÅ« Á¤ÀÇ
¦§¦¡¦¡ ss_parser.hpp/.cpp     # ±¸¹® ºÐ¼®±â
¦§¦¡¦¡ ss_ast.hpp             # AST ³ëµå Á¤ÀÇ
¦§¦¡¦¡ ss_ast_clone.cpp       # AST º¹Á¦ À¯Æ¿¸®Æ¼
¦§¦¡¦¡ ss_type_checker.hpp/.cpp# Å¸ÀÔ °Ë»ç±â
¦§¦¡¦¡ ss_compiler.hpp/.cpp   # ÄÄÆÄÀÏ·¯
¦§¦¡¦¡ ss_chunk.hpp/.cpp      # Assembly ±¸Á¶
¦§¦¡¦¡ ss_opcodes.hpp/.def    # OPCODE Á¤ÀÇ
¦§¦¡¦¡ ss_vm.hpp/.cpp         # °¡»ó ¸Ó½Å
¦§¦¡¦¡ ss_vm_opcodes.inl      # OPCODE ÇÚµé·¯
¦§¦¡¦¡ ss_vm_opcodes_basic.inl# ±âº» OPCODE ÇÚµé·¯
¦¦¦¡¦¡ ss_value.hpp/.cpp      # °ª Å¸ÀÔ Á¤ÀÇ
```
