#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import argparse
import io
import os.path
import sys


class ParserBase:
    def process_lines(self, inputfile):
        self.input_line = 1
        for line in inputfile:
            start = 0
            if line.endswith('\n'):
                line = line[:-1]
            used = 0
            while used is not None:
                start += used
                used = self.processors[self.parse_state](line[start:])
            self.input_line += 1


class CppParser(ParserBase):
    TOKEN_LEAD = frozenset(
            [chr(x) for x in range(ord('A'), ord('Z') + 1)] +
            [chr(x) for x in range(ord('a'), ord('z') + 1)] +
            ['_'])
    TOKEN_CONTINUATION = frozenset(
            [chr(x) for x in range(ord('0'), ord('9') + 1)] +
            [chr(x) for x in range(ord('A'), ord('Z') + 1)] +
            [chr(x) for x in range(ord('a'), ord('z') + 1)] +
            ['_'])
    HEXADECIMAL_DIGIT = frozenset(
            [chr(x) for x in range(ord('0'), ord('9') + 1)] +
            [chr(x) for x in range(ord('A'), ord('F') + 1)] +
            [chr(x) for x in range(ord('a'), ord('f') + 1)])

    class Handler:
        def line(self, text):
            pass

        def comment(self, text):
            pass

        def line_comment(self, text):
            pass

    class ParseState:
        DEFAULT = 0
        COMMENT = 1
        LINE_COMMENT = 2
        TOKEN = 3
        STRING_CONSTANT = 4
        CHARACTER_CONSTANT = 5
        NUMERIC_CONSTANT = 6

    def __init__(self, handler, **kwargs):
        super().__init__(**kwargs)
        self.handler = handler
        self.processors = {
                self.ParseState.DEFAULT: self.process_default,
                self.ParseState.COMMENT: self.process_comment,
                self.ParseState.LINE_COMMENT: self.process_line_comment,
                self.ParseState.TOKEN: self.process_token,
                self.ParseState.STRING_CONSTANT: self.process_text,
                self.ParseState.CHARACTER_CONSTANT: self.process_text,
                self.ParseState.NUMERIC_CONSTANT: self.process_numeric }

    def parse(self, inputfile):
        self.parse_state = self.ParseState.DEFAULT
        self.comment_line = None
        self.lead_digit = None
        self.radix = None
        self.line_buffer = ''
        self.comment_buffer = ''
        self.process_lines(inputfile)
        if self.parse_state == self.ParseState.COMMENT:
            raise Exception('unterminated multi-line comment beginning on line %d' % (self.comment_line, ))
        elif self.parse_state == self.ParseState.CHARACTER_CONSTANT:
            raise Exception('unterminated character literal on line %d' % (self.input_line, ))
        elif self.parse_state == self.ParseState.STRING_CONSTANT:
            raise Exception('unterminated string literal on line %d' % (self.input_line, ))

    def process_default(self, line):
        escape = False
        pos = 0
        length = len(line)
        while pos < length:
            ch = line[pos]
            if (ch == '"') or (ch == "'"):
                self.parse_state = self.ParseState.STRING_CONSTANT if ch == '"' else self.ParseState.CHARACTER_CONSTANT
                self.line_buffer += line[:pos + 1]
                return pos + 1
            elif ch == '*':
                if escape:
                    self.parse_state = self.ParseState.COMMENT
                    self.comment_line = self.input_line
                    self.line_buffer += line[:pos - 1] + ' '
                    return pos + 1
            elif ch == '/':
                if escape:
                    self.parse_state = self.ParseState.LINE_COMMENT
                    self.handler.line(self.line_buffer + line[:pos - 1] + ' ')
                    self.line_buffer = ''
                    return pos + 1
            elif ch in self.TOKEN_LEAD:
                self.parse_state = self.ParseState.TOKEN
                self.line_buffer += line[:pos]
                return pos
            elif (ch >= '0') and (ch <= '9'):
                self.parse_state = self.ParseState.NUMERIC_CONSTANT
                self.line_buffer += line[:pos]
                return pos
            escape = ch == '/'
            pos += 1
        if line.endswith('\\'):
            self.line_buffer += line[:-1]
        else:
            self.handler.line(self.line_buffer + line)
            self.line_buffer = ''

    def process_comment(self, line):
        escape = False
        pos = 0
        length = len(line)
        while pos < length:
            ch = line[pos]
            if escape and (ch == '/'):
                self.parse_state = self.ParseState.DEFAULT
                self.comment_line = None
                self.handler.comment(self.comment_buffer + line[:pos - 1])
                self.comment_buffer = ''
                return pos + 1
            escape = ch == '*'
            pos += 1
        if line.endswith('\\'):
            self.comment_buffer += line[:-1]
        else:
            self.comment_buffer += line + '\n'

    def process_line_comment(self, line):
        self.parse_state = self.ParseState.DEFAULT
        self.handler.line_comment(self.comment_buffer + line)
        self.comment_buffer = ''

    def process_token(self, line):
        pos = 0
        length = len(line)
        while pos < length:
            ch = line[pos]
            if ch not in self.TOKEN_CONTINUATION:
                self.parse_state = self.ParseState.DEFAULT
                self.line_buffer += line[:pos]
                return pos
            pos += 1
        self.parse_state = self.ParseState.DEFAULT
        self.handler.line(self.line_buffer + line)
        self.line_buffer = ''

    def process_text(self, line):
        quote = '"' if self.parse_state == self.ParseState.STRING_CONSTANT else "'"
        escape = False
        pos = 0
        length = len(line)
        while pos < length:
            ch = line[pos]
            if (ch == quote) and not escape:
                self.parse_state = self.ParseState.DEFAULT
                self.line_buffer += line[:pos + 1]
                return pos + 1
            escape = (ch == '\\') and not escape
            pos += 1
        if line.endswith('\\'):
            self.line_buffer += line[:-1]
        else:
            t = 'string' if self.ParseState == self.ParseState.STRING_CONSTANT else 'character'
            raise Exception('unterminated %s literal on line %d' % (t, self.input_line))

    def process_numeric(self, line):
        escape = False
        pos = 0
        length = len(line)
        while pos < length:
            ch = line[pos]
            if self.lead_digit is None:
                self.lead_digit = ch
                if ch != '0':
                    self.radix = 10
            elif self.radix is None:
                if ch == "'":
                    if escape:
                        raise Exception('adjacent digit separators on line %d' % (self.input_line, ))
                    else:
                        escape = True
                elif (ch == 'B') or (ch == 'b'):
                    self.radix = 2
                elif (ch == 'X') or (ch == 'x'):
                    self.radix = 16
                elif (ch >= '0') and (ch <= '7'):
                    self.radix = 8
                else:
                    self.parse_state = self.ParseState.DEFAULT # probably an argument to a token-pasting or stringifying macro
            else:
                if ch == "'":
                    if escape:
                        raise Exception('adjacent digit separators on line %d' % (self.input_line, ))
                    else:
                        escape = True
                else:
                    escape = False
                    if self.radix == 2:
                        if (ch < '0') or (ch > '1'):
                            self.parse_state = self.ParseState.DEFAULT
                    elif self.radix == 8:
                        if (ch < '0') or (ch > '7'):
                            self.parse_state = self.ParseState.DEFAULT
                    elif self.radix == 10:
                        if (ch < '0') or (ch > '9'):
                            self.parse_state = self.ParseState.DEFAULT
                    elif self.radix == 16:
                        if ch not in self.HEXADECIMAL_DIGIT:
                            self.parse_state = self.ParseState.DEFAULT
            if self.parse_state == self.ParseState.DEFAULT:
                self.lead_digit = None
                self.radix = None
                self.line_buffer += line[:pos]
                return pos
            pos += 1
        self.parse_state = self.ParseState.DEFAULT
        self.lead_digit = None
        self.radix = None
        self.handler.line(self.line_buffer + line)
        self.line_buffer = ''


