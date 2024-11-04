// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************************

PINBALL
Recel/Petaco System III

This is similar - but not the same - as Gottlieb System 1. Used by various Spanish
pinball manufacturers. Recel is the name used for export Petaco machines.

Known machines:
Alaska, Hot & Cold, Screech, Mr. Evil, Torneo, Crazy Race, Fair Fight, Poker Plus, Mr. Doom, Cavalier, SwashBuckler,
Don Quijote, Space Game, Space Game (Bingo 6+1), The Flipper Game, Black Magic, Black Magic 4, Conquistador.

Suspected machines:
Bingo Space, Black Aritipe, Formula 1, Lucky Roll.

Chips used:
B1   A2361-13 or A1761-13|14  Early RIOT-type device: Custom 1kx8 ROM, 128x4 RAM, 16x1 I/O.
B2   A2362-13 or A1762-13|14  Early RIOT-type device: Custom 1kx8 ROM, 128x4 RAM, 16x1 I/O.
B3   11696                    General Purpose I/O expander (no datasheet found, assuming it's similar to 10696).
B4   11660                    Rockwell Parallel Processing System 4-bit CPU (PPS/4-2).
B5   10788                    Display driver.
C2   HM6508                   1x1024-bit static RAM, battery-backed.
C5   1702A or 2716            Personality PROM (the first revision of the Recel PCB uses 1702A as EPROM, a later revision upgraded to 2716).
C4   10738                    Bus Interface Circuit }
C6   10738                    Bus Interface Circuit } These 2 interface the C5 EPROM to the CPU.


ToDo:
- Everything (the code below is mostly a carry-over from gts1 and is incomplete or guesswork).
- There are lots of manuals, with lots of info, but not what we need. For example, no proper schematics.
- No info on the sound (all it says is 4 TTL chips controlled by 6 bits of the I/O expander).
- A plug-in printer is used to view and alter settings. We have no info about it.
- Default layout.
- Outputs.

*****************************************************************************************************/


#include "emu.h"
#include "genpin.h"
#include "machine/ra17xx.h"
#include "machine/r10696.h"
#include "machine/r10788.h"
#include "cpu/pps4/pps4.h"
#include "recel.lh"

#define VERBOSE    1
#include "logmacro.h"

namespace {

class recel_state : public genpin_class
{
public:
	recel_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pm(*this, "module")   // personality module
		, m_nvram(*this, "nvram")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digit(*this, "digit%d", 0U)
	{ }

	void recel(machine_config &config);

private:
	u8 solenoids_r(offs_t offset);
	void solenoids_w(offs_t offset, u8 data);
	u8 switches_r(offs_t offset);
	void switches_w(offs_t offset, u8 data);
	void display_w(offs_t offset, u8 data);
	u8 lamps_r(offs_t offset);
	void lamps_w(offs_t offset, u8 data);
	u8 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u8 data);
	[[maybe_unused]]u8 bic_r(offs_t offset);
	[[maybe_unused]]void bic_w(offs_t offset, u8 data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void recel_map(address_map &map) ATTR_COLD;
	void recel_data(address_map &map) ATTR_COLD;
	void recel_io(address_map &map) ATTR_COLD;

	required_device<pps4_2_device> m_maincpu;
	required_region_ptr<u8> m_pm;
	required_shared_ptr<u8> m_nvram;
	required_ioport_array<6> m_io_keyboard;
	output_finder<32> m_digit;

	u8 m_strobe = 0U;
	u16 m_nvram_addr = 0U;
	u8 m_nvram_data = 0U;
	bool m_nvram_prev_clk = false;
	u8 m_prom_addr = 0U;
};

void recel_state::recel_map(address_map &map) // need address ranges
{
	map(0x0000, 0x07ff).rom();  // ROM inside B1/B2 chips
	// map(0x0800, 0x08ff).w(FUNC(recel_state::bic_w));  //  set address to BIC C4
}

void recel_state::recel_data(address_map &map) // should be ok
{
	map(0x0000, 0x00ff).ram();  // RAM inside B1/B2 chips
	map(0x0100, 0x04ff).ram().share("nvram");   // Battery-backed HM6508, stored as individual bits
}

void recel_state::recel_io(address_map &map) // to be done
{
	//map(0x10, 0x1f).rw("b1", FUNC(ra17xx_device::io_r), FUNC(ra17xx_device::io_w));
	//map(0x20, 0x2f).rw("b2", FUNC(ra17xx_device::io_r), FUNC(ra17xx_device::io_w));
	//map(0x30, 0x3f).rw("b3", FUNC(r10696_device::io_r), FUNC(r10696_device::io_w));
	//map(0x50, 0x5f).rw("b5", FUNC(r10788_device::io_r), FUNC(r10788_device::io_w));
	//map(0x??, 0x?f).r(FUNC(recel_state::bic_r)); //  get module data via BIC C6
}

static INPUT_PORTS_START( recel )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Play/Test")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP20")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP30")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP40")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP50")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP60")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP70")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP11")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP21")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP31")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP41")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP51")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP61")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP71")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP12")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP22")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP32")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP42")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP52")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP62")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP72")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP13")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP23")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP33")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP43")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP53")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP63")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP73")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP14")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP24")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP34")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP44")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP54")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP64")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP74")

	PORT_START("X5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Reset")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Slam Tilt")
