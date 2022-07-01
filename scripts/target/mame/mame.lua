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

CPUS["8X300"] = true
CPUS["ADSP21062"] = true
CPUS["ADSP21XX"] = true
CPUS["ALPHA"] = true
CPUS["ALTO2"] = true
CPUS["AM29000"] = true
CPUS["AMIS2000"] = true
CPUS["APEXC"] = true
CPUS["ARC"] = true
CPUS["ARCOMPACT"] = true
CPUS["ARM"] = true
CPUS["ARM7"] = true
CPUS["ASAP"] = true
CPUS["AVR8"] = true
CPUS["BCP"] = true
CPUS["CAPRICORN"] = true
CPUS["CCPU"] = true
CPUS["CLIPPER"] = true
CPUS["COP400"] = true
CPUS["COPS1"] = true
CPUS["COSMAC"] = true
CPUS["CP1610"] = true
CPUS["CR16B"] = true
CPUS["CUBEQCPU"] = true
CPUS["DIABLO"] = true
CPUS["DSP16"] = true
CPUS["DSP32C"] = true
CPUS["DSP56000"] = true
CPUS["DSP56156"] = true
CPUS["DSPP"] = true
CPUS["DSPV"] = true
CPUS["E0C6200"] = true
CPUS["E1"] = true
CPUS["ES5510"] = true
CPUS["ESRIP"] = true
CPUS["F2MC16"] = true
CPUS["F8"] = true
CPUS["FR"] = true
CPUS["G65816"] = true
CPUS["GTRON"] = true
CPUS["H16"] = true
CPUS["H6280"] = true
CPUS["H8"] = true
CPUS["H8500"] = true
CPUS["HCD62121"] = true
CPUS["HD61700"] = true
CPUS["HD6309"] = true
CPUS["HMCS40"] = true
CPUS["HPC"] = true
CPUS["HPHYBRID"] = true
CPUS["I386"] = true
CPUS["I8008"] = true
CPUS["I8085"] = true
CPUS["I8089"] = true
CPUS["I86"] = true
CPUS["I860"] = true
CPUS["I960"] = true
CPUS["IE15"] = true
CPUS["JAGUAR"] = true
CPUS["KC80"] = true
CPUS["KONAMI"] = true
CPUS["KS0164"] = true
CPUS["LC8670"] = true
CPUS["LH5801"] = true
CPUS["LR35902"] = true
CPUS["M37710"] = true
CPUS["M6502"] = true
CPUS["M6800"] = true
CPUS["M6805"] = true
CPUS["M6809"] = true
CPUS["M680X0"] = true
CPUS["M68HC16"] = true
CPUS["M88000"] = true
CPUS["MB86233"] = true
CPUS["MB86235"] = true
CPUS["MB88XX"] = true
CPUS["MC68HC11"] = true
CPUS["MCS40"] = true
CPUS["MCS48"] = true
CPUS["MCS51"] = true
CPUS["MCS96"] = true
CPUS["MEG"] = true
CPUS["MELPS4"] = true
CPUS["MINX"] = true
CPUS["MIPS1"] = true
CPUS["MIPS3"] = true
CPUS["MK1"] = true
CPUS["MN10200"] = true
CPUS["MN1880"] = true
CPUS["NANOPROCESSOR"] = true
CPUS["NEC"] = true
CPUS["NS32000"] = true
CPUS["PACE"] = true
CPUS["PATINHOFEIO"] = true
CPUS["PDP1"] = true
CPUS["PDP8"] = true
CPUS["PIC16C5X"] = true
CPUS["PIC16C62X"] = true
CPUS["PIC17"] = true
CPUS["POWERPC"] = true
CPUS["PPS4"] = true
CPUS["PPS41"] = true
CPUS["PSX"] = true
CPUS["RII"] = true
CPUS["ROMP"] = true
CPUS["RSP"] = true
CPUS["RW5000"] = true
CPUS["RX01"] = true
CPUS["S2650"] = true
CPUS["SATURN"] = true
CPUS["SC61860"] = true
CPUS["SCMP"] = true
CPUS["SCORE"] = true
CPUS["SCUDSP"] = true
CPUS["SE3208"] = true
CPUS["SH"] = true
CPUS["SM510"] = true
CPUS["SM8500"] = true
CPUS["SPARC"] = true
CPUS["SPC700"] = true
CPUS["SSEM"] = true
CPUS["SSP1601"] = true
CPUS["ST2XXX"] = true
CPUS["ST62XX"] = true
CPUS["SUPERFX"] = true
CPUS["T11"] = true
CPUS["TLCS870"] = true
CPUS["TLCS90"] = true
CPUS["TLCS900"] = true
CPUS["TMS1000"] = true
CPUS["TMS32010"] = true
CPUS["TMS32025"] = true
CPUS["TMS32031"] = true
CPUS["TMS32051"] = true
CPUS["TMS32082"] = true
CPUS["TMS340X0"] = true
CPUS["TMS57002"] = true
CPUS["TMS7000"] = true
CPUS["TMS9900"] = true
CPUS["TMS9900L"] = true
CPUS["TMS9995"] = true
CPUS["TX0"] = true
CPUS["UCOM4"] = true
CPUS["UNSP"] = true
CPUS["UPD7725"] = true
CPUS["UPD7810"] = true
CPUS["UPD78K"] = true
CPUS["V30MZ"] = true
CPUS["V60"] = true
CPUS["V810"] = true
CPUS["VT50"] = true
CPUS["VT61"] = true
--CPUS["W65816"] = true
CPUS["WE32000"] = true
CPUS["XAVIX"] = true
CPUS["XAVIX2"] = true
CPUS["XAVIX2000"] = true
CPUS["Z180"] = true
CPUS["Z8"] = true
CPUS["Z80"] = true
CPUS["Z8000"] = true
CPUS["Z8001"] = true

--------------------------------------------------
-- specify available sound cores
--------------------------------------------------

SOUNDS["AC97"] = true
SOUNDS["AD1848"] = true
SOUNDS["ADPCM"] = true
SOUNDS["AICA"] = true
SOUNDS["ASC"] = true
SOUNDS["ASTROCADE"] = true
SOUNDS["AWACS"] = true
SOUNDS["AY8910"] = true
SOUNDS["BEEP"] = true
SOUNDS["BSMT2000"] = true
SOUNDS["C140"] = true
SOUNDS["C352"] = true
SOUNDS["C6280"] = true
SOUNDS["CD2801"] = true
SOUNDS["CD2802"] = true
SOUNDS["CDDA"] = true
SOUNDS["CDP1863"] = true
SOUNDS["CDP1864"] = true
SOUNDS["CDP1869"] = true
SOUNDS["CEM3394"] = true
SOUNDS["DAC"] = true
SOUNDS["DAC76"] = true
SOUNDS["DAVE"] = true
SOUNDS["DIGITALKER"] = true
SOUNDS["DISCRETE"] = true
SOUNDS["DMADAC"] = true
SOUNDS["ES1373"] = true
SOUNDS["ES5503"] = true
SOUNDS["ES5505"] = true
SOUNDS["ES5506"] = true
SOUNDS["ES8712"] = true
SOUNDS["ESQPUMP"] = true
SOUNDS["GAELCO_CG1V"] = true
SOUNDS["GAELCO_GAE1"] = true
SOUNDS["GB_SOUND"] = true
SOUNDS["HC55516"] = true
SOUNDS["HUC6230"] = true
SOUNDS["I5000_SND"] = true
SOUNDS["ICS2115"] = true
SOUNDS["IOPSPU"] = true
SOUNDS["IREMGA20"] = true
SOUNDS["K005289"] = true
SOUNDS["K007232"] = true
SOUNDS["K051649"] = true
SOUNDS["K053260"] = true
SOUNDS["K054539"] = true
SOUNDS["K056800"] = true
SOUNDS["KS0164"] = true
SOUNDS["L7A1045"] = true
SOUNDS["LC7535"] = true
SOUNDS["LMC1992"] = true
SOUNDS["LYNX"] = true
SOUNDS["M58817"] = true
SOUNDS["MAS3507D"] = true
SOUNDS["MEA8000"] = true
SOUNDS["MM5837"] = true
SOUNDS["MOS656X"] = true
SOUNDS["MOS7360"] = true
SOUNDS["MPEG_AUDIO"] = true
SOUNDS["MSM5205"] = true
SOUNDS["MSM5232"] = true
SOUNDS["MULTIPCM"] = true
SOUNDS["NAMCO"] = true
SOUNDS["NAMCO_15XX"] = true
SOUNDS["NAMCO_163"] = true
SOUNDS["NAMCO_52XX"] = true
SOUNDS["NAMCO_63701X"] = true
SOUNDS["NAMCO_CUS30"] = true
SOUNDS["NES_APU"] = true
SOUNDS["OKIM6258"] = true
SOUNDS["OKIM6295"] = true
SOUNDS["OKIM6376"] = true
SOUNDS["OKIM9810"] = true
SOUNDS["PAULA_8364"] = true
SOUNDS["PCD3311"] = true
SOUNDS["POKEY"] = true
SOUNDS["QS1000"] = true
SOUNDS["QSOUND"] = true
SOUNDS["RF5C400"] = true
SOUNDS["RF5C68"] = true
SOUNDS["ROLANDPCM"] = true
SOUNDS["RP2C33_SOUND"] = true
SOUNDS["S14001A"] = true
SOUNDS["SAA1099"] = true
SOUNDS["SAMPLES"] = true
SOUNDS["SB0400"] = true
SOUNDS["SCSP"] = true
SOUNDS["SEGAPCM"] = true
SOUNDS["SETAPCM"] = true
SOUNDS["SID6581"] = true
SOUNDS["SID8580"] = true
SOUNDS["SN76477"] = true
SOUNDS["SN76496"] = true
SOUNDS["SNKWAVE"] = true
SOUNDS["SOCRATES"] = true
SOUNDS["SP0250"] = true
SOUNDS["SP0256"] = true
SOUNDS["SPEAKER"] = true
SOUNDS["SPU"] = true
SOUNDS["ST0016"] = true
SOUNDS["SWP00"] = true
SOUNDS["SWP20"] = true
SOUNDS["SWP30"] = true
SOUNDS["S_DSP"] = true
SOUNDS["T6721A"] = true
SOUNDS["T6W28"] = true
SOUNDS["TA7630"] = true
SOUNDS["TC8830F"] = true
SOUNDS["TIA"] = true
SOUNDS["TMC0281"] = true
SOUNDS["TMC0285"] = true
SOUNDS["TMS3615"] = true
SOUNDS["TMS36XX"] = true
SOUNDS["TMS5100"] = true
SOUNDS["TMS5110"] = true
SOUNDS["TMS5110A"] = true
SOUNDS["TMS5200"] = true
SOUNDS["TMS5220"] = true
SOUNDS["TT5665"] = true
SOUNDS["UDA1344"] = true
SOUNDS["UPD1771"] = true
SOUNDS["UPD7752"] = true
SOUNDS["UPD7759"] = true
SOUNDS["UPD934G"] = true
SOUNDS["VGMVIZ"] = true
SOUNDS["VLM5030"] = true
SOUNDS["VOTRAX"] = true
SOUNDS["VRC6"] = true
SOUNDS["VRENDER0"] = true
SOUNDS["WAVE"] = true
SOUNDS["X1_010"] = true
SOUNDS["XT446"] = true
SOUNDS["Y8950"] = true
SOUNDS["YM2151"] = true
SOUNDS["YM2154"] = true
SOUNDS["YM2203"] = true
SOUNDS["YM2413"] = true
SOUNDS["YM2414"] = true
SOUNDS["YM2608"] = true
SOUNDS["YM2610"] = true
SOUNDS["YM2610B"] = true
SOUNDS["YM2612"] = true
SOUNDS["YM3438"] = true
SOUNDS["YM3526"] = true
SOUNDS["YM3806"] = true
SOUNDS["YM3812"] = true
SOUNDS["YMF262"] = true
SOUNDS["YMF271"] = true
SOUNDS["YMF278B"] = true
SOUNDS["YMZ280B"] = true
SOUNDS["YMZ770"] = true
SOUNDS["ZSG2"] = true

--------------------------------------------------
-- specify available video cores
--------------------------------------------------

