find src -name "*.cpp" -exec clang-format -style=file -i {} \;
find test -name "*.cpp" -exec clang-format -style=file -i {} \;
