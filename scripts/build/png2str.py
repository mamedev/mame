#!/usr/bin/python
## license:BSD-3-Clause
## copyright-holders:Maurizio Petrarota
import os
import png
import sys


def main():
    if len(sys.argv) < 3:
        sys.stderr.write("Usage:\n%s <input.png> [<input2.png> [...]] <output.ipp>\n" % sys.argv[0])
        return 1
 
    headerName = sys.argv[-1]
    text = "// license:BSD-3-Clause\n" \
           "// copyright-holders:Maurizio Petrarota\n" \
           "#ifndef MAME_FRONTEND_UI_TOOLBAR_IPP\n" \
           "#define MAME_FRONTEND_UI_TOOLBAR_IPP\n" \
           "#pragma once\n\n" \
           "namespace ui {\n" \
           "namespace {\n\n" \
           "uint32_t const toolbar_bitmap_bmp[][1024] = {\n"

    files = len(sys.argv)-1

    for i in range(1, files):
        path = sys.argv[i]
        if not os.path.exists(path):
            sys.stderr.write("Error attempting to open PNG file.\n")
            return 1

        filename = os.path.basename(path)

        pngObject = png.Reader(path)
        try:
            pngObject.validate_signature()
        except:
            sys.stderr.write("Error reading PNG file.\n")
            return 1
        
        text += "\t{"
        width = pngObject.asRGBA()[0]
        height = pngObject.asRGBA()[1]
        rowGenerator = pngObject.asRGBA()[2]
        for row in rowGenerator:
            text += '\n\t\t'
            irpd = iter(row)
            for r,g,b,a in zip(irpd, irpd, irpd, irpd):
                text += "0x%08x, " % (a << 24 | r << 16 | g << 8 | b)
 
	    # Now conclude the C source
        text += "\n\t}"
        if i < files - 1:
            text += ","

        text += "\n"

    text += "};\n\n"
    text += "#define UI_TOOLBAR_BUTTONS    (ARRAY_LENGTH(toolbar_bitmap_bmp))\n\n"
    text += "} // anonymous namespace\n"
    text += "} // namespace ui\n\n"
    text += "#endif // MAME_FRONTEND_UI_TOOLBAR_IPP"

    open(headerName, 'w').write(text)
    return 0

########################################
## Program entry point
########################################
if __name__ == "__main__":
    sys.exit(main())
