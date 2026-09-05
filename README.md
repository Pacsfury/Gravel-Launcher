<img src="img/logo.png" alt="Gravel Logo">

# Gravel, A Programming Language
<sub>Version 0.3.0</sub>
## Table of Contents

- [Approach](#approach)
- [Syntax](#syntax)
- [Status](#status)
- [Launcher](#launcher)
- [Benchmarks](#benchmarks)
- [Flags](#flags)
- [Optimizations](#optimizations)
- [Update](#update)
- [Changelog](#changelog)
- [Website](https://github.com/Pacsfury/Gravel-Web)

---

## Approach

Gravel is an **experimental programming language** currently under active development. Its goal is to provide a clean and intuitive syntax while preserving the flexibility and power of low-level programming languages.

As the project is still in its early stages, features and syntax may evolve over time.

## Syntax

> **Note**
>
> Gravel is still in active development. The syntax shown below represents the current design and may change in future releases.

### Packages

For using files as libraries, use packages, so you will need to use the package name instead of the path.

```grv
package: string
```

_If no package defined, couldn't be used for libraries. Naming the package isn't mandatory, but really recommended_

### Import

To import packages, use a tuple or a single string for importing them.

```grv
import "string"
```

or

```grv
import ("string", "stdio", "math", "rand")
```

### Variables

There are two ways of defining variables: explicitly and inferenced.

Explicit:

```grv
int name = value //or any other type (builtin: int, char, float)
```

Inferred:

```grv
val name := value
```

You can now define constant variables (actually modificable now, tho) using `const name := val`

### Namespaces

Create namespaces using `namespace name` and use the `end` keyword. (separation: '.')

#### Virtual Namespaces

Instead of defining a lot of small namespaces like this:

```grv
namespace rounded_math
    val pi := 3
    val e := 2
end
```

You can write, getting the exact same effect:

```grv
val rounded_math.pi := 3
val rounded_math.e := 2
```

### If, while and for

Use the `end` keyword, and use the following syntax: `whatever cond`. For `for`, use: `for i in n`, which iterates `i` from `0` to `n-1`, but classic syntax will also be accepted `for int i=0; i<10; i++` or as wanted.

Traditional for-loop:

```grv
for int i=0; i<10; i++
    std.out.print("hello, but tenfold!")
end
```

Modern for-loop:

```grv
for i in 5
    std.out.print(i)
end
```

While loops:

```grv
val qux := 10

while qux < 10
    qux += 1
end
```

Conditionals:

```lua
if cond
    ...
elseif cond
    ...
else
    ...
end
```

### Repeat

Use this syntax:

```grv
repeat 10
    scho('a')
end
```

output:

```text
aaaaaaaaaa
```

### Functions

Use the `end` keyword, and use the reserved word `fun`. Define return type after args (optional).

```grv
fun Main() char
```

### Classes

As everywhere, use `end` to declare when the class ends. if you put `type` at the end of `class name:`, you are saying that this class will act as a type, instead of being a constructor for organization and creating objects.

With type:

```grv
class string: type
    extl char[] text
    impl int len
    fun __USE__()
        text = extl
        len = sizeof(text)
    end
end
```

Explanation: extl and impl are from type classes, and extl is the external value (in `string name = "hi"`, extl would be "hi"), and impl means implicit: can't be changed, but can be accessed by other than the class itself. It also means it's calculated by the class automatically.

The \_\_USE\_\_(): function is triggered when defined a value with the class type. This is ONLY for type classes.

### Pointers

Pointers use the default way for reference-dereference (& and *).
To create a pointer it's also the C way.

`int* name = &reference`

### Input and output

I will make a lib with at least these two functions: (_For single character output, use `scho`_)

`std.out.print(char[])`

and

`std.in.ask(&ref, char[])`

### Methods

Gravel features a handful of methods for primitives.
Integer methods are still a work in progress.

String Methods:

- upcase -> uppercase every character of a string
- downcase -> lowercase every character of a string
- split() -> separate each value in a string by the given operator. if none, split by spaces. Returns an array of each extracted value.
- bite -> removes the newline from std.in.ask
- to_integer -> turns a string like '1' into the integer 1.

Array Methods:

- prepend -> add a value to the front of an array and shift every other value
- push -> add a value to the end of an array
- erase/clear -> removes all values
- length/size -> the amount of values
- purity -> returns a boolean of true if all values are truthy; returns false if there is a falsy value.
- divide -> argue an integer; it will split the array at the index of the given integer:
`['foobar', 'baz', 'qux'].divide(2) // => ['foobar', 'baz']`
Note that it will discard every value after the given index.

Dictionary Methods:

- exists -> checks if the dictionary has a key matching the argument -- see 'alive' below.
- alive -> checks if the dictionary has a value matching the argument
- count -> how many keys a dictionary has
- brothers/siblings -> returns an integer of how many keys have a value that is truthy.
- family -> returns a boolean if any key is missing a value or if any key is falsy
- purify -> assigns undefined/undef to every value; preserves keys. Optionally, this could return an array of every removed value.

## Package and imports

Gravel has a little special way of how imports work.

Instead of relying on paths, every file defines how it wants to be called:
`package: name`. Then, for using the library, just do `import "name"`. For making it work, you have two options:

1. Libs.grvdep
    Create a file called `Libs.grvdep` and put, line by line, every path your program needs. Then, execute the main file:

    ```bash
    gravel run main.grv
    ```

    The system will detect the file and do all the work for you.

    You can use webs as library like this:

    ```grvdep
    web:https://raw.githubusercontent.com/Pacsfury/Gravel-Launcher/refs/heads/main/libs/math.grv
    ```

2. Directly in the terminal

    You can also do it like this:

    ```bash
    gravel run main.grv depen1.grv depen2.grv
    ```

## Custom opeations

Inside a type class, you can define custom operations like this:

```grv
class List: type
    op #
        return self.len //This is a placeholder for getting length
    end

    op {x}
        return self[x]
    end
end
```

Then, just do:

```grv
List list = new List
list.append(12) // Placeholder method
list# => 1
list{0} => 12
```

## Null and exception safety

You can declare a type nullable with `?`.

You can declare a type may be exception with `!`.

So, `int!?` could be an int, a null, or an exception.

Exceptions are thrown at the end of the scope.

## Compile

### With your default compiler

Compile `src/*.c`.

### With AutoPy

_Cl_:

```bash
autopy cl
```

_gcc_:

```bash
autopy gcc
```

_run Python tests_:

```bash
autopy test
```

## Status

Right now, this is the current development of every feature:

| Feature | Status |
|---------|--------|
|Tokenizer|Working|
|AST      |Working|
|Parser   |Working|
|LLVM converter |Working|
|Variables, types and classes | 2/3 |
|Functions, namespaces, if, while, for, repeat| Working |
|Packages, pointers, import and basic packages | 2/3 |
|Custom operations, null safety, exception safety | Not started |

To propose or vote on small syntax changes, please go to discussions.

## Run the .ll

To run the resulting `output.ll`, do the following:
**IF WINDOWS**: you can use:

```bash
./argc winll llvm\llvm.exe
```

but it will run automatically, so you can use this for executing othre output.ll.

**ELSE**: you can use: (_make sure to have llvmlite installed_)

```bash
./argc pyll llvm\llvm.py
```

it also runs automatically, but you need to do `pip install llvmlite`.

## Flags

- `-wE`: Shows various information, as time and token count. (only time used compiling to LLVM, not the LLVM execution itself)

## Optimization

- **Constant Folding**: Numerical operations including numbers (and future constant varibles) are done during compilation.
- **Namespace Flattening**: Namespace are flattened instead of saving complex tree structures.

## Update

### Currently available contents

- scho('A') / scho(var)
- int intvar = 65 / val intvar := 65
- namespace name ... end / name.getthis
- val namespace.gettheanother := 65
- Line comments `// comment` and block comments `/* comment */`
- If elseif else
- While and for loops (with comparison, increment/decrement and compound-assignment operators)
- Int args and int/void return functions
- Reassing varibles
- | & ^ ~
- == != < > <= >=, etc
- Import and packages
- Float type
- `Libs.grvdep`
- Char type (`i32` under the hood, interoperable with `int`)