class LuaParser(ParserBase):
    class Handler:
        def short_comment(self, text):
            pass

        def long_comment_start(self, level):
            pass

        def long_comment_line(self, text):
            pass

        def long_comment_end(self):
            pass

    class ParseState:
        DEFAULT = 0
        SHORT_COMMENT = 1
        LONG_COMMENT = 2
        STRING_CONSTANT = 3
        LONG_STRING_CONSTANT = 4

    def __init__(self, handler, **kwargs):
        super().__init__(**kwargs)
        self.handler = handler
        self.processors = {
                self.ParseState.DEFAULT: self.process_default,
                self.ParseState.SHORT_COMMENT: self.process_short_comment,
                self.ParseState.LONG_COMMENT: self.process_long_comment,
                self.ParseState.STRING_CONSTANT: self.process_string_constant,
                self.ParseState.LONG_STRING_CONSTANT: self.process_long_string_constant }

    def parse(self, inputfile):
        self.parse_state = self.ParseState.DEFAULT
        self.long_bracket_level = None
        self.escape = False
        self.block_line = None
        self.block_level = None
        self.string_quote = None
        self.process_lines(inputfile)
        if self.parse_state == self.ParseState.LONG_COMMENT:
            raise Exception('unterminated long comment beginning on line %d' % (self.block_line, ))
        if self.parse_state == self.ParseState.STRING_CONSTANT:
            raise Exception('unterminated string literal on line %d' % (self.input_line, ))
        if self.parse_state == self.ParseState.LONG_STRING_CONSTANT:
            raise Exception('unterminated long string literal beginning on line %d' % (self.block_line, ))

    def process_default(self, line):
        pos = 0
        length = len(line)
        while pos < length:
            ch = line[pos]
            if (ch == '"') or (ch == "'"):
                self.string_quote = ch
                self.parse_state = self.ParseState.STRING_CONSTANT
                self.long_bracket_level = None
                self.escape = False
                return pos + 1
            elif (ch == '-') and self.escape:
                self.parse_state = self.ParseState.SHORT_COMMENT
                self.long_bracket_level = None
                self.escape = False
                return pos + 1
            elif self.long_bracket_level is not None:
                if ch == '=':
                    self.long_bracket_level += 1
                elif ch == '[':
                    self.block_line = self.input_line
                    self.block_level = self.long_bracket_level
                    self.parse_state = self.ParseState.LONG_STRING_CONSTANT
                    self.long_bracket_level = None
                    self.escape = False
                    return pos + 1
                else:
                    self.long_bracket_level = None
            elif ch == '[':
                self.long_bracket_level = 0
            self.escape = ch == '-'
            pos += 1
        self.escape = False

    def process_short_comment(self, line):
        pos = 0
        length = len(line)
        while pos < length:
            ch = line[pos]
            if self.long_bracket_level is not None:
                if ch == '=':
                    self.long_bracket_level += 1
                elif ch == '[':
                    self.block_line = self.input_line
                    self.block_level = self.long_bracket_level
                    self.parse_state = self.ParseState.LONG_COMMENT
                    self.long_bracket_level = None
                    self.handler.long_comment_start(self.block_level)
                    return pos + 1
                else:
                    self.long_bracket_level = None
            elif ch == '[':
                self.long_bracket_level = 0
            if self.long_bracket_level is None:
                self.handler.short_comment(line[pos:])
                self.parse_state = self.ParseState.DEFAULT
                return None
            pos += 1
        self.handler.short_comment(line)
        self.parse_state = self.ParseState.DEFAULT

    def process_long_comment(self, line):
        pos = 0
        length = len(line)
        while pos < length:
            ch = line[pos]
            if self.long_bracket_level is not None:
                if ch == '=':
                    self.long_bracket_level += 1
                elif ch == ']':
                    if self.long_bracket_level == self.block_level:
                        if self.parse_state == self.ParseState.LONG_COMMENT:
                            self.handler.long_comment_line(line[:endpos])
                            self.handler.long_comment_end()
                        self.parse_state = self.ParseState.DEFAULT
                        return pos + 1
                    else:
                        self.long_bracket_level = 0
                else:
                    self.long_bracket_level = None
            elif ch == ']':
                endpos = pos
                self.long_bracket_level = 0
            pos += 1
        self.long_bracket_level = None
        self.handler.long_comment_line(line)

    def process_string_constant(self, line):
        pos = 0
        length = len(line)
        while pos < length:
            ch = line[pos]
            if (ch == self.string_quote) and not self.escape:
                self.parse_state = self.ParseState.DEFAULT
                return pos + 1
            self.escape = (ch == '\\') and not self.escape
            pos += 1
        if not self.escape:
            raise Exception('unterminated string literal on line %d' % (self.input_line, ))

    def process_long_string_constant(self, line):
        self.process_long_comment(line) # this works because they're both closed by a matching long bracket


