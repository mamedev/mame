// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Turret Tower by Dell Electronics

    driver by Phil Bennett

****************************************************************************/

#include "emu.h"
#include "cpu/mips/r3000.h"
#include "machine/ataintf.h"
#include "includes/turrett.h"



/*************************************
 *
 *  Definitions
 *
 *************************************/

#define R3041_CLOCK         XTAL_25MHz



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void turrett_state::machine_start()
{
	// Allocate memory for the two 256kx16 banks of video RAM
	m_video_ram[0] = std::make_unique<UINT16[]>(VRAM_BANK_WORDS);
	m_video_ram[1] = std::make_unique<UINT16[]>(VRAM_BANK_WORDS);

	// Register our state for saving
	save_pointer(NAME(m_video_ram[0].get()), VRAM_BANK_WORDS);
	save_pointer(NAME(m_video_ram[1].get()), VRAM_BANK_WORDS);
	save_item(NAME(m_inputs_active));
	save_item(NAME(m_last_pixel));
	save_item(NAME(m_video_ctrl));
	save_item(NAME(m_video_fade));
	save_item(NAME(m_x_pos));
	save_item(NAME(m_x_start));
	save_item(NAME(m_x_mod));
	save_item(NAME(m_dx));
	save_item(NAME(m_y_pos));
	save_item(NAME(m_scale_cnt_y));
	save_item(NAME(m_scale_cnt_x));
	save_item(NAME(m_skip_x));
	save_item(NAME(m_skip_y));
	save_item(NAME(m_scale));
	save_item(NAME(m_hotspot_x));
	save_item(NAME(m_hotspot_y));
	save_item(NAME(m_dma_idle));
	save_item(NAME(m_dma_addr));
	save_item(NAME(m_ipt_val));
	save_item(NAME(m_frame));
	save_item(NAME(m_adc));

	m_dma_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(turrett_state::dma_complete), this));
}


void turrett_state::machine_reset()
{
	m_dma_idle = true;
	m_frame = 0;
	m_adc = 0;
	m_inputs_active = 0;
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 32, turrett_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM
	AM_RANGE(0x02000010, 0x02000013) AM_RAM
	AM_RANGE(0x02000040, 0x02000043) AM_RAM
	AM_RANGE(0x02000050, 0x02000053) AM_RAM
	AM_RANGE(0x02000060, 0x02000063) AM_RAM
	AM_RANGE(0x02000070, 0x02000073) AM_RAM // TODO: What are these?
	AM_RANGE(0x04000000, 0x0400000f) AM_WRITE(dma_w)
	AM_RANGE(0x04000100, 0x04000103) AM_READWRITE(int_r, int_w)
	AM_RANGE(0x04000200, 0x040003ff) AM_DEVREADWRITE("ttsound", turrett_device, read, write)
	AM_RANGE(0x08000000, 0x0800000f) AM_READWRITE(video_r, video_w)
	AM_RANGE(0x08000200, 0x080003ff) AM_DEVREADWRITE16("ata", ata_interface_device, read_cs0, write_cs0, 0xffffffff)
	AM_RANGE(0x1fc00000, 0x1fdfffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START( turrett_sound_map, AS_0, 16, turrett_state )
	AM_RANGE(0x0000000, 0x7ffffff) AM_RAM AM_SHARE("bank_a")
	AM_RANGE(0x8000000, 0xfffffff) AM_RAM AM_SHARE("bank_b")
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( turrett )
	PORT_START("PORT 0X")
	PORT_BIT( 0x3f, 0x00, IPT_AD_STICK_Y ) PORT_MINMAX(0x20,0x1f) PORT_SENSITIVITY(60) PORT_KEYDELTA(2)

	PORT_START("PORT 4X")
	PORT_BIT( 0x3f, 0x00, IPT_AD_STICK_X ) PORT_MINMAX(0x20,0x1f) PORT_SENSITIVITY(60) PORT_KEYDELTA(2)

	PORT_START("PORT CX")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00000100)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )   PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00000200)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BILL1 )      PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00000400)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00000800)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )      PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00001000)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )      PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00002000)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START )      PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00004000)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00008000)

	PORT_START("PORT DX")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00010000)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )    PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00020000)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )    PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00040000)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00080000)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00100000)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00200000)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00400000)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, turrett_state, ipt_change, (void *)0x00800000)

	PORT_START("PORT EX")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Floor mat")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Door Lock") PORT_TOGGLE

	PORT_START("PORT FX")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Seat Belt") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Home")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Emergency Stop") PORT_TOGGLE
