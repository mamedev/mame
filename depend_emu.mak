
$(OBJ)/emu/bus/abcbus/abc890.o : \
	src/emu/bus/abcbus/abc890.c \
	src/emu/bus/abcbus/abc890.h \
	src/emu/bus/abcbus/abcbus.h \

$(OBJ)/emu/bus/abcbus/abcbus.o : \
	src/emu/bus/abcbus/abcbus.c \
	src/emu/bus/abcbus/abcbus.h \

$(OBJ)/emu/bus/abcbus/dos.o : \
	src/emu/bus/abcbus/dos.c \

$(OBJ)/emu/bus/abcbus/fd2.o : \
	src/emu/bus/abcbus/fd2.c \

$(OBJ)/emu/bus/abcbus/hdc.o : \
	src/emu/bus/abcbus/hdc.c \

$(OBJ)/emu/bus/abcbus/lux10828.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/z80pio.h \
	src/emu/bus/abcbus/lux10828.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/abcbus/lux10828.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/bus/abcbus/abcbus.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/bus/abcbus/lux21046.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/abcbus/lux21046.c \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/abcbus/lux21046.h \
	src/emu/machine/z80dma.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/bus/abcbus/abcbus.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/bus/abcbus/sio.o : \
	src/emu/bus/abcbus/sio.c \

$(OBJ)/emu/bus/abcbus/slutprov.o : \
	src/emu/bus/abcbus/slutprov.c \

$(OBJ)/emu/bus/abcbus/turbo.o : \
	src/emu/bus/abcbus/turbo.c \

$(OBJ)/emu/bus/abcbus/uni800.o : \
	src/emu/bus/abcbus/uni800.c \

$(OBJ)/emu/bus/abcbus/xebec.o : \
	src/emu/bus/abcbus/xebec.c \

$(OBJ)/emu/bus/adamnet/adamnet.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/adamnet/ddp.h \
	src/emu/bus/adamnet/spi.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/adamnet/fdc.h \
	src/emu/cpu/m6800/m6800.h \
	src/lib/formats/adam_cas.h \
	src/emu/bus/adamnet/adamnet.c \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/bus/adamnet/adamnet.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/adamnet/kb.h \
	src/emu/bus/adamnet/printer.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/pool.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/adam_dsk.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/bus/adamnet/ddp.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/adamnet/ddp.h \
	src/emu/bus/adamnet/spi.h \
	src/emu/imagedev/cassette.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/cassimg.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/adamnet/fdc.h \
	src/emu/cpu/m6800/m6800.h \
	src/lib/formats/adam_cas.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/bus/adamnet/adamnet.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/adamnet/kb.h \
	src/emu/bus/adamnet/printer.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/pool.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/adam_dsk.h \
	src/lib/formats/d88_dsk.h \
	src/emu/bus/adamnet/ddp.c \

$(OBJ)/emu/bus/adamnet/fdc.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/adamnet/ddp.h \
	src/emu/bus/adamnet/spi.h \
	src/emu/bus/adamnet/fdc.c \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/adamnet/fdc.h \
	src/emu/cpu/m6800/m6800.h \
	src/lib/formats/adam_cas.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/adamnet/adamnet.h \
	src/lib/util/opresolv.h \
	src/emu/bus/adamnet/kb.h \
	src/emu/bus/adamnet/printer.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/pool.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/adam_dsk.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/bus/adamnet/kb.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/adamnet/ddp.h \
	src/emu/bus/adamnet/spi.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/adamnet/fdc.h \
	src/emu/cpu/m6800/m6800.h \
	src/lib/formats/adam_cas.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/bus/adamnet/adamnet.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/adamnet/printer.h \
	src/emu/bus/adamnet/kb.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/pool.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/adam_dsk.h \
	src/lib/formats/d88_dsk.h \
	src/emu/bus/adamnet/kb.c \

$(OBJ)/emu/bus/adamnet/printer.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/adamnet/ddp.h \
	src/emu/bus/adamnet/spi.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/adamnet/fdc.h \
	src/emu/cpu/m6800/m6800.h \
	src/lib/formats/adam_cas.h \
	src/emu/bus/adamnet/printer.c \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/bus/adamnet/adamnet.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/adamnet/kb.h \
	src/emu/bus/adamnet/printer.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/pool.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/adam_dsk.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/bus/adamnet/spi.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/adamnet/ddp.h \
	src/emu/bus/adamnet/spi.h \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/adamnet/spi.c \
	src/emu/bus/adamnet/fdc.h \
	src/emu/cpu/m6800/m6800.h \
	src/lib/formats/adam_cas.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/bus/adamnet/adamnet.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/adamnet/kb.h \
	src/emu/bus/adamnet/printer.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/pool.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/adam_dsk.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/bus/c64/16kb.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/bus/c64/16kb.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/16kb.c \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/4cga.o : \
	src/emu/bus/c64/4tba.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/c64/4dxh.h \
	src/emu/imagedev/printer.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/c64/4cga.c \
	src/emu/bus/c64/geocable.h \
	src/mess/machine/serial.h \
	src/emu/bus/c64/4cga.h \

$(OBJ)/emu/bus/c64/4dxh.o : \
	src/emu/bus/c64/4tba.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/c64/4dxh.c \
	src/emu/bus/c64/4dxh.h \
	src/emu/imagedev/printer.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/c64/geocable.h \
	src/mess/machine/serial.h \
	src/emu/bus/c64/4cga.h \

$(OBJ)/emu/bus/c64/4ksa.o : \
	src/emu/bus/c64/4tba.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/c64/4dxh.h \
	src/emu/imagedev/printer.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/geocable.h \
	src/mess/machine/serial.h \
	src/emu/bus/c64/4cga.h \
	src/emu/bus/c64/4ksa.c \

$(OBJ)/emu/bus/c64/4tba.o : \
	src/emu/bus/c64/4tba.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/c64/4dxh.h \
	src/emu/imagedev/printer.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/c64/geocable.h \
	src/mess/machine/serial.h \
	src/emu/bus/c64/4tba.c \
	src/emu/bus/c64/4cga.h \

$(OBJ)/emu/bus/c64/bn1541.o : \
	src/emu/bus/c64/4tba.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/c64/bn1541.c \
	src/emu/bus/c64/4dxh.h \
	src/emu/imagedev/printer.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/c64/geocable.h \
	src/mess/machine/serial.h \
	src/emu/bus/c64/4cga.h \

$(OBJ)/emu/bus/c64/c128_comal80.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/c128_comal80.c \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/comal80.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/bus/c64/comal80.c \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/bus/c64/comal80.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/cpm.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/bus/c64/cpm.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/bus/c64/cpm.c \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/currah_speech.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/currah_speech.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/bus/c64/super_games.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/currah_speech.c \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/dela_ep256.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep256.c \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/dela_ep64.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/bus/c64/dela_ep64.c \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/dela_ep7x8.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/bus/c64/dela_ep7x8.c \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/dinamic.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \
	src/emu/bus/c64/dinamic.c \

$(OBJ)/emu/bus/c64/dqbb.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/nvram.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/dqbb.c \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/machine/com8116.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/easy_calc_result.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/emu/bus/c64/easy_calc_result.c \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/easyflash.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/bus/c64/easyflash.c \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/machine/intelfsh.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/epyx_fast_load.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/c64/epyx_fast_load.c \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/exos.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/exos.c \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/bus/c64/exos.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/exp.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exp.c \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/fcc.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/bus/c64/fcc.c \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/fcc.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/bus/c64/supercpu.h \
	src/emu/cpu/m6502/r65c02.h \

$(OBJ)/emu/bus/c64/final.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/final.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/emu/bus/c64/final.c \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/final3.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/bus/c64/final3.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/emu/bus/c64/final3.c \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/fun_play.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/fun_play.c \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/emu/bus/c64/fun_play.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/geocable.o : \
	src/emu/bus/c64/4tba.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/c64/4dxh.h \
	src/emu/imagedev/printer.h \
	src/emu/bus/c64/geocable.c \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/c64/geocable.h \
	src/mess/machine/serial.h \
	src/emu/bus/c64/4cga.h \

$(OBJ)/emu/bus/c64/georam.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/georam.c \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/bus/c64/georam.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/ide64.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/cpu/g65816/g65816.h \
	src/lib/util/palette.h \
	src/emu/bus/c64/ide64.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/c64/ide64.c \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/emu/machine/mos8726.h \
	src/lib/util/chd.h \
	src/emu/bus/c64/prophet64.h \
	src/emu/machine/ds1302.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/lib/util/md5.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/machine/ataintf.h \
	src/emu/bus/c64/georam.h \
	src/emu/bus/c64/kingsoft.h \
	src/lib/util/corefile.h \
	src/emu/machine/mos6530.h \
	src/emu/bus/c64/rex.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/machine/intelfsh.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/emu/sound/t6721a.h \
	src/lib/util/astring.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/ieee488.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/currah_speech.h \
	src/emu/bus/c64/midi_sci.h \
	src/lib/util/palette.h \
	src/emu/bus/c64/ide64.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/ieee488/hardbox.h \
	src/emu/bus/c64/c128_comal80.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/ieee488.c \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/machine/i8255.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/com8116.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/mess/includes/corvushd.h \
	src/lib/formats/imageutl.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/bus/c64/comal80.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/16kb.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/final3.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/machine/intelfsh.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/ieee488/d9060.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/kingsoft.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/kingsoft.c \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/mach5.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/mach5.c \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/bus/c64/mach5.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/magic_desk.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/magic_desk.c \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/magic_formel.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/magic_formel.c \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/magic_voice.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/40105.h \
	src/emu/machine/i8251.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/magic_voice.c \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/midi_maplin.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/c64/ross.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/midi_maplin.c \
	src/emu/bus/c64/fun_play.h \
	src/mess/machine/null_modem.h \
	src/emu/imagedev/midiout.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/mess/machine/midioutport.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/midi_namesoft.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/c64/ross.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/mess/machine/null_modem.h \
	src/emu/imagedev/midiout.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/midi_namesoft.c \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/mess/machine/midioutport.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/midi_passport.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/c64/ross.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/bus/c64/midi_passport.c \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/mess/machine/null_modem.h \
	src/emu/imagedev/midiout.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/mess/machine/midioutport.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/midi_sci.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/c64/ross.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/midi_sci.c \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/mess/machine/null_modem.h \
	src/emu/imagedev/midiout.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/mess/machine/midioutport.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/midi_siel.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/c64/ross.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/mess/machine/null_modem.h \
	src/emu/imagedev/midiout.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/bus/c64/midi_siel.c \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/mess/machine/midioutport.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/mikro_assembler.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/mikro_assembler.c \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/multiscreen.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/emu/cpu/m6800/m6800.h \
	src/mess/machine/cbm_crt.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/bus/c64/multiscreen.c \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/super_explode.h \
	src/emu/bus/c64/multiscreen.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/music64.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/ps64.h \
	src/emu/bus/c64/music64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/emu/bus/c64/music64.c \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/neoram.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/imagedev/cartslot.h \
	src/emu/bus/c64/neoram.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/neoram.c \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/ocean.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/bus/c64/ocean.c \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/pagefox.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/pagefox.c \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/partner.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/partner.c \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/prophet64.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/prophet64.c \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/ps64.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/video/mc6845.h \
	src/emu/sound/samples.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/ps64.c \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/reu.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/emu/bus/c64/reu.c \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/reu.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/rex.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/bus/c64/rex.c \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/rex_ep256.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/rex_ep256.c \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/ross.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/ross.c \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/sfx_sound_expander.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/bus/c64/westermann.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/bus/c64/sfx_sound_expander.c \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/silverrock.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/bus/c64/silverrock.c \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/simons_basic.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/emu/bus/c64/simons_basic.c \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/tdos.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/stardos.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/stardos.c \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/std.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/std.c \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/structured_basic.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/structured_basic.c \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/super_explode.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/bus/c64/super_explode.c \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/super_games.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/emu/bus/c64/super_games.c \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/supercpu.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/bus/c64/supercpu.c \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/bus/c64/system3.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/sw8k.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/sw8k.c \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/machine/mos6551.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/swiftlink.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/swiftlink.c \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/system3.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/bus/c64/system3.c \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/tdos.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/mess/machine/serial.h \
	src/emu/machine/mc6852.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/bus/c64/tdos.c \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/turbo232.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/emu/bus/c64/warp_speed.h \
	src/mess/machine/terminal.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/emu/bus/c64/turbo232.c \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/user.o : \
	src/emu/bus/c64/4tba.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/c64/4dxh.h \
	src/emu/bus/c64/user.c \
	src/emu/imagedev/printer.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/c64/geocable.h \
	src/mess/machine/serial.h \
	src/emu/bus/c64/4cga.h \

$(OBJ)/emu/bus/c64/vic1011.o : \
	src/emu/bus/c64/4tba.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/c64/vic1011.c \
	src/emu/bus/c64/4dxh.h \
	src/emu/imagedev/printer.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/c64/geocable.h \
	src/mess/machine/serial.h \
	src/emu/bus/c64/4cga.h \

$(OBJ)/emu/bus/c64/vizastar.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/vizastar.c \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/vw64.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/vw64.c \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/warp_speed.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/emu/bus/c64/warp_speed.c \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/westermann.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/westermann.c \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/xl80.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/xl80.c \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/c64/zaxxon.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/imagedev/midiin.h \
	src/mess/machine/midiinport.h \
	src/emu/bus/c64/ross.h \
	src/emu/bus/c64/vizastar.h \
	src/emu/bus/c64/currah_speech.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/c64/midi_sci.h \
	src/emu/bus/c64/ide64.h \
	src/lib/util/palette.h \
	src/emu/cpu/g65816/g65816.h \
	src/emu/bus/c64/music64.h \
	src/emu/bus/c64/ps64.h \
	src/emu/sound/sp0256.h \
	src/emu/bus/c64/super_games.h \
	src/emu/bus/c64/dinamic.h \
	src/emu/machine/nvram.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/c64/ieee488.h \
	src/emu/bus/c64/dela_ep256.h \
	src/emu/bus/c64/std.h \
	src/emu/bus/c64/swiftlink.h \
	src/emu/bus/c64/easy_calc_result.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/c64/rex_ep256.h \
	src/emu/bus/c64/sw8k.h \
	src/emu/machine/mos6551.h \
	src/emu/machine/i8251.h \
	src/emu/machine/40105.h \
	src/emu/bus/c64/midi_namesoft.h \
	src/emu/bus/c64/ocean.h \
	src/emu/bus/c64/structured_basic.h \
	src/emu/bus/c64/zaxxon.h \
	src/emu/bus/c64/c128_comal80.h \
	src/emu/bus/ieee488/hardbox.h \
	src/mess/machine/keyboard.h \
	src/emu/bus/c64/cpm.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/bus/c64/magic_formel.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/c64/fun_play.h \
	src/emu/imagedev/midiout.h \
	src/mess/machine/null_modem.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/c64/epyx_fast_load.h \
	src/lib/util/chd.h \
	src/emu/machine/mos8726.h \
	src/emu/machine/ds1302.h \
	src/emu/bus/c64/prophet64.h \
	src/emu/bus/c64/zaxxon.c \
	src/mess/machine/cbm_crt.h \
	src/emu/cpu/m6800/m6800.h \
	src/emu/sound/votrax.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/midi_maplin.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/bus/c64/final.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/c64/dqbb.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/c64/magic_desk.h \
	src/emu/bus/c64/pagefox.h \
	src/emu/bus/c64/comal80.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/bus/c64/reu.h \
	src/emu/sound/3526intf.h \
	src/emu/bus/c64/westermann.h \
	src/emu/bus/c64/16kb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/c64/neoram.h \
	src/emu/imagedev/cartslot.h \
	src/emu/sound/samples.h \
	src/emu/video/mc6845.h \
	src/emu/bus/c64/magic_voice.h \
	src/emu/bus/c64/xl80.h \
	src/emu/bus/c64/dela_ep64.h \
	src/emu/bus/c64/easyflash.h \
	src/emu/bus/c64/sfx_sound_expander.h \
	src/emu/bus/c64/exp.h \
	src/emu/imagedev/harddriv.h \
	src/mess/machine/terminal.h \
	src/emu/bus/c64/warp_speed.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/emu/machine/6840ptm.h \
	src/emu/bus/c64/turbo232.h \
	src/emu/bus/c64/fcc.h \
	src/emu/machine/ataintf.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/imagedev/bitbngr.h \
	src/emu/bus/c64/georam.h \
	src/lib/util/corefile.h \
	src/emu/bus/c64/kingsoft.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/c64/rex.h \
	src/emu/bus/c64/final3.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/mach5.h \
	src/emu/bus/c64/simons_basic.h \
	src/emu/bus/c64/tdos.h \
	src/emu/machine/intelfsh.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/machine/6525tpi.h \
	src/emu/bus/c64/partner.h \
	src/emu/bus/c64/silverrock.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/bus/c64/system3.h \
	src/emu/bus/c64/exos.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/bus/c64/stardos.h \
	src/emu/machine/mc6852.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/c64/multiscreen.h \
	src/emu/bus/c64/super_explode.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/mess/machine/midioutport.h \
	src/emu/bus/c64/mikro_assembler.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/midi_siel.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/c64/midi_passport.h \
	src/emu/bus/c64/vw64.h \
	src/emu/bus/c64/dela_ep7x8.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \
	src/emu/sound/t6721a.h \
	src/emu/cpu/m6502/r65c02.h \
	src/emu/bus/c64/supercpu.h \

$(OBJ)/emu/bus/cbmiec/c1541.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/cbmiec/interpod.h \
	src/emu/bus/c64/4tba.h \
	src/lib/formats/flopimg.h \
	src/mess/machine/64h156.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/cbmiec/c64_nl10.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/ctronics.h \
	src/emu/machine/i8251.h \
	src/emu/bus/ieee488/hardbox.h \
	src/emu/bus/c64/vic1011.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/machine/wd_fdc.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/bus/cbmiec/c1581.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/mess/machine/isa_wd1002a_wx1.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/lib/formats/d81_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/bus/c64/4dxh.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/com8116.h \
	src/emu/bus/cbmiec/serialbox.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/6821pia.h \
	src/mess/machine/isa.h \
	src/mess/includes/corvushd.h \
	src/lib/formats/imageutl.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/cbmiec/cmdhd.h \
	src/emu/imagedev/harddriv.h \
	src/emu/imagedev/printer.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/bus/cbmiec/diag264_lb_iec.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/cbmiec/fd2000.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/bus/cbmiec/c1541.c \
	src/emu/imagedev/floppy.h \
	src/emu/bus/c64/geocable.h \
	src/emu/machine/mos6526.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/scsihd.h \
	src/emu/machine/6532riot.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/bus/cbmiec/c1541.h \
	src/lib/formats/d64_dsk.h \
	src/emu/machine/scsihle.h \
	src/emu/bus/c64/4cga.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/cbmiec/c1571.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/cbmiec/c1571.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/cbmiec/interpod.h \
	src/emu/bus/c64/4tba.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/cbmiec/c64_nl10.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/ctronics.h \
	src/emu/machine/i8251.h \
	src/emu/bus/ieee488/hardbox.h \
	src/emu/bus/c64/vic1011.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/cbmiec/c1581.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/mess/machine/isa_wd1002a_wx1.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/lib/formats/d81_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/bus/c64/4dxh.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/com8116.h \
	src/emu/bus/cbmiec/serialbox.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/scsidev.h \
	src/mess/includes/corvushd.h \
	src/lib/formats/imageutl.h \
	src/mess/machine/isa.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/cbmiec/cmdhd.h \
	src/emu/imagedev/harddriv.h \
	src/emu/imagedev/printer.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/bus/cbmiec/c1571.c \
	src/emu/bus/cbmiec/diag264_lb_iec.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/cbmiec/fd2000.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/machine/6522via.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/c64/geocable.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/mos6526.h \
	src/emu/machine/scsihd.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/cbmiec/c1541.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/d64_dsk.h \
	src/emu/machine/scsihle.h \
	src/emu/bus/c64/4cga.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/lib/formats/d88_dsk.h \
	src/emu/bus/cbmiec/c1571.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/cbmiec/c1581.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/cbmiec/interpod.h \
	src/emu/bus/c64/4tba.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/cbmiec/c64_nl10.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/ieee488/hardbox.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/cbmiec/c1581.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/mess/machine/isa_wd1002a_wx1.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/d81_dsk.h \
	src/emu/bus/c64/4dxh.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/com8116.h \
	src/emu/bus/cbmiec/serialbox.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/scsidev.h \
	src/mess/machine/isa.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/bus/cbmiec/c1581.c \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/cbmiec/cmdhd.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/harddriv.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/bus/cbmiec/diag264_lb_iec.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/cbmiec/fd2000.h \
	src/emu/machine/6522via.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/c64/geocable.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/mos6526.h \
	src/emu/machine/scsihd.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/cbmiec/c1541.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/4cga.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/cbmiec/c1571.h \
	src/lib/util/harddisk.h \
	src/lib/formats/d88_dsk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/cbmiec/c64_nl10.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/cbmiec/interpod.h \
	src/emu/bus/c64/4tba.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/cbmiec/c64_nl10.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/bus/cbmiec/c64_nl10.c \
	src/emu/machine/i8251.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/ieee488/hardbox.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/cbmiec/c1581.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/mess/machine/isa_wd1002a_wx1.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/d81_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/4dxh.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/com8116.h \
	src/emu/bus/cbmiec/serialbox.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/scsidev.h \
	src/mess/machine/isa.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/cbmiec/cmdhd.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/bus/cbmiec/diag264_lb_iec.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/cbmiec/fd2000.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/c64/geocable.h \
	src/emu/machine/mos6526.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/scsihd.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/cbmiec/c1541.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/4cga.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/cbmiec/c1571.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/cbmiec/cbmiec.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/cbmiec/interpod.h \
	src/emu/bus/c64/4tba.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/cbmiec/c64_nl10.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/ieee488/hardbox.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/cbmiec/c1581.h \
	src/emu/bus/cbmiec/cbmiec.c \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/mess/machine/isa_wd1002a_wx1.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/d81_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/4dxh.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/com8116.h \
	src/emu/bus/cbmiec/serialbox.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/scsidev.h \
	src/mess/machine/isa.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/cbmiec/cmdhd.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/bus/cbmiec/diag264_lb_iec.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/cbmiec/fd2000.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/c64/geocable.h \
	src/emu/machine/mos6526.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/scsihd.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/cbmiec/c1541.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/4cga.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/cbmiec/c1571.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/cbmiec/cmdhd.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/cbmiec/interpod.h \
	src/emu/bus/c64/4tba.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/cbmiec/c64_nl10.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/ieee488/hardbox.h \
	src/emu/bus/ieee488/softbox.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/cbmiec/c1581.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/mess/machine/isa_wd1002a_wx1.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/lib/formats/d81_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/4dxh.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/com8116.h \
	src/emu/bus/cbmiec/serialbox.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/scsidev.h \
	src/mess/machine/isa.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/bus/cbmiec/cmdhd.c \
	src/emu/machine/6850acia.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/cbmiec/cmdhd.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/bus/cbmiec/diag264_lb_iec.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/cbmiec/fd2000.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/c64/geocable.h \
	src/emu/machine/mos6526.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/machine/scsihd.h \
	src/emu/bus/cbmiec/c1541.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/4cga.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/scsihle.h \
	src/emu/bus/cbmiec/c1571.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/cbmiec/diag264_lb_iec.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/cbmiec/interpod.h \
	src/emu/bus/c64/4tba.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/cbmiec/c64_nl10.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/ieee488/hardbox.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/cbmiec/c1581.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/mess/machine/isa_wd1002a_wx1.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/d81_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/4dxh.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/com8116.h \
	src/emu/bus/cbmiec/serialbox.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/scsidev.h \
	src/mess/machine/isa.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/cbmiec/cmdhd.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/cbmiec/diag264_lb_iec.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/cbmiec/fd2000.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/c64/geocable.h \
	src/emu/machine/mos6526.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/scsihd.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/cbmiec/c1541.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/4cga.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/cbmiec/c1571.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/cbmiec/diag264_lb_iec.c \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/cbmiec/fd2000.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/cbmiec/interpod.h \
	src/emu/bus/c64/4tba.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/cbmiec/c64_nl10.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/ieee488/hardbox.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/cbmiec/c1581.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/mess/machine/isa_wd1002a_wx1.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/cpu/z80/z80.h \
	src/lib/formats/d81_dsk.h \
	src/emu/bus/c64/4dxh.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/com8116.h \
	src/emu/bus/cbmiec/serialbox.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/scsidev.h \
	src/mess/machine/isa.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/machine/upd765.h \
	src/emu/bus/cbmiec/fd2000.c \
	src/emu/bus/cbmiec/cmdhd.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/harddriv.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/bus/cbmiec/diag264_lb_iec.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/machine/6522via.h \
	src/lib/util/opresolv.h \
	src/emu/bus/cbmiec/fd2000.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/c64/geocable.h \
	src/emu/machine/mos6526.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/scsihd.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/cbmiec/c1541.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/4cga.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/cbmiec/c1571.h \
	src/lib/util/harddisk.h \
	src/lib/formats/d88_dsk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/cbmiec/interpod.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/cbmiec/interpod.h \
	src/emu/bus/c64/4tba.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/cbmiec/c64_nl10.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/bus/cbmiec/interpod.c \
	src/emu/machine/ctronics.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/ieee488/hardbox.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/cbmiec/c1581.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/mess/machine/isa_wd1002a_wx1.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/lib/formats/d81_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/4dxh.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/machine/t10spc.h \
	src/emu/bus/cbmiec/serialbox.h \
	src/emu/machine/com8116.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/scsidev.h \
	src/mess/machine/isa.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/cbmiec/cmdhd.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/bus/cbmiec/diag264_lb_iec.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/cbmiec/fd2000.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/c64/geocable.h \
	src/emu/machine/mos6526.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/scsihd.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/cbmiec/c1541.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/4cga.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/cbmiec/c1571.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/cbmiec/serialbox.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/emu/bus/cbmiec/interpod.h \
	src/emu/bus/c64/4tba.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/cbmiec/c64_nl10.h \
	src/emu/bus/c64/bn1541.h \
	src/emu/machine/ctronics.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/c64/vic1011.h \
	src/emu/bus/ieee488/hardbox.h \
	src/emu/bus/cbmiec/serialbox.c \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/cbmiec/c1581.h \
	src/lib/formats/wd177x_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/mess/machine/isa_wd1002a_wx1.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/emu/cpu/m6502/m65c02.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/lib/formats/d81_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/emu/bus/c64/4dxh.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/bus/cbmiec/cbmiec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/com8116.h \
	src/emu/bus/cbmiec/serialbox.h \
	src/emu/machine/6821pia.h \
	src/emu/machine/scsidev.h \
	src/mess/machine/isa.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/machine/6850acia.h \
	src/emu/machine/upd765.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/bus/cbmiec/cmdhd.h \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/bus/cbmiec/diag264_lb_iec.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/bus/c64/4ksa.h \
	src/emu/bus/c64/user.h \
	src/emu/bus/cbmiec/fd2000.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/c64/geocable.h \
	src/emu/machine/mos6526.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/scsihd.h \
	src/emu/machine/6532riot.h \
	src/emu/bus/cbmiec/c1541.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/emu/bus/c64/4cga.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/cbmiec/c1571.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/ieee488/c2031.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/lib/formats/flopimg.h \
	src/mess/machine/64h156.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/ieee488/hardbox.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/machine/wd_fdc.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/ieee488/c2031.c \
	src/lib/util/chd.h \
	src/emu/bus/ieee488/shark.h \
	src/lib/formats/g64_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/mess/includes/corvushd.h \
	src/lib/formats/imageutl.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/ieee488/c2040.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/ieee488/hardbox.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/machine/wd_fdc.h \
	src/emu/imagedev/flopdrv.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/emu/bus/ieee488/shark.h \
	src/lib/formats/g64_dsk.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/mess/includes/corvushd.h \
	src/lib/formats/imageutl.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/mos6530.h \
	src/emu/bus/ieee488/c2040.c \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/ieee488/c8280.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/emu/bus/ieee488/c8280.c \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/ieee488/hardbox.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/imagedev/harddriv.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/machine/6522via.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/serial.h \
	src/emu/bus/ieee488/c8280.h \
	src/emu/machine/6532riot.h \
	src/lib/util/hashing.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/d64_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/util/harddisk.h \
	src/lib/formats/d88_dsk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/ieee488/d9060.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/ieee488/hardbox.h \
	src/emu/bus/ieee488/softbox.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/ieee488/d9060hd.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/machine/t10sbc.h \
	src/emu/machine/scsibus.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/com8116.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/lib/util/md5.h \
	src/emu/machine/scsicb.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/ieee488/d9060.c \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/emu/machine/scsihd.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/scsihle.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/ieee488/d9060hd.o : \
	src/lib/util/palette.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/d9060hd.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/ieee488/d9060hd.c \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/scsihd.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/ieee488/hardbox.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/emu/bus/ieee488/hardbox.c \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/ieee488/hardbox.h \
	src/emu/bus/ieee488/softbox.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/ieee488/ieee488.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/ieee488/hardbox.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/emu/bus/ieee488/ieee488.c \
	src/lib/formats/d64_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/ieee488/shark.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/ieee488/hardbox.h \
	src/emu/bus/ieee488/softbox.h \
	src/lib/util/sha1.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/i8255.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/bus/ieee488/shark.c \
	src/emu/cpu/m6502/m6504.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/ieee488/softbox.o : \
	src/emu/bus/ieee488/ieee488.h \
	src/mess/machine/64h156.h \
	src/lib/formats/flopimg.h \
	src/lib/util/palette.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/ieee488/softbox.c \
	src/emu/bus/ieee488/c2031.h \
	src/emu/machine/i8251.h \
	src/emu/bus/ieee488/hardbox.h \
	src/lib/util/sha1.h \
	src/emu/bus/ieee488/softbox.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/chd.h \
	src/lib/formats/g64_dsk.h \
	src/emu/bus/ieee488/shark.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/i8255.h \
	src/emu/machine/com8116.h \
	src/emu/machine/scsidev.h \
	src/lib/formats/imageutl.h \
	src/mess/includes/corvushd.h \
	src/emu/bus/ieee488/c2040.h \
	src/emu/machine/scsicb.h \
	src/lib/util/md5.h \
	src/emu/cpu/m6502/m6504.h \
	src/emu/imagedev/harddriv.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/cpu/i8085/i8085.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/emu/machine/mos6530.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/cpu/m6502/m6502.h \
	src/lib/util/opresolv.h \
	src/emu/machine/6522via.h \
	src/emu/imagedev/floppy.h \
	src/emu/bus/ieee488/c8280.h \
	src/mess/machine/serial.h \
	src/emu/machine/6532riot.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/util/hashing.h \
	src/lib/formats/d64_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/harddisk.h \
	src/emu/bus/ieee488/d9060.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/isbx/compis_fdc.o : \
	src/lib/formats/cpis_dsk.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/upd765_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/isbx/isbx.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/emu/bus/isbx/compis_fdc.c \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/bus/isbx/compis_fdc.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/bus/isbx/isbx.o : \
	src/lib/formats/cpis_dsk.h \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/upd765_dsk.h \
	src/emu/bus/isbx/isbx.c \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/isbx/isbx.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/bus/isbx/compis_fdc.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/bus/s100/dj2db.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/s100/nsmdsad.h \
	src/emu/bus/s100/wunderbus.h \
	src/emu/bus/s100/s100.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/s100/nsmdsa.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/com8116.h \
	src/emu/bus/s100/dj2db.h \
	src/emu/machine/ins8250.h \
	src/emu/bus/s100/dj2db.c \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/s100/djdma.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/serial.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/bus/s100/mm65k16s.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/upd1990a.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/bus/s100/djdma.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/s100/nsmdsad.h \
	src/emu/bus/s100/wunderbus.h \
	src/emu/bus/s100/s100.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/s100/nsmdsa.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/com8116.h \
	src/emu/bus/s100/dj2db.h \
	src/emu/machine/ins8250.h \
	src/emu/bus/s100/djdma.c \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/s100/djdma.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/serial.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/bus/s100/mm65k16s.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/machine/upd1990a.h \

