#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import sqlite3
import sys

if sys.version_info >= (3, 4):
    import urllib.request


class SchemaQueries(object):
    CREATE_FEATURETYPE = \
            'CREATE TABLE featuretype (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    name            TEXT    NOT NULL,\n' \
            '    UNIQUE (name ASC))'
    CREATE_SOURCEFILE = \
            'CREATE TABLE sourcefile (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    filename        TEXT    NOT NULL,\n' \
            '    UNIQUE (filename ASC))'
    CREATE_MACHINE = \
            'CREATE TABLE machine (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    shortname       TEXT    NOT NULL,\n' \
            '    description     TEXT    NOT NULL,\n' \
            '    sourcefile      INTEGER NOT NULL,\n' \
            '    isdevice        INTEGER NOT NULL,\n' \
            '    runnable        INTEGER NOT NULL,\n' \
            '    UNIQUE (shortname ASC),\n' \
            '    UNIQUE (description ASC),\n' \
            '    FOREIGN KEY (sourcefile) REFERENCES sourcefile (id))'
    CREATE_SYSTEM = \
            'CREATE TABLE system (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    year            TEXT    NOT NULL,\n' \
            '    manufacturer    TEXT    NOT NULL,\n' \
            '    FOREIGN KEY (id) REFERENCES machine (id))'
    CREATE_CLONEOF = \
            'CREATE TABLE cloneof (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    parent          TEXT    NOT NULL,\n' \
            '    FOREIGN KEY (id) REFERENCES machine (id))'
    CREATE_ROMOF = \
            'CREATE TABLE romof (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    parent          TEXT    NOT NULL,\n' \
            '    FOREIGN KEY (id) REFERENCES machine (id))'
    CREATE_BIOSSET = \
            'CREATE TABLE biosset (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    machine         INTEGER NOT NULL,\n' \
            '    name            TEXT    NOT NULL,\n' \
            '    description     TEXT    NOT NULL,\n' \
            '    UNIQUE (machine ASC, name ASC),\n' \
            '    FOREIGN KEY (machine) REFERENCES machine (id))'
    CREATE_BIOSSETDEFAULT = \
            'CREATE TABLE biossetdefault (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    FOREIGN KEY (id) REFERENCES biosset (id))'
    CREATE_DEVICEREFERENCE = \
            'CREATE TABLE devicereference (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    machine         INTEGER NOT NULL,\n' \
            '    device          INTEGER NOT NULL,\n' \
            '    UNIQUE (machine ASC, device ASC),\n' \
            '    FOREIGN KEY (machine) REFERENCES machine (id),\n' \
            '    FOREIGN KEY (device) REFERENCES machine (id))'
    CREATE_DIPSWITCH = \
            'CREATE TABLE dipswitch (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    machine         INTEGER NOT NULL,\n' \
            '    isconfig        INTEGER NOT NULL,\n' \
            '    name            TEXT    NOT NULL,\n' \
            '    tag             TEXT    NOT NULL,\n' \
            '    mask            INTEGER NOT NULL,\n' \
            '    --UNIQUE (machine ASC, tag ASC, mask ASC), not necessarily true, need to expose port conditions\n' \
            '    FOREIGN KEY (machine) REFERENCES machine (id))'
    CREATE_DIPLOCATION = \
            'CREATE TABLE diplocation (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    dipswitch       INTEGER NOT NULL,\n' \
            '    bit             INTEGER NOT NULL,\n' \
            '    name            TEXT    NOT NULL,\n' \
            '    num             INTEGER NOT NULL,\n' \
            '    inverted        INTEGER NOT NULL,\n' \
            '    UNIQUE (dipswitch ASC, bit ASC),\n' \
            '    FOREIGN KEY (dipswitch) REFERENCES dipswitch (id))'
    CREATE_DIPVALUE = \
            'CREATE TABLE dipvalue (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    dipswitch       INTEGER NOT NULL,\n' \
            '    name            TEXT    NOT NULL,\n' \
            '    value           INTEGER NOT NULL,\n' \
            '    isdefault       INTEGER NOT NULL,\n' \
            '    FOREIGN KEY (dipswitch) REFERENCES dipswitch (id))'
    CREATE_FEATURE = \
            'CREATE TABLE feature (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    machine         INTEGER NOT NULL,\n' \
            '    featuretype     INTEGER NOT NULL,\n' \
            '    status          INTEGER NOT NULL,\n' \
            '    overall         INTEGER NOT NULL,\n' \
            '    UNIQUE (machine ASC, featuretype ASC),\n' \
            '    FOREIGN KEY (machine) REFERENCES machine (id),\n' \
            '    FOREIGN KEY (featuretype) REFERENCES featuretype (id))'
    CREATE_SLOT = \
            'CREATE TABLE slot (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    machine         INTEGER NOT NULL,\n' \
            '    name            TEXT    NOT NULL,\n' \
            '    UNIQUE (machine ASC, name ASC),\n' \
            '    FOREIGN KEY (machine) REFERENCES machine (id))'
    CREATE_SLOTOPTION = \
            'CREATE TABLE slotoption (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    slot            INTEGER NOT NULL,\n' \
            '    device          INTEGER NOT NULL,\n' \
            '    name            TEXT    NOT NULL,\n' \
            '    UNIQUE (slot ASC, name ASC),\n' \
            '    FOREIGN KEY (slot) REFERENCES slot (id),\n' \
            '    FOREIGN KEY (device) REFERENCES machine (id))'
    CREATE_SLOTDEFAULT = \
            'CREATE TABLE slotdefault (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    slotoption      INTEGER NOT NULL,\n' \
            '    FOREIGN KEY (id) REFERENCES slot (id),\n' \
            '    FOREIGN KEY (slotoption) REFERENCES slotoption (id))'
    CREATE_RAMOPTION = \
            'CREATE TABLE ramoption (\n' \
            '    machine         INTEGER NOT NULL,\n' \
            '    size            INTEGER NOT NULL,\n' \
            '    name            TEXT    NOT NULL,\n' \
            '    PRIMARY KEY (machine ASC, size ASC),\n' \
            '    FOREIGN KEY (machine) REFERENCES machine (id))'
    CREATE_RAMDEFAULT = \
            'CREATE TABLE ramdefault (\n' \
            '    machine         INTEGER PRIMARY KEY,\n' \
            '    size            INTEGER NOT NULL,\n' \
            '    FOREIGN KEY (machine) REFERENCES machine (id),\n' \
            '    FOREIGN KEY (machine, size) REFERENCES ramoption (machine, size))'
    CREATE_ROM = \
            'CREATE TABLE rom (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    crc             INTEGER NOT NULL,\n' \
            '    sha1            TEXT    NOT NULL,\n' \
            '    UNIQUE (crc ASC, sha1 ASC))'
    CREATE_ROMDUMP = \
            'CREATE TABLE romdump (\n' \
            '    machine         INTEGER NOT NULL,\n' \
            '    rom             INTEGER NOT NULL,\n' \
            '    name            TEXT NOT NULL,\n' \
            '    bad             INTEGER NOT NULL,\n' \
            '    FOREIGN KEY (machine) REFERENCES machine (id),\n' \
            '    FOREIGN KEY (rom) REFERENCES rom (id),\n' \
            '    UNIQUE (machine, rom, name))'
    CREATE_DISK = \
            'CREATE TABLE disk (\n' \
            '    id              INTEGER PRIMARY KEY,\n' \
            '    sha1            TEXT    NOT NULL,\n' \
            '    UNIQUE (sha1 ASC))'
    CREATE_DISKDUMP = \
            'CREATE TABLE diskdump (\n' \
            '    machine         INTEGER NOT NULL,\n' \
            '    disk            INTEGER NOT NULL,\n' \
            '    name            TEXT NOT NULL,\n' \
            '    bad             INTEGER NOT NULL,\n' \
            '    FOREIGN KEY (machine) REFERENCES machine (id),\n' \
            '    FOREIGN KEY (disk) REFERENCES disk (id),\n' \
            '    UNIQUE (machine, disk, name))'

    CREATE_TEMPORARY_DEVICEREFERENCE = 'CREATE TEMPORARY TABLE temp_devicereference (id INTEGER PRIMARY KEY, machine INTEGER NOT NULL, device TEXT NOT NULL, UNIQUE (machine, device))'
    CREATE_TEMPORARY_SLOTOPTION = 'CREATE TEMPORARY TABLE temp_slotoption (id INTEGER PRIMARY KEY, slot INTEGER NOT NULL, device TEXT NOT NULL, name TEXT NOT NULL)'
    CREATE_TEMPORARY_SLOTDEFAULT = 'CREATE TEMPORARY TABLE temp_slotdefault (id INTEGER PRIMARY KEY, slotoption INTEGER NOT NULL)'

    DROP_TEMPORARY_DEVICEREFERENCE = 'DROP TABLE IF EXISTS temp_devicereference'
    DROP_TEMPORARY_SLOTOPTION = 'DROP TABLE IF EXISTS temp_slotoption'
    DROP_TEMPORARY_SLOTDEFAULT = 'DROP TABLE IF EXISTS temp_slotdefault'

    INDEX_MACHINE_ISDEVICE_SHORTNAME = 'CREATE INDEX machine_isdevice_shortname ON machine (isdevice ASC, shortname ASC)'
    INDEX_MACHINE_ISDEVICE_DESCRIPTION = 'CREATE INDEX machine_isdevice_description ON machine (isdevice ASC, description ASC)'
    INDEX_MACHINE_RUNNABLE_SHORTNAME = 'CREATE INDEX machine_runnable_shortname ON machine (runnable ASC, shortname ASC)'
    INDEX_MACHINE_RUNNABLE_DESCRIPTION = 'CREATE INDEX machine_runnable_description ON machine (runnable ASC, description ASC)'

    INDEX_SYSTEM_YEAR = 'CREATE INDEX system_year ON system (year ASC)'
    INDEX_SYSTEM_MANUFACTURER = 'CREATE INDEX system_manufacturer ON system (manufacturer ASC)'

    INDEX_ROMOF_PARENT = 'CREATE INDEX romof_parent ON romof (parent ASC)'

    INDEX_CLONEOF_PARENT = 'CREATE INDEX cloneof_parent ON cloneof (parent ASC)'

    INDEX_DIPSWITCH_MACHINE_ISCONFIG = 'CREATE INDEX dipswitch_machine_isconfig ON dipswitch (machine ASC, isconfig ASC)'

    INDEX_ROMDUMP_ROM = 'CREATE INDEX romdump_rom ON romdump (rom ASC)'

    INDEX_DISKDUMP_DISK = 'CREATE INDEX diskdump_disk ON diskdump (disk ASC)'

    DROP_MACHINE_ISDEVICE_SHORTNAME = 'DROP INDEX IF EXISTS machine_isdevice_shortname'
    DROP_MACHINE_ISDEVICE_DESCRIPTION = 'DROP INDEX IF EXISTS machine_isdevice_description'
    DROP_MACHINE_RUNNABLE_SHORTNAME = 'DROP INDEX IF EXISTS machine_runnable_shortname'
    DROP_MACHINE_RUNNABLE_DESCRIPTION = 'DROP INDEX IF EXISTS machine_runnable_description'

    DROP_SYSTEM_YEAR = 'DROP INDEX IF EXISTS system_year'
    DROP_SYSTEM_MANUFACTURER = 'DROP INDEX IF EXISTS system_manufacturer'

    DROP_ROMOF_PARENT = 'DROP INDEX IF EXISTS romof_parent'

    DROP_CLONEOF_PARENT = 'DROP INDEX IF EXISTS cloneof_parent'

    DROP_DIPSWITCH_MACHINE_ISCONFIG = 'DROP INDEX IF EXISTS dipswitch_machine_isconfig'

    DROP_ROMDUMP_ROM = 'DROP INDEX IF EXISTS romdump_rom'

    DROP_DISKDUMP_DISK = 'DROP INDEX IF EXISTS diskdump_disk'

    CREATE_TABLES = (
            CREATE_FEATURETYPE,
            CREATE_SOURCEFILE,
            CREATE_MACHINE,
            CREATE_SYSTEM,
            CREATE_CLONEOF,
            CREATE_ROMOF,
            CREATE_BIOSSET,
            CREATE_BIOSSETDEFAULT,
            CREATE_DEVICEREFERENCE,
            CREATE_DIPSWITCH,
            CREATE_DIPLOCATION,
            CREATE_DIPVALUE,
            CREATE_FEATURE,
            CREATE_SLOT,
            CREATE_SLOTOPTION,
            CREATE_SLOTDEFAULT,
            CREATE_RAMOPTION,
            CREATE_RAMDEFAULT,
            CREATE_ROM,
            CREATE_ROMDUMP,
            CREATE_DISK,
            CREATE_DISKDUMP)

    CREATE_TEMPORARY_TABLES = (
            CREATE_TEMPORARY_DEVICEREFERENCE,
            CREATE_TEMPORARY_SLOTOPTION,
            CREATE_TEMPORARY_SLOTDEFAULT)

    CREATE_INDEXES = (
            INDEX_MACHINE_ISDEVICE_SHORTNAME,
            INDEX_MACHINE_ISDEVICE_DESCRIPTION,
            INDEX_MACHINE_RUNNABLE_SHORTNAME,
            INDEX_MACHINE_RUNNABLE_DESCRIPTION,
            INDEX_SYSTEM_YEAR,
            INDEX_SYSTEM_MANUFACTURER,
            INDEX_ROMOF_PARENT,
            INDEX_CLONEOF_PARENT,
            INDEX_DIPSWITCH_MACHINE_ISCONFIG,
            INDEX_ROMDUMP_ROM,
            INDEX_DISKDUMP_DISK)

    DROP_INDEXES = (
            DROP_MACHINE_ISDEVICE_SHORTNAME,
            DROP_MACHINE_ISDEVICE_DESCRIPTION,
            DROP_MACHINE_RUNNABLE_SHORTNAME,
            DROP_MACHINE_RUNNABLE_DESCRIPTION,
            DROP_SYSTEM_YEAR,
            DROP_SYSTEM_MANUFACTURER,
            DROP_ROMOF_PARENT,
            DROP_CLONEOF_PARENT,
            DROP_DIPSWITCH_MACHINE_ISCONFIG,
            DROP_ROMDUMP_ROM,
            DROP_DISKDUMP_DISK)


