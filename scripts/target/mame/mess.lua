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
CPUS["PATINHOFEIO"] = true
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
VIDEOS["EF9365"] = true
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
VIDEOS["PCD8544"] = true
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
MACHINES["PDC"] = true
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
MACHINES["VT82C496"] = true

--------------------------------------------------
-- specify available bus cores
--------------------------------------------------

BUSES["A1BUS"] = true
BUSES["A2BUS"] = true
BUSES["A7800"] = true
BUSES["A800"] = true
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
BUSES["TIKI100"] = true
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
		"rolm",
		"sage",
		"samcoupe",
		"samsung",
		"sanyo",
		"saturn",
		"sega",
		"sequential",
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
		"unisonic",
		"unisys",
		"usp",
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
	MAME_DIR .. "src/mame/machine/archimds.cpp",
	MAME_DIR .. "src/mame/video/archimds.cpp",
	MAME_DIR .. "src/mame/machine/amiga.cpp",
	MAME_DIR .. "src/mame/video/amiga.cpp",
	MAME_DIR .. "src/mame/video/amigaaga.cpp",
	MAME_DIR .. "src/mame/video/tia.cpp",
	MAME_DIR .. "src/mame/video/tia.h",
	MAME_DIR .. "src/mame/machine/atari.cpp",
	MAME_DIR .. "src/mame/video/atari.cpp",
	MAME_DIR .. "src/mame/includes/atari.h",
	MAME_DIR .. "src/mame/video/antic.cpp",
	MAME_DIR .. "src/mame/video/antic.h",
	MAME_DIR .. "src/mame/video/gtia.cpp",
	MAME_DIR .. "src/mame/video/gtia.h",
	MAME_DIR .. "src/mame/drivers/jaguar.cpp",
	MAME_DIR .. "src/mame/includes/jaguar.h",
	MAME_DIR .. "src/mame/audio/jaguar.cpp",
	MAME_DIR .. "src/mame/video/jaguar.cpp",
	MAME_DIR .. "src/mame/video/jagblit.h",
	MAME_DIR .. "src/mame/video/jagblit.inc",
	MAME_DIR .. "src/mame/video/jagobj.inc",	
	MAME_DIR .. "src/mame/audio/gorf.cpp",
	MAME_DIR .. "src/mame/audio/wow.cpp",
	MAME_DIR .. "src/mame/drivers/astrocde.cpp",
	MAME_DIR .. "src/mame/includes/astrocde.h",
	MAME_DIR .. "src/mame/video/astrocde.cpp",
	MAME_DIR .. "src/mame/machine/kabuki.cpp",
	MAME_DIR .. "src/mame/machine/kabuki.h",
	MAME_DIR .. "src/mame/video/pk8000.cpp",
	MAME_DIR .. "src/mame/video/ppu2c0x.cpp",
	MAME_DIR .. "src/mame/video/ppu2c0x.h",
	MAME_DIR .. "src/mame/machine/snes.cpp",
	MAME_DIR .. "src/mame/audio/snes_snd.cpp",
	MAME_DIR .. "src/mame/audio/snes_snd.h",
	MAME_DIR .. "src/mame/machine/n64.cpp",
	MAME_DIR .. "src/mame/video/n64.cpp",
	MAME_DIR .. "src/mame/video/n64types.h",
	MAME_DIR .. "src/mame/video/rdpfiltr.inc",	
	MAME_DIR .. "src/mame/video/n64.h",
	MAME_DIR .. "src/mame/video/rdpblend.cpp",
	MAME_DIR .. "src/mame/video/rdpblend.h",
	MAME_DIR .. "src/mame/video/rdptpipe.cpp",
	MAME_DIR .. "src/mame/video/rdptpipe.h",
	MAME_DIR .. "src/mame/machine/megadriv.cpp",
	MAME_DIR .. "src/mame/drivers/naomi.cpp",
	MAME_DIR .. "src/mame/includes/naomi.h",
	MAME_DIR .. "src/mame/includes/dc.h",
	MAME_DIR .. "src/mame/machine/awboard.cpp",
	MAME_DIR .. "src/mame/machine/awboard.h",
	MAME_DIR .. "src/mame/machine/dc.cpp",
	MAME_DIR .. "src/mame/machine/dc-ctrl.cpp",
	MAME_DIR .. "src/mame/machine/dc-ctrl.h",
	MAME_DIR .. "src/mame/machine/gdrom.cpp",
	MAME_DIR .. "src/mame/machine/gdrom.h",
	MAME_DIR .. "src/mame/machine/jvs13551.cpp",
	MAME_DIR .. "src/mame/machine/jvs13551.h",
	MAME_DIR .. "src/mame/machine/maple-dc.cpp",
	MAME_DIR .. "src/mame/machine/maple-dc.h",
	MAME_DIR .. "src/mame/machine/mapledev.cpp",
	MAME_DIR .. "src/mame/machine/mapledev.h",
	MAME_DIR .. "src/mame/machine/mie.cpp",
	MAME_DIR .. "src/mame/machine/mie.h",
	MAME_DIR .. "src/mame/machine/naomi.cpp",
	MAME_DIR .. "src/mame/machine/naomibd.cpp",
	MAME_DIR .. "src/mame/machine/naomibd.h",
	MAME_DIR .. "src/mame/machine/naomig1.cpp",
	MAME_DIR .. "src/mame/machine/naomig1.h",
	MAME_DIR .. "src/mame/machine/naomigd.cpp",
	MAME_DIR .. "src/mame/machine/naomigd.h",
	MAME_DIR .. "src/mame/machine/naomim1.cpp",
	MAME_DIR .. "src/mame/machine/naomim1.h",
	MAME_DIR .. "src/mame/machine/naomim2.cpp",
	MAME_DIR .. "src/mame/machine/naomim2.h",
	MAME_DIR .. "src/mame/machine/naomim4.cpp",
	MAME_DIR .. "src/mame/machine/naomim4.h",
	MAME_DIR .. "src/mame/machine/naomirom.cpp",
	MAME_DIR .. "src/mame/machine/naomirom.h",
	MAME_DIR .. "src/mame/machine/315-5881_crypt.cpp",
	MAME_DIR .. "src/mame/machine/315-5881_crypt.h",
	MAME_DIR .. "src/mame/video/powervr2.cpp",
	MAME_DIR .. "src/mame/video/powervr2.h",
	MAME_DIR .. "src/mame/drivers/neogeo.cpp",
	MAME_DIR .. "src/mame/includes/neogeo.h",
	MAME_DIR .. "src/mame/machine/neocrypt.cpp",
	MAME_DIR .. "src/mame/machine/ng_memcard.cpp",
	MAME_DIR .. "src/mame/machine/ng_memcard.h",
	MAME_DIR .. "src/mame/video/neogeo.cpp",
	MAME_DIR .. "src/mame/video/neogeo_spr.cpp",
	MAME_DIR .. "src/mame/video/neogeo_spr.h",
	MAME_DIR .. "src/mame/drivers/cdi.cpp",
	MAME_DIR .. "src/mame/includes/cdi.h",
	MAME_DIR .. "src/mame/machine/cdi070.cpp",
	MAME_DIR .. "src/mame/machine/cdi070.h",
	MAME_DIR .. "src/mame/machine/cdicdic.cpp",
	MAME_DIR .. "src/mame/machine/cdicdic.h",
	MAME_DIR .. "src/mame/machine/cdislave.cpp",
	MAME_DIR .. "src/mame/machine/cdislave.h",
	MAME_DIR .. "src/mame/video/mcd212.cpp",
	MAME_DIR .. "src/mame/video/mcd212.h",
	MAME_DIR .. "src/mame/drivers/3do.cpp",
	MAME_DIR .. "src/mame/includes/3do.h",
	MAME_DIR .. "src/mame/machine/3do.cpp",
	MAME_DIR .. "src/mame/drivers/konamim2.cpp",
	MAME_DIR .. "src/mame/drivers/vectrex.cpp",
	MAME_DIR .. "src/mame/includes/vectrex.h",
	MAME_DIR .. "src/mame/machine/vectrex.cpp",
	MAME_DIR .. "src/mame/video/vectrex.cpp",
	MAME_DIR .. "src/mame/drivers/cps1.cpp",
	MAME_DIR .. "src/mame/includes/cps1.h",
	MAME_DIR .. "src/mame/video/cps1.cpp",
	MAME_DIR .. "src/mame/video/chihiro.cpp",
	MAME_DIR .. "src/mame/machine/xbox.cpp",
}
end
--------------------------------------------------
-- the following files are general components and
-- shared across a number of drivers
--------------------------------------------------
createMESSProjects(_target, _subtarget, "messshared")
files {
	MAME_DIR .. "src/mame/audio/mea8000.cpp",
	MAME_DIR .. "src/mame/audio/mea8000.h",
	MAME_DIR .. "src/mame/machine/appldriv.cpp",
	MAME_DIR .. "src/mame/machine/appldriv.h",
	MAME_DIR .. "src/mame/machine/applefdc.cpp",
	MAME_DIR .. "src/mame/machine/applefdc.h",
	MAME_DIR .. "src/mame/machine/microdrv.cpp",
	MAME_DIR .. "src/mame/machine/microdrv.h",
	MAME_DIR .. "src/mame/machine/smartmed.cpp",
	MAME_DIR .. "src/mame/machine/smartmed.h",
	MAME_DIR .. "src/mame/machine/sonydriv.cpp",
	MAME_DIR .. "src/mame/machine/sonydriv.h",
	MAME_DIR .. "src/mame/machine/teleprinter.cpp",
	MAME_DIR .. "src/mame/machine/teleprinter.h",
	MAME_DIR .. "src/mame/machine/z80bin.cpp",
	MAME_DIR .. "src/mame/machine/z80bin.h",
}
--------------------------------------------------
-- manufacturer-specific groupings for drivers
--------------------------------------------------

createMESSProjects(_target, _subtarget, "acorn")
files {
	MAME_DIR .. "src/mame/drivers/a310.cpp",
	MAME_DIR .. "src/mame/drivers/a6809.cpp",
	MAME_DIR .. "src/mame/drivers/acrnsys1.cpp",
	MAME_DIR .. "src/mame/drivers/atom.cpp",
	MAME_DIR .. "src/mame/includes/atom.h",
	MAME_DIR .. "src/mame/drivers/bbc.cpp",
	MAME_DIR .. "src/mame/includes/bbc.h",
	MAME_DIR .. "src/mame/machine/bbc.cpp",
	MAME_DIR .. "src/mame/video/bbc.cpp",
	MAME_DIR .. "src/mame/drivers/bbcbc.cpp",
	MAME_DIR .. "src/mame/drivers/electron.cpp",
	MAME_DIR .. "src/mame/includes/electron.h",
	MAME_DIR .. "src/mame/machine/electron.cpp",
	MAME_DIR .. "src/mame/video/electron.cpp",
	MAME_DIR .. "src/mame/drivers/riscpc.cpp",
	MAME_DIR .. "src/mame/drivers/z88.cpp",
	MAME_DIR .. "src/mame/includes/z88.h",
	MAME_DIR .. "src/mame/machine/upd65031.cpp",
	MAME_DIR .. "src/mame/machine/upd65031.h",
	MAME_DIR .. "src/mame/video/z88.cpp",
}

createMESSProjects(_target, _subtarget, "act")
files {
	MAME_DIR .. "src/mame/drivers/apricot.cpp",
	MAME_DIR .. "src/mame/drivers/apricotf.cpp",
	MAME_DIR .. "src/mame/drivers/apricotp.cpp",
	MAME_DIR .. "src/mame/machine/apricotkb.cpp",
	MAME_DIR .. "src/mame/machine/apricotkb.h",
	MAME_DIR .. "src/mame/machine/apricotkb_hle.cpp",
	MAME_DIR .. "src/mame/machine/apricotkb_hle.h",
	MAME_DIR .. "src/mame/drivers/victor9k.cpp",
	MAME_DIR .. "src/mame/includes/victor9k.h",
	MAME_DIR .. "src/mame/machine/victor9kb.cpp",
	MAME_DIR .. "src/mame/machine/victor9kb.h",
	MAME_DIR .. "src/mame/machine/victor9k_fdc.cpp",
	MAME_DIR .. "src/mame/machine/victor9k_fdc.h",
}

createMESSProjects(_target, _subtarget, "adc")
files {
	MAME_DIR .. "src/mame/drivers/super6.cpp",
	MAME_DIR .. "src/mame/includes/super6.h",
	MAME_DIR .. "src/mame/drivers/superslave.cpp",
	MAME_DIR .. "src/mame/includes/superslave.h",
}

createMESSProjects(_target, _subtarget, "alesis")
files {
	MAME_DIR .. "src/mame/drivers/alesis.cpp",
	MAME_DIR .. "src/mame/includes/alesis.h",
	MAME_DIR .. "src/mame/audio/alesis.cpp",
	MAME_DIR .. "src/mame/video/alesis.cpp",
}

createMESSProjects(_target, _subtarget, "altos")
files {
	MAME_DIR .. "src/mame/drivers/altos5.cpp",
}

createMESSProjects(_target, _subtarget, "amiga")
files {
	MAME_DIR .. "src/mame/drivers/amiga.cpp",
	MAME_DIR .. "src/mame/includes/amiga.h",
	MAME_DIR .. "src/mame/machine/amigakbd.cpp",
	MAME_DIR .. "src/mame/machine/amigakbd.h",
}

createMESSProjects(_target, _subtarget, "amstrad")
files {
	MAME_DIR .. "src/mame/drivers/amstrad.cpp",
	MAME_DIR .. "src/mame/includes/amstrad.h",
	MAME_DIR .. "src/mame/machine/amstrad.cpp",
	MAME_DIR .. "src/mame/drivers/amstr_pc.cpp",
	MAME_DIR .. "src/mame/drivers/nc.cpp",
	MAME_DIR .. "src/mame/includes/nc.h",
	MAME_DIR .. "src/mame/machine/nc.cpp",
	MAME_DIR .. "src/mame/video/nc.cpp",
	MAME_DIR .. "src/mame/drivers/pc1512.cpp",
	MAME_DIR .. "src/mame/includes/pc1512.h",
	MAME_DIR .. "src/mame/machine/pc1512kb.cpp",
	MAME_DIR .. "src/mame/machine/pc1512kb.h",
	MAME_DIR .. "src/mame/video/pc1512.cpp",
	MAME_DIR .. "src/mame/drivers/pcw.cpp",
	MAME_DIR .. "src/mame/includes/pcw.h",
	MAME_DIR .. "src/mame/video/pcw.cpp",
	MAME_DIR .. "src/mame/drivers/pcw16.cpp",
	MAME_DIR .. "src/mame/includes/pcw16.h",
	MAME_DIR .. "src/mame/video/pcw16.cpp",
	MAME_DIR .. "src/mame/drivers/pda600.cpp",
}

