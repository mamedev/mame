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

namespace {

class stellafr_state : public driver_device
{
public:
	stellafr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_duart(*this, "duart")
	{ }

	void stellafr(machine_config &config);

private:
	void write_8000c1(uint8_t data);
	uint8_t read_800101();
	void write_800101(uint8_t data);
	void duart_output_w(uint8_t data);
	void ay8910_portb_w(uint8_t data);

	void mem_map(address_map &map) ATTR_COLD;
	void fc7_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;
};


void stellafr_state::write_8000c1(uint8_t data)
{
}

uint8_t stellafr_state::read_800101()
{
	return 0xff;
}

void stellafr_state::write_800101(uint8_t data)
{
}

void stellafr_state::duart_output_w(uint8_t data)
{
}

void stellafr_state::ay8910_portb_w(uint8_t data)
{
}



void stellafr_state::mem_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x8000c1, 0x8000c1).w(FUNC(stellafr_state::write_8000c1));
	map(0x800101, 0x800101).rw(FUNC(stellafr_state::read_800101), FUNC(stellafr_state::write_800101));
	map(0x800141, 0x800141).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0x800143, 0x800143).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x800180, 0x80019f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0xff0000, 0xffffff).ram();
}

void stellafr_state::fc7_map(address_map &map)
{
	map(0xfffff5, 0xfffff5).r(m_duart, FUNC(mc68681_device::get_irq_vector));
}


static INPUT_PORTS_START( stellafr )
	PORT_START("INPUTS")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END


void stellafr_state::stellafr(machine_config &config)
{
	M68000(config, m_maincpu, 10000000 ); //?
	m_maincpu->set_addrmap(AS_PROGRAM, &stellafr_state::mem_map);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &stellafr_state::fc7_map);

	MC68681(config, m_duart, 3686400);
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2); // ?
	m_duart->outport_cb().set(FUNC(stellafr_state::duart_output_w));

	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", 1000000));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.85);
	aysnd.port_a_read_callback().set_ioport("INPUTS");
	aysnd.port_b_write_callback().set(FUNC(stellafr_state::ay8910_portb_w));
}



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

} // anonymous namespace


GAME(199?,  st_ohla,   0,  stellafr,  stellafr, stellafr_state, empty_init, ROT0, "Stella", "Oh La La (Stella)",    MACHINE_IS_SKELETON_MECHANICAL )
GAME(199?,  st_vulkn,  0,  stellafr,  stellafr, stellafr_state, empty_init, ROT0, "Stella", "Vulkan (Stella)",      MACHINE_IS_SKELETON_MECHANICAL )
