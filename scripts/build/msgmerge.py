#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-
#
# Copyright Terje Røsten <terjeros@phys.ntnu.no> Nov. 2003.
# 
'''Merge two Uniforum style .po files together.

This is a implementation (not complete) in Python of the GNU
msgmerge(1) program. It can be used on the command line (or as a Python
module).

Usage: msgmerge.py [OPTIONS] def.po ref.pot

The def.po file is an existing PO file with translations. The ref.pot
file is the last created PO file with up-to-date source references but
old translations, or a PO Template file.

Options:
  -U, --update           update def.po,
                         do nothing if def.po is already up to date.
  -o, --output-file=FILE write output to file FILE. Output is written to
                         stdout if set to - or if the option is not present.
  -D, --docstrings       don\'t remove docstring flag.
  -h, --help             display help text and exit.
  -V, --version          display version and exit.
  -q, --quiet, --silent  suppress progress indicators.
'''
from __future__ import generators

if not __name__ == '__main__':
    __doc__ += '''\

When used as module the interesting functions are merge() and
merge_dir().

The merge() function does the same as the command line version, and
the arguments are as follows. The first argument is the def.po file,
then the ref.pot file. The third argument controls whether do work in
update mode or not, then the next argument sets the output file. Set
the next argument to False to remove docstring flags. The last
argument can be used to suppress progress indicators. The default is
to work in update mode with progress indicators.

Example:
 merge("def.po", "ref.pot")
  merge the files def.po and ref.pot and write output to def.po if
  there are any changes.
 merge("def.po", "red.pot", docstrings = False, verbose = False,
       update = False, outfile = "-")
  merge the files def.po and ref.pot and write output to stdout,
  remove docstring flag and be quiet.

The merge_dir() function is useful when merging a directory of po
files. The only required argument is the name of the directory with po
files and the pot file. It will use simple glob to find the files. The
second argument can be used to specify the pot file (in the
directory). Third argument is a list of po files (then globbing will
not be used) and the next argument is list of filename to exclude. The
last argument can be used to suppress progress indicators. Docstring
flag will not be removed.

Example:
 merge_dir("po")
  merge (and update) all po files in directory po with the single pot
  file in the same directory.

The module raises the MsgmergeError exception in case of error.
'''
__revision__ = '$Id: msgmerge.py,v 1.41 2003/11/18 19:10:42 terjeros Exp $'
__version__ = '0.1'
name = 'msgmerge.py'

__all__ = [ 'merge', 'merge_dir', 'MsgmergeError' ]

import sys
import re
import string
import getopt
import difflib
import glob
import os.path
import codecs

try:
    True, False
except NameError:
    True, False = 1, 0

class Msgs:
    '''Class to hold information about messages.'''
    width = 80
    file = ''
    def __init__(self, msgid, msgstr, flag, lno, entry, **kwds):
        self.id = msgid
        self.str = msgstr
        self.cmt = kwds.get('cmt', '')
        self.ref = kwds.get('ref', '')
        self.autocmt = kwds.get('autocmt', '')
        self.flag = flag
        self.entry = entry
        self.lno = lno
        self.count = 0
    def wash(self):
        self.id = wash(self.id, width = self.width,
                       filename = self.file, lno = self.lno)
        self.str = wash(self.str, 'msgstr', width = self.width,
                        filename = self.file, lno = self.lno)
    def used(self):
        self.count += 1
    def get_clean_id(self):
        return self.id.replace('msgid "','', 1)
    def obsolete(self):
        self.width -= len('#~ ')
        self.wash()
        t = [ '#~ %s\n' % s for s in self.id.splitlines() ]
        self.id = ''.join(t)
        t = [ '#~ %s\n' % s for s in self.str.splitlines() ]
        self.str = ''.join(t)

class Options:
    '''Class to hold options'''
    def __init__(self, cmdline = False, **kwds):
        if not cmdline:
            self.update = kwds.get('update', True)
            self.outfile = kwds.get('outfile', '-')
            self.docstrings = kwds.get('docstrings', True)
            self.verbose = kwds.get('verbose', False)
            self.suffix = kwds.get('suffix', '~')
            self.backup = kwds.get('backup', True)
        else:
            self.update = False
            self.outfile = False
            self.docstrings = False
            self.verbose = True
            self.suffix = '~'
            self.backup = True

class MsgmergeError(Exception):
    '''Exception class for msgmerge'''

def gen(lines):
    '''
    Generator which returns a line (with the obsolete prefix removed)
    from the list of lines in <lines>, the line number is also
    returned.
    '''
    lno = 0
    for l in lines:
        lno += 1
        yield l.replace('#~ ', '', 1), lno
    yield l, lno

