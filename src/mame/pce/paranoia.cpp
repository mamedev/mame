// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/****************************************************************************

    Paranoia
    Driver by Mariusz Wojcieszek

    Notes:
    - jamma interface is not emulated, hence the game is marked as 'not working'
    - rom mapping, memory maps and clocks for jamma interface cpus are probably not correct

Paranoia by Naxat Soft 1990

CPU Z84C00A85 (Z80A CPU)

Xtal : 18.000 Mhz

Ram : GM76C28A (Goldstar)

Ram : 2x W2416K-70 (Winbond)

Else :

Winbond WF19054

Sound : Nec D8085AHC + Nec D8155HC

This board has also :

HuC6260A (Hudson)
HuC6270  (Hudson)
HuC6280A (Hudson)
2x HSRM2564LM12
1x HSRM2564LM10

****************************************************************************/

#include "emu.h"
#include "pcecommn.h"

#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "video/huc6260.h"
#include "video/huc6270.h"
#include "cpu/h6280/h6280.h"
#include "screen.h"
#include "speaker.h"


namespace {

class paranoia_state : public pce_common_state
{
public:
	paranoia_state(const machine_config &mconfig, device_type type, const char *tag)
		: pce_common_state(mconfig, type, tag) { }

	void paranoia(machine_config &config);

private:
	void i8085_d000_w(uint8_t data);
	uint8_t z80_io_01_r();
	uint8_t z80_io_02_r();
	void z80_io_17_w(uint8_t data);
	void z80_io_37_w(uint8_t data);
	void i8155_a_w(uint8_t data);
	void i8155_b_w(uint8_t data);
	void i8155_c_w(uint8_t data);
	void i8155_timer_out(int state);
	void paranoia_8085_io_map(address_map &map) ATTR_COLD;
	void paranoia_8085_map(address_map &map) ATTR_COLD;
	void paranoia_z80_io_map(address_map &map) ATTR_COLD;
	void paranoia_z80_map(address_map &map) ATTR_COLD;
	void pce_io(address_map &map) ATTR_COLD;
	void pce_mem(address_map &map) ATTR_COLD;
};


static INPUT_PORTS_START( paranoia )
	PCE_STANDARD_INPUT_PORT_P1
INPUT_PORTS_END

void paranoia_state::pce_mem(address_map &map)
{
	map(0x000000, 0x03FFFF).rom();
	map(0x1F0000, 0x1F1FFF).ram().mirror(0x6000);
	map(0x1FE000, 0x1FE3FF).rw("huc6270", FUNC(huc6270_device::read), FUNC(huc6270_device::write));
	map(0x1FE400, 0x1FE7FF).rw(m_huc6260, FUNC(huc6260_device::read), FUNC(huc6260_device::write));
}

void paranoia_state::pce_io(address_map &map)
{
	map(0x00, 0x03).rw("huc6270", FUNC(huc6270_device::read), FUNC(huc6270_device::write));
}

void paranoia_state::i8085_d000_w(uint8_t data)
{
	//logerror( "D000 (8085) write %02x\n", data );
}

void paranoia_state::paranoia_8085_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x80ff).rw("i8155", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x8100, 0x8107).rw("i8155", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xd000, 0xd000).w(FUNC(paranoia_state::i8085_d000_w));
	map(0xe000, 0xe1ff).ram();
}

void paranoia_state::paranoia_8085_io_map(address_map &map)
{
}

void paranoia_state::paranoia_z80_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x6000, 0x67ff).ram();
	map(0x7000, 0x73ff).ram();
}

uint8_t paranoia_state::z80_io_01_r()
{
	return 0;
}

uint8_t paranoia_state::z80_io_02_r()
{
	return 0;
}

void paranoia_state::z80_io_17_w(uint8_t data)
{
}

void paranoia_state::z80_io_37_w(uint8_t data)
{
}

void paranoia_state::paranoia_z80_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(FUNC(paranoia_state::z80_io_01_r));
	map(0x02, 0x02).r(FUNC(paranoia_state::z80_io_02_r));
	map(0x17, 0x17).w(FUNC(paranoia_state::z80_io_17_w));
	map(0x37, 0x37).w(FUNC(paranoia_state::z80_io_37_w));
}

void paranoia_state::i8155_a_w(uint8_t data)
{
	//logerror("i8155 Port A: %02X\n", data);
}

void paranoia_state::i8155_b_w(uint8_t data)
{
	//logerror("i8155 Port B: %02X\n", data);
}

