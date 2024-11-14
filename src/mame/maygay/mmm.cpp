// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Early Maygay HW
 I believe this is 'Triple M' or 'MMM' hardware

 Z80 based Fruit Machine
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "sound/ay8910.h"
#include "speaker.h"


namespace {

class mmm_state : public driver_device
{
public:
	mmm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ctc(*this, "ctc")
		, m_inputs(*this, "IN%u", 0)
	{ }

	void mmm(machine_config &config);

private:
	void strobe_w(u8 data);
	u8 inputs_r();
	void ay_porta_w(u8 data);

	u8 ctc_r(offs_t offset);
	void ctc_w(offs_t offset, u8 data);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_ioport_array<8> m_inputs;
	u8 m_strobe = 0;
};


void mmm_state::strobe_w(u8 data)
{
	m_strobe = data;
}

u8 mmm_state::inputs_r()
{
	u8 result = 0xff;
	for (int i = 0; i < 8; i++)
		if (BIT(m_strobe, i))
			result &= m_inputs[i]->read();
	return result;
}

void mmm_state::ay_porta_w(u8 data)
{
	logerror("Writing %02X to AY-3-8910 port A\n", data);
}

u8 mmm_state::ctc_r(offs_t offset)
{
	return m_ctc->read(offset >> 4);
}

void mmm_state::ctc_w(offs_t offset, u8 data)
{
	m_ctc->write(offset >> 4, data);
}

void mmm_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x40ff).ram();
}

void mmm_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(mmm_state::strobe_w));
	map(0x03, 0x03).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x04, 0x04).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x05, 0x05).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x06, 0x06).select(0x30).rw(FUNC(mmm_state::ctc_r), FUNC(mmm_state::ctc_w));
	map(0x07, 0x07).r(FUNC(mmm_state::inputs_r));
}


static INPUT_PORTS_START( mmm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const z80_daisy_config mmm_daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};


void mmm_state::mmm(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2000000);         /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &mmm_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mmm_state::io_map);
	m_maincpu->set_daisy_config(mmm_daisy_chain);

	Z80CTC(config, m_ctc, 2000000);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device& ay(AY8910(config, "aysnd", 1000000));
	ay.port_a_write_callback().set(FUNC(mmm_state::ay_porta_w));
	ay.add_route(ALL_OUTPUTS, "mono", 0.30);
}


ROM_START( mmm_ldip )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ld1.bin", 0x0000, 0x1000, CRC(5a3c2402) SHA1(4972d309e6aabef4f9277ea851e45981d0cb3dbb) )
	ROM_LOAD( "ld2.bin", 0x1000, 0x1000, CRC(ff82643b) SHA1(0e47cdc9c0eb6f05a420d2ffeb2ebf22acbda15b) )
	ROM_LOAD( "ld3.bin", 0x2000, 0x1000, CRC(9e7158ae) SHA1(7f3b8730add127ed0608365875be3042fb2e3e7a) )
	ROM_LOAD( "ld4.bin", 0x3000, 0x1000, CRC(970b749f) SHA1(fe6da7abc699db69c0761304f588b5bed899c674) )
ROM_END

} // anonymous namespace


GAME( 198?,  mmm_ldip,  0,  mmm,  mmm, mmm_state, empty_init, ROT0,  "Maygay",    "Lucky Dip (Maygay)",    MACHINE_IS_SKELETON_MECHANICAL)
