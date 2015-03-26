---------------------------------------------------------------------------
--
--   mess.mak
--
--   MESS target makefile
--
---------------------------------------------------------------------------

--------------------------------------------------
-- specify available CPU cores
--------------------------------------------------

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
-- specify available sound cores; some of these are
-- only for MAME and so aren't included
--------------------------------------------------

--SOUNDS["SAMPLES"] = true
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
--SOUNDS["YM3438"] = true
SOUNDS["YM3812"] = true
SOUNDS["YM3526"] = true
SOUNDS["Y8950"] = true
SOUNDS["YMF262"] = true
--SOUNDS["YMF271"] = true
--SOUNDS["YMF278B"] = true
--SOUNDS["YMZ280B"] = true
SOUNDS["SN76477"] = true
SOUNDS["SN76496"] = true
SOUNDS["POKEY"] = true
SOUNDS["TIA"] = true
SOUNDS["NES_APU"] = true
SOUNDS["AMIGA"] = true
SOUNDS["ASTROCADE"] = true
--SOUNDS["NAMCO"] = true
--SOUNDS["NAMCO_15XX"] = true
--SOUNDS["NAMCO_CUS30"] = true
--SOUNDS["NAMCO_52XX"] = true
--SOUNDS["NAMCO_63701X"] = true
SOUNDS["T6W28"] = true
--SOUNDS["SNKWAVE"] = true
--SOUNDS["C140"] = true
--SOUNDS["C352"] = true
--SOUNDS["TMS36XX"] = true
--SOUNDS["TMS3615"] = true
SOUNDS["TMS5110"] = true
SOUNDS["TMS5220"] = true
SOUNDS["VLM5030"] = true
--SOUNDS["ADPCM"] = true
SOUNDS["MSM5205"] = true
--SOUNDS["MSM5232"] = true
SOUNDS["OKIM6258"] = true
SOUNDS["OKIM6295"] = true
--SOUNDS["OKIM6376"] = true
--SOUNDS["OKIM9810"] = true
SOUNDS["UPD7752"] = true
SOUNDS["UPD7759"] = true
SOUNDS["HC55516"] = true
--SOUNDS["TC8830F"] = true
--SOUNDS["K005289"] = true
--SOUNDS["K007232"] = true
SOUNDS["K051649"] = true
--SOUNDS["K053260"] = true
--SOUNDS["K054539"] = true
--SOUNDS["K056800"] = true
--SOUNDS["SEGAPCM"] = true
--SOUNDS["MULTIPCM"] = true
SOUNDS["SCSP"] = true
SOUNDS["AICA"] = true
SOUNDS["RF5C68"] = true
--SOUNDS["RF5C400"] = true
--SOUNDS["CEM3394"] = true
SOUNDS["QSOUND"] = true
--SOUNDS["QS1000"] = true
SOUNDS["SAA1099"] = true
--SOUNDS["IREMGA20"] = true
SOUNDS["ES5503"] = true
SOUNDS["ES5505"] = true
SOUNDS["ES5506"] = true
--SOUNDS["BSMT2000"] = true
--SOUNDS["GAELCO_CG1V"] = true
--SOUNDS["GAELCO_GAE1"] = true
SOUNDS["C6280"] = true
--SOUNDS["SP0250"] = true
SOUNDS["SPU"] = true
SOUNDS["CDDA"] = true
--SOUNDS["ICS2115"] = true
--SOUNDS["I5000_SND"] = true
--SOUNDS["ST0016"] = true
--SOUNDS["NILE"] = true
--SOUNDS["X1_010"] = true
--SOUNDS["VRENDER0"] = true
SOUNDS["VOTRAX"] = true
--SOUNDS["ES8712"] = true
SOUNDS["CDP1869"] = true
SOUNDS["S14001A"] = true
SOUNDS["WAVE"] = true
SOUNDS["SID6581"] = true
SOUNDS["SID8580"] = true
SOUNDS["SP0256"] = true
--SOUNDS["DIGITALKER"] = true
SOUNDS["CDP1863"] = true
SOUNDS["CDP1864"] = true
--SOUNDS["ZSG2"] = true
SOUNDS["MOS656X"] = true
SOUNDS["ASC"] = true
--SOUNDS["MAS3507D"] = true
SOUNDS["SOCRATES"] = true
SOUNDS["TMC0285"] = true
SOUNDS["TMS5200"] = true
SOUNDS["CD2801"] = true
SOUNDS["CD2802"] = true
--SOUNDS["M58817"] = true
SOUNDS["TMC0281"] = true
SOUNDS["TMS5100"] = true
SOUNDS["TMS5110A"] = true
SOUNDS["LMC1992"] = true
SOUNDS["AWACS"] = true
--SOUNDS["YMZ770"] = true
SOUNDS["T6721A"] = true
SOUNDS["MOS7360"] = true
SOUNDS["ESQPUMP"] = true
SOUNDS["VRC6"] = true

--------------------------------------------------
-- specify available video cores
--------------------------------------------------

VIDEOS["SEGA315_5124"] = true
VIDEOS["SEGA315_5313"] = true
--VIDEOS+= BUFSPRITE"] = true
VIDEOS["CDP1861"] = true
VIDEOS["CDP1862"] = true
VIDEOS["CRT9007"] = true
VIDEOS["CRT9021"] = true
VIDEOS["CRT9212"] = true
VIDEOS["CRTC_EGA"] = true
VIDEOS["DL1416"] = true
VIDEOS["DM9368"] = true
VIDEOS["EF9340_1"] = true
VIDEOS["EF9345"] = true
VIDEOS["GF4500"] = true
--VIDEOS+= EPIC12"] = true
--VIDEOS+= FIXFREQ"] = true
--VIDEOS+= H63484"] = true
VIDEOS["HD44102"] = true
VIDEOS["HD44352"] = true
VIDEOS["HD44780"] = true
VIDEOS["HD61830"] = true
--VIDEOS+= HD63484"] = true
VIDEOS["HD66421"] = true
VIDEOS["HUC6202"] = true
VIDEOS["HUC6260"] = true
VIDEOS["HUC6261"] = true
VIDEOS["HUC6270"] = true
VIDEOS["HUC6272"] = true
VIDEOS["I8244"] = true
VIDEOS["I8275"] = true
--VIDEOS+= M50458"] = true
--VIDEOS+= MB90082"] = true
--VIDEOS+= MB_VCU"] = true
VIDEOS["MC6845"] = true
VIDEOS["MC6847"] = true
VIDEOS["MSM6222B"] = true
VIDEOS["MSM6255"] = true
VIDEOS["MOS6566"] = true
VIDEOS["PC_VGA"] = true
--VIDEOS+= POLY"] = true
VIDEOS["PSX"] = true
VIDEOS["RAMDAC"] = true
VIDEOS["S2636"] = true
VIDEOS["SAA5050"] = true
VIDEOS["SED1200"] = true
VIDEOS["SED1330"] = true
VIDEOS["SED1520"] = true
VIDEOS["SNES_PPU"] = true
VIDEOS["STVVDP"] = true
VIDEOS["T6A04"] = true
VIDEOS["TEA1002"] = true
--VIDEOS+= TLC34076"] = true
--VIDEOS+= TMS34061"] = true
VIDEOS["TMS3556"] = true
VIDEOS["TMS9927"] = true
VIDEOS["TMS9928A"] = true
VIDEOS["UPD3301"] = true
VIDEOS["UPD7220"] = true
VIDEOS["UPD7227"] = true
VIDEOS["V9938"] = true
VIDEOS["VIC4567"] = true
--VIDEOS+= VOODOO"] = true
VIDEOS["SCN2674"] = true

--------------------------------------------------
-- specify available machine cores
--------------------------------------------------

MACHINES["AKIKO"] = true
MACHINES["AUTOCONFIG"] = true
MACHINES["CR511B"] = true
MACHINES["DMAC"] = true
MACHINES["GAYLE"] = true
--MACHINES["NCR53C7XX"] = true
--MACHINES["LSI53C810"] = true
MACHINES["6522VIA"] = true
--MACHINES["TPI6525"] = true
--MACHINES["RIOT6532"] = true
MACHINES["6821PIA"] = true
MACHINES["6840PTM"] = true
MACHINES["68561MPCC"] = true
--MACHINES["ACIA6850"] = true
MACHINES["68681"] = true
MACHINES["7200FIFO"] = true
MACHINES["8530SCC"] = true
--MACHINES["TTL74123"] = true
--MACHINES["TTL74145"] = true
--MACHINES["TTL74148"] = true
--MACHINES["TTL74153"] = true
--MACHINES["TTL74181"] = true
--MACHINES["TTL7474"] = true
--MACHINES["KBDC8042"] = true
--MACHINES["I8257"] = true
MACHINES["AAKARTDEV"] = true
MACHINES["ACIA6850"] = true
MACHINES["ADC0808"] = true
MACHINES["ADC083X"] = true
MACHINES["ADC1038"] = true
MACHINES["ADC1213X"] = true
MACHINES["AICARTC"] = true
MACHINES["AM53CF96"] = true
MACHINES["AM9517A"] = true
MACHINES["AMIGAFDC"] = true
MACHINES["AT_KEYBC"] = true
MACHINES["AT28C16"] = true
MACHINES["AT29040"] = true
MACHINES["AT45DBXX"] = true
MACHINES["ATAFLASH"] = true
MACHINES["AY31015"] = true
MACHINES["BANKDEV"] = true
MACHINES["CDP1852"] = true
MACHINES["CDP1871"] = true
MACHINES["CMOS40105"] = true
--MACHINES["CDU76S"] = true
MACHINES["COM8116"] = true
MACHINES["CR589"] = true
MACHINES["CS4031"] = true
MACHINES["CS8221"] = true
MACHINES["DP8390"] = true
--MACHINES["DS1204"] = true
MACHINES["DS1302"] = true
MACHINES["DS1315"] = true
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
MACHINES["HD63450"] = true
MACHINES["HD64610"] = true
MACHINES["I2CMEM"] = true
MACHINES["I80130"] = true
MACHINES["I8089"] = true
MACHINES["I8155"] = true
MACHINES["I8212"] = true
MACHINES["I8214"] = true
MACHINES["I8243"] = true
MACHINES["I8251"] = true
MACHINES["I8255"] = true
MACHINES["I8257"] = true
MACHINES["I8271"] = true
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
MACHINES["KB3600"] = true
MACHINES["KBDC8042"] = true
MACHINES["KR2376"] = true
MACHINES["LATCH8"] = true
MACHINES["LC89510"] = true
MACHINES["LDPR8210"] = true
MACHINES["LDSTUB"] = true
MACHINES["LDV1000"] = true
MACHINES["LDVP931"] = true
MACHINES["LH5810"] = true
MACHINES["LINFLASH"] = true
--MACHINES["LPCI"] = true
MACHINES["LSI53C810"] = true
MACHINES["M68307"] = true
MACHINES["M68340"] = true
MACHINES["M6M80011AP"] = true
MACHINES["MATSUCD"] = true
MACHINES["MB14241"] = true
MACHINES["MB3773"] = true
MACHINES["MB8421"] = true
MACHINES["MB87078"] = true
MACHINES["MB8795"] = true
MACHINES["MB89352"] = true
MACHINES["MB89371"] = true
MACHINES["MC146818"] = true
MACHINES["MC2661"] = true
MACHINES["MC6843"] = true
MACHINES["MC6846"] = true
MACHINES["MC6852"] = true
MACHINES["MC6854"] = true
MACHINES["MC68328"] = true
MACHINES["MC68901"] = true
MACHINES["MCCS1850"] = true
--MACHINES["M68307"] = true
--MACHINES["M68340"] = true
MACHINES["MCF5206E"] = true
MACHINES["MICROTOUCH"] = true
MACHINES["MIOT6530"] = true
MACHINES["MM58167"] = true
MACHINES["MM58274C"] = true
MACHINES["MM74C922"] = true
MACHINES["MOS6526"] = true
MACHINES["MOS6529"] = true
--MACHINES["MIOT6530"] = true
MACHINES["MOS6551"] = true
MACHINES["MOS6702"] = true
MACHINES["MOS8706"] = true
MACHINES["MOS8722"] = true
MACHINES["MOS8726"] = true
MACHINES["MPU401"] = true
MACHINES["MSM5832"] = true
MACHINES["MSM58321"] = true
MACHINES["MSM6242"] = true
MACHINES["NCR5380"] = true
MACHINES["NCR5380N"] = true
MACHINES["NCR5390"] = true
MACHINES["NCR539x"] = true
MACHINES["NCR53C7XX"] = true
MACHINES["NMC9306"] = true
MACHINES["NSC810"] = true
MACHINES["NSCSI"] = true
MACHINES["OMTI5100"] = true
MACHINES["PC_FDC"] = true
MACHINES["PC_LPT"] = true
MACHINES["PCCARD"] = true
MACHINES["PCF8593"] = true
MACHINES["PCKEYBRD"] = true
MACHINES["PIC8259"] = true
MACHINES["PIT8253"] = true
MACHINES["PLA"] = true
--MACHINES["PROFILE"] = true
MACHINES["R64H156"] = true
MACHINES["RF5C296"] = true
MACHINES["RIOT6532"] = true
MACHINES["ROC10937"] = true
MACHINES["RP5C01"] = true
MACHINES["RP5C15"] = true
MACHINES["RP5H01"] = true
MACHINES["RTC4543"] = true
MACHINES["RTC65271"] = true
MACHINES["RTC9701"] = true
--MACHINES["S2636"] = true
MACHINES["S3520CF"] = true
MACHINES["S3C2400"] = true
MACHINES["S3C2410"] = true
MACHINES["S3C2440"] = true
MACHINES["S3C44B0"] = true
MACHINES["SATURN"] = true
--MACHINES["SCSI"] = true
MACHINES["SCUDSP"] = true
MACHINES["SECFLASH"] = true
MACHINES["SEIBU_COP"] = true
--MACHINES["SERFLASH"] = true
MACHINES["SMC91C9X"] = true
MACHINES["SMPC"] = true
MACHINES["STVCD"] = true
MACHINES["TC0091LVC"] = true
MACHINES["TIMEKPR"] = true
MACHINES["TMP68301"] = true
MACHINES["TMS5501"] = true
MACHINES["TMS6100"] = true
MACHINES["TMS9901"] = true
MACHINES["TMS9902"] = true
MACHINES["TPI6525"] = true
MACHINES["TTL74123"] = true
MACHINES["TTL74145"] = true
MACHINES["TTL74148"] = true
MACHINES["TTL74153"] = true
MACHINES["TTL74181"] = true
MACHINES["TTL7474"] = true
MACHINES["UPD1990A"] = true
--MACHINES["UPD4992"] = true
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
MACHINES["WD7600"] = true
MACHINES["X2212"] = true
MACHINES["X76F041"] = true
MACHINES["X76F100"] = true
MACHINES["YM2148"] = true
MACHINES["Z80CTC"] = true
MACHINES["Z80DART"] = true
MACHINES["Z80DMA"] = true
MACHINES["Z80PIO"] = true
MACHINES["Z80STI"] = true
MACHINES["Z8536"] = true
--MACHINES["SECFLASH"] = true
--MACHINES["PCCARD"] = true
MACHINES["SMC92X4"] = true
MACHINES["HDC9234"] = true
MACHINES["TI99_HD"] = true
MACHINES["STRATA"] = true
MACHINES["STEPPERS"] = true
MACHINES["CORVUSHD"] = true
MACHINES["WOZFDC"] = true
MACHINES["DIABLO_HD"] = true

