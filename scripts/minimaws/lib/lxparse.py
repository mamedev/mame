#!/usr/bin/python3
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

from . import dbaccess

import os
import os.path
import subprocess
import xml.sax
import xml.sax.saxutils


class ElementHandlerBase(object):
    def __init__(self, parent, **kwargs):
        super().__init__(**kwargs)
        self.dbconn = parent.dbconn if parent is not None else None
        self.locator = parent.locator if parent is not None else None
        self.depth = 0
        self.childhandler = None
        self.childdepth = 0

    def startMainElement(self, name, attrs):
        pass

    def endMainElement(self, name):
        pass

    def mainCharacters(self, content):
        pass

    def mainIgnorableWitespace(self, whitespace):
        pass

    def startChildElement(self, name, attrs):
        pass

    def endChildElement(self, name):
        pass

    def childCharacters(self, content):
        pass

    def childIgnorableWitespace(self, whitespace):
        pass

    def endChildHandler(self, name, handler):
        pass

    def setChildHandler(self, name, attrs, handler):
        self.depth -= 1
        self.childhandler = handler
        self.childdepth += 1
        handler.startElement(name, attrs)

    def setDocumentLocator(self, locator):
        self.locator = locator

    def startElement(self, name, attrs):
        if self.childhandler is not None:
            self.childdepth += 1
            self.childhandler.startElement(name, attrs)
        else:
            self.depth += 1
            if 1 == self.depth:
                self.startMainElement(name, attrs)
            else:
                self.startChildElement(name, attrs)

    def endElement(self, name):
        if self.childhandler is not None:
            self.childdepth -= 1
            self.childhandler.endElement(name)
            if 0 == self.childdepth:
                self.endChildHandler(name, self.childhandler)
                self.childhandler = None
        else:
            self.depth -= 1
            if 0 == self.depth:
                self.endMainElement(name)
            else:
                self.endChildElement(name)

    def characters(self, content):
        if self.childhandler is not None:
            self.childhandler.characters(content)
        elif 1 < self.depth:
            self.childCharacters(content)
        else:
            self.mainCharacters(content)

    def ignorableWhitespace(self, content):
        if self.childhandler is not None:
            self.childhandler.ignorableWhitespace(content)
        elif 1 < self.depth:
            self.childIgnorableWitespace(content)
        else:
            self.mainIgnorableWitespace(content)


class ElementHandler(ElementHandlerBase):
    IGNORE = ElementHandlerBase(parent=None)


class TextAccumulator(ElementHandler):
    def __init__(self, parent, **kwargs):
        super().__init__(parent=parent, **kwargs)
        self.text = ''

    def mainCharacters(self, content):
        self.text += content


class DipSwitchHandler(ElementHandler):
    def __init__(self, parent, **kwargs):
        super().__init__(parent=parent, **kwargs)
        self.dbcurs = parent.dbcurs
        self.machine = parent.id

    def startMainElement(self, name, attrs):
        self.mask = int(attrs['mask'])
        self.bit = 0
        self.id = self.dbcurs.add_dipswitch(self.machine, name == 'configuration', attrs['name'], attrs['tag'], self.mask)

    def startChildElement(self, name, attrs):
        if (name == 'diplocation') or (name == 'conflocation'):
            while (0 != self.mask) and not (self.mask & 1):
                self.mask >>= 1
                self.bit += 1
            self.dbcurs.add_diplocation(self.id, self.bit, attrs['name'], attrs['number'], attrs.get('inverted', 'no')  == 'yes')
            self.mask >>= 1
            self.bit += 1
        elif (name == 'dipvalue') or (name == 'confsetting'):
            self.dbcurs.add_dipvalue(self.id, attrs['name'], attrs['value'], attrs.get('default', 'no') == 'yes')
        self.setChildHandler(name, attrs, self.IGNORE)


