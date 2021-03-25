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

## Class Built-in Functions ##
```to_string()```:
Returns a string version, used in ```print```.

```index(i)```:
Used with ```<expr>[...]```

```operator_add(rhs)```, ```operator_subtract(rhs)```, ```operator_multiply(rhs)```, ```operator_divide(rhs)```:
Used with ```+```, ```-```, ```*```, ```/``` when the instance is on the left-hand side.

```increment()```, ```decrement()```:
Used with ```++``` and ```--``` respectively.

## Data Types ##

### Literals ###
intiger: ```1```, ```2```, ```3```, etc.

float: ```1.0```, ```1.1```, etc.

str: ```"..."```

boolean: ```true```, ```false```

### Complex ###
list: ```[...]```

function: ```fn(...) {...}```

instance: ```inst ClassName(...)```

## Example Code ##

### Classes ###

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

print inst Example(msg="Hello") + ", World!";
```
```
Hello, World!
```

### Pointers ###

```
set a = 1;
set b = &a;
*b = *b + 1;
print a;
```
```
2
```
