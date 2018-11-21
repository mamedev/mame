// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************************

PINBALL
Gottlieb System 1

Gottlieb's first foray into computerised pinball.

Typical of Gottlieb's golden period, these machines are physically well-designed and made.
However, the computer side was another story, an attempt to catch up to its competitors who
were way ahead in the technology race. Instead of each board being solidly grounded to the
chassis, the only connections were through flaky edge connectors. Voltage differences would
then cause solenoids and lights to switch on at random and destroy transistors. Further, the
CPU chips chosen were an unusual 4-bit design that was already old.

The first games had chimes. Then, this was replaced by 3 NE555 tone oscillators. The last
machines had a real sound board which had more computing power than the main cpu.

Game numbering:
Each Gottlieb game had the model number printed on the instruction card, so it was very
easy to gather information. Gottlieb either made a single-player game, or a 2-player and
a 4-player game. For example, Centigrade 37 (#407) was a single-player game, while Bronco
(4-player)(#396) was exactly the same as Mustang (2-player)(#397). Cleopatra (#409) was
originally a 4-player EM game (with Pyramid #410 being the 2-player version). Then, the SS
version was made, and it kept the same number. After that, the SS versions were suffixed
with 'SS' up to The Incredible Hulk (#433), and then the 'SS' was dropped.


Game List:
Number  ROM  Name
409     A    Cleopatra
412SS   B    Sinbad
417SS   C    Joker Poker
419SS   D    Dragon
421SS   E    Solar Ride
422SS   F    Countdown
424SS   G    Close Encounters of the third kind
425SS   H    Charlie's Angels
427SS   I    Pinball Pool
429SS   J    Totem
433SS   K    The Incredible Hulk
435     L    Genie
437     N    Buck Rogers
438     P    Torch
440     R    Roller Disco
442     S    Asteroid Annie and the Aliens

Chips used:
U1 11660     CPU
U2 10696EE   5101L RAM interface (device#6)
U3 10696EE   General purpose I/O (dipswitches, lamps, misc) (device#3)
U4 A1753CX   Custom 2kx8 ROM, 128x4 RAM, 16x1 I/O (solenoid control)
U5 A1752CX   Custom 2kx8 ROM, 128x4 RAM, 16x1 I/O (switch matrix)
U6 10788     Display driver
   5101L     4-bit static RAM
   MM6351-IJ ROM


ToDo:
- Everything
- Hard to debug because no errors are logged; also the program flow seems odd.
- 5101L RAM (battery-backed) is driven from the 10696.
- MM6351 ROM is driven from the CPU I/O ports and has 4 banks.

*****************************************************************************************************/


#include "emu.h"
#include "machine/genpin.h"
#include "machine/ra17xx.h"
#include "machine/r10696.h"
#include "machine/r10788.h"
#include "cpu/pps4/pps4.h"
#include "gts1.lh"

#define VERBOSE    1
#include "logmacro.h"


class gts1_state : public genpin_class
{
public:
	gts1_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dips(*this, "DSW%u", 0)
		, m_switches(*this, "X.%u", 0)
		, m_digit8(*this, "digit8_%u", 0U)
		, m_digit7(*this, "digit7_%u", 0U)
	{ }

	void gts1(machine_config &config);

private:
	DECLARE_READ8_MEMBER (gts1_solenoid_r);
	DECLARE_WRITE8_MEMBER(gts1_solenoid_w);
	DECLARE_READ8_MEMBER (gts1_switches_r);
	DECLARE_WRITE8_MEMBER(gts1_switches_w);
	DECLARE_WRITE8_MEMBER(gts1_display_w);
	DECLARE_READ8_MEMBER (gts1_lamp_apm_r);
	DECLARE_WRITE8_MEMBER(gts1_lamp_apm_w);
	DECLARE_READ8_MEMBER (gts1_nvram_r);
	DECLARE_WRITE8_MEMBER(gts1_nvram_w);
	DECLARE_READ8_MEMBER (gts1_io_r);
	DECLARE_WRITE8_MEMBER(gts1_io_w);
	DECLARE_READ8_MEMBER (gts1_pa_r);
	DECLARE_WRITE8_MEMBER(gts1_do_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void gts1_map(address_map &map);
	void gts1_data(address_map &map);
	void gts1_io(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_ioport_array<3> m_dips;
	required_ioport_array<5> m_switches;
	output_finder<32> m_digit8; // driver currently uses 0-6, 8-14, 16-22 and 24-30
	output_finder<32> m_digit7; // driver currently uses 7, 15, 23 and 31

	uint8_t m_strobe;             //!< switches strobe lines (5 lower bits used)
	uint8_t m_nvram_addr;         //!< NVRAM address
	bool m_nvram_e2;            //!< NVRWAM enable (E2 line)
	bool m_nvram_wr;            //!< NVRWAM write (W/R line)
	uint16_t m_6351_addr;         //!< ROM MM6351 address (12 bits)
	uint16_t m_z30_out;           //!< 4-to-16 decoder outputs
};

void gts1_state::gts1_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void gts1_state::gts1_data(address_map &map)
{
	map(0x0000, 0x00ff).ram();
	map(0x0100, 0x01ff).ram().share("nvram");
}

void gts1_state::gts1_io(address_map &map)
{
	map(0x0000, 0x00ff).r(FUNC(gts1_state::gts1_io_r)).w(FUNC(gts1_state::gts1_io_w));         // catch undecoded I/O accesss

	map(0x0020, 0x002f).rw("u4", FUNC(ra17xx_device::io_r), FUNC(ra17xx_device::io_w)); // (U4) solenoid
	map(0x0030, 0x003f).rw("u3", FUNC(r10696_device::io_r), FUNC(r10696_device::io_w)); // (U3) solenoid + dips
	map(0x0040, 0x004f).rw("u5", FUNC(ra17xx_device::io_r), FUNC(ra17xx_device::io_w)); // (U5) switch matrix
	map(0x0060, 0x006f).rw("u2", FUNC(r10696_device::io_r), FUNC(r10696_device::io_w)); // (U2) NVRAM io chip
	map(0x00d0, 0x00df).rw("u6", FUNC(r10788_device::io_r), FUNC(r10788_device::io_w)); // (U6) display chip
}

static INPUT_PORTS_START( gts1_dips )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "S01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x20, "S06")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ))
	PORT_DIPNAME( 0x40, 0x40, "S07")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ))
	PORT_DIPNAME( 0x80, 0x80, "S08")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "S09")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S14")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ))
	PORT_DIPSETTING(    0x20, DEF_STR( No ))
	PORT_DIPNAME( 0x40, 0x40, "S15")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ))
	PORT_DIPNAME( 0x80, 0x00, "S16")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "S17")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S18")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S19")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S20")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S21")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S22")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S23")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "S24")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))
