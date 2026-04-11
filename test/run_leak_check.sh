#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build/leak"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

cc \
  -std=gnu99 \
  -O0 \
  -g3 \
  -I"$ROOT_DIR/include" \
  -I"$ROOT_DIR/include/vision" \
  -I"$ROOT_DIR/test" \
  "$ROOT_DIR"/src/*.c \
  "$ROOT_DIR"/test/mock_driver.c \
  "$ROOT_DIR"/test/unity.c \
  "$ROOT_DIR"/test/test_api.c \
  "$ROOT_DIR"/test/test_main.c \
  -lm \
  -o "$BUILD_DIR/vision_ui_tests"

valgrind \
  --tool=memcheck \
  --leak-check=full \
  --show-leak-kinds=all \
  --errors-for-leak-kinds=definite,indirect,possible,reachable \
  --track-origins=yes \
  --error-exitcode=101 \
  "$BUILD_DIR/vision_ui_tests"