$(OBJ)/emu/bus/s100/mm65k16s.o : \
	src/emu/bus/s100/mm65k16s.c \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/s100/nsmdsad.h \
	src/emu/bus/s100/wunderbus.h \
	src/emu/bus/s100/s100.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/s100/nsmdsa.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/com8116.h \
	src/emu/bus/s100/dj2db.h \
	src/emu/machine/ins8250.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/s100/djdma.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/serial.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/emu/bus/s100/mm65k16s.h \
	src/lib/formats/d88_dsk.h \
	src/emu/machine/upd1990a.h \

$(OBJ)/emu/bus/s100/nsmdsa.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/s100/nsmdsa.c \
	src/emu/bus/s100/nsmdsad.h \
	src/emu/bus/s100/wunderbus.h \
	src/emu/bus/s100/s100.h \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/emu/bus/s100/nsmdsa.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/com8116.h \
	src/emu/bus/s100/dj2db.h \
	src/emu/machine/ins8250.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/s100/djdma.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/serial.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/bus/s100/mm65k16s.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/upd1990a.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/bus/s100/nsmdsad.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/s100/nsmdsad.c \
	src/emu/bus/s100/nsmdsad.h \
	src/emu/bus/s100/wunderbus.h \
	src/emu/bus/s100/s100.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/s100/nsmdsa.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/com8116.h \
	src/emu/bus/s100/dj2db.h \
	src/emu/machine/ins8250.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/s100/djdma.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/serial.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/bus/s100/mm65k16s.h \
	src/lib/formats/ioprocs.h \
	src/emu/machine/upd1990a.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/bus/s100/s100.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/s100/s100.c \
	src/emu/bus/s100/nsmdsad.h \
	src/emu/bus/s100/wunderbus.h \
	src/emu/bus/s100/s100.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/s100/nsmdsa.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/com8116.h \
	src/emu/bus/s100/dj2db.h \
	src/emu/machine/ins8250.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/s100/djdma.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/serial.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/bus/s100/mm65k16s.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/machine/upd1990a.h \

$(OBJ)/emu/bus/s100/wunderbus.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/bus/s100/nsmdsad.h \
	src/emu/bus/s100/wunderbus.h \
	src/emu/bus/s100/s100.h \
	src/emu/machine/wd_fdc.h \
	src/emu/bus/s100/nsmdsa.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/pic8259.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/com8116.h \
	src/emu/bus/s100/dj2db.h \
	src/emu/bus/s100/wunderbus.c \
	src/emu/machine/ins8250.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/bus/s100/djdma.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/mess/machine/serial.h \
	src/lib/formats/ipf_dsk.h \
	src/emu/bus/s100/mm65k16s.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/emu/machine/upd1990a.h \

$(OBJ)/emu/bus/wangpc/emb.o : \
	src/emu/bus/wangpc/lic.h \
	src/lib/util/palette.h \
	src/emu/bus/wangpc/wdc.h \
	src/emu/bus/wangpc/mvc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/bus/wangpc/rtc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/bus/wangpc/wangpc.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/z80ctc.h \
	src/emu/bus/wangpc/mcc.h \
	src/lib/util/md5.h \
	src/emu/video/mc6845.h \
	src/emu/video/upd7220.h \
	src/emu/bus/wangpc/tig.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/bus/wangpc/emb.c \
	src/emu/machine/z80dart.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/lib/util/harddisk.h \
	src/emu/bus/wangpc/lvc.h \
	src/lib/util/astring.h \
	src/emu/bus/wangpc/emb.h \

$(OBJ)/emu/bus/wangpc/lic.o : \
	src/emu/bus/wangpc/lic.h \
	src/lib/util/palette.h \
	src/emu/bus/wangpc/wdc.h \
	src/emu/bus/wangpc/lic.c \
	src/emu/bus/wangpc/mvc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/bus/wangpc/rtc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/bus/wangpc/wangpc.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/z80ctc.h \
	src/emu/bus/wangpc/mcc.h \
	src/lib/util/md5.h \
	src/emu/video/mc6845.h \
	src/emu/video/upd7220.h \
	src/emu/bus/wangpc/tig.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/z80dart.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/lib/util/harddisk.h \
	src/emu/bus/wangpc/lvc.h \
	src/emu/bus/wangpc/emb.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/wangpc/lvc.o : \
	src/emu/bus/wangpc/lic.h \
	src/lib/util/palette.h \
	src/emu/bus/wangpc/wdc.h \
	src/emu/bus/wangpc/mvc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/bus/wangpc/rtc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/bus/wangpc/wangpc.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/z80ctc.h \
	src/emu/bus/wangpc/mcc.h \
	src/lib/util/md5.h \
	src/emu/video/mc6845.h \
	src/emu/video/upd7220.h \
	src/emu/bus/wangpc/tig.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/z80dart.h \
	src/emu/bus/wangpc/lvc.c \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/lib/util/harddisk.h \
	src/emu/bus/wangpc/lvc.h \
	src/emu/bus/wangpc/emb.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/wangpc/mcc.o : \
	src/emu/bus/wangpc/lic.h \
	src/lib/util/palette.h \
	src/emu/bus/wangpc/wdc.h \
	src/emu/bus/wangpc/mvc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/bus/wangpc/rtc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/bus/wangpc/wangpc.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/wangpc/mcc.c \
	src/emu/machine/z80ctc.h \
	src/lib/util/md5.h \
	src/emu/bus/wangpc/mcc.h \
	src/emu/video/mc6845.h \
	src/emu/video/upd7220.h \
	src/emu/bus/wangpc/tig.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/z80dart.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/lib/util/harddisk.h \
	src/emu/bus/wangpc/lvc.h \
	src/emu/bus/wangpc/emb.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/wangpc/mvc.o : \
	src/emu/bus/wangpc/mvc.c \
	src/emu/bus/wangpc/lic.h \
	src/lib/util/palette.h \
	src/emu/bus/wangpc/wdc.h \
	src/emu/bus/wangpc/mvc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/bus/wangpc/rtc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/bus/wangpc/wangpc.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/z80ctc.h \
	src/emu/bus/wangpc/mcc.h \
	src/lib/util/md5.h \
	src/emu/video/mc6845.h \
	src/emu/video/upd7220.h \
	src/emu/bus/wangpc/tig.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/z80dart.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/lib/util/harddisk.h \
	src/emu/bus/wangpc/lvc.h \
	src/emu/bus/wangpc/emb.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/wangpc/rtc.o : \
	src/emu/bus/wangpc/lic.h \
	src/lib/util/palette.h \
	src/emu/bus/wangpc/wdc.h \
	src/emu/bus/wangpc/mvc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/bus/wangpc/rtc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/bus/wangpc/wangpc.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/wangpc/rtc.c \
	src/emu/machine/z80ctc.h \
	src/emu/bus/wangpc/mcc.h \
	src/lib/util/md5.h \
	src/emu/video/mc6845.h \
	src/emu/video/upd7220.h \
	src/emu/bus/wangpc/tig.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/z80dart.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/lib/util/harddisk.h \
	src/emu/bus/wangpc/lvc.h \
	src/emu/bus/wangpc/emb.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/wangpc/tig.o : \
	src/emu/bus/wangpc/lic.h \
	src/lib/util/palette.h \
	src/emu/bus/wangpc/wdc.h \
	src/emu/bus/wangpc/mvc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/bus/wangpc/rtc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/bus/wangpc/wangpc.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/bus/wangpc/tig.c \
	src/emu/machine/z80ctc.h \
	src/emu/bus/wangpc/mcc.h \
	src/lib/util/md5.h \
	src/emu/video/mc6845.h \
	src/emu/video/upd7220.h \
	src/emu/imagedev/harddriv.h \
	src/emu/bus/wangpc/tig.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/z80dart.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/lib/util/harddisk.h \
	src/emu/bus/wangpc/lvc.h \
	src/emu/bus/wangpc/emb.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/wangpc/wangpc.o : \
	src/emu/bus/wangpc/lic.h \
	src/lib/util/palette.h \
	src/emu/bus/wangpc/wdc.h \
	src/emu/bus/wangpc/mvc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/bus/wangpc/rtc.h \
	src/emu/bus/wangpc/wangpc.c \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/bus/wangpc/wangpc.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/z80ctc.h \
	src/emu/bus/wangpc/mcc.h \
	src/lib/util/md5.h \
	src/emu/video/mc6845.h \
	src/emu/video/upd7220.h \
	src/emu/bus/wangpc/tig.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/z80dart.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/lib/util/harddisk.h \
	src/emu/bus/wangpc/lvc.h \
	src/emu/bus/wangpc/emb.h \
	src/lib/util/astring.h \

$(OBJ)/emu/bus/wangpc/wdc.o : \
	src/emu/bus/wangpc/lic.h \
	src/emu/bus/wangpc/wdc.h \
	src/lib/util/palette.h \
	src/emu/bus/wangpc/mvc.h \
	src/emu/bus/wangpc/wdc.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/bus/wangpc/rtc.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/bus/wangpc/wangpc.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/emu/cpu/z80/z80.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/z80ctc.h \
	src/emu/bus/wangpc/mcc.h \
	src/lib/util/md5.h \
	src/emu/video/mc6845.h \
	src/emu/video/upd7220.h \
	src/emu/bus/wangpc/tig.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/z80dart.h \
	src/emu/machine/scsihd.h \
	src/lib/util/hashing.h \
	src/emu/machine/am9517a.h \
	src/emu/machine/scsihle.h \
	src/lib/util/harddisk.h \
	src/emu/bus/wangpc/lvc.h \
	src/emu/bus/wangpc/emb.h \
	src/lib/util/astring.h \

$(OBJ)/emu/cpu/adsp2100/2100dasm.o : \
	src/emu/cpu/adsp2100/2100dasm.c \
	src/emu/cpu/adsp2100/adsp2100.h \

$(OBJ)/emu/cpu/adsp2100/2100ops.o : \
	src/emu/cpu/adsp2100/2100ops.c \

$(OBJ)/emu/cpu/adsp2100/adsp2100.o : \
	src/emu/cpu/adsp2100/adsp2100.c \
	src/emu/cpu/adsp2100/2100ops.c \
	src/emu/cpu/adsp2100/adsp2100.h \

$(OBJ)/emu/cpu/alph8201/8201dasm.o : \
	src/emu/cpu/alph8201/8201dasm.c \

$(OBJ)/emu/cpu/alph8201/alph8201.o : \
	src/emu/cpu/alph8201/alph8201.h \
	src/emu/cpu/alph8201/alph8201.c \

$(OBJ)/emu/cpu/am29000/am29000.o : \
	src/emu/cpu/am29000/am29000.c \
	src/emu/cpu/am29000/am29000.h \
	src/emu/cpu/am29000/am29ops.h \

$(OBJ)/emu/cpu/am29000/am29dasm.o : \
	src/emu/cpu/am29000/am29000.h \
	src/emu/cpu/am29000/am29dasm.c \

$(OBJ)/emu/cpu/apexc/apexc.o : \
	src/emu/cpu/apexc/apexc.c \
	src/emu/cpu/apexc/apexc.h \

$(OBJ)/emu/cpu/apexc/apexcdsm.o : \
	src/emu/cpu/apexc/apexcdsm.c \
	src/emu/cpu/apexc/apexc.h \

$(OBJ)/emu/cpu/arm/arm.o : \
	src/emu/cpu/arm/arm.c \
	src/emu/cpu/arm/arm.h \

$(OBJ)/emu/cpu/arm/armdasm.o : \
	src/emu/cpu/arm/armdasm.c \
	src/emu/cpu/arm/arm.h \

$(OBJ)/emu/cpu/arm7/arm7.o : \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/arm7/arm7tdrc.c \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7drc.c \
	src/emu/cpu/arm7/arm7help.h \
	src/emu/cpu/arm7/arm7core.h \
	src/emu/cpu/arm7/arm7core.c \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \
	src/emu/cpu/arm7/arm7.c \

$(OBJ)/emu/cpu/arm7/arm7core.o : \
	src/emu/cpu/arm7/arm7core.c \

$(OBJ)/emu/cpu/arm7/arm7dasm.o : \
	src/emu/cpu/arm7/arm7dasm.c \
	src/emu/cpu/arm7/arm7core.h \

$(OBJ)/emu/cpu/arm7/arm7drc.o : \
	src/emu/cpu/arm7/arm7tdrc.c \
	src/emu/cpu/arm7/arm7drc.c \
	src/emu/cpu/arm7/arm7help.h \
	src/emu/cpu/arm7/arm7core.h \

$(OBJ)/emu/cpu/arm7/arm7ops.o : \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7help.h \
	src/emu/cpu/arm7/arm7core.h \
	src/emu/cpu/arm7/arm7ops.c \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/emu/cpu/arm7/arm7tdrc.o : \
	src/emu/cpu/arm7/arm7tdrc.c \
	src/emu/cpu/arm7/arm7help.h \
	src/emu/cpu/arm7/arm7core.h \

$(OBJ)/emu/cpu/arm7/arm7thmb.o : \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/arm7/arm7thmb.c \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7help.h \
	src/emu/cpu/arm7/arm7core.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/emu/cpu/asap/asap.o : \
	src/emu/cpu/asap/asap.c \
	src/emu/cpu/asap/asap.h \

$(OBJ)/emu/cpu/asap/asapdasm.o : \
	src/emu/cpu/asap/asap.h \
	src/emu/cpu/asap/asapdasm.c \

$(OBJ)/emu/cpu/avr8/avr8.o : \
	src/emu/cpu/avr8/avr8.c \
	src/emu/cpu/avr8/avr8.h \

$(OBJ)/emu/cpu/avr8/avr8dasm.o : \
	src/emu/cpu/avr8/avr8.h \
	src/emu/cpu/avr8/avr8dasm.c \

$(OBJ)/emu/cpu/ccpu/ccpu.o : \
	src/emu/cpu/ccpu/ccpu.h \
	src/emu/cpu/ccpu/ccpu.c \

$(OBJ)/emu/cpu/ccpu/ccpudasm.o : \
	src/emu/cpu/ccpu/ccpu.h \
	src/emu/cpu/ccpu/ccpudasm.c \

$(OBJ)/emu/cpu/cop400/cop400.o : \
	src/emu/cpu/cop400/cop400.h \
	src/emu/cpu/cop400/cop400op.c \
	src/emu/cpu/cop400/cop400.c \

$(OBJ)/emu/cpu/cop400/cop400op.o : \
	src/emu/cpu/cop400/cop400op.c \

$(OBJ)/emu/cpu/cop400/cop410ds.o : \
	src/emu/cpu/cop400/cop410ds.c \

$(OBJ)/emu/cpu/cop400/cop420ds.o : \
	src/emu/cpu/cop400/cop420ds.c \

$(OBJ)/emu/cpu/cop400/cop440ds.o : \
	src/emu/cpu/cop400/cop440ds.c \

$(OBJ)/emu/cpu/cosmac/cosdasm.o : \
	src/emu/cpu/cosmac/cosdasm.c \

$(OBJ)/emu/cpu/cosmac/cosmac.o : \
	src/lib/util/coreutil.h \
	src/emu/cpu/cosmac/cosmac.c \
	src/emu/cpu/cosmac/cosmac.h \

$(OBJ)/emu/cpu/cp1610/1610dasm.o : \
	src/emu/cpu/cp1610/cp1610.h \
	src/emu/cpu/cp1610/1610dasm.c \

$(OBJ)/emu/cpu/cp1610/cp1610.o : \
	src/emu/cpu/cp1610/cp1610.h \
	src/emu/cpu/cp1610/cp1610.c \

$(OBJ)/emu/cpu/cubeqcpu/cubedasm.o : \
	src/emu/cpu/cubeqcpu/cubedasm.c \
	src/emu/cpu/cubeqcpu/cubeqcpu.h \

$(OBJ)/emu/cpu/cubeqcpu/cubeqcpu.o : \
	src/emu/cpu/cubeqcpu/cubeqcpu.c \
	src/emu/cpu/cubeqcpu/cubeqcpu.h \

$(OBJ)/emu/cpu/dsp16/dsp16.o : \
	src/emu/cpu/dsp16/dsp16ops.c \
	src/emu/cpu/dsp16/dsp16.c \
	src/emu/cpu/dsp16/dsp16.h \

$(OBJ)/emu/cpu/dsp16/dsp16dis.o : \
	src/emu/cpu/dsp16/dsp16dis.c \
	src/emu/cpu/dsp16/dsp16.h \

$(OBJ)/emu/cpu/dsp16/dsp16ops.o : \
	src/emu/cpu/dsp16/dsp16ops.c \
	src/emu/cpu/dsp16/dsp16.h \

$(OBJ)/emu/cpu/dsp32/dsp32.o : \
	src/emu/cpu/dsp32/dsp32ops.c \
	src/emu/cpu/dsp32/dsp32.c \
	src/emu/cpu/dsp32/dsp32.h \

$(OBJ)/emu/cpu/dsp32/dsp32dis.o : \
	src/emu/cpu/dsp32/dsp32.h \
	src/emu/cpu/dsp32/dsp32dis.c \

$(OBJ)/emu/cpu/dsp32/dsp32ops.o : \
	src/emu/cpu/dsp32/dsp32ops.c \

$(OBJ)/emu/cpu/dsp56k/dsp56dsm.o : \
	src/emu/cpu/dsp56k/dsp56def.h \
	src/emu/cpu/dsp56k/pmove.h \
	src/emu/cpu/dsp56k/opcode.h \
	src/emu/cpu/dsp56k/inst.h \
	src/emu/cpu/dsp56k/dsp56dsm.c \
	src/emu/cpu/dsp56k/tables.h \
	src/emu/cpu/dsp56k/dsp56pcu.h \
	src/emu/cpu/dsp56k/dsp56k.h \

