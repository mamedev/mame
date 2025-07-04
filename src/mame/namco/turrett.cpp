// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Turret Tower by Dell Electronics

    driver by Phil Bennett

****************************************************************************/

#include "emu.h"
#include "turrett.h"

#include "bus/ata/hdd.h"


/*************************************
 *
 *  Definitions
 *
 *************************************/

#define R3041_CLOCK         XTAL(25'000'000)



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void turrett_state::machine_start()
{
	// Allocate memory for the two 256kx16 banks of video RAM
	m_video_ram[0] = std::make_unique<uint16_t[]>(VRAM_BANK_WORDS);
	m_video_ram[1] = std::make_unique<uint16_t[]>(VRAM_BANK_WORDS);

	// Register our state for saving
	save_pointer(NAME(m_video_ram[0]), VRAM_BANK_WORDS);
	save_pointer(NAME(m_video_ram[1]), VRAM_BANK_WORDS);
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

	m_dma_timer = timer_alloc(FUNC(turrett_state::dma_complete), this);
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

void turrett_state::cpu_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).ram().mirror(0x40000000);
	map(0x02000010, 0x02000013).ram();
	map(0x02000040, 0x02000043).ram();
	map(0x02000050, 0x02000053).ram();
	map(0x02000060, 0x02000063).ram();
	map(0x02000070, 0x02000073).ram(); // TODO: What are these?
	map(0x04000000, 0x0400000f).w(FUNC(turrett_state::dma_w));
	map(0x04000100, 0x04000103).rw(FUNC(turrett_state::int_r), FUNC(turrett_state::int_w));
	map(0x04000200, 0x040003ff).rw("ttsound", FUNC(turrett_device::read), FUNC(turrett_device::write));
	map(0x08000000, 0x0800000f).rw(FUNC(turrett_state::video_r), FUNC(turrett_state::video_w));
	map(0x08000200, 0x080003ff).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));
	map(0x1fc00000, 0x1fdfffff).rom().region("maincpu", 0);
}


void turrett_state::turrett_sound_map(address_map &map)
{
	map(0x0000000, 0x7ffffff).ram().share("bank_a");
	map(0x8000000, 0xfffffff).ram().share("bank_b");
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( turrett )
	PORT_START("PORT.0X")
	PORT_BIT( 0x3f, 0x00, IPT_AD_STICK_Y ) PORT_MINMAX(0x20,0x1f) PORT_SENSITIVITY(60) PORT_KEYDELTA(2)

	PORT_START("PORT.4X")
	PORT_BIT( 0x3f, 0x00, IPT_AD_STICK_X ) PORT_MINMAX(0x20,0x1f) PORT_SENSITIVITY(60) PORT_KEYDELTA(2)

	PORT_START("PORT.CX")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00000100)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00000200)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BILL1 )      PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00000400)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00000800)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )      PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00001000)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )      PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00002000)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )     PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00004000)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00008000)

	PORT_START("PORT.DX")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00010000)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00020000)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00040000)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00080000)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00100000)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00200000)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00400000)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(turrett_state::ipt_change), 0x00800000)

	PORT_START("PORT.EX")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Floor mat")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Door Lock") PORT_TOGGLE

	PORT_START("PORT.FX")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Seat Belt") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Home")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Emergency Stop") PORT_TOGGLE
INPUT_PORTS_END



/*************************************
 *
 *  I/O handling
 *
 *************************************/

int turrett_state::sbrc2_r()
{
	return m_screen->vblank();
}


int turrett_state::sbrc3_r()
{
	return m_dma_idle;
}


uint32_t turrett_state::int_r()
{
	return update_inputs() << 24;
}


void turrett_state::int_w(uint32_t data)
{
	// TODO
	logerror("Output write: %08x\n", data);
}


