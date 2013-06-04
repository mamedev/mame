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
# Core machine types
#-------------------------------------------------

MACHINEOBJS +=  $(MACHINEOBJ)/53c7xx.o      \
				$(MACHINEOBJ)/53c810.o      \
				$(MACHINEOBJ)/6522via.o     \
				$(MACHINEOBJ)/6525tpi.o     \
				$(MACHINEOBJ)/6526cia.o     \
				$(MACHINEOBJ)/6532riot.o    \
				$(MACHINEOBJ)/6821pia.o     \
				$(MACHINEOBJ)/6840ptm.o     \
				$(MACHINEOBJ)/6850acia.o    \
				$(MACHINEOBJ)/68681.o       \
				$(MACHINEOBJ)/7200fifo.o    \
				$(MACHINEOBJ)/74123.o       \
				$(MACHINEOBJ)/74145.o       \
				$(MACHINEOBJ)/74148.o       \
				$(MACHINEOBJ)/74153.o       \
				$(MACHINEOBJ)/74181.o       \
				$(MACHINEOBJ)/7474.o        \
				$(MACHINEOBJ)/8042kbdc.o    \
				$(MACHINEOBJ)/8257dma.o     \
				$(MACHINEOBJ)/aakart.o      \
				$(MACHINEOBJ)/adc0808.o     \
				$(MACHINEOBJ)/adc083x.o     \
				$(MACHINEOBJ)/adc1038.o     \
				$(MACHINEOBJ)/adc1213x.o    \
				$(MACHINEOBJ)/am53cf96.o    \
				$(MACHINEOBJ)/am9517a.o     \
				$(MACHINEOBJ)/amigafdc.o    \
				$(MACHINEOBJ)/at28c16.o     \
				$(MACHINEOBJ)/at29040a.o    \
				$(MACHINEOBJ)/at45dbxx.o    \
				$(MACHINEOBJ)/ataflash.o    \
				$(MACHINEOBJ)/ay31015.o     \
				$(MACHINEOBJ)/bankdev.o     \
				$(MACHINEOBJ)/cdp1852.o     \
				$(MACHINEOBJ)/cdp1871.o     \
				$(MACHINEOBJ)/com8116.o     \
				$(MACHINEOBJ)/cr589.o       \
				$(MACHINEOBJ)/ctronics.o    \
				$(MACHINEOBJ)/ds1302.o      \
				$(MACHINEOBJ)/ds2401.o      \
				$(MACHINEOBJ)/ds2404.o      \
				$(MACHINEOBJ)/ds75160a.o    \
				$(MACHINEOBJ)/ds75161a.o    \
				$(MACHINEOBJ)/e0516.o       \
				$(MACHINEOBJ)/eeprom.o      \
				$(MACHINEOBJ)/er2055.o      \
				$(MACHINEOBJ)/er59256.o     \
				$(MACHINEOBJ)/f3853.o       \
				$(MACHINEOBJ)/fdc_pll.o     \
				$(MACHINEOBJ)/i2cmem.o      \
				$(MACHINEOBJ)/i8155.o       \
				$(MACHINEOBJ)/i8212.o       \
				$(MACHINEOBJ)/i8214.o       \
				$(MACHINEOBJ)/i8243.o       \
				$(MACHINEOBJ)/i8251.o       \
				$(MACHINEOBJ)/i8255.o       \
				$(MACHINEOBJ)/i8279.o       \
				$(MACHINEOBJ)/i8355.o       \
				$(MACHINEOBJ)/idectrl.o     \
				$(MACHINEOBJ)/im6402.o      \
				$(MACHINEOBJ)/ins8154.o     \
				$(MACHINEOBJ)/ins8250.o     \
				$(MACHINEOBJ)/intelfsh.o    \
				$(MACHINEOBJ)/jvsdev.o      \
				$(MACHINEOBJ)/jvshost.o     \
				$(MACHINEOBJ)/k033906.o     \
				$(MACHINEOBJ)/k053252.o     \
				$(MACHINEOBJ)/k056230.o     \
				$(MACHINEOBJ)/laserdsc.o    \
				$(MACHINEOBJ)/latch8.o      \
				$(MACHINEOBJ)/lc89510.o     \
				$(MACHINEOBJ)/ldpr8210.o    \
				$(MACHINEOBJ)/ldstub.o      \
				$(MACHINEOBJ)/ldv1000.o     \
				$(MACHINEOBJ)/ldvp931.o     \
				$(MACHINEOBJ)/linflash.o    \
				$(MACHINEOBJ)/m6m80011ap.o  \
				$(MACHINEOBJ)/matsucd.o     \
				$(MACHINEOBJ)/mb14241.o     \
				$(MACHINEOBJ)/mb3773.o      \
				$(MACHINEOBJ)/mb87078.o     \
				$(MACHINEOBJ)/mb89371.o     \
				$(MACHINEOBJ)/mc146818.o    \
				$(MACHINEOBJ)/mc2661.o      \
				$(MACHINEOBJ)/mc6843.o      \
				$(MACHINEOBJ)/mc6846.o      \
				$(MACHINEOBJ)/mc6852.o      \
				$(MACHINEOBJ)/mc6854.o      \
				$(MACHINEOBJ)/mc68901.o     \
				$(MACHINEOBJ)/mccs1850.o    \
				$(MACHINEOBJ)/mcf5206e.o    \
				$(MACHINEOBJ)/microtch.o    \
				$(MACHINEOBJ)/mm58274c.o    \
				$(MACHINEOBJ)/mm74c922.o    \
				$(MACHINEOBJ)/mos6526.o     \
				$(MACHINEOBJ)/mos6529.o     \
				$(MACHINEOBJ)/mos6530.o     \
				$(MACHINEOBJ)/mos6551.o     \
				$(MACHINEOBJ)/msm5832.o     \
				$(MACHINEOBJ)/msm58321.o    \
				$(MACHINEOBJ)/msm6242.o     \
				$(MACHINEOBJ)/n68681.o      \
				$(MACHINEOBJ)/ncr539x.o     \
				$(MACHINEOBJ)/net_lib.o     \
				$(MACHINEOBJ)/netlist.o     \
				$(MACHINEOBJ)/nmc9306.o     \
				$(MACHINEOBJ)/nscsi_bus.o   \
				$(MACHINEOBJ)/nscsi_cd.o    \
				$(MACHINEOBJ)/nscsi_hd.o    \
				$(MACHINEOBJ)/pc16552d.o    \
				$(MACHINEOBJ)/pccard.o      \
				$(MACHINEOBJ)/pcf8593.o     \
				$(MACHINEOBJ)/pci.o         \
				$(MACHINEOBJ)/pckeybrd.o    \
				$(MACHINEOBJ)/pd4990a.o     \
				$(MACHINEOBJ)/pic8259.o     \
				$(MACHINEOBJ)/pit8253.o     \
				$(MACHINEOBJ)/pla.o         \
				$(MACHINEOBJ)/rf5c296.o     \
				$(MACHINEOBJ)/roc10937.o    \
				$(MACHINEOBJ)/rp5c01.o      \
				$(MACHINEOBJ)/rp5c15.o      \
				$(MACHINEOBJ)/rp5h01.o      \
				$(MACHINEOBJ)/rtc4543.o     \
				$(MACHINEOBJ)/rtc65271.o    \
				$(MACHINEOBJ)/rtc9701.o     \
				$(MACHINEOBJ)/s3520cf.o     \
				$(MACHINEOBJ)/s3c2400.o     \
				$(MACHINEOBJ)/s3c2410.o     \
				$(MACHINEOBJ)/s3c2440.o     \
				$(MACHINEOBJ)/saturn.o      \
				$(MACHINEOBJ)/scsibus.o     \
				$(MACHINEOBJ)/scsicb.o      \
				$(MACHINEOBJ)/scsicd.o      \
				$(MACHINEOBJ)/scsidev.o     \
				$(MACHINEOBJ)/scsihd.o      \
				$(MACHINEOBJ)/scsihle.o     \
				$(MACHINEOBJ)/scudsp.o      \
				$(MACHINEOBJ)/secflash.o    \
				$(MACHINEOBJ)/seibu_cop.o   \
				$(MACHINEOBJ)/smc91c9x.o    \
				$(MACHINEOBJ)/smpc.o        \
				$(MACHINEOBJ)/stvcd.o       \
				$(MACHINEOBJ)/tc009xlvc.o   \
				$(MACHINEOBJ)/timekpr.o     \
				$(MACHINEOBJ)/tmp68301.o    \
				$(MACHINEOBJ)/tms6100.o     \
				$(MACHINEOBJ)/tms9901.o     \
				$(MACHINEOBJ)/tms9902.o     \
				$(MACHINEOBJ)/upd1990a.o    \
				$(MACHINEOBJ)/upd4701.o     \
				$(MACHINEOBJ)/upd7002.o     \
				$(MACHINEOBJ)/upd765.o      \
				$(MACHINEOBJ)/v3021.o       \
				$(MACHINEOBJ)/wd_fdc.o      \
				$(MACHINEOBJ)/wd11c00_17.o  \
				$(MACHINEOBJ)/wd17xx.o      \
				$(MACHINEOBJ)/wd2010.o      \
				$(MACHINEOBJ)/wd33c93.o     \
				$(MACHINEOBJ)/x2212.o       \
				$(MACHINEOBJ)/x76f041.o     \
				$(MACHINEOBJ)/x76f100.o     \
				$(MACHINEOBJ)/z80ctc.o      \
				$(MACHINEOBJ)/z80dart.o     \
				$(MACHINEOBJ)/z80dma.o      \
				$(MACHINEOBJ)/z80pio.o      \
				$(MACHINEOBJ)/z80sio.o      \
				$(MACHINEOBJ)/z80sti.o      \
				$(MACHINEOBJ)/z8536.o       \

$(MACHINEOBJ)/s3c2400.o:    $(MACHINESRC)/s3c24xx.c
$(MACHINEOBJ)/s3c2410.o:    $(MACHINESRC)/s3c24xx.c
$(MACHINEOBJ)/s3c2440.o:    $(MACHINESRC)/s3c24xx.c