INPUT_PORTS_END

static INPUT_PORTS_START( gts1_switches )
	PORT_START("X.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_START("X.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_START("X.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_START("X.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_START("X.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)
INPUT_PORTS_END

static INPUT_PORTS_START( gts1 )
	PORT_INCLUDE( gts1_dips )

	PORT_INCLUDE( gts1_switches )
INPUT_PORTS_END

static INPUT_PORTS_START( jokrpokr )
	PORT_INCLUDE( gts1_dips )

	PORT_START("X.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PLAY/TEST")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("POP/BUMBER")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EXTRA BALL TARGET")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SPECIAL ROLLOVER")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("10 POINT CONTACTS")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"A\" DROP TARGET (red)")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)

	PORT_START("X.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("#1 COIN CHUTE")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"A\" ROLLOVER")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"10\" DROP TARGET")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"Q\" DROP TARGET (red)")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"K\" DROP TARGET (black)")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"A\" DROP TARGET (black)")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)

	PORT_START("X.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("#2 COIN CHUTE")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"B\" ROLLOVER")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"J\" DROP TARGET (black)")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"O\" DROP TARGET (black)")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"K\" DROP TARGET")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("JOKER DROP TARGET")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)

	PORT_START("X.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("REPLAY BUTTON")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"C\" ROLLOVER")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"J\" DROP TARGET (red)")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"O\" DROP TARGET (red)")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"K\" DROP TARGET")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"A\" DROP TARGET (red)")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)

	PORT_START("X.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TILT PANEL")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"K\" DROP TARGET")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("\"A\" DROP TARGET (red)")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)
INPUT_PORTS_END

