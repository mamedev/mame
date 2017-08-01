#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import sqlite3


class SchemaQueries(object):
    INDEX_MACHINE_ISDEVICE_SHORTNAME = 'CREATE INDEX machine_isdevice_shortname ON machine (isdevice ASC, shortname ASC)'
    INDEX_MACHINE_ISDEVICE_DESCRIPTION = 'CREATE INDEX machine_isdevice_description ON machine (isdevice ASC, description ASC)'
    INDEX_MACHINE_RUNNABLE_SHORTNAME = 'CREATE INDEX machine_runnable_shortname ON machine (runnable ASC, shortname ASC)'
    INDEX_MACHINE_RUNNABLE_DESCRIPTION = 'CREATE INDEX machine_runnable_description ON machine (runnable ASC, description ASC)'

    INDEX_SYSTEM_YEAR = 'CREATE INDEX system_year ON system (year ASC)'
    INDEX_SYSTEM_MANUFACTURER = 'CREATE INDEX system_manufacturer ON system (manufacturer ASC)'

    INDEX_ROMOF_PARENT = 'CREATE INDEX romof_parent ON romof (parent ASC)'

    INDEX_CLONEOF_PARENT = 'CREATE INDEX cloneof_parent ON cloneof (parent ASC)'

    INDEX_DEVICEREFERENCE_DEVICE = 'CREATE INDEX devicereference_device ON devicereference (device ASC)'

    INDEX_DIPSWITCH_MACHINE_ISCONFIG = 'CREATE INDEX dipswitch_machine_isconfig ON dipswitch (machine ASC, isconfig ASC)'

    DROP_MACHINE_ISDEVICE_SHORTNAME = 'DROP INDEX IF EXISTS machine_isdevice_shortname'
    DROP_MACHINE_ISDEVICE_DESCRIPTION = 'DROP INDEX IF EXISTS machine_isdevice_description'
    DROP_MACHINE_RUNNABLE_SHORTNAME = 'DROP INDEX IF EXISTS machine_runnable_shortname'
    DROP_MACHINE_RUNNABLE_DESCRIPTION = 'DROP INDEX IF EXISTS machine_runnable_description'

    DROP_SYSTEM_YEAR = 'DROP INDEX IF EXISTS system_year'
    DROP_SYSTEM_MANUFACTURER = 'DROP INDEX IF EXISTS system_manufacturer'

    DROP_ROMOF_PARENT = 'DROP INDEX IF EXISTS romof_parent'

    DROP_CLONEOF_PARENT = 'DROP INDEX IF EXISTS cloneof_parent'

    DROP_DEVICEREFERENCE_DEVICE = 'DROP INDEX IF EXISTS devicereference_device'

    DROP_DIPSWITCH_MACHINE_ISCONFIG = 'DROP INDEX IF EXISTS dipswitch_machine_isconfig'


class UpdateQueries(object):
    ADD_FEATURETYPE = 'INSERT OR IGNORE INTO featuretype (name) VALUES (?)'
    ADD_SOURCEFILE = 'INSERT OR IGNORE INTO sourcefile (filename) VALUES (?)'
    ADD_MACHINE = 'INSERT INTO machine (shortname, description, sourcefile, isdevice, runnable) SELECT ?, ?, id, ?, ? FROM sourcefile WHERE filename = ?'
    ADD_SYSTEM = 'INSERT INTO system (id, year, manufacturer) VALUES (?, ?, ?)'
    ADD_CLONEOF = 'INSERT INTO cloneof (id, parent) VALUES (?, ?)'
    ADD_ROMOF = 'INSERT INTO romof (id, parent) VALUES (?, ?)'
    ADD_DEVICEREFERENCE = 'INSERT OR IGNORE INTO devicereference (machine, device) VALUES (?, ?)'
    ADD_DIPSWITCH = 'INSERT INTO dipswitch (machine, isconfig, name, tag, mask) VALUES (?, ?, ?, ?, ?)'
    ADD_DIPLOCATION = 'INSERT INTO diplocation (dipswitch, bit, name, num, inverted) VALUES (?, ?, ?, ?, ?)'
    ADD_DIPVALUE = 'INSERT INTO dipvalue (dipswitch, name, value, isdefault) VALUES (?, ?, ?, ?)'
    ADD_FEATURE = 'INSERT INTO feature (machine, featuretype, status, overall) SELECT ?, id, ?, ? FROM featuretype WHERE name = ?'


