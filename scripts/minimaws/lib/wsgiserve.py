#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

from . import dbaccess
from . import htmltmpl

import html
import inspect
import json
import mimetypes
import os.path
import re
import sys
import urllib
import urllib.parse
import wsgiref.util

htmlescape = html.escape
shiftpath = wsgiref.util.shift_path_info
urljoin = urllib.parse.urljoin
urlparsequery = urllib.parse.parse_qs
urlquote = urllib.parse.quote


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
        super().__init__(**kwargs)
        self.app = app
        self.js_escape = app.js_escape
        self.application_uri = application_uri
        self.environ = environ
        self.start_response = start_response

    def error_page(self, code):
        yield htmltmpl.ERROR_PAGE.substitute(code=htmlescape('%d' % (code, )), message=htmlescape(self.STATUS_MESSAGE[code])).encode('utf-8')


class ErrorPageHandler(HandlerBase):
    def __init__(self, code, app, application_uri, environ, start_response, **kwargs):
        super().__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
        self.code = code
        self.start_response('%d %s' % (self.code, self.STATUS_MESSAGE[code]), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])

    def __iter__(self):
        return self.error_page(self.code)


class AssetHandler(HandlerBase):
    EXTENSIONMAP = { '.js': 'application/javascript', '.svg': 'image/svg+xml' }

    def __init__(self, directory, app, application_uri, environ, start_response, **kwargs):
        super().__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
        self.directory = directory
        self.asset = shiftpath(environ)

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
                    base, extension = os.path.splitext(path)
                    mimetype = self.EXTENSIONMAP.get(extension)
                    if mimetype is None:
                        mimetype, encoding = mimetypes.guess_type(path)
                    self.start_response('200 OK', [('Content-type', mimetype or 'application/octet-stream'), ('Cache-Control', 'public, max-age=3600')])
                    return wsgiref.util.FileWrapper(f)
                except:
                    self.start_response('500 %s' % (self.STATUS_MESSAGE[500], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                    return self.error_page(500)


class QueryPageHandler(HandlerBase):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super().__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
        self.dbcurs = app.dbconn.cursor()

    def machine_href(self, shortname):
        return htmlescape(urljoin(self.application_uri, 'machine/%s' % (urlquote(shortname), )), True)

    def sourcefile_href(self, sourcefile):
        return htmlescape(urljoin(self.application_uri, 'sourcefile/%s' % (urlquote(sourcefile), )), True)

    def softwarelist_href(self, softwarelist):
        return htmlescape(urljoin(self.application_uri, 'softwarelist/%s' % (urlquote(softwarelist), )), True)

    def software_href(self, softwarelist, software):
        return htmlescape(urljoin(self.application_uri, 'softwarelist/%s/%s' % (urlquote(softwarelist), urlquote(software))), True)

    def bios_data(self, machine):
        result = { }
        for name, description, isdefault in self.dbcurs.get_biossets(machine):
            result[name] = { 'description': description, 'isdefault': True if isdefault else False }
        return result

    def flags_data(self, machine):
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
        return result

    def slot_data(self, machine):
        result = { 'defaults': { }, 'slots': { } }

        # get slot options
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

        # if there are any slots, get defaults
        if result['slots']:
            for slot, default in self.dbcurs.get_slot_defaults(machine):
                result['defaults'][slot] = default

            # remove slots that come from default cards in other slots
            for slot in tuple(result['slots'].keys()):
                slot += ':'
                for candidate in tuple(result['slots'].keys()):
                    if candidate.startswith(slot):
                        del result['slots'][candidate]

        return result

    def softwarelist_data(self, machine):
        result = { }

        # get software lists referenced by machine
        for softwarelist in self.dbcurs.get_machine_softwarelists(machine):
            result[softwarelist['tag']] = {
                    'status':               softwarelist['status'],
                    'shortname':            softwarelist['shortname'],
                    'description':          softwarelist['description'],
                    'total':                softwarelist['total'],
                    'supported':            softwarelist['supported'],
                    'partiallysupported':   softwarelist['partiallysupported'],
                    'unsupported':          softwarelist['unsupported'] }

        # remove software lists that come from default cards in slots
        if result:
            for slot, default in self.dbcurs.get_slot_defaults(machine):
                slot += ':'
                for candidate in tuple(result.keys()):
                    if candidate.startswith(slot):
                        del result[candidate]

        return result


class MachineRpcHandlerBase(QueryPageHandler):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super().__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
        self.shortname = shiftpath(environ)

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
        super().__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
        self.shortname = shiftpath(environ)

    def __iter__(self):
        if not self.shortname:
            # could probably list machines here or something
            self.start_response('403 %s' % (self.STATUS_MESSAGE[403], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(403)
        elif self.environ['PATH_INFO']:
            self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(404)
        else:
            machine_info = self.dbcurs.get_machine_details(self.shortname).fetchone()
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
                app=self.js_escape(htmlescape(self.application_uri, True)),
                assets=self.js_escape(htmlescape(urljoin(self.application_uri, 'static'), True)),
                sourcehref=self.sourcefile_href(machine_info['sourcefile']),
                description=htmlescape(description),
                shortname=htmlescape(self.shortname),
                isdevice=htmlescape('Yes' if machine_info['isdevice'] else 'No'),
                runnable=htmlescape('Yes' if machine_info['runnable'] else 'No'),
                sourcefile=htmlescape(machine_info['sourcefile'])).encode('utf-8')
        if machine_info['year'] is not None:
            yield (
                    '    <tr><th>Year:</th><td>%s</td></tr>\n' \
                    '    <tr><th>Manufacturer:</th><td>%s</td></tr>\n' %
                    (htmlescape(machine_info['year']), htmlescape(machine_info['Manufacturer']))).encode('utf-8')
        if machine_info['cloneof'] is not None:
            parent = self.dbcurs.listfull(machine_info['cloneof']).fetchone()
            if parent:
                yield (
                        '    <tr><th>Parent machine:</th><td><a href="%s">%s (%s)</a></td></tr>\n' %
                        (self.machine_href(machine_info['cloneof']), htmlescape(parent[1]), htmlescape(machine_info['cloneof']))).encode('utf-8')
            else:
                yield (
                        '    <tr><th>Parent machine:</th><td><a href="%s">%s</a></td></tr>\n' %
                        (self.machine_href(machine_info['cloneof']), htmlescape(machine_info['cloneof']))).encode('utf-8')
        if (machine_info['romof'] is not None) and (machine_info['romof'] != machine_info['cloneof']):
            parent = self.dbcurs.listfull(machine_info['romof']).fetchone()
            if parent:
                yield (
                        '    <tr><th>Parent ROM set:</th><td><a href="%s">%s (%s)</a></td></tr>\n' %
                        (self.machine_href(machine_info['romof']), htmlescape(parent[1]), htmlescape(machine_info['romof']))).encode('utf-8')
            else:
                yield (
                        '    <tr><th>Parent ROM set:</th><td><a href="%s">%s</a></td></tr>\n' %
                        (self.machine_href(machine_info['romof']), htmlescape(machine_info['romof']))).encode('utf-8')
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

        # make a table of clones
        first = True
        for clone, clonedescription, cloneyear, clonemanufacturer in self.dbcurs.get_clones(self.shortname):
            if first:
                yield htmltmpl.MACHINE_CLONES_PROLOGUE.substitute().encode('utf-8')
                first = False
            yield htmltmpl.MACHINE_CLONES_ROW.substitute(
                    href=self.machine_href(clone),
                    shortname=htmlescape(clone),
                    description=htmlescape(clonedescription),
                    year=htmlescape(cloneyear or ''),
                    manufacturer=htmlescape(clonemanufacturer or '')).encode('utf-8')
        if not first:
            yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-clones').encode('utf-8')
            yield '<script>make_collapsible(document.getElementById("heading-clones"), document.getElementById("tbl-clones"));</script>\n'.encode('utf-8')

        # make a table of software lists
        yield htmltmpl.MACHINE_SOFTWARELISTS_TABLE_PROLOGUE.substitute().encode('utf-8')
        for softwarelist in self.dbcurs.get_machine_softwarelists(id):
            total = softwarelist['total']
            yield htmltmpl.MACHINE_SOFTWARELISTS_TABLE_ROW.substitute(
                    rowid=htmlescape(softwarelist['tag'].replace(':', '-'), True),
                    href=self.softwarelist_href(softwarelist['shortname']),
                    shortname=htmlescape(softwarelist['shortname']),
                    description=htmlescape(softwarelist['description']),
                    status=htmlescape(softwarelist['status']),
                    total=htmlescape('%d' % (total, )),
                    supported=htmlescape('%.1f%%' % (softwarelist['supported'] * 100.0 / (total or 1), )),
                    partiallysupported=htmlescape('%.1f%%' % (softwarelist['partiallysupported'] * 100.0 / (total or 1), )),
                    unsupported=htmlescape('%.1f%%' % (softwarelist['unsupported'] * 100.0 / (total or 1), ))).encode('utf-8')
        yield htmltmpl.MACHINE_SOFTWARELISTS_TABLE_EPILOGUE.substitute().encode('utf-8')

        # allow system BIOS selection
        haveoptions = False
        for name, desc, isdef in self.dbcurs.get_biossets(id):
            if not haveoptions:
                haveoptions = True;
                yield htmltmpl.MACHINE_OPTIONS_HEADING.substitute().encode('utf-8')
                yield htmltmpl.MACHINE_BIOS_PROLOGUE.substitute().encode('utf-8')
            yield htmltmpl.MACHINE_BIOS_OPTION.substitute(
                    name=htmlescape(name, True),
                    description=htmlescape(desc),
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
                    name=htmlescape(name, True),
                    size=htmlescape('{:,}'.format(size)),
                    isdefault=('yes' if isdef else 'no')).encode('utf-8')
        if not first:
            yield '    </select>\n    <script>set_default_ram_option();</script>\n'.encode('utf-8')

        # placeholder for machine slots - populated by client-side JavaScript
        if self.dbcurs.count_slots(id):
            if not haveoptions:
                haveoptions = True
                yield htmltmpl.MACHINE_OPTIONS_HEADING.substitute().encode('utf-8')
            yield htmltmpl.MACHINE_SLOTS_PLACEHOLDER_PROLOGUE.substitute().encode('utf=8')
            pending = set((self.shortname, ))
            added = set((self.shortname, ))
            haveextra = set()
            while pending:
                requested = pending.pop()
                slots = self.slot_data(self.dbcurs.get_machine_id(requested))
                yield ('        slot_info[%s] = %s;\n' % (self.sanitised_json(requested), self.sanitised_json(slots))).encode('utf-8')
                for slotname, slot in slots['slots'].items():
                    for choice, card in slot.items():
                        carddev = card['device']
                        if carddev not in added:
                            pending.add(carddev)
                            added.add(carddev)
                        if (carddev not in haveextra) and (slots['defaults'].get(slotname) == choice):
                            haveextra.add(carddev)
                            cardid = self.dbcurs.get_machine_id(carddev)
                            carddev = self.sanitised_json(carddev)
                            yield (
                                    '        bios_sets[%s] = %s;\n        machine_flags[%s] = %s;\n        softwarelist_info[%s] = %s;\n' %
                                    (carddev, self.sanitised_json(self.bios_data(cardid)), carddev, self.sanitised_json(self.flags_data(cardid)), carddev, self.sanitised_json(self.softwarelist_data(cardid)))).encode('utf-8')
            yield htmltmpl.MACHINE_SLOTS_PLACEHOLDER_EPILOGUE.substitute(
                    machine=self.sanitised_json(self.shortname)).encode('utf=8')

        # add disclosure triangle for options if present
        if haveoptions:
            yield htmltmpl.MACHINE_OPTIONS_EPILOGUE.substitute().encode('utf=8')

        # list devices referenced by this system/device
        first = True
        for name, desc, src in self.dbcurs.get_devices_referenced(id):
            if first:
                yield \
                        '<h2 id="heading-dev-refs">Devices Referenced</h2>\n' \
                        '<table id="tbl-dev-refs">\n' \
                        '    <thead>\n' \
                        '        <tr><th>Short name</th><th>Description</th><th>Source file</th></tr>\n' \
                        '    </thead>\n' \
                        '    <tbody>\n'.encode('utf-8')
                first = False
            yield self.machine_row(name, desc, src)
        if not first:
            yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-dev-refs').encode('utf-8')
            yield '<script>make_collapsible(document.getElementById("heading-dev-refs"), document.getElementById("tbl-dev-refs"));</script>\n'.encode('utf-8')

        # list slots where this device is an option
        first = True
        for name, desc, slot, opt, src in self.dbcurs.get_compatible_slots(id):
            if (first):
                yield \
                        '<h2 id="heading-comp-slots">Compatible Slots</h2>\n' \
                        '<table id="tbl-comp-slots">\n' \
                        '    <thead>\n' \
                        '        <tr><th>Short name</th><th>Description</th><th>Slot</th><th>Choice</th><th>Source file</th></tr>\n' \
                        '    </thead>\n' \
                        '    <tbody>\n'.encode('utf-8')
                first = False
            yield htmltmpl.COMPATIBLE_SLOT_ROW.substitute(
                    machinehref=self.machine_href(name),
                    sourcehref=self.sourcefile_href(src),
                    shortname=htmlescape(name),
                    description=htmlescape(desc),
                    sourcefile=htmlescape(src),
                    slot=htmlescape(slot),
                    slotoption=htmlescape(opt)).encode('utf-8')
        if not first:
            yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-comp-slots').encode('utf-8')
            yield '<script>make_collapsible(document.getElementById("heading-comp-slots"), document.getElementById("tbl-comp-slots"));</script>\n'.encode('utf-8')

        # list systems/devices that reference this device
        first = True
        for name, desc, src in self.dbcurs.get_device_references(id):
            if first:
                yield \
                        '<h2 id="heading-ref-by">Referenced By</h2>\n' \
                        '<table id="tbl-ref-by">\n' \
                        '    <thead>\n' \
                        '        <tr><th>Short name</th><th>Description</th><th>Source file</th></tr>\n' \
                        '    </thead>\n' \
                        '    <tbody>\n'.encode('utf-8')
                first = False
            yield self.machine_row(name, desc, src)
        if not first:
            yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-ref-by').encode('utf-8')
            yield '<script>make_collapsible(document.getElementById("heading-ref-by"), document.getElementById("tbl-ref-by"));</script>\n'.encode('utf-8')

        yield '</html>\n'.encode('utf-8')

    def machine_row(self, shortname, description, sourcefile):
        return (htmltmpl.MACHINE_ROW if description is not None else htmltmpl.EXCL_MACHINE_ROW).substitute(
                machinehref=self.machine_href(shortname),
                sourcehref=self.sourcefile_href(sourcefile),
                shortname=htmlescape(shortname),
                description=htmlescape(description or ''),
                sourcefile=htmlescape(sourcefile or '')).encode('utf-8')

    @staticmethod
    def sanitised_json(data):
        return json.dumps(data).replace('<', '\\u003c').replace('>', '\\u003e')


class SourceFileHandler(QueryPageHandler):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super().__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)

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
            title = 'Source Files: ' + htmlescape(pattern)
        yield htmltmpl.SOURCEFILE_LIST_PROLOGUE.substitute(
                assets=htmlescape(urljoin(self.application_uri, 'static'), True),
                title=title,
                heading=heading).encode('utf-8')
        for filename, machines in self.dbcurs.get_sourcefiles(pattern):
            yield htmltmpl.SOURCEFILE_LIST_ROW.substitute(
                    sourcefile=self.linked_title(filename, True),
                    machines=htmlescape('%d' % (machines, ))).encode('utf-8')
        yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-sourcefiles').encode('utf-8')

    def sourcefile_page(self, id):
        yield htmltmpl.SOURCEFILE_PROLOGUE.substitute(
                assets=htmlescape(urljoin(self.application_uri, 'static'), True),
                filename=htmlescape(self.filename),
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
            yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-machines').encode('utf-8')

        yield '</body>\n</html>\n'.encode('utf-8')

    def linked_title(self, filename, linkfinal=False):
        parts = filename.split('/')
        final = parts[-1]
        del parts[-1]
        uri = urljoin(self.application_uri, 'sourcefile')
        title = ''
        for part in parts:
            uri = urljoin(uri + '/', urlquote(part))
            title += '<a href="{0}">{1}</a>/'.format(htmlescape(uri, True), htmlescape(part))
        if linkfinal:
            uri = urljoin(uri + '/', urlquote(final))
            return title + '<a href="{0}">{1}</a>'.format(htmlescape(uri, True), htmlescape(final))
        else:
            return title + final

    def machine_row(self, machine_info):
        return (htmltmpl.SOURCEFILE_ROW_PARENT if machine_info['cloneof'] is None else htmltmpl.SOURCEFILE_ROW_CLONE).substitute(
                machinehref=self.machine_href(machine_info['shortname']),
                parenthref=self.machine_href(machine_info['cloneof'] or '__invalid'),
                shortname=htmlescape(machine_info['shortname']),
                description=htmlescape(machine_info['description']),
                year=htmlescape(machine_info['year'] or ''),
                manufacturer=htmlescape(machine_info['manufacturer'] or ''),
                runnable=htmlescape('Yes' if machine_info['runnable'] else 'No'),
                parent=htmlescape(machine_info['cloneof'] or '')).encode('utf-8')


class SoftwareListHandler(QueryPageHandler):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super().__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
        self.shortname = shiftpath(environ)
        self.software = shiftpath(environ)

    def __iter__(self):
        if self.environ['PATH_INFO']:
            self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(404)
        elif self.software and ('*' not in self.software) and ('?' not in self.software):
            software_info = self.dbcurs.get_software_details(self.shortname, self.software).fetchone()
            if not software_info:
                self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(404)
            elif self.environ['REQUEST_METHOD'] != 'GET':
                self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(405)
            else:
                self.start_response('200 OK', [('Content-type', 'text/html; chearset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                return self.software_page(software_info)
        elif self.software or (self.shortname and ('*' not in self.shortname) and ('?' not in self.shortname)):
            softwarelist_info = self.dbcurs.get_softwarelist_details(self.shortname, self.software or None).fetchone()
            if not softwarelist_info:
                self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(404)
            elif self.environ['REQUEST_METHOD'] != 'GET':
                self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(405)
            else:
                self.start_response('200 OK', [('Content-type', 'text/html; chearset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                return self.softwarelist_page(softwarelist_info, self.software or None)
        else:
            if self.environ['REQUEST_METHOD'] != 'GET':
                self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
                return self.error_page(405)
            else:
                self.start_response('200 OK', [('Content-type', 'text/html; chearset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
                return self.softwarelist_listing_page(self.shortname or None)

    def softwarelist_listing_page(self, pattern):
        if not pattern:
            title = heading = 'All Software Lists'
        else:
            title = heading = 'Software Lists: ' + htmlescape(pattern)
        yield htmltmpl.SOFTWARELIST_LIST_PROLOGUE.substitute(
                assets=htmlescape(urljoin(self.application_uri, 'static'), True),
                title=title,
                heading=heading).encode('utf-8')
        for shortname, description, total, supported, partiallysupported, unsupported in self.dbcurs.get_softwarelists(pattern):
            yield htmltmpl.SOFTWARELIST_LIST_ROW.substitute(
                    href=self.softwarelist_href(shortname),
                    shortname=htmlescape(shortname),
                    description=htmlescape(description),
                    total=htmlescape('%d' % (total, )),
                    supported=htmlescape('%.1f%%' % (supported * 100.0 / (total or 1), )),
                    partiallysupported=htmlescape('%.1f%%' % (partiallysupported * 100.0 / (total or 1), )),
                    unsupported=htmlescape('%.1f%%' % (unsupported * 100.0 / (total or 1), ))).encode('utf-8')
        yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-softwarelists').encode('utf-8')

    def softwarelist_page(self, softwarelist_info, pattern):
        if not pattern:
            title = 'Software List: %s (%s)' % (htmlescape(softwarelist_info['description']), htmlescape(softwarelist_info['shortname']))
            heading = htmlescape(softwarelist_info['description'])
        else:
            title = 'Software List: %s (%s): %s' % (htmlescape(softwarelist_info['description']), htmlescape(softwarelist_info['shortname']), htmlescape(pattern))
            heading = '<a href="%s">%s</a>: %s' % (self.softwarelist_href(softwarelist_info['shortname']), htmlescape(softwarelist_info['description']), htmlescape(pattern))
        yield htmltmpl.SOFTWARELIST_PROLOGUE.substitute(
                assets=htmlescape(urljoin(self.application_uri, 'static'), True),
                title=title,
                heading=heading,
                shortname=htmlescape(softwarelist_info['shortname']),
                total=htmlescape('%d' % (softwarelist_info['total'], )),
                supported=htmlescape('%d' % (softwarelist_info['supported'], )),
                supportedpc=htmlescape('%.1f' % (softwarelist_info['supported'] * 100.0 / (softwarelist_info['total'] or 1), )),
                partiallysupported=htmlescape('%d' % (softwarelist_info['partiallysupported'], )),
                partiallysupportedpc=htmlescape('%.1f' % (softwarelist_info['partiallysupported'] * 100.0 / (softwarelist_info['total'] or 1), )),
                unsupported=htmlescape('%d' % (softwarelist_info['unsupported'], )),
                unsupportedpc=htmlescape('%.1f' % (softwarelist_info['unsupported'] * 100.0 / (softwarelist_info['total'] or 1), ))).encode('utf-8')

        if softwarelist_info['notes'] is not None:
            yield htmltmpl.SOFTWARELIST_NOTES_PROLOGUE.substitute().encode('utf-8')
            first = True
            for line in softwarelist_info['notes'].strip().splitlines():
                if line:
                    yield (('<p>%s' if first else '<br />\n%s') % (htmlescape(line), )).encode('utf-8')
                    first = False
                elif not first:
                    yield '</p>\n'.encode('utf-8')
                    first = True
            if not first:
                    yield '</p>\n'.encode('utf-8')
            yield htmltmpl.SOFTWARELIST_NOTES_EPILOGUE.substitute().encode('utf-8')

        first = True
        for machine_info in self.dbcurs.get_softwarelist_machines(softwarelist_info['id']):
            if first:
                yield htmltmpl.SOFTWARELIST_MACHINE_TABLE_HEADER.substitute().encode('utf-8')
                first = False
            yield htmltmpl.SOFTWARELIST_MACHINE_TABLE_ROW.substitute(
                    machinehref=self.machine_href(machine_info['shortname']),
                    shortname=htmlescape(machine_info['shortname']),
                    description=htmlescape(machine_info['description']),
                    year=htmlescape(machine_info['year'] or ''),
                    manufacturer=htmlescape(machine_info['manufacturer'] or ''),
                    status=htmlescape(machine_info['status'])).encode('utf-8')
        if not first:
            yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-machines').encode('utf-8')
            yield '<script>make_collapsible(document.getElementById("heading-machines"), document.getElementById("tbl-machines"));</script>\n'.encode('utf-8')

        first = True
        for software_info in self.dbcurs.get_softwarelist_software(softwarelist_info['id'], self.software or None):
            if first:
                yield htmltmpl.SOFTWARELIST_SOFTWARE_TABLE_HEADER.substitute().encode('utf-8')
                first = False
            yield self.software_row(software_info)
        if first:
            yield '<p>No software found.</p>\n'.encode('utf-8')
        else:
            yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-software').encode('utf-8')
            yield '<script>make_collapsible(document.getElementById("heading-software"), document.getElementById("tbl-software"));</script>\n'.encode('utf-8')

        yield '</body>\n</html>\n'.encode('utf-8')

    def software_page(self, software_info):
        yield htmltmpl.SOFTWARE_PROLOGUE.substitute(
                assets=htmlescape(urljoin(self.application_uri, 'static'), True),
                title=htmlescape(software_info['description']),
                heading=htmlescape(software_info['description']),
                softwarelisthref=self.softwarelist_href(self.shortname),
                softwarelistdescription=htmlescape(software_info['softwarelistdescription']),
                softwarelist=htmlescape(self.shortname),
                shortname=htmlescape(software_info['shortname']),
                year=htmlescape(software_info['year']),
                publisher=htmlescape(software_info['publisher'])).encode('utf-8')
        if software_info['parent'] is not None:
            yield ('    <tr><th>Parent:</th><td><a href="%s">%s</a></td>\n' % (self.software_href(software_info['parentsoftwarelist'], software_info['parent']), htmlescape(software_info['parentdescription']))).encode('utf-8')
        yield ('    <tr><th>Supported:</th><td>%s</td>\n' % (self.format_supported(software_info['supported']), )).encode('utf-8')
        for name, value in self.dbcurs.get_software_info(software_info['id']):
            yield ('    <tr><th>%s:</th><td>%s</td>\n' % (htmlescape(name), htmlescape(value))).encode('utf-8')
        yield '</table>\n\n'.encode('utf-8')

        first = True
        for clone_info in self.dbcurs.get_software_clones(software_info['id']):
            if first:
                yield htmltmpl.SOFTWARE_CLONES_PROLOGUE.substitute().encode('utf-8')
                first = False
            yield self.clone_row(clone_info)
        if not first:
            yield htmltmpl.SORTABLE_TABLE_EPILOGUE.substitute(id='tbl-clones').encode('utf-8')
            yield '<script>make_collapsible(document.getElementById("heading-clones"), document.getElementById("tbl-clones"));</script>\n'.encode('utf-8')

        if software_info['notes'] is not None:
            yield htmltmpl.SOFTWARE_NOTES_PROLOGUE.substitute().encode('utf-8')
            first = True
            for line in software_info['notes'].strip().splitlines():
                if line:
                    yield (('<p>%s' if first else '<br />\n%s') % (htmlescape(line), )).encode('utf-8')
                    first = False
                elif not first:
                    yield '</p>\n'.encode('utf-8')
                    first = True
            if not first:
                    yield '</p>\n'.encode('utf-8')
            yield htmltmpl.SOFTWARE_NOTES_EPILOGUE.substitute().encode('utf-8')

        parts = self.dbcurs.get_software_parts(software_info['id']).fetchall()
        first = True
        for id, partname, interface, part_id in parts:
            if first:
                yield htmltmpl.SOFTWARE_PARTS_PROLOGUE.substitute().encode('utf-8')
                first = False
            yield htmltmpl.SOFTWARE_PART_PROLOGUE.substitute(
                    heading=htmlescape(('%s (%s)' % (part_id, partname)) if part_id is not None else partname),
                    shortname=htmlescape(partname),
                    interface=htmlescape(interface)).encode('utf-8')
            for name, value in self.dbcurs.get_softwarepart_features(id):
                yield ('        <tr><th>%s:</th><td>%s</td>\n' % (htmlescape(name), htmlescape(value))).encode('utf-8')
            yield '    </table>\n\n'.encode('utf-8')
        if not first:
            yield htmltmpl.SOFTWARE_PARTS_EPILOGUE.substitute().encode('utf-8')

        yield '</body>\n</html>\n'.encode('utf-8')

    def software_row(self, software_info):
        parent = software_info['parent']
        return htmltmpl.SOFTWARELIST_SOFTWARE_ROW.substitute(
                softwarehref=self.software_href(self.shortname, software_info['shortname']),
                shortname=htmlescape(software_info['shortname']),
                description=htmlescape(software_info['description']),
                year=htmlescape(software_info['year']),
                publisher=htmlescape(software_info['publisher']),
                supported=self.format_supported(software_info['supported']),
                parts=htmlescape('%d' % (software_info['parts'], )),
                baddumps=htmlescape('%d' % (software_info['baddumps'], )),
                parent='<a href="%s">%s</a>' % (self.software_href(software_info['parentsoftwarelist'], parent), htmlescape(parent)) if parent is not None else '').encode('utf-8')

    def clone_row(self, clone_info):
        return htmltmpl.SOFTWARE_CLONES_ROW.substitute(
                href=self.software_href(clone_info['softwarelist'], clone_info['shortname']),
                shortname=htmlescape(clone_info['shortname']),
                description=htmlescape(clone_info['description']),
                year=htmlescape(clone_info['year']),
                publisher=htmlescape(clone_info['publisher']),
                supported=self.format_supported(clone_info['supported'])).encode('utf-8')

    @staticmethod
    def format_supported(supported):
        return 'Yes' if supported == 0 else 'Partial' if supported == 1 else 'No'


class RomIdentHandler(QueryPageHandler):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super().__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)
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
                app=self.js_escape(htmlescape(self.application_uri, True)),
                assets=self.js_escape(htmlescape(urljoin(self.application_uri, 'static'), True))).encode('utf-8')


class BiosRpcHandler(MachineRpcHandlerBase):
    def data_page(self, machine):
        result = { }
        for name, description, isdefault in self.dbcurs.get_biossets(machine):
            result[name] = { 'description': description, 'isdefault': True if isdefault else False }
        yield json.dumps(result).encode('utf-8')


class FlagsRpcHandler(MachineRpcHandlerBase):
    def data_page(self, machine):
        yield json.dumps(self.flags_data(machine)).encode('utf-8')


class SlotsRpcHandler(MachineRpcHandlerBase):
    def data_page(self, machine):
        yield json.dumps(self.slot_data(machine)).encode('utf-8')


class SoftwareListsRpcHandler(MachineRpcHandlerBase):
    def data_page(self, machine):
        yield json.dumps(self.softwarelist_data(machine)).encode('utf-8')


class RomDumpsRpcHandler(QueryPageHandler):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super().__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)

    def __iter__(self):
        if self.environ['PATH_INFO']:
            self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(404)
        elif self.environ['REQUEST_METHOD'] != 'GET':
            self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(405)
        else:
            try:
                args = urlparsequery(self.environ['QUERY_STRING'], keep_blank_values=True, strict_parsing=True)
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
        machines = { }
        for shortname, description, label, bad in self.dbcurs.get_rom_dumps(crc, sha1):
            machine = machines.get(shortname)
            if machine is None:
                machine = { 'description': description, 'matches': [ ] }
                machines[shortname] = machine
            machine['matches'].append({ 'name': label, 'bad': bool(bad) })

        software = { }
        for softwarelist, softwarelistdescription, shortname, description, part, part_id, label, bad in self.dbcurs.get_software_rom_dumps(crc, sha1):
            listinfo = software.get(softwarelist)
            if listinfo is None:
                listinfo = { 'description': softwarelistdescription, 'software': { } }
                software[softwarelist] = listinfo
            softwareinfo = listinfo['software'].get(shortname)
            if softwareinfo is None:
                softwareinfo = { 'description': description, 'parts': { } }
                listinfo['software'][shortname] = softwareinfo
            partinfo = softwareinfo['parts'].get(part)
            if partinfo is None:
                partinfo = { 'matches': [ ] }
                if part_id is not None:
                    partinfo['description'] = part_id
                softwareinfo['parts'][part] = partinfo
            partinfo['matches'].append({ 'name': label, 'bad': bool(bad) })

        result = { 'machines': machines, 'software': software }
        yield json.dumps(result).encode('utf-8')


class DiskDumpsRpcHandler(QueryPageHandler):
    def __init__(self, app, application_uri, environ, start_response, **kwargs):
        super().__init__(app=app, application_uri=application_uri, environ=environ, start_response=start_response, **kwargs)

    def __iter__(self):
        if self.environ['PATH_INFO']:
            self.start_response('404 %s' % (self.STATUS_MESSAGE[404], ), [('Content-type', 'text/html; charset=utf-8'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(404)
        elif self.environ['REQUEST_METHOD'] != 'GET':
            self.start_response('405 %s' % (self.STATUS_MESSAGE[405], ), [('Content-type', 'text/html; charset=utf-8'), ('Accept', 'GET, HEAD, OPTIONS'), ('Cache-Control', 'public, max-age=3600')])
            return self.error_page(405)
        else:
            try:
                args = urlparsequery(self.environ['QUERY_STRING'], keep_blank_values=True, strict_parsing=True)
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
        machines = { }
        for shortname, description, label, bad in self.dbcurs.get_disk_dumps(sha1):
            machine = machines.get(shortname)
            if machine is None:
                machine = { 'description': description, 'matches': [ ] }
                machines[shortname] = machine
            machine['matches'].append({ 'name': label, 'bad': bool(bad) })

        software = { }
        for softwarelist, softwarelistdescription, shortname, description, part, part_id, label, bad in self.dbcurs.get_software_disk_dumps(sha1):
            listinfo = software.get(softwarelist)
            if listinfo is None:
                listinfo = { 'description': softwarelistdescription, 'software': { } }
                software[softwarelist] = listinfo
            softwareinfo = listinfo['software'].get(shortname)
            if softwareinfo is None:
                softwareinfo = { 'description': description, 'parts': { } }
                listinfo['software'][shortname] = softwareinfo
            partinfo = softwareinfo['parts'].get(part)
            if partinfo is None:
                partinfo = { 'matches': [ ] }
                if part_id is not None:
                    partinfo['description'] = part_id
                softwareinfo['parts'][part] = partinfo
            partinfo['matches'].append({ 'name': label, 'bad': bool(bad) })

        result = { 'machines': machines, 'software': software }
        yield json.dumps(result).encode('utf-8')


class MiniMawsApp(object):
    JS_ESCAPE = re.compile('([\"\'\\\\])')
    RPC_SERVICES = {
            'bios':             BiosRpcHandler,
            'flags':            FlagsRpcHandler,
            'slots':            SlotsRpcHandler,
            'softwarelists':    SoftwareListsRpcHandler,
            'romdumps':         RomDumpsRpcHandler,
            'diskdumps':        DiskDumpsRpcHandler }

    def __init__(self, dbfile, **kwargs):
        super().__init__(**kwargs)
        self.dbconn = dbaccess.QueryConnection(dbfile)
        self.assetsdir = os.path.join(os.path.dirname(inspect.getfile(self.__class__)), 'assets')
        if not mimetypes.inited:
            mimetypes.init()

    def __call__(self, environ, start_response):
        application_uri = wsgiref.util.application_uri(environ)
        if application_uri[-1] != '/':
            application_uri += '/'
        module = shiftpath(environ)
        if module == 'machine':
            return MachineHandler(self, application_uri, environ, start_response)
        elif module == 'sourcefile':
            return SourceFileHandler(self, application_uri, environ, start_response)
        elif module == 'softwarelist':
            return SoftwareListHandler(self, application_uri, environ, start_response)
        elif module == 'romident':
            return RomIdentHandler(self, application_uri, environ, start_response)
        elif module == 'static':
            return AssetHandler(self.assetsdir, self, application_uri, environ, start_response)
        elif module == 'rpc':
            service = shiftpath(environ)
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