--------------------------------------------------
-- specify available bus cores
--------------------------------------------------

BUSES["A1BUS"] = true
BUSES["A2BUS"] = true
BUSES["A7800"] = true
BUSES["A800"] = true
BUSES["A8SIO"] = true
BUSES["ABCBUS"] = true
BUSES["ABCKB"] = true
BUSES["ADAM"] = true
BUSES["ADAMNET"] = true
BUSES["APF"] = true
BUSES["ARCADIA"] = true
BUSES["ASTROCADE"] = true
BUSES["BML3"] = true
BUSES["BW2"] = true
BUSES["C64"] = true
BUSES["CBM2"] = true
BUSES["CBMIEC"] = true
BUSES["CENTRONICS"] = true
BUSES["CHANNELF"] = true
BUSES["COCO"] = true
BUSES["COLECO"] = true
BUSES["COMPUCOLOR"] = true
BUSES["COMX35"] = true
BUSES["CPC"] = true
BUSES["CRVISION"] = true
BUSES["DMV"] = true
BUSES["ECBBUS"] = true
BUSES["ECONET"] = true
BUSES["EP64"] = true
BUSES["EPSON_SIO"] = true
BUSES["GAMEBOY"] = true
BUSES["GAMEGEAR"] = true
BUSES["GBA"] = true
BUSES["GENERIC"] = true
BUSES["IEEE488"] = true
BUSES["IMI7000"] = true
BUSES["INTV"] = true
BUSES["IQ151"] = true
BUSES["ISA"] = true
BUSES["ISBX"] = true
BUSES["KC"] = true
BUSES["LPCI"] = true
BUSES["MACPDS"] = true
BUSES["MIDI"] = true
BUSES["MEGADRIVE"] = true
BUSES["MSX_SLOT"] = true
BUSES["NEOGEO"] = true
BUSES["NES"] = true
BUSES["NES_CTRL"] = true
BUSES["NUBUS"] = true
BUSES["O2"] = true
BUSES["ORICEXT"] = true
BUSES["PCE"] = true
BUSES["PC_JOY"] = true
BUSES["PC_KBD"] = true
BUSES["PET"] = true
BUSES["PLUS4"] = true
BUSES["PSX_CONTROLLER"] = true
BUSES["QL"] = true
BUSES["RS232"] = true
BUSES["S100"] = true
BUSES["SATURN"] = true
BUSES["SCSI"] = true
BUSES["SCV"] = true
BUSES["SEGA8"] = true
BUSES["SMS_CTRL"] = true
BUSES["SMS_EXP"] = true
BUSES["SNES"] = true
BUSES["SNES_CTRL"] = true
BUSES["SPC1000"] = true
BUSES["TI99PEB"] = true
BUSES["TVC"] = true
BUSES["VBOY"] = true
BUSES["VC4000"] = true
BUSES["VCS"] = true
BUSES["VCS_CTRL"] = true
BUSES["VECTREX"] = true
BUSES["VIC10"] = true
BUSES["VIC20"] = true
BUSES["VIDBRAIN"] = true
BUSES["VIP"] = true
BUSES["VTECH_IOEXP"] = true
BUSES["VTECH_MEMEXP"] = true
BUSES["WANGPC"] = true
BUSES["WSWAN"] = true
BUSES["X68K"] = true
BUSES["Z88"] = true
BUSES["ZORRO"] = true

--------------------------------------------------
-- this is the list of driver libraries that
-- comprise MESS plus messdriv.*", which contains
-- the list of drivers
--------------------------------------------------
function linkProjects(_target, _subtarget)
	links {
		"acorn",
		"act",
		"adc",
		"alesis",
		"altos",
		"amiga",
		"amstrad",
		"apf",
		"apollo",
		"apple",
		"applied",
		"arcadia",
		"ascii",
		"at",
		"atari",
		"att",
		"bally",
		"bandai",
		"banctec",
		"be",
		"bnpo",
		"bondwell",
		"booth",
		"camputers",
		"canon",
		"cantab",
		"casio",
		"cbm",
		"cccp",
		"cce",
		"ccs",
		"chromatics",
		"coleco",
		"cromemco",
		"comx",
		"concept",
		"conitec",
		"cybiko",
		"dai",
		"ddr",
		"dec",
		"dicksmth",
		"dms",
		"dragon",
		"drc",
		"eaca",
		"einis",
		"elektor",
		"elektrka",
		"ensoniq",
		"enterprise",
		"entex",
		"epoch",
		"epson",
		"exidy",
		"fairch",
		"fidelity",
		"fujitsu",
		"funtech",
		"galaxy",
		"gamepark",
		"gi",
		"grundy",
		"hartung",
		"heathkit",
		"hec2hrp",
		"hegener",
		"hitachi",
		"homebrew",
		"homelab",
		"hp",
		"imp",
		"intel",
		"interton",
		"intv",
		"isc",
		"kaypro",
		"koei",
		"kyocera",
		"luxor",
		"magnavox",
		"makerbot",
		"marx",
		"matsushi",
		"mattel",
		"mb",
		"mchester",
		"memotech",
		"mgu",
		"microkey",
		"mit",
		"mits",
		"mitsubishi",
		"morrow",
		"mos",
		"motorola",
		"multitch",
		"nakajima",
		"nascom",
		"ne",
		"nec",
		"netronic",
		"next",
		"nintendo",
		"nokia",
		"northstar",
		"novag",
		"olivetti",
		"omnibyte",
		"orion",
		"osborne",
		"osi",
		"palm",
		"parker",
		"pc",
		"pdp1",
		"pel",
		"philips",
		"pitronic",
		"poly88",
		"psion",
		"radio",
		"rca",
		"rm",
		"robotron",
		"rockwell",
		"roland",
		"sage",
		"samcoupe",
		"samsung",
		"sanyo",
		"sega",
		"sgi",
		"sharp",
		"siemens",
		"sinclair",
		"skeleton",
		"slicer",
		"snk",
		"sony",
		"sord",
		"special",
		"sun",
		"svi",
		"svision",
		"swtpc09",
		"synertec",
		"ta",
		"tandberg",
		"tangerin",
		"tatung",
		"teamconc",
		"tektroni",
		"telenova",
		"telercas",
		"televideo",
		"tem",
		"tesla",
		"test",
		"thomson",
		"ti",
		"tiger",
		"tigertel",
		"tiki",
		"tomy",
		"toshiba",
		"trainer",
		"trs",
		"ultratec",
		"unisys",
		"veb",
		"vidbrain",
		"videoton",
		"visual",
		"votrax",
		"vtech",
		"wang",
		"wavemate",
		"xerox",
		"xussrpc",
		"yamaha",
		"zenith",
		"zpa",
		"zvt",
		"shared",
		"mameshared",
	}
end

function createMESSProjects(_target, _subtarget, _name)
	project (_name)
	targetsubdir(_target .."_" .. _subtarget)
	kind "StaticLib"
	uuid (os.uuid("drv-" .. _target .."_" .. _subtarget .. "_" .._name))
	
	options {
		"ForceCPP",
	}
	
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/mess",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/zlib",
		GEN_DIR  .. "mess/layout",
		GEN_DIR  .. "mame/layout",
		GEN_DIR .. "emu/cpu/m68000",
	}

	includeosd()
end
	
function createProjects(_target, _subtarget)
--------------------------------------------------
-- the following files are MAME components and
-- shared across a number of drivers
--
-- a310.c (MESS), aristmk5.c, ertictac.c (MAME)
-- amiga.c (MESS), alg.c, arcadia.c, cubo.c, mquake.c, upscope.c (MAME)
-- a2600.c (MESS), tourtabl.c (MAME)
-- atari400.c (MESS), bartop52.c, maxaflex.c (MAME)
-- jaguar.c (MAME)
-- astrocde.c (MAME+MESS), g627.c
-- cps1.c (MAME + MESS), cbaseball.c, mitchell.c (MAME)
-- pk8000.c (MESS), photon.c (MAME)
-- nes.c (MESS), cham23.c, famibox.c, multigam.c, playch10.c, vsnes.c (MAME)
-- snes.c (MESS), nss.c, sfcbox.c, snesb.c (MAME)
-- n64.c (MESS), aleck64.c (MAME)
-- megadriv.c, segapico.c (MESS), hshavoc.c, megadrvb.c, megaplay.c, megatech.c, puckpkmn.c, segac2.c, segas18.c (MAME)
-- dccons.c (MESS), naomi.c (MAME)
-- ng_aes.c (MESS), midas.c, neogeo.c, neogeo_noslot.c, neoprint.c (MAME)
-- cdi.c (MESS + MAME)
-- 3do.c (MESS + MAME), konamim2.c (MAME)
-- vectrex.c (MESS + MAME)
-- cps1.c (MESS + MAME)
--------------------------------------------------

createMESSProjects(_target, _subtarget, "mameshared")
files { 
	MAME_DIR .. "src/mame/machine/archimds.*",  
	MAME_DIR .. "src/mame/video/archimds.*",    
	MAME_DIR .. "src/mame/machine/amiga.*",     
	MAME_DIR .. "src/mame/video/amiga.*",       
	MAME_DIR .. "src/mame/video/amigaaga.*",    
	MAME_DIR .. "src/mame/video/tia.*",         
	MAME_DIR .. "src/mame/machine/atari.*",     
	MAME_DIR .. "src/mame/video/atari.*",       
	MAME_DIR .. "src/mame/video/antic.*",       
	MAME_DIR .. "src/mame/video/gtia.*",        
	MAME_DIR .. "src/mame/drivers/jaguar.*",    
	MAME_DIR .. "src/mame/audio/jaguar.*",      
	MAME_DIR .. "src/mame/video/jaguar.*",      
	MAME_DIR .. "src/mame/video/astrocde.*",    
	MAME_DIR .. "src/mame/machine/kabuki.*",    
	MAME_DIR .. "src/mame/video/pk8000.*",      
	MAME_DIR .. "src/mame/video/ppu2c0x.*",     
	MAME_DIR .. "src/mame/machine/snes.*",      
	MAME_DIR .. "src/mame/audio/snes_snd.*",    
	MAME_DIR .. "src/mame/machine/n64.*",       
	MAME_DIR .. "src/mame/video/n64.*",         
	MAME_DIR .. "src/mame/video/rdpblend.*",    
	MAME_DIR .. "src/mame/video/rdptpipe.*",    
	MAME_DIR .. "src/mame/video/rdpspn16.*",    
	MAME_DIR .. "src/mame/machine/megadriv.*",  
	MAME_DIR .. "src/mame/drivers/naomi.*",     
	MAME_DIR .. "src/mame/machine/awboard.*",   
	MAME_DIR .. "src/mame/machine/dc.*",        
	MAME_DIR .. "src/mame/machine/dc-ctrl.*",   
	MAME_DIR .. "src/mame/machine/gdrom.*",     
	MAME_DIR .. "src/mame/machine/jvs13551.*",  
	MAME_DIR .. "src/mame/machine/maple-dc.*",  
	MAME_DIR .. "src/mame/machine/mapledev.*",  
	MAME_DIR .. "src/mame/machine/mie.*",       
	MAME_DIR .. "src/mame/machine/naomi.*",     
	MAME_DIR .. "src/mame/machine/naomibd.*",   
	MAME_DIR .. "src/mame/machine/naomig1.*",   
	MAME_DIR .. "src/mame/machine/naomigd.*",   
	MAME_DIR .. "src/mame/machine/naomim1.*",   
	MAME_DIR .. "src/mame/machine/naomim2.*",   
	MAME_DIR .. "src/mame/machine/naomim4.*",   
	MAME_DIR .. "src/mame/machine/naomirom.*",  
	MAME_DIR .. "src/mame/machine/315-5881_crypt.*",  
	MAME_DIR .. "src/mame/video/powervr2.*",    
	MAME_DIR .. "src/mame/drivers/neogeo.*",    
	MAME_DIR .. "src/mame/machine/neoboot.*",   
	MAME_DIR .. "src/mame/machine/neocrypt.*",  
	MAME_DIR .. "src/mame/machine/neoprot.*",   
	MAME_DIR .. "src/mame/machine/ng_memcard.*",
	MAME_DIR .. "src/mame/video/neogeo.*",      
	MAME_DIR .. "src/mame/video/neogeo_spr.*",  
	MAME_DIR .. "src/mame/drivers/cdi.*",       
	MAME_DIR .. "src/mame/machine/cdi070.*",    
	MAME_DIR .. "src/mame/machine/cdicdic.*",   
	MAME_DIR .. "src/mame/machine/cdislave.*",  
	MAME_DIR .. "src/mame/video/mcd212.*",      
	MAME_DIR .. "src/mame/drivers/3do.*",       
	MAME_DIR .. "src/mame/machine/3do.*",       
	MAME_DIR .. "src/mame/drivers/konamim2.*",  
	MAME_DIR .. "src/mame/drivers/vectrex.*",   
	MAME_DIR .. "src/mame/machine/vectrex.*",   
	MAME_DIR .. "src/mame/video/vectrex.*",     
	MAME_DIR .. "src/mame/drivers/cps1.*",      
	MAME_DIR .. "src/mame/video/cps1.*",        
}

