#!/bin/bash

set -e

echo "Running pre-commit checks"
pre-commit run --all-files

echo "Running clang-format"
clang-format include/**/*.hpp src/*.cpp src/**/*.cpp --verbose --dry-run --Werror

echo "Running cppcheck"
cppcheck -Iinclude/ src --verbose --enable=all --error-exitcode=1 --std=c++17 --language=c++ --suppressions-list=cppcheckSuppressions.txt --inline-suppr

echo "Running clang-tidy"
run-clang-tidy -j`nproc` -p=build -header-filter=`pwd`/include/ src/*.cpp src/**/*.cpp