void paranoia_state::i8155_c_w(uint8_t data)
{
	//logerror("i8155 Port C: %02X\n", data);
}

void paranoia_state::i8155_timer_out(int state)
{
	//m_subcpu->set_input_line(I8085_RST55_LINE, state ? CLEAR_LINE : ASSERT_LINE );
	//logerror("Timer out %d\n", state);
}

void paranoia_state::paranoia(machine_config &config)
{
	/* basic machine hardware */
	H6280(config, m_maincpu, PCE_MAIN_CLOCK/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &paranoia_state::pce_mem);
	m_maincpu->set_addrmap(AS_IO, &paranoia_state::pce_io);
	m_maincpu->port_in_cb().set(FUNC(paranoia_state::pce_joystick_r));
	m_maincpu->port_out_cb().set(FUNC(paranoia_state::pce_joystick_w));
	m_maincpu->add_route(0, "speaker", 1.00, 0);
	m_maincpu->add_route(1, "speaker", 1.00, 1);

	config.set_maximum_quantum(attotime::from_hz(60));

	i8085a_cpu_device &sub(I8085A(config, "sub", 18000000/3));
	sub.set_addrmap(AS_PROGRAM, &paranoia_state::paranoia_8085_map);
	sub.set_addrmap(AS_IO, &paranoia_state::paranoia_8085_io_map);

	z80_device &sub2(Z80(config, "sub2", 18000000/6));
	sub2.set_addrmap(AS_PROGRAM, &paranoia_state::paranoia_z80_map);
	sub2.set_addrmap(AS_IO, &paranoia_state::paranoia_z80_io_map);

	i8155_device &i8155(I8155(config, "i8155", 1000000 /*?*/));
	i8155.out_pa_callback().set(FUNC(paranoia_state::i8155_a_w));
	i8155.out_pb_callback().set(FUNC(paranoia_state::i8155_b_w));
	i8155.out_pc_callback().set(FUNC(paranoia_state::i8155_c_w));
	i8155.out_to_callback().set(FUNC(paranoia_state::i8155_timer_out));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PCE_MAIN_CLOCK, huc6260_device::WPF, 64, 64 + 1024 + 64, huc6260_device::LPF, 18, 18 + 242);
	screen.set_screen_update(FUNC(pce_common_state::screen_update));
	screen.set_palette(m_huc6260);

	HUC6260(config, m_huc6260, PCE_MAIN_CLOCK);
	m_huc6260->next_pixel_data().set("huc6270", FUNC(huc6270_device::next_pixel));
	m_huc6260->time_til_next_event().set("huc6270", FUNC(huc6270_device::time_until_next_event));
	m_huc6260->vsync_changed().set("huc6270", FUNC(huc6270_device::vsync_changed));
	m_huc6260->hsync_changed().set("huc6270", FUNC(huc6270_device::hsync_changed));

	huc6270_device &huc6270(HUC6270(config, "huc6270", 0));
	huc6270.set_vram_size(0x10000);
	huc6270.irq().set_inputline(m_maincpu, 0);

	SPEAKER(config, "speaker", 2).front();
}

ROM_START(paranoia)
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "5.201", 0x00000, 0x40000, CRC(9893e0e6) SHA1(b3097e7f163e4a067cf32f290e59657a8b5e271b) )

	ROM_REGION( 0x8000, "sub", 0 )
	ROM_LOAD( "6.29", 0x0000, 0x8000, CRC(5517532e) SHA1(df8f1621abf1f0c65d86d406cd79d97ec233c378) )

	ROM_REGION( 0x20000, "sub2", 0 )
	ROM_LOAD( "1.319", 0x00000, 0x8000, CRC(ef9f85d8) SHA1(951239042b56cd256daf1965ead2949e2bddcd8b) )
	ROM_LOAD( "2.318", 0x08000, 0x8000, CRC(a35fccca) SHA1(d50e9044a97fe77f31e3198bb6759ba451359069) )
	ROM_LOAD( "3.317", 0x10000, 0x8000, CRC(e3e48ec1) SHA1(299820d0e4fb2fd947c7a52f1c49e2e4d0dd050a) )
	ROM_LOAD( "4.352", 0x18000, 0x8000, CRC(11297fed) SHA1(17a294e65ba1c4806307602dee4c7c627ad1fcfd) )
ROM_END

} // anonymous namespace


GAME( 1990, paranoia, 0, paranoia, paranoia, paranoia_state, init_pce_common, ROT0, "Naxat Soft", "Paranoia (Arcade PC Engine, bootleg?)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // Based off PCE not TG16, cfr. stage clear screen
