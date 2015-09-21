-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   mess.lua
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
--CPUS["MN10200"] = true
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
CPUS["E0C6200"] = true
CPUS["MELPS4"] = true
CPUS["HPHYBRID"] = true
CPUS["SM510"] = true

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
SOUNDS["YMF278B"] = true
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
VIDEOS["I82730"] = true
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
MACHINES["AT29X"] = true
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
MACHINES["LPCI"] = true
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
MACHINES["NETLIST"] = true
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
MACHINES["PIT68230"] = true
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
MACHINES["WD2010"] = true
MACHINES["WD33C93"] = true
MACHINES["WD7600"] = true
MACHINES["X2212"] = true
MACHINES["X76F041"] = true
MACHINES["X76F100"] = true
MACHINES["YM2148"] = true
MACHINES["Z80CTC"] = true
MACHINES["Z80DART"] = true
MACHINES["Z80SIO"] = true
MACHINES["Z80SCC"] = true
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
MACHINES["TMS1024"] = true
MACHINES["NSC810"] = true

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
BUSES["APRICOT_EXPANSION"] = true
BUSES["ARCADIA"] = true
BUSES["ASTROCADE"] = true
BUSES["BML3"] = true
BUSES["BW2"] = true
BUSES["C64"] = true
BUSES["CBM2"] = true
BUSES["CBMIEC"] = true
BUSES["CENTRONICS"] = true
BUSES["CGENIE_EXPANSION"] = true
BUSES["CGENIE_PARALLEL"] = true
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
BUSES["NASBUS"] = true
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
BUSES["TI99X"] = true
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
function linkProjects_mame_mess(_target, _subtarget)
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
		"force",
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
		"heurikon",
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
		"microsoft",
		"mit",
		"mits",
		"mitsubishi",
		"mizar",
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
		"ns",
		"olivetti",
		"olympia",
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
		"regnecentralen",
		"ritam",
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
		"ultimachine",
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
		"messshared",
	}
	if (_subtarget=="mess") then
	links {
		"mameshared",
	}
	end
end

function createMESSProjects(_target, _subtarget, _name)
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
		MAME_DIR .. "src/mess",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/lib/netlist",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mess/layout",
		GEN_DIR  .. "mame/layout",
	}
end

function createProjects_mame_mess(_target, _subtarget)
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
if (_subtarget=="mess") then
createMESSProjects(_target, _subtarget, "mameshared")
files {
	MAME_DIR .. "src/mame/machine/archimds.c",
	MAME_DIR .. "src/mame/video/archimds.c",
	MAME_DIR .. "src/mame/machine/amiga.c",
	MAME_DIR .. "src/mame/video/amiga.c",
	MAME_DIR .. "src/mame/video/amigaaga.c",
	MAME_DIR .. "src/mame/video/tia.c",
	MAME_DIR .. "src/mame/machine/atari.c",
	MAME_DIR .. "src/mame/video/atari.c",
	MAME_DIR .. "src/mame/video/antic.c",
	MAME_DIR .. "src/mame/video/gtia.c",
	MAME_DIR .. "src/mame/drivers/jaguar.c",
	MAME_DIR .. "src/mame/audio/jaguar.c",
	MAME_DIR .. "src/mame/video/jaguar.c",
	MAME_DIR .. "src/mame/audio/gorf.c",
	MAME_DIR .. "src/mame/audio/wow.c",
	MAME_DIR .. "src/mame/drivers/astrocde.c",
	MAME_DIR .. "src/mame/video/astrocde.c",
	MAME_DIR .. "src/mame/machine/kabuki.c",
	MAME_DIR .. "src/mame/video/pk8000.c",
	MAME_DIR .. "src/mame/video/ppu2c0x.c",
	MAME_DIR .. "src/mame/machine/snes.c",
	MAME_DIR .. "src/mame/audio/snes_snd.c",
	MAME_DIR .. "src/mame/machine/n64.c",
	MAME_DIR .. "src/mame/video/n64.c",
	MAME_DIR .. "src/mame/video/rdpblend.c",
	MAME_DIR .. "src/mame/video/rdptpipe.c",
	MAME_DIR .. "src/mame/machine/megadriv.c",
	MAME_DIR .. "src/mame/drivers/naomi.c",
	MAME_DIR .. "src/mame/machine/awboard.c",
	MAME_DIR .. "src/mame/machine/dc.c",
	MAME_DIR .. "src/mame/machine/dc-ctrl.c",
	MAME_DIR .. "src/mame/machine/gdrom.c",
	MAME_DIR .. "src/mame/machine/jvs13551.c",
	MAME_DIR .. "src/mame/machine/maple-dc.c",
	MAME_DIR .. "src/mame/machine/mapledev.c",
	MAME_DIR .. "src/mame/machine/mie.c",
	MAME_DIR .. "src/mame/machine/naomi.c",
	MAME_DIR .. "src/mame/machine/naomibd.c",
	MAME_DIR .. "src/mame/machine/naomig1.c",
	MAME_DIR .. "src/mame/machine/naomigd.c",
	MAME_DIR .. "src/mame/machine/naomim1.c",
	MAME_DIR .. "src/mame/machine/naomim2.c",
	MAME_DIR .. "src/mame/machine/naomim4.c",
	MAME_DIR .. "src/mame/machine/naomirom.c",
	MAME_DIR .. "src/mame/machine/315-5881_crypt.c",
	MAME_DIR .. "src/mame/video/powervr2.c",
	MAME_DIR .. "src/mame/drivers/neogeo.c",
	MAME_DIR .. "src/mame/machine/neocrypt.c",
	MAME_DIR .. "src/mame/machine/ng_memcard.c",
	MAME_DIR .. "src/mame/video/neogeo.c",
	MAME_DIR .. "src/mame/video/neogeo_spr.c",
	MAME_DIR .. "src/mame/drivers/cdi.c",
	MAME_DIR .. "src/mame/machine/cdi070.c",
	MAME_DIR .. "src/mame/machine/cdicdic.c",
	MAME_DIR .. "src/mame/machine/cdislave.c",
	MAME_DIR .. "src/mame/video/mcd212.c",
	MAME_DIR .. "src/mame/drivers/3do.c",
	MAME_DIR .. "src/mame/machine/3do.c",
	MAME_DIR .. "src/mame/drivers/konamim2.c",
	MAME_DIR .. "src/mame/drivers/vectrex.c",
	MAME_DIR .. "src/mame/machine/vectrex.c",
	MAME_DIR .. "src/mame/video/vectrex.c",
	MAME_DIR .. "src/mame/drivers/cps1.c",
	MAME_DIR .. "src/mame/video/cps1.c",
	MAME_DIR .. "src/mame/video/chihiro.c",
	MAME_DIR .. "src/mame/machine/xbox.c",
}
end
--------------------------------------------------
-- the following files are general components and
-- shared across a number of drivers
--------------------------------------------------
createMESSProjects(_target, _subtarget, "messshared")
files {
	MAME_DIR .. "src/mess/audio/mea8000.c",
	MAME_DIR .. "src/mess/machine/appldriv.c",
	MAME_DIR .. "src/mess/machine/applefdc.c",
	MAME_DIR .. "src/mess/machine/microdrv.c",
	MAME_DIR .. "src/mess/machine/smartmed.c",
	MAME_DIR .. "src/mess/machine/sonydriv.c",
	MAME_DIR .. "src/mess/machine/teleprinter.c",
	MAME_DIR .. "src/mess/machine/z80bin.c",
}
--------------------------------------------------
-- manufacturer-specific groupings for drivers
--------------------------------------------------

createMESSProjects(_target, _subtarget, "acorn")
files {
	MAME_DIR .. "src/mess/drivers/a310.c",
	MAME_DIR .. "src/mess/drivers/a6809.c",
	MAME_DIR .. "src/mess/drivers/acrnsys1.c",
	MAME_DIR .. "src/mess/drivers/atom.c",
	MAME_DIR .. "src/mess/drivers/bbc.c",
	MAME_DIR .. "src/mess/machine/bbc.c",
	MAME_DIR .. "src/mess/video/bbc.c",
	MAME_DIR .. "src/mess/drivers/bbcbc.c",
	MAME_DIR .. "src/mess/drivers/electron.c",
	MAME_DIR .. "src/mess/machine/electron.c",
	MAME_DIR .. "src/mess/video/electron.c",
	MAME_DIR .. "src/mess/drivers/riscpc.c",
	MAME_DIR .. "src/mess/drivers/z88.c",
	MAME_DIR .. "src/mess/machine/upd65031.c",
	MAME_DIR .. "src/mess/video/z88.c",
}

createMESSProjects(_target, _subtarget, "act")
files {
	MAME_DIR .. "src/mess/drivers/apricot.c",
	MAME_DIR .. "src/mess/drivers/apricotf.c",
	MAME_DIR .. "src/mess/drivers/apricotp.c",
	MAME_DIR .. "src/mess/machine/apricotkb.c",
	MAME_DIR .. "src/mess/machine/apricotkb_hle.c",
	MAME_DIR .. "src/mess/drivers/victor9k.c",
	MAME_DIR .. "src/mess/machine/victor9kb.c",
	MAME_DIR .. "src/mess/machine/victor9k_fdc.c",
}

createMESSProjects(_target, _subtarget, "adc")
files {
	MAME_DIR .. "src/mess/drivers/super6.c",
	MAME_DIR .. "src/mess/drivers/superslave.c",
}

createMESSProjects(_target, _subtarget, "alesis")
files {
	MAME_DIR .. "src/mess/drivers/alesis.c",
	MAME_DIR .. "src/mess/audio/alesis.c",
	MAME_DIR .. "src/mess/video/alesis.c",
}

createMESSProjects(_target, _subtarget, "altos")
files {
	MAME_DIR .. "src/mess/drivers/altos5.c",
}

createMESSProjects(_target, _subtarget, "amiga")
files {
	MAME_DIR .. "src/mess/drivers/amiga.c",
	MAME_DIR .. "src/mess/machine/amigakbd.c",
}

createMESSProjects(_target, _subtarget, "amstrad")
files {
	MAME_DIR .. "src/mess/drivers/amstrad.c",
	MAME_DIR .. "src/mess/machine/amstrad.c",
	MAME_DIR .. "src/mess/drivers/amstr_pc.c",
	MAME_DIR .. "src/mess/drivers/nc.c",
	MAME_DIR .. "src/mess/machine/nc.c",
	MAME_DIR .. "src/mess/video/nc.c",
	MAME_DIR .. "src/mess/drivers/pc1512.c",
	MAME_DIR .. "src/mess/machine/pc1512kb.c",
	MAME_DIR .. "src/mess/video/pc1512.c",
	MAME_DIR .. "src/mess/drivers/pcw.c",
	MAME_DIR .. "src/mess/video/pcw.c",
	MAME_DIR .. "src/mess/drivers/pcw16.c",
	MAME_DIR .. "src/mess/video/pcw16.c",
	MAME_DIR .. "src/mess/drivers/pda600.c",
}