uint32_t turrett_state::update_inputs(void)
{
	uint32_t val = 0;

	// TODO: Prioritise?
	if (m_inputs_active)
	{
		if (m_inputs_active & 0x00000001)
		{
			val = 0x00 | (ioport("PORT.0X")->read() & 0x3f);
			m_inputs_active &= ~1;
		}
		else if (m_inputs_active & 0x00000002)
		{
			val = 0x40 | (ioport("PORT.4X")->read() & 0x3f);
			m_inputs_active &= ~2;
		}
		else if (m_inputs_active & 0x0000ff00)
		{
			uint32_t data = ioport("PORT.CX")->read();
			uint32_t bits = m_inputs_active >> 8;

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
			uint32_t data = ioport("PORT.DX")->read();
			uint32_t bits = m_inputs_active >> 16;

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
			val = 0xe0 | ioport("PORT.EX")->read();
			m_inputs_active &= ~0x01000000;
		}
		else if (m_inputs_active & 0x02000000)
		{
			val = 0xf0 | ioport("PORT.FX")->read();
			m_inputs_active &= ~0x02000000;
		}
	}

	// Update IRQ state
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, m_inputs_active ? ASSERT_LINE : CLEAR_LINE);
	return val;
}


INPUT_CHANGED_MEMBER( turrett_state::ipt_change )
{
	int p = param;

	if (newval != oldval)
	{
		// TODO: Tidy this up
		if (p & (0x02000000 | 0x00000200 | 0x00000400 | 0x00001000 | 0x00002000))
		{
			if (newval == 0)
			{
				m_inputs_active |= p;
				m_maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
			}
		}
		else
		{
			m_inputs_active |= p;
			m_maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
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
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
}


INTERRUPT_GEN_MEMBER( turrett_state::adc )
{
	if (m_adc)
		m_inputs_active |= 0x00000001;
	else
		m_inputs_active |= 0x00000002;

	m_adc ^= 1;
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
}

/*************************************
 *
 *  Hard drive
 *
 *************************************/

/// HACK: The game expects a different LBA mapping to the standard HDD.
/// The reason for this is unknown.

DECLARE_DEVICE_TYPE(TURRETT_HARDDISK, turrett_hdd)

class turrett_hdd : public ide_hdd_device
{
public:
	turrett_hdd(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: ide_hdd_device(mconfig, TURRETT_HARDDISK, tag, owner, clock)
	{
	}

	virtual uint32_t lba_address() override
	{
		if (m_device_head & IDE_DEVICE_HEAD_L)
			return (((m_device_head & IDE_DEVICE_HEAD_HS) << 24) | (m_cylinder_high << 16) | (m_cylinder_low << 8) | m_sector_number) - 63;

		return ide_hdd_device::lba_address();
	}
};

DEFINE_DEVICE_TYPE(TURRETT_HARDDISK, turrett_hdd, "turrett_hdd", "Turret Tower HDD")

void turrett_devices(device_slot_interface &device)
{
	device.option_add("hdd", TURRETT_HARDDISK);
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void turrett_state::turrett(machine_config &config)
{
	/* basic machine hardware */
	R3041(config, m_maincpu, R3041_CLOCK);
	m_maincpu->set_endianness(ENDIANNESS_BIG);
	m_maincpu->in_brcond<2>().set(FUNC(turrett_state::sbrc2_r));
	m_maincpu->in_brcond<3>().set(FUNC(turrett_state::sbrc3_r));
	m_maincpu->set_addrmap(AS_PROGRAM, &turrett_state::cpu_map);
	m_maincpu->set_vblank_int("screen", FUNC(turrett_state::vblank));
	m_maincpu->set_periodic_int(FUNC(turrett_state::adc), attotime::from_hz(60));

	ATA_INTERFACE(config, m_ata).options(turrett_devices, "hdd", nullptr, true);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: Likely not correct. Refresh rate empirically determined
	// to ensure in-sync streaming sound
	m_screen->set_raw(4000000, 512, 0, 336, 259, 0, 244);
	m_screen->set_screen_update(FUNC(turrett_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_555);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	turrett_device &ttsound(TURRETT(config, "ttsound", R3041_CLOCK)); // ?
	ttsound.set_addrmap(0, &turrett_state::turrett_sound_map);
	ttsound.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	ttsound.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}



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

	DISK_REGION( "ata:0:hdd" )
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

GAME( 2001, turrett, 0, turrett, turrett, turrett_state, empty_init, ROT0, "Dell Electronics (Namco license)", "Turret Tower", 0 )
