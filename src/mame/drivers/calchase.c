/************************************************************************************

California Chase (c) 1999 The Game Room

preliminary driver by Angelo Salese

TODO:
- currently calls int 13h with ah=0 -> "Reset Disk Drives" and returns an error code
for whatever reason;
- video emulation not even started (it currently fills vram with 0x0720, hence it's
unneeded for now...)
- clean-ups;

I/O Memo (http://bochs.sourceforge.net/techspec/PORTS.LST):
46E8    ----    8514/A and compatible video cards (e.g. ATI Graphics Ultra)
46E8    w   ROM page select
83C0-83CF ----  Compaq QVision - Line Draw Engine
83C4      ----  Compaq Qvision EISA - Virtual Controller Select
83C6-83C9 ----  Compaq Qvision EISA - DAC color registers

43c4 is a 83c4 mirror?

04D0-04D1 ---- EISA IRQ control
00F0-00F5 ----  PCjr Disk Controller
(or)
00F0-00FF ----  coprocessor (8087..80387)

=====================================================================================

California Chase
The Game Room, 1999

This is a rip off of Chase HQ running on PC hardware
and standard 15kHz arcade monitor

Main board is a cheap Taiwanese one made by SOYO.
Model SY-5EAS
Major main board chips are....
Cyrix 686MX-PR200 CPU
1M BIOS (Winbond 29C011)
64M RAM (2x 32M 72 pin SIMMs)
ETEQ EQ82C6617'97 MB817A0AG12 (QFP100, x2)
UT6164C32Q-6 (QFP100, x2)
ETEQ 82C6619'97 MB13J15001 (QFP208)
ETEQ 82C6618'97 MB14B10971 (QFP208)
UM61256
PLL52C61
SMC FD37C669QF (QFP100)
3V coin battery
3x PCI slots
4x 16-bit ISA slots
4x 72 pin RAM slots
connectors for COM1, COM2, LPT1, IDE0, IDE1, floppy etc
uses standard AT PSU

Video card is Trident TGUI9680 with 512k on-board VRAM
RAM is AS4C256K16EO-50JC (x2)
Trident BIOS V5.5 (DIP28). Actual size unknown, dumped as 64k, 128k, 256k and 512k (can only be one of these sizes)
6.5536MHz xtal

Custom JAMMA I/O board plugs into one ISA slot
Major components are...
XILINX XC3042
Dallas DS1220Y NVRAM (dumped)
MACH210 (protected)
16MHz OSC
VGA connector (tied to VGA card with a cable)
2x 4-pin connectors for controls
AD7547
ULN2803
LM324 (op amp)
TDA1552 (power amp)
ST TS272 (dual op amp)
2 volume pots
power input connector (from AT PSU)
JAMMA edge connector
ISA edge connector

HDD is WD Caviar 2170. C/H/S = 1010/6/55. Capacity = 170.6MB
The HDD is DOS-readable and in fact the OS is just Windows 98 DOS and can
be easily copied. Tested with another HDD.... formatted with DOS, copied
all files across to new HDD, boots up fine.

************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "memconv.h"
#include "devconv.h"
#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/pcshare.h"
#include "machine/pci.h"
#include "machine/8042kbdc.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"

static void ide_interrupt(running_device *device, int state);

static UINT32 *bios_ram;
static UINT32 *vga_vram;

static struct {
	running_device	*pit8254;
	running_device	*pic8259_1;
	running_device	*pic8259_2;
	running_device	*dma8237_1;
	running_device	*dma8237_2;
} calchase_devices;


static VIDEO_START(calchase)
{

}

static VIDEO_UPDATE(calchase)
{
	int x,y,count,i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	count = (0);

	for(y=0;y<256;y++)
	{
		for(x=0;x<320;x+=32)
		{
			for (i=0;i<32;i++)
			{
				UINT32 color;

				color = (vga_vram[count])>>(32-i) & 0x1;

				if((x+i)<video_screen_get_visible_area(screen)->max_x && ((y)+0)<video_screen_get_visible_area(screen)->max_y)
					*BITMAP_ADDR32(bitmap, y, x+(32-i)) = screen->machine->pens[color];

			}

			count++;
		}
	}

	return 0;
}

static READ8_DEVICE_HANDLER(at_dma8237_2_r)
{
	return i8237_r(device, offset / 2);
}

static WRITE8_DEVICE_HANDLER(at_dma8237_2_w)
{
	i8237_w(device, offset / 2, data);
}

static READ32_DEVICE_HANDLER(at32_dma8237_2_r)
{
	return read32le_with_read8_device_handler(at_dma8237_2_r, device, offset, mem_mask);
}

static WRITE32_DEVICE_HANDLER(at32_dma8237_2_w)
{
	write32le_with_write8_device_handler(at_dma8237_2_w, device, offset, data, mem_mask);
}

static int dma_channel;
static UINT8 dma_offset[2][4];
static UINT8 at_pages[0x10];


static READ8_HANDLER(at_page8_r)
{
	UINT8 data = at_pages[offset % 0x10];

	switch(offset % 8) {
	case 1:
		data = dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}


static WRITE8_HANDLER(at_page8_w)
{
	at_pages[offset % 0x10] = data;

	switch(offset % 8) {
	case 1:
		dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}


static WRITE_LINE_DEVICE_HANDLER( pc_dma_hrq_changed )
{
	cputag_set_input_line(device->machine, "maincpu", INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( device, state );
}


static READ8_HANDLER( pc_dma_read_byte )
{
	offs_t page_offset = (((offs_t) dma_offset[0][dma_channel]) << 16)
		& 0xFF0000;

	return memory_read_byte(space, page_offset + offset);
}


static WRITE8_HANDLER( pc_dma_write_byte )
{
	offs_t page_offset = (((offs_t) dma_offset[0][dma_channel]) << 16)
		& 0xFF0000;

	memory_write_byte(space, page_offset + offset, data);
}

static void set_dma_channel(running_device *device, int channel, int state)
{
	if (!state) dma_channel = channel;
}

static WRITE_LINE_DEVICE_HANDLER( pc_dack0_w ) { set_dma_channel(device, 0, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack1_w ) { set_dma_channel(device, 1, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack2_w ) { set_dma_channel(device, 2, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack3_w ) { set_dma_channel(device, 3, state); }

static I8237_INTERFACE( dma8237_1_config )
{
	DEVCB_LINE(pc_dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, pc_dma_read_byte),
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, pc_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_LINE(pc_dack0_w), DEVCB_LINE(pc_dack1_w), DEVCB_LINE(pc_dack2_w), DEVCB_LINE(pc_dack3_w) }
};

static I8237_INTERFACE( dma8237_2_config )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL }
};

static READ32_HANDLER(at_page32_r)
{
	return read32le_with_read8_handler(at_page8_r, space, offset, mem_mask);
}


static WRITE32_HANDLER(at_page32_w)
{
	write32le_with_write8_handler(at_page8_w, space, offset, data, mem_mask);
}

static READ32_DEVICE_HANDLER( ide_r )
{
	return ide_controller32_r(device, 0x1f0/4 + offset, mem_mask);
}

static WRITE32_DEVICE_HANDLER( ide_w )
{
	ide_controller32_w(device, 0x1f0/4 + offset, data, mem_mask);
}

static READ32_DEVICE_HANDLER( fdc_r )
{
	return ide_controller32_r(device, 0x3f0/4 + offset, mem_mask);
}

static WRITE32_DEVICE_HANDLER( fdc_w )
{
	//mame_printf_debug("FDC: write %08X, %08X, %08X\n", data, offset, mem_mask);
	ide_controller32_w(device, 0x3f0/4 + offset, data, mem_mask);
}

// Intel 82439TX System Controller (MXTC)
static UINT8 mxtc_config_reg[256];

static UINT8 mxtc_config_r(running_device *busdevice, running_device *device, int function, int reg)
{
//  mame_printf_debug("MXTC: read %d, %02X\n", function, reg);

	return mxtc_config_reg[reg];
}

static void mxtc_config_w(running_device *busdevice, running_device *device, int function, int reg, UINT8 data)
{
//  mame_printf_debug("%s:MXTC: write %d, %02X, %02X\n", cpuexec_describe_context(machine), function, reg, data);

	switch(reg)
	{
		case 0x59:		// PAM0
		{
			if (data & 0x10)		// enable RAM access to region 0xf0000 - 0xfffff
			{
				memory_set_bankptr(busdevice->machine, "bank1", bios_ram);
			}
			else					// disable RAM access (reads go to BIOS ROM)
			{
				memory_set_bankptr(busdevice->machine, "bank1", memory_region(busdevice->machine, "bios") + 0x10000);
			}
			break;
		}
	}

	mxtc_config_reg[reg] = data;
}

static void intel82439tx_init(void)
{
	mxtc_config_reg[0x60] = 0x02;
	mxtc_config_reg[0x61] = 0x02;
	mxtc_config_reg[0x62] = 0x02;
	mxtc_config_reg[0x63] = 0x02;
	mxtc_config_reg[0x64] = 0x02;
	mxtc_config_reg[0x65] = 0x02;
}

static UINT32 intel82439tx_pci_r(running_device *busdevice, running_device *device, int function, int reg, UINT32 mem_mask)
{
	UINT32 r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= mxtc_config_r(busdevice, device, function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= mxtc_config_r(busdevice, device, function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= mxtc_config_r(busdevice, device, function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= mxtc_config_r(busdevice, device, function, reg + 0) << 0;
	}
	return r;
}

static void intel82439tx_pci_w(running_device *busdevice, running_device *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		mxtc_config_w(busdevice, device, function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		mxtc_config_w(busdevice, device, function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		mxtc_config_w(busdevice, device, function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		mxtc_config_w(busdevice, device, function, reg + 0, (data >> 0) & 0xff);
	}
}

// Intel 82371AB PCI-to-ISA / IDE bridge (PIIX4)
static UINT8 piix4_config_reg[4][256];

static UINT8 piix4_config_r(running_device *busdevice, running_device *device, int function, int reg)
{
//  mame_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	return piix4_config_reg[function][reg];
}

static void piix4_config_w(running_device *busdevice, running_device *device, int function, int reg, UINT8 data)
{
//  mame_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", cpuexec_describe_context(machine), function, reg, data);
	piix4_config_reg[function][reg] = data;
}

static UINT32 intel82371ab_pci_r(running_device *busdevice, running_device *device, int function, int reg, UINT32 mem_mask)
{
	UINT32 r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 0) << 0;
	}
	return r;
}

static void intel82371ab_pci_w(running_device *busdevice, running_device *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		piix4_config_w(busdevice, device, function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		piix4_config_w(busdevice, device, function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		piix4_config_w(busdevice, device, function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		piix4_config_w(busdevice, device, function, reg + 0, (data >> 0) & 0xff);
	}
}

static WRITE32_HANDLER(bios_ram_w)
{
	if (mxtc_config_reg[0x59] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(bios_ram + offset);
	}
}

static ADDRESS_MAP_START( calchase_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM AM_BASE(&vga_vram)
	AM_RANGE(0x000c0000, 0x000c7fff) AM_RAM AM_REGION("video_bios", 0)
	AM_RANGE(0x000e0000, 0x000effff) AM_RAM
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROMBANK("bank1")
	AM_RANGE(0x000f0000, 0x000fffff) AM_WRITE(bios_ram_w)
	AM_RANGE(0x00100000, 0x01ffffff) AM_RAM
	AM_RANGE(0x04000000, 0x040001ff) AM_RAM
	AM_RANGE(0x08000000, 0x080001ff) AM_RAM
	AM_RANGE(0x0c000000, 0x0c0001ff) AM_RAM
	AM_RANGE(0x10000000, 0x100001ff) AM_RAM
	AM_RANGE(0x14000000, 0x140001ff) AM_RAM
	AM_RANGE(0x18000000, 0x180001ff) AM_RAM
	AM_RANGE(0x20000000, 0x200001ff) AM_RAM
	AM_RANGE(0x28000000, 0x280001ff) AM_RAM
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START( calchase_io, ADDRESS_SPACE_IO, 32)
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237_1", i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE(kbdc8042_32le_r,			kbdc8042_32le_w)
	AM_RANGE(0x0070, 0x007f) AM_READWRITE(mc146818_port32le_r,		mc146818_port32le_w)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE(at_page32_r,				at_page32_w)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE("dma8237_2", at32_dma8237_2_r, at32_dma8237_2_w)
	AM_RANGE(0x00e8, 0x00eb) AM_NOP
	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE("ide", ide_r, ide_w)
	AM_RANGE(0x0300, 0x03af) AM_NOP
	AM_RANGE(0x03b0, 0x03df) AM_NOP
	AM_RANGE(0x0278, 0x027b) AM_WRITENOP//AM_WRITE(pnp_config_w)
	AM_RANGE(0x03f0, 0x03ff) AM_DEVREADWRITE("ide", fdc_r, fdc_w)
	AM_RANGE(0x0a78, 0x0a7b) AM_WRITENOP//AM_WRITE(pnp_data_w)
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_32le_r,	pci_32le_w)
	AM_RANGE(0x43c0, 0x43cf) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x83c0, 0x83cf) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END


static const gfx_layout CGA_charlayout =
{
	8,8,
    256,
    1,
    { 0 },
    { 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
    8*8
};

static GFXDECODE_START( CGA )
	GFXDECODE_ENTRY( "video_bios", 0x5182, CGA_charlayout,              0, 256 )
	//there's also a 8x16 entry (just after the 8x8)
GFXDECODE_END

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(text) PORT_CODE(key1)

static INPUT_PORTS_START( calchase )
	PORT_START("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED ) 	/* unused scancode 0 */
	AT_KEYB_HELPER( 0x0002, "Esc",          KEYCODE_Q           ) /* Esc                         01  81 */

	PORT_START("pc_keyboard_1")
	AT_KEYB_HELPER( 0x0020, "Y",            KEYCODE_Y           ) /* Y                           15  95 */
	AT_KEYB_HELPER( 0x1000, "Enter",        KEYCODE_ENTER       ) /* Enter                       1C  9C */

	PORT_START("pc_keyboard_2")

	PORT_START("pc_keyboard_3")
	AT_KEYB_HELPER( 0x0002, "N",            KEYCODE_N           ) /* N                           31  B1 */
	AT_KEYB_HELPER( 0x0800, "F1",           KEYCODE_S           ) /* F1                          3B  BB */

	PORT_START("pc_keyboard_4")

	PORT_START("pc_keyboard_5")

	PORT_START("pc_keyboard_6")
	AT_KEYB_HELPER( 0x0040, "(MF2)Cursor Up",		KEYCODE_UP          ) /* Up                          67  e7 */
	AT_KEYB_HELPER( 0x0080, "(MF2)Page Up",			KEYCODE_PGUP        ) /* Page Up                     68  e8 */
	AT_KEYB_HELPER( 0x0100, "(MF2)Cursor Left",		KEYCODE_LEFT        ) /* Left                        69  e9 */
	AT_KEYB_HELPER( 0x0200, "(MF2)Cursor Right",	KEYCODE_RIGHT       ) /* Right                       6a  ea */
	AT_KEYB_HELPER( 0x0800, "(MF2)Cursor Down",		KEYCODE_DOWN        ) /* Down                        6c  ec */
	AT_KEYB_HELPER( 0x1000, "(MF2)Page Down",		KEYCODE_PGDN        ) /* Page Down                   6d  ed */
	AT_KEYB_HELPER( 0x4000, "Del",      		    KEYCODE_A           ) /* Delete                      6f  ef */

	PORT_START("pc_keyboard_7")