def slurp(s, g, sign):
    '''
    The string returned from iterator <g>\'s next() method is added to
    the string <s> if string returned is beginning with the string
    <sign>. The return value is the first returned string which do not
    start with <sign>, the line number, the iterator <g> and the
    (possibly) updated string <s>.
    '''
    l, lno = g.next()
    while l.startswith(sign) or (sign == '# ' and l.strip() == '#'):
        s += l
        l, lno = g.next()
    return l, lno, g, s

def splitted_fit(chunk, line, width, break_always, break_after_space):
    '''
    Check if string <chunk> can be splitted by newline to fit into
    string <line> with width smaller than <width>. The return value is
    a tuple where the first element is the part of chunk which fits
    and the second element is the rest of chunk.
    '''
    ret = '', chunk
    l = len(chunk)
    for i in range(l - 1, -1, -1):
        if chunk[i] in break_always and len(chunk[0:i] + line) <= width:
            ret = chunk[0:i], chunk[i:]
            break
        elif chunk[i] in break_after_space and i and chunk[i-1].strip() == '':
            ret = chunk[0:i], chunk[i:]
            break
        elif chunk[i] == '\\' and len(chunk[i:]) > 1 and chunk[i+1] == '"' \
             and len(chunk[0:i] + line) <= width:
            ret = chunk[0:i], chunk[i:]
            break
    return ret

def wrap(msg, width):
    '''
    Accept a list <msg> of strings to wrap, each string is wrapped to
    width <width> and surrounded with a pair of ". The return value is
    a string with these wrapped strings joined together with newlines.
    '''
    if msg.isspace() or not msg:
        return '"%s"' % msg

    # \ and " is here, but " is special in po files.
    break_always = '$%+({['
    # XXX what about: « © » ¦ § etc?
    break_after_space = '_-=^`~\'<|>&*#@'
    enders = '.:,;!?/])}|%-'
    extra = string.punctuation
    for c in enders:
        extra = extra.replace(c, '')
    escaped = { 'enders' : re.escape(enders),
                'extra'  : re.escape(extra) }
    regex = r'([\w%(extra)s]*[\s%(enders)s)]+[\s%(enders)s]*)' % escaped
    r = re.compile(regex, re.UNICODE)
    msg = [ m for m in r.split(msg) if not m == '']

    lines = []
    line = msg.pop(0)
    
    # Handle \n on end of line
    if len(msg) > 1 and msg[-1] == 'n' and len(msg[-2]) > 0 \
           and msg[-2][-1] == '\\':
        msg[-2] += msg[-1]
        msg.pop()
    # Do not allow a single \n on a line
    if len(msg) > 2 and msg[-1] == '\\n':
        msg[-2] += msg[-1]
        msg.pop()

    for m in msg:
        if len(line) > width or len(m) > width or len(line + m) > width:
            fit, rest = splitted_fit(m, line, width, break_always,
                                     break_after_space)
            line += fit
            lines.append(line)
            line = rest
        else:
            line += m
    lines.append(line)
    lines = [ '"%s"' % l for l in lines ]
    return '\n'.join(lines)

def normalize(lines):
    '''
    Normalize <lines>: e.g "\n\nText\n\n" becomes:
    "\n"
    "\n"
    "Text\n"
    "\n"
    '''
    if  0 < lines.find('\\n') < len(lines) - 3:
        if lines[-3:] == '\\n"':    
            lines = lines[:-3].replace('\\n','\\n"\n"').replace('""\n','') \
                    + '\\n"'
        else:
            lines = lines.replace('\\n','\\n"\n"').replace('""\n','')
    return lines

def wash(msg, idx = 'msgid', width = 80, **kwds):
    '''
    Do washing on the msgstr or msgid fields. Wrap the text to fit in
    width <width>. <msg> is a list of lines that makes up the field.
    <idx> indicate msgid or msgstr, <width> holds the width. <filename>
    and <lno> (line number) is picked up from <kwds>.
    Returns the washed field as a string.
    '''
    msg = normalize(msg)
    lines = msg.splitlines()
    size = len(lines)
    if size > 1 or len(msg) > width:
        washed = []
        # The first line is special
        m = re.match('^%s "(.*)"$' % (idx, ), lines[0])
        if not m:
            print lines[0]
            kwds['lno'] -= size + 1            
            raise MsgmergeError('parse error: %(filename)s:%(lno)s.'
                                % kwds)
        washed.append(m.group(1))
        if m.group(1).endswith(r'\n'):
            washed.append('')
        i = 0
        for line in lines[1:]:
            m = re.match('^"(\s*.*)"$', line)
            i += 1
            if not m:
                print line
                kwds['lno'] -= size - i + 1
                raise MsgmergeError('parse error: %(filename)s:%(lno)s.'
                                    % kwds)
            washed[-1] += m.group(1)
            if m.group(1).endswith(r'\n'):
                washed.append('')
        if washed[0] == '':
            washed.pop(0)
        if washed[-1] == '':
            washed.pop()
    
        washed = [ wrap(w, width - 3) for w in washed ] # " and \n removed.

        # One line or multiline
        if len(washed) == 1 and len('%s %s\n' % (idx, washed[0])) < width:
            washed = '%s %s\n' % (idx, washed[0])
        else:
            washed = '%s ""\n%s\n' % (idx, '\n'.join(washed))
    else:
        washed = msg

    return washed

