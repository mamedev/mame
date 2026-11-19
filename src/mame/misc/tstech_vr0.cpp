// license:BSD-3-Clause
// copyright-holders:

/*
TS Technology VRenderZERO+ based hardware

Main components:
VRenderZERO+ MagicEyes EISC SoC
3x Samsung K4S641632H-TC75 SDRAM
2x Sharp LH28F320BJD-TTL80 flash (U9, U10)
Actel eX128-F TQ100 FPGA
Hitachi HM62256ALP-10 SRAM (battery-backed)
SOIC8 I2C EEPROM (unreadable, the program accesses it as a 24C04)
3x 74HC165 parallel-in/serial-out shift registers (inputs)
25.175 MHz oscillator (OSC1, near VR0+)
14.318 MHz XTAL
battery

The flash data goes through the Actel, which decrypts it (see init_xfkddz).
The program runs straight from the flash, there's no loader.

TODO:
- the 74HC165 on IN0 is also scanned as a 5x6 matrix (IN0 bits 1-6, rows on PIO bits 0-4), used
  instead of IN0-IN2 when bit 7 of the third settings byte (0x0231bc22) is set, not mapped;
- outputs written at 0x07000002;
- the boot code reads 0x05000000 32 times and 0x01500000 72 times, maybe an unlock sequence
  for the Actel, the decryption is done statically for now;
- verify CPU and VR0+ clocks;
*/

#include "emu.h"

#include "cpu/se3208/se3208.h"
#include "machine/74165.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/vrender0.h"

#include "speaker.h"

#include "endianness.h"


namespace {

class tstech_vr0_state : public driver_device
{
public:
	tstech_vr0_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc"),
		m_shifter(*this, "ttl165_%u", 0U),
		m_eeprom(*this, "eeprom"),
		m_hopper(*this, "hopper"),
		m_in(*this, "IN%u", 0U),
		m_keys(*this, "KEY%u", 0U),
		m_io(*this, "IO")
	{}

	void xfkddz(machine_config &config) ATTR_COLD;

	void init_xfkddz() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
	required_device_array<ttl165_device, 3> m_shifter;
	required_device<i2cmem_device> m_eeprom;
	required_device<hopper_device> m_hopper;

	required_ioport_array<3> m_in;
	required_ioport_array<5> m_keys;
	required_ioport m_io;

	uint32_t m_pio = 0;
	uint32_t m_pio_in = 0;

	uint32_t pioldat_r();
	void pioldat_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t pioedat_r();
	uint8_t in0_r();
	uint8_t io_r();
	void eeprom_w(uint8_t data);
	template <unsigned N> void qh_w(int state);

	void program_map(address_map &map) ATTR_COLD;
};


void tstech_vr0_state::machine_start()
{
	save_item(NAME(m_pio));
	save_item(NAME(m_pio_in));
}

uint32_t tstech_vr0_state::pioldat_r()
{
	return m_pio;
}

// PIO Latched output DATa Register
// bits 0-4: input matrix rows (active low), bit 13: 74HC165 SH/LD, bit 14: 74HC165 CLK
void tstech_vr0_state::pioldat_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pio);

	for (auto &shifter : m_shifter)
	{
		shifter->shift_load_w(BIT(m_pio, 13));
		shifter->clock_w(BIT(m_pio, 14));
	}
}

// PIO External DATa Register
// bits 5-7: 74HC165 QH
uint32_t tstech_vr0_state::pioedat_r()
{
	return m_pio_in;
}

uint8_t tstech_vr0_state::in0_r()
{
	uint8_t data = m_in[0]->read();

	for (int i = 0; i < 5; i++)
		if (!BIT(m_pio, i))
			data &= m_keys[i]->read();

	return data;
}

// bit 4: EEPROM SDA
uint8_t tstech_vr0_state::io_r()
{
	return (m_io->read() & ~0x10) | (m_eeprom->read_sda() << 4);
}

// bit 0: EEPROM SCL, bit 1: EEPROM SDA
void tstech_vr0_state::eeprom_w(uint8_t data)
{
	m_eeprom->write_sda(BIT(data, 1));
	m_eeprom->write_scl(BIT(data, 0));
}

template <unsigned N>
void tstech_vr0_state::qh_w(int state)
{
	m_pio_in = (m_pio_in & ~(1 << (5 + N))) | (state << (5 + N));
}


void tstech_vr0_state::program_map(address_map &map)
{
	map(0x00000000, 0x007fffff).rom();

	map(0x01500000, 0x01507fff).ram().share("nvram");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x01802004, 0x01802007).rw(FUNC(tstech_vr0_state::pioldat_r), FUNC(tstech_vr0_state::pioldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(tstech_vr0_state::pioedat_r));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));

	map(0x05000000, 0x057fffff).rom().region("maincpu", 0);

	map(0x07000000, 0x07000003).lw16(NAME([this] (uint16_t data) { m_hopper->motor_w(BIT(data, 0)); })).umask32(0x0000ffff);
	map(0x07000000, 0x07000003).nopw().umask32(0xffff0000); // TODO: outputs
	map(0x07000004, 0x07000007).r(FUNC(tstech_vr0_state::io_r)).umask32(0x000000ff);
	map(0x07000004, 0x07000007).w(FUNC(tstech_vr0_state::eeprom_w)).umask32(0xff000000);
}


