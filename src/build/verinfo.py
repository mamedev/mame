#!/usr/bin/python

from __future__ import with_statement

import sys
import os

def usage():
    sys.stderr.write('Usage: verinfo.py [-b mame|mess|ume] <filename>\n')
    return 0
	
build = "mame"

if (len(sys.argv)==1):
   usage()
   sys.exit(1)

if (sys.argv[1]=='-b'):
    if (sys.argv[2]=='mame'):
        build = "mame"
    elif (sys.argv[2]=='mess'):
        build = "mess"
    elif (sys.argv[2]=='ume'):
        build = "ume"
    else :
		usage()
		sys.exit(1)

srcfile = sys.argv[len(sys.argv)-1]
try:
	fp = open(srcfile, 'rb')
except IOError:
	sys.stderr.write("Unable to open source file '%s'" % srcfile)
	sys.exit(1)

for line in fp.readlines():
    if line.find("BARE_BUILD_VERSION")!=-1 and line.find('"')!=-1 and line.find('.')!=-1:
        version_string = line[line.find('"')+1:]
        version_string = version_string[0:version_string.find('"')]
        break

version_major = version_string[0:version_string.find('.')]
version_minor = version_string[version_string.find('.')+1:]
version_build = "0"
version_subbuild = "0"
if (build == "mess") :
    # MESS
    author = "MESS Team";
    comments = "Multi Emulation Super System";
    company_name = "MESS Team";
    file_description = "Multi Emulation Super System";
    internal_name = "MESS";
    original_filename = "MESS";
    product_name = "MESS";
elif (build == "ume") :
    # UME
    author = "MAME and MESS Team"
    comments = "Universal Machine Emulator"
    company_name = "MAME and MESS Team"
    file_description = "Universal Machine Emulator"
    internal_name = "UME"
    original_filename = "UME"
    product_name = "UME"
else : 
    # MAME
    author = "Nicola Salmoria and the MAME Team"
    comments = "Multiple Arcade Machine Emulator"
    company_name = "MAME Team"
    file_description = "Multiple Arcade Machine Emulator"
    internal_name = "MAME"
    original_filename = "MAME"
    product_name = "MAME"

legal_copyright = "Copyright Nicola Salmoria and the MAME team"

print("VS_VERSION_INFO VERSIONINFO")
print("\tFILEVERSION %s,%s,%s,%s" % (version_major, version_minor, version_build, version_subbuild))
print("\tPRODUCTVERSION %s,%s,%s,%s" % (version_major, version_minor, version_build, version_subbuild))
print("\tFILEFLAGSMASK 0x3fL")
if (version_build == 0) :
    print("\tFILEFLAGS 0x0L")
else :
    print("\tFILEFLAGS VS_FF_PRERELEASE")
print("\tFILEOS VOS_NT_WINDOWS32")
print("\tFILETYPE VFT_APP")
print("\tFILESUBTYPE VFT2_UNKNOWN")
print("BEGIN")
print("\tBLOCK \"StringFileInfo\"")
print("\tBEGIN")
print("#ifdef UNICODE")
print("\t\tBLOCK \"040904b0\"")
print("#else")
print("\t\tBLOCK \"040904E4\"")
print("#endif")
print("\t\tBEGIN")
print("\t\t\tVALUE \"Author\", \"%s\\0\"" % author)
print("\t\t\tVALUE \"Comments\", \"%s\\0\"" % comments)
print("\t\t\tVALUE \"CompanyName\", \"%s\\0\"" % company_name)
print("\t\t\tVALUE \"FileDescription\", \"%s\\0\"" % file_description)
print("\t\t\tVALUE \"FileVersion\", \"%s, %s, %s, %s\\0\"" % (version_major, version_minor, version_build, version_subbuild))
print("\t\t\tVALUE \"InternalName\", \"%s\\0\"" % internal_name)
print("\t\t\tVALUE \"LegalCopyright\", \"%s\\0\"" % legal_copyright)
print("\t\t\tVALUE \"OriginalFilename\", \"%s\\0\"" % original_filename)
print("\t\t\tVALUE \"ProductName\", \"%s\\0\"" % product_name)
print("\t\t\tVALUE \"ProductVersion\", \"%s\\0\"" % version_string)
print("\t\tEND")
print("\tEND")
print("\tBLOCK \"VarFileInfo\"")
print("\tBEGIN")
print("#ifdef UNICODE")
print("\t\tVALUE \"Translation\", 0x409, 1200")
print("#else")
print("\t\tVALUE \"Translation\", 0x409, 1252")
print("#endif")
print("\tEND")
print("END")