// license:BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Qume QVT-70/QVT-82 terminal

    QVT-70:
    - Z80 (Z8040008VSC)
    - Z80 DART (Z0847006PSC)
    - QUME 303489-01 QFP144
    - DTC 801000-02 QFP100
    - ROM 128k + 64k
    - CXK5864CM-70LL (8k, next to ROMs)
    - W242575-70LL (32k) + 5x CXK5864CM-70LL (8k)
    - DS1231
    - Beeper + Battery
    - XTAL unreadable

    Features:
    - 65 hz with 16x16 characters
    - 78 hz with 16x13 characters
    - 64 background/foreground colors
    - 80/132 columns

    QVT-82:
    - Z80 (Z0840008PSC)
    - Z80 DART (Z0847006PSC)
    - QUME 303489-01 QFP144
    - ROM 64k * 2
    - RAM 8k UM6264AK-10L (above Z80) + 8k UM6264K-70L * 3 (below Z80)
    - DS1231
    - 54.2857MHz XTAL
    - Battery

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80sio.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"


namespace {

class qvt70_state : public driver_device
{
public:
	qvt70_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rombank(*this, "rom"),
		m_rambank(*this, "ram%d", 0U),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void qvt70(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_memory_bank m_rombank;
	required_memory_bank_array<2> m_rambank;
	required_device<gfxdecode_device> m_gfxdecode;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);

	void rombank_w(uint8_t data);

	// gate array
	void voffset_lsb_w(uint8_t data);
	void voffset_msb_w(uint8_t data);
	uint8_t unk_1d_r();
	void unk_1d_w(uint8_t data);
	uint8_t unk_1e_r();
	void unk_1e_w(uint8_t data);
	void unk_1f_w(uint8_t data);
	uint8_t unk_32_r();
	void unk_42_w(uint8_t data);
	void unk_60_w(uint8_t data);

	std::unique_ptr<uint8_t[]> m_ram;

	uint8_t m_1d, m_1e, m_1f;
	bool m_nmi_enable;

	uint16_t m_voffset;
};

void qvt70_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr(m_rombank);
	map(0x8000, 0x8000).w(FUNC(qvt70_state::rombank_w));
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xdfff).bankrw(m_rambank[0]);
	map(0xe000, 0xffff).bankrw(m_rambank[1]);
}

