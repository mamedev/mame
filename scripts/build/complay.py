#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import os
import sys
import xml.sax
import xml.sax.saxutils
import zlib


class ErrorHandler(object):
    def __init__(self, **kwargs):
        super(ErrorHandler, self).__init__(**kwargs)
        self.errors = 0
        self.warnings = 0

    def error(self, exception):
        self.errors += 1
        sys.stderr.write('error: %s' % (exception))

    def fatalError(self, exception):
        raise exception

    def warning(self, exception):
        self.warnings += 1
        sys.stderr.write('warning: %s' % (exception))


class Minifyer(object):
    def __init__(self, output, **kwargs):
        super(Minifyer, self).__init__(**kwargs)

        self.output = output
        self.incomplete_tag = False
        self.element_content = ''

    def setDocumentLocator(self, locator):
        pass

    def startDocument(self):
        self.output('<?xml version="1.0"?>')

    def endDocument(self):
        self.output('\n')

    def startElement(self, name, attrs):
        self.flushElementContent()
        if self.incomplete_tag:
            self.output('>')
        self.output('<%s' % (name))
        for name in attrs.getNames():
            self.output(' %s=%s' % (name, xml.sax.saxutils.quoteattr(attrs[name])))
        self.incomplete_tag = True

    def endElement(self, name):
        self.flushElementContent()
        if self.incomplete_tag:
            self.output('/>')
        else:
            self.output('</%s>' % (name))
        self.incomplete_tag = False

    def characters(self, content):
        self.element_content += content

    def ignorableWhitespace(self, whitespace):
        pass

    def processingInstruction(self, target, data):
        pass

    def flushElementContent(self):
        self.element_content = self.element_content.strip()
        if self.element_content:
            if self.incomplete_tag:
                self.output('>')
                self.incomplete_tag = False
            self.output(xml.sax.saxutils.escape(self.element_content))
            self.element_content = ''


class XmlError(Exception):
    pass


def compressLayout(src, dst, comp):
    state = [0, 0]
    def write(block):
        for ch in bytearray(block):
            if 0 == state[0]:
                dst('\t')
            elif 0 == (state[0] % 32):
                dst(',\n\t')
            else:
                dst(', ')
            state[0] += 1
            dst('%3u' % (ch))

    def output(text):
        block = text.encode('UTF-8')
        state[1] += len(block)
        write(comp.compress(block))

    error_handler = ErrorHandler()
    content_handler = Minifyer(output)
    parser = xml.sax.make_parser()
    parser.setErrorHandler(error_handler)
    parser.setContentHandler(content_handler)
    try:
        parser.parse(src)
        write(comp.flush())
        dst('\n')
    except xml.sax.SAXException as exception:
        print('fatal error: %s' % (exception))
        raise XmlError('Fatal error parsing XML')
    if (error_handler.errors > 0) or (error_handler.warnings > 0):
        raise XmlError('Error(s) and/or warning(s) parsing XML')

    return state[1], state[0]


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print('Usage:')
        print('  complay <source.lay> <output.h> <varname>')
        sys.exit(0 if len(sys.argv) <= 1 else 1)

    srcfile = sys.argv[1]
    dstfile = sys.argv[2]
    varname = sys.argv[3]

    comp_type = 1
    try:
        dst = open(dstfile,'w')
        dst.write('static const unsigned char %s_data[] = {\n' % (varname))
        byte_count, comp_size = compressLayout(srcfile, lambda x: dst.write(x), zlib.compressobj())
        dst.write('};\n\n')
        dst.write('const internal_layout %s = {\n' % (varname))
        dst.write('\t%d, sizeof(%s_data), %d, %s_data\n' % (byte_count, varname, comp_type, varname))
        dst.write('};\n')
        dst.close()
    except XmlError:
        dst.close()
        os.remove(dstfile)
        sys.exit(2)
    except IOError:
        sys.stderr.write("Unable to open output file '%s'\n" % dstfile)
        os.remove(dstfile)
        dst.close()
        sys.exit(3)