INPUT_PORTS_END


void recel_state::machine_start()
{
	genpin_class::machine_start();

	m_digit.resolve();

	save_item(NAME(m_strobe));
	save_item(NAME(m_nvram_addr));
	save_item(NAME(m_nvram_data));
	save_item(NAME(m_prom_addr));
}

void recel_state::machine_reset()
{
	genpin_class::machine_reset();

	m_strobe = 0;
	m_nvram_addr = 0xff;
	m_nvram_data = 0;
	m_nvram_prev_clk = 0;
	m_prom_addr = 0xff;
}


u8 recel_state::solenoids_r(offs_t offset)  // anything to be done?
{
	u8 data = 0;
	//LOG("%s: solenoid[%02x] -> %x\n", __FUNCTION__, offset, data);
	return data;
}

void recel_state::solenoids_w(offs_t offset, u8 data)  // to be tested
{
	//LOG("%s: solenoid #[%02X] gets data=%X\n", __FUNCTION__, offset, data);
	switch (offset)
	{
	case  0:
	case  1:
	case  2:
	case  3:
	case  4:
	case  5:
		// sound command
		break;
	case  6:  // knocker
		if (data)
			m_samples->start(0, 6);
		break;
	case  7:  // outhole
		if (data)
			m_samples->start(5, 5);
		break;
	default:
		break;
	}
}

u8 recel_state::switches_r(offs_t offset) // to be done
{
	u8 data = 0;
	if (offset > 7)
		for (u8 i = 0; i < 5; i++)
			if (BIT(m_strobe, i))
			{
				data |= BIT(m_io_keyboard[i]->read(), offset & 7);
				//LOG("%s: switches[bit %X of %X, using offset of %X] got %x\n", __FUNCTION__, i, m_strobe, offset&7, data);
			}
	return data ? 0 : 1;    // FIXME: inverted or normal?
}

void recel_state::switches_w(offs_t offset, u8 data) // to be done
{
	// outputs O-0 to O-4 are the 5 strobe lines
	if (offset < 5)
	{
		if (data)
			m_strobe |= (1<<offset);
		else
			m_strobe &= ~(1<<offset);
		//LOG("%s: strobe is now[%x], data was %x\n", __FUNCTION__, m_strobe, data);
	}
}

void recel_state::display_w(offs_t offset, u8 data) // to be tested
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7448
	data ^= 0xff;  // It was stored in the 10788 inverted
	u8 a = patterns[BIT(data, 0, 4)];
	u8 b = patterns[BIT(data, 4, 4)];
	// LOG("%s: offset:%d data:%02x a:%02x b:%02x\n", __FUNCTION__, offset, data, a, b);
	m_digit[offset] = a;
	m_digit[offset + 16] = b;
}