INPUT_PORTS_END



/*************************************
 *
 *  I/O handling
 *
 *************************************/

READ_LINE_MEMBER( turrett_state::sbrc2_r )
{
	return m_screen->vblank();
}


READ_LINE_MEMBER( turrett_state::sbrc3_r )
{
	return m_dma_idle;
}


READ32_MEMBER( turrett_state::int_r )
{
	return update_inputs() << 24;
}


WRITE32_MEMBER( turrett_state::int_w )
{
	// TODO
	logerror("Output write: %08x\n", data);
}


UINT32 turrett_state::update_inputs(void)
{
	UINT32 val = 0;

	// TODO: Prioritise?
	if (m_inputs_active)
	{
		if (m_inputs_active & 0x00000001)
		{
			val = 0x00 | (ioport("PORT 0X")->read() & 0x3f);
			m_inputs_active &= ~1;
		}
		else if (m_inputs_active & 0x00000002)
		{
			val = 0x40 | (ioport("PORT 4X")->read() & 0x3f);
			m_inputs_active &= ~2;
		}
		else if (m_inputs_active & 0x0000ff00)
		{
			UINT32 data = ioport("PORT CX")->read();
			UINT32 bits = m_inputs_active >> 8;

			val = 0xc0;

			for (int i = 0; i < 8; ++i)
			{
				if (bits & (1 << i))
				{
					val |= i << 1;
					val |= (data >> i) & 1;
					m_inputs_active &= ~(1 << (i + 8));
					break;
				}
			}
		}
		else if (m_inputs_active & 0x00ff0000)
		{
			UINT32 data = ioport("PORT DX")->read();
			UINT32 bits = m_inputs_active >> 16;

			val = 0xd0;

			for (int i = 0; i < 8; ++i)
			{
				if (bits & (1 << i))
				{
					val |= i << 1;
					val |= (data >> i) & 1;
					m_inputs_active &= ~(1 << (i + 16));
					break;
				}
			}
		}
		else if (m_inputs_active & 0x01000000)
		{
			val = 0xe0 | ioport("PORT EX")->read();
			m_inputs_active &= ~0x01000000;
		}
		else if (m_inputs_active & 0x02000000)
		{
			val = 0xf0 | ioport("PORT FX")->read();
			m_inputs_active &= ~0x02000000;
		}
	}

	// Update IRQ state
	m_maincpu->set_input_line(R3000_IRQ1, m_inputs_active ? ASSERT_LINE : CLEAR_LINE);
	return val;
}


INPUT_CHANGED_MEMBER( turrett_state::ipt_change )
{
	int p = (FPTR)param;

	if (newval != oldval)
	{
		// TODO: Tidy this up
		if (p & (0x02000000 | 0x00000200 | 0x00000400 | 0x00001000 | 0x00002000))
		{
			if (newval == 0)
			{
				m_inputs_active |= p;
				m_maincpu->set_input_line(R3000_IRQ1, ASSERT_LINE);
			}
		}
		else
		{
			m_inputs_active |= p;
			m_maincpu->set_input_line(R3000_IRQ1, ASSERT_LINE);
		}
	}
}



/*************************************
 *
 *  Interrupts
 *
 *************************************/

INTERRUPT_GEN_MEMBER( turrett_state::vblank )
{
	if (m_frame)
		m_inputs_active |= 0x01000000;
	else
		m_inputs_active |= 0x02000000;

	m_frame ^= 1;
	m_maincpu->set_input_line(R3000_IRQ1, ASSERT_LINE);
}


