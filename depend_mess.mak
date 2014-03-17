
$(OBJ)/mess/audio/alesis.o : \
	src/emu/machine/nvram.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/includes/alesis.h \
	src/lib/formats/cassimg.h \
	src/mess/audio/alesis.c \
	src/emu/cpu/mcs51/mcs51.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/video/hd44780.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/audio/arcadia.o : \
	src/emu/cpu/s2650/s2650.h \
	src/emu/imagedev/cartslot.h \
	src/mess/audio/arcadia.c \
	src/mess/includes/arcadia.h \
	src/mess/audio/arcadia.h \

$(OBJ)/mess/audio/channelf.o : \
	src/mess/audio/channelf.h \
	src/mess/audio/channelf.c \

$(OBJ)/mess/audio/dai.o : \
	src/emu/machine/pit8253.h \
	src/emu/machine/tms5501.h \
	src/mess/includes/dai.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mess/audio/dai.c \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/audio/dave.o : \
	src/mess/audio/dave.h \
	src/mess/audio/dave.c \

$(OBJ)/mess/audio/gb.o : \
	src/mess/audio/gb.h \
	src/mess/audio/gb.c \

$(OBJ)/mess/audio/lynx.o : \
	src/mess/audio/lynx.h \
	src/mess/audio/lynx.c \

$(OBJ)/mess/audio/mac.o : \
	src/mess/machine/egret.h \
	src/mess/machine/macpds.h \
	src/emu/sound/asc.h \
	src/mess/machine/macrtc.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/mess/includes/mac.h \
	src/emu/machine/ram.h \
	src/mess/machine/cuda.h \
	src/lib/softfloat/mamesf.h \
	src/mess/machine/ncr5380.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/sound/awacs.h \
	src/mess/machine/mackbd.h \
	src/emu/bus/nubus/nubus.h \
	src/mess/machine/8530scc.h \
	src/emu/machine/ncr539x.h \
	src/emu/machine/6522via.h \
	src/mess/audio/mac.c \
	src/emu/machine/scsihle.h \

$(OBJ)/mess/audio/mea8000.o : \
	src/mess/audio/mea8000.h \
	src/mess/audio/mea8000.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/audio/socrates.o : \
	src/mess/audio/socrates.c \
	src/mess/audio/socrates.h \

$(OBJ)/mess/audio/special.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/mess/includes/special.h \
	src/mess/audio/special.c \
	src/lib/formats/rk_cas.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/lib/formats/smx_dsk.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/audio/svision.o : \
	src/mess/includes/svision.h \
	src/mess/audio/svision.c \

$(OBJ)/mess/audio/tvc.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/tvc/hbf.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/tvc_dsk.h \
	src/mess/audio/tvc.c \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/mess/includes/tvc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/formats/wd177x_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/tvc/tvc.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/formats/tvc_cas.h \

$(OBJ)/mess/audio/upd1771.o : \
	src/mess/audio/upd1771.h \
	src/mess/audio/upd1771.c \

$(OBJ)/mess/audio/vboy.o : \
	src/mess/audio/vboy.c \
	src/mess/audio/vboy.h \

$(OBJ)/mess/audio/vc4000.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/includes/vc4000.h \
	src/emu/cpu/s2650/s2650.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/cartslot.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/audio/vc4000.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/audio/wswan.o : \
	src/emu/machine/nvram.h \
	src/mess/includes/wswan.h \
	src/emu/imagedev/cartslot.h \
	src/mess/audio/wswan.c \
	src/emu/cpu/v30mz/v30mz.h \

$(OBJ)/mess/drivers/4004clk.o : \
	src/mess/drivers/4004clk.c \
	src/emu/cpu/i4004/i4004.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/68ksbc.o : \
	src/emu/bus/rs232/rs232.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/emu/machine/6850acia.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/clock.h \
	src/mess/drivers/68ksbc.c \

$(OBJ)/mess/drivers/a2600.o : \
	src/lib/formats/a26_cas.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/sound/tiaintf.h \
	src/mess/drivers/a2600.c \
	src/emu/imagedev/cartslot.h \
	src/mame/video/tia.h \
	src/emu/bus/vcs/ctrl.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6532riot.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/a310.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/aakart.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/mess/drivers/a310.c \
	src/emu/cpu/arm/arm.h \
	src/mame/includes/archimds.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \
	src/emu/machine/i2cmem.h \

$(OBJ)/mess/drivers/a5105.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/a5105_dsk.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/lib/formats/upd765_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/mess/drivers/a5105.c \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/upd765.h \
	src/emu/video/upd7220.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/a51xx.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/a51xx.c \

$(OBJ)/mess/drivers/a6809.o : \
	src/emu/video/saa5050.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/mess/drivers/a6809.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/emu/machine/6522via.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/a7000.o : \
	src/mess/drivers/a7000.c \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7core.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/mess/drivers/a7150.o : \
	src/mess/drivers/a7150.c \
	src/emu/cpu/i86/i86.h \

$(OBJ)/mess/drivers/a7800.o : \
	src/mess/drivers/a7800.c \
	src/emu/machine/rescap.h \
	src/mess/includes/a7800.h \
	src/emu/sound/tiaintf.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/tiasound.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6532riot.h \
	src/emu/sound/pokey.h \

$(OBJ)/mess/drivers/abc1600.o : \
	src/mess/video/abc1600.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/z8536.h \
	src/mess/drivers/abc1600.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/emu/machine/e0516.h \
	src/lib/softfloat/milieu.h \
	src/mess/includes/abc1600.h \
	src/emu/machine/z80dma.h \
	src/lib/softfloat/softfloat.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/machine/8530scc.h \
	src/emu/machine/nmc9306.h \
	src/lib/util/opresolv.h \
	src/emu/bus/abckb/abckb.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/bus/abcbus/abcbus.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/machine/abc1600mac.h \

$(OBJ)/mess/drivers/abc80.o : \
	src/lib/formats/flopimg.h \
	src/mess/machine/abc80kb.h \
	src/emu/machine/rescap.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/includes/abc80.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/machine/keyboard.h \
	src/emu/imagedev/printer.h \
	src/emu/sound/sn76477.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/bus/abcbus/abcbus.h \
	src/lib/formats/ioprocs.h \
	src/mess/drivers/abc80.c \

$(OBJ)/mess/drivers/abc80x.o : \
	src/emu/video/saa5050.h \
	src/emu/machine/rescap.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/sound/disc_dev.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/abc80x.c \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/bus/abckb/abc800kb.h \
	src/emu/machine/e0516.h \
	src/emu/sound/discrete.h \
	src/emu/machine/z80ctc.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/wavwrite.h \
	src/emu/video/mc6845.h \
	src/mess/includes/abc80x.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/abckb/abckb.h \
	src/emu/machine/z80dart.h \
	src/emu/bus/abcbus/abcbus.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/drivers/ac1.o : \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/ac1.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/ac1.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/ace.o : \
	src/emu/machine/buffer.h \
	src/emu/sound/sp0256.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/lib/formats/ace_tap.h \
	src/emu/imagedev/snapquik.h \
	src/mess/drivers/ace.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/ace.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/acrnsys1.o : \
	src/mess/drivers/acrnsys1.c \
	src/lib/util/corealloc.h \
	src/emu/machine/ins8154.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/74145.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/adam.o : \
	src/emu/bus/coleco/ctrl.h \
	src/mess/includes/adam.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/video/tms9928a.h \
	src/emu/bus/adam/exp.h \
	src/emu/bus/coleco/exp.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/sn76496.h \
	src/emu/imagedev/cartslot.h \
	src/emu/bus/adamnet/adamnet.h \
	src/mess/drivers/adam.c \
	src/mess/machine/coleco.h \

$(OBJ)/mess/drivers/advision.o : \
	src/mess/drivers/advision.c \
	src/emu/cpu/cop400/cop400.h \
	src/mess/includes/advision.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/aim65.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/video/dl1416.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/aim65.c \
	src/mess/includes/aim65.h \
	src/emu/machine/6821pia.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6522via.h \
	src/emu/machine/6532riot.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/aim65_40.o : \
	src/emu/machine/mos6551.h \
	src/mess/drivers/aim65_40.c \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6522via.h \
	src/emu/machine/clock.h \

$(OBJ)/mess/drivers/alesis.o : \
	src/mess/drivers/alesis.c \
	src/emu/machine/nvram.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/includes/alesis.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/video/hd44780.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/alphasma.o : \
	src/mess/drivers/alphasma.c \
	src/emu/machine/nvram.h \
	src/emu/cpu/mc68hc11/mc68hc11.h \
	src/emu/video/hd44780.h \

$(OBJ)/mess/drivers/alphatro.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/alphatro.c \
	src/emu/cpu/z80/z80.h \
	src/emu/video/mc6845.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/altair.o : \
	src/emu/bus/rs232/rs232.h \
	src/mess/drivers/altair.c \
	src/emu/machine/6850acia.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/imagedev/snapquik.h \
	src/emu/machine/clock.h \

$(OBJ)/mess/drivers/altos5.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/z80pio.h \
	src/mess/drivers/altos5.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/z80dma.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/amico2k.o : \
	src/mess/drivers/amico2k.c \
	src/emu/machine/i8255.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/mess/drivers/amiga.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/nvram.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/emu/machine/microtch.h \
	src/mame/includes/cd32.h \
	src/lib/formats/imd_dsk.h \
	src/mame/includes/amiga.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/emu/machine/6526cia.h \
	src/emu/machine/amigafdc.h \
	src/lib/util/chdcodec.h \
	src/lib/softfloat/milieu.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/lib/softfloat/softfloat.h \
	src/emu/imagedev/cartslot.h \
	src/mess/machine/amigacrt.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6525tpi.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/amigakbd.h \
	src/emu/sound/cdda.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/msm6242.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/machine/amigacd.h \
	src/lib/util/astring.h \
	src/mess/drivers/amiga.c \
	src/emu/machine/i2cmem.h \

$(OBJ)/mess/drivers/ampro.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/mess/drivers/ampro.c \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/amstr_pc.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/mess/video/pc_aga.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/video/pc_vga.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pc_fdc.h \
	src/lib/util/chd.h \
	src/emu/cpu/nec/nec.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/ins8250.h \
	src/emu/sound/sn76496.h \
	src/lib/util/md5.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/i86/i86.h \
	src/emu/machine/pckeybrd.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/emu/machine/pc_lpt.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/includes/pc.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/rs232/ser_mouse.h \
	src/mess/drivers/amstr_pc.c \
	src/emu/video/pc_cga.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/mess/includes/amstr_pc.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/amstrad.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/cpc/cpcexp.h \
	src/emu/sound/sp0256.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/mess/includes/amstrad.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/bus/cpc/mface2.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/cpc/cpc_rom.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/cpc/cpc_ssa1.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/tzx_cas.h \
	src/lib/util/opresolv.h \
	src/emu/bus/cpc/cpc_pds.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/formats/dsk_dsk.h \
	src/emu/sound/ay8910.h \
	src/mess/drivers/amstrad.c \

$(OBJ)/mess/drivers/apc.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/mess/audio/upd1771.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/nvram.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/i86/i86.h \
	src/mess/drivers/apc.c \
	src/emu/video/upd7220.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/machine/upd1990a.h \

$(OBJ)/mess/drivers/apexc.o : \
	src/mess/drivers/apexc.c \
	src/emu/cpu/apexc/apexc.h \

$(OBJ)/mess/drivers/apf.o : \
	src/lib/formats/flopimg.h \
	src/emu/video/mc6847.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/m6800/m6800.h \
	src/mess/drivers/apf.c \
	src/emu/machine/6821pia.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/apf_apt.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/apogee.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/rk_cas.h \
	src/mess/drivers/apogee.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/video/i8275.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mess/includes/radio86.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/8257dma.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/apollo.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/mess/machine/apollo_kbd.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/sound/2151intf.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/bus/isa/gus.h \
	src/emu/bus/isa/ne2000.h \
	src/mess/includes/apollo.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/lib/softfloat/milieu.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/lib/softfloat/softfloat.h \
	src/emu/cpu/m68000/m68kcpu.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/machine/keyboard.h \
	src/emu/bus/isa/finalchs.h \
	src/emu/imagedev/harddriv.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/lib/util/opresolv.h \
	src/emu/machine/n68681.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/clock.h \
	src/mess/drivers/apollo.c \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/terminal.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/beep.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/apple1.o : \
	src/mess/includes/apple1.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/apple1.c \
	src/emu/machine/6821pia.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/apple2.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/sound/tms5220.h \
	src/emu/bus/a2bus/a2ultraterm.h \
	src/emu/bus/a2bus/a2mockingboard.h \
	src/emu/bus/a2bus/a2memexp.h \
	src/emu/bus/centronics/image.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/mos6551.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/bus/a2bus/a2scsi.h \
	src/emu/video/tms9928a.h \
	src/emu/machine/atadev.h \
	src/emu/bus/a2bus/a2applicard.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/bus/a2bus/a2sam.h \
	src/emu/bus/a2bus/a2eext80col.h \
	src/emu/bus/a2bus/a2themill.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/machine/spchrom.h \
	src/emu/bus/a2bus/a2eramworks3.h \
	src/emu/bus/a2bus/a2diskii.h \
	src/mess/machine/applefdc.h \
	src/emu/sound/sn76496.h \
	src/emu/bus/a2bus/a2eauxslot.h \
	src/emu/bus/a2bus/a2alfam2.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/a2bus/a2videoterm.h \
	src/emu/video/mc6845.h \
	src/emu/bus/a2bus/a2ssc.h \
	src/emu/bus/a2bus/a2softcard.h \
	src/emu/bus/a2bus/a2swyft.h \
	src/emu/imagedev/printer.h \
	src/emu/machine/ncr5380n.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/a2bus/a2zipdrive.h \
	src/emu/machine/ataintf.h \
	src/emu/bus/a2bus/a2arcadebd.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/a2bus/laser128.h \
	src/lib/formats/ap2_dsk.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/a2bus/a2midi.h \
	src/mess/drivers/apple2.c \
	src/lib/util/opresolv.h \
	src/emu/bus/a2bus/a2lang.h \
	src/emu/machine/6522via.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/machine/kb3600.h \
	src/mess/includes/apple2.h \
	src/emu/machine/clock.h \
	src/emu/bus/a2bus/a2pic.h \
	src/emu/bus/a2bus/a2bus.h \
	src/emu/bus/a2bus/a2echoii.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/a2bus/a2thunderclock.h \
	src/emu/machine/upd1990a.h \
	src/emu/bus/a2bus/a2cffa.h \
	src/emu/sound/ay8910.h \
	src/emu/machine/nscsi_bus.h \
	src/emu/sound/dac.h \
	src/emu/bus/a2bus/a2estd80col.h \

$(OBJ)/mess/drivers/apple2gs.o : \
	src/lib/formats/flopimg.h \
	src/emu/sound/tms5220.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/cpu/m6502/m5074x.h \
	src/mess/includes/apple2gs.h \
	src/emu/machine/nvram.h \
	src/emu/bus/a2bus/a2mockingboard.h \
	src/emu/bus/a2bus/a2memexp.h \
	src/lib/util/corealloc.h \
	src/emu/cpu/m6502/m740.h \
	src/emu/sound/es5503.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/mos6551.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/bus/a2bus/a2scsi.h \
	src/emu/video/tms9928a.h \
	src/emu/machine/atadev.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/bus/a2bus/a2sam.h \
	src/mess/machine/appldriv.h \
	src/lib/formats/ap_dsk35.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/machine/spchrom.h \
	src/emu/bus/a2bus/a2diskii.h \
	src/emu/bus/a2bus/a2eauxslot.h \
	src/mess/machine/sonydriv.h \
	src/mess/machine/applefdc.h \
	src/emu/sound/sn76496.h \
	src/emu/bus/a2bus/a2alfam2.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/a2bus/a2ssc.h \
	src/emu/bus/a2bus/a2softcard.h \
	src/mess/drivers/apple2gs.c \
	src/emu/machine/6840ptm.h \
	src/emu/bus/a2bus/a2zipdrive.h \
	src/emu/machine/ncr5380n.h \
	src/emu/machine/ataintf.h \
	src/mess/machine/8530scc.h \
	src/emu/bus/a2bus/a2arcadebd.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/ap2_dsk.h \
	src/emu/bus/a2bus/a2midi.h \
	src/lib/util/opresolv.h \
	src/emu/bus/a2bus/a2lang.h \
	src/emu/machine/6522via.h \
	src/mess/machine/kb3600.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/mess/includes/apple2.h \
	src/emu/machine/clock.h \
	src/emu/bus/a2bus/a2vulcan.h \
	src/emu/bus/a2bus/a2hsscsi.h \
	src/emu/bus/a2bus/a2bus.h \
	src/emu/bus/a2bus/a2echoii.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/a2bus/a2thunderclock.h \
	src/emu/machine/upd1990a.h \
	src/emu/bus/a2bus/a2cffa.h \
	src/emu/sound/ay8910.h \
	src/emu/machine/nscsi_bus.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/apple3.o : \
	src/lib/formats/flopimg.h \
	src/mess/drivers/apple3.c \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/machine/mos6551.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/machine/atadev.h \
	src/emu/bus/a2bus/a2applicard.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/mess/machine/appldriv.h \
	src/emu/cpu/m6502/m65c02.h \
	src/mess/includes/apple3.h \
	src/emu/bus/a2bus/a2eauxslot.h \
	src/mess/machine/applefdc.h \
	src/emu/machine/mm58167.h \
	src/emu/machine/ataintf.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/ap2_dsk.h \
	src/emu/machine/6522via.h \
	src/lib/util/opresolv.h \
	src/mess/machine/kb3600.h \
	src/emu/machine/clock.h \
	src/mess/includes/apple2.h \
	src/emu/bus/a2bus/a2bus.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/a2bus/a2cffa.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/applix.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/applix_dsk.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/z80/z80.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/6522via.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/drivers/applix.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/apricot.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/emu/cpu/i8089/i8089_channel.h \
	src/lib/formats/apridisk.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/latch.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/sound/sn76496.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8089/i8089.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/mess/drivers/apricot.c \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/apricotf.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/mess/machine/apricotkb.h \
	src/emu/machine/z80ctc.h \
	src/mess/drivers/apricotf.c \
	src/emu/cpu/i86/i86.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/mess/includes/apricotf.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/apricotp.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/latch.h \
	src/mess/drivers/apricotp.c \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/includes/apricotp.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/mess/machine/apricotkb.h \
	src/emu/sound/sn76496.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/aquarius.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/aquarius.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/aquarius.c \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/arcadia.o : \
	src/emu/cpu/s2650/s2650.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/arcadia.c \
	src/mess/includes/arcadia.h \
	src/mess/audio/arcadia.h \

$(OBJ)/mess/drivers/argo.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/argo.c \

$(OBJ)/mess/drivers/astrocde.o : \
	src/emu/sound/astrocde.h \
	src/mame/includes/astrocde.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/mess/drivers/astrocde.c \

$(OBJ)/mess/drivers/at.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/bus/pci/i82371sb.h \
	src/emu/bus/pci/northbridge.h \
	src/mess/machine/cs8221.h \
	src/emu/machine/wd11c00_17.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/sound/2151intf.h \
	src/emu/cpu/i86/i286.h \
	src/emu/bus/pci/pci.h \
	src/emu/bus/isa/gus.h \
	src/emu/machine/latch.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/emu/machine/at_keybc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/bus/pci/i82439tx.h \
	src/emu/bus/pci/southbridge.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/lib/util/simple_set.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/mess/includes/at.h \
	src/lib/softfloat/milieu.h \
	src/emu/cpu/i386/i386.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/machine/ins8250.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/emu/bus/pc_kbd/keyboards.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/debug/express.h \
	src/mess/drivers/at.c \
	src/emu/bus/isa/finalchs.h \
	src/emu/imagedev/harddriv.h \
	src/emu/debug/debugcpu.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/emu/cpu/vtlb.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/ds128x.h \
	src/emu/machine/wd2010.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/pc_lpt.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/pci/i82371ab.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/atari400.o : \
	src/emu/machine/rescap.h \
	src/emu/machine/ram.h \
	src/mess/machine/atarifdc.h \
	src/mame/includes/atari.h \
	src/emu/machine/6821pia.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/m6502/m6502.h \
	src/mame/video/gtia.h \
	src/mess/drivers/atari400.c \
	src/emu/sound/pokey.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/atarist.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/rescap.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/dfi_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/latch.h \
	src/emu/machine/ram.h \
	src/mess/video/atarist.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/lmc1992.h \
	src/mess/includes/atarist.h \
	src/lib/formats/st_dsk.h \
	src/lib/softfloat/milieu.h \
	src/emu/machine/mc68901.h \
	src/emu/machine/6850acia.h \
	src/lib/softfloat/softfloat.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/rp5c15.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/mess/drivers/atarist.c \
	src/lib/formats/pasti_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/machine/8530scc.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/clock.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/atm.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/wd17xx.h \
	src/mess/machine/beta.h \
	src/mess/includes/spectrum.h \
	src/mess/drivers/atm.c \
	src/emu/machine/upd765.h \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/atom.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/mess/machine/i8271.h \
	src/lib/util/corealloc.h \
	src/emu/video/mc6847.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/atom.c \
	src/lib/formats/atom_tap.h \
	src/emu/machine/i8255.h \
	src/lib/formats/imageutl.h \
	src/emu/imagedev/cartslot.h \
	src/mess/includes/atom.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/uef_cas.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/6522via.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/attache.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/msm5832.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/nvram.h \
	src/emu/machine/z80pio.h \
	src/mess/drivers/attache.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/emu/video/tms9927.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/avigo.o : \
	src/mess/drivers/avigo.c \
	src/emu/machine/nvram.h \
	src/emu/bus/rs232/rs232.h \
	src/mess/includes/avigo.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/rp5c01.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/bus/rs232/null_modem.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/ins8250.h \
	src/emu/imagedev/snapquik.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/machine/intelfsh.h \

$(OBJ)/mess/drivers/ax20.o : \
	src/lib/formats/flopimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/cpu/i86/i86.h \
	src/mess/drivers/ax20.c \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \

$(OBJ)/mess/drivers/b16.o : \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/mess/drivers/b16.c \
	src/emu/machine/am9517a.h \

$(OBJ)/mess/drivers/b2m.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/formats/wd177x_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/lib/formats/smx_dsk.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/mess/drivers/b2m.c \
	src/lib/formats/d88_dsk.h \
	src/mess/includes/b2m.h \

$(OBJ)/mess/drivers/babbage.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/babbage.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \

$(OBJ)/mess/drivers/basic52.o : \
	src/emu/machine/i8255.h \
	src/mess/drivers/basic52.c \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/bbc.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/video/saa5050.h \
	src/emu/sound/tms5220.h \
	src/mess/includes/bbc.h \
	src/mess/machine/i8271.h \
	src/mess/drivers/bbc.c \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/upd7002.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/machine/spchrom.h \
	src/emu/machine/mc6854.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/sound/sn76496.h \
	src/emu/machine/6850acia.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/lib/formats/uef_cas.h \
	src/emu/bus/econet/econet.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/machine/clock.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/csw_cas.h \
	src/emu/cpu/m6502/r65c02.h \

$(OBJ)/mess/drivers/bbcbc.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/video/tms9928a.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/bbcbc.c \

$(OBJ)/mess/drivers/bcs3.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/bcs3.c \
	src/emu/cpu/z80/z80.h \

$(OBJ)/mess/drivers/bebox.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/machine/pit8253.h \
	src/mess/includes/bebox.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/video/pc_vga.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/bus/pci/pci.h \
	src/lib/util/cdrom.h \
	src/emu/machine/ram.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/53c810.h \
	src/lib/util/chd.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/emu/machine/8042kbdc.h \
	src/emu/machine/idectrl.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/sound/3812intf.h \
	src/emu/machine/scsidev.h \
	src/emu/imagedev/chd_cd.h \
	src/emu/machine/ins8250.h \
	src/lib/util/md5.h \
	src/emu/machine/upd765.h \
	src/emu/machine/pckeybrd.h \
	src/emu/bus/pci/mpc105.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/emu/machine/t10mmc.h \
	src/lib/util/corefile.h \
	src/emu/machine/scsicd.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/video/cirrus.h \
	src/emu/sound/cdda.h \
	src/mess/drivers/bebox.c \
	src/emu/machine/scsihd.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/cpu/powerpc/ppc.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/beehive.o : \
	src/mess/drivers/beehive.c \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/drivers/besta.o : \
	src/mess/machine/68561mpcc.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \
	src/mess/drivers/besta.c \

$(OBJ)/mess/drivers/beta.o : \
	src/mess/includes/beta.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6532riot.h \
	src/mess/drivers/beta.c \

$(OBJ)/mess/drivers/bigbord2.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/z80dma.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \
	src/mess/drivers/bigbord2.c \

$(OBJ)/mess/drivers/binbug.o : \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/s2650/s2650.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/binbug.c \
	src/emu/machine/z80ctc.h \
	src/emu/bus/rs232/keyboard.h \
	src/emu/machine/keyboard.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/bk.o : \
	src/lib/formats/rk_cas.h \
	src/mess/includes/bk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/drivers/bk.c \
	src/lib/formats/cassimg.h \
	src/emu/cpu/t11/t11.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/bmjr.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/m6800/m6800.h \
	src/mess/drivers/bmjr.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/bml3.o : \
	src/lib/formats/flopimg.h \
	src/emu/bus/bml3/bml3mp1802.h \
	src/emu/machine/mc6843.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/drivers/bml3.c \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/bml3/bml3kanji.h \
	src/emu/video/mc6845.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/lib/util/opresolv.h \
	src/emu/bus/bml3/bml3mp1805.h \
	src/emu/machine/clock.h \
	src/emu/bus/bml3/bml3bus.h \
	src/emu/sound/2203intf.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/bob85.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/bob85.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/br8641.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/br8641.c \

$(OBJ)/mess/drivers/bullet.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/bus/rs232/rs232.h \
	src/mess/drivers/bullet.c \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/util/sha1.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/z80dma.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/mess/includes/bullet.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/emu/machine/scsihd.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/busicom.o : \
	src/mess/includes/busicom.h \
	src/emu/cpu/i4004/i4004.h \
	src/mess/drivers/busicom.c \

$(OBJ)/mess/drivers/bw12.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/bw12_dsk.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/machine/latch.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/imd_dsk.h \
	src/mess/includes/bw12.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/drivers/bw12.c \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/mess/machine/kb3600.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/bw2.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/mess/drivers/bw2.c \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/lib/formats/bw2_dsk.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/video/msm6255.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/bw2.h \
	src/emu/machine/i8255.h \
	src/emu/bus/bw2/exp.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/c10.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/c10.c \

$(OBJ)/mess/drivers/c128.o : \
	src/lib/formats/flopimg.h \
	src/mess/drivers/c128.c \
	src/emu/bus/vic20/user.h \
	src/lib/formats/cbm_crt.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/mos8722.h \
	src/emu/bus/c64/bn1541.h \
	src/mess/video/mos6566.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/machine/64h156.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/cbmiec/c1581.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/formats/g64_dsk.h \
	src/lib/formats/d81_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/bus/pet/diag264_lb_tape.h \
	src/emu/sound/mos6581.h \
	src/lib/formats/imageutl.h \
	src/emu/cpu/m6502/m6510.h \
	src/emu/machine/pla.h \
	src/mess/machine/cbm_snqk.h \
	src/lib/util/jedparse.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/exp.h \
	src/emu/bus/vcs/ctrl.h \
	src/lib/formats/d71_dsk.h \
	src/emu/cpu/m6502/m8502.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/imagedev/snapquik.h \
	src/mess/includes/c128.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/user.h \
	src/emu/machine/6522via.h \
	src/lib/util/opresolv.h \
	src/lib/formats/cbm_tap.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/pet/c2n.h \
	src/emu/bus/pet/cass.h \
	src/emu/machine/mos6526.h \
	src/emu/bus/pet/user.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/d64_dsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/cbmiec/c1571.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/c64.o : \
	src/lib/formats/flopimg.h \
	src/emu/bus/vic20/user.h \
	src/lib/formats/cbm_crt.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/includes/c64.h \
	src/emu/bus/c64/bn1541.h \
	src/mess/video/mos6566.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/machine/64h156.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/bus/pet/diag264_lb_tape.h \
	src/emu/machine/6821pia.h \
	src/emu/sound/mos6581.h \
	src/lib/formats/imageutl.h \
	src/emu/cpu/m6502/m6510.h \
	src/emu/machine/pla.h \
	src/mess/machine/cbm_snqk.h \
	src/lib/util/jedparse.h \
	src/emu/bus/c64/exp.h \
	src/emu/bus/vcs/ctrl.h \
	src/lib/formats/d71_dsk.h \
	src/mess/drivers/c64.c \
	src/lib/formats/td0_dsk.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/user.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/lib/formats/cbm_tap.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/pet/c2n.h \
	src/emu/bus/pet/cass.h \
	src/emu/machine/mos6526.h \
	src/emu/bus/pet/user.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/bus/cbmiec/c1541.h \
	src/lib/formats/d64_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/c64dtv.o : \
	src/mess/drivers/c64dtv.c \

$(OBJ)/mess/drivers/c65.o : \
	src/mess/drivers/c65.c \
	src/emu/machine/ram.h \
	src/emu/cpu/m6502/m4510.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/sound/mos6581.h \
	src/mess/machine/cbm_snqk.h \
	src/emu/imagedev/cartslot.h \
	src/emu/imagedev/snapquik.h \
	src/mess/includes/c65.h \
	src/emu/cpu/m6502/m6502.h \
	src/mess/video/vic4567.h \
	src/emu/machine/mos6526.h \
	src/emu/cpu/m6502/m65ce02.h \

$(OBJ)/mess/drivers/c80.o : \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/mess/drivers/c80.c \
	src/mess/includes/c80.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/c900.o : \
	src/emu/cpu/z8000/z8000.h \
	src/emu/machine/keyboard.h \
	src/mess/drivers/c900.c \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/camplynx.o : \
	src/mess/drivers/camplynx.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/video/mc6845.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/casloopy.o : \
	src/mess/drivers/casloopy.c \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/sh2/sh2.h \

$(OBJ)/mess/drivers/cat.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/nvram.h \
	src/emu/bus/centronics/image.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/latch.h \
	src/emu/sound/speaker.h \
	src/lib/softfloat/mamesf.h \
	src/mess/drivers/cat.c \
	src/lib/softfloat/milieu.h \
	src/emu/machine/6850acia.h \
	src/lib/softfloat/softfloat.h \
	src/emu/imagedev/printer.h \
	src/emu/machine/6522via.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/n68681.h \
	src/emu/machine/clock.h \

$(OBJ)/mess/drivers/cbm2.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/mess/includes/cbm2.h \
	src/emu/bus/rs232/rs232.h \
	src/mess/video/mos6566.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/cbm2.c \
	src/emu/machine/pic8259.h \
	src/emu/machine/ds75160a.h \
	src/emu/bus/cbm2/exp.h \
	src/emu/bus/pet/diag264_lb_tape.h \
	src/emu/sound/mos6581.h \
	src/emu/machine/pla.h \
	src/mess/machine/cbm_snqk.h \
	src/lib/util/jedparse.h \
	src/emu/bus/cbm2/user.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/emu/bus/vcs/ctrl.h \
	src/emu/imagedev/snapquik.h \
	src/emu/machine/ds75161a.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/cbm_tap.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/pet/c2n.h \
	src/emu/bus/pet/cass.h \
	src/emu/machine/clock.h \
	src/emu/machine/mos6526.h \
	src/lib/formats/ioprocs.h \
	src/emu/cpu/m6502/m6509.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/ccs2810.o : \
	src/mess/drivers/ccs2810.c \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/terminal.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/ccs300.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/mess/drivers/ccs300.c \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/cd2650.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/drivers/cd2650.c \
	src/emu/cpu/s2650/s2650.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/keyboard.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/cdc721.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/cdc721.c \

