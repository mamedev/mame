// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

   M89 hardware for kiddie rides from Falgas.

   Base Falgas M89 PCB:

   _|_|_|_|___|_|_|_|___|_|_|_|_|_|_|____
  |          _______   _______ _______  |
  | _______  TIC206M   TIC206M TIC206M  |
  |                                     |
  |                   __________________|
  |                  | AY8910A         ||
  |                  |_________________||
  |                                     |
  | 7805CV            :::::::::         |
  |                   __________________|
  |                  | 82C55           ||
  |                  |_________________||
  | TDA7241B             _______________|
  |                     | GM76C28A     ||
  |                     |______________||
  |                    _________________|
  |                   | EPROM          ||
  |                   |________________||
  |    __________           __________  |
  |   |_PAL16V8_|          |SN74LS373N  |
  |  __________       __________________|
  | |_PAL16V8_|      |OKI M80C85A-2    ||
  |                  |_________________||
  |  6.000 MHz Xtal  _________ ________ |
  |                MC14020BCP MC14020BCP|
  | ____________RISER_PCB_______________|
  |_____________________________________|

  The riser PCB contains:
   -4 LEDs (motor on, coin input, timer-sound, light).
   -Bank of 4 dipswitches for timer configuration.
   -Bank of 4 dipswitches for coinage configuration.
   -Volume knob.


   Optional video PCB (25291):
   ______________________________________
  |   Power conn -> ::::::  :::::::::: <- Conn to M89 (timer, sound)
  |                   __________________|
  |                  | NEC D8155HC     ||
  |                  |_________________||
  |                                     |
  |                     ________________|
  |           __       | GM76C28A-10   ||
  |          | |       |_______________||
SN74LS14N -> | |       _________________|
  |          |_|      | EPROM          ||
  |                   |________________||
  |      __________        __________   |
  |     |_PAL16V8_|       |SN74LS373N|  |
  |                   __________________|
  |             Xtal | OKI M80C85A-2   ||
  |        6.000 MHz |_________________||
  |                   __________________|
  |             Xtal | TMS9129NL       ||
  |    10.738635 MHz |_________________||
  |                           __________|
  |                          |UD61464DC||
  |                           __________|
  |                          |UD61464DC||
  |  ___   ___   ___     _________  :   |
  |LM318P LM318P LM318P |CD4016BCN  : <- Conn to monitor
  |                                 :   |
  |_____________________________________|


  TODO (for games with video):
  * main - video CPUs communications
  * inputs

***************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/tms9928a.h"

#include "screen.h"
#include "speaker.h"

namespace
{

class falgasm89_state : public driver_device
{
public:
	falgasm89_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_inputs(*this, "IN%u", 0U)
		, m_psg_pa(0xff)
	{
	}

	void falgasm89_simple(machine_config &config);
	void falgasm89(machine_config &config);
	void falgasm87(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	required_device<i8085a_cpu_device> m_maincpu;

private:
	void psg_pa_w(u8 data);
	u8 psg_pb_r();

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_ioport_array<4> m_inputs;

	u8 m_psg_pa;
};

class falgasm89_video_state : public falgasm89_state
{
public:
	falgasm89_video_state(const machine_config &mconfig, device_type type, const char *tag)
		: falgasm89_state(mconfig, type, tag)
		, m_videocpu(*this, "videocpu")
	{
	}

	void falgasm89_video(machine_config &config);

private:
	required_device<i8085a_cpu_device> m_videocpu;

