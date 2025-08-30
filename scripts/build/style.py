#!/usr/bin/env python3
import re
import sys
from pathlib import Path

def is_screaming_snake(name: str):
    return re.fullmatch(r"[A-Z][A-Z0-9_]*", name) is not None

def is_snake_case(name: str):
    return re.fullmatch(r"[a-z][a-z0-9_]*(_[a-z0-9]+)*", name) is not None

def check_cpp_file(path: Path):
    errors = []
    try:
        text = path.read_text()
    except Exception:
        return errors  # skip non-text files

    lines = text.splitlines()

    if not text.endswith("\n"):
        errors.append((len(lines) or 1, "File should end with a newline"))

    for i, line in enumerate(lines, 1):
        hex_pattern = re.compile(r"0x[A-F]+")
        if hex_pattern.search(line):
            errors.append((i, "Hex literals should be lowercase (0x1a not 0x1A)"))

        comment_pattern = re.compile(r"/\*.*\*/")
        if comment_pattern.search(line.strip()):
            errors.append((i, "/* Single-line block comments */ are not allowed, use // instead"))

        macro_pattern = re.compile(r"^\s*#define\s+([A-Za-z0-9_]+)")
        m = macro_pattern.match(line)
        if m and not is_screaming_snake(m.group(1)):
            errors.append((i, f"Macro '{m.group(1)}' should use SCREAMING_SNAKE_CASE"))

        const_pattern = re.compile(r"\bconst\b[^;=()]*\b([A-Za-z_][A-Za-z0-9_]*)\b\s*(?:=|;)")
        c = const_pattern.search(line)
        if c and not is_screaming_snake(c.group(1)):
            errors.append((i, f"Constant '{c.group(1)}' should use SCREAMING_SNAKE_CASE"))

        function_pattern = re.compile(r"\b([a-z][a-z0-9_]*)\s*\(")
        f = function_pattern.search(line)
        if f and not is_snake_case(f.group(1)):
            errors.append((i, f"Function '{f.group(1)}' should use snake_case"))

        class_pattern = re.compile(r"\bclass\s+([A-Za-z0-9_]+)")
        cl = class_pattern.search(line)
        if cl and not is_snake_case(cl.group(1)):
            errors.append((i, f"Class '{cl.group(1)}' should use snake_case"))

        enum_pattern = re.compile(r"\benum\s+(class\s+)?([A-Za-z0-9_]+)")
        en = enum_pattern.search(line)
        if en and not is_snake_case(en.group(2)):
            errors.append((i, f"Enum '{en.group(2)}' should use snake_case"))

    return errors

def check_mame_lst(changed_cpp_files: set[str]):
    errors = []
    try:
        lines = Path("src/mame/mame.lst").read_text().splitlines()
    except Exception:
        return errors

    current_block = []
    block_start_line = 0
    current_source = None

    def check_block(block, start_line, src_file):
        if not src_file or src_file not in changed_cpp_files:
            return []
        sorted_block = sorted(block, key=lambda s: s.lower())
        errors_local = []
        for offset, (expected, actual) in enumerate(zip(sorted_block, block)):
            if expected != actual:
                line_no = start_line + offset + 1
                errors_local.append(
                    (line_no, f"Entry '{actual}' is out of order, expected '{expected}'")
                )
        return errors_local

    for i, line in enumerate(lines):
        if line.startswith("@source:"):
            # clear previous block
            if current_block:
                errors.extend(check_block(current_block, block_start_line, current_source))
            # new block
            current_source = "src/mame/" + line[len("@source:"):].strip()
            
            current_block = []
            block_start_line = i + 1
        elif line.strip():
            current_block.append(line.strip())

    if current_block:
        errors.extend(check_block(current_block, block_start_line, current_source))

    return errors

def main():
    cpp_files = {f for f in sys.argv[1:] if f.endswith(".cpp")}

    for file in sys.argv[1:]:
        path = Path(file)
        errors = check_cpp_file(path)

        for lineno, msg in errors:
            print(f"{path}:{lineno}: {msg}")

    errors = check_mame_lst(cpp_files)
    for lineno, msg in errors:
            print(f"src/mame/mame.lst:{lineno}: {msg}")

    sys.exit(0) # needed so workflow doesn't fail

if __name__ == "__main__":
    main()
