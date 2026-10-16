#!/usr/bin/env bash
set -euo pipefail

linuxdeploy_url="https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20251107-1/linuxdeploy-x86_64.AppImage"
linuxdeploy_sha256="c20cd71e3a4e3b80c3483cef793cda3f4e990aca14014d23c544ca3ce1270b4d"
qt_plugin_url="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/1-alpha-20250213-1/linuxdeploy-plugin-qt-x86_64.AppImage"
qt_plugin_sha256="15106be885c1c48a021198e7e1e9a48ce9d02a86dd0a1848f00bdbf3c1c92724"
runtime_url="https://github.com/AppImage/type2-runtime/releases/download/20251108/runtime-x86_64"
runtime_sha256="2fca8b443c92510f1483a883f60061ad09b46b978b2631c807cd873a47ec260d"

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
: "${QT_ROOT_DIR:?QT_ROOT_DIR must point at the Qt kit}"
: "${APP_VERSION:?APP_VERSION must be MAJOR.MINOR.PATCH}"
build_dir="${BUILD_DIR:-$root/build/linux-release}"
out_dir="${OUT_DIR:-$root/dist}"
work_dir="${WORK_DIR:-$root/build/appimage}"
tools_dir="$work_dir/tools"
app_dir="$work_dir/AppDir"
asset="gsx-integrator-installer-${APP_VERSION}-x86_64.AppImage"
exe="$build_dir/bin/gsx-integrator-installer"

fetch() {
    local url="$1" sha256="$2" dest="$3"
    if [[ ! -f "$dest" ]] || ! echo "$sha256  $dest" | sha256sum --check --status; then
        curl --fail --silent --show-error --location --retry 3 --output "$dest" "$url"
    fi
    echo "$sha256  $dest" | sha256sum --check --status \
        || { echo "checksum mismatch for $url" >&2; exit 1; }
}

[[ -x "$exe" ]] || { echo "build the app first: $exe not found" >&2; exit 1; }

mkdir -p "$tools_dir" "$out_dir"
fetch "$linuxdeploy_url" "$linuxdeploy_sha256" "$tools_dir/linuxdeploy"
fetch "$qt_plugin_url" "$qt_plugin_sha256" "$tools_dir/linuxdeploy-plugin-qt"
fetch "$runtime_url" "$runtime_sha256" "$tools_dir/runtime-x86_64"
chmod +x "$tools_dir/linuxdeploy" "$tools_dir/linuxdeploy-plugin-qt"

export PATH="$tools_dir:$PATH"
export APPIMAGE_EXTRACT_AND_RUN=1
export QMAKE="$QT_ROOT_DIR/bin/qmake"
export QML_SOURCES_PATHS="$root/src/qml"
export EXTRA_PLATFORM_PLUGINS="libqwayland-generic.so;libqwayland-egl.so"
export LDAI_RUNTIME_FILE="$tools_dir/runtime-x86_64"
export LDAI_OUTPUT="$asset"

openssl_dir="$(dirname "$(ldconfig -p | awk '/libssl\.so\.3 .*x86-64/ {print $NF; exit}')")"
[[ -f "$openssl_dir/libssl.so.3" && -f "$openssl_dir/libcrypto.so.3" ]] \
    || { echo "OpenSSL 3 runtime libraries not found" >&2; exit 1; }

rm -rf "$app_dir"
linuxdeploy \
    --appdir "$app_dir" \
    --executable "$exe" \
    --desktop-file "$root/packaging/linux/gsx-integrator-installer.desktop" \
    --icon-file "$root/assets/branding/icon_256.png" \
    --icon-filename gsx-integrator-installer \
    --library "$openssl_dir/libssl.so.3" \
    --library "$openssl_dir/libcrypto.so.3" \
    --plugin qt

mkdir -p "$app_dir/usr/plugins/platformthemes"
cp "$QT_ROOT_DIR/plugins/platformthemes/libqxdgdesktopportal.so" "$app_dir/usr/plugins/platformthemes/"
for plugin in wayland-shell-integration/libxdg-shell.so wayland-decoration-client/libbradient.so \
    wayland-graphics-integration-client/libqt-plugin-wayland-egl.so; do
    mkdir -p "$app_dir/usr/plugins/$(dirname "$plugin")"
    cp "$QT_ROOT_DIR/plugins/$plugin" "$app_dir/usr/plugins/$plugin"
done

qml_dir="$app_dir/usr/qml"
rm -rf \
    "$qml_dir"/QtQuick/Controls/{FluentWinUI3,Fusion,Imagine,Material,Universal,designer} \
    "$qml_dir"/QtQuick/{Dialogs,Effects,LocalStorage,NativeStyle,Particles,Shapes,VectorImage,tooling} \
    "$qml_dir"/QtQml/XmlListModel \
    "$app_dir"/usr/plugins/{imageformats,iconengines}
rm -f \
    "$app_dir"/usr/lib/libQt6QuickControls2{FluentWinUI3StyleImpl,Fusion,FusionStyleImpl,Imagine,ImagineStyleImpl,Material,MaterialStyleImpl,Universal,UniversalStyleImpl}.so* \
    "$app_dir"/usr/lib/libQt6QuickDialogs2*.so* \
    "$app_dir"/usr/lib/libQt6Quick{Effects,Particles,Shapes,VectorImage,VectorImageGenerator}.so* \
    "$app_dir"/usr/lib/libQt6Qml{LocalStorage,XmlListModel}.so* \
    "$app_dir"/usr/lib/libQt6{Sql,Svg}.so*

missing=0
while IFS= read -r -d '' file; do
    if LD_LIBRARY_PATH="$app_dir/usr/lib" ldd "$file" 2>/dev/null | grep -q "not found"; then
        echo "unresolved libraries in ${file#"$app_dir"/}:" >&2
        LD_LIBRARY_PATH="$app_dir/usr/lib" ldd "$file" | grep "not found" >&2
        missing=1
    fi
done < <(find "$app_dir/usr" -type f \( -name '*.so' -o -name '*.so.*' -o -path '*/usr/bin/*' \) -print0)
[[ "$missing" -eq 0 ]] || exit 1

rm -f "$out_dir/$asset" "$out_dir/$asset.sha256"
(cd "$out_dir" && linuxdeploy --appdir "$app_dir" --output appimage)
[[ -f "$out_dir/$asset" ]] || { echo "linuxdeploy did not produce $asset" >&2; exit 1; }

(cd "$out_dir" && sha256sum "$asset" > "$asset.sha256")
ls -l "$out_dir/$asset" "$out_dir/$asset.sha256"
