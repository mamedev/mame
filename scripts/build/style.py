#!/usr/bin/env python3
##
## license:BSD-3-Clause
## copyright-holders:stonedDiscord

import json, re, sys, subprocess
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
            errors.append((i, "Lines should not have trailing whitespaces"))

    return errors

LICENSE_RE = re.compile(r'^//\s*license:')
COPYRIGHT_RE = re.compile(r'^//\s*copyright-holders:')

def check_spdx_header(lines):
    errors = []
    i = 0

    # skip blank lines
    while i < len(lines) and not lines[i].strip():
        i += 1

    if i >= len(lines) or not LICENSE_RE.match(lines[i]):
        errors.append((i + 1, "Missing or incorrect // license: header"))

    i += 1
    while i < len(lines) and not lines[i].strip():
        i += 1

    if i >= len(lines) or not COPYRIGHT_RE.match(lines[i]):
        errors.append((i + 1, "Missing or incorrect // copyright-holders: header"))

    return errors

def check_includes(path: Path, lines):
    errors = []
    includes = []
    for i, line in enumerate(lines):
        stripped = line.strip()
        if stripped.startswith('#include'):
            includes.append((i + 1, stripped))

    if not includes:
        return errors

    def get_group(include_line):
        m = re.match(r'#include\s+["<]([^">]+)[">]', include_line)
        if not m:
            return None
        inc_path = m.group(1)
        if inc_path == 'emu.h':
            return 0
        if inc_path == 'logmacro.h':
            return 5
        if include_line.endswith('>'):
            return 3
        else:
            # " includes
            if '/' in inc_path:
                return 1
            else:
                if inc_path.endswith('.lh'):
                    return 4
                else:
                    return 2

    include_data = []
    for line_no, inc_line in includes:
        group = get_group(inc_line)
        if group is None:
            continue
        m = re.match(r'#include\s+["<]([^">]+)[">]', inc_line)
        inc_path = m.group(1)
        include_data.append((group, inc_path, line_no))

    expected = sorted(include_data, key=lambda x: (x[0], x[1].lower()))
    for (exp_group, exp_path, _), (act_group, act_path, act_line) in zip(expected, include_data):
        if exp_group != act_group or exp_path != act_path:
            errors.append((act_line, f"Include '{act_path}' is out of order, expected '{exp_path}'"))
            break

    # check for blank lines between groups
    for i in range(len(includes) - 1):
        line_no1, inc_line1 = includes[i]
        line_no2, inc_line2 = includes[i + 1]
        group1 = get_group(inc_line1)
        group2 = get_group(inc_line2)
        if group1 != group2:
            # check if there is at least one blank line between line_no1 and line_no2
            has_blank = any(lines[j].strip() == '' for j in range(line_no1, line_no2))
            if not has_blank:
                errors.append((line_no2, "Missing blank line between include groups"))

    return errors

def check_rom_regions(lines):
    errors = []
    rom_start_pattern = re.compile(r'^\s*ROM_START\s*\(')
    rom_end_pattern = re.compile(r'^\s*ROM_END\s*$')
    rom_region_pattern = re.compile(r'^\s*ROM_REGION')

    inside_rom = False
    first_region = True

    for i, line in enumerate(lines, 1):
        if rom_start_pattern.match(line):
            inside_rom = True
            first_region = True
        elif rom_end_pattern.match(line):
            inside_rom = False
            # Check if next line is blank (unless end of file)
            if i < len(lines) and lines[i].strip() != '':
                errors.append((i + 1, "ROM_END should be followed by a blank line"))
        elif inside_rom and rom_region_pattern.match(line):
            if first_region:
                # First ROM_REGION should not have a blank line before it
                if i > 0 and lines[i - 1].strip() == '':
                    errors.append((i, "First ROM_REGION should not have a blank line before it"))
                first_region = False
            else:
                # Subsequent ROM_REGION should have a blank line before it
                if i <= 1 or lines[i - 2].strip() != '':
                    errors.append((i, "ROM_REGION should have a blank line before it"))

    return errors