--------------------------------------------------
-- the following files are general components and
-- shared across a number of drivers
--------------------------------------------------
createMESSProjects(_target, _subtarget, "shared")
files { 
	MAME_DIR .. "src/mess/audio/mea8000.*",     
	MAME_DIR .. "src/mess/machine/appldriv.*",  
	MAME_DIR .. "src/mess/machine/applefdc.*",  
	MAME_DIR .. "src/mess/machine/microdrv.*",  
	MAME_DIR .. "src/mess/machine/smartmed.*",  
	MAME_DIR .. "src/mess/machine/sonydriv.*",  
	MAME_DIR .. "src/mess/machine/teleprinter.*", 
	MAME_DIR .. "src/mess/machine/z80bin.*",    
}
--------------------------------------------------
-- manufacturer-specific groupings for drivers
--------------------------------------------------

createMESSProjects(_target, _subtarget, "acorn")
files {             
	MAME_DIR .. "src/mess/drivers/a310.*",      
	MAME_DIR .. "src/mess/drivers/a6809.*",     
	MAME_DIR .. "src/mess/drivers/acrnsys1.*",  
	MAME_DIR .. "src/mess/drivers/atom.*",      
	MAME_DIR .. "src/mess/drivers/bbc.*",
	MAME_DIR .. "src/mess/machine/bbc.*",
	MAME_DIR .. "src/mess/video/bbc.*", 
	MAME_DIR .. "src/mess/drivers/bbcbc.*",     
	MAME_DIR .. "src/mess/drivers/electron.*",
	MAME_DIR .. "src/mess/machine/electron.*",
	MAME_DIR .. "src/mess/video/electron.*", 
	MAME_DIR .. "src/mess/drivers/riscpc.*",    
	MAME_DIR .. "src/mess/drivers/z88.*",
	MAME_DIR .. "src/mess/machine/upd65031.*",
	MAME_DIR .. "src/mess/video/z88.*", 
}

createMESSProjects(_target, _subtarget, "act")
files {               
	MAME_DIR .. "src/mess/drivers/apricot.*",   
	MAME_DIR .. "src/mess/drivers/apricotf.*",  
	MAME_DIR .. "src/mess/drivers/apricotp.*",  
	MAME_DIR .. "src/mess/machine/apricotkb.*", 
	MAME_DIR .. "src/mess/drivers/victor9k.*",
	MAME_DIR .. "src/mess/machine/victor9kb.*",
	MAME_DIR .. "src/mess/machine/victor9k_fdc.*", 
}

createMESSProjects(_target, _subtarget, "adc")
files {               
	MAME_DIR .. "src/mess/drivers/super6.*",    
	MAME_DIR .. "src/mess/drivers/superslave.*",
}

createMESSProjects(_target, _subtarget, "alesis")
files {            
	MAME_DIR .. "src/mess/drivers/alesis.*",
	MAME_DIR .. "src/mess/audio/alesis.*",
	MAME_DIR .. "src/mess/video/alesis.*", 
}

createMESSProjects(_target, _subtarget, "altos")
files {             
	MAME_DIR .. "src/mess/drivers/altos5.*",    
}

createMESSProjects(_target, _subtarget, "amiga")
files {             
	MAME_DIR .. "src/mess/drivers/amiga.*",
	MAME_DIR .. "src/mess/machine/amigakbd.*", 
}

createMESSProjects(_target, _subtarget, "amstrad")
files {           
	MAME_DIR .. "src/mess/drivers/amstrad.*",
	MAME_DIR .. "src/mess/machine/amstrad.*", 
	MAME_DIR .. "src/mess/drivers/amstr_pc.*",  
	MAME_DIR .. "src/mess/drivers/nc.*",
	MAME_DIR .. "src/mess/machine/nc.*",
	MAME_DIR .. "src/mess/video/nc.*", 
	MAME_DIR .. "src/mess/drivers/pc1512.*",
	MAME_DIR .. "src/mess/machine/pc1512kb.*",
	MAME_DIR .. "src/mess/video/pc1512.*", 
	MAME_DIR .. "src/mess/drivers/pcw.*",
	MAME_DIR .. "src/mess/video/pcw.*", 
	MAME_DIR .. "src/mess/drivers/pcw16.*",
	MAME_DIR .. "src/mess/video/pcw16.*", 
	MAME_DIR .. "src/mess/drivers/pda600.*",    
}

createMESSProjects(_target, _subtarget, "apf")
files {               
	MAME_DIR .. "src/mess/drivers/apf.*",       
}

createMESSProjects(_target, _subtarget, "apollo")
files {            
	MAME_DIR .. "src/mess/drivers/apollo.*",
	MAME_DIR .. "src/mess/machine/apollo.*",
	MAME_DIR .. "src/mess/machine/apollo_dbg.*",
	MAME_DIR .. "src/mess/machine/apollo_kbd.*",
	MAME_DIR .. "src/mess/video/apollo.*", 
}

createMESSProjects(_target, _subtarget, "apple")
files {             
	MAME_DIR .. "src/mess/drivers/apple1.*",
	MAME_DIR .. "src/mess/machine/apple1.*",
	MAME_DIR .. "src/mess/video/apple1.*", 
	MAME_DIR .. "src/mess/drivers/apple2.*",
	MAME_DIR .. "src/mess/drivers/apple2e.*",
	MAME_DIR .. "src/mess/machine/apple2.*",
	MAME_DIR .. "src/mess/video/apple2.*", 
	MAME_DIR .. "src/mess/drivers/tk2000.*",    
	MAME_DIR .. "src/mess/drivers/apple2gs.*",
	MAME_DIR .. "src/mess/machine/apple2gs.*",
	MAME_DIR .. "src/mess/video/apple2gs.*", 
	MAME_DIR .. "src/mess/drivers/apple3.*",
	MAME_DIR .. "src/mess/machine/apple3.*",
	MAME_DIR .. "src/mess/video/apple3.*", 
	MAME_DIR .. "src/mess/drivers/lisa.*",
	MAME_DIR .. "src/mess/machine/lisa.*", 
	MAME_DIR .. "src/mess/drivers/mac.*",
	MAME_DIR .. "src/mess/audio/mac.*",
	MAME_DIR .. "src/mess/machine/egret.*",
	MAME_DIR .. "src/mess/machine/mac.*",
	MAME_DIR .. "src/mess/machine/macadb.*",
	MAME_DIR .. "src/mess/machine/macrtc.*",
	MAME_DIR .. "src/mess/machine/mackbd.*",
	MAME_DIR .. "src/mess/machine/swim.*",
	MAME_DIR .. "src/mess/video/mac.*", 
	MAME_DIR .. "src/mess/drivers/macpci.*",
	MAME_DIR .. "src/mess/machine/macpci.*", 
	MAME_DIR .. "src/mess/machine/cuda.*",      
}

createMESSProjects(_target, _subtarget, "applied")
files {           
	MAME_DIR .. "src/mess/drivers/mbee.*",
	MAME_DIR .. "src/mess/machine/mbee.*",
	MAME_DIR .. "src/mess/video/mbee.*", 
}

createMESSProjects(_target, _subtarget, "arcadia")
files {           
	MAME_DIR .. "src/mess/drivers/arcadia.*",
	MAME_DIR .. "src/mess/audio/arcadia.*",
	MAME_DIR .. "src/mess/video/arcadia.*", 
}

createMESSProjects(_target, _subtarget, "ascii")
files {             
	MAME_DIR .. "src/mess/drivers/msx.*",
	MAME_DIR .. "src/mess/machine/msx.*",
	MAME_DIR .. "src/mess/machine/msx_matsushita.*",
	MAME_DIR .. "src/mess/machine/msx_s1985.*",
	MAME_DIR .. "src/mess/machine/msx_switched.*",
	MAME_DIR .. "src/mess/machine/msx_systemflags.*", 
}

createMESSProjects(_target, _subtarget, "at")
files {                
	MAME_DIR .. "src/mess/drivers/at.*",
	MAME_DIR .. "src/mess/machine/at.*", 
	MAME_DIR .. "src/mess/drivers/ct486.*",     
}

createMESSProjects(_target, _subtarget, "atari")
files {             
	MAME_DIR .. "src/mess/drivers/a2600.*",     
	MAME_DIR .. "src/mess/drivers/a7800.*",
	MAME_DIR .. "src/mess/video/maria.*", 
	MAME_DIR .. "src/mess/drivers/atari400.*",
	MAME_DIR .. "src/mess/machine/atarifdc.*", 
	MAME_DIR .. "src/mess/drivers/atarist.*",
	MAME_DIR .. "src/mess/video/atarist.*", 
	MAME_DIR .. "src/mess/drivers/lynx.*",
	MAME_DIR .. "src/mess/audio/lynx.*",
	MAME_DIR .. "src/mess/machine/lynx.*", 
	MAME_DIR .. "src/mess/drivers/portfoli.*",  
}

createMESSProjects(_target, _subtarget, "att")
files {               
	MAME_DIR .. "src/mess/drivers/unixpc.*",    
}

createMESSProjects(_target, _subtarget, "bally")
files {             
	MAME_DIR .. "src/mess/drivers/astrocde.*",  
}

createMESSProjects(_target, _subtarget, "banctec")
files {           
	MAME_DIR .. "src/mess/drivers/banctec.*",   
}

createMESSProjects(_target, _subtarget, "bandai")
files {            
	MAME_DIR .. "src/mess/drivers/sv8000.*",    
	MAME_DIR .. "src/mess/drivers/rx78.*",      
	MAME_DIR .. "src/mess/drivers/wswan.*",
	MAME_DIR .. "src/mess/audio/wswan_snd.*",
	MAME_DIR .. "src/mess/machine/wswan.*",
	MAME_DIR .. "src/mess/video/wswan_video.*", 
}

createMESSProjects(_target, _subtarget, "be")
files {                
	MAME_DIR .. "src/mess/drivers/bebox.*",
	MAME_DIR .. "src/mess/machine/bebox.*", 
}

createMESSProjects(_target, _subtarget, "bnpo")
files {              
	MAME_DIR .. "src/mess/drivers/b2m.*",
	MAME_DIR .. "src/mess/machine/b2m.*",
	MAME_DIR .. "src/mess/video/b2m.*", 
}

createMESSProjects(_target, _subtarget, "bondwell")
files {          
	MAME_DIR .. "src/mess/drivers/bw12.*",      
	MAME_DIR .. "src/mess/drivers/bw2.*",       
}

createMESSProjects(_target, _subtarget, "booth")
files {             
	MAME_DIR .. "src/mess/drivers/apexc.*",     
}

createMESSProjects(_target, _subtarget, "camputers")
files {         
	MAME_DIR .. "src/mess/drivers/camplynx.*",  
}

createMESSProjects(_target, _subtarget, "canon")
files {             
	MAME_DIR .. "src/mess/drivers/cat.*",       
	MAME_DIR .. "src/mess/drivers/x07.*",       
}

createMESSProjects(_target, _subtarget, "cantab")
files {            
	MAME_DIR .. "src/mess/drivers/ace.*",       
}

