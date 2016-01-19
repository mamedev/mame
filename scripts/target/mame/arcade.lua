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
SOUNDS["S14001A_NEW"] = true
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
--VIDEOS["EF9365"] = true
--VIDEOS["GF4500"] = true
VIDEOS["GF7600GS"] = true
VIDEOS["EPIC12"] = true
VIDEOS["FIXFREQ"] = true
VIDEOS["H63484"] = true
--VIDEOS["HD44102"] = true
--VIDEOS["HD44352"] = true
VIDEOS["HD44780"] = true
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
	addprojectflags()
	precompiledheaders()

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
	MAME_DIR .. "src/mame/machine/nmk112.cpp",
	MAME_DIR .. "src/mame/machine/nmk112.h",
	MAME_DIR .. "src/mame/machine/pcshare.cpp",
	MAME_DIR .. "src/mame/machine/pcshare.h",
	MAME_DIR .. "src/mame/machine/segacrpt.cpp",
	MAME_DIR .. "src/mame/machine/segacrpt.h",
	MAME_DIR .. "src/mame/machine/segacrp2.cpp",
	MAME_DIR .. "src/mame/machine/segacrp2.h",
	MAME_DIR .. "src/mame/machine/ticket.cpp",
	MAME_DIR .. "src/mame/machine/ticket.h",
	MAME_DIR .. "src/mame/video/avgdvg.cpp",
	MAME_DIR .. "src/mame/video/avgdvg.h",
	MAME_DIR .. "src/mame/audio/dcs.cpp",
	MAME_DIR .. "src/mame/audio/dcs.h",
	MAME_DIR .. "src/mame/audio/decobsmt.cpp",
	MAME_DIR .. "src/mame/audio/decobsmt.h",
	MAME_DIR .. "src/mame/audio/segam1audio.cpp",
	MAME_DIR .. "src/mame/audio/segam1audio.h",
}

--------------------------------------------------
-- manufacturer-specific groupings for drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "acorn")
files {
	MAME_DIR .. "src/mame/drivers/ertictac.cpp",
	MAME_DIR .. "src/mame/drivers/ssfindo.cpp",
	MAME_DIR .. "src/mame/drivers/aristmk5.cpp",
	MAME_DIR .. "src/mame/machine/archimds.cpp",
	MAME_DIR .. "src/mame/includes/archimds.h",
	MAME_DIR .. "src/mame/video/archimds.cpp",
}

createMAMEProjects(_target, _subtarget, "alba")
files {
	MAME_DIR .. "src/mame/drivers/albazc.cpp",
	MAME_DIR .. "src/mame/drivers/albazg.cpp",
	MAME_DIR .. "src/mame/drivers/rmhaihai.cpp",
}

createMAMEProjects(_target, _subtarget, "alliedl")
files {
	MAME_DIR .. "src/mame/drivers/ace.cpp",
	MAME_DIR .. "src/mame/drivers/aleisttl.cpp",
	MAME_DIR .. "src/mame/drivers/clayshoo.cpp",
}

createMAMEProjects(_target, _subtarget, "alpha")
files {
	MAME_DIR .. "src/mame/drivers/alpha68k.cpp",
	MAME_DIR .. "src/mame/includes/alpha68k.h",
	MAME_DIR .. "src/mame/video/alpha68k.cpp",
	MAME_DIR .. "src/mame/drivers/champbas.cpp",
	MAME_DIR .. "src/mame/includes/champbas.h",
	MAME_DIR .. "src/mame/video/champbas.cpp",
	MAME_DIR .. "src/mame/drivers/equites.cpp",
	MAME_DIR .. "src/mame/includes/equites.h",
	MAME_DIR .. "src/mame/video/equites.cpp",
	MAME_DIR .. "src/mame/drivers/meijinsn.cpp",
	MAME_DIR .. "src/mame/drivers/shougi.cpp",
	MAME_DIR .. "src/mame/machine/alpha8201.cpp",
	MAME_DIR .. "src/mame/machine/alpha8201.h",
}

createMAMEProjects(_target, _subtarget, "amiga")
files {
	MAME_DIR .. "src/mame/drivers/alg.cpp",
	MAME_DIR .. "src/mame/machine/amiga.cpp",
	MAME_DIR .. "src/mame/video/amiga.cpp",
	MAME_DIR .. "src/mame/video/amigaaga.cpp",
	MAME_DIR .. "src/mame/drivers/arsystems.cpp",
	MAME_DIR .. "src/mame/drivers/cubo.cpp",
	MAME_DIR .. "src/mame/drivers/mquake.cpp",
	MAME_DIR .. "src/mame/drivers/upscope.cpp",
}

createMAMEProjects(_target, _subtarget, "aristocr")
files {
	MAME_DIR .. "src/mame/drivers/aristmk4.cpp",
	MAME_DIR .. "src/mame/drivers/aristmk6.cpp",
	MAME_DIR .. "src/mame/drivers/caswin.cpp",
}

createMAMEProjects(_target, _subtarget, "ascii")
files {
	MAME_DIR .. "src/mame/drivers/big10.cpp",
	MAME_DIR .. "src/mame/drivers/forte2.cpp",
	MAME_DIR .. "src/mame/drivers/pengadvb.cpp",
	MAME_DIR .. "src/mame/drivers/sangho.cpp",
	MAME_DIR .. "src/mame/drivers/sfkick.cpp",
}

createMAMEProjects(_target, _subtarget, "atari")
files {
	MAME_DIR .. "src/mame/drivers/arcadecl.cpp",
	MAME_DIR .. "src/mame/includes/arcadecl.h",
	MAME_DIR .. "src/mame/video/arcadecl.cpp",
	MAME_DIR .. "src/mame/drivers/asteroid.cpp",
	MAME_DIR .. "src/mame/includes/asteroid.h",
	MAME_DIR .. "src/mame/machine/asteroid.cpp",
	MAME_DIR .. "src/mame/audio/asteroid.cpp",
	MAME_DIR .. "src/mame/audio/llander.cpp",
	MAME_DIR .. "src/mame/drivers/atarifb.cpp",
	MAME_DIR .. "src/mame/includes/atarifb.h",
	MAME_DIR .. "src/mame/machine/atarifb.cpp",
	MAME_DIR .. "src/mame/audio/atarifb.cpp",
	MAME_DIR .. "src/mame/video/atarifb.cpp",
	MAME_DIR .. "src/mame/drivers/atarig1.cpp",
	MAME_DIR .. "src/mame/includes/atarig1.h",
	MAME_DIR .. "src/mame/video/atarig1.cpp",
	MAME_DIR .. "src/mame/includes/slapstic.h",
	MAME_DIR .. "src/mame/drivers/atarig42.cpp",
	MAME_DIR .. "src/mame/includes/atarig42.h",
	MAME_DIR .. "src/mame/video/atarig42.cpp",
	MAME_DIR .. "src/mame/drivers/atarigt.cpp",
	MAME_DIR .. "src/mame/includes/atarigt.h",
	MAME_DIR .. "src/mame/video/atarigt.cpp",
	MAME_DIR .. "src/mame/drivers/atarigx2.cpp",
	MAME_DIR .. "src/mame/includes/atarigx2.h",
	MAME_DIR .. "src/mame/video/atarigx2.cpp",
	MAME_DIR .. "src/mame/drivers/atarisy1.cpp",
	MAME_DIR .. "src/mame/includes/atarisy1.h",
	MAME_DIR .. "src/mame/video/atarisy1.cpp",
	MAME_DIR .. "src/mame/drivers/atarisy2.cpp",
	MAME_DIR .. "src/mame/includes/atarisy2.h",
	MAME_DIR .. "src/mame/video/atarisy2.cpp",
	MAME_DIR .. "src/mame/drivers/atarisy4.cpp",
	MAME_DIR .. "src/mame/drivers/atarittl.cpp",
	MAME_DIR .. "src/mame/drivers/atetris.cpp",
	MAME_DIR .. "src/mame/includes/atetris.h",
	MAME_DIR .. "src/mame/video/atetris.cpp",
	MAME_DIR .. "src/mame/drivers/avalnche.cpp",
	MAME_DIR .. "src/mame/includes/avalnche.h",
	MAME_DIR .. "src/mame/audio/avalnche.cpp",
	MAME_DIR .. "src/mame/drivers/badlands.cpp",
	MAME_DIR .. "src/mame/includes/badlands.h",
	MAME_DIR .. "src/mame/video/badlands.cpp",
	MAME_DIR .. "src/mame/drivers/bartop52.cpp",
	MAME_DIR .. "src/mame/drivers/batman.cpp",
	MAME_DIR .. "src/mame/includes/batman.h",
	MAME_DIR .. "src/mame/video/batman.cpp",
	MAME_DIR .. "src/mame/drivers/beathead.cpp",
	MAME_DIR .. "src/mame/includes/beathead.h",
	MAME_DIR .. "src/mame/video/beathead.cpp",
	MAME_DIR .. "src/mame/drivers/blstroid.cpp",
	MAME_DIR .. "src/mame/includes/blstroid.h",
	MAME_DIR .. "src/mame/video/blstroid.cpp",
	MAME_DIR .. "src/mame/drivers/boxer.cpp",
	MAME_DIR .. "src/mame/drivers/bsktball.cpp",
	MAME_DIR .. "src/mame/includes/bsktball.h",
	MAME_DIR .. "src/mame/machine/bsktball.cpp",
	MAME_DIR .. "src/mame/audio/bsktball.cpp",
	MAME_DIR .. "src/mame/video/bsktball.cpp",
	MAME_DIR .. "src/mame/drivers/bwidow.cpp",
	MAME_DIR .. "src/mame/includes/bwidow.h",
	MAME_DIR .. "src/mame/audio/bwidow.cpp",
	MAME_DIR .. "src/mame/drivers/bzone.cpp",
	MAME_DIR .. "src/mame/includes/bzone.h",
	MAME_DIR .. "src/mame/audio/bzone.cpp",
	MAME_DIR .. "src/mame/drivers/canyon.cpp",
	MAME_DIR .. "src/mame/includes/canyon.h",
	MAME_DIR .. "src/mame/audio/canyon.cpp",
	MAME_DIR .. "src/mame/video/canyon.cpp",
	MAME_DIR .. "src/mame/drivers/cball.cpp",
	MAME_DIR .. "src/mame/drivers/ccastles.cpp",
	MAME_DIR .. "src/mame/includes/ccastles.h",
	MAME_DIR .. "src/mame/video/ccastles.cpp",
	MAME_DIR .. "src/mame/drivers/centiped.cpp",
	MAME_DIR .. "src/mame/includes/centiped.h",
	MAME_DIR .. "src/mame/video/centiped.cpp",
	MAME_DIR .. "src/mame/drivers/cloak.cpp",
	MAME_DIR .. "src/mame/includes/cloak.h",
	MAME_DIR .. "src/mame/video/cloak.cpp",
	MAME_DIR .. "src/mame/drivers/cloud9.cpp",
	MAME_DIR .. "src/mame/includes/cloud9.h",
	MAME_DIR .. "src/mame/video/cloud9.cpp",
	MAME_DIR .. "src/mame/drivers/cmmb.cpp",
	MAME_DIR .. "src/mame/drivers/cops.cpp",
	MAME_DIR .. "src/mame/drivers/copsnrob.cpp",
	MAME_DIR .. "src/mame/includes/copsnrob.h",
	MAME_DIR .. "src/mame/audio/copsnrob.cpp",
	MAME_DIR .. "src/mame/video/copsnrob.cpp",
	MAME_DIR .. "src/mame/drivers/cyberbal.cpp",
	MAME_DIR .. "src/mame/includes/cyberbal.h",
	MAME_DIR .. "src/mame/audio/cyberbal.cpp",
	MAME_DIR .. "src/mame/video/cyberbal.cpp",
	MAME_DIR .. "src/mame/drivers/destroyr.cpp",
	MAME_DIR .. "src/mame/drivers/dragrace.cpp",
	MAME_DIR .. "src/mame/includes/dragrace.h",
	MAME_DIR .. "src/mame/audio/dragrace.cpp",
	MAME_DIR .. "src/mame/video/dragrace.cpp",
	MAME_DIR .. "src/mame/drivers/eprom.cpp",
	MAME_DIR .. "src/mame/includes/eprom.h",
	MAME_DIR .. "src/mame/video/eprom.cpp",
	MAME_DIR .. "src/mame/drivers/firefox.cpp",
	MAME_DIR .. "src/mame/drivers/firetrk.cpp",
	MAME_DIR .. "src/mame/includes/firetrk.h",
	MAME_DIR .. "src/mame/audio/firetrk.cpp",
	MAME_DIR .. "src/mame/video/firetrk.cpp",
	MAME_DIR .. "src/mame/drivers/flyball.cpp",
	MAME_DIR .. "src/mame/drivers/foodf.cpp",
	MAME_DIR .. "src/mame/includes/foodf.h",
	MAME_DIR .. "src/mame/video/foodf.cpp",
	MAME_DIR .. "src/mame/drivers/gauntlet.cpp",
	MAME_DIR .. "src/mame/includes/gauntlet.h",
	MAME_DIR .. "src/mame/video/gauntlet.cpp",
	MAME_DIR .. "src/mame/drivers/harddriv.cpp",
	MAME_DIR .. "src/mame/includes/harddriv.h",
	MAME_DIR .. "src/mame/machine/harddriv.cpp",
	MAME_DIR .. "src/mame/audio/harddriv.cpp",
	MAME_DIR .. "src/mame/video/harddriv.cpp",
	MAME_DIR .. "src/mame/drivers/irobot.cpp",
	MAME_DIR .. "src/mame/includes/irobot.h",
	MAME_DIR .. "src/mame/machine/irobot.cpp",
	MAME_DIR .. "src/mame/video/irobot.cpp",
	MAME_DIR .. "src/mame/drivers/jaguar.cpp",
	MAME_DIR .. "src/mame/includes/jaguar.h",
	MAME_DIR .. "src/mame/audio/jaguar.cpp",
	MAME_DIR .. "src/mame/video/jaguar.cpp",
	MAME_DIR .. "src/mame/video/jagblit.h",
	MAME_DIR .. "src/mame/video/jagblit.inc",
	MAME_DIR .. "src/mame/video/jagobj.inc",
	MAME_DIR .. "src/mame/drivers/jedi.cpp",
	MAME_DIR .. "src/mame/includes/jedi.h",
	MAME_DIR .. "src/mame/audio/jedi.cpp",
	MAME_DIR .. "src/mame/video/jedi.cpp",
	MAME_DIR .. "src/mame/drivers/klax.cpp",
	MAME_DIR .. "src/mame/includes/klax.h",
	MAME_DIR .. "src/mame/video/klax.cpp",
	MAME_DIR .. "src/mame/drivers/liberatr.cpp",
	MAME_DIR .. "src/mame/includes/liberatr.h",
	MAME_DIR .. "src/mame/video/liberatr.cpp",
	MAME_DIR .. "src/mame/drivers/mediagx.cpp",
	MAME_DIR .. "src/mame/drivers/metalmx.cpp",
	MAME_DIR .. "src/mame/includes/metalmx.h",
	MAME_DIR .. "src/mame/drivers/mgolf.cpp",
	MAME_DIR .. "src/mame/drivers/mhavoc.cpp",
	MAME_DIR .. "src/mame/includes/mhavoc.h",
	MAME_DIR .. "src/mame/machine/mhavoc.cpp",
	MAME_DIR .. "src/mame/drivers/missile.cpp",
	MAME_DIR .. "src/mame/drivers/nitedrvr.cpp",
	MAME_DIR .. "src/mame/includes/nitedrvr.h",
	MAME_DIR .. "src/mame/machine/nitedrvr.cpp",
	MAME_DIR .. "src/mame/audio/nitedrvr.cpp",
	MAME_DIR .. "src/mame/video/nitedrvr.cpp",
	MAME_DIR .. "src/mame/drivers/offtwall.cpp",
	MAME_DIR .. "src/mame/includes/offtwall.h",
	MAME_DIR .. "src/mame/video/offtwall.cpp",
	MAME_DIR .. "src/mame/drivers/orbit.cpp",
	MAME_DIR .. "src/mame/includes/orbit.h",
	MAME_DIR .. "src/mame/audio/orbit.cpp",
	MAME_DIR .. "src/mame/video/orbit.cpp",
	MAME_DIR .. "src/mame/drivers/pong.cpp",
	MAME_DIR .. "src/mame/drivers/nl_pong.cpp",
	MAME_DIR .. "src/mame/drivers/nl_pongd.cpp",
	MAME_DIR .. "src/mame/drivers/nl_breakout.cpp",
	MAME_DIR .. "src/mame/drivers/poolshrk.cpp",
	MAME_DIR .. "src/mame/includes/poolshrk.h",
	MAME_DIR .. "src/mame/audio/poolshrk.cpp",
	MAME_DIR .. "src/mame/video/poolshrk.cpp",
	MAME_DIR .. "src/mame/drivers/quantum.cpp",
	MAME_DIR .. "src/mame/drivers/quizshow.cpp",
	MAME_DIR .. "src/mame/drivers/rampart.cpp",
	MAME_DIR .. "src/mame/includes/rampart.h",
	MAME_DIR .. "src/mame/video/rampart.cpp",
	MAME_DIR .. "src/mame/drivers/relief.cpp",
	MAME_DIR .. "src/mame/includes/relief.h",
	MAME_DIR .. "src/mame/video/relief.cpp",
	MAME_DIR .. "src/mame/drivers/runaway.cpp",
	MAME_DIR .. "src/mame/includes/runaway.h",
	MAME_DIR .. "src/mame/video/runaway.cpp",
	MAME_DIR .. "src/mame/drivers/sbrkout.cpp",
	MAME_DIR .. "src/mame/drivers/shuuz.cpp",
	MAME_DIR .. "src/mame/includes/shuuz.h",
	MAME_DIR .. "src/mame/video/shuuz.cpp",
	MAME_DIR .. "src/mame/drivers/skullxbo.cpp",
	MAME_DIR .. "src/mame/includes/skullxbo.h",
	MAME_DIR .. "src/mame/video/skullxbo.cpp",
	MAME_DIR .. "src/mame/drivers/skydiver.cpp",
	MAME_DIR .. "src/mame/includes/skydiver.h",
	MAME_DIR .. "src/mame/audio/skydiver.cpp",
	MAME_DIR .. "src/mame/video/skydiver.cpp",
	MAME_DIR .. "src/mame/drivers/skyraid.cpp",
	MAME_DIR .. "src/mame/includes/skyraid.h",
	MAME_DIR .. "src/mame/audio/skyraid.cpp",
	MAME_DIR .. "src/mame/video/skyraid.cpp",
	MAME_DIR .. "src/mame/drivers/sprint2.cpp",
	MAME_DIR .. "src/mame/includes/sprint2.h",
	MAME_DIR .. "src/mame/audio/sprint2.cpp",
	MAME_DIR .. "src/mame/video/sprint2.cpp",
	MAME_DIR .. "src/mame/drivers/sprint4.cpp",
	MAME_DIR .. "src/mame/includes/sprint4.h",
	MAME_DIR .. "src/mame/video/sprint4.cpp",
	MAME_DIR .. "src/mame/audio/sprint4.cpp",
	MAME_DIR .. "src/mame/audio/sprint4.h",
	MAME_DIR .. "src/mame/drivers/sprint8.cpp",
	MAME_DIR .. "src/mame/includes/sprint8.h",
	MAME_DIR .. "src/mame/audio/sprint8.cpp",
	MAME_DIR .. "src/mame/video/sprint8.cpp",
	MAME_DIR .. "src/mame/drivers/starshp1.cpp",
	MAME_DIR .. "src/mame/includes/starshp1.h",
	MAME_DIR .. "src/mame/audio/starshp1.cpp",
	MAME_DIR .. "src/mame/video/starshp1.cpp",
	MAME_DIR .. "src/mame/drivers/starwars.cpp",
	MAME_DIR .. "src/mame/includes/starwars.h",
	MAME_DIR .. "src/mame/machine/starwars.cpp",
	MAME_DIR .. "src/mame/audio/starwars.cpp",
	MAME_DIR .. "src/mame/drivers/subs.cpp",
	MAME_DIR .. "src/mame/includes/subs.h",
	MAME_DIR .. "src/mame/machine/subs.cpp",
	MAME_DIR .. "src/mame/audio/subs.cpp",
	MAME_DIR .. "src/mame/video/subs.cpp",
	MAME_DIR .. "src/mame/drivers/tank8.cpp",
	MAME_DIR .. "src/mame/includes/tank8.h",
	MAME_DIR .. "src/mame/audio/tank8.cpp",
	MAME_DIR .. "src/mame/video/tank8.cpp",
	MAME_DIR .. "src/mame/drivers/tempest.cpp",
	MAME_DIR .. "src/mame/drivers/thunderj.cpp",
	MAME_DIR .. "src/mame/includes/thunderj.h",
	MAME_DIR .. "src/mame/video/thunderj.cpp",
	MAME_DIR .. "src/mame/drivers/tomcat.cpp",
	MAME_DIR .. "src/mame/drivers/toobin.cpp",
	MAME_DIR .. "src/mame/includes/toobin.h",
	MAME_DIR .. "src/mame/video/toobin.cpp",
	MAME_DIR .. "src/mame/drivers/tourtabl.cpp",
	MAME_DIR .. "src/mame/video/tia.cpp",
	MAME_DIR .. "src/mame/video/tia.h",
	MAME_DIR .. "src/mame/drivers/triplhnt.cpp",
	MAME_DIR .. "src/mame/includes/triplhnt.h",
	MAME_DIR .. "src/mame/audio/triplhnt.cpp",
	MAME_DIR .. "src/mame/video/triplhnt.cpp",
	MAME_DIR .. "src/mame/drivers/tunhunt.cpp",
	MAME_DIR .. "src/mame/includes/tunhunt.h",
	MAME_DIR .. "src/mame/video/tunhunt.cpp",
	MAME_DIR .. "src/mame/drivers/ultratnk.cpp",
	MAME_DIR .. "src/mame/includes/ultratnk.h",
	MAME_DIR .. "src/mame/video/ultratnk.cpp",
	MAME_DIR .. "src/mame/drivers/videopin.cpp",
	MAME_DIR .. "src/mame/includes/videopin.h",
	MAME_DIR .. "src/mame/audio/videopin.cpp",
	MAME_DIR .. "src/mame/video/videopin.cpp",
	MAME_DIR .. "src/mame/drivers/vindictr.cpp",
	MAME_DIR .. "src/mame/includes/vindictr.h",
	MAME_DIR .. "src/mame/video/vindictr.cpp",
	MAME_DIR .. "src/mame/drivers/wolfpack.cpp",
	MAME_DIR .. "src/mame/includes/wolfpack.h",
	MAME_DIR .. "src/mame/video/wolfpack.cpp",
	MAME_DIR .. "src/mame/drivers/xybots.cpp",
	MAME_DIR .. "src/mame/includes/xybots.h",
	MAME_DIR .. "src/mame/video/xybots.cpp",
	MAME_DIR .. "src/mame/machine/asic65.cpp",
	MAME_DIR .. "src/mame/machine/asic65.h",
	MAME_DIR .. "src/mame/machine/atari_vg.cpp",
	MAME_DIR .. "src/mame/machine/atari_vg.h",
	MAME_DIR .. "src/mame/machine/atarigen.cpp",
	MAME_DIR .. "src/mame/machine/atarigen.h",
	MAME_DIR .. "src/mame/machine/mathbox.cpp",
	MAME_DIR .. "src/mame/machine/mathbox.h",
	MAME_DIR .. "src/mame/machine/slapstic.cpp",
	MAME_DIR .. "src/mame/audio/atarijsa.cpp",
	MAME_DIR .. "src/mame/audio/atarijsa.h",
	MAME_DIR .. "src/mame/audio/cage.cpp",
	MAME_DIR .. "src/mame/audio/cage.h",
	MAME_DIR .. "src/mame/audio/redbaron.cpp",
	MAME_DIR .. "src/mame/audio/redbaron.h",
	MAME_DIR .. "src/mame/video/atarimo.cpp",
	MAME_DIR .. "src/mame/video/atarimo.h",
	MAME_DIR .. "src/mame/video/atarirle.cpp",
	MAME_DIR .. "src/mame/video/atarirle.h",
}

createMAMEProjects(_target, _subtarget, "atlus")
files {
	MAME_DIR .. "src/mame/drivers/blmbycar.cpp",
	MAME_DIR .. "src/mame/includes/blmbycar.h",
	MAME_DIR .. "src/mame/video/blmbycar.cpp",
	MAME_DIR .. "src/mame/drivers/ohmygod.cpp",
	MAME_DIR .. "src/mame/includes/ohmygod.h",
	MAME_DIR .. "src/mame/video/ohmygod.cpp",
	MAME_DIR .. "src/mame/drivers/powerins.cpp",
	MAME_DIR .. "src/mame/includes/powerins.h",
	MAME_DIR .. "src/mame/video/powerins.cpp",
	MAME_DIR .. "src/mame/drivers/bowltry.cpp",
}

createMAMEProjects(_target, _subtarget, "barcrest")
files {
	MAME_DIR .. "src/mame/drivers/mpu2.cpp",
	MAME_DIR .. "src/mame/drivers/mpu3.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4hw.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4sw.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4.cpp",
	MAME_DIR .. "src/mame/includes/mpu4.h",
	MAME_DIR .. "src/mame/drivers/mpu4mod2sw.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4mod4yam.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4plasma.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4dealem.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4vid.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4avan.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4union.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4concept.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4empire.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4mdm.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4crystal.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4bwb.cpp",
	MAME_DIR .. "src/mame/drivers/mpu4misc.cpp",
	MAME_DIR .. "src/mame/drivers/mpu5hw.cpp",
	MAME_DIR .. "src/mame/drivers/mpu5.cpp",
	MAME_DIR .. "src/mame/video/awpvid.cpp",
	MAME_DIR .. "src/mame/video/awpvid.h",
	MAME_DIR .. "src/mame/machine/meters.cpp",
	MAME_DIR .. "src/mame/machine/meters.h",
}

createMAMEProjects(_target, _subtarget, "bfm")
files {
	MAME_DIR .. "src/mame/drivers/bfcobra.cpp",
	MAME_DIR .. "src/mame/machine/bfm_comn.cpp",
	MAME_DIR .. "src/mame/machine/bfm_comn.h",
	MAME_DIR .. "src/mame/drivers/bfm_sc1.cpp",
	MAME_DIR .. "src/mame/drivers/bfm_sc2.cpp",
	MAME_DIR .. "src/mame/video/bfm_adr2.cpp",
	MAME_DIR .. "src/mame/video/bfm_adr2.h",
	MAME_DIR .. "src/mame/drivers/bfm_sc4.cpp",
	MAME_DIR .. "src/mame/drivers/bfm_sc4h.cpp",
	MAME_DIR .. "src/mame/drivers/bfm_sc5.cpp",
	MAME_DIR .. "src/mame/includes/bfm_sc5.h",
	MAME_DIR .. "src/mame/drivers/bfm_sc5sw.cpp",
	MAME_DIR .. "src/mame/drivers/bfm_ad5.cpp",
	MAME_DIR .. "src/mame/includes/bfm_ad5.h",
	MAME_DIR .. "src/mame/drivers/bfm_ad5sw.cpp",
	MAME_DIR .. "src/mame/drivers/bfm_sc45_helper.cpp",
	MAME_DIR .. "src/mame/drivers/bfm_sc45_helper.h",
	MAME_DIR .. "src/mame/includes/bfm_sc45.h",
	MAME_DIR .. "src/mame/drivers/bfm_swp.cpp",
	MAME_DIR .. "src/mame/drivers/bfmsys83.cpp",
	MAME_DIR .. "src/mame/drivers/bfmsys85.cpp",
	MAME_DIR .. "src/mame/machine/sec.cpp",
	MAME_DIR .. "src/mame/machine/sec.h",
	MAME_DIR .. "src/mame/machine/bfm_bd1.cpp",
	MAME_DIR .. "src/mame/machine/bfm_bd1.h",
	MAME_DIR .. "src/mame/machine/bfm_bda.cpp",
	MAME_DIR .. "src/mame/machine/bfm_bda.h",
	MAME_DIR .. "src/mame/video/bfm_dm01.cpp",
	MAME_DIR .. "src/mame/video/bfm_dm01.h",
	MAME_DIR .. "src/mame/drivers/rastersp.cpp",
}

createMAMEProjects(_target, _subtarget, "bmc")
files {
	MAME_DIR .. "src/mame/drivers/bmcbowl.cpp",
	MAME_DIR .. "src/mame/drivers/koftball.cpp",
	MAME_DIR .. "src/mame/drivers/popobear.cpp",
	MAME_DIR .. "src/mame/drivers/bmcpokr.cpp",
}

