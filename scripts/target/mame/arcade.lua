-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   mame.lua
--
--   MAME target makefile
--
---------------------------------------------------------------------------

--------------------------------------------------
-- specify available CPU cores
---------------------------------------------------

CPUS["Z80"] = true
CPUS["Z180"] = true
CPUS["I8085"] = true
CPUS["I8089"] = true
CPUS["M6502"] = true
CPUS["H6280"] = true
CPUS["I86"] = true
CPUS["I386"] = true
CPUS["NEC"] = true
CPUS["V30MZ"] = true
CPUS["V60"] = true
CPUS["MCS48"] = true
CPUS["MCS51"] = true
CPUS["MCS96"] = true
CPUS["M6800"] = true
CPUS["M6805"] = true
CPUS["HD6309"] = true
CPUS["M6809"] = true
CPUS["KONAMI"] = true
CPUS["M680X0"] = true
CPUS["T11"] = true
CPUS["S2650"] = true
CPUS["TMS340X0"] = true
CPUS["TMS9900"] = true
CPUS["TMS9995"] = true
CPUS["TMS9900L"] = true
CPUS["Z8000"] = true
CPUS["Z8001"] = true
CPUS["TMS32010"] = true
CPUS["TMS32025"] = true
CPUS["TMS32031"] = true
CPUS["TMS32051"] = true
CPUS["TMS32082"] = true
CPUS["TMS57002"] = true
CPUS["CCPU"] = true
CPUS["ADSP21XX"] = true
CPUS["ASAP"] = true
CPUS["AM29000"] = true
CPUS["UPD7810"] = true
CPUS["ARM"] = true
CPUS["ARM7"] = true
CPUS["JAGUAR"] = true
CPUS["CUBEQCPU"] = true
CPUS["ESRIP"] = true
CPUS["MIPS"] = true
CPUS["PSX"] = true
CPUS["SH2"] = true
CPUS["SH4"] = true
CPUS["DSP16A"] = true
CPUS["DSP32C"] = true
CPUS["PIC16C5X"] = true
CPUS["PIC16C62X"] = true
CPUS["G65816"] = true
CPUS["SPC700"] = true
CPUS["E1"] = true
CPUS["I860"] = true
CPUS["I960"] = true
CPUS["H8"] = true
CPUS["V810"] = true
CPUS["M37710"] = true
CPUS["POWERPC"] = true
CPUS["SE3208"] = true
CPUS["MC68HC11"] = true
CPUS["ADSP21062"] = true
CPUS["DSP56156"] = true
CPUS["RSP"] = true
CPUS["ALPHA8201"] = true
CPUS["COP400"] = true
CPUS["TLCS90"] = true
CPUS["TLCS900"] = true
CPUS["MB88XX"] = true
CPUS["MB86233"] = true
CPUS["MB86235"] = true
CPUS["SSP1601"] = true
CPUS["APEXC"] = true
CPUS["CP1610"] = true
CPUS["F8"] = true
CPUS["LH5801"] = true
CPUS["PDP1"] = true
CPUS["SATURN"] = true
CPUS["SC61860"] = true
CPUS["LR35902"] = true
CPUS["TMS7000"] = true
CPUS["SM8500"] = true
CPUS["MINX"] = true
CPUS["SSEM"] = true
CPUS["AVR8"] = true
--CPUS["TMS0980"] = true
CPUS["I4004"] = true
CPUS["SUPERFX"] = true
CPUS["Z8"] = true
CPUS["I8008"] = true
CPUS["SCMP"] = true
CPUS["MN10200"] = true
CPUS["COSMAC"] = true
CPUS["UNSP"] = true
CPUS["HCD62121"] = true
CPUS["PPS4"] = true
CPUS["UPD7725"] = true
CPUS["HD61700"] = true
CPUS["LC8670"] = true
CPUS["SCORE"] = true
CPUS["ES5510"] = true
CPUS["SCUDSP"] = true
CPUS["IE15"] = true
CPUS["8X300"] = true
CPUS["ALTO2"] = true
--CPUS["W65816"] = true
CPUS["ARC"] = true
CPUS["ARCOMPACT"] = true
--CPUS["AMIS2000"] = true
--CPUS["UCOM4"] = true
CPUS["HMCS40"] = true
--CPUS["E0C6200"] = true
--CPUS["MELPS4"] = true
--CPUS["HPHYBRID"] = true
--CPUS["SM510"] = true

--------------------------------------------------
-- specify available sound cores
--------------------------------------------------

SOUNDS["SAMPLES"] = true
SOUNDS["DAC"] = true
SOUNDS["DMADAC"] = true
SOUNDS["SPEAKER"] = true
SOUNDS["BEEP"] = true
SOUNDS["DISCRETE"] = true
SOUNDS["AY8910"] = true
SOUNDS["YM2151"] = true
SOUNDS["YM2203"] = true
SOUNDS["YM2413"] = true
SOUNDS["YM2608"] = true
SOUNDS["YM2610"] = true
SOUNDS["YM2610B"] = true
SOUNDS["YM2612"] = true
SOUNDS["YM3438"] = true
SOUNDS["YM3812"] = true
SOUNDS["YM3526"] = true
SOUNDS["Y8950"] = true
SOUNDS["YMF262"] = true
SOUNDS["YMF271"] = true
SOUNDS["YMF278B"] = true
SOUNDS["YMZ280B"] = true
SOUNDS["SN76477"] = true
SOUNDS["SN76496"] = true
SOUNDS["POKEY"] = true
SOUNDS["TIA"] = true
SOUNDS["NES_APU"] = true
SOUNDS["AMIGA"] = true
SOUNDS["ASTROCADE"] = true
SOUNDS["NAMCO"] = true
SOUNDS["NAMCO_15XX"] = true
SOUNDS["NAMCO_CUS30"] = true
SOUNDS["NAMCO_52XX"] = true
SOUNDS["NAMCO_63701X"] = true
SOUNDS["T6W28"] = true
SOUNDS["SNKWAVE"] = true
SOUNDS["C140"] = true
SOUNDS["C352"] = true
SOUNDS["TMS36XX"] = true
SOUNDS["TMS3615"] = true
SOUNDS["TMS5110"] = true
SOUNDS["TMS5220"] = true
SOUNDS["VLM5030"] = true
SOUNDS["ADPCM"] = true
SOUNDS["MSM5205"] = true
SOUNDS["MSM5232"] = true
SOUNDS["OKIM6258"] = true
SOUNDS["OKIM6295"] = true
SOUNDS["OKIM6376"] = true
SOUNDS["OKIM9810"] = true
--SOUNDS["UPD7752"] = true
SOUNDS["UPD7759"] = true
SOUNDS["HC55516"] = true
SOUNDS["TC8830F"] = true
SOUNDS["K005289"] = true
SOUNDS["K007232"] = true
SOUNDS["K051649"] = true
SOUNDS["K053260"] = true
SOUNDS["K054539"] = true
SOUNDS["K056800"] = true
SOUNDS["SEGAPCM"] = true
SOUNDS["MULTIPCM"] = true
SOUNDS["SCSP"] = true
SOUNDS["AICA"] = true
SOUNDS["RF5C68"] = true
SOUNDS["RF5C400"] = true
SOUNDS["CEM3394"] = true
SOUNDS["QSOUND"] = true
SOUNDS["QS1000"] = true
SOUNDS["SAA1099"] = true
SOUNDS["IREMGA20"] = true
SOUNDS["ES5503"] = true
SOUNDS["ES5505"] = true
SOUNDS["ES5506"] = true
SOUNDS["BSMT2000"] = true
SOUNDS["GAELCO_CG1V"] = true
SOUNDS["GAELCO_GAE1"] = true
SOUNDS["C6280"] = true
SOUNDS["SP0250"] = true
SOUNDS["SPU"] = true
SOUNDS["CDDA"] = true
SOUNDS["ICS2115"] = true
SOUNDS["I5000_SND"] = true
SOUNDS["ST0016"] = true
SOUNDS["NILE"] = true
SOUNDS["X1_010"] = true
SOUNDS["VRENDER0"] = true
SOUNDS["VOTRAX"] = true
SOUNDS["ES8712"] = true
SOUNDS["CDP1869"] = true
SOUNDS["S14001A"] = true
SOUNDS["WAVE"] = true
SOUNDS["SID6581"] = true
SOUNDS["SID8580"] = true
SOUNDS["SP0256"] = true
SOUNDS["DIGITALKER"] = true
SOUNDS["CDP1863"] = true
SOUNDS["CDP1864"] = true
SOUNDS["ZSG2"] = true
SOUNDS["MOS656X"] = true
SOUNDS["ASC"] = true
SOUNDS["MAS3507D"] = true
SOUNDS["SOCRATES"] = true
SOUNDS["TMC0285"] = true
SOUNDS["TMS5200"] = true
SOUNDS["CD2801"] = true
SOUNDS["CD2802"] = true
SOUNDS["M58817"] = true
SOUNDS["TMC0281"] = true
SOUNDS["TMS5100"] = true
SOUNDS["TMS5110A"] = true
SOUNDS["LMC1992"] = true
SOUNDS["AWACS"] = true
SOUNDS["YMZ770"] = true
SOUNDS["T6721A"] = true
SOUNDS["MOS7360"] = true
--SOUNDS["ESQPUMP"] = true
--SOUNDS["VRC6"] = true
SOUNDS["SB0400"] = true
SOUNDS["AC97"] = true
SOUNDS["ES1373"] = true
SOUNDS["L7A1045"] = true

--------------------------------------------------
-- specify available video cores
--------------------------------------------------

VIDEOS["SEGA315_5124"] = true
VIDEOS["SEGA315_5313"] = true
VIDEOS["BUFSPRITE"] = true
--VIDEOS["CDP1861"] = true
--VIDEOS["CDP1862"] = true
--VIDEOS["CRT9007"] = true
--VIDEOS["CRT9021"] = true
--VIDEOS["CRT9212"] = true
--VIDEOS["CRTC_EGA"] = true
--VIDEOS["DL1416"] = true
VIDEOS["DM9368"] = true
--VIDEOS["EF9340_1"] = true
--VIDEOS["EF9345"] = true
--VIDEOS["GF4500"] = true
VIDEOS["GF7600GS"] = true
VIDEOS["EPIC12"] = true
VIDEOS["FIXFREQ"] = true
VIDEOS["H63484"] = true
--VIDEOS["HD44102"] = true
--VIDEOS["HD44352"] = true
--VIDEOS["HD44780"] = true
VIDEOS["HD61830"] = true
VIDEOS["HD63484"] = true
--VIDEOS["HD66421"] = true
VIDEOS["HUC6202"] = true
VIDEOS["HUC6260"] = true
--VIDEOS["HUC6261"] = true
VIDEOS["HUC6270"] = true
--VIDEOS["HUC6272"] = true
--VIDEOS["I8244"] = true
VIDEOS["I8275"] = true
VIDEOS["M50458"] = true
VIDEOS["MB90082"] = true
VIDEOS["MB_VCU"] = true
VIDEOS["MC6845"] = true
--VIDEOS["MC6847"] = true
--VIDEOS["MSM6222B"] = true
--VIDEOS["MSM6255"] = true
--VIDEOS["MOS6566"] = true
VIDEOS["PC_VGA"] = true
VIDEOS["POLY"] = true
VIDEOS["PSX"] = true
VIDEOS["RAMDAC"] = true
--VIDEOS["S2636"] = true
VIDEOS["SAA5050"] = true
VIDEOS["SCN2674"] = true
--VIDEOS["SED1200"] = true
--VIDEOS["SED1330"] = true
--VIDEOS["SED1520"] = true
VIDEOS["SNES_PPU"] = true
VIDEOS["STVVDP"] = true
--VIDEOS["T6A04"] = true
VIDEOS["TLC34076"] = true
VIDEOS["TMS34061"] = true
--VIDEOS["TMS3556"] = true
VIDEOS["TMS9927"] = true
VIDEOS["TMS9928A"] = true
--VIDEOS["UPD3301"] = true
--VIDEOS["UPD7220"] = true
--VIDEOS["UPD7227"] = true
VIDEOS["V9938"] = true
--VIDEOS["VIC4567"] = true
VIDEOS["VOODOO"] = true
VIDEOS["VOODOO_PCI"] = true

--------------------------------------------------
-- specify available machine cores
--------------------------------------------------

MACHINES["AKIKO"] = true
MACHINES["NCR53C7XX"] = true
MACHINES["LSI53C810"] = true
MACHINES["6522VIA"] = true
MACHINES["TPI6525"] = true
MACHINES["RIOT6532"] = true
MACHINES["6821PIA"] = true
MACHINES["6840PTM"] = true
--MACHINES["68561MPCC"] = true
MACHINES["ACIA6850"] = true
MACHINES["68681"] = true
MACHINES["7200FIFO"] = true
--MACHINES["8530SCC"] = true
MACHINES["TTL74123"] = true
MACHINES["TTL74145"] = true
MACHINES["TTL74148"] = true
MACHINES["TTL74153"] = true
MACHINES["TTL74181"] = true
MACHINES["TTL7474"] = true
MACHINES["KBDC8042"] = true
MACHINES["I8257"] = true
MACHINES["AAKARTDEV"] = true
--MACHINES["ACIA6850"] = true
MACHINES["ADC0808"] = true
MACHINES["ADC083X"] = true
MACHINES["ADC1038"] = true
MACHINES["ADC1213X"] = true
MACHINES["AICARTC"] = true
MACHINES["AM53CF96"] = true
MACHINES["AM9517A"] = true
MACHINES["AMIGAFDC"] = true
--MACHINES["AT_KEYBC"] = true
MACHINES["AT28C16"] = true
MACHINES["AT29X"] = true
MACHINES["AT45DBXX"] = true
MACHINES["ATAFLASH"] = true
MACHINES["AY31015"] = true
MACHINES["BANKDEV"] = true
MACHINES["CDP1852"] = true
MACHINES["CDP1871"] = true
--MACHINES["CMOS40105"] = true
MACHINES["CDU76S"] = true
MACHINES["COM8116"] = true
MACHINES["CR589"] = true
--MACHINES["CS4031"] = true
--MACHINES["CS8221"] = true
--MACHINES["DP8390"] = true
MACHINES["DS1204"] = true
MACHINES["DS1302"] = true
--MACHINES["DS1315"] = true
MACHINES["DS2401"] = true
MACHINES["DS2404"] = true
MACHINES["DS75160A"] = true
MACHINES["DS75161A"] = true
MACHINES["E0516"] = true
MACHINES["E05A03"] = true
MACHINES["E05A30"] = true
MACHINES["EEPROMDEV"] = true
MACHINES["ER2055"] = true
MACHINES["F3853"] = true
--MACHINES["HD63450"] = true
--MACHINES["HD64610"] = true
MACHINES["I2CMEM"] = true
--MACHINES["I80130"] = true
--MACHINES["I8089"] = true
MACHINES["I8155"] = true
MACHINES["I8212"] = true
MACHINES["I8214"] = true
MACHINES["I8243"] = true
MACHINES["I8251"] = true
MACHINES["I8255"] = true
--MACHINES["I8257"] = true
--MACHINES["I8271"] = true
MACHINES["I8279"] = true
MACHINES["I8355"] = true
MACHINES["IDE"] = true
MACHINES["IM6402"] = true
MACHINES["INS8154"] = true
MACHINES["INS8250"] = true
MACHINES["INTELFLASH"] = true
MACHINES["JVS"] = true
MACHINES["K033906"] = true
MACHINES["K053252"] = true
MACHINES["K056230"] = true
--MACHINES["KB3600"] = true
--MACHINES["KBDC8042"] = true
--MACHINES["KR2376"] = true
MACHINES["LATCH8"] = true
MACHINES["LC89510"] = true
MACHINES["LDPR8210"] = true
MACHINES["LDSTUB"] = true
MACHINES["LDV1000"] = true
MACHINES["LDVP931"] = true
--MACHINES["LH5810"] = true
MACHINES["LINFLASH"] = true
MACHINES["LPCI"] = true
--MACHINES["LSI53C810"] = true
--MACHINES["M68307"] = true
--MACHINES["M68340"] = true
MACHINES["M6M80011AP"] = true
MACHINES["MATSUCD"] = true
MACHINES["MB14241"] = true
MACHINES["MB3773"] = true
MACHINES["MB8421"] = true
MACHINES["MB87078"] = true
--MACHINES["MB8795"] = true
--MACHINES["MB89352"] = true
MACHINES["MB89371"] = true
MACHINES["MC146818"] = true
MACHINES["MC2661"] = true
MACHINES["MC6843"] = true
MACHINES["MC6846"] = true
MACHINES["MC6852"] = true
MACHINES["MC6854"] = true
--MACHINES["MC68328"] = true
MACHINES["MC68901"] = true
MACHINES["MCCS1850"] = true
MACHINES["M68307"] = true
MACHINES["M68340"] = true
MACHINES["MCF5206E"] = true
MACHINES["MICROTOUCH"] = true
--MACHINES["MIOT6530"] = true
--MACHINES["MM58167"] = true
MACHINES["MM58274C"] = true
MACHINES["MM74C922"] = true
MACHINES["MOS6526"] = true
MACHINES["MOS6529"] = true
MACHINES["MIOT6530"] = true
MACHINES["MOS6551"] = true
--MACHINES["MOS6702"] = true
--MACHINES["MOS8706"] = true
--MACHINES["MOS8722"] = true
--MACHINES["MOS8726"] = true
--MACHINES["MPU401"] = true
MACHINES["MSM5832"] = true
MACHINES["MSM58321"] = true
MACHINES["MSM6242"] = true
--MACHINES["NCR5380"] = true
--MACHINES["NCR5380N"] = true
--MACHINES["NCR5390"] = true
MACHINES["NCR539x"] = true
MACHINES["NETLIST"] = true
--MACHINES["NCR53C7XX"] = true
MACHINES["NMC9306"] = true
--MACHINES["NSC810"] = true
MACHINES["NSCSI"] = true
--MACHINES["PC_FDC"] = true
--MACHINES["PC_LPT"] = true
--MACHINES["PCCARD"] = true
MACHINES["PCF8593"] = true
MACHINES["PCI"] = true
MACHINES["PCKEYBRD"] = true
MACHINES["PIC8259"] = true
MACHINES["PIT8253"] = true
MACHINES["PLA"] = true
--MACHINES["PROFILE"] = true
MACHINES["R10696"] = true
MACHINES["R10788"] = true
MACHINES["RA17XX"] = true
--MACHINES["R64H156"] = true
MACHINES["RF5C296"] = true
--MACHINES["RIOT6532"] = true
MACHINES["ROC10937"] = true
MACHINES["RP5C01"] = true
MACHINES["RP5C15"] = true
MACHINES["RP5H01"] = true
MACHINES["RTC4543"] = true
MACHINES["RTC65271"] = true
MACHINES["RTC9701"] = true
MACHINES["S2636"] = true
MACHINES["S3520CF"] = true
MACHINES["S3C2400"] = true
MACHINES["S3C2410"] = true
MACHINES["S3C2440"] = true
--MACHINES["S3C44B0"] = true
MACHINES["SATURN"] = true
MACHINES["SCSI"] = true
MACHINES["SCUDSP"] = true
--MACHINES["SECFLASH"] = true
MACHINES["SERFLASH"] = true
MACHINES["SMC91C9X"] = true
MACHINES["SMPC"] = true
MACHINES["STVCD"] = true
MACHINES["TC0091LVC"] = true
MACHINES["TIMEKPR"] = true
MACHINES["TMP68301"] = true
--MACHINES["TMS5501"] = true
MACHINES["TMS6100"] = true
MACHINES["TMS9901"] = true
MACHINES["TMS9902"] = true
--MACHINES["TPI6525"] = true
--MACHINES["TTL74123"] = true
--MACHINES["TTL74145"] = true
--MACHINES["TTL74148"] = true
--MACHINES["TTL74153"] = true
--MACHINES["TTL74181"] = true
--MACHINES["TTL7474"] = true
MACHINES["UPD1990A"] = true
MACHINES["UPD4992"] = true
MACHINES["UPD4701"] = true
MACHINES["UPD7002"] = true
MACHINES["UPD71071"] = true
MACHINES["UPD765"] = true
MACHINES["V3021"] = true
MACHINES["WD_FDC"] = true
MACHINES["WD11C00_17"] = true
MACHINES["WD2010"] = true
MACHINES["WD33C93"] = true
MACHINES["X2212"] = true
MACHINES["X76F041"] = true
MACHINES["X76F100"] = true
MACHINES["Z80CTC"] = true
MACHINES["Z80DART"] = true
MACHINES["Z80DMA"] = true
MACHINES["Z80PIO"] = true
MACHINES["Z80STI"] = true
MACHINES["Z8536"] = true
MACHINES["SECFLASH"] = true
MACHINES["PCCARD"] = true
MACHINES["FDC37C665GT"] = true
--MACHINES["SMC92X4"] = true
--MACHINES["TI99_HD"] = true
--MACHINES["STRATA"] = true
MACHINES["STEPPERS"] = true
--MACHINES["CORVUSHD"] = true
--MACHINES["WOZFDC"] = true
--MACHINES["DIABLO_HD"] = true
MACHINES["PCI9050"] = true
--MACHINES["TMS1024"] = true

--------------------------------------------------
-- specify available bus cores
--------------------------------------------------

--BUSES["A1BUS"] = true
--BUSES["A2BUS"] = true
--BUSES["A7800"] = true
--BUSES["A800"] = true
--BUSES["ABCBUS"] = true
--BUSES["ABCKB"] = true
--BUSES["ADAM"] = true
--BUSES["ADAMNET"] = true
--BUSES["APF"] = true
--BUSES["ARCADIA"] = true
--BUSES["ASTROCADE"] = true
--BUSES["BML3"] = true
--BUSES["BW2"] = true
--BUSES["C64"] = true
--BUSES["CBM2"] = true
--BUSES["CBMIEC"] = true
BUSES["CENTRONICS"] = true
--BUSES["CHANNELF"] = true
--BUSES["COCO"] = true
--BUSES["COLECO"] = true
--BUSES["COMPUCOLOR"] = true
--BUSES["COMX35"] = true
--BUSES["CPC"] = true
--BUSES["CRVISION"] = true
--BUSES["DMV"] = true
--BUSES["ECBBUS"] = true
--BUSES["ECONET"] = true
--BUSES["EP64"] = true
--BUSES["EPSON_SIO"] = true
--BUSES["GAMEBOY"] = true
--BUSES["GBA"] = true
BUSES["GENERIC"] = true
--BUSES["IEEE488"] = true
--BUSES["IMI7000"] = true
--BUSES["INTV"] = true
--BUSES["IQ151"] = true
BUSES["ISA"] = true
--BUSES["ISBX"] = true
--BUSES["KC"] = true
--BUSES["LPCI"] = true
--BUSES["MACPDS"] = true
BUSES["MIDI"] = true
--BUSES["MEGADRIVE"] = true
--BUSES["MSX_SLOT"] = true
BUSES["NEOGEO"] = true
--BUSES["NES"] = true
--BUSES["NUBUS"] = true
--BUSES["O2"] = true
--BUSES["ORICEXT"] = true
--BUSES["PCE"] = true
BUSES["PC_JOY"] = true
--BUSES["PC_KBD"] = true
--BUSES["PET"] = true
--BUSES["PLUS4"] = true
--BUSES["PSX_CONTROLLER"] = true
--BUSES["QL"] = true
BUSES["RS232"] = true
--BUSES["S100"] = true
--BUSES["SATURN"] = true
BUSES["SCSI"] = true
--BUSES["SCV"] = true
--BUSES["SEGA8"] = true
--BUSES["SMS_CTRL"] = true
--BUSES["SMS_EXP"] = true
--BUSES["SNES"] = true
--BUSES["SPC1000"] = true
--BUSES["TI99PEB"] = true
--BUSES["TVC"] = true
--BUSES["VBOY"] = true
--BUSES["VC4000"] = true
--BUSES["VCS"] = true
--BUSES["VCS_CTRL"] = true
BUSES["VECTREX"] = true
--BUSES["VIC10"] = true
--BUSES["VIC20"] = true
--BUSES["VIDBRAIN"] = true
--BUSES["VIP"] = true
--BUSES["VTECH_IOEXP"] = true
--BUSES["VTECH_MEMEXP"] = true
--BUSES["WANGPC"] = true
--BUSES["WSWAN"] = true
--BUSES["X68K"] = true
--BUSES["Z88"] = true
--BUSES["ZORRO"] = true

