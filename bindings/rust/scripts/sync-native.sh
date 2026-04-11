#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CRATE_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
REPO_DIR="$(cd "${CRATE_DIR}/../.." && pwd)"

SOURCE_INCLUDE_DIR="${REPO_DIR}/include"
SOURCE_VISION_INCLUDE_DIR="${SOURCE_INCLUDE_DIR}/vision"
SOURCE_SRC_DIR="${REPO_DIR}/src"

TARGET_NATIVE_DIR="${CRATE_DIR}/native"
TARGET_INCLUDE_DIR="${TARGET_NATIVE_DIR}/include"
TARGET_VISION_INCLUDE_DIR="${TARGET_INCLUDE_DIR}/vision"
TARGET_SRC_DIR="${TARGET_NATIVE_DIR}/src"

CORE_SOURCES=(
  "vision_ui_animation.c"
  "vision_ui_core.c"
  "vision_ui_item.c"
  "vision_ui_renderer.c"
)

mkdir -p "${TARGET_VISION_INCLUDE_DIR}" "${TARGET_SRC_DIR}"

cp "${SOURCE_INCLUDE_DIR}/vision_ui_config.h" "${TARGET_INCLUDE_DIR}/vision_ui_config.h"

for header in "${SOURCE_VISION_INCLUDE_DIR}"/*.h; do
  cp "${header}" "${TARGET_VISION_INCLUDE_DIR}/"
done

for source in "${CORE_SOURCES[@]}"; do
  cp "${SOURCE_SRC_DIR}/${source}" "${TARGET_SRC_DIR}/${source}"
done

echo "Synced Vision UI native sources into ${TARGET_NATIVE_DIR}"