createMAMEProjects(_target, _subtarget, "capcom")
files {
	MAME_DIR .. "src/mame/drivers/1942.cpp",
	MAME_DIR .. "src/mame/includes/1942.h",
	MAME_DIR .. "src/mame/video/1942.cpp",
	MAME_DIR .. "src/mame/drivers/1943.cpp",
	MAME_DIR .. "src/mame/includes/1943.h",
	MAME_DIR .. "src/mame/video/1943.cpp",
	MAME_DIR .. "src/mame/drivers/alien.cpp",
	MAME_DIR .. "src/mame/drivers/bionicc.cpp",
	MAME_DIR .. "src/mame/includes/bionicc.h",
	MAME_DIR .. "src/mame/video/bionicc.cpp",
	MAME_DIR .. "src/mame/drivers/supduck.cpp",
	MAME_DIR .. "src/mame/video/tigeroad_spr.cpp",
	MAME_DIR .. "src/mame/video/tigeroad_spr.h",
	MAME_DIR .. "src/mame/drivers/blktiger.cpp",
	MAME_DIR .. "src/mame/includes/blktiger.h",
	MAME_DIR .. "src/mame/video/blktiger.cpp",
	MAME_DIR .. "src/mame/drivers/cbasebal.cpp",
	MAME_DIR .. "src/mame/includes/cbasebal.h",
	MAME_DIR .. "src/mame/video/cbasebal.cpp",
	MAME_DIR .. "src/mame/drivers/commando.cpp",
	MAME_DIR .. "src/mame/includes/commando.h",
	MAME_DIR .. "src/mame/video/commando.cpp",
	MAME_DIR .. "src/mame/drivers/cps1.cpp",
	MAME_DIR .. "src/mame/includes/cps1.h",
	MAME_DIR .. "src/mame/video/cps1.cpp",
	MAME_DIR .. "src/mame/drivers/kenseim.cpp",
	MAME_DIR .. "src/mame/drivers/cps2.cpp",
	MAME_DIR .. "src/mame/machine/cps2crpt.cpp",
	MAME_DIR .. "src/mame/machine/cps2crypt.h",
	MAME_DIR .. "src/mame/drivers/cps3.cpp",
	MAME_DIR .. "src/mame/includes/cps3.h",
	MAME_DIR .. "src/mame/audio/cps3.cpp",
	MAME_DIR .. "src/mame/audio/cps3.h",
	MAME_DIR .. "src/mame/drivers/egghunt.cpp",
	MAME_DIR .. "src/mame/drivers/exedexes.cpp",
	MAME_DIR .. "src/mame/includes/exedexes.h",
	MAME_DIR .. "src/mame/video/exedexes.cpp",
	MAME_DIR .. "src/mame/drivers/fcrash.cpp",
	MAME_DIR .. "src/mame/drivers/gng.cpp",
	MAME_DIR .. "src/mame/includes/gng.h",
	MAME_DIR .. "src/mame/video/gng.cpp",
	MAME_DIR .. "src/mame/drivers/gunsmoke.cpp",
	MAME_DIR .. "src/mame/includes/gunsmoke.h",
	MAME_DIR .. "src/mame/video/gunsmoke.cpp",
	MAME_DIR .. "src/mame/drivers/higemaru.cpp",
	MAME_DIR .. "src/mame/includes/higemaru.h",
	MAME_DIR .. "src/mame/video/higemaru.cpp",
	MAME_DIR .. "src/mame/drivers/lastduel.cpp",
	MAME_DIR .. "src/mame/includes/lastduel.h",
	MAME_DIR .. "src/mame/video/lastduel.cpp",
	MAME_DIR .. "src/mame/drivers/lwings.cpp",
	MAME_DIR .. "src/mame/includes/lwings.h",
	MAME_DIR .. "src/mame/video/lwings.cpp",
	MAME_DIR .. "src/mame/drivers/mitchell.cpp",
	MAME_DIR .. "src/mame/includes/mitchell.h",
	MAME_DIR .. "src/mame/video/mitchell.cpp",
	MAME_DIR .. "src/mame/drivers/sf.cpp",
	MAME_DIR .. "src/mame/includes/sf.h",
	MAME_DIR .. "src/mame/video/sf.cpp",
	MAME_DIR .. "src/mame/drivers/sidearms.cpp",
	MAME_DIR .. "src/mame/includes/sidearms.h",
	MAME_DIR .. "src/mame/video/sidearms.cpp",
	MAME_DIR .. "src/mame/drivers/sonson.cpp",
	MAME_DIR .. "src/mame/includes/sonson.h",
	MAME_DIR .. "src/mame/video/sonson.cpp",
	MAME_DIR .. "src/mame/drivers/srumbler.cpp",
	MAME_DIR .. "src/mame/includes/srumbler.h",
	MAME_DIR .. "src/mame/video/srumbler.cpp",
	MAME_DIR .. "src/mame/drivers/tigeroad.cpp",
	MAME_DIR .. "src/mame/includes/tigeroad.h",
	MAME_DIR .. "src/mame/video/tigeroad.cpp",
	MAME_DIR .. "src/mame/machine/tigeroad.cpp",
	MAME_DIR .. "src/mame/drivers/vulgus.cpp",
	MAME_DIR .. "src/mame/includes/vulgus.h",
	MAME_DIR .. "src/mame/video/vulgus.cpp",
	MAME_DIR .. "src/mame/machine/kabuki.cpp",
	MAME_DIR .. "src/mame/machine/kabuki.h",
	MAME_DIR .. "src/mame/drivers/tvcapcom.cpp",
}

createMAMEProjects(_target, _subtarget, "cinemat")
files {
	MAME_DIR .. "src/mame/drivers/ataxx.cpp",
	MAME_DIR .. "src/mame/drivers/cinemat.cpp",
	MAME_DIR .. "src/mame/includes/cinemat.h",
	MAME_DIR .. "src/mame/audio/cinemat.cpp",
	MAME_DIR .. "src/mame/video/cinemat.cpp",
	MAME_DIR .. "src/mame/drivers/cchasm.cpp",
	MAME_DIR .. "src/mame/includes/cchasm.h",
	MAME_DIR .. "src/mame/machine/cchasm.cpp",
	MAME_DIR .. "src/mame/audio/cchasm.cpp",
	MAME_DIR .. "src/mame/video/cchasm.cpp",
	MAME_DIR .. "src/mame/drivers/dlair.cpp",
	MAME_DIR .. "src/mame/drivers/dlair2.cpp",
	MAME_DIR .. "src/mame/drivers/embargo.cpp",
	MAME_DIR .. "src/mame/drivers/jack.cpp",
	MAME_DIR .. "src/mame/includes/jack.h",
	MAME_DIR .. "src/mame/video/jack.cpp",
	MAME_DIR .. "src/mame/drivers/leland.cpp",
	MAME_DIR .. "src/mame/includes/leland.h",
	MAME_DIR .. "src/mame/machine/leland.cpp",
	MAME_DIR .. "src/mame/audio/leland.cpp",
	MAME_DIR .. "src/mame/video/leland.cpp",
}

createMAMEProjects(_target, _subtarget, "comad")
files {
	MAME_DIR .. "src/mame/drivers/funybubl.cpp",
	MAME_DIR .. "src/mame/includes/funybubl.h",
	MAME_DIR .. "src/mame/video/funybubl.cpp",
	MAME_DIR .. "src/mame/drivers/galspnbl.cpp",
	MAME_DIR .. "src/mame/includes/galspnbl.h",
	MAME_DIR .. "src/mame/video/galspnbl.cpp",
	MAME_DIR .. "src/mame/drivers/zerozone.cpp",
	MAME_DIR .. "src/mame/includes/zerozone.h",
	MAME_DIR .. "src/mame/video/zerozone.cpp",
}

createMAMEProjects(_target, _subtarget, "cvs")
files {
	MAME_DIR .. "src/mame/drivers/cvs.cpp",
	MAME_DIR .. "src/mame/includes/cvs.h",
	MAME_DIR .. "src/mame/video/cvs.cpp",
	MAME_DIR .. "src/mame/drivers/galaxia.cpp",
	MAME_DIR .. "src/mame/includes/galaxia.h",
	MAME_DIR .. "src/mame/video/galaxia.cpp",
	MAME_DIR .. "src/mame/drivers/quasar.cpp",
	MAME_DIR .. "src/mame/includes/quasar.h",
	MAME_DIR .. "src/mame/video/quasar.cpp",
}

createMAMEProjects(_target, _subtarget, "dataeast")
files {
	MAME_DIR .. "src/mame/drivers/actfancr.cpp",
	MAME_DIR .. "src/mame/includes/actfancr.h",
	MAME_DIR .. "src/mame/video/actfancr.cpp",
	MAME_DIR .. "src/mame/drivers/astrof.cpp",
	MAME_DIR .. "src/mame/includes/astrof.h",
	MAME_DIR .. "src/mame/audio/astrof.cpp",
	MAME_DIR .. "src/mame/drivers/backfire.cpp",
	MAME_DIR .. "src/mame/drivers/battlera.cpp",
	MAME_DIR .. "src/mame/includes/battlera.h",
	MAME_DIR .. "src/mame/drivers/boogwing.cpp",
	MAME_DIR .. "src/mame/includes/boogwing.h",
	MAME_DIR .. "src/mame/video/boogwing.cpp",
	MAME_DIR .. "src/mame/drivers/brkthru.cpp",
	MAME_DIR .. "src/mame/includes/brkthru.h",
	MAME_DIR .. "src/mame/video/brkthru.cpp",
	MAME_DIR .. "src/mame/drivers/btime.cpp",
	MAME_DIR .. "src/mame/includes/btime.h",
	MAME_DIR .. "src/mame/machine/btime.cpp",
	MAME_DIR .. "src/mame/video/btime.cpp",
	MAME_DIR .. "src/mame/drivers/bwing.cpp",
	MAME_DIR .. "src/mame/includes/bwing.h",
	MAME_DIR .. "src/mame/video/bwing.cpp",
	MAME_DIR .. "src/mame/drivers/cbuster.cpp",
	MAME_DIR .. "src/mame/includes/cbuster.h",
	MAME_DIR .. "src/mame/video/cbuster.cpp",
	MAME_DIR .. "src/mame/drivers/chanbara.cpp",
	MAME_DIR .. "src/mame/drivers/cninja.cpp",
	MAME_DIR .. "src/mame/includes/cninja.h",
	MAME_DIR .. "src/mame/video/cninja.cpp",
	MAME_DIR .. "src/mame/drivers/cntsteer.cpp",
	MAME_DIR .. "src/mame/drivers/compgolf.cpp",
	MAME_DIR .. "src/mame/includes/compgolf.h",
	MAME_DIR .. "src/mame/video/compgolf.cpp",
	MAME_DIR .. "src/mame/drivers/darkseal.cpp",
	MAME_DIR .. "src/mame/includes/darkseal.h",
	MAME_DIR .. "src/mame/video/darkseal.cpp",
	MAME_DIR .. "src/mame/drivers/dassault.cpp",
	MAME_DIR .. "src/mame/includes/dassault.h",
	MAME_DIR .. "src/mame/video/dassault.cpp",
	MAME_DIR .. "src/mame/drivers/dblewing.cpp",
	MAME_DIR .. "src/mame/drivers/dec0.cpp",
	MAME_DIR .. "src/mame/includes/dec0.h",
	MAME_DIR .. "src/mame/machine/dec0.cpp",
	MAME_DIR .. "src/mame/video/dec0.cpp",
	MAME_DIR .. "src/mame/drivers/dec8.cpp",
	MAME_DIR .. "src/mame/includes/dec8.h",
	MAME_DIR .. "src/mame/video/dec8.cpp",
	MAME_DIR .. "src/mame/machine/deco222.cpp",
	MAME_DIR .. "src/mame/machine/deco222.h",
	MAME_DIR .. "src/mame/machine/decocpu7.cpp",
	MAME_DIR .. "src/mame/machine/decocpu7.h",
	MAME_DIR .. "src/mame/machine/decocpu6.cpp",
	MAME_DIR .. "src/mame/machine/decocpu6.h",
	MAME_DIR .. "src/mame/drivers/deco_ld.cpp",
	MAME_DIR .. "src/mame/drivers/deco_mlc.cpp",
	MAME_DIR .. "src/mame/includes/deco_mlc.h",
	MAME_DIR .. "src/mame/video/deco_mlc.cpp",
	MAME_DIR .. "src/mame/drivers/deco156.cpp",
	MAME_DIR .. "src/mame/machine/deco156.cpp",
	MAME_DIR .. "src/mame/drivers/deco32.cpp",
	MAME_DIR .. "src/mame/includes/deco32.h",
	MAME_DIR .. "src/mame/video/deco32.cpp",
	MAME_DIR .. "src/mame/video/dvi.cpp",
	MAME_DIR .. "src/mame/video/deco_zoomspr.cpp",
	MAME_DIR .. "src/mame/video/deco_zoomspr.h",
	MAME_DIR .. "src/mame/drivers/decocass.cpp",
	MAME_DIR .. "src/mame/includes/decocass.h",
	MAME_DIR .. "src/mame/machine/decocass.cpp",
	MAME_DIR .. "src/mame/machine/decocass_tape.cpp",
	MAME_DIR .. "src/mame/machine/decocass_tape.h",
	MAME_DIR .. "src/mame/video/decocass.cpp",
	MAME_DIR .. "src/mame/drivers/deshoros.cpp",
	MAME_DIR .. "src/mame/drivers/dietgo.cpp",
	MAME_DIR .. "src/mame/includes/dietgo.h",
	MAME_DIR .. "src/mame/video/dietgo.cpp",
	MAME_DIR .. "src/mame/drivers/dreambal.cpp",
	MAME_DIR .. "src/mame/drivers/exprraid.cpp",
	MAME_DIR .. "src/mame/includes/exprraid.h",
	MAME_DIR .. "src/mame/video/exprraid.cpp",
	MAME_DIR .. "src/mame/drivers/firetrap.cpp",
	MAME_DIR .. "src/mame/includes/firetrap.h",
	MAME_DIR .. "src/mame/video/firetrap.cpp",
	MAME_DIR .. "src/mame/drivers/funkyjet.cpp",
	MAME_DIR .. "src/mame/includes/funkyjet.h",
	MAME_DIR .. "src/mame/video/funkyjet.cpp",
	MAME_DIR .. "src/mame/drivers/karnov.cpp",
	MAME_DIR .. "src/mame/includes/karnov.h",
	MAME_DIR .. "src/mame/video/karnov.cpp",
	MAME_DIR .. "src/mame/drivers/kchamp.cpp",
	MAME_DIR .. "src/mame/includes/kchamp.h",
	MAME_DIR .. "src/mame/video/kchamp.cpp",
	MAME_DIR .. "src/mame/drivers/kingobox.cpp",
	MAME_DIR .. "src/mame/includes/kingobox.h",
	MAME_DIR .. "src/mame/video/kingobox.cpp",
	MAME_DIR .. "src/mame/drivers/lemmings.cpp",
	MAME_DIR .. "src/mame/includes/lemmings.h",
	MAME_DIR .. "src/mame/video/lemmings.cpp",
	MAME_DIR .. "src/mame/drivers/liberate.cpp",
	MAME_DIR .. "src/mame/includes/liberate.h",
	MAME_DIR .. "src/mame/video/liberate.cpp",
	MAME_DIR .. "src/mame/drivers/madalien.cpp",
	MAME_DIR .. "src/mame/includes/madalien.h",
	MAME_DIR .. "src/mame/audio/madalien.cpp",
	MAME_DIR .. "src/mame/video/madalien.cpp",
	MAME_DIR .. "src/mame/drivers/madmotor.cpp",
	MAME_DIR .. "src/mame/includes/madmotor.h",
	MAME_DIR .. "src/mame/video/madmotor.cpp",
	MAME_DIR .. "src/mame/drivers/metlclsh.cpp",
	MAME_DIR .. "src/mame/includes/metlclsh.h",
	MAME_DIR .. "src/mame/video/metlclsh.cpp",
	MAME_DIR .. "src/mame/drivers/mirage.cpp",
	MAME_DIR .. "src/mame/drivers/pcktgal.cpp",
	MAME_DIR .. "src/mame/includes/pcktgal.h",
	MAME_DIR .. "src/mame/video/pcktgal.cpp",
	MAME_DIR .. "src/mame/drivers/pktgaldx.cpp",
	MAME_DIR .. "src/mame/includes/pktgaldx.h",
	MAME_DIR .. "src/mame/video/pktgaldx.cpp",
	MAME_DIR .. "src/mame/drivers/progolf.cpp",
	MAME_DIR .. "src/mame/drivers/rohga.cpp",
	MAME_DIR .. "src/mame/includes/rohga.h",
	MAME_DIR .. "src/mame/video/rohga.cpp",
	MAME_DIR .. "src/mame/drivers/shootout.cpp",
	MAME_DIR .. "src/mame/includes/shootout.h",
	MAME_DIR .. "src/mame/video/shootout.cpp",
	MAME_DIR .. "src/mame/drivers/sidepckt.cpp",
	MAME_DIR .. "src/mame/includes/sidepckt.h",
	MAME_DIR .. "src/mame/video/sidepckt.cpp",
	MAME_DIR .. "src/mame/drivers/simpl156.cpp",
	MAME_DIR .. "src/mame/includes/simpl156.h",
	MAME_DIR .. "src/mame/video/simpl156.cpp",
	MAME_DIR .. "src/mame/drivers/sshangha.cpp",
	MAME_DIR .. "src/mame/includes/sshangha.h",
	MAME_DIR .. "src/mame/video/sshangha.cpp",
	MAME_DIR .. "src/mame/drivers/stadhero.cpp",
	MAME_DIR .. "src/mame/includes/stadhero.h",
	MAME_DIR .. "src/mame/video/stadhero.cpp",
	MAME_DIR .. "src/mame/drivers/supbtime.cpp",
	MAME_DIR .. "src/mame/includes/supbtime.h",
	MAME_DIR .. "src/mame/video/supbtime.cpp",
	MAME_DIR .. "src/mame/drivers/tryout.cpp",
	MAME_DIR .. "src/mame/includes/tryout.h",
	MAME_DIR .. "src/mame/video/tryout.cpp",
	MAME_DIR .. "src/mame/drivers/tumbleb.cpp",
	MAME_DIR .. "src/mame/includes/tumbleb.h",
	MAME_DIR .. "src/mame/video/tumbleb.cpp",
	MAME_DIR .. "src/mame/drivers/tumblep.cpp",
	MAME_DIR .. "src/mame/includes/tumblep.h",
	MAME_DIR .. "src/mame/video/tumblep.cpp",
	MAME_DIR .. "src/mame/drivers/vaportra.cpp",
	MAME_DIR .. "src/mame/includes/vaportra.h",
	MAME_DIR .. "src/mame/video/vaportra.cpp",
	MAME_DIR .. "src/mame/machine/deco102.cpp",
	MAME_DIR .. "src/mame/machine/decocrpt.cpp",
	MAME_DIR .. "src/mame/includes/decocrpt.h",
	MAME_DIR .. "src/mame/machine/deco104.cpp",
	MAME_DIR .. "src/mame/machine/deco104.h",
	MAME_DIR .. "src/mame/machine/deco146.cpp",
	MAME_DIR .. "src/mame/machine/deco146.h",
	MAME_DIR .. "src/mame/video/decbac06.cpp",
	MAME_DIR .. "src/mame/video/decbac06.h",
	MAME_DIR .. "src/mame/video/deco16ic.cpp",
	MAME_DIR .. "src/mame/video/deco16ic.h",
	MAME_DIR .. "src/mame/video/decocomn.cpp",
	MAME_DIR .. "src/mame/video/decocomn.h",
	MAME_DIR .. "src/mame/video/decospr.cpp",
	MAME_DIR .. "src/mame/video/decospr.h",
	MAME_DIR .. "src/mame/video/decmxc06.cpp",
	MAME_DIR .. "src/mame/video/decmxc06.h",
	MAME_DIR .. "src/mame/video/deckarn.cpp",
	MAME_DIR .. "src/mame/video/deckarn.h",
}

createMAMEProjects(_target, _subtarget, "dgrm")
files {
	MAME_DIR .. "src/mame/drivers/blackt96.cpp",
	MAME_DIR .. "src/mame/drivers/pokechmp.cpp",
	MAME_DIR .. "src/mame/includes/pokechmp.h",
	MAME_DIR .. "src/mame/video/pokechmp.cpp",
}

createMAMEProjects(_target, _subtarget, "dooyong")
files {
	MAME_DIR .. "src/mame/drivers/dooyong.cpp",
	MAME_DIR .. "src/mame/includes/dooyong.h",
	MAME_DIR .. "src/mame/video/dooyong.cpp",
	MAME_DIR .. "src/mame/drivers/gundealr.cpp",
	MAME_DIR .. "src/mame/includes/gundealr.h",
	MAME_DIR .. "src/mame/video/gundealr.cpp",
}

createMAMEProjects(_target, _subtarget, "dynax")
files {
	MAME_DIR .. "src/mame/drivers/ddenlovr.cpp",
	MAME_DIR .. "src/mame/drivers/dynax.cpp",
	MAME_DIR .. "src/mame/includes/dynax.h",
	MAME_DIR .. "src/mame/video/dynax.cpp",
	MAME_DIR .. "src/mame/drivers/hnayayoi.cpp",
	MAME_DIR .. "src/mame/includes/hnayayoi.h",
	MAME_DIR .. "src/mame/video/hnayayoi.cpp",
	MAME_DIR .. "src/mame/drivers/realbrk.cpp",
	MAME_DIR .. "src/mame/includes/realbrk.h",
	MAME_DIR .. "src/mame/video/realbrk.cpp",
	MAME_DIR .. "src/mame/drivers/royalmah.cpp",
}

createMAMEProjects(_target, _subtarget, "edevices")
files {
	MAME_DIR .. "src/mame/drivers/diverboy.cpp",
	MAME_DIR .. "src/mame/drivers/fantland.cpp",
	MAME_DIR .. "src/mame/includes/fantland.h",
	MAME_DIR .. "src/mame/video/fantland.cpp",
	MAME_DIR .. "src/mame/drivers/mwarr.cpp",
	MAME_DIR .. "src/mame/drivers/mugsmash.cpp",
	MAME_DIR .. "src/mame/includes/mugsmash.h",
	MAME_DIR .. "src/mame/video/mugsmash.cpp",
	MAME_DIR .. "src/mame/drivers/ppmast93.cpp",
	MAME_DIR .. "src/mame/drivers/pzletime.cpp",
	MAME_DIR .. "src/mame/drivers/stlforce.cpp",
	MAME_DIR .. "src/mame/includes/stlforce.h",
	MAME_DIR .. "src/mame/video/stlforce.cpp",
	MAME_DIR .. "src/mame/drivers/twins.cpp",
}

createMAMEProjects(_target, _subtarget, "eolith")
files {
	MAME_DIR .. "src/mame/drivers/eolith.cpp",
	MAME_DIR .. "src/mame/includes/eolith.h",
	MAME_DIR .. "src/mame/video/eolith.cpp",
	MAME_DIR .. "src/mame/drivers/eolith16.cpp",
	MAME_DIR .. "src/mame/drivers/eolithsp.cpp",
	MAME_DIR .. "src/mame/drivers/ghosteo.cpp",
	MAME_DIR .. "src/mame/drivers/vegaeo.cpp",
}

createMAMEProjects(_target, _subtarget, "excelent")
files {
	MAME_DIR .. "src/mame/drivers/aquarium.cpp",
	MAME_DIR .. "src/mame/includes/aquarium.h",
	MAME_DIR .. "src/mame/video/aquarium.cpp",
	MAME_DIR .. "src/mame/drivers/d9final.cpp",
	MAME_DIR .. "src/mame/drivers/dblcrown.cpp",
	MAME_DIR .. "src/mame/drivers/gcpinbal.cpp",
	MAME_DIR .. "src/mame/includes/gcpinbal.h",
	MAME_DIR .. "src/mame/video/gcpinbal.cpp",
	MAME_DIR .. "src/mame/video/excellent_spr.cpp",
	MAME_DIR .. "src/mame/video/excellent_spr.h",
	MAME_DIR .. "src/mame/drivers/lastbank.cpp",
}

createMAMEProjects(_target, _subtarget, "exidy")
files {
	MAME_DIR .. "src/mame/drivers/carpolo.cpp",
	MAME_DIR .. "src/mame/includes/carpolo.h",
	MAME_DIR .. "src/mame/machine/carpolo.cpp",
	MAME_DIR .. "src/mame/video/carpolo.cpp",
	MAME_DIR .. "src/mame/drivers/circus.cpp",
	MAME_DIR .. "src/mame/includes/circus.h",
	MAME_DIR .. "src/mame/audio/circus.cpp",
	MAME_DIR .. "src/mame/video/circus.cpp",
	MAME_DIR .. "src/mame/drivers/exidy.cpp",
	MAME_DIR .. "src/mame/includes/exidy.h",
	MAME_DIR .. "src/mame/audio/exidy.cpp",
	MAME_DIR .. "src/mame/audio/exidy.h",
	MAME_DIR .. "src/mame/video/exidy.cpp",
	MAME_DIR .. "src/mame/audio/targ.cpp",
	MAME_DIR .. "src/mame/drivers/exidy440.cpp",
	MAME_DIR .. "src/mame/includes/exidy440.h",
	MAME_DIR .. "src/mame/audio/exidy440.cpp",
	MAME_DIR .. "src/mame/audio/exidy440.h",
	MAME_DIR .. "src/mame/video/exidy440.cpp",
	MAME_DIR .. "src/mame/drivers/exidyttl.cpp",
	MAME_DIR .. "src/mame/drivers/maxaflex.cpp",
	MAME_DIR .. "src/mame/machine/atari.cpp",
	MAME_DIR .. "src/mame/includes/atari.h",
	MAME_DIR .. "src/mame/video/atari.cpp",
	MAME_DIR .. "src/mame/video/antic.cpp",
	MAME_DIR .. "src/mame/video/antic.h",
	MAME_DIR .. "src/mame/video/gtia.cpp",
	MAME_DIR .. "src/mame/video/gtia.h",
	MAME_DIR .. "src/mame/drivers/starfire.cpp",
	MAME_DIR .. "src/mame/includes/starfire.h",
	MAME_DIR .. "src/mame/video/starfire.cpp",
	MAME_DIR .. "src/mame/drivers/vertigo.cpp",
	MAME_DIR .. "src/mame/includes/vertigo.h",
	MAME_DIR .. "src/mame/machine/vertigo.cpp",
	MAME_DIR .. "src/mame/video/vertigo.cpp",
	MAME_DIR .. "src/mame/drivers/victory.cpp",
	MAME_DIR .. "src/mame/includes/victory.h",
	MAME_DIR .. "src/mame/video/victory.cpp",
}

createMAMEProjects(_target, _subtarget, "f32")
files {
	MAME_DIR .. "src/mame/drivers/crospang.cpp",
	MAME_DIR .. "src/mame/includes/crospang.h",
	MAME_DIR .. "src/mame/video/crospang.cpp",
	MAME_DIR .. "src/mame/drivers/silvmil.cpp",
	MAME_DIR .. "src/mame/drivers/f-32.cpp",
}

createMAMEProjects(_target, _subtarget, "funworld")
files {
	MAME_DIR .. "src/mame/drivers/4roses.cpp",
	MAME_DIR .. "src/mame/drivers/funworld.cpp",
	MAME_DIR .. "src/mame/includes/funworld.h",
	MAME_DIR .. "src/mame/video/funworld.cpp",
	MAME_DIR .. "src/mame/drivers/snookr10.cpp",
	MAME_DIR .. "src/mame/includes/snookr10.h",
	MAME_DIR .. "src/mame/video/snookr10.cpp",
}

createMAMEProjects(_target, _subtarget, "fuuki")
files {
	MAME_DIR .. "src/mame/drivers/fuukifg2.cpp",
	MAME_DIR .. "src/mame/includes/fuukifg2.h",
	MAME_DIR .. "src/mame/video/fuukifg2.cpp",
	MAME_DIR .. "src/mame/drivers/fuukifg3.cpp",
	MAME_DIR .. "src/mame/includes/fuukifg3.h",
	MAME_DIR .. "src/mame/video/fuukifg3.cpp",
	MAME_DIR .. "src/mame/video/fuukifg.cpp",
	MAME_DIR .. "src/mame/video/fuukifg.h",
}

createMAMEProjects(_target, _subtarget, "gaelco")
files {
	MAME_DIR .. "src/mame/drivers/atvtrack.cpp",
	MAME_DIR .. "src/mame/drivers/gaelco.cpp",
	MAME_DIR .. "src/mame/includes/gaelco.h",
	MAME_DIR .. "src/mame/video/gaelco.cpp",
	MAME_DIR .. "src/mame/machine/gaelcrpt.cpp",
	MAME_DIR .. "src/mame/includes/gaelcrpt.h",
	MAME_DIR .. "src/mame/drivers/gaelco2.cpp",
	MAME_DIR .. "src/mame/includes/gaelco2.h",
	MAME_DIR .. "src/mame/machine/gaelco2.cpp",
	MAME_DIR .. "src/mame/video/gaelco2.cpp",
	MAME_DIR .. "src/mame/drivers/gaelco3d.cpp",
	MAME_DIR .. "src/mame/includes/gaelco3d.h",
	MAME_DIR .. "src/mame/video/gaelco3d.cpp",
	MAME_DIR .. "src/mame/machine/gaelco3d.cpp",
	MAME_DIR .. "src/mame/machine/gaelco3d.h",
	MAME_DIR .. "src/mame/drivers/glass.cpp",
	MAME_DIR .. "src/mame/includes/glass.h",
	MAME_DIR .. "src/mame/video/glass.cpp",
	MAME_DIR .. "src/mame/drivers/mastboy.cpp",
	MAME_DIR .. "src/mame/drivers/rollext.cpp",
	MAME_DIR .. "src/mame/drivers/splash.cpp",
	MAME_DIR .. "src/mame/includes/splash.h",
	MAME_DIR .. "src/mame/video/splash.cpp",
	MAME_DIR .. "src/mame/drivers/targeth.cpp",
	MAME_DIR .. "src/mame/includes/targeth.h",
	MAME_DIR .. "src/mame/video/targeth.cpp",
	MAME_DIR .. "src/mame/drivers/thoop2.cpp",
	MAME_DIR .. "src/mame/includes/thoop2.h",
	MAME_DIR .. "src/mame/video/thoop2.cpp",
	MAME_DIR .. "src/mame/drivers/tokyocop.cpp",
	MAME_DIR .. "src/mame/drivers/wrally.cpp",
	MAME_DIR .. "src/mame/includes/wrally.h",
	MAME_DIR .. "src/mame/machine/wrally.cpp",
	MAME_DIR .. "src/mame/video/wrally.cpp",
	MAME_DIR .. "src/mame/drivers/xorworld.cpp",
	MAME_DIR .. "src/mame/includes/xorworld.h",
	MAME_DIR .. "src/mame/video/xorworld.cpp",
}

