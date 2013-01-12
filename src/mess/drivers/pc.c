/***************************************************************************

    drivers/pc.c

Driver file for IBM PC, IBM PC XT, and related machines.

    PC-XT memory map

    00000-9FFFF   RAM
    A0000-AFFFF   NOP       or videoram EGA/VGA
    B0000-B7FFF   videoram  MDA, page #0
    B8000-BFFFF   videoram  CGA and/or MDA page #1, T1T mapped RAM
    C0000-C7FFF   NOP       or ROM EGA/VGA
    C8000-C9FFF   ROM       XT HDC #1
    CA000-CBFFF   ROM       XT HDC #2
    D0000-EFFFF   NOP       or 'adapter RAM'
    F0000-FDFFF   NOP       or ROM Basic + other Extensions
    FE000-FFFFF   ROM

Tandy 1000
==========

Tandy 1000 machines are similar to the IBM 5160s with CGA graphics. Tandy
added some additional graphic capabilities similar, but not equal, to
those added for the IBM PC Jr.

Tandy 1000 (8088) variations:
1000                128KB-640KB RAM     4.77 MHz        v01.00.00, v01.01.00
1000A/1000HD        128KB-640KB RAM     4.77 MHz        v01.01.00
1000SX/1000AX       384KB-640KB RAM     7.16/4.77 MHz   v01.02.00
1000EX              256KB-640KB RAM     7.16/4.77 MHz   v01.02.00
1000HX              256KB-640KB RAM     7.16/4.77 MHz   v02.00.00

Tandy 1000 (8086) variations:
1000RL/1000RL-HD    512KB-768KB RAM     9.44/4.77 MHz   v02.00.00, v02.00.01
1000SL/1000PC       384KB-640KB RAM     8.0/4.77 MHz    v01.04.00, v01.04.01, v01.04.02, v02.00.01
1000SL/2            512KB-640KB RAM     8.0/4.77 MHz    v01.04.04

Tandy 1000 (80286) variations:
1000TX              640KB-768KB RAM     8.0/4.77 MHz    v01.03.00
1000TL              640KB-768KB RAM     8.0/4.77 MHz    v01.04.00, v01.04.01, v01.04.02
1000TL/2            640KB-768KB RAM     8.0/4.77 MHz    v02.00.00
1000TL/3            640KB-768KB RAM     10.0/5.0 MHz    v02.00.00
1000RLX             512KB-1024KB RAM    10.0/5.0 MHz    v02.00.00
1000RLX-HD          1024MB RAM          10.0/5.0 MHz    v02.00.00

Tandy 1000 (80386) variations:
1000RSX/1000RSX-HD  1M-9M RAM           25.0/8.0 MHz    v01.10.00


IBM5550
=======
Information can be found at http://homepage3.nifty.com/ibm5550/index-e.html
It's an heavily modified IBM PC-XT machine, with a completely different
video HW too.

***************************************************************************/


#include "emu.h"
#include "cpu/nec/nec.h"
#include "cpu/i86/i86.h"
#include "cpu/i86/i286.h"
#include "sound/speaker.h"

#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/i8251.h"
#include "machine/mc146818.h"
#include "machine/pic8259.h"

#include "machine/pit8253.h"
#include "video/pc_vga.h"
#include "video/pc_cga.h"
#include "video/pc_aga.h"
#include "video/pc_t1t.h"

#include "machine/pc_fdc.h"
#include "machine/pc_joy.h"
#include "machine/pckeybrd.h"
#include "machine/pc_lpt.h"
#include "machine/serial.h"

#include "includes/europc.h"
#include "includes/tandy1t.h"

#include "machine/pcshare.h"
#include "includes/pc.h"

#include "imagedev/flopdrv.h"
#include "imagedev/harddriv.h"
#include "imagedev/cassette.h"
#include "imagedev/cartslot.h"
#include "formats/mfi_dsk.h"
#include "formats/pc_dsk.h"

#include "machine/am9517a.h"
#include "sound/sn76496.h"

#include "machine/wd_fdc.h"
#include "machine/kb_7007_3.h"

#include "machine/ram.h"
#include "machine/pc_keyboards.h"


