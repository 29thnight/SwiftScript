# ?? AST (Abstract Syntax Tree) ½ÉÃþ ºÐ¼®

## ¹ßÇ¥ ÀÚ·á: SwiveÀÇ AST º¯È¯ °úÁ¤

---

## ?? Part 1: AST¶õ ¹«¾ùÀÎ°¡?

### Á¤ÀÇ

**AST (Abstract Syntax Tree, Ãß»ó ±¸¹® Æ®¸®)**´Â ¼Ò½º ÄÚµåÀÇ ±¸Á¶¸¦ Æ®¸® ÇüÅÂ·Î Ç¥ÇöÇÑ ÀÚ·á±¸Á¶ÀÔ´Ï´Ù.

> "ÄÚµåÀÇ **ÀÇ¹Ì**´Â º¸Á¸ÇÏ¸é¼­, **¹®¹ýÀû ¼¼ºÎ»çÇ×**Àº Ãß»óÈ­ÇÑ Æ®¸®"

### ¿Ö AST°¡ ÇÊ¿äÇÑ°¡?

```
¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
¦¢  ¼Ò½º ÄÚµå (ÅØ½ºÆ®)                                          ¦¢
¦¢  "let x = 1 + 2 * 3"                                        ¦¢
¦¢                                                             ¦¢
¦¢  ? ¹®Á¦Á¡:                                                  ¦¢
¦¢  - ¹®ÀÚ¿­Àº ºÐ¼®ÇÏ±â ¾î·Æ´Ù                                   ¦¢
¦¢  - ¿¬»êÀÚ ¿ì¼±¼øÀ§°¡ ºÒºÐ¸íÇÏ´Ù                               ¦¢
¦¢  - ÁßÃ¸ ±¸Á¶¸¦ ÆÄ¾ÇÇÏ±â Èûµé´Ù                                ¦¢
¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
                              ¦¢
                              ¡å
¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
¦¢  AST (Æ®¸® ±¸Á¶)                                             ¦¢
¦¢                                                             ¦¢
¦¢         VarDecl("x")                                        ¦¢
¦¢              ¦¢                                              ¦¢
¦¢           Binary(+)                                         ¦¢
¦¢           ?      ?                                          ¦¢
¦¢      Literal(1)  Binary(*)     ¡ç ¿¬»êÀÚ ¿ì¼±¼øÀ§ ¹Ý¿µ!       ¦¢
¦¢                  ?      ?                                   ¦¢
¦¢             Literal(2) Literal(3)                           ¦¢
¦¢                                                             ¦¢
¦¢  ? ÀåÁ¡:                                                    ¦¢
¦¢  - ±¸Á¶°¡ ¸íÈ®ÇÏ´Ù                                           ¦¢
¦¢  - ¼øÈ¸ÇÏ±â ½±´Ù                                             ¦¢
¦¢  - º¯È¯/ÃÖÀûÈ­°¡ ¿ëÀÌÇÏ´Ù                                    ¦¢
¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
```

### Concrete vs Abstract Syntax Tree

| ±¸ºÐ | CST (Parse Tree) | AST |
|-----|------------------|-----|
| Á¤º¸·® | ¸ðµç ÅäÅ« Æ÷ÇÔ | ÀÇ¹ÌÀÖ´Â Á¤º¸¸¸ |
| °ýÈ£ | ¸í½ÃÀûÀ¸·Î Ç¥Çö | Æ®¸® ±¸Á¶·Î ¾Ï½Ã |
| ¼¼¹ÌÄÝ·Ð | Æ÷ÇÔ | Á¦¿Ü |
| ¿ëµµ | ÆÄ½Ì °á°ú | ºÐ¼®/ÄÄÆÄÀÏ¿ë |

```
¼Ò½º: (1 + 2) * 3

CST:                          AST:
    expr                         Binary(*)
   / | \                         ?      ?
  (  +  )  *  3               Binary(+)  Literal(3)
    / \                        ?    ?
   1   2                  Literal(1) Literal(2)
```

---

## ?? Part 2: AST ³ëµåÀÇ Á¾·ù

### ³ëµå ºÐ·ù Ã¼°è