createMESSProjects(_target, _subtarget, "apf")
files {
	MAME_DIR .. "src/mess/drivers/apf.c",
}

createMESSProjects(_target, _subtarget, "apollo")
files {
	MAME_DIR .. "src/mess/drivers/apollo.c",
	MAME_DIR .. "src/mess/machine/apollo.c",
	MAME_DIR .. "src/mess/machine/apollo_dbg.c",
	MAME_DIR .. "src/mess/machine/apollo_kbd.c",
	MAME_DIR .. "src/mess/video/apollo.c",
}

createMESSProjects(_target, _subtarget, "apple")
files {
	MAME_DIR .. "src/mess/drivers/apple1.c",
	MAME_DIR .. "src/mess/machine/apple1.c",
	MAME_DIR .. "src/mess/video/apple1.c",
	MAME_DIR .. "src/mess/drivers/apple2.c",
	MAME_DIR .. "src/mess/drivers/apple2e.c",
	MAME_DIR .. "src/mess/machine/apple2.c",
	MAME_DIR .. "src/mess/video/apple2.c",
	MAME_DIR .. "src/mess/drivers/tk2000.c",
	MAME_DIR .. "src/mess/drivers/apple2gs.c",
	MAME_DIR .. "src/mess/machine/apple2gs.c",
	MAME_DIR .. "src/mess/video/apple2gs.c",
	MAME_DIR .. "src/mess/drivers/apple3.c",
	MAME_DIR .. "src/mess/machine/apple3.c",
	MAME_DIR .. "src/mess/video/apple3.c",
	MAME_DIR .. "src/mess/drivers/lisa.c",
	MAME_DIR .. "src/mess/machine/lisa.c",
	MAME_DIR .. "src/mess/drivers/mac.c",
	MAME_DIR .. "src/mess/audio/mac.c",
	MAME_DIR .. "src/mess/machine/egret.c",
	MAME_DIR .. "src/mess/machine/mac.c",
	MAME_DIR .. "src/mess/machine/macadb.c",
	MAME_DIR .. "src/mess/machine/macrtc.c",
	MAME_DIR .. "src/mess/machine/mackbd.c",
	MAME_DIR .. "src/mess/machine/swim.c",
	MAME_DIR .. "src/mess/video/mac.c",
	MAME_DIR .. "src/mess/drivers/macpci.c",
	MAME_DIR .. "src/mess/machine/macpci.c",
	MAME_DIR .. "src/mess/machine/cuda.c",
}

createMESSProjects(_target, _subtarget, "applied")
files {
	MAME_DIR .. "src/mess/drivers/mbee.c",
	MAME_DIR .. "src/mess/machine/mbee.c",
	MAME_DIR .. "src/mess/video/mbee.c",
}

createMESSProjects(_target, _subtarget, "arcadia")
files {
	MAME_DIR .. "src/mess/drivers/arcadia.c",
	MAME_DIR .. "src/mess/audio/arcadia.c",
	MAME_DIR .. "src/mess/video/arcadia.c",
}

createMESSProjects(_target, _subtarget, "ascii")
files {
	MAME_DIR .. "src/mess/drivers/msx.c",
	MAME_DIR .. "src/mess/machine/msx.c",
	MAME_DIR .. "src/mess/machine/msx_matsushita.c",
	MAME_DIR .. "src/mess/machine/msx_s1985.c",
	MAME_DIR .. "src/mess/machine/msx_switched.c",
	MAME_DIR .. "src/mess/machine/msx_systemflags.c",
}

createMESSProjects(_target, _subtarget, "at")
files {
	MAME_DIR .. "src/mess/drivers/at.c",
	MAME_DIR .. "src/mess/machine/at.c",
	MAME_DIR .. "src/mess/drivers/ct486.c",
}

createMESSProjects(_target, _subtarget, "atari")
files {
	MAME_DIR .. "src/mess/drivers/a2600.c",
	MAME_DIR .. "src/mess/drivers/a7800.c",
	MAME_DIR .. "src/mess/video/maria.c",
	MAME_DIR .. "src/mess/drivers/atari400.c",
	MAME_DIR .. "src/mess/machine/atarifdc.c",
	MAME_DIR .. "src/mess/drivers/atarist.c",
	MAME_DIR .. "src/mess/video/atarist.c",
	MAME_DIR .. "src/mess/drivers/lynx.c",
	MAME_DIR .. "src/mess/audio/lynx.c",
	MAME_DIR .. "src/mess/machine/lynx.c",
	MAME_DIR .. "src/mess/drivers/portfoli.c",
}

createMESSProjects(_target, _subtarget, "att")
files {
	MAME_DIR .. "src/mess/drivers/unixpc.c",
}

createMESSProjects(_target, _subtarget, "bally")
files {
	MAME_DIR .. "src/mess/drivers/astrocde.c",
}

createMESSProjects(_target, _subtarget, "banctec")
files {
	MAME_DIR .. "src/mess/drivers/banctec.c",
}

createMESSProjects(_target, _subtarget, "bandai")
files {
	MAME_DIR .. "src/mess/drivers/sv8000.c",
	MAME_DIR .. "src/mess/drivers/rx78.c",
	MAME_DIR .. "src/mess/drivers/tamag1.c",
	MAME_DIR .. "src/mess/drivers/wswan.c",
	MAME_DIR .. "src/mess/audio/wswan_snd.c",
	MAME_DIR .. "src/mess/machine/wswan.c",
	MAME_DIR .. "src/mess/video/wswan_video.c",
}

createMESSProjects(_target, _subtarget, "be")
files {
	MAME_DIR .. "src/mess/drivers/bebox.c",
	MAME_DIR .. "src/mess/machine/bebox.c",
}

createMESSProjects(_target, _subtarget, "bnpo")
files {
	MAME_DIR .. "src/mess/drivers/b2m.c",
	MAME_DIR .. "src/mess/machine/b2m.c",
	MAME_DIR .. "src/mess/video/b2m.c",
}

createMESSProjects(_target, _subtarget, "bondwell")
files {
	MAME_DIR .. "src/mess/drivers/bw12.c",
	MAME_DIR .. "src/mess/drivers/bw2.c",
}

createMESSProjects(_target, _subtarget, "booth")
files {
	MAME_DIR .. "src/mess/drivers/apexc.c",
}

createMESSProjects(_target, _subtarget, "camputers")
files {
	MAME_DIR .. "src/mess/drivers/camplynx.c",
}

createMESSProjects(_target, _subtarget, "canon")
files {             
	MAME_DIR .. "src/mess/drivers/cat.c",       
	MAME_DIR .. "src/mess/drivers/x07.c",       
	MAME_DIR .. "src/mess/drivers/canon_s80.c",
}

createMESSProjects(_target, _subtarget, "cantab")
files {
	MAME_DIR .. "src/mess/drivers/ace.c",
}

createMESSProjects(_target, _subtarget, "casio")
files {
	MAME_DIR .. "src/mess/drivers/casloopy.c",
	MAME_DIR .. "src/mess/drivers/cfx9850.c",
	MAME_DIR .. "src/mess/drivers/fp200.c",
	MAME_DIR .. "src/mess/drivers/fp1100.c",
	MAME_DIR .. "src/mess/drivers/fp6000.c",
	MAME_DIR .. "src/mess/drivers/pb1000.c",
	MAME_DIR .. "src/mess/drivers/pv1000.c",
	MAME_DIR .. "src/mess/drivers/pv2000.c",
}

createMESSProjects(_target, _subtarget, "cbm")
files {
	MAME_DIR .. "src/mess/drivers/c128.c",
	MAME_DIR .. "src/mess/drivers/c64.c",
	MAME_DIR .. "src/mess/drivers/c64dtv.c",
	MAME_DIR .. "src/mess/drivers/c65.c",
	MAME_DIR .. "src/mess/drivers/c900.c",
	MAME_DIR .. "src/mess/drivers/cbm2.c",
	MAME_DIR .. "src/mess/drivers/clcd.c",
	MAME_DIR .. "src/mess/drivers/pet.c",
	MAME_DIR .. "src/mess/drivers/plus4.c",
	MAME_DIR .. "src/mess/drivers/vic10.c",
	MAME_DIR .. "src/mess/drivers/vic20.c",
	MAME_DIR .. "src/mess/machine/cbm_snqk.c",
	MAME_DIR .. "src/mess/drivers/mps1230.c",
}

createMESSProjects(_target, _subtarget, "cccp")
files {
	MAME_DIR .. "src/mess/drivers/argo.c",
	MAME_DIR .. "src/mess/drivers/cm1800.c",
	MAME_DIR .. "src/mess/drivers/lviv.c",
	MAME_DIR .. "src/mess/machine/lviv.c",
	MAME_DIR .. "src/mess/video/lviv.c",
	MAME_DIR .. "src/mess/drivers/mikro80.c",
	MAME_DIR .. "src/mess/machine/mikro80.c",
	MAME_DIR .. "src/mess/video/mikro80.c",
	MAME_DIR .. "src/mess/drivers/pk8000.c",
	MAME_DIR .. "src/mess/drivers/pk8020.c",
	MAME_DIR .. "src/mess/machine/pk8020.c",
	MAME_DIR .. "src/mess/video/pk8020.c",
	MAME_DIR .. "src/mess/drivers/pyl601.c",
	MAME_DIR .. "src/mess/drivers/sm1800.c",
	MAME_DIR .. "src/mess/drivers/uknc.c",
	MAME_DIR .. "src/mess/drivers/unior.c",
	MAME_DIR .. "src/mess/drivers/ut88.c",
	MAME_DIR .. "src/mess/machine/ut88.c",
	MAME_DIR .. "src/mess/video/ut88.c",
	MAME_DIR .. "src/mess/drivers/vector06.c",
	MAME_DIR .. "src/mess/machine/vector06.c",
	MAME_DIR .. "src/mess/video/vector06.c",
	MAME_DIR .. "src/mess/drivers/vta2000.c",
}

createMESSProjects(_target, _subtarget, "cce")
files {
	MAME_DIR .. "src/mess/drivers/mc1000.c",
}

createMESSProjects(_target, _subtarget, "ccs")
files {
	MAME_DIR .. "src/mess/drivers/ccs2810.c",
	MAME_DIR .. "src/mess/drivers/ccs300.c",
}

