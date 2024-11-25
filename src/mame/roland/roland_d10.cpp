// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Jonathan Gevaryahu
/*************************************************************************************************

    Roland D-10/D-110 driver

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
#include "machine/bankdev.h"
#include "mb63h149.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "video/msm6222b.h"
#include "emupal.h"
#include "screen.h"


namespace {

static INPUT_PORTS_START( d10 )
INPUT_PORTS_END

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


class roland_d10_state : public driver_device
{
public:
	roland_d10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bank(*this, "bank")
		, m_rams(*this, "rams")
		, m_memcs(*this, "memcs")
		, m_lcd(*this, "lcd")
		, m_midi_timer(*this, "midi_timer")
		, m_maincpu(*this, "maincpu")
	{ }

	void d10(machine_config &config);
	void d110(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void bank_w(uint8_t data);
	uint8_t fixed_r(offs_t offset);
	void fixed_w(offs_t offset, uint8_t data);
	void so_w(uint8_t data);
	void midi_w(uint8_t data);
	uint8_t lcd_ctrl_r();
	void lcd_ctrl_w(uint8_t data);
	void lcd_data_w(uint8_t data);
	uint8_t port0_r();
	TIMER_DEVICE_CALLBACK_MEMBER(midi_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(samples_timer_cb);
	void d10_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void d10_map(address_map &map) ATTR_COLD;
	void d10_bank_map(address_map &map) ATTR_COLD;
	void d110_map(address_map &map) ATTR_COLD;
	void d110_bank_map(address_map &map) ATTR_COLD;

	uint8_t  m_lcd_data_buffer[256]{};
	int      m_lcd_data_buffer_pos = 0;
	uint8_t  m_midi = 0;
	int      m_midi_pos = 0;
	uint8_t  m_port0 = 0;
	required_device<address_map_bank_device> m_bank;
	required_device<nvram_device> m_rams;
	required_device<nvram_device> m_memcs;
	required_device<msm6222b_device> m_lcd;
	required_device<timer_device> m_midi_timer;
	required_device<i8x9x_device> m_maincpu;
};


uint32_t roland_d10_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0;
	uint8_t const *const data = m_lcd->render();
	bitmap.fill(0);

	for (uint8_t y = 0; y < 2; y++)
	{
		for (uint8_t ra = 0; ra < 9; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = 0; x < 16; x++)
			{
				uint8_t gfx = 0;
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

void roland_d10_state::machine_start()
{
	m_lcd_data_buffer_pos = 0;
}

void roland_d10_state::machine_reset()
{
	//  midi_timer->adjust(attotime::from_hz(1));
	m_midi_pos = 0;
	m_port0 = 0x80; // battery ok
}

void roland_d10_state::lcd_ctrl_w(uint8_t data)
{
	m_lcd->control_w(data);
	for(int i=0; i != m_lcd_data_buffer_pos; i++)
		m_lcd->data_w(m_lcd_data_buffer[i]);
	m_lcd_data_buffer_pos = 0;
}

uint8_t roland_d10_state::lcd_ctrl_r()
{
	// Busy flag in the msm622b is bit 7, while the software expects it in bit 0...
	return m_lcd->control_r() >> 7;
}

void roland_d10_state::lcd_data_w(uint8_t data)
{
	if(m_lcd_data_buffer_pos == sizeof(m_lcd_data_buffer)) {
		logerror("Warning: lcd data buffer overflow (%04x)\n", m_maincpu->pc());
		return;
	}
	m_lcd_data_buffer[m_lcd_data_buffer_pos++] = data;
}

void roland_d10_state::bank_w(uint8_t data)
{
	m_bank->set_bank(data);
}

uint8_t roland_d10_state::fixed_r(offs_t offset)
{
	return m_bank->space(0).read_byte(0x40000 + offset);
}

void roland_d10_state::fixed_w(offs_t offset, uint8_t data)
{
	m_bank->space(0).write_byte(0x40000 + offset, data);
}

void roland_d10_state::midi_w(uint8_t data)
{
	logerror("midi_out %02x\n", data);
	m_midi = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(roland_d10_state::midi_timer_cb)
{
	const static uint8_t midi_data[3] = { 0x91, 0x40, 0x7f };
	m_midi = midi_data[m_midi_pos++];
	logerror("midi_in %02x\n", m_midi);
	m_maincpu->serial_w(m_midi);
	if(m_midi_pos < sizeof(midi_data))
		m_midi_timer->adjust(attotime::from_hz(1250));
}

uint8_t roland_d10_state::port0_r()
{
	return m_port0;
}

TIMER_DEVICE_CALLBACK_MEMBER(roland_d10_state::samples_timer_cb)
{
	m_port0 ^= 0x10;
}

void roland_d10_state::so_w(uint8_t data)
{
	// bit 0   = led
	// bit 1-2 = reverb program a13/a14
	// bit 3   = R. SW. to ananlog board
	// bit 5   = boss 8Mhz clock, handled internally
	//  logerror("so: rw=%d bank=%d led=%d\n", (data >> 3) & 1, (data >> 1) & 3, data & 1);
}

void roland_d10_state::d10_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0, 255, 0));
	palette.set_pen_color(1, rgb_t(0, 0, 0));
}

void roland_d10_state::d10_map(address_map &map)
{
	map(0x0100, 0x0100).w(FUNC(roland_d10_state::bank_w));
	map(0x0200, 0x0200).w(FUNC(roland_d10_state::so_w));
	map(0x0300, 0x0300).w(FUNC(roland_d10_state::lcd_data_w));
	map(0x0380, 0x0380).rw(FUNC(roland_d10_state::lcd_ctrl_r), FUNC(roland_d10_state::lcd_ctrl_w));
	map(0x0c00, 0x0dff).rw("keyscan", FUNC(mb63h149_device::read), FUNC(mb63h149_device::write));
	map(0x1000, 0x7fff).rom().region("firmware", 0x1000);
	map(0x8000, 0xbfff).m(m_bank, FUNC(address_map_bank_device::amap16));
	map(0xc000, 0xffff).rw(FUNC(roland_d10_state::fixed_r), FUNC(roland_d10_state::fixed_w));
}

void roland_d10_state::d10_bank_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("firmware", 0);
	map(0x40000, 0x47fff).ram();
	map(0x80000, 0x87fff).ram().share("rams");
	map(0xa0000, 0xbffff).rom().region("presets", 0);
	map(0xc0000, 0xc7fff).ram().share("memcs");
}

void roland_d10_state::d110_map(address_map &map)
{
	map(0x0100, 0x0100).w(FUNC(roland_d10_state::bank_w));
	map(0x0200, 0x0200).w(FUNC(roland_d10_state::so_w));
	map(0x021a, 0x021b).portr("SC0").nopw();
	map(0x021c, 0x021d).portr("SC1");
	map(0x0300, 0x0300).w(FUNC(roland_d10_state::lcd_data_w));
	map(0x0380, 0x0380).rw(FUNC(roland_d10_state::lcd_ctrl_r), FUNC(roland_d10_state::lcd_ctrl_w));
	//map(0x1000, 0x7fff).rom().region("firmware", 0x1000);
	map(0x1000, 0x7fff).lr8([this](offs_t offset) { return m_bank->space(0).read_byte(offset + 0x01000); }, "firmware_r");
	map(0x8000, 0xbfff).m(m_bank, FUNC(address_map_bank_device::amap8));
	map(0xc000, 0xffff).rw(FUNC(roland_d10_state::fixed_r), FUNC(roland_d10_state::fixed_w));
}

void roland_d10_state::d110_bank_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("firmware", 0);
	map(0x40000, 0x47fff).ram().share("rams");
	map(0x80000, 0x9ffff).rom().region("presets", 0);
	map(0xc0000, 0xc7fff).ram().share("memcs");
}

void roland_d10_state::d10(machine_config &config)
{
	N8097BH(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_d10_state::d10_map);
	m_maincpu->serial_tx_cb().set(FUNC(roland_d10_state::midi_w));
	m_maincpu->in_p0_cb().set(FUNC(roland_d10_state::port0_r));

	ADDRESS_MAP_BANK(config, m_bank);
	m_bank->set_endianness(ENDIANNESS_LITTLE);
	m_bank->set_data_width(16);
	m_bank->set_addr_width(20);
	m_bank->set_stride(0x4000);
	m_bank->set_addrmap(0, &roland_d10_state::d10_bank_map);

// Battery-backed main ram
	NVRAM(config, m_rams, nvram_device::DEFAULT_ALL_0);

// Shall become a proper memcard device someday
	NVRAM( config, m_memcs, nvram_device::DEFAULT_ALL_0 );

	mb63h149_device &keyscan(MB63H149(config, "keyscan", 16.384_MHz_XTAL));
	keyscan.int_callback().set_inputline(m_maincpu, i8x9x_device::HSI0_LINE);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update(FUNC(roland_d10_state::screen_update));
//  screen.set_size(20*6-1, 2*9-1);
	screen.set_size(16*6-1, (16*6-1)*3/4);
	screen.set_visarea(0, 16*6-2, 0, (16*6-1)*3/4-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(roland_d10_state::d10_palette), 2);

	MSM6222B_01(config, m_lcd, 0);

	TIMER(config, m_midi_timer).configure_generic(FUNC(roland_d10_state::midi_timer_cb));

	TIMER(config,  "samples_timer").configure_periodic(FUNC(roland_d10_state::samples_timer_cb), attotime::from_hz(32000*2) );
}

void roland_d10_state::d110(machine_config &config)
{
	d10(config);

	// D-110 ties BUSWIDTH to GND to make all external accesses 8 bits wide
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_d10_state::d110_map);
	m_bank->set_data_width(8);
	m_bank->set_addrmap(0, &roland_d10_state::d110_bank_map);

	config.device_remove("keyscan");
}

ROM_START( d10 )
	ROM_REGION16_LE( 0x10000, "firmware", 0 )
	ROM_DEFAULT_BIOS( "106" )

	ROM_SYSTEM_BIOS( 0, "102", "Firmware 1.02" )
	ROMX_LOAD( "d-10_v1.02_a.ic14",            1,   0x8000, CRC(71162a61) SHA1(a39f6ea90682346bc259615198eac18c1f0ad0bd), ROM_BIOS(0) | ROM_SKIP(1) )
	ROMX_LOAD( "d-10_v1.02_b.ic13",            0,   0x8000, CRC(99def0ed) SHA1(ff6d3f88a3cdfa901bd0e9a83350ea2842fd16bc), ROM_BIOS(0) | ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 1, "106", "Firmware 1.06" )
	ROMX_LOAD( "d-10a_1.06.ic14",              1,   0x8000, CRC(ea90848a) SHA1(1689b47210e4033b922816a1817c430358d96641), ROM_BIOS(1) | ROM_SKIP(1) ) // AM27C256-200DC
	ROMX_LOAD( "d-10b_1.06.ic13",              0,   0x8000, CRC(26dbbf48) SHA1(f6063c2e3000e08a0dc3e45c8ee9400916f5493c), ROM_BIOS(1) | ROM_SKIP(1) ) // AM27C256-155DC

	ROM_REGION16_LE( 0x20000, "presets", 0 )
	ROM_LOAD(  "r15179873-lh5310-97.ic12",     0,  0x20000, CRC(580a8f9e) SHA1(05587a0542b01625dcde37de5bb339880e47eb93) )

	ROM_REGION( 0x100000, "la32", 0 )
	ROM_LOAD(  "r15179878-hn62304bpc99.ic27",  0,       0x80000, CRC(e117e6ab) SHA1(6760d14900161b8715c2bfd4ebe997877087c90c) )
	ROM_LOAD(  "r15179880-hn62304bpd10.ic28",  0x80000, 0x80000, CRC(b329f945) SHA1(9c59f50518a070461b2ec6cb4e43ee7cc1e905b6) )

	ROM_REGION( 0x8000, "boss", 0 )
	ROM_LOAD(  "r15179879-hn623257pz20.ic33",  0,   0x8000, CRC(5d34174e) SHA1(17bd2887711c5c5458aba6d3be5972b2096eb450) )
ROM_END

ROM_START( d110 )
	ROM_REGION( 0x10000, "firmware", 0 )
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

} // anonymous namespace


SYST( 1988, d10,  0,   0, d10,  d10,  roland_d10_state, empty_init, "Roland", "D-10 Multi Timbral Linear Synthesizer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
SYST( 1988, d110, d10, 0, d110, d110, roland_d10_state, empty_init, "Roland", "D-110 Multi Timbral Sound Module",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
