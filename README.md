# Casket #

Bytecode interpreter written in C++. 

## EBNF ##
```EBNF
<source-code> ::= <declaration>* ;

<declaration> ::= ( <if-statement> | <while-statement> | <class-statement> ) | ( ( <goto-statement> | <label-statement> | <expression> ) ";" ) ;

<expression> ::= <operation> | <group> | <literal> | IDENTIFIER ;
<if-statement> ::= "if" "(" <expression> ")" <flexible-block> [ "else" <flexible-block> ] ;
<while-statement> ::= "while" "(" <expression> ")" <flexible-block> ;
<class-statement> ::= "class" IDENTIFIER "{" { IDENTIFIER [ ":" <expression> ] ";" } "}" ;
<goto-statement> ::= "goto" IDENTIFIER ;
<label-statement> ::= "label" IDENTIFIER ;

<flexible-block> ::= ( "{" <declaration>* "}" ) | <declaration> ;
<operation> ::= <prefix> | <infix> | ( <expression> "(" [ <expression> [ "," <expression> ]* ] ) ;
<group> ::= "(" <expression> ")" ;
<literal> ::= STRING | NUMBER | "true" | "false" | "null" | <anon-function> | <instance-creation> ;

<prefix> ::= ( "!" | "-" | "&" | "*" | ":" ) <expression> ;
<infix> ::= <expression> ( "+" | "-" | "*" | "/" | "%" | "=" | ">" | "<" | "==" | ">=" | "<=" | "&&" | "||" ) <expression> ;
<anon-function> ::= "fn" "(" [ IDENTIFIER [ "," IDENTIFIER ]* ] ")" "{" <declaration>* "}" ;
<instance-creation> ::= "inst" IDENTIFIER "(" [ IDENTIFIER "=" <expression> [ "," IDENTIFIER "=" <expression> ] ] ")" ;

```

## Example Code ##

```
class Example {
    msg: "default value"; // default
    to_string: fn() {
        return this.msg;
    };
    operator_add: fn(a) {
        return this.msg % a;
    };
}

set instance = inst Example(msg="Hello");

instance = instance + ", World!";

if (true) {
    print instance;
}
```