class QueryCursor(object):
    def __init__(self, dbconn, **kwargs):
        super(QueryCursor, self).__init__(**kwargs)
        self.dbcurs = dbconn.cursor()

    def close(self):
        self.dbcurs.close()

    def is_glob(self, *patterns):
        for pattern in patterns:
            if any(ch in pattern for ch in '?*['):
                return True
        return False

    def count_systems(self, pattern):
        if pattern is not None:
            return self.dbcurs.execute(
                    'SELECT COUNT(*) ' \
                    'FROM machine WHERE isdevice = 0 AND shortname GLOB ? ',
                    (pattern, ))
        else:
            return self.dbcurs.execute(
                    'SELECT COUNT(*) ' \
                    'FROM machine WHERE isdevice = 0 ')

    def listfull(self, pattern):
        if pattern is not None:
            return self.dbcurs.execute(
                    'SELECT shortname, description ' \
                    'FROM machine WHERE isdevice = 0 AND shortname GLOB ? ' \
                    'ORDER BY shortname ASC',
                    (pattern, ))
        else:
            return self.dbcurs.execute(
                    'SELECT shortname, description ' \
                    'FROM machine WHERE isdevice = 0 ' \
                    'ORDER BY shortname ASC')

    def listsource(self, pattern):
        if pattern is not None:
            return self.dbcurs.execute(
                    'SELECT machine.shortname, sourcefile.filename ' \
                    'FROM machine JOIN sourcefile ON machine.sourcefile = sourcefile.id ' \
                    'WHERE machine.isdevice = 0 AND machine.shortname GLOB ? ' \
                    'ORDER BY machine.shortname ASC',
                    (pattern, ))
        else:
            return self.dbcurs.execute(
                    'SELECT machine.shortname, sourcefile.filename ' \
                    'FROM machine JOIN sourcefile ON machine.sourcefile = sourcefile.id ' \
                    'WHERE machine.isdevice = 0 ORDER BY machine.shortname ASC')

    def listclones(self, pattern):
        if pattern is not None:
            return self.dbcurs.execute(
                    'SELECT machine.shortname, cloneof.parent ' \
                    'FROM machine JOIN cloneof ON machine.id = cloneof.id ' \
                    'WHERE machine.shortname GLOB ? OR cloneof.parent GLOB ? ' \
                    'ORDER BY machine.shortname ASC',
                    (pattern, pattern))
        else:
            return self.dbcurs.execute(
                    'SELECT machine.shortname, cloneof.parent ' \
                    'FROM machine JOIN cloneof ON machine.id = cloneof.id ' \
                    'ORDER BY machine.shortname ASC')

    def listbrothers(self, pattern):
        if pattern is not None:
            return self.dbcurs.execute(
                    'SELECT sourcefile.filename, machine.shortname, cloneof.parent ' \
                    'FROM machine JOIN sourcefile ON machine.sourcefile = sourcefile.id LEFT JOIN cloneof ON machine.id = cloneof.id ' \
                    'WHERE machine.isdevice = 0 AND sourcefile.id IN (SELECT sourcefile FROM machine WHERE shortname GLOB ?)' \
                    'ORDER BY machine.shortname ASC',
                    (pattern, ))
        else:
            return self.dbcurs.execute(
                    'SELECT sourcefile.filename, machine.shortname, cloneof.parent ' \
                    'FROM machine JOIN sourcefile ON machine.sourcefile = sourcefile.id LEFT JOIN cloneof ON machine.id = cloneof.id ' \
                    'WHERE machine.isdevice = 0 ' \
                    'ORDER BY machine.shortname ASC')

    def listaffected(self, *patterns):
        if 1 == len(patterns):
            return self.dbcurs.execute(
                    'SELECT shortname, description ' \
                    'FROM machine ' \
                    'WHERE id IN (SELECT machine FROM devicereference WHERE device IN (SELECT shortname FROM machine WHERE sourcefile IN (SELECT id FROM sourcefile WHERE filename GLOB ?))) AND runnable = 1 ' \
                    'ORDER BY shortname ASC',
                    patterns)
        elif self.is_glob(*patterns):
            return self.dbcurs.execute(
                    'SELECT shortname, description ' \
                    'FROM machine ' \
                    'WHERE id IN (SELECT machine FROM devicereference WHERE device IN (SELECT shortname FROM machine WHERE sourcefile IN (SELECT id FROM sourcefile WHERE filename GLOB ?' + (' OR filename GLOB ?' * (len(patterns) - 1)) + '))) AND runnable = 1 ' \
                    'ORDER BY shortname ASC',
                    patterns)
        else:
            return self.dbcurs.execute(
                    'SELECT shortname, description ' \
                    'FROM machine ' \
                    'WHERE id IN (SELECT machine FROM devicereference WHERE device IN (SELECT shortname FROM machine WHERE sourcefile IN (SELECT id FROM sourcefile WHERE filename IN (?' + (', ?' * (len(patterns) - 1)) + ')))) AND runnable = 1 ' \
                    'ORDER BY shortname ASC',
                    patterns)

    def get_machine_info(self, machine):
        return self.dbcurs.execute(
                'SELECT machine.id AS id, machine.description AS description, machine.isdevice AS isdevice, machine.runnable AS runnable, sourcefile.filename AS sourcefile, system.year AS year, system.manufacturer AS manufacturer, cloneof.parent AS cloneof, romof.parent AS romof ' \
                'FROM machine JOIN sourcefile ON machine.sourcefile = sourcefile.id LEFT JOIN system ON machine.id = system.id LEFT JOIN cloneof ON system.id = cloneof.id LEFT JOIN romof ON system.id = romof.id ' \
                'WHERE machine.shortname = ?',
                (machine, ))

    def get_devices_referenced(self, machine):
        return self.dbcurs.execute(
                'SELECT devicereference.device AS shortname, machine.description AS description, sourcefile.filename AS sourcefile ' \
                'FROM devicereference LEFT JOIN machine ON devicereference.device = machine.shortname LEFT JOIN sourcefile ON machine.sourcefile = sourcefile.id ' \
                'WHERE devicereference.machine = ?',
                (machine, ))

    def get_device_references(self, shortname):
        return self.dbcurs.execute(
                'SELECT machine.shortname AS shortname, machine.description AS description, sourcefile.filename AS sourcefile ' \
                'FROM machine JOIN sourcefile ON machine.sourcefile = sourcefile.id ' \
                'WHERE machine.id IN (SELECT machine FROM devicereference WHERE device = ?)',
                (shortname, ))

    def get_sourcefile_id(self, filename):
        return (self.dbcurs.execute('SELECT id FROM sourcefile WHERE filename = ?', (filename, )).fetchone() or (None, ))[0]

    def get_sourcefile_machines(self, id):
        return self.dbcurs.execute(
                'SELECT machine.shortname AS shortname, machine.description AS description, machine.isdevice AS isdevice, machine.runnable AS runnable, sourcefile.filename AS sourcefile, system.year AS year, system.manufacturer AS manufacturer, cloneof.parent AS cloneof, romof.parent AS romof ' \
                'FROM machine JOIN sourcefile ON machine.sourcefile = sourcefile.id LEFT JOIN system ON machine.id = system.id LEFT JOIN cloneof ON system.id = cloneof.id LEFT JOIN romof ON system.id = romof.id ' \
                'WHERE machine.sourcefile = ?',
                (id, ))

    def get_sourcefiles(self, pattern):
        if pattern is not None:
            return self.dbcurs.execute(
                    'SELECT sourcefile.filename AS filename, COUNT(machine.id) AS machines ' \
                    'FROM sourcefile LEFT JOIN machine ON sourcefile.id = machine.sourcefile ' \
                    'WHERE sourcefile.filename GLOB ?' \
                    'GROUP BY sourcefile.id ',
                    (pattern, ))
        else:
            return self.dbcurs.execute(
                    'SELECT sourcefile.filename AS filename, COUNT(machine.id) AS machines ' \
                    'FROM sourcefile LEFT JOIN machine ON sourcefile.id = machine.sourcefile ' \
                    'GROUP BY sourcefile.id')

    def count_sourcefiles(self, pattern):
        if pattern is not None:
            return self.dbcurs.execute('SELECT COUNT(*) FROM sourcefile WHERE filename GLOB ?', (pattern, )).fetchone()[0]
        else:
            return self.dbcurs.execute('SELECT COUNT(*) FROM sourcefile').fetchone()[0]


