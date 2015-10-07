#!/usr/bin/env python 
##
## license:BSD-3-Clause
## copyright-holders:Aaron Giles, Andrew Gardner
## ****************************************************************************
##
##    png2bdc.c
##
##    Super-simple PNG to BDC file generator
##
## ****************************************************************************
##
##    Format of PNG data:
##
##    Multiple rows of characters. A black pixel means "on". All other colors
##    mean "off". Each row looks like this:
##
##    * 8888  ***    *
##    * 4444 *   *  **
##    * 2222 *   *   *
##    * 1111 *   *   *
##    *      *   *   *
##    **      ***   ***
##    *
##    *
##
##           ****** ****
##
##    The column of pixels on the left-hand side (column 0) indicates the
##    character cell height. This column must be present on each row and
##    the height must be consistent for each row.
##
##    Protruding one pixel into column 1 is the baseline indicator. There
##    should only be one row with a pixel in column 1 for each line, and
##    that pixel row index should be consistent for each row.
##
##    In columns 2-5 are a 4-hex-digit starting character index number. This
##    is encoded as binary value. Each column is 4 pixels tall and represents
##    one binary digit. The character index number is the unicode character
##    number of the first character encoded in this row; subsequent
##    characters in the row are at increasing character indices.
##
##    Starting in column 6 and beyond are the actual character bitmaps.
##    Below them, in the second row after the last row of the character,
##    is a solid line that indicates the width of the character, and also
##    where the character bitmap begins and ends.
##
## ***************************************************************************

##
## Python note:
##   This is a near-literal translation of the original C++ code.  As such there
##   are some very non-pythonic things done throughout.  The conversion was done 
##   this way so as to insure compatibility as much as possible given the small
##   number of test cases.
##

import os
import png
import sys

if sys.version_info >= (3,):
    def b2p(v):
        return bytes([v])
else:
    def b2p(v):
        return chr(v)


########################################
## Helper classes
########################################
class RenderFontChar:
    """
    Contains information about a single character in a font.
    """
    
    def __init__(self):
        """
        """
        self.width = 0          # width from this character to the next
        self.xOffs = 0          # X offset from baseline to top,left of bitmap
        self.yOffs = 0          # Y offset from baseline to top,left of bitmap
        self.bmWidth = 0        # width of bitmap
        self.bmHeight = 0       # height of bitmap
        self.bitmap = None      # pointer to the bitmap containing the raw data


class RenderFont:
    """
    Contains information about a font
    """
    
    def __init__(self):
        self.height = 0         # height of the font, from ascent to descent
        self.yOffs = 0          # y offset from baseline to descent
        self.chars = list()     # array of characters
        for i in range(0, 65536):
            self.chars.append(RenderFontChar())



########################################
## Helper functions
########################################
def pixelIsSet(value):
    return (value & 0xffffff) == 0


def renderFontSaveCached(font, filename, hash32):
    """
    """
    fp = open(filename, "wb")
    if not fp:
        return 1

    # Write the header
    numChars = 0
    for c in font.chars:
        if c.width > 0:
            numChars += 1

    CACHED_CHAR_SIZE = 12
    CACHED_HEADER_SIZE = 16
    
    try:
        fp.write(b'f')
        fp.write(b'o')
        fp.write(b'n')
        fp.write(b't')
        fp.write(b2p(hash32 >> 24 & 0xff))
        fp.write(b2p(hash32 >> 16 & 0xff))
        fp.write(b2p(hash32 >> 8 & 0xff))
        fp.write(b2p(hash32 >> 0 & 0xff))
        fp.write(b2p(font.height >> 8 & 0xff))
        fp.write(b2p(font.height >> 0 & 0xff))
        fp.write(b2p(font.yOffs >> 8 & 0xff))
        fp.write(b2p(font.yOffs >> 0 & 0xff))
        fp.write(b2p(numChars >> 24 & 0xff))
        fp.write(b2p(numChars >> 16 & 0xff))
        fp.write(b2p(numChars >> 8 & 0xff))
        fp.write(b2p(numChars >> 0 & 0xff))
        
        # Write a blank table at first (?)
        charTable = [0]*(numChars * CACHED_CHAR_SIZE)
        for i in range(numChars * CACHED_CHAR_SIZE):
            fp.write(b2p(charTable[i]))
        
        # Loop over all characters
        tableIndex = 0
        
        for i in range(len(font.chars)):
            c = font.chars[i]
            if c.width == 0:
                continue
            
            if c.bitmap:
                dBuffer = list()
                accum = 0
                accbit = 7
    
                # Bit-encode the character data
                for y in range(0, c.bmHeight):
                    src = None
                    desty = y + font.height + font.yOffs - c.yOffs - c.bmHeight
                    if desty >= 0 and desty < font.height:
                        src = c.bitmap[desty]
                    for x in range(0, c.bmWidth):
                        if src is not None and src[x] != 0:
                            accum |= 1 << accbit
                        accbit -= 1
                        if accbit+1 == 0:
                            dBuffer.append(accum)
                            accum = 0
                            accbit = 7
                
                # Flush any extra
                if accbit != 7:
                    dBuffer.append(accum)
                
                # Write the data
                for j in range(len(dBuffer)):
                    fp.write(b2p(dBuffer[j]))
            
            destIndex = tableIndex * CACHED_CHAR_SIZE
            charTable[destIndex +  0] = i >> 8 & 0xff
            charTable[destIndex +  1] = i >> 0 & 0xff
            charTable[destIndex +  2] = c.width >> 8 & 0xff
            charTable[destIndex +  3] = c.width >> 0 & 0xff
            charTable[destIndex +  4] = c.xOffs >> 8 & 0xff
            charTable[destIndex +  5] = c.xOffs >> 0 & 0xff
            charTable[destIndex +  6] = c.yOffs >> 8 & 0xff
            charTable[destIndex +  7] = c.yOffs >> 0 & 0xff
            charTable[destIndex +  8] = c.bmWidth >> 8 & 0xff
            charTable[destIndex +  9] = c.bmWidth >> 0 & 0xff
            charTable[destIndex + 10] = c.bmHeight >> 8 & 0xff
            charTable[destIndex + 11] = c.bmHeight >> 0 & 0xff
            tableIndex += 1
    
        # Seek back to the beginning and rewrite the table
        fp.seek(CACHED_HEADER_SIZE, 0)
        for i in range(numChars * CACHED_CHAR_SIZE):
            fp.write(b2p(charTable[i]))
    
        fp.close()
        return 0

    except:
        print(sys.exc_info[1])
        return 1
    