$(OBJ)/emu/cpu/dsp56k/dsp56k.o : \
	src/emu/cpu/dsp56k/dsp56def.h \
	src/emu/cpu/dsp56k/pmove.h \
	src/emu/cpu/dsp56k/opcode.h \
	src/emu/cpu/dsp56k/inst.h \
	src/emu/cpu/dsp56k/dsp56mem.h \
	src/emu/cpu/dsp56k/dsp56k.c \
	src/emu/cpu/dsp56k/tables.h \
	src/emu/cpu/dsp56k/dsp56pcu.h \
	src/emu/cpu/dsp56k/dsp56ops.c \
	src/emu/cpu/dsp56k/dsp56k.h \

$(OBJ)/emu/cpu/dsp56k/dsp56mem.o : \
	src/emu/cpu/dsp56k/dsp56mem.h \
	src/emu/cpu/dsp56k/dsp56mem.c \
	src/emu/cpu/dsp56k/dsp56pcu.h \
	src/emu/cpu/dsp56k/dsp56k.h \

$(OBJ)/emu/cpu/dsp56k/dsp56ops.o : \
	src/emu/cpu/dsp56k/dsp56ops.c \

$(OBJ)/emu/cpu/dsp56k/dsp56pcu.o : \
	src/emu/cpu/dsp56k/dsp56mem.h \
	src/emu/cpu/dsp56k/dsp56pcu.h \
	src/emu/cpu/dsp56k/dsp56k.h \
	src/emu/cpu/dsp56k/dsp56pcu.c \

$(OBJ)/emu/cpu/dsp56k/inst.o : \
	src/emu/cpu/dsp56k/dsp56def.h \
	src/emu/cpu/dsp56k/pmove.h \
	src/emu/cpu/dsp56k/opcode.h \
	src/emu/cpu/dsp56k/inst.h \
	src/emu/cpu/dsp56k/inst.c \
	src/emu/cpu/dsp56k/tables.h \
	src/emu/cpu/dsp56k/dsp56pcu.h \
	src/emu/cpu/dsp56k/dsp56k.h \

$(OBJ)/emu/cpu/dsp56k/opcode.o : \
	src/emu/cpu/dsp56k/dsp56def.h \
	src/emu/cpu/dsp56k/pmove.h \
	src/emu/cpu/dsp56k/opcode.h \
	src/emu/cpu/dsp56k/opcode.c \
	src/emu/cpu/dsp56k/inst.h \
	src/emu/cpu/dsp56k/tables.h \
	src/emu/cpu/dsp56k/dsp56pcu.h \
	src/emu/cpu/dsp56k/dsp56k.h \

$(OBJ)/emu/cpu/dsp56k/pmove.o : \
	src/emu/cpu/dsp56k/dsp56def.h \
	src/emu/cpu/dsp56k/opcode.h \
	src/emu/cpu/dsp56k/pmove.h \
	src/emu/cpu/dsp56k/pmove.c \
	src/emu/cpu/dsp56k/inst.h \
	src/emu/cpu/dsp56k/tables.h \
	src/emu/cpu/dsp56k/dsp56pcu.h \
	src/emu/cpu/dsp56k/dsp56k.h \

$(OBJ)/emu/cpu/dsp56k/tables.o : \
	src/emu/cpu/dsp56k/dsp56def.h \
	src/emu/cpu/dsp56k/tables.c \
	src/emu/cpu/dsp56k/tables.h \
	src/emu/cpu/dsp56k/dsp56k.h \

$(OBJ)/emu/cpu/e132xs/32xsdasm.o : \
	src/emu/cpu/e132xs/e132xs.h \
	src/emu/cpu/e132xs/32xsdasm.c \

$(OBJ)/emu/cpu/e132xs/e132xs.o : \
	src/emu/cpu/e132xs/e132xs.c \
	src/emu/cpu/e132xs/e132xs.h \
	src/emu/cpu/e132xs/e132xsop.c \

$(OBJ)/emu/cpu/e132xs/e132xsop.o : \
	src/emu/cpu/e132xs/e132xsop.c \

$(OBJ)/emu/cpu/es5510/es5510.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/es5510/es5510.c \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/cpu/es5510/es5510.h \

$(OBJ)/emu/cpu/esrip/esrip.o : \
	src/emu/cpu/esrip/esrip.c \
	src/emu/cpu/esrip/esrip.h \

$(OBJ)/emu/cpu/esrip/esripdsm.o : \
	src/emu/cpu/esrip/esripdsm.c \

$(OBJ)/emu/cpu/f8/f8.o : \
	src/emu/cpu/f8/f8.h \
	src/emu/cpu/f8/f8.c \

$(OBJ)/emu/cpu/f8/f8dasm.o : \
	src/emu/cpu/f8/f8.h \
	src/emu/cpu/f8/f8dasm.c \

$(OBJ)/emu/cpu/g65816/g65816.o : \
	src/emu/cpu/g65816/g65816.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/cpu/g65816/g65816.c \
	src/emu/cpu/g65816/g65816ds.h \

$(OBJ)/emu/cpu/g65816/g65816ds.o : \
	src/emu/cpu/g65816/g65816ds.c \
	src/emu/cpu/g65816/g65816ds.h \

$(OBJ)/emu/cpu/g65816/g65816o0.o : \
	src/emu/cpu/g65816/g65816.h \
	src/emu/cpu/g65816/g65816o0.c \
	src/emu/cpu/g65816/g65816op.h \
	src/emu/cpu/g65816/g65816cm.h \

$(OBJ)/emu/cpu/g65816/g65816o1.o : \
	src/emu/cpu/g65816/g65816.h \
	src/emu/cpu/g65816/g65816op.h \
	src/emu/cpu/g65816/g65816o1.c \
	src/emu/cpu/g65816/g65816cm.h \

$(OBJ)/emu/cpu/g65816/g65816o2.o : \
	src/emu/cpu/g65816/g65816.h \
	src/emu/cpu/g65816/g65816o2.c \
	src/emu/cpu/g65816/g65816op.h \
	src/emu/cpu/g65816/g65816cm.h \

$(OBJ)/emu/cpu/g65816/g65816o3.o : \
	src/emu/cpu/g65816/g65816.h \
	src/emu/cpu/g65816/g65816op.h \
	src/emu/cpu/g65816/g65816cm.h \
	src/emu/cpu/g65816/g65816o3.c \

$(OBJ)/emu/cpu/g65816/g65816o4.o : \
	src/emu/cpu/g65816/g65816.h \
	src/emu/cpu/g65816/g65816o4.c \
	src/emu/cpu/g65816/g65816op.h \
	src/emu/cpu/g65816/g65816cm.h \

$(OBJ)/emu/cpu/h6280/6280dasm.o : \
	src/emu/cpu/h6280/6280dasm.c \

$(OBJ)/emu/cpu/h6280/h6280.o : \
	src/emu/cpu/h6280/h6280.h \
	src/emu/cpu/h6280/tblh6280.inc \
	src/emu/cpu/h6280/h6280ops.h \
	src/emu/cpu/h6280/h6280.c \

$(OBJ)/emu/cpu/h83002/h8_16.o : \
	src/emu/cpu/h83002/h8.h \
	src/emu/cpu/h83002/h8ops.h \
	src/emu/cpu/h83002/h8priv.h \
	src/emu/cpu/h83002/h8_16.c \

$(OBJ)/emu/cpu/h83002/h8_8.o : \
	src/emu/cpu/h83002/h8.h \
	src/emu/cpu/h83002/h8ops.h \
	src/emu/cpu/h83002/h8priv.h \
	src/emu/cpu/h83002/h8_8.c \

$(OBJ)/emu/cpu/h83002/h8disasm.o : \
	src/emu/cpu/h83002/h8disasm.c \
	src/emu/cpu/h83002/h8.h \

$(OBJ)/emu/cpu/h83002/h8periph.o : \
	src/emu/cpu/h83002/h8.h \
	src/emu/cpu/h83002/h8periph.c \
	src/emu/cpu/h83002/h8priv.h \

$(OBJ)/emu/cpu/h83002/h8speriph.o : \
	src/emu/cpu/h83002/h8.h \
	src/emu/cpu/h83002/h8priv.h \
	src/emu/cpu/h83002/h8speriph.c \

$(OBJ)/emu/cpu/hcd62121/hcd62121.o : \
	src/emu/cpu/hcd62121/hcd62121_ops.h \
	src/emu/cpu/hcd62121/hcd62121.c \
	src/emu/cpu/hcd62121/hcd62121.h \

$(OBJ)/emu/cpu/hcd62121/hcd62121d.o : \
	src/emu/cpu/hcd62121/hcd62121.h \
	src/emu/cpu/hcd62121/hcd62121d.c \

$(OBJ)/emu/cpu/hd61700/hd61700.o : \
	src/emu/cpu/hd61700/hd61700.h \
	src/emu/cpu/hd61700/hd61700.c \

$(OBJ)/emu/cpu/hd61700/hd61700d.o : \
	src/emu/cpu/hd61700/hd61700d.c \
	src/emu/cpu/hd61700/hd61700.h \

$(OBJ)/emu/cpu/i386/i386.o : \
	src/emu/cpu/i386/i386ops.h \
	src/emu/cpu/i386/i386op16.c \
	src/emu/cpu/i386/x87ops.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/i386/i386ops.c \
	src/emu/cpu/i386/cycles.h \
	src/emu/cpu/i386/i386.c \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/i386/i386op32.c \
	src/lib/util/simple_set.h \
	src/lib/softfloat/milieu.h \
	src/emu/cpu/i386/i386.h \
	src/emu/cpu/i386/pentops.c \
	src/emu/cpu/i386/i486ops.c \
	src/lib/softfloat/softfloat.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \
	src/emu/cpu/vtlb.h \
	src/emu/cpu/i386/i386priv.h \

$(OBJ)/emu/cpu/i386/i386dasm.o : \
	src/emu/cpu/i386/i386dasm.c \

$(OBJ)/emu/cpu/i386/i386op16.o : \
	src/emu/cpu/i386/i386op16.c \

$(OBJ)/emu/cpu/i386/i386op32.o : \
	src/emu/cpu/i386/i386op32.c \

$(OBJ)/emu/cpu/i386/i386ops.o : \
	src/emu/cpu/i386/i386ops.c \

$(OBJ)/emu/cpu/i386/i486ops.o : \
	src/emu/cpu/i386/i486ops.c \

$(OBJ)/emu/cpu/i386/pentops.o : \
	src/emu/cpu/i386/pentops.c \

$(OBJ)/emu/cpu/i386/x87ops.o : \
	src/emu/cpu/i386/x87ops.c \

$(OBJ)/emu/cpu/i4004/4004dasm.o : \
	src/emu/cpu/i4004/4004dasm.c \

$(OBJ)/emu/cpu/i4004/i4004.o : \
	src/emu/cpu/i4004/i4004.c \
	src/emu/cpu/i4004/i4004.h \

$(OBJ)/emu/cpu/i8008/8008dasm.o : \
	src/emu/cpu/i8008/8008dasm.c \

$(OBJ)/emu/cpu/i8008/i8008.o : \
	src/emu/cpu/i8008/i8008.c \
	src/emu/cpu/i8008/i8008.h \

$(OBJ)/emu/cpu/i8085/8085dasm.o : \
	src/emu/cpu/i8085/8085dasm.c \

$(OBJ)/emu/cpu/i8085/i8085.o : \
	src/emu/cpu/i8085/i8085.c \
	src/emu/cpu/i8085/i8085cpu.h \
	src/emu/cpu/i8085/i8085.h \

$(OBJ)/emu/cpu/i8089/i8089.o : \
	src/emu/cpu/i8089/i8089_channel.h \
	src/emu/cpu/i8089/i8089.c \
	src/emu/cpu/i8089/i8089.h \

$(OBJ)/emu/cpu/i8089/i8089_channel.o : \
	src/emu/cpu/i8089/i8089_channel.h \
	src/emu/cpu/i8089/i8089_channel.c \
	src/emu/cpu/i8089/i8089.h \

$(OBJ)/emu/cpu/i8089/i8089_dasm.o : \
	src/emu/cpu/i8089/i8089_dasm.c \

$(OBJ)/emu/cpu/i8089/i8089_ops.o : \
	src/emu/cpu/i8089/i8089_channel.h \
	src/emu/cpu/i8089/i8089_ops.c \
	src/emu/cpu/i8089/i8089.h \

$(OBJ)/emu/cpu/i86/i186.o : \
	src/emu/cpu/i86/i86inline.h \
	src/emu/cpu/i86/i86.h \
	src/emu/cpu/i86/i186.c \
	src/emu/cpu/i86/i186.h \

$(OBJ)/emu/cpu/i86/i286.o : \
	src/emu/cpu/i86/i286.c \
	src/emu/cpu/i86/i286.h \
	src/emu/cpu/i86/i86inline.h \
	src/emu/cpu/i86/i86.h \

$(OBJ)/emu/cpu/i86/i86.o : \
	src/emu/cpu/i86/i86inline.h \
	src/emu/cpu/i86/i86.c \
	src/emu/cpu/i86/i86.h \

$(OBJ)/emu/cpu/i860/i860.o : \
	src/emu/cpu/i860/i860.c \
	src/emu/cpu/i860/i860.h \
	src/emu/cpu/i860/i860dec.c \

$(OBJ)/emu/cpu/i860/i860dasm.o : \
	src/emu/cpu/i860/i860.h \
	src/emu/cpu/i860/i860dasm.c \

$(OBJ)/emu/cpu/i860/i860dec.o : \
	src/emu/cpu/i860/i860.h \
	src/emu/cpu/i860/i860dec.c \

$(OBJ)/emu/cpu/i860/i860dis.o : \
	src/emu/cpu/i860/i860.h \
	src/emu/cpu/i860/i860dis.c \

$(OBJ)/emu/cpu/i960/i960.o : \
	src/emu/cpu/i960/i960.c \
	src/emu/cpu/i960/i960.h \

$(OBJ)/emu/cpu/i960/i960dis.o : \
	src/emu/cpu/i960/i960dis.h \
	src/emu/cpu/i960/i960dis.c \
	src/emu/cpu/i960/i960.h \

$(OBJ)/emu/cpu/ie15/ie15.o : \
	src/emu/cpu/ie15/ie15.c \
	src/emu/cpu/ie15/ie15.h \

$(OBJ)/emu/cpu/ie15/ie15dasm.o : \
	src/emu/cpu/ie15/ie15dasm.c \

$(OBJ)/emu/cpu/jaguar/jagdasm.o : \
	src/emu/cpu/jaguar/jagdasm.c \
	src/emu/cpu/jaguar/jaguar.h \

$(OBJ)/emu/cpu/jaguar/jaguar.o : \
	src/emu/cpu/jaguar/jaguar.c \
	src/emu/cpu/jaguar/jaguar.h \

$(OBJ)/emu/cpu/konami/konamops.o : \
	src/emu/cpu/konami/konamops.c \

$(OBJ)/emu/cpu/konami/konamtbl.o : \
	src/emu/cpu/konami/konamtbl.c \

$(OBJ)/emu/cpu/lc8670/lc8670.o : \
	src/emu/cpu/lc8670/lc8670.c \
	src/emu/cpu/lc8670/lc8670.h \

$(OBJ)/emu/cpu/lc8670/lc8670dsm.o : \
	src/emu/cpu/lc8670/lc8670.h \
	src/emu/cpu/lc8670/lc8670dsm.c \

$(OBJ)/emu/cpu/lh5801/5801dasm.o : \
	src/emu/cpu/lh5801/5801dasm.c \
	src/emu/cpu/lh5801/lh5801.h \

$(OBJ)/emu/cpu/lh5801/5801tbl.o : \
	src/emu/cpu/lh5801/5801tbl.c \

$(OBJ)/emu/cpu/lh5801/lh5801.o : \
	src/emu/cpu/lh5801/lh5801.c \
	src/emu/cpu/lh5801/5801tbl.c \
	src/emu/cpu/lh5801/lh5801.h \

$(OBJ)/emu/cpu/lr35902/lr35902.o : \
	src/emu/cpu/lr35902/opc_cb.h \
	src/emu/cpu/lr35902/lr35902.h \
	src/emu/cpu/lr35902/opc_main.h \
	src/emu/cpu/lr35902/lr35902.c \

$(OBJ)/emu/cpu/lr35902/lr35902d.o : \
	src/emu/cpu/lr35902/lr35902.h \
	src/emu/cpu/lr35902/lr35902d.c \

$(OBJ)/emu/cpu/m37710/m37710.o : \
	src/emu/cpu/m37710/m37710cm.h \
	src/emu/cpu/m37710/m7700ds.h \
	src/emu/cpu/m37710/m37710.c \
	src/emu/cpu/m37710/m37710.h \

$(OBJ)/emu/cpu/m37710/m37710o0.o : \
	src/emu/cpu/m37710/m37710op.h \
	src/emu/cpu/m37710/m37710cm.h \
	src/emu/cpu/m37710/m37710.h \
	src/emu/cpu/m37710/m37710o0.c \

$(OBJ)/emu/cpu/m37710/m37710o1.o : \
	src/emu/cpu/m37710/m37710op.h \
	src/emu/cpu/m37710/m37710o1.c \
	src/emu/cpu/m37710/m37710cm.h \
	src/emu/cpu/m37710/m37710.h \

$(OBJ)/emu/cpu/m37710/m37710o2.o : \
	src/emu/cpu/m37710/m37710o2.c \
	src/emu/cpu/m37710/m37710op.h \
	src/emu/cpu/m37710/m37710cm.h \
	src/emu/cpu/m37710/m37710.h \

$(OBJ)/emu/cpu/m37710/m37710o3.o : \
	src/emu/cpu/m37710/m37710op.h \
	src/emu/cpu/m37710/m37710cm.h \
	src/emu/cpu/m37710/m37710o3.c \
	src/emu/cpu/m37710/m37710.h \

$(OBJ)/emu/cpu/m37710/m7700ds.o : \
	src/emu/cpu/m37710/m7700ds.c \
	src/emu/cpu/m37710/m7700ds.h \

$(OBJ)/emu/cpu/m6502/deco16.o : \
	src/emu/cpu/m6502/deco16.c \
	src/emu/cpu/m6502/deco16.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/emu/cpu/m6502/m3745x.o : \
	src/emu/cpu/m6502/m740.h \
	src/emu/cpu/m6502/m3745x.h \
	src/emu/cpu/m6502/m3745x.c \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/emu/cpu/m6502/m4510.o : \
	src/emu/cpu/m6502/m4510.c \
	src/emu/cpu/m6502/m4510.h \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/cpu/m6502/m65ce02.h \

$(OBJ)/emu/cpu/m6502/m5074x.o : \
	src/emu/cpu/m6502/m5074x.h \
	src/emu/cpu/m6502/m740.h \
	src/emu/cpu/m6502/m5074x.c \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/emu/cpu/m6502/m6502.o : \
	src/emu/cpu/m6502/m6502.c \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/emu/cpu/m6502/m6504.o : \
	src/emu/cpu/m6502/m6504.c \
	src/emu/cpu/m6502/m6504.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/emu/cpu/m6502/m6509.o : \
	src/emu/cpu/m6502/m6509.c \
	src/emu/cpu/m6502/m6502.h \
	src/emu/cpu/m6502/m6509.h \

$(OBJ)/emu/cpu/m6502/m6510.o : \
	src/emu/cpu/m6502/m6510.c \
	src/emu/cpu/m6502/m6510.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/emu/cpu/m6502/m6510t.o : \
	src/emu/cpu/m6502/m6510t.c \
	src/emu/cpu/m6502/m6510t.h \
	src/emu/cpu/m6502/m6510.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/emu/cpu/m6502/m65c02.o : \
	src/emu/cpu/m6502/m65c02.c \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/emu/cpu/m6502/m65ce02.o : \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/cpu/m6502/m65ce02.c \
	src/emu/cpu/m6502/m6502.h \
	src/emu/cpu/m6502/m65ce02.h \

$(OBJ)/emu/cpu/m6502/m65sc02.o : \
	src/emu/cpu/m6502/m65sc02.c \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/cpu/m6502/m65sc02.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/cpu/m6502/r65c02.h \

$(OBJ)/emu/cpu/m6502/m740.o : \
	src/emu/cpu/m6502/m740.c \
	src/emu/cpu/m6502/m740.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/emu/cpu/m6502/m7501.o : \
	src/emu/cpu/m6502/m7501.h \
	src/emu/cpu/m6502/m6510.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/cpu/m6502/m7501.c \

$(OBJ)/emu/cpu/m6502/m8502.o : \
	src/emu/cpu/m6502/m6510.h \
	src/emu/cpu/m6502/m8502.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/cpu/m6502/m8502.c \

$(OBJ)/emu/cpu/m6502/n2a03.o : \
	src/emu/cpu/m6502/n2a03.c \
	src/emu/cpu/m6502/n2a03.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/emu/cpu/m6502/r65c02.o : \
	src/emu/cpu/m6502/r65c02.c \
	src/emu/cpu/m6502/m65c02.h \
	src/emu/cpu/m6502/m6502.h \
	src/emu/cpu/m6502/r65c02.h \

$(OBJ)/emu/cpu/m6800/6800dasm.o : \
	src/emu/cpu/m6800/6800dasm.c \
	src/emu/cpu/m6800/m6800.h \

$(OBJ)/emu/cpu/m6800/6800ops.o : \
	src/emu/cpu/m6800/6800ops.c \

$(OBJ)/emu/cpu/m6800/6800tbl.o : \
	src/emu/cpu/m6800/6800tbl.c \

$(OBJ)/emu/cpu/m6800/m6800.o : \
	src/emu/cpu/m6800/m6800.c \
	src/emu/cpu/m6800/6800ops.c \
	src/emu/cpu/m6800/m6800.h \
	src/emu/cpu/m6800/6800tbl.c \

$(OBJ)/emu/cpu/m68000/m68k_in.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/m68000/m68k_in.c \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/cpu/m68000/m68kcpu.h \

$(OBJ)/emu/cpu/m68000/m68kcpu.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/m68000/m68kfpu.c \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/cpu/m68000/m68kcpu.h \
	src/emu/cpu/m68000/m68kmmu.h \
	src/emu/cpu/m68000/m68kcpu.c \

$(OBJ)/emu/cpu/m68000/m68kdasm.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/cpu/m68000/m68kdasm.c \

$(OBJ)/emu/cpu/m68000/m68kfpu.o : \
	src/emu/cpu/m68000/m68kfpu.c \

$(OBJ)/emu/cpu/m68000/m68kmake.o : \
	src/emu/cpu/m68000/m68kmake.c \

$(OBJ)/emu/cpu/m6805/6805dasm.o : \
	src/emu/cpu/m6805/6805dasm.c \
	src/emu/cpu/m6805/m6805.h \

$(OBJ)/emu/cpu/m6805/6805ops.o : \
	src/emu/cpu/m6805/6805ops.c \

$(OBJ)/emu/cpu/m6805/m6805.o : \
	src/emu/cpu/m6805/6805ops.c \
	src/emu/cpu/m6805/m6805.c \
	src/emu/cpu/m6805/m6805.h \

$(OBJ)/emu/cpu/m6809/6309dasm.o : \
	src/emu/cpu/m6809/hd6309.h \
	src/emu/cpu/m6809/6309dasm.c \
	src/emu/cpu/m6809/m6809.h \

$(OBJ)/emu/cpu/m6809/6809dasm.o : \
	src/emu/cpu/m6809/m6809inl.h \
	src/emu/cpu/m6809/6809dasm.c \
	src/emu/cpu/m6809/m6809.h \

$(OBJ)/emu/cpu/m6809/hd6309.o : \
	src/emu/cpu/m6809/hd6309.h \
	src/emu/cpu/m6809/m6809inl.h \
	src/emu/cpu/m6809/hd6309.c \
	src/emu/cpu/m6809/m6809.h \

$(OBJ)/emu/cpu/m6809/knmidasm.o : \
	src/emu/cpu/m6809/knmidasm.c \
	src/emu/cpu/m6809/konami.h \
	src/emu/cpu/m6809/m6809.h \

$(OBJ)/emu/cpu/m6809/konami.o : \
	src/emu/cpu/m6809/m6809inl.h \
	src/emu/cpu/m6809/konami.c \
	src/emu/cpu/m6809/konami.h \
	src/emu/cpu/m6809/m6809.h \

$(OBJ)/emu/cpu/m6809/m6809.o : \
	src/emu/cpu/m6809/m6809inl.h \
	src/emu/cpu/m6809/m6809.c \
	src/emu/cpu/m6809/m6809.h \

$(OBJ)/emu/cpu/mb86233/mb86233.o : \
	src/emu/cpu/mb86233/mb86233.c \
	src/emu/cpu/mb86233/mb86233.h \

