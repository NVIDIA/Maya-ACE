# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import os
import urllib.request
import io
import zipfile
from urllib.parse import quote
import argparse

DHT_EXTERNAL_S3_URL = 'https://d2iqmcaurylv9r.cloudfront.net/'
TARGET_DEPS_ROOT = './_build/target-deps'

class Package:
    def __init__(self, name, version, has_config=True):
        self.name = name
        self.version = version
        self.has_config = has_config

deps = [
    Package('audiofile', '1.1.1-0', False),
    Package('gtest', '1.12.1-pic-a5a89802-windows-x86_64'),
    Package('protobuf', '5.26.0-1-windows-x86_64-static'),
    Package('grpc', '1.65.4-dev.5295+main.e78681f9721c6ac292b2c6600437601bebd30126-windows-x86_64-static'),
    Package('abseil', '20240116.1-0-windows-x86_64-static'),
    Package('openssl', '3.0.12-1-windows-x86_64-static'),
    Package('re2', '20230301-0-windows-x86_64-dynamic'),
    Package('zlib', '1.3.1-0-windows-x86_64-static'),
    Package('c-ares', '1.32.1-dev.5180+main.5d9788307bf02a4036efba61a08ebcba39d4ed1a-windows-x86_64-dynamic'),
]

def pull_and_extract(name, version, config=None):
    extract_dir = os.path.join(TARGET_DEPS_ROOT, name)
    if config:
        extract_dir = os.path.join(extract_dir, config)

    if os.path.exists(extract_dir):
        # silently skip
        return

    full_version = f'{version}-{config}' if config else version
    print(f'Preparing dependency {name}@{full_version}')
    zip_file_path = quote(f'{name}@{full_version}.zip')
    url = f"https://d2iqmcaurylv9r.cloudfront.net/{zip_file_path}"

    print(f'Downloading from {url}')
    with urllib.request.urlopen(url) as response:
        with zipfile.ZipFile(io.BytesIO(response.read()), mode='r') as z:
            z.extractall(extract_dir)

def setup_premake():
    url = 'https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-windows.zip'
    print(f'Downloading from {url}')
    with urllib.request.urlopen(url) as response:
        with zipfile.ZipFile(io.BytesIO(response.read()), mode='r') as z:
            z.extractall('./deps/premake')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", choices=["debug", "release"], default="release", help="Set the configuration mode")

    args = parser.parse_args()

    for p in deps:
        config = None
        if p.has_config:
            config = args.config

        pull_and_extract(p.name, p.version, config)

    setup_premake()

if __name__ == '__main__':
    main()