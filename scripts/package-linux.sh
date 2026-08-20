#!/bin/bash
# package-linux.sh — Build .deb and .rpm packages from the compiled binary
# Usage: ./scripts/package-linux.sh [architecture]
#   architecture: amd64 (default) or arm64

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
ARCH="${1:-amd64}"
VERSION="4.0.1"
PRODUCT_DIR="$PROJECT_DIR/build/linux/release/product"
PKG_DIR="$PROJECT_DIR/build/packages"

echo "=== Open-Sankoré Linux Packaging ==="
echo "Version: $VERSION"
echo "Architecture: $ARCH"
echo "Product dir: $PRODUCT_DIR"

# Verify binary exists
if [ ! -f "$PRODUCT_DIR/Open-Sankore" ]; then
    echo "ERROR: Binary not found at $PRODUCT_DIR/Open-Sankore"
    echo "Run 'make' first."
    exit 1
fi

# Clean package dir
rm -rf "$PKG_DIR"
mkdir -p "$PKG_DIR"

# ============================================================
# .deb package
# ============================================================
echo ""
echo "=== Building .deb package ==="

DEB_ROOT="$PKG_DIR/deb-root"
mkdir -p "$DEB_ROOT/usr/bin"
mkdir -p "$DEB_ROOT/usr/share/open-sankore/resources"
mkdir -p "$DEB_ROOT/usr/share/applications"
mkdir -p "$DEB_ROOT/usr/share/pixmaps"
mkdir -p "$DEB_ROOT/DEBIAN"

# Copy binary
cp "$PRODUCT_DIR/Open-Sankore" "$DEB_ROOT/usr/bin/open-sankore"
chmod 755 "$DEB_ROOT/usr/bin/open-sankore"

# Copy resources (if available)
if [ -d "$PROJECT_DIR/resources/etc" ]; then
    cp -r "$PROJECT_DIR/resources/etc" "$DEB_ROOT/usr/share/open-sankore/"
fi
if [ -d "$PROJECT_DIR/resources/library" ]; then
    cp -r "$PROJECT_DIR/resources/library" "$DEB_ROOT/usr/share/open-sankore/"
fi

# Desktop entry
cat > "$DEB_ROOT/usr/share/applications/open-sankore.desktop" << EOF
[Desktop Entry]
Name=Open-Sankoré
Comment=Interactive whiteboard application
Exec=open-sankore
Icon=open-sankore
Type=Application
Categories=Education;
Terminal=false
EOF

# Control file
if [ "$ARCH" = "arm64" ]; then
    DEB_ARCH="arm64"
else
    DEB_ARCH="amd64"
fi

cat > "$DEB_ROOT/DEBIAN/control" << EOF
Package: open-sankore
Version: $VERSION
Section: education
Priority: optional
Architecture: $DEB_ARCH
Depends: libqt6core6 (>= 6.2), libqt6gui6 (>= 6.2), libqt6widgets6 (>= 6.2), libqt6svg6 (>= 6.2), libqt6multimedia6 (>= 6.2), libquazip1-qt6-1, libssl3, libx11-6, libgomp1, zlib1g
Maintainer: David Guyomarch <david.guyomarch@gmail.com>
Description: Open-Sankoré interactive whiteboard
 Open-Sankoré is an open-source interactive whiteboard application
 for digital teaching. It supports pen drawing, geometric tools,
 document annotation, and multimedia content.
Homepage: https://github.com/davidguyomarch/Sankore
EOF

# Build .deb
DEB_FILE="$PKG_DIR/open-sankore_${VERSION}_${DEB_ARCH}.deb"
dpkg-deb --build --root-owner-group "$DEB_ROOT" "$DEB_FILE"
echo "Created: $DEB_FILE"

# ============================================================
# .rpm package
# ============================================================
echo ""
echo "=== Building .rpm package ==="

# Check if rpmbuild is available
if command -v rpmbuild &> /dev/null; then
    RPM_BUILD_DIR="$PKG_DIR/rpmbuild"
    mkdir -p "$RPM_BUILD_DIR"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

    if [ "$ARCH" = "arm64" ]; then
        RPM_ARCH="aarch64"
    else
        RPM_ARCH="x86_64"
    fi

    # Create tarball for rpmbuild
    TARBALL_DIR="open-sankore-$VERSION"
    mkdir -p "$PKG_DIR/$TARBALL_DIR/usr/bin"
    mkdir -p "$PKG_DIR/$TARBALL_DIR/usr/share/applications"
    cp "$PRODUCT_DIR/Open-Sankore" "$PKG_DIR/$TARBALL_DIR/usr/bin/open-sankore"
    chmod 755 "$PKG_DIR/$TARBALL_DIR/usr/bin/open-sankore"
    cp "$DEB_ROOT/usr/share/applications/open-sankore.desktop" "$PKG_DIR/$TARBALL_DIR/usr/share/applications/"

    tar -czf "$RPM_BUILD_DIR/SOURCES/open-sankore-$VERSION.tar.gz" -C "$PKG_DIR" "$TARBALL_DIR"

    # Spec file
    cat > "$RPM_BUILD_DIR/SPECS/open-sankore.spec" << EOF
Name:           open-sankore
Version:        $VERSION
Release:        1%{?dist}
Summary:        Interactive whiteboard application
License:        GPLv3
URL:            https://github.com/davidguyomarch/Sankore
Source0:        open-sankore-%{version}.tar.gz
BuildArch:      $RPM_ARCH

Requires:       qt6-qtbase >= 6.2
Requires:       qt6-qtsvg >= 6.2
Requires:       qt6-qtmultimedia >= 6.2
Requires:       quazip-qt6
Requires:       openssl-libs
Requires:       libX11
Requires:       libgomp
Requires:       zlib

%description
Open-Sankoré is an open-source interactive whiteboard application
for digital teaching.

%prep
%setup -q

%install
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/share/applications
cp usr/bin/open-sankore %{buildroot}/usr/bin/
cp usr/share/applications/open-sankore.desktop %{buildroot}/usr/share/applications/

%files
%attr(755, root, root) /usr/bin/open-sankore
/usr/share/applications/open-sankore.desktop
EOF

    rpmbuild --define "_topdir $RPM_BUILD_DIR" -bb "$RPM_BUILD_DIR/SPECS/open-sankore.spec"
    RPM_FILE=$(find "$RPM_BUILD_DIR/RPMS" -name "*.rpm" | head -1)
    if [ -n "$RPM_FILE" ]; then
        cp "$RPM_FILE" "$PKG_DIR/"
        echo "Created: $PKG_DIR/$(basename "$RPM_FILE")"
    fi
else
    echo "rpmbuild not found, skipping .rpm package"
fi

echo ""
echo "=== Packages ==="
ls -la "$PKG_DIR"/*.deb "$PKG_DIR"/*.rpm 2>/dev/null || echo "No packages created"