createMAMEProjects(_target, _subtarget, "gameplan")
files {
	MAME_DIR .. "src/mame/drivers/enigma2.cpp",
	MAME_DIR .. "src/mame/drivers/gameplan.cpp",
	MAME_DIR .. "src/mame/includes/gameplan.h",
	MAME_DIR .. "src/mame/video/gameplan.cpp",
	MAME_DIR .. "src/mame/drivers/toratora.cpp",
}

createMAMEProjects(_target, _subtarget, "gametron")
files {
	MAME_DIR .. "src/mame/drivers/gatron.cpp",
	MAME_DIR .. "src/mame/includes/gatron.h",
	MAME_DIR .. "src/mame/video/gatron.cpp",
	MAME_DIR .. "src/mame/drivers/gotya.cpp",
	MAME_DIR .. "src/mame/includes/gotya.h",
	MAME_DIR .. "src/mame/audio/gotya.cpp",
	MAME_DIR .. "src/mame/video/gotya.cpp",
	MAME_DIR .. "src/mame/drivers/sbugger.cpp",
	MAME_DIR .. "src/mame/includes/sbugger.h",
	MAME_DIR .. "src/mame/video/sbugger.cpp",
}

createMAMEProjects(_target, _subtarget, "gottlieb")
files {
	MAME_DIR .. "src/mame/drivers/exterm.cpp",
	MAME_DIR .. "src/mame/includes/exterm.h",
	MAME_DIR .. "src/mame/video/exterm.cpp",
	MAME_DIR .. "src/mame/drivers/gottlieb.cpp",
	MAME_DIR .. "src/mame/includes/gottlieb.h",
	MAME_DIR .. "src/mame/audio/gottlieb.cpp",
	MAME_DIR .. "src/mame/audio/gottlieb.h",
	MAME_DIR .. "src/mame/video/gottlieb.cpp",
}

createMAMEProjects(_target, _subtarget, "ibmpc")
files {
	MAME_DIR .. "src/mame/drivers/calchase.cpp",
	MAME_DIR .. "src/mame/drivers/fruitpc.cpp",
	MAME_DIR .. "src/mame/drivers/pangofun.cpp",
	MAME_DIR .. "src/mame/drivers/pcat_dyn.cpp",
	MAME_DIR .. "src/mame/drivers/pcat_nit.cpp",
	MAME_DIR .. "src/mame/drivers/pcxt.cpp",
	MAME_DIR .. "src/mame/drivers/quakeat.cpp",
	MAME_DIR .. "src/mame/drivers/queen.cpp",
	MAME_DIR .. "src/mame/drivers/igspc.cpp",
}

createMAMEProjects(_target, _subtarget, "igs")
files {
	MAME_DIR .. "src/mame/drivers/cabaret.cpp",
	MAME_DIR .. "src/mame/drivers/dunhuang.cpp",
	MAME_DIR .. "src/mame/drivers/goldstar.cpp",
	MAME_DIR .. "src/mame/includes/goldstar.h",
	MAME_DIR .. "src/mame/video/goldstar.cpp",
	MAME_DIR .. "src/mame/drivers/jackie.cpp",
	MAME_DIR .. "src/mame/drivers/igspoker.cpp",
	MAME_DIR .. "src/mame/drivers/igs009.cpp",
	MAME_DIR .. "src/mame/drivers/igs011.cpp",
	MAME_DIR .. "src/mame/drivers/igs017.cpp",
	MAME_DIR .. "src/mame/video/igs017_igs031.cpp",
	MAME_DIR .. "src/mame/video/igs017_igs031.h",
	MAME_DIR .. "src/mame/drivers/igs_fear.cpp",
	MAME_DIR .. "src/mame/drivers/igs_m027.cpp",
	MAME_DIR .. "src/mame/drivers/igs_m036.cpp",
	MAME_DIR .. "src/mame/drivers/iqblock.cpp",
	MAME_DIR .. "src/mame/includes/iqblock.h",
	MAME_DIR .. "src/mame/video/iqblock.cpp",
	MAME_DIR .. "src/mame/drivers/lordgun.cpp",
	MAME_DIR .. "src/mame/includes/lordgun.h",
	MAME_DIR .. "src/mame/video/lordgun.cpp",
	MAME_DIR .. "src/mame/drivers/pgm.cpp",
	MAME_DIR .. "src/mame/includes/pgm.h",
	MAME_DIR .. "src/mame/video/pgm.cpp",
	MAME_DIR .. "src/mame/machine/pgmprot_igs027a_type1.cpp",
	MAME_DIR .. "src/mame/machine/pgmprot_igs027a_type2.cpp",
	MAME_DIR .. "src/mame/machine/pgmprot_igs027a_type3.cpp",
	MAME_DIR .. "src/mame/machine/pgmprot_igs025_igs012.cpp",
	MAME_DIR .. "src/mame/machine/pgmprot_igs025_igs022.cpp",
	MAME_DIR .. "src/mame/machine/pgmprot_igs025_igs028.cpp",
	MAME_DIR .. "src/mame/machine/pgmprot_orlegend.cpp",
	MAME_DIR .. "src/mame/drivers/pgm2.cpp",
	MAME_DIR .. "src/mame/drivers/spoker.cpp",
	MAME_DIR .. "src/mame/machine/igs036crypt.cpp",
	MAME_DIR .. "src/mame/machine/igs036crypt.h",
	MAME_DIR .. "src/mame/machine/pgmcrypt.cpp",
	MAME_DIR .. "src/mame/machine/pgmcrypt.h",
	MAME_DIR .. "src/mame/machine/igs025.cpp",
	MAME_DIR .. "src/mame/machine/igs025.h",
	MAME_DIR .. "src/mame/machine/igs022.cpp",
	MAME_DIR .. "src/mame/machine/igs022.h",
	MAME_DIR .. "src/mame/machine/igs028.cpp",
	MAME_DIR .. "src/mame/machine/igs028.h",
}

createMAMEProjects(_target, _subtarget, "irem")
files {
	MAME_DIR .. "src/mame/drivers/m10.cpp",
	MAME_DIR .. "src/mame/includes/m10.h",
	MAME_DIR .. "src/mame/video/m10.cpp",
	MAME_DIR .. "src/mame/drivers/m14.cpp",
	MAME_DIR .. "src/mame/drivers/m52.cpp",
	MAME_DIR .. "src/mame/includes/m52.h",
	MAME_DIR .. "src/mame/video/m52.cpp",
	MAME_DIR .. "src/mame/drivers/m57.cpp",
	MAME_DIR .. "src/mame/includes/m57.h",
	MAME_DIR .. "src/mame/video/m57.cpp",
	MAME_DIR .. "src/mame/drivers/m58.cpp",
	MAME_DIR .. "src/mame/includes/m58.h",
	MAME_DIR .. "src/mame/video/m58.cpp",
	MAME_DIR .. "src/mame/drivers/m62.cpp",
	MAME_DIR .. "src/mame/includes/m62.h",
	MAME_DIR .. "src/mame/video/m62.cpp",
	MAME_DIR .. "src/mame/drivers/m63.cpp",
	MAME_DIR .. "src/mame/drivers/m72.cpp",
	MAME_DIR .. "src/mame/includes/m72.h",
	MAME_DIR .. "src/mame/audio/m72.cpp",
	MAME_DIR .. "src/mame/audio/m72.h",
	MAME_DIR .. "src/mame/video/m72.cpp",
	MAME_DIR .. "src/mame/drivers/m90.cpp",
	MAME_DIR .. "src/mame/includes/m90.h",
	MAME_DIR .. "src/mame/video/m90.cpp",
	MAME_DIR .. "src/mame/drivers/m92.cpp",
	MAME_DIR .. "src/mame/includes/m92.h",
	MAME_DIR .. "src/mame/video/m92.cpp",
	MAME_DIR .. "src/mame/drivers/m107.cpp",
	MAME_DIR .. "src/mame/includes/m107.h",
	MAME_DIR .. "src/mame/video/m107.cpp",
	MAME_DIR .. "src/mame/includes/iremipt.h",
	MAME_DIR .. "src/mame/drivers/olibochu.cpp",
	MAME_DIR .. "src/mame/drivers/redalert.cpp",
	MAME_DIR .. "src/mame/includes/redalert.h",
	MAME_DIR .. "src/mame/audio/redalert.cpp",
	MAME_DIR .. "src/mame/video/redalert.cpp",
	MAME_DIR .. "src/mame/drivers/shisen.cpp",
	MAME_DIR .. "src/mame/includes/shisen.h",
	MAME_DIR .. "src/mame/video/shisen.cpp",
	MAME_DIR .. "src/mame/drivers/travrusa.cpp",
	MAME_DIR .. "src/mame/includes/travrusa.h",
	MAME_DIR .. "src/mame/video/travrusa.cpp",
	MAME_DIR .. "src/mame/drivers/vigilant.cpp",
	MAME_DIR .. "src/mame/includes/vigilant.h",
	MAME_DIR .. "src/mame/video/vigilant.cpp",
	MAME_DIR .. "src/mame/machine/irem_cpu.cpp",
	MAME_DIR .. "src/mame/machine/irem_cpu.h",
	MAME_DIR .. "src/mame/audio/irem.cpp",
	MAME_DIR .. "src/mame/audio/irem.h",
}

createMAMEProjects(_target, _subtarget, "itech")
files {
	MAME_DIR .. "src/mame/drivers/capbowl.cpp",
	MAME_DIR .. "src/mame/includes/capbowl.h",
	MAME_DIR .. "src/mame/video/capbowl.cpp",
	MAME_DIR .. "src/mame/drivers/itech8.cpp",
	MAME_DIR .. "src/mame/includes/itech8.h",
	MAME_DIR .. "src/mame/machine/slikshot.cpp",
	MAME_DIR .. "src/mame/video/itech8.cpp",
	MAME_DIR .. "src/mame/drivers/itech32.cpp",
	MAME_DIR .. "src/mame/includes/itech32.h",
	MAME_DIR .. "src/mame/video/itech32.cpp",
	MAME_DIR .. "src/mame/drivers/iteagle.cpp",
	MAME_DIR .. "src/mame/machine/iteagle_fpga.cpp",
	MAME_DIR .. "src/mame/machine/iteagle_fpga.h",
}

createMAMEProjects(_target, _subtarget, "jaleco")
files {
	MAME_DIR .. "src/mame/drivers/aeroboto.cpp",
	MAME_DIR .. "src/mame/includes/aeroboto.h",
	MAME_DIR .. "src/mame/video/aeroboto.cpp",
	MAME_DIR .. "src/mame/drivers/argus.cpp",
	MAME_DIR .. "src/mame/includes/argus.h",
	MAME_DIR .. "src/mame/video/argus.cpp",
	MAME_DIR .. "src/mame/drivers/bestleag.cpp",
	MAME_DIR .. "src/mame/drivers/bigstrkb.cpp",
	MAME_DIR .. "src/mame/includes/bigstrkb.h",
	MAME_DIR .. "src/mame/video/bigstrkb.cpp",
	MAME_DIR .. "src/mame/drivers/blueprnt.cpp",
	MAME_DIR .. "src/mame/includes/blueprnt.h",
	MAME_DIR .. "src/mame/video/blueprnt.cpp",
	MAME_DIR .. "src/mame/drivers/bnstars.cpp",
	MAME_DIR .. "src/mame/drivers/cischeat.cpp",
	MAME_DIR .. "src/mame/includes/cischeat.h",
	MAME_DIR .. "src/mame/video/cischeat.cpp",
	MAME_DIR .. "src/mame/drivers/citycon.cpp",
	MAME_DIR .. "src/mame/includes/citycon.h",
	MAME_DIR .. "src/mame/video/citycon.cpp",
	MAME_DIR .. "src/mame/drivers/ddayjlc.cpp",
	MAME_DIR .. "src/mame/drivers/exerion.cpp",
	MAME_DIR .. "src/mame/includes/exerion.h",
	MAME_DIR .. "src/mame/video/exerion.cpp",
	MAME_DIR .. "src/mame/drivers/fcombat.cpp",
	MAME_DIR .. "src/mame/includes/fcombat.h",
	MAME_DIR .. "src/mame/video/fcombat.cpp",
	MAME_DIR .. "src/mame/drivers/ginganin.cpp",
	MAME_DIR .. "src/mame/includes/ginganin.h",
	MAME_DIR .. "src/mame/video/ginganin.cpp",
	MAME_DIR .. "src/mame/drivers/homerun.cpp",
	MAME_DIR .. "src/mame/includes/homerun.h",
	MAME_DIR .. "src/mame/video/homerun.cpp",
	MAME_DIR .. "src/mame/drivers/megasys1.cpp",
	MAME_DIR .. "src/mame/includes/megasys1.h",
	MAME_DIR .. "src/mame/video/megasys1.cpp",
	MAME_DIR .. "src/mame/drivers/momoko.cpp",
	MAME_DIR .. "src/mame/includes/momoko.h",
	MAME_DIR .. "src/mame/video/momoko.cpp",
	MAME_DIR .. "src/mame/drivers/ms32.cpp",
	MAME_DIR .. "src/mame/includes/ms32.h",
	MAME_DIR .. "src/mame/video/ms32.cpp",
	MAME_DIR .. "src/mame/drivers/psychic5.cpp",
	MAME_DIR .. "src/mame/includes/psychic5.h",
	MAME_DIR .. "src/mame/video/psychic5.cpp",
	MAME_DIR .. "src/mame/drivers/pturn.cpp",
	MAME_DIR .. "src/mame/drivers/skyfox.cpp",
	MAME_DIR .. "src/mame/includes/skyfox.h",
	MAME_DIR .. "src/mame/video/skyfox.cpp",
	MAME_DIR .. "src/mame/drivers/tetrisp2.cpp",
	MAME_DIR .. "src/mame/includes/tetrisp2.h",
	MAME_DIR .. "src/mame/video/tetrisp2.cpp",
	MAME_DIR .. "src/mame/machine/jalcrpt.cpp",
	MAME_DIR .. "src/mame/machine/jalcrpt.h",
	MAME_DIR .. "src/mame/video/jalblend.cpp",
	MAME_DIR .. "src/mame/video/jalblend.h",
}

createMAMEProjects(_target, _subtarget, "jpm")
files {
	MAME_DIR .. "src/mame/drivers/guab.cpp",
	MAME_DIR .. "src/mame/drivers/jpmsys5.cpp",
	MAME_DIR .. "src/mame/includes/jpmsys5.h",
	MAME_DIR .. "src/mame/drivers/jpmsys5sw.cpp",
	MAME_DIR .. "src/mame/drivers/jpmmps.cpp",
	MAME_DIR .. "src/mame/drivers/jpms80.cpp",
	MAME_DIR .. "src/mame/drivers/jpmsru.cpp",
	MAME_DIR .. "src/mame/drivers/jpmimpct.cpp",
	MAME_DIR .. "src/mame/includes/jpmimpct.h",
	MAME_DIR .. "src/mame/video/jpmimpct.cpp",
	MAME_DIR .. "src/mame/drivers/jpmimpctsw.cpp",
	MAME_DIR .. "src/mame/drivers/pluto5.cpp",
	MAME_DIR .. "src/mame/drivers/jpmsys7.cpp",
	MAME_DIR .. "src/mame/video/awpvid.cpp",
	MAME_DIR .. "src/mame/video/awpvid.h",
	MAME_DIR .. "src/mame/machine/meters.cpp",
	MAME_DIR .. "src/mame/machine/meters.h",
}

createMAMEProjects(_target, _subtarget, "kaneko")
files {
	MAME_DIR .. "src/mame/drivers/airbustr.cpp",
	MAME_DIR .. "src/mame/includes/airbustr.h",
	MAME_DIR .. "src/mame/video/airbustr.cpp",
	MAME_DIR .. "src/mame/drivers/djboy.cpp",
	MAME_DIR .. "src/mame/includes/djboy.h",
	MAME_DIR .. "src/mame/video/djboy.cpp",
	MAME_DIR .. "src/mame/drivers/expro02.cpp",
	MAME_DIR .. "src/mame/drivers/galpanic.cpp",
	MAME_DIR .. "src/mame/includes/galpnipt.h",
	MAME_DIR .. "src/mame/includes/galpanic.h",
	MAME_DIR .. "src/mame/video/galpanic.cpp",
	MAME_DIR .. "src/mame/drivers/galpani2.cpp",
	MAME_DIR .. "src/mame/includes/galpani2.h",
	MAME_DIR .. "src/mame/video/galpani2.cpp",
	MAME_DIR .. "src/mame/drivers/galpani3.cpp",
	MAME_DIR .. "src/mame/video/kaneko_grap2.cpp",
	MAME_DIR .. "src/mame/video/kaneko_grap2.h",
	MAME_DIR .. "src/mame/drivers/hvyunit.cpp",
	MAME_DIR .. "src/mame/drivers/jchan.cpp",
	MAME_DIR .. "src/mame/drivers/kaneko16.cpp",
	MAME_DIR .. "src/mame/includes/kaneko16.h",
	MAME_DIR .. "src/mame/video/kaneko16.cpp",
	MAME_DIR .. "src/mame/video/kaneko_tmap.cpp",
	MAME_DIR .. "src/mame/video/kaneko_tmap.h",
	MAME_DIR .. "src/mame/video/kaneko_spr.cpp",
	MAME_DIR .. "src/mame/video/kaneko_spr.h",
	MAME_DIR .. "src/mame/machine/kaneko_hit.cpp",
	MAME_DIR .. "src/mame/machine/kaneko_hit.h",
	MAME_DIR .. "src/mame/machine/kaneko_calc3.cpp",
	MAME_DIR .. "src/mame/machine/kaneko_calc3.h",
	MAME_DIR .. "src/mame/machine/kaneko_toybox.cpp",
	MAME_DIR .. "src/mame/machine/kaneko_toybox.h",
	MAME_DIR .. "src/mame/drivers/sandscrp.cpp",
	MAME_DIR .. "src/mame/drivers/suprnova.cpp",
	MAME_DIR .. "src/mame/includes/suprnova.h",
	MAME_DIR .. "src/mame/video/suprnova.cpp",
	MAME_DIR .. "src/mame/video/sknsspr.cpp",
	MAME_DIR .. "src/mame/video/sknsspr.h",
}

createMAMEProjects(_target, _subtarget, "konami")
files {
	MAME_DIR .. "src/mame/drivers/88games.cpp",
	MAME_DIR .. "src/mame/includes/88games.h",
	MAME_DIR .. "src/mame/video/88games.cpp",
	MAME_DIR .. "src/mame/drivers/ajax.cpp",
	MAME_DIR .. "src/mame/includes/ajax.h",
	MAME_DIR .. "src/mame/machine/ajax.cpp",
	MAME_DIR .. "src/mame/video/ajax.cpp",
	MAME_DIR .. "src/mame/drivers/aliens.cpp",
	MAME_DIR .. "src/mame/includes/aliens.h",
	MAME_DIR .. "src/mame/video/aliens.cpp",
	MAME_DIR .. "src/mame/drivers/asterix.cpp",
	MAME_DIR .. "src/mame/includes/asterix.h",
	MAME_DIR .. "src/mame/video/asterix.cpp",
	MAME_DIR .. "src/mame/drivers/battlnts.cpp",
	MAME_DIR .. "src/mame/includes/battlnts.h",
	MAME_DIR .. "src/mame/video/battlnts.cpp",
	MAME_DIR .. "src/mame/drivers/bishi.cpp",
	MAME_DIR .. "src/mame/includes/bishi.h",
	MAME_DIR .. "src/mame/video/bishi.cpp",
	MAME_DIR .. "src/mame/drivers/bladestl.cpp",
	MAME_DIR .. "src/mame/includes/bladestl.h",
	MAME_DIR .. "src/mame/video/bladestl.cpp",
	MAME_DIR .. "src/mame/drivers/blockhl.cpp",
	MAME_DIR .. "src/mame/drivers/bottom9.cpp",
	MAME_DIR .. "src/mame/includes/bottom9.h",
	MAME_DIR .. "src/mame/video/bottom9.cpp",
	MAME_DIR .. "src/mame/drivers/chqflag.cpp",
	MAME_DIR .. "src/mame/includes/chqflag.h",
	MAME_DIR .. "src/mame/video/chqflag.cpp",
	MAME_DIR .. "src/mame/drivers/circusc.cpp",
	MAME_DIR .. "src/mame/includes/circusc.h",
	MAME_DIR .. "src/mame/video/circusc.cpp",
	MAME_DIR .. "src/mame/drivers/cobra.cpp",
	MAME_DIR .. "src/mame/drivers/combatsc.cpp",
	MAME_DIR .. "src/mame/includes/combatsc.h",
	MAME_DIR .. "src/mame/video/combatsc.cpp",
	MAME_DIR .. "src/mame/drivers/contra.cpp",
	MAME_DIR .. "src/mame/includes/contra.h",
	MAME_DIR .. "src/mame/video/contra.cpp",
	MAME_DIR .. "src/mame/drivers/crimfght.cpp",
	MAME_DIR .. "src/mame/includes/crimfght.h",
	MAME_DIR .. "src/mame/video/crimfght.cpp",
	MAME_DIR .. "src/mame/drivers/dbz.cpp",
	MAME_DIR .. "src/mame/includes/dbz.h",
	MAME_DIR .. "src/mame/video/dbz.cpp",
	MAME_DIR .. "src/mame/drivers/ddribble.cpp",
	MAME_DIR .. "src/mame/includes/ddribble.h",
	MAME_DIR .. "src/mame/video/ddribble.cpp",
	MAME_DIR .. "src/mame/drivers/djmain.cpp",
	MAME_DIR .. "src/mame/includes/djmain.h",
	MAME_DIR .. "src/mame/video/djmain.cpp",
	MAME_DIR .. "src/mame/drivers/fastfred.cpp",
	MAME_DIR .. "src/mame/includes/fastfred.h",
	MAME_DIR .. "src/mame/video/fastfred.cpp",
	MAME_DIR .. "src/mame/drivers/fastlane.cpp",
	MAME_DIR .. "src/mame/includes/fastlane.h",
	MAME_DIR .. "src/mame/video/fastlane.cpp",
	MAME_DIR .. "src/mame/drivers/finalizr.cpp",
	MAME_DIR .. "src/mame/includes/finalizr.h",
	MAME_DIR .. "src/mame/video/finalizr.cpp",
	MAME_DIR .. "src/mame/drivers/firebeat.cpp",
	MAME_DIR .. "src/mame/machine/midikbd.cpp",
	MAME_DIR .. "src/mame/machine/midikbd.h",
	MAME_DIR .. "src/mame/drivers/flkatck.cpp",
	MAME_DIR .. "src/mame/includes/flkatck.h",
	MAME_DIR .. "src/mame/video/flkatck.cpp",
	MAME_DIR .. "src/mame/drivers/gberet.cpp",
	MAME_DIR .. "src/mame/includes/gberet.h",
	MAME_DIR .. "src/mame/video/gberet.cpp",
	MAME_DIR .. "src/mame/drivers/gijoe.cpp",
	MAME_DIR .. "src/mame/includes/gijoe.h",
	MAME_DIR .. "src/mame/video/gijoe.cpp",
	MAME_DIR .. "src/mame/drivers/gradius3.cpp",
	MAME_DIR .. "src/mame/includes/gradius3.h",
	MAME_DIR .. "src/mame/video/gradius3.cpp",
	MAME_DIR .. "src/mame/drivers/gticlub.cpp",
	MAME_DIR .. "src/mame/drivers/gyruss.cpp",
	MAME_DIR .. "src/mame/includes/gyruss.h",
	MAME_DIR .. "src/mame/video/gyruss.cpp",
	MAME_DIR .. "src/mame/drivers/hcastle.cpp",
	MAME_DIR .. "src/mame/includes/hcastle.h",
	MAME_DIR .. "src/mame/video/hcastle.cpp",
	MAME_DIR .. "src/mame/drivers/hexion.cpp",
	MAME_DIR .. "src/mame/includes/hexion.h",
	MAME_DIR .. "src/mame/video/hexion.cpp",
	MAME_DIR .. "src/mame/drivers/hornet.cpp",
	MAME_DIR .. "src/mame/machine/konppc.cpp",
	MAME_DIR .. "src/mame/machine/konppc.h",
	MAME_DIR .. "src/mame/drivers/hyperspt.cpp",
	MAME_DIR .. "src/mame/includes/hyperspt.h",
	MAME_DIR .. "src/mame/audio/hyprolyb.cpp",
	MAME_DIR .. "src/mame/audio/hyprolyb.h",
	MAME_DIR .. "src/mame/video/hyperspt.cpp",
	MAME_DIR .. "src/mame/drivers/ironhors.cpp",
	MAME_DIR .. "src/mame/includes/ironhors.h",
	MAME_DIR .. "src/mame/video/ironhors.cpp",
	MAME_DIR .. "src/mame/drivers/jackal.cpp",
	MAME_DIR .. "src/mame/includes/jackal.h",
	MAME_DIR .. "src/mame/video/jackal.cpp",
	MAME_DIR .. "src/mame/drivers/jailbrek.cpp",
	MAME_DIR .. "src/mame/includes/jailbrek.h",
	MAME_DIR .. "src/mame/video/jailbrek.cpp",
	MAME_DIR .. "src/mame/drivers/junofrst.cpp",
	MAME_DIR .. "src/mame/drivers/konamigq.cpp",
	MAME_DIR .. "src/mame/drivers/konamigv.cpp",
	MAME_DIR .. "src/mame/drivers/konamigx.cpp",
	MAME_DIR .. "src/mame/includes/konamigx.h",
	MAME_DIR .. "src/mame/machine/konamigx.cpp",
	MAME_DIR .. "src/mame/video/konamigx.cpp",
	MAME_DIR .. "src/mame/drivers/konamim2.cpp",
	MAME_DIR .. "src/mame/drivers/kontest.cpp",
	MAME_DIR .. "src/mame/drivers/konendev.cpp",
	MAME_DIR .. "src/mame/drivers/ksys573.cpp",
	MAME_DIR .. "src/mame/machine/k573cass.cpp",
	MAME_DIR .. "src/mame/machine/k573cass.h",
	MAME_DIR .. "src/mame/machine/k573dio.cpp",
	MAME_DIR .. "src/mame/machine/k573dio.h",
	MAME_DIR .. "src/mame/machine/k573mcr.cpp",
	MAME_DIR .. "src/mame/machine/k573mcr.h",
	MAME_DIR .. "src/mame/machine/k573msu.cpp",
	MAME_DIR .. "src/mame/machine/k573msu.h",
	MAME_DIR .. "src/mame/machine/k573npu.cpp",
	MAME_DIR .. "src/mame/machine/k573npu.h",
	MAME_DIR .. "src/mame/machine/zs01.cpp",
	MAME_DIR .. "src/mame/machine/zs01.h",
	MAME_DIR .. "src/mame/drivers/labyrunr.cpp",
	MAME_DIR .. "src/mame/includes/labyrunr.h",
	MAME_DIR .. "src/mame/video/labyrunr.cpp",
	MAME_DIR .. "src/mame/drivers/lethal.cpp",
	MAME_DIR .. "src/mame/includes/lethal.h",
	MAME_DIR .. "src/mame/video/lethal.cpp",
	MAME_DIR .. "src/mame/drivers/mainevt.cpp",
	MAME_DIR .. "src/mame/includes/mainevt.h",
	MAME_DIR .. "src/mame/video/mainevt.cpp",
	MAME_DIR .. "src/mame/drivers/megazone.cpp",
	MAME_DIR .. "src/mame/includes/megazone.h",
	MAME_DIR .. "src/mame/video/megazone.cpp",
	MAME_DIR .. "src/mame/drivers/mikie.cpp",
	MAME_DIR .. "src/mame/includes/mikie.h",
	MAME_DIR .. "src/mame/video/mikie.cpp",
	MAME_DIR .. "src/mame/drivers/mogura.cpp",
	MAME_DIR .. "src/mame/drivers/moo.cpp",
	MAME_DIR .. "src/mame/includes/moo.h",
	MAME_DIR .. "src/mame/video/moo.cpp",
	MAME_DIR .. "src/mame/drivers/mystwarr.cpp",
	MAME_DIR .. "src/mame/includes/mystwarr.h",
	MAME_DIR .. "src/mame/video/mystwarr.cpp",
	MAME_DIR .. "src/mame/drivers/nemesis.cpp",
	MAME_DIR .. "src/mame/includes/nemesis.h",
	MAME_DIR .. "src/mame/video/nemesis.cpp",
	MAME_DIR .. "src/mame/drivers/nwk-tr.cpp",
	MAME_DIR .. "src/mame/drivers/overdriv.cpp",
	MAME_DIR .. "src/mame/includes/overdriv.h",
	MAME_DIR .. "src/mame/video/overdriv.cpp",
	MAME_DIR .. "src/mame/drivers/pandoras.cpp",
	MAME_DIR .. "src/mame/includes/pandoras.h",
	MAME_DIR .. "src/mame/video/pandoras.cpp",
	MAME_DIR .. "src/mame/drivers/parodius.cpp",
	MAME_DIR .. "src/mame/includes/parodius.h",
	MAME_DIR .. "src/mame/video/parodius.cpp",
	MAME_DIR .. "src/mame/drivers/pingpong.cpp",
	MAME_DIR .. "src/mame/includes/pingpong.h",
	MAME_DIR .. "src/mame/video/pingpong.cpp",
	MAME_DIR .. "src/mame/drivers/plygonet.cpp",
	MAME_DIR .. "src/mame/includes/plygonet.h",
	MAME_DIR .. "src/mame/video/plygonet.cpp",
	MAME_DIR .. "src/mame/drivers/pooyan.cpp",
	MAME_DIR .. "src/mame/includes/pooyan.h",
	MAME_DIR .. "src/mame/video/pooyan.cpp",
	MAME_DIR .. "src/mame/drivers/pyson.cpp",
	MAME_DIR .. "src/mame/drivers/qdrmfgp.cpp",
	MAME_DIR .. "src/mame/includes/qdrmfgp.h",
	MAME_DIR .. "src/mame/video/qdrmfgp.cpp",
	MAME_DIR .. "src/mame/drivers/rockrage.cpp",
	MAME_DIR .. "src/mame/includes/rockrage.h",
	MAME_DIR .. "src/mame/video/rockrage.cpp",
	MAME_DIR .. "src/mame/drivers/rocnrope.cpp",
	MAME_DIR .. "src/mame/includes/rocnrope.h",
	MAME_DIR .. "src/mame/video/rocnrope.cpp",
	MAME_DIR .. "src/mame/drivers/rollerg.cpp",
	MAME_DIR .. "src/mame/includes/rollerg.h",
	MAME_DIR .. "src/mame/video/rollerg.cpp",
	MAME_DIR .. "src/mame/drivers/rungun.cpp",
	MAME_DIR .. "src/mame/includes/rungun.h",
	MAME_DIR .. "src/mame/video/rungun.cpp",
	MAME_DIR .. "src/mame/drivers/sbasketb.cpp",
	MAME_DIR .. "src/mame/includes/sbasketb.h",
	MAME_DIR .. "src/mame/video/sbasketb.cpp",
	MAME_DIR .. "src/mame/drivers/scobra.cpp",
	MAME_DIR .. "src/mame/drivers/scotrsht.cpp",
	MAME_DIR .. "src/mame/includes/scotrsht.h",
	MAME_DIR .. "src/mame/video/scotrsht.cpp",
	MAME_DIR .. "src/mame/drivers/scramble.cpp",
	MAME_DIR .. "src/mame/includes/scramble.h",
	MAME_DIR .. "src/mame/machine/scramble.cpp",
	MAME_DIR .. "src/mame/audio/scramble.cpp",
	MAME_DIR .. "src/mame/drivers/shaolins.cpp",
	MAME_DIR .. "src/mame/includes/shaolins.h",
	MAME_DIR .. "src/mame/video/shaolins.cpp",
	MAME_DIR .. "src/mame/drivers/simpsons.cpp",
	MAME_DIR .. "src/mame/includes/simpsons.h",
	MAME_DIR .. "src/mame/machine/simpsons.cpp",
	MAME_DIR .. "src/mame/video/simpsons.cpp",
	MAME_DIR .. "src/mame/drivers/spy.cpp",
	MAME_DIR .. "src/mame/includes/spy.h",
	MAME_DIR .. "src/mame/video/spy.cpp",
	MAME_DIR .. "src/mame/drivers/surpratk.cpp",
	MAME_DIR .. "src/mame/includes/surpratk.h",
	MAME_DIR .. "src/mame/video/surpratk.cpp",
	MAME_DIR .. "src/mame/drivers/tasman.cpp",
	MAME_DIR .. "src/mame/drivers/tgtpanic.cpp",
	MAME_DIR .. "src/mame/drivers/thunderx.cpp",
	MAME_DIR .. "src/mame/includes/thunderx.h",
	MAME_DIR .. "src/mame/video/thunderx.cpp",
	MAME_DIR .. "src/mame/drivers/timeplt.cpp",
	MAME_DIR .. "src/mame/includes/timeplt.h",
	MAME_DIR .. "src/mame/audio/timeplt.cpp",
	MAME_DIR .. "src/mame/audio/timeplt.h",
	MAME_DIR .. "src/mame/video/timeplt.cpp",
	MAME_DIR .. "src/mame/drivers/tmnt.cpp",
	MAME_DIR .. "src/mame/includes/tmnt.h",
	MAME_DIR .. "src/mame/video/tmnt.cpp",
	MAME_DIR .. "src/mame/drivers/tp84.cpp",
	MAME_DIR .. "src/mame/includes/tp84.h",
	MAME_DIR .. "src/mame/video/tp84.cpp",
	MAME_DIR .. "src/mame/drivers/trackfld.cpp",
	MAME_DIR .. "src/mame/includes/trackfld.h",
	MAME_DIR .. "src/mame/machine/konami1.cpp",
	MAME_DIR .. "src/mame/machine/konami1.h",
	MAME_DIR .. "src/mame/audio/trackfld.cpp",
	MAME_DIR .. "src/mame/audio/trackfld.h",
	MAME_DIR .. "src/mame/video/trackfld.cpp",
	MAME_DIR .. "src/mame/drivers/tutankhm.cpp",
	MAME_DIR .. "src/mame/includes/tutankhm.h",
	MAME_DIR .. "src/mame/video/tutankhm.cpp",
	MAME_DIR .. "src/mame/drivers/twin16.cpp",
	MAME_DIR .. "src/mame/includes/twin16.h",
	MAME_DIR .. "src/mame/video/twin16.cpp",
	MAME_DIR .. "src/mame/drivers/twinkle.cpp",
	MAME_DIR .. "src/mame/drivers/ultrsprt.cpp",
	MAME_DIR .. "src/mame/drivers/ultraman.cpp",
	MAME_DIR .. "src/mame/includes/ultraman.h",
	MAME_DIR .. "src/mame/video/ultraman.cpp",
	MAME_DIR .. "src/mame/drivers/vendetta.cpp",
	MAME_DIR .. "src/mame/includes/vendetta.h",
	MAME_DIR .. "src/mame/video/vendetta.cpp",
	MAME_DIR .. "src/mame/drivers/viper.cpp",
	MAME_DIR .. "src/mame/drivers/wecleman.cpp",
	MAME_DIR .. "src/mame/includes/wecleman.h",
	MAME_DIR .. "src/mame/video/wecleman.cpp",
	MAME_DIR .. "src/mame/drivers/xexex.cpp",
	MAME_DIR .. "src/mame/includes/xexex.h",
	MAME_DIR .. "src/mame/video/xexex.cpp",
	MAME_DIR .. "src/mame/drivers/xmen.cpp",
	MAME_DIR .. "src/mame/includes/xmen.h",
	MAME_DIR .. "src/mame/video/xmen.cpp",
	MAME_DIR .. "src/mame/drivers/yiear.cpp",
	MAME_DIR .. "src/mame/includes/yiear.h",
	MAME_DIR .. "src/mame/video/yiear.cpp",
	MAME_DIR .. "src/mame/drivers/zr107.cpp",
	MAME_DIR .. "src/mame/includes/konamipt.h",
	MAME_DIR .. "src/mame/video/konami_helper.cpp",
	MAME_DIR .. "src/mame/video/konami_helper.h",
	MAME_DIR .. "src/mame/video/k007121.cpp",
	MAME_DIR .. "src/mame/video/k007121.h",
	MAME_DIR .. "src/mame/video/k007342.cpp",
	MAME_DIR .. "src/mame/video/k007342.h",
	MAME_DIR .. "src/mame/video/k007420.cpp",
	MAME_DIR .. "src/mame/video/k007420.h",
	MAME_DIR .. "src/mame/video/k037122.cpp",
	MAME_DIR .. "src/mame/video/k037122.h",
	MAME_DIR .. "src/mame/video/k051316.cpp",
	MAME_DIR .. "src/mame/video/k051316.h",
	MAME_DIR .. "src/mame/video/k051733.cpp",
	MAME_DIR .. "src/mame/video/k051733.h",
	MAME_DIR .. "src/mame/video/k051960.cpp",
	MAME_DIR .. "src/mame/video/k051960.h",
	MAME_DIR .. "src/mame/video/k052109.cpp",
	MAME_DIR .. "src/mame/video/k052109.h",
	MAME_DIR .. "src/mame/video/k053250.cpp",
	MAME_DIR .. "src/mame/video/k053250.h",
	MAME_DIR .. "src/mame/video/k053251.cpp",
	MAME_DIR .. "src/mame/video/k053251.h",
	MAME_DIR .. "src/mame/video/k054156_k054157_k056832.cpp",
	MAME_DIR .. "src/mame/video/k054156_k054157_k056832.h",
	MAME_DIR .. "src/mame/video/k053244_k053245.cpp",
	MAME_DIR .. "src/mame/video/k053244_k053245.h",
	MAME_DIR .. "src/mame/video/k053246_k053247_k055673.cpp",
	MAME_DIR .. "src/mame/video/k053246_k053247_k055673.h",
	MAME_DIR .. "src/mame/video/k055555.cpp",
	MAME_DIR .. "src/mame/video/k055555.h",
	MAME_DIR .. "src/mame/video/k054000.cpp",
	MAME_DIR .. "src/mame/video/k054000.h",
	MAME_DIR .. "src/mame/video/k054338.cpp",
	MAME_DIR .. "src/mame/video/k054338.h",
	MAME_DIR .. "src/mame/video/k053936.cpp",
	MAME_DIR .. "src/mame/video/k053936.h",
	MAME_DIR .. "src/mame/video/k001006.cpp",
	MAME_DIR .. "src/mame/video/k001006.h",
	MAME_DIR .. "src/mame/video/k001005.cpp",
	MAME_DIR .. "src/mame/video/k001005.h",
	MAME_DIR .. "src/mame/video/k001604.cpp",
	MAME_DIR .. "src/mame/video/k001604.h",
	MAME_DIR .. "src/mame/video/k057714.cpp",
	MAME_DIR .. "src/mame/video/k057714.h",
}