$(OBJ)/mess/drivers/cfx9850.o : \
	src/emu/cpu/hcd62121/hcd62121.h \
	src/mess/drivers/cfx9850.c \

$(OBJ)/mess/drivers/cgc7900.o : \
	src/lib/formats/flopimg.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/lib/softfloat/mamesf.h \
	src/mess/includes/cgc7900.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/mess/drivers/cgc7900.c \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/cgenie.o : \
	src/lib/formats/flopimg.h \
	src/mess/includes/cgenie.h \
	src/lib/util/corealloc.h \
	src/lib/formats/cgen_cas.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/cgenie.c \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/channelf.o : \
	src/emu/cpu/f8/f8.h \
	src/mess/audio/channelf.h \
	src/mess/drivers/channelf.c \
	src/mess/includes/channelf.h \
	src/emu/imagedev/cartslot.h \

$(OBJ)/mess/drivers/chaos.o : \
	src/emu/cpu/s2650/s2650.h \
	src/mess/drivers/chaos.c \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/chessmst.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/chessmst.c \

$(OBJ)/mess/drivers/chesstrv.o : \
	src/emu/cpu/f8/f8.h \
	src/mess/drivers/chesstrv.c \

$(OBJ)/mess/drivers/clcd.o : \
	src/emu/machine/buffer.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/latch.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/mess/drivers/clcd.c \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/machine/msm58321.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/6522via.h \
	src/emu/machine/clock.h \
	src/emu/machine/bankdev.h \

$(OBJ)/mess/drivers/cm1800.o : \
	src/emu/machine/keyboard.h \
	src/emu/cpu/i8085/i8085.h \
	src/mess/drivers/cm1800.c \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/coco12.o : \
	src/emu/bus/coco/coco_232.h \
	src/emu/machine/ds1315.h \
	src/mess/machine/coco_vhd.h \
	src/emu/video/mc6847.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/ram.h \
	src/emu/bus/coco/coco_fdc.h \
	src/lib/formats/cassimg.h \
	src/emu/bus/coco/coco_orch90.h \
	src/mess/drivers/coco12.c \
	src/mess/includes/coco.h \
	src/mess/machine/6883sam.h \
	src/emu/machine/6821pia.h \
	src/mess/includes/coco12.h \
	src/emu/bus/coco/cococart.h \
	src/emu/bus/coco/coco_multi.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/emu/machine/clock.h \
	src/emu/machine/msm6242.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \
	src/emu/bus/coco/coco_pak.h \

$(OBJ)/mess/drivers/coco3.o : \
	src/mess/machine/coco_vhd.h \
	src/lib/util/corealloc.h \
	src/emu/video/mc6847.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/mess/includes/coco3.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/coco3.c \
	src/emu/cpu/m6809/hd6309.h \
	src/mess/includes/coco.h \
	src/mess/machine/6883sam.h \
	src/emu/machine/6821pia.h \
	src/mess/video/gime.h \
	src/emu/bus/coco/cococart.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/codata.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/mess/drivers/codata.c \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/coleco.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/video/tms9928a.h \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/sn76496.h \
	src/emu/imagedev/cartslot.h \
	src/mess/includes/coleco.h \
	src/mess/machine/coleco.h \
	src/mess/drivers/coleco.c \

$(OBJ)/mess/drivers/compis.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/emu/machine/i80130.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/bus/isbx/isbx.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/machine/i8255.h \
	src/mess/includes/compis.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/upd7220.h \
	src/mess/machine/compiskb.h \
	src/mess/drivers/compis.c \
	src/emu/machine/mm58274c.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/z80dart.h \
	src/emu/cpu/i86/i186.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/compucolor.o : \
	src/lib/formats/flopimg.h \
	src/emu/bus/compucolor/floppy.h \
	src/emu/machine/tms5501.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/ccvf_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/ram.h \
	src/mess/drivers/compucolor.c \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/video/tms9927.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/comquest.o : \
	src/mess/drivers/comquest.c \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/m6805/m6805.h \
	src/mess/includes/comquest.h \

$(OBJ)/mess/drivers/comx35.o : \
	src/emu/machine/rescap.h \
	src/emu/sound/cdp1869.h \
	src/mess/drivers/comx35.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imageutl.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/emu/bus/comx35/exp.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/cdp1871.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/mess/includes/comx35.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/concept.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/mos6551.h \
	src/lib/softfloat/mamesf.h \
	src/mess/machine/concept_exp.h \
	src/mess/includes/concept.h \
	src/lib/softfloat/milieu.h \
	src/mess/drivers/concept.c \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/mm58274c.h \
	src/emu/machine/6522via.h \
	src/emu/machine/clock.h \

$(OBJ)/mess/drivers/cortex.o : \
	src/emu/video/tms9928a.h \
	src/emu/cpu/tms9900/tms9995.h \
	src/mess/drivers/cortex.c \
	src/emu/cpu/tms9900/tms99com.h \

$(OBJ)/mess/drivers/cosmicos.o : \
	src/emu/video/resnet.h \
	src/emu/machine/rescap.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/sound/cdp1864.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/emu/video/dm9368.h \
	src/mess/includes/cosmicos.h \
	src/lib/formats/ioprocs.h \
	src/mess/drivers/cosmicos.c \

$(OBJ)/mess/drivers/cp1.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/drivers/cp1.c \
	src/lib/formats/cassimg.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/imagedev/snapquik.h \
	src/emu/machine/i8155.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/craft.o : \
	src/mess/drivers/craft.c \
	src/emu/cpu/avr8/avr8.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/crvision.o : \
	src/emu/machine/buffer.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/video/tms9928a.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/6821pia.h \
	src/mess/drivers/crvision.c \
	src/emu/sound/sn76496.h \
	src/emu/imagedev/cartslot.h \
	src/mess/includes/crvision.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/csc.o : \
	src/emu/sound/s14001a.h \
	src/mess/drivers/csc.c \
	src/emu/machine/6821pia.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/mess/drivers/ct486.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/sound/2151intf.h \
	src/emu/bus/isa/gus.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/emu/machine/at_keybc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/lib/util/simple_set.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/lib/softfloat/milieu.h \
	src/emu/cpu/i386/i386.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/lib/softfloat/softfloat.h \
	src/emu/bus/pc_kbd/keyboards.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/debug/express.h \
	src/emu/bus/isa/finalchs.h \
	src/emu/imagedev/harddriv.h \
	src/emu/debug/debugcpu.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/emu/cpu/vtlb.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/ct486.c \
	src/emu/machine/ds128x.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/emu/machine/wd2010.h \
	src/lib/util/opresolv.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/mess/machine/cs4031.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/cvicny.o : \
	src/mess/drivers/cvicny.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \

$(OBJ)/mess/drivers/cxhumax.o : \
	src/mess/drivers/cxhumax.c \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7core.h \
	src/emu/machine/keyboard.h \
	src/mess/includes/cxhumax.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/machine/intelfsh.h \
	src/emu/cpu/drcumlsh.h \
	src/emu/machine/terminal.h \
	src/emu/machine/i2cmem.h \

$(OBJ)/mess/drivers/cybiko.o : \
	src/mess/includes/cybiko.h \
	src/emu/machine/at45dbxx.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/video/hd66421.h \
	src/emu/cpu/h83002/h8.h \
	src/mess/drivers/cybiko.c \
	src/emu/imagedev/snapquik.h \
	src/emu/machine/intelfsh.h \
	src/emu/machine/pcf8593.h \

$(OBJ)/mess/drivers/czk80.o : \
	src/lib/formats/flopimg.h \
	src/mess/drivers/czk80.c \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/upd765.h \
	src/emu/machine/keyboard.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/d6800.o : \
	src/mess/drivers/d6800.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/6821pia.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/d6809.o : \
	src/mess/drivers/d6809.c \
	src/emu/machine/keyboard.h \
	src/emu/cpu/m6809/m6809.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/dai.o : \
	src/emu/machine/pit8253.h \
	src/emu/machine/tms5501.h \
	src/mess/includes/dai.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/dai.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/dccons.o : \
	src/mame/machine/maple-dc.h \
	src/emu/machine/aicartc.h \
	src/lib/util/palette.h \
	src/emu/sound/aica.h \
	src/mame/machine/naomig1.h \
	src/emu/cpu/sh4/sh4.h \
	src/mame/includes/dc.h \
	src/emu/cpu/arm7/arm7.h \
	src/emu/machine/atahle.h \
	src/lib/util/corealloc.h \
	src/lib/util/cdrom.h \
	src/emu/cpu/uml.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/cpu/drccache.h \
	src/lib/util/chd.h \
	src/mess/drivers/dccons.c \
	src/emu/cpu/arm7/arm7core.h \
	src/emu/machine/scsibus.h \
	src/lib/util/chdcodec.h \
	src/mame/video/powervr2.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/imagedev/chd_cd.h \
	src/emu/machine/atapicdr.h \
	src/mame/machine/gdrom.h \
	src/lib/util/md5.h \
	src/mame/machine/dc-ctrl.h \
	src/emu/cpu/drcuml.h \
	src/emu/machine/ataintf.h \
	src/emu/machine/t10mmc.h \
	src/mess/includes/dccons.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/drcfe.h \
	src/emu/machine/intelfsh.h \
	src/emu/sound/cdda.h \
	src/emu/cpu/drcumlsh.h \
	src/mame/machine/mapledev.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/lib/util/astring.h \
	src/emu/machine/atapihle.h \

$(OBJ)/mess/drivers/dct11em.o : \
	src/mess/drivers/dct11em.c \
	src/emu/cpu/t11/t11.h \

$(OBJ)/mess/drivers/dectalk.o : \
	src/emu/machine/x2212.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/n68681.h \
	src/emu/sound/dac.h \
	src/mess/drivers/dectalk.c \

$(OBJ)/mess/drivers/dgn_beta.o : \
	src/lib/formats/coco_dsk.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/6821pia.h \
	src/emu/video/mc6845.h \
	src/mess/drivers/dgn_beta.c \
	src/mess/includes/dgn_beta.h \
	src/emu/cpu/m6809/m6809.h \
	src/lib/util/opresolv.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/digel804.o : \
	src/emu/machine/roc10937.h \
	src/emu/machine/mos6551.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/mess/drivers/digel804.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/mm74c922.h \
	src/emu/machine/clock.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/dim68k.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/sound/speaker.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/mess/drivers/dim68k.c \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/dm7000.o : \
	src/mess/drivers/dm7000.c \
	src/mess/includes/dm7000.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \
	src/emu/cpu/powerpc/ppc.h \

$(OBJ)/mess/drivers/dms5000.o : \
	src/mess/drivers/dms5000.c \
	src/emu/cpu/i86/i86.h \

$(OBJ)/mess/drivers/dms86.o : \
	src/emu/cpu/i86/i86.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \
	src/mess/drivers/dms86.c \

$(OBJ)/mess/drivers/dmv.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/mess/drivers/dmv.c \
	src/emu/machine/upd765.h \
	src/emu/video/upd7220.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/dolphunk.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/s2650/s2650.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/dolphunk.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/dps1.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/dps1.c \

$(OBJ)/mess/drivers/dragon.o : \
	src/emu/bus/coco/coco_232.h \
	src/lib/formats/coco_dsk.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/ds1315.h \
	src/mess/machine/coco_vhd.h \
	src/emu/video/mc6847.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/ram.h \
	src/emu/bus/coco/coco_fdc.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/bus/coco/coco_orch90.h \
	src/mess/includes/coco.h \
	src/mess/machine/6883sam.h \
	src/emu/machine/6821pia.h \
	src/mess/includes/coco12.h \
	src/emu/bus/coco/cococart.h \
	src/emu/bus/coco/coco_multi.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/dgnalpha.h \
	src/emu/cpu/m6809/m6809.h \
	src/mess/includes/dragon.h \
	src/lib/util/opresolv.h \
	src/emu/machine/clock.h \
	src/mess/drivers/dragon.c \
	src/emu/machine/msm6242.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/dac.h \
	src/emu/bus/coco/coco_pak.h \

$(OBJ)/mess/drivers/dsb46.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/mess/drivers/dsb46.c \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/dual68.o : \
	src/mess/drivers/dual68.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/keyboard.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/eacc.o : \
	src/emu/machine/nvram.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/6821pia.h \
	src/mess/drivers/eacc.c \

$(OBJ)/mess/drivers/ec184x.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/emu/sound/2151intf.h \
	src/lib/util/corealloc.h \
	src/emu/bus/isa/gus.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/machine/ins8250.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/bus/pc_kbd/keyboards.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/bus/isa/finalchs.h \
	src/mess/includes/genpc.h \
	src/emu/imagedev/harddriv.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/lib/util/opresolv.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/mess/drivers/ec184x.c \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/ec65.o : \
	src/emu/cpu/g65816/g65816.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6522via.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/machine/clock.h \
	src/mess/drivers/ec65.c \

$(OBJ)/mess/drivers/einstein.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/video/tms9928a.h \
	src/emu/machine/wd_fdc.h \
	src/emu/machine/z80sio.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/einstein.h \
	src/emu/machine/z80ctc.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/imagedev/floppy.h \
	src/mess/drivers/einstein.c \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/dsk_dsk.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/electron.o : \
	src/lib/util/corealloc.h \
	src/mess/drivers/electron.c \
	src/mess/includes/electron.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/uef_cas.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/elekscmp.o : \
	src/mess/drivers/elekscmp.c \
	src/emu/cpu/scmp/scmp.h \

$(OBJ)/mess/drivers/elf.o : \
	src/emu/machine/rescap.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/video/cdp1861.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/elf.c \
	src/emu/imagedev/snapquik.h \
	src/emu/machine/mm74c922.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/emu/video/dm9368.h \
	src/mess/includes/elf.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/elwro800.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/mess/includes/spectrum.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/mess/drivers/elwro800.c \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/tzx_cas.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/ep64.o : \
	src/emu/machine/buffer.h \
	src/emu/video/resnet.h \
	src/emu/machine/rescap.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/mess/video/nick.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/ep64.c \
	src/mess/includes/ep64.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/ep64/exp.h \
	src/emu/imagedev/cartslot.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/audio/dave.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/esq1.o : \
	src/lib/formats/flopimg.h \
	src/emu/bus/midi/midi.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/sound/es5503.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/mess/machine/esqpanel.h \
	src/mess/drivers/esq1.c \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/machine/esqvfd.h \
	src/emu/cpu/m6809/m6809.h \
	src/lib/util/opresolv.h \
	src/emu/machine/n68681.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/esq5505.o : \
	src/lib/formats/flopimg.h \
	src/emu/bus/midi/midi.h \
	src/emu/sound/esqpump.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/wd_fdc.h \
	src/mess/machine/hd63450.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/mess/machine/esqpanel.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/lib/formats/esq16_dsk.h \
	src/emu/cpu/es5510/es5510.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/sound/es5506.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/machine/esqvfd.h \
	src/lib/util/opresolv.h \
	src/emu/machine/n68681.h \
	src/emu/imagedev/floppy.h \
	src/mess/drivers/esq5505.c \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/esqasr.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/mess/drivers/esqasr.c \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/emu/machine/68340tmu.h \
	src/emu/cpu/es5510/es5510.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/sound/es5506.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/machine/esqvfd.h \
	src/emu/machine/68340dma.h \
	src/lib/util/opresolv.h \
	src/emu/machine/68340ser.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/68340sim.h \
	src/emu/machine/68340.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/esqkt.o : \
	src/emu/bus/midi/midi.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/mess/machine/esqpanel.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/mess/drivers/esqkt.c \
	src/emu/cpu/es5510/es5510.h \
	src/emu/sound/es5506.h \
	src/mess/machine/esqvfd.h \
	src/emu/machine/n68681.h \

$(OBJ)/mess/drivers/esqmr.o : \
	src/mess/drivers/esqmr.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/68340tmu.h \
	src/emu/sound/es5506.h \
	src/mess/machine/esqvfd.h \
	src/emu/machine/68340dma.h \
	src/emu/machine/68340ser.h \
	src/emu/machine/68340sim.h \
	src/emu/machine/68340.h \

$(OBJ)/mess/drivers/et3400.o : \
	src/emu/cpu/m6800/m6800.h \
	src/mess/drivers/et3400.c \

$(OBJ)/mess/drivers/eti660.o : \
	src/emu/video/resnet.h \
	src/emu/machine/rescap.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/eti660.c \
	src/emu/machine/6821pia.h \
	src/emu/sound/cdp1864.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/eti660.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/evmbug.o : \
	src/mess/drivers/evmbug.c \
	src/emu/cpu/tms9900/tms9995.h \
	src/emu/machine/keyboard.h \
	src/emu/cpu/tms9900/tms99com.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/ex800.o : \
	src/emu/cpu/upd7810/upd7810.h \
	src/mess/drivers/ex800.c \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/exelv.o : \
	src/emu/sound/tms5220.h \
	src/emu/cpu/tms7000/tms7000.h \
	src/emu/machine/spchrom.h \
	src/mess/drivers/exelv.c \
	src/emu/video/tms3556.h \

$(OBJ)/mess/drivers/exp85.o : \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/exp85.c \
	src/mess/includes/exp85.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/i8155.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/i8355.h \

$(OBJ)/mess/drivers/fanucs15.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/cpu/i86/i286.h \
	src/mess/drivers/fanucs15.c \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/cpu/i86/i86.h \

$(OBJ)/mess/drivers/fidelz80.o : \
	src/emu/machine/z80pio.h \
	src/emu/sound/s14001a.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/mess/drivers/fidelz80.c \
	src/mess/includes/fidelz80.h \
	src/emu/machine/i8243.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/fk1.o : \
	src/emu/machine/pit8253.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/mess/drivers/fk1.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \

$(OBJ)/mess/drivers/fm7.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/bus/centronics/dsjoy.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/fm7_cas.h \
	src/emu/cpu/i86/i86.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/mess/includes/fm7.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/drivers/fm7.c \
	src/lib/formats/ioprocs.h \
	src/emu/sound/2203intf.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/fmtowns.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/mess/machine/upd71071.h \
	src/lib/util/palette.h \
	src/mess/drivers/fmtowns.c \
	src/emu/machine/nvram.h \
	src/emu/sound/2612intf.h \
	src/lib/softfloat/softfloat-macros \
	src/mess/includes/fmtowns.h \
	src/lib/util/corealloc.h \
	src/emu/sound/rf5c68.h \
	src/emu/sound/speaker.h \
	src/lib/util/cdrom.h \
	src/emu/machine/ram.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/lib/util/simple_set.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/lib/softfloat/milieu.h \
	src/emu/cpu/i386/i386.h \
	src/emu/machine/scsidev.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/lib/softfloat/softfloat.h \
	src/emu/debug/express.h \
	src/emu/imagedev/harddriv.h \
	src/emu/debug/debugcpu.h \
	src/emu/cpu/vtlb.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/machine/fm_scsi.h \
	src/emu/sound/cdda.h \
	src/emu/machine/scsihd.h \
	src/lib/util/hashing.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/scsihle.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/fp1100.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/upd7810/upd7810.h \
	src/mess/drivers/fp1100.c \

$(OBJ)/mess/drivers/fp200.o : \
	src/emu/cpu/i8085/i8085.h \
	src/mess/drivers/fp200.c \

$(OBJ)/mess/drivers/fp6000.o : \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/mess/drivers/fp6000.c \

$(OBJ)/mess/drivers/ft68m.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/mess/drivers/ft68m.c \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/galaxy.o : \
	src/lib/formats/gtp_cas.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/galaxy.c \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/mess/includes/galaxy.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/galeb.o : \
	src/mess/includes/galeb.h \
	src/mess/drivers/galeb.c \
	src/emu/cpu/m6502/m6502.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/gamecom.o : \
	src/emu/cpu/sm8500/sm8500.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/dac.h \
	src/mess/includes/gamecom.h \
	src/mess/drivers/gamecom.c \

$(OBJ)/mess/drivers/gamepock.o : \
	src/emu/sound/speaker.h \
	src/mess/drivers/gamepock.c \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/upd7810/upd7810.h \
	src/mess/includes/gamepock.h \

$(OBJ)/mess/drivers/gb.o : \
	src/emu/bus/gameboy/gb_slot.h \
	src/emu/cpu/lr35902/lr35902.h \
	src/emu/machine/ram.h \
	src/mess/includes/gb.h \
	src/mess/video/gb_lcd.h \
	src/mess/drivers/gb.c \
	src/emu/bus/gameboy/mbc.h \
	src/mess/audio/gb.h \
	src/emu/bus/gameboy/rom.h \

$(OBJ)/mess/drivers/gba.o : \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/emu/bus/gba/rom.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7core.h \
	src/mess/includes/gba.h \
	src/emu/cpu/drcuml.h \
	src/emu/bus/gba/gba_slot.h \
	src/mess/audio/gb.h \
	src/emu/cpu/drcfe.h \
	src/emu/machine/intelfsh.h \
	src/emu/cpu/drcumlsh.h \
	src/mess/drivers/gba.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/geneve.o : \
	src/mess/machine/ti99/peribox.h \
	src/emu/video/v9938.h \
	src/emu/video/tms9928a.h \
	src/emu/cpu/tms9900/tms9995.h \
	src/mess/machine/ti99/videowrp.h \
	src/mess/machine/ti99/ti99defs.h \
	src/emu/sound/sn76496.h \
	src/mess/drivers/geneve.c \
	src/emu/machine/mm58274c.h \
	src/mess/machine/ti99/genboard.h \
	src/mess/machine/ti99/joyport.h \
	src/emu/cpu/tms9900/tms99com.h \
	src/emu/machine/tms9901.h \

$(OBJ)/mess/drivers/geniusiq.o : \
	src/mess/drivers/geniusiq.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/intelfsh.h \

$(OBJ)/mess/drivers/genpc.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/mess/drivers/genpc.c \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/emu/sound/2151intf.h \
	src/lib/util/corealloc.h \
	src/emu/bus/isa/gus.h \
	src/emu/machine/latch.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/cpu/nec/nec.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/machine/ins8250.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/bus/pc_kbd/keyboards.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/bus/isa/finalchs.h \
	src/mess/includes/genpc.h \
	src/emu/imagedev/harddriv.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/pc_lpt.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/gizmondo.o : \
	src/emu/cpu/arm7/arm7.h \
	src/mess/machine/docg3.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/machine/s3c2440.h \
	src/mess/drivers/gizmondo.c \
	src/mess/video/gf4500.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/mess/drivers/glasgow.o : \
	src/mess/drivers/glasgow.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/mess/includes/mboard.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/gmaster.o : \
	src/emu/sound/speaker.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/upd7810/upd7810.h \
	src/mess/drivers/gmaster.c \

$(OBJ)/mess/drivers/gp2x.o : \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/mess/drivers/gp2x.c \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/mess/drivers/gp32.o : \
	src/emu/machine/nvram.h \
	src/emu/cpu/arm7/arm7.h \
	src/mess/machine/smartmed.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7core.h \
	src/mess/drivers/gp32.c \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \
	src/mess/includes/gp32.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/grfd2301.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/grfd2301.c \
	src/emu/cpu/z80/z80.h \

$(OBJ)/mess/drivers/h19.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/ins8250.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/emu/sound/beep.h \
	src/mess/drivers/h19.c \

$(OBJ)/mess/drivers/h8.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \
	src/mess/drivers/h8.c \

$(OBJ)/mess/drivers/h89.o : \
	src/emu/bus/rs232/rs232.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/ins8250.h \
	src/mess/drivers/h89.c \

$(OBJ)/mess/drivers/harriet.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/mess/drivers/harriet.c \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/hec2hrp.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/hect_tap.h \
	src/emu/sound/disc_dev.h \
	src/mess/includes/hec2hrp.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/hec2hrp.c \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/wavwrite.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/printer.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/emu/sound/sn76477.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/hect_dsk.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/drivers/homelab.o : \
	src/mess/drivers/homelab.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/mess/audio/mea8000.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/homez80.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/homez80.c \
	src/emu/cpu/z80/z80.h \

$(OBJ)/mess/drivers/horizon.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/bus/s100/nsmdsad.h \
	src/emu/bus/s100/s100.h \
	src/mess/drivers/horizon.c \
	src/emu/bus/s100/nsmdsa.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/hp16500.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/mess/drivers/hp16500.c \

$(OBJ)/mess/drivers/hp48.o : \
	src/emu/machine/nvram.h \
	src/mess/drivers/hp48.c \
	src/emu/cpu/saturn/saturn.h \
	src/mess/includes/hp48.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/hp49gp.o : \
	src/emu/machine/s3c2410.h \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/mess/drivers/hp49gp.c \
	src/emu/cpu/drccache.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/mess/drivers/hp9k.o : \
	src/mess/drivers/hp9k.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/hprot1.o : \
	src/mess/drivers/hprot1.c \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/video/hd44780.h \

$(OBJ)/mess/drivers/hpz80unk.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \
	src/mess/drivers/hpz80unk.c \

$(OBJ)/mess/drivers/ht68k.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/mess/drivers/ht68k.c \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/machine/n68681.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/huebler.o : \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/huebler.h \
	src/emu/machine/z80ctc.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/huebler.c \
	src/emu/machine/z80dart.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/hunter2.o : \
	src/emu/machine/nvram.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/video/hd61830.h \
	src/emu/bus/rs232/null_modem.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/mm58274c.h \
	src/emu/imagedev/bitbngr.h \
	src/mess/drivers/hunter2.c \
	src/emu/machine/nsc810.h \

$(OBJ)/mess/drivers/hx20.o : \
	src/emu/video/upd7227.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/machine/mc146818.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/bus/epson_sio/epson_sio.h \
	src/mess/includes/hx20.h \
	src/mess/drivers/hx20.c \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/hyperscan.o : \
	src/mess/drivers/hyperscan.c \
	src/emu/cpu/score/score.h \

$(OBJ)/mess/drivers/ibm6580.o : \
	src/mess/drivers/ibm6580.c \
	src/emu/cpu/i86/i86.h \

$(OBJ)/mess/drivers/ibmpc.o : \
	src/emu/sound/saa1099.h \
	src/emu/machine/buffer.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/emu/sound/2151intf.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/bus/isa/gus.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/mess/drivers/ibmpc.c \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/machine/ins8250.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/pc_kbd/keyboards.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/bus/isa/finalchs.h \
	src/mess/includes/genpc.h \
	src/emu/imagedev/harddriv.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/pc_lpt.h \
	src/lib/util/opresolv.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/ie15.o : \
	src/emu/cpu/ie15/ie15.h \
	src/emu/machine/keyboard.h \
	src/emu/imagedev/bitbngr.h \
	src/mess/drivers/ie15.c \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/if800.o : \
	src/mess/drivers/if800.c \
	src/emu/machine/pic8259.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/upd7220.h \

$(OBJ)/mess/drivers/imds.o : \
	src/mess/drivers/imds.c \
	src/emu/machine/keyboard.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/imsai.o : \
	src/emu/machine/pit8253.h \
	src/mess/drivers/imsai.c \
	src/emu/machine/i8251.h \
	src/emu/machine/keyboard.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/indiana.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/corealloc.h \
	src/emu/sound/2151intf.h \
	src/emu/bus/isa/gus.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/machine/i8251.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/lib/softfloat/milieu.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/machine/mc68901.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/bus/rs232/keyboard.h \
	src/emu/machine/keyboard.h \
	src/emu/bus/isa/finalchs.h \
	src/emu/imagedev/harddriv.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/mess/drivers/indiana.c \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/lib/util/opresolv.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/instruct.o : \
	src/mess/drivers/instruct.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/s2650/s2650.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/interact.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/hect_tap.h \
	src/emu/sound/disc_dev.h \
	src/mess/includes/hec2hrp.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/mess/drivers/interact.c \
	src/emu/sound/wavwrite.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/printer.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/emu/sound/sn76477.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/drivers/intv.o : \
	src/emu/cpu/cp1610/cp1610.h \
	src/emu/sound/sp0256.h \
	src/mess/drivers/intv.c \
	src/mess/video/stic.h \
	src/mess/includes/intv.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/ip20.o : \
	src/emu/cpu/mips/mips3.h \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/emu/machine/eepromser.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/mess/drivers/ip20.c \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/imagedev/chd_cd.h \
	src/emu/machine/eeprom.h \
	src/lib/util/md5.h \
	src/emu/machine/wd33c93.h \
	src/emu/machine/t10mmc.h \
	src/mess/machine/8530scc.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/scsicd.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/mess/machine/sgi.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/ip22.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/emu/machine/buffer.h \
	src/emu/cpu/mips/mips3.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/lib/util/palette.h \
	src/emu/bus/pci/i82371sb.h \
	src/emu/bus/pci/northbridge.h \
	src/mess/machine/cs8221.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/nvram.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/machine/mc146818.h \
	src/emu/sound/2151intf.h \
	src/lib/util/corealloc.h \
	src/emu/cpu/i86/i286.h \
	src/emu/bus/pci/pci.h \
	src/emu/bus/isa/gus.h \
	src/emu/machine/latch.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/lib/util/cdrom.h \
	src/emu/bus/isa/hdc.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/lib/util/sha1.h \
	src/emu/bus/isa/svga_s3.h \
	src/emu/machine/at_keybc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/softfloat/mamesf.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/lib/util/chd.h \
	src/emu/bus/pci/i82439tx.h \
	src/emu/bus/pci/southbridge.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/8042kbdc.h \
	src/emu/machine/t10sbc.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/lib/util/simple_set.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/ide.h \
	src/mess/includes/at.h \
	src/emu/machine/t10spc.h \
	src/lib/softfloat/milieu.h \
	src/emu/cpu/i386/i386.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/machine/scsidev.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/imagedev/chd_cd.h \
	src/emu/machine/ins8250.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/lib/util/md5.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/emu/machine/pckeybrd.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/debug/express.h \
	src/emu/bus/isa/finalchs.h \
	src/emu/imagedev/harddriv.h \
	src/emu/machine/wd33c93.h \
	src/emu/debug/debugcpu.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/emu/cpu/vtlb.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/emu/machine/t10mmc.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/emu/machine/scsicd.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/ip22.c \
	src/emu/machine/ds128x.h \
	src/emu/machine/wd2010.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/pc_lpt.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/mess/video/newport.h \
	src/emu/sound/cdda.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/pci/i82371ab.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/machine/scsihd.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/scsihle.h \
	src/mess/machine/sgi.h \
	src/lib/formats/d88_dsk.h \
	src/emu/bus/isa/aha1542.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/mufdc.h \
	src/emu/bus/isa/stereo_fx.h \
	src/lib/util/astring.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/ipc.o : \
	src/mess/drivers/ipc.c \
	src/emu/machine/keyboard.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/ipds.o : \
	src/emu/video/i8275.h \
	src/mess/drivers/ipds.c \
	src/emu/machine/keyboard.h \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/drivers/iq151.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/bus/iq151/grafik.h \
	src/emu/bus/iq151/video32.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/bus/iq151/disc2.h \
	src/emu/bus/iq151/video64.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/bus/iq151/minigraf.h \
	src/emu/machine/upd765.h \
	src/emu/bus/iq151/ms151a.h \
	src/emu/imagedev/printer.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/iq151/staper.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/bus/iq151/rom.h \
	src/mess/drivers/iq151.c \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/bus/iq151/iq151.h \

$(OBJ)/mess/drivers/irisha.o : \
	src/emu/machine/pit8253.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/mess/drivers/irisha.c \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/drivers/isbc.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/emu/cpu/i8089/i8089_channel.h \
	src/lib/util/palette.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/cpu/i86/i286.h \
	src/emu/machine/latch.h \
	src/emu/machine/i8251.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/util/sha1.h \
	src/mess/machine/isbc_215g.h \
	src/lib/util/chd.h \
	src/emu/bus/isbx/isbx.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/lib/util/md5.h \
	src/emu/cpu/i86/i86.h \
	src/emu/machine/keyboard.h \
	src/emu/imagedev/harddriv.h \
	src/emu/cpu/i8089/i8089.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/z80dart.h \
	src/lib/util/hashing.h \
	src/mess/drivers/isbc.c \
	src/emu/machine/terminal.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/iskr103x.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/emu/sound/2151intf.h \
	src/lib/util/corealloc.h \
	src/emu/bus/isa/gus.h \
	src/emu/machine/latch.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/cpu/nec/nec.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/machine/ins8250.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/bus/pc_kbd/keyboards.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/bus/isa/finalchs.h \
	src/mess/includes/genpc.h \
	src/emu/imagedev/harddriv.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/pc_lpt.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/mess/drivers/iskr103x.c \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/itt3030.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/formats/wd177x_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/bankdev.h \
	src/emu/video/tms9927.h \
	src/mess/drivers/itt3030.c \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/itt3030_dsk.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/jade.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/jade.c \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/jonos.o : \
	src/mess/drivers/jonos.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \

