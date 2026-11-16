#!/usr/bin/env bash
# Builds ImmersiveOrnament (Release) and packages it as an AppImage.
#
#   scripts/make_appimage.sh            -> ImmersiveOrnament-x86_64.AppImage in the project root
#
# Settings stay editable: on start the AppImage uses a data folder
#   <folder of the .AppImage>/ImmersiveOrnament-data/
# (fallback if not writable: ~/.config/ImmersiveOrnament/data, or set
#  IMMERSIVE_ORNAMENT_DATA=/some/dir yourself). Default files from bin/data are
# copied there on first start; existing files are never overwritten.
#
# GStreamer + GLib are taken from the target system (GStreamer loads its plugins
# from the system, bundling only the core libs breaks that). Install on target:
#   gstreamer, gst-plugins-base, gst-plugins-good (+ libcamera gst plugin if needed)
set -euo pipefail

APP=ImmersiveOrnament
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build/appimage"
APPDIR="$BUILD/AppDir"
TOOLS="$ROOT/tools"
LINUXDEPLOY="$TOOLS/linuxdeploy-x86_64.AppImage"
# run tool AppImages without FUSE
export APPIMAGE_EXTRACT_AND_RUN=1

cd "$ROOT"

echo "==> Building Release"
make Release -j"$(nproc)"

echo "==> Fetching linuxdeploy (if needed)"
mkdir -p "$TOOLS"
if [ ! -x "$LINUXDEPLOY" ]; then
    curl -fL -o "$LINUXDEPLOY" \
        https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x "$LINUXDEPLOY"
fi

echo "==> Preparing AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/share/$APP" "$BUILD"
cp "bin/$APP" "$APPDIR/usr/bin/$APP"
cp -r bin/data "$APPDIR/usr/share/$APP/data"

# desktop entry
cat > "$BUILD/$APP.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Immersive Ornament
Exec=$APP
Icon=$APP
Categories=Graphics;
Terminal=false
EOF

# icon: use icon.png in the project root if present, otherwise generate a simple one
ICON="$BUILD/$APP.png"
if [ -f "$ROOT/icon.png" ]; then
    cp "$ROOT/icon.png" "$ICON"
else
    python3 - "$ICON" <<'EOF'
import struct, sys, zlib, math
n = 256
rows = []
for y in range(n):
    row = bytearray([0])
    for x in range(n):
        t = x / (n - 1)
        r, g, b = (255, 80, 0) if t < .5 else (0, 120, 255)
        k = 1 - abs(t - .5) * 2              # white in the center like the gradient
        v = .6 + .4 * math.sin(x * .15) * math.sin(y * .15)
        row += bytes(int((c + (255 - c) * k) * v) for c in (r, g, b))
    rows.append(bytes(row))
def chunk(t, d): return struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d))
png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", n, n, 8, 2, 0, 0, 0)) \
    + chunk(b"IDAT", zlib.compress(b"".join(rows), 9)) + chunk(b"IEND", b"")
open(sys.argv[1], "wb").write(png)
EOF
fi

# start script: sets up the writable data folder, then starts the app
cat > "$BUILD/AppRun" <<'EOF'
#!/usr/bin/env bash
HERE="$(dirname "$(readlink -f "$0")")"
APP=ImmersiveOrnament

if [ -z "${IMMERSIVE_ORNAMENT_DATA:-}" ]; then
    if [ -n "${APPIMAGE:-}" ]; then BASE="$(dirname "$APPIMAGE")"; else BASE="$HERE/.."; fi
    DATA="$BASE/$APP-data"
    if ! mkdir -p "$DATA" 2>/dev/null || [ ! -w "$DATA" ]; then
        DATA="${XDG_CONFIG_HOME:-$HOME/.config}/$APP/data"
        mkdir -p "$DATA"
    fi
    export IMMERSIVE_ORNAMENT_DATA="$DATA"
fi

# copy default files that don't exist yet (never overwrite user edits)
DEFAULTS="$HERE/usr/share/$APP/data"
( cd "$DEFAULTS" && find . -type d -exec mkdir -p "$IMMERSIVE_ORNAMENT_DATA/{}" \; \
                 && find . -type f -exec sh -c '[ -e "$2/$1" ] || cp "$1" "$2/$1"' _ {} "$IMMERSIVE_ORNAMENT_DATA" \; )
echo "[$APP] data folder: $IMMERSIVE_ORNAMENT_DATA"

# locally installed GStreamer plugins (e.g. self-built libcamerasrc)
if [ -d /usr/local/lib/gstreamer-1.0 ]; then
    export GST_PLUGIN_PATH="${GST_PLUGIN_PATH:+$GST_PLUGIN_PATH:}/usr/local/lib/gstreamer-1.0"
fi

exec "$HERE/usr/bin/$APP" "$@"
EOF
chmod +x "$BUILD/AppRun"

echo "==> Bundling libraries + creating AppImage"
cd "$BUILD"
OUTPUT="$ROOT/$APP-x86_64.AppImage" LDAI_OUTPUT="$ROOT/$APP-x86_64.AppImage" "$LINUXDEPLOY" \
    --appdir "$APPDIR" \
    --executable "$APPDIR/usr/bin/$APP" \
    --desktop-file "$BUILD/$APP.desktop" \
    --icon-file "$ICON" \
    --custom-apprun "$BUILD/AppRun" \
    --exclude-library 'libgst*' \
    --exclude-library 'liborc-*' \
    --exclude-library 'libglib-2.0*' \
    --exclude-library 'libgobject-2.0*' \
    --exclude-library 'libgmodule-2.0*' \
    --exclude-library 'libgio-2.0*' \
    --output appimage

echo
echo "==> Done: $ROOT/$APP-x86_64.AppImage"
echo "    Settings will be in: <folder of the AppImage>/$APP-data/settings.json"
