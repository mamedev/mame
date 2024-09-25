// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        4004 Nixie Clock

        03/08/2009 Initial driver

****************************************************************************/

#include "emu.h"

#include "cpu/mcs40/mcs40.h"
#include "machine/clock.h"
#include "sound/dac.h"

#include "speaker.h"

#include "4004clk.lh"


namespace {

class nixieclock_state : public driver_device
{
public:
	nixieclock_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_test_line(*this, "test_line")
		, m_nixie_out(*this, "nixie%u", 0U)
		, m_neon_out(*this, "neon%u", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(switch_hz) { m_test_line->set_clock(newval ? 60 : 50); }
	void _4004clk(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<i4004_cpu_device> m_maincpu;
	required_device<clock_device> m_test_line;
	output_finder<6> m_nixie_out;
	output_finder<4> m_neon_out;

	void nixie_w(offs_t offset, uint8_t data);
	void neon_w(uint8_t data);

	void _4004clk_mem(address_map &map) ATTR_COLD;
	void _4004clk_mp(address_map &map) ATTR_COLD;
	void _4004clk_rom(address_map &map) ATTR_COLD;
	void _4004clk_rp(address_map &map) ATTR_COLD;
	void _4004clk_stat(address_map &map) ATTR_COLD;

	static constexpr uint8_t nixie_to_num(uint16_t val)
	{
		return
				(BIT(val, 0)) ? 0 :
				(BIT(val, 1)) ? 1 :
				(BIT(val, 2)) ? 2 :
				(BIT(val, 3)) ? 3 :
				(BIT(val, 4)) ? 4 :
				(BIT(val, 5)) ? 5 :
				(BIT(val, 6)) ? 6 :
				(BIT(val, 7)) ? 7 :
				(BIT(val, 8)) ? 8 :
				(BIT(val, 9)) ? 9 :
				10;
	}

	uint16_t m_nixie[16];
};

void nixieclock_state::machine_start()
{
	m_nixie_out.resolve();
	m_neon_out.resolve();

	std::fill(std::begin(m_nixie), std::end(m_nixie), 0);
	save_item(NAME(m_nixie));
}


// I/O handlers

void nixieclock_state::nixie_w(offs_t offset, uint8_t data)
{
	m_nixie[offset >> 4] = data;
	m_nixie_out[5] = nixie_to_num(((m_nixie[2] & 3) << 8) | (m_nixie[1] << 4) | m_nixie[0]);
	m_nixie_out[4] = nixie_to_num((m_nixie[4] << 6) | (m_nixie[3] << 2) | (m_nixie[2] >> 2));
	m_nixie_out[3] = nixie_to_num(((m_nixie[7] & 3) << 8) | (m_nixie[6] << 4) | m_nixie[5]);
	m_nixie_out[2] = nixie_to_num((m_nixie[9] << 6) | (m_nixie[8] << 2) | (m_nixie[7] >> 2));
	m_nixie_out[1] = nixie_to_num(((m_nixie[12] & 3) << 8) | (m_nixie[11] << 4) | m_nixie[10]);
	m_nixie_out[0] = nixie_to_num((m_nixie[14] << 6) | (m_nixie[13] << 2) | (m_nixie[12] >> 2));
}

void nixieclock_state::neon_w(uint8_t data)
{
	m_neon_out[0] = BIT(data,3);
	m_neon_out[1] = BIT(data,2);
	m_neon_out[2] = BIT(data,1);
	m_neon_out[3] = BIT(data,0);
}


// Address maps

void nixieclock_state::_4004clk_rom(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("maincpu", 0);
}

void nixieclock_state::_4004clk_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).ram();
}

void nixieclock_state::_4004clk_stat(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x001f).ram();
}

void nixieclock_state::_4004clk_rp(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x000f).mirror(0x0700).portr("INPUT");
	map(0x0000, 0x00ef).mirror(0x0700).w(FUNC(nixieclock_state::nixie_w));
	map(0x00f0, 0x00ff).mirror(0x0700).w(FUNC(nixieclock_state::neon_w));
}

void nixieclock_state::_4004clk_mp(address_map &map)
{
	map(0x00, 0x00).w("dac", FUNC(dac_bit_interface::data_w));
}


// Input ports

static INPUT_PORTS_START( 4004clk )
	PORT_START("INPUT")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Browse") PORT_CODE(KEYCODE_2)

	PORT_CONFNAME( 0x04, 0x00, "Tick-Tock")
	PORT_CONFSETTING( 0x00, "ON" )
	PORT_CONFSETTING( 0x04, "OFF" )
	PORT_CONFNAME( 0x08, 0x08, "50/60 Hz") PORT_CHANGED_MEMBER(DEVICE_SELF, nixieclock_state, switch_hz, 0)
	PORT_CONFSETTING( 0x00, "50 Hz" )
	PORT_CONFSETTING( 0x08, "60 Hz" )