$(OBJ)/emu/cpu/mb86233/mb86233d.o : \
	src/emu/cpu/mb86233/mb86233d.c \
	src/emu/cpu/mb86233/mb86233.h \

$(OBJ)/emu/cpu/mb88xx/mb88dasm.o : \
	src/emu/cpu/mb88xx/mb88dasm.c \
	src/emu/cpu/mb88xx/mb88xx.h \

$(OBJ)/emu/cpu/mb88xx/mb88xx.o : \
	src/emu/cpu/mb88xx/mb88xx.h \
	src/emu/cpu/mb88xx/mb88xx.c \

$(OBJ)/emu/cpu/mc68hc11/hc11dasm.o : \
	src/emu/cpu/mc68hc11/hc11dasm.c \

$(OBJ)/emu/cpu/mc68hc11/hc11ops.o : \
	src/emu/cpu/mc68hc11/hc11ops.c \

$(OBJ)/emu/cpu/mc68hc11/mc68hc11.o : \
	src/emu/cpu/mc68hc11/hc11ops.h \
	src/emu/cpu/mc68hc11/mc68hc11.c \
	src/emu/cpu/mc68hc11/hc11ops.c \
	src/emu/cpu/mc68hc11/mc68hc11.h \

$(OBJ)/emu/cpu/mcs48/mcs48.o : \
	src/emu/cpu/mcs48/mcs48.c \
	src/emu/cpu/mcs48/mcs48.h \

$(OBJ)/emu/cpu/mcs48/mcs48dsm.o : \
	src/emu/cpu/mcs48/mcs48dsm.c \

$(OBJ)/emu/cpu/mcs51/mcs51.o : \
	src/emu/cpu/mcs51/mcs51.c \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/mcs51/mcs51ops.c \

$(OBJ)/emu/cpu/mcs51/mcs51dasm.o : \
	src/emu/cpu/mcs51/mcs51.h \
	src/emu/cpu/mcs51/mcs51dasm.c \

$(OBJ)/emu/cpu/mcs51/mcs51ops.o : \
	src/emu/cpu/mcs51/mcs51ops.c \

$(OBJ)/emu/cpu/mcs96/i8x9x.o : \
	src/emu/cpu/mcs96/mcs96.h \
	src/emu/cpu/mcs96/i8x9x.h \
	src/emu/cpu/mcs96/i8x9x.c \

$(OBJ)/emu/cpu/mcs96/i8xc196.o : \
	src/emu/cpu/mcs96/i8xc196.c \
	src/emu/cpu/mcs96/mcs96.h \
	src/emu/cpu/mcs96/i8xc196.h \

$(OBJ)/emu/cpu/mcs96/mcs96.o : \
	src/emu/cpu/mcs96/mcs96.c \
	src/emu/cpu/mcs96/mcs96.h \

$(OBJ)/emu/cpu/minx/minx.o : \
	src/emu/cpu/minx/minxfunc.h \
	src/emu/cpu/minx/minxops.h \
	src/emu/cpu/minx/minx.c \
	src/emu/cpu/minx/minxopce.h \
	src/emu/cpu/minx/minxopcf.h \
	src/emu/cpu/minx/minx.h \

$(OBJ)/emu/cpu/minx/minxd.o : \
	src/emu/cpu/minx/minxd.c \
	src/emu/cpu/minx/minx.h \

$(OBJ)/emu/cpu/mips/mips3.o : \
	src/emu/cpu/mips/mips3.h \
	src/emu/cpu/mips/mips3com.h \
	src/emu/cpu/vtlb.h \
	src/emu/cpu/mips/mips3.c \

$(OBJ)/emu/cpu/mips/mips3com.o : \
	src/emu/cpu/mips/mips3com.c \
	src/emu/cpu/mips/mips3.h \
	src/emu/cpu/mips/mips3com.h \
	src/emu/cpu/vtlb.h \

$(OBJ)/emu/cpu/mips/mips3drc.o : \
	src/emu/cpu/mips/mips3.h \
	src/emu/cpu/mips/mips3fe.h \
	src/emu/cpu/mips/mips3com.h \
	src/emu/cpu/mips/mips3drc.c \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/vtlb.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/emu/cpu/mips/mips3dsm.o : \
	src/emu/cpu/mips/mips3dsm.c \

$(OBJ)/emu/cpu/mips/mips3fe.o : \
	src/emu/cpu/mips/mips3.h \
	src/emu/cpu/mips/mips3fe.h \
	src/emu/cpu/mips/mips3com.h \
	src/emu/cpu/vtlb.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/mips/mips3fe.c \

$(OBJ)/emu/cpu/mips/r3000.o : \
	src/emu/cpu/mips/r3000.c \
	src/emu/cpu/mips/r3000.h \

$(OBJ)/emu/cpu/mips/r3kdasm.o : \
	src/emu/cpu/mips/r3kdasm.c \

$(OBJ)/emu/cpu/mn10200/mn10200.o : \
	src/emu/cpu/mn10200/mn10200.c \
	src/emu/cpu/mn10200/mn10200.h \

$(OBJ)/emu/cpu/mn10200/mn102dis.o : \
	src/emu/cpu/mn10200/mn102dis.c \

$(OBJ)/emu/cpu/nec/nec.o : \
	src/emu/cpu/nec/nec.c \
	src/emu/cpu/nec/necmodrm.h \
	src/emu/cpu/nec/nec.h \
	src/emu/cpu/nec/necea.h \
	src/emu/cpu/nec/necmacro.h \
	src/emu/cpu/nec/necinstr.c \
	src/emu/cpu/nec/necpriv.h \
	src/emu/cpu/nec/necinstr.h \

$(OBJ)/emu/cpu/nec/necdasm.o : \
	src/emu/cpu/nec/necdasm.c \

$(OBJ)/emu/cpu/nec/necinstr.o : \
	src/emu/cpu/nec/necinstr.c \

$(OBJ)/emu/cpu/nec/v25.o : \
	src/emu/cpu/nec/v25.c \
	src/emu/cpu/nec/v25instr.c \
	src/emu/cpu/nec/necmodrm.h \
	src/emu/cpu/nec/necea.h \
	src/emu/cpu/nec/necmacro.h \
	src/emu/cpu/nec/necinstr.c \
	src/emu/cpu/nec/v25priv.h \
	src/emu/cpu/nec/v25.h \
	src/emu/cpu/nec/v25instr.h \

$(OBJ)/emu/cpu/nec/v25instr.o : \
	src/emu/cpu/nec/v25instr.c \

$(OBJ)/emu/cpu/nec/v25sfr.o : \
	src/emu/cpu/nec/v25sfr.c \
	src/emu/cpu/nec/v25priv.h \
	src/emu/cpu/nec/v25.h \

$(OBJ)/emu/cpu/pdp1/pdp1.o : \
	src/emu/cpu/pdp1/pdp1.c \
	src/emu/cpu/pdp1/pdp1.h \

$(OBJ)/emu/cpu/pdp1/pdp1dasm.o : \
	src/emu/cpu/pdp1/pdp1.h \
	src/emu/cpu/pdp1/pdp1dasm.c \

$(OBJ)/emu/cpu/pdp1/tx0.o : \
	src/emu/cpu/pdp1/tx0.h \
	src/emu/cpu/pdp1/tx0.c \

$(OBJ)/emu/cpu/pdp1/tx0dasm.o : \
	src/emu/cpu/pdp1/tx0dasm.c \
	src/emu/cpu/pdp1/tx0.h \

$(OBJ)/emu/cpu/pic16c5x/16c5xdsm.o : \
	src/emu/cpu/pic16c5x/16c5xdsm.c \

$(OBJ)/emu/cpu/pic16c5x/dis16c5x.o : \
	src/emu/cpu/pic16c5x/dis16c5x.c \
	src/emu/cpu/pic16c5x/16c5xdsm.c \

$(OBJ)/emu/cpu/pic16c5x/pic16c5x.o : \
	src/emu/cpu/pic16c5x/pic16c5x.c \
	src/emu/cpu/pic16c5x/pic16c5x.h \

$(OBJ)/emu/cpu/pic16c62x/16c62xdsm.o : \
	src/emu/cpu/pic16c62x/16c62xdsm.c \

$(OBJ)/emu/cpu/pic16c62x/dis16c62x.o : \
	src/emu/cpu/pic16c62x/16c62xdsm.c \
	src/emu/cpu/pic16c62x/dis16c62x.c \

$(OBJ)/emu/cpu/pic16c62x/pic16c62x.o : \
	src/emu/cpu/pic16c62x/pic16c62x.c \
	src/emu/cpu/pic16c62x/pic16c62x.h \

$(OBJ)/emu/cpu/powerpc/drc_ops.o : \
	src/emu/cpu/powerpc/drc_ops.c \

$(OBJ)/emu/cpu/powerpc/ppc.o : \
	src/emu/cpu/powerpc/ppc.c \
	src/emu/cpu/powerpc/ppc_ops.h \
	src/emu/cpu/powerpc/ppc_ops.c \
	src/emu/cpu/powerpc/ppc602.c \
	src/emu/cpu/powerpc/ppc_mem.c \
	src/emu/cpu/powerpc/ppc403.c \
	src/emu/cpu/powerpc/ppc.h \
	src/emu/cpu/powerpc/ppc603.c \

$(OBJ)/emu/cpu/powerpc/ppc403.o : \
	src/emu/cpu/powerpc/ppc403.c \

$(OBJ)/emu/cpu/powerpc/ppc602.o : \
	src/emu/cpu/powerpc/ppc602.c \

$(OBJ)/emu/cpu/powerpc/ppc603.o : \
	src/emu/cpu/powerpc/ppc603.c \

$(OBJ)/emu/cpu/powerpc/ppc_dasm.o : \
	src/emu/cpu/powerpc/ppccom.h \
	src/emu/cpu/powerpc/ppc_dasm.c \
	src/emu/cpu/vtlb.h \
	src/emu/cpu/powerpc/ppc.h \

$(OBJ)/emu/cpu/powerpc/ppc_mem.o : \
	src/emu/cpu/powerpc/ppc_mem.c \

$(OBJ)/emu/cpu/powerpc/ppc_ops.o : \
	src/emu/cpu/powerpc/ppc_ops.c \

$(OBJ)/emu/cpu/powerpc/ppccom.o : \
	src/emu/cpu/powerpc/ppccom.h \
	src/emu/cpu/powerpc/ppccom.c \
	src/emu/cpu/vtlb.h \
	src/emu/cpu/powerpc/ppc.h \

$(OBJ)/emu/cpu/powerpc/ppcdrc.o : \
	src/emu/cpu/powerpc/ppcfe.h \
	src/emu/cpu/powerpc/ppccom.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/powerpc/ppcdrc.c \
	src/emu/cpu/vtlb.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \
	src/emu/cpu/powerpc/ppc.h \

$(OBJ)/emu/cpu/powerpc/ppcfe.o : \
	src/emu/cpu/powerpc/ppcfe.h \
	src/emu/cpu/powerpc/ppccom.h \
	src/emu/cpu/powerpc/ppcfe.c \
	src/emu/cpu/vtlb.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/powerpc/ppc.h \

$(OBJ)/emu/cpu/pps4/pps4.o : \
	src/emu/cpu/pps4/pps4.c \
	src/emu/cpu/pps4/pps4.h \

$(OBJ)/emu/cpu/pps4/pps4dasm.o : \
	src/emu/cpu/pps4/pps4dasm.c \

$(OBJ)/emu/cpu/psx/dismips.o : \
	src/emu/cpu/psx/gte.h \
	src/emu/machine/ram.h \
	src/emu/cpu/psx/psx.h \
	src/emu/cpu/psx/dismips.c \
	src/emu/cpu/psx/dma.h \
	src/emu/cpu/psx/irq.h \
	src/emu/cpu/psx/sio.h \
	src/emu/cpu/psx/siodev.h \

$(OBJ)/emu/cpu/psx/dma.o : \
	src/emu/cpu/psx/dma.c \
	src/emu/cpu/psx/dma.h \

$(OBJ)/emu/cpu/psx/gte.o : \
	src/emu/cpu/psx/gte.h \
	src/emu/cpu/psx/gte.c \

$(OBJ)/emu/cpu/psx/irq.o : \
	src/emu/cpu/psx/gte.h \
	src/emu/machine/ram.h \
	src/emu/cpu/psx/psx.h \
	src/emu/cpu/psx/dma.h \
	src/emu/cpu/psx/irq.c \
	src/emu/cpu/psx/irq.h \
	src/emu/cpu/psx/sio.h \
	src/emu/cpu/psx/siodev.h \

$(OBJ)/emu/cpu/psx/mdec.o : \
	src/emu/cpu/psx/mdec.c \
	src/emu/cpu/psx/dma.h \
	src/emu/cpu/psx/mdec.h \

$(OBJ)/emu/cpu/psx/psx.o : \
	src/emu/cpu/psx/psx.c \
	src/emu/cpu/psx/rcnt.h \
	src/emu/cpu/psx/gte.h \
	src/emu/machine/ram.h \
	src/emu/cpu/psx/psx.h \
	src/emu/cpu/psx/dma.h \
	src/emu/sound/spu.h \
	src/emu/cpu/psx/mdec.h \
	src/emu/cpu/psx/irq.h \
	src/emu/cpu/psx/sio.h \
	src/emu/cpu/psx/siodev.h \
	src/emu/sound/spureverb.h \

$(OBJ)/emu/cpu/psx/psxdasm.o : \
	src/emu/cpu/psx/gte.h \
	src/emu/machine/ram.h \
	src/emu/cpu/psx/psx.h \
	src/emu/cpu/psx/dma.h \
	src/emu/cpu/psx/irq.h \
	src/emu/cpu/psx/sio.h \
	src/emu/cpu/psx/psxdasm.c \
	src/emu/cpu/psx/siodev.h \

$(OBJ)/emu/cpu/psx/rcnt.o : \
	src/emu/cpu/psx/rcnt.h \
	src/emu/cpu/psx/rcnt.c \

$(OBJ)/emu/cpu/psx/sio.o : \
	src/emu/cpu/psx/sio.c \
	src/emu/cpu/psx/sio.h \
	src/emu/cpu/psx/siodev.h \

$(OBJ)/emu/cpu/psx/siodev.o : \
	src/emu/cpu/psx/siodev.c \
	src/emu/cpu/psx/sio.h \
	src/emu/cpu/psx/siodev.h \

$(OBJ)/emu/cpu/rsp/rsp.o : \
	src/emu/cpu/rsp/rsp.c \
	src/emu/cpu/rsp/rsp.h \
	src/emu/cpu/rsp/rspdiv.h \

$(OBJ)/emu/cpu/rsp/rsp_dasm.o : \
	src/emu/cpu/rsp/rsp_dasm.c \

$(OBJ)/emu/cpu/rsp/rspdrc.o : \
	src/emu/cpu/rsp/rspfe.h \
	src/emu/cpu/rsp/rspdrc.c \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/rsp/rsp.h \
	src/emu/cpu/rsp/rspdiv.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/emu/cpu/rsp/rspfe.o : \
	src/emu/cpu/rsp/rspfe.h \
	src/emu/cpu/rsp/rsp.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/rsp/rspfe.c \

$(OBJ)/emu/cpu/s2650/2650dasm.o : \
	src/emu/cpu/s2650/2650dasm.c \

$(OBJ)/emu/cpu/s2650/s2650.o : \
	src/emu/cpu/s2650/s2650.c \
	src/emu/cpu/s2650/s2650.h \
	src/emu/cpu/s2650/s2650cpu.h \

$(OBJ)/emu/cpu/saturn/satops.o : \
	src/emu/cpu/saturn/satops.c \

$(OBJ)/emu/cpu/saturn/sattable.o : \
	src/emu/cpu/saturn/sattable.c \

$(OBJ)/emu/cpu/saturn/saturn.o : \
	src/emu/cpu/saturn/saturn.c \
	src/emu/cpu/saturn/saturn.h \
	src/emu/cpu/saturn/sattable.c \
	src/emu/cpu/saturn/satops.c \

$(OBJ)/emu/cpu/saturn/saturnds.o : \
	src/emu/cpu/saturn/saturnds.c \
	src/emu/cpu/saturn/saturn.h \

$(OBJ)/emu/cpu/sc61860/readpc.o : \
	src/emu/cpu/sc61860/readpc.c \

$(OBJ)/emu/cpu/sc61860/sc61860.o : \
	src/emu/cpu/sc61860/sc61860.c \
	src/emu/cpu/sc61860/sc61860.h \
	src/emu/cpu/sc61860/scops.c \
	src/emu/cpu/sc61860/sctable.c \
	src/emu/cpu/sc61860/sc.h \

$(OBJ)/emu/cpu/sc61860/scdasm.o : \
	src/emu/cpu/sc61860/scdasm.c \
	src/emu/cpu/sc61860/sc61860.h \
	src/emu/cpu/sc61860/sc.h \

$(OBJ)/emu/cpu/sc61860/scops.o : \
	src/emu/cpu/sc61860/scops.c \

$(OBJ)/emu/cpu/sc61860/sctable.o : \
	src/emu/cpu/sc61860/sctable.c \

$(OBJ)/emu/cpu/scmp/scmp.o : \
	src/emu/cpu/scmp/scmp.c \
	src/emu/cpu/scmp/scmp.h \

$(OBJ)/emu/cpu/scmp/scmpdasm.o : \
	src/emu/cpu/scmp/scmpdasm.c \

$(OBJ)/emu/cpu/score/score.o : \
	src/emu/cpu/score/scorem.h \
	src/emu/cpu/score/score.c \
	src/emu/cpu/score/score.h \

$(OBJ)/emu/cpu/score/scoredsm.o : \
	src/emu/cpu/score/scorem.h \
	src/emu/cpu/score/score.h \
	src/emu/cpu/score/scoredsm.c \

$(OBJ)/emu/cpu/scudsp/scudsp.o : \
	src/emu/cpu/scudsp/scudsp.c \
	src/emu/cpu/scudsp/scudsp.h \

$(OBJ)/emu/cpu/scudsp/scudspdasm.o : \
	src/emu/cpu/scudsp/scudsp.h \
	src/emu/cpu/scudsp/scudspdasm.c \

$(OBJ)/emu/cpu/se3208/se3208.o : \
	src/emu/cpu/se3208/se3208.c \
	src/emu/cpu/se3208/se3208.h \

$(OBJ)/emu/cpu/se3208/se3208dis.o : \
	src/emu/cpu/se3208/se3208dis.c \
	src/emu/cpu/se3208/se3208.h \

$(OBJ)/emu/cpu/sh2/sh2.o : \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/sh2/sh2.c \
	src/emu/cpu/sh2/sh2comn.h \
	src/emu/cpu/sh2/sh2.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/emu/cpu/sh2/sh2comn.o : \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/sh2/sh2comn.h \
	src/emu/cpu/sh2/sh2.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/sh2/sh2comn.c \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/emu/cpu/sh2/sh2dasm.o : \
	src/emu/cpu/sh2/sh2.h \
	src/emu/cpu/sh2/sh2dasm.c \

$(OBJ)/emu/cpu/sh2/sh2drc.o : \
	src/emu/cpu/sh2/sh2drc.c \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/sh2/sh2comn.h \
	src/emu/cpu/sh2/sh2.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/emu/cpu/sh2/sh2fe.o : \
	src/emu/cpu/sh2/sh2fe.c \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/sh2/sh2comn.h \
	src/emu/cpu/sh2/sh2.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/emu/cpu/sh4/sh3comn.o : \
	src/emu/cpu/sh4/sh4.h \
	src/emu/cpu/sh4/sh3comn.h \
	src/emu/cpu/sh4/sh3comn.c \
	src/emu/cpu/uml.h \
	src/emu/cpu/sh4/sh4tmu.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/sh4/sh4dmac.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \
	src/emu/cpu/sh4/sh4comn.h \

$(OBJ)/emu/cpu/sh4/sh4.o : \
	src/emu/cpu/sh4/sh4.h \
	src/emu/cpu/sh4/sh3comn.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/sh4/sh4tmu.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/sh4/sh4regs.h \
	src/emu/cpu/drcumlsh.h \
	src/emu/cpu/sh4/sh4.c \
	src/emu/cpu/sh4/sh4comn.h \

$(OBJ)/emu/cpu/sh4/sh4comn.o : \
	src/emu/cpu/sh4/sh4.h \
	src/emu/cpu/sh4/sh3comn.h \
	src/emu/cpu/sh4/sh4comn.c \
	src/emu/cpu/uml.h \
	src/emu/cpu/sh4/sh4tmu.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/sh4/sh4dmac.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/sh4/sh4regs.h \
	src/emu/cpu/drcumlsh.h \
	src/emu/cpu/sh4/sh4comn.h \

$(OBJ)/emu/cpu/sh4/sh4dasm.o : \
	src/emu/cpu/sh4/sh4.h \
	src/emu/cpu/sh4/sh4dasm.c \

$(OBJ)/emu/cpu/sh4/sh4dmac.o : \
	src/emu/cpu/sh4/sh4.h \
	src/emu/cpu/sh4/sh3comn.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/sh4/sh4dmac.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/sh4/sh4dmac.c \
	src/emu/cpu/drcumlsh.h \
	src/emu/cpu/sh4/sh4comn.h \

$(OBJ)/emu/cpu/sh4/sh4tmu.o : \
	src/emu/cpu/sh4/sh4.h \
	src/emu/cpu/sh4/sh3comn.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/sh4/sh4tmu.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/sh4/sh4tmu.c \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcumlsh.h \
	src/emu/cpu/sh4/sh4comn.h \

$(OBJ)/emu/cpu/sharc/compute.o : \
	src/emu/cpu/sharc/compute.c \

$(OBJ)/emu/cpu/sharc/sharc.o : \
	src/emu/cpu/sharc/sharcdma.c \
	src/emu/cpu/sharc/sharc.c \
	src/emu/cpu/sharc/sharcops.h \
	src/emu/cpu/sharc/sharcops.c \
	src/emu/cpu/sharc/sharc.h \
	src/emu/cpu/sharc/compute.c \
	src/emu/cpu/sharc/sharcmem.c \

$(OBJ)/emu/cpu/sharc/sharcdma.o : \
	src/emu/cpu/sharc/sharcdma.c \

$(OBJ)/emu/cpu/sharc/sharcdsm.o : \
	src/emu/cpu/sharc/sharcdsm.h \
	src/emu/cpu/sharc/sharcdsm.c \

$(OBJ)/emu/cpu/sharc/sharcmem.o : \
	src/emu/cpu/sharc/sharcmem.c \

$(OBJ)/emu/cpu/sharc/sharcops.o : \
	src/emu/cpu/sharc/sharcops.c \
	src/emu/cpu/sharc/compute.c \

$(OBJ)/emu/cpu/sm8500/sm8500.o : \
	src/emu/cpu/sm8500/sm8500.h \
	src/emu/cpu/sm8500/sm85ops.h \
	src/emu/cpu/sm8500/sm8500.c \

$(OBJ)/emu/cpu/sm8500/sm8500d.o : \
	src/emu/cpu/sm8500/sm8500.h \
	src/emu/cpu/sm8500/sm8500d.c \

$(OBJ)/emu/cpu/spc700/spc700.o : \
	src/emu/cpu/spc700/spc700ds.h \
	src/emu/cpu/spc700/spc700.c \
	src/emu/cpu/spc700/spc700.h \

$(OBJ)/emu/cpu/spc700/spc700ds.o : \
	src/emu/cpu/spc700/spc700ds.h \
	src/emu/cpu/spc700/spc700ds.c \

$(OBJ)/emu/cpu/ssem/ssem.o : \
	src/emu/cpu/ssem/ssem.h \
	src/emu/cpu/ssem/ssem.c \

$(OBJ)/emu/cpu/ssem/ssemdasm.o : \
	src/emu/cpu/ssem/ssemdasm.c \

$(OBJ)/emu/cpu/ssp1601/ssp1601.o : \
	src/emu/cpu/ssp1601/ssp1601.c \
	src/emu/cpu/ssp1601/ssp1601.h \

$(OBJ)/emu/cpu/ssp1601/ssp1601d.o : \
	src/emu/cpu/ssp1601/ssp1601d.c \

$(OBJ)/emu/cpu/superfx/sfx_dasm.o : \
	src/emu/cpu/superfx/superfx.h \
	src/emu/cpu/superfx/sfx_dasm.c \

$(OBJ)/emu/cpu/superfx/superfx.o : \
	src/emu/cpu/superfx/superfx.h \
	src/emu/cpu/superfx/superfx.c \

$(OBJ)/emu/cpu/t11/t11.o : \
	src/emu/cpu/t11/t11table.c \
	src/emu/cpu/t11/t11ops.c \
	src/emu/cpu/t11/t11.c \
	src/emu/cpu/t11/t11.h \

$(OBJ)/emu/cpu/t11/t11dasm.o : \
	src/emu/cpu/t11/t11.h \
	src/emu/cpu/t11/t11dasm.c \