class SlotHandler(ElementHandler):
    def __init__(self, parent, **kwargs):
        super().__init__(parent=parent, **kwargs)
        self.dbcurs = parent.dbcurs
        self.machine = parent.id

    def startMainElement(self, name, attrs):
        self.id = self.dbcurs.add_slot(self.machine, attrs['name'])

    def startChildElement(self, name, attrs):
        if name == 'slotoption':
            option = self.dbcurs.add_slotoption(self.id, attrs['devname'], attrs['name'])
            if attrs.get('default') == 'yes':
                self.dbcurs.add_slotdefault(self.id, option)
        self.setChildHandler(name, attrs, self.IGNORE)


class RamOptionHandler(TextAccumulator):
    def __init__(self, parent, **kwargs):
        super().__init__(parent=parent, **kwargs)
        self.dbcurs = parent.dbcurs
        self.machine = parent.id

    def startMainElement(self, name, attrs):
        self.name = attrs['name']
        self.default = attrs.get('default', 'no') == 'yes';

    def endMainElement(self, name):
        self.size = int(self.text)
        self.dbcurs.add_ramoption(self.machine, self.name, self.size)
        if self.default:
            self.dbcurs.add_ramdefault(self.machine, self.size)


class MachineHandler(ElementHandler):
    CHILD_HANDLERS = {
            'description':      TextAccumulator,
            'year':             TextAccumulator,
            'manufacturer':     TextAccumulator,
            'dipswitch':        DipSwitchHandler,
            'configuration':    DipSwitchHandler,
            'slot':             SlotHandler,
            'ramoption':        RamOptionHandler }

    def __init__(self, parent, **kwargs):
        super().__init__(parent=parent, **kwargs)
        self.dbcurs = self.dbconn.cursor()

    def startMainElement(self, name, attrs):
        self.shortname = attrs['name']
        self.sourcefile = attrs['sourcefile']
        self.isdevice = attrs.get('isdevice', 'no') == 'yes'
        self.runnable = attrs.get('runnable', 'yes') == 'yes'
        self.cloneof = attrs.get('cloneof')
        self.romof = attrs.get('romof')
        self.dbcurs.add_sourcefile(self.sourcefile)

    def endMainElement(self, name):
        self.dbcurs.close()

    def startChildElement(self, name, attrs):
        if name in self.CHILD_HANDLERS:
            self.setChildHandler(name, attrs, self.CHILD_HANDLERS[name](self))
        else:
            if name == 'biosset':
                bios = self.dbcurs.add_biosset(self.id, attrs['name'], attrs['description'])
                if attrs.get('default', 'no') == 'yes':
                    self.dbcurs.add_biossetdefault(bios)
            elif name == 'device_ref':
                self.dbcurs.add_devicereference(self.id, attrs['name'])
            elif name == 'feature':
                self.dbcurs.add_featuretype(attrs['type'])
                status = 0 if 'status' not in attrs else 2 if attrs['status'] == 'unemulated' else 1
                overall = status if 'overall' not in attrs else 2 if attrs['overall'] == 'unemulated' else 1
                self.dbcurs.add_feature(self.id, attrs['type'], status, overall)
            elif name == 'softwarelist':
                self.dbcurs.add_machinesoftwarelist(self.id, attrs['name'], attrs['tag'], attrs['status'])
            elif name == 'rom':
                crc = attrs.get('crc')
                sha1 = attrs.get('sha1')
                if (crc is not None) and (sha1 is not None):
                    crc = int(crc, 16)
                    sha1 = sha1.lower()
                    self.dbcurs.add_rom(crc, sha1)
                    status = attrs.get('status', 'good')
                    self.dbcurs.add_romdump(self.id, attrs['name'], crc, sha1, status != 'good')
            elif name == 'disk':
                sha1 = attrs.get('sha1')
                if sha1 is not None:
                    sha1 = sha1.lower()
                    self.dbcurs.add_disk(sha1)
                    status = attrs.get('status', 'good')
                    self.dbcurs.add_diskdump(self.id, attrs['name'], sha1, status != 'good')
            self.setChildHandler(name, attrs, self.IGNORE)

    def endChildHandler(self, name, handler):
        if name == 'description':
            self.description = handler.text
            self.id = self.dbcurs.add_machine(self.shortname, self.description, self.sourcefile, self.isdevice, self.runnable)
            if self.cloneof is not None:
                self.dbcurs.add_cloneof(self.id, self.cloneof)
            if self.romof is not None:
                self.dbcurs.add_romof(self.id, self.romof)
        elif name == 'year':
            self.year = handler.text
        elif name == 'manufacturer':
            self.manufacturer = handler.text
            self.dbcurs.add_system(self.id, self.year, self.manufacturer)


