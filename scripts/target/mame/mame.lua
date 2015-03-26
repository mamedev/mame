---------------------------------------------------------------------------
--
--   mame.lua
--
--   MAME target makefile
--
--   Copyright Nicola Salmoria and the MAME Team.
--   Visit http://mamedev.org for licensing and usage restrictions.
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
CPUS["TMS0980"] = true
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
CPUS["AMIS2000"] = true
CPUS["UCOM4"] = true
CPUS["HMCS40"] = true

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
MACHINES["AT29040"] = true
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
MACHINES["WD17XX"] = true
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
--BUSES["MIDI"] = true
--BUSES["MEGADRIVE"] = true
--BUSES["MSX_SLOT"] = true
BUSES["NEOGEO"] = true
--BUSES["NES"] = true
--BUSES["NUBUS"] = true
--BUSES["O2"] = true
--BUSES["ORICEXT"] = true
--BUSES["PCE"] = true
--BUSES["PC_JOY"] = true
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

function linkProjects(_target, _subtarget)
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
	kind "StaticLib"
	uuid (os.uuid("drv-" .. _target .."_" .. _subtarget .. "_" .._name))
	
	options {
		"ForceCPP",
	}
	
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/zlib",
		GEN_DIR  .. "mame/layout",
	}

	includeosd()
end
	
function createProjects(_target, _subtarget)
--------------------------------------------------
-- the following files are general components and
-- shared across a number of drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "shared")
files {
	MAME_DIR .. "src/mame/machine/nmk112.*",
	MAME_DIR .. "src/mame/machine/pcshare.*",
	MAME_DIR .. "src/mame/machine/segacrpt.*",
	MAME_DIR .. "src/mame/machine/segacrp2.*",
	MAME_DIR .. "src/mame/machine/ticket.*",
	MAME_DIR .. "src/mame/video/avgdvg.*",
	MAME_DIR .. "src/mame/audio/dcs.*",
	MAME_DIR .. "src/mame/audio/decobsmt.*",
	MAME_DIR .. "src/mame/audio/segam1audio.*",
}

--------------------------------------------------
-- manufacturer-specific groupings for drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "acorn")
files {
	MAME_DIR .. "src/mame/drivers/ertictac.*",
	MAME_DIR .. "src/mame/drivers/ssfindo.*",
	MAME_DIR .. "src/mame/drivers/aristmk5.*",
	MAME_DIR .. "src/mame/machine/archimds.*",
	MAME_DIR .. "src/mame/video/archimds.*",
}

createMAMEProjects(_target, _subtarget, "alba")
files {
	MAME_DIR .. "src/mame/drivers/albazc.*",
	MAME_DIR .. "src/mame/drivers/albazg.*",
	MAME_DIR .. "src/mame/drivers/rmhaihai.*",
}

createMAMEProjects(_target, _subtarget, "alliedl")
files {
	MAME_DIR .. "src/mame/drivers/ace.*",
	MAME_DIR .. "src/mame/drivers/aleisttl.*",
	MAME_DIR .. "src/mame/drivers/clayshoo.*",
}

createMAMEProjects(_target, _subtarget, "alpha")
files {
	MAME_DIR .. "src/mame/drivers/alpha68k.*",
	MAME_DIR .. "src/mame/video/alpha68k.*",
	MAME_DIR .. "src/mame/drivers/champbas.*",
	MAME_DIR .. "src/mame/video/champbas.*",
	MAME_DIR .. "src/mame/drivers/equites.*",
	MAME_DIR .. "src/mame/video/equites.*",
	MAME_DIR .. "src/mame/drivers/meijinsn.*",
	MAME_DIR .. "src/mame/drivers/shougi.*",
}

createMAMEProjects(_target, _subtarget, "amiga")
files {
	MAME_DIR .. "src/mame/drivers/alg.*",
	MAME_DIR .. "src/mame/machine/amiga.*",
	MAME_DIR .. "src/mame/video/amiga.*",
	MAME_DIR .. "src/mame/video/amigaaga.*",
	MAME_DIR .. "src/mame/drivers/arcadia.*",
	MAME_DIR .. "src/mame/drivers/cubo.*",
	MAME_DIR .. "src/mame/drivers/mquake.*",
	MAME_DIR .. "src/mame/drivers/upscope.*",
}

createMAMEProjects(_target, _subtarget, "aristocr")
files {
	MAME_DIR .. "src/mame/drivers/aristmk4.*",
	MAME_DIR .. "src/mame/drivers/aristmk6.*",
	MAME_DIR .. "src/mame/drivers/caswin.*",
}

createMAMEProjects(_target, _subtarget, "ascii")
files {
	MAME_DIR .. "src/mame/drivers/big10.*",
	MAME_DIR .. "src/mame/drivers/forte2.*",
	MAME_DIR .. "src/mame/drivers/pengadvb.*",
	MAME_DIR .. "src/mame/drivers/sangho.*",
	MAME_DIR .. "src/mame/drivers/sfkick.*",
}

createMAMEProjects(_target, _subtarget, "atari")
files {
	MAME_DIR .. "src/mame/drivers/arcadecl.*",
	MAME_DIR .. "src/mame/video/arcadecl.*",
	MAME_DIR .. "src/mame/drivers/asteroid.*",
	MAME_DIR .. "src/mame/machine/asteroid.*",
	MAME_DIR .. "src/mame/audio/asteroid.*",
	MAME_DIR .. "src/mame/audio/llander.*",
	MAME_DIR .. "src/mame/drivers/atarifb.*",
	MAME_DIR .. "src/mame/machine/atarifb.*",
	MAME_DIR .. "src/mame/audio/atarifb.*",
	MAME_DIR .. "src/mame/video/atarifb.*",
	MAME_DIR .. "src/mame/drivers/atarig1.*",
	MAME_DIR .. "src/mame/video/atarig1.*",
	MAME_DIR .. "src/mame/drivers/atarig42.*",
	MAME_DIR .. "src/mame/video/atarig42.*",
	MAME_DIR .. "src/mame/drivers/atarigt.*",
	MAME_DIR .. "src/mame/video/atarigt.*",
	MAME_DIR .. "src/mame/drivers/atarigx2.*",
	MAME_DIR .. "src/mame/video/atarigx2.*",
	MAME_DIR .. "src/mame/drivers/atarisy1.*",
	MAME_DIR .. "src/mame/video/atarisy1.*",
	MAME_DIR .. "src/mame/drivers/atarisy2.*",
	MAME_DIR .. "src/mame/video/atarisy2.*",
	MAME_DIR .. "src/mame/drivers/atarisy4.*",
	MAME_DIR .. "src/mame/drivers/atarittl.*",
	MAME_DIR .. "src/mame/drivers/atetris.*",
	MAME_DIR .. "src/mame/video/atetris.*",
	MAME_DIR .. "src/mame/drivers/avalnche.*",
	MAME_DIR .. "src/mame/audio/avalnche.*",
	MAME_DIR .. "src/mame/drivers/badlands.*",
	MAME_DIR .. "src/mame/video/badlands.*",
	MAME_DIR .. "src/mame/drivers/bartop52.*",
	MAME_DIR .. "src/mame/drivers/batman.*",
	MAME_DIR .. "src/mame/video/batman.*",
	MAME_DIR .. "src/mame/drivers/beathead.*",
	MAME_DIR .. "src/mame/video/beathead.*",
	MAME_DIR .. "src/mame/drivers/blstroid.*",
	MAME_DIR .. "src/mame/video/blstroid.*",
	MAME_DIR .. "src/mame/drivers/boxer.*",
	MAME_DIR .. "src/mame/drivers/bsktball.*",
	MAME_DIR .. "src/mame/machine/bsktball.*",
	MAME_DIR .. "src/mame/audio/bsktball.*",
	MAME_DIR .. "src/mame/video/bsktball.*",
	MAME_DIR .. "src/mame/drivers/bwidow.*",
	MAME_DIR .. "src/mame/audio/bwidow.*",
	MAME_DIR .. "src/mame/drivers/bzone.*",
	MAME_DIR .. "src/mame/audio/bzone.*",
	MAME_DIR .. "src/mame/drivers/canyon.*",
	MAME_DIR .. "src/mame/audio/canyon.*",
	MAME_DIR .. "src/mame/video/canyon.*",
	MAME_DIR .. "src/mame/drivers/cball.*",
	MAME_DIR .. "src/mame/drivers/ccastles.*",
	MAME_DIR .. "src/mame/video/ccastles.*",
	MAME_DIR .. "src/mame/drivers/centiped.*",
	MAME_DIR .. "src/mame/video/centiped.*",
	MAME_DIR .. "src/mame/drivers/cloak.*",
	MAME_DIR .. "src/mame/video/cloak.*",
	MAME_DIR .. "src/mame/drivers/cloud9.*",
	MAME_DIR .. "src/mame/video/cloud9.*",
	MAME_DIR .. "src/mame/drivers/cmmb.*",
	MAME_DIR .. "src/mame/drivers/cops.*",
	MAME_DIR .. "src/mame/drivers/copsnrob.*",
	MAME_DIR .. "src/mame/audio/copsnrob.*",
	MAME_DIR .. "src/mame/video/copsnrob.*",
	MAME_DIR .. "src/mame/drivers/cyberbal.*",
	MAME_DIR .. "src/mame/audio/cyberbal.*",
	MAME_DIR .. "src/mame/video/cyberbal.*",
	MAME_DIR .. "src/mame/drivers/destroyr.*",
	MAME_DIR .. "src/mame/drivers/dragrace.*",
	MAME_DIR .. "src/mame/audio/dragrace.*",
	MAME_DIR .. "src/mame/video/dragrace.*",
	MAME_DIR .. "src/mame/drivers/eprom.*",
	MAME_DIR .. "src/mame/video/eprom.*",
	MAME_DIR .. "src/mame/drivers/firefox.*",
	MAME_DIR .. "src/mame/drivers/firetrk.*",
	MAME_DIR .. "src/mame/audio/firetrk.*",
	MAME_DIR .. "src/mame/video/firetrk.*",
	MAME_DIR .. "src/mame/drivers/flyball.*",
	MAME_DIR .. "src/mame/drivers/foodf.*",
	MAME_DIR .. "src/mame/video/foodf.*",
	MAME_DIR .. "src/mame/drivers/gauntlet.*",
	MAME_DIR .. "src/mame/video/gauntlet.*",
	MAME_DIR .. "src/mame/drivers/harddriv.*",
	MAME_DIR .. "src/mame/machine/harddriv.*",
	MAME_DIR .. "src/mame/audio/harddriv.*",
	MAME_DIR .. "src/mame/video/harddriv.*",
	MAME_DIR .. "src/mame/drivers/irobot.*",
	MAME_DIR .. "src/mame/machine/irobot.*",
	MAME_DIR .. "src/mame/video/irobot.*",
	MAME_DIR .. "src/mame/drivers/jaguar.*",
	MAME_DIR .. "src/mame/audio/jaguar.*",
	MAME_DIR .. "src/mame/video/jaguar.*",
	MAME_DIR .. "src/mame/drivers/jedi.*",
	MAME_DIR .. "src/mame/audio/jedi.*",
	MAME_DIR .. "src/mame/video/jedi.*",
	MAME_DIR .. "src/mame/drivers/klax.*",
	MAME_DIR .. "src/mame/video/klax.*",
	MAME_DIR .. "src/mame/drivers/liberatr.*",
	MAME_DIR .. "src/mame/video/liberatr.*",
	MAME_DIR .. "src/mame/drivers/mediagx.*",
	MAME_DIR .. "src/mame/drivers/metalmx.*",
	MAME_DIR .. "src/mame/drivers/mgolf.*",
	MAME_DIR .. "src/mame/drivers/mhavoc.*",
	MAME_DIR .. "src/mame/machine/mhavoc.*",
	MAME_DIR .. "src/mame/drivers/missile.*",
	MAME_DIR .. "src/mame/drivers/nitedrvr.*",
	MAME_DIR .. "src/mame/machine/nitedrvr.*",
	MAME_DIR .. "src/mame/audio/nitedrvr.*",
	MAME_DIR .. "src/mame/video/nitedrvr.*",
	MAME_DIR .. "src/mame/drivers/offtwall.*",
	MAME_DIR .. "src/mame/video/offtwall.*",
	MAME_DIR .. "src/mame/drivers/orbit.*",
	MAME_DIR .. "src/mame/audio/orbit.*",
	MAME_DIR .. "src/mame/video/orbit.*",
	MAME_DIR .. "src/mame/drivers/pong.*",
	MAME_DIR .. "src/mame/drivers/nl_pong.*",
	MAME_DIR .. "src/mame/drivers/nl_pongd.*",
	MAME_DIR .. "src/mame/drivers/poolshrk.*",
	MAME_DIR .. "src/mame/audio/poolshrk.*",
	MAME_DIR .. "src/mame/video/poolshrk.*",
	MAME_DIR .. "src/mame/drivers/quantum.*",
	MAME_DIR .. "src/mame/drivers/quizshow.*",
	MAME_DIR .. "src/mame/drivers/rampart.*",
	MAME_DIR .. "src/mame/video/rampart.*",
	MAME_DIR .. "src/mame/drivers/relief.*",
	MAME_DIR .. "src/mame/video/relief.*",
	MAME_DIR .. "src/mame/drivers/runaway.*",
	MAME_DIR .. "src/mame/video/runaway.*",
	MAME_DIR .. "src/mame/drivers/sbrkout.*",
	MAME_DIR .. "src/mame/drivers/shuuz.*",
	MAME_DIR .. "src/mame/video/shuuz.*",
	MAME_DIR .. "src/mame/drivers/skullxbo.*",
	MAME_DIR .. "src/mame/video/skullxbo.*",
	MAME_DIR .. "src/mame/drivers/skydiver.*",
	MAME_DIR .. "src/mame/audio/skydiver.*",
	MAME_DIR .. "src/mame/video/skydiver.*",
	MAME_DIR .. "src/mame/drivers/skyraid.*",
	MAME_DIR .. "src/mame/audio/skyraid.*",
	MAME_DIR .. "src/mame/video/skyraid.*",
	MAME_DIR .. "src/mame/drivers/sprint2.*",
	MAME_DIR .. "src/mame/audio/sprint2.*",
	MAME_DIR .. "src/mame/video/sprint2.*",
	MAME_DIR .. "src/mame/drivers/sprint4.*",
	MAME_DIR .. "src/mame/video/sprint4.*",
	MAME_DIR .. "src/mame/audio/sprint4.*",
	MAME_DIR .. "src/mame/drivers/sprint8.*",
	MAME_DIR .. "src/mame/audio/sprint8.*",
	MAME_DIR .. "src/mame/video/sprint8.*",
	MAME_DIR .. "src/mame/drivers/starshp1.*",
	MAME_DIR .. "src/mame/audio/starshp1.*",
	MAME_DIR .. "src/mame/video/starshp1.*",
	MAME_DIR .. "src/mame/drivers/starwars.*",
	MAME_DIR .. "src/mame/machine/starwars.*",
	MAME_DIR .. "src/mame/audio/starwars.*",
	MAME_DIR .. "src/mame/drivers/subs.*",
	MAME_DIR .. "src/mame/machine/subs.*",
	MAME_DIR .. "src/mame/audio/subs.*",
	MAME_DIR .. "src/mame/video/subs.*",
	MAME_DIR .. "src/mame/drivers/tank8.*",
	MAME_DIR .. "src/mame/audio/tank8.*",
	MAME_DIR .. "src/mame/video/tank8.*",
	MAME_DIR .. "src/mame/drivers/tempest.*",
	MAME_DIR .. "src/mame/drivers/thunderj.*",
	MAME_DIR .. "src/mame/video/thunderj.*",
	MAME_DIR .. "src/mame/drivers/tomcat.*",
	MAME_DIR .. "src/mame/drivers/toobin.*",
	MAME_DIR .. "src/mame/video/toobin.*",
	MAME_DIR .. "src/mame/drivers/tourtabl.*",
	MAME_DIR .. "src/mame/video/tia.*",
	MAME_DIR .. "src/mame/drivers/triplhnt.*",
	MAME_DIR .. "src/mame/audio/triplhnt.*",
	MAME_DIR .. "src/mame/video/triplhnt.*",
	MAME_DIR .. "src/mame/drivers/tunhunt.*",
	MAME_DIR .. "src/mame/video/tunhunt.*",
	MAME_DIR .. "src/mame/drivers/ultratnk.*",
	MAME_DIR .. "src/mame/video/ultratnk.*",
	MAME_DIR .. "src/mame/drivers/videopin.*",
	MAME_DIR .. "src/mame/audio/videopin.*",
	MAME_DIR .. "src/mame/video/videopin.*",
	MAME_DIR .. "src/mame/drivers/vindictr.*",
	MAME_DIR .. "src/mame/video/vindictr.*",
	MAME_DIR .. "src/mame/drivers/wolfpack.*",
	MAME_DIR .. "src/mame/video/wolfpack.*",
	MAME_DIR .. "src/mame/drivers/xybots.*",
	MAME_DIR .. "src/mame/video/xybots.*",
	MAME_DIR .. "src/mame/machine/asic65.*",
	MAME_DIR .. "src/mame/machine/atari_vg.*",
	MAME_DIR .. "src/mame/machine/atarigen.*",
	MAME_DIR .. "src/mame/machine/mathbox.*",
	MAME_DIR .. "src/mame/machine/slapstic.*",
	MAME_DIR .. "src/mame/audio/atarijsa.*",
	MAME_DIR .. "src/mame/audio/cage.*",
	MAME_DIR .. "src/mame/audio/redbaron.*",
	MAME_DIR .. "src/mame/video/atarimo.*",
	MAME_DIR .. "src/mame/video/atarirle.*",
}

createMAMEProjects(_target, _subtarget, "atlus")
files {
	MAME_DIR .. "src/mame/drivers/blmbycar.*",
	MAME_DIR .. "src/mame/video/blmbycar.*",
	MAME_DIR .. "src/mame/drivers/ohmygod.*",
	MAME_DIR .. "src/mame/video/ohmygod.*",
	MAME_DIR .. "src/mame/drivers/powerins.*",
	MAME_DIR .. "src/mame/video/powerins.*",
	MAME_DIR .. "src/mame/drivers/bowltry.*",
}

createMAMEProjects(_target, _subtarget, "barcrest")
files {
	MAME_DIR .. "src/mame/drivers/mpu2.*",
	MAME_DIR .. "src/mame/drivers/mpu3.*",
	MAME_DIR .. "src/mame/drivers/mpu4hw.*",
	MAME_DIR .. "src/mame/drivers/mpu4sw.*",
	MAME_DIR .. "src/mame/drivers/mpu4.*",
	MAME_DIR .. "src/mame/drivers/mpu4mod2sw.*",
	MAME_DIR .. "src/mame/drivers/mpu4mod4yam.*",
	MAME_DIR .. "src/mame/drivers/mpu4plasma.*",
	MAME_DIR .. "src/mame/drivers/mpu4dealem.*",
	MAME_DIR .. "src/mame/drivers/mpu4vid.*",
	MAME_DIR .. "src/mame/drivers/mpu4avan.*",
	MAME_DIR .. "src/mame/drivers/mpu4union.*",
	MAME_DIR .. "src/mame/drivers/mpu4concept.*",
	MAME_DIR .. "src/mame/drivers/mpu4empire.*",
	MAME_DIR .. "src/mame/drivers/mpu4mdm.*",
	MAME_DIR .. "src/mame/drivers/mpu4crystal.*",
	MAME_DIR .. "src/mame/drivers/mpu4bwb.*",
	MAME_DIR .. "src/mame/drivers/mpu4misc.*",
	MAME_DIR .. "src/mame/drivers/mpu5hw.*",
	MAME_DIR .. "src/mame/drivers/mpu5.*",
	MAME_DIR .. "src/mame/video/awpvid.*",
	MAME_DIR .. "src/mame/machine/meters.*",
}

createMAMEProjects(_target, _subtarget, "bfm")
files {
	MAME_DIR .. "src/mame/drivers/bfcobra.*",
	MAME_DIR .. "src/mame/machine/bfm_comn.*",
	MAME_DIR .. "src/mame/drivers/bfm_sc1.*",
	MAME_DIR .. "src/mame/drivers/bfm_sc2.*",
	MAME_DIR .. "src/mame/video/bfm_adr2.*",
	MAME_DIR .. "src/mame/drivers/bfm_sc4.*",
	MAME_DIR .. "src/mame/drivers/bfm_sc4h.*",
	MAME_DIR .. "src/mame/drivers/bfm_sc5.*",
	MAME_DIR .. "src/mame/drivers/bfm_sc5sw.*",
	MAME_DIR .. "src/mame/drivers/bfm_ad5.*",
	MAME_DIR .. "src/mame/drivers/bfm_ad5sw.*",
	MAME_DIR .. "src/mame/drivers/bfm_sc45_helper.*",
	MAME_DIR .. "src/mame/drivers/bfm_swp.*",
	MAME_DIR .. "src/mame/drivers/bfmsys83.*",
	MAME_DIR .. "src/mame/drivers/bfmsys85.*",
	MAME_DIR .. "src/mame/machine/sec.*",
	MAME_DIR .. "src/mame/machine/bfm_bd1.*",
	MAME_DIR .. "src/mame/machine/bfm_bda.*",
	MAME_DIR .. "src/mame/video/bfm_dm01.*",
	MAME_DIR .. "src/mame/drivers/rastersp.*",
}

