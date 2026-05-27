// license:BSD-3-Clause
// copyright-holders:

/*
System 6(MG) MAIN PCB
8910960104 (8910970104)

VRenderZERO+ MagicEyes SoC
M27C801 program ROM
15.34098 XTAL (OSC 2)
14.31818 XTAL (OSC 3)
3x T436416A SRAM (near SoC)
CY62256VLL-70 SRAM
TC58DVM82A1FT00 NAND
R4543 RTC
Altera MAX EPM3128AT100-10 with KC004B sticker
2x empty spaces marked for CXD1178Q
ADM3222 ARU transceiver

List of games in the プチプチメダル (Puchi Puchi Medal) series (so probably all on this system):

1 - ねこショット！ - Neko Shot!
2 - ぱっくんレストラン - Pakkun Restaurant
3 - わくわくかんらんしゃ - Wakuwaku Kanransha
4 - おてんきテンちゃん - Otenki Ten-chan
5 - ぶんぶんブーメラン - Bunbun Boomerang
6 - にがおえぴったんこ - Nigaoe Pittanko
7 - ハッピーおみくじ - Happy Omikuji
8 - ドキドキ！フラワー - Doki Doki! Flower (Z062)
9 - きせかえスタジオ - Kisekae Studio

TODO:
- fix GFX glitches (then it could probably be considered working);
- hook up RTC (is is even used by the dumped game?)
*/


#include "emu.h"

#include "cpu/se3208/se3208.h"
#include "machine/nandflash.h"
#include "machine/nvram.h"
#include "machine/rtc4543.h"
#include "machine/ticket.h"
#include "machine/vrender0.h"

#include "emupal.h"
#include "speaker.h"


// configurable logging
#define LOG_CPLD     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_CPLD)

#include "logmacro.h"

#define LOGCPLD(...)     LOGMASKED(LOG_CPLD,     __VA_ARGS__)


namespace {

class namcos6_state : public driver_device
{
public:
	namcos6_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_hopper(*this, "hopper"),
		m_vr0soc(*this, "vr0soc"),
		m_nand(*this, "nand"),
		m_in0(*this, "IN0")
	{ }

	void namcos6(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<se3208_device> m_maincpu;
	required_device<hopper_device> m_hopper;
	required_device<vrender0soc_device> m_vr0soc;
	required_device<nand_device> m_nand;

	required_ioport m_in0;

	uint32_t m_nand_addr_phase = 0;
	bool m_nand_addr_active = false;

	uint32_t m_pio = 0;

	uint32_t io_cpld_r(offs_t offset, uint32_t mem_mask = ~0);
	void io_cpld_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t pioldat_r();
	void pioldat_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t pioedat_r();

	void program_map(address_map &map) ATTR_COLD;
};


void namcos6_state::machine_start()
{
	save_item(NAME(m_nand_addr_phase));
	save_item(NAME(m_nand_addr_active));
	save_item(NAME(m_pio));
}

uint32_t namcos6_state::io_cpld_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t const addr = offset * 4;

	// keycus check (same number as the keycus sticker)
	if (addr == 0x18)
		return 0x004b0000;

	if (addr == 0x14)
	{
		if (ACCESSING_BITS_16_31)
		{
			return (m_in0->read() << 16) | 0xffff;
		}
	}

	if (addr == 0x0c)
	{
		if (ACCESSING_BITS_16_31)
		{
			return m_nand->is_busy() ? 0x00000000 : 0x00010000;
		}
		// NAND data port
		if (ACCESSING_BITS_0_15)
		{
			return m_nand->data_r() & 0x00ff;
		}
	}

	LOGCPLD("%s: io_cpld_r %02x & %08x\n", machine().describe_context(), addr, mem_mask);

	return 0;
}

void namcos6_state::io_cpld_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t const addr = offset * 4;

	if ((addr == 0x00) && (ACCESSING_BITS_0_15))
	{
		m_hopper->motor_w(BIT(data, 4));
	}

