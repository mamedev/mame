#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

from . import dbaccess

import subprocess
import xml.sax
import xml.sax.saxutils


class ElementHandlerBase(object):
    def __init__(self, parent, **kwargs):
        super(ElementHandlerBase, self).__init__(**kwargs)
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
        super(TextAccumulator, self).__init__(parent=parent, **kwargs)
        self.text = ''

    def mainCharacters(self, content):
        self.text += content


class DipSwitchHandler(ElementHandler):
    def __init__(self, parent, **kwargs):
        super(DipSwitchHandler, self).__init__(parent=parent, **kwargs)
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
        super(SlotHandler, self).__init__(parent=parent, **kwargs)
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
        super(RamOptionHandler, self).__init__(parent=parent, **kwargs)
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
        super(MachineHandler, self).__init__(parent=parent, **kwargs)
        self.dbcurs = self.dbconn.cursor()

    def startMainElement(self, name, attrs):
        self.shortname = attrs['name']
        self.sourcefile = attrs['sourcefile']
        self.isdevice = attrs.get('isdevice', 'no') == 'yes'
        self.runnable = attrs.get('runnable', 'yes') == 'yes'
        self.cloneof = attrs.get('cloneof')
        self.romof = attrs.get('romof')
        self.dbcurs.add_sourcefile(self.sourcefile)

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

    def endMainElement(self, name):
        self.dbcurs.close()


class ListXmlHandler(ElementHandler):
    def __init__(self, dbconn, **kwargs):
        super(ListXmlHandler, self).__init__(parent=None, **kwargs)
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
        self.dbconn.prepare_for_load()
        self.machines = 0

    def endMainElement(self, name):
        # TODO: build index by first letter or whatever
        self.dbconn.finalise_load()

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


def load_info(options):
    parser = xml.sax.make_parser()
    parser.setContentHandler(ListXmlHandler(dbaccess.UpdateConnection(options.database)))
    if options.executable is not None:
        task = subprocess.Popen([options.executable, '-listxml'], stdout=subprocess.PIPE)
        parser.parse(task.stdout)
    else:
        parser.parse(options.file)
