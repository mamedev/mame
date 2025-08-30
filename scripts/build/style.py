#!/usr/bin/env python3
import re
import sys
from pathlib import Path

def check_file(path: Path):
    errors = []
    try:
        text = path.read_text()
    except Exception:
        return errors  # skip non-text files

    # Must end with newline
    if not text.endswith("\n"):
        errors.append((len(text.splitlines()) or 1, "File must end with a newline"))

    # Hex literals lowercase
    hex_pattern = re.compile(r"0x[A-F]+")
    for i, line in enumerate(text.splitlines(), 1):
        if hex_pattern.search(line):
            errors.append((i, "Hex literals must be lowercase"))

    # No single-line C-style comments
    comment_pattern = re.compile(r"/\*.*\*/")
    for i, line in enumerate(text.splitlines(), 1):
        if comment_pattern.search(line):
            errors.append((i, "Single-line block comments (/* ... */) are not allowed"))

    return errors

def main():
    failed = False
    for file in sys.argv[1:]:
        path = Path(file)
        for lineno, msg in check_file(path):
            print(f"{path}:{lineno}: {msg}")
            failed = True
    sys.exit(1 if failed else 0)

if __name__ == "__main__":
    main()