createMAMEProjects(_target, _subtarget, "matic")
files {
	MAME_DIR .. "src/mame/drivers/barata.cpp",
}

createMAMEProjects(_target, _subtarget, "maygay")
files {
	MAME_DIR .. "src/mame/drivers/maygay1b.cpp",
	MAME_DIR .. "src/mame/includes/maygay1b.h",
	MAME_DIR .. "src/mame/drivers/maygay1bsw.cpp",
	MAME_DIR .. "src/mame/drivers/maygayv1.cpp",
	MAME_DIR .. "src/mame/drivers/maygayep.cpp",
	MAME_DIR .. "src/mame/drivers/maygaysw.cpp",
	MAME_DIR .. "src/mame/drivers/mmm.cpp",
}

createMAMEProjects(_target, _subtarget, "meadows")
files {
	MAME_DIR .. "src/mame/drivers/lazercmd.cpp",
	MAME_DIR .. "src/mame/includes/lazercmd.h",
	MAME_DIR .. "src/mame/video/lazercmd.cpp",
	MAME_DIR .. "src/mame/drivers/meadwttl.cpp",
	MAME_DIR .. "src/mame/drivers/meadows.cpp",
	MAME_DIR .. "src/mame/includes/meadows.h",
	MAME_DIR .. "src/mame/audio/meadows.cpp",
	MAME_DIR .. "src/mame/video/meadows.cpp",
	MAME_DIR .. "src/mame/drivers/warpsped.cpp",
}

createMAMEProjects(_target, _subtarget, "merit")
files {
	MAME_DIR .. "src/mame/drivers/mgames.cpp",
	MAME_DIR .. "src/mame/drivers/merit.cpp",
	MAME_DIR .. "src/mame/drivers/meritm.cpp",
}

createMAMEProjects(_target, _subtarget, "metro")
files {
	MAME_DIR .. "src/mame/drivers/hyprduel.cpp",
	MAME_DIR .. "src/mame/includes/hyprduel.h",
	MAME_DIR .. "src/mame/video/hyprduel.cpp",
	MAME_DIR .. "src/mame/drivers/metro.cpp",
	MAME_DIR .. "src/mame/includes/metro.h",
	MAME_DIR .. "src/mame/video/metro.cpp",
	MAME_DIR .. "src/mame/drivers/rabbit.cpp",
	MAME_DIR .. "src/mame/drivers/tmmjprd.cpp",
}

createMAMEProjects(_target, _subtarget, "midcoin")
files {
	MAME_DIR .. "src/mame/drivers/wallc.cpp",
	MAME_DIR .. "src/mame/drivers/wink.cpp",
	MAME_DIR .. "src/mame/drivers/24cdjuke.cpp",
}

createMAMEProjects(_target, _subtarget, "midw8080")
files {
	MAME_DIR .. "src/mame/drivers/8080bw.cpp",
	MAME_DIR .. "src/mame/includes/8080bw.h",
	MAME_DIR .. "src/mame/audio/8080bw.cpp",
	MAME_DIR .. "src/mame/video/8080bw.cpp",
	MAME_DIR .. "src/mame/drivers/m79amb.cpp",
	MAME_DIR .. "src/mame/includes/m79amb.h",
	MAME_DIR .. "src/mame/audio/m79amb.cpp",
	MAME_DIR .. "src/mame/drivers/mw8080bw.cpp",
	MAME_DIR .. "src/mame/includes/mw8080bw.h",
	MAME_DIR .. "src/mame/machine/mw8080bw.cpp",
	MAME_DIR .. "src/mame/audio/mw8080bw.cpp",
	MAME_DIR .. "src/mame/video/mw8080bw.cpp",
	MAME_DIR .. "src/mame/drivers/rotaryf.cpp",
}

createMAMEProjects(_target, _subtarget, "midway")
files {
	MAME_DIR .. "src/mame/drivers/astrocde.cpp",
	MAME_DIR .. "src/mame/includes/astrocde.h",
	MAME_DIR .. "src/mame/video/astrocde.cpp",
	MAME_DIR .. "src/mame/audio/gorf.cpp",
	MAME_DIR .. "src/mame/audio/wow.cpp",
	MAME_DIR .. "src/mame/drivers/atlantis.cpp",
	MAME_DIR .. "src/mame/drivers/balsente.cpp",
	MAME_DIR .. "src/mame/includes/balsente.h",
	MAME_DIR .. "src/mame/machine/balsente.cpp",
	MAME_DIR .. "src/mame/video/balsente.cpp",
	MAME_DIR .. "src/mame/drivers/gridlee.cpp",
	MAME_DIR .. "src/mame/includes/gridlee.h",
	MAME_DIR .. "src/mame/audio/gridlee.cpp",
	MAME_DIR .. "src/mame/video/gridlee.cpp",
	MAME_DIR .. "src/mame/drivers/mcr.cpp",
	MAME_DIR .. "src/mame/includes/mcr.h",
	MAME_DIR .. "src/mame/machine/mcr.cpp",
	MAME_DIR .. "src/mame/video/mcr.cpp",
	MAME_DIR .. "src/mame/drivers/mcr3.cpp",
	MAME_DIR .. "src/mame/includes/mcr3.h",
	MAME_DIR .. "src/mame/video/mcr3.cpp",
	MAME_DIR .. "src/mame/drivers/mcr68.cpp",
	MAME_DIR .. "src/mame/includes/mcr68.h",
	MAME_DIR .. "src/mame/machine/mcr68.cpp",
	MAME_DIR .. "src/mame/video/mcr68.cpp",
	MAME_DIR .. "src/mame/drivers/midqslvr.cpp",
	MAME_DIR .. "src/mame/drivers/midtunit.cpp",
	MAME_DIR .. "src/mame/includes/midtunit.h",
	MAME_DIR .. "src/mame/machine/midtunit.cpp",
	MAME_DIR .. "src/mame/video/midtunit.cpp",
	MAME_DIR .. "src/mame/drivers/midvunit.cpp",
	MAME_DIR .. "src/mame/includes/midvunit.h",
	MAME_DIR .. "src/mame/video/midvunit.cpp",
	MAME_DIR .. "src/mame/drivers/midwunit.cpp",
	MAME_DIR .. "src/mame/includes/midwunit.h",
	MAME_DIR .. "src/mame/machine/midwunit.cpp",
	MAME_DIR .. "src/mame/drivers/midxunit.cpp",
	MAME_DIR .. "src/mame/includes/midxunit.h",
	MAME_DIR .. "src/mame/machine/midxunit.cpp",
	MAME_DIR .. "src/mame/drivers/midyunit.cpp",
	MAME_DIR .. "src/mame/includes/midyunit.h",
	MAME_DIR .. "src/mame/machine/midyunit.cpp",
	MAME_DIR .. "src/mame/video/midyunit.cpp",
	MAME_DIR .. "src/mame/drivers/midzeus.cpp",
	MAME_DIR .. "src/mame/includes/midzeus.h",
	MAME_DIR .. "src/mame/video/midzeus.cpp",
	MAME_DIR .. "src/mame/video/midzeus2.cpp",
	MAME_DIR .. "src/mame/drivers/mw18w.cpp",
	MAME_DIR .. "src/mame/drivers/mwsub.cpp",
	MAME_DIR .. "src/mame/drivers/omegrace.cpp",
	MAME_DIR .. "src/mame/drivers/pinball2k.cpp",
	MAME_DIR .. "src/mame/drivers/seattle.cpp",
	MAME_DIR .. "src/mame/drivers/sspeedr.cpp",
	MAME_DIR .. "src/mame/includes/sspeedr.h",
	MAME_DIR .. "src/mame/video/sspeedr.cpp",
	MAME_DIR .. "src/mame/drivers/tmaster.cpp",
	MAME_DIR .. "src/mame/drivers/vegas.cpp",
	MAME_DIR .. "src/mame/drivers/wmg.cpp",
	MAME_DIR .. "src/mame/drivers/williams.cpp",
	MAME_DIR .. "src/mame/includes/williams.h",
	MAME_DIR .. "src/mame/machine/williams.cpp",
	MAME_DIR .. "src/mame/audio/williams.cpp",
	MAME_DIR .. "src/mame/audio/williams.h",
	MAME_DIR .. "src/mame/video/williams.cpp",
	MAME_DIR .. "src/mame/machine/midwayic.cpp",
	MAME_DIR .. "src/mame/machine/midwayic.h",
	MAME_DIR .. "src/mame/audio/midway.cpp",
	MAME_DIR .. "src/mame/audio/midway.h",
}

createMAMEProjects(_target, _subtarget, "namco")
files {
	MAME_DIR .. "src/mame/drivers/20pacgal.cpp",
	MAME_DIR .. "src/mame/includes/20pacgal.h",
	MAME_DIR .. "src/mame/video/20pacgal.cpp",
	MAME_DIR .. "src/mame/drivers/30test.cpp",
	MAME_DIR .. "src/mame/drivers/baraduke.cpp",
	MAME_DIR .. "src/mame/includes/baraduke.h",
	MAME_DIR .. "src/mame/video/baraduke.cpp",
	MAME_DIR .. "src/mame/drivers/cswat.cpp",
	MAME_DIR .. "src/mame/drivers/dambustr.cpp",
	MAME_DIR .. "src/mame/drivers/gal3.cpp",
	MAME_DIR .. "src/mame/drivers/galaga.cpp",
	MAME_DIR .. "src/mame/includes/galaga.h",
	MAME_DIR .. "src/mame/audio/galaga.cpp",
	MAME_DIR .. "src/mame/video/galaga.cpp",
	MAME_DIR .. "src/mame/video/bosco.cpp",
	MAME_DIR .. "src/mame/video/digdug.cpp",
	MAME_DIR .. "src/mame/machine/xevious.cpp",
	MAME_DIR .. "src/mame/video/xevious.cpp",
	MAME_DIR .. "src/mame/drivers/galaxian.cpp",
	MAME_DIR .. "src/mame/includes/galaxian.h",
	MAME_DIR .. "src/mame/audio/galaxian.cpp",
	MAME_DIR .. "src/mame/audio/galaxian.h",
	MAME_DIR .. "src/mame/video/galaxian.cpp",
	MAME_DIR .. "src/mame/drivers/galaxold.cpp",
	MAME_DIR .. "src/mame/includes/galaxold.h",
	MAME_DIR .. "src/mame/machine/galaxold.cpp",
	MAME_DIR .. "src/mame/video/galaxold.cpp",
	MAME_DIR .. "src/mame/drivers/gaplus.cpp",
	MAME_DIR .. "src/mame/includes/gaplus.h",
	MAME_DIR .. "src/mame/machine/gaplus.cpp",
	MAME_DIR .. "src/mame/video/gaplus.cpp",
	MAME_DIR .. "src/mame/drivers/kungfur.cpp",
	MAME_DIR .. "src/mame/drivers/mappy.cpp",
	MAME_DIR .. "src/mame/includes/mappy.h",
	MAME_DIR .. "src/mame/video/mappy.cpp",
	MAME_DIR .. "src/mame/drivers/namcofl.cpp",
	MAME_DIR .. "src/mame/includes/namcofl.h",
	MAME_DIR .. "src/mame/video/namcofl.cpp",
	MAME_DIR .. "src/mame/drivers/namcoic.cpp",
	MAME_DIR .. "src/mame/includes/namcoic.h",
	MAME_DIR .. "src/mame/drivers/namcona1.cpp",
	MAME_DIR .. "src/mame/includes/namcona1.h",
	MAME_DIR .. "src/mame/video/namcona1.cpp",
	MAME_DIR .. "src/mame/drivers/namconb1.cpp",
	MAME_DIR .. "src/mame/includes/namconb1.h",
	MAME_DIR .. "src/mame/video/namconb1.cpp",
	MAME_DIR .. "src/mame/drivers/namcond1.cpp",
	MAME_DIR .. "src/mame/includes/namcond1.h",
	MAME_DIR .. "src/mame/machine/namcond1.cpp",
	MAME_DIR .. "src/mame/video/ygv608.cpp",
	MAME_DIR .. "src/mame/video/ygv608.h",
	MAME_DIR .. "src/mame/drivers/namcops2.cpp",
	MAME_DIR .. "src/mame/drivers/namcos1.cpp",
	MAME_DIR .. "src/mame/includes/namcos1.h",
	MAME_DIR .. "src/mame/machine/namcos1.cpp",
	MAME_DIR .. "src/mame/video/namcos1.cpp",
	MAME_DIR .. "src/mame/drivers/namcos10.cpp",
	MAME_DIR .. "src/mame/machine/ns10crypt.cpp",
	MAME_DIR .. "src/mame/machine/ns10crypt.h",
	MAME_DIR .. "src/mame/drivers/namcos11.cpp",
	MAME_DIR .. "src/mame/machine/ns11prot.cpp",
	MAME_DIR .. "src/mame/machine/ns11prot.h",
	MAME_DIR .. "src/mame/drivers/namcos12.cpp",
	MAME_DIR .. "src/mame/machine/namco_settings.cpp",
	MAME_DIR .. "src/mame/machine/namco_settings.h",
	MAME_DIR .. "src/mame/drivers/namcos2.cpp",
	MAME_DIR .. "src/mame/includes/namcos2.h",
	MAME_DIR .. "src/mame/machine/namcos2.cpp",
	MAME_DIR .. "src/mame/video/namcos2.cpp",
	MAME_DIR .. "src/mame/drivers/namcos21.cpp",
	MAME_DIR .. "src/mame/includes/namcos21.h",
	MAME_DIR .. "src/mame/video/namcos21.cpp",
	MAME_DIR .. "src/mame/drivers/namcos22.cpp",
	MAME_DIR .. "src/mame/includes/namcos22.h",
	MAME_DIR .. "src/mame/video/namcos22.cpp",
	MAME_DIR .. "src/mame/drivers/namcos23.cpp",
	MAME_DIR .. "src/mame/drivers/namcos86.cpp",
	MAME_DIR .. "src/mame/includes/namcos86.h",
	MAME_DIR .. "src/mame/video/namcos86.cpp",
	MAME_DIR .. "src/mame/drivers/pacland.cpp",
	MAME_DIR .. "src/mame/includes/pacland.h",
	MAME_DIR .. "src/mame/video/pacland.cpp",
	MAME_DIR .. "src/mame/drivers/polepos.cpp",
	MAME_DIR .. "src/mame/includes/polepos.h",
	MAME_DIR .. "src/mame/audio/polepos.cpp",
	MAME_DIR .. "src/mame/video/polepos.cpp",
	MAME_DIR .. "src/mame/drivers/rallyx.cpp",
	MAME_DIR .. "src/mame/includes/rallyx.h",
	MAME_DIR .. "src/mame/video/rallyx.cpp",
	MAME_DIR .. "src/mame/drivers/skykid.cpp",
	MAME_DIR .. "src/mame/includes/skykid.h",
	MAME_DIR .. "src/mame/video/skykid.cpp",
	MAME_DIR .. "src/mame/drivers/tankbatt.cpp",
	MAME_DIR .. "src/mame/includes/tankbatt.h",
	MAME_DIR .. "src/mame/video/tankbatt.cpp",
	MAME_DIR .. "src/mame/drivers/tceptor.cpp",
	MAME_DIR .. "src/mame/includes/tceptor.h",
	MAME_DIR .. "src/mame/video/tceptor.cpp",
	MAME_DIR .. "src/mame/drivers/toypop.cpp",
	MAME_DIR .. "src/mame/includes/toypop.h",
	MAME_DIR .. "src/mame/video/toypop.cpp",
	MAME_DIR .. "src/mame/drivers/turrett.cpp",
	MAME_DIR .. "src/mame/includes/turrett.h",
	MAME_DIR .. "src/mame/audio/turrett.cpp",
	MAME_DIR .. "src/mame/video/turrett.cpp",
	MAME_DIR .. "src/mame/drivers/warpwarp.cpp",
	MAME_DIR .. "src/mame/includes/warpwarp.h",
	MAME_DIR .. "src/mame/audio/geebee.cpp",
	MAME_DIR .. "src/mame/audio/warpwarp.cpp",
	MAME_DIR .. "src/mame/audio/warpwarp.h",
	MAME_DIR .. "src/mame/video/warpwarp.cpp",
	MAME_DIR .. "src/mame/machine/c117.cpp",
	MAME_DIR .. "src/mame/machine/c117.h",
	MAME_DIR .. "src/mame/machine/namcoio.cpp",
	MAME_DIR .. "src/mame/machine/namcoio.h",
	MAME_DIR .. "src/mame/machine/namco06.cpp",
	MAME_DIR .. "src/mame/machine/namco06.h",
	MAME_DIR .. "src/mame/machine/namco50.cpp",
	MAME_DIR .. "src/mame/machine/namco50.h",
	MAME_DIR .. "src/mame/machine/namco51.cpp",
	MAME_DIR .. "src/mame/machine/namco51.h",
	MAME_DIR .. "src/mame/machine/namco53.cpp",
	MAME_DIR .. "src/mame/machine/namco53.h",
	MAME_DIR .. "src/mame/machine/namco62.cpp",
	MAME_DIR .. "src/mame/machine/namco62.h",
	MAME_DIR .. "src/mame/machine/namcomcu.cpp",
	MAME_DIR .. "src/mame/machine/namcomcu.h",
	MAME_DIR .. "src/mame/audio/namco52.cpp",
	MAME_DIR .. "src/mame/audio/namco52.h",
	MAME_DIR .. "src/mame/audio/namco54.cpp",
	MAME_DIR .. "src/mame/audio/namco54.h",
	MAME_DIR .. "src/mame/video/c116.cpp",
	MAME_DIR .. "src/mame/video/c116.h",
	MAME_DIR .. "src/mame/video/c45.cpp",
	MAME_DIR .. "src/mame/video/c45.h",
}

createMAMEProjects(_target, _subtarget, "nasco")
files {
	MAME_DIR .. "src/mame/drivers/crgolf.cpp",
	MAME_DIR .. "src/mame/includes/crgolf.h",
	MAME_DIR .. "src/mame/video/crgolf.cpp",
	MAME_DIR .. "src/mame/drivers/suprgolf.cpp",
}

createMAMEProjects(_target, _subtarget, "neogeo")
files {
	MAME_DIR .. "src/mame/drivers/neogeo.cpp",
	MAME_DIR .. "src/mame/includes/neogeo.h",
	MAME_DIR .. "src/mame/video/neogeo.cpp",
	MAME_DIR .. "src/mame/drivers/neogeo_noslot.cpp",
	MAME_DIR .. "src/mame/video/neogeo_spr.cpp",
	MAME_DIR .. "src/mame/video/neogeo_spr.h",
	MAME_DIR .. "src/mame/machine/neocrypt.cpp",
	MAME_DIR .. "src/mame/machine/ng_memcard.cpp",
	MAME_DIR .. "src/mame/machine/ng_memcard.h",
}