createMESSProjects(_target, _subtarget, "apf")
files {
	MAME_DIR .. "src/mame/drivers/apf.cpp",
}

createMESSProjects(_target, _subtarget, "apollo")
files {
	MAME_DIR .. "src/mame/drivers/apollo.cpp",
	MAME_DIR .. "src/mame/includes/apollo.h",
	MAME_DIR .. "src/mame/machine/apollo.cpp",
	MAME_DIR .. "src/mame/machine/apollo_dbg.cpp",
	MAME_DIR .. "src/mame/machine/apollo_kbd.cpp",
	MAME_DIR .. "src/mame/machine/apollo_kbd.h",
	MAME_DIR .. "src/mame/video/apollo.cpp",
}

createMESSProjects(_target, _subtarget, "apple")
files {
	MAME_DIR .. "src/mame/drivers/apple1.cpp",
	MAME_DIR .. "src/mame/includes/apple1.h",
	MAME_DIR .. "src/mame/machine/apple1.cpp",
	MAME_DIR .. "src/mame/video/apple1.cpp",
	MAME_DIR .. "src/mame/drivers/apple2.cpp",
	MAME_DIR .. "src/mame/includes/apple2.h",
	MAME_DIR .. "src/mame/drivers/apple2e.cpp",
	MAME_DIR .. "src/mame/machine/apple2.cpp",
	MAME_DIR .. "src/mame/video/apple2.cpp",
	MAME_DIR .. "src/mame/video/apple2.h",
	MAME_DIR .. "src/mame/drivers/tk2000.cpp",
	MAME_DIR .. "src/mame/drivers/apple2gs.cpp",
	MAME_DIR .. "src/mame/includes/apple2gs.h",
	MAME_DIR .. "src/mame/machine/apple2gs.cpp",
	MAME_DIR .. "src/mame/video/apple2gs.cpp",
	MAME_DIR .. "src/mame/drivers/apple3.cpp",
	MAME_DIR .. "src/mame/includes/apple3.h",
	MAME_DIR .. "src/mame/machine/apple3.cpp",
	MAME_DIR .. "src/mame/video/apple3.cpp",
	MAME_DIR .. "src/mame/drivers/lisa.cpp",
	MAME_DIR .. "src/mame/includes/lisa.h",
	MAME_DIR .. "src/mame/machine/lisa.cpp",
	MAME_DIR .. "src/mame/drivers/mac.cpp",
	MAME_DIR .. "src/mame/includes/mac.h",
	MAME_DIR .. "src/mame/audio/mac.cpp",
	MAME_DIR .. "src/mame/machine/egret.cpp",
	MAME_DIR .. "src/mame/machine/egret.h",
	MAME_DIR .. "src/mame/machine/mac.cpp",
	MAME_DIR .. "src/mame/machine/macadb.cpp",
	MAME_DIR .. "src/mame/machine/macrtc.cpp",
	MAME_DIR .. "src/mame/machine/macrtc.h",
	MAME_DIR .. "src/mame/machine/mackbd.cpp",
	MAME_DIR .. "src/mame/machine/mackbd.h",
	MAME_DIR .. "src/mame/machine/swim.cpp",
	MAME_DIR .. "src/mame/machine/swim.h",
	MAME_DIR .. "src/mame/video/mac.cpp",
	MAME_DIR .. "src/mame/drivers/macpci.cpp",
	MAME_DIR .. "src/mame/includes/macpci.h",
	MAME_DIR .. "src/mame/machine/macpci.cpp",
	MAME_DIR .. "src/mame/machine/cuda.cpp",
	MAME_DIR .. "src/mame/machine/cuda.h",
}

createMESSProjects(_target, _subtarget, "applied")
files {
	MAME_DIR .. "src/mame/drivers/mbee.cpp",
	MAME_DIR .. "src/mame/includes/mbee.h",
	MAME_DIR .. "src/mame/machine/mbee.cpp",
	MAME_DIR .. "src/mame/video/mbee.cpp",
}

createMESSProjects(_target, _subtarget, "arcadia")
files {
	MAME_DIR .. "src/mame/drivers/arcadia.cpp",
	MAME_DIR .. "src/mame/includes/arcadia.h",
	MAME_DIR .. "src/mame/audio/arcadia.cpp",
	MAME_DIR .. "src/mame/audio/arcadia.h",
	MAME_DIR .. "src/mame/video/arcadia.cpp",
}

createMESSProjects(_target, _subtarget, "ascii")
files {
	MAME_DIR .. "src/mame/drivers/msx.cpp",
	MAME_DIR .. "src/mame/includes/msx.h",
	MAME_DIR .. "src/mame/machine/msx.cpp",
	MAME_DIR .. "src/mame/machine/msx_matsushita.cpp",
	MAME_DIR .. "src/mame/machine/msx_matsushita.h",
	MAME_DIR .. "src/mame/machine/msx_s1985.cpp",
	MAME_DIR .. "src/mame/machine/msx_s1985.h",
	MAME_DIR .. "src/mame/machine/msx_switched.cpp",
	MAME_DIR .. "src/mame/machine/msx_switched.h",
	MAME_DIR .. "src/mame/machine/msx_systemflags.cpp",
	MAME_DIR .. "src/mame/machine/msx_systemflags.h",
}

createMESSProjects(_target, _subtarget, "at")
files {
	MAME_DIR .. "src/mame/drivers/at.cpp",
	MAME_DIR .. "src/mame/includes/at.h",
	MAME_DIR .. "src/mame/machine/at.cpp",
	MAME_DIR .. "src/mame/drivers/ct486.cpp",
}

createMESSProjects(_target, _subtarget, "atari")
files {
	MAME_DIR .. "src/mame/drivers/a2600.cpp",
	MAME_DIR .. "src/mame/drivers/a7800.cpp",
	MAME_DIR .. "src/mame/video/maria.cpp",
	MAME_DIR .. "src/mame/video/maria.h",
	MAME_DIR .. "src/mame/drivers/atari400.cpp",
	MAME_DIR .. "src/mame/machine/atarifdc.cpp",
	MAME_DIR .. "src/mame/machine/atarifdc.h",
	MAME_DIR .. "src/mame/drivers/atarist.cpp",
	MAME_DIR .. "src/mame/includes/atarist.h",
	MAME_DIR .. "src/mame/video/atarist.cpp",
	MAME_DIR .. "src/mame/video/atarist.h",
	MAME_DIR .. "src/mame/drivers/lynx.cpp",
	MAME_DIR .. "src/mame/includes/lynx.h",
	MAME_DIR .. "src/mame/audio/lynx.cpp",
	MAME_DIR .. "src/mame/audio/lynx.h",
	MAME_DIR .. "src/mame/machine/lynx.cpp",
	MAME_DIR .. "src/mame/drivers/portfoli.cpp",
	MAME_DIR .. "src/mame/includes/portfoli.h",
}

createMESSProjects(_target, _subtarget, "att")
files {
	MAME_DIR .. "src/mame/drivers/unixpc.cpp",
}

createMESSProjects(_target, _subtarget, "bally")
files {
	MAME_DIR .. "src/mame/drivers/astrohome.cpp",
}

createMESSProjects(_target, _subtarget, "banctec")
files {
	MAME_DIR .. "src/mame/drivers/banctec.cpp",
	MAME_DIR .. "src/mame/includes/banctec.h",
}

createMESSProjects(_target, _subtarget, "bandai")
files {
	MAME_DIR .. "src/mame/drivers/sv8000.cpp",
	MAME_DIR .. "src/mame/drivers/rx78.cpp",
	MAME_DIR .. "src/mame/drivers/tamag1.cpp",
	MAME_DIR .. "src/mame/drivers/wswan.cpp",
	MAME_DIR .. "src/mame/includes/wswan.h",
	MAME_DIR .. "src/mame/audio/wswan_snd.cpp",
	MAME_DIR .. "src/mame/audio/wswan_snd.h",
	MAME_DIR .. "src/mame/machine/wswan.cpp",
	MAME_DIR .. "src/mame/video/wswan_video.cpp",
	MAME_DIR .. "src/mame/video/wswan_video.h",
}

createMESSProjects(_target, _subtarget, "be")
files {
	MAME_DIR .. "src/mame/drivers/bebox.cpp",
	MAME_DIR .. "src/mame/includes/bebox.h",
	MAME_DIR .. "src/mame/machine/bebox.cpp",
}

createMESSProjects(_target, _subtarget, "bnpo")
files {
	MAME_DIR .. "src/mame/drivers/b2m.cpp",
	MAME_DIR .. "src/mame/includes/b2m.h",
	MAME_DIR .. "src/mame/machine/b2m.cpp",
	MAME_DIR .. "src/mame/video/b2m.cpp",
}

createMESSProjects(_target, _subtarget, "bondwell")
files {
	MAME_DIR .. "src/mame/drivers/bw12.cpp",
	MAME_DIR .. "src/mame/includes/bw12.h",
	MAME_DIR .. "src/mame/drivers/bw2.cpp",
	MAME_DIR .. "src/mame/includes/bw2.h",
}

createMESSProjects(_target, _subtarget, "booth")
files {
	MAME_DIR .. "src/mame/drivers/apexc.cpp",
}

createMESSProjects(_target, _subtarget, "camputers")
files {
	MAME_DIR .. "src/mame/drivers/camplynx.cpp",
}

createMESSProjects(_target, _subtarget, "canon")
files {
	MAME_DIR .. "src/mame/drivers/cat.cpp",
	MAME_DIR .. "src/mame/drivers/x07.cpp",
	MAME_DIR .. "src/mame/includes/x07.h",
	MAME_DIR .. "src/mame/drivers/canon_s80.cpp",
}

createMESSProjects(_target, _subtarget, "cantab")
files {
	MAME_DIR .. "src/mame/drivers/jupace.cpp",
}

createMESSProjects(_target, _subtarget, "casio")
files {
	MAME_DIR .. "src/mame/drivers/casloopy.cpp",
	MAME_DIR .. "src/mame/drivers/cfx9850.cpp",
	MAME_DIR .. "src/mame/drivers/fp200.cpp",
	MAME_DIR .. "src/mame/drivers/fp1100.cpp",
	MAME_DIR .. "src/mame/drivers/fp6000.cpp",
	MAME_DIR .. "src/mame/drivers/pb1000.cpp",
	MAME_DIR .. "src/mame/drivers/pv1000.cpp",
	MAME_DIR .. "src/mame/drivers/pv2000.cpp",
}

createMESSProjects(_target, _subtarget, "cbm")
files {
	MAME_DIR .. "src/mame/drivers/c128.cpp",
	MAME_DIR .. "src/mame/includes/c128.h",
	MAME_DIR .. "src/mame/drivers/c64.cpp",
	MAME_DIR .. "src/mame/includes/c64.h",
	MAME_DIR .. "src/mame/drivers/c64dtv.cpp",
	MAME_DIR .. "src/mame/drivers/c65.cpp",
	MAME_DIR .. "src/mame/includes/c65.h",
	MAME_DIR .. "src/mame/drivers/c900.cpp",
	MAME_DIR .. "src/mame/drivers/cbm2.cpp",
	MAME_DIR .. "src/mame/includes/cbm2.h",
	MAME_DIR .. "src/mame/drivers/clcd.cpp",
	MAME_DIR .. "src/mame/drivers/pet.cpp",
	MAME_DIR .. "src/mame/includes/pet.h",
	MAME_DIR .. "src/mame/drivers/plus4.cpp",
	MAME_DIR .. "src/mame/includes/plus4.h",
	MAME_DIR .. "src/mame/drivers/vic10.cpp",
	MAME_DIR .. "src/mame/includes/vic10.h",
	MAME_DIR .. "src/mame/drivers/vic20.cpp",
	MAME_DIR .. "src/mame/includes/vic20.h",
	MAME_DIR .. "src/mame/machine/cbm_snqk.cpp",
	MAME_DIR .. "src/mame/machine/cbm_snqk.h",
	MAME_DIR .. "src/mame/drivers/mps1230.cpp",
}

createMESSProjects(_target, _subtarget, "cccp")
files {
	MAME_DIR .. "src/mame/drivers/argo.cpp",
	MAME_DIR .. "src/mame/drivers/cm1800.cpp",
	MAME_DIR .. "src/mame/drivers/lviv.cpp",
	MAME_DIR .. "src/mame/includes/lviv.h",
	MAME_DIR .. "src/mame/machine/lviv.cpp",
	MAME_DIR .. "src/mame/video/lviv.cpp",
	MAME_DIR .. "src/mame/drivers/mikro80.cpp",
	MAME_DIR .. "src/mame/includes/mikro80.h",
	MAME_DIR .. "src/mame/machine/mikro80.cpp",
	MAME_DIR .. "src/mame/video/mikro80.cpp",
	MAME_DIR .. "src/mame/drivers/pk8000.cpp",
	MAME_DIR .. "src/mame/includes/pk8000.h",
	MAME_DIR .. "src/mame/drivers/pk8020.cpp",
	MAME_DIR .. "src/mame/includes/pk8020.h",
	MAME_DIR .. "src/mame/machine/pk8020.cpp",
	MAME_DIR .. "src/mame/video/pk8020.cpp",
	MAME_DIR .. "src/mame/drivers/pyl601.cpp",
	MAME_DIR .. "src/mame/drivers/sm1800.cpp",
	MAME_DIR .. "src/mame/drivers/uknc.cpp",
	MAME_DIR .. "src/mame/drivers/unior.cpp",
	MAME_DIR .. "src/mame/drivers/ut88.cpp",
	MAME_DIR .. "src/mame/includes/ut88.h",
	MAME_DIR .. "src/mame/machine/ut88.cpp",
	MAME_DIR .. "src/mame/video/ut88.cpp",
	MAME_DIR .. "src/mame/drivers/vector06.cpp",
	MAME_DIR .. "src/mame/includes/vector06.h",
	MAME_DIR .. "src/mame/machine/vector06.cpp",
	MAME_DIR .. "src/mame/video/vector06.cpp",
	MAME_DIR .. "src/mame/drivers/vta2000.cpp",
}

