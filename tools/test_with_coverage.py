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
import re

import unittest

cur_dir = os.path.dirname(__file__)
root_dir = re.sub(r"tools.*", "", __file__)
report_dir = os.path.join(root_dir, "_report")


if __name__ == "__main__":
    loader = unittest.TestLoader()
    tests = loader.discover(os.path.join(root_dir, "source", "maya_aceclient", "tests"))

    test_runner = unittest.TextTestRunner(verbosity=2)

    # disable any user-setup side effects
    os.environ['MAYA_SKIP_USERSETUP_PY'] = '1'

    try:
        import coverage  # need to be installed by user
    except ImportError:
        result = test_runner.run(tests)
    else:
        cov = coverage.Coverage(
            data_file=os.path.join(report_dir, '.coverage'),
            include=[os.path.join(root_dir, 'source', 'maya_aceclient', '**')],
        )

        with cov.collect():
            result = test_runner.run(tests)

        cov.report()
        cov.html_report(directory=report_dir)