```
                    ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
                    ¦¢  Node   ¦¢
                    ¦¦¦¡¦¡¦¡¦¡¦¨¦¡¦¡¦¡¦¡¦¥
           ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦ª¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
           ¡å                           ¡å
      ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤                 ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
      ¦¢  Expr  ¦¢                 ¦¢   Stmt   ¦¢
      ¦¢ (Ç¥Çö½Ä)¦¢                 ¦¢  (¹®Àå)  ¦¢
      ¦¦¦¡¦¡¦¡¦¡¦¨¦¡¦¡¦¡¦¥                 ¦¦¦¡¦¡¦¡¦¡¦¨¦¡¦¡¦¡¦¡¦¡¦¥
           ¦¢                          ¦¢
    ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦«¦¡¦¡¦¡¦¡¦¡¦¡¦¤           ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦«¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
    ¡å      ¡å      ¡å           ¡å       ¡å       ¡å
 Literal Binary Call       VarDecl  If    FuncDecl
  (°ª)   (¿¬»ê) (È£Ãâ)     (º¯¼ö¼±¾ð)(Á¶°Ç) (ÇÔ¼ö¼±¾ð)
```

### Ç¥Çö½Ä (Expression) - °ªÀ» ¹ÝÈ¯

```cpp
// Swive Ç¥Çö½Ä ¿¹½Ã
42                    // LiteralExpr
x                     // IdentifierExpr
-x                    // UnaryExpr
a + b                 // BinaryExpr
foo(1, 2)            // CallExpr
obj.property         // MemberExpr
arr[0]               // SubscriptExpr
x > 0 ? "¾ç¼ö" : "À½¼ö" // TernaryExpr
```

### ¹®Àå (Statement) - µ¿ÀÛÀ» ¼öÇà

```cpp
// Swive ¹®Àå ¿¹½Ã
let x = 10           // VarDeclStmt
if condition { }     // IfStmt
while true { }       // WhileStmt
for i in 0..<10 { }  // ForInStmt
return value         // ReturnStmt
func foo() { }       // FuncDeclStmt
class MyClass { }    // ClassDeclStmt
```

---

## ?? Part 3: ½ÇÁ¦ º¯È¯ °úÁ¤ ºÐ¼®

### ¿¹Á¦ ÄÚµå

```swift
func add(a: Int, b: Int) -> Int {
    let result = a + b
    return result
}
```

---

### ?? Step 1: Lexical Analysis (¾îÈÖ ºÐ¼®)

¼Ò½º ÄÚµå¸¦ **ÅäÅ«(Token)**À¸·Î ºÐ¸®ÇÕ´Ï´Ù.

```
¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
¦¢  ÀÔ·Â: "func add(a: Int, b: Int) -> Int { let result = ... ¦¢
¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
                              ¦¢
                         Lexer Ã³¸®
                              ¦¢
                              ¡å
¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
¦¢  Ãâ·Â: ÅäÅ« ½ºÆ®¸²                                            ¦¢
¦¢                                                              ¦¢
¦¢  [ Func     ] ¡ç Å°¿öµå                                       ¦¢
¦¢  [ add      ] ¡ç ½Äº°ÀÚ (Identifier)                          ¦¢
¦¢  [ (        ] ¡ç ±¸ºÐÀÚ (LeftParen)                           ¦¢
¦¢  [ a        ] ¡ç ½Äº°ÀÚ                                       ¦¢
¦¢  [ :        ] ¡ç ±¸ºÐÀÚ (Colon)                               ¦¢
¦¢  [ Int      ] ¡ç ½Äº°ÀÚ (Å¸ÀÔ¸í)                               ¦¢
¦¢  [ ,        ] ¡ç ±¸ºÐÀÚ (Comma)                               ¦¢
¦¢  [ b        ] ¡ç ½Äº°ÀÚ                                       ¦¢
¦¢  [ :        ] ¡ç ±¸ºÐÀÚ                                       ¦¢
¦¢  [ Int      ] ¡ç ½Äº°ÀÚ                                       ¦¢
¦¢  [ )        ] ¡ç ±¸ºÐÀÚ (RightParen)                          ¦¢
¦¢  [ ->       ] ¡ç ¿¬»êÀÚ (Arrow)                               ¦¢
¦¢  [ Int      ] ¡ç ½Äº°ÀÚ                                       ¦¢
¦¢  [ {        ] ¡ç ±¸ºÐÀÚ (LeftBrace)                           ¦¢
¦¢  [ let      ] ¡ç Å°¿öµå                                       ¦¢
¦¢  [ result   ] ¡ç ½Äº°ÀÚ                                       ¦¢
¦¢  [ =        ] ¡ç ¿¬»êÀÚ (Equal)                               ¦¢
¦¢  [ a        ] ¡ç ½Äº°ÀÚ                                       ¦¢
¦¢  [ +        ] ¡ç ¿¬»êÀÚ (Plus)                                ¦¢
¦¢  [ b        ] ¡ç ½Äº°ÀÚ                                       ¦¢
¦¢  [ return   ] ¡ç Å°¿öµå                                       ¦¢
¦¢  [ result   ] ¡ç ½Äº°ÀÚ                                       ¦¢
¦¢  [ }        ] ¡ç ±¸ºÐÀÚ (RightBrace)                          ¦¢
¦¢  [ EOF      ] ¡ç ³¡                                           ¦¢
¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
```