def check_cpp_file(path: Path, fix: bool = False):
    errors = check_file(path, fix)

    try:
        text = path.read_text()
    except Exception:
        return errors

    lines = text.splitlines()

    # SPDX header
    errors.extend(check_spdx_header(lines))

    # include order
    errors.extend(check_includes(path, lines))

    # normal style checks
    for i, line in enumerate(lines, 1):

        if re.search(r"0x[A-F]+", line):
            errors.append((i, "Hex literals should be lowercase (0x1a not 0x1A)"))

        if re.search(r"/\*.*\*/", line.strip()):
            errors.append((i, "/* Single-line block comments */ should use // instead"))

        m = re.match(r"^\s*#define\s+([A-Za-z0-9_]+)", line)
        if m and not is_screaming_snake(m.group(1)):
            errors.append((i, f"Macro '{m.group(1)}' should use SCREAMING_SNAKE_CASE"))

        c = re.search(r"\bconst\b[^;=()]*\b([A-Za-z_][A-Za-z0-9_]*)\b\s*(?:=|;)", line)
        if c and not is_screaming_snake(c.group(1)):
            errors.append((i, f"Constant '{c.group(1)}' should use SCREAMING_SNAKE_CASE"))

        f = re.search(r"\b([a-z][a-z0-9_]*)\s*\(", line)
        if f and not is_snake_case(f.group(1)):
            errors.append((i, f"Function '{f.group(1)}' should use snake_case"))

        cl = re.search(r"\bclass\s+([A-Za-z0-9_]+)", line)
        if cl and not is_snake_case(cl.group(1)):
            errors.append((i, f"Class '{cl.group(1)}' should use snake_case"))

        en = re.search(r"\benum\s+(class\s+)?([A-Za-z0-9_]+)", line)
        if en and not is_snake_case(en.group(2)):
            errors.append((i, f"Enum '{en.group(2)}' should use snake_case"))

    # ROM region whitespace checks
    errors.extend(check_rom_regions(lines))

    return errors

def natural_sort(l): 
    convert = lambda text: int(text) if text.isdigit() else text.lower()
    alphanum_key = lambda key: [convert(c) for c in re.split('([0-9]+)', key)]
    return sorted(l, key=alphanum_key)

def check_lst_block(block, changed_cpp_files, start_line, src_file):
    if not src_file or src_file not in changed_cpp_files:
        return []
    sorted_block = natural_sort(block)
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

def print_review(path, lineno, msg):
    review = {"body": str(msg), "path": str(path), "line": int(lineno)}
    print(json.dumps(review))

def get_changed_lines(base_sha, file_path):
    if not base_sha:
        return None
    try:
        result = subprocess.run(['git', 'diff', '--unified=0', base_sha, 'HEAD', '--', file_path], capture_output=True, text=True)
        if result.returncode != 0:
            return None
        changed_lines = set()
        lines = result.stdout.splitlines()
        for line in lines:
            if line.startswith('@@'):
                m = re.match(r'@@ -\d+,\d+ \+(\d+),(\d+) @@', line)
                if m:
                    new_start = int(m.group(1))
                    new_count = int(m.group(2))
                    for i in range(new_count):
                        changed_lines.add(new_start + i)
        return changed_lines
    except Exception:
        return None

def get_changed_files(base_sha):
    if not base_sha:
        return set()
    try:
        result = subprocess.run(['git', 'diff', '--name-only', '--diff-filter=ACMRT', base_sha, 'HEAD'], capture_output=True, text=True)
        if result.returncode != 0:
            return set()
        files = set(result.stdout.strip().split('\n'))
        return files
    except Exception:
        return set()

def main():
    fix = "-f" in sys.argv
    args = sys.argv[1:]
    if fix:
        args.remove("-f")

    base_sha = None
    if args and args[0] == '--base-sha':
        base_sha = args[1]
        args = args[2:]

    if base_sha and not args:
        # Find changed files
        changed_files = get_changed_files(base_sha)
        cpp_files = {f for f in changed_files if f.endswith((".c", ".cpp"))}
        h_files = {f for f in changed_files if f.endswith((".h", ".hpp", ".hxx", ".ipp"))}
        other_files = {f for f in changed_files if f.endswith((".py", ".lua", ".mm", ".lay", ".lst"))}
    else:
        # Use provided files
        cpp_files = {f for f in args if f.endswith((".c", ".cpp"))}
        h_files = {f for f in args if f.endswith((".h", ".hpp", ".hxx", ".ipp"))}
        other_files = {f for f in args if f.endswith((".py", ".lua", ".mm", ".lay", ".lst"))}

    all_files = cpp_files | h_files | other_files

    for file in all_files:
        changed_lines = get_changed_lines(base_sha, file)
        path = Path(file)
        if file in cpp_files | h_files:
            errors = check_cpp_file(path, fix)
        else:
            errors = check_file(path, fix)
        for lineno, msg in errors:
            if changed_lines is None or lineno in changed_lines:
                print_review(path, lineno, msg)

    changed_lines_lst = get_changed_lines(base_sha, "src/mame/mame.lst")
    for lineno, msg in check_mame_lst(cpp_files):
        if changed_lines_lst is None or lineno in changed_lines_lst:
            print_review("src/mame/mame.lst", lineno, msg)

    sys.exit(0)

if __name__ == "__main__":
    main()
