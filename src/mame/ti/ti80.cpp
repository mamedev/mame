// license:BSD-3-Clause
// copyright-holders:grubbyplaya
/*****************************************************************************

    TI-80 graphing calculator

****************************************************************************/

#include "emu.h"

#include "bus/ti8x/ti8x.h"
#include "cpu/t6m53/t6m53.h"
#include "video/t6b79.h"

#include "emupal.h"
#include "screen.h"


namespace {

class ti80_state : public driver_device
{
public:
	ti80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
        m_maincpu(*this, "maincpu"),
        m_link_port(*this, "linkport"),
        m_btn_cols(*this, "COL%u", 0U)
	{
	}

	void ti80(machine_config &config);

private:
	required_device<t6m53_device> m_maincpu;
    required_device<ti8x_link_port_device> m_link_port;
	required_ioport_array<7> m_btn_cols;

    void mem_map(address_map &map);
    uint8_t btns_r(offs_t cols);
    void palette(palette_device &palette) const;
};


static constexpr rgb_t ti80_colors[128] =
{
    rgb_t(84, 111, 138),   rgb_t(83, 111, 138),  //0x00
    rgb_t(86, 114, 138),   rgb_t(83, 111, 138),  //0x01
    rgb_t(89, 116, 139),   rgb_t(83, 111, 138),  //0x02
    rgb_t(91, 119, 141),   rgb_t(83, 111, 138),  //0x03
    rgb_t(94, 122, 142),   rgb_t(83, 111, 138),  //0x04
    rgb_t(95, 124, 142),   rgb_t(83, 111, 138),  //0x05
    rgb_t(97, 127, 143),   rgb_t(83, 111, 138),  //0x06
    rgb_t(100, 128, 145),  rgb_t(83, 111, 138),  //0x07
    rgb_t(102, 131, 146),  rgb_t(83, 111, 138),  //0x08
    rgb_t(105, 133, 146),  rgb_t(83, 111, 138),  //0x09
    rgb_t(107, 136, 148),  rgb_t(83, 111, 138),  //0x0a
    rgb_t(110, 138, 149),  rgb_t(83, 111, 138),  //0x0b
    rgb_t(112, 141, 149),  rgb_t(83, 111, 138),  //0x0c
    rgb_t(114, 144, 150),  rgb_t(83, 111, 138),  //0x0d
    rgb_t(117, 146, 152),  rgb_t(83, 111, 138),  //0x0e
    rgb_t(119, 149, 153),  rgb_t(83, 111, 138),  //0x0f
    rgb_t(122, 150, 153),  rgb_t(83, 111, 138),  //0x10
    rgb_t(124, 153, 154),  rgb_t(83, 111, 138),  //0x11
    rgb_t(127, 155, 156),  rgb_t(83, 111, 138),  //0x12
    rgb_t(129, 158, 157),  rgb_t(83, 111, 138),  //0x13
    rgb_t(131, 161, 157),  rgb_t(83, 111, 138),  //0x14
    rgb_t(134, 163, 158),  rgb_t(83, 111, 138),  //0x15
    rgb_t(136, 166, 160),  rgb_t(83, 111, 138),  //0x16
    rgb_t(139, 168, 160),  rgb_t(83, 111, 138),  //0x17
    rgb_t(141, 170, 161),  rgb_t(83, 111, 138),  //0x18
    rgb_t(144, 172, 162),  rgb_t(83, 111, 138),  //0x19
    rgb_t(145, 175, 164),  rgb_t(83, 111, 138),  //0x1a
    rgb_t(147, 178, 164),  rgb_t(83, 111, 138),  //0x1b
    rgb_t(150, 180, 165),  rgb_t(83, 111, 138),  //0x1c
    rgb_t(152, 183, 166),  rgb_t(83, 111, 138),  //0x1d
    rgb_t(155, 185, 168),  rgb_t(83, 111, 138),  //0x1e
    rgb_t(158, 187, 168),  rgb_t(83, 111, 138),  //0x1f
    rgb_t(159, 188, 169),  rgb_t(83, 111, 138),  //0x20
    rgb_t(160, 190, 170),  rgb_t(83, 111, 138),  //0x21
    rgb_t(160, 190, 170),  rgb_t(83, 111, 138),  //0x22
    rgb_t(160, 190, 170),  rgb_t(83, 111, 138),  //0x23
    rgb_t(160, 190, 170),  rgb_t(83, 112, 138),  //0x24
    rgb_t(160, 190, 170),  rgb_t(84, 112, 138),  //0x25
    rgb_t(160, 190, 170),  rgb_t(85, 113, 139),  //0x26
    rgb_t(160, 190, 170),  rgb_t(87, 114, 139),  //0x27
    rgb_t(160, 190, 170),  rgb_t(88, 116, 139),  //0x28
    rgb_t(160, 190, 170),  rgb_t(89, 118, 140),  //0x29
    rgb_t(160, 190, 170),  rgb_t(92, 119, 141),  //0x2a
    rgb_t(160, 190, 170),  rgb_t(93, 121, 141),  //0x2b
    rgb_t(160, 190, 170),  rgb_t(95, 124, 142),  //0x2c
    rgb_t(160, 190, 170),  rgb_t(97, 127, 143),  //0x2d
    rgb_t(160, 190, 170),  rgb_t(101, 129, 144), //0x2e
    rgb_t(160, 190, 170),  rgb_t(104, 132, 145), //0x2f
    rgb_t(160, 190, 170),  rgb_t(106, 135, 147), //0x30
    rgb_t(160, 190, 170),  rgb_t(109, 138, 148), //0x31
    rgb_t(160, 190, 170),  rgb_t(112, 141, 149), //0x32
    rgb_t(160, 190, 170),  rgb_t(116, 145, 151), //0x33
    rgb_t(160, 190, 170),  rgb_t(120, 150, 153), //0x34
    rgb_t(160, 190, 170),  rgb_t(124, 153, 154), //0x35
    rgb_t(160, 190, 170),  rgb_t(129, 158, 157), //0x36
    rgb_t(160, 190, 170),  rgb_t(133, 161, 158), //0x37
    rgb_t(160, 190, 170),  rgb_t(137, 167, 160), //0x38
    rgb_t(160, 190, 170),  rgb_t(142, 171, 162), //0x39
    rgb_t(160, 190, 170),  rgb_t(147, 177, 164), //0x3a
    rgb_t(160, 190, 170),  rgb_t(152, 181, 166), //0x3b
    rgb_t(160, 190, 170),  rgb_t(157, 188, 168), //0x3c
    rgb_t(160, 190, 170),  rgb_t(160, 190, 170), //0x3d
    rgb_t(160, 190, 170),  rgb_t(160, 190, 170), //0x3e
    rgb_t(160, 190, 170),  rgb_t(160, 190, 170), //0x3f
};

void ti80_state::mem_map(address_map &map)
{
    map(0x0000, 0x3ffd).rom();
    map(0x3ffe, 0x3ffe).rw("t6b79", FUNC(t6b79_device::control_read), FUNC(t6b79_device::control_write));
    map(0x3fff, 0x3fff).rw("t6b79", FUNC(t6b79_device::data_read), FUNC(t6b79_device::data_write));
    map(0x4000, 0x4000).mirror(0x0ffe).rw("t6b79", FUNC(t6b79_device::control_read), FUNC(t6b79_device::control_write));
    map(0x4001, 0x4001).mirror(0x0ffe).rw("t6b79", FUNC(t6b79_device::data_read), FUNC(t6b79_device::data_write));
    map(0x5000, 0x6fff).ram();
    map(0x8000, 0xffff).rom();
}

static INPUT_PORTS_START (ti80)
	PORT_START("COL0")   /* col 0 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)

	PORT_START("COL1")   /* col 1 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('+')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('*')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('^')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_PGDN)

	PORT_START("COL2")   /* col 2 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(-)") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(')')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAN") PORT_CODE(KEYCODE_PGUP)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("VARS") PORT_CODE(KEYCODE_F9)

	PORT_START("COL3")   /* col 3 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('(')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("COS") PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PRGM") PORT_CODE(KEYCODE_F8)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("STAT") PORT_CODE(KEYCODE_TILDE)

	PORT_START("COL4")   /* col 4 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_CHAR(',')
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

uint8_t ti80_state::btns_r(offs_t cols)
{
    uint8_t data = 0;
    for (int col = 0; col < 7; col++)
        if (BIT(cols, col))
            for (int row = 0; row < 8; row++)
                data |= BIT(m_btn_cols[col]->read(), row) ? (1 << row) : 0;

    return data;
}

void ti80_state::palette(palette_device &palette) const
{
	for (int i = 0; i < 64 * 2; i++)
        palette.set_pen_color(i, ti80_colors[i]);
}

void ti80_state::ti80(machine_config &config)
{
    T6M53(config, m_maincpu, 1'960'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ti80_state::mem_map);
    m_maincpu->btn_rows().set(FUNC(ti80_state::btns_r));
    m_maincpu->on_btn().set_ioport("ON");

	screen_device &screen(SCREEN(config, "screen").set_lcd());
    screen.set_refresh_hz(60);
	screen.set_size(64, 48);
	screen.set_visarea(0, 64-1, 0, 48-1);

    T6B79(config, "t6b79");
	screen.set_screen_update("t6b79", FUNC(t6b79_device::screen_update));
    m_maincpu->lcd_stb().set("t6b79", FUNC(t6b79_device::stb_write));

	PALETTE(config, "palette", FUNC(ti80_state::palette), 128, 128);
	screen.set_palette("palette");

    // The link port is only present on viewscreen TI-80s, which have the exact same ROMs as a regular TI-80.
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

} // anonymous namespace

//    YEAR  NAME   PARENT   COMPAT  MACHINE   INPUT  STATE       INIT        COMPANY              FULLNAME    FLAGS
COMP( 1995, ti80,  0,       0,      ti80,     ti80,  ti80_state, empty_init, "Texas Instruments", "TI-80",    MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
