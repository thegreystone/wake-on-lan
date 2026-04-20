#!/bin/sh
set -e

CC=${CC:-cc}
CFLAGS="-O2 -Wall -Wextra"
TARGET=wol

usage() {
    echo "Usage: $0 [--static] [--install]"
    echo "  --static   Link statically (Linux only)"
    echo "  --install  Install to /usr/local/bin after build"
    exit 1
}

STATIC=0
INSTALL=0
for arg in "$@"; do
    case "$arg" in
        --static)  STATIC=1 ;;
        --install) INSTALL=1 ;;
        *) usage ;;
    esac
done

if [ "$STATIC" = "1" ]; then
    case "$(uname)" in
        Darwin) echo "Static linking is not supported on macOS." >&2; exit 1 ;;
    esac
    CFLAGS="$CFLAGS -static"
fi

echo "Building with: $CC $CFLAGS"
$CC $CFLAGS -o "$TARGET" wol.c
echo "Built: $(ls -lh "$TARGET" | awk '{print $5, $9}')"

if [ "$INSTALL" = "1" ]; then
    install -m 755 "$TARGET" /usr/local/bin/"$TARGET"
    echo "Installed to /usr/local/bin/$TARGET"
fi