$(OBJ)/mess/drivers/jr100.o : \
	src/mess/drivers/jr100.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/6522via.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/jr200.o : \
	src/mess/drivers/jr200.c \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/jtc.o : \
	src/emu/machine/buffer.h \
	src/mess/drivers/jtc.c \
	src/emu/cpu/z8/z8.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/juicebox.o : \
	src/emu/cpu/arm7/arm7.h \
	src/mess/machine/smartmed.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/mess/drivers/juicebox.c \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \
	src/mess/machine/s3c44b0.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/junior.o : \
	src/mess/drivers/junior.c \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6532riot.h \

$(OBJ)/mess/drivers/jupiter.o : \
	src/lib/formats/flopimg.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/m6800/m6800.h \
	src/mess/drivers/jupiter.c \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/jupiter.h \
	src/emu/machine/keyboard.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/k1003.o : \
	src/mess/drivers/k1003.c \
	src/emu/cpu/i8008/i8008.h \

$(OBJ)/mess/drivers/k8915.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/k8915.c \
	src/emu/machine/keyboard.h \

$(OBJ)/mess/drivers/kaypro.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/mess/drivers/kaypro.c \
	src/emu/machine/z80sio.h \
	src/lib/formats/imd_dsk.h \
	src/lib/formats/wd177x_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/kaypro.h \
	src/emu/machine/com8116.h \
	src/emu/video/mc6845.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/formats/kaypro_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/kc.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/kc_cas.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/bus/kc/d004.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/z80ctc.h \
	src/lib/util/md5.h \
	src/emu/machine/upd765.h \
	src/emu/bus/kc/ram.h \
	src/emu/imagedev/harddriv.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/emu/sound/wave.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/kc/d002.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/kc_keyb.h \
	src/mess/includes/kc.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/bus/kc/rom.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \
	src/emu/bus/kc/kc.h \
	src/mess/drivers/kc.c \

$(OBJ)/mess/drivers/kim1.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/kim1_cas.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/kim1.c \
	src/emu/machine/mos6530.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/konin.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/konin.c \
	src/emu/cpu/z80/z80.h \

$(OBJ)/mess/drivers/korgm1.o : \
	src/emu/cpu/nec/nec.h \
	src/mess/drivers/korgm1.c \

$(OBJ)/mess/drivers/kramermc.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/kramermc.c \
	src/mess/includes/kramermc.h \

$(OBJ)/mess/drivers/kyocera.o : \
	src/emu/machine/buffer.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/emu/machine/rp5c01.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/video/hd61830.h \
	src/emu/machine/im6402.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/hd44102.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/i8155.h \
	src/mess/drivers/kyocera.c \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/includes/kyocera.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/upd1990a.h \

$(OBJ)/mess/drivers/lc80.o : \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/lc80.h \
	src/emu/machine/z80ctc.h \
	src/mess/drivers/lc80.c \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/lcmate2.o : \
	src/emu/machine/nvram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/lcmate2.c \
	src/emu/machine/rp5c15.h \
	src/emu/video/hd44780.h \

$(OBJ)/mess/drivers/lft.o : \
	src/emu/cpu/i86/i86.h \
	src/emu/machine/keyboard.h \
	src/mess/drivers/lft.c \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/lisa.o : \
	src/lib/formats/flopimg.h \
	src/emu/cpu/cop400/cop400.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/sound/speaker.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/softfloat/mamesf.h \
	src/lib/formats/ap_dsk35.h \
	src/lib/softfloat/milieu.h \
	src/mess/machine/sonydriv.h \
	src/mess/machine/applefdc.h \
	src/emu/cpu/m6502/m6504.h \
	src/lib/softfloat/softfloat.h \
	src/mess/includes/lisa.h \
	src/mess/machine/8530scc.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/lib/formats/ioprocs.h \
	src/mess/drivers/lisa.c \

$(OBJ)/mess/drivers/llc.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/mess/drivers/llc.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/keyboard.h \
	src/mess/includes/llc.h \
	src/mess/machine/k7659kb.h \

$(OBJ)/mess/drivers/lola8a.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/lola8a.c \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/lviv.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mess/drivers/lviv.c \
	src/emu/cpu/i8085/i8085.h \
	src/mess/includes/lviv.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/lviv_lvt.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/lx800.o : \
	src/mess/machine/e05a03.h \
	src/emu/cpu/upd7810/upd7810.h \
	src/mess/drivers/lx800.c \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/lynx.o : \
	src/mess/audio/lynx.h \
	src/mess/includes/lynx.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/imagedev/cartslot.h \
	src/emu/imagedev/snapquik.h \
	src/emu/cpu/m6502/m6502.h \
	src/mess/drivers/lynx.c \
	src/emu/cpu/m6502/r65c02.h \

$(OBJ)/mess/drivers/m20.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/drivers/m20.c \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/formats/wd177x_dsk.h \
	src/emu/cpu/z8000/z8000.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/m20_dsk.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/m5.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/upd765_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/sord_cas.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/video/tms9928a.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/z80ctc.h \
	src/emu/sound/sn76496.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/mess/includes/m5.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/m5.c \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/m5_dsk.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/m6805evs.o : \
	src/mess/drivers/m6805evs.c \
	src/emu/cpu/m6805/m6805.h \

$(OBJ)/mess/drivers/m79152pc.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/m79152pc.c \
	src/emu/cpu/z80/z80.h \

$(OBJ)/mess/drivers/mac.o : \
	src/emu/bus/nubus/nubus_asntmc3b.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/mess/machine/egret.h \
	src/emu/bus/nubus/nubus_vikbw.h \
	src/mess/machine/macpds.h \
	src/emu/sound/asc.h \
	src/emu/bus/nubus/nubus_wsportrait.h \
	src/mess/machine/macrtc.h \
	src/lib/util/corealloc.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/mess/includes/mac.h \
	src/lib/util/cdrom.h \
	src/emu/machine/ram.h \
	src/emu/bus/nubus/nubus_radiustpd.h \
	src/lib/util/sha1.h \
	src/emu/bus/nubus/pds30_mc30.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/bus/nubus/nubus_cb264.h \
	src/mess/machine/cuda.h \
	src/emu/bus/nubus/pds30_30hr.h \
	src/emu/machine/dp8390.h \
	src/lib/util/chd.h \
	src/emu/bus/nubus/nubus_image.h \
	src/lib/softfloat/mamesf.h \
	src/lib/formats/ap_dsk35.h \
	src/emu/bus/nubus/pds30_cb264.h \
	src/mess/machine/ncr5380.h \
	src/emu/machine/t10sbc.h \
	src/emu/bus/nubus/nubus_m2video.h \
	src/emu/machine/scsibus.h \
	src/mess/drivers/mac.c \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/lib/softfloat/milieu.h \
	src/emu/imagedev/chd_cd.h \
	src/mess/machine/applefdc.h \
	src/mess/machine/sonydriv.h \
	src/emu/bus/nubus/nubus_spec8.h \
	src/lib/util/md5.h \
	src/emu/bus/nubus/pds30_procolor816.h \
	src/lib/softfloat/softfloat.h \
	src/mess/video/pds_tpdfpd.h \
	src/mess/machine/swim.h \
	src/emu/imagedev/harddriv.h \
	src/emu/bus/nubus/nubus_48gc.h \
	src/emu/sound/awacs.h \
	src/mess/machine/mackbd.h \
	src/emu/machine/t10mmc.h \
	src/emu/bus/nubus/nubus.h \
	src/lib/util/corefile.h \
	src/mess/machine/8530scc.h \
	src/emu/machine/scsicd.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/ncr539x.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/cpu/m6805/m6805.h \
	src/emu/sound/cdda.h \
	src/emu/bus/nubus/nubus_specpdq.h \
	src/emu/machine/scsihd.h \
	src/lib/util/hashing.h \
	src/emu/bus/nubus/nubus_m2hires.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/scsihle.h \
	src/lib/util/harddisk.h \
	src/emu/bus/nubus/pds30_sigmalview.h \
	src/emu/cpu/powerpc/ppc.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/macpci.o : \
	src/lib/util/palette.h \
	src/mess/drivers/macpci.c \
	src/mess/includes/macpci.h \
	src/lib/util/corealloc.h \
	src/emu/machine/ram.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/mess/machine/cuda.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/emu/sound/awacs.h \
	src/mess/machine/8530scc.h \
	src/lib/util/corefile.h \
	src/emu/machine/ncr539x.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/6522via.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/emu/cpu/powerpc/ppc.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/mbc200.o : \
	src/lib/formats/flopimg.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/imagedev/flopdrv.h \
	src/mess/drivers/mbc200.c \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/video/mc6845.h \
	src/lib/util/opresolv.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/mbc55x.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/emu/debug/debugcon.h \
	src/mess/includes/mbc55x.h \
	src/lib/formats/upd765_dsk.h \
	src/mess/drivers/mbc55x.c \
	src/emu/machine/latch.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/emu/debug/textbuf.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/mbee.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/lib/formats/wd177x_dsk.h \
	src/mess/drivers/mbee.c \
	src/emu/cpu/z80/z80.h \
	src/emu/video/mc6845.h \
	src/mess/includes/mbee.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/mbee_cas.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/formats/dsk_dsk.h \
	src/lib/formats/mbee_dsk.h \

$(OBJ)/mess/drivers/mc10.o : \
	src/emu/video/mc6847.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/m6800/m6800.h \
	src/mess/video/ef9345.h \
	src/lib/formats/coco_cas.h \
	src/emu/imagedev/printer.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/mess/drivers/mc10.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/mc1000.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/rescap.h \
	src/lib/util/corealloc.h \
	src/emu/video/mc6847.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/mc1000.h \
	src/emu/video/mc6845.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \
	src/mess/drivers/mc1000.c \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/mc1502.o : \
	src/emu/bus/isa/p1_hdc.h \
	src/emu/bus/isa/xtide.h \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/mc1502_rom.h \
	src/mess/includes/mc1502.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/bus/isa/lpt.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/mess/machine/kb_7007_3.h \
	src/mess/drivers/mc1502.c \
	src/emu/bus/isa/p1_fdc.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/cpu/i86/i86.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/sound/wave.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/xsu_cards.h \
	src/emu/video/pc_cga.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/isa/mc1502_fdc.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/p1_rom.h \

$(OBJ)/mess/drivers/mc80.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/mess/includes/mc80.h \
	src/mess/drivers/mc80.c \
	src/emu/machine/z80dart.h \

$(OBJ)/mess/drivers/mcb216.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/mcb216.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/mccpm.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/mccpm.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/megadriv.o : \
	src/mame/machine/megacdcd.h \
	src/lib/util/palette.h \
	src/mame/includes/megadriv.h \
	src/emu/machine/nvram.h \
	src/emu/sound/upd7759.h \
	src/emu/sound/2612intf.h \
	src/emu/bus/megadrive/jcart.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/corealloc.h \
	src/emu/sound/rf5c68.h \
	src/mame/machine/megavdp.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/util/cdrom.h \
	src/emu/cpu/uml.h \
	src/lib/util/coreutil.h \
	src/emu/bus/megadrive/sk.h \
	src/lib/util/sha1.h \
	src/emu/cpu/drccache.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/ssp1601/ssp1601.h \
	src/lib/util/chdcodec.h \
	src/emu/cpu/sh2/sh2comn.h \
	src/lib/softfloat/milieu.h \
	src/emu/bus/megadrive/md_carts.h \
	src/mess/includes/md_cons.h \
	src/emu/bus/megadrive/svp.h \
	src/emu/bus/megadrive/md_slot.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/formats/imageutl.h \
	src/emu/sound/sn76496.h \
	src/lib/util/md5.h \
	src/lib/softfloat/softfloat.h \
	src/emu/bus/megadrive/eeprom.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/megadriv.c \
	src/emu/bus/megadrive/rom.h \
	src/emu/cpu/sh2/sh2.h \
	src/emu/machine/lc89510.h \
	src/emu/bus/megadrive/stm95.h \
	src/emu/cpu/drcuml.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/drcfe.h \
	src/emu/sound/cdda.h \
	src/emu/cpu/drcumlsh.h \
	src/mame/machine/megacd.h \
	src/lib/util/hashing.h \
	src/mame/machine/mega32x.h \
	src/lib/util/astring.h \
	src/emu/sound/dac.h \
	src/emu/video/315_5124.h \
	src/emu/machine/i2cmem.h \

$(OBJ)/mess/drivers/mekd2.o : \
	src/mess/drivers/mekd2.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/mephisto.o : \
	src/mess/drivers/mephisto.c \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/cpu/m6502/m6502.h \
	src/mess/includes/mboard.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/merlin.o : \
	src/emu/cpu/tms0980/tms0980.h \
	src/emu/sound/speaker.h \
	src/mess/drivers/merlin.c \

$(OBJ)/mess/drivers/mes.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/mes.c \

$(OBJ)/mess/drivers/mice.o : \
	src/emu/machine/keyboard.h \
	src/emu/cpu/i8085/i8085.h \
	src/mess/drivers/mice.c \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/microdec.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/drivers/microdec.c \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/upd765.h \
	src/emu/machine/keyboard.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/microkit.o : \
	src/mess/drivers/microkit.c \
	src/emu/cpu/cosmac/cosmac.h \

$(OBJ)/mess/drivers/micronic.o : \
	src/mess/drivers/micronic.c \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/video/hd61830.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/micronic.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/microtan.o : \
	src/mess/drivers/microtan.c \
	src/mess/includes/microtan.h \
	src/lib/util/corealloc.h \
	src/emu/machine/mos6551.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6522via.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/microvsn.o : \
	src/mess/drivers/microvsn.c \
	src/emu/cpu/tms0980/tms0980.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/mikro80.o : \
	src/lib/formats/rk_cas.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/i8085/i8085.h \
	src/mess/includes/mikro80.h \
	src/mess/drivers/mikro80.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/mikromik.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/mess/drivers/mikromik.c \
	src/lib/formats/upd765_dsk.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/i8212.h \
	src/emu/video/i8275x.h \
	src/lib/formats/imd_dsk.h \
	src/mess/includes/mikromik.h \
	src/lib/formats/mm_dsk.h \
	src/emu/machine/upd765.h \
	src/emu/video/upd7220.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/mikrosha.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/rk_cas.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/video/i8275.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mess/drivers/mikrosha.c \
	src/mess/includes/radio86.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/8257dma.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/mini2440.o : \
	src/mess/drivers/mini2440.c \
	src/emu/cpu/arm7/arm7.h \
	src/mess/machine/smartmed.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/machine/s3c2440.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/mirage.o : \
	src/mess/drivers/mirage.c \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/sound/es5503.h \
	src/lib/formats/esq8_dsk.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/6850acia.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/cpu/m6809/m6809.h \
	src/emu/machine/6522via.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/mits680b.o : \
	src/emu/machine/mos6551.h \
	src/emu/cpu/m6800/m6800.h \
	src/mess/drivers/mits680b.c \
	src/emu/machine/keyboard.h \
	src/emu/machine/clock.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/mk1.o : \
	src/emu/cpu/f8/f8.h \
	src/mess/drivers/mk1.c \
	src/emu/machine/f3853.h \

$(OBJ)/mess/drivers/mk14.o : \
	src/emu/machine/ins8154.h \
	src/emu/cpu/scmp/scmp.h \
	src/mess/drivers/mk14.c \

$(OBJ)/mess/drivers/mk2.o : \
	src/emu/sound/speaker.h \
	src/mess/drivers/mk2.c \
	src/emu/machine/mos6530.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/mess/drivers/mk85.o : \
	src/mess/drivers/mk85.c \
	src/emu/cpu/t11/t11.h \

$(OBJ)/mess/drivers/mk90.o : \
	src/mess/drivers/mk90.c \
	src/emu/cpu/t11/t11.h \

$(OBJ)/mess/drivers/mkit09.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/6821pia.h \
	src/mess/drivers/mkit09.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/mmd1.o : \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/i8279.h \
	src/mess/drivers/mmd1.c \

$(OBJ)/mess/drivers/mmodular.o : \
	src/emu/machine/nvram.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/mess/drivers/mmodular.c \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/softfloat/milieu.h \
	src/emu/cpu/arm/arm.h \
	src/lib/softfloat/softfloat.h \
	src/emu/cpu/m6502/m6502.h \
	src/mess/includes/mboard.h \
	src/emu/sound/beep.h \
	src/emu/video/hd44780.h \

$(OBJ)/mess/drivers/mod8.o : \
	src/emu/bus/rs232/teleprinter.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \
	src/emu/cpu/i8008/i8008.h \
	src/mess/drivers/mod8.c \

$(OBJ)/mess/drivers/modellot.o : \
	src/mess/drivers/modellot.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \

$(OBJ)/mess/drivers/molecular.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/molecular.c \
	src/emu/cpu/i86/i86.h \

$(OBJ)/mess/drivers/mpf1.o : \
	src/emu/sound/tms5220.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/z80sio.h \
	src/emu/machine/spchrom.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/z80ctc.h \
	src/mess/includes/mpf1.h \
	src/mess/drivers/mpf1.c \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/mpz80.o : \
	src/mess/drivers/mpz80.c \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/bus/s100/wunderbus.h \
	src/emu/bus/s100/s100.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/com8116.h \
	src/emu/bus/s100/dj2db.h \
	src/emu/machine/ins8250.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/s100/djdma.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/includes/mpz80.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/s100/mm65k16s.h \
	src/lib/formats/d88_dsk.h \
	src/emu/machine/upd1990a.h \

$(OBJ)/mess/drivers/ms0515.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/ram.h \
	src/mess/drivers/ms0515.c \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/t11/t11.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \

$(OBJ)/mess/drivers/ms9540.o : \
	src/mess/drivers/ms9540.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/msbc1.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/keyboard.h \
	src/mess/includes/msbc1.h \
	src/mess/drivers/msbc1.c \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/mstation.o : \
	src/mess/drivers/mstation.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/rp5c01.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/intelfsh.h \

$(OBJ)/mess/drivers/msx.o : \
	src/lib/formats/fmsx_cas.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/centronics/covox.h \
	src/lib/util/corealloc.h \
	src/mess/drivers/msx.c \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/rp5c01.h \
	src/emu/video/v9938.h \
	src/emu/video/tms9928a.h \
	src/mess/includes/msx_slot.h \
	src/emu/sound/k051649.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/msx_dsk.h \
	src/emu/machine/i8255.h \
	src/emu/sound/2413intf.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/mess/includes/msx.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/mtx.o : \
	src/emu/machine/buffer.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/video/tms9928a.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/mtx.c \
	src/emu/machine/z80ctc.h \
	src/emu/sound/sn76496.h \
	src/mess/includes/mtx.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/multi16.o : \
	src/mess/drivers/multi16.c \
	src/emu/machine/pic8259.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \

$(OBJ)/mess/drivers/multi8.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/video/mc6845.h \
	src/mess/drivers/multi8.c \
	src/emu/sound/2203intf.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/mx2178.o : \
	src/emu/bus/rs232/rs232.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/6850acia.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/clock.h \
	src/mess/drivers/mx2178.c \

$(OBJ)/mess/drivers/myb3k.o : \
	src/lib/formats/flopimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/mess/drivers/myb3k.c \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \

$(OBJ)/mess/drivers/mycom.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/msm5832.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/sound/sn76496.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/mess/drivers/mycom.c \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/myvision.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/video/tms9928a.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/myvision.c \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/mz2000.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/z80sio.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/mz_cas.h \
	src/emu/machine/i8255.h \
	src/mess/drivers/mz2000.c \
	src/emu/machine/rp5c15.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/mz2500.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/rp5c15.h \
	src/mess/drivers/mz2500.c \
	src/lib/util/opresolv.h \
	src/emu/machine/z80dart.h \
	src/emu/sound/2203intf.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/mz3500.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/upd765.h \
	src/mess/drivers/mz3500.c \
	src/emu/video/upd7220.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/mz6500.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/drivers/mz6500.c \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/upd7220.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/mz700.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/mess/includes/mz700.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/74145.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/mz_cas.h \
	src/emu/machine/i8255.h \
	src/emu/sound/sn76496.h \
	src/mess/drivers/mz700.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/mz80.o : \
	src/emu/machine/pit8253.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/mess/includes/mz80.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/mess/drivers/mz80.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/n64.o : \
	src/emu/cpu/mips/mips3.h \
	src/emu/cpu/rsp/rsp.h \
	src/emu/sound/dmadac.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/n64.c \
	src/mame/includes/n64.h \

$(OBJ)/mess/drivers/nakajies.o : \
	src/mess/drivers/nakajies.c \
	src/emu/machine/rp5c01.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/nec/nec.h \

$(OBJ)/mess/drivers/nanos.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/upd765.h \
	src/lib/formats/nanos_dsk.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/mess/drivers/nanos.c \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/nascom1.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/nascom1.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/nascom1.c \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/ay31015.h \

$(OBJ)/mess/drivers/nc.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/mc146818.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/machine/rp5c01.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/mess/includes/nc.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/nc.c \
	src/emu/machine/upd765.h \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/clock.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/nes.o : \
	src/emu/bus/nes/konami.h \
	src/emu/bus/nes/bootleg.h \
	src/emu/bus/nes/rexsoft.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/nes/karastudio.h \
	src/emu/bus/nes/act53.h \
	src/emu/bus/nes/mmc1.h \
	src/emu/bus/nes/txc.h \
	src/emu/bus/nes/sunsoft.h \
	src/emu/cpu/m6502/n2a03.h \
	src/emu/bus/nes/aladdin.h \
	src/emu/bus/nes/henggedianzi.h \
	src/emu/bus/nes/legacy.h \
	src/emu/bus/nes/pt554.h \
	src/emu/bus/nes/rcm.h \
	src/lib/util/corealloc.h \
	src/emu/sound/nes_apu.h \
	src/emu/bus/nes/namcot.h \
	src/emu/bus/nes/sunsoft_dcs.h \
	src/emu/imagedev/cassette.h \
	src/emu/bus/nes/mmc5.h \
	src/emu/bus/nes/discrete.h \
	src/emu/bus/nes/ave.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/bus/nes/jaleco.h \
	src/emu/bus/nes/nes_slot.h \
	src/emu/bus/nes/hes.h \
	src/emu/bus/nes/jy.h \
	src/emu/bus/nes/racermate.h \
	src/emu/bus/nes/bandai.h \
	src/emu/bus/nes/hosenkan.h \
	src/emu/bus/nes/nanjing.h \
	src/emu/bus/nes/datach.h \
	src/emu/bus/nes/sachen.h \
	src/emu/bus/nes/mmc3_clones.h \
	src/emu/bus/nes/cony.h \
	src/emu/bus/nes/somari.h \
	src/emu/machine/bcreader.h \
	src/emu/sound/2413intf.h \
	src/emu/bus/nes/nes_carts.h \
	src/lib/formats/nes_dsk.h \
	src/emu/sound/samples.h \
	src/emu/sound/nes_defs.h \
	src/emu/bus/nes/pirate.h \
	src/mess/includes/nes.h \
	src/emu/bus/nes/taito.h \
	src/mess/drivers/nes.c \
	src/emu/bus/nes/nxrom.h \
	src/emu/bus/nes/mmc3.h \
	src/emu/bus/nes/multigame.h \
	src/emu/bus/nes/camerica.h \
	src/emu/bus/nes/kaiser.h \
	src/mame/video/ppu2c0x.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/nes/ggenie.h \
	src/lib/util/opresolv.h \
	src/emu/bus/nes/waixing.h \
	src/emu/bus/nes/tengen.h \
	src/emu/bus/nes/ntdec.h \
	src/emu/bus/nes/benshieng.h \
	src/emu/bus/nes/event.h \
	src/emu/bus/nes/irem.h \
	src/emu/sound/vrc6.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \
	src/emu/bus/nes/cne.h \
	src/emu/bus/nes/mmc2.h \
	src/emu/machine/i2cmem.h \

$(OBJ)/mess/drivers/newbrain.o : \
	src/emu/machine/adc0808.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/drivers/newbrain.c \
	src/emu/cpu/cop400/cop400.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/z80sio.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/mess/includes/newbrain.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/next.o : \
	src/lib/formats/pc_dsk.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/formats/upd765_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/nscsi_cd.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/mess/machine/nextkbd.h \
	src/lib/util/chdcodec.h \
	src/lib/softfloat/milieu.h \
	src/emu/imagedev/chd_cd.h \
	src/mess/includes/next.h \
	src/mess/machine/ncr5390.h \
	src/lib/util/md5.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/mess/machine/mb8795.h \
	src/mess/machine/nextmo.h \
	src/emu/imagedev/harddriv.h \
	src/emu/machine/nscsi_hd.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/machine/8530scc.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/mccs1850.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/machine/nscsi_bus.h \
	src/lib/util/astring.h \
	src/mess/drivers/next.c \

$(OBJ)/mess/drivers/ng_aes.o : \
	src/mame/machine/megacdcd.h \
	src/lib/util/palette.h \
	src/emu/machine/nvram.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/corealloc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/util/cdrom.h \
	src/mame/includes/neogeo.h \
	src/lib/util/sha1.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/lib/softfloat/milieu.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/lib/softfloat/softfloat.h \
	src/emu/imagedev/cartslot.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/sound/2610intf.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/mess/drivers/ng_aes.c \
	src/emu/machine/upd1990a.h \
	src/emu/sound/ay8910.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/ngp.o : \
	src/emu/sound/t6w28.h \
	src/emu/cpu/tlcs900/tlcs900.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/ngp.c \
	src/mess/video/k1ge.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/ob68k1a.o : \
	src/emu/bus/rs232/rs232.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/ram.h \
	src/mess/includes/ob68k1a.h \
	src/lib/softfloat/mamesf.h \
	src/emu/machine/com8116.h \
	src/lib/softfloat/milieu.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/lib/softfloat/softfloat.h \
	src/mess/drivers/ob68k1a.c \
	src/emu/machine/6840ptm.h \

$(OBJ)/mess/drivers/octopus.o : \
	src/mess/drivers/octopus.c \
	src/emu/cpu/i86/i86.h \

$(OBJ)/mess/drivers/odyssey2.o : \
	src/emu/sound/sp0256.h \
	src/mess/drivers/odyssey2.c \
	src/emu/video/i8244.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/ef9340_1.h \
	src/emu/machine/i8243.h \

$(OBJ)/mess/drivers/okean240.o : \
	src/emu/machine/keyboard.h \
	src/emu/cpu/i8085/i8085.h \
	src/mess/drivers/okean240.c \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/ondra.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/ondra.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/ondra.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/onyx.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z8000/z8000.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/onyx.c \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/orao.o : \
	src/mess/drivers/orao.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/orao.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/orao_cas.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/oric.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/machine/mos6551.h \
	src/emu/imagedev/cassette.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/mess/drivers/oric.c \
	src/mess/machine/applefdc.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/ap2_dsk.h \
	src/mess/includes/oric.h \
	src/emu/machine/6522via.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/machine/clock.h \
	src/lib/formats/oric_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/oric_tap.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/orion.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/rk_cas.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/mc146818.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/machine/wd_fdc.h \
	src/emu/video/i8275.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/orion.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/mess/drivers/orion.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/imagedev/cartslot.h \
	src/mess/includes/radio86.h \
	src/lib/formats/smx_dsk.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/8257dma.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/osbexec.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/mess/drivers/osbexec.c \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/6821pia.h \
	src/lib/util/opresolv.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \

$(OBJ)/mess/drivers/osborne1.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/lib/formats/flopimg.h \
	src/mess/includes/osborne1.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/lib/util/opresolv.h \
	src/mess/drivers/osborne1.c \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/osi.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/mess/includes/osi.h \
	src/emu/sound/disc_dev.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/mess/drivers/osi.c \
	src/emu/machine/6821pia.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/machine/6850acia.h \
	src/emu/sound/wavwrite.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/emu/machine/clock.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/drivers/p112.o : \
	src/emu/cpu/z180/z180.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/p112.c \

$(OBJ)/mess/drivers/p2000t.o : \
	src/emu/video/saa5050.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/mess/drivers/p2000t.c \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/p2000t.h \

$(OBJ)/mess/drivers/p8k.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/z80sio.h \
	src/emu/cpu/z8000/z8000.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/z80dma.h \
	src/emu/machine/upd765.h \
	src/emu/machine/keyboard.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/drivers/p8k.c \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/machine/terminal.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/palm.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/ram.h \
	src/mess/machine/mc68328.h \
	src/mess/drivers/palm_dbg.inc \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/mess/drivers/palm.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/palmz22.o : \
	src/emu/machine/s3c2410.h \
	src/emu/cpu/arm7/arm7.h \
	src/mess/machine/smartmed.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/mess/drivers/palmz22.c \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/mess/drivers/partner.o : \
	src/lib/formats/flopimg.h \
	src/mess/drivers/partner.c \
	src/lib/formats/rk_cas.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/video/i8275.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/mess/includes/partner.h \
	src/emu/machine/i8255.h \
	src/mess/includes/radio86.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/machine/8257dma.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/paso1600.o : \
	src/mess/drivers/paso1600.c \
	src/emu/machine/pic8259.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/emu/machine/am9517a.h \

$(OBJ)/mess/drivers/pasogo.o : \
	src/emu/machine/pit8253.h \
	src/emu/sound/speaker.h \
	src/mess/drivers/pasogo.c \
	src/emu/cpu/nec/nec.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/am9517a.h \

$(OBJ)/mess/drivers/pasopia.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/pasopia.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/z80ctc.h \
	src/emu/video/mc6845.h \
	src/mess/includes/pasopia.h \

$(OBJ)/mess/drivers/pasopia7.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/z80ctc.h \
	src/emu/sound/sn76496.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/mess/drivers/pasopia7.c \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/mess/includes/pasopia.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/pb1000.o : \
	src/emu/machine/nvram.h \
	src/emu/cpu/hd61700/hd61700.h \
	src/emu/video/hd44352.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/pb1000.c \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/pc.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/mess/video/pc_aga.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/video/pc_vga.h \
	src/lib/formats/naslite_dsk.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/cpu/i86/i286.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pc_fdc.h \
	src/lib/util/chd.h \
	src/emu/cpu/nec/nec.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/ins8250.h \
	src/emu/sound/sn76496.h \
	src/lib/util/md5.h \
	src/mess/includes/tandy1t.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/cartslot.h \
	src/emu/bus/pc_kbd/keyboards.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/emu/machine/pckeybrd.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/corefile.h \
	src/lib/formats/asst128_dsk.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/emu/machine/pc_lpt.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/includes/pc.h \
	src/lib/util/opresolv.h \
	src/mess/video/pc_t1t.h \
	src/emu/imagedev/floppy.h \
	src/mess/drivers/pc.c \
	src/emu/bus/rs232/ser_mouse.h \
	src/emu/video/pc_cga.h \
	src/mess/includes/europc.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/pc100.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/machine/msm58321.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/i86/i86.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \
	src/mess/drivers/pc100.c \

$(OBJ)/mess/drivers/pc1500.o : \
	src/mess/machine/lh5810.h \
	src/mess/drivers/pc1500.c \
	src/emu/machine/upd1990a.h \
	src/emu/cpu/lh5801/lh5801.h \