--------------------------------------------------
-- this is the list of driver libraries that
-- comprise MAME plus mamedriv.o which contains
-- the list of drivers
--------------------------------------------------

function linkProjects_mame_arcade(_target, _subtarget)
	links {
		"acorn",
		"alba",
		"alliedl",
		"alpha",
		"amiga",
		"aristocr",
		"ascii",
		"atari",
		"atlus",
		"barcrest",
		"bfm",
		"bmc",
		"capcom",
		"cinemat",
		"comad",
		"cvs",
		"dataeast",
		"dgrm",
		"dooyong",
		"dynax",
		"edevices",
		"eolith",
		"excelent",
		"exidy",
		"f32",
		"funworld",
		"fuuki",
		"gaelco",
		"gameplan",
		"gametron",
		"gottlieb",
		"ibmpc",
		"igs",
		"irem",
		"itech",
		"jaleco",
		"jpm",
		"kaneko",
		"konami",
		"matic",
		"maygay",
		"meadows",
		"merit",
		"metro",
		"midcoin",
		"midw8080",
		"midway",
		"namco",
		"nasco",
		"neogeo",
		"nichibut",
		"nintendo",
		"nix",
		"nmk",
		"omori",
		"olympia",
		"orca",
		"pacific",
		"pacman",
		"pce",
		"phoenix",
		"playmark",
		"psikyo",
		"ramtek",
		"rare",
		"sanritsu",
		"sega",
		"seibu",
		"seta",
		"sigma",
		"snk",
		"sony",
		"stern",
		"subsino",
		"sun",
		"suna",
		"sure",
		"taito",
		"tatsumi",
		"tch",
		"tecfri",
		"technos",
		"tehkan",
		"thepit",
		"toaplan",
		"tong",
		"unico",
		"univers",
		"upl",
		"valadon",
		"veltmjr",
		"venture",
		"vsystem",
		"yunsung",
		"zaccaria",
		"misc",
		"pinball",
		"shared",
	}
end

function createMAMEProjects(_target, _subtarget, _name)
	project (_name)
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-" .. _target .."_" .. _subtarget .. "_" .._name))

	options {
		"ForceCPP",
	}

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/lib/netlist",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
	}
end

function createProjects_mame_arcade(_target, _subtarget)
--------------------------------------------------
-- the following files are general components and
-- shared across a number of drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "shared")
files {
	MAME_DIR .. "src/mame/machine/nmk112.c",
	MAME_DIR .. "src/mame/machine/pcshare.c",
	MAME_DIR .. "src/mame/machine/segacrpt.c",
	MAME_DIR .. "src/mame/machine/segacrp2.c",
	MAME_DIR .. "src/mame/machine/ticket.c",
	MAME_DIR .. "src/mame/video/avgdvg.c",
	MAME_DIR .. "src/mame/audio/dcs.c",
	MAME_DIR .. "src/mame/audio/decobsmt.c",
	MAME_DIR .. "src/mame/audio/segam1audio.c",
}

--------------------------------------------------
-- manufacturer-specific groupings for drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "acorn")
files {
	MAME_DIR .. "src/mame/drivers/ertictac.c",
	MAME_DIR .. "src/mame/drivers/ssfindo.c",
	MAME_DIR .. "src/mame/drivers/aristmk5.c",
	MAME_DIR .. "src/mame/machine/archimds.c",
	MAME_DIR .. "src/mame/video/archimds.c",
}

createMAMEProjects(_target, _subtarget, "alba")
files {
	MAME_DIR .. "src/mame/drivers/albazc.c",
	MAME_DIR .. "src/mame/drivers/albazg.c",
	MAME_DIR .. "src/mame/drivers/rmhaihai.c",
}

createMAMEProjects(_target, _subtarget, "alliedl")
files {
	MAME_DIR .. "src/mame/drivers/ace.c",
	MAME_DIR .. "src/mame/drivers/aleisttl.c",
	MAME_DIR .. "src/mame/drivers/clayshoo.c",
}

createMAMEProjects(_target, _subtarget, "alpha")
files {
	MAME_DIR .. "src/mame/drivers/alpha68k.c",
	MAME_DIR .. "src/mame/video/alpha68k.c",
	MAME_DIR .. "src/mame/drivers/champbas.c",
	MAME_DIR .. "src/mame/video/champbas.c",
	MAME_DIR .. "src/mame/drivers/equites.c",
	MAME_DIR .. "src/mame/video/equites.c",
	MAME_DIR .. "src/mame/drivers/meijinsn.c",
	MAME_DIR .. "src/mame/drivers/shougi.c",
}

createMAMEProjects(_target, _subtarget, "amiga")
files {
	MAME_DIR .. "src/mame/drivers/alg.c",
	MAME_DIR .. "src/mame/machine/amiga.c",
	MAME_DIR .. "src/mame/video/amiga.c",
	MAME_DIR .. "src/mame/video/amigaaga.c",
	MAME_DIR .. "src/mame/drivers/arcadia.c",
	MAME_DIR .. "src/mame/drivers/cubo.c",
	MAME_DIR .. "src/mame/drivers/mquake.c",
	MAME_DIR .. "src/mame/drivers/upscope.c",
}

createMAMEProjects(_target, _subtarget, "aristocr")
files {
	MAME_DIR .. "src/mame/drivers/aristmk4.c",
	MAME_DIR .. "src/mame/drivers/aristmk6.c",
	MAME_DIR .. "src/mame/drivers/caswin.c",
}

createMAMEProjects(_target, _subtarget, "ascii")
files {
	MAME_DIR .. "src/mame/drivers/big10.c",
	MAME_DIR .. "src/mame/drivers/forte2.c",
	MAME_DIR .. "src/mame/drivers/pengadvb.c",
	MAME_DIR .. "src/mame/drivers/sangho.c",
	MAME_DIR .. "src/mame/drivers/sfkick.c",
}

createMAMEProjects(_target, _subtarget, "atari")
files {
	MAME_DIR .. "src/mame/drivers/arcadecl.c",
	MAME_DIR .. "src/mame/video/arcadecl.c",
	MAME_DIR .. "src/mame/drivers/asteroid.c",
	MAME_DIR .. "src/mame/machine/asteroid.c",
	MAME_DIR .. "src/mame/audio/asteroid.c",
	MAME_DIR .. "src/mame/audio/llander.c",
	MAME_DIR .. "src/mame/drivers/atarifb.c",
	MAME_DIR .. "src/mame/machine/atarifb.c",
	MAME_DIR .. "src/mame/audio/atarifb.c",
	MAME_DIR .. "src/mame/video/atarifb.c",
	MAME_DIR .. "src/mame/drivers/atarig1.c",
	MAME_DIR .. "src/mame/video/atarig1.c",
	MAME_DIR .. "src/mame/drivers/atarig42.c",
	MAME_DIR .. "src/mame/video/atarig42.c",
	MAME_DIR .. "src/mame/drivers/atarigt.c",
	MAME_DIR .. "src/mame/video/atarigt.c",
	MAME_DIR .. "src/mame/drivers/atarigx2.c",
	MAME_DIR .. "src/mame/video/atarigx2.c",
	MAME_DIR .. "src/mame/drivers/atarisy1.c",
	MAME_DIR .. "src/mame/video/atarisy1.c",
	MAME_DIR .. "src/mame/drivers/atarisy2.c",
	MAME_DIR .. "src/mame/video/atarisy2.c",
	MAME_DIR .. "src/mame/drivers/atarisy4.c",
	MAME_DIR .. "src/mame/drivers/atarittl.c",
	MAME_DIR .. "src/mame/drivers/atetris.c",
	MAME_DIR .. "src/mame/video/atetris.c",
	MAME_DIR .. "src/mame/drivers/avalnche.c",
	MAME_DIR .. "src/mame/audio/avalnche.c",
	MAME_DIR .. "src/mame/drivers/badlands.c",
	MAME_DIR .. "src/mame/video/badlands.c",
	MAME_DIR .. "src/mame/drivers/bartop52.c",
	MAME_DIR .. "src/mame/drivers/batman.c",
	MAME_DIR .. "src/mame/video/batman.c",
	MAME_DIR .. "src/mame/drivers/beathead.c",
	MAME_DIR .. "src/mame/video/beathead.c",
	MAME_DIR .. "src/mame/drivers/blstroid.c",
	MAME_DIR .. "src/mame/video/blstroid.c",
	MAME_DIR .. "src/mame/drivers/boxer.c",
	MAME_DIR .. "src/mame/drivers/bsktball.c",
	MAME_DIR .. "src/mame/machine/bsktball.c",
	MAME_DIR .. "src/mame/audio/bsktball.c",
	MAME_DIR .. "src/mame/video/bsktball.c",
	MAME_DIR .. "src/mame/drivers/bwidow.c",
	MAME_DIR .. "src/mame/audio/bwidow.c",
	MAME_DIR .. "src/mame/drivers/bzone.c",
	MAME_DIR .. "src/mame/audio/bzone.c",
	MAME_DIR .. "src/mame/drivers/canyon.c",
	MAME_DIR .. "src/mame/audio/canyon.c",
	MAME_DIR .. "src/mame/video/canyon.c",
	MAME_DIR .. "src/mame/drivers/cball.c",
	MAME_DIR .. "src/mame/drivers/ccastles.c",
	MAME_DIR .. "src/mame/video/ccastles.c",
	MAME_DIR .. "src/mame/drivers/centiped.c",
	MAME_DIR .. "src/mame/video/centiped.c",
	MAME_DIR .. "src/mame/drivers/cloak.c",
	MAME_DIR .. "src/mame/video/cloak.c",
	MAME_DIR .. "src/mame/drivers/cloud9.c",
	MAME_DIR .. "src/mame/video/cloud9.c",
	MAME_DIR .. "src/mame/drivers/cmmb.c",
	MAME_DIR .. "src/mame/drivers/cops.c",
	MAME_DIR .. "src/mame/drivers/copsnrob.c",
	MAME_DIR .. "src/mame/audio/copsnrob.c",
	MAME_DIR .. "src/mame/video/copsnrob.c",
	MAME_DIR .. "src/mame/drivers/cyberbal.c",
	MAME_DIR .. "src/mame/audio/cyberbal.c",
	MAME_DIR .. "src/mame/video/cyberbal.c",
	MAME_DIR .. "src/mame/drivers/destroyr.c",
	MAME_DIR .. "src/mame/drivers/dragrace.c",
	MAME_DIR .. "src/mame/audio/dragrace.c",
	MAME_DIR .. "src/mame/video/dragrace.c",
	MAME_DIR .. "src/mame/drivers/eprom.c",
	MAME_DIR .. "src/mame/video/eprom.c",
	MAME_DIR .. "src/mame/drivers/firefox.c",
	MAME_DIR .. "src/mame/drivers/firetrk.c",
	MAME_DIR .. "src/mame/audio/firetrk.c",
	MAME_DIR .. "src/mame/video/firetrk.c",
	MAME_DIR .. "src/mame/drivers/flyball.c",
	MAME_DIR .. "src/mame/drivers/foodf.c",
	MAME_DIR .. "src/mame/video/foodf.c",
	MAME_DIR .. "src/mame/drivers/gauntlet.c",
	MAME_DIR .. "src/mame/video/gauntlet.c",
	MAME_DIR .. "src/mame/drivers/harddriv.c",
	MAME_DIR .. "src/mame/machine/harddriv.c",
	MAME_DIR .. "src/mame/audio/harddriv.c",
	MAME_DIR .. "src/mame/video/harddriv.c",
	MAME_DIR .. "src/mame/drivers/irobot.c",
	MAME_DIR .. "src/mame/machine/irobot.c",
	MAME_DIR .. "src/mame/video/irobot.c",
	MAME_DIR .. "src/mame/drivers/jaguar.c",
	MAME_DIR .. "src/mame/audio/jaguar.c",
	MAME_DIR .. "src/mame/video/jaguar.c",
	MAME_DIR .. "src/mame/drivers/jedi.c",
	MAME_DIR .. "src/mame/audio/jedi.c",
	MAME_DIR .. "src/mame/video/jedi.c",
	MAME_DIR .. "src/mame/drivers/klax.c",
	MAME_DIR .. "src/mame/video/klax.c",
	MAME_DIR .. "src/mame/drivers/liberatr.c",
	MAME_DIR .. "src/mame/video/liberatr.c",
	MAME_DIR .. "src/mame/drivers/mediagx.c",
	MAME_DIR .. "src/mame/drivers/metalmx.c",
	MAME_DIR .. "src/mame/drivers/mgolf.c",
	MAME_DIR .. "src/mame/drivers/mhavoc.c",
	MAME_DIR .. "src/mame/machine/mhavoc.c",
	MAME_DIR .. "src/mame/drivers/missile.c",
	MAME_DIR .. "src/mame/drivers/nitedrvr.c",
	MAME_DIR .. "src/mame/machine/nitedrvr.c",
	MAME_DIR .. "src/mame/audio/nitedrvr.c",
	MAME_DIR .. "src/mame/video/nitedrvr.c",
	MAME_DIR .. "src/mame/drivers/offtwall.c",
	MAME_DIR .. "src/mame/video/offtwall.c",
	MAME_DIR .. "src/mame/drivers/orbit.c",
	MAME_DIR .. "src/mame/audio/orbit.c",
	MAME_DIR .. "src/mame/video/orbit.c",
	MAME_DIR .. "src/mame/drivers/pong.c",
	MAME_DIR .. "src/mame/drivers/nl_pong.c",
	MAME_DIR .. "src/mame/drivers/nl_pongd.c",
	MAME_DIR .. "src/mame/drivers/nl_breakout.c",
	MAME_DIR .. "src/mame/drivers/poolshrk.c",
	MAME_DIR .. "src/mame/audio/poolshrk.c",
	MAME_DIR .. "src/mame/video/poolshrk.c",
	MAME_DIR .. "src/mame/drivers/quantum.c",
	MAME_DIR .. "src/mame/drivers/quizshow.c",
	MAME_DIR .. "src/mame/drivers/rampart.c",
	MAME_DIR .. "src/mame/video/rampart.c",
	MAME_DIR .. "src/mame/drivers/relief.c",
	MAME_DIR .. "src/mame/video/relief.c",
	MAME_DIR .. "src/mame/drivers/runaway.c",
	MAME_DIR .. "src/mame/video/runaway.c",
	MAME_DIR .. "src/mame/drivers/sbrkout.c",
	MAME_DIR .. "src/mame/drivers/shuuz.c",
	MAME_DIR .. "src/mame/video/shuuz.c",
	MAME_DIR .. "src/mame/drivers/skullxbo.c",
	MAME_DIR .. "src/mame/video/skullxbo.c",
	MAME_DIR .. "src/mame/drivers/skydiver.c",
	MAME_DIR .. "src/mame/audio/skydiver.c",
	MAME_DIR .. "src/mame/video/skydiver.c",
	MAME_DIR .. "src/mame/drivers/skyraid.c",
	MAME_DIR .. "src/mame/audio/skyraid.c",
	MAME_DIR .. "src/mame/video/skyraid.c",
	MAME_DIR .. "src/mame/drivers/sprint2.c",
	MAME_DIR .. "src/mame/audio/sprint2.c",
	MAME_DIR .. "src/mame/video/sprint2.c",
	MAME_DIR .. "src/mame/drivers/sprint4.c",
	MAME_DIR .. "src/mame/video/sprint4.c",
	MAME_DIR .. "src/mame/audio/sprint4.c",
	MAME_DIR .. "src/mame/drivers/sprint8.c",
	MAME_DIR .. "src/mame/audio/sprint8.c",
	MAME_DIR .. "src/mame/video/sprint8.c",
	MAME_DIR .. "src/mame/drivers/starshp1.c",
	MAME_DIR .. "src/mame/audio/starshp1.c",
	MAME_DIR .. "src/mame/video/starshp1.c",
	MAME_DIR .. "src/mame/drivers/starwars.c",
	MAME_DIR .. "src/mame/machine/starwars.c",
	MAME_DIR .. "src/mame/audio/starwars.c",
	MAME_DIR .. "src/mame/drivers/subs.c",
	MAME_DIR .. "src/mame/machine/subs.c",
	MAME_DIR .. "src/mame/audio/subs.c",
	MAME_DIR .. "src/mame/video/subs.c",
	MAME_DIR .. "src/mame/drivers/tank8.c",
	MAME_DIR .. "src/mame/audio/tank8.c",
	MAME_DIR .. "src/mame/video/tank8.c",
	MAME_DIR .. "src/mame/drivers/tempest.c",
	MAME_DIR .. "src/mame/drivers/thunderj.c",
	MAME_DIR .. "src/mame/video/thunderj.c",
	MAME_DIR .. "src/mame/drivers/tomcat.c",
	MAME_DIR .. "src/mame/drivers/toobin.c",
	MAME_DIR .. "src/mame/video/toobin.c",
	MAME_DIR .. "src/mame/drivers/tourtabl.c",
	MAME_DIR .. "src/mame/video/tia.c",
	MAME_DIR .. "src/mame/drivers/triplhnt.c",
	MAME_DIR .. "src/mame/audio/triplhnt.c",
	MAME_DIR .. "src/mame/video/triplhnt.c",
	MAME_DIR .. "src/mame/drivers/tunhunt.c",
	MAME_DIR .. "src/mame/video/tunhunt.c",
	MAME_DIR .. "src/mame/drivers/ultratnk.c",
	MAME_DIR .. "src/mame/video/ultratnk.c",
	MAME_DIR .. "src/mame/drivers/videopin.c",
	MAME_DIR .. "src/mame/audio/videopin.c",
	MAME_DIR .. "src/mame/video/videopin.c",
	MAME_DIR .. "src/mame/drivers/vindictr.c",
	MAME_DIR .. "src/mame/video/vindictr.c",
	MAME_DIR .. "src/mame/drivers/wolfpack.c",
	MAME_DIR .. "src/mame/video/wolfpack.c",
	MAME_DIR .. "src/mame/drivers/xybots.c",
	MAME_DIR .. "src/mame/video/xybots.c",
	MAME_DIR .. "src/mame/machine/asic65.c",
	MAME_DIR .. "src/mame/machine/atari_vg.c",
	MAME_DIR .. "src/mame/machine/atarigen.c",
	MAME_DIR .. "src/mame/machine/mathbox.c",
	MAME_DIR .. "src/mame/machine/slapstic.c",
	MAME_DIR .. "src/mame/audio/atarijsa.c",
	MAME_DIR .. "src/mame/audio/cage.c",
	MAME_DIR .. "src/mame/audio/redbaron.c",
	MAME_DIR .. "src/mame/video/atarimo.c",
	MAME_DIR .. "src/mame/video/atarirle.c",
}

createMAMEProjects(_target, _subtarget, "atlus")
files {
	MAME_DIR .. "src/mame/drivers/blmbycar.c",
	MAME_DIR .. "src/mame/video/blmbycar.c",
	MAME_DIR .. "src/mame/drivers/ohmygod.c",
	MAME_DIR .. "src/mame/video/ohmygod.c",
	MAME_DIR .. "src/mame/drivers/powerins.c",
	MAME_DIR .. "src/mame/video/powerins.c",
	MAME_DIR .. "src/mame/drivers/bowltry.c",
}

createMAMEProjects(_target, _subtarget, "barcrest")
files {
	MAME_DIR .. "src/mame/drivers/mpu2.c",
	MAME_DIR .. "src/mame/drivers/mpu3.c",
	MAME_DIR .. "src/mame/drivers/mpu4hw.c",
	MAME_DIR .. "src/mame/drivers/mpu4sw.c",
	MAME_DIR .. "src/mame/drivers/mpu4.c",
	MAME_DIR .. "src/mame/drivers/mpu4mod2sw.c",
	MAME_DIR .. "src/mame/drivers/mpu4mod4yam.c",
	MAME_DIR .. "src/mame/drivers/mpu4plasma.c",
	MAME_DIR .. "src/mame/drivers/mpu4dealem.c",
	MAME_DIR .. "src/mame/drivers/mpu4vid.c",
	MAME_DIR .. "src/mame/drivers/mpu4avan.c",
	MAME_DIR .. "src/mame/drivers/mpu4union.c",
	MAME_DIR .. "src/mame/drivers/mpu4concept.c",
	MAME_DIR .. "src/mame/drivers/mpu4empire.c",
	MAME_DIR .. "src/mame/drivers/mpu4mdm.c",
	MAME_DIR .. "src/mame/drivers/mpu4crystal.c",
	MAME_DIR .. "src/mame/drivers/mpu4bwb.c",
	MAME_DIR .. "src/mame/drivers/mpu4misc.c",
	MAME_DIR .. "src/mame/drivers/mpu5hw.c",
	MAME_DIR .. "src/mame/drivers/mpu5.c",
	MAME_DIR .. "src/mame/video/awpvid.c",
	MAME_DIR .. "src/mame/machine/meters.c",
}

createMAMEProjects(_target, _subtarget, "bfm")
files {
	MAME_DIR .. "src/mame/drivers/bfcobra.c",
	MAME_DIR .. "src/mame/machine/bfm_comn.c",
	MAME_DIR .. "src/mame/drivers/bfm_sc1.c",
	MAME_DIR .. "src/mame/drivers/bfm_sc2.c",
	MAME_DIR .. "src/mame/video/bfm_adr2.c",
	MAME_DIR .. "src/mame/drivers/bfm_sc4.c",
	MAME_DIR .. "src/mame/drivers/bfm_sc4h.c",
	MAME_DIR .. "src/mame/drivers/bfm_sc5.c",
	MAME_DIR .. "src/mame/drivers/bfm_sc5sw.c",
	MAME_DIR .. "src/mame/drivers/bfm_ad5.c",
	MAME_DIR .. "src/mame/drivers/bfm_ad5sw.c",
	MAME_DIR .. "src/mame/drivers/bfm_sc45_helper.c",
	MAME_DIR .. "src/mame/drivers/bfm_swp.c",
	MAME_DIR .. "src/mame/drivers/bfmsys83.c",
	MAME_DIR .. "src/mame/drivers/bfmsys85.c",
	MAME_DIR .. "src/mame/machine/sec.c",
	MAME_DIR .. "src/mame/machine/bfm_bd1.c",
	MAME_DIR .. "src/mame/machine/bfm_bda.c",
	MAME_DIR .. "src/mame/video/bfm_dm01.c",
	MAME_DIR .. "src/mame/drivers/rastersp.c",
}

createMAMEProjects(_target, _subtarget, "bmc")
files {
	MAME_DIR .. "src/mame/drivers/bmcbowl.c",
	MAME_DIR .. "src/mame/drivers/koftball.c",
	MAME_DIR .. "src/mame/drivers/popobear.c",
	MAME_DIR .. "src/mame/drivers/bmcpokr.c",
}