#### Token ±¸Á¶

```cpp
struct Token {
    TokenType type;     // ÅäÅ« Á¾·ù
    std::string lexeme; // ½ÇÁ¦ ¹®ÀÚ¿­
    uint32_t line;      // ÁÙ ¹øÈ£
    uint32_t column;    // ¿­ ¹øÈ£
};

// ¿¹½Ã
Token { type: Func, lexeme: "func", line: 1, column: 1 }
Token { type: Identifier, lexeme: "add", line: 1, column: 6 }
```

---

### ?? Step 2: Parsing (±¸¹® ºÐ¼®)

ÅäÅ« ½ºÆ®¸²À» **AST**·Î º¯È¯ÇÕ´Ï´Ù.

```
¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
¦¢  ÀÔ·Â: ÅäÅ« ½ºÆ®¸²                                            ¦¢
¦¢  [ Func ] [ add ] [ ( ] [ a ] [ : ] [ Int ] ...              ¦¢
¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
                              ¦¢
                         Parser Ã³¸®
                              ¦¢
                              ¡å
```

#### »ý¼ºµÈ AST Æ®¸®

```
                        FuncDeclStmt
                        ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
                        ¦¢ name: "add"                     ¦¢
                        ¦¢ return_type: "Int"              ¦¢
                        ¦¢ params: [ParamDecl, ParamDecl]  ¦¢
                        ¦¢ body: BlockStmt                 ¦¢
                        ¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
                                       ¦¢
          ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦«¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
          ¦¢                            ¦¢                            ¦¢
          ¡å                            ¡å                            ¡å
    ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤              ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤              ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
    ¦¢ ParamDecl ¦¢              ¦¢ ParamDecl ¦¢              ¦¢  BlockStmt   ¦¢
    ¦¢ name: "a" ¦¢              ¦¢ name: "b" ¦¢              ¦¢ statements[] ¦¢
    ¦¢ type: Int ¦¢              ¦¢ type: Int ¦¢              ¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
    ¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥              ¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥                     ¦¢
                                                    ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦ª¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
                                                    ¦¢                         ¦¢
                                                    ¡å                         ¡å
                                            ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤           ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
                                            ¦¢ VarDeclStmt ¦¢           ¦¢ ReturnStmt  ¦¢
                                            ¦¢ name:"result"¦¢           ¦¢ value: Expr ¦¢
                                            ¦¢ is_let: true¦¢           ¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
                                            ¦¢ init: Expr  ¦¢                  ¦¢
                                            ¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥                  ¦¢
                                                   ¦¢                         ¡å
                                                   ¡å                  ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
                                            ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤           ¦¢IdentifierExpr¦¢
                                            ¦¢ BinaryExpr  ¦¢           ¦¢ name:"result"¦¢
                                            ¦¢ op: Plus    ¦¢           ¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
                                            ¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
                                                   ¦¢
                                      ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦ª¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
                                      ¦¢                         ¦¢
                                      ¡å                         ¡å
                              ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤           ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
                              ¦¢IdentifierExpr¦¢           ¦¢IdentifierExpr¦¢
                              ¦¢ name: "a"   ¦¢           ¦¢ name: "b"   ¦¢
                              ¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥           ¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
```

