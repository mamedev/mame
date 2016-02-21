#! /usr/bin/env python
#
# check_po - a gramps tool to check validity of po files
#
# Copyright (C) 2006-2006  Kees Bakker
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#
# TODO
#
# * Check for HTML text in msgstr when there is none in msgid
# * Check for matching HTML tag/endtag in msgstr
#

# Adapted for Umit by Guilherme Polo, original file:
# https://gramps.svn.sourceforge.net/svnroot/gramps/branches/gramps22/po/check_po

import re
import sys
from optparse import OptionParser

APP = "Umit"

all_total = {}
all_fuzzy = {}
all_untranslated = {}
all_percent_s = {}
all_named_s = {}
all_bnamed_s = {}
all_context = {}
all_coverage = {}
all_template_coverage = {}

def strip_quotes(st):
    if len(st) >= 2 and st[0] == '"' and st[len(st)-1] == '"':
        st = st.strip()[1:-1]
    return st

# This is a base class for all checks
class Check:
    def __init__( self ):
        self.msgs = []
    def diag( self ):
        if len( self.msgs ):
            print
            print self.diag_header
            for m in self.msgs:
                m.diag()
    def summary( self ):
        print "%-20s%d" % ( self.summary_text, len(self.msgs) )

class Check_fmt( Check ):
    def __init__( self, fmt ):
        Check.__init__( self )
        self.diag_header = "-------- %s mismatches --------------" % fmt
        self.summary_text = "%s mismatches:" % fmt
        self.fmt = fmt
    def process( self, msg ):
        msgid = msg.msgid
        msgstr = msg.msgstr
        cnt1 = msgid.count( self.fmt )
        cnt2 = msgstr.count( self.fmt )
        if cnt1 != cnt2:
            self.msgs.append( msg )

class Check_named_fmt( Check ):
    # A pattern to find all %()
    find_named_fmt_pat = re.compile('% \( \w+ \) \d* \D', re.VERBOSE)

    def __init__( self ):
        Check.__init__( self )
        self.diag_header = "-------- %() name mismatches --------------"
        self.summary_text = "%() name mismatches:"
    def process( self, msg ):
        msgid = msg.msgid
        msgstr = msg.msgstr
        # Same number of named formats?
        fmts1 = self.find_named_fmt_pat.findall( msgid )
        fmts2 = self.find_named_fmt_pat.findall( msgstr )
        if len( fmts1 ) != len( fmts2 ):
            self.msgs.append( msg )
        else:
            # Do we have the same named formats?
            fmts1.sort()
            fmts2.sort()
            if fmts1 != fmts2:
                self.msgs.append( msg )

class Check_missing_sd( Check ):
    # A pattern to find %() without s or d
    # Here is a command to use for testing
    # print re.compile('% \( \w+ \) \d* (\D|$)', re.VERBOSE).findall( '%(event_name)s: %(place)s%(endnotes)s. ' )
    find_named_fmt_pat2 = re.compile('% \( \w+ \) \d* (\D|$)', re.VERBOSE)

    def __init__( self ):
        Check.__init__( self )
        self.diag_header = "-------- %() without 's' or 'd' mismatches --------------"
        self.summary_text = "%() missing s/d:"
    def process( self, msg ):
        msgstr = msg.msgstr
        fmts = self.find_named_fmt_pat2.findall( msgstr )
        for f in fmts:
            if not f in ('s', 'd'):
                self.msgs.append( msg )
                break

class Check_runaway( Check ):
    def __init__( self ):
        Check.__init__( self )
        self.diag_header = "-------- Runaway context in translation ---------"
        self.summary_text = "Runaway context:"
    def process( self, msg ):
        msgid = msg.msgid
        msgstr = msg.msgstr

        # Runaway context. In the translated part we only to see
        # the translation of the word after the |
        if msgid.count('|') > 0 and msgstr.count('|') > 0 and msgid != msgstr:
            self.msgs.append( msg )

