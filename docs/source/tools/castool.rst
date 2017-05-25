Castool - *A generic casette image manipulation tool for MAME*
==============================================================



Castool is a tool for the maintenance and manipulation of cassette images that MAME users need to deal with. MAME directly supports .WAV audio formatted images, but many of the existing images out there may come in forms such as .TAP for Commodore 64 tapes, .CAS for Tandy Color Computer tapes, and so forth. Castool will convert these other formats to .WAV for use in MAME.

Castool is part of the MAME project. It shares large portions of code with MAME, and its existence would not be if it were not for MAME.  As such, the distribution terms are the same as MAME.  Please read the MAME license thoroughly. 


Using Castool
=============

Castool is a command line program that contains a simple set of instructions. Commands are invoked in a manner along the lines of this:

	**castool convert <format> <image> <output>**

* **<format>** is the format of the image
* **<image>** is the filename of the image you're converting from
* **<output>** is the filename of the output WAV file

Example usage:
	castool convert coco zaxxon.cas zaxxon.wav
	
	castool convert cbm arkanoid.tap arkanoid.wav
	
	castool convert ddp mybasicprogram.ddp mybasicprogram.wav




Castool Formats
===============

These are the formats supported by Castool for conversion to .WAV files.

**A26**

	Atari 2600 SuperCharger image
	
	File extension: a26
	
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	
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
	

**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

      kc85 - VEB Mikroelektronik KC 85
                kcc,kcb
                tap,853,854,855,tp2,kcm
                sss
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

      kim1 - MOS KIM-1
                kim,kim1
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

      lviv - PK-01 Lviv
                lvt,lvr,lv0,lv1,lv2,lv3
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       mo5 - Thomson MO-series
                k5,k7
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

        mz - Sharp MZ-700
                m12,mzf,mzt
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

      orao - PEL Varazdin Orao
                tap
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

      oric - Tangerine Oric
                tap
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

    pc6001 - NEC PC-6001
                cas
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

     phc25 - Sanyo PHC-25
                phc
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

     pmd85 - Tesla PMD-85
                pmd,tap,ptp
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

     primo - Microkey Primo
                ptp
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       rku - UT-88
                rku
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       rk8 - Mikro-80
                rk8
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       rks - Specialist
                rks
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       rko - Orion
                rko
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       rkr - Radio-86RK
                rk,rkr
                gam,g16,pki
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       rka - Zavod BRA Apogee BK-01
                rka
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       rkm - Mikrosha
                rkm
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       rkp - SAM SKB VM Partner-01.01
                rkp
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

    sc3000 - Sega SC-3000
                bit
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

     sol20 - PTC SOL-20
                svt
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

  sorcerer - Exidy Sorcerer
                tape
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

    sordm5 - Sord M5
                cas
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

   spc1000 - Samsung SPC-1000
                tap
                cas
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       svi - Spectravideo SVI-318 & SVI-328
                cas
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       to7 - Thomson TO-series
                k7
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

   trs80l2 - TRS-80 Level 2
                cas
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

     tvc64 - Videoton TVC 64
                cas
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       tzx - Sinclair ZX Spectrum
                tzx
                tap,blk
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       uef - Acorn Electron
                uef
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

      vg5k - Philips VG 5000
                k7
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

    vtech1 - Video Technology Laser 110-310
                cas
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

    vtech2 - Video Technology Laser 350-700
                cas
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

       x07 - Canon X-07
                k7,lst,cas
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

        x1 - Sharp X1
                tap
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

    zx80_o - Sinclair ZX80
                o,80
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	

    zx81_p - Sinclair ZX81
                p,81
				
**APF**

	APF Imagination Machine
	
	File extensions: cas, cpf, apt
	
