#!/usr/bin/env python3
# Open-Sankoré Community Edition
#
# Copyright (C) 2026 David Guyomarch
#
# SPDX-License-Identifier: GPL-3.0-only

"""Update the coverage badge gist with the current coverage percentage."""
import urllib.request
import json
import os
import sys

GIST_ID = '0b042ec6f4db15ba496d37215c18a649'

def main():
    token = os.environ.get('GIST_TOKEN', '')
    if not token:
        print('GIST_TOKEN not set — badge not updated')
        return

    coverage_file = sys.argv[1] if len(sys.argv) > 1 else '/tmp/coverage.json'
    if not os.path.exists(coverage_file):
        print(f'Coverage file not found: {coverage_file}')
        return

    content = open(coverage_file).read()
    data = json.dumps({'files': {'coverage.json': {'content': content}}}).encode()

    req = urllib.request.Request(
        f'https://api.github.com/gists/{GIST_ID}',
        data=data,
        method='PATCH'
    )
    req.add_header('Authorization', f'token {token}')
    req.add_header('Accept', 'application/vnd.github.v3+json')
    req.add_header('Content-Type', 'application/json')

    resp = urllib.request.urlopen(req)
    print(f'Badge gist updated (HTTP {resp.status})')

if __name__ == '__main__':
    main()
