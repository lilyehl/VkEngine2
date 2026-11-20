#!/usr/bin/env bash
# Compiles every .slang file in this directory to a .spv with the same name
# (voxel.slang -> voxel.spv). Each shader must define vertMain and fragMain.
set -euo pipefail

cd "$(dirname "$0")"

echo "Removing existing .spv files .."
rm *.spv

for src in *.slang; do
  out="${src%.slang}.spv"
  echo "Compiling $src -> $out .."
  slangc "$src" -target spirv -profile spirv_1_4 -emit-spirv-directly \
    -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o "$out"
done
