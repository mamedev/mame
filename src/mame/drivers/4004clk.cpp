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
#include "sound/volt_reg.h"
#include "speaker.h"

#include "4004clk.lh"


class nixieclock_state : public driver_device
{
public:
	nixieclock_state(const machine_config &mconfig, device_type type, const char *tag) : driver_device(mconfig, type, tag) { }

	DECLARE_WRITE8_MEMBER( nixie_w );
	DECLARE_WRITE8_MEMBER( neon_w );

	void _4004clk(machine_config &config);
	void _4004clk_mem(address_map &map);
	void _4004clk_mp(address_map &map);
	void _4004clk_rom(address_map &map);
	void _4004clk_rp(address_map &map);
	void _4004clk_stat(address_map &map);
protected:
	virtual void machine_start() override;

private:
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

	void output_set_nixie_value(int index, int value)
	{
		output().set_indexed_value("nixie", index, value);
	}

	void output_set_neon_value(int index, int value)
	{
		output().set_indexed_value("neon", index, value);
	}

	uint16_t m_nixie[16];
};

WRITE8_MEMBER(nixieclock_state::nixie_w)
{
	m_nixie[offset >> 4] = data;
	output_set_nixie_value(5, nixie_to_num(((m_nixie[2] & 3)<<8) | (m_nixie[1] << 4) | m_nixie[0]));
	output_set_nixie_value(4, nixie_to_num((m_nixie[4] << 6) | (m_nixie[3] << 2) | (m_nixie[2] >>2)));
	output_set_nixie_value(3, nixie_to_num(((m_nixie[7] & 3)<<8) | (m_nixie[6] << 4) | m_nixie[5]));
	output_set_nixie_value(2, nixie_to_num((m_nixie[9] << 6) | (m_nixie[8] << 2) | (m_nixie[7] >>2)));
	output_set_nixie_value(1, nixie_to_num(((m_nixie[12] & 3)<<8) | (m_nixie[11] << 4) | m_nixie[10]));
	output_set_nixie_value(0, nixie_to_num((m_nixie[14] << 6) | (m_nixie[13] << 2) | (m_nixie[12] >>2)));
}

WRITE8_MEMBER(nixieclock_state::neon_w)
{
	output_set_neon_value(0, BIT(data,3));
	output_set_neon_value(1, BIT(data,2));
	output_set_neon_value(2, BIT(data,1));
	output_set_neon_value(3, BIT(data,0));
}

ADDRESS_MAP_START(nixieclock_state::_4004clk_rom)
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

ADDRESS_MAP_START(nixieclock_state::_4004clk_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007f) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(nixieclock_state::_4004clk_stat)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x001f) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(nixieclock_state::_4004clk_rp)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_MIRROR(0x0700) AM_READ_PORT("INPUT")
	AM_RANGE(0x0000, 0x00ef) AM_MIRROR(0x0700) AM_WRITE(nixie_w)
	AM_RANGE(0x00f0, 0x00ff) AM_MIRROR(0x0700) AM_WRITE(neon_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(nixieclock_state::_4004clk_mp)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("dac", dac_bit_interface, write)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( 4004clk )
	PORT_START("INPUT")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Browse") PORT_CODE(KEYCODE_2)

	PORT_CONFNAME( 0x04, 0x00, "Tick-Tock")
	PORT_CONFSETTING( 0x00, "ON" )
	PORT_CONFSETTING( 0x04, "OFF" )
	PORT_CONFNAME( 0x08, 0x08, "50/60 Hz")
	PORT_CONFSETTING( 0x00, "50 Hz" )
	PORT_CONFSETTING( 0x08, "60 Hz" )
INPUT_PORTS_END

void nixieclock_state::machine_start()
{
	/* register for state saving */
	save_pointer(NAME(m_nixie), 6);
}

MACHINE_CONFIG_START(nixieclock_state::_4004clk)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I4004, XTAL(5'000'000) / 8)
	MCFG_I4004_ROM_MAP(_4004clk_rom)
	MCFG_I4004_RAM_MEMORY_MAP(_4004clk_mem)
	MCFG_I4004_ROM_PORTS_MAP(_4004clk_rp)
	MCFG_I4004_RAM_STATUS_MAP(_4004clk_stat)
	MCFG_I4004_RAM_PORTS_MAP(_4004clk_mp)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_4004clk)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac", DAC_1BIT, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.25)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT)

	MCFG_CLOCK_ADD("clk", 60)
	MCFG_CLOCK_SIGNAL_HANDLER(INPUTLINE("maincpu", I4004_TEST_LINE))
MACHINE_CONFIG_END

/* ROM definition */
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
	ROM_LOAD( "clock.u40", 0x0A00, 0x0100, CRC(1c2e94b5) SHA1(89e6c70b936fd9882a229de671dcada7c39b9e8e))
	ROM_LOAD( "clock.u41", 0x0B00, 0x0100, CRC(48b4510d) SHA1(17c5eedc36b469bfae23e204c614ccc01bd4df02))
	ROM_LOAD( "clock.u42", 0x0C00, 0x0100, CRC(4b768675) SHA1(8862b9911bd5907e679539c1e98921f0686b8a76))
	ROM_LOAD( "clock.u43", 0x0D00, 0x0100, CRC(df8db80f) SHA1(34ef8e9ae9fd4e88659e1e14759a2baf1cf589a5))
	ROM_LOAD( "clock.u44", 0x0E00, 0x0100, CRC(23037c71) SHA1(87702bdf5985fa58d4cabcc0d4530e229bfebcbb))
	ROM_LOAD( "clock.u45", 0x0F00, 0x0100, CRC(a8d419ef) SHA1(86742a5ad410c027e9cf9a95e2006dc1128715e5))
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    STATE             INIT  COMPANY             FULLNAME            FLAGS
SYST( 2008, 4004clk,  0,      0,       _4004clk,  4004clk, nixieclock_state, 0,    "John L. Weinrich", "4004 Nixie Clock", MACHINE_SUPPORTS_SAVE )
