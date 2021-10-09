# Casket #

Bytecode interpreter written in C++. Slow and inefficient, made for fun.

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


## Data Types ##

### Literals ###
intiger: ```1```, ```2```, ```3```, etc.

float: ```1.0```, ```1.1```, etc.

str: ```"..."```

boolean: ```true```, ```false```

### Complex ###
list: ```[...]```

function: ```fn(...) {...}```

instance: ```inst ClassName(field=..., ...)```

## Example Code ##

### Classes ###

```
class Example {
    msg: "default value"; // default
    to_string: fn() {
        return this.msg;
    };
    operator +: fn(a) {
        return this.msg % a;
    };
}

print inst Example(msg="Hello") + ", World!";
```

Class functions are fed one (or two) variables automatically:
```this```: pointer to object
```left```: only for operator overloads, true if the class is one the left of the operation

Pretty much every operator can be overloaded by a class, using the syntax ```operator <op>: fn...```. ```op``` is just the operator, with the exception of prefix - being ```prefix_negate```. 

### Control Flow ###
```
if (true) {
    "some expression";
    "another one";
} else "brackets aren't needed for one-line blocks";
```
```
while (true) {
    "same applies";
}
```
```
for (set i = 0; i < 5; i++)
    "loops 5 times";
```
```
goto skipping;
"gets skipped";
label skipping;
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