createMAMEProjects(_target, _subtarget, "capcom")
files {
	MAME_DIR .. "src/mame/drivers/1942.c",
	MAME_DIR .. "src/mame/video/1942.c",
	MAME_DIR .. "src/mame/drivers/1943.c",
	MAME_DIR .. "src/mame/video/1943.c",
	MAME_DIR .. "src/mame/drivers/alien.c",
	MAME_DIR .. "src/mame/drivers/bionicc.c",
	MAME_DIR .. "src/mame/video/bionicc.c",
	MAME_DIR .. "src/mame/drivers/supduck.c",
	MAME_DIR .. "src/mame/video/tigeroad_spr.c",
	MAME_DIR .. "src/mame/drivers/blktiger.c",
	MAME_DIR .. "src/mame/video/blktiger.c",
	MAME_DIR .. "src/mame/drivers/cbasebal.c",
	MAME_DIR .. "src/mame/video/cbasebal.c",
	MAME_DIR .. "src/mame/drivers/commando.c",
	MAME_DIR .. "src/mame/video/commando.c",
	MAME_DIR .. "src/mame/drivers/cps1.c",
	MAME_DIR .. "src/mame/video/cps1.c",
	MAME_DIR .. "src/mame/drivers/kenseim.c",
	MAME_DIR .. "src/mame/drivers/cps2.c",
	MAME_DIR .. "src/mame/machine/cps2crpt.c",
	MAME_DIR .. "src/mame/drivers/cps3.c",
	MAME_DIR .. "src/mame/audio/cps3.c",
	MAME_DIR .. "src/mame/drivers/egghunt.c",
	MAME_DIR .. "src/mame/drivers/exedexes.c",
	MAME_DIR .. "src/mame/video/exedexes.c",
	MAME_DIR .. "src/mame/drivers/fcrash.c",
	MAME_DIR .. "src/mame/drivers/gng.c",
	MAME_DIR .. "src/mame/video/gng.c",
	MAME_DIR .. "src/mame/drivers/gunsmoke.c",
	MAME_DIR .. "src/mame/video/gunsmoke.c",
	MAME_DIR .. "src/mame/drivers/higemaru.c",
	MAME_DIR .. "src/mame/video/higemaru.c",
	MAME_DIR .. "src/mame/drivers/lastduel.c",
	MAME_DIR .. "src/mame/video/lastduel.c",
	MAME_DIR .. "src/mame/drivers/lwings.c",
	MAME_DIR .. "src/mame/video/lwings.c",
	MAME_DIR .. "src/mame/drivers/mitchell.c",
	MAME_DIR .. "src/mame/video/mitchell.c",
	MAME_DIR .. "src/mame/drivers/sf.c",
	MAME_DIR .. "src/mame/video/sf.c",
	MAME_DIR .. "src/mame/drivers/sidearms.c",
	MAME_DIR .. "src/mame/video/sidearms.c",
	MAME_DIR .. "src/mame/drivers/sonson.c",
	MAME_DIR .. "src/mame/video/sonson.c",
	MAME_DIR .. "src/mame/drivers/srumbler.c",
	MAME_DIR .. "src/mame/video/srumbler.c",
	MAME_DIR .. "src/mame/drivers/tigeroad.c",
	MAME_DIR .. "src/mame/video/tigeroad.c",
	MAME_DIR .. "src/mame/machine/tigeroad.c",
	MAME_DIR .. "src/mame/drivers/vulgus.c",
	MAME_DIR .. "src/mame/video/vulgus.c",
	MAME_DIR .. "src/mame/machine/kabuki.c",
}

createMAMEProjects(_target, _subtarget, "cinemat")
files {
	MAME_DIR .. "src/mame/drivers/ataxx.c",
	MAME_DIR .. "src/mame/drivers/cinemat.c",
	MAME_DIR .. "src/mame/audio/cinemat.c",
	MAME_DIR .. "src/mame/video/cinemat.c",
	MAME_DIR .. "src/mame/drivers/cchasm.c",
	MAME_DIR .. "src/mame/machine/cchasm.c",
	MAME_DIR .. "src/mame/audio/cchasm.c",
	MAME_DIR .. "src/mame/video/cchasm.c",
	MAME_DIR .. "src/mame/drivers/dlair.c",
	MAME_DIR .. "src/mame/drivers/dlair2.c",
	MAME_DIR .. "src/mame/drivers/embargo.c",
	MAME_DIR .. "src/mame/drivers/jack.c",
	MAME_DIR .. "src/mame/video/jack.c",
	MAME_DIR .. "src/mame/drivers/leland.c",
	MAME_DIR .. "src/mame/machine/leland.c",
	MAME_DIR .. "src/mame/audio/leland.c",
	MAME_DIR .. "src/mame/video/leland.c",
}

createMAMEProjects(_target, _subtarget, "comad")
files {
	MAME_DIR .. "src/mame/drivers/funybubl.c",
	MAME_DIR .. "src/mame/video/funybubl.c",
	MAME_DIR .. "src/mame/drivers/galspnbl.c",
	MAME_DIR .. "src/mame/video/galspnbl.c",
	MAME_DIR .. "src/mame/drivers/zerozone.c",
	MAME_DIR .. "src/mame/video/zerozone.c",
}

createMAMEProjects(_target, _subtarget, "cvs")
files {
	MAME_DIR .. "src/mame/drivers/cvs.c",
	MAME_DIR .. "src/mame/video/cvs.c",
	MAME_DIR .. "src/mame/drivers/galaxia.c",
	MAME_DIR .. "src/mame/video/galaxia.c",
	MAME_DIR .. "src/mame/drivers/quasar.c",
	MAME_DIR .. "src/mame/video/quasar.c",
}

createMAMEProjects(_target, _subtarget, "dataeast")
files {
	MAME_DIR .. "src/mame/drivers/actfancr.c",
	MAME_DIR .. "src/mame/video/actfancr.c",
	MAME_DIR .. "src/mame/drivers/astrof.c",
	MAME_DIR .. "src/mame/audio/astrof.c",
	MAME_DIR .. "src/mame/drivers/backfire.c",
	MAME_DIR .. "src/mame/drivers/battlera.c",
	MAME_DIR .. "src/mame/drivers/boogwing.c",
	MAME_DIR .. "src/mame/video/boogwing.c",
	MAME_DIR .. "src/mame/drivers/brkthru.c",
	MAME_DIR .. "src/mame/video/brkthru.c",
	MAME_DIR .. "src/mame/drivers/btime.c",
	MAME_DIR .. "src/mame/machine/btime.c",
	MAME_DIR .. "src/mame/video/btime.c",
	MAME_DIR .. "src/mame/drivers/bwing.c",
	MAME_DIR .. "src/mame/video/bwing.c",
	MAME_DIR .. "src/mame/drivers/cbuster.c",
	MAME_DIR .. "src/mame/video/cbuster.c",
	MAME_DIR .. "src/mame/drivers/chanbara.c",
	MAME_DIR .. "src/mame/drivers/cninja.c",
	MAME_DIR .. "src/mame/video/cninja.c",
	MAME_DIR .. "src/mame/drivers/cntsteer.c",
	MAME_DIR .. "src/mame/drivers/compgolf.c",
	MAME_DIR .. "src/mame/video/compgolf.c",
	MAME_DIR .. "src/mame/drivers/darkseal.c",
	MAME_DIR .. "src/mame/video/darkseal.c",
	MAME_DIR .. "src/mame/drivers/dassault.c",
	MAME_DIR .. "src/mame/video/dassault.c",
	MAME_DIR .. "src/mame/drivers/dblewing.c",
	MAME_DIR .. "src/mame/drivers/dec0.c",
	MAME_DIR .. "src/mame/machine/dec0.c",
	MAME_DIR .. "src/mame/video/dec0.c",
	MAME_DIR .. "src/mame/drivers/dec8.c",
	MAME_DIR .. "src/mame/video/dec8.c",
	MAME_DIR .. "src/mame/machine/deco222.c",
	MAME_DIR .. "src/mame/machine/decocpu7.c",
	MAME_DIR .. "src/mame/machine/decocpu6.c",
	MAME_DIR .. "src/mame/drivers/deco_ld.c",
	MAME_DIR .. "src/mame/drivers/deco_mlc.c",
	MAME_DIR .. "src/mame/video/deco_mlc.c",
	MAME_DIR .. "src/mame/drivers/deco156.c",
	MAME_DIR .. "src/mame/machine/deco156.c",
	MAME_DIR .. "src/mame/drivers/deco32.c",
	MAME_DIR .. "src/mame/video/deco32.c",
	MAME_DIR .. "src/mame/video/dvi.c",
	MAME_DIR .. "src/mame/video/deco_zoomspr.c",
	MAME_DIR .. "src/mame/drivers/decocass.c",
	MAME_DIR .. "src/mame/machine/decocass.c",
	MAME_DIR .. "src/mame/machine/decocass_tape.c",
	MAME_DIR .. "src/mame/video/decocass.c",
	MAME_DIR .. "src/mame/drivers/deshoros.c",
	MAME_DIR .. "src/mame/drivers/dietgo.c",
	MAME_DIR .. "src/mame/video/dietgo.c",
	MAME_DIR .. "src/mame/drivers/dreambal.c",
	MAME_DIR .. "src/mame/drivers/exprraid.c",
	MAME_DIR .. "src/mame/video/exprraid.c",
	MAME_DIR .. "src/mame/drivers/firetrap.c",
	MAME_DIR .. "src/mame/video/firetrap.c",
	MAME_DIR .. "src/mame/drivers/funkyjet.c",
	MAME_DIR .. "src/mame/video/funkyjet.c",
	MAME_DIR .. "src/mame/drivers/karnov.c",
	MAME_DIR .. "src/mame/video/karnov.c",
	MAME_DIR .. "src/mame/drivers/kchamp.c",
	MAME_DIR .. "src/mame/video/kchamp.c",
	MAME_DIR .. "src/mame/drivers/kingobox.c",
	MAME_DIR .. "src/mame/video/kingobox.c",
	MAME_DIR .. "src/mame/drivers/lemmings.c",
	MAME_DIR .. "src/mame/video/lemmings.c",
	MAME_DIR .. "src/mame/drivers/liberate.c",
	MAME_DIR .. "src/mame/video/liberate.c",
	MAME_DIR .. "src/mame/drivers/madalien.c",
	MAME_DIR .. "src/mame/audio/madalien.c",
	MAME_DIR .. "src/mame/video/madalien.c",
	MAME_DIR .. "src/mame/drivers/madmotor.c",
	MAME_DIR .. "src/mame/video/madmotor.c",
	MAME_DIR .. "src/mame/drivers/metlclsh.c",
	MAME_DIR .. "src/mame/video/metlclsh.c",
	MAME_DIR .. "src/mame/drivers/mirage.c",
	MAME_DIR .. "src/mame/drivers/pcktgal.c",
	MAME_DIR .. "src/mame/video/pcktgal.c",
	MAME_DIR .. "src/mame/drivers/pktgaldx.c",
	MAME_DIR .. "src/mame/video/pktgaldx.c",
	MAME_DIR .. "src/mame/drivers/progolf.c",
	MAME_DIR .. "src/mame/drivers/rohga.c",
	MAME_DIR .. "src/mame/video/rohga.c",
	MAME_DIR .. "src/mame/drivers/shootout.c",
	MAME_DIR .. "src/mame/video/shootout.c",
	MAME_DIR .. "src/mame/drivers/sidepckt.c",
	MAME_DIR .. "src/mame/video/sidepckt.c",
	MAME_DIR .. "src/mame/drivers/simpl156.c",
	MAME_DIR .. "src/mame/video/simpl156.c",
	MAME_DIR .. "src/mame/drivers/sshangha.c",
	MAME_DIR .. "src/mame/video/sshangha.c",
	MAME_DIR .. "src/mame/drivers/stadhero.c",
	MAME_DIR .. "src/mame/video/stadhero.c",
	MAME_DIR .. "src/mame/drivers/supbtime.c",
	MAME_DIR .. "src/mame/video/supbtime.c",
	MAME_DIR .. "src/mame/drivers/tryout.c",
	MAME_DIR .. "src/mame/video/tryout.c",
	MAME_DIR .. "src/mame/drivers/tumbleb.c",
	MAME_DIR .. "src/mame/video/tumbleb.c",
	MAME_DIR .. "src/mame/drivers/tumblep.c",
	MAME_DIR .. "src/mame/video/tumblep.c",
	MAME_DIR .. "src/mame/drivers/vaportra.c",
	MAME_DIR .. "src/mame/video/vaportra.c",
	MAME_DIR .. "src/mame/machine/deco102.c",
	MAME_DIR .. "src/mame/machine/decocrpt.c",
	MAME_DIR .. "src/mame/machine/deco104.c",
	MAME_DIR .. "src/mame/machine/deco146.c",
	MAME_DIR .. "src/mame/video/decbac06.c",
	MAME_DIR .. "src/mame/video/deco16ic.c",
	MAME_DIR .. "src/mame/video/decocomn.c",
	MAME_DIR .. "src/mame/video/decospr.c",
	MAME_DIR .. "src/mame/video/decmxc06.c",
	MAME_DIR .. "src/mame/video/deckarn.c",
}

createMAMEProjects(_target, _subtarget, "dgrm")
files {
	MAME_DIR .. "src/mame/drivers/blackt96.c",
	MAME_DIR .. "src/mame/drivers/pokechmp.c",
	MAME_DIR .. "src/mame/video/pokechmp.c",
}

createMAMEProjects(_target, _subtarget, "dooyong")
files {
	MAME_DIR .. "src/mame/drivers/dooyong.c",
	MAME_DIR .. "src/mame/video/dooyong.c",
	MAME_DIR .. "src/mame/drivers/gundealr.c",
	MAME_DIR .. "src/mame/video/gundealr.c",
}

createMAMEProjects(_target, _subtarget, "dynax")
files {
	MAME_DIR .. "src/mame/drivers/ddenlovr.c",
	MAME_DIR .. "src/mame/drivers/dynax.c",
	MAME_DIR .. "src/mame/video/dynax.c",
	MAME_DIR .. "src/mame/drivers/hnayayoi.c",
	MAME_DIR .. "src/mame/video/hnayayoi.c",
	MAME_DIR .. "src/mame/drivers/realbrk.c",
	MAME_DIR .. "src/mame/video/realbrk.c",
	MAME_DIR .. "src/mame/drivers/royalmah.c",
}

createMAMEProjects(_target, _subtarget, "edevices")
files {
	MAME_DIR .. "src/mame/drivers/diverboy.c",
	MAME_DIR .. "src/mame/drivers/fantland.c",
	MAME_DIR .. "src/mame/video/fantland.c",
	MAME_DIR .. "src/mame/drivers/mwarr.c",
	MAME_DIR .. "src/mame/drivers/mugsmash.c",
	MAME_DIR .. "src/mame/video/mugsmash.c",
	MAME_DIR .. "src/mame/drivers/ppmast93.c",
	MAME_DIR .. "src/mame/drivers/pzletime.c",
	MAME_DIR .. "src/mame/drivers/stlforce.c",
	MAME_DIR .. "src/mame/video/stlforce.c",
	MAME_DIR .. "src/mame/drivers/twins.c",
}

createMAMEProjects(_target, _subtarget, "eolith")
files {
	MAME_DIR .. "src/mame/drivers/eolith.c",
	MAME_DIR .. "src/mame/video/eolith.c",
	MAME_DIR .. "src/mame/drivers/eolith16.c",
	MAME_DIR .. "src/mame/drivers/eolithsp.c",
	MAME_DIR .. "src/mame/drivers/ghosteo.c",
	MAME_DIR .. "src/mame/drivers/vegaeo.c",
}

createMAMEProjects(_target, _subtarget, "excelent")
files {
	MAME_DIR .. "src/mame/drivers/aquarium.c",
	MAME_DIR .. "src/mame/video/aquarium.c",
	MAME_DIR .. "src/mame/drivers/d9final.c",
	MAME_DIR .. "src/mame/drivers/dblcrown.c",
	MAME_DIR .. "src/mame/drivers/gcpinbal.c",
	MAME_DIR .. "src/mame/video/gcpinbal.c",
	MAME_DIR .. "src/mame/video/excellent_spr.c",
	MAME_DIR .. "src/mame/drivers/lastbank.c",
}

createMAMEProjects(_target, _subtarget, "exidy")
files {
	MAME_DIR .. "src/mame/drivers/carpolo.c",
	MAME_DIR .. "src/mame/machine/carpolo.c",
	MAME_DIR .. "src/mame/video/carpolo.c",
	MAME_DIR .. "src/mame/drivers/circus.c",
	MAME_DIR .. "src/mame/audio/circus.c",
	MAME_DIR .. "src/mame/video/circus.c",
	MAME_DIR .. "src/mame/drivers/exidy.c",
	MAME_DIR .. "src/mame/audio/exidy.c",
	MAME_DIR .. "src/mame/video/exidy.c",
	MAME_DIR .. "src/mame/audio/targ.c",
	MAME_DIR .. "src/mame/drivers/exidy440.c",
	MAME_DIR .. "src/mame/audio/exidy440.c",
	MAME_DIR .. "src/mame/video/exidy440.c",
	MAME_DIR .. "src/mame/drivers/exidyttl.c",
	MAME_DIR .. "src/mame/drivers/maxaflex.c",
	MAME_DIR .. "src/mame/machine/atari.c",
	MAME_DIR .. "src/mame/video/atari.c",
	MAME_DIR .. "src/mame/video/antic.c",
	MAME_DIR .. "src/mame/video/gtia.c",
	MAME_DIR .. "src/mame/drivers/starfire.c",
	MAME_DIR .. "src/mame/video/starfire.c",
	MAME_DIR .. "src/mame/drivers/vertigo.c",
	MAME_DIR .. "src/mame/machine/vertigo.c",
	MAME_DIR .. "src/mame/video/vertigo.c",
	MAME_DIR .. "src/mame/drivers/victory.c",
	MAME_DIR .. "src/mame/video/victory.c",
}

createMAMEProjects(_target, _subtarget, "f32")
files {
	MAME_DIR .. "src/mame/drivers/crospang.c",
	MAME_DIR .. "src/mame/video/crospang.c",
	MAME_DIR .. "src/mame/drivers/silvmil.c",
	MAME_DIR .. "src/mame/drivers/f-32.c",
}

createMAMEProjects(_target, _subtarget, "funworld")
files {
	MAME_DIR .. "src/mame/drivers/4roses.c",
	MAME_DIR .. "src/mame/drivers/funworld.c",
	MAME_DIR .. "src/mame/video/funworld.c",
	MAME_DIR .. "src/mame/drivers/snookr10.c",
	MAME_DIR .. "src/mame/video/snookr10.c",
}

createMAMEProjects(_target, _subtarget, "fuuki")
files {
	MAME_DIR .. "src/mame/drivers/fuukifg2.c",
	MAME_DIR .. "src/mame/video/fuukifg2.c",
	MAME_DIR .. "src/mame/drivers/fuukifg3.c",
	MAME_DIR .. "src/mame/video/fuukifg3.c",
	MAME_DIR .. "src/mame/video/fuukifg.c",
}

createMAMEProjects(_target, _subtarget, "gaelco")
files {
	MAME_DIR .. "src/mame/drivers/atvtrack.c",
	MAME_DIR .. "src/mame/drivers/gaelco.c",
	MAME_DIR .. "src/mame/video/gaelco.c",
	MAME_DIR .. "src/mame/machine/gaelcrpt.c",
	MAME_DIR .. "src/mame/drivers/gaelco2.c",
	MAME_DIR .. "src/mame/machine/gaelco2.c",
	MAME_DIR .. "src/mame/video/gaelco2.c",
	MAME_DIR .. "src/mame/drivers/gaelco3d.c",
	MAME_DIR .. "src/mame/video/gaelco3d.c",
	MAME_DIR .. "src/mame/machine/gaelco3d.c",
	MAME_DIR .. "src/mame/drivers/glass.c",
	MAME_DIR .. "src/mame/video/glass.c",
	MAME_DIR .. "src/mame/drivers/mastboy.c",
	MAME_DIR .. "src/mame/drivers/rollext.c",
	MAME_DIR .. "src/mame/drivers/splash.c",
	MAME_DIR .. "src/mame/video/splash.c",
	MAME_DIR .. "src/mame/drivers/targeth.c",
	MAME_DIR .. "src/mame/video/targeth.c",
	MAME_DIR .. "src/mame/drivers/thoop2.c",
	MAME_DIR .. "src/mame/video/thoop2.c",
	MAME_DIR .. "src/mame/drivers/tokyocop.c",
	MAME_DIR .. "src/mame/drivers/wrally.c",
	MAME_DIR .. "src/mame/machine/wrally.c",
	MAME_DIR .. "src/mame/video/wrally.c",
	MAME_DIR .. "src/mame/drivers/xorworld.c",
	MAME_DIR .. "src/mame/video/xorworld.c",
}

createMAMEProjects(_target, _subtarget, "gameplan")
files {
	MAME_DIR .. "src/mame/drivers/enigma2.c",
	MAME_DIR .. "src/mame/drivers/gameplan.c",
	MAME_DIR .. "src/mame/video/gameplan.c",
	MAME_DIR .. "src/mame/drivers/toratora.c",
}

createMAMEProjects(_target, _subtarget, "gametron")
files {
	MAME_DIR .. "src/mame/drivers/gatron.c",
	MAME_DIR .. "src/mame/video/gatron.c",
	MAME_DIR .. "src/mame/drivers/gotya.c",
	MAME_DIR .. "src/mame/audio/gotya.c",
	MAME_DIR .. "src/mame/video/gotya.c",
	MAME_DIR .. "src/mame/drivers/sbugger.c",
	MAME_DIR .. "src/mame/video/sbugger.c",
}

createMAMEProjects(_target, _subtarget, "gottlieb")
files {
	MAME_DIR .. "src/mame/drivers/exterm.c",
	MAME_DIR .. "src/mame/video/exterm.c",
	MAME_DIR .. "src/mame/drivers/gottlieb.c",
	MAME_DIR .. "src/mame/audio/gottlieb.c",
	MAME_DIR .. "src/mame/video/gottlieb.c",
}

createMAMEProjects(_target, _subtarget, "ibmpc")
files {
	MAME_DIR .. "src/mame/drivers/calchase.c",
	MAME_DIR .. "src/mame/drivers/fruitpc.c",
	MAME_DIR .. "src/mame/drivers/pangofun.c",
	MAME_DIR .. "src/mame/drivers/pcat_dyn.c",
	MAME_DIR .. "src/mame/drivers/pcat_nit.c",
	MAME_DIR .. "src/mame/drivers/pcxt.c",
	MAME_DIR .. "src/mame/drivers/quakeat.c",
	MAME_DIR .. "src/mame/drivers/queen.c",
	MAME_DIR .. "src/mame/drivers/igspc.c",
}

createMAMEProjects(_target, _subtarget, "igs")
files {
	MAME_DIR .. "src/mame/drivers/cabaret.c",
	MAME_DIR .. "src/mame/drivers/ddz.c",
	MAME_DIR .. "src/mame/drivers/dunhuang.c",
	MAME_DIR .. "src/mame/drivers/goldstar.c",
	MAME_DIR .. "src/mame/video/goldstar.c",
	MAME_DIR .. "src/mame/drivers/jackie.c",
	MAME_DIR .. "src/mame/drivers/igspoker.c",
	MAME_DIR .. "src/mame/drivers/igs009.c",
	MAME_DIR .. "src/mame/drivers/igs011.c",
	MAME_DIR .. "src/mame/drivers/igs017.c",
	MAME_DIR .. "src/mame/drivers/igs_fear.c",
	MAME_DIR .. "src/mame/drivers/igs_m027.c",
	MAME_DIR .. "src/mame/drivers/igs_m036.c",
	MAME_DIR .. "src/mame/drivers/iqblock.c",
	MAME_DIR .. "src/mame/video/iqblock.c",
	MAME_DIR .. "src/mame/drivers/lordgun.c",
	MAME_DIR .. "src/mame/video/lordgun.c",
	MAME_DIR .. "src/mame/drivers/pgm.c",
	MAME_DIR .. "src/mame/video/pgm.c",
	MAME_DIR .. "src/mame/machine/pgmprot_igs027a_type1.c",
	MAME_DIR .. "src/mame/machine/pgmprot_igs027a_type2.c",
	MAME_DIR .. "src/mame/machine/pgmprot_igs027a_type3.c",
	MAME_DIR .. "src/mame/machine/pgmprot_igs025_igs012.c",
	MAME_DIR .. "src/mame/machine/pgmprot_igs025_igs022.c",
	MAME_DIR .. "src/mame/machine/pgmprot_igs025_igs028.c",
	MAME_DIR .. "src/mame/machine/pgmprot_orlegend.c",
	MAME_DIR .. "src/mame/drivers/pgm2.c",
	MAME_DIR .. "src/mame/drivers/spoker.c",
	MAME_DIR .. "src/mame/machine/igs036crypt.c",
	MAME_DIR .. "src/mame/machine/pgmcrypt.c",
	MAME_DIR .. "src/mame/machine/igs025.c",
	MAME_DIR .. "src/mame/machine/igs022.c",
	MAME_DIR .. "src/mame/machine/igs028.c",
}

