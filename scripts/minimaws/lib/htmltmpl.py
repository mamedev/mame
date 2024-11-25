#!/usr/bin/python3
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

MACHINE_CLONES_PROLOGUE = string.Template(
        '<h2 id="heading-clones">Clones</h2>\n' \
        '<table id="tbl-clones">\n' \
        '    <thead>\n' \
        '        <tr>\n' \
        '            <th>Short name</th>\n' \
        '            <th>Description</th>\n' \
        '            <th>Year</th>\n' \
        '            <th>Manufacturer</th>\n' \
        '        </tr>\n' \
        '    </thead>\n' \
        '    <tbody>\n')

MACHINE_CLONES_ROW = string.Template(
        '        <tr>\n' \
        '            <td><a href="${href}">${shortname}</a></td>\n' \
        '            <td><a href="${href}">${description}</a></td>\n' \
        '            <td>${year}</td>\n' \
        '            <td>${manufacturer}</td>\n' \
        '        </tr>\n')

MACHINE_SOFTWARELISTS_TABLE_PROLOGUE = string.Template(
        '<h2 id="heading-softwarelists">Software Lists</h2>\n' \
        '<table id="tbl-softwarelists">\n' \
        '    <thead>\n' \
        '        <tr>\n' \
        '            <th>Card</th>\n' \
        '            <th>Short name</th>\n' \
        '            <th>Description</th>\n' \
        '            <th>Status</th>\n' \
        '            <th class="numeric">Total</th>\n' \
        '            <th class="numeric">Supported</th>\n' \
        '            <th class="numeric">Partially supported</th>\n' \
        '            <th class="numeric">Unsupported</th>\n' \
        '        </tr>\n' \
        '    </thead>\n' \
        '    <tbody>\n')

MACHINE_SOFTWARELISTS_TABLE_ROW = string.Template(
        '        <tr id="row-softwarelists-${rowid}">\n' \
        '            <td></td>\n' \
        '            <td><a href="${href}">${shortname}</a></td>\n' \
        '            <td><a href="${href}">${description}</a></td>\n' \
        '            <td>${status}</td>\n' \
        '            <td style="text-align: right">${total}</td>\n' \
        '            <td style="text-align: right">${supported}</td>\n' \
        '            <td style="text-align: right">${partiallysupported}</td>\n' \
        '            <td style="text-align: right">${unsupported}</td>\n' \
        '        </tr>\n')

MACHINE_SOFTWARELISTS_TABLE_EPILOGUE = string.Template(
        '    </tbody>\n' \
        '</table>\n' \
        '<script>\n' \
        '    make_table_sortable(document.getElementById("tbl-softwarelists"));\n' \
        '    make_collapsible(document.getElementById("heading-softwarelists"), document.getElementById("tbl-softwarelists"));\n' \
        '    if (!document.getElementById("tbl-softwarelists").tBodies[0].rows.length)\n' \
        '    {\n' \
        '        document.getElementById("heading-softwarelists").style.display = "none";\n' \
        '        document.getElementById("tbl-softwarelists").style.display = "none";\n' \
        '    }\n' \
        '</script>\n')

MACHINE_OPTIONS_HEADING = string.Template(
        '<h2 id="heading-options">Options</h2>\n' \
        '<div id="div-options">\n' \
        '    <p>\n' \
        '        Format: <select id="select-options-format" onchange="update_cmd_preview()"><option value="cmd">Command line</option><option value="ini">INI file</option></select>\n' \
        '        <input type="checkbox" id="check-explicit-defaults" onchange="update_cmd_preview()"><label for="check-explicit-defaults">Explicit defaults</label>\n' \
        '    </p>\n' \
        '    <p id="para-cmd-preview"></p>\n')

MACHINE_OPTIONS_EPILOGUE = string.Template(
        '</div>\n' \
        '<script>make_collapsible(document.getElementById("heading-options"), document.getElementById("div-options"));</script>\n')

MACHINE_BIOS_PROLOGUE = string.Template(
        '    <h3>System BIOS</h3>' \
        '    <select id="select-system-bios" onchange="update_cmd_preview()">')

MACHINE_BIOS_OPTION = string.Template(
        '        <option value="${name}" data-isdefault="${isdefault}">${name} - ${description}</option>\n')

MACHINE_RAM_PROLOGUE = string.Template(
        '    <h3>RAM Size</h3>\n' \
        '    <select id="select-ram-option" onchange="update_cmd_preview()">\n')

MACHINE_RAM_OPTION = string.Template(
        '        <option value="${name}" data-isdefault="${isdefault}">${name} (${size})</option>\n')