---

### ?? Step 3: AST ³ëµå »ó¼¼ ºÐ¼®

#### FuncDeclStmt (ÇÔ¼ö ¼±¾ð ³ëµå)

```cpp
struct FuncDeclStmt {
    std::string name = "add";
    
    std::vector<ParamDecl> params = {
        { external_name: "",  internal_name: "a", type: Int },
        { external_name: "",  internal_name: "b", type: Int }
    };
    
    std::optional<TypeAnnotation> return_type = {
        name: "Int",
        is_optional: false
    };
    
    std::unique_ptr<BlockStmt> body = /* BlockStmt Æ÷ÀÎÅÍ */;
    
    bool is_static = false;
    bool is_override = false;
};
```

#### BinaryExpr (ÀÌÇ× ¿¬»ê ³ëµå)

```cpp
struct BinaryExpr {
    TokenType op = TokenType::Plus;  // +
    
    ExprPtr left = std::make_unique<IdentifierExpr>("a");
    ExprPtr right = std::make_unique<IdentifierExpr>("b");
};
```

---

### ?? Step 4: AST ¼øÈ¸ (Traversal)

ÄÄÆÄÀÏ·¯´Â AST¸¦ **±íÀÌ ¿ì¼± Å½»ö(DFS)**À¸·Î ¼øÈ¸ÇÕ´Ï´Ù.

```
¼øÈ¸ ¼ø¼­ (Post-order):

     FuncDeclStmt ¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡ 7?? visit
           ¦¢
     BlockStmt ¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡ 6?? visit
           ¦¢
    ¦£¦¡¦¡¦¡¦¡¦¡¦¡¦ª¦¡¦¡¦¡¦¡¦¡¦¡¦¤
    ¦¢             ¦¢
VarDeclStmt   ReturnStmt ¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡ 5?? visit
    ¦¢             ¦¢
    ¦¢        IdentifierExpr("result") ¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡ 4?? visit
    ¦¢
BinaryExpr(+) ¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡ 3?? visit
    ¦¢
 ¦£¦¡¦¡¦ª¦¡¦¡¦¤
 ¦¢     ¦¢
 a     b ¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡ 1?? 2?? visit
```

#### Visitor ÆÐÅÏ Àû¿ë

```cpp
void Compiler::visit(FuncDeclStmt* stmt) {
    // 1. ÇÔ¼ö ÇÁ·ÎÅäÅ¸ÀÔ »ý¼º
    // 2. »õ ½ºÄÚÇÁ ½ÃÀÛ
    begin_scope();
    
    // 3. ÆÄ¶ó¹ÌÅÍ¸¦ ·ÎÄÃ º¯¼ö·Î ¼±¾ð
    for (const auto& param : stmt->params) {
        declare_local(param.internal_name, false);
    }
    
    // 4. ÇÔ¼ö º»¹® ÄÄÆÄÀÏ
    visit(stmt->body.get());
    
    // 5. ½ºÄÚÇÁ Á¾·á
    end_scope();
}

void Compiler::visit(BinaryExpr* expr) {
    // 1. ¿ÞÂÊ ÇÇ¿¬»êÀÚ ÄÄÆÄÀÏ ¡æ ½ºÅÃ¿¡ push
    compile_expr(expr->left.get());
    
    // 2. ¿À¸¥ÂÊ ÇÇ¿¬»êÀÚ ÄÄÆÄÀÏ ¡æ ½ºÅÃ¿¡ push
    compile_expr(expr->right.get());
    
    // 3. ¿¬»êÀÚ¿¡ ÇØ´çÇÏ´Â OPCODE emit
    switch (expr->op) {
        case TokenType::Plus:  emit_op(OP_ADD); break;
        case TokenType::Minus: emit_op(OP_SUB); break;
        // ...
    }
}
```

---

## ?? Part 4: º¹ÀâÇÑ ¿¹Á¦ ºÐ¼®

### Å¬·¡½º¿Í ¸Þ¼­µå

```swift
class Calculator {
    var value: Int = 0
    
    func add(n: Int) -> Int {
        value = value + n
        return value
    }
}
```

#### AST ±¸Á¶