VIDEOS["AM8052"] = true
VIDEOS["BT431"] = true
VIDEOS["BT459"] = true
VIDEOS["BT45X"] = true
VIDEOS["BT47X"] = true
VIDEOS["BUFSPRITE"] = true
VIDEOS["CATSEYE"] = true
VIDEOS["CDP1861"] = true
VIDEOS["CDP1862"] = true
VIDEOS["CESBLIT"] = true
VIDEOS["CRT9007"] = true
VIDEOS["CRT9021"] = true
VIDEOS["CRT9028"] = true
VIDEOS["CRT9212"] = true
VIDEOS["CRTC_EGA"] = true
VIDEOS["DECSFB"] = true
VIDEOS["DL1416"] = true
VIDEOS["DM9368"] = true
VIDEOS["DP8350"] = true
VIDEOS["DP8510"] = true
VIDEOS["EF9340_1"] = true
VIDEOS["EF9345"] = true
VIDEOS["EF9364"] = true
VIDEOS["EF9365"] = true
VIDEOS["EF9369"] = true
VIDEOS["EPIC12"] = true
VIDEOS["FIXFREQ"] = true
VIDEOS["GBA_LCD"] = true
VIDEOS["GB_LCD"] = true
VIDEOS["GF4500"] = true
VIDEOS["GF7600GS"] = true
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
VIDEOS["I4100"] = true
VIDEOS["I8244"] = true
VIDEOS["I82730"] = true
VIDEOS["I8275"] = true
VIDEOS["IMS_CVC"] = true
VIDEOS["JANGOU_BLITTER"] = true
VIDEOS["K051316"] = true
VIDEOS["K053936"] = true
VIDEOS["LC7582"] = true
VIDEOS["LC7985"] = true
VIDEOS["M50458"] = true
VIDEOS["MB88303"] = true
VIDEOS["MB90082"] = true
VIDEOS["MB_VCU"] = true
VIDEOS["MC6845"] = true
VIDEOS["MC6847"] = true
VIDEOS["MD4330B"] = true
VIDEOS["MGA2064W"] = true
VIDEOS["MM5445"] = true
VIDEOS["MOS6566"] = true
VIDEOS["MSM6222B"] = true
VIDEOS["MSM6255"] = true
VIDEOS["NEREID"] = true
VIDEOS["NT7534"] = true
VIDEOS["PCD8544"] = true
VIDEOS["PCF2100"] = true
VIDEOS["PC_VGA"] = true
VIDEOS["POLY"] = true
VIDEOS["PPU2C0X"] = true
VIDEOS["PS2GIF"] = true
VIDEOS["PS2GS"] = true
VIDEOS["PSX"] = true
VIDEOS["PWM_DISPLAY"] = true
VIDEOS["RAMDAC"] = true
VIDEOS["S2636"] = true
VIDEOS["SAA5050"] = true
VIDEOS["SAA5240"] = true
VIDEOS["SCN2674"] = true
VIDEOS["SDA5708"] = true
VIDEOS["SED1200"] = true
VIDEOS["SED1330"] = true
VIDEOS["SED1356"] = true
VIDEOS["SED1500"] = true
VIDEOS["SED1520"] = true
VIDEOS["SEGA315_5124"] = true
VIDEOS["SEGA315_5313"] = true
VIDEOS["SNES_PPU"] = true
VIDEOS["T6963C"] = true
VIDEOS["T6A04"] = true
VIDEOS["TEA1002"] = true
VIDEOS["TLC34076"] = true
VIDEOS["TMAP038"] = true
VIDEOS["TMS34061"] = true
VIDEOS["TMS3556"] = true
VIDEOS["TMS9927"] = true
VIDEOS["TMS9928A"] = true
VIDEOS["TOPCAT"] = true
VIDEOS["UPD3301"] = true
VIDEOS["UPD7220"] = true
VIDEOS["UPD7227"] = true
VIDEOS["V9938"] = true
VIDEOS["VIC4567"] = true
VIDEOS["VIRGE_PCI"] = true
VIDEOS["VOODOO"] = true
VIDEOS["VOODOO_PCI"] = true
VIDEOS["VRENDER0"] = true
VIDEOS["ZEUS2"] = true

--------------------------------------------------
-- specify available machine cores
--------------------------------------------------

MACHINES["1MA6"] = true
MACHINES["1MB5"] = true
MACHINES["2812FIFO"] = true
MACHINES["28FXXX"] = true
MACHINES["6522VIA"] = true
MACHINES["6821PIA"] = true
MACHINES["6840PTM"] = true
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
MACHINES["ADC0844"] = true
MACHINES["ADC1038"] = true
MACHINES["ADC1213X"] = true
MACHINES["AIC565"] = true
MACHINES["AIC580"] = true
MACHINES["AIC6250"] = true
MACHINES["AICARTC"] = true
MACHINES["AKIKO"] = true
MACHINES["ALPHA_8921"] = true
MACHINES["AM25S55X"] = true
MACHINES["AM2847"] = true
MACHINES["AM2901B"] = true
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
MACHINES["APPLE_DRIVE"] = true
MACHINES["APPLE_FDC"] = true
MACHINES["APPLE_FDINTF"] = true
MACHINES["ARCHIMEDES_KEYB"] = true
MACHINES["ARM_AIC"] = true
MACHINES["ARM_IOMD"] = true
MACHINES["AT28C16"] = true
MACHINES["AT28C64B"] = true
MACHINES["AT29X"] = true
MACHINES["AT45DBXX"] = true
MACHINES["ATAFLASH"] = true
MACHINES["AT_KEYBC"] = true
MACHINES["AT_MB"] = true
MACHINES["AUTOCONFIG"] = true
MACHINES["AY31015"] = true
MACHINES["BACTA_DATALOGGER"] = true
MACHINES["BANKDEV"] = true
MACHINES["BIM68153"] = true
MACHINES["BITMAP_PRINTER"] = true
MACHINES["BL_HANDHELDS_MENUCONTROL"] = true
MACHINES["BQ4847"] = true
MACHINES["BQ4852"] = true
MACHINES["BUSMOUSE"] = true
MACHINES["CAMMU"] = true
MACHINES["CDP1852"] = true
MACHINES["CDP1871"] = true
MACHINES["CDP1879"] = true
MACHINES["CDU76S"] = true
MACHINES["CH376"] = true
MACHINES["CHESSMACHINE"] = true
MACHINES["CMOS40105"] = true
MACHINES["COM52C50"] = true
MACHINES["COM8116"] = true
MACHINES["COP452"] = true
MACHINES["CORVUSHD"] = true
MACHINES["CR511B"] = true
MACHINES["CR589"] = true
MACHINES["CS4031"] = true
MACHINES["CS8221"] = true
MACHINES["CS8900A"] = true
MACHINES["CXD1095"] = true
MACHINES["CXD1185"] = true
MACHINES["DC7085"] = true
MACHINES["DIABLO_HD"] = true
MACHINES["DL11"] = true
MACHINES["DMAC"] = true
MACHINES["DP8390"] = true
MACHINES["DP83932C"] = true
MACHINES["DP8573"] = true
MACHINES["DS1204"] = true
MACHINES["DS1205"] = true
MACHINES["DS1302"] = true
MACHINES["DS1315"] = true
MACHINES["DS1386"] = true
MACHINES["DS17X85"] = true
MACHINES["DS1994"] = true
MACHINES["DS2401"] = true
MACHINES["DS2404"] = true
MACHINES["DS6417"] = true
MACHINES["DS75160A"] = true
MACHINES["DS75161A"] = true
MACHINES["DS8874"] = true
MACHINES["E0516"] = true
MACHINES["E05A03"] = true
MACHINES["E05A30"] = true
MACHINES["EDLC"] = true
MACHINES["EEPROMDEV"] = true
MACHINES["ER1400"] = true
MACHINES["ER2055"] = true
MACHINES["EXORTERM"] = true
MACHINES["F3853"] = true
MACHINES["F4702"] = true
MACHINES["FDC37C665GT"] = true
MACHINES["FDC37C93X"] = true
MACHINES["FDC_PLL"] = true
MACHINES["FGA002"] = true
MACHINES["FM_SCSI"] = true
MACHINES["GAYLE"] = true
MACHINES["GENPC"] = true
MACHINES["GEN_FIFO"] = true
MACHINES["GEN_LATCH"] = true
MACHINES["GLUKRS"] = true
MACHINES["GT913"] = true
MACHINES["HD63450"] = true
MACHINES["HD64610"] = true
MACHINES["HDC9234"] = true
MACHINES["HP_DC100_TAPE"] = true
MACHINES["HP_TACO"] = true
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
MACHINES["I82357"] = true
MACHINES["I8243"] = true
MACHINES["I8251"] = true
MACHINES["I8255"] = true
MACHINES["I8257"] = true
MACHINES["I82586"] = true
MACHINES["I8271"] = true
MACHINES["I8279"] = true
MACHINES["I8291A"] = true
MACHINES["I8355"] = true
MACHINES["IBM21S850"] = true
MACHINES["ICM7170"] = true
MACHINES["IDECTRL"] = true
MACHINES["IE15"] = true
MACHINES["IM6402"] = true
MACHINES["INPUT_MERGER"] = true
MACHINES["INS8154"] = true
MACHINES["INS8250"] = true
MACHINES["INTELFLASH"] = true
MACHINES["IOPCDVD"] = true
MACHINES["IOPDMA"] = true
MACHINES["IOPINTC"] = true
MACHINES["IOPSIO2"] = true
MACHINES["IOPTIMER"] = true
MACHINES["IWM"] = true
MACHINES["JVS"] = true
MACHINES["K033906"] = true
MACHINES["K053252"] = true
MACHINES["K054321"] = true
MACHINES["K056230"] = true
MACHINES["KB3600"] = true
MACHINES["KBDC8042"] = true
MACHINES["KR1601RR1"] = true
MACHINES["KR2376"] = true
MACHINES["LATCH8"] = true
MACHINES["LC89510"] = true
MACHINES["LDP1000"] = true
MACHINES["LDP1450"] = true
MACHINES["LDPR8210"] = true
MACHINES["LDSTUB"] = true
MACHINES["LDV1000"] = true
MACHINES["LDV4200HLE"] = true
MACHINES["LDVP931"] = true
MACHINES["LH5810"] = true
MACHINES["LINFLASH"] = true
MACHINES["LOCOMO"] = true
MACHINES["LPCI"] = true
MACHINES["LSI53C810"] = true
MACHINES["M3002"] = true
MACHINES["M68307"] = true
MACHINES["M68340"] = true
MACHINES["M68SFDC"] = true
MACHINES["M6M80011AP"] = true
MACHINES["M950X0"] = true
MACHINES["MAC_VIDEO_SONORA"] = true
MACHINES["MB14241"] = true
MACHINES["MB3773"] = true
MACHINES["MB8421"] = true
MACHINES["MB87030"] = true
MACHINES["MB87078"] = true
MACHINES["MB8795"] = true
MACHINES["MB89352"] = true
MACHINES["MB89371"] = true
MACHINES["MB89374"] = true
MACHINES["MC14411"] = true
MACHINES["MC146818"] = true
MACHINES["MC68328"] = true
MACHINES["MC6843"] = true
MACHINES["MC6844"] = true
MACHINES["MC6846"] = true
MACHINES["MC6852"] = true
MACHINES["MC6854"] = true
MACHINES["MC68901"] = true
MACHINES["MCCS1850"] = true
MACHINES["MCF5206E"] = true
MACHINES["METERS"] = true
MACHINES["MICROTOUCH"] = true
MACHINES["MIOT6530"] = true
MACHINES["MM5307"] = true
MACHINES["MM5740"] = true
MACHINES["MM58167"] = true
MACHINES["MM58174"] = true
MACHINES["MM58274C"] = true
MACHINES["MM74C922"] = true
MACHINES["MOS6526"] = true
MACHINES["MOS6529"] = true
MACHINES["MOS6551"] = true
MACHINES["MOS6702"] = true
MACHINES["MOS8706"] = true
MACHINES["MOS8722"] = true
MACHINES["MOS8726"] = true
MACHINES["MPCC68561"] = true
MACHINES["MPU401"] = true
MACHINES["MSM5832"] = true
MACHINES["MSM58321"] = true
MACHINES["MSM6242"] = true
MACHINES["MSM6253"] = true
MACHINES["MYB3K_KEYBOARD"] = true
MACHINES["NCR5380"] = true
MACHINES["NCR5385"] = true
MACHINES["NCR5390"] = true
MACHINES["NCR539x"] = true
MACHINES["NCR53C7XX"] = true
MACHINES["NETLIST"] = true
MACHINES["NMC9306"] = true
MACHINES["NMK112"] = true
MACHINES["NS32081"] = true
MACHINES["NS32082"] = true
MACHINES["NS32202"] = true
MACHINES["NS32382"] = true
MACHINES["NSC810"] = true
MACHINES["NSCSI"] = true
MACHINES["OMTI5100"] = true
MACHINES["OUTPUT_LATCH"] = true
MACHINES["PCCARD"] = true
MACHINES["PCF8573"] = true
MACHINES["PCF8583"] = true
MACHINES["PCF8584"] = true
MACHINES["PCF8593"] = true
MACHINES["PCI"] = true
MACHINES["PCI9050"] = true
MACHINES["PCKEYBRD"] = true
MACHINES["PC_FDC"] = true
MACHINES["PC_LPT"] = true
MACHINES["PDC"] = true
MACHINES["PHI"] = true
MACHINES["PIC8259"] = true
MACHINES["PIT68230"] = true
MACHINES["PIT8253"] = true
MACHINES["PLA"] = true
--MACHINES["PROFILE"] = true
MACHINES["PROM82S129"] = true
MACHINES["PS2DMAC"] = true
MACHINES["PS2INTC"] = true
MACHINES["PS2MC"] = true
MACHINES["PS2PAD"] = true
MACHINES["PS2SIF"] = true
MACHINES["PS2TIMER"] = true
MACHINES["PXA255"] = true
MACHINES["R10696"] = true
MACHINES["R10788"] = true
MACHINES["R64H156"] = true
MACHINES["RA17XX"] = true
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
MACHINES["S2636"] = true
MACHINES["S3520CF"] = true
MACHINES["S3C24XX"] = true
MACHINES["S3C44B0"] = true
MACHINES["SA1110"] = true
MACHINES["SA1111"] = true
MACHINES["SAA1043"] = true
MACHINES["SAA7191"] = true
MACHINES["SATURN"] = true
MACHINES["SCC2698B"] = true
MACHINES["SCC68070"] = true
MACHINES["SCNXX562"] = true
MACHINES["SCN_PCI"] = true
MACHINES["SCOOP"] = true
MACHINES["SCSI"] = true
MACHINES["SCUDSP"] = true
MACHINES["SDA2006"] = true
MACHINES["SECFLASH"] = true
MACHINES["SEGA_SCU"] = true
MACHINES["SEIBU_COP"] = true
MACHINES["SENSORBOARD"] = true
MACHINES["SERFLASH"] = true
MACHINES["SMARTMEDIA"] = true
MACHINES["SMC91C9X"] = true
MACHINES["SMC92X4"] = true
MACHINES["SMIOC"] = true
MACHINES["SMPC"] = true
MACHINES["SONY_DRIVE"] = true
MACHINES["SPG290"] = true
MACHINES["SPG2XX"] = true
MACHINES["SPISDCARD"] = true
MACHINES["STEPPERS"] = true
MACHINES["STRATA"] = true
MACHINES["STVCD"] = true
MACHINES["SUN4C_MMU"] = true
MACHINES["SWIM1"] = true
MACHINES["SWIM2"] = true
MACHINES["SWIM3"] = true
MACHINES["SWTPC8212"] = true
MACHINES["S_SMP"] = true
MACHINES["TASC_SB30"] = true
MACHINES["TC0091LVC"] = true
MACHINES["TDC1008"] = true
MACHINES["TE7750"] = true
MACHINES["TI99_HD"] = true
MACHINES["TICKET"] = true
MACHINES["TIMEKPR"] = true
MACHINES["TMC0430"] = true
MACHINES["TMC0999"] = true
MACHINES["TMC208K"] = true
MACHINES["TMP68301"] = true
MACHINES["TMS1024"] = true
MACHINES["TMS5501"] = true
MACHINES["TMS6100"] = true
MACHINES["TMS9901"] = true
MACHINES["TMS9902"] = true
MACHINES["TMS9914"] = true
MACHINES["TPI6525"] = true
MACHINES["TSB12LV01A"] = true
MACHINES["TSCONF_DMA"] = true
MACHINES["TTL7400"] = true
MACHINES["TTL7404"] = true
MACHINES["TTL74123"] = true
MACHINES["TTL74145"] = true
MACHINES["TTL74148"] = true
MACHINES["TTL74153"] = true
MACHINES["TTL74157"] = true
MACHINES["TTL74161"] = true
MACHINES["TTL74164"] = true
MACHINES["TTL74165"] = true
MACHINES["TTL74166"] = true
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
MACHINES["UPD4701"] = true
MACHINES["UPD4991A"] = true
MACHINES["UPD4992"] = true
MACHINES["UPD7001"] = true
MACHINES["UPD7002"] = true
MACHINES["UPD7004"] = true
MACHINES["UPD71071"] = true
MACHINES["UPD765"] = true
MACHINES["V3021"] = true
MACHINES["VIC_PL192"] = true
MACHINES["VRENDER0"] = true
MACHINES["VT82C496"] = true
MACHINES["WATCHDOG"] = true
MACHINES["WD1000"] = true
MACHINES["WD1010"] = true
MACHINES["WD11C00_17"] = true
MACHINES["WD2010"] = true
MACHINES["WD33C9X"] = true
MACHINES["WD7600"] = true
MACHINES["WD_FDC"] = true
MACHINES["WOZFDC"] = true
MACHINES["WTL3132"] = true
MACHINES["X2201"] = true
MACHINES["X2212"] = true
MACHINES["X76F041"] = true
MACHINES["X76F100"] = true
MACHINES["XC1700E"] = true
MACHINES["YM2148"] = true
MACHINES["YM3802"] = true
MACHINES["Z8038"] = true
MACHINES["Z80CTC"] = true
MACHINES["Z80DAISY"] = true
MACHINES["Z80DMA"] = true
MACHINES["Z80PIO"] = true
MACHINES["Z80SCC"] = true
MACHINES["Z80SIO"] = true
MACHINES["Z80STI"] = true
MACHINES["Z8536"] = true

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
BUSES["ELECTRON"] = true
BUSES["ELECTRON_CART"] = true
BUSES["EP64"] = true
BUSES["EPSON_QX"] = true
BUSES["EPSON_SIO"] = true
BUSES["FMT_SCSI"] = true
BUSES["GAMATE"] = true
BUSES["GAMEBOY"] = true
BUSES["GAMEGEAR"] = true
BUSES["GBA"] = true
BUSES["GENERIC"] = true
BUSES["GIO64"] = true
BUSES["HEXBUS"] = true
BUSES["HP80_IO"] = true
BUSES["HP9845_IO"] = true
BUSES["HPDIO"] = true
BUSES["HPHIL"] = true
BUSES["HP_IPC_IO"] = true
BUSES["IEEE488"] = true
BUSES["IMI7000"] = true
BUSES["INTELLEC4"] = true
BUSES["INTERPRO_KEYBOARD"] = true
BUSES["INTERPRO_MOUSE"] = true
BUSES["INTERPRO_SR"] = true
BUSES["INTV"] = true
BUSES["INTV_CTRL"] = true
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
BUSES["NEOGEO"] = true
BUSES["NEOGEO_CTRL"] = true
BUSES["NES"] = true
BUSES["NES_CTRL"] = true
BUSES["NEWBRAIN"] = true
BUSES["NSCSI"] = true
BUSES["NUBUS"] = true
BUSES["O2"] = true
BUSES["ORICEXT"] = true
BUSES["P2000"] = true
BUSES["PASOPIA"] = true
BUSES["PC1512"] = true
BUSES["PC8801"] = true
BUSES["PCE"] = true
BUSES["PCE_CTRL"] = true
BUSES["PC_JOY"] = true
BUSES["PC_KBD"] = true
BUSES["PET"] = true
BUSES["PLUS4"] = true
BUSES["POFO"] = true
BUSES["PSI_KEYBOARD"] = true
BUSES["PSX_CONTROLLER"] = true
BUSES["PSX_PARALLEL"] = true
BUSES["QBUS"] = true
BUSES["QL"] = true
BUSES["RC2014"] = true
BUSES["RS232"] = true
BUSES["RTPC_KBD"] = true
BUSES["S100"] = true
BUSES["SAITEK_OSA"] = true
BUSES["SAMCOUPE_DRIVE_PORT"] = true
BUSES["SAMCOUPE_EXPANSION"] = true
BUSES["SAMCOUPE_MOUSE_PORT"] = true
BUSES["SATURN"] = true
BUSES["SAT_CTRL"] = true
BUSES["SBUS"] = true
BUSES["SCSI"] = true
BUSES["SCV"] = true
BUSES["SDK85"] = true
BUSES["SEGA8"] = true
BUSES["SG1000_EXP"] = true
BUSES["SGIKBD"] = true
BUSES["SMS_CTRL"] = true
BUSES["SMS_EXP"] = true
BUSES["SNES"] = true
BUSES["SNES_CTRL"] = true
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
FORMATS["COCO_RAWDSK"] = true
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
FORMATS["DMV_DSK"] = true
FORMATS["DS9_DSK"] = true
FORMATS["EP64_DSK"] = true
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
FORMATS["FS_CBMDOS"] = true
FORMATS["FS_COCO_OS9"] = true
FORMATS["FS_COCO_RSDOS"] = true
FORMATS["FS_ORIC_JASMIN"] = true
FORMATS["FS_PRODOS"] = true
FORMATS["FS_VTECH"] = true
FORMATS["G64_DSK"] = true
FORMATS["GTP_CAS"] = true
FORMATS["GUAB_DSK"] = true
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
FORMATS["SDF_DSK"] = true
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
FORMATS["TIM011_DSK"] = true
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
FORMATS["WD177X_DSK"] = true
FORMATS["X07_CAS"] = true
FORMATS["X1_TAP"] = true
FORMATS["XDF_DSK"] = true
FORMATS["ZX81_P"] = true


