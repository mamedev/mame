#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

from . import dbaccess
from . import htmltmpl

import cgi
import inspect
import json
import mimetypes
import os.path
import re
import sys
import urllib
import wsgiref.util

if sys.version_info >= (3, ):
    import urllib.parse as urlparse
    urlquote = urlparse.quote
else:
    import urlparse
    urlquote = urllib.quote


class HandlerBase(object):
    STATUS_MESSAGE = {
            400: 'Bad Request',
            401: 'Unauthorized',
            403: 'Forbidden',
            404: 'Not Found',
            405: 'Method Not Allowed',
            500: 'Internal Server Error',
            501: 'Not Implemented',
            502: 'Bad Gateway',
            503: 'Service Unavailable',
            504: 'Gateway Timeout',
            505: 'HTTP Version Not Supported' }

    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super(HandlerBase, self).__init__(**kwargs)
        self.app = app
        self.js_escape = app.js_escape
        self.application_uri = application_uri
        self.environ = environ
        self.start_response = start_response

    def error_page(self, code):
        yield htmltmpl.ERROR_PAGE.substitute(code=cgi.escape('%d' % code), message=cgi.escape(self.STATUS_MESSAGE[code])).encode('utf-8')


class ErrorPageHandler(HandlerBase):
    def __init__(self, code, app, application_uri, environ, start_response, **kwargs):
        super(ErrorPageHandler, self).__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
        self.code = code
        self.start_response('%d %s' % (self.code, self.STATUS_MESSAGE[code]), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])

    def __iter__(self):
        return self.error_page(self.code)