createMESSProjects(_target, _subtarget, "cce")
files {
	MAME_DIR .. "src/mame/drivers/mc1000.cpp",
	MAME_DIR .. "src/mame/includes/mc1000.h",
}

createMESSProjects(_target, _subtarget, "ccs")
files {
	MAME_DIR .. "src/mame/drivers/ccs2810.cpp",
	MAME_DIR .. "src/mame/drivers/ccs300.cpp",
}

createMESSProjects(_target, _subtarget, "chromatics")
files {
	MAME_DIR .. "src/mame/drivers/cgc7900.cpp",
	MAME_DIR .. "src/mame/includes/cgc7900.h",
	MAME_DIR .. "src/mame/video/cgc7900.cpp",
}

createMESSProjects(_target, _subtarget, "coleco")
files {
	MAME_DIR .. "src/mame/drivers/adam.cpp",
	MAME_DIR .. "src/mame/includes/adam.h",
	MAME_DIR .. "src/mame/drivers/coleco.cpp",
	MAME_DIR .. "src/mame/includes/coleco.h",
	MAME_DIR .. "src/mame/machine/coleco.cpp",
	MAME_DIR .. "src/mame/machine/coleco.h",
}

createMESSProjects(_target, _subtarget, "cromemco")
files {
	MAME_DIR .. "src/mame/drivers/c10.cpp",
	MAME_DIR .. "src/mame/drivers/mcb216.cpp",
}

createMESSProjects(_target, _subtarget, "comx")
files {
	MAME_DIR .. "src/mame/drivers/comx35.cpp",
	MAME_DIR .. "src/mame/includes/comx35.h",
	MAME_DIR .. "src/mame/video/comx35.cpp",
}

createMESSProjects(_target, _subtarget, "concept")
files {
	MAME_DIR .. "src/mame/drivers/concept.cpp",
	MAME_DIR .. "src/mame/includes/concept.h",
	MAME_DIR .. "src/mame/machine/concept.cpp",
}

createMESSProjects(_target, _subtarget, "conitec")
files {
	MAME_DIR .. "src/mame/drivers/prof180x.cpp",
	MAME_DIR .. "src/mame/includes/prof180x.h",
	MAME_DIR .. "src/mame/drivers/prof80.cpp",
	MAME_DIR .. "src/mame/includes/prof80.h",
	MAME_DIR .. "src/mame/machine/prof80mmu.cpp",
	MAME_DIR .. "src/mame/machine/prof80mmu.h",
}

createMESSProjects(_target, _subtarget, "cybiko")
files {
	MAME_DIR .. "src/mame/drivers/cybiko.cpp",
	MAME_DIR .. "src/mame/includes/cybiko.h",
	MAME_DIR .. "src/mame/machine/cybiko.cpp",
}

createMESSProjects(_target, _subtarget, "dai")
files {
	MAME_DIR .. "src/mame/drivers/dai.cpp",
	MAME_DIR .. "src/mame/includes/dai.h",
	MAME_DIR .. "src/mame/audio/dai_snd.cpp",
	MAME_DIR .. "src/mame/audio/dai_snd.h",
	MAME_DIR .. "src/mame/machine/dai.cpp",
	MAME_DIR .. "src/mame/video/dai.cpp",
}

createMESSProjects(_target, _subtarget, "ddr")
files {
	MAME_DIR .. "src/mame/drivers/ac1.cpp",
	MAME_DIR .. "src/mame/includes/ac1.h",
	MAME_DIR .. "src/mame/machine/ac1.cpp",
	MAME_DIR .. "src/mame/video/ac1.cpp",
	MAME_DIR .. "src/mame/drivers/bcs3.cpp",
	MAME_DIR .. "src/mame/drivers/c80.cpp",
	MAME_DIR .. "src/mame/includes/c80.h",
	MAME_DIR .. "src/mame/drivers/huebler.cpp",
	MAME_DIR .. "src/mame/includes/huebler.h",
	MAME_DIR .. "src/mame/drivers/jtc.cpp",
	MAME_DIR .. "src/mame/drivers/kramermc.cpp",
	MAME_DIR .. "src/mame/includes/kramermc.h",
	MAME_DIR .. "src/mame/machine/kramermc.cpp",
	MAME_DIR .. "src/mame/video/kramermc.cpp",
	MAME_DIR .. "src/mame/drivers/llc.cpp",
	MAME_DIR .. "src/mame/includes/llc.h",
	MAME_DIR .. "src/mame/machine/llc.cpp",
	MAME_DIR .. "src/mame/video/llc.cpp",
	MAME_DIR .. "src/mame/drivers/nanos.cpp",
	MAME_DIR .. "src/mame/drivers/pcm.cpp",
	MAME_DIR .. "src/mame/drivers/vcs80.cpp",
	MAME_DIR .. "src/mame/includes/vcs80.h",
	MAME_DIR .. "src/mame/machine/k7659kb.cpp",
	MAME_DIR .. "src/mame/machine/k7659kb.h",
}

createMESSProjects(_target, _subtarget, "dec")
files {
	MAME_DIR .. "src/mame/drivers/dct11em.cpp",
	MAME_DIR .. "src/mame/drivers/dectalk.cpp",
	MAME_DIR .. "src/mame/drivers/decwritr.cpp",
	MAME_DIR .. "src/mame/drivers/pdp11.cpp",
	MAME_DIR .. "src/mame/drivers/vax11.cpp",
	MAME_DIR .. "src/mame/drivers/rainbow.cpp",
	MAME_DIR .. "src/mame/drivers/vk100.cpp",
	MAME_DIR .. "src/mame/drivers/vt100.cpp",
	MAME_DIR .. "src/mame/drivers/vt220.cpp",
	MAME_DIR .. "src/mame/drivers/vt240.cpp",
	MAME_DIR .. "src/mame/drivers/vt320.cpp",
	MAME_DIR .. "src/mame/drivers/vt520.cpp",
	MAME_DIR .. "src/mame/machine/dec_lk201.cpp",
	MAME_DIR .. "src/mame/machine/dec_lk201.h",
	MAME_DIR .. "src/mame/machine/rx01.cpp",
	MAME_DIR .. "src/mame/machine/rx01.h",
	MAME_DIR .. "src/mame/video/vtvideo.cpp",
	MAME_DIR .. "src/mame/video/vtvideo.h",
}

createMESSProjects(_target, _subtarget, "dicksmth")
files {
	MAME_DIR .. "src/mame/drivers/super80.cpp",
	MAME_DIR .. "src/mame/includes/super80.h",
	MAME_DIR .. "src/mame/machine/super80.cpp",
	MAME_DIR .. "src/mame/video/super80.cpp",
}

createMESSProjects(_target, _subtarget, "dms")
files {
	MAME_DIR .. "src/mame/drivers/dms5000.cpp",
	MAME_DIR .. "src/mame/drivers/dms86.cpp",
	MAME_DIR .. "src/mame/drivers/zsbc3.cpp",
}

createMESSProjects(_target, _subtarget, "dragon")
files {
	MAME_DIR .. "src/mame/drivers/dgn_beta.cpp",
	MAME_DIR .. "src/mame/includes/dgn_beta.h",
	MAME_DIR .. "src/mame/machine/dgn_beta.cpp",
	MAME_DIR .. "src/mame/video/dgn_beta.cpp",
}

createMESSProjects(_target, _subtarget, "drc")
files {
	MAME_DIR .. "src/mame/drivers/zrt80.cpp",
}

createMESSProjects(_target, _subtarget, "eaca")
files {
	MAME_DIR .. "src/mame/drivers/cgenie.cpp",
}

createMESSProjects(_target, _subtarget, "einis")
files {
	MAME_DIR .. "src/mame/drivers/pecom.cpp",
	MAME_DIR .. "src/mame/includes/pecom.h",
	MAME_DIR .. "src/mame/machine/pecom.cpp",
	MAME_DIR .. "src/mame/video/pecom.cpp",
}

createMESSProjects(_target, _subtarget, "elektrka")
files {
	MAME_DIR .. "src/mame/drivers/bk.cpp",
	MAME_DIR .. "src/mame/includes/bk.h",
	MAME_DIR .. "src/mame/machine/bk.cpp",
	MAME_DIR .. "src/mame/video/bk.cpp",
	MAME_DIR .. "src/mame/drivers/dvk_kcgd.cpp",
	MAME_DIR .. "src/mame/drivers/dvk_ksm.cpp",
	MAME_DIR .. "src/mame/machine/ms7004.cpp",
	MAME_DIR .. "src/mame/machine/ms7004.h",
	MAME_DIR .. "src/mame/drivers/mk85.cpp",
	MAME_DIR .. "src/mame/drivers/mk90.cpp",
}

createMESSProjects(_target, _subtarget, "elektor")
files {
	MAME_DIR .. "src/mame/drivers/ec65.cpp",
	MAME_DIR .. "src/mame/drivers/elekscmp.cpp",
	MAME_DIR .. "src/mame/drivers/junior.cpp",
}

createMESSProjects(_target, _subtarget, "ensoniq")
files {
	MAME_DIR .. "src/mame/drivers/esq1.cpp",
	MAME_DIR .. "src/mame/drivers/esq5505.cpp",
	MAME_DIR .. "src/mame/drivers/esqasr.cpp",
	MAME_DIR .. "src/mame/drivers/esqkt.cpp",
	MAME_DIR .. "src/mame/drivers/esqmr.cpp",
	MAME_DIR .. "src/mame/drivers/enmirage.cpp",
	MAME_DIR .. "src/mame/machine/esqpanel.cpp",
	MAME_DIR .. "src/mame/machine/esqpanel.h",
	MAME_DIR .. "src/mame/machine/esqvfd.cpp",
	MAME_DIR .. "src/mame/machine/esqvfd.h",
}

createMESSProjects(_target, _subtarget, "enterprise")
files {
	MAME_DIR .. "src/mame/drivers/ep64.cpp",
	MAME_DIR .. "src/mame/includes/ep64.h",
	MAME_DIR .. "src/mame/audio/dave.cpp",
	MAME_DIR .. "src/mame/audio/dave.h",
	MAME_DIR .. "src/mame/video/nick.cpp",
	MAME_DIR .. "src/mame/video/nick.h",
}

createMESSProjects(_target, _subtarget, "entex")
files {
	MAME_DIR .. "src/mame/drivers/advision.cpp",
	MAME_DIR .. "src/mame/includes/advision.h",
	MAME_DIR .. "src/mame/machine/advision.cpp",
	MAME_DIR .. "src/mame/video/advision.cpp",
}

createMESSProjects(_target, _subtarget, "epoch")
files {
	MAME_DIR .. "src/mame/drivers/gamepock.cpp",
	MAME_DIR .. "src/mame/includes/gamepock.h",
	MAME_DIR .. "src/mame/machine/gamepock.cpp",
	MAME_DIR .. "src/mame/drivers/scv.cpp",
	MAME_DIR .. "src/mame/audio/upd1771.cpp",
	MAME_DIR .. "src/mame/audio/upd1771.h",
}

createMESSProjects(_target, _subtarget, "epson")
files {
	MAME_DIR .. "src/mame/drivers/hx20.cpp",
	MAME_DIR .. "src/mame/includes/hx20.h",
	MAME_DIR .. "src/mame/drivers/px4.cpp",
	MAME_DIR .. "src/mame/drivers/px8.cpp",
	MAME_DIR .. "src/mame/includes/px8.h",
	MAME_DIR .. "src/mame/drivers/qx10.cpp",
	MAME_DIR .. "src/mame/machine/qx10kbd.cpp",
	MAME_DIR .. "src/mame/machine/qx10kbd.h",
}

createMESSProjects(_target, _subtarget, "exidy")
files {
	MAME_DIR .. "src/mame/machine/sorcerer.cpp",
	MAME_DIR .. "src/mame/drivers/sorcerer.cpp",
	MAME_DIR .. "src/mame/includes/sorcerer.h",
	MAME_DIR .. "src/mame/machine/micropolis.cpp",
	MAME_DIR .. "src/mame/machine/micropolis.h",
}

createMESSProjects(_target, _subtarget, "fairch")
files {
	MAME_DIR .. "src/mame/drivers/channelf.cpp",
	MAME_DIR .. "src/mame/includes/channelf.h",
	MAME_DIR .. "src/mame/audio/channelf.cpp",
	MAME_DIR .. "src/mame/audio/channelf.h",
	MAME_DIR .. "src/mame/video/channelf.cpp",
}

createMESSProjects(_target, _subtarget, "fidelity")
files {
	MAME_DIR .. "src/mame/drivers/fidelz80.cpp",
	MAME_DIR .. "src/mame/includes/fidelz80.h",
	MAME_DIR .. "src/mame/drivers/fidel6502.cpp",
}

createMESSProjects(_target, _subtarget, "force")
files {
	MAME_DIR .. "src/mame/drivers/force68k.cpp",
}

createMESSProjects(_target, _subtarget, "fujitsu")
files {
	MAME_DIR .. "src/mame/drivers/fmtowns.cpp",
	MAME_DIR .. "src/mame/includes/fmtowns.h",
	MAME_DIR .. "src/mame/video/fmtowns.cpp",
	MAME_DIR .. "src/mame/machine/fm_scsi.cpp",
	MAME_DIR .. "src/mame/machine/fm_scsi.h",
	MAME_DIR .. "src/mame/drivers/fm7.cpp",
	MAME_DIR .. "src/mame/includes/fm7.h",
	MAME_DIR .. "src/mame/video/fm7.cpp",
}

createMESSProjects(_target, _subtarget, "funtech")
files {
	MAME_DIR .. "src/mame/drivers/supracan.cpp",
}

createMESSProjects(_target, _subtarget, "galaxy")
files {
	MAME_DIR .. "src/mame/drivers/galaxy.cpp",
	MAME_DIR .. "src/mame/includes/galaxy.h",
	MAME_DIR .. "src/mame/machine/galaxy.cpp",
	MAME_DIR .. "src/mame/video/galaxy.cpp",
}

createMESSProjects(_target, _subtarget, "gamepark")
files {
	MAME_DIR .. "src/mame/drivers/gp2x.cpp",
	MAME_DIR .. "src/mame/drivers/gp32.cpp",
	MAME_DIR .. "src/mame/includes/gp32.h",
}

createMESSProjects(_target, _subtarget, "gi")
files {
	MAME_DIR .. "src/mame/drivers/hh_pic16.cpp",
}