createMAMEProjects(_target, _subtarget, "nichibut")
files {
	MAME_DIR .. "src/mame/drivers/armedf.cpp",
	MAME_DIR .. "src/mame/includes/armedf.h",
	MAME_DIR .. "src/mame/video/armedf.cpp",
	MAME_DIR .. "src/mame/drivers/cclimber.cpp",
	MAME_DIR .. "src/mame/includes/cclimber.h",
	MAME_DIR .. "src/mame/machine/cclimber.cpp",
	MAME_DIR .. "src/mame/audio/cclimber.cpp",
	MAME_DIR .. "src/mame/audio/cclimber.h",
	MAME_DIR .. "src/mame/video/cclimber.cpp",
	MAME_DIR .. "src/mame/drivers/clshroad.cpp",
	MAME_DIR .. "src/mame/includes/clshroad.h",
	MAME_DIR .. "src/mame/video/clshroad.cpp",
	MAME_DIR .. "src/mame/drivers/csplayh5.cpp",
	MAME_DIR .. "src/mame/drivers/cop01.cpp",
	MAME_DIR .. "src/mame/includes/cop01.h",
	MAME_DIR .. "src/mame/video/cop01.cpp",
	MAME_DIR .. "src/mame/drivers/dacholer.cpp",
	MAME_DIR .. "src/mame/drivers/galivan.cpp",
	MAME_DIR .. "src/mame/includes/galivan.h",
	MAME_DIR .. "src/mame/video/galivan.cpp",
	MAME_DIR .. "src/mame/drivers/gomoku.cpp",
	MAME_DIR .. "src/mame/includes/gomoku.h",
	MAME_DIR .. "src/mame/audio/gomoku.cpp",
	MAME_DIR .. "src/mame/video/gomoku.cpp",
	MAME_DIR .. "src/mame/drivers/hyhoo.cpp",
	MAME_DIR .. "src/mame/includes/hyhoo.h",
	MAME_DIR .. "src/mame/video/hyhoo.cpp",
	MAME_DIR .. "src/mame/drivers/jangou.cpp",
	MAME_DIR .. "src/mame/drivers/magmax.cpp",
	MAME_DIR .. "src/mame/includes/magmax.h",
	MAME_DIR .. "src/mame/video/magmax.cpp",
	MAME_DIR .. "src/mame/drivers/nbmj8688.cpp",
	MAME_DIR .. "src/mame/includes/nbmj8688.h",
	MAME_DIR .. "src/mame/video/nbmj8688.cpp",
	MAME_DIR .. "src/mame/drivers/nbmj8891.cpp",
	MAME_DIR .. "src/mame/includes/nbmj8891.h",
	MAME_DIR .. "src/mame/video/nbmj8891.cpp",
	MAME_DIR .. "src/mame/drivers/nbmj8900.cpp",
	MAME_DIR .. "src/mame/includes/nbmj8900.h",
	MAME_DIR .. "src/mame/video/nbmj8900.cpp",
	MAME_DIR .. "src/mame/drivers/nbmj8991.cpp",
	MAME_DIR .. "src/mame/includes/nbmj8991.h",
	MAME_DIR .. "src/mame/video/nbmj8991.cpp",
	MAME_DIR .. "src/mame/drivers/nbmj9195.cpp",
	MAME_DIR .. "src/mame/includes/nbmj9195.h",
	MAME_DIR .. "src/mame/video/nbmj9195.cpp",
	MAME_DIR .. "src/mame/drivers/nightgal.cpp",
	MAME_DIR .. "src/mame/drivers/niyanpai.cpp",
	MAME_DIR .. "src/mame/includes/niyanpai.h",
	MAME_DIR .. "src/mame/video/niyanpai.cpp",
	MAME_DIR .. "src/mame/drivers/pastelg.cpp",
	MAME_DIR .. "src/mame/includes/pastelg.h",
	MAME_DIR .. "src/mame/video/pastelg.cpp",
	MAME_DIR .. "src/mame/drivers/seicross.cpp",
	MAME_DIR .. "src/mame/includes/seicross.h",
	MAME_DIR .. "src/mame/video/seicross.cpp",
	MAME_DIR .. "src/mame/drivers/terracre.cpp",
	MAME_DIR .. "src/mame/includes/terracre.h",
	MAME_DIR .. "src/mame/video/terracre.cpp",
	MAME_DIR .. "src/mame/drivers/tubep.cpp",
	MAME_DIR .. "src/mame/includes/tubep.h",
	MAME_DIR .. "src/mame/video/tubep.cpp",
	MAME_DIR .. "src/mame/drivers/wiping.cpp",
	MAME_DIR .. "src/mame/includes/wiping.h",
	MAME_DIR .. "src/mame/audio/wiping.cpp",
	MAME_DIR .. "src/mame/audio/wiping.h",
	MAME_DIR .. "src/mame/video/wiping.cpp",
	MAME_DIR .. "src/mame/machine/nb1413m3.cpp",
	MAME_DIR .. "src/mame/includes/nb1413m3.h",
	MAME_DIR .. "src/mame/machine/nb1414m4.cpp",
	MAME_DIR .. "src/mame/includes/nb1414m4.h",
}

createMAMEProjects(_target, _subtarget, "nintendo")
files {
	MAME_DIR .. "src/mame/drivers/cham24.cpp",
	MAME_DIR .. "src/mame/drivers/dkong.cpp",
	MAME_DIR .. "src/mame/includes/dkong.h",
	MAME_DIR .. "src/mame/audio/dkong.cpp",
	MAME_DIR .. "src/mame/video/dkong.cpp",
	MAME_DIR .. "src/mame/drivers/mario.cpp",
	MAME_DIR .. "src/mame/includes/mario.h",
	MAME_DIR .. "src/mame/audio/mario.cpp",
	MAME_DIR .. "src/mame/video/mario.cpp",
	MAME_DIR .. "src/mame/drivers/mmagic.cpp",
	MAME_DIR .. "src/mame/drivers/multigam.cpp",
	MAME_DIR .. "src/mame/drivers/n8080.cpp",
	MAME_DIR .. "src/mame/includes/n8080.h",
	MAME_DIR .. "src/mame/audio/n8080.cpp",
	MAME_DIR .. "src/mame/video/n8080.cpp",
	MAME_DIR .. "src/mame/drivers/nss.cpp",
	MAME_DIR .. "src/mame/machine/snes.cpp",
	MAME_DIR .. "src/mame/audio/snes_snd.cpp",
	MAME_DIR .. "src/mame/audio/snes_snd.h",
	MAME_DIR .. "src/mame/drivers/playch10.cpp",
	MAME_DIR .. "src/mame/includes/playch10.h",
	MAME_DIR .. "src/mame/machine/playch10.cpp",
	MAME_DIR .. "src/mame/video/playch10.cpp",
	MAME_DIR .. "src/mame/drivers/popeye.cpp",
	MAME_DIR .. "src/mame/includes/popeye.h",
	MAME_DIR .. "src/mame/video/popeye.cpp",
	MAME_DIR .. "src/mame/drivers/punchout.cpp",
	MAME_DIR .. "src/mame/includes/punchout.h",
	MAME_DIR .. "src/mame/video/punchout.cpp",
	MAME_DIR .. "src/mame/drivers/famibox.cpp",
	MAME_DIR .. "src/mame/drivers/sfcbox.cpp",
	MAME_DIR .. "src/mame/drivers/snesb.cpp",
	MAME_DIR .. "src/mame/drivers/spacefb.cpp",
	MAME_DIR .. "src/mame/includes/spacefb.h",
	MAME_DIR .. "src/mame/audio/spacefb.cpp",
	MAME_DIR .. "src/mame/video/spacefb.cpp",
	MAME_DIR .. "src/mame/drivers/vsnes.cpp",
	MAME_DIR .. "src/mame/includes/vsnes.h",
	MAME_DIR .. "src/mame/machine/vsnes.cpp",
	MAME_DIR .. "src/mame/video/vsnes.cpp",
	MAME_DIR .. "src/mame/video/ppu2c0x.cpp",
	MAME_DIR .. "src/mame/video/ppu2c0x.h",


}

createMAMEProjects(_target, _subtarget, "nix")
files {
	MAME_DIR .. "src/mame/drivers/fitfight.cpp",
	MAME_DIR .. "src/mame/includes/fitfight.h",
	MAME_DIR .. "src/mame/video/fitfight.cpp",
	MAME_DIR .. "src/mame/drivers/pirates.cpp",
	MAME_DIR .. "src/mame/includes/pirates.h",
	MAME_DIR .. "src/mame/video/pirates.cpp",
}

createMAMEProjects(_target, _subtarget, "nmk")
files {
	MAME_DIR .. "src/mame/drivers/acommand.cpp",
	MAME_DIR .. "src/mame/drivers/cultures.cpp",
	MAME_DIR .. "src/mame/drivers/ddealer.cpp",
	MAME_DIR .. "src/mame/drivers/jalmah.cpp",
	MAME_DIR .. "src/mame/drivers/macrossp.cpp",
	MAME_DIR .. "src/mame/includes/macrossp.h",
	MAME_DIR .. "src/mame/video/macrossp.cpp",
	MAME_DIR .. "src/mame/drivers/nmk16.cpp",
	MAME_DIR .. "src/mame/includes/nmk16.h",
	MAME_DIR .. "src/mame/machine/nmk004.cpp",
	MAME_DIR .. "src/mame/machine/nmk004.h",
	MAME_DIR .. "src/mame/video/nmk16.cpp",
	MAME_DIR .. "src/mame/drivers/quizdna.cpp",
	MAME_DIR .. "src/mame/includes/quizdna.h",
	MAME_DIR .. "src/mame/video/quizdna.cpp",
	MAME_DIR .. "src/mame/drivers/quizpani.cpp",
	MAME_DIR .. "src/mame/includes/quizpani.h",
	MAME_DIR .. "src/mame/video/quizpani.cpp",
}

createMAMEProjects(_target, _subtarget, "olympia")
files {
	MAME_DIR .. "src/mame/drivers/dday.cpp",
	MAME_DIR .. "src/mame/includes/dday.h",
	MAME_DIR .. "src/mame/video/dday.cpp",
	MAME_DIR .. "src/mame/drivers/lbeach.cpp",
	MAME_DIR .. "src/mame/drivers/monzagp.cpp",
	MAME_DIR .. "src/mame/drivers/portrait.cpp",
	MAME_DIR .. "src/mame/includes/portrait.h",
	MAME_DIR .. "src/mame/video/portrait.cpp",
	MAME_DIR .. "src/mame/drivers/vega.cpp",
}

createMAMEProjects(_target, _subtarget, "omori")
files {
	MAME_DIR .. "src/mame/drivers/battlex.cpp",
	MAME_DIR .. "src/mame/includes/battlex.h",
	MAME_DIR .. "src/mame/video/battlex.cpp",
	MAME_DIR .. "src/mame/drivers/carjmbre.cpp",
	MAME_DIR .. "src/mame/includes/carjmbre.h",
	MAME_DIR .. "src/mame/video/carjmbre.cpp",
	MAME_DIR .. "src/mame/drivers/popper.cpp",
	MAME_DIR .. "src/mame/includes/popper.h",
	MAME_DIR .. "src/mame/video/popper.cpp",
	MAME_DIR .. "src/mame/drivers/spaceg.cpp",
}

createMAMEProjects(_target, _subtarget, "orca")
files {
	MAME_DIR .. "src/mame/drivers/espial.cpp",
	MAME_DIR .. "src/mame/includes/espial.h",
	MAME_DIR .. "src/mame/video/espial.cpp",
	MAME_DIR .. "src/mame/drivers/funkybee.cpp",
	MAME_DIR .. "src/mame/includes/funkybee.h",
	MAME_DIR .. "src/mame/video/funkybee.cpp",
	MAME_DIR .. "src/mame/drivers/marineb.cpp",
	MAME_DIR .. "src/mame/includes/marineb.h",
	MAME_DIR .. "src/mame/video/marineb.cpp",
	MAME_DIR .. "src/mame/drivers/vastar.cpp",
	MAME_DIR .. "src/mame/includes/vastar.h",
	MAME_DIR .. "src/mame/video/vastar.cpp",
	MAME_DIR .. "src/mame/drivers/zodiack.cpp",
	MAME_DIR .. "src/mame/includes/zodiack.h",
	MAME_DIR .. "src/mame/video/zodiack.cpp",
}

createMAMEProjects(_target, _subtarget, "pacific")
files {
	MAME_DIR .. "src/mame/drivers/mrflea.cpp",
	MAME_DIR .. "src/mame/includes/mrflea.h",
	MAME_DIR .. "src/mame/video/mrflea.cpp",
	MAME_DIR .. "src/mame/drivers/thief.cpp",
	MAME_DIR .. "src/mame/includes/thief.h",
	MAME_DIR .. "src/mame/video/thief.cpp",
}

createMAMEProjects(_target, _subtarget, "pacman")
files {
	MAME_DIR .. "src/mame/drivers/jrpacman.cpp",
	MAME_DIR .. "src/mame/drivers/pacman.cpp",
	MAME_DIR .. "src/mame/includes/pacman.h",
	MAME_DIR .. "src/mame/video/pacman.cpp",
	MAME_DIR .. "src/mame/machine/acitya.cpp",
	MAME_DIR .. "src/mame/machine/jumpshot.cpp",
	MAME_DIR .. "src/mame/machine/pacplus.cpp",
	MAME_DIR .. "src/mame/machine/theglobp.cpp",
	MAME_DIR .. "src/mame/drivers/pengo.cpp",
}

createMAMEProjects(_target, _subtarget, "pce")
files {
	MAME_DIR .. "src/mame/drivers/ggconnie.cpp",
	MAME_DIR .. "src/mame/drivers/paranoia.cpp",
	MAME_DIR .. "src/mame/drivers/tourvis.cpp",
	MAME_DIR .. "src/mame/drivers/uapce.cpp",
	MAME_DIR .. "src/mame/machine/pcecommn.cpp",
	MAME_DIR .. "src/mame/machine/pcecommn.h",
}

createMAMEProjects(_target, _subtarget, "phoenix")
files {
	MAME_DIR .. "src/mame/drivers/naughtyb.cpp",
	MAME_DIR .. "src/mame/includes/naughtyb.h",
	MAME_DIR .. "src/mame/video/naughtyb.cpp",
	MAME_DIR .. "src/mame/drivers/phoenix.cpp",
	MAME_DIR .. "src/mame/includes/phoenix.h",
	MAME_DIR .. "src/mame/audio/phoenix.cpp",
	MAME_DIR .. "src/mame/video/phoenix.cpp",
	MAME_DIR .. "src/mame/drivers/safarir.cpp",
	MAME_DIR .. "src/mame/audio/pleiads.cpp",
	MAME_DIR .. "src/mame/audio/pleiads.h",
}

createMAMEProjects(_target, _subtarget, "playmark")
files {
	MAME_DIR .. "src/mame/drivers/drtomy.cpp",
	MAME_DIR .. "src/mame/drivers/playmark.cpp",
	MAME_DIR .. "src/mame/includes/playmark.h",
	MAME_DIR .. "src/mame/video/playmark.cpp",
	MAME_DIR .. "src/mame/drivers/powerbal.cpp",
	MAME_DIR .. "src/mame/drivers/sderby.cpp",
	MAME_DIR .. "src/mame/includes/sderby.h",
	MAME_DIR .. "src/mame/video/sderby.cpp",
	MAME_DIR .. "src/mame/drivers/sslam.cpp",
	MAME_DIR .. "src/mame/includes/sslam.h",
	MAME_DIR .. "src/mame/video/sslam.cpp",
}

createMAMEProjects(_target, _subtarget, "psikyo")
files {
	MAME_DIR .. "src/mame/drivers/psikyo.cpp",
	MAME_DIR .. "src/mame/includes/psikyo.h",
	MAME_DIR .. "src/mame/video/psikyo.cpp",
	MAME_DIR .. "src/mame/drivers/psikyo4.cpp",
	MAME_DIR .. "src/mame/includes/psikyo4.h",
	MAME_DIR .. "src/mame/video/psikyo4.cpp",
	MAME_DIR .. "src/mame/drivers/psikyosh.cpp",
	MAME_DIR .. "src/mame/includes/psikyosh.h",
	MAME_DIR .. "src/mame/video/psikyosh.cpp",
}

createMAMEProjects(_target, _subtarget, "ramtek")
files {
	MAME_DIR .. "src/mame/drivers/hitme.cpp",
	MAME_DIR .. "src/mame/includes/hitme.h",
	MAME_DIR .. "src/mame/audio/hitme.cpp",
	MAME_DIR .. "src/mame/drivers/ramtek.cpp",
	MAME_DIR .. "src/mame/drivers/starcrus.cpp",
	MAME_DIR .. "src/mame/includes/starcrus.h",
	MAME_DIR .. "src/mame/video/starcrus.cpp",
}

createMAMEProjects(_target, _subtarget, "rare")
files {
	MAME_DIR .. "src/mame/drivers/btoads.cpp",
	MAME_DIR .. "src/mame/includes/btoads.h",
	MAME_DIR .. "src/mame/video/btoads.cpp",
	MAME_DIR .. "src/mame/drivers/kinst.cpp",
	MAME_DIR .. "src/mame/drivers/xtheball.cpp",
}

createMAMEProjects(_target, _subtarget, "sanritsu")
files {
	MAME_DIR .. "src/mame/drivers/appoooh.cpp",
	MAME_DIR .. "src/mame/includes/appoooh.h",
	MAME_DIR .. "src/mame/video/appoooh.cpp",
	MAME_DIR .. "src/mame/drivers/bankp.cpp",
	MAME_DIR .. "src/mame/includes/bankp.h",
	MAME_DIR .. "src/mame/video/bankp.cpp",
	MAME_DIR .. "src/mame/drivers/chinsan.cpp",
	MAME_DIR .. "src/mame/drivers/drmicro.cpp",
	MAME_DIR .. "src/mame/includes/drmicro.h",
	MAME_DIR .. "src/mame/video/drmicro.cpp",
	MAME_DIR .. "src/mame/drivers/jantotsu.cpp",
	MAME_DIR .. "src/mame/drivers/mayumi.cpp",
	MAME_DIR .. "src/mame/drivers/mermaid.cpp",
	MAME_DIR .. "src/mame/includes/mermaid.h",
	MAME_DIR .. "src/mame/video/mermaid.cpp",
	MAME_DIR .. "src/mame/drivers/mjkjidai.cpp",
	MAME_DIR .. "src/mame/includes/mjkjidai.h",
	MAME_DIR .. "src/mame/video/mjkjidai.cpp",
}

createMAMEProjects(_target, _subtarget, "sega")
files {
	MAME_DIR .. "src/mame/drivers/angelkds.cpp",
	MAME_DIR .. "src/mame/includes/angelkds.h",
	MAME_DIR .. "src/mame/video/angelkds.cpp",
	MAME_DIR .. "src/mame/drivers/bingoc.cpp",
	MAME_DIR .. "src/mame/drivers/blockade.cpp",
	MAME_DIR .. "src/mame/includes/blockade.h",
	MAME_DIR .. "src/mame/audio/blockade.cpp",
	MAME_DIR .. "src/mame/video/blockade.cpp",
	MAME_DIR .. "src/mame/drivers/calorie.cpp",
	MAME_DIR .. "src/mame/drivers/chihiro.cpp",
	MAME_DIR .. "src/mame/includes/chihiro.h",
	MAME_DIR .. "src/mame/video/chihiro.cpp",
	MAME_DIR .. "src/mame/drivers/coolridr.cpp",
	MAME_DIR .. "src/mame/drivers/deniam.cpp",
	MAME_DIR .. "src/mame/includes/deniam.h",
	MAME_DIR .. "src/mame/video/deniam.cpp",
	MAME_DIR .. "src/mame/drivers/dotrikun.cpp",
	MAME_DIR .. "src/mame/drivers/gpworld.cpp",
	MAME_DIR .. "src/mame/drivers/hikaru.cpp",
	MAME_DIR .. "src/mame/drivers/hshavoc.cpp",
	MAME_DIR .. "src/mame/drivers/kopunch.cpp",
	MAME_DIR .. "src/mame/includes/kopunch.h",
	MAME_DIR .. "src/mame/video/kopunch.cpp",
	MAME_DIR .. "src/mame/drivers/lindbergh.cpp",
	MAME_DIR .. "src/mame/machine/segabb.cpp",
	MAME_DIR .. "src/mame/machine/segabb.h",
	MAME_DIR .. "src/mame/machine/megadriv.cpp",
	MAME_DIR .. "src/mame/includes/md_cons.h",
	MAME_DIR .. "src/mame/drivers/megadrvb.cpp",
	MAME_DIR .. "src/mame/drivers/megaplay.cpp",
	MAME_DIR .. "src/mame/drivers/megatech.cpp",
	MAME_DIR .. "src/mame/drivers/model1.cpp",
	MAME_DIR .. "src/mame/includes/model1.h",
	MAME_DIR .. "src/mame/machine/model1.cpp",
	MAME_DIR .. "src/mame/video/model1.cpp",
	MAME_DIR .. "src/mame/machine/s32comm.cpp",
	MAME_DIR .. "src/mame/machine/s32comm.h",
	MAME_DIR .. "src/mame/machine/m1comm.cpp",
	MAME_DIR .. "src/mame/machine/m1comm.h",
	MAME_DIR .. "src/mame/audio/dsbz80.cpp",
	MAME_DIR .. "src/mame/audio/dsbz80.h",
	MAME_DIR .. "src/mame/drivers/model2.cpp",
	MAME_DIR .. "src/mame/includes/model2.h",
	MAME_DIR .. "src/mame/video/model2rd.inc",
	MAME_DIR .. "src/mame/video/model2.cpp",
	MAME_DIR .. "src/mame/drivers/model3.cpp",
	MAME_DIR .. "src/mame/includes/model3.h",
	MAME_DIR .. "src/mame/video/model3.cpp",
	MAME_DIR .. "src/mame/machine/model3.cpp",
	MAME_DIR .. "src/mame/drivers/monacogp.cpp",
	MAME_DIR .. "src/mame/drivers/naomi.cpp",
	MAME_DIR .. "src/mame/includes/naomi.h",
	MAME_DIR .. "src/mame/includes/dc.h",
	MAME_DIR .. "src/mame/drivers/segasp.cpp",
	MAME_DIR .. "src/mame/includes/segasp.h",
	MAME_DIR .. "src/mame/machine/dc.cpp",
	MAME_DIR .. "src/mame/video/powervr2.cpp",
	MAME_DIR .. "src/mame/video/powervr2.h",
	MAME_DIR .. "src/mame/machine/naomi.cpp",
	MAME_DIR .. "src/mame/machine/naomig1.cpp",
	MAME_DIR .. "src/mame/machine/naomig1.h",
	MAME_DIR .. "src/mame/machine/naomibd.cpp",
	MAME_DIR .. "src/mame/machine/naomibd.h",
	MAME_DIR .. "src/mame/machine/naomirom.cpp",
	MAME_DIR .. "src/mame/machine/naomirom.h",
	MAME_DIR .. "src/mame/machine/naomigd.cpp",
	MAME_DIR .. "src/mame/machine/naomigd.h",
	MAME_DIR .. "src/mame/machine/naomim1.cpp",
	MAME_DIR .. "src/mame/machine/naomim1.h",
	MAME_DIR .. "src/mame/machine/naomim2.cpp",
	MAME_DIR .. "src/mame/machine/naomim2.h",
	MAME_DIR .. "src/mame/machine/naomim4.cpp",
	MAME_DIR .. "src/mame/machine/naomim4.h",
	MAME_DIR .. "src/mame/machine/315-5881_crypt.cpp",
	MAME_DIR .. "src/mame/machine/315-5881_crypt.h",
	MAME_DIR .. "src/mame/machine/awboard.cpp",
	MAME_DIR .. "src/mame/machine/awboard.h",
	MAME_DIR .. "src/mame/machine/mie.cpp",
	MAME_DIR .. "src/mame/machine/mie.h",
	MAME_DIR .. "src/mame/machine/maple-dc.cpp",
	MAME_DIR .. "src/mame/machine/maple-dc.h",
	MAME_DIR .. "src/mame/machine/mapledev.cpp",
	MAME_DIR .. "src/mame/machine/mapledev.h",
	MAME_DIR .. "src/mame/machine/dc-ctrl.cpp",
	MAME_DIR .. "src/mame/machine/dc-ctrl.h",
	MAME_DIR .. "src/mame/machine/jvs13551.cpp",
	MAME_DIR .. "src/mame/machine/jvs13551.h",
	MAME_DIR .. "src/mame/drivers/triforce.cpp",
	MAME_DIR .. "src/mame/drivers/puckpkmn.cpp",
	MAME_DIR .. "src/mame/drivers/segac2.cpp",
	MAME_DIR .. "src/mame/drivers/segae.cpp",
	MAME_DIR .. "src/mame/drivers/shtzone.cpp",
	MAME_DIR .. "src/mame/drivers/segacoin.cpp",
	MAME_DIR .. "src/mame/drivers/segag80r.cpp",
	MAME_DIR .. "src/mame/includes/segag80r.h",
	MAME_DIR .. "src/mame/machine/segag80.cpp",
	MAME_DIR .. "src/mame/machine/segag80.h",
	MAME_DIR .. "src/mame/audio/segag80r.cpp",
	MAME_DIR .. "src/mame/video/segag80r.cpp",
	MAME_DIR .. "src/mame/drivers/segag80v.cpp",
	MAME_DIR .. "src/mame/includes/segag80v.h",
	MAME_DIR .. "src/mame/audio/segag80v.cpp",
	MAME_DIR .. "src/mame/video/segag80v.cpp",
	MAME_DIR .. "src/mame/drivers/segahang.cpp",
	MAME_DIR .. "src/mame/includes/segahang.h",
	MAME_DIR .. "src/mame/video/segahang.cpp",
	MAME_DIR .. "src/mame/drivers/segajw.cpp",
	MAME_DIR .. "src/mame/drivers/segald.cpp",
	MAME_DIR .. "src/mame/drivers/segaorun.cpp",
	MAME_DIR .. "src/mame/includes/segaorun.h",
	MAME_DIR .. "src/mame/video/segaorun.cpp",
	MAME_DIR .. "src/mame/drivers/segas16a.cpp",
	MAME_DIR .. "src/mame/includes/segas16a.h",
	MAME_DIR .. "src/mame/video/segas16a.cpp",
	MAME_DIR .. "src/mame/drivers/segas16b.cpp",
	MAME_DIR .. "src/mame/includes/segas16b.h",
	MAME_DIR .. "src/mame/video/segas16b.cpp",
	MAME_DIR .. "src/mame/drivers/segas18.cpp",
	MAME_DIR .. "src/mame/includes/segas18.h",
	MAME_DIR .. "src/mame/video/segas18.cpp",
	MAME_DIR .. "src/mame/drivers/segas24.cpp",
	MAME_DIR .. "src/mame/includes/segas24.h",
	MAME_DIR .. "src/mame/video/segas24.cpp",
	MAME_DIR .. "src/mame/drivers/segam1.cpp",
	MAME_DIR .. "src/mame/drivers/segas32.cpp",
	MAME_DIR .. "src/mame/includes/segas32.h",
	MAME_DIR .. "src/mame/machine/segas32.cpp",
	MAME_DIR .. "src/mame/video/segas32.cpp",
	MAME_DIR .. "src/mame/drivers/segaufo.cpp",
	MAME_DIR .. "src/mame/drivers/segaxbd.cpp",
	MAME_DIR .. "src/mame/includes/segaxbd.h",
	MAME_DIR .. "src/mame/video/segaxbd.cpp",
	MAME_DIR .. "src/mame/drivers/segaybd.cpp",
	MAME_DIR .. "src/mame/includes/segaybd.h",
	MAME_DIR .. "src/mame/video/segaybd.cpp",
	MAME_DIR .. "src/mame/includes/segaipt.h",
	MAME_DIR .. "src/mame/drivers/sg1000a.cpp",
	MAME_DIR .. "src/mame/drivers/stactics.cpp",
	MAME_DIR .. "src/mame/includes/stactics.h",
	MAME_DIR .. "src/mame/video/stactics.cpp",
	MAME_DIR .. "src/mame/drivers/stv.cpp",
	MAME_DIR .. "src/mame/includes/stv.h",
	MAME_DIR .. "src/mame/machine/stvprot.cpp",
	MAME_DIR .. "src/mame/machine/315-5838_317-0229_comp.cpp",
	MAME_DIR .. "src/mame/machine/315-5838_317-0229_comp.h",
	MAME_DIR .. "src/mame/drivers/suprloco.cpp",
	MAME_DIR .. "src/mame/includes/suprloco.h",
	MAME_DIR .. "src/mame/video/suprloco.cpp",
	MAME_DIR .. "src/mame/drivers/system1.cpp",
	MAME_DIR .. "src/mame/includes/system1.h",
	MAME_DIR .. "src/mame/video/system1.cpp",
	MAME_DIR .. "src/mame/drivers/system16.cpp",
	MAME_DIR .. "src/mame/includes/system16.h",
	MAME_DIR .. "src/mame/video/system16.cpp",
	MAME_DIR .. "src/mame/drivers/timetrv.cpp",
	MAME_DIR .. "src/mame/drivers/turbo.cpp",
	MAME_DIR .. "src/mame/includes/turbo.h",
	MAME_DIR .. "src/mame/audio/turbo.cpp",
	MAME_DIR .. "src/mame/video/turbo.cpp",
	MAME_DIR .. "src/mame/drivers/vicdual.cpp",
	MAME_DIR .. "src/mame/includes/vicdual.h",
	MAME_DIR .. "src/mame/audio/vicdual.cpp",
	MAME_DIR .. "src/mame/video/vicdual.cpp",
	MAME_DIR .. "src/mame/audio/carnival.cpp",
	MAME_DIR .. "src/mame/audio/depthch.cpp",
	MAME_DIR .. "src/mame/audio/invinco.cpp",
	MAME_DIR .. "src/mame/audio/pulsar.cpp",
	MAME_DIR .. "src/mame/drivers/zaxxon.cpp",
	MAME_DIR .. "src/mame/includes/zaxxon.h",
	MAME_DIR .. "src/mame/audio/zaxxon.cpp",
	MAME_DIR .. "src/mame/video/zaxxon.cpp",
	MAME_DIR .. "src/mame/machine/315_5296.cpp",
	MAME_DIR .. "src/mame/machine/315_5296.h",
	MAME_DIR .. "src/mame/machine/fd1089.cpp",
	MAME_DIR .. "src/mame/machine/fd1089.h",
	MAME_DIR .. "src/mame/machine/fd1094.cpp",
	MAME_DIR .. "src/mame/machine/fd1094.h",
	MAME_DIR .. "src/mame/machine/fddebug.cpp",
	MAME_DIR .. "src/mame/machine/fddebug.h",
	MAME_DIR .. "src/mame/machine/mc8123.cpp",
	MAME_DIR .. "src/mame/machine/mc8123.h",
	MAME_DIR .. "src/mame/machine/segaic16.cpp",
	MAME_DIR .. "src/mame/machine/segaic16.h",
	MAME_DIR .. "src/mame/audio/segasnd.cpp",
	MAME_DIR .. "src/mame/audio/segasnd.h",
	MAME_DIR .. "src/mame/video/segaic16.cpp",
	MAME_DIR .. "src/mame/video/segaic16.h",
	MAME_DIR .. "src/mame/video/segaic16_road.cpp",
	MAME_DIR .. "src/mame/video/segaic16_road.h",
	MAME_DIR .. "src/mame/video/sega16sp.cpp",
	MAME_DIR .. "src/mame/video/sega16sp.h",
	MAME_DIR .. "src/mame/video/segaic24.cpp",
	MAME_DIR .. "src/mame/video/segaic24.h",
	MAME_DIR .. "src/mame/machine/gdrom.cpp",
	MAME_DIR .. "src/mame/machine/gdrom.h",
	MAME_DIR .. "src/mame/machine/xbox.cpp",
}

