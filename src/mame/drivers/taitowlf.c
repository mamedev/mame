/*  Taito Wolf System

    Driver by Ville Linde
*/

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

static void ide_interrupt(device_t *device, int state);

static UINT32 *cga_ram;
static UINT32 *bios_ram;

static struct {
	device_t	*pit8254;
	device_t	*pic8259_1;
	device_t	*pic8259_2;
	device_t	*dma8237_1;
	device_t	*dma8237_2;
} taitowlf_devices;


static const rgb_t cga_palette[16] =
{
	MAKE_RGB( 0x00, 0x00, 0x00 ), MAKE_RGB( 0x00, 0x00, 0xaa ), MAKE_RGB( 0x00, 0xaa, 0x00 ), MAKE_RGB( 0x00, 0xaa, 0xaa ),
	MAKE_RGB( 0xaa, 0x00, 0x00 ), MAKE_RGB( 0xaa, 0x00, 0xaa ), MAKE_RGB( 0xaa, 0x55, 0x00 ), MAKE_RGB( 0xaa, 0xaa, 0xaa ),
	MAKE_RGB( 0x55, 0x55, 0x55 ), MAKE_RGB( 0x55, 0x55, 0xff ), MAKE_RGB( 0x55, 0xff, 0x55 ), MAKE_RGB( 0x55, 0xff, 0xff ),
	MAKE_RGB( 0xff, 0x55, 0x55 ), MAKE_RGB( 0xff, 0x55, 0xff ), MAKE_RGB( 0xff, 0xff, 0x55 ), MAKE_RGB( 0xff, 0xff, 0xff ),
};

static VIDEO_START(taitowlf)
{
	int i;
	for (i=0; i < 16; i++)
	{
		palette_set_color(machine, i, cga_palette[i]);
	}
}

static void draw_char(bitmap_t *bitmap, const rectangle *cliprect, const gfx_element *gfx, int ch, int att, int x, int y)
{
	int i,j;
	const UINT8 *dp;
	int index = 0;
	dp = gfx_element_get_data(gfx, ch);

	for (j=y; j < y+8; j++)
	{
		UINT16 *p = BITMAP_ADDR16(bitmap, j, 0);
		for (i=x; i < x+8; i++)
		{
			UINT8 pen = dp[index++];
			if (pen)
				p[i] = gfx->color_base + (att & 0xf);
			else
				p[i] = gfx->color_base  + ((att >> 4) & 0x7);
		}
	}
}

static VIDEO_UPDATE(taitowlf)
{
	int i, j;
	const gfx_element *gfx = screen->machine->gfx[0];
	UINT32 *cga = cga_ram;
	int index = 0;

	bitmap_fill(bitmap, cliprect, 0);

	for (j=0; j < 25; j++)
	{
		for (i=0; i < 80; i+=2)
		{
			int att0 = (cga[index] >> 8) & 0xff;
			int ch0 = (cga[index] >> 0) & 0xff;
			int att1 = (cga[index] >> 24) & 0xff;
			int ch1 = (cga[index] >> 16) & 0xff;

			draw_char(bitmap, cliprect, gfx, ch0, att0, i*8, j*8);
			draw_char(bitmap, cliprect, gfx, ch1, att1, (i*8)+8, j*8);
			index++;
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



// Intel 82439TX System Controller (MXTC)
static UINT8 mxtc_config_reg[256];

static UINT8 mxtc_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
//  mame_printf_debug("MXTC: read %d, %02X\n", function, reg);

	return mxtc_config_reg[reg];
}

static void mxtc_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
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
				memory_set_bankptr(busdevice->machine, "bank1", busdevice->machine->region("user1")->base() + 0x30000);
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

static UINT32 intel82439tx_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
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

static void intel82439tx_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
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

static UINT8 piix4_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
//  mame_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	return piix4_config_reg[function][reg];
}

static void piix4_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
//  mame_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", cpuexec_describe_context(machine), function, reg, data);
	piix4_config_reg[function][reg] = data;
}

static UINT32 intel82371ab_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
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

static void intel82371ab_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
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

// ISA Plug-n-Play
static WRITE32_HANDLER( pnp_config_w )
{
	if (ACCESSING_BITS_8_15)
	{
//      mame_printf_debug("PNP Config: %02X\n", (data >> 8) & 0xff);
	}
}