--------------------------------------------------
-- this is the list of driver libraries that
-- comprise MAME
--------------------------------------------------
function linkProjects_mame_mame(_target, _subtarget)
	links {
		"access",
		"acorn",
		"act",
		"adc",
		"adp",
		"agat",
		"akai",
		"alba",
		"alesis",
		"alliedl",
		"alpha",
		"altos",
		"amiga",
		"amirix",
		"ampro",
		"amstrad",
		"apf",
		"apollo",
		"apple",
		"appliedconcepts",
		"appliedtech",
		"arcadia",
		"aristocr",
		"at",
		"atari",
		"atlus",
		"att",
		"aviion",
		"banctec",
		"bandai",
		"barcrest",
		"be",
		"beehive",
		"bfm",
		"bitcorp",
		"bmc",
		"bnpo",
		"bondwell",
		"booth",
		"camputers",
		"canon",
		"cantab",
		"capcom",
		"casio",
		"commodore",
		"cce",
		"ccs",
		"ceres",
		"ces",
		"chromatics",
		"chrysler",
		"cinemat",
		"cirsa",
		"citoh",
		"coleco",
		"comad",
		"compugraphic",
		"comx",
		"concept",
		"conitec",
		"cromemco",
		"cvs",
		"cxg",
		"cybiko",
		"dai",
		"dataeast",
		"ddr",
		"dec",
		"dgrm",
		"dicksmth",
		"dms",
		"dooyong",
		"drc",
		"dulmont",
		"dynax",
		"eaca",
		"edevices",
		"efo",
		"elektor",
		"elektron",
		"emusys",
		"ensoniq",
		"enterprise",
		"entex",
		"eolith",
		"epoch",
		"epson",
		"ericsson",
		"excelent",
		"exidy",
		"exorterm",
		"f32",
		"fairchild",
		"fairlight",
		"falco",
		"fidelity",
		"force",
		"fujitsu",
		"funtech",
		"funworld",
		"fuuki",
		"gaelco",
		"galaxian",
		"gamepark",
		"gameplan",
		"gametron",
		"gottlieb",
		"gridcomp",
		"grundy",
		"h01x",
		"hartung",
		"hds",
		"heathkit",
		"hec2hrp",
		"hegenerglaser",
		"heurikon",
		"hitachi",
		"homebrew",
		"homelab",
		"hp",
		"husky",
		"ibm6580",
		"ibmpc",
		"ie15",
		"igs",
		"igt",
		"informer",
		"intel",
		"interpro",
		"interton",
		"irem",
		"isc",
		"itech",
		"jaleco",
		"jazz",
		"jpm",
		"kaneko",
		"kawai",
		"kaypro",
		"kiwako",
		"koei",
		"konami",
		"kontron",
		"korg",
		"kurzweil",
		"kyber",
		"kyocera",
		"labtam",
		"leapfrog",
		"learsiegler",
		"lsi",
		"luxor",
		"makerbot",
		"matic",
		"matsushi",
		"mattel",
		"maygay",
		"mchester",
		"meadows",
		"memotech",
		"mera",
		"merit",
		"metro",
		"mg1",
		"mgu",
		"microkey",
		"microsoft",
		"microterm",
		"midcoin",
		"midw8080",
		"midway",
		"miltonbradley",
		"mips",
		"misc",
		"misc_chess",
		"misc_handheld",
		"mit",
		"mits",
		"mitsubishi",
		"mizar",
		"morrow",
		"mos",
		"motorola",
		"mr",
		"msx",
		"multitch",
		"mupid",
		"nakajima",
		"namco",
		"nasco",
		"nascom",
		"natsemi",
		"ncd",
		"ne",
		"nec",
		"neogeo",
		"netronic",
		"next",
		"nichibut",
		"nintendo",
		"nix",
		"nmk",
		"nokia",
		"northstar",
		"novadesitec",
		"novag",
		"novation",
		"olivetti",
		"olympia",
		"omnibyte",
		"omori",
		"omron",
		"openuni",
		"orca",
		"osborne",
		"osi",
		"pacific",
		"pacman",
		"palm",
		"pc",
		"pce",
		"pdp1",
		"philips",
		"phoenix",
		"pinball",
		"pitronic",
		"playmark",
		"poly",
		"poly88",
		"positron",
		"promat",
		"psikyo",
		"psion",
		"quantel",
		"qume",
		"ramtek",
		"rare",
		"rca",
		"regnecentralen",
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
		"sanritsu",
		"sanyo",
		"saturn",
		"sega",
		"segacons",
		"seibu",
		"sequential",
		"sfrj",
		"seta",
		"sgi",
		"sharp",
		"siemens",
		"sigma",
		"sinclair",
		"skeleton",
		"slicer",
		"snk",
		"sony",
		"sony_news",
		"sord",
		"sssr",
		"stern",
		"stm",
		"subsino",
		"sun",
		"suna",
		"sunelect",
		"svi",
		"svision",
		"swtpc",
		"synertek",
		"ta",
		"tab",
		"taito",
		"tandberg",
		"tangerin",
		"tatsumi",
		"tatung",
		"tch",
		"teamconc",
		"tecfri",
		"technos",
		"tectoy",
		"tehkan",
		"tektroni",
		"telenova",
		"telercas",
		"televideo",
		"tesla",
		"thepit",
		"thomson",
		"ti",
		"tiger",
		"tigertel",
		"tiki",
		"toaplan",
		"tomy",
		"toshiba",
		"trainer",
		"trs",
		"tvgames",
		"ultimachine",
		"ultratec",
		"unicard",
		"unico",
		"unisonic",
		"unisys",
		"univers",
		"upl",
		"usp",
		"valadon",
		"venture",
		"verifone",
		"vidbrain",
		"videoton",
		"virtual",
		"visual",
		"votrax",
		"vsystem",
		"vtech",
		"wang",
		"wavemate",
		"westinghouse",
		"wing",
		"wyse",
		"xerox",
		"xussrpc",
		"yamaha",
		"yunsung",
		"zaccaria",
		"zenith",
		"zpa",
		"zvt",
		"shared", -- must stay near the end
	}
