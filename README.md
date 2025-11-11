## RAIN-FP

Current: Simple-typed lambda calculus with based type nat(*)

### Grammar

```
literal := DEC_INTEGER
identifier := IDENTIFIER

type := '*' ('->' type)* | '(' type ')' ('->' type)*
abstraction := '$' '(' identifier ':' type ')' '.' expression

application := '#' expression ('(' expression ')')*

induce_expression := '@' identifier '[' expression ',' abstraction ']'

prim_expression := literal | identifier | '(' expression ')' | induce_expression | application

succ_expression := ('++')* prim_expr

expression := succ_expression | abstraction

let_statement := 'let' identifier ':' type '=' expression ';'
```

`++ a` means `succ a`, `@x[y, z]` means `y` if `x = 0` and `z(w)` if `x = succ w`.
`*` stands for `nat`

`FIRST(expression) = {DEC_INTEGER, IDENTIFIER, '(', '++', '@', '$'}`