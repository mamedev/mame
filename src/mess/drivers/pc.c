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


#include "bus/rs232/rs232.h"
#include "bus/rs232/ser_mouse.h"
#include "cpu/nec/nec.h"
#include "cpu/i86/i86.h"
#include "cpu/i86/i286.h"
#include "sound/speaker.h"
#include "sound/wave.h"

#include "machine/i8255.h"
#include "machine/ins8250.h"
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

#include "includes/europc.h"
#include "includes/tandy1t.h"

#include "includes/pc.h"

#include "imagedev/flopdrv.h"
#include "imagedev/harddriv.h"
#include "imagedev/cassette.h"
#include "imagedev/cartslot.h"
#include "formats/naslite_dsk.h"
#include "formats/pc_dsk.h"
#include "formats/asst128_dsk.h"

#include "machine/am9517a.h"
#include "sound/sn76496.h"

#include "machine/wd_fdc.h"

#include "machine/ram.h"
#include "bus/pc_kbd/keyboards.h"

#include "mcfglgcy.h"


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

static ADDRESS_MAP_START( asst128_map, AS_PROGRAM, 16, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x7ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
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
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE(pc_page_r,            pc_page_w)
	AM_RANGE(0x00a0, 0x00a0) AM_WRITE(pc_nmi_enable_w )
	AM_RANGE(0x0200, 0x0207) AM_DEVREADWRITE("pc_joy", pc_joy_device, joy_port_r, joy_port_w)
	AM_RANGE(0x0240, 0x0257) AM_READWRITE(pc_rtc_r,             pc_rtc_w)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE("ins8250_1", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x0340, 0x0357) AM_NOP /* anonymous bios should not recogniced realtimeclock */
	AM_RANGE(0x0378, 0x037f) AM_DEVREADWRITE("lpt_0", pc_lpt_device, read, write)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE("fdc", pc_fdc_interface, map)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE("ins8250_0", ins8250_device, ins8250_r, ins8250_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(oliv_io, AS_IO, 8, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START(pc16_io, AS_IO, 16, pc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE8("dma8237", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8("pic8259", pic8259_device, read, write, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8("pit8253", pit8253_device, read, write, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0xffff)
	AM_RANGE(0x0070, 0x007f) AM_RAM // needed for Poisk-2
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r,               pc_page_w, 0xffff)
	AM_RANGE(0x00a0, 0x00a1) AM_WRITE8(pc_nmi_enable_w, 0x00ff )
	AM_RANGE(0x0200, 0x0207) AM_DEVREADWRITE8("pc_joy", pc_joy_device, joy_port_r, joy_port_w, 0xffff)
	AM_RANGE(0x0240, 0x0257) AM_READWRITE8(pc_rtc_r,                pc_rtc_w, 0xffff)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE8("ins8250_1", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x0340, 0x0357) AM_NOP /* anonymous bios should not recogniced realtimeclock */
	AM_RANGE(0x0378, 0x037f) AM_DEVREADWRITE8("lpt_0", pc_lpt_device, read, write, 0xffff)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE8("fdc", pc_fdc_interface, map, 0xffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ins8250_0", ins8250_device, ins8250_r, ins8250_w, 0xffff)
ADDRESS_MAP_END


static ADDRESS_MAP_START(asst128_io, AS_IO, 16, pc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8("pic8259", pic8259_device, read, write, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8("pit8253", pit8253_device, read, write, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0xffff)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r,               pc_page_w, 0xffff)
	AM_RANGE(0x00a0, 0x00a1) AM_WRITE8( pc_nmi_enable_w, 0x00ff )
	AM_RANGE(0x0240, 0x0257) AM_READWRITE8(pc_rtc_r,                pc_rtc_w, 0xffff)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE8("ins8250_1", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x0340, 0x0357) AM_NOP /* anonymous bios should not recogniced realtimeclock */
	AM_RANGE(0x0378, 0x037f) AM_DEVREADWRITE8("lpt_0", pc_lpt_device, read, write, 0xffff)
	AM_RANGE(0x03f2, 0x03f3) AM_WRITE8(asst128_fdc_dor_w, 0xffff)
	AM_RANGE(0x03f4, 0x03f5) AM_DEVICE8("fdc:upd765", upd765a_device, map, 0xffff)
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
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8("pic8259", pic8259_device, read, write, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8("pit8253", pit8253_device, read, write, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0xffff)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r,               pc_page_w, 0xffff)
	AM_RANGE(0x00a0, 0x00a1) AM_READWRITE8(unk_r, pc_nmi_enable_w, 0x00ff )
	AM_RANGE(0x0240, 0x0257) AM_READWRITE8(pc_rtc_r,                pc_rtc_w, 0xffff)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE8("ins8250_1", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x0340, 0x0357) AM_NOP /* anonymous bios should not recogniced realtimeclock */
	AM_RANGE(0x0378, 0x037f) AM_DEVREADWRITE8("lpt_0", pc_lpt_device, read, write, 0xffff)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE8("fdc", pc_fdc_interface, map, 0xffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ins8250_0", ins8250_device, ins8250_r, ins8250_w, 0xffff)
ADDRESS_MAP_END


static ADDRESS_MAP_START( europc_map, AS_PROGRAM, 8, europc_pc_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END



static ADDRESS_MAP_START(europc_io, AS_IO, 8, europc_pc_state )
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE("dma8237", am9517a_device, read, write)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE(europc_pio_r,          europc_pio_w)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE(pc_page_r,            pc_page_w)
	AM_RANGE(0x0200, 0x0207) AM_DEVREADWRITE("pc_joy", pc_joy_device, joy_port_r, joy_port_w)
	AM_RANGE(0x0250, 0x025f) AM_READWRITE(europc_jim_r,          europc_jim_w)
	AM_RANGE(0x02e0, 0x02e0) AM_READ(europc_jim2_r)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE("ins8250_1", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x0378, 0x037b) AM_DEVREADWRITE("lpt_0", pc_lpt_device, read, write)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE("fdc", pc_fdc_interface, map)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE("ins8250_0", ins8250_device, ins8250_r, ins8250_w)
ADDRESS_MAP_END



static ADDRESS_MAP_START(tandy1000_map, AS_PROGRAM, 8, tandy_pc_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_RAM
	AM_RANGE(0xb0000, 0xb7fff) AM_NOP
	AM_RANGE(0xb8000, 0xbffff) AM_DEVREADWRITE("pcvideo_t1000", pcvideo_t1000_device, videoram_r, videoram_w);
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xc9fff) AM_ROM
	AM_RANGE(0xca000, 0xcffff) AM_NOP
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END



static ADDRESS_MAP_START(tandy1000_io, AS_IO, 8, tandy_pc_state )
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE("dma8237", am9517a_device, read, write)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE(tandy1000_pio_r,           tandy1000_pio_w)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE(pc_page_r,                pc_page_w)
	AM_RANGE(0x00c0, 0x00c0) AM_DEVWRITE("sn76496", ncr7496_device, write)
	AM_RANGE(0x0200, 0x0207) AM_DEVREADWRITE("pc_joy", pc_joy_device, joy_port_r, joy_port_w)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE("ins8250_1", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x0378, 0x037f) AM_READWRITE(pc_t1t_p37x_r,         pc_t1t_p37x_w)
	AM_RANGE(0x03bc, 0x03be) AM_DEVREADWRITE("lpt_0", pc_lpt_device, read, write)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE("pcvideo_t1000", pcvideo_t1000_device, read, write)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE("fdc", pc_fdc_interface, map)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE("ins8250_0", ins8250_device, ins8250_r, ins8250_w)
ADDRESS_MAP_END



static ADDRESS_MAP_START(tandy1000_16_map, AS_PROGRAM, 16, pc_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_RAM
	AM_RANGE(0xb0000, 0xb7fff) AM_NOP
	AM_RANGE(0xb8000, 0xbffff) AM_DEVREADWRITE8("pcvideo_t1000", pcvideo_t1000_device, videoram_r, videoram_w, 0xffff)
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xc9fff) AM_ROM
	AM_RANGE(0xca000, 0xcffff) AM_NOP
	AM_RANGE(0xe0000, 0xeffff) AM_ROMBANK("biosbank")                     /* Banked part of the BIOS */
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION( "romcs0", 0x70000 )
ADDRESS_MAP_END


static ADDRESS_MAP_START(tandy1000_16_io, AS_IO, 16, tandy_pc_state )
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE8("dma8237", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8("pic8259", pic8259_device, read, write, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8("pit8253", pit8253_device, read, write, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE8(tandy1000_pio_r,          tandy1000_pio_w, 0xffff)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r,               pc_page_w, 0xffff)
	AM_RANGE(0x00c0, 0x00c1) AM_DEVWRITE8("sn76496",    ncr7496_device, write, 0xffff)
	AM_RANGE(0x0200, 0x0207) AM_DEVREADWRITE8("pc_joy", pc_joy_device, joy_port_r, joy_port_w, 0xffff)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE8("ins8250_1", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x0378, 0x037f) AM_READWRITE8(pc_t1t_p37x_r,            pc_t1t_p37x_w, 0xffff)
	AM_RANGE(0x03bc, 0x03bf) AM_DEVREADWRITE8("lpt_0", pc_lpt_device, read, write, 0xffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("pcvideo_t1000", pcvideo_t1000_device, read, write, 0xffff)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE8("fdc", pc_fdc_interface, map, 0xffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ins8250_0", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0xffea, 0xffeb) AM_READWRITE8(tandy1000_bank_r, tandy1000_bank_w, 0xffff)
ADDRESS_MAP_END



static ADDRESS_MAP_START(tandy1000_286_map, AS_PROGRAM, 16, tandy_pc_state )
	ADDRESS_MAP_GLOBAL_MASK(0x000fffff)
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_RAM
	AM_RANGE(0xb0000, 0xb7fff) AM_NOP
	AM_RANGE(0xb8000, 0xbffff) AM_DEVREADWRITE8("pcvideo_t1000", pcvideo_t1000_device, videoram_r, videoram_w, 0xffff)
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xc9fff) AM_ROM
	AM_RANGE(0xca000, 0xcffff) AM_NOP
	AM_RANGE(0xe0000, 0xeffff) AM_NOP
	AM_RANGE(0xf8000, 0xfffff) AM_ROM
ADDRESS_MAP_END



static ADDRESS_MAP_START(tandy1000_286_io, AS_IO, 16, tandy_pc_state )
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE8("dma8237", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8("pic8259", pic8259_device, read, write, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8("pit8253", pit8253_device, read, write, 0xffff)
	AM_RANGE(0x0060, 0x0063) AM_READWRITE8(tandy1000_pio_r,         tandy1000_pio_w, 0xffff)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r,               pc_page_w, 0xffff)
	AM_RANGE(0x00c0, 0x00c1) AM_DEVWRITE8("sn76496", ncr7496_device, write, 0xffff)
	AM_RANGE(0x0200, 0x0207) AM_DEVREADWRITE8("pc_joy", pc_joy_device, joy_port_r, joy_port_w, 0xffff)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE8("ins8250_1", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x0378, 0x037f) AM_READWRITE8(pc_t1t_p37x_r,           pc_t1t_p37x_w, 0xffff)
	AM_RANGE(0x03bc, 0x03bf) AM_DEVREADWRITE8("lpt_0", pc_lpt_device, read, write, 0xffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("pcvideo_t1000", pcvideo_t1000_device, read, write, 0xffff)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVICE8("fdc", pc_fdc_interface, map, 0xffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ins8250_0", ins8250_device, ins8250_r, ins8250_w, 0xffff)
ADDRESS_MAP_END


static ADDRESS_MAP_START(ibmpcjr_map, AS_PROGRAM, 8, tandy_pc_state )
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


static ADDRESS_MAP_START(ibmpcjr_io, AS_IO, 8, tandy_pc_state )
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x00a0, 0x00a0) AM_READWRITE(pcjr_nmi_enable_r, pc_nmi_enable_w )
	AM_RANGE(0x00c0, 0x00c0) AM_DEVWRITE("sn76496", sn76496_device, write)
	AM_RANGE(0x00f2, 0x00f2) AM_WRITE(pcjr_fdc_dor_w)
	AM_RANGE(0x00f4, 0x00f5) AM_DEVICE("upd765", upd765a_device, map)
	AM_RANGE(0x0200, 0x0207) AM_DEVREADWRITE("pc_joy", pc_joy_device, joy_port_r, joy_port_w)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE("ins8250_1", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x0378, 0x037b) AM_DEVREADWRITE("lpt_0", pc_lpt_device, read, write)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE("pcvideo_pcjr", pcvideo_pcjr_device, read, write)
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
	PORT_INCLUDE( pcvideo_cga )
INPUT_PORTS_END

static INPUT_PORTS_START( tandy1t )
	PORT_START("IN0") /* IN0 */
	PORT_BIT ( 0xf0, 0xf0,   IPT_UNUSED )
	PORT_BIT ( 0x08, 0x08,   IPT_CUSTOM ) PORT_VBLANK("pcvideo_t1000:screen")
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
INPUT_PORTS_END

static INPUT_PORTS_START( ibmpcjr )
	PORT_INCLUDE( tandy1t )
	PORT_MODIFY("pc_keyboard_3")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_MODIFY("pc_keyboard_4")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NumLock") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_MODIFY("IN0")
	PORT_BIT ( 0x08, 0x08,   IPT_CUSTOM ) PORT_VBLANK("pcvideo_pcjr:screen")
INPUT_PORTS_END



FLOPPY_FORMATS_MEMBER( pc_state::floppy_formats )
	FLOPPY_PC_FORMAT,
	FLOPPY_NASLITE_FORMAT
FLOPPY_FORMATS_END

FLOPPY_FORMATS_MEMBER( pc_state::asst128_formats )
	FLOPPY_ASST128_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( ibmpc_floppies )
		SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
		SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

static SLOT_INTERFACE_START( asst128_floppies )
		SLOT_INTERFACE( "525ssqd", FLOPPY_525_SSQD )
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(ibm5150_com)
	SLOT_INTERFACE("microsoft_mouse", MSFT_SERIAL_MOUSE)
	SLOT_INTERFACE("mouse_systems_mouse", MSYSTEM_SERIAL_MOUSE)
SLOT_INTERFACE_END

#define MCFG_CPU_PC(mem, port, type, clock, vblankfunc) \
	MCFG_CPU_ADD("maincpu", type, clock)                \
	MCFG_CPU_PROGRAM_MAP(mem##_map) \
	MCFG_CPU_IO_MAP(port##_io)  \
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

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */

	MCFG_RS232_PORT_ADD( "serport0", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, cts_w))

	MCFG_FRAGMENT_ADD( pcvideo_cga )
	MCFG_GFXDECODE_ADD("gfxdecode", ibm5150)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w))

	MCFG_PC_JOY_ADD("pc_joy")

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

static const gfx_layout pc10iii_16_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	2048,                    /* 2048 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

// 8-byte chars: 080-0FF, 180-27F, 300-37F, 480-4FF, 580-67F, 700-77F
// 16-byte chars: 000-07F, 100-17F, 280-2FF, 380-47F, 500-57F, 680-6FF, 780-7FF
static GFXDECODE_START( pc10iii )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc10iii_16_charlayout, 3, 1 )
GFXDECODE_END

static MACHINE_CONFIG_DERIVED( pc10iii, pccga )
	MCFG_GFXDECODE_MODIFY("gfxdecode", pc10iii)
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

static MACHINE_CONFIG_START( europc, europc_pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(europc, europc, I8088, 4772720*2, pc_frame_interrupt)

	MCFG_MACHINE_START_OVERRIDE(europc_pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(europc_pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_I8255_ADD( "ppi8255", pc_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */

	MCFG_RS232_PORT_ADD( "serport0", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, cts_w))

	MCFG_PC_JOY_ADD("pc_joy")

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_aga )
	MCFG_GFXDECODE_ADD("gfxdecode", europc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_NVRAM_HANDLER( europc_rtc )

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w))

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


static const gfx_layout t1000_charlayout =
{
	8, 16,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432, 20480, 22528, 24576, 26624, 28672, 30720 },
	8
};


static GFXDECODE_START( t1000 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, t1000_charlayout, 3, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( t1000hx, tandy_pc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, 8000000)                \
	MCFG_CPU_PROGRAM_MAP(tandy1000_map) \
	MCFG_CPU_IO_MAP(tandy1000_io)  \
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", pc_state, pc_frame_interrupt, "pcvideo_t1000:screen", 0, 1) //with this line commented out, it boots further though keyboard doesn't work, obviously

	MCFG_MACHINE_START_OVERRIDE(tandy_pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_I8255_ADD( "ppi8255", pc_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */

	MCFG_RS232_PORT_ADD( "serport0", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, cts_w))

	/* video hardware */
	MCFG_PCVIDEO_T1000_ADD("pcvideo_t1000")
	MCFG_GFXDECODE_ADD("gfxdecode", t1000)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
	MCFG_SOUND_ADD("sn76496", NCR7496, XTAL_14_31818MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_NVRAM_HANDLER( tandy1000 )

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w))

	MCFG_PC_JOY_ADD("pc_joy")

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "35dd", pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( t1000sx, t1000hx )
	MCFG_DEVICE_REMOVE("fdc:0")
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", pc_state::floppy_formats)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( t1000_16, tandy_pc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, XTAL_28_63636MHz / 3)                \
	MCFG_CPU_PROGRAM_MAP(tandy1000_16_map) \
	MCFG_CPU_IO_MAP(tandy1000_16_io)  \
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", pc_state, pc_frame_interrupt, "pcvideo_t1000:screen", 0, 1)


	MCFG_MACHINE_START_OVERRIDE(tandy_pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(tandy_pc_state,tandy1000rl)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_I8255_ADD( "ppi8255", pc_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */

	MCFG_RS232_PORT_ADD( "serport0", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, cts_w))

	/* video hardware */
	MCFG_PCVIDEO_T1000_ADD("pcvideo_t1000")
	MCFG_GFXDECODE_ADD("gfxdecode", t1000)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
	MCFG_SOUND_ADD("sn76496", NCR7496, XTAL_14_31818MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_NVRAM_HANDLER( tandy1000 )

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w))

	MCFG_PC_JOY_ADD("pc_joy")

	MCFG_PC_FDC_XT_ADD("fdc")
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "35dd", pc_state::floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( t1000_16_8, t1000_16 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_CLOCK( XTAL_24MHz / 3 )
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( t1000_286, tandy_pc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, XTAL_28_63636MHz / 2)                \
	MCFG_CPU_PROGRAM_MAP(tandy1000_286_map) \
	MCFG_CPU_IO_MAP(tandy1000_286_io)  \
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", pc_state, pc_frame_interrupt, "pcvideo_t1000:screen", 0, 1)


	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_I8255_ADD( "ppi8255", pc_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */

	MCFG_RS232_PORT_ADD( "serport0", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, cts_w))

	/* video hardware */
	MCFG_PCVIDEO_T1000_ADD("pcvideo_t1000")
	MCFG_GFXDECODE_ADD("gfxdecode", t1000)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
	MCFG_SOUND_ADD("sn76496", NCR7496, XTAL_14_31818MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_NVRAM_HANDLER( tandy1000 )

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w))

	MCFG_PC_JOY_ADD("pc_joy")

	MCFG_PC_FDC_XT_ADD("fdc")
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "35dd", pc_state::floppy_formats)

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

static MACHINE_CONFIG_START( ibmpcjr, tandy_pc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, 4900000)   \
	MCFG_CPU_PROGRAM_MAP(ibmpcjr_map) \
	MCFG_CPU_IO_MAP(ibmpcjr_io)  \
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", pc_state, pcjr_frame_interrupt, "pcvideo_pcjr:screen", 0, 1) //with this line commented out, it boots further though keyboard doesn't work, obviously

	MCFG_MACHINE_START_OVERRIDE(pc_state,pcjr)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pcjr)

	MCFG_PIT8253_ADD( "pit8253", pcjr_pit8253_config )

	MCFG_PIC8259_ADD( "pic8259", WRITELINE(pc_state,pcjr_pic8259_set_int_line), VCC, NULL )

	MCFG_I8255_ADD( "ppi8255", pcjr_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */

	MCFG_RS232_PORT_ADD( "serport1", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, cts_w))

	/* video hardware */
	MCFG_PCVIDEO_PCJR_ADD("pcvideo_pcjr")
	MCFG_GFXDECODE_ADD("gfxdecode", ibmpcjr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
	MCFG_SOUND_ADD("sn76496", SN76496, XTAL_14_31818MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_NVRAM_HANDLER( tandy1000 )

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w))

	MCFG_PC_JOY_ADD("pc_joy")

	/* cassette */
	MCFG_CASSETTE_ADD( "cassette", ibm5150_cassette_interface )

	MCFG_UPD765A_ADD("upd765", false, false)

	MCFG_FLOPPY_DRIVE_ADD("upd765:0", ibmpc_floppies, "525dd", pc_state::floppy_formats)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart1")
	MCFG_CARTSLOT_INTERFACE("ibmpcjr_cart")
	MCFG_CARTSLOT_EXTENSION_LIST("jrc")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(pc_state,pcjr_cartridge)
	MCFG_CARTSLOT_ADD("cart2")
	MCFG_CARTSLOT_INTERFACE("ibmpcjr_cart")
	MCFG_CARTSLOT_EXTENSION_LIST("jrc")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(pc_state,pcjr_cartridge)

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
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", ibmpc_floppies, "35dd", pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", ibmpc_floppies, "35dd", pc_state::floppy_formats)

	MCFG_GFXDECODE_MODIFY("gfxdecode", ibmpcjx)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( asst128, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(asst128, asst128, I8086, 4772720, pc_frame_interrupt)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", pcjr_pit8253_config )

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_mc1502 )
	MCFG_GFXDECODE_ADD("gfxdecode", ibmpcjr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w)) // verify

	MCFG_CASSETTE_ADD( "cassette", mc1502_cassette_interface )

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", asst128_floppies, "525ssqd", pc_state::asst128_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", asst128_floppies, "525ssqd", pc_state::asst128_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */

	MCFG_RS232_PORT_ADD( "serport0", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, cts_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( iskr3104, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(pc16, pc16, I8086, 4772720, pc_frame_interrupt)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pc_state,pc)
	MCFG_MACHINE_RESET_OVERRIDE(pc_state,pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */

	MCFG_RS232_PORT_ADD( "serport0", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, cts_w))

	/* video hardware */
//  MCFG_FRAGMENT_ADD( pcvideo_ega ) // Put this back after ISA are added to this driver
	MCFG_FRAGMENT_ADD( pcvideo_cga )
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w)) // verify

	MCFG_PC_JOY_ADD("pc_joy")

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", pc_state::floppy_formats)

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

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */

	MCFG_RS232_PORT_ADD( "serport0", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, cts_w))

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_poisk2 )
	MCFG_GFXDECODE_ADD("gfxdecode", ibm5150)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w)) // verify

	MCFG_PC_JOY_ADD("pc_joy")

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", pc_state::floppy_formats)

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

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */

	MCFG_RS232_PORT_ADD( "serport0", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, cts_w))

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_cga )
	MCFG_GFXDECODE_ADD("gfxdecode", ibm5150)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w)) // verify

	MCFG_PC_JOY_ADD("pc_joy")

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", pc_state::floppy_formats)

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

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */

	MCFG_RS232_PORT_ADD( "serport0", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, cts_w))

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_cga )
	MCFG_GFXDECODE_ADD("gfxdecode", ibm5150)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w)) // verify

	MCFG_PC_JOY_ADD("pc_joy")

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", pc_state::floppy_formats)

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

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_I8255_ADD( "ppi8255", ibm5160_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )   /* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )   /* TODO: Verify model */

	MCFG_RS232_PORT_ADD( "serport0", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_0", ins8250_uart_device, cts_w))

	MCFG_RS232_PORT_ADD( "serport1", ibm5150_com, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250_1", ins8250_uart_device, cts_w))

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_cga )
	MCFG_GFXDECODE_ADD("gfxdecode", ibm5150)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* keyboard */
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w)) // verify

	MCFG_PC_JOY_ADD("pc_joy")

	MCFG_PC_FDC_XT_ADD("fdc")

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ibmpc_floppies, "525dd", pc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", ibmpc_floppies, "525dd", pc_state::floppy_formats)

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

	ROM_REGION(0x08100,"gfx1", 0) //TODO: needs a different charset
	ROM_LOAD("cga.chr",     0x00000, 0x01000, BAD_DUMP CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd)) // from an unknown clone cga card

	ROM_REGION(0x38000,"kanji", 0)
	ROM_LOAD("kanji.rom",     0x00000, 0x38000, BAD_DUMP CRC(eaa6e3c3) SHA1(35554587d02d947fae8446964b1886fff5c9d67f)) // hand-made rom
