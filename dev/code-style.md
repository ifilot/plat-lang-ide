# plat-lang C++ Code Style

The `plat-lang` implementation follows the Google C++ Style Guide as its base,
with the project-specific rules listed below taking precedence when they differ.

## Files

Use `.h` for C++ headers and `.cpp` for C++ source files.

File names use `lower_case_with_underscores`.

Examples:

```text
ast_node.h
ast_node.cpp
source_location.h
source_location.cpp
```

## Indentation

Use 4 spaces for indentation.

Do not use tab characters.

## Naming

Classes use `PascalCase`.

```cpp
class SourceLocation {
    ...
};
```

Functions use `lower_case_with_underscores`.

```cpp
void report_error();
```

Native Qt functions are exempt from this rule and should be used with their
original Qt naming. New functions introduced by this project should still use
`lower_case_with_underscores`. This distinction is intentional: it makes it easy
to tell native Qt API calls apart from project-owned functions.

Variables should follow the Google C++ Style Guide unless a more specific
project rule is introduced later.

## Class Layout

Classes are ordered as follows:

1. Private member variables
2. Public member functions
3. Private member functions

Example:

```cpp
class Parser {
private:
    std::string source_name_;

public:
    /** Parses the current source file and returns the resulting AST. */
    Program parse();

private:
    /** Parses a single statement from the token stream. */
    Stmt parse_statement();
};
```

## Documentation

All classes and functions must have clear Doxygen-style comments.

Class comments should explain the role of the class in the implementation.
Function comments should explain what the function does, its important inputs,
and any meaningful error behavior.

Example:

```cpp
/**
 * Represents a source position used for parser and runtime diagnostics.
 */
class SourceLocation {
private:
    int line_;
    int column_;

public:
    /**
     * Creates a source location.
     *
     * @param line One-based line number.
     * @param column One-based column number.
     */
    SourceLocation(int line, int column);
};
```

Prefer useful comments over mechanical repetition. A Doxygen comment should make
the surrounding code easier to understand, not simply restate the declaration.