def bitmapToChars(pngObject, font):
    """
    Convert a bitmap to characters in the given font
    """
    # Just cache the bitmap into a list of lists since random access is paramount
    bitmap = list()
    width = pngObject.asRGBA8()[0]
    height = pngObject.asRGBA8()[1]
    rowGenerator = pngObject.asRGBA8()[2]
    for row in rowGenerator:
        cRow = list()
        irpd = iter(row)
        for r,g,b,a in zip(irpd, irpd, irpd, irpd):
            cRow.append(a << 24 | r << 16 | g << 8 | b)
        bitmap.append(cRow)
    
    rowStart = 0
    while rowStart < height:
        # Find the top of the row
        for i in range(rowStart, height):
            if pixelIsSet(bitmap[rowStart][0]):
                break
            rowStart += 1
        if rowStart >= height:
            break

        # Find the bottom of the row
        rowEnd = rowStart + 1
        for i in range(rowEnd, height):
            if not pixelIsSet(bitmap[rowEnd][0]):
                rowEnd -= 1
                break
            rowEnd += 1

        # Find the baseline
        baseline = rowStart
        for i in range(rowStart, rowEnd+1):
            if pixelIsSet(bitmap[baseline][1]):
                break
            baseline += 1
        if baseline > rowEnd:
            sys.stderr.write("No baseline found between rows %d-%d\n" % (rowStart, rowEnd))
            break

        # Set or confirm the height
        if font.height == 0:
            font.height = rowEnd - rowStart + 1
            font.yOffs = baseline - rowEnd
        else:
            if font.height != (rowEnd - rowStart + 1):
                sys.stderr.write("Inconsistent font height at rows %d-%d\n" % (rowStart, rowEnd))
                break
            if font.yOffs != (baseline - rowEnd):
                sys.stderr.write("Inconsistent baseline at rows %d-%d\n" % (rowStart, rowEnd))
                break

        # decode the starting character
        chStart = 0
        for x in range(0, 4):
            for y in range(0, 4):
                chStart = (chStart << 1) | pixelIsSet(bitmap[rowStart+y][2+x])

        # Print debug info
        # print("Row %d-%d, baseline %d, character start %X" % (rowStart, rowEnd, baseline, chStart))

        # scan the column to find characters
        colStart = 0
        while colStart < width:
            ch = RenderFontChar()

            # Find the start of the character
            for i in range(colStart, width):
                if pixelIsSet(bitmap[rowEnd+2][colStart]):
                    break
                colStart += 1
            if colStart >= width:
                break

            # Find the end of the character
            colEnd = colStart + 1
            for i in range(colEnd, width):
                if not pixelIsSet(bitmap[rowEnd+2][colEnd]):
                    colEnd -= 1
                    break
                colEnd += 1

            # Skip char which code is already registered
            if ch.width <= 0:
                # Print debug info
                # print "  Character %X - width = %d" % (chStart, colEnd - colStart + 1)

                # Plot the character
                ch.bitmap = list()
                for y in range(rowStart, rowEnd+1):
                    ch.bitmap.append(list())
                    for x in range(colStart, colEnd+1):
                        if pixelIsSet(bitmap[y][x]):
                            ch.bitmap[-1].append(0xffffffff)
                        else:
                            ch.bitmap[-1].append(0x00000000)

                # Set the character parameters
                ch.width = colEnd - colStart + 1
                ch.xOffs = 0
                ch.yOffs = font.yOffs
                ch.bmWidth = len(ch.bitmap[0])
                ch.bmHeight = len(ch.bitmap)
                
                # Insert the character into the list
                font.chars[chStart] = ch

            # Next character
            chStart += 1
            colStart = colEnd + 1

        # Next row
        rowStart = rowEnd + 1
    
    # Return non-zero if we errored
    return rowStart < height



########################################
## Main
########################################
def main():
    if len(sys.argv) < 3:
        sys.stderr.write("Usage:\n%s <input.png> [<input2.png> [...]] <output.bdc>\n" % sys.argv[0])
        return 1
    bdcName = sys.argv[-1]
    
    font = RenderFont()
    for i in range(1, len(sys.argv)-1):
        filename = sys.argv[i]
    if not os.path.exists(filename):
        sys.stderr.write("Error attempting to open PNG file.\n")
        return 1

    pngObject = png.Reader(filename)
    try:
        pngObject.validate_signature()
    except:
        sys.stderr.write("Error reading PNG file.\n")
        return 1

    error = bitmapToChars(pngObject, font)
    if error:
        return 1
    
    error = renderFontSaveCached(font, bdcName, 0)
    return error
    
    
    
########################################
## Program entry point
########################################
if __name__ == "__main__":
    sys.exit(main())