createMESSProjects(_target, _subtarget, "chromatics")
files {
	MAME_DIR .. "src/mess/drivers/cgc7900.c",
	MAME_DIR .. "src/mess/video/cgc7900.c",
}

createMESSProjects(_target, _subtarget, "coleco")
files {
	MAME_DIR .. "src/mess/drivers/adam.c",
	MAME_DIR .. "src/mess/drivers/coleco.c",
	MAME_DIR .. "src/mess/machine/coleco.c",
}

createMESSProjects(_target, _subtarget, "cromemco")
files {
	MAME_DIR .. "src/mess/drivers/c10.c",
	MAME_DIR .. "src/mess/drivers/mcb216.c",
}

createMESSProjects(_target, _subtarget, "comx")
files {
	MAME_DIR .. "src/mess/drivers/comx35.c",
	MAME_DIR .. "src/mess/video/comx35.c",
}

createMESSProjects(_target, _subtarget, "concept")
files {
	MAME_DIR .. "src/mess/drivers/concept.c",
	MAME_DIR .. "src/mess/machine/concept.c",
}

createMESSProjects(_target, _subtarget, "conitec")
files {
	MAME_DIR .. "src/mess/drivers/prof180x.c",
	MAME_DIR .. "src/mess/drivers/prof80.c",
	MAME_DIR .. "src/mess/machine/prof80mmu.c",
}

createMESSProjects(_target, _subtarget, "cybiko")
files {
	MAME_DIR .. "src/mess/drivers/cybiko.c",
	MAME_DIR .. "src/mess/machine/cybiko.c",
}

createMESSProjects(_target, _subtarget, "dai")
files {
	MAME_DIR .. "src/mess/drivers/dai.c",
	MAME_DIR .. "src/mess/audio/dai_snd.c",
	MAME_DIR .. "src/mess/machine/dai.c",
	MAME_DIR .. "src/mess/video/dai.c",
}

createMESSProjects(_target, _subtarget, "ddr")
files {
	MAME_DIR .. "src/mess/drivers/ac1.c",
	MAME_DIR .. "src/mess/machine/ac1.c",
	MAME_DIR .. "src/mess/video/ac1.c",
	MAME_DIR .. "src/mess/drivers/bcs3.c",
	MAME_DIR .. "src/mess/drivers/c80.c",
	MAME_DIR .. "src/mess/drivers/huebler.c",
	MAME_DIR .. "src/mess/drivers/jtc.c",
	MAME_DIR .. "src/mess/drivers/kramermc.c",
	MAME_DIR .. "src/mess/machine/kramermc.c",
	MAME_DIR .. "src/mess/video/kramermc.c",
	MAME_DIR .. "src/mess/drivers/llc.c",
	MAME_DIR .. "src/mess/machine/llc.c",
	MAME_DIR .. "src/mess/video/llc.c",
	MAME_DIR .. "src/mess/drivers/nanos.c",
	MAME_DIR .. "src/mess/drivers/pcm.c",
	MAME_DIR .. "src/mess/drivers/vcs80.c",
	MAME_DIR .. "src/mess/machine/k7659kb.c",
}

createMESSProjects(_target, _subtarget, "dec")
files {
	MAME_DIR .. "src/mess/drivers/dct11em.c",
	MAME_DIR .. "src/mess/drivers/dectalk.c",
	MAME_DIR .. "src/mess/drivers/decwritr.c",
	MAME_DIR .. "src/mess/drivers/pdp11.c",
	MAME_DIR .. "src/mess/drivers/vax11.c",
	MAME_DIR .. "src/mess/drivers/rainbow.c",
	MAME_DIR .. "src/mess/drivers/vk100.c",
	MAME_DIR .. "src/mess/drivers/vt100.c",
	MAME_DIR .. "src/mess/drivers/vt220.c",
	MAME_DIR .. "src/mess/drivers/vt240.c",
	MAME_DIR .. "src/mess/drivers/vt320.c",
	MAME_DIR .. "src/mess/drivers/vt520.c",
	MAME_DIR .. "src/mess/machine/dec_lk201.c",
	MAME_DIR .. "src/mess/machine/rx01.c",
	MAME_DIR .. "src/mess/video/vtvideo.c",
}

createMESSProjects(_target, _subtarget, "dicksmth")
files {
	MAME_DIR .. "src/mess/drivers/super80.c",
	MAME_DIR .. "src/mess/machine/super80.c",
	MAME_DIR .. "src/mess/video/super80.c",
}

createMESSProjects(_target, _subtarget, "dms")
files {
	MAME_DIR .. "src/mess/drivers/dms5000.c",
	MAME_DIR .. "src/mess/drivers/dms86.c",
	MAME_DIR .. "src/mess/drivers/zsbc3.c",
}

createMESSProjects(_target, _subtarget, "dragon")
files {
	MAME_DIR .. "src/mess/drivers/dgn_beta.c",
	MAME_DIR .. "src/mess/machine/dgn_beta.c",
	MAME_DIR .. "src/mess/video/dgn_beta.c",
}

createMESSProjects(_target, _subtarget, "drc")
files {
	MAME_DIR .. "src/mess/drivers/zrt80.c",
}

createMESSProjects(_target, _subtarget, "eaca")
files {
	MAME_DIR .. "src/mess/drivers/cgenie.c",
}

createMESSProjects(_target, _subtarget, "einis")
files {
	MAME_DIR .. "src/mess/drivers/pecom.c",
	MAME_DIR .. "src/mess/machine/pecom.c",
	MAME_DIR .. "src/mess/video/pecom.c",
}

createMESSProjects(_target, _subtarget, "elektrka")
files {
	MAME_DIR .. "src/mess/drivers/bk.c",
	MAME_DIR .. "src/mess/machine/bk.c",
	MAME_DIR .. "src/mess/video/bk.c",
	MAME_DIR .. "src/mess/drivers/dvk_kcgd.c",
	MAME_DIR .. "src/mess/drivers/dvk_ksm.c",
	MAME_DIR .. "src/mess/machine/ms7004.c",
	MAME_DIR .. "src/mess/drivers/mk85.c",
	MAME_DIR .. "src/mess/drivers/mk90.c",
}

createMESSProjects(_target, _subtarget, "elektor")
files {
	MAME_DIR .. "src/mess/drivers/ec65.c",
	MAME_DIR .. "src/mess/drivers/elekscmp.c",
	MAME_DIR .. "src/mess/drivers/junior.c",
}

createMESSProjects(_target, _subtarget, "ensoniq")
files {
	MAME_DIR .. "src/mess/drivers/esq1.c",
	MAME_DIR .. "src/mess/drivers/esq5505.c",
	MAME_DIR .. "src/mess/drivers/esqasr.c",
	MAME_DIR .. "src/mess/drivers/esqkt.c",
	MAME_DIR .. "src/mess/drivers/esqmr.c",
	MAME_DIR .. "src/mess/drivers/mirage.c",
	MAME_DIR .. "src/mess/machine/esqpanel.c",
	MAME_DIR .. "src/mess/machine/esqvfd.c",
}

createMESSProjects(_target, _subtarget, "enterprise")
files {
	MAME_DIR .. "src/mess/drivers/ep64.c",
	MAME_DIR .. "src/mess/audio/dave.c",
	MAME_DIR .. "src/mess/video/nick.c",
}

createMESSProjects(_target, _subtarget, "entex")
files {
	MAME_DIR .. "src/mess/drivers/advision.c",
	MAME_DIR .. "src/mess/machine/advision.c",
	MAME_DIR .. "src/mess/video/advision.c",
}

createMESSProjects(_target, _subtarget, "epoch")
files {
	MAME_DIR .. "src/mess/drivers/gamepock.c",
	MAME_DIR .. "src/mess/machine/gamepock.c",
	MAME_DIR .. "src/mess/drivers/scv.c",
	MAME_DIR .. "src/mess/audio/upd1771.c",
}

createMESSProjects(_target, _subtarget, "epson")
files {
	MAME_DIR .. "src/mess/drivers/hx20.c",
	MAME_DIR .. "src/mess/drivers/px4.c",
	MAME_DIR .. "src/mess/drivers/px8.c",
	MAME_DIR .. "src/mess/drivers/qx10.c",
	MAME_DIR .. "src/mess/machine/qx10kbd.c",
}

createMESSProjects(_target, _subtarget, "exidy")
files {
	MAME_DIR .. "src/mess/machine/sorcerer.c",
	MAME_DIR .. "src/mess/drivers/sorcerer.c",
	MAME_DIR .. "src/mess/machine/micropolis.c",
}

createMESSProjects(_target, _subtarget, "fairch")
files {
	MAME_DIR .. "src/mess/drivers/channelf.c",
	MAME_DIR .. "src/mess/audio/channelf.c",
	MAME_DIR .. "src/mess/video/channelf.c",
}

createMESSProjects(_target, _subtarget, "fidelity")
files {
	MAME_DIR .. "src/mess/drivers/csc.c",
	MAME_DIR .. "src/mess/drivers/fidelz80.c",
}

createMESSProjects(_target, _subtarget, "force")
files {
	MAME_DIR .. "src/mess/drivers/force68k.c",
}

createMESSProjects(_target, _subtarget, "fujitsu")
files {
	MAME_DIR .. "src/mess/drivers/fmtowns.c",
	MAME_DIR .. "src/mess/video/fmtowns.c",
	MAME_DIR .. "src/mess/machine/fm_scsi.c",
	MAME_DIR .. "src/mess/drivers/fm7.c",
	MAME_DIR .. "src/mess/video/fm7.c",
}

createMESSProjects(_target, _subtarget, "funtech")
files {
	MAME_DIR .. "src/mess/drivers/supracan.c",
}

createMESSProjects(_target, _subtarget, "galaxy")
files {
	MAME_DIR .. "src/mess/drivers/galaxy.c",
	MAME_DIR .. "src/mess/machine/galaxy.c",
	MAME_DIR .. "src/mess/video/galaxy.c",
}

createMESSProjects(_target, _subtarget, "gamepark")
files {
	MAME_DIR .. "src/mess/drivers/gp2x.c",
	MAME_DIR .. "src/mess/drivers/gp32.c",
}

createMESSProjects(_target, _subtarget, "gi")
files {
	MAME_DIR .. "src/mess/drivers/hh_pic16.c",
}

createMESSProjects(_target, _subtarget, "grundy")
files {
	MAME_DIR .. "src/mess/drivers/newbrain.c",
	MAME_DIR .. "src/mess/video/newbrain.c",
}

