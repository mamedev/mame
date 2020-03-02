// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Jonathan Gevaryahu
/*************************************************************************************************

    Roland D-110 driver

    Driver by Olivier Galibert and Jonathan Gevaryahu

    The Roland D-110 is an expander (synthesizer without the keyboard)
    from 1988.  Internally it's very similar to a mt32, with a better
    LCD screen (16x2) and more control buttons.  More importantly, it
    has more sound rom, a battery-backed ram and a port for memory
    cards allowing to load and save new sounds.

    After the first boot, the ram needs to be reinitialized to factory
    default values.  Press Write/Copy (I) while resetting then
    validate with Enter (K).
*/

#include "emu.h"
#include "cpu/mcs96/i8x9x.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "video/msm6222b.h"
#include "emupal.h"
#include "screen.h"

static INPUT_PORTS_START( d110 )
	PORT_START("SC0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Write/Copy") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Number +") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank +") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Group +") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part +") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Timbre") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Patch") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit") PORT_CODE(KEYCODE_Q)

	PORT_START("SC1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Number -") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank -") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Group -") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part -") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("System") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit") PORT_CODE(KEYCODE_A)
INPUT_PORTS_END


class d110_state : public driver_device
{
public:
	d110_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ram(*this, "ram")
		, m_rams(*this, "rams")
		, m_memc(*this, "memc")
		, m_memcs(*this, "memcs")
		, m_lcd(*this, "lcd")
		, m_midi_timer(*this, "midi_timer")
		, m_maincpu(*this, "maincpu")
	{ }

	void d110(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(so_w);
	DECLARE_WRITE16_MEMBER(midi_w);
	DECLARE_READ8_MEMBER(lcd_ctrl_r);
	DECLARE_WRITE8_MEMBER(lcd_ctrl_w);
	DECLARE_WRITE8_MEMBER(lcd_data_w);
	DECLARE_READ16_MEMBER(port0_r);
	TIMER_DEVICE_CALLBACK_MEMBER(midi_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(samples_timer_cb);
	void d110_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void d110_map(address_map &map);

	uint8_t  m_lcd_data_buffer[256];
	int      m_lcd_data_buffer_pos;
	uint8_t  m_midi;
	int      m_midi_pos;
	uint8_t  m_port0;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_rams;
	required_device<ram_device> m_memc;
	required_device<nvram_device> m_memcs;
	required_device<msm6222b_device> m_lcd;
	required_device<timer_device> m_midi_timer;
	required_device<i8x9x_device> m_maincpu;
};


uint32_t d110_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,gfx;
	uint16_t sy=0,x;
	const uint8_t *data = m_lcd->render();
	bitmap.fill(0);

	for (y = 0; y < 2; y++)
	{
		for (ra = 0; ra < 9; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = 0; x < 16; x++)
			{
				gfx = 0;
				if (ra < 8)
					gfx = data[x*16 + y*640 + ra];

				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
				*p++ = 0;
			}
		}
	}
	return 0;
}

void d110_state::machine_start()
{
	m_rams->set_base(m_ram->pointer(), 32768);
	m_memcs->set_base(m_memc->pointer(), 32768);

	membank("bank")->configure_entries(0x00, 4, memregion("maincpu")->base(), 0x4000);
	membank("bank")->configure_entries(0x10, 2, m_ram->pointer(), 0x4000);
	membank("bank")->configure_entries(0x20, 8, memregion("presets")->base(), 0x4000);
	membank("bank")->configure_entries(0x30, 2, m_memc->pointer(), 0x4000);
	membank("fixed")->set_base(m_ram->pointer());

	m_lcd_data_buffer_pos = 0;
}

void d110_state::machine_reset()
{
	//  midi_timer->adjust(attotime::from_hz(1));
	m_midi_pos = 0;
	m_port0 = 0x80; // battery ok
}

WRITE8_MEMBER(d110_state::lcd_ctrl_w)
{
	m_lcd->control_w(data);
	for(int i=0; i != m_lcd_data_buffer_pos; i++)
		m_lcd->data_w(m_lcd_data_buffer[i]);
	m_lcd_data_buffer_pos = 0;
}

READ8_MEMBER(d110_state::lcd_ctrl_r)
{
	// Busy flag in the msm622b is bit 7, while the software expects it in bit 0...
	return m_lcd->control_r() >> 7;
}

WRITE8_MEMBER(d110_state::lcd_data_w)
{
	if(m_lcd_data_buffer_pos == sizeof(m_lcd_data_buffer)) {
		logerror("Warning: lcd data buffer overflow (%04x)\n", m_maincpu->pc());
		return;
	}
	m_lcd_data_buffer[m_lcd_data_buffer_pos++] = data;
}

WRITE8_MEMBER(d110_state::bank_w)
{
	membank("bank")->set_entry(data);
}