createMAMEProjects(_target, _subtarget, "seibu")
files {
	MAME_DIR .. "src/mame/drivers/bloodbro.cpp",
	MAME_DIR .. "src/mame/includes/bloodbro.h",
	MAME_DIR .. "src/mame/video/bloodbro.cpp",
	MAME_DIR .. "src/mame/drivers/cabal.cpp",
	MAME_DIR .. "src/mame/includes/cabal.h",
	MAME_DIR .. "src/mame/video/cabal.cpp",
	MAME_DIR .. "src/mame/drivers/cshooter.cpp",
	MAME_DIR .. "src/mame/drivers/dcon.cpp",
	MAME_DIR .. "src/mame/includes/dcon.h",
	MAME_DIR .. "src/mame/video/dcon.cpp",
	MAME_DIR .. "src/mame/drivers/deadang.cpp",
	MAME_DIR .. "src/mame/includes/deadang.h",
	MAME_DIR .. "src/mame/video/deadang.cpp",
	MAME_DIR .. "src/mame/drivers/dynduke.cpp",
	MAME_DIR .. "src/mame/includes/dynduke.h",
	MAME_DIR .. "src/mame/video/dynduke.cpp",
	MAME_DIR .. "src/mame/drivers/feversoc.cpp",
	MAME_DIR .. "src/mame/drivers/goal92.cpp",
	MAME_DIR .. "src/mame/includes/goal92.h",
	MAME_DIR .. "src/mame/video/goal92.cpp",
	MAME_DIR .. "src/mame/drivers/goodejan.cpp",
	MAME_DIR .. "src/mame/drivers/kncljoe.cpp",
	MAME_DIR .. "src/mame/includes/kncljoe.h",
	MAME_DIR .. "src/mame/video/kncljoe.cpp",
	MAME_DIR .. "src/mame/drivers/legionna.cpp",
	MAME_DIR .. "src/mame/includes/legionna.h",
	MAME_DIR .. "src/mame/video/legionna.cpp",
	MAME_DIR .. "src/mame/drivers/mustache.cpp",
	MAME_DIR .. "src/mame/includes/mustache.h",
	MAME_DIR .. "src/mame/video/mustache.cpp",
	MAME_DIR .. "src/mame/drivers/panicr.cpp",
	MAME_DIR .. "src/mame/drivers/raiden.cpp",
	MAME_DIR .. "src/mame/includes/raiden.h",
	MAME_DIR .. "src/mame/video/raiden.cpp",
	MAME_DIR .. "src/mame/drivers/raiden2.cpp",
	MAME_DIR .. "src/mame/includes/raiden2.h",
	MAME_DIR .. "src/mame/machine/r2crypt.cpp",
	MAME_DIR .. "src/mame/machine/raiden2cop.cpp",
	MAME_DIR .. "src/mame/machine/raiden2cop.h",
	MAME_DIR .. "src/mame/drivers/r2dx_v33.cpp",
	MAME_DIR .. "src/mame/drivers/seibuspi.cpp",
	MAME_DIR .. "src/mame/includes/seibuspi.h",
	MAME_DIR .. "src/mame/machine/seibuspi.cpp",
	MAME_DIR .. "src/mame/machine/seibuspi.h",
	MAME_DIR .. "src/mame/video/seibuspi.cpp",
	MAME_DIR .. "src/mame/drivers/sengokmj.cpp",
	MAME_DIR .. "src/mame/drivers/stfight.cpp",
	MAME_DIR .. "src/mame/includes/stfight.h",
	MAME_DIR .. "src/mame/machine/stfight.cpp",
	MAME_DIR .. "src/mame/video/stfight.cpp",
	MAME_DIR .. "src/mame/drivers/toki.cpp",
	MAME_DIR .. "src/mame/includes/toki.h",
	MAME_DIR .. "src/mame/video/toki.cpp",
	MAME_DIR .. "src/mame/drivers/wiz.cpp",
	MAME_DIR .. "src/mame/includes/wiz.h",
	MAME_DIR .. "src/mame/video/wiz.cpp",
	MAME_DIR .. "src/mame/machine/seicop.cpp",
	MAME_DIR .. "src/mame/machine/seicop.h",
	MAME_DIR .. "src/mame/machine/spisprit.cpp",
	MAME_DIR .. "src/mame/audio/seibu.cpp",
	MAME_DIR .. "src/mame/audio/seibu.h",
	MAME_DIR .. "src/mame/video/seibu_crtc.cpp",
	MAME_DIR .. "src/mame/video/seibu_crtc.h",
}

createMAMEProjects(_target, _subtarget, "seta")
files {
	MAME_DIR .. "src/mame/drivers/aleck64.cpp",
	MAME_DIR .. "src/mame/machine/n64.cpp",
	MAME_DIR .. "src/mame/video/n64.cpp",
	MAME_DIR .. "src/mame/video/n64types.h",
	MAME_DIR .. "src/mame/video/rdpfiltr.inc",		
	MAME_DIR .. "src/mame/video/n64.h",
	MAME_DIR .. "src/mame/video/rdpblend.cpp",
	MAME_DIR .. "src/mame/video/rdpblend.h",
	MAME_DIR .. "src/mame/video/rdptpipe.cpp",
	MAME_DIR .. "src/mame/video/rdptpipe.h",
	MAME_DIR .. "src/mame/drivers/hanaawas.cpp",
	MAME_DIR .. "src/mame/includes/hanaawas.h",
	MAME_DIR .. "src/mame/video/hanaawas.cpp",
	MAME_DIR .. "src/mame/drivers/jclub2.cpp",
	MAME_DIR .. "src/mame/drivers/macs.cpp",
	MAME_DIR .. "src/mame/drivers/seta.cpp",
	MAME_DIR .. "src/mame/includes/seta.h",
	MAME_DIR .. "src/mame/video/seta.cpp",
	MAME_DIR .. "src/mame/drivers/seta2.cpp",
	MAME_DIR .. "src/mame/includes/seta2.h",
	MAME_DIR .. "src/mame/video/seta2.cpp",
	MAME_DIR .. "src/mame/drivers/speedatk.cpp",
	MAME_DIR .. "src/mame/includes/speedatk.h",
	MAME_DIR .. "src/mame/video/speedatk.cpp",
	MAME_DIR .. "src/mame/drivers/speglsht.cpp",
	MAME_DIR .. "src/mame/drivers/srmp2.cpp",
	MAME_DIR .. "src/mame/includes/srmp2.h",
	MAME_DIR .. "src/mame/video/srmp2.cpp",
	MAME_DIR .. "src/mame/drivers/srmp5.cpp",
	MAME_DIR .. "src/mame/drivers/srmp6.cpp",
	MAME_DIR .. "src/mame/drivers/ssv.cpp",
	MAME_DIR .. "src/mame/includes/ssv.h",
	MAME_DIR .. "src/mame/video/ssv.cpp",
	MAME_DIR .. "src/mame/video/st0020.cpp",
	MAME_DIR .. "src/mame/video/st0020.h",
	MAME_DIR .. "src/mame/machine/st0016.cpp",
	MAME_DIR .. "src/mame/machine/st0016.h",
	MAME_DIR .. "src/mame/drivers/simple_st0016.cpp",
	MAME_DIR .. "src/mame/includes/simple_st0016.h",
	MAME_DIR .. "src/mame/video/seta001.cpp",
	MAME_DIR .. "src/mame/video/seta001.h",
	MAME_DIR .. "src/mame/drivers/thedealr.cpp",
}

createMAMEProjects(_target, _subtarget, "sigma")
files {
	MAME_DIR .. "src/mame/drivers/nyny.cpp",
	MAME_DIR .. "src/mame/drivers/r2dtank.cpp",
	MAME_DIR .. "src/mame/drivers/sigmab52.cpp",
	MAME_DIR .. "src/mame/drivers/sigmab98.cpp",
	MAME_DIR .. "src/mame/drivers/spiders.cpp",
	MAME_DIR .. "src/mame/includes/spiders.h",
	MAME_DIR .. "src/mame/audio/spiders.cpp",
	MAME_DIR .. "src/mame/drivers/sub.cpp",
}

createMAMEProjects(_target, _subtarget, "snk")
files {
	MAME_DIR .. "src/mame/drivers/bbusters.cpp",
	MAME_DIR .. "src/mame/includes/bbusters.h",
	MAME_DIR .. "src/mame/video/bbusters.cpp",
	MAME_DIR .. "src/mame/drivers/dmndrby.cpp",
	MAME_DIR .. "src/mame/drivers/hng64.cpp",
	MAME_DIR .. "src/mame/includes/hng64.h",
	MAME_DIR .. "src/mame/video/hng64.cpp",
	MAME_DIR .. "src/mame/audio/hng64.cpp",
	MAME_DIR .. "src/mame/machine/hng64_net.cpp",
	MAME_DIR .. "src/mame/video/hng64_3d.cpp",
	MAME_DIR .. "src/mame/video/hng64_sprite.cpp",
	MAME_DIR .. "src/mame/drivers/lasso.cpp",
	MAME_DIR .. "src/mame/includes/lasso.h",
	MAME_DIR .. "src/mame/video/lasso.cpp",
	MAME_DIR .. "src/mame/drivers/mainsnk.cpp",
	MAME_DIR .. "src/mame/includes/mainsnk.h",
	MAME_DIR .. "src/mame/video/mainsnk.cpp",
	MAME_DIR .. "src/mame/drivers/munchmo.cpp",
	MAME_DIR .. "src/mame/includes/munchmo.h",
	MAME_DIR .. "src/mame/video/munchmo.cpp",
	MAME_DIR .. "src/mame/drivers/prehisle.cpp",
	MAME_DIR .. "src/mame/includes/prehisle.h",
	MAME_DIR .. "src/mame/video/prehisle.cpp",
	MAME_DIR .. "src/mame/drivers/snk6502.cpp",
	MAME_DIR .. "src/mame/includes/snk6502.h",
	MAME_DIR .. "src/mame/audio/snk6502.cpp",
	MAME_DIR .. "src/mame/video/snk6502.cpp",
	MAME_DIR .. "src/mame/drivers/snk.cpp",
	MAME_DIR .. "src/mame/includes/snk.h",
	MAME_DIR .. "src/mame/video/snk.cpp",
	MAME_DIR .. "src/mame/drivers/snk68.cpp",
	MAME_DIR .. "src/mame/includes/snk68.h",
	MAME_DIR .. "src/mame/video/snk68.cpp",
}

createMAMEProjects(_target, _subtarget, "sony")
files {
	MAME_DIR .. "src/mame/drivers/zn.cpp",
	MAME_DIR .. "src/mame/machine/zndip.cpp",
	MAME_DIR .. "src/mame/machine/zndip.h",
	MAME_DIR .. "src/mame/machine/cat702.cpp",
	MAME_DIR .. "src/mame/machine/cat702.h",
}

createMAMEProjects(_target, _subtarget, "stern")
files {
	MAME_DIR .. "src/mame/drivers/astinvad.cpp",
	MAME_DIR .. "src/mame/drivers/berzerk.cpp",
	MAME_DIR .. "src/mame/drivers/cliffhgr.cpp",
	MAME_DIR .. "src/mame/audio/cliffhgr.cpp",
	MAME_DIR .. "src/mame/drivers/mazerbla.cpp",
	MAME_DIR .. "src/mame/drivers/supdrapo.cpp",
}

createMAMEProjects(_target, _subtarget, "subsino")
files {
	MAME_DIR .. "src/mame/drivers/lastfght.cpp",
	MAME_DIR .. "src/mame/drivers/subsino.cpp",
	MAME_DIR .. "src/mame/drivers/subsino2.cpp",
	MAME_DIR .. "src/mame/machine/subsino.cpp",
	MAME_DIR .. "src/mame/machine/subsino.h",
}

createMAMEProjects(_target, _subtarget, "sun")
files {
	MAME_DIR .. "src/mame/drivers/arabian.cpp",
	MAME_DIR .. "src/mame/includes/arabian.h",
	MAME_DIR .. "src/mame/video/arabian.cpp",
	MAME_DIR .. "src/mame/drivers/dai3wksi.cpp",
	MAME_DIR .. "src/mame/drivers/ikki.cpp",
	MAME_DIR .. "src/mame/includes/ikki.h",
	MAME_DIR .. "src/mame/video/ikki.cpp",
	MAME_DIR .. "src/mame/drivers/kangaroo.cpp",
	MAME_DIR .. "src/mame/includes/kangaroo.h",
	MAME_DIR .. "src/mame/video/kangaroo.cpp",
	MAME_DIR .. "src/mame/drivers/markham.cpp",
	MAME_DIR .. "src/mame/includes/markham.h",
	MAME_DIR .. "src/mame/video/markham.cpp",
	MAME_DIR .. "src/mame/drivers/route16.cpp",
	MAME_DIR .. "src/mame/includes/route16.h",
	MAME_DIR .. "src/mame/video/route16.cpp",
	MAME_DIR .. "src/mame/drivers/shanghai.cpp",
	MAME_DIR .. "src/mame/drivers/shangha3.cpp",
	MAME_DIR .. "src/mame/includes/shangha3.h",
	MAME_DIR .. "src/mame/video/shangha3.cpp",
	MAME_DIR .. "src/mame/drivers/strnskil.cpp",
	MAME_DIR .. "src/mame/includes/strnskil.h",
	MAME_DIR .. "src/mame/video/strnskil.cpp",
	MAME_DIR .. "src/mame/drivers/tonton.cpp",
}

createMAMEProjects(_target, _subtarget, "suna")
files {
	MAME_DIR .. "src/mame/drivers/go2000.cpp",
	MAME_DIR .. "src/mame/drivers/goindol.cpp",
	MAME_DIR .. "src/mame/includes/goindol.h",
	MAME_DIR .. "src/mame/video/goindol.cpp",
	MAME_DIR .. "src/mame/drivers/suna8.cpp",
	MAME_DIR .. "src/mame/includes/suna8.h",
	MAME_DIR .. "src/mame/audio/suna8.cpp",
	MAME_DIR .. "src/mame/video/suna8.cpp",
	MAME_DIR .. "src/mame/drivers/suna16.cpp",
	MAME_DIR .. "src/mame/includes/suna16.h",
	MAME_DIR .. "src/mame/video/suna16.cpp",
}

createMAMEProjects(_target, _subtarget, "sure")
files {
	MAME_DIR .. "src/mame/drivers/mil4000.cpp",

}

createMAMEProjects(_target, _subtarget, "taito")
files {
	MAME_DIR .. "src/mame/drivers/2mindril.cpp",
	MAME_DIR .. "src/mame/drivers/40love.cpp",
	MAME_DIR .. "src/mame/includes/40love.h",
	MAME_DIR .. "src/mame/video/40love.cpp",
	MAME_DIR .. "src/mame/drivers/arkanoid.cpp",
	MAME_DIR .. "src/mame/includes/arkanoid.h",
	MAME_DIR .. "src/mame/machine/arkanoid.cpp",
	MAME_DIR .. "src/mame/video/arkanoid.cpp",
	MAME_DIR .. "src/mame/drivers/ashnojoe.cpp",
	MAME_DIR .. "src/mame/includes/ashnojoe.h",
	MAME_DIR .. "src/mame/video/ashnojoe.cpp",
	MAME_DIR .. "src/mame/drivers/asuka.cpp",
	MAME_DIR .. "src/mame/includes/asuka.h",
	MAME_DIR .. "src/mame/machine/bonzeadv.cpp",
	MAME_DIR .. "src/mame/video/asuka.cpp",
	MAME_DIR .. "src/mame/drivers/bigevglf.cpp",
	MAME_DIR .. "src/mame/includes/bigevglf.h",
	MAME_DIR .. "src/mame/machine/bigevglf.cpp",
	MAME_DIR .. "src/mame/video/bigevglf.cpp",
	MAME_DIR .. "src/mame/drivers/bking.cpp",
	MAME_DIR .. "src/mame/includes/bking.h",
	MAME_DIR .. "src/mame/video/bking.cpp",
	MAME_DIR .. "src/mame/drivers/bublbobl.cpp",
	MAME_DIR .. "src/mame/includes/bublbobl.h",
	MAME_DIR .. "src/mame/machine/bublbobl.cpp",
	MAME_DIR .. "src/mame/video/bublbobl.cpp",
	MAME_DIR .. "src/mame/drivers/buggychl.cpp",
	MAME_DIR .. "src/mame/includes/buggychl.h",
	MAME_DIR .. "src/mame/machine/buggychl.cpp",
	MAME_DIR .. "src/mame/machine/buggychl.h",
	MAME_DIR .. "src/mame/video/buggychl.cpp",
	MAME_DIR .. "src/mame/drivers/capr1.cpp",
	MAME_DIR .. "src/mame/drivers/caprcyc.cpp",
	MAME_DIR .. "src/mame/drivers/cchance.cpp",
	MAME_DIR .. "src/mame/drivers/chaknpop.cpp",
	MAME_DIR .. "src/mame/includes/chaknpop.h",
	MAME_DIR .. "src/mame/machine/chaknpop.cpp",
	MAME_DIR .. "src/mame/video/chaknpop.cpp",
	MAME_DIR .. "src/mame/drivers/champbwl.cpp",
	MAME_DIR .. "src/mame/drivers/changela.cpp",
	MAME_DIR .. "src/mame/includes/changela.h",
	MAME_DIR .. "src/mame/video/changela.cpp",
	MAME_DIR .. "src/mame/drivers/crbaloon.cpp",
	MAME_DIR .. "src/mame/includes/crbaloon.h",
	MAME_DIR .. "src/mame/video/crbaloon.cpp",
	MAME_DIR .. "src/mame/audio/crbaloon.cpp",
	MAME_DIR .. "src/mame/drivers/cyclemb.cpp",
	MAME_DIR .. "src/mame/drivers/darius.cpp",
	MAME_DIR .. "src/mame/includes/darius.h",
	MAME_DIR .. "src/mame/video/darius.cpp",
	MAME_DIR .. "src/mame/drivers/darkmist.cpp",
	MAME_DIR .. "src/mame/includes/darkmist.h",
	MAME_DIR .. "src/mame/video/darkmist.cpp",
	MAME_DIR .. "src/mame/drivers/exzisus.cpp",
	MAME_DIR .. "src/mame/includes/exzisus.h",
	MAME_DIR .. "src/mame/video/exzisus.cpp",
	MAME_DIR .. "src/mame/drivers/fgoal.cpp",
	MAME_DIR .. "src/mame/includes/fgoal.h",
	MAME_DIR .. "src/mame/video/fgoal.cpp",
	MAME_DIR .. "src/mame/drivers/flstory.cpp",
	MAME_DIR .. "src/mame/includes/flstory.h",
	MAME_DIR .. "src/mame/machine/flstory.cpp",
	MAME_DIR .. "src/mame/video/flstory.cpp",
	MAME_DIR .. "src/mame/drivers/galastrm.cpp",
	MAME_DIR .. "src/mame/includes/galastrm.h",
	MAME_DIR .. "src/mame/video/galastrm.cpp",
	MAME_DIR .. "src/mame/drivers/gladiatr.cpp",
	MAME_DIR .. "src/mame/includes/gladiatr.h",
	MAME_DIR .. "src/mame/video/gladiatr.cpp",
	MAME_DIR .. "src/mame/drivers/grchamp.cpp",
	MAME_DIR .. "src/mame/includes/grchamp.h",
	MAME_DIR .. "src/mame/audio/grchamp.cpp",
	MAME_DIR .. "src/mame/video/grchamp.cpp",
	MAME_DIR .. "src/mame/drivers/groundfx.cpp",
	MAME_DIR .. "src/mame/includes/groundfx.h",
	MAME_DIR .. "src/mame/video/groundfx.cpp",
	MAME_DIR .. "src/mame/drivers/gsword.cpp",
	MAME_DIR .. "src/mame/includes/gsword.h",
	MAME_DIR .. "src/mame/machine/tait8741.cpp",
	MAME_DIR .. "src/mame/machine/tait8741.h",
	MAME_DIR .. "src/mame/video/gsword.cpp",
	MAME_DIR .. "src/mame/drivers/gunbustr.cpp",
	MAME_DIR .. "src/mame/includes/gunbustr.h",
	MAME_DIR .. "src/mame/video/gunbustr.cpp",
	MAME_DIR .. "src/mame/drivers/halleys.cpp",
	MAME_DIR .. "src/mame/drivers/invqix.cpp",
	MAME_DIR .. "src/mame/drivers/jollyjgr.cpp",
	MAME_DIR .. "src/mame/drivers/ksayakyu.cpp",
	MAME_DIR .. "src/mame/includes/ksayakyu.h",
	MAME_DIR .. "src/mame/video/ksayakyu.cpp",
	MAME_DIR .. "src/mame/drivers/lgp.cpp",
	MAME_DIR .. "src/mame/drivers/lkage.cpp",
	MAME_DIR .. "src/mame/includes/lkage.h",
	MAME_DIR .. "src/mame/machine/lkage.cpp",
	MAME_DIR .. "src/mame/video/lkage.cpp",
	MAME_DIR .. "src/mame/drivers/lsasquad.cpp",
	MAME_DIR .. "src/mame/includes/lsasquad.h",
	MAME_DIR .. "src/mame/machine/lsasquad.cpp",
	MAME_DIR .. "src/mame/video/lsasquad.cpp",
	MAME_DIR .. "src/mame/drivers/marinedt.cpp",
	MAME_DIR .. "src/mame/drivers/mexico86.cpp",
	MAME_DIR .. "src/mame/includes/mexico86.h",
	MAME_DIR .. "src/mame/machine/mexico86.cpp",
	MAME_DIR .. "src/mame/video/mexico86.cpp",
	MAME_DIR .. "src/mame/drivers/minivadr.cpp",
	MAME_DIR .. "src/mame/drivers/missb2.cpp",
	MAME_DIR .. "src/mame/drivers/mlanding.cpp",
	MAME_DIR .. "src/mame/drivers/msisaac.cpp",
	MAME_DIR .. "src/mame/includes/msisaac.h",
	MAME_DIR .. "src/mame/video/msisaac.cpp",
	MAME_DIR .. "src/mame/drivers/ninjaw.cpp",
	MAME_DIR .. "src/mame/includes/ninjaw.h",
	MAME_DIR .. "src/mame/video/ninjaw.cpp",
	MAME_DIR .. "src/mame/drivers/nycaptor.cpp",
	MAME_DIR .. "src/mame/includes/nycaptor.h",
	MAME_DIR .. "src/mame/machine/nycaptor.cpp",
	MAME_DIR .. "src/mame/video/nycaptor.cpp",
	MAME_DIR .. "src/mame/drivers/opwolf.cpp",
	MAME_DIR .. "src/mame/includes/opwolf.h",
	MAME_DIR .. "src/mame/machine/opwolf.cpp",
	MAME_DIR .. "src/mame/video/opwolf.cpp",
	MAME_DIR .. "src/mame/drivers/othunder.cpp",
	MAME_DIR .. "src/mame/includes/othunder.h",
	MAME_DIR .. "src/mame/video/othunder.cpp",
	MAME_DIR .. "src/mame/drivers/pitnrun.cpp",
	MAME_DIR .. "src/mame/includes/pitnrun.h",
	MAME_DIR .. "src/mame/machine/pitnrun.cpp",
	MAME_DIR .. "src/mame/video/pitnrun.cpp",
	MAME_DIR .. "src/mame/drivers/qix.cpp",
	MAME_DIR .. "src/mame/includes/qix.h",
	MAME_DIR .. "src/mame/machine/qix.cpp",
	MAME_DIR .. "src/mame/audio/qix.cpp",
	MAME_DIR .. "src/mame/video/qix.cpp",
	MAME_DIR .. "src/mame/drivers/rbisland.cpp",
	MAME_DIR .. "src/mame/includes/rbisland.h",
	MAME_DIR .. "src/mame/machine/rbisland.cpp",
	MAME_DIR .. "src/mame/video/rbisland.cpp",
	MAME_DIR .. "src/mame/drivers/rastan.cpp",
	MAME_DIR .. "src/mame/includes/rastan.h",
	MAME_DIR .. "src/mame/video/rastan.cpp",
	MAME_DIR .. "src/mame/drivers/retofinv.cpp",
	MAME_DIR .. "src/mame/includes/retofinv.h",
	MAME_DIR .. "src/mame/machine/retofinv.cpp",
	MAME_DIR .. "src/mame/video/retofinv.cpp",
	MAME_DIR .. "src/mame/drivers/rollrace.cpp",
	MAME_DIR .. "src/mame/includes/rollrace.h",
	MAME_DIR .. "src/mame/video/rollrace.cpp",
	MAME_DIR .. "src/mame/drivers/sbowling.cpp",
	MAME_DIR .. "src/mame/drivers/slapshot.cpp",
	MAME_DIR .. "src/mame/includes/slapshot.h",
	MAME_DIR .. "src/mame/video/slapshot.cpp",
	MAME_DIR .. "src/mame/drivers/ssrj.cpp",
	MAME_DIR .. "src/mame/includes/ssrj.h",
	MAME_DIR .. "src/mame/video/ssrj.cpp",
	MAME_DIR .. "src/mame/drivers/superchs.cpp",
	MAME_DIR .. "src/mame/includes/superchs.h",
	MAME_DIR .. "src/mame/video/superchs.cpp",
	MAME_DIR .. "src/mame/drivers/superqix.cpp",
	MAME_DIR .. "src/mame/includes/superqix.h",
	MAME_DIR .. "src/mame/video/superqix.cpp",
	MAME_DIR .. "src/mame/drivers/taito_b.cpp",
	MAME_DIR .. "src/mame/includes/taito_b.h",
	MAME_DIR .. "src/mame/video/taito_b.cpp",
	MAME_DIR .. "src/mame/includes/taitoipt.h",
	MAME_DIR .. "src/mame/drivers/taito_f2.cpp",
	MAME_DIR .. "src/mame/includes/taito_f2.h",
	MAME_DIR .. "src/mame/video/taito_f2.cpp",
	MAME_DIR .. "src/mame/drivers/taito_f3.cpp",
	MAME_DIR .. "src/mame/includes/taito_f3.h",
	MAME_DIR .. "src/mame/video/taito_f3.cpp",
	MAME_DIR .. "src/mame/audio/taito_en.cpp",
	MAME_DIR .. "src/mame/audio/taito_en.h",
	MAME_DIR .. "src/mame/drivers/taito_h.cpp",
	MAME_DIR .. "src/mame/includes/taito_h.h",
	MAME_DIR .. "src/mame/video/taito_h.cpp",
	MAME_DIR .. "src/mame/drivers/taito_l.cpp",
	MAME_DIR .. "src/mame/includes/taito_l.h",
	MAME_DIR .. "src/mame/video/taito_l.cpp",
	MAME_DIR .. "src/mame/drivers/taito_x.cpp",
	MAME_DIR .. "src/mame/includes/taito_x.h",
	MAME_DIR .. "src/mame/machine/cchip.cpp",
	MAME_DIR .. "src/mame/drivers/taito_z.cpp",
	MAME_DIR .. "src/mame/includes/taito_z.h",
	MAME_DIR .. "src/mame/video/taito_z.cpp",
	MAME_DIR .. "src/mame/drivers/taito_o.cpp",
	MAME_DIR .. "src/mame/includes/taito_o.h",
	MAME_DIR .. "src/mame/video/taito_o.cpp",
	MAME_DIR .. "src/mame/drivers/taitoair.cpp",
	MAME_DIR .. "src/mame/includes/taitoair.h",
	MAME_DIR .. "src/mame/video/taitoair.cpp",
	MAME_DIR .. "src/mame/drivers/taitogn.cpp",
	MAME_DIR .. "src/mame/drivers/taitojc.cpp",
	MAME_DIR .. "src/mame/includes/taitojc.h",
	MAME_DIR .. "src/mame/video/taitojc.cpp",
	MAME_DIR .. "src/mame/drivers/taitopjc.cpp",
	MAME_DIR .. "src/mame/drivers/taitosj.cpp",
	MAME_DIR .. "src/mame/includes/taitosj.h",
	MAME_DIR .. "src/mame/machine/taitosj.cpp",
	MAME_DIR .. "src/mame/video/taitosj.cpp",
	MAME_DIR .. "src/mame/drivers/taitottl.cpp",
	MAME_DIR .. "src/mame/drivers/taitotz.cpp",
	MAME_DIR .. "src/mame/drivers/taitotx.cpp",
	MAME_DIR .. "src/mame/drivers/taitowlf.cpp",
	MAME_DIR .. "src/mame/drivers/tnzs.cpp",
	MAME_DIR .. "src/mame/includes/tnzs.h",
	MAME_DIR .. "src/mame/machine/tnzs.cpp",
	MAME_DIR .. "src/mame/video/tnzs.cpp",
	MAME_DIR .. "src/mame/drivers/topspeed.cpp",
	MAME_DIR .. "src/mame/includes/topspeed.h",
	MAME_DIR .. "src/mame/video/topspeed.cpp",
	MAME_DIR .. "src/mame/drivers/tsamurai.cpp",
	MAME_DIR .. "src/mame/includes/tsamurai.h",
	MAME_DIR .. "src/mame/video/tsamurai.cpp",
	MAME_DIR .. "src/mame/drivers/undrfire.cpp",
	MAME_DIR .. "src/mame/includes/undrfire.h",
	MAME_DIR .. "src/mame/video/undrfire.cpp",
	MAME_DIR .. "src/mame/drivers/volfied.cpp",
	MAME_DIR .. "src/mame/includes/volfied.h",
	MAME_DIR .. "src/mame/machine/volfied.cpp",
	MAME_DIR .. "src/mame/video/volfied.cpp",
	MAME_DIR .. "src/mame/drivers/warriorb.cpp",
	MAME_DIR .. "src/mame/includes/warriorb.h",
	MAME_DIR .. "src/mame/video/warriorb.cpp",
	MAME_DIR .. "src/mame/drivers/wgp.cpp",
	MAME_DIR .. "src/mame/includes/wgp.h",
	MAME_DIR .. "src/mame/video/wgp.cpp",
	MAME_DIR .. "src/mame/drivers/wyvernf0.cpp",
	MAME_DIR .. "src/mame/audio/taitosnd.cpp",
	MAME_DIR .. "src/mame/audio/taitosnd.h",
	MAME_DIR .. "src/mame/audio/taito_zm.cpp",
	MAME_DIR .. "src/mame/audio/taito_zm.h",
	MAME_DIR .. "src/mame/audio/t5182.cpp",
	MAME_DIR .. "src/mame/audio/t5182.h",
	MAME_DIR .. "src/mame/machine/taitoio.cpp",
	MAME_DIR .. "src/mame/machine/taitoio.h",
	MAME_DIR .. "src/mame/video/taito_helper.cpp",
	MAME_DIR .. "src/mame/video/taito_helper.h",
	MAME_DIR .. "src/mame/video/pc080sn.cpp",
	MAME_DIR .. "src/mame/video/pc080sn.h",
	MAME_DIR .. "src/mame/video/pc090oj.cpp",
	MAME_DIR .. "src/mame/video/pc090oj.h",
	MAME_DIR .. "src/mame/video/tc0080vco.cpp",
	MAME_DIR .. "src/mame/video/tc0080vco.h",
	MAME_DIR .. "src/mame/video/tc0100scn.cpp",
	MAME_DIR .. "src/mame/video/tc0100scn.h",
	MAME_DIR .. "src/mame/video/tc0150rod.cpp",
	MAME_DIR .. "src/mame/video/tc0150rod.h",
	MAME_DIR .. "src/mame/video/tc0280grd.cpp",
	MAME_DIR .. "src/mame/video/tc0280grd.h",
	MAME_DIR .. "src/mame/video/tc0360pri.cpp",
	MAME_DIR .. "src/mame/video/tc0360pri.h",
	MAME_DIR .. "src/mame/video/tc0480scp.cpp",
	MAME_DIR .. "src/mame/video/tc0480scp.h",
	MAME_DIR .. "src/mame/video/tc0110pcr.cpp",
	MAME_DIR .. "src/mame/video/tc0110pcr.h",
	MAME_DIR .. "src/mame/video/tc0180vcu.cpp",
	MAME_DIR .. "src/mame/video/tc0180vcu.h",
	MAME_DIR .. "src/mame/video/tc0780fpa.cpp",
	MAME_DIR .. "src/mame/video/tc0780fpa.h",
}