createMESSProjects(_target, _subtarget, "hartung")
files {
	MAME_DIR .. "src/mess/drivers/gmaster.c",
}

createMESSProjects(_target, _subtarget, "heathkit")
files {
	MAME_DIR .. "src/mess/drivers/et3400.c",
	MAME_DIR .. "src/mess/drivers/h8.c",
	MAME_DIR .. "src/mess/drivers/h19.c",
	MAME_DIR .. "src/mess/drivers/h89.c",
}

createMESSProjects(_target, _subtarget, "hegener")
files {
	MAME_DIR .. "src/mess/drivers/glasgow.c",
	MAME_DIR .. "src/mess/drivers/mephisto.c",
	MAME_DIR .. "src/mess/drivers/mmodular.c",
	MAME_DIR .. "src/mess/drivers/stratos.c",
	MAME_DIR .. "src/mess/machine/mboard.c",
}

createMESSProjects(_target, _subtarget, "hitachi")
files {
	MAME_DIR .. "src/mess/drivers/b16.c",
	MAME_DIR .. "src/mess/drivers/bmjr.c",
	MAME_DIR .. "src/mess/drivers/bml3.c",
	MAME_DIR .. "src/mess/drivers/hh_hmcs40.c",
}

createMESSProjects(_target, _subtarget, "homebrew")
files {
	MAME_DIR .. "src/mess/drivers/4004clk.c",
	MAME_DIR .. "src/mess/drivers/68ksbc.c",
	MAME_DIR .. "src/mess/drivers/craft.c",
	MAME_DIR .. "src/mess/drivers/homez80.c",
	MAME_DIR .. "src/mess/drivers/p112.c",
	MAME_DIR .. "src/mess/drivers/phunsy.c",
	MAME_DIR .. "src/mess/drivers/pimps.c",
	MAME_DIR .. "src/mess/drivers/ravens.c",
	MAME_DIR .. "src/mess/drivers/sbc6510.c",
	MAME_DIR .. "src/mess/drivers/sitcom.c",
	MAME_DIR .. "src/mess/drivers/slc1.c",
	MAME_DIR .. "src/mess/drivers/uzebox.c",
	MAME_DIR .. "src/mess/drivers/z80dev.c",
}

createMESSProjects(_target, _subtarget, "homelab")
files {
	MAME_DIR .. "src/mess/drivers/homelab.c",
}

createMESSProjects(_target, _subtarget, "hp")
files {
	MAME_DIR .. "src/mess/drivers/hp16500.c",
	MAME_DIR .. "src/mess/drivers/hp48.c",
	MAME_DIR .. "src/mess/machine/hp48.c",
	MAME_DIR .. "src/mess/video/hp48.c",
	MAME_DIR .. "src/mess/drivers/hp49gp.c",
	MAME_DIR .. "src/mess/drivers/hp9845.c",
	MAME_DIR .. "src/mess/drivers/hp9k.c",
	MAME_DIR .. "src/mess/drivers/hp9k_3xx.c",
	MAME_DIR .. "src/mess/drivers/hp64k.c",
	MAME_DIR .. "src/mess/drivers/hp_ipc.c",
}

createMESSProjects(_target, _subtarget, "hec2hrp")
files {
	MAME_DIR .. "src/mess/drivers/hec2hrp.c",
	MAME_DIR .. "src/mess/machine/hec2hrp.c",
	MAME_DIR .. "src/mess/machine/hecdisk2.c",
	MAME_DIR .. "src/mess/video/hec2video.c",
	MAME_DIR .. "src/mess/drivers/interact.c",
}

createMESSProjects(_target, _subtarget, "heurikon")
files {          
	MAME_DIR .. "src/mess/drivers/hk68v10.c",  
}

createMESSProjects(_target, _subtarget, "intel")
files {
	MAME_DIR .. "src/mess/drivers/basic52.c",
	MAME_DIR .. "src/mess/drivers/imds.c",
	MAME_DIR .. "src/mess/drivers/ipc.c",
	MAME_DIR .. "src/mess/drivers/ipds.c",
	MAME_DIR .. "src/mess/drivers/isbc.c",
	MAME_DIR .. "src/mess/machine/isbc_215g.c",
	MAME_DIR .. "src/mess/drivers/rex6000.c",
	MAME_DIR .. "src/mess/drivers/sdk80.c",
	MAME_DIR .. "src/mess/drivers/sdk85.c",
	MAME_DIR .. "src/mess/drivers/sdk86.c",
	MAME_DIR .. "src/mess/drivers/imds2.c",
}

createMESSProjects(_target, _subtarget, "imp")
files {
	MAME_DIR .. "src/mess/drivers/tim011.c",
	MAME_DIR .. "src/mess/drivers/tim100.c",
}

createMESSProjects(_target, _subtarget, "interton")
files {
	MAME_DIR .. "src/mess/drivers/vc4000.c",
	MAME_DIR .. "src/mess/audio/vc4000snd.c",
	MAME_DIR .. "src/mess/video/vc4000.c",
}

createMESSProjects(_target, _subtarget, "intv")
files {
	MAME_DIR .. "src/mess/drivers/intv.c",
	MAME_DIR .. "src/mess/machine/intv.c",
	MAME_DIR .. "src/mess/video/intv.c",
	MAME_DIR .. "src/mess/video/stic.c",
}

createMESSProjects(_target, _subtarget, "isc")
files {
	MAME_DIR .. "src/mess/drivers/compucolor.c",
}

createMESSProjects(_target, _subtarget, "kaypro")
files {
	MAME_DIR .. "src/mess/drivers/kaypro.c",
	MAME_DIR .. "src/mess/machine/kaypro.c",
	MAME_DIR .. "src/mess/machine/kay_kbd.c",
	MAME_DIR .. "src/mess/video/kaypro.c",
}

createMESSProjects(_target, _subtarget, "koei")
files {
	MAME_DIR .. "src/mess/drivers/pasogo.c",
}

createMESSProjects(_target, _subtarget, "kyocera")
files {
	MAME_DIR .. "src/mess/drivers/kyocera.c",
	MAME_DIR .. "src/mess/video/kyocera.c",
}

createMESSProjects(_target, _subtarget, "luxor")
files {
	MAME_DIR .. "src/mess/drivers/abc80.c",
	MAME_DIR .. "src/mess/machine/abc80kb.c",
	MAME_DIR .. "src/mess/video/abc80.c",
	MAME_DIR .. "src/mess/drivers/abc80x.c",
	MAME_DIR .. "src/mess/video/abc800.c",
	MAME_DIR .. "src/mess/video/abc802.c",
	MAME_DIR .. "src/mess/video/abc806.c",
	MAME_DIR .. "src/mess/drivers/abc1600.c",
	MAME_DIR .. "src/mess/machine/abc1600mac.c",
	MAME_DIR .. "src/mess/video/abc1600.c",
}

createMESSProjects(_target, _subtarget, "magnavox")
files {
	MAME_DIR .. "src/mess/drivers/odyssey2.c",
}

createMESSProjects(_target, _subtarget, "makerbot")
files {
	MAME_DIR .. "src/mess/drivers/replicator.c",
}

createMESSProjects(_target, _subtarget, "marx")
files {
	MAME_DIR .. "src/mess/drivers/elecbowl.c",
}

createMESSProjects(_target, _subtarget, "mattel")
files {
	MAME_DIR .. "src/mess/drivers/aquarius.c",
	MAME_DIR .. "src/mess/video/aquarius.c",
	MAME_DIR .. "src/mess/drivers/juicebox.c",
	MAME_DIR .. "src/mess/drivers/hyperscan.c",
}

createMESSProjects(_target, _subtarget, "matsushi")
files {
	MAME_DIR .. "src/mess/drivers/jr100.c",
	MAME_DIR .. "src/mess/drivers/jr200.c",
	MAME_DIR .. "src/mess/drivers/myb3k.c",
}

createMESSProjects(_target, _subtarget, "mb")
files {
	MAME_DIR .. "src/mess/drivers/mbdtower.c",
	MAME_DIR .. "src/mess/drivers/microvsn.c",
}

createMESSProjects(_target, _subtarget, "mchester")
files {
	MAME_DIR .. "src/mess/drivers/ssem.c",
}

createMESSProjects(_target, _subtarget, "memotech")
files {
	MAME_DIR .. "src/mess/drivers/mtx.c",
	MAME_DIR .. "src/mess/machine/mtx.c",
}

createMESSProjects(_target, _subtarget, "mgu")
files {
	MAME_DIR .. "src/mess/drivers/irisha.c",
}

createMESSProjects(_target, _subtarget, "microkey")
files {
	MAME_DIR .. "src/mess/drivers/primo.c",
	MAME_DIR .. "src/mess/machine/primo.c",
	MAME_DIR .. "src/mess/video/primo.c",
}

createMESSProjects(_target, _subtarget, "microsoft")
files {
	MAME_DIR .. "src/mess/drivers/xbox.c",
}

createMESSProjects(_target, _subtarget, "mit")
files {
	MAME_DIR .. "src/mess/drivers/tx0.c",
	MAME_DIR .. "src/mess/video/crt.c",
	MAME_DIR .. "src/mess/video/tx0.c",
}

createMESSProjects(_target, _subtarget, "mits")
files {
	MAME_DIR .. "src/mess/drivers/altair.c",
	MAME_DIR .. "src/mess/drivers/mits680b.c",
}

createMESSProjects(_target, _subtarget, "mitsubishi")
files {
	MAME_DIR .. "src/mess/drivers/hh_melps4.c",
	MAME_DIR .. "src/mess/drivers/multi8.c",
	MAME_DIR .. "src/mess/drivers/multi16.c",
}

createMESSProjects(_target, _subtarget, "mizar")
files {          
	MAME_DIR .. "src/mess/drivers/mzr8105.c",  
}

createMESSProjects(_target, _subtarget, "morrow")
files {
	MAME_DIR .. "src/mess/drivers/microdec.c",
	MAME_DIR .. "src/mess/drivers/mpz80.c",
	MAME_DIR .. "src/mess/drivers/tricep.c",
}

createMESSProjects(_target, _subtarget, "mos")
files {
	MAME_DIR .. "src/mess/drivers/kim1.c",
}

createMESSProjects(_target, _subtarget, "motorola")
files {
	MAME_DIR .. "src/mess/drivers/m6805evs.c",
	MAME_DIR .. "src/mess/drivers/mekd2.c",
}

createMESSProjects(_target, _subtarget, "multitch")
files {
	MAME_DIR .. "src/mess/drivers/mkit09.c",
	MAME_DIR .. "src/mess/drivers/mpf1.c",
}