void qvt70_state::io_map(address_map &map)
{
	map.global_mask(0xff);
//  map(0x07, 0x07) // ? (set to 0x5d)
//  map(0x08, 0x08) // ? (set to 0x00)
//  map(0x09, 0x09) // ? (set to 0x00)
	map(0x0a, 0x0a).w(FUNC(qvt70_state::voffset_lsb_w));
	map(0x0b, 0x0b).w(FUNC(qvt70_state::voffset_msb_w));
//  map(0x0c, 0x0c) // ? (set to 0x5e then 0x67)
//  map(0x0d, 0x0d) // ? (set to 0x0c then 0x04)
//  map(0x0e, 0x0e) // ? (set to 0x0f then 0x07)
//  map(0x0f, 0x0f) // ? (set to 0x06 then 0x07)
//  map(0x10, 0x10) // columns? (set to 0x50 = 80)
//  map(0x11, 0x11) // columns? (set to 0x84 = 132)
//  map(0x12, 0x12) // ? (set to 0x31 = 49)
//  map(0x13, 0x13) // rows? (set to 0x19 = 25)
//  map(0x14, 0x14) // ? (set to 0x39 = 57)
//  map(0x15, 0x15) // ? (set to 0x60 = 96)
//  map(0x16, 0x16) // ? (set to 0x39 = 57)
//  map(0x17, 0x17) // debug output? (used during memtest)
//  map(0x18, 0x18) // ? (set to 0x20 = 32)
//  map(0x19, 0x19) // ? (set to 0x0f = 15)
//  map(0x1a, 0x1a) // ? (set to 0x00 then 0xff)
//  map(0x1b, 0x1b) // ? (set to 0x20 = 32)
//  map(0x1c, 0x1c) // ? (set to 0x50 then 0xff)
	map(0x1d, 0x1d).rw(FUNC(qvt70_state::unk_1d_r), FUNC(qvt70_state::unk_1d_w)); // ram banking? status?
	map(0x1e, 0x1e).rw(FUNC(qvt70_state::unk_1e_r), FUNC(qvt70_state::unk_1e_w)); // ram banking?
	map(0x1f, 0x1f).w(FUNC(qvt70_state::unk_1f_w));
//  map(0x20, 0x20) // ? (set to 0x00)
//  map(0x21, 0x21) // ? (set to 0x00)
//  map(0x22, 0x22) // ? (set to 0x00)
//  map(0x23, 0x23) // ? (set to 0x00)
//  map(0x24, 0x24) // ? (set to 0x00)
//  map(0x25, 0x25) // ? (set to 0xa0)
//  map(0x26, 0x26) // ? (set to 0xff)
//  map(0x27, 0x27) // ? (set to 0x80)
//  map(0x28, 0x28) // ? (set to 0x9f)
	// 29-31
	map(0x32, 0x32).r(FUNC(qvt70_state::unk_32_r)); // keyboard data?
	// 33-41
	map(0x42, 0x42).w(FUNC(qvt70_state::unk_42_w)); // nmi enable?
	// 43-54
	map(0x60, 0x60).w(FUNC(qvt70_state::unk_60_w)); // nmi enable?
	// 61-6f
	map(0x80, 0x83).rw("dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
}

static INPUT_PORTS_START( qvt70 )
	PORT_START("1d")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "1d:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "1d:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "1d:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "1d:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "1d:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "1d:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "1d:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x00, "1d:8")

	PORT_START("1e")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "1e:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "1e:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "1e:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "1e:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "1e:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "1e:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "1e:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x00, "1e:8")

	PORT_START("32")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "32:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "32:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "32:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "32:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "32:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "32:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "32:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x00, "32:8")
INPUT_PORTS_END

static const gfx_layout char_layout_8x16 =
{
	8, 16,
	256,
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	8*16
};

static const gfx_layout char_layout_8x9 =
{
	8, 9,
	256,
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_RAM(nullptr, 0, char_layout_8x16, 0, 1)
	GFXDECODE_RAM(nullptr, 0, char_layout_8x9, 0, 1)
GFXDECODE_END

uint32_t qvt70_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_voffset == 0)
		return 0;

	for (int y = 0; y < 27; y++)
	{
		for (int x = 0; x < 132; x++)
		{
			uint8_t code = m_maincpu->space(AS_PROGRAM).read_byte(m_voffset + (y * 132) + x);

			// draw 16 lines
			for (int i = 0; i < 16; i++)
			{
				uint8_t data = m_ram[0x2000 * 7 | ((code << 4) + i)];

				// 8 pixels of the character
				for (int p = 0; p < 8; p++)
					bitmap.pix(y * 16 + i, x * 8 + p) = BIT(data, 7 - p) ? rgb_t::white() : rgb_t::black();
			}
		}
	}

	return 0;
}

