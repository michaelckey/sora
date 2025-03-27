#!/bin/bash


# build.bat
#
# usage: build.bat [options] [source_file]
#
# options:
#     [build modes]:
#         debug    - compile with debug symbols and sanitizers (default).
#         release  - compile with optimizations.
# 
#     [compilers]:
#         gcc     - use GNU Compiler Collection (default).
#         clang    - use Clang compiler.
# 
# notes: 
#     - argument order can be specified in any sequence.
#     - source_file can be specified with or without a file extensions.
#     - source_file defaults to "main.*"
#
# examples:
#     build.bat                          compile main with default settings.
#     build.bat debug                    compile main in debug mode with default compiler.
#     build.bat clang myfile             compile myfile with default build mode with Clang.
#     build.bat release gcc              compile main in release mode with gcc.
#     build.bat myfile.cpp               compile myfile.cpp with default settings
#     build.bat debug myfile.cpp clang   combine options in any order.
#

set -o nounset
set -o errexit

# define options
build_modes="debug release"
compilers="gcc clang"
src_dir="src"
src_exts=".c .cpp .h"

# gcc options
gcc_common_compile_flags="-Wall -Wextra"
gcc_common_link_flags=""
gcc_debug_compile_flags="-DBUILD_DEBUG=1 -g -O0 -fsanitize=address"
gcc_debug_link_flags="-fsanitize=address"
gcc_release_compile_flags="-DBUILD_RELEASE=1 -O2"
gcc_release_link_flags=""

# clang options
clang_common_compile_flags="-Wno-everything"
clang_common_link_flags=""
clang_debug_compile_flags="-DBUILD_DEBUG=1 -g -O0 -fsanitize=address"
clang_debug_link_flags="-fsanitize=address"
clang_release_compile_flags="-DBUILD_RELEASE=1 -O2"
clang_release_link_flags=""

# defaults
build_mode="debug"
compiler="gcc"
src_file="main"

# get arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        
        # check for build mode options
        debug|release)
            build_mode="$1"
            ;;
        
        # check for compiler options
        gcc|clang)
            compiler="$1"
            ;;
        
        # set source file
        *)
            src_file=$(basename "$1" | cut -d. -f1)
            src_file_ext=$(basename "$1" | cut -d. -f2-)
            ;;
    esac
    shift
done

# check for src_file in src_dir
found_file=""
for file in "$src_dir"/"$src_file"*.*; do
    if [[ -f "$file" ]]; then
        base=$(basename "$file")
        filename="${base%.*}"
        if [[ "${filename,,}" == "${src_file,,}" ]]; then
            found_file="$filename"
            src_file_ext="${base##*.}"
            break
        fi
    fi
done

if [[ -z "$found_file" ]]; then
    echo "[error] Could not find '$src_file' in '$src_dir'."
    exit 1
fi

# check file extension
ext_valid=0
for ext in $src_exts; do
    if [[ ".${src_file_ext,,}" == "$ext" ]]; then
        ext_valid=1
        break
    fi
done

if [[ $ext_valid -eq 0 ]]; then
    echo "[error] '$src_file' has an unsupported file extension."
    exit 1
fi

echo "[build_mode: $build_mode]"
echo "[compiler: $compiler]"
echo "[src_file: $src_dir/$found_file.$src_file_ext]"

# create build dir if needed
mkdir -p build

# set compile and link flags
common_compile_flags_var="${compiler}_common_compile_flags"
build_compile_flags_var="${compiler}_${build_mode}_compile_flags"
compile_flags="${!common_compile_flags_var} ${!build_compile_flags_var}"

common_link_flags_var="${compiler}_common_link_flags"
build_link_flags_var="${compiler}_${build_mode}_link_flags"
linker_flags="${!common_link_flags_var} ${!build_link_flags_var}"

# compile 
pushd build > /dev/null
if [[ "$compiler" == "gcc" ]]; then
    gcc $compile_flags "../$src_dir/$found_file.$src_file_ext" -o "$found_file" $linker_flags
elif [[ "$compiler" == "clang" ]]; then
    clang $compile_flags "../$src_dir/$found_file.$src_file_ext" -o "$found_file" $linker_flags
fi
popd > /dev/null

# copy file 
cp "build/$found_file" "$found_file"

echo "[info] Build successful."