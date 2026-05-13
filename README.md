# plat-lang IDE

A Qt 6 desktop IDE for the `platlang` language.

The IDE is still early, but already provides a usable workspace for opening a
project folder, editing `.plat` files, and running them through the configured
`platlang` compiler/interpreter.

The language and compiler live in the companion repository:

```text
https://github.com/ifilot/platlang
```

## Features

- Three-column IDE layout: file explorer, editor/terminal, code assistant
- Tabbed code editor with line numbers and platlang syntax highlighting
- Light and dark themes
- Integrated terminal for running `.plat` files
- File explorer rooted at an opened folder or CLI path argument
- FontAwesome-based file/folder icons
- Custom titlebar and application logo
- Basic Problems and Output panels
- Windows installer packaging through CPack/NSIS

## Requirements

- CMake 3.21+
- C++20 compiler
- Qt 6 with `Core`, `Gui`, `Network`, `Widgets`, `Svg`, and `LinguistTools`
- libarchive

On Ubuntu-like systems:

```sh
sudo apt-get install cmake g++ libarchive-dev ninja-build qt6-base-dev qt6-svg-dev qt6-tools-dev
```

On MSYS2 UCRT64:

```sh
pacman -S mingw-w64-ucrt-x86_64-libarchive mingw-w64-ucrt-x86_64-qt6-tools
```

## Build

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run the IDE:

```sh
./build/plat_lang_ide
```

Open a folder on startup:

```sh
./build/plat_lang_ide examples
```

## Compiler Toolchain

The IDE expects a `platlang` compiler/interpreter. During development it looks
for a sibling compiler build and copies it into the app data toolchain folder:

```text
../platlang/build/platlang
```

On Windows the executable name is expected to be `platlang.exe`.

Build or download the compiler from the companion `platlang` repository:

```text
https://github.com/ifilot/platlang
```

You can also configure a compiler manually from:

```text
Run -> Configure Interpreter...
```

## Packaging

CMake includes install and CPack metadata. On Windows, the GitHub Actions
workflow builds with MSYS2 and creates an NSIS installer.

For local install testing:

```sh
cmake --install build --prefix /tmp/plat-lang-ide-install
```

## GitHub Actions

The workflow in `.github/workflows/build.yml`:

- builds on Linux
- builds on Windows with MSYS2
- creates and smoke-tests a Windows installer
- uploads the installer as an artifact
- publishes the installer to a GitHub Release for tags like `v0.1.0`

## Current Limitations

- Problems panel exists, but diagnostics parsing/navigation is not implemented yet.
- Code assistant is a placeholder panel.
- Compiler update checks refresh only the local development compiler copy.
- Formatting currently performs basic whitespace cleanup only.

## License

This project is distributed under the GNU Lesser General Public License v3.

Bundled third-party assets include Space Mono and Font Awesome Free. See
`THIRD_PARTY_NOTICES.md` for details.
