find ./src ./tests ./sandbox ./tools ./shaders ./samples -name '*.h' -o -name '*.hpp' -o -name '*.c' -o -name '*.cpp' -o -name '*.glsl' | xargs -I % ./thirdparty/bin/clang-format -sort-includes=false -i %