u8 recel_state::nvram_r(offs_t offset) // to be tested
{
	u8 data = 0x0f;
	switch (offset)
	{
		case 0: // group A
			data = m_nvram_data;  // the data bit goes into i/o 0
			LOG("%s: NVRAM READ @[%02x] -> %x\n", __FUNCTION__, m_nvram_addr, data);
			break;
		case 1: // group B
			// read from the printer into i/o 7
		case 2: // group C
			break;
	}
	return ~data & 1;   // polarity?
}

void recel_state::nvram_w(offs_t offset, u8 data) // to be tested
{
	m_nvram_addr &= 0x3ff;
	switch (offset)
	{
		case 0:
			// i/o 1 - data bit to nvram
			// i/o 2 - enable nvram
			// i/o 3 - r/w to nvram
			if ((data & 12)==12)
				m_nvram[m_nvram_addr] = BIT(data, 1);
			else
			if ((data & 12)==4)
				m_nvram_data = m_nvram[m_nvram_addr];
			break;
		case 1:
			// i/o 4 - clock 4040
			// i/o 5 - /reset 4040
			// i/o 6 - write to printer
			if (!BIT(data, 1))
				m_nvram_addr = 0;
			else
			if ((BIT(data, 0)==0) && m_nvram_prev_clk) // increment count on hi->lo
				m_nvram_addr++;
			m_nvram_prev_clk = BIT(data, 0);
			break;
		case 2:
			break;
	}
}

u8 recel_state::lamps_r(offs_t offset) // anything to be done?
{
	u8 data = 0x0f;
	//LOG("%s: offs=%d\n", __FUNCTION__, offset);
	return data;
}

void recel_state::lamps_w(offs_t offset, u8 data) // to be done
{
	//LOG("%s: offs=%d, data=%d\n", __FUNCTION__, offset, data);
}

u8 recel_state::bic_r(offs_t offset)  // to be tested
{
	// return nybble from personality module ROM
	u8 data = m_pm[m_prom_addr];
	LOG("%s: PROM READ @[%03x]:%02x\n", __FUNCTION__, m_prom_addr, data);
	// All 8 data lines are returned to the BIC. Perhaps it can return the high or low nybble on command?
	return ~data & 15;  // Todo: Inverted or normal?
}

void recel_state::bic_w(offs_t offset, u8 data)  // to be tested
{
	m_prom_addr = offset ^ 0xff;  // inverted or normal?
	LOG("%s: PROM addr:%02x\n", __FUNCTION__, m_prom_addr);
}


