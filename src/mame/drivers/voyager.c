/************************************************************************************

Star Trek Voyager (c) 2002 Team Play, Inc. / Game Refuge / Monaco Entertainment

skeleton driver by R. Belmont

Motherboard is FIC AZIIEA with AMD Duron processor of unknown speed
Chipset: VIA KT133a with VT8363A Northbridge and VT82C686B Southbridge
Video: Jaton 3DForce2MX-32, based on Nvidia GeForce 2MX chipset w/32 MB of VRAM

TODO: VIA KT133a chipset support, GeForce 2MX video support, lots of things ;-)

*************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/pcshare.h"
#include "machine/pci.h"
#include "machine/8042kbdc.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "video/pc_vga.h"

class voyager_state : public driver_device
{
public:
	voyager_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
		  { }

	UINT32 *m_bios_ram;
	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];
	UINT8 m_mxtc_config_reg[256];
	UINT8 m_piix4_config_reg[4][256];

	device_t	*m_pit8254;
	device_t	*m_pic8259_1;
	device_t	*m_pic8259_2;
	device_t	*m_dma8237_1;
	device_t	*m_dma8237_2;

	UINT32 m_idle_skip_ram;
	required_device<cpu_device> m_maincpu;
	DECLARE_READ8_MEMBER(at_page8_r);
	DECLARE_WRITE8_MEMBER(at_page8_w);
	DECLARE_READ8_MEMBER(pc_dma_read_byte);
	DECLARE_WRITE8_MEMBER(pc_dma_write_byte);
	DECLARE_WRITE32_MEMBER(bios_ram_w);
	DECLARE_READ8_MEMBER(at_dma8237_2_r);
	DECLARE_WRITE8_MEMBER(at_dma8237_2_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dma_hrq_changed);
	DECLARE_WRITE_LINE_MEMBER(pc_dack0_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack1_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack2_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack3_w);
	DECLARE_READ32_MEMBER(ide_r);
	DECLARE_WRITE32_MEMBER(ide_w);
	DECLARE_READ32_MEMBER(fdc_r);
	DECLARE_WRITE32_MEMBER(fdc_w);
	DECLARE_WRITE_LINE_MEMBER(voyager_pic8259_1_set_int_line);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_DRIVER_INIT(voyager);
};


static void ide_interrupt(device_t *device, int state);


READ8_MEMBER(voyager_state::at_dma8237_2_r)
{
	device_t *device = machine().device("dma8237_2");
	return i8237_r(device, offset / 2);
}

WRITE8_MEMBER(voyager_state::at_dma8237_2_w)
{
	device_t *device = machine().device("dma8237_2");
	i8237_w(device, offset / 2, data);
}

READ8_MEMBER(voyager_state::at_page8_r)
{
	UINT8 data = m_at_pages[offset % 0x10];

	switch(offset % 8) {
	case 1:
		data = m_dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = m_dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = m_dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = m_dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}


WRITE8_MEMBER(voyager_state::at_page8_w)
{
	m_at_pages[offset % 0x10] = data;

	switch(offset % 8) {
	case 1:
		m_dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		m_dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		m_dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		m_dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}


WRITE_LINE_MEMBER(voyager_state::pc_dma_hrq_changed)
{
	machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( m_dma8237_1, state );
}


READ8_MEMBER(voyager_state::pc_dma_read_byte)
{
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	return space.read_byte(page_offset + offset);
}


WRITE8_MEMBER(voyager_state::pc_dma_write_byte)
{
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	space.write_byte(page_offset + offset, data);
}

static void set_dma_channel(device_t *device, int channel, int state)
{
	voyager_state *drvstate = device->machine().driver_data<voyager_state>();
	if (!state) drvstate->m_dma_channel = channel;
}

WRITE_LINE_MEMBER(voyager_state::pc_dack0_w){ set_dma_channel(m_dma8237_1, 0, state); }
WRITE_LINE_MEMBER(voyager_state::pc_dack1_w){ set_dma_channel(m_dma8237_1, 1, state); }
WRITE_LINE_MEMBER(voyager_state::pc_dack2_w){ set_dma_channel(m_dma8237_1, 2, state); }
WRITE_LINE_MEMBER(voyager_state::pc_dack3_w){ set_dma_channel(m_dma8237_1, 3, state); }

static I8237_INTERFACE( dma8237_1_config )
{
	DEVCB_DRIVER_LINE_MEMBER(voyager_state,pc_dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(voyager_state, pc_dma_read_byte),
	DEVCB_DRIVER_MEMBER(voyager_state, pc_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_DRIVER_LINE_MEMBER(voyager_state,pc_dack0_w), DEVCB_DRIVER_LINE_MEMBER(voyager_state,pc_dack1_w), DEVCB_DRIVER_LINE_MEMBER(voyager_state,pc_dack2_w), DEVCB_DRIVER_LINE_MEMBER(voyager_state,pc_dack3_w) }
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

READ32_MEMBER(voyager_state::ide_r)
{
	device_t *device = machine().device("ide");
	return ide_controller32_r(device, 0x1f0/4 + offset, mem_mask);
}

WRITE32_MEMBER(voyager_state::ide_w)
{
	device_t *device = machine().device("ide");
	ide_controller32_w(device, 0x1f0/4 + offset, data, mem_mask);
}





READ32_MEMBER(voyager_state::fdc_r)
{
	device_t *device = machine().device("ide");
	return ide_controller32_r(device, 0x3f0/4 + offset, mem_mask);
}

WRITE32_MEMBER(voyager_state::fdc_w)
{
	device_t *device = machine().device("ide");
	//mame_printf_debug("FDC: write %08X, %08X, %08X\n", data, offset, mem_mask);
	ide_controller32_w(device, 0x3f0/4 + offset, data, mem_mask);
}


// Intel 82439TX System Controller (MXTC)

static UINT8 mxtc_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	voyager_state *state = busdevice->machine().driver_data<voyager_state>();
//  mame_printf_debug("MXTC: read %d, %02X\n", function, reg);

	return state->m_mxtc_config_reg[reg];
}

static void mxtc_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	voyager_state *state = busdevice->machine().driver_data<voyager_state>();
//  mame_printf_debug("%s:MXTC: write %d, %02X, %02X\n", machine.describe_context(), function, reg, data);

	switch(reg)
	{
		//case 0x59:
		case 0x63:	// PAM0
		{
			//if (data & 0x10)     // enable RAM access to region 0xf0000 - 0xfffff
			if ((data & 0x50) | (data & 0xA0))
			{
				state->membank("bank1")->set_base(state->m_bios_ram);
			}
			else				// disable RAM access (reads go to BIOS ROM)
			{
				//Execution Hack to avoid crash when switch back from Shadow RAM to Bios ROM, since i386 emu haven't yet pipelined execution structure.
				//It happens when exit from BIOS SETUP.
				#if 0
				if ((state->m_mxtc_config_reg[0x63] & 0x50) | ( state->m_mxtc_config_reg[0x63] & 0xA0)) // Only DO if comes a change to disable ROM.
				{
					if ( busdevice->machine(->safe_pc().device("maincpu"))==0xff74e) busdevice->machine().device("maincpu")->state().set_pc(0xff74d);
				}
				#endif

				state->membank("bank1")->set_base(busdevice->machine().root_device().memregion("bios")->base() + 0x10000);
				state->membank("bank1")->set_base(busdevice->machine().root_device().memregion("bios")->base());
			}
			break;
		}
	}

	state->m_mxtc_config_reg[reg] = data;
}

static void intel82439tx_init(running_machine &machine)
{
	voyager_state *state = machine.driver_data<voyager_state>();
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

	if(reg == 0)
		return 0x05851106; // VT82C585VPX, VIA

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
	voyager_state *state = busdevice->machine().driver_data<voyager_state>();
//  mame_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	return state->m_piix4_config_reg[function][reg];
}

static void piix4_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	voyager_state *state = busdevice->machine().driver_data<voyager_state>();
//  mame_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", machine.describe_context(), function, reg, data);
	state->m_piix4_config_reg[function][reg] = data;
}

static UINT32 intel82371ab_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	UINT32 r = 0;

	if(reg == 0)
		return 0x30401106; // VT82C586B, VIA

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

WRITE32_MEMBER(voyager_state::bios_ram_w)
{
	//if (m_mxtc_config_reg[0x59] & 0x20)       // write to RAM if this region is write-enabled
	       if (m_mxtc_config_reg[0x63] & 0x50)
	{
		COMBINE_DATA(m_bios_ram + offset);
	}
}

static ADDRESS_MAP_START( voyager_map, AS_PROGRAM, 32, voyager_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM // VGA VRAM
	AM_RANGE(0x000c0000, 0x000c7fff) AM_RAM AM_REGION("video_bios", 0)
	AM_RANGE(0x000c8000, 0x000cffff) AM_NOP
	//AM_RANGE(0x000d0000, 0x000d0003) AM_RAM  // XYLINX - Sincronus serial communication
	AM_RANGE(0x000d0008, 0x000d000b) AM_WRITENOP // ???
	AM_RANGE(0x000d0800, 0x000d0fff) AM_ROM AM_REGION("nvram",0) //
	AM_RANGE(0x000d0800, 0x000d0fff) AM_RAM  // GAME_CMOS

	//GRULL AM_RANGE(0x000e0000, 0x000effff) AM_RAM
	//GRULL-AM_RANGE(0x000f0000, 0x000fffff) AM_ROMBANK("bank1")
	//GRULL AM_RANGE(0x000f0000, 0x000fffff) AM_WRITE(bios_ram_w)
	AM_RANGE(0x000e0000, 0x000fffff) AM_ROMBANK("bank1")
	AM_RANGE(0x000e0000, 0x000fffff) AM_WRITE(bios_ram_w)
	AM_RANGE(0x00100000, 0x03ffffff) AM_RAM  // 64MB
	AM_RANGE(0x02000000, 0x28ffffff) AM_NOP
	//AM_RANGE(0x04000000, 0x040001ff) AM_RAM
	//AM_RANGE(0x08000000, 0x080001ff) AM_RAM
	//AM_RANGE(0x0c000000, 0x0c0001ff) AM_RAM
	//AM_RANGE(0x10000000, 0x100001ff) AM_RAM
	//AM_RANGE(0x14000000, 0x140001ff) AM_RAM
	//AM_RANGE(0x18000000, 0x180001ff) AM_RAM
	//AM_RANGE(0x20000000, 0x200001ff) AM_RAM
	//AM_RANGE(0x28000000, 0x280001ff) AM_RAM
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START( voyager_io, AS_IO, 32, voyager_state )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8_LEGACY("dma8237_1", i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8_LEGACY("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE8_LEGACY(kbdc8042_8_r, kbdc8042_8_w, 0xffffffff)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", mc146818_device, read, write, 0xffffffff) /* todo: nvram (CMOS Setup Save)*/
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(at_page8_r, at_page8_w, 0xffffffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_READWRITE8(at_dma8237_2_r, at_dma8237_2_w, 0xffffffff)
	//AM_RANGE(0x00e8, 0x00eb) AM_NOP
	AM_RANGE(0x00e8, 0x00ef) AM_NOP //AMI BIOS write to this ports as delays between I/O ports operations sending al value -> NEWIODELAY
	AM_RANGE(0x0170, 0x0177) AM_NOP //To debug
	AM_RANGE(0x01f0, 0x01f7) AM_READWRITE(ide_r, ide_w)
	AM_RANGE(0x0200, 0x021f) AM_NOP //To debug
	AM_RANGE(0x0260, 0x026f) AM_NOP //To debug
	AM_RANGE(0x0278, 0x027b) AM_WRITENOP//AM_WRITE_LEGACY(pnp_config_w)
	AM_RANGE(0x0280, 0x0287) AM_NOP //To debug
	AM_RANGE(0x02a0, 0x02a7) AM_NOP //To debug
	AM_RANGE(0x02c0, 0x02c7) AM_NOP //To debug
	AM_RANGE(0x02e0, 0x02ef) AM_NOP //To debug
	AM_RANGE(0x0278, 0x02ff) AM_NOP //To debug
	AM_RANGE(0x02f8, 0x02ff) AM_NOP //To debug
	AM_RANGE(0x0320, 0x038f) AM_NOP //To debug
	AM_RANGE(0x03a0, 0x03a7) AM_NOP //To debug
	AM_RANGE(0x03e0, 0x03ef) AM_NOP //To debug
	AM_RANGE(0x0378, 0x037f) AM_NOP //To debug
	// AM_RANGE(0x0300, 0x03af) AM_NOP
	// AM_RANGE(0x03b0, 0x03df) AM_NOP
	AM_RANGE(0x03f0, 0x03f7) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0x03f8, 0x03ff) AM_NOP // To debug Serial Port COM1:
	AM_RANGE(0x0a78, 0x0a7b) AM_WRITENOP//AM_WRITE_LEGACY(pnp_data_w)
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)
	AM_RANGE(0x42e8, 0x43ef) AM_NOP //To debug
	AM_RANGE(0x43c0, 0x43cf) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x46e8, 0x46ef) AM_NOP //To debug
	AM_RANGE(0x4ae8, 0x4aef) AM_NOP //To debug
	AM_RANGE(0x83c0, 0x83cf) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x92e8, 0x92ef) AM_NOP //To debug

