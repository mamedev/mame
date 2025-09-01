#!/usr/bin/env python3
##
## license:BSD-3-Clause
## copyright-holders:stonedDiscord

import re
import sys
from pathlib import Path

def is_screaming_snake(name: str):
    return re.fullmatch(r"[A-Z][A-Z0-9_]*", name) is not None

def is_snake_case(name: str):
    return re.fullmatch(r"[a-z][a-z0-9_]*(_[a-z0-9]+)*", name) is not None

def check_file(path: Path, fix: bool = False):
    errors = []
    try:
        raw_bytes = path.read_bytes()
        try:
            text = raw_bytes.decode("utf-8", errors="strict")
        except UnicodeDecodeError as e:
            errors.append((1, f"Invalid UTF-8 sequence at byte {e.start}-{e.end}"))
            return errors
    except Exception:
        return errors

    # Detect non-native line endings
    if sys.platform != "win32" and b"\r\n" in raw_bytes:
        errors.append((1, "File contains Windows (CRLF) line endings on a non-Windows system"))
    elif sys.platform == "win32" and b"\n" in raw_bytes and b"\r\n" not in raw_bytes:
        errors.append((1, "File contains Unix (LF) line endings on Windows"))

    lines = text.splitlines()

    if not text.endswith("\n"):
        if fix:
            path.write_text(text + "\n")
        errors.append((len(lines) or 1, "File should end with a newline"))

    for i, line in enumerate(lines, 1):
        if line.rstrip() != line:
            if fix:
                lines[i - 1] = line.rstrip()
            errors.append((i, "Trailing whitespace detected"))

    return errors

def check_cpp_file(path: Path, fix: bool = False):
    errors = []
    try:
        text = path.read_text()
    except Exception:
        return errors

    lines = text.splitlines()

    for i, line in enumerate(lines, 1):
        hex_pattern = re.compile(r"0x[A-F]+")
        if hex_pattern.search(line):
            errors.append((i, "Hex literals should be lowercase (0x1a not 0x1A)"))

        comment_pattern = re.compile(r"/\*.*\*/")
        if comment_pattern.search(line.strip()):
            errors.append((i, "/* Single-line block comments */ should use // instead"))

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

def check_lst_block(block, changed_cpp_files, start_line, src_file):
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

def check_mame_lst(changed_cpp_files: set[str]):
    path = Path("src/mame/mame.lst")
    errors = check_file(path, False)
    try:
        lines = path.read_text().splitlines()
    except Exception:
        return errors

    current_block = []
    block_start_line = 0
    current_source = None

    for i, line in enumerate(lines):
        if line.startswith("@source:"):
            if current_block:
                errors.extend(check_lst_block(current_block, changed_cpp_files, block_start_line, current_source))
            current_source = "src/mame/" + line[len("@source:"):].strip()
            
            current_block = []
            block_start_line = i + 1
        elif line.strip():
            current_block.append(line.strip())

    if current_block:
        errors.extend(check_lst_block(current_block, changed_cpp_files, block_start_line, current_source))

    return errors

def main():
    fix = "-f" in sys.argv
    args = [f for f in sys.argv[1:] if f != "-f"]

    cpp_files = {f for f in args if f.endswith(".c") or f.endswith(".cpp")}
    h_files = {f for f in args if f.endswith(".h") or f.endswith(".hpp") or f.endswith(".hxx")}

    for file in cpp_files:
        path = Path(file)
        errors = check_cpp_file(path, fix=fix)

        for lineno, msg in errors:
            print(f"{path}:{lineno}: {msg}")

    for file in h_files:
        path = Path(file)
        errors = check_cpp_file(path, fix=fix)

        for lineno, msg in errors:
            print(f"{path}:{lineno}: {msg}")

    errors = check_mame_lst(cpp_files)
    for lineno, msg in errors:
            print(f"src/mame/mame.lst:{lineno}: {msg}")

    sys.exit(0)

if __name__ == "__main__":
    main()