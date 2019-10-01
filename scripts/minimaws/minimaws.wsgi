#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb
##
## Simple script for deploying minimaws with mod_wsgi.

import os.path
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import lib.wsgiserve

application = lib.wsgiserve.MiniMawsApp(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'minimaws.sqlite3'))