$(OBJ)/emu/cpu/t11/t11ops.o : \
	src/emu/cpu/t11/t11ops.c \

$(OBJ)/emu/cpu/t11/t11table.o : \
	src/emu/cpu/t11/t11table.c \

$(OBJ)/emu/cpu/tlcs90/tlcs90.o : \
	src/emu/cpu/tlcs90/tlcs90.h \
	src/emu/cpu/tlcs90/tlcs90.c \

$(OBJ)/emu/cpu/tlcs900/900tbl.o : \
	src/emu/cpu/tlcs900/900tbl.c \

$(OBJ)/emu/cpu/tlcs900/dasm900.o : \
	src/emu/cpu/tlcs900/tlcs900.h \
	src/emu/cpu/tlcs900/dasm900.c \

$(OBJ)/emu/cpu/tlcs900/tlcs900.o : \
	src/emu/cpu/tlcs900/tlcs900.c \
	src/emu/cpu/tlcs900/tlcs900.h \
	src/emu/cpu/tlcs900/900tbl.c \

$(OBJ)/emu/cpu/tms0980/tms0980.o : \
	src/emu/cpu/tms0980/tms0980.h \
	src/emu/cpu/tms0980/tms0980.c \

$(OBJ)/emu/cpu/tms0980/tms0980d.o : \
	src/emu/cpu/tms0980/tms0980d.c \
	src/emu/cpu/tms0980/tms0980.h \

$(OBJ)/emu/cpu/tms32010/32010dsm.o : \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/tms32010/32010dsm.c \

$(OBJ)/emu/cpu/tms32010/dis32010.o : \
	src/emu/cpu/tms32010/tms32010.h \
	src/emu/cpu/tms32010/dis32010.c \
	src/emu/cpu/tms32010/32010dsm.c \

$(OBJ)/emu/cpu/tms32010/tms32010.o : \
	src/emu/cpu/tms32010/tms32010.c \
	src/emu/cpu/tms32010/tms32010.h \

$(OBJ)/emu/cpu/tms32025/32025dsm.o : \
	src/emu/cpu/tms32025/32025dsm.c \
	src/emu/cpu/tms32025/tms32025.h \

$(OBJ)/emu/cpu/tms32025/dis32025.o : \
	src/emu/cpu/tms32025/32025dsm.c \
	src/emu/cpu/tms32025/tms32025.h \
	src/emu/cpu/tms32025/dis32025.c \

$(OBJ)/emu/cpu/tms32025/tms32025.o : \
	src/emu/cpu/tms32025/tms32025.c \
	src/emu/cpu/tms32025/tms32025.h \

$(OBJ)/emu/cpu/tms32031/32031ops.o : \
	src/emu/cpu/tms32031/32031ops.c \

$(OBJ)/emu/cpu/tms32031/dis32031.o : \
	src/emu/cpu/tms32031/tms32031.h \
	src/emu/cpu/tms32031/dis32031.c \

$(OBJ)/emu/cpu/tms32031/tms32031.o : \
	src/emu/cpu/tms32031/tms32031.c \
	src/emu/cpu/tms32031/32031ops.c \
	src/emu/cpu/tms32031/tms32031.h \

$(OBJ)/emu/cpu/tms32051/32051ops.o : \
	src/emu/cpu/tms32051/32051ops.c \

$(OBJ)/emu/cpu/tms32051/dis32051.o : \
	src/emu/cpu/tms32051/dis32051.c \

$(OBJ)/emu/cpu/tms32051/tms32051.o : \
	src/emu/cpu/tms32051/tms32051.c \
	src/emu/cpu/tms32051/32051ops.c \
	src/emu/cpu/tms32051/32051ops.h \
	src/emu/cpu/tms32051/tms32051.h \

$(OBJ)/emu/cpu/tms32082/dis_mp.o : \
	src/emu/cpu/tms32082/dis_mp.c \

$(OBJ)/emu/cpu/tms32082/dis_pp.o : \
	src/emu/cpu/tms32082/dis_pp.c \

$(OBJ)/emu/cpu/tms32082/mp_ops.o : \
	src/emu/cpu/tms32082/tms32082.h \
	src/emu/cpu/tms32082/mp_ops.c \

$(OBJ)/emu/cpu/tms32082/tms32082.o : \
	src/emu/cpu/tms32082/tms32082.h \
	src/emu/cpu/tms32082/tms32082.c \

$(OBJ)/emu/cpu/tms34010/34010dsm.o : \
	src/emu/cpu/tms34010/34010dsm.c \

$(OBJ)/emu/cpu/tms34010/34010fld.o : \
	src/emu/cpu/tms34010/34010fld.c \

$(OBJ)/emu/cpu/tms34010/34010gfx.o : \
	src/emu/cpu/tms34010/34010gfx.c \

$(OBJ)/emu/cpu/tms34010/34010ops.o : \
	src/emu/cpu/tms34010/34010ops.c \

$(OBJ)/emu/cpu/tms34010/34010tbl.o : \
	src/emu/cpu/tms34010/34010tbl.c \

$(OBJ)/emu/cpu/tms34010/dis34010.o : \
	src/emu/cpu/tms34010/34010dsm.c \
	src/emu/cpu/tms34010/dis34010.c \

$(OBJ)/emu/cpu/tms34010/tms34010.o : \
	src/emu/cpu/tms34010/34010ops.c \
	src/emu/cpu/tms34010/tms34010.c \
	src/emu/cpu/tms34010/34010ops.h \
	src/emu/cpu/tms34010/tms34010.h \
	src/emu/cpu/tms34010/34010tbl.c \
	src/emu/cpu/tms34010/34010gfx.c \
	src/emu/cpu/tms34010/34010fld.c \

$(OBJ)/emu/cpu/tms57002/57002dsm.o : \
	src/emu/cpu/tms57002/57002dsm.c \
	src/emu/cpu/tms57002/tms57002.h \

$(OBJ)/emu/cpu/tms57002/tms57002.o : \
	src/emu/cpu/tms57002/tms57002.c \
	src/emu/cpu/tms57002/tms57002.h \

$(OBJ)/emu/cpu/tms57002/tms57kdec.o : \
	src/emu/cpu/tms57002/tms57kdec.c \
	src/emu/cpu/tms57002/tms57002.h \

$(OBJ)/emu/cpu/tms7000/7000dasm.o : \
	src/emu/cpu/tms7000/7000dasm.c \
	src/emu/cpu/tms7000/tms7000.h \

$(OBJ)/emu/cpu/tms7000/tms7000.o : \
	src/emu/cpu/tms7000/tms7000.c \
	src/emu/cpu/tms7000/tms70op.c \
	src/emu/cpu/tms7000/tms7000.h \
	src/emu/cpu/tms7000/tms70tb.c \

$(OBJ)/emu/cpu/tms7000/tms70op.o : \
	src/emu/cpu/tms7000/tms70op.c \

$(OBJ)/emu/cpu/tms7000/tms70tb.o : \
	src/emu/cpu/tms7000/tms70tb.c \

$(OBJ)/emu/cpu/tms9900/9900dasm.o : \
	src/emu/cpu/tms9900/tms9900.h \
	src/emu/cpu/tms9900/9900dasm.c \
	src/emu/cpu/tms9900/tms99com.h \

$(OBJ)/emu/cpu/tms9900/ti990_10.o : \
	src/emu/cpu/tms9900/ti990_10.c \

$(OBJ)/emu/cpu/tms9900/ti990_10l.o : \
	src/emu/cpu/tms9900/99xxcore.h \
	src/emu/cpu/tms9900/tms9900l.h \
	src/emu/cpu/tms9900/ti990_10l.c \
	src/emu/cpu/tms9900/99xxstat.h \

$(OBJ)/emu/cpu/tms9900/tms9900.o : \
	src/emu/cpu/tms9900/tms9900.c \
	src/emu/cpu/tms9900/tms9900.h \
	src/emu/cpu/tms9900/tms99com.h \

$(OBJ)/emu/cpu/tms9900/tms9900l.o : \
	src/emu/cpu/tms9900/99xxcore.h \
	src/emu/cpu/tms9900/tms9900l.h \
	src/emu/cpu/tms9900/tms9900l.c \
	src/emu/cpu/tms9900/99xxstat.h \

$(OBJ)/emu/cpu/tms9900/tms9980a.o : \
	src/emu/cpu/tms9900/tms9980a.h \
	src/emu/cpu/tms9900/tms9980a.c \
	src/emu/cpu/tms9900/tms9900.h \
	src/emu/cpu/tms9900/tms99com.h \

$(OBJ)/emu/cpu/tms9900/tms9980al.o : \
	src/emu/cpu/tms9900/99xxcore.h \
	src/emu/cpu/tms9900/tms9980al.c \
	src/emu/cpu/tms9900/tms9900l.h \
	src/emu/cpu/tms9900/99xxstat.h \

$(OBJ)/emu/cpu/tms9900/tms9995.o : \
	src/emu/cpu/tms9900/tms9995.h \
	src/emu/cpu/tms9900/tms9995.c \
	src/emu/cpu/tms9900/tms99com.h \

$(OBJ)/emu/cpu/tms9900/tms9995l.o : \
	src/emu/cpu/tms9900/99xxcore.h \
	src/emu/cpu/tms9900/tms9900l.h \
	src/emu/cpu/tms9900/tms9995l.c \
	src/emu/cpu/tms9900/99xxstat.h \

$(OBJ)/emu/cpu/unsp/unsp.o : \
	src/emu/cpu/unsp/unsp.c \
	src/emu/cpu/unsp/unsp.h \

$(OBJ)/emu/cpu/unsp/unspdasm.o : \
	src/emu/cpu/unsp/unspdasm.c \

$(OBJ)/emu/cpu/upd7725/dasm7725.o : \
	src/emu/cpu/upd7725/upd7725.h \
	src/emu/cpu/upd7725/dasm7725.c \

$(OBJ)/emu/cpu/upd7725/upd7725.o : \
	src/emu/cpu/upd7725/upd7725.c \
	src/emu/cpu/upd7725/upd7725.h \

$(OBJ)/emu/cpu/upd7810/7810dasm.o : \
	src/emu/cpu/upd7810/7810dasm.c \
	src/emu/cpu/upd7810/upd7810.h \

$(OBJ)/emu/cpu/upd7810/7810ops.o : \
	src/emu/cpu/upd7810/7810ops.c \

$(OBJ)/emu/cpu/upd7810/7810tbl.o : \
	src/emu/cpu/upd7810/7810tbl.c \

$(OBJ)/emu/cpu/upd7810/upd7810.o : \
	src/emu/cpu/upd7810/7810tbl.c \
	src/emu/cpu/upd7810/upd7810.c \
	src/emu/cpu/upd7810/upd7810.h \
	src/emu/cpu/upd7810/7810ops.c \

$(OBJ)/emu/cpu/v30mz/v30mz.o : \
	src/emu/cpu/v30mz/v30mz.c \
	src/emu/cpu/v30mz/v30mz.h \

$(OBJ)/emu/cpu/v60/am.o : \
	src/emu/cpu/v60/am1.c \
	src/emu/cpu/v60/am2.c \
	src/emu/cpu/v60/am3.c \
	src/emu/cpu/v60/am.c \

$(OBJ)/emu/cpu/v60/am1.o : \
	src/emu/cpu/v60/am1.c \

$(OBJ)/emu/cpu/v60/am2.o : \
	src/emu/cpu/v60/am2.c \

$(OBJ)/emu/cpu/v60/am3.o : \
	src/emu/cpu/v60/am3.c \

$(OBJ)/emu/cpu/v60/op12.o : \
	src/emu/cpu/v60/op12.c \

$(OBJ)/emu/cpu/v60/op2.o : \
	src/emu/cpu/v60/op2.c \

$(OBJ)/emu/cpu/v60/op3.o : \
	src/emu/cpu/v60/op3.c \

$(OBJ)/emu/cpu/v60/op4.o : \
	src/emu/cpu/v60/op4.c \

$(OBJ)/emu/cpu/v60/op5.o : \
	src/emu/cpu/v60/op5.c \

$(OBJ)/emu/cpu/v60/op6.o : \
	src/emu/cpu/v60/op6.c \

$(OBJ)/emu/cpu/v60/op7a.o : \
	src/emu/cpu/v60/op7a.c \

$(OBJ)/emu/cpu/v60/optable.o : \
	src/emu/cpu/v60/optable.c \

$(OBJ)/emu/cpu/v60/v60.o : \
	src/emu/cpu/v60/am1.c \
	src/emu/cpu/v60/am2.c \
	src/emu/cpu/v60/op2.c \
	src/emu/cpu/v60/op7a.c \
	src/emu/cpu/v60/op5.c \
	src/emu/cpu/v60/v60.c \
	src/emu/cpu/v60/op4.c \
	src/emu/cpu/v60/v60.h \
	src/emu/cpu/v60/optable.c \
	src/emu/cpu/v60/am3.c \
	src/emu/cpu/v60/op3.c \
	src/emu/cpu/v60/am.c \
	src/emu/cpu/v60/op6.c \
	src/emu/cpu/v60/op12.c \

$(OBJ)/emu/cpu/v60/v60d.o : \
	src/emu/cpu/v60/v60d.c \
	src/emu/cpu/v60/v60.h \

$(OBJ)/emu/cpu/v810/v810.o : \
	src/emu/cpu/v810/v810.h \
	src/emu/cpu/v810/v810.c \

$(OBJ)/emu/cpu/v810/v810dasm.o : \
	src/emu/cpu/v810/v810.h \
	src/emu/cpu/v810/v810dasm.c \

$(OBJ)/emu/cpu/z180/z180.o : \
	src/emu/cpu/z180/z180dd.c \
	src/emu/cpu/z180/z180.h \
	src/emu/cpu/z180/z180xy.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z180/z180ops.h \
	src/emu/cpu/z180/z180tbl.h \
	src/emu/cpu/z180/z180ed.c \
	src/emu/cpu/z180/z180fd.c \
	src/emu/cpu/z180/z180op.c \
	src/emu/cpu/z180/z180cb.c \
	src/emu/cpu/z180/z180.c \

$(OBJ)/emu/cpu/z180/z180cb.o : \
	src/emu/cpu/z180/z180cb.c \

$(OBJ)/emu/cpu/z180/z180dasm.o : \
	src/emu/cpu/z180/z180.h \
	src/emu/cpu/z180/z180dasm.c \

$(OBJ)/emu/cpu/z180/z180dd.o : \
	src/emu/cpu/z180/z180dd.c \

$(OBJ)/emu/cpu/z180/z180ed.o : \
	src/emu/cpu/z180/z180ed.c \

$(OBJ)/emu/cpu/z180/z180fd.o : \
	src/emu/cpu/z180/z180fd.c \

$(OBJ)/emu/cpu/z180/z180op.o : \
	src/emu/cpu/z180/z180op.c \

$(OBJ)/emu/cpu/z180/z180xy.o : \
	src/emu/cpu/z180/z180xy.c \

$(OBJ)/emu/cpu/z8/z8.o : \
	src/emu/cpu/z8/z8ops.c \
	src/emu/cpu/z8/z8.h \
	src/emu/cpu/z8/z8.c \

$(OBJ)/emu/cpu/z8/z8dasm.o : \
	src/emu/cpu/z8/z8.h \
	src/emu/cpu/z8/z8dasm.c \

$(OBJ)/emu/cpu/z8/z8ops.o : \
	src/emu/cpu/z8/z8ops.c \

$(OBJ)/emu/cpu/z80/z80.o : \
	src/emu/cpu/z80/z80.c \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \

$(OBJ)/emu/cpu/z80/z80daisy.o : \
	src/emu/cpu/z80/z80daisy.c \
	src/emu/cpu/z80/z80daisy.h \

$(OBJ)/emu/cpu/z80/z80dasm.o : \
	src/emu/cpu/z80/z80dasm.c \
	src/emu/cpu/z80/z80.h \

$(OBJ)/emu/cpu/z8000/8000dasm.o : \
	src/emu/debug/debugcon.h \
	src/emu/cpu/z8000/z8000.h \
	src/emu/debug/debugvw.h \
	src/emu/debug/express.h \
	src/emu/cpu/z8000/z8000cpu.h \
	src/emu/debug/textbuf.h \
	src/emu/cpu/z8000/8000dasm.c \

$(OBJ)/emu/cpu/z8000/makedab.o : \
	src/emu/cpu/z8000/makedab.c \

$(OBJ)/emu/cpu/z8000/z8000.o : \
	src/emu/debug/debugcon.h \
	src/emu/cpu/z8000/z8000.c \
	src/emu/cpu/z8000/z8000ops.c \
	src/emu/cpu/z8000/z8000tbl.c \
	src/emu/cpu/z8000/z8000.h \
	src/emu/cpu/z8000/z8000dab.h \
	src/emu/debug/textbuf.h \
	src/emu/cpu/z8000/z8000cpu.h \

$(OBJ)/emu/cpu/z8000/z8000ops.o : \
	src/emu/cpu/z8000/z8000ops.c \

$(OBJ)/emu/cpu/z8000/z8000tbl.o : \
	src/emu/cpu/z8000/z8000tbl.c \

$(OBJ)/emu/cpu/drcbec.o : \
	src/emu/cpu/drcbeut.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/drcbec.h \
	src/emu/cpu/drcbec.c \
	src/emu/cpu/drcuml.h \

$(OBJ)/emu/cpu/drcbeut.o : \
	src/emu/cpu/drcbeut.h \
	src/emu/cpu/drcbeut.c \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/drcuml.h \

$(OBJ)/emu/cpu/drcbex64.o : \
	src/emu/cpu/drcbeut.h \
	src/emu/cpu/drcbex64.c \
	src/emu/cpu/drcbex64.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/x86log.h \
	src/emu/cpu/x86emit.h \
	src/emu/cpu/drcuml.h \

$(OBJ)/emu/cpu/drcbex86.o : \
	src/emu/cpu/drcbex86.c \
	src/emu/cpu/drcbeut.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/x86log.h \
	src/emu/cpu/x86emit.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcbex86.h \

$(OBJ)/emu/cpu/drccache.o : \
	src/emu/cpu/drccache.c \
	src/emu/cpu/drccache.h \

$(OBJ)/emu/cpu/drcfe.o : \
	src/emu/cpu/drcfe.h \
	src/emu/cpu/drcfe.c \

$(OBJ)/emu/cpu/drcuml.o : \
	src/emu/cpu/drcbeut.h \
	src/emu/cpu/drcbex64.h \
	src/emu/cpu/uml.h \
	src/emu/cpu/drcbec.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/x86log.h \
	src/emu/cpu/x86emit.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcbex86.h \
	src/emu/cpu/drcuml.c \

$(OBJ)/emu/cpu/uml.o : \
	src/emu/cpu/uml.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/uml.c \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/emu/cpu/vtlb.o : \
	src/emu/cpu/vtlb.c \
	src/emu/cpu/vtlb.h \

$(OBJ)/emu/cpu/x86log.o : \
	src/emu/cpu/x86log.h \
	src/emu/cpu/x86emit.h \
	src/emu/cpu/x86log.c \

$(OBJ)/emu/debug/debugcmd.o : \
	src/emu/debug/debugcon.h \
	src/emu/debug/debugcmd.c \
	src/emu/debug/debugcmd.h \
	src/emu/debug/debughlp.h \
	src/emu/debug/debugvw.h \
	src/lib/util/simple_set.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \
	src/emu/debug/textbuf.h \

$(OBJ)/emu/debug/debugcon.o : \
	src/emu/debug/debugcon.h \
	src/emu/debug/debugcon.c \
	src/emu/debug/debughlp.h \
	src/emu/debug/debugvw.h \
	src/lib/util/simple_set.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \
	src/emu/debug/textbuf.h \

$(OBJ)/emu/debug/debugcpu.o : \
	src/emu/debug/debugcon.h \
	src/emu/debug/debugcmd.h \
	src/emu/debug/debugvw.h \
	src/lib/util/simple_set.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \
	src/emu/debugint/debugint.h \
	src/emu/debug/textbuf.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/xmlfile.h \
	src/emu/debug/debugcpu.c \
	src/lib/util/astring.h \

$(OBJ)/emu/debug/debughlp.o : \
	src/emu/debug/debughlp.h \
	src/emu/debug/debughlp.c \

$(OBJ)/emu/debug/debugvw.o : \
	src/emu/debug/debugcon.h \
	src/emu/debug/dvbpoints.h \
	src/emu/debug/debugcmd.h \
	src/emu/debug/dvdisasm.h \
	src/emu/debug/debugvw.h \
	src/emu/debug/dvwpoints.h \
	src/lib/util/simple_set.h \
	src/emu/debug/debugvw.c \
	src/emu/debug/dvmemory.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \
	src/emu/debug/textbuf.h \
	src/emu/debug/dvstate.h \
	src/emu/debug/dvtext.h \

$(OBJ)/emu/debug/dvbpoints.o : \
	src/emu/debug/dvbpoints.c \
	src/emu/debug/dvbpoints.h \
	src/emu/debug/debugvw.h \
	src/lib/util/simple_set.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \

$(OBJ)/emu/debug/dvdisasm.o : \
	src/emu/debug/debugvw.h \
	src/emu/debug/dvdisasm.h \
	src/lib/util/simple_set.h \
	src/emu/debug/express.h \
	src/emu/debug/dvdisasm.c \
	src/emu/debug/debugcpu.h \

$(OBJ)/emu/debug/dvmemory.o : \
	src/emu/debug/debugvw.h \
	src/lib/util/simple_set.h \
	src/emu/debug/dvmemory.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \
	src/emu/debug/dvstate.h \
	src/emu/debug/dvmemory.c \

$(OBJ)/emu/debug/dvstate.o : \
	src/emu/debug/debugvw.h \
	src/emu/debug/express.h \
	src/emu/debug/dvstate.h \
	src/emu/debug/dvstate.c \

$(OBJ)/emu/debug/dvtext.o : \
	src/emu/debug/debugcon.h \
	src/emu/debug/debugvw.h \
	src/emu/debug/express.h \
	src/emu/debug/textbuf.h \
	src/emu/debug/dvtext.c \
	src/emu/debug/dvtext.h \

$(OBJ)/emu/debug/dvwpoints.o : \
	src/emu/debug/dvwpoints.c \
	src/emu/debug/debugvw.h \
	src/emu/debug/dvwpoints.h \
	src/lib/util/simple_set.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \

$(OBJ)/emu/debug/express.o : \
	src/emu/debug/express.c \
	src/emu/debug/express.h \

$(OBJ)/emu/debug/textbuf.o : \
	src/emu/debug/textbuf.c \
	src/emu/debug/textbuf.h \

$(OBJ)/emu/debugint/debugint.o : \
	src/emu/debug/debugcon.h \
	src/emu/debug/dvdisasm.h \
	src/emu/debug/debugvw.h \
	src/lib/util/simple_set.h \
	src/emu/debug/dvmemory.h \
	src/emu/debug/express.h \
	src/emu/debug/debugcpu.h \
	src/emu/debug/textbuf.h \
	src/emu/debug/dvstate.h \
	src/emu/debugint/debugint.c \

$(OBJ)/emu/drivers/empty.o : \
	src/emu/drivers/empty.c \

$(OBJ)/emu/drivers/emudummy.o : \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/drivers/emudummy.c \

$(OBJ)/emu/drivers/testcpu.o : \
	src/emu/drivers/testcpu.c \
	src/emu/cpu/powerpc/ppc.h \

$(OBJ)/emu/imagedev/bitbngr.o : \
	src/emu/imagedev/bitbngr.h \
	src/emu/imagedev/bitbngr.c \

$(OBJ)/emu/imagedev/cartslot.o : \
	src/emu/imagedev/cartslot.h \
	src/emu/imagedev/cartslot.c \

$(OBJ)/emu/imagedev/cassette.o : \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/imagedev/cassette.c \
	src/lib/formats/imageutl.h \
	src/lib/util/pool.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/emu/imagedev/chd_cd.o : \
	src/lib/util/palette.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/emu/imagedev/chd_cd.c \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/lib/util/astring.h \

$(OBJ)/emu/imagedev/flopdrv.o : \
	src/lib/formats/flopimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/imagedev/flopdrv.c \
	src/lib/formats/imageutl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/emu/imagedev/floppy.o : \
	src/lib/formats/flopimg.h \
	src/lib/util/unzip.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/lib/util/zippath.h \
	src/lib/formats/imageutl.h \
	src/emu/imagedev/floppy.c \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \
	src/lib/util/astring.h \

$(OBJ)/emu/imagedev/harddriv.o : \
	src/lib/util/palette.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/emu/imagedev/harddriv.c \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/emu/imagedev/midiin.o : \
	src/emu/imagedev/midiin.h \
	src/emu/imagedev/midiin.c \

$(OBJ)/emu/imagedev/midiout.o : \
	src/emu/imagedev/midiout.h \
	src/emu/imagedev/midiout.c \

