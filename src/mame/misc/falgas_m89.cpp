// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

   M89 hardware for kiddie rides from Falgas

   Base Falgas M89-4 N/E PCB

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


   Optional video PCB (25291)
   ______________________________________
  |   Power conn -> ::::::  :::::::::: <- Conn to M89E (timer, sound)
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

protected:
	virtual void machine_start() override;

private:
	void psg_pa_w(u8 data);
	u8 psg_pb_r();

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<i8085a_cpu_device> m_maincpu;
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
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

void falgasm89_state::io_map(address_map &map)
{
	map(0x00, 0x00).rw("psg", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x04, 0x04).w("psg", FUNC(ay8910_device::address_w));
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

void falgasm89_state::falgasm89(machine_config &config)
{
	falgasm89_simple(config);

	I8255(config, "i8255"); // NEC D71055C
}

void falgasm89_video_state::falgasm89_video(machine_config &config)
{
	falgasm89(config);

	I8085A(config, m_videocpu, 6_MHz_XTAL); // OKI M80C85A-2

	tms9129_device &vdp(TMS9129(config, "vdp", 10.738635_MHz_XTAL));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x10000); // 2 x UD61464DC
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	I8155(config, "i8155", 6_MHz_XTAL); // NEC D8155HC
}

ROM_START(cbully)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bully-gs_m89-iv_16-1-91.u2", 0x0000, 0x8000, CRC(4cc85230) SHA1(c3851e6610bcb3427f81ecfcd4575603a9edca6e)) // 27C256

	ROM_REGION(0x22e, "plds", 0)
	ROM_LOAD("palce16v8_m894-bt.u11", 0x000, 0x117, NO_DUMP) // Protected
	ROM_LOAD("palce16v8_m894-a.u10",  0x117, 0x117, NO_DUMP) // Protected
ROM_END

// Bootleg of Konami's Hyper Rally for MSX
ROM_START(rmontecarlo)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("uj_504_m-89es_17-7-91.u2", 0x00000, 0x10000, CRC(ff1be338) SHA1(9a3f4760bd7e4d9328d44e546bb588561fc53016)) // 27C512

	ROM_REGION(0x10000, "videocpu", 0)
	ROM_LOAD("uj_v10_22-5-91.bin",       0x00000, 0x10000, CRC(8ac21706) SHA1(bd399136d4793c1eaa49c2d5a35022864e771833)) // 27C512

	ROM_REGION(0x345, "plds", 0)
	ROM_LOAD("palce16v8_m894-bt.u11", 0x000, 0x117, NO_DUMP) // Protected, on M89 PCB
	ROM_LOAD("palce16v8_m894-a.u10",  0x117, 0x117, NO_DUMP) // Protected, on M89 PCB
	ROM_LOAD("palce16v8_video91.bin", 0x22e, 0x117, NO_DUMP) // Protected, on video PCB
ROM_END

} // anonymous namespace

//    YEAR  NAME         PARENT MACHINE           INPUT      CLASS                  INIT        ROT   COMPANY   FULLNAME            FLAGS
GAME( 1991, cbully,      0,     falgasm89_simple, falgasm89, falgasm89_state,       empty_init, ROT0, "Falgas", "Coche Bully",      MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1991, rmontecarlo, 0,     falgasm89_video,  falgasm89, falgasm89_video_state, empty_init, ROT0, "Falgas", "Rally Montecarlo", MACHINE_IS_SKELETON_MECHANICAL )