createMESSProjects(_target, _subtarget, "nakajima")
files {
	MAME_DIR .. "src/mess/drivers/nakajies.c",
}

createMESSProjects(_target, _subtarget, "nascom")
files {
	MAME_DIR .. "src/mess/drivers/nascom1.c",
}

createMESSProjects(_target, _subtarget, "ne")
files {
	MAME_DIR .. "src/mess/drivers/z80ne.c",
	MAME_DIR .. "src/mess/machine/z80ne.c",
}

createMESSProjects(_target, _subtarget, "nec")
files {
	MAME_DIR .. "src/mess/drivers/apc.c",
	MAME_DIR .. "src/mess/drivers/pce.c",
	MAME_DIR .. "src/mess/machine/pce.c",
	MAME_DIR .. "src/mess/machine/pce_cd.c",
	MAME_DIR .. "src/mess/drivers/pcfx.c",
	MAME_DIR .. "src/mess/drivers/pc6001.c",
	MAME_DIR .. "src/mess/drivers/pc8401a.c",
	MAME_DIR .. "src/mess/video/pc8401a.c",
	MAME_DIR .. "src/mess/drivers/pc8001.c",
	MAME_DIR .. "src/mess/drivers/pc8801.c",
	MAME_DIR .. "src/mess/drivers/pc88va.c",
	MAME_DIR .. "src/mess/drivers/pc100.c",
	MAME_DIR .. "src/mess/drivers/pc9801.c",
	MAME_DIR .. "src/mess/machine/pc9801_26.c",
	MAME_DIR .. "src/mess/machine/pc9801_86.c",
	MAME_DIR .. "src/mess/machine/pc9801_118.c",
	MAME_DIR .. "src/mess/machine/pc9801_cbus.c",
	MAME_DIR .. "src/mess/machine/pc9801_kbd.c",
	MAME_DIR .. "src/mess/drivers/tk80bs.c",
	MAME_DIR .. "src/mess/drivers/hh_ucom4.c",
}

createMESSProjects(_target, _subtarget, "netronic")
files {
	MAME_DIR .. "src/mess/drivers/elf.c",
	MAME_DIR .. "src/mess/drivers/exp85.c",
}

createMESSProjects(_target, _subtarget, "next")
files {
	MAME_DIR .. "src/mess/drivers/next.c",
	MAME_DIR .. "src/mess/machine/nextkbd.c",
	MAME_DIR .. "src/mess/machine/nextmo.c",
}

createMESSProjects(_target, _subtarget, "nintendo")
files {
	MAME_DIR .. "src/mess/drivers/gb.c",
	MAME_DIR .. "src/mess/audio/gb.c",
	MAME_DIR .. "src/mess/machine/gb.c",
	MAME_DIR .. "src/mess/video/gb_lcd.c",
	MAME_DIR .. "src/mess/drivers/gba.c",
	MAME_DIR .. "src/mess/video/gba.c",
	MAME_DIR .. "src/mess/drivers/n64.c",
	MAME_DIR .. "src/mess/drivers/nes.c",
	MAME_DIR .. "src/mess/machine/nes.c",
	MAME_DIR .. "src/mess/video/nes.c",
	MAME_DIR .. "src/mess/drivers/pokemini.c",
	MAME_DIR .. "src/mess/drivers/snes.c",
	MAME_DIR .. "src/mess/machine/snescx4.c",
	MAME_DIR .. "src/mess/drivers/vboy.c",
	MAME_DIR .. "src/mess/audio/vboy.c",
}

createMESSProjects(_target, _subtarget, "nokia")
files {
	MAME_DIR .. "src/mess/drivers/mikromik.c",
	MAME_DIR .. "src/mess/machine/mm1kb.c",
	MAME_DIR .. "src/mess/video/mikromik.c",
}

createMESSProjects(_target, _subtarget, "northstar")
files {
	MAME_DIR .. "src/mess/drivers/horizon.c",
}

createMESSProjects(_target, _subtarget, "novag")
files {
	MAME_DIR .. "src/mess/drivers/mk1.c",
	MAME_DIR .. "src/mess/drivers/mk2.c",
	MAME_DIR .. "src/mess/drivers/ssystem3.c",
	MAME_DIR .. "src/mess/video/ssystem3.c",
	MAME_DIR .. "src/mess/drivers/supercon.c",
}

createMESSProjects(_target, _subtarget, "olivetti")
files {
	MAME_DIR .. "src/mess/drivers/m20.c",
	MAME_DIR .. "src/mess/machine/m20_kbd.c",
	MAME_DIR .. "src/mess/machine/m20_8086.c",
	MAME_DIR .. "src/mess/drivers/m24.c",
	MAME_DIR .. "src/mess/machine/m24_kbd.c",
	MAME_DIR .. "src/mess/machine/m24_z8000.c"
}

createMESSProjects(_target, _subtarget, "olympia")
files {
	MAME_DIR .. "src/mess/drivers/peoplepc.c"
}

createMESSProjects(_target, _subtarget, "ns")
files {
	MAME_DIR .. "src/mess/drivers/hh_cop400.c",
}

createMESSProjects(_target, _subtarget, "omnibyte")
files {
	MAME_DIR .. "src/mess/drivers/msbc1.c",
	MAME_DIR .. "src/mess/drivers/ob68k1a.c",
}

createMESSProjects(_target, _subtarget, "orion")
files {
	MAME_DIR .. "src/mess/drivers/orion.c",
	MAME_DIR .. "src/mess/machine/orion.c",
	MAME_DIR .. "src/mess/video/orion.c",
}

createMESSProjects(_target, _subtarget, "osborne")
files {
	MAME_DIR .. "src/mess/drivers/osborne1.c",
	MAME_DIR .. "src/mess/machine/osborne1.c",
	MAME_DIR .. "src/mess/drivers/osbexec.c",
	MAME_DIR .. "src/mess/drivers/vixen.c",
}

createMESSProjects(_target, _subtarget, "osi")
files {
	MAME_DIR .. "src/mess/drivers/osi.c",
	MAME_DIR .. "src/mess/video/osi.c",
}

createMESSProjects(_target, _subtarget, "palm")
files {
	MAME_DIR .. "src/mess/drivers/palm.c",
	MAME_DIR .. "src/mess/drivers/palmz22.c",
}

createMESSProjects(_target, _subtarget, "parker")
files {
	MAME_DIR .. "src/mess/drivers/wildfire.c",
}

createMESSProjects(_target, _subtarget, "pitronic")
files {
	MAME_DIR .. "src/mess/drivers/beta.c",
}

createMESSProjects(_target, _subtarget, "pc")
files {
	MAME_DIR .. "src/mess/drivers/asst128.c",
	MAME_DIR .. "src/mess/drivers/europc.c",
	MAME_DIR .. "src/mess/drivers/genpc.c",
	MAME_DIR .. "src/mess/machine/genpc.c",
	MAME_DIR .. "src/mess/drivers/ibmpc.c",
	MAME_DIR .. "src/mess/drivers/ibmpcjr.c",
	MAME_DIR .. "src/mess/drivers/pc.c",
	MAME_DIR .. "src/mess/drivers/tandy1t.c",
	MAME_DIR .. "src/mess/video/pc_t1t.c",
}

createMESSProjects(_target, _subtarget, "pdp1")
files {
	MAME_DIR .. "src/mess/drivers/pdp1.c",
	MAME_DIR .. "src/mess/video/pdp1.c",
}

createMESSProjects(_target, _subtarget, "pel")
files {
	MAME_DIR .. "src/mess/drivers/galeb.c",
	MAME_DIR .. "src/mess/video/galeb.c",
		MAME_DIR .. "src/mess/drivers/orao.c",
	MAME_DIR .. "src/mess/machine/orao.c",
	MAME_DIR .. "src/mess/video/orao.c",
}

createMESSProjects(_target, _subtarget, "philips")
files {
	MAME_DIR .. "src/mess/drivers/p2000t.c",
	MAME_DIR .. "src/mess/machine/p2000t.c",
	MAME_DIR .. "src/mess/video/p2000m.c",
	MAME_DIR .. "src/mess/drivers/vg5k.c",
}

createMESSProjects(_target, _subtarget, "poly88")
files {
	MAME_DIR .. "src/mess/drivers/poly88.c",
	MAME_DIR .. "src/mess/machine/poly88.c",
	MAME_DIR .. "src/mess/video/poly88.c",
}

createMESSProjects(_target, _subtarget, "psion")
files {
	MAME_DIR .. "src/mess/drivers/psion.c",
	MAME_DIR .. "src/mess/machine/psion_pack.c",
}

createMESSProjects(_target, _subtarget, "radio")
files {
	MAME_DIR .. "src/mess/drivers/apogee.c",
	MAME_DIR .. "src/mess/drivers/mikrosha.c",
	MAME_DIR .. "src/mess/drivers/partner.c",
	MAME_DIR .. "src/mess/machine/partner.c",
	MAME_DIR .. "src/mess/drivers/radio86.c",
	MAME_DIR .. "src/mess/machine/radio86.c",
}

createMESSProjects(_target, _subtarget, "rca")
files {
	MAME_DIR .. "src/mess/drivers/microkit.c",
	MAME_DIR .. "src/mess/drivers/studio2.c",
	MAME_DIR .. "src/mess/drivers/vip.c",
}

createMESSProjects(_target, _subtarget, "regnecentralen")
files {
	MAME_DIR .. "src/mess/drivers/rc759.c",
}

createMESSProjects(_target, _subtarget, "ritam")
files {
	MAME_DIR .. "src/mess/drivers/monty.c",
}

createMESSProjects(_target, _subtarget, "rm")
files {
	MAME_DIR .. "src/mess/drivers/rm380z.c",
	MAME_DIR .. "src/mess/machine/rm380z.c",
	MAME_DIR .. "src/mess/video/rm380z.c",
	MAME_DIR .. "src/mess/drivers/rmnimbus.c",
	MAME_DIR .. "src/mess/machine/rmnimbus.c",
	MAME_DIR .. "src/mess/video/rmnimbus.c",
	MAME_DIR .. "src/mess/machine/rmnkbd.c",
}

createMESSProjects(_target, _subtarget, "robotron")
files {
	MAME_DIR .. "src/mess/drivers/a5105.c",
	MAME_DIR .. "src/mess/drivers/a51xx.c",
	MAME_DIR .. "src/mess/drivers/a7150.c",
	MAME_DIR .. "src/mess/drivers/k1003.c",
	MAME_DIR .. "src/mess/drivers/k8915.c",
	MAME_DIR .. "src/mess/drivers/rt1715.c",
	MAME_DIR .. "src/mess/drivers/z1013.c",
	MAME_DIR .. "src/mess/drivers/z9001.c",
}