$(OBJ)/mess/drivers/pc1512.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/sound/saa1099.h \
	src/emu/machine/buffer.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/emu/video/crtc_ega.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/wd11c00_17.h \
	src/mess/drivers/pc1512.c \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/bus/isa/lpt.h \
	src/lib/formats/upd765_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/sound/2151intf.h \
	src/emu/machine/mc146818.h \
	src/emu/machine/latch.h \
	src/emu/bus/isa/gus.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/emu/machine/dp8390.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/machine/pc_fdc.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/bus/isa/ide.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/machine/ins8250.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/bus/isa/finalchs.h \
	src/emu/imagedev/harddriv.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/pc1640_iga.h \
	src/emu/bus/isa/isa.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/pc1512.h \
	src/emu/bus/isa/ega.h \
	src/emu/machine/wd2010.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/mess/machine/pc1512kb.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/lib/formats/d88_dsk.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/pc2000.o : \
	src/mess/drivers/pc2000.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/sed1520.h \
	src/emu/video/hd44780.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/pc4.o : \
	src/mess/drivers/pc4.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/rp5c01.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/pc4.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/pc6001.o : \
	src/emu/video/mc6847.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/mess/drivers/pc6001.c \
	src/emu/sound/upd7752.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/p6001_cas.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/pc8001.o : \
	src/emu/machine/buffer.h \
	src/emu/video/upd3301.h \
	src/mess/drivers/pc8001.c \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/8257dma.h \
	src/mess/includes/pc8001.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/upd1990a.h \

$(OBJ)/mess/drivers/pc8401a.o : \
	src/emu/bus/rs232/rs232.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/emu/video/sed1330.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/mess/includes/pc8401a.h \
	src/mess/drivers/pc8401a.c \
	src/emu/machine/upd1990a.h \

$(OBJ)/mess/drivers/pc8801.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/drivers/pc8801.c \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/machine/i8214.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/sound/2608intf.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/2203intf.h \
	src/emu/machine/upd1990a.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/pc88va.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/mess/machine/upd71071.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/nec/nec.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/simple_set.h \
	src/emu/machine/i8255.h \
	src/emu/machine/upd765.h \
	src/emu/debug/express.h \
	src/lib/formats/xdf_dsk.h \
	src/emu/debug/debugcpu.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/sound/2203intf.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \
	src/mess/drivers/pc88va.c \

$(OBJ)/mess/drivers/pc9801.o : \
	src/lib/formats/pc98_dsk.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/softfloat/softfloat-macros \
	src/lib/util/corealloc.h \
	src/lib/formats/upd765_dsk.h \
	src/mess/drivers/pc9801.c \
	src/emu/cpu/i86/i286.h \
	src/mess/machine/pc9801_kbd.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/mess/machine/pc9801_118.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/emu/cpu/nec/nec.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/lib/util/simple_set.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/lib/softfloat/milieu.h \
	src/emu/cpu/i386/i386.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/upd7220.h \
	src/emu/debug/express.h \
	src/emu/imagedev/harddriv.h \
	src/emu/debug/debugcpu.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/vtlb.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/machine/pc9801_26.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/s1410.h \
	src/emu/machine/scsihd.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/sound/2608intf.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/2203intf.h \
	src/lib/formats/d88_dsk.h \
	src/emu/machine/upd1990a.h \
	src/lib/util/harddisk.h \
	src/lib/formats/pc98fdi_dsk.h \
	src/emu/sound/ay8910.h \
	src/lib/util/astring.h \
	src/emu/sound/beep.h \
	src/mess/machine/pc9801_cbus.h \
	src/mess/machine/pc9801_86.h \

$(OBJ)/mess/drivers/pce.o : \
	src/emu/cpu/h6280/h6280.h \
	src/lib/util/palette.h \
	src/emu/machine/nvram.h \
	src/emu/sound/msm5205.h \
	src/lib/util/corealloc.h \
	src/mess/includes/pce.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/emu/video/huc6260.h \
	src/lib/util/chd.h \
	src/mess/machine/pce_cd.h \
	src/lib/util/chdcodec.h \
	src/emu/video/huc6270.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/emu/sound/c6280.h \
	src/emu/video/huc6202.h \
	src/emu/bus/pce/pce_rom.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/bus/pce/pce_slot.h \
	src/lib/util/astring.h \
	src/mess/drivers/pce.c \

$(OBJ)/mess/drivers/pce220.o : \
	src/mess/machine/pce220_ser.h \
	src/emu/machine/nvram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/pce220.c \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/pcfx.o : \
	src/emu/video/huc6272.h \
	src/emu/cpu/v810/v810.h \
	src/mess/drivers/pcfx.c \
	src/emu/video/huc6270.h \
	src/emu/video/huc6261.h \

$(OBJ)/mess/drivers/pcm.o : \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/mess/drivers/pcm.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/z80dart.h \
	src/mess/machine/k7659kb.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/pcw.o : \
	src/lib/formats/pc_dsk.h \
	src/lib/formats/flopimg.h \
	src/mess/includes/pcw.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/mess/drivers/pcw.c \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/i8243.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/formats/dsk_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/pcw16.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/mess/includes/pcw16.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/mess/drivers/pcw16.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/ins8250.h \
	src/emu/machine/upd765.h \
	src/emu/machine/pckeybrd.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/pc_lpt.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/pda600.o : \
	src/mess/machine/hd64610.h \
	src/emu/machine/nvram.h \
	src/emu/cpu/z180/z180.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/pda600.c \

$(OBJ)/mess/drivers/pdp1.o : \
	src/mess/drivers/pdp1.c \
	src/mess/includes/pdp1.h \
	src/emu/cpu/pdp1/pdp1.h \
	src/mess/video/crt.h \

$(OBJ)/mess/drivers/pdp11.o : \
	src/mess/drivers/pdp11.c \
	src/mess/machine/rx01.h \
	src/emu/cpu/t11/t11.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/pecom.o : \
	src/emu/sound/cdp1869.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/pecom.c \
	src/mess/includes/pecom.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/pegasus.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/6821pia.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/mess/drivers/pegasus.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/pencil2.o : \
	src/emu/machine/buffer.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/video/tms9928a.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/sn76496.h \
	src/mess/drivers/pencil2.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/pentagon.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/mess/machine/beta.h \
	src/mess/drivers/pentagon.c \
	src/mess/includes/spectrum.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/tzx_cas.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/pes.o : \
	src/emu/sound/tms5220.h \
	src/mess/includes/pes.h \
	src/emu/machine/spchrom.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/machine/keyboard.h \
	src/mess/drivers/pes.c \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/pet.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/includes/pet.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/lib/formats/d82_dsk.h \
	src/mess/drivers/pet.c \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/d80_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/pet/exp.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/c8050.h \
	src/emu/bus/pet/diag264_lb_tape.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/emu/machine/pla.h \
	src/emu/bus/ieee488/c2040.h \
	src/mess/machine/cbm_snqk.h \
	src/lib/util/jedparse.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/mos6530.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/lib/formats/cbm_tap.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/pet/c2n.h \
	src/emu/bus/pet/cass.h \
	src/emu/bus/ieee488/c2040fdc.h \
	src/emu/bus/pet/user.h \
	src/emu/machine/6532riot.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/d64_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/formats/d67_dsk.h \

$(OBJ)/mess/drivers/phc25.o : \
	src/emu/machine/buffer.h \
	src/mess/includes/phc25.h \
	src/emu/video/mc6847.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/phc25_cas.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/phc25.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/phunsy.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/s2650/s2650.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/phunsy.c \
	src/emu/machine/keyboard.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/pimps.o : \
	src/mess/drivers/pimps.c \
	src/emu/machine/keyboard.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/pipbug.o : \
	src/emu/bus/rs232/rs232.h \
	src/mess/drivers/pipbug.c \
	src/emu/cpu/s2650/s2650.h \
	src/emu/machine/keyboard.h \
	src/emu/imagedev/snapquik.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/pitagjr.o : \
	src/mess/drivers/pitagjr.c \
	src/emu/cpu/m6805/m6805.h \

$(OBJ)/mess/drivers/pk8000.o : \
	src/lib/formats/fmsx_cas.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mame/includes/pk8000.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/pk8000.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/pk8020.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/pic8259.h \
	src/mess/includes/pk8020.h \
	src/emu/machine/i8255.h \
	src/mess/drivers/pk8020.c \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/plan80.o : \
	src/mess/drivers/plan80.c \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/drivers/plus4.o : \
	src/emu/cpu/m6502/m7501.h \
	src/emu/bus/vic20/user.h \
	src/lib/util/corealloc.h \
	src/emu/machine/mos8706.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/plus4.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/bus/pet/diag264_lb_tape.h \
	src/emu/cpu/m6502/m6510.h \
	src/emu/machine/pla.h \
	src/mess/machine/cbm_snqk.h \
	src/lib/util/jedparse.h \
	src/emu/imagedev/cartslot.h \
	src/emu/bus/vcs/ctrl.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/mos7360.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/plus4/exp.h \
	src/emu/bus/plus4/user.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/mos6529.h \
	src/mess/drivers/plus4.c \
	src/lib/formats/cbm_tap.h \
	src/emu/bus/pet/c2n.h \
	src/emu/bus/pet/cass.h \
	src/emu/machine/clock.h \
	src/emu/bus/pet/user.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/t6721a.h \

$(OBJ)/mess/drivers/pm68k.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/mess/drivers/pm68k.c \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/pmd85.o : \
	src/lib/formats/pmd_cas.h \
	src/emu/machine/pit8253.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mess/includes/pmd85.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/mess/drivers/pmd85.c \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/pmi80.o : \
	src/mess/drivers/pmi80.c \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/drivers/pocketc.o : \
	src/mess/includes/pc1401.h \
	src/emu/machine/nvram.h \
	src/mess/includes/pc1350.h \
	src/emu/machine/ram.h \
	src/mess/includes/pocketc.h \
	src/mess/drivers/pocketc.c \
	src/emu/cpu/sc61860/sc61860.h \
	src/mess/includes/pc1403.h \
	src/mess/includes/pc1251.h \

$(OBJ)/mess/drivers/pockstat.o : \
	src/emu/cpu/arm7/arm7.h \
	src/mess/drivers/pockstat.c \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7core.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/poisk1.o : \
	src/emu/bus/isa/p1_hdc.h \
	src/emu/bus/isa/xtide.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/mc1502_rom.h \
	src/emu/bus/isa/lpt.h \
	src/mess/includes/poisk1.h \
	src/mess/drivers/poisk1.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/p1_fdc.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/mess/machine/kb_poisk1.h \
	src/emu/cpu/i86/i86.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/xsu_cards.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/isa/mc1502_fdc.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/p1_rom.h \

$(OBJ)/mess/drivers/pokemini.o : \
	src/mess/drivers/pokemini.c \
	src/emu/sound/speaker.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/minx/minx.h \
	src/emu/machine/i2cmem.h \

$(OBJ)/mess/drivers/poly.o : \
	src/emu/video/saa5050.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/mc6854.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/6840ptm.h \
	src/emu/cpu/m6809/m6809.h \
	src/emu/machine/clock.h \
	src/mess/drivers/poly.c \

$(OBJ)/mess/drivers/poly88.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/poly88.h \
	src/mess/drivers/poly88.c \
	src/emu/cpu/i8085/i8085.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/poly880.o : \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/includes/poly880.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/mess/drivers/poly880.c \

$(OBJ)/mess/drivers/portfoli.o : \
	src/emu/machine/buffer.h \
	src/mess/includes/portfoli.h \
	src/mess/drivers/portfoli.c \
	src/emu/machine/nvram.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/latch.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/video/hd61830.h \
	src/emu/machine/i8255.h \
	src/emu/machine/ins8250.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/centronics/ctronics.h \

$(OBJ)/mess/drivers/pp01.o : \
	src/mess/drivers/pp01.c \
	src/emu/machine/pit8253.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/i8255.h \
	src/mess/includes/pp01.h \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/drivers/prestige.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/prestige.c \

$(OBJ)/mess/drivers/primo.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/primo.c \
	src/mess/includes/primo.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/primoptp.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/pro80.o : \
	src/mess/drivers/pro80.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/prof180x.o : \
	src/mess/drivers/prof180x.c \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/includes/prof180x.h \

$(OBJ)/mess/drivers/prof80.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/mess/drivers/prof80.c \
	src/mess/includes/prof80.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/upd765.h \
	src/emu/bus/ecbbus/ecbbus.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/upd1990a.h \
	src/lib/formats/d88_dsk.h \
	src/mess/machine/prof80mmu.h \

$(OBJ)/mess/drivers/psion.o : \
	src/mess/includes/psion.h \
	src/emu/cpu/m6800/m6800.h \
	src/mess/drivers/psion.c \
	src/mess/machine/psion_pack.h \
	src/emu/video/hd44780.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/psx.o : \
	src/lib/util/palette.h \
	src/mess/machine/psxcd.h \
	src/lib/zlib/zconf.h \
	src/mess/machine/psxcard.h \
	src/emu/cpu/psx/gte.h \
	src/lib/util/corealloc.h \
	src/emu/machine/ram.h \
	src/emu/cpu/psx/psx.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/emu/cpu/psx/dma.h \
	src/emu/sound/spu.h \
	src/lib/util/chd.h \
	src/emu/video/psx.h \
	src/lib/util/chdcodec.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/mess/drivers/psx.c \
	src/emu/cpu/psx/irq.h \
	src/mess/machine/psxcport.h \
	src/lib/zlib/zlib.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6805/m6805.h \
	src/emu/cpu/psx/sio.h \
	src/lib/util/hashing.h \
	src/emu/cpu/psx/siodev.h \
	src/emu/sound/spureverb.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/pt68k4.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/corealloc.h \
	src/emu/sound/2151intf.h \
	src/emu/bus/isa/gus.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/softfloat/mamesf.h \
	src/emu/machine/pc_fdc.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/machine/timekpr.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/lib/softfloat/milieu.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/emu/bus/pc_kbd/keyboards.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/bus/isa/finalchs.h \
	src/emu/imagedev/harddriv.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/lib/util/opresolv.h \
	src/emu/machine/n68681.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/s3virge.h \
	src/mess/drivers/pt68k4.c \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/ptcsol.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/sol_cas.h \
	src/emu/machine/keyboard.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/mess/drivers/ptcsol.c \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/ay31015.h \

$(OBJ)/mess/drivers/pulsar.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/msm5832.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/com8116.h \
	src/mess/drivers/pulsar.c \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/pv1000.o : \
	src/mess/drivers/pv1000.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \

$(OBJ)/mess/drivers/pv2000.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/video/tms9928a.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/sn76496.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/pv2000.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/pv9234.o : \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/mess/drivers/pv9234.c \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/mess/drivers/pve500.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/pve500.c \

$(OBJ)/mess/drivers/px4.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/nvram.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/px4.c \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/epson_sio/epson_sio.h \
	src/emu/imagedev/cartslot.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/px8.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/mess/includes/px8.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/epson_sio/epson_sio.h \
	src/mess/drivers/px8.c \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/upd765.h \
	src/emu/bus/epson_sio/pf10.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/pyl601.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/formats/pyldin_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/drivers/pyl601.c \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/ql.o : \
	src/lib/formats/flopimg.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/lib/softfloat/mamesf.h \
	src/mess/video/zx8301.h \
	src/mess/drivers/ql.c \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/imagedev/printer.h \
	src/mess/machine/microdrv.h \
	src/lib/util/opresolv.h \
	src/mess/machine/zx8302.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/mess/includes/ql.h \

$(OBJ)/mess/drivers/qtsbc.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/qtsbc.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/qx10.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/mc146818.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/z80/z80.h \
	src/mess/machine/qx10kbd.h \
	src/emu/machine/i8255.h \
	src/emu/machine/upd765.h \
	src/emu/video/upd7220.h \
	src/emu/bus/rs232/keyboard.h \
	src/emu/machine/keyboard.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/drivers/qx10.c \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/radio86.o : \
	src/lib/formats/rk_cas.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/video/i8275.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/emu/imagedev/cartslot.h \
	src/mess/includes/radio86.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/radio86.c \
	src/emu/machine/8257dma.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/rainbow.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/nvram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/i86/i86.h \
	src/mess/video/vtvideo.h \
	src/lib/util/opresolv.h \
	src/mess/machine/dec_lk201.h \
	src/emu/machine/clock.h \
	src/mess/drivers/rainbow.c \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/ravens.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/s2650/s2650.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/keyboard.h \
	src/mess/drivers/ravens.c \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/rd110.o : \
	src/emu/machine/nvram.h \
	src/emu/machine/ram.h \
	src/emu/cpu/mcs96/mcs96.h \
	src/mess/drivers/rd110.c \
	src/emu/cpu/mcs96/i8x9x.h \

$(OBJ)/mess/drivers/replicator.o : \
	src/mess/drivers/replicator.c \
	src/emu/cpu/avr8/avr8.h \
	src/emu/video/hd44780.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/rex6000.o : \
	src/mess/drivers/rex6000.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/rp5c01.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/snapquik.h \
	src/emu/machine/intelfsh.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/rm380z.o : \
	src/lib/formats/flopimg.h \
	src/mess/drivers/rm380z.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/mess/includes/rm380z.h \

$(OBJ)/mess/drivers/rmnimbus.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/emu/sound/msm5205.h \
	src/lib/formats/upd765_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/er59256.h \
	src/emu/machine/z80sio.h \
	src/emu/machine/wd17xx.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/mess/machine/acb4070.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/drivers/rmnimbus.c \
	src/mess/includes/rmnimbus.h \
	src/mess/machine/s1410.h \
	src/emu/cpu/i86/i186.h \
	src/emu/machine/scsihd.h \
	src/lib/util/hashing.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/scsihle.h \
	src/lib/util/harddisk.h \
	src/emu/sound/ay8910.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/rmt32.o : \
	src/emu/machine/ram.h \
	src/emu/cpu/mcs96/mcs96.h \
	src/emu/cpu/mcs96/i8x9x.h \
	src/mess/drivers/rmt32.c \

$(OBJ)/mess/drivers/rsc55.o : \
	src/emu/machine/ram.h \
	src/emu/cpu/mcs96/mcs96.h \
	src/mess/drivers/rsc55.c \
	src/emu/cpu/mcs96/i8x9x.h \

$(OBJ)/mess/drivers/rt1715.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/video/i8275.h \
	src/mess/drivers/rt1715.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/z80dma.h \
	src/emu/machine/z80dart.h \

$(OBJ)/mess/drivers/rvoice.o : \
	src/emu/machine/mos6551.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/keyboard.h \
	src/mess/drivers/rvoice.c \
	src/emu/machine/clock.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/rx78.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/rx78.c \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/sn76496.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/sacstate.o : \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \
	src/emu/cpu/i8008/i8008.h \
	src/mess/drivers/sacstate.c \

$(OBJ)/mess/drivers/sage2.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/latch.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/lib/softfloat/milieu.h \
	src/mess/drivers/sage2.c \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/mess/includes/sage2.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/samcoupe.o : \
	src/emu/sound/saa1099.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/samcoupe.c \
	src/mess/includes/samcoupe.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/tzx_cas.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/msm6242.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/formats/coupedsk.h \

$(OBJ)/mess/drivers/sapi1.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/emu/cpu/i8085/i8085.h \
	src/mess/drivers/sapi1.c \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/saturn.o : \
	src/emu/bus/saturn/sat_slot.h \
	src/mame/includes/stv.h \
	src/lib/util/palette.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/corealloc.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/lib/util/coreutil.h \
	src/emu/machine/smpc.h \
	src/emu/cpu/scudsp/scudsp.h \
	src/emu/machine/eepromser.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/adsp2100/adsp2100.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/lib/softfloat/milieu.h \
	src/emu/imagedev/chd_cd.h \
	src/emu/machine/eeprom.h \
	src/lib/util/md5.h \
	src/lib/softfloat/softfloat.h \
	src/mess/drivers/saturn.c \
	src/emu/cpu/sh2/sh2.h \
	src/emu/bus/saturn/rom.h \
	src/lib/util/corefile.h \
	src/emu/sound/scsp.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/saturn/dram.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/bus/saturn/bram.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/savia84.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/savia84.c \
	src/emu/machine/i8255.h \

$(OBJ)/mess/drivers/sbc6510.o : \
	src/mess/drivers/sbc6510.c \
	src/emu/cpu/m6502/m6510.h \
	src/emu/cpu/avr8/avr8.h \
	src/emu/machine/keyboard.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/mos6526.h \
	src/emu/machine/terminal.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/sbrain.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/com8116.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/drivers/sbrain.c \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/sc1.o : \
	src/emu/machine/z80pio.h \
	src/mess/drivers/sc1.c \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \

$(OBJ)/mess/drivers/sc2.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/sc2.c \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/scorpion.o : \
	src/mess/drivers/scorpion.c \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/mess/machine/beta.h \
	src/mess/includes/spectrum.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/tzx_cas.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/scv.o : \
	src/mess/audio/upd1771.h \
	src/mess/drivers/scv.c \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/upd7810/upd7810.h \

$(OBJ)/mess/drivers/sdk85.o : \
	src/mess/drivers/sdk85.c \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/i8279.h \

$(OBJ)/mess/drivers/sdk86.o : \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/i8251.h \
	src/mess/drivers/sdk86.c \
	src/emu/cpu/i86/i86.h \
	src/emu/machine/i8279.h \
	src/emu/machine/clock.h \

$(OBJ)/mess/drivers/seattle.o : \
	src/mess/drivers/seattle.c \
	src/emu/cpu/i86/i86.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/segapico.o : \
	src/mame/machine/megacdcd.h \
	src/lib/util/palette.h \
	src/mame/includes/megadriv.h \
	src/emu/machine/nvram.h \
	src/emu/sound/upd7759.h \
	src/emu/sound/2612intf.h \
	src/lib/util/corealloc.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/sound/rf5c68.h \
	src/mame/machine/megavdp.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/util/cdrom.h \
	src/emu/cpu/uml.h \
	src/lib/util/coreutil.h \
	src/lib/util/sha1.h \
	src/emu/cpu/drccache.h \
	src/lib/util/chd.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/ssp1601/ssp1601.h \
	src/lib/util/chdcodec.h \
	src/emu/cpu/sh2/sh2comn.h \
	src/lib/softfloat/milieu.h \
	src/mess/includes/md_cons.h \
	src/emu/imagedev/chd_cd.h \
	src/emu/bus/megadrive/md_slot.h \
	src/emu/sound/sn76496.h \
	src/lib/util/md5.h \
	src/lib/softfloat/softfloat.h \
	src/emu/bus/megadrive/rom.h \
	src/mess/drivers/segapico.c \
	src/emu/cpu/sh2/sh2.h \
	src/emu/machine/lc89510.h \
	src/emu/cpu/drcuml.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/drcfe.h \
	src/emu/sound/cdda.h \
	src/emu/cpu/drcumlsh.h \
	src/lib/util/hashing.h \
	src/mame/machine/megacd.h \
	src/mame/machine/mega32x.h \
	src/lib/util/astring.h \
	src/emu/sound/dac.h \
	src/emu/video/315_5124.h \

$(OBJ)/mess/drivers/selz80.o : \
	src/mess/drivers/selz80.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8279.h \

$(OBJ)/mess/drivers/sg1000.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/mess/drivers/sg1000.c \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/bus/sega8/rom.h \
	src/lib/formats/upd765_dsk.h \
	src/lib/util/corealloc.h \
	src/lib/formats/sf7000_dsk.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/emu/video/tms9928a.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/eepromser.h \
	src/lib/formats/sc3000_bit.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/sound/sn76496.h \
	src/emu/machine/eeprom.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/printer.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/sega8/sega8_slot.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/mess/includes/sg1000.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/sgi_ip2.o : \
	src/emu/bus/rs232/rs232.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/mc146818.h \
	src/lib/softfloat/mamesf.h \
	src/mess/drivers/sgi_ip2.c \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/n68681.h \
	src/emu/machine/terminal.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/sgi_ip6.o : \
	src/emu/cpu/mips/r3000.h \
	src/mess/drivers/sgi_ip6.c \

$(OBJ)/mess/drivers/sh4robot.o : \
	src/emu/cpu/sh4/sh4.h \
	src/mess/drivers/sh4robot.c \

$(OBJ)/mess/drivers/sitcom.o : \
	src/emu/video/dl1416.h \
	src/mess/drivers/sitcom.c \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/drivers/slc1.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/slc1.c \

$(OBJ)/mess/drivers/sm1800.o : \
	src/mess/drivers/sm1800.c \
	src/emu/machine/i8251.h \
	src/emu/video/i8275.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/drivers/smc777.o : \
	src/lib/formats/flopimg.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/smc777.c \
	src/emu/sound/sn76496.h \
	src/emu/video/mc6845.h \
	src/lib/util/opresolv.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/sms.o : \
	src/emu/bus/sms_exp/gender.h \
	src/mess/drivers/sms.c \
	src/emu/bus/sms_ctrl/joypad.h \
	src/emu/bus/sms_ctrl/smsctrl.h \
	src/emu/bus/sega8/rom.h \
	src/emu/bus/sms_ctrl/rfu.h \
	src/emu/bus/sms_ctrl/paddle.h \
	src/emu/bus/sms_exp/smsexp.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/includes/sms.h \
	src/emu/machine/eepromser.h \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/sn76496.h \
	src/emu/sound/2413intf.h \
	src/emu/machine/eeprom.h \
	src/emu/imagedev/cartslot.h \
	src/emu/bus/sms_ctrl/sportsjp.h \
	src/emu/bus/sega8/sega8_slot.h \
	src/emu/bus/sms_ctrl/lphaser.h \
	src/emu/bus/sms_ctrl/sports.h \
	src/emu/video/315_5124.h \

$(OBJ)/mess/drivers/snes.o : \
	src/emu/cpu/superfx/superfx.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/gameboy/gb_slot.h \
	src/emu/bus/snes/snes_carts.h \
	src/emu/bus/snes/sfx.h \
	src/emu/bus/snes/spc7110.h \
	src/emu/cpu/lr35902/lr35902.h \
	src/emu/bus/snes/upd.h \
	src/emu/bus/snes/bsx.h \
	src/emu/cpu/spc700/spc700.h \
	src/mess/video/gb_lcd.h \
	src/emu/bus/snes/event.h \
	src/emu/bus/snes/sa1.h \
	src/mess/drivers/snes.c \
	src/emu/bus/snes/sgb.h \
	src/mame/includes/snes.h \
	src/emu/bus/snes/rom.h \
	src/emu/bus/snes/rom21.h \
	src/emu/bus/gameboy/mbc.h \
	src/emu/bus/snes/sufami.h \
	src/mame/audio/snes_snd.h \
	src/mess/machine/snescx4.h \
	src/emu/bus/snes/snes_slot.h \
	src/mess/audio/gb.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/snes/sdd1.h \
	src/emu/cpu/upd7725/upd7725.h \
	src/emu/bus/gameboy/rom.h \

$(OBJ)/mess/drivers/socrates.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/socrates.c \
	src/mess/audio/socrates.h \

$(OBJ)/mess/drivers/softbox.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/mess/drivers/softbox.c \
	src/lib/util/palette.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/machine/com8116.h \
	src/mess/includes/corvushd.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/softbox.h \
	src/emu/bus/imi7000/imi7000.h \
	src/lib/util/hashing.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/sorcerer.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/mess/includes/sorcerer.h \
	src/emu/bus/centronics/covox.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/sorc_dsk.h \
	src/mess/machine/micropolis.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/sorcerer.c \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/sorc_cas.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/ay31015.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/spc1000.o : \
	src/emu/video/mc6847.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/spc1000.c \
	src/lib/formats/spc1000_cas.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/spec128.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/mess/drivers/spec128.c \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/spectrum.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/tzx_cas.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/special.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/mess/includes/special.h \
	src/lib/formats/rk_cas.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/mess/drivers/special.c \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/lib/formats/smx_dsk.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/specpls3.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/mess/drivers/specpls3.c \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/mess/includes/spectrum.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/tzx_cas.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/formats/dsk_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/spectrum.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/mess/machine/spec_snqk.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/spectrum.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/tzx_cas.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/drivers/spectrum.c \

$(OBJ)/mess/drivers/ssem.o : \
	src/emu/cpu/ssem/ssem.h \
	src/mess/drivers/ssem.c \
	src/emu/imagedev/cartslot.h \

$(OBJ)/mess/drivers/ssystem3.o : \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6522via.h \
	src/mess/includes/ssystem3.h \
	src/mess/drivers/ssystem3.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/stopthie.o : \
	src/emu/cpu/tms0980/tms0980.h \
	src/mess/drivers/stopthie.c \

$(OBJ)/mess/drivers/stratos.o : \
	src/emu/machine/nvram.h \
	src/emu/cpu/m6502/m65c02.h \
	src/mess/drivers/stratos.c \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/mess/drivers/studio2.o : \
	src/mess/includes/studio2.h \
	src/emu/video/resnet.h \
	src/emu/machine/rescap.h \
	src/emu/sound/disc_dev.h \
	src/emu/video/cdp1861.h \
	src/mess/drivers/studio2.c \
	src/emu/sound/cdp1864.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wavwrite.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/emu/sound/beep.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/drivers/sun1.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/softfloat/mamesf.h \
	src/mess/drivers/sun1.c \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/z80dart.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/sun2.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/mess/drivers/sun2.c \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \

$(OBJ)/mess/drivers/sun3.o : \
	src/mess/drivers/sun3.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \

$(OBJ)/mess/drivers/sun4.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/mess/drivers/sun4.c \

$(OBJ)/mess/drivers/super6.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/includes/super6.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/z80pio.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/com8116.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/z80dma.h \
	src/mess/drivers/super6.c \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/super80.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/super80.c \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/super80.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/supercon.o : \
	src/mess/drivers/supercon.c \
	src/emu/cpu/m6502/m6502.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/superslave.o : \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/z80pio.h \
	src/mess/includes/superslave.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/com8116.h \
	src/mess/drivers/superslave.c \
	src/emu/machine/z80dart.h \

$(OBJ)/mess/drivers/supracan.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/mess/drivers/supracan.c \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/mess/drivers/sv8000.o : \
	src/emu/video/mc6847.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/sv8000.c \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/svi318.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/svi_dsk.h \
	src/emu/video/tms9928a.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/lib/formats/svi_cas.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/ins8250.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/emu/sound/wave.h \
	src/mess/includes/svi318.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \
	src/mess/drivers/svi318.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/svision.o : \
	src/mess/drivers/svision.c \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/imagedev/cartslot.h \
	src/mess/includes/svision.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/mess/drivers/svmu.o : \
	src/mess/drivers/svmu.c \
	src/emu/cpu/lc8670/lc8670.h \
	src/emu/sound/speaker.h \
	src/emu/imagedev/snapquik.h \
	src/emu/machine/intelfsh.h \

$(OBJ)/mess/drivers/swtpc.o : \
	src/mess/drivers/swtpc.c \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/sym1.o : \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/mess/drivers/sym1.c \
	src/emu/machine/74145.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6522via.h \
	src/emu/machine/6532riot.h \

$(OBJ)/mess/drivers/sys2900.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/sys2900.c \
	src/emu/cpu/z80/z80.h \

$(OBJ)/mess/drivers/systec.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/systec.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/tandy2k.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/emu/video/crt9021.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/mess/includes/tandy2k.h \
	src/emu/video/crt9007.h \
	src/lib/util/sha1.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/video/crt9212.h \
	src/emu/machine/pic8259.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/md5.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/i86/i86.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/cpu/i86/i186.h \
	src/mess/machine/tandy2kb.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/mess/drivers/tandy2k.c \
	src/lib/util/harddisk.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/tavernie.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/6840ptm.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/clock.h \
	src/mess/drivers/tavernie.c \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/tdv2324.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/util/sha1.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/lib/util/chdcodec.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/drivers/tdv2324.c \
	src/emu/machine/z80dart.h \
	src/emu/video/tms9927.h \
	src/mess/includes/tdv2324.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/tec1.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/mess/drivers/tec1.c \

$(OBJ)/mess/drivers/tek405x.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/video/vector.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/emu/imagedev/cartslot.h \
	src/mess/includes/tek405x.h \
	src/emu/machine/clock.h \
	src/mess/drivers/tek405x.c \