INPUT_PORTS_END

static IRQ_CALLBACK(irq_callback)
{
	int r = 0;
	r = pic8259_acknowledge( calchase_devices.pic8259_2);
	if (r==0)
	{
		r = pic8259_acknowledge( calchase_devices.pic8259_1);
	}
	return r;
}

static MACHINE_START(calchase)
{
	cpu_set_irq_callback(devtag_get_device(machine, "maincpu"), irq_callback);

	calchase_devices.pit8254 = devtag_get_device( machine, "pit8254" );
	calchase_devices.pic8259_1 = devtag_get_device( machine, "pic8259_1" );
	calchase_devices.pic8259_2 = devtag_get_device( machine, "pic8259_2" );
	calchase_devices.dma8237_1 = devtag_get_device( machine, "dma8237_1" );
	calchase_devices.dma8237_2 = devtag_get_device( machine, "dma8237_2" );
}

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

static WRITE_LINE_DEVICE_HANDLER( calchase_pic8259_1_set_int_line )
{
	cputag_set_input_line(device->machine, "maincpu", 0, state ? HOLD_LINE : CLEAR_LINE);
}

static const struct pic8259_interface calchase_pic8259_1_config =
{
	DEVCB_LINE(calchase_pic8259_1_set_int_line)
};

static const struct pic8259_interface calchase_pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir2_w)
};