createMESSProjects(_target, _subtarget, "roland")
files {
	MAME_DIR .. "src/mess/drivers/rmt32.c",
	MAME_DIR .. "src/mess/drivers/rd110.c",
	MAME_DIR .. "src/mess/drivers/rsc55.c",
	MAME_DIR .. "src/mess/drivers/tb303.c",
}

createMESSProjects(_target, _subtarget, "rockwell")
files {
	MAME_DIR .. "src/mess/drivers/aim65.c",
	MAME_DIR .. "src/mess/machine/aim65.c",
	MAME_DIR .. "src/mess/drivers/aim65_40.c",
}

createMESSProjects(_target, _subtarget, "sage")
files {
	MAME_DIR .. "src/mess/drivers/sage2.c",
}

createMESSProjects(_target, _subtarget, "samcoupe")
files {
	MAME_DIR .. "src/mess/drivers/samcoupe.c",
	MAME_DIR .. "src/mess/machine/samcoupe.c",
	MAME_DIR .. "src/mess/video/samcoupe.c",
}

createMESSProjects(_target, _subtarget, "samsung")
files {
	MAME_DIR .. "src/mess/drivers/spc1000.c",
}

createMESSProjects(_target, _subtarget, "sanyo")
files {
	MAME_DIR .. "src/mess/drivers/mbc200.c",
	MAME_DIR .. "src/mess/drivers/mbc55x.c",
	MAME_DIR .. "src/mess/machine/mbc55x.c",
	MAME_DIR .. "src/mess/video/mbc55x.c",
	MAME_DIR .. "src/mess/drivers/phc25.c",
}

createMESSProjects(_target, _subtarget, "sega")
files {
	MAME_DIR .. "src/mess/drivers/dccons.c",
	MAME_DIR .. "src/mess/machine/dccons.c",
	MAME_DIR .. "src/mess/drivers/megadriv.c",
	MAME_DIR .. "src/mess/drivers/saturn.c",
	MAME_DIR .. "src/mess/drivers/segapico.c",
	MAME_DIR .. "src/mess/drivers/segapm.c",
	MAME_DIR .. "src/mess/drivers/sg1000.c",
	MAME_DIR .. "src/mess/drivers/sms.c",
	MAME_DIR .. "src/mess/machine/sms.c",
	MAME_DIR .. "src/mess/drivers/svmu.c",
	MAME_DIR .. "src/mess/machine/mega32x.c",
	MAME_DIR .. "src/mess/machine/megacd.c",
	MAME_DIR .. "src/mess/machine/megacdcd.c",
}

createMESSProjects(_target, _subtarget, "sgi")
files {
	MAME_DIR .. "src/mess/machine/sgi.c",
	MAME_DIR .. "src/mess/drivers/sgi_ip2.c",
	MAME_DIR .. "src/mess/drivers/sgi_ip6.c",
	MAME_DIR .. "src/mess/drivers/ip20.c",
	MAME_DIR .. "src/mess/drivers/ip22.c",
	MAME_DIR .. "src/mess/video/newport.c",
}

createMESSProjects(_target, _subtarget, "sharp")
files {
	MAME_DIR .. "src/mess/drivers/hh_sm510.c",
	MAME_DIR .. "src/mess/video/mz700.c",
	MAME_DIR .. "src/mess/drivers/mz700.c",
	MAME_DIR .. "src/mess/drivers/pc1500.c",
	MAME_DIR .. "src/mess/drivers/pocketc.c",
	MAME_DIR .. "src/mess/video/pc1401.c",
	MAME_DIR .. "src/mess/machine/pc1401.c",
	MAME_DIR .. "src/mess/video/pc1403.c",
	MAME_DIR .. "src/mess/machine/pc1403.c",
	MAME_DIR .. "src/mess/video/pc1350.c",
	MAME_DIR .. "src/mess/machine/pc1350.c",
	MAME_DIR .. "src/mess/video/pc1251.c",
	MAME_DIR .. "src/mess/machine/pc1251.c",
	MAME_DIR .. "src/mess/video/pocketc.c",
	MAME_DIR .. "src/mess/machine/mz700.c",
	MAME_DIR .. "src/mess/drivers/x68k.c",
	MAME_DIR .. "src/mess/video/x68k.c",
	MAME_DIR .. "src/mess/machine/x68k_hdc.c",
	MAME_DIR .. "src/mess/machine/x68k_kbd.c",
	MAME_DIR .. "src/mess/drivers/mz80.c",
	MAME_DIR .. "src/mess/video/mz80.c",
	MAME_DIR .. "src/mess/machine/mz80.c",
	MAME_DIR .. "src/mess/drivers/mz2000.c",
	MAME_DIR .. "src/mess/drivers/x1.c",
	MAME_DIR .. "src/mess/machine/x1.c",
	MAME_DIR .. "src/mess/drivers/x1twin.c",
	MAME_DIR .. "src/mess/drivers/mz2500.c",
	MAME_DIR .. "src/mess/drivers/mz3500.c",
	MAME_DIR .. "src/mess/drivers/pce220.c",
	MAME_DIR .. "src/mess/machine/pce220_ser.c",
	MAME_DIR .. "src/mess/drivers/mz6500.c",
	MAME_DIR .. "src/mess/drivers/zaurus.c",
}

createMESSProjects(_target, _subtarget, "sinclair")
files {
	MAME_DIR .. "src/mess/video/spectrum.c",
	MAME_DIR .. "src/mess/video/timex.c",
	MAME_DIR .. "src/mess/video/zx.c",
	MAME_DIR .. "src/mess/drivers/zx.c",
	MAME_DIR .. "src/mess/machine/zx.c",
	MAME_DIR .. "src/mess/drivers/spectrum.c",
	MAME_DIR .. "src/mess/drivers/spec128.c",
	MAME_DIR .. "src/mess/drivers/timex.c",
	MAME_DIR .. "src/mess/drivers/specpls3.c",
	MAME_DIR .. "src/mess/drivers/scorpion.c",
	MAME_DIR .. "src/mess/drivers/atm.c",
	MAME_DIR .. "src/mess/drivers/pentagon.c",
	MAME_DIR .. "src/mess/machine/beta.c",
	MAME_DIR .. "src/mess/machine/spec_snqk.c",
	MAME_DIR .. "src/mess/drivers/ql.c",
	MAME_DIR .. "src/mess/machine/qimi.c",
	MAME_DIR .. "src/mess/video/zx8301.c",
	MAME_DIR .. "src/mess/machine/zx8302.c",
}

createMESSProjects(_target, _subtarget, "siemens")
files {
	MAME_DIR .. "src/mess/drivers/pcd.c",
	MAME_DIR .. "src/mess/machine/pcd_kbd.c",
	MAME_DIR .. "src/mess/video/pcd.c",
}

createMESSProjects(_target, _subtarget, "slicer")
files {
	MAME_DIR .. "src/mess/drivers/slicer.c",
}

createMESSProjects(_target, _subtarget, "snk")
files {
	MAME_DIR .. "src/mess/drivers/ng_aes.c",
	MAME_DIR .. "src/mess/drivers/ngp.c",
	MAME_DIR .. "src/mess/video/k1ge.c",
}

createMESSProjects(_target, _subtarget, "sony")
files {
	MAME_DIR .. "src/mess/drivers/pockstat.c",
	MAME_DIR .. "src/mess/drivers/psx.c",
	MAME_DIR .. "src/mess/machine/psxcd.c",
	MAME_DIR .. "src/mess/drivers/pve500.c",
	MAME_DIR .. "src/mess/drivers/smc777.c",
}

createMESSProjects(_target, _subtarget, "sord")
files {
	MAME_DIR .. "src/mess/drivers/m5.c",
}

createMESSProjects(_target, _subtarget, "special")
files {
	MAME_DIR .. "src/mess/drivers/special.c",
	MAME_DIR .. "src/mess/audio/specimx_snd.c",
	MAME_DIR .. "src/mess/machine/special.c",
	MAME_DIR .. "src/mess/video/special.c",
}

createMESSProjects(_target, _subtarget, "sun")
files {
	MAME_DIR .. "src/mess/drivers/sun1.c",
	MAME_DIR .. "src/mess/drivers/sun2.c",
	MAME_DIR .. "src/mess/drivers/sun3.c",
	MAME_DIR .. "src/mess/drivers/sun4.c",
}

createMESSProjects(_target, _subtarget, "svi")
files {
	MAME_DIR .. "src/mess/drivers/svi318.c",
	MAME_DIR .. "src/mess/machine/svi318.c",
}

createMESSProjects(_target, _subtarget, "svision")
files {
	MAME_DIR .. "src/mess/drivers/svision.c",
	MAME_DIR .. "src/mess/audio/svis_snd.c",
}

createMESSProjects(_target, _subtarget, "swtpc09")
files {
	MAME_DIR .. "src/mess/drivers/swtpc09.c",
	MAME_DIR .. "src/mess/machine/swtpc09.c",
}

createMESSProjects(_target, _subtarget, "synertec")
files {
	MAME_DIR .. "src/mess/drivers/sym1.c",
}

createMESSProjects(_target, _subtarget, "ta")
files {
	MAME_DIR .. "src/mess/drivers/alphatro.c",
}

createMESSProjects(_target, _subtarget, "tandberg")
files {
	MAME_DIR .. "src/mess/drivers/tdv2324.c",
}

createMESSProjects(_target, _subtarget, "tangerin")
files {
	MAME_DIR .. "src/mess/drivers/microtan.c",
	MAME_DIR .. "src/mess/machine/microtan.c",
	MAME_DIR .. "src/mess/video/microtan.c",
	MAME_DIR .. "src/mess/drivers/oric.c",
}

createMESSProjects(_target, _subtarget, "tatung")
files {
	MAME_DIR .. "src/mess/drivers/einstein.c",
	MAME_DIR .. "src/mess/machine/einstein.c",
}

createMESSProjects(_target, _subtarget, "teamconc")
files {
	MAME_DIR .. "src/mess/drivers/comquest.c",
	MAME_DIR .. "src/mess/video/comquest.c",
}

createMESSProjects(_target, _subtarget, "tektroni")
files {
	MAME_DIR .. "src/mess/drivers/tek405x.c",
	MAME_DIR .. "src/mess/drivers/tek410x.c",
}

createMESSProjects(_target, _subtarget, "telenova")
files {
	MAME_DIR .. "src/mess/drivers/compis.c",
	MAME_DIR .. "src/mess/machine/compiskb.c",
}