MACHINE_SLOTS_PLACEHOLDER_PROLOGUE = string.Template(
        '    <h3>Slots</h3>\n' \
        '    <p id="para-slots-placeholder">Loading slot information&hellip;<p>\n' \
        '    <script>\n')

MACHINE_SLOTS_PLACEHOLDER_EPILOGUE = string.Template(
        '        populate_slots(${machine});\n'
        '    </script>\n')

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


SOFTWARE_PROLOGUE = string.Template(
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
        '<table class="sysinfo">\n' \
        '    <tr><th>Software list:</th><td><a href="${softwarelisthref}">${softwarelistdescription} (${softwarelist})</a></td></tr>\n' \
        '    <tr><th>Short name:</th><td>${shortname}</td></tr>\n' \
        '    <tr><th>Year:</th><td>${year}</td></tr>\n' \
        '    <tr><th>Publisher:</th><td>${publisher}</td></tr>\n');

SOFTWARE_CLONES_PROLOGUE = string.Template(
        '<h2 id="heading-clones">Clones</h2>\n' \
        '<table id="tbl-clones">\n' \
        '    <thead>\n' \
        '        <tr>\n' \
        '            <th>Short name</th>\n' \
        '            <th>Description</th>\n' \
        '            <th>Year</th>\n' \
        '            <th>Publisher</th>\n' \
        '            <th>Supported</th>\n' \
        '        </tr>\n' \
        '    </thead>\n' \
        '    <tbody>\n')

SOFTWARE_CLONES_ROW = string.Template(
        '        <tr>\n' \
        '            <td><a href="${href}">${shortname}</a></td>\n' \
        '            <td><a href="${href}">${description}</a></td>\n' \
        '            <td>${year}</td>\n' \
        '            <td>${publisher}</td>\n' \
        '            <td>${supported}</td>\n' \
        '        </tr>\n')

SOFTWARE_NOTES_PROLOGUE = string.Template(
        '<h2 id="heading-notes">Notes</h2>\n' \
        '<div id="div-notes">\n')

SOFTWARE_NOTES_EPILOGUE = string.Template(
        '</div>\n' \
        '<script>make_collapsible(document.getElementById("heading-notes"), document.getElementById("div-notes"))();</script>\n\n')

SOFTWARE_PARTS_PROLOGUE = string.Template(
        '<h2 id="heading-parts">Parts</h2>\n' \
        '<div id="div-parts">\n\n')

SOFTWARE_PARTS_EPILOGUE = string.Template(
        '</div>\n' \
        '<script>make_collapsible(document.getElementById("heading-parts"), document.getElementById("div-parts"));</script>\n\n')

SOFTWARE_PART_PROLOGUE = string.Template(
        '    <h3>${heading}</h3>\n' \
        '    <table class="sysinfo">\n' \
        '        <tr><th>Short name:</th><td>${shortname}</td></tr>\n' \
        '        <tr><th>Interface:</th><td>${interface}</td></tr>\n')


SOFTWARELIST_PROLOGUE = string.Template(
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
        '<table class="sysinfo">\n' \
        '    <tr>\n' \
        '        <th>Short name:</th>\n' \
        '        <td>${shortname}</td>\n' \
        '    </tr>\n' \
        '    <tr>\n' \
        '        <th>Total:</th>\n' \
        '        <td style="text-align: right">${total}</td>\n' \
        '    </tr>\n' \
        '    <tr>\n' \
        '        <th>Supported:</th>\n' \
        '        <td style="text-align: right">${supported}</td>\n' \
        '        <td style="text-align: right">(${supportedpc}%)</td>\n' \
        '    </tr>\n' \
        '    <tr>\n' \
        '        <th>Partially supported:</th>\n' \
        '        <td style="text-align: right">${partiallysupported}</td>\n' \
        '        <td style="text-align: right">(${partiallysupportedpc}%)</td>\n' \
        '    </tr>\n' \
        '    <tr>\n' \
        '        <th>Unsupported:</th>\n' \
        '        <td style="text-align: right">${unsupported}</td>\n' \
        '        <td style="text-align: right">(${unsupportedpc}%)</td>\n' \
        '    </tr>\n' \
        '</table>\n')

SOFTWARELIST_NOTES_PROLOGUE = string.Template(
        '<h2 id="heading-notes">Notes</h2>\n' \
        '<div id="div-notes">\n')

SOFTWARELIST_NOTES_EPILOGUE = string.Template(
        '</div>\n' \
        '<script>make_collapsible(document.getElementById("heading-notes"), document.getElementById("div-notes"))();</script>\n\n')