createMESSProjects(_target, _subtarget, "casio")
files {             
	MAME_DIR .. "src/mess/drivers/casloopy.*",  
	MAME_DIR .. "src/mess/drivers/cfx9850.*",   
	MAME_DIR .. "src/mess/drivers/fp200.*",     
	MAME_DIR .. "src/mess/drivers/fp1100.*",    
	MAME_DIR .. "src/mess/drivers/fp6000.*",    
	MAME_DIR .. "src/mess/drivers/pb1000.*",    
	MAME_DIR .. "src/mess/drivers/pv1000.*",    
	MAME_DIR .. "src/mess/drivers/pv2000.*",    
}

createMESSProjects(_target, _subtarget, "cbm")
files {               
	MAME_DIR .. "src/mess/drivers/c128.*",      
	MAME_DIR .. "src/mess/drivers/c64.*",       
	MAME_DIR .. "src/mess/drivers/c64dtv.*",    
	MAME_DIR .. "src/mess/drivers/c65.*", 
	MAME_DIR .. "src/mess/drivers/c900.*",      
	MAME_DIR .. "src/mess/drivers/cbm2.*",      
	MAME_DIR .. "src/mess/drivers/clcd.*",      
	MAME_DIR .. "src/mess/drivers/pet.*",       
	MAME_DIR .. "src/mess/drivers/plus4.*",     
	MAME_DIR .. "src/mess/drivers/vic10.*",     
	MAME_DIR .. "src/mess/drivers/vic20.*",     
	MAME_DIR .. "src/mess/machine/cbm_snqk.*",  
}

createMESSProjects(_target, _subtarget, "cccp")
files {              
	MAME_DIR .. "src/mess/drivers/argo.*",      
	MAME_DIR .. "src/mess/drivers/cm1800.*",    
	MAME_DIR .. "src/mess/drivers/lviv.*",
	MAME_DIR .. "src/mess/machine/lviv.*",
	MAME_DIR .. "src/mess/video/lviv.*", 
	MAME_DIR .. "src/mess/drivers/mikro80.*",
	MAME_DIR .. "src/mess/machine/mikro80.*",
	MAME_DIR .. "src/mess/video/mikro80.*", 
	MAME_DIR .. "src/mess/drivers/pk8000.*",    
	MAME_DIR .. "src/mess/drivers/pk8020.*",
	MAME_DIR .. "src/mess/machine/pk8020.*",
	MAME_DIR .. "src/mess/video/pk8020.*", 
	MAME_DIR .. "src/mess/drivers/pyl601.*",    
	MAME_DIR .. "src/mess/drivers/sm1800.*",    
	MAME_DIR .. "src/mess/drivers/uknc.*",      
	MAME_DIR .. "src/mess/drivers/unior.*",     
	MAME_DIR .. "src/mess/drivers/ut88.*",
	MAME_DIR .. "src/mess/machine/ut88.*",
	MAME_DIR .. "src/mess/video/ut88.*", 
	MAME_DIR .. "src/mess/drivers/vector06.*",
	MAME_DIR .. "src/mess/machine/vector06.*",
	MAME_DIR .. "src/mess/video/vector06.*", 
	MAME_DIR .. "src/mess/drivers/vta2000.*",   
}

createMESSProjects(_target, _subtarget, "cce")
files {               
	MAME_DIR .. "src/mess/drivers/mc1000.*",    
}

createMESSProjects(_target, _subtarget, "ccs")
files {               
	MAME_DIR .. "src/mess/drivers/ccs2810.*",   
	MAME_DIR .. "src/mess/drivers/ccs300.*",    
}

createMESSProjects(_target, _subtarget, "chromatics")
files {        
	MAME_DIR .. "src/mess/drivers/cgc7900.*",
	MAME_DIR .. "src/mess/video/cgc7900.*", 
}

createMESSProjects(_target, _subtarget, "coleco")
files {            
	MAME_DIR .. "src/mess/drivers/adam.*",      
	MAME_DIR .. "src/mess/drivers/coleco.*",    
	MAME_DIR .. "src/mess/machine/coleco.*",    
}

createMESSProjects(_target, _subtarget, "cromemco")
files {          
	MAME_DIR .. "src/mess/drivers/c10.*",       
	MAME_DIR .. "src/mess/drivers/mcb216.*",    
}

createMESSProjects(_target, _subtarget, "comx")
files {              
	MAME_DIR .. "src/mess/drivers/comx35.*",
	MAME_DIR .. "src/mess/video/comx35.*", 
}

createMESSProjects(_target, _subtarget, "concept")
files {           
	MAME_DIR .. "src/mess/drivers/concept.*",
	MAME_DIR .. "src/mess/machine/concept.*", 
}

createMESSProjects(_target, _subtarget, "conitec")
files {           
	MAME_DIR .. "src/mess/drivers/prof180x.*",  
	MAME_DIR .. "src/mess/drivers/prof80.*",
	MAME_DIR .. "src/mess/machine/prof80mmu.*", 
}

createMESSProjects(_target, _subtarget, "cybiko")
files {            
	MAME_DIR .. "src/mess/drivers/cybiko.*",
	MAME_DIR .. "src/mess/machine/cybiko.*", 
}

createMESSProjects(_target, _subtarget, "dai")
files {               
	MAME_DIR .. "src/mess/drivers/dai.*",
	MAME_DIR .. "src/mess/audio/dai_snd.*",
	MAME_DIR .. "src/mess/machine/dai.*",
	MAME_DIR .. "src/mess/video/dai.*", 
}

createMESSProjects(_target, _subtarget, "ddr")
files {               
	MAME_DIR .. "src/mess/drivers/ac1.*",
	MAME_DIR .. "src/mess/machine/ac1.*",
	MAME_DIR .. "src/mess/video/ac1.*", 
	MAME_DIR .. "src/mess/drivers/bcs3.*",      
	MAME_DIR .. "src/mess/drivers/c80.*",       
	MAME_DIR .. "src/mess/drivers/huebler.*",   
	MAME_DIR .. "src/mess/drivers/jtc.*",       
	MAME_DIR .. "src/mess/drivers/kramermc.*",
	MAME_DIR .. "src/mess/machine/kramermc.*",
	MAME_DIR .. "src/mess/video/kramermc.*", 
	MAME_DIR .. "src/mess/drivers/llc.*",
	MAME_DIR .. "src/mess/machine/llc.*",
	MAME_DIR .. "src/mess/video/llc.*", 
	MAME_DIR .. "src/mess/drivers/nanos.*",     
	MAME_DIR .. "src/mess/drivers/pcm.*",       
	MAME_DIR .. "src/mess/drivers/vcs80.*",     
	MAME_DIR .. "src/mess/machine/k7659kb.*",   
}

createMESSProjects(_target, _subtarget, "dec")
files {               
	MAME_DIR .. "src/mess/drivers/dct11em.*",   
	MAME_DIR .. "src/mess/drivers/dectalk.*",   
	MAME_DIR .. "src/mess/drivers/pdp11.*",     
	MAME_DIR .. "src/mess/drivers/vax11.*",     
	MAME_DIR .. "src/mess/drivers/rainbow.*",   
	MAME_DIR .. "src/mess/drivers/vk100.*",     
	MAME_DIR .. "src/mess/drivers/vt100.*",     
	MAME_DIR .. "src/mess/drivers/vt220.*",     
	MAME_DIR .. "src/mess/drivers/vt240.*",     
	MAME_DIR .. "src/mess/drivers/vt320.*",     
	MAME_DIR .. "src/mess/drivers/vt520.*",     
	MAME_DIR .. "src/mess/machine/dec_lk201.*", 
	MAME_DIR .. "src/mess/machine/rx01.*",      
	MAME_DIR .. "src/mess/video/vtvideo.*",     
}

createMESSProjects(_target, _subtarget, "dicksmth")
files {          
	MAME_DIR .. "src/mess/drivers/super80.*",
	MAME_DIR .. "src/mess/machine/super80.*",
	MAME_DIR .. "src/mess/video/super80.*", 
}

createMESSProjects(_target, _subtarget, "dms")
files {               
	MAME_DIR .. "src/mess/drivers/dms5000.*",   
	MAME_DIR .. "src/mess/drivers/dms86.*",     
	MAME_DIR .. "src/mess/drivers/zsbc3.*",     
}

createMESSProjects(_target, _subtarget, "dragon")
files {            
	MAME_DIR .. "src/mess/drivers/dgn_beta.*",
	MAME_DIR .. "src/mess/machine/dgn_beta.*",
	MAME_DIR .. "src/mess/video/dgn_beta.*", 
}

createMESSProjects(_target, _subtarget, "drc")
files {               
	MAME_DIR .. "src/mess/drivers/zrt80.*",     
}

createMESSProjects(_target, _subtarget, "eaca")
files {              
	MAME_DIR .. "src/mess/drivers/cgenie.*",
	MAME_DIR .. "src/mess/machine/cgenie.*",
	MAME_DIR .. "src/mess/video/cgenie.*", 
}

createMESSProjects(_target, _subtarget, "einis")
files {             
	MAME_DIR .. "src/mess/drivers/pecom.*",
	MAME_DIR .. "src/mess/machine/pecom.*",
	MAME_DIR .. "src/mess/video/pecom.*", 
}

createMESSProjects(_target, _subtarget, "elektrka")
files {          
	MAME_DIR .. "src/mess/drivers/bk.*",
	MAME_DIR .. "src/mess/machine/bk.*",
	MAME_DIR .. "src/mess/video/bk.*", 
	MAME_DIR .. "src/mess/drivers/dvk_ksm.*",
	MAME_DIR .. "src/mess/machine/ms7004.*", 
	MAME_DIR .. "src/mess/drivers/mk85.*",      
	MAME_DIR .. "src/mess/drivers/mk90.*",      
}

createMESSProjects(_target, _subtarget, "elektor")
files {           
	MAME_DIR .. "src/mess/drivers/ec65.*",      
	MAME_DIR .. "src/mess/drivers/elekscmp.*",  
	MAME_DIR .. "src/mess/drivers/junior.*",    
}

createMESSProjects(_target, _subtarget, "ensoniq")
files {           
	MAME_DIR .. "src/mess/drivers/esq1.*",      
	MAME_DIR .. "src/mess/drivers/esq5505.*",   
	MAME_DIR .. "src/mess/drivers/esqasr.*",    
	MAME_DIR .. "src/mess/drivers/esqkt.*",     
	MAME_DIR .. "src/mess/drivers/esqmr.*",     
	MAME_DIR .. "src/mess/drivers/mirage.*",    
	MAME_DIR .. "src/mess/machine/esqpanel.*",  
	MAME_DIR .. "src/mess/machine/esqvfd.*",    
}

createMESSProjects(_target, _subtarget, "enterprise")
files {        
	MAME_DIR .. "src/mess/drivers/ep64.*",
	MAME_DIR .. "src/mess/audio/dave.*",
	MAME_DIR .. "src/mess/video/nick.*", 
}

createMESSProjects(_target, _subtarget, "entex")
files {             
	MAME_DIR .. "src/mess/drivers/advision.*",
	MAME_DIR .. "src/mess/machine/advision.*",
	MAME_DIR .. "src/mess/video/advision.*", 
}

createMESSProjects(_target, _subtarget, "epoch")
files {             
	MAME_DIR .. "src/mess/drivers/gamepock.*",
	MAME_DIR .. "src/mess/machine/gamepock.*", 
	MAME_DIR .. "src/mess/drivers/scv.*",
	MAME_DIR .. "src/mess/audio/upd1771.*", 
}

createMESSProjects(_target, _subtarget, "epson")
files {             
	MAME_DIR .. "src/mess/drivers/hx20.*",      
	MAME_DIR .. "src/mess/drivers/px4.*",       
	MAME_DIR .. "src/mess/drivers/px8.*",       
	MAME_DIR .. "src/mess/drivers/qx10.*",
	MAME_DIR .. "src/mess/machine/qx10kbd.*", 
}

createMESSProjects(_target, _subtarget, "exidy")
files {             
	MAME_DIR .. "src/mess/machine/sorcerer.*",
	MAME_DIR .. "src/mess/drivers/sorcerer.*", 
	MAME_DIR .. "src/mess/machine/micropolis.*",
}

createMESSProjects(_target, _subtarget, "fairch")
files {            
	MAME_DIR .. "src/mess/drivers/channelf.*",
	MAME_DIR .. "src/mess/audio/channelf.*",
	MAME_DIR .. "src/mess/video/channelf.*", 
}

createMESSProjects(_target, _subtarget, "fidelity")
files {          
	MAME_DIR .. "src/mess/drivers/csc.*",       
	MAME_DIR .. "src/mess/drivers/fidelz80.*",  
}

createMESSProjects(_target, _subtarget, "fujitsu")
files {           
	MAME_DIR .. "src/mess/drivers/fmtowns.*",
	MAME_DIR .. "src/mess/video/fmtowns.*",
	MAME_DIR .. "src/mess/machine/fm_scsi.*", 
	MAME_DIR .. "src/mess/drivers/fm7.*",
	MAME_DIR .. "src/mess/video/fm7.*", 
}

createMESSProjects(_target, _subtarget, "funtech")
files {           
	MAME_DIR .. "src/mess/drivers/supracan.*",  
}

createMESSProjects(_target, _subtarget, "galaxy")
files {            
	MAME_DIR .. "src/mess/drivers/galaxy.*",
	MAME_DIR .. "src/mess/machine/galaxy.*",
	MAME_DIR .. "src/mess/video/galaxy.*", 
}

