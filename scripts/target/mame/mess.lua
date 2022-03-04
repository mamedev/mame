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
--CPUS["KC80"] = true
CPUS["Z180"] = true
CPUS["I8085"] = true
CPUS["I8089"] = true
CPUS["M6502"] = true
CPUS["ST2XXX"] = true
CPUS["XAVIX"] = true
CPUS["XAVIX2000"] = true
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
CPUS["MIPS1"] = true
CPUS["MIPS3"] = true
CPUS["PSX"] = true
CPUS["SH"] = true
CPUS["DSP16"] = true
CPUS["DSP32C"] = true
CPUS["PIC16C5X"] = true
CPUS["PIC16C62X"] = true
CPUS["PIC17"] = true
CPUS["G65816"] = true
CPUS["SPC700"] = true
CPUS["E1"] = true
CPUS["I860"] = true
CPUS["I960"] = true
CPUS["H16"] = true
CPUS["H8"] = true
CPUS["H8500"] = true
CPUS["V810"] = true
CPUS["M37710"] = true
CPUS["POWERPC"] = true
CPUS["SE3208"] = true
CPUS["MC68HC11"] = true
CPUS["ADSP21062"] = true
CPUS["DSP56156"] = true
CPUS["RSP"] = true
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
CPUS["PDP8"] = true
CPUS["TX0"] = true
CPUS["SATURN"] = true
CPUS["SC61860"] = true
CPUS["LR35902"] = true
CPUS["TMS7000"] = true
CPUS["SM8500"] = true
CPUS["MINX"] = true
CPUS["SSEM"] = true
CPUS["DIABLO"] = true
CPUS["AVR8"] = true
CPUS["TMS1000"] = true
CPUS["MCS40"] = true
CPUS["SUPERFX"] = true
CPUS["Z8"] = true
CPUS["I8008"] = true
CPUS["SCMP"] = true
CPUS["MN1880"] = true
--CPUS["MN10200"] = true
CPUS["COSMAC"] = true
CPUS["UNSP"] = true
CPUS["HCD62121"] = true
CPUS["PPS4"] = true
CPUS["PPS41"] = true
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
CPUS["SPARC"] = true
CPUS["NANOPROCESSOR"] = true
CPUS["CLIPPER"] = true
CPUS["CAPRICORN"] = true
CPUS["ALPHA"] = true
CPUS["NS32000"] = true
--CPUS["DSPP"] = true
CPUS["HPC"] = true
CPUS["MEG"] = true
CPUS["DSPV"] = true
CPUS["RII"] = true
CPUS["BCP"] = true
CPUS["F2MC16"] = true
CPUS["CR16B"] = true
CPUS["FR"] = true
CPUS["DSP56000"] = true
CPUS["VT50"] = true
CPUS["VT61"] = true
CPUS["PACE"] = true
CPUS["WE32000"] = true
CPUS["RX01"] = true
CPUS["GTRON"] = true
CPUS["M88000"] = true
CPUS["XAVIX2"] = true
CPUS["UPD78K"] = true
CPUS["ROMP"] = true
CPUS["COPS1"] = true
CPUS["MK1"] = true
CPUS["M68HC16"] = true

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
SOUNDS["YM2154"] = true
SOUNDS["YM2151"] = true
SOUNDS["YM2414"] = true
SOUNDS["YM3806"] = true
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
SOUNDS["YMF271"] = true
SOUNDS["YMF278B"] = true
SOUNDS["YMZ280B"] = true
SOUNDS["SN76477"] = true
SOUNDS["SN76496"] = true
SOUNDS["POKEY"] = true
SOUNDS["TIA"] = true
SOUNDS["NES_APU"] = true
SOUNDS["PAULA_8364"] = true
SOUNDS["ASTROCADE"] = true
--SOUNDS["NAMCO"] = true
--SOUNDS["NAMCO_15XX"] = true
--SOUNDS["NAMCO_CUS30"] = true
--SOUNDS["NAMCO_52XX"] = true
--SOUNDS["NAMCO_63701X"] = true
SOUNDS["NAMCO_163"] = true
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
SOUNDS["MULTIPCM"] = true
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
SOUNDS["HUC6230"] = true
SOUNDS["C6280"] = true
SOUNDS["SP0250"] = true
SOUNDS["SPU"] = true
SOUNDS["CDDA"] = true
--SOUNDS["ICS2115"] = true
--SOUNDS["I5000_SND"] = true
--SOUNDS["ST0016"] = true
--SOUNDS["SETAPCM"] = true
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
--SOUNDS["MPEG_AUDIO"] = true
SOUNDS["T6721A"] = true
SOUNDS["MOS7360"] = true
SOUNDS["ESQPUMP"] = true
SOUNDS["VRC6"] = true
--SOUNDS["SB0400"] = true
--SOUNDS["AC97"] = true
--SOUNDS["ES1373"] = true
SOUNDS["L7A1045"] = true
SOUNDS["AD1848"] = true
SOUNDS["UPD1771"] = true
SOUNDS["GB_SOUND"] = true
SOUNDS["PCD3311"] = true
SOUNDS["MEA8000"] = true
--SOUNDS["DAC76"] = true
--SOUNDS["MM5837"] = true
SOUNDS["DAVE"] = true
--SOUNDS["LC7535"] = true
SOUNDS["UPD934G"] = true
SOUNDS["IOPSPU"] = true
SOUNDS["SWP00"] = true
SOUNDS["SWP20"] = true
SOUNDS["SWP30"] = true
--SOUNDS["XT446"] = true
SOUNDS["S_DSP"] = true
SOUNDS["ROLANDPCM"] = true
--SOUNDS["TT5665"] = true
SOUNDS["RP2C33_SOUND"] = true
SOUNDS["UDA1344"] = true
SOUNDS["LYNX"] = true

--------------------------------------------------
-- specify available video cores
--------------------------------------------------

VIDEOS["SEGA315_5124"] = true
VIDEOS["SEGA315_5313"] = true
VIDEOS["AM8052"] = true
--VIDEOS["BUFSPRITE"] = true
VIDEOS["BT45X"] = true
VIDEOS["BT459"] = true
VIDEOS["BT47X"] = true
VIDEOS["CATSEYE"] = true
VIDEOS["CDP1861"] = true
VIDEOS["CDP1862"] = true
--VIDEOS["CESBLIT"] = true
VIDEOS["CRT9007"] = true
VIDEOS["CRT9021"] = true
VIDEOS["CRT9028"] = true
VIDEOS["CRT9212"] = true
VIDEOS["CRTC_EGA"] = true
VIDEOS["DL1416"] = true
VIDEOS["DM9368"] = true
VIDEOS["DP8350"] = true
VIDEOS["EF9340_1"] = true
VIDEOS["EF9345"] = true
VIDEOS["EF9364"] = true
VIDEOS["EF9365"] = true
--VIDEOS["EF9369"] = true
VIDEOS["FIXFREQ"] = true
VIDEOS["GF4500"] = true
--VIDEOS["EPIC12"] = true
VIDEOS["NT7534"] = true
VIDEOS["HD44102"] = true
VIDEOS["HD44352"] = true
VIDEOS["HD44780"] = true
VIDEOS["HD61202"] = true
VIDEOS["HD61603"] = true
VIDEOS["HD61830"] = true
VIDEOS["HD63484"] = true
VIDEOS["HD66421"] = true
VIDEOS["HLCD0438"] = true
VIDEOS["HLCD0488"] = true
VIDEOS["HLCD0515"] = true
VIDEOS["HLCD0538"] = true
VIDEOS["HP1LL3"] = true
VIDEOS["HUC6202"] = true
VIDEOS["HUC6260"] = true
VIDEOS["HUC6261"] = true
VIDEOS["HUC6270"] = true
VIDEOS["HUC6271"] = true
VIDEOS["HUC6272"] = true
VIDEOS["I8244"] = true
VIDEOS["I82730"] = true
VIDEOS["I8275"] = true
VIDEOS["IMS_CVC"] = true
VIDEOS["LC7582"] = true
VIDEOS["LC7985"] = true
--VIDEOS["M50458"] = true
--VIDEOS["MB90082"] = true
--VIDEOS["MB_VCU"] = true
VIDEOS["MC6845"] = true
VIDEOS["MC6847"] = true
VIDEOS["MD4330B"] = true
VIDEOS["MM5445"] = true
VIDEOS["MSM6222B"] = true
VIDEOS["MSM6255"] = true
VIDEOS["MOS6566"] = true
VIDEOS["PC_VGA"] = true
VIDEOS["PCD8544"] = true
VIDEOS["PCF2100"] = true
--VIDEOS["POLY"] = true
VIDEOS["PSX"] = true
VIDEOS["RAMDAC"] = true
VIDEOS["S2636"] = true
VIDEOS["SAA5050"] = true
VIDEOS["SAA5240"] = true
VIDEOS["PWM_DISPLAY"] = true
VIDEOS["SDA5708"] = true
VIDEOS["SED1200"] = true
VIDEOS["SED1330"] = true
VIDEOS["SED1356"] = true
VIDEOS["SED1500"] = true
VIDEOS["SED1520"] = true
VIDEOS["SNES_PPU"] = true
VIDEOS["T6963C"] = true
VIDEOS["T6A04"] = true
VIDEOS["TEA1002"] = true
--VIDEOS["TLC34076"] = true
--VIDEOS["TMS34061"] = true
VIDEOS["TMS3556"] = true
VIDEOS["TMS9927"] = true
VIDEOS["TMS9928A"] = true
VIDEOS["TOPCAT"] = true
VIDEOS["NEREID"] = true
VIDEOS["UPD3301"] = true
VIDEOS["UPD7220"] = true
VIDEOS["UPD7227"] = true
VIDEOS["V9938"] = true
VIDEOS["VIC4567"] = true
VIDEOS["VIRGE_PCI"] = true
--VIDEOS["VOODOO"] = true
VIDEOS["SCN2674"] = true
VIDEOS["GB_LCD"] = true
VIDEOS["GBA_LCD"] = true
VIDEOS["MGA2064W"] = true
VIDEOS["PPU2C0X"] = true
VIDEOS["DP8510"] = true
VIDEOS["MB88303"] = true
VIDEOS["PS2GS"] = true
VIDEOS["PS2GIF"] = true
VIDEOS["DECSFB"] = true
VIDEOS["BT431"] = true
--VIDEOS["VRENDER0"] = true

--------------------------------------------------
-- specify available machine cores
--------------------------------------------------

MACHINES["AKIKO"] = true
MACHINES["ALPHA_8921"] = true
MACHINES["AM2901B"] = true
MACHINES["AUTOCONFIG"] = true
MACHINES["BUSMOUSE"] = true
MACHINES["CR511B"] = true
MACHINES["DMAC"] = true
MACHINES["GAYLE"] = true
MACHINES["6522VIA"] = true
MACHINES["6821PIA"] = true
MACHINES["6840PTM"] = true
MACHINES["MPCC68561"] = true
MACHINES["68681"] = true
MACHINES["7200FIFO"] = true
MACHINES["8530SCC"] = true
MACHINES["ACIA6850"] = true
MACHINES["ACORN_BMU"] = true
MACHINES["ACORN_IOC"] = true
MACHINES["ACORN_LC"] = true
MACHINES["ACORN_MEMC"] = true
MACHINES["ACORN_VIDC"] = true
MACHINES["ADC0804"] = true
MACHINES["ADC0808"] = true
MACHINES["ADC083X"] = true
MACHINES["ADC1038"] = true
MACHINES["ADC1213X"] = true
MACHINES["AICARTC"] = true
MACHINES["AM25S55X"] = true
MACHINES["AM2847"] = true
MACHINES["AM2910"] = true
MACHINES["AM53CF96"] = true
MACHINES["AM79C30"] = true
MACHINES["AM79C90"] = true
MACHINES["AM9513"] = true
MACHINES["AM9517A"] = true
MACHINES["AM9519"] = true
MACHINES["AMIGAFDC"] = true
MACHINES["AMIGA_COPPER"] = true
MACHINES["APPLEPIC"] = true
MACHINES["ARM_IOMD"] = true
MACHINES["AT_KEYBC"] = true
MACHINES["AT28C16"] = true
MACHINES["AT28C64B"] = true
MACHINES["AT29X"] = true
MACHINES["AT45DBXX"] = true
MACHINES["ATAFLASH"] = true
MACHINES["AY31015"] = true
MACHINES["BANKDEV"] = true
MACHINES["BIM68153"] = true
MACHINES["BQ4847"] = true
MACHINES["BQ4852"] = true
MACHINES["CDP1852"] = true
MACHINES["CDP1871"] = true
MACHINES["CDP1879"] = true
--MACHINES["CDU76S"] = true
MACHINES["CH376"] = true
MACHINES["CHESSMACHINE"] = true
MACHINES["CMOS40105"] = true
MACHINES["COM52C50"] = true
MACHINES["COM8116"] = true
MACHINES["COP452"] = true
MACHINES["CR589"] = true
MACHINES["CS4031"] = true
MACHINES["CS8221"] = true
MACHINES["CS8900A"] = true
MACHINES["CXD1095"] = true
MACHINES["DL11"] = true
MACHINES["DP8390"] = true
MACHINES["DP83932C"] = true
MACHINES["DP8573"] = true
--MACHINES["DS1204"] = true
MACHINES["DS1302"] = true
MACHINES["DS1315"] = true
MACHINES["DS1386"] = true
MACHINES["DS17X85"] = true
MACHINES["DS2401"] = true
MACHINES["DS2404"] = true
MACHINES["DS6417"] = true
MACHINES["DS75160A"] = true
MACHINES["DS75161A"] = true
MACHINES["DS8874"] = true
MACHINES["E0516"] = true
MACHINES["E05A03"] = true
MACHINES["E05A30"] = true
MACHINES["EEPROMDEV"] = true
MACHINES["ER1400"] = true
MACHINES["ER2055"] = true
MACHINES["EXORTERM"] = true
MACHINES["F3853"] = true
MACHINES["F4702"] = true
MACHINES["GLUKRS"] = true
MACHINES["GT913"] = true
MACHINES["HD63450"] = true
MACHINES["HD64610"] = true
MACHINES["HP_DC100_TAPE"] = true
MACHINES["HP_TACO"] = true
MACHINES["1MA6"] = true
MACHINES["1MB5"] = true
MACHINES["I2CMEM"] = true
MACHINES["I3001"] = true
MACHINES["I3002"] = true
MACHINES["I7220"] = true
MACHINES["I80130"] = true
MACHINES["I8087"] = true
MACHINES["I8155"] = true
MACHINES["I8212"] = true
MACHINES["I8214"] = true
MACHINES["I82355"] = true
MACHINES["I8243"] = true
MACHINES["I8251"] = true
MACHINES["I8255"] = true
MACHINES["I8257"] = true
MACHINES["I8271"] = true
MACHINES["I8279"] = true
MACHINES["I8291A"] = true
MACHINES["I8355"] = true
--MACHINES["IBM21S850"] = true
MACHINES["ICM7170"] = true
MACHINES["IDECTRL"] = true
MACHINES["IE15"] = true
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
MACHINES["LOCOMO"] = true
MACHINES["LPCI"] = true
MACHINES["LSI53C810"] = true
MACHINES["M3002"] = true
MACHINES["M68307"] = true
MACHINES["M68340"] = true
MACHINES["M950X0"] = true
MACHINES["M68SFDC"] = true
MACHINES["M6M80011AP"] = true
MACHINES["MB14241"] = true
MACHINES["MB3773"] = true
MACHINES["MB8421"] = true
MACHINES["MB87030"] = true
MACHINES["MB87078"] = true
MACHINES["MB8795"] = true
MACHINES["MB89352"] = true
MACHINES["MB89371"] = true
MACHINES["MC14411"] = true
MACHINES["MC146818"] = true
MACHINES["MC6843"] = true
MACHINES["MC6844"] = true
MACHINES["MC6846"] = true
MACHINES["MC6852"] = true
MACHINES["MC6854"] = true
MACHINES["MC68328"] = true
MACHINES["MC68901"] = true
MACHINES["MCCS1850"] = true
MACHINES["MCF5206E"] = true
MACHINES["MICROTOUCH"] = true
MACHINES["MIOT6530"] = true
MACHINES["MM5307"] = true
MACHINES["MM58167"] = true
MACHINES["MM58174"] = true
MACHINES["MM58274C"] = true
MACHINES["MM74C922"] = true
MACHINES["MM5740"] = true
MACHINES["MOS6526"] = true
MACHINES["MOS6529"] = true
MACHINES["MOS6551"] = true
MACHINES["MOS6702"] = true
MACHINES["MOS8706"] = true
MACHINES["MOS8722"] = true
MACHINES["MOS8726"] = true
MACHINES["MPU401"] = true
MACHINES["MSM5832"] = true
MACHINES["MSM58321"] = true
MACHINES["MSM6242"] = true
--MACHINES["MSM6253"] = true
MACHINES["MYB3K_KEYBOARD"] = true
MACHINES["NCR5380"] = true
MACHINES["NCR5385"] = true
MACHINES["NCR5390"] = true
MACHINES["NCR539x"] = true
MACHINES["NCR53C7XX"] = true
MACHINES["NETLIST"] = true
MACHINES["NMC9306"] = true
MACHINES["NSC810"] = true
MACHINES["NSCSI"] = true
MACHINES["OMTI5100"] = true
MACHINES["OUTPUT_LATCH"] = true
MACHINES["PC_FDC"] = true
MACHINES["PC_LPT"] = true
MACHINES["PCCARD"] = true
MACHINES["PCF8573"] = true
MACHINES["PCF8583"] = true
--MACHINES["PCF8584"] = true
MACHINES["PCF8593"] = true
MACHINES["PCI"] = true
MACHINES["PCKEYBRD"] = true
MACHINES["PDC"] = true
MACHINES["PHI"] = true
MACHINES["PIC8259"] = true
MACHINES["PIT68230"] = true
MACHINES["PIT8253"] = true
MACHINES["PLA"] = true
--MACHINES["PROFILE"] = true
MACHINES["PROM82S129"] = true
MACHINES["PXA255"] = true
MACHINES["R64H156"] = true
MACHINES["RF5C296"] = true
MACHINES["RIOT6532"] = true
MACHINES["RIPPLE_COUNTER"] = true
MACHINES["ROC10937"] = true
MACHINES["RP5C01"] = true
MACHINES["RP5C15"] = true
MACHINES["RP5H01"] = true
MACHINES["RSTBUF"] = true
MACHINES["RTC4543"] = true
MACHINES["RTC65271"] = true
MACHINES["RTC9701"] = true
MACHINES["S_SMP"] = true
--MACHINES["S2636"] = true
MACHINES["S3520CF"] = true
MACHINES["S3C24XX"] = true
MACHINES["S3C44B0"] = true
MACHINES["SA1110"] = true
MACHINES["SA1111"] = true
MACHINES["SAA1043"] = true
MACHINES["SATURN"] = true
MACHINES["SCC68070"] = true
--MACHINES["SCSI"] = true
MACHINES["SCC2698B"] = true
MACHINES["SCN_PCI"] = true
MACHINES["SCOOP"] = true
MACHINES["SCUDSP"] = true
MACHINES["SECFLASH"] = true
MACHINES["SEIBU_COP"] = true
MACHINES["SENSORBOARD"] = true
--MACHINES["SERFLASH"] = true
MACHINES["SMC91C9X"] = true
MACHINES["SMIOC"] = true
MACHINES["SEGA_SCU"] = true
MACHINES["SMPC"] = true
MACHINES["SPG2XX"] = true
MACHINES["SPG290"] = true
MACHINES["SPISDCARD"] = true
MACHINES["STVCD"] = true
MACHINES["SUN4C_MMU"] = true
MACHINES["SWTPC8212"] = true
MACHINES["TASC_SB30"] = true
MACHINES["TC0091LVC"] = true
MACHINES["TDC1008"] = true
--MACHINES["TE7750"] = true
MACHINES["TIMEKPR"] = true
MACHINES["TMC0430"] = true
MACHINES["TMC0999"] = true
MACHINES["TMC208K"] = true
MACHINES["TMP68301"] = true
MACHINES["TMS5501"] = true
MACHINES["TMS6100"] = true
MACHINES["TMS9901"] = true
MACHINES["TMS9902"] = true
MACHINES["TMS9914"] = true
MACHINES["TPI6525"] = true
MACHINES["TSCONF_DMA"] = true
MACHINES["TTL7400"] = true
MACHINES["TTL7404"] = true
--MACHINES["TSB12LV01A"] = true
MACHINES["TTL74123"] = true
MACHINES["TTL74145"] = true
MACHINES["TTL74148"] = true
MACHINES["TTL74153"] = true
MACHINES["TTL74157"] = true
MACHINES["TTL74161"] = true
MACHINES["TTL74164"] = true
MACHINES["TTL74175"] = true
MACHINES["TTL74181"] = true
MACHINES["TTL74259"] = true
MACHINES["TTL74381"] = true
MACHINES["TTL74543"] = true
MACHINES["TTL7474"] = true
MACHINES["TUBE"] = true
MACHINES["UCB1200"] = true
MACHINES["UPC82C710"] = true
MACHINES["UPC82C711"] = true
MACHINES["UPD1990A"] = true
MACHINES["UPD4991A"] = true
--MACHINES["UPD4992"] = true
MACHINES["UPD4701"] = true
MACHINES["UPD7001"] = true
MACHINES["UPD7002"] = true
MACHINES["UPD7004"] = true
MACHINES["UPD71071"] = true
MACHINES["UPD765"] = true
MACHINES["FDC_PLL"] = true
MACHINES["V3021"] = true
MACHINES["VIC_PL192"] = true
MACHINES["WD_FDC"] = true
MACHINES["WD1000"] = true
MACHINES["WD1010"] = true
MACHINES["WD11C00_17"] = true
MACHINES["WD2010"] = true
MACHINES["WD33C9X"] = true
MACHINES["WD7600"] = true
MACHINES["X2201"] = true
MACHINES["X2212"] = true
MACHINES["X76F041"] = true
MACHINES["X76F100"] = true
MACHINES["YM2148"] = true
MACHINES["YM3802"] = true
MACHINES["Z80CTC"] = true
MACHINES["Z80SIO"] = true
MACHINES["Z80SCC"] = true
MACHINES["Z80DMA"] = true
MACHINES["Z80PIO"] = true
MACHINES["Z80STI"] = true
MACHINES["Z8536"] = true
MACHINES["SMC92X4"] = true
MACHINES["HDC9234"] = true
MACHINES["TI99_HD"] = true
MACHINES["STRATA"] = true
MACHINES["STEPPERS"] = true
MACHINES["CORVUSHD"] = true
MACHINES["WOZFDC"] = true
MACHINES["APPLE_FDINTF"] = true
MACHINES["IWM"] = true
MACHINES["SWIM1"] = true
MACHINES["SWIM2"] = true
MACHINES["SWIM3"] = true
MACHINES["MAC_VIDEO_SONORA"] = true
MACHINES["DIABLO_HD"] = true
MACHINES["TMS1024"] = true
MACHINES["NSC810"] = true
MACHINES["VT82C496"] = true
MACHINES["FDC37C93X"] = true
MACHINES["GENPC"] = true
MACHINES["GEN_LATCH"] = true
MACHINES["WATCHDOG"] = true
MACHINES["SMARTMEDIA"] = true
MACHINES["APPLE_DRIVE"] = true
MACHINES["APPLE_FDC"] = true
MACHINES["SONY_DRIVE"] = true
MACHINES["SCNXX562"] = true
MACHINES["FGA002"] = true
MACHINES["I82586"] = true
MACHINES["INPUT_MERGER"] = true
-- MACHINES["K054321"] = true
MACHINES["ADC0844"] = true
MACHINES["28FXXX"] = true
-- MACHINES["GEN_FIFO"] = true
MACHINES["Z80DAISY"] = true
MACHINES["PS2DMAC"] = true
MACHINES["PS2INTC"] = true
MACHINES["PS2MC"] = true
MACHINES["PS2PAD"] = true
MACHINES["PS2SIF"] = true
MACHINES["PS2TIMER"] = true
MACHINES["IOPCDVD"] = true
MACHINES["IOPDMA"] = true
MACHINES["IOPINTC"] = true
MACHINES["IOPSIO2"] = true
MACHINES["IOPTIMER"] = true
MACHINES["Z8038"] = true
MACHINES["AIC565"] = true
MACHINES["AIC580"] = true
MACHINES["AIC6250"] = true
MACHINES["DC7085"] = true
MACHINES["I82357"] = true
MACHINES["XC1700E"] = true
MACHINES["EDLC"] = true
MACHINES["WTL3132"] = true
MACHINES["CXD1185"] = true
MACHINES["BL_HANDHELDS_MENUCONTROL"] = true
MACHINES["NS32081"] = true
MACHINES["NS32202"] = true
MACHINES["NS32082"] = true
MACHINES["BITMAP_PRINTER"] = true
MACHINES["NS32382"] = true