class Check_xml_chars( Check ):
    # Special XML characters
    # It is not allowed to have a quote, an ampersand or an angle bracket
    xml_chars_pat = re.compile( r'(?<=\W) > | " | & (?!(quot|nbsp|gt|amp);)', re.VERBOSE )

    def __init__( self ):
        Check.__init__( self )
        self.diag_header = "-------- unescaped XML special characters ---------"
        self.summary_text = "XML special chars:"
    def process( self, msg ):
        msgid = msg.msgid
        msgstr = msg.msgstr

        # XML errors
        # Only look at messages in the tips.xml
        if msg.is_tips_xml:
            if self.xml_chars_pat.search( msgstr ):
                self.msgs.append( msg )

class Check_last_char( Check ):
    def __init__( self ):
        Check.__init__( self )
        self.diag_header = "-------- last character not identical ---------"
        self.summary_text = "Last character:"
    def process( self, msg ):
        msgid = msg.msgid
        msgstr = msg.msgstr

        # Last character of msgid? White space? Period?
        if msg.is_fuzzy:
            return

        msgid_last = msgid[-1:]
        msgstr_last = msgstr[-1:]
        if msgid_last.isspace() != msgstr_last.isspace():
            self.msgs.append( msg )
        elif (msgid_last == '.') != (msgstr_last == '.'):
            self.msgs.append( msg )

class Check_shortcut_trans( Check ):
    def __init__( self ):
        Check.__init__( self )
        self.diag_header = "-------- shortcut key in translation ---------"
        self.summary_text = "Shortcut in msgstr:"
    def process( self, msg ):
        msgid = msg.msgid
        msgstr = msg.msgstr

        if msgid.count('_') == 0 and msgstr.count('_') > 0:
            self.msgs.append( msg )

class Msgid:
    fuzzy_pat = re.compile( 'fuzzy' )
    tips_xml_pat = re.compile( r'tips\.xml' )
    def __init__( self, msgnr, lineno ):
        self._msgid = []
        self._msgstr = []
        self.msgid = ''
        self.msgstr = ''
        self._cmnt = []
        self.nr = msgnr
        self.lineno = lineno
        self.is_fuzzy = 0
        self.is_tips_xml = 0

    def diag( self ):
        if 1:
            print
            print "msg nr: %d, lineno: %d%s" % ( self.nr, self.lineno, self.is_fuzzy and " (fuzzy)" or "" )
            sys.stdout.write( ''.join( self._msgid ) )
            sys.stdout.write( ''.join( self._msgstr ) )
        else:
            # Compatible with the old check_po
            print "%d '%s' : '%s'" % ( self.lineno, self.msgid, self.msgstr )

    def add_msgid( self, line, lineno ):
        self._msgid.append( line )
        line = re.sub( r'msgid\s+', '', line )
        line = line.strip()
        if line[0] != '"' or line[-1:] != '"':
            print "ERROR at line %d: Missing quote." % lineno
        line = strip_quotes( line )
        self.msgid += line

    def add_msgstr( self, line, lineno ):
        self._msgstr.append( line )
        line = re.sub( r'msgstr\s+', '', line )
        line = line.strip()
        if line[0] != '"' or line[-1:] != '"':
            print "ERROR at line %d: Missing quote." % lineno
        line = strip_quotes( line )
        self.msgstr += line

    def add_cmnt( self, line ):
        self._cmnt.append( line )
        if not self.is_fuzzy and self.fuzzy_pat.search( line ):
            self.is_fuzzy = 1
        if not self.is_tips_xml and self.tips_xml_pat.search( line ):
            self.is_tips_xml = 1