createMESSProjects(_target, _subtarget, "grundy")
files {
	MAME_DIR .. "src/mame/drivers/newbrain.cpp",
	MAME_DIR .. "src/mame/includes/newbrain.h",
	MAME_DIR .. "src/mame/video/newbrain.cpp",
}

createMESSProjects(_target, _subtarget, "hartung")
files {
	MAME_DIR .. "src/mame/drivers/gmaster.cpp",
}

createMESSProjects(_target, _subtarget, "heathkit")
files {
	MAME_DIR .. "src/mame/drivers/et3400.cpp",
	MAME_DIR .. "src/mame/drivers/h8.cpp",
	MAME_DIR .. "src/mame/drivers/h19.cpp",
	MAME_DIR .. "src/mame/drivers/h89.cpp",
}

createMESSProjects(_target, _subtarget, "hegener")
files {
	MAME_DIR .. "src/mame/drivers/glasgow.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto.cpp",
	MAME_DIR .. "src/mame/drivers/mmodular.cpp",
	MAME_DIR .. "src/mame/drivers/stratos.cpp",
	MAME_DIR .. "src/mame/machine/mboard.cpp",
	MAME_DIR .. "src/mame/includes/mboard.h",
}

createMESSProjects(_target, _subtarget, "hitachi")
files {
	MAME_DIR .. "src/mame/drivers/b16.cpp",
	MAME_DIR .. "src/mame/drivers/bmjr.cpp",
	MAME_DIR .. "src/mame/drivers/bml3.cpp",
	MAME_DIR .. "src/mame/drivers/hh_hmcs40.cpp",
}

createMESSProjects(_target, _subtarget, "homebrew")
files {
	MAME_DIR .. "src/mame/drivers/4004clk.cpp",
	MAME_DIR .. "src/mame/drivers/68ksbc.cpp",
	MAME_DIR .. "src/mame/drivers/craft.cpp",
	MAME_DIR .. "src/mame/drivers/homez80.cpp",
	MAME_DIR .. "src/mame/drivers/p112.cpp",
	MAME_DIR .. "src/mame/drivers/phunsy.cpp",
	MAME_DIR .. "src/mame/drivers/pimps.cpp",
	MAME_DIR .. "src/mame/drivers/ravens.cpp",
	MAME_DIR .. "src/mame/drivers/sbc6510.cpp",
	MAME_DIR .. "src/mame/drivers/sitcom.cpp",
	MAME_DIR .. "src/mame/drivers/slc1.cpp",
	MAME_DIR .. "src/mame/drivers/uzebox.cpp",
	MAME_DIR .. "src/mame/drivers/z80dev.cpp",
}

createMESSProjects(_target, _subtarget, "homelab")
files {
	MAME_DIR .. "src/mame/drivers/homelab.cpp",
}

createMESSProjects(_target, _subtarget, "hp")
files {
	MAME_DIR .. "src/mame/drivers/hp16500.cpp",
	MAME_DIR .. "src/mame/drivers/hp48.cpp",
	MAME_DIR .. "src/mame/includes/hp48.h",
	MAME_DIR .. "src/mame/machine/hp48.cpp",
	MAME_DIR .. "src/mame/video/hp48.cpp",
	MAME_DIR .. "src/mame/drivers/hp49gp.cpp",
	MAME_DIR .. "src/mame/drivers/hp9845.cpp",
	MAME_DIR .. "src/mame/drivers/hp9k.cpp",
	MAME_DIR .. "src/mame/drivers/hp9k_3xx.cpp",
	MAME_DIR .. "src/mame/drivers/hp64k.cpp",
	MAME_DIR .. "src/mame/drivers/hp_ipc.cpp",
}

createMESSProjects(_target, _subtarget, "hec2hrp")
files {
	MAME_DIR .. "src/mame/drivers/hec2hrp.cpp",
	MAME_DIR .. "src/mame/includes/hec2hrp.h",
	MAME_DIR .. "src/mame/machine/hec2hrp.cpp",
	MAME_DIR .. "src/mame/machine/hecdisk2.cpp",
	MAME_DIR .. "src/mame/video/hec2video.cpp",
	MAME_DIR .. "src/mame/drivers/interact.cpp",
}

createMESSProjects(_target, _subtarget, "heurikon")
files {
	MAME_DIR .. "src/mame/drivers/hk68v10.cpp",
}

createMESSProjects(_target, _subtarget, "intel")
files {
	MAME_DIR .. "src/mame/drivers/basic52.cpp",
	MAME_DIR .. "src/mame/drivers/imds.cpp",
	MAME_DIR .. "src/mame/drivers/ipc.cpp",
	MAME_DIR .. "src/mame/drivers/ipds.cpp",
	MAME_DIR .. "src/mame/drivers/isbc.cpp",
	MAME_DIR .. "src/mame/machine/isbc_215g.cpp",
	MAME_DIR .. "src/mame/machine/isbc_215g.h",
	MAME_DIR .. "src/mame/drivers/rex6000.cpp",
	MAME_DIR .. "src/mame/drivers/sdk80.cpp",
	MAME_DIR .. "src/mame/drivers/sdk85.cpp",
	MAME_DIR .. "src/mame/drivers/sdk86.cpp",
	MAME_DIR .. "src/mame/drivers/imds2.cpp",
	MAME_DIR .. "src/mame/includes/imds2.h",
}

createMESSProjects(_target, _subtarget, "imp")
files {
	MAME_DIR .. "src/mame/drivers/tim011.cpp",
	MAME_DIR .. "src/mame/drivers/tim100.cpp",
}

createMESSProjects(_target, _subtarget, "interton")
files {
	MAME_DIR .. "src/mame/drivers/vc4000.cpp",
	MAME_DIR .. "src/mame/includes/vc4000.h",
	MAME_DIR .. "src/mame/audio/vc4000snd.cpp",
	MAME_DIR .. "src/mame/audio/vc4000snd.h",
	MAME_DIR .. "src/mame/video/vc4000.cpp",
}

createMESSProjects(_target, _subtarget, "intv")
files {
	MAME_DIR .. "src/mame/drivers/intv.cpp",
	MAME_DIR .. "src/mame/includes/intv.h",
	MAME_DIR .. "src/mame/machine/intv.cpp",
	MAME_DIR .. "src/mame/video/intv.cpp",
	MAME_DIR .. "src/mame/video/stic.cpp",
	MAME_DIR .. "src/mame/video/stic.h",
}

createMESSProjects(_target, _subtarget, "isc")
files {
	MAME_DIR .. "src/mame/drivers/compucolor.cpp",
}

createMESSProjects(_target, _subtarget, "kaypro")
files {
	MAME_DIR .. "src/mame/drivers/kaypro.cpp",
	MAME_DIR .. "src/mame/includes/kaypro.h",
	MAME_DIR .. "src/mame/machine/kaypro.cpp",
	MAME_DIR .. "src/mame/machine/kay_kbd.cpp",
	MAME_DIR .. "src/mame/video/kaypro.cpp",
}

createMESSProjects(_target, _subtarget, "koei")
files {
	MAME_DIR .. "src/mame/drivers/pasogo.cpp",
}

createMESSProjects(_target, _subtarget, "kyocera")
files {
	MAME_DIR .. "src/mame/drivers/kyocera.cpp",
	MAME_DIR .. "src/mame/includes/kyocera.h",
	MAME_DIR .. "src/mame/video/kyocera.cpp",
}

createMESSProjects(_target, _subtarget, "luxor")
files {
	MAME_DIR .. "src/mame/drivers/abc80.cpp",
	MAME_DIR .. "src/mame/includes/abc80.h",
	MAME_DIR .. "src/mame/machine/abc80kb.cpp",
	MAME_DIR .. "src/mame/machine/abc80kb.h",
	MAME_DIR .. "src/mame/video/abc80.cpp",
	MAME_DIR .. "src/mame/drivers/abc80x.cpp",
	MAME_DIR .. "src/mame/includes/abc80x.h",
	MAME_DIR .. "src/mame/video/abc800.cpp",
	MAME_DIR .. "src/mame/video/abc802.cpp",
	MAME_DIR .. "src/mame/video/abc806.cpp",
	MAME_DIR .. "src/mame/drivers/abc1600.cpp",
	MAME_DIR .. "src/mame/includes/abc1600.h",
	MAME_DIR .. "src/mame/machine/abc1600mac.cpp",
	MAME_DIR .. "src/mame/machine/abc1600mac.h",
	MAME_DIR .. "src/mame/video/abc1600.cpp",
	MAME_DIR .. "src/mame/video/abc1600.h",
}

createMESSProjects(_target, _subtarget, "magnavox")
files {
	MAME_DIR .. "src/mame/drivers/odyssey2.cpp",
}

createMESSProjects(_target, _subtarget, "makerbot")
files {
	MAME_DIR .. "src/mame/drivers/replicator.cpp",
}

createMESSProjects(_target, _subtarget, "marx")
files {
	MAME_DIR .. "src/mame/drivers/elecbowl.cpp",
}

createMESSProjects(_target, _subtarget, "mattel")
files {
	MAME_DIR .. "src/mame/drivers/aquarius.cpp",
	MAME_DIR .. "src/mame/includes/aquarius.h",
	MAME_DIR .. "src/mame/video/aquarius.cpp",
	MAME_DIR .. "src/mame/drivers/juicebox.cpp",
	MAME_DIR .. "src/mame/drivers/hyperscan.cpp",
}

createMESSProjects(_target, _subtarget, "matsushi")
files {
	MAME_DIR .. "src/mame/drivers/jr100.cpp",
	MAME_DIR .. "src/mame/drivers/jr200.cpp",
	MAME_DIR .. "src/mame/drivers/myb3k.cpp",
}

createMESSProjects(_target, _subtarget, "mb")
files {
	MAME_DIR .. "src/mame/drivers/mbdtower.cpp",
	MAME_DIR .. "src/mame/drivers/microvsn.cpp",
}

createMESSProjects(_target, _subtarget, "mchester")
files {
	MAME_DIR .. "src/mame/drivers/ssem.cpp",
}

createMESSProjects(_target, _subtarget, "memotech")
files {
	MAME_DIR .. "src/mame/drivers/mtx.cpp",
	MAME_DIR .. "src/mame/includes/mtx.h",
	MAME_DIR .. "src/mame/machine/mtx.cpp",
}

createMESSProjects(_target, _subtarget, "mgu")
files {
	MAME_DIR .. "src/mame/drivers/irisha.cpp",
}

createMESSProjects(_target, _subtarget, "microkey")
files {
	MAME_DIR .. "src/mame/drivers/primo.cpp",
	MAME_DIR .. "src/mame/includes/primo.h",
	MAME_DIR .. "src/mame/machine/primo.cpp",
	MAME_DIR .. "src/mame/video/primo.cpp",
}

createMESSProjects(_target, _subtarget, "microsoft")
files {
	MAME_DIR .. "src/mame/drivers/xbox.cpp",
	MAME_DIR .. "src/mame/includes/xbox.h",
}

createMESSProjects(_target, _subtarget, "mit")
files {
	MAME_DIR .. "src/mame/drivers/tx0.cpp",
	MAME_DIR .. "src/mame/includes/tx0.h",
	MAME_DIR .. "src/mame/video/crt.cpp",
	MAME_DIR .. "src/mame/video/crt.h",
	MAME_DIR .. "src/mame/video/tx0.cpp",
}

createMESSProjects(_target, _subtarget, "mits")
files {
	MAME_DIR .. "src/mame/drivers/altair.cpp",
	MAME_DIR .. "src/mame/drivers/mits680b.cpp",
}

createMESSProjects(_target, _subtarget, "mitsubishi")
files {
	MAME_DIR .. "src/mame/drivers/hh_melps4.cpp",
	MAME_DIR .. "src/mame/drivers/multi8.cpp",
	MAME_DIR .. "src/mame/drivers/multi16.cpp",
}

createMESSProjects(_target, _subtarget, "mizar")
files {
	MAME_DIR .. "src/mame/drivers/mzr8105.cpp",
}

createMESSProjects(_target, _subtarget, "morrow")
files {
	MAME_DIR .. "src/mame/drivers/microdec.cpp",
	MAME_DIR .. "src/mame/drivers/mpz80.cpp",
	MAME_DIR .. "src/mame/includes/mpz80.h",
	MAME_DIR .. "src/mame/drivers/tricep.cpp",
}

createMESSProjects(_target, _subtarget, "mos")
files {
	MAME_DIR .. "src/mame/drivers/kim1.cpp",
}

createMESSProjects(_target, _subtarget, "motorola")
files {
	MAME_DIR .. "src/mame/drivers/m6805evs.cpp",
	MAME_DIR .. "src/mame/drivers/mekd2.cpp",
}

createMESSProjects(_target, _subtarget, "multitch")
files {
	MAME_DIR .. "src/mame/drivers/mkit09.cpp",
	MAME_DIR .. "src/mame/drivers/mpf1.cpp",
	MAME_DIR .. "src/mame/includes/mpf1.h",
}

createMESSProjects(_target, _subtarget, "nakajima")
files {
	MAME_DIR .. "src/mame/drivers/nakajies.cpp",
}

createMESSProjects(_target, _subtarget, "nascom")
files {
	MAME_DIR .. "src/mame/drivers/nascom1.cpp",
}

createMESSProjects(_target, _subtarget, "ne")
files {
	MAME_DIR .. "src/mame/drivers/z80ne.cpp",
	MAME_DIR .. "src/mame/includes/z80ne.h",
	MAME_DIR .. "src/mame/machine/z80ne.cpp",
}

