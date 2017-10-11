#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import os
import re
import sys
import xml.sax
import xml.sax.saxutils
import zlib


# workaround for version incompatibility
if sys.version_info > (3, ):
    long = int


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


class LayoutChecker(Minifyer):
    VARPATTERN = re.compile('^~scr(0|[1-9][0-9]*)(native[xy]aspect|width|height)~$')
    SHAPES = frozenset(('disk', 'led14seg', 'led14segsc', 'led16seg', 'led16segsc', 'led7seg', 'led8seg_gts1', 'rect'))
    OBJECTS = frozenset(('backdrop', 'bezel', 'cpanel', 'marquee', 'overlay'))

    def __init__(self, output, **kwargs):
        super(LayoutChecker, self).__init__(output=output, **kwargs)
        self.locator = None
        self.errors = 0
        self.elements = { }
        self.groups = { }
        self.views = { }
        self.referenced_elements = { }
        self.referenced_groups = { }
        self.have_bounds = [ ]
        self.have_color = [ ]

    def formatLocation(self):
        return '%s:%d:%d' % (self.locator.getSystemId(), self.locator.getLineNumber(), self.locator.getColumnNumber())

    def handleError(self, msg):
        self.errors += 1
        sys.stderr.write('error: %s: %s\n' % (self.formatLocation(), msg))

    def checkBoundsDimension(self, attrs, name):
        if name in attrs:
            try:
                return float(attrs[name])
            except:
                if not self.VARPATTERN.match(attrs[name]):
                    self.handleError('Element bounds attribute %s "%s" is not numeric' % (name, attrs[name]))
        return None

    def checkBounds(self, attrs):
        if self.have_bounds[-1]:
            self.handleError('Duplicate element bounds')
        else:
            self.have_bounds[-1] = True
        left = self.checkBoundsDimension(attrs, 'left')
        top = self.checkBoundsDimension(attrs, 'top')
        right = self.checkBoundsDimension(attrs, 'right')
        bottom = self.checkBoundsDimension(attrs, 'bottom')
        x = self.checkBoundsDimension(attrs, 'bottom')
        y = self.checkBoundsDimension(attrs, 'bottom')
        width = self.checkBoundsDimension(attrs, 'width')
        height = self.checkBoundsDimension(attrs, 'height')
        if (left is not None) and (right is not None) and (left > right):
            self.handleError('Element bounds attribute left "%s" is greater than attribute right "%s"' % (
                    attrs['left'],
                    attrs['right']))
        if (top is not None) and (bottom is not None) and (top > bottom):
            self.handleError('Element bounds attribute top "%s" is greater than attribute bottom "%s"' % (
                    attrs['top'],
                    attrs['bottom']))
        if (width is not None) and (0.0 > width):
            self.handleError('Element bounds attribute width "%s" is negative' % (attrs['width'], ))
        if (height is not None) and (0.0 > height):
            self.handleError('Element bounds attribute height "%s" is negative' % (attrs['height'], ))
        if ('left' not in attrs) and ('x' not in attrs):
            self.handleError('Element bounds has neither attribute left nor attribute x')
        has_ltrb = ('left' in attrs) or ('top' in attrs) or ('right' in attrs) or ('bottom' in attrs)
        has_origin_size = ('x' in attrs) or ('y' in attrs) or ('width' in attrs) or ('height' in attrs)
        if has_ltrb and has_origin_size:
            self.handleError('Element bounds has both left/top/right/bottom and origin/size')

    def checkColorChannel(self, attrs, name):
        if name in attrs:
            try:
                channel = float(attrs[name])
                if (0.0 > channel) or (1.0 < channel):
                    self.handleError('Element color attribute %s "%s" outside valid range 0.0-1.0' % (name, attrs[name]))
            except:
                self.handleError('Element color attribute %s "%s" is not numeric' % (name, attrs[name]))

    def checkGroupViewItem(self, name, attrs):
        if name in self.OBJECTS:
            if 'element' not in attrs:
                self.handleError('Element %s missing attribute element', (name, ))
            elif attrs['element'] not in self.referenced_elements:
                self.referenced_elements[attrs['element']] = self.formatLocation()
            self.in_object = True
            self.have_bounds.append(False)
        elif 'screen' == name:
            if 'index' in attrs:
                try:
                    index = long(attrs['index'])
                    if 0 > index:
                        self.handleError('Element screen attribute index "%s" is negative' % (attrs['index'], ))
                except:
                    self.handleError('Element screen attribute index "%s" is not an integer' % (attrs['index'], ))
            self.in_object = True
            self.have_bounds.append(False)
        elif 'group' == name:
            if 'ref' not in attrs:
                self.handleError('Element group missing attribute ref')
            elif attrs['ref'] not in self.referenced_groups:
                self.referenced_groups[attrs['ref']] = self.formatLocation()
            self.in_object = True
            self.have_bounds.append(False)
        elif 'bounds' == name:
            self.checkBounds(attrs)
            self.ignored_depth = 1
        else:
            self.handleError('Encountered unexpected element %s' % (name, ))
            self.ignored_depth = 1

    def setDocumentLocator(self, locator):
        self.locator = locator
        super(LayoutChecker, self).setDocumentLocator(locator)

    def startDocument(self):
        self.in_layout = False
        self.in_element = False
        self.in_group = False
        self.in_view = False
        self.in_shape = False
        self.in_object = False
        self.ignored_depth = 0
        super(LayoutChecker, self).startDocument()

    def endDocument(self):
        self.locator = None
        self.elements.clear()
        self.groups.clear()
        self.views.clear()
        self.referenced_elements.clear()
        self.referenced_groups.clear()
        del self.have_bounds[:]
        del self.have_color[:]
        super(LayoutChecker, self).endDocument()

    def startElement(self, name, attrs):
        if 0 < self.ignored_depth:
            self.ignored_depth += 1
        elif not self.in_layout:
            if 'mamelayout' != name:
                self.ignored_depth = 1
                self.handleError('Expected root element mamelayout but found %s' % (name, ))
            else:
                if 'version' not in attrs:
                    self.handleError('Element mamelayout missing attribute version')
                else:
                    try:
                        long(attrs['version'])
                    except:
                        self.handleError('Element mamelayout attribute version "%s" is not an integer' % (attrs['version'], ))
                self.in_layout = True
        elif self.in_object:
            if 'bounds' == name:
                self.checkBounds(attrs)
            self.ignored_depth = 1
        elif self.in_shape:
            if 'bounds' == name:
                self.checkBounds(attrs)
            elif 'color' == name:
                if self.have_color[-1]:
                    self.handleError('Duplicate bounds element')
                else:
                    self.have_color[-1] = True
                self.checkColorChannel(attrs, 'red')
                self.checkColorChannel(attrs, 'green')
                self.checkColorChannel(attrs, 'blue')
                self.checkColorChannel(attrs, 'alpha')
            self.ignored_depth = 1
        elif self.in_element:
            if name in self.SHAPES:
                self.in_shape = True
                self.have_bounds.append(False)
                self.have_color.append(False)
            elif 'text' == name:
                if 'string' not in attrs:
                    self.handleError('Element bounds missing attribute string')
                if 'align' in attrs:
                    try:
                        align = long(attrs['align'])
                        if (0 > align) or (2 < align):
                            self.handleError('Element text attribute align "%s" not in valid range 0-2' % (attrs['align'], ))
                    except:
                        self.handleError('Element text attribute align "%s" is not an integer' % (attrs['align'], ))
                self.in_shape = True
                self.have_bounds.append(False)
                self.have_color.append(False)
            else:
                self.ignored_depth = 1
        elif self.in_group or self.in_view:
            self.checkGroupViewItem(name, attrs)
        elif 'element' == name:
            if 'name' not in attrs:
                self.handleError('Element element missing attribute name')
            else:
                if attrs['name'] in self.elements:
                    self.handleError('Element element has duplicate name (previous %s)' % (self.elements[attrs['name']], ))
                else:
                    self.elements[attrs['name']] = self.formatLocation()
            self.in_element = True
        elif 'group' == name:
            if 'name' not in attrs:
                self.handleError('Element group missing attribute name')
            else:
                if attrs['name'] in self.groups:
                    self.handleError('Element group has duplicate name (previous %s)' % (self.groups[attrs['name']], ))
                else:
                    self.groups[attrs['name']] = self.formatLocation()
            self.in_group = True
            self.have_bounds.append(False)
        elif 'view' == name:
            if 'name' not in attrs:
                self.handleError('Element view missing attribute name')
            else:
                if attrs['name'] in self.views:
                    self.handleError('Element view has duplicate name (previous %s)' % (self.views[attrs['name']], ))
                else:
                    self.views[attrs['name']] = self.formatLocation()
            self.in_view = True
            self.have_bounds.append(False)
        elif 'script' == name:
            self.ignored_depth = 1
        else:
            self.handleError('Encountered unexpected element %s' % (name, ))
            self.ignored_depth = 1
        super(LayoutChecker, self).startElement(name, attrs)

    def endElement(self, name):
        if 0 < self.ignored_depth:
            self.ignored_depth -= 1
        elif self.in_object:
            self.in_object = False
            self.have_bounds.pop()
        elif self.in_shape:
            self.in_shape = False
            self.have_bounds.pop()
            self.have_color.pop()
        elif self.in_element:
            self.in_element = False
        elif self.in_group:
            self.in_group = False
            self.have_bounds.pop()
        elif self.in_view:
            self.in_view = False
            self.have_bounds.pop()
        elif self.in_layout:
            for element in self.referenced_elements:
                if element not in self.elements:
                    self.handleError('Element "%s" not found (first referenced at %s)' % (element, self.referenced_elements[element]))
            for group in self.referenced_groups:
                if group not in self.groups:
                    self.handleError('Group "%s" not found (first referenced at %s)' % (group, self.referenced_groups[group]))
            self.in_layout = False
        super(LayoutChecker, self).endElement(name)


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
    content_handler = LayoutChecker(output)
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
    if (content_handler.errors > 0) or (error_handler.errors > 0) or (error_handler.warnings > 0):
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