$(OBJ)/emu/imagedev/printer.o : \
	src/emu/imagedev/printer.h \
	src/emu/imagedev/printer.c \

$(OBJ)/emu/imagedev/serial.o : \
	src/emu/imagedev/serial.h \
	src/emu/imagedev/serial.c \

$(OBJ)/emu/imagedev/snapquik.o : \
	src/emu/imagedev/snapquik.c \
	src/emu/imagedev/snapquik.h \

$(OBJ)/emu/machine/40105.o : \
	src/emu/machine/40105.h \
	src/emu/machine/40105.c \

$(OBJ)/emu/machine/53c7xx.o : \
	src/emu/machine/53c7xx.h \
	src/emu/machine/53c7xx.c \
	src/emu/machine/nscsi_bus.h \

$(OBJ)/emu/machine/53c810.o : \
	src/emu/machine/53c810.c \
	src/emu/machine/53c810.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/scsihle.h \

$(OBJ)/emu/machine/6522via.o : \
	src/emu/machine/6522via.c \
	src/emu/machine/6522via.h \

$(OBJ)/emu/machine/6525tpi.o : \
	src/emu/machine/6525tpi.c \
	src/emu/machine/6525tpi.h \

$(OBJ)/emu/machine/6526cia.o : \
	src/emu/machine/6526cia.c \
	src/emu/machine/6526cia.h \

$(OBJ)/emu/machine/6532riot.o : \
	src/emu/machine/6532riot.c \
	src/emu/machine/6532riot.h \

$(OBJ)/emu/machine/6821pia.o : \
	src/emu/machine/6821pia.h \
	src/emu/machine/6821pia.c \

$(OBJ)/emu/machine/68307.o : \
	src/emu/machine/68307sim.h \
	src/emu/machine/68307.c \
	src/emu/machine/68681.h \
	src/emu/machine/68307.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/68307ser.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/emu/machine/68307bus.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/68307tmu.h \

$(OBJ)/emu/machine/68307bus.o : \
	src/emu/machine/68307sim.h \
	src/emu/machine/68681.h \
	src/emu/machine/68307.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/68307ser.h \
	src/lib/softfloat/mamesf.h \
	src/emu/machine/68307bus.c \
	src/lib/softfloat/milieu.h \
	src/emu/machine/68307bus.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/68307tmu.h \

$(OBJ)/emu/machine/68307ser.o : \
	src/emu/machine/68307sim.h \
	src/emu/machine/68681.h \
	src/emu/machine/68307.h \
	src/emu/machine/68307ser.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/68307ser.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/emu/machine/68307bus.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/68307tmu.h \

$(OBJ)/emu/machine/68307sim.o : \
	src/emu/machine/68307sim.h \
	src/emu/machine/68681.h \
	src/emu/machine/68307.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/68307ser.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/emu/machine/68307bus.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/68307sim.c \
	src/emu/machine/68307tmu.h \

$(OBJ)/emu/machine/68307tmu.o : \
	src/emu/machine/68307sim.h \
	src/emu/machine/68681.h \
	src/emu/machine/68307tmu.c \
	src/emu/machine/68307.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/68307ser.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/emu/machine/68307bus.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/68307tmu.h \

$(OBJ)/emu/machine/68340.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/68340tmu.h \
	src/emu/machine/68340.c \
	src/emu/machine/68340dma.h \
	src/emu/machine/68340ser.h \
	src/emu/machine/68340sim.h \
	src/emu/machine/68340.h \

$(OBJ)/emu/machine/68340dma.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/68340tmu.h \
	src/emu/machine/68340dma.h \
	src/emu/machine/68340ser.h \
	src/emu/machine/68340sim.h \
	src/emu/machine/68340.h \
	src/emu/machine/68340dma.c \

$(OBJ)/emu/machine/68340ser.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/68340ser.c \
	src/emu/machine/68340tmu.h \
	src/emu/machine/68340dma.h \
	src/emu/machine/68340ser.h \
	src/emu/machine/68340sim.h \
	src/emu/machine/68340.h \

$(OBJ)/emu/machine/68340sim.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/68340tmu.h \
	src/emu/machine/68340sim.c \
	src/emu/machine/68340dma.h \
	src/emu/machine/68340ser.h \
	src/emu/machine/68340sim.h \
	src/emu/machine/68340.h \

$(OBJ)/emu/machine/68340tmu.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/68340tmu.h \
	src/emu/machine/68340dma.h \
	src/emu/machine/68340ser.h \
	src/emu/machine/68340sim.h \
	src/emu/machine/68340tmu.c \
	src/emu/machine/68340.h \

$(OBJ)/emu/machine/6840ptm.o : \
	src/emu/machine/6840ptm.h \
	src/emu/machine/6840ptm.c \

$(OBJ)/emu/machine/6850acia.o : \
	src/emu/machine/6850acia.h \
	src/emu/machine/6850acia.c \

$(OBJ)/emu/machine/68681.o : \
	src/emu/machine/68681.h \
	src/emu/machine/68681.c \

$(OBJ)/emu/machine/7200fifo.o : \
	src/emu/machine/7200fifo.c \
	src/emu/machine/7200fifo.h \

$(OBJ)/emu/machine/74123.o : \
	src/emu/machine/74123.c \
	src/emu/machine/rescap.h \
	src/emu/machine/74123.h \

$(OBJ)/emu/machine/74145.o : \
	src/lib/util/coreutil.h \
	src/emu/machine/74145.h \
	src/emu/machine/74145.c \

$(OBJ)/emu/machine/74148.o : \
	src/emu/machine/74148.c \
	src/emu/machine/74148.h \

$(OBJ)/emu/machine/74153.o : \
	src/emu/machine/74153.h \
	src/emu/machine/74153.c \

$(OBJ)/emu/machine/74181.o : \
	src/emu/machine/74181.c \
	src/emu/machine/74181.h \

$(OBJ)/emu/machine/7474.o : \
	src/emu/machine/7474.h \
	src/emu/machine/7474.c \

$(OBJ)/emu/machine/8042kbdc.o : \
	src/emu/machine/8042kbdc.h \
	src/emu/machine/8042kbdc.c \
	src/emu/machine/pckeybrd.h \

$(OBJ)/emu/machine/8257dma.o : \
	src/emu/machine/8257dma.h \
	src/emu/machine/8257dma.c \

$(OBJ)/emu/machine/aakart.o : \
	src/emu/machine/aakart.h \
	src/emu/machine/aakart.c \

$(OBJ)/emu/machine/adc0808.o : \
	src/emu/machine/adc0808.h \
	src/emu/machine/adc0808.c \

$(OBJ)/emu/machine/adc083x.o : \
	src/emu/machine/adc083x.h \
	src/emu/machine/adc083x.c \

$(OBJ)/emu/machine/adc1038.o : \
	src/emu/machine/adc1038.h \
	src/emu/machine/adc1038.c \

$(OBJ)/emu/machine/adc1213x.o : \
	src/emu/machine/adc1213x.h \
	src/emu/machine/adc1213x.c \

$(OBJ)/emu/machine/aicartc.o : \
	src/emu/machine/aicartc.h \
	src/emu/machine/aicartc.c \

$(OBJ)/emu/machine/am53cf96.o : \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/am53cf96.h \
	src/emu/machine/am53cf96.c \
	src/emu/machine/scsihle.h \

$(OBJ)/emu/machine/am9517a.o : \
	src/emu/machine/am9517a.c \
	src/emu/machine/am9517a.h \

$(OBJ)/emu/machine/amigafdc.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/formats/imd_dsk.h \
	src/lib/softfloat/mamesf.h \
	src/mame/includes/amiga.h \
	src/emu/machine/6526cia.h \
	src/emu/machine/amigafdc.h \
	src/lib/formats/ami_dsk.h \
	src/lib/softfloat/milieu.h \
	src/lib/softfloat/softfloat.h \
	src/emu/machine/amigafdc.c \
	src/lib/formats/td0_dsk.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/machine/at28c16.o : \
	src/emu/machine/at28c16.c \
	src/emu/machine/at28c16.h \

$(OBJ)/emu/machine/at29040a.o : \
	src/emu/machine/at29040a.h \
	src/emu/machine/at29040a.c \

$(OBJ)/emu/machine/at45dbxx.o : \
	src/emu/machine/at45dbxx.h \
	src/emu/machine/at45dbxx.c \

$(OBJ)/emu/machine/atadev.o : \
	src/emu/machine/atadev.h \
	src/emu/machine/atadev.c \

$(OBJ)/emu/machine/ataflash.o : \
	src/lib/util/palette.h \
	src/emu/machine/atahle.h \
	src/emu/machine/ataflash.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/machine/ataflash.c \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/emu/machine/pccard.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/idehd.h \
	src/lib/util/hashing.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/atahle.o : \
	src/emu/machine/atahle.h \
	src/emu/machine/atahle.c \
	src/emu/machine/atadev.h \

$(OBJ)/emu/machine/ataintf.o : \
	src/lib/util/palette.h \
	src/emu/machine/atahle.h \
	src/lib/util/cdrom.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/imagedev/chd_cd.h \
	src/emu/machine/atapicdr.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/emu/machine/ataintf.h \
	src/emu/machine/t10mmc.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/idehd.h \
	src/emu/sound/cdda.h \
	src/emu/machine/ataintf.c \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \
	src/emu/machine/atapihle.h \

$(OBJ)/emu/machine/atapicdr.o : \
	src/lib/util/palette.h \
	src/emu/machine/atahle.h \
	src/lib/util/cdrom.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/emu/machine/atapicdr.c \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/imagedev/chd_cd.h \
	src/emu/machine/atapicdr.h \
	src/lib/util/md5.h \
	src/emu/machine/t10mmc.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/lib/util/astring.h \
	src/emu/machine/atapihle.h \

$(OBJ)/emu/machine/atapihle.o : \
	src/emu/machine/atahle.h \
	src/emu/machine/atadev.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/atapihle.c \
	src/emu/machine/scsihle.h \
	src/emu/machine/atapihle.h \

$(OBJ)/emu/machine/ay31015.o : \
	src/emu/machine/ay31015.c \
	src/emu/machine/ay31015.h \

$(OBJ)/emu/machine/bankdev.o : \
	src/emu/machine/bankdev.h \
	src/emu/machine/bankdev.c \

$(OBJ)/emu/machine/cdp1852.o : \
	src/emu/machine/cdp1852.c \
	src/emu/machine/cdp1852.h \

$(OBJ)/emu/machine/cdp1871.o : \
	src/emu/machine/cdp1871.c \
	src/emu/machine/cdp1871.h \

$(OBJ)/emu/machine/cdu76s.o : \
	src/lib/util/palette.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/machine/cdu76s.h \
	src/emu/machine/scsibus.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/cdu76s.c \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/emu/machine/t10mmc.h \
	src/lib/util/corefile.h \
	src/emu/machine/scsicd.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/com8116.o : \
	src/emu/machine/com8116.c \
	src/emu/machine/com8116.h \

$(OBJ)/emu/machine/cr589.o : \
	src/lib/util/palette.h \
	src/emu/machine/atahle.h \
	src/lib/util/cdrom.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/atapicdr.h \
	src/emu/imagedev/chd_cd.h \
	src/emu/machine/cr589.h \
	src/lib/util/md5.h \
	src/emu/machine/cr589.c \
	src/emu/machine/t10mmc.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/lib/util/astring.h \
	src/emu/machine/atapihle.h \

$(OBJ)/emu/machine/ctronics.o : \
	src/emu/machine/ctronics.c \
	src/emu/machine/ctronics.h \
	src/emu/imagedev/printer.h \

$(OBJ)/emu/machine/ds1302.o : \
	src/emu/machine/ds1302.h \
	src/emu/machine/ds1302.c \

$(OBJ)/emu/machine/ds2401.o : \
	src/emu/machine/ds2401.h \
	src/emu/machine/ds2401.c \

$(OBJ)/emu/machine/ds2404.o : \
	src/emu/machine/ds2404.h \
	src/emu/machine/ds2404.c \

$(OBJ)/emu/machine/ds75160a.o : \
	src/emu/machine/ds75160a.h \
	src/emu/machine/ds75160a.c \

$(OBJ)/emu/machine/ds75161a.o : \
	src/emu/machine/ds75161a.h \
	src/emu/machine/ds75161a.c \

$(OBJ)/emu/machine/e0516.o : \
	src/emu/machine/e0516.h \
	src/emu/machine/e0516.c \

$(OBJ)/emu/machine/eeprom.o : \
	src/emu/machine/eeprom.h \
	src/emu/machine/eeprom.c \

$(OBJ)/emu/machine/eeprompar.o : \
	src/emu/machine/eeprompar.h \
	src/emu/machine/eeprompar.c \
	src/emu/machine/eeprom.h \

$(OBJ)/emu/machine/eepromser.o : \
	src/emu/machine/eepromser.h \
	src/emu/machine/eepromser.c \
	src/emu/machine/eeprom.h \

$(OBJ)/emu/machine/er2055.o : \
	src/emu/machine/er2055.c \
	src/emu/machine/er2055.h \

$(OBJ)/emu/machine/er59256.o : \
	src/emu/machine/er59256.c \
	src/emu/machine/er59256.h \

$(OBJ)/emu/machine/f3853.o : \
	src/emu/machine/f3853.c \
	src/emu/machine/f3853.h \

$(OBJ)/emu/machine/fdc_pll.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/fdc_pll.c \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/machine/generic.o : \
	src/emu/machine/generic.c \

$(OBJ)/emu/machine/i2cmem.o : \
	src/emu/machine/i2cmem.c \
	src/emu/machine/i2cmem.h \

$(OBJ)/emu/machine/i8155.o : \
	src/emu/machine/i8155.h \
	src/emu/machine/i8155.c \

$(OBJ)/emu/machine/i8212.o : \
	src/emu/machine/i8212.h \
	src/emu/machine/i8212.c \

$(OBJ)/emu/machine/i8214.o : \
	src/emu/machine/i8214.h \
	src/emu/machine/i8214.c \

$(OBJ)/emu/machine/i8243.o : \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/machine/i8243.h \
	src/emu/machine/i8243.c \

$(OBJ)/emu/machine/i8251.o : \
	src/emu/machine/i8251.h \
	src/emu/machine/i8251.c \

$(OBJ)/emu/machine/i8255.o : \
	src/emu/machine/i8255.h \
	src/emu/machine/i8255.c \

$(OBJ)/emu/machine/i8279.o : \
	src/emu/machine/i8279.c \
	src/emu/machine/i8279.h \

$(OBJ)/emu/machine/i8355.o : \
	src/emu/machine/i8355.c \
	src/emu/machine/i8355.h \

$(OBJ)/emu/machine/idectrl.o : \
	src/emu/machine/atadev.h \
	src/emu/machine/idectrl.h \
	src/emu/machine/idectrl.c \
	src/emu/machine/ataintf.h \

$(OBJ)/emu/machine/idehd.o : \
	src/lib/util/palette.h \
	src/emu/machine/atahle.h \
	src/emu/machine/atadev.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/idehd.h \
	src/emu/machine/idehd.c \
	src/lib/util/hashing.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/im6402.o : \
	src/emu/machine/im6402.h \
	src/emu/machine/im6402.c \

$(OBJ)/emu/machine/ins8154.o : \
	src/emu/machine/ins8154.c \
	src/emu/machine/ins8154.h \

$(OBJ)/emu/machine/ins8250.o : \
	src/emu/machine/ins8250.c \
	src/emu/machine/ins8250.h \

$(OBJ)/emu/machine/intelfsh.o : \
	src/emu/machine/intelfsh.h \
	src/emu/machine/intelfsh.c \

$(OBJ)/emu/machine/jvsdev.o : \
	src/emu/machine/jvsdev.c \
	src/emu/machine/jvsdev.h \
	src/emu/machine/jvshost.h \

$(OBJ)/emu/machine/jvshost.o : \
	src/emu/machine/jvsdev.h \
	src/emu/machine/jvshost.c \
	src/emu/machine/jvshost.h \

$(OBJ)/emu/machine/k033906.o : \
	src/emu/video/voodoo.h \
	src/emu/machine/k033906.h \
	src/emu/machine/k033906.c \

$(OBJ)/emu/machine/k053252.o : \
	src/emu/machine/k053252.h \
	src/emu/machine/k053252.c \

$(OBJ)/emu/machine/k056230.o : \
	src/emu/machine/k056230.h \
	src/emu/machine/k056230.c \

$(OBJ)/emu/machine/laserdsc.o : \
	src/lib/util/avhuff.h \
	src/lib/libflac/include/flac/all.h \
	src/lib/util/flac.h \
	src/lib/util/palette.h \
	src/emu/machine/laserdsc.c \
	src/lib/libflac/include/flac/ordinals.h \
	src/emu/machine/laserdsc.h \
	src/lib/util/sha1.h \
	src/lib/libflac/include/flac/metadata.h \
	src/lib/util/chd.h \
	src/lib/libflac/include/flac/export.h \
	src/lib/util/chdcodec.h \
	src/lib/util/vbiparse.h \
	src/lib/util/md5.h \
	src/lib/util/bitstream.h \
	src/lib/libflac/include/flac/format.h \
	src/lib/libflac/include/flac/stream_encoder.h \
	src/lib/util/huffman.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/libflac/include/flac/assert.h \
	src/lib/libflac/include/flac/stream_decoder.h \
	src/lib/util/hashing.h \
	src/lib/util/astring.h \
	src/lib/libflac/include/flac/callback.h \

$(OBJ)/emu/machine/latch8.o : \
	src/emu/machine/rescap.h \
	src/emu/sound/disc_dev.h \
	src/emu/machine/latch8.c \
	src/emu/machine/latch8.h \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/wavwrite.h \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/emu/sound/disc_wav.h \

$(OBJ)/emu/machine/lc89510.o : \
	src/emu/machine/lc89510.c \
	src/emu/machine/lc89510.h \

$(OBJ)/emu/machine/ldpr8210.o : \
	src/lib/util/avhuff.h \
	src/emu/machine/ldpr8210.h \
	src/lib/util/palette.h \
	src/lib/libflac/include/flac/all.h \
	src/lib/util/flac.h \
	src/emu/machine/ldpr8210.c \
	src/lib/libflac/include/flac/ordinals.h \
	src/emu/machine/laserdsc.h \
	src/lib/libflac/include/flac/metadata.h \
	src/lib/libflac/include/flac/export.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/vbiparse.h \
	src/lib/util/bitstream.h \
	src/lib/libflac/include/flac/format.h \
	src/lib/libflac/include/flac/stream_encoder.h \
	src/lib/util/huffman.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/libflac/include/flac/assert.h \
	src/lib/libflac/include/flac/stream_decoder.h \
	src/lib/util/astring.h \
	src/lib/libflac/include/flac/callback.h \

$(OBJ)/emu/machine/ldstub.o : \
	src/lib/util/avhuff.h \
	src/lib/util/palette.h \
	src/lib/libflac/include/flac/all.h \
	src/lib/util/flac.h \
	src/emu/machine/ldstub.c \
	src/lib/libflac/include/flac/ordinals.h \
	src/emu/machine/laserdsc.h \
	src/emu/machine/ldstub.h \
	src/lib/libflac/include/flac/metadata.h \
	src/lib/libflac/include/flac/export.h \
	src/lib/util/vbiparse.h \
	src/lib/util/bitstream.h \
	src/lib/libflac/include/flac/format.h \
	src/lib/libflac/include/flac/stream_encoder.h \
	src/lib/util/huffman.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/libflac/include/flac/assert.h \
	src/lib/libflac/include/flac/stream_decoder.h \
	src/lib/util/astring.h \
	src/lib/libflac/include/flac/callback.h \

$(OBJ)/emu/machine/ldv1000.o : \
	src/lib/util/avhuff.h \
	src/lib/util/palette.h \
	src/lib/libflac/include/flac/all.h \
	src/lib/util/flac.h \
	src/lib/libflac/include/flac/ordinals.h \
	src/emu/machine/ldv1000.c \
	src/emu/machine/laserdsc.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/ldv1000.h \
	src/lib/libflac/include/flac/metadata.h \
	src/lib/libflac/include/flac/export.h \
	src/emu/cpu/z80/z80.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/emu/machine/i8255.h \
	src/lib/util/vbiparse.h \
	src/emu/machine/z80ctc.h \
	src/lib/util/bitstream.h \
	src/lib/libflac/include/flac/format.h \
	src/lib/libflac/include/flac/stream_encoder.h \
	src/lib/util/huffman.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/libflac/include/flac/assert.h \
	src/lib/libflac/include/flac/stream_decoder.h \
	src/lib/util/astring.h \
	src/lib/libflac/include/flac/callback.h \

$(OBJ)/emu/machine/ldvp931.o : \
	src/lib/util/avhuff.h \
	src/lib/util/palette.h \
	src/lib/libflac/include/flac/all.h \
	src/lib/util/flac.h \
	src/lib/libflac/include/flac/ordinals.h \
	src/emu/machine/laserdsc.h \
	src/lib/libflac/include/flac/metadata.h \
	src/lib/libflac/include/flac/export.h \
	src/emu/cpu/mcs48/mcs48.h \
	src/lib/util/vbiparse.h \
	src/lib/util/bitstream.h \
	src/lib/libflac/include/flac/format.h \
	src/lib/libflac/include/flac/stream_encoder.h \
	src/lib/util/huffman.h \
	src/emu/machine/ldvp931.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/ldvp931.c \
	src/lib/libflac/include/flac/assert.h \
	src/lib/libflac/include/flac/stream_decoder.h \
	src/lib/util/astring.h \
	src/lib/libflac/include/flac/callback.h \

$(OBJ)/emu/machine/linflash.o : \
	src/emu/machine/pccard.h \
	src/emu/machine/intelfsh.h \
	src/emu/machine/linflash.c \
	src/emu/machine/linflash.h \

$(OBJ)/emu/machine/m6m80011ap.o : \
	src/emu/machine/m6m80011ap.c \
	src/emu/machine/m6m80011ap.h \

$(OBJ)/emu/machine/matsucd.o : \
	src/lib/util/palette.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/matsucd.c \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/machine/matsucd.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/mb14241.o : \
	src/emu/machine/mb14241.c \
	src/emu/machine/mb14241.h \

$(OBJ)/emu/machine/mb3773.o : \
	src/emu/machine/mb3773.c \
	src/emu/machine/mb3773.h \

$(OBJ)/emu/machine/mb87078.o : \
	src/emu/machine/mb87078.c \
	src/emu/machine/mb87078.h \

$(OBJ)/emu/machine/mb89371.o : \
	src/emu/machine/mb89371.c \
	src/emu/machine/mb89371.h \

$(OBJ)/emu/machine/mc146818.o : \
	src/emu/machine/mc146818.h \
	src/lib/util/coreutil.h \
	src/emu/machine/mc146818.c \

$(OBJ)/emu/machine/mc2661.o : \
	src/emu/machine/mc2661.c \
	src/emu/machine/mc2661.h \

$(OBJ)/emu/machine/mc6843.o : \
	src/lib/formats/flopimg.h \
	src/emu/machine/mc6843.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/mc6843.c \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/emu/machine/mc6846.o : \
	src/emu/machine/mc6846.h \
	src/emu/machine/mc6846.c \

$(OBJ)/emu/machine/mc6852.o : \
	src/emu/machine/mc6852.c \
	src/emu/machine/mc6852.h \

$(OBJ)/emu/machine/mc6854.o : \
	src/emu/machine/mc6854.h \
	src/emu/machine/mc6854.c \

$(OBJ)/emu/machine/mc68901.o : \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/emu/machine/mc68901.c \
	src/lib/softfloat/mamesf.h \
	src/lib/softfloat/milieu.h \
	src/emu/machine/mc68901.h \
	src/lib/softfloat/softfloat.h \

$(OBJ)/emu/machine/mccs1850.o : \
	src/emu/machine/mccs1850.c \
	src/emu/machine/mccs1850.h \

$(OBJ)/emu/machine/mcf5206e.o : \
	src/emu/machine/mcf5206e.c \
	src/emu/machine/mcf5206e.h \

$(OBJ)/emu/machine/microtch.o : \
	src/emu/machine/microtch.h \
	src/emu/machine/microtch.c \

$(OBJ)/emu/machine/mm58274c.o : \
	src/emu/machine/mm58274c.c \
	src/emu/machine/mm58274c.h \

$(OBJ)/emu/machine/mm74c922.o : \
	src/emu/machine/mm74c922.c \
	src/emu/machine/mm74c922.h \

$(OBJ)/emu/machine/mos6526.o : \
	src/emu/machine/mos6526.c \
	src/emu/machine/mos6526.h \

$(OBJ)/emu/machine/mos6529.o : \
	src/emu/machine/mos6529.c \
	src/emu/machine/mos6529.h \

$(OBJ)/emu/machine/mos6530.o : \
	src/emu/machine/mos6530.h \
	src/emu/machine/mos6530.c \

