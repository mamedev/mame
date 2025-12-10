// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "cpu/mcs51/i80c52.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

#define LOG_SERIAL (1U << 1)
#define LOG_KEYS   (1U << 2)
#define LOG_COMM   (1U << 3)
#define LOG_SOUND  (1U << 4)
#define VERBOSE (LOG_SERIAL | LOG_KEYS | LOG_COMM | LOG_SOUND)
#include "logmacro.h"

namespace {

class xe9_state : public driver_device
{
public:
    xe9_state(const machine_config &mconfig, device_type type, const char *tag)
        : driver_device(mconfig, type, tag)
        , m_maincpu(*this, "maincpu")
        , m_soundcpu_l(*this, "soundcpu_l")
        , m_soundcpu_r(*this, "soundcpu_r")
        , m_lcdc(*this, "lcdc")
    { }

    void xe9(machine_config &config);

private:
    void program_map(address_map &map) ATTR_COLD;
    void data_map(address_map &map) ATTR_COLD;
    void soundcpu_l_program_map(address_map &map) ATTR_COLD;
    void soundcpu_l_data_map(address_map &map) ATTR_COLD;
    void soundcpu_r_program_map(address_map &map) ATTR_COLD;
    void soundcpu_r_data_map(address_map &map) ATTR_COLD;

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
    void sam_data_w(u8 data);
    u8 soundcpu_l_port1_r();
    u8 soundcpu_r_port1_r();
    void update_handshake();

    // Sound chip interface (0x8000-0x8007 on sound CPUs)
    u8 soundchip_l_r(offs_t offset);
    void soundchip_l_w(offs_t offset, u8 data);
    u8 soundchip_r_r(offs_t offset);
    void soundchip_r_w(offs_t offset, u8 data);

    // Sound CPU port handlers (P2 used for external memory addressing)
    void soundcpu_l_port2_w(u8 data) { }  // Address high byte during MOVX
    void soundcpu_r_port2_w(u8 data) { }  // Address high byte during MOVX

    required_device<i80c32_device> m_maincpu;
    required_device<i80c32_device> m_soundcpu_l;
    required_device<i80c32_device> m_soundcpu_r;
    required_device<hd44780_device> m_lcdc;
    u8 m_port0 = 0xff;
    u8 m_port1 = 0xff;
    u8 m_port2 = 0xff;
    u8 m_port3 = 0xff;