createMESSProjects(_target, _subtarget, "gamepark")
files {          
	MAME_DIR .. "src/mess/drivers/gp2x.*",      
	MAME_DIR .. "src/mess/drivers/gp32.*",      
}

createMESSProjects(_target, _subtarget, "gi")
files {                
	MAME_DIR .. "src/mess/drivers/hh_pic16.*",  
}

createMESSProjects(_target, _subtarget, "grundy")
files {            
	MAME_DIR .. "src/mess/drivers/newbrain.*",
	MAME_DIR .. "src/mess/video/newbrain.*", 
}

createMESSProjects(_target, _subtarget, "hartung")
files {           
	MAME_DIR .. "src/mess/drivers/gmaster.*",   
}

createMESSProjects(_target, _subtarget, "heathkit")
files {          
	MAME_DIR .. "src/mess/drivers/et3400.*",    
	MAME_DIR .. "src/mess/drivers/h8.*",        
	MAME_DIR .. "src/mess/drivers/h19.*",       
	MAME_DIR .. "src/mess/drivers/h89.*",       
}

createMESSProjects(_target, _subtarget, "hegener")
files {           
	MAME_DIR .. "src/mess/drivers/glasgow.*",   
	MAME_DIR .. "src/mess/drivers/mephisto.*",  
	MAME_DIR .. "src/mess/drivers/mmodular.*",  
	MAME_DIR .. "src/mess/drivers/stratos.*",   
	MAME_DIR .. "src/mess/machine/mboard.*",    
}

createMESSProjects(_target, _subtarget, "hitachi")
files {           
	MAME_DIR .. "src/mess/drivers/b16.*",       
	MAME_DIR .. "src/mess/drivers/bmjr.*",      
	MAME_DIR .. "src/mess/drivers/bml3.*",      
	MAME_DIR .. "src/mess/drivers/hh_hmcs40.*", 
}

createMESSProjects(_target, _subtarget, "homebrew")
files {          
	MAME_DIR .. "src/mess/drivers/4004clk.*",   
	MAME_DIR .. "src/mess/drivers/68ksbc.*",    
	MAME_DIR .. "src/mess/drivers/craft.*",     
	MAME_DIR .. "src/mess/drivers/homez80.*",   
	MAME_DIR .. "src/mess/drivers/p112.*",      
	MAME_DIR .. "src/mess/drivers/phunsy.*",    
	MAME_DIR .. "src/mess/drivers/pimps.*",     
	MAME_DIR .. "src/mess/drivers/ravens.*",    
	MAME_DIR .. "src/mess/drivers/sbc6510.*",   
	MAME_DIR .. "src/mess/drivers/sitcom.*",    
	MAME_DIR .. "src/mess/drivers/slc1.*",      
	MAME_DIR .. "src/mess/drivers/uzebox.*",    
	MAME_DIR .. "src/mess/drivers/z80dev.*",    
}

createMESSProjects(_target, _subtarget, "homelab")
files {           
	MAME_DIR .. "src/mess/drivers/homelab.*",   
}

createMESSProjects(_target, _subtarget, "hp")
files {                
	MAME_DIR .. "src/mess/drivers/hp16500.*",   
	MAME_DIR .. "src/mess/drivers/hp48.*",      
	MAME_DIR .. "src/mess/machine/hp48.*",      
	MAME_DIR .. "src/mess/video/hp48.*",        
	MAME_DIR .. "src/mess/drivers/hp49gp.*",    
	MAME_DIR .. "src/mess/drivers/hp9845.*",    
	MAME_DIR .. "src/mess/drivers/hp9k.*",      
	MAME_DIR .. "src/mess/drivers/hp9k_3xx.*",  
}

createMESSProjects(_target, _subtarget, "hec2hrp")
files {           
	MAME_DIR .. "src/mess/drivers/hec2hrp.*",   
	MAME_DIR .. "src/mess/machine/hec2hrp.*",   
	MAME_DIR .. "src/mess/machine/hecdisk2.*",  
	MAME_DIR .. "src/mess/video/hec2video.*",   
	MAME_DIR .. "src/mess/drivers/interact.*",  
}

createMESSProjects(_target, _subtarget, "intel")
files {             
	MAME_DIR .. "src/mess/drivers/basic52.*",   
	MAME_DIR .. "src/mess/drivers/imds.*",      
	MAME_DIR .. "src/mess/drivers/ipc.*",       
	MAME_DIR .. "src/mess/drivers/ipds.*",      
	MAME_DIR .. "src/mess/drivers/isbc.*",
	MAME_DIR .. "src/mess/machine/isbc_215g.*", 
	MAME_DIR .. "src/mess/drivers/rex6000.*",   
	MAME_DIR .. "src/mess/drivers/sdk85.*",     
	MAME_DIR .. "src/mess/drivers/sdk86.*",   
	MAME_DIR .. "src/mess/drivers/imds2.*",  
}

createMESSProjects(_target, _subtarget, "imp")
files {               
	MAME_DIR .. "src/mess/drivers/tim011.*",    
	MAME_DIR .. "src/mess/drivers/tim100.*",    
}

createMESSProjects(_target, _subtarget, "interton")
files {          
	MAME_DIR .. "src/mess/drivers/vc4000.*",
	MAME_DIR .. "src/mess/audio/vc4000snd.*",
	MAME_DIR .. "src/mess/video/vc4000.*", 
}

createMESSProjects(_target, _subtarget, "intv")
files {              
	MAME_DIR .. "src/mess/drivers/intv.*",
	MAME_DIR .. "src/mess/machine/intv.*",
	MAME_DIR .. "src/mess/video/intv.*",
	MAME_DIR .. "src/mess/video/stic.*", 
}

createMESSProjects(_target, _subtarget, "isc")
files {               
	MAME_DIR .. "src/mess/drivers/compucolor.*",
}

createMESSProjects(_target, _subtarget, "kaypro")
files {            
	MAME_DIR .. "src/mess/drivers/kaypro.*",
	MAME_DIR .. "src/mess/machine/kaypro.*",
	MAME_DIR .. "src/mess/machine/kay_kbd.*",
	MAME_DIR .. "src/mess/video/kaypro.*", 
}

createMESSProjects(_target, _subtarget, "koei")
files {              
	MAME_DIR .. "src/mess/drivers/pasogo.*",    
}

createMESSProjects(_target, _subtarget, "kyocera")
files {           
	MAME_DIR .. "src/mess/drivers/kyocera.*",
	MAME_DIR .. "src/mess/video/kyocera.*", 
}

createMESSProjects(_target, _subtarget, "luxor")
files {             
	MAME_DIR .. "src/mess/drivers/abc80.*",
	MAME_DIR .. "src/mess/machine/abc80kb.*",
	MAME_DIR .. "src/mess/video/abc80.*", 
	MAME_DIR .. "src/mess/drivers/abc80x.*",
	MAME_DIR .. "src/mess/video/abc800.*",
	MAME_DIR .. "src/mess/video/abc802.*",
	MAME_DIR .. "src/mess/video/abc806.*", 
	MAME_DIR .. "src/mess/drivers/abc1600.*",
	MAME_DIR .. "src/mess/machine/abc1600mac.*",
	MAME_DIR .. "src/mess/video/abc1600.*", 
}

createMESSProjects(_target, _subtarget, "magnavox")
files {          
	MAME_DIR .. "src/mess/drivers/odyssey2.*",  
}

createMESSProjects(_target, _subtarget, "makerbot")
files {          
	MAME_DIR .. "src/mess/drivers/replicator.*",
}

createMESSProjects(_target, _subtarget, "marx")
files {              
	MAME_DIR .. "src/mess/drivers/elecbowl.*",  
}

createMESSProjects(_target, _subtarget, "mattel")
files {            
	MAME_DIR .. "src/mess/drivers/aquarius.*",
	MAME_DIR .. "src/mess/video/aquarius.*", 
	MAME_DIR .. "src/mess/drivers/juicebox.*",  
	MAME_DIR .. "src/mess/drivers/hyperscan.*", 
}

createMESSProjects(_target, _subtarget, "matsushi")
files {          
	MAME_DIR .. "src/mess/drivers/jr100.*",     
	MAME_DIR .. "src/mess/drivers/jr200.*",     
	MAME_DIR .. "src/mess/drivers/myb3k.*",     
}

createMESSProjects(_target, _subtarget, "mb")
files {                
	MAME_DIR .. "src/mess/drivers/mbdtower.*",
	MAME_DIR .. "src/mess/drivers/microvsn.*",  
}

createMESSProjects(_target, _subtarget, "mchester")
files {          
	MAME_DIR .. "src/mess/drivers/ssem.*",      
}

createMESSProjects(_target, _subtarget, "memotech")
files {          
	MAME_DIR .. "src/mess/drivers/mtx.*",
	MAME_DIR .. "src/mess/machine/mtx.*", 
}

createMESSProjects(_target, _subtarget, "mgu")
files {               
	MAME_DIR .. "src/mess/drivers/irisha.*",    
}

createMESSProjects(_target, _subtarget, "microkey")
files {          
	MAME_DIR .. "src/mess/drivers/primo.*",
	MAME_DIR .. "src/mess/machine/primo.*",
	MAME_DIR .. "src/mess/video/primo.*", 
}

createMESSProjects(_target, _subtarget, "mit")
files {               
	MAME_DIR .. "src/mess/drivers/tx0.*",
	MAME_DIR .. "src/mess/video/crt.*",
	MAME_DIR .. "src/mess/video/tx0.*", 
}

createMESSProjects(_target, _subtarget, "mits")
files {              
	MAME_DIR .. "src/mess/drivers/altair.*",    
	MAME_DIR .. "src/mess/drivers/mits680b.*",  
}

createMESSProjects(_target, _subtarget, "mitsubishi")
files {        
	MAME_DIR .. "src/mess/drivers/multi8.*",    
	MAME_DIR .. "src/mess/drivers/multi16.*",   
}

createMESSProjects(_target, _subtarget, "morrow")
files {            
	MAME_DIR .. "src/mess/drivers/microdec.*",  
	MAME_DIR .. "src/mess/drivers/mpz80.*",     
	MAME_DIR .. "src/mess/drivers/tricep.*",    
}

createMESSProjects(_target, _subtarget, "mos")
files {               
	MAME_DIR .. "src/mess/drivers/kim1.*",      
}

createMESSProjects(_target, _subtarget, "motorola")
files {          
	MAME_DIR .. "src/mess/drivers/m6805evs.*",  
	MAME_DIR .. "src/mess/drivers/mekd2.*",     
}

createMESSProjects(_target, _subtarget, "multitch")
files {          
	MAME_DIR .. "src/mess/drivers/mkit09.*",    
	MAME_DIR .. "src/mess/drivers/mpf1.*",      
}

createMESSProjects(_target, _subtarget, "nakajima")
files {          
	MAME_DIR .. "src/mess/drivers/nakajies.*",  
}

createMESSProjects(_target, _subtarget, "nascom")
files {            
	MAME_DIR .. "src/mess/drivers/nascom1.*",
	MAME_DIR .. "src/mess/machine/nascom1.*",
	MAME_DIR .. "src/mess/video/nascom1.*", 
}

createMESSProjects(_target, _subtarget, "ne")
files {                
	MAME_DIR .. "src/mess/drivers/z80ne.*",
	MAME_DIR .. "src/mess/machine/z80ne.*", 
}

createMESSProjects(_target, _subtarget, "nec")
files {               
	MAME_DIR .. "src/mess/drivers/apc.*",       
	MAME_DIR .. "src/mess/drivers/pce.*",
	MAME_DIR .. "src/mess/machine/pce.*",
	MAME_DIR .. "src/mess/machine/pce_cd.*", 
	MAME_DIR .. "src/mess/drivers/pcfx.*",      
	MAME_DIR .. "src/mess/drivers/pc6001.*",    
	MAME_DIR .. "src/mess/drivers/pc8401a.*",
	MAME_DIR .. "src/mess/video/pc8401a.*",
	MAME_DIR .. "src/mess/drivers/pc8001.*", 
	MAME_DIR .. "src/mess/drivers/pc8801.*",    
	MAME_DIR .. "src/mess/drivers/pc88va.*",    
	MAME_DIR .. "src/mess/drivers/pc100.*",     
	MAME_DIR .. "src/mess/drivers/pc9801.*",
	MAME_DIR .. "src/mess/machine/pc9801_26.*",
	MAME_DIR .. "src/mess/machine/pc9801_86.*",
	MAME_DIR .. "src/mess/machine/pc9801_118.*",
	MAME_DIR .. "src/mess/machine/pc9801_cbus.*",
	MAME_DIR .. "src/mess/machine/pc9801_kbd.*", 
	MAME_DIR .. "src/mess/drivers/tk80bs.*",    
	MAME_DIR .. "src/mess/drivers/hh_ucom4.*",  
}

createMESSProjects(_target, _subtarget, "netronic")
files {          
	MAME_DIR .. "src/mess/drivers/elf.*",       
	MAME_DIR .. "src/mess/drivers/exp85.*",     
}

createMESSProjects(_target, _subtarget, "next")
files {              
	MAME_DIR .. "src/mess/drivers/next.*",
	MAME_DIR .. "src/mess/machine/nextkbd.*",
	MAME_DIR .. "src/mess/machine/nextmo.*", 
}