$(OBJ)/mess/drivers/tek410x.o : \
	src/emu/cpu/i86/i86.h \
	src/mess/drivers/tek410x.c \
	src/emu/cpu/i86/i186.h \

$(OBJ)/mess/drivers/terak.o : \
	src/emu/cpu/t11/t11.h \
	src/mess/drivers/terak.c \

$(OBJ)/mess/drivers/test_t400.o : \
	src/mess/drivers/test_t400.c \
	src/emu/cpu/cop400/cop400.h \

$(OBJ)/mess/drivers/thomson.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/mc6843.h \
	src/emu/bus/rs232/rs232.h \
	src/mess/machine/thomflop.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/machine/mos6551.h \
	src/emu/imagedev/cassette.h \
	src/mess/audio/mea8000.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/mc6854.h \
	src/mess/includes/thomson.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/mc6846.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/thom_dsk.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/thom_cas.h \
	src/mess/drivers/thomson.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/ti85.o : \
	src/mess/drivers/ti85.c \
	src/emu/machine/nvram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/snapquik.h \
	src/mess/video/t6a04.h \
	src/mess/includes/ti85.h \

$(OBJ)/mess/drivers/ti89.o : \
	src/emu/machine/nvram.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/mess/includes/ti89.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/intelfsh.h \
	src/mess/drivers/ti89.c \

$(OBJ)/mess/drivers/ti990_10.o : \
	src/mess/machine/ti99/ti990.h \
	src/lib/util/palette.h \
	src/mess/drivers/ti990_10.c \
	src/lib/util/corealloc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/mess/video/911_vdt.h \
	src/lib/util/chdcodec.h \
	src/lib/util/md5.h \
	src/mess/machine/ti99/990_hd.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/ti99/990_tap.h \
	src/emu/cpu/tms9900/tms99com.h \
	src/lib/util/hashing.h \
	src/lib/util/harddisk.h \
	src/emu/cpu/tms9900/ti990_10.h \
	src/lib/util/astring.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/ti990_4.o : \
	src/lib/formats/flopimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/cpu/tms9900/tms9900.h \
	src/mess/video/911_vdt.h \
	src/mess/drivers/ti990_4.c \
	src/lib/util/opresolv.h \
	src/mess/video/733_asr.h \
	src/emu/cpu/tms9900/tms99com.h \
	src/mess/machine/ti99/990_dk.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/ti99_2.o : \
	src/emu/cpu/tms9900/tms9995.h \
	src/mess/drivers/ti99_2.c \
	src/emu/cpu/tms9900/tms99com.h \
	src/emu/machine/tms9901.h \

$(OBJ)/mess/drivers/ti99_4p.o : \
	src/mess/drivers/ti99_4p.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/machine/ti99/peribox.h \
	src/emu/video/v9938.h \
	src/emu/video/tms9928a.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/tms9900/tms9900.h \
	src/mess/machine/ti99/videowrp.h \
	src/mess/machine/ti99/ti99defs.h \
	src/emu/sound/sn76496.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/ti99/joyport.h \
	src/emu/cpu/tms9900/tms99com.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/tms9901.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/ti99_4x.o : \
	src/mess/machine/ti99/gromport.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/machine/ti99/peribox.h \
	src/emu/video/v9938.h \
	src/emu/video/tms9928a.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/tms9900/tms9900.h \
	src/mess/machine/ti99/datamux.h \
	src/mess/machine/ti99/videowrp.h \
	src/mess/machine/ti99/ti99defs.h \
	src/emu/sound/sn76496.h \
	src/mess/machine/ti99/grom.h \
	src/mess/drivers/ti99_4x.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/ti99/joyport.h \
	src/emu/cpu/tms9900/tms99com.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/tms9901.h \

$(OBJ)/mess/drivers/ti99_8.o : \
	src/emu/sound/tms5220.h \
	src/mess/machine/ti99/gromport.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/machine/ti99/peribox.h \
	src/emu/video/v9938.h \
	src/emu/video/tms9928a.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/tms9900/tms9995.h \
	src/mess/machine/ti99/mapper8.h \
	src/emu/machine/spchrom.h \
	src/mess/machine/ti99/videowrp.h \
	src/mess/machine/ti99/ti99defs.h \
	src/emu/sound/sn76496.h \
	src/mess/machine/ti99/grom.h \
	src/mess/machine/ti99/speech8.h \
	src/mess/drivers/ti99_8.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/ti99/joyport.h \
	src/emu/cpu/tms9900/tms99com.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/tms9901.h \

$(OBJ)/mess/drivers/tiki100.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/mess/includes/tiki100.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/formats/tiki100_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/mess/drivers/tiki100.c \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/tim011.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/cpu/z180/z180.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/imd_dsk.h \
	src/mess/drivers/tim011.c \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/tim100.o : \
	src/emu/machine/i8251.h \
	src/emu/video/i8275.h \
	src/mess/drivers/tim100.c \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/drivers/timex.o : \
	src/mess/drivers/timex.c \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/mess/machine/beta.h \
	src/mess/machine/spec_snqk.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/spectrum.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/tzx_cas.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/tk80.o : \
	src/emu/machine/i8255.h \
	src/emu/machine/keyboard.h \
	src/emu/cpu/i8085/i8085.h \
	src/mess/drivers/tk80.c \

$(OBJ)/mess/drivers/tk80bs.o : \
	src/emu/machine/i8255.h \
	src/emu/machine/keyboard.h \
	src/mess/drivers/tk80bs.c \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/drivers/tm990189.o : \
	src/emu/cpu/tms9900/tms9980a.h \
	src/emu/machine/tms9902.h \
	src/mess/drivers/tm990189.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/video/tms9928a.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/tms9900/tms9900.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/tms9900/tms99com.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/tms9901.h \

$(OBJ)/mess/drivers/tmc1800.o : \
	src/emu/video/resnet.h \
	src/emu/machine/rescap.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/video/cdp1861.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/tmc1800.c \
	src/emu/sound/cdp1864.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/lib/formats/ioprocs.h \
	src/mess/includes/tmc1800.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/tmc2000e.o : \
	src/mess/includes/tmc2000e.h \
	src/emu/video/resnet.h \
	src/emu/machine/rescap.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/sound/cdp1864.h \
	src/mess/drivers/tmc2000e.c \
	src/lib/util/coretmpl.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/tmc600.o : \
	src/emu/machine/buffer.h \
	src/emu/sound/cdp1869.h \
	src/mess/drivers/tmc600.c \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/mess/includes/tmc600.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/tricep.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/mess/drivers/tricep.c \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/trs80.o : \
	src/emu/machine/buffer.h \
	src/mess/drivers/trs80.c \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/trs_dsk.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/mess/includes/trs80.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/trs_cas.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/ay31015.h \

$(OBJ)/mess/drivers/trs80m2.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/sound/disc_dev.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/latch.h \
	src/mess/machine/trs80m2kb.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/softfloat/milieu.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/z80dma.h \
	src/lib/softfloat/softfloat.h \
	src/emu/sound/wavwrite.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/mess/includes/trs80m2.h \
	src/lib/formats/d88_dsk.h \
	src/mess/drivers/trs80m2.c \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/drivers/ts802.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/z80dma.h \
	src/emu/machine/keyboard.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/drivers/ts802.c \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/terminal.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/ts803.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/ts803.c \
	src/emu/cpu/z80/z80.h \

$(OBJ)/mess/drivers/ts816.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/mess/drivers/ts816.c \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/tsispch.o : \
	src/emu/machine/i8251.h \
	src/mess/includes/tsispch.h \
	src/emu/machine/pic8259.h \
	src/mess/drivers/tsispch.c \
	src/emu/cpu/i86/i86.h \
	src/emu/machine/keyboard.h \
	src/emu/cpu/upd7725/upd7725.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/tutor.o : \
	src/emu/machine/buffer.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/video/tms9928a.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/tms9900/tms9995.h \
	src/emu/sound/sn76496.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/tutor.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/cpu/tms9900/tms99com.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/tv950.o : \
	src/mess/drivers/tv950.c \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/mess/drivers/tvc.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/tvc/hbf.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/tvc_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/mess/includes/tvc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/formats/wd177x_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/tvc.c \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/tvc/tvc.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/formats/tvc_cas.h \

$(OBJ)/mess/drivers/tx0.o : \
	src/mess/includes/tx0.h \
	src/mess/drivers/tx0.c \
	src/mess/video/crt.h \
	src/emu/cpu/pdp1/tx0.h \

$(OBJ)/mess/drivers/uknc.o : \
	src/mess/drivers/uknc.c \
	src/emu/cpu/t11/t11.h \

$(OBJ)/mess/drivers/unior.o : \
	src/emu/machine/pit8253.h \
	src/mess/drivers/unior.c \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/video/i8275.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/8257dma.h \

$(OBJ)/mess/drivers/unistar.o : \
	src/mess/drivers/unistar.c \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/drivers/univac.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/univac.c \

$(OBJ)/mess/drivers/unixpc.o : \
	src/lib/formats/flopimg.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/lib/util/opresolv.h \
	src/mess/drivers/unixpc.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/ut88.o : \
	src/lib/formats/rk_cas.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mess/drivers/ut88.c \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/ut88.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/uzebox.o : \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/avr8/avr8.h \
	src/mess/drivers/uzebox.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/v1050.o : \
	src/emu/machine/buffer.h \
	src/mess/machine/v1050kb.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/sound/disc_dev.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/lib/util/sha1.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/machine/msm58321.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/sound/wavwrite.h \
	src/emu/video/mc6845.h \
	src/emu/imagedev/harddriv.h \
	src/emu/machine/i8214.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/mess/includes/v1050.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/clock.h \
	src/mess/machine/s1410.h \
	src/emu/machine/scsihd.h \
	src/mess/drivers/v1050.c \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/astring.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/drivers/v6809.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/sound/speaker.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/6840ptm.h \
	src/emu/machine/mm58274c.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/cpu/m6809/m6809.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/clock.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/drivers/v6809.c \

$(OBJ)/mess/drivers/vax11.o : \
	src/mess/drivers/vax11.c \
	src/mess/machine/rx01.h \
	src/emu/cpu/t11/t11.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/vboy.o : \
	src/emu/cpu/v810/v810.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/vboy.c \
	src/mess/audio/vboy.h \

$(OBJ)/mess/drivers/vc4000.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/includes/vc4000.h \
	src/emu/cpu/s2650/s2650.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/cartslot.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/vc4000.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/vcs80.o : \
	src/emu/machine/z80pio.h \
	src/mess/includes/vcs80.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/mess/drivers/vcs80.c \
	src/emu/cpu/z80/z80.h \

$(OBJ)/mess/drivers/vector06.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/mess/includes/vector06.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/drivers/vector06.c \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \

$(OBJ)/mess/drivers/vector4.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \
	src/mess/drivers/vector4.c \

$(OBJ)/mess/drivers/vg5k.o : \
	src/lib/formats/vg5k_cas.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/video/ef9345.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/printer.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/vg5k.c \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/vic10.o : \
	src/mess/includes/vic10.h \
	src/lib/formats/cbm_crt.h \
	src/mess/video/mos6566.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/bus/vic10/exp.h \
	src/emu/bus/pet/diag264_lb_tape.h \
	src/lib/formats/imageutl.h \
	src/emu/sound/mos6581.h \
	src/emu/cpu/m6502/m6510.h \
	src/emu/bus/vcs/ctrl.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/cbm_tap.h \
	src/emu/bus/pet/c2n.h \
	src/emu/bus/pet/cass.h \
	src/mess/drivers/vic10.c \
	src/emu/machine/mos6526.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/vic20.o : \
	src/emu/bus/vic20/user.h \
	src/mess/includes/vic20.h \
	src/mess/drivers/vic20.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/bus/vic20/exp.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/bus/pet/diag264_lb_tape.h \
	src/emu/cpu/m6502/m6510.h \
	src/mess/machine/cbm_snqk.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/mos6560.h \
	src/emu/bus/vcs/ctrl.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6522via.h \
	src/lib/formats/cbm_tap.h \
	src/emu/bus/pet/c2n.h \
	src/emu/bus/pet/cass.h \
	src/emu/bus/pet/user.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/drivers/victor9k.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/latch.h \
	src/mess/includes/victor9k.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/victor9k.c \
	src/lib/formats/imd_dsk.h \
	src/mess/machine/victor9kb.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/6522via.h \
	src/emu/sound/hc55516.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/emu/machine/mc6852.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/vidbrain.o : \
	src/emu/cpu/f8/f8.h \
	src/emu/machine/rescap.h \
	src/emu/sound/disc_dev.h \
	src/emu/machine/ram.h \
	src/mess/video/uv201.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wavwrite.h \
	src/mess/drivers/vidbrain.c \
	src/emu/bus/vidbrain/exp.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/emu/machine/f3853.h \
	src/mess/includes/vidbrain.h \
	src/emu/sound/dac.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/drivers/vii.o : \
	src/emu/cpu/unsp/unsp.h \
	src/lib/formats/imageutl.h \
	src/emu/imagedev/cartslot.h \
	src/mess/drivers/vii.c \
	src/emu/machine/i2cmem.h \

$(OBJ)/mess/drivers/vip.o : \
	src/emu/bus/vip/exp.h \
	src/emu/machine/rescap.h \
	src/emu/bus/vip/byteio.h \
	src/emu/sound/disc_dev.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/video/cdp1861.h \
	src/lib/formats/cassimg.h \
	src/emu/bus/vip/vp620.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/wavwrite.h \
	src/emu/machine/keyboard.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/mess/drivers/vip.c \
	src/mess/includes/vip.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/drivers/vixen.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/sound/disc_dev.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/wavwrite.h \
	src/mess/drivers/vixen.c \
	src/mess/includes/vixen.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/emu/machine/i8155.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/drivers/vk100.o : \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/i8251.h \
	src/mess/drivers/vk100.c \
	src/emu/machine/com8116.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/votrpss.o : \
	src/emu/machine/pit8253.h \
	src/emu/bus/rs232/rs232.h \
	src/mess/drivers/votrpss.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/sound/samples.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/clock.h \
	src/emu/machine/terminal.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/votrtnt.o : \
	src/emu/bus/rs232/rs232.h \
	src/mess/drivers/votrtnt.c \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/machine/6850acia.h \
	src/emu/sound/samples.h \
	src/emu/machine/clock.h \

$(OBJ)/mess/drivers/vt100.o : \
	src/emu/bus/rs232/rs232.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/mess/drivers/vt100.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/com8116.h \
	src/emu/cpu/i8085/i8085.h \
	src/mess/video/vtvideo.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/vt220.o : \
	src/emu/machine/ram.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/mess/drivers/vt220.c \

$(OBJ)/mess/drivers/vt240.o : \
	src/emu/machine/ram.h \
	src/emu/cpu/t11/t11.h \
	src/emu/video/upd7220.h \
	src/emu/cpu/i8085/i8085.h \
	src/mess/drivers/vt240.c \

$(OBJ)/mess/drivers/vt320.o : \
	src/mess/drivers/vt320.c \
	src/emu/machine/ram.h \
	src/emu/cpu/mcs51/mcs51.h \

$(OBJ)/mess/drivers/vt520.o : \
	src/emu/machine/ram.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/mess/drivers/vt520.c \

$(OBJ)/mess/drivers/vta2000.o : \
	src/mess/drivers/vta2000.c \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/drivers/vtech1.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/vtech1_dsk.h \
	src/emu/video/mc6847.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/imageutl.h \
	src/emu/imagedev/cartslot.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/vt_cas.h \
	src/mess/drivers/vtech1.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/vtech2.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/mess/includes/vtech2.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/vt_cas.h \
	src/emu/sound/wave.h \
	src/mess/drivers/vtech2.c \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/wangpc.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/mess/machine/wangpckb.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/machine/latch.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/mess/drivers/wangpc.c \
	src/emu/bus/wangpc/wangpc.h \
	src/emu/machine/im6402.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/sound/sn76496.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/i86/i86.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/includes/wangpc.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/mc2661.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/wicat.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/x2212.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/video/i8275x.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/z8000/z8000.h \
	src/emu/machine/im6402.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/mess/drivers/wicat.c \
	src/emu/cpu/8x300/8x300.h \
	src/emu/machine/mm58274c.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/6522via.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/mc2661.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/wswan.o : \
	src/emu/machine/nvram.h \
	src/mess/drivers/wswan.c \
	src/mess/includes/wswan.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/v30mz/v30mz.h \

$(OBJ)/mess/drivers/x07.o : \
	src/lib/formats/x07_cas.h \
	src/lib/util/corealloc.h \
	src/mess/drivers/x07.c \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/mess/includes/x07.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \
	src/emu/imagedev/printer.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/x1.o : \
	src/lib/formats/flopimg.h \
	src/emu/sound/2151intf.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/includes/x1.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/mess/drivers/x1.c \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/z80dma.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/x1_tap.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/drivers/x1twin.o : \
	src/emu/cpu/h6280/h6280.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/emu/machine/nvram.h \
	src/emu/sound/msm5205.h \
	src/emu/sound/2151intf.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/includes/pce.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/util/cdrom.h \
	src/mess/includes/x1.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/video/huc6260.h \
	src/emu/machine/wd17xx.h \
	src/lib/util/chd.h \
	src/emu/cpu/z80/z80.h \
	src/mess/machine/pce_cd.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/z80ctc.h \
	src/emu/imagedev/chd_cd.h \
	src/emu/machine/z80dma.h \
	src/lib/util/md5.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/mess/drivers/x1twin.c \
	src/emu/sound/wave.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/machine/z80dart.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/lib/formats/x1_tap.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \
	src/emu/bus/pce/pce_slot.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/x68k.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/mess/machine/x68k_kbd.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/nvram.h \
	src/mess/machine/x68k_hdc.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/bus/x68k/x68k_scsiext.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/sound/2151intf.h \
	src/lib/formats/upd765_dsk.h \
	src/mess/includes/x68k.h \
	src/lib/util/corealloc.h \
	src/emu/machine/ram.h \
	src/emu/bus/x68k/x68k_neptunex.h \
	src/lib/util/sha1.h \
	src/mess/machine/hd63450.h \
	src/mess/drivers/x68k.c \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/mess/machine/mb89352.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/lib/softfloat/milieu.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/mc68901.h \
	src/lib/util/md5.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/emu/bus/rs232/keyboard.h \
	src/emu/machine/keyboard.h \
	src/lib/formats/xdf_dsk.h \
	src/emu/imagedev/harddriv.h \
	src/emu/machine/rp5c15.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/machine/8530scc.h \
	src/emu/sound/okim6258.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/x68k/x68kexp.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/scsihd.h \
	src/lib/formats/dim_dsk.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/drivers/xavix.o : \
	src/emu/cpu/g65816/g65816.h \
	src/mess/drivers/xavix.c \
	src/emu/cpu/m6502/m6502.h \
	src/emu/cpu/g65816/g65816cm.h \

$(OBJ)/mess/drivers/xerox820.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/mess/drivers/xerox820.c \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/lib/util/sha1.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/com8116.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/cpu/i86/i86.h \
	src/emu/machine/keyboard.h \
	src/mess/machine/sa1403d.h \
	src/mess/includes/xerox820.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/emu/machine/scsihd.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/astring.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/xor100.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/includes/xor100.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/latch.h \
	src/mess/drivers/xor100.c \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/bus/s100/s100.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/com8116.h \
	src/emu/machine/z80ctc.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/drivers/ymmu100.o : \
	src/emu/cpu/h83002/h8.h \
	src/emu/video/hd44780.h \
	src/mess/drivers/ymmu100.c \

$(OBJ)/mess/drivers/z100.o : \
	src/lib/formats/flopimg.h \
	src/mess/drivers/z100.c \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/6821pia.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \

$(OBJ)/mess/drivers/z1013.o : \
	src/emu/machine/z80pio.h \
	src/mess/drivers/z1013.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/drivers/z80dev.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/mess/drivers/z80dev.c \

$(OBJ)/mess/drivers/z80ne.o : \
	src/lib/formats/flopimg.h \
	src/mess/drivers/z80ne.c \
	src/emu/video/mc6847.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/z80ne_dsk.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/kr2376.h \
	src/lib/util/opresolv.h \
	src/mess/includes/z80ne.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/ay31015.h \

$(OBJ)/mess/drivers/z88.o : \
	src/mess/drivers/z88.c \
	src/mess/includes/z88.h \
	src/emu/bus/z88/flash.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80.h \
	src/mess/machine/upd65031.h \
	src/emu/bus/z88/z88.h \
	src/emu/machine/intelfsh.h \
	src/emu/bus/z88/ram.h \
	src/emu/bus/z88/rom.h \

$(OBJ)/mess/drivers/z9001.o : \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/keyboard.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/drivers/z9001.c \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/zaurus.o : \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7core.h \
	src/mess/drivers/zaurus.c \
	src/mame/machine/pxa255.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/mess/drivers/zexall.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/zexall.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/zorba.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/i8251.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/video/i8275x.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/z80dma.h \
	src/emu/machine/keyboard.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/cpu/m6805/m6805.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \
	src/mess/drivers/zorba.c \

$(OBJ)/mess/drivers/zrt80.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/drivers/zrt80.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/ins8250.h \
	src/emu/video/mc6845.h \
	src/emu/machine/keyboard.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/drivers/zsbc3.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/mess/drivers/zsbc3.c \
	src/emu/machine/terminal.h \

$(OBJ)/mess/drivers/zx.o : \
	src/mess/drivers/zx.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/zx81_p.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/zx.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/ti99/990_dk.o : \
	src/lib/formats/flopimg.h \
	src/mess/machine/ti99/990_dk.c \
	src/emu/imagedev/flopdrv.h \
	src/lib/util/opresolv.h \
	src/mess/machine/ti99/990_dk.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/ti99/990_hd.o : \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/lib/util/md5.h \
	src/mess/machine/ti99/990_hd.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/ti99/990_hd.c \
	src/lib/util/hashing.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/ti99/990_tap.o : \
	src/mess/machine/ti99/990_tap.c \
	src/mess/machine/ti99/990_tap.h \

$(OBJ)/mess/machine/ti99/bwg.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/machine/ti99/peribox.h \
	src/emu/machine/wd_fdc.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/wd17xx.h \
	src/lib/formats/wd177x_dsk.h \
	src/mess/machine/ti99/bwg.h \
	src/lib/formats/ti99_dsk.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/bwg.c \
	src/emu/machine/mm58274c.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/machine/ti99/datamux.o : \
	src/mess/machine/ti99/datamux.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/datamux.c \

$(OBJ)/mess/machine/ti99/evpc.o : \
	src/mess/machine/ti99/peribox.h \
	src/mess/machine/ti99/evpc.h \
	src/mess/machine/ti99/evpc.c \
	src/mess/machine/ti99/ti99defs.h \

$(OBJ)/mess/machine/ti99/genboard.o : \
	src/emu/video/v9938.h \
	src/emu/cpu/tms9900/tms9995.h \
	src/mess/machine/ti99/ti99defs.h \
	src/emu/machine/mm58274c.h \
	src/mess/machine/ti99/genboard.h \
	src/mess/machine/ti99/genboard.c \
	src/emu/cpu/tms9900/tms99com.h \

$(OBJ)/mess/machine/ti99/grom.o : \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/grom.h \
	src/mess/machine/ti99/grom.c \

$(OBJ)/mess/machine/ti99/gromport.o : \
	src/mess/machine/ti99/gromport.h \
	src/lib/util/unzip.h \
	src/lib/util/corealloc.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/grom.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/xmlfile.h \
	src/mess/machine/ti99/gromport.c \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/ti99/handset.o : \
	src/mess/machine/ti99/handset.h \
	src/mess/machine/ti99/handset.c \
	src/mess/machine/ti99/joyport.h \
	src/emu/machine/tms9901.h \

$(OBJ)/mess/machine/ti99/hfdc.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/mess/machine/ti99/hfdc.h \
	src/lib/util/corealloc.h \
	src/mess/machine/ti99/peribox.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/util/chd.h \
	src/mess/machine/ti99/ti99_hd.h \
	src/lib/util/chdcodec.h \
	src/lib/formats/ti99_dsk.h \
	src/mess/machine/ti99/ti99defs.h \
	src/lib/formats/imageutl.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/emu/machine/mm58274c.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/util/hashing.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/mess/machine/smc92x4.h \
	src/lib/util/astring.h \
	src/mess/machine/ti99/hfdc.c \

$(OBJ)/mess/machine/ti99/horizon.o : \
	src/mess/machine/ti99/horizon.c \
	src/mess/machine/ti99/peribox.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/horizon.h \

$(OBJ)/mess/machine/ti99/hsgpl.o : \
	src/emu/machine/at29040a.h \
	src/mess/machine/ti99/peribox.h \
	src/mess/machine/ti99/hsgpl.h \
	src/mess/machine/ti99/hsgpl.c \
	src/mess/machine/ti99/ti99defs.h \

$(OBJ)/mess/machine/ti99/joyport.o : \
	src/mess/machine/ti99/joyport.c \
	src/mess/machine/ti99/handset.h \
	src/mess/machine/ti99/joyport.h \
	src/mess/machine/ti99/mecmouse.h \

$(OBJ)/mess/machine/ti99/mapper8.o : \
	src/mess/machine/ti99/peribox.h \
	src/mess/machine/ti99/mapper8.h \
	src/mess/machine/ti99/mapper8.c \
	src/mess/machine/ti99/ti99defs.h \

$(OBJ)/mess/machine/ti99/mecmouse.o : \
	src/mess/machine/ti99/mecmouse.c \
	src/mess/machine/ti99/joyport.h \
	src/mess/machine/ti99/mecmouse.h \

$(OBJ)/mess/machine/ti99/memex.o : \
	src/mess/machine/ti99/peribox.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/memex.c \
	src/mess/machine/ti99/memex.h \

$(OBJ)/mess/machine/ti99/myarcmem.o : \
	src/mess/machine/ti99/peribox.h \
	src/mess/machine/ti99/myarcmem.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/myarcmem.c \

$(OBJ)/mess/machine/ti99/p_code.o : \
	src/mess/machine/ti99/p_code.h \
	src/mess/machine/ti99/p_code.c \
	src/mess/machine/ti99/peribox.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/grom.h \

$(OBJ)/mess/machine/ti99/peribox.o : \
	src/emu/machine/at29040a.h \
	src/mess/machine/ti99/tn_usbsm.h \
	src/mess/machine/ti99/peribox.c \
	src/emu/sound/tms5220.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/mess/machine/ti99/p_code.h \
	src/emu/machine/tms9902.h \
	src/mess/machine/ti99/hfdc.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/machine/ti99/tn_ide.h \
	src/lib/util/corealloc.h \
	src/mess/machine/smartmed.h \
	src/emu/machine/rtc65271.h \
	src/mess/machine/ti99/peribox.h \
	src/mess/machine/ti99/evpc.h \
	src/mess/machine/ti99/hsgpl.h \
	src/mess/machine/ti99/ti32kmem.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/machine/wd_fdc.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/wd17xx.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/util/chd.h \
	src/mess/machine/ti99/ti99_hd.h \
	src/emu/machine/spchrom.h \
	src/mess/machine/ti99/bwg.h \
	src/lib/util/chdcodec.h \
	src/mess/machine/ti99/myarcmem.h \
	src/lib/formats/ti99_dsk.h \
	src/lib/formats/imageutl.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/grom.h \
	src/lib/util/md5.h \
	src/mess/machine/ti99/spchsyn.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/machine/mm58274c.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/strata.h \
	src/mess/machine/ti99/ti_fdc.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/ti99/samsmem.h \
	src/mess/machine/ti99/ti_rs232.h \
	src/mess/machine/ti99/memex.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/mess/machine/smc92x4.h \
	src/lib/util/astring.h \
	src/mess/machine/ti99/horizon.h \

$(OBJ)/mess/machine/ti99/samsmem.o : \
	src/mess/machine/ti99/peribox.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/samsmem.c \
	src/mess/machine/ti99/samsmem.h \

$(OBJ)/mess/machine/ti99/spchsyn.o : \
	src/emu/sound/tms5220.h \
	src/mess/machine/ti99/peribox.h \
	src/emu/machine/spchrom.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/spchsyn.c \
	src/mess/machine/ti99/spchsyn.h \
	src/emu/sound/wave.h \

$(OBJ)/mess/machine/ti99/speech8.o : \
	src/emu/sound/tms5220.h \
	src/emu/machine/spchrom.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/speech8.h \
	src/emu/sound/wave.h \
	src/mess/machine/ti99/speech8.c \

$(OBJ)/mess/machine/ti99/ti32kmem.o : \
	src/mess/machine/ti99/peribox.h \
	src/mess/machine/ti99/ti32kmem.h \
	src/mess/machine/ti99/ti32kmem.c \
	src/mess/machine/ti99/ti99defs.h \

$(OBJ)/mess/machine/ti99/ti990.o : \
	src/mess/machine/ti99/ti990.h \
	src/mess/machine/ti99/ti990.c \

$(OBJ)/mess/machine/ti99/ti99_hd.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/mess/machine/ti99/ti99_hd.c \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/util/chd.h \
	src/mess/machine/ti99/ti99_hd.h \
	src/lib/util/chdcodec.h \
	src/lib/formats/imageutl.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/util/hashing.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/mess/machine/smc92x4.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/ti99/ti_fdc.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/machine/ti99/peribox.h \
	src/emu/machine/wd_fdc.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/wd17xx.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/ti99_dsk.h \
	src/mess/machine/ti99/ti99defs.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/machine/ti99/ti_fdc.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/ti99/ti_fdc.c \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/machine/ti99/ti_rs232.o : \
	src/emu/machine/tms9902.h \
	src/mess/machine/ti99/ti_rs232.c \
	src/mess/machine/ti99/peribox.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/ti99/ti_rs232.h \

$(OBJ)/mess/machine/ti99/tn_ide.o : \
	src/mess/machine/ti99/tn_ide.c \
	src/lib/util/palette.h \
	src/mess/machine/ti99/tn_ide.h \
	src/lib/util/corealloc.h \
	src/mess/machine/ti99/peribox.h \
	src/emu/machine/rtc65271.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/mess/machine/ti99/ti99_hd.h \
	src/lib/util/chdcodec.h \
	src/mess/machine/ti99/ti99defs.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/ti99/tn_usbsm.o : \
	src/mess/machine/ti99/tn_usbsm.h \
	src/mess/machine/smartmed.h \
	src/mess/machine/ti99/peribox.h \
	src/mess/machine/ti99/ti99defs.h \
	src/mess/machine/strata.h \
	src/mess/machine/ti99/tn_usbsm.c \

$(OBJ)/mess/machine/ti99/videowrp.o : \
	src/emu/video/v9938.h \
	src/emu/video/tms9928a.h \
	src/mess/machine/ti99/videowrp.h \
	src/mess/machine/ti99/ti99defs.h \
	src/emu/sound/sn76496.h \
	src/mess/machine/ti99/videowrp.c \

$(OBJ)/mess/machine/68561mpcc.o : \
	src/mess/machine/68561mpcc.h \
	src/mess/machine/68561mpcc.c \

$(OBJ)/mess/machine/6883sam.o : \
	src/mess/machine/6883sam.c \
	src/mess/machine/6883sam.h \

$(OBJ)/mess/machine/8530scc.o : \
	src/mess/machine/8530scc.c \
	src/mess/machine/8530scc.h \

$(OBJ)/mess/machine/a7800.o : \
	src/mess/machine/a7800.c \
	src/emu/machine/rescap.h \
	src/mess/includes/a7800.h \
	src/emu/sound/tiaintf.h \
	src/emu/sound/tiasound.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6532riot.h \
	src/emu/sound/pokey.h \

$(OBJ)/mess/machine/abc1600mac.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/mess/machine/abc1600mac.c \
	src/mess/machine/abc1600mac.h \

$(OBJ)/mess/machine/abc80kb.o : \
	src/mess/machine/abc80kb.h \
	src/mess/machine/abc80kb.c \
	src/emu/cpu/mcs48/mcs48.h \

