#!/usr/bin/env bash
#
# build-pkg.sh — package the universal abiv1 dylib as a signed + notarized macOS .pkg.
#
# Produces UAP-macOS-<VERSION>.pkg installing into the Wooting plugin dir:
#   /usr/local/share/WootingAnalogPlugins/universal-analog-plugin/abiv1.dylib
#
# Prerequisites (one-time, on the signing machine):
#   1. "Developer ID Application: KrookedLilly LLC (CJ6VS87K3J)" in the keychain  (signs the dylib)
#   2. "Developer ID Installer: KrookedLilly LLC (CJ6VS87K3J)"  in the keychain  (signs the pkg)
#        - create at developer.apple.com → Certificates → "+" → Developer ID Installer
#   3. A notarytool keychain profile:
#        xcrun notarytool store-credentials "uap-notary" --apple-id "<email>" --team-id "CJ6VS87K3J"
#
# Usage:
#   ./macos-spike/build-pkg.sh            # full: sign → pkg → notarize → staple → verify
#   NOTARIZE=0 ./macos-spike/build-pkg.sh # build + sign the pkg but skip notarize/staple (dry run)
#
set -euo pipefail

# ---- config -----------------------------------------------------------------
VERSION="${VERSION:-1.0.0}"                          # pkg --version; release tag is macos-$VERSION
TEAM_ID="CJ6VS87K3J"
APP_IDENTITY="${APP_IDENTITY:-Developer ID Application: KrookedLilly LLC (${TEAM_ID})}"
INSTALLER_IDENTITY="${INSTALLER_IDENTITY:-Developer ID Installer: KrookedLilly LLC (${TEAM_ID})}"
NOTARY_PROFILE="${NOTARY_PROFILE:-uap-notary}"
PKG_IDENTIFIER="com.krookedlilly.universalanalogplugin.macos"
PLUGIN_DIRNAME="universal-analog-plugin"              # must match the Wooting plugin folder name
NOTARIZE="${NOTARIZE:-1}"

# ---- paths ------------------------------------------------------------------
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/.." && pwd)"
DYLIB="$HERE/abiv1-universal.dylib"
STAGE="$(mktemp -d /tmp/uap-stage.XXXXXX)"
OUT_UNSIGNED="$REPO/UAP-macOS-${VERSION}-unsigned.pkg"
OUT_PKG="$REPO/UAP-macOS-${VERSION}.pkg"

cleanup() { rm -rf "$STAGE"; }
trap cleanup EXIT

say() { printf '\n\033[1;36m== %s\033[0m\n' "$*"; }
die() { printf '\033[1;31mERROR: %s\033[0m\n' "$*" >&2; exit 1; }

# ---- preflight --------------------------------------------------------------
say "Preflight"
[ -f "$DYLIB" ] || die "missing $DYLIB — build it first (see RELEASE-HANDOFF.md 'Universal build')."
arches="$(lipo -archs "$DYLIB")"
echo "  dylib arches: $arches"
case "$arches" in *x86_64*arm64*|*arm64*x86_64*) ;; *) die "dylib is not universal (got: $arches)";; esac
security find-identity -v -p codesigning | grep -qF "$APP_IDENTITY" \
  || die "Application identity not found: $APP_IDENTITY"
security find-identity -v | grep -qF "$INSTALLER_IDENTITY" \
  || die "Installer identity not found: $INSTALLER_IDENTITY  (create a 'Developer ID Installer' cert)"

# ---- 1. sign the dylib (Application cert, hardened runtime, secure timestamp)
say "Signing dylib"
codesign --force --options runtime --timestamp --sign "$APP_IDENTITY" "$DYLIB"
codesign --verify --strict --verbose=2 "$DYLIB"

# ---- 2. stage the install tree ---------------------------------------------
say "Staging install tree"
mkdir -p "$STAGE/$PLUGIN_DIRNAME"
cp "$DYLIB" "$STAGE/$PLUGIN_DIRNAME/abiv1.dylib"

# ---- 3. build component pkg -------------------------------------------------
say "Building component pkg"
pkgbuild --root "$STAGE" \
  --install-location /usr/local/share/WootingAnalogPlugins \
  --identifier "$PKG_IDENTIFIER" \
  --version "$VERSION" \
  "$OUT_UNSIGNED"

# ---- 4. sign the pkg (Installer cert) --------------------------------------
say "Signing pkg"
productsign --sign "$INSTALLER_IDENTITY" "$OUT_UNSIGNED" "$OUT_PKG"
rm -f "$OUT_UNSIGNED"
pkgutil --check-signature "$OUT_PKG"

# ---- 5. notarize + staple --------------------------------------------------
if [ "$NOTARIZE" = "1" ]; then
  say "Notarizing (this waits for Apple)"
  xcrun notarytool submit "$OUT_PKG" --keychain-profile "$NOTARY_PROFILE" --wait
  say "Stapling"
  xcrun stapler staple "$OUT_PKG"
  say "Gatekeeper assessment"
  spctl --assess --type install -vv "$OUT_PKG"
else
  say "NOTARIZE=0 — skipped notarize/staple. $OUT_PKG is signed but NOT notarized."
fi

say "Done"
echo "  Artifact: $OUT_PKG"
echo "  Next: test-install on Intel, then create GitHub Release tag macos-${VERSION} on KrookedLilly/universal-analog-plugin."