end


function createMAMEProjects(_target, _subtarget, _name)
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
		MAME_DIR .. "src/mame/shared",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
	}

	includedirs {
		ext_includedir("asio"),
		ext_includedir("flac"),
		ext_includedir("glm"),
		ext_includedir("jpeg"),
		ext_includedir("rapidjson"),
		ext_includedir("zlib")
	}

end


function createProjects_mame_mame(_target, _subtarget)

--------------------------------------------------
-- the following files are general components and
-- shared across a number of drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "shared")
files {
	MAME_DIR .. "src/mame/shared/*.h",
	MAME_DIR .. "src/mame/shared/*.cpp",
}

--------------------------------------------------
-- manufacturer-specific groupings for drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "access")
files {
	MAME_DIR .. "src/mame/access/*.cpp",
}

createMAMEProjects(_target, _subtarget, "acorn")
files {
	MAME_DIR .. "src/mame/acorn/*.cpp",
	MAME_DIR .. "src/mame/acorn/*.h",
}

createMAMEProjects(_target, _subtarget, "act")
files {
	MAME_DIR .. "src/mame/act/*.cpp",
	MAME_DIR .. "src/mame/act/*.h",
}

createMAMEProjects(_target, _subtarget, "adc")
files {
	MAME_DIR .. "src/mame/adc/*.cpp",
	MAME_DIR .. "src/mame/adc/*.h",
}

createMAMEProjects(_target, _subtarget, "adp")
files {
	MAME_DIR .. "src/mame/adp/*.cpp",
}

createMAMEProjects(_target, _subtarget, "agat")
files {
	MAME_DIR .. "src/mame/agat/*.cpp",
	MAME_DIR .. "src/mame/agat/*.h",
}

createMAMEProjects(_target, _subtarget, "akai")
files {
	MAME_DIR .. "src/mame/akai/*.cpp",
}

createMAMEProjects(_target, _subtarget, "alba")
files {
	MAME_DIR .. "src/mame/alba/*.cpp",
}

createMAMEProjects(_target, _subtarget, "alesis")
files {
	MAME_DIR .. "src/mame/alesis/*.cpp",
	MAME_DIR .. "src/mame/alesis/*.h",
}

createMAMEProjects(_target, _subtarget, "alliedl")
files {
	MAME_DIR .. "src/mame/alliedl/*.cpp",
}

createMAMEProjects(_target, _subtarget, "alpha")
files {
	MAME_DIR .. "src/mame/alpha/*.cpp",
	MAME_DIR .. "src/mame/alpha/*.h",
}

createMAMEProjects(_target, _subtarget, "altos")
files {
	MAME_DIR .. "src/mame/altos/*.cpp",
	MAME_DIR .. "src/mame/altos/*.h",
}

createMAMEProjects(_target, _subtarget, "amirix")
files {
	MAME_DIR .. "src/mame/amirix/*.cpp",
}

createMAMEProjects(_target, _subtarget, "amiga")
files {
	MAME_DIR .. "src/mame/amiga/*.cpp",
	MAME_DIR .. "src/mame/amiga/*.h",
}

createMAMEProjects(_target, _subtarget, "ampro")
files {
	MAME_DIR .. "src/mame/ampro/*.cpp",
}

createMAMEProjects(_target, _subtarget, "amstrad")
files {
	MAME_DIR .. "src/mame/amstrad/*.cpp",
	MAME_DIR .. "src/mame/amstrad/*.h",
}

createMAMEProjects(_target, _subtarget, "apf")
files {
	MAME_DIR .. "src/mame/apf/*.cpp",
}

createMAMEProjects(_target, _subtarget, "apollo")
files {
	MAME_DIR .. "src/mame/apollo/*.cpp",
	MAME_DIR .. "src/mame/apollo/*.h",
}

createMAMEProjects(_target, _subtarget, "apple")
files {
	MAME_DIR .. "src/mame/apple/*.cpp",
	MAME_DIR .. "src/mame/apple/*.h",
}

createMAMEProjects(_target, _subtarget, "appliedconcepts")
files {
	MAME_DIR .. "src/mame/appliedconcepts/*.cpp",
}

createMAMEProjects(_target, _subtarget, "appliedtech")
files {
	MAME_DIR .. "src/mame/appliedtech/*.cpp",
	MAME_DIR .. "src/mame/appliedtech/*.h",
}

createMAMEProjects(_target, _subtarget, "arcadia")
files {
	MAME_DIR .. "src/mame/arcadia/*.cpp",
	MAME_DIR .. "src/mame/arcadia/*.h",
}

createMAMEProjects(_target, _subtarget, "aristocr")
files {
	MAME_DIR .. "src/mame/aristocr/*.cpp",
}

createMAMEProjects(_target, _subtarget, "at")
files {
	MAME_DIR .. "src/mame/at/*.cpp",
}

createMAMEProjects(_target, _subtarget, "atari")
files {
	MAME_DIR .. "src/mame/atari/*.cpp",
	MAME_DIR .. "src/mame/atari/*.h",
	MAME_DIR .. "src/mame/atari/*.ipp",
}

createMAMEProjects(_target, _subtarget, "atlus")
files {
	MAME_DIR .. "src/mame/atlus/*.cpp",
}

createMAMEProjects(_target, _subtarget, "att")
files {
	MAME_DIR .. "src/mame/att/*.cpp",
}

createMAMEProjects(_target, _subtarget, "aviion")
files {
	MAME_DIR .. "src/mame/aviion/*.cpp",
}

createMAMEProjects(_target, _subtarget, "banctec")
files {
	MAME_DIR .. "src/mame/banctec/*.cpp",
}

createMAMEProjects(_target, _subtarget, "bandai")
files {
	MAME_DIR .. "src/mame/bandai/*.cpp",
	MAME_DIR .. "src/mame/bandai/*.h",
}

createMAMEProjects(_target, _subtarget, "barcrest")
files {
	MAME_DIR .. "src/mame/barcrest/*.cpp",
	MAME_DIR .. "src/mame/barcrest/*.h",
}

createMAMEProjects(_target, _subtarget, "be")
files {
	MAME_DIR .. "src/mame/be/*.cpp",
	MAME_DIR .. "src/mame/be/*.h",
}

createMAMEProjects(_target, _subtarget, "beehive")
files {
	MAME_DIR .. "src/mame/beehive/*.cpp",
}

createMAMEProjects(_target, _subtarget, "bfm")
files {
	MAME_DIR .. "src/mame/bfm/*.cpp",
	MAME_DIR .. "src/mame/bfm/*.h",
}

createMAMEProjects(_target, _subtarget, "bitcorp")
files {
	MAME_DIR .. "src/mame/bitcorp/*.cpp",
	MAME_DIR .. "src/mame/bitcorp/*.h",
}

createMAMEProjects(_target, _subtarget, "bmc")
files {
	MAME_DIR .. "src/mame/bmc/*.cpp",
}

createMAMEProjects(_target, _subtarget, "bnpo")
files {
	MAME_DIR .. "src/mame/bnpo/*.cpp",
	MAME_DIR .. "src/mame/bnpo/*.h",
}

createMAMEProjects(_target, _subtarget, "bondwell")
files {
	MAME_DIR .. "src/mame/bondwell/*.cpp",
	MAME_DIR .. "src/mame/bondwell/*.h",
}

createMAMEProjects(_target, _subtarget, "booth")
files {
	MAME_DIR .. "src/mame/booth/*.cpp",
	MAME_DIR .. "src/mame/booth/*.h",
}

createMAMEProjects(_target, _subtarget, "camputers")
files {
	MAME_DIR .. "src/mame/camputers/*.cpp",
}

createMAMEProjects(_target, _subtarget, "canon")
files {
	MAME_DIR .. "src/mame/canon/*.cpp",
	MAME_DIR .. "src/mame/canon/*.h",
}

createMAMEProjects(_target, _subtarget, "cantab")
files {
	MAME_DIR .. "src/mame/cantab/*.cpp",
}

createMAMEProjects(_target, _subtarget, "capcom")
files {
	MAME_DIR .. "src/mame/capcom/*.cpp",
	MAME_DIR .. "src/mame/capcom/*.h",
}

createMAMEProjects(_target, _subtarget, "casio")
files {
	MAME_DIR .. "src/mame/casio/*.cpp",
	MAME_DIR .. "src/mame/casio/*.h",
}

createMAMEProjects(_target, _subtarget, "commodore")
files {
	MAME_DIR .. "src/mame/commodore/*.cpp",
	MAME_DIR .. "src/mame/commodore/*.h",
}

createMAMEProjects(_target, _subtarget, "sssr")
files {
	MAME_DIR .. "src/mame/sssr/*.cpp",
	MAME_DIR .. "src/mame/sssr/*.h",
}

createMAMEProjects(_target, _subtarget, "cce")
files {
	MAME_DIR .. "src/mame/cce/*.cpp",
}

createMAMEProjects(_target, _subtarget, "ccs")
files {
	MAME_DIR .. "src/mame/ccs/*.cpp",
}

createMAMEProjects(_target, _subtarget, "ceres")
files {
	MAME_DIR .. "src/mame/ceres/*.cpp",
}

createMAMEProjects(_target, _subtarget, "ces")
files {
	MAME_DIR .. "src/mame/ces/*.cpp",
}

createMAMEProjects(_target, _subtarget, "chromatics")
files {
	MAME_DIR .. "src/mame/chromatics/*.cpp",
	MAME_DIR .. "src/mame/chromatics/*.h",
}

createMAMEProjects(_target, _subtarget, "chrysler")
files {
	MAME_DIR .. "src/mame/chrysler/*.cpp",
}

createMAMEProjects(_target, _subtarget, "cinemat")
files {
	MAME_DIR .. "src/mame/cinemat/*.cpp",
	MAME_DIR .. "src/mame/cinemat/*.h",
}

createMAMEProjects(_target, _subtarget, "cirsa")
files {
	MAME_DIR .. "src/mame/cirsa/*.cpp",
}

createMAMEProjects(_target, _subtarget, "citoh")
files {
	MAME_DIR .. "src/mame/citoh/*.cpp",
	MAME_DIR .. "src/mame/citoh/*.h",
}

createMAMEProjects(_target, _subtarget, "coleco")
files {
	MAME_DIR .. "src/mame/coleco/*.cpp",
	MAME_DIR .. "src/mame/coleco/*.h",
}

createMAMEProjects(_target, _subtarget, "comad")
files {
	MAME_DIR .. "src/mame/comad/*.cpp",
	MAME_DIR .. "src/mame/comad/*.h",
}

createMAMEProjects(_target, _subtarget, "compugraphic")
files {
	MAME_DIR .. "src/mame/compugraphic/*.cpp",
}