class UpdateQueries(object):
    ADD_FEATURETYPE = 'INSERT OR IGNORE INTO featuretype (name) VALUES (?)'
    ADD_SOURCEFILE = 'INSERT OR IGNORE INTO sourcefile (filename) VALUES (?)'
    ADD_MACHINE = 'INSERT INTO machine (shortname, description, sourcefile, isdevice, runnable) SELECT ?, ?, id, ?, ? FROM sourcefile WHERE filename = ?'
    ADD_SYSTEM = 'INSERT INTO system (id, year, manufacturer) VALUES (?, ?, ?)'
    ADD_CLONEOF = 'INSERT INTO cloneof (id, parent) VALUES (?, ?)'
    ADD_ROMOF = 'INSERT INTO romof (id, parent) VALUES (?, ?)'
    ADD_BIOSSET = 'INSERT INTO biosset (machine, name, description) VALUES (?, ?, ?)'
    ADD_BIOSSETDEFAULT = 'INSERT INTO biossetdefault (id) VALUES (?)'
    ADD_DIPSWITCH = 'INSERT INTO dipswitch (machine, isconfig, name, tag, mask) VALUES (?, ?, ?, ?, ?)'
    ADD_DIPLOCATION = 'INSERT INTO diplocation (dipswitch, bit, name, num, inverted) VALUES (?, ?, ?, ?, ?)'
    ADD_DIPVALUE = 'INSERT INTO dipvalue (dipswitch, name, value, isdefault) VALUES (?, ?, ?, ?)'
    ADD_FEATURE = 'INSERT INTO feature (machine, featuretype, status, overall) SELECT ?, id, ?, ? FROM featuretype WHERE name = ?'
    ADD_SLOT = 'INSERT INTO slot (machine, name) VALUES (?, ?)'
    ADD_RAMOPTION = 'INSERT INTO ramoption (machine, size, name) VALUES (?, ?, ?)'
    ADD_RAMDEFAULT = 'INSERT INTO ramdefault (machine, size) VALUES (?, ?)'
    ADD_ROM = 'INSERT OR IGNORE INTO rom (crc, sha1) VALUES (?, ?)'
    ADD_ROMDUMP = 'INSERT OR IGNORE INTO romdump (machine, rom, name, bad) SELECT ?, id, ?, ? FROM rom WHERE crc = ? AND sha1 = ?'
    ADD_DISK = 'INSERT OR IGNORE INTO disk (sha1) VALUES (?)'
    ADD_DISKDUMP = 'INSERT OR IGNORE INTO diskdump (machine, disk, name, bad) SELECT ?, id, ?, ? FROM disk WHERE sha1 = ?'

    ADD_TEMPORARY_DEVICEREFERENCE = 'INSERT OR IGNORE INTO temp_devicereference (machine, device) VALUES (?, ?)'
    ADD_TEMPORARY_SLOTOPTION = 'INSERT INTO temp_slotoption (slot, device, name) VALUES (?, ?, ?)'
    ADD_TEMPORARY_SLOTDEFAULT = 'INSERT INTO temp_slotdefault (id, slotoption) VALUES (?, ?)'

    FINALISE_DEVICEREFERENCES = 'INSERT INTO devicereference (id, machine, device) SELECT temp_devicereference.id, temp_devicereference.machine, machine.id FROM temp_devicereference LEFT JOIN machine ON temp_devicereference.device = machine.shortname'
    FINALISE_SLOTOPTIONS = 'INSERT INTO slotoption (id, slot, device, name) SELECT temp_slotoption.id, temp_slotoption.slot, machine.id, temp_slotoption.name FROM temp_slotoption LEFT JOIN machine ON temp_slotoption.device = machine.shortname'
    FINALISE_SLOTDEFAULTS = 'INSERT INTO slotdefault (id, slotoption) SELECT id, slotoption FROM temp_slotdefault'


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
                    'WHERE id IN (SELECT machine FROM devicereference WHERE device IN (SELECT id FROM machine WHERE sourcefile IN (SELECT id FROM sourcefile WHERE filename GLOB ?))) AND runnable = 1 ' \
                    'ORDER BY shortname ASC',
                    patterns)
        elif self.is_glob(*patterns):
            return self.dbcurs.execute(
                    'SELECT shortname, description ' \
                    'FROM machine ' \
                    'WHERE id IN (SELECT machine FROM devicereference WHERE device IN (SELECT id FROM machine WHERE sourcefile IN (SELECT id FROM sourcefile WHERE filename GLOB ?' + (' OR filename GLOB ?' * (len(patterns) - 1)) + '))) AND runnable = 1 ' \
                    'ORDER BY shortname ASC',
                    patterns)
        else:
            return self.dbcurs.execute(
                    'SELECT shortname, description ' \
                    'FROM machine ' \
                    'WHERE id IN (SELECT machine FROM devicereference WHERE device IN (SELECT id FROM machine WHERE sourcefile IN (SELECT id FROM sourcefile WHERE filename IN (?' + (', ?' * (len(patterns) - 1)) + ')))) AND runnable = 1 ' \
                    'ORDER BY shortname ASC',
                    patterns)

    def get_machine_id(self, machine):
        return (self.dbcurs.execute('SELECT id FROM machine WHERE shortname = ?', (machine, )).fetchone() or (None, ))[0]

    def get_machine_info(self, machine):
        return self.dbcurs.execute(
                'SELECT machine.id AS id, machine.description AS description, machine.isdevice AS isdevice, machine.runnable AS runnable, sourcefile.filename AS sourcefile, system.year AS year, system.manufacturer AS manufacturer, cloneof.parent AS cloneof, romof.parent AS romof ' \
                'FROM machine JOIN sourcefile ON machine.sourcefile = sourcefile.id LEFT JOIN system ON machine.id = system.id LEFT JOIN cloneof ON system.id = cloneof.id LEFT JOIN romof ON system.id = romof.id ' \
                'WHERE machine.shortname = ?',
                (machine, ))

    def get_biossets(self, machine):
        return self.dbcurs.execute(
                'SELECT biosset.name AS name, biosset.description AS description, COUNT(biossetdefault.id) AS isdefault ' \
                'FROM biosset LEFT JOIN biossetdefault USING (id) ' \
                'WHERE biosset.machine = ? ' \
                'GROUP BY biosset.id',
                (machine, ))

    def get_devices_referenced(self, machine):
        return self.dbcurs.execute(
                'SELECT machine.shortname AS shortname, machine.description AS description, sourcefile.filename AS sourcefile ' \
                'FROM devicereference LEFT JOIN machine ON devicereference.device = machine.id LEFT JOIN sourcefile ON machine.sourcefile = sourcefile.id ' \
                'WHERE devicereference.machine = ?',
                (machine, ))

    def get_device_references(self, device):
        return self.dbcurs.execute(
                'SELECT machine.shortname AS shortname, machine.description AS description, sourcefile.filename AS sourcefile ' \
                'FROM machine JOIN sourcefile ON machine.sourcefile = sourcefile.id ' \
                'WHERE machine.id IN (SELECT machine FROM devicereference WHERE device = ?)',
                (device, ))

    def get_compatible_slots(self, device):
        return self.dbcurs.execute(
                'SELECT machine.shortname AS shortname, machine.description AS description, slot.name AS slot, slotoption.name AS slotoption, sourcefile.filename AS sourcefile ' \
                'FROM slotoption JOIN slot ON slotoption.slot = slot.id JOIN machine on slot.machine = machine.id JOIN sourcefile ON machine.sourcefile = sourcefile.id '
                'WHERE slotoption.device = ?',
                (device, ))

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

    def count_slots(self, machine):
        return self.dbcurs.execute(
                'SELECT COUNT(*) FROM slot WHERE machine = ?', (machine, )).fetchone()[0]

    def get_feature_flags(self, machine):
        return self.dbcurs.execute(
                'SELECT featuretype.name AS featuretype, feature.status AS status, feature.overall AS overall ' \
                'FROM feature JOIN featuretype ON feature.featuretype = featuretype.id ' \
                'WHERE feature.machine = ?',
                (machine, ))

    def get_slot_defaults(self, machine):
        return self.dbcurs.execute(
                'SELECT slot.name AS name, slotoption.name AS option ' \
                'FROM slot JOIN slotdefault ON slot.id = slotdefault.id JOIN slotoption ON slotdefault.slotoption = slotoption.id ' \
                'WHERE slot.machine = ?',
                (machine, ))

    def get_slot_options(self, machine):
        return self.dbcurs.execute(
                'SELECT slot.name AS slot, slotoption.name AS option, machine.shortname AS shortname, machine.description AS description ' \
                'FROM slot JOIN slotoption ON slot.id = slotoption.slot JOIN machine ON slotoption.device = machine.id ' \
                'WHERE slot.machine = ?',
                (machine, ))

    def get_ram_options(self, machine):
        return self.dbcurs.execute(
                'SELECT ramoption.name AS name, ramoption.size AS size, COUNT(ramdefault.machine) AS isdefault ' \
                'FROM ramoption LEFT JOIN ramdefault USING (machine, size) WHERE ramoption.machine = ? ' \
                'GROUP BY ramoption.machine, ramoption.size ' \
                'ORDER BY ramoption.size',
                (machine, ))

    def get_rom_dumps(self, crc, sha1):
        return self.dbcurs.execute(
                'SELECT machine.shortname AS shortname, machine.description AS description, romdump.name AS label, romdump.bad AS bad ' \
                'FROM romdump LEFT JOIN machine ON romdump.machine = machine.id ' \
                'WHERE romdump.rom = (SELECT id FROM rom WHERE crc = ? AND sha1 = ?)',
                (crc, sha1))

    def get_disk_dumps(self, sha1):
        return self.dbcurs.execute(
                'SELECT machine.shortname AS shortname, machine.description AS description, diskdump.name AS label, diskdump.bad AS bad ' \
                'FROM diskdump LEFT JOIN machine ON diskdump.machine = machine.id ' \
                'WHERE diskdump.disk = (SELECT id FROM disk WHERE sha1 = ?)',
                (sha1, ))


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

    def add_biosset(self, machine, name, description):
        self.dbcurs.execute(UpdateQueries.ADD_BIOSSET, (machine, name, description))
        return self.dbcurs.lastrowid

    def add_biossetdefault(self, biosset):
        self.dbcurs.execute(UpdateQueries.ADD_BIOSSETDEFAULT, (biosset, ))
        return self.dbcurs.lastrowid

    def add_devicereference(self, machine, device):
        self.dbcurs.execute(UpdateQueries.ADD_TEMPORARY_DEVICEREFERENCE, (machine, device))

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

    def add_slot(self, machine, name):
        self.dbcurs.execute(UpdateQueries.ADD_SLOT, (machine, name))
        return self.dbcurs.lastrowid

    def add_slotoption(self, slot, device, name):
        self.dbcurs.execute(UpdateQueries.ADD_TEMPORARY_SLOTOPTION, (slot, device, name))
        return self.dbcurs.lastrowid

    def add_slotdefault(self, slot, slotoption):
        self.dbcurs.execute(UpdateQueries.ADD_TEMPORARY_SLOTDEFAULT, (slot, slotoption))
        return self.dbcurs.lastrowid

    def add_ramoption(self, machine, name, size):
        self.dbcurs.execute(UpdateQueries.ADD_RAMOPTION, (machine, size, name))
        return self.dbcurs.lastrowid

    def add_ramdefault(self, machine, size):
        self.dbcurs.execute(UpdateQueries.ADD_RAMDEFAULT, (machine, size))
        return self.dbcurs.lastrowid

    def add_rom(self, crc, sha1):
        self.dbcurs.execute(UpdateQueries.ADD_ROM, (crc, sha1))
        return self.dbcurs.lastrowid

    def add_romdump(self, machine, name, crc, sha1, bad):
        self.dbcurs.execute(UpdateQueries.ADD_ROMDUMP, (machine, name, 1 if bad else 0, crc, sha1))
        return self.dbcurs.lastrowid

    def add_disk(self, sha1):
        self.dbcurs.execute(UpdateQueries.ADD_DISK, (sha1, ))
        return self.dbcurs.lastrowid

    def add_diskdump(self, machine, name, sha1, bad):
        self.dbcurs.execute(UpdateQueries.ADD_DISKDUMP, (machine, name, 1 if bad else 0, sha1))
        return self.dbcurs.lastrowid