createMAMEProjects(_target, _subtarget, "irem")
files {
	MAME_DIR .. "src/mame/drivers/m10.c",
	MAME_DIR .. "src/mame/video/m10.c",
	MAME_DIR .. "src/mame/drivers/m14.c",
	MAME_DIR .. "src/mame/drivers/m52.c",
	MAME_DIR .. "src/mame/video/m52.c",
	MAME_DIR .. "src/mame/drivers/m57.c",
	MAME_DIR .. "src/mame/video/m57.c",
	MAME_DIR .. "src/mame/drivers/m58.c",
	MAME_DIR .. "src/mame/video/m58.c",
	MAME_DIR .. "src/mame/drivers/m62.c",
	MAME_DIR .. "src/mame/video/m62.c",
	MAME_DIR .. "src/mame/drivers/m63.c",
	MAME_DIR .. "src/mame/drivers/m72.c",
	MAME_DIR .. "src/mame/audio/m72.c",
	MAME_DIR .. "src/mame/video/m72.c",
	MAME_DIR .. "src/mame/drivers/m90.c",
	MAME_DIR .. "src/mame/video/m90.c",
	MAME_DIR .. "src/mame/drivers/m92.c",
	MAME_DIR .. "src/mame/video/m92.c",
	MAME_DIR .. "src/mame/drivers/m107.c",
	MAME_DIR .. "src/mame/video/m107.c",
	MAME_DIR .. "src/mame/drivers/olibochu.c",
	MAME_DIR .. "src/mame/drivers/redalert.c",
	MAME_DIR .. "src/mame/audio/redalert.c",
	MAME_DIR .. "src/mame/video/redalert.c",
	MAME_DIR .. "src/mame/drivers/shisen.c",
	MAME_DIR .. "src/mame/video/shisen.c",
	MAME_DIR .. "src/mame/drivers/travrusa.c",
	MAME_DIR .. "src/mame/video/travrusa.c",
	MAME_DIR .. "src/mame/drivers/vigilant.c",
	MAME_DIR .. "src/mame/video/vigilant.c",
	MAME_DIR .. "src/mame/machine/irem_cpu.c",
	MAME_DIR .. "src/mame/audio/irem.c",
}

createMAMEProjects(_target, _subtarget, "itech")
files {
	MAME_DIR .. "src/mame/drivers/capbowl.c",
	MAME_DIR .. "src/mame/video/capbowl.c",
	MAME_DIR .. "src/mame/drivers/itech8.c",
	MAME_DIR .. "src/mame/machine/slikshot.c",
	MAME_DIR .. "src/mame/video/itech8.c",
	MAME_DIR .. "src/mame/drivers/itech32.c",
	MAME_DIR .. "src/mame/video/itech32.c",
	MAME_DIR .. "src/mame/drivers/iteagle.c",
	MAME_DIR .. "src/mame/machine/iteagle_fpga.c",
}

createMAMEProjects(_target, _subtarget, "jaleco")
files {
	MAME_DIR .. "src/mame/drivers/aeroboto.c",
	MAME_DIR .. "src/mame/video/aeroboto.c",
	MAME_DIR .. "src/mame/drivers/argus.c",
	MAME_DIR .. "src/mame/video/argus.c",
	MAME_DIR .. "src/mame/drivers/bestleag.c",
	MAME_DIR .. "src/mame/drivers/bigstrkb.c",
	MAME_DIR .. "src/mame/video/bigstrkb.c",
	MAME_DIR .. "src/mame/drivers/blueprnt.c",
	MAME_DIR .. "src/mame/video/blueprnt.c",
	MAME_DIR .. "src/mame/drivers/bnstars.c",
	MAME_DIR .. "src/mame/drivers/cischeat.c",
	MAME_DIR .. "src/mame/video/cischeat.c",
	MAME_DIR .. "src/mame/drivers/citycon.c",
	MAME_DIR .. "src/mame/video/citycon.c",
	MAME_DIR .. "src/mame/drivers/ddayjlc.c",
	MAME_DIR .. "src/mame/drivers/exerion.c",
	MAME_DIR .. "src/mame/video/exerion.c",
	MAME_DIR .. "src/mame/drivers/fcombat.c",
	MAME_DIR .. "src/mame/video/fcombat.c",
	MAME_DIR .. "src/mame/drivers/ginganin.c",
	MAME_DIR .. "src/mame/video/ginganin.c",
	MAME_DIR .. "src/mame/drivers/homerun.c",
	MAME_DIR .. "src/mame/video/homerun.c",
	MAME_DIR .. "src/mame/drivers/megasys1.c",
	MAME_DIR .. "src/mame/video/megasys1.c",
	MAME_DIR .. "src/mame/drivers/momoko.c",
	MAME_DIR .. "src/mame/video/momoko.c",
	MAME_DIR .. "src/mame/drivers/ms32.c",
	MAME_DIR .. "src/mame/video/ms32.c",
	MAME_DIR .. "src/mame/drivers/psychic5.c",
	MAME_DIR .. "src/mame/video/psychic5.c",
	MAME_DIR .. "src/mame/drivers/pturn.c",
	MAME_DIR .. "src/mame/drivers/skyfox.c",
	MAME_DIR .. "src/mame/video/skyfox.c",
	MAME_DIR .. "src/mame/drivers/tetrisp2.c",
	MAME_DIR .. "src/mame/video/tetrisp2.c",
	MAME_DIR .. "src/mame/machine/jalcrpt.c",
	MAME_DIR .. "src/mame/video/jalblend.c",
}

createMAMEProjects(_target, _subtarget, "jpm")
files {
	MAME_DIR .. "src/mame/drivers/guab.c",
	MAME_DIR .. "src/mame/drivers/jpmsys5.c",
	MAME_DIR .. "src/mame/drivers/jpmsys5sw.c",
	MAME_DIR .. "src/mame/drivers/jpmmps.c",
	MAME_DIR .. "src/mame/drivers/jpms80.c",
	MAME_DIR .. "src/mame/drivers/jpmsru.c",
	MAME_DIR .. "src/mame/drivers/jpmimpct.c",
	MAME_DIR .. "src/mame/video/jpmimpct.c",
	MAME_DIR .. "src/mame/drivers/jpmimpctsw.c",
	MAME_DIR .. "src/mame/drivers/pluto5.c",
	MAME_DIR .. "src/mame/drivers/jpmsys7.c",
	MAME_DIR .. "src/mame/video/awpvid.c",
	MAME_DIR .. "src/mame/machine/meters.c",
}

createMAMEProjects(_target, _subtarget, "kaneko")
files {
	MAME_DIR .. "src/mame/drivers/airbustr.c",
	MAME_DIR .. "src/mame/video/airbustr.c",
	MAME_DIR .. "src/mame/drivers/djboy.c",
	MAME_DIR .. "src/mame/video/djboy.c",
	MAME_DIR .. "src/mame/drivers/expro02.c",
	MAME_DIR .. "src/mame/drivers/galpanic.c",
	MAME_DIR .. "src/mame/video/galpanic.c",
	MAME_DIR .. "src/mame/drivers/galpani2.c",
	MAME_DIR .. "src/mame/video/galpani2.c",
	MAME_DIR .. "src/mame/drivers/galpani3.c",
	MAME_DIR .. "src/mame/video/kaneko_grap2.c",
	MAME_DIR .. "src/mame/drivers/hvyunit.c",
	MAME_DIR .. "src/mame/drivers/jchan.c",
	MAME_DIR .. "src/mame/drivers/kaneko16.c",
	MAME_DIR .. "src/mame/video/kaneko16.c",
	MAME_DIR .. "src/mame/video/kaneko_tmap.c",
	MAME_DIR .. "src/mame/video/kaneko_spr.c",
	MAME_DIR .. "src/mame/machine/kaneko_hit.c",
	MAME_DIR .. "src/mame/machine/kaneko_calc3.c",
	MAME_DIR .. "src/mame/machine/kaneko_toybox.c",
	MAME_DIR .. "src/mame/drivers/sandscrp.c",
	MAME_DIR .. "src/mame/drivers/suprnova.c",
	MAME_DIR .. "src/mame/video/suprnova.c",
	MAME_DIR .. "src/mame/video/sknsspr.c",
}

createMAMEProjects(_target, _subtarget, "konami")
files {
	MAME_DIR .. "src/mame/drivers/88games.c",
	MAME_DIR .. "src/mame/video/88games.c",
	MAME_DIR .. "src/mame/drivers/ajax.c",
	MAME_DIR .. "src/mame/machine/ajax.c",
	MAME_DIR .. "src/mame/video/ajax.c",
	MAME_DIR .. "src/mame/drivers/aliens.c",
	MAME_DIR .. "src/mame/video/aliens.c",
	MAME_DIR .. "src/mame/drivers/asterix.c",
	MAME_DIR .. "src/mame/video/asterix.c",
	MAME_DIR .. "src/mame/drivers/battlnts.c",
	MAME_DIR .. "src/mame/video/battlnts.c",
	MAME_DIR .. "src/mame/drivers/bishi.c",
	MAME_DIR .. "src/mame/video/bishi.c",
	MAME_DIR .. "src/mame/drivers/bladestl.c",
	MAME_DIR .. "src/mame/video/bladestl.c",
	MAME_DIR .. "src/mame/drivers/blockhl.c",
	MAME_DIR .. "src/mame/drivers/bottom9.c",
	MAME_DIR .. "src/mame/video/bottom9.c",
	MAME_DIR .. "src/mame/drivers/chqflag.c",
	MAME_DIR .. "src/mame/video/chqflag.c",
	MAME_DIR .. "src/mame/drivers/circusc.c",
	MAME_DIR .. "src/mame/video/circusc.c",
	MAME_DIR .. "src/mame/drivers/cobra.c",
	MAME_DIR .. "src/mame/drivers/combatsc.c",
	MAME_DIR .. "src/mame/video/combatsc.c",
	MAME_DIR .. "src/mame/drivers/contra.c",
	MAME_DIR .. "src/mame/video/contra.c",
	MAME_DIR .. "src/mame/drivers/crimfght.c",
	MAME_DIR .. "src/mame/video/crimfght.c",
	MAME_DIR .. "src/mame/drivers/dbz.c",
	MAME_DIR .. "src/mame/video/dbz.c",
	MAME_DIR .. "src/mame/drivers/ddribble.c",
	MAME_DIR .. "src/mame/video/ddribble.c",
	MAME_DIR .. "src/mame/drivers/djmain.c",
	MAME_DIR .. "src/mame/video/djmain.c",
	MAME_DIR .. "src/mame/drivers/fastfred.c",
	MAME_DIR .. "src/mame/video/fastfred.c",
	MAME_DIR .. "src/mame/drivers/fastlane.c",
	MAME_DIR .. "src/mame/video/fastlane.c",
	MAME_DIR .. "src/mame/drivers/finalizr.c",
	MAME_DIR .. "src/mame/video/finalizr.c",
	MAME_DIR .. "src/mame/drivers/firebeat.c",
	MAME_DIR .. "src/mame/machine/midikbd.c",
	MAME_DIR .. "src/mame/drivers/flkatck.c",
	MAME_DIR .. "src/mame/video/flkatck.c",
	MAME_DIR .. "src/mame/drivers/gberet.c",
	MAME_DIR .. "src/mame/video/gberet.c",
	MAME_DIR .. "src/mame/drivers/gijoe.c",
	MAME_DIR .. "src/mame/video/gijoe.c",
	MAME_DIR .. "src/mame/drivers/gradius3.c",
	MAME_DIR .. "src/mame/video/gradius3.c",
	MAME_DIR .. "src/mame/drivers/gticlub.c",
	MAME_DIR .. "src/mame/drivers/gyruss.c",
	MAME_DIR .. "src/mame/video/gyruss.c",
	MAME_DIR .. "src/mame/drivers/hcastle.c",
	MAME_DIR .. "src/mame/video/hcastle.c",
	MAME_DIR .. "src/mame/drivers/hexion.c",
	MAME_DIR .. "src/mame/video/hexion.c",
	MAME_DIR .. "src/mame/drivers/hornet.c",
	MAME_DIR .. "src/mame/machine/konppc.c",
	MAME_DIR .. "src/mame/drivers/hyperspt.c",
	MAME_DIR .. "src/mame/audio/hyprolyb.c",
	MAME_DIR .. "src/mame/video/hyperspt.c",
	MAME_DIR .. "src/mame/drivers/ironhors.c",
	MAME_DIR .. "src/mame/video/ironhors.c",
	MAME_DIR .. "src/mame/drivers/jackal.c",
	MAME_DIR .. "src/mame/video/jackal.c",
	MAME_DIR .. "src/mame/drivers/jailbrek.c",
	MAME_DIR .. "src/mame/video/jailbrek.c",
	MAME_DIR .. "src/mame/drivers/junofrst.c",
	MAME_DIR .. "src/mame/drivers/konamigq.c",
	MAME_DIR .. "src/mame/drivers/konamigv.c",
	MAME_DIR .. "src/mame/drivers/konamigx.c",
	MAME_DIR .. "src/mame/machine/konamigx.c",
	MAME_DIR .. "src/mame/video/konamigx.c",
	MAME_DIR .. "src/mame/drivers/konamim2.c",
	MAME_DIR .. "src/mame/drivers/kontest.c",
	MAME_DIR .. "src/mame/drivers/konendev.c",
	MAME_DIR .. "src/mame/drivers/ksys573.c",
	MAME_DIR .. "src/mame/machine/k573cass.c",
	MAME_DIR .. "src/mame/machine/k573dio.c",
	MAME_DIR .. "src/mame/machine/k573mcr.c",
	MAME_DIR .. "src/mame/machine/k573msu.c",
	MAME_DIR .. "src/mame/machine/k573npu.c",
	MAME_DIR .. "src/mame/machine/zs01.c",
	MAME_DIR .. "src/mame/drivers/labyrunr.c",
	MAME_DIR .. "src/mame/video/labyrunr.c",
	MAME_DIR .. "src/mame/drivers/lethal.c",
	MAME_DIR .. "src/mame/video/lethal.c",
	MAME_DIR .. "src/mame/drivers/mainevt.c",
	MAME_DIR .. "src/mame/video/mainevt.c",
	MAME_DIR .. "src/mame/drivers/megazone.c",
	MAME_DIR .. "src/mame/video/megazone.c",
	MAME_DIR .. "src/mame/drivers/mikie.c",
	MAME_DIR .. "src/mame/video/mikie.c",
	MAME_DIR .. "src/mame/drivers/mogura.c",
	MAME_DIR .. "src/mame/drivers/moo.c",
	MAME_DIR .. "src/mame/video/moo.c",
	MAME_DIR .. "src/mame/drivers/mystwarr.c",
	MAME_DIR .. "src/mame/video/mystwarr.c",
	MAME_DIR .. "src/mame/drivers/nemesis.c",
	MAME_DIR .. "src/mame/video/nemesis.c",
	MAME_DIR .. "src/mame/drivers/nwk-tr.c",
	MAME_DIR .. "src/mame/drivers/overdriv.c",
	MAME_DIR .. "src/mame/video/overdriv.c",
	MAME_DIR .. "src/mame/drivers/pandoras.c",
	MAME_DIR .. "src/mame/video/pandoras.c",
	MAME_DIR .. "src/mame/drivers/parodius.c",
	MAME_DIR .. "src/mame/video/parodius.c",
	MAME_DIR .. "src/mame/drivers/pingpong.c",
	MAME_DIR .. "src/mame/video/pingpong.c",
	MAME_DIR .. "src/mame/drivers/plygonet.c",
	MAME_DIR .. "src/mame/video/plygonet.c",
	MAME_DIR .. "src/mame/drivers/pooyan.c",
	MAME_DIR .. "src/mame/video/pooyan.c",
	MAME_DIR .. "src/mame/drivers/pyson.c",
	MAME_DIR .. "src/mame/drivers/qdrmfgp.c",
	MAME_DIR .. "src/mame/video/qdrmfgp.c",
	MAME_DIR .. "src/mame/drivers/rockrage.c",
	MAME_DIR .. "src/mame/video/rockrage.c",
	MAME_DIR .. "src/mame/drivers/rocnrope.c",
	MAME_DIR .. "src/mame/video/rocnrope.c",
	MAME_DIR .. "src/mame/drivers/rollerg.c",
	MAME_DIR .. "src/mame/video/rollerg.c",
	MAME_DIR .. "src/mame/drivers/rungun.c",
	MAME_DIR .. "src/mame/video/rungun.c",
	MAME_DIR .. "src/mame/drivers/sbasketb.c",
	MAME_DIR .. "src/mame/video/sbasketb.c",
	MAME_DIR .. "src/mame/drivers/scobra.c",
	MAME_DIR .. "src/mame/drivers/scotrsht.c",
	MAME_DIR .. "src/mame/video/scotrsht.c",
	MAME_DIR .. "src/mame/drivers/scramble.c",
	MAME_DIR .. "src/mame/machine/scramble.c",
	MAME_DIR .. "src/mame/audio/scramble.c",
	MAME_DIR .. "src/mame/drivers/shaolins.c",
	MAME_DIR .. "src/mame/video/shaolins.c",
	MAME_DIR .. "src/mame/drivers/simpsons.c",
	MAME_DIR .. "src/mame/machine/simpsons.c",
	MAME_DIR .. "src/mame/video/simpsons.c",
	MAME_DIR .. "src/mame/drivers/spy.c",
	MAME_DIR .. "src/mame/video/spy.c",
	MAME_DIR .. "src/mame/drivers/surpratk.c",
	MAME_DIR .. "src/mame/video/surpratk.c",
	MAME_DIR .. "src/mame/drivers/tasman.c",
	MAME_DIR .. "src/mame/drivers/tgtpanic.c",
	MAME_DIR .. "src/mame/drivers/thunderx.c",
	MAME_DIR .. "src/mame/video/thunderx.c",
	MAME_DIR .. "src/mame/drivers/timeplt.c",
	MAME_DIR .. "src/mame/audio/timeplt.c",
	MAME_DIR .. "src/mame/video/timeplt.c",
	MAME_DIR .. "src/mame/drivers/tmnt.c",
	MAME_DIR .. "src/mame/video/tmnt.c",
	MAME_DIR .. "src/mame/drivers/tp84.c",
	MAME_DIR .. "src/mame/video/tp84.c",
	MAME_DIR .. "src/mame/drivers/trackfld.c",
	MAME_DIR .. "src/mame/machine/konami1.c",
	MAME_DIR .. "src/mame/audio/trackfld.c",
	MAME_DIR .. "src/mame/video/trackfld.c",
	MAME_DIR .. "src/mame/drivers/tutankhm.c",
	MAME_DIR .. "src/mame/video/tutankhm.c",
	MAME_DIR .. "src/mame/drivers/twin16.c",
	MAME_DIR .. "src/mame/video/twin16.c",
	MAME_DIR .. "src/mame/drivers/twinkle.c",
	MAME_DIR .. "src/mame/drivers/ultrsprt.c",
	MAME_DIR .. "src/mame/drivers/ultraman.c",
	MAME_DIR .. "src/mame/video/ultraman.c",
	MAME_DIR .. "src/mame/drivers/vendetta.c",
	MAME_DIR .. "src/mame/video/vendetta.c",
	MAME_DIR .. "src/mame/drivers/viper.c",
	MAME_DIR .. "src/mame/drivers/wecleman.c",
	MAME_DIR .. "src/mame/video/wecleman.c",
	MAME_DIR .. "src/mame/drivers/xexex.c",
	MAME_DIR .. "src/mame/video/xexex.c",
	MAME_DIR .. "src/mame/drivers/xmen.c",
	MAME_DIR .. "src/mame/video/xmen.c",
	MAME_DIR .. "src/mame/drivers/yiear.c",
	MAME_DIR .. "src/mame/video/yiear.c",
	MAME_DIR .. "src/mame/drivers/zr107.c",
	MAME_DIR .. "src/mame/video/konami_helper.c",
	MAME_DIR .. "src/mame/video/k007121.c",
	MAME_DIR .. "src/mame/video/k007342.c",
	MAME_DIR .. "src/mame/video/k007420.c",
	MAME_DIR .. "src/mame/video/k037122.c",
	MAME_DIR .. "src/mame/video/k051316.c",
	MAME_DIR .. "src/mame/video/k051733.c",
	MAME_DIR .. "src/mame/video/k051960.c",
	MAME_DIR .. "src/mame/video/k052109.c",
	MAME_DIR .. "src/mame/video/k053250.c",
	MAME_DIR .. "src/mame/video/k053251.c",
	MAME_DIR .. "src/mame/video/k054156_k054157_k056832.c",
	MAME_DIR .. "src/mame/video/k053244_k053245.c",
	MAME_DIR .. "src/mame/video/k053246_k053247_k055673.c",
	MAME_DIR .. "src/mame/video/k055555.c",
	MAME_DIR .. "src/mame/video/k054000.c",
	MAME_DIR .. "src/mame/video/k054338.c",
	MAME_DIR .. "src/mame/video/k053936.c",
	MAME_DIR .. "src/mame/video/k001006.c",
	MAME_DIR .. "src/mame/video/k001005.c",
	MAME_DIR .. "src/mame/video/k001604.c",
	MAME_DIR .. "src/mame/video/k057714.c",
}

createMAMEProjects(_target, _subtarget, "matic")
files {
	MAME_DIR .. "src/mame/drivers/barata.c",
}

createMAMEProjects(_target, _subtarget, "maygay")
files {
	MAME_DIR .. "src/mame/drivers/maygay1b.c",
	MAME_DIR .. "src/mame/drivers/maygay1bsw.c",
	MAME_DIR .. "src/mame/drivers/maygayv1.c",
	MAME_DIR .. "src/mame/drivers/maygayep.c",
	MAME_DIR .. "src/mame/drivers/maygaysw.c",
	MAME_DIR .. "src/mame/drivers/mmm.c",
}

createMAMEProjects(_target, _subtarget, "meadows")
files {
	MAME_DIR .. "src/mame/drivers/lazercmd.c",
	MAME_DIR .. "src/mame/video/lazercmd.c",
	MAME_DIR .. "src/mame/drivers/meadwttl.c",
	MAME_DIR .. "src/mame/drivers/meadows.c",
	MAME_DIR .. "src/mame/audio/meadows.c",
	MAME_DIR .. "src/mame/video/meadows.c",
	MAME_DIR .. "src/mame/drivers/warpsped.c",
}

createMAMEProjects(_target, _subtarget, "merit")
files {
	MAME_DIR .. "src/mame/drivers/mgames.c",
	MAME_DIR .. "src/mame/drivers/merit.c",
	MAME_DIR .. "src/mame/drivers/meritm.c",
}

createMAMEProjects(_target, _subtarget, "metro")
files {
	MAME_DIR .. "src/mame/drivers/hyprduel.c",
	MAME_DIR .. "src/mame/video/hyprduel.c",
	MAME_DIR .. "src/mame/drivers/metro.c",
	MAME_DIR .. "src/mame/video/metro.c",
	MAME_DIR .. "src/mame/drivers/rabbit.c",
	MAME_DIR .. "src/mame/drivers/tmmjprd.c",
}

createMAMEProjects(_target, _subtarget, "midcoin")
files {
	MAME_DIR .. "src/mame/drivers/wallc.c",
	MAME_DIR .. "src/mame/drivers/wink.c",
	MAME_DIR .. "src/mame/drivers/24cdjuke.c",
}

createMAMEProjects(_target, _subtarget, "midw8080")
files {
	MAME_DIR .. "src/mame/drivers/8080bw.c",
	MAME_DIR .. "src/mame/audio/8080bw.c",
	MAME_DIR .. "src/mame/video/8080bw.c",
	MAME_DIR .. "src/mame/drivers/m79amb.c",
	MAME_DIR .. "src/mame/audio/m79amb.c",
	MAME_DIR .. "src/mame/drivers/mw8080bw.c",
	MAME_DIR .. "src/mame/machine/mw8080bw.c",
	MAME_DIR .. "src/mame/audio/mw8080bw.c",
	MAME_DIR .. "src/mame/video/mw8080bw.c",
	MAME_DIR .. "src/mame/drivers/rotaryf.c",
}