createMAMEProjects(_target, _subtarget, "cromemco")
files {
	MAME_DIR .. "src/mame/cromemco/*.cpp",
}

createMAMEProjects(_target, _subtarget, "comx")
files {
	MAME_DIR .. "src/mame/comx/*.cpp",
	MAME_DIR .. "src/mame/comx/*.h",
}

createMAMEProjects(_target, _subtarget, "concept")
files {
	MAME_DIR .. "src/mame/concept/*.cpp",
	MAME_DIR .. "src/mame/concept/*.h",
}

createMAMEProjects(_target, _subtarget, "conitec")
files {
	MAME_DIR .. "src/mame/conitec/*.cpp",
	MAME_DIR .. "src/mame/conitec/*.h",
}

createMAMEProjects(_target, _subtarget, "cvs")
files {
	MAME_DIR .. "src/mame/cvs/*.cpp",
	MAME_DIR .. "src/mame/cvs/*.h",
}

createMAMEProjects(_target, _subtarget, "cxg")
files {
	MAME_DIR .. "src/mame/cxg/*.cpp",
}

createMAMEProjects(_target, _subtarget, "cybiko")
files {
	MAME_DIR .. "src/mame/cybiko/*.cpp",
	MAME_DIR .. "src/mame/cybiko/*.h",
}

createMAMEProjects(_target, _subtarget, "dai")
files {
	MAME_DIR .. "src/mame/dai/*.cpp",
	MAME_DIR .. "src/mame/dai/*.h",
}

createMAMEProjects(_target, _subtarget, "dataeast")
files {
	MAME_DIR .. "src/mame/dataeast/*.cpp",
	MAME_DIR .. "src/mame/dataeast/*.h",
}

createMAMEProjects(_target, _subtarget, "ddr")
files {
	MAME_DIR .. "src/mame/ddr/*.cpp",
	MAME_DIR .. "src/mame/ddr/*.h",
}

createMAMEProjects(_target, _subtarget, "dec")
files {
	MAME_DIR .. "src/mame/dec/*.cpp",
	MAME_DIR .. "src/mame/dec/*.h",
}

createMAMEProjects(_target, _subtarget, "dgrm")
files {
	MAME_DIR .. "src/mame/dgrm/*.cpp",
	MAME_DIR .. "src/mame/dgrm/*.h",
}

createMAMEProjects(_target, _subtarget, "dicksmth")
files {
	MAME_DIR .. "src/mame/dicksmth/*.cpp",
	MAME_DIR .. "src/mame/dicksmth/*.h",
}

createMAMEProjects(_target, _subtarget, "dms")
files {
	MAME_DIR .. "src/mame/dms/*.cpp",
}

createMAMEProjects(_target, _subtarget, "dooyong")
files {
	MAME_DIR .. "src/mame/dooyong/*.cpp",
	MAME_DIR .. "src/mame/dooyong/*.cpp",
}

createMAMEProjects(_target, _subtarget, "drc")
files {
	MAME_DIR .. "src/mame/drc/*.cpp",
}

createMAMEProjects(_target, _subtarget, "dulmont")
files {
	MAME_DIR .. "src/mame/dulmont/*.cpp",
}

createMAMEProjects(_target, _subtarget, "dynax")
files {
	MAME_DIR .. "src/mame/dynax/*.cpp",
	MAME_DIR .. "src/mame/dynax/*.h",
}

createMAMEProjects(_target, _subtarget, "eaca")
files {
	MAME_DIR .. "src/mame/eaca/*.cpp",
}

createMAMEProjects(_target, _subtarget, "edevices")
files {
	MAME_DIR .. "src/mame/edevices/*.cpp",
	MAME_DIR .. "src/mame/edevices/*.h",
}

createMAMEProjects(_target, _subtarget, "efo")
files {
	MAME_DIR .. "src/mame/efo/*.cpp",
	MAME_DIR .. "src/mame/efo/*.h",
}

createMAMEProjects(_target, _subtarget, "elektor")
files {
	MAME_DIR .. "src/mame/elektor/*.cpp",
}

createMAMEProjects(_target, _subtarget, "elektron")
files {
	MAME_DIR .. "src/mame/elektron/*.cpp",
}

createMAMEProjects(_target, _subtarget, "emusys")
files {
	MAME_DIR .. "src/mame/emusys/*.cpp",
}

createMAMEProjects(_target, _subtarget, "ensoniq")
files {
	MAME_DIR .. "src/mame/ensoniq/*.cpp",
	MAME_DIR .. "src/mame/ensoniq/*.h",
}

createMAMEProjects(_target, _subtarget, "enterprise")
files {
	MAME_DIR .. "src/mame/enterprise/*.cpp",
	MAME_DIR .. "src/mame/enterprise/*.h",
}

createMAMEProjects(_target, _subtarget, "entex")
files {
	MAME_DIR .. "src/mame/entex/*.cpp",
}

createMAMEProjects(_target, _subtarget, "eolith")
files {
	MAME_DIR .. "src/mame/eolith/*.cpp",
	MAME_DIR .. "src/mame/eolith/*.h",
}

createMAMEProjects(_target, _subtarget, "epoch")
files {
	MAME_DIR .. "src/mame/epoch/*.cpp",
	MAME_DIR .. "src/mame/epoch/*.h",
}

createMAMEProjects(_target, _subtarget, "epson")
files {
	MAME_DIR .. "src/mame/epson/*.cpp",
	MAME_DIR .. "src/mame/epson/*.h",
}

createMAMEProjects(_target, _subtarget, "ericsson")
files {
	MAME_DIR .. "src/mame/ericsson/*.cpp",
	MAME_DIR .. "src/mame/ericsson/*.h",
}

createMAMEProjects(_target, _subtarget, "excelent")
files {
	MAME_DIR .. "src/mame/excelent/*.cpp",
	MAME_DIR .. "src/mame/excelent/*.h",
}

createMAMEProjects(_target, _subtarget, "exidy")
files {
	MAME_DIR .. "src/mame/exidy/*.cpp",
	MAME_DIR .. "src/mame/exidy/*.h",
}

createMAMEProjects(_target, _subtarget, "exorterm")
files {
	MAME_DIR .. "src/mame/exorterm/*.cpp",
}

createMAMEProjects(_target, _subtarget, "f32")
files {
	MAME_DIR .. "src/mame/f32/*.cpp",
	MAME_DIR .. "src/mame/f32/*.h",
}

createMAMEProjects(_target, _subtarget, "fairchild")
files {
	MAME_DIR .. "src/mame/fairchild/*.cpp",
	MAME_DIR .. "src/mame/fairchild/*.h",
}

createMAMEProjects(_target, _subtarget, "fairlight")
files {
	MAME_DIR .. "src/mame/fairlight/*.cpp",
	MAME_DIR .. "src/mame/fairlight/*.h",
}

createMAMEProjects(_target, _subtarget, "falco")
files {
	MAME_DIR .. "src/mame/falco/*.cpp",
	MAME_DIR .. "src/mame/falco/*.h",
}

createMAMEProjects(_target, _subtarget, "fidelity")
files {
	MAME_DIR .. "src/mame/fidelity/*.cpp",
	MAME_DIR .. "src/mame/fidelity/*.h",
}

createMAMEProjects(_target, _subtarget, "force")
files {
	MAME_DIR .. "src/mame/force/*.cpp",
}

createMAMEProjects(_target, _subtarget, "fujitsu")
files {
	MAME_DIR .. "src/mame/fujitsu/*.cpp",
	MAME_DIR .. "src/mame/fujitsu/*.h",
}

createMAMEProjects(_target, _subtarget, "funtech")
files {
	MAME_DIR .. "src/mame/funtech/*.cpp",
	MAME_DIR .. "src/mame/funtech/*.h",
}

createMAMEProjects(_target, _subtarget, "funworld")
files {
	MAME_DIR .. "src/mame/funworld/*.cpp",
	MAME_DIR .. "src/mame/funworld/*.h",
}

createMAMEProjects(_target, _subtarget, "fuuki")
files {
	MAME_DIR .. "src/mame/fuuki/*.cpp",
	MAME_DIR .. "src/mame/fuuki/*.h",
}

createMAMEProjects(_target, _subtarget, "gaelco")
files {
	MAME_DIR .. "src/mame/gaelco/*.cpp",
	MAME_DIR .. "src/mame/gaelco/*.h",
}

createMAMEProjects(_target, _subtarget, "galaxian")
files {
	MAME_DIR .. "src/mame/galaxian/*.cpp",
	MAME_DIR .. "src/mame/galaxian/*.h",
}

createMAMEProjects(_target, _subtarget, "gamepark")
files {
	MAME_DIR .. "src/mame/gamepark/*.cpp",
	MAME_DIR .. "src/mame/gamepark/*.h",
}

createMAMEProjects(_target, _subtarget, "gameplan")
files {
	MAME_DIR .. "src/mame/gameplan/*.cpp",
	MAME_DIR .. "src/mame/gameplan/*.h",
}

createMAMEProjects(_target, _subtarget, "gametron")
files {
	MAME_DIR .. "src/mame/gametron/*.cpp",
	MAME_DIR .. "src/mame/gametron/*.h",
}

createMAMEProjects(_target, _subtarget, "gottlieb")
files {
	MAME_DIR .. "src/mame/gottlieb/*.cpp",
	MAME_DIR .. "src/mame/gottlieb/*.h",
}

createMAMEProjects(_target, _subtarget, "gridcomp")
files {
	MAME_DIR .. "src/mame/gridcomp/*.cpp",
	MAME_DIR .. "src/mame/gridcomp/*.h",
}

createMAMEProjects(_target, _subtarget, "grundy")
files {
	MAME_DIR .. "src/mame/grundy/*.cpp",
	MAME_DIR .. "src/mame/grundy/*.h",
}

createMAMEProjects(_target, _subtarget, "h01x")
files {
	MAME_DIR .. "src/mame/h01x/*.cpp",
}

createMAMEProjects(_target, _subtarget, "hartung")
files {
	MAME_DIR .. "src/mame/hartung/*.cpp",
}

createMAMEProjects(_target, _subtarget, "hds")
files {
	MAME_DIR .. "src/mame/hds/*.cpp",
	MAME_DIR .. "src/mame/hds/*.h",
}

createMAMEProjects(_target, _subtarget, "heathkit")
files {
	MAME_DIR .. "src/mame/heathkit/*.cpp",
}

createMAMEProjects(_target, _subtarget, "hec2hrp")
files {
	MAME_DIR .. "src/mame/hec2hrp/*.cpp",
	MAME_DIR .. "src/mame/hec2hrp/*.h",
}

createMAMEProjects(_target, _subtarget, "hegenerglaser")
files {
	MAME_DIR .. "src/mame/hegenerglaser/*.cpp",
	MAME_DIR .. "src/mame/hegenerglaser/*.h",
}

createMAMEProjects(_target, _subtarget, "heurikon")
files {
	MAME_DIR .. "src/mame/heurikon/*.cpp",
}

createMAMEProjects(_target, _subtarget, "hitachi")
files {
	MAME_DIR .. "src/mame/hitachi/*.cpp",
}

createMAMEProjects(_target, _subtarget, "homebrew")
files {
	MAME_DIR .. "src/mame/homebrew/*.cpp",
}

createMAMEProjects(_target, _subtarget, "homelab")
files {
	MAME_DIR .. "src/mame/homelab/*.cpp",
}

createMAMEProjects(_target, _subtarget, "hp")
files {
	MAME_DIR .. "src/mame/hp/*.cpp",
	MAME_DIR .. "src/mame/hp/*.h",
}

createMAMEProjects(_target, _subtarget, "husky")
files {
	MAME_DIR .. "src/mame/husky/*.cpp",
}

createMAMEProjects(_target, _subtarget, "ibm6580")
files {
	MAME_DIR .. "src/mame/ibm6580/*.cpp",
	MAME_DIR .. "src/mame/ibm6580/*.h",
}

createMAMEProjects(_target, _subtarget, "ibmpc")
files {
	MAME_DIR .. "src/mame/ibmpc/*.cpp",
}

createMAMEProjects(_target, _subtarget, "ie15")
files {
	MAME_DIR .. "src/mame/ie15/*.cpp",
}

createMAMEProjects(_target, _subtarget, "igs")
files {
	MAME_DIR .. "src/mame/igs/*.cpp",
	MAME_DIR .. "src/mame/igs/*.h",
}

createMAMEProjects(_target, _subtarget, "igt")
files {
	MAME_DIR .. "src/mame/igt/*.cpp",
}

createMAMEProjects(_target, _subtarget, "informer")
files {
	MAME_DIR .. "src/mame/informer/*.cpp",
	MAME_DIR .. "src/mame/informer/*.h",
}

createMAMEProjects(_target, _subtarget, "intel")
files {
	MAME_DIR .. "src/mame/intel/*.cpp",
	MAME_DIR .. "src/mame/intel/*.h",
}