INPUT_PORTS_END


// Machine config

void nixieclock_state::_4004clk(machine_config &config)
{
	// basic machine hardware
	I4004(config, m_maincpu, 5_MHz_XTAL / 8);
	m_maincpu->set_rom_map(&nixieclock_state::_4004clk_rom);
	m_maincpu->set_ram_memory_map(&nixieclock_state::_4004clk_mem);
	m_maincpu->set_rom_ports_map(&nixieclock_state::_4004clk_rp);
	m_maincpu->set_ram_status_map(&nixieclock_state::_4004clk_stat);
	m_maincpu->set_ram_ports_map(&nixieclock_state::_4004clk_mp);

	CLOCK(config, m_test_line, 60).signal_handler().set_inputline(m_maincpu, I4004_TEST_LINE);

	// video hardware
	config.set_default_layout(layout_4004clk);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
}


// ROM definition

ROM_START( 4004clk )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "clock.u30", 0x0000, 0x0100, CRC(3d8608a5) SHA1(47fa0a91779e1afc34592f91068f8af2e74593d4))
	ROM_LOAD( "clock.u31", 0x0100, 0x0100, CRC(c83af4bc) SHA1(c8bd7ff1322e5ad280364ed87fc9e2ed38120c21))
	ROM_LOAD( "clock.u32", 0x0200, 0x0100, CRC(72442ae7) SHA1(dc25269cf9db7a9e70a86fededeb01efecf80fe7))
	ROM_LOAD( "clock.u33", 0x0300, 0x0100, CRC(74789383) SHA1(ca5ef2c42dae1761599ead56498fb0bfa0db80a5))
	ROM_LOAD( "clock.u34", 0x0400, 0x0100, CRC(d817cc54) SHA1(a54e744465dde20d37ba54bf2bdb1c4708235f9a))
	ROM_LOAD( "clock.u35", 0x0500, 0x0100, CRC(ece36d21) SHA1(c8cf7e08e90463e910ed2d21d8e562e73d7e0b08))
	ROM_LOAD( "clock.u36", 0x0600, 0x0100, CRC(65aa3cb1) SHA1(9b134bd46747a1bccc004c347b3162e9dc846426))
	ROM_LOAD( "clock.u37", 0x0700, 0x0100, CRC(4c2a2632) SHA1(5f75c2d67571ffcfb98f37944f7f4bc7f531c109))
	ROM_LOAD( "clock.u38", 0x0800, 0x0100, CRC(133da0d6) SHA1(08863a287471c0e77f27cea087cb4a3b372d49c1))
	ROM_LOAD( "clock.u39", 0x0900, 0x0100, CRC(0628593c) SHA1(34249753056cd425e0d48c188c830d64464006c9))
	ROM_LOAD( "clock.u40", 0x0a00, 0x0100, CRC(1c2e94b5) SHA1(89e6c70b936fd9882a229de671dcada7c39b9e8e))
	ROM_LOAD( "clock.u41", 0x0b00, 0x0100, CRC(48b4510d) SHA1(17c5eedc36b469bfae23e204c614ccc01bd4df02))
	ROM_LOAD( "clock.u42", 0x0c00, 0x0100, CRC(4b768675) SHA1(8862b9911bd5907e679539c1e98921f0686b8a76))
	ROM_LOAD( "clock.u43", 0x0d00, 0x0100, CRC(df8db80f) SHA1(34ef8e9ae9fd4e88659e1e14759a2baf1cf589a5))
	ROM_LOAD( "clock.u44", 0x0e00, 0x0100, CRC(23037c71) SHA1(87702bdf5985fa58d4cabcc0d4530e229bfebcbb))
	ROM_LOAD( "clock.u45", 0x0f00, 0x0100, CRC(a8d419ef) SHA1(86742a5ad410c027e9cf9a95e2006dc1128715e5))
ROM_END

} // anonymous namespace


// Driver

//    YEAR  NAME     PARENT  COMPAT  MACHINE   INPUT    CLASS             INIT        COMPANY             FULLNAME            FLAGS
SYST( 2008, 4004clk, 0,      0,      _4004clk, 4004clk, nixieclock_state, empty_init, "John L. Weinrich", "4004 Nixie Clock", MACHINE_SUPPORTS_SAVE )