class UpdateCursor(object):
    def __init__(self, dbconn, **kwargs):
        super(UpdateCursor, self).__init__(**kwargs)
        self.dbcurs = dbconn.cursor()

    def close(self):
        self.dbcurs.close()

    def add_featuretype(self, name):
        self.dbcurs.execute(UpdateQueries.ADD_FEATURETYPE, (name, ))

    def add_sourcefile(self, filename):
        self.dbcurs.execute(UpdateQueries.ADD_SOURCEFILE, (filename, ))

    def add_machine(self, shortname, description, sourcefile, isdevice, runnable):
        self.dbcurs.execute(UpdateQueries.ADD_MACHINE, (shortname, description, isdevice, runnable, sourcefile))
        return self.dbcurs.lastrowid

    def add_system(self, machine, year, manufacturer):
        self.dbcurs.execute(UpdateQueries.ADD_SYSTEM, (machine, year, manufacturer))
        return self.dbcurs.lastrowid

    def add_cloneof(self, machine, parent):
        self.dbcurs.execute(UpdateQueries.ADD_CLONEOF, (machine, parent))
        return self.dbcurs.lastrowid

    def add_romof(self, machine, parent):
        self.dbcurs.execute(UpdateQueries.ADD_ROMOF, (machine, parent))
        return self.dbcurs.lastrowid

    def add_devicereference(self, machine, device):
        self.dbcurs.execute(UpdateQueries.ADD_DEVICEREFERENCE, (machine, device))

    def add_dipswitch(self, machine, isconfig, name, tag, mask):
        self.dbcurs.execute(UpdateQueries.ADD_DIPSWITCH, (machine, isconfig, name, tag, mask))
        return self.dbcurs.lastrowid

    def add_diplocation(self, dipswitch, bit, name, num, inverted):
        self.dbcurs.execute(UpdateQueries.ADD_DIPLOCATION, (dipswitch, bit, name, num, inverted))
        return self.dbcurs.lastrowid

    def add_dipvalue(self, dipswitch, name, value, isdefault):
        self.dbcurs.execute(UpdateQueries.ADD_DIPVALUE, (dipswitch, name, value, isdefault))
        return self.dbcurs.lastrowid

    def add_feature(self, machine, featuretype, status, overall):
        self.dbcurs.execute(UpdateQueries.ADD_FEATURE, (machine, status, overall, featuretype))
        return self.dbcurs.lastrowid