createMAMEProjects(_target, _subtarget, "bmc")
files {
	MAME_DIR .. "src/mame/drivers/bmcbowl.*",
	MAME_DIR .. "src/mame/drivers/koftball.*",
	MAME_DIR .. "src/mame/drivers/popobear.*",
	MAME_DIR .. "src/mame/drivers/bmcpokr.*",
}

createMAMEProjects(_target, _subtarget, "capcom")
files {
	MAME_DIR .. "src/mame/drivers/1942.*",
	MAME_DIR .. "src/mame/video/1942.*",
	MAME_DIR .. "src/mame/drivers/1943.*",
	MAME_DIR .. "src/mame/video/1943.*",
	MAME_DIR .. "src/mame/drivers/alien.*",
	MAME_DIR .. "src/mame/drivers/bionicc.*",
	MAME_DIR .. "src/mame/video/bionicc.*",
	MAME_DIR .. "src/mame/drivers/supduck.*",
	MAME_DIR .. "src/mame/video/tigeroad_spr.*",
	MAME_DIR .. "src/mame/drivers/blktiger.*",
	MAME_DIR .. "src/mame/video/blktiger.*",
	MAME_DIR .. "src/mame/drivers/cbasebal.*",
	MAME_DIR .. "src/mame/video/cbasebal.*",
	MAME_DIR .. "src/mame/drivers/commando.*",
	MAME_DIR .. "src/mame/video/commando.*",
	MAME_DIR .. "src/mame/drivers/cps1.*",
	MAME_DIR .. "src/mame/video/cps1.*",
	MAME_DIR .. "src/mame/drivers/kenseim.*",
	MAME_DIR .. "src/mame/drivers/cps2.*",
	MAME_DIR .. "src/mame/machine/cps2crpt.*",
	MAME_DIR .. "src/mame/drivers/cps3.*",
	MAME_DIR .. "src/mame/audio/cps3.*",
	MAME_DIR .. "src/mame/drivers/egghunt.*",
	MAME_DIR .. "src/mame/drivers/exedexes.*",
	MAME_DIR .. "src/mame/video/exedexes.*",
	MAME_DIR .. "src/mame/drivers/fcrash.*",
	MAME_DIR .. "src/mame/drivers/gng.*",
	MAME_DIR .. "src/mame/video/gng.*",
	MAME_DIR .. "src/mame/drivers/gunsmoke.*",
	MAME_DIR .. "src/mame/video/gunsmoke.*",
	MAME_DIR .. "src/mame/drivers/higemaru.*",
	MAME_DIR .. "src/mame/video/higemaru.*",
	MAME_DIR .. "src/mame/drivers/lastduel.*",
	MAME_DIR .. "src/mame/video/lastduel.*",
	MAME_DIR .. "src/mame/drivers/lwings.*",
	MAME_DIR .. "src/mame/video/lwings.*",
	MAME_DIR .. "src/mame/drivers/mitchell.*",
	MAME_DIR .. "src/mame/video/mitchell.*",
	MAME_DIR .. "src/mame/drivers/sf.*",
	MAME_DIR .. "src/mame/video/sf.*",
	MAME_DIR .. "src/mame/drivers/sidearms.*",
	MAME_DIR .. "src/mame/video/sidearms.*",
	MAME_DIR .. "src/mame/drivers/sonson.*",
	MAME_DIR .. "src/mame/video/sonson.*",
	MAME_DIR .. "src/mame/drivers/srumbler.*",
	MAME_DIR .. "src/mame/video/srumbler.*",
	MAME_DIR .. "src/mame/drivers/tigeroad.*",
	MAME_DIR .. "src/mame/video/tigeroad.*",
	MAME_DIR .. "src/mame/machine/tigeroad.*",
	MAME_DIR .. "src/mame/drivers/vulgus.*",
	MAME_DIR .. "src/mame/video/vulgus.*",
	MAME_DIR .. "src/mame/machine/kabuki.*",
}

createMAMEProjects(_target, _subtarget, "cinemat")
files {
	MAME_DIR .. "src/mame/drivers/ataxx.*",
	MAME_DIR .. "src/mame/drivers/cinemat.*",
	MAME_DIR .. "src/mame/audio/cinemat.*",
	MAME_DIR .. "src/mame/video/cinemat.*",
	MAME_DIR .. "src/mame/drivers/cchasm.*",
	MAME_DIR .. "src/mame/machine/cchasm.*",
	MAME_DIR .. "src/mame/audio/cchasm.*",
	MAME_DIR .. "src/mame/video/cchasm.*",
	MAME_DIR .. "src/mame/drivers/dlair.*",
	MAME_DIR .. "src/mame/drivers/dlair2.*",
	MAME_DIR .. "src/mame/drivers/embargo.*",
	MAME_DIR .. "src/mame/drivers/jack.*",
	MAME_DIR .. "src/mame/video/jack.*",
	MAME_DIR .. "src/mame/drivers/leland.*",
	MAME_DIR .. "src/mame/machine/leland.*",
	MAME_DIR .. "src/mame/audio/leland.*",
	MAME_DIR .. "src/mame/video/leland.*",
}

createMAMEProjects(_target, _subtarget, "comad")
files {
	MAME_DIR .. "src/mame/drivers/funybubl.*",
	MAME_DIR .. "src/mame/video/funybubl.*",
	MAME_DIR .. "src/mame/drivers/galspnbl.*",
	MAME_DIR .. "src/mame/video/galspnbl.*",
	MAME_DIR .. "src/mame/drivers/zerozone.*",
	MAME_DIR .. "src/mame/video/zerozone.*",
}

createMAMEProjects(_target, _subtarget, "cvs")
files {
	MAME_DIR .. "src/mame/drivers/cvs.*",
	MAME_DIR .. "src/mame/video/cvs.*",
	MAME_DIR .. "src/mame/drivers/galaxia.*",
	MAME_DIR .. "src/mame/video/galaxia.*",
	MAME_DIR .. "src/mame/drivers/quasar.*",
	MAME_DIR .. "src/mame/video/quasar.*",
}

createMAMEProjects(_target, _subtarget, "dataeast")
files {
	MAME_DIR .. "src/mame/drivers/actfancr.*",
	MAME_DIR .. "src/mame/video/actfancr.*",
	MAME_DIR .. "src/mame/drivers/astrof.*",
	MAME_DIR .. "src/mame/audio/astrof.*",
	MAME_DIR .. "src/mame/drivers/backfire.*",
	MAME_DIR .. "src/mame/drivers/battlera.*",
	MAME_DIR .. "src/mame/video/battlera.*",
	MAME_DIR .. "src/mame/drivers/boogwing.*",
	MAME_DIR .. "src/mame/video/boogwing.*",
	MAME_DIR .. "src/mame/drivers/brkthru.*",
	MAME_DIR .. "src/mame/video/brkthru.*",
	MAME_DIR .. "src/mame/drivers/btime.*",
	MAME_DIR .. "src/mame/machine/btime.*",
	MAME_DIR .. "src/mame/video/btime.*",
	MAME_DIR .. "src/mame/drivers/bwing.*",
	MAME_DIR .. "src/mame/video/bwing.*",
	MAME_DIR .. "src/mame/drivers/cbuster.*",
	MAME_DIR .. "src/mame/video/cbuster.*",
	MAME_DIR .. "src/mame/drivers/chanbara.*",
	MAME_DIR .. "src/mame/drivers/cninja.*",
	MAME_DIR .. "src/mame/video/cninja.*",
	MAME_DIR .. "src/mame/drivers/cntsteer.*",
	MAME_DIR .. "src/mame/drivers/compgolf.*",
	MAME_DIR .. "src/mame/video/compgolf.*",
	MAME_DIR .. "src/mame/drivers/darkseal.*",
	MAME_DIR .. "src/mame/video/darkseal.*",
	MAME_DIR .. "src/mame/drivers/dassault.*",
	MAME_DIR .. "src/mame/video/dassault.*",
	MAME_DIR .. "src/mame/drivers/dblewing.*",
	MAME_DIR .. "src/mame/drivers/dec0.*",
	MAME_DIR .. "src/mame/machine/dec0.*",
	MAME_DIR .. "src/mame/video/dec0.*",
	MAME_DIR .. "src/mame/drivers/dec8.*",
	MAME_DIR .. "src/mame/video/dec8.*",
	MAME_DIR .. "src/mame/machine/deco222.*",
	MAME_DIR .. "src/mame/machine/decocpu7.*",
	MAME_DIR .. "src/mame/machine/decocpu6.*",
	MAME_DIR .. "src/mame/drivers/deco_ld.*",
	MAME_DIR .. "src/mame/drivers/deco_mlc.*",
	MAME_DIR .. "src/mame/video/deco_mlc.*",
	MAME_DIR .. "src/mame/drivers/deco156.*",
	MAME_DIR .. "src/mame/machine/deco156.*",
	MAME_DIR .. "src/mame/drivers/deco32.*",
	MAME_DIR .. "src/mame/video/deco32.*",
	MAME_DIR .. "src/mame/video/dvi.*",
	MAME_DIR .. "src/mame/video/deco_zoomspr.*",
	MAME_DIR .. "src/mame/drivers/decocass.*",
	MAME_DIR .. "src/mame/machine/decocass.*",
	MAME_DIR .. "src/mame/machine/decocass_tape.*",
	MAME_DIR .. "src/mame/video/decocass.*",
	MAME_DIR .. "src/mame/drivers/deshoros.*",
	MAME_DIR .. "src/mame/drivers/dietgo.*",
	MAME_DIR .. "src/mame/video/dietgo.*",
	MAME_DIR .. "src/mame/drivers/dreambal.*",
	MAME_DIR .. "src/mame/drivers/exprraid.*",
	MAME_DIR .. "src/mame/video/exprraid.*",
	MAME_DIR .. "src/mame/drivers/firetrap.*",
	MAME_DIR .. "src/mame/video/firetrap.*",
	MAME_DIR .. "src/mame/drivers/funkyjet.*",
	MAME_DIR .. "src/mame/video/funkyjet.*",
	MAME_DIR .. "src/mame/drivers/karnov.*",
	MAME_DIR .. "src/mame/video/karnov.*",
	MAME_DIR .. "src/mame/drivers/kchamp.*",
	MAME_DIR .. "src/mame/video/kchamp.*",
	MAME_DIR .. "src/mame/drivers/kingobox.*",
	MAME_DIR .. "src/mame/video/kingobox.*",
	MAME_DIR .. "src/mame/drivers/lemmings.*",
	MAME_DIR .. "src/mame/video/lemmings.*",
	MAME_DIR .. "src/mame/drivers/liberate.*",
	MAME_DIR .. "src/mame/video/liberate.*",
	MAME_DIR .. "src/mame/drivers/madalien.*",
	MAME_DIR .. "src/mame/audio/madalien.*",
	MAME_DIR .. "src/mame/video/madalien.*",
	MAME_DIR .. "src/mame/drivers/madmotor.*",
	MAME_DIR .. "src/mame/video/madmotor.*",
	MAME_DIR .. "src/mame/drivers/metlclsh.*",
	MAME_DIR .. "src/mame/video/metlclsh.*",
	MAME_DIR .. "src/mame/drivers/mirage.*",
	MAME_DIR .. "src/mame/drivers/pcktgal.*",
	MAME_DIR .. "src/mame/video/pcktgal.*",
	MAME_DIR .. "src/mame/drivers/pktgaldx.*",
	MAME_DIR .. "src/mame/video/pktgaldx.*",
	MAME_DIR .. "src/mame/drivers/progolf.*",
	MAME_DIR .. "src/mame/drivers/rohga.*",
	MAME_DIR .. "src/mame/video/rohga.*",
	MAME_DIR .. "src/mame/drivers/shootout.*",
	MAME_DIR .. "src/mame/video/shootout.*",
	MAME_DIR .. "src/mame/drivers/sidepckt.*",
	MAME_DIR .. "src/mame/video/sidepckt.*",
	MAME_DIR .. "src/mame/drivers/simpl156.*",
	MAME_DIR .. "src/mame/video/simpl156.*",
	MAME_DIR .. "src/mame/drivers/sshangha.*",
	MAME_DIR .. "src/mame/video/sshangha.*",
	MAME_DIR .. "src/mame/drivers/stadhero.*",
	MAME_DIR .. "src/mame/video/stadhero.*",
	MAME_DIR .. "src/mame/drivers/supbtime.*",
	MAME_DIR .. "src/mame/video/supbtime.*",
	MAME_DIR .. "src/mame/drivers/tryout.*",
	MAME_DIR .. "src/mame/video/tryout.*",
	MAME_DIR .. "src/mame/drivers/tumbleb.*",
	MAME_DIR .. "src/mame/video/tumbleb.*",
	MAME_DIR .. "src/mame/drivers/tumblep.*",
	MAME_DIR .. "src/mame/video/tumblep.*",
	MAME_DIR .. "src/mame/drivers/vaportra.*",
	MAME_DIR .. "src/mame/video/vaportra.*",
	MAME_DIR .. "src/mame/machine/deco102.*",
	MAME_DIR .. "src/mame/machine/decocrpt.*",
	MAME_DIR .. "src/mame/machine/deco104.*",
	MAME_DIR .. "src/mame/machine/deco146.*",
	MAME_DIR .. "src/mame/video/decbac06.*",
	MAME_DIR .. "src/mame/video/deco16ic.*",
	MAME_DIR .. "src/mame/video/decocomn.*",
	MAME_DIR .. "src/mame/video/decospr.*",
	MAME_DIR .. "src/mame/video/decmxc06.*",
	MAME_DIR .. "src/mame/video/deckarn.*",
}

createMAMEProjects(_target, _subtarget, "dgrm")
files {
	MAME_DIR .. "src/mame/drivers/blackt96.*",
	MAME_DIR .. "src/mame/drivers/pokechmp.*",
	MAME_DIR .. "src/mame/video/pokechmp.*",
}

createMAMEProjects(_target, _subtarget, "dooyong")
files {
	MAME_DIR .. "src/mame/drivers/dooyong.*",
	MAME_DIR .. "src/mame/video/dooyong.*",
	MAME_DIR .. "src/mame/drivers/gundealr.*",
	MAME_DIR .. "src/mame/video/gundealr.*",
}

createMAMEProjects(_target, _subtarget, "dynax")
files {
	MAME_DIR .. "src/mame/drivers/ddenlovr.*",
	MAME_DIR .. "src/mame/drivers/dynax.*",
	MAME_DIR .. "src/mame/video/dynax.*",
	MAME_DIR .. "src/mame/drivers/hnayayoi.*",
	MAME_DIR .. "src/mame/video/hnayayoi.*",
	MAME_DIR .. "src/mame/drivers/realbrk.*",
	MAME_DIR .. "src/mame/video/realbrk.*",
	MAME_DIR .. "src/mame/drivers/royalmah.*",
}

createMAMEProjects(_target, _subtarget, "edevices")
files {
	MAME_DIR .. "src/mame/drivers/diverboy.*",
	MAME_DIR .. "src/mame/drivers/fantland.*",
	MAME_DIR .. "src/mame/video/fantland.*",
	MAME_DIR .. "src/mame/drivers/mwarr.*",
	MAME_DIR .. "src/mame/drivers/mugsmash.*",
	MAME_DIR .. "src/mame/video/mugsmash.*",
	MAME_DIR .. "src/mame/drivers/ppmast93.*",
	MAME_DIR .. "src/mame/drivers/pzletime.*",
	MAME_DIR .. "src/mame/drivers/stlforce.*",
	MAME_DIR .. "src/mame/video/stlforce.*",
	MAME_DIR .. "src/mame/drivers/twins.*",
}

createMAMEProjects(_target, _subtarget, "eolith")
files {
	MAME_DIR .. "src/mame/drivers/eolith.*",
	MAME_DIR .. "src/mame/video/eolith.*",
	MAME_DIR .. "src/mame/drivers/eolith16.*",
	MAME_DIR .. "src/mame/drivers/eolithsp.*",
	MAME_DIR .. "src/mame/drivers/ghosteo.*",
	MAME_DIR .. "src/mame/drivers/vegaeo.*",
}

createMAMEProjects(_target, _subtarget, "excelent")
files {
	MAME_DIR .. "src/mame/drivers/aquarium.*",
	MAME_DIR .. "src/mame/video/aquarium.*",
	MAME_DIR .. "src/mame/drivers/d9final.*",
	MAME_DIR .. "src/mame/drivers/dblcrown.*",
	MAME_DIR .. "src/mame/drivers/gcpinbal.*",
	MAME_DIR .. "src/mame/video/gcpinbal.*",
	MAME_DIR .. "src/mame/video/excellent_spr.*",
	MAME_DIR .. "src/mame/drivers/lastbank.*",
}

createMAMEProjects(_target, _subtarget, "exidy")
files {
	MAME_DIR .. "src/mame/drivers/carpolo.*",
	MAME_DIR .. "src/mame/machine/carpolo.*",
	MAME_DIR .. "src/mame/video/carpolo.*",
	MAME_DIR .. "src/mame/drivers/circus.*",
	MAME_DIR .. "src/mame/audio/circus.*",
	MAME_DIR .. "src/mame/video/circus.*",
	MAME_DIR .. "src/mame/drivers/exidy.*",
	MAME_DIR .. "src/mame/audio/exidy.*",
	MAME_DIR .. "src/mame/video/exidy.*",
	MAME_DIR .. "src/mame/audio/targ.*",
	MAME_DIR .. "src/mame/drivers/exidy440.*",
	MAME_DIR .. "src/mame/audio/exidy440.*",
	MAME_DIR .. "src/mame/video/exidy440.*",
	MAME_DIR .. "src/mame/drivers/exidyttl.*",
	MAME_DIR .. "src/mame/drivers/maxaflex.*",
	MAME_DIR .. "src/mame/machine/atari.*",
	MAME_DIR .. "src/mame/video/atari.*",
	MAME_DIR .. "src/mame/video/antic.*",
	MAME_DIR .. "src/mame/video/gtia.*",
	MAME_DIR .. "src/mame/drivers/starfire.*",
	MAME_DIR .. "src/mame/video/starfire.*",
	MAME_DIR .. "src/mame/drivers/vertigo.*",
	MAME_DIR .. "src/mame/machine/vertigo.*",
	MAME_DIR .. "src/mame/video/vertigo.*",
	MAME_DIR .. "src/mame/drivers/victory.*",
	MAME_DIR .. "src/mame/video/victory.*",
}

createMAMEProjects(_target, _subtarget, "f32")
files {
	MAME_DIR .. "src/mame/drivers/crospang.*",
	MAME_DIR .. "src/mame/video/crospang.*",
	MAME_DIR .. "src/mame/drivers/silvmil.*",
	MAME_DIR .. "src/mame/drivers/f-32.*",
}

createMAMEProjects(_target, _subtarget, "funworld")
files {
	MAME_DIR .. "src/mame/drivers/4roses.*",
	MAME_DIR .. "src/mame/drivers/funworld.*",
	MAME_DIR .. "src/mame/video/funworld.*",
	MAME_DIR .. "src/mame/drivers/snookr10.*",
	MAME_DIR .. "src/mame/video/snookr10.*",
}

createMAMEProjects(_target, _subtarget, "fuuki")
files {
	MAME_DIR .. "src/mame/drivers/fuukifg2.*",
	MAME_DIR .. "src/mame/video/fuukifg2.*",
	MAME_DIR .. "src/mame/drivers/fuukifg3.*",
	MAME_DIR .. "src/mame/video/fuukifg3.*",
	MAME_DIR .. "src/mame/video/fuukifg.*",
}