static ADDRESS_MAP_START( pc8_map, AS_PROGRAM, 8, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( oliv_map, AS_PROGRAM, 8, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAM
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( iskr1031_map, AS_PROGRAM, 16, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x7ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ec1841_map, AS_PROGRAM, 16, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x7ffff) AM_RAMBANK("bank10") // up to 4 banks
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xdc000, 0xdffff) AM_RAM       // monochrome chargen
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mc1502_map, AS_PROGRAM, 8, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x97fff) AM_RAMBANK("bank10") /* 96K on mainboard + 512K on extension card */
	AM_RANGE(0xe8000, 0xeffff) AM_ROM       /* BASIC */
	AM_RANGE(0xfc000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(mc1502_io, AS_IO, 8, pc_state )
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE_LEGACY("pic8259", pic8259_r, pic8259_w)
	AM_RANGE(0x0028, 0x0028) AM_DEVREADWRITE("upd8251", i8251_device, data_r, data_w)   // not working yet
	AM_RANGE(0x0029, 0x0029) AM_DEVREADWRITE("upd8251", i8251_device, status_r, control_w)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE_LEGACY("pit8253", pit8253_r, pit8253_w)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x0068, 0x006B) AM_DEVREADWRITE("ppi8255n2", i8255_device, read, write)    // keyboard poll
	AM_RANGE(0x0100, 0x0100) AM_READWRITE(mc1502_wd17xx_aux_r, mc1502_wd17xx_aux_w)
	AM_RANGE(0x0108, 0x0108) AM_READ(mc1502_wd17xx_drq_r)           // blocking read!
	AM_RANGE(0x010a, 0x010a) AM_READ(mc1502_wd17xx_motor_r)
	AM_RANGE(0x010c, 0x010f) AM_DEVREADWRITE("vg93", fd1793_t, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( zenith_map, AS_PROGRAM, 8, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xf7fff) AM_RAM
	AM_RANGE(0xf8000, 0xfffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( pc16_map, AS_PROGRAM, 16, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(pc8_io, AS_IO, 8, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE("dma8237", am9517a_device, read, write)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE_LEGACY("pic8259", pic8259_r, pic8259_w)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE_LEGACY("pit8253", pit8253_r, pit8253_w)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE(pc_page_r,            pc_page_w)
	AM_RANGE(0x00a0, 0x00a0) AM_WRITE(pc_nmi_enable_w )
	AM_RANGE(0x0200, 0x0207) AM_READWRITE_LEGACY(pc_JOY_r,              pc_JOY_w)
	AM_RANGE(0x0240, 0x0257) AM_READWRITE(pc_rtc_r,             pc_rtc_w)
	AM_RANGE(0x0278, 0x027b) AM_DEVREADWRITE_LEGACY("lpt_2", pc_lpt_r, pc_lpt_w)
	AM_RANGE(0x02e8, 0x02ef) AM_DEVREADWRITE("ins8250_3", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE("ins8250_1", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x0340, 0x0357) AM_NOP /* anonymous bios should not recogniced realtimeclock */
	AM_RANGE(0x0378, 0x037f) AM_DEVREADWRITE_LEGACY("lpt_1", pc_lpt_r, pc_lpt_w)
	AM_RANGE(0x03bc, 0x03be) AM_DEVREADWRITE_LEGACY("lpt_0", pc_lpt_r, pc_lpt_w)
	AM_RANGE(0x03e8, 0x03ef) AM_DEVREADWRITE("ins8250_2", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE("fdc", pc_fdc_interface, map)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE("ins8250_0", ins8250_device, ins8250_r, ins8250_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(oliv_io, AS_IO, 8, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START(pc16_io, AS_IO, 16, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE8("dma8237", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8_LEGACY("pic8259", pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8_LEGACY("pit8253", pit8253_r, pit8253_w, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0xffff)
	AM_RANGE(0x0070, 0x007f) AM_RAM // needed for Poisk-2
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r,               pc_page_w, 0xffff)
	AM_RANGE(0x00a0, 0x00a1) AM_WRITE8(pc_nmi_enable_w, 0x00ff )
	AM_RANGE(0x0200, 0x0207) AM_READWRITE8_LEGACY(pc_JOY_r, pc_JOY_w, 0xffff)
	AM_RANGE(0x0240, 0x0257) AM_READWRITE8(pc_rtc_r,                pc_rtc_w, 0xffff)
	AM_RANGE(0x0278, 0x027b) AM_DEVREADWRITE8_LEGACY("lpt_2", pc_lpt_r, pc_lpt_w, 0xffff)
	AM_RANGE(0x02e8, 0x02ef) AM_DEVREADWRITE8("ins8250_3", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE8("ins8250_1", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x0340, 0x0357) AM_NOP /* anonymous bios should not recogniced realtimeclock */
	AM_RANGE(0x0378, 0x037f) AM_DEVREADWRITE8_LEGACY("lpt_1", pc_lpt_r, pc_lpt_w, 0xffff)
	AM_RANGE(0x03bc, 0x03bf) AM_DEVREADWRITE8_LEGACY("lpt_0", pc_lpt_r, pc_lpt_w, 0xffff)
	AM_RANGE(0x03e8, 0x03ef) AM_DEVREADWRITE8("ins8250_2", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE8("fdc", pc_fdc_interface, map, 0xffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ins8250_0", ins8250_device, ins8250_r, ins8250_w, 0xffff)
ADDRESS_MAP_END


static ADDRESS_MAP_START(ec1841_io, AS_IO, 16, pc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE8("dma8237", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8_LEGACY("pic8259", pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8_LEGACY("pit8253", pit8253_r, pit8253_w, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0xffff)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r,               pc_page_w, 0xffff)
	AM_RANGE(0x00a0, 0x00a1) AM_WRITE8( pc_nmi_enable_w, 0x00ff )
	AM_RANGE(0x0210, 0x0217) AM_NOP // expansion chassis interface
//  AM_RANGE(0x0230, 0x021f)    // mouse
	AM_RANGE(0x0240, 0x0257) AM_READWRITE8(pc_rtc_r,                pc_rtc_w, 0xffff)
	AM_RANGE(0x02b0, 0x02b3) AM_READWRITE8(ec1841_memboard_r, ec1841_memboard_w, 0xffff);
//  AM_RANGE(0x02f8, 0x02f8) AM_DEVREADWRITE8_LEGACY("upd8251_1", i8251_device, data_r, data_w, 0x00ff)
//  AM_RANGE(0x02f9, 0x02f9) AM_DEVREADWRITE8_LEGACY("upd8251_1", i8251_device, status_r, control_w, 0xff00)
	AM_RANGE(0x0378, 0x037f) AM_DEVREADWRITE8_LEGACY("lpt_0", pc_lpt_r, pc_lpt_w, 0xffff)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE8("fdc", pc_fdc_interface, map, 0xffff)
//  AM_RANGE(0x03f8, 0x03f9) AM_DEVREADWRITE8_LEGACY("upd8251_0", i8251_device, data_r, data_w, 0x00ff)
//  AM_RANGE(0x03f8, 0x03f9) AM_DEVREADWRITE8_LEGACY("upd8251_0", i8251_device, status_r, control_w, 0xff00)
ADDRESS_MAP_END


static ADDRESS_MAP_START(iskr1031_io, AS_IO, 16, pc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE8("dma8237", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8_LEGACY("pic8259", pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8_LEGACY("pit8253", pit8253_r, pit8253_w, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0xffff)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r,               pc_page_w, 0xffff)
	AM_RANGE(0x00a0, 0x00a1) AM_WRITE8( pc_nmi_enable_w, 0x00ff )
//  AM_RANGE(0x0200, 0x0207) AM_READWRITE8_LEGACY(pc_JOY_r, pc_JOY_w, 0xffff)
	AM_RANGE(0x0240, 0x0257) AM_READWRITE8(pc_rtc_r,                pc_rtc_w, 0xffff)
//  AM_RANGE(0x02e8, 0x02ef) AM_DEVREADWRITE8("ins8250_3", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE8("ins8250_1", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x0340, 0x0357) AM_NOP /* anonymous bios should not recogniced realtimeclock */
	AM_RANGE(0x0378, 0x037f) AM_DEVREADWRITE8_LEGACY("lpt_0", pc_lpt_r, pc_lpt_w, 0xffff)
//  AM_RANGE(0x03e8, 0x03ef) AM_DEVREADWRITE8("ins8250_2", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE8("fdc", pc_fdc_interface, map, 0xffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ins8250_0", ins8250_device, ins8250_r, ins8250_w, 0xffff)
ADDRESS_MAP_END


static ADDRESS_MAP_START( ibm5550_map, AS_PROGRAM, 16, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xeffff) AM_RAM
	AM_RANGE(0xfc000, 0xfffff) AM_ROM
ADDRESS_MAP_END

READ8_MEMBER(pc_state::unk_r)
{
	return 0;
}

static ADDRESS_MAP_START(ibm5550_io, AS_IO, 16, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE8("dma8237", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8_LEGACY("pic8259", pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8_LEGACY("pit8253", pit8253_r, pit8253_w, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0xffff)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r,               pc_page_w, 0xffff)
	AM_RANGE(0x00a0, 0x00a1) AM_READWRITE8(unk_r, pc_nmi_enable_w, 0x00ff )
	AM_RANGE(0x0200, 0x0207) AM_READWRITE8_LEGACY(pc_JOY_r, pc_JOY_w, 0xffff)
	AM_RANGE(0x0240, 0x0257) AM_READWRITE8(pc_rtc_r,                pc_rtc_w, 0xffff)
	AM_RANGE(0x0278, 0x027b) AM_DEVREADWRITE8_LEGACY("lpt_2", pc_lpt_r, pc_lpt_w, 0xffff)
	AM_RANGE(0x02e8, 0x02ef) AM_DEVREADWRITE8("ins8250_3", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE8("ins8250_1", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x0340, 0x0357) AM_NOP /* anonymous bios should not recogniced realtimeclock */
	AM_RANGE(0x0378, 0x037f) AM_DEVREADWRITE8_LEGACY("lpt_1", pc_lpt_r, pc_lpt_w, 0xffff)
	AM_RANGE(0x03bc, 0x03bf) AM_DEVREADWRITE8_LEGACY("lpt_0", pc_lpt_r, pc_lpt_w, 0xffff)
	AM_RANGE(0x03e8, 0x03ef) AM_DEVREADWRITE8("ins8250_2", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE8("fdc", pc_fdc_interface, map, 0xffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ins8250_0", ins8250_device, ins8250_r, ins8250_w, 0xffff)
ADDRESS_MAP_END


static ADDRESS_MAP_START( europc_map, AS_PROGRAM, 8, pc_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END



static ADDRESS_MAP_START(europc_io, AS_IO, 8, pc_state )
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE("dma8237", am9517a_device, read, write)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE_LEGACY("pic8259", pic8259_r, pic8259_w)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE_LEGACY("pit8253", pit8253_r, pit8253_w)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE_LEGACY(europc_pio_r,          europc_pio_w)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE(pc_page_r,            pc_page_w)
	AM_RANGE(0x0200, 0x0207) AM_READWRITE_LEGACY(pc_JOY_r,              pc_JOY_w)
	AM_RANGE(0x0250, 0x025f) AM_READWRITE_LEGACY(europc_jim_r,          europc_jim_w)
	AM_RANGE(0x0278, 0x027b) AM_DEVREADWRITE_LEGACY("lpt_2", pc_lpt_r, pc_lpt_w)
	AM_RANGE(0x02e0, 0x02e0) AM_READ_LEGACY(europc_jim2_r)
	AM_RANGE(0x02e8, 0x02ef) AM_DEVREADWRITE("ins8250_3", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE("ins8250_1", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x0378, 0x037b) AM_DEVREADWRITE_LEGACY("lpt_1", pc_lpt_r, pc_lpt_w)
//  AM_RANGE(0x03bc, 0x03bf) AM_DEVREADWRITE_LEGACY("lpt_0", pc_lpt_r, pc_lpt_w)
	AM_RANGE(0x03e8, 0x03ef) AM_DEVREADWRITE("ins8250_2", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE("fdc", pc_fdc_interface, map)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE("ins8250_0", ins8250_device, ins8250_r, ins8250_w)
ADDRESS_MAP_END



static ADDRESS_MAP_START(tandy1000_map, AS_PROGRAM, 8, pc_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_RAM
	AM_RANGE(0xb0000, 0xb7fff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xc9fff) AM_ROM
	AM_RANGE(0xca000, 0xcffff) AM_NOP
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END



static ADDRESS_MAP_START(tandy1000_io, AS_IO, 8, pc_state )
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE("dma8237", am9517a_device, read, write)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE_LEGACY("pic8259", pic8259_r, pic8259_w)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE_LEGACY("pit8253", pit8253_r, pit8253_w)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE_LEGACY(tandy1000_pio_r,           tandy1000_pio_w)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE(pc_page_r,                pc_page_w)
	AM_RANGE(0x00c0, 0x00c0) AM_DEVWRITE("sn76496", ncr7496_device, write)
	AM_RANGE(0x0200, 0x0207) AM_READWRITE_LEGACY(pc_JOY_r,                  pc_JOY_w)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE("ins8250_1", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x0378, 0x037f) AM_READWRITE_LEGACY(pc_t1t_p37x_r,         pc_t1t_p37x_w)
	AM_RANGE(0x03bc, 0x03be) AM_DEVREADWRITE_LEGACY("lpt_0", pc_lpt_r, pc_lpt_w)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE("fdc", pc_fdc_interface, map)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE("ins8250_0", ins8250_device, ins8250_r, ins8250_w)
ADDRESS_MAP_END



static ADDRESS_MAP_START(tandy1000_16_map, AS_PROGRAM, 16, pc_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_RAM
	AM_RANGE(0xb0000, 0xb7fff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xc9fff) AM_ROM
	AM_RANGE(0xca000, 0xcffff) AM_NOP
	AM_RANGE(0xe0000, 0xeffff) AM_ROMBANK("bank11")                     /* Banked part of the BIOS */
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION("maincpu", 0x70000)
ADDRESS_MAP_END



static ADDRESS_MAP_START(tandy1000_16_io, AS_IO, 16, pc_state )
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE8("dma8237", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8_LEGACY("pic8259", pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8_LEGACY("pit8253", pit8253_r, pit8253_w, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE8_LEGACY(tandy1000_pio_r,          tandy1000_pio_w, 0xffff)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r,               pc_page_w, 0xffff)
	AM_RANGE(0x00c0, 0x00c1) AM_DEVWRITE8("sn76496",    ncr7496_device, write, 0xffff)
	AM_RANGE(0x0200, 0x0207) AM_READWRITE8_LEGACY(pc_JOY_r,                 pc_JOY_w, 0xffff)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE8("ins8250_1", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x0378, 0x037f) AM_READWRITE8_LEGACY(pc_t1t_p37x_r,            pc_t1t_p37x_w, 0xffff)
	AM_RANGE(0x03bc, 0x03bf) AM_DEVREADWRITE8_LEGACY("lpt_0", pc_lpt_r, pc_lpt_w, 0xffff)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE8("fdc", pc_fdc_interface, map, 0xffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ins8250_0", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0xffea, 0xffeb) AM_READWRITE8_LEGACY(tandy1000_bank_r, tandy1000_bank_w, 0xffff)
ADDRESS_MAP_END



static ADDRESS_MAP_START(tandy1000_286_map, AS_PROGRAM, 16, pc_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_RAM
	AM_RANGE(0xb0000, 0xb7fff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xc9fff) AM_ROM
	AM_RANGE(0xca000, 0xcffff) AM_NOP
	AM_RANGE(0xe0000, 0xeffff) AM_NOP
	AM_RANGE(0xf8000, 0xfffff) AM_ROM
ADDRESS_MAP_END



static ADDRESS_MAP_START(tandy1000_286_io, AS_IO, 16, pc_state )
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE8("dma8237", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8_LEGACY("pic8259", pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8_LEGACY("pit8253", pit8253_r, pit8253_w, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE8_LEGACY(tandy1000_pio_r,         tandy1000_pio_w, 0xffff)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r,               pc_page_w, 0xffff)
	AM_RANGE(0x00c0, 0x00c1) AM_DEVWRITE8("sn76496", ncr7496_device, write, 0xffff)
	AM_RANGE(0x0200, 0x0207) AM_READWRITE8_LEGACY(pc_JOY_r,                    pc_JOY_w, 0xffff)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE8("ins8250_1", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x0378, 0x037f) AM_READWRITE8_LEGACY(pc_t1t_p37x_r,           pc_t1t_p37x_w, 0xffff)
	AM_RANGE(0x03bc, 0x03bf) AM_DEVREADWRITE8_LEGACY("lpt_0", pc_lpt_r, pc_lpt_w, 0xffff)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE8("fdc", pc_fdc_interface, map, 0xffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ins8250_0", ins8250_device, ins8250_r, ins8250_w, 0xffff)
ADDRESS_MAP_END


static ADDRESS_MAP_START(ibmpcjr_map, AS_PROGRAM, 8, pc_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_RAM
	AM_RANGE(0xb0000, 0xb7fff) AM_NOP
	AM_RANGE(0xb8000, 0xbffff) AM_RAMBANK("bank14")
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xc9fff) AM_ROM
	AM_RANGE(0xca000, 0xcffff) AM_NOP
	AM_RANGE(0xd0000, 0xdffff) AM_ROM
	AM_RANGE(0xe0000, 0xeffff) AM_ROM
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(ibmpcjr_io, AS_IO, 8, pc_state )
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE_LEGACY("pic8259", pic8259_r, pic8259_w)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE_LEGACY("pit8253", pit8253_r, pit8253_w)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE(pc_page_r,                pc_page_w)
	AM_RANGE(0x00a0, 0x00a0) AM_READWRITE(pcjr_nmi_enable_r, pc_nmi_enable_w )
	AM_RANGE(0x00c0, 0x00c0) AM_DEVWRITE("sn76496", sn76496_device, write)
	AM_RANGE(0x00f2, 0x00f2) AM_WRITE(pcjr_fdc_dor_w)
	AM_RANGE(0x00f4, 0x00f5) AM_DEVICE("upd765", upd765a_device, map)
	AM_RANGE(0x0200, 0x0207) AM_READWRITE_LEGACY(pc_JOY_r,                  pc_JOY_w)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE("ins8250_1", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x0378, 0x037f) AM_READWRITE_LEGACY(pc_t1t_p37x_r,         pc_t1t_p37x_w)
	AM_RANGE(0x03bc, 0x03be) AM_DEVREADWRITE_LEGACY("lpt_0", pc_lpt_r, pc_lpt_w)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE("ins8250_0", ins8250_device, ins8250_r, ins8250_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ibmpcjx_map, AS_PROGRAM, 8, pc_state )
	AM_RANGE(0x80000, 0xb7fff) AM_ROM AM_REGION("kanji",0)
	AM_IMPORT_FROM( ibmpcjr_map )
ADDRESS_MAP_END

static ADDRESS_MAP_START(ibmpcjx_io, AS_IO, 8, pc_state )
	AM_RANGE(0x01ff, 0x01ff) AM_READWRITE(pcjx_port_1ff_r, pcjx_port_1ff_w)
	AM_IMPORT_FROM( ibmpcjr_io )
ADDRESS_MAP_END


static INPUT_PORTS_START( pccga )
	PORT_START("IN0") /* IN0 */
	PORT_BIT ( 0xf0, 0xf0,   IPT_UNUSED )
	PORT_BIT ( 0x08, 0x08,   IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT ( 0x07, 0x07,   IPT_UNUSED )

	PORT_START("DSW0") /* IN1 */
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPNAME( 0x30, 0x20, "Graphics adapter")
	PORT_DIPSETTING(    0x00, "EGA/VGA" )
	PORT_DIPSETTING(    0x10, "Color 40x25" )
	PORT_DIPSETTING(    0x20, "Color 80x25" )
	PORT_DIPSETTING(    0x30, "Monochrome" )
	PORT_DIPNAME( 0x0c, 0x0c, "RAM banks")
	PORT_DIPSETTING(    0x00, "1 - 16  64 256K" )
	PORT_DIPSETTING(    0x04, "2 - 32 128 512K" )
	PORT_DIPSETTING(    0x08, "3 - 48 192 576K" )
	PORT_DIPSETTING(    0x0c, "4 - 64 256 640K" )
	PORT_DIPNAME( 0x02, 0x00, "80387 installed")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x01, "Floppy installed")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW1") /* IN2 */
	PORT_DIPNAME( 0x80, 0x80, "COM1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "COM2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "COM3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "COM4: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "LPT1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "LPT2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "LPT3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x00, "Game port enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW2") /* IN3 */
	PORT_DIPNAME( 0x08, 0x08, "HDC1 (C800:0 port 320-323)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "HDC2 (CA00:0 port 324-327)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_BIT( 0x02, 0x02,   IPT_UNUSED ) /* no turbo switch */
	PORT_BIT( 0x01, 0x01,   IPT_UNUSED )

	PORT_INCLUDE( pc_joystick )         /* IN15 - IN19 */
	PORT_INCLUDE( pcvideo_cga )
INPUT_PORTS_END

static INPUT_PORTS_START( pcega )
	PORT_START("IN0") /* IN0 */
	PORT_BIT ( 0xf0, 0xf0,   IPT_UNUSED )
	PORT_BIT ( 0x08, 0x08,   IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT ( 0x07, 0x07,   IPT_UNUSED )

	PORT_START("DSW0") /* IN1 */
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPNAME( 0x00, 0x20, "Graphics adapter")
	PORT_DIPSETTING(    0x00, "EGA/VGA" )
	PORT_DIPSETTING(    0x10, "Color 40x25" )
	PORT_DIPSETTING(    0x20, "Color 80x25" )
	PORT_DIPSETTING(    0x30, "Monochrome" )
	PORT_DIPNAME( 0x0c, 0x0c, "RAM banks")
	PORT_DIPSETTING(    0x00, "1 - 16  64 256K" )
	PORT_DIPSETTING(    0x04, "2 - 32 128 512K" )
	PORT_DIPSETTING(    0x08, "3 - 48 192 576K" )
	PORT_DIPSETTING(    0x0c, "4 - 64 256 640K" )
	PORT_DIPNAME( 0x02, 0x00, "80387 installed")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x01, "Floppy installed")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW1") /* IN2 */
	PORT_DIPNAME( 0x80, 0x80, "COM1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "COM2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "COM3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "COM4: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "LPT1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "LPT2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "LPT3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x00, "Game port enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW2") /* IN3 */
	PORT_DIPNAME( 0x08, 0x08, "HDC1 (C800:0 port 320-323)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "HDC2 (CA00:0 port 324-327)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_BIT( 0x02, 0x02,   IPT_UNUSED ) /* no turbo switch */
	PORT_BIT( 0x01, 0x01,   IPT_UNUSED )

	PORT_INCLUDE( pc_joystick )         /* IN15 - IN19 */
	PORT_INCLUDE( pcvideo_cga )
INPUT_PORTS_END


static INPUT_PORTS_START( europc )
	PORT_START("IN0") /* IN0 */
	PORT_BIT ( 0xf0, 0xf0,   IPT_UNUSED )
	PORT_BIT ( 0x08, 0x08,   IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT ( 0x07, 0x07,   IPT_UNUSED )

	PORT_START("DSW0") /* IN1 */

	PORT_START("DSW1") /* IN2 */
	PORT_DIPNAME( 0x80, 0x80, "COM1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "COM2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "COM3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "COM4: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "LPT1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "LPT2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "LPT3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x00, "Game port enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW2") /* IN3 */
	PORT_DIPNAME( 0x08, 0x08, "HDC1 (C800:0 port 320-323)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "HDC2 (CA00:0 port 324-327)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_BIT( 0x02, 0x02,   IPT_UNUSED ) /* no turbo switch */
	PORT_BIT( 0x01, 0x01,   IPT_UNUSED )

	EUROPC_KEYBOARD

	PORT_INCLUDE( pc_joystick )         /* IN15 - IN19 */
	PORT_INCLUDE( pcvideo_cga )
INPUT_PORTS_END

static INPUT_PORTS_START( bondwell )
	PORT_START("IN0") /* IN0 */
	PORT_BIT ( 0xf0, 0xf0,   IPT_UNUSED )
	PORT_BIT ( 0x08, 0x08,   IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT ( 0x07, 0x07,   IPT_UNUSED )

	PORT_START("DSW0") /* IN1 */
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPNAME( 0x30, 0x20, "Graphics adapter")
	PORT_DIPSETTING(    0x00, "EGA/VGA" )
	PORT_DIPSETTING(    0x10, "Color 40x25" )
	PORT_DIPSETTING(    0x20, "Color 80x25" )
	PORT_DIPSETTING(    0x30, "Monochrome" )
	PORT_DIPNAME( 0x0c, 0x0c, "RAM banks")
	PORT_DIPSETTING(    0x00, "1 - 16  64 256K" )
	PORT_DIPSETTING(    0x04, "2 - 32 128 512K" )
	PORT_DIPSETTING(    0x08, "3 - 48 192 576K" )
	PORT_DIPSETTING(    0x0c, "4 - 64 256 640K" )
	PORT_DIPNAME( 0x02, 0x00, "80387 installed")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x01, "Floppy installed")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW1") /* IN2 */
	PORT_DIPNAME( 0x80, 0x80, "COM1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "COM2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "COM3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "COM4: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "LPT1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "LPT2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "LPT3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x00, "Game port enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW2") /* IN3 */
	PORT_DIPNAME( 0x08, 0x08, "HDC1 (C800:0 port 320-323)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "HDC2 (CA00:0 port 324-327)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Turbo Switch" )
	PORT_DIPSETTING(    0x00, "Off (4.77 MHz)" )
	PORT_DIPSETTING(    0x02, "On (12 MHz)" )
	PORT_BIT( 0x01, 0x01,   IPT_UNUSED )

//  PORT_INCLUDE( at_keyboard )     /* IN4 - IN11 */
	PORT_INCLUDE( pc_joystick )         /* IN15 - IN19 */
	PORT_INCLUDE( pcvideo_cga )
INPUT_PORTS_END

static INPUT_PORTS_START( tandy1t )
	PORT_START("IN0") /* IN0 */
	PORT_BIT ( 0xf0, 0xf0,   IPT_UNUSED )
	PORT_BIT ( 0x08, 0x08,   IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT ( 0x07, 0x07,   IPT_UNUSED )

	PORT_START("DSW0") /* IN1 */
	PORT_BIT ( 0xff, 0xff,   IPT_UNUSED )

	PORT_START("DSW1") /* IN2 */
	PORT_DIPNAME( 0x80, 0x80, "COM1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "COM2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_BIT ( 0x30, 0x00,   IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, "LPT1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_BIT ( 0x06, 0x00,   IPT_UNUSED )
	PORT_DIPNAME( 0x01, 0x00, "Game port enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW2") /* IN3 */
	PORT_DIPNAME( 0x08, 0x08, "HDC1 (C800:0 port 320-323)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "HDC2 (CA00:0 port 324-327)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_BIT( 0x02, 0x02,   IPT_UNUSED ) /* no turbo switch */
	PORT_BIT( 0x01, 0x01,   IPT_UNUSED )

	PORT_INCLUDE( t1000_keyboard )
	PORT_INCLUDE( pc_joystick )         /* IN15 - IN19 */
INPUT_PORTS_END

static INPUT_PORTS_START( ibmpcjr )
	PORT_INCLUDE( tandy1t )
	PORT_MODIFY("pc_keyboard_3")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_MODIFY("pc_keyboard_4")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NumLock") PORT_CODE(KEYCODE_NUMLOCK)
INPUT_PORTS_END

static INPUT_PORTS_START( mc1502 )          /* fix */
	PORT_START("IN0") /* IN0 */
	PORT_BIT ( 0xf0, 0xf0,   IPT_UNUSED )
	PORT_BIT ( 0x08, 0x08,   IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT ( 0x07, 0x07,   IPT_UNUSED )

	PORT_START("DSW0") /* IN1 */
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPNAME( 0x30, 0x20, "Graphics adapter")
	PORT_DIPSETTING(    0x00, "EGA/VGA" )
	PORT_DIPSETTING(    0x10, "Color 40x25" )
	PORT_DIPSETTING(    0x20, "Color 80x25" )
	PORT_DIPSETTING(    0x30, "Monochrome" )
	PORT_DIPNAME( 0x0c, 0x0c, "RAM banks")
	PORT_DIPSETTING(    0x00, "1 - 16  64 256K" )
	PORT_DIPSETTING(    0x04, "2 - 32 128 512K" )
	PORT_DIPSETTING(    0x08, "3 - 48 192 576K" )
	PORT_DIPSETTING(    0x0c, "4 - 64 256 640K" )
	PORT_DIPNAME( 0x02, 0x00, "80387 installed")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x01, "Floppy installed")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW1") /* IN2 */
	PORT_DIPNAME( 0x80, 0x80, "COM1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "COM2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "COM3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "COM4: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "LPT1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "LPT2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "LPT3: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x00, "Game port enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW2") /* IN3 */
	PORT_DIPNAME( 0xf0, 0x80, "Serial mouse")
	PORT_DIPSETTING(    0x80, "COM1" )
	PORT_DIPSETTING(    0x40, "COM2" )
	PORT_DIPSETTING(    0x20, "COM3" )
	PORT_DIPSETTING(    0x10, "COM4" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x08, "HDC1 (C800:0 port 320-323)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "HDC2 (CA00:0 port 324-327)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_BIT( 0x02, 0x02,   IPT_UNUSED ) /* no turbo switch */
	PORT_BIT( 0x01, 0x01,   IPT_UNUSED )

	PORT_INCLUDE( mc7007_3_keyboard )
	PORT_INCLUDE( pcvideo_mc1502 )
INPUT_PORTS_END


static const unsigned i86_address_mask = 0x000fffff;

static const pc_lpt_interface pc_lpt_config =
{
	DEVCB_CPU_INPUT_LINE("maincpu", 0)
};

FLOPPY_FORMATS_MEMBER( pc_state::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( ibmpc_floppies )
		SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
		SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

SLOT_INTERFACE_START(ibm5150_com)
	SLOT_INTERFACE("microsoft_mouse", MSFT_SERIAL_MOUSE)
	SLOT_INTERFACE("mouse_systems_mouse", MSYSTEM_SERIAL_MOUSE)
SLOT_INTERFACE_END

#define MCFG_CPU_PC(mem, port, type, clock, vblankfunc) \
	MCFG_CPU_ADD("maincpu", type, clock)                \
	MCFG_CPU_PROGRAM_MAP(mem##_map) \
	MCFG_CPU_IO_MAP(port##_io)  \
	MCFG_CPU_CONFIG(i86_address_mask)   \
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", pc_state, vblankfunc, "screen", 0, 1)


/* F4 Character Displayer */
static const gfx_layout pc_16_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 2048*8, 2049*8, 2050*8, 2051*8, 2052*8, 2053*8, 2054*8, 2055*8 },
	8*8                 /* every char takes 2 x 8 bytes */
};

static const gfx_layout pc_8_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static const gfx_layout kanji_layout =
{
	16, 16,                 /* 8 x 8 characters */
	RGN_FRAC(1,1),                  /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ STEP16(0,1) },
	/* y offsets */
	{ STEP16(0,16) },
	16*16                   /* every char takes 8 bytes */
};

static GFXDECODE_START( ibm5150 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc_16_charlayout, 3, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, pc_8_charlayout, 3, 1 )
GFXDECODE_END


/*************************************
 *
 *  Sound interface
 *
 *************************************/


//-------------------------------------------------
//  sn76496_config psg_intf
//-------------------------------------------------

static const sn76496_config psg_intf =
{
	DEVCB_NULL
};


static const pc_kbdc_interface pc_kbdc_intf =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, pc_state, keyboard_clock_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, pc_state, keyboard_data_w)
};


static MACHINE_CONFIG_START( pccga, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(pc8, pc8, I8088, 4772720, pc_frame_interrupt)   /* 4,77 MHz */

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_2", ibm5150_com_interface[2], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_3", ibm5150_com_interface[3], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport2", ibm5150_serport_config[2], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport3", ibm5150_serport_config[3], ibm5150_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_cga )
	MCFG_GFXDECODE(ibm5150)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

static const gfx_layout europc_8_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*16                    /* every char takes 16 bytes */
};

static const gfx_layout europc_16_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( europc )
	GFXDECODE_ENTRY( "gfx1", 0x0000, europc_8_charlayout, 3, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x0800, europc_16_charlayout, 3, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( europc, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(europc, europc, I8088, 4772720*2, pc_frame_interrupt)

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", pc_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_2", ibm5150_com_interface[2], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_3", ibm5150_com_interface[3], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport2", ibm5150_serport_config[2], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport3", ibm5150_serport_config[3], ibm5150_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_aga )
	MCFG_GFXDECODE(europc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_NVRAM_HANDLER( europc_rtc )

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( t1000hx, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(tandy1000, tandy1000, I8088, 8000000, pc_frame_interrupt)

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", pc_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], ibm5150_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_t1000 )
	MCFG_GFXDECODE(europc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
	MCFG_SOUND_ADD("sn76496", NCR7496, XTAL_14_31818MHz/4)
	MCFG_SOUND_CONFIG(psg_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_NVRAM_HANDLER( tandy1000 )

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "35dd", 0, pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( t1000sx, t1000hx )
	MCFG_DEVICE_REMOVE("fdc:0")
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( t1000_16, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(tandy1000_16, tandy1000_16, I8086, XTAL_28_63636MHz / 3, pc_frame_interrupt)

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,tandy1000rl)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", pc_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], ibm5150_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_t1000 )
	MCFG_GFXDECODE(europc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
	MCFG_SOUND_ADD("sn76496", NCR7496, XTAL_14_31818MHz/4)
	MCFG_SOUND_CONFIG(psg_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_NVRAM_HANDLER( tandy1000 )

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_PC_FDC_XT_ADD("fdc")
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "35dd", 0, pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( t1000_286, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(tandy1000_286, tandy1000_286, I80286, XTAL_28_63636MHz / 2, pc_frame_interrupt)

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", pc_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], ibm5150_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_t1000 )
	MCFG_GFXDECODE(europc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
	MCFG_SOUND_ADD("sn76496", NCR7496, XTAL_14_31818MHz/4)
	MCFG_SOUND_CONFIG(psg_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_NVRAM_HANDLER( tandy1000 )

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_PC_FDC_XT_ADD("fdc")
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "35dd", 0, pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


static GFXDECODE_START( ibmpcjr )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc_8_charlayout, 3, 1 )
GFXDECODE_END

static const cassette_interface ibm5150_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED),
	NULL,
	NULL
};

static const cassette_interface mc1502_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED),
	NULL,
	NULL
};

static MACHINE_CONFIG_START( ibmpcjr, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(ibmpcjr, ibmpcjr, I8088, 4900000, pcjr_frame_interrupt) /* TODO: Get correct cpu frequency, probably XTAL_14_31818MHz/3 */

	MCFG_MACHINE_START_OVERRIDE(pc_state,pcjr)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pcjr)

	MCFG_PIT8253_ADD( "pit8253", pcjr_pit8253_config )

	MCFG_PIC8259_ADD( "pic8259", pcjr_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", pcjr_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], ibm5150_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_pcjr )
	MCFG_GFXDECODE(ibmpcjr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
	MCFG_SOUND_ADD("sn76496", SN76496, XTAL_14_31818MHz/4)
	MCFG_SOUND_CONFIG(psg_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_NVRAM_HANDLER( tandy1000 )

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	/* cassette */
	MCFG_CASSETTE_ADD( CASSETTE_TAG, ibm5150_cassette_interface )

	MCFG_UPD765A_ADD("upd765", false, false)

	MCFG_FLOPPY_DRIVE_ADD("upd765:0", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart1")
	MCFG_CARTSLOT_INTERFACE("ibmpcjr_cart")
	MCFG_CARTSLOT_EXTENSION_LIST("jrc")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(pcjr_cartridge)
	MCFG_CARTSLOT_ADD("cart2")
	MCFG_CARTSLOT_INTERFACE("ibmpcjr_cart")
	MCFG_CARTSLOT_EXTENSION_LIST("jrc")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(pcjr_cartridge)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","ibmpcjr_cart")
	MCFG_SOFTWARE_LIST_ADD("flop_list","ibmpcjr_flop")
MACHINE_CONFIG_END

static GFXDECODE_START( ibmpcjx )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc_8_charlayout, 3, 1 )
	GFXDECODE_ENTRY( "kanji", 0x0000, kanji_layout, 3, 1 )
GFXDECODE_END


static MACHINE_CONFIG_DERIVED( ibmpcjx, ibmpcjr )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ibmpcjx_map)
	MCFG_CPU_IO_MAP(ibmpcjx_io)

	MCFG_DEVICE_REMOVE("upd765:0");
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", ibmpc_floppies, "35dd", 0, pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", ibmpc_floppies, "35dd", 0, pc_state::floppy_formats)

	MCFG_GFXDECODE(ibmpcjx)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( mc1502, pc_state )
	/* basic machine hardware */
//  MCFG_CPU_PC(mc1502, mc1502, I8088, XTAL_16MHz/3, pcjr_frame_interrupt)  /* check frame_interrupt */
	MCFG_CPU_ADD("maincpu", I8088, XTAL_16MHz/3)
	MCFG_CPU_PROGRAM_MAP(mc1502_map)
	MCFG_CPU_IO_MAP(mc1502_io)
	MCFG_CPU_CONFIG(i86_address_mask)

	MCFG_MACHINE_START_OVERRIDE(pc_state,mc1502)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", mc1502_pit8253_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", mc1502_ppi8255_interface )       /* not complete */
	MCFG_I8255_ADD( "ppi8255n2", mc1502_ppi8255_interface_2 )   /* not complete */

	MCFG_I8251_ADD( "upd8251", default_i8251_interface )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_mc1502 )             /* only 1 chargen, CGA_FONT dip always 1 */
	MCFG_GFXDECODE(ibmpcjr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* printer */
//  MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)             /* TODO: non-standard */

	/* cassette */
	MCFG_CASSETTE_ADD( CASSETTE_TAG, mc1502_cassette_interface )    // has no motor control

	MCFG_FD1793x_ADD("vg93", XTAL_8MHz / 8) // clock?
	MCFG_FLOPPY_DRIVE_ADD("fd0", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("608K")                   /* 96 base + 512 on expansion card */
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( ec1841, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(ec1841, ec1841, I8086, 4096000, pc_frame_interrupt) // correct but slow
//  MCFG_CPU_PC(ec1841, ec1841, I8086, 4772720, pc_frame_interrupt)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	// maybe XTAL_12_288MHz
	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

//  MCFG_I8251_ADD( "upd8251_0", default_i8251_interface )  // modeled after BSC adapter?
//  MCFG_I8251_ADD( "upd8251_1", default_i8251_interface )

	/* video hardware -- supports font uploads */
	MCFG_FRAGMENT_ADD( pcvideo_cga )
	MCFG_GFXDECODE(ibm5150)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)

	/* keyboard -- needs dump */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* internal ram -- up to 4 banks of 512K */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
//  MCFG_RAM_EXTRA_OPTIONS("640K,1024K,1576K,2048K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( iskr1031, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(iskr1031, iskr1031, I8086, 4772720, pc_frame_interrupt)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_2", ibm5150_com_interface[2], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_3", ibm5150_com_interface[3], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport2", ibm5150_serport_config[2], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport3", ibm5150_serport_config[3], ibm5150_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_cga )
	MCFG_GFXDECODE(ibm5150)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( iskr3104, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(pc16, pc16, I8086, 4772720, pc_frame_interrupt)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_2", ibm5150_com_interface[2], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_3", ibm5150_com_interface[3], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport2", ibm5150_serport_config[2], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport3", ibm5150_serport_config[3], ibm5150_com, NULL, NULL )

	/* video hardware */
//  MCFG_FRAGMENT_ADD( pcvideo_ega ) // Put this back after ISA are added to this driver
	MCFG_FRAGMENT_ADD( pcvideo_cga )
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( poisk2, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(pc16, pc16, I8086, 4772720, pc_frame_interrupt)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_2", ibm5150_com_interface[2], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_3", ibm5150_com_interface[3], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport2", ibm5150_serport_config[2], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport3", ibm5150_serport_config[3], ibm5150_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_poisk2 )
	MCFG_GFXDECODE(ibm5150)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( zenith, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(zenith, pc8, I8088, XTAL_14_31818MHz/3, pc_frame_interrupt) /* 4,77 MHz */

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_2", ibm5150_com_interface[2], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_3", ibm5150_com_interface[3], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport2", ibm5150_serport_config[2], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport3", ibm5150_serport_config[3], ibm5150_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_cga )
	MCFG_GFXDECODE(ibm5150)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( olivetti, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(pc16, pc16, I8086, 8000000, pc_vga_frame_interrupt)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_2", ibm5150_com_interface[2], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_3", ibm5150_com_interface[3], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport2", ibm5150_serport_config[2], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport3", ibm5150_serport_config[3], ibm5150_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_cga )
	MCFG_GFXDECODE(ibm5150)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ibm5550, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(ibm5550, ibm5550, I8086, 8000000, pc_vga_frame_interrupt)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_2", ibm5150_com_interface[2], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_3", ibm5150_com_interface[3], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport2", ibm5150_serport_config[2], ibm5150_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport3", ibm5150_serport_config[3], ibm5150_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_cga )
	MCFG_GFXDECODE(ibm5150)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270, NULL)

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", 0, pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( olivm15, pc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, 4772720)
	MCFG_CPU_PROGRAM_MAP(oliv_map)
	MCFG_CPU_IO_MAP(oliv_io)
MACHINE_CONFIG_END


#if 0
	//pcjr roms? (incomplete dump, most likely 64 kbyte)
	// basic c1.20
	ROM_LOAD("basic.rom", 0xf6000, 0x8000, CRC(0c19c1a8))
	// ???
	ROM_LOAD("bios.rom", 0x??000, 0x2000, CRC(98463f95))

	/* turbo xt */
	/* basic c1.10 */
	ROM_LOAD("rom05.bin", 0xf6000, 0x2000, CRC(80d3cf5d))
	ROM_LOAD("rom04.bin", 0xf8000, 0x2000, CRC(673a4acc))
	ROM_LOAD("rom03.bin", 0xfa000, 0x2000, CRC(aac3fc37))
	ROM_LOAD("rom02.bin", 0xfc000, 0x2000, CRC(3062b3fc))
	/* sw1 0x60 readback fails write 301 to screen fe3b7 */
	/* disk problems no disk gives 601 */
	/* 5000-026 08/16/82 */
	ROM_LOAD("rom01.bin", 0xfe000, 0x2000, CRC(5c3f0256))

	/* anonymous works nice */
	ROM_LOAD("pcxt.rom",    0xfe000, 0x02000, CRC(031aafad))

	ROM_LOAD("bondwell.bin", 0xfe000, 0x2000, CRC(d435a405))

	/* europc */
	ROM_LOAD("50145", 0xf8000, 0x8000, CRC(1775a11d)) // V2.07
//  ROM_LOAD("eurobios.bin", 0xf8000, 0x8000, CRC(52185223)) scrap
	/* cga, hercules character set */
	ROM_LOAD("50146", 0x00000, 0x02000, CRC(1305dcf5)) //D1.0

	// ibm pc
	// most likely 8 kbyte chips
	ROM_LOAD("basicpc.bin", 0xf6000, 0x8000, CRC(ebacb791)) // IBM C1.1
	// split into 8 kbyte parts
	// the same as in the basic c1.10 as in the turboxt
	// 1501-476 10/27/82
	ROM_LOAD("biospc.bin", 0xfe000, 0x2000, CRC(e88792b3))

	/* tandy 1000 hx */
	ROM_LOAD("tandy1t.rom", 0xf0000, 0x10000, CRC(d37a1d5f))

	// ibm xt
	ROM_LOAD("xthdd.c8", 0xc8000, 0x2000, CRC(a96317da))
	ROM_LOAD("biosxt.bin", 0xf0000, 0x10000, CRC(36c32fde)) // BASIC C1.1
	// split into 2 chips for 16 bit access
	ROM_LOAD_EVEN("ibmxt.0", 0xf0000, 0x8000, CRC(83727c42))
	ROM_LOAD_ODD("ibmxt.1", 0xf0000, 0x8000, CRC(2a629953))

	/* pc xt mfm controller
	   2 harddisks 17 sectors, 4 head, 613 tracks
	   serves 2 controllers? 0x320-3, 0x324-7, dma 3, irq5
	   movable, works at 0xee000 */
	/* western digital 06/28/89 */
	ROM_LOAD("wdbios.rom",  0xc8000, 0x02000, CRC(8e9e2bd4) SHA1(601d7ceab282394ebab50763c267e915a6a2166a)) /* WDC IDE Superbios 2.0 (06/28/89) Expansion Rom C8000-C9FFF  */

	/* lcs 6210d asic i2.1 09/01/1988 */
	/* problematic, currently showing menu and calls int21 (hangs)! */
	ROM_LOAD("xthdd.rom",  0xc8000, 0x02000, CRC(a96317da))
#endif


ROM_START( bw230 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("bondwell.bin", 0xfe000, 0x2000, CRC(d435a405) SHA1(a57c705d1144c7b61940b6f5c05d785c272fc9bb))

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( zdsupers )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "v31d", "v3.1d" )
	ROMX_LOAD( "z184m v3.1d.10d", 0xf8000, 0x8000, CRC(44012c3b) SHA1(f2f28979798874386ca8ba3dd3ead24ae7c2aeb4), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v29e", "v2.9e" )
	ROMX_LOAD( "z184m v2.9e.10d", 0xf8000, 0x8000, CRC(de2f200b) SHA1(ad5ce601669a82351e412fc6c1c70c47779a1e55), ROM_BIOS(2))
	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( europc )
	ROM_REGION(0x100000,"maincpu", 0)
	// hdd bios integrated!
	ROM_LOAD("50145", 0xf8000, 0x8000, CRC(1775a11d) SHA1(54430d4d0462860860397487c9c109e6f70db8e3)) // V2.07
	ROM_REGION(0x08100,"gfx1", 0)
	ROM_LOAD("50146 char d1.0 euro.u16", 0x00000, 0x02000, CRC(1305dcf5) SHA1(aca488a16ae4ff05a1f4d14574379ff49cd48343)) //D1.0
ROM_END


ROM_START( ibmpcjr )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("bios.rom", 0xf0000, 0x10000,CRC(31e3a7aa) SHA1(1f5f7013f18c08ff50d7942e76c4fbd782412414))

	ROM_REGION(0x08100,"gfx1", 0)
	ROM_LOAD("cga.chr",     0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd)) // from an unknown clone cga card
ROM_END

ROM_START( ibmpcjx )
	ROM_REGION(0x100000,"maincpu", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("unk")
	ROM_SYSTEM_BIOS( 0, "5601jda", "5601jda" )
	ROMX_LOAD("5601jda.bin", 0xf0000, 0x10000, CRC(b1e12366) SHA1(751feb16b985aa4f1ec1437493ff77e2ebd5e6a6), ROM_BIOS(1))
	ROMX_LOAD("basicjx.rom",   0xe8000, 0x08000, NO_DUMP, ROM_BIOS(1)) // boot fails due of this.
	ROM_SYSTEM_BIOS( 1, "unk", "unk" )
	ROMX_LOAD("ipljx.rom", 0xe0000, 0x20000, CRC(36a7b2de) SHA1(777db50c617725e149bca9b18cf51ce78f6dc548), ROM_BIOS(2))
	ROM_FILL(0xff195, 1, 0x20) // the bios has a bug that causes an interrupt
	ROM_FILL(0xff196, 1, 0x06) // to arrive before a flag is set causing the boot to hang
	ROM_FILL(0xff197, 1, 0x84) // technically this should be fixed in the PIC by delaying
	ROM_FILL(0xff198, 1, 0x04) // sending an irq after the mask is changed but there is
	ROM_FILL(0xff199, 1, 0xe6) // a strong possiblility that will cause problems with later
	ROM_FILL(0xff19a, 1, 0x21) // faster x86 machines.

	ROM_REGION(0x08100,"gfx1", 0) //TODO: needs a different charset
	ROM_LOAD("cga.chr",     0x00000, 0x01000, BAD_DUMP CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd)) // from an unknown clone cga card

	ROM_REGION(0x38000,"kanji", 0)
	ROM_LOAD("kanji.rom",     0x00000, 0x38000, BAD_DUMP CRC(eaa6e3c3) SHA1(35554587d02d947fae8446964b1886fff5c9d67f)) // hand-made rom
ROM_END

#ifdef UNUSED_DEFINITION
ROM_START( t1000 )
	ROM_REGION(0x100000,"maincpu", 0)
	// partlist says it has 1 128kbyte rom
	ROM_LOAD("t1000hx.e0", 0xe0000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_SYSTEM_BIOS( 0, "v010000", "v010000" )
	ROMX_LOAD("v010000.f0", 0xf0000, 0x10000, NO_DUMP, ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v010100", "v010100" )
	ROMX_LOAD("v010100.f0", 0xf0000, 0x10000, CRC(b6760881) SHA1(8275e4c48ac09cf36685db227434ca438aebe0b9), ROM_BIOS(2))
	ROM_REGION(0x08000,"gfx1", 0)
	// expects 8x9 charset!
	ROM_LOAD("50146", 0x00000, 0x02000, BAD_DUMP CRC(1305dcf5) SHA1(aca488a16ae4ff05a1f4d14574379ff49cd48343)) //taken from europc, 9th blank
ROM_END

ROM_START( t1000a )
	ROM_REGION(0x100000,"maincpu", 0)
	// partlist says it has 1 128kbyte rom
	ROM_LOAD("t1000hx.e0", 0xe0000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_LOAD("v010100.f0", 0xf0000, 0x10000, CRC(b6760881) SHA1(8275e4c48ac09cf36685db227434ca438aebe0b9))
	ROM_REGION(0x08000,"gfx1", 0)
	// expects 8x9 charset!
	ROM_LOAD("50146", 0x00000, 0x02000, BAD_DUMP CRC(1305dcf5) SHA1(aca488a16ae4ff05a1f4d14574379ff49cd48343)) //taken from europc, 9th blank
ROM_END

ROM_START( t1000ex )
	ROM_REGION(0x100000,"maincpu", 0)
	// partlist says it has 1 128kbyte rom
	ROM_LOAD("t1000hx.e0", 0xe0000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_LOAD("v010200.f0", 0xf0000, 0x10000, CRC(0e016ecf) SHA1(2f5ac8921b7cba56b02122ef772f5f11bbf6d8a2))
	ROM_REGION(0x08000,"gfx1", 0)
	// expects 8x9 charset!
	ROM_LOAD("50146", 0x00000, 0x02000, BAD_DUMP CRC(1305dcf5) SHA1(aca488a16ae4ff05a1f4d14574379ff49cd48343)) //taken from europc, 9th blank
ROM_END
#endif

ROM_START( t1000hx )
	ROM_REGION(0x100000,"maincpu", 0)
	// partlist says it has 1 128kbyte rom
	ROM_LOAD("t1000hx.e0", 0xe0000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))
	ROM_LOAD("v020000.f0", 0xf0000, 0x10000, CRC(d37a1d5f) SHA1(5ec031c31a7967cc3fd53a535d81833e4a1c385e))
	ROM_REGION(0x08000,"gfx1", 0)
	// expects 8x9 charset!
	ROM_LOAD("50146", 0x00000, 0x02000, BAD_DUMP CRC(1305dcf5) SHA1(aca488a16ae4ff05a1f4d14574379ff49cd48343)) //taken from europc, 9th blank
ROM_END

#ifdef UNUSED_DEFINITION
ROM_START( t1000sl )
	ROM_REGION(0x100000,"maincpu", 0)
	// partlist says it has 1 128kbyte rom
	ROM_LOAD("t1000hx.e0", 0xe0000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_SYSTEM_BIOS( 0, "v010400", "v010400" )
	ROMX_LOAD("v010400.f0", 0xf0000, 0x10000, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v010401", "v010401" )
	ROMX_LOAD("v010401.f0", 0xf0000, 0x10000, NO_DUMP, ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "v010402", "v010402" )
	ROMX_LOAD("v010402.f0", 0xf0000, 0x10000, NO_DUMP, ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "v020001", "v020001" )
	ROMX_LOAD("v020001.f0", 0xf0000, 0x10000, NO_DUMP, ROM_BIOS(4) )
	ROM_REGION(0x08000,"gfx1", 0)
	// expects 8x9 charset!
	ROM_LOAD("50146", 0x00000, 0x02000, BAD_DUMP CRC(1305dcf5) SHA1(aca488a16ae4ff05a1f4d14574379ff49cd48343)) //taken from europc, 9th blank
ROM_END

ROM_START( t1000sl2 )
	ROM_REGION(0x100000,"maincpu", 0)
	// partlist says it has 1 128kbyte rom
	ROM_LOAD("t1000hx.e0", 0xe0000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_LOAD("v010404.f0", 0xf0000, 0x10000, NO_DUMP )
	ROM_REGION(0x08000,"gfx1", 0)
	// expects 8x9 charset!
	ROM_LOAD("50146", 0x00000, 0x02000, BAD_DUMP CRC(1305dcf5) SHA1(aca488a16ae4ff05a1f4d14574379ff49cd48343)) //taken from europc, 9th blank
ROM_END
#endif

ROM_START( t1000sx )
	ROM_REGION(0x100000,"maincpu", 0)
	// partlist says it has 1 128kbyte rom
	ROM_LOAD("t1000hx.e0", 0xe0000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_LOAD("v010200.f0", 0xf0000, 0x10000, CRC(0e016ecf) SHA1(2f5ac8921b7cba56b02122ef772f5f11bbf6d8a2))
	ROM_REGION(0x08000,"gfx1", 0)
	// expects 8x9 charset!
	ROM_LOAD("50146", 0x00000, 0x02000, BAD_DUMP CRC(1305dcf5) SHA1(aca488a16ae4ff05a1f4d14574379ff49cd48343)) //taken from europc, 9th blank
ROM_END


ROM_START( t1000tx )
	ROM_REGION(0x100000,"maincpu", 0)
	/* There might be a second 32KB rom, but it seems to work fine with just this one */
	ROM_LOAD("t1000tx.bin", 0xf8000, 0x8000, CRC(9b34765c) SHA1(0b07e87f6843393f7d4ca4634b832b0c0bec304e))

	ROM_REGION(0x08000,"gfx1", 0)
	// expects 8x9 charset!
	ROM_LOAD("50146", 0x00000, 0x02000, BAD_DUMP CRC(1305dcf5) SHA1(aca488a16ae4ff05a1f4d14574379ff49cd48343)) //taken from europc, 9th blank
ROM_END


ROM_START( t1000rl )
	ROM_REGION(0x100000,"maincpu", 0)

	/* v2.0.0.1 */
	/* Rom is labeled "(C) TANDY CORP. 1990 // 8079073 // LH534G70 JAPAN // 9034 D" */
	ROM_LOAD("8079073.u23", 0x00000, 0x80000, CRC(6fab50f7) SHA1(2ccc02bee4c250dc1b7c17faef2590bc158860b0) )

	ROM_REGION(0x08000,"gfx1", 0)
	// expects 8x9 charset!
	/* Character rom located at U3 w/label "8079027 // NCR // 609-2495004 // F841030 A9025" */
	ROM_LOAD("50146", 0x00000, 0x02000, BAD_DUMP CRC(1305dcf5) SHA1(aca488a16ae4ff05a1f4d14574379ff49cd48343)) //taken from europc, 9th blank
ROM_END

ROM_START( t1000tl2 )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "t10000tl2.bin", 0xf0000, 0x10000, CRC(e288f12c) SHA1(9d54ccf773cd7202c9906323f1b5a68b1b3a3a67))

		ROM_REGION(0x08000,"gfx1", 0)
	// expects 8x9 charset!
	/* Character rom located at U3 w/label "8079027 // NCR // 609-2495004 // F841030 A9025" */
	ROM_LOAD("50146", 0x00000, 0x02000, BAD_DUMP CRC(1305dcf5) SHA1(aca488a16ae4ff05a1f4d14574379ff49cd48343)) //taken from europc, 9th blank
ROM_END

ROM_START( dgone )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "dgone.bin",  0xf8000, 0x08000, CRC(2c38c86e) SHA1(c0f85a000d1d13cd354965689e925d677822549e))

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( iskr1031 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v1", "v1")
	ROMX_LOAD( "150-02.bin", 0xfc000, 0x2000, CRC(e33fb974) SHA1(f5f3ece67c025c0033716ff516e1a34fbeb32749), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "150-03.bin", 0xfc001, 0x2000, CRC(8c482258) SHA1(90ef48955e0df556dc06a000a797ef42ccf430c5), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v2", "v2")
	ROMX_LOAD( "150-06.bin", 0xfc000, 0x2000, CRC(1adbf969) SHA1(08c0a0fc50a75e6207b1987bae389cca60893eac), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "150-07.bin", 0xfc001, 0x2000, CRC(0dc4b65a) SHA1(c96f066251a7343eac8113ea9dcb2cb12d0334d5), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD( "iskra-1031_font.bin", 0x0000, 0x2000, CRC(f4d62e80) SHA1(ad7e81a0c9abc224671422bbcf6f6262da92b510))
ROM_END

ROM_START( iskr1030m )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROMX_LOAD( "iskra-1030m_0.rom", 0xfc000, 0x2000, CRC(0d698e19) SHA1(2fe117c9f4f8c4b59085d5a41f919d743c425fdd), ROM_SKIP(1))
	ROMX_LOAD( "iskra-1030m_1.rom", 0xfc001, 0x2000, CRC(fe808337) SHA1(b0b7ebe14324ada8aa9a6926a82b18e80f78a257), ROM_SKIP(1))
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD( "iskra-1030m.chr", 0x0000, 0x2000, CRC(50b162eb) SHA1(5bd7cb1705a69bd16115a4c9ed1c2748a5c8ad51))
ROM_END

ROM_START( ec1840 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v4", "EC-1840.04")
	ROMX_LOAD( "000-04-971b.bin", 0xfe000, 0x0800, CRC(06aeaee8) SHA1(9f954e4c48156d573a8e0109e7ca652be9e6036a), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "001-04-92b7.bin", 0xff000, 0x0800, CRC(3fae650a) SHA1(c98b777fdeceadd72d6eb9465b3501b9ead55a08), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "002-04-9e17.bin", 0xfe001, 0x0800, CRC(d59712df) SHA1(02ea1b3ae9662f5c64c58920a32ca9db0f6fbd12), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "003-04-3ccb.bin", 0xff001, 0x0800, CRC(7fc362c7) SHA1(538e13639ad2b4c30bd72582e323181e63513306), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
ROM_END

ROM_START( ec1841 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v1", "EC-1841.01")
	ROMX_LOAD( "012-01-3107.bin", 0xfc000, 0x0800, CRC(77957396) SHA1(785f1dceb6e2b4618f5c5f0af15eb74a8c951448), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "013-01-203f.bin", 0xfc001, 0x0800, CRC(768bd3d5) SHA1(2e948f2ad262de306d889b7964c3f1aad45ff5bc), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "014-01-fa40.bin", 0xfd000, 0x0800, CRC(47722b58) SHA1(a6339ee8af516f834826b7828a5cf79cb650480c), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "015-01-bf1d.bin", 0xfd001, 0x0800, CRC(b585b5ea) SHA1(d0ebed586eb13031477c2e071c50416682f80489), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "016-01-65f7.bin", 0xfe000, 0x0800, CRC(28a07db4) SHA1(17fbcd60dacd1d3f8d8355db429f97e4d1d1ac88), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "017-01-5be1.bin", 0xfe001, 0x0800, CRC(928bda26) SHA1(ee889184067e2680b29a8ef1c3a76cf5afd4c78d), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "018-01-7090.bin", 0xff000, 0x0800, CRC(75ca7d7e) SHA1(6356426820c5326a7893a437d54b02f250ef8609), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "019-01-0492.bin", 0xff001, 0x0800, CRC(8a9d593e) SHA1(f3936d2cb4e6d130dd732973f126c3aa20612463), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS(1, "v2", "EC-1841.02")
	ROMX_LOAD( "012-02-37f6.bin", 0xfc000, 0x0800, CRC(8f5c6a20) SHA1(874b62f9cee8d3b974f33732f94eff10fc002c44), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "013-02-2552.bin", 0xfc001, 0x0800, CRC(e3c10128) SHA1(d6ed743ebe9c130925c9f17aad1a45db9194c967), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "014-02-0fbe.bin", 0xfd000, 0x0800, CRC(f8517e5e) SHA1(8034cd6ff5778365dc9daa494524f1753a74f1ed), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "015-02-d736.bin", 0xfd001, 0x0800, CRC(8538c52a) SHA1(ee981ce90870b6546a18f2a2e64d71b0038ce0dd), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "016-02-5b2c.bin", 0xfe000, 0x0800, CRC(3d1d1e67) SHA1(c527e29796537787c0f6c329f3c203f6131ca77f), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "017-02-4b9d.bin", 0xfe001, 0x0800, CRC(1b985264) SHA1(5ddcb9c13564be208c5068c105444a87159c67ee), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "018-02-7090.bin", 0xff000, 0x0800, CRC(75ca7d7e) SHA1(6356426820c5326a7893a437d54b02f250ef8609), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "019-02-0493.bin", 0xff001, 0x0800, CRC(61aae23d) SHA1(7b3aa24a63ee31b194297eb1e61c3827edfcb95a), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_SYSTEM_BIOS(2, "v3", "EC-1841.03")
	ROMX_LOAD( "012-03-37e7.bin", 0xfc000, 0x0800, CRC(49992bd5) SHA1(119121e1b4af1c44b9b8c2edabe7dc1d3019c4a6), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "013-03-2554.bin", 0xfc001, 0x0800, CRC(834bd7d7) SHA1(e37514fc4cb8a5cbe68e7564e0e07d5116c4021a), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "014-03-0fbe.bin", 0xfd000, 0x0800, CRC(f8517e5e) SHA1(8034cd6ff5778365dc9daa494524f1753a74f1ed), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "015-03-d736.bin", 0xfd001, 0x0800, CRC(8538c52a) SHA1(ee981ce90870b6546a18f2a2e64d71b0038ce0dd), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "016-03-5b2c.bin", 0xfe000, 0x0800, CRC(3d1d1e67) SHA1(c527e29796537787c0f6c329f3c203f6131ca77f), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "017-03-4b9d.bin", 0xfe001, 0x0800, CRC(1b985264) SHA1(5ddcb9c13564be208c5068c105444a87159c67ee), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "018-03-7090.bin", 0xff000, 0x0800, CRC(75ca7d7e) SHA1(6356426820c5326a7893a437d54b02f250ef8609), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "019-03-0493.bin", 0xff001, 0x0800, CRC(61aae23d) SHA1(7b3aa24a63ee31b194297eb1e61c3827edfcb95a), ROM_SKIP(1) | ROM_BIOS(3))

	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
ROM_END

ROM_START( ec1845 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROMX_LOAD( "184500.bin", 0xfc000, 0x0800, CRC(7c472ef7) SHA1(3af53f27b49bbc731bf51f9300fbada23a1bfcfc), ROM_SKIP(1))
	ROMX_LOAD( "184501.bin", 0xfc001, 0x0800, CRC(db240dc6) SHA1(d7bb022213d09bbf2a8107fe4f1cd27b23939e18), ROM_SKIP(1))
	ROMX_LOAD( "184502.bin", 0xfd000, 0x0800, CRC(149e7e29) SHA1(7f2a297588fef1bc750c57e6ae0d5acf3d27c486), ROM_SKIP(1))
	ROMX_LOAD( "184503.bin", 0xfd001, 0x0800, CRC(e28cbd74) SHA1(cf1fba4e67c8e1dd8cdda547118e84b704029b03), ROM_SKIP(1))
	ROMX_LOAD( "184504.bin", 0xfe000, 0x0800, CRC(55fa7a1d) SHA1(58f7abab08b9d2f0a1c1636e11bb72af2694c95f), ROM_SKIP(1))
	ROMX_LOAD( "184505.bin", 0xfe001, 0x0800, CRC(c807e3f5) SHA1(08117e449f0d04f96041cff8d34893f500f3760d), ROM_SKIP(1))
	ROMX_LOAD( "184506.bin", 0xff000, 0x0800, CRC(24f5c27c) SHA1(7822dd7f715ef00ccf6d8408be8bbfe01c2eba20), ROM_SKIP(1))
	ROMX_LOAD( "184507.bin", 0xff001, 0x0800, CRC(75122203) SHA1(7b0fbdf1315230633e39574ac7360163bc7361e1), ROM_SKIP(1))
	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
ROM_END


ROM_START( mk88 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_DEFAULT_BIOS("v392")
	ROM_SYSTEM_BIOS(0, "v290", "v2.90")
	ROMX_LOAD( "mk88m.bin", 0xfc000, 0x2000, CRC(09c9da3b) SHA1(d1e7ad23b5f5b3576ad128c1198294129754f39f), ROM_BIOS(1))
	ROMX_LOAD( "mk88b.bin", 0xfe000, 0x2000, CRC(8a922476) SHA1(c19c3644ab92fd12e13f32b410cd26e3c844a03b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v391", "v3.91")
	ROMX_LOAD( "mkm.bin", 0xfc000, 0x2000, CRC(65f979e8) SHA1(13e85be9bc8ceb5ab9e559e7d0089e26fbbb84fc), ROM_BIOS(2))
	ROMX_LOAD( "mkb.bin", 0xfe000, 0x2000, CRC(830a0447) SHA1(11bc200fdbcfbbe335f4c282020750c0b5ca4167), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v392", "v3.92")
	ROMX_LOAD( "m88.bin", 0xfc000, 0x2000, CRC(fe1b4e36) SHA1(fcb420af0ff09a7d43fcb9b7d0b0233a2071c159), ROM_BIOS(3))
	ROMX_LOAD( "b88.bin", 0xfe000, 0x2000, CRC(58a418df) SHA1(216398d4e4302ee7efcc2c8f9ff9d8a1161229ea), ROM_BIOS(3))
	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	// Here CGA rom with cyrillic support should be added
ROM_END

ROM_START( iskr3104 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROMX_LOAD( "198.bin", 0xfc000, 0x2000, CRC(bcfd8e41) SHA1(e21ddf78839aa51fa5feb23f511ff5e2da31b433),ROM_SKIP(1))
	ROMX_LOAD( "199.bin", 0xfc001, 0x2000, CRC(2da5fe79) SHA1(14d5dccc141a0b3367f7f8a7188306fdf03c2b6c),ROM_SKIP(1))
	// EGA card from Iskra-3104
	ROMX_LOAD( "143-03.bin", 0xc0001, 0x2000, CRC(d0706345) SHA1(e04bb40d944426a4ae2e3a614d3f4953d7132ede),ROM_SKIP(1))
	ROMX_LOAD( "143-02.bin", 0xc0000, 0x2000, CRC(c8c18ebb) SHA1(fd6dac76d43ab8b582e70f1d5cc931d679036fb9),ROM_SKIP(1))

	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
ROM_END

ROM_START( poisk1 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_LOAD( "b_hd_v11.rf2", 0xc8000, 0x0800, CRC(a19c39b2) SHA1(57faa56b320abf801fedbed578cf97d253e5b777)) // HDD controller ver 1.1
	ROM_LOAD( "b942_5mb.bin", 0x00000, 0x0800, CRC(a3cfa240) SHA1(0b0aa1ce839a957153bfbbe70310480ca9fe21b6)) // HDD controller ver 1.4

	ROM_LOAD( "b_ngmd_n.rf2", 0x0000, 0x0800, CRC(967e172a) SHA1(95117c40fd9f624fee08ccf37f615b16ff249688)) // Floppy
	ROM_LOAD( "b_ngmd_t.rf2", 0x0000, 0x0800, CRC(630010b1) SHA1(50876fe4f5f4f32a242faa70f9154574cd315ec4)) // Floppy
	ROM_SYSTEM_BIOS(0, "v89", "1989")
	ROMX_LOAD( "biosp1s.rf4",    0xfe000, 0x2000, CRC(1a85f671) SHA1(f0e59b2c4d92164abca55a96a58071ce869ff988), ROM_BIOS(1)) // Main BIOS
	ROM_SYSTEM_BIOS(1, "v91", "1991")
	ROMX_LOAD( "poisk_1991.bin", 0xfe000, 0x2000, CRC(d61c56fd) SHA1(de202e1f7422d585a1385a002a4fcf9d756236e5), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "t1", "Test I/O")
	ROMX_LOAD( "p1_t_i_o.rf4", 0xfe000, 0x2000, CRC(18a781de) SHA1(7267970ee27e3ea1d972bee8e74b17bac1051619), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "t2", "Test MB")
	ROMX_LOAD( "p1_t_pls.rf4", 0xfe000, 0x2000, CRC(c8210ffb) SHA1(f2d1a6c90e4708bcc56186b2fb906fa852667084), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "t3", "Test RAM")
	ROMX_LOAD( "p1_t_ram.rf4", 0xfe000, 0x2000, CRC(e42f5a61) SHA1(ce2554eae8f0d2b6d482890dd198cf7e2d29c655), ROM_BIOS(5))

	ROM_LOAD( "boot_net.rf4", 0x0000, 0x2000, CRC(316c2030) SHA1(d043325596455772252e465b85321f1b5c529d0b)) // NET BUIS
	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	ROM_LOAD( "poisk.cga", 0x0000, 0x0800, CRC(f6eb39f0) SHA1(0b788d8d7a8e92cc612d044abcb2523ad964c200))
ROM_END

ROM_START( poisk2 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v20", "v2.0")
	ROMX_LOAD( "b_p2_20h.rf4", 0xfc001, 0x2000, CRC(d53189b7) SHA1(ace40f1a40642b51fe5d2874acef81e48768b23b), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "b_p2_20l.rf4", 0xfc000, 0x2000, CRC(2d61fcc9) SHA1(11873c8741ba37d6c2fe1f482296aece514b7618), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v21", "v2.1")
	ROMX_LOAD( "b_p2_21h.rf4", 0xfc001, 0x2000, CRC(22197297) SHA1(506c7e63027f734d62ef537f484024548546011f), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "b_p2_21l.rf4", 0xfc000, 0x2000, CRC(0eb2ea7f) SHA1(67bb5fec53ebfa2a5cad2a3d3d595678d6023024), ROM_SKIP(1) | ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v24", "v2.4")
	ROMX_LOAD( "b_p2_24h.rf4", 0xfc001, 0x2000, CRC(ea842c9e) SHA1(dcdbf27374149dae0ef76d410cc6c615d9b99372), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "b_p2_24l.rf4", 0xfc000, 0x2000, CRC(02f21250) SHA1(f0b133fb4470bddf2f7bf59688cf68198ed8ce55), ROM_SKIP(1) | ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "v21d", "v2.1d")
	ROMX_LOAD( "opp2_1h.rf4", 0xfc001, 0x2000, CRC(b7cd7f4f) SHA1(ac473822fb44d7b898d628732cf0a27fcb4d26d6), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD( "opp2_1l.rf4", 0xfc000, 0x2000, CRC(1971dca3) SHA1(ecd61cc7952af834d8abc11db372c3e70775489d), ROM_SKIP(1) | ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "v22d", "v2.2d")
	ROMX_LOAD( "opp2_2h.rf4", 0xfc001, 0x2000, CRC(b9e3a5cc) SHA1(0a28afbff612471ee81d69a98789e75253c57a30), ROM_SKIP(1) | ROM_BIOS(5))
	ROMX_LOAD( "opp2_2l.rf4", 0xfc000, 0x2000, CRC(6877aad6) SHA1(1d0031d044beb4f9f321e3c8fdedf57467958900), ROM_SKIP(1) | ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "v23d", "v2.3d")
	ROMX_LOAD( "opp2_3h.rf4", 0xfc001, 0x2000, CRC(ac7d4f06) SHA1(858d6e084a38814280b3e29fb54971f4f532e484), ROM_SKIP(1) | ROM_BIOS(6))
	ROMX_LOAD( "opp2_3l.rf4", 0xfc000, 0x2000, CRC(3c877ea1) SHA1(0753168659653538311c0ad1df851cbbdba426f4), ROM_SKIP(1) | ROM_BIOS(6))
	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	ROM_LOAD( "p2_ecga.rf4", 0x0000, 0x2000, CRC(d537f665) SHA1(d70f085b9b0cbd53df7c3122fbe7592998ba8fed))
ROM_END

ROM_START( mc1702 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_LOAD16_BYTE( "2764_2_(573rf4).rom", 0xfc000,  0x2000, CRC(34a0c8fb) SHA1(88dc247f2e417c2848a2fd3e9b52258ad22a2c07))
	ROM_LOAD16_BYTE( "2764_3_(573rf4).rom", 0xfc001, 0x2000, CRC(68ab212b) SHA1(f3313f77392877d28ce290ffa3432f0a32fc4619))
	ROM_LOAD( "ba1m_(573rf5).rom", 0x0000, 0x0800, CRC(08d938e8) SHA1(957b6c691dbef75c1c735e8e4e81669d056971e4))

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END


ROM_START( mc1502 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_DEFAULT_BIOS("v52")
	ROM_LOAD( "basic.rom",        0xe8000, 0x8000, CRC(173d69fa) SHA1(003f872e12f00800e22ab6bbc009d36bfde67b9d))
	ROM_SYSTEM_BIOS(0, "v50", "v5.0")
	ROMX_LOAD( "monitor_5_0.rom",  0xfc000, 0x4000, CRC(9e97c6a0) SHA1(16a304e8de69ec4d8b92acda6bf28454c361a24f),ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v52", "v5.2")
	ROMX_LOAD( "monitor_5_2.rom",  0xfc000, 0x4000, CRC(0e65491e) SHA1(8a4d556473b5e0e59b05fab77c79c29f4d562412),ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v531", "v5.31")
	ROMX_LOAD( "monitor_5_31.rom", 0xfc000, 0x4000, CRC(a48295d5) SHA1(6f38977c22f9cc6c2bc6f6e53edc4048ca6b6721),ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "v533", "v5.33")
	ROMX_LOAD( "0_(cbc0).bin", 0xfc000, 0x2000, CRC(9a55bc4f) SHA1(81da44eec2e52cf04b1fc7053502270f51270590),ROM_BIOS(4))
	ROMX_LOAD( "1_(dfe2).bin", 0xfe000, 0x2000, CRC(8dec077a) SHA1(d6f6d7cc2183abc77fbd9cd59132de5766f7c458),ROM_BIOS(4))
	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	ROM_LOAD( "symgen.rom", 0x0000, 0x2000, CRC(b2747a52) SHA1(6766d275467672436e91ac2997ac6b77700eba1e))
ROM_END

ROM_START( m24 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROMX_LOAD("olivetti_m24_version_1.43_high.bin",0xfc001, 0x2000, CRC(04e697ba) SHA1(1066dcc849e6289b5ac6372c84a590e456d497a6), ROM_SKIP(1))
	ROMX_LOAD("olivetti_m24_version_1.43_low.bin", 0xfc000, 0x2000, CRC(ff7e0f10) SHA1(13423011a9bae3f3193e8c199f98a496cab48c0f), ROM_SKIP(1))

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( m240 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROMX_LOAD("olivetti_m240_pch5_2.04_high.bin", 0xf8001, 0x4000, CRC(ceb97b59) SHA1(84fabbeab355e0a4c9445910f2b7d1ec98886642), ROM_SKIP(1))
	ROMX_LOAD("olivetti_m240_pch6_2.04_low.bin",  0xf8000, 0x4000, CRC(c463aa94) SHA1(a30c763c1ace9f3ff79e7136b252d624108a50ae), ROM_SKIP(1))

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( ibm5550 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_LOAD( "ipl5550.rom", 0xfc000, 0x4000, CRC(40cf34c9) SHA1(d41f77fdfa787b0e97ed311e1c084b8699a5b197))

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0) /* original font rom is undumped */
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, BAD_DUMP CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( pc7000 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROMX_LOAD( "mitsubishi-m5l27128k-1.bin", 0xf8000, 0x4000, CRC(9683957f) SHA1(4569eab6d88eb1bba0d553d1358e593c326978aa), ROM_SKIP(1))
	ROMX_LOAD( "mitsubishi-m5l27128k-2.bin", 0xf8001, 0x4000, CRC(99b229a4) SHA1(5800c8bafed26873d8cfcc79a05f93a780a31c91), ROM_SKIP(1))

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( olivm15 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "oliv_m15.bin",0xfc000, 0x04000, CRC(bf2ef795) SHA1(02d497131f5ca2c78f2accd38ab0eab6813e3ebf))
ROM_END

// Siemens PC-D (80186)
ROM_START( pcd )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD16_BYTE( "sni_pcd1.bin", 0xfc001, 0x2000, CRC(e20244dd) SHA1(0ebc5ddb93baacd9106f1917380de58aac64fe73))
	ROM_LOAD16_BYTE( "sni_pcd2.bin", 0xfc000, 0x2000, CRC(e03db2ec) SHA1(fcae8b0c9e7543706817b0a53872826633361fda))
	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0) /* original font rom is undumped */
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, BAD_DUMP CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( olypeopl )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD16_BYTE( "uo1271c0.bin", 0xfe000, 0x1000, CRC(c9187bce) SHA1(464e1f96046657b49afa4223ede1040650643d58))
	ROM_LOAD16_BYTE( "uo1271d0.bin", 0xfe001, 0x1000, CRC(10e6437b) SHA1(0b77bb7a62f0a8240602f4cdcc3d6765e62894f4))
	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0) /* original font rom is undumped */
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, BAD_DUMP CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( sx16 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "tmm27128ad.bin",0xfc000, 0x4000, CRC(f8543362) SHA1(fef625e260ca89ba02174584bdc12db609f0780e))
	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( compc1 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "compc1.bin",0xfc000, 0x4000, CRC(75135d37) SHA1(177283642240fee191ba2d87e1d0c2a377c78ccb))
	ROM_REGION(0x8000,"gfx1", 0)
	ROM_LOAD("pc1_char.bin", 0x00000, 0x8000, CRC(4773a945) SHA1(bcc38abecc75d3f641d42987cb0d2ed71d71bc4c))
ROM_END

ROM_START( pc10iii )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "pc10iii_bios.bin",0xf8000, 0x8000, CRC(ae9e6a31) SHA1(853ee251cf230818c407a8d13ef060a21c90a8c1))
	ROM_REGION(0x8000,"gfx1", 0)
	ROM_LOAD("pc10iii_char.bin", 0x00000, 0x8000, CRC(b406651c) SHA1(856f58353391a74a06ebb8ec9f8333d7d69e5fd6))
ROM_END

ROM_START( mbc16 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "mbc16.bin", 0xfc000, 0x4000, CRC(f3e0934a) SHA1(e4b91c3d395be0414e20f23ad4919b8ac52639b2))
	ROM_REGION(0x2000,"gfx1", 0)
	//ATI Graphics Solution SR (graphics card, need to make it ISA card)
	ROM_LOAD( "atigssr.bin", 0x0000, 0x2000, CRC(aca81498) SHA1(0d84c89487ee7a6ac4c9e73fdb30c5fd8aa595f8))
	// override for now with IBM CGA card
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( ataripc3 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "c101701-004 308.u61",0xf8000, 0x8000, CRC(929a2443) SHA1(8e98f3c9180c55b1f5521727779c016083d27960))

	ROM_REGION(0x8000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, BAD_DUMP CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) // not the real character ROM

	ROM_REGION(0x8000,"plds", 0)
	ROM_LOAD( "c101681 6ffb.u60",0x000, 0x100, NO_DUMP ) // PAL20L10NC
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT      MACHINE     INPUT       INIT        COMPANY            FULLNAME */
COMP( 1984, dgone,      ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "Data General", "Data General/One" , GAME_NOT_WORKING)/* CGA, 2x 3.5" disk drives */
COMP( 1985, bw230,      ibm5150,    0,          pccga,      bondwell, pc_state,   bondwell,   "Bondwell Holding", "BW230 (PRO28 Series)", 0 )
COMP( 1988, europc,     ibm5150,    0,          europc,     europc, pc_state,     europc,     "Schneider Rdf. AG", "EURO PC", GAME_NOT_WORKING)
COMP( 1984, compc1,     ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "Commodore Business Machines", "Commodore PC-1" , GAME_NOT_WORKING)
COMP( 1987, pc10iii,    ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "Commodore Business Machines", "Commodore PC-10 III" , GAME_NOT_WORKING)

// pcjr (better graphics, better sound)
COMP( 1983, ibmpcjr,    ibm5150,    0,          ibmpcjr,    ibmpcjr,  pc_state,    pcjr,       "International Business Machines", "IBM PC Jr", GAME_IMPERFECT_COLORS )
COMP( 1985, ibmpcjx,    ibm5150,    0,          ibmpcjx,    ibmpcjr, pc_state,    pcjr,       "International Business Machines", "IBM PC JX", GAME_IMPERFECT_COLORS | GAME_NOT_WORKING)

// tandy 1000
COMP( 1987, t1000hx,    ibm5150,    0,          t1000hx,    tandy1t, pc_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 HX", 0)
COMP( 1987, t1000sx,    ibm5150,    0,          t1000sx,    tandy1t, pc_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 SX", GAME_NOT_WORKING)
COMP( 1987, t1000tx,    ibm5150,    0,          t1000_286,  tandy1t, pc_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 TX", 0)
COMP( 1989, t1000rl,    ibm5150,    0,          t1000_16,   tandy1t, pc_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 RL", 0)
COMP( 1989, t1000tl2,   ibm5150,    0,          t1000_286,  tandy1t, pc_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 TL/2", 0)

COMP( 1989, iskr1031,   ibm5150,    0,          iskr1031,   pccga, pc_state,      pccga,      "Schetmash", "Iskra 1031", GAME_NOT_WORKING)
COMP( 1989, iskr1030m,  ibm5150,    0,          iskr1031,   pccga, pc_state,      pccga,      "Schetmash", "Iskra 1030M", GAME_NOT_WORKING)
COMP( 1992, iskr3104,   ibm5150,    0,          iskr3104,   pcega, pc_state,      pccga,      "Schetmash", "Iskra 3104", GAME_NOT_WORKING)
COMP( 1987, ec1840,     ibm5150,    0,          iskr1031,   pccga, pc_state,      pccga,      "<unknown>", "EC-1840", GAME_NOT_WORKING)
COMP( 1987, ec1841,     ibm5150,    0,          ec1841,     pccga, pc_state,      pccga,      "<unknown>", "EC-1841", GAME_NOT_WORKING)
COMP( 1989, ec1845,     ibm5150,    0,          iskr1031,   pccga, pc_state,      pccga,      "<unknown>", "EC-1845", GAME_NOT_WORKING)
COMP( 1989, mk88,       ibm5150,    0,          iskr1031,   pccga, pc_state,      pccga,      "<unknown>", "MK-88", GAME_NOT_WORKING)
COMP( 1990, poisk1,     ibm5150,    0,          iskr1031,   pccga, pc_state,      pccga,      "<unknown>", "Poisk-1", GAME_NOT_WORKING)
COMP( 1991, poisk2,     ibm5150,    0,          poisk2,     pccga, pc_state,      pccga,      "<unknown>", "Poisk-2", GAME_NOT_WORKING)
COMP( 1990, mc1702,     ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "<unknown>", "Elektronika MC-1702", GAME_NOT_WORKING)
COMP( 1989, mc1502,     ibm5150,    0,          mc1502,     mc1502, pc_state,     mc1502,     "NPO Microprocessor", "Elektronika MC-1502", GAME_NOT_WORKING | GAME_NO_SOUND)

COMP( 1987, zdsupers,   ibm5150,    0,          zenith,     pccga, pc_state,      pccga,      "Zenith Data Systems", "SuperSport", 0)

COMP( 1983, m24,        ibm5150,    0,          olivetti,   pccga, pc_state,      pccga,      "Olivetti", "M24", GAME_NOT_WORKING)
COMP( 1987, m240,       ibm5150,    0,          olivetti,   pccga, pc_state,      pccga,      "Olivetti", "M240", GAME_NOT_WORKING)
COMP( 198?, olivm15,    ibm5150,    0,          olivm15,    0, driver_device,       0,          "Olivetti", "M15", GAME_NOT_WORKING | GAME_NO_SOUND)

COMP( 1983, ibm5550,    ibm5150,    0,          ibm5550,    pccga, pc_state,      pccga,      "International Business Machines", "IBM 5550", GAME_NOT_WORKING)

COMP( 1985, pc7000,     ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "Sharp", "PC-7000", GAME_NOT_WORKING)

COMP( 198?, pcd,        ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "Siemens", "PC-D", GAME_NOT_WORKING)
COMP( 198?, olypeopl,   ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "Olympia", "People PC", GAME_NOT_WORKING)
COMP( 1988, sx16,       ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "Sanyo", "SX-16", GAME_NOT_WORKING)
COMP( 198?, mbc16,      ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "Sanyo", "MBC-16" , GAME_NOT_WORKING)

COMP( 198?, ataripc3,   ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "Atari", "PC-3" , GAME_NOT_WORKING)