createMAMEProjects(_target, _subtarget, "tatsumi")
files {
	MAME_DIR .. "src/mame/drivers/kingdrby.cpp",
	MAME_DIR .. "src/mame/drivers/lockon.cpp",
	MAME_DIR .. "src/mame/includes/lockon.h",
	MAME_DIR .. "src/mame/video/lockon.cpp",
	MAME_DIR .. "src/mame/drivers/tatsumi.cpp",
	MAME_DIR .. "src/mame/includes/tatsumi.h",
	MAME_DIR .. "src/mame/machine/tatsumi.cpp",
	MAME_DIR .. "src/mame/video/tatsumi.cpp",
	MAME_DIR .. "src/mame/drivers/tx1.cpp",
	MAME_DIR .. "src/mame/includes/tx1.h",
	MAME_DIR .. "src/mame/machine/tx1.cpp",
	MAME_DIR .. "src/mame/audio/tx1.cpp",
	MAME_DIR .. "src/mame/video/tx1.cpp",
}

createMAMEProjects(_target, _subtarget, "tch")
files {
	MAME_DIR .. "src/mame/drivers/kickgoal.cpp",
	MAME_DIR .. "src/mame/includes/kickgoal.h",
	MAME_DIR .. "src/mame/video/kickgoal.cpp",
	MAME_DIR .. "src/mame/drivers/littlerb.cpp",
	MAME_DIR .. "src/mame/drivers/rltennis.cpp",
	MAME_DIR .. "src/mame/includes/rltennis.h",
	MAME_DIR .. "src/mame/video/rltennis.cpp",
	MAME_DIR .. "src/mame/drivers/speedspn.cpp",
	MAME_DIR .. "src/mame/includes/speedspn.h",
	MAME_DIR .. "src/mame/video/speedspn.cpp",
	MAME_DIR .. "src/mame/drivers/wheelfir.cpp",
}

createMAMEProjects(_target, _subtarget, "tecfri")
files {
	MAME_DIR .. "src/mame/drivers/ambush.cpp",
	MAME_DIR .. "src/mame/includes/ambush.h",
	MAME_DIR .. "src/mame/video/ambush.cpp",
	MAME_DIR .. "src/mame/drivers/holeland.cpp",
	MAME_DIR .. "src/mame/includes/holeland.h",
	MAME_DIR .. "src/mame/video/holeland.cpp",
	MAME_DIR .. "src/mame/drivers/sauro.cpp",
	MAME_DIR .. "src/mame/includes/sauro.h",
	MAME_DIR .. "src/mame/video/sauro.cpp",
	MAME_DIR .. "src/mame/drivers/speedbal.cpp",
	MAME_DIR .. "src/mame/includes/speedbal.h",
	MAME_DIR .. "src/mame/video/speedbal.cpp",
}

createMAMEProjects(_target, _subtarget, "technos")
files {
	MAME_DIR .. "src/mame/drivers/battlane.cpp",
	MAME_DIR .. "src/mame/includes/battlane.h",
	MAME_DIR .. "src/mame/video/battlane.cpp",
	MAME_DIR .. "src/mame/drivers/blockout.cpp",
	MAME_DIR .. "src/mame/includes/blockout.h",
	MAME_DIR .. "src/mame/video/blockout.cpp",
	MAME_DIR .. "src/mame/drivers/bogeyman.cpp",
	MAME_DIR .. "src/mame/includes/bogeyman.h",
	MAME_DIR .. "src/mame/video/bogeyman.cpp",
	MAME_DIR .. "src/mame/drivers/chinagat.cpp",
	MAME_DIR .. "src/mame/drivers/ddragon.cpp",
	MAME_DIR .. "src/mame/includes/ddragon.h",
	MAME_DIR .. "src/mame/video/ddragon.cpp",
	MAME_DIR .. "src/mame/drivers/ddragon3.cpp",
	MAME_DIR .. "src/mame/includes/ddragon3.h",
	MAME_DIR .. "src/mame/video/ddragon3.cpp",
	MAME_DIR .. "src/mame/drivers/dogfgt.cpp",
	MAME_DIR .. "src/mame/includes/dogfgt.h",
	MAME_DIR .. "src/mame/video/dogfgt.cpp",
	MAME_DIR .. "src/mame/drivers/matmania.cpp",
	MAME_DIR .. "src/mame/includes/matmania.h",
	MAME_DIR .. "src/mame/video/matmania.cpp",
	MAME_DIR .. "src/mame/drivers/mystston.cpp",
	MAME_DIR .. "src/mame/includes/mystston.h",
	MAME_DIR .. "src/mame/video/mystston.cpp",
	MAME_DIR .. "src/mame/drivers/renegade.cpp",
	MAME_DIR .. "src/mame/includes/renegade.h",
	MAME_DIR .. "src/mame/video/renegade.cpp",
	MAME_DIR .. "src/mame/drivers/scregg.cpp",
	MAME_DIR .. "src/mame/drivers/shadfrce.cpp",
	MAME_DIR .. "src/mame/includes/shadfrce.h",
	MAME_DIR .. "src/mame/video/shadfrce.cpp",
	MAME_DIR .. "src/mame/drivers/spdodgeb.cpp",
	MAME_DIR .. "src/mame/includes/spdodgeb.h",
	MAME_DIR .. "src/mame/video/spdodgeb.cpp",
	MAME_DIR .. "src/mame/drivers/ssozumo.cpp",
	MAME_DIR .. "src/mame/includes/ssozumo.h",
	MAME_DIR .. "src/mame/video/ssozumo.cpp",
	MAME_DIR .. "src/mame/drivers/tagteam.cpp",
	MAME_DIR .. "src/mame/includes/tagteam.h",
	MAME_DIR .. "src/mame/video/tagteam.cpp",
	MAME_DIR .. "src/mame/drivers/vball.cpp",
	MAME_DIR .. "src/mame/includes/vball.h",
	MAME_DIR .. "src/mame/video/vball.cpp",
	MAME_DIR .. "src/mame/drivers/wwfsstar.cpp",
	MAME_DIR .. "src/mame/includes/wwfsstar.h",
	MAME_DIR .. "src/mame/video/wwfsstar.cpp",
	MAME_DIR .. "src/mame/drivers/xain.cpp",
	MAME_DIR .. "src/mame/includes/xain.h",
	MAME_DIR .. "src/mame/video/xain.cpp",
}

createMAMEProjects(_target, _subtarget, "tehkan")
files {
	MAME_DIR .. "src/mame/video/tecmo_spr.cpp",
	MAME_DIR .. "src/mame/video/tecmo_spr.h",
	MAME_DIR .. "src/mame/video/tecmo_mix.cpp",
	MAME_DIR .. "src/mame/video/tecmo_mix.h",
	MAME_DIR .. "src/mame/drivers/bombjack.cpp",
	MAME_DIR .. "src/mame/includes/bombjack.h",
	MAME_DIR .. "src/mame/video/bombjack.cpp",
	MAME_DIR .. "src/mame/drivers/gaiden.cpp",
	MAME_DIR .. "src/mame/includes/gaiden.h",
	MAME_DIR .. "src/mame/video/gaiden.cpp",
	MAME_DIR .. "src/mame/drivers/lvcards.cpp",
	MAME_DIR .. "src/mame/includes/lvcards.h",
	MAME_DIR .. "src/mame/video/lvcards.cpp",
	MAME_DIR .. "src/mame/drivers/pbaction.cpp",
	MAME_DIR .. "src/mame/includes/pbaction.h",
	MAME_DIR .. "src/mame/video/pbaction.cpp",
	MAME_DIR .. "src/mame/drivers/senjyo.cpp",
	MAME_DIR .. "src/mame/includes/senjyo.h",
	MAME_DIR .. "src/mame/audio/senjyo.cpp",
	MAME_DIR .. "src/mame/video/senjyo.cpp",
	MAME_DIR .. "src/mame/drivers/solomon.cpp",
	MAME_DIR .. "src/mame/includes/solomon.h",
	MAME_DIR .. "src/mame/video/solomon.cpp",
	MAME_DIR .. "src/mame/drivers/spbactn.cpp",
	MAME_DIR .. "src/mame/includes/spbactn.h",
	MAME_DIR .. "src/mame/video/spbactn.cpp",
	MAME_DIR .. "src/mame/drivers/tbowl.cpp",
	MAME_DIR .. "src/mame/includes/tbowl.h",
	MAME_DIR .. "src/mame/video/tbowl.cpp",
	MAME_DIR .. "src/mame/drivers/tecmo.cpp",
	MAME_DIR .. "src/mame/includes/tecmo.h",
	MAME_DIR .. "src/mame/video/tecmo.cpp",
	MAME_DIR .. "src/mame/drivers/tecmo16.cpp",
	MAME_DIR .. "src/mame/includes/tecmo16.h",
	MAME_DIR .. "src/mame/video/tecmo16.cpp",
	MAME_DIR .. "src/mame/drivers/tecmosys.cpp",
	MAME_DIR .. "src/mame/includes/tecmosys.h",
	MAME_DIR .. "src/mame/machine/tecmosys.cpp",
	MAME_DIR .. "src/mame/video/tecmosys.cpp",
	MAME_DIR .. "src/mame/drivers/tehkanwc.cpp",
	MAME_DIR .. "src/mame/includes/tehkanwc.h",
	MAME_DIR .. "src/mame/video/tehkanwc.cpp",
	MAME_DIR .. "src/mame/drivers/wc90.cpp",
	MAME_DIR .. "src/mame/includes/wc90.h",
	MAME_DIR .. "src/mame/video/wc90.cpp",
	MAME_DIR .. "src/mame/drivers/wc90b.cpp",
	MAME_DIR .. "src/mame/includes/wc90b.h",
	MAME_DIR .. "src/mame/video/wc90b.cpp",
}

createMAMEProjects(_target, _subtarget, "thepit")
files {
	MAME_DIR .. "src/mame/drivers/thepit.cpp",
	MAME_DIR .. "src/mame/includes/thepit.h",
	MAME_DIR .. "src/mame/video/thepit.cpp",
	MAME_DIR .. "src/mame/drivers/timelimt.cpp",
	MAME_DIR .. "src/mame/includes/timelimt.h",
	MAME_DIR .. "src/mame/video/timelimt.cpp",
}

createMAMEProjects(_target, _subtarget, "toaplan")
files {
	MAME_DIR .. "src/mame/drivers/mjsister.cpp",
	MAME_DIR .. "src/mame/drivers/slapfght.cpp",
	MAME_DIR .. "src/mame/includes/slapfght.h",
	MAME_DIR .. "src/mame/machine/slapfght.cpp",
	MAME_DIR .. "src/mame/video/slapfght.cpp",
	MAME_DIR .. "src/mame/drivers/snowbros.cpp",
	MAME_DIR .. "src/mame/includes/snowbros.h",
	MAME_DIR .. "src/mame/video/kan_pand.cpp",
	MAME_DIR .. "src/mame/video/kan_pand.h",
	MAME_DIR .. "src/mame/video/kan_panb.cpp",
	MAME_DIR .. "src/mame/drivers/toaplan1.cpp",
	MAME_DIR .. "src/mame/includes/toaplan1.h",
	MAME_DIR .. "src/mame/machine/toaplan1.cpp",
	MAME_DIR .. "src/mame/video/toaplan1.cpp",
	MAME_DIR .. "src/mame/includes/toaplipt.h",
	MAME_DIR .. "src/mame/drivers/toaplan2.cpp",
	MAME_DIR .. "src/mame/includes/toaplan2.h",
	MAME_DIR .. "src/mame/video/toaplan2.cpp",
	MAME_DIR .. "src/mame/video/gp9001.cpp",
	MAME_DIR .. "src/mame/video/gp9001.h",
	MAME_DIR .. "src/mame/drivers/twincobr.cpp",
	MAME_DIR .. "src/mame/includes/twincobr.h",
	MAME_DIR .. "src/mame/machine/twincobr.cpp",
	MAME_DIR .. "src/mame/video/twincobr.cpp",
	MAME_DIR .. "src/mame/drivers/wardner.cpp",
	MAME_DIR .. "src/mame/video/toaplan_scu.cpp",
	MAME_DIR .. "src/mame/video/toaplan_scu.h",
}

createMAMEProjects(_target, _subtarget, "tong")
files {
	MAME_DIR .. "src/mame/drivers/beezer.cpp",
	MAME_DIR .. "src/mame/includes/beezer.h",
	MAME_DIR .. "src/mame/machine/beezer.cpp",
	MAME_DIR .. "src/mame/video/beezer.cpp",
	MAME_DIR .. "src/mame/audio/beezer.cpp",
}

createMAMEProjects(_target, _subtarget, "unico")
files {
	MAME_DIR .. "src/mame/drivers/drgnmst.cpp",
	MAME_DIR .. "src/mame/includes/drgnmst.h",
	MAME_DIR .. "src/mame/video/drgnmst.cpp",
	MAME_DIR .. "src/mame/drivers/silkroad.cpp",
	MAME_DIR .. "src/mame/includes/silkroad.h",
	MAME_DIR .. "src/mame/video/silkroad.cpp",
	MAME_DIR .. "src/mame/drivers/unico.cpp",
	MAME_DIR .. "src/mame/includes/unico.h",
	MAME_DIR .. "src/mame/video/unico.cpp",
}

createMAMEProjects(_target, _subtarget, "univers")
files {
	MAME_DIR .. "src/mame/drivers/cheekyms.cpp",
	MAME_DIR .. "src/mame/includes/cheekyms.h",
	MAME_DIR .. "src/mame/video/cheekyms.cpp",
	MAME_DIR .. "src/mame/drivers/cosmic.cpp",
	MAME_DIR .. "src/mame/includes/cosmic.h",
	MAME_DIR .. "src/mame/video/cosmic.cpp",
	MAME_DIR .. "src/mame/drivers/docastle.cpp",
	MAME_DIR .. "src/mame/includes/docastle.h",
	MAME_DIR .. "src/mame/machine/docastle.cpp",
	MAME_DIR .. "src/mame/video/docastle.cpp",
	MAME_DIR .. "src/mame/drivers/ladybug.cpp",
	MAME_DIR .. "src/mame/includes/ladybug.h",
	MAME_DIR .. "src/mame/video/ladybug.cpp",
	MAME_DIR .. "src/mame/drivers/mrdo.cpp",
	MAME_DIR .. "src/mame/includes/mrdo.h",
	MAME_DIR .. "src/mame/video/mrdo.cpp",
	MAME_DIR .. "src/mame/drivers/redclash.cpp",
	MAME_DIR .. "src/mame/video/redclash.cpp",
	MAME_DIR .. "src/mame/drivers/superdq.cpp",
}

createMAMEProjects(_target, _subtarget, "upl")
files {
	MAME_DIR .. "src/mame/drivers/mouser.cpp",
	MAME_DIR .. "src/mame/includes/mouser.h",
	MAME_DIR .. "src/mame/video/mouser.cpp",
	MAME_DIR .. "src/mame/drivers/ninjakd2.cpp",
	MAME_DIR .. "src/mame/includes/ninjakd2.h",
	MAME_DIR .. "src/mame/video/ninjakd2.cpp",
	MAME_DIR .. "src/mame/drivers/nova2001.cpp",
	MAME_DIR .. "src/mame/includes/nova2001.h",
	MAME_DIR .. "src/mame/video/nova2001.cpp",
	MAME_DIR .. "src/mame/drivers/xxmissio.cpp",
	MAME_DIR .. "src/mame/includes/xxmissio.h",
	MAME_DIR .. "src/mame/video/xxmissio.cpp",
}

createMAMEProjects(_target, _subtarget, "valadon")
files {
	MAME_DIR .. "src/mame/drivers/bagman.cpp",
	MAME_DIR .. "src/mame/includes/bagman.h",
	MAME_DIR .. "src/mame/machine/bagman.cpp",
	MAME_DIR .. "src/mame/video/bagman.cpp",
	MAME_DIR .. "src/mame/drivers/tankbust.cpp",
	MAME_DIR .. "src/mame/includes/tankbust.h",
	MAME_DIR .. "src/mame/video/tankbust.cpp",
}

createMAMEProjects(_target, _subtarget, "veltmjr")
files {
	MAME_DIR .. "src/mame/drivers/cardline.cpp",
	MAME_DIR .. "src/mame/drivers/witch.cpp",
}

createMAMEProjects(_target, _subtarget, "venture")
files {
	MAME_DIR .. "src/mame/drivers/looping.cpp",
	MAME_DIR .. "src/mame/drivers/spcforce.cpp",
	MAME_DIR .. "src/mame/includes/spcforce.h",
	MAME_DIR .. "src/mame/video/spcforce.cpp",
	MAME_DIR .. "src/mame/drivers/suprridr.cpp",
	MAME_DIR .. "src/mame/includes/suprridr.h",
	MAME_DIR .. "src/mame/video/suprridr.cpp",
}

createMAMEProjects(_target, _subtarget, "vsystem")
files {
	MAME_DIR .. "src/mame/video/vsystem_spr.cpp",
	MAME_DIR .. "src/mame/video/vsystem_spr.h",
	MAME_DIR .. "src/mame/video/vsystem_spr2.cpp",
	MAME_DIR .. "src/mame/video/vsystem_spr2.h",
	MAME_DIR .. "src/mame/drivers/aerofgt.cpp",
	MAME_DIR .. "src/mame/includes/aerofgt.h",
	MAME_DIR .. "src/mame/video/aerofgt.cpp",
	MAME_DIR .. "src/mame/drivers/crshrace.cpp",
	MAME_DIR .. "src/mame/includes/crshrace.h",
	MAME_DIR .. "src/mame/video/crshrace.cpp",
	MAME_DIR .. "src/mame/drivers/f1gp.cpp",
	MAME_DIR .. "src/mame/includes/f1gp.h",
	MAME_DIR .. "src/mame/video/f1gp.cpp",
	MAME_DIR .. "src/mame/drivers/fromance.cpp",
	MAME_DIR .. "src/mame/includes/fromance.h",
	MAME_DIR .. "src/mame/video/fromance.cpp",
	MAME_DIR .. "src/mame/drivers/fromanc2.cpp",
	MAME_DIR .. "src/mame/includes/fromanc2.h",
	MAME_DIR .. "src/mame/video/fromanc2.cpp",
	MAME_DIR .. "src/mame/drivers/gstriker.cpp",
	MAME_DIR .. "src/mame/includes/gstriker.h",
	MAME_DIR .. "src/mame/video/gstriker.cpp",
	MAME_DIR .. "src/mame/video/mb60553.cpp",
	MAME_DIR .. "src/mame/video/mb60553.h",
	MAME_DIR .. "src/mame/video/vs920a.cpp",
	MAME_DIR .. "src/mame/video/vs920a.h",
	MAME_DIR .. "src/mame/drivers/inufuku.cpp",
	MAME_DIR .. "src/mame/includes/inufuku.h",
	MAME_DIR .. "src/mame/video/inufuku.cpp",
	MAME_DIR .. "src/mame/drivers/ojankohs.cpp",
	MAME_DIR .. "src/mame/includes/ojankohs.h",
	MAME_DIR .. "src/mame/video/ojankohs.cpp",
	MAME_DIR .. "src/mame/drivers/pipedrm.cpp",
	MAME_DIR .. "src/mame/drivers/rpunch.cpp",
	MAME_DIR .. "src/mame/includes/rpunch.h",
	MAME_DIR .. "src/mame/video/rpunch.cpp",
	MAME_DIR .. "src/mame/drivers/suprslam.cpp",
	MAME_DIR .. "src/mame/includes/suprslam.h",
	MAME_DIR .. "src/mame/video/suprslam.cpp",
	MAME_DIR .. "src/mame/drivers/tail2nos.cpp",
	MAME_DIR .. "src/mame/includes/tail2nos.h",
	MAME_DIR .. "src/mame/video/tail2nos.cpp",
	MAME_DIR .. "src/mame/drivers/taotaido.cpp",
	MAME_DIR .. "src/mame/includes/taotaido.h",
	MAME_DIR .. "src/mame/video/taotaido.cpp",
	MAME_DIR .. "src/mame/drivers/welltris.cpp",
	MAME_DIR .. "src/mame/includes/welltris.h",
	MAME_DIR .. "src/mame/video/welltris.cpp",
}

createMAMEProjects(_target, _subtarget, "yunsung")
files {
	MAME_DIR .. "src/mame/drivers/nmg5.cpp",
	MAME_DIR .. "src/mame/drivers/paradise.cpp",
	MAME_DIR .. "src/mame/includes/paradise.h",
	MAME_DIR .. "src/mame/video/paradise.cpp",
	MAME_DIR .. "src/mame/drivers/yunsung8.cpp",
	MAME_DIR .. "src/mame/includes/yunsung8.h",
	MAME_DIR .. "src/mame/video/yunsung8.cpp",
	MAME_DIR .. "src/mame/drivers/yunsun16.cpp",
	MAME_DIR .. "src/mame/includes/yunsun16.h",
	MAME_DIR .. "src/mame/video/yunsun16.cpp",
}

createMAMEProjects(_target, _subtarget, "zaccaria")
files {
	MAME_DIR .. "src/mame/drivers/laserbat.cpp",
	MAME_DIR .. "src/mame/includes/laserbat.h",
	MAME_DIR .. "src/mame/video/laserbat.cpp",
	MAME_DIR .. "src/mame/audio/laserbat.cpp",
	MAME_DIR .. "src/mame/drivers/seabattl.cpp",
	MAME_DIR .. "src/mame/drivers/zac2650.cpp",
	MAME_DIR .. "src/mame/includes/zac2650.h",
	MAME_DIR .. "src/mame/video/zac2650.cpp",
	MAME_DIR .. "src/mame/drivers/zaccaria.cpp",
	MAME_DIR .. "src/mame/includes/zaccaria.h",
	MAME_DIR .. "src/mame/video/zaccaria.cpp",
}

--------------------------------------------------
-- pinball drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "pinball")
files {
	MAME_DIR .. "src/mame/drivers/allied.cpp",
	MAME_DIR .. "src/mame/drivers/alvg.cpp",
	MAME_DIR .. "src/mame/drivers/atari_s1.cpp",
	MAME_DIR .. "src/mame/drivers/atari_s2.cpp",
	MAME_DIR .. "src/mame/drivers/bingo.cpp",
	MAME_DIR .. "src/mame/drivers/by17.cpp",
	MAME_DIR .. "src/mame/drivers/by35.cpp",
	MAME_DIR .. "src/mame/drivers/by6803.cpp",
	MAME_DIR .. "src/mame/drivers/by68701.cpp",
	MAME_DIR .. "src/mame/drivers/byvid.cpp",
	MAME_DIR .. "src/mame/drivers/capcom.cpp",
	MAME_DIR .. "src/mame/drivers/de_2.cpp",
	MAME_DIR .. "src/mame/drivers/de_3.cpp",
	MAME_DIR .. "src/mame/machine/decopincpu.cpp",
	MAME_DIR .. "src/mame/machine/decopincpu.h",
	MAME_DIR .. "src/mame/video/decodmd1.cpp",
	MAME_DIR .. "src/mame/video/decodmd1.h",
	MAME_DIR .. "src/mame/video/decodmd2.cpp",
	MAME_DIR .. "src/mame/video/decodmd2.h",
	MAME_DIR .. "src/mame/video/decodmd3.cpp",
	MAME_DIR .. "src/mame/video/decodmd3.h",
	MAME_DIR .. "src/mame/drivers/de_3b.cpp",
	MAME_DIR .. "src/mame/drivers/flicker.cpp",
	MAME_DIR .. "src/mame/drivers/g627.cpp",
	MAME_DIR .. "src/mame/drivers/gp_1.cpp",
	MAME_DIR .. "src/mame/machine/genpin.cpp",
	MAME_DIR .. "src/mame/machine/genpin.h",
	MAME_DIR .. "src/mame/drivers/gp_2.cpp",
	MAME_DIR .. "src/mame/drivers/gts1.cpp",
	MAME_DIR .. "src/mame/drivers/gts3.cpp",
	MAME_DIR .. "src/mame/drivers/gts3a.cpp",
	MAME_DIR .. "src/mame/drivers/gts80.cpp",
	MAME_DIR .. "src/mame/drivers/gts80a.cpp",
	MAME_DIR .. "src/mame/drivers/gts80b.cpp",
	MAME_DIR .. "src/mame/drivers/hankin.cpp",
	MAME_DIR .. "src/mame/drivers/icecold.cpp",
	MAME_DIR .. "src/mame/drivers/inder.cpp",
	MAME_DIR .. "src/mame/drivers/jeutel.cpp",
	MAME_DIR .. "src/mame/drivers/jp.cpp",
	MAME_DIR .. "src/mame/drivers/jvh.cpp",
	MAME_DIR .. "src/mame/drivers/kissproto.cpp",
	MAME_DIR .. "src/mame/drivers/ltd.cpp",
	MAME_DIR .. "src/mame/drivers/micropin.cpp",
	MAME_DIR .. "src/mame/drivers/mephistp.cpp",
	MAME_DIR .. "src/mame/drivers/mrgame.cpp",
	MAME_DIR .. "src/mame/drivers/nsm.cpp",
	MAME_DIR .. "src/mame/drivers/peyper.cpp",
	MAME_DIR .. "src/mame/drivers/play_1.cpp",
	MAME_DIR .. "src/mame/drivers/play_2.cpp",
	MAME_DIR .. "src/mame/drivers/play_3.cpp",
	MAME_DIR .. "src/mame/drivers/play_5.cpp",
	MAME_DIR .. "src/mame/drivers/rowamet.cpp",
	MAME_DIR .. "src/mame/drivers/s11.cpp",
	MAME_DIR .. "src/mame/includes/s11.h",
	MAME_DIR .. "src/mame/drivers/s11a.cpp",
	MAME_DIR .. "src/mame/drivers/s11b.cpp",
	MAME_DIR .. "src/mame/drivers/s11c.cpp",
	MAME_DIR .. "src/mame/audio/s11c_bg.cpp",
	MAME_DIR .. "src/mame/audio/s11c_bg.h",
	MAME_DIR .. "src/mame/drivers/s3.cpp",
	MAME_DIR .. "src/mame/drivers/s4.cpp",
	MAME_DIR .. "src/mame/drivers/s6.cpp",
	MAME_DIR .. "src/mame/drivers/s6a.cpp",
	MAME_DIR .. "src/mame/drivers/s7.cpp",
	MAME_DIR .. "src/mame/drivers/s8.cpp",
	MAME_DIR .. "src/mame/drivers/s8a.cpp",
	MAME_DIR .. "src/mame/drivers/s9.cpp",
	MAME_DIR .. "src/mame/drivers/sam.cpp",
	MAME_DIR .. "src/mame/drivers/sleic.cpp",
	MAME_DIR .. "src/mame/drivers/spectra.cpp",
	MAME_DIR .. "src/mame/drivers/spinb.cpp",
	MAME_DIR .. "src/mame/drivers/st_mp100.cpp",
	MAME_DIR .. "src/mame/drivers/st_mp200.cpp",
	MAME_DIR .. "src/mame/drivers/taito.cpp",
	MAME_DIR .. "src/mame/drivers/techno.cpp",
	MAME_DIR .. "src/mame/drivers/vd.cpp",
	MAME_DIR .. "src/mame/drivers/whitestar.cpp",
	MAME_DIR .. "src/mame/drivers/white_mod.cpp",
	MAME_DIR .. "src/mame/drivers/wico.cpp",
	MAME_DIR .. "src/mame/drivers/wpc_95.cpp",
	MAME_DIR .. "src/mame/drivers/wpc_an.cpp",
	MAME_DIR .. "src/mame/drivers/wpc_dcs.cpp",
	MAME_DIR .. "src/mame/drivers/wpc_dot.cpp",
	MAME_DIR .. "src/mame/drivers/wpc_flip1.cpp",
	MAME_DIR .. "src/mame/drivers/wpc_flip2.cpp",
	MAME_DIR .. "src/mame/drivers/wpc_s.cpp",
	MAME_DIR .. "src/mame/machine/wpc.cpp",
	MAME_DIR .. "src/mame/machine/wpc.h",
	MAME_DIR .. "src/mame/includes/wpc_pin.h",
	MAME_DIR .. "src/mame/audio/wpcsnd.cpp",
	MAME_DIR .. "src/mame/audio/wpcsnd.h",
	MAME_DIR .. "src/mame/video/wpc_dmd.cpp",
	MAME_DIR .. "src/mame/video/wpc_dmd.h",
	MAME_DIR .. "src/mame/machine/wpc_pic.cpp",
	MAME_DIR .. "src/mame/machine/wpc_pic.h",
	MAME_DIR .. "src/mame/machine/wpc_lamp.cpp",
	MAME_DIR .. "src/mame/machine/wpc_lamp.h",
	MAME_DIR .. "src/mame/machine/wpc_out.cpp",
	MAME_DIR .. "src/mame/machine/wpc_out.h",
	MAME_DIR .. "src/mame/machine/wpc_shift.cpp",
	MAME_DIR .. "src/mame/machine/wpc_shift.h",
	MAME_DIR .. "src/mame/drivers/zac_1.cpp",
	MAME_DIR .. "src/mame/drivers/zac_2.cpp",
	MAME_DIR .. "src/mame/drivers/zac_proto.cpp",
}