INTERRUPT_GEN_MEMBER( turrett_state::adc )
{
	if (m_adc)
		m_inputs_active |= 0x00000001;
	else
		m_inputs_active |= 0x00000002;

	m_adc ^= 1;
	m_maincpu->set_input_line(R3000_IRQ1, ASSERT_LINE);
}

/*************************************
 *
 *  Hard drive
 *
 *************************************/

/// HACK: The game expects a different LBA mapping to the standard HDD.
/// The reason for this is unknown.

#include "machine/idehd.h"

extern const device_type TURRETT_HARDDISK;

class turrett_hdd : public ide_hdd_device
{
public:
	turrett_hdd(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: ide_hdd_device(mconfig, TURRETT_HARDDISK, "HDD Turrett Tower", tag, owner, clock, "turrett_hdd", __FILE__)
	{
	}

	virtual UINT32 lba_address() override
	{
		if (m_device_head & IDE_DEVICE_HEAD_L)
			return (((m_device_head & IDE_DEVICE_HEAD_HS) << 24) | (m_cylinder_high << 16) | (m_cylinder_low << 8) | m_sector_number) - 63;

		return ata_mass_storage_device::lba_address();
	}
};

const device_type TURRETT_HARDDISK = &device_creator<turrett_hdd>;

SLOT_INTERFACE_START(turrett_devices)
	SLOT_INTERFACE("hdd", TURRETT_HARDDISK)
SLOT_INTERFACE_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( turrett, turrett_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R3041, R3041_CLOCK)
	MCFG_R3000_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_R3000_BRCOND2_INPUT(READLINE(turrett_state, sbrc2_r))
	MCFG_R3000_BRCOND3_INPUT(READLINE(turrett_state, sbrc3_r))
	MCFG_CPU_PROGRAM_MAP(cpu_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", turrett_state, vblank)
	MCFG_CPU_PERIODIC_INT_DRIVER(turrett_state, adc, 60)

	MCFG_ATA_INTERFACE_ADD("ata", turrett_devices, "hdd", nullptr, true)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	// TODO: Likely not correct. Refresh rate empirically determined
	// to ensure in-sync streaming sound
	MCFG_SCREEN_RAW_PARAMS(4000000, 512, 0, 336, 259, 0, 244)
	MCFG_SCREEN_UPDATE_DRIVER(turrett_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_RRRRRGGGGGBBBBB("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_DEVICE_ADD("ttsound", TURRETT, R3041_CLOCK) // ?
	MCFG_DEVICE_ADDRESS_MAP(AS_0, turrett_sound_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END



	/*************************************
	*
	*  ROM definition
	*
	*************************************/

ROM_START( turrett )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "turret.u13", 0x000000, 0x080000, CRC(85287007) SHA1(990b954905c66340d3e88918b2f8cc7f1b9c7cf4) )
	ROM_LOAD32_BYTE( "turret.u12", 0x000001, 0x080000, CRC(a2a498fc) SHA1(47f2c9c9f2496b49fd923acb400166e963095e1d) )
	ROM_LOAD32_BYTE( "turret.u8",  0x000002, 0x080000, CRC(ddff4898) SHA1(a8f859a0dcab8ec83fbfe255d58b3e644933b923) )
	ROM_LOAD32_BYTE( "turret.u7",  0x000003, 0x080000, CRC(fa8b5a5a) SHA1(658e9eeadc9c70185973470565d562c76f4fcdd7) )

	DISK_REGION( "ata:0:hdd:image" )
	/// According to http://personal.inet.fi/cool/lwgt/myoldvdr/V40ProductManual.pdf
	/// The drive should have CYLS:38869, HEADS:16, SECS:63, Total Units:39,179,952
	/// We are missing 13482 sectors and we have to adjust the LBA by 63 sectors for
	/// the game to work.
	DISK_IMAGE( "turrett", 0, BAD_DUMP SHA1(b0c98c5876870dd8b3e37a38fe35846c9e011df4) )
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 2001, turrett, 0, turrett, turrett, driver_device, 0, ROT0, "Dell Electronics (Namco license)", "Turret Tower", 0 )