createMAMEProjects(_target, _subtarget, "midway")
files {
	MAME_DIR .. "src/mame/drivers/astrocde.c",
	MAME_DIR .. "src/mame/video/astrocde.c",
	MAME_DIR .. "src/mame/audio/gorf.c",
	MAME_DIR .. "src/mame/audio/wow.c",
	MAME_DIR .. "src/mame/drivers/atlantis.c",
	MAME_DIR .. "src/mame/drivers/balsente.c",
	MAME_DIR .. "src/mame/machine/balsente.c",
	MAME_DIR .. "src/mame/video/balsente.c",
	MAME_DIR .. "src/mame/drivers/gridlee.c",
	MAME_DIR .. "src/mame/audio/gridlee.c",
	MAME_DIR .. "src/mame/video/gridlee.c",
	MAME_DIR .. "src/mame/drivers/mcr.c",
	MAME_DIR .. "src/mame/machine/mcr.c",
	MAME_DIR .. "src/mame/video/mcr.c",
	MAME_DIR .. "src/mame/drivers/mcr3.c",
	MAME_DIR .. "src/mame/video/mcr3.c",
	MAME_DIR .. "src/mame/drivers/mcr68.c",
	MAME_DIR .. "src/mame/machine/mcr68.c",
	MAME_DIR .. "src/mame/video/mcr68.c",
	MAME_DIR .. "src/mame/drivers/midqslvr.c",
	MAME_DIR .. "src/mame/drivers/midtunit.c",
	MAME_DIR .. "src/mame/machine/midtunit.c",
	MAME_DIR .. "src/mame/video/midtunit.c",
	MAME_DIR .. "src/mame/drivers/midvunit.c",
	MAME_DIR .. "src/mame/video/midvunit.c",
	MAME_DIR .. "src/mame/drivers/midwunit.c",
	MAME_DIR .. "src/mame/machine/midwunit.c",
	MAME_DIR .. "src/mame/drivers/midxunit.c",
	MAME_DIR .. "src/mame/machine/midxunit.c",
	MAME_DIR .. "src/mame/drivers/midyunit.c",
	MAME_DIR .. "src/mame/machine/midyunit.c",
	MAME_DIR .. "src/mame/video/midyunit.c",
	MAME_DIR .. "src/mame/drivers/midzeus.c",
	MAME_DIR .. "src/mame/video/midzeus.c",
	MAME_DIR .. "src/mame/video/midzeus2.c",
	MAME_DIR .. "src/mame/drivers/mw18w.c",
	MAME_DIR .. "src/mame/drivers/mwsub.c",
	MAME_DIR .. "src/mame/drivers/omegrace.c",
	MAME_DIR .. "src/mame/drivers/pinball2k.c",
	MAME_DIR .. "src/mame/drivers/seattle.c",
	MAME_DIR .. "src/mame/drivers/sspeedr.c",
	MAME_DIR .. "src/mame/video/sspeedr.c",
	MAME_DIR .. "src/mame/drivers/tmaster.c",
	MAME_DIR .. "src/mame/drivers/vegas.c",
	MAME_DIR .. "src/mame/drivers/wmg.c",
	MAME_DIR .. "src/mame/drivers/williams.c",
	MAME_DIR .. "src/mame/machine/williams.c",
	MAME_DIR .. "src/mame/audio/williams.c",
	MAME_DIR .. "src/mame/video/williams.c",
	MAME_DIR .. "src/mame/machine/midwayic.c",
	MAME_DIR .. "src/mame/audio/midway.c",
}

createMAMEProjects(_target, _subtarget, "namco")
files {
	MAME_DIR .. "src/mame/drivers/20pacgal.c",
	MAME_DIR .. "src/mame/video/20pacgal.c",
	MAME_DIR .. "src/mame/drivers/30test.c",
	MAME_DIR .. "src/mame/drivers/baraduke.c",
	MAME_DIR .. "src/mame/video/baraduke.c",
	MAME_DIR .. "src/mame/drivers/cswat.c",
	MAME_DIR .. "src/mame/drivers/dambustr.c",
	MAME_DIR .. "src/mame/drivers/gal3.c",
	MAME_DIR .. "src/mame/drivers/galaga.c",
	MAME_DIR .. "src/mame/audio/galaga.c",
	MAME_DIR .. "src/mame/video/galaga.c",
	MAME_DIR .. "src/mame/video/bosco.c",
	MAME_DIR .. "src/mame/video/digdug.c",
	MAME_DIR .. "src/mame/machine/xevious.c",
	MAME_DIR .. "src/mame/video/xevious.c",
	MAME_DIR .. "src/mame/drivers/galaxian.c",
	MAME_DIR .. "src/mame/audio/galaxian.c",
	MAME_DIR .. "src/mame/video/galaxian.c",
	MAME_DIR .. "src/mame/drivers/galaxold.c",
	MAME_DIR .. "src/mame/machine/galaxold.c",
	MAME_DIR .. "src/mame/video/galaxold.c",
	MAME_DIR .. "src/mame/drivers/gaplus.c",
	MAME_DIR .. "src/mame/machine/gaplus.c",
	MAME_DIR .. "src/mame/video/gaplus.c",
	MAME_DIR .. "src/mame/drivers/kungfur.c",
	MAME_DIR .. "src/mame/drivers/mappy.c",
	MAME_DIR .. "src/mame/video/mappy.c",
	MAME_DIR .. "src/mame/drivers/namcofl.c",
	MAME_DIR .. "src/mame/video/namcofl.c",
	MAME_DIR .. "src/mame/drivers/namcoic.c",
	MAME_DIR .. "src/mame/drivers/namcona1.c",
	MAME_DIR .. "src/mame/video/namcona1.c",
	MAME_DIR .. "src/mame/drivers/namconb1.c",
	MAME_DIR .. "src/mame/video/namconb1.c",
	MAME_DIR .. "src/mame/drivers/namcond1.c",
	MAME_DIR .. "src/mame/machine/namcond1.c",
	MAME_DIR .. "src/mame/video/ygv608.c",
	MAME_DIR .. "src/mame/drivers/namcops2.c",
	MAME_DIR .. "src/mame/drivers/namcos1.c",
	MAME_DIR .. "src/mame/machine/namcos1.c",
	MAME_DIR .. "src/mame/video/namcos1.c",
	MAME_DIR .. "src/mame/drivers/namcos10.c",
	MAME_DIR .. "src/mame/machine/ns10crypt.c",
	MAME_DIR .. "src/mame/drivers/namcos11.c",
	MAME_DIR .. "src/mame/machine/ns11prot.c",
	MAME_DIR .. "src/mame/drivers/namcos12.c",
	MAME_DIR .. "src/mame/machine/namco_settings.c",
	MAME_DIR .. "src/mame/drivers/namcos2.c",
	MAME_DIR .. "src/mame/machine/namcos2.c",
	MAME_DIR .. "src/mame/video/namcos2.c",
	MAME_DIR .. "src/mame/drivers/namcos21.c",
	MAME_DIR .. "src/mame/video/namcos21.c",
	MAME_DIR .. "src/mame/drivers/namcos22.c",
	MAME_DIR .. "src/mame/video/namcos22.c",
	MAME_DIR .. "src/mame/drivers/namcos23.c",
	MAME_DIR .. "src/mame/drivers/namcos86.c",
	MAME_DIR .. "src/mame/video/namcos86.c",
	MAME_DIR .. "src/mame/drivers/pacland.c",
	MAME_DIR .. "src/mame/video/pacland.c",
	MAME_DIR .. "src/mame/drivers/polepos.c",
	MAME_DIR .. "src/mame/audio/polepos.c",
	MAME_DIR .. "src/mame/video/polepos.c",
	MAME_DIR .. "src/mame/drivers/rallyx.c",
	MAME_DIR .. "src/mame/video/rallyx.c",
	MAME_DIR .. "src/mame/drivers/skykid.c",
	MAME_DIR .. "src/mame/video/skykid.c",
	MAME_DIR .. "src/mame/drivers/tankbatt.c",
	MAME_DIR .. "src/mame/video/tankbatt.c",
	MAME_DIR .. "src/mame/drivers/tceptor.c",
	MAME_DIR .. "src/mame/video/tceptor.c",
	MAME_DIR .. "src/mame/drivers/toypop.c",
	MAME_DIR .. "src/mame/video/toypop.c",
	MAME_DIR .. "src/mame/drivers/turrett.c",
	MAME_DIR .. "src/mame/audio/turrett.c",
	MAME_DIR .. "src/mame/video/turrett.c",
	MAME_DIR .. "src/mame/drivers/warpwarp.c",
	MAME_DIR .. "src/mame/audio/geebee.c",
	MAME_DIR .. "src/mame/audio/warpwarp.c",
	MAME_DIR .. "src/mame/video/warpwarp.c",
	MAME_DIR .. "src/mame/machine/c117.c",
	MAME_DIR .. "src/mame/machine/namcoio.c",
	MAME_DIR .. "src/mame/machine/namco06.c",
	MAME_DIR .. "src/mame/machine/namco50.c",
	MAME_DIR .. "src/mame/machine/namco51.c",
	MAME_DIR .. "src/mame/machine/namco53.c",
	MAME_DIR .. "src/mame/machine/namco62.c",
	MAME_DIR .. "src/mame/machine/namcomcu.c",
	MAME_DIR .. "src/mame/audio/namco52.c",
	MAME_DIR .. "src/mame/audio/namco54.c",
	MAME_DIR .. "src/mame/video/c116.c",
	MAME_DIR .. "src/mame/video/c45.c",
}

createMAMEProjects(_target, _subtarget, "nasco")
files {
	MAME_DIR .. "src/mame/drivers/crgolf.c",
	MAME_DIR .. "src/mame/video/crgolf.c",
	MAME_DIR .. "src/mame/drivers/suprgolf.c",
}

createMAMEProjects(_target, _subtarget, "neogeo")
files {
	MAME_DIR .. "src/mame/drivers/neogeo.c",
	MAME_DIR .. "src/mame/video/neogeo.c",
	MAME_DIR .. "src/mame/drivers/neogeo_noslot.c",
	MAME_DIR .. "src/mame/video/neogeo_spr.c",
	MAME_DIR .. "src/mame/machine/neocrypt.c",
	MAME_DIR .. "src/mame/machine/ng_memcard.c",
}

createMAMEProjects(_target, _subtarget, "nichibut")
files {
	MAME_DIR .. "src/mame/drivers/armedf.c",
	MAME_DIR .. "src/mame/video/armedf.c",
	MAME_DIR .. "src/mame/drivers/cclimber.c",
	MAME_DIR .. "src/mame/machine/cclimber.c",
	MAME_DIR .. "src/mame/audio/cclimber.c",
	MAME_DIR .. "src/mame/video/cclimber.c",
	MAME_DIR .. "src/mame/drivers/clshroad.c",
	MAME_DIR .. "src/mame/video/clshroad.c",
	MAME_DIR .. "src/mame/drivers/csplayh5.c",
	MAME_DIR .. "src/mame/drivers/cop01.c",
	MAME_DIR .. "src/mame/video/cop01.c",
	MAME_DIR .. "src/mame/drivers/dacholer.c",
	MAME_DIR .. "src/mame/drivers/galivan.c",
	MAME_DIR .. "src/mame/video/galivan.c",
	MAME_DIR .. "src/mame/drivers/gomoku.c",
	MAME_DIR .. "src/mame/audio/gomoku.c",
	MAME_DIR .. "src/mame/video/gomoku.c",
	MAME_DIR .. "src/mame/drivers/hyhoo.c",
	MAME_DIR .. "src/mame/video/hyhoo.c",
	MAME_DIR .. "src/mame/drivers/jangou.c",
	MAME_DIR .. "src/mame/drivers/magmax.c",
	MAME_DIR .. "src/mame/video/magmax.c",
	MAME_DIR .. "src/mame/drivers/nbmj8688.c",
	MAME_DIR .. "src/mame/video/nbmj8688.c",
	MAME_DIR .. "src/mame/drivers/nbmj8891.c",
	MAME_DIR .. "src/mame/video/nbmj8891.c",
	MAME_DIR .. "src/mame/drivers/nbmj8900.c",
	MAME_DIR .. "src/mame/video/nbmj8900.c",
	MAME_DIR .. "src/mame/drivers/nbmj8991.c",
	MAME_DIR .. "src/mame/video/nbmj8991.c",
	MAME_DIR .. "src/mame/drivers/nbmj9195.c",
	MAME_DIR .. "src/mame/video/nbmj9195.c",
	MAME_DIR .. "src/mame/drivers/nightgal.c",
	MAME_DIR .. "src/mame/drivers/niyanpai.c",
	MAME_DIR .. "src/mame/video/niyanpai.c",
	MAME_DIR .. "src/mame/drivers/pastelg.c",
	MAME_DIR .. "src/mame/video/pastelg.c",
	MAME_DIR .. "src/mame/drivers/seicross.c",
	MAME_DIR .. "src/mame/video/seicross.c",
	MAME_DIR .. "src/mame/drivers/terracre.c",
	MAME_DIR .. "src/mame/video/terracre.c",
	MAME_DIR .. "src/mame/drivers/tubep.c",
	MAME_DIR .. "src/mame/video/tubep.c",
	MAME_DIR .. "src/mame/drivers/wiping.c",
	MAME_DIR .. "src/mame/audio/wiping.c",
	MAME_DIR .. "src/mame/video/wiping.c",
	MAME_DIR .. "src/mame/machine/nb1413m3.c",
	MAME_DIR .. "src/mame/machine/nb1414m4.c",
}

createMAMEProjects(_target, _subtarget, "nintendo")
files {
	MAME_DIR .. "src/mame/drivers/cham24.c",
	MAME_DIR .. "src/mame/drivers/dkong.c",
	MAME_DIR .. "src/mame/audio/dkong.c",
	MAME_DIR .. "src/mame/video/dkong.c",
	MAME_DIR .. "src/mame/drivers/mario.c",
	MAME_DIR .. "src/mame/audio/mario.c",
	MAME_DIR .. "src/mame/video/mario.c",
	MAME_DIR .. "src/mame/drivers/mmagic.c",
	MAME_DIR .. "src/mame/drivers/multigam.c",
	MAME_DIR .. "src/mame/drivers/n8080.c",
	MAME_DIR .. "src/mame/audio/n8080.c",
	MAME_DIR .. "src/mame/video/n8080.c",
	MAME_DIR .. "src/mame/drivers/nss.c",
	MAME_DIR .. "src/mame/machine/snes.c",
	MAME_DIR .. "src/mame/audio/snes_snd.c",
	MAME_DIR .. "src/mame/drivers/playch10.c",
	MAME_DIR .. "src/mame/machine/playch10.c",
	MAME_DIR .. "src/mame/video/playch10.c",
	MAME_DIR .. "src/mame/drivers/popeye.c",
	MAME_DIR .. "src/mame/video/popeye.c",
	MAME_DIR .. "src/mame/drivers/punchout.c",
	MAME_DIR .. "src/mame/video/punchout.c",
	MAME_DIR .. "src/mame/drivers/famibox.c",
	MAME_DIR .. "src/mame/drivers/sfcbox.c",
	MAME_DIR .. "src/mame/drivers/snesb.c",
	MAME_DIR .. "src/mame/drivers/spacefb.c",
	MAME_DIR .. "src/mame/audio/spacefb.c",
	MAME_DIR .. "src/mame/video/spacefb.c",
	MAME_DIR .. "src/mame/drivers/vsnes.c",
	MAME_DIR .. "src/mame/machine/vsnes.c",
	MAME_DIR .. "src/mame/video/vsnes.c",
	MAME_DIR .. "src/mame/video/ppu2c0x.c",


}

createMAMEProjects(_target, _subtarget, "nix")
files {
	MAME_DIR .. "src/mame/drivers/fitfight.c",
	MAME_DIR .. "src/mame/video/fitfight.c",
	MAME_DIR .. "src/mame/drivers/pirates.c",
	MAME_DIR .. "src/mame/video/pirates.c",
}

createMAMEProjects(_target, _subtarget, "nmk")
files {
	MAME_DIR .. "src/mame/drivers/acommand.c",
	MAME_DIR .. "src/mame/drivers/cultures.c",
	MAME_DIR .. "src/mame/drivers/ddealer.c",
	MAME_DIR .. "src/mame/drivers/jalmah.c",
	MAME_DIR .. "src/mame/drivers/macrossp.c",
	MAME_DIR .. "src/mame/video/macrossp.c",
	MAME_DIR .. "src/mame/drivers/nmk16.c",
	MAME_DIR .. "src/mame/machine/nmk004.c",
	MAME_DIR .. "src/mame/video/nmk16.c",
	MAME_DIR .. "src/mame/drivers/quizdna.c",
	MAME_DIR .. "src/mame/video/quizdna.c",
	MAME_DIR .. "src/mame/drivers/quizpani.c",
	MAME_DIR .. "src/mame/video/quizpani.c",
}

createMAMEProjects(_target, _subtarget, "olympia")
files {
	MAME_DIR .. "src/mame/drivers/dday.c",
	MAME_DIR .. "src/mame/video/dday.c",
	MAME_DIR .. "src/mame/drivers/lbeach.c",
	MAME_DIR .. "src/mame/drivers/monzagp.c",
	MAME_DIR .. "src/mame/drivers/portrait.c",
	MAME_DIR .. "src/mame/video/portrait.c",
	MAME_DIR .. "src/mame/drivers/vega.c",
}

createMAMEProjects(_target, _subtarget, "omori")
files {
	MAME_DIR .. "src/mame/drivers/battlex.c",
	MAME_DIR .. "src/mame/video/battlex.c",
	MAME_DIR .. "src/mame/drivers/carjmbre.c",
	MAME_DIR .. "src/mame/video/carjmbre.c",
	MAME_DIR .. "src/mame/drivers/popper.c",
	MAME_DIR .. "src/mame/video/popper.c",
	MAME_DIR .. "src/mame/drivers/spaceg.c",
}

createMAMEProjects(_target, _subtarget, "orca")
files {
	MAME_DIR .. "src/mame/drivers/espial.c",
	MAME_DIR .. "src/mame/video/espial.c",
	MAME_DIR .. "src/mame/drivers/funkybee.c",
	MAME_DIR .. "src/mame/video/funkybee.c",
	MAME_DIR .. "src/mame/drivers/marineb.c",
	MAME_DIR .. "src/mame/video/marineb.c",
	MAME_DIR .. "src/mame/drivers/vastar.c",
	MAME_DIR .. "src/mame/video/vastar.c",
	MAME_DIR .. "src/mame/drivers/zodiack.c",
	MAME_DIR .. "src/mame/video/zodiack.c",
}

createMAMEProjects(_target, _subtarget, "pacific")
files {
	MAME_DIR .. "src/mame/drivers/mrflea.c",
	MAME_DIR .. "src/mame/video/mrflea.c",
	MAME_DIR .. "src/mame/drivers/thief.c",
	MAME_DIR .. "src/mame/video/thief.c",
}

createMAMEProjects(_target, _subtarget, "pacman")
files {
	MAME_DIR .. "src/mame/drivers/jrpacman.c",
	MAME_DIR .. "src/mame/drivers/pacman.c",
	MAME_DIR .. "src/mame/video/pacman.c",
	MAME_DIR .. "src/mame/machine/acitya.c",
	MAME_DIR .. "src/mame/machine/jumpshot.c",
	MAME_DIR .. "src/mame/machine/pacplus.c",
	MAME_DIR .. "src/mame/machine/theglobp.c",
	MAME_DIR .. "src/mame/drivers/pengo.c",
}

createMAMEProjects(_target, _subtarget, "pce")
files {
	MAME_DIR .. "src/mame/drivers/ggconnie.c",
	MAME_DIR .. "src/mame/drivers/paranoia.c",
	MAME_DIR .. "src/mame/drivers/tourvis.c",
	MAME_DIR .. "src/mame/drivers/uapce.c",
	MAME_DIR .. "src/mame/machine/pcecommn.c",
}

createMAMEProjects(_target, _subtarget, "phoenix")
files {
	MAME_DIR .. "src/mame/drivers/naughtyb.c",
	MAME_DIR .. "src/mame/video/naughtyb.c",
	MAME_DIR .. "src/mame/drivers/phoenix.c",
	MAME_DIR .. "src/mame/audio/phoenix.c",
	MAME_DIR .. "src/mame/video/phoenix.c",
	MAME_DIR .. "src/mame/drivers/safarir.c",
	MAME_DIR .. "src/mame/audio/pleiads.c",
}

createMAMEProjects(_target, _subtarget, "playmark")
files {
	MAME_DIR .. "src/mame/drivers/drtomy.c",
	MAME_DIR .. "src/mame/drivers/playmark.c",
	MAME_DIR .. "src/mame/video/playmark.c",
	MAME_DIR .. "src/mame/drivers/powerbal.c",
	MAME_DIR .. "src/mame/drivers/sderby.c",
	MAME_DIR .. "src/mame/video/sderby.c",
	MAME_DIR .. "src/mame/drivers/sslam.c",
	MAME_DIR .. "src/mame/video/sslam.c",
}

createMAMEProjects(_target, _subtarget, "psikyo")
files {
	MAME_DIR .. "src/mame/drivers/psikyo.c",
	MAME_DIR .. "src/mame/video/psikyo.c",
	MAME_DIR .. "src/mame/drivers/psikyo4.c",
	MAME_DIR .. "src/mame/video/psikyo4.c",
	MAME_DIR .. "src/mame/drivers/psikyosh.c",
	MAME_DIR .. "src/mame/video/psikyosh.c",
}

createMAMEProjects(_target, _subtarget, "ramtek")
files {
	MAME_DIR .. "src/mame/drivers/hitme.c",
	MAME_DIR .. "src/mame/audio/hitme.c",
	MAME_DIR .. "src/mame/drivers/ramtek.c",
	MAME_DIR .. "src/mame/drivers/starcrus.c",
	MAME_DIR .. "src/mame/video/starcrus.c",
}

createMAMEProjects(_target, _subtarget, "rare")
files {
	MAME_DIR .. "src/mame/drivers/btoads.c",
	MAME_DIR .. "src/mame/video/btoads.c",
	MAME_DIR .. "src/mame/drivers/kinst.c",
	MAME_DIR .. "src/mame/drivers/xtheball.c",
}

createMAMEProjects(_target, _subtarget, "sanritsu")
files {
	MAME_DIR .. "src/mame/drivers/appoooh.c",
	MAME_DIR .. "src/mame/video/appoooh.c",
	MAME_DIR .. "src/mame/drivers/bankp.c",
	MAME_DIR .. "src/mame/video/bankp.c",
	MAME_DIR .. "src/mame/drivers/chinsan.c",
	MAME_DIR .. "src/mame/drivers/drmicro.c",
	MAME_DIR .. "src/mame/video/drmicro.c",
	MAME_DIR .. "src/mame/drivers/jantotsu.c",
	MAME_DIR .. "src/mame/drivers/mayumi.c",
	MAME_DIR .. "src/mame/drivers/mermaid.c",
	MAME_DIR .. "src/mame/video/mermaid.c",
	MAME_DIR .. "src/mame/drivers/mjkjidai.c",
	MAME_DIR .. "src/mame/video/mjkjidai.c",
}

