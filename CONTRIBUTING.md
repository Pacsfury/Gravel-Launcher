# Contributing to Gravel

First off, thank you for considering contributing to Gravel! It's open-source projects like this that make the developer community such a great place to learn, inspire, and create.

Gravel is an experimental programming language written in C that transpiles to LLVM IR. Whether you are fixing a bug, improving documentation, adding new syntax features, or writing tests, your help is warmly welcomed!

---

## Code of Conduct & AI Policy

- **Respect & Kindness:** Please ensure all interactions in issues and pull requests remain respectful and collaborative.
- **AI Policy:** Contributions generated using AI tools (LLMs, Copilot, etc.) are permitted **only if they follow [AI guidelines](./AI-POLICY.md)** else, your PR will be rejected.

---

## Setting Up the Development Environment

To build and contribute to Gravel, you need a C compiler and LLVM tools installed on your system.

### Prerequisites

- **C Compiler:** `clang` or `gcc` (C11 support recommended)
- **LLVM Tools:** `clang` (for compiling generated `.ll` files to executable binaries) you can also use [this website](https://godbolt.org/)
- **Git:** For version control

### Building Gravel Locally

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Pacsfury/Gravel-Launcher.git
   cd Gravel-Launcher
   ```

2. **Compile Gravel directly:**
   Depending on your platform and preferred compiler, compile all source files in `src/`:

   *Using Clang:*
   ```bash
   clang src/*.c -Iinclude -o gravel
   ```

   *Using GCC:*
   ```bash
   gcc src/*.c -Iinclude -o gravel
   ```

3. **Run a test Gravel file:**
   ```bash
   ./gravel run examples/02-HelloWorld.grv -wE
   ```

---

## How to Contribute

### 1. Pick an Issue or Feature
Check the **[GitHub Issues](https://github.com/Pacsfury/Gravel-Launcher/issues)** tab for tasks labeled:
- `good first issue`: Ideal for newcomers to the codebase.
- `help wanted`: Features or bug fixes requiring community assistance.
- `documentation`: Help improve the README, `DESIGN.md`, or guides.

> *If you want to work on a new feature not listed in issues, please open an issue first to discuss the design before writing code!*

### 2. Understand the Architecture
Before diving into code, please read our **[`DESIGN.md`](./DESIGN.md)**. It breaks down Gravel's internal pipeline:
```
Source Code (.grv) ➔ Tokenizer (tokens.c) ➔ AST (ast.c) ➔ CodeGen / LLVM IR (tollvm.c)
```

### 3. Workflow & Pull Requests

1. **Fork the repository** on GitHub.
2. **Create a feature branch:**
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make your changes** while adhering to the C coding conventions listed below.
4. **Test your code:** Ensure existing examples still compile and run properly (exceptuating the `libs` folder)
5. **Commit your changes:** Write clear, concise commit messages.
   ```bash
   git commit -m "feat(ast): add support for function argument parsing"
   ```
6. **Push to your fork & submit a PR:**
   - Describe what changed and why.
   - Link any related issue (e.g., `Fixes #12`).

### 4. Tests

After a major compiler change, please make sure that the language is working correctly using the tests folder:
```bash
python ./tests/tests.py
```
If some test fail, make sure the difference is small (gives same output when LLVM IR is executed). If so, **update the corresponding `.txt` file**. 

If the test fails giving a different output when executed, revise your code, as it's probably wrong.

---

## Coding Standards

- **Language:** Clean standard C. Avoid compiler-specific non-standard extensions.
- **Formatting:** Use 4 spaces for indentation. Keep functions focused and modular.
- **AST Additions:** If adding new syntax:
  1. Add token in `include/tokens.h` and lexing logic in `src/tokens.c`.
  2. Define AST node variant in `include/ast.h` and parser logic in `src/ast.c`.
  3. Emit LLVM IR logic in `src/tollvm.c`.
- **Memory Management:** Always check for `NULL` pointers and ensure allocated AST nodes are safely freed where applicable.

---

## Need Help?

If you have questions or get stuck while contributing, feel free to open a Q&A issue or ask directly on active Pull Request threads.