--------------------------------------------------
-- specify available bus cores
--------------------------------------------------

BUSES["A1BUS"] = true
BUSES["A2BUS"] = true
BUSES["A2GAMEIO"] = true
BUSES["A7800"] = true
BUSES["A800"] = true
BUSES["ABCBUS"] = true
BUSES["ABCKB"] = true
BUSES["ACORN"] = true
BUSES["ADAM"] = true
BUSES["ADAMNET"] = true
BUSES["ADB"] = true
BUSES["AMIGA_KEYBOARD"] = true
BUSES["APF"] = true
BUSES["APRICOT_EXPANSION"] = true
BUSES["APRICOT_KEYBOARD"] = true
BUSES["APRICOT_VIDEO"] = true
BUSES["AQUARIUS"] = true
BUSES["ARCADIA"] = true
BUSES["ARCHIMEDES_ECONET"] = true
BUSES["ARCHIMEDES_PODULE"] = true
BUSES["ASTROCADE"] = true
BUSES["ATA"] = true
BUSES["BBC_1MHZBUS"] = true
BUSES["BBC_ANALOGUE"] = true
BUSES["BBC_CART"] = true
BUSES["BBC_EXP"] = true
BUSES["BBC_FDC"] = true
BUSES["BBC_INTERNAL"] = true
BUSES["BBC_JOYPORT"] = true
BUSES["BBC_MODEM"] = true
BUSES["BBC_ROM"] = true
BUSES["BBC_TUBE"] = true
BUSES["BBC_USERPORT"] = true
BUSES["BML3"] = true
BUSES["BW2"] = true
BUSES["C64"] = true
BUSES["CBM2"] = true
BUSES["CBMIEC"] = true
BUSES["CBUS"] = true
BUSES["CENTRONICS"] = true
BUSES["CGENIE_EXPANSION"] = true
BUSES["CGENIE_PARALLEL"] = true
BUSES["CHANNELF"] = true
BUSES["COCO"] = true
BUSES["COLECO_CART"] = true
BUSES["COLECO_CONTROLLER"] = true
BUSES["COMPIS_GRAPHICS"] = true
BUSES["COMPUCOLOR"] = true
BUSES["COMX35"] = true
BUSES["CPC"] = true
BUSES["CRVISION"] = true
BUSES["DMV"] = true
BUSES["ECBBUS"] = true
BUSES["ECONET"] = true
BUSES["EINSTEIN_USERPORT"] = true
BUSES["EKARA"] = true
BUSES["ELECTRON_CART"] = true
BUSES["ELECTRON"] = true
BUSES["EP64"] = true
BUSES["EPSON_SIO"] = true
BUSES["EPSON_QX"] = true
BUSES["FMT_SCSI"] = true
BUSES["GAMATE"] = true
BUSES["GAMEBOY"] = true
BUSES["GAMEGEAR"] = true
BUSES["GBA"] = true
BUSES["GENERIC"] = true
BUSES["GIO64"] = true
BUSES["HEXBUS"] = true
BUSES["HP_IPC_IO"] = true
BUSES["HP80_IO"] = true
BUSES["HP9845_IO"] = true
BUSES["HPDIO"] = true
BUSES["HPHIL"] = true
BUSES["IEEE488"] = true
BUSES["IMI7000"] = true
BUSES["INTELLEC4"] = true
BUSES["INTERPRO_KEYBOARD"] = true
BUSES["INTERPRO_MOUSE"] = true
BUSES["INTERPRO_SR"] = true
BUSES["INTV_CTRL"] = true
BUSES["INTV"] = true
BUSES["IQ151"] = true
BUSES["ISA"] = true
BUSES["ISBX"] = true
BUSES["JAKKS_GAMEKEY"] = true
BUSES["KC"] = true
BUSES["LPCI"] = true
BUSES["M5"] = true
BUSES["MACKBD"] = true
BUSES["MACPDS"] = true
BUSES["MC10"] = true
BUSES["MEGADRIVE"] = true
BUSES["MIDI"] = true
BUSES["MSX_SLOT"] = true
BUSES["MTX"] = true
BUSES["MULTIBUS"] = true
BUSES["NASBUS"] = true
BUSES["NEOGEO_CTRL"] = true
BUSES["NEOGEO"] = true
BUSES["NES_CTRL"] = true
BUSES["NES"] = true
BUSES["NEWBRAIN"] = true
BUSES["NSCSI"] = true
BUSES["NUBUS"] = true
BUSES["O2"] = true
BUSES["ORICEXT"] = true
BUSES["P2000"] = true
BUSES["PASOPIA"] = true
BUSES["PC_JOY"] = true
BUSES["PC_KBD"] = true
BUSES["PC1512"] = true
BUSES["PCE"] = true
BUSES["PCE_CTRL"] = true
BUSES["PET"] = true
BUSES["PLUS4"] = true
BUSES["POFO"] = true
BUSES["PSI_KEYBOARD"] = true
BUSES["PSX_CONTROLLER"] = true
BUSES["PSX_PARALLEL"] = true
BUSES["QBUS"] = true
BUSES["QL"] = true
BUSES["RS232"] = true
BUSES["RTPC_KBD"] = true
BUSES["S100"] = true
BUSES["SAITEK_OSA"] = true
BUSES["SAMCOUPE_DRIVE_PORT"] = true
BUSES["SAMCOUPE_EXPANSION"] = true
BUSES["SAMCOUPE_MOUSE_PORT"] = true
BUSES["SAT_CTRL"] = true
BUSES["SATURN"] = true
BUSES["SBUS"] = true
BUSES["SCSI"] = true
BUSES["SCV"] = true
BUSES["SDK85"] = true
BUSES["SEGA8"] = true
BUSES["SG1000_EXP"] = true
BUSES["SGIKBD"] = true
BUSES["SMS_CTRL"] = true
BUSES["SMS_EXP"] = true
BUSES["SNES_CTRL"] = true
BUSES["SNES"] = true
BUSES["SPC1000"] = true
BUSES["SPECTRUM"] = true
BUSES["SS50"] = true
BUSES["SUNKBD"] = true
BUSES["SUNMOUSE"] = true
BUSES["SVI_EXPANDER"] = true
BUSES["SVI_SLOT"] = true
BUSES["TANBUS"] = true
BUSES["TATUNG_PIPE"] = true
BUSES["THOMSON"] = true
BUSES["TI8X"] = true
BUSES["TI99"] = true
BUSES["TI99X"] = true
BUSES["TIKI100"] = true
BUSES["TMC600"] = true
BUSES["TVC"] = true
BUSES["UTS_KBD"] = true
BUSES["VBOY"] = true
BUSES["VC4000"] = true
BUSES["VCS"] = true
BUSES["VCS_CTRL"] = true
BUSES["VECTREX"] = true
BUSES["VIC10"] = true
BUSES["VIC20"] = true
BUSES["VIDBRAIN"] = true
BUSES["VIP"] = true
BUSES["VME"] = true
BUSES["VSMILE"] = true
BUSES["VTECH_IOEXP"] = true
BUSES["VTECH_MEMEXP"] = true
BUSES["WANGPC"] = true
BUSES["WSWAN"] = true
BUSES["X68K"] = true
BUSES["Z29_KBD"] = true
BUSES["Z88"] = true
BUSES["ZORRO"] = true

--------------------------------------------------
-- specify used file formats
--------------------------------------------------

FORMATS["2D_DSK"] = true
FORMATS["A26_CAS"] = true
FORMATS["A5105_DSK"] = true
FORMATS["ABC1600_DSK"] = true
FORMATS["ABC800_DSK"] = true
FORMATS["ABCFD2_DSK"] = true
FORMATS["ACE_TAP"] = true
FORMATS["ACORN_DSK"] = true
FORMATS["ADAM_CAS"] = true
FORMATS["ADAM_DSK"] = true
FORMATS["AFS_DSK"] = true
FORMATS["AGAT840K_HLE_DSK"] = true
FORMATS["AIM_DSK"] = true
FORMATS["AMI_DSK"] = true
FORMATS["AP2_DSK"] = true
FORMATS["APD_DSK"] = true
FORMATS["APF_APT"] = true
FORMATS["APOLLO_DSK"] = true
FORMATS["APPLIX_DSK"] = true
FORMATS["APRIDISK"] = true
FORMATS["AP_DSK35"] = true
FORMATS["AQUARIUS_CAQ"] = true
FORMATS["ASST128_DSK"] = true
FORMATS["ASTROCADE_WAV"] = true
FORMATS["ATARI_DSK"] = true
FORMATS["ATOM_DSK"] = true
FORMATS["ATOM_TAP"] = true
FORMATS["BASICDSK"] = true
FORMATS["BW12_DSK"] = true
FORMATS["BW2_DSK"] = true
FORMATS["C3040_DSK"] = true
FORMATS["C4040_DSK"] = true
FORMATS["C8280_DSK"] = true
FORMATS["CAMPLYNX_CAS"] = true
FORMATS["CAMPLYNX_DSK"] = true
FORMATS["CBM_CRT"] = true
FORMATS["CBM_TAP"] = true
FORMATS["CCVF_DSK"] = true
FORMATS["CD90_640_DSK"] = true
FORMATS["CGENIE_DSK"] = true
FORMATS["CGEN_CAS"] = true
FORMATS["COCO_CAS"] = true
FORMATS["COMX35_DSK"] = true
FORMATS["CONCEPT_DSK"] = true
FORMATS["COUPEDSK"] = true
FORMATS["CPIS_DSK"] = true
FORMATS["CSW_CAS"] = true
FORMATS["D64_DSK"] = true
FORMATS["D71_DSK"] = true
FORMATS["D80_DSK"] = true
FORMATS["D81_DSK"] = true
FORMATS["D82_DSK"] = true
FORMATS["DCP_DSK"] = true
FORMATS["DIM_DSK"] = true
FORMATS["DIP_DSK"] = true
FORMATS["DMK_DSK"] = true
FORMATS["DS9_DSK"] = true
FORMATS["SDF_DSK"] = true
FORMATS["EP64_DSK"] = true
FORMATS["DMV_DSK"] = true
FORMATS["ESQ16_DSK"] = true
FORMATS["ESQ8_DSK"] = true
FORMATS["EXCALI64_DSK"] = true
FORMATS["FC100_CAS"] = true
FORMATS["FDD_DSK"] = true
FORMATS["FL1_DSK"] = true
FORMATS["FLEX_DSK"] = true
FORMATS["FM7_CAS"] = true
FORMATS["FMSX_CAS"] = true
FORMATS["FMTOWNS_DSK"] = true
FORMATS["FSD_DSK"] = true
FORMATS["G64_DSK"] = true
FORMATS["GTP_CAS"] = true
FORMATS["H8_CAS"] = true
FORMATS["HECTOR_MINIDISC"] = true
FORMATS["HECT_DSK"] = true
FORMATS["HECT_TAP"] = true
FORMATS["HPI_DSK"] = true
FORMATS["HP_IPC_DSK"] = true
FORMATS["HTI_TAP"] = true
FORMATS["IBMXDF_DSK"] = true
FORMATS["IMG_DSK"] = true
FORMATS["IPF_DSK"] = true
FORMATS["IQ151_DSK"] = true
FORMATS["ITT3030_DSK"] = true
FORMATS["JFD_DSK"] = true
FORMATS["JUKU_DSK"] = true
FORMATS["JVC_DSK"] = true
FORMATS["KAYPRO_DSK"] = true
FORMATS["KC85_DSK"] = true
FORMATS["KC_CAS"] = true
FORMATS["KIM1_CAS"] = true
FORMATS["LVIV_LVT"] = true
FORMATS["M20_DSK"] = true
FORMATS["M5_DSK"] = true
FORMATS["MBEE_CAS"] = true
FORMATS["MDOS_DSK"] = true
FORMATS["MFM_HD"] = true
FORMATS["MM_DSK"] = true
FORMATS["MS0515_DSK"] = true
FORMATS["MSX_DSK"] = true
FORMATS["MTX_DSK"] = true
FORMATS["MZ_CAS"] = true
FORMATS["NANOS_DSK"] = true
FORMATS["NASCOM_DSK"] = true
FORMATS["NASLITE_DSK"] = true
FORMATS["NES_DSK"] = true
FORMATS["NFD_DSK"] = true
FORMATS["OPD_DSK"] = true
FORMATS["ORAO_CAS"] = true
FORMATS["ORIC_DSK"] = true
FORMATS["ORIC_TAP"] = true
FORMATS["OS9_DSK"] = true
FORMATS["P2000T_CAS"] = true
FORMATS["P6001_CAS"] = true
FORMATS["PASTI_DSK"] = true
FORMATS["PC98FDI_DSK"] = true
FORMATS["PC98_DSK"] = true
FORMATS["PHC25_CAS"] = true
FORMATS["PK8020_DSK"] = true
FORMATS["PMD_CAS"] = true
FORMATS["POLY_DSK"] = true
FORMATS["PPG_DSK"] = true
FORMATS["PRIMOPTP"] = true
FORMATS["PYLDIN_DSK"] = true
FORMATS["QL_DSK"] = true
FORMATS["RC759_DSK"] = true
FORMATS["RK_CAS"] = true
FORMATS["RX50_DSK"] = true
FORMATS["SC3000_BIT"] = true
FORMATS["SDD_DSK"] = true
FORMATS["SF7000_DSK"] = true
FORMATS["SMX_DSK"] = true
FORMATS["SOL_CAS"] = true
FORMATS["SORC_CAS"] = true
FORMATS["SORC_DSK"] = true
FORMATS["SORD_CAS"] = true
FORMATS["SPC1000_CAS"] = true
FORMATS["ST_DSK"] = true
FORMATS["SVI_CAS"] = true
FORMATS["SVI_DSK"] = true
FORMATS["SWD_DSK"] = true
FORMATS["TANDY2K_DSK"] = true
FORMATS["THOM_CAS"] = true
FORMATS["THOM_DSK"] = true
FORMATS["TI99_DSK"] = true
FORMATS["TIKI100_DSK"] = true
FORMATS["TRD_DSK"] = true
FORMATS["TRS80_DSK"] = true
FORMATS["TRS_CAS"] = true
FORMATS["TVC_CAS"] = true
FORMATS["TVC_DSK"] = true
FORMATS["TZX_CAS"] = true
FORMATS["UEF_CAS"] = true
FORMATS["UNIFLEX_DSK"] = true
FORMATS["VDK_DSK"] = true
FORMATS["VECTOR06_DSK"] = true
FORMATS["VG5K_CAS"] = true
FORMATS["VICTOR9K_DSK"] = true
FORMATS["VT_CAS"] = true
FORMATS["VT_DSK"] = true
FORMATS["FS_VTECH"] = true
FORMATS["WD177X_DSK"] = true
FORMATS["X07_CAS"] = true
FORMATS["X1_TAP"] = true
FORMATS["XDF_DSK"] = true
FORMATS["ZX81_P"] = true
FORMATS["FS_PRODOS"] = true
FORMATS["FS_ORIC_JASMIN"] = true
FORMATS["FS_COCO_RSDOS"] = true
FORMATS["FS_COCO_OS9"] = true

--------------------------------------------------
-- this is the list of driver libraries that
-- comprise MESS plus messdriv.*", which contains
-- the list of drivers
--------------------------------------------------
function linkProjects_mame_mess(_target, _subtarget)
	links {
		"access",
		"aci",
		"acorn",
		"act",
		"adc",
		"agat",
		"akai",
		"alesis",
		"altos",
		"ami",
		"amirix",
		"amiga",
		"ampro",
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
		"ave",
		"aviion",
		"bally",
		"bandai",
		"banctec",
		"be",
		"beehive",
		"bitcorp",
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
		"ceres",
		"chessking",
		"chromatics",
		"chrysler",
		"citoh",
		"coleco",
		"compugraphic",
		"conic",
		"consumenta",
		"cromemco",
		"comx",
		"concept",
		"conitec",
		"cxg",
		"cybiko",
		"dai",
		"dcs",
		"ddr",
		"dec",
		"dicksmth",
		"dms",
		"dragon",
		"drc",
		"dulmont",
		"eaca",
		"einis",
		"elektor",
		"elektron",
		"elektronika",
		"emusys",
		"ensoniq",
		"enterprise",
		"entex",
		"epoch",
		"epson",
		"ericsson",
		"exidy",
		"exorterm",
		"fairch",
		"fairlight",
		"falco",
		"fidelity",
		"force",
		"francedr",
		"fujitsu",
		"funtech",
		"galaxy",
		"gamepark",
		"gi",
		"gridcomp",
		"grundy",
		"h01x",
		"hartung",
		"hds",
		"heathkit",
		"hec2hrp",
		"hegener",
		"heurikon",
		"hitachi",
		"homebrew",
		"homelab",
		"hp",
		"husky",
		"ibm6580",
		"ie15",
		"imp",
		"informer",
		"intel",
		"interpro",
		"interton",
		"intv",
		"isc",
		"jazz",
		"kawai",
		"kaypro",
		"koei",
		"kontron",
		"korg",
		"kurzweil",
		"kyber",
		"kyocera",
		"leapfrog",
		"learsiegler",
		"lsi",
		"luxor",
		"magnavox",
		"makerbot",
		"matsushi",
		"mattel",
		"mb",
		"mchester",
		"memotech",
		"mera",
		"mg1",
		"mgu",
		"microkey",
		"microsoft",
		"microterm",
		"mips",
		"mit",
		"mits",
		"mitsubishi",
		"mizar",
		"morrow",
		"mos",
		"motorola",
		"multitch",
		"mupid",
		"nakajima",
		"nascom",
		"natsemi",
		"ncd",
		"ne",
		"nec",
		"netronic",
		"next",
		"nintendo",
		"nokia",
		"northstar",
		"novag",
		"novation",
		"olivetti",
		"olympia",
		"omnibyte",
		"omron",
		"openuni",
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
		"poly",
		"poly88",
		"positron",
		"psion",
		"quantel",
		"qume",
		"radio",
		"rca",
		"regnecentralen",
		"ritam",
		"rm",
		"robotron",
		"rockwell",
		"roland",
		"rolm",
		"rtpc",
		"sage",
		"saitek",
		"samcoupe",
		"samsung",
		"sanyo",
		"saturn",
		"segacons",
		"sequential",
		"sgi",
		"sharp",
		"siemens",
		"sinclair",
		"skeleton",
		"slicer",
		"snk",
		"sony",
		"sony_news",
		"sord",
		"special",
		"stm",
		"sun",
		"svi",
		"svision",
		"swtpc",
		"synertek",
		"ta",
		"tab",
		"tandberg",
		"tangerin",
		"tasc",
		"tatung",
		"teamconc",
		"tectoy",
		"tektroni",
		"telenova",
		"telercas",
		"televideo",
		"tesla",
		"thomson",
		"ti",
		"tiger",
		"tigertel",
		"tiki",
		"tomy",
		"toshiba",
		"trainer",
		"trs",
		"tvgames",
		"ultimachine",
		"ultratec",
		"unicard",
		"unisonic",
		"unisys",
		"usp",
		"veb",
		"verifone",
		"vidbrain",
		"videoton",
		"visual",
		"votrax",
		"vtech",
		"wang",
		"wavemate",
		"wyse",
		"westinghouse",
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
	precompiledheaders_novs()

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
	}

	includedirs {
		ext_includedir("zlib")
	}
end