void gts1_state::machine_start()
{
	genpin_class::machine_start();

	m_digit8.resolve();
	m_digit7.resolve();

	save_item(NAME(m_strobe));
	save_item(NAME(m_nvram_addr));
	save_item(NAME(m_nvram_e2));
	save_item(NAME(m_nvram_wr));
	save_item(NAME(m_6351_addr));
	save_item(NAME(m_z30_out));
}

void gts1_state::machine_reset()
{
	genpin_class::machine_reset();

	m_strobe = 0;
	m_nvram_addr = 0;
	m_nvram_e2 = false;
	m_nvram_wr = false;
	m_6351_addr = 0;
	m_z30_out = 0;
}

READ8_MEMBER (gts1_state::gts1_solenoid_r)
{
	uint8_t data = 0;
	LOG("%s: solenoid[%02x] -> %x\n", __FUNCTION__, offset, data);
	return data;
}

WRITE8_MEMBER(gts1_state::gts1_solenoid_w)
{
	switch (offset)
	{
	case  0:
		LOG("%s: outhole <- %x\n", __FUNCTION__, data);
		break;
	case  1:
		LOG("%s: knocker <- %x\n", __FUNCTION__, data);
		break;
	case  2:
		LOG("%s: tens chime <- %x\n", __FUNCTION__, data);
		break;
	case  3:
		LOG("%s: hundreds chime <- %x\n", __FUNCTION__, data);
		break;
	case  4:
		LOG("%s: thousands chime <- %x\n", __FUNCTION__, data);
		break;
	case  5:
		LOG("%s: no. 6 <- %x\n", __FUNCTION__, data);
		break;
	case  6:
		LOG("%s: no. 7 <- %x\n", __FUNCTION__, data);
		break;
	case  7:
		LOG("%s: no. 8 <- %x\n", __FUNCTION__, data);
		break;
	case  8: case  9: case 10: case 11:
		LOG("%s: not used [%x] <- %x\n", __FUNCTION__, offset, data);
		break;
	case 12:    // spare
		LOG("%s: spare [%x] <- %x\n", __FUNCTION__, offset, data);
		break;
	case 13:    // RAM control E2
		LOG("%s: RAM control E2 <- %x\n", __FUNCTION__, data);
		m_nvram_e2 = (data & 1) ? true : false;
		break;
	case 14:    // RAM control W/R
		LOG("%s: RAM control W/R <- %x\n", __FUNCTION__, data);
		m_nvram_wr = (data & 1) ? true : false;
		break;
	case 15:    // spare
		LOG("%s: spare [%x] <- %x\n", __FUNCTION__, offset, data);
		break;
	}
}

READ8_MEMBER (gts1_state::gts1_switches_r)
{
	uint8_t data = 1;
	if (offset >= 8 && offset < 16) {
		const int bit = offset - 8;
		for (int i = 0; i < 5; i++) {
			if (m_strobe & (1 << i)) {
				data &= BIT(m_switches[i]->read(), bit);
			}
		}
	}
	LOG("%s: switches[%x,%x] -> %x\n", __FUNCTION__, m_strobe, offset, data);
	return data;
}

WRITE8_MEMBER(gts1_state::gts1_switches_w)
{
	LOG("%s: switches[%x] <- %x\n", __FUNCTION__, offset, data);
	if (offset < 5) {
		// outputs O-0 to O-4 are the 5 strobe lines
		m_strobe = (m_strobe & ~(1 << offset)) | ((data & 1) << offset);
	}
}

/**
 * @brief write a 8seg display value
 * @param offset digit number 0 .. 19
 * @param data 4-bit value to display
 */