createMESSProjects(_target, _subtarget, "nintendo")
files {          
	MAME_DIR .. "src/mess/drivers/gb.*",
	MAME_DIR .. "src/mess/audio/gb.*",
	MAME_DIR .. "src/mess/machine/gb.*",
	MAME_DIR .. "src/mess/video/gb_lcd.*", 
	MAME_DIR .. "src/mess/drivers/gba.*",
	MAME_DIR .. "src/mess/video/gba.*", 
	MAME_DIR .. "src/mess/drivers/n64.*",       
	MAME_DIR .. "src/mess/drivers/nes.*",
	MAME_DIR .. "src/mess/machine/nes.*",
	MAME_DIR .. "src/mess/video/nes.*", 
	MAME_DIR .. "src/mess/drivers/pokemini.*",  
	MAME_DIR .. "src/mess/drivers/snes.*",
	MAME_DIR .. "src/mess/machine/snescx4.*", 
	MAME_DIR .. "src/mess/drivers/vboy.*",
	MAME_DIR .. "src/mess/audio/vboy.*", 
}

createMESSProjects(_target, _subtarget, "nokia")
files {             
	MAME_DIR .. "src/mess/drivers/mikromik.*",
	MAME_DIR .. "src/mess/machine/mm1kb.*",
	MAME_DIR .. "src/mess/video/mikromik.*", 
}

createMESSProjects(_target, _subtarget, "northstar")
files {         
	MAME_DIR .. "src/mess/drivers/horizon.*",   
}

createMESSProjects(_target, _subtarget, "novag")
files {             
	MAME_DIR .. "src/mess/drivers/mk1.*",       
	MAME_DIR .. "src/mess/drivers/mk2.*",       
	MAME_DIR .. "src/mess/drivers/ssystem3.*",
	MAME_DIR .. "src/mess/video/ssystem3.*", 
	MAME_DIR .. "src/mess/drivers/supercon.*",  
}

createMESSProjects(_target, _subtarget, "olivetti")
files {          
	MAME_DIR .. "src/mess/drivers/m20.*",       
	MAME_DIR .. "src/mess/drivers/m24.*",
	MAME_DIR .. "src/mess/machine/m24_kbd.*",
}

createMESSProjects(_target, _subtarget, "omnibyte")
files {          
	MAME_DIR .. "src/mess/drivers/msbc1.*",     
	MAME_DIR .. "src/mess/drivers/ob68k1a.*",   
}

createMESSProjects(_target, _subtarget, "orion")
files {             
	MAME_DIR .. "src/mess/drivers/orion.*",
	MAME_DIR .. "src/mess/machine/orion.*",
	MAME_DIR .. "src/mess/video/orion.*", 
}

createMESSProjects(_target, _subtarget, "osborne")
files {           
	MAME_DIR .. "src/mess/drivers/osborne1.*",
	MAME_DIR .. "src/mess/machine/osborne1.*", 
	MAME_DIR .. "src/mess/drivers/osbexec.*",   
	MAME_DIR .. "src/mess/drivers/vixen.*",     
}

createMESSProjects(_target, _subtarget, "osi")
files {               
	MAME_DIR .. "src/mess/drivers/osi.*",
	MAME_DIR .. "src/mess/video/osi.*", 
}

createMESSProjects(_target, _subtarget, "palm")
files {              
	MAME_DIR .. "src/mess/drivers/palm.*",      
	MAME_DIR .. "src/mess/drivers/palmz22.*",   
}

createMESSProjects(_target, _subtarget, "parker")
files {            
	MAME_DIR .. "src/mess/drivers/wildfire.*",  
}

createMESSProjects(_target, _subtarget, "pitronic")
files {          
	MAME_DIR .. "src/mess/drivers/beta.*",      
}

createMESSProjects(_target, _subtarget, "pc")
files {                
	MAME_DIR .. "src/mess/drivers/asst128.*",   
	MAME_DIR .. "src/mess/drivers/europc.*",    
	MAME_DIR .. "src/mess/drivers/genpc.*",
	MAME_DIR .. "src/mess/machine/genpc.*", 
	MAME_DIR .. "src/mess/drivers/ibmpc.*",     
	MAME_DIR .. "src/mess/drivers/ibmpcjr.*",   
	MAME_DIR .. "src/mess/drivers/pc.*",        
	MAME_DIR .. "src/mess/drivers/tandy1t.*",
	MAME_DIR .. "src/mess/video/pc_t1t.*", 
}

createMESSProjects(_target, _subtarget, "pdp1")
files {              
	MAME_DIR .. "src/mess/drivers/pdp1.*",
	MAME_DIR .. "src/mess/video/pdp1.*", 
}

createMESSProjects(_target, _subtarget, "pel")
files {               
	MAME_DIR .. "src/mess/drivers/galeb.*",
	MAME_DIR .. "src/mess/video/galeb.*", 
		MAME_DIR .. "src/mess/drivers/orao.*",
	MAME_DIR .. "src/mess/machine/orao.*",
	MAME_DIR .. "src/mess/video/orao.*", 
}

createMESSProjects(_target, _subtarget, "philips")
files {           
	MAME_DIR .. "src/mess/drivers/p2000t.*",
	MAME_DIR .. "src/mess/machine/p2000t.*",
	MAME_DIR .. "src/mess/video/p2000m.*", 
	MAME_DIR .. "src/mess/drivers/vg5k.*",      
}

createMESSProjects(_target, _subtarget, "poly88")
files {            
	MAME_DIR .. "src/mess/drivers/poly88.*",
	MAME_DIR .. "src/mess/machine/poly88.*",
	MAME_DIR .. "src/mess/video/poly88.*", 
}

createMESSProjects(_target, _subtarget, "psion")
files {             
	MAME_DIR .. "src/mess/drivers/psion.*",
	MAME_DIR .. "src/mess/machine/psion_pack.*", 
}

createMESSProjects(_target, _subtarget, "radio")
files {             
	MAME_DIR .. "src/mess/drivers/apogee.*",    
	MAME_DIR .. "src/mess/drivers/mikrosha.*",  
	MAME_DIR .. "src/mess/drivers/partner.*",
	MAME_DIR .. "src/mess/machine/partner.*", 
	MAME_DIR .. "src/mess/drivers/radio86.*",
	MAME_DIR .. "src/mess/machine/radio86.*", 
}

createMESSProjects(_target, _subtarget, "rca")
files {               
	MAME_DIR .. "src/mess/drivers/microkit.*",  
	MAME_DIR .. "src/mess/drivers/studio2.*",   
	MAME_DIR .. "src/mess/drivers/vip.*",       
}

createMESSProjects(_target, _subtarget, "rm")
files {                
	MAME_DIR .. "src/mess/drivers/rm380z.*",
	MAME_DIR .. "src/mess/machine/rm380z.*",
	MAME_DIR .. "src/mess/video/rm380z.*", 
	MAME_DIR .. "src/mess/drivers/rmnimbus.*",
	MAME_DIR .. "src/mess/machine/rmnimbus.*",
	MAME_DIR .. "src/mess/video/rmnimbus.*",
	MAME_DIR .. "src/mess/machine/rmnkbd.*", 
}

createMESSProjects(_target, _subtarget, "robotron")
files {          
	MAME_DIR .. "src/mess/drivers/a5105.*",     
	MAME_DIR .. "src/mess/drivers/a51xx.*",     
	MAME_DIR .. "src/mess/drivers/a7150.*",     
	MAME_DIR .. "src/mess/drivers/k1003.*",     
	MAME_DIR .. "src/mess/drivers/k8915.*",     
	MAME_DIR .. "src/mess/drivers/rt1715.*",    
	MAME_DIR .. "src/mess/drivers/z1013.*",     
	MAME_DIR .. "src/mess/drivers/z9001.*",     
}

createMESSProjects(_target, _subtarget, "roland")
files {            
	MAME_DIR .. "src/mess/drivers/rmt32.*",     
	MAME_DIR .. "src/mess/drivers/rd110.*",     
	MAME_DIR .. "src/mess/drivers/rsc55.*",     
	MAME_DIR .. "src/mess/drivers/tb303.*",     
}

createMESSProjects(_target, _subtarget, "rockwell")
files {          
	MAME_DIR .. "src/mess/drivers/aim65.*",
	MAME_DIR .. "src/mess/machine/aim65.*", 
	MAME_DIR .. "src/mess/drivers/aim65_40.*",  
}

createMESSProjects(_target, _subtarget, "sage")
files {              
	MAME_DIR .. "src/mess/drivers/sage2.*",     
}

createMESSProjects(_target, _subtarget, "samcoupe")
files {          
	MAME_DIR .. "src/mess/drivers/samcoupe.*",
	MAME_DIR .. "src/mess/machine/samcoupe.*",
	MAME_DIR .. "src/mess/video/samcoupe.*", 
}

createMESSProjects(_target, _subtarget, "samsung")
files {           
	MAME_DIR .. "src/mess/drivers/spc1000.*",   
}

createMESSProjects(_target, _subtarget, "sanyo")
files {             
	MAME_DIR .. "src/mess/drivers/mbc200.*",    
	MAME_DIR .. "src/mess/drivers/mbc55x.*",    
	MAME_DIR .. "src/mess/machine/mbc55x.*",    
	MAME_DIR .. "src/mess/video/mbc55x.*",      
	MAME_DIR .. "src/mess/drivers/phc25.*",     
}

createMESSProjects(_target, _subtarget, "sega")
files {              
	MAME_DIR .. "src/mess/drivers/dccons.*",
	MAME_DIR .. "src/mess/machine/dccons.*", 
	MAME_DIR .. "src/mess/drivers/megadriv.*",  
	MAME_DIR .. "src/mess/drivers/saturn.*",    
	MAME_DIR .. "src/mess/drivers/segapico.*",  
	MAME_DIR .. "src/mess/drivers/sg1000.*",    
	MAME_DIR .. "src/mess/drivers/sms.*",
	MAME_DIR .. "src/mess/machine/sms.*", 
	MAME_DIR .. "src/mess/drivers/svmu.*",      
	MAME_DIR .. "src/mess/machine/mega32x.*",   
	MAME_DIR .. "src/mess/machine/megacd.*",    
	MAME_DIR .. "src/mess/machine/megacdcd.*",  
}

createMESSProjects(_target, _subtarget, "sgi")
files {               
	MAME_DIR .. "src/mess/machine/sgi.*",       
	MAME_DIR .. "src/mess/drivers/sgi_ip2.*",   
	MAME_DIR .. "src/mess/drivers/sgi_ip6.*",   
	MAME_DIR .. "src/mess/drivers/ip20.*",      
	MAME_DIR .. "src/mess/drivers/ip22.*",      
	MAME_DIR .. "src/mess/video/newport.*",     
}

createMESSProjects(_target, _subtarget, "sharp")
files {             
	MAME_DIR .. "src/mess/video/mz700.*",       
	MAME_DIR .. "src/mess/drivers/mz700.*",     
	MAME_DIR .. "src/mess/drivers/pc1500.*",    
	MAME_DIR .. "src/mess/drivers/pocketc.*",   
	MAME_DIR .. "src/mess/video/pc1401.*",      
	MAME_DIR .. "src/mess/machine/pc1401.*",    
	MAME_DIR .. "src/mess/video/pc1403.*",      
	MAME_DIR .. "src/mess/machine/pc1403.*",    
	MAME_DIR .. "src/mess/video/pc1350.*",      
	MAME_DIR .. "src/mess/machine/pc1350.*",    
	MAME_DIR .. "src/mess/video/pc1251.*",      
	MAME_DIR .. "src/mess/machine/pc1251.*",    
	MAME_DIR .. "src/mess/video/pocketc.*",     
	MAME_DIR .. "src/mess/machine/mz700.*",     
	MAME_DIR .. "src/mess/drivers/x68k.*",      
	MAME_DIR .. "src/mess/video/x68k.*",        
	MAME_DIR .. "src/mess/machine/x68k_hdc.*",  
	MAME_DIR .. "src/mess/machine/x68k_kbd.*",  
	MAME_DIR .. "src/mess/drivers/mz80.*",      
	MAME_DIR .. "src/mess/video/mz80.*",        
	MAME_DIR .. "src/mess/machine/mz80.*",      
	MAME_DIR .. "src/mess/drivers/mz2000.*",    
	MAME_DIR .. "src/mess/drivers/x1.*",        
	MAME_DIR .. "src/mess/machine/x1.*",        
	MAME_DIR .. "src/mess/drivers/x1twin.*",    
	MAME_DIR .. "src/mess/drivers/mz2500.*",    
	MAME_DIR .. "src/mess/drivers/mz3500.*",    
	MAME_DIR .. "src/mess/drivers/pce220.*",    
	MAME_DIR .. "src/mess/machine/pce220_ser.*",
	MAME_DIR .. "src/mess/drivers/mz6500.*",    
	MAME_DIR .. "src/mess/drivers/zaurus.*",    
}