	// NAND is accessed through the CPLD
	if (addr == 0x08 && ACCESSING_BITS_16_31)
	{
		uint16_t const cmd = (data >> 16) & 0xffff;

		if (cmd == 0x0100)
		{
			// Read mode
			m_nand->command_w(0x00);
			m_nand_addr_phase = 0;
			m_nand_addr_active = false;
		}
		else if (cmd == 0x0001)
		{
			// Open address phase
			m_nand_addr_phase = 0;
			m_nand_addr_active = true;
		}
		else if (cmd == 0x0000)
		{
			// Close address phase; if 3 bytes arrived, NAND is ready to stream
			m_nand_addr_active = false;
		}
	}

	// NAND address bytes
	if (addr == 0x0c && ACCESSING_BITS_0_15)
	{
		if (m_nand_addr_active && m_nand_addr_phase < 3)
		{
			m_nand->address_w(data & 0xff);
			m_nand_addr_phase++;
		}
	}

	LOGCPLD("%s: io_cpld_w %02x = %08x & %08x\n", machine().describe_context(), addr, data, mem_mask);
}

// RTC is accessed through PIO?
uint32_t namcos6_state::pioldat_r()
{
	return m_pio;
}

void namcos6_state::pioldat_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pio);
}

uint32_t namcos6_state::pioedat_r()
{
	return 0x00800000;
}


void namcos6_state::program_map(address_map &map)
{
	map(0x00000000, 0x000fffff).rom().nopw();
	map(0x01000000, 0x01007fff).ram().share("nvram");
	map(0x01600000, 0x0160001f).rw(FUNC(namcos6_state::io_cpld_r), FUNC(namcos6_state::io_cpld_w));
	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x01802004, 0x01802007).rw(FUNC(namcos6_state::pioldat_r), FUNC(namcos6_state::pioldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(namcos6_state::pioedat_r));
	map(0x02000000, 0x027fffff).ram();
	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));
}


static INPUT_PORTS_START( namcos6 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) // move between items in test screen
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) // select item
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) // select in gameplay
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void namcos6_state::namcos6(machine_config &config)
{
	SE3208(config, m_maincpu, 14.318181_MHz_XTAL * 3); // multiplier not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos6_state::program_map);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	RTC4543(config, "rtc", 32.768_kHz_XTAL);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TOSHIBA_TC58256AFT(config, m_nand, 0); // actually TC58DVM82A1FT00, but compatible as far as MAME is concerned

	HOPPER(config, m_hopper, attotime::from_msec(100));

	VRENDER0_SOC(config, m_vr0soc, 14.318181_MHz_XTAL * 6); // multiplier not verified
	m_vr0soc->set_host_space_tag(m_maincpu, AS_PROGRAM);
	m_vr0soc->int_callback().set_inputline(m_maincpu, se3208_device::SE3208_INT);
	m_vr0soc->set_external_vclk(14.318181_MHz_XTAL * 2); // not verified

	SPEAKER(config, "speaker", 2).front();
	m_vr0soc->add_route(0, "speaker", 1.0, 0);
	m_vr0soc->add_route(1, "speaker", 1.0, 1);
}


ROM_START( ddflower )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "dfl1_prg0.6e", 0x000000, 0x100000, CRC(74106ef7) SHA1(d98360b6eb14f5e43ed8386f48e8be4954fa28f7) ) // 1xxxxxxxxxxxxxxxxxxx = 0x00

	// the following was hand-fixed to have the spare bytes at the end of every page instead of all at the end of the file.
	// original dump with following hashes: CRC(51d1ac3c) SHA1(dccecee9c1313a02fa9360501d6e15e961da2909)
	ROM_REGION( 0x1080000, "nand", 0 )
	ROM_LOAD( "tc58dvm82a1ft00.7c", 0x0000000, 0x1080000, CRC(d4f06272) SHA1(28f141887223c132fdbbdd88aa32d47609d220c8) BAD_DUMP )

	ROM_REGION( 0x8000, "nvram", 0 ) // pre-initialized
	ROM_LOAD( "nvram", 0x0000, 0x8000, CRC(2632547f) SHA1(c7b064bbc2e53124fed22b7bd67e846541ddbd91) )
ROM_END

} // anonymous namespace


GAME( 2003, ddflower, 0, namcos6, namcos6, namcos6_state, empty_init, ROT0, "Namco", "Doki Doki! Flower (DFL1, Ver. A07)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
