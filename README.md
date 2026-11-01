# NumWorks C Interpreter

This repository contains a portable C interpreter/runtime core and a NumWorks
Epsilon frontend:

- **Epsilon**: an EADK/NWA application boundary.
- **Upsilon**: a planned external-application boundary.

The execution core is based on the original
[PicoC](https://github.com/jpoirier/picoc) project, vendored under
`third_party/picoc/`. PicoC supplies the lexer, parser, type system, embedded
C libraries, and interpreter. It is adapted to NumWorks with a project-specific
platform bridge; the runtime wrapper, editor, keyboard handling, persistent
storage, and Epsilon UI are project code.

The current application is an embedded **C interpreter**. It parses and
executes C source directly on the calculator; it does not generate native ARM
code and does not support C++.

```text
NumWorks Epsilon UI
        |
        v
NumWorks/PicoC platform bridge
        |
        v
PicoC interpreter and embedded C libraries
        |
        v
On-device C execution
```

## Project status and roadmap

1. **Portable PicoC interpreter integration (implemented)**
   - Vendored PicoC lexer, parser, type system, C library, and interpreter.
   - C features including functions, arrays, pointers, structs, enums,
     typedefs, loops, switch statements, preprocessing, strings, formatted
     output, and selected standard-library headers.
   - A small platform adapter captures diagnostics and program output for the
     host and calculator UI, and connects `scanf` to the calculator keyboard.
2. **Epsilon editor and application UI (implemented)**
   - EADK editor with a script selector, source buffer, diagnostics, execution
     screen, and interactive input screen.
   - Horizontal and vertical cursor movement, held-arrow repeat, modifier
     handling, and NumWorks keyboard mappings.
   - The app opens on the script selector. The launch key is released before
     selector input is accepted, preventing an immediate accidental open.
   - The `.nwa` is built with `nwlink` and remains below the ordinary
     external-app budget.
3. **Persistent script storage (implemented)**
   - Scripts are stored through the Nwagyu/NumWorks Extapp Storage API.
   - Storage parsing validates addresses and record sizes and fails closed on
     unavailable or malformed storage.
4. **Upsilon release (planned)**
   - Connect the same core to the Upsilon external-app API/archive format.
   - Add persistent project storage and a larger standard library where the
     target firmware permits it.
5. **Language expansion**
   - Add a desktop compiler/import workflow and expand PicoC compatibility.
   - Native ARM code generation would require a separate compiler backend.
6. **Device validation**
   - Measure `.nwa` size, peak RAM, compile time, and executable size on each
     supported calculator model.

## Build

The host build requires CMake 3.20 and a C++17 compiler:

```sh
make
make test
./build/numworkscc examples/basic.nwc
```

CMake is also supported when available:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The Upsilon target remains a planned adapter boundary. The Epsilon target
contains the current device application and produces a real `.nwa` package.
The device SDK is supplied by `nwlink` and is not vendored here.

## Epsilon NWA application

With the ARM toolchain and `nwlink` installed, build the first device smoke
test from the repository root:

```sh
cd epsilon
make clean
make
ls -lh build/numworkscc.nwa
```

The app opens on the script selector and starts with one built-in script:

```c
int a = 2 + 3 * 4;
printf(a);
```

Use the calculator keyboard to type digits and operators directly. Press
**ALPHA**, then press a letter key. This follows Upsilon's
`Ion::Events` modifier state machine: one Alpha press selects lowercase, two
Alpha presses enable Alpha-lock, Shift+Alpha selects uppercase, and Shift
toggles independently. The mapping is the same as Upsilon's Python app:
`e^x=A`, `ln=B`, `log=C`, `i=D`, `,=E`, `sto→=F`; `sin=G`, `cos=H`,
`tan=I`, `π=J`, `√=K`, `x²=L`; `7–9=M–O`, `(=P`, `)=Q`; `4–6=R–T`,
`×=U`, `÷=V`; and `1–3=W–Y`, `+=Z`. Alpha also provides `:` `;` `"` `%`,
space, `?`, and `!` on the same special keys as Upsilon. Shift provides
brackets, braces, underscore, `asin(`, `acos(`, `atan(`, `=`, `<`, and `>`.
The cursor is drawn as a graphical overlay aligned to the source text, so it
does not change the source while moving. **Left/Right/Up/Down** move it,
holding an arrow repeats movement after a short delay, **Backspace** erases,
**ALPHA+EXE** inserts `&`, **OK** compiles and runs, **BACK** returns to the
script list from the editor or returns from the result screen, and **HOME**
exits. In the script list, **Up/Down** selects a script, **OK** opens it,
**+** creates a blank script, and **Backspace** deletes the selected script.
Script creation is edge-triggered, so one held `+` press creates only one
script. Alpha-modified keys are handled before editing actions, so
**ALPHA+Backspace** inserts `%` without deleting the character before the
cursor.
Scripts are persisted as `nwcc-0.c` through `nwcc-4.c` using the
[NumWorks Extapp Storage](https://framagit.org/Yaya.Cout/numworks-extapp-storage)
implementation used by Nwagyu. Changes are saved as they are edited, and
scripts are restored when the app is reopened. If storage is unavailable, the
app falls back to the built-in default script for that session.
The interpreter is PicoC rather than the former project-specific expression
runtime. For example:

```c
int main(void) {
  int i = 0;
  while (i < 10) {
    if (i % 2 == 0) printf("%d\n", i);
    i = i + 1;
  }
  return 0;
}
```

It validates and interprets the C source directly on the calculator. It is not
a native ARM code generator, and C++ is not supported. The resulting
application package is currently around 305 KB, well below the normal
external-app budget.

Supported `#include` headers are `ctype.h`, `errno.h`, `math.h`, `stdbool.h`,
`stdio.h`, `stdlib.h`, `string.h`, `time.h`, and `unistd.h`. They are PicoC's
embedded standard-library implementations; arbitrary files cannot be included
from calculator storage.

The application currently supports up to five scripts. The visible names are
`main.c`, `script2.c`, `script3.c`, `script4.c`, and `script5.c`; source is
stored under the internal names `nwcc-0.c` through `nwcc-4.c`.

The host regression suite also exercises advanced PicoC features including
switch statements, structs, enums, pointers, heap allocation, strings,
`sscanf`, and math functions. `scanf` now uses the runtime input callback:
on-device it opens an input screen only when execution reaches `scanf`; output
already produced by the program remains visible above the input prompt. A line
can be entered with the calculator keyboard and submitted with **EXE**.
**BACK** or **HOME** cancels input and causes `scanf` to return `EOF`.

To install it directly while the calculator is on the connected screen:

```sh
cd epsilon
make run
```

## Host simulator smoke test

The installed `nwlink` package does not provide a graphical simulator binary,
but the project includes a headless EADK simulator harness. It feeds scripted
keyboard events and logs rendered text, which is useful for checking UI flows
without a calculator:

```sh
make
cd epsilon
make simulator
```

The smoke test verifies opening the selector, entering the editor, and
inserting `&` with **ALPHA+EXE**. It also exercises the selector/editor
rendering path. The harness is intentionally headless; it does not emulate
the calculator's physical display, persistent firmware storage, or USB
transport. Visual layout and real storage behavior still need calculator
testing.

To upload the package manually, open
[my.numworks.com/apps](https://my.numworks.com/apps) in a Chromium-based
browser and select `epsilon/build/numworkscc.nwa`. The calculator must be
connected over USB and visible to the browser.

## Known limitations

- PicoC interprets C source on-device; this is not a native ARM compiler.
- C++ is not supported.
- The editor source buffer is limited to 512 bytes.
- The app supports five scripts and fixed script names; script renaming is not
  implemented.
- `#include` is limited to the embedded PicoC headers listed above.
- Storage depends on the calculator's userland layout and supported model
  detection. If storage cannot be safely identified, the app continues with
  the built-in script for that session rather than risking a storage write.

The app may disappear after a calculator reset, as with the normal Epsilon
external-app workflow. Rebuild and reinstall it when necessary.

## Main implementation areas

- `third_party/picoc/`: vendored PicoC interpreter and embedded C libraries.
- `src/compiler.cpp` and `src/runtime.cpp`: host/device compile and execution
  bridge, output capture, and interactive input callback.
- `epsilon/src/main.cpp`: EADK selector, editor, keyboard mapping, cursor,
  execution UI, and persistent-script integration.
- `epsilon/src/storage.c` and `epsilon/src/storage.h`: Nwagyu-compatible
  calculator storage access.
- `epsilon/src/syscalls.cpp`: embedded syscall compatibility, including the
  filesystem-link stubs required by `nwlink`.
- `epsilon/src/simulator_stubs.cpp`: headless keyboard, display, and timing
  stubs used by the simulator target.
# numworks-C-compiler
PicoC port as an NWA app for Numworks 
