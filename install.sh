#!/bin/sh
set -e

REPO="thegreystone/wake-on-lan"
TARGET=wol
INSTALL_DIR=/usr/local/bin

usage() {
    echo "Usage: $0 [VERSION] [--dir DIR]"
    echo "  VERSION    Tag to install, e.g. v1.0.0 (default: latest release)"
    echo "  --dir DIR  Install directory (default: /usr/local/bin)"
    exit 1
}

VERSION=""
while [ $# -gt 0 ]; do
    case "$1" in
        --dir)
            shift
            INSTALL_DIR="${1:?--dir requires an argument}"
            ;;
        --dir=*)
            INSTALL_DIR="${1#--dir=}"
            ;;
        v[0-9]*)
            VERSION="$1"
            ;;
        --help|-h)
            usage
            ;;
        *)
            usage
            ;;
    esac
    shift
done

OS=$(uname -s)
ARCH=$(uname -m)
case "$OS-$ARCH" in
    Linux-x86_64)          SUFFIX="linux-x86_64" ;;
    Linux-aarch64)         SUFFIX="linux-aarch64" ;;
    Darwin-arm64)          SUFFIX="macos-aarch64" ;;
    Darwin-x86_64)         SUFFIX="macos-x86_64" ;;
    *)
        echo "No pre-built binary for $OS/$ARCH — please build from source: ./build.sh" >&2
        exit 1
        ;;
esac

if [ -z "$VERSION" ]; then
    echo "Fetching latest release..."
    VERSION=$(curl -sf "https://api.github.com/repos/${REPO}/releases/latest" \
        | grep '"tag_name"' | grep -o 'v[0-9][^"]*')
    if [ -z "$VERSION" ]; then
        echo "Could not determine latest release version." >&2
        exit 1
    fi
fi

BINARY="wol-${VERSION}-${SUFFIX}"
URL="https://github.com/${REPO}/releases/download/${VERSION}/${BINARY}"
TMPFILE=$(mktemp)

echo "Downloading $BINARY..."
curl -fL -o "$TMPFILE" "$URL"
chmod 755 "$TMPFILE"

install -m 755 "$TMPFILE" "${INSTALL_DIR}/${TARGET}"
rm -f "$TMPFILE"

echo "Installed ${TARGET} ${VERSION} to ${INSTALL_DIR}/${TARGET}"