void recel_state::recel(machine_config & config)
{
	// Basic machine hardware
	PPS4_2(config, m_maincpu, XTAL(3'579'545));  // divided by 18 in the CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &recel_state::recel_map);
	m_maincpu->set_addrmap(AS_DATA, &recel_state::recel_data);
	m_maincpu->set_addrmap(AS_IO, &recel_state::recel_io);
	m_maincpu->dia_cb().set(FUNC(recel_state::switches_r));
	m_maincpu->do_cb().set(FUNC(recel_state::switches_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	ra17xx_device &u5(RA17XX(config, "b2", 0));
	u5.iord_cb().set(FUNC(recel_state::nvram_r));
	u5.iowr_cb().set(FUNC(recel_state::nvram_w));     // control NVRAM, printer
	u5.set_cpu_tag(m_maincpu);

	ra17xx_device &u4(RA17XX(config, "b1", 0));
	u4.iord_cb().set(FUNC(recel_state::lamps_r));
	u4.iowr_cb().set(FUNC(recel_state::lamps_w));    // control lamps
	u4.set_cpu_tag(m_maincpu);

	r10696_device &u3(R10696(config, "b3", 0));
	u3.iord_cb().set(FUNC(recel_state::solenoids_r));
	u3.iowr_cb().set(FUNC(recel_state::solenoids_w));   // to sound, solenoids, lamps

	r10788_device &u6(R10788(config, "b5", XTAL(3'579'545) / 18 ));  // divided in the circuit
	u6.update_cb().set(FUNC(recel_state::display_w));

	// Video
	config.set_default_layout(layout_recel);

	// Sound
	genpin_audio(config);
}


/* The BIOS is the same for all sets, but is labeled differently depending on the ROM type:
    -13: For machines with personality PROM 1702.
    -14: For machines with 2716 EPROM.
   In both cases, the second half of each chip is not used, with their A11 pins grounded
   (these chips have A1 to A11, there's no A0).
*/
#define RECEL_BIOS \
	ROM_REGION( 0x800, "maincpu", ROMREGION_ERASEFF ) \
	ROM_LOAD("a2361.b1", 0x0000, 0x0400, CRC(d0c4695d) SHA1(4846adb3f6c292626840ba5255ffc5e788a69301) ) \
		ROM_IGNORE( 0x400 ) \
	ROM_LOAD("a2362.b2", 0x0400, 0x0400, CRC(39a70611) SHA1(8545e168a5f256150bcff12d1e6d8efffd08c3cd) ) \
		ROM_IGNORE( 0x400 )

ROM_START( recel )
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
ROM_END

ROM_START(r_alaska)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("al.c5",         0x0000, 0x0100, CRC(905ef624) SHA1(ab0bb2e7262650b670524ce9f88bd1f14ffd749a) )
ROM_END

/* There's no Hot & Cold version for 2716, but some collectors made fake ROMs just repeating the content until filling
   0x800, so they can replace the original personality PROM 1702 with a more common 2716 EPROM. */
ROM_START(r_hotcold)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("hc.c5",         0x0000, 0x0100, CRC(f58d0c05) SHA1(54ecf9f67ce3a5264bfd9c063353705f9202d524) )
ROM_END

/* There's a fake set (CRC(ddf2beac) SHA1(2ce67e2679bf7d545434a90209c462ad53c50e01)) made by collectors
   for replacing the original personality PROM 1702 with a more common 2716 EPROM. */
ROM_START(r_screech)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("sc_1_1702.bin", 0x0000, 0x0100, CRC(c9185ef3) SHA1(3ace6cccc96375c5eab3d43f86f52bf52124334e) )
ROM_END

ROM_START(r_mrevil)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD( "me.c5",        0x0000, 0x0100, CRC(53ce24a0) SHA1(42d376e3e7a4e94a09db2f974af8d4869579d0f5) )
ROM_END

ROM_START(r_torneo)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("to.c5",         0x0000, 0x0100, CRC(06518bca) SHA1(6e8d4dba3cc5713208794aafc40cad6aca558aa6) )
ROM_END

ROM_START(r_crzyrace)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("cr.c5",         0x0000, 0x0800, CRC(60088804) SHA1(a73a7f8a0583a79588f9823a5e65ed28edad96a3) )
ROM_END

ROM_START(r_fairfght)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("fa.c5",         0x0000, 0x0100, CRC(5d3694da) SHA1(4d0a8033acb6ef2e2af107f76540fd19b4a39b12) )
ROM_END

ROM_START(r_pokrplus)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("po.c5",         0x0000, 0x0100, CRC(60a199a8) SHA1(045d61f56ea03a694722da810d465ab65d85cbfd) )
	//ROM_LOAD( "po2.c5",       0x0000, 0x0100, CRC(571ee27b) SHA1(482a3ba18eff05bce4cab073b1f13fc2f145bb2b) )
	//ROM_LOAD( "po3.c5",       0x0000, 0x0800, CRC(fadd715a) SHA1(6c5b6e8fcf77be2b0b7076dc1139760f7e4d5688) )
ROM_END

ROM_START(r_mrdoom)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("md.c5",         0x0000, 0x0100, CRC(ca679a69) SHA1(f08f0cfe646f08882473dcd5d23889fffe4a03c8) )
ROM_END

ROM_START(r_cavalier)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("ca.c5",         0x0000, 0x0100, CRC(dc2e865f) SHA1(3f15f90dafa9d5e42381605044b6c9b529afd3af) )
	//ROM_LOAD( "ca2.c5",       0x0000, 0x0100, CRC(dc2e865f) SHA1(3f15f90dafa9d5e42381605044b6c9b529afd3af) )
	//ROM_LOAD( "ca3.c5",       0x0000, 0x0800, CRC(fddd2373) SHA1(d0c79aefd2806066455c721a1361d11d6dab7d5f) )
ROM_END

ROM_START(r_swash)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("sw.c5",         0x0000, 0x0100, CRC(69326f5f) SHA1(f0bb4251f579ccf97c1cabb63254ba466ccd141e) )
ROM_END

ROM_START(r_quijote)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("qu.c5",         0x0000, 0x0100, CRC(1fd535d0) SHA1(a9c9a72881d195a0de751f10fa54fb181523a33f) )
	//ROM_LOAD( "qu2.c5",       0x0000, 0x0100, CRC(a88224ee) SHA1(cb85edcacc6001a9d865ef7e22711d6f62f1fdc1) )
	//ROM_LOAD( "qu3.c5",       0x0000, 0x0800, CRC(6eb5a08d) SHA1(3bfec2c0fdd1d8e1b03a5c189d2f37e1a52d065b) )
ROM_END

// PCB modified to use two 2716 instead of only one.
ROM_START(r_spcgame7)
	RECEL_BIOS

	ROM_REGION( 0x1000, "module", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "spcgme_system_iii_l.bin", 0x0001, 0x0800, CRC(8700559c) SHA1(cd61e16cf30420e976537ee0b8ba9e95e3577ddc) )
	ROM_LOAD16_BYTE( "spcgme_system_iii_h.bin", 0x0000, 0x0800, CRC(13eb1f52) SHA1(d0607b88314e86a37486ac8118d7fd0a17beb404) )
ROM_END

// Formula 1 was renamed as Antar for the Portuguese market, being distributed by Gorsam (without Recel logos).
ROM_START(r_antar)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("antar.bin",     0x0000, 0x0800, CRC(17882b53) SHA1(6a4f34fcc2fa88aee0c7843be00f75c9f3af03ba) ) // TMS2516
ROM_END

ROM_START(r_flipper)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("fl.c5",         0x0000, 0x0800, CRC(76ee0370) SHA1(f2a835a0b76f7258d5e65390c239f5456e30e87a) )
ROM_END

ROM_START(r_blackmag)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("bm_1065_1.bin", 0x0000, 0x0800, CRC(a917718c) SHA1(0b4fdf270560df902e95b34c25cca20e91f1071c) )
ROM_END

ROM_START(r_blackm4)
	RECEL_BIOS

	ROM_REGION( 0x0800, "module", ROMREGION_ERASEFF )
	ROM_LOAD("b4.c5",         0x0000, 0x0800, CRC(cd383f5b) SHA1(c38acaae46e5fd2660efbd0e2d35e295892e60a5) )
ROM_END

} // anonymous namespace

//   YEAR   NAME        PARENT  MACHINE  INPUT  CLASS        INIT        ROT   COMPANY      FULLNAME                  FLAGS

GAME(1977,  recel,      0,      recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Recel BIOS",             MACHINE_IS_BIOS_ROOT | MACHINE_NOT_WORKING)

GAME(1978,  r_alaska,   recel,  recel,   recel, recel_state, empty_init, ROT0, "Interflip", "Alaska",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  r_hotcold,  recel,  recel,   recel, recel_state, empty_init, ROT0, "Inder",     "Hot & Cold",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  r_screech,  recel,  recel,   recel, recel_state, empty_init, ROT0, "Inder",     "Screech",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  r_mrevil,   recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Mr. Evil",               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  r_torneo,   recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Torneo",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  r_crzyrace, recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Crazy Race",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  r_fairfght, recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Fair Fight",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  r_pokrplus, recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Poker Plus",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979,  r_mrdoom,   recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Mr. Doom",               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979,  r_cavalier, recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Cavalier",               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979,  r_swash,    recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "SwashBuckler",           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979,  r_quijote,  recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Don Quijote",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979,  r_spcgame7, recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Space Game (Bingo 6+1)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  r_antar,    recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Antar (Recel)",          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  r_flipper,  recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "The Flipper Game",       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  r_blackmag, recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Black Magic",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1980,  r_blackm4,  recel,  recel,   recel, recel_state, empty_init, ROT0, "Recel",     "Black Magic 4",          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
