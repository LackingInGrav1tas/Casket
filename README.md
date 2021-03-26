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


### Iterating Lists ###
```
set a = [1, 2, 3];
for (set it = &a[0]; *it != a; it++)
    print *it, "\n";
```

This method works because of how lists are stored in memory: ```..., ITEM1, ITEM2, ITEM3, LIST_OBJ, ... ```

By first setting ```it``` to ```&a[0]```, incrementing ```it``` moves it through the list until eventually ```*it == a```, meaning that ```it``` points to ```LIST_OBJ``` and has iterated through the list.