class QueryConnection(object):
    def __init__(self, database, **kwargs):
        # TODO: detect python versions that allow URL-based read-only connection
        super(QueryConnection, self).__init__(**kwargs)
        self.dbconn = sqlite3.connect(database)
        self.dbconn.row_factory = sqlite3.Row
        self.dbconn.execute('PRAGMA foreign_keys = ON')

    def close(self):
        self.dbconn.close()

    def cursor(self):
        return QueryCursor(self.dbconn)


class UpdateConnection(object):
    def __init__(self, database, **kwargs):
        super(UpdateConnection, self).__init__(**kwargs)
        self.dbconn = sqlite3.connect(database)
        self.dbconn.execute('PRAGMA foreign_keys = ON')

    def commit(self):
        self.dbconn.commit()

    def rollback(self):
        self.dbconn.rollback()

    def close(self):
        self.dbconn.close()

    def cursor(self):
        return UpdateCursor(self.dbconn)

    def drop_indexes(self):
        self.dbconn.execute(SchemaQueries.DROP_MACHINE_ISDEVICE_SHORTNAME)
        self.dbconn.execute(SchemaQueries.DROP_MACHINE_ISDEVICE_DESCRIPTION)
        self.dbconn.execute(SchemaQueries.DROP_MACHINE_RUNNABLE_SHORTNAME)
        self.dbconn.execute(SchemaQueries.DROP_MACHINE_RUNNABLE_DESCRIPTION)
        self.dbconn.execute(SchemaQueries.DROP_SYSTEM_YEAR)
        self.dbconn.execute(SchemaQueries.DROP_SYSTEM_MANUFACTURER)
        self.dbconn.execute(SchemaQueries.DROP_ROMOF_PARENT)
        self.dbconn.execute(SchemaQueries.DROP_CLONEOF_PARENT)
        self.dbconn.execute(SchemaQueries.DROP_DEVICEREFERENCE_DEVICE)
        self.dbconn.execute(SchemaQueries.DROP_DIPSWITCH_MACHINE_ISCONFIG)
        self.dbconn.commit()

    def create_indexes(self):
        self.dbconn.execute(SchemaQueries.INDEX_MACHINE_ISDEVICE_SHORTNAME)
        self.dbconn.execute(SchemaQueries.INDEX_MACHINE_ISDEVICE_DESCRIPTION)
        self.dbconn.execute(SchemaQueries.INDEX_MACHINE_RUNNABLE_SHORTNAME)
        self.dbconn.execute(SchemaQueries.INDEX_MACHINE_RUNNABLE_DESCRIPTION)
        self.dbconn.execute(SchemaQueries.INDEX_SYSTEM_YEAR)
        self.dbconn.execute(SchemaQueries.INDEX_SYSTEM_MANUFACTURER)
        self.dbconn.execute(SchemaQueries.INDEX_ROMOF_PARENT)
        self.dbconn.execute(SchemaQueries.INDEX_CLONEOF_PARENT)
        self.dbconn.execute(SchemaQueries.INDEX_DEVICEREFERENCE_DEVICE)
        self.dbconn.execute(SchemaQueries.INDEX_DIPSWITCH_MACHINE_ISCONFIG)
        self.dbconn.commit()
