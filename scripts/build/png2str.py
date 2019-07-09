#!/usr/bin/python
## license:BSD-3-Clause
## copyright-holders:Maurizio Petrarota
import os
import png
import sys
import glob

def main():
    if len(sys.argv) < 3:
        sys.stderr.write("Usage:\n%s <input.png> [<input2.png> [...]] <output.ipp>\n" % sys.argv[0])
        return 1        

    headerName = sys.argv[-1]

    text = "// license:BSD-3-Clause\n" \
           "// copyright-holders:Maurizio Petrarota\n"
    if len(sys.argv) > 3:
        text += "#ifndef MAME_FRONTEND_UI_TOOLBAR_IPP\n" \
           "#define MAME_FRONTEND_UI_TOOLBAR_IPP\n"
    else:
        text += "#ifndef MAME_FRONTEND_UI_%s_IPP\n" % os.path.basename(sys.argv[1]).upper()
        text += "#define MAME_FRONTEND_UI_%s_IPP\n" % os.path.basename(sys.argv[1]).upper()


    text += "#pragma once\n\n" \
            "namespace ui {\n" \
            "namespace {\n\n"

    if len(sys.argv) > 3:
        text += "uint32_t const toolbar_bitmap_bmp[][1024] = {\n"
    else:
        text += "uint32_t const %s_bmp[] = {\n" % os.path.splitext(os.path.basename(sys.argv[1]))[0]

    for i in range(1, len(sys.argv)-1):
        path = sys.argv[i]
        pngObject = png.Reader(path)
        try:
            pngObject.validate_signature()
        except:
            sys.stderr.write("Error reading PNG file.\n")
            return 1

        if len(sys.argv) > 3:
            text += "\t{"
        width = pngObject.asRGBA()[0]
        height = pngObject.asRGBA()[1]
        rowGenerator = pngObject.asRGBA()[2]
        for row in rowGenerator:
            text += '\n\t\t'
            irpd = iter(row)
            for r,g,b,a in zip(irpd, irpd, irpd, irpd):
                text += "0x%08X, " % (a << 24 | r << 16 | g << 8 | b)
 
	    # Now conclude the C source
	if len(sys.argv) > 3:
            text += "\n\t},\n"
        else:
            text += "\n"

    text += "};\n\n"
    if len(sys.argv) > 3:
        text += "#define UI_TOOLBAR_BUTTONS    (ARRAY_LENGTH(toolbar_bitmap_bmp))\n\n"
    text += "} // anonymous namespace\n" \
            "} // namespace ui\n\n"
    if len(sys.argv) > 3:
        text += "#endif // MAME_FRONTEND_UI_TOOLBAR_IPP"
    else:
        text += "#endif // MAME_FRONTEND_UI_%s_IPP" % os.path.basename(sys.argv[1]).upper()

    open(headerName, 'w').write(text)
    return 0

########################################
## Program entry point
########################################
if __name__ == "__main__":
    sys.exit(main())