createMESSProjects(_target, _subtarget, "nec")
files {
	MAME_DIR .. "src/mame/drivers/apc.cpp",
	MAME_DIR .. "src/mame/drivers/pce.cpp",
	MAME_DIR .. "src/mame/includes/pce.h",
	MAME_DIR .. "src/mame/machine/pce.cpp",
	MAME_DIR .. "src/mame/machine/pce_cd.cpp",
	MAME_DIR .. "src/mame/machine/pce_cd.h",
	MAME_DIR .. "src/mame/drivers/pcfx.cpp",
	MAME_DIR .. "src/mame/drivers/pc6001.cpp",
	MAME_DIR .. "src/mame/drivers/pc8401a.cpp",
	MAME_DIR .. "src/mame/includes/pc8401a.h",
	MAME_DIR .. "src/mame/video/pc8401a.cpp",
	MAME_DIR .. "src/mame/drivers/pc8001.cpp",
	MAME_DIR .. "src/mame/includes/pc8001.h",
	MAME_DIR .. "src/mame/drivers/pc8801.cpp",
	MAME_DIR .. "src/mame/drivers/pc88va.cpp",
	MAME_DIR .. "src/mame/drivers/pc100.cpp",
	MAME_DIR .. "src/mame/drivers/pc9801.cpp",
	MAME_DIR .. "src/mame/machine/pc9801_26.cpp",
	MAME_DIR .. "src/mame/machine/pc9801_26.h",
	MAME_DIR .. "src/mame/machine/pc9801_86.cpp",
	MAME_DIR .. "src/mame/machine/pc9801_86.h",
	MAME_DIR .. "src/mame/machine/pc9801_118.cpp",
	MAME_DIR .. "src/mame/machine/pc9801_118.h",
	MAME_DIR .. "src/mame/machine/pc9801_cbus.cpp",
	MAME_DIR .. "src/mame/machine/pc9801_cbus.h",
	MAME_DIR .. "src/mame/machine/pc9801_kbd.cpp",
	MAME_DIR .. "src/mame/machine/pc9801_kbd.h",
	MAME_DIR .. "src/mame/drivers/tk80bs.cpp",
	MAME_DIR .. "src/mame/drivers/hh_ucom4.cpp",
	MAME_DIR .. "src/mame/includes/hh_ucom4.h",
}

createMESSProjects(_target, _subtarget, "netronic")
files {
	MAME_DIR .. "src/mame/drivers/elf.cpp",
	MAME_DIR .. "src/mame/includes/elf.h",
	MAME_DIR .. "src/mame/drivers/exp85.cpp",
	MAME_DIR .. "src/mame/includes/exp85.h",
}

createMESSProjects(_target, _subtarget, "next")
files {
	MAME_DIR .. "src/mame/drivers/next.cpp",
	MAME_DIR .. "src/mame/includes/next.h",
	MAME_DIR .. "src/mame/machine/nextkbd.cpp",
	MAME_DIR .. "src/mame/machine/nextkbd.h",
	MAME_DIR .. "src/mame/machine/nextmo.cpp",
	MAME_DIR .. "src/mame/machine/nextmo.h",
}

createMESSProjects(_target, _subtarget, "nintendo")
files {
	MAME_DIR .. "src/mame/drivers/gb.cpp",
	MAME_DIR .. "src/mame/includes/gb.h",
	MAME_DIR .. "src/mame/audio/gb.cpp",
	MAME_DIR .. "src/mame/audio/gb.h",
	MAME_DIR .. "src/mame/machine/gb.cpp",
	MAME_DIR .. "src/mame/video/gb_lcd.cpp",
	MAME_DIR .. "src/mame/video/gb_lcd.h",
	MAME_DIR .. "src/mame/drivers/gba.cpp",
	MAME_DIR .. "src/mame/includes/gba.h",
	MAME_DIR .. "src/mame/video/gba.cpp",
	MAME_DIR .. "src/mame/drivers/n64.cpp",
	MAME_DIR .. "src/mame/includes/n64.h",
	MAME_DIR .. "src/mame/drivers/nes.cpp",
	MAME_DIR .. "src/mame/includes/nes.h",
	MAME_DIR .. "src/mame/machine/nes.cpp",
	MAME_DIR .. "src/mame/video/nes.cpp",
	MAME_DIR .. "src/mame/drivers/pokemini.cpp",
	MAME_DIR .. "src/mame/drivers/snes.cpp",
	MAME_DIR .. "src/mame/includes/snes.h",
	MAME_DIR .. "src/mame/machine/snescx4.cpp",
	MAME_DIR .. "src/mame/machine/snescx4.h",
	MAME_DIR .. "src/mame/machine/cx4data.inc",
	MAME_DIR .. "src/mame/machine/cx4fn.inc",
	MAME_DIR .. "src/mame/machine/cx4oam.inc",
	MAME_DIR .. "src/mame/machine/cx4ops.inc",
	MAME_DIR .. "src/mame/drivers/vboy.cpp",
	MAME_DIR .. "src/mame/audio/vboy.cpp",
	MAME_DIR .. "src/mame/audio/vboy.h",
}

createMESSProjects(_target, _subtarget, "nokia")
files {
	MAME_DIR .. "src/mame/drivers/mikromik.cpp",
	MAME_DIR .. "src/mame/includes/mikromik.h",
	MAME_DIR .. "src/mame/machine/mm1kb.cpp",
	MAME_DIR .. "src/mame/machine/mm1kb.h",
	MAME_DIR .. "src/mame/video/mikromik.cpp",
	MAME_DIR .. "src/mame/drivers/nokia_3310.cpp",
}

createMESSProjects(_target, _subtarget, "northstar")
files {
	MAME_DIR .. "src/mame/drivers/horizon.cpp",
}

createMESSProjects(_target, _subtarget, "novag")
files {
	MAME_DIR .. "src/mame/drivers/mk1.cpp",
	MAME_DIR .. "src/mame/drivers/mk2.cpp",
	MAME_DIR .. "src/mame/drivers/ssystem3.cpp",
	MAME_DIR .. "src/mame/includes/ssystem3.h",
	MAME_DIR .. "src/mame/video/ssystem3.cpp",
	MAME_DIR .. "src/mame/drivers/supercon.cpp",
}

createMESSProjects(_target, _subtarget, "olivetti")
files {
	MAME_DIR .. "src/mame/drivers/m20.cpp",
	MAME_DIR .. "src/mame/machine/m20_kbd.cpp",
	MAME_DIR .. "src/mame/machine/m20_kbd.h",
	MAME_DIR .. "src/mame/machine/m20_8086.cpp",
	MAME_DIR .. "src/mame/machine/m20_8086.h",
	MAME_DIR .. "src/mame/drivers/m24.cpp",
	MAME_DIR .. "src/mame/machine/m24_kbd.cpp",
	MAME_DIR .. "src/mame/machine/m24_kbd.h",
	MAME_DIR .. "src/mame/machine/m24_z8000.cpp",
	MAME_DIR .. "src/mame/machine/m24_z8000.h",
}

createMESSProjects(_target, _subtarget, "olympia")
files {
	MAME_DIR .. "src/mame/drivers/peoplepc.cpp"
}

createMESSProjects(_target, _subtarget, "ns")
files {
	MAME_DIR .. "src/mame/drivers/hh_cop400.cpp",
}

createMESSProjects(_target, _subtarget, "omnibyte")
files {
	MAME_DIR .. "src/mame/drivers/msbc1.cpp",
	MAME_DIR .. "src/mame/includes/msbc1.h",
	MAME_DIR .. "src/mame/drivers/ob68k1a.cpp",
	MAME_DIR .. "src/mame/includes/ob68k1a.h",
}

createMESSProjects(_target, _subtarget, "orion")
files {
	MAME_DIR .. "src/mame/drivers/orion.cpp",
	MAME_DIR .. "src/mame/includes/orion.h",
	MAME_DIR .. "src/mame/machine/orion.cpp",
	MAME_DIR .. "src/mame/video/orion.cpp",
}

createMESSProjects(_target, _subtarget, "osborne")
files {
	MAME_DIR .. "src/mame/drivers/osborne1.cpp",
	MAME_DIR .. "src/mame/includes/osborne1.h",
	MAME_DIR .. "src/mame/machine/osborne1.cpp",
	MAME_DIR .. "src/mame/drivers/osbexec.cpp",
	MAME_DIR .. "src/mame/drivers/vixen.cpp",
	MAME_DIR .. "src/mame/includes/vixen.h",
}

createMESSProjects(_target, _subtarget, "osi")
files {
	MAME_DIR .. "src/mame/drivers/osi.cpp",
	MAME_DIR .. "src/mame/includes/osi.h",
	MAME_DIR .. "src/mame/video/osi.cpp",
}

createMESSProjects(_target, _subtarget, "palm")
files {
	MAME_DIR .. "src/mame/drivers/palm.cpp",
	MAME_DIR .. "src/mame/drivers/palm_dbg.inc",
	MAME_DIR .. "src/mame/drivers/palmz22.cpp",
}

createMESSProjects(_target, _subtarget, "parker")
files {
	MAME_DIR .. "src/mame/drivers/wildfire.cpp",
}

createMESSProjects(_target, _subtarget, "pitronic")
files {
	MAME_DIR .. "src/mame/drivers/beta.cpp",
}

createMESSProjects(_target, _subtarget, "pc")
files {
	MAME_DIR .. "src/mame/drivers/asst128.cpp",
	MAME_DIR .. "src/mame/drivers/europc.cpp",
	MAME_DIR .. "src/mame/drivers/genpc.cpp",
	MAME_DIR .. "src/mame/includes/genpc.h",
	MAME_DIR .. "src/mame/machine/genpc.cpp",
	MAME_DIR .. "src/mame/drivers/ibmpc.cpp",
	MAME_DIR .. "src/mame/drivers/ibmpcjr.cpp",
	MAME_DIR .. "src/mame/drivers/pc.cpp",
	MAME_DIR .. "src/mame/drivers/tandy1t.cpp",
	MAME_DIR .. "src/mame/video/pc_t1t.cpp",
	MAME_DIR .. "src/mame/video/pc_t1t.h",
}

createMESSProjects(_target, _subtarget, "pdp1")
files {
	MAME_DIR .. "src/mame/drivers/pdp1.cpp",
	MAME_DIR .. "src/mame/includes/pdp1.h",
	MAME_DIR .. "src/mame/video/pdp1.cpp",
}

createMESSProjects(_target, _subtarget, "pel")
files {
	MAME_DIR .. "src/mame/drivers/galeb.cpp",
	MAME_DIR .. "src/mame/includes/galeb.h",
	MAME_DIR .. "src/mame/video/galeb.cpp",
		MAME_DIR .. "src/mame/drivers/orao.cpp",
	MAME_DIR .. "src/mame/includes/orao.h",
	MAME_DIR .. "src/mame/machine/orao.cpp",
	MAME_DIR .. "src/mame/video/orao.cpp",
}

createMESSProjects(_target, _subtarget, "philips")
files {
	MAME_DIR .. "src/mame/drivers/p2000t.cpp",
	MAME_DIR .. "src/mame/includes/p2000t.h",
	MAME_DIR .. "src/mame/machine/p2000t.cpp",
	MAME_DIR .. "src/mame/video/p2000m.cpp",
	MAME_DIR .. "src/mame/drivers/vg5k.cpp",
}

createMESSProjects(_target, _subtarget, "poly88")
files {
	MAME_DIR .. "src/mame/drivers/poly88.cpp",
	MAME_DIR .. "src/mame/includes/poly88.h",
	MAME_DIR .. "src/mame/machine/poly88.cpp",
	MAME_DIR .. "src/mame/video/poly88.cpp",
}

createMESSProjects(_target, _subtarget, "psion")
files {
	MAME_DIR .. "src/mame/drivers/psion.cpp",
	MAME_DIR .. "src/mame/includes/psion.h",
	MAME_DIR .. "src/mame/machine/psion_pack.cpp",
	MAME_DIR .. "src/mame/machine/psion_pack.h",
}

createMESSProjects(_target, _subtarget, "radio")
files {
	MAME_DIR .. "src/mame/drivers/apogee.cpp",
	MAME_DIR .. "src/mame/drivers/mikrosha.cpp",
	MAME_DIR .. "src/mame/drivers/partner.cpp",
	MAME_DIR .. "src/mame/includes/partner.h",
	MAME_DIR .. "src/mame/machine/partner.cpp",
	MAME_DIR .. "src/mame/drivers/radio86.cpp",
	MAME_DIR .. "src/mame/includes/radio86.h",
	MAME_DIR .. "src/mame/machine/radio86.cpp",
}

createMESSProjects(_target, _subtarget, "rca")
files {
	MAME_DIR .. "src/mame/drivers/microkit.cpp",
	MAME_DIR .. "src/mame/drivers/studio2.cpp",
	MAME_DIR .. "src/mame/drivers/vip.cpp",
	MAME_DIR .. "src/mame/includes/vip.h",
}

createMESSProjects(_target, _subtarget, "regnecentralen")
files {
	MAME_DIR .. "src/mame/drivers/rc759.cpp",
}

createMESSProjects(_target, _subtarget, "ritam")
files {
	MAME_DIR .. "src/mame/drivers/monty.cpp",
}

createMESSProjects(_target, _subtarget, "rm")
files {
	MAME_DIR .. "src/mame/drivers/rm380z.cpp",
	MAME_DIR .. "src/mame/includes/rm380z.h",
	MAME_DIR .. "src/mame/machine/rm380z.cpp",
	MAME_DIR .. "src/mame/video/rm380z.cpp",
	MAME_DIR .. "src/mame/drivers/rmnimbus.cpp",
	MAME_DIR .. "src/mame/includes/rmnimbus.h",
	MAME_DIR .. "src/mame/machine/rmnimbus.cpp",
	MAME_DIR .. "src/mame/video/rmnimbus.cpp",
	MAME_DIR .. "src/mame/machine/rmnkbd.cpp",
	MAME_DIR .. "src/mame/machine/rmnkbd.h",
}

createMESSProjects(_target, _subtarget, "robotron")
files {
	MAME_DIR .. "src/mame/drivers/a5105.cpp",
	MAME_DIR .. "src/mame/drivers/a51xx.cpp",
	MAME_DIR .. "src/mame/drivers/a7150.cpp",
	MAME_DIR .. "src/mame/drivers/k1003.cpp",
	MAME_DIR .. "src/mame/drivers/k8915.cpp",
	MAME_DIR .. "src/mame/drivers/rt1715.cpp",
	MAME_DIR .. "src/mame/drivers/z1013.cpp",
	MAME_DIR .. "src/mame/drivers/z9001.cpp",
}

createMESSProjects(_target, _subtarget, "roland")
files {
	MAME_DIR .. "src/mame/drivers/rmt32.cpp",
	MAME_DIR .. "src/mame/drivers/rd110.cpp",
	MAME_DIR .. "src/mame/drivers/rsc55.cpp",
	MAME_DIR .. "src/mame/drivers/tb303.cpp",
	MAME_DIR .. "src/mame/drivers/tr606.cpp",
}

createMESSProjects(_target, _subtarget, "rolm")
files {
	MAME_DIR .. "src/mame/drivers/r9751.cpp",
}