function createProjects_mame_mess(_target, _subtarget)
--------------------------------------------------
-- the following files are MAME components and
-- shared across a number of drivers
--
-- aa310.c (MESS), aristmk5.c, ertictac.c (MAME)
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
-- neogeocd.c (MESS), midas.c, neogeo.c, neoprint.c (MAME)
-- cdi.c (MESS + MAME)
-- 3do.c (MESS + MAME), konamim2.c (MAME)
-- vectrex.c (MESS + MAME)
-- cps1.c (MESS + MAME)
-- apple
--------------------------------------------------
if (_subtarget=="mess") then
createMESSProjects(_target, _subtarget, "mameshared")
files {
	MAME_DIR .. "src/mame/machine/amiga.cpp",
	MAME_DIR .. "src/mame/video/amiga.cpp",
	MAME_DIR .. "src/mame/video/amigaaga.cpp",
	MAME_DIR .. "src/mame/video/amigaaga.h",
	MAME_DIR .. "src/mame/video/tia.cpp",
	MAME_DIR .. "src/mame/video/tia.h",
	MAME_DIR .. "src/mame/machine/atari400.cpp",
	MAME_DIR .. "src/mame/video/atari400.cpp",
	MAME_DIR .. "src/mame/includes/atari400.h",
	MAME_DIR .. "src/mame/video/antic.cpp",
	MAME_DIR .. "src/mame/video/antic.h",
	MAME_DIR .. "src/mame/video/gtia.cpp",
	MAME_DIR .. "src/mame/video/gtia.h",
	MAME_DIR .. "src/mame/drivers/jaguar.cpp",
	MAME_DIR .. "src/mame/includes/jaguar.h",
	MAME_DIR .. "src/mame/video/jag_blitter.cpp",
	MAME_DIR .. "src/mame/video/jag_blitter.h",
	MAME_DIR .. "src/mame/audio/jaguar.cpp",
	MAME_DIR .. "src/mame/video/jaguar.cpp",
	MAME_DIR .. "src/mame/video/jagblit.h",
	MAME_DIR .. "src/mame/video/jagblit.hxx",
	MAME_DIR .. "src/mame/video/jagobj.hxx",
	MAME_DIR .. "src/mame/drivers/astrocde.cpp",
	MAME_DIR .. "src/mame/includes/astrocde.h",
	MAME_DIR .. "src/mame/video/astrocde.cpp",
	MAME_DIR .. "src/mame/machine/kabuki.cpp",
	MAME_DIR .. "src/mame/machine/kabuki.h",
	MAME_DIR .. "src/mame/video/pk8000.cpp",
	MAME_DIR .. "src/mame/machine/snes.cpp",
	MAME_DIR .. "src/mame/machine/n64.cpp",
	MAME_DIR .. "src/mame/video/n64.cpp",
	MAME_DIR .. "src/mame/video/n64types.h",
	MAME_DIR .. "src/mame/video/rdpfiltr.hxx",
	MAME_DIR .. "src/mame/video/n64.h",
	MAME_DIR .. "src/mame/video/rdpblend.cpp",
	MAME_DIR .. "src/mame/video/rdpblend.h",
	MAME_DIR .. "src/mame/video/rdptpipe.cpp",
	MAME_DIR .. "src/mame/video/rdptpipe.h",
	MAME_DIR .. "src/mame/video/pin64.cpp",
	MAME_DIR .. "src/mame/video/pin64.h",
	MAME_DIR .. "src/mame/machine/megadriv.cpp",
	MAME_DIR .. "src/mame/drivers/naomi.cpp",
	MAME_DIR .. "src/mame/includes/naomi.h",
	MAME_DIR .. "src/mame/includes/dc.h",
	MAME_DIR .. "src/mame/machine/awboard.cpp",
	MAME_DIR .. "src/mame/machine/awboard.h",
	MAME_DIR .. "src/mame/machine/dc.cpp",
	MAME_DIR .. "src/mame/machine/dc-ctrl.cpp",
	MAME_DIR .. "src/mame/machine/dc-ctrl.h",
	MAME_DIR .. "src/mame/machine/jvs13551.cpp",
	MAME_DIR .. "src/mame/machine/jvs13551.h",
	MAME_DIR .. "src/mame/machine/maple-dc.cpp",
	MAME_DIR .. "src/mame/machine/maple-dc.h",
	MAME_DIR .. "src/mame/machine/mapledev.cpp",
	MAME_DIR .. "src/mame/machine/mapledev.h",
	MAME_DIR .. "src/mame/machine/mie.cpp",
	MAME_DIR .. "src/mame/machine/mie.h",
	MAME_DIR .. "src/mame/machine/naomibd.cpp",
	MAME_DIR .. "src/mame/machine/naomibd.h",
	MAME_DIR .. "src/mame/machine/naomig1.cpp",
	MAME_DIR .. "src/mame/machine/naomig1.h",
	MAME_DIR .. "src/mame/machine/dc_g2if.cpp",
	MAME_DIR .. "src/mame/machine/dc_g2if.h",
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
	MAME_DIR .. "src/mame/machine/315-6154.cpp",
	MAME_DIR .. "src/mame/machine/315-6154.h",
	MAME_DIR .. "src/mame/machine/segashiobd.cpp",
	MAME_DIR .. "src/mame/machine/segashiobd.h",
	MAME_DIR .. "src/mame/video/powervr2.cpp",
	MAME_DIR .. "src/mame/video/powervr2.h",
	MAME_DIR .. "src/mame/drivers/neogeo.cpp",
	MAME_DIR .. "src/mame/includes/neogeo.h",
	MAME_DIR .. "src/mame/machine/ng_memcard.cpp",
	MAME_DIR .. "src/mame/machine/ng_memcard.h",
	MAME_DIR .. "src/mame/video/neogeo.cpp",
	MAME_DIR .. "src/mame/video/neogeo_spr.cpp",
	MAME_DIR .. "src/mame/video/neogeo_spr.h",
	MAME_DIR .. "src/mame/drivers/cdi.cpp",
	MAME_DIR .. "src/mame/includes/cdi.h",
	MAME_DIR .. "src/mame/machine/cdicdic.cpp",
	MAME_DIR .. "src/mame/machine/cdicdic.h",
	MAME_DIR .. "src/mame/machine/cdislavehle.cpp",
	MAME_DIR .. "src/mame/machine/cdislavehle.h",
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
	MAME_DIR .. "src/mame/drivers/fcrash.cpp",
	MAME_DIR .. "src/mame/video/xbox_nv2a.cpp",
	MAME_DIR .. "src/mame/machine/xbox.cpp",
	MAME_DIR .. "src/mame/machine/xbox_usb.cpp",
	MAME_DIR .. "src/mame/machine/xbox_pci.cpp",
	MAME_DIR .. "src/mame/includes/saturn.h",
	MAME_DIR .. "src/mame/drivers/saturn.cpp",
	MAME_DIR .. "src/mame/machine/saturn.cpp",
	MAME_DIR .. "src/mame/video/saturn.cpp",
	MAME_DIR .. "src/mame/machine/saturn_cdb.cpp",
	MAME_DIR .. "src/mame/machine/saturn_cdb.h",
	MAME_DIR .. "src/mame/video/apple2.cpp",
	MAME_DIR .. "src/mame/video/apple2.h",
	MAME_DIR .. "src/mame/machine/apple2common.cpp",
	MAME_DIR .. "src/mame/machine/apple2common.h",
}
end
--------------------------------------------------
-- the following files are general components and
-- shared across a number of drivers
--------------------------------------------------
createMESSProjects(_target, _subtarget, "messshared")
files {
	MAME_DIR .. "src/mame/machine/teleprinter.cpp",
	MAME_DIR .. "src/mame/machine/teleprinter.h",
	MAME_DIR .. "src/mame/machine/z80bin.cpp",
	MAME_DIR .. "src/mame/machine/z80bin.h",
}
--------------------------------------------------
-- manufacturer-specific groupings for drivers
--------------------------------------------------

createMESSProjects(_target, _subtarget, "access")
files {
	MAME_DIR .. "src/mame/drivers/acvirus.cpp",
}

createMESSProjects(_target, _subtarget, "aci")
files {
	MAME_DIR .. "src/mame/drivers/aci_boris.cpp",
	MAME_DIR .. "src/mame/drivers/aci_borisdpl.cpp",
	MAME_DIR .. "src/mame/drivers/aci_ggm.cpp",
	MAME_DIR .. "src/mame/drivers/aci_prodigy.cpp",
}

createMESSProjects(_target, _subtarget, "acorn")
files {
	MAME_DIR .. "src/mame/drivers/aa310.cpp",
	MAME_DIR .. "src/mame/machine/archimedes_keyb.cpp",
	MAME_DIR .. "src/mame/machine/archimedes_keyb.h",
	MAME_DIR .. "src/mame/drivers/accomm.cpp",
	MAME_DIR .. "src/mame/drivers/acrnsys1.cpp",
	MAME_DIR .. "src/mame/drivers/acrnsys.cpp",
	MAME_DIR .. "src/mame/drivers/atom.cpp",
	MAME_DIR .. "src/mame/includes/atom.h",
	MAME_DIR .. "src/mame/drivers/bbc.cpp",
	MAME_DIR .. "src/mame/includes/bbc.h",
	MAME_DIR .. "src/mame/machine/bbc.cpp",
	MAME_DIR .. "src/mame/video/bbc.cpp",
	MAME_DIR .. "src/mame/drivers/cms.cpp",
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
	MAME_DIR .. "src/mame/drivers/apxen.cpp",
	MAME_DIR .. "src/mame/machine/apricotkb.cpp",
	MAME_DIR .. "src/mame/machine/apricotkb.h",
	MAME_DIR .. "src/mame/drivers/victor9k.cpp",
	MAME_DIR .. "src/mame/machine/victor9k_kb.cpp",
	MAME_DIR .. "src/mame/machine/victor9k_kb.h",
	MAME_DIR .. "src/mame/machine/victor9k_fdc.cpp",
	MAME_DIR .. "src/mame/machine/victor9k_fdc.h",
}

createMESSProjects(_target, _subtarget, "adc")
files {
	MAME_DIR .. "src/mame/drivers/super6.cpp",
	MAME_DIR .. "src/mame/includes/super6.h",
	MAME_DIR .. "src/mame/drivers/superslave.cpp",
}

createMESSProjects(_target, _subtarget, "agat")
files {
	MAME_DIR .. "src/mame/drivers/agat.cpp",
	MAME_DIR .. "src/mame/machine/agatkeyb.cpp",
	MAME_DIR .. "src/mame/machine/agatkeyb.h",
	MAME_DIR .. "src/mame/video/agat7.cpp",
	MAME_DIR .. "src/mame/video/agat7.h",
	MAME_DIR .. "src/mame/video/agat9.cpp",
	MAME_DIR .. "src/mame/video/agat9.h",
}

createMESSProjects(_target, _subtarget, "akai")
files {
	MAME_DIR .. "src/mame/drivers/akaiax80.cpp",
	MAME_DIR .. "src/mame/drivers/akaivx600.cpp",
	MAME_DIR .. "src/mame/drivers/mpc3000.cpp",
	MAME_DIR .. "src/mame/drivers/mpc60.cpp",
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
	MAME_DIR .. "src/mame/drivers/altos2.cpp",
	MAME_DIR .. "src/mame/drivers/altos5.cpp",
	MAME_DIR .. "src/mame/drivers/altos486.cpp",
	MAME_DIR .. "src/mame/drivers/altos8600.cpp",
	MAME_DIR .. "src/mame/machine/acs8600_ics.cpp",
}

createMESSProjects(_target, _subtarget, "ami")
files {
	MAME_DIR .. "src/mame/drivers/hh_amis2k.cpp",
}

createMESSProjects(_target, _subtarget, "amirix")
files {
	MAME_DIR .. "src/mame/drivers/wxstar4000.cpp",
}

createMESSProjects(_target, _subtarget, "amiga")
files {
	MAME_DIR .. "src/mame/drivers/amiga.cpp",
	MAME_DIR .. "src/mame/includes/amiga.h",
}

createMESSProjects(_target, _subtarget, "ampro")
files {
	MAME_DIR .. "src/mame/drivers/ampro.cpp",
	MAME_DIR .. "src/mame/drivers/lb186.cpp",
	MAME_DIR .. "src/mame/drivers/lbpc.cpp",
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
	MAME_DIR .. "src/mame/video/ams40041.cpp",
	MAME_DIR .. "src/mame/video/ams40041.h",
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
	MAME_DIR .. "src/mame/machine/apollo_kbd.cpp",
	MAME_DIR .. "src/mame/machine/apollo_kbd.h",
	MAME_DIR .. "src/mame/video/apollo.cpp",
}

createMESSProjects(_target, _subtarget, "apple")
files {
	MAME_DIR .. "src/mame/drivers/apple1.cpp",
	MAME_DIR .. "src/mame/drivers/apple2.cpp",
	MAME_DIR .. "src/mame/drivers/apple2e.cpp",
	MAME_DIR .. "src/mame/includes/apple2e.h",
	MAME_DIR .. "src/mame/drivers/tk2000.cpp",
	MAME_DIR .. "src/mame/drivers/apple2gs.cpp",
	MAME_DIR .. "src/mame/drivers/apple3.cpp",
	MAME_DIR .. "src/mame/includes/apple3.h",
	MAME_DIR .. "src/mame/machine/apple3.cpp",
	MAME_DIR .. "src/mame/video/apple3.cpp",
	MAME_DIR .. "src/mame/drivers/lisa.cpp",
	MAME_DIR .. "src/mame/includes/lisa.h",
	MAME_DIR .. "src/mame/machine/lisa.cpp",
	MAME_DIR .. "src/mame/drivers/lwriter.cpp",
	MAME_DIR .. "src/mame/drivers/mac128.cpp",
	MAME_DIR .. "src/mame/drivers/macquadra700.cpp",
	MAME_DIR .. "src/mame/drivers/macpdm.cpp",
	MAME_DIR .. "src/mame/drivers/macprtb.cpp",
	MAME_DIR .. "src/mame/drivers/macpwrbk030.cpp",
	MAME_DIR .. "src/mame/drivers/mac.cpp",
	MAME_DIR .. "src/mame/includes/mac.h",
	MAME_DIR .. "src/mame/machine/egret.cpp",
	MAME_DIR .. "src/mame/machine/egret.h",
	MAME_DIR .. "src/mame/machine/mac.cpp",
	MAME_DIR .. "src/mame/machine/macadb.cpp",
	MAME_DIR .. "src/mame/machine/macadb.h",
	MAME_DIR .. "src/mame/machine/macrtc.cpp",
	MAME_DIR .. "src/mame/machine/macrtc.h",
	MAME_DIR .. "src/mame/machine/swim.cpp",
	MAME_DIR .. "src/mame/machine/swim.h",
	MAME_DIR .. "src/mame/video/mac.cpp",
	MAME_DIR .. "src/mame/drivers/macpci.cpp",
	MAME_DIR .. "src/mame/includes/macpci.h",
	MAME_DIR .. "src/mame/machine/macpci.cpp",
	MAME_DIR .. "src/mame/machine/cuda.cpp",
	MAME_DIR .. "src/mame/machine/cuda.h",
	MAME_DIR .. "src/mame/machine/macscsi.cpp",
	MAME_DIR .. "src/mame/machine/macscsi.h",
	MAME_DIR .. "src/mame/drivers/iphone2g.cpp",
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
	MAME_DIR .. "src/mame/machine/msx_switched.h",
	MAME_DIR .. "src/mame/machine/msx_systemflags.cpp",
	MAME_DIR .. "src/mame/machine/msx_systemflags.h",
}

createMESSProjects(_target, _subtarget, "at")
files {
	MAME_DIR .. "src/mame/drivers/at.cpp",
	MAME_DIR .. "src/mame/drivers/atpci.cpp",
	MAME_DIR .. "src/mame/drivers/ps2.cpp",
	MAME_DIR .. "src/mame/machine/at.h",
	MAME_DIR .. "src/mame/machine/at.cpp",
	MAME_DIR .. "src/mame/drivers/ct486.cpp",
}

createMESSProjects(_target, _subtarget, "atari")
files {
	MAME_DIR .. "src/mame/includes/a2600.h",
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
	MAME_DIR .. "src/mame/machine/lynx.cpp",
	MAME_DIR .. "src/mame/drivers/pofo.cpp",
	MAME_DIR .. "src/mame/machine/pofo_kbd.cpp",
	MAME_DIR .. "src/mame/machine/pofo_kbd.h",
	MAME_DIR .. "src/mame/drivers/tvboy.cpp",
}

createMESSProjects(_target, _subtarget, "att")
files {
	MAME_DIR .. "src/mame/drivers/att3b2.cpp",
	MAME_DIR .. "src/mame/drivers/att4425.cpp",
	MAME_DIR .. "src/mame/drivers/att610.cpp",
	MAME_DIR .. "src/mame/drivers/att630.cpp",
	MAME_DIR .. "src/mame/drivers/unixpc.cpp",
}

createMESSProjects(_target, _subtarget, "ave")
files {
	MAME_DIR .. "src/mame/drivers/ave_arb.cpp",
}

createMESSProjects(_target, _subtarget, "aviion")
files {
	MAME_DIR .. "src/mame/drivers/aviion88k.cpp",
}

createMESSProjects(_target, _subtarget, "bally")
files {
	MAME_DIR .. "src/mame/drivers/astrohome.cpp",
}

createMESSProjects(_target, _subtarget, "banctec")
files {
	MAME_DIR .. "src/mame/drivers/banctec.cpp",
}

createMESSProjects(_target, _subtarget, "bandai")
files {
	MAME_DIR .. "src/mame/drivers/sv8000.cpp",
	MAME_DIR .. "src/mame/drivers/rx78.cpp",
	MAME_DIR .. "src/mame/drivers/tamag1.cpp",
	MAME_DIR .. "src/mame/drivers/wswan.cpp",
	MAME_DIR .. "src/mame/audio/wswan.cpp",
	MAME_DIR .. "src/mame/audio/wswan.h",
	MAME_DIR .. "src/mame/video/wswan.cpp",
	MAME_DIR .. "src/mame/video/wswan.h",
	MAME_DIR .. "src/mame/drivers/bandai_design_master.cpp",
}

createMESSProjects(_target, _subtarget, "be")
files {
	MAME_DIR .. "src/mame/drivers/bebox.cpp",
	MAME_DIR .. "src/mame/includes/bebox.h",
	MAME_DIR .. "src/mame/machine/bebox.cpp",
}

createMESSProjects(_target, _subtarget, "beehive")
files {
	MAME_DIR .. "src/mame/drivers/microb.cpp",
}

createMESSProjects(_target, _subtarget, "bitcorp")
files {
	MAME_DIR .. "src/mame/drivers/gamate.cpp",
	MAME_DIR .. "src/mame/video/gamate.cpp",
	MAME_DIR .. "src/mame/video/gamate.h",
}

createMESSProjects(_target, _subtarget, "bnpo")
files {
	MAME_DIR .. "src/mame/drivers/b2m.cpp",
	MAME_DIR .. "src/mame/includes/b2m.h",
	MAME_DIR .. "src/mame/machine/b2m.cpp",
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
	MAME_DIR .. "src/mame/includes/apexc.h",
	MAME_DIR .. "src/mame/machine/apexc.h",
	MAME_DIR .. "src/mame/machine/apexc.cpp",
	MAME_DIR .. "src/mame/video/apexc.cpp",
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
	MAME_DIR .. "src/mame/drivers/cz101.cpp",
	MAME_DIR .. "src/mame/drivers/fp200.cpp",
	MAME_DIR .. "src/mame/drivers/fp1100.cpp",
	MAME_DIR .. "src/mame/drivers/fp6000.cpp",
	MAME_DIR .. "src/mame/machine/fp6000_kbd.cpp",
	MAME_DIR .. "src/mame/machine/fp6000_kbd.h",
	MAME_DIR .. "src/mame/drivers/ctk2000.cpp",
	MAME_DIR .. "src/mame/drivers/ctk551.cpp",
	MAME_DIR .. "src/mame/drivers/ht6000.cpp",
	MAME_DIR .. "src/mame/drivers/ld50.cpp",
	MAME_DIR .. "src/mame/drivers/pb1000.cpp",
	MAME_DIR .. "src/mame/drivers/pv1000.cpp",
	MAME_DIR .. "src/mame/drivers/pv2000.cpp",
	MAME_DIR .. "src/mame/drivers/sk1.cpp",
	MAME_DIR .. "src/mame/drivers/sx1000.cpp",
	MAME_DIR .. "src/mame/drivers/rz1.cpp",
	MAME_DIR .. "src/mame/drivers/casio_rompack.cpp",
}

createMESSProjects(_target, _subtarget, "cbm")
files {
	MAME_DIR .. "src/mame/drivers/c128.cpp",
	MAME_DIR .. "src/mame/drivers/c64.cpp",
	MAME_DIR .. "src/mame/drivers/c64dtv.cpp",
	MAME_DIR .. "src/mame/drivers/c65.cpp",
	MAME_DIR .. "src/mame/includes/c65.h",
	MAME_DIR .. "src/mame/drivers/c900.cpp",
	MAME_DIR .. "src/mame/drivers/cbm2.cpp",
	MAME_DIR .. "src/mame/drivers/chessmate.cpp",
	MAME_DIR .. "src/mame/drivers/clcd.cpp",
	MAME_DIR .. "src/mame/drivers/pet.cpp",
	MAME_DIR .. "src/mame/drivers/plus4.cpp",
	MAME_DIR .. "src/mame/drivers/vic10.cpp",
	MAME_DIR .. "src/mame/drivers/vic20.cpp",
	MAME_DIR .. "src/mame/machine/cbm_snqk.cpp",
	MAME_DIR .. "src/mame/machine/cbm_snqk.h",
	MAME_DIR .. "src/mame/drivers/mps1230.cpp",
}

createMESSProjects(_target, _subtarget, "cccp")
files {
	MAME_DIR .. "src/mame/drivers/argo.cpp",
	MAME_DIR .. "src/mame/drivers/cm1800.cpp",
	MAME_DIR .. "src/mame/drivers/debut.cpp",
	MAME_DIR .. "src/mame/drivers/intellect02.cpp",
	MAME_DIR .. "src/mame/drivers/lviv.cpp",
	MAME_DIR .. "src/mame/includes/lviv.h",
	MAME_DIR .. "src/mame/machine/lviv.cpp",
	MAME_DIR .. "src/mame/video/lviv.cpp",
	MAME_DIR .. "src/mame/drivers/mikro80.cpp",
	MAME_DIR .. "src/mame/includes/mikro80.h",
	MAME_DIR .. "src/mame/machine/mikro80.cpp",
	MAME_DIR .. "src/mame/drivers/okean240.cpp",
	MAME_DIR .. "src/mame/drivers/pk8000.cpp",
	MAME_DIR .. "src/mame/includes/pk8000.h",
	MAME_DIR .. "src/mame/drivers/pk8020.cpp",
	MAME_DIR .. "src/mame/includes/pk8020.h",
	MAME_DIR .. "src/mame/machine/pk8020.cpp",
	MAME_DIR .. "src/mame/video/pk8020.cpp",
	MAME_DIR .. "src/mame/drivers/pyl601.cpp",
	MAME_DIR .. "src/mame/drivers/sm1800.cpp",
	MAME_DIR .. "src/mame/drivers/sm7238.cpp",
	MAME_DIR .. "src/mame/machine/km035.cpp",
	MAME_DIR .. "src/mame/machine/km035.h",
	MAME_DIR .. "src/mame/drivers/uknc.cpp",
	MAME_DIR .. "src/mame/drivers/unior.cpp",
	MAME_DIR .. "src/mame/drivers/ut88.cpp",
	MAME_DIR .. "src/mame/includes/ut88.h",
	MAME_DIR .. "src/mame/machine/ut88.cpp",
	MAME_DIR .. "src/mame/drivers/vector06.cpp",
	MAME_DIR .. "src/mame/includes/vector06.h",
	MAME_DIR .. "src/mame/machine/vector06.cpp",
	MAME_DIR .. "src/mame/video/vector06.cpp",
	MAME_DIR .. "src/mame/drivers/vta2000.cpp",
}