class DriverFilter:
    DRIVER_CHARS = frozenset(
            [chr(x) for x in range(ord('0'), ord('9') + 1)] +
            [chr(x) for x in range(ord('a'), ord('z') + 1)] +
            ['_'])

    def __init__(self, options, **kwargs):
        super().__init__(**kwargs)
        self.parse_filter(options.filter)
        self.parse_list(options.list)

    def write_source(self, f):
        f.write(
                '#include "emu.h"\n' \
                '\n' \
                '#include "drivenum.h"\n' \
                '\n')
        for driver in self.drivers:
            f.write('GAME_EXTERN(%s);\n' % driver)
        f.write(
                '\n' \
                'game_driver const *const driver_list::s_drivers_sorted[%d] =\n' \
                '{\n' % (len(self.drivers), ))
        for driver in self.drivers:
            f.write('\t&GAME_NAME(%s),\n' % driver)
        f.write(
                '};\n' \
                '\n' \
                'std::size_t const driver_list::s_driver_count = %d;\n' % (len(self.drivers), ))

    def parse_filter(self, path):
        def do_parse(p):
            def line_hook(text):
                text = text.strip()
                if text.startswith('#'):
                    do_parse(os.path.join(os.path.dirname(n), text[1:].lstrip()))
                elif text.startswith('+'):
                    text = text[1:].lstrip()
                    if not text:
                        sys.stderr.write('%s:%s: Empty driver name\n' % (p, parser.input_line))
                        sys.exit(1)
                    elif not all(x in self.DRIVER_CHARS for x in text):
                        sys.stderr.write('%s:%s: Invalid character in driver name "%s"\n' % (p, parser.input_line, text))
                        sys.exit(1)
                    includes.add(text)
                    excludes.discard(text)
                elif text.startswith('-'):
                    text = text[1:].lstrip()
                    if not text:
                        sys.stderr.write('%s:%s: Empty driver name\n' % (p, parser.input_line))
                        sys.exit(1)
                    elif not all(x in self.DRIVER_CHARS for x in text):
                        sys.stderr.write('%s:%s: Invalid character in driver name "%s"\n' % (p, parser.input_line, text))
                        sys.exit(1)
                    includes.discard(text)
                    excludes.add(text)
                elif text:
                    sources.add(text)

            n = os.path.normpath(p)
            if n not in filters:
                filters.add(n)
                try:
                    filterfile = io.open(n, 'r', encoding='utf-8')
                except IOError:
                    sys.stderr.write('Unable to open filter file "%s"\n' % (p, ))
                    sys.exit(1)
                with filterfile:
                    handler = CppParser.Handler()
                    handler.line = line_hook
                    parser = CppParser(handler)
                    try:
                        parser.parse(filterfile)
                    except IOError:
                        sys.stderr.write('Error reading filter file "%s"\n' % (p, ))
                        sys.exit(1)
                    except Exception as e:
                        sys.stderr.write('Error parsing filter file "%s": %s\n' % (p, e))
                        sys.exit(1)

        sources = set()
        includes = set()
        excludes = set()
        filters = set()
        if path is not None:
            do_parse(path)
            sys.stderr.write('%d source file(s) found\n' % (len(sources), ))
        self.sources = frozenset(sources)
        self.includes = frozenset(includes)
        self.excludes = frozenset(excludes)

    def parse_list(self, path):
        def do_parse(p):
            def line_hook(text):
                text = text.strip()
                if text.startswith('#'):
                    do_parse(os.path.join(os.path.dirname(n), text[1:].lstrip()))
                elif text.startswith('@'):
                    parts = text[1:].lstrip().split(':', 1)
                    parts[0] = parts[0].strip()
                    if (parts[0] == 'source') and (len(parts) == 2):
                        parts[1] = parts[1].strip()
                        if not parts[1]:
                            sys.stderr.write('%s:%s: Empty source file name "%s"\n' % (p, parser.input_line, text))
                            sys.exit(1)
                        elif self.sources:
                            state['includesrc'] = parts[1] in self.sources
                    else:
                        sys.stderr.write('%s:%s: Unsupported directive "%s"\n' % (p, parser.input_line, text))
                        sys.exit(1)
                elif text:
                    if not all(x in self.DRIVER_CHARS for x in text):
                        sys.stderr.write('%s:%s: Invalid character in driver name "%s"\n' % (p, parser.input_line, text))
                        sys.exit(1)
                    elif state['includesrc'] and (text not in self.excludes):
                        drivers.add(text)

            n = os.path.normpath(p)
            if n not in lists:
                lists.add(n)
                try:
                    listfile = io.open(n, 'r', encoding='utf-8')
                except IOError:
                    sys.stderr.write('Unable to open list file "%s"\n' % (p, ))
                    sys.exit(1)
                with listfile:
                    handler = CppParser.Handler()
                    handler.line = line_hook
                    parser = CppParser(handler)
                    try:
                        parser.parse(listfile)
                    except IOError:
                        sys.stderr.write('Error reading list file "%s"\n' % (p, ))
                        sys.exit(1)
                    except Exception as e:
                        sys.stderr.write('Error parsing list file "%s": %s\n' % (p, e))
                        sys.exit(1)

        lists = set()
        drivers = set()
        state = object()
        state = { 'includesrc': True }
        do_parse(path)
        for driver in self.includes:
            drivers.add(driver)
        sys.stderr.write('%d driver(s) found\n' % (len(drivers), ))
        drivers.add('___empty')
        self.drivers = sorted(drivers)


