#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb
##
## Demonstrates use of MAME's XML system information output
##
## This script requires Python 2.7 or Python 3.4, and SQLite 3.6.19 at
## the very least.  Help is provided for all command-line options (use
## -h or --help).
##
## Before you can use the scripts, you need to load MAME system
## information into a database:
##
## $ python minimaws.py load --executable path/to/mame
##
## (The script uses the name "minimaws.sqlite3" for the database by
## default, but you can override this with the --database option.)
##
## After you've loaded the database, you can use query commands.  Most
## of the query commands behave similarly to MAME's auxiliary verbs but
## case-sensitive and with better globbing (output not shown for
## brevity):
##
## $ python minimaws.py listfull "unkch*"
## $ python minimaws.py listclones "unkch*"
## $ python minimaws.py listbrothers superx
##
## The romident command does not support 7zip archives, but it's far
## faster than using MAME as it has optimised indexes, and results are
## grouped by machine rather than by file:
##
## $ python minimaws.py romident 27c64.bin dump-dir
##
## One more sophisticated query command is provided that MAME has no
## equivalent for.  The listaffected command shows all runnable machines
## that reference devices defined in specified source files:
##
## $ python minimaws.py listaffected "devices/cpu/m6805/*" devices/cpu/mcs40/mcs40.cpp
##
## This script can also run a local web server allowing you to explore
## systems, devices and source files:
##
## $ python minimaws.py serve
##
## The default TCP port is 8080 but if desired, this can be changed with
## the --port option.  The web service is implemented using WSGI, so it
## can be run in a web server if desired (e.g. using Apache mod_wsgi).
## It uses get queries and provides cacheable reponses, so it should
## work behind a caching proxy (e.g. squid or nginx).  Although the
## service is written to avoid SQL injected and directory traversal
## attacks, and it avoids common sources of security issues, it has not
## been audited for vulnerabilities and is not recommended for use on
## public web sites.
##
## To use the web service, you need to know the short name of a device/
## system, or the name of a source file containing a system:
##
## http://localhost:8080/machine/intlc440
## http://localhost:8080/machine/a2mouse
## http://localhost:8080/sourcefile/src/devices/cpu/m68000/m68kcpu.cpp
##
## You can also start with a list of all source files containing machine
## definitions, but this is quite a large page and may perform poorly:
##
## http://localhost:8080/sourcefile/
##
## One feature that may be of iterest to front-end authors or users of
## computer emulation is the ability to show available slot options and
## update live as changes are made.  This can be seen in action on
## computer systems:
##
## http://localhost:8080/machine/ibm5150
## http://localhost:8080/machine/apple2e
## http://localhost:8080/machine/ti82
##
## On any of these, and many other systems, you can select slot options
## and see dependent slots update.  Required command-line arguments to
## produce the selected configuration are also displayed.
##
## Media files can be identified using a web browser interface
##
## Checksums and digests are calculated locally - no files are uploaded
## to the server.

if __name__ == '__main__':
    import argparse

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

    subparser = subparsers.add_parser('romident', help='identify ROM dump(s)')
    subparser.add_argument('path', nargs='+', metavar='<path>', help='ROM dump file/directory path')

    subparser = subparsers.add_parser('serve', help='serve over HTTP')
    subparser.add_argument('--port', metavar='<port>', default=8080, type=int, help='server TCP port')
    subparser.add_argument('--host', metavar='<host>', default='', help='server TCP hostname')

    subparser = subparsers.add_parser('load', help='load machine information')
    group = subparser.add_mutually_exclusive_group(required=True)
    group.add_argument('--executable', metavar='<exe>', help='emulator executable')
    group.add_argument('--file', metavar='<xmlfile>', help='XML machine information file')
    subparser.add_argument('--softwarepath', required=True, action='append', metavar='<path>', help='Software list directory path')

    options = parser.parse_args()

    import lib.auxverbs
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
    elif options.command == 'romident':
        lib.auxverbs.do_romident(options)
    elif options.command == 'serve':
        import wsgiref.simple_server
        import lib.wsgiserve
        application = lib.wsgiserve.MiniMawsApp(options.database)
        server = wsgiref.simple_server.make_server(options.host, options.port, application)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass
    elif options.command == 'load':
        import lib.lxparse
        lib.lxparse.load_info(options)