SOFTWARELIST_MACHINE_TABLE_HEADER = string.Template(
        '<h2 id="heading-machines">Machines</h2>\n' \
        '<table id="tbl-machines">\n' \
        '    <thead>\n' \
        '        <tr>\n' \
        '            <th>Short name</th>\n' \
        '            <th>Description</th>\n' \
        '            <th>Year</th>\n' \
        '            <th>Manufacturer</th>\n' \
        '            <th>Status</th>\n' \
        '        </tr>\n' \
        '    </thead>\n' \
        '    <tbody>\n')

SOFTWARELIST_MACHINE_TABLE_ROW = string.Template(
        '        <tr>\n' \
        '            <td><a href="${machinehref}">${shortname}</a></td>\n' \
        '            <td><a href="${machinehref}">${description}</a></td>\n' \
        '            <td>${year}</td>\n' \
        '            <td>${manufacturer}</td>\n' \
        '            <td>${status}</td>\n' \
        '        </tr>\n')

SOFTWARELIST_SOFTWARE_TABLE_HEADER = string.Template(
        '<h2 id="heading-software">Software</h2>\n' \
        '<table id="tbl-software">\n' \
        '    <thead>\n' \
        '        <tr>\n' \
        '            <th>Short name</th>\n' \
        '            <th>Description</th>\n' \
        '            <th>Year</th>\n' \
        '            <th>Publisher</th>\n' \
        '            <th>Supported</th>\n' \
        '            <th class="numeric">Parts</th>\n' \
        '            <th class="numeric">Bad dumps</th>\n' \
        '            <th>Parent</th>\n' \
        '        </tr>\n' \
        '    </thead>\n' \
        '    <tbody>\n')

SOFTWARELIST_SOFTWARE_ROW = string.Template(
        '        <tr>\n' \
        '            <td><a href="${softwarehref}">${shortname}</a></td>\n' \
        '            <td><a href="${softwarehref}">${description}</a></td>\n' \
        '            <td>${year}</td>\n' \
        '            <td>${publisher}</td>\n' \
        '            <td>${supported}</td>\n' \
        '            <td style="text-align: right">${parts}</td>\n' \
        '            <td style="text-align: right">${baddumps}</td>\n' \
        '            <td>${parent}</td>\n' \
        '        </tr>\n')


SOFTWARELIST_LIST_PROLOGUE = string.Template(
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
        '<table id="tbl-softwarelists">\n' \
        '    <thead>\n' \
        '        <tr>\n' \
        '            <th>Short name</th>\n' \
        '            <th>Description</th>\n' \
        '            <th class="numeric">Total</th>\n' \
        '            <th class="numeric">Supported</th>\n' \
        '            <th class="numeric">Partially supported</th>\n' \
        '            <th class="numeric">Unsupported</th>\n' \
        '        </tr>\n' \
        '    </thead>\n' \
        '    <tbody>\n')

SOFTWARELIST_LIST_ROW = string.Template(
        '        <tr>\n' \
        '            <td><a href="${href}">${shortname}</a></td>\n' \
        '            <td><a href="${href}">${description}</a></td>\n' \
        '            <td style="text-align: right">${total}</td>\n' \
        '            <td style="text-align: right">${supported}</td>\n' \
        '            <td style="text-align: right">${partiallysupported}</td>\n' \
        '            <td style="text-align: right">${unsupported}</td>\n' \
        '        </tr>\n')


ROMIDENT_PAGE = string.Template(
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
        '    <script type="text/javascript" src="${assets}/digest.js"></script>\n' \
        '    <script type="text/javascript" src="${assets}/romident.js"></script>\n' \
        '    <title>Identify ROM/Disk Dumps</title>\n' \
        '</head>\n' \
        '<body>\n' \
        '<h1>Identify ROM/Disk Dumps</h1>\n' \
        '<p>No files are uploaded.  Files are examined locally and checksums/digests are sent to the server.  File checksums and digests may be logged on the server.</p>\n' \
        '<div id="div-dropzone" class="dropzone" ondragover="div_dropzone_dragover(event)" ondrop="div_dropzone_drop(event)">\n' \
        '<p><button type="button" onclick="document.getElementById(&quot;input-dumps&quot;).click()">Select ROM/disk dumps</button></p>\n' \
        '<p>Drag and drop ROM/disk dump files here to identify them.</p>\n' \
        '</div>\n' \
        '<input id="input-dumps" type="file" multiple onchange="add_dump_files(this.files)" style="display: none">\n' \
        '<div id="div-progress"></div>\n' \
        '<div id="div-issues"></div>\n' \
        '<div id="div-unmatched"></div>\n' \
        '<div id="div-machines"></div>\n' \
        '<div id="div-software"></div>\n' \
        '</body>\n' \
        '</html>\n')