def read_msgs( fname ):
    empty_pat   = re.compile( r'^ \s* $',      re.VERBOSE )
    comment_pat = re.compile( r'\#',           re.VERBOSE )
    msgid_pat   = re.compile( r'msgid \s+ "',  re.VERBOSE )
    msgstr_pat  = re.compile( r'msgstr \s+ "', re.VERBOSE )
    str_pat     = re.compile( r'"',            re.VERBOSE )
    old_pat     = re.compile( r'\#~ \s+ ',     re.VERBOSE )

    msgnr = 0			# This is the message number of the next message to read. The first real message is 1.
    f = open( fname )
    lines = f.readlines()

    # parse it like a statemachine
    NONE   = 0			# Nothing detected, yet
    CMNT   = 1			# Inside comment part
    MSGID  = 2			# Inside msgid part
    MSGSTR = 3			# Inside msgstr part
    STR    = 4			# A continuation string
    OLD    = 5			# An old pattern with #~

    state = NONE
    msg = None
    msgs = []

    for ix in range( len(lines) ):	# Use line numbers for messages
        line = lines[ix]
        lineno = ix + 1

        m = empty_pat.match( line )
        if m:
            continue	# Empty lines are not interesting

        # What's the next state?
        if  old_pat.match( line ):
            next_state = OLD
        elif comment_pat.match( line ):
            next_state = CMNT
        elif msgid_pat.match( line ):
            next_state = MSGID
        elif msgstr_pat.match( line ):
            next_state = MSGSTR
        elif str_pat.match( line ):
            next_state = STR
        else:
            print 'WARNING: Unexpected input at %(fname)s:%(lineno)d' % vars()
            next_state = NONE

        #print "%(state)d->%(next_state)d\t%(line)s" % vars()
        if state == NONE:
            # expect msgid or comment or old stuff
            if next_state == CMNT:
                state = CMNT
                msg = Msgid( msgnr, lineno ) # Start with an empty new item
                msgnr += 1
                msgs.append( msg )
                msg.add_cmnt( line )

            elif next_state == MSGID:
                state = MSGID
                msg = Msgid( msgnr, lineno ) # Start with an empty new item
                msgnr += 1
                msgs.append( msg )
                msg.add_msgid( line, lineno )

            elif next_state == MSGSTR:
                print 'WARNING: Wild msgstr at %(fname)s:%(lineno)d' % vars()
                state = MSGSTR
                msg = Msgid( msgnr, lineno ) # Start with an empty new item
                msgnr += 1
                msgs.append( msg )
                msg.add_msgstr( line, lineno )

            elif next_state == STR:
                print 'WARNING: Wild string at %(fname)s:%(lineno)d' % vars()

            elif next_state == OLD:
                pass	# Just skip

        elif state == CMNT:
            if next_state == CMNT:
                if msg:
                    msg.add_cmnt( line )
                else:
                    # Note. We may need to do something about these comments
                    # Skip for now
                    pass

            elif next_state == MSGID:
                state = MSGID
                if not msg:
                    msg = Msgid( msgnr, lineno ) # Start with an empty new item
                    msgnr += 1
                    msgs.append( msg )
                msg.add_msgid( line, lineno )

            elif next_state == MSGSTR:
                print 'WARNING: Wild msgstr at %(fname)s:%(lineno)d' % vars()
                state = MSGSTR
                msg = Msgid( msgnr, lineno ) # Start with an empty new item
                msgnr += 1
                msgs.append( msg )
                msg.add_msgstr( line, lineno )

            elif next_state == STR:
                print 'WARNING: Wild string at %(fname)s:%(lineno)d' % vars()

            elif next_state == OLD:
                msg = None
                pass	# Just skip

        elif state == MSGID:
            if next_state == CMNT:
                # Hmmm. A comment here?
                print 'WARNING: Unexpted comment at %(fname)s:%(lineno)d' % vars()

            elif next_state == MSGID:
                raise Exception( 'Unexpected msgid at %(fname)s:%(lineno)d' % vars() )

            elif next_state == MSGSTR:
                state = MSGSTR
                msg.add_msgstr( line, lineno )

            elif next_state == STR:
                msg.add_msgid( line, lineno )

            elif next_state == OLD:
                msg = None
                pass	# Just skip

        elif state == MSGSTR:
            if next_state == CMNT:
                # A comment probably starts a new item
                state = CMNT
                msg = Msgid( msgnr, lineno )
                msgnr += 1
                msgs.append( msg )
                msg.add_cmnt( line )

            elif next_state == MSGID:
                state = MSGID
                msg = Msgid( msgnr, lineno )
                msgnr += 1
                msgs.append( msg )
                msg.add_msgid( line, lineno )

            elif next_state == MSGSTR:
                raise Exception( 'Unexpected msgstr at %(fname)s:%(lineno)d' % vars() )

            elif next_state == STR:
                msg.add_msgstr( line, lineno )

            elif next_state == OLD:
                msg = None
                pass	# Just skip

        else:
            raise Exception( 'Unexpected state in po parsing (state = %d)' % state )

    # Strip items with just comments. (Can this happen?)
    msgs1 = []
    for m in msgs:
        if not m.msgid and not m.msgstr:
            #print "INFO: No msgid or msgstr at %s:%s" % ( fname, m.lineno )
            pass
        else:
            msgs1.append( m )
    msgs = msgs1
    return msgs