def split_path(path):
    path = os.path.normpath(path)
    result = [ ]
    while True:
        dirname, basename = os.path.split(path)
        if dirname == path:
            result.insert(0, dirname)
            return result
        elif basename == path:
            result.insert(0, basename)
            return result
        else:
            result.insert(0, basename)
            path = dirname


def parse_command_line():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(title='commands', dest='command', metavar='<command>')

    subparser = subparsers.add_parser('sourcesproject', help='generate project directives for source files')
    subparser.add_argument('-r', '--root', metavar='<srcroot>', default='.', help='path to emulator source root (defaults to working directory)')
    subparser.add_argument('-t', '--target', metavar='<target>', required=True, help='generated emulator target name')
    subparser.add_argument('sources', metavar='<srcfile>', nargs='+', help='source files to include')

    subparser = subparsers.add_parser('sourcesfilter', help='generate driver filter for source files')
    subparser.add_argument('sources', metavar='<srcfile>', nargs='+', help='source files to include')

    subparser = subparsers.add_parser('driverlist', help='generate driver list source')
    subparser.add_argument('-f', '--filter', metavar='<fltfile>', help='input filter file')
    subparser.add_argument('list', metavar='<lstfile>', help='input list file')

    return parser.parse_args()


def collect_lua_directives(options):
    def short_comment_hook(text):
        if text.startswith('@'):
            name, action = text[1:].rstrip().rsplit(',', 1)
            if name not in result:
                result[name] = [ ]
            result[name].append(action)

    base = os.path.join(options.root, 'scripts', 'src')
    result = { }
    handler = LuaParser.Handler()
    handler.short_comment = short_comment_hook
    parser = LuaParser(handler)
    for name in ('bus', 'cpu', 'machine', 'sound', 'video', 'formats'):
        path = os.path.join(base, name + '.lua')
        try:
            f = io.open(path, 'r', encoding='utf-8')
        except IOError:
            sys.stderr.write('Unable to open source file "%s"\n' % (path, ))
            sys.exit(1)
        try:
            with f:
                parser.parse(f)
        except IOError:
            sys.stderr.write('Error reading source file "%s"\n' % (path, ))
            sys.exit(1)
        except Exception as e:
            sys.stderr.write('Error parsing source file "%s": %s\n' % (path, e))
            sys.exit(1)
    return result