ROM_END

#ifdef UNUSED_DEFINITION
ROM_START( t1000 )
	// Schematics displays 2 32KB ROMs at U9 and U10
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "v010000", "v010000" )
	ROMX_LOAD("v010000.f0", 0xf0000, 0x10000, NO_DUMP, ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v010100", "v010100" )
	ROMX_LOAD("v010100.f0", 0xf0000, 0x10000, CRC(b6760881) SHA1(8275e4c48ac09cf36685db227434ca438aebe0b9), ROM_BIOS(2))

	// Part of video array at u76?
	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u76", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END

ROM_START( t1000a )
	ROM_REGION(0x100000,"maincpu", 0)
	// partlist says it has 1 128kbyte rom
	ROM_LOAD("t1000hx.e0", 0xe0000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_LOAD("v010100.f0", 0xf0000, 0x10000, CRC(b6760881) SHA1(8275e4c48ac09cf36685db227434ca438aebe0b9))

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u25", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END

ROM_START( t1000ex )
	ROM_REGION(0x100000,"maincpu", 0)
	// partlist says it has 1 128kb rom, schematics list a 32k x 8 rom
	// "8040328.u17"
	ROM_LOAD("t1000hx.e0", 0xe0000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_LOAD("v010200.f0", 0xf0000, 0x10000, CRC(0e016ecf) SHA1(2f5ac8921b7cba56b02122ef772f5f11bbf6d8a2))

	// TODO: Add dump of the 8048 at u8 if it ever gets dumped
	ROM_REGION(0x400, "kbdc", 0)
	ROM_LOAD("8048.u8", 0x000, 0x400, NO_DUMP)

	// Most likely part of big blue at u28
	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u28", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END
#endif

ROM_START( t1000hx )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("v020000.u12", 0xe0000, 0x20000, CRC(6f3acd80) SHA1(976af8c04c3f6fde14d7047f6521d302bdc2d017)) // TODO: Rom label

	// TODO: Add dump of the 8048 at u9 if it ever gets dumped
	ROM_REGION(0x400, "kbdc", 0)
	ROM_LOAD("8048.u9", 0x000, 0x400, NO_DUMP)

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u31", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location, probably internal to "big blue" at u31
ROM_END

#ifdef UNUSED_DEFINITION
// The T1000SL and T1000SL/2 only differ in amount of RAM installed and BIOS version (SL/2 has v01.04.04)
ROM_START( t1000sl )
	ROM_REGION(0x100000,"maincpu", 0)

	// 8076312.hu1 - most likely v01.04.00
	// 8075312.hu2


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
	ROM_LOAD("8079027.u25", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450))
ROM_END


ROM_START( t1000tl )
	ROM_REGIoN(0x100000, "maincpu", ROMREGION_ERASE00)

	ROM_REGION(0x80000, "romcs0", 0)
	// These 2 sets most likely have the same contents
	// v01.04.00
	// 8076323.u55 - Sharp - 256KB
	// 8075323.u57 - Sharp - 256KB
	// v01.04.00
	// 8079025.u54 - Hitachi - 256KB
	// 8079026.u56 - Hitachi - 256KB
	ROM_REGION(0x80000, "romcs1", 0)

	// 2x 128x8 eeprom?? @ u58 and u59 - not mentioned in parts list

	ROM_REGION(0x80, "eeprom", 0)
	ROM_LOAD("8040346_9346.u12", xxx ) // 64x16 eeprom

	ROM_REGION(0x08000, "gfx1", 0)
	ROM_LOAD("8079027.u24", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450))
ROM_END
#endif


ROM_START( t1000sx )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("8040328.u41", 0xf8000, 0x8000, CRC(4e2b9f0b) SHA1(e79a9ed9e885736e30d9b135557f0e596ce5a70b))

	// No character rom is listed in the schematics?
	// But disabling it results in no text being printed
	// Part of bigblue at u30??
	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u30", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END


ROM_START( t1000tx )
	ROM_REGION(0x100000,"maincpu", 0)
	// There should be 2 32KBx8 ROMs, one for odd at u38, one for even at u39
	// The machine already boots up with just this one rom
	ROM_LOAD("t1000tx.bin", 0xf8000, 0x8000, BAD_DUMP CRC(9b34765c) SHA1(0b07e87f6843393f7d4ca4634b832b0c0bec304e))

	// No character rom is listed in the schematics?
	// It is most likely part of the big blue chip at u36
	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u36", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END


ROM_START( t1000rl )
	ROM_REGION(0x100000,"maincpu", ROMREGION_ERASE00)

	// bankable ROM regions
	ROM_REGION(0x80000, "romcs0", 0)
	/* v2.0.0.1 */
	/* Rom is labeled "(C) TANDY CORP. 1990 // 8079073 // LH534G70 JAPAN // 9034 D" */
	ROM_LOAD("8079073.u23", 0x00000, 0x80000, CRC(6fab50f7) SHA1(2ccc02bee4c250dc1b7c17faef2590bc158860b0) )
	ROM_REGION(0x80000, "romcs1", ROMREGION_ERASEFF)

	ROM_REGION(0x08000,"gfx1", 0)
	/* Character rom located at U3 w/label "8079027 // NCR // 609-2495004 // F841030 A9025" */
	ROM_LOAD("8079027.u3", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END


ROM_START( t1000sl2 )
	ROM_REGION(0x100000,"maincpu", ROMREGION_ERASE00)

	// bankable ROM regions
	ROM_REGION(0x80000, "romcs0", 0)
	// v01.04.04 BIOS
	ROM_LOAD16_BYTE("8079047.hu1", 0x00000, 0x40000, CRC(c773ec0e) SHA1(7deb71f14c2c418400b639d60066ab61b7e9df32))
	ROM_LOAD16_BYTE("8079048.hu2", 0x00001, 0x40000, CRC(0f3e6586) SHA1(10f1a7204f69b82a18bc94a3010c9660aec0c802))
	ROM_REGION(0x80000, "romcs1", ROMREGION_ERASEFF)

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u25", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450))

	ROM_REGION(0x80, "nmc9246n", 0)
	ROM_LOAD("seeprom.bin", 0, 0x80, CRC(4fff41df) SHA1(41a7009694550c017996932beade608cff968f4a))
ROM_END


ROM_START( t1000tl2 )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "t10000tl2.bin", 0xf0000, 0x10000, CRC(e288f12c) SHA1(9d54ccf773cd7202c9906323f1b5a68b1b3a3a67))

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u24", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END


ROM_START( dgone )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "dgone.bin",  0xf8000, 0x08000, CRC(2c38c86e) SHA1(c0f85a000d1d13cd354965689e925d677822549e))

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( ssam88s )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "samsung_samtron_88s_vers_2.0a.bin",  0xf8000, 0x08000, CRC(d1252a91) SHA1(469d15b6ecd7b70234975dc12c6bda4212a66652))

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