def parse(filename, entry):
    '''
    Parse po or pot file with name <filename>. Set the variable
    <entry> to msgid/msgstr to indicate pot/po file.  The return value
    is a dict with msgid (washed) as key and Msgs instances as
    values.
    '''
    lines = io(filename).readlines()
    Msgs.file = filename
    messages = {}
    last = len(lines)
    g = gen(lines)            
    cmt = autocmt = ref = flag = ''
    msgid = False
    lno = 0
    while not lno == last:
        l, lno = g.next()
        if l.startswith('# '):
            l, lno, g, cmt  = slurp(l, g, '# ')
        if l.startswith('#.'):
            l, lno, g, autocmt = slurp(l, g, '#.')
        if l.startswith('#:'):
            l, lno, g, ref = slurp(l, g, '#:')
        if l.startswith('#,'):
            l, lno, g, flag = slurp(l, g, '#,')
        if l.startswith('msgid'):
            l, lno, g, msgid = slurp(l, g, '"')
        if l.startswith('msgstr'):
            l, lno, g, msgstr = slurp(l, g, '"')

        if not lno == last and not l.strip() == '':
            raise MsgmergeError('parse error: %s:%s.' % (filename, lno))

        if msgid and entry == 'msgstr':
            idx = wash(msgid, filename = filename, lno = lno)
            messages[idx] = Msgs(msgid, msgstr, flag, lno, entry, cmt = cmt)
            msgid = False; msgstr = cmt = autocmt = ref = flag = ''
        elif msgid and entry == 'msgid':
            idx = wash(msgid, filename = filename, lno = lno)
            messages[idx] = Msgs(msgid, msgstr, flag, lno, entry,
                                 autocmt = autocmt, ref = ref)
            msgid = False; msgstr = cmt = autocmt = ref = flag = ''

    for m in messages.values():
        m.wash()
    return messages

def fuzzy_match(pot, defs):
    '''
    Try to find the best difflib match (with ratio > 0.6) between
    id of Msgs object <pot> and Msgs in the dict <defs>.
    Return value is the Msgs object in <defs> with highest ratio,
    False is returned if no suitable Msgs is found.
    '''
    limit = 0.6
    l, po = limit - 0.01, False
    s = difflib.SequenceMatcher(lambda x: x == ' "', '', pot.get_clean_id())
    len2 = len(pot.get_clean_id())   
    for candidate in defs.values():
        if candidate.str == 'msgstr ""\n':       # Empty translation
            continue
        if candidate.id == 'msgid ""\n':         # Empty msgid (header)
            continue
        len1 = len(candidate.get_clean_id())
        if len2 > 2 * len1 or len1 > 1.5 * len2: # Simple and fast tests first
            continue
        s.set_seq1(candidate.get_clean_id())
        if s.quick_ratio() < l:
            continue
        r = s.ratio()                            # This is expensive
        if r > l:
            l, po = r, candidate
    return po

def flags(po, pot, fuzzy = False, obs = False):
    '''
    Create flag field from flag field in Msgs objects <po> and
    <pot>. When <fuzzy> is true <po>\'s flags are ignored and the
    fuzzy flag is added. If <obs> is set then most flags but fuzzy are
    removed. If the global variable option.docstrings is set then
    docstring flags will not be removed. The return value is a string
    which holds the combined flag.
    '''
    global option
    flag = ''
    if po.flag or pot.flag or fuzzy:
        if not fuzzy:
            flag = '%s, %s' % (po.flag.strip(), pot.flag.strip())
        else:
            flag = '%s, %s' % ('#, fuzzy', pot.flag.strip())
        flag = flag.split(', ')
        fl = {}
        flag = [fl.setdefault(f, f) for f in flag if f not in fl and f]
        if not option.docstrings:
            try:
                flag.remove('docstring')
            except ValueError:
                pass
        if obs:
            removes = ['c-format', 'python-format', 'docstring']
            for remove in removes:
                try:
                    flag.remove(remove)
                except ValueError:
                    pass
        # Put fuzzy first
        if 'fuzzy' in flag and not flag.index('fuzzy') == 1:
            i = flag.index('fuzzy')
            flag[1], flag[i] = flag[i], flag[1]

        if len(flag) == 1:
            flag = ''
        else:
            flag = ', '.join(flag) + '\n'
    return flag