createMESSProjects(_target, _subtarget, "telercas")
files {
	MAME_DIR .. "src/mess/drivers/tmc1800.c",
	MAME_DIR .. "src/mess/video/tmc1800.c",
	MAME_DIR .. "src/mess/drivers/tmc600.c",
	MAME_DIR .. "src/mess/video/tmc600.c",
	MAME_DIR .. "src/mess/drivers/tmc2000e.c",
}

createMESSProjects(_target, _subtarget, "televideo")
files {
	MAME_DIR .. "src/mess/drivers/ts802.c",
	MAME_DIR .. "src/mess/drivers/ts803.c",
	MAME_DIR .. "src/mess/drivers/ts816.c",
	MAME_DIR .. "src/mess/drivers/tv950.c",
}

createMESSProjects(_target, _subtarget, "tem")
files {
	MAME_DIR .. "src/mess/drivers/tec1.c",
}

createMESSProjects(_target, _subtarget, "tesla")
files {
	MAME_DIR .. "src/mess/drivers/ondra.c",
	MAME_DIR .. "src/mess/machine/ondra.c",
	MAME_DIR .. "src/mess/video/ondra.c",
	MAME_DIR .. "src/mess/drivers/pmd85.c",
	MAME_DIR .. "src/mess/machine/pmd85.c",
	MAME_DIR .. "src/mess/drivers/pmi80.c",
	MAME_DIR .. "src/mess/drivers/sapi1.c",
}

createMESSProjects(_target, _subtarget, "test")
files {
	MAME_DIR .. "src/mess/drivers/test_t400.c",
	MAME_DIR .. "src/mess/drivers/zexall.c",
}

createMESSProjects(_target, _subtarget, "thomson")
files {
	MAME_DIR .. "src/mess/drivers/thomson.c",
	MAME_DIR .. "src/mess/machine/thomson.c",
	MAME_DIR .. "src/mess/machine/thomflop.c",
	MAME_DIR .. "src/mess/video/thomson.c",
}

createMESSProjects(_target, _subtarget, "ti")
files {
	MAME_DIR .. "src/mess/drivers/avigo.c",
	MAME_DIR .. "src/mess/video/avigo.c",
	MAME_DIR .. "src/mess/drivers/cc40.c",
	MAME_DIR .. "src/mess/drivers/evmbug.c",
	MAME_DIR .. "src/mess/drivers/exelv.c",
	MAME_DIR .. "src/mess/drivers/geneve.c",
	MAME_DIR .. "src/mess/drivers/ticalc1x.c",
	MAME_DIR .. "src/mess/drivers/tispeak.c",
	MAME_DIR .. "src/mess/drivers/ti74.c",
	MAME_DIR .. "src/mess/drivers/ti85.c",
	MAME_DIR .. "src/mess/machine/ti85.c",
	MAME_DIR .. "src/mess/video/ti85.c",
	MAME_DIR .. "src/mess/drivers/ti89.c",
	MAME_DIR .. "src/mess/drivers/ti99_2.c",
	MAME_DIR .. "src/mess/drivers/ti99_4x.c",
	MAME_DIR .. "src/mess/drivers/ti99_4p.c",
	MAME_DIR .. "src/mess/drivers/ti99_8.c",
	MAME_DIR .. "src/mess/drivers/ti990_4.c",
	MAME_DIR .. "src/mess/drivers/ti990_10.c",
	MAME_DIR .. "src/mess/drivers/tm990189.c",
	MAME_DIR .. "src/mess/video/733_asr.c",
	MAME_DIR .. "src/mess/video/911_vdt.c",
	MAME_DIR .. "src/mess/drivers/hh_tms1k.c",
}

createMESSProjects(_target, _subtarget, "tiger")
files {
	MAME_DIR .. "src/mess/drivers/gamecom.c",
	MAME_DIR .. "src/mess/machine/gamecom.c",
	MAME_DIR .. "src/mess/video/gamecom.c",
}

createMESSProjects(_target, _subtarget, "tigertel")
files {
	MAME_DIR .. "src/mess/drivers/gizmondo.c",
	MAME_DIR .. "src/mess/machine/docg3.c",
}

createMESSProjects(_target, _subtarget, "tiki")
files {
	MAME_DIR .. "src/mess/drivers/tiki100.c",
}

createMESSProjects(_target, _subtarget, "tomy")
files {
	MAME_DIR .. "src/mess/drivers/tutor.c",
}

createMESSProjects(_target, _subtarget, "toshiba")
files {
	MAME_DIR .. "src/mess/drivers/pasopia.c",
	MAME_DIR .. "src/mess/drivers/pasopia7.c",
	MAME_DIR .. "src/mess/drivers/paso1600.c",
}

createMESSProjects(_target, _subtarget, "trainer")
files {
	MAME_DIR .. "src/mess/drivers/amico2k.c",
	MAME_DIR .. "src/mess/drivers/babbage.c",
	MAME_DIR .. "src/mess/drivers/bob85.c",
	MAME_DIR .. "src/mess/drivers/cvicny.c",
	MAME_DIR .. "src/mess/drivers/dolphunk.c",
	MAME_DIR .. "src/mess/drivers/instruct.c",
	MAME_DIR .. "src/mess/drivers/mk14.c",
	MAME_DIR .. "src/mess/drivers/pro80.c",
	MAME_DIR .. "src/mess/drivers/savia84.c",
	MAME_DIR .. "src/mess/drivers/selz80.c",
	MAME_DIR .. "src/mess/drivers/tk80.c",
}

createMESSProjects(_target, _subtarget, "trs")
files {
	MAME_DIR .. "src/mess/drivers/coco12.c",
	MAME_DIR .. "src/mess/drivers/coco3.c",
	MAME_DIR .. "src/mess/drivers/dragon.c",
	MAME_DIR .. "src/mess/drivers/mc10.c",
	MAME_DIR .. "src/mess/machine/6883sam.c",
	MAME_DIR .. "src/mess/machine/coco.c",
	MAME_DIR .. "src/mess/machine/coco12.c",
	MAME_DIR .. "src/mess/machine/coco3.c",
	MAME_DIR .. "src/mess/machine/coco_vhd.c",
	MAME_DIR .. "src/mess/machine/dragon.c",
	MAME_DIR .. "src/mess/machine/dgnalpha.c",
	MAME_DIR .. "src/mess/video/gime.c",
	MAME_DIR .. "src/mess/drivers/trs80.c",
	MAME_DIR .. "src/mess/machine/trs80.c",
	MAME_DIR .. "src/mess/video/trs80.c",
	MAME_DIR .. "src/mess/drivers/trs80m2.c",
	MAME_DIR .. "src/mess/machine/trs80m2kb.c",
	MAME_DIR .. "src/mess/drivers/tandy2k.c",
	MAME_DIR .. "src/mess/machine/tandy2kb.c",
}

createMESSProjects(_target, _subtarget, "ultimachine")
files {
	MAME_DIR .. "src/mess/drivers/rambo.c",
}

createMESSProjects(_target, _subtarget, "ultratec")
files {
	MAME_DIR .. "src/mess/drivers/minicom.c",
}

createMESSProjects(_target, _subtarget, "unisys")
files {
	MAME_DIR .. "src/mess/drivers/univac.c",
}

createMESSProjects(_target, _subtarget, "veb")
files {
	MAME_DIR .. "src/mess/drivers/chessmst.c",
	MAME_DIR .. "src/mess/drivers/kc.c",
	MAME_DIR .. "src/mess/machine/kc.c",
	MAME_DIR .. "src/mess/machine/kc_keyb.c",
	MAME_DIR .. "src/mess/video/kc.c",
	MAME_DIR .. "src/mess/drivers/lc80.c",
	MAME_DIR .. "src/mess/drivers/mc80.c",
	MAME_DIR .. "src/mess/machine/mc80.c",
	MAME_DIR .. "src/mess/video/mc80.c",
	MAME_DIR .. "src/mess/drivers/poly880.c",
	MAME_DIR .. "src/mess/drivers/sc1.c",
	MAME_DIR .. "src/mess/drivers/sc2.c",
}

createMESSProjects(_target, _subtarget, "vidbrain")
files {
	MAME_DIR .. "src/mess/drivers/vidbrain.c",
	MAME_DIR .. "src/mess/video/uv201.c",
}

createMESSProjects(_target, _subtarget, "videoton")
files {
	MAME_DIR .. "src/mess/drivers/tvc.c",
	MAME_DIR .. "src/mess/audio/tvc_snd.c",
}

createMESSProjects(_target, _subtarget, "visual")
files {
	MAME_DIR .. "src/mess/drivers/v1050.c",
	MAME_DIR .. "src/mess/machine/v1050kb.c",
	MAME_DIR .. "src/mess/video/v1050.c",
}

createMESSProjects(_target, _subtarget, "votrax")
files {
	MAME_DIR .. "src/mess/drivers/votrpss.c",
	MAME_DIR .. "src/mess/drivers/votrtnt.c",
}

createMESSProjects(_target, _subtarget, "vtech")
files {
	MAME_DIR .. "src/mess/drivers/crvision.c",
	MAME_DIR .. "src/mess/drivers/geniusiq.c",
	MAME_DIR .. "src/mess/drivers/laser3k.c",
	MAME_DIR .. "src/mess/drivers/lcmate2.c",
	MAME_DIR .. "src/mess/drivers/pc4.c",
	MAME_DIR .. "src/mess/video/pc4.c",
	MAME_DIR .. "src/mess/drivers/pc2000.c",
	MAME_DIR .. "src/mess/drivers/pitagjr.c",
	MAME_DIR .. "src/mess/drivers/prestige.c",
	MAME_DIR .. "src/mess/drivers/vtech1.c",
	MAME_DIR .. "src/mess/drivers/vtech2.c",
	MAME_DIR .. "src/mess/machine/vtech2.c",
	MAME_DIR .. "src/mess/video/vtech2.c",
	MAME_DIR .. "src/mess/drivers/socrates.c",
	MAME_DIR .. "src/mess/audio/socrates.c",
}

createMESSProjects(_target, _subtarget, "wang")
files {
	MAME_DIR .. "src/mess/drivers/wangpc.c",
	MAME_DIR .. "src/mess/machine/wangpckb.c",
}

createMESSProjects(_target, _subtarget, "wavemate")
files {
	MAME_DIR .. "src/mess/drivers/bullet.c",
	MAME_DIR .. "src/mess/drivers/jupiter.c",
}

createMESSProjects(_target, _subtarget, "xerox")
files {
	MAME_DIR .. "src/mess/drivers/xerox820.c",
	MAME_DIR .. "src/mess/machine/x820kb.c",
	MAME_DIR .. "src/mess/drivers/bigbord2.c",
	MAME_DIR .. "src/mess/drivers/alto2.c",
}

