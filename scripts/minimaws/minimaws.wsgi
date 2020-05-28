#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb
##
## Simple wrapper for deploying minimaws with mod_wsgi.

import os.path
import sys

scriptdir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, scriptdir)

import lib.wsgiserve

application = lib.wsgiserve.MiniMawsApp(os.path.join(scriptdir, 'minimaws.sqlite3'))