createMAMEProjects(_target, _subtarget, "gaelco")
files {
	MAME_DIR .. "src/mame/drivers/atvtrack.*",
	MAME_DIR .. "src/mame/drivers/gaelco.*",
	MAME_DIR .. "src/mame/video/gaelco.*",
	MAME_DIR .. "src/mame/machine/gaelcrpt.*",
	MAME_DIR .. "src/mame/drivers/gaelco2.*",
	MAME_DIR .. "src/mame/machine/gaelco2.*",
	MAME_DIR .. "src/mame/video/gaelco2.*",
	MAME_DIR .. "src/mame/drivers/gaelco3d.*",
	MAME_DIR .. "src/mame/video/gaelco3d.*",
	MAME_DIR .. "src/mame/machine/gaelco3d.*",
	MAME_DIR .. "src/mame/drivers/glass.*",
	MAME_DIR .. "src/mame/video/glass.*",
	MAME_DIR .. "src/mame/drivers/mastboy.*",
	MAME_DIR .. "src/mame/drivers/splash.*",
	MAME_DIR .. "src/mame/video/splash.*",
	MAME_DIR .. "src/mame/drivers/targeth.*",
	MAME_DIR .. "src/mame/video/targeth.*",
	MAME_DIR .. "src/mame/drivers/thoop2.*",
	MAME_DIR .. "src/mame/video/thoop2.*",
	MAME_DIR .. "src/mame/drivers/tokyocop.*",
	MAME_DIR .. "src/mame/drivers/wrally.*",
	MAME_DIR .. "src/mame/machine/wrally.*",
	MAME_DIR .. "src/mame/video/wrally.*",
	MAME_DIR .. "src/mame/drivers/xorworld.*",
	MAME_DIR .. "src/mame/video/xorworld.*",
}

createMAMEProjects(_target, _subtarget, "gameplan")
files {
	MAME_DIR .. "src/mame/drivers/enigma2.*",
	MAME_DIR .. "src/mame/drivers/gameplan.*",
	MAME_DIR .. "src/mame/video/gameplan.*",
	MAME_DIR .. "src/mame/drivers/toratora.*",
}

createMAMEProjects(_target, _subtarget, "gametron")
files {
	MAME_DIR .. "src/mame/drivers/gatron.*",
	MAME_DIR .. "src/mame/video/gatron.*",
	MAME_DIR .. "src/mame/drivers/gotya.*",
	MAME_DIR .. "src/mame/audio/gotya.*",
	MAME_DIR .. "src/mame/video/gotya.*",
	MAME_DIR .. "src/mame/drivers/sbugger.*",
	MAME_DIR .. "src/mame/video/sbugger.*",
}

createMAMEProjects(_target, _subtarget, "gottlieb")
files {
	MAME_DIR .. "src/mame/drivers/exterm.*",
	MAME_DIR .. "src/mame/video/exterm.*",
	MAME_DIR .. "src/mame/drivers/gottlieb.*",
	MAME_DIR .. "src/mame/audio/gottlieb.*",
	MAME_DIR .. "src/mame/video/gottlieb.*",
}

createMAMEProjects(_target, _subtarget, "ibmpc")
files {
	MAME_DIR .. "src/mame/drivers/calchase.*",
	MAME_DIR .. "src/mame/drivers/fruitpc.*",
	MAME_DIR .. "src/mame/drivers/pangofun.*",
	MAME_DIR .. "src/mame/drivers/pcat_dyn.*",
	MAME_DIR .. "src/mame/drivers/pcat_nit.*",
	MAME_DIR .. "src/mame/drivers/pcxt.*",
	MAME_DIR .. "src/mame/drivers/quakeat.*",
	MAME_DIR .. "src/mame/drivers/queen.*",
	MAME_DIR .. "src/mame/drivers/igspc.*",
}

createMAMEProjects(_target, _subtarget, "igs")
files {
	MAME_DIR .. "src/mame/drivers/cabaret.*",
	MAME_DIR .. "src/mame/drivers/ddz.*",
	MAME_DIR .. "src/mame/drivers/dunhuang.*",
	MAME_DIR .. "src/mame/drivers/goldstar.*",
	MAME_DIR .. "src/mame/video/goldstar.*",
	MAME_DIR .. "src/mame/drivers/jackie.*",
	MAME_DIR .. "src/mame/drivers/igspoker.*",
	MAME_DIR .. "src/mame/drivers/igs009.*",
	MAME_DIR .. "src/mame/drivers/igs011.*",
	MAME_DIR .. "src/mame/drivers/igs017.*",
	MAME_DIR .. "src/mame/drivers/igs_m027.*",
	MAME_DIR .. "src/mame/drivers/igs_m036.*",
	MAME_DIR .. "src/mame/drivers/iqblock.*",
	MAME_DIR .. "src/mame/video/iqblock.*",
	MAME_DIR .. "src/mame/drivers/lordgun.*",
	MAME_DIR .. "src/mame/video/lordgun.*",
	MAME_DIR .. "src/mame/drivers/pgm.*",
	MAME_DIR .. "src/mame/video/pgm.*",
	MAME_DIR .. "src/mame/machine/pgmprot_igs027a_type1.*",
	MAME_DIR .. "src/mame/machine/pgmprot_igs027a_type2.*",
	MAME_DIR .. "src/mame/machine/pgmprot_igs027a_type3.*",
	MAME_DIR .. "src/mame/machine/pgmprot_igs025_igs012.*",
	MAME_DIR .. "src/mame/machine/pgmprot_igs025_igs022.*",
	MAME_DIR .. "src/mame/machine/pgmprot_igs025_igs028.*",
	MAME_DIR .. "src/mame/machine/pgmprot_orlegend.*",
	MAME_DIR .. "src/mame/drivers/pgm2.*",
	MAME_DIR .. "src/mame/drivers/spoker.*",
	MAME_DIR .. "src/mame/machine/igs036crypt.*",
	MAME_DIR .. "src/mame/machine/pgmcrypt.*",
	MAME_DIR .. "src/mame/machine/igs025.*",
	MAME_DIR .. "src/mame/machine/igs022.*",
	MAME_DIR .. "src/mame/machine/igs028.*",
}

createMAMEProjects(_target, _subtarget, "irem")
files {
	MAME_DIR .. "src/mame/drivers/m10.*",
	MAME_DIR .. "src/mame/video/m10.*",
	MAME_DIR .. "src/mame/drivers/m14.*",
	MAME_DIR .. "src/mame/drivers/m52.*",
	MAME_DIR .. "src/mame/video/m52.*",
	MAME_DIR .. "src/mame/drivers/m57.*",
	MAME_DIR .. "src/mame/video/m57.*",
	MAME_DIR .. "src/mame/drivers/m58.*",
	MAME_DIR .. "src/mame/video/m58.*",
	MAME_DIR .. "src/mame/drivers/m62.*",
	MAME_DIR .. "src/mame/video/m62.*",
	MAME_DIR .. "src/mame/drivers/m63.*",
	MAME_DIR .. "src/mame/drivers/m72.*",
	MAME_DIR .. "src/mame/audio/m72.*",
	MAME_DIR .. "src/mame/video/m72.*",
	MAME_DIR .. "src/mame/drivers/m90.*",
	MAME_DIR .. "src/mame/video/m90.*",
	MAME_DIR .. "src/mame/drivers/m92.*",
	MAME_DIR .. "src/mame/video/m92.*",
	MAME_DIR .. "src/mame/drivers/m107.*",
	MAME_DIR .. "src/mame/video/m107.*",
	MAME_DIR .. "src/mame/drivers/olibochu.*",
	MAME_DIR .. "src/mame/drivers/redalert.*",
	MAME_DIR .. "src/mame/audio/redalert.*",
	MAME_DIR .. "src/mame/video/redalert.*",
	MAME_DIR .. "src/mame/drivers/shisen.*",
	MAME_DIR .. "src/mame/video/shisen.*",
	MAME_DIR .. "src/mame/drivers/travrusa.*",
	MAME_DIR .. "src/mame/video/travrusa.*",
	MAME_DIR .. "src/mame/drivers/vigilant.*",
	MAME_DIR .. "src/mame/video/vigilant.*",
	MAME_DIR .. "src/mame/machine/irem_cpu.*",
	MAME_DIR .. "src/mame/audio/irem.*",
}

createMAMEProjects(_target, _subtarget, "itech")
files {
	MAME_DIR .. "src/mame/drivers/capbowl.*",
	MAME_DIR .. "src/mame/video/capbowl.*",
	MAME_DIR .. "src/mame/drivers/itech8.*",
	MAME_DIR .. "src/mame/machine/slikshot.*",
	MAME_DIR .. "src/mame/video/itech8.*",
	MAME_DIR .. "src/mame/drivers/itech32.*",
	MAME_DIR .. "src/mame/video/itech32.*",
	MAME_DIR .. "src/mame/drivers/iteagle.*",
	MAME_DIR .. "src/mame/machine/iteagle_fpga.*",
}

createMAMEProjects(_target, _subtarget, "jaleco")
files {
	MAME_DIR .. "src/mame/drivers/aeroboto.*",
	MAME_DIR .. "src/mame/video/aeroboto.*",
	MAME_DIR .. "src/mame/drivers/argus.*",
	MAME_DIR .. "src/mame/video/argus.*",
	MAME_DIR .. "src/mame/drivers/bestleag.*",
	MAME_DIR .. "src/mame/drivers/bigstrkb.*",
	MAME_DIR .. "src/mame/video/bigstrkb.*",
	MAME_DIR .. "src/mame/drivers/blueprnt.*",
	MAME_DIR .. "src/mame/video/blueprnt.*",
	MAME_DIR .. "src/mame/drivers/bnstars.*",
	MAME_DIR .. "src/mame/drivers/cischeat.*",
	MAME_DIR .. "src/mame/video/cischeat.*",
	MAME_DIR .. "src/mame/drivers/citycon.*",
	MAME_DIR .. "src/mame/video/citycon.*",
	MAME_DIR .. "src/mame/drivers/ddayjlc.*",
	MAME_DIR .. "src/mame/drivers/exerion.*",
	MAME_DIR .. "src/mame/video/exerion.*",
	MAME_DIR .. "src/mame/drivers/fcombat.*",
	MAME_DIR .. "src/mame/video/fcombat.*",
	MAME_DIR .. "src/mame/drivers/ginganin.*",
	MAME_DIR .. "src/mame/video/ginganin.*",
	MAME_DIR .. "src/mame/drivers/homerun.*",
	MAME_DIR .. "src/mame/video/homerun.*",
	MAME_DIR .. "src/mame/drivers/megasys1.*",
	MAME_DIR .. "src/mame/video/megasys1.*",
	MAME_DIR .. "src/mame/drivers/momoko.*",
	MAME_DIR .. "src/mame/video/momoko.*",
	MAME_DIR .. "src/mame/drivers/ms32.*",
	MAME_DIR .. "src/mame/video/ms32.*",
	MAME_DIR .. "src/mame/drivers/psychic5.*",
	MAME_DIR .. "src/mame/video/psychic5.*",
	MAME_DIR .. "src/mame/drivers/pturn.*",
	MAME_DIR .. "src/mame/drivers/skyfox.*",
	MAME_DIR .. "src/mame/video/skyfox.*",
	MAME_DIR .. "src/mame/drivers/tetrisp2.*",
	MAME_DIR .. "src/mame/video/tetrisp2.*",
	MAME_DIR .. "src/mame/machine/jalcrpt.*",
	MAME_DIR .. "src/mame/video/jalblend.*",
}

createMAMEProjects(_target, _subtarget, "jpm")
files {
	MAME_DIR .. "src/mame/drivers/guab.*",
	MAME_DIR .. "src/mame/drivers/jpmsys5.*",
	MAME_DIR .. "src/mame/drivers/jpmsys5sw.*",
	MAME_DIR .. "src/mame/drivers/jpmmps.*",
	MAME_DIR .. "src/mame/drivers/jpms80.*",
	MAME_DIR .. "src/mame/drivers/jpmsru.*",
	MAME_DIR .. "src/mame/drivers/jpmimpct.*",
	MAME_DIR .. "src/mame/video/jpmimpct.*",
	MAME_DIR .. "src/mame/drivers/jpmimpctsw.*",
	MAME_DIR .. "src/mame/drivers/pluto5.*",
	MAME_DIR .. "src/mame/drivers/jpmsys7.*",
	MAME_DIR .. "src/mame/video/awpvid.*",
	MAME_DIR .. "src/mame/machine/meters.*",
}

createMAMEProjects(_target, _subtarget, "kaneko")
files {
	MAME_DIR .. "src/mame/drivers/airbustr.*",
	MAME_DIR .. "src/mame/video/airbustr.*",
	MAME_DIR .. "src/mame/drivers/djboy.*",
	MAME_DIR .. "src/mame/video/djboy.*",
	MAME_DIR .. "src/mame/drivers/expro02.*",
	MAME_DIR .. "src/mame/drivers/galpanic.*",
	MAME_DIR .. "src/mame/video/galpanic.*",
	MAME_DIR .. "src/mame/drivers/galpani2.*",
	MAME_DIR .. "src/mame/video/galpani2.*",
	MAME_DIR .. "src/mame/drivers/galpani3.*",
	MAME_DIR .. "src/mame/video/kaneko_grap2.*",
	MAME_DIR .. "src/mame/drivers/hvyunit.*",
	MAME_DIR .. "src/mame/drivers/jchan.*",
	MAME_DIR .. "src/mame/drivers/kaneko16.*",
	MAME_DIR .. "src/mame/video/kaneko16.*",
	MAME_DIR .. "src/mame/video/kaneko_tmap.*",
	MAME_DIR .. "src/mame/video/kaneko_spr.*",
	MAME_DIR .. "src/mame/machine/kaneko_hit.*",
	MAME_DIR .. "src/mame/machine/kaneko_calc3.*",
	MAME_DIR .. "src/mame/machine/kaneko_toybox.*",
	MAME_DIR .. "src/mame/drivers/sandscrp.*",
	MAME_DIR .. "src/mame/drivers/suprnova.*",
	MAME_DIR .. "src/mame/video/suprnova.*",
	MAME_DIR .. "src/mame/video/sknsspr.*",
}

createMAMEProjects(_target, _subtarget, "konami")
files {
	MAME_DIR .. "src/mame/drivers/88games.*",
	MAME_DIR .. "src/mame/video/88games.*",
	MAME_DIR .. "src/mame/drivers/ajax.*",
	MAME_DIR .. "src/mame/machine/ajax.*",
	MAME_DIR .. "src/mame/video/ajax.*",
	MAME_DIR .. "src/mame/drivers/aliens.*",
	MAME_DIR .. "src/mame/video/aliens.*",
	MAME_DIR .. "src/mame/drivers/asterix.*",
	MAME_DIR .. "src/mame/video/asterix.*",
	MAME_DIR .. "src/mame/drivers/battlnts.*",
	MAME_DIR .. "src/mame/video/battlnts.*",
	MAME_DIR .. "src/mame/drivers/bishi.*",
	MAME_DIR .. "src/mame/video/bishi.*",
	MAME_DIR .. "src/mame/drivers/bladestl.*",
	MAME_DIR .. "src/mame/video/bladestl.*",
	MAME_DIR .. "src/mame/drivers/blockhl.*",
	MAME_DIR .. "src/mame/video/blockhl.*",
	MAME_DIR .. "src/mame/drivers/bottom9.*",
	MAME_DIR .. "src/mame/video/bottom9.*",
	MAME_DIR .. "src/mame/drivers/chqflag.*",
	MAME_DIR .. "src/mame/video/chqflag.*",
	MAME_DIR .. "src/mame/drivers/circusc.*",
	MAME_DIR .. "src/mame/video/circusc.*",
	MAME_DIR .. "src/mame/drivers/cobra.*",
	MAME_DIR .. "src/mame/drivers/combatsc.*",
	MAME_DIR .. "src/mame/video/combatsc.*",
	MAME_DIR .. "src/mame/drivers/contra.*",
	MAME_DIR .. "src/mame/video/contra.*",
	MAME_DIR .. "src/mame/drivers/crimfght.*",
	MAME_DIR .. "src/mame/video/crimfght.*",
	MAME_DIR .. "src/mame/drivers/dbz.*",
	MAME_DIR .. "src/mame/video/dbz.*",
	MAME_DIR .. "src/mame/drivers/ddribble.*",
	MAME_DIR .. "src/mame/video/ddribble.*",
	MAME_DIR .. "src/mame/drivers/djmain.*",
	MAME_DIR .. "src/mame/video/djmain.*",
	MAME_DIR .. "src/mame/drivers/fastfred.*",
	MAME_DIR .. "src/mame/video/fastfred.*",
	MAME_DIR .. "src/mame/drivers/fastlane.*",
	MAME_DIR .. "src/mame/video/fastlane.*",
	MAME_DIR .. "src/mame/drivers/finalizr.*",
	MAME_DIR .. "src/mame/video/finalizr.*",
	MAME_DIR .. "src/mame/drivers/firebeat.*",
	MAME_DIR .. "src/mame/machine/midikbd.*",
	MAME_DIR .. "src/mame/drivers/flkatck.*",
	MAME_DIR .. "src/mame/video/flkatck.*",
	MAME_DIR .. "src/mame/drivers/gberet.*",
	MAME_DIR .. "src/mame/video/gberet.*",
	MAME_DIR .. "src/mame/drivers/gijoe.*",
	MAME_DIR .. "src/mame/video/gijoe.*",
	MAME_DIR .. "src/mame/drivers/gradius3.*",
	MAME_DIR .. "src/mame/video/gradius3.*",
	MAME_DIR .. "src/mame/drivers/gticlub.*",
	MAME_DIR .. "src/mame/drivers/gyruss.*",
	MAME_DIR .. "src/mame/video/gyruss.*",
	MAME_DIR .. "src/mame/drivers/hcastle.*",
	MAME_DIR .. "src/mame/video/hcastle.*",
	MAME_DIR .. "src/mame/drivers/hexion.*",
	MAME_DIR .. "src/mame/video/hexion.*",
	MAME_DIR .. "src/mame/drivers/hornet.*",
	MAME_DIR .. "src/mame/machine/konppc.*",
	MAME_DIR .. "src/mame/drivers/hyperspt.*",
	MAME_DIR .. "src/mame/audio/hyprolyb.*",
	MAME_DIR .. "src/mame/video/hyperspt.*",
	MAME_DIR .. "src/mame/drivers/ironhors.*",
	MAME_DIR .. "src/mame/video/ironhors.*",
	MAME_DIR .. "src/mame/drivers/jackal.*",
	MAME_DIR .. "src/mame/video/jackal.*",
	MAME_DIR .. "src/mame/drivers/jailbrek.*",
	MAME_DIR .. "src/mame/video/jailbrek.*",
	MAME_DIR .. "src/mame/drivers/junofrst.*",
	MAME_DIR .. "src/mame/drivers/konamigq.*",
	MAME_DIR .. "src/mame/drivers/konamigv.*",
	MAME_DIR .. "src/mame/drivers/konamigx.*",
	MAME_DIR .. "src/mame/machine/konamigx.*",
	MAME_DIR .. "src/mame/video/konamigx.*",
	MAME_DIR .. "src/mame/drivers/konamim2.*",
	MAME_DIR .. "src/mame/drivers/kontest.*",
	MAME_DIR .. "src/mame/drivers/konendev.*",
	MAME_DIR .. "src/mame/drivers/ksys573.*",
	MAME_DIR .. "src/mame/machine/k573cass.*",
	MAME_DIR .. "src/mame/machine/k573dio.*",
	MAME_DIR .. "src/mame/machine/k573mcr.*",
	MAME_DIR .. "src/mame/machine/k573msu.*",
	MAME_DIR .. "src/mame/machine/k573npu.*",
	MAME_DIR .. "src/mame/machine/zs01.*",
	MAME_DIR .. "src/mame/drivers/labyrunr.*",
	MAME_DIR .. "src/mame/video/labyrunr.*",
	MAME_DIR .. "src/mame/drivers/lethal.*",
	MAME_DIR .. "src/mame/video/lethal.*",
	MAME_DIR .. "src/mame/drivers/mainevt.*",
	MAME_DIR .. "src/mame/video/mainevt.*",
	MAME_DIR .. "src/mame/drivers/megazone.*",
	MAME_DIR .. "src/mame/video/megazone.*",
	MAME_DIR .. "src/mame/drivers/mikie.*",
	MAME_DIR .. "src/mame/video/mikie.*",
	MAME_DIR .. "src/mame/drivers/mogura.*",
	MAME_DIR .. "src/mame/drivers/moo.*",
	MAME_DIR .. "src/mame/video/moo.*",
	MAME_DIR .. "src/mame/drivers/mystwarr.*",
	MAME_DIR .. "src/mame/video/mystwarr.*",
	MAME_DIR .. "src/mame/drivers/nemesis.*",
	MAME_DIR .. "src/mame/video/nemesis.*",
	MAME_DIR .. "src/mame/drivers/nwk-tr.*",
	MAME_DIR .. "src/mame/drivers/overdriv.*",
	MAME_DIR .. "src/mame/video/overdriv.*",
	MAME_DIR .. "src/mame/drivers/pandoras.*",
	MAME_DIR .. "src/mame/video/pandoras.*",
	MAME_DIR .. "src/mame/drivers/parodius.*",
	MAME_DIR .. "src/mame/video/parodius.*",
	MAME_DIR .. "src/mame/drivers/pingpong.*",
	MAME_DIR .. "src/mame/video/pingpong.*",
	MAME_DIR .. "src/mame/drivers/plygonet.*",
	MAME_DIR .. "src/mame/video/plygonet.*",
	MAME_DIR .. "src/mame/drivers/pooyan.*",
	MAME_DIR .. "src/mame/video/pooyan.*",
	MAME_DIR .. "src/mame/drivers/pyson.*",
	MAME_DIR .. "src/mame/drivers/qdrmfgp.*",
	MAME_DIR .. "src/mame/video/qdrmfgp.*",
	MAME_DIR .. "src/mame/drivers/rockrage.*",
	MAME_DIR .. "src/mame/video/rockrage.*",
	MAME_DIR .. "src/mame/drivers/rocnrope.*",
	MAME_DIR .. "src/mame/video/rocnrope.*",
	MAME_DIR .. "src/mame/drivers/rollerg.*",
	MAME_DIR .. "src/mame/video/rollerg.*",
	MAME_DIR .. "src/mame/drivers/rungun.*",
	MAME_DIR .. "src/mame/video/rungun.*",
	MAME_DIR .. "src/mame/drivers/sbasketb.*",
	MAME_DIR .. "src/mame/video/sbasketb.*",
	MAME_DIR .. "src/mame/drivers/scobra.*",
	MAME_DIR .. "src/mame/drivers/scotrsht.*",
	MAME_DIR .. "src/mame/video/scotrsht.*",
	MAME_DIR .. "src/mame/drivers/scramble.*",
	MAME_DIR .. "src/mame/machine/scramble.*",
	MAME_DIR .. "src/mame/audio/scramble.*",
	MAME_DIR .. "src/mame/drivers/shaolins.*",
	MAME_DIR .. "src/mame/video/shaolins.*",
	MAME_DIR .. "src/mame/drivers/simpsons.*",
	MAME_DIR .. "src/mame/machine/simpsons.*",
	MAME_DIR .. "src/mame/video/simpsons.*",
	MAME_DIR .. "src/mame/drivers/spy.*",
	MAME_DIR .. "src/mame/video/spy.*",
	MAME_DIR .. "src/mame/drivers/surpratk.*",
	MAME_DIR .. "src/mame/video/surpratk.*",
	MAME_DIR .. "src/mame/drivers/tasman.*",
	MAME_DIR .. "src/mame/drivers/tgtpanic.*",
	MAME_DIR .. "src/mame/drivers/thunderx.*",
	MAME_DIR .. "src/mame/video/thunderx.*",
	MAME_DIR .. "src/mame/drivers/timeplt.*",
	MAME_DIR .. "src/mame/audio/timeplt.*",
	MAME_DIR .. "src/mame/video/timeplt.*",
	MAME_DIR .. "src/mame/drivers/tmnt.*",
	MAME_DIR .. "src/mame/video/tmnt.*",
	MAME_DIR .. "src/mame/drivers/tp84.*",
	MAME_DIR .. "src/mame/video/tp84.*",
	MAME_DIR .. "src/mame/drivers/trackfld.*",
	MAME_DIR .. "src/mame/machine/konami1.*",
	MAME_DIR .. "src/mame/audio/trackfld.*",
	MAME_DIR .. "src/mame/video/trackfld.*",
	MAME_DIR .. "src/mame/drivers/tutankhm.*",
	MAME_DIR .. "src/mame/video/tutankhm.*",
	MAME_DIR .. "src/mame/drivers/twin16.*",
	MAME_DIR .. "src/mame/video/twin16.*",
	MAME_DIR .. "src/mame/drivers/twinkle.*",
	MAME_DIR .. "src/mame/drivers/ultrsprt.*",
	MAME_DIR .. "src/mame/drivers/ultraman.*",
	MAME_DIR .. "src/mame/video/ultraman.*",
	MAME_DIR .. "src/mame/drivers/vendetta.*",
	MAME_DIR .. "src/mame/video/vendetta.*",
	MAME_DIR .. "src/mame/drivers/viper.*",
	MAME_DIR .. "src/mame/drivers/wecleman.*",
	MAME_DIR .. "src/mame/video/wecleman.*",
	MAME_DIR .. "src/mame/drivers/xexex.*",
	MAME_DIR .. "src/mame/video/xexex.*",
	MAME_DIR .. "src/mame/drivers/xmen.*",
	MAME_DIR .. "src/mame/video/xmen.*",
	MAME_DIR .. "src/mame/drivers/yiear.*",
	MAME_DIR .. "src/mame/video/yiear.*",
	MAME_DIR .. "src/mame/drivers/zr107.*",
	MAME_DIR .. "src/mame/video/konami_helper.*",
	MAME_DIR .. "src/mame/video/k007121.*",
	MAME_DIR .. "src/mame/video/k007342.*",
	MAME_DIR .. "src/mame/video/k007420.*",
	MAME_DIR .. "src/mame/video/k037122.*",
	MAME_DIR .. "src/mame/video/k051316.*",
	MAME_DIR .. "src/mame/video/k051733.*",
	MAME_DIR .. "src/mame/video/k051960.*",
	MAME_DIR .. "src/mame/video/k052109.*",
	MAME_DIR .. "src/mame/video/k053250.*",
	MAME_DIR .. "src/mame/video/k053251.*",
	MAME_DIR .. "src/mame/video/k054156_k054157_k056832.*",
	MAME_DIR .. "src/mame/video/k053244_k053245.*",
	MAME_DIR .. "src/mame/video/k053246_k053247_k055673.*",
	MAME_DIR .. "src/mame/video/k055555.*",
	MAME_DIR .. "src/mame/video/k054000.*",
	MAME_DIR .. "src/mame/video/k054338.*",
	MAME_DIR .. "src/mame/video/k053936.*",
	MAME_DIR .. "src/mame/video/k001006.*",
	MAME_DIR .. "src/mame/video/k001005.*",
	MAME_DIR .. "src/mame/video/k001604.*",
}

