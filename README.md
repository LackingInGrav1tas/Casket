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
<literal> ::= STRING | NUMBER | LIST | "true" | "false" | "null" | <anon-function> | <instance-creation> ;

<prefix> ::= ( "!" | "-" | "&" | "*" | ":" ) <expression> ;
<infix> ::= <expression> ( "+" | "-" | "*" | "/" | "%" | "=" | ">" | "<" | "==" | ">=" | "<=" | "&&" | "||" | "|" | "&" | "^" ) <expression> ;
<anon-function> ::= "fn" "(" [ IDENTIFIER [ "," IDENTIFIER ]* ] ")" "{" <declaration>* "}" ;
<instance-creation> ::= "new" IDENTIFIER "(" [ IDENTIFIER "=" <expression> [ "," IDENTIFIER "=" <expression> ] ] ")" ;

```


## Data Types ##

### Literals ###
intiger: ```1```, ```2```, ```3```, etc.

float: ```1.0```, ```1.1```, etc.

byte: ```0b01010101```, etc.

str: ```"..."```

boolean: ```true```, ```false```

### Complex ###
list: ```[...]```

function: ```fn(...) {...}```

instance: ```new ClassName(field=value, ...)```

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

Stream.out.print(new Example(msg="Hello") + ", World!");
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

## Standard Library ##

### Stream ###


Handles input/output


#### in ####

```Stream.in.input(message)```: prints ```message```, returns input

```Stream.in.read(buffer)```: reads input to ```buffer```. ```buffer``` should be a pointer to a string variable.


#### out ####

```Stream.out.print(message)```: prints ```message``` to standard output

```Stream.out.println(message)```: prints ```message``` and flushes


#### file ####

```Stream.file.read(filename)```: reads and returns file at path ```filename```.

```Stream.file.write(filename, data)```: writes ```data``` to file at ```filename```.

```Stream.file.read_bytes(filename)```: reads and returns file at path ```filename``` as a list of bytes.

```Stream.file.write_bytes(filename, data)```: writes ```data```, which should be a list, to file at ```filename```.



### Environment ###

Interfaces with OS


```Environment.args()```: returns arguements given to program as list of strings

```Environment.command(cmd)```: runs ```cmd``` and returns it's output

```Environment.exit(val)```: exits program, returning ```val```

```Environment.casket(code)```: runs ```code```, a string, as if it were Casket code




### Debug ###

Gives information about the virtual machine


```Debug.print_scopes()```: prints scopes




### Types ###

Gives information about a value's type


```Types.prims```: Enum containing ```INT```, ```DOUBLE```, ```STRING```, ```BOOLEAN```, ```NULL```, ```BYTE```, ```POINTER```, ```LIST```, ```FUNCTION```, ```INSTANCE```


```Types.is(val, prim)```: returns true if ```val```'s type is ```prim```.

```Types.get_type(val)```: returns a ```Types.prims``` of ```val```'s type


## Example Programs ##



### Hello World ###

```Stream.out.print("Hello, World");```



### Stream Cipher: ###
```
set argv = Environment.args();
set pfile = argv[2];
set kfile = argv[3];
set ofile = argv[4];

set pdata = Stream.file.read_bytes(pfile);
set kdata = Stream.file.read_bytes(kfile);

set cdata = [];
for (set i = 0; i < pdata.size(); i++) {
    cdata.push(pdata[i] ^ kdata[i]);
}

Stream.file.write_bytes(ofile, cdata);
```