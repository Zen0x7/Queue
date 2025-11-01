#!/usr/bin/env bash
set -e

if ! find ./src ./tests -regextype posix-extended -regex '.*\.(cpp|hpp|ipp|cc|cxx)' \
  -exec clang-format --dry-run --Werror {} +; then
  exit 1
fi

if [ -f main.cpp ]; then
  if ! clang-format --dry-run --Werror main.cpp; then
    exit 1
  fi
fi