def add(pot, po, fuzzy = False):
    '''
    Build a new entry from the Msgs objects <pot> and <pot>. If
    <fuzzy> is true, <po>\'s flag field is ignored (in
    flags()). Returns a multiline string with a up to date entry.
    '''
    msg = []
    msg.append(po.cmt)
    msg.append(pot.autocmt)
    msg.append(pot.ref)
    msg.append(flags(po, pot, fuzzy = fuzzy))
    msg.append(pot.id)
    msg.append(po.str)
    return ''.join(msg)

def header(pot, defs):
    '''
    Update date in header entry. Returns the updated header entry.
    '''
    try:
        [po] = [ d for d in defs.values() if d.id == 'msgid ""\n' ]
    except ValueError:
        raise MsgmergeError('Error: did not find header in po file.')

    r = re.compile(r'(.*^"POT-Creation-Date:\s+)(.*?)(\\n"$.*)',
                   re.MULTILINE | re.DOTALL)
    m = r.match(pot.str)
    if not m:
        raise MsgmergeError(
            'Error: did not find POT-Creation-Date field in pot file.')

    subs = '\\1%s\\3' % m.group(2)
    _, count = r.subn(subs, po.str)
    if not count == 1:
        raise MsgmergeError(
            'Error: did not find POT-Creation-Date field in po file.')
    return po

def match(defs, refs):
    '''
    Try to match Msgs objects in <refs> with Msgs objects in
    <defs>. The return value is a list with po entries.
    '''
    global option
    matches = []
    empty = Msgs('msgid ""\n', 'msgstr ""\n', '', -1, 'str')
    deco = [(r.lno, r) for r in refs.values()]
    deco.sort()
    po = header(deco.pop(0)[1], defs)       # Header entry
    matches.append(add(empty, po))
    po.used()
    sorted = [ a[1] for a in deco ]
    for pot in sorted:
        if option.verbose:
            sys.stderr.write('.')
        po = defs.get(pot.id, False)        # Perfect match
        if po:
            matches.append(add(pot, po))
            po.used(); pot.used()
            continue
        po = fuzzy_match(pot, defs)         # Fuzzy match
        if po:
            matches.append(add(pot, po, fuzzy = True))
            po.used(); pot.used()
            continue
        matches.append(add(pot, empty))     # No match

    obsolete(defs, matches)
    return matches

def obsolete(defs, matches):
    '''Handle obsolete translations.'''
    deco = [ (d.lno, d) for d in defs.values() if
             d.count == 0 and not d.str == 'msgstr ""\n' ]
    deco.sort()
    empty = Msgs('msgid ""\n', 'msgstr ""\n', '', -1, 'str')
    obs = [ o[1] for o in deco ]
    for o in obs:
        o.flag = flags(o, empty, obs = True) 
        o.obsolete()
        matches.append('%s%s%s' % (o.flag, o.id, o.str))

def help():
    '''Print help text and exit.'''
    print __doc__
    sys.exit(0)

def cmdline():
    '''Parse options and arguments from command line.'''
    advice = 'Try `%(name)s --help\' for more information.'
    try:
        long_opt = ['help', 'version', 'update', 'output-file=',
                    'quiet', 'silent', 'docstrings', 'suffix', 'backup']
        opts, args = getopt.getopt(sys.argv[1:], 'hVUo:qD', long_opt)
    except getopt.error, msg:
        print '%s: %s\n%s' % ('%(name)s', msg, advice) % globals()
        sys.exit(1)
        
    option = Options(cmdline = True)
    for opt, arg in opts:
        if opt in ['-h', '--help']:
            help()
        elif opt in ['-V', '--version']:
            print '%(name)s %(__version__)s' % globals()
            sys.exit(0)
        elif opt in ['-o', '--output-file']:
            option.outfile = arg
        elif opt in ['-U', '--update']:
            option.update = True
        elif opt in ['-q', '--silent', '--quiet']:
            option.verbose = False
        elif opt in ['-D', '--docstrings']:
            option.docstrings = True
        elif opt in ['--suffix']:
            option.suffix = arg
        elif opt in ['--backup']:
            option.backup = arg
            
    # Sanity checks
    warn = False
    if option.update and option.outfile:
        warn = '--update and --output-file are mutually exclusive.'
    if len(args) == 0:
        warn = 'no input files given.'
    elif len(args) == 1 or len(args) > 2:
        warn = 'exactly 2 input files required.'
    if warn:
        print '%s: %s\n%s' % ('%(name)s', warn, advice) % globals()
        sys.exit(1)

    if option.update:
        option.outfile = args[0]
    elif not option.outfile:
        option.outfile = '-'

    defs, refs = args

    try:
        merge(defs, refs, option = option)
    except MsgmergeError, err:
        print '%(name)s: ' % globals() + '%s' % err
        sys.exit(1)