	void main_io_map(address_map &map) ATTR_COLD;
	void video_mem_map(address_map &map) ATTR_COLD;
	void video_io_map(address_map &map) ATTR_COLD;
};

void falgasm89_state::machine_start()
{
	save_item(NAME(m_psg_pa));
}

void falgasm89_state::psg_pa_w(u8 data)
{
	m_psg_pa = data;
}

u8 falgasm89_state::psg_pb_r()
{
	u8 result = 0xff;
	for (int n = 0; n < 4; n++)
		if (!BIT(m_psg_pa, n))
			result &= m_inputs[n]->read();

	return result;
}

void falgasm89_state::mem_map(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("maincpu", 0);
	map(0xfc00, 0xffff).ram();
}

void falgasm89_state::io_map(address_map &map)
{
	map(0x00, 0x00).rw("psg", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x04, 0x04).w("psg", FUNC(ay8910_device::address_w));
}

void falgasm89_video_state::main_io_map(address_map &map)
{
	map(0x00, 0x00).rw("psg", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x04, 0x04).w("psg", FUNC(ay8910_device::address_w));
	map(0x98, 0x98).lw8(NAME([this] (u8 data) { logerror("to video: %02x\n", data); }));
	map(0x99, 0x99).lr8(NAME([this] () -> u8 { logerror("from video\n"); return 0xff; }));
}

void falgasm89_video_state::video_mem_map(address_map &map)
{
	map(0x0000, 0x8fff).rom().region("videocpu", 0);
	map(0xf800, 0xffff).ram();
	//map(0xf800, 0xf8ff).rw("i8155", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w)); // TODO: where's this?
}

void falgasm89_video_state::video_io_map(address_map &map)
{
	map(0x00, 0x07).rw("i8155", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x08, 0x08).rw("vdp", FUNC(tms9129_device::vram_read), FUNC(tms9129_device::vram_write));
	map(0x09, 0x09).rw("vdp", FUNC(tms9129_device::register_read), FUNC(tms9129_device::register_write));
}

INPUT_PORTS_START(falgasm89)
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

// The "simple" PCB has the i8255 socket empty
void falgasm89_state::falgasm89_simple(machine_config &config)
{
	I8085A(config, m_maincpu, 6_MHz_XTAL); // OKI M80C85A-2
	m_maincpu->set_addrmap(AS_PROGRAM, &falgasm89_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &falgasm89_state::io_map);

	SPEAKER(config, "mono").front_center();

	ay8910_device &psg(AY8910(config, "psg", 6_MHz_XTAL / 4)); // divider unknown
	psg.add_route(ALL_OUTPUTS, "mono", 0.50);
	psg.port_a_write_callback().set(FUNC(falgasm89_state::psg_pa_w));
	psg.port_b_read_callback().set(FUNC(falgasm89_state::psg_pb_r));
}

// Falgas Micro-87 hardware. Mainly the same as the "simple" PCB, but with slower clock
void falgasm89_state::falgasm87(machine_config &config)
{
	falgasm89_simple(config);
	m_maincpu->set_clock(4_MHz_XTAL);
}

void falgasm89_state::falgasm89(machine_config &config)
{
	falgasm89_simple(config);

	I8255(config, "i8255"); // NEC D71055C
}

void falgasm89_video_state::falgasm89_video(machine_config &config)
{
	falgasm89(config);

	m_maincpu->set_addrmap(AS_IO, &falgasm89_video_state::main_io_map);

	I8085A(config, m_videocpu, 6_MHz_XTAL); // OKI M80C85A-2
	m_videocpu->set_addrmap(AS_PROGRAM, &falgasm89_video_state::video_mem_map);
	m_videocpu->set_addrmap(AS_IO, &falgasm89_video_state::video_io_map);

	tms9129_device &vdp(TMS9129(config, "vdp", 10.738635_MHz_XTAL));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x10000); // 2 x UD61464DC
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	i8155_device &i8155(I8155(config, "i8155", 6_MHz_XTAL)); // NEC D8155HC
	i8155.in_pa_callback().set([this] () { logerror("from main (i8155 PA in)\n"); return 0x00; }); // TODO: from main? returning rand() shows inputs come from here, probably sent from the main CPU
	i8155.out_pb_callback().set([this] (u8 data) { logerror("to main (i8155 PB out): %02x\n", data); }); // TODO: to main? bit 7 toggles continuously
	// other ports seem unused
	i8155.out_to_callback().set_inputline(m_videocpu, I8085_TRAP_LINE);
	i8155.out_to_callback().append_inputline("maincpu", I8085_TRAP_LINE); // TODO: wrong
}

// Falgas M89-N main PCB.
ROM_START(cbully)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bully-gs_m89-iv_16-1-91.u2", 0x0000, 0x8000, CRC(4cc85230) SHA1(c3851e6610bcb3427f81ecfcd4575603a9edca6e)) // 27C256

	ROM_REGION(0x22e, "plds", 0)
	ROM_LOAD("palce16v8_m894-bt.u11", 0x000, 0x117, NO_DUMP) // Protected
	ROM_LOAD("palce16v8_m894-a.u10",  0x117, 0x117, NO_DUMP) // Protected
ROM_END

// Falgas M89-E5 main PCB
ROM_START(fantcar)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cochefantastico89.p25", 0x0000, 0x8000, CRC(884a9768) SHA1(6f36a63312ae1f6899d26ca6953f942ddd860742))