createMAMEProjects(_target, _subtarget, "interpro")
files {
	MAME_DIR .. "src/mame/interpro/*.cpp",
	MAME_DIR .. "src/mame/interpro/*.h",
}

createMAMEProjects(_target, _subtarget, "interton")
files {
	MAME_DIR .. "src/mame/interton/*.cpp",
	MAME_DIR .. "src/mame/interton/*.h",
}

createMAMEProjects(_target, _subtarget, "irem")
files {
	MAME_DIR .. "src/mame/irem/*.cpp",
	MAME_DIR .. "src/mame/irem/*.h",
}

createMAMEProjects(_target, _subtarget, "itech")
files {
	MAME_DIR .. "src/mame/itech/*.cpp",
	MAME_DIR .. "src/mame/itech/*.h",
}

createMAMEProjects(_target, _subtarget, "isc")
files {
	MAME_DIR .. "src/mame/isc/*.cpp",
}

createMAMEProjects(_target, _subtarget, "jaleco")
files {
	MAME_DIR .. "src/mame/jaleco/*.cpp",
	MAME_DIR .. "src/mame/jaleco/*.h",
}

createMAMEProjects(_target, _subtarget, "jazz")
files {
	MAME_DIR .. "src/mame/jazz/*.cpp",
	MAME_DIR .. "src/mame/jazz/*.h",
}

createMAMEProjects(_target, _subtarget, "jpm")
files {
	MAME_DIR .. "src/mame/jpm/*.cpp",
	MAME_DIR .. "src/mame/jpm/*.h",
}

createMAMEProjects(_target, _subtarget, "kaneko")
files {
	MAME_DIR .. "src/mame/kaneko/*.cpp",
	MAME_DIR .. "src/mame/kaneko/*.h",
}

createMAMEProjects(_target, _subtarget, "kawai")
files {
	MAME_DIR .. "src/mame/kawai/*.cpp",
}

createMAMEProjects(_target, _subtarget, "kaypro")
files {
	MAME_DIR .. "src/mame/kaypro/*.cpp",
	MAME_DIR .. "src/mame/kaypro/*.h",
}

createMAMEProjects(_target, _subtarget, "kiwako")
files {
	MAME_DIR .. "src/mame/kiwako/*.cpp",
	MAME_DIR .. "src/mame/kiwako/*.h",
}

createMAMEProjects(_target, _subtarget, "koei")
files {
	MAME_DIR .. "src/mame/koei/*.cpp",
}

createMAMEProjects(_target, _subtarget, "konami")
files {
	MAME_DIR .. "src/mame/konami/*.cpp",
	MAME_DIR .. "src/mame/konami/*.h",
}

createMAMEProjects(_target, _subtarget, "kontron")
files {
	MAME_DIR .. "src/mame/kontron/*.cpp",
}

createMAMEProjects(_target, _subtarget, "korg")
files {
	MAME_DIR .. "src/mame/korg/*.cpp",
}

createMAMEProjects(_target, _subtarget, "kurzweil")
files {
	MAME_DIR .. "src/mame/kurzweil/*.cpp",
}

createMAMEProjects(_target, _subtarget, "kyber")
files {
	MAME_DIR .. "src/mame/kyber/*.cpp",
}

createMAMEProjects(_target, _subtarget, "kyocera")
files {
	MAME_DIR .. "src/mame/kyocera/*.cpp",
	MAME_DIR .. "src/mame/kyocera/*.h",
}

createMAMEProjects(_target, _subtarget, "labtam")
files {
	MAME_DIR .. "src/mame/labtam/*.cpp",
}

createMAMEProjects(_target, _subtarget, "leapfrog")
files {
	MAME_DIR .. "src/mame/leapfrog/*.cpp",
}

createMAMEProjects(_target, _subtarget, "learsiegler")
files {
	MAME_DIR .. "src/mame/learsiegler/*.cpp",
}

createMAMEProjects(_target, _subtarget, "lsi")
files {
	MAME_DIR .. "src/mame/lsi/*.cpp",
	MAME_DIR .. "src/mame/lsi/*.h",
}

createMAMEProjects(_target, _subtarget, "luxor")
files {
	MAME_DIR .. "src/mame/luxor/*.cpp",
	MAME_DIR .. "src/mame/luxor/*.h",
}

createMAMEProjects(_target, _subtarget, "makerbot")
files {
	MAME_DIR .. "src/mame/makerbot/*.cpp",
}

createMAMEProjects(_target, _subtarget, "matic")
files {
	MAME_DIR .. "src/mame/matic/*.cpp",
}

createMAMEProjects(_target, _subtarget, "matsushi")
files {
	MAME_DIR .. "src/mame/matsushi/*.cpp",
}

createMAMEProjects(_target, _subtarget, "mattel")
files {
	MAME_DIR .. "src/mame/mattel/*.cpp",
	MAME_DIR .. "src/mame/mattel/*.h",
}

createMAMEProjects(_target, _subtarget, "maygay")
files {
	MAME_DIR .. "src/mame/maygay/*.cpp",
	MAME_DIR .. "src/mame/maygay/*.h",
}

createMAMEProjects(_target, _subtarget, "mchester")
files {
	MAME_DIR .. "src/mame/mchester/*.cpp",
}

createMAMEProjects(_target, _subtarget, "meadows")
files {
	MAME_DIR .. "src/mame/meadows/*.cpp",
	MAME_DIR .. "src/mame/meadows/*.h",
}

createMAMEProjects(_target, _subtarget, "memotech")
files {
	MAME_DIR .. "src/mame/memotech/*.cpp",
	MAME_DIR .. "src/mame/memotech/*.h",
}

createMAMEProjects(_target, _subtarget, "mera")
files {
	MAME_DIR .. "src/mame/mera/*.cpp",
}

createMAMEProjects(_target, _subtarget, "merit")
files {
	MAME_DIR .. "src/mame/merit/*.cpp",
}

createMAMEProjects(_target, _subtarget, "metro")
files {
	MAME_DIR .. "src/mame/metro/*.cpp",
	MAME_DIR .. "src/mame/metro/*.h",
}

createMAMEProjects(_target, _subtarget, "mg1")
files {
	MAME_DIR .. "src/mame/mg1/*.cpp",
}

createMAMEProjects(_target, _subtarget, "mgu")
files {
	MAME_DIR .. "src/mame/mgu/*.cpp",
}

createMAMEProjects(_target, _subtarget, "microkey")
files {
	MAME_DIR .. "src/mame/microkey/*.cpp",
	MAME_DIR .. "src/mame/microkey/*.h",
}

createMAMEProjects(_target, _subtarget, "microsoft")
files {
	MAME_DIR .. "src/mame/microsoft/*.cpp",
}

createMAMEProjects(_target, _subtarget, "microterm")
files {
	MAME_DIR .. "src/mame/microterm/*.cpp",
}

createMAMEProjects(_target, _subtarget, "midcoin")
files {
	MAME_DIR .. "src/mame/midcoin/*.cpp",
}

createMAMEProjects(_target, _subtarget, "midw8080")
files {
	MAME_DIR .. "src/mame/midw8080/*.cpp",
	MAME_DIR .. "src/mame/midw8080/*.h",
}

createMAMEProjects(_target, _subtarget, "midway")
files {
	MAME_DIR .. "src/mame/midway/*.cpp",
	MAME_DIR .. "src/mame/midway/*.h",
	MAME_DIR .. "src/mame/midway/*.ipp",
}

createMAMEProjects(_target, _subtarget, "miltonbradley")
files {
	MAME_DIR .. "src/mame/miltonbradley/*.cpp",
	MAME_DIR .. "src/mame/miltonbradley/*.h",
}

createMAMEProjects(_target, _subtarget, "mips")
files {
	MAME_DIR .. "src/mame/mips/*.cpp",
}

createMAMEProjects(_target, _subtarget, "mit")
files {
	MAME_DIR .. "src/mame/mit/*.cpp",
	MAME_DIR .. "src/mame/mit/*.h",
}

createMAMEProjects(_target, _subtarget, "mits")
files {
	MAME_DIR .. "src/mame/mits/*.cpp",
	MAME_DIR .. "src/mame/mits/*.cpp",
}

createMAMEProjects(_target, _subtarget, "mitsubishi")
files {
	MAME_DIR .. "src/mame/mitsubishi/*.cpp",
}

createMAMEProjects(_target, _subtarget, "mizar")
files {
	MAME_DIR .. "src/mame/mizar/*.cpp",
}

createMAMEProjects(_target, _subtarget, "morrow")
files {
	MAME_DIR .. "src/mame/morrow/*.cpp",
}

createMAMEProjects(_target, _subtarget, "mos")
files {
	MAME_DIR .. "src/mame/mos/*.cpp",
}

createMAMEProjects(_target, _subtarget, "motorola")
files {
	MAME_DIR .. "src/mame/motorola/*.cpp",
}

createMAMEProjects(_target, _subtarget, "mr")
files {
	MAME_DIR .. "src/mame/mr/*.cpp",
	MAME_DIR .. "src/mame/mr/*.h",
}

createMAMEProjects(_target, _subtarget, "msx")
files {
	MAME_DIR .. "src/mame/msx/*.cpp",
	MAME_DIR .. "src/mame/msx/*.h",
}

createMAMEProjects(_target, _subtarget, "multitch")
files {
	MAME_DIR .. "src/mame/multitch/*.cpp",
	MAME_DIR .. "src/mame/multitch/*.h",
}

createMAMEProjects(_target, _subtarget, "mupid")
files {
	MAME_DIR .. "src/mame/mupid/*.cpp",
}

createMAMEProjects(_target, _subtarget, "nakajima")
files {
	MAME_DIR .. "src/mame/nakajima/*.cpp",
}

createMAMEProjects(_target, _subtarget, "namco")
files {
	MAME_DIR .. "src/mame/namco/*.cpp",
	MAME_DIR .. "src/mame/namco/*.h",
}

createMAMEProjects(_target, _subtarget, "nasco")
files {
	MAME_DIR .. "src/mame/nasco/*.cpp",
}

createMAMEProjects(_target, _subtarget, "nascom")
files {
	MAME_DIR .. "src/mame/nascom/*.cpp",
}

createMAMEProjects(_target, _subtarget, "natsemi")
files {
	MAME_DIR .. "src/mame/natsemi/*.cpp",
}

createMAMEProjects(_target, _subtarget, "ncd")
files {
	MAME_DIR .. "src/mame/ncd/*.cpp",
	MAME_DIR .. "src/mame/ncd/*.h",
}

createMAMEProjects(_target, _subtarget, "ne")
files {
	MAME_DIR .. "src/mame/ne/*.cpp",
	MAME_DIR .. "src/mame/ne/*.h",
}

createMAMEProjects(_target, _subtarget, "nec")
files {
	MAME_DIR .. "src/mame/nec/*.cpp",
	MAME_DIR .. "src/mame/nec/*.h",
}

createMAMEProjects(_target, _subtarget, "neogeo")
files {
	MAME_DIR .. "src/mame/neogeo/*.cpp",
	MAME_DIR .. "src/mame/neogeo/*.h",
}

createMAMEProjects(_target, _subtarget, "netronic")
files {
	MAME_DIR .. "src/mame/netronic/*.cpp",
	MAME_DIR .. "src/mame/netronic/*.h",
	MAME_DIR .. "src/mame/netronic/*.cpp",
}

createMAMEProjects(_target, _subtarget, "next")
files {
	MAME_DIR .. "src/mame/next/*.cpp",
	MAME_DIR .. "src/mame/next/*.h",
}

createMAMEProjects(_target, _subtarget, "nichibut")
files {
	MAME_DIR .. "src/mame/nichibut/*.cpp",
	MAME_DIR .. "src/mame/nichibut/*.h",
}

createMAMEProjects(_target, _subtarget, "nintendo")
files {
	MAME_DIR .. "src/mame/nintendo/*.cpp",
	MAME_DIR .. "src/mame/nintendo/*.h",
	MAME_DIR .. "src/mame/nintendo/*.ipp",
}

createMAMEProjects(_target, _subtarget, "nix")
files {
	MAME_DIR .. "src/mame/nix/*.cpp",
	MAME_DIR .. "src/mame/nix/*.h",
}

createMAMEProjects(_target, _subtarget, "nmk")
files {
	MAME_DIR .. "src/mame/nmk/*.cpp",
	MAME_DIR .. "src/mame/nmk/*.h",
}

createMAMEProjects(_target, _subtarget, "nokia")
files {
	MAME_DIR .. "src/mame/nokia/*.cpp",
	MAME_DIR .. "src/mame/nokia/*.h",
}

createMAMEProjects(_target, _subtarget, "northstar")
files {
	MAME_DIR .. "src/mame/northstar/*.cpp",
}

createMAMEProjects(_target, _subtarget, "novadesitec")
files {
	MAME_DIR .. "src/mame/novadesitec/*.cpp",
}