/*************************************************************
 *
 * pit8254 configuration
 *
 *************************************************************/

static PIT8253_OUTPUT_CHANGED( pc_timer0_w )
{
	pic8259_ir0_w(calchase_devices.pic8259_1, state);
}

static const struct pit8253_config calchase_pit8254_config =
{
	{
		{
			4772720/4,				/* heartbeat IRQ */
			pc_timer0_w
		}, {
			4772720/4,				/* dram refresh */
			NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			NULL
		}
	}
};

static MACHINE_RESET(calchase)
{
	memory_set_bankptr(machine, "bank1", memory_region(machine, "bios") + 0x10000);
}

static void set_gate_a20(running_machine *machine, int a20)
{
	cputag_set_input_line(machine, "maincpu", INPUT_LINE_A20, a20);
}

static void keyboard_interrupt(running_machine *machine, int state)
{
	pic8259_ir1_w(calchase_devices.pic8259_1, state);
}

static void ide_interrupt(running_device *device, int state)
{
	pic8259_ir6_w(calchase_devices.pic8259_2, state);
}

static int calchase_get_out2(running_machine *machine) {
	return pit8253_get_output(calchase_devices.pit8254, 2 );
}

static const struct kbdc8042_interface at8042 =
{
	KBDC8042_AT386, set_gate_a20, keyboard_interrupt, calchase_get_out2
};

