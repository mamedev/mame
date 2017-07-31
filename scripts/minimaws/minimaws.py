#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import lib.auxverbs
import lib.lxparse
import lib.wsgiserve

import argparse
import sys


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--database', metavar='<dbfile>', default='minimaws.sqlite3', help='SQLite 3 info database file (defaults to minimaws.sqlite3)')
    subparsers = parser.add_subparsers(title='commands', dest='command', metavar='<command>')

    subparser = subparsers.add_parser('listfull', help='list short names and full names')
    subparser.add_argument('pattern', nargs='?', metavar='<pat>', help='short name glob pattern')

    subparser = subparsers.add_parser('listsource', help='list short names and source files')
    subparser.add_argument('pattern', nargs='?', metavar='<pat>', help='short name glob pattern')

    subparser = subparsers.add_parser('listclones', help='show clones')
    subparser.add_argument('pattern', nargs='?', metavar='<pat>', help='short name/parent glob pattern')

    subparser = subparsers.add_parser('listbrothers', help='show drivers from the same source file(s)')
    subparser.add_argument('pattern', nargs='?', metavar='<pat>', help='short name glob pattern')

    subparser = subparsers.add_parser('listaffected', help='show drivers affected by source change(s)')
    subparser.add_argument('pattern', nargs='+', metavar='<pat>', help='source file glob pattern')

    subparser = subparsers.add_parser('serve', help='serve over HTTP')
    subparser.add_argument('--port', metavar='<port>', default=8080, type=int, help='server TCP port')
    subparser.add_argument('--host', metavar='<host>', default='', help='server TCP hostname')

    subparser = subparsers.add_parser('load', help='load machine information')
    group = subparser.add_mutually_exclusive_group(required=True)
    group.add_argument('--executable', metavar='<exe>', help='emulator executable')
    group.add_argument('--file', metavar='<xmlfile>', help='XML machine information file')

    options = parser.parse_args()
    if options.command == 'listfull':
        lib.auxverbs.do_listfull(options)
    elif options.command == 'listsource':
        lib.auxverbs.do_listsource(options)
    elif options.command == 'listclones':
        lib.auxverbs.do_listclones(options)
    elif options.command == 'listbrothers':
        lib.auxverbs.do_listbrothers(options)
    elif options.command == 'listaffected':
        lib.auxverbs.do_listaffected(options)
    elif options.command == 'serve':
        lib.wsgiserve.run_server(options)
    elif options.command == 'load':
        lib.lxparse.load_info(options)
    else:
        print('%s' % (options, ))