static WRITE32_HANDLER( pnp_data_w )
{
	if (ACCESSING_BITS_8_15)
	{
//      mame_printf_debug("PNP Data: %02X\n", (data >> 8) & 0xff);
	}
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



static WRITE32_HANDLER(bios_ram_w)
{
	if (mxtc_config_reg[0x59] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(bios_ram + offset);
	}
}


/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/

static int dma_channel;
static UINT8 dma_offset[2][4];
static UINT8 at_pages[0x10];


static READ8_HANDLER(at_page8_r)
{
	UINT8 data = at_pages[offset % 0x10];

	switch(offset % 8)
	{
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

	switch(offset % 8)
	{
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

	return space->read_byte(page_offset + offset);
}


static WRITE8_HANDLER( pc_dma_write_byte )
{
	offs_t page_offset = (((offs_t) dma_offset[0][dma_channel]) << 16)
		& 0xFF0000;

	space->write_byte(page_offset + offset, data);
}


static WRITE_LINE_DEVICE_HANDLER( pc_dack0_w ) { if (state) dma_channel = 0; }
static WRITE_LINE_DEVICE_HANDLER( pc_dack1_w ) { if (state) dma_channel = 1; }
static WRITE_LINE_DEVICE_HANDLER( pc_dack2_w ) { if (state) dma_channel = 2; }
static WRITE_LINE_DEVICE_HANDLER( pc_dack3_w ) { if (state) dma_channel = 3; }

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


/*****************************************************************************/

static ADDRESS_MAP_START( taitowlf_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000affff) AM_RAM
	AM_RANGE(0x000b0000, 0x000b7fff) AM_RAM AM_BASE(&cga_ram)
	AM_RANGE(0x000e0000, 0x000effff) AM_RAM
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROMBANK("bank1")
	AM_RANGE(0x000f0000, 0x000fffff) AM_WRITE(bios_ram_w)
	AM_RANGE(0x00100000, 0x01ffffff) AM_RAM
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("user1", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(taitowlf_io, ADDRESS_SPACE_IO, 32)
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237_1", i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE(kbdc8042_32le_r,			kbdc8042_32le_w)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8_MODERN("rtc", mc146818_device, read, write, 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE(at_page32_r,				at_page32_w)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE("dma8237_2", at32_dma8237_2_r, at32_dma8237_2_w)
	AM_RANGE(0x00e8, 0x00eb) AM_NOP
	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE("ide", ide_r, ide_w)
	AM_RANGE(0x0300, 0x03af) AM_NOP
	AM_RANGE(0x03b0, 0x03df) AM_NOP
	AM_RANGE(0x0278, 0x027b) AM_WRITE(pnp_config_w)
	AM_RANGE(0x03f0, 0x03ff) AM_DEVREADWRITE("ide", fdc_r, fdc_w)
	AM_RANGE(0x0a78, 0x0a7b) AM_WRITE(pnp_data_w)
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_32le_r,	pci_32le_w)
ADDRESS_MAP_END

/*****************************************************************************/

static const gfx_layout CGA_charlayout =
{
	8,8,					/* 8 x 16 characters */
    256,                    /* 256 characters */
    1,                      /* 1 bits per pixel */
    { 0 },                  /* no bitplanes; 1 bit per pixel */
    /* x offsets */
    { 0,1,2,3,4,5,6,7 },
    /* y offsets */
	{ 0*8,1*8,2*8,3*8,
	  4*8,5*8,6*8,7*8 },
    8*8                     /* every char takes 8 bytes */
};

static GFXDECODE_START( CGA )
/* Support up to four CGA fonts */
	GFXDECODE_ENTRY( "gfx1", 0x0000, CGA_charlayout,              0, 256 )   /* Font 0 */
	GFXDECODE_ENTRY( "gfx1", 0x0800, CGA_charlayout,              0, 256 )   /* Font 1 */
	GFXDECODE_ENTRY( "gfx1", 0x1000, CGA_charlayout,              0, 256 )   /* Font 2 */
	GFXDECODE_ENTRY( "gfx1", 0x1800, CGA_charlayout,              0, 256 )   /* Font 3*/
GFXDECODE_END

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(text) PORT_CODE(key1)

static INPUT_PORTS_START(taitowlf)
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
	r = pic8259_acknowledge( taitowlf_devices.pic8259_2);
	if (r==0)
	{
		r = pic8259_acknowledge( taitowlf_devices.pic8259_1);
	}
	return r;
}

static MACHINE_START(taitowlf)
{
	cpu_set_irq_callback(machine->device("maincpu"), irq_callback);

	taitowlf_devices.pit8254 = machine->device( "pit8254" );
	taitowlf_devices.pic8259_1 = machine->device( "pic8259_1" );
	taitowlf_devices.pic8259_2 = machine->device( "pic8259_2" );
	taitowlf_devices.dma8237_1 = machine->device( "dma8237_1" );
	taitowlf_devices.dma8237_2 = machine->device( "dma8237_2" );
}

static MACHINE_RESET(taitowlf)
{
	memory_set_bankptr(machine, "bank1", machine->region("user1")->base() + 0x30000);
}


/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

static WRITE_LINE_DEVICE_HANDLER( taitowlf_pic8259_1_set_int_line )
{
	cputag_set_input_line(device->machine, "maincpu", 0, state ? HOLD_LINE : CLEAR_LINE);
}

static const struct pic8259_interface taitowlf_pic8259_1_config =
{
	DEVCB_LINE(taitowlf_pic8259_1_set_int_line)
};

static const struct pic8259_interface taitowlf_pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir2_w)
};