createMAMEProjects(_target, _subtarget, "novag")
files {
	MAME_DIR .. "src/mame/novag/*.cpp",
}

createMAMEProjects(_target, _subtarget, "novation")
files {
	MAME_DIR .. "src/mame/novation/*.cpp",
}

createMAMEProjects(_target, _subtarget, "olivetti")
files {
	MAME_DIR .. "src/mame/olivetti/*.cpp",
	MAME_DIR .. "src/mame/olivetti/*.h",
}

createMAMEProjects(_target, _subtarget, "olympia")
files {
	MAME_DIR .. "src/mame/olympia/*.cpp",
	MAME_DIR .. "src/mame/olympia/*.h",
}

createMAMEProjects(_target, _subtarget, "omnibyte")
files {
	MAME_DIR .. "src/mame/omnibyte/*.cpp",
	MAME_DIR .. "src/mame/omnibyte/*.h",
}

createMAMEProjects(_target, _subtarget, "omori")
files {
	MAME_DIR .. "src/mame/omori/*.cpp",
	MAME_DIR .. "src/mame/omori/*.h",
}

createMAMEProjects(_target, _subtarget, "omron")
files {
	MAME_DIR .. "src/mame/omron/*.cpp",
}

createMAMEProjects(_target, _subtarget, "openuni")
files {
	MAME_DIR .. "src/mame/openuni/*.cpp",
}

createMAMEProjects(_target, _subtarget, "orca")
files {
	MAME_DIR .. "src/mame/orca/*.cpp",
	MAME_DIR .. "src/mame/orca/*.h",
}

createMAMEProjects(_target, _subtarget, "osborne")
files {
	MAME_DIR .. "src/mame/osborne/*.cpp",
	MAME_DIR .. "src/mame/osborne/*.h",
}

createMAMEProjects(_target, _subtarget, "osi")
files {
	MAME_DIR .. "src/mame/osi/*.cpp",
	MAME_DIR .. "src/mame/osi/*.h",
}

createMAMEProjects(_target, _subtarget, "pacific")
files {
	MAME_DIR .. "src/mame/pacific/*.cpp",
	MAME_DIR .. "src/mame/pacific/*.h",
}

createMAMEProjects(_target, _subtarget, "pacman")
files {
	MAME_DIR .. "src/mame/pacman/*.cpp",
	MAME_DIR .. "src/mame/pacman/*.h",
}

createMAMEProjects(_target, _subtarget, "palm")
files {
	MAME_DIR .. "src/mame/palm/*.cpp",
	MAME_DIR .. "src/mame/palm/*.ipp",
}

createMAMEProjects(_target, _subtarget, "pc")
files {
	MAME_DIR .. "src/mame/pc/*.cpp",
	MAME_DIR .. "src/mame/pc/*.h",
}

createMAMEProjects(_target, _subtarget, "pce")
files {
	MAME_DIR .. "src/mame/pce/*.cpp",
	MAME_DIR .. "src/mame/pce/*.cpp",
}

createMAMEProjects(_target, _subtarget, "pdp1")
files {
	MAME_DIR .. "src/mame/pdp1/*.cpp",
	MAME_DIR .. "src/mame/pdp1/*.h",
}

createMAMEProjects(_target, _subtarget, "philips")
files {
	MAME_DIR .. "src/mame/philips/*.cpp",
	MAME_DIR .. "src/mame/philips/*.h",
}

createMAMEProjects(_target, _subtarget, "phoenix")
files {
	MAME_DIR .. "src/mame/phoenix/*.cpp",
	MAME_DIR .. "src/mame/phoenix/*.h",
}

createMAMEProjects(_target, _subtarget, "pitronic")
files {
	MAME_DIR .. "src/mame/pitronic/*.cpp",
}

createMAMEProjects(_target, _subtarget, "playmark")
files {
	MAME_DIR .. "src/mame/playmark/*.cpp",
	MAME_DIR .. "src/mame/playmark/*.h",
}

createMAMEProjects(_target, _subtarget, "poly")
files {
	MAME_DIR .. "src/mame/poly/*.cpp",
	MAME_DIR .. "src/mame/poly/*.h",
}

createMAMEProjects(_target, _subtarget, "poly88")
files {
	MAME_DIR .. "src/mame/poly88/*.cpp",
	MAME_DIR .. "src/mame/poly88/*.h",
}

createMAMEProjects(_target, _subtarget, "positron")
files {
	MAME_DIR .. "src/mame/positron/*.cpp",
}

createMAMEProjects(_target, _subtarget, "promat")
files {
	MAME_DIR .. "src/mame/promat/*.cpp",
}

createMAMEProjects(_target, _subtarget, "psikyo")
files {
	MAME_DIR .. "src/mame/psikyo/*.cpp",
	MAME_DIR .. "src/mame/psikyo/*.h",
}

createMAMEProjects(_target, _subtarget, "psion")
files {
	MAME_DIR .. "src/mame/psion/*.cpp",
	MAME_DIR .. "src/mame/psion/*.h",
}

createMAMEProjects(_target, _subtarget, "quantel")
files {
	MAME_DIR .. "src/mame/quantel/*.cpp",
}

createMAMEProjects(_target, _subtarget, "qume")
files {
	MAME_DIR .. "src/mame/qume/*.cpp",
}

createMAMEProjects(_target, _subtarget, "ramtek")
files {
	MAME_DIR .. "src/mame/ramtek/*.cpp",
	MAME_DIR .. "src/mame/ramtek/*.h",
}

createMAMEProjects(_target, _subtarget, "rare")
files {
	MAME_DIR .. "src/mame/rare/*.cpp",
	MAME_DIR .. "src/mame/rare/*.h",
}

createMAMEProjects(_target, _subtarget, "rca")
files {
	MAME_DIR .. "src/mame/rca/*.cpp",
	MAME_DIR .. "src/mame/rca/*.h",
}

createMAMEProjects(_target, _subtarget, "regnecentralen")
files {
	MAME_DIR .. "src/mame/regnecentralen/*.cpp",
	MAME_DIR .. "src/mame/regnecentralen/*.h",
}

createMAMEProjects(_target, _subtarget, "rm")
files {
	MAME_DIR .. "src/mame/rm/*.cpp",
	MAME_DIR .. "src/mame/rm/*.h",
}

createMAMEProjects(_target, _subtarget, "robotron")
files {
	MAME_DIR .. "src/mame/robotron/*.cpp",
}

createMAMEProjects(_target, _subtarget, "roland")
files {
	MAME_DIR .. "src/mame/roland/*.cpp",
	MAME_DIR .. "src/mame/roland/*.h",
}

createMAMEProjects(_target, _subtarget, "rolm")
files {
	MAME_DIR .. "src/mame/rolm/*.cpp",
}

createMAMEProjects(_target, _subtarget, "rockwell")
files {
	MAME_DIR .. "src/mame/rockwell/*.cpp",
	MAME_DIR .. "src/mame/rockwell/*.h",
}

createMAMEProjects(_target, _subtarget, "rtpc")
files {
	MAME_DIR .. "src/mame/rtpc/*.cpp",
	MAME_DIR .. "src/mame/rtpc/*.h",
}

createMAMEProjects(_target, _subtarget, "sage")
files {
	MAME_DIR .. "src/mame/sage/*.cpp",
	MAME_DIR .. "src/mame/sage/*.h",
}

createMAMEProjects(_target, _subtarget, "saitek")
files {
	MAME_DIR .. "src/mame/saitek/*.cpp",
	MAME_DIR .. "src/mame/saitek/*.h",
}

createMAMEProjects(_target, _subtarget, "samcoupe")
files {
	MAME_DIR .. "src/mame/samcoupe/*.cpp",
}

createMAMEProjects(_target, _subtarget, "samsung")
files {
	MAME_DIR .. "src/mame/samsung/*.cpp",
}

createMAMEProjects(_target, _subtarget, "sanritsu")
files {
	MAME_DIR .. "src/mame/sanritsu/*.cpp",
	MAME_DIR .. "src/mame/sanritsu/*.h",
}

createMAMEProjects(_target, _subtarget, "sanyo")
files {
	MAME_DIR .. "src/mame/sanyo/*.cpp",
	MAME_DIR .. "src/mame/sanyo/*.h",
}

createMAMEProjects(_target, _subtarget, "saturn")
files {
	MAME_DIR .. "src/mame/saturn/*.cpp",
}

createMAMEProjects(_target, _subtarget, "sega")
files {
	MAME_DIR .. "src/mame/sega/*.cpp",
	MAME_DIR .. "src/mame/sega/*.h",
	MAME_DIR .. "src/mame/sega/*.ipp",
}

createMAMEProjects(_target, _subtarget, "segacons")
files {
	MAME_DIR .. "src/mame/segacons/*.cpp",
	MAME_DIR .. "src/mame/segacons/*.h",
}

createMAMEProjects(_target, _subtarget, "seibu")
files {
	MAME_DIR .. "src/mame/seibu/*.cpp",
	MAME_DIR .. "src/mame/seibu/*.h",
	MAME_DIR .. "src/mame/seibu/*.ipp",
}

createMAMEProjects(_target, _subtarget, "sequential")
files {
	MAME_DIR .. "src/mame/sequential/*.cpp",
}

createMAMEProjects(_target, _subtarget, "sfrj")
files {
	MAME_DIR .. "src/mame/sfrj/*.cpp",
	MAME_DIR .. "src/mame/sfrj/*.h",
}

createMAMEProjects(_target, _subtarget, "seta")
files {
	MAME_DIR .. "src/mame/seta/*.cpp",
	MAME_DIR .. "src/mame/seta/*.h",
}

createMAMEProjects(_target, _subtarget, "sgi")
files {
	MAME_DIR .. "src/mame/sgi/*.cpp",
	MAME_DIR .. "src/mame/sgi/*.h",
}

createMAMEProjects(_target, _subtarget, "sharp")
files {
	MAME_DIR .. "src/mame/sharp/*.cpp",
	MAME_DIR .. "src/mame/sharp/*.h",
}

createMAMEProjects(_target, _subtarget, "siemens")
files {
	MAME_DIR .. "src/mame/siemens/*.cpp",
	MAME_DIR .. "src/mame/siemens/*.h",
}

createMAMEProjects(_target, _subtarget, "sinclair")
files {
	MAME_DIR .. "src/mame/sinclair/*.cpp",
	MAME_DIR .. "src/mame/sinclair/*.h",
}

createMAMEProjects(_target, _subtarget, "sigma")
files {
	MAME_DIR .. "src/mame/sigma/*.cpp",
	MAME_DIR .. "src/mame/sigma/*.h",
}

createMAMEProjects(_target, _subtarget, "slicer")
files {
	MAME_DIR .. "src/mame/slicer/*.cpp",
}

createMAMEProjects(_target, _subtarget, "snk")
files {
	MAME_DIR .. "src/mame/snk/*.cpp",
	MAME_DIR .. "src/mame/snk/*.h",
	MAME_DIR .. "src/mame/snk/*.ipp",
}

createMAMEProjects(_target, _subtarget, "sony")
files {
	MAME_DIR .. "src/mame/sony/*.cpp",
	MAME_DIR .. "src/mame/sony/*.h",
}

createMAMEProjects(_target, _subtarget, "sony_news")
files {
	MAME_DIR .. "src/mame/sony_news/*.cpp",
	MAME_DIR .. "src/mame/sony_news/*.h",
}

createMAMEProjects(_target, _subtarget, "sord")
files {
	MAME_DIR .. "src/mame/sord/*.cpp",
}

createMAMEProjects(_target, _subtarget, "stern")
files {
	MAME_DIR .. "src/mame/stern/*.cpp",
	MAME_DIR .. "src/mame/stern/*.h",
}

createMAMEProjects(_target, _subtarget, "stm")
files {
	MAME_DIR .. "src/mame/stm/*.cpp",
}

createMAMEProjects(_target, _subtarget, "subsino")
files {
	MAME_DIR .. "src/mame/subsino/*.cpp",
	MAME_DIR .. "src/mame/subsino/*.h",
}

createMAMEProjects(_target, _subtarget, "sun")
files {
	MAME_DIR .. "src/mame/sun/*.cpp",
}

createMAMEProjects(_target, _subtarget, "suna")
files {
	MAME_DIR .. "src/mame/suna/*.cpp",
	MAME_DIR .. "src/mame/suna/*.h",
}

createMAMEProjects(_target, _subtarget, "sunelect")
files {
	MAME_DIR .. "src/mame/sunelect/*.cpp",
	MAME_DIR .. "src/mame/sunelect/*.h",
}

createMAMEProjects(_target, _subtarget, "svi")
files {
	MAME_DIR .. "src/mame/svi/*.cpp",
}

createMAMEProjects(_target, _subtarget, "svision")
files {
	MAME_DIR .. "src/mame/svision/*.cpp",
	MAME_DIR .. "src/mame/svision/*.h",
}

