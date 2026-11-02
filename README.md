# NumWorks C Interpreter

A lightweight C interpreter for NumWorks calculators, based on [PicoC](https://github.com/jpoirier/picoc).

The project brings an interactive C programming environment to NumWorks, allowing users to write, edit, save, and execute C programs directly on their calculator.

**This is an interpreter, not a native compiler:** C source code is parsed and executed on-device. C++ and native ARM code generation are not supported.

**The app makes the calculator crash the first time you open it, (whatever, I couldn't fix it), so I'd recommend backing up important files.**

## /!\ VIBE SLOPPED ALERT /!\
**This README is entirely AI generated, and AI helped me a lot while making this project, particularly for small shitty bugs in the app or things I couldn't get over (it's not entirely vibecoded tho)**

My current goal is to make this project less and less AI generated. Currently, the estimated percentage of AI-written code in this project is 30–40%.

## Features

- **C interpreter:** functions, arrays, pointers, structs, enums, loops, switch statements, and more.
- **Code editor:** on-device source editor with NumWorks keyboard support.
- **Interactive input:** supports `scanf()` through the calculator keyboard.
- **Persistent storage:** scripts are saved and restored between application sessions.
- **Multiple scripts:** manage up to five programs directly on the calculator.
- **Standard library:** selected PicoC implementations of common C headers.

## To-Do List
(Things that would be nice to have but I'm not talented enough for)

- Epsilon-like theme (this one is achieveable I think)

- The ability to send and exchange files, just like python scripts

- A toolbox with all usable libraries

- Syntax highlighting

## Architecture

The interpreter is based on the original PicoC project, adapted through a platform-specific runtime.

```text
NumWorks Epsilon interface
          |
          v
   Platform adapter
          |
          v
    PicoC interpreter
          |
          v
   C program execution
```

The codebase is designed to support multiple application environments:

- **Epsilon:** current implementation, distributed as an `.nwa` application.
- **Upsilon:** planned support through a separate application adapter.

## Build

### Requirements

- CMake 3.20 or newer
- C++17 compiler
- Python dependencies required by the PicoC integration, if applicable

### Desktop

From the repository root:

```sh
make
make test
```

To execute a program:

```sh
./build/numworkscc examples/basic.nwc
```

Alternatively, using CMake:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### NumWorks (Epsilon)

Requires the ARM toolchain and `nwlink`.

```sh
cd epsilon
make
```

The resulting application is:

```text
epsilon/build/numworkscc.nwa
```

To install it directly on a connected calculator:

```sh
make run
```

Alternatively, upload the `.nwa` file through [my.numworks.com/apps](https://my.numworks.com/apps).

## Limitations

- C is interpreted directly on the calculator; programs are not compiled into native ARM code.
- C++ is not supported.
- The editor has a 512-byte source buffer.
- A maximum of five scripts can be stored.
- Only selected standard-library headers are available.
- Persistent storage depends on the calculator model and firmware environment.

## Repository Structure

| Directory / File | Description |
|---|---|
| `third_party/picoc/` | Original PicoC interpreter and libraries |
| `src/` | Runtime and interpreter integration |
| `epsilon/` | NumWorks Epsilon application |
| `epsilon/src/storage.c` | Persistent script storage |
| `examples/` | Example C programs |


## License

PicoC is included as a vendored dependency. Refer to its original repository for its licensing terms. Check the project-specific source files for their respective licenses.

## Third-party components

- `epsilon/src/storage.c` and `epsilon/src/storage.h`:
  From [NumWorks Extapp Storage](https://framagit.org/Yaya.Cout/numworks-extapp-storage)
  by Yaya-Cout, licensed under the MIT License.