createMAMEProjects(_target, _subtarget, "matic")
files {
	MAME_DIR .. "src/mame/drivers/barata.*",
}

createMAMEProjects(_target, _subtarget, "maygay")
files {
	MAME_DIR .. "src/mame/drivers/maygay1b.*",
	MAME_DIR .. "src/mame/drivers/maygay1bsw.*",
	MAME_DIR .. "src/mame/drivers/maygayv1.*",
	MAME_DIR .. "src/mame/drivers/maygayep.*",
	MAME_DIR .. "src/mame/drivers/maygaysw.*",
	MAME_DIR .. "src/mame/drivers/mmm.*",
}

createMAMEProjects(_target, _subtarget, "meadows")
files {
	MAME_DIR .. "src/mame/drivers/lazercmd.*",
	MAME_DIR .. "src/mame/video/lazercmd.*",
	MAME_DIR .. "src/mame/drivers/meadwttl.*",
	MAME_DIR .. "src/mame/drivers/meadows.*",
	MAME_DIR .. "src/mame/audio/meadows.*",
	MAME_DIR .. "src/mame/video/meadows.*",
	MAME_DIR .. "src/mame/drivers/warpsped.*",
}

createMAMEProjects(_target, _subtarget, "merit")
files {
	MAME_DIR .. "src/mame/drivers/mgames.*",
	MAME_DIR .. "src/mame/drivers/merit.*",
	MAME_DIR .. "src/mame/drivers/meritm.*",
}

createMAMEProjects(_target, _subtarget, "metro")
files {
	MAME_DIR .. "src/mame/drivers/hyprduel.*",
	MAME_DIR .. "src/mame/video/hyprduel.*",
	MAME_DIR .. "src/mame/drivers/metro.*",
	MAME_DIR .. "src/mame/video/metro.*",
	MAME_DIR .. "src/mame/drivers/rabbit.*",
	MAME_DIR .. "src/mame/drivers/tmmjprd.*",
}

createMAMEProjects(_target, _subtarget, "midcoin")
files {
	MAME_DIR .. "src/mame/drivers/wallc.*",
	MAME_DIR .. "src/mame/drivers/wink.*",
	MAME_DIR .. "src/mame/drivers/24cdjuke.*",
}

createMAMEProjects(_target, _subtarget, "midw8080")
files {
	MAME_DIR .. "src/mame/drivers/8080bw.*",
	MAME_DIR .. "src/mame/audio/8080bw.*",
	MAME_DIR .. "src/mame/video/8080bw.*",
	MAME_DIR .. "src/mame/drivers/m79amb.*",
	MAME_DIR .. "src/mame/audio/m79amb.*",
	MAME_DIR .. "src/mame/drivers/mw8080bw.*",
	MAME_DIR .. "src/mame/machine/mw8080bw.*",
	MAME_DIR .. "src/mame/audio/mw8080bw.*",
	MAME_DIR .. "src/mame/video/mw8080bw.*",
	MAME_DIR .. "src/mame/drivers/rotaryf.*",
}

createMAMEProjects(_target, _subtarget, "midway")
files {
	MAME_DIR .. "src/mame/drivers/astrocde.*",
	MAME_DIR .. "src/mame/video/astrocde.*",
	MAME_DIR .. "src/mame/audio/gorf.*",
	MAME_DIR .. "src/mame/audio/wow.*",
	MAME_DIR .. "src/mame/drivers/atlantis.*",
	MAME_DIR .. "src/mame/drivers/balsente.*",
	MAME_DIR .. "src/mame/machine/balsente.*",
	MAME_DIR .. "src/mame/video/balsente.*",
	MAME_DIR .. "src/mame/drivers/gridlee.*",
	MAME_DIR .. "src/mame/audio/gridlee.*",
	MAME_DIR .. "src/mame/video/gridlee.*",
	MAME_DIR .. "src/mame/drivers/mcr.*",
	MAME_DIR .. "src/mame/machine/mcr.*",
	MAME_DIR .. "src/mame/video/mcr.*",
	MAME_DIR .. "src/mame/drivers/mcr3.*",
	MAME_DIR .. "src/mame/video/mcr3.*",
	MAME_DIR .. "src/mame/drivers/mcr68.*",
	MAME_DIR .. "src/mame/machine/mcr68.*",
	MAME_DIR .. "src/mame/video/mcr68.*",
	MAME_DIR .. "src/mame/drivers/midqslvr.*",
	MAME_DIR .. "src/mame/drivers/midtunit.*",
	MAME_DIR .. "src/mame/machine/midtunit.*",
	MAME_DIR .. "src/mame/video/midtunit.*",
	MAME_DIR .. "src/mame/drivers/midvunit.*",
	MAME_DIR .. "src/mame/video/midvunit.*",
	MAME_DIR .. "src/mame/drivers/midwunit.*",
	MAME_DIR .. "src/mame/machine/midwunit.*",
	MAME_DIR .. "src/mame/drivers/midxunit.*",
	MAME_DIR .. "src/mame/machine/midxunit.*",
	MAME_DIR .. "src/mame/drivers/midyunit.*",
	MAME_DIR .. "src/mame/machine/midyunit.*",
	MAME_DIR .. "src/mame/video/midyunit.*",
	MAME_DIR .. "src/mame/drivers/midzeus.*",
	MAME_DIR .. "src/mame/video/midzeus.*",
	MAME_DIR .. "src/mame/video/midzeus2.*",
	MAME_DIR .. "src/mame/drivers/mw18w.*",
	MAME_DIR .. "src/mame/drivers/mwsub.*",
	MAME_DIR .. "src/mame/drivers/omegrace.*",
	MAME_DIR .. "src/mame/drivers/pinball2k.*",
	MAME_DIR .. "src/mame/drivers/seattle.*",
	MAME_DIR .. "src/mame/drivers/sspeedr.*",
	MAME_DIR .. "src/mame/video/sspeedr.*",
	MAME_DIR .. "src/mame/drivers/tmaster.*",
	MAME_DIR .. "src/mame/drivers/vegas.*",
	MAME_DIR .. "src/mame/drivers/wmg.*",
	MAME_DIR .. "src/mame/drivers/williams.*",
	MAME_DIR .. "src/mame/machine/williams.*",
	MAME_DIR .. "src/mame/audio/williams.*",
	MAME_DIR .. "src/mame/video/williams.*",
	MAME_DIR .. "src/mame/machine/midwayic.*",
	MAME_DIR .. "src/mame/audio/midway.*",
}

createMAMEProjects(_target, _subtarget, "namco")
files {
	MAME_DIR .. "src/mame/drivers/20pacgal.*",
	MAME_DIR .. "src/mame/video/20pacgal.*",
	MAME_DIR .. "src/mame/drivers/30test.*",
	MAME_DIR .. "src/mame/drivers/baraduke.*",
	MAME_DIR .. "src/mame/video/baraduke.*",
	MAME_DIR .. "src/mame/drivers/cswat.*",
	MAME_DIR .. "src/mame/drivers/dambustr.*",
	MAME_DIR .. "src/mame/drivers/gal3.*",
	MAME_DIR .. "src/mame/drivers/galaga.*",
	MAME_DIR .. "src/mame/audio/galaga.*",
	MAME_DIR .. "src/mame/video/galaga.*",
	MAME_DIR .. "src/mame/video/bosco.*",
	MAME_DIR .. "src/mame/video/digdug.*",
	MAME_DIR .. "src/mame/machine/xevious.*",
	MAME_DIR .. "src/mame/video/xevious.*",
	MAME_DIR .. "src/mame/drivers/galaxian.*",
	MAME_DIR .. "src/mame/audio/galaxian.*",
	MAME_DIR .. "src/mame/video/galaxian.*",
	MAME_DIR .. "src/mame/drivers/galaxold.*",
	MAME_DIR .. "src/mame/machine/galaxold.*",
	MAME_DIR .. "src/mame/video/galaxold.*",
	MAME_DIR .. "src/mame/drivers/gaplus.*",
	MAME_DIR .. "src/mame/machine/gaplus.*",
	MAME_DIR .. "src/mame/video/gaplus.*",
	MAME_DIR .. "src/mame/drivers/kungfur.*",
	MAME_DIR .. "src/mame/drivers/mappy.*",
	MAME_DIR .. "src/mame/video/mappy.*",
	MAME_DIR .. "src/mame/drivers/namcofl.*",
	MAME_DIR .. "src/mame/video/namcofl.*",
	MAME_DIR .. "src/mame/drivers/namcoic.*",
	MAME_DIR .. "src/mame/drivers/namcona1.*",
	MAME_DIR .. "src/mame/video/namcona1.*",
	MAME_DIR .. "src/mame/drivers/namconb1.*",
	MAME_DIR .. "src/mame/video/namconb1.*",
	MAME_DIR .. "src/mame/drivers/namcond1.*",
	MAME_DIR .. "src/mame/machine/namcond1.*",
	MAME_DIR .. "src/mame/video/ygv608.*",
	MAME_DIR .. "src/mame/drivers/namcops2.*",
	MAME_DIR .. "src/mame/drivers/namcos1.*",
	MAME_DIR .. "src/mame/machine/namcos1.*",
	MAME_DIR .. "src/mame/video/namcos1.*",
	MAME_DIR .. "src/mame/drivers/namcos10.*",
	MAME_DIR .. "src/mame/drivers/namcos11.*",
	MAME_DIR .. "src/mame/machine/ns11prot.*",
	MAME_DIR .. "src/mame/drivers/namcos12.*",
	MAME_DIR .. "src/mame/machine/namco_settings.*",
	MAME_DIR .. "src/mame/drivers/namcos2.*",
	MAME_DIR .. "src/mame/machine/namcos2.*",
	MAME_DIR .. "src/mame/video/namcos2.*",
	MAME_DIR .. "src/mame/drivers/namcos21.*",
	MAME_DIR .. "src/mame/video/namcos21.*",
	MAME_DIR .. "src/mame/drivers/namcos22.*",
	MAME_DIR .. "src/mame/video/namcos22.*",
	MAME_DIR .. "src/mame/drivers/namcos23.*",
	MAME_DIR .. "src/mame/drivers/namcos86.*",
	MAME_DIR .. "src/mame/video/namcos86.*",
	MAME_DIR .. "src/mame/drivers/pacland.*",
	MAME_DIR .. "src/mame/video/pacland.*",
	MAME_DIR .. "src/mame/drivers/polepos.*",
	MAME_DIR .. "src/mame/audio/polepos.*",
	MAME_DIR .. "src/mame/video/polepos.*",
	MAME_DIR .. "src/mame/drivers/rallyx.*",
	MAME_DIR .. "src/mame/video/rallyx.*",
	MAME_DIR .. "src/mame/drivers/skykid.*",
	MAME_DIR .. "src/mame/video/skykid.*",
	MAME_DIR .. "src/mame/drivers/tankbatt.*",
	MAME_DIR .. "src/mame/video/tankbatt.*",
	MAME_DIR .. "src/mame/drivers/tceptor.*",
	MAME_DIR .. "src/mame/video/tceptor.*",
	MAME_DIR .. "src/mame/drivers/toypop.*",
	MAME_DIR .. "src/mame/video/toypop.*",
	MAME_DIR .. "src/mame/drivers/turrett.*",
	MAME_DIR .. "src/mame/audio/turrett.*",
	MAME_DIR .. "src/mame/video/turrett.*",
	MAME_DIR .. "src/mame/drivers/warpwarp.*",
	MAME_DIR .. "src/mame/audio/geebee.*",
	MAME_DIR .. "src/mame/audio/warpwarp.*",
	MAME_DIR .. "src/mame/video/warpwarp.*",
	MAME_DIR .. "src/mame/machine/c117.*",
	MAME_DIR .. "src/mame/machine/namcoio.*",
	MAME_DIR .. "src/mame/machine/namco06.*",
	MAME_DIR .. "src/mame/machine/namco50.*",
	MAME_DIR .. "src/mame/machine/namco51.*",
	MAME_DIR .. "src/mame/machine/namco53.*",
	MAME_DIR .. "src/mame/machine/namco62.*",
	MAME_DIR .. "src/mame/machine/namcomcu.*",
	MAME_DIR .. "src/mame/audio/namco52.*",
	MAME_DIR .. "src/mame/audio/namco54.*",
	MAME_DIR .. "src/mame/video/c116.*",
	MAME_DIR .. "src/mame/video/c45.*",
}

createMAMEProjects(_target, _subtarget, "nasco")
files {
	MAME_DIR .. "src/mame/drivers/crgolf.*",
	MAME_DIR .. "src/mame/video/crgolf.*",
	MAME_DIR .. "src/mame/drivers/suprgolf.*",
}

createMAMEProjects(_target, _subtarget, "neogeo")
files {
	MAME_DIR .. "src/mame/drivers/neogeo.*",
	MAME_DIR .. "src/mame/video/neogeo.*",
	MAME_DIR .. "src/mame/drivers/neogeo_noslot.*",
	MAME_DIR .. "src/mame/video/neogeo_spr.*",
	MAME_DIR .. "src/mame/machine/neoboot.*",
	MAME_DIR .. "src/mame/machine/neocrypt.*",
	MAME_DIR .. "src/mame/machine/neoprot.*",
	MAME_DIR .. "src/mame/machine/ng_memcard.*",
}

createMAMEProjects(_target, _subtarget, "nichibut")
files {
	MAME_DIR .. "src/mame/drivers/armedf.*",
	MAME_DIR .. "src/mame/video/armedf.*",
	MAME_DIR .. "src/mame/drivers/cclimber.*",
	MAME_DIR .. "src/mame/machine/cclimber.*",
	MAME_DIR .. "src/mame/audio/cclimber.*",
	MAME_DIR .. "src/mame/video/cclimber.*",
	MAME_DIR .. "src/mame/drivers/clshroad.*",
	MAME_DIR .. "src/mame/video/clshroad.*",
	MAME_DIR .. "src/mame/drivers/csplayh5.*",
	MAME_DIR .. "src/mame/drivers/cop01.*",
	MAME_DIR .. "src/mame/video/cop01.*",
	MAME_DIR .. "src/mame/drivers/dacholer.*",
	MAME_DIR .. "src/mame/drivers/galivan.*",
	MAME_DIR .. "src/mame/video/galivan.*",
	MAME_DIR .. "src/mame/drivers/gomoku.*",
	MAME_DIR .. "src/mame/audio/gomoku.*",
	MAME_DIR .. "src/mame/video/gomoku.*",
	MAME_DIR .. "src/mame/drivers/hyhoo.*",
	MAME_DIR .. "src/mame/video/hyhoo.*",
	MAME_DIR .. "src/mame/drivers/jangou.*",
	MAME_DIR .. "src/mame/drivers/magmax.*",
	MAME_DIR .. "src/mame/video/magmax.*",
	MAME_DIR .. "src/mame/drivers/nbmj8688.*",
	MAME_DIR .. "src/mame/video/nbmj8688.*",
	MAME_DIR .. "src/mame/drivers/nbmj8891.*",
	MAME_DIR .. "src/mame/video/nbmj8891.*",
	MAME_DIR .. "src/mame/drivers/nbmj8900.*",
	MAME_DIR .. "src/mame/video/nbmj8900.*",
	MAME_DIR .. "src/mame/drivers/nbmj8991.*",
	MAME_DIR .. "src/mame/video/nbmj8991.*",
	MAME_DIR .. "src/mame/drivers/nbmj9195.*",
	MAME_DIR .. "src/mame/video/nbmj9195.*",
	MAME_DIR .. "src/mame/drivers/nightgal.*",
	MAME_DIR .. "src/mame/drivers/niyanpai.*",
	MAME_DIR .. "src/mame/video/niyanpai.*",
	MAME_DIR .. "src/mame/drivers/pastelg.*",
	MAME_DIR .. "src/mame/video/pastelg.*",
	MAME_DIR .. "src/mame/drivers/seicross.*",
	MAME_DIR .. "src/mame/video/seicross.*",
	MAME_DIR .. "src/mame/drivers/terracre.*",
	MAME_DIR .. "src/mame/video/terracre.*",
	MAME_DIR .. "src/mame/drivers/tubep.*",
	MAME_DIR .. "src/mame/video/tubep.*",
	MAME_DIR .. "src/mame/drivers/wiping.*",
	MAME_DIR .. "src/mame/audio/wiping.*",
	MAME_DIR .. "src/mame/video/wiping.*",
	MAME_DIR .. "src/mame/machine/nb1413m3.*",
	MAME_DIR .. "src/mame/machine/nb1414m4.*",
}

