/*  Taito Wolf System

    Driver by Ville Linde

AMD M4-128N/64 stamped 'E58-01'
AMD MACH231 stamped 'E58-02'
AMD MACH211 stamped 'E58-03'
Zoom ZFX2
Zoom ZSG-2
Taito TC0510NIO
Panasonic MN1020019
1x RAM NEC 42S4260
1x RAM GM71C4400
12x RAM Alliance AS4C256K16E0-35 (256k x 16)
Mitsubishi M66220
Fujitsu MB87078
Atmel 93C66 EEPROM (4kb probably for high scores, test mode settings etc)
ICS GENDAC ICS5342-3
3DFX 500-0003-03 F805281.1 FBI
3DFX 500-0004-02 F804701.1 TMU
some logic
clocks 50MHz (near 3DFX) and 14.31818MHz (near RAMDAC)

*/

#define ENABLE_VGA 0

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
#if ENABLE_VGA
#include "video/pc_vga.h"
#endif

class taitowlf_state : public driver_device
{
public:
	taitowlf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 *m_bios_ram;
	UINT8 m_mxtc_config_reg[256];
	UINT8 m_piix4_config_reg[4][256];
	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];

	device_t	*m_pit8254;
	device_t	*m_pic8259_1;
	device_t	*m_pic8259_2;
	device_t	*m_dma8237_1;
	device_t	*m_dma8237_2;
};

#if !ENABLE_VGA
static SCREEN_UPDATE_RGB32( taitowlf )
{
	int x,y,count;
	const UINT8 *blit_ram = screen.machine().region("user5")->base();

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	count = (0);

	for(y=0;y<256;y++)
	{
		for(x=0;x<512;x++)
		{
			UINT32 color;

			color = (blit_ram[count] & 0xff);

			if(cliprect.contains(x+0, y))
				bitmap.pix32(y, x+0) = screen.machine().pens[color];

			count++;
		}
	}

	return 0;
}
#endif

static void ide_interrupt(device_t *device, int state);


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

static UINT8 mxtc_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	taitowlf_state *state = busdevice->machine().driver_data<taitowlf_state>();
//  mame_printf_debug("MXTC: read %d, %02X\n", function, reg);

	return state->m_mxtc_config_reg[reg];
}

static void mxtc_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	taitowlf_state *state = busdevice->machine().driver_data<taitowlf_state>();
//  mame_printf_debug("%s:MXTC: write %d, %02X, %02X\n", machine.describe_context(), function, reg, data);

	switch(reg)
	{
		case 0x59:		// PAM0
		{
			if (data & 0x10)		// enable RAM access to region 0xf0000 - 0xfffff
			{
				memory_set_bankptr(busdevice->machine(), "bank1", state->m_bios_ram);
			}
			else					// disable RAM access (reads go to BIOS ROM)
			{
				memory_set_bankptr(busdevice->machine(), "bank1", busdevice->machine().region("user1")->base() + 0x30000);
			}
			break;
		}
	}

	state->m_mxtc_config_reg[reg] = data;
}

static void intel82439tx_init(running_machine &machine)
{
	taitowlf_state *state = machine.driver_data<taitowlf_state>();
	state->m_mxtc_config_reg[0x60] = 0x02;
	state->m_mxtc_config_reg[0x61] = 0x02;
	state->m_mxtc_config_reg[0x62] = 0x02;
	state->m_mxtc_config_reg[0x63] = 0x02;
	state->m_mxtc_config_reg[0x64] = 0x02;
	state->m_mxtc_config_reg[0x65] = 0x02;
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

static UINT8 piix4_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	taitowlf_state *state = busdevice->machine().driver_data<taitowlf_state>();
//  mame_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	return state->m_piix4_config_reg[function][reg];
}

static void piix4_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	taitowlf_state *state = busdevice->machine().driver_data<taitowlf_state>();
//  mame_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", machine.describe_context(), function, reg, data);
	state->m_piix4_config_reg[function][reg] = data;
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
	taitowlf_state *state = space->machine().driver_data<taitowlf_state>();
	if (state->m_mxtc_config_reg[0x59] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(state->m_bios_ram + offset);
	}
}


/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/



