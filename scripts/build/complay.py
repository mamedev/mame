#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Vas Crabb

import os
import os.path
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
    BADTAGPATTERN = re.compile('[^abcdefghijklmnopqrstuvwxyz0123456789_.:^$]')
    VARPATTERN = re.compile('^.*~[0-9A-Za-z_]+~.*$')
    FLOATCHARS = re.compile('^.*[.eE].*$')
    SHAPES = frozenset(('disk', 'dotmatrix', 'dotmatrix5dot', 'dotmatrixdot', 'led14seg', 'led14segsc', 'led16seg', 'led16segsc', 'led7seg', 'led8seg_gts1', 'rect'))
    OBJECTS = frozenset(('backdrop', 'bezel', 'cpanel', 'marquee', 'overlay'))
    ORIENTATIONS = frozenset((0, 90, 180, 270))
    YESNO = frozenset(("yes", "no"))

    def __init__(self, output, **kwargs):
        super(LayoutChecker, self).__init__(output=output, **kwargs)
        self.locator = None
        self.errors = 0
        self.elements = { }
        self.groups = { }
        self.views = { }
        self.referenced_elements = { }
        self.referenced_groups = { }

    def formatLocation(self):
        return '%s:%d:%d' % (self.locator.getSystemId(), self.locator.getLineNumber(), self.locator.getColumnNumber())

    def handleError(self, msg):
        self.errors += 1
        sys.stderr.write('error: %s: %s\n' % (self.formatLocation(), msg))

    def checkIntAttribute(self, name, attrs, key, default):
        if key not in attrs:
            return default
        val = attrs[key]
        if self.VARPATTERN.match(val):
            return None
        base = 10
        offs = 0
        if (len(val) >= 1) and ('$' == val[0]):
            base = 16
            offs = 1
        elif (len(val) >= 2) and ('0' == val[0]) and (('x' == val[1]) or ('X' == val[1])):
            base = 16
            offs = 2
        elif (len(val) >= 1) and ('#' == val[0]):
            offs = 1
        try:
            return int(val[offs:], base)
        except:
            self.handleError('Element %s attribute %s "%s" is not an integer' % (name, key, val))
            return None

    def checkFloatAttribute(self, name, attrs, key, default):
        if key not in attrs:
            return default
        val = attrs[key]
        if self.VARPATTERN.match(val):
            return None
        try:
            return float(val)
        except:
            self.handleError('Element %s attribute %s "%s" is not a floating point number' % (name, key, val))
            return None

    def checkNumericAttribute(self, name, attrs, key, default):
        if key not in attrs:
            return default
        val = attrs[key]
        if self.VARPATTERN.match(val):
            return None
        base = 0
        offs = 0
        try:
            if (len(val) >= 1) and ('$' == val[0]):
                base = 16
                offs = 1
            elif (len(val) >= 2) and ('0' == val[0]) and (('x' == val[1]) or ('X' == val[1])):
                base = 16
                offs = 2
            elif (len(val) >= 1) and ('#' == val[0]):
                base = 10
                offs = 1
            elif self.FLOATCHARS.match(val):
                return float(val)
            return int(val[offs:], base)
        except:
            self.handleError('Element %s attribute %s "%s" is not a number' % (name, key, val))
            return None

    def checkParameter(self, attrs):
        if 'name' not in attrs:
            self.handleError('Element param missing attribute name')
        else:
            name = attrs['name']
        self.checkNumericAttribute('param', attrs, 'increment', None)
        lshift = self.checkIntAttribute('param', attrs, 'lshift', None)
        if (lshift is not None) and (0 > lshift):
            self.handleError('Element param attribute lshift "%s" is negative' % (attrs['lshift'], ))
        rshift = self.checkIntAttribute('param', attrs, 'rshift', None)
        if (rshift is not None) and (0 > rshift):
            self.handleError('Element param attribute rshift "%s" is negative' % (attrs['rshift'], ))
        if self.repeat_depth and self.repeat_depth[-1]:
            if 'start' in attrs:
                if 'value' in attrs:
                    self.handleError('Element param has both start and value attributes')
                if 'name' in attrs:
                    if name not in self.variable_scopes[-1]:
                        self.variable_scopes[-1][name] = True
                    elif not self.VARPATTERN.match(name):
                        self.handleError('Generator parameter "%s" redefined' % (name, ))
            else:
                if 'value' not in attrs:
                    self.handleError('Element param missing attribute value')
                if ('increment' in attrs) or ('lshift' in attrs) or ('rshift' in attrs):
                    self.handleError('Element param has increment/lshift/rshift attribute(s) without start attribute')
                if 'name' in attrs:
                    if not self.variable_scopes[-1].get(name, False):
                        self.variable_scopes[-1][name] = False
                    elif not self.VARPATTERN.match(name):
                        self.handleError('Generator parameter "%s" redefined' % (name, ))
        else:
            if ('start' in attrs) or ('increment' in attrs) or ('lshift' in attrs) or ('rshift' in attrs):
                self.handleError('Element param with start/increment/lshift/rshift attribute(s) not in repeat scope')
            if 'value' not in attrs:
                self.handleError('Element param missing attribute value')
            if 'name' in attrs:
                self.variable_scopes[-1][attrs['name']] = False

    def checkBounds(self, attrs):
        if self.have_bounds[-1]:
            self.handleError('Duplicate element bounds')
        else:
            self.have_bounds[-1] = True
        left = self.checkFloatAttribute('bounds', attrs, 'left', 0.0)
        top = self.checkFloatAttribute('bounds', attrs, 'top', 0.0)
        right = self.checkFloatAttribute('bounds', attrs, 'right', 1.0)
        bottom = self.checkFloatAttribute('bounds', attrs, 'bottom', 1.0)
        x = self.checkFloatAttribute('bounds', attrs, 'x', 0.0)
        y = self.checkFloatAttribute('bounds', attrs, 'y', 0.0)
        width = self.checkFloatAttribute('bounds', attrs, 'width', 1.0)
        height = self.checkFloatAttribute('bounds', attrs, 'height', 1.0)
        if (left is not None) and (right is not None) and (left > right):
            self.handleError('Element bounds attribute left "%s" is greater than attribute right "%s"' % (
                    attrs.get('left', 0.0),
                    attrs.get('right', 1.0)))
        if (top is not None) and (bottom is not None) and (top > bottom):
            self.handleError('Element bounds attribute top "%s" is greater than attribute bottom "%s"' % (
                    attrs.get('top', 0.0),
                    attrs.get('bottom', 1.0)))
        if (width is not None) and (0.0 > width):
            self.handleError('Element bounds attribute width "%s" is negative' % (attrs['width'], ))
        if (height is not None) and (0.0 > height):
            self.handleError('Element bounds attribute height "%s" is negative' % (attrs['height'], ))
        if ('left' not in attrs) and ('x' not in attrs):
            self.handleError('Element bounds has neither attribute left nor attribute x')
        has_ltrb = ('left' in attrs) or ('top' in attrs) or ('right' in attrs) or ('bottom' in attrs)
        has_origin_size = ('x' in attrs) or ('y' in attrs) or ('width' in attrs) or ('height' in attrs)
        if has_ltrb and has_origin_size:
            self.handleError('Element bounds has both left/top/right/bottom and origin/size attributes')

    def checkOrientation(self, attrs):
        if self.have_orientation[-1]:
            self.handleError('Duplicate element orientation')
        else:
            self.have_orientation[-1] = True
        if self.checkIntAttribute('orientation', attrs, 'rotate', 0) not in self.ORIENTATIONS:
            self.handleError('Element orientation attribute rotate "%s" is unsupported' % (attrs['rotate'], ))
        for name in ('swapxy', 'flipx', 'flipy'):
            if attrs.get(name, 'no') not in self.YESNO:
                self.handleError('Element orientation attribute %s "%s" is not "yes" or "no"' % (name, attrs[name]))

    def checkColorChannel(self, attrs, name):
        channel = self.checkFloatAttribute('color', attrs, name, None)
        if (channel is not None) and ((0.0 > channel) or (1.0 < channel)):
            self.handleError('Element color attribute %s "%s" outside valid range 0.0-1.0' % (name, attrs[name]))

    def checkTag(self, tag, element, attr):
        if '' == tag:
            self.handleError('Element %s attribute %s is empty', (element, attr))
        else:
            if tag.find('^') >= 0:
                self.handleError('Element %s attribute %s "%s" contains parent device reference' % (element, attr, tag))
            if ':' == tag[-1]:
                self.handleError('Element %s attribute %s "%s" ends with separator' % (element, attr, tag))
            if tag.find('::') >= 0:
                self.handleError('Element %s attribute %s "%s" contains double separator' % (element, attr, tag))

    def checkComponent(self, name, attrs):
        state = self.checkIntAttribute(name, attrs, 'state', None)
        if (state is not None) and (0 > state):
            self.handleError('Element %s attribute state "%s" is negative' % (name, attrs['state']))
        self.handlers.append((self.componentStartHandler, self.componentEndHandler))
        self.have_bounds.append(False)
        self.have_color.append(False)

    def rootStartHandler(self, name, attrs):
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
            self.variable_scopes.append({ })
            self.repeat_depth.append(0)
            self.handlers.append((self.layoutStartHandler, self.layoutEndHandler))

    def rootEndHandler(self, name, attrs):
        pass # should be unreachable

    def layoutStartHandler(self, name, attrs):
        if 'element' == name:
            if 'name' not in attrs:
                self.handleError('Element element missing attribute name')
            else:
                generated_name = self.VARPATTERN.match(attrs['name'])
                if generated_name:
                    self.generated_element_names = True
                if attrs['name'] not in self.elements:
                    self.elements[attrs['name']] = self.formatLocation()
                elif not generated_name:
                    self.handleError('Element element has duplicate name (previous %s)' % (self.elements[attrs['name']], ))
            defstate = self.checkIntAttribute(name, attrs, 'defstate', None)
            if (defstate is not None) and (0 > defstate):
                self.handleError('Element element attribute defstate "%s" is negative' % (attrs['defstate'], ))
            self.handlers.append((self.elementStartHandler, self.elementEndHandler))
        elif 'group' == name:
            if 'name' not in attrs:
                self.handleError('Element group missing attribute name')
            else:
                generated_name = self.VARPATTERN.match(attrs['name'])
                if generated_name:
                    self.generated_group_names = True
                if attrs['name'] not in self.groups:
                    self.groups[attrs['name']] = self.formatLocation()
                elif not generated_name:
                    self.handleError('Element group has duplicate name (previous %s)' % (self.groups[attrs['name']], ))
            self.handlers.append((self.groupViewStartHandler, self.groupViewEndHandler))
            self.variable_scopes.append({ })
            self.repeat_depth.append(0)
            self.have_bounds.append(False)
        elif ('view' == name) and (not self.repeat_depth[-1]):
            if 'name' not in attrs:
                self.handleError('Element view missing attribute name')
            else:
                if attrs['name'] not in self.views:
                    self.views[attrs['name']] = self.formatLocation()
                elif not self.VARPATTERN.match(attrs['name']):
                    self.handleError('Element view has duplicate name (previous %s)' % (self.views[attrs['name']], ))
            self.handlers.append((self.groupViewStartHandler, self.groupViewEndHandler))
            self.variable_scopes.append({ })
            self.repeat_depth.append(0)
            self.have_bounds.append(False)
        elif 'repeat' == name:
            if 'count' not in attrs:
                self.handleError('Element repeat missing attribute count')
            else:
                count = self.checkIntAttribute(name, attrs, 'count', None)
                if (count is not None) and (0 >= count):
                    self.handleError('Element repeat attribute count "%s" is not positive' % (attrs['count'], ))
            self.variable_scopes.append({ })
            self.repeat_depth[-1] += 1
        elif 'param' == name:
            self.checkParameter(attrs)
            self.ignored_depth = 1
        elif ('script' == name) and (not self.repeat_depth[-1]):
            self.ignored_depth = 1
        else:
            self.handleError('Encountered unexpected element %s' % (name, ))
            self.ignored_depth = 1

    def layoutEndHandler(self, name):
        self.variable_scopes.pop()
        if self.repeat_depth[-1]:
            self.repeat_depth[-1] -= 1
        else:
            if not self.generated_element_names:
                for element in self.referenced_elements:
                    if (element not in self.elements) and (not self.VARPATTERN.match(element)):
                        self.handleError('Element "%s" not found (first referenced at %s)' % (element, self.referenced_elements[element]))
            if not self.generated_group_names:
                for group in self.referenced_groups:
                    if (group not in self.groups) and (not self.VARPATTERN.match(group)):
                        self.handleError('Group "%s" not found (first referenced at %s)' % (group, self.referenced_groups[group]))
            if not self.views:
                self.handleError('No view elements found')
            self.handlers.pop()

    def elementStartHandler(self, name, attrs):
        if name in self.SHAPES:
            self.checkComponent(name, attrs)
        elif 'text' == name:
            if 'string' not in attrs:
                self.handleError('Element text missing attribute string')
            align = self.checkIntAttribute(name, attrs, 'align', None)
            if (align is not None) and ((0 > align) or (2 < align)):
                self.handleError('Element text attribute align "%s" not in valid range 0-2' % (attrs['align'], ))
            self.checkComponent(name, attrs)
        elif 'simplecounter' == name:
            maxstate = self.checkIntAttribute(name, attrs, 'maxstate', None)
            if (maxstate is not None) and (0 > maxstate):
                self.handleError('Element simplecounter attribute maxstate "%s" is negative' % (attrs['maxstate'], ))
            digits = self.checkIntAttribute(name, attrs, 'digits', None)
            if (digits is not None) and (0 >= digits):
                self.handleError('Element simplecounter attribute digits "%s" is not positive' % (attrs['digits'], ))
            align = self.checkIntAttribute(name, attrs, 'align', None)
            if (align is not None) and ((0 > align) or (2 < align)):
                self.handleError('Element simplecounter attribute align "%s" not in valid range 0-2' % (attrs['align'], ))
            self.checkComponent(name, attrs)
        elif 'image' == name:
            if 'file' not in attrs:
                self.handleError('Element image missing attribute file')
            self.checkComponent(name, attrs)
        elif 'reel' == name:
            # TODO: validate symbollist and improve validation of other attributes
            self.checkIntAttribute(name, attrs, 'stateoffset', None)
            numsymbolsvisible = self.checkIntAttribute(name, attrs, 'numsymbolsvisible', None)
            if (numsymbolsvisible is not None) and (0 >= numsymbolsvisible):
                self.handleError('Element reel attribute numsymbolsvisible "%s" not positive' % (attrs['numsymbolsvisible'], ))
            reelreversed = self.checkIntAttribute(name, attrs, 'reelreversed', None)
            if (reelreversed is not None) and ((0 > reelreversed) or (1 < reelreversed)):
                self.handleError('Element reel attribute reelreversed "%s" not in valid range 0-1' % (attrs['reelreversed'], ))
            beltreel = self.checkIntAttribute(name, attrs, 'beltreel', None)
            if (beltreel is not None) and ((0 > beltreel) or (1 < beltreel)):
                self.handleError('Element reel attribute beltreel "%s" not in valid range 0-1' % (attrs['beltreel'], ))
            self.checkComponent(name, attrs)
        else:
            self.handleError('Encountered unexpected element %s' % (name, ))
            self.ignored_depth = 1

    def elementEndHandler(self, name):
        self.handlers.pop()

    def componentStartHandler(self, name, attrs):
        if 'bounds' == name:
            self.checkBounds(attrs)
        elif 'color' == name:
            if self.have_color[-1]:
                self.handleError('Duplicate color element')
            else:
                self.have_color[-1] = True
            self.checkColorChannel(attrs, 'red')
            self.checkColorChannel(attrs, 'green')
            self.checkColorChannel(attrs, 'blue')
            self.checkColorChannel(attrs, 'alpha')
        self.ignored_depth = 1

    def componentEndHandler(self, name):
        self.have_bounds.pop()
        self.have_color.pop()
        self.handlers.pop()

    def groupViewStartHandler(self, name, attrs):
        if name in self.OBJECTS:
            if 'element' not in attrs:
                self.handleError('Element %s missing attribute element' % (name, ))
            elif attrs['element'] not in self.referenced_elements:
                self.referenced_elements[attrs['element']] = self.formatLocation()
            if 'inputtag' in attrs:
                if 'inputmask' not in attrs:
                    self.handleError('Element %s has inputtag attribute without inputmask attribute' % (name, ))
                self.checkTag(attrs['inputtag'], name, 'inputtag')
            elif 'inputmask' in attrs:
                self.handleError('Element %s has inputmask attribute without inputtag attribute' % (name, ))
            inputmask = self.checkIntAttribute(name, attrs, 'inputmask', None)
            if (inputmask is not None) and (0 == inputmask):
                self.handleError('Element %s has attribute inputmask "%s" is zero' % (name, attrs['inputmask']))
            inputraw = self.checkIntAttribute(name, attrs, 'inputraw', None)
            if (inputraw is not None):
                if 'inputmask' not in attrs:
                    self.handleError('Element %s has inputraw attribute without inputmask attribute' % (name, ))
                if 'inputtag' not in attrs:
                    self.handleError('Element %s has inputraw attribute without inputtag attribute' % (name, ))
                if ((0 > inputraw) or (1 < inputraw)):
                    self.handleError('Element %s attribute inputraw "%s" not in valid range 0-1' % (name, attrs['inputraw']))
            self.handlers.append((self.objectStartHandler, self.objectEndHandler))
            self.have_bounds.append(False)
            self.have_orientation.append(False)
        elif 'screen' == name:
            if 'index' in attrs:
                index = self.checkIntAttribute(name, attrs, 'index', None)
                if (index is not None) and (0 > index):
                    self.handleError('Element screen attribute index "%s" is negative' % (attrs['index'], ))
                if 'tag' in attrs:
                    self.handleError('Element screen has both index and tag attributes')
            if 'tag' in attrs:
                tag = attrs['tag']
                self.checkTag(tag, name, 'tag')
                if self.BADTAGPATTERN.search(tag):
                    self.handleError('Element screen attribute tag "%s" contains invalid characters' % (tag, ))
            self.handlers.append((self.objectStartHandler, self.objectEndHandler))
            self.have_bounds.append(False)
            self.have_orientation.append(False)
        elif 'group' == name:
            if 'ref' not in attrs:
                self.handleError('Element group missing attribute ref')
            elif attrs['ref'] not in self.referenced_groups:
                self.referenced_groups[attrs['ref']] = self.formatLocation()
            self.handlers.append((self.objectStartHandler, self.objectEndHandler))
            self.have_bounds.append(False)
            self.have_orientation.append(False)
        elif 'repeat' == name:
            if 'count' not in attrs:
                self.handleError('Element repeat missing attribute count')
            else:
                count = self.checkIntAttribute(name, attrs, 'count', None)
                if (count is not None) and (0 >= count):
                    self.handleError('Element repeat attribute count "%s" is negative' % (attrs['count'], ))
            self.variable_scopes.append({ })
            self.repeat_depth[-1] += 1
        elif 'param' == name:
            self.checkParameter(attrs)
            self.ignored_depth = 1
        elif 'bounds' == name:
            self.checkBounds(attrs)
            if self.repeat_depth[-1]:
                self.handleError('Element bounds inside repeat')
            self.ignored_depth = 1
        else:
            self.handleError('Encountered unexpected element %s' % (name, ))
            self.ignored_depth = 1

    def groupViewEndHandler(self, name):
        self.variable_scopes.pop()
        if self.repeat_depth[-1]:
            self.repeat_depth[-1] -= 1
        else:
            self.repeat_depth.pop()
            self.have_bounds.pop()
            self.handlers.pop()

    def objectStartHandler(self, name, attrs):
        if 'bounds' == name:
            self.checkBounds(attrs)
        elif 'orientation' == name:
            self.checkOrientation(attrs)
        self.ignored_depth = 1

    def objectEndHandler(self, name):
        self.have_bounds.pop()
        self.have_orientation.pop()
        self.handlers.pop()

    def setDocumentLocator(self, locator):
        self.locator = locator
        super(LayoutChecker, self).setDocumentLocator(locator)

    def startDocument(self):
        self.handlers = [(self.rootStartHandler, self.rootEndHandler)]
        self.ignored_depth = 0
        self.variable_scopes = [ ]
        self.repeat_depth = [ ]
        self.have_bounds = [ ]
        self.have_orientation = [ ]
        self.have_color = [ ]
        self.generated_element_names = False
        self.generated_group_names = False
        super(LayoutChecker, self).startDocument()

    def endDocument(self):
        self.locator = None
        self.elements.clear()
        self.groups.clear()
        self.views.clear()
        self.referenced_elements.clear()
        self.referenced_groups.clear()
        del self.handlers
        del self.ignored_depth
        del self.variable_scopes
        del self.repeat_depth
        del self.have_bounds
        del self.have_orientation
        del self.have_color
        del self.generated_element_names
        del self.generated_group_names
        super(LayoutChecker, self).endDocument()

    def startElement(self, name, attrs):
        if 0 < self.ignored_depth:
            self.ignored_depth += 1
        else:
            self.handlers[-1][0](name, attrs)
        super(LayoutChecker, self).startElement(name, attrs)

    def endElement(self, name):
        if 0 < self.ignored_depth:
            self.ignored_depth -= 1
        else:
            self.handlers[-1][1](name)
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