createMAMEProjects(_target, _subtarget, "sega")
files {
	MAME_DIR .. "src/mame/drivers/angelkds.c",
	MAME_DIR .. "src/mame/video/angelkds.c",
	MAME_DIR .. "src/mame/drivers/bingoc.c",
	MAME_DIR .. "src/mame/drivers/blockade.c",
	MAME_DIR .. "src/mame/audio/blockade.c",
	MAME_DIR .. "src/mame/video/blockade.c",
	MAME_DIR .. "src/mame/drivers/calorie.c",
	MAME_DIR .. "src/mame/drivers/chihiro.c",
	MAME_DIR .. "src/mame/video/chihiro.c",
	MAME_DIR .. "src/mame/drivers/coolridr.c",
	MAME_DIR .. "src/mame/drivers/deniam.c",
	MAME_DIR .. "src/mame/video/deniam.c",
	MAME_DIR .. "src/mame/drivers/dotrikun.c",
	MAME_DIR .. "src/mame/drivers/gpworld.c",
	MAME_DIR .. "src/mame/drivers/hikaru.c",
	MAME_DIR .. "src/mame/drivers/hshavoc.c",
	MAME_DIR .. "src/mame/drivers/kopunch.c",
	MAME_DIR .. "src/mame/video/kopunch.c",
	MAME_DIR .. "src/mame/drivers/lindbergh.c",
	MAME_DIR .. "src/mame/machine/segabb.c",
	MAME_DIR .. "src/mame/machine/megadriv.c",
	MAME_DIR .. "src/mame/drivers/megadrvb.c",
	MAME_DIR .. "src/mame/drivers/megaplay.c",
	MAME_DIR .. "src/mame/drivers/megatech.c",
	MAME_DIR .. "src/mame/drivers/model1.c",
	MAME_DIR .. "src/mame/machine/model1.c",
	MAME_DIR .. "src/mame/video/model1.c",
	MAME_DIR .. "src/mame/machine/s32comm.c",
	MAME_DIR .. "src/mame/machine/m1comm.c",
	MAME_DIR .. "src/mame/audio/dsbz80.c",
	MAME_DIR .. "src/mame/drivers/model2.c",
	MAME_DIR .. "src/mame/video/model2.c",
	MAME_DIR .. "src/mame/drivers/model3.c",
	MAME_DIR .. "src/mame/video/model3.c",
	MAME_DIR .. "src/mame/machine/model3.c",
	MAME_DIR .. "src/mame/drivers/monacogp.c",
	MAME_DIR .. "src/mame/drivers/naomi.c",
	MAME_DIR .. "src/mame/drivers/segasp.c",
	MAME_DIR .. "src/mame/machine/dc.c",
	MAME_DIR .. "src/mame/video/powervr2.c",
	MAME_DIR .. "src/mame/machine/naomi.c",
	MAME_DIR .. "src/mame/machine/naomig1.c",
	MAME_DIR .. "src/mame/machine/naomibd.c",
	MAME_DIR .. "src/mame/machine/naomirom.c",
	MAME_DIR .. "src/mame/machine/naomigd.c",
	MAME_DIR .. "src/mame/machine/naomim1.c",
	MAME_DIR .. "src/mame/machine/naomim2.c",
	MAME_DIR .. "src/mame/machine/naomim4.c",
	MAME_DIR .. "src/mame/machine/315-5881_crypt.c",
	MAME_DIR .. "src/mame/machine/awboard.c",
	MAME_DIR .. "src/mame/machine/mie.c",
	MAME_DIR .. "src/mame/machine/maple-dc.c",
	MAME_DIR .. "src/mame/machine/mapledev.c",
	MAME_DIR .. "src/mame/machine/dc-ctrl.c",
	MAME_DIR .. "src/mame/machine/jvs13551.c",
	MAME_DIR .. "src/mame/drivers/triforce.c",
	MAME_DIR .. "src/mame/drivers/puckpkmn.c",
	MAME_DIR .. "src/mame/drivers/segac2.c",
	MAME_DIR .. "src/mame/drivers/segae.c",
	MAME_DIR .. "src/mame/drivers/shtzone.c",
	MAME_DIR .. "src/mame/drivers/segacoin.c",
	MAME_DIR .. "src/mame/drivers/segag80r.c",
	MAME_DIR .. "src/mame/machine/segag80.c",
	MAME_DIR .. "src/mame/audio/segag80r.c",
	MAME_DIR .. "src/mame/video/segag80r.c",
	MAME_DIR .. "src/mame/drivers/segag80v.c",
	MAME_DIR .. "src/mame/audio/segag80v.c",
	MAME_DIR .. "src/mame/video/segag80v.c",
	MAME_DIR .. "src/mame/drivers/segahang.c",
	MAME_DIR .. "src/mame/video/segahang.c",
	MAME_DIR .. "src/mame/drivers/segajw.c",
	MAME_DIR .. "src/mame/drivers/segald.c",
	MAME_DIR .. "src/mame/drivers/segaorun.c",
	MAME_DIR .. "src/mame/video/segaorun.c",
	MAME_DIR .. "src/mame/drivers/segas16a.c",
	MAME_DIR .. "src/mame/video/segas16a.c",
	MAME_DIR .. "src/mame/drivers/segas16b.c",
	MAME_DIR .. "src/mame/video/segas16b.c",
	MAME_DIR .. "src/mame/drivers/segas18.c",
	MAME_DIR .. "src/mame/video/segas18.c",
	MAME_DIR .. "src/mame/drivers/segas24.c",
	MAME_DIR .. "src/mame/video/segas24.c",
	MAME_DIR .. "src/mame/drivers/segam1.c",
	MAME_DIR .. "src/mame/drivers/segas32.c",
	MAME_DIR .. "src/mame/machine/segas32.c",
	MAME_DIR .. "src/mame/video/segas32.c",
	MAME_DIR .. "src/mame/drivers/segaufo.c",
	MAME_DIR .. "src/mame/drivers/segaxbd.c",
	MAME_DIR .. "src/mame/video/segaxbd.c",
	MAME_DIR .. "src/mame/drivers/segaybd.c",
	MAME_DIR .. "src/mame/video/segaybd.c",
	MAME_DIR .. "src/mame/drivers/sg1000a.c",
	MAME_DIR .. "src/mame/drivers/stactics.c",
	MAME_DIR .. "src/mame/video/stactics.c",
	MAME_DIR .. "src/mame/drivers/stv.c",
	MAME_DIR .. "src/mame/machine/stvprot.c",
	MAME_DIR .. "src/mame/machine/315-5838_317-0229_comp.c",
	MAME_DIR .. "src/mame/drivers/suprloco.c",
	MAME_DIR .. "src/mame/video/suprloco.c",
	MAME_DIR .. "src/mame/drivers/system1.c",
	MAME_DIR .. "src/mame/video/system1.c",
	MAME_DIR .. "src/mame/drivers/system16.c",
	MAME_DIR .. "src/mame/video/system16.c",
	MAME_DIR .. "src/mame/drivers/timetrv.c",
	MAME_DIR .. "src/mame/drivers/turbo.c",
	MAME_DIR .. "src/mame/audio/turbo.c",
	MAME_DIR .. "src/mame/video/turbo.c",
	MAME_DIR .. "src/mame/drivers/vicdual.c",
	MAME_DIR .. "src/mame/audio/vicdual.c",
	MAME_DIR .. "src/mame/video/vicdual.c",
	MAME_DIR .. "src/mame/audio/carnival.c",
	MAME_DIR .. "src/mame/audio/depthch.c",
	MAME_DIR .. "src/mame/audio/invinco.c",
	MAME_DIR .. "src/mame/audio/pulsar.c",
	MAME_DIR .. "src/mame/drivers/zaxxon.c",
	MAME_DIR .. "src/mame/audio/zaxxon.c",
	MAME_DIR .. "src/mame/video/zaxxon.c",
	MAME_DIR .. "src/mame/machine/315_5296.c",
	MAME_DIR .. "src/mame/machine/fd1089.c",
	MAME_DIR .. "src/mame/machine/fd1094.c",
	MAME_DIR .. "src/mame/machine/fddebug.c",
	MAME_DIR .. "src/mame/machine/mc8123.c",
	MAME_DIR .. "src/mame/machine/segaic16.c",
	MAME_DIR .. "src/mame/audio/segasnd.c",
	MAME_DIR .. "src/mame/video/segaic16.c",
	MAME_DIR .. "src/mame/video/segaic16_road.c",
	MAME_DIR .. "src/mame/video/sega16sp.c",
	MAME_DIR .. "src/mame/video/segaic24.c",
	MAME_DIR .. "src/mame/machine/gdrom.c",
	MAME_DIR .. "src/mame/machine/xbox.c",
}

createMAMEProjects(_target, _subtarget, "seibu")
files {
	MAME_DIR .. "src/mame/drivers/bloodbro.c",
	MAME_DIR .. "src/mame/video/bloodbro.c",
	MAME_DIR .. "src/mame/drivers/cabal.c",
	MAME_DIR .. "src/mame/video/cabal.c",
	MAME_DIR .. "src/mame/drivers/cshooter.c",
	MAME_DIR .. "src/mame/drivers/dcon.c",
	MAME_DIR .. "src/mame/video/dcon.c",
	MAME_DIR .. "src/mame/drivers/deadang.c",
	MAME_DIR .. "src/mame/video/deadang.c",
	MAME_DIR .. "src/mame/drivers/dynduke.c",
	MAME_DIR .. "src/mame/video/dynduke.c",
	MAME_DIR .. "src/mame/drivers/feversoc.c",
	MAME_DIR .. "src/mame/drivers/goal92.c",
	MAME_DIR .. "src/mame/video/goal92.c",
	MAME_DIR .. "src/mame/drivers/goodejan.c",
	MAME_DIR .. "src/mame/drivers/kncljoe.c",
	MAME_DIR .. "src/mame/video/kncljoe.c",
	MAME_DIR .. "src/mame/drivers/legionna.c",
	MAME_DIR .. "src/mame/video/legionna.c",
	MAME_DIR .. "src/mame/drivers/mustache.c",
	MAME_DIR .. "src/mame/video/mustache.c",
	MAME_DIR .. "src/mame/drivers/panicr.c",
	MAME_DIR .. "src/mame/drivers/raiden.c",
	MAME_DIR .. "src/mame/video/raiden.c",
	MAME_DIR .. "src/mame/drivers/raiden2.c",
	MAME_DIR .. "src/mame/machine/r2crypt.c",
	MAME_DIR .. "src/mame/machine/raiden2cop.c",
	MAME_DIR .. "src/mame/drivers/r2dx_v33.c",
	MAME_DIR .. "src/mame/drivers/seibuspi.c",
	MAME_DIR .. "src/mame/machine/seibuspi.c",
	MAME_DIR .. "src/mame/video/seibuspi.c",
	MAME_DIR .. "src/mame/drivers/sengokmj.c",
	MAME_DIR .. "src/mame/drivers/stfight.c",
	MAME_DIR .. "src/mame/machine/stfight.c",
	MAME_DIR .. "src/mame/video/stfight.c",
	MAME_DIR .. "src/mame/drivers/toki.c",
	MAME_DIR .. "src/mame/video/toki.c",
	MAME_DIR .. "src/mame/drivers/wiz.c",
	MAME_DIR .. "src/mame/video/wiz.c",
	MAME_DIR .. "src/mame/machine/seicop.c",
	MAME_DIR .. "src/mame/machine/spisprit.c",
	MAME_DIR .. "src/mame/audio/seibu.c",
	MAME_DIR .. "src/mame/video/seibu_crtc.c",
}

createMAMEProjects(_target, _subtarget, "seta")
files {
	MAME_DIR .. "src/mame/drivers/aleck64.c",
	MAME_DIR .. "src/mame/machine/n64.c",
	MAME_DIR .. "src/mame/video/n64.c",
	MAME_DIR .. "src/mame/video/rdpblend.c",
	MAME_DIR .. "src/mame/video/rdptpipe.c",
	MAME_DIR .. "src/mame/drivers/hanaawas.c",
	MAME_DIR .. "src/mame/video/hanaawas.c",
	MAME_DIR .. "src/mame/drivers/jclub2.c",
	MAME_DIR .. "src/mame/drivers/macs.c",
	MAME_DIR .. "src/mame/drivers/seta.c",
	MAME_DIR .. "src/mame/video/seta.c",
	MAME_DIR .. "src/mame/drivers/seta2.c",
	MAME_DIR .. "src/mame/video/seta2.c",
	MAME_DIR .. "src/mame/drivers/speedatk.c",
	MAME_DIR .. "src/mame/video/speedatk.c",
	MAME_DIR .. "src/mame/drivers/speglsht.c",
	MAME_DIR .. "src/mame/drivers/srmp2.c",
	MAME_DIR .. "src/mame/video/srmp2.c",
	MAME_DIR .. "src/mame/drivers/srmp5.c",
	MAME_DIR .. "src/mame/drivers/srmp6.c",
	MAME_DIR .. "src/mame/drivers/ssv.c",
	MAME_DIR .. "src/mame/video/ssv.c",
	MAME_DIR .. "src/mame/video/st0020.c",
	MAME_DIR .. "src/mame/machine/st0016.c",
	MAME_DIR .. "src/mame/drivers/simple_st0016.c",
	MAME_DIR .. "src/mame/video/seta001.c",
	MAME_DIR .. "src/mame/drivers/thedealr.c",
}

createMAMEProjects(_target, _subtarget, "sigma")
files {
	MAME_DIR .. "src/mame/drivers/nyny.c",
	MAME_DIR .. "src/mame/drivers/r2dtank.c",
	MAME_DIR .. "src/mame/drivers/sigmab52.c",
	MAME_DIR .. "src/mame/drivers/sigmab98.c",
	MAME_DIR .. "src/mame/drivers/spiders.c",
	MAME_DIR .. "src/mame/audio/spiders.c",
	MAME_DIR .. "src/mame/drivers/sub.c",
}

createMAMEProjects(_target, _subtarget, "snk")
files {
	MAME_DIR .. "src/mame/drivers/bbusters.c",
	MAME_DIR .. "src/mame/video/bbusters.c",
	MAME_DIR .. "src/mame/drivers/dmndrby.c",
	MAME_DIR .. "src/mame/drivers/hng64.c",
	MAME_DIR .. "src/mame/video/hng64.c",
	MAME_DIR .. "src/mame/audio/hng64.c",
	MAME_DIR .. "src/mame/machine/hng64_net.c",
	MAME_DIR .. "src/mame/video/hng64_3d.c",
	MAME_DIR .. "src/mame/video/hng64_sprite.c",
	MAME_DIR .. "src/mame/drivers/lasso.c",
	MAME_DIR .. "src/mame/video/lasso.c",
	MAME_DIR .. "src/mame/drivers/mainsnk.c",
	MAME_DIR .. "src/mame/video/mainsnk.c",
	MAME_DIR .. "src/mame/drivers/munchmo.c",
	MAME_DIR .. "src/mame/video/munchmo.c",
	MAME_DIR .. "src/mame/drivers/prehisle.c",
	MAME_DIR .. "src/mame/video/prehisle.c",
	MAME_DIR .. "src/mame/drivers/snk6502.c",
	MAME_DIR .. "src/mame/audio/snk6502.c",
	MAME_DIR .. "src/mame/video/snk6502.c",
	MAME_DIR .. "src/mame/drivers/snk.c",
	MAME_DIR .. "src/mame/video/snk.c",
	MAME_DIR .. "src/mame/drivers/snk68.c",
	MAME_DIR .. "src/mame/video/snk68.c",
}

createMAMEProjects(_target, _subtarget, "sony")
files {
	MAME_DIR .. "src/mame/drivers/zn.c",
	MAME_DIR .. "src/mame/machine/zndip.c",
	MAME_DIR .. "src/mame/machine/cat702.c",
}

createMAMEProjects(_target, _subtarget, "stern")
files {
	MAME_DIR .. "src/mame/drivers/astinvad.c",
	MAME_DIR .. "src/mame/drivers/berzerk.c",
	MAME_DIR .. "src/mame/drivers/cliffhgr.c",
	MAME_DIR .. "src/mame/audio/cliffhgr.c",
	MAME_DIR .. "src/mame/drivers/mazerbla.c",
	MAME_DIR .. "src/mame/drivers/supdrapo.c",
}

createMAMEProjects(_target, _subtarget, "subsino")
files {
	MAME_DIR .. "src/mame/drivers/lastfght.c",
	MAME_DIR .. "src/mame/drivers/subsino.c",
	MAME_DIR .. "src/mame/drivers/subsino2.c",
	MAME_DIR .. "src/mame/machine/subsino.c",
}

createMAMEProjects(_target, _subtarget, "sun")
files {
	MAME_DIR .. "src/mame/drivers/arabian.c",
	MAME_DIR .. "src/mame/video/arabian.c",
	MAME_DIR .. "src/mame/drivers/dai3wksi.c",
	MAME_DIR .. "src/mame/drivers/ikki.c",
	MAME_DIR .. "src/mame/video/ikki.c",
	MAME_DIR .. "src/mame/drivers/kangaroo.c",
	MAME_DIR .. "src/mame/video/kangaroo.c",
	MAME_DIR .. "src/mame/drivers/markham.c",
	MAME_DIR .. "src/mame/video/markham.c",
	MAME_DIR .. "src/mame/drivers/route16.c",
	MAME_DIR .. "src/mame/video/route16.c",
	MAME_DIR .. "src/mame/drivers/shanghai.c",
	MAME_DIR .. "src/mame/drivers/shangha3.c",
	MAME_DIR .. "src/mame/video/shangha3.c",
	MAME_DIR .. "src/mame/drivers/strnskil.c",
	MAME_DIR .. "src/mame/video/strnskil.c",
	MAME_DIR .. "src/mame/drivers/tonton.c",
}

createMAMEProjects(_target, _subtarget, "suna")
files {
	MAME_DIR .. "src/mame/drivers/go2000.c",
	MAME_DIR .. "src/mame/drivers/goindol.c",
	MAME_DIR .. "src/mame/video/goindol.c",
	MAME_DIR .. "src/mame/drivers/suna8.c",
	MAME_DIR .. "src/mame/audio/suna8.c",
	MAME_DIR .. "src/mame/video/suna8.c",
	MAME_DIR .. "src/mame/drivers/suna16.c",
	MAME_DIR .. "src/mame/video/suna16.c",
}

createMAMEProjects(_target, _subtarget, "sure")
files {
	MAME_DIR .. "src/mame/drivers/mil4000.c",

}

createMAMEProjects(_target, _subtarget, "taito")
files {
	MAME_DIR .. "src/mame/drivers/2mindril.c",
	MAME_DIR .. "src/mame/drivers/40love.c",
	MAME_DIR .. "src/mame/video/40love.c",
	MAME_DIR .. "src/mame/drivers/arkanoid.c",
	MAME_DIR .. "src/mame/machine/arkanoid.c",
	MAME_DIR .. "src/mame/video/arkanoid.c",
	MAME_DIR .. "src/mame/drivers/ashnojoe.c",
	MAME_DIR .. "src/mame/video/ashnojoe.c",
	MAME_DIR .. "src/mame/drivers/asuka.c",
	MAME_DIR .. "src/mame/machine/bonzeadv.c",
	MAME_DIR .. "src/mame/video/asuka.c",
	MAME_DIR .. "src/mame/drivers/bigevglf.c",
	MAME_DIR .. "src/mame/machine/bigevglf.c",
	MAME_DIR .. "src/mame/video/bigevglf.c",
	MAME_DIR .. "src/mame/drivers/bking.c",
	MAME_DIR .. "src/mame/video/bking.c",
	MAME_DIR .. "src/mame/drivers/bublbobl.c",
	MAME_DIR .. "src/mame/machine/bublbobl.c",
	MAME_DIR .. "src/mame/video/bublbobl.c",
	MAME_DIR .. "src/mame/drivers/buggychl.c",
	MAME_DIR .. "src/mame/machine/buggychl.c",
	MAME_DIR .. "src/mame/video/buggychl.c",
	MAME_DIR .. "src/mame/drivers/capr1.c",
	MAME_DIR .. "src/mame/drivers/caprcyc.c",
	MAME_DIR .. "src/mame/drivers/cchance.c",
	MAME_DIR .. "src/mame/drivers/chaknpop.c",
	MAME_DIR .. "src/mame/machine/chaknpop.c",
	MAME_DIR .. "src/mame/video/chaknpop.c",
	MAME_DIR .. "src/mame/drivers/champbwl.c",
	MAME_DIR .. "src/mame/drivers/changela.c",
	MAME_DIR .. "src/mame/video/changela.c",
	MAME_DIR .. "src/mame/drivers/crbaloon.c",
	MAME_DIR .. "src/mame/video/crbaloon.c",
	MAME_DIR .. "src/mame/audio/crbaloon.c",
	MAME_DIR .. "src/mame/drivers/cyclemb.c",
	MAME_DIR .. "src/mame/drivers/darius.c",
	MAME_DIR .. "src/mame/video/darius.c",
	MAME_DIR .. "src/mame/drivers/darkmist.c",
	MAME_DIR .. "src/mame/video/darkmist.c",
	MAME_DIR .. "src/mame/drivers/exzisus.c",
	MAME_DIR .. "src/mame/video/exzisus.c",
	MAME_DIR .. "src/mame/drivers/fgoal.c",
	MAME_DIR .. "src/mame/video/fgoal.c",
	MAME_DIR .. "src/mame/drivers/flstory.c",
	MAME_DIR .. "src/mame/machine/flstory.c",
	MAME_DIR .. "src/mame/video/flstory.c",
	MAME_DIR .. "src/mame/drivers/galastrm.c",
	MAME_DIR .. "src/mame/video/galastrm.c",
	MAME_DIR .. "src/mame/drivers/gladiatr.c",
	MAME_DIR .. "src/mame/video/gladiatr.c",
	MAME_DIR .. "src/mame/drivers/grchamp.c",
	MAME_DIR .. "src/mame/audio/grchamp.c",
	MAME_DIR .. "src/mame/video/grchamp.c",
	MAME_DIR .. "src/mame/drivers/groundfx.c",
	MAME_DIR .. "src/mame/video/groundfx.c",
	MAME_DIR .. "src/mame/drivers/gsword.c",
	MAME_DIR .. "src/mame/machine/tait8741.c",
	MAME_DIR .. "src/mame/video/gsword.c",
	MAME_DIR .. "src/mame/drivers/gunbustr.c",
	MAME_DIR .. "src/mame/video/gunbustr.c",
	MAME_DIR .. "src/mame/drivers/halleys.c",
	MAME_DIR .. "src/mame/drivers/invqix.c",
	MAME_DIR .. "src/mame/drivers/jollyjgr.c",
	MAME_DIR .. "src/mame/drivers/ksayakyu.c",
	MAME_DIR .. "src/mame/video/ksayakyu.c",
	MAME_DIR .. "src/mame/drivers/lgp.c",
	MAME_DIR .. "src/mame/drivers/lkage.c",
	MAME_DIR .. "src/mame/machine/lkage.c",
	MAME_DIR .. "src/mame/video/lkage.c",
	MAME_DIR .. "src/mame/drivers/lsasquad.c",
	MAME_DIR .. "src/mame/machine/lsasquad.c",
	MAME_DIR .. "src/mame/video/lsasquad.c",
	MAME_DIR .. "src/mame/drivers/marinedt.c",
	MAME_DIR .. "src/mame/drivers/mexico86.c",
	MAME_DIR .. "src/mame/machine/mexico86.c",
	MAME_DIR .. "src/mame/video/mexico86.c",
	MAME_DIR .. "src/mame/drivers/minivadr.c",
	MAME_DIR .. "src/mame/drivers/missb2.c",
	MAME_DIR .. "src/mame/drivers/mlanding.c",
	MAME_DIR .. "src/mame/drivers/msisaac.c",
	MAME_DIR .. "src/mame/video/msisaac.c",
	MAME_DIR .. "src/mame/drivers/ninjaw.c",
	MAME_DIR .. "src/mame/video/ninjaw.c",
	MAME_DIR .. "src/mame/drivers/nycaptor.c",
	MAME_DIR .. "src/mame/machine/nycaptor.c",
	MAME_DIR .. "src/mame/video/nycaptor.c",
	MAME_DIR .. "src/mame/drivers/opwolf.c",
	MAME_DIR .. "src/mame/machine/opwolf.c",
	MAME_DIR .. "src/mame/video/opwolf.c",
	MAME_DIR .. "src/mame/drivers/othunder.c",
	MAME_DIR .. "src/mame/video/othunder.c",
	MAME_DIR .. "src/mame/drivers/pitnrun.c",
	MAME_DIR .. "src/mame/machine/pitnrun.c",
	MAME_DIR .. "src/mame/video/pitnrun.c",
	MAME_DIR .. "src/mame/drivers/qix.c",
	MAME_DIR .. "src/mame/machine/qix.c",
	MAME_DIR .. "src/mame/audio/qix.c",
	MAME_DIR .. "src/mame/video/qix.c",
	MAME_DIR .. "src/mame/drivers/rainbow.c",
	MAME_DIR .. "src/mame/machine/rainbow.c",
	MAME_DIR .. "src/mame/video/rainbow.c",
	MAME_DIR .. "src/mame/drivers/rastan.c",
	MAME_DIR .. "src/mame/video/rastan.c",
	MAME_DIR .. "src/mame/drivers/retofinv.c",
	MAME_DIR .. "src/mame/machine/retofinv.c",
	MAME_DIR .. "src/mame/video/retofinv.c",
	MAME_DIR .. "src/mame/drivers/rollrace.c",
	MAME_DIR .. "src/mame/video/rollrace.c",
	MAME_DIR .. "src/mame/drivers/sbowling.c",
	MAME_DIR .. "src/mame/drivers/slapshot.c",
	MAME_DIR .. "src/mame/video/slapshot.c",
	MAME_DIR .. "src/mame/drivers/ssrj.c",
	MAME_DIR .. "src/mame/video/ssrj.c",
	MAME_DIR .. "src/mame/drivers/superchs.c",
	MAME_DIR .. "src/mame/video/superchs.c",
	MAME_DIR .. "src/mame/drivers/superqix.c",
	MAME_DIR .. "src/mame/video/superqix.c",
	MAME_DIR .. "src/mame/drivers/taito_b.c",
	MAME_DIR .. "src/mame/video/taito_b.c",
	MAME_DIR .. "src/mame/drivers/taito_f2.c",
	MAME_DIR .. "src/mame/video/taito_f2.c",
	MAME_DIR .. "src/mame/drivers/taito_f3.c",
	MAME_DIR .. "src/mame/video/taito_f3.c",
	MAME_DIR .. "src/mame/audio/taito_en.c",
	MAME_DIR .. "src/mame/drivers/taito_h.c",
	MAME_DIR .. "src/mame/video/taito_h.c",
	MAME_DIR .. "src/mame/drivers/taito_l.c",
	MAME_DIR .. "src/mame/video/taito_l.c",
	MAME_DIR .. "src/mame/drivers/taito_x.c",
	MAME_DIR .. "src/mame/machine/cchip.c",
	MAME_DIR .. "src/mame/drivers/taito_z.c",
	MAME_DIR .. "src/mame/video/taito_z.c",
	MAME_DIR .. "src/mame/drivers/taito_o.c",
	MAME_DIR .. "src/mame/video/taito_o.c",
	MAME_DIR .. "src/mame/drivers/taitoair.c",
	MAME_DIR .. "src/mame/video/taitoair.c",
	MAME_DIR .. "src/mame/drivers/taitogn.c",
	MAME_DIR .. "src/mame/drivers/taitojc.c",
	MAME_DIR .. "src/mame/video/taitojc.c",
	MAME_DIR .. "src/mame/drivers/taitopjc.c",
	MAME_DIR .. "src/mame/drivers/taitosj.c",
	MAME_DIR .. "src/mame/machine/taitosj.c",
	MAME_DIR .. "src/mame/video/taitosj.c",
	MAME_DIR .. "src/mame/drivers/taitottl.c",
	MAME_DIR .. "src/mame/drivers/taitotz.c",
	MAME_DIR .. "src/mame/drivers/taitotx.c",
	MAME_DIR .. "src/mame/drivers/taitowlf.c",
	MAME_DIR .. "src/mame/drivers/tnzs.c",
	MAME_DIR .. "src/mame/machine/tnzs.c",
	MAME_DIR .. "src/mame/video/tnzs.c",
	MAME_DIR .. "src/mame/drivers/topspeed.c",
	MAME_DIR .. "src/mame/video/topspeed.c",
	MAME_DIR .. "src/mame/drivers/tsamurai.c",
	MAME_DIR .. "src/mame/video/tsamurai.c",
	MAME_DIR .. "src/mame/drivers/undrfire.c",
	MAME_DIR .. "src/mame/video/undrfire.c",
	MAME_DIR .. "src/mame/drivers/volfied.c",
	MAME_DIR .. "src/mame/machine/volfied.c",
	MAME_DIR .. "src/mame/video/volfied.c",
	MAME_DIR .. "src/mame/drivers/warriorb.c",
	MAME_DIR .. "src/mame/video/warriorb.c",
	MAME_DIR .. "src/mame/drivers/wgp.c",
	MAME_DIR .. "src/mame/video/wgp.c",
	MAME_DIR .. "src/mame/drivers/wyvernf0.c",
	MAME_DIR .. "src/mame/audio/taitosnd.c",
	MAME_DIR .. "src/mame/audio/taito_zm.c",
	MAME_DIR .. "src/mame/audio/t5182.c",
	MAME_DIR .. "src/mame/machine/taitoio.c",
	MAME_DIR .. "src/mame/video/taito_helper.c",
	MAME_DIR .. "src/mame/video/pc080sn.c",
	MAME_DIR .. "src/mame/video/pc090oj.c",
	MAME_DIR .. "src/mame/video/tc0080vco.c",
	MAME_DIR .. "src/mame/video/tc0100scn.c",
	MAME_DIR .. "src/mame/video/tc0150rod.c",
	MAME_DIR .. "src/mame/video/tc0280grd.c",
	MAME_DIR .. "src/mame/video/tc0360pri.c",
	MAME_DIR .. "src/mame/video/tc0480scp.c",
	MAME_DIR .. "src/mame/video/tc0110pcr.c",
	MAME_DIR .. "src/mame/video/tc0180vcu.c",
}