WRITE8_MEMBER(gts1_state::gts1_display_w)
{
	/*
	 * The 7448 is modified to be disabled through RI/RBO
	 * when the input is 0001, and in this case the extra
	 * output H is generated instead.
	 */
	enum : uint8_t
	{
		_a = 1 << 0,
		_b = 1 << 1,
		_c = 1 << 2,
		_d = 1 << 3,
		_e = 1 << 4,
		_f = 1 << 5,
		_g = 1 << 6,
		_h = 1 << 7
	};
	static constexpr uint8_t ttl7448_mod[16] = {
	/* 0 */  _a | _b | _c | _d | _e | _f,
	/* 1 */  _h,
	/* 2 */  _a | _b | _d | _e | _g,
	/* 3 */  _a | _b | _c | _d | _g,
	/* 4 */  _b | _c | _f | _g,
	/* 5 */  _a | _c | _d | _f | _g,
	/* 6 */  _a | _c | _d | _e | _f | _g,
	/* 7 */  _a | _b | _c,
	/* 8 */  _a | _b | _c | _d | _e | _f | _g,
	/* 9 */  _a | _b | _c | _d | _f | _g,
	/* a */  _d | _e | _g,
	/* b */  _c | _d | _g,
	/* c */  _b | _f | _g,
	/* d */  _a | _d | _f | _g,
	/* e */  _d | _e | _f | _g,
	/* f */  0
	};
	uint8_t a = ttl7448_mod[(data >> 0) & 15];
	uint8_t b = ttl7448_mod[(data >> 4) & 15];
	// LOG("%s: offset:%d data:%02x a:%02x b:%02x\n", __FUNCTION__, offset, data, a, b);
	if ((offset % 8) < 7) { // FIXME: layout suggests this should be < 6 rather than < 7
		m_digit8[offset] = a;
		m_digit8[offset + 16] = b;
	} else {
		/*
		 * For the 4 7-seg displays the segment h is turned back into
		 * segments b and c to display the 7-seg "1".
		 */
		if (a & _h)
			a = _b | _c;
		if (b & _h)
			b = _b | _c;
		m_digit7[offset] = a;
		// FIXME: there is nothing on outputs 22, 23, 30 and 31?
		m_digit7[offset + 16] = b;
	}
}

/**
 * @brief read input groups A, B, C of NVRAM io chip (U2)
 * @param offset 0 ... 2 = group
 * @return 4-bit value read from the group
 */
READ8_MEMBER (gts1_state::gts1_nvram_r)
{
	uint8_t data = 0x0f;
	switch (offset)
	{
		case 0: // group A
			// FIXME: Schematics says TO Z5
			if (!m_nvram_wr && m_nvram_e2) {
				uint8_t* nvram = memregion("nvram")->base();
				assert(nvram != nullptr);
				data = nvram[m_nvram_addr];
				LOG("%s: nvram[%02x] -> %x\n", __FUNCTION__, m_nvram_addr, data);
			}
			break;
		case 1: // group B
		case 2: // group C
			// Schematics says: SPARES
			break;
	}
	return data;
}

/**
 * @brief write output groups A, B, C of NVRAM io chip (U2)
 * @param offset 0 ... 2 = group
 * @param data 4 bit value to write
 */
WRITE8_MEMBER(gts1_state::gts1_nvram_w)
{
	switch (offset)
	{
		case 0: // group A - address lines 3:0
			m_nvram_addr = (m_nvram_addr & ~15) | (data & 15);
			break;
		case 1: // group B - address lines 7:4
			m_nvram_addr = (m_nvram_addr & ~(15 << 4)) | ((data & 15) << 4);
			break;
		case 2: // group C - data bits 3:0 of NVRAM
			if (m_nvram_wr && m_nvram_e2) {
				LOG("%s: nvram[%02x] <- %x\n", __FUNCTION__, m_nvram_addr, data & 15);
				uint8_t* nvram = memregion("nvram")->base();
				assert(nvram != nullptr);
				nvram[m_nvram_addr] = data & 15;
			}
			break;
	}
}

/**
 * @brief read input groups A, B, C of lamp + apm I/O chip (U3)
 * @param offset 0 ... 2 = group
 * @return 4-bit value read from the group
 */
