// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Stella
German Fruit Machines / Gambling Machines

Possibly related to ADP hardware? The HD63484 video board is definitely absent here.


*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "sound/ay8910.h"
#include "speaker.h"

class stellafr_state : public driver_device
{
public:
	stellafr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_duart(*this, "duart")
	{ }

	IRQ_CALLBACK_MEMBER(irq_ack);
	DECLARE_WRITE8_MEMBER(write_8000c1);
	DECLARE_READ8_MEMBER(read_800101);
	DECLARE_WRITE8_MEMBER(write_800101);
	DECLARE_WRITE8_MEMBER(duart_output_w);
	DECLARE_WRITE8_MEMBER(ay8910_portb_w);

	void stellafr(machine_config &config);
	void stellafr_map(address_map &map);
protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;
};


IRQ_CALLBACK_MEMBER(stellafr_state::irq_ack)
{
	return m_duart->get_irq_vector();
}

WRITE8_MEMBER(stellafr_state::write_8000c1)
{
}

READ8_MEMBER(stellafr_state::read_800101)
{
	return 0xff;
}

WRITE8_MEMBER(stellafr_state::write_800101)
{
}

WRITE8_MEMBER(stellafr_state::duart_output_w)
{
}

WRITE8_MEMBER(stellafr_state::ay8910_portb_w)
{
}



ADDRESS_MAP_START(stellafr_state::stellafr_map)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x8000c0, 0x8000c1) AM_WRITE8(write_8000c1, 0x00ff)
	AM_RANGE(0x800100, 0x800101) AM_READWRITE8(read_800101, write_800101, 0x00ff)
	AM_RANGE(0x800140, 0x800141) AM_DEVREADWRITE8("aysnd", ay8910_device, data_r, address_w, 0x00ff)
	AM_RANGE(0x800142, 0x800143) AM_DEVWRITE8("aysnd", ay8910_device, data_w, 0x00ff)
	AM_RANGE(0x800180, 0x80019f) AM_DEVREADWRITE8("duart", mc68681_device, read, write, 0x00ff)
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( stellafr )
	PORT_START("INPUTS")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END


MACHINE_CONFIG_START(stellafr_state::stellafr)
	MCFG_CPU_ADD("maincpu", M68000, 10000000 ) //?
	MCFG_CPU_PROGRAM_MAP(stellafr_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(stellafr_state, irq_ack)

	MCFG_DEVICE_ADD("duart", MC68681, 3686400)
	MCFG_MC68681_IRQ_CALLBACK(INPUTLINE("maincpu", M68K_IRQ_2)) // ?
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(stellafr_state, duart_output_w))

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 1000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("INPUTS"))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(stellafr_state, ay8910_portb_w))
MACHINE_CONFIG_END



ROM_START( st_ohla )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "oh_la_la_f1_1.bin", 0x00000, 0x010000, CRC(94583885) SHA1(5083d65da0347a37ffbb537f94d3b247241f1e8c) )
	ROM_LOAD16_BYTE( "oh_la_la_f1_2.bin", 0x00001, 0x010000, CRC(8ac647cd) SHA1(858f67d6121dde28477a5df8569e7ae92db6299e) )
ROM_END

ROM_START( st_vulkn )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "vulkan_f1_1.bin", 0x00000, 0x010000, CRC(06109bd5) SHA1(78f6b0cb3ae5873350fd50af8990fa38454c1183) )
	ROM_LOAD16_BYTE( "vulkan_f1_2.bin", 0x00001, 0x010000, CRC(951baf42) SHA1(1346043155ba85926b2bf9eef8136b377953abe1) )
ROM_END


GAME(199?,  st_ohla,   0,  stellafr,  stellafr, stellafr_state,  0,  ROT0,  "Stella",    "Oh La La (Stella)",    MACHINE_IS_SKELETON_MECHANICAL )
GAME(199?,  st_vulkn,  0,  stellafr,  stellafr, stellafr_state,  0,  ROT0,  "Stella",    "Vulkan (Stella)",      MACHINE_IS_SKELETON_MECHANICAL )
