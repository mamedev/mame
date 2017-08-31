#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

from . import dbaccess

import sys


def do_listfull(options):
    dbconn = dbaccess.QueryConnection(options.database)
    dbcurs = dbconn.cursor()
    first = True
    for shortname, description in dbcurs.listfull(options.pattern):
        if first:
            sys.stdout.write('Name:             Description:\n')
            first = False
        sys.stdout.write('%-16s  "%s"\n' % (shortname, description))
    if first:
        sys.stderr.write('No matching systems found for \'%s\'\n' % (options.pattern, ))
    dbcurs.close()
    dbconn.close()


def do_listsource(options):
    dbconn = dbaccess.QueryConnection(options.database)
    dbcurs = dbconn.cursor()
    shortname = None
    for shortname, sourcefile in dbcurs.listsource(options.pattern):
        sys.stdout.write('%-16s %s\n' % (shortname, sourcefile))
    if shortname is None:
        sys.stderr.write('No matching systems found for \'%s\'\n' % (options.pattern, ))
    dbcurs.close()
    dbconn.close()


def do_listclones(options):
    dbconn = dbaccess.QueryConnection(options.database)
    dbcurs = dbconn.cursor()
    first = True
    for shortname, parent in dbcurs.listclones(options.pattern):
        if first:
            sys.stdout.write('Name:            Clone of:\n')
            first = False
        sys.stdout.write('%-16s %s\n' % (shortname, parent))
    if first:
        count = dbcurs.count_systems(options.pattern).fetchone()[0]
        if count:
            sys.stderr.write('Found %d match(es) for \'%s\' but none were clones\n' % (count, options.pattern))
        else:
            sys.stderr.write('No matching systems found for \'%s\'\n' % (options.pattern, ))
    dbcurs.close()
    dbconn.close()


def do_listbrothers(options):
    dbconn = dbaccess.QueryConnection(options.database)
    dbcurs = dbconn.cursor()
    first = True
    for sourcefile, shortname, parent in dbcurs.listbrothers(options.pattern):
        if first:
            sys.stdout.write('%-20s %-16s %s\n' % ('Source file:', 'Name:', 'Parent:'))
            first = False
        sys.stdout.write('%-20s %-16s %s\n' % (sourcefile, shortname, parent or ''))
    if first:
        sys.stderr.write('No matching systems found for \'%s\'\n' % (options.pattern, ))
    dbcurs.close()
    dbconn.close()

def do_listaffected(options):
    dbconn = dbaccess.QueryConnection(options.database)
    dbcurs = dbconn.cursor()
    first = True
    for shortname, description in dbcurs.listaffected(*options.pattern):
        if first:
            sys.stdout.write('Name:             Description:\n')
            first = False
        sys.stdout.write('%-16s  "%s"\n' % (shortname, description))
    if first:
        sys.stderr.write('No matching systems found for \'%s\'\n' % (options.pattern, ))
    dbcurs.close()
    dbconn.close()
