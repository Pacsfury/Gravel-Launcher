<img src="img/logo.png">

# Gravel
---
> This is temporary, as can change in any moment.

## Approach
**Gravel** is a **IN DEVELOPMENT** programming language. It's made to have a simple and neat syntax while having all low level power.

## Syntax
_Note: Syntax is really variable in this early-development, as this is **JUST PLANNING**_

### Packages
For using files as libraries, use packages, so you will need to use the package name instead of the path.

```
package: string
```

_If no package defined, couldn't be used for libraries. Naming the package isn't mandatory, but really recommended_

### Import
To import packages, use a tuple of a single string for importing them.

```
import "string"
```
or
```
import ("string", "stdio", "math", "rand")
```

### Variables
There are two ways of defining variables: explicitly and inferenced.

Explicit:
```
int name = value //or any other type (builtin: int, char, float)
```
Inferenced:
```
val name := value
```

### Namespaces
Create namespaces using `namespace name:` and use indentation. (separation: '.')

### If, while and for
Use indentation, and use the following syntax: `whatever cond:`. For `for`, use: `for i in list`, but classic syntax will also  be accepted `for int i=0; i<10; i++` or as wanted.

### Functions
Use indentation, and use the reserved word `fun`. Define return type after args (optional).

```
fun Main() char:
```

### Classes
As everywhere, use indents. if you put `-> type` at the end of `class name:`, you are saying that this class will act as a type, instead of being a constructor that only saves things in memory using another type.

With -> type:

```
class string: -> type
    extl char[] text
    impl int len
    fun __USE__():
        text = extl
        len = bytes(text)
```
Explanation: extl and impl are from type classes, and extl is the external value (in `string name = "hi"`, extl would be "hi"), and impl means implicit: can't be changed, but can be accessed by other than the class itself. It also means it's calculated by the class automatically.

The \_\_USE\_\_(): function is triggered when defined a value with the class type. This is ONLY for type classes.

### Pointers
Pointers use the default way for reference-dereference (& and *).
To create a pointer it's also the C way.

`int* name = &reference`
### Input and output
I will make a C lib called stdio, with, at least, two functions:

`std.out.print(char[])`

and

`std.in.ask(&ref, char[])`

_For a SINGLE character output I will add scho()_

## Status
Right now, this is the current development of every feature:
 
| Feature | Status |
|---------|--------|
|Tokenizer|FINISHED (can have updates)|
|AST      |FINISHED (can have updates)|
|Parser   |FINISHED (can have updates) |
|LLVM converter | IN (advanced) PROCESS |
|Variables, types and classes | IN (early) PROCESS |
|Functions, namespaces, if, while, etc | NOT STARTED |
|Packages, pointers, import and basic packages | NOT STARTED |

## Launcher
This is the Gravel _launcher_. This launcher goal is to provide basic CLI tools to run your Gravel code.

Use the following pipeline for executing a file.

```powershell
gravel run main.grv dependencies path space separated.
```
