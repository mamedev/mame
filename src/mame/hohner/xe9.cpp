// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "cpu/mcs51/i80c52.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

#define LOG_SERIAL (1U << 1)
#define LOG_KEYS   (1U << 2)
#define VERBOSE (LOG_SERIAL | LOG_KEYS)
#include "logmacro.h"

namespace {

class xe9_state : public driver_device
{
public:
    xe9_state(const machine_config &mconfig, device_type type, const char *tag)
        : driver_device(mconfig, type, tag)
        , m_maincpu(*this, "maincpu")
        , m_lcdc(*this, "lcdc")
    { }

    void xe9(machine_config &config);

private:
    void program_map(address_map &map) ATTR_COLD;
    void data_map(address_map &map) ATTR_COLD;

    void port0_w(u8 data);
    void port1_w(u8 data);
    void port2_w(u8 data);
    void port3_w(u8 data);
    u8 port0_r();
    u8 port1_r();
    u8 port2_r();
    u8 port3_r();

    void palette_init(palette_device &palette);
    HD44780_PIXEL_UPDATE(lcd_pixel_update);

    void keys_w(offs_t offset, u8 data);

    required_device<i80c32_device> m_maincpu;
    required_device<hd44780_device> m_lcdc;
    u8 m_port0 = 0xff;
    u8 m_port1 = 0xff;
    u8 m_port2 = 0xff;
    u8 m_port3 = 0xff;
};

void xe9_state::program_map(address_map &map)
{
    map(0x0000, 0xffff).rom().region("program", 0);
}

void xe9_state::data_map(address_map &map)
{
    //map(0x0000, 0xffff).ram();
    map(0x0000, 0x1fff).ram();
    map(0x2000, 0x2001).rw(m_lcdc, FUNC(hd44780_device::read), FUNC(hd44780_device::write));
    map(0x8000, 0x8010).w(FUNC(xe9_state::keys_w));
}

void xe9_state::keys_w(offs_t offset, u8 data)
{
    //LOGMASKED(LOG_KEYS, "Keys write: [0x%04X] = 0x%02X\n", 0x8000 + offset, data);
}

void xe9_state::palette_init(palette_device &palette)
{
    palette.set_pen_color(0, rgb_t(131, 136, 139)); // LCD background
    palette.set_pen_color(1, rgb_t( 92,  83,  88)); // LCD pixel on
}

HD44780_PIXEL_UPDATE(xe9_state::lcd_pixel_update)
{
    if (x < 5 && y < 8 && line < 2 && pos < 20)
        bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

u8 xe9_state::port0_r()
{
    LOGMASKED(LOG_SERIAL, "P0 read: 0x%02X\n", m_port0);
    return m_port0;
}

u8 xe9_state::port1_r()
{
    LOGMASKED(LOG_SERIAL, "P1 read: 0x%02X\n", m_port1);
    return m_port1;
}

u8 xe9_state::port2_r()
{
    LOGMASKED(LOG_SERIAL, "P2 read: 0x%02X\n", m_port2);
    return m_port2;
}

u8 xe9_state::port3_r()
{
    if (m_port3 != 0xff) LOGMASKED(LOG_SERIAL, "P3 read: 0x%02X\n", m_port3);
    return m_port3;
}

void xe9_state::port0_w(u8 data)
{
    if (m_port0 != data)
        LOGMASKED(LOG_SERIAL, "P0 write: 0x%02X\n", data);
    m_port0 = data;
}

void xe9_state::port1_w(u8 data)
{
    if (m_port1 != data)
        LOGMASKED(LOG_SERIAL, "P1 write: 0x%02X\n", data);
    m_port1 = data;
}

void xe9_state::port2_w(u8 data)
{
    if (m_port2 != data)
        LOGMASKED(LOG_SERIAL, "P2 write: 0x%02X\n", data);
    m_port2 = data;
}

void xe9_state::port3_w(u8 data)
{
    if (m_port3 != data)
        LOGMASKED(LOG_SERIAL, "P3 write: 0x%02X\n", data);
    m_port3 = data;
}

static INPUT_PORTS_START(xe9)
INPUT_PORTS_END

void xe9_state::xe9(machine_config &config)
{
    I80C32(config, m_maincpu, 16_MHz_XTAL);
    m_maincpu->set_addrmap(AS_PROGRAM, &xe9_state::program_map);
    m_maincpu->set_addrmap(AS_DATA, &xe9_state::data_map);
    m_maincpu->port_in_cb<0>().set(FUNC(xe9_state::port0_r));
    m_maincpu->port_in_cb<1>().set(FUNC(xe9_state::port1_r));
    m_maincpu->port_in_cb<2>().set(FUNC(xe9_state::port2_r));
    m_maincpu->port_in_cb<3>().set(FUNC(xe9_state::port3_r));
    m_maincpu->port_out_cb<0>().set(FUNC(xe9_state::port0_w));
    m_maincpu->port_out_cb<1>().set(FUNC(xe9_state::port1_w));
    m_maincpu->port_out_cb<2>().set(FUNC(xe9_state::port2_w));
    m_maincpu->port_out_cb<3>().set(FUNC(xe9_state::port3_w));

    // LCD
    screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
    screen.set_refresh_hz(60);
    screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
    screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
    screen.set_size(6 * 20, 8 * 2);  // 20 chars x 2 lines, 6x8 pixels per char
    screen.set_visarea_full();
    screen.set_palette("palette");

    PALETTE(config, "palette", FUNC(xe9_state::palette_init), 2);

    HD44780(config, m_lcdc, 270'000);  // typical HD44780 clock
    m_lcdc->set_lcd_size(2, 20);
    m_lcdc->set_pixel_update_cb(FUNC(xe9_state::lcd_pixel_update));
}

ROM_START(xe9)
    ROM_REGION(0x10000, "program", 0)
    ROM_SYSTEM_BIOS(0, "v14", "V1.4")
    ROMX_LOAD("xe9v_v14.bin", 0x0000, 0x10000, CRC(c1fd59d5) SHA1(36d28ca3eba29faecfad11e953833777ea1ed6e4), ROM_BIOS(0))
    ROM_SYSTEM_BIOS(1, "v141l", "V1.41 (L)")
    ROMX_LOAD("xe9l_v141.bin", 0x0000, 0x10000, CRC(5c826efe) SHA1(ea3b61129baafeb0bf0e2ff72f297f6eb098287c), ROM_BIOS(1))
    ROM_SYSTEM_BIOS(2, "v141r", "V1.41 (R)")
    ROMX_LOAD("xe9r_v141.bin", 0x0000, 0x10000, CRC(37e2e807) SHA1(1039870d922c09685eb4c32a1a8e3fa8922ffd42), ROM_BIOS(2))
ROM_END

} // anonymous namespace

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY   FULLNAME  FLAGS
SYST( 1990, xe9,  0,      0,      xe9,     xe9,   xe9_state,  empty_init, "Hohner", "XE9",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
