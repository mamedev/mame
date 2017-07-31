#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

from . import dbaccess
from . import htmltmpl

import cgi
import sys
import wsgiref.simple_server
import wsgiref.util

if sys.version_info > (3, ):
    import urllib.parse as urlparse
else:
    import urlparse


class MachineHandler(object):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super(MachineHandler, self).__init__(**kwargs)
        self.application_uri = application_uri
        self.environ = environ
        self.start_response = start_response
        self.dbcurs = app.dbconn.cursor()
        self.shortname = wsgiref.util.shift_path_info(environ)

    def __iter__(self):
        if not self.shortname:
            # could probably list machines here or something
            self.start_response('403 Forbidden', [('Content-type', 'text/plain')])
            yield '403 Forbidden'.encode('utf-8')
        elif self.environ['PATH_INFO']:
            # subdirectory of a machine
            self.start_response('404 Not Found', [('Content-type', 'text/plain')])
            yield '404 Not Found'.encode('utf-8')
        else:
            machine_info = self.dbcurs.get_machine_info(self.shortname).fetchone()
            if not machine_info:
                self.start_response('404 Not Found', [('Content-type', 'text/plain')])
                yield '404 Not Found'.encode('utf-8')
            else:
                self.start_response('200 OK', [('Content-type', 'text/html; chearset=utf-8')])
                description = machine_info['description']
                yield htmltmpl.MACHINE_PROLOGUE.substitute(
                        description=cgi.escape(description),
                        shortname=cgi.escape(self.shortname),
                        isdevice=cgi.escape('Yes' if machine_info['isdevice'] else 'No'),
                        runnable=cgi.escape('Yes' if machine_info['runnable'] else 'No'),
                        sourcefile=cgi.escape(machine_info['sourcefile'])).encode('utf-8')
                if machine_info['year'] is not None:
                    yield (
                            '    <tr><th style="text-align: right">Year:</th><td>%s</td></tr>\n' \
                            '    <tr><th style="text-align: right">Manufacturer:</th><td>%s</td></tr>\n' %
                            (cgi.escape(machine_info['year']), cgi.escape(machine_info['Manufacturer']))).encode('utf-8')
                if machine_info['cloneof'] is not None:
                    parent = self.dbcurs.listfull(machine_info['cloneof']).fetchone()
                    if parent:
                        yield (
                                '    <tr><th style="text-align: right">Parent Machine:</th><td><a href="%s">%s (%s)</a></td></tr>\n' %
                                (cgi.escape('%s/machine/%s' % (self.application_uri, machine_info['cloneof']), True), cgi.escape(parent[1]), cgi.escape(machine_info['cloneof']))).encode('utf-8')
                    else:
                        yield (
                                '    <tr><th style="text-align: right">Parent Machine:</th><td><a href="%s">%s</a></td></tr>\n' %
                                (cgi.escape('%s/machine/%s' % (self.application_uri, machine_info['cloneof']), True), cgi.escape(machine_info['cloneof']))).encode('utf-8')
                if (machine_info['romof'] is not None) and (machine_info['romof'] != machine_info['cloneof']):
                    parent = self.dbcurs.listfull(machine_info['romof']).fetchone()
                    if parent:
                        yield (
                                '    <tr><th style="text-align: right">Parent ROM set:</th><td><a href="%s">%s (%s)</a></td></tr>\n' %
                                (cgi.escape('%s/machine/%s' % (self.application_uri, machine_info['romof']), True), cgi.escape(parent[1]), cgi.escape(machine_info['romof']))).encode('utf-8')
                    else:
                        yield (
                                '    <tr><th style="text-align: right">Parent Machine:</th><td><a href="%s">%s</a></td></tr>\n' %
                                (cgi.escape('%s/machine/%s' % (self.application_uri, machine_info['romof']), True), cgi.escape(machine_info['romof']))).encode('utf-8')
                yield '</table>\n'.encode('utf-8')

                devices = self.dbcurs.get_devices_referenced(machine_info['id'])
                first = True
                for name, desc in devices:
                    if first:
                        yield '<h2>Devices Referenced</h2>\n<ul>\n'.encode('utf-8')
                        first = False
                    if desc is not None:
                        yield (
                                '    <li><a href="%s">%s (%s)</a></li>\n' %
                                (self.machine_href(name), cgi.escape(desc), cgi.escape(name))).encode('utf-8')
                    else:
                        yield (
                                '    <li><a href="%s">%s</a></li>\n' %
                                (self.machine_href(name), cgi.escape(name))).encode('utf-8')
                if not first:
                    yield '</ul>\n'.encode('utf-8')

                devices = self.dbcurs.get_device_references(self.shortname)
                first = True
                for name, desc in devices:
                    if first:
                        yield '<h2>Referenced By</h2>\n<ul>\n'.encode('utf-8')
                        first = False
                    yield (
                            '    <li><a href="%s">%s (%s)</a></li>\n' %
                            (self.machine_href(name), cgi.escape(desc), cgi.escape(name))).encode('utf-8')
                if not first:
                    yield '</ul>\n'.encode('utf-8')

                yield '</html>\n'.encode('utf-8')

    def machine_href(self, shortname):
        return cgi.escape(urlparse.urljoin(self.application_uri, 'machine/%s' % (shortname, )), True)


class MiniMawsApp(object):
    def __init__(self, dbfile, **kwargs):
        super(MiniMawsApp, self).__init__(**kwargs)
        self.dbconn = dbaccess.QueryConnection(dbfile)

    def __call__(self, environ, start_response):
        application_uri = wsgiref.util.application_uri(environ)
        module = wsgiref.util.shift_path_info(environ)
        if module == 'machine':
            return MachineHandler(self, application_uri, environ, start_response)
        else:
            start_response('200 OK', [('Content-type', 'text/plain')])
            return ('Module is %s\n' % (module, ), )


def run_server(options):
    application = MiniMawsApp(options.database)
    server = wsgiref.simple_server.make_server(options.host, options.port, application)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
