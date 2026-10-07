// license:GPL-2.0+
// copyright-holders:
/*************************************************************************
mantx
x3 8255ac-2
x1 z86e21f1
3567 - ym2413
6116p-3
xtal:8.000 near of mcu
x3.579545 near of ym2413

mantxa (same pcb of mantxa except the z86e21f1 mcu)
at front back pcb
Nec D8255AC-2
Unreadable mark

at back of pcb

5 buttons and 8 dip switch
at box
BMC8628?
ACC91018-
Bui? et mic?roCircuit in?
18cv8 74hc00 (
**************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "cpu/z8/z8.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/i8255.h"
#include "sound/ymopl.h"
#include "speaker.h"

namespace {

class mantx_state : public driver_device
{
public:
	mantx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_leds(*this, "led%u", 0U)

	{ }

	void mantx(machine_config &config);
	void mantxa(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void ppi1_port_a_w(uint8_t data);
	void ppi1_port_b_w(uint8_t data);
	void ppi1_port_c_w(uint8_t data);
	void ppi2_port_a_w(uint8_t data);
	void ppi2_port_b_w(uint8_t data);
	void ppi2_port_c_w(uint8_t data);
	void ppi3_port_a_w(uint8_t data);
	void ppi3_port_b_w(uint8_t data);
	void ppi3_port_c_w(uint8_t data);

	void io_map(address_map &map);
	
	void program_map(address_map &map);

	void mcu_map(address_map &map);
	output_finder<72> m_leds;

};

static INPUT_PORTS_START( mantx )
	
INPUT_PORTS_END


void mantx_state::ppi1_port_a_w(uint8_t data)
{
	
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i] = BIT(~data, i);
}

void mantx_state::ppi1_port_b_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 8] = BIT(~data, i);
}

void mantx_state::ppi1_port_c_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 16] = BIT(~data, i);
}

void mantx_state::ppi2_port_a_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 24] = BIT(~data, i);
}

void mantx_state::ppi2_port_b_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 32] = BIT(~data, i);
}
void mantx_state::ppi2_port_c_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 40] = BIT(~data, i);
}

void mantx_state::ppi3_port_a_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 48] = BIT(~data, i);
}

void mantx_state::ppi3_port_b_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 56] = BIT(~data, i);
}
void mantx_state::ppi3_port_c_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 64] = BIT(~data, i);
}

void mantx_state::program_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
  map(0xc000, 0xc003).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
  map(0xc800, 0xc803).w("ppi2", FUNC(i8255_device::write)); 
  map(0xd000, 0xd003).rw("ppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf800, 0xffff).ram().share("nvram");


}

void mantx_state::io_map(address_map &map)
{
map.global_mask(0xff);
}

void mantx_state::mcu_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void mantx_state::machine_start()
{
}

void mantx_state::mantx(machine_config &config)
{

	/* basic machine hardware */
	z80_device &maincpu(Z80(config, "maincpu", XTAL(8'000'000)));
	maincpu.set_addrmap(AS_PROGRAM, &mantx_state::program_map);
	maincpu.set_addrmap(AS_IO, &mantx_state::io_map);
	z8_device &mcu(Z86E02(config, "mcu", XTAL(8'000'000)));
 	mcu.set_addrmap(AS_PROGRAM, &mantx_state::mcu_map);
  i8255_device &ppi1(I8255A(config, "ppi1"));
	ppi1.out_pa_callback().set(FUNC(mantx_state::ppi1_port_a_w));
	ppi1.out_pb_callback().set(FUNC(mantx_state::ppi1_port_b_w));
	ppi1.out_pc_callback().set(FUNC(mantx_state::ppi1_port_c_w));

	i8255_device &ppi2(I8255A(config, "ppi2"));
	ppi2.out_pa_callback().set(FUNC(mantx_state::ppi2_port_a_w));
	ppi2.out_pb_callback().set(FUNC(mantx_state::ppi2_port_b_w));
	ppi2.out_pc_callback().set(FUNC(mantx_state::ppi2_port_c_w));

	
	i8255_device &ppi3(I8255A(config, "ppi3"));
	ppi3.out_pa_callback().set(FUNC(mantx_state::ppi3_port_a_w));
	ppi3.out_pb_callback().set(FUNC(mantx_state::ppi3_port_b_w));
	ppi3.out_pc_callback().set(FUNC(mantx_state::ppi3_port_c_w));

  /* sound hardware */
	SPEAKER(config, "mono").front_center();
  ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono", 0.50);




}
void mantx_state::mantxa(machine_config &config)
{
	mantx(config);
	z80_device &maincpu(Z80(config.replace(),"maincpu", XTAL(8'000'000)));
	maincpu.set_addrmap(AS_PROGRAM, &mantx_state::program_map);
	maincpu.set_addrmap(AS_IO, &mantx_state::io_map);
  config.device_remove("mcu");}

ROM_START( mantx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mantx.512", 0x0000, 0x10000, CRC(0F41D2DB) SHA1(90d572a8c22d8e19f27743d037d86c526759bc2d) ) 
  ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "mtc86e21.bin", 0x0000, 0x2000, CRC(73F7A944) SHA1(305e1a161db97b0db6ee68c2134394569eae7032) ) 	
ROM_END

ROM_START( mantxa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "512cj-mt", 0x0000, 0x10000, CRC(63FF1A13) SHA1(13d76efe64814ee1b1beae4e0c7dca1fad83d34d) ) 
	ROM_REGION( 0xE13, "pld", 0 )
	ROM_LOAD( "ict18cv8.jed", 0x0000, 0xE13, CRC(CEE2FE34) SHA1(cb29b0aa85ead16268f287605db92eb5c229181c) ) 	
ROM_END

} // anonymous namespace

 
//    YEAR  NAME            PARENT     MACHINE      INPUT     STATE          INIT        ROT    COMPANY        FULLNAME                     FLAGS
GAME( ????, mantx,            0,       mantx,       mantx,    mantx_state,   empty_init, ROT0,  "<unknown>",  "Man Tian Xing (z86e21 mcu)", MACHINE_NOT_WORKING )
GAME( ????, mantxa,           0,       mantxa,      mantx,    mantx_state,   empty_init, ROT0,  "<unknown>",  "Man Tian Xing (18cv8)",      MACHINE_NOT_WORKING )
