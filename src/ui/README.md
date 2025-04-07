# project
**simui** is an immediate mode ui implemented using the [sora](https://github.com/michaelckey/sora) framework. 

> [!WARNING]
> this is a personal project created for learning purposes. it may contain incomplete features, experimental code, or unoptimized solutions. use at your own risk! contributions and feedback are welcome, but please note that this is not a production-ready project.

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
     - source_file defaults to "ui_testing.cpp"

examples:
    build.bat                          compile ui_testing.cpp with default settings.
    build.bat debug                    compile ui_testing.cpp in debug mode with default compiler.
    build.bat clang myfile             compile myfile with default build mode with Clang.
    build.bat release msvc             compile ui_testing.cpp in release mode with MSVC.
    build.bat myfile.cpp               compile myfile.cpp with default settings
    build.bat debug myfile.cpp clang   combine options in any order.
```

### linux:

not implemented yet.

## screenshots