class AssetHandler(HandlerBase):
    def __init__(self, directory, app, application_uri, environ, start_response, **kwargs):
        super(AssetHandler, self).__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
        self.directory = directory
        self.asset = wsgiref.util.shift_path_info(environ)

    def __iter__(self):
        if not self.asset:
            self.start_response('403 %s' % (self.STATUS_MESSAGE[403], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(403)
        elif self.environ['PATH_INFO']:
            self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(404)
        else:
            path = os.path.join(self.directory, self.asset)
            if not os.path.isfile(path):
                self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(404)
            elif self.environ['REQUEST_METHOD'] != 'GET':
                self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(405)
            else:
                try:
                    f = open(path, 'rb')
                    type, encoding = mimetypes.guess_type(path)
                    self.start_response('200 OK', [('Content-type', type or 'application/octet-stream'), ('Cache-Control', 'public, max-age=3600')])
                    return wsgiref.util.FileWrapper(f)
                except:
                    self.start_response('500 %s' % (self.STATUS_MESSAGE[500], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                    return self.error_page(500)


class QueryPageHandler(HandlerBase):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super(QueryPageHandler, self).__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
        self.dbcurs = app.dbconn.cursor()

    def machine_href(self, shortname):
        return cgi.escape(urlparse.urljoin(self.application_uri, 'machine/%s' % (urlquote(shortname), )), True)

    def sourcefile_href(self, sourcefile):
        return cgi.escape(urlparse.urljoin(self.application_uri, 'sourcefile/%s' % (urlquote(sourcefile), )), True)


class MachineRpcHandlerBase(QueryPageHandler):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super(MachineRpcHandlerBase, self).__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
        self.shortname = wsgiref.util.shift_path_info(environ)

    def __iter__(self):
        if not self.shortname:
            self.start_response('403 %s' % (self.STATUS_MESSAGE[403], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(403)
        elif self.environ['PATH_INFO']:
            self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(404)
        else:
            machine = self.dbcurs.get_machine_id(self.shortname)
            if machine is None:
                self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(404)
            elif self.environ['REQUEST_METHOD'] != 'GET':
                self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(405)
            else:
                self.start_response('200 OK', [('Content-type', 'application/json; chearset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                return self.data_page(machine)


class MachineHandler(QueryPageHandler):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super(MachineHandler, self).__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
        self.shortname = wsgiref.util.shift_path_info(environ)

    def __iter__(self):
        if not self.shortname:
            # could probably list machines here or something
            self.start_response('403 %s' % (self.STATUS_MESSAGE[403], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(403)
        elif self.environ['PATH_INFO']:
            self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(404)
        else:
            machine_info = self.dbcurs.get_machine_info(self.shortname).fetchone()
            if not machine_info:
                self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(404)
            elif self.environ['REQUEST_METHOD'] != 'GET':
                self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(405)
            else:
                self.start_response('200 OK', [('Content-type', 'text/html; chearset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                return self.machine_page(machine_info)

    def machine_page(self, machine_info):
        id = machine_info['id']
        description = machine_info['description']
        yield htmltmpl.MACHINE_PROLOGUE.substitute(
                app=self.js_escape(cgi.escape(self.application_uri, True)),
                assets=self.js_escape(cgi.escape(urlparse.urljoin(self.application_uri, 'static'), True)),
                sourcehref=self.sourcefile_href(machine_info['sourcefile']),
                description=cgi.escape(description),
                shortname=cgi.escape(self.shortname),
                isdevice=cgi.escape('Yes' if machine_info['isdevice'] else 'No'),
                runnable=cgi.escape('Yes' if machine_info['runnable'] else 'No'),
                sourcefile=cgi.escape(machine_info['sourcefile'])).encode('utf-8')
        if machine_info['year'] is not None:
            yield (
                    '    <tr><th>Year:</th><td>%s</td></tr>\n' \
                    '    <tr><th>Manufacturer:</th><td>%s</td></tr>\n' %
                    (cgi.escape(machine_info['year']), cgi.escape(machine_info['Manufacturer']))).encode('utf-8')
        if machine_info['cloneof'] is not None:
            parent = self.dbcurs.listfull(machine_info['cloneof']).fetchone()
            if parent:
                yield (
                        '    <tr><th>Parent Machine:</th><td><a href="%s">%s (%s)</a></td></tr>\n' %
                        (self.machine_href(machine_info['cloneof']), cgi.escape(parent[1]), cgi.escape(machine_info['cloneof']))).encode('utf-8')
            else:
                yield (
                        '    <tr><th>Parent Machine:</th><td><a href="%s">%s</a></td></tr>\n' %
                        (self.machine_href(machine_info['cloneof']), cgi.escape(machine_info['cloneof']))).encode('utf-8')
        if (machine_info['romof'] is not None) and (machine_info['romof'] != machine_info['cloneof']):
            parent = self.dbcurs.listfull(machine_info['romof']).fetchone()
            if parent:
                yield (
                        '    <tr><th>Parent ROM set:</th><td><a href="%s">%s (%s)</a></td></tr>\n' %
                        (self.machine_href(machine_info['romof']), cgi.escape(parent[1]), cgi.escape(machine_info['romof']))).encode('utf-8')
            else:
                yield (
                        '    <tr><th>Parent Machine:</th><td><a href="%s">%s</a></td></tr>\n' %
                        (self.machine_href(machine_info['romof']), cgi.escape(machine_info['romof']))).encode('utf-8')
        unemulated = []
        imperfect = []
        for feature, status, overall in self.dbcurs.get_feature_flags(id):
            if overall == 1:
                imperfect.append(feature)
            elif overall > 1:
                unemulated.append(feature)
        if (unemulated):
            unemulated.sort()
            yield(
                    ('    <tr><th>Unemulated Features:</th><td>%s' + (', %s' * (len(unemulated) - 1)) + '</td></tr>\n') %
                    tuple(unemulated)).encode('utf-8');
        if (imperfect):
            yield(
                    ('    <tr><th>Imperfect Features:</th><td>%s' + (', %s' * (len(imperfect) - 1)) + '</td></tr>\n') %
                    tuple(imperfect)).encode('utf-8');
        yield '</table>\n'.encode('utf-8')

        # allow system BIOS selection
        haveoptions = False
        for name, desc, isdef in self.dbcurs.get_biossets(id):
            if not haveoptions:
                haveoptions = True;
                yield htmltmpl.MACHINE_OPTIONS_HEADING.substitute().encode('utf-8')
                yield htmltmpl.MACHINE_BIOS_PROLOGUE.substitute().encode('utf-8')
            yield htmltmpl.MACHINE_BIOS_OPTION.substitute(
                    name=cgi.escape(name, True),
                    description=cgi.escape(desc),
                    isdefault=('yes' if isdef else 'no')).encode('utf-8')
        if haveoptions:
            yield '</select>\n<script>set_default_system_bios();</script>\n'.encode('utf-8')

        # allow RAM size selection
        first = True
        for name, size, isdef in self.dbcurs.get_ram_options(id):
            if first:
                if not haveoptions:
                    haveoptions = True;
                    yield htmltmpl.MACHINE_OPTIONS_HEADING.substitute().encode('utf-8')
                yield htmltmpl.MACHINE_RAM_PROLOGUE.substitute().encode('utf-8')
                first = False
            yield htmltmpl.MACHINE_RAM_OPTION.substitute(
                    name=cgi.escape(name, True),
                    size=cgi.escape('{:,}'.format(size)),
                    isdefault=('yes' if isdef else 'no')).encode('utf-8')
        if not first:
            yield '</select>\n<script>set_default_ram_option();</script>\n'.encode('utf-8')

        # placeholder for machine slots - populated by client-side JavaScript
        if self.dbcurs.count_slots(id):
            if not haveoptions:
                haveoptions = True
                yield htmltmpl.MACHINE_OPTIONS_HEADING.substitute().encode('utf-8')
            yield htmltmpl.MACHINE_SLOTS_PLACEHOLDER.substitute(
                    machine=self.js_escape(self.shortname)).encode('utf=8')

        # list devices referenced by this system/device
        first = True
        for name, desc, src in self.dbcurs.get_devices_referenced(id):
            if first:
                yield \
                        '<h2>Devices Referenced</h2>\n' \
                        '<table id="tbl-dev-refs">\n' \
                        '    <thead>\n' \
                        '        <tr><th>Short name</th><th>Description</th><th>Source file</th></tr>\n' \
                        '    </thead>\n' \
                        '    <tbody>\n'.encode('utf-8')
                first = False
            yield self.machine_row(name, desc, src)
        if not first:
            yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-dev-refs').encode('utf-8')

        # list slots where this device is an option
        first = True
        for name, desc, slot, opt, src in self.dbcurs.get_compatible_slots(id):
            if (first):
                yield \
                        '<h2>Compatible Slots</h2>\n' \
                        '<table id="tbl-comp-slots">\n' \
                        '    <thead>\n' \
                        '        <tr><th>Short name</th><th>Description</th><th>Slot</th><th>Choice</th><th>Source file</th></tr>\n' \
                        '    </thead>\n' \
                        '    <tbody>\n'.encode('utf-8')
                first = False
            yield htmltmpl.COMPATIBLE_SLOT_ROW.substitute(
                    machinehref=self.machine_href(name),
                    sourcehref=self.sourcefile_href(src),
                    shortname=cgi.escape(name),
                    description=cgi.escape(desc),
                    sourcefile=cgi.escape(src),
                    slot=cgi.escape(slot),
                    slotoption=cgi.escape(opt)).encode('utf-8')
        if not first:
            yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-comp-slots').encode('utf-8')

        # list systems/devices that reference this device
        first = True
        for name, desc, src in self.dbcurs.get_device_references(id):
            if first:
                yield \
                        '<h2>Referenced By</h2>\n' \
                        '<table id="tbl-ref-by">\n' \
                        '    <thead>\n' \
                        '        <tr><th>Short name</th><th>Description</th><th>Source file</th></tr>\n' \
                        '    </thead>\n' \
                        '    <tbody>\n'.encode('utf-8')
                first = False
            yield self.machine_row(name, desc, src)
        if not first:
            yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-ref-by').encode('utf-8')

        yield '</html>\n'.encode('utf-8')

    def machine_row(self, shortname, description, sourcefile):
        return (htmltmpl.MACHINE_ROW if description is not None else htmltmpl.EXCL_MACHINE_ROW).substitute(
                machinehref=self.machine_href(shortname),
                sourcehref=self.sourcefile_href(sourcefile),
                shortname=cgi.escape(shortname),
                description=cgi.escape(description or ''),
                sourcefile=cgi.escape(sourcefile or '')).encode('utf-8')


class SourceFileHandler(QueryPageHandler):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super(SourceFileHandler, self).__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)

    def __iter__(self):
        self.filename = self.environ['PATH_INFO']
        if self.filename and (self.filename[0] == '/'):
            self.filename = self.filename[1:]
        if not self.filename:
            if self.environ['REQUEST_METHOD'] != 'GET':
                self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(405)
            else:
                self.start_response('200 OK', [('Content-type', 'text/html; chearset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                return self.sourcefile_listing_page(None)
        else:
            id = self.dbcurs.get_sourcefile_id(self.filename)
            if id is None:
                if ('*' not in self.filename) and ('?' not in self.filename) and ('?' not in self.filename):
                    self.filename += '*' if self.filename[-1] == '/' else '/*'
                    if not self.dbcurs.count_sourcefiles(self.filename):
                        self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                        return self.error_page(404)
                    elif self.environ['REQUEST_METHOD'] != 'GET':
                        self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
                        return self.error_page(405)
                    else:
                        self.start_response('200 OK', [('Content-type', 'text/html; chearset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                        return self.sourcefile_listing_page(self.filename)
                else:
                    self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                    return self.error_page(404)
            elif self.environ['REQUEST_METHOD'] != 'GET':
                self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(405)
            else:
                self.start_response('200 OK', [('Content-type', 'text/html; chearset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                return self.sourcefile_page(id)

    def sourcefile_listing_page(self, pattern):
        if not pattern:
            title = heading = 'All Source Files'
        else:
            heading = self.linked_title(pattern)
            title = 'Source Files: ' + cgi.escape(pattern)
        yield htmltmpl.SOURCEFILE_LIST_PROLOGUE.substitute(
                assets=cgi.escape(urlparse.urljoin(self.application_uri, 'static'), True),
                title=title,
                heading=heading).encode('utf-8')
        for filename, machines in self.dbcurs.get_sourcefiles(pattern):
            yield htmltmpl.SOURCEFILE_LIST_ROW.substitute(
                    sourcefile=self.linked_title(filename, True),
                    machines=cgi.escape('%d' % machines)).encode('utf-8')
        yield '    </tbody>\n</table>\n<script>make_table_sortable(document.getElementById("tbl-sourcefiles"));</script>\n</body>\n</html>\n'.encode('utf-8')

    def sourcefile_page(self, id):
        yield htmltmpl.SOURCEFILE_PROLOGUE.substitute(
                assets=cgi.escape(urlparse.urljoin(self.application_uri, 'static'), True),
                filename=cgi.escape(self.filename),
                title=self.linked_title(self.filename)).encode('utf-8')

        first = True
        for machine_info in self.dbcurs.get_sourcefile_machines(id):
            if first:
                yield \
                        '<table id="tbl-machines">\n' \
                        '    <thead>\n' \
                        '        <tr>\n' \
                        '            <th>Short name</th>\n' \
                        '            <th>Description</th>\n' \
                        '            <th>Year</th>\n' \
                        '            <th>Manufacturer</th>\n' \
                        '            <th>Runnable</th>\n' \
                        '            <th>Parent</th>\n' \
                        '        </tr>\n' \
                        '    </thead>\n' \
                        '    <tbody>\n'.encode('utf-8')
                first = False
            yield self.machine_row(machine_info)
        if first:
            yield '<p>No machines found.</p>\n'.encode('utf-8')
        else:
            yield '    </tbody>\n</table>\n<script>make_table_sortable(document.getElementById("tbl-machines"));</script>\n'.encode('utf-8')

        yield '</body>\n</html>\n'.encode('utf-8')

    def linked_title(self, filename, linkfinal=False):
        parts = filename.split('/')
        final = parts[-1]
        del parts[-1]
        uri = urlparse.urljoin(self.application_uri, 'sourcefile')
        title = ''
        for part in parts:
            uri = urlparse.urljoin(uri + '/', urlquote(part))
            title += '<a href="{0}">{1}</a>/'.format(cgi.escape(uri, True), cgi.escape(part))
        if linkfinal:
            uri = urlparse.urljoin(uri + '/', urlquote(final))
            return title + '<a href="{0}">{1}</a>'.format(cgi.escape(uri, True), cgi.escape(final))
        else:
            return title + final

    def machine_row(self, machine_info):
        return (htmltmpl.SOURCEFILE_ROW_PARENT if machine_info['cloneof'] is None else htmltmpl.SOURCEFILE_ROW_CLONE).substitute(
                machinehref=self.machine_href(machine_info['shortname']),
                parenthref=self.machine_href(machine_info['cloneof'] or '__invalid'),
                shortname=cgi.escape(machine_info['shortname']),
                description=cgi.escape(machine_info['description']),
                year=cgi.escape(machine_info['year'] or ''),
                manufacturer=cgi.escape(machine_info['manufacturer'] or ''),
                runnable=cgi.escape('Yes' if machine_info['runnable'] else 'No'),
                parent=cgi.escape(machine_info['cloneof'] or '')).encode('utf-8')


class RomIdentHandler(QueryPageHandler):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super(QueryPageHandler, self).__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
        self.dbcurs = app.dbconn.cursor()

    def __iter__(self):
        if self.environ['PATH_INFO']:
            self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(404)
        elif self.environ['REQUEST_METHOD'] != 'GET':
            self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(405)
        else:
            self.start_response('200 OK', [('Content-type', 'text/html; chearset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.form_page()

    def form_page(self):
        yield htmltmpl.ROMIDENT_PAGE.substitute(
                app=self.js_escape(cgi.escape(self.application_uri, True)),
                assets=self.js_escape(cgi.escape(urlparse.urljoin(self.application_uri, 'static'), True))).encode('utf-8')


class BiosRpcHandler(MachineRpcHandlerBase):
    def data_page(self, machine):
        result = { }
        for name, description, isdefault in self.dbcurs.get_biossets(machine):
            result[name] = { 'description': description, 'isdefault': True if isdefault else False }
        yield json.dumps(result).encode('utf-8')


class FlagsRpcHandler(MachineRpcHandlerBase):
    def data_page(self, machine):
        result = { 'features': { } }
        for feature, status, overall in self.dbcurs.get_feature_flags(machine):
            detail = { }
            if status == 1:
                detail['status'] = 'imperfect'
            elif status > 1:
                detail['status'] = 'unemulated'
            if overall == 1:
                detail['overall'] = 'imperfect'
            elif overall > 1:
                detail['overall'] = 'unemulated'
            result['features'][feature] = detail
        yield json.dumps(result).encode('utf-8')


class SlotsRpcHandler(MachineRpcHandlerBase):
    def data_page(self, machine):
        result = { 'defaults': { }, 'slots': { } }

        # get defaults and slot options
        for slot, default in self.dbcurs.get_slot_defaults(machine):
            result['defaults'][slot] = default
        prev = None
        for slot, option, shortname, description in self.dbcurs.get_slot_options(machine):
            if slot != prev:
                if slot in result['slots']:
                    options = result['slots'][slot]
                else:
                    options = { }
                    result['slots'][slot] = options
                prev = slot
            options[option] = { 'device': shortname, 'description': description }

        # remove slots that come from default cards in other slots
        for slot in tuple(result['slots'].keys()):
            slot += ':'
            for candidate in tuple(result['slots'].keys()):
                if candidate.startswith(slot):
                    del result['slots'][candidate]

        yield json.dumps(result).encode('utf-8')


class RomDumpsRpcHandler(QueryPageHandler):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super(RomDumpsRpcHandler, self).__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)

    def __iter__(self):
        if self.environ['PATH_INFO']:
            self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(404)
        elif self.environ['REQUEST_METHOD'] != 'GET':
            self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(405)
        else:
            try:
                args = urlparse.parse_qs(self.environ['QUERY_STRING'], keep_blank_values=True, strict_parsing=True)
                crc = args.get('crc')
                sha1 = args.get('sha1')
                if (len(args) == 2) and (crc is not None) and (len(crc) == 1) and (sha1 is not None) and (len(sha1) == 1):
                    crc = int(crc[0], 16)
                    sha1 = sha1[0]
                    self.start_response('200 OK', [('Content-type', 'application/json; chearset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                    return self.data_page(crc, sha1)
            except BaseException as e:
                pass
            self.start_response('500 %s' % (self.STATUS_MESSAGE[500], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(500)

    def data_page(self, crc, sha1):
        result = { }
        for shortname, description, label, bad in self.dbcurs.get_rom_dumps(crc, sha1):
            machine = result.get(shortname)
            if machine is None:
                machine = { 'description': description, 'matches': [ ] }
                result[shortname] = machine
            machine['matches'].append({ 'name': label, 'bad': bool(bad) })
        yield json.dumps(result).encode('utf-8')


class DiskDumpsRpcHandler(QueryPageHandler):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super(DiskDumpsRpcHandler, self).__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)

    def __iter__(self):
        if self.environ['PATH_INFO']:
            self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(404)
        elif self.environ['REQUEST_METHOD'] != 'GET':
            self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(405)
        else:
            try:
                args = urlparse.parse_qs(self.environ['QUERY_STRING'], keep_blank_values=True, strict_parsing=True)
                sha1 = args.get('sha1')
                if (len(args) == 1) and (sha1 is not None) and (len(sha1) == 1):
                    sha1 = sha1[0]
                    self.start_response('200 OK', [('Content-type', 'application/json; chearset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                    return self.data_page(sha1)
            except BaseException as e:
                pass
            self.start_response('500 %s' % (self.STATUS_MESSAGE[500], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(500)

    def data_page(self, sha1):
        result = { }
        for shortname, description, label, bad in self.dbcurs.get_disk_dumps(sha1):
            machine = result.get(shortname)
            if machine is None:
                machine = { 'description': description, 'matches': [ ] }
                result[shortname] = machine
            machine['matches'].append({ 'name': label, 'bad': bool(bad) })
        yield json.dumps(result).encode('utf-8')


class MiniMawsApp(object):
    JS_ESCAPE = re.compile('([\"\'\\\\])')
    RPC_SERVICES = {
            'bios':         BiosRpcHandler,
            'flags':        FlagsRpcHandler,
            'slots':        SlotsRpcHandler,
            'romdumps':     RomDumpsRpcHandler,
            'diskdumps':    DiskDumpsRpcHandler }

    def __init__(self, dbfile, **kwargs):
        super(MiniMawsApp, self).__init__(**kwargs)
        self.dbconn = dbaccess.QueryConnection(dbfile)
        self.assetsdir = os.path.join(os.path.dirname(inspect.getfile(self.__class__)), 'assets')
        if not mimetypes.inited:
            mimetypes.init()

    def __call__(self, environ, start_response):
        application_uri = wsgiref.util.application_uri(environ)
        if application_uri[-1] != '/':
            application_uri += '/'
        module = wsgiref.util.shift_path_info(environ)
        if module == 'machine':
            return MachineHandler(self, application_uri, environ, start_response)
        elif module == 'sourcefile':
            return SourceFileHandler(self, application_uri, environ, start_response)
        elif module == 'romident':
            return RomIdentHandler(self, application_uri, environ, start_response)
        elif module == 'static':
            return AssetHandler(self.assetsdir, self, application_uri, environ, start_response)
        elif module == 'rpc':
            service = wsgiref.util.shift_path_info(environ)
            if not service:
                return ErrorPageHandler(403, self, application_uri, environ, start_response)
            elif service in self.RPC_SERVICES:
                return self.RPC_SERVICES[service](self, application_uri, environ, start_response)
            else:
                return ErrorPageHandler(404, self, application_uri, environ, start_response)
        elif not module:
            return ErrorPageHandler(403, self, application_uri, environ, start_response)
        else:
            return ErrorPageHandler(404, self, application_uri, environ, start_response)

    def js_escape(self, str):
        return self.JS_ESCAPE.sub('\\\\\\1', str).replace('\0', '\\0')