createMESSProjects(_target, _subtarget, "sinclair")
files {          
	MAME_DIR .. "src/mess/video/spectrum.*",    
	MAME_DIR .. "src/mess/video/timex.*",       
	MAME_DIR .. "src/mess/video/zx.*",          
	MAME_DIR .. "src/mess/drivers/zx.*",        
	MAME_DIR .. "src/mess/machine/zx.*",        
	MAME_DIR .. "src/mess/drivers/spectrum.*",  
	MAME_DIR .. "src/mess/drivers/spec128.*",   
	MAME_DIR .. "src/mess/drivers/timex.*",     
	MAME_DIR .. "src/mess/drivers/specpls3.*",  
	MAME_DIR .. "src/mess/drivers/scorpion.*",  
	MAME_DIR .. "src/mess/drivers/atm.*",       
	MAME_DIR .. "src/mess/drivers/pentagon.*",  
	MAME_DIR .. "src/mess/machine/beta.*",      
	MAME_DIR .. "src/mess/machine/spec_snqk.*", 
	MAME_DIR .. "src/mess/drivers/ql.*",        
	MAME_DIR .. "src/mess/machine/qimi.*",      
	MAME_DIR .. "src/mess/video/zx8301.*",      
	MAME_DIR .. "src/mess/machine/zx8302.*",    
}

createMESSProjects(_target, _subtarget, "siemens")
files {           
	MAME_DIR .. "src/mess/drivers/pcd.*",       
	MAME_DIR .. "src/mess/machine/pcd_kbd.*",   
}

createMESSProjects(_target, _subtarget, "slicer")
files {            
	MAME_DIR .. "src/mess/drivers/slicer.*",    
}

createMESSProjects(_target, _subtarget, "snk")
files {               
	MAME_DIR .. "src/mess/drivers/ng_aes.*",    
	MAME_DIR .. "src/mess/drivers/ngp.*",
	MAME_DIR .. "src/mess/video/k1ge.*", 
}

createMESSProjects(_target, _subtarget, "sony")
files {              
	MAME_DIR .. "src/mess/drivers/pockstat.*",
	MAME_DIR .. "src/mess/drivers/psx.*",
	MAME_DIR .. "src/mess/machine/psxcd.*", 
	MAME_DIR .. "src/mess/drivers/pve500.*",    
	MAME_DIR .. "src/mess/drivers/smc777.*",    
}

createMESSProjects(_target, _subtarget, "sord")
files {              
	MAME_DIR .. "src/mess/drivers/m5.*",        
}

createMESSProjects(_target, _subtarget, "special")
files {           
	MAME_DIR .. "src/mess/drivers/special.*",
	MAME_DIR .. "src/mess/audio/specimx_snd.*",
	MAME_DIR .. "src/mess/machine/special.*",
	MAME_DIR .. "src/mess/video/special.*", 
}

createMESSProjects(_target, _subtarget, "sun")
files {               
	MAME_DIR .. "src/mess/drivers/sun1.*",      
	MAME_DIR .. "src/mess/drivers/sun2.*",      
	MAME_DIR .. "src/mess/drivers/sun3.*",      
	MAME_DIR .. "src/mess/drivers/sun4.*",      
}

createMESSProjects(_target, _subtarget, "svi")
files {               
	MAME_DIR .. "src/mess/drivers/svi318.*",
	MAME_DIR .. "src/mess/machine/svi318.*", 
}

createMESSProjects(_target, _subtarget, "svision")
files {           
	MAME_DIR .. "src/mess/drivers/svision.*",
	MAME_DIR .. "src/mess/audio/svis_snd.*", 
}

createMESSProjects(_target, _subtarget, "swtpc09")
files {           
	MAME_DIR .. "src/mess/drivers/swtpc09.*",
	MAME_DIR .. "src/mess/machine/swtpc09.*", 
}

createMESSProjects(_target, _subtarget, "synertec")
files {          
	MAME_DIR .. "src/mess/drivers/sym1.*",      
}

createMESSProjects(_target, _subtarget, "ta")
files {                
	MAME_DIR .. "src/mess/drivers/alphatro.*",  
}

createMESSProjects(_target, _subtarget, "tandberg")
files {          
	MAME_DIR .. "src/mess/drivers/tdv2324.*",   
}

createMESSProjects(_target, _subtarget, "tangerin")
files {          
	MAME_DIR .. "src/mess/drivers/microtan.*",
	MAME_DIR .. "src/mess/machine/microtan.*",
	MAME_DIR .. "src/mess/video/microtan.*", 
	MAME_DIR .. "src/mess/drivers/oric.*",
}

createMESSProjects(_target, _subtarget, "tatung")
files {            
	MAME_DIR .. "src/mess/drivers/einstein.*",
	MAME_DIR .. "src/mess/machine/einstein.*", 
}

createMESSProjects(_target, _subtarget, "teamconc")
files {          
	MAME_DIR .. "src/mess/drivers/comquest.*",
	MAME_DIR .. "src/mess/video/comquest.*", 
}

createMESSProjects(_target, _subtarget, "tektroni")
files {          
	MAME_DIR .. "src/mess/drivers/tek405x.*",   
	MAME_DIR .. "src/mess/drivers/tek410x.*",   
}

createMESSProjects(_target, _subtarget, "telenova")
files {          
	MAME_DIR .. "src/mess/drivers/compis.*",
	MAME_DIR .. "src/mess/machine/compiskb.*", 
}

createMESSProjects(_target, _subtarget, "telercas")
files {          
	MAME_DIR .. "src/mess/drivers/tmc1800.*",
	MAME_DIR .. "src/mess/video/tmc1800.*", 
	MAME_DIR .. "src/mess/drivers/tmc600.*",
	MAME_DIR .. "src/mess/video/tmc600.*", 
	MAME_DIR .. "src/mess/drivers/tmc2000e.*",  
}

createMESSProjects(_target, _subtarget, "televideo")
files {         
	MAME_DIR .. "src/mess/drivers/ts802.*",     
	MAME_DIR .. "src/mess/drivers/ts803.*",     
	MAME_DIR .. "src/mess/drivers/ts816.*",     
	MAME_DIR .. "src/mess/drivers/tv950.*",     
}

createMESSProjects(_target, _subtarget, "tem")
files {               
	MAME_DIR .. "src/mess/drivers/tec1.*",      
}

createMESSProjects(_target, _subtarget, "tesla")
files {             
	MAME_DIR .. "src/mess/drivers/ondra.*",
	MAME_DIR .. "src/mess/machine/ondra.*",
	MAME_DIR .. "src/mess/video/ondra.*", 
	MAME_DIR .. "src/mess/drivers/pmd85.*",
	MAME_DIR .. "src/mess/machine/pmd85.*",
	MAME_DIR .. "src/mess/video/pmd85.*", 
	MAME_DIR .. "src/mess/drivers/pmi80.*",     
	MAME_DIR .. "src/mess/drivers/sapi1.*",     
}

createMESSProjects(_target, _subtarget, "test")
files {              
	MAME_DIR .. "src/mess/drivers/test_t400.*", 
	MAME_DIR .. "src/mess/drivers/zexall.*",    
}

createMESSProjects(_target, _subtarget, "thomson")
files {           
	MAME_DIR .. "src/mess/drivers/thomson.*",
	MAME_DIR .. "src/mess/machine/thomson.*",
	MAME_DIR .. "src/mess/machine/thomflop.*",
	MAME_DIR .. "src/mess/video/thomson.*", 
}

createMESSProjects(_target, _subtarget, "ti")
files {                
	MAME_DIR .. "src/mess/drivers/avigo.*",
	MAME_DIR .. "src/mess/video/avigo.*", 
	MAME_DIR .. "src/mess/drivers/cc40.*",      
	MAME_DIR .. "src/mess/drivers/evmbug.*",    
	MAME_DIR .. "src/mess/drivers/exelv.*",     
	MAME_DIR .. "src/mess/drivers/geneve.*",    
	MAME_DIR .. "src/mess/drivers/ticalc1x.*",  
	MAME_DIR .. "src/mess/drivers/tispeak.*",   
	MAME_DIR .. "src/mess/drivers/ti74.*",      
	MAME_DIR .. "src/mess/drivers/ti85.*",
	MAME_DIR .. "src/mess/machine/ti85.*",
	MAME_DIR .. "src/mess/video/ti85.*", 
	MAME_DIR .. "src/mess/drivers/ti89.*",      
	MAME_DIR .. "src/mess/drivers/ti99_2.*",    
	MAME_DIR .. "src/mess/drivers/ti99_4x.*",   
	MAME_DIR .. "src/mess/drivers/ti99_4p.*",   
	MAME_DIR .. "src/mess/drivers/ti99_8.*",    
	MAME_DIR .. "src/mess/drivers/ti990_4.*",   
	MAME_DIR .. "src/mess/drivers/ti990_10.*",  
	MAME_DIR .. "src/mess/drivers/tm990189.*",  
	MAME_DIR .. "src/mess/machine/ti99/990_dk.*", 
	MAME_DIR .. "src/mess/machine/ti99/990_hd.*", 
	MAME_DIR .. "src/mess/machine/ti99/990_tap.*", 
	MAME_DIR .. "src/mess/machine/ti99/datamux.*", 
	MAME_DIR .. "src/mess/machine/ti99/genboard.*", 
	MAME_DIR .. "src/mess/machine/ti99/grom.*", 
	MAME_DIR .. "src/mess/machine/ti99/gromport.*", 
	MAME_DIR .. "src/mess/machine/ti99/handset.*", 
	MAME_DIR .. "src/mess/machine/ti99/joyport.*", 
	MAME_DIR .. "src/mess/machine/ti99/mapper8.*", 
	MAME_DIR .. "src/mess/machine/ti99/mecmouse.*", 
	MAME_DIR .. "src/mess/machine/ti99/speech8.*", 
	MAME_DIR .. "src/mess/machine/ti99/videowrp.*", 
	MAME_DIR .. "src/mess/video/733_asr.*",     
	MAME_DIR .. "src/mess/video/911_vdt.*",     
	MAME_DIR .. "src/mess/drivers/hh_tms1k.*",  
}

createMESSProjects(_target, _subtarget, "tiger")
files {             
	MAME_DIR .. "src/mess/drivers/gamecom.*",
	MAME_DIR .. "src/mess/machine/gamecom.*",
	MAME_DIR .. "src/mess/video/gamecom.*", 
}

createMESSProjects(_target, _subtarget, "tigertel")
files {          
	MAME_DIR .. "src/mess/drivers/gizmondo.*",
	MAME_DIR .. "src/mess/machine/docg3.*", 
}

createMESSProjects(_target, _subtarget, "tiki")
files {              
	MAME_DIR .. "src/mess/drivers/tiki100.*",   
}

createMESSProjects(_target, _subtarget, "tomy")
files {              
	MAME_DIR .. "src/mess/drivers/tutor.*",     
}

createMESSProjects(_target, _subtarget, "toshiba")
files {           
	MAME_DIR .. "src/mess/drivers/pasopia.*",   
	MAME_DIR .. "src/mess/drivers/pasopia7.*",  
	MAME_DIR .. "src/mess/drivers/paso1600.*",  
}

createMESSProjects(_target, _subtarget, "trainer")
files {           
	MAME_DIR .. "src/mess/drivers/amico2k.*",   
	MAME_DIR .. "src/mess/drivers/babbage.*",   
	MAME_DIR .. "src/mess/drivers/bob85.*",     
	MAME_DIR .. "src/mess/drivers/cvicny.*",    
	MAME_DIR .. "src/mess/drivers/dolphunk.*",  
	MAME_DIR .. "src/mess/drivers/instruct.*",  
	MAME_DIR .. "src/mess/drivers/mk14.*",      
	MAME_DIR .. "src/mess/drivers/pro80.*",     
	MAME_DIR .. "src/mess/drivers/savia84.*",   
	MAME_DIR .. "src/mess/drivers/selz80.*",    
	MAME_DIR .. "src/mess/drivers/tk80.*",      
}

createMESSProjects(_target, _subtarget, "trs")
files {               
	MAME_DIR .. "src/mess/drivers/coco12.*",    
	MAME_DIR .. "src/mess/drivers/coco3.*",     
	MAME_DIR .. "src/mess/drivers/dragon.*",    
	MAME_DIR .. "src/mess/drivers/mc10.*",      
	MAME_DIR .. "src/mess/machine/6883sam.*",   
	MAME_DIR .. "src/mess/machine/coco.*",      
	MAME_DIR .. "src/mess/machine/coco12.*",    
	MAME_DIR .. "src/mess/machine/coco3.*",     
	MAME_DIR .. "src/mess/machine/coco_vhd.*",  
	MAME_DIR .. "src/mess/machine/dragon.*",    
	MAME_DIR .. "src/mess/machine/dgnalpha.*",  
	MAME_DIR .. "src/mess/video/gime.*",        
	MAME_DIR .. "src/mess/drivers/trs80.*",
	MAME_DIR .. "src/mess/machine/trs80.*",
	MAME_DIR .. "src/mess/video/trs80.*", 
	MAME_DIR .. "src/mess/drivers/trs80m2.*",
	MAME_DIR .. "src/mess/machine/trs80m2kb.*", 
	MAME_DIR .. "src/mess/drivers/tandy2k.*",
	MAME_DIR .. "src/mess/machine/tandy2kb.*", 
}

createMESSProjects(_target, _subtarget, "ultratec")
files {          
	MAME_DIR .. "src/mess/drivers/minicom.*",   
}

createMESSProjects(_target, _subtarget, "unisys")
files {            
	MAME_DIR .. "src/mess/drivers/univac.*",    
}