$(OBJ)/mess/machine/ac1.o : \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/ac1.h \
	src/mess/machine/ac1.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/acb4070.o : \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/lib/util/md5.h \
	src/mess/machine/acb4070.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/acb4070.c \
	src/emu/machine/scsihd.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/advision.o : \
	src/mess/machine/advision.c \
	src/mess/includes/advision.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/aim65.o : \
	src/mess/machine/aim65.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/video/dl1416.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/aim65.h \
	src/emu/machine/6821pia.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6522via.h \
	src/emu/machine/6532riot.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/amigacd.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/mess/machine/amigacd.c \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/mame/includes/amiga.h \
	src/lib/util/chd.h \
	src/emu/machine/6526cia.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/amigafdc.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/lib/softfloat/milieu.h \
	src/emu/machine/scsidev.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/wd33c93.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/msm6242.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/scsihle.h \
	src/emu/machine/matsucd.h \
	src/lib/formats/d88_dsk.h \
	src/mess/machine/amigacd.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/amigacrt.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/latch.h \
	src/lib/formats/imd_dsk.h \
	src/mame/includes/amiga.h \
	src/lib/softfloat/mamesf.h \
	src/emu/machine/6526cia.h \
	src/emu/machine/amigafdc.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/mess/machine/amigacrt.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/mess/machine/amigacrt.c \
	src/emu/machine/msm6242.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/machine/amigakbd.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/latch.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/mame/includes/amiga.h \
	src/emu/machine/6526cia.h \
	src/emu/machine/amigafdc.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/machine/amigakbd.c \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/amigakbd.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/msm6242.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/machine/amstr_pc.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/video/pc_vga.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/machine/ins8250.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/emu/machine/pc_lpt.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/includes/pc.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/mess/machine/amstr_pc.c \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/includes/amstr_pc.h \

$(OBJ)/mess/machine/amstrad.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/cpc/cpcexp.h \
	src/mess/machine/amstrad.c \
	src/emu/sound/sp0256.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/mess/includes/amstrad.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/bus/cpc/mface2.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/cpc/cpc_rom.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/cpc/cpc_ssa1.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/cpc/cpc_pds.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/machine/apollo.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/mess/machine/apollo_kbd.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/sound/2151intf.h \
	src/emu/bus/isa/gus.h \
	src/emu/bus/isa/ne2000.h \
	src/mess/includes/apollo.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/mess/machine/apollo.c \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/emu/machine/dp8390.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/machine/pc_fdc.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/lib/softfloat/milieu.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/bus/isa/sc499.h \
	src/emu/machine/keyboard.h \
	src/emu/bus/isa/finalchs.h \
	src/emu/imagedev/harddriv.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/isa/3c505.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/lib/util/opresolv.h \
	src/emu/machine/n68681.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/clock.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/omti8621.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/terminal.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/apollo_dbg.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/mess/machine/apollo_kbd.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/sound/2151intf.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/bus/isa/gus.h \
	src/emu/bus/isa/ne2000.h \
	src/mess/includes/apollo.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/lib/softfloat/milieu.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/lib/softfloat/softfloat.h \
	src/emu/cpu/m68000/m68kcpu.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/machine/keyboard.h \
	src/emu/bus/isa/finalchs.h \
	src/emu/imagedev/harddriv.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/lib/util/opresolv.h \
	src/emu/machine/n68681.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/apollo_dbg.c \
	src/emu/machine/clock.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/terminal.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/emu/sound/beep.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/apollo_kbd.o : \
	src/mess/machine/apollo_kbd.h \
	src/mess/machine/apollo_kbd.c \
	src/emu/sound/beep.h \

$(OBJ)/mess/machine/appldriv.o : \
	src/lib/formats/flopimg.h \
	src/emu/imagedev/flopdrv.h \
	src/mess/machine/appldriv.h \
	src/mess/machine/appldriv.c \
	src/lib/formats/ap2_dsk.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/apple1.o : \
	src/mess/includes/apple1.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/6821pia.h \
	src/mess/machine/apple1.c \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/apple2.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/mess/machine/appldriv.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/bus/a2bus/a2eauxslot.h \
	src/mess/machine/applefdc.h \
	src/mess/machine/sonydriv.h \
	src/mess/machine/apple2.c \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/ap2_dsk.h \
	src/lib/util/opresolv.h \
	src/mess/machine/kb3600.h \
	src/mess/includes/apple2.h \
	src/emu/bus/a2bus/a2bus.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/apple2gs.o : \
	src/lib/formats/flopimg.h \
	src/emu/cpu/m6502/m5074x.h \
	src/mess/includes/apple2gs.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/machine/nvram.h \
	src/lib/util/corealloc.h \
	src/emu/cpu/m6502/m740.h \
	src/emu/sound/es5503.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/bus/a2bus/a2eauxslot.h \
	src/mess/machine/applefdc.h \
	src/mess/machine/sonydriv.h \
	src/mess/machine/apple2gs.c \
	src/mess/machine/8530scc.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/mess/machine/kb3600.h \
	src/mess/includes/apple2.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/a2bus/a2bus.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/apple3.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/mess/machine/apple3.c \
	src/emu/machine/mos6551.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/mess/machine/appldriv.h \
	src/emu/cpu/m6502/m65c02.h \
	src/mess/includes/apple3.h \
	src/emu/bus/a2bus/a2eauxslot.h \
	src/mess/machine/applefdc.h \
	src/emu/machine/mm58167.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/ap2_dsk.h \
	src/emu/machine/6522via.h \
	src/lib/util/opresolv.h \
	src/mess/machine/kb3600.h \
	src/emu/machine/clock.h \
	src/mess/includes/apple2.h \
	src/emu/bus/a2bus/a2bus.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/applefdc.o : \
	src/mess/machine/applefdc.c \
	src/mess/machine/applefdc.h \

$(OBJ)/mess/machine/apricotkb.o : \
	src/mess/machine/apricotkb.c \
	src/mess/machine/apricotkb.h \

$(OBJ)/mess/machine/at.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/bus/pci/i82371sb.h \
	src/emu/bus/pci/northbridge.h \
	src/mess/machine/cs8221.h \
	src/emu/machine/wd11c00_17.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/sound/2151intf.h \
	src/emu/cpu/i86/i286.h \
	src/emu/bus/pci/pci.h \
	src/emu/bus/isa/gus.h \
	src/emu/machine/latch.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/emu/machine/at_keybc.h \
	src/mess/machine/at.c \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/bus/pci/i82439tx.h \
	src/emu/bus/pci/southbridge.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/lib/util/simple_set.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/mess/includes/at.h \
	src/lib/softfloat/milieu.h \
	src/emu/cpu/i386/i386.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/machine/ins8250.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/debug/express.h \
	src/emu/bus/isa/finalchs.h \
	src/emu/imagedev/harddriv.h \
	src/emu/debug/debugcpu.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/emu/cpu/vtlb.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/ds128x.h \
	src/emu/machine/wd2010.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/pc_lpt.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/pci/i82371ab.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/ataricrt.o : \
	src/emu/machine/rescap.h \
	src/emu/machine/ram.h \
	src/mame/includes/atari.h \
	src/emu/machine/6821pia.h \
	src/mess/machine/ataricrt.c \
	src/emu/sound/pokey.h \

$(OBJ)/mess/machine/atarifdc.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/mess/machine/atarifdc.c \
	src/mess/machine/atarifdc.h \
	src/emu/imagedev/flopdrv.h \
	src/mame/includes/atari.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/atari_dsk.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/pokey.h \

$(OBJ)/mess/machine/b2m.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/includes/b2m.h \
	src/mess/machine/b2m.c \

$(OBJ)/mess/machine/bbc.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/video/saa5050.h \
	src/emu/sound/tms5220.h \
	src/mess/includes/bbc.h \
	src/mess/machine/i8271.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/mess/machine/bbc.c \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/upd7002.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/spchrom.h \
	src/emu/machine/mc6854.h \
	src/emu/sound/sn76496.h \
	src/emu/machine/6850acia.h \
	src/emu/video/mc6845.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/bebox.o : \
	src/emu/machine/pit8253.h \
	src/mess/includes/bebox.h \
	src/lib/formats/flopimg.h \
	src/mess/machine/bebox.c \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/video/pc_vga.h \
	src/emu/machine/mc146818.h \
	src/emu/bus/pci/pci.h \
	src/emu/machine/ram.h \
	src/emu/machine/atadev.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/53c810.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/idectrl.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/ins8250.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/opresolv.h \
	src/emu/machine/intelfsh.h \
	src/mess/video/cirrus.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/d88_dsk.h \
	src/emu/cpu/powerpc/ppc.h \

$(OBJ)/mess/machine/beta.o : \
	src/lib/formats/flopimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/mess/machine/beta.h \
	src/mess/machine/beta.c \
	src/lib/util/opresolv.h \
	src/lib/formats/trd_dsk.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/bk.o : \
	src/lib/util/corealloc.h \
	src/mess/includes/bk.h \
	src/mess/machine/bk.c \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/c65.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/m6502/m4510.h \
	src/emu/machine/6526cia.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/sound/mos6581.h \
	src/emu/imagedev/cartslot.h \
	src/emu/imagedev/snapquik.h \
	src/mess/includes/c65.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/mess/video/vic4567.h \
	src/mess/machine/c65.c \
	src/lib/formats/cbm_tap.h \
	src/emu/machine/mos6526.h \
	src/emu/cpu/m6502/m65ce02.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/cbm_snqk.o : \
	src/mess/machine/cbm_snqk.h \
	src/emu/imagedev/snapquik.h \
	src/mess/machine/cbm_snqk.c \

$(OBJ)/mess/machine/cgenie.o : \
	src/lib/formats/flopimg.h \
	src/mess/includes/cgenie.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/cartslot.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/dac.h \
	src/mess/machine/cgenie.c \

$(OBJ)/mess/machine/coco.o : \
	src/mess/machine/coco_vhd.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/machine/coco.c \
	src/mess/includes/coco.h \
	src/lib/formats/coco_cas.h \
	src/lib/util/simple_set.h \
	src/emu/machine/6821pia.h \
	src/emu/bus/coco/cococart.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/coco12.o : \
	src/mess/machine/coco_vhd.h \
	src/lib/util/corealloc.h \
	src/emu/video/mc6847.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/coco.h \
	src/mess/machine/coco12.c \
	src/mess/machine/6883sam.h \
	src/emu/machine/6821pia.h \
	src/mess/includes/coco12.h \
	src/emu/bus/coco/cococart.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/coco3.o : \
	src/mess/machine/coco_vhd.h \
	src/lib/util/corealloc.h \
	src/emu/video/mc6847.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/mess/includes/coco3.h \
	src/mess/machine/coco3.c \
	src/lib/formats/cassimg.h \
	src/mess/includes/coco.h \
	src/mess/machine/6883sam.h \
	src/emu/machine/6821pia.h \
	src/mess/video/gime.h \
	src/emu/bus/coco/cococart.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/coco_vhd.o : \
	src/mess/machine/coco_vhd.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/mess/machine/coco_vhd.c \
	src/lib/formats/cassimg.h \
	src/mess/includes/coco.h \
	src/emu/machine/6821pia.h \
	src/emu/bus/coco/cococart.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/coleco.o : \
	src/mess/machine/coleco.c \
	src/mess/machine/coleco.h \

$(OBJ)/mess/machine/compiskb.o : \
	src/emu/sound/speaker.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/mess/machine/compiskb.h \
	src/mess/machine/compiskb.c \

$(OBJ)/mess/machine/concept.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/mos6551.h \
	src/lib/softfloat/mamesf.h \
	src/mess/machine/concept_exp.h \
	src/mess/includes/concept.h \
	src/lib/softfloat/milieu.h \
	src/mess/machine/concept.c \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/mm58274c.h \
	src/emu/machine/6522via.h \
	src/emu/machine/clock.h \

$(OBJ)/mess/machine/concept_exp.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/lib/util/chd.h \
	src/mess/machine/concept_exp.h \
	src/lib/util/chdcodec.h \
	src/mess/includes/corvushd.h \
	src/lib/util/md5.h \
	src/mess/machine/concept_exp.c \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/util/hashing.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/corvushd.o : \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/mess/includes/corvushd.h \
	src/lib/util/md5.h \
	src/mess/machine/corvushd.c \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/cs4031.o : \
	src/emu/machine/pit8253.h \
	src/mess/machine/cs4031.c \
	src/emu/machine/mc146818.h \
	src/emu/machine/ram.h \
	src/emu/machine/at_keybc.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/machine/ds128x.h \
	src/emu/machine/am9517a.h \
	src/mess/machine/cs4031.h \

$(OBJ)/mess/machine/cs8221.o : \
	src/mess/machine/cs8221.h \
	src/emu/machine/ram.h \
	src/mess/machine/cs8221.c \

$(OBJ)/mess/machine/cuda.o : \
	src/mess/machine/egret.h \
	src/mess/machine/macpds.h \
	src/emu/sound/asc.h \
	src/mess/machine/macrtc.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/mess/includes/mac.h \
	src/emu/machine/ram.h \
	src/mess/machine/cuda.h \
	src/lib/softfloat/mamesf.h \
	src/mess/machine/ncr5380.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/mess/machine/cuda.c \
	src/emu/machine/scsidev.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/sound/awacs.h \
	src/mess/machine/mackbd.h \
	src/emu/bus/nubus/nubus.h \
	src/mess/machine/8530scc.h \
	src/emu/machine/ncr539x.h \
	src/emu/machine/6522via.h \
	src/emu/cpu/m6805/m6805.h \
	src/emu/machine/scsihle.h \

$(OBJ)/mess/machine/cybiko.o : \
	src/mess/includes/cybiko.h \
	src/emu/machine/at45dbxx.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/video/hd66421.h \
	src/emu/cpu/h83002/h8.h \
	src/mess/machine/cybiko.c \
	src/emu/imagedev/snapquik.h \
	src/emu/machine/intelfsh.h \
	src/emu/machine/pcf8593.h \

$(OBJ)/mess/machine/dai.o : \
	src/emu/machine/pit8253.h \
	src/emu/machine/tms5501.h \
	src/mess/includes/dai.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/dai.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/dccons.o : \
	src/mame/machine/maple-dc.h \
	src/lib/util/palette.h \
	src/mame/machine/naomig1.h \
	src/emu/sound/aica.h \
	src/mame/includes/dc.h \
	src/emu/cpu/sh4/sh4.h \
	src/emu/machine/atahle.h \
	src/lib/util/corealloc.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/lib/util/chd.h \
	src/mess/machine/dccons.c \
	src/emu/machine/scsibus.h \
	src/lib/util/chdcodec.h \
	src/mame/video/powervr2.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/imagedev/chd_cd.h \
	src/emu/machine/atapicdr.h \
	src/mame/machine/gdrom.h \
	src/lib/util/md5.h \
	src/emu/machine/t10mmc.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/mess/includes/dccons.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/intelfsh.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/lib/util/astring.h \
	src/emu/machine/atapihle.h \

$(OBJ)/mess/machine/dec_lk201.o : \
	src/mess/machine/dec_lk201.c \
	src/mess/machine/dec_lk201.h \
	src/emu/cpu/m6805/m6805.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/machine/dgn_beta.o : \
	src/lib/formats/flopimg.h \
	src/emu/debug/debugcon.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/lib/util/simple_set.h \
	src/mess/machine/dgn_beta.c \
	src/emu/machine/6821pia.h \
	src/emu/video/mc6845.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \
	src/emu/debug/textbuf.h \
	src/mess/includes/dgn_beta.h \
	src/emu/cpu/m6809/m6809.h \
	src/lib/util/opresolv.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/dgnalpha.o : \
	src/lib/formats/flopimg.h \
	src/mess/machine/dgnalpha.c \
	src/mess/machine/coco_vhd.h \
	src/lib/util/corealloc.h \
	src/emu/video/mc6847.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/mess/includes/coco.h \
	src/mess/machine/6883sam.h \
	src/emu/machine/6821pia.h \
	src/mess/includes/coco12.h \
	src/emu/bus/coco/cococart.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/dgnalpha.h \
	src/mess/includes/dragon.h \
	src/lib/util/opresolv.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/docg3.o : \
	src/mess/machine/docg3.c \
	src/mess/machine/docg3.h \

$(OBJ)/mess/machine/dragon.o : \
	src/mess/machine/coco_vhd.h \
	src/mess/machine/dragon.c \
	src/lib/util/corealloc.h \
	src/emu/video/mc6847.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/coco.h \
	src/mess/machine/6883sam.h \
	src/emu/machine/6821pia.h \
	src/mess/includes/coco12.h \
	src/emu/bus/coco/cococart.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/dragon.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/e05a03.o : \
	src/mess/machine/e05a03.h \
	src/mess/machine/e05a03.c \

$(OBJ)/mess/machine/egret.o : \
	src/mess/machine/egret.h \
	src/mess/machine/macpds.h \
	src/emu/sound/asc.h \
	src/mess/machine/macrtc.h \
	src/mess/machine/egret.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/mess/includes/mac.h \
	src/emu/machine/ram.h \
	src/mess/machine/cuda.h \
	src/lib/softfloat/mamesf.h \
	src/mess/machine/ncr5380.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/sound/awacs.h \
	src/mess/machine/mackbd.h \
	src/emu/bus/nubus/nubus.h \
	src/mess/machine/8530scc.h \
	src/emu/machine/ncr539x.h \
	src/emu/machine/6522via.h \
	src/emu/cpu/m6805/m6805.h \
	src/emu/machine/scsihle.h \

$(OBJ)/mess/machine/einstein.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/video/tms9928a.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/mess/includes/einstein.h \
	src/emu/machine/z80ctc.h \
	src/emu/video/mc6845.h \
	src/mess/machine/einstein.c \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/machine/electron.o : \
	src/lib/util/corealloc.h \
	src/mess/includes/electron.h \
	src/emu/imagedev/cassette.h \
	src/mess/machine/electron.c \
	src/lib/formats/cassimg.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/machine/esqpanel.o : \
	src/mess/machine/esqpanel.h \
	src/mess/machine/esqpanel.c \
	src/mess/machine/esqvfd.h \

$(OBJ)/mess/machine/esqvfd.o : \
	src/mess/machine/esqvfd.h \
	src/mess/machine/esqvfd.c \

$(OBJ)/mess/machine/europc.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/video/pc_aga.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/util/coreutil.h \
	src/lib/formats/cassimg.h \
	src/mess/machine/europc.c \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/machine/ins8250.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/includes/pc.h \
	src/emu/imagedev/floppy.h \
	src/mess/includes/europc.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/machine/fm_scsi.o : \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/mess/machine/fm_scsi.c \
	src/mess/machine/fm_scsi.h \
	src/emu/machine/scsihle.h \

$(OBJ)/mess/machine/galaxy.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/machine/galaxy.c \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/galaxy.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/gamecom.o : \
	src/emu/cpu/sm8500/sm8500.h \
	src/emu/imagedev/cartslot.h \
	src/mess/machine/gamecom.c \
	src/emu/sound/dac.h \
	src/mess/includes/gamecom.h \

$(OBJ)/mess/machine/gamepock.o : \
	src/emu/sound/speaker.h \
	src/emu/cpu/upd7810/upd7810.h \
	src/mess/machine/gamepock.c \
	src/mess/includes/gamepock.h \

$(OBJ)/mess/machine/gb.o : \
	src/emu/bus/gameboy/gb_slot.h \
	src/mess/machine/gb.c \
	src/emu/cpu/lr35902/lr35902.h \
	src/emu/machine/ram.h \
	src/mess/includes/gb.h \
	src/mess/video/gb_lcd.h \
	src/mess/audio/gb.h \

$(OBJ)/mess/machine/genpc.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/emu/sound/2151intf.h \
	src/lib/util/corealloc.h \
	src/emu/bus/isa/gus.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/mess/machine/genpc.c \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/machine/ins8250.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/bus/isa/finalchs.h \
	src/mess/includes/genpc.h \
	src/emu/imagedev/harddriv.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/lib/util/opresolv.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/hd63450.o : \
	src/mess/machine/hd63450.c \
	src/mess/machine/hd63450.h \

$(OBJ)/mess/machine/hd64610.o : \
	src/mess/machine/hd64610.h \
	src/lib/util/coreutil.h \
	src/mess/machine/hd64610.c \

$(OBJ)/mess/machine/hec2hrp.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/machine/hec2hrp.c \
	src/lib/formats/hect_tap.h \
	src/emu/sound/disc_dev.h \
	src/mess/includes/hec2hrp.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/wd17xx.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/wavwrite.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/printer.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/emu/sound/sn76477.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/machine/hecdisk2.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/sound/disc_dev.h \
	src/mess/includes/hec2hrp.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/wavwrite.h \
	src/emu/machine/upd765.h \
	src/mess/machine/hecdisk2.c \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/emu/sound/sn76477.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/hect_dsk.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/machine/hp48.o : \
	src/emu/machine/nvram.h \
	src/emu/cpu/saturn/saturn.h \
	src/mess/machine/hp48.c \
	src/mess/includes/hp48.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/i8271.o : \
	src/lib/formats/flopimg.h \
	src/mess/machine/i8271.h \
	src/mess/machine/i8271.c \
	src/emu/imagedev/flopdrv.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/intv.o : \
	src/emu/cpu/cp1610/cp1610.h \
	src/emu/sound/sp0256.h \
	src/mess/machine/intv.c \
	src/mess/video/stic.h \
	src/mess/includes/intv.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/machine/isbc_215g.o : \
	src/emu/cpu/i8089/i8089_channel.h \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/lib/util/sha1.h \
	src/mess/machine/isbc_215g.h \
	src/lib/util/chd.h \
	src/emu/bus/isbx/isbx.h \
	src/mess/machine/isbc_215g.c \
	src/lib/util/chdcodec.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/emu/cpu/i8089/i8089.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/k7659kb.o : \
	src/mess/machine/k7659kb.c \
	src/mess/machine/k7659kb.h \

$(OBJ)/mess/machine/kay_kbd.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/emu/machine/z80sio.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/kaypro.h \
	src/emu/machine/com8116.h \
	src/emu/video/mc6845.h \
	src/mess/machine/kay_kbd.c \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/machine/kaypro.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/emu/machine/z80sio.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/kaypro.h \
	src/emu/machine/com8116.h \
	src/emu/video/mc6845.h \
	src/mess/machine/kaypro.c \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/machine/kb3600.o : \
	src/mess/machine/kb3600.c \
	src/mess/machine/kb3600.h \

$(OBJ)/mess/machine/kc.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/mess/machine/kc.c \
	src/lib/formats/kc_cas.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/bus/kc/d004.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/z80ctc.h \
	src/lib/util/md5.h \
	src/emu/machine/upd765.h \
	src/emu/bus/kc/ram.h \
	src/emu/imagedev/harddriv.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/emu/sound/wave.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/kc/d002.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/kc_keyb.h \
	src/mess/includes/kc.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/bus/kc/rom.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \
	src/emu/bus/kc/kc.h \

$(OBJ)/mess/machine/kc_keyb.o : \
	src/mess/machine/kc_keyb.c \
	src/mess/machine/kc_keyb.h \

$(OBJ)/mess/machine/kr2376.o : \
	src/mess/machine/kr2376.c \
	src/mess/machine/kr2376.h \

$(OBJ)/mess/machine/kramermc.o : \
	src/emu/machine/z80pio.h \
	src/mess/machine/kramermc.c \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/includes/kramermc.h \

$(OBJ)/mess/machine/lh5810.o : \
	src/mess/machine/lh5810.c \
	src/mess/machine/lh5810.h \

$(OBJ)/mess/machine/lisa.o : \
	src/lib/formats/flopimg.h \
	src/mess/machine/lisa.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/sound/speaker.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/mess/machine/applefdc.h \
	src/mess/machine/sonydriv.h \
	src/lib/softfloat/softfloat.h \
	src/mess/includes/lisa.h \
	src/mess/machine/8530scc.h \
	src/emu/machine/6522via.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/llc.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/mess/machine/llc.c \
	src/mess/includes/llc.h \
	src/mess/machine/k7659kb.h \

$(OBJ)/mess/machine/lviv.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mess/machine/lviv.c \
	src/emu/cpu/i8085/i8085.h \
	src/emu/imagedev/snapquik.h \
	src/mess/includes/lviv.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/lynx.o : \
	src/mess/audio/lynx.h \
	src/mess/includes/lynx.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/mess/machine/lynx.c \
	src/emu/imagedev/cartslot.h \
	src/emu/imagedev/snapquik.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/cpu/m6502/r65c02.h \

$(OBJ)/mess/machine/mac.o : \
	src/lib/formats/flopimg.h \
	src/mess/machine/egret.h \
	src/mess/machine/macpds.h \
	src/emu/sound/asc.h \
	src/mess/machine/macrtc.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/mess/includes/mac.h \
	src/emu/machine/ram.h \
	src/mess/machine/cuda.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/softfloat/mamesf.h \
	src/mess/machine/ncr5380.h \
	src/emu/machine/scsibus.h \
	src/lib/util/simple_set.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/lib/softfloat/milieu.h \
	src/mess/machine/mac.c \
	src/mess/machine/applefdc.h \
	src/mess/machine/sonydriv.h \
	src/lib/softfloat/softfloat.h \
	src/emu/debug/express.h \
	src/emu/sound/awacs.h \
	src/mess/machine/mackbd.h \
	src/emu/debug/debugcpu.h \
	src/emu/bus/nubus/nubus.h \
	src/mess/machine/8530scc.h \
	src/emu/machine/ncr539x.h \
	src/emu/machine/6522via.h \
	src/lib/util/opresolv.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/macadb.o : \
	src/mess/machine/egret.h \
	src/mess/machine/macpds.h \
	src/mess/machine/macadb.c \
	src/emu/sound/asc.h \
	src/mess/machine/macrtc.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/mess/includes/mac.h \
	src/emu/machine/ram.h \
	src/mess/machine/cuda.h \
	src/lib/softfloat/mamesf.h \
	src/mess/machine/ncr5380.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/sound/awacs.h \
	src/mess/machine/mackbd.h \
	src/emu/bus/nubus/nubus.h \
	src/mess/machine/8530scc.h \
	src/emu/machine/ncr539x.h \
	src/emu/machine/6522via.h \
	src/emu/machine/scsihle.h \

$(OBJ)/mess/machine/mackbd.o : \
	src/emu/cpu/mcs48/mcs48.h \
	src/mess/machine/mackbd.h \
	src/mess/machine/mackbd.c \

$(OBJ)/mess/machine/macpci.o : \
	src/lib/formats/flopimg.h \
	src/mess/includes/macpci.h \
	src/mess/machine/macpci.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/mess/machine/cuda.h \
	src/lib/softfloat/mamesf.h \
	src/emu/machine/scsibus.h \
	src/lib/util/simple_set.h \
	src/emu/machine/t10spc.h \
	src/lib/softfloat/milieu.h \
	src/emu/machine/scsidev.h \
	src/mess/machine/applefdc.h \
	src/mess/machine/sonydriv.h \
	src/lib/softfloat/softfloat.h \
	src/emu/debug/express.h \
	src/emu/sound/awacs.h \
	src/emu/debug/debugcpu.h \
	src/mess/machine/8530scc.h \
	src/emu/machine/ncr539x.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/scsihle.h \

$(OBJ)/mess/machine/macpds.o : \
	src/mess/machine/macpds.h \
	src/mess/machine/macpds.c \

$(OBJ)/mess/machine/macrtc.o : \
	src/mess/machine/macrtc.c \
	src/mess/machine/macrtc.h \

$(OBJ)/mess/machine/mb8795.o : \
	src/mess/machine/mb8795.c \
	src/mess/machine/mb8795.h \

$(OBJ)/mess/machine/mb89352.o : \
	src/mess/machine/mb89352.c \
	src/emu/machine/scsibus.h \
	src/mess/machine/mb89352.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/scsihle.h \

$(OBJ)/mess/machine/mbc55x.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/emu/debug/debugcon.h \
	src/mess/includes/mbc55x.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/machine/latch.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/mess/machine/mbc55x.c \
	src/emu/debug/textbuf.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/mbee.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/machine/mc146818.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/video/mc6845.h \
	src/mess/includes/mbee.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/machine/z80bin.h \
	src/mess/machine/mbee.c \

$(OBJ)/mess/machine/mboard.o : \
	src/mess/includes/mboard.h \
	src/mess/machine/mboard.c \

$(OBJ)/mess/machine/mc68328.o : \
	src/mess/machine/mc68328.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/ram.h \
	src/mess/machine/mc68328.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \

$(OBJ)/mess/machine/mc80.o : \
	src/mess/machine/mc80.c \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/mess/includes/mc80.h \
	src/emu/machine/z80dart.h \

$(OBJ)/mess/machine/microdrv.o : \
	src/mess/machine/microdrv.c \
	src/mess/machine/microdrv.h \

$(OBJ)/mess/machine/micropolis.o : \
	src/lib/formats/flopimg.h \
	src/mess/machine/micropolis.h \
	src/emu/imagedev/flopdrv.h \
	src/mess/machine/micropolis.c \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/microtan.o : \
	src/mess/machine/microtan.c \
	src/mess/includes/microtan.h \
	src/lib/util/corealloc.h \
	src/emu/machine/mos6551.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6522via.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/machine/mikro80.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i8085/i8085.h \
	src/mess/machine/mikro80.c \
	src/mess/includes/mikro80.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/msx.o : \
	src/lib/formats/fmsx_cas.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/mess/machine/msx.c \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/rp5c01.h \
	src/emu/video/v9938.h \
	src/emu/video/tms9928a.h \
	src/mess/includes/msx_slot.h \
	src/emu/sound/k051649.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/msx_dsk.h \
	src/emu/machine/i8255.h \
	src/emu/sound/2413intf.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/mess/includes/msx.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/msx_slot.o : \
	src/lib/formats/fmsx_cas.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/rp5c01.h \
	src/emu/video/v9938.h \
	src/emu/video/tms9928a.h \
	src/mess/includes/msx_slot.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/sound/k051649.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/msx_dsk.h \
	src/emu/machine/i8255.h \
	src/mess/machine/msx_slot.c \
	src/emu/sound/2413intf.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/mess/includes/msx.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/mtx.o : \
	src/emu/machine/buffer.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/video/tms9928a.h \
	src/lib/formats/cassimg.h \
	src/mess/machine/mtx.c \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/imageutl.h \
	src/emu/machine/z80ctc.h \
	src/emu/sound/sn76496.h \
	src/mess/includes/mtx.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/mz700.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/mess/includes/mz700.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/74145.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/mess/machine/mz700.c \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/mz80.o : \
	src/emu/machine/pit8253.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/mess/includes/mz80.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/mz80.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/nascom1.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/mess/machine/nascom1.c \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/nascom1.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/ay31015.h \

$(OBJ)/mess/machine/nc.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/latch.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/mess/includes/nc.h \
	src/mess/machine/nc.c \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/clock.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/machine/ncr5380.o : \
	src/mess/machine/ncr5380.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/mess/machine/ncr5380.c \
	src/emu/machine/scsihle.h \

$(OBJ)/mess/machine/ncr5390.o : \
	src/mess/machine/ncr5390.h \
	src/mess/machine/ncr5390.c \
	src/emu/machine/nscsi_bus.h \

