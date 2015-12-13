// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        4004 Nixie Clock

        03/08/2009 Initial driver

****************************************************************************/

#include "emu.h"
#include "cpu/i4004/i4004.h"
#include "sound/dac.h"
#include "4004clk.lh"

class nixieclock_state : public driver_device
{
public:
	nixieclock_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_input(*this, "INPUT")
	{ }

	required_device<i4004_cpu_device> m_maincpu;
	required_device<dac_device> m_dac;
	required_ioport m_input;
	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( nixie_w );
	DECLARE_WRITE8_MEMBER( neon_w );
	DECLARE_WRITE8_MEMBER( relays_w );
	UINT16 m_nixie[16];
	UINT8 m_timer;
	virtual void machine_start() override;
	TIMER_DEVICE_CALLBACK_MEMBER(timer_callback);
	UINT8 nixie_to_num(UINT16 val);
	inline void output_set_nixie_value(int index, int value);
	inline void output_set_neon_value(int index, int value);
};

READ8_MEMBER(nixieclock_state::data_r)
{
	return m_input->read() & 0x0f;
}

UINT8 nixieclock_state::nixie_to_num(UINT16 val)
{
	if (BIT(val,0)) return 0;
	if (BIT(val,1)) return 1;
	if (BIT(val,2)) return 2;
	if (BIT(val,3)) return 3;
	if (BIT(val,4)) return 4;
	if (BIT(val,5)) return 5;
	if (BIT(val,6)) return 6;
	if (BIT(val,7)) return 7;
	if (BIT(val,8)) return 8;
	if (BIT(val,9)) return 9;
	return 10;
}

inline void nixieclock_state::output_set_nixie_value(int index, int value)
{
	output_set_indexed_value("nixie", index, value);
}

inline void nixieclock_state::output_set_neon_value(int index, int value)
{
	output_set_indexed_value("neon", index, value);
}

WRITE8_MEMBER(nixieclock_state::nixie_w)
{
	m_nixie[offset] = data;
	output_set_nixie_value(5,nixie_to_num(((m_nixie[2] & 3)<<8) | (m_nixie[1] << 4) | m_nixie[0]));
	output_set_nixie_value(4,nixie_to_num((m_nixie[4] << 6) | (m_nixie[3] << 2) | (m_nixie[2] >>2)));
	output_set_nixie_value(3,nixie_to_num(((m_nixie[7] & 3)<<8) | (m_nixie[6] << 4) | m_nixie[5]));
	output_set_nixie_value(2,nixie_to_num((m_nixie[9] << 6) | (m_nixie[8] << 2) | (m_nixie[7] >>2)));
	output_set_nixie_value(1,nixie_to_num(((m_nixie[12] & 3)<<8) | (m_nixie[11] << 4) | m_nixie[10]));
	output_set_nixie_value(0,nixie_to_num((m_nixie[14] << 6) | (m_nixie[13] << 2) | (m_nixie[12] >>2)));
}

WRITE8_MEMBER(nixieclock_state::neon_w)
{
	output_set_neon_value(0,BIT(data,3));
	output_set_neon_value(1,BIT(data,2));
	output_set_neon_value(2,BIT(data,1));
	output_set_neon_value(3,BIT(data,0));
}

WRITE8_MEMBER(nixieclock_state::relays_w)
{
	m_dac->write_unsigned8((data & 1) ? 0x80 : 0x40); //tick - tock
}

static ADDRESS_MAP_START(4004clk_rom, AS_PROGRAM, 8, nixieclock_state)
	AM_RANGE(0x0000, 0x0FFF) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(4004clk_mem, AS_DATA, 8, nixieclock_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007F) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( 4004clk_io, AS_IO, 8, nixieclock_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x0e) AM_WRITE(nixie_w)
	AM_RANGE(0x00, 0x00) AM_READ(data_r)
	AM_RANGE(0x0f, 0x0f) AM_WRITE(neon_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(relays_w)
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

/*
        16ms Int generator
          __  __
        _| |_|
        0 1 0 1

*/

TIMER_DEVICE_CALLBACK_MEMBER(nixieclock_state::timer_callback)
{
	m_maincpu->set_test(m_timer);
	m_timer^=1;
}

void nixieclock_state::machine_start()
{
	m_timer = 0;

	/* register for state saving */
	save_item(NAME(m_timer));
	save_pointer(NAME(m_nixie), 6);
}

static MACHINE_CONFIG_START( 4004clk, nixieclock_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I4004, XTAL_5MHz / 8)
	MCFG_CPU_PROGRAM_MAP(4004clk_rom)
	MCFG_CPU_DATA_MAP(4004clk_mem)
	MCFG_CPU_IO_MAP(4004clk_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_4004clk)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("4004clk_timer", nixieclock_state, timer_callback, attotime::from_hz(120))
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

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    INIT              COMPANY             FULLNAME            FLAGS */
SYST( 2008, 4004clk,  0,      0,       4004clk,   4004clk, driver_device, 0, "John L. Weinrich", "4004 Nixie Clock", MACHINE_SUPPORTS_SAVE )