createMAMEProjects(_target, _subtarget, "tatsumi")
files {
	MAME_DIR .. "src/mame/drivers/kingdrby.c",
	MAME_DIR .. "src/mame/drivers/lockon.c",
	MAME_DIR .. "src/mame/video/lockon.c",
	MAME_DIR .. "src/mame/drivers/tatsumi.c",
	MAME_DIR .. "src/mame/machine/tatsumi.c",
	MAME_DIR .. "src/mame/video/tatsumi.c",
	MAME_DIR .. "src/mame/drivers/tx1.c",
	MAME_DIR .. "src/mame/machine/tx1.c",
	MAME_DIR .. "src/mame/audio/tx1.c",
	MAME_DIR .. "src/mame/video/tx1.c",
}

createMAMEProjects(_target, _subtarget, "tch")
files {
	MAME_DIR .. "src/mame/drivers/kickgoal.c",
	MAME_DIR .. "src/mame/video/kickgoal.c",
	MAME_DIR .. "src/mame/drivers/littlerb.c",
	MAME_DIR .. "src/mame/drivers/rltennis.c",
	MAME_DIR .. "src/mame/video/rltennis.c",
	MAME_DIR .. "src/mame/drivers/speedspn.c",
	MAME_DIR .. "src/mame/video/speedspn.c",
	MAME_DIR .. "src/mame/drivers/wheelfir.c",
}

createMAMEProjects(_target, _subtarget, "tecfri")
files {
	MAME_DIR .. "src/mame/drivers/ambush.c",
	MAME_DIR .. "src/mame/video/ambush.c",
	MAME_DIR .. "src/mame/drivers/holeland.c",
	MAME_DIR .. "src/mame/video/holeland.c",
	MAME_DIR .. "src/mame/drivers/sauro.c",
	MAME_DIR .. "src/mame/video/sauro.c",
	MAME_DIR .. "src/mame/drivers/speedbal.c",
	MAME_DIR .. "src/mame/video/speedbal.c",
}

createMAMEProjects(_target, _subtarget, "technos")
files {
	MAME_DIR .. "src/mame/drivers/battlane.c",
	MAME_DIR .. "src/mame/video/battlane.c",
	MAME_DIR .. "src/mame/drivers/blockout.c",
	MAME_DIR .. "src/mame/video/blockout.c",
	MAME_DIR .. "src/mame/drivers/bogeyman.c",
	MAME_DIR .. "src/mame/video/bogeyman.c",
	MAME_DIR .. "src/mame/drivers/chinagat.c",
	MAME_DIR .. "src/mame/drivers/ddragon.c",
	MAME_DIR .. "src/mame/video/ddragon.c",
	MAME_DIR .. "src/mame/drivers/ddragon3.c",
	MAME_DIR .. "src/mame/video/ddragon3.c",
	MAME_DIR .. "src/mame/drivers/dogfgt.c",
	MAME_DIR .. "src/mame/video/dogfgt.c",
	MAME_DIR .. "src/mame/drivers/matmania.c",
	MAME_DIR .. "src/mame/video/matmania.c",
	MAME_DIR .. "src/mame/drivers/mystston.c",
	MAME_DIR .. "src/mame/video/mystston.c",
	MAME_DIR .. "src/mame/drivers/renegade.c",
	MAME_DIR .. "src/mame/video/renegade.c",
	MAME_DIR .. "src/mame/drivers/scregg.c",
	MAME_DIR .. "src/mame/drivers/shadfrce.c",
	MAME_DIR .. "src/mame/video/shadfrce.c",
	MAME_DIR .. "src/mame/drivers/spdodgeb.c",
	MAME_DIR .. "src/mame/video/spdodgeb.c",
	MAME_DIR .. "src/mame/drivers/ssozumo.c",
	MAME_DIR .. "src/mame/video/ssozumo.c",
	MAME_DIR .. "src/mame/drivers/tagteam.c",
	MAME_DIR .. "src/mame/video/tagteam.c",
	MAME_DIR .. "src/mame/drivers/vball.c",
	MAME_DIR .. "src/mame/video/vball.c",
	MAME_DIR .. "src/mame/drivers/wwfsstar.c",
	MAME_DIR .. "src/mame/video/wwfsstar.c",
	MAME_DIR .. "src/mame/drivers/xain.c",
	MAME_DIR .. "src/mame/video/xain.c",
}

createMAMEProjects(_target, _subtarget, "tehkan")
files {
	MAME_DIR .. "src/mame/video/tecmo_spr.c",
	MAME_DIR .. "src/mame/video/tecmo_mix.c",
	MAME_DIR .. "src/mame/drivers/bombjack.c",
	MAME_DIR .. "src/mame/video/bombjack.c",
	MAME_DIR .. "src/mame/drivers/gaiden.c",
	MAME_DIR .. "src/mame/video/gaiden.c",
	MAME_DIR .. "src/mame/drivers/lvcards.c",
	MAME_DIR .. "src/mame/video/lvcards.c",
	MAME_DIR .. "src/mame/drivers/pbaction.c",
	MAME_DIR .. "src/mame/video/pbaction.c",
	MAME_DIR .. "src/mame/drivers/senjyo.c",
	MAME_DIR .. "src/mame/audio/senjyo.c",
	MAME_DIR .. "src/mame/video/senjyo.c",
	MAME_DIR .. "src/mame/drivers/solomon.c",
	MAME_DIR .. "src/mame/video/solomon.c",
	MAME_DIR .. "src/mame/drivers/spbactn.c",
	MAME_DIR .. "src/mame/video/spbactn.c",
	MAME_DIR .. "src/mame/drivers/tbowl.c",
	MAME_DIR .. "src/mame/video/tbowl.c",
	MAME_DIR .. "src/mame/drivers/tecmo.c",
	MAME_DIR .. "src/mame/video/tecmo.c",
	MAME_DIR .. "src/mame/drivers/tecmo16.c",
	MAME_DIR .. "src/mame/video/tecmo16.c",
	MAME_DIR .. "src/mame/drivers/tecmosys.c",
	MAME_DIR .. "src/mame/machine/tecmosys.c",
	MAME_DIR .. "src/mame/video/tecmosys.c",
	MAME_DIR .. "src/mame/drivers/tehkanwc.c",
	MAME_DIR .. "src/mame/video/tehkanwc.c",
	MAME_DIR .. "src/mame/drivers/wc90.c",
	MAME_DIR .. "src/mame/video/wc90.c",
	MAME_DIR .. "src/mame/drivers/wc90b.c",
	MAME_DIR .. "src/mame/video/wc90b.c",
}

createMAMEProjects(_target, _subtarget, "thepit")
files {
	MAME_DIR .. "src/mame/drivers/thepit.c",
	MAME_DIR .. "src/mame/video/thepit.c",
	MAME_DIR .. "src/mame/drivers/timelimt.c",
	MAME_DIR .. "src/mame/video/timelimt.c",
}

createMAMEProjects(_target, _subtarget, "toaplan")
files {
	MAME_DIR .. "src/mame/drivers/mjsister.c",
	MAME_DIR .. "src/mame/drivers/slapfght.c",
	MAME_DIR .. "src/mame/machine/slapfght.c",
	MAME_DIR .. "src/mame/video/slapfght.c",
	MAME_DIR .. "src/mame/drivers/snowbros.c",
	MAME_DIR .. "src/mame/video/kan_pand.c",
	MAME_DIR .. "src/mame/video/kan_panb.c",
	MAME_DIR .. "src/mame/drivers/toaplan1.c",
	MAME_DIR .. "src/mame/machine/toaplan1.c",
	MAME_DIR .. "src/mame/video/toaplan1.c",
	MAME_DIR .. "src/mame/drivers/toaplan2.c",
	MAME_DIR .. "src/mame/video/toaplan2.c",
	MAME_DIR .. "src/mame/video/gp9001.c",
	MAME_DIR .. "src/mame/drivers/twincobr.c",
	MAME_DIR .. "src/mame/machine/twincobr.c",
	MAME_DIR .. "src/mame/video/twincobr.c",
	MAME_DIR .. "src/mame/drivers/wardner.c",
	MAME_DIR .. "src/mame/video/toaplan_scu.c",
}

createMAMEProjects(_target, _subtarget, "tong")
files {
	MAME_DIR .. "src/mame/drivers/beezer.c",
	MAME_DIR .. "src/mame/machine/beezer.c",
	MAME_DIR .. "src/mame/video/beezer.c",
	MAME_DIR .. "src/mame/audio/beezer.c",
}

createMAMEProjects(_target, _subtarget, "unico")
files {
	MAME_DIR .. "src/mame/drivers/drgnmst.c",
	MAME_DIR .. "src/mame/video/drgnmst.c",
	MAME_DIR .. "src/mame/drivers/silkroad.c",
	MAME_DIR .. "src/mame/video/silkroad.c",
	MAME_DIR .. "src/mame/drivers/unico.c",
	MAME_DIR .. "src/mame/video/unico.c",
}

createMAMEProjects(_target, _subtarget, "univers")
files {
	MAME_DIR .. "src/mame/drivers/cheekyms.c",
	MAME_DIR .. "src/mame/video/cheekyms.c",
	MAME_DIR .. "src/mame/drivers/cosmic.c",
	MAME_DIR .. "src/mame/video/cosmic.c",
	MAME_DIR .. "src/mame/drivers/docastle.c",
	MAME_DIR .. "src/mame/machine/docastle.c",
	MAME_DIR .. "src/mame/video/docastle.c",
	MAME_DIR .. "src/mame/drivers/ladybug.c",
	MAME_DIR .. "src/mame/video/ladybug.c",
	MAME_DIR .. "src/mame/drivers/mrdo.c",
	MAME_DIR .. "src/mame/video/mrdo.c",
	MAME_DIR .. "src/mame/drivers/redclash.c",
	MAME_DIR .. "src/mame/video/redclash.c",
	MAME_DIR .. "src/mame/drivers/superdq.c",
}

createMAMEProjects(_target, _subtarget, "upl")
files {
	MAME_DIR .. "src/mame/drivers/mouser.c",
	MAME_DIR .. "src/mame/video/mouser.c",
	MAME_DIR .. "src/mame/drivers/ninjakd2.c",
	MAME_DIR .. "src/mame/video/ninjakd2.c",
	MAME_DIR .. "src/mame/drivers/nova2001.c",
	MAME_DIR .. "src/mame/video/nova2001.c",
	MAME_DIR .. "src/mame/drivers/xxmissio.c",
	MAME_DIR .. "src/mame/video/xxmissio.c",
}

createMAMEProjects(_target, _subtarget, "valadon")
files {
	MAME_DIR .. "src/mame/drivers/bagman.c",
	MAME_DIR .. "src/mame/machine/bagman.c",
	MAME_DIR .. "src/mame/video/bagman.c",
	MAME_DIR .. "src/mame/drivers/tankbust.c",
	MAME_DIR .. "src/mame/video/tankbust.c",
}

createMAMEProjects(_target, _subtarget, "veltmjr")
files {
	MAME_DIR .. "src/mame/drivers/cardline.c",
	MAME_DIR .. "src/mame/drivers/witch.c",
}

createMAMEProjects(_target, _subtarget, "venture")
files {
	MAME_DIR .. "src/mame/drivers/looping.c",
	MAME_DIR .. "src/mame/drivers/spcforce.c",
	MAME_DIR .. "src/mame/video/spcforce.c",
	MAME_DIR .. "src/mame/drivers/suprridr.c",
	MAME_DIR .. "src/mame/video/suprridr.c",
}

createMAMEProjects(_target, _subtarget, "vsystem")
files {
	MAME_DIR .. "src/mame/video/vsystem_spr.c",
	MAME_DIR .. "src/mame/video/vsystem_spr2.c",
	MAME_DIR .. "src/mame/drivers/aerofgt.c",
	MAME_DIR .. "src/mame/video/aerofgt.c",
	MAME_DIR .. "src/mame/drivers/crshrace.c",
	MAME_DIR .. "src/mame/video/crshrace.c",
	MAME_DIR .. "src/mame/drivers/f1gp.c",
	MAME_DIR .. "src/mame/video/f1gp.c",
	MAME_DIR .. "src/mame/drivers/fromance.c",
	MAME_DIR .. "src/mame/video/fromance.c",
	MAME_DIR .. "src/mame/drivers/fromanc2.c",
	MAME_DIR .. "src/mame/video/fromanc2.c",
	MAME_DIR .. "src/mame/drivers/gstriker.c",
	MAME_DIR .. "src/mame/video/gstriker.c",
	MAME_DIR .. "src/mame/video/mb60553.c",
	MAME_DIR .. "src/mame/video/vs920a.c",
	MAME_DIR .. "src/mame/drivers/inufuku.c",
	MAME_DIR .. "src/mame/video/inufuku.c",
	MAME_DIR .. "src/mame/drivers/ojankohs.c",
	MAME_DIR .. "src/mame/video/ojankohs.c",
	MAME_DIR .. "src/mame/drivers/pipedrm.c",
	MAME_DIR .. "src/mame/drivers/rpunch.c",
	MAME_DIR .. "src/mame/video/rpunch.c",
	MAME_DIR .. "src/mame/drivers/suprslam.c",
	MAME_DIR .. "src/mame/video/suprslam.c",
	MAME_DIR .. "src/mame/drivers/tail2nos.c",
	MAME_DIR .. "src/mame/video/tail2nos.c",
	MAME_DIR .. "src/mame/drivers/taotaido.c",
	MAME_DIR .. "src/mame/video/taotaido.c",
	MAME_DIR .. "src/mame/drivers/welltris.c",
	MAME_DIR .. "src/mame/video/welltris.c",
}

createMAMEProjects(_target, _subtarget, "yunsung")
files {
	MAME_DIR .. "src/mame/drivers/nmg5.c",
	MAME_DIR .. "src/mame/drivers/paradise.c",
	MAME_DIR .. "src/mame/video/paradise.c",
	MAME_DIR .. "src/mame/drivers/yunsung8.c",
	MAME_DIR .. "src/mame/video/yunsung8.c",
	MAME_DIR .. "src/mame/drivers/yunsun16.c",
	MAME_DIR .. "src/mame/video/yunsun16.c",
}

createMAMEProjects(_target, _subtarget, "zaccaria")
files {
	MAME_DIR .. "src/mame/drivers/laserbat.c",
	MAME_DIR .. "src/mame/audio/laserbat.c",
	MAME_DIR .. "src/mame/drivers/seabattl.c",
	MAME_DIR .. "src/mame/drivers/zac2650.c",
	MAME_DIR .. "src/mame/video/zac2650.c",
	MAME_DIR .. "src/mame/drivers/zaccaria.c",
	MAME_DIR .. "src/mame/video/zaccaria.c",
}

--------------------------------------------------
-- pinball drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "pinball")
files {
	MAME_DIR .. "src/mame/drivers/allied.c",
	MAME_DIR .. "src/mame/drivers/alvg.c",
	MAME_DIR .. "src/mame/drivers/atari_s1.c",
	MAME_DIR .. "src/mame/drivers/atari_s2.c",
	MAME_DIR .. "src/mame/drivers/bingo.c",
	MAME_DIR .. "src/mame/drivers/by17.c",
	MAME_DIR .. "src/mame/drivers/by35.c",
	MAME_DIR .. "src/mame/drivers/by6803.c",
	MAME_DIR .. "src/mame/drivers/by68701.c",
	MAME_DIR .. "src/mame/drivers/byvid.c",
	MAME_DIR .. "src/mame/drivers/capcom.c",
	MAME_DIR .. "src/mame/drivers/de_2.c",
	MAME_DIR .. "src/mame/drivers/de_3.c",
	MAME_DIR .. "src/mame/machine/decopincpu.c",
	MAME_DIR .. "src/mame/video/decodmd1.c",
	MAME_DIR .. "src/mame/video/decodmd2.c",
	MAME_DIR .. "src/mame/video/decodmd3.c",
	MAME_DIR .. "src/mame/drivers/de_3b.c",
	MAME_DIR .. "src/mame/drivers/flicker.c",
	MAME_DIR .. "src/mame/drivers/g627.c",
	MAME_DIR .. "src/mame/drivers/gp_1.c",
	MAME_DIR .. "src/mame/machine/genpin.c",
	MAME_DIR .. "src/mame/drivers/gp_2.c",
	MAME_DIR .. "src/mame/drivers/gts1.c",
	MAME_DIR .. "src/mame/drivers/gts3.c",
	MAME_DIR .. "src/mame/drivers/gts3a.c",
	MAME_DIR .. "src/mame/drivers/gts80.c",
	MAME_DIR .. "src/mame/drivers/gts80a.c",
	MAME_DIR .. "src/mame/drivers/gts80b.c",
	MAME_DIR .. "src/mame/drivers/hankin.c",
	MAME_DIR .. "src/mame/drivers/icecold.c",
	MAME_DIR .. "src/mame/drivers/inder.c",
	MAME_DIR .. "src/mame/drivers/jeutel.c",
	MAME_DIR .. "src/mame/drivers/jp.c",
	MAME_DIR .. "src/mame/drivers/jvh.c",
	MAME_DIR .. "src/mame/drivers/kissproto.c",
	MAME_DIR .. "src/mame/drivers/ltd.c",
	MAME_DIR .. "src/mame/drivers/micropin.c",
	MAME_DIR .. "src/mame/drivers/mephisto.c",
	MAME_DIR .. "src/mame/drivers/mrgame.c",
	MAME_DIR .. "src/mame/drivers/nsm.c",
	MAME_DIR .. "src/mame/drivers/peyper.c",
	MAME_DIR .. "src/mame/drivers/play_1.c",
	MAME_DIR .. "src/mame/drivers/play_2.c",
	MAME_DIR .. "src/mame/drivers/play_3.c",
	MAME_DIR .. "src/mame/drivers/play_5.c",
	MAME_DIR .. "src/mame/drivers/rowamet.c",
	MAME_DIR .. "src/mame/drivers/s11.c",
	MAME_DIR .. "src/mame/drivers/s11a.c",
	MAME_DIR .. "src/mame/drivers/s11b.c",
	MAME_DIR .. "src/mame/drivers/s11c.c",
	MAME_DIR .. "src/mame/audio/s11c_bg.c",
	MAME_DIR .. "src/mame/drivers/s3.c",
	MAME_DIR .. "src/mame/drivers/s4.c",
	MAME_DIR .. "src/mame/drivers/s6.c",
	MAME_DIR .. "src/mame/drivers/s6a.c",
	MAME_DIR .. "src/mame/drivers/s7.c",
	MAME_DIR .. "src/mame/drivers/s8.c",
	MAME_DIR .. "src/mame/drivers/s8a.c",
	MAME_DIR .. "src/mame/drivers/s9.c",
	MAME_DIR .. "src/mame/drivers/sam.c",
	MAME_DIR .. "src/mame/drivers/sleic.c",
	MAME_DIR .. "src/mame/drivers/spectra.c",
	MAME_DIR .. "src/mame/drivers/spinb.c",
	MAME_DIR .. "src/mame/drivers/st_mp100.c",
	MAME_DIR .. "src/mame/drivers/st_mp200.c",
	MAME_DIR .. "src/mame/drivers/taito.c",
	MAME_DIR .. "src/mame/drivers/techno.c",
	MAME_DIR .. "src/mame/drivers/vd.c",
	MAME_DIR .. "src/mame/drivers/whitestar.c",
	MAME_DIR .. "src/mame/drivers/white_mod.c",
	MAME_DIR .. "src/mame/drivers/wico.c",
	MAME_DIR .. "src/mame/drivers/wpc_95.c",
	MAME_DIR .. "src/mame/drivers/wpc_an.c",
	MAME_DIR .. "src/mame/drivers/wpc_dcs.c",
	MAME_DIR .. "src/mame/drivers/wpc_dot.c",
	MAME_DIR .. "src/mame/drivers/wpc_flip1.c",
	MAME_DIR .. "src/mame/drivers/wpc_flip2.c",
	MAME_DIR .. "src/mame/drivers/wpc_s.c",
	MAME_DIR .. "src/mame/machine/wpc.c",
	MAME_DIR .. "src/mame/audio/wpcsnd.c",
	MAME_DIR .. "src/mame/video/wpc_dmd.c",
	MAME_DIR .. "src/mame/machine/wpc_pic.c",
	MAME_DIR .. "src/mame/machine/wpc_lamp.c",
	MAME_DIR .. "src/mame/machine/wpc_out.c",
	MAME_DIR .. "src/mame/machine/wpc_shift.c",
	MAME_DIR .. "src/mame/drivers/zac_1.c",
	MAME_DIR .. "src/mame/drivers/zac_2.c",
	MAME_DIR .. "src/mame/drivers/zac_proto.c",
}

