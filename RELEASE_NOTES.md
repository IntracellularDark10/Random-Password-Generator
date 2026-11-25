# v1.1.0

Patch release. It replaces the weak password seeding, stops the UI from freezing during generation, fixes the settings dialog, and removes the committed binaries.

## Security

- **Seeding.** Passwords came from `std::mt19937` seeded with one 32-bit `std::random_device` value. That allowed at most 2^32 distinct outputs for a given length and charset, and Mersenne Twister is not a cryptographic generator. Every character is now drawn from `std::random_device` directly.
- **Character sets.** A password could miss a selected set (for example, no digits with Digits checked). Each selected set now appears at least once, and the result is shuffled with Fisher-Yates.
- **Length limit.** Length was unbounded, and values like 1000000000 froze the window and used gigabytes of memory. The limit is now 4096.
- **Binaries.** `PasswordGenerator` and `password_generator_multithreaded` are removed from the repository because nobody could check them against the source. Build from source (see README).

## Bug fixes

- **UI freeze.** The multithreaded build called `join()` right after starting its thread, which blocked the UI thread. It now uses `QtConcurrent::run` with a `QFutureWatcher`, and the Generate button is disabled while a password is being generated.
- **Build.** Removed `#include "moc_multithreaded_main.cpp"` and the manual moc step. The classes had `Q_OBJECT` but no signals or slots, so no moc file was needed. A `CMakeLists.txt` replaces the README's gcc instructions.
- **Settings dialog.** Close and the window's X button now undo the live preview. The theme box starts on the active theme. Theme and accent color are saved with `QSettings` and restored on launch. A Reset button clears the accent color.
- **Error handling in `main.cpp`.** An error was returned as a string (`"Error: No character sets selected."`) and shown in the password box as if it were a password. Errors now go to a separate label and the copy button stays disabled.
- **Defaults.** The multithreaded build started with every checkbox cleared, so the first click always failed. Both programs now start with length 16 and all four sets checked.
- **Includes.** Added `<stdexcept>`, `<QStyle>`, `<QMenu>` and `<QAction>`. Removed the duplicate `<QPushButton>`, the unused `<QThread>`, `<vector>`, `<thread>` and `<future>`, and an unused exception variable.

## Added

- `password_generator.h`, one generator shared by both programs (they had separate copies with different error handling).
- Copy to Clipboard button, Enter key to generate, monospace output.
- "Exclude ambiguous characters" option (I, l, 1, O, 0).
- Unit tests (`ctest`): length, character set coverage, excluded characters, invalid input.
- GitHub Actions workflow that builds and runs the tests, and a `.gitignore`.

## Behavior changes

- The multithreaded build forces the Fusion style so the light and dark palettes work on every platform.
- Minimum length is now the number of selected character sets (4 with all sets checked).

## Not changed

- The password is not wiped from memory, and clipboard managers may keep a copy.
- `std::random_device` is deterministic on old MinGW-w64 (GCC older than 9.2).

## Testing

Built with GCC and Qt 6.4.2 on Ubuntu 24.04 with `-Wall -Wextra` and no warnings. Unit tests pass. Both programs were started under the offscreen Qt platform and driven by script: generate, invalid length, no sets selected, and the settings dialog (preview, Close reverts, Apply saves). Not tested on Windows or macOS.

## Contributors

IntracellularDark10, Claude (AI assistant made by Anthropic).