createMESSProjects(_target, _subtarget, "cce")
files {
	MAME_DIR .. "src/mame/drivers/mc1000.cpp",
}

createMESSProjects(_target, _subtarget, "ccs")
files {
	MAME_DIR .. "src/mame/drivers/ccs2810.cpp",
}

createMESSProjects(_target, _subtarget, "ceres")
files {
	MAME_DIR .. "src/mame/drivers/ceres.cpp",
}

createMESSProjects(_target, _subtarget, "chessking")
files {
	MAME_DIR .. "src/mame/drivers/cking_master.cpp",
}

createMESSProjects(_target, _subtarget, "chromatics")
files {
	MAME_DIR .. "src/mame/drivers/cgc7900.cpp",
	MAME_DIR .. "src/mame/includes/cgc7900.h",
	MAME_DIR .. "src/mame/video/cgc7900.cpp",
}

createMESSProjects(_target, _subtarget, "chrysler")
files {
	MAME_DIR .. "src/mame/drivers/eva.cpp",
}

createMESSProjects(_target, _subtarget, "citoh")
files {
	MAME_DIR .. "src/mame/drivers/cit101.cpp",
	MAME_DIR .. "src/mame/machine/cit101_kbd.cpp",
	MAME_DIR .. "src/mame/machine/cit101_kbd.h",
	MAME_DIR .. "src/mame/drivers/cit101xl.cpp",
	MAME_DIR .. "src/mame/drivers/cit220.cpp",
	MAME_DIR .. "src/mame/machine/cit220_kbd.cpp",
	MAME_DIR .. "src/mame/machine/cit220_kbd.h",
}

createMESSProjects(_target, _subtarget, "coleco")
files {
	MAME_DIR .. "src/mame/drivers/adam.cpp",
	MAME_DIR .. "src/mame/includes/adam.h",
	MAME_DIR .. "src/mame/drivers/coleco.cpp",
	MAME_DIR .. "src/mame/includes/coleco.h",
	MAME_DIR .. "src/mame/machine/coleco.cpp",
	MAME_DIR .. "src/mame/machine/coleco.h",
	MAME_DIR .. "src/mame/drivers/wrinkles.cpp",
}

createMESSProjects(_target, _subtarget, "compugraphic")
files {
	MAME_DIR .. "src/mame/drivers/pwrview.cpp",
}

createMESSProjects(_target, _subtarget, "conic")
files {
	MAME_DIR .. "src/mame/drivers/conic_cchess2.cpp",
	MAME_DIR .. "src/mame/drivers/conic_cchess3.cpp",
}