createMESSProjects(_target, _subtarget, "xussrpc")
files {
	MAME_DIR .. "src/mess/drivers/ec184x.c",
	MAME_DIR .. "src/mess/drivers/iskr103x.c",
	MAME_DIR .. "src/mess/drivers/mc1502.c",
	MAME_DIR .. "src/mess/drivers/poisk1.c",
	MAME_DIR .. "src/mess/video/poisk1.c",
}

createMESSProjects(_target, _subtarget, "yamaha")
files {
	MAME_DIR .. "src/mess/drivers/ymmu100.c",
	MAME_DIR .. "src/mess/drivers/fb01.c",
}
dependency {
	{ MAME_DIR .. "src/mess/drivers/ymmu100.c",    GEN_DIR .. "mess/drivers/ymmu100.inc" },
}
custombuildtask {
	{ MAME_DIR .. "src/mess/drivers/ymmu100.ppm", GEN_DIR .. "mess/drivers/ymmu100.inc",  {  MAME_DIR .. "src/build/file2str.py" }, {"@echo Converting src/drivers/ymmu100.ppm...", PYTHON .. " $(1) $(<) $(@) ymmu100_bkg UINT8" }},
}

createMESSProjects(_target, _subtarget, "zenith")
files {
	MAME_DIR .. "src/mess/drivers/z100.c",
}

createMESSProjects(_target, _subtarget, "zpa")
files {
	MAME_DIR .. "src/mess/drivers/iq151.c",
}

createMESSProjects(_target, _subtarget, "zvt")
files {
	MAME_DIR .. "src/mess/drivers/pp01.c",
	MAME_DIR .. "src/mess/machine/pp01.c",
	MAME_DIR .. "src/mess/video/pp01.c",
}

createMESSProjects(_target, _subtarget, "skeleton")
files {
	MAME_DIR .. "src/mess/drivers/alphasma.c",
	MAME_DIR .. "src/mess/drivers/ampro.c",
	MAME_DIR .. "src/mess/drivers/amust.c",
	MAME_DIR .. "src/mess/drivers/applix.c",
	MAME_DIR .. "src/mess/drivers/attache.c",
	MAME_DIR .. "src/mess/drivers/aussiebyte.c",
	MAME_DIR .. "src/mess/video/aussiebyte.c",
	MAME_DIR .. "src/mess/drivers/ax20.c",
	MAME_DIR .. "src/mess/drivers/beehive.c",
	MAME_DIR .. "src/mess/drivers/binbug.c",
	MAME_DIR .. "src/mess/drivers/besta.c",
	MAME_DIR .. "src/mess/drivers/bitgraph.c",
	MAME_DIR .. "src/mess/drivers/br8641.c",
	MAME_DIR .. "src/mess/drivers/busicom.c",
	MAME_DIR .. "src/mess/video/busicom.c",
	MAME_DIR .. "src/mess/drivers/chaos.c",
	MAME_DIR .. "src/mess/drivers/chesstrv.c",
	MAME_DIR .. "src/mess/drivers/cd2650.c",
	MAME_DIR .. "src/mess/drivers/cdc721.c",
	MAME_DIR .. "src/mess/drivers/codata.c",
	MAME_DIR .. "src/mess/drivers/cortex.c",
	MAME_DIR .. "src/mess/drivers/cosmicos.c",
	MAME_DIR .. "src/mess/drivers/cp1.c",
	MAME_DIR .. "src/mess/drivers/cxhumax.c",
	MAME_DIR .. "src/mess/drivers/czk80.c",
	MAME_DIR .. "src/mess/drivers/d6800.c",
	MAME_DIR .. "src/mess/drivers/d6809.c",
	MAME_DIR .. "src/mess/drivers/daruma.c",
	MAME_DIR .. "src/mess/drivers/digel804.c",
	MAME_DIR .. "src/mess/drivers/dim68k.c",
	MAME_DIR .. "src/mess/drivers/dm7000.c",
	MAME_DIR .. "src/mess/drivers/dmv.c",
	MAME_DIR .. "src/mess/machine/dmv_keyb.c",
	MAME_DIR .. "src/mess/drivers/dps1.c",
	MAME_DIR .. "src/mess/drivers/dsb46.c",
	MAME_DIR .. "src/mess/drivers/dual68.c",
	MAME_DIR .. "src/mess/drivers/eacc.c",
	MAME_DIR .. "src/mess/drivers/elwro800.c",
	MAME_DIR .. "src/mess/drivers/eti660.c",
	MAME_DIR .. "src/mess/drivers/excali64.c",
	MAME_DIR .. "src/mess/drivers/fanucs15.c",
	MAME_DIR .. "src/mess/drivers/fanucspmg.c",
	MAME_DIR .. "src/mess/drivers/fc100.c",
	MAME_DIR .. "src/mess/drivers/fk1.c",
	MAME_DIR .. "src/mess/drivers/ft68m.c",
	MAME_DIR .. "src/mess/drivers/gamate.c",
	MAME_DIR .. "src/mess/audio/gamate.c",
	MAME_DIR .. "src/mess/drivers/gameking.c",
	MAME_DIR .. "src/mess/drivers/gimix.c",
	MAME_DIR .. "src/mess/drivers/grfd2301.c",
	MAME_DIR .. "src/mess/drivers/harriet.c",
	MAME_DIR .. "src/mess/drivers/hprot1.c",
	MAME_DIR .. "src/mess/drivers/hpz80unk.c",
	MAME_DIR .. "src/mess/drivers/ht68k.c",
	MAME_DIR .. "src/mess/drivers/hunter2.c",
	MAME_DIR .. "src/mess/drivers/i7000.c",
	MAME_DIR .. "src/mess/drivers/ibm6580.c",
	MAME_DIR .. "src/mess/drivers/icatel.c",
	MAME_DIR .. "src/mess/drivers/ie15.c",
	MAME_DIR .. "src/mess/machine/ie15_kbd.c",
	MAME_DIR .. "src/mess/drivers/if800.c",
	MAME_DIR .. "src/mess/drivers/imsai.c",
	MAME_DIR .. "src/mess/drivers/indiana.c",
	MAME_DIR .. "src/mess/drivers/itt3030.c",
	MAME_DIR .. "src/mess/drivers/jade.c",
	MAME_DIR .. "src/mess/drivers/jonos.c",
	MAME_DIR .. "src/mess/drivers/konin.c",
	MAME_DIR .. "src/mess/drivers/leapster.c",
	MAME_DIR .. "src/mess/drivers/lft.c",
	MAME_DIR .. "src/mess/drivers/lg-dvd.c",
	MAME_DIR .. "src/mess/drivers/lola8a.c",
	MAME_DIR .. "src/mess/drivers/m79152pc.c",
	MAME_DIR .. "src/mess/drivers/mccpm.c",
	MAME_DIR .. "src/mess/drivers/mes.c",
	MAME_DIR .. "src/mess/drivers/mice.c",
	MAME_DIR .. "src/mess/drivers/micronic.c",
	MAME_DIR .. "src/mess/drivers/mini2440.c",
	MAME_DIR .. "src/mess/drivers/mmd1.c",
	MAME_DIR .. "src/mess/drivers/mod8.c",
	MAME_DIR .. "src/mess/drivers/modellot.c",
	MAME_DIR .. "src/mess/drivers/molecular.c",
	MAME_DIR .. "src/mess/drivers/ms0515.c",
	MAME_DIR .. "src/mess/drivers/ms9540.c",
	MAME_DIR .. "src/mess/drivers/mstation.c",
	MAME_DIR .. "src/mess/drivers/mt735.c",
	MAME_DIR .. "src/mess/drivers/mx2178.c",
	MAME_DIR .. "src/mess/drivers/mycom.c",
	MAME_DIR .. "src/mess/drivers/myvision.c",
	MAME_DIR .. "src/mess/drivers/ngen.c",
	MAME_DIR .. "src/mess/machine/ngen_kb.c",
	MAME_DIR .. "src/mess/drivers/octopus.c",
	MAME_DIR .. "src/mess/drivers/onyx.c",
	MAME_DIR .. "src/mess/drivers/okean240.c",
	MAME_DIR .. "src/mess/drivers/p8k.c",
	MAME_DIR .. "src/mess/drivers/pegasus.c",
	MAME_DIR .. "src/mess/drivers/pencil2.c",
	MAME_DIR .. "src/mess/drivers/pes.c",
	MAME_DIR .. "src/mess/drivers/pipbug.c",
	MAME_DIR .. "src/mess/drivers/plan80.c",
	MAME_DIR .. "src/mess/drivers/pm68k.c",
	MAME_DIR .. "src/mess/drivers/poly.c",
	MAME_DIR .. "src/mess/drivers/pt68k4.c",
	MAME_DIR .. "src/mess/drivers/ptcsol.c",
	MAME_DIR .. "src/mess/drivers/pulsar.c",
	MAME_DIR .. "src/mess/drivers/pv9234.c",
	MAME_DIR .. "src/mess/drivers/qtsbc.c",
	MAME_DIR .. "src/mess/drivers/rvoice.c",
	MAME_DIR .. "src/mess/drivers/sacstate.c",
	MAME_DIR .. "src/mess/drivers/sbrain.c",
	MAME_DIR .. "src/mess/drivers/seattle.c",
	MAME_DIR .. "src/mess/drivers/sh4robot.c",
	MAME_DIR .. "src/mess/drivers/softbox.c",
	MAME_DIR .. "src/mess/drivers/swtpc.c",
	MAME_DIR .. "src/mess/drivers/sys2900.c",
	MAME_DIR .. "src/mess/drivers/systec.c",
	MAME_DIR .. "src/mess/drivers/tavernie.c",
	MAME_DIR .. "src/mess/drivers/tecnbras.c",
	MAME_DIR .. "src/mess/drivers/terak.c",
	MAME_DIR .. "src/mess/drivers/ti630.c",
	MAME_DIR .. "src/mess/drivers/tsispch.c",
	MAME_DIR .. "src/mess/drivers/tvgame.c",
	MAME_DIR .. "src/mess/drivers/unistar.c",
	MAME_DIR .. "src/mess/drivers/v6809.c",
	MAME_DIR .. "src/mess/drivers/vector4.c",
	MAME_DIR .. "src/mess/drivers/vii.c",
	MAME_DIR .. "src/mess/drivers/wicat.c",
	MAME_DIR .. "src/mess/drivers/xor100.c",
	MAME_DIR .. "src/mess/drivers/xavix.c",
	MAME_DIR .. "src/mess/drivers/zorba.c",
}

end