static READ8_HANDLER(at_page8_r)
{
	taitowlf_state *state = space->machine().driver_data<taitowlf_state>();
	UINT8 data = state->m_at_pages[offset % 0x10];

	switch(offset % 8)
	{
	case 1:
		data = state->m_dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = state->m_dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = state->m_dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = state->m_dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}


static WRITE8_HANDLER(at_page8_w)
{
	taitowlf_state *state = space->machine().driver_data<taitowlf_state>();
	state->m_at_pages[offset % 0x10] = data;

	switch(offset % 8)
	{
	case 1:
		state->m_dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		state->m_dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		state->m_dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		state->m_dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}


static WRITE_LINE_DEVICE_HANDLER( pc_dma_hrq_changed )
{
	cputag_set_input_line(device->machine(), "maincpu", INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( device, state );
}


static READ8_HANDLER( pc_dma_read_byte )
{
	taitowlf_state *state = space->machine().driver_data<taitowlf_state>();
	offs_t page_offset = (((offs_t) state->m_dma_offset[0][state->m_dma_channel]) << 16)
		& 0xFF0000;

	return space->read_byte(page_offset + offset);
}


static WRITE8_HANDLER( pc_dma_write_byte )
{
	taitowlf_state *state = space->machine().driver_data<taitowlf_state>();
	offs_t page_offset = (((offs_t) state->m_dma_offset[0][state->m_dma_channel]) << 16)
		& 0xFF0000;

	space->write_byte(page_offset + offset, data);
}


static WRITE_LINE_DEVICE_HANDLER( pc_dack0_w )
{
	taitowlf_state *drvstate = device->machine().driver_data<taitowlf_state>();
	if (state) drvstate->m_dma_channel = 0;
}

static WRITE_LINE_DEVICE_HANDLER( pc_dack1_w )
{
	taitowlf_state *drvstate = device->machine().driver_data<taitowlf_state>();
	if (state) drvstate->m_dma_channel = 1;
}

static WRITE_LINE_DEVICE_HANDLER( pc_dack2_w )
{
	taitowlf_state *drvstate = device->machine().driver_data<taitowlf_state>();
	if (state) drvstate->m_dma_channel = 2;
}

static WRITE_LINE_DEVICE_HANDLER( pc_dack3_w )
{
	taitowlf_state *drvstate = device->machine().driver_data<taitowlf_state>();
	if (state) drvstate->m_dma_channel = 3;
}

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

static ADDRESS_MAP_START( taitowlf_map, AS_PROGRAM, 32, taitowlf_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM
	#if ENABLE_VGA
	AM_RANGE(0x000c0000, 0x000c7fff) AM_RAM AM_REGION("video_bios", 0)
	#else
	AM_RANGE(0x000c0000, 0x000c7fff) AM_NOP
	#endif
	AM_RANGE(0x000e0000, 0x000effff) AM_RAM
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROMBANK("bank1")
	AM_RANGE(0x000f0000, 0x000fffff) AM_WRITE_LEGACY(bios_ram_w)
	AM_RANGE(0x00100000, 0x01ffffff) AM_RAM
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("user1", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(taitowlf_io, AS_IO, 32, taitowlf_state )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8_LEGACY("dma8237_1", i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8_LEGACY("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE_LEGACY(kbdc8042_32le_r,			kbdc8042_32le_w)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", mc146818_device, read, write, 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE_LEGACY(at_page32_r,				at_page32_w)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE_LEGACY("dma8237_2", at32_dma8237_2_r, at32_dma8237_2_w)
	AM_RANGE(0x00e8, 0x00eb) AM_NOP
	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE_LEGACY("ide", ide_r, ide_w)
	AM_RANGE(0x0300, 0x03af) AM_NOP
	AM_RANGE(0x03b0, 0x03df) AM_NOP
	AM_RANGE(0x0278, 0x027b) AM_WRITE_LEGACY(pnp_config_w)
	AM_RANGE(0x03f0, 0x03ff) AM_DEVREADWRITE_LEGACY("ide", fdc_r, fdc_w)
	AM_RANGE(0x0a78, 0x0a7b) AM_WRITE_LEGACY(pnp_data_w)
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE_LEGACY("pcibus", pci_32le_r,	pci_32le_w)
ADDRESS_MAP_END

/*****************************************************************************/

#if 0
#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(text) PORT_CODE(key1)

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
#endif

static IRQ_CALLBACK(irq_callback)
{
	taitowlf_state *state = device->machine().driver_data<taitowlf_state>();
	return pic8259_acknowledge( state->m_pic8259_1);
}

static MACHINE_START(taitowlf)
{
	taitowlf_state *state = machine.driver_data<taitowlf_state>();
	device_set_irq_callback(machine.device("maincpu"), irq_callback);

	state->m_pit8254 = machine.device( "pit8254" );
	state->m_pic8259_1 = machine.device( "pic8259_1" );
	state->m_pic8259_2 = machine.device( "pic8259_2" );
	state->m_dma8237_1 = machine.device( "dma8237_1" );
	state->m_dma8237_2 = machine.device( "dma8237_2" );
}

static MACHINE_RESET(taitowlf)
{
	memory_set_bankptr(machine, "bank1", machine.region("user1")->base() + 0x30000);
}


/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

static WRITE_LINE_DEVICE_HANDLER( taitowlf_pic8259_1_set_int_line )
{
	cputag_set_input_line(device->machine(), "maincpu", 0, state ? HOLD_LINE : CLEAR_LINE);
}

static READ8_DEVICE_HANDLER( get_slave_ack )
{
	taitowlf_state *state = device->machine().driver_data<taitowlf_state>();
	if (offset==2) { // IRQ = 2
		return pic8259_acknowledge(state->m_pic8259_2);
	}
	return 0x00;
}

static const struct pic8259_interface taitowlf_pic8259_1_config =
{
	DEVCB_LINE(taitowlf_pic8259_1_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_HANDLER(get_slave_ack)
};

static const struct pic8259_interface taitowlf_pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir2_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
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

#if !ENABLE_VGA
/* debug purpose*/
static PALETTE_INIT( taitowlf )
{
	palette_set_color(machine,0x70,MAKE_RGB(0xff,0xff,0xff));
	palette_set_color(machine,0x71,MAKE_RGB(0xff,0xff,0xff));
	palette_set_color(machine,0x01,MAKE_RGB(0x55,0x00,0x00));
	palette_set_color(machine,0x10,MAKE_RGB(0xaa,0x00,0x00));
	palette_set_color(machine,0x00,MAKE_RGB(0x00,0x00,0x00));
}
#endif

static MACHINE_CONFIG_START( taitowlf, taitowlf_state )

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
	MCFG_IDE_CONTROLLER_ADD("ide", ide_interrupt, ide_devices, "hdd", NULL)
	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )

	/* video hardware */
	#if ENABLE_VGA
	MCFG_FRAGMENT_ADD( pcvideo_vga )
	#else
	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_PALETTE_LENGTH(256)
	MCFG_SCREEN_UPDATE_STATIC(taitowlf)
	MCFG_PALETTE_INIT(taitowlf)
	#endif
MACHINE_CONFIG_END

static void set_gate_a20(running_machine &machine, int a20)
{
	cputag_set_input_line(machine, "maincpu", INPUT_LINE_A20, a20);
}

static void keyboard_interrupt(running_machine &machine, int state)
{
	taitowlf_state *drvstate = machine.driver_data<taitowlf_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}

static void ide_interrupt(device_t *device, int state)
{
	taitowlf_state *drvstate = device->machine().driver_data<taitowlf_state>();
	pic8259_ir6_w(drvstate->m_pic8259_2, state);
}

static int taitowlf_get_out2(running_machine &machine)
{
	taitowlf_state *state = machine.driver_data<taitowlf_state>();
	return pit8253_get_output(state->m_pit8254, 2 );
}

static const struct kbdc8042_interface at8042 =
{
	KBDC8042_AT386, set_gate_a20, keyboard_interrupt, NULL, taitowlf_get_out2
};

static void taitowlf_set_keyb_int(running_machine &machine, int state)
{
	taitowlf_state *drvstate = machine.driver_data<taitowlf_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}

#if ENABLE_VGA
static READ8_HANDLER( vga_setting ) { return 0xff; } // hard-code to color
#endif

static DRIVER_INIT( taitowlf )
{
	taitowlf_state *state = machine.driver_data<taitowlf_state>();
	state->m_bios_ram = auto_alloc_array(machine, UINT32, 0x10000/4);

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT, taitowlf_set_keyb_int);

	intel82439tx_init(machine);

	kbdc8042_init(machine, &at8042);
	#if ENABLE_VGA
	pc_vga_init(machine, vga_setting, NULL);
	pc_vga_io_init(machine, machine.device("maincpu")->memory().space(AS_PROGRAM), 0xa0000, machine.device("maincpu")->memory().space(AS_IO), 0x0000);
	#endif
}

/*****************************************************************************/

ROM_START(pf2012)
	ROM_REGION32_LE(0x40000, "user1", 0)
	ROM_LOAD("p5tx-la.bin", 0x00000, 0x40000, CRC(072e6d51) SHA1(70414349b37e478fc28ecbaba47ad1033ae583b7))

	#if ENABLE_VGA
	ROM_REGION( 0x8000, "video_bios", 0 ) // debug
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )
	#endif

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

GAME(1997, pf2012, 0,	taitowlf, pc_keyboard, taitowlf,	ROT0,   "Taito",  "Psychic Force 2012", GAME_NOT_WORKING | GAME_NO_SOUND)
