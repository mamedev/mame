#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import string


ERROR_PAGE = string.Template(
        '<!DOCTYPE html>\n' \
        '<html>\n' \
        '<head>\n' \
        '    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">\n' \
        '    <title>${code} ${message}</title>\n' \
        '</head>\n' \
        '<body>\n' \
        '<h1>${message}</h1>\n' \
        '</body>\n' \
        '</html>\n')


SORTABLE_TABLE_EPILOGUE = string.Template(
        '    </tbody>\n'
        '</table>\n'
        '<script>make_table_sortable(document.getElementById("${id}"));</script>\n')

MACHINE_PROLOGUE = string.Template(
        '<!DOCTYPE html>\n' \
        '<html>\n' \
        '<head>\n' \
        '    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">\n' \
        '    <meta http-equiv="Content-Style-Type" content="text/css">\n' \
        '    <meta http-equiv="Content-Script-Type" content="text/javascript">\n' \
        '    <link rel="stylesheet" type="text/css" href="${assets}/style.css">\n' \
        '    <script type="text/javascript">\n' \
        '        var appurl="${app}"\n' \
        '        var assetsurl="${assets}"\n' \
        '    </script>\n' \
        '    <script type="text/javascript" src="${assets}/common.js"></script>\n' \
        '    <script type="text/javascript" src="${assets}/machine.js"></script>\n' \
        '    <title>Machine: ${description} (${shortname})</title>\n' \
        '</head>\n' \
        '<body>\n' \
        '<h1>${description}</h1>\n' \
        '<table class="sysinfo">\n' \
        '    <tr><th>Short name:</th><td>${shortname}</td></tr>\n' \
        '    <tr><th>Is device:</th><td>${isdevice}</td></tr>\n' \
        '    <tr><th>Runnable:</th><td>${runnable}</td></tr>\n' \
        '    <tr><th>Source file:</th><td><a href="${sourcehref}">${sourcefile}</a></td></tr>\n')

MACHINE_OPTIONS_HEADING = string.Template(
        '<h2>Options</h2>\n' \
        '<p>\n' \
        '    Format: <select id="select-options-format" onchange="update_cmd_preview()"><option value="cmd">Command line</option><option value="ini">INI file</option></select>\n' \
        '    <input type="checkbox" id="check-explicit-defaults" onchange="update_cmd_preview()"><label for="check-explicit-defaults">Explicit defaults</label>\n' \
        '</p>\n' \
        '<p id="para-cmd-preview"></p>\n')

MACHINE_BIOS_PROLOGUE = string.Template(
        '<h3>System BIOS</h3>' \
        '<select id="select-system-bios" onchange="update_cmd_preview()">')

MACHINE_BIOS_OPTION = string.Template(
        '    <option value="${name}" data-isdefault="${isdefault}">${name} - ${description}</option>\n')

MACHINE_RAM_PROLOGUE = string.Template(
        '<h3>RAM Size</h3>' \
        '<select id="select-ram-option" onchange="update_cmd_preview()">')

MACHINE_RAM_OPTION = string.Template(
        '    <option value="${name}" data-isdefault="${isdefault}">${name} (${size})</option>\n')

MACHINE_SLOTS_PLACEHOLDER = string.Template(
        '<h3>Slots</h3>\n' \
        '<p id="para-slots-placeholder">Loading slot information&hellip;<p>\n' \
        '<script>fetch_slots("${machine}");</script>\n')

MACHINE_ROW = string.Template(
        '        <tr>\n' \
        '            <td><a href="${machinehref}">${shortname}</a></td>\n' \
        '            <td><a href="${machinehref}">${description}</a></td>\n' \
        '            <td><a href="${sourcehref}">${sourcefile}</a></td>\n' \
        '        </tr>\n')

EXCL_MACHINE_ROW = string.Template(
        '        <tr>\n' \
        '            <td><a href="${machinehref}">${shortname}</a></td>\n' \
        '            <td></td>\n' \
        '            <td></td>\n' \
        '        </tr>\n')

COMPATIBLE_SLOT_ROW = string.Template(
        '        <tr>\n' \
        '            <td><a href="${machinehref}">${shortname}</a></td>\n' \
        '            <td><a href="${machinehref}">${description}</a></td>\n' \
        '            <td>${slot}</td>\n' \
        '            <td>${slotoption}</td>\n' \
        '            <td><a href="${sourcehref}">${sourcefile}</a></td>\n' \
        '        </tr>\n')


SOURCEFILE_PROLOGUE = string.Template(
        '<!DOCTYPE html>\n' \
        '<html>\n' \
        '<head>\n' \
        '    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">\n' \
        '    <meta http-equiv="Content-Style-Type" content="text/css">\n' \
        '    <meta http-equiv="Content-Script-Type" content="text/javascript">\n' \
        '    <link rel="stylesheet" type="text/css" href="${assets}/style.css">\n' \
        '    <script type="text/javascript">var assetsurl="${assets}"</script>\n' \
        '    <script type="text/javascript" src="${assets}/common.js"></script>\n' \
        '    <title>Source File: ${filename}</title>\n' \
        '</head>\n' \
        '<body>\n' \
        '<h1>${title}</h1>\n')

SOURCEFILE_ROW_PARENT = string.Template(
        '        <tr>\n' \
        '            <td><a href="${machinehref}">${shortname}</a></td>\n' \
        '            <td><a href="${machinehref}">${description}</a></td>\n' \
        '            <td>${year}</td>\n' \
        '            <td>${manufacturer}</td>\n' \
        '            <td>${runnable}</td>\n' \
        '            <td></td>\n' \
        '        </tr>\n')

SOURCEFILE_ROW_CLONE = string.Template(
        '        <tr>\n' \
        '            <td><a href="${machinehref}">${shortname}</a></td>\n' \
        '            <td><a href="${machinehref}">${description}</a></td>\n' \
        '            <td>${year}</td>\n' \
        '            <td>${manufacturer}</td>\n' \
        '            <td>${runnable}</td>\n' \
        '            <td><a href="${parenthref}">${parent}</a></td>\n' \
        '        </tr>\n')


SOURCEFILE_LIST_PROLOGUE = string.Template(
        '<!DOCTYPE html>\n' \
        '<html>\n' \
        '<head>\n' \
        '    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">\n' \
        '    <meta http-equiv="Content-Style-Type" content="text/css">\n' \
        '    <meta http-equiv="Content-Script-Type" content="text/javascript">\n' \
        '    <link rel="stylesheet" type="text/css" href="${assets}/style.css">\n' \
        '    <script type="text/javascript">var assetsurl="${assets}"</script>\n' \
        '    <script type="text/javascript" src="${assets}/common.js"></script>\n' \
        '    <title>${title}</title>\n' \
        '</head>\n' \
        '<body>\n' \
        '<h1>${heading}</h1>\n' \
        '<table id="tbl-sourcefiles">\n' \
        '    <thead>\n' \
        '        <tr>\n' \
        '            <th>Source file</th>\n' \
        '            <th class="numeric">Machines</th>\n' \
        '        </tr>\n' \
        '    </thead>\n' \
        '    <tbody>\n')

SOURCEFILE_LIST_ROW = string.Template(
        '        <tr>\n' \
        '            <td>${sourcefile}</td>\n' \
        '            <td style="text-align: right">${machines}</td>\n' \
        '        </tr>\n')