createMAMEProjects(_target, _subtarget, "nintendo")
files {
	MAME_DIR .. "src/mame/drivers/cham24.*",
	MAME_DIR .. "src/mame/drivers/dkong.*",
	MAME_DIR .. "src/mame/audio/dkong.*",
	MAME_DIR .. "src/mame/video/dkong.*",
	MAME_DIR .. "src/mame/drivers/mario.*",
	MAME_DIR .. "src/mame/audio/mario.*",
	MAME_DIR .. "src/mame/video/mario.*",
	MAME_DIR .. "src/mame/drivers/multigam.*",
	MAME_DIR .. "src/mame/drivers/n8080.*",
	MAME_DIR .. "src/mame/audio/n8080.*",
	MAME_DIR .. "src/mame/video/n8080.*",
	MAME_DIR .. "src/mame/drivers/nss.*",
	MAME_DIR .. "src/mame/machine/snes.*",
	MAME_DIR .. "src/mame/audio/snes_snd.*",
	MAME_DIR .. "src/mame/drivers/playch10.*",
	MAME_DIR .. "src/mame/machine/playch10.*",
	MAME_DIR .. "src/mame/video/playch10.*",
	MAME_DIR .. "src/mame/drivers/popeye.*",
	MAME_DIR .. "src/mame/video/popeye.*",
	MAME_DIR .. "src/mame/drivers/punchout.*",
	MAME_DIR .. "src/mame/video/punchout.*",
	MAME_DIR .. "src/mame/drivers/famibox.*",
	MAME_DIR .. "src/mame/drivers/sfcbox.*",
	MAME_DIR .. "src/mame/drivers/snesb.*",
	MAME_DIR .. "src/mame/drivers/spacefb.*",
	MAME_DIR .. "src/mame/audio/spacefb.*",
	MAME_DIR .. "src/mame/video/spacefb.*",
	MAME_DIR .. "src/mame/drivers/vsnes.*",
	MAME_DIR .. "src/mame/machine/vsnes.*",
	MAME_DIR .. "src/mame/video/vsnes.*",
	MAME_DIR .. "src/mame/video/ppu2c0x.*",


}

createMAMEProjects(_target, _subtarget, "nix")
files {
	MAME_DIR .. "src/mame/drivers/fitfight.*",
	MAME_DIR .. "src/mame/video/fitfight.*",
	MAME_DIR .. "src/mame/drivers/pirates.*",
	MAME_DIR .. "src/mame/video/pirates.*",
}

createMAMEProjects(_target, _subtarget, "nmk")
files {
	MAME_DIR .. "src/mame/drivers/acommand.*",
	MAME_DIR .. "src/mame/drivers/cultures.*",
	MAME_DIR .. "src/mame/drivers/ddealer.*",
	MAME_DIR .. "src/mame/drivers/jalmah.*",
	MAME_DIR .. "src/mame/drivers/macrossp.*",
	MAME_DIR .. "src/mame/video/macrossp.*",
	MAME_DIR .. "src/mame/drivers/nmk16.*",
	MAME_DIR .. "src/mame/machine/nmk004.*",
	MAME_DIR .. "src/mame/video/nmk16.*",
	MAME_DIR .. "src/mame/drivers/quizdna.*",
	MAME_DIR .. "src/mame/video/quizdna.*",
	MAME_DIR .. "src/mame/drivers/quizpani.*",
	MAME_DIR .. "src/mame/video/quizpani.*",
}

createMAMEProjects(_target, _subtarget, "olympia")
files {
	MAME_DIR .. "src/mame/drivers/dday.*",
	MAME_DIR .. "src/mame/video/dday.*",
	MAME_DIR .. "src/mame/drivers/lbeach.*",
	MAME_DIR .. "src/mame/drivers/monzagp.*",
	MAME_DIR .. "src/mame/drivers/portrait.*",
	MAME_DIR .. "src/mame/video/portrait.*",
	MAME_DIR .. "src/mame/drivers/vega.*",
}

createMAMEProjects(_target, _subtarget, "omori")
files {
	MAME_DIR .. "src/mame/drivers/battlex.*",
	MAME_DIR .. "src/mame/video/battlex.*",
	MAME_DIR .. "src/mame/drivers/carjmbre.*",
	MAME_DIR .. "src/mame/video/carjmbre.*",
	MAME_DIR .. "src/mame/drivers/popper.*",
	MAME_DIR .. "src/mame/video/popper.*",
	MAME_DIR .. "src/mame/drivers/spaceg.*",
}

createMAMEProjects(_target, _subtarget, "orca")
files {
	MAME_DIR .. "src/mame/drivers/espial.*",
	MAME_DIR .. "src/mame/video/espial.*",
	MAME_DIR .. "src/mame/drivers/funkybee.*",
	MAME_DIR .. "src/mame/video/funkybee.*",
	MAME_DIR .. "src/mame/drivers/marineb.*",
	MAME_DIR .. "src/mame/video/marineb.*",
	MAME_DIR .. "src/mame/drivers/vastar.*",
	MAME_DIR .. "src/mame/video/vastar.*",
	MAME_DIR .. "src/mame/drivers/zodiack.*",
	MAME_DIR .. "src/mame/video/zodiack.*",
}

createMAMEProjects(_target, _subtarget, "pacific")
files {
	MAME_DIR .. "src/mame/drivers/mrflea.*",
	MAME_DIR .. "src/mame/video/mrflea.*",
	MAME_DIR .. "src/mame/drivers/thief.*",
	MAME_DIR .. "src/mame/video/thief.*",
}

createMAMEProjects(_target, _subtarget, "pacman")
files {
	MAME_DIR .. "src/mame/drivers/jrpacman.*",
	MAME_DIR .. "src/mame/drivers/pacman.*",
	MAME_DIR .. "src/mame/video/pacman.*",
	MAME_DIR .. "src/mame/machine/acitya.*",
	MAME_DIR .. "src/mame/machine/jumpshot.*",
	MAME_DIR .. "src/mame/machine/pacplus.*",
	MAME_DIR .. "src/mame/machine/theglobp.*",
	MAME_DIR .. "src/mame/drivers/pengo.*",
}

createMAMEProjects(_target, _subtarget, "pce")
files {
	MAME_DIR .. "src/mame/drivers/ggconnie.*",
	MAME_DIR .. "src/mame/drivers/paranoia.*",
	MAME_DIR .. "src/mame/drivers/tourvis.*",
	MAME_DIR .. "src/mame/drivers/uapce.*",
	MAME_DIR .. "src/mame/machine/pcecommn.*",
}

createMAMEProjects(_target, _subtarget, "phoenix")
files {
	MAME_DIR .. "src/mame/drivers/naughtyb.*",
	MAME_DIR .. "src/mame/video/naughtyb.*",
	MAME_DIR .. "src/mame/drivers/phoenix.*",
	MAME_DIR .. "src/mame/audio/phoenix.*",
	MAME_DIR .. "src/mame/video/phoenix.*",
	MAME_DIR .. "src/mame/drivers/safarir.*",
	MAME_DIR .. "src/mame/audio/pleiads.*",
}

createMAMEProjects(_target, _subtarget, "playmark")
files {
	MAME_DIR .. "src/mame/drivers/drtomy.*",
	MAME_DIR .. "src/mame/drivers/playmark.*",
	MAME_DIR .. "src/mame/video/playmark.*",
	MAME_DIR .. "src/mame/drivers/powerbal.*",
	MAME_DIR .. "src/mame/drivers/sderby.*",
	MAME_DIR .. "src/mame/video/sderby.*",
	MAME_DIR .. "src/mame/drivers/sslam.*",
	MAME_DIR .. "src/mame/video/sslam.*",
}

createMAMEProjects(_target, _subtarget, "psikyo")
files {
	MAME_DIR .. "src/mame/drivers/psikyo.*",
	MAME_DIR .. "src/mame/video/psikyo.*",
	MAME_DIR .. "src/mame/drivers/psikyo4.*",
	MAME_DIR .. "src/mame/video/psikyo4.*",
	MAME_DIR .. "src/mame/drivers/psikyosh.*",
	MAME_DIR .. "src/mame/video/psikyosh.*",
}

createMAMEProjects(_target, _subtarget, "ramtek")
files {
	MAME_DIR .. "src/mame/drivers/hitme.*",
	MAME_DIR .. "src/mame/audio/hitme.*",
	MAME_DIR .. "src/mame/drivers/ramtek.*",
	MAME_DIR .. "src/mame/drivers/starcrus.*",
	MAME_DIR .. "src/mame/video/starcrus.*",
}

createMAMEProjects(_target, _subtarget, "rare")
files {
	MAME_DIR .. "src/mame/drivers/btoads.*",
	MAME_DIR .. "src/mame/video/btoads.*",
	MAME_DIR .. "src/mame/drivers/kinst.*",
	MAME_DIR .. "src/mame/drivers/xtheball.*",
}

createMAMEProjects(_target, _subtarget, "sanritsu")
files {
	MAME_DIR .. "src/mame/drivers/appoooh.*",
	MAME_DIR .. "src/mame/video/appoooh.*",
	MAME_DIR .. "src/mame/drivers/bankp.*",
	MAME_DIR .. "src/mame/video/bankp.*",
	MAME_DIR .. "src/mame/drivers/chinsan.*",
	MAME_DIR .. "src/mame/drivers/drmicro.*",
	MAME_DIR .. "src/mame/video/drmicro.*",
	MAME_DIR .. "src/mame/drivers/jantotsu.*",
	MAME_DIR .. "src/mame/drivers/mayumi.*",
	MAME_DIR .. "src/mame/drivers/mermaid.*",
	MAME_DIR .. "src/mame/video/mermaid.*",
	MAME_DIR .. "src/mame/drivers/mjkjidai.*",
	MAME_DIR .. "src/mame/video/mjkjidai.*",
}

createMAMEProjects(_target, _subtarget, "sega")
files {
	MAME_DIR .. "src/mame/drivers/angelkds.*",
	MAME_DIR .. "src/mame/video/angelkds.*",
	MAME_DIR .. "src/mame/drivers/bingoc.*",
	MAME_DIR .. "src/mame/drivers/blockade.*",
	MAME_DIR .. "src/mame/audio/blockade.*",
	MAME_DIR .. "src/mame/video/blockade.*",
	MAME_DIR .. "src/mame/drivers/calorie.*",
	MAME_DIR .. "src/mame/drivers/chihiro.*",
	MAME_DIR .. "src/mame/video/chihiro.*", 
	MAME_DIR .. "src/mame/drivers/coolridr.*",
	MAME_DIR .. "src/mame/drivers/deniam.*",
	MAME_DIR .. "src/mame/video/deniam.*",
	MAME_DIR .. "src/mame/drivers/dotrikun.*",
	MAME_DIR .. "src/mame/drivers/gpworld.*",
	MAME_DIR .. "src/mame/drivers/hikaru.*",
	MAME_DIR .. "src/mame/drivers/hshavoc.*",
	MAME_DIR .. "src/mame/drivers/kopunch.*",
	MAME_DIR .. "src/mame/video/kopunch.*",
	MAME_DIR .. "src/mame/drivers/lindbergh.*",
	MAME_DIR .. "src/mame/machine/segabb.*",
	MAME_DIR .. "src/mame/machine/megadriv.*",
	MAME_DIR .. "src/mame/drivers/megadrvb.*",
	MAME_DIR .. "src/mame/drivers/megaplay.*",
	MAME_DIR .. "src/mame/drivers/megatech.*",
	MAME_DIR .. "src/mame/drivers/model1.*",
	MAME_DIR .. "src/mame/machine/model1.*",
	MAME_DIR .. "src/mame/video/model1.*",
	MAME_DIR .. "src/mame/audio/dsbz80.*",
	MAME_DIR .. "src/mame/drivers/model2.*",
	MAME_DIR .. "src/mame/video/model2.*",
	MAME_DIR .. "src/mame/drivers/model3.*",
	MAME_DIR .. "src/mame/video/model3.*",
	MAME_DIR .. "src/mame/machine/model3.*",
	MAME_DIR .. "src/mame/drivers/monacogp.*",
	MAME_DIR .. "src/mame/drivers/naomi.*",
	MAME_DIR .. "src/mame/machine/dc.*",
	MAME_DIR .. "src/mame/video/powervr2.*",
	MAME_DIR .. "src/mame/machine/naomi.*",
	MAME_DIR .. "src/mame/machine/naomig1.*",
	MAME_DIR .. "src/mame/machine/naomibd.*",
	MAME_DIR .. "src/mame/machine/naomirom.*",
	MAME_DIR .. "src/mame/machine/naomigd.*",
	MAME_DIR .. "src/mame/machine/naomim1.*",
	MAME_DIR .. "src/mame/machine/naomim2.*",
	MAME_DIR .. "src/mame/machine/naomim4.*",
	MAME_DIR .. "src/mame/machine/315-5881_crypt.*",
	MAME_DIR .. "src/mame/machine/awboard.*",
	MAME_DIR .. "src/mame/machine/mie.*",
	MAME_DIR .. "src/mame/machine/maple-dc.*",
	MAME_DIR .. "src/mame/machine/mapledev.*",
	MAME_DIR .. "src/mame/machine/dc-ctrl.*",
	MAME_DIR .. "src/mame/machine/jvs13551.*",
	MAME_DIR .. "src/mame/drivers/triforce.*",
	MAME_DIR .. "src/mame/drivers/puckpkmn.*",
	MAME_DIR .. "src/mame/drivers/segac2.*",
	MAME_DIR .. "src/mame/drivers/segae.*",
	MAME_DIR .. "src/mame/drivers/shtzone.*",
	MAME_DIR .. "src/mame/drivers/segacoin.*",
	MAME_DIR .. "src/mame/drivers/segag80r.*",
	MAME_DIR .. "src/mame/machine/segag80.*",
	MAME_DIR .. "src/mame/audio/segag80r.*",
	MAME_DIR .. "src/mame/video/segag80r.*",
	MAME_DIR .. "src/mame/drivers/segag80v.*",
	MAME_DIR .. "src/mame/audio/segag80v.*",
	MAME_DIR .. "src/mame/video/segag80v.*",
	MAME_DIR .. "src/mame/drivers/segahang.*",
	MAME_DIR .. "src/mame/video/segahang.*",
	MAME_DIR .. "src/mame/drivers/segajw.*",
	MAME_DIR .. "src/mame/drivers/segald.*",
	MAME_DIR .. "src/mame/drivers/segaorun.*",
	MAME_DIR .. "src/mame/video/segaorun.*",
	MAME_DIR .. "src/mame/drivers/segas16a.*",
	MAME_DIR .. "src/mame/video/segas16a.*",
	MAME_DIR .. "src/mame/drivers/segas16b.*",
	MAME_DIR .. "src/mame/video/segas16b.*",
	MAME_DIR .. "src/mame/drivers/segas18.*",
	MAME_DIR .. "src/mame/video/segas18.*",
	MAME_DIR .. "src/mame/drivers/segas24.*",
	MAME_DIR .. "src/mame/video/segas24.*",
	MAME_DIR .. "src/mame/drivers/segas32.*",
	MAME_DIR .. "src/mame/machine/segas32.*",
	MAME_DIR .. "src/mame/video/segas32.*",
	MAME_DIR .. "src/mame/drivers/segaufo.*",
	MAME_DIR .. "src/mame/drivers/segaxbd.*",
	MAME_DIR .. "src/mame/video/segaxbd.*",
	MAME_DIR .. "src/mame/drivers/segaybd.*",
	MAME_DIR .. "src/mame/video/segaybd.*",
	MAME_DIR .. "src/mame/drivers/sg1000a.*",
	MAME_DIR .. "src/mame/drivers/stactics.*",
	MAME_DIR .. "src/mame/video/stactics.*",
	MAME_DIR .. "src/mame/drivers/stv.*",
	MAME_DIR .. "src/mame/machine/stvprot.*",
	MAME_DIR .. "src/mame/machine/315-5838_317-0229_comp.*",
	MAME_DIR .. "src/mame/drivers/suprloco.*",
	MAME_DIR .. "src/mame/video/suprloco.*",
	MAME_DIR .. "src/mame/drivers/system1.*",
	MAME_DIR .. "src/mame/video/system1.*",
	MAME_DIR .. "src/mame/drivers/system16.*",
	MAME_DIR .. "src/mame/video/system16.*",
	MAME_DIR .. "src/mame/drivers/timetrv.*",
	MAME_DIR .. "src/mame/drivers/turbo.*",
	MAME_DIR .. "src/mame/audio/turbo.*",
	MAME_DIR .. "src/mame/video/turbo.*",
	MAME_DIR .. "src/mame/drivers/vicdual.*",
	MAME_DIR .. "src/mame/audio/vicdual.*",
	MAME_DIR .. "src/mame/video/vicdual.*",
	MAME_DIR .. "src/mame/audio/carnival.*",
	MAME_DIR .. "src/mame/audio/depthch.*",
	MAME_DIR .. "src/mame/audio/invinco.*",
	MAME_DIR .. "src/mame/audio/pulsar.*",
	MAME_DIR .. "src/mame/drivers/zaxxon.*",
	MAME_DIR .. "src/mame/audio/zaxxon.*",
	MAME_DIR .. "src/mame/video/zaxxon.*",
	MAME_DIR .. "src/mame/machine/315_5296.*",
	MAME_DIR .. "src/mame/machine/fd1089.*",
	MAME_DIR .. "src/mame/machine/fd1094.*",
	MAME_DIR .. "src/mame/machine/fddebug.*",
	MAME_DIR .. "src/mame/machine/mc8123.*",
	MAME_DIR .. "src/mame/machine/segaic16.*",
	MAME_DIR .. "src/mame/audio/segasnd.*",
	MAME_DIR .. "src/mame/video/segaic16.*",
	MAME_DIR .. "src/mame/video/segaic16_road.*",
	MAME_DIR .. "src/mame/video/sega16sp.*",
	MAME_DIR .. "src/mame/video/segaic24.*",
	MAME_DIR .. "src/mame/machine/gdrom.*",
}

createMAMEProjects(_target, _subtarget, "seibu")
files {
	MAME_DIR .. "src/mame/drivers/bloodbro.*",
	MAME_DIR .. "src/mame/video/bloodbro.*",
	MAME_DIR .. "src/mame/drivers/cabal.*",
	MAME_DIR .. "src/mame/video/cabal.*",
	MAME_DIR .. "src/mame/drivers/cshooter.*",
	MAME_DIR .. "src/mame/drivers/dcon.*",
	MAME_DIR .. "src/mame/video/dcon.*",
	MAME_DIR .. "src/mame/drivers/deadang.*",
	MAME_DIR .. "src/mame/video/deadang.*",
	MAME_DIR .. "src/mame/drivers/dynduke.*",
	MAME_DIR .. "src/mame/video/dynduke.*",
	MAME_DIR .. "src/mame/drivers/feversoc.*",
	MAME_DIR .. "src/mame/drivers/goal92.*",
	MAME_DIR .. "src/mame/video/goal92.*",
	MAME_DIR .. "src/mame/drivers/goodejan.*",
	MAME_DIR .. "src/mame/drivers/kncljoe.*",
	MAME_DIR .. "src/mame/video/kncljoe.*",
	MAME_DIR .. "src/mame/drivers/legionna.*",
	MAME_DIR .. "src/mame/video/legionna.*",
	MAME_DIR .. "src/mame/drivers/mustache.*",
	MAME_DIR .. "src/mame/video/mustache.*",
	MAME_DIR .. "src/mame/drivers/panicr.*",
	MAME_DIR .. "src/mame/drivers/raiden.*",
	MAME_DIR .. "src/mame/video/raiden.*",
	MAME_DIR .. "src/mame/drivers/raiden2.*",
	MAME_DIR .. "src/mame/machine/r2crypt.*",
	MAME_DIR .. "src/mame/machine/raiden2cop.*",
	MAME_DIR .. "src/mame/drivers/r2dx_v33.*",
	MAME_DIR .. "src/mame/drivers/seibuspi.*",
	MAME_DIR .. "src/mame/machine/seibuspi.*",
	MAME_DIR .. "src/mame/video/seibuspi.*",
	MAME_DIR .. "src/mame/drivers/sengokmj.*",
	MAME_DIR .. "src/mame/drivers/stfight.*",
	MAME_DIR .. "src/mame/machine/stfight.*",
	MAME_DIR .. "src/mame/video/stfight.*",
	MAME_DIR .. "src/mame/drivers/toki.*",
	MAME_DIR .. "src/mame/video/toki.*",
	MAME_DIR .. "src/mame/drivers/wiz.*",
	MAME_DIR .. "src/mame/video/wiz.*",
	MAME_DIR .. "src/mame/machine/seicop.*",
	MAME_DIR .. "src/mame/machine/spisprit.*",
	MAME_DIR .. "src/mame/audio/seibu.*",
	MAME_DIR .. "src/mame/video/seibu_crtc.*",
}

