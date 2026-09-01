# Third-Party Components

Technical and legal inventory of all third-party dependencies bundled with or
linked by Open-Sankoré Community Edition.

Last updated: 2026-09-01

## SBOM (Software Bill of Materials)

A machine-readable SBOM is maintained at [`sbom.spdx.json`](sbom.spdx.json)
(SPDX 2.3 format). It is a curated document derived from this file, not the
output of an automatic scanner — the project mixes several dependency sources
(Qt via aqtinstall, OpenSSL via vcpkg, QuaZip/zlib built from source, bundled
assets), so a hand-maintained SBOM is more accurate than a generated one.

> **When cutting a new release:** update both this file and `sbom.spdx.json`
> together. Bump the `versionInfo` of the top-level package and the `created`
> timestamp, and make sure every component listed in the tables below has a
> matching entry in the SBOM (and vice versa). The `sbom-check` CI job flags
> drift between the two, but the content update is manual.

## Asset licensing vs source license

The licenses of bundled assets (fonts, icons, widgets) are independent of the
GPL-3.0 license covering the Open-Sankoré source code. All assets currently
bundled permit commercial redistribution. Should any asset with additional
restrictions be introduced, those restrictions would apply to that asset only
and would not alter the GPL-3.0 license of the source code.

## Libraries

| Component | Version | License | Bundled? | Linking | Commercial redistribution | Source |
|-----------|---------|---------|----------|---------|--------------------------|--------|
| Qt 6 | 6.8.3 (Win), 6.8.3 (Linux) | LGPL-3.0 / GPL-3.0 | No (system / CI) | Dynamic | Yes, under LGPL terms | [qt.io](https://www.qt.io/) |
| OpenSSL | 3.x | Apache-2.0 | No (vcpkg / system) | Dynamic | Yes | [openssl.org](https://www.openssl.org/) |
| QuaZip | 1.4 | LGPL-2.1 | No (built from source in CI) | Static (Win), Dynamic (Linux) | Yes | [github.com/stachenov/quazip](https://github.com/stachenov/quazip) |
| zlib | 1.3.1 | zlib | No (built from source in CI) | Static (Win), System (Linux) | Yes | [zlib.net](https://zlib.net/) |
| pdf-merger | — | GPL-3.0 (original project) | Yes (`src/pdf-merger/`) | Compiled in | GPL constraints | Original Open-Sankoré |
| Zinnia | system | BSD-3-Clause | No (`libzinnia-dev`) | Dynamic (Linux only) | Yes | [github.com/taku910/zinnia](https://github.com/taku910/zinnia) |

### Notes on libraries

- **Qt 6**: installed via `aqtinstall` on Windows CI, system packages on Linux. Not bundled in the repository. The LGPL requires that users can re-link against a different Qt version.
- **OpenSSL**: installed via vcpkg on Windows, system package on Linux. Since OpenSSL 3.0 (2021), the license is Apache-2.0, which is GPL-3.0 compatible.
- **QuaZip**: built from source during CI. Static on Windows (linked into the executable), dynamic on Linux (shared library).
- **zlib**: used by pdf-merger for FlateDecode (PDF compression). Built from source on Windows, system library on Linux.
- **pdf-merger**: built-in PDF merge library inherited from the original Open-Sankoré project. Uses zlib internally. Source is in `src/pdf-merger/`.
- **Zinnia**: handwriting recognition engine, Linux only. Loaded at runtime. Not available on Windows (Windows uses its own Ink API instead).

### Xpdf — historical, not active

Xpdf source files (`XPDFRenderer.h`, `XPDFRenderer.cpp`) exist in `src/pdf/` but are **fully stubbed** and **excluded from the build** (commented out in `src/pdf/pdf.pri`). Xpdf is not linked, not compiled, and not a runtime dependency. It is listed here for completeness only.

## Icons

| Component | License | Bundled? | Location | Commercial redistribution | Source |
|-----------|---------|----------|----------|--------------------------|--------|
| Phosphor Icons (regular) | MIT (`resources/icons/phosphor/LICENSE`) | Yes | `resources/icons/phosphor/` | Yes | [github.com/phosphor-icons/core](https://github.com/phosphor-icons/core) |
| Font Awesome 4 | OFL-1.1 (fonts), MIT (CSS/code) | Yes (2 widgets) | `Combinoscope.wgt/fonts/`, `Compteur.wgt/fonts/` | Yes | [fontawesome.com](https://fontawesome.com) |

## Fonts

| Component | License | Bundled? | Location | Commercial redistribution | Source |
|-----------|---------|----------|----------|--------------------------|--------|
| Andika Basic | OFL-1.1 | Yes | `resources/customizations/fonts/AndBasR.ttf` | Yes | [scripts.sil.org](https://scripts.sil.org/Andika) |
| Écolier Court | OFL | Yes | `resources/customizations/fonts/ec_cour.ttf` | Yes | Jean-Marie Douteau |
| Écolier Lignes Court | OFL | Yes | `resources/customizations/fonts/ecl_cour.ttf` | Yes | Jean-Marie Douteau |
| Marelle (8 variants) | OFL-1.1 | Yes | `resources/customizations/fonts/Marelle*.ttf`, `Marelle*.otf` | Yes | [marelle.forge.apps.education.fr](https://marelle.forge.apps.education.fr/) |
| URW Base35 (14 files) | AGPL-3.0 + font exception | Yes | `resources/fonts/*.pfb` | Yes | [github.com/ArtifexSoftware/urw-base35-fonts](https://github.com/ArtifexSoftware/urw-base35-fonts) |

### Notes on fonts

- **Marelle**: official cursive school font from the French Ministère de l'Éducation nationale (2026). License file at `resources/customizations/fonts/Marelle-OFL-LICENSE.txt`.
- **URW Base35**: PostScript base fonts (Nimbus Sans = Helvetica, Nimbus Roman = Times, Nimbus Mono = Courier, etc.). The AGPL font exception allows embedding in documents and applications without triggering AGPL copyleft.
- All non-free fonts were removed in issue #186.

## Widgets

30 `.wgt` mini-applications are bundled in `resources/library/applications/`. These are HTML/JS/CSS widgets inherited from the original Open-Sankoré project (GPL-3.0). Two of them (Combinoscope, Compteur) embed Font Awesome (see Icons section above).

## License files in the repository

| File | Covers |
|------|--------|
| `LICENSE.md` | Main project license (GPL-3.0) + OpenSSL exception text |
| `NOTICE.md` | Copyright holders and licensing summary |
| `CREDITS.md` | Human-readable acknowledgments |
| `resources/customizations/fonts/Marelle-OFL-LICENSE.txt` | Marelle font OFL-1.1 license |
| `resources/icons/phosphor/LICENSE` | Phosphor Icons MIT license |
