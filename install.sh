#!/bin/sh

set -eu

if [ "$(uname -s)" != "Darwin" ]; then
    echo "This installer is for macOS. On Linux, install the equivalent development packages and run GNU make." >&2
    exit 1
fi

if ! command -v brew >/dev/null 2>&1; then
    echo "Homebrew is required. Install it from https://brew.sh, then run this script again." >&2
    exit 1
fi

echo "[+] Installing build dependencies..."
brew install make pkg-config libusb libimobiledevice libirecovery libusbmuxd \
    libimobiledevice-glue libplist mbedtls readline xz vim

if command -v gmake >/dev/null 2>&1; then
    MAKE=gmake
elif make --version 2>/dev/null | grep -q 'GNU Make'; then
    MAKE=make
else
    echo "GNU Make was not found. Try opening a new shell so Homebrew's gmake is on PATH." >&2
    exit 1
fi

echo "[+] Building with $MAKE..."
$MAKE clean >/dev/null 2>&1 || true
$MAKE

if [ ! -x ./src/palera1n ]; then
    echo "error: build completed without creating ./src/palera1n" >&2
    exit 1
fi

echo "[+] Installing palera1n..."
sudo $MAKE install

echo "Done. Run: palera1n --help"