def analyze_msgs( options, fname, msgs, nr_templates = None, nth = 0 ):
    nr_fuzzy = 0
    nr_untranslated = 0

    checks = []
    checks.append( Check_fmt( '%s' ) )
    checks.append( Check_fmt( '%d' ) )
    checks.append( Check_named_fmt() )
    checks.append( Check_missing_sd() )
    checks.append( Check_runaway() )
    checks.append( Check_xml_chars() )
    checks.append( Check_last_char() )
    checks.append( Check_shortcut_trans() )

    for msg in msgs:
        msgid = msg.msgid
        msgstr = msg.msgstr
        #print
        #print "msgid: %(msgid)s" % vars()
        #print "msgstr: %(msgstr)s" % vars()

        if not msgstr:
            nr_untranslated += 1
            continue

        if msg.is_fuzzy:
            nr_fuzzy += 1
            if options.skip_fuzzy:
                continue

        for c in checks:
            c.process( msg )

    nr_msgs = len(msgs)
    if nth > 0:
        print
        print "====================================="
    print "%-20s%s"     % ( "File:",              fname )
    print "%-20s%d"     % ( "Template total:",    nr_templates )
    print "%-20s%d"     % ( "PO total:",          nr_msgs )
    print "%-20s%d"     % ( "Fuzzy:",             nr_fuzzy )
    print "%-20s%d"     % ( "Untranslated:",      nr_untranslated )

    for c in checks:
        c.summary()

    po_coverage = (1.0 - (float(nr_untranslated) / float(nr_msgs))) * 100
    print "%-20s%5.2f%%" % ( "PO Coverage:",       po_coverage )

    template_coverage = po_coverage * float(nr_msgs) / float(nr_templates)
    print "%-20s%5.2f%%" % ( "Template Coverage:", template_coverage )

    if not options.only_summary:
        for c in checks:
            c.diag()

def main(args):
    if len(sys.argv) < 2:
        print "Error: Especify the umit.pot file path"
        sys.exit(1)

    parser = OptionParser(description="This program validates a PO file for "
                          "%s." % APP, usage='%prog [options] po-file...' )

    parser.add_option("", "--skip-fuzzy",
                      action="store_true", dest="skip_fuzzy", default=False,
                      help="skip fuzzies")

    parser.add_option("-s", "--only-summary",
                      action="store_true", dest="only_summary", default=False,
                      help="only give the summary")

    options, args = parser.parse_args()

    try:
        pot_msgs = read_msgs(sys.argv[1])
        nr_templates = len(pot_msgs)
        nth = 0
        for fname in args:
            msgs = read_msgs(fname)
            analyze_msgs(options, fname, msgs, nr_templates, nth)
            nth += 1

    except Exception, e:
        print e

if __name__ == "__main__":
    main(sys.argv)