```
ClassDeclStmt
¦§¦¡¦¡ name: "Calculator"
¦§¦¡¦¡ properties[]
¦¢   ¦¦¦¡¦¡ VarDeclStmt
¦¢       ¦§¦¡¦¡ name: "value"
¦¢       ¦§¦¡¦¡ type: Int
¦¢       ¦§¦¡¦¡ is_let: false
¦¢       ¦¦¦¡¦¡ initializer: LiteralExpr(0)
¦¢
¦¦¦¡¦¡ methods[]
    ¦¦¦¡¦¡ FuncDeclStmt
        ¦§¦¡¦¡ name: "add"
        ¦§¦¡¦¡ params: [{ name: "n", type: Int }]
        ¦§¦¡¦¡ return_type: Int
        ¦¦¦¡¦¡ body: BlockStmt
            ¦§¦¡¦¡ ExprStmt
            ¦¢   ¦¦¦¡¦¡ AssignExpr
            ¦¢       ¦§¦¡¦¡ name: "value"
            ¦¢       ¦¦¦¡¦¡ value: BinaryExpr(+)
            ¦¢           ¦§¦¡¦¡ left: IdentifierExpr("value")
            ¦¢           ¦¦¦¡¦¡ right: IdentifierExpr("n")
            ¦¢
            ¦¦¦¡¦¡ ReturnStmt
                ¦¦¦¡¦¡ value: IdentifierExpr("value")
```

---

### Å¬·ÎÀú (Closure)

```swift
let multiply = { (x: Int, y: Int) -> Int in
    return x * y
}
```

#### AST ±¸Á¶

```
VarDeclStmt
¦§¦¡¦¡ name: "multiply"
¦§¦¡¦¡ is_let: true
¦¦¦¡¦¡ initializer: ClosureExpr
    ¦§¦¡¦¡ params[]
    ¦¢   ¦§¦¡¦¡ { name: "x", type: Int }
    ¦¢   ¦¦¦¡¦¡ { name: "y", type: Int }
    ¦§¦¡¦¡ return_type: Int
    ¦¦¦¡¦¡ body[]
        ¦¦¦¡¦¡ ReturnStmt
            ¦¦¦¡¦¡ value: BinaryExpr(*)
                ¦§¦¡¦¡ left: IdentifierExpr("x")
                ¦¦¦¡¦¡ right: IdentifierExpr("y")
```

---

### Switch ¹®

```swift
switch direction {
    case .north:
        print("ºÏÂÊ")
    case .south:
        print("³²ÂÊ")
    default:
        print("±âÅ¸")
}
```

#### AST ±¸Á¶

```
SwitchStmt
¦§¦¡¦¡ value: IdentifierExpr("direction")
¦¦¦¡¦¡ cases[]
    ¦§¦¡¦¡ CaseClause
    ¦¢   ¦§¦¡¦¡ patterns: [EnumCasePattern(".north")]
    ¦¢   ¦¦¦¡¦¡ statements: [PrintStmt("ºÏÂÊ")]
    ¦¢
    ¦§¦¡¦¡ CaseClause
    ¦¢   ¦§¦¡¦¡ patterns: [EnumCasePattern(".south")]
    ¦¢   ¦¦¦¡¦¡ statements: [PrintStmt("³²ÂÊ")]
    ¦¢
    ¦¦¦¡¦¡ CaseClause
        ¦§¦¡¦¡ is_default: true
        ¦¦¦¡¦¡ statements: [PrintStmt("±âÅ¸")]
```

---

## ?? Part 5: ASTÀÇ È°¿ë

### 1?? Å¸ÀÔ °Ë»ç (Type Checking)

```
AST ¼øÈ¸ÇÏ¸ç Å¸ÀÔ Á¤ÇÕ¼º °ËÁõ:

BinaryExpr(+)
¦§¦¡¦¡ left: IdentifierExpr("a")  ¡æ Å¸ÀÔ Á¶È¸ ¡æ Int
¦§¦¡¦¡ right: IdentifierExpr("b") ¡æ Å¸ÀÔ Á¶È¸ ¡æ Int
¦¦¦¡¦¡ °á°ú Å¸ÀÔ: Int + Int = Int ?
```

### 2?? ÄÚµå ÃÖÀûÈ­

