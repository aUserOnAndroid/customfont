#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
OUTPUT_PATH="${1:-${ROOT_DIR}/customfont.geode}"
WORK_DIR="$(mktemp -d)"
trap 'rm -rf "${WORK_DIR}"' EXIT

mkdir -p "${WORK_DIR}/resources"

cp "${ROOT_DIR}/mod.json" "${WORK_DIR}/"

if [ -d "${ROOT_DIR}/resources" ]; then
  cp -R "${ROOT_DIR}/resources/." "${WORK_DIR}/resources/"
fi

# Copy any built binaries, using Geode-compatible names for the current platform.
# The Linux build produced libauseronandroid.customfont.so, which is copied to the
# root as auseronandroid.customfont.so for package compatibility.
for artifact in \
  "${BUILD_DIR}/libauseronandroid.customfont.so" \
  "${BUILD_DIR}/auseronandroid.customfont.so" \
  "${BUILD_DIR}/auseronandroid.customfont.dll" \
  "${BUILD_DIR}/auseronandroid.customfont.dylib" \
  "${BUILD_DIR}/auseronandroid.customfont.android32.so" \
  "${BUILD_DIR}/auseronandroid.customfont.android64.so"; do
  if [ -f "$artifact" ]; then
    cp "$artifact" "${WORK_DIR}/$(basename "$artifact")"
    if [ "$(basename "$artifact")" = "libauseronandroid.customfont.so" ]; then
      cp "$artifact" "${WORK_DIR}/auseronandroid.customfont.so"
    fi
  fi
done

(
  cd "${WORK_DIR}"
  zip -r "${OUTPUT_PATH}" .
)

echo "Created ${OUTPUT_PATH}"