createMAMEProjects(_target, _subtarget, "swtpc")
files {
	MAME_DIR .. "src/mame/swtpc/*.cpp",
	MAME_DIR .. "src/mame/swtpc/*.h",
}

createMAMEProjects(_target, _subtarget, "synertek")
files {
	MAME_DIR .. "src/mame/synertek/*.cpp",
}

createMAMEProjects(_target, _subtarget, "ta")
files {
	MAME_DIR .. "src/mame/ta/*.cpp",
}

createMAMEProjects(_target, _subtarget, "tab")
files {
	MAME_DIR .. "src/mame/tab/*.cpp",
	MAME_DIR .. "src/mame/tab/*.h",
}

createMAMEProjects(_target, _subtarget, "taito")
files {
	MAME_DIR .. "src/mame/taito/*.cpp",
	MAME_DIR .. "src/mame/taito/*.h",
}

createMAMEProjects(_target, _subtarget, "tandberg")
files {
	MAME_DIR .. "src/mame/tandberg/*.cpp",
	MAME_DIR .. "src/mame/tandberg/*.h",
}

createMAMEProjects(_target, _subtarget, "tangerin")
files {
	MAME_DIR .. "src/mame/tangerin/*.cpp",
	MAME_DIR .. "src/mame/tangerin/*.h",
}

createMAMEProjects(_target, _subtarget, "tatsumi")
files {
	MAME_DIR .. "src/mame/tatsumi/*.cpp",
	MAME_DIR .. "src/mame/tatsumi/*.h",
}

createMAMEProjects(_target, _subtarget, "tatung")
files {
	MAME_DIR .. "src/mame/tatung/*.cpp",
}

createMAMEProjects(_target, _subtarget, "tch")
files {
	MAME_DIR .. "src/mame/tch/*.cpp",
	MAME_DIR .. "src/mame/tch/*.h",
}

createMAMEProjects(_target, _subtarget, "teamconc")
files {
	MAME_DIR .. "src/mame/teamconc/*.cpp",
}

createMAMEProjects(_target, _subtarget, "tecfri")
files {
	MAME_DIR .. "src/mame/tecfri/*.cpp",
	MAME_DIR .. "src/mame/tecfri/*.h",
}

createMAMEProjects(_target, _subtarget, "technos")
files {
	MAME_DIR .. "src/mame/technos/*.cpp",
	MAME_DIR .. "src/mame/technos/*.h",
}

createMAMEProjects(_target, _subtarget, "tectoy")
files {
	MAME_DIR .. "src/mame/tectoy/*.cpp",
}

createMAMEProjects(_target, _subtarget, "tehkan")
files {
	MAME_DIR .. "src/mame/tehkan/*.cpp",
	MAME_DIR .. "src/mame/tehkan/*.h",
}

createMAMEProjects(_target, _subtarget, "tektroni")
files {
	MAME_DIR .. "src/mame/tektroni/*.cpp",
	MAME_DIR .. "src/mame/tektroni/*.h",
}

createMAMEProjects(_target, _subtarget, "telenova")
files {
	MAME_DIR .. "src/mame/telenova/*.cpp",
	MAME_DIR .. "src/mame/telenova/*.h",
}

createMAMEProjects(_target, _subtarget, "telercas")
files {
	MAME_DIR .. "src/mame/telercas/*.cpp",
	MAME_DIR .. "src/mame/telercas/*.h",
}

createMAMEProjects(_target, _subtarget, "televideo")
files {
	MAME_DIR .. "src/mame/televideo/*.cpp",
	MAME_DIR .. "src/mame/televideo/*.h",
}

createMAMEProjects(_target, _subtarget, "tesla")
files {
	MAME_DIR .. "src/mame/tesla/*.cpp",
	MAME_DIR .. "src/mame/tesla/*.h",
}

createMAMEProjects(_target, _subtarget, "thepit")
files {
	MAME_DIR .. "src/mame/thepit/*.cpp",
	MAME_DIR .. "src/mame/thepit/*.h",
}

createMAMEProjects(_target, _subtarget, "toaplan")
files {
	MAME_DIR .. "src/mame/toaplan/*.cpp",
	MAME_DIR .. "src/mame/toaplan/*.h",
}

createMAMEProjects(_target, _subtarget, "thomson")
files {
	MAME_DIR .. "src/mame/thomson/*.cpp",
	MAME_DIR .. "src/mame/thomson/*.h",
}

createMAMEProjects(_target, _subtarget, "ti")
files {
	MAME_DIR .. "src/mame/ti/*.cpp",
	MAME_DIR .. "src/mame/ti/*.h",
}

createMAMEProjects(_target, _subtarget, "tiger")
files {
	MAME_DIR .. "src/mame/tiger/*.cpp",
	MAME_DIR .. "src/mame/tiger/*.h",
}

createMAMEProjects(_target, _subtarget, "tigertel")
files {
	MAME_DIR .. "src/mame/tigertel/*.cpp",
	MAME_DIR .. "src/mame/tigertel/*.h",
}

createMAMEProjects(_target, _subtarget, "tiki")
files {
	MAME_DIR .. "src/mame/tiki/*.cpp",
	MAME_DIR .. "src/mame/tiki/*.h",
}

createMAMEProjects(_target, _subtarget, "tomy")
files {
	MAME_DIR .. "src/mame/tomy/*.cpp",
}

createMAMEProjects(_target, _subtarget, "toshiba")
files {
	MAME_DIR .. "src/mame/toshiba/*.h",
	MAME_DIR .. "src/mame/toshiba/*.cpp",
}

createMAMEProjects(_target, _subtarget, "trainer")
files {
	MAME_DIR .. "src/mame/trainer/*.cpp",
}

createMAMEProjects(_target, _subtarget, "trs")
files {
	MAME_DIR .. "src/mame/trs/*.cpp",
	MAME_DIR .. "src/mame/trs/*.h",
}

createMAMEProjects(_target, _subtarget, "tvgames")
files {
	MAME_DIR .. "src/mame/tvgames/*.cpp",
	MAME_DIR .. "src/mame/tvgames/*.h",
}

createMAMEProjects(_target, _subtarget, "ultimachine")
files {
	MAME_DIR .. "src/mame/ultimachine/*.cpp",
}

createMAMEProjects(_target, _subtarget, "ultratec")
files {
	MAME_DIR .. "src/mame/ultratec/*.cpp",
}

createMAMEProjects(_target, _subtarget, "unicard")
files {
	MAME_DIR .. "src/mame/unicard/*.cpp",
}

createMAMEProjects(_target, _subtarget, "unico")
files {
	MAME_DIR .. "src/mame/unico/*.cpp",
	MAME_DIR .. "src/mame/unico/*.h",
}

createMAMEProjects(_target, _subtarget, "unisonic")
files {
	MAME_DIR .. "src/mame/unisonic/*.cpp",
	MAME_DIR .. "src/mame/unisonic/*.h",
}

createMAMEProjects(_target, _subtarget, "unisys")
files {
	MAME_DIR .. "src/mame/unisys/*.cpp",
}

createMAMEProjects(_target, _subtarget, "univers")
files {
	MAME_DIR .. "src/mame/univers/*.cpp",
	MAME_DIR .. "src/mame/univers/*.h",
}

createMAMEProjects(_target, _subtarget, "upl")
files {
	MAME_DIR .. "src/mame/upl/*.cpp",
	MAME_DIR .. "src/mame/upl/*.h",
}

createMAMEProjects(_target, _subtarget, "usp")
files {
	MAME_DIR .. "src/mame/usp/*.cpp",
	MAME_DIR .. "src/mame/usp/*.h",
}

createMAMEProjects(_target, _subtarget, "valadon")
files {
	MAME_DIR .. "src/mame/valadon/*.cpp",
	MAME_DIR .. "src/mame/valadon/*.h",
}

createMAMEProjects(_target, _subtarget, "venture")
files {
	MAME_DIR .. "src/mame/venture/*.cpp",
	MAME_DIR .. "src/mame/venture/*.h",
}

createMAMEProjects(_target, _subtarget, "vsystem")
files {
	MAME_DIR .. "src/mame/vsystem/*.cpp",
	MAME_DIR .. "src/mame/vsystem/*.h",
}

createMAMEProjects(_target, _subtarget, "verifone")
files {
	MAME_DIR .. "src/mame/verifone/*.cpp",
	MAME_DIR .. "src/mame/verifone/*.h"
}

createMAMEProjects(_target, _subtarget, "vidbrain")
files {
	MAME_DIR .. "src/mame/vidbrain/*.cpp",
	MAME_DIR .. "src/mame/vidbrain/*.h",
}

createMAMEProjects(_target, _subtarget, "videoton")
files {
	MAME_DIR .. "src/mame/videoton/*.cpp",
	MAME_DIR .. "src/mame/videoton/*.h",
}

createMAMEProjects(_target, _subtarget, "visual")
files {
	MAME_DIR .. "src/mame/visual/*.cpp",
	MAME_DIR .. "src/mame/visual/*.h",
}

createMAMEProjects(_target, _subtarget, "votrax")
files {
	MAME_DIR .. "src/mame/votrax/*.cpp",
}

createMAMEProjects(_target, _subtarget, "vtech")
files {
	MAME_DIR .. "src/mame/vtech/*.cpp",
	MAME_DIR .. "src/mame/vtech/*.h",
}

createMAMEProjects(_target, _subtarget, "wang")
files {
	MAME_DIR .. "src/mame/wang/*.cpp",
	MAME_DIR .. "src/mame/wang/*.h",
}

createMAMEProjects(_target, _subtarget, "westinghouse")
files {
	MAME_DIR .. "src/mame/westinghouse/*.cpp",
}

createMAMEProjects(_target, _subtarget, "wavemate")
files {
	MAME_DIR .. "src/mame/wavemate/*.cpp",
	MAME_DIR .. "src/mame/wavemate/*.h",
}

createMAMEProjects(_target, _subtarget, "wing")
files {
	MAME_DIR .. "src/mame/wing/*.cpp",
}

createMAMEProjects(_target, _subtarget, "wyse")
files {
	MAME_DIR .. "src/mame/wyse/*.cpp",
	MAME_DIR .. "src/mame/wyse/*.h",
}

createMAMEProjects(_target, _subtarget, "xerox")
files {
	MAME_DIR .. "src/mame/xerox/*.cpp",
	MAME_DIR .. "src/mame/xerox/*.h",
}

createMAMEProjects(_target, _subtarget, "xussrpc")
files {
	MAME_DIR .. "src/mame/xussrpc/*.cpp",
	MAME_DIR .. "src/mame/xussrpc/*.h",
}

createMAMEProjects(_target, _subtarget, "yamaha")
files {
	MAME_DIR .. "src/mame/yamaha/*.cpp",
	MAME_DIR .. "src/mame/yamaha/*.h"
}

createMAMEProjects(_target, _subtarget, "yunsung")
files {
	MAME_DIR .. "src/mame/yunsung/*.cpp",
}

createMAMEProjects(_target, _subtarget, "zaccaria")
files {
	MAME_DIR .. "src/mame/zaccaria/*.cpp",
	MAME_DIR .. "src/mame/zaccaria/*.h",
}

createMAMEProjects(_target, _subtarget, "zenith")
files {
	MAME_DIR .. "src/mame/zenith/*.cpp",
}

createMAMEProjects(_target, _subtarget, "zpa")
files {
	MAME_DIR .. "src/mame/zpa/*.cpp",
}

createMAMEProjects(_target, _subtarget, "zvt")
files {
	MAME_DIR .. "src/mame/zvt/*.cpp",
	MAME_DIR .. "src/mame/zvt/*.h",
}


--------------------------------------------------
-- pinball drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "pinball")
files {
	MAME_DIR .. "src/mame/pinball/*.cpp",
	MAME_DIR .. "src/mame/pinball/*.h",
}


--------------------------------------------------
-- remaining drivers
--------------------------------------------------

createMAMEProjects(_target, _subtarget, "misc")
files {
	MAME_DIR .. "src/mame/misc/*.cpp",
	MAME_DIR .. "src/mame/misc/*.h",
}

createMAMEProjects(_target, _subtarget, "misc_chess")
files {
	MAME_DIR .. "src/mame/misc_chess/*.cpp",
}

createMAMEProjects(_target, _subtarget, "misc_handheld")
files {
	MAME_DIR .. "src/mame/misc_handheld/*.cpp",
	MAME_DIR .. "src/mame/misc_handheld/*.h",
}

createMAMEProjects(_target, _subtarget, "skeleton")
files {
	MAME_DIR .. "src/mame/skeleton/*.cpp",
	MAME_DIR .. "src/mame/skeleton/*.h",
}

createMAMEProjects(_target, _subtarget, "virtual")
files {
	MAME_DIR .. "src/mame/virtual/*.cpp",
}

end