```
ÃÖÀûÈ­ Àü AST:              ÃÖÀûÈ­ ÈÄ AST:

BinaryExpr(+)               LiteralExpr(5)
¦§¦¡¦¡ LiteralExpr(2)          
¦¦¦¡¦¡ LiteralExpr(3)          (»ó¼ö Æúµù)
```

### 3?? ¹ÙÀÌÆ®ÄÚµå »ý¼º

```
AST:                        Bytecode:
                           
BinaryExpr(+)       ¡æ      OP_GET_LOCAL 0    ; a¸¦ ½ºÅÃ¿¡
¦§¦¡¦¡ Identifier("a")        OP_GET_LOCAL 1    ; b¸¦ ½ºÅÃ¿¡  
¦¦¦¡¦¡ Identifier("b")        OP_ADD            ; ´õÇÏ±â
```

---

## ?? Part 6: Swive AST Æ¯¼ö ±â´É

### Optional Chaining

```swift
user?.address?.city
```

```
OptionalChainExpr
¦§¦¡¦¡ object: OptionalChainExpr
¦¢   ¦§¦¡¦¡ object: IdentifierExpr("user")
¦¢   ¦¦¦¡¦¡ member: "address"
¦¦¦¡¦¡ member: "city"
```

### Nil Coalescing

```swift
name ?? "Unknown"
```

```
NilCoalesceExpr
¦§¦¡¦¡ optional_expr: IdentifierExpr("name")
¦¦¦¡¦¡ fallback: LiteralExpr("Unknown")
```

### Tuple Destructuring

```swift
let (x, y) = point
```

```
TupleDestructuringStmt
¦§¦¡¦¡ bindings: [{ name: "x" }, { name: "y" }]
¦§¦¡¦¡ is_let: true
¦¦¦¡¦¡ initializer: IdentifierExpr("point")
```

---

## ?? ÇÙ½É Á¤¸®

| ´Ü°è | ÀÔ·Â | Ãâ·Â | ¿ªÇÒ |
|-----|-----|-----|-----|
| **Lexer** | ¼Ò½º ÄÚµå | ÅäÅ« ½ºÆ®¸² | ¹®ÀÚ¿­ ¡æ ÅäÅ« ºÐ¸® |
| **Parser** | ÅäÅ« ½ºÆ®¸² | AST | ¹®¹ý ±¸Á¶ ÆÄ¾Ç |
| **Type Checker** | AST | °ËÁõµÈ AST | Å¸ÀÔ Á¤ÇÕ¼º °Ë»ç |
| **Compiler** | AST | ¹ÙÀÌÆ®ÄÚµå | ½ÇÇà ÄÚµå »ý¼º |

### ASTÀÇ ÇÙ½É °¡Ä¡

```
¦£¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¤
¦¢                                                            ¦¢
¦¢  ? ±¸Á¶È­µÈ Ç¥Çö - ÄÚµåÀÇ °èÃþ ±¸Á¶¸¦ ¸íÈ®È÷ Ç¥Çö         ¦¢
¦¢                                                            ¦¢
¦¢  ? Ãß»óÈ­ - ºÒÇÊ¿äÇÑ ¹®¹ý ¿ä¼Ò Á¦°Å (°ýÈ£, ¼¼¹ÌÄÝ·Ð µî)   ¦¢
¦¢                                                            ¦¢
¦¢  ? ¼øÈ¸ ¿ëÀÌ¼º - Visitor ÆÐÅÏÀ¸·Î ½±°Ô Ã³¸®              ¦¢
¦¢                                                            ¦¢
¦¢  ? º¯È¯ °¡´É - ÃÖÀûÈ­, ÄÚµå »ý¼º µî ´Ù¾çÇÑ º¯È¯ Àû¿ë      ¦¢
¦¢                                                            ¦¢
¦¦¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¥
```

---

## ?? Âü°í ÀÚ·á

- [Crafting Interpreters](https://craftinginterpreters.com/) - Bob Nystrom
- [Engineering a Compiler](https://www.elsevier.com/books/engineering-a-compiler/cooper/978-0-12-815412-0) - Cooper & Torczon
- [Modern Compiler Implementation](https://www.cs.princeton.edu/~appel/modern/) - Andrew Appel