READ8_MEMBER (gts1_state::gts1_lamp_apm_r)
{
	uint8_t data = 0x0f;
	switch (offset) {
		case 0: // group A switches S01-S04, S09-S12, S17-S20
			if (m_z30_out & 1) {
				uint8_t dsw0 = m_dips[0]->read();
				if (0 == BIT(dsw0,0)) // S01
					data &= ~(1 << 3);
				if (0 == BIT(dsw0,1)) // S02
					data &= ~(1 << 2);
				if (0 == BIT(dsw0,2)) // S03
					data &= ~(1 << 1);
				if (0 == BIT(dsw0,3)) // S04
					data &= ~(1 << 0);
			}
			if (m_z30_out & 2) {
				uint8_t dsw1 = m_dips[1]->read();
				if (0 == BIT(dsw1,0)) // S09
					data &= ~(1 << 0);
				if (0 == BIT(dsw1,1)) // S10
					data &= ~(1 << 1);
				if (0 == BIT(dsw1,2)) // S11
					data &= ~(1 << 2);
				if (0 == BIT(dsw1,3)) // S12
					data &= ~(1 << 3);
			}
			if (m_z30_out & 4) {
				uint8_t dsw2 = m_dips[2]->read();
				if (0 == BIT(dsw2,0)) // S17
					data &= ~(1 << 0);
				if (0 == BIT(dsw2,1)) // S18
					data &= ~(1 << 1);
				if (0 == BIT(dsw2,2)) // S19
					data &= ~(1 << 2);
				if (0 == BIT(dsw2,3)) // S20
					data &= ~(1 << 3);
			}
			break;
		case 1: // group B switches S05-S08, S09-S12, S17-S20
			if (m_z30_out & 1) {
				uint8_t dsw0 = m_dips[0]->read();
				if (0 == BIT(dsw0,4)) // S05
					data &= ~(1 << 3);
				if (0 == BIT(dsw0,5)) // S06
					data &= ~(1 << 2);
				if (0 == BIT(dsw0,6)) // S07
					data &= ~(1 << 1);
				if (0 == BIT(dsw0,7)) // S08
					data &= ~(1 << 0);
			}
			if (m_z30_out & 2) {
				uint8_t dsw1 = m_dips[1]->read();
				if (0 == BIT(dsw1,4)) // S13
					data &= ~(1 << 0);
				if (0 == BIT(dsw1,5)) // S14
					data &= ~(1 << 1);
				if (0 == BIT(dsw1,6)) // S15
					data &= ~(1 << 2);
				if (0 == BIT(dsw1,7)) // S16
					data &= ~(1 << 3);
			}
			if (m_z30_out & 4) {
				uint8_t dsw2 = m_dips[2]->read();
				if (0 == BIT(dsw2,4)) // S21
					data &= ~(1 << 0);
				if (0 == BIT(dsw2,5)) // S22
					data &= ~(1 << 1);
				if (0 == BIT(dsw2,6)) // S23
					data &= ~(1 << 2);
				if (0 == BIT(dsw2,7)) // S24
					data &= ~(1 << 3);
			}
			break;
		case 2: // TODO: connect
			// IN-9 (unused?)
			// IN-10 (reset sw25)
			// IN-11 (outhole sw)
			// IN-12 (slam sw)
			break;
	}
	return data;
}

/**
 * @brief write output groups A, B, C of lamp + apm I/O chip (U3)
 * @param offset 0 ... 2 = group
 * @param data 4 bit value to write
 */
WRITE8_MEMBER(gts1_state::gts1_lamp_apm_w)
{
	switch (offset) {
		case 0: // LD1-LD4 on jumper J5
			break;
		case 1: // Z30 1-of-16 decoder
			m_z30_out = 1 << (data & 15);
			break;
		case 2: // O9: PGOL PROM A8, O10: PGOL PROM A9
			m_6351_addr = (m_6351_addr & ~(3 << 8)) | ((data & 3) << 8);
			// O11 and O12 are unused(?)
			break;
	}
}

READ8_MEMBER (gts1_state::gts1_io_r)
{
	const uint8_t data = 0x0f;
	LOG("%s: unmapped io[%02x] -> %x\n", __FUNCTION__, offset, data);
	return data;
}

WRITE8_MEMBER(gts1_state::gts1_io_w)
{
	LOG("%s: unmapped io[%02x] <- %x\n", __FUNCTION__, offset, data);
}

READ8_MEMBER (gts1_state::gts1_pa_r)
{
	// return ROM nibble
	uint8_t *ROM = memregion("maincpu")->base();
	uint8_t data = ROM[0x2000 + m_6351_addr] & 0x0f;
	LOG("%s: ROM[%03x]:%02x\n", __FUNCTION__, m_6351_addr, data);
	return data;
}

WRITE8_MEMBER(gts1_state::gts1_do_w)
{
	// write address lines (DO1-4 to A0-3, DIO1-4 to A4-7)
	m_6351_addr = (m_6351_addr & 0x300) | data;
	LOG("%s: ROM addr:%02x\n", __FUNCTION__, m_6351_addr);
}