createMESSProjects(_target, _subtarget, "veb")
files {               
	MAME_DIR .. "src/mess/drivers/chessmst.*",  
	MAME_DIR .. "src/mess/drivers/kc.*",
	MAME_DIR .. "src/mess/machine/kc.*",
	MAME_DIR .. "src/mess/machine/kc_keyb.*",
	MAME_DIR .. "src/mess/video/kc.*", 
	MAME_DIR .. "src/mess/drivers/lc80.*",      
	MAME_DIR .. "src/mess/drivers/mc80.*",
	MAME_DIR .. "src/mess/machine/mc80.*",
	MAME_DIR .. "src/mess/video/mc80.*", 
	MAME_DIR .. "src/mess/drivers/poly880.*",   
	MAME_DIR .. "src/mess/drivers/sc1.*",       
	MAME_DIR .. "src/mess/drivers/sc2.*",       
}

createMESSProjects(_target, _subtarget, "vidbrain")
files {          
	MAME_DIR .. "src/mess/drivers/vidbrain.*",
	MAME_DIR .. "src/mess/video/uv201.*", 
}

createMESSProjects(_target, _subtarget, "videoton")
files {          
	MAME_DIR .. "src/mess/drivers/tvc.*",
	MAME_DIR .. "src/mess/audio/tvc_snd.*", 
}

createMESSProjects(_target, _subtarget, "visual")
files {            
	MAME_DIR .. "src/mess/drivers/v1050.*",
	MAME_DIR .. "src/mess/machine/v1050kb.*",
	MAME_DIR .. "src/mess/video/v1050.*", 
}

createMESSProjects(_target, _subtarget, "votrax")
files {            
	MAME_DIR .. "src/mess/drivers/votrpss.*",   
	MAME_DIR .. "src/mess/drivers/votrtnt.*",   
}

createMESSProjects(_target, _subtarget, "vtech")
files {             
	MAME_DIR .. "src/mess/drivers/crvision.*",  
	MAME_DIR .. "src/mess/drivers/geniusiq.*",  
	MAME_DIR .. "src/mess/drivers/laser3k.*",   
	MAME_DIR .. "src/mess/drivers/lcmate2.*",   
	MAME_DIR .. "src/mess/drivers/pc4.*",
	MAME_DIR .. "src/mess/video/pc4.*", 
	MAME_DIR .. "src/mess/drivers/pc2000.*",    
	MAME_DIR .. "src/mess/drivers/pitagjr.*",   
	MAME_DIR .. "src/mess/drivers/prestige.*",  
	MAME_DIR .. "src/mess/drivers/vtech1.*",    
	MAME_DIR .. "src/mess/drivers/vtech2.*",
	MAME_DIR .. "src/mess/machine/vtech2.*",
	MAME_DIR .. "src/mess/video/vtech2.*", 
	MAME_DIR .. "src/mess/drivers/socrates.*",
	MAME_DIR .. "src/mess/audio/socrates.*", 
}

createMESSProjects(_target, _subtarget, "wang")
files {              
	MAME_DIR .. "src/mess/drivers/wangpc.*",
	MAME_DIR .. "src/mess/machine/wangpckb.*", 
}

createMESSProjects(_target, _subtarget, "wavemate")
files {          
	MAME_DIR .. "src/mess/drivers/bullet.*",    
	MAME_DIR .. "src/mess/drivers/jupiter.*",   
}

createMESSProjects(_target, _subtarget, "xerox")
files {             
	MAME_DIR .. "src/mess/drivers/xerox820.*",
	MAME_DIR .. "src/mess/machine/x820kb.*", 
	MAME_DIR .. "src/mess/drivers/bigbord2.*",  
	MAME_DIR .. "src/mess/drivers/alto2.*",     
}

createMESSProjects(_target, _subtarget, "xussrpc")
files {           
	MAME_DIR .. "src/mess/drivers/ec184x.*",    
	MAME_DIR .. "src/mess/drivers/iskr103x.*",  
	MAME_DIR .. "src/mess/drivers/mc1502.*",    
	MAME_DIR .. "src/mess/drivers/poisk1.*",
	MAME_DIR .. "src/mess/video/poisk1.*", 
}

createMESSProjects(_target, _subtarget, "yamaha")
files {            
	MAME_DIR .. "src/mess/drivers/ymmu100.*",   
	MAME_DIR .. "src/mess/drivers/fb01.*",      
}

createMESSProjects(_target, _subtarget, "zenith")
files {            
	MAME_DIR .. "src/mess/drivers/z100.*",      
}

createMESSProjects(_target, _subtarget, "zpa")
files {               
	MAME_DIR .. "src/mess/drivers/iq151.*",     
}

createMESSProjects(_target, _subtarget, "zvt")
files {               
	MAME_DIR .. "src/mess/drivers/pp01.*",
	MAME_DIR .. "src/mess/machine/pp01.*",
	MAME_DIR .. "src/mess/video/pp01.*", 
}

createMESSProjects(_target, _subtarget, "skeleton")
files {          
	MAME_DIR .. "src/mess/drivers/alphasma.*",  
	MAME_DIR .. "src/mess/drivers/ampro.*",     
	MAME_DIR .. "src/mess/drivers/amust.*",     
	MAME_DIR .. "src/mess/drivers/applix.*",    
	MAME_DIR .. "src/mess/drivers/attache.*",   
	MAME_DIR .. "src/mess/drivers/ax20.*",      
	MAME_DIR .. "src/mess/drivers/beehive.*",   
	MAME_DIR .. "src/mess/drivers/binbug.*",    
	MAME_DIR .. "src/mess/drivers/besta.*",     
	MAME_DIR .. "src/mess/drivers/bitgraph.*",  
	MAME_DIR .. "src/mess/drivers/br8641.*",    
	MAME_DIR .. "src/mess/drivers/busicom.*",
	MAME_DIR .. "src/mess/video/busicom.*", 
	MAME_DIR .. "src/mess/drivers/chaos.*",     
	MAME_DIR .. "src/mess/drivers/chesstrv.*",  
	MAME_DIR .. "src/mess/drivers/cd2650.*",    
	MAME_DIR .. "src/mess/drivers/cdc721.*",    
	MAME_DIR .. "src/mess/drivers/codata.*",    
	MAME_DIR .. "src/mess/drivers/cortex.*",    
	MAME_DIR .. "src/mess/drivers/cosmicos.*",  
	MAME_DIR .. "src/mess/drivers/cp1.*",       
	MAME_DIR .. "src/mess/drivers/cxhumax.*",   
	MAME_DIR .. "src/mess/drivers/czk80.*",     
	MAME_DIR .. "src/mess/drivers/d6800.*",     
	MAME_DIR .. "src/mess/drivers/d6809.*",     
	MAME_DIR .. "src/mess/drivers/digel804.*",  
	MAME_DIR .. "src/mess/drivers/dim68k.*",    
	MAME_DIR .. "src/mess/drivers/dm7000.*",    
	MAME_DIR .. "src/mess/drivers/dmv.*",
	MAME_DIR .. "src/mess/machine/dmv_keyb.*", 
	MAME_DIR .. "src/mess/drivers/dps1.*",      
	MAME_DIR .. "src/mess/drivers/dsb46.*",     
	MAME_DIR .. "src/mess/drivers/dual68.*",    
	MAME_DIR .. "src/mess/drivers/eacc.*",      
	MAME_DIR .. "src/mess/drivers/elwro800.*",  
	MAME_DIR .. "src/mess/drivers/eti660.*",    
	MAME_DIR .. "src/mess/drivers/excali64.*",  
	MAME_DIR .. "src/mess/drivers/fanucs15.*",  
	MAME_DIR .. "src/mess/drivers/fanucspmg.*", 
	MAME_DIR .. "src/mess/drivers/fc100.*",     
	MAME_DIR .. "src/mess/drivers/fk1.*",       
	MAME_DIR .. "src/mess/drivers/ft68m.*",     
	MAME_DIR .. "src/mess/drivers/gamate.*",
	MAME_DIR .. "src/mess/audio/gamate.*", 
	MAME_DIR .. "src/mess/drivers/gameking.*",  
	MAME_DIR .. "src/mess/drivers/gimix.*",     
	MAME_DIR .. "src/mess/drivers/grfd2301.*",  
	MAME_DIR .. "src/mess/drivers/harriet.*",   
	MAME_DIR .. "src/mess/drivers/hprot1.*",    
	MAME_DIR .. "src/mess/drivers/hpz80unk.*",  
	MAME_DIR .. "src/mess/drivers/ht68k.*",     
	MAME_DIR .. "src/mess/drivers/hunter2.*", 
	MAME_DIR .. "src/emu/machine/nsc810.*", 
	MAME_DIR .. "src/mess/drivers/ibm6580.*",   
	MAME_DIR .. "src/mess/drivers/ie15.*",
	MAME_DIR .. "src/mess/machine/ie15_kbd.*", 
	MAME_DIR .. "src/mess/drivers/if800.*",     
	MAME_DIR .. "src/mess/drivers/imsai.*",     
	MAME_DIR .. "src/mess/drivers/indiana.*",   
	MAME_DIR .. "src/mess/drivers/itt3030.*",   
	MAME_DIR .. "src/mess/drivers/jade.*",      
	MAME_DIR .. "src/mess/drivers/jonos.*",     
	MAME_DIR .. "src/mess/drivers/konin.*",     
	MAME_DIR .. "src/mess/drivers/leapster.*",  
	MAME_DIR .. "src/mess/drivers/lft.*",       
	MAME_DIR .. "src/mess/drivers/lola8a.*",    
	MAME_DIR .. "src/mess/drivers/m79152pc.*",  
	MAME_DIR .. "src/mess/drivers/mccpm.*",     
	MAME_DIR .. "src/mess/drivers/mes.*",       
	MAME_DIR .. "src/mess/drivers/mice.*",      
	MAME_DIR .. "src/mess/drivers/micronic.*",  
	MAME_DIR .. "src/mess/drivers/mini2440.*",  
	MAME_DIR .. "src/mess/drivers/mmd1.*",      
	MAME_DIR .. "src/mess/drivers/mod8.*",      
	MAME_DIR .. "src/mess/drivers/modellot.*",  
	MAME_DIR .. "src/mess/drivers/molecular.*", 
	MAME_DIR .. "src/mess/drivers/ms0515.*",    
	MAME_DIR .. "src/mess/drivers/ms9540.*",    
	MAME_DIR .. "src/mess/drivers/mstation.*",  
	MAME_DIR .. "src/mess/drivers/mx2178.*",    
	MAME_DIR .. "src/mess/drivers/mycom.*",     
	MAME_DIR .. "src/mess/drivers/myvision.*",  
	MAME_DIR .. "src/mess/drivers/ngen.*",
	MAME_DIR .. "src/mess/machine/ngen_kb.*", 
	MAME_DIR .. "src/mess/drivers/octopus.*",   
	MAME_DIR .. "src/mess/drivers/onyx.*",      
	MAME_DIR .. "src/mess/drivers/okean240.*",  
	MAME_DIR .. "src/mess/drivers/p8k.*",       
	MAME_DIR .. "src/mess/drivers/pegasus.*",   
	MAME_DIR .. "src/mess/drivers/pencil2.*",   
	MAME_DIR .. "src/mess/drivers/pes.*",       
	MAME_DIR .. "src/mess/drivers/pipbug.*",    
	MAME_DIR .. "src/mess/drivers/plan80.*",    
	MAME_DIR .. "src/mess/drivers/pm68k.*",     
	MAME_DIR .. "src/mess/drivers/poly.*",      
	MAME_DIR .. "src/mess/drivers/pt68k4.*",    
	MAME_DIR .. "src/mess/drivers/ptcsol.*",    
	MAME_DIR .. "src/mess/drivers/pulsar.*",    
	MAME_DIR .. "src/mess/drivers/pv9234.*",    
	MAME_DIR .. "src/mess/drivers/qtsbc.*",     
	MAME_DIR .. "src/mess/drivers/rvoice.*",    
	MAME_DIR .. "src/mess/drivers/sacstate.*",  
	MAME_DIR .. "src/mess/drivers/sbrain.*",    
	MAME_DIR .. "src/mess/drivers/seattle.*",   
	MAME_DIR .. "src/mess/drivers/sh4robot.*",  
	MAME_DIR .. "src/mess/drivers/softbox.*",   
	MAME_DIR .. "src/mess/drivers/swtpc.*",     
	MAME_DIR .. "src/mess/drivers/sys2900.*",   
	MAME_DIR .. "src/mess/drivers/systec.*",    
	MAME_DIR .. "src/mess/drivers/tavernie.*",  
	MAME_DIR .. "src/mess/drivers/tecnbras.*",  
	MAME_DIR .. "src/mess/drivers/terak.*",     
	MAME_DIR .. "src/mess/drivers/ti630.*",     
	MAME_DIR .. "src/mess/drivers/tsispch.*",   
	MAME_DIR .. "src/mess/drivers/unistar.*",   
	MAME_DIR .. "src/mess/drivers/v6809.*",     
	MAME_DIR .. "src/mess/drivers/vector4.*",   
	MAME_DIR .. "src/mess/drivers/vii.*",       
	MAME_DIR .. "src/mess/drivers/wicat.*",     
	MAME_DIR .. "src/mess/drivers/xor100.*",    
	MAME_DIR .. "src/mess/drivers/xavix.*",     
	MAME_DIR .. "src/mess/drivers/zorba.*",     
}

end
