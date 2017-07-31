#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import string


MACHINE_PROLOGUE = string.Template(
        '<!DOCTYPE html>\n' \
        '<html>\n' \
        '<head>\n' \
        '    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">\n' \
        '    <title>${description} (${shortname})</title>\n' \
        '</head>\n' \
        '<body>\n' \
        '<h1>${description}</h1>\n' \
        '<table>\n' \
        '    <tr><th style="text-align: right">Short name:</th><td>${shortname}</td></tr>\n' \
        '    <tr><th style="text-align: right">Is device:</th><td>${isdevice}</td></tr>\n' \
        '    <tr><th style="text-align: right">Runnable:</th><td>${runnable}</td></tr>\n' \
        '    <tr><th style="text-align: right">Source file:</th><td>${sourcefile}</td></tr>\n')