void gts1_state::gts1(machine_config & config)
{
	/* basic machine hardware */
	pps4_2_device &maincpu(PPS4_2(config, m_maincpu, XTAL(3'579'545)));  // divided by 18 in the CPU
	maincpu.set_addrmap(AS_PROGRAM, &gts1_state::gts1_map);
	maincpu.set_addrmap(AS_DATA, &gts1_state::gts1_data);
	maincpu.set_addrmap(AS_IO, &gts1_state::gts1_io);
	maincpu.dia_cb().set(FUNC(gts1_state::gts1_pa_r));
	maincpu.do_cb().set(FUNC(gts1_state::gts1_do_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* A1753CE 2048 x 8 ROM (000-7ff), 128 x 4 RAM (00-7f) and 16 I/O lines (20 ... 2f) */
	ra17xx_device &u5(RA17XX(config, "u5", 0));
	u5.iord_cb().set(FUNC(gts1_state::gts1_switches_r));
	u5.iowr_cb().set(FUNC(gts1_state::gts1_switches_w));
	u5.set_cpu_tag(m_maincpu);

	/* A1752CF 2048 x 8 ROM (800-fff), 128 x 4 RAM (80-ff) and 16 I/O lines (40 ... 4f) */
	ra17xx_device &u4(RA17XX(config, "u4", 0));
	u4.iord_cb().set(FUNC(gts1_state::gts1_solenoid_r));
	u4.iowr_cb().set(FUNC(gts1_state::gts1_solenoid_w));
	u4.set_cpu_tag(m_maincpu);

	/* 10696 General Purpose Input/Output */
	r10696_device &u2(R10696(config, "u2", 0));
	u2.iord_cb().set(FUNC(gts1_state::gts1_nvram_r));
	u2.iowr_cb().set(FUNC(gts1_state::gts1_nvram_w));

	/* 10696 General Purpose Input/Output */
	r10696_device &u3(R10696(config, "u3", 0));
	u3.iord_cb().set(FUNC(gts1_state::gts1_lamp_apm_r));
	u3.iowr_cb().set(FUNC(gts1_state::gts1_lamp_apm_w));

	/* 10788 General Purpose Display and Keyboard */
	r10788_device &u6(R10788(config, "u6", XTAL(3'579'545) / 18 ));  // divided in the circuit
	u6.update_cb().set(FUNC(gts1_state::gts1_display_w));

	/* Video */
	config.set_default_layout(layout_gts1);

	/* Sound */
	genpin_audio(config);
}


ROM_START( gts1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
ROM_END

ROM_START( gts1s )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
ROM_END

/*-------------------------------------------------------------------
/ Asteroid Annie and the Aliens (12/1980) #442
/-------------------------------------------------------------------*/
ROM_START(astannie)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("442.cpu", 0x2000, 0x0400, CRC(579521e0) SHA1(b1b19473e1ca3373955ee96104b87f586c4c311c))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("442.snd", 0x0400, 0x0400, CRC(c70195b4) SHA1(ff06197f07111d6a4b8942dcfe8d2279bda6f281))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Buck Rogers (01/1980) #437
/-------------------------------------------------------------------*/
ROM_START(buckrgrs)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("437.cpu", 0x2000, 0x0400, CRC(e57d9278) SHA1(dfc4ebff1e14b9a074468671a8e5ac7948d5b352))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("437.snd", 0x0400, 0x0400, CRC(732b5a27) SHA1(7860ea54e75152246c3ac3205122d750b243b40c))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Charlie's Angels (11/1978) #425
/-------------------------------------------------------------------*/
ROM_START(charlies)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("425.cpu", 0x2000, 0x0400, CRC(928b4279) SHA1(51096d45e880d6a8263eaeaa0cdab0f61ad2f58d))
ROM_END
/*-------------------------------------------------------------------
/ Cleopatra (11/1977) #409
/-------------------------------------------------------------------*/
ROM_START(cleoptra)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("409.cpu", 0x2000, 0x0400, CRC(8063ff71) SHA1(205f09f067bf79544d2ce2a48d23259901f935dd))
ROM_END

/*-------------------------------------------------------------------
/ Close Encounters of the Third Kind (10/1978) #424
/-------------------------------------------------------------------*/
ROM_START(closeenc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("424.cpu", 0x2000, 0x0400, CRC(a7a5dd13) SHA1(223c67b9484baa719c91de52b363ff22813db160))
ROM_END

/*-------------------------------------------------------------------
/ Count-Down (05/1979) #422
/-------------------------------------------------------------------*/
ROM_START(countdwn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("422.cpu", 0x2000, 0x0400, CRC(51bc2df0) SHA1(d4b555d106c6b4e420b0fcd1df8871f869476c22))
ROM_END

/*-------------------------------------------------------------------
/ Dragon (10/1978) #419
/-------------------------------------------------------------------*/
ROM_START(dragon)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("419.cpu", 0x2000, 0x0400, CRC(018d9b3a) SHA1(da37ef5017c71bc41bdb1f30d3fd7ac3b7e1ee7e))
ROM_END

/*-------------------------------------------------------------------
/ Genie (11/1979) #435
/-------------------------------------------------------------------*/
ROM_START(geniep)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("435.cpu", 0x2000, 0x0400, CRC(7749fd92) SHA1(9cd3e799842392e3939877bf295759c27f199e58))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("435.snd", 0x0400, 0x0400, CRC(4a98ceed) SHA1(f1d7548e03107033c39953ee04b043b5301dbb47))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Joker Poker (08/1978) #417
/-------------------------------------------------------------------*/
ROM_START(jokrpokr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("417.cpu", 0x2000, 0x0400, CRC(33dade08) SHA1(23b8dbd7b6c84b806fc0d2da95478235cbf9f80a))
ROM_END

/*-------------------------------------------------------------------
/ Jungle Queen (1985)
/-------------------------------------------------------------------*/
/*-------------------------------------------------------------------
/ L'Hexagone (04/1986)
/-------------------------------------------------------------------*/
ROM_START(hexagone)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("435.cpu", 0x2000, 0x0400, CRC(7749fd92) SHA1(9cd3e799842392e3939877bf295759c27f199e58))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("hexagone.bin", 0, 0x4000, CRC(002b5464) SHA1(e2d971c4e85b4fb6580c2d3945c9946ea0cebc2e))
ROM_END
/*-------------------------------------------------------------------
/ Movie
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Pinball Pool (08/1979) #427
/-------------------------------------------------------------------*/
ROM_START(pinpool)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("427.cpu", 0x2000, 0x0400, CRC(c496393d) SHA1(e91d9596aacdb4277fa200a3f8f9da099c278f32))
ROM_END

/*-------------------------------------------------------------------
/ Roller Disco (02/1980) #440
/-------------------------------------------------------------------*/
ROM_START(roldisco)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("440.cpu", 0x2000, 0x0400, CRC(bc50631f) SHA1(6aa3124d09fc4e369d087a5ad6dd1737ace55e41))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("440.snd", 0x0400, 0x0400, CRC(4a0a05ae) SHA1(88f21b5638494d8e78dc0b6b7d69873b76b5f75d))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Sahara Love (1984)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Sinbad (05/1978) #412
/-------------------------------------------------------------------*/
ROM_START(sinbad)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("412.cpu", 0x2000, 0x0400, CRC(84a86b83) SHA1(f331f2ffd7d1b279b4ffbb939aa8649e723f5fac))
ROM_END

ROM_START(sinbadn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("412no1.cpu", 0x2000, 0x0400, CRC(f5373f5f) SHA1(027840501416ff01b2adf07188c7d667adf3ad5f))
ROM_END

/*-------------------------------------------------------------------
/ Sky Warrior (1983)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Solar Ride (02/1979) #421
/-------------------------------------------------------------------*/
ROM_START(solaride)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("421.cpu", 0x2000, 0x0400, CRC(6b5c5da6) SHA1(a09b7009473be53586f53f48b7bfed9a0c5ecd55))
ROM_END

/*-------------------------------------------------------------------
/ The Incredible Hulk (10/1979) #433
/-------------------------------------------------------------------*/
ROM_START(hulk)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("433.cpu", 0x2000, 0x0400, CRC(c05d2b52) SHA1(393fe063b029246317c90ee384db95a84d61dbb7))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("433.snd", 0x0400, 0x0400, CRC(20cd1dff) SHA1(93e7c47ff7051c3c0dc9f8f95aa33ba094e7cf25))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Torch (02/1980) #438
/-------------------------------------------------------------------*/
ROM_START(torch)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("438.cpu", 0x2000, 0x0400, CRC(2d396a64) SHA1(38a1862771500faa471071db08dfbadc6e8759e8))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("438.snd", 0x0400, 0x0400, CRC(a9619b48) SHA1(1906bc1b059bf31082e3b4546f5a30159479ad3c))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Totem (10/1979) #429
/-------------------------------------------------------------------*/
ROM_START(totem)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("429.cpu", 0x2000, 0x0400, CRC(7885a384) SHA1(1770662af7d48ad8297097a9877c5c497119978d))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("429.snd", 0x0400, 0x0400, CRC(5d1b7ed4) SHA1(4a584f880e907fb21da78f3b3a0617f20599688f))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sys1.bin", 0x0c00, 0x0400, CRC(b7831321) SHA1(c94f4bee97854d0373653a6867016e27d3fc1340))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ System 1 Test prom
/-------------------------------------------------------------------*/
ROM_START(sys1test)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u5_cf.bin", 0x0000, 0x0800, CRC(e0d4b405) SHA1(17aadd79c0dcbb336aadd5d203bc6ca866492345))
	ROM_LOAD("u4_ce.bin", 0x0800, 0x0800, CRC(4cd312dd) SHA1(31245daa9972ef8652caee69986585bb8239e86e))
	ROM_LOAD("test.cpu", 0x2000, 0x0400, CRC(8b0704bb) SHA1(5f0eb8d5af867b815b6012c9d078927398efe6d8))
ROM_END


GAME(1977,  gts1,     0,      gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "System 1",                  MACHINE_IS_BIOS_ROOT | MACHINE_NOT_WORKING)

//Exact same roms as gts1 with added hardware we'll likely need roms for to emulate properly
GAME(1979,  gts1s,    gts1,   gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "System 1 with sound board", MACHINE_IS_BIOS_ROOT | MACHINE_NOT_WORKING)
GAME(19??,  sys1test, gts1,   gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "System 1 Test prom",                   MACHINE_IS_SKELETON_MECHANICAL)

// chimes
GAME(1977,  cleoptra, gts1,   gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Cleopatra",                            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1978,  sinbad,   gts1,   gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Sinbad",                               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1978,  sinbadn,  sinbad, gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Sinbad (Norway)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1978,  jokrpokr, gts1,   gts1, jokrpokr, gts1_state, empty_init, ROT0, "Gottlieb",         "Joker Poker",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1978,  dragon,   gts1,   gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Dragon",                               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1979,  solaride, gts1,   gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Solar Ride",                           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1979,  countdwn, gts1,   gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Count-Down",                           MACHINE_IS_SKELETON_MECHANICAL)

// NE555 beeper
GAME(1978,  closeenc, gts1,   gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Close Encounters of the Third Kind",   MACHINE_IS_SKELETON_MECHANICAL)
GAME(1978,  charlies, gts1,   gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Charlie's Angels",                     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1979,  pinpool,  gts1,   gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Pinball Pool",                         MACHINE_IS_SKELETON_MECHANICAL)

// sound card
GAME(1979,  totem,    gts1s,  gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Totem",                                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1979,  hulk,     gts1s,  gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "The Incredible Hulk",                  MACHINE_IS_SKELETON_MECHANICAL)
GAME(1979,  geniep,   gts1s,  gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Genie (Pinball)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  buckrgrs, gts1s,  gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Buck Rogers",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  torch,    gts1s,  gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Torch",                                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  roldisco, gts1s,  gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Roller Disco",                         MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  astannie, gts1s,  gts1, gts1,     gts1_state, empty_init, ROT0, "Gottlieb",         "Asteroid Annie and the Aliens",        MACHINE_IS_SKELETON_MECHANICAL)

// homebrew
GAME(1986,  hexagone, gts1s,  gts1, gts1,     gts1_state, empty_init, ROT0, "Christian Tabart", "L'Hexagone (France)",                  MACHINE_IS_SKELETON_MECHANICAL)