static INPUT_PORTS_START( xfkddz )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) // also plays the selected cards
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Pass / Double")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Confirm / Cancel Selection")

	PORT_START("IN2")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW ) // held at boot with Book: clears the bookkeeping
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x1c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	// matrix on IN0 bits 1-6, rows selected by PIO bits 0-4
	PORT_START("KEY0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IO")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) // EEPROM SDA
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void tstech_vr0_state::xfkddz(machine_config &config)
{
	SE3208(config, m_maincpu, 14.318181_MHz_XTAL * 3); // multiplier not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &tstech_vr0_state::program_map);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	VRENDER0_SOC(config, m_vr0soc, 14.318181_MHz_XTAL * 6); // multiplier not verified
	m_vr0soc->set_host_space_tag(m_maincpu, AS_PROGRAM);
	m_vr0soc->int_callback().set_inputline(m_maincpu, se3208_device::SE3208_INT);
	m_vr0soc->set_external_vclk(25.175_MHz_XTAL);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	I2C_24C04(config, m_eeprom);

	HOPPER(config, m_hopper, attotime::from_msec(50));

	TTL165(config, m_shifter[0]);
	m_shifter[0]->data_callback().set(FUNC(tstech_vr0_state::in0_r));
	m_shifter[0]->qh_callback().set(FUNC(tstech_vr0_state::qh_w<0>));

	TTL165(config, m_shifter[1]);
	m_shifter[1]->data_callback().set_ioport(m_in[1]);
	m_shifter[1]->qh_callback().set(FUNC(tstech_vr0_state::qh_w<1>));

	TTL165(config, m_shifter[2]);
	m_shifter[2]->data_callback().set_ioport(m_in[2]);
	m_shifter[2]->qh_callback().set(FUNC(tstech_vr0_state::qh_w<2>));

	SPEAKER(config, "speaker", 2).front();
	m_vr0soc->add_route(0, "speaker", 1.0, 0);
	m_vr0soc->add_route(1, "speaker", 1.0, 1);
}


// 新疯狂斗地主 (两副) (Xīn Fēngkuáng Dòu Dìzhǔ (Liǎng Fù))
ROM_START( xfkddz )
	ROM_REGION32_LE( 0x800000, "maincpu", ROMREGION_ERASEFF )
	// decrypted here

	ROM_REGION( 0x800000, "flash", 0 ) // 2x LH28F320BJD-TTL80
	ROM_LOAD( "u10.bin", 0x000000, 0x400000, CRC(c720dbac) SHA1(fdba9dfcdf8820c4b6b47a3c6f617090c701917b) )
	ROM_LOAD( "u9.bin",  0x400000, 0x400000, CRC(c1e90bca) SHA1(a33841049a7e6c5fb0c9df9c9e26ae24493b005a) )

	ROM_REGION32_LE( 0x8000, "nvram", 0 )
	ROM_LOAD( "nvram", 0x0000, 0x8000, CRC(24b3e626) SHA1(c557365fca4d6785bbf136e5cef7611f189dd6f6) ) // pre-initialized to avoid black screen at first boot
ROM_END


void tstech_vr0_state::init_xfkddz()
{
	uint8_t const *const src = memregion("flash")->base();
	uint8_t *const rom = memregion("maincpu")->base();

	for (uint32_t i = 0; i < 0x400000; i++)
	{
		// word address bits 0-20 go to A1-A21 of the U10/U9 pair, bit 21 to A0
		uint32_t const a = ((i << 1) & 0x3ffffe) | BIT(i, 21);
		uint16_t x = src[a * 2] | (src[a * 2 + 1] << 8);

		// word address bits 0-7 invert a data line each
		if (i & 0x01) x ^= 0x8000;
		if (i & 0x02) x ^= 0x2000;
		if (i & 0x04) x ^= 0x0010;
		if (i & 0x08) x ^= 0x0001;
		if (i & 0x10) x ^= 0x0800;
		if (i & 0x20) x ^= 0x0004;
		if (i & 0x40) x ^= 0x0200;
		if (i & 0x80) x ^= 0x0020;

		// then the data lines are mixed
		uint16_t const x15_13_11_9 = BIT(x, 15) ^ BIT(x, 13) ^ BIT(x, 11) ^ BIT(x, 9);
		uint16_t const x4_2_0 = BIT(x, 4) ^ BIT(x, 2) ^ BIT(x, 0);
		uint16_t const data =
				((x15_13_11_9 ^ x4_2_0 ^ BIT(x, 5) ^ BIT(x, 1)) << 15) |
				(BIT(x, 7) << 14) |
				(BIT(x, 12) << 13) |
				(BIT(x, 3) << 12) |
				(BIT(x, 6) << 11) |
				(BIT(x, 8) << 10) |
				((BIT(x, 15) ^ BIT(x, 14)) << 9) |
				((BIT(x, 15) ^ BIT(x, 13) ^ BIT(x, 10)) << 8) |
				((BIT(x, 13) ^ BIT(x, 4)) << 7) |
				((BIT(x, 4) ^ BIT(x, 0)) << 6) |
				((BIT(x, 11) ^ BIT(x, 0)) << 5) |
				((BIT(x, 11) ^ BIT(x, 2)) << 4) |
				((BIT(x, 9) ^ BIT(x, 2)) << 3) |
				((BIT(x, 9) ^ BIT(x, 5)) << 2) |
				((x15_13_11_9 ^ x4_2_0) << 1) |
				((x15_13_11_9 ^ x4_2_0 ^ BIT(x, 5)) << 0);

		rom[BYTE4_XOR_LE(i * 2)] = data & 0xff;
		rom[BYTE4_XOR_LE(i * 2 + 1)] = data >> 8;
	}
}

} // anonymous namespace


GAME( 2008, xfkddz, 0, xfkddz, xfkddz, tstech_vr0_state, init_xfkddz, ROT0, "TS Technology", "Xin Fengkuang Dou Dizhu", MACHINE_NOT_WORKING )