$(OBJ)/emu/machine/mos6551.o : \
	src/emu/machine/mos6551.h \
	src/emu/machine/mos6551.c \

$(OBJ)/emu/machine/mos8726.o : \
	src/emu/machine/mos8726.c \
	src/emu/machine/mos8726.h \

$(OBJ)/emu/machine/msm5832.o : \
	src/emu/machine/msm5832.c \
	src/emu/machine/msm5832.h \

$(OBJ)/emu/machine/msm58321.o : \
	src/emu/machine/msm58321.c \
	src/emu/machine/msm58321.h \

$(OBJ)/emu/machine/msm6242.o : \
	src/emu/machine/msm6242.c \
	src/emu/machine/msm6242.h \

$(OBJ)/emu/machine/n68681.o : \
	src/emu/machine/n68681.c \
	src/emu/machine/n68681.h \

$(OBJ)/emu/machine/ncr539x.o : \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/ncr539x.c \
	src/emu/machine/ncr539x.h \
	src/emu/machine/scsihle.h \

$(OBJ)/emu/machine/net_lib.o : \
	src/emu/machine/net_lib.h \
	src/emu/machine/net_lib.c \
	src/emu/machine/netlist.h \
	src/lib/util/tagmap.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/netlist.o : \
	src/emu/machine/net_lib.h \
	src/emu/machine/netlist.c \
	src/emu/machine/netlist.h \
	src/lib/util/tagmap.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/nmc9306.o : \
	src/emu/machine/nmc9306.c \
	src/emu/machine/nmc9306.h \

$(OBJ)/emu/machine/nscsi_bus.o : \
	src/emu/machine/nscsi_bus.c \
	src/emu/machine/nscsi_bus.h \

$(OBJ)/emu/machine/nscsi_cd.o : \
	src/emu/machine/nscsi_cd.c \
	src/lib/util/palette.h \
	src/emu/machine/nscsi_cd.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/emu/machine/nscsi_bus.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/nscsi_hd.o : \
	src/lib/util/palette.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/lib/util/md5.h \
	src/emu/machine/nscsi_hd.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/emu/machine/nscsi_hd.c \
	src/lib/util/harddisk.h \
	src/emu/machine/nscsi_bus.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/nvram.o : \
	src/emu/machine/nvram.h \
	src/emu/machine/nvram.c \

$(OBJ)/emu/machine/pccard.o : \
	src/emu/machine/pccard.h \
	src/emu/machine/pccard.c \

$(OBJ)/emu/machine/pcf8593.o : \
	src/emu/machine/pcf8593.c \
	src/emu/machine/pcf8593.h \

$(OBJ)/emu/machine/pci.o : \
	src/emu/machine/pci.c \
	src/emu/machine/pci.h \

$(OBJ)/emu/machine/pckeybrd.o : \
	src/emu/machine/pckeybrd.h \
	src/emu/machine/pckeybrd.c \

$(OBJ)/emu/machine/pd4990a.o : \
	src/emu/machine/pd4990a.h \
	src/emu/machine/pd4990a.c \

$(OBJ)/emu/machine/pic8259.o : \
	src/emu/machine/pic8259.c \
	src/emu/machine/pic8259.h \

$(OBJ)/emu/machine/pit8253.o : \
	src/emu/machine/pit8253.h \
	src/emu/machine/pit8253.c \

$(OBJ)/emu/machine/pla.o : \
	src/emu/machine/pla.c \
	src/emu/machine/pla.h \
	src/lib/util/jedparse.h \

$(OBJ)/emu/machine/ram.o : \
	src/emu/machine/ram.c \
	src/emu/machine/ram.h \

$(OBJ)/emu/machine/rf5c296.o : \
	src/emu/machine/rf5c296.c \
	src/emu/machine/rf5c296.h \
	src/emu/machine/pccard.h \

$(OBJ)/emu/machine/roc10937.o : \
	src/emu/machine/roc10937.h \
	src/emu/machine/roc10937.c \

$(OBJ)/emu/machine/rp5c01.o : \
	src/emu/machine/rp5c01.h \
	src/emu/machine/rp5c01.c \

$(OBJ)/emu/machine/rp5c15.o : \
	src/emu/machine/rp5c15.h \
	src/emu/machine/rp5c15.c \

$(OBJ)/emu/machine/rp5h01.o : \
	src/emu/machine/rp5h01.c \
	src/emu/machine/rp5h01.h \

$(OBJ)/emu/machine/rtc4543.o : \
	src/emu/machine/rtc4543.h \
	src/emu/machine/rtc4543.c \

$(OBJ)/emu/machine/rtc65271.o : \
	src/emu/machine/rtc65271.h \
	src/emu/machine/rtc65271.c \

$(OBJ)/emu/machine/rtc9701.o : \
	src/emu/machine/rtc9701.h \
	src/emu/machine/rtc9701.c \

$(OBJ)/emu/machine/s2636.o : \
	src/emu/machine/s2636.h \
	src/emu/machine/s2636.c \

$(OBJ)/emu/machine/s3520cf.o : \
	src/emu/machine/s3520cf.c \
	src/emu/machine/s3520cf.h \

$(OBJ)/emu/machine/s3c2400.o : \
	src/emu/cpu/arm7/arm7.h \
	src/emu/machine/s3c2400.h \
	src/emu/cpu/uml.h \
	src/lib/util/coreutil.h \
	src/emu/machine/s3c2400.c \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7core.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/machine/s3c24xx.c \
	src/emu/cpu/drcumlsh.h \
	src/emu/sound/dac.h \

$(OBJ)/emu/machine/s3c2410.o : \
	src/emu/machine/s3c2410.h \
	src/emu/machine/s3c2410.c \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/lib/util/coreutil.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7core.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/machine/s3c24xx.c \
	src/emu/cpu/drcumlsh.h \
	src/emu/sound/dac.h \

$(OBJ)/emu/machine/s3c2440.o : \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/lib/util/coreutil.h \
	src/emu/cpu/drccache.h \
	src/emu/machine/s3c2440.h \
	src/emu/cpu/arm7/arm7core.h \
	src/emu/machine/s3c2440.c \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/machine/s3c24xx.c \
	src/emu/cpu/drcumlsh.h \
	src/emu/sound/dac.h \

$(OBJ)/emu/machine/s3c24xx.o : \
	src/emu/cpu/arm7/arm7.h \
	src/emu/cpu/uml.h \
	src/lib/util/coreutil.h \
	src/emu/cpu/drccache.h \
	src/emu/cpu/arm7/arm7core.h \
	src/emu/cpu/drcuml.h \
	src/emu/cpu/drcfe.h \
	src/emu/machine/s3c24xx.c \
	src/emu/cpu/drcumlsh.h \

$(OBJ)/emu/machine/saturn.o : \
	src/mame/includes/stv.h \
	src/lib/util/palette.h \
	src/emu/machine/saturn.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/emu/cpu/scudsp/scudsp.h \
	src/lib/util/chd.h \
	src/emu/machine/eepromser.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/adsp2100/adsp2100.h \
	src/lib/util/chdcodec.h \
	src/lib/softfloat/milieu.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/lib/softfloat/softfloat.h \
	src/emu/cpu/sh2/sh2.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/scsibus.o : \
	src/emu/machine/scsibus.c \
	src/emu/machine/scsibus.h \
	src/emu/machine/scsidev.h \

$(OBJ)/emu/machine/scsicb.o : \
	src/emu/machine/scsibus.h \
	src/emu/machine/scsicb.c \
	src/emu/machine/scsidev.h \
	src/emu/machine/scsicb.h \

$(OBJ)/emu/machine/scsicd.o : \
	src/lib/util/palette.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/emu/machine/scsicd.c \
	src/emu/machine/t10mmc.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/scsicd.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/scsidev.o : \
	src/emu/machine/scsidev.c \
	src/emu/machine/scsibus.h \
	src/emu/machine/scsidev.h \

$(OBJ)/emu/machine/scsihd.o : \
	src/emu/machine/scsihd.c \
	src/lib/util/palette.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10sbc.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/lib/util/md5.h \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/machine/scsihd.h \
	src/lib/util/hashing.h \
	src/emu/machine/scsihle.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/scsihle.o : \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/scsihle.c \
	src/emu/machine/scsihle.h \

$(OBJ)/emu/machine/secflash.o : \
	src/emu/machine/secflash.c \
	src/emu/machine/secflash.h \

$(OBJ)/emu/machine/seibu_cop.o : \
	src/emu/machine/seibu_cop.c \
	src/emu/machine/seibu_cop.h \

$(OBJ)/emu/machine/serflash.o : \
	src/emu/machine/serflash.h \
	src/emu/machine/serflash.c \

$(OBJ)/emu/machine/smc91c9x.o : \
	src/emu/machine/smc91c9x.h \
	src/emu/machine/smc91c9x.c \

$(OBJ)/emu/machine/smpc.o : \
	src/mame/includes/stv.h \
	src/lib/util/palette.h \
	src/emu/machine/smpc.c \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/cdrom.h \
	src/lib/util/coreutil.h \
	src/lib/util/sha1.h \
	src/emu/cpu/scudsp/scudsp.h \
	src/emu/machine/smpc.h \
	src/lib/util/chd.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/adsp2100/adsp2100.h \
	src/emu/machine/eepromser.h \
	src/lib/util/chdcodec.h \
	src/lib/softfloat/milieu.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/lib/softfloat/softfloat.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/spchrom.o : \
	src/emu/machine/spchrom.c \
	src/emu/machine/spchrom.h \

$(OBJ)/emu/machine/stvcd.o : \
	src/mame/includes/stv.h \
	src/lib/util/palette.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/lib/util/coreutil.h \
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
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/machine/stvcd.c \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/t10mmc.o : \
	src/lib/util/palette.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/emu/imagedev/chd_cd.h \
	src/lib/util/md5.h \
	src/emu/machine/t10mmc.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/machine/t10mmc.c \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/t10sbc.o : \
	src/lib/util/palette.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/emu/machine/t10sbc.h \
	src/lib/util/chdcodec.h \
	src/emu/machine/t10spc.h \
	src/lib/util/md5.h \
	src/emu/machine/t10sbc.c \
	src/emu/imagedev/harddriv.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/lib/util/harddisk.h \
	src/lib/util/astring.h \

$(OBJ)/emu/machine/t10spc.o : \
	src/emu/machine/t10spc.h \
	src/emu/machine/t10spc.c \

$(OBJ)/emu/machine/tc009xlvc.o : \
	src/emu/machine/tc009xlvc.c \
	src/emu/machine/tc009xlvc.h \

$(OBJ)/emu/machine/timekpr.o : \
	src/emu/machine/timekpr.c \
	src/emu/machine/timekpr.h \

$(OBJ)/emu/machine/tmp68301.o : \
	src/emu/machine/tmp68301.h \
	src/emu/machine/tmp68301.c \

$(OBJ)/emu/machine/tms6100.o : \
	src/emu/machine/tms6100.h \
	src/emu/machine/tms6100.c \

$(OBJ)/emu/machine/tms9901.o : \
	src/emu/machine/tms9901.c \
	src/emu/machine/tms9901.h \

$(OBJ)/emu/machine/tms9902.o : \
	src/emu/machine/tms9902.h \
	src/emu/machine/tms9902.c \

$(OBJ)/emu/machine/upd1990a.o : \
	src/emu/machine/upd1990a.c \
	src/emu/machine/upd1990a.h \

$(OBJ)/emu/machine/upd4701.o : \
	src/emu/machine/upd4701.h \
	src/emu/machine/upd4701.c \

$(OBJ)/emu/machine/upd7002.o : \
	src/emu/machine/upd7002.h \
	src/emu/machine/upd7002.c \

$(OBJ)/emu/machine/upd765.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/lib/formats/imd_dsk.h \
	src/emu/machine/upd765.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/emu/machine/upd765.c \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/machine/v3021.o : \
	src/emu/machine/v3021.h \
	src/emu/machine/v3021.c \

$(OBJ)/emu/machine/vt83c461.o : \
	src/emu/machine/vt83c461.h \
	src/emu/machine/atadev.h \
	src/emu/machine/idectrl.h \
	src/emu/machine/ataintf.h \
	src/emu/machine/vt83c461.c \

$(OBJ)/emu/machine/wd11c00_17.o : \
	src/emu/machine/wd11c00_17.h \
	src/emu/machine/wd11c00_17.c \

$(OBJ)/emu/machine/wd17xx.o : \
	src/lib/formats/flopimg.h \
	src/emu/imagedev/flopdrv.h \
	src/emu/machine/wd17xx.h \
	src/emu/machine/wd17xx.c \
	src/lib/formats/imageutl.h \
	src/lib/util/opresolv.h \
	src/lib/formats/ioprocs.h \

$(OBJ)/emu/machine/wd2010.o : \
	src/emu/machine/wd2010.h \
	src/emu/machine/wd2010.c \

$(OBJ)/emu/machine/wd33c93.o : \
	src/emu/machine/scsibus.h \
	src/emu/machine/t10spc.h \
	src/emu/machine/scsidev.h \
	src/emu/machine/wd33c93.c \
	src/emu/machine/wd33c93.h \
	src/emu/machine/scsihle.h \

$(OBJ)/emu/machine/wd_fdc.o : \
	src/lib/formats/flopimg.h \
	src/lib/formats/dfi_dsk.h \
	src/lib/formats/hxcmfm_dsk.h \
	src/emu/machine/wd_fdc.c \
	src/emu/machine/wd_fdc.h \
	src/lib/formats/imd_dsk.h \
	src/lib/formats/td0_dsk.h \
	src/emu/machine/fdc_pll.h \
	src/lib/formats/mfi_dsk.h \
	src/lib/util/opresolv.h \
	src/emu/imagedev/floppy.h \
	src/lib/formats/ipf_dsk.h \
	src/lib/formats/ioprocs.h \
	src/lib/formats/d88_dsk.h \

$(OBJ)/emu/machine/x2212.o : \
	src/emu/machine/x2212.c \
	src/emu/machine/x2212.h \

$(OBJ)/emu/machine/x76f041.o : \
	src/emu/machine/x76f041.h \
	src/emu/machine/secflash.h \
	src/emu/machine/x76f041.c \

$(OBJ)/emu/machine/x76f100.o : \
	src/emu/machine/x76f100.c \
	src/emu/machine/x76f100.h \
	src/emu/machine/secflash.h \

$(OBJ)/emu/machine/z80ctc.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80ctc.h \
	src/emu/machine/z80ctc.c \

$(OBJ)/emu/machine/z80dart.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/z80dart.h \
	src/emu/machine/z80dart.c \

$(OBJ)/emu/machine/z80dma.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/z80dma.c \
	src/emu/machine/z80dma.h \

$(OBJ)/emu/machine/z80pio.o : \
	src/emu/machine/z80pio.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/z80pio.c \

$(OBJ)/emu/machine/z80sio.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/z80sio.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80sio.c \

$(OBJ)/emu/machine/z80sti.o : \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/cpu/z80/z80.h \
	src/emu/machine/z80sti.c \
	src/emu/machine/z80sti.h \

$(OBJ)/emu/machine/z8536.o : \
	src/emu/machine/z8536.h \
	src/emu/cpu/z80/z80daisy.h \
	src/emu/machine/z8536.c \

$(OBJ)/emu/sound/2151intf.o : \
	src/emu/sound/ym2151.h \
	src/emu/sound/2151intf.h \
	src/emu/sound/2151intf.c \
	src/emu/sound/fm.h \

$(OBJ)/emu/sound/2203intf.o : \
	src/emu/sound/2203intf.c \
	src/emu/sound/fm.h \
	src/emu/sound/2203intf.h \
	src/emu/sound/ay8910.h \

$(OBJ)/emu/sound/2413intf.o : \
	src/emu/sound/2413intf.h \
	src/emu/sound/ym2413.h \
	src/emu/sound/2413intf.c \

$(OBJ)/emu/sound/2608intf.o : \
	src/emu/sound/fm.h \
	src/emu/sound/2608intf.c \
	src/emu/sound/2608intf.h \
	src/emu/sound/ay8910.h \

$(OBJ)/emu/sound/2610intf.o : \
	src/emu/sound/fm.h \
	src/emu/sound/2610intf.h \
	src/emu/sound/ay8910.h \
	src/emu/sound/2610intf.c \

$(OBJ)/emu/sound/2612intf.o : \
	src/emu/sound/2612intf.h \
	src/emu/sound/2612intf.c \
	src/emu/sound/fm.h \

$(OBJ)/emu/sound/262intf.o : \
	src/emu/sound/262intf.h \
	src/emu/sound/262intf.c \
	src/emu/sound/ymf262.h \

$(OBJ)/emu/sound/3526intf.o : \
	src/emu/sound/fmopl.h \
	src/emu/sound/fm.h \
	src/emu/sound/3526intf.h \
	src/emu/sound/3526intf.c \

$(OBJ)/emu/sound/3812intf.o : \
	src/emu/sound/fmopl.h \
	src/emu/sound/3812intf.c \
	src/emu/sound/fm.h \
	src/emu/sound/3812intf.h \

$(OBJ)/emu/sound/8950intf.o : \
	src/emu/sound/fmopl.h \
	src/emu/sound/8950intf.h \
	src/emu/sound/fm.h \
	src/emu/sound/8950intf.c \

$(OBJ)/emu/sound/aica.o : \
	src/emu/sound/aica.h \
	src/emu/sound/aicalfo.c \
	src/emu/sound/aicadsp.h \
	src/emu/sound/aica.c \

$(OBJ)/emu/sound/aicadsp.o : \
	src/emu/sound/aica.h \
	src/emu/sound/aicadsp.c \
	src/emu/sound/aicadsp.h \

$(OBJ)/emu/sound/aicalfo.o : \
	src/emu/sound/aicalfo.c \

$(OBJ)/emu/sound/asc.o : \
	src/emu/sound/asc.h \
	src/emu/sound/asc.c \

$(OBJ)/emu/sound/astrocde.o : \
	src/emu/sound/astrocde.h \
	src/emu/sound/astrocde.c \

$(OBJ)/emu/sound/awacs.o : \
	src/emu/sound/awacs.h \
	src/emu/sound/awacs.c \

$(OBJ)/emu/sound/ay8910.o : \
	src/emu/sound/ay8910.c \
	src/emu/sound/ay8910.h \

$(OBJ)/emu/sound/beep.o : \
	src/emu/sound/beep.c \
	src/emu/sound/beep.h \

$(OBJ)/emu/sound/bsmt2000.o : \
	src/emu/sound/bsmt2000.c \
	src/emu/sound/bsmt2000.h \
	src/emu/cpu/tms32010/tms32010.h \

$(OBJ)/emu/sound/c140.o : \
	src/emu/sound/c140.h \
	src/emu/sound/c140.c \

$(OBJ)/emu/sound/c352.o : \
	src/emu/sound/c352.h \
	src/emu/sound/c352.c \

$(OBJ)/emu/sound/c6280.o : \
	src/emu/cpu/h6280/h6280.h \
	src/emu/cpu/h6280/tblh6280.inc \
	src/emu/cpu/h6280/h6280ops.h \
	src/emu/sound/c6280.c \
	src/emu/sound/c6280.h \

$(OBJ)/emu/sound/cdda.o : \
	src/lib/util/palette.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/lib/util/chd.h \
	src/lib/util/chdcodec.h \
	src/lib/util/md5.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/emu/sound/cdda.h \
	src/lib/util/hashing.h \
	src/emu/sound/cdda.c \
	src/lib/util/astring.h \

$(OBJ)/emu/sound/cdp1863.o : \
	src/emu/sound/cdp1863.c \
	src/emu/sound/cdp1863.h \

$(OBJ)/emu/sound/cdp1864.o : \
	src/emu/video/resnet.h \
	src/emu/machine/rescap.h \
	src/emu/sound/cdp1864.c \
	src/emu/sound/cdp1864.h \

$(OBJ)/emu/sound/cdp1869.o : \
	src/emu/sound/cdp1869.h \
	src/emu/sound/cdp1869.c \

$(OBJ)/emu/sound/cem3394.o : \
	src/emu/sound/cem3394.h \
	src/emu/sound/cem3394.c \

$(OBJ)/emu/sound/dac.o : \
	src/emu/sound/dac.c \
	src/emu/sound/dac.h \

$(OBJ)/emu/sound/digitalk.o : \
	src/emu/sound/digitalk.h \
	src/emu/sound/digitalk.c \

$(OBJ)/emu/sound/disc_dev.o : \
	src/emu/sound/disc_dev.c \

$(OBJ)/emu/sound/disc_flt.o : \
	src/emu/sound/disc_flt.c \

$(OBJ)/emu/sound/disc_inp.o : \
	src/emu/sound/disc_inp.c \

$(OBJ)/emu/sound/disc_mth.o : \
	src/emu/sound/disc_mth.c \

$(OBJ)/emu/sound/disc_sys.o : \
	src/emu/sound/disc_sys.c \

$(OBJ)/emu/sound/disc_wav.o : \
	src/emu/sound/disc_wav.c \

$(OBJ)/emu/sound/discrete.o : \
	src/emu/sound/disc_sys.c \
	src/emu/sound/disc_wav.c \
	src/emu/machine/rescap.h \
	src/emu/sound/disc_dev.h \
	src/emu/sound/discrete.c \
	src/emu/sound/disc_dev.c \
	src/emu/sound/discrete.h \
	src/emu/sound/disc_mth.h \
	src/emu/sound/disc_flt.c \
	src/emu/sound/wavwrite.h \
	src/emu/sound/disc_mth.c \
	src/emu/sound/disc_flt.h \
	src/emu/sound/disc_cls.h \
	src/emu/sound/disc_inp.c \
	src/emu/sound/disc_wav.h \

$(OBJ)/emu/sound/dmadac.o : \
	src/emu/sound/dmadac.c \
	src/emu/sound/dmadac.h \

$(OBJ)/emu/sound/es5503.o : \
	src/emu/sound/es5503.h \
	src/emu/sound/es5503.c \

$(OBJ)/emu/sound/es5506.o : \
	src/emu/sound/wavwrite.h \
	src/emu/sound/es5506.h \
	src/emu/sound/es5506.c \

$(OBJ)/emu/sound/es8712.o : \
	src/emu/sound/es8712.h \
	src/emu/sound/es8712.c \

$(OBJ)/emu/sound/esqpump.o : \
	src/emu/sound/esqpump.h \
	src/emu/cpu/es5510/es5510.h \
	src/emu/sound/es5506.h \
	src/emu/sound/esqpump.c \

$(OBJ)/emu/sound/filter.o : \
	src/emu/sound/filter.h \
	src/emu/sound/filter.c \

$(OBJ)/emu/sound/flt_rc.o : \
	src/emu/machine/rescap.h \
	src/emu/sound/flt_rc.h \
	src/emu/sound/flt_rc.c \

$(OBJ)/emu/sound/flt_vol.o : \
	src/emu/sound/flt_vol.h \
	src/emu/sound/flt_vol.c \

$(OBJ)/emu/sound/fm.o : \
	src/emu/sound/fm.c \
	src/emu/sound/fm.h \
	src/emu/sound/ymdeltat.h \

$(OBJ)/emu/sound/fm2612.o : \
	src/emu/sound/fm.h \
	src/emu/sound/fm2612.c \

$(OBJ)/emu/sound/fmopl.o : \
	src/emu/sound/fmopl.c \
	src/emu/sound/fmopl.h \
	src/emu/sound/ymdeltat.h \

$(OBJ)/emu/sound/gaelco.o : \
	src/emu/sound/gaelco.h \
	src/emu/sound/gaelco.c \
	src/emu/sound/wavwrite.h \

$(OBJ)/emu/sound/hc55516.o : \
	src/emu/sound/hc55516.c \
	src/emu/sound/hc55516.h \

$(OBJ)/emu/sound/i5000.o : \
	src/emu/sound/okiadpcm.h \
	src/emu/sound/i5000.c \
	src/emu/sound/i5000.h \

$(OBJ)/emu/sound/ics2115.o : \
	src/emu/sound/ics2115.c \
	src/emu/sound/ics2115.h \

$(OBJ)/emu/sound/iremga20.o : \
	src/emu/sound/iremga20.h \
	src/emu/sound/iremga20.c \

$(OBJ)/emu/sound/k005289.o : \
	src/emu/sound/k005289.c \
	src/emu/sound/k005289.h \

$(OBJ)/emu/sound/k007232.o : \
	src/emu/sound/k007232.c \
	src/emu/sound/k007232.h \

$(OBJ)/emu/sound/k051649.o : \
	src/emu/sound/k051649.h \
	src/emu/sound/k051649.c \

$(OBJ)/emu/sound/k053260.o : \
	src/emu/sound/k053260.c \
	src/emu/sound/k053260.h \

$(OBJ)/emu/sound/k054539.o : \
	src/emu/sound/k054539.c \
	src/emu/sound/k054539.h \

$(OBJ)/emu/sound/k056800.o : \
	src/emu/sound/k056800.c \
	src/emu/sound/k056800.h \

