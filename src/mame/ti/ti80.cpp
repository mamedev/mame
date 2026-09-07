// license:BSD-3-Clause
// copyright-holders:grubbyplaya
/*****************************************************************************

    TI-80 graphing calculator

****************************************************************************/

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "bus/ti8x/ti8x.h"
#include "cpu/t6m53/t6m53.h"
#include "video/t6b79.h"

class ti80_state : public driver_device
{
public:
	ti80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag), 
          m_maincpu(*this, "maincpu"),
          m_link_port(*this, "linkport"),
          m_btn_cols(*this, "COL%u", 0U), 
          m_on_button(*this, "ON")
	{
	}

	void ti80(machine_config &config);
    
private:
	required_device<t6m53_device> m_maincpu;
    required_device<ti8x_link_port_device> m_link_port;
	required_ioport_array<7> m_btn_cols;
	required_ioport m_on_button;
    
    void ti80_mem(address_map &map);
    uint8_t ti80_btns_r(offs_t cols);
    uint8_t ti80_on_r();
    void ti80_palette(palette_device &palette) const;
};

void ti80_state::ti80_mem(address_map &map)
{
    map(0x0000, 0x3FFD).rom();
    map(0x3FFE, 0x3FFE).rw("t6b79", FUNC(t6b79_device::control_read), FUNC(t6b79_device::control_write));
    map(0x3FFF, 0x3FFF).rw("t6b79", FUNC(t6b79_device::data_read), FUNC(t6b79_device::data_write));
    map(0x4000, 0x4000).mirror(0x0FFE).rw("t6b79", FUNC(t6b79_device::control_read), FUNC(t6b79_device::control_write));
    map(0x4001, 0x4001).mirror(0x0FFE).rw("t6b79", FUNC(t6b79_device::data_read), FUNC(t6b79_device::data_write));
    map(0x5000, 0x6FFF).ram();
    map(0x8000, 0xFFFF).rom();
}

static INPUT_PORTS_START (ti80)
	PORT_START("COL0")   /* col 0 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_START("COL1")   /* col 1 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_PGDN)
	PORT_START("COL2")   /* col 2 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(-)") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(")") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAN") PORT_CODE(KEYCODE_PGUP)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("VARS") PORT_CODE(KEYCODE_F9)
	PORT_START("COL3")   /* col 3 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("COS") PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PRGM") PORT_CODE(KEYCODE_F8)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("STAT") PORT_CODE(KEYCODE_TILDE)
	PORT_START("COL4")   /* col 4 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_END)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SIN") PORT_CODE(KEYCODE_INSERT)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FRAC") PORT_CODE(KEYCODE_F7)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X,T") PORT_CODE(KEYCODE_X)
	PORT_START("COL5")   /* col 5 */
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("STORE") PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LN") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOG") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("x^2") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("x^-1") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MATH") PORT_CODE(KEYCODE_F6)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ALPHA") PORT_CODE(KEYCODE_LSHIFT)
	PORT_START("COL6")   /* col 6 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_F5)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRACE") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ZOOM") PORT_CODE(KEYCODE_F3)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("WINDOW") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y=") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2nd") PORT_CODE(KEYCODE_LALT)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_START("ON")   /* ON */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ON/OFF") PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

uint8_t ti80_state::ti80_btns_r(offs_t cols)
{
    uint8_t data = 0;
    for (int col = 0; col < 7; col++)
        if (cols & (1 << col))
            for (int row = 0; row < 8; row++)
                data |= BIT(m_btn_cols[col]->read(), row) ? (1 << row) : 0;

    return data;
}

uint8_t ti80_state::ti80_on_r()
{
    return m_on_button->read();
}

void ti80_state::ti80_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(160, 190, 170));
	palette.set_pen_color(1, rgb_t(83, 111, 138));
}

void ti80_state::ti80(machine_config &config)
{
    T6M53(config, m_maincpu, 980'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ti80_state::ti80_mem);
    m_maincpu->btn_rows().set(FUNC(ti80_state::ti80_btns_r));
    m_maincpu->on_btn().set(FUNC(ti80_state::ti80_on_r));
  
	screen_device &screen(SCREEN(config, "screen").set_lcd());
    screen.set_refresh_hz(60);
	screen.set_size(64, 48);
	screen.set_visarea(0, 64-1, 0, 48-1);

    T6B79(config, "t6b79");
	screen.set_screen_update("t6b79", FUNC(t6b79_device::screen_update));
    m_maincpu->lcd_stb().set("t6b79", FUNC(t6b79_device::stb_write));
    
	PALETTE(config, "palette", FUNC(ti80_state::ti80_palette), 2, 2);
	screen.set_palette("palette"); 

    // The link port is only present on viewscreen TI-80s, 
    // which have the exact same ROMs as a base TI-80.
	TI8X_LINK_PORT(config, m_link_port, default_ti8x_link_devices, nullptr);
    m_maincpu->ring_out().set(m_link_port, FUNC(ti8x_link_port_device::ring_w));
    m_maincpu->tip_out().set(m_link_port, FUNC(ti8x_link_port_device::tip_w));
    m_maincpu->ring_in().set(m_link_port, FUNC(ti8x_link_port_device::ring_r));
    m_maincpu->tip_in().set(m_link_port, FUNC(ti8x_link_port_device::tip_r));
}

ROM_START (ti80)
    ROM_REGION( 0x10000, "maincpu", 0)
	ROM_DEFAULT_BIOS("v40")

	ROM_SYSTEM_BIOS( 0, "v30", "V 3.0" )
    ROMX_LOAD( "ti80v3.u1", 0x0000, 0x4000, CRC(701e2b6a) SHA1(8ffb402eabd70c3fc0750b55d5f0e4c3757fbe38), ROM_BIOS(0) )
	ROMX_LOAD( "ti80v3.u2", 0x8000, 0x8000, CRC(9d9d47ac) SHA1(00f53bb4da90fb05458b031fc3a61f093652dfda), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "v40", "V 4.0" )
    ROMX_LOAD( "ti80v4.u1", 0x0000, 0x4000, CRC(5961a60c) SHA1(cb106586620d528007ea9dc04f46d7d3f5c25aeb), ROM_BIOS(1) )
	ROMX_LOAD( "ti80v4.u2", 0x8000, 0x8000, CRC(f5c9edf9) SHA1(0e4bba1825f4d53f65957814f3a999f8d92a7ce4), ROM_BIOS(1) )
ROM_END

//    YEAR  NAME   PARENT   COMPAT  MACHINE   INPUT  STATE       INIT        COMPANY              FULLNAME    FLAGS
COMP( 1995, ti80,  0,       0,      ti80,     ti80,  ti80_state, empty_init, "Texas Instruments", "TI-80",    MACHINE_NO_SOUND_HW | MACHINE_IMPERFECT_TIMING )