createMAMEProjects(_target, _subtarget, "seta")
files {
	MAME_DIR .. "src/mame/drivers/aleck64.*",
	MAME_DIR .. "src/mame/machine/n64.*",
	MAME_DIR .. "src/mame/video/n64.*",
	MAME_DIR .. "src/mame/video/rdpblend.*",
	MAME_DIR .. "src/mame/video/rdpspn16.*",
	MAME_DIR .. "src/mame/video/rdptpipe.*",
	MAME_DIR .. "src/mame/drivers/hanaawas.*",
	MAME_DIR .. "src/mame/video/hanaawas.*",
	MAME_DIR .. "src/mame/drivers/jclub2.*",
	MAME_DIR .. "src/mame/drivers/macs.*",
	MAME_DIR .. "src/mame/drivers/seta.*",
	MAME_DIR .. "src/mame/video/seta.*",
	MAME_DIR .. "src/mame/drivers/seta2.*",
	MAME_DIR .. "src/mame/video/seta2.*",
	MAME_DIR .. "src/mame/drivers/speedatk.*",
	MAME_DIR .. "src/mame/video/speedatk.*",
	MAME_DIR .. "src/mame/drivers/speglsht.*",
	MAME_DIR .. "src/mame/drivers/srmp2.*",
	MAME_DIR .. "src/mame/video/srmp2.*",
	MAME_DIR .. "src/mame/drivers/srmp5.*",
	MAME_DIR .. "src/mame/drivers/srmp6.*",
	MAME_DIR .. "src/mame/drivers/ssv.*",
	MAME_DIR .. "src/mame/video/ssv.*",
	MAME_DIR .. "src/mame/video/st0020.*",
	MAME_DIR .. "src/mame/machine/st0016.*",
	MAME_DIR .. "src/mame/drivers/simple_st0016.*",
	MAME_DIR .. "src/mame/video/seta001.*",
}

createMAMEProjects(_target, _subtarget, "sigma")
files {
	MAME_DIR .. "src/mame/drivers/nyny.*",
	MAME_DIR .. "src/mame/drivers/r2dtank.*",
	MAME_DIR .. "src/mame/drivers/sigmab52.*",
	MAME_DIR .. "src/mame/drivers/sigmab98.*",
	MAME_DIR .. "src/mame/drivers/spiders.*",
	MAME_DIR .. "src/mame/audio/spiders.*",
	MAME_DIR .. "src/mame/drivers/sub.*",
}

createMAMEProjects(_target, _subtarget, "snk")
files {
	MAME_DIR .. "src/mame/drivers/bbusters.*",
	MAME_DIR .. "src/mame/video/bbusters.*",
	MAME_DIR .. "src/mame/drivers/dmndrby.*",
	MAME_DIR .. "src/mame/drivers/hng64.*",
	MAME_DIR .. "src/mame/video/hng64.*",
	MAME_DIR .. "src/mame/audio/hng64.*",
	MAME_DIR .. "src/mame/machine/hng64_net.*",
	MAME_DIR .. "src/mame/video/hng64_3d.*",
	MAME_DIR .. "src/mame/video/hng64_sprite.*",
	MAME_DIR .. "src/mame/drivers/lasso.*",
	MAME_DIR .. "src/mame/video/lasso.*",
	MAME_DIR .. "src/mame/drivers/mainsnk.*",
	MAME_DIR .. "src/mame/video/mainsnk.*",
	MAME_DIR .. "src/mame/drivers/munchmo.*",
	MAME_DIR .. "src/mame/video/munchmo.*",
	MAME_DIR .. "src/mame/drivers/prehisle.*",
	MAME_DIR .. "src/mame/video/prehisle.*",
	MAME_DIR .. "src/mame/drivers/snk6502.*",
	MAME_DIR .. "src/mame/audio/snk6502.*",
	MAME_DIR .. "src/mame/video/snk6502.*",
	MAME_DIR .. "src/mame/drivers/snk.*",
	MAME_DIR .. "src/mame/video/snk.*",
	MAME_DIR .. "src/mame/drivers/snk68.*",
	MAME_DIR .. "src/mame/video/snk68.*",
}

createMAMEProjects(_target, _subtarget, "sony")
files {
	MAME_DIR .. "src/mame/drivers/zn.*",
	MAME_DIR .. "src/mame/machine/zndip.*",
	MAME_DIR .. "src/mame/machine/cat702.*",
}

createMAMEProjects(_target, _subtarget, "stern")
files {
	MAME_DIR .. "src/mame/drivers/astinvad.*",
	MAME_DIR .. "src/mame/drivers/berzerk.*",
	MAME_DIR .. "src/mame/drivers/cliffhgr.*",
	MAME_DIR .. "src/mame/audio/cliffhgr.*",
	MAME_DIR .. "src/mame/drivers/mazerbla.*",
	MAME_DIR .. "src/mame/drivers/supdrapo.*",
}

createMAMEProjects(_target, _subtarget, "subsino")
files {
	MAME_DIR .. "src/mame/drivers/lastfght.*",
	MAME_DIR .. "src/mame/drivers/subsino.*",
	MAME_DIR .. "src/mame/drivers/subsino2.*",
	MAME_DIR .. "src/mame/machine/subsino.*",
}

createMAMEProjects(_target, _subtarget, "sun")
files {
	MAME_DIR .. "src/mame/drivers/arabian.*",
	MAME_DIR .. "src/mame/video/arabian.*",
	MAME_DIR .. "src/mame/drivers/dai3wksi.*",
	MAME_DIR .. "src/mame/drivers/ikki.*",
	MAME_DIR .. "src/mame/video/ikki.*",
	MAME_DIR .. "src/mame/drivers/kangaroo.*",
	MAME_DIR .. "src/mame/video/kangaroo.*",
	MAME_DIR .. "src/mame/drivers/markham.*",
	MAME_DIR .. "src/mame/video/markham.*",
	MAME_DIR .. "src/mame/drivers/route16.*",
	MAME_DIR .. "src/mame/video/route16.*",
	MAME_DIR .. "src/mame/drivers/shanghai.*",
	MAME_DIR .. "src/mame/drivers/shangha3.*",
	MAME_DIR .. "src/mame/video/shangha3.*",
	MAME_DIR .. "src/mame/drivers/strnskil.*",
	MAME_DIR .. "src/mame/video/strnskil.*",
	MAME_DIR .. "src/mame/drivers/tonton.*",
}

createMAMEProjects(_target, _subtarget, "suna")
files {
	MAME_DIR .. "src/mame/drivers/go2000.*",
	MAME_DIR .. "src/mame/drivers/goindol.*",
	MAME_DIR .. "src/mame/video/goindol.*",
	MAME_DIR .. "src/mame/drivers/suna8.*",
	MAME_DIR .. "src/mame/audio/suna8.*",
	MAME_DIR .. "src/mame/video/suna8.*",
	MAME_DIR .. "src/mame/drivers/suna16.*",
	MAME_DIR .. "src/mame/video/suna16.*",
}

createMAMEProjects(_target, _subtarget, "sure")
files {
	MAME_DIR .. "src/mame/drivers/mil4000.*",

}

createMAMEProjects(_target, _subtarget, "taito")
files {
	MAME_DIR .. "src/mame/drivers/2mindril.*",
	MAME_DIR .. "src/mame/drivers/40love.*",
	MAME_DIR .. "src/mame/video/40love.*",
	MAME_DIR .. "src/mame/drivers/arkanoid.*",
	MAME_DIR .. "src/mame/machine/arkanoid.*",
	MAME_DIR .. "src/mame/video/arkanoid.*",
	MAME_DIR .. "src/mame/drivers/ashnojoe.*",
	MAME_DIR .. "src/mame/video/ashnojoe.*",
	MAME_DIR .. "src/mame/drivers/asuka.*",
	MAME_DIR .. "src/mame/machine/bonzeadv.*",
	MAME_DIR .. "src/mame/video/asuka.*",
	MAME_DIR .. "src/mame/drivers/bigevglf.*",
	MAME_DIR .. "src/mame/machine/bigevglf.*",
	MAME_DIR .. "src/mame/video/bigevglf.*",
	MAME_DIR .. "src/mame/drivers/bking.*",
	MAME_DIR .. "src/mame/video/bking.*",
	MAME_DIR .. "src/mame/drivers/bublbobl.*",
	MAME_DIR .. "src/mame/machine/bublbobl.*",
	MAME_DIR .. "src/mame/video/bublbobl.*",
	MAME_DIR .. "src/mame/drivers/buggychl.*",
	MAME_DIR .. "src/mame/machine/buggychl.*",
	MAME_DIR .. "src/mame/video/buggychl.*",
	MAME_DIR .. "src/mame/drivers/capr1.*",
	MAME_DIR .. "src/mame/drivers/caprcyc.*",
	MAME_DIR .. "src/mame/drivers/cchance.*",
	MAME_DIR .. "src/mame/drivers/chaknpop.*",
	MAME_DIR .. "src/mame/machine/chaknpop.*",
	MAME_DIR .. "src/mame/video/chaknpop.*",
	MAME_DIR .. "src/mame/drivers/champbwl.*",
	MAME_DIR .. "src/mame/drivers/changela.*",
	MAME_DIR .. "src/mame/video/changela.*",
	MAME_DIR .. "src/mame/drivers/crbaloon.*",
	MAME_DIR .. "src/mame/video/crbaloon.*",
	MAME_DIR .. "src/mame/audio/crbaloon.*",
	MAME_DIR .. "src/mame/drivers/cyclemb.*",
	MAME_DIR .. "src/mame/drivers/darius.*",
	MAME_DIR .. "src/mame/video/darius.*",
	MAME_DIR .. "src/mame/drivers/darkmist.*",
	MAME_DIR .. "src/mame/video/darkmist.*",
	MAME_DIR .. "src/mame/drivers/exzisus.*",
	MAME_DIR .. "src/mame/video/exzisus.*",
	MAME_DIR .. "src/mame/drivers/fgoal.*",
	MAME_DIR .. "src/mame/video/fgoal.*",
	MAME_DIR .. "src/mame/drivers/flstory.*",
	MAME_DIR .. "src/mame/machine/flstory.*",
	MAME_DIR .. "src/mame/video/flstory.*",
	MAME_DIR .. "src/mame/drivers/galastrm.*",
	MAME_DIR .. "src/mame/video/galastrm.*",
	MAME_DIR .. "src/mame/drivers/gladiatr.*",
	MAME_DIR .. "src/mame/video/gladiatr.*",
	MAME_DIR .. "src/mame/drivers/grchamp.*",
	MAME_DIR .. "src/mame/audio/grchamp.*",
	MAME_DIR .. "src/mame/video/grchamp.*",
	MAME_DIR .. "src/mame/drivers/groundfx.*",
	MAME_DIR .. "src/mame/video/groundfx.*",
	MAME_DIR .. "src/mame/drivers/gsword.*",
	MAME_DIR .. "src/mame/machine/tait8741.*",
	MAME_DIR .. "src/mame/video/gsword.*",
	MAME_DIR .. "src/mame/drivers/gunbustr.*",
	MAME_DIR .. "src/mame/video/gunbustr.*",
	MAME_DIR .. "src/mame/drivers/halleys.*",
	MAME_DIR .. "src/mame/drivers/invqix.*",
	MAME_DIR .. "src/mame/drivers/jollyjgr.*",
	MAME_DIR .. "src/mame/drivers/ksayakyu.*",
	MAME_DIR .. "src/mame/video/ksayakyu.*",
	MAME_DIR .. "src/mame/drivers/lgp.*",
	MAME_DIR .. "src/mame/drivers/lkage.*",
	MAME_DIR .. "src/mame/machine/lkage.*",
	MAME_DIR .. "src/mame/video/lkage.*",
	MAME_DIR .. "src/mame/drivers/lsasquad.*",
	MAME_DIR .. "src/mame/machine/lsasquad.*",
	MAME_DIR .. "src/mame/video/lsasquad.*",
	MAME_DIR .. "src/mame/drivers/marinedt.*",
	MAME_DIR .. "src/mame/drivers/mexico86.*",
	MAME_DIR .. "src/mame/machine/mexico86.*",
	MAME_DIR .. "src/mame/video/mexico86.*",
	MAME_DIR .. "src/mame/drivers/minivadr.*",
	MAME_DIR .. "src/mame/drivers/missb2.*",
	MAME_DIR .. "src/mame/drivers/mlanding.*",
	MAME_DIR .. "src/mame/drivers/msisaac.*",
	MAME_DIR .. "src/mame/video/msisaac.*",
	MAME_DIR .. "src/mame/drivers/ninjaw.*",
	MAME_DIR .. "src/mame/video/ninjaw.*",
	MAME_DIR .. "src/mame/drivers/nycaptor.*",
	MAME_DIR .. "src/mame/machine/nycaptor.*",
	MAME_DIR .. "src/mame/video/nycaptor.*",
	MAME_DIR .. "src/mame/drivers/opwolf.*",
	MAME_DIR .. "src/mame/machine/opwolf.*",
	MAME_DIR .. "src/mame/video/opwolf.*",
	MAME_DIR .. "src/mame/drivers/othunder.*",
	MAME_DIR .. "src/mame/video/othunder.*",
	MAME_DIR .. "src/mame/drivers/pitnrun.*",
	MAME_DIR .. "src/mame/machine/pitnrun.*",
	MAME_DIR .. "src/mame/video/pitnrun.*",
	MAME_DIR .. "src/mame/drivers/qix.*",
	MAME_DIR .. "src/mame/machine/qix.*",
	MAME_DIR .. "src/mame/audio/qix.*",
	MAME_DIR .. "src/mame/video/qix.*",
	MAME_DIR .. "src/mame/drivers/rainbow.*",
	MAME_DIR .. "src/mame/machine/rainbow.*",
	MAME_DIR .. "src/mame/video/rainbow.*",
	MAME_DIR .. "src/mame/drivers/rastan.*",
	MAME_DIR .. "src/mame/video/rastan.*",
	MAME_DIR .. "src/mame/drivers/retofinv.*",
	MAME_DIR .. "src/mame/machine/retofinv.*",
	MAME_DIR .. "src/mame/video/retofinv.*",
	MAME_DIR .. "src/mame/drivers/rollrace.*",
	MAME_DIR .. "src/mame/video/rollrace.*",
	MAME_DIR .. "src/mame/drivers/sbowling.*",
	MAME_DIR .. "src/mame/drivers/slapshot.*",
	MAME_DIR .. "src/mame/video/slapshot.*",
	MAME_DIR .. "src/mame/drivers/ssrj.*",
	MAME_DIR .. "src/mame/video/ssrj.*",
	MAME_DIR .. "src/mame/drivers/superchs.*",
	MAME_DIR .. "src/mame/video/superchs.*",
	MAME_DIR .. "src/mame/drivers/superqix.*",
	MAME_DIR .. "src/mame/video/superqix.*",
	MAME_DIR .. "src/mame/drivers/taito_b.*",
	MAME_DIR .. "src/mame/video/taito_b.*",
	MAME_DIR .. "src/mame/drivers/taito_f2.*",
	MAME_DIR .. "src/mame/video/taito_f2.*",
	MAME_DIR .. "src/mame/drivers/taito_f3.*",
	MAME_DIR .. "src/mame/video/taito_f3.*",
	MAME_DIR .. "src/mame/audio/taito_en.*",
	MAME_DIR .. "src/mame/drivers/taito_h.*",
	MAME_DIR .. "src/mame/video/taito_h.*",
	MAME_DIR .. "src/mame/drivers/taito_l.*",
	MAME_DIR .. "src/mame/video/taito_l.*",
	MAME_DIR .. "src/mame/drivers/taito_x.*",
	MAME_DIR .. "src/mame/machine/cchip.*",
	MAME_DIR .. "src/mame/drivers/taito_z.*",
	MAME_DIR .. "src/mame/video/taito_z.*",
	MAME_DIR .. "src/mame/drivers/taito_o.*",
	MAME_DIR .. "src/mame/video/taito_o.*",
	MAME_DIR .. "src/mame/drivers/taitoair.*",
	MAME_DIR .. "src/mame/video/taitoair.*",
	MAME_DIR .. "src/mame/drivers/taitogn.*",
	MAME_DIR .. "src/mame/drivers/taitojc.*",
	MAME_DIR .. "src/mame/video/taitojc.*",
	MAME_DIR .. "src/mame/drivers/taitopjc.*",
	MAME_DIR .. "src/mame/drivers/taitosj.*",
	MAME_DIR .. "src/mame/machine/taitosj.*",
	MAME_DIR .. "src/mame/video/taitosj.*",
	MAME_DIR .. "src/mame/drivers/taitottl.*",
	MAME_DIR .. "src/mame/drivers/taitotz.*",
	MAME_DIR .. "src/mame/drivers/taitotx.*",
	MAME_DIR .. "src/mame/drivers/taitowlf.*",
	MAME_DIR .. "src/mame/drivers/tnzs.*",
	MAME_DIR .. "src/mame/machine/tnzs.*",
	MAME_DIR .. "src/mame/video/tnzs.*",
	MAME_DIR .. "src/mame/drivers/topspeed.*",
	MAME_DIR .. "src/mame/video/topspeed.*",
	MAME_DIR .. "src/mame/drivers/tsamurai.*",
	MAME_DIR .. "src/mame/video/tsamurai.*",
	MAME_DIR .. "src/mame/drivers/undrfire.*",
	MAME_DIR .. "src/mame/video/undrfire.*",
	MAME_DIR .. "src/mame/drivers/volfied.*",
	MAME_DIR .. "src/mame/machine/volfied.*",
	MAME_DIR .. "src/mame/video/volfied.*",
	MAME_DIR .. "src/mame/drivers/warriorb.*",
	MAME_DIR .. "src/mame/video/warriorb.*",
	MAME_DIR .. "src/mame/drivers/wgp.*",
	MAME_DIR .. "src/mame/video/wgp.*",
	MAME_DIR .. "src/mame/drivers/wyvernf0.*",
	MAME_DIR .. "src/mame/audio/taitosnd.*",
	MAME_DIR .. "src/mame/audio/taito_zm.*",
	MAME_DIR .. "src/mame/audio/t5182.*",
	MAME_DIR .. "src/mame/machine/taitoio.*",
	MAME_DIR .. "src/mame/video/taito_helper.*",
	MAME_DIR .. "src/mame/video/pc080sn.*",
	MAME_DIR .. "src/mame/video/pc090oj.*",
	MAME_DIR .. "src/mame/video/tc0080vco.*",
	MAME_DIR .. "src/mame/video/tc0100scn.*",
	MAME_DIR .. "src/mame/video/tc0150rod.*",
	MAME_DIR .. "src/mame/video/tc0280grd.*",
	MAME_DIR .. "src/mame/video/tc0360pri.*",
	MAME_DIR .. "src/mame/video/tc0480scp.*",
	MAME_DIR .. "src/mame/video/tc0110pcr.*",
	MAME_DIR .. "src/mame/video/tc0180vcu.*",
}

createMAMEProjects(_target, _subtarget, "tatsumi")
files {
	MAME_DIR .. "src/mame/drivers/kingdrby.*",
	MAME_DIR .. "src/mame/drivers/lockon.*",
	MAME_DIR .. "src/mame/video/lockon.*",
	MAME_DIR .. "src/mame/drivers/tatsumi.*",
	MAME_DIR .. "src/mame/machine/tatsumi.*",
	MAME_DIR .. "src/mame/video/tatsumi.*",
	MAME_DIR .. "src/mame/drivers/tx1.*",
	MAME_DIR .. "src/mame/machine/tx1.*",
	MAME_DIR .. "src/mame/audio/tx1.*",
	MAME_DIR .. "src/mame/video/tx1.*",
}

