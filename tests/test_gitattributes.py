import unittest
from pathlib import Path


class GitAttributesTest(unittest.TestCase):
    def test_bat_files_use_crlf(self):
        gitattributes = Path(__file__).resolve().parent.parent / ".gitattributes"
        self.assertTrue(gitattributes.exists(), ".gitattributes must exist")
        for line in gitattributes.read_text().splitlines():
            stripped = line.strip()
            if stripped.startswith("*.bat"):
                self.assertIn("eol=crlf", stripped, ".bat files must be checked out with CRLF")
                self.assertNotIn("eol=lf", stripped)


if __name__ == "__main__":
