<img src="img/logo.png">

# Gravel, A Programming Language
<sub>Alpha 0.0.4</sub>
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
Inferred:
```
val name := value
```

You can now define constant variables (as normal ones can't be modified, at the moment, there is no difference at all, tho) using `const name := val`

### Namespaces
Create namespaces using `namespace name` and use the `end` keyword. (separation: '.')

#### Virtual Namespaces
Instead of defining a lot of small namespaces like this:
```
namespace rounded_math
    val pi := 3
    val e := 2
end
```
You can write, getting the exact same effect:
```
val rounded_math.pi := 3
val rounded_math.e := 2
```

### If, while and for
Use the `end` keyword, and use the following syntax: `whatever cond:`. For `for`, use: `for i in list`, but classic syntax will also  be accepted `for int i=0; i<10; i++` or as wanted.

### Repeat
Use this syntax:
```
repeat 10
    scho('a')
end
```
output:
```
aaaaaaaaaa
```

### Functions
Use the `end` keyword, and use the reserved word `fun`. Define return type after args (optional).

```
fun Main() char:
```

### Classes
As everywhere, use `end` to declare when the class ends. if you put `-> type` at the end of `class name:`, you are saying that this class will act as a type, instead of being a constructor that only saves things in memory using another type.

With -> type:

```
class string: -> type
    extl char[] text
    impl int len
    fun __USE__():
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

## Status
Right now, this is the current development of every feature:
 
| Feature | Status |
|---------|--------|
|Tokenizer|Working|
|AST      |Working|
|Parser   |Working|
|LLVM converter |Working|
|Variables, types and classes | 1/3 |
|Functions, namespaces, if, while, repeat, etc | 2/6 |
|Packages, pointers, import and basic packages | NOT STARTED |

## Launcher
This is the Gravel _launcher_. This launcher goal is to provide basic CLI tools to run your Gravel code.

Use the following pipeline for executing a file.

```powershell
gravel run main.grv dependencies path space separated.
```
(Maybe we will add a file for tracking dependencies, like Cargo.toml but for Gravel)

## Benchmarks
**New version of the benchmark: it now repeats 30 times!**
This repo includes a `bench.grv` file with 33576 tokens. You can execute it to test the speed. I got 0.379000 s, let's see what you get running `./main run main.grv -wE` (change `./main` for the actual executable).

> This time includes only:
> * Detection of argument "run" and getting the path
> * Opening the file and compiling it
> * Write the `.ll` file

## Flags
* `-wE`: Shows various information, as time and token count. (only time used compiling to LLVM, not the LLVM execution itself)

## Optimization
* **Constant Folding**: Numerical operations including numbers (and future constant varibles) are done during compilation.
* **Namespace Flattening**: Namespace are flattened instead of saving complex tree structures.

## Update
Current compiler status:
* Can compile files
* Can do `scho`
* Can declare and use `namespace`
* Can define int variables (with inference) and be used

## Changelog
<sub>The changelog idea is from [BeknYTprogamador](https://github.com/BeknYTprogamador)</sub>

### 2026-06-12
- Project initialization and first commit
- Started development on the compiler (Day 1: Tokenizer)
- Added initial README file with small fixes

### 2026-06-13
- Continued tokenizer development
- Removed accidental files
- Updated README with new `gravel` command usage details
- Clarified `extl` and `impl` definitions in the README

### 2026-06-14
- Finished tokenizer development and started AST (Abstract Syntax Tree)

### 2026-06-15
- Finished AST development and started the parser
- Updated project status section in the README

### 2026-06-16
- Prepared AST and parser for bug fixes
- Fixed bugs, debugged, and added `clang.exe` for future use

### 2026-06-17
- Fixed various bugs

### 2026-06-18
- Started LLVM IR translation and noted initial bugs

### 2026-06-20
- Started mapping variables to LLVM IR

### 2026-06-21
- Added `libs` folder featuring standard packages written in Gravel
- Implemented single-character output experimentation in LLVM
- Continued LLVM IR transpilation work and resolved active bugs
- Updated the README

### 2026-06-23
- Codebase fixes and general maintenance

### 2026-06-25
- Code debugging and system fixes
- Fixed the random library implementation

### 2026-06-26
- Fixed spelling errors and enhanced terminology clarity in the README

### 2026-06-28
- Implemented LLVM transpilation updates and performed debugging

### 2026-06-30
- Removed forced indentation requirements
- Added notes to the README regarding the future Gravel dependency tracking file

### 2026-07-01
- Released the first working version of the project
- Updated project status and removed temporary notes from the README

### 2026-07-02
- Added compilation details to the update section of the README

### 2026-07-04
- Achieved successful file compilation and executed the first "Hello, World!"
- Updated the README with bug notes and future development plans
- General code fixes

### 2026-07-05
- Added error management logic and variable inference capabilities for `INT`
- General code tweaks and performance optimization
- Updated project status in the README
- Deleted the temporary `main.grv` file

### 2026-07-06
- Added full support and implementation for Namespaces
- Updated the README with the new namespace syntax and declaration capabilities
- Revised progress status in the README
- Prepared infrastructure for Error Explaining 2.0

### 2026-07-07
- Updated README with changelog

### 2026-07-10
- Optimized with constant folding
- Added new flag (`-wE`) for showing time spent compiling

### 2026-07-11
- Added token count when using `-wE`

### 2026-07-12
- Updated libs with new syntax
- Added `bench.grv` to test its speed
- Added Actions for Windows, Ubuntu, MacOS and FreeBSD

### 2026-07-13
- Add `repeat` functionality

### 2026-07-14
- Add constant definition

### 2026-07-19
- Upgrade README (PR)
- Comment support (PR)

### 2026-07-20
- Create examples folder (PR)
- Add newline (`\n`) support
- Add AI-POLICY.md