void qvt70_state::vblank_w(int state)
{
	if (state == 1 && m_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void qvt70_state::voffset_lsb_w(uint8_t data)
{
	m_voffset = (m_voffset & 0xff00) | data;
}

void qvt70_state::voffset_msb_w(uint8_t data)
{
	m_voffset = (data << 8) | (m_voffset & 0x00ff);
	logerror("voffset = %04x\n", m_voffset);
}

uint8_t qvt70_state::unk_1d_r()
{
	// 7654---- unknown
	// ----3--- data available on io port 32
	// -----210 unknown

	uint8_t val = 0;
	val = ioport("1d")->read();
//  val = machine().rand();
	logerror("1d read: %02x\n", val);

	return val;
}

void qvt70_state::unk_1d_w(uint8_t data)
{
	logerror("1d = %02x\n", data);

	m_1d = data;

	// banking is likely wrong

	// the vram test sets 1d = 04 and wants c000-dfff and e000-ffff to
	// point to the same area

	// c000-dfff
	switch (data & 0x0c)
	{
	case 0x00: m_rambank[0]->set_entry(0); break;
	case 0x04: m_rambank[0]->set_entry(1); break;
	case 0x08: m_rambank[0]->set_entry(2); break;
	case 0x0c: m_rambank[0]->set_entry(3); break;
	}

	// e000-ffff
	switch (data & 0xc0)
	{
	case 0x00: m_rambank[1]->set_entry(1); break;
	case 0x40: m_rambank[1]->set_entry(4 + BIT(m_1e, 3)); break;
	case 0x80: m_rambank[1]->set_entry(6 + BIT(m_1e, 2)); break;
	case 0xc0: m_rambank[1]->set_entry(8); break;
	}
}

uint8_t qvt70_state::unk_1e_r()
{
	uint8_t val = 0;
	val = ioport("1e")->read();
//  val = machine().rand();
	logerror("1e read: %02x\n", val);

	return val;
}

void qvt70_state::unk_1e_w(uint8_t data)
{
	logerror("1e = %02x\n", data);
	m_1e = data;
}

void qvt70_state::unk_1f_w(uint8_t data)
{
	logerror("1f = %02x\n", data);
	m_1f = data;
}

uint8_t qvt70_state::unk_32_r()
{
	uint8_t val = 0;
	val = ioport("32")->read();
//  val = machine().rand();
	logerror("32 read: %02x\n", val);

	return val;
}

void qvt70_state::unk_42_w(uint8_t data)
{
	logerror("42 = %02x\n", data);
	m_nmi_enable = bool(BIT(data, 0));
}

void qvt70_state::unk_60_w(uint8_t data)
{
	logerror("60 = %02x\n", data);
//  m_nmi_enable = bool(BIT(data, 7));
}

void qvt70_state::rombank_w(uint8_t data)
{
	if (data & ~0x19)
		logerror("rombank_w: %02x\n", data);

	// 765-----  unknown
	// ---43---  bankswitching
	// -----21-  unknown
	// -------0  bankswitching

	switch (data & 0x19)
	{
		case 0x00: m_rombank->set_entry(0); break;
		case 0x08: m_rombank->set_entry(1); break;
		case 0x10: m_rombank->set_entry(2); break;
		case 0x18: m_rombank->set_entry(3); break;
		case 0x01: m_rombank->set_entry(4); break;
		case 0x11: m_rombank->set_entry(5); break;

		default:
			logerror("Unknown ROM bank: %02x\n", data);
	}
}

void qvt70_state::machine_start()
{
	m_rombank->configure_entries(0, 6, memregion("maincpu")->base(), 0x8000);

	m_ram = std::make_unique<uint8_t[]>(0x12000);
	m_rambank[0]->configure_entries(0, 9, &m_ram[0], 0x2000);
	m_rambank[1]->configure_entries(0, 9, &m_ram[0], 0x2000);

	m_gfxdecode->gfx(0)->set_source(&m_ram[0x2000*7 + 0x0000]);
	m_gfxdecode->gfx(1)->set_source(&m_ram[0x2000*7 + 0x1000]);

	save_pointer(NAME(m_ram), 0x12000);
}

void qvt70_state::machine_reset()
{
	m_nmi_enable = false;
}

void qvt70_state::qvt70(machine_config &config)
{
	Z80(config, m_maincpu, 8'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &qvt70_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &qvt70_state::io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(70);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_screen_update(FUNC(qvt70_state::screen_update));
	screen.set_size(1056, 432);
	screen.set_visarea(0, 1056-1, 0, 432-1);
	screen.screen_vblank().set(FUNC(qvt70_state::vblank_w));

	PALETTE(config, "palette").set_entries(64);

	GFXDECODE(config, m_gfxdecode, "palette", chars);

	Z80DART(config, "dart", 4'000'000);
	// dart irq connected to maincpu int

	// 25-pin (dcd, rxd, txd, dtr, dsr, rts, cts and current loop)
	RS232_PORT(config, "serial1", default_rs232_devices, nullptr);

	// 9-pin (dcd, rxd, txd, dtr, dsr, rts, cts)
	RS232_PORT(config, "serial2", default_rs232_devices, nullptr);

	CENTRONICS(config, "parallel", centronics_devices, nullptr);
}

ROM_START( qvt70 )
	ROM_REGION(0x30000, "maincpu", 0)
	ROM_LOAD( "251513-03_revj.u11", 0x00000, 0x20000, CRC(c56796fe) SHA1(afe024ff93d5e75dc18041219d61e1a22fc6d883) ) // 251513-03  C/S:D1DA  95' REV.J (checksum matches)
	ROM_LOAD( "251513-04_revj.u12", 0x20000, 0x10000, CRC(3960bbd5) SHA1(9db306cef09be21ff43c081ebe11e9b46f617861) ) // 251513-04  C/S:18D0  95' REV.J (checksum matches)
ROM_END

ROM_START( qvt82 )
	ROM_REGION(0x30000, "maincpu", 0)
	ROM_LOAD( "304229-02d_revd.u6", 0x00000, 0x10000, CRC(597431df) SHA1(10c4669b759dd7cfd6746e54dc12807197cf841a) ) // 304229-02D  QVT-82 REV. D  U6 (BF7F) (checksum matches)
	ROM_LOAD( "304229-01d_revd.u5", 0x20000, 0x10000, CRC(9ebd09b6) SHA1(ef9f002016d05b770e7b66d15f05fc286bd022d9) ) // 304229-01D  QVT-82 REV. D  U5 (462B) (checksum matches)
ROM_END

} // anonymous namespace


COMP( 1992, qvt70, 0, 0, qvt70, qvt70, qvt70_state, empty_init, "Qume", "QVT-70", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1993, qvt82, 0, 0, qvt70, qvt70, qvt70_state, empty_init, "Qume", "QVT-82", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
