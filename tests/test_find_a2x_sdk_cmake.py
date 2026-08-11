import unittest
from pathlib import Path


class FindA2XSDKCMakeTest(unittest.TestCase):
    def test_no_root_dir_typo(self):
        cmake_file = Path(__file__).resolve().parent.parent / "cmake/modules/FindA2X_SDK.cmake"
        self.assertTrue(cmake_file.exists())
        content = cmake_file.read_text()
        self.assertNotIn("mark_as_advanced(A2X_SDK_ROOT_DIR", content)
        self.assertIn("mark_as_advanced(A2X_SDK_ROOT", content)

    def test_a2x_sdk_root_path_quoted(self):
        cmake_file = Path(__file__).resolve().parent.parent / "cmake/modules/FindA2X_SDK.cmake"
        self.assertTrue(cmake_file.exists())
        content = cmake_file.read_text()
        self.assertIn(
            'file(READ "${A2X_SDK_ROOT}/VERSION.md" A2X_SDK_VERSION_CONTENTS)',
            content,
            "A2X_SDK_ROOT must be quoted to handle install paths with spaces",
        )


if __name__ == "__main__":