/*************************************************************
 *
 * pit8254 configuration
 *
 *************************************************************/

static const struct pit8253_config taitowlf_pit8254_config =
{
	{
		{
			4772720/4,				/* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir0_w)
		}, {
			4772720/4,				/* dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};

static MACHINE_CONFIG_START( taitowlf, driver_device )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM, 200000000)
	MCFG_CPU_PROGRAM_MAP(taitowlf_map)
	MCFG_CPU_IO_MAP(taitowlf_io)

	MCFG_MACHINE_START(taitowlf)
	MCFG_MACHINE_RESET(taitowlf)

	MCFG_PCI_BUS_ADD("pcibus", 0)
	MCFG_PCI_BUS_DEVICE(0, NULL, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_DEVICE(7, NULL, intel82371ab_pci_r, intel82371ab_pci_w)

	MCFG_PIT8254_ADD( "pit8254", taitowlf_pit8254_config )

	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )

	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )

	MCFG_PIC8259_ADD( "pic8259_1", taitowlf_pic8259_1_config )

	MCFG_PIC8259_ADD( "pic8259_2", taitowlf_pic8259_2_config )

	MCFG_IDE_CONTROLLER_ADD("ide", ide_interrupt)

	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 199)

	MCFG_GFXDECODE(CGA)
	MCFG_PALETTE_LENGTH(16)

	MCFG_VIDEO_START(taitowlf)
	MCFG_VIDEO_UPDATE(taitowlf)

MACHINE_CONFIG_END

static void set_gate_a20(running_machine *machine, int a20)
{
	cputag_set_input_line(machine, "maincpu", INPUT_LINE_A20, a20);
}

static void keyboard_interrupt(running_machine *machine, int state)
{
	pic8259_ir1_w(taitowlf_devices.pic8259_1, state);
}

static void ide_interrupt(device_t *device, int state)
{
	pic8259_ir6_w(taitowlf_devices.pic8259_2, state);
}

static int taitowlf_get_out2(running_machine *machine)
{
	return pit8253_get_output(taitowlf_devices.pit8254, 2 );
}

static const struct kbdc8042_interface at8042 =
{
	KBDC8042_AT386, set_gate_a20, keyboard_interrupt, taitowlf_get_out2
};

static void taitowlf_set_keyb_int(running_machine *machine, int state)
{
	pic8259_ir1_w(taitowlf_devices.pic8259_1, state);
}

static DRIVER_INIT( taitowlf )
{
	bios_ram = auto_alloc_array(machine, UINT32, 0x10000/4);

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT, taitowlf_set_keyb_int);

	intel82439tx_init();

	kbdc8042_init(machine, &at8042);
}

/*****************************************************************************/

ROM_START(pf2012)
	ROM_REGION32_LE(0x40000, "user1", 0)
	ROM_LOAD("p5tx-la.bin", 0x00000, 0x40000, CRC(072e6d51) SHA1(70414349b37e478fc28ecbaba47ad1033ae583b7))

	ROM_REGION(0x08100, "gfx1", 0)
    ROM_LOAD("cga.chr",     0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))

	ROM_REGION32_LE(0x400000, "user3", 0)		// Program ROM disk
	ROM_LOAD("u1.bin", 0x000000, 0x200000, CRC(8f4c09cb) SHA1(0969a92fec819868881683c580f9e01cbedf4ad2))
	ROM_LOAD("u2.bin", 0x200000, 0x200000, CRC(59881781) SHA1(85ff074ab2a922eac37cf96f0bf153a2dac55aa4))

	ROM_REGION32_LE(0x4000000, "user4", 0)		// Data ROM disk
	ROM_LOAD("e59-01.u20", 0x0000000, 0x800000, CRC(701d3a9a) SHA1(34c9f34f4da34bb8eed85a4efd1d9eea47a21d77) )
	ROM_LOAD("e59-02.u23", 0x0800000, 0x800000, CRC(626df682) SHA1(35bb4f91201734ce7ccdc640a75030aaca3d1151) )
	ROM_LOAD("e59-03.u26", 0x1000000, 0x800000, CRC(74e4efde) SHA1(630235c2e4a11f615b5f3b8c93e1e645da09eefe) )
	ROM_LOAD("e59-04.u21", 0x1800000, 0x800000, CRC(c900e8df) SHA1(93c06b8f5082e33f0dcc41f1be6a79283de16c40) )
	ROM_LOAD("e59-05.u24", 0x2000000, 0x800000, CRC(85b0954c) SHA1(1b533d5888d56d1510c79f790e4fa708f77e836f) )
	ROM_LOAD("e59-06.u27", 0x2800000, 0x800000, CRC(0573a113) SHA1(ee76a71dfd31289a9a5428653a36d01d914fc5d9) )
	ROM_LOAD("e59-07.u22", 0x3000000, 0x800000, CRC(1f0ddcdc) SHA1(72ffe08f5effab093bdfe9863f8a11f80e914272) )
	ROM_LOAD("e59-08.u25", 0x3800000, 0x800000, CRC(8db38ffd) SHA1(4b71ea86fb774ba6a8ac45abf4191af64af007e7) )

	ROM_REGION(0x1400000, "samples", 0)			// ZOOM sample data
	ROM_LOAD("e59-09.u29", 0x0000000, 0x800000, CRC(d0da5c50) SHA1(56fb3c38f35244720d32a44fed28e6b58c7851f7) )
	ROM_LOAD("e59-10.u32", 0x0800000, 0x800000, CRC(4c0e0a5c) SHA1(6454befa3a1dd532eb2a760129dcd7e611508730) )
	ROM_LOAD("e59-11.u33", 0x1000000, 0x400000, CRC(c90a896d) SHA1(2b62992f20e4ca9634e7953fe2c553906de44f04) )

	ROM_REGION(0x180000, "cpu1", 0)			// MN10200 program
	ROM_LOAD("e59-12.u13", 0x000000, 0x80000, CRC(9a473a7e) SHA1(b0ec7b0ae2b33a32da98899aa79d44e8e318ceb7) )
	ROM_LOAD("e59-13.u15", 0x080000, 0x80000, CRC(77719880) SHA1(8382dd2dfb0dae60a3831ed6d3ff08539e2d94eb) )
	ROM_LOAD("e59-14.u14", 0x100000, 0x40000, CRC(d440887c) SHA1(d965871860d757bc9111e9adb2303a633c662d6b) )
	ROM_LOAD("e59-15.u16", 0x140000, 0x40000, CRC(eae8e523) SHA1(8a054d3ded7248a7906c4f0bec755ddce53e2023) )

	ROM_REGION(0x20000, "user5", 0)			// bootscreen
	ROM_LOAD("e58-04.u71", 0x000000, 0x20000, CRC(500e6113) SHA1(93226706517c02e336f96bdf9443785158e7becf) )
ROM_END

/*****************************************************************************/

GAME(1997, pf2012, 0,	taitowlf, taitowlf, taitowlf,	ROT0,   "Taito",  "Psychic Force 2012", GAME_NOT_WORKING | GAME_NO_SOUND)
