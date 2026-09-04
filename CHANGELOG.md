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
- Add methods for primitives ideas on readme (PR)

### 2026-07-21
- Implement comment blocks
- Bug fixes
- Add explicit int variables
- Update "Update" section
- Add if, elseif, else

### 2026-07-25
- Implement noarg & no return function definition
- These functions are now callable

### 2026-07-26
- Add DESIGN.md
- Fix spelling in readme and design (PR)
- Add VScode extension

### 2026-07-27
- Change syntax: no colons and _-> type_ to only _type_

### 2026-07-28
- Fix AST example at DESIGN.md

### 2026-07-30
- Functions can return ints now

### 2026-07-31
- Updated vscode extension
- Fixed libaries code
- Added CONTRIBUTING.md

### 2026-08-01
- Added modulo operation (PR)
- Names can contain '_'
- Fix ifs
- Change main.grv to a factorial function
- Add variable reassing
- Add and improve examples
- If reassigning an undefined variable, an error is thrown (PR)
- If file doesn't exist, error is thrown (PR)

### 2026-08-02
- Add int arguments
- Add tests
- Add LLVM IR execution

### 2026-08-03
- Add basic EBNF grammar explanation
- Implemented some commits from [skeeto's fork](https://github.com/skeeto/Gravel-Launcher):
  - **`b283d3c`**: Fix integer overflow when folding literals
  - **`a055ec6`**: Fix buffer overflow when names too long
  - **`a0b2132`**: Fix unclosed /* comments
  - **`4b1fc85`**: Fix null or empty dereference
- Fix Python test
- Bug fix
- Add sanitizers.yml workflow
- Fix possible memory error

### 2026-08-04
- Improve Python LLVM executor
- Use Python always to execute LLVM
- Fix sanitizer warnings
- Add `while` and `for` loops (classic `for int i=0; i<10; i++` and modern `for i in n`) (PR)
- Add comparison operators (`<`, `>`, `<=`, `>=`, `!=`) (PR)
- Add increment/decrement (`i++`, `i--`) and compound assignment (`+=`, `-=`, `*=`, `/=`, `%=`) operators (PR)
- Fix tokenizer never emitting `,`, enabling multi-argument functions and calls (PR)
- Update grammar documentation and add loop tests (PR)

### 2026-08-05
- Fix and improve tests
- Test and implement PR's for and while loop
- Divide time in COMPILE and TOTAL with flag `-wE`

### 2026-08-06
- Update documentation
- Add | & ^ and ~ (PR)
- Add a fuzzer for tests

### 2026-08-07
- Fix tests

### 2026-08-13
- Add a checker for:
  - SCHO
  - NAMESPACE
  - IF-ELSEIF-ELSE
  - WHILE
  - FOR
  - FUN

### 2026-08-20
- Add `float` type
- Add packages and import

### 2026-08-21
- Make `Cargo.grvdep` work
- Move changelog to a separated file
- Cargo.grvdep can have libraries hosted in the web
- Update extension to show icons at `.grvdep` files

### 2026-08-22
- Continue float type. Missing: overload symbols for `float`

### 2026-08-23
- Add compilation and testing available with AutoPy

### 2026-08-30
- Add codeQL and other security improvements
- Add SECURITY.md
- Optimize lookup with binary search (PR)

## 2026-09-01
- Change Cargo.grvdep to Libs.grvdep as Gravel doesn't use Cargo

## 2026-09-04
- Add a `char` type

TODO: Add checker, imports and packages to documentation
KNOWN BUGS: void functions return 0