	ROM_REGION(0x22e, "plds", 0)
	ROM_LOAD("pal.u11", 0x000, 0x117, NO_DUMP)
	ROM_LOAD("pal.u10", 0x117, 0x117, NO_DUMP)
ROM_END

/* First version of "Fantastic Car" runs on Falgas Micro-87 hardware. It was developed by Gaelco, but distributed and sold by Falgas.
   The Micro-87 hardware is older than M89, but shares the main components and architecture (without 8255):

   _|_|_|_|___|_|_|_|____________________
  |                                     |
  |                                     |
  |                   __________________|
  |                  | AY8910A         ||
  |                  |_________________||
  |                                     |
  |                   :::::::::         |
  |                      _______________|
  |       __________    | KM6816AL-15  ||
  |      |TC74HC138P    |______________||
  |                    _________________|
  |       __________  | EPROM          ||
  |      |_74LS00__|  |________________||
  |       __________        __________  |
  |      |TC74HC32P|       |SN74LS373N  |
  |                   __________________|
  |  Xtal            | NEC D8085AHC    ||
  |  4.000 Mhz       |_________________||
  |                  _________          |
  |                 |CD4093BE|          |
  | ____________RISER_PCB_______________|
  |_____________________________________|

  The riser PCB contains:
   -5 LEDs (motor on, coin input, sound, timer, light, and fail).
   -Knob for timer configuration.
   -Bank of 4 dipswitches for coinage configuration.
   -Volume knob.
*/
ROM_START(fantcar87)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mikel_es_m87e_15_2_88.u2",  0x0000, 0x8000, CRC(e1db4836) SHA1(7020bde14ee9afae41691beb0708ed50ca3506d3)) // NM27C256Q
ROM_END

ROM_START(fantcar87a)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mikel_micro_87_25-3-87.u2", 0x0000, 0x8000, CRC(83a16ff4) SHA1(52a1fcd89882fd00c1f46328d75c2623f6f2f83e))
ROM_END


// Falgas M89-E5 main PCB with 25291 video PCB. Bootleg of Konami's Hyper Rally for MSX
ROM_START(rmontecarlo)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("uj_504_m-89e5_17-7-91.u2", 0x00000, 0x10000, CRC(ff1be338) SHA1(9a3f4760bd7e4d9328d44e546bb588561fc53016)) // 27C512

	ROM_REGION(0x10000, "videocpu", 0)
	ROM_LOAD("uj_v10_22-5-91.bin",       0x00000, 0x10000, CRC(8ac21706) SHA1(bd399136d4793c1eaa49c2d5a35022864e771833)) // 27C512

	ROM_REGION(0x345, "plds", 0)
	ROM_LOAD("palce16v8_m894-bt.u11", 0x000, 0x117, NO_DUMP) // Protected, on M89 PCB
	ROM_LOAD("palce16v8_m894-a.u10",  0x117, 0x117, NO_DUMP) // Protected, on M89 PCB
	ROM_LOAD("palce16v8_video91.bin", 0x22e, 0x117, NO_DUMP) // Protected, on video PCB
ROM_END

} // anonymous namespace

//    YEAR  NAME         PARENT   MACHINE           INPUT      CLASS                  INIT        ROT   COMPANY   FULLNAME                                    FLAGS
GAME( 1991, cbully,      0,       falgasm89_simple, falgasm89, falgasm89_state,       empty_init, ROT0, "Falgas", "Coche Bully",                              MACHINE_IS_SKELETON_MECHANICAL )
GAME( 19??, fantcar,     0,       falgasm89,        falgasm89, falgasm89_state,       empty_init, ROT0, "Falgas", "Fantastic Car (M89 hardware)",             MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1988, fantcar87,   fantcar, falgasm87,        falgasm89, falgasm89_state,       empty_init, ROT0, "Falgas", "Fantastic Car (Micro-87 hardware, newer)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, fantcar87a,  fantcar, falgasm87,        falgasm89, falgasm89_state,       empty_init, ROT0, "Falgas", "Fantastic Car (Micro-87 hardware, older)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1991, rmontecarlo, 0,       falgasm89_video,  falgasm89, falgasm89_video_state, empty_init, ROT0, "Falgas", "Rally Montecarlo",                         MACHINE_IS_SKELETON_MECHANICAL )