$(OBJ)/mess/machine/nes.o : \
	src/emu/bus/nes/konami.h \
	src/emu/bus/nes/bootleg.h \
	src/emu/bus/nes/rexsoft.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/nes/karastudio.h \
	src/emu/bus/nes/act53.h \
	src/emu/bus/nes/mmc1.h \
	src/emu/bus/nes/txc.h \
	src/emu/bus/nes/sunsoft.h \
	src/emu/bus/nes/aladdin.h \
	src/emu/bus/nes/henggedianzi.h \
	src/emu/bus/nes/legacy.h \
	src/emu/bus/nes/pt554.h \
	src/emu/bus/nes/rcm.h \
	src/lib/util/corealloc.h \
	src/emu/sound/nes_apu.h \
	src/emu/bus/nes/namcot.h \
	src/emu/bus/nes/sunsoft_dcs.h \
	src/emu/imagedev/cassette.h \
	src/emu/bus/nes/mmc5.h \
	src/emu/bus/nes/discrete.h \
	src/emu/bus/nes/ave.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/bus/nes/jaleco.h \
	src/emu/bus/nes/nes_slot.h \
	src/emu/bus/nes/hes.h \
	src/emu/bus/nes/jy.h \
	src/emu/bus/nes/racermate.h \
	src/emu/bus/nes/bandai.h \
	src/emu/bus/nes/hosenkan.h \
	src/emu/bus/nes/nanjing.h \
	src/emu/bus/nes/datach.h \
	src/emu/bus/nes/sachen.h \
	src/emu/bus/nes/mmc3_clones.h \
	src/emu/bus/nes/cony.h \
	src/emu/bus/nes/somari.h \
	src/emu/machine/bcreader.h \
	src/emu/sound/2413intf.h \
	src/emu/bus/nes/nes_carts.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/sound/nes_defs.h \
	src/emu/bus/nes/pirate.h \
	src/mess/includes/nes.h \
	src/emu/bus/nes/taito.h \
	src/emu/bus/nes/nxrom.h \
	src/emu/bus/nes/mmc3.h \
	src/emu/bus/nes/multigame.h \
	src/emu/bus/nes/camerica.h \
	src/emu/bus/nes/kaiser.h \
	src/mame/video/ppu2c0x.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/nes/ggenie.h \
	src/lib/util/opresolv.h \
	src/emu/bus/nes/waixing.h \
	src/emu/bus/nes/tengen.h \
	src/emu/bus/nes/ntdec.h \
	src/mess/machine/nes.c \
	src/emu/bus/nes/benshieng.h \
	src/emu/bus/nes/event.h \
	src/emu/bus/nes/irem.h \
	src/emu/sound/vrc6.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \
	src/emu/bus/nes/cne.h \
	src/emu/bus/nes/mmc2.h \
	src/emu/machine/i2cmem.h \

$(OBJ)/mess/machine/nextkbd.o : \
	src/mess/machine/nextkbd.c \
	src/mess/machine/nextkbd.h \

$(OBJ)/mess/machine/nextmo.o : \
	src/mess/machine/nextmo.h \
	src/mess/machine/nextmo.c \

$(OBJ)/mess/machine/ondra.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/mess/machine/ondra.c \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/ondra.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/orao.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/orao.h \
	src/mess/machine/orao.c \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/oric.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/machine/mos6551.h \
	src/emu/imagedev/cassette.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/mess/machine/applefdc.h \
	src/mess/machine/oric.c \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/ap2_dsk.h \
	src/mess/includes/oric.h \
	src/emu/machine/6522via.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/machine/clock.h \
	src/lib/formats/oric_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/oric_tap.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/machine/orion.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/mc146818.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/machine/wd_fdc.h \
	src/emu/video/i8275.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/orion.h \
	src/lib/formats/imd_dsk.h \
	src/mess/machine/orion.c \
	src/emu/machine/i8255.h \
	src/mess/includes/radio86.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/8257dma.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/machine/osborne1.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/lib/formats/flopimg.h \
	src/mess/includes/osborne1.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/mess/machine/osborne1.c \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/machine/p2000t.o : \
	src/emu/video/saa5050.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80.h \
	src/mess/machine/p2000t.c \
	src/mess/includes/p2000t.h \

$(OBJ)/mess/machine/partner.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/video/i8275.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/mess/includes/partner.h \
	src/emu/machine/i8255.h \
	src/mess/machine/partner.c \
	src/mess/includes/radio86.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/machine/8257dma.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/pc.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/video/pc_vga.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/util/coreutil.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pc_fdc.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/machine/ins8250.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/emu/machine/pckeybrd.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/mess/includes/pc.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/pc_lpt.h \
	src/lib/util/opresolv.h \
	src/mess/video/pc_t1t.h \
	src/emu/imagedev/floppy.h \
	src/emu/video/pc_cga.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/mess/machine/pc.c \
	src/lib/formats/d88_dsk.h \
	src/mess/includes/amstr_pc.h \

$(OBJ)/mess/machine/pc1251.o : \
	src/emu/machine/nvram.h \
	src/mess/includes/pocketc.h \
	src/emu/cpu/sc61860/sc61860.h \
	src/mess/includes/pc1251.h \
	src/mess/machine/pc1251.c \

$(OBJ)/mess/machine/pc1350.o : \
	src/emu/machine/nvram.h \
	src/mess/includes/pc1350.h \
	src/mess/machine/pc1350.c \
	src/emu/machine/ram.h \
	src/mess/includes/pocketc.h \
	src/emu/cpu/sc61860/sc61860.h \

$(OBJ)/mess/machine/pc1401.o : \
	src/mess/includes/pc1401.h \
	src/emu/machine/nvram.h \
	src/emu/machine/ram.h \
	src/mess/includes/pocketc.h \
	src/emu/cpu/sc61860/sc61860.h \
	src/mess/machine/pc1401.c \

$(OBJ)/mess/machine/pc1403.o : \
	src/emu/machine/nvram.h \
	src/emu/machine/ram.h \
	src/mess/includes/pocketc.h \
	src/emu/cpu/sc61860/sc61860.h \
	src/mess/includes/pc1403.h \
	src/mess/machine/pc1403.c \

$(OBJ)/mess/machine/pc1512kb.o : \
	src/emu/cpu/mcs48/mcs48.h \
	src/mess/machine/pc1512kb.c \
	src/mess/machine/pc1512kb.h \

$(OBJ)/mess/machine/pc9801_118.o : \
	src/mess/machine/pc9801_118.h \
	src/emu/machine/pic8259.h \
	src/mess/machine/pc9801_118.c \
	src/emu/sound/2608intf.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/machine/pc9801_26.o : \
	src/emu/machine/pic8259.h \
	src/mess/machine/pc9801_26.c \
	src/mess/machine/pc9801_26.h \
	src/emu/sound/2203intf.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/machine/pc9801_86.o : \
	src/mess/machine/pc9801_86.c \
	src/emu/machine/pic8259.h \
	src/emu/sound/2608intf.h \
	src/emu/sound/ay8910.h \
	src/mess/machine/pc9801_86.h \

$(OBJ)/mess/machine/pc9801_cbus.o : \
	src/mess/machine/pc9801_cbus.c \
	src/mess/machine/pc9801_cbus.h \

$(OBJ)/mess/machine/pc9801_kbd.o : \
	src/mess/machine/pc9801_kbd.h \
	src/mess/machine/pc9801_kbd.c \

$(OBJ)/mess/machine/pce.o : \
	src/emu/cpu/h6280/h6280.h \
	src/lib/util/palette.h \
	src/emu/machine/nvram.h \
	src/emu/sound/msm5205.h \
	src/lib/util/corealloc.h \
	src/mess/includes/pce.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/emu/video/huc6260.h \
	src/mess/machine/pce.c \
	src/lib/util/chd.h \
	src/mess/machine/pce_cd.h \
	src/lib/util/chdcodec.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/bus/pce/pce_slot.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/pce220_ser.o : \
	src/mess/machine/pce220_ser.h \
	src/mess/machine/pce220_ser.c \

$(OBJ)/mess/machine/pce_cd.o : \
	src/lib/util/palette.h \
	src/emu/machine/nvram.h \
	src/emu/sound/msm5205.h \
	src/lib/util/corealloc.h \
	src/lib/util/cdrom.h \
	src/lib/util/coreutil.h \
	src/lib/util/sha1.h \
	src/mess/machine/pce_cd.c \
	src/lib/util/chd.h \
	src/mess/machine/pce_cd.h \
	src/lib/util/chdcodec.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/pecom.o : \
	src/mess/machine/pecom.c \
	src/emu/sound/cdp1869.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/pecom.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/pk8020.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/pic8259.h \
	src/mess/includes/pk8020.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/mess/machine/pk8020.c \

$(OBJ)/mess/machine/pmd85.o : \
	src/emu/machine/pit8253.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mess/includes/pmd85.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/pmd85.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/poly88.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/mess/machine/poly88.c \
	src/lib/formats/cassimg.h \
	src/mess/includes/poly88.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/pp01.o : \
	src/emu/machine/pit8253.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/i8255.h \
	src/mess/includes/pp01.h \
	src/emu/cpu/i8085/i8085.h \
	src/mess/machine/pp01.c \

$(OBJ)/mess/machine/primo.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/imagedev/cartslot.h \
	src/mess/includes/primo.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/primo.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/prof80mmu.o : \
	src/mess/machine/prof80mmu.c \
	src/mess/machine/prof80mmu.h \

$(OBJ)/mess/machine/psion_pack.o : \
	src/mess/machine/psion_pack.c \
	src/mess/machine/psion_pack.h \

$(OBJ)/mess/machine/psxanalog.o : \
	src/mess/machine/psxcard.h \
	src/mess/machine/psxanalog.h \
	src/mess/machine/psxanalog.c \
	src/mess/machine/psxcport.h \
	src/emu/cpu/psx/siodev.h \

$(OBJ)/mess/machine/psxcard.o : \
	src/mess/machine/psxcard.c \
	src/mess/machine/psxcard.h \
	src/mess/machine/psxcport.h \
	src/emu/cpu/psx/siodev.h \

$(OBJ)/mess/machine/psxcd.o : \
	src/lib/util/palette.h \
	src/mess/machine/psxcd.h \
	src/lib/util/corealloc.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/emu/sound/spu.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/emu/sound/spureverb.h \
	src/lib/util/astring.h \
	src/mess/machine/psxcd.c \

$(OBJ)/mess/machine/psxcport.o : \
	src/mess/machine/psxcard.h \
	src/mess/machine/psxmultitap.h \
	src/mess/machine/psxanalog.h \
	src/mess/machine/psxcport.h \
	src/mess/machine/psxcport.c \
	src/emu/cpu/psx/siodev.h \

$(OBJ)/mess/machine/psxmultitap.o : \
	src/mess/machine/psxcard.h \
	src/mess/machine/psxmultitap.h \
	src/mess/machine/psxmultitap.c \
	src/mess/machine/psxcport.h \
	src/emu/cpu/psx/siodev.h \

$(OBJ)/mess/machine/qx10kbd.o : \
	src/emu/bus/rs232/rs232.h \
	src/mess/machine/qx10kbd.h \
	src/mess/machine/qx10kbd.c \
	src/emu/bus/rs232/keyboard.h \
	src/emu/machine/keyboard.h \

$(OBJ)/mess/machine/radio86.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/video/i8275.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mess/machine/radio86.c \
	src/mess/includes/radio86.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/8257dma.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/rm380z.o : \
	src/lib/formats/flopimg.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/keyboard.h \
	src/lib/util/opresolv.h \
	src/mess/machine/rm380z.c \
	src/lib/formats/ioprocs.h \
	src/mess/includes/rm380z.h \

$(OBJ)/mess/machine/rmnimbus.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/emu/debug/debugcon.h \
	src/emu/sound/msm5205.h \
	src/emu/machine/latch.h \
	src/emu/machine/i8251.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/er59256.h \
	src/emu/machine/z80sio.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/scsibus.h \
	src/lib/util/simple_set.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/scsicb.h \
	src/emu/cpu/i86/i86.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \
	src/emu/debug/textbuf.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/includes/rmnimbus.h \
	src/emu/cpu/i86/i186.h \
	src/lib/formats/ioprocs.h \
	src/mess/machine/rmnimbus.c \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/machine/rx01.o : \
	src/lib/formats/flopimg.h \
	src/mess/machine/rx01.c \
	src/emu/imagedev/flopdrv.h \
	src/mess/machine/rx01.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \

$(OBJ)/mess/machine/s1410.o : \
	src/mess/machine/s1410.c \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/s1410.h \
	src/emu/machine/scsihd.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/s3c44b0.o : \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/lib/util/coreutil.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7core.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \
	src/mess/machine/s3c44b0.h \
	src/emu/sound/dac.h \
	src/mess/machine/s3c44b0.c \

$(OBJ)/mess/machine/sa1403d.o : \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/lib/util/md5.h \
	src/mess/machine/sa1403d.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/sa1403d.c \
	src/emu/machine/scsihd.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/samcoupe.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/mess/includes/samcoupe.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/msm6242.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/machine/samcoupe.c \

$(OBJ)/mess/machine/sgi.o : \
	src/mess/machine/sgi.c \
	src/mess/machine/sgi.h \

$(OBJ)/mess/machine/smartmed.o : \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/mess/machine/smartmed.h \
	src/lib/util/sha1.h \
	src/mess/machine/smartmed.c \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/lib/formats/imageutl.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/smc92x4.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/util/chd.h \
	src/mess/machine/ti99/ti99_hd.h \
	src/lib/util/chdcodec.h \
	src/lib/formats/imageutl.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/machine/smc92x4.c \
	src/lib/util/hashing.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/mess/machine/smc92x4.h \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/sms.o : \
	src/emu/bus/sms_exp/gender.h \
	src/emu/bus/sms_ctrl/joypad.h \
	src/emu/bus/sms_ctrl/smsctrl.h \
	src/emu/bus/sega8/rom.h \
	src/emu/bus/sms_ctrl/rfu.h \
	src/emu/bus/sms_ctrl/paddle.h \
	src/emu/bus/sms_exp/smsexp.h \
	src/mess/includes/sms.h \
	src/emu/machine/eepromser.h \
	src/emu/sound/2413intf.h \
	src/emu/machine/eeprom.h \
	src/mess/machine/sms.c \
	src/emu/bus/sms_ctrl/sportsjp.h \
	src/emu/bus/sega8/sega8_slot.h \
	src/emu/bus/sms_ctrl/lphaser.h \
	src/emu/bus/sms_ctrl/sports.h \
	src/emu/video/315_5124.h \

$(OBJ)/mess/machine/snescx4.o : \
	src/mess/machine/cx4ops.inc \
	src/mess/machine/cx4oam.inc \
	src/mess/machine/cx4data.inc \
	src/mess/machine/cx4fn.inc \
	src/mess/machine/snescx4.h \
	src/mess/machine/snescx4.c \

$(OBJ)/mess/machine/sonydriv.o : \
	src/lib/formats/flopimg.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/ap_dsk35.h \
	src/mess/machine/applefdc.h \
	src/mess/machine/sonydriv.h \
	src/mess/machine/sonydriv.c \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/sorcerer.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/mess/includes/sorcerer.h \
	src/emu/bus/centronics/covox.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/sorc_dsk.h \
	src/mess/machine/micropolis.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/machine/sorcerer.c \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/sorc_cas.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/mess/machine/z80bin.h \
	src/emu/machine/ay31015.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/spec_snqk.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/mess/machine/spec_snqk.c \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/mess/machine/spec_snqk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/ui/ui.h \
	src/mess/includes/spectrum.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/cartslot.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/machine/special.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/mess/includes/special.h \
	src/lib/formats/rk_cas.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/lib/formats/smx_dsk.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/machine/special.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/strata.o : \
	src/mess/machine/strata.h \
	src/mess/machine/strata.c \

$(OBJ)/mess/machine/super80.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/machine/super80.c \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/super80.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \
	src/mess/machine/z80bin.h \

$(OBJ)/mess/machine/svi318.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/video/tms9928a.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/lib/formats/svi_cas.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/ins8250.h \
	src/emu/video/mc6845.h \
	src/mess/machine/svi318.c \
	src/mess/includes/svi318.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/swim.o : \
	src/mess/machine/applefdc.h \
	src/mess/machine/swim.h \
	src/mess/machine/swim.c \

$(OBJ)/mess/machine/tandy1t.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/machine/ins8250.h \
	src/mess/machine/tandy1t.c \
	src/emu/machine/upd765.h \
	src/mess/includes/tandy1t.h \
	src/emu/machine/pckeybrd.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/includes/pc.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/machine/tandy2kb.o : \
	src/emu/cpu/mcs48/mcs48.h \
	src/mess/machine/tandy2kb.c \
	src/mess/machine/tandy2kb.h \

$(OBJ)/mess/machine/thomflop.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/mess/machine/thomflop.c \
	src/emu/machine/mc6843.h \
	src/emu/bus/rs232/rs232.h \
	src/mess/machine/thomflop.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/machine/mos6551.h \
	src/emu/imagedev/cassette.h \
	src/mess/audio/mea8000.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/mc6854.h \
	src/mess/includes/thomson.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/mc6846.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/thom_dsk.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/thom_cas.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/thomson.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/mess/machine/thomson.c \
	src/emu/machine/mc6843.h \
	src/emu/bus/rs232/rs232.h \
	src/mess/machine/thomflop.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/machine/mos6551.h \
	src/emu/imagedev/cassette.h \
	src/mess/audio/mea8000.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/mc6854.h \
	src/mess/includes/thomson.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/mc6846.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/thom_dsk.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/thom_cas.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/ti85.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/mess/machine/ti85.c \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/snapquik.h \
	src/mess/video/t6a04.h \
	src/mess/includes/ti85.h \

$(OBJ)/mess/machine/trs80.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/mess/machine/trs80.c \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/trs_dsk.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/mess/includes/trs80.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/trs_cas.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/ay31015.h \

$(OBJ)/mess/machine/trs80m2kb.o : \
	src/emu/machine/rescap.h \
	src/mess/machine/trs80m2kb.c \
	src/emu/sound/disc_dev.h \
	src/mess/machine/trs80m2kb.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/wavwrite.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/machine/upd65031.o : \
	src/mess/machine/upd65031.h \
	src/mess/machine/upd65031.c \

$(OBJ)/mess/machine/upd71071.o : \
	src/mess/machine/upd71071.h \
	src/mess/machine/upd71071.c \

$(OBJ)/mess/machine/ut88.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/machine/ut88.c \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/mess/includes/ut88.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/machine/v1050kb.o : \
	src/mess/machine/v1050kb.h \
	src/emu/machine/rescap.h \
	src/emu/sound/disc_dev.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/wavwrite.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/mess/machine/v1050kb.c \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/machine/vector06.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/mess/includes/vector06.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/machine/vector06.c \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \

$(OBJ)/mess/machine/victor9kb.o : \
	src/mess/machine/victor9kb.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/mess/machine/victor9kb.c \

$(OBJ)/mess/machine/vtech2.o : \
	src/lib/formats/flopimg.h \
	src/mess/machine/vtech2.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/mess/includes/vtech2.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/wangpckb.o : \
	src/mess/machine/wangpckb.h \
	src/mess/machine/wangpckb.c \
	src/emu/sound/sn76496.h \
	src/emu/cpu/mcs51/mcs51.h \

$(OBJ)/mess/machine/wswan.o : \
	src/emu/machine/nvram.h \
	src/mess/includes/wswan.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/v30mz/v30mz.h \
	src/mess/machine/wswan.c \

$(OBJ)/mess/machine/x1.o : \
	src/lib/formats/flopimg.h \
	src/emu/sound/2151intf.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/includes/x1.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/mess/machine/x1.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/z80dma.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/x1_tap.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/machine/x68k_hdc.o : \
	src/lib/util/palette.h \
	src/mess/machine/x68k_hdc.h \
	src/lib/util/corealloc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/lib/util/harddisk.h \
	src/mess/machine/x68k_hdc.c \
	src/lib/util/astring.h \

$(OBJ)/mess/machine/x68k_kbd.o : \
	src/mess/machine/x68k_kbd.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/bus/rs232/keyboard.h \
	src/emu/machine/keyboard.h \
	src/mess/machine/x68k_kbd.c \

$(OBJ)/mess/machine/z80bin.o : \
	src/mess/machine/z80bin.c \
	src/mess/machine/z80bin.h \

$(OBJ)/mess/machine/z80ne.o : \
	src/lib/formats/flopimg.h \
	src/mess/machine/z80ne.c \
	src/emu/video/mc6847.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/lib/util/coretmpl.h \
	src/mess/machine/kr2376.h \
	src/lib/util/opresolv.h \
	src/mess/includes/z80ne.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/ay31015.h \

$(OBJ)/mess/machine/zx.o : \
	src/mess/machine/zx.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/zx81_p.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/zx.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/machine/zx8302.o : \
	src/mess/machine/zx8302.c \
	src/mess/machine/microdrv.h \
	src/mess/machine/zx8302.h \

$(OBJ)/mess/tools/castool/main.o : \
	src/lib/formats/fmsx_cas.h \
	src/lib/formats/pmd_cas.h \
	src/lib/formats/x07_cas.h \
	src/lib/formats/a26_cas.h \
	src/lib/formats/kc_cas.h \
	src/lib/formats/gtp_cas.h \
	src/lib/formats/vg5k_cas.h \
	src/lib/formats/rk_cas.h \
	src/lib/formats/hect_tap.h \
	src/lib/util/corealloc.h \
	src/lib/formats/cgen_cas.h \
	src/lib/util/corestr.h \
	src/lib/formats/kim1_cas.h \
	src/lib/formats/sord_cas.h \
	src/lib/formats/zx81_p.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/svi_cas.h \
	src/lib/formats/adam_cas.h \
	src/lib/formats/mz_cas.h \
	src/lib/formats/coco_cas.h \
	src/lib/formats/ace_tap.h \
	src/mess/tools/castool/main.c \
	src/lib/formats/primoptp.h \
	src/lib/formats/uef_cas.h \
	src/lib/formats/vt_cas.h \
	src/lib/formats/lviv_lvt.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/tzx_cas.h \
	src/lib/formats/cbm_tap.h \
	src/lib/formats/trs_cas.h \
	src/lib/formats/apf_apt.h \
	src/lib/formats/orao_cas.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/oric_tap.h \
	src/lib/formats/thom_cas.h \
	src/lib/formats/csw_cas.h \
	src/lib/formats/tvc_cas.h \

$(OBJ)/mess/tools/floptool/main.o : \
	src/lib/formats/pc_dsk.h \
	src/mess/tools/floptool/main.c \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/naslite_dsk.h \
	src/lib/formats/upd765_dsk.h \
	src/lib/util/corestr.h \
	src/lib/formats/ap_dsk35.h \
	src/lib/formats/ami_dsk.h \
	src/lib/formats/st_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/formats/pasti_dsk.h \
	src/lib/formats/ap2_dsk.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/dsk_dsk.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/tools/imgtool/modules/amiga.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/iflopimg.h \
	src/mess/tools/imgtool/modules/amiga.c \
	src/mess/tools/imgtool/filter.h \
	src/lib/formats/ami_dsk.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/formats/imageutl.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/mess/tools/imgtool/library.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/bml3.o : \
	src/lib/formats/flopimg.h \
	src/mess/tools/imgtool/modules/bml3.c \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/iflopimg.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/concept.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/modules/concept.c \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/cybiko.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/modules/cybiko.c \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/cybikoxt.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/mess/tools/imgtool/modules/cybikoxt.c \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/fat.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/formats/imageutl.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/mess/tools/imgtool/modules/fat.c \
	src/lib/formats/ioprocs.h \
	src/mess/tools/imgtool/modules/fat.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/mac.o : \
	src/lib/formats/flopimg.h \
	src/mess/tools/imgtool/modules/macutil.h \
	src/lib/util/corealloc.h \
	src/mess/tools/imgtool/modules/mac.c \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/iflopimg.h \
	src/lib/formats/ap_dsk35.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/formats/imageutl.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/mess/tools/imgtool/library.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/macbin.o : \
	src/lib/formats/flopimg.h \
	src/mess/tools/imgtool/modules/macutil.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/formats/imageutl.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/mess/tools/imgtool/modules/macbin.c \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/macutil.o : \
	src/lib/formats/flopimg.h \
	src/mess/tools/imgtool/modules/macutil.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/modules/macutil.c \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/os9.o : \
	src/lib/formats/coco_dsk.h \
	src/lib/formats/flopimg.h \
	src/mess/tools/imgtool/modules/os9.c \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/iflopimg.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/formats/imageutl.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/pc_flop.o : \
	src/lib/formats/pc_dsk.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/upd765_dsk.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/iflopimg.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/formats/imageutl.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/mess/tools/imgtool/modules/pc_flop.c \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/mess/tools/imgtool/modules/fat.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/pc_hard.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/imghd.h \
	src/lib/util/chdcodec.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/formats/imageutl.h \
	src/lib/util/md5.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/mess/tools/imgtool/modules/pc_hard.c \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/util/hashing.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/prodos.o : \
	src/lib/formats/flopimg.h \
	src/mess/tools/imgtool/modules/macutil.h \
	src/lib/util/corealloc.h \
	src/mess/tools/imgtool/iflopimg.h \
	src/lib/util/corestr.h \
	src/lib/formats/ap_dsk35.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/modules/prodos.c \
	src/mess/tools/imgtool/charconv.h \
	src/lib/formats/imageutl.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ap2_dsk.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/psion.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/modules/psion.c \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/rsdos.o : \
	src/lib/formats/coco_dsk.h \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/iflopimg.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/mess/tools/imgtool/modules/rsdos.c \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/thomson.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/iflopimg.h \
	src/mess/tools/imgtool/modules/thomson.c \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/mess/tools/imgtool/library.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/ti99.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/mess/tools/imgtool/modules/ti99.c \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/imghd.h \
	src/lib/util/chdcodec.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/util/md5.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/util/hashing.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/ti990hd.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/modules/ti990hd.c \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules/vzdos.o : \
	src/lib/formats/vt_dsk.h \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/iflopimg.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/formats/imageutl.h \
	src/mess/tools/imgtool/modules/vzdos.c \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/charconv.o : \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/charconv.c \

$(OBJ)/mess/tools/imgtool/filtbas.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/formats/imageutl.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/mess/tools/imgtool/filtbas.c \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/filteoln.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/mess/tools/imgtool/filteoln.c \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/filter.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/filter.c \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/iflopimg.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/iflopimg.h \
	src/mess/tools/imgtool/iflopimg.c \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/imghd.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/imghd.h \
	src/lib/util/chdcodec.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imghd.c \
	src/lib/util/md5.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/util/hashing.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/imgterrs.o : \
	src/mess/tools/imgtool/imgterrs.c \
	src/mess/tools/imgtool/imgterrs.h \

$(OBJ)/mess/tools/imgtool/imgtool.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/formats/imageutl.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/mess/tools/imgtool/imgtool.c \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/mess/tools/imgtool/modules.h \
	src/lib/util/pool.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/library.o : \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/mess/tools/imgtool/library.c \
	src/lib/util/pool.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/main.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/mess/tools/imgtool/main.h \
	src/mess/tools/imgtool/modules.h \
	src/lib/formats/ioprocs.h \
	src/mess/tools/imgtool/main.c \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/modules.o : \
	src/mess/tools/imgtool/modules.c \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/charconv.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/mess/tools/imgtool/modules.h \
	src/lib/util/astring.h \

$(OBJ)/mess/tools/imgtool/stream.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/unzip.h \
	src/lib/util/corealloc.h \
	src/lib/util/corestr.h \
	src/mess/tools/imgtool/filter.h \
	src/mess/tools/imgtool/charconv.h \
	src/mess/tools/imgtool/imgtool.h \
	src/lib/util/unicode.h \
	src/mess/tools/imgtool/stream.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/tools/imgtool/library.h \
	src/mess/tools/imgtool/imgterrs.h \
	src/lib/formats/ioprocs.h \
	src/mess/tools/imgtool/stream.c \
	src/lib/util/astring.h \

$(OBJ)/mess/video/733_asr.o : \
	src/mess/video/733_asr.c \
	src/mess/video/733_asr.h \

$(OBJ)/mess/video/911_vdt.o : \
	src/mess/video/911_chr.h \
	src/mess/video/911_vdt.h \
	src/mess/video/911_vdt.c \
	src/mess/video/911_key.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/video/a7800.o : \
	src/emu/machine/rescap.h \
	src/mess/video/a7800.c \
	src/mess/includes/a7800.h \
	src/emu/sound/tiaintf.h \
	src/emu/sound/tiasound.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6532riot.h \
	src/emu/sound/pokey.h \

$(OBJ)/mess/video/abc1600.o : \
	src/mess/video/abc1600.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/z8536.h \
	src/mess/video/abc1600.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/emu/machine/e0516.h \
	src/lib/softfloat/milieu.h \
	src/mess/includes/abc1600.h \
	src/emu/machine/z80dma.h \
	src/lib/softfloat/softfloat.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/machine/8530scc.h \
	src/emu/machine/nmc9306.h \
	src/lib/util/opresolv.h \
	src/emu/bus/abckb/abckb.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/bus/abcbus/abcbus.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/machine/abc1600mac.h \

$(OBJ)/mess/video/abc80.o : \
	src/lib/formats/flopimg.h \
	src/mess/machine/abc80kb.h \
	src/emu/machine/rescap.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/includes/abc80.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/machine/keyboard.h \
	src/mess/video/abc80.c \
	src/emu/imagedev/printer.h \
	src/emu/sound/sn76477.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/bus/abcbus/abcbus.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/abc800.o : \
	src/emu/video/saa5050.h \
	src/emu/machine/rescap.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/sound/disc_dev.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/bus/abckb/abc800kb.h \
	src/emu/machine/e0516.h \
	src/mess/video/abc800.c \
	src/emu/sound/discrete.h \
	src/emu/machine/z80ctc.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/wavwrite.h \
	src/emu/video/mc6845.h \
	src/mess/includes/abc80x.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/abckb/abckb.h \
	src/emu/machine/z80dart.h \
	src/emu/bus/abcbus/abcbus.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/video/abc802.o : \
	src/emu/video/saa5050.h \
	src/emu/machine/rescap.h \
	src/emu/bus/rs232/rs232.h \
	src/mess/video/abc802.c \
	src/emu/sound/disc_dev.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/bus/abckb/abc800kb.h \
	src/emu/machine/e0516.h \
	src/emu/sound/discrete.h \
	src/emu/machine/z80ctc.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/wavwrite.h \
	src/emu/video/mc6845.h \
	src/mess/includes/abc80x.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/abckb/abckb.h \
	src/emu/machine/z80dart.h \
	src/emu/bus/abcbus/abcbus.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/video/abc806.o : \
	src/emu/video/saa5050.h \
	src/emu/machine/rescap.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/sound/disc_dev.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/bus/abckb/abc800kb.h \
	src/emu/machine/e0516.h \
	src/emu/sound/discrete.h \
	src/emu/machine/z80ctc.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/wavwrite.h \
	src/emu/video/mc6845.h \
	src/mess/includes/abc80x.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/abckb/abckb.h \
	src/emu/machine/z80dart.h \
	src/emu/bus/abcbus/abcbus.h \
	src/lib/formats/ioprocs.h \
	src/mess/video/abc806.c \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/video/ac1.o : \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/ac1.h \
	src/lib/formats/ioprocs.h \
	src/mess/video/ac1.c \

