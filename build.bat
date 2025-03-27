:: build.bat
::
:: usage: build.bat [options] [source_file]
::
:: options:
::     [build modes]:
::         debug    - compile with debug symbols and sanitizers (default).
::         release  - compile with optimizations.
:: 
::     [compilers]:
::         msvc     - use Microsoft Visual C++ compiler (default).
::         clang    - use Clang compiler.
:: 
:: notes: 
::     - argument order can be specified in any sequence.
::     - source_file can be specified with or without a file extensions.
::     - source_file defaults to "main.*"
::
:: examples:
::     build.bat                          compile main with default settings.
::     build.bat debug                    compile main in debug mode with default compiler.
::     build.bat clang myfile             compile myfile with default build mode with Clang.
::     build.bat release msvc             compile main in release mode with MSVC.
::     build.bat myfile.cpp               compile myfile.cpp with default settings
::     build.bat debug myfile.cpp clang   combine options in any order.
::

@echo off
setlocal enabledelayedexpansion

:: define options
set "build_modes=debug release"
set "compilers=msvc clang"
set "src_dir=src"
set "src_exts=.c .cpp .h"

:: msvc options
set "msvc_common_compile_flags=/nologo /FC /W0"
set "msvc_common_link_flags=/link /MANIFEST:EMBED /INCREMENTAL:NO"
set "msvc_debug_compile_flags=/DBUILD_DEBUG=1 /Zi /Od /fsanitize=address"
set "msvc_debug_link_flags=/SUBSYSTEM:CONSOLE"
set "msvc_release_compile_flags=/DBUILD_RELEASE=1 /O2 /Zi"
set "msvc_release_link_flags=/SUBSYSTEM:WINDOWS"

:: clang options
set "clang_common_compile_flags=-Wno-everything"
set "clang_common_link_flags=-fuse-ld=lld -Xlinker /MANIFEST:EMBED"
set "clang_debug_compile_flags=-DBUILD_DEBUG=1 -g -O0 -fsanitize=address"
set "clang_debug_link_flags=-Xlinker /SUBSYSTEM:CONSOLE"
set "clang_release_compile_flags=-DBUILD_RELEASE=1 -O2"
set "clang_release_link_flags=-Xlinker /SUBSYSTEM:WINDOWS"

:: defaults
set "build_mode=debug"
set "compiler=msvc"
set "src_file=main"

:: get arguments
for %%a in (%*) do (
    set "matched=0"

    :: check for build mode options
    for %%b in (%build_modes%) do (
        if /I "%%a"=="%%b" (
            set "build_mode=%%b"
            set "matched=1"
        )
    )

    :: check for compiler options
    for %%c in (%compilers%) do (
        if /I "%%a"=="%%c" (
            set "compiler=%%c"
            set "matched=1"
        )
    )

    :: set source file
    if !matched!==0 (
        set "src_file=%%~na"

        :: get extension
        for %%l in ("%%a") do (
            set "src_file_ext=%%~xl"
        )
    )
)

:: check for src_file in src_dir
set "found_file="
for %%f in ("%src_dir%\%src_file%*.*") do (
    
    set "file_name=%%~nf"
    
    if /i "!file_name!"=="%src_file%" (
        set "found_file=%src_file%"
        for %%l in ("%%f") do (
            set "src_file_ext=%%~xl"
        )
    )
)

if defined found_file (
    set "src_file=%found_file%"
) else (
    echo [error] could not find "%src_file%" in "/%src_dir%".
    exit /b 1
)

:: check file extension
if defined src_file_ext (

    :: check if extension matches src_exts
    set "ext_check=0"
    for %%e in (%src_exts%) do (
        if /I "%src_file_ext%"=="%%e" set "ext_check=1"
    )

    :: source file does not match src_exts
    if !ext_check!==0 (
        echo [error] "%src_file%" is an unsupported file extension. 
        exit /b 1
    )
)

echo [build_mode: %build_mode%]
echo [compiler: %compiler%]
echo [src_file: %src_dir%\%src_file%%src_file_ext%]

:: create build dir if needed
if not exist "build\" (
	mkdir build
)

:: set compile and link flags

set "common_compile_flags_var=%compiler%_common_compile_flags"
set "build_compile_flags_var=%compiler%_%build_mode%_compile_flags"
set "compile_flags=!%common_compile_flags_var%! !%build_compile_flags_var%!"

set "common_link_flags_var=%compiler%_common_link_flags"
set "build_link_flags_var=%compiler%_%build_mode%_link_flags"
set "linker_flags=!%common_link_flags_var%! !%build_link_flags_var%!"

:: compile 
pushd build

if "%compiler%"=="msvc" call cl %compile_flags% ..\%src_dir%\%src_file%%src_file_ext% %linker_flags%
if "%compiler%"=="clang" call clang %compile_flags% ..\%src_dir%\%src_file%%src_file_ext% %linker_flags%

popd

:: check for errors
if %errorlevel% neq 0 (
	echo [error] build failed.
	exit /b %errorlevel%
)

:: if no error copy file 
copy build\%src_file%.exe %src_file%.exe >nul
echo [info] build successful.