createMESSProjects(_target, _subtarget, "rockwell")
files {
	MAME_DIR .. "src/mame/drivers/aim65.cpp",
	MAME_DIR .. "src/mame/includes/aim65.h",
	MAME_DIR .. "src/mame/machine/aim65.cpp",
	MAME_DIR .. "src/mame/drivers/aim65_40.cpp",
}

createMESSProjects(_target, _subtarget, "saturn")
files {
	MAME_DIR .. "src/mame/drivers/st17xx.cpp",
}

createMESSProjects(_target, _subtarget, "sage")
files {
	MAME_DIR .. "src/mame/drivers/sage2.cpp",
	MAME_DIR .. "src/mame/includes/sage2.h",
}

createMESSProjects(_target, _subtarget, "samcoupe")
files {
	MAME_DIR .. "src/mame/drivers/samcoupe.cpp",
	MAME_DIR .. "src/mame/includes/samcoupe.h",
	MAME_DIR .. "src/mame/machine/samcoupe.cpp",
	MAME_DIR .. "src/mame/video/samcoupe.cpp",
}

createMESSProjects(_target, _subtarget, "samsung")
files {
	MAME_DIR .. "src/mame/drivers/spc1000.cpp",
	MAME_DIR .. "src/mame/drivers/spc1500.cpp",
}

createMESSProjects(_target, _subtarget, "sanyo")
files {
	MAME_DIR .. "src/mame/drivers/mbc200.cpp",
	MAME_DIR .. "src/mame/drivers/mbc55x.cpp",
	MAME_DIR .. "src/mame/includes/mbc55x.h",
	MAME_DIR .. "src/mame/machine/mbc55x.cpp",
	MAME_DIR .. "src/mame/video/mbc55x.cpp",
	MAME_DIR .. "src/mame/drivers/phc25.cpp",
	MAME_DIR .. "src/mame/includes/phc25.h",
}

createMESSProjects(_target, _subtarget, "sega")
files {
	MAME_DIR .. "src/mame/drivers/dccons.cpp",
	MAME_DIR .. "src/mame/includes/dccons.h",
	MAME_DIR .. "src/mame/machine/dccons.cpp",
	MAME_DIR .. "src/mame/drivers/megadriv.cpp",
	MAME_DIR .. "src/mame/includes/megadriv.h",
	MAME_DIR .. "src/mame/includes/md_cons.h",
	MAME_DIR .. "src/mame/drivers/saturn.cpp",
	MAME_DIR .. "src/mame/drivers/segapico.cpp",
	MAME_DIR .. "src/mame/drivers/sega_sawatte.cpp",
	MAME_DIR .. "src/mame/drivers/segapm.cpp",
	MAME_DIR .. "src/mame/drivers/sg1000.cpp",
	MAME_DIR .. "src/mame/includes/sg1000.h",
	MAME_DIR .. "src/mame/drivers/sms.cpp",
	MAME_DIR .. "src/mame/includes/sms.h",
	MAME_DIR .. "src/mame/machine/sms.cpp",
	MAME_DIR .. "src/mame/drivers/svmu.cpp",
	MAME_DIR .. "src/mame/machine/mega32x.cpp",
	MAME_DIR .. "src/mame/machine/mega32x.h",
	MAME_DIR .. "src/mame/machine/megacd.cpp",
	MAME_DIR .. "src/mame/machine/megacd.h",
	MAME_DIR .. "src/mame/machine/megacdcd.cpp",
	MAME_DIR .. "src/mame/machine/megacdcd.h",
}

createMESSProjects(_target, _subtarget, "sequential")
files {
	MAME_DIR .. "src/mame/drivers/prophet600.cpp",
}

createMESSProjects(_target, _subtarget, "sgi")
files {
	MAME_DIR .. "src/mame/machine/sgi.cpp",
	MAME_DIR .. "src/mame/machine/sgi.h",
	MAME_DIR .. "src/mame/drivers/iris3130.cpp",
	MAME_DIR .. "src/mame/drivers/4dpi.cpp",
	MAME_DIR .. "src/mame/drivers/indigo.cpp",
	MAME_DIR .. "src/mame/drivers/indy_indigo2.cpp",
	MAME_DIR .. "src/mame/video/newport.cpp",
	MAME_DIR .. "src/mame/video/newport.h",
}

createMESSProjects(_target, _subtarget, "sharp")
files {
	MAME_DIR .. "src/mame/drivers/hh_sm510.cpp",
	MAME_DIR .. "src/mame/video/mz700.cpp",
	MAME_DIR .. "src/mame/drivers/mz700.cpp",
	MAME_DIR .. "src/mame/includes/mz700.h",
	MAME_DIR .. "src/mame/drivers/pc1500.cpp",
	MAME_DIR .. "src/mame/drivers/pocketc.cpp",
	MAME_DIR .. "src/mame/includes/pocketc.h",
	MAME_DIR .. "src/mame/video/pc1401.cpp",
	MAME_DIR .. "src/mame/machine/pc1401.cpp",
	MAME_DIR .. "src/mame/includes/pc1401.h",
	MAME_DIR .. "src/mame/video/pc1403.cpp",
	MAME_DIR .. "src/mame/machine/pc1403.cpp",
	MAME_DIR .. "src/mame/includes/pc1403.h",
	MAME_DIR .. "src/mame/video/pc1350.cpp",
	MAME_DIR .. "src/mame/machine/pc1350.cpp",
	MAME_DIR .. "src/mame/includes/pc1350.h",
	MAME_DIR .. "src/mame/video/pc1251.cpp",
	MAME_DIR .. "src/mame/machine/pc1251.cpp",
	MAME_DIR .. "src/mame/includes/pc1251.h",
	MAME_DIR .. "src/mame/video/pocketc.cpp",
	MAME_DIR .. "src/mame/machine/mz700.cpp",
	MAME_DIR .. "src/mame/drivers/x68k.cpp",
	MAME_DIR .. "src/mame/includes/x68k.h",
	MAME_DIR .. "src/mame/video/x68k.cpp",
	MAME_DIR .. "src/mame/machine/x68k_hdc.cpp",
	MAME_DIR .. "src/mame/machine/x68k_hdc.h",
	MAME_DIR .. "src/mame/machine/x68k_kbd.cpp",
	MAME_DIR .. "src/mame/machine/x68k_kbd.h",
	MAME_DIR .. "src/mame/drivers/mz80.cpp",
	MAME_DIR .. "src/mame/includes/mz80.h",
	MAME_DIR .. "src/mame/video/mz80.cpp",
	MAME_DIR .. "src/mame/machine/mz80.cpp",
	MAME_DIR .. "src/mame/drivers/mz2000.cpp",
	MAME_DIR .. "src/mame/drivers/x1.cpp",
	MAME_DIR .. "src/mame/includes/x1.h",
	MAME_DIR .. "src/mame/machine/x1.cpp",
	MAME_DIR .. "src/mame/drivers/x1twin.cpp",
	MAME_DIR .. "src/mame/drivers/mz2500.cpp",
	MAME_DIR .. "src/mame/drivers/mz3500.cpp",
	MAME_DIR .. "src/mame/drivers/pce220.cpp",
	MAME_DIR .. "src/mame/machine/pce220_ser.cpp",
	MAME_DIR .. "src/mame/machine/pce220_ser.h",
	MAME_DIR .. "src/mame/drivers/mz6500.cpp",
	MAME_DIR .. "src/mame/drivers/zaurus.cpp",
	MAME_DIR .. "src/mame/machine/pxa255.h",
}

createMESSProjects(_target, _subtarget, "sinclair")
files {
	MAME_DIR .. "src/mame/video/spectrum.cpp",
	MAME_DIR .. "src/mame/video/timex.cpp",
	MAME_DIR .. "src/mame/video/zx.cpp",
	MAME_DIR .. "src/mame/drivers/zx.cpp",
	MAME_DIR .. "src/mame/includes/zx.h",
	MAME_DIR .. "src/mame/machine/zx.cpp",
	MAME_DIR .. "src/mame/drivers/spectrum.cpp",
	MAME_DIR .. "src/mame/includes/spectrum.h",
	MAME_DIR .. "src/mame/drivers/spec128.cpp",
	MAME_DIR .. "src/mame/drivers/timex.cpp",
	MAME_DIR .. "src/mame/drivers/specpls3.cpp",
	MAME_DIR .. "src/mame/drivers/scorpion.cpp",
	MAME_DIR .. "src/mame/drivers/atm.cpp",
	MAME_DIR .. "src/mame/drivers/pentagon.cpp",
	MAME_DIR .. "src/mame/machine/beta.cpp",
	MAME_DIR .. "src/mame/machine/beta.h",
	MAME_DIR .. "src/mame/machine/spec_snqk.cpp",
	MAME_DIR .. "src/mame/machine/spec_snqk.h",
	MAME_DIR .. "src/mame/drivers/ql.cpp",
	MAME_DIR .. "src/mame/includes/ql.h",
	MAME_DIR .. "src/mame/machine/qimi.cpp",
	MAME_DIR .. "src/mame/machine/qimi.h",
	MAME_DIR .. "src/mame/video/zx8301.cpp",
	MAME_DIR .. "src/mame/video/zx8301.h",
	MAME_DIR .. "src/mame/machine/zx8302.cpp",
	MAME_DIR .. "src/mame/machine/zx8302.h",
}

createMESSProjects(_target, _subtarget, "siemens")
files {
	MAME_DIR .. "src/mame/drivers/pcd.cpp",
	MAME_DIR .. "src/mame/machine/pcd_kbd.cpp",
	MAME_DIR .. "src/mame/machine/pcd_kbd.h",
	MAME_DIR .. "src/mame/video/pcd.cpp",
	MAME_DIR .. "src/mame/video/pcd.h",
}

createMESSProjects(_target, _subtarget, "slicer")
files {
	MAME_DIR .. "src/mame/drivers/slicer.cpp",
}

createMESSProjects(_target, _subtarget, "snk")
files {
	MAME_DIR .. "src/mame/drivers/ng_aes.cpp",
	MAME_DIR .. "src/mame/drivers/ngp.cpp",
	MAME_DIR .. "src/mame/video/k1ge.cpp",
	MAME_DIR .. "src/mame/video/k1ge.h",
}

createMESSProjects(_target, _subtarget, "sony")
files {
	MAME_DIR .. "src/mame/drivers/pockstat.cpp",
	MAME_DIR .. "src/mame/drivers/psx.cpp",
	MAME_DIR .. "src/mame/machine/psxcd.cpp",
	MAME_DIR .. "src/mame/machine/psxcd.h",
	MAME_DIR .. "src/mame/drivers/pve500.cpp",
	MAME_DIR .. "src/mame/drivers/smc777.cpp",
}

createMESSProjects(_target, _subtarget, "sord")
files {
	MAME_DIR .. "src/mame/drivers/m5.cpp",
	MAME_DIR .. "src/mame/includes/m5.h",
}

createMESSProjects(_target, _subtarget, "special")
files {
	MAME_DIR .. "src/mame/drivers/special.cpp",
	MAME_DIR .. "src/mame/includes/special.h",
	MAME_DIR .. "src/mame/audio/specimx_snd.cpp",
	MAME_DIR .. "src/mame/audio/specimx_snd.h",
	MAME_DIR .. "src/mame/machine/special.cpp",
	MAME_DIR .. "src/mame/video/special.cpp",
}

createMESSProjects(_target, _subtarget, "sun")
files {
	MAME_DIR .. "src/mame/drivers/sun1.cpp",
	MAME_DIR .. "src/mame/drivers/sun2.cpp",
	MAME_DIR .. "src/mame/drivers/sun3.cpp",
	MAME_DIR .. "src/mame/drivers/sun4.cpp",
}

createMESSProjects(_target, _subtarget, "svi")
files {
	MAME_DIR .. "src/mame/drivers/svi318.cpp",
	MAME_DIR .. "src/mame/includes/svi318.h",
	MAME_DIR .. "src/mame/machine/svi318.cpp",
}

createMESSProjects(_target, _subtarget, "svision")
files {
	MAME_DIR .. "src/mame/drivers/svision.cpp",
	MAME_DIR .. "src/mame/includes/svision.h",
	MAME_DIR .. "src/mame/audio/svis_snd.cpp",
	MAME_DIR .. "src/mame/audio/svis_snd.h",
}

createMESSProjects(_target, _subtarget, "swtpc09")
files {
	MAME_DIR .. "src/mame/drivers/swtpc09.cpp",
	MAME_DIR .. "src/mame/includes/swtpc09.h",
	MAME_DIR .. "src/mame/machine/swtpc09.cpp",
}

createMESSProjects(_target, _subtarget, "synertec")
files {
	MAME_DIR .. "src/mame/drivers/sym1.cpp",
}

createMESSProjects(_target, _subtarget, "ta")
files {
	MAME_DIR .. "src/mame/drivers/alphatro.cpp",
}

createMESSProjects(_target, _subtarget, "tandberg")
files {
	MAME_DIR .. "src/mame/drivers/tdv2324.cpp",
	MAME_DIR .. "src/mame/includes/tdv2324.h",
}

createMESSProjects(_target, _subtarget, "tangerin")
files {
	MAME_DIR .. "src/mame/drivers/microtan.cpp",
	MAME_DIR .. "src/mame/includes/microtan.h",
	MAME_DIR .. "src/mame/machine/microtan.cpp",
	MAME_DIR .. "src/mame/video/microtan.cpp",
	MAME_DIR .. "src/mame/drivers/oric.cpp",
}

createMESSProjects(_target, _subtarget, "tatung")
files {
	MAME_DIR .. "src/mame/drivers/einstein.cpp",
	MAME_DIR .. "src/mame/includes/einstein.h",
	MAME_DIR .. "src/mame/machine/einstein.cpp",
}

createMESSProjects(_target, _subtarget, "teamconc")
files {
	MAME_DIR .. "src/mame/drivers/comquest.cpp",
	MAME_DIR .. "src/mame/includes/comquest.h",
	MAME_DIR .. "src/mame/video/comquest.cpp",
}

createMESSProjects(_target, _subtarget, "tektroni")
files {
	MAME_DIR .. "src/mame/drivers/tek405x.cpp",
	MAME_DIR .. "src/mame/includes/tek405x.h",
	MAME_DIR .. "src/mame/drivers/tek410x.cpp",
	MAME_DIR .. "src/mame/drivers/tekxp33x.cpp",
}

createMESSProjects(_target, _subtarget, "telenova")
files {
	MAME_DIR .. "src/mame/drivers/compis.cpp",
	MAME_DIR .. "src/mame/includes/compis.h",
	MAME_DIR .. "src/mame/machine/compiskb.cpp",
	MAME_DIR .. "src/mame/machine/compiskb.h",
}