WRITE16_MEMBER(d110_state::midi_w)
{
	logerror("midi_out %02x\n", data);
	m_midi = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(d110_state::midi_timer_cb)
{
	const static uint8_t midi_data[3] = { 0x91, 0x40, 0x7f };
	m_midi = midi_data[m_midi_pos++];
	logerror("midi_in %02x\n", m_midi);
	m_maincpu->serial_w(m_midi);
	if(m_midi_pos < sizeof(midi_data))
		m_midi_timer->adjust(attotime::from_hz(1250));
}

READ16_MEMBER(d110_state::port0_r)
{
	return m_port0;
}

TIMER_DEVICE_CALLBACK_MEMBER(d110_state::samples_timer_cb)
{
	m_port0 ^= 0x10;
}

WRITE8_MEMBER(d110_state::so_w)
{
	// bit 0   = led
	// bit 1-2 = reverb program a13/a14
	// bit 3   = R. SW. to ananlog board
	// bit 5   = boss 8Mhz clock, handled internally
	//  logerror("so: rw=%d bank=%d led=%d\n", (data >> 3) & 1, (data >> 1) & 3, data & 1);
}

void d110_state::d110_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0, 255, 0));
	palette.set_pen_color(1, rgb_t(0, 0, 0));
}

void d110_state::d110_map(address_map &map)
{
	map(0x0100, 0x0100).w(FUNC(d110_state::bank_w));
	map(0x0200, 0x0200).w(FUNC(d110_state::so_w));
	map(0x021a, 0x021a).portr("SC0").nopw();
	map(0x021c, 0x021c).portr("SC1");
	map(0x0300, 0x0300).w(FUNC(d110_state::lcd_data_w));
	map(0x0380, 0x0380).rw(FUNC(d110_state::lcd_ctrl_r), FUNC(d110_state::lcd_ctrl_w));
	map(0x1000, 0x7fff).rom().region("maincpu", 0x1000);
	map(0x8000, 0xbfff).bankrw("bank");
	map(0xc000, 0xffff).bankrw("fixed");
}

void d110_state::d110(machine_config &config)
{
	P8098(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &d110_state::d110_map);
	m_maincpu->serial_tx_cb().set(FUNC(d110_state::midi_w));
	m_maincpu->in_p0_cb().set(FUNC(d110_state::port0_r));

// Battery-backed main ram
	RAM( config, "ram" ).set_default_size( "32K" );
	NVRAM(config, m_rams, nvram_device::DEFAULT_ALL_0);

// Shall become a proper memcard device someday
	RAM( config, m_memc ).set_default_size( "32K" );
	NVRAM( config, m_memcs, nvram_device::DEFAULT_ALL_0 );

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update(FUNC(d110_state::screen_update));
//  screen.set_size(20*6-1, 2*9-1);
	screen.set_size(16*6-1, (16*6-1)*3/4);
	screen.set_visarea(0, 16*6-2, 0, (16*6-1)*3/4-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(d110_state::d110_palette), 2);

	MSM6222B_01(config, m_lcd, 0);

	TIMER(config, m_midi_timer).configure_generic(FUNC(d110_state::midi_timer_cb));

	TIMER(config,  "samples_timer").configure_periodic(FUNC(d110_state::samples_timer_cb), attotime::from_hz(32000*2) );
}

ROM_START( d110 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_DEFAULT_BIOS( "110" )

	ROM_SYSTEM_BIOS( 0, "106", "Firmware 1.06" )
	ROMX_LOAD( "d-110.v1.06.ic19.bin",         0,   0x8000, CRC(3dd5b6e9) SHA1(73b155fb0a8adc2362e73cb0803dafba9ccfb508), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "110", "Firmware 1.10" )
	ROMX_LOAD( "d-110.v1.10.ic19.bin",         0,   0x8000, CRC(3ae68187) SHA1(28635510f30d6c1fb88e00da03e5b4e045c380cb), ROM_BIOS(1) )

	ROM_REGION( 0x20000, "presets", 0 )
	ROM_LOAD(  "r15179873-lh5310-97.ic12.bin", 0,  0x20000, CRC(580a8f9e) SHA1(05587a0542b01625dcde37de5bb339880e47eb93) )

	ROM_REGION( 0x100000, "la32", 0 )
	ROM_LOAD(  "r15179878.ic7.bin",            0,  0x80000, CRC(e117e6ab) SHA1(6760d14900161b8715c2bfd4ebe997877087c90c) )
	ROM_LOAD(  "r15179880.ic8.bin",      0x80000,  0x80000, CRC(b329f945) SHA1(9c59f50518a070461b2ec6cb4e43ee7cc1e905b6) )

	ROM_REGION( 0x8000, "boss", 0 )
	ROM_LOAD(  "r15179879.ic6.bin",            0,   0x8000, CRC(5d34174e) SHA1(17bd2887711c5c5458aba6d3be5972b2096eb450) )
ROM_END

CONS( 1988, d110, 0, 0, d110, d110, d110_state, empty_init, "Roland", "D110", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