createMESSProjects(_target, _subtarget, "consumenta")
files {
	MAME_DIR .. "src/mame/drivers/conchess.cpp",
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

createMESSProjects(_target, _subtarget, "cxg")
files {
	MAME_DIR .. "src/mame/drivers/cxg_ch2001.cpp",
	MAME_DIR .. "src/mame/drivers/cxg_dominator.cpp",
	MAME_DIR .. "src/mame/drivers/cxg_scptchess.cpp",
	MAME_DIR .. "src/mame/drivers/cxg_sphinx40.cpp",
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

createMESSProjects(_target, _subtarget, "dcs")
files {
	MAME_DIR .. "src/mame/drivers/compuchess.cpp",
}

createMESSProjects(_target, _subtarget, "ddr")
files {
	MAME_DIR .. "src/mame/drivers/ac1.cpp",
	MAME_DIR .. "src/mame/drivers/bcs3.cpp",
	MAME_DIR .. "src/mame/drivers/c80.cpp",
	MAME_DIR .. "src/mame/includes/c80.h",
	MAME_DIR .. "src/mame/drivers/huebler.cpp",
	MAME_DIR .. "src/mame/includes/huebler.h",
	MAME_DIR .. "src/mame/drivers/jtc.cpp",
	MAME_DIR .. "src/mame/drivers/kramermc.cpp",
	MAME_DIR .. "src/mame/drivers/llc1.cpp",
	MAME_DIR .. "src/mame/drivers/llc2.cpp",
	MAME_DIR .. "src/mame/drivers/nanos.cpp",
	MAME_DIR .. "src/mame/drivers/pcm.cpp",
	MAME_DIR .. "src/mame/drivers/vcs80.cpp",
	MAME_DIR .. "src/mame/machine/k7659kb.cpp",
	MAME_DIR .. "src/mame/machine/k7659kb.h",
}

createMESSProjects(_target, _subtarget, "dec")
files {
	MAME_DIR .. "src/mame/drivers/dct11em.cpp",
	MAME_DIR .. "src/mame/drivers/decmate2.cpp",
	MAME_DIR .. "src/mame/drivers/decstation.cpp",
	MAME_DIR .. "src/mame/machine/decioga.cpp",
	MAME_DIR .. "src/mame/machine/decioga.h",
	MAME_DIR .. "src/mame/drivers/dectalk.cpp",
	MAME_DIR .. "src/mame/drivers/decwritr.cpp",
	MAME_DIR .. "src/mame/machine/dc305.cpp",
	MAME_DIR .. "src/mame/machine/dc305.h",
	MAME_DIR .. "src/mame/drivers/jensen.cpp",
	MAME_DIR .. "src/mame/drivers/pdp11.cpp",
	MAME_DIR .. "src/mame/drivers/vax11.cpp",
	MAME_DIR .. "src/mame/drivers/rainbow.cpp",
	MAME_DIR .. "src/mame/drivers/vk100.cpp",
	MAME_DIR .. "src/mame/drivers/vt52.cpp",
	MAME_DIR .. "src/mame/drivers/vt62.cpp",
	MAME_DIR .. "src/mame/drivers/vt100.cpp",
	MAME_DIR .. "src/mame/drivers/vt220.cpp",
	MAME_DIR .. "src/mame/drivers/vt240.cpp",
	MAME_DIR .. "src/mame/drivers/vt320.cpp",
	MAME_DIR .. "src/mame/drivers/vt520.cpp",
	MAME_DIR .. "src/mame/machine/dec_lk201.cpp",
	MAME_DIR .. "src/mame/machine/dec_lk201.h",
	MAME_DIR .. "src/mame/machine/rx01.cpp",
	MAME_DIR .. "src/mame/machine/rx01.h",
	MAME_DIR .. "src/mame/machine/vt100_kbd.cpp",
	MAME_DIR .. "src/mame/machine/vt100_kbd.h",
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

createMESSProjects(_target, _subtarget, "dulmont")
files {
	MAME_DIR .. "src/mame/drivers/magnum.cpp",
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

createMESSProjects(_target, _subtarget, "elektor")
files {
	MAME_DIR .. "src/mame/drivers/avrmax.cpp",
	MAME_DIR .. "src/mame/drivers/ec65.cpp",
	MAME_DIR .. "src/mame/drivers/elekscmp.cpp",
	MAME_DIR .. "src/mame/drivers/junior.cpp",
}

createMESSProjects(_target, _subtarget, "elektron")
files {
	MAME_DIR .. "src/mame/drivers/elektronmono.cpp",
}

createMESSProjects(_target, _subtarget, "elektronika")
files {
	MAME_DIR .. "src/mame/drivers/bk.cpp",
	MAME_DIR .. "src/mame/includes/bk.h",
	MAME_DIR .. "src/mame/machine/bk.cpp",
	MAME_DIR .. "src/mame/drivers/dvk_kcgd.cpp",
	MAME_DIR .. "src/mame/drivers/dvk_ksm.cpp",
	MAME_DIR .. "src/mame/drivers/im01.cpp",
	MAME_DIR .. "src/mame/machine/ms7004.cpp",
	MAME_DIR .. "src/mame/machine/ms7004.h",
	MAME_DIR .. "src/mame/drivers/mk85.cpp",
	MAME_DIR .. "src/mame/drivers/mk90.cpp",
	MAME_DIR .. "src/mame/drivers/ms6102.cpp",
	MAME_DIR .. "src/mame/machine/kr1601rr1.cpp",
	MAME_DIR .. "src/mame/machine/kr1601rr1.h",
}

createMESSProjects(_target, _subtarget, "emusys")
files {
	MAME_DIR .. "src/mame/drivers/emax.cpp",
	MAME_DIR .. "src/mame/drivers/emu2.cpp",
	MAME_DIR .. "src/mame/drivers/emu3.cpp",
	MAME_DIR .. "src/mame/drivers/emu68k.cpp",
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
	MAME_DIR .. "src/mame/machine/esqlcd.cpp",
	MAME_DIR .. "src/mame/machine/esqlcd.h",
}

createMESSProjects(_target, _subtarget, "enterprise")
files {
	MAME_DIR .. "src/mame/drivers/ep64.cpp",
	MAME_DIR .. "src/mame/video/nick.cpp",
	MAME_DIR .. "src/mame/video/nick.h",
}

createMESSProjects(_target, _subtarget, "entex")
files {
	MAME_DIR .. "src/mame/drivers/advision.cpp",
	MAME_DIR .. "src/mame/includes/advision.h",
	MAME_DIR .. "src/mame/machine/advision.cpp",
	MAME_DIR .. "src/mame/video/advision.cpp",
	MAME_DIR .. "src/mame/drivers/sag.cpp",
}

createMESSProjects(_target, _subtarget, "epoch")
files {
	MAME_DIR .. "src/mame/drivers/gamepock.cpp",
	MAME_DIR .. "src/mame/includes/gamepock.h",
	MAME_DIR .. "src/mame/machine/gamepock.cpp",
	MAME_DIR .. "src/mame/drivers/scv.cpp",
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

createMESSProjects(_target, _subtarget, "ericsson")
files {
	MAME_DIR .. "src/mame/drivers/e9161.cpp",
	MAME_DIR .. "src/mame/drivers/eispc.cpp",
	MAME_DIR .. "src/mame/machine/eispc_kb.cpp",
	MAME_DIR .. "src/mame/machine/eispc_kb.h",
}

createMESSProjects(_target, _subtarget, "exidy")
files {
	MAME_DIR .. "src/mame/machine/sorcerer.cpp",
	MAME_DIR .. "src/mame/drivers/sorcerer.cpp",
	MAME_DIR .. "src/mame/includes/sorcerer.h",
	MAME_DIR .. "src/mame/machine/micropolis.cpp",
	MAME_DIR .. "src/mame/machine/micropolis.h",
}

createMESSProjects(_target, _subtarget, "exorterm")
files {
	MAME_DIR .. "src/mame/drivers/exorterm.cpp",
}

createMESSProjects(_target, _subtarget, "fairch")
files {
	MAME_DIR .. "src/mame/drivers/channelf.cpp",
	MAME_DIR .. "src/mame/includes/channelf.h",
	MAME_DIR .. "src/mame/audio/channelf.cpp",
	MAME_DIR .. "src/mame/audio/channelf.h",
	MAME_DIR .. "src/mame/video/channelf.cpp",
	MAME_DIR .. "src/mame/drivers/f387x.cpp",
}

createMESSProjects(_target, _subtarget, "fairlight")
files {
	MAME_DIR .. "src/mame/drivers/cmi.cpp",
	MAME_DIR .. "src/mame/audio/cmi01a.cpp",
	MAME_DIR .. "src/mame/audio/cmi01a.h",
	MAME_DIR .. "src/mame/machine/cmi_ankbd.cpp",
	MAME_DIR .. "src/mame/machine/cmi_ankbd.h",
	MAME_DIR .. "src/mame/machine/cmi_mkbd.cpp",
	MAME_DIR .. "src/mame/machine/cmi_mkbd.h",
}

createMESSProjects(_target, _subtarget, "falco")
files {
	MAME_DIR .. "src/mame/drivers/falco500.cpp",
	MAME_DIR .. "src/mame/machine/f5220_kbd.cpp",
	MAME_DIR .. "src/mame/machine/f5220_kbd.h",
	MAME_DIR .. "src/mame/drivers/falcots.cpp",
	MAME_DIR .. "src/mame/drivers/falcots28.cpp",
}

createMESSProjects(_target, _subtarget, "fidelity")
files {
	MAME_DIR .. "src/mame/machine/fidel_clockdiv.cpp",
	MAME_DIR .. "src/mame/machine/fidel_clockdiv.h",
	MAME_DIR .. "src/mame/drivers/fidel_as12.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_card.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_cc1.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_cc10.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_cc7.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_checkc2.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_chesster.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_csc.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_dames.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_desdis.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_eag68k.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_eldorado.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_elite.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_excel.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_msc.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_phantom.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_sc12.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_sc6.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_sc8.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_sc9.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_vcc.cpp",
	MAME_DIR .. "src/mame/drivers/fidel_vsc.cpp",
}

createMESSProjects(_target, _subtarget, "force")
files {
	MAME_DIR .. "src/mame/drivers/miniforce.cpp",
	MAME_DIR .. "src/mame/drivers/fccpu20.cpp",
	MAME_DIR .. "src/mame/drivers/fccpu30.cpp",
	MAME_DIR .. "src/mame/drivers/force68k.cpp",
}

createMESSProjects(_target, _subtarget, "francedr")
files {
	MAME_DIR .. "src/mame/drivers/regence.cpp",
}

createMESSProjects(_target, _subtarget, "fujitsu")
files {
	MAME_DIR .. "src/mame/drivers/fmtowns.cpp",
	MAME_DIR .. "src/mame/includes/fmtowns.h",
	MAME_DIR .. "src/mame/video/fmtowns.cpp",
	MAME_DIR .. "src/mame/machine/fm_scsi.cpp",
	MAME_DIR .. "src/mame/machine/fm_scsi.h",
	MAME_DIR .. "src/mame/machine/fmt_icmem.cpp",
	MAME_DIR .. "src/mame/machine/fmt_icmem.h",
	MAME_DIR .. "src/mame/drivers/fm7.cpp",
	MAME_DIR .. "src/mame/includes/fm7.h",
	MAME_DIR .. "src/mame/video/fm7.cpp",
}

createMESSProjects(_target, _subtarget, "funtech")
files {
	MAME_DIR .. "src/mame/drivers/supracan.cpp",
	MAME_DIR .. "src/mame/audio/acan.cpp",
	MAME_DIR .. "src/mame/audio/acan.h",
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

createMESSProjects(_target, _subtarget, "gridcomp")
files {
	MAME_DIR .. "src/mame/drivers/gridcomp.cpp",
	MAME_DIR .. "src/mame/machine/gridkeyb.cpp",
	MAME_DIR .. "src/mame/machine/gridkeyb.h",
}

createMESSProjects(_target, _subtarget, "grundy")
files {
	MAME_DIR .. "src/mame/drivers/newbrain.cpp",
	MAME_DIR .. "src/mame/includes/newbrain.h",
	MAME_DIR .. "src/mame/video/newbrain.cpp",
}

createMESSProjects(_target, _subtarget, "h01x")
files {
	MAME_DIR .. "src/mame/drivers/h01x.cpp",
}

createMESSProjects(_target, _subtarget, "hartung")
files {
	MAME_DIR .. "src/mame/drivers/gmaster.cpp",
}

createMESSProjects(_target, _subtarget, "hds")
files {
	MAME_DIR .. "src/mame/drivers/hds200.cpp",
	MAME_DIR .. "src/mame/machine/hds200_kbd.cpp",
	MAME_DIR .. "src/mame/machine/hds200_kbd.h",
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
	MAME_DIR .. "src/mame/drivers/mephisto_academy.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_amsterdam.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_berlin.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_brikett.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_glasgow.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_milano.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_mm1.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_mm2.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_modena.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_modular.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_modular_tm.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_mondial.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_mondial2.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_mondial68k.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_montec.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_polgar.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_risc.cpp",
	MAME_DIR .. "src/mame/drivers/mephisto_smondial.cpp",
	MAME_DIR .. "src/mame/machine/mmboard.cpp",
	MAME_DIR .. "src/mame/machine/mmboard.h",
	MAME_DIR .. "src/mame/video/mmdisplay1.cpp",
	MAME_DIR .. "src/mame/video/mmdisplay1.h",
	MAME_DIR .. "src/mame/video/mmdisplay2.cpp",
	MAME_DIR .. "src/mame/video/mmdisplay2.h",
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
	MAME_DIR .. "src/mame/drivers/lft_chiptune.cpp",
	MAME_DIR .. "src/mame/drivers/lft_craft.cpp",
	MAME_DIR .. "src/mame/drivers/lft_phasor.cpp",
	MAME_DIR .. "src/mame/drivers/dcebridge.cpp",
	MAME_DIR .. "src/mame/drivers/homez80.cpp",
	MAME_DIR .. "src/mame/drivers/mk1forth.cpp",
	MAME_DIR .. "src/mame/drivers/p112.cpp",
	MAME_DIR .. "src/mame/drivers/phunsy.cpp",
	MAME_DIR .. "src/mame/drivers/pimps.cpp",
	MAME_DIR .. "src/mame/drivers/ravens.cpp",
	MAME_DIR .. "src/mame/drivers/sbc6510.cpp",
	MAME_DIR .. "src/mame/drivers/sitcom.cpp",
	MAME_DIR .. "src/mame/drivers/slc1.cpp",
	MAME_DIR .. "src/mame/drivers/test_t400.cpp",
	MAME_DIR .. "src/mame/drivers/uzebox.cpp",
	MAME_DIR .. "src/mame/drivers/z80clock.cpp",
	MAME_DIR .. "src/mame/drivers/z80dev.cpp",
	MAME_DIR .. "src/mame/drivers/zexall.cpp",
}

createMESSProjects(_target, _subtarget, "homelab")
files {
	MAME_DIR .. "src/mame/drivers/braiplus.cpp",
	MAME_DIR .. "src/mame/drivers/homelab.cpp",
}

createMESSProjects(_target, _subtarget, "hp")
files {
	MAME_DIR .. "src/mame/drivers/hp16500.cpp",
	MAME_DIR .. "src/mame/drivers/hp48.cpp",
	MAME_DIR .. "src/mame/includes/hp48.h",
	MAME_DIR .. "src/mame/machine/hp48.cpp",
	MAME_DIR .. "src/mame/machine/hp48_port.cpp",
	MAME_DIR .. "src/mame/machine/hp48_port.h",
	MAME_DIR .. "src/mame/machine/hp80_optrom.cpp",
	MAME_DIR .. "src/mame/machine/hp80_optrom.h",
	MAME_DIR .. "src/mame/machine/hp9825_optrom.cpp",
	MAME_DIR .. "src/mame/machine/hp9825_optrom.h",
	MAME_DIR .. "src/mame/machine/hp9825_tape.cpp",
	MAME_DIR .. "src/mame/machine/hp9825_tape.h",
	MAME_DIR .. "src/mame/machine/hp9845_optrom.cpp",
	MAME_DIR .. "src/mame/machine/hp9845_optrom.h",
	MAME_DIR .. "src/mame/machine/hp9845_printer.cpp",
	MAME_DIR .. "src/mame/machine/hp9845_printer.h",
	MAME_DIR .. "src/mame/machine/hp98x5_io_sys.cpp",
	MAME_DIR .. "src/mame/machine/hp98x5_io_sys.h",
	MAME_DIR .. "src/mame/machine/hp_ipc_optrom.cpp",
	MAME_DIR .. "src/mame/machine/hp_ipc_optrom.h",
	MAME_DIR .. "src/mame/video/hp48.cpp",
	MAME_DIR .. "src/mame/drivers/hp49gp.cpp",
	MAME_DIR .. "src/mame/drivers/hp9845.cpp",
	MAME_DIR .. "src/mame/drivers/hp9k.cpp",
	MAME_DIR .. "src/mame/drivers/hp9k_3xx.cpp",
	MAME_DIR .. "src/mame/drivers/hp64k.cpp",
	MAME_DIR .. "src/mame/drivers/hp_ipc.cpp",
	MAME_DIR .. "src/mame/drivers/hp80.cpp",
	MAME_DIR .. "src/mame/drivers/hp2100.cpp",
	MAME_DIR .. "src/mame/drivers/hp2620.cpp",
	MAME_DIR .. "src/mame/drivers/hp700.cpp",
	MAME_DIR .. "src/mame/machine/hp2640_tape.cpp",
	MAME_DIR .. "src/mame/machine/hp2640_tape.h",
	MAME_DIR .. "src/mame/drivers/hp2640.cpp",
	MAME_DIR .. "src/mame/drivers/hp95lx.cpp",
	MAME_DIR .. "src/mame/drivers/hp9825.cpp",
	MAME_DIR .. "src/mame/drivers/jornada.cpp",
}

createMESSProjects(_target, _subtarget, "hec2hrp")
files {
	MAME_DIR .. "src/mame/drivers/hec2hrp.cpp",
	MAME_DIR .. "src/mame/includes/hec2hrp.h",
	MAME_DIR .. "src/mame/machine/hec2hrp.cpp",
	MAME_DIR .. "src/mame/video/hec2hrp.cpp",
}

createMESSProjects(_target, _subtarget, "heurikon")
files {
	MAME_DIR .. "src/mame/drivers/hk68v10.cpp",
}

createMESSProjects(_target, _subtarget, "husky")
files {
	MAME_DIR .. "src/mame/drivers/hawk.cpp",
	MAME_DIR .. "src/mame/drivers/hunter2.cpp",
	MAME_DIR .. "src/mame/drivers/hunter16.cpp",
	MAME_DIR .. "src/mame/drivers/husky.cpp",
}

createMESSProjects(_target, _subtarget, "ibm6580")
files {
	MAME_DIR .. "src/mame/drivers/ibm6580.cpp",
	MAME_DIR .. "src/mame/machine/ibm6580_kbd.cpp",
	MAME_DIR .. "src/mame/machine/ibm6580_kbd.h",
	MAME_DIR .. "src/mame/machine/ibm6580_fdc.cpp",
	MAME_DIR .. "src/mame/machine/ibm6580_fdc.h",
}

createMESSProjects(_target, _subtarget, "ie15")
files {
	MAME_DIR .. "src/mame/drivers/ie15.cpp",
}

createMESSProjects(_target, _subtarget, "informer")
files {
	MAME_DIR .. "src/mame/drivers/informer_207_100.cpp",
	MAME_DIR .. "src/mame/drivers/informer_207_376.cpp",
	MAME_DIR .. "src/mame/drivers/informer_213.cpp",
	MAME_DIR .. "src/mame/machine/informer_207_376_kbd.cpp",
	MAME_DIR .. "src/mame/machine/informer_207_376_kbd.h",
	MAME_DIR .. "src/mame/machine/informer_213_kbd.cpp",
	MAME_DIR .. "src/mame/machine/informer_213_kbd.h",
}

createMESSProjects(_target, _subtarget, "intel")
files {
	MAME_DIR .. "src/mame/drivers/basic52.cpp",
	MAME_DIR .. "src/mame/drivers/imds2.cpp",
	MAME_DIR .. "src/mame/drivers/intellec4.cpp",
	MAME_DIR .. "src/mame/drivers/intellec8.cpp",
	MAME_DIR .. "src/mame/drivers/ipc.cpp",
	MAME_DIR .. "src/mame/drivers/ipds.cpp",
	MAME_DIR .. "src/mame/drivers/isbc.cpp",
	MAME_DIR .. "src/mame/drivers/isbc8010.cpp",
	MAME_DIR .. "src/mame/drivers/isbc8030.cpp",
	MAME_DIR .. "src/mame/machine/imm6_76.cpp",
	MAME_DIR .. "src/mame/machine/imm6_76.h",
	MAME_DIR .. "src/mame/machine/isbc_215g.cpp",
	MAME_DIR .. "src/mame/machine/isbc_215g.h",
	MAME_DIR .. "src/mame/machine/isbc_208.cpp",
	MAME_DIR .. "src/mame/machine/isbc_208.h",
	MAME_DIR .. "src/mame/drivers/rex6000.cpp",
	MAME_DIR .. "src/mame/drivers/sdk51.cpp",
	MAME_DIR .. "src/mame/drivers/sdk80.cpp",
	MAME_DIR .. "src/mame/drivers/sdk85.cpp",
	MAME_DIR .. "src/mame/drivers/sdk86.cpp",
	MAME_DIR .. "src/mame/machine/imds2ioc.cpp",
	MAME_DIR .. "src/mame/machine/imds2ioc.h",
}

createMESSProjects(_target, _subtarget, "imp")
files {
	MAME_DIR .. "src/mame/drivers/tim011.cpp",
	MAME_DIR .. "src/mame/drivers/tim100.cpp",
}

createMESSProjects(_target, _subtarget, "interpro")
files {
	MAME_DIR .. "src/mame/drivers/interpro.cpp",
	MAME_DIR .. "src/mame/machine/cammu.h",
	MAME_DIR .. "src/mame/machine/cammu.cpp",
	MAME_DIR .. "src/mame/machine/interpro_ioga.h",
	MAME_DIR .. "src/mame/machine/interpro_ioga.cpp",
	MAME_DIR .. "src/mame/machine/interpro_mcga.h",
	MAME_DIR .. "src/mame/machine/interpro_mcga.cpp",
	MAME_DIR .. "src/mame/machine/interpro_sga.h",
	MAME_DIR .. "src/mame/machine/interpro_sga.cpp",
	MAME_DIR .. "src/mame/machine/interpro_arbga.h",
	MAME_DIR .. "src/mame/machine/interpro_arbga.cpp",
}

createMESSProjects(_target, _subtarget, "interton")
files {
	MAME_DIR .. "src/mame/drivers/vc4000.cpp",
	MAME_DIR .. "src/mame/includes/vc4000.h",
	MAME_DIR .. "src/mame/audio/vc4000.cpp",
	MAME_DIR .. "src/mame/audio/vc4000.h",
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

createMESSProjects(_target, _subtarget, "jazz")
files {
	MAME_DIR .. "src/mame/drivers/jazz.cpp",
	MAME_DIR .. "src/mame/machine/mct_adr.cpp",
	MAME_DIR .. "src/mame/machine/mct_adr.h",
}

createMESSProjects(_target, _subtarget, "kawai")
files {
	MAME_DIR .. "src/mame/drivers/kawai_acr20.cpp",
	MAME_DIR .. "src/mame/drivers/kawai_k1.cpp",
	MAME_DIR .. "src/mame/drivers/kawai_k4.cpp",
	MAME_DIR .. "src/mame/drivers/kawai_k5.cpp",
	MAME_DIR .. "src/mame/drivers/kawai_ksp10.cpp",
	MAME_DIR .. "src/mame/drivers/kawai_r100.cpp",
	MAME_DIR .. "src/mame/drivers/kawai_sx240.cpp",
}

createMESSProjects(_target, _subtarget, "kaypro")
files {
	MAME_DIR .. "src/mame/drivers/kaypro.cpp",
	MAME_DIR .. "src/mame/includes/kaypro.h",
	MAME_DIR .. "src/mame/machine/kaypro.cpp",
	MAME_DIR .. "src/mame/machine/kay_kbd.cpp",
	MAME_DIR .. "src/mame/machine/kay_kbd.h",
	MAME_DIR .. "src/mame/video/kaypro.cpp",
}

createMESSProjects(_target, _subtarget, "koei")
files {
	MAME_DIR .. "src/mame/drivers/pasogo.cpp",
}

createMESSProjects(_target, _subtarget, "kontron")
files {
	MAME_DIR .. "src/mame/drivers/kdt6.cpp",
}

createMESSProjects(_target, _subtarget, "korg")
files {
	MAME_DIR .. "src/mame/drivers/korgds8.cpp",
	MAME_DIR .. "src/mame/drivers/korgdss1.cpp",
	MAME_DIR .. "src/mame/drivers/korgdvp1.cpp",
	MAME_DIR .. "src/mame/drivers/korgdw8k.cpp",
	MAME_DIR .. "src/mame/drivers/korgm1.cpp",
	MAME_DIR .. "src/mame/drivers/korgws.cpp",
	MAME_DIR .. "src/mame/drivers/korgz3.cpp",
	MAME_DIR .. "src/mame/drivers/microkorg.cpp",
	MAME_DIR .. "src/mame/drivers/poly800.cpp",
	MAME_DIR .. "src/mame/drivers/polysix.cpp",
}

createMESSProjects(_target, _subtarget, "kurzweil")
files {
	MAME_DIR .. "src/mame/drivers/krz2000.cpp",
}

createMESSProjects(_target, _subtarget, "kyber")
files {
	MAME_DIR .. "src/mame/drivers/kminus.cpp",
}

createMESSProjects(_target, _subtarget, "kyocera")
files {
	MAME_DIR .. "src/mame/drivers/kyocera.cpp",
	MAME_DIR .. "src/mame/includes/kyocera.h",
	MAME_DIR .. "src/mame/video/kyocera.cpp",
}

createMESSProjects(_target, _subtarget, "leapfrog")
files {
	MAME_DIR .. "src/mame/drivers/leapster.cpp",
	MAME_DIR .. "src/mame/drivers/leapfrog_leappad.cpp",
	MAME_DIR .. "src/mame/drivers/leapfrog_leapster_explorer.cpp",
	MAME_DIR .. "src/mame/drivers/leapfrog_iquest.cpp",
}

createMESSProjects(_target, _subtarget, "learsiegler")
files {
	MAME_DIR .. "src/mame/drivers/adm11.cpp",
	MAME_DIR .. "src/mame/drivers/adm23.cpp",
	MAME_DIR .. "src/mame/drivers/adm31.cpp",
	MAME_DIR .. "src/mame/drivers/adm36.cpp",
}

createMESSProjects(_target, _subtarget, "lsi")
files {
	MAME_DIR .. "src/mame/drivers/m3.cpp",
	MAME_DIR .. "src/mame/drivers/octopus.cpp",
	MAME_DIR .. "src/mame/machine/octo_kbd.cpp",
	MAME_DIR .. "src/mame/machine/octo_kbd.h",
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

createMESSProjects(_target, _subtarget, "mattel")
files {
	MAME_DIR .. "src/mame/drivers/aquarius.cpp",
	MAME_DIR .. "src/mame/includes/aquarius.h",
	MAME_DIR .. "src/mame/video/aquarius.cpp",
	MAME_DIR .. "src/mame/drivers/juicebox.cpp",
	MAME_DIR .. "src/mame/drivers/mattelchess.cpp",
}

createMESSProjects(_target, _subtarget, "matsushi")
files {
	MAME_DIR .. "src/mame/drivers/jr100.cpp",
	MAME_DIR .. "src/mame/drivers/jr200.cpp",
	MAME_DIR .. "src/mame/drivers/myb3k.cpp",
	MAME_DIR .. "src/mame/drivers/duet16.cpp",
}

createMESSProjects(_target, _subtarget, "mb")
files {
	MAME_DIR .. "src/mame/drivers/microvsn.cpp",
	MAME_DIR .. "src/mame/drivers/milton6805.cpp",
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

createMESSProjects(_target, _subtarget, "mera")
files {
	MAME_DIR .. "src/mame/drivers/ec7915.cpp",
	MAME_DIR .. "src/mame/drivers/konin.cpp",
	MAME_DIR .. "src/mame/drivers/m79152pc.cpp",
	MAME_DIR .. "src/mame/drivers/meritum.cpp",
	MAME_DIR .. "src/mame/drivers/vdm7932x.cpp",
}

createMESSProjects(_target, _subtarget, "mg1")
files {
	MAME_DIR .. "src/mame/drivers/mg1.cpp",
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
}

createMESSProjects(_target, _subtarget, "microsoft")
files {
	MAME_DIR .. "src/mame/drivers/xbox.cpp",
	MAME_DIR .. "src/mame/includes/xbox.h",
	MAME_DIR .. "src/mame/includes/xbox_usb.h",
	MAME_DIR .. "src/mame/includes/xbox_pci.h",
}

createMESSProjects(_target, _subtarget, "microterm")
files {
	MAME_DIR .. "src/mame/drivers/ergo201.cpp",
	MAME_DIR .. "src/mame/drivers/microterm_f8.cpp",
	MAME_DIR .. "src/mame/drivers/mt420.cpp",
	MAME_DIR .. "src/mame/drivers/mt5510.cpp",
}

createMESSProjects(_target, _subtarget, "mips")
files {
	MAME_DIR .. "src/mame/drivers/mips.cpp",
	MAME_DIR .. "src/mame/machine/mips_rambo.h",
	MAME_DIR .. "src/mame/machine/mips_rambo.cpp",
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
	MAME_DIR .. "src/mame/drivers/exorciser.cpp",
	MAME_DIR .. "src/mame/drivers/m6805evs.cpp",
	MAME_DIR .. "src/mame/drivers/m68705prg.cpp",
	MAME_DIR .. "src/mame/drivers/mekd1.cpp",
	MAME_DIR .. "src/mame/drivers/mekd2.cpp",
	MAME_DIR .. "src/mame/drivers/mekd3.cpp",
	MAME_DIR .. "src/mame/drivers/mekd4.cpp",
	MAME_DIR .. "src/mame/drivers/mekd5.cpp",
	MAME_DIR .. "src/mame/drivers/mvme147.cpp",
	MAME_DIR .. "src/mame/drivers/mvme162.cpp",
	MAME_DIR .. "src/mame/drivers/sys1121.cpp",
	MAME_DIR .. "src/mame/drivers/uchroma68.cpp",
}

createMESSProjects(_target, _subtarget, "multitch")
files {
	MAME_DIR .. "src/mame/drivers/mkit09.cpp",
	MAME_DIR .. "src/mame/drivers/mpf1.cpp",
	MAME_DIR .. "src/mame/includes/mpf1.h",
}

createMESSProjects(_target, _subtarget, "mupid")
files {
	MAME_DIR .. "src/mame/drivers/mdisk.cpp",
	MAME_DIR .. "src/mame/drivers/mupid2.cpp",
}

createMESSProjects(_target, _subtarget, "nakajima")
files {
	MAME_DIR .. "src/mame/drivers/nakajies.cpp",
}

createMESSProjects(_target, _subtarget, "nascom")
files {
	MAME_DIR .. "src/mame/drivers/nascom1.cpp",
}

createMESSProjects(_target, _subtarget, "natsemi")
files {
	MAME_DIR .. "src/mame/drivers/hh_cop400.cpp",
	MAME_DIR .. "src/mame/drivers/hh_cops1.cpp",
	MAME_DIR .. "src/mame/drivers/ns5652.cpp",
	MAME_DIR .. "src/mame/drivers/ns32kdb.cpp",
}

createMESSProjects(_target, _subtarget, "ncd")
files {
	MAME_DIR .. "src/mame/drivers/ncd68k.cpp",
	MAME_DIR .. "src/mame/drivers/ncd88k.cpp",
	MAME_DIR .. "src/mame/drivers/ncdmips.cpp",
	MAME_DIR .. "src/mame/drivers/ncdppc.cpp",
	MAME_DIR .. "src/mame/machine/bert.cpp",
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
	MAME_DIR .. "src/mame/drivers/ews4800.cpp",
	MAME_DIR .. "src/mame/drivers/hh_ucom4.cpp",
	MAME_DIR .. "src/mame/drivers/pce.cpp",
	MAME_DIR .. "src/mame/includes/pce.h",
	MAME_DIR .. "src/mame/machine/pce.cpp",
	MAME_DIR .. "src/mame/machine/pce_cd.cpp",
	MAME_DIR .. "src/mame/machine/pce_cd.h",
	MAME_DIR .. "src/mame/drivers/pcfx.cpp",
	MAME_DIR .. "src/mame/drivers/pc6001.cpp",
	MAME_DIR .. "src/mame/includes/pc6001.h",
	MAME_DIR .. "src/mame/video/pc6001.cpp",
	MAME_DIR .. "src/mame/drivers/pc8401a.cpp",
	MAME_DIR .. "src/mame/includes/pc8401a.h",
	MAME_DIR .. "src/mame/video/pc8401a.cpp",
	MAME_DIR .. "src/mame/machine/pc80s31k.cpp",
	MAME_DIR .. "src/mame/machine/pc80s31k.h",
	MAME_DIR .. "src/mame/drivers/pc8001.cpp",
	MAME_DIR .. "src/mame/includes/pc8001.h",
	MAME_DIR .. "src/mame/drivers/pc8801.cpp",
	MAME_DIR .. "src/mame/includes/pc8801.h",
	MAME_DIR .. "src/mame/drivers/pc88va.cpp",
	MAME_DIR .. "src/mame/includes/pc88va.h",
	MAME_DIR .. "src/mame/drivers/pc100.cpp",
	MAME_DIR .. "src/mame/drivers/pc9801.cpp",
	MAME_DIR .. "src/mame/includes/pc9801.h",
	MAME_DIR .. "src/mame/video/pc9801.cpp",
	MAME_DIR .. "src/mame/machine/pc9801_kbd.cpp",
	MAME_DIR .. "src/mame/machine/pc9801_kbd.h",
	MAME_DIR .. "src/mame/machine/pc9801_cd.cpp",
	MAME_DIR .. "src/mame/machine/pc9801_cd.h",
	MAME_DIR .. "src/mame/machine/pc9801_memsw.cpp",
	MAME_DIR .. "src/mame/machine/pc9801_memsw.h",
	MAME_DIR .. "src/mame/drivers/pc98ha.cpp",
	MAME_DIR .. "src/mame/includes/pc98ha.h",
	MAME_DIR .. "src/mame/drivers/pc9801_epson.cpp",
	MAME_DIR .. "src/mame/includes/pc9801_epson.h",
	MAME_DIR .. "src/mame/drivers/pc9821.cpp",
	MAME_DIR .. "src/mame/includes/pc9821.h",
	MAME_DIR .. "src/mame/drivers/tk80bs.cpp",
}

createMESSProjects(_target, _subtarget, "netronic")
files {
	MAME_DIR .. "src/mame/drivers/elf.cpp",
	MAME_DIR .. "src/mame/includes/elf.h",
	MAME_DIR .. "src/mame/drivers/exp85.cpp",
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
	MAME_DIR .. "src/mame/machine/gb.cpp",
	MAME_DIR .. "src/mame/drivers/gba.cpp",
	MAME_DIR .. "src/mame/includes/gba.h",
	MAME_DIR .. "src/mame/drivers/n64.cpp",
	MAME_DIR .. "src/mame/includes/n64.h",
	MAME_DIR .. "src/mame/drivers/nds.cpp",
	MAME_DIR .. "src/mame/includes/nds.h",
	MAME_DIR .. "src/mame/drivers/nes.cpp",
	MAME_DIR .. "src/mame/includes/nes.h",
	MAME_DIR .. "src/mame/machine/nes.cpp",
	MAME_DIR .. "src/mame/video/nes.cpp",
	MAME_DIR .. "src/mame/drivers/nes_vt02_vt03.cpp",
	MAME_DIR .. "src/mame/drivers/nes_vt09.cpp",
	MAME_DIR .. "src/mame/drivers/nes_vt32.cpp",
	MAME_DIR .. "src/mame/drivers/nes_vt369_vtunknown.cpp",
	MAME_DIR .. "src/mame/machine/nes_vt_soc.cpp",
	MAME_DIR .. "src/mame/machine/nes_vt_soc.h",
	MAME_DIR .. "src/mame/machine/nes_vt09_soc.cpp",
	MAME_DIR .. "src/mame/machine/nes_vt09_soc.h",
	MAME_DIR .. "src/mame/machine/nes_vt32_soc.cpp",
	MAME_DIR .. "src/mame/machine/nes_vt32_soc.h",
	MAME_DIR .. "src/mame/machine/nes_vt369_vtunknown_soc.cpp",
	MAME_DIR .. "src/mame/machine/nes_vt369_vtunknown_soc.h",
	MAME_DIR .. "src/mame/drivers/nes_sh6578.cpp",
	MAME_DIR .. "src/mame/drivers/nes_clone.cpp",
	MAME_DIR .. "src/mame/drivers/pokemini.cpp",
	MAME_DIR .. "src/mame/drivers/snes.cpp",
	MAME_DIR .. "src/mame/includes/snes.h",
	MAME_DIR .. "src/mame/machine/snescx4.cpp",
	MAME_DIR .. "src/mame/machine/snescx4.h",
	MAME_DIR .. "src/mame/machine/cx4data.hxx",
	MAME_DIR .. "src/mame/machine/cx4fn.hxx",
	MAME_DIR .. "src/mame/machine/cx4oam.hxx",
	MAME_DIR .. "src/mame/machine/cx4ops.hxx",
	MAME_DIR .. "src/mame/drivers/vboy.cpp",
	MAME_DIR .. "src/mame/audio/vboy.cpp",
	MAME_DIR .. "src/mame/audio/vboy.h",
	MAME_DIR .. "src/mame/drivers/gamecube.cpp",
	MAME_DIR .. "src/mame/machine/m6502_vtscr.cpp",
	MAME_DIR .. "src/mame/machine/m6502_vtscr.h",
	MAME_DIR .. "src/mame/machine/m6502_swap_op_d5_d6.cpp",
	MAME_DIR .. "src/mame/machine/m6502_swap_op_d5_d6.h",
	MAME_DIR .. "src/mame/machine/m6502_swap_op_d2_d7.cpp",
	MAME_DIR .. "src/mame/machine/m6502_swap_op_d2_d7.h",
	MAME_DIR .. "src/mame/drivers/vt1682.cpp",
	MAME_DIR .. "src/mame/machine/vt1682_io.h",
	MAME_DIR .. "src/mame/machine/vt1682_io.cpp",
	MAME_DIR .. "src/mame/machine/vt1682_uio.h",
	MAME_DIR .. "src/mame/machine/vt1682_uio.cpp",
	MAME_DIR .. "src/mame/machine/vt1682_alu.h",
	MAME_DIR .. "src/mame/machine/vt1682_alu.cpp",
	MAME_DIR .. "src/mame/machine/vt1682_timer.h",
	MAME_DIR .. "src/mame/machine/vt1682_timer.cpp",
	MAME_DIR .. "src/mame/drivers/vt_unknown.cpp",
	MAME_DIR .. "src/mame/drivers/compmahj.cpp",
}

createMESSProjects(_target, _subtarget, "nokia")
files {
	MAME_DIR .. "src/mame/drivers/dbox.cpp",
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
	MAME_DIR .. "src/mame/drivers/novag_cexpert.cpp",
	MAME_DIR .. "src/mame/drivers/novag_cforte.cpp",
	MAME_DIR .. "src/mame/drivers/novag_const.cpp",
	MAME_DIR .. "src/mame/drivers/novag_diablo.cpp",
	MAME_DIR .. "src/mame/drivers/novag_micro.cpp",
	MAME_DIR .. "src/mame/drivers/novag_micro2.cpp",
	MAME_DIR .. "src/mame/drivers/novag_savant.cpp",
	MAME_DIR .. "src/mame/drivers/novag_sexpert.cpp",
	MAME_DIR .. "src/mame/drivers/novag_snova.cpp",
}

createMESSProjects(_target, _subtarget, "novation")
files {
	MAME_DIR .. "src/mame/drivers/basssta.cpp",
	MAME_DIR .. "src/mame/drivers/drumsta.cpp",
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
	MAME_DIR .. "src/mame/drivers/olivpc1.cpp",
}

createMESSProjects(_target, _subtarget, "olympia")
files {
	MAME_DIR .. "src/mame/drivers/olyboss.cpp",
	MAME_DIR .. "src/mame/drivers/olytext.cpp",
	MAME_DIR .. "src/mame/drivers/peoplepc.cpp",
}

createMESSProjects(_target, _subtarget, "omnibyte")
files {
	MAME_DIR .. "src/mame/drivers/msbc1.cpp",
	MAME_DIR .. "src/mame/drivers/ob68k1a.cpp",
	MAME_DIR .. "src/mame/includes/ob68k1a.h",
}

createMESSProjects(_target, _subtarget, "omron")
files {
	MAME_DIR .. "src/mame/drivers/luna_68k.cpp",
}

createMESSProjects(_target, _subtarget, "openuni")
files {
	MAME_DIR .. "src/mame/drivers/hektor.cpp",
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
	MAME_DIR .. "src/mame/drivers/palm_dbg.hxx",
	MAME_DIR .. "src/mame/drivers/palmz22.cpp",
}

createMESSProjects(_target, _subtarget, "parker")
files {
	MAME_DIR .. "src/mame/drivers/talkingbb.cpp",
	MAME_DIR .. "src/mame/drivers/talkingfb.cpp",
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
	MAME_DIR .. "src/mame/drivers/ibmpc.cpp",
	MAME_DIR .. "src/mame/drivers/ibmpcjr.cpp",
	MAME_DIR .. "src/mame/drivers/nforcepc.cpp",
	MAME_DIR .. "src/mame/drivers/pc.cpp",
	MAME_DIR .. "src/mame/drivers/pcipc.cpp",
	MAME_DIR .. "src/mame/drivers/tandy1t.cpp",
	MAME_DIR .. "src/mame/drivers/tosh1000.cpp",
	MAME_DIR .. "src/mame/machine/tosh1000_bram.cpp",
	MAME_DIR .. "src/mame/machine/tosh1000_bram.h",
	MAME_DIR .. "src/mame/drivers/compc.cpp",
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
	MAME_DIR .. "src/mame/drivers/orao.cpp",
}

createMESSProjects(_target, _subtarget, "philips")
files {
	MAME_DIR .. "src/mame/drivers/p2000t.cpp",
	MAME_DIR .. "src/mame/includes/p2000t.h",
	MAME_DIR .. "src/mame/machine/p2000t.cpp",
	MAME_DIR .. "src/mame/machine/p2000t_mdcr.cpp",
	MAME_DIR .. "src/mame/video/p2000t.cpp",
	MAME_DIR .. "src/mame/drivers/vg5k.cpp",
	MAME_DIR .. "src/mame/drivers/yes.cpp",
}

createMESSProjects(_target, _subtarget, "poly")
files {
	MAME_DIR .. "src/mame/drivers/poly.cpp",
	MAME_DIR .. "src/mame/includes/poly.h",
	MAME_DIR .. "src/mame/machine/poly.cpp",
	MAME_DIR .. "src/mame/drivers/proteus.cpp",
}

createMESSProjects(_target, _subtarget, "poly88")
files {
	MAME_DIR .. "src/mame/drivers/poly88.cpp",
	MAME_DIR .. "src/mame/includes/poly88.h",
	MAME_DIR .. "src/mame/machine/poly88.cpp",
}

createMESSProjects(_target, _subtarget, "positron")
files {
	MAME_DIR .. "src/mame/drivers/positron.cpp",
}

createMESSProjects(_target, _subtarget, "psion")
files {
	MAME_DIR .. "src/mame/drivers/psion.cpp",
	MAME_DIR .. "src/mame/includes/psion.h",
	MAME_DIR .. "src/mame/drivers/psion5.cpp",
	MAME_DIR .. "src/mame/includes/psion5.h",
	MAME_DIR .. "src/mame/machine/etna.cpp",
	MAME_DIR .. "src/mame/machine/etna.h",
	MAME_DIR .. "src/mame/machine/psion_pack.cpp",
	MAME_DIR .. "src/mame/machine/psion_pack.h",
}

createMESSProjects(_target, _subtarget, "quantel")
files {
	MAME_DIR .. "src/mame/drivers/dpb7000.cpp",
	MAME_DIR .. "src/mame/drivers/harriet.cpp",
}

createMESSProjects(_target, _subtarget, "qume")
files {
	MAME_DIR .. "src/mame/drivers/qvt70.cpp",
	MAME_DIR .. "src/mame/drivers/qvt102.cpp",
	MAME_DIR .. "src/mame/drivers/qvt103.cpp",
	MAME_DIR .. "src/mame/drivers/qvt190.cpp",
	MAME_DIR .. "src/mame/drivers/qvt201.cpp",
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
	MAME_DIR .. "src/mame/drivers/rc702.cpp",
	MAME_DIR .. "src/mame/drivers/rc759.cpp",
	MAME_DIR .. "src/mame/machine/rc759_kbd.cpp",
	MAME_DIR .. "src/mame/machine/rc759_kbd.h",
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
	MAME_DIR .. "src/mame/drivers/alphajuno.cpp",
	MAME_DIR .. "src/mame/drivers/boss_se70.cpp",
	MAME_DIR .. "src/mame/drivers/boss_sx700.cpp",
	MAME_DIR .. "src/mame/drivers/juno106.cpp",
	MAME_DIR .. "src/mame/drivers/juno6.cpp",
	MAME_DIR .. "src/mame/drivers/roland_cm32p.cpp",
	MAME_DIR .. "src/mame/drivers/roland_d10.cpp",
	MAME_DIR .. "src/mame/drivers/roland_d50.cpp",
	MAME_DIR .. "src/mame/drivers/roland_jd800.cpp",
	MAME_DIR .. "src/mame/drivers/roland_jv80.cpp",
	MAME_DIR .. "src/mame/drivers/roland_jx3p.cpp",
	MAME_DIR .. "src/mame/drivers/roland_jx8p.cpp",
	MAME_DIR .. "src/mame/drivers/roland_mc50.cpp",
	MAME_DIR .. "src/mame/drivers/roland_mt32.cpp",
	MAME_DIR .. "src/mame/drivers/roland_pr100.cpp",
	MAME_DIR .. "src/mame/drivers/roland_r8.cpp",
	MAME_DIR .. "src/mame/drivers/roland_ra30.cpp",
	MAME_DIR .. "src/mame/drivers/roland_s10.cpp",
	MAME_DIR .. "src/mame/drivers/roland_s50.cpp",
	MAME_DIR .. "src/mame/drivers/roland_sc55.cpp",
	MAME_DIR .. "src/mame/drivers/roland_sc88.cpp",
	MAME_DIR .. "src/mame/drivers/roland_tb303.cpp",
	MAME_DIR .. "src/mame/drivers/roland_tr505.cpp",
	MAME_DIR .. "src/mame/drivers/roland_tr606.cpp",
	MAME_DIR .. "src/mame/drivers/roland_tr707.cpp",
	MAME_DIR .. "src/mame/drivers/roland_tr808.cpp",
	MAME_DIR .. "src/mame/drivers/roland_tr909.cpp",
	MAME_DIR .. "src/mame/drivers/roland_u20.cpp",
	MAME_DIR .. "src/mame/audio/bu3905.cpp",
	MAME_DIR .. "src/mame/audio/bu3905.h",
	MAME_DIR .. "src/mame/audio/jx8p_synth.cpp",
	MAME_DIR .. "src/mame/audio/jx8p_synth.h",
	MAME_DIR .. "src/mame/audio/mb63h114.cpp",
	MAME_DIR .. "src/mame/audio/mb63h114.h",
	MAME_DIR .. "src/mame/audio/sa16.cpp",
	MAME_DIR .. "src/mame/audio/sa16.h",
	MAME_DIR .. "src/mame/machine/mb62h195.cpp",
	MAME_DIR .. "src/mame/machine/mb62h195.h",
	MAME_DIR .. "src/mame/machine/mb63h149.cpp",
	MAME_DIR .. "src/mame/machine/mb63h149.h",
	MAME_DIR .. "src/mame/machine/mb87013.cpp",
	MAME_DIR .. "src/mame/machine/mb87013.h",
	MAME_DIR .. "src/mame/machine/pg200.cpp",
	MAME_DIR .. "src/mame/machine/pg200.h",
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
	MAME_DIR .. "src/mame/drivers/hh_pps41.cpp",
}

createMESSProjects(_target, _subtarget, "rtpc")
files {
	MAME_DIR .. "src/mame/drivers/rtpc.cpp",
	MAME_DIR .. "src/mame/machine/rosetta.cpp",
	MAME_DIR .. "src/mame/machine/rosetta.h",
	MAME_DIR .. "src/mame/machine/rtpc_iocc.cpp",
	MAME_DIR .. "src/mame/machine/rtpc_iocc.h",
}

createMESSProjects(_target, _subtarget, "sage")
files {
	MAME_DIR .. "src/mame/drivers/sage2.cpp",
	MAME_DIR .. "src/mame/includes/sage2.h",
}

createMESSProjects(_target, _subtarget, "saitek")
files {
	MAME_DIR .. "src/mame/drivers/saitek_ccompan.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_chesstrv.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_cp2000.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_delta1.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_exchess.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_intchess.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_leonardo.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_mark5.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_minichess.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_prschess.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_renaissance.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_risc2500.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_schess.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_simultano.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_ssystem3.cpp",
	MAME_DIR .. "src/mame/includes/saitek_stratos.h",
	MAME_DIR .. "src/mame/drivers/saitek_stratos.cpp",
	MAME_DIR .. "src/mame/drivers/saitek_corona.cpp", -- subdriver of saitek_stratos
	MAME_DIR .. "src/mame/drivers/saitek_superstar.cpp",
}

createMESSProjects(_target, _subtarget, "samcoupe")
files {
	MAME_DIR .. "src/mame/drivers/samcoupe.cpp",
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
	MAME_DIR .. "src/mame/video/mbc55x.cpp",
	MAME_DIR .. "src/mame/drivers/phc25.cpp",
	MAME_DIR .. "src/mame/includes/phc25.h",
	MAME_DIR .. "src/mame/machine/mbc55x_kbd.cpp",
	MAME_DIR .. "src/mame/machine/mbc55x_kbd.h",
}

createMESSProjects(_target, _subtarget, "saturn")
files {
	MAME_DIR .. "src/mame/drivers/st17xx.cpp",
}

-- Don't call this project "sega" or it collides with the arcade one
-- and merges with it, which ends up with libsega.a linked after
-- libshared.a.  The link then fails on linux because SEGAM1AUDIO and RAX
-- are in shared while model* and stv are in sega.
createMESSProjects(_target, _subtarget, "segacons")
files {
	MAME_DIR .. "src/mame/drivers/dccons.cpp",
	MAME_DIR .. "src/mame/includes/dccons.h",
	MAME_DIR .. "src/mame/machine/dccons.cpp",
	MAME_DIR .. "src/mame/machine/gdrom.cpp",
	MAME_DIR .. "src/mame/machine/gdrom.h",
	MAME_DIR .. "src/mame/drivers/megadriv.cpp",
	MAME_DIR .. "src/mame/includes/megadriv.h",
	MAME_DIR .. "src/mame/drivers/megadriv_rad.cpp",
	MAME_DIR .. "src/mame/includes/megadriv_rad.h",
	MAME_DIR .. "src/mame/drivers/megadriv_vt_hybrid.cpp",
	MAME_DIR .. "src/mame/drivers/megadriv_sunplus_hybrid.cpp",
	MAME_DIR .. "src/mame/drivers/segapico.cpp",
	MAME_DIR .. "src/mame/drivers/sega_sawatte.cpp",
	MAME_DIR .. "src/mame/drivers/sega_beena.cpp",
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
	MAME_DIR .. "src/mame/drivers/iris_power.cpp",
	MAME_DIR .. "src/mame/drivers/crimson.cpp",
	MAME_DIR .. "src/mame/drivers/o2.cpp",
	MAME_DIR .. "src/mame/drivers/octane.cpp",
	MAME_DIR .. "src/mame/machine/vino.cpp",
	MAME_DIR .. "src/mame/machine/vino.h",
	MAME_DIR .. "src/mame/machine/saa7191.cpp",
	MAME_DIR .. "src/mame/machine/saa7191.h",
	MAME_DIR .. "src/mame/machine/sgi.cpp",
	MAME_DIR .. "src/mame/machine/sgi.h",
	MAME_DIR .. "src/mame/machine/hal2.cpp",
	MAME_DIR .. "src/mame/machine/hal2.h",
	MAME_DIR .. "src/mame/machine/hpc1.cpp",
	MAME_DIR .. "src/mame/machine/hpc1.h",
	MAME_DIR .. "src/mame/machine/hpc3.cpp",
	MAME_DIR .. "src/mame/machine/hpc3.h",
	MAME_DIR .. "src/mame/machine/ioc2.cpp",
	MAME_DIR .. "src/mame/machine/ioc2.h",
	MAME_DIR .. "src/mame/machine/mace.cpp",
	MAME_DIR .. "src/mame/machine/mace.h",
	MAME_DIR .. "src/mame/drivers/iris3130.cpp",
	MAME_DIR .. "src/mame/drivers/4dpi.cpp",
	MAME_DIR .. "src/mame/drivers/indigo.cpp",
	MAME_DIR .. "src/mame/drivers/indy_indigo2.cpp",
	MAME_DIR .. "src/mame/video/light.cpp",
	MAME_DIR .. "src/mame/video/light.h",
	MAME_DIR .. "src/mame/video/crime.cpp",
	MAME_DIR .. "src/mame/video/crime.h",
	MAME_DIR .. "src/mame/video/sgi_gr1.cpp",
	MAME_DIR .. "src/mame/video/sgi_gr1.h",
	MAME_DIR .. "src/mame/video/sgi_ge5.cpp",
	MAME_DIR .. "src/mame/video/sgi_ge5.h",
	MAME_DIR .. "src/mame/video/sgi_re2.cpp",
	MAME_DIR .. "src/mame/video/sgi_re2.h",
	MAME_DIR .. "src/mame/video/sgi_xmap2.cpp",
	MAME_DIR .. "src/mame/video/sgi_xmap2.h",
}

createMESSProjects(_target, _subtarget, "sharp")
files {
	MAME_DIR .. "src/mame/drivers/hh_sm510.cpp",
	MAME_DIR .. "src/mame/includes/hh_sm510.h",
	MAME_DIR .. "src/mame/drivers/rzone.cpp", -- subdriver of hh_sm510
	MAME_DIR .. "src/mame/video/mz700.cpp",
	MAME_DIR .. "src/mame/drivers/mz700.cpp",
	MAME_DIR .. "src/mame/includes/mz700.h",
	MAME_DIR .. "src/mame/drivers/pc1500.cpp",
	MAME_DIR .. "src/mame/drivers/pocketc.cpp",
	MAME_DIR .. "src/mame/machine/pocketc.cpp",
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
	MAME_DIR .. "src/mame/video/x68k_crtc.cpp",
	MAME_DIR .. "src/mame/video/x68k_crtc.h",
	MAME_DIR .. "src/mame/drivers/mz80.cpp",
	MAME_DIR .. "src/mame/includes/mz80.h",
	MAME_DIR .. "src/mame/video/mz80.cpp",
	MAME_DIR .. "src/mame/machine/mz80.cpp",
	MAME_DIR .. "src/mame/drivers/mz2000.cpp",
	MAME_DIR .. "src/mame/drivers/wizard.cpp",
	MAME_DIR .. "src/mame/drivers/x1.cpp",
	MAME_DIR .. "src/mame/includes/x1.h",
	MAME_DIR .. "src/mame/machine/x1.cpp",
	MAME_DIR .. "src/mame/video/x1.cpp",
	MAME_DIR .. "src/mame/drivers/x1twin.cpp",
	MAME_DIR .. "src/mame/drivers/mz2500.cpp",
	MAME_DIR .. "src/mame/includes/mz2500.h",
	MAME_DIR .. "src/mame/drivers/mz3500.cpp",
	MAME_DIR .. "src/mame/drivers/pce220.cpp",
	MAME_DIR .. "src/mame/machine/pce220_ser.cpp",
	MAME_DIR .. "src/mame/machine/pce220_ser.h",
	MAME_DIR .. "src/mame/drivers/mz6500.cpp",
	MAME_DIR .. "src/mame/drivers/zaurus.cpp",
	MAME_DIR .. "src/mame/drivers/fontwriter.cpp",
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
	MAME_DIR .. "src/mame/includes/spec128.h",
	MAME_DIR .. "src/mame/drivers/timex.cpp",
	MAME_DIR .. "src/mame/includes/timex.h",
	MAME_DIR .. "src/mame/drivers/specpls3.cpp",
	MAME_DIR .. "src/mame/includes/specpls3.h",
	MAME_DIR .. "src/mame/drivers/scorpion.cpp",
	MAME_DIR .. "src/mame/drivers/atm.cpp",
	MAME_DIR .. "src/mame/drivers/pentagon.cpp",
	MAME_DIR .. "src/mame/machine/beta.cpp",
	MAME_DIR .. "src/mame/machine/beta.h",
	MAME_DIR .. "src/mame/machine/spec_snqk.cpp",
	MAME_DIR .. "src/mame/machine/spec_snqk.h",
	MAME_DIR .. "src/mame/drivers/ql.cpp",
	MAME_DIR .. "src/mame/machine/qimi.cpp",
	MAME_DIR .. "src/mame/machine/qimi.h",
	MAME_DIR .. "src/mame/video/zx8301.cpp",
	MAME_DIR .. "src/mame/video/zx8301.h",
	MAME_DIR .. "src/mame/machine/zx8302.cpp",
	MAME_DIR .. "src/mame/machine/zx8302.h",
	MAME_DIR .. "src/mame/drivers/tsconf.cpp",
	MAME_DIR .. "src/mame/includes/tsconf.h",
	MAME_DIR .. "src/mame/machine/tsconf.cpp",
}

createMESSProjects(_target, _subtarget, "siemens")
files {
	MAME_DIR .. "src/mame/drivers/bitel.cpp",
	MAME_DIR .. "src/mame/drivers/pcd.cpp",
	MAME_DIR .. "src/mame/drivers/pcmx2.cpp",
	MAME_DIR .. "src/mame/machine/pcd_kbd.cpp",
	MAME_DIR .. "src/mame/machine/pcd_kbd.h",
	MAME_DIR .. "src/mame/video/pcd.cpp",
	MAME_DIR .. "src/mame/video/pcd.h",
	MAME_DIR .. "src/mame/drivers/pg685.cpp",
}

createMESSProjects(_target, _subtarget, "slicer")
files {
	MAME_DIR .. "src/mame/drivers/slicer.cpp",
}

createMESSProjects(_target, _subtarget, "snk")
files {
	MAME_DIR .. "src/mame/drivers/neogeocd.cpp",
	MAME_DIR .. "src/mame/drivers/ngp.cpp",
	MAME_DIR .. "src/mame/video/k1ge.cpp",
	MAME_DIR .. "src/mame/video/k1ge.h",
}

createMESSProjects(_target, _subtarget, "sony")
files {
	MAME_DIR .. "src/mame/drivers/betacam.cpp",
	MAME_DIR .. "src/mame/drivers/bvm.cpp",
	MAME_DIR .. "src/mame/drivers/dfs500.cpp",
	MAME_DIR .. "src/mame/drivers/dpsv55.cpp",
	MAME_DIR .. "src/mame/drivers/pockstat.cpp",
	MAME_DIR .. "src/mame/drivers/psx.cpp",
	MAME_DIR .. "src/mame/machine/psxcd.cpp",
	MAME_DIR .. "src/mame/machine/psxcd.h",
	MAME_DIR .. "src/mame/drivers/pve500.cpp",
	MAME_DIR .. "src/mame/drivers/smc777.cpp",
	MAME_DIR .. "src/mame/drivers/ps2sony.cpp",
	MAME_DIR .. "src/mame/drivers/umatic.cpp",
}

createMESSProjects(_target, _subtarget, "sony_news")
files {
	MAME_DIR .. "src/mame/drivers/news_68k.cpp",
	MAME_DIR .. "src/mame/drivers/news_r3k.cpp",
	MAME_DIR .. "src/mame/drivers/news_38xx.cpp",
	MAME_DIR .. "src/mame/machine/dmac_0448.cpp",
	MAME_DIR .. "src/mame/machine/dmac_0448.h",
	MAME_DIR .. "src/mame/machine/dmac_0266.cpp",
	MAME_DIR .. "src/mame/machine/dmac_0266.h",
	MAME_DIR .. "src/mame/machine/news_hid.cpp",
	MAME_DIR .. "src/mame/machine/news_hid.h",
}

createMESSProjects(_target, _subtarget, "sord")
files {
	MAME_DIR .. "src/mame/drivers/m5.cpp",
}

createMESSProjects(_target, _subtarget, "special")
files {
	MAME_DIR .. "src/mame/drivers/special.cpp",
	MAME_DIR .. "src/mame/includes/special.h",
	MAME_DIR .. "src/mame/audio/special.cpp",
	MAME_DIR .. "src/mame/audio/special.h",
	MAME_DIR .. "src/mame/machine/special.cpp",
	MAME_DIR .. "src/mame/video/special.cpp",
}

createMESSProjects(_target, _subtarget, "stm")
files {
	MAME_DIR .. "src/mame/drivers/pp.cpp",
}

createMESSProjects(_target, _subtarget, "sun")
files {
	MAME_DIR .. "src/mame/drivers/sun1.cpp",
	MAME_DIR .. "src/mame/drivers/sun2.cpp",
	MAME_DIR .. "src/mame/drivers/sun3.cpp",
	MAME_DIR .. "src/mame/drivers/sun3x.cpp",
	MAME_DIR .. "src/mame/drivers/sun4.cpp",
}

createMESSProjects(_target, _subtarget, "svi")
files {
	MAME_DIR .. "src/mame/drivers/svi318.cpp",
}

createMESSProjects(_target, _subtarget, "svision")
files {
	MAME_DIR .. "src/mame/drivers/svision.cpp",
	MAME_DIR .. "src/mame/includes/svision.h",
	MAME_DIR .. "src/mame/audio/svis_snd.cpp",
	MAME_DIR .. "src/mame/audio/svis_snd.h",
}

createMESSProjects(_target, _subtarget, "swtpc")
files {
	MAME_DIR .. "src/mame/drivers/swtpc.cpp",
	MAME_DIR .. "src/mame/drivers/swtpc09.cpp",
	MAME_DIR .. "src/mame/includes/swtpc09.h",
	MAME_DIR .. "src/mame/machine/swtpc09.cpp",
	MAME_DIR .. "src/mame/drivers/swtpc8212.cpp",
}

createMESSProjects(_target, _subtarget, "synertek")
files {
	MAME_DIR .. "src/mame/drivers/ktm3.cpp",
	MAME_DIR .. "src/mame/drivers/mbc020.cpp",
	MAME_DIR .. "src/mame/drivers/sym1.cpp",
}

createMESSProjects(_target, _subtarget, "ta")
files {
	MAME_DIR .. "src/mame/drivers/alphatpx.cpp",
	MAME_DIR .. "src/mame/drivers/alphatpc16.cpp",
	MAME_DIR .. "src/mame/drivers/alphatro.cpp",
}

createMESSProjects(_target, _subtarget, "tab")
files {
	MAME_DIR .. "src/mame/drivers/tabe22.cpp",
	MAME_DIR .. "src/mame/machine/e22_kbd.cpp",
	MAME_DIR .. "src/mame/machine/e22_kbd.h",
}

createMESSProjects(_target, _subtarget, "tandberg")
files {
	MAME_DIR .. "src/mame/drivers/tdv2324.cpp",
	MAME_DIR .. "src/mame/includes/tdv2324.h",
}

createMESSProjects(_target, _subtarget, "tangerin")
files {
	MAME_DIR .. "src/mame/drivers/alphatan.cpp",
	MAME_DIR .. "src/mame/drivers/hhtiger.cpp",
	MAME_DIR .. "src/mame/drivers/microtan.cpp",
	MAME_DIR .. "src/mame/includes/microtan.h",
	MAME_DIR .. "src/mame/machine/microtan.cpp",
	MAME_DIR .. "src/mame/video/microtan.cpp",
	MAME_DIR .. "src/mame/drivers/oric.cpp",
}

createMESSProjects(_target, _subtarget, "tasc")
files {
	MAME_DIR .. "src/mame/drivers/tasc.cpp",
}

createMESSProjects(_target, _subtarget, "tatung")
files {
	MAME_DIR .. "src/mame/drivers/einstein.cpp",
}

createMESSProjects(_target, _subtarget, "teamconc")
files {
	MAME_DIR .. "src/mame/drivers/comquest.cpp",
}

createMESSProjects(_target, _subtarget, "tectoy")
files {
	MAME_DIR .. "src/mame/drivers/pensebem.cpp",
}

createMESSProjects(_target, _subtarget, "tektroni")
files {
	MAME_DIR .. "src/mame/drivers/tek405x.cpp",
	MAME_DIR .. "src/mame/includes/tek405x.h",
	MAME_DIR .. "src/mame/drivers/tek410x.cpp",
	MAME_DIR .. "src/mame/drivers/tek440x.cpp",
	MAME_DIR .. "src/mame/drivers/tekigw.cpp",
	MAME_DIR .. "src/mame/drivers/tekxp33x.cpp",
	MAME_DIR .. "src/mame/machine/tek410x_kbd.cpp",
	MAME_DIR .. "src/mame/machine/tek410x_kbd.h",
}

createMESSProjects(_target, _subtarget, "telenova")
files {
	MAME_DIR .. "src/mame/drivers/compis.cpp",
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
	MAME_DIR .. "src/mame/drivers/tv910.cpp",
	MAME_DIR .. "src/mame/drivers/tv912.cpp",
	MAME_DIR .. "src/mame/drivers/tv924.cpp",
	MAME_DIR .. "src/mame/drivers/tv950.cpp",
	MAME_DIR .. "src/mame/drivers/tv955.cpp",
	MAME_DIR .. "src/mame/drivers/tv965.cpp",
	MAME_DIR .. "src/mame/drivers/tv990.cpp",
	MAME_DIR .. "src/mame/drivers/ts3000.cpp",
	MAME_DIR .. "src/mame/machine/tv950kb.cpp",
	MAME_DIR .. "src/mame/machine/tv950kb.h",
	MAME_DIR .. "src/mame/machine/tv955kb.cpp",
	MAME_DIR .. "src/mame/machine/tv955kb.h",
}

createMESSProjects(_target, _subtarget, "tesla")
files {
	MAME_DIR .. "src/mame/drivers/ondra.cpp",
	MAME_DIR .. "src/mame/includes/ondra.h",
	MAME_DIR .. "src/mame/machine/ondra.cpp",
	MAME_DIR .. "src/mame/drivers/pmd85.cpp",
	MAME_DIR .. "src/mame/includes/pmd85.h",
	MAME_DIR .. "src/mame/machine/pmd85.cpp",
	MAME_DIR .. "src/mame/drivers/pmi80.cpp",
	MAME_DIR .. "src/mame/drivers/sapi1.cpp",
}

createMESSProjects(_target, _subtarget, "thomson")
files {
	MAME_DIR .. "src/mame/drivers/thomson.cpp",
	MAME_DIR .. "src/mame/includes/thomson.h",
	MAME_DIR .. "src/mame/machine/thomson.cpp",
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
	MAME_DIR .. "src/mame/drivers/hh_tms1k.cpp",
	MAME_DIR .. "src/mame/includes/hh_tms1k.h",
	MAME_DIR .. "src/mame/drivers/tispeak.cpp",  -- subdriver of hh_tms1k
	MAME_DIR .. "src/mame/drivers/tispellb.cpp", -- "
	MAME_DIR .. "src/mame/drivers/ti74.cpp",
	MAME_DIR .. "src/mame/drivers/ti85.cpp",
	MAME_DIR .. "src/mame/includes/ti85.h",
	MAME_DIR .. "src/mame/machine/ti85.cpp",
	MAME_DIR .. "src/mame/video/ti85.cpp",
	MAME_DIR .. "src/mame/drivers/ti89.cpp",
	MAME_DIR .. "src/mame/includes/ti89.h",
	MAME_DIR .. "src/mame/drivers/ti931.cpp",
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
}

createMESSProjects(_target, _subtarget, "tiger")
files {
	MAME_DIR .. "src/mame/drivers/gamecom.cpp",
	MAME_DIR .. "src/mame/includes/gamecom.h",
	MAME_DIR .. "src/mame/machine/gamecom.cpp",
	MAME_DIR .. "src/mame/video/gamecom.cpp",
	MAME_DIR .. "src/mame/drivers/k28.cpp",
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
	MAME_DIR .. "src/mame/drivers/tomy_princ.cpp",
}

createMESSProjects(_target, _subtarget, "toshiba")
files {
	MAME_DIR .. "src/mame/drivers/pasopia.cpp",
	MAME_DIR .. "src/mame/drivers/pasopia7.cpp",
	MAME_DIR .. "src/mame/drivers/paso1600.cpp",
}

createMESSProjects(_target, _subtarget, "trainer")
files {
	MAME_DIR .. "src/mame/drivers/amico2k.cpp",
	MAME_DIR .. "src/mame/drivers/babbage.cpp",
	MAME_DIR .. "src/mame/drivers/bob85.cpp",
	MAME_DIR .. "src/mame/drivers/crei680.cpp",
	MAME_DIR .. "src/mame/drivers/cvicny.cpp",
	MAME_DIR .. "src/mame/drivers/datum.cpp",
	MAME_DIR .. "src/mame/drivers/dolphunk.cpp",
	MAME_DIR .. "src/mame/drivers/emma2.cpp",
	MAME_DIR .. "src/mame/drivers/instruct.cpp",
	MAME_DIR .. "src/mame/drivers/mk14.cpp",
	MAME_DIR .. "src/mame/drivers/pro80.cpp",
	MAME_DIR .. "src/mame/drivers/savia84.cpp",
	MAME_DIR .. "src/mame/drivers/selz80.cpp",
	MAME_DIR .. "src/mame/drivers/tec1.cpp",
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
	MAME_DIR .. "src/mame/drivers/lnw80.cpp",
	MAME_DIR .. "src/mame/drivers/max80.cpp",
	MAME_DIR .. "src/mame/drivers/radionic.cpp",
	MAME_DIR .. "src/mame/drivers/trs80.cpp",
	MAME_DIR .. "src/mame/includes/trs80.h",
	MAME_DIR .. "src/mame/machine/trs80.cpp",
	MAME_DIR .. "src/mame/video/trs80.cpp",
	MAME_DIR .. "src/mame/drivers/trs80m2.cpp",
	MAME_DIR .. "src/mame/includes/trs80m2.h",
	MAME_DIR .. "src/mame/machine/trs80m2kb.cpp",
	MAME_DIR .. "src/mame/machine/trs80m2kb.h",
	MAME_DIR .. "src/mame/drivers/trs80m3.cpp",
	MAME_DIR .. "src/mame/includes/trs80m3.h",
	MAME_DIR .. "src/mame/machine/trs80m3.cpp",
	MAME_DIR .. "src/mame/video/trs80m3.cpp",
	MAME_DIR .. "src/mame/drivers/tandy2k.cpp",
	MAME_DIR .. "src/mame/includes/tandy2k.h",
	MAME_DIR .. "src/mame/machine/tandy2kb.cpp",
	MAME_DIR .. "src/mame/machine/tandy2kb.h",
	MAME_DIR .. "src/mame/drivers/vis.cpp",
}

createMESSProjects(_target, _subtarget, "tvgames")
files {
	MAME_DIR .. "src/mame/drivers/elan_ep3a19a.cpp",
	MAME_DIR .. "src/mame/drivers/elan_eu3a14.cpp",
	MAME_DIR .. "src/mame/drivers/elan_eu3a05.cpp",
	MAME_DIR .. "src/mame/audio/elan_eu3a05.cpp",
	MAME_DIR .. "src/mame/audio/elan_eu3a05.h",
	MAME_DIR .. "src/mame/machine/elan_eu3a05gpio.cpp",
	MAME_DIR .. "src/mame/machine/elan_eu3a05gpio.h",
	MAME_DIR .. "src/mame/machine/elan_eu3a05commonsys.cpp",
	MAME_DIR .. "src/mame/machine/elan_eu3a05commonsys.h",
	MAME_DIR .. "src/mame/machine/elan_ep3a19asys.cpp",
	MAME_DIR .. "src/mame/machine/elan_ep3a19asys.h",
	MAME_DIR .. "src/mame/machine/elan_eu3a05sys.cpp",
	MAME_DIR .. "src/mame/machine/elan_eu3a05sys.h",
	MAME_DIR .. "src/mame/machine/elan_eu3a14sys.cpp",
	MAME_DIR .. "src/mame/machine/elan_eu3a14sys.h",
	MAME_DIR .. "src/mame/video/elan_eu3a05commonvid.cpp",
	MAME_DIR .. "src/mame/video/elan_eu3a05commonvid.h",
	MAME_DIR .. "src/mame/video/elan_eu3a05vid.cpp",
	MAME_DIR .. "src/mame/video/elan_eu3a05vid.h",
	MAME_DIR .. "src/mame/video/elan_eu3a14vid.cpp",
	MAME_DIR .. "src/mame/video/elan_eu3a14vid.h",
	MAME_DIR .. "src/mame/drivers/trkfldch.cpp",
	MAME_DIR .. "src/mame/drivers/tvgame.cpp",
	MAME_DIR .. "src/mame/drivers/spg110.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_skannerztv.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_digimake.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_jakks.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_jakks_gkr.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_jakks_sharp.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_jakks_tvtouch.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_zone.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_senca.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_senario.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_senario_poker.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_mysprtch.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_vii.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_wiwi.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_ican.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_playvision.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_shredmjr.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_telestory.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_tvgogo.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_pdc.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_dreamlife.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_lexibook.cpp",
	MAME_DIR .. "src/mame/drivers/spg2xx_smarttv.cpp",
	MAME_DIR .. "src/mame/includes/spg2xx.h",
	MAME_DIR .. "src/mame/drivers/spg29x.cpp",
	MAME_DIR .. "src/mame/machine/hyperscan_card.cpp",
	MAME_DIR .. "src/mame/machine/hyperscan_card.h",
	MAME_DIR .. "src/mame/machine/hyperscan_ctrl.cpp",
	MAME_DIR .. "src/mame/machine/hyperscan_ctrl.h",
	MAME_DIR .. "src/mame/drivers/spg29x_lexibook_jg7425.cpp",
	MAME_DIR .. "src/mame/drivers/generalplus_gpl16250.cpp",
	MAME_DIR .. "src/mame/drivers/generalplus_gpl16250_rom.cpp",
	MAME_DIR .. "src/mame/drivers/generalplus_gpl16250_romram.cpp",
	MAME_DIR .. "src/mame/drivers/generalplus_gpl16250_nand.cpp",
	MAME_DIR .. "src/mame/drivers/generalplus_gpl16250_mobigo.cpp",
	MAME_DIR .. "src/mame/drivers/generalplus_gpl16250_spi.cpp",
	MAME_DIR .. "src/mame/drivers/generalplus_gpl16250_spi_direct.cpp",
	MAME_DIR .. "src/mame/includes/generalplus_gpl16250.h",
	MAME_DIR .. "src/mame/includes/generalplus_gpl16250_romram.h",
	MAME_DIR .. "src/mame/includes/generalplus_gpl16250_nand.h",
	MAME_DIR .. "src/mame/machine/generalplus_gpl16250.cpp",
	MAME_DIR .. "src/mame/machine/generalplus_gpl16250.h",
	MAME_DIR .. "src/mame/drivers/generalplus_gpl32612.cpp",
	MAME_DIR .. "src/mame/drivers/generalplus_gpl162xx_lcdtype.cpp",
	MAME_DIR .. "src/mame/drivers/generalplus_gpl_unknown.cpp",
	MAME_DIR .. "src/mame/drivers/xavix.cpp",
	MAME_DIR .. "src/mame/includes/xavix.h",
	MAME_DIR .. "src/mame/drivers/xavix_2000.cpp",
	MAME_DIR .. "src/mame/includes/xavix_2000.h",
	MAME_DIR .. "src/mame/drivers/xavix_2002.cpp",
	MAME_DIR .. "src/mame/includes/xavix_2002.h",
	MAME_DIR .. "src/mame/video/xavix.cpp",
	MAME_DIR .. "src/mame/machine/xavix.cpp",
	MAME_DIR .. "src/mame/audio/xavix.cpp",
	MAME_DIR .. "src/mame/machine/xavix_mtrk_wheel.cpp",
	MAME_DIR .. "src/mame/machine/xavix_mtrk_wheel.h",
	MAME_DIR .. "src/mame/machine/xavix_madfb_ball.cpp",
	MAME_DIR .. "src/mame/machine/xavix_madfb_ball.h",
	MAME_DIR .. "src/mame/machine/xavix_io.cpp",
	MAME_DIR .. "src/mame/machine/xavix_io.h",
	MAME_DIR .. "src/mame/machine/xavix_adc.cpp",
	MAME_DIR .. "src/mame/machine/xavix_adc.h",
	MAME_DIR .. "src/mame/machine/xavix_anport.h",
	MAME_DIR .. "src/mame/machine/xavix_anport.cpp",
	MAME_DIR .. "src/mame/machine/xavix_math.h",
	MAME_DIR .. "src/mame/machine/xavix_math.cpp",
	MAME_DIR .. "src/mame/machine/xavix2002_io.cpp",
	MAME_DIR .. "src/mame/machine/xavix2002_io.h",
	MAME_DIR .. "src/mame/drivers/xavix2.cpp",
	MAME_DIR .. "src/mame/drivers/titan_soc.cpp",
	MAME_DIR .. "src/mame/drivers/st2302u_bbl_rom.cpp",
	MAME_DIR .. "src/mame/drivers/st2302u_bbl_spi.cpp",
	MAME_DIR .. "src/mame/video/bl_handhelds_lcdc.cpp",
	MAME_DIR .. "src/mame/video/bl_handhelds_lcdc.h",
	MAME_DIR .. "src/mame/drivers/actions_atj2279b.cpp",
	MAME_DIR .. "src/mame/drivers/pubint_storyreader.cpp",
	MAME_DIR .. "src/mame/drivers/magiceyes_pollux_vr3520f.cpp",
	MAME_DIR .. "src/mame/drivers/monkey_king_3b.cpp",
}

createMESSProjects(_target, _subtarget, "ultimachine")
files {
	MAME_DIR .. "src/mame/drivers/rambo.cpp",
}

createMESSProjects(_target, _subtarget, "ultratec")
files {
	MAME_DIR .. "src/mame/drivers/minicom.cpp",
}

createMESSProjects(_target, _subtarget, "unicard")
files {
	MAME_DIR .. "src/mame/drivers/bbcbc.cpp",
}

createMESSProjects(_target, _subtarget, "unisonic")
files {
	MAME_DIR .. "src/mame/drivers/unichamp.cpp",
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
	MAME_DIR .. "src/mame/includes/patinhofeio.h",
}

createMESSProjects(_target, _subtarget, "veb")
files {
	MAME_DIR .. "src/mame/drivers/chessmst.cpp",
	MAME_DIR .. "src/mame/drivers/chessmstdm.cpp",
	MAME_DIR .. "src/mame/drivers/kc.cpp",
	MAME_DIR .. "src/mame/includes/kc.h",
	MAME_DIR .. "src/mame/machine/kc.cpp",
	MAME_DIR .. "src/mame/machine/kc_keyb.cpp",
	MAME_DIR .. "src/mame/machine/kc_keyb.h",
	MAME_DIR .. "src/mame/video/kc.cpp",
	MAME_DIR .. "src/mame/drivers/lc80.cpp",
	MAME_DIR .. "src/mame/drivers/mc8020.cpp",
	MAME_DIR .. "src/mame/drivers/mc8030.cpp",
	MAME_DIR .. "src/mame/drivers/poly880.cpp",
	MAME_DIR .. "src/mame/drivers/sc2.cpp",
}

createMESSProjects(_target, _subtarget, "verifone")
files {
	MAME_DIR .. "src/mame/drivers/tranz330.cpp",
	MAME_DIR .. "src/mame/includes/tranz330.h"
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
	MAME_DIR .. "src/mame/audio/tvc.cpp",
	MAME_DIR .. "src/mame/audio/tvc.h",
}

createMESSProjects(_target, _subtarget, "visual")
files {
	MAME_DIR .. "src/mame/drivers/v100.cpp",
	MAME_DIR .. "src/mame/drivers/v102.cpp",
	MAME_DIR .. "src/mame/machine/v102_kbd.cpp",
	MAME_DIR .. "src/mame/machine/v102_kbd.h",
	MAME_DIR .. "src/mame/drivers/v550.cpp",
	MAME_DIR .. "src/mame/drivers/v1050.cpp",
	MAME_DIR .. "src/mame/includes/v1050.h",
	MAME_DIR .. "src/mame/machine/v1050kb.cpp",
	MAME_DIR .. "src/mame/machine/v1050kb.h",
	MAME_DIR .. "src/mame/video/v1050.cpp",
}

createMESSProjects(_target, _subtarget, "votrax")
files {
	MAME_DIR .. "src/mame/drivers/votrhv.cpp",
	MAME_DIR .. "src/mame/drivers/votrpss.cpp",
	MAME_DIR .. "src/mame/drivers/votrtnt.cpp",
}

createMESSProjects(_target, _subtarget, "vtech")
files {
	MAME_DIR .. "src/mame/drivers/clickstart.cpp",
	MAME_DIR .. "src/mame/drivers/crvision.cpp",
	MAME_DIR .. "src/mame/includes/crvision.h",
	MAME_DIR .. "src/mame/drivers/gamemachine.cpp",
	MAME_DIR .. "src/mame/audio/nl_gamemachine.cpp",
	MAME_DIR .. "src/mame/audio/nl_gamemachine.h",
	MAME_DIR .. "src/mame/drivers/geniusiq.cpp",
	MAME_DIR .. "src/mame/drivers/geniusjr.cpp",
	MAME_DIR .. "src/mame/drivers/gkidabc.cpp",
	MAME_DIR .. "src/mame/drivers/glcx.cpp",
	MAME_DIR .. "src/mame/drivers/innotv_innotabmax.cpp",
	MAME_DIR .. "src/mame/drivers/inteladv.cpp",
	MAME_DIR .. "src/mame/drivers/iqunlim.cpp",
	MAME_DIR .. "src/mame/drivers/laser3k.cpp",
	MAME_DIR .. "src/mame/drivers/lcmate2.cpp",
	MAME_DIR .. "src/mame/drivers/pc2000.cpp",
	MAME_DIR .. "src/mame/drivers/pc4.cpp",
	MAME_DIR .. "src/mame/includes/pc4.h",
	MAME_DIR .. "src/mame/video/pc4.cpp",
	MAME_DIR .. "src/mame/drivers/prestige.cpp",
	MAME_DIR .. "src/mame/drivers/socrates.cpp",
	MAME_DIR .. "src/mame/audio/socrates.cpp",
	MAME_DIR .. "src/mame/audio/socrates.h",
	MAME_DIR .. "src/mame/drivers/storio.cpp",
	MAME_DIR .. "src/mame/drivers/vsmile.cpp",
	MAME_DIR .. "src/mame/includes/vsmile.h",
	MAME_DIR .. "src/mame/drivers/vsmileb.cpp",
	MAME_DIR .. "src/mame/drivers/vtech1.cpp",
	MAME_DIR .. "src/mame/drivers/vtech2.cpp",
	MAME_DIR .. "src/mame/includes/vtech2.h",
	MAME_DIR .. "src/mame/video/vtech2.cpp",
	MAME_DIR .. "src/mame/machine/vtech2.cpp",
	MAME_DIR .. "src/mame/drivers/vtech_eu3a12.cpp",
	MAME_DIR .. "src/mame/drivers/vtech_innotab.cpp",
}

createMESSProjects(_target, _subtarget, "wang")
files {
	MAME_DIR .. "src/mame/drivers/wangpc.cpp",
	MAME_DIR .. "src/devices/bus/wangpc/wangpc.h",
	MAME_DIR .. "src/mame/machine/wangpckb.cpp",
	MAME_DIR .. "src/mame/machine/wangpckb.h",
}

createMESSProjects(_target, _subtarget, "westinghouse")
files {
	MAME_DIR .. "src/mame/drivers/testconsole.cpp",
}

createMESSProjects(_target, _subtarget, "wavemate")
files {
	MAME_DIR .. "src/mame/drivers/bullet.cpp",
	MAME_DIR .. "src/mame/includes/bullet.h",
	MAME_DIR .. "src/mame/drivers/jupiter.cpp",
}

createMESSProjects(_target, _subtarget, "wyse")
files {
	MAME_DIR .. "src/mame/drivers/wy100.cpp",
	MAME_DIR .. "src/mame/drivers/wy150.cpp",
	MAME_DIR .. "src/mame/drivers/wy30p.cpp",
	MAME_DIR .. "src/mame/drivers/wy50.cpp",
	MAME_DIR .. "src/mame/drivers/wy55.cpp",
	MAME_DIR .. "src/mame/drivers/wy60.cpp",
	MAME_DIR .. "src/mame/drivers/wy85.cpp",
	MAME_DIR .. "src/mame/machine/wy50kb.cpp",
	MAME_DIR .. "src/mame/machine/wy50kb.h",
}

createMESSProjects(_target, _subtarget, "xerox")
files {
	MAME_DIR .. "src/mame/drivers/xerox820.cpp",
	MAME_DIR .. "src/mame/includes/xerox820.h",
	MAME_DIR .. "src/mame/machine/x820kb.cpp",
	MAME_DIR .. "src/mame/machine/x820kb.h",
	MAME_DIR .. "src/mame/drivers/bigbord2.cpp",
	MAME_DIR .. "src/mame/drivers/alto1.cpp",
	MAME_DIR .. "src/mame/drivers/alto2.cpp",
}

createMESSProjects(_target, _subtarget, "xussrpc")
files {
	MAME_DIR .. "src/mame/drivers/ec184x.cpp",
	MAME_DIR .. "src/mame/drivers/iskr103x.cpp",
	MAME_DIR .. "src/mame/drivers/mc1502.cpp",
	MAME_DIR .. "src/mame/machine/kb_7007_3.h",
	MAME_DIR .. "src/mame/drivers/poisk1.cpp",
	MAME_DIR .. "src/mame/machine/kb_poisk1.h",
}

createMESSProjects(_target, _subtarget, "yamaha")
files {
	MAME_DIR .. "src/mame/machine/mulcd.cpp",
	MAME_DIR .. "src/mame/drivers/yman1x.cpp",
	MAME_DIR .. "src/mame/drivers/ymdx100.cpp",
	MAME_DIR .. "src/mame/drivers/ymdx11.cpp",
	MAME_DIR .. "src/mame/drivers/ymmu5.cpp",
	MAME_DIR .. "src/mame/drivers/ymmu50.cpp",
	MAME_DIR .. "src/mame/drivers/ymmu80.cpp",
	MAME_DIR .. "src/mame/drivers/ymmu100.cpp",
	MAME_DIR .. "src/mame/drivers/ympsr16.cpp",
	MAME_DIR .. "src/mame/drivers/ympsr40.cpp",
	MAME_DIR .. "src/mame/drivers/ympsr60.cpp",
	MAME_DIR .. "src/mame/drivers/ympsr340.cpp",
	MAME_DIR .. "src/mame/drivers/ymrx15.cpp",
	MAME_DIR .. "src/mame/drivers/ymsy35.cpp",
	MAME_DIR .. "src/mame/drivers/ymtx81z.cpp",
	MAME_DIR .. "src/mame/drivers/ymvl70.cpp",
	MAME_DIR .. "src/mame/drivers/fb01.cpp",
	MAME_DIR .. "src/mame/drivers/tg100.cpp",
}

createMESSProjects(_target, _subtarget, "zenith")
files {
	MAME_DIR .. "src/mame/drivers/mdt60.cpp",
	MAME_DIR .. "src/mame/drivers/z100.cpp",
	MAME_DIR .. "src/mame/drivers/z22.cpp",
	MAME_DIR .. "src/mame/drivers/z29.cpp",
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
	MAME_DIR .. "src/mame/drivers/aaa.cpp",
	MAME_DIR .. "src/mame/drivers/acd.cpp",
	MAME_DIR .. "src/mame/drivers/aceex.cpp",
	MAME_DIR .. "src/mame/drivers/adacp150.cpp",
	MAME_DIR .. "src/mame/drivers/adds2020.cpp",
	MAME_DIR .. "src/mame/drivers/aid80f.cpp",
	MAME_DIR .. "src/mame/drivers/airbase99.cpp",
	MAME_DIR .. "src/mame/drivers/alcat7100.cpp",
	MAME_DIR .. "src/mame/drivers/alesis_qs.cpp",
	MAME_DIR .. "src/mame/drivers/alfaskop41xx.cpp",
	MAME_DIR .. "src/mame/drivers/alphasma.cpp",
	MAME_DIR .. "src/mame/drivers/alphasma3k.cpp",
	MAME_DIR .. "src/mame/drivers/am1000.cpp",
	MAME_DIR .. "src/mame/drivers/ampex.cpp",
	MAME_DIR .. "src/mame/drivers/ampex210.cpp",
	MAME_DIR .. "src/mame/machine/ampex210_kbd.cpp",
	MAME_DIR .. "src/mame/machine/ampex210_kbd.h",
	MAME_DIR .. "src/mame/drivers/ampscarp.cpp",
	MAME_DIR .. "src/mame/drivers/amust.cpp",
	MAME_DIR .. "src/mame/drivers/anzterm.cpp",
	MAME_DIR .. "src/mame/drivers/applix.cpp",
	MAME_DIR .. "src/mame/drivers/argox.cpp",
	MAME_DIR .. "src/mame/drivers/attache.cpp",
	MAME_DIR .. "src/mame/drivers/aussiebyte.cpp",
	MAME_DIR .. "src/mame/includes/aussiebyte.h",
	MAME_DIR .. "src/mame/video/aussiebyte.cpp",
	MAME_DIR .. "src/mame/drivers/ax20.cpp",
	MAME_DIR .. "src/mame/drivers/basf7100.cpp",
	MAME_DIR .. "src/mame/machine/basf7100_kbd.cpp",
	MAME_DIR .. "src/mame/machine/basf7100_kbd.h",
	MAME_DIR .. "src/mame/drivers/binbug.cpp",
	MAME_DIR .. "src/mame/drivers/bert.cpp",
	MAME_DIR .. "src/mame/drivers/bitgraph.cpp",
	MAME_DIR .. "src/mame/drivers/blit.cpp",
	MAME_DIR .. "src/mame/drivers/bpmmicro.cpp",
	MAME_DIR .. "src/mame/drivers/blw700i.cpp",
	MAME_DIR .. "src/mame/drivers/br8641.cpp",
	MAME_DIR .. "src/mame/drivers/busicom.cpp",
	MAME_DIR .. "src/mame/includes/busicom.h",
	MAME_DIR .. "src/mame/video/busicom.cpp",
	MAME_DIR .. "src/mame/drivers/c2color.cpp",
	MAME_DIR .. "src/mame/drivers/candela.cpp",
	MAME_DIR .. "src/mame/drivers/cardinal.cpp",
	MAME_DIR .. "src/mame/drivers/cbnt2039.cpp",
	MAME_DIR .. "src/mame/drivers/cdsys5.cpp",
	MAME_DIR .. "src/mame/drivers/chaos.cpp",
	MAME_DIR .. "src/mame/drivers/cd100.cpp",
	MAME_DIR .. "src/mame/drivers/cd2650.cpp",
	MAME_DIR .. "src/mame/drivers/cdc721.cpp",
	MAME_DIR .. "src/mame/drivers/cit1500.cpp",
	MAME_DIR .. "src/mame/drivers/clxvme186.cpp",
	MAME_DIR .. "src/mame/drivers/codata.cpp",
	MAME_DIR .. "src/mame/drivers/consola_emt.cpp",
	MAME_DIR .. "src/mame/drivers/controlid.cpp",
	MAME_DIR .. "src/mame/drivers/cortex.cpp",
	MAME_DIR .. "src/mame/drivers/cosmicos.cpp",
	MAME_DIR .. "src/mame/includes/cosmicos.h",
	MAME_DIR .. "src/mame/drivers/cp1.cpp",
	MAME_DIR .. "src/mame/drivers/cxhumax.cpp",
	MAME_DIR .. "src/mame/includes/cxhumax.h",
	MAME_DIR .. "src/mame/drivers/ckz80.cpp",
	MAME_DIR .. "src/mame/drivers/d400.cpp",
	MAME_DIR .. "src/mame/drivers/d6800.cpp",
	MAME_DIR .. "src/mame/drivers/d6809.cpp",
	MAME_DIR .. "src/mame/drivers/daruma.cpp",
	MAME_DIR .. "src/mame/drivers/datacast.cpp",
	MAME_DIR .. "src/mame/drivers/design.cpp",
	MAME_DIR .. "src/mame/drivers/dg680.cpp",
	MAME_DIR .. "src/mame/drivers/diablo1300.cpp",
	MAME_DIR .. "src/mame/drivers/didact.cpp",
	MAME_DIR .. "src/mame/drivers/digel804.cpp",
	MAME_DIR .. "src/mame/drivers/digilog320.cpp",
	MAME_DIR .. "src/mame/drivers/digilog400.cpp",
	MAME_DIR .. "src/mame/machine/digilog_kbd.cpp",
	MAME_DIR .. "src/mame/machine/digilog_kbd.h",
	MAME_DIR .. "src/mame/drivers/digijet.cpp",
	MAME_DIR .. "src/mame/drivers/dim68k.cpp",
	MAME_DIR .. "src/mame/drivers/dm7000.cpp",
	MAME_DIR .. "src/mame/includes/dm7000.h",
	MAME_DIR .. "src/mame/drivers/dmax8000.cpp",
	MAME_DIR .. "src/mame/drivers/dmv.cpp",
	MAME_DIR .. "src/mame/machine/dmv_keyb.cpp",
	MAME_DIR .. "src/mame/machine/dmv_keyb.h",
	MAME_DIR .. "src/mame/drivers/dps1.cpp",
	MAME_DIR .. "src/mame/drivers/dsb46.cpp",
	MAME_DIR .. "src/mame/drivers/dual68.cpp",
	MAME_DIR .. "src/mame/drivers/e100.cpp",
	MAME_DIR .. "src/mame/drivers/eacc.cpp",
	MAME_DIR .. "src/mame/drivers/easy_karaoke.cpp",
	MAME_DIR .. "src/mame/drivers/elwro800.cpp",
	MAME_DIR .. "src/mame/drivers/elzet80.cpp",
	MAME_DIR .. "src/mame/drivers/epic14e.cpp",
	MAME_DIR .. "src/mame/drivers/esprit.cpp",
	MAME_DIR .. "src/mame/drivers/eti660.cpp",
	MAME_DIR .. "src/mame/drivers/eurit.cpp",
	MAME_DIR .. "src/mame/drivers/eurocom2.cpp",
	MAME_DIR .. "src/mame/drivers/excali64.cpp",
	MAME_DIR .. "src/mame/drivers/facit4440.cpp",
	MAME_DIR .. "src/mame/drivers/fanucs15.cpp",
	MAME_DIR .. "src/mame/drivers/fanucspmg.cpp",
	MAME_DIR .. "src/mame/drivers/fc100.cpp",
	MAME_DIR .. "src/mame/drivers/fk1.cpp",
	MAME_DIR .. "src/mame/drivers/freedom120.cpp",
	MAME_DIR .. "src/mame/drivers/fs3216.cpp",
	MAME_DIR .. "src/mame/drivers/ft68m.cpp",
	MAME_DIR .. "src/mame/drivers/gameking.cpp",
	MAME_DIR .. "src/mame/drivers/gem_rp.cpp",
	MAME_DIR .. "src/mame/drivers/gigatron.cpp",
	MAME_DIR .. "src/mame/drivers/gimix.cpp",
	MAME_DIR .. "src/mame/drivers/gm1000.cpp",
	MAME_DIR .. "src/mame/drivers/gnat10.cpp",
	MAME_DIR .. "src/mame/drivers/goupil.cpp",
	MAME_DIR .. "src/mame/drivers/grfd2301.cpp",
	MAME_DIR .. "src/mame/drivers/hazeltin.cpp",
	MAME_DIR .. "src/mame/drivers/hazl1420.cpp",
	MAME_DIR .. "src/mame/drivers/hohnadam.cpp",
	MAME_DIR .. "src/mame/drivers/hp3478a.cpp",
	MAME_DIR .. "src/mame/drivers/hprot1.cpp",
	MAME_DIR .. "src/mame/drivers/hpz80unk.cpp",
	MAME_DIR .. "src/mame/drivers/ht68k.cpp",
	MAME_DIR .. "src/mame/drivers/i7000.cpp",
	MAME_DIR .. "src/mame/drivers/ibm3153.cpp",
	MAME_DIR .. "src/mame/drivers/icatel.cpp",
	MAME_DIR .. "src/mame/drivers/icebox.cpp",
	MAME_DIR .. "src/mame/drivers/iez80.cpp",
	MAME_DIR .. "src/mame/drivers/if800.cpp",
	MAME_DIR .. "src/mame/drivers/ikt5a.cpp",
	MAME_DIR .. "src/mame/drivers/imsai.cpp",
	MAME_DIR .. "src/mame/drivers/indiana.cpp",
	MAME_DIR .. "src/mame/drivers/is48x.cpp",
	MAME_DIR .. "src/mame/drivers/itc4.cpp",
	MAME_DIR .. "src/mame/drivers/itt1700.cpp",
	MAME_DIR .. "src/mame/machine/itt1700_kbd.cpp",
	MAME_DIR .. "src/mame/machine/itt1700_kbd.h",
	MAME_DIR .. "src/mame/drivers/itt3030.cpp",
	MAME_DIR .. "src/mame/drivers/itt9216.cpp",
	MAME_DIR .. "src/mame/drivers/jade.cpp",
	MAME_DIR .. "src/mame/drivers/jonos.cpp",
	MAME_DIR .. "src/mame/drivers/juku.cpp",
	MAME_DIR .. "src/mame/drivers/junior80.cpp",
	MAME_DIR .. "src/mame/drivers/kron.cpp",
	MAME_DIR .. "src/mame/drivers/lee1214.cpp",
	MAME_DIR .. "src/mame/drivers/lee1220.cpp",
	MAME_DIR .. "src/mame/drivers/learnwin.cpp",
	MAME_DIR .. "src/mame/drivers/lft.cpp",
	MAME_DIR .. "src/mame/drivers/lg-dvd.cpp",
	MAME_DIR .. "src/mame/drivers/lk3000.cpp",
	MAME_DIR .. "src/mame/drivers/lms46.cpp",
	MAME_DIR .. "src/mame/drivers/lola8a.cpp",
	MAME_DIR .. "src/mame/drivers/lilith.cpp",
	MAME_DIR .. "src/mame/drivers/mccpm.cpp",
	MAME_DIR .. "src/mame/drivers/mes.cpp",
	MAME_DIR .. "src/mame/drivers/mfabfz.cpp",
	MAME_DIR .. "src/mame/drivers/mice.cpp",
	MAME_DIR .. "src/mame/drivers/micral.cpp",
	MAME_DIR .. "src/mame/drivers/micro20.cpp",
	MAME_DIR .. "src/mame/drivers/micromon.cpp",
	MAME_DIR .. "src/mame/drivers/micronic.cpp",
	MAME_DIR .. "src/mame/includes/micronic.h",
	MAME_DIR .. "src/mame/drivers/mightyframe.cpp",
	MAME_DIR .. "src/mame/drivers/milwaukee.cpp",
	MAME_DIR .. "src/mame/drivers/mini2440.cpp",
	MAME_DIR .. "src/mame/drivers/miniframe.cpp",
	MAME_DIR .. "src/mame/drivers/minitel_2_rpic.cpp",
	MAME_DIR .. "src/mame/drivers/miuchiz.cpp",
	MAME_DIR .. "src/mame/drivers/ml20.cpp",
	MAME_DIR .. "src/mame/drivers/mmd1.cpp",
	MAME_DIR .. "src/mame/drivers/mmd2.cpp",
	MAME_DIR .. "src/mame/drivers/mod8.cpp",
	MAME_DIR .. "src/mame/drivers/modellot.cpp",
	MAME_DIR .. "src/mame/drivers/molecular.cpp",
	MAME_DIR .. "src/mame/drivers/monon_color.cpp",
	MAME_DIR .. "src/mame/drivers/ms0515.cpp",
	MAME_DIR .. "src/mame/drivers/ms9540.cpp",
	MAME_DIR .. "src/mame/drivers/mstation.cpp",
	MAME_DIR .. "src/mame/drivers/mt735.cpp",
	MAME_DIR .. "src/mame/drivers/mtd1256.cpp",
	MAME_DIR .. "src/mame/drivers/mx2178.cpp",
	MAME_DIR .. "src/mame/drivers/mycom.cpp",
	MAME_DIR .. "src/mame/drivers/myvision.cpp",
	MAME_DIR .. "src/mame/drivers/newton.cpp",
	MAME_DIR .. "src/mame/machine/nl_hazelvid.cpp",
	MAME_DIR .. "src/mame/machine/nl_hazelvid.h",
	MAME_DIR .. "src/mame/drivers/notetaker.cpp",
	MAME_DIR .. "src/mame/drivers/ngen.cpp",
	MAME_DIR .. "src/mame/machine/ngen_kb.cpp",
	MAME_DIR .. "src/mame/machine/ngen_kb.h",
	MAME_DIR .. "src/mame/drivers/onyx.cpp",
	MAME_DIR .. "src/mame/drivers/p8k.cpp",
	MAME_DIR .. "src/mame/drivers/pdt3100.cpp",
	MAME_DIR .. "src/mame/drivers/pegasus.cpp",
	MAME_DIR .. "src/mame/drivers/pencil2.cpp",
	MAME_DIR .. "src/mame/drivers/perq.cpp",
	MAME_DIR .. "src/mame/drivers/pes.cpp",
	MAME_DIR .. "src/mame/drivers/picno.cpp",
	MAME_DIR .. "src/mame/drivers/pipbug.cpp",
	MAME_DIR .. "src/mame/drivers/plan80.cpp",
	MAME_DIR .. "src/mame/drivers/pm68k.cpp",
	MAME_DIR .. "src/mame/drivers/pockchal.cpp",
	MAME_DIR .. "src/mame/drivers/powerstack.cpp",
	MAME_DIR .. "src/mame/drivers/proteus3.cpp",
	MAME_DIR .. "src/mame/drivers/pt68k4.cpp",
	MAME_DIR .. "src/mame/drivers/ptcsol.cpp",
	MAME_DIR .. "src/mame/drivers/pulsar.cpp",
	MAME_DIR .. "src/mame/drivers/pv9234.cpp",
	MAME_DIR .. "src/mame/drivers/pwp14.cpp",
	MAME_DIR .. "src/mame/drivers/qtsbc.cpp",
	MAME_DIR .. "src/mame/drivers/rd100.cpp",
	MAME_DIR .. "src/mame/drivers/rvoice.cpp",
	MAME_DIR .. "src/mame/drivers/sacstate.cpp",
	MAME_DIR .. "src/mame/drivers/sartorius.cpp",
	MAME_DIR .. "src/mame/drivers/sb8085.cpp",
	MAME_DIR .. "src/mame/drivers/sbrain.cpp",
	MAME_DIR .. "src/mame/drivers/seattlecmp.cpp",
	MAME_DIR .. "src/mame/drivers/sh4robot.cpp",
	MAME_DIR .. "src/mame/drivers/sansa_fuze.cpp",
	MAME_DIR .. "src/mame/drivers/scopus.cpp",
	MAME_DIR .. "src/mame/drivers/shine.cpp",
	MAME_DIR .. "src/mame/drivers/si5500.cpp",
	MAME_DIR .. "src/mame/drivers/sk101bl.cpp",
	MAME_DIR .. "src/mame/drivers/slsstars.cpp",
	MAME_DIR .. "src/mame/drivers/softbox.cpp",
	MAME_DIR .. "src/mame/includes/softbox.h",
	MAME_DIR .. "src/mame/drivers/squale.cpp",
	MAME_DIR .. "src/mame/drivers/solbourne.cpp",
	MAME_DIR .. "src/mame/drivers/swyft.cpp",
	MAME_DIR .. "src/mame/drivers/symbolics.cpp",
	MAME_DIR .. "src/mame/drivers/synthex.cpp",
	MAME_DIR .. "src/mame/drivers/sys2900.cpp",
	MAME_DIR .. "src/mame/drivers/sys9002.cpp",
	MAME_DIR .. "src/mame/drivers/systec.cpp",
	MAME_DIR .. "src/mame/drivers/systel1.cpp",
	MAME_DIR .. "src/mame/drivers/tavernie.cpp",
	MAME_DIR .. "src/mame/drivers/tecnbras.cpp",
	MAME_DIR .. "src/mame/drivers/teleray10.cpp",
	MAME_DIR .. "src/mame/drivers/telex1192.cpp",
	MAME_DIR .. "src/mame/drivers/telex274.cpp",
	MAME_DIR .. "src/mame/drivers/telex277d.cpp",
	MAME_DIR .. "src/mame/drivers/terak.cpp",
	MAME_DIR .. "src/mame/drivers/terco.cpp",
	MAME_DIR .. "src/mame/drivers/terminal.cpp",
	MAME_DIR .. "src/mame/drivers/textelcomp.cpp",
	MAME_DIR .. "src/mame/drivers/ti630.cpp",
	MAME_DIR .. "src/mame/drivers/tk635.cpp",
	MAME_DIR .. "src/mame/drivers/tr175.cpp",
	MAME_DIR .. "src/mame/drivers/trs80dt1.cpp",
	MAME_DIR .. "src/mame/drivers/tsispch.cpp",
	MAME_DIR .. "src/mame/drivers/tulip1.cpp",
	MAME_DIR .. "src/mame/drivers/unistar.cpp",
	MAME_DIR .. "src/mame/drivers/v6809.cpp",
	MAME_DIR .. "src/mame/drivers/vanguardmk1.cpp",
	MAME_DIR .. "src/mame/drivers/vd56sp.cpp",
	MAME_DIR .. "src/mame/drivers/vector4.cpp",
	MAME_DIR .. "src/mame/drivers/vectrix.cpp",
	MAME_DIR .. "src/mame/drivers/vp60.cpp",
	MAME_DIR .. "src/mame/includes/vp415.h",
	MAME_DIR .. "src/mame/drivers/vp415.cpp",
	MAME_DIR .. "src/mame/drivers/vsmilepro.cpp",
	MAME_DIR .. "src/mame/drivers/wicat.cpp",
	MAME_DIR .. "src/mame/drivers/xbase09.cpp",
	MAME_DIR .. "src/mame/drivers/xor100.cpp",
	MAME_DIR .. "src/mame/includes/xor100.h",
	MAME_DIR .. "src/mame/drivers/zms8085.cpp",
	MAME_DIR .. "src/mame/drivers/zorba.cpp",
	MAME_DIR .. "src/mame/includes/zorba.h",
	MAME_DIR .. "src/mame/machine/zorbakbd.cpp",
	MAME_DIR .. "src/mame/machine/zorbakbd.h",
	MAME_DIR .. "src/mame/drivers/zt8802.cpp",
	MAME_DIR .. "src/mame/drivers/testpat.cpp",
	MAME_DIR .. "src/mame/machine/nl_tp1983.cpp",
	MAME_DIR .. "src/mame/machine/nl_tp1985.cpp",
	MAME_DIR .. "src/mame/drivers/palestra.cpp",
	MAME_DIR .. "src/mame/machine/nl_palestra.cpp",
	MAME_DIR .. "src/mame/drivers/philipsbo.cpp",
	MAME_DIR .. "src/mame/drivers/mindset.cpp",
	MAME_DIR .. "src/mame/drivers/gs6502.cpp",
	MAME_DIR .. "src/mame/drivers/gs6809.cpp",
	MAME_DIR .. "src/mame/drivers/gscpm.cpp",
	MAME_DIR .. "src/mame/drivers/gsz80.cpp",
	MAME_DIR .. "src/mame/drivers/ultim809.cpp",
	MAME_DIR .. "src/mame/drivers/zeebo_qualcomm_adreno130.cpp",
}

end
