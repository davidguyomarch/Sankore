# Security Policy

## Supported versions

Only the latest release on the `master` branch is actively maintained.
Older versions do not receive security updates.

## Reporting a vulnerability

If you discover a security vulnerability in Open-Sankoré Community Edition,
please report it **privately** rather than opening a public issue.

**Method:** Open a
[private security advisory](https://github.com/davidguyomarch/Sankore/security/advisories/new)
on GitHub.

Please include:
- A description of the vulnerability
- Steps to reproduce
- The version or commit hash affected
- Any potential impact assessment

You should receive an acknowledgment within 7 days. A fix will be prioritized
based on severity.

## Scope

This policy covers the Open-Sankoré application code and its bundled resources.
It does not cover third-party dependencies (Qt, OpenSSL, QuaZip, zlib) — please
report vulnerabilities in those projects to their respective maintainers.

## Known limitations

- **Web features are stubbed.** The embedded browser and web widgets are disabled
  (QtWebEngine is not linked). This eliminates a large class of web-related
  attack vectors.
- **Network access is minimal.** The application does not phone home, does not
  auto-update, and only makes network requests when explicitly triggered by the
  user (e.g., OEmbed lookups).