    // Handshake flip-flops (U9) and data latch (U22)
    u8 m_ff_l = 0;       // FF1 state (LEFT co-CPU interrupt)
    u8 m_ff_r = 0;       // FF2 state (RIGHT co-CPU interrupt)
    u8 m_sam_data = 0;   // Data latch U22
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
    map(0xa000, 0xa000).w(FUNC(xe9_state::sam_data_w));  // U22 data latch
}

void xe9_state::keys_w(offs_t offset, u8 data)
{
    //LOGMASKED(LOG_KEYS, "Keys write: [0x%04X] = 0x%02X\n", 0x8000 + offset, data);
}

void xe9_state::sam_data_w(u8 data)
{
    LOGMASKED(LOG_COMM, "SAM data latch write: 0x%02X\n", data);
    m_sam_data = data;
}

u8 xe9_state::soundcpu_l_port1_r()
{
    LOGMASKED(LOG_COMM, "Sound CPU L P1 read: 0x%02X\n", m_sam_data);
    return m_sam_data;
}

u8 xe9_state::soundcpu_r_port1_r()
{
    LOGMASKED(LOG_COMM, "Sound CPU R P1 read: 0x%02X\n", m_sam_data);
    return m_sam_data;
}

void xe9_state::update_handshake()
{
    // Decode T0 (P3.4) and T1 (P3.5) from main CPU port3
    // U8 74LS139: A1=T0, A0=T1
    int t0 = BIT(m_port3, 4);  // P3.4/T0
    int t1 = BIT(m_port3, 5);  // P3.5/T1

    // Decoder outputs (directly from T0,T1 bits - active low)
    // O0 active when T0=0, T1=0 -> reset both flip-flops
    // O1 active when T0=0, T1=1 -> set FF1 (LEFT)
    // O2 active when T0=1, T1=0 -> set FF2 (RIGHT)
    // O3 active when T0=1, T1=1 -> other function (address decoder enable)

    if (t0 == 0 && t1 == 0)
    {
        // Reset both flip-flops
        if (m_ff_l || m_ff_r)
            LOGMASKED(LOG_COMM, "Handshake: Reset both flip-flops\n");
        m_ff_l = 0;
        m_ff_r = 0;
    }
    else if (t0 == 0 && t1 == 1)
    {
        // Set FF1 (LEFT co-CPU)
        if (!m_ff_l)
            LOGMASKED(LOG_COMM, "Handshake: Trigger LEFT co-CPU INT0\n");
        m_ff_l = 1;
    }
    else if (t0 == 1 && t1 == 0)
    {
        // Set FF2 (RIGHT co-CPU)
        if (!m_ff_r)
            LOGMASKED(LOG_COMM, "Handshake: Trigger RIGHT co-CPU INT0\n");
        m_ff_r = 1;
    }

    // Update interrupt lines
    // FF.Q -> INT0 on sound CPUs (directly active high)
    m_soundcpu_l->set_input_line(MCS51_INT0_LINE, m_ff_l ? ASSERT_LINE : CLEAR_LINE);
    m_soundcpu_r->set_input_line(MCS51_INT0_LINE, m_ff_r ? ASSERT_LINE : CLEAR_LINE);

    // FF.~Q -> INT0/INT1 on main CPU (directly active high, so inverted)
    m_maincpu->set_input_line(MCS51_INT0_LINE, m_ff_l ? CLEAR_LINE : ASSERT_LINE);
    m_maincpu->set_input_line(MCS51_INT1_LINE, m_ff_r ? CLEAR_LINE : ASSERT_LINE);
}

void xe9_state::soundcpu_l_program_map(address_map &map)
{
    map(0x0000, 0xffff).rom().region("soundcpu_l", 0);
}

void xe9_state::soundcpu_l_data_map(address_map &map)
{
    map(0x0000, 0x1fff).ram();  // 8KB RAM
    map(0x8000, 0x8007).rw(FUNC(xe9_state::soundchip_l_r), FUNC(xe9_state::soundchip_l_w));
}

void xe9_state::soundcpu_r_program_map(address_map &map)
{
    map(0x0000, 0xffff).rom().region("soundcpu_r", 0);
}

void xe9_state::soundcpu_r_data_map(address_map &map)
{
    map(0x0000, 0x1fff).ram();  // 8KB RAM
    map(0x8000, 0x8007).rw(FUNC(xe9_state::soundchip_r_r), FUNC(xe9_state::soundchip_r_w));
}

u8 xe9_state::soundchip_l_r(offs_t offset)
{
    LOGMASKED(LOG_SOUND, "Sound chip L read: [0x%04X]\n", 0x8000 + offset);
    return 0xff;
}

void xe9_state::soundchip_l_w(offs_t offset, u8 data)
{
    LOGMASKED(LOG_SOUND, "Sound chip L write: [0x%04X] = 0x%02X\n", 0x8000 + offset, data);
}

u8 xe9_state::soundchip_r_r(offs_t offset)
{
    LOGMASKED(LOG_SOUND, "Sound chip R read: [0x%04X]\n", 0x8000 + offset);
    return 0xff;
}

void xe9_state::soundchip_r_w(offs_t offset, u8 data)
{
    LOGMASKED(LOG_SOUND, "Sound chip R write: [0x%04X] = 0x%02X\n", 0x8000 + offset, data);
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
    // if (m_port3 != 0xff) LOGMASKED(LOG_SERIAL, "P3 read: 0x%02X\n", m_port3);
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
    // if (m_port3 != data)
    //     LOGMASKED(LOG_SERIAL, "P3 write: 0x%02X\n", data);
    m_port3 = data;
    update_handshake();
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

    // Sound CPU L
    I80C32(config, m_soundcpu_l, 16_MHz_XTAL);
    m_soundcpu_l->set_addrmap(AS_PROGRAM, &xe9_state::soundcpu_l_program_map);
    m_soundcpu_l->set_addrmap(AS_DATA, &xe9_state::soundcpu_l_data_map);
    m_soundcpu_l->port_in_cb<1>().set(FUNC(xe9_state::soundcpu_l_port1_r));  // Read data from U22 latch

    // Sound CPU R
    I80C32(config, m_soundcpu_r, 16_MHz_XTAL);
    m_soundcpu_r->set_addrmap(AS_PROGRAM, &xe9_state::soundcpu_r_program_map);
    m_soundcpu_r->set_addrmap(AS_DATA, &xe9_state::soundcpu_r_data_map);
    m_soundcpu_r->port_in_cb<1>().set(FUNC(xe9_state::soundcpu_r_port1_r));  // Read data from U22 latch
}

ROM_START(xe9)
    ROM_REGION(0x10000, "program", 0)  // Main CPU
    ROM_LOAD("xe9v_v14.bin", 0x0000, 0x10000, CRC(c1fd59d5) SHA1(36d28ca3eba29faecfad11e953833777ea1ed6e4))

    ROM_REGION(0x10000, "soundcpu_l", 0)  // Sound CPU L
    ROM_LOAD("xe9l_v141.bin", 0x0000, 0x10000, CRC(5c826efe) SHA1(ea3b61129baafeb0bf0e2ff72f297f6eb098287c))

    ROM_REGION(0x10000, "soundcpu_r", 0)  // Sound CPU R
    ROM_LOAD("xe9r_v141.bin", 0x0000, 0x10000, CRC(37e2e807) SHA1(1039870d922c09685eb4c32a1a8e3fa8922ffd42))
ROM_END

} // anonymous namespace

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY   FULLNAME  FLAGS
SYST( 1990, xe9,  0,      0,      xe9,     xe9,   xe9_state,  empty_init, "Hohner", "XE9",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