$(OBJ)/emu/sound/lmc1992.o : \
	src/emu/sound/lmc1992.h \
	src/emu/sound/lmc1992.c \

$(OBJ)/emu/sound/mas3507d.o : \
	src/emu/sound/mas3507d.h \
	src/emu/sound/mas3507d.c \

$(OBJ)/emu/sound/mos6560.o : \
	src/emu/sound/mos6560.h \
	src/emu/sound/mos6560.c \

$(OBJ)/emu/sound/mos6581.o : \
	src/emu/sound/sidvoice.h \
	src/emu/sound/mos6581.c \
	src/emu/sound/mos6581.h \
	src/emu/sound/sid.h \

$(OBJ)/emu/sound/mos7360.o : \
	src/emu/sound/mos7360.h \
	src/emu/sound/mos7360.c \

$(OBJ)/emu/sound/mpeg_audio.o : \
	src/emu/sound/mpeg_audio.h \
	src/emu/sound/mpeg_audio.c \

$(OBJ)/emu/sound/msm5205.o : \
	src/emu/sound/msm5205.h \
	src/emu/sound/msm5205.c \

$(OBJ)/emu/sound/msm5232.o : \
	src/emu/sound/msm5232.h \
	src/emu/sound/msm5232.c \

$(OBJ)/emu/sound/multipcm.o : \
	src/emu/sound/multipcm.h \
	src/emu/sound/multipcm.c \

$(OBJ)/emu/sound/n63701x.o : \
	src/emu/sound/n63701x.h \
	src/emu/sound/n63701x.c \

$(OBJ)/emu/sound/namco.o : \
	src/emu/sound/namco.c \
	src/emu/sound/namco.h \

$(OBJ)/emu/sound/nes_apu.o : \
	src/emu/cpu/m6502/n2a03.h \
	src/emu/sound/nes_apu.h \
	src/emu/sound/nes_apu.c \
	src/emu/sound/nes_defs.h \
	src/emu/cpu/m6502/m6502.h \

$(OBJ)/emu/sound/nile.o : \
	src/emu/sound/nile.c \
	src/emu/sound/nile.h \

$(OBJ)/emu/sound/okiadpcm.o : \
	src/emu/sound/okiadpcm.h \
	src/emu/sound/okiadpcm.c \

$(OBJ)/emu/sound/okim6258.o : \
	src/emu/sound/okim6258.c \
	src/emu/sound/okim6258.h \

$(OBJ)/emu/sound/okim6295.o : \
	src/emu/sound/okim6295.c \
	src/emu/sound/okim6295.h \
	src/emu/sound/okiadpcm.h \

$(OBJ)/emu/sound/okim6376.o : \
	src/emu/sound/okim6376.c \
	src/emu/sound/okim6376.h \

$(OBJ)/emu/sound/okim9810.o : \
	src/emu/sound/okiadpcm.h \
	src/emu/sound/okim9810.h \
	src/emu/sound/okim9810.c \

$(OBJ)/emu/sound/pokey.o : \
	src/emu/machine/rescap.h \
	src/emu/sound/pokey.c \
	src/emu/sound/pokey.h \

$(OBJ)/emu/sound/qs1000.o : \
	src/emu/sound/qs1000.h \
	src/emu/sound/qs1000.c \
	src/emu/sound/okiadpcm.h \
	src/emu/cpu/mcs51/mcs51.h \

$(OBJ)/emu/sound/qsound.o : \
	src/emu/sound/qsound.h \
	src/emu/cpu/dsp16/dsp16.h \
	src/emu/sound/qsound.c \

$(OBJ)/emu/sound/rf5c400.o : \
	src/emu/sound/rf5c400.h \
	src/emu/sound/rf5c400.c \

$(OBJ)/emu/sound/rf5c68.o : \
	src/emu/sound/rf5c68.c \
	src/emu/sound/rf5c68.h \

$(OBJ)/emu/sound/s14001a.o : \
	src/emu/sound/s14001a.h \
	src/emu/sound/s14001a.c \

$(OBJ)/emu/sound/saa1099.o : \
	src/emu/sound/saa1099.h \
	src/emu/sound/saa1099.c \

$(OBJ)/emu/sound/samples.o : \
	src/lib/libflac/include/flac/all.h \
	src/lib/util/flac.h \
	src/lib/libflac/include/flac/ordinals.h \
	src/lib/libflac/include/flac/metadata.h \
	src/lib/libflac/include/flac/export.h \
	src/emu/sound/samples.h \
	src/lib/libflac/include/flac/format.h \
	src/lib/libflac/include/flac/stream_encoder.h \
	src/lib/util/corefile.h \
	src/lib/util/coretmpl.h \
	src/emu/sound/samples.c \
	src/lib/libflac/include/flac/assert.h \
	src/lib/libflac/include/flac/stream_decoder.h \
	src/lib/util/astring.h \
	src/lib/libflac/include/flac/callback.h \

$(OBJ)/emu/sound/scsp.o : \
	src/emu/sound/scsplfo.c \
	src/emu/sound/scsp.h \
	src/emu/sound/scsp.c \
	src/emu/sound/scspdsp.h \

$(OBJ)/emu/sound/scspdsp.o : \
	src/emu/sound/scspdsp.c \
	src/emu/sound/scsp.h \
	src/emu/sound/scspdsp.h \

$(OBJ)/emu/sound/scsplfo.o : \
	src/emu/sound/scsplfo.c \

$(OBJ)/emu/sound/segapcm.o : \
	src/emu/sound/segapcm.c \
	src/emu/sound/segapcm.h \

$(OBJ)/emu/sound/sid.o : \
	src/emu/sound/sidenvel.h \
	src/emu/sound/sidvoice.h \
	src/emu/sound/mos6581.h \
	src/emu/sound/sid.c \
	src/emu/sound/sid.h \

$(OBJ)/emu/sound/sidenvel.o : \
	src/emu/sound/sidenvel.h \
	src/emu/sound/side6581.h \
	src/emu/sound/sidvoice.h \
	src/emu/sound/mos6581.h \
	src/emu/sound/sidenvel.c \
	src/emu/sound/sid.h \

$(OBJ)/emu/sound/sidvoice.o : \
	src/emu/sound/sidenvel.h \
	src/emu/sound/sidw6581.h \
	src/emu/sound/sidvoice.c \
	src/emu/sound/sidw8580.h \
	src/emu/sound/sidvoice.h \
	src/emu/sound/mos6581.h \
	src/emu/sound/sid.h \

$(OBJ)/emu/sound/sn76477.o : \
	src/emu/machine/rescap.h \
	src/emu/sound/wavwrite.h \
	src/emu/sound/sn76477.c \
	src/emu/sound/sn76477.h \

$(OBJ)/emu/sound/sn76496.o : \
	src/emu/sound/sn76496.h \
	src/emu/sound/sn76496.c \

$(OBJ)/emu/sound/snkwave.o : \
	src/emu/sound/snkwave.h \
	src/emu/sound/snkwave.c \

$(OBJ)/emu/sound/sp0250.o : \
	src/emu/sound/sp0250.c \
	src/emu/sound/sp0250.h \

$(OBJ)/emu/sound/sp0256.o : \
	src/emu/sound/sp0256.h \
	src/emu/sound/sp0256.c \

$(OBJ)/emu/sound/speaker.o : \
	src/emu/sound/speaker.c \
	src/emu/sound/speaker.h \

$(OBJ)/emu/sound/spu.o : \
	src/emu/sound/spu.h \
	src/emu/sound/spu.c \
	src/emu/sound/spureverb.h \

$(OBJ)/emu/sound/spu_tables.o : \
	src/emu/sound/spu.h \
	src/emu/sound/spu_tables.c \
	src/emu/sound/spureverb.h \

$(OBJ)/emu/sound/spureverb.o : \
	src/emu/sound/spureverb.c \
	src/emu/sound/spureverb.h \

$(OBJ)/emu/sound/st0016.o : \
	src/emu/sound/st0016.h \
	src/emu/sound/st0016.c \

$(OBJ)/emu/sound/t6721a.o : \
	src/emu/sound/t6721a.c \
	src/emu/sound/t6721a.h \

$(OBJ)/emu/sound/t6w28.o : \
	src/emu/sound/t6w28.h \
	src/emu/sound/t6w28.c \

$(OBJ)/emu/sound/tc8830f.o : \
	src/emu/sound/tc8830f.h \
	src/emu/sound/tc8830f.c \

$(OBJ)/emu/sound/tiaintf.o : \
	src/emu/sound/tiaintf.c \
	src/emu/sound/tiaintf.h \
	src/emu/sound/tiasound.h \

$(OBJ)/emu/sound/tiasound.o : \
	src/emu/sound/tiaintf.h \
	src/emu/sound/tiasound.c \
	src/emu/sound/tiasound.h \

$(OBJ)/emu/sound/tms3615.o : \
	src/emu/sound/tms3615.c \
	src/emu/sound/tms3615.h \

$(OBJ)/emu/sound/tms36xx.o : \
	src/emu/sound/tms36xx.c \
	src/emu/sound/tms36xx.h \

$(OBJ)/emu/sound/tms5110.o : \
	src/emu/sound/tms5110.c \
	src/emu/sound/tms5110.h \
	src/emu/sound/tms5110r.c \

$(OBJ)/emu/sound/tms5110r.o : \
	src/emu/sound/tms5110r.c \

$(OBJ)/emu/sound/tms5220.o : \
	src/emu/sound/tms5220.h \
	src/emu/machine/spchrom.h \
	src/emu/sound/tms5220.c \
	src/emu/sound/tms5110r.c \

$(OBJ)/emu/sound/upd7752.o : \
	src/emu/sound/upd7752.h \
	src/emu/sound/upd7752.c \

$(OBJ)/emu/sound/upd7759.o : \
	src/emu/sound/upd7759.c \
	src/emu/sound/upd7759.h \

$(OBJ)/emu/sound/vlm5030.o : \
	src/emu/sound/vlm5030.h \
	src/emu/sound/vlm5030.c \
	src/emu/sound/tms5110r.c \

$(OBJ)/emu/sound/votrax.o : \
	src/emu/sound/votrax.h \
	src/emu/sound/votrax.c \
	src/emu/sound/samples.h \

$(OBJ)/emu/sound/vrender0.o : \
	src/emu/sound/vrender0.c \
	src/emu/sound/vrender0.h \

$(OBJ)/emu/sound/wave.o : \
	src/emu/imagedev/cassette.h \
	src/lib/formats/cassimg.h \
	src/emu/sound/wave.h \
	src/lib/util/pool.h \
	src/lib/formats/ioprocs.h \
	src/emu/sound/wave.c \

$(OBJ)/emu/sound/wavwrite.o : \
	src/emu/sound/wavwrite.c \
	src/emu/sound/wavwrite.h \

$(OBJ)/emu/sound/x1_010.o : \
	src/emu/sound/x1_010.h \
	src/emu/sound/x1_010.c \

$(OBJ)/emu/sound/ym2151.o : \
	src/emu/sound/ym2151.h \
	src/emu/sound/ym2151.c \

$(OBJ)/emu/sound/ym2413.o : \
	src/emu/sound/ym2413.h \
	src/emu/sound/ym2413.c \

$(OBJ)/emu/sound/ymdeltat.o : \
	src/emu/sound/ymdeltat.c \
	src/emu/sound/ymdeltat.h \

$(OBJ)/emu/sound/ymf262.o : \
	src/emu/sound/ymf262.h \
	src/emu/sound/ymf262.c \

$(OBJ)/emu/sound/ymf271.o : \
	src/emu/sound/ymf271.h \
	src/emu/sound/ymf271.c \

$(OBJ)/emu/sound/ymf278b.o : \
	src/emu/sound/ymf278b.c \
	src/emu/sound/ymf278b.h \

$(OBJ)/emu/sound/ymz280b.o : \
	src/emu/sound/ymz280b.h \
	src/emu/sound/ymz280b.c \
	src/emu/sound/wavwrite.h \

$(OBJ)/emu/sound/ymz770.o : \
	src/emu/sound/ymz770.h \
	src/emu/sound/mpeg_audio.h \
	src/emu/sound/ymz770.c \

$(OBJ)/emu/sound/zsg2.o : \
	src/emu/sound/zsg2.c \
	src/emu/sound/zsg2.h \

$(OBJ)/emu/video/315_5124.o : \
	src/emu/video/315_5124.c \
	src/emu/video/315_5124.h \

$(OBJ)/emu/video/bufsprite.o : \
	src/emu/video/bufsprite.h \
	src/emu/video/bufsprite.c \

$(OBJ)/emu/video/cdp1861.o : \
	src/emu/video/cdp1861.c \
	src/emu/video/cdp1861.h \

$(OBJ)/emu/video/cdp1862.o : \
	src/emu/video/cdp1862.c \
	src/emu/video/cdp1862.h \

$(OBJ)/emu/video/cgapal.o : \
	src/emu/video/cgapal.c \
	src/emu/video/cgapal.h \
	src/emu/video/pc_cga.h \

$(OBJ)/emu/video/crt9007.o : \
	src/emu/video/crt9007.c \
	src/emu/video/crt9007.h \

$(OBJ)/emu/video/crt9021.o : \
	src/emu/video/crt9021.h \
	src/emu/video/crt9021.c \

$(OBJ)/emu/video/crt9212.o : \
	src/emu/video/crt9212.c \
	src/emu/video/crt9212.h \

$(OBJ)/emu/video/dl1416.o : \
	src/emu/video/dl1416.c \
	src/emu/video/dl1416.h \

$(OBJ)/emu/video/dm9368.o : \
	src/emu/video/dm9368.c \
	src/emu/video/dm9368.h \

$(OBJ)/emu/video/ef9340_1.o : \
	src/emu/video/ef9341_chargen.h \
	src/emu/video/ef9340_1.c \
	src/emu/video/ef9340_1.h \

$(OBJ)/emu/video/fixfreq.o : \
	src/emu/video/fixfreq.c \
	src/emu/video/fixfreq.h \

$(OBJ)/emu/video/generic.o : \
	src/emu/video/generic.c \

$(OBJ)/emu/video/h63484.o : \
	src/emu/video/h63484.h \
	src/emu/video/h63484.c \

$(OBJ)/emu/video/hd44102.o : \
	src/emu/video/hd44102.c \
	src/emu/video/hd44102.h \

$(OBJ)/emu/video/hd44352.o : \
	src/emu/video/hd44352.c \
	src/emu/video/hd44352.h \

$(OBJ)/emu/video/hd44780.o : \
	src/emu/video/hd44780.c \
	src/emu/video/hd44780.h \

$(OBJ)/emu/video/hd61830.o : \
	src/emu/video/hd61830.h \
	src/emu/video/hd61830.c \

$(OBJ)/emu/video/hd63484.o : \
	src/emu/video/hd63484.h \
	src/emu/video/hd63484.c \

$(OBJ)/emu/video/hd66421.o : \
	src/emu/video/hd66421.c \
	src/emu/video/hd66421.h \

$(OBJ)/emu/video/huc6202.o : \
	src/emu/video/huc6202.c \
	src/emu/video/huc6270.h \
	src/emu/video/huc6202.h \

$(OBJ)/emu/video/huc6260.o : \
	src/emu/video/huc6260.c \
	src/emu/video/huc6260.h \

$(OBJ)/emu/video/huc6261.o : \
	src/emu/video/huc6261.c \
	src/emu/video/huc6270.h \
	src/emu/video/huc6261.h \

$(OBJ)/emu/video/huc6270.o : \
	src/emu/video/huc6270.c \
	src/emu/video/huc6270.h \

$(OBJ)/emu/video/huc6272.o : \
	src/emu/video/huc6272.h \
	src/emu/video/huc6272.c \

$(OBJ)/emu/video/i8244.o : \
	src/emu/video/i8244.h \
	src/emu/video/i8244.c \

$(OBJ)/emu/video/i8275.o : \
	src/emu/video/i8275.h \
	src/emu/video/i8275.c \

$(OBJ)/emu/video/i8275x.o : \
	src/emu/video/i8275x.c \
	src/emu/video/i8275x.h \

$(OBJ)/emu/video/k053250.o : \
	src/emu/video/k053250.h \
	src/emu/video/k053250.c \

$(OBJ)/emu/video/m50458.o : \
	src/emu/video/m50458.h \
	src/emu/video/m50458.c \

$(OBJ)/emu/video/mb90082.o : \
	src/emu/video/mb90082.c \
	src/emu/video/mb90082.h \

$(OBJ)/emu/video/mc6845.o : \
	src/emu/video/mc6845.h \
	src/emu/video/mc6845.c \

$(OBJ)/emu/video/mc6847.o : \
	src/emu/video/mc6847.h \
	src/emu/video/mc6847.c \

$(OBJ)/emu/video/msm6255.o : \
	src/emu/video/msm6255.h \
	src/emu/video/msm6255.c \

$(OBJ)/emu/video/pc_cga.o : \
	src/emu/video/mc6845.h \
	src/emu/video/pc_cga.c \
	src/emu/video/cgapal.h \
	src/emu/video/pc_cga.h \

$(OBJ)/emu/video/pc_vga.o : \
	src/emu/video/pc_vga.h \
	src/emu/machine/eepromser.h \
	src/emu/machine/eeprom.h \
	src/emu/video/pc_vga.c \

$(OBJ)/emu/video/poly.o : \
	src/emu/video/poly.c \
	src/emu/video/poly.h \

$(OBJ)/emu/video/psx.o : \
	src/emu/video/psx.c \
	src/emu/video/psx.h \

$(OBJ)/emu/video/ramdac.o : \
	src/emu/video/ramdac.c \
	src/emu/video/ramdac.h \

$(OBJ)/emu/video/resnet.o : \
	src/emu/video/resnet.h \
	src/emu/video/resnet.c \

$(OBJ)/emu/video/rgbutil.o : \
	src/emu/video/rgbgen.h \
	src/emu/video/rgbutil.h \
	src/emu/video/rgbvmx.h \
	src/emu/video/rgbutil.c \
	src/emu/video/rgbsse.h \

$(OBJ)/emu/video/saa5050.o : \
	src/emu/video/saa5050.h \
	src/emu/video/saa5050.c \

$(OBJ)/emu/video/sed1330.o : \
	src/emu/video/sed1330.h \
	src/emu/video/sed1330.c \

$(OBJ)/emu/video/stvvdp1.o : \
	src/mame/includes/stv.h \
	src/lib/util/palette.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/cdrom.h \
	src/lib/util/sha1.h \
	src/emu/video/stvvdp1.c \
	src/emu/cpu/scudsp/scudsp.h \
	src/lib/util/chd.h \
	src/emu/machine/eepromser.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/adsp2100/adsp2100.h \
	src/lib/util/chdcodec.h \
	src/lib/softfloat/milieu.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/lib/softfloat/softfloat.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/lib/util/astring.h \

$(OBJ)/emu/video/stvvdp2.o : \
	src/mame/includes/stv.h \
	src/lib/util/palette.h \
	src/lib/softfloat/softfloat-macros \
	src/emu/cpu/m68000/m68000.h \
	src/lib/util/cdrom.h \
	src/emu/video/stvvdp2.c \
	src/lib/util/sha1.h \
	src/emu/cpu/scudsp/scudsp.h \
	src/lib/util/chd.h \
	src/emu/machine/eepromser.h \
	src/lib/softfloat/mamesf.h \
	src/emu/cpu/adsp2100/adsp2100.h \
	src/lib/util/chdcodec.h \
	src/lib/softfloat/milieu.h \
	src/lib/util/md5.h \
	src/emu/machine/eeprom.h \
	src/lib/softfloat/softfloat.h \
	src/lib/util/corefile.h \
	src/lib/util/bitmap.h \
	src/lib/util/coretmpl.h \
	src/lib/util/hashing.h \
	src/lib/util/astring.h \

$(OBJ)/emu/video/tlc34076.o : \
	src/emu/video/tlc34076.c \
	src/emu/video/tlc34076.h \

$(OBJ)/emu/video/tms34061.o : \
	src/emu/video/tms34061.c \
	src/emu/video/tms34061.h \

$(OBJ)/emu/video/tms3556.o : \
	src/emu/video/tms3556.c \
	src/emu/video/tms3556.h \

$(OBJ)/emu/video/tms9927.o : \
	src/emu/video/tms9927.c \
	src/emu/video/tms9927.h \

$(OBJ)/emu/video/tms9928a.o : \
	src/emu/video/tms9928a.h \
	src/emu/video/tms9928a.c \

$(OBJ)/emu/video/upd3301.o : \
	src/emu/video/upd3301.h \
	src/emu/video/upd3301.c \

$(OBJ)/emu/video/upd7220.o : \
	src/emu/video/upd7220.h \
	src/emu/video/upd7220.c \

$(OBJ)/emu/video/upd7227.o : \
	src/emu/video/upd7227.h \
	src/emu/video/upd7227.c \

$(OBJ)/emu/video/v9938.o : \
	src/emu/video/v9938.c \
	src/emu/video/v9938.h \

$(OBJ)/emu/video/vector.o : \
	src/emu/video/vector.h \
	src/emu/video/vector.c \

$(OBJ)/emu/video/voodoo.o : \
	src/emu/video/rgbgen.h \
	src/emu/video/vooddefs.h \
	src/emu/video/rgbutil.h \
	src/emu/video/voodoo.h \
	src/emu/video/rgbvmx.h \
	src/emu/video/voodoo.c \
	src/emu/video/poly.h \
	src/emu/video/rgbsse.h \

$(OBJ)/emu/addrmap.o : \

$(OBJ)/emu/attotime.o : \

$(OBJ)/emu/audit.o : \

$(OBJ)/emu/cheat.o : \

$(OBJ)/emu/clifront.o : \

$(OBJ)/emu/config.o : \

$(OBJ)/emu/crsshair.o : \

$(OBJ)/emu/debugger.o : \

$(OBJ)/emu/delegate.o : \

$(OBJ)/emu/devcb.o : \

$(OBJ)/emu/devcb2.o : \

$(OBJ)/emu/devcpu.o : \

$(OBJ)/emu/devdelegate.o : \

$(OBJ)/emu/devfind.o : \

$(OBJ)/emu/device.o : \

$(OBJ)/emu/didisasm.o : \

$(OBJ)/emu/diexec.o : \

$(OBJ)/emu/diimage.o : \

$(OBJ)/emu/dimemory.o : \

$(OBJ)/emu/dinetwork.o : \

$(OBJ)/emu/dinvram.o : \

$(OBJ)/emu/dirtc.o : \

$(OBJ)/emu/diserial.o : \

$(OBJ)/emu/dislot.o : \

$(OBJ)/emu/disound.o : \

$(OBJ)/emu/distate.o : \

$(OBJ)/emu/divideo.o : \

$(OBJ)/emu/drawgfx.o : \

$(OBJ)/emu/drivenum.o : \

$(OBJ)/emu/driver.o : \

$(OBJ)/emu/emualloc.o : \

$(OBJ)/emu/emucore.o : \

$(OBJ)/emu/emuopts.o : \

$(OBJ)/emu/emupal.o : \

$(OBJ)/emu/fileio.o : \

$(OBJ)/emu/hash.o : \

$(OBJ)/emu/hashfile.o : \

$(OBJ)/emu/image.o : \

$(OBJ)/emu/info.o : \

$(OBJ)/emu/input.o : \

$(OBJ)/emu/ioport.o : \

$(OBJ)/emu/luaengine.o : \

$(OBJ)/emu/machine.o : \

$(OBJ)/emu/mame.o : \

$(OBJ)/emu/mconfig.o : \

$(OBJ)/emu/memarray.o : \

$(OBJ)/emu/memory.o : \

$(OBJ)/emu/network.o : \

$(OBJ)/emu/output.o : \

$(OBJ)/emu/profiler.o : \

$(OBJ)/emu/render.o : \

$(OBJ)/emu/rendersw.o : \

$(OBJ)/emu/rendfont.o : \

$(OBJ)/emu/rendlay.o : \

$(OBJ)/emu/rendutil.o : \

$(OBJ)/emu/romload.o : \

$(OBJ)/emu/save.o : \

$(OBJ)/emu/schedule.o : \

$(OBJ)/emu/screen.o : \

$(OBJ)/emu/softlist.o : \

$(OBJ)/emu/sound.o : \

$(OBJ)/emu/speaker.o : \

$(OBJ)/emu/sprite.o : \

$(OBJ)/emu/tilemap.o : \

$(OBJ)/emu/timer.o : \

$(OBJ)/emu/ui.o : \

$(OBJ)/emu/uigfx.o : \

$(OBJ)/emu/uiimage.o : \

$(OBJ)/emu/uiinput.o : \

$(OBJ)/emu/uimain.o : \

$(OBJ)/emu/uimenu.o : \

$(OBJ)/emu/uiswlist.o : \

$(OBJ)/emu/validity.o : \

$(OBJ)/emu/video.o : \

$(OBJ)/emu/webengine.o : \