def scan_source_dependencies(options):
    def locate_include(path):
        split = [ ]
        forward = 0
        reverse = 0
        for part in path.split('/'):
            if part and (part != '.'):
                if part != '..':
                    forward += 1
                    split.append(part)
                elif forward:
                    split.pop()
                    forward -= 1
                else:
                    split.append(part)
                    reverse += 1
        split = tuple(split)
        for incdir, depth in roots:
            if (not depth) or (not reverse):
                components = incdir + split
                depth = depth + forward - 1
            elif depth >= reverse:
                components = incdir[:-reverse] + split[reverse:]
                depth = depth + forward - reverse - 1
            else:
                components = incdir[:-depth] + split[depth:]
                depth = forward - 1
            if os.path.isfile(os.path.join(options.root, *components)):
                return components, depth
        return None, 0

    def test_siblings(relative, basename, depth):
        pathbase = '/'.join(relative) + '/'
        dirname = os.path.join(options.root, *relative)
        for ext in ('.cpp', '.ipp', '.hxx'):
            path = pathbase + basename + ext
            if (path not in seen) and os.path.isfile(os.path.join(dirname, basename + ext)):
                remaining.append((path, depth))
                seen.add(path)

    def line_hook(text):
        text = text.lstrip()
        if text.startswith('#'):
            text = text[1:].lstrip()
            if text.startswith('include'):
                text = text[7:]
                if text[:1].isspace():
                    text = text.strip()
                    if (len(text) > 2) and (text[0] == '"') and (text[-1] == '"'):
                        components, depth = locate_include(text[1:-1])
                        if components:
                            path = '/'.join(components)
                            if path not in seen:
                                remaining.append((path, depth))
                                seen.add(path)
                                base, ext = os.path.splitext(components[-1])
                                if ext.lower().startswith('.h'):
                                    components = components[:-1]
                                    test_siblings(components, base, depth)
                                    if components == ('src', 'mame', 'includes'):
                                        for aspect in ('audio', 'drivers', 'video', 'machine'):
                                            test_siblings(('src', 'mame', aspect), base, depth)

    handler = CppParser.Handler()
    handler.line = line_hook
    parser = CppParser(handler)
    seen = set('/'.join(x for x in split_path(source) if x) for source in options.sources)
    remaining = list([(x, 0) for x in seen])
    default_roots = ((('src', 'devices'), 0), (('src', 'mame'), 0), (('src', 'lib'), 0))
    while remaining:
        source, depth = remaining.pop()
        components = tuple(source.split('/'))
        roots = ((components[:-1], depth), ) + default_roots
        try:
            f = io.open(os.path.join(options.root, *components), 'r', encoding='utf-8')
        except IOError:
            sys.stderr.write('Unable to open source file "%s"\n' % (source, ))
            sys.exit(1)
        try:
            with f:
                parser.parse(f)
        except IOError:
            sys.stderr.write('Error reading source file "%s"\n' % (source, ))
            sys.exit(1)
        except Exception as e:
            sys.stderr.write('Error parsing source file "%s": %s\n' % (source, e))
            sys.exit(1)
    return seen