--------------------------------------------------
-- remaining drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "misc")
files {
	MAME_DIR .. "src/mame/drivers/1945kiii.c",
	MAME_DIR .. "src/mame/drivers/39in1.c",
	MAME_DIR .. "src/mame/drivers/3do.c",
	MAME_DIR .. "src/mame/machine/3do.c",
	MAME_DIR .. "src/mame/drivers/3x3puzzl.c",
	MAME_DIR .. "src/mame/drivers/4enraya.c",
	MAME_DIR .. "src/mame/video/4enraya.c",
	MAME_DIR .. "src/mame/drivers/4enlinea.c",
	MAME_DIR .. "src/mame/drivers/5clown.c",
	MAME_DIR .. "src/mame/drivers/a1supply.c",
	MAME_DIR .. "src/mame/drivers/acefruit.c",
	MAME_DIR .. "src/mame/drivers/aces1.c",
	MAME_DIR .. "src/mame/drivers/acesp.c",
	MAME_DIR .. "src/mame/drivers/adp.c",
	MAME_DIR .. "src/mame/drivers/alinvade.c",
	MAME_DIR .. "src/mame/drivers/amaticmg.c",
	MAME_DIR .. "src/mame/drivers/ampoker2.c",
	MAME_DIR .. "src/mame/video/ampoker2.c",
	MAME_DIR .. "src/mame/drivers/amspdwy.c",
	MAME_DIR .. "src/mame/video/amspdwy.c",
	MAME_DIR .. "src/mame/drivers/amusco.c",
	MAME_DIR .. "src/mame/drivers/arachnid.c",
	MAME_DIR .. "src/mame/drivers/artmagic.c",
	MAME_DIR .. "src/mame/video/artmagic.c",
	MAME_DIR .. "src/mame/drivers/astrafr.c",
	MAME_DIR .. "src/mame/drivers/astrcorp.c",
	MAME_DIR .. "src/mame/drivers/astropc.c",
	MAME_DIR .. "src/mame/drivers/atronic.c",
	MAME_DIR .. "src/mame/drivers/attckufo.c",
	MAME_DIR .. "src/mame/drivers/avt.c",
	MAME_DIR .. "src/mame/drivers/aztarac.c",
	MAME_DIR .. "src/mame/audio/aztarac.c",
	MAME_DIR .. "src/mame/video/aztarac.c",
	MAME_DIR .. "src/mame/drivers/bailey.c",
	MAME_DIR .. "src/mame/drivers/beaminv.c",
	MAME_DIR .. "src/mame/drivers/belatra.c",
	MAME_DIR .. "src/mame/drivers/bgt.c",
	MAME_DIR .. "src/mame/drivers/bingoman.c",
	MAME_DIR .. "src/mame/drivers/bingor.c",
	MAME_DIR .. "src/mame/drivers/blitz.c",
	MAME_DIR .. "src/mame/drivers/blitz68k.c",
	MAME_DIR .. "src/mame/drivers/buster.c",
	MAME_DIR .. "src/mame/drivers/calomega.c",
	MAME_DIR .. "src/mame/video/calomega.c",
	MAME_DIR .. "src/mame/drivers/carrera.c",
	MAME_DIR .. "src/mame/drivers/castle.c",
	MAME_DIR .. "src/mame/drivers/cave.c",
	MAME_DIR .. "src/mame/video/cave.c",
	MAME_DIR .. "src/mame/drivers/cavepc.c",
	MAME_DIR .. "src/mame/drivers/cv1k.c",
	MAME_DIR .. "src/mame/drivers/cb2001.c",
	MAME_DIR .. "src/mame/drivers/cdi.c",
	MAME_DIR .. "src/mame/video/mcd212.c",
	MAME_DIR .. "src/mame/machine/cdi070.c",
	MAME_DIR .. "src/mame/machine/cdislave.c",
	MAME_DIR .. "src/mame/machine/cdicdic.c",
	MAME_DIR .. "src/mame/drivers/cesclass.c",
	MAME_DIR .. "src/mame/drivers/chance32.c",
	MAME_DIR .. "src/mame/drivers/chexx.c",
	MAME_DIR .. "src/mame/drivers/chicago.c",
	MAME_DIR .. "src/mame/drivers/chsuper.c",
	MAME_DIR .. "src/mame/drivers/cidelsa.c",
	MAME_DIR .. "src/mame/video/cidelsa.c",
	MAME_DIR .. "src/mame/drivers/cocoloco.c",
	MAME_DIR .. "src/mame/drivers/coinmstr.c",
	MAME_DIR .. "src/mame/drivers/coinmvga.c",
	MAME_DIR .. "src/mame/drivers/comebaby.c",
	MAME_DIR .. "src/mame/drivers/cupidon.c",
	MAME_DIR .. "src/mame/drivers/bntyhunt.c",
	MAME_DIR .. "src/mame/drivers/coolpool.c",
	MAME_DIR .. "src/mame/drivers/megaphx.c",
	MAME_DIR .. "src/mame/machine/inder_sb.c",
	MAME_DIR .. "src/mame/machine/inder_vid.c",
	MAME_DIR .. "src/mame/drivers/corona.c",
	MAME_DIR .. "src/mame/drivers/crystal.c",
	MAME_DIR .. "src/mame/video/vrender0.c",
	MAME_DIR .. "src/mame/drivers/cubeqst.c",
	MAME_DIR .. "src/mame/drivers/cybertnk.c",
	MAME_DIR .. "src/mame/drivers/dcheese.c",
	MAME_DIR .. "src/mame/video/dcheese.c",
	MAME_DIR .. "src/mame/drivers/dfruit.c",
	MAME_DIR .. "src/mame/drivers/dgpix.c",
	MAME_DIR .. "src/mame/drivers/discoboy.c",
	MAME_DIR .. "src/mame/drivers/dominob.c",
	MAME_DIR .. "src/mame/drivers/dorachan.c",
	MAME_DIR .. "src/mame/drivers/dreamwld.c",
	MAME_DIR .. "src/mame/drivers/dribling.c",
	MAME_DIR .. "src/mame/video/dribling.c",
	MAME_DIR .. "src/mame/drivers/drw80pkr.c",
	MAME_DIR .. "src/mame/drivers/dwarfd.c",
	MAME_DIR .. "src/mame/drivers/dynadice.c",
	MAME_DIR .. "src/mame/drivers/ecoinfr.c",
	MAME_DIR .. "src/mame/drivers/ecoinf1.c",
	MAME_DIR .. "src/mame/drivers/ecoinf2.c",
	MAME_DIR .. "src/mame/drivers/ecoinf3.c",
	MAME_DIR .. "src/mame/drivers/electra.c",
	MAME_DIR .. "src/mame/drivers/epos.c",
	MAME_DIR .. "src/mame/video/epos.c",
	MAME_DIR .. "src/mame/drivers/esd16.c",
	MAME_DIR .. "src/mame/video/esd16.c",
	MAME_DIR .. "src/mame/drivers/esh.c",
	MAME_DIR .. "src/mame/drivers/esripsys.c",
	MAME_DIR .. "src/mame/video/esripsys.c",
	MAME_DIR .. "src/mame/drivers/ettrivia.c",
	MAME_DIR .. "src/mame/drivers/extrema.c",
	MAME_DIR .. "src/mame/drivers/fireball.c",
	MAME_DIR .. "src/mame/drivers/flipjack.c",
	MAME_DIR .. "src/mame/drivers/flower.c",
	MAME_DIR .. "src/mame/audio/flower.c",
	MAME_DIR .. "src/mame/video/flower.c",
	MAME_DIR .. "src/mame/drivers/fortecar.c",
	MAME_DIR .. "src/mame/drivers/fresh.c",
	MAME_DIR .. "src/mame/drivers/freekick.c",
	MAME_DIR .. "src/mame/video/freekick.c",
	MAME_DIR .. "src/mame/drivers/fungames.c",
	MAME_DIR .. "src/mame/drivers/funkball.c",
	MAME_DIR .. "src/mame/drivers/gambl186.c",
	MAME_DIR .. "src/mame/drivers/galaxi.c",
	MAME_DIR .. "src/mame/drivers/galgame.c",
	MAME_DIR .. "src/mame/drivers/gamecstl.c",
	MAME_DIR .. "src/mame/drivers/gammagic.c",
	MAME_DIR .. "src/mame/drivers/gamtor.c",
	MAME_DIR .. "src/mame/drivers/gei.c",
	MAME_DIR .. "src/mame/drivers/globalfr.c",
	MAME_DIR .. "src/mame/drivers/globalvr.c",
	MAME_DIR .. "src/mame/drivers/gluck2.c",
	MAME_DIR .. "src/mame/drivers/goldngam.c",
	MAME_DIR .. "src/mame/drivers/goldnpkr.c",
	MAME_DIR .. "src/mame/drivers/good.c",
	MAME_DIR .. "src/mame/drivers/gotcha.c",
	MAME_DIR .. "src/mame/video/gotcha.c",
	MAME_DIR .. "src/mame/drivers/gstream.c",
	MAME_DIR .. "src/mame/drivers/gumbo.c",
	MAME_DIR .. "src/mame/video/gumbo.c",
	MAME_DIR .. "src/mame/drivers/gunpey.c",
	MAME_DIR .. "src/mame/drivers/hideseek.c",
	MAME_DIR .. "src/mame/drivers/hazelgr.c",
	MAME_DIR .. "src/mame/drivers/headonb.c",
	MAME_DIR .. "src/mame/drivers/highvdeo.c",
	MAME_DIR .. "src/mame/drivers/himesiki.c",
	MAME_DIR .. "src/mame/video/himesiki.c",
	MAME_DIR .. "src/mame/drivers/hitpoker.c",
	MAME_DIR .. "src/mame/drivers/homedata.c",
	MAME_DIR .. "src/mame/video/homedata.c",
	MAME_DIR .. "src/mame/drivers/hotblock.c",
	MAME_DIR .. "src/mame/drivers/hotstuff.c",
	MAME_DIR .. "src/mame/drivers/ichiban.c",
	MAME_DIR .. "src/mame/drivers/imolagp.c",
	MAME_DIR .. "src/mame/drivers/intrscti.c",
	MAME_DIR .. "src/mame/drivers/istellar.c",
	MAME_DIR .. "src/mame/drivers/itgambl2.c",
	MAME_DIR .. "src/mame/drivers/itgambl3.c",
	MAME_DIR .. "src/mame/drivers/itgamble.c",
	MAME_DIR .. "src/mame/drivers/jackpool.c",
	MAME_DIR .. "src/mame/drivers/jankenmn.c",
	MAME_DIR .. "src/mame/drivers/jokrwild.c",
	MAME_DIR .. "src/mame/drivers/jongkyo.c",
	MAME_DIR .. "src/mame/drivers/joystand.c",
	MAME_DIR .. "src/mame/drivers/jubilee.c",
	MAME_DIR .. "src/mame/drivers/kas89.c",
	MAME_DIR .. "src/mame/drivers/kingpin.c",
	MAME_DIR .. "src/mame/drivers/koikoi.c",
	MAME_DIR .. "src/mame/drivers/kurukuru.c",
	MAME_DIR .. "src/mame/drivers/kyugo.c",
	MAME_DIR .. "src/mame/video/kyugo.c",
	MAME_DIR .. "src/mame/drivers/ladyfrog.c",
	MAME_DIR .. "src/mame/video/ladyfrog.c",
	MAME_DIR .. "src/mame/drivers/laserbas.c",
	MAME_DIR .. "src/mame/drivers/lethalj.c",
	MAME_DIR .. "src/mame/video/lethalj.c",
	MAME_DIR .. "src/mame/drivers/limenko.c",
	MAME_DIR .. "src/mame/drivers/ltcasino.c",
	MAME_DIR .. "src/mame/drivers/lucky74.c",
	MAME_DIR .. "src/mame/video/lucky74.c",
	MAME_DIR .. "src/mame/drivers/luckgrln.c",
	MAME_DIR .. "src/mame/drivers/magic10.c",
	MAME_DIR .. "src/mame/drivers/magicard.c",
	MAME_DIR .. "src/mame/drivers/magicfly.c",
	MAME_DIR .. "src/mame/drivers/magictg.c",
	MAME_DIR .. "src/mame/drivers/magtouch.c",
	MAME_DIR .. "src/mame/drivers/majorpkr.c",
	MAME_DIR .. "src/mame/drivers/malzak.c",
	MAME_DIR .. "src/mame/video/malzak.c",
	MAME_DIR .. "src/mame/drivers/manohman.c",
	MAME_DIR .. "src/mame/drivers/mcatadv.c",
	MAME_DIR .. "src/mame/video/mcatadv.c",
	MAME_DIR .. "src/mame/drivers/mgavegas.c",
	MAME_DIR .. "src/mame/drivers/meyc8080.c",
	MAME_DIR .. "src/mame/drivers/meyc8088.c",
	MAME_DIR .. "src/mame/drivers/micro3d.c",
	MAME_DIR .. "src/mame/machine/micro3d.c",
	MAME_DIR .. "src/mame/video/micro3d.c",
	MAME_DIR .. "src/mame/audio/micro3d.c",
	MAME_DIR .. "src/mame/drivers/midas.c",
	MAME_DIR .. "src/mame/drivers/miniboy7.c",
	MAME_DIR .. "src/mame/drivers/mirax.c",
	MAME_DIR .. "src/mame/drivers/mole.c",
	MAME_DIR .. "src/mame/drivers/mosaic.c",
	MAME_DIR .. "src/mame/video/mosaic.c",
	MAME_DIR .. "src/mame/drivers/mpu12wbk.c",
	MAME_DIR .. "src/mame/drivers/mrjong.c",
	MAME_DIR .. "src/mame/video/mrjong.c",
	MAME_DIR .. "src/mame/drivers/multfish.c",
	MAME_DIR .. "src/mame/drivers/multfish_boot.c",
	MAME_DIR .. "src/mame/drivers/multfish_ref.c",
	MAME_DIR .. "src/mame/drivers/murogem.c",
	MAME_DIR .. "src/mame/drivers/murogmbl.c",
	MAME_DIR .. "src/mame/drivers/neoprint.c",
	MAME_DIR .. "src/mame/drivers/neptunp2.c",
	MAME_DIR .. "src/mame/drivers/news.c",
	MAME_DIR .. "src/mame/video/news.c",
	MAME_DIR .. "src/mame/drivers/nexus3d.c",
	MAME_DIR .. "src/mame/drivers/norautp.c",
	MAME_DIR .. "src/mame/audio/norautp.c",
	MAME_DIR .. "src/mame/drivers/nsmpoker.c",
	MAME_DIR .. "src/mame/drivers/oneshot.c",
	MAME_DIR .. "src/mame/video/oneshot.c",
	MAME_DIR .. "src/mame/drivers/onetwo.c",
	MAME_DIR .. "src/mame/drivers/othello.c",
	MAME_DIR .. "src/mame/drivers/pachifev.c",
	MAME_DIR .. "src/mame/drivers/pasha2.c",
	MAME_DIR .. "src/mame/drivers/pass.c",
	MAME_DIR .. "src/mame/video/pass.c",
	MAME_DIR .. "src/mame/drivers/peplus.c",
	MAME_DIR .. "src/mame/drivers/photon.c",
	MAME_DIR .. "src/mame/video/pk8000.c",
	MAME_DIR .. "src/mame/drivers/photon2.c",
	MAME_DIR .. "src/mame/drivers/photoply.c",
	MAME_DIR .. "src/mame/drivers/pinkiri8.c",
	MAME_DIR .. "src/mame/drivers/pipeline.c",
	MAME_DIR .. "src/mame/drivers/pkscram.c",
	MAME_DIR .. "src/mame/drivers/pntnpuzl.c",
	MAME_DIR .. "src/mame/drivers/policetr.c",
	MAME_DIR .. "src/mame/video/policetr.c",
	MAME_DIR .. "src/mame/drivers/polyplay.c",
	MAME_DIR .. "src/mame/audio/polyplay.c",
	MAME_DIR .. "src/mame/video/polyplay.c",
	MAME_DIR .. "src/mame/drivers/poker72.c",
	MAME_DIR .. "src/mame/drivers/potgoldu.c",
	MAME_DIR .. "src/mame/drivers/proconn.c",
	MAME_DIR .. "src/mame/drivers/psattack.c",
	MAME_DIR .. "src/mame/drivers/pse.c",
	MAME_DIR .. "src/mame/drivers/quizo.c",
	MAME_DIR .. "src/mame/drivers/quizpun2.c",
	MAME_DIR .. "src/mame/drivers/rbmk.c",
	MAME_DIR .. "src/mame/drivers/rcorsair.c",
	MAME_DIR .. "src/mame/drivers/re900.c",
	MAME_DIR .. "src/mame/drivers/rgum.c",
	MAME_DIR .. "src/mame/drivers/roul.c",
	MAME_DIR .. "src/mame/drivers/savquest.c",
	MAME_DIR .. "src/mame/drivers/sanremo.c",
	MAME_DIR .. "src/mame/drivers/sealy.c",
	MAME_DIR .. "src/mame/drivers/sfbonus.c",
	MAME_DIR .. "src/mame/drivers/shangkid.c",
	MAME_DIR .. "src/mame/video/shangkid.c",
	MAME_DIR .. "src/mame/drivers/skeetsht.c",
	MAME_DIR .. "src/mame/drivers/skimaxx.c",
	MAME_DIR .. "src/mame/drivers/skyarmy.c",
	MAME_DIR .. "src/mame/drivers/skylncr.c",
	MAME_DIR .. "src/mame/drivers/sliver.c",
	MAME_DIR .. "src/mame/drivers/slotcarn.c",
	MAME_DIR .. "src/mame/drivers/smsmcorp.c",
	MAME_DIR .. "src/mame/drivers/sothello.c",
	MAME_DIR .. "src/mame/drivers/splus.c",
	MAME_DIR .. "src/mame/drivers/spool99.c",
	MAME_DIR .. "src/mame/drivers/sprcros2.c",
	MAME_DIR .. "src/mame/video/sprcros2.c",
	MAME_DIR .. "src/mame/drivers/sshot.c",
	MAME_DIR .. "src/mame/drivers/ssingles.c",
	MAME_DIR .. "src/mame/drivers/sstrangr.c",
	MAME_DIR .. "src/mame/drivers/statriv2.c",
	MAME_DIR .. "src/mame/drivers/stellafr.c",
	MAME_DIR .. "src/mame/drivers/stuntair.c",
	MAME_DIR .. "src/mame/drivers/su2000.c",
	MAME_DIR .. "src/mame/drivers/summit.c",
	MAME_DIR .. "src/mame/drivers/sumt8035.c",
	MAME_DIR .. "src/mame/drivers/supercrd.c",
	MAME_DIR .. "src/mame/drivers/supertnk.c",
	MAME_DIR .. "src/mame/drivers/superwng.c",
	MAME_DIR .. "src/mame/drivers/tapatune.c",
	MAME_DIR .. "src/mame/drivers/tattack.c",
	MAME_DIR .. "src/mame/drivers/taxidriv.c",
	MAME_DIR .. "src/mame/video/taxidriv.c",
	MAME_DIR .. "src/mame/drivers/tcl.c",
	MAME_DIR .. "src/mame/drivers/thayers.c",
	MAME_DIR .. "src/mame/drivers/thedeep.c",
	MAME_DIR .. "src/mame/video/thedeep.c",
	MAME_DIR .. "src/mame/drivers/tiamc1.c",
	MAME_DIR .. "src/mame/video/tiamc1.c",
	MAME_DIR .. "src/mame/audio/tiamc1.c",
	MAME_DIR .. "src/mame/drivers/tickee.c",
	MAME_DIR .. "src/mame/drivers/tmspoker.c",
	MAME_DIR .. "src/mame/drivers/truco.c",
	MAME_DIR .. "src/mame/video/truco.c",
	MAME_DIR .. "src/mame/drivers/trucocl.c",
	MAME_DIR .. "src/mame/video/trucocl.c",
	MAME_DIR .. "src/mame/drivers/trvmadns.c",
	MAME_DIR .. "src/mame/drivers/trvquest.c",
	MAME_DIR .. "src/mame/drivers/ttchamp.c",
	MAME_DIR .. "src/mame/drivers/tugboat.c",
	MAME_DIR .. "src/mame/drivers/umipoker.c",
	MAME_DIR .. "src/mame/drivers/unkfr.c",
	MAME_DIR .. "src/mame/drivers/unkhorse.c",
	MAME_DIR .. "src/mame/drivers/usgames.c",
	MAME_DIR .. "src/mame/video/usgames.c",
	MAME_DIR .. "src/mame/drivers/vamphalf.c",
	MAME_DIR .. "src/mame/drivers/vcombat.c",
	MAME_DIR .. "src/mame/drivers/vectrex.c",
	MAME_DIR .. "src/mame/video/vectrex.c",
	MAME_DIR .. "src/mame/machine/vectrex.c",
	MAME_DIR .. "src/mame/drivers/videopkr.c",
	MAME_DIR .. "src/mame/drivers/vlc.c",
	MAME_DIR .. "src/mame/drivers/voyager.c",
	MAME_DIR .. "src/mame/drivers/vp101.c",
	MAME_DIR .. "src/mame/drivers/vpoker.c",
	MAME_DIR .. "src/mame/drivers/vroulet.c",
	MAME_DIR .. "src/mame/drivers/wildpkr.c",
	MAME_DIR .. "src/mame/drivers/wms.c",
	MAME_DIR .. "src/mame/drivers/xtom3d.c",
	MAME_DIR .. "src/mame/drivers/xyonix.c",
	MAME_DIR .. "src/mame/video/xyonix.c",
}
end

