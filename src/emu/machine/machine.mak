###########################################################################
#
#   machine.mak
#
#   Rules for building machine cores
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


MACHINESRC = $(EMUSRC)/machine
MACHINEOBJ = $(EMUOBJ)/machine


#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter NCR53C7XX,$(MACHINES)),)
MACHINES += NSCSI
MACHINEOBJS += $(MACHINEOBJ)/53c7xx.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter LSI53C810,$(MACHINES)),)
MACHINES += SCSI
MACHINEOBJS += $(MACHINEOBJ)/53c810.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter 6522VIA,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/6522via.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TPI6525,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/6525tpi.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter 6526CIA,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/6526cia.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter RIOT6532,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/6532riot.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter 6821PIA,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/6821pia.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter 6840PTM,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/6840ptm.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter ACIA6850,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/6850acia.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter 68681,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/68681.o 
MACHINEOBJS += $(MACHINEOBJ)/n68681.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter 7200FIFO,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/7200fifo.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TTL74123,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/74123.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TTL74145,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/74145.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TTL74148,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/74148.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TTL74153,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/74153.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TTL74181,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/74181.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TTL7474,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/7474.o  
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter KBDC8042,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/8042kbdc.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter I8257,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/8257dma.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter AAKARTDEV,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/aakart.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter ADC0808,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/adc0808.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter ADC083X,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/adc083x.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter ADC1038,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/adc1038.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter ADC1213X,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/adc1213x.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter AM53CF96,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/am53cf96.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter AM9517A,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/am9517a.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter AMIGAFDC,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/amigafdc.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter AT28C16,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/at28c16.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter AT29040,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/at29040a.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter AT45DBXX,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/at45dbxx.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter ATAFLASH,$(MACHINES)),)
MACHINES += IDE
MACHINES += PCCARD
MACHINEOBJS += $(MACHINEOBJ)/ataflash.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter AY31015,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/ay31015.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter BANKDEV,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/bankdev.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter CDP1852,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/cdp1852.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter CDP1871,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/cdp1871.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter COM8116,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/com8116.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter CR589,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/cr589.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter CTRONICS,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/ctronics.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter DS1302,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/ds1302.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter DS2401,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/ds2401.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter DS2404,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/ds2404.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter DS75160A,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/ds75160a.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter DS75161A,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/ds75161a.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter E0516,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/e0516.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter EEPROMDEV,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/eeprom.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter ER2055,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/er2055.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter ER59256,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/er59256.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter F3853,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/f3853.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter I2CMEM,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/i2cmem.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter I8155,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/i8155.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter I8212,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/i8212.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter I8214,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/i8214.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter I8243,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/i8243.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter I8251,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/i8251.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter I8279,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/i8279.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter I8355,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/i8355.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter IDE,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/idectrl.o
MACHINEOBJS += $(MACHINEOBJ)/idehd.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter IM6402,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/im6402.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter INS8154,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/ins8154.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter INS8250,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/ins8250.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter INTELFLASH,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/intelfsh.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter JVS,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/jvsdev.o
MACHINEOBJS += $(MACHINEOBJ)/jvshost.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter K033906,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/k033906.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter K053252,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/k053252.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter K056230,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/k056230.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter LATCH8,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/latch8.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter LC89510,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/lc89510.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter LDPR8210,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/ldpr8210.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter LDSTUB,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/ldstub.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter LDV1000,$(MACHINES)),)
MACHINES += Z80CTC
MACHINES += I8255
MACHINEOBJS += $(MACHINEOBJ)/ldv1000.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter LDVP931,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/ldvp931.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter LINFLASH,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/linflash.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter M6M80011AP,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/m6m80011ap.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MATSUCD,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/matsucd.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MB14241,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mb14241.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MB3773,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mb3773.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MB87078,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mb87078.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MB89371,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mb89371.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MC146818,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mc146818.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MC2661,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mc2661.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MC6843,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mc6843.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MC6846,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mc6846.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MC6852,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mc6852.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MC6854,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mc6854.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MC68901,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mc68901.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MCCS1850,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mccs1850.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MCF5206E,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mcf5206e.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MICROTOUCH,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/microtch.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MM58274C,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mm58274c.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MM74C922,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mm74c922.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MOS6526,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mos6526.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MOS6529,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mos6529.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MIOT6530,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mos6530.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MOS6551,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/mos6551.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MSM5832,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/msm5832.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MSM58321,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/msm58321.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter MSM6242,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/msm6242.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter NCR539x,$(MACHINES)),)
MACHINES += SCSI
MACHINEOBJS += $(MACHINEOBJ)/ncr539x.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter NMC9306,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/nmc9306.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter NSCSI,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/nscsi_bus.o
MACHINEOBJS += $(MACHINEOBJ)/nscsi_cd.o
MACHINEOBJS += $(MACHINEOBJ)/nscsi_hd.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter PCF8593,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/pcf8593.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter PCI,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/pci.o   
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter PCKEYBRD,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/pckeybrd.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter PD4990A_OLD,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/pd4990a.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter PIC8259,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/pic8259.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter PIT8253,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/pit8253.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter PLA,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/pla.o   
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter RF5C296,$(MACHINES)),)
MACHINES += PCCARD
MACHINEOBJS += $(MACHINEOBJ)/rf5c296.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter ROC10937,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/roc10937.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter RP5C01,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/rp5c01.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter RP5C15,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/rp5c15.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter RP5H01,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/rp5h01.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter RTC4543,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/rtc4543.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter RTC65271,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/rtc65271.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter RTC9701,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/rtc9701.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter S3520CF,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/s3520cf.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter S3C2400,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/s3c2400.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter S3C2410,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/s3c2410.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter S3C2440,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/s3c2440.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter SATURN,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/saturn.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter SCSI,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/scsibus.o
MACHINEOBJS += $(MACHINEOBJ)/scsicb.o
MACHINEOBJS += $(MACHINEOBJ)/scsicd.o
MACHINEOBJS += $(MACHINEOBJ)/scsidev.o
MACHINEOBJS += $(MACHINEOBJ)/scsihd.o
MACHINEOBJS += $(MACHINEOBJ)/scsihle.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter SCUDSP,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/scudsp.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter SEIBU_COP,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/seibu_cop.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter SERFLASH,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/serflash.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter SMC91C9X,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/smc91c9x.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter SMPC,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/smpc.o  
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter STVCD,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/stvcd.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TC0091LVC,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/tc009xlvc.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TIMEKPR,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/timekpr.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TMP68301,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/tmp68301.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TMS6100,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/tms6100.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TMS9901,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/tms9901.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter TMS9902,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/tms9902.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter UPD1990A,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/upd1990a.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter UPD4701,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/upd4701.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter UPD7002,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/upd7002.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter UPD765,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/upd765.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter V3021,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/v3021.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter WD_FDC,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/wd_fdc.o
MACHINEOBJS += $(MACHINEOBJ)/fdc_pll.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter WD11C00_17,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/wd11c00_17.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter WD17XX,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/wd17xx.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter WD2010,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/wd2010.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter WD33C93,$(MACHINES)),)
MACHINES += SCSI
MACHINEOBJS += $(MACHINEOBJ)/wd33c93.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter X2212,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/x2212.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter X76F041,$(MACHINES)),)
MACHINES += SECFLASH
MACHINEOBJS += $(MACHINEOBJ)/x76f041.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter X76F100,$(MACHINES)),)
MACHINES += SECFLASH
MACHINEOBJS += $(MACHINEOBJ)/x76f100.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter Z80CTC,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/z80ctc.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter Z80DART,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/z80dart.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter Z80DMA,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/z80dma.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter Z80PIO,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/z80pio.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter Z80SIO,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/z80sio.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter Z80STI,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/z80sti.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter Z8536,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/z8536.o 
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter SECFLASH,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/secflash.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter PCCARD,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/pccard.o
endif

#-------------------------------------------------
# 
#-------------------------------------------------

ifneq ($(filter I8255,$(MACHINES)),)
MACHINEOBJS += $(MACHINEOBJ)/i8255.o 
endif

$(MACHINEOBJ)/s3c2400.o:    $(MACHINESRC)/s3c24xx.c
$(MACHINEOBJ)/s3c2410.o:    $(MACHINESRC)/s3c24xx.c
$(MACHINEOBJ)/s3c2440.o:    $(MACHINESRC)/s3c24xx.c