class ListXmlHandler(ElementHandler):
    def __init__(self, dbconn, **kwargs):
        super().__init__(parent=None, **kwargs)
        self.dbconn = dbconn

    def startDocument(self):
        pass

    def endDocument(self):
        pass

    def startMainElement(self, name, attrs):
        if name != 'mame':
            raise xml.sax.SAXParseException(
                    msg=('Expected "mame" element but found "%s"' % (name, )),
                    exception=None,
                    locator=self.locator)
        self.machines = 0

    def endMainElement(self, name):
        self.dbconn.commit()

    def startChildElement(self, name, attrs):
        if name != 'machine':
            raise xml.sax.SAXParseException(
                    msg=('Expected "machine" element but found "%s"' % (name, )),
                    exception=None,
                    locator=self.locator)
        self.setChildHandler(name, attrs, MachineHandler(self))

    def endChildHandler(self, name, handler):
        if name == 'machine':
            if self.machines >= 1023:
                self.dbconn.commit()
                self.machines = 0
            else:
                self.machines += 1

    def processingInstruction(self, target, data):
        pass


class DataAreaHandler(ElementHandler):
    def __init__(self, parent, **kwargs):
        super().__init__(parent=parent, **kwargs)
        self.dbcurs = parent.dbcurs
        self.part = parent.id

    def startChildElement(self, name, attrs):
        if name == 'rom':
            crc = attrs.get('crc')
            sha1 = attrs.get('sha1')
            if ('name' in attrs) and (crc is not None) and (sha1 is not None):
                crc = int(crc, 16)
                sha1 = sha1.lower()
                self.dbcurs.add_rom(crc, sha1)
                status = attrs.get('status', 'good')
                self.dbcurs.add_softwareromdump(self.part, attrs['name'], crc, sha1, status != 'good')
        self.setChildHandler(name, attrs, self.IGNORE)


class DiskAreaHandler(ElementHandler):
    def __init__(self, parent, **kwargs):
        super().__init__(parent=parent, **kwargs)
        self.dbcurs = parent.dbcurs
        self.part = parent.id

    def startChildElement(self, name, attrs):
        if name == 'disk':
            sha1 = attrs.get('sha1')
            if sha1 is not None:
                sha1 = sha1.lower()
                self.dbcurs.add_disk(sha1)
                status = attrs.get('status', 'good')
                self.dbcurs.add_softwarediskdump(self.part, attrs['name'], sha1, status != 'good')
        self.setChildHandler(name, attrs, self.IGNORE)


class SoftwarePartHandler(ElementHandler):
    CHILD_HANDLERS = {
            'dataarea':         DataAreaHandler,
            'diskarea':         DiskAreaHandler }

    def __init__(self, parent, **kwargs):
        super().__init__(parent=parent, **kwargs)
        self.dbcurs = parent.dbcurs
        self.software = parent.id

    def startMainElement(self, name, attrs):
        self.id = self.dbcurs.add_softwarepart(self.software, attrs['name'], attrs['interface'])

    def startChildElement(self, name, attrs):
        if name in self.CHILD_HANDLERS:
            self.setChildHandler(name, attrs, self.CHILD_HANDLERS[name](self))
        else:
            if name == 'feature':
                self.dbcurs.add_softwarepartfeaturetype(attrs['name'])
                self.dbcurs.add_softwarepartfeature(self.id, attrs['name'], attrs['value'])
            self.setChildHandler(name, attrs, self.IGNORE)


