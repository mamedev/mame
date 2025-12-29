Floptool - *A generic floppy image manipulation tool for MAME*
==============================================================



Floptool is a tool for the maintenance and manipulation of floppy images that MAME users need to deal with. MAME directly supports .WAV audio formatted images, but many of the existing images out there may come in forms such as .TAP for Commodore 64 tapes, .CAS for Tandy Color Computer tapes, and so forth. Castool will convert these other formats to .WAV for use in MAME.

Floptool is part of the MAME project. It shares large portions of code with MAME, and its existence would not be if it were not for MAME.  As such, the distribution terms are the same as MAME.  Please read the MAME license thoroughly.


Using Floptool
--------------

Floptool is a command line program that contains a simple set of instructions. Commands are invoked in a manner along the lines of this:

	**floptool identify <inputfile> [<inputfile> ...]**
	**floptool convert [input_format|auto] output_format <inputfile> <outputile>**

* **<format>** is the format of the image
* **<input_format>** is the format of the inputfile, use auto if not known
* **<output_format>** is the format of the converted file
* **<inputfile>** is the filename of the image you're identifying/converting from
* **<outputfile>** is the filename of the converted file

Example usage:
	floptool convert coco zaxxon.cas zaxxon.wav

	floptool convert cbm arkanoid.tap arkanoid.wav

	floptool convert ddp mybasicprogram.ddp mybasicprogram.wav




Floptool Formats
----------------

These are the formats supported by Floptool for conversion to other formats.

**MFI**

	MAME floppy image

	File extension: mfi

**DFI**

	DiscFerret flux dump format

	File extensions: dfi

**IPF**

	SPS floppy disk image

	File extensions: ipf

**MFM**

	HxC Floppy Emulator floppy disk image

	File extensions: mfm

**ADF**

	Amiga ADF floppy disk image

	File extensions: adf

**ST**

	Atari ST floppy disk image

	File extensions: st

**MSA**

	Atari MSA floppy disk image

	File extensions: msa

**PASTI**

	Atari PASTI floppy disk image

	File extensions: stx

**DSK**

	CPC DSK format

	File extensions: dsk

**D88**

	D88 disk image

	File extensions: d77, d88, 1dd

**IMD**

	IMD disk image

	File extensions: imd

**TD0**

	Teledisk disk image

	File extensions: td0

**CQM**

	CopyQM disk image

	File extensions: cqm, cqi, dsk

**PC**

	PC floppy disk image

	File extensions: dsk, ima, img, ufi, 360

**NASLITE**

	NASLite disk image

	File extensions: img

**DC42**

	DiskCopy 4.2 image

	File extensions: dc42

**A2_16SECT**

	Apple II 16-sector disk image

	File extensions: dsk, do, po

**A2_EDD**

	Apple II EDD image

	File extensions: edd

**ATOM**

	Acorn Atom disk image

	File extensions: 40t, dsk

**SSD**

	Acorn SSD disk image

	File extensions: ssd, bbc, img

**DSD**

	Acorn DSD disk image

	File extensions: dsd

**DOS**

	Acorn DOS disk image

	File extensions: img

**ADFS_O**

	Acorn ADFS (OldMap) disk image

	File extensions: adf, ads, adm, adl

**ADFS_N**

	Acorn ADFS (NewMap) disk image

	File extensions: adf

**ORIC_DSK**

	Oric disk image

	File extensions: dsk

**APPLIX**

	Applix disk image

	File extensions: raw

**HPI**

	HP9845A floppy disk image

	File extensions: hpi