class QueryConnection(object):
    def __init__(self, database, **kwargs):
        super(QueryConnection, self).__init__(**kwargs)
        if sys.version_info >= (3, 4):
            self.dbconn = sqlite3.connect('file:' + urllib.request.pathname2url(database) + '?mode=ro', uri=True, check_same_thread=False)
        else:
            self.dbconn = sqlite3.connect(database, check_same_thread=False)
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
        self.dbconn.execute('PRAGMA page_size = 4096')
        self.dbconn.execute('PRAGMA foreign_keys = ON')

    def commit(self):
        self.dbconn.commit()

    def rollback(self):
        self.dbconn.rollback()

    def close(self):
        self.dbconn.close()

    def cursor(self):
        return UpdateCursor(self.dbconn)

    def prepare_for_load(self):
        # here be dragons - this is a poor man's DROP ALL TABLES etc.
        self.dbconn.execute('PRAGMA foreign_keys = OFF')
        for query in self.dbconn.execute('SELECT \'DROP INDEX \' || name FROM sqlite_master WHERE type = \'index\' AND NOT name GLOB \'sqlite_autoindex_*\'').fetchall():
            self.dbconn.execute(query[0])
        for query in self.dbconn.execute('SELECT \'DROP TABLE \' || name FROM sqlite_master WHERE type = \'table\'').fetchall():
            self.dbconn.execute(query[0])
        self.dbconn.execute('PRAGMA foreign_keys = ON')

        # this is where the sanity starts
        for query in SchemaQueries.DROP_INDEXES:
            self.dbconn.execute(query)
        for query in SchemaQueries.CREATE_TABLES:
            self.dbconn.execute(query)
        for query in SchemaQueries.CREATE_TEMPORARY_TABLES:
            self.dbconn.execute(query)
        self.dbconn.commit()

    def finalise_load(self):
        self.dbconn.execute(UpdateQueries.FINALISE_DEVICEREFERENCES)
        self.dbconn.commit()
        self.dbconn.execute(SchemaQueries.DROP_TEMPORARY_DEVICEREFERENCE)
        self.dbconn.execute(UpdateQueries.FINALISE_SLOTOPTIONS)
        self.dbconn.commit()
        self.dbconn.execute(SchemaQueries.DROP_TEMPORARY_SLOTOPTION)
        self.dbconn.execute(UpdateQueries.FINALISE_SLOTDEFAULTS)
        self.dbconn.commit()
        self.dbconn.execute(SchemaQueries.DROP_TEMPORARY_SLOTDEFAULT)
        for query in SchemaQueries.CREATE_INDEXES:
            self.dbconn.execute(query)
        self.dbconn.commit()
