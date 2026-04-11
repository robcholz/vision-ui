#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
CRATE_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
REPO_DIR=$(CDPATH= cd -- "$CRATE_DIR/../.." && pwd)

NATIVE_DIR="$CRATE_DIR/native"

rm -rf "$NATIVE_DIR/include" "$NATIVE_DIR/src"
mkdir -p "$NATIVE_DIR/include/vision" "$NATIVE_DIR/src"

cp "$REPO_DIR/include/vision_ui_config.h" "$NATIVE_DIR/include/"
cp "$REPO_DIR/include/vision/"*.h "$NATIVE_DIR/include/vision/"
cp "$REPO_DIR/src/"*.c "$NATIVE_DIR/src/"

echo "synced native sources into $NATIVE_DIR"