createMAMEProjects(_target, _subtarget, "tch")
files {
	MAME_DIR .. "src/mame/drivers/kickgoal.*",
	MAME_DIR .. "src/mame/video/kickgoal.*",
	MAME_DIR .. "src/mame/drivers/littlerb.*",
	MAME_DIR .. "src/mame/drivers/rltennis.*",
	MAME_DIR .. "src/mame/video/rltennis.*",
	MAME_DIR .. "src/mame/drivers/speedspn.*",
	MAME_DIR .. "src/mame/video/speedspn.*",
	MAME_DIR .. "src/mame/drivers/wheelfir.*",
}

createMAMEProjects(_target, _subtarget, "tecfri")
files {
	MAME_DIR .. "src/mame/drivers/ambush.*",
	MAME_DIR .. "src/mame/video/ambush.*",
	MAME_DIR .. "src/mame/drivers/holeland.*",
	MAME_DIR .. "src/mame/video/holeland.*",
	MAME_DIR .. "src/mame/drivers/sauro.*",
	MAME_DIR .. "src/mame/video/sauro.*",
	MAME_DIR .. "src/mame/drivers/speedbal.*",
	MAME_DIR .. "src/mame/video/speedbal.*",
}

createMAMEProjects(_target, _subtarget, "technos")
files {
	MAME_DIR .. "src/mame/drivers/battlane.*",
	MAME_DIR .. "src/mame/video/battlane.*",
	MAME_DIR .. "src/mame/drivers/blockout.*",
	MAME_DIR .. "src/mame/video/blockout.*",
	MAME_DIR .. "src/mame/drivers/bogeyman.*",
	MAME_DIR .. "src/mame/video/bogeyman.*",
	MAME_DIR .. "src/mame/drivers/chinagat.*",
	MAME_DIR .. "src/mame/drivers/ddragon.*",
	MAME_DIR .. "src/mame/video/ddragon.*",
	MAME_DIR .. "src/mame/drivers/ddragon3.*",
	MAME_DIR .. "src/mame/video/ddragon3.*",
	MAME_DIR .. "src/mame/drivers/dogfgt.*",
	MAME_DIR .. "src/mame/video/dogfgt.*",
	MAME_DIR .. "src/mame/drivers/matmania.*",
	MAME_DIR .. "src/mame/video/matmania.*",
	MAME_DIR .. "src/mame/drivers/mystston.*",
	MAME_DIR .. "src/mame/video/mystston.*",
	MAME_DIR .. "src/mame/drivers/renegade.*",
	MAME_DIR .. "src/mame/video/renegade.*",
	MAME_DIR .. "src/mame/drivers/scregg.*",
	MAME_DIR .. "src/mame/drivers/shadfrce.*",
	MAME_DIR .. "src/mame/video/shadfrce.*",
	MAME_DIR .. "src/mame/drivers/spdodgeb.*",
	MAME_DIR .. "src/mame/video/spdodgeb.*",
	MAME_DIR .. "src/mame/drivers/ssozumo.*",
	MAME_DIR .. "src/mame/video/ssozumo.*",
	MAME_DIR .. "src/mame/drivers/tagteam.*",
	MAME_DIR .. "src/mame/video/tagteam.*",
	MAME_DIR .. "src/mame/drivers/vball.*",
	MAME_DIR .. "src/mame/video/vball.*",
	MAME_DIR .. "src/mame/drivers/wwfsstar.*",
	MAME_DIR .. "src/mame/video/wwfsstar.*",
	MAME_DIR .. "src/mame/drivers/xain.*",
	MAME_DIR .. "src/mame/video/xain.*",
}

createMAMEProjects(_target, _subtarget, "tehkan")
files {
	MAME_DIR .. "src/mame/video/tecmo_spr.*",
	MAME_DIR .. "src/mame/video/tecmo_mix.*",
	MAME_DIR .. "src/mame/drivers/bombjack.*",
	MAME_DIR .. "src/mame/video/bombjack.*",
	MAME_DIR .. "src/mame/drivers/gaiden.*",
	MAME_DIR .. "src/mame/video/gaiden.*",
	MAME_DIR .. "src/mame/drivers/lvcards.*",
	MAME_DIR .. "src/mame/video/lvcards.*",
	MAME_DIR .. "src/mame/drivers/pbaction.*",
	MAME_DIR .. "src/mame/video/pbaction.*",
	MAME_DIR .. "src/mame/drivers/senjyo.*",
	MAME_DIR .. "src/mame/audio/senjyo.*",
	MAME_DIR .. "src/mame/video/senjyo.*",
	MAME_DIR .. "src/mame/drivers/solomon.*",
	MAME_DIR .. "src/mame/video/solomon.*",
	MAME_DIR .. "src/mame/drivers/spbactn.*",
	MAME_DIR .. "src/mame/video/spbactn.*",
	MAME_DIR .. "src/mame/drivers/tbowl.*",
	MAME_DIR .. "src/mame/video/tbowl.*",
	MAME_DIR .. "src/mame/drivers/tecmo.*",
	MAME_DIR .. "src/mame/video/tecmo.*",
	MAME_DIR .. "src/mame/drivers/tecmo16.*",
	MAME_DIR .. "src/mame/video/tecmo16.*",
	MAME_DIR .. "src/mame/drivers/tecmosys.*",
	MAME_DIR .. "src/mame/machine/tecmosys.*",
	MAME_DIR .. "src/mame/video/tecmosys.*",
	MAME_DIR .. "src/mame/drivers/tehkanwc.*",
	MAME_DIR .. "src/mame/video/tehkanwc.*",
	MAME_DIR .. "src/mame/drivers/wc90.*",
	MAME_DIR .. "src/mame/video/wc90.*",
	MAME_DIR .. "src/mame/drivers/wc90b.*",
	MAME_DIR .. "src/mame/video/wc90b.*",
}

createMAMEProjects(_target, _subtarget, "thepit")
files {
	MAME_DIR .. "src/mame/drivers/thepit.*",
	MAME_DIR .. "src/mame/video/thepit.*",
	MAME_DIR .. "src/mame/drivers/timelimt.*",
	MAME_DIR .. "src/mame/video/timelimt.*",
}

createMAMEProjects(_target, _subtarget, "toaplan")
files {
	MAME_DIR .. "src/mame/drivers/mjsister.*",
	MAME_DIR .. "src/mame/drivers/slapfght.*",
	MAME_DIR .. "src/mame/machine/slapfght.*",
	MAME_DIR .. "src/mame/video/slapfght.*",
	MAME_DIR .. "src/mame/drivers/snowbros.*",
	MAME_DIR .. "src/mame/video/kan_pand.*",
	MAME_DIR .. "src/mame/video/kan_panb.*",
	MAME_DIR .. "src/mame/drivers/toaplan1.*",
	MAME_DIR .. "src/mame/machine/toaplan1.*",
	MAME_DIR .. "src/mame/video/toaplan1.*",
	MAME_DIR .. "src/mame/drivers/toaplan2.*",
	MAME_DIR .. "src/mame/video/toaplan2.*",
	MAME_DIR .. "src/mame/video/gp9001.*",
	MAME_DIR .. "src/mame/drivers/twincobr.*",
	MAME_DIR .. "src/mame/machine/twincobr.*",
	MAME_DIR .. "src/mame/video/twincobr.*",
	MAME_DIR .. "src/mame/drivers/wardner.*",
	MAME_DIR .. "src/mame/video/toaplan_scu.*",
}

createMAMEProjects(_target, _subtarget, "tong")
files {
	MAME_DIR .. "src/mame/drivers/beezer.*",
	MAME_DIR .. "src/mame/machine/beezer.*",
	MAME_DIR .. "src/mame/video/beezer.*",
	MAME_DIR .. "src/mame/audio/beezer.*",
}

createMAMEProjects(_target, _subtarget, "unico")
files {
	MAME_DIR .. "src/mame/drivers/drgnmst.*",
	MAME_DIR .. "src/mame/video/drgnmst.*",
	MAME_DIR .. "src/mame/drivers/silkroad.*",
	MAME_DIR .. "src/mame/video/silkroad.*",
	MAME_DIR .. "src/mame/drivers/unico.*",
	MAME_DIR .. "src/mame/video/unico.*",
}

createMAMEProjects(_target, _subtarget, "univers")
files {
	MAME_DIR .. "src/mame/drivers/cheekyms.*",
	MAME_DIR .. "src/mame/video/cheekyms.*",
	MAME_DIR .. "src/mame/drivers/cosmic.*",
	MAME_DIR .. "src/mame/video/cosmic.*",
	MAME_DIR .. "src/mame/drivers/docastle.*",
	MAME_DIR .. "src/mame/machine/docastle.*",
	MAME_DIR .. "src/mame/video/docastle.*",
	MAME_DIR .. "src/mame/drivers/ladybug.*",
	MAME_DIR .. "src/mame/video/ladybug.*",
	MAME_DIR .. "src/mame/drivers/mrdo.*",
	MAME_DIR .. "src/mame/video/mrdo.*",
	MAME_DIR .. "src/mame/drivers/redclash.*",
	MAME_DIR .. "src/mame/video/redclash.*",
	MAME_DIR .. "src/mame/drivers/superdq.*",
}

createMAMEProjects(_target, _subtarget, "upl")
files {
	MAME_DIR .. "src/mame/drivers/mouser.*",
	MAME_DIR .. "src/mame/video/mouser.*",
	MAME_DIR .. "src/mame/drivers/ninjakd2.*",
	MAME_DIR .. "src/mame/video/ninjakd2.*",
	MAME_DIR .. "src/mame/drivers/nova2001.*",
	MAME_DIR .. "src/mame/video/nova2001.*",
	MAME_DIR .. "src/mame/drivers/xxmissio.*",
	MAME_DIR .. "src/mame/video/xxmissio.*",
}

createMAMEProjects(_target, _subtarget, "valadon")
files {
	MAME_DIR .. "src/mame/drivers/bagman.*",
	MAME_DIR .. "src/mame/machine/bagman.*",
	MAME_DIR .. "src/mame/video/bagman.*",
	MAME_DIR .. "src/mame/drivers/tankbust.*",
	MAME_DIR .. "src/mame/video/tankbust.*",
}

createMAMEProjects(_target, _subtarget, "veltmjr")
files {
	MAME_DIR .. "src/mame/drivers/cardline.*",
	MAME_DIR .. "src/mame/drivers/witch.*",
}

createMAMEProjects(_target, _subtarget, "venture")
files {
	MAME_DIR .. "src/mame/drivers/looping.*",
	MAME_DIR .. "src/mame/drivers/spcforce.*",
	MAME_DIR .. "src/mame/video/spcforce.*",
	MAME_DIR .. "src/mame/drivers/suprridr.*",
	MAME_DIR .. "src/mame/video/suprridr.*",
}

createMAMEProjects(_target, _subtarget, "vsystem")
files {
	MAME_DIR .. "src/mame/video/vsystem_spr.*",
	MAME_DIR .. "src/mame/video/vsystem_spr2.*",
	MAME_DIR .. "src/mame/drivers/aerofgt.*",
	MAME_DIR .. "src/mame/video/aerofgt.*",
	MAME_DIR .. "src/mame/drivers/crshrace.*",
	MAME_DIR .. "src/mame/video/crshrace.*",
	MAME_DIR .. "src/mame/drivers/f1gp.*",
	MAME_DIR .. "src/mame/video/f1gp.*",
	MAME_DIR .. "src/mame/drivers/fromance.*",
	MAME_DIR .. "src/mame/video/fromance.*",
	MAME_DIR .. "src/mame/drivers/fromanc2.*",
	MAME_DIR .. "src/mame/video/fromanc2.*",
	MAME_DIR .. "src/mame/drivers/gstriker.*",
	MAME_DIR .. "src/mame/video/gstriker.*",
	MAME_DIR .. "src/mame/video/mb60553.*",
	MAME_DIR .. "src/mame/video/vs920a.*",
	MAME_DIR .. "src/mame/drivers/inufuku.*",
	MAME_DIR .. "src/mame/video/inufuku.*",
	MAME_DIR .. "src/mame/drivers/ojankohs.*",
	MAME_DIR .. "src/mame/video/ojankohs.*",
	MAME_DIR .. "src/mame/drivers/pipedrm.*",
	MAME_DIR .. "src/mame/drivers/rpunch.*",
	MAME_DIR .. "src/mame/video/rpunch.*",
	MAME_DIR .. "src/mame/drivers/suprslam.*",
	MAME_DIR .. "src/mame/video/suprslam.*",
	MAME_DIR .. "src/mame/drivers/tail2nos.*",
	MAME_DIR .. "src/mame/video/tail2nos.*",
	MAME_DIR .. "src/mame/drivers/taotaido.*",
	MAME_DIR .. "src/mame/video/taotaido.*",
	MAME_DIR .. "src/mame/drivers/welltris.*",
	MAME_DIR .. "src/mame/video/welltris.*",
}

createMAMEProjects(_target, _subtarget, "yunsung")
files {
	MAME_DIR .. "src/mame/drivers/nmg5.*",
	MAME_DIR .. "src/mame/drivers/paradise.*",
	MAME_DIR .. "src/mame/video/paradise.*",
	MAME_DIR .. "src/mame/drivers/yunsung8.*",
	MAME_DIR .. "src/mame/video/yunsung8.*",
	MAME_DIR .. "src/mame/drivers/yunsun16.*",
	MAME_DIR .. "src/mame/video/yunsun16.*",
}

createMAMEProjects(_target, _subtarget, "zaccaria")
files {
	MAME_DIR .. "src/mame/drivers/laserbat.*",
	MAME_DIR .. "src/mame/audio/laserbat.*",
	MAME_DIR .. "src/mame/drivers/seabattl.*",
	MAME_DIR .. "src/mame/drivers/zac2650.*",
	MAME_DIR .. "src/mame/video/zac2650.*",
	MAME_DIR .. "src/mame/drivers/zaccaria.*",
	MAME_DIR .. "src/mame/video/zaccaria.*",
}

--------------------------------------------------
-- pinball drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "pinball")
files {
	MAME_DIR .. "src/mame/drivers/allied.*",
	MAME_DIR .. "src/mame/drivers/alvg.*",
	MAME_DIR .. "src/mame/drivers/atari_s1.*",
	MAME_DIR .. "src/mame/drivers/atari_s2.*",
	MAME_DIR .. "src/mame/drivers/bingo.*",
	MAME_DIR .. "src/mame/drivers/by17.*",
	MAME_DIR .. "src/mame/drivers/by35.*",
	MAME_DIR .. "src/mame/drivers/by6803.*",
	MAME_DIR .. "src/mame/drivers/by68701.*",
	MAME_DIR .. "src/mame/drivers/byvid.*",
	MAME_DIR .. "src/mame/drivers/capcom.*",
	MAME_DIR .. "src/mame/drivers/de_2.*",
	MAME_DIR .. "src/mame/drivers/de_3.*",
	MAME_DIR .. "src/mame/machine/decopincpu.*",
	MAME_DIR .. "src/mame/video/decodmd1.*",
	MAME_DIR .. "src/mame/video/decodmd2.*",
	MAME_DIR .. "src/mame/video/decodmd3.*",
	MAME_DIR .. "src/mame/drivers/de_3b.*",
	MAME_DIR .. "src/mame/drivers/flicker.*",
	MAME_DIR .. "src/mame/drivers/g627.*",
	MAME_DIR .. "src/mame/drivers/gp_1.*",
	MAME_DIR .. "src/mame/machine/genpin.*",
	MAME_DIR .. "src/mame/drivers/gp_2.*",
	MAME_DIR .. "src/mame/drivers/gts1.*",
	MAME_DIR .. "src/mame/drivers/gts3.*",
	MAME_DIR .. "src/mame/drivers/gts3a.*",
	MAME_DIR .. "src/mame/drivers/gts80.*",
	MAME_DIR .. "src/mame/drivers/gts80a.*",
	MAME_DIR .. "src/mame/drivers/gts80b.*",
	MAME_DIR .. "src/mame/drivers/hankin.*",
	MAME_DIR .. "src/mame/drivers/icecold.*",
	MAME_DIR .. "src/mame/drivers/inder.*",
	MAME_DIR .. "src/mame/drivers/jeutel.*",
	MAME_DIR .. "src/mame/drivers/jp.*",
	MAME_DIR .. "src/mame/drivers/jvh.*",
	MAME_DIR .. "src/mame/drivers/kissproto.*",
	MAME_DIR .. "src/mame/drivers/ltd.*",
	MAME_DIR .. "src/mame/drivers/micropin.*",
	MAME_DIR .. "src/mame/drivers/mephisto.*",
	MAME_DIR .. "src/mame/drivers/mrgame.*",
	MAME_DIR .. "src/mame/drivers/nsm.*",
	MAME_DIR .. "src/mame/drivers/peyper.*",
	MAME_DIR .. "src/mame/drivers/play_1.*",
	MAME_DIR .. "src/mame/drivers/play_2.*",
	MAME_DIR .. "src/mame/drivers/play_3.*",
	MAME_DIR .. "src/mame/drivers/play_5.*",
	MAME_DIR .. "src/mame/drivers/rowamet.*",
	MAME_DIR .. "src/mame/drivers/s11.*",
	MAME_DIR .. "src/mame/drivers/s11a.*",
	MAME_DIR .. "src/mame/drivers/s11b.*",
	MAME_DIR .. "src/mame/drivers/s11c.*",
	MAME_DIR .. "src/mame/audio/s11c_bg.*",
	MAME_DIR .. "src/mame/drivers/s3.*",
	MAME_DIR .. "src/mame/drivers/s4.*",
	MAME_DIR .. "src/mame/drivers/s6.*",
	MAME_DIR .. "src/mame/drivers/s6a.*",
	MAME_DIR .. "src/mame/drivers/s7.*",
	MAME_DIR .. "src/mame/drivers/s8.*",
	MAME_DIR .. "src/mame/drivers/s8a.*",
	MAME_DIR .. "src/mame/drivers/s9.*",
	MAME_DIR .. "src/mame/drivers/sam.*",
	MAME_DIR .. "src/mame/drivers/sleic.*",
	MAME_DIR .. "src/mame/drivers/spectra.*",
	MAME_DIR .. "src/mame/drivers/spinb.*",
	MAME_DIR .. "src/mame/drivers/st_mp100.*",
	MAME_DIR .. "src/mame/drivers/st_mp200.*",
	MAME_DIR .. "src/mame/drivers/taito.*",
	MAME_DIR .. "src/mame/drivers/techno.*",
	MAME_DIR .. "src/mame/drivers/vd.*",
	MAME_DIR .. "src/mame/drivers/whitestar.*",
	MAME_DIR .. "src/mame/drivers/white_mod.*",
	MAME_DIR .. "src/mame/drivers/wico.*",
	MAME_DIR .. "src/mame/drivers/wpc_95.*",
	MAME_DIR .. "src/mame/drivers/wpc_an.*",
	MAME_DIR .. "src/mame/drivers/wpc_dcs.*",
	MAME_DIR .. "src/mame/drivers/wpc_dot.*",
	MAME_DIR .. "src/mame/drivers/wpc_flip1.*",
	MAME_DIR .. "src/mame/drivers/wpc_flip2.*",
	MAME_DIR .. "src/mame/drivers/wpc_s.*",
	MAME_DIR .. "src/mame/machine/wpc.*",
	MAME_DIR .. "src/mame/audio/wpcsnd.*",
	MAME_DIR .. "src/mame/video/wpc_dmd.*",
	MAME_DIR .. "src/mame/machine/wpc_pic.*",
	MAME_DIR .. "src/mame/machine/wpc_lamp.*",
	MAME_DIR .. "src/mame/machine/wpc_out.*",
	MAME_DIR .. "src/mame/machine/wpc_shift.*",
	MAME_DIR .. "src/mame/drivers/zac_1.*",
	MAME_DIR .. "src/mame/drivers/zac_2.*",
	MAME_DIR .. "src/mame/drivers/zac_proto.*",
}