--------------------------------------------------
-- remaining drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "misc")
files {
	MAME_DIR .. "src/mame/drivers/1945kiii.cpp",
	MAME_DIR .. "src/mame/drivers/39in1.cpp",
	MAME_DIR .. "src/mame/machine/pxa255.h",
	MAME_DIR .. "src/mame/drivers/3do.cpp",
	MAME_DIR .. "src/mame/includes/3do.h",
	MAME_DIR .. "src/mame/machine/3do.cpp",
	MAME_DIR .. "src/mame/drivers/3x3puzzl.cpp",
	MAME_DIR .. "src/mame/drivers/4enraya.cpp",
	MAME_DIR .. "src/mame/includes/4enraya.h",
	MAME_DIR .. "src/mame/video/4enraya.cpp",
	MAME_DIR .. "src/mame/drivers/4enlinea.cpp",
	MAME_DIR .. "src/mame/drivers/5clown.cpp",
	MAME_DIR .. "src/mame/drivers/a1supply.cpp",
	MAME_DIR .. "src/mame/drivers/acefruit.cpp",
	MAME_DIR .. "src/mame/drivers/aces1.cpp",
	MAME_DIR .. "src/mame/drivers/acesp.cpp",
	MAME_DIR .. "src/mame/drivers/adp.cpp",
	MAME_DIR .. "src/mame/drivers/age_candy.cpp",
	MAME_DIR .. "src/mame/drivers/alinvade.cpp",
	MAME_DIR .. "src/mame/drivers/amaticmg.cpp",
	MAME_DIR .. "src/mame/drivers/amerihok.cpp",
	MAME_DIR .. "src/mame/drivers/ampoker2.cpp",
	MAME_DIR .. "src/mame/includes/ampoker2.h",
	MAME_DIR .. "src/mame/video/ampoker2.cpp",
	MAME_DIR .. "src/mame/drivers/amspdwy.cpp",
	MAME_DIR .. "src/mame/includes/amspdwy.h",
	MAME_DIR .. "src/mame/video/amspdwy.cpp",
	MAME_DIR .. "src/mame/drivers/amusco.cpp",
	MAME_DIR .. "src/mame/drivers/arachnid.cpp",
	MAME_DIR .. "src/mame/drivers/artmagic.cpp",
	MAME_DIR .. "src/mame/includes/artmagic.h",
	MAME_DIR .. "src/mame/video/artmagic.cpp",
	MAME_DIR .. "src/mame/drivers/astrafr.cpp",
	MAME_DIR .. "src/mame/drivers/astrcorp.cpp",
	MAME_DIR .. "src/mame/drivers/astropc.cpp",
	MAME_DIR .. "src/mame/drivers/atronic.cpp",
	MAME_DIR .. "src/mame/drivers/attckufo.cpp",
	MAME_DIR .. "src/mame/drivers/avt.cpp",
	MAME_DIR .. "src/mame/drivers/aztarac.cpp",
	MAME_DIR .. "src/mame/includes/aztarac.h",
	MAME_DIR .. "src/mame/audio/aztarac.cpp",
	MAME_DIR .. "src/mame/video/aztarac.cpp",
	MAME_DIR .. "src/mame/drivers/bailey.cpp",
	MAME_DIR .. "src/mame/drivers/beaminv.cpp",
	MAME_DIR .. "src/mame/drivers/belatra.cpp",
	MAME_DIR .. "src/mame/drivers/bgt.cpp",
	MAME_DIR .. "src/mame/drivers/bingoman.cpp",
	MAME_DIR .. "src/mame/drivers/bingor.cpp",
	MAME_DIR .. "src/mame/drivers/blitz.cpp",
	MAME_DIR .. "src/mame/drivers/blitz68k.cpp",
	MAME_DIR .. "src/mame/drivers/buster.cpp",
	MAME_DIR .. "src/mame/drivers/calomega.cpp",
	MAME_DIR .. "src/mame/includes/calomega.h",
	MAME_DIR .. "src/mame/video/calomega.cpp",
	MAME_DIR .. "src/mame/drivers/carrera.cpp",
	MAME_DIR .. "src/mame/drivers/castle.cpp",
	MAME_DIR .. "src/mame/drivers/cave.cpp",
	MAME_DIR .. "src/mame/includes/cave.h",
	MAME_DIR .. "src/mame/video/cave.cpp",
	MAME_DIR .. "src/mame/drivers/cavepc.cpp",
	MAME_DIR .. "src/mame/drivers/cv1k.cpp",
	MAME_DIR .. "src/mame/drivers/cb2001.cpp",
	MAME_DIR .. "src/mame/drivers/cdi.cpp",
	MAME_DIR .. "src/mame/includes/cdi.h",
	MAME_DIR .. "src/mame/video/mcd212.cpp",
	MAME_DIR .. "src/mame/video/mcd212.h",
	MAME_DIR .. "src/mame/machine/cdi070.cpp",
	MAME_DIR .. "src/mame/machine/cdi070.h",
	MAME_DIR .. "src/mame/machine/cdislave.cpp",
	MAME_DIR .. "src/mame/machine/cdislave.h",
	MAME_DIR .. "src/mame/machine/cdicdic.cpp",
	MAME_DIR .. "src/mame/machine/cdicdic.h",
	MAME_DIR .. "src/mame/drivers/cesclass.cpp",
	MAME_DIR .. "src/mame/drivers/chance32.cpp",
	MAME_DIR .. "src/mame/drivers/chexx.cpp",
	MAME_DIR .. "src/mame/drivers/chicago.cpp",
	MAME_DIR .. "src/mame/drivers/chsuper.cpp",
	MAME_DIR .. "src/mame/drivers/cidelsa.cpp",
	MAME_DIR .. "src/mame/includes/cidelsa.h",
	MAME_DIR .. "src/mame/video/cidelsa.cpp",
	MAME_DIR .. "src/mame/drivers/clowndwn.cpp",
	MAME_DIR .. "src/mame/drivers/cocoloco.cpp",
	MAME_DIR .. "src/mame/drivers/coinmstr.cpp",
	MAME_DIR .. "src/mame/drivers/coinmvga.cpp",
	MAME_DIR .. "src/mame/drivers/comebaby.cpp",
	MAME_DIR .. "src/mame/drivers/cupidon.cpp",
	MAME_DIR .. "src/mame/drivers/bntyhunt.cpp",
	MAME_DIR .. "src/mame/drivers/coolpool.cpp",
	MAME_DIR .. "src/mame/includes/coolpool.h",
	MAME_DIR .. "src/mame/drivers/megaphx.cpp",
	MAME_DIR .. "src/mame/machine/inder_sb.cpp",
	MAME_DIR .. "src/mame/machine/inder_sb.h",
	MAME_DIR .. "src/mame/machine/inder_vid.cpp",
	MAME_DIR .. "src/mame/machine/inder_vid.h",
	MAME_DIR .. "src/mame/drivers/corona.cpp",
	MAME_DIR .. "src/mame/drivers/crystal.cpp",
	MAME_DIR .. "src/mame/video/vrender0.cpp",
	MAME_DIR .. "src/mame/video/vrender0.h",
	MAME_DIR .. "src/mame/drivers/cubeqst.cpp",
	MAME_DIR .. "src/mame/drivers/cybertnk.cpp",
	MAME_DIR .. "src/mame/drivers/dcheese.cpp",
	MAME_DIR .. "src/mame/includes/dcheese.h",
	MAME_DIR .. "src/mame/video/dcheese.cpp",
	MAME_DIR .. "src/mame/drivers/dfruit.cpp",
	MAME_DIR .. "src/mame/drivers/dgpix.cpp",
	MAME_DIR .. "src/mame/drivers/discoboy.cpp",
	MAME_DIR .. "src/mame/drivers/dominob.cpp",
	MAME_DIR .. "src/mame/drivers/dorachan.cpp",
	MAME_DIR .. "src/mame/drivers/dreamwld.cpp",
	MAME_DIR .. "src/mame/drivers/dribling.cpp",
	MAME_DIR .. "src/mame/includes/dribling.h",
	MAME_DIR .. "src/mame/video/dribling.cpp",
	MAME_DIR .. "src/mame/drivers/drw80pkr.cpp",
	MAME_DIR .. "src/mame/drivers/dwarfd.cpp",
	MAME_DIR .. "src/mame/drivers/dynadice.cpp",
	MAME_DIR .. "src/mame/drivers/ecoinfr.cpp",
	MAME_DIR .. "src/mame/drivers/ecoinf1.cpp",
	MAME_DIR .. "src/mame/drivers/ecoinf2.cpp",
	MAME_DIR .. "src/mame/drivers/ecoinf3.cpp",
	MAME_DIR .. "src/mame/drivers/electra.cpp",
	MAME_DIR .. "src/mame/drivers/epos.cpp",
	MAME_DIR .. "src/mame/includes/epos.h",
	MAME_DIR .. "src/mame/video/epos.cpp",
	MAME_DIR .. "src/mame/drivers/esd16.cpp",
	MAME_DIR .. "src/mame/includes/esd16.h",
	MAME_DIR .. "src/mame/video/esd16.cpp",
	MAME_DIR .. "src/mame/drivers/esh.cpp",
	MAME_DIR .. "src/mame/drivers/esripsys.cpp",
	MAME_DIR .. "src/mame/includes/esripsys.h",
	MAME_DIR .. "src/mame/video/esripsys.cpp",
	MAME_DIR .. "src/mame/drivers/ettrivia.cpp",
	MAME_DIR .. "src/mame/drivers/extrema.cpp",
	MAME_DIR .. "src/mame/drivers/fastinvaders.cpp",
	MAME_DIR .. "src/mame/drivers/fireball.cpp",
	MAME_DIR .. "src/mame/drivers/flipjack.cpp",
	MAME_DIR .. "src/mame/drivers/flower.cpp",
	MAME_DIR .. "src/mame/includes/flower.h",
	MAME_DIR .. "src/mame/audio/flower.cpp",
	MAME_DIR .. "src/mame/video/flower.cpp",
	MAME_DIR .. "src/mame/drivers/fortecar.cpp",
	MAME_DIR .. "src/mame/drivers/fresh.cpp",
	MAME_DIR .. "src/mame/drivers/freekick.cpp",
	MAME_DIR .. "src/mame/includes/freekick.h",
	MAME_DIR .. "src/mame/video/freekick.cpp",
	MAME_DIR .. "src/mame/drivers/fungames.cpp",
	MAME_DIR .. "src/mame/drivers/funkball.cpp",
	MAME_DIR .. "src/mame/drivers/gambl186.cpp",
	MAME_DIR .. "src/mame/drivers/galaxi.cpp",
	MAME_DIR .. "src/mame/drivers/galgame.cpp",
	MAME_DIR .. "src/mame/drivers/gamecstl.cpp",
	MAME_DIR .. "src/mame/drivers/gammagic.cpp",
	MAME_DIR .. "src/mame/drivers/gamtor.cpp",
	MAME_DIR .. "src/mame/drivers/gei.cpp",
	MAME_DIR .. "src/mame/drivers/gkigt.cpp",
	MAME_DIR .. "src/mame/drivers/globalfr.cpp",
	MAME_DIR .. "src/mame/drivers/globalvr.cpp",
	MAME_DIR .. "src/mame/drivers/gluck2.cpp",
	MAME_DIR .. "src/mame/drivers/goldngam.cpp",
	MAME_DIR .. "src/mame/drivers/goldnpkr.cpp",
	MAME_DIR .. "src/mame/drivers/good.cpp",
	MAME_DIR .. "src/mame/drivers/gotcha.cpp",
	MAME_DIR .. "src/mame/includes/gotcha.h",
	MAME_DIR .. "src/mame/video/gotcha.cpp",
	MAME_DIR .. "src/mame/drivers/gstream.cpp",
	MAME_DIR .. "src/mame/drivers/gumbo.cpp",
	MAME_DIR .. "src/mame/includes/gumbo.h",
	MAME_DIR .. "src/mame/video/gumbo.cpp",
	MAME_DIR .. "src/mame/drivers/gunpey.cpp",
	MAME_DIR .. "src/mame/drivers/hideseek.cpp",
	MAME_DIR .. "src/mame/drivers/hazelgr.cpp",
	MAME_DIR .. "src/mame/drivers/headonb.cpp",
	MAME_DIR .. "src/mame/drivers/highvdeo.cpp",
	MAME_DIR .. "src/mame/drivers/himesiki.cpp",
	MAME_DIR .. "src/mame/includes/himesiki.h",
	MAME_DIR .. "src/mame/video/himesiki.cpp",
	MAME_DIR .. "src/mame/drivers/hitpoker.cpp",
	MAME_DIR .. "src/mame/drivers/homedata.cpp",
	MAME_DIR .. "src/mame/includes/homedata.h",
	MAME_DIR .. "src/mame/video/homedata.cpp",
	MAME_DIR .. "src/mame/drivers/hotblock.cpp",
	MAME_DIR .. "src/mame/drivers/hotstuff.cpp",
	MAME_DIR .. "src/mame/drivers/ichiban.cpp",
	MAME_DIR .. "src/mame/drivers/imolagp.cpp",
	MAME_DIR .. "src/mame/drivers/intrscti.cpp",
	MAME_DIR .. "src/mame/drivers/istellar.cpp",
	MAME_DIR .. "src/mame/drivers/itgambl2.cpp",
	MAME_DIR .. "src/mame/drivers/itgambl3.cpp",
	MAME_DIR .. "src/mame/drivers/itgamble.cpp",
	MAME_DIR .. "src/mame/drivers/jackpool.cpp",
	MAME_DIR .. "src/mame/drivers/jankenmn.cpp",
	MAME_DIR .. "src/mame/drivers/jokrwild.cpp",
	MAME_DIR .. "src/mame/drivers/jongkyo.cpp",
	MAME_DIR .. "src/mame/drivers/joystand.cpp",
	MAME_DIR .. "src/mame/drivers/jubilee.cpp",
	MAME_DIR .. "src/mame/drivers/kas89.cpp",
	MAME_DIR .. "src/mame/drivers/kingpin.cpp",
	MAME_DIR .. "src/mame/drivers/koikoi.cpp",
	MAME_DIR .. "src/mame/drivers/kurukuru.cpp",
	MAME_DIR .. "src/mame/drivers/kyugo.cpp",
	MAME_DIR .. "src/mame/includes/kyugo.h",
	MAME_DIR .. "src/mame/video/kyugo.cpp",
	MAME_DIR .. "src/mame/drivers/ladyfrog.cpp",
	MAME_DIR .. "src/mame/includes/ladyfrog.h",
	MAME_DIR .. "src/mame/video/ladyfrog.cpp",
	MAME_DIR .. "src/mame/drivers/laserbas.cpp",
	MAME_DIR .. "src/mame/drivers/laz_awetoss.cpp",
	MAME_DIR .. "src/mame/drivers/laz_aftrshok.cpp",
	MAME_DIR .. "src/mame/drivers/laz_ribrac.cpp",
	MAME_DIR .. "src/mame/drivers/lethalj.cpp",
	MAME_DIR .. "src/mame/includes/lethalj.h",
	MAME_DIR .. "src/mame/video/lethalj.cpp",
	MAME_DIR .. "src/mame/drivers/limenko.cpp",
	MAME_DIR .. "src/mame/drivers/ltcasino.cpp",
	MAME_DIR .. "src/mame/drivers/lucky74.cpp",
	MAME_DIR .. "src/mame/includes/lucky74.h",
	MAME_DIR .. "src/mame/video/lucky74.cpp",
	MAME_DIR .. "src/mame/drivers/luckgrln.cpp",
	MAME_DIR .. "src/mame/drivers/magic10.cpp",
	MAME_DIR .. "src/mame/drivers/magicard.cpp",
	MAME_DIR .. "src/mame/drivers/magicfly.cpp",
	MAME_DIR .. "src/mame/drivers/magictg.cpp",
	MAME_DIR .. "src/mame/drivers/magtouch.cpp",
	MAME_DIR .. "src/mame/drivers/majorpkr.cpp",
	MAME_DIR .. "src/mame/drivers/malzak.cpp",
	MAME_DIR .. "src/mame/includes/malzak.h",
	MAME_DIR .. "src/mame/video/malzak.cpp",
	MAME_DIR .. "src/mame/drivers/manohman.cpp",
	MAME_DIR .. "src/mame/drivers/mcatadv.cpp",
	MAME_DIR .. "src/mame/includes/mcatadv.h",
	MAME_DIR .. "src/mame/video/mcatadv.cpp",
	MAME_DIR .. "src/mame/drivers/mgavegas.cpp",
	MAME_DIR .. "src/mame/drivers/meyc8080.cpp",
	MAME_DIR .. "src/mame/drivers/meyc8088.cpp",
	MAME_DIR .. "src/mame/drivers/micro3d.cpp",
	MAME_DIR .. "src/mame/includes/micro3d.h",
	MAME_DIR .. "src/mame/machine/micro3d.cpp",
	MAME_DIR .. "src/mame/video/micro3d.cpp",
	MAME_DIR .. "src/mame/audio/micro3d.cpp",
	MAME_DIR .. "src/mame/drivers/midas.cpp",
	MAME_DIR .. "src/mame/drivers/miniboy7.cpp",
	MAME_DIR .. "src/mame/drivers/mirax.cpp",
	MAME_DIR .. "src/mame/drivers/mole.cpp",
	MAME_DIR .. "src/mame/drivers/mosaic.cpp",
	MAME_DIR .. "src/mame/includes/mosaic.h",
	MAME_DIR .. "src/mame/video/mosaic.cpp",
	MAME_DIR .. "src/mame/drivers/mpu12wbk.cpp",
	MAME_DIR .. "src/mame/drivers/mrjong.cpp",
	MAME_DIR .. "src/mame/includes/mrjong.h",
	MAME_DIR .. "src/mame/video/mrjong.cpp",
	MAME_DIR .. "src/mame/drivers/multfish.cpp",
	MAME_DIR .. "src/mame/includes/multfish.h",
	MAME_DIR .. "src/mame/drivers/multfish_boot.cpp",
	MAME_DIR .. "src/mame/drivers/multfish_ref.cpp",
	MAME_DIR .. "src/mame/drivers/murogem.cpp",
	MAME_DIR .. "src/mame/drivers/murogmbl.cpp",
	MAME_DIR .. "src/mame/drivers/neoprint.cpp",
	MAME_DIR .. "src/mame/drivers/neptunp2.cpp",
	MAME_DIR .. "src/mame/drivers/news.cpp",
	MAME_DIR .. "src/mame/includes/news.h",
	MAME_DIR .. "src/mame/video/news.cpp",
	MAME_DIR .. "src/mame/drivers/nexus3d.cpp",
	MAME_DIR .. "src/mame/drivers/norautp.cpp",
	MAME_DIR .. "src/mame/includes/norautp.h",
	MAME_DIR .. "src/mame/audio/norautp.cpp",
	MAME_DIR .. "src/mame/drivers/nsmpoker.cpp",
	MAME_DIR .. "src/mame/drivers/oneshot.cpp",
	MAME_DIR .. "src/mame/includes/oneshot.h",
	MAME_DIR .. "src/mame/video/oneshot.cpp",
	MAME_DIR .. "src/mame/drivers/onetwo.cpp",
	MAME_DIR .. "src/mame/drivers/othello.cpp",
	MAME_DIR .. "src/mame/drivers/pachifev.cpp",
	MAME_DIR .. "src/mame/drivers/pasha2.cpp",
	MAME_DIR .. "src/mame/drivers/pass.cpp",
	MAME_DIR .. "src/mame/includes/pass.h",
	MAME_DIR .. "src/mame/video/pass.cpp",
	MAME_DIR .. "src/mame/drivers/peplus.cpp",
	MAME_DIR .. "src/mame/drivers/photon.cpp",
	MAME_DIR .. "src/mame/drivers/piggypas.cpp",
	MAME_DIR .. "src/mame/video/pk8000.cpp",
	MAME_DIR .. "src/mame/drivers/photon2.cpp",
	MAME_DIR .. "src/mame/drivers/photoply.cpp",
	MAME_DIR .. "src/mame/drivers/pinkiri8.cpp",
	MAME_DIR .. "src/mame/drivers/pipeline.cpp",
	MAME_DIR .. "src/mame/drivers/pkscram.cpp",
	MAME_DIR .. "src/mame/drivers/pntnpuzl.cpp",
	MAME_DIR .. "src/mame/drivers/policetr.cpp",
	MAME_DIR .. "src/mame/includes/policetr.h",
	MAME_DIR .. "src/mame/video/policetr.cpp",
	MAME_DIR .. "src/mame/drivers/polyplay.cpp",
	MAME_DIR .. "src/mame/includes/polyplay.h",
	MAME_DIR .. "src/mame/audio/polyplay.cpp",
	MAME_DIR .. "src/mame/video/polyplay.cpp",
	MAME_DIR .. "src/mame/drivers/poker72.cpp",
	MAME_DIR .. "src/mame/drivers/potgoldu.cpp",
	MAME_DIR .. "src/mame/drivers/proconn.cpp",
	MAME_DIR .. "src/mame/drivers/pse.cpp",
	MAME_DIR .. "src/mame/drivers/quizo.cpp",
	MAME_DIR .. "src/mame/drivers/quizpun2.cpp",
	MAME_DIR .. "src/mame/drivers/rbmk.cpp",
	MAME_DIR .. "src/mame/drivers/rcorsair.cpp",
	MAME_DIR .. "src/mame/drivers/re900.cpp",
	MAME_DIR .. "src/mame/drivers/rgum.cpp",
	MAME_DIR .. "src/mame/drivers/roul.cpp",
	MAME_DIR .. "src/mame/drivers/savquest.cpp",
	MAME_DIR .. "src/mame/drivers/sanremo.cpp",
	MAME_DIR .. "src/mame/drivers/sanremmg.cpp",
	MAME_DIR .. "src/mame/drivers/sealy.cpp",
	MAME_DIR .. "src/mame/drivers/scm_500.cpp",
	MAME_DIR .. "src/mame/drivers/sfbonus.cpp",
	MAME_DIR .. "src/mame/drivers/shangkid.cpp",
	MAME_DIR .. "src/mame/includes/shangkid.h",
	MAME_DIR .. "src/mame/video/shangkid.cpp",
	MAME_DIR .. "src/mame/drivers/skeetsht.cpp",
	MAME_DIR .. "src/mame/drivers/skimaxx.cpp",
	MAME_DIR .. "src/mame/drivers/skyarmy.cpp",
	MAME_DIR .. "src/mame/drivers/skylncr.cpp",
	MAME_DIR .. "src/mame/drivers/sliver.cpp",
	MAME_DIR .. "src/mame/drivers/slotcarn.cpp",
	MAME_DIR .. "src/mame/drivers/smsmcorp.cpp",
	MAME_DIR .. "src/mame/drivers/sothello.cpp",
	MAME_DIR .. "src/mame/drivers/splus.cpp",
	MAME_DIR .. "src/mame/drivers/spool99.cpp",
	MAME_DIR .. "src/mame/drivers/sprcros2.cpp",
	MAME_DIR .. "src/mame/includes/sprcros2.h",
	MAME_DIR .. "src/mame/video/sprcros2.cpp",
	MAME_DIR .. "src/mame/drivers/sshot.cpp",
	MAME_DIR .. "src/mame/drivers/ssingles.cpp",
	MAME_DIR .. "src/mame/drivers/sstrangr.cpp",
	MAME_DIR .. "src/mame/drivers/statriv2.cpp",
	MAME_DIR .. "src/mame/drivers/stellafr.cpp",
	MAME_DIR .. "src/mame/drivers/stuntair.cpp",
	MAME_DIR .. "src/mame/drivers/su2000.cpp",
	MAME_DIR .. "src/mame/drivers/subhuntr.cpp",
	MAME_DIR .. "src/mame/drivers/summit.cpp",
	MAME_DIR .. "src/mame/drivers/sumt8035.cpp",
	MAME_DIR .. "src/mame/drivers/supercrd.cpp",
	MAME_DIR .. "src/mame/drivers/supertnk.cpp",
	MAME_DIR .. "src/mame/drivers/superwng.cpp",
	MAME_DIR .. "src/mame/drivers/tapatune.cpp",
	MAME_DIR .. "src/mame/drivers/tattack.cpp",
	MAME_DIR .. "src/mame/drivers/taxidriv.cpp",
	MAME_DIR .. "src/mame/includes/taxidriv.h",
	MAME_DIR .. "src/mame/video/taxidriv.cpp",
	MAME_DIR .. "src/mame/drivers/tcl.cpp",
	MAME_DIR .. "src/mame/drivers/thayers.cpp",
	MAME_DIR .. "src/mame/drivers/thedeep.cpp",
	MAME_DIR .. "src/mame/includes/thedeep.h",
	MAME_DIR .. "src/mame/video/thedeep.cpp",
	MAME_DIR .. "src/mame/drivers/tiamc1.cpp",
	MAME_DIR .. "src/mame/includes/tiamc1.h",
	MAME_DIR .. "src/mame/video/tiamc1.cpp",
	MAME_DIR .. "src/mame/audio/tiamc1.cpp",
	MAME_DIR .. "src/mame/drivers/tickee.cpp",
	MAME_DIR .. "src/mame/drivers/tmspoker.cpp",
	MAME_DIR .. "src/mame/drivers/truco.cpp",
	MAME_DIR .. "src/mame/includes/truco.h",
	MAME_DIR .. "src/mame/video/truco.cpp",
	MAME_DIR .. "src/mame/drivers/trucocl.cpp",
	MAME_DIR .. "src/mame/includes/trucocl.h",
	MAME_DIR .. "src/mame/video/trucocl.cpp",
	MAME_DIR .. "src/mame/drivers/trvmadns.cpp",
	MAME_DIR .. "src/mame/drivers/trvquest.cpp",
	MAME_DIR .. "src/mame/drivers/ttchamp.cpp",
	MAME_DIR .. "src/mame/drivers/tugboat.cpp",
	MAME_DIR .. "src/mame/drivers/ice_bozopail.cpp",
	MAME_DIR .. "src/mame/drivers/ice_tbd.cpp",
	MAME_DIR .. "src/mame/drivers/umipoker.cpp",
	MAME_DIR .. "src/mame/drivers/unkfr.cpp",
	MAME_DIR .. "src/mame/drivers/unkhorse.cpp",
	MAME_DIR .. "src/mame/drivers/usgames.cpp",
	MAME_DIR .. "src/mame/includes/usgames.h",
	MAME_DIR .. "src/mame/video/usgames.cpp",
	MAME_DIR .. "src/mame/drivers/vamphalf.cpp",
	MAME_DIR .. "src/mame/drivers/vcombat.cpp",
	MAME_DIR .. "src/mame/drivers/vectrex.cpp",
	MAME_DIR .. "src/mame/includes/vectrex.h",
	MAME_DIR .. "src/mame/video/vectrex.cpp",
	MAME_DIR .. "src/mame/machine/vectrex.cpp",
	MAME_DIR .. "src/mame/drivers/videopkr.cpp",
	MAME_DIR .. "src/mame/drivers/vlc.cpp",
	MAME_DIR .. "src/mame/drivers/voyager.cpp",
	MAME_DIR .. "src/mame/drivers/vp101.cpp",
	MAME_DIR .. "src/mame/drivers/vpoker.cpp",
	MAME_DIR .. "src/mame/drivers/vroulet.cpp",
	MAME_DIR .. "src/mame/drivers/wildpkr.cpp",
	MAME_DIR .. "src/mame/drivers/wms.cpp",
	MAME_DIR .. "src/mame/drivers/wacky_gator.cpp",
	MAME_DIR .. "src/mame/drivers/xtom3d.cpp",
	MAME_DIR .. "src/mame/drivers/xyonix.cpp",
	MAME_DIR .. "src/mame/includes/xyonix.h",
	MAME_DIR .. "src/mame/video/xyonix.cpp",
}
end