class BlackHole(object):
    def write(self, *args):
        pass
    def close(self):
        pass


if __name__ == '__main__':
    if (len(sys.argv) > 4) or (len(sys.argv) < 2):
        print('Usage:')
        print('  complay <source.lay> [<output.h> [<varname>]]')
        sys.exit(0 if len(sys.argv) <= 1 else 1)

    srcfile = sys.argv[1]
    dstfile = sys.argv[2] if len(sys.argv) >= 3 else None
    if len(sys.argv) >= 4:
        varname = sys.argv[3]
    else:
        varname = os.path.basename(srcfile)
        base, ext = os.path.splitext(varname)
        if ext.lower() == '.lay':
            varname = base
        varname = 'layout_' + re.sub('[^0-9A-Za-z_]', '_', varname)

    comp_type = 1
    try:
        dst = open(dstfile,'w') if dstfile is not None else BlackHole()
        dst.write('static const unsigned char %s_data[] = {\n' % (varname))
        byte_count, comp_size = compressLayout(srcfile, lambda x: dst.write(x), zlib.compressobj())
        dst.write('};\n\n')
        dst.write('const internal_layout %s = {\n' % (varname))
        dst.write('\t%d, sizeof(%s_data), %d, %s_data\n' % (byte_count, varname, comp_type, varname))
        dst.write('};\n')
        dst.close()
    except XmlError:
        dst.close()
        if dstfile is not None:
            os.remove(dstfile)
        sys.exit(2)
    except IOError:
        sys.stderr.write("Unable to open output file '%s'\n" % dstfile)
        dst.close()
        if dstfile is not None:
            os.remove(dstfile)
        sys.exit(3)