static void calchase_set_keyb_int(running_machine *machine, int state) {
	pic8259_ir1_w(calchase_devices.pic8259_1, state);
}


static MACHINE_DRIVER_START( calchase )
	MDRV_CPU_ADD("maincpu", PENTIUM, 200000000) // Cyrix 686MX-PR200 CPU
	MDRV_CPU_PROGRAM_MAP(calchase_map)
	MDRV_CPU_IO_MAP(calchase_io)

	MDRV_MACHINE_START(calchase)
	MDRV_MACHINE_RESET(calchase)

	MDRV_PIT8254_ADD( "pit8254", calchase_pit8254_config )
	MDRV_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )
	MDRV_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )
	MDRV_PIC8259_ADD( "pic8259_1", calchase_pic8259_1_config )
	MDRV_PIC8259_ADD( "pic8259_2", calchase_pic8259_2_config )
	MDRV_IDE_CONTROLLER_ADD("ide", ide_interrupt)
	MDRV_NVRAM_HANDLER( mc146818 )
	MDRV_PCI_BUS_ADD("pcibus", 0)
	MDRV_PCI_BUS_DEVICE(0, NULL, intel82439tx_pci_r, intel82439tx_pci_w)
	MDRV_PCI_BUS_DEVICE(7, NULL, intel82371ab_pci_r, intel82371ab_pci_w)

	MDRV_PALETTE_LENGTH(0x200)
	MDRV_GFXDECODE( CGA )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_VIDEO_START(calchase)
	MDRV_VIDEO_UPDATE(calchase)
MACHINE_DRIVER_END

static DRIVER_INIT( calchase )
{
	bios_ram = auto_alloc_array(machine, UINT32, 0x10000/4);

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT, calchase_set_keyb_int);
	mc146818_init(machine, MC146818_STANDARD);

	intel82439tx_init();

	kbdc8042_init(machine, &at8042);
}


ROM_START( calchase )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "mb_bios.bin", 0x00000, 0x20000, CRC(dea7a51b) SHA1(e2028c00bfa6d12959fc88866baca8b06a1eab68) )

	ROM_REGION( 0x8000, "video_bios", 0 )
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )

	ROM_REGION( 0x800, "nvram", 0 )
	ROM_LOAD( "ds1220y_nv.bin", 0x000, 0x800, CRC(7912c070) SHA1(b4c55c7ca76bcd8dad1c4b50297233349ae02ed3) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "calchase", 0,SHA1(487e304ffeed23ca618fa936258136605ce9d1a1) )
ROM_END


GAME( 1999, calchase,  0,    calchase, calchase,  calchase, ROT0, "The Game Room", "California Chase", GAME_NOT_WORKING|GAME_NO_SOUND )