class SoftwareHandler(ElementHandler):
    CHILD_HANDLERS = {
            'description':      TextAccumulator,
            'year':             TextAccumulator,
            'publisher':        TextAccumulator,
            'notes':            TextAccumulator,
            'part':             SoftwarePartHandler }

    def __init__(self, parent, **kwargs):
        super().__init__(parent=parent, **kwargs)
        self.dbcurs = self.dbconn.cursor()
        self.softwarelist = parent.id

    def startMainElement(self, name, attrs):
        self.shortname = attrs['name']
        self.cloneof = attrs.get('cloneof')
        self.supported = 0 if (attrs.get('supported', 'yes') == 'yes') else 1 if (attrs.get('supported', 'yes') == 'partial') else 2

    def endMainElement(self, name):
        self.dbcurs.close()

    def startChildElement(self, name, attrs):
        if name in self.CHILD_HANDLERS:
            self.setChildHandler(name, attrs, self.CHILD_HANDLERS[name](self))
        else:
            if name == 'info':
                self.dbcurs.add_softwareinfotype(attrs['name'])
                self.dbcurs.add_softwareinfo(self.id, attrs['name'], attrs['value'])
            elif name == 'sharedfeat':
                self.dbcurs.add_softwaresharedfeattype(attrs['name'])
                self.dbcurs.add_softwaresharedfeat(self.id, attrs['name'], attrs['value'])
            self.setChildHandler(name, attrs, self.IGNORE)

    def endChildHandler(self, name, handler):
        if name == 'description':
            self.description = handler.text
        elif name == 'year':
            self.year = handler.text
        elif name == 'publisher':
            self.publisher = handler.text
            self.id = self.dbcurs.add_software(self.softwarelist, self.shortname, self.supported, self.description, self.year, self.publisher)
            if self.cloneof is not None:
                self.dbcurs.add_softwarecloneof(self.id, self.cloneof)
        elif name == 'notes':
            self.dbcurs.add_softwarenotes(self.id, handler.text)


class SoftwareListHandler(ElementHandler):
    CHILD_HANDLERS = {
            'notes':            TextAccumulator,
            'software':         SoftwareHandler }

    def __init__(self, dbconn, **kwargs):
        super().__init__(parent=None, **kwargs)
        self.dbconn = dbconn

    def startDocument(self):
        pass

    def endDocument(self):
        pass

    def startMainElement(self, name, attrs):
        if name != 'softwarelist':
            raise xml.sax.SAXParseException(
                    msg=('Expected "softwarelist" element but found "%s"' % (name, )),
                    exception=None,
                    locator=self.locator)
        self.shortname = attrs['name']
        self.description = attrs['description']
        self.entries = 0
        dbcurs = self.dbconn.cursor()
        self.id = dbcurs.add_softwarelist(self.shortname, self.description)
        dbcurs.close()

    def endMainElement(self, name):
        self.dbconn.commit()

    def startChildElement(self, name, attrs):
        if name in self.CHILD_HANDLERS:
            self.setChildHandler(name, attrs, self.CHILD_HANDLERS[name](self))
        else:
            raise xml.sax.SAXParseException(
                    msg=('Found unexpected element "%s"' % (name, )),
                    exception=None,
                    locator=self.locator)

    def endChildHandler(self, name, handler):
        if name == 'notes':
            dbcurs = self.dbconn.cursor()
            dbcurs.add_softwarelistnotes(self.id, handler.text)
            dbcurs.close()
        elif name == 'software':
            if self.entries >= 1023:
                self.dbconn.commit()
                self.entries = 0
            else:
                self.entries += 1

    def processingInstruction(self, target, data):
        pass


def load_info(options):
    dbconn = dbaccess.UpdateConnection(options.database)
    dbconn.prepare_for_load()
    parser = xml.sax.make_parser()

    parser.setContentHandler(SoftwareListHandler(dbconn))
    for path in options.softwarepath:
        files = [os.path.join(path, f) for f in os.listdir(path) if f.endswith('.xml')]
        for filename in files:
            parser.parse(filename)

    parser.setContentHandler(ListXmlHandler(dbconn))
    if options.executable is not None:
        task = subprocess.Popen([options.executable, '-listxml'], stdout=subprocess.PIPE)
        parser.parse(task.stdout)
    else:
        parser.parse(options.file)

    dbconn.finalise_load()
