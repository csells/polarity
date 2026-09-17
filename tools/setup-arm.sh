#!/bin/sh
# Install the pinned, relocatable compiler inside this checkout.
set -eu
cd "$(dirname "$0")/.."
if [ -x tools/arm-toolchain/bin/arm-none-eabi-gcc ]; then
  tools/arm-toolchain/bin/arm-none-eabi-gcc --version
  exit 0
fi
case "$(uname -s)" in Darwin) platform=darwin;; Linux) platform=linux;; *) echo 'Set ARM_CC and ARM_OBJCOPY to an ARM bare-metal toolchain; see README.' >&2; exit 1;; esac
case "$(uname -m)" in arm64|aarch64) arch=arm64;; x86_64) arch=x64;; *) echo 'Unsupported host architecture' >&2; exit 1;; esac
version=15.2.1-1.1
archive="xpack-arm-none-eabi-gcc-$version-$platform-$arch.tar.gz"
url="https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases/download/v$version/$archive"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
curl -fL --progress-bar "$url" -o "$tmp/$archive"
curl -fL --silent --show-error "$url.sha" -o "$tmp/checksum"
(cd "$tmp" && shasum -a 256 -c checksum)
mkdir -p tools/arm-toolchain
tar -xzf "$tmp/$archive" -C tools/arm-toolchain --strip-components=1
tools/arm-toolchain/bin/arm-none-eabi-gcc --version