def write_project(options, projectfile, mappings, sources):
    targetsrc = ''
    for source in sorted(sources):
        action = mappings.get(source)
        if action:
            for line in action:
                projectfile.write(line + '\n')
        if source.startswith('src/mame/'):
            targetsrc += '        MAME_DIR .. "%s",\n' % (source, )
    projectfile.write(
            '\n' \
            'function createProjects_mame_%s(_target, _subtarget)\n' \
            '    project ("mame_%s")\n' \
            '    targetsubdir(_target .."_" .. _subtarget)\n' \
            '    kind (LIBTYPE)\n' \
            '    uuid (os.uuid("drv-mame-%s"))\n' \
            '    addprojectflags()\n' \
            '    \n' \
            '    includedirs {\n' \
            '        MAME_DIR .. "src/osd",\n' \
            '        MAME_DIR .. "src/emu",\n' \
            '        MAME_DIR .. "src/devices",\n' \
            '        MAME_DIR .. "src/mame",\n' \
            '        MAME_DIR .. "src/lib",\n' \
            '        MAME_DIR .. "src/lib/util",\n' \
            '        MAME_DIR .. "src/lib/netlist",\n' \
            '        MAME_DIR .. "3rdparty",\n' \
            '        GEN_DIR  .. "mame/layout",\n' \
            '        ext_includedir("flac"),\n' \
            '        ext_includedir("glm"),\n' \
            '        ext_includedir("jpeg"),\n' \
            '        ext_includedir("rapidjson"),\n' \
            '        ext_includedir("zlib"),\n' \
            '    }\n' \
            '\n' \
            '    files{\n%s' \
            '    }\n' \
            'end\n' \
            '\n' \
            'function linkProjects_mame_%s(_target, _subtarget)\n' \
            '    links {\n' \
            '        "mame_%s",\n' \
            '    }\n' \
            'end\n' % (options.target, options.target, options.target, targetsrc, options.target, options.target))


def write_filter(options, filterfile):
    drivers = set()
    for source in options.sources:
        components = tuple(x for x in split_path(source) if x)
        if (len(components) > 3) and (components[:3] == ('src', 'mame', 'drivers')):
            ext = os.path.splitext(components[-1])[1].lower()
            if ext.startswith('.c'):
                drivers.add('/'.join(components[3:]))
    for driver in sorted(drivers):
        filterfile.write(driver + '\n')


if __name__ == '__main__':
    options = parse_command_line()
    if options.command == 'sourcesproject':
        header_to_optional = collect_lua_directives(options)
        source_dependencies = scan_source_dependencies(options)
        write_project(options, sys.stdout, header_to_optional, source_dependencies)
    elif options.command == 'sourcesfilter':
        write_filter(options, sys.stdout)
    elif options.command == 'driverlist':
        DriverFilter(options).write_source(sys.stdout)
