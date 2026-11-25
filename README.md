# Random-Password-Generator

Qt desktop password generator with adjustable length and character sets. The build produces two programs:

- `PasswordGenerator`: a single window.
- `password_generator_multithreaded`: the same window plus a Settings menu (light or dark theme, accent color). Generation runs off the UI thread.

## Features

- Length from 1 to 4096
- Upper-case, lower-case, digits and special characters, each optional
- Every selected set appears at least once in the result
- Option to exclude ambiguous characters (I, l, 1, O, 0)
- Copy to Clipboard button

## Building

Requires a C++17 compiler, CMake 3.16 or newer, and Qt 6 with the Widgets and Concurrent modules (tested with Qt 6.4.2).

```
sudo apt install build-essential cmake qt6-base-dev
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The programs end up in `build/`.

## Security notes

- Characters are drawn from `std::random_device` directly. Old MinGW-w64 releases (GCC older than 9.2) have a deterministic `std::random_device`, so build on Linux, macOS, MSVC or a current MinGW-w64.
- Copy to Clipboard leaves the password in the clipboard. Clipboard managers can keep a copy.
- The password is not wiped from memory after use.

## P.S

This code was written fully by LLMs, e.g. ChatGPT, GitHub Copilot and DeepSeek. The v1.1.0 fixes were written by Claude. See `CONTRIBUTORS.md`.

## License

BSD 2-Clause, see `LICENSE`.