def io(iofile, mode = 'rU'):
    '''Wrapper around open().'''
    try:
        fo = open(iofile, mode)        
        if 'r' in mode and fo.read(3) != codecs.BOM_UTF8:
            fo.seek(0)

    except IOError, msg:
        raise MsgmergeError('error while opening file: %s: %s.' %
                            (msg[1], iofile))
    return fo

def backup(infile):
    '''Handle backup of files in update mode'''
    os.environ.get('VERSION_CONTROL', '')
    suffix = os.environ.get('SIMPLE_BACKUP_SUFFIX', '~')
    
    backup_file = '%s%s' % (infile, suffix)
    
def changes(new, old):
    return cmp(''.join(old), '\n'.join(new))

def write(matches, outfile):
    '''Write the list <matches> to file <outfile>'''
    if not outfile == '-':
        fd = io(outfile, 'w')
    else:
        fd = sys.stdout
    fd.write('\n'.join(matches))
    
def merge(def_file, ref_file, update = True, outfile = '-',
          docstrings = True, suffix = '~', backup = True,
          verbose = True, **kwds):
    '''
    Merge po file <def_file> with pot file <ref_file> . If <update> is
    set to True then only update if there are changes to the po
    file. Set outfile to write updated po file to an another file. Set
    to `-\' for writing to standard out. If docstrings is False
    docstrings flag will removed. Set verbose to False to suppress
    progress indicators. <kwds> is used to pass options from the
    command line interface.
    '''
    global option
    option = kwds.get('option', Options(update = update,
                                        outfile = outfile,
                                        docstrings = docstrings,
                                        suffix = suffix,
                                        backup = backup,
                                        verbose = verbose))
    def_msgs = parse(def_file, 'msgstr')
    ref_msgs = parse(ref_file, 'msgid')
    if verbose and not __name__ == '__main__':
        print >> sys.stderr, 'Merging %s with %s' % (ref_file, def_file)
    updated_lines = match(def_msgs, ref_msgs)
    if option.verbose:
        print >> sys.stderr, ' done.'
    if not option.update:
        write(updated_lines, option.outfile)
    elif option.update and changes(updated_lines, io(def_file).readlines()):
        write(updated_lines, def_file)
        
def merge_dir(directory, pot = False, include = [], exclude = [],
              verbose = True):
    '''
    Tries to merge a directory of po files. Uses simple glob to find
    po files and pot file. The parameter <pot> can be used to specify
    the pot file in the directory. If the list <include> is given only
    files in this list is merged. Use the list <exclude> to exclude
    files to be merged. This function is only useful if po files and
    pot file are in the same directory. Set <verbose> to get
    information when running.
    '''
    if directory[-1] == '/':
        directory = os.path.dirname(directory)
    if pot:
        pot = os.path.basename(pot)
    else:
        pot = glob.glob('%s/*.pot' % directory)
        if not pot:
            raise MsgmergeError('No pot file found.')
        elif len(pot) > 1:
            raise MsgmergeError('More than one pot file found: %s.' % pot)
        pot = os.path.basename(pot[0])
    
    if not include:
        pos = glob.glob('%s/*po' % directory)
        if not len(pos) > 1:
            raise MsgmergeError('No po file(s) found.')
        pos = [ os.path.basename(po) for po in pos ]
    else:
        pos = [ os.path.basename(po) for po in include ]
    
    for po in exclude:
        try:
            pos.remove(po)
        except ValueError:
            pass
    format = '%s/%s'
    for po in pos:
        try:
            merge(format % (directory, po), format % (directory, pot),
                  update = True, verbose = verbose,
                  outfile = format % (directory, po))
        except MsgmergeError, err:            
            if verbose:
                print >> sys.stderr, '%s Not updated.' % err
            else:
                print >> sys.stderr, '%s %s not updated.' % (err, po)

if __name__ == '__main__':
    cmdline()
