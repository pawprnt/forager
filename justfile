# forager build commands

# default: build debug
default: build

# configure cmake (debug)
configure:
    cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# configure cmake (release)
configure-release:
    cmake -B build -DCMAKE_BUILD_TYPE=Release

# build debug
build:
    cmake --build build -j$(nproc)

# build release
release: configure-release
    cmake --build build -j$(nproc)

# run
run: build
    ./build/forager

# clean build dir
clean:
    rm -rf build/

# clean + full rebuild
rebuild: clean configure build

# run tests
test: build
    cd build && ctest --output-on-failure

# install
install:
    cmake --install build

# format all source files
format:
    find src/ -name '*.cpp' -o -name '*.h' | xargs clang-format -i -style=file

# update compile_commands.json symlink
compiledb: configure
    ln -sf build/compile_commands.json .
