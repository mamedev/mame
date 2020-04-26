// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

    8085-based slots by Recreativos Franco

    IC17 = NEC uPD8155HC-2 Static RAM I/O Timer
    IC21 = NEC uPD8279C-2 Programmable Keyboard Display Interface
    IC24 = Toshiba TMP8255AP-5 Programmable Peripheral Interface
    IC32 = 27C128
    IC34 = Toshiba TC5517APL 2.048 Word X 8 Bit CMOS Static RAM
    IC31 = uPD8085AC CPU (Xtal 5.0688 MHz)
    IC33 = AMD P8212 8-Bit IO Port
    IC40 = Signetics SCN8035A MCU (Xtal 6.000 MHz)
    IC44 = 27128
    IC39 = AY-3-8910A
    IC38 = AY-3-8910A

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8155.h"
#include "machine/i8212.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "speaker.h"

class rfslots8085_state : public driver_device
{
public:
	rfslots8085_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
	{
	}

	void unkrfslt(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void main_io_map(address_map &map);
	void main_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8035_device> m_audiocpu;
};

void rfslots8085_state::machine_start()
{
}

void rfslots8085_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x87ff).ram();
}

void rfslots8085_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
}

void rfslots8085_state::sound_map(address_map &map)
{
	map(0x000, 0xfff).rom(); // external EPROM
}

void rfslots8085_state::sound_io_map(address_map &map)
{
}

static INPUT_PORTS_START(unkrfslt)
	PORT_START("DSW") // 1 x 6-dips bank
	PORT_BIT(0x80, 0x80, IPT_UNKNOWN)
	PORT_BIT(0x40, 0x40, IPT_UNKNOWN)
	PORT_BIT(0x20, 0x20, IPT_UNKNOWN)
	PORT_BIT(0x10, 0x10, IPT_UNKNOWN)
	PORT_BIT(0x08, 0x08, IPT_UNKNOWN)
	PORT_BIT(0x04, 0x04, IPT_UNKNOWN)
	PORT_BIT(0x02, 0x02, IPT_UNKNOWN)
	PORT_BIT(0x01, 0x01, IPT_UNKNOWN)
INPUT_PORTS_END

void rfslots8085_state::unkrfslt(machine_config &config)
{
	I8085A(config, m_maincpu, 5.0688_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &rfslots8085_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &rfslots8085_state::main_io_map);

	I8035(config, m_audiocpu, 6_MHz_XTAL / 2); // divider unknown
	m_audiocpu->set_addrmap(AS_PROGRAM, &rfslots8085_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &rfslots8085_state::sound_io_map);

	I8155(config, "i8155", 0);

	I8255(config, "i8255");

	I8212(config, "i8212");

	I8279(config, "i8279", 0);

	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay0", 6_MHz_XTAL / 6).add_route(ALL_OUTPUTS, "mono", 0.50); // divider unknown

	AY8910(config, "ay1", 6_MHz_XTAL / 6).add_route(ALL_OUTPUTS, "mono", 0.50); // divider unknown
}

// May be "Limon y Baby 100"
ROM_START(unkrfslt)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("m1-31_b_1704.ic32", 0x0000, 0x4000, CRC(a74a85b7) SHA1(f562495a6b97f34165cc9fd5c750664701cac21f))

	ROM_REGION(0x1000, "audiocpu", 0)
	ROM_LOAD( "8a.ic44", 0x0000, 0x1000, CRC(51b564b6) SHA1(8992a5cb4dff8c6b38b77a7e0199a71f2969b496) )
	ROM_IGNORE(                  0x3000 ) // 0xff filled and it's outside of the 8035's global address mask (fff)
ROM_END

// Date "25-05-87" engraved on the PCB
GAME( 1987?, unkrfslt, 0, unkrfslt, unkrfslt, rfslots8085_state, empty_init, ROT0, "Recreativos Franco", "unknown Recreativos Franco slot machine", MACHINE_IS_SKELETON_MECHANICAL )