--------------------------------------------------
-- remaining drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "misc")
files {
	MAME_DIR .. "src/mame/drivers/1945kiii.*",
	MAME_DIR .. "src/mame/drivers/39in1.*",
	MAME_DIR .. "src/mame/drivers/3do.*",
	MAME_DIR .. "src/mame/machine/3do.*",
	MAME_DIR .. "src/mame/drivers/3x3puzzl.*",
	MAME_DIR .. "src/mame/drivers/4enraya.*",
	MAME_DIR .. "src/mame/video/4enraya.*",
	MAME_DIR .. "src/mame/drivers/4enlinea.*",
	MAME_DIR .. "src/mame/drivers/5clown.*",
	MAME_DIR .. "src/mame/drivers/a1supply.*",
	MAME_DIR .. "src/mame/drivers/acefruit.*",
	MAME_DIR .. "src/mame/drivers/aces1.*",
	MAME_DIR .. "src/mame/drivers/acesp.*",
	MAME_DIR .. "src/mame/drivers/adp.*",
	MAME_DIR .. "src/mame/drivers/alinvade.*",
	MAME_DIR .. "src/mame/drivers/amaticmg.*",
	MAME_DIR .. "src/mame/drivers/ampoker2.*",
	MAME_DIR .. "src/mame/video/ampoker2.*",
	MAME_DIR .. "src/mame/drivers/amspdwy.*",
	MAME_DIR .. "src/mame/video/amspdwy.*",
	MAME_DIR .. "src/mame/drivers/amusco.*",
	MAME_DIR .. "src/mame/drivers/arachnid.*",
	MAME_DIR .. "src/mame/drivers/artmagic.*",
	MAME_DIR .. "src/mame/video/artmagic.*",
	MAME_DIR .. "src/mame/drivers/astrafr.*",
	MAME_DIR .. "src/mame/drivers/astrcorp.*",
	MAME_DIR .. "src/mame/drivers/astropc.*",
	MAME_DIR .. "src/mame/drivers/atronic.*",
	MAME_DIR .. "src/mame/drivers/attckufo.*",
	MAME_DIR .. "src/mame/drivers/avt.*",
	MAME_DIR .. "src/mame/drivers/aztarac.*",
	MAME_DIR .. "src/mame/audio/aztarac.*",
	MAME_DIR .. "src/mame/video/aztarac.*",
	MAME_DIR .. "src/mame/drivers/bailey.*",
	MAME_DIR .. "src/mame/drivers/beaminv.*",
	MAME_DIR .. "src/mame/drivers/belatra.*",
	MAME_DIR .. "src/mame/drivers/bgt.*",
	MAME_DIR .. "src/mame/drivers/bingoman.*",
	MAME_DIR .. "src/mame/drivers/bingor.*",
	MAME_DIR .. "src/mame/drivers/blitz.*",
	MAME_DIR .. "src/mame/drivers/blitz68k.*",
	MAME_DIR .. "src/mame/drivers/buster.*",
	MAME_DIR .. "src/mame/drivers/calomega.*",
	MAME_DIR .. "src/mame/video/calomega.*",
	MAME_DIR .. "src/mame/drivers/carrera.*",
	MAME_DIR .. "src/mame/drivers/castle.*",
	MAME_DIR .. "src/mame/drivers/cave.*",
	MAME_DIR .. "src/mame/video/cave.*",
	MAME_DIR .. "src/mame/drivers/cavepc.*",
	MAME_DIR .. "src/mame/drivers/cv1k.*",
	MAME_DIR .. "src/mame/drivers/cb2001.*",
	MAME_DIR .. "src/mame/drivers/cdi.*",
	MAME_DIR .. "src/mame/video/mcd212.*",
	MAME_DIR .. "src/mame/machine/cdi070.*",
	MAME_DIR .. "src/mame/machine/cdislave.*",
	MAME_DIR .. "src/mame/machine/cdicdic.*",
	MAME_DIR .. "src/mame/drivers/cesclass.*",
	MAME_DIR .. "src/mame/drivers/chance32.*",
	MAME_DIR .. "src/mame/drivers/chicago.*",
	MAME_DIR .. "src/mame/drivers/chsuper.*",
	MAME_DIR .. "src/mame/drivers/cidelsa.*",
	MAME_DIR .. "src/mame/video/cidelsa.*",
	MAME_DIR .. "src/mame/drivers/cocoloco.*",
	MAME_DIR .. "src/mame/drivers/coinmstr.*",
	MAME_DIR .. "src/mame/drivers/coinmvga.*",
	MAME_DIR .. "src/mame/drivers/comebaby.*",
	MAME_DIR .. "src/mame/drivers/cupidon.*",
	MAME_DIR .. "src/mame/drivers/bntyhunt.*",
	MAME_DIR .. "src/mame/drivers/coolpool.*",
	MAME_DIR .. "src/mame/drivers/megaphx.*",
	MAME_DIR .. "src/mame/machine/inder_sb.*",
	MAME_DIR .. "src/mame/machine/inder_vid.*",
	MAME_DIR .. "src/mame/drivers/corona.*",
	MAME_DIR .. "src/mame/drivers/crystal.*",
	MAME_DIR .. "src/mame/video/vrender0.*",
	MAME_DIR .. "src/mame/drivers/cubeqst.*",
	MAME_DIR .. "src/mame/drivers/cybertnk.*",
	MAME_DIR .. "src/mame/drivers/dcheese.*",
	MAME_DIR .. "src/mame/video/dcheese.*",
	MAME_DIR .. "src/mame/drivers/dfruit.*",
	MAME_DIR .. "src/mame/drivers/dgpix.*",
	MAME_DIR .. "src/mame/drivers/discoboy.*",
	MAME_DIR .. "src/mame/drivers/dominob.*",
	MAME_DIR .. "src/mame/drivers/dorachan.*",
	MAME_DIR .. "src/mame/drivers/dreamwld.*",
	MAME_DIR .. "src/mame/drivers/dribling.*",
	MAME_DIR .. "src/mame/video/dribling.*",
	MAME_DIR .. "src/mame/drivers/drw80pkr.*",
	MAME_DIR .. "src/mame/drivers/dwarfd.*",
	MAME_DIR .. "src/mame/drivers/dynadice.*",
	MAME_DIR .. "src/mame/drivers/ecoinfr.*",
	MAME_DIR .. "src/mame/drivers/ecoinf1.*",
	MAME_DIR .. "src/mame/drivers/ecoinf2.*",
	MAME_DIR .. "src/mame/drivers/ecoinf3.*",
	MAME_DIR .. "src/mame/drivers/electra.*",
	MAME_DIR .. "src/mame/drivers/epos.*",
	MAME_DIR .. "src/mame/video/epos.*",
	MAME_DIR .. "src/mame/drivers/esd16.*",
	MAME_DIR .. "src/mame/video/esd16.*",
	MAME_DIR .. "src/mame/drivers/esh.*",
	MAME_DIR .. "src/mame/drivers/esripsys.*",
	MAME_DIR .. "src/mame/video/esripsys.*",
	MAME_DIR .. "src/mame/drivers/ettrivia.*",
	MAME_DIR .. "src/mame/drivers/extrema.*",
	MAME_DIR .. "src/mame/drivers/fireball.*",
	MAME_DIR .. "src/mame/drivers/flipjack.*",
	MAME_DIR .. "src/mame/drivers/flower.*",
	MAME_DIR .. "src/mame/audio/flower.*",
	MAME_DIR .. "src/mame/video/flower.*",
	MAME_DIR .. "src/mame/drivers/fortecar.*",
	MAME_DIR .. "src/mame/drivers/fresh.*",
	MAME_DIR .. "src/mame/drivers/freekick.*",
	MAME_DIR .. "src/mame/video/freekick.*",
	MAME_DIR .. "src/mame/drivers/fungames.*",
	MAME_DIR .. "src/mame/drivers/funkball.*",
	MAME_DIR .. "src/mame/drivers/gambl186.*",
	MAME_DIR .. "src/mame/drivers/galaxi.*",
	MAME_DIR .. "src/mame/drivers/galgame.*",
	MAME_DIR .. "src/mame/drivers/gamecstl.*",
	MAME_DIR .. "src/mame/drivers/gammagic.*",
	MAME_DIR .. "src/mame/drivers/gamtor.*",
	MAME_DIR .. "src/mame/drivers/gei.*",
	MAME_DIR .. "src/mame/drivers/globalfr.*",
	MAME_DIR .. "src/mame/drivers/globalvr.*",
	MAME_DIR .. "src/mame/drivers/gluck2.*",
	MAME_DIR .. "src/mame/drivers/goldngam.*",
	MAME_DIR .. "src/mame/drivers/goldnpkr.*",
	MAME_DIR .. "src/mame/drivers/good.*",
	MAME_DIR .. "src/mame/drivers/gotcha.*",
	MAME_DIR .. "src/mame/video/gotcha.*",
	MAME_DIR .. "src/mame/drivers/gstream.*",
	MAME_DIR .. "src/mame/drivers/gumbo.*",
	MAME_DIR .. "src/mame/video/gumbo.*",
	MAME_DIR .. "src/mame/drivers/gunpey.*",
	MAME_DIR .. "src/mame/drivers/hideseek.*",
	MAME_DIR .. "src/mame/drivers/hazelgr.*",
	MAME_DIR .. "src/mame/drivers/headonb.*",
	MAME_DIR .. "src/mame/drivers/highvdeo.*",
	MAME_DIR .. "src/mame/drivers/himesiki.*",
	MAME_DIR .. "src/mame/video/himesiki.*",
	MAME_DIR .. "src/mame/drivers/hitpoker.*",
	MAME_DIR .. "src/mame/drivers/homedata.*",
	MAME_DIR .. "src/mame/video/homedata.*",
	MAME_DIR .. "src/mame/drivers/hotblock.*",
	MAME_DIR .. "src/mame/drivers/hotstuff.*",
	MAME_DIR .. "src/mame/drivers/ichiban.*",
	MAME_DIR .. "src/mame/drivers/imolagp.*",
	MAME_DIR .. "src/mame/drivers/intrscti.*",
	MAME_DIR .. "src/mame/drivers/istellar.*",
	MAME_DIR .. "src/mame/drivers/itgambl2.*",
	MAME_DIR .. "src/mame/drivers/itgambl3.*",
	MAME_DIR .. "src/mame/drivers/itgamble.*",
	MAME_DIR .. "src/mame/drivers/jackpool.*",
	MAME_DIR .. "src/mame/drivers/jankenmn.*",
	MAME_DIR .. "src/mame/drivers/jokrwild.*",
	MAME_DIR .. "src/mame/drivers/jongkyo.*",
	MAME_DIR .. "src/mame/drivers/jubilee.*",
	MAME_DIR .. "src/mame/drivers/kas89.*",
	MAME_DIR .. "src/mame/drivers/kingpin.*",
	MAME_DIR .. "src/mame/drivers/koikoi.*",
	MAME_DIR .. "src/mame/drivers/kurukuru.*",
	MAME_DIR .. "src/mame/drivers/kyugo.*",
	MAME_DIR .. "src/mame/video/kyugo.*",
	MAME_DIR .. "src/mame/drivers/ladyfrog.*",
	MAME_DIR .. "src/mame/video/ladyfrog.*",
	MAME_DIR .. "src/mame/drivers/laserbas.*",
	MAME_DIR .. "src/mame/drivers/lethalj.*",
	MAME_DIR .. "src/mame/video/lethalj.*",
	MAME_DIR .. "src/mame/drivers/limenko.*",
	MAME_DIR .. "src/mame/drivers/ltcasino.*",
	MAME_DIR .. "src/mame/drivers/lucky74.*",
	MAME_DIR .. "src/mame/video/lucky74.*",
	MAME_DIR .. "src/mame/drivers/luckgrln.*",
	MAME_DIR .. "src/mame/drivers/magic10.*",
	MAME_DIR .. "src/mame/drivers/magicard.*",
	MAME_DIR .. "src/mame/drivers/magicfly.*",
	MAME_DIR .. "src/mame/drivers/magictg.*",
	MAME_DIR .. "src/mame/drivers/magtouch.*",
	MAME_DIR .. "src/mame/drivers/majorpkr.*",
	MAME_DIR .. "src/mame/drivers/malzak.*",
	MAME_DIR .. "src/mame/video/malzak.*",
	MAME_DIR .. "src/mame/drivers/manohman.*",
	MAME_DIR .. "src/mame/drivers/mcatadv.*",
	MAME_DIR .. "src/mame/video/mcatadv.*",
	MAME_DIR .. "src/mame/drivers/mgavegas.*",
	MAME_DIR .. "src/mame/drivers/meyc8080.*",
	MAME_DIR .. "src/mame/drivers/meyc8088.*",
	MAME_DIR .. "src/mame/drivers/micro3d.*",
	MAME_DIR .. "src/mame/machine/micro3d.*",
	MAME_DIR .. "src/mame/video/micro3d.*",
	MAME_DIR .. "src/mame/audio/micro3d.*",
	MAME_DIR .. "src/mame/drivers/midas.*",
	MAME_DIR .. "src/mame/drivers/miniboy7.*",
	MAME_DIR .. "src/mame/drivers/mirax.*",
	MAME_DIR .. "src/mame/drivers/mole.*",
	MAME_DIR .. "src/mame/drivers/mosaic.*",
	MAME_DIR .. "src/mame/video/mosaic.*",
	MAME_DIR .. "src/mame/drivers/mpu12wbk.*",
	MAME_DIR .. "src/mame/drivers/mrjong.*",
	MAME_DIR .. "src/mame/video/mrjong.*",
	MAME_DIR .. "src/mame/drivers/multfish.*",
	MAME_DIR .. "src/mame/drivers/multfish_boot.*",
	MAME_DIR .. "src/mame/drivers/multfish_ref.*",
	MAME_DIR .. "src/mame/drivers/murogem.*",
	MAME_DIR .. "src/mame/drivers/murogmbl.*",
	MAME_DIR .. "src/mame/drivers/neoprint.*",
	MAME_DIR .. "src/mame/drivers/neptunp2.*",
	MAME_DIR .. "src/mame/drivers/news.*",
	MAME_DIR .. "src/mame/video/news.*",
	MAME_DIR .. "src/mame/drivers/nexus3d.*",
	MAME_DIR .. "src/mame/drivers/norautp.*",
	MAME_DIR .. "src/mame/audio/norautp.*",
	MAME_DIR .. "src/mame/drivers/nsmpoker.*",
	MAME_DIR .. "src/mame/drivers/oneshot.*",
	MAME_DIR .. "src/mame/video/oneshot.*",
	MAME_DIR .. "src/mame/drivers/onetwo.*",
	MAME_DIR .. "src/mame/drivers/othello.*",
	MAME_DIR .. "src/mame/drivers/pachifev.*",
	MAME_DIR .. "src/mame/drivers/pasha2.*",
	MAME_DIR .. "src/mame/drivers/pass.*",
	MAME_DIR .. "src/mame/video/pass.*",
	MAME_DIR .. "src/mame/drivers/peplus.*",
	MAME_DIR .. "src/mame/drivers/photon.*",
	MAME_DIR .. "src/mame/video/pk8000.*",
	MAME_DIR .. "src/mame/drivers/photon2.*",
	MAME_DIR .. "src/mame/drivers/photoply.*",
	MAME_DIR .. "src/mame/drivers/pinkiri8.*",
	MAME_DIR .. "src/mame/drivers/pipeline.*",
	MAME_DIR .. "src/mame/drivers/pkscram.*",
	MAME_DIR .. "src/mame/drivers/pntnpuzl.*",
	MAME_DIR .. "src/mame/drivers/policetr.*",
	MAME_DIR .. "src/mame/video/policetr.*",
	MAME_DIR .. "src/mame/drivers/polyplay.*",
	MAME_DIR .. "src/mame/audio/polyplay.*",
	MAME_DIR .. "src/mame/video/polyplay.*",
	MAME_DIR .. "src/mame/drivers/poker72.*",
	MAME_DIR .. "src/mame/drivers/potgoldu.*",
	MAME_DIR .. "src/mame/drivers/proconn.*",
	MAME_DIR .. "src/mame/drivers/psattack.*",
	MAME_DIR .. "src/mame/drivers/pse.*",
	MAME_DIR .. "src/mame/drivers/quizo.*",
	MAME_DIR .. "src/mame/drivers/quizpun2.*",
	MAME_DIR .. "src/mame/drivers/rbmk.*",
	MAME_DIR .. "src/mame/drivers/rcorsair.*",
	MAME_DIR .. "src/mame/drivers/re900.*",
	MAME_DIR .. "src/mame/drivers/rgum.*",
	MAME_DIR .. "src/mame/drivers/roul.*",
	MAME_DIR .. "src/mame/drivers/savquest.*",
	MAME_DIR .. "src/mame/drivers/sanremo.*",
	MAME_DIR .. "src/mame/drivers/sfbonus.*",
	MAME_DIR .. "src/mame/drivers/shangkid.*",
	MAME_DIR .. "src/mame/video/shangkid.*",
	MAME_DIR .. "src/mame/drivers/skeetsht.*",
	MAME_DIR .. "src/mame/drivers/skimaxx.*",
	MAME_DIR .. "src/mame/drivers/skyarmy.*",
	MAME_DIR .. "src/mame/drivers/skylncr.*",
	MAME_DIR .. "src/mame/drivers/sliver.*",
	MAME_DIR .. "src/mame/drivers/slotcarn.*",
	MAME_DIR .. "src/mame/drivers/smsmcorp.*",
	MAME_DIR .. "src/mame/drivers/sothello.*",
	MAME_DIR .. "src/mame/drivers/splus.*",
	MAME_DIR .. "src/mame/drivers/spool99.*",
	MAME_DIR .. "src/mame/drivers/sprcros2.*",
	MAME_DIR .. "src/mame/video/sprcros2.*",
	MAME_DIR .. "src/mame/drivers/sshot.*",
	MAME_DIR .. "src/mame/drivers/ssingles.*",
	MAME_DIR .. "src/mame/drivers/sstrangr.*",
	MAME_DIR .. "src/mame/drivers/statriv2.*",
	MAME_DIR .. "src/mame/drivers/stellafr.*",
	MAME_DIR .. "src/mame/drivers/stuntair.*",
	MAME_DIR .. "src/mame/drivers/su2000.*",
	MAME_DIR .. "src/mame/drivers/summit.*",
	MAME_DIR .. "src/mame/drivers/sumt8035.*",
	MAME_DIR .. "src/mame/drivers/supercrd.*",
	MAME_DIR .. "src/mame/drivers/supertnk.*",
	MAME_DIR .. "src/mame/drivers/superwng.*",
	MAME_DIR .. "src/mame/drivers/tapatune.*",
	MAME_DIR .. "src/mame/drivers/tattack.*",
	MAME_DIR .. "src/mame/drivers/taxidriv.*",
	MAME_DIR .. "src/mame/video/taxidriv.*",
	MAME_DIR .. "src/mame/drivers/tcl.*",
	MAME_DIR .. "src/mame/drivers/thayers.*",
	MAME_DIR .. "src/mame/drivers/thedeep.*",
	MAME_DIR .. "src/mame/video/thedeep.*",
	MAME_DIR .. "src/mame/drivers/tiamc1.*",
	MAME_DIR .. "src/mame/video/tiamc1.*",
	MAME_DIR .. "src/mame/audio/tiamc1.*",
	MAME_DIR .. "src/mame/drivers/tickee.*",
	MAME_DIR .. "src/mame/drivers/tmspoker.*",
	MAME_DIR .. "src/mame/drivers/truco.*",
	MAME_DIR .. "src/mame/video/truco.*",
	MAME_DIR .. "src/mame/drivers/trucocl.*",
	MAME_DIR .. "src/mame/video/trucocl.*",
	MAME_DIR .. "src/mame/drivers/trvmadns.*",
	MAME_DIR .. "src/mame/drivers/trvquest.*",
	MAME_DIR .. "src/mame/drivers/ttchamp.*",
	MAME_DIR .. "src/mame/drivers/tugboat.*",
	MAME_DIR .. "src/mame/drivers/umipoker.*",
	MAME_DIR .. "src/mame/drivers/unkfr.*",
	MAME_DIR .. "src/mame/drivers/unkhorse.*",
	MAME_DIR .. "src/mame/drivers/usgames.*",
	MAME_DIR .. "src/mame/video/usgames.*",
	MAME_DIR .. "src/mame/drivers/vamphalf.*",
	MAME_DIR .. "src/mame/drivers/vcombat.*",
	MAME_DIR .. "src/mame/drivers/vectrex.*",
	MAME_DIR .. "src/mame/video/vectrex.*",
	MAME_DIR .. "src/mame/machine/vectrex.*",
	MAME_DIR .. "src/mame/drivers/videopkr.*",
	MAME_DIR .. "src/mame/drivers/vlc.*",
	MAME_DIR .. "src/mame/drivers/voyager.*",
	MAME_DIR .. "src/mame/drivers/vp101.*",
	MAME_DIR .. "src/mame/drivers/vpoker.*",
	MAME_DIR .. "src/mame/drivers/vroulet.*",
	MAME_DIR .. "src/mame/drivers/wildpkr.*",
	MAME_DIR .. "src/mame/drivers/wms.*",
	MAME_DIR .. "src/mame/drivers/xtom3d.*",
	MAME_DIR .. "src/mame/drivers/xyonix.*",
	MAME_DIR .. "src/mame/video/xyonix.*",
}
end

