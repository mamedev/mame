// license:BSD-3-Clause
// copyright-holders:AJR

// CPU die (epoxy blob) is an Elan EU3A12 (Elan "RISC II Series" quasi-PIC with 16-bit opcodes)

#include "emu.h"
#include "cpu/rii/riscii.h"
#include "video/sed1520.h"
#include "emupal.h"
#include "screen.h"


namespace {

class vreadere_state : public driver_device
{
public:
	vreadere_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_epl(*this, "epl")
		, m_portb_control(0xff)
		, m_portc_data(0xff)
	{ }

	void vreadere(machine_config &config);

	void power_on_w(int state);
	void power_off_w(int state);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	EPL43102_UPDATE_CB(lcd_update);

	void portb_w(u8 data);
	u8 portc_r();
	void portc_w(u8 data);

	void prog_map(address_map &map) ATTR_COLD;

	void palette_init(palette_device &palette);

	required_device<riscii_series_device> m_maincpu;
	required_device<epl43102_device> m_epl;

	u8 m_portb_control;
	u8 m_portc_data;
};

void vreadere_state::machine_start()
{
	save_item(NAME(m_portb_control));
	save_item(NAME(m_portc_data));
}

EPL43102_UPDATE_CB(vreadere_state::lcd_update)
{
	if (lcd_on)
	{
		for (int y = 0; y < 32; y++)
		{
			int ys = (y + start_line) & 31;
			for (int x = 0; x < 68; x++)
			{
				int bitpos = ((ys & ~7) * 102 + x * 8 + (ys & 7)) % (42 * 102);
				bitmap.pix(y, x) = BIT(dram[bitpos / 8], bitpos & 7) ? !reverse : reverse;
			}
		}
		for (int y = 32; y < 48; y++)
		{
			int ys = (y + start_line) & 15;
			for (int x = 0; x < 34; x++)
			{
				int bitpos = (((ys & ~7) | 16) * 102 + (68 + x) * 8 + (ys & 7)) % (42 * 102);
				bitmap.pix(y, x) = BIT(dram[bitpos / 8], bitpos & 7) ? !reverse : reverse;
			}
			for (int x = 34; x < 68; x++)
			{
				int bitpos = ((ys & ~7) * 102 + (135 - x) * 8 + (ys & 7)) % (42 * 102);
				bitmap.pix(y, x) = BIT(dram[bitpos / 8], bitpos & 7) ? !reverse : reverse;
			}
		}
	}
	else
		bitmap.fill(0, cliprect);

	return 0;
}

void vreadere_state::power_on_w(int state)
{
	m_maincpu->set_input_line(riscii_series_device::PA6_LINE, state ? CLEAR_LINE : ASSERT_LINE);
}

void vreadere_state::power_off_w(int state)
{
	m_maincpu->set_input_line(riscii_series_device::PA7_LINE, state ? CLEAR_LINE : ASSERT_LINE);
}

void vreadere_state::portb_w(u8 data)
{
	u8 old_control = std::exchange(m_portb_control, data);

	if (!BIT(data, 3))
	{
		if (!BIT(old_control, 6) && BIT(data, 6))
			m_epl->write(BIT(data, 4), m_portc_data);

		if (BIT(old_control, 5) && !BIT(data, 5))
			m_portc_data = m_epl->read(BIT(data, 4));
	}
}

u8 vreadere_state::portc_r()
{
	return m_portc_data;
}

void vreadere_state::portc_w(u8 data)
{
	m_portc_data = data;
}

void vreadere_state::prog_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START(vreadere)
	PORT_START("POWER")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_POWER_ON) PORT_WRITE_LINE_MEMBER(vreadere_state, power_on_w)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_POWER_OFF) PORT_WRITE_LINE_MEMBER(vreadere_state, power_off_w)
INPUT_PORTS_END

void vreadere_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t(  0,   0,   0));
}

void vreadere_state::vreadere(machine_config &config)
{
	EPG3231(config, m_maincpu, 8'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &vreadere_state::prog_map);
	m_maincpu->out_portb_cb().set(FUNC(vreadere_state::portb_w));
	m_maincpu->in_portc_cb().set(FUNC(vreadere_state::portc_r));
	m_maincpu->out_portc_cb().set(FUNC(vreadere_state::portc_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(68, 48);
	screen.set_visarea_full();
	screen.set_screen_update(m_epl, FUNC(epl43102_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(vreadere_state::palette_init), 2);

	EPL43102(config, m_epl);
	m_epl->set_screen_update_cb(FUNC(vreadere_state::lcd_update));
}

ROM_START( vreadere )
	ROM_REGION(0x400000, "maincpu", 0)
	ROM_LOAD( "27-08291.u2", 0x000000, 0x400000, CRC(f2eb801f) SHA1(33e2d28ab2f04b17f66880898832265d50de54d4) )
ROM_END

} // anonymous namespace


COMP( 2004, vreadere, 0, 0, vreadere, vreadere, vreadere_state, empty_init, "Video Technology", "Reader Laptop E (Germany)", MACHINE_IS_SKELETON )