ADDRESS_MAP_END

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(text) PORT_CODE(key1)



#if 1
static INPUT_PORTS_START( voyager )
	PORT_START("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED ) 	/* unused scancode 0 */
	AT_KEYB_HELPER( 0x0002, "Esc",          KEYCODE_Q           ) /* Esc                         01  81 */

	PORT_START("pc_keyboard_1")
	AT_KEYB_HELPER( 0x0010, "T",            KEYCODE_T           ) /* T                           14  94 */
	AT_KEYB_HELPER( 0x0020, "Y",            KEYCODE_Y           ) /* Y                           15  95 */
	AT_KEYB_HELPER( 0x0100, "O",            KEYCODE_O           ) /* O                           18  98 */
	AT_KEYB_HELPER( 0x1000, "Enter",        KEYCODE_ENTER       ) /* Enter                       1C  9C */

	PORT_START("pc_keyboard_2")

	PORT_START("pc_keyboard_3")
	AT_KEYB_HELPER( 0x0001, "B",            KEYCODE_B           ) /* B                           30  B0 */
	AT_KEYB_HELPER( 0x0002, "N",            KEYCODE_N           ) /* N                           31  B1 */
	AT_KEYB_HELPER( 0x0800, "F1",           KEYCODE_S           ) /* F1                          3B  BB */
//  AT_KEYB_HELPER( 0x8000, "F5",           KEYCODE_F5          )

	PORT_START("pc_keyboard_4")
//  AT_KEYB_HELPER( 0x0004, "F8",           KEYCODE_F8          )

	PORT_START("pc_keyboard_5")

	PORT_START("pc_keyboard_6")
	AT_KEYB_HELPER( 0x0040, "(MF2)Cursor Up",		KEYCODE_UP          ) /* Up                          67  e7 */
	AT_KEYB_HELPER( 0x0080, "(MF2)Page Up",			KEYCODE_PGUP        ) /* Page Up                     68  e8 */
	AT_KEYB_HELPER( 0x0100, "(MF2)Cursor Left",		KEYCODE_LEFT        ) /* Left                        69  e9 */
	AT_KEYB_HELPER( 0x0200, "(MF2)Cursor Right",		KEYCODE_RIGHT       ) /* Right                       6a  ea */
	AT_KEYB_HELPER( 0x0800, "(MF2)Cursor Down",		KEYCODE_DOWN        ) /* Down                        6c  ec */
	AT_KEYB_HELPER( 0x1000, "(MF2)Page Down",		KEYCODE_PGDN        ) /* Page Down                   6d  ed */
	AT_KEYB_HELPER( 0x4000, "Del",      		    	KEYCODE_A           ) /* Delete                      6f  ef */

	PORT_START("pc_keyboard_7")

	PORT_START("IOCARD1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x0008, 0x0008, "1" )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Accelerator")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("IOCARD2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 ) // guess
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset SW")
	PORT_DIPNAME( 0x0004, 0x0004, "2" )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Turbo")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) // returns back to MS-DOS (likely to be unmapped and actually used as a lame protection check)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("IOCARD3")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xdfff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IOCARD4")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSWA" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_START("IOCARD5")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSWA" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END
#endif

static IRQ_CALLBACK(irq_callback)
{
	voyager_state *state = device->machine().driver_data<voyager_state>();
	return pic8259_acknowledge( state->m_pic8259_1);
}

static READ8_HANDLER( vga_setting ) { return 0xff; } // hard-code to color

static MACHINE_START(voyager)
{
	voyager_state *state = machine.driver_data<voyager_state>();
	device_set_irq_callback(machine.device("maincpu"), irq_callback);

	state->m_pit8254 = machine.device( "pit8254" );
	state->m_pic8259_1 = machine.device( "pic8259_1" );
	state->m_pic8259_2 = machine.device( "pic8259_2" );
	state->m_dma8237_1 = machine.device( "dma8237_1" );
	state->m_dma8237_2 = machine.device( "dma8237_2" );
}

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER(voyager_state::voyager_pic8259_1_set_int_line)
{
	machine().device("maincpu")->execute().set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

READ8_MEMBER(voyager_state::get_slave_ack)
{
	if (offset==2) {
		return pic8259_acknowledge(m_pic8259_2);
	}
	return 0x00;
}

static const struct pic8259_interface voyager_pic8259_1_config =
{
	DEVCB_DRIVER_LINE_MEMBER(voyager_state,voyager_pic8259_1_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_DRIVER_MEMBER(voyager_state,get_slave_ack)
};

static const struct pic8259_interface voyager_pic8259_2_config =
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

static const struct pit8253_config voyager_pit8254_config =
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

static MACHINE_RESET(voyager)
{
	//machine.root_device().membank("bank1")->set_base(machine.root_device().memregion("bios")->base() + 0x10000);
	machine.root_device().membank("bank1")->set_base(machine.root_device().memregion("bios")->base());
}

static void set_gate_a20(running_machine &machine, int a20)
{
	machine.device("maincpu")->execute().set_input_line(INPUT_LINE_A20, a20);
}

static void keyboard_interrupt(running_machine &machine, int state)
{
	voyager_state *drvstate = machine.driver_data<voyager_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}

static void ide_interrupt(device_t *device, int state)
{
	voyager_state *drvstate = device->machine().driver_data<voyager_state>();
	pic8259_ir6_w(drvstate->m_pic8259_2, state);
}

static int voyager_get_out2(running_machine &machine)
{
	voyager_state *state = machine.driver_data<voyager_state>();
	return pit8253_get_output(state->m_pit8254, 2 );
}

static const struct kbdc8042_interface at8042 =
{
	KBDC8042_AT386, set_gate_a20, keyboard_interrupt, NULL, voyager_get_out2
};

static void voyager_set_keyb_int(running_machine &machine, int state)
{
	voyager_state *drvstate = machine.driver_data<voyager_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}

static const ide_config ide_intf = 
{
	ide_interrupt, 
	NULL, 
	0
};

static MACHINE_CONFIG_START( voyager, voyager_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 133000000) // actually AMD Duron CPU of unknown clock
	MCFG_CPU_PROGRAM_MAP(voyager_map)
	MCFG_CPU_IO_MAP(voyager_io)

	MCFG_MACHINE_START(voyager)
	MCFG_MACHINE_RESET(voyager)

	MCFG_PIT8254_ADD( "pit8254", voyager_pit8254_config )
	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )
	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )
	MCFG_PIC8259_ADD( "pic8259_1", voyager_pic8259_1_config )
	MCFG_PIC8259_ADD( "pic8259_2", voyager_pic8259_2_config )
	MCFG_IDE_CONTROLLER_ADD("ide", ide_intf, ide_devices, "hdd", NULL, true)

	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )
	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, NULL, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(7, NULL, intel82371ab_pci_r, intel82371ab_pci_w)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(voyager_state,voyager)
{
	m_bios_ram = auto_alloc_array(machine(), UINT32, 0x20000/4);

	pc_vga_init(machine(), vga_setting, NULL);
	pc_svga_trident_io_init(machine(), machine().device("maincpu")->memory().space(AS_PROGRAM), 0xa0000, machine().device("maincpu")->memory().space(AS_IO), 0x0000);
	init_pc_common(machine(), PCCOMMON_KEYBOARD_AT, voyager_set_keyb_int);

	intel82439tx_init(machine());

	kbdc8042_init(machine(), &at8042);
}

ROM_START( voyager )
	ROM_REGION( 0x40000, "bios", 0 )
    ROM_LOAD( "stv.u23", 0x000000, 0x040000, CRC(0bed28b6) SHA1(8e7f17af65ca9d17c5c7ddedb2313507d0ea8181) )

	ROM_REGION( 0x8000, "video_bios", 0 )   // incorrect,
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )

	ROM_REGION( 0x800, "nvram", ROMREGION_ERASE00 )

	DISK_REGION( "drive_0" )
    DISK_IMAGE_READONLY( "voyager", 0, SHA1(8b94f2420f6abb40148e4ba6eed8819d8e85dbde))
ROM_END

GAME( 2002, voyager,  0, voyager, voyager, voyager_state,  voyager, ROT0, "Team Play/Game Refuge/Monaco Entertainment", "Star Trek: Voyager", GAME_NOT_WORKING|GAME_NO_SOUND )
