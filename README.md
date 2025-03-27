# project
**sora (そら)** is a framework for creating cross platform applications. 

> [!WARNING]
> this is a personal project created for learning purposes. it may contain incomplete features, experimental code, or unoptimized solutions. use at your own risk! contributions and feedback are welcome, but please note that this is not a production-ready project.

features:
- abstracted platform layers:
	- os platforms (win32, linux, etc.)
	- graphics api (d3d11, opengl, vulkan, etc.)
	- audio (wasapi, alsa, etc.)
	- font rasterizers (dwrite, freetype, etc. )
- math library:
	- vectors, matrices, quaternions.
	- simd support.
	- colors.
- 2d drawing library:
	- variety of shapes (rects, circles, lines, etc.)
	- blended textures and colors.
	- batched into single draw call.

# build

### windows:
use the `build.bat` script:
```
usage: build.bat [options] [source_file]

options:
    [build modes]:
        debug    - compile with debug symbols and sanitizers (default).
        release  - compile with optimizations.
 
    [compilers]:
        msvc     - use Microsoft Visual C++ compiler (default).
        clang    - use Clang compiler.
 
notes: 
     - argument order can be specified in any sequence.
     - source_file can be specified with or without a file extensions.
     - source_file defaults to "main.*"

examples:
    build.bat                          compile main.* with default settings.
    build.bat debug                    compile main.* in debug mode with default compiler.
    build.bat clang myfile             compile myfile with default build mode with Clang.
    build.bat release msvc             compile main.* in release mode with MSVC.
    build.bat myfile.cpp               compile myfile.cpp with default settings
    build.bat debug myfile.cpp clang   combine options in any order.
```
`build.bat` expects the main file to be in `\src` and outputs generated files to `\build`.

### linux:

use the `build.sh` script:
```
usage: build.sh [options] [source_file]

options:
    [build modes]:
        debug    - compile with debug symbols and sanitizers (default).
        release  - compile with optimizations.
 
    [compilers]:
        gcc      - use GNU Compiler Collection (default).
        clang    - use Clang compiler.
 
notes: 
     - argument order can be specified in any sequence.
     - source_file can be specified with or without a file extensions.
     - source_file defaults to "main.*"

examples:
    build.sh                          compile main.* with default settings.
    build.sh debug                    compile main.* in debug mode with default compiler.
    build.sh clang myfile             compile myfile with default build mode with Clang.
    build.sh release msvc             compile main.* in release mode with gcc.
    build.sh myfile.cpp               compile myfile.cpp with default settings
    build.sh debug myfile.cpp clang   combine options in any order.
```
`build.sh` expects the main file to be in `\src` and outputs generated files to `\build`.

# directory
- `\src`: contains all source code.
- `\templates`: contains template main files.
- `\build`: all build artifacts and generated binaries.

# structure
the codebase is set up in *layers*. some layers depend on other layers.
here is the current list of layers:

- `audio` (`audio_`): implements audio abstraction for different backends.
- `base` (none): includes basic types, strings, math, and helper macros.
- `draw` (`draw_`): implements a simple 2d shape renderer.
- `font` (`font_`): implements a font renderer and cache for different backends.
- `gfx` (`gfx_`): implements an abstraction over different graphics apis.
- `os` (`os_`): implements an abstraction over operating system features such as events, windows, threads, etc.

# templates

this codebase contains several templates to start off from.

- `template_console.cpp`: simple console program.
- `template_window.cpp`: opens a single window set up for rendering.
- `template_multi_window.cpp`: can open any number of windows set up for rendering.
- `template_custom_border.cpp`: opens a clean custom borderless window.