createMESSProjects(_target, _subtarget, "telercas")
files {
	MAME_DIR .. "src/mame/drivers/tmc1800.cpp",
	MAME_DIR .. "src/mame/includes/tmc1800.h",
	MAME_DIR .. "src/mame/video/tmc1800.cpp",
	MAME_DIR .. "src/mame/drivers/tmc600.cpp",
	MAME_DIR .. "src/mame/includes/tmc600.h",
	MAME_DIR .. "src/mame/video/tmc600.cpp",
	MAME_DIR .. "src/mame/drivers/tmc2000e.cpp",
	MAME_DIR .. "src/mame/includes/tmc2000e.h",
}

createMESSProjects(_target, _subtarget, "televideo")
files {
	MAME_DIR .. "src/mame/drivers/ts802.cpp",
	MAME_DIR .. "src/mame/drivers/ts803.cpp",
	MAME_DIR .. "src/mame/drivers/ts816.cpp",
	MAME_DIR .. "src/mame/drivers/tv950.cpp",
}

createMESSProjects(_target, _subtarget, "tem")
files {
	MAME_DIR .. "src/mame/drivers/tec1.cpp",
}

createMESSProjects(_target, _subtarget, "tesla")
files {
	MAME_DIR .. "src/mame/drivers/ondra.cpp",
	MAME_DIR .. "src/mame/includes/ondra.h",
	MAME_DIR .. "src/mame/machine/ondra.cpp",
	MAME_DIR .. "src/mame/video/ondra.cpp",
	MAME_DIR .. "src/mame/drivers/pmd85.cpp",
	MAME_DIR .. "src/mame/includes/pmd85.h",
	MAME_DIR .. "src/mame/machine/pmd85.cpp",
	MAME_DIR .. "src/mame/drivers/pmi80.cpp",
	MAME_DIR .. "src/mame/drivers/sapi1.cpp",
}

createMESSProjects(_target, _subtarget, "test")
files {
	MAME_DIR .. "src/mame/drivers/test_t400.cpp",
	MAME_DIR .. "src/mame/drivers/zexall.cpp",
}

createMESSProjects(_target, _subtarget, "thomson")
files {
	MAME_DIR .. "src/mame/drivers/thomson.cpp",
	MAME_DIR .. "src/mame/includes/thomson.h",
	MAME_DIR .. "src/mame/machine/thomson.cpp",
	MAME_DIR .. "src/mame/machine/thomflop.cpp",
	MAME_DIR .. "src/mame/machine/thomflop.h",
	MAME_DIR .. "src/mame/video/thomson.cpp",
}

createMESSProjects(_target, _subtarget, "ti")
files {
	MAME_DIR .. "src/mame/drivers/avigo.cpp",
	MAME_DIR .. "src/mame/includes/avigo.h",
	MAME_DIR .. "src/mame/video/avigo.cpp",
	MAME_DIR .. "src/mame/drivers/cc40.cpp",
	MAME_DIR .. "src/mame/drivers/evmbug.cpp",
	MAME_DIR .. "src/mame/drivers/exelv.cpp",
	MAME_DIR .. "src/mame/drivers/geneve.cpp",
	MAME_DIR .. "src/mame/drivers/ticalc1x.cpp",
	MAME_DIR .. "src/mame/drivers/tispeak.cpp",
	MAME_DIR .. "src/mame/drivers/tispellb.cpp",
	MAME_DIR .. "src/mame/drivers/ti74.cpp",
	MAME_DIR .. "src/mame/drivers/ti85.cpp",
	MAME_DIR .. "src/mame/includes/ti85.h",
	MAME_DIR .. "src/mame/machine/ti85.cpp",
	MAME_DIR .. "src/mame/video/ti85.cpp",
	MAME_DIR .. "src/mame/drivers/ti89.cpp",
	MAME_DIR .. "src/mame/includes/ti89.h",
	MAME_DIR .. "src/mame/drivers/ti99_2.cpp",
	MAME_DIR .. "src/mame/drivers/ti99_4x.cpp",
	MAME_DIR .. "src/mame/drivers/ti99_4p.cpp",
	MAME_DIR .. "src/mame/drivers/ti99_8.cpp",
	MAME_DIR .. "src/mame/drivers/ti990_4.cpp",
	MAME_DIR .. "src/mame/drivers/ti990_10.cpp",
	MAME_DIR .. "src/mame/drivers/tm990189.cpp",
	MAME_DIR .. "src/mame/video/733_asr.cpp",
	MAME_DIR .. "src/mame/video/733_asr.h",
	MAME_DIR .. "src/mame/video/911_vdt.cpp",
	MAME_DIR .. "src/mame/video/911_vdt.h",
	MAME_DIR .. "src/mame/video/911_chr.h",
	MAME_DIR .. "src/mame/video/911_key.h",
	MAME_DIR .. "src/mame/drivers/hh_tms1k.cpp",
	MAME_DIR .. "src/mame/includes/hh_tms1k.h",
}

createMESSProjects(_target, _subtarget, "tiger")
files {
	MAME_DIR .. "src/mame/drivers/gamecom.cpp",
	MAME_DIR .. "src/mame/includes/gamecom.h",
	MAME_DIR .. "src/mame/machine/gamecom.cpp",
	MAME_DIR .. "src/mame/video/gamecom.cpp",
}

createMESSProjects(_target, _subtarget, "tigertel")
files {
	MAME_DIR .. "src/mame/drivers/gizmondo.cpp",
	MAME_DIR .. "src/mame/machine/docg3.cpp",
	MAME_DIR .. "src/mame/machine/docg3.h",
}

createMESSProjects(_target, _subtarget, "tiki")
files {
	MAME_DIR .. "src/mame/drivers/tiki100.cpp",
	MAME_DIR .. "src/mame/includes/tiki100.h",
}

createMESSProjects(_target, _subtarget, "tomy")
files {
	MAME_DIR .. "src/mame/drivers/tutor.cpp",
}

createMESSProjects(_target, _subtarget, "toshiba")
files {
	MAME_DIR .. "src/mame/drivers/pasopia.cpp",
	MAME_DIR .. "src/mame/includes/pasopia.h",
	MAME_DIR .. "src/mame/drivers/pasopia7.cpp",
	MAME_DIR .. "src/mame/drivers/paso1600.cpp",
}

createMESSProjects(_target, _subtarget, "trainer")
files {
	MAME_DIR .. "src/mame/drivers/amico2k.cpp",
	MAME_DIR .. "src/mame/drivers/babbage.cpp",
	MAME_DIR .. "src/mame/drivers/bob85.cpp",
	MAME_DIR .. "src/mame/drivers/cvicny.cpp",
	MAME_DIR .. "src/mame/drivers/dolphunk.cpp",
	MAME_DIR .. "src/mame/drivers/instruct.cpp",
	MAME_DIR .. "src/mame/drivers/mk14.cpp",
	MAME_DIR .. "src/mame/drivers/pro80.cpp",
	MAME_DIR .. "src/mame/drivers/savia84.cpp",
	MAME_DIR .. "src/mame/drivers/selz80.cpp",
	MAME_DIR .. "src/mame/drivers/tk80.cpp",
	MAME_DIR .. "src/mame/drivers/zapcomputer.cpp",
}

createMESSProjects(_target, _subtarget, "trs")
files {
	MAME_DIR .. "src/mame/drivers/coco12.cpp",
	MAME_DIR .. "src/mame/includes/coco12.h",
	MAME_DIR .. "src/mame/drivers/coco3.cpp",
	MAME_DIR .. "src/mame/includes/coco3.h",
	MAME_DIR .. "src/mame/drivers/dragon.cpp",
	MAME_DIR .. "src/mame/includes/dragon.h",
	MAME_DIR .. "src/mame/drivers/mc10.cpp",
	MAME_DIR .. "src/mame/machine/6883sam.cpp",
	MAME_DIR .. "src/mame/machine/6883sam.h",
	MAME_DIR .. "src/mame/machine/coco.cpp",
	MAME_DIR .. "src/mame/includes/coco.h",
	MAME_DIR .. "src/mame/machine/coco12.cpp",
	MAME_DIR .. "src/mame/machine/coco3.cpp",
	MAME_DIR .. "src/mame/machine/coco_vhd.cpp",
	MAME_DIR .. "src/mame/machine/coco_vhd.h",
	MAME_DIR .. "src/mame/machine/dragon.cpp",
	MAME_DIR .. "src/mame/machine/dgnalpha.cpp",
	MAME_DIR .. "src/mame/includes/dgnalpha.h",
	MAME_DIR .. "src/mame/video/gime.cpp",
	MAME_DIR .. "src/mame/video/gime.h",
	MAME_DIR .. "src/mame/drivers/trs80.cpp",
	MAME_DIR .. "src/mame/includes/trs80.h",
	MAME_DIR .. "src/mame/machine/trs80.cpp",
	MAME_DIR .. "src/mame/video/trs80.cpp",
	MAME_DIR .. "src/mame/drivers/trs80m2.cpp",
	MAME_DIR .. "src/mame/includes/trs80m2.h",
	MAME_DIR .. "src/mame/machine/trs80m2kb.cpp",
	MAME_DIR .. "src/mame/machine/trs80m2kb.h",
	MAME_DIR .. "src/mame/drivers/tandy2k.cpp",
	MAME_DIR .. "src/mame/includes/tandy2k.h",
	MAME_DIR .. "src/mame/machine/tandy2kb.cpp",
	MAME_DIR .. "src/mame/machine/tandy2kb.h",
}

createMESSProjects(_target, _subtarget, "ultimachine")
files {
	MAME_DIR .. "src/mame/drivers/rambo.cpp",
}

createMESSProjects(_target, _subtarget, "ultratec")
files {
	MAME_DIR .. "src/mame/drivers/minicom.cpp",
}

createMESSProjects(_target, _subtarget, "unisonic")
files {
	MAME_DIR .. "src/mame/drivers/unichamp.cpp",
	MAME_DIR .. "src/mame/video/gic.cpp",
	MAME_DIR .. "src/mame/video/gic.h",
	MAME_DIR .. "src/mame/video/gic.cpp",
	MAME_DIR .. "src/mame/video/gic.h",
}


createMESSProjects(_target, _subtarget, "unisys")
files {
	MAME_DIR .. "src/mame/drivers/univac.cpp",
}

createMESSProjects(_target, _subtarget, "usp")
files {
    MAME_DIR .. "src/mame/drivers/patinho_feio.cpp",
}

createMESSProjects(_target, _subtarget, "veb")
files {
	MAME_DIR .. "src/mame/drivers/chessmst.cpp",
	MAME_DIR .. "src/mame/drivers/kc.cpp",
	MAME_DIR .. "src/mame/includes/kc.h",
	MAME_DIR .. "src/mame/machine/kc.cpp",
	MAME_DIR .. "src/mame/machine/kc_keyb.cpp",
	MAME_DIR .. "src/mame/machine/kc_keyb.h",
	MAME_DIR .. "src/mame/video/kc.cpp",
	MAME_DIR .. "src/mame/drivers/lc80.cpp",
	MAME_DIR .. "src/mame/includes/lc80.h",
	MAME_DIR .. "src/mame/drivers/mc80.cpp",
	MAME_DIR .. "src/mame/includes/mc80.h",
	MAME_DIR .. "src/mame/machine/mc80.cpp",
	MAME_DIR .. "src/mame/video/mc80.cpp",
	MAME_DIR .. "src/mame/drivers/poly880.cpp",
	MAME_DIR .. "src/mame/includes/poly880.h",
	MAME_DIR .. "src/mame/drivers/sc1.cpp",
	MAME_DIR .. "src/mame/drivers/sc2.cpp",
}

createMESSProjects(_target, _subtarget, "vidbrain")
files {
	MAME_DIR .. "src/mame/drivers/vidbrain.cpp",
	MAME_DIR .. "src/mame/includes/vidbrain.h",
	MAME_DIR .. "src/mame/video/uv201.cpp",
	MAME_DIR .. "src/mame/video/uv201.h",
}

createMESSProjects(_target, _subtarget, "videoton")
files {
	MAME_DIR .. "src/mame/drivers/tvc.cpp",
	MAME_DIR .. "src/mame/audio/tvc_snd.cpp",
	MAME_DIR .. "src/mame/audio/tvc_snd.h",
}

createMESSProjects(_target, _subtarget, "visual")
files {
	MAME_DIR .. "src/mame/drivers/v1050.cpp",
	MAME_DIR .. "src/mame/includes/v1050.h",
	MAME_DIR .. "src/mame/machine/v1050kb.cpp",
	MAME_DIR .. "src/mame/machine/v1050kb.h",
	MAME_DIR .. "src/mame/video/v1050.cpp",
}

createMESSProjects(_target, _subtarget, "votrax")
files {
	MAME_DIR .. "src/mame/drivers/votrpss.cpp",
	MAME_DIR .. "src/mame/drivers/votrtnt.cpp",
}

createMESSProjects(_target, _subtarget, "vtech")
files {
	MAME_DIR .. "src/mame/drivers/crvision.cpp",
	MAME_DIR .. "src/mame/includes/crvision.h",
	MAME_DIR .. "src/mame/drivers/geniusiq.cpp",
	MAME_DIR .. "src/mame/drivers/laser3k.cpp",
	MAME_DIR .. "src/mame/drivers/lcmate2.cpp",
	MAME_DIR .. "src/mame/drivers/pc4.cpp",
	MAME_DIR .. "src/mame/includes/pc4.h",
	MAME_DIR .. "src/mame/video/pc4.cpp",
	MAME_DIR .. "src/mame/drivers/pc2000.cpp",
	MAME_DIR .. "src/mame/drivers/pitagjr.cpp",
	MAME_DIR .. "src/mame/drivers/prestige.cpp",
	MAME_DIR .. "src/mame/drivers/vtech1.cpp",
	MAME_DIR .. "src/mame/drivers/vtech2.cpp",
	MAME_DIR .. "src/mame/includes/vtech2.h",
	MAME_DIR .. "src/mame/machine/vtech2.cpp",
	MAME_DIR .. "src/mame/video/vtech2.cpp",
	MAME_DIR .. "src/mame/drivers/socrates.cpp",
	MAME_DIR .. "src/mame/audio/socrates.cpp",
	MAME_DIR .. "src/mame/audio/socrates.h",
}