ROM_START( asst128 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_DEFAULT_BIOS("floppy")
	/* BASIC ROM taken from IBM 5150 and needs dumping */
	ROM_LOAD( "basic-1.10.rom",    0xf6000, 0x8000, CRC(ebacb791) SHA1(07449ebca18f979b9ab748582b736e402f2bf940))
	ROM_LOAD( "asf400-f600.bin",   0xf4000, 0x2000, CRC(e3bf22de) SHA1(d4319edc82c0015ca0adc6c8771e887659717e62))
	ROM_SYSTEM_BIOS(0, "floppy", "3rd party floppy support")
	ROMX_LOAD( "rombios7.bin",     0xfc001, 0x2000, CRC(7d7c8d6a) SHA1(a731a65ee547f1d78cfc91461f38166da014f3dc), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "rombios8.bin",     0xfc000, 0x2000, CRC(ba304663) SHA1(b2533b8f8240f72b7315f27c7b64f95ac52687ca), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "stock", "cassette-only BIOS?")
	ROMX_LOAD( "mainbios.bin",     0xfe000, 0x2000, CRC(8426cbf5) SHA1(41d14137ffa651977041da22aa8071c0f7854158), ROM_BIOS(2))
	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	ROM_COPY( "maincpu", 0xffa6e, 0x0800, 0x0400 )
	ROM_COPY( "maincpu", 0xfc000, 0x0c00, 0x0400 )
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
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("380270-01.bin", 0xfc000, 0x4000, BAD_DUMP CRC(75135d37) SHA1(177283642240fee191ba2d87e1d0c2a377c78ccb))
	ROM_REGION(0x8000, "gfx1", 0)
	ROM_LOAD("pc1_char.bin", 0x0000, 0x8000, CRC(4773a945) SHA1(bcc38abecc75d3f641d42987cb0d2ed71d71bc4c))
ROM_END

// Note: Commodore PC20-III, PC10-III and COLT share the same BIOS
ROM_START( pc10iii )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_DEFAULT_BIOS("v441")
	ROM_SYSTEM_BIOS(0, "v435", "v4.35")
	ROMX_LOAD("318085-01.u201", 0xf8000, 0x8000, NO_DUMP, ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v436", "v4.36")
	ROMX_LOAD("318085-02.u201", 0xf8000, 0x8000, NO_DUMP, ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v436c", "v4.36c")
	ROMX_LOAD("318085-04.u201", 0xf8000, 0x8000, NO_DUMP, ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "v438", "v4.38")
	ROMX_LOAD("318085-05.u201", 0xf8000, 0x8000, CRC(ae9e6a31) SHA1(853ee251cf230818c407a8d13ef060a21c90a8c1), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "v439", "v4.39")
	ROMX_LOAD("318085-06.u201", 0xf8000, 0x8000, NO_DUMP, ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "v440", "v4.40")
	ROMX_LOAD("318085-07.u201", 0xf8000, 0x8000, NO_DUMP, ROM_BIOS(6))
	ROM_SYSTEM_BIOS(6, "v441", "v4.41")
	ROMX_LOAD("318085-08.u201", 0xf8000, 0x8000, CRC(7e228dc8) SHA1(958dfdd637bd31c01b949fac729d6973a7e630bc), ROM_BIOS(7))
	ROM_REGION(0x8000, "gfx1", 0)
	ROM_LOAD("318086-02.u607", 0x0000, 0x8000, CRC(b406651c) SHA1(856f58353391a74a06ebb8ec9f8333d7d69e5fd6))
	//ROM_LOAD("5788005.u33", 0x00000, 0x2000, BAD_DUMP CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* temp so you can read the text */
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
COMP( 1988, europc,     ibm5150,    0,          europc,     europc, europc_pc_state,     europc,     "Schneider Rdf. AG", "EURO PC", GAME_NOT_WORKING)
COMP( 1984, compc1,     ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "Commodore Business Machines", "Commodore PC-1" , GAME_NOT_WORKING)
COMP( 1987, pc10iii,    ibm5150,    0,          pc10iii,    pccga, pc_state,      pccga,      "Commodore Business Machines", "Commodore PC-10 III" , GAME_NOT_WORKING)

// pcjr (better graphics, better sound)
COMP( 1983, ibmpcjr,    ibm5150,    0,          ibmpcjr,    ibmpcjr,  pc_state,    pcjr,       "International Business Machines", "IBM PC Jr", GAME_IMPERFECT_COLORS )
COMP( 1985, ibmpcjx,    ibm5150,    0,          ibmpcjx,    ibmpcjr, pc_state,    pcjr,       "International Business Machines", "IBM PC JX", GAME_IMPERFECT_COLORS | GAME_NOT_WORKING)

// tandy 1000
COMP( 1987, t1000hx,    ibm5150,    0,          t1000hx,    tandy1t, tandy_pc_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 HX", 0)
COMP( 1987, t1000sx,    ibm5150,    0,          t1000sx,    tandy1t, tandy_pc_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 SX", GAME_NOT_WORKING)
COMP( 1987, t1000tx,    ibm5150,    0,          t1000_286,  tandy1t, tandy_pc_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 TX", 0)
COMP( 1989, t1000rl,    ibm5150,    0,          t1000_16,   tandy1t, tandy_pc_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 RL", 0)
COMP( 1989, t1000tl2,   ibm5150,    0,          t1000_286,  tandy1t, tandy_pc_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 TL/2", 0)
COMP( 1988, t1000sl2,   ibm5150,    0,          t1000_16_8, tandy1t, tandy_pc_state,    t1000sl,    "Tandy Radio Shack", "Tandy 1000 SL/2", GAME_NOT_WORKING)

COMP( 1992, iskr3104,   ibm5150,    0,          iskr3104,   pcega, pc_state,      pccga,      "Schetmash", "Iskra 3104", GAME_NOT_WORKING)
COMP( 198?, asst128,    ibm5150,    0,          asst128,    pccga, pc_state,      pccga,      "Schetmash", "Assistent 128", GAME_NOT_WORKING)
COMP( 1989, mk88,       ibm5150,    0,          iskr3104,   pccga, pc_state,      pccga,      "<unknown>", "MK-88", GAME_NOT_WORKING)
COMP( 1991, poisk2,     ibm5150,    0,          poisk2,     pccga, pc_state,      pccga,      "<unknown>", "Poisk-2", GAME_NOT_WORKING)
COMP( 1990, mc1702,     ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "<unknown>", "Elektronika MC-1702", GAME_NOT_WORKING)

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
COMP( 1989, ssam88s,    ibm5150,    0,          pccga,      pccga, pc_state,      pccga,      "Samsung", "Samtron 88S" , GAME_NOT_WORKING)
