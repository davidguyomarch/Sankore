# Open-Sankoré 3.1

![Build Windows](https://github.com/davidguyomarch/Sankore-3.1/actions/workflows/build-windows.yml/badge.svg)
![Build Linux](https://github.com/davidguyomarch/Sankore-3.1/actions/workflows/build-linux.yml/badge.svg)
![Coverage](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/davidguyomarch/0b042ec6f4db15ba496d37215c18a649/raw/coverage.json)

Interactive whiteboard application for education, migrated to Qt 6.

## Features

- Freehand drawing with multiple tools (pen, marker, eraser)
- Geometric instruments (ruler, compass, protractor, triangle)
- Document management (.ubz format)
- PDF export and import
- Desktop annotation mode
- Multi-page whiteboard with thumbnails

## Building

### Windows (CI)

```bash
qmake OpenSankore.pro CONFIG+=no_webengine
nmake release
windeployqt --release build\win32\release\product\Open-Sankore.exe
```

### Linux (Docker)

```bash
docker run --rm -v $(pwd):/src -w /src sankore-qt6 bash -c 'qmake6 OpenSankore.pro CONFIG+=no_webengine && make -j$(nproc)'
```

## Downloads

See [Releases](https://github.com/davidguyomarch/Sankore-3.1/releases) for pre-built packages:
- Windows: `.exe` installer + `.zip` portable
- Linux x64: `.deb` (Debian/Ubuntu) + `.rpm` (Fedora/RedHat)

## Requirements

- Qt 6.2+ (6.8.2 on Windows CI)
- OpenSSL 3.x
- QuaZip 1.4
- zlib 1.3+

## License

GNU General Public License v3 — see [LICENSE.txt](LICENSE.txt)