createMESSProjects(_target, _subtarget, "wang")
files {
	MAME_DIR .. "src/mame/drivers/wangpc.cpp",
	MAME_DIR .. "src/mame/includes/wangpc.h",
	MAME_DIR .. "src/mame/machine/wangpckb.cpp",
	MAME_DIR .. "src/mame/machine/wangpckb.h",
}

createMESSProjects(_target, _subtarget, "wavemate")
files {
	MAME_DIR .. "src/mame/drivers/bullet.cpp",
	MAME_DIR .. "src/mame/includes/bullet.h",
	MAME_DIR .. "src/mame/drivers/jupiter.cpp",
	MAME_DIR .. "src/mame/includes/jupiter.h",
}

createMESSProjects(_target, _subtarget, "xerox")
files {
	MAME_DIR .. "src/mame/drivers/xerox820.cpp",
	MAME_DIR .. "src/mame/includes/xerox820.h",
	MAME_DIR .. "src/mame/machine/x820kb.cpp",
	MAME_DIR .. "src/mame/machine/x820kb.h",
	MAME_DIR .. "src/mame/drivers/bigbord2.cpp",
	MAME_DIR .. "src/mame/drivers/alto2.cpp",
}

createMESSProjects(_target, _subtarget, "xussrpc")
files {
	MAME_DIR .. "src/mame/drivers/ec184x.cpp",
	MAME_DIR .. "src/mame/includes/ec184x.h",
	MAME_DIR .. "src/mame/drivers/iskr103x.cpp",
	MAME_DIR .. "src/mame/drivers/mc1502.cpp",
	MAME_DIR .. "src/mame/machine/kb_7007_3.h",
	MAME_DIR .. "src/mame/includes/mc1502.h",
	MAME_DIR .. "src/mame/drivers/poisk1.cpp",
	MAME_DIR .. "src/mame/machine/kb_poisk1.h",
	MAME_DIR .. "src/mame/includes/poisk1.h",
	MAME_DIR .. "src/mame/video/poisk1.cpp",
	MAME_DIR .. "src/mame/video/poisk1.h",
}

createMESSProjects(_target, _subtarget, "yamaha")
files {
	MAME_DIR .. "src/mame/drivers/ymmu100.cpp",
	MAME_DIR .. "src/mame/drivers/fb01.cpp",
}
dependency {
	{ MAME_DIR .. "src/mame/drivers/ymmu100.cpp",    GEN_DIR .. "mame/drivers/ymmu100.inc" },
}
custombuildtask {
	{ MAME_DIR .. "src/mame/drivers/ymmu100.ppm", GEN_DIR .. "mame/drivers/ymmu100.inc",  {  MAME_DIR .. "scripts/build/file2str.py" }, {"@echo Converting src/drivers/ymmu100.ppm...", PYTHON .. " $(1) $(<) $(@) ymmu100_bkg UINT8" }},
}

createMESSProjects(_target, _subtarget, "zenith")
files {
	MAME_DIR .. "src/mame/drivers/z100.cpp",
}

createMESSProjects(_target, _subtarget, "zpa")
files {
	MAME_DIR .. "src/mame/drivers/iq151.cpp",
}

createMESSProjects(_target, _subtarget, "zvt")
files {
	MAME_DIR .. "src/mame/drivers/pp01.cpp",
	MAME_DIR .. "src/mame/includes/pp01.h",
	MAME_DIR .. "src/mame/machine/pp01.cpp",
	MAME_DIR .. "src/mame/video/pp01.cpp",
}

createMESSProjects(_target, _subtarget, "skeleton")
files {
	MAME_DIR .. "src/mame/drivers/alphasma.cpp",
	MAME_DIR .. "src/mame/drivers/ampro.cpp",
	MAME_DIR .. "src/mame/drivers/amust.cpp",
	MAME_DIR .. "src/mame/drivers/applix.cpp",
	MAME_DIR .. "src/mame/drivers/attache.cpp",
	MAME_DIR .. "src/mame/drivers/aussiebyte.cpp",
	MAME_DIR .. "src/mame/includes/aussiebyte.h",
	MAME_DIR .. "src/mame/video/aussiebyte.cpp",
	MAME_DIR .. "src/mame/drivers/ax20.cpp",
	MAME_DIR .. "src/mame/drivers/beehive.cpp",
	MAME_DIR .. "src/mame/drivers/binbug.cpp",
	MAME_DIR .. "src/mame/drivers/besta.cpp",
	MAME_DIR .. "src/mame/drivers/bitgraph.cpp",
	MAME_DIR .. "src/mame/drivers/br8641.cpp",
	MAME_DIR .. "src/mame/drivers/busicom.cpp",
	MAME_DIR .. "src/mame/includes/busicom.h",
	MAME_DIR .. "src/mame/video/busicom.cpp",
	MAME_DIR .. "src/mame/drivers/chaos.cpp",
	MAME_DIR .. "src/mame/drivers/chesstrv.cpp",
	MAME_DIR .. "src/mame/drivers/cd2650.cpp",
	MAME_DIR .. "src/mame/drivers/cdc721.cpp",
	MAME_DIR .. "src/mame/drivers/codata.cpp",
	MAME_DIR .. "src/mame/drivers/cortex.cpp",
	MAME_DIR .. "src/mame/drivers/cosmicos.cpp",
	MAME_DIR .. "src/mame/includes/cosmicos.h",
	MAME_DIR .. "src/mame/drivers/cp1.cpp",
	MAME_DIR .. "src/mame/drivers/cxhumax.cpp",
	MAME_DIR .. "src/mame/includes/cxhumax.h",
	MAME_DIR .. "src/mame/drivers/czk80.cpp",
	MAME_DIR .. "src/mame/drivers/d6800.cpp",
	MAME_DIR .. "src/mame/drivers/d6809.cpp",
	MAME_DIR .. "src/mame/drivers/daruma.cpp",
	MAME_DIR .. "src/mame/drivers/didact.cpp",
	MAME_DIR .. "src/mame/drivers/digel804.cpp",
	MAME_DIR .. "src/mame/drivers/dim68k.cpp",
	MAME_DIR .. "src/mame/drivers/dm7000.cpp",
	MAME_DIR .. "src/mame/includes/dm7000.h",
	MAME_DIR .. "src/mame/drivers/dmv.cpp",
	MAME_DIR .. "src/mame/machine/dmv_keyb.cpp",
	MAME_DIR .. "src/mame/machine/dmv_keyb.h",
	MAME_DIR .. "src/mame/drivers/dps1.cpp",
	MAME_DIR .. "src/mame/drivers/dsb46.cpp",
	MAME_DIR .. "src/mame/drivers/dual68.cpp",
	MAME_DIR .. "src/mame/drivers/eacc.cpp",
	MAME_DIR .. "src/mame/drivers/elwro800.cpp",
	MAME_DIR .. "src/mame/drivers/eti660.cpp",
	MAME_DIR .. "src/mame/includes/eti660.h",
	MAME_DIR .. "src/mame/drivers/excali64.cpp",
	MAME_DIR .. "src/mame/drivers/fanucs15.cpp",
	MAME_DIR .. "src/mame/drivers/fanucspmg.cpp",
	MAME_DIR .. "src/mame/drivers/fc100.cpp",
	MAME_DIR .. "src/mame/drivers/fk1.cpp",
	MAME_DIR .. "src/mame/drivers/ft68m.cpp",
	MAME_DIR .. "src/mame/drivers/gamate.cpp",
	MAME_DIR .. "src/mame/includes/gamate.h",
	MAME_DIR .. "src/mame/audio/gamate.cpp",
	MAME_DIR .. "src/mame/drivers/gameking.cpp",
	MAME_DIR .. "src/mame/drivers/gimix.cpp",
	MAME_DIR .. "src/mame/drivers/grfd2301.cpp",
	MAME_DIR .. "src/mame/drivers/harriet.cpp",
	MAME_DIR .. "src/mame/drivers/hprot1.cpp",
	MAME_DIR .. "src/mame/drivers/hpz80unk.cpp",
	MAME_DIR .. "src/mame/drivers/ht68k.cpp",
	MAME_DIR .. "src/mame/drivers/hunter2.cpp",
	MAME_DIR .. "src/mame/drivers/i7000.cpp",
	MAME_DIR .. "src/mame/drivers/ibm6580.cpp",
	MAME_DIR .. "src/mame/drivers/icatel.cpp",
	MAME_DIR .. "src/mame/drivers/ie15.cpp",
	MAME_DIR .. "src/mame/machine/ie15_kbd.cpp",
	MAME_DIR .. "src/mame/machine/ie15_kbd.h",
	MAME_DIR .. "src/mame/drivers/if800.cpp",
	MAME_DIR .. "src/mame/drivers/imsai.cpp",
	MAME_DIR .. "src/mame/drivers/indiana.cpp",
	MAME_DIR .. "src/mame/drivers/itt3030.cpp",
	MAME_DIR .. "src/mame/drivers/jade.cpp",
	MAME_DIR .. "src/mame/drivers/jonos.cpp",
	MAME_DIR .. "src/mame/drivers/konin.cpp",
	MAME_DIR .. "src/mame/drivers/leapster.cpp",
	MAME_DIR .. "src/mame/drivers/lft.cpp",
	MAME_DIR .. "src/mame/drivers/lg-dvd.cpp",
	MAME_DIR .. "src/mame/drivers/lola8a.cpp",
    MAME_DIR .. "src/mame/drivers/m79152pc.cpp",
    MAME_DIR .. "src/mame/drivers/marywu.cpp",
	MAME_DIR .. "src/mame/drivers/mccpm.cpp",
	MAME_DIR .. "src/mame/drivers/mes.cpp",
	MAME_DIR .. "src/mame/drivers/mice.cpp",
	MAME_DIR .. "src/mame/drivers/micral.cpp",
	MAME_DIR .. "src/mame/drivers/micronic.cpp",
	MAME_DIR .. "src/mame/includes/micronic.h",
	MAME_DIR .. "src/mame/drivers/mini2440.cpp",
	MAME_DIR .. "src/mame/drivers/mmd1.cpp",
	MAME_DIR .. "src/mame/drivers/mod8.cpp",
	MAME_DIR .. "src/mame/drivers/modellot.cpp",
	MAME_DIR .. "src/mame/drivers/molecular.cpp",
	MAME_DIR .. "src/mame/drivers/ms0515.cpp",
	MAME_DIR .. "src/mame/drivers/ms9540.cpp",
	MAME_DIR .. "src/mame/drivers/mstation.cpp",
	MAME_DIR .. "src/mame/drivers/mt735.cpp",
	MAME_DIR .. "src/mame/drivers/mx2178.cpp",
	MAME_DIR .. "src/mame/drivers/mycom.cpp",
	MAME_DIR .. "src/mame/drivers/myvision.cpp",
	MAME_DIR .. "src/mame/drivers/ngen.cpp",
	MAME_DIR .. "src/mame/machine/ngen_kb.cpp",
	MAME_DIR .. "src/mame/machine/ngen_kb.h",
	MAME_DIR .. "src/mame/drivers/octopus.cpp",
	MAME_DIR .. "src/mame/drivers/onyx.cpp",
	MAME_DIR .. "src/mame/drivers/okean240.cpp",
	MAME_DIR .. "src/mame/drivers/p8k.cpp",
	MAME_DIR .. "src/mame/drivers/pegasus.cpp",
	MAME_DIR .. "src/mame/drivers/pencil2.cpp",
	MAME_DIR .. "src/mame/drivers/pes.cpp",
	MAME_DIR .. "src/mame/includes/pes.h",
	MAME_DIR .. "src/mame/drivers/pipbug.cpp",
	MAME_DIR .. "src/mame/drivers/plan80.cpp",
	MAME_DIR .. "src/mame/drivers/pm68k.cpp",
	MAME_DIR .. "src/mame/drivers/poly.cpp",
	MAME_DIR .. "src/mame/drivers/proteus3.cpp",
	MAME_DIR .. "src/mame/drivers/pt68k4.cpp",
	MAME_DIR .. "src/mame/drivers/ptcsol.cpp",
	MAME_DIR .. "src/mame/drivers/pulsar.cpp",
	MAME_DIR .. "src/mame/drivers/pv9234.cpp",
	MAME_DIR .. "src/mame/drivers/qtsbc.cpp",
	MAME_DIR .. "src/mame/drivers/rd100.cpp",
	MAME_DIR .. "src/mame/drivers/rvoice.cpp",
	MAME_DIR .. "src/mame/drivers/sacstate.cpp",
	MAME_DIR .. "src/mame/drivers/sbrain.cpp",
	MAME_DIR .. "src/mame/drivers/seattlecmp.cpp",
	MAME_DIR .. "src/mame/drivers/sh4robot.cpp",
	MAME_DIR .. "src/mame/drivers/sansa_fuze.cpp",
	MAME_DIR .. "src/mame/drivers/softbox.cpp",
	MAME_DIR .. "src/mame/includes/softbox.h",
	MAME_DIR .. "src/mame/drivers/squale.cpp",
	MAME_DIR .. "src/mame/drivers/swtpc.cpp",
	MAME_DIR .. "src/mame/drivers/swyft.cpp",
	MAME_DIR .. "src/mame/drivers/sys2900.cpp",
	MAME_DIR .. "src/mame/drivers/systec.cpp",
	MAME_DIR .. "src/mame/drivers/tavernie.cpp",
	MAME_DIR .. "src/mame/drivers/tecnbras.cpp",
	MAME_DIR .. "src/mame/drivers/terak.cpp",
	MAME_DIR .. "src/mame/drivers/ti630.cpp",
	MAME_DIR .. "src/mame/drivers/tsispch.cpp",
	MAME_DIR .. "src/mame/includes/tsispch.h",
	MAME_DIR .. "src/mame/drivers/tvgame.cpp",
	MAME_DIR .. "src/mame/drivers/unistar.cpp",
	MAME_DIR .. "src/mame/drivers/v6809.cpp",
	MAME_DIR .. "src/mame/drivers/vector4.cpp",
	MAME_DIR .. "src/mame/drivers/vii.cpp",
	MAME_DIR .. "src/mame/drivers/wicat.cpp",
	MAME_DIR .. "src/mame/drivers/xor100.cpp",
	MAME_DIR .. "src/mame/includes/xor100.h",
	MAME_DIR .. "src/mame/drivers/xavix.cpp",
	MAME_DIR .. "src/mame/drivers/zorba.cpp",
}

end
