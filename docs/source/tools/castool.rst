Castool - *A generic cassette image manipulation tool for MAME*
===============================================================



Castool is a tool for the maintenance and manipulation of cassette images that MAME users need to deal with. MAME directly supports .WAV audio formatted images, but many of the existing images out there may come in forms such as .TAP for Commodore 64 tapes, .CAS for Tandy Color Computer tapes, and so forth. Castool will convert these other formats to .WAV for use in MAME.

Castool is part of the MAME project. It shares large portions of code with MAME, and its existence would not be if it were not for MAME.  As such, the distribution terms are the same as MAME.  Please read the MAME license thoroughly.


Using Castool
-------------

Castool is a command line program that contains a simple set of instructions. Commands are invoked in a manner along the lines of this:

	**castool convert <format> <inputfile> <outputfile>**

* **<format>** is the format of the image
* **<inputfile>** is the filename of the image you're converting from
* **<outputfile>** is the filename of the output WAV file

Example usage:
	castool convert coco zaxxon.cas zaxxon.wav

	castool convert cbm arkanoid.tap arkanoid.wav

	castool convert ddp mybasicprogram.ddp mybasicprogram.wav




Castool Formats
---------------

These are the formats supported by Castool for conversion to .WAV files.

**A26**

	Atari 2600 SuperCharger image

	File extension: a26

**APF**

	APF Imagination Machine

	File extensions: cas, cpf, apt

**ATOM**

	Acorn Atom

	File extensions: tap, csw, uef

**BBC**

	Acorn BBC & Electron

	File extensions: csw, uef

**CBM**

	Commodore 8-bit series

	File extensions: tap

**CDT**

	Amstrad CPC

	File extensions: cdt

**CGENIE**

	EACA Colour Genie

	File extensions: cas

**COCO**

	Tandy Radio Shack Color Computer

	File extensions: cas

**CSW**

	Compressed Square Wave

	File extensions: csw

**DDP**

	Coleco ADAM

	File extensions: ddp

**FM7**

	Fujitsu FM-7

	File extensions: t77

**FMSX**

	MSX

	File extensions: tap, cas

**GTP**

	Elektronika inzenjering Galaksija

	File extensions: gtp

**HECTOR**

	Micronique Hector & Interact Family Computer

	File extensions: k7, cin, for

**JUPITER**

	Jupiter Cantab Jupiter Ace

	File extensions: tap

**KC85**

	VEB Mikroelektronik KC 85

	File extensions: kcc, kcb, tap, 853, 854, 855, tp2, kcm, sss

**KIM1**

	MOS KIM-1

	File extensions: kim, kim1

**LVIV**

	PK-01 Lviv

	File extensions: lvt, lvr, lv0, lv1, lv2, lv3

**MO5**

	Thomson MO-series

	File extensions: k5, k7

**MZ**

	Sharp MZ-700

	File extensions: m12, mzf, mzt

**ORAO**

	PEL Varazdin Orao

	File extensions: tap

**ORIC**

	Tangerine Oric

	File extensions: tap

**PC6001**

	NEC PC-6001

	File extensions: cas

**PHC25**

	Sanyo PHC-25

	File extensions: phc

**PMD85**

	Tesla PMD-85

	File extensions: pmd, tap, ptp

**PRIMO**

	Microkey Primo

	File extensions: ptp

**RKU**

	UT-88

	File extensions: rku

**RK8**

	Mikro-80

	File extensions: rk8

**RKS**

	Specialist

	File extensions: rks

**RKO**

	Orion

	File extensions: rko

**RKR**

	Radio-86RK

	File extensions: rk, rkr, gam, g16, pki

**RKA**

	Zavod BRA Apogee BK-01

	File extensions: rka

**RKM**

	Mikrosha

	File extensions: rkm

**RKP**

	SAM SKB VM Partner-01.01

	File extensions: rkp

**SC3000**

	Sega SC-3000

	File extensions: bit

**SOL20**

	PTC SOL-20

	File extensions: svt

**SORCERER**

	Exidy Sorcerer

	File extensions: tape

**SORDM5**

	Sord M5

	File extensions: cas

**SPC1000**

	Samsung SPC-1000

	File extensions: tap, cas

**SVI**

	Spectravideo SVI-318 & SVI-328

	File extensions: cas

**TO7**

	Thomson TO-series

	File extensions: k7

**TRS8012**

	TRS-80 Level 2

	File extensions: cas

**TVC64**

	Videoton TVC 64

	File extensions: cas

**TZX**

	Sinclair ZX Spectrum

	File extensions: tzx, tap, blk

**VG5K**

	Philips VG 5000

	File extensions: k7

**VTECH1**

	Video Technology Laser 110-310

	File extensions: cas

**VTECH2**

	Video Technology Laser 350-700

	File extensions: cas

**X07**

	Canon X-07

	File extensions: k7, lst, cas

**X1**

	Sharp X1

	File extensions: tap

**ZX80_O**

	Sinclair ZX80

	File extensions: o, 80

**ZX81_P**

	Sinclair ZX81

	File extensions: p, 81