$(OBJ)/mess/video/advision.o : \
	src/mess/video/advision.c \
	src/mess/includes/advision.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/video/alesis.o : \
	src/emu/machine/nvram.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/includes/alesis.h \
	src/lib/formats/cassimg.h \
	src/mess/video/alesis.c \
	src/emu/cpu/mcs51/mcs51.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/video/hd44780.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/video/apollo.o : \
	src/emu/sound/saa1099.h \
	src/emu/video/crtc_ega.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/mess/machine/apollo_kbd.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/bus/rs232/rs232.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/emu/bus/isa/lpt.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/mc146818.h \
	src/lib/util/corealloc.h \
	src/emu/sound/2151intf.h \
	src/emu/bus/isa/gus.h \
	src/emu/bus/isa/ne2000.h \
	src/mess/includes/apollo.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/dp8390.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/bus/isa/ide.h \
	src/lib/softfloat/milieu.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/machine/keyboard.h \
	src/emu/bus/isa/finalchs.h \
	src/emu/imagedev/harddriv.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/lib/util/corefile.h \
	src/emu/bus/isa/ega.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/lib/util/opresolv.h \
	src/emu/machine/n68681.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/clock.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/terminal.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/emu/sound/beep.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/mess/video/apollo.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/video/apple1.o : \
	src/mess/video/apple1.c \
	src/mess/includes/apple1.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/apple2.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/video/apple2.c \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/bus/a2bus/a2eauxslot.h \
	src/mess/machine/applefdc.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/mess/machine/kb3600.h \
	src/mess/includes/apple2.h \
	src/emu/bus/a2bus/a2bus.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/apple2gs.o : \
	src/emu/cpu/m6502/m5074x.h \
	src/mess/includes/apple2gs.h \
	src/emu/machine/nvram.h \
	src/lib/util/corealloc.h \
	src/emu/sound/es5503.h \
	src/emu/cpu/m6502/m740.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/video/apple2gs.c \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/bus/a2bus/a2eauxslot.h \
	src/mess/machine/applefdc.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/mess/machine/kb3600.h \
	src/mess/includes/apple2.h \
	src/emu/bus/a2bus/a2bus.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/apple3.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/mos6551.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/m6502/m65c02.h \
	src/mess/includes/apple3.h \
	src/emu/bus/a2bus/a2eauxslot.h \
	src/mess/machine/applefdc.h \
	src/emu/machine/mm58167.h \
	src/mess/video/apple3.c \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6522via.h \
	src/mess/machine/kb3600.h \
	src/mess/includes/apple2.h \
	src/emu/machine/clock.h \
	src/emu/bus/a2bus/a2bus.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/video/aquarius.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/mess/video/aquarius.c \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/aquarius.h \
	src/emu/imagedev/cartslot.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/video/arcadia.o : \
	src/emu/cpu/s2650/s2650.h \
	src/emu/imagedev/cartslot.h \
	src/mess/video/arcadia.c \
	src/mess/includes/arcadia.h \
	src/mess/audio/arcadia.h \

$(OBJ)/mess/video/atarist.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/video/atarist.c \
	src/emu/bus/rs232/rs232.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/latch.h \
	src/emu/machine/ram.h \
	src/mess/video/atarist.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/lmc1992.h \
	src/mess/includes/atarist.h \
	src/lib/softfloat/milieu.h \
	src/emu/machine/mc68901.h \
	src/emu/machine/6850acia.h \
	src/lib/softfloat/softfloat.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/rp5c15.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/machine/8530scc.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/video/avigo.o : \
	src/emu/machine/nvram.h \
	src/emu/bus/rs232/rs232.h \
	src/mess/includes/avigo.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/rp5c01.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/ins8250.h \
	src/emu/imagedev/snapquik.h \
	src/emu/machine/intelfsh.h \
	src/mess/video/avigo.c \

$(OBJ)/mess/video/b2m.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/mess/video/b2m.c \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/includes/b2m.h \

$(OBJ)/mess/video/bbc.o : \
	src/emu/sound/tms5220.h \
	src/mess/includes/bbc.h \
	src/emu/video/saa5050.h \
	src/mess/machine/i8271.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/mess/video/bbc.c \
	src/lib/formats/cassimg.h \
	src/emu/machine/upd7002.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/spchrom.h \
	src/emu/machine/mc6854.h \
	src/emu/sound/sn76496.h \
	src/emu/machine/6850acia.h \
	src/emu/video/mc6845.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/6522via.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/bk.o : \
	src/lib/util/corealloc.h \
	src/mess/includes/bk.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/lib/util/coretmpl.h \
	src/mess/video/bk.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/busicom.o : \
	src/mess/includes/busicom.h \
	src/mess/video/busicom.c \
	src/emu/cpu/i4004/i4004.h \

$(OBJ)/mess/video/cgc7900.o : \
	src/lib/formats/flopimg.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/lib/softfloat/mamesf.h \
	src/mess/video/cgc7900.c \
	src/mess/includes/cgc7900.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/video/cgenie.o : \
	src/mess/includes/cgenie.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/lib/util/coretmpl.h \
	src/mess/video/cgenie.c \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/video/channelf.o : \
	src/emu/cpu/f8/f8.h \
	src/mess/audio/channelf.h \
	src/mess/includes/channelf.h \
	src/emu/imagedev/cartslot.h \
	src/mess/video/channelf.c \

$(OBJ)/mess/video/cirrus.o : \
	src/emu/video/pc_vga.h \
	src/emu/bus/pci/pci.h \
	src/mess/video/cirrus.c \
	src/mess/video/cirrus.h \

$(OBJ)/mess/video/comquest.o : \
	src/mess/video/comquest.c \
	src/mess/includes/comquest.h \

$(OBJ)/mess/video/comx35.o : \
	src/emu/machine/rescap.h \
	src/emu/sound/cdp1869.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/video/mc6845.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/emu/bus/comx35/exp.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/cdp1871.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/mess/video/comx35.c \
	src/mess/includes/comx35.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/crt.o : \
	src/mess/video/crt.h \
	src/mess/video/crt.c \

$(OBJ)/mess/video/dai.o : \
	src/emu/machine/pit8253.h \
	src/emu/machine/tms5501.h \
	src/mess/includes/dai.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/video/dai.c \
	src/emu/machine/i8255.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/dgn_beta.o : \
	src/emu/machine/ram.h \
	src/mess/video/dgn_beta.c \
	src/emu/machine/wd17xx.h \
	src/emu/machine/6821pia.h \
	src/emu/video/mc6845.h \
	src/mess/includes/dgn_beta.h \

$(OBJ)/mess/video/ef9345.o : \
	src/mess/video/ef9345.c \
	src/mess/video/ef9345.h \

$(OBJ)/mess/video/electron.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/includes/electron.h \
	src/lib/formats/cassimg.h \
	src/mess/video/electron.c \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/video/fm7.o : \
	src/emu/machine/buffer.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/mess/video/fm7.c \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/mess/includes/fm7.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/2203intf.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/video/fmtowns.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/mess/machine/upd71071.h \
	src/emu/machine/nvram.h \
	src/emu/sound/2612intf.h \
	src/lib/softfloat/softfloat-macros \
	src/lib/util/corealloc.h \
	src/mess/includes/fmtowns.h \
	src/emu/sound/rf5c68.h \
	src/emu/sound/speaker.h \
	src/lib/util/cdrom.h \
	src/emu/machine/ram.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/lib/softfloat/mamesf.h \
	src/lib/util/chd.h \
	src/mess/video/fmtowns.c \
	src/emu/machine/pic8259.h \
	src/emu/machine/scsibus.h \
	src/lib/util/simple_set.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/lib/softfloat/milieu.h \
	src/emu/cpu/i386/i386.h \
	src/emu/machine/scsidev.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/lib/softfloat/softfloat.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \
	src/emu/cpu/vtlb.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/mess/machine/fm_scsi.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/scsihle.h \
	src/lib/util/astring.h \

$(OBJ)/mess/video/galaxy.o : \
	src/mess/video/galaxy.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/mess/includes/galaxy.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/galeb.o : \
	src/mess/video/galeb.c \
	src/mess/includes/galeb.h \

$(OBJ)/mess/video/gamecom.o : \
	src/emu/cpu/sm8500/sm8500.h \
	src/mess/video/gamecom.c \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/dac.h \
	src/mess/includes/gamecom.h \

$(OBJ)/mess/video/gb_lcd.o : \
	src/mess/video/gb_lcd.h \
	src/mess/video/gb_lcd.c \

$(OBJ)/mess/video/gba.o : \
	src/mess/includes/gba.h \
	src/emu/bus/gba/gba_slot.h \
	src/mess/audio/gb.h \
	src/emu/machine/intelfsh.h \
	src/mess/video/gba.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/video/gf4500.o : \
	src/mess/video/gf4500.h \
	src/mess/video/gf4500.c \

$(OBJ)/mess/video/gime.o : \
	src/emu/video/mc6847.h \
	src/emu/machine/ram.h \
	src/mess/machine/6883sam.h \
	src/mess/video/gime.h \
	src/emu/bus/coco/cococart.h \
	src/mess/video/gime.c \

$(OBJ)/mess/video/hec2video.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/includes/hec2hrp.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/sn76477.h \
	src/mess/video/hec2video.c \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/video/hp48.o : \
	src/mess/video/hp48.c \
	src/mess/includes/hp48.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/video/intv.o : \
	src/emu/sound/sp0256.h \
	src/mess/video/intv.c \
	src/mess/video/stic.h \
	src/mess/includes/intv.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/video/k1ge.o : \
	src/mess/video/k1ge.c \
	src/mess/video/k1ge.h \

$(OBJ)/mess/video/kaypro.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/emu/machine/z80sio.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/kaypro.h \
	src/emu/machine/com8116.h \
	src/emu/video/mc6845.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/video/kaypro.c \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/video/kc.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/kc_cas.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/bus/kc/d004.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/z80ctc.h \
	src/lib/util/md5.h \
	src/emu/machine/upd765.h \
	src/emu/bus/kc/ram.h \
	src/emu/imagedev/harddriv.h \
	src/mess/video/kc.c \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/emu/sound/wave.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/kc/d002.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/kc_keyb.h \
	src/mess/includes/kc.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/bus/kc/rom.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \
	src/emu/bus/kc/kc.h \

$(OBJ)/mess/video/kramermc.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/mess/includes/kramermc.h \
	src/mess/video/kramermc.c \

$(OBJ)/mess/video/kyocera.o : \
	src/emu/machine/buffer.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/emu/machine/rp5c01.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/video/hd61830.h \
	src/emu/machine/im6402.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/hd44102.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/i8155.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/includes/kyocera.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/upd1990a.h \
	src/mess/video/kyocera.c \

$(OBJ)/mess/video/llc.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/mess/video/llc.c \
	src/mess/includes/llc.h \
	src/mess/machine/k7659kb.h \

$(OBJ)/mess/video/lviv.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/emu/imagedev/snapquik.h \
	src/mess/includes/lviv.h \
	src/lib/util/coretmpl.h \
	src/mess/video/lviv.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/mac.o : \
	src/mess/machine/egret.h \
	src/mess/machine/macpds.h \
	src/emu/sound/asc.h \
	src/mess/machine/macrtc.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/mess/video/mac.c \
	src/mess/includes/mac.h \
	src/emu/machine/ram.h \
	src/mess/machine/cuda.h \
	src/lib/softfloat/mamesf.h \
	src/mess/machine/ncr5380.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/sound/awacs.h \
	src/mess/machine/mackbd.h \
	src/emu/bus/nubus/nubus.h \
	src/mess/machine/8530scc.h \
	src/emu/machine/ncr539x.h \
	src/emu/machine/6522via.h \
	src/emu/machine/scsihle.h \

$(OBJ)/mess/video/mbc55x.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/emu/debug/debugcon.h \
	src/mess/includes/mbc55x.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/machine/latch.h \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/emu/debug/textbuf.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/mess/video/mbc55x.c \

$(OBJ)/mess/video/mbee.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/machine/mc146818.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/mess/video/mbee.c \
	src/emu/video/mc6845.h \
	src/mess/includes/mbee.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/video/mc80.o : \
	src/emu/machine/z80pio.h \
	src/mess/video/mc80.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/mess/includes/mc80.h \
	src/emu/machine/z80dart.h \

$(OBJ)/mess/video/microtan.o : \
	src/mess/includes/microtan.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/mess/video/microtan.c \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/6522via.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/mikro80.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mess/video/mikro80.c \
	src/mess/includes/mikro80.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/mikromik.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/rs232/rs232.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/machine/ram.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/i8212.h \
	src/emu/video/i8275x.h \
	src/lib/formats/imd_dsk.h \
	src/mess/includes/mikromik.h \
	src/lib/formats/mm_dsk.h \
	src/emu/machine/upd765.h \
	src/emu/video/upd7220.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/z80dart.h \
	src/mess/video/mikromik.c \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/video/mos6566.o : \
	src/mess/video/mos6566.h \
	src/mess/video/mos6566.c \

$(OBJ)/mess/video/mz700.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/pit8253.h \
	src/mess/video/mz700.c \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/mess/includes/mz700.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/mz80.o : \
	src/emu/machine/pit8253.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/mess/includes/mz80.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/mess/video/mz80.c \

$(OBJ)/mess/video/nascom1.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/mess/video/nascom1.c \
	src/mess/includes/nascom1.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/ay31015.h \

$(OBJ)/mess/video/nc.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/latch.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/mess/includes/nc.h \
	src/mess/video/nc.c \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/clock.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/video/nes.o : \
	src/emu/bus/nes/konami.h \
	src/emu/bus/nes/bootleg.h \
	src/emu/bus/nes/rexsoft.h \
	src/emu/bus/nes/karastudio.h \
	src/emu/bus/nes/act53.h \
	src/emu/bus/nes/mmc1.h \
	src/emu/bus/nes/txc.h \
	src/emu/bus/nes/sunsoft.h \
	src/emu/bus/nes/aladdin.h \
	src/emu/bus/nes/henggedianzi.h \
	src/emu/bus/nes/legacy.h \
	src/emu/bus/nes/pt554.h \
	src/emu/bus/nes/rcm.h \
	src/lib/util/corealloc.h \
	src/emu/sound/nes_apu.h \
	src/emu/bus/nes/namcot.h \
	src/emu/bus/nes/sunsoft_dcs.h \
	src/emu/imagedev/cassette.h \
	src/emu/bus/nes/mmc5.h \
	src/emu/bus/nes/discrete.h \
	src/emu/bus/nes/ave.h \
	src/lib/formats/cassimg.h \
	src/emu/bus/nes/jaleco.h \
	src/emu/bus/nes/nes_slot.h \
	src/emu/bus/nes/hes.h \
	src/emu/bus/nes/jy.h \
	src/emu/bus/nes/racermate.h \
	src/emu/bus/nes/bandai.h \
	src/emu/bus/nes/hosenkan.h \
	src/emu/bus/nes/nanjing.h \
	src/emu/bus/nes/datach.h \
	src/emu/bus/nes/sachen.h \
	src/emu/bus/nes/mmc3_clones.h \
	src/emu/bus/nes/cony.h \
	src/emu/bus/nes/somari.h \
	src/emu/machine/bcreader.h \
	src/emu/sound/2413intf.h \
	src/emu/bus/nes/nes_carts.h \
	src/emu/sound/samples.h \
	src/emu/sound/nes_defs.h \
	src/emu/bus/nes/pirate.h \
	src/mess/includes/nes.h \
	src/emu/bus/nes/taito.h \
	src/emu/bus/nes/nxrom.h \
	src/emu/bus/nes/mmc3.h \
	src/emu/bus/nes/multigame.h \
	src/emu/bus/nes/camerica.h \
	src/emu/bus/nes/kaiser.h \
	src/mame/video/ppu2c0x.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/nes/ggenie.h \
	src/emu/bus/nes/waixing.h \
	src/emu/bus/nes/tengen.h \
	src/mess/video/nes.c \
	src/emu/bus/nes/ntdec.h \
	src/emu/bus/nes/benshieng.h \
	src/emu/bus/nes/event.h \
	src/emu/bus/nes/irem.h \
	src/emu/sound/vrc6.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/ay8910.h \
	src/emu/bus/nes/cne.h \
	src/emu/bus/nes/mmc2.h \
	src/emu/machine/i2cmem.h \

$(OBJ)/mess/video/newbrain.o : \
	src/emu/machine/adc0808.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/cpu/cop400/cop400.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/z80sio.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/mess/includes/newbrain.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/video/newbrain.c \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/video/newport.o : \
	src/mess/video/newport.h \
	src/mess/video/newport.c \

$(OBJ)/mess/video/nick.o : \
	src/emu/video/resnet.h \
	src/emu/machine/rescap.h \
	src/mess/video/nick.c \
	src/mess/video/nick.h \

$(OBJ)/mess/video/ondra.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/ondra.h \
	src/lib/util/coretmpl.h \
	src/mess/video/ondra.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/orao.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/orao.h \
	src/mess/video/orao.c \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/video/oric.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/machine/mos6551.h \
	src/emu/imagedev/cassette.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/mess/video/oric.c \
	src/mess/machine/applefdc.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/formats/ap2_dsk.h \
	src/mess/includes/oric.h \
	src/emu/machine/6522via.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/machine/clock.h \
	src/lib/formats/oric_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/oric_tap.h \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/video/orion.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/mc146818.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/machine/wd_fdc.h \
	src/emu/video/i8275.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/orion.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/i8255.h \
	src/mess/includes/radio86.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/8257dma.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/video/orion.c \
	src/emu/sound/ay8910.h \

$(OBJ)/mess/video/osi.o : \
	src/mess/video/osi.c \
	src/lib/formats/flopimg.h \
	src/emu/machine/rescap.h \
	src/mess/includes/osi.h \
	src/emu/sound/disc_dev.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/6821pia.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/machine/6850acia.h \
	src/emu/sound/wavwrite.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/lib/formats/basicdsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/beep.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/video/p2000m.o : \
	src/emu/video/saa5050.h \
	src/mess/video/p2000m.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/p2000t.h \

$(OBJ)/mess/video/pc1251.o : \
	src/emu/machine/nvram.h \
	src/mess/includes/pocketc.h \
	src/emu/cpu/sc61860/sc61860.h \
	src/mess/video/pc1251.c \
	src/mess/includes/pc1251.h \

$(OBJ)/mess/video/pc1350.o : \
	src/emu/machine/nvram.h \
	src/mess/includes/pc1350.h \
	src/emu/machine/ram.h \
	src/mess/includes/pocketc.h \
	src/emu/cpu/sc61860/sc61860.h \
	src/mess/video/pc1350.c \

$(OBJ)/mess/video/pc1401.o : \
	src/mess/includes/pc1401.h \
	src/emu/machine/nvram.h \
	src/mess/includes/pocketc.h \
	src/emu/cpu/sc61860/sc61860.h \
	src/mess/video/pc1401.c \

$(OBJ)/mess/video/pc1403.o : \
	src/emu/machine/nvram.h \
	src/mess/video/pc1403.c \
	src/mess/includes/pocketc.h \
	src/emu/cpu/sc61860/sc61860.h \
	src/mess/includes/pc1403.h \

$(OBJ)/mess/video/pc1512.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/sound/saa1099.h \
	src/emu/machine/buffer.h \
	src/emu/bus/isa/xtide.h \
	src/emu/bus/isa/gblaster.h \
	src/emu/video/crtc_ega.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/emu/bus/midi/midi.h \
	src/emu/machine/wd11c00_17.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/ibm_mfc.h \
	src/emu/bus/isa/pds.h \
	src/emu/video/pc_vga.h \
	src/emu/bus/isa/wdxt_gen.h \
	src/mess/video/pc1512.c \
	src/emu/bus/isa/lpt.h \
	src/lib/formats/upd765_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/sound/2151intf.h \
	src/emu/machine/mc146818.h \
	src/emu/machine/latch.h \
	src/emu/bus/isa/gus.h \
	src/emu/bus/isa/ne2000.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/bus/isa/hdc.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/emu/bus/pc_joy/pc_joy.h \
	src/emu/bus/isa/svga_s3.h \
	src/emu/machine/dp8390.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/bus/isa/isa_cards.h \
	src/emu/machine/pc_fdc.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/idectrl.h \
	src/emu/cpu/tms32010/tms32010.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/adlib.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/bus/isa/ide.h \
	src/emu/bus/isa/cga.h \
	src/emu/sound/3812intf.h \
	src/emu/sound/mos6581.h \
	src/emu/bus/isa/3c503.h \
	src/emu/machine/ins8250.h \
	src/emu/bus/isa/vga_ati.h \
	src/emu/machine/mpu401.h \
	src/emu/bus/isa/mpu401.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/i86/i86.h \
	src/emu/video/mc6845.h \
	src/emu/bus/isa/svga_cirrus.h \
	src/emu/bus/isa/ne1000.h \
	src/emu/bus/isa/finalchs.h \
	src/emu/imagedev/harddriv.h \
	src/emu/bus/isa/mda.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/emu/bus/isa/pc1640_iga.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/isa/ega.h \
	src/mess/includes/pc1512.h \
	src/emu/machine/wd2010.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/bus/isa/fdc.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/s3virge.h \
	src/emu/bus/isa/vga.h \
	src/emu/bus/isa/side116.h \
	src/emu/bus/isa/wd1002a_wx1.h \
	src/emu/bus/isa/ssi2001.h \
	src/emu/cpu/i86/i186.h \
	src/emu/bus/isa/dectalk.h \
	src/emu/bus/isa/svga_tseng.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/emu/bus/isa/sblaster.h \
	src/mess/machine/pc1512kb.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/emu/bus/isa/aha1542.h \
	src/lib/formats/d88_dsk.h \
	src/emu/bus/isa/mufdc.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/stereo_fx.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/video/pc4.o : \
	src/mess/includes/pc4.h \
	src/mess/video/pc4.c \
	src/emu/sound/beep.h \

$(OBJ)/mess/video/pc8401a.o : \
	src/mess/video/pc8401a.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/emu/video/sed1330.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/mess/includes/pc8401a.h \
	src/emu/machine/upd1990a.h \

$(OBJ)/mess/video/pc_aga.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/pit8253.h \
	src/mess/video/pc_aga.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/emu/machine/ins8250.h \
	src/emu/machine/upd765.h \
	src/emu/video/mc6845.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/pc_kbd/pc_kbdc.h \
	src/lib/util/opresolv.h \
	src/mess/includes/pc.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/pc_lpt.h \
	src/emu/imagedev/floppy.h \
	src/emu/video/cgapal.h \
	src/emu/video/pc_cga.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/am9517a.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/mess/includes/amstr_pc.h \
	src/mess/video/pc_aga.c \

$(OBJ)/mess/video/pc_t1t.o : \
	src/emu/machine/ram.h \
	src/emu/machine/pic8259.h \
	src/emu/video/mc6845.h \
	src/mess/video/pc_t1t.c \
	src/mess/video/pc_t1t.h \

$(OBJ)/mess/video/pcw.o : \
	src/lib/formats/flopimg.h \
	src/mess/includes/pcw.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/mess/video/pcw.c \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/video/pcw16.o : \
	src/lib/formats/pc_dsk.h \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/mess/includes/pcw16.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/ins8250.h \
	src/emu/machine/upd765.h \
	src/emu/machine/pckeybrd.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/machine/pc_lpt.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/video/pcw16.c \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/video/pdp1.o : \
	src/mess/includes/pdp1.h \
	src/emu/cpu/pdp1/pdp1.h \
	src/mess/video/crt.h \
	src/mess/video/pdp1.c \

$(OBJ)/mess/video/pds_tpdfpd.o : \
	src/mess/machine/macpds.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/mess/video/pds_tpdfpd.c \
	src/lib/softfloat/softfloat.h \
	src/mess/video/pds_tpdfpd.h \

$(OBJ)/mess/video/pecom.o : \
	src/emu/sound/cdp1869.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/video/pecom.c \
	src/mess/includes/pecom.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/pk8020.o : \
	src/emu/machine/pit8253.h \
	src/mess/video/pk8020.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/pic8259.h \
	src/mess/includes/pk8020.h \
	src/emu/machine/i8255.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/pmd85.o : \
	src/emu/machine/pit8253.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/video/pmd85.c \
	src/emu/machine/i8255.h \
	src/mess/includes/pmd85.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/pocketc.o : \
	src/mess/includes/pocketc.h \
	src/mess/video/pocketc.c \

$(OBJ)/mess/video/poisk1.o : \
	src/emu/bus/isa/p1_hdc.h \
	src/emu/bus/isa/xtide.h \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/emu/machine/eeprompar.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/isa/mc1502_rom.h \
	src/emu/bus/isa/lpt.h \
	src/mess/includes/poisk1.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/lib/util/sha1.h \
	src/emu/machine/atadev.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/bus/isa/com.h \
	src/emu/machine/pic8259.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/isa/p1_fdc.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isa/isa.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/ataintf.h \
	src/mess/video/poisk1.c \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/wd2010.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/isa/xsu_cards.h \
	src/emu/video/cgapal.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/isa/mc1502_fdc.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \
	src/emu/bus/isa/p1_rom.h \

$(OBJ)/mess/video/poly88.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/i8251.h \
	src/lib/formats/cassimg.h \
	src/mess/includes/poly88.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/mess/video/poly88.c \

$(OBJ)/mess/video/pp01.o : \
	src/emu/machine/pit8253.h \
	src/mess/video/pp01.c \
	src/emu/machine/ram.h \
	src/emu/machine/i8251.h \
	src/emu/sound/speaker.h \
	src/emu/machine/i8255.h \
	src/mess/includes/pp01.h \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/mess/video/primo.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/lib/formats/cassimg.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/mess/video/primo.c \
	src/mess/includes/primo.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/radio86.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/video/i8275.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mess/includes/radio86.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/8257dma.h \
	src/mess/video/radio86.c \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/rm380z.o : \
	src/lib/formats/flopimg.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/mess/video/rm380z.c \
	src/emu/machine/keyboard.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/mess/includes/rm380z.h \

$(OBJ)/mess/video/rmnimbus.o : \
	src/emu/machine/buffer.h \
	src/emu/debug/debugcon.h \
	src/emu/sound/msm5205.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/machine/er59256.h \
	src/emu/machine/z80sio.h \
	src/emu/machine/wd17xx.h \
	src/lib/util/simple_set.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/scsicb.h \
	src/emu/cpu/i86/i86.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \
	src/emu/debug/textbuf.h \
	src/emu/machine/6522via.h \
	src/emu/bus/centronics/ctronics.h \
	src/mess/includes/rmnimbus.h \
	src/emu/cpu/i86/i186.h \
	src/emu/sound/ay8910.h \
	src/mess/video/rmnimbus.c \

$(OBJ)/mess/video/samcoupe.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/mess/video/samcoupe.c \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/mess/includes/samcoupe.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/bus/centronics/ctronics.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/video/special.o : \
	src/emu/machine/pit8253.h \
	src/lib/formats/flopimg.h \
	src/mess/includes/special.h \
	src/lib/formats/rk_cas.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/mess/video/special.c \
	src/lib/formats/smx_dsk.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/video/spectrum.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/mess/video/spectrum.c \
	src/lib/formats/imd_dsk.h \
	src/mess/includes/spectrum.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/video/ssystem3.o : \
	src/mess/includes/ssystem3.h \
	src/mess/video/ssystem3.c \

$(OBJ)/mess/video/stic.o : \
	src/mess/video/stic.c \
	src/mess/video/stic.h \

$(OBJ)/mess/video/super80.o : \
	src/emu/machine/buffer.h \
	src/emu/machine/z80pio.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80daisy.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/includes/super80.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/mess/video/super80.c \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/t6a04.o : \
	src/mess/video/t6a04.c \
	src/mess/video/t6a04.h \

$(OBJ)/mess/video/thomson.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/emu/machine/mc6843.h \
	src/emu/bus/rs232/rs232.h \
	src/mess/machine/thomflop.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/machine/mos6551.h \
	src/emu/imagedev/cassette.h \
	src/mess/audio/mea8000.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/mc6854.h \
	src/mess/includes/thomson.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/6850acia.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/mc6846.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6809/m6809.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/lib/formats/thom_dsk.h \
	src/emu/machine/clock.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/thom_cas.h \
	src/mess/video/thomson.c \
	src/emu/sound/dac.h \

$(OBJ)/mess/video/ti85.o : \
	src/mess/video/ti85.c \
	src/emu/sound/speaker.h \
	src/emu/imagedev/snapquik.h \
	src/mess/video/t6a04.h \
	src/mess/includes/ti85.h \

$(OBJ)/mess/video/timex.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/imd_dsk.h \
	src/mess/includes/spectrum.h \
	src/emu/imagedev/cartslot.h \
	src/emu/machine/upd765.h \
	src/emu/imagedev/snapquik.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/mess/video/timex.c \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/video/tmc1800.o : \
	src/mess/video/tmc1800.c \
	src/emu/video/resnet.h \
	src/emu/machine/rescap.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/emu/video/cdp1861.h \
	src/lib/formats/cassimg.h \
	src/emu/sound/cdp1864.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/lib/formats/ioprocs.h \
	src/mess/includes/tmc1800.h \
	src/emu/sound/beep.h \

$(OBJ)/mess/video/tmc600.o : \
	src/emu/machine/buffer.h \
	src/mess/video/tmc600.c \
	src/emu/sound/cdp1869.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/mess/includes/tmc600.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/snapquik.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/cosmac/cosmac.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/trs80.o : \
	src/emu/machine/buffer.h \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd17xx.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/trs_dsk.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/mess/includes/trs80.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/mess/video/trs80.c \
	src/lib/formats/trs_cas.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/ay31015.h \

$(OBJ)/mess/video/tx0.o : \
	src/mess/includes/tx0.h \
	src/mess/video/tx0.c \
	src/mess/video/crt.h \
	src/emu/cpu/pdp1/tx0.h \

$(OBJ)/mess/video/ut88.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/i8255.h \
	src/mess/video/ut88.c \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \
	src/mess/includes/ut88.h \
	src/emu/sound/dac.h \

$(OBJ)/mess/video/uv201.o : \
	src/mess/video/uv201.h \
	src/mess/video/uv201.c \

$(OBJ)/mess/video/v1050.o : \
	src/emu/machine/buffer.h \
	src/mess/machine/v1050kb.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/emu/machine/rescap.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/sound/disc_dev.h \
	src/lib/util/corealloc.h \
	src/emu/machine/latch.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/i8251.h \
	src/emu/machine/ram.h \
	src/lib/util/sha1.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/machine/msm58321.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/sound/wavwrite.h \
	src/emu/video/mc6845.h \
	src/emu/imagedev/harddriv.h \
	src/mess/video/v1050.c \
	src/emu/machine/i8214.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/mess/includes/v1050.h \
	src/emu/bus/centronics/ctronics.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/clock.h \
	src/mess/machine/s1410.h \
	src/emu/machine/scsihd.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/astring.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/mess/video/vc4000.o : \
	src/mess/video/vc4000.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/mess/includes/vc4000.h \
	src/emu/cpu/s2650/s2650.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/cartslot.h \
	src/emu/imagedev/snapquik.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/vector06.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/mess/video/vector06.c \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/sound/wave.h \
	src/mess/includes/vector06.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/basicdsk.h \

$(OBJ)/mess/video/vic4567.o : \
	src/mess/video/vic4567.c \
	src/mess/video/vic4567.h \

$(OBJ)/mess/video/vtech2.o : \
	src/mess/video/vtech2.c \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/sound/speaker.h \
	src/mess/includes/vtech2.h \
	src/lib/formats/cassimg.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/vtvideo.o : \
	src/mess/video/vtvideo.h \
	src/mess/video/vtvideo.c \

$(OBJ)/mess/video/wswan.o : \
	src/mess/video/wswan.c \
	src/emu/machine/nvram.h \
	src/mess/includes/wswan.h \
	src/emu/imagedev/cartslot.h \
	src/emu/cpu/v30mz/v30mz.h \

$(OBJ)/mess/video/x68k.o : \
	src/lib/formats/flopimg.h \
	src/mess/video/x68k.c \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/mess/includes/x68k.h \
	src/emu/machine/ram.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/mc68901.h \
	src/emu/machine/upd765.h \
	src/emu/machine/rp5c15.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/sound/okim6258.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/mess/video/z88.o : \
	src/mess/includes/z88.h \
	src/emu/bus/z88/flash.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ram.h \
	src/emu/sound/speaker.h \
	src/emu/cpu/z80/z80.h \
	src/mess/machine/upd65031.h \
	src/emu/bus/z88/z88.h \
	src/emu/machine/intelfsh.h \
	src/mess/video/z88.c \
	src/emu/bus/z88/ram.h \
	src/emu/bus/z88/rom.h \

$(OBJ)/mess/video/zx.o : \
	src/lib/util/corealloc.h \
	src/emu/imagedev/cassette.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/sound/speaker.h \
	src/emu/machine/ram.h \
	src/lib/formats/zx81_p.h \
	src/lib/formats/cassimg.h \
	src/emu/cpu/z80/z80.h \
	src/mess/video/zx.c \
	src/mess/includes/zx.h \
	src/emu/sound/wave.h \
	src/lib/util/coretmpl.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/mess/video/zx8301.o : \
	src/mess/video/zx8301.c \
	src/mess/video/zx8301.h \

$(OBJ)/mess/mess.o : \
	src/mess/mess.c \
