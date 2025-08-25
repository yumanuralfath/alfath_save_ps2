#!/bin/bash
# Build dan install alfathsave di Termux

set -e

# Pastikan dependensi
pkg install -y clang make git

# Build
echo "Building alfathsave for Termux..."
make release

# Install ke $PREFIX/bin
echo "Installing to $PREFIX/bin..."
mkdir -p "$PREFIX"/bin
cp alfathsave "$PREFIX"/bin/

echo "alfathsave installed successfully!"
echo "Run: alfathsave --version"
