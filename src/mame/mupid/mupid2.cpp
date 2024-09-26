// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    mupid/Infonova C2A2
    Grundig PTC-100

    - Z80
    - 128 + 8 KB RAM
    - Z80 SIO/0
    - 8035
    - M58990P-1 ADC

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/adc0808.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/z80sio.h"
#include "emupal.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mupid2_state : public driver_device
{
public:
	mupid2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_kbdcpu(*this, "kbdcpu"),
		m_ram(*this, "ram"),
		m_rombank(*this, "rombank"),
		m_rambank(*this, "rambank"),
		m_palette(*this, "palette"),
		m_nmi_enabled(false)
		{ }

	void c2a2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(nmi);
	void port_a0_w(uint8_t data);
	void port_c0_w(uint8_t data);
	template<int C> void palette_w(uint8_t data);
	uint8_t kbd_bus_r();
	uint8_t kbd_p1_r();
	void kbd_p1_w(uint8_t data);
	void kbd_p2_w(uint8_t data);

	void palette_init(palette_device &palette);

	required_device<cpu_device> m_maincpu;
	required_device<i8035_device> m_kbdcpu;
	required_device<ram_device> m_ram;
	required_memory_bank m_rombank;
	required_memory_bank m_rambank;
	required_device<palette_device> m_palette;

	void maincpu_mem(address_map &map) ATTR_COLD;
	void maincpu_io(address_map &map) ATTR_COLD;
	void kbdcpu_mem(address_map &map) ATTR_COLD;

	std::unique_ptr<std::array<uint8_t, 3>[]> m_color_ram;
	bool m_nmi_enabled;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void mupid2_state::maincpu_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x5fff).bankr(m_rombank);
	map(0x6000, 0x7fff).ram();
	map(0x8000, 0xffff).bankrw(m_rambank);
}

void mupid2_state::maincpu_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0xa0, 0xa0).w(FUNC(mupid2_state::port_a0_w));
	map(0xc0, 0xc0).w(FUNC(mupid2_state::port_c0_w));
	map(0xe0, 0xe0).w(FUNC(mupid2_state::palette_w<0>));
	map(0xe8, 0xe8).w(FUNC(mupid2_state::palette_w<1>));
	map(0xf0, 0xf0).w(FUNC(mupid2_state::palette_w<2>));
}

void mupid2_state::kbdcpu_mem(address_map &map)
{
	map(0x000, 0x3ff).mirror(0xc00).rom();
}


//**************************************************************************
//  INPUTS
//**************************************************************************

INPUT_PORTS_START( mupid2 )
INPUT_PORTS_END

uint8_t mupid2_state::kbd_bus_r()
{
//  logerror("kbd_bus_r\n");
	return 0xff;
}

uint8_t mupid2_state::kbd_p1_r()
{
//  logerror("kbd_p1_r\n");
	return 0xff;
}

void mupid2_state::kbd_p1_w(uint8_t data)
{
//  logerror("kbd_p1_w: %02x\n", data);
}

void mupid2_state::kbd_p2_w(uint8_t data)
{
//  logerror("kbd_p2_w: %02x\n", data);
}


//**************************************************************************
//  VIDEO
//**************************************************************************

void mupid2_state::palette_init(palette_device &palette)
{
	// first 16 colors are fixed (arbitrary, colors unknown)
	for (int i = 0; i < 16; i++)
	{
		int r = (i >> 0) & 1;
		int g = (i >> 1) & 1;
		int b = (i >> 2) & 3;
		palette.set_pen_color(i, rgb_t(pal1bit(r), pal1bit(g), pal2bit(b)));
	}
}

template<int C>
void mupid2_state::palette_w(uint8_t data)
{
	int i = data & 0x0f;
	m_color_ram[i][C] = data >> 4;
	m_palette->set_pen_color(16 + i, rgb_t(
		pal4bit(m_color_ram[i][0]),
		pal4bit(m_color_ram[i][1]),
		pal4bit(m_color_ram[i][2])
	));
}


//**************************************************************************
//  MACHINE
//**************************************************************************

void mupid2_state::machine_start()
{
	// additional rom space
	m_rombank->configure_entries(0, 4, memregion("maincpu")->base() + 0x4000, 0x2000);
	m_rombank->set_entry(0);

	// 128k memory
	m_rambank->configure_entries(0, 4, m_ram->pointer(), 0x8000);
	m_rambank->set_entry(0);

	// allocate space for colors
	m_color_ram = make_unique_clear<std::array<uint8_t, 3>[]>(16);

	// register for save states
	save_pointer(NAME(m_color_ram), 16);
	save_item(NAME(m_nmi_enabled));
}

TIMER_DEVICE_CALLBACK_MEMBER(mupid2_state::nmi)
{
	if (m_nmi_enabled)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void mupid2_state::port_a0_w(uint8_t data)
{
	logerror("port_a0_w: %02x\n", data);

	// 7-------  unknown
	// -6------  unknown
	// --5-----  unknown
	// ---4----  unknown
	// ----3---  unknown
	// -----2--  unknown
	// ------1-  unknown
	// -------0  unknown
}

void mupid2_state::port_c0_w(uint8_t data)
{
	logerror("port_c0_w: %02x\n", data);

	// 7-------  nmi mask
	// -6------  unknown
	// --5-----  unknown
	// ---4----  unknown
	// ----32--  ram bank?
	// ------1-  unknown
	// -------0  unknown

	m_nmi_enabled = BIT(data, 7);
	m_rambank->set_entry((data >> 2) & 0x03);
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void mupid2_state::c2a2(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mupid2_state::maincpu_mem);
	m_maincpu->set_addrmap(AS_IO, &mupid2_state::maincpu_io);

	RAM(config, m_ram).set_default_size("128K");

	// timing unknown
	TIMER(config, "nmi_timer").configure_periodic(FUNC(mupid2_state::nmi), attotime::from_hz(5000));

	Z80SIO(config, "sio", 4000000);

	I8035(config, m_kbdcpu, 4000000);
	m_kbdcpu->set_addrmap(AS_PROGRAM, &mupid2_state::kbdcpu_mem);
	m_kbdcpu->bus_in_cb().set(FUNC(mupid2_state::kbd_bus_r));
	m_kbdcpu->p1_in_cb().set(FUNC(mupid2_state::kbd_p1_r));
	m_kbdcpu->p1_out_cb().set(FUNC(mupid2_state::kbd_p1_w));
	m_kbdcpu->p2_out_cb().set(FUNC(mupid2_state::kbd_p2_w));

	M58990(config, "adc", 1000000);

	PALETTE(config, m_palette, FUNC(mupid2_state::palette_init), 32);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( mupid2 )
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD("0090.7012.40.02_27.09.85.u40", 0x0000, 0x4000, CRC(0b320f46) SHA1(064e1b1697b9b767f89be7c1e3d20e1157324791))
	ROM_LOAD("0090.7012.39.02_27.09.85.u39", 0x4000, 0x4000, CRC(b2fb634c) SHA1(70ced48d0a27a661ddd7fbc529a891bd0bcec926))
	ROM_RELOAD(0x8000, 0x4000)

	ROM_REGION(0x800, "kbdcpu", 0)
	ROM_LOAD("motronic_0090.6820.06.00", 0x000, 0x400, CRC(78b6d827) SHA1(200f2b33c518a889f8c6a5f4dd4443ac76884b21))
	ROM_CONTINUE(0x000, 0x400)
ROM_END

ROM_START( mupid2i )
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD("kv_2.5.90_c2a2_ffd3.u40", 0x0000, 0x8000, CRC(a77ccb92) SHA1(9588e07ee0d4f06b346b7f5b58b8086a1b6ef140))
	ROM_LOAD("kh_2.5.90_c2a2_6860.u39", 0x8000, 0x4000, CRC(cdf64a6d) SHA1(a4a76ac761de016c7a196120a1c9fef6016c171c))

	ROM_REGION(0x800, "kbdcpu", 0)
	ROM_LOAD("motronic_0090.6820.06.00", 0x000, 0x400, CRC(78b6d827) SHA1(200f2b33c518a889f8c6a5f4dd4443ac76884b21))
	ROM_CONTINUE(0x000, 0x400)
ROM_END

ROM_START( ptc100 )
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD("mup.u40", 0x0000, 0x4000, CRC(b4ac8ccb) SHA1(01bc818ec571d099176b6f69aaa736bb5410dd8e))
	ROM_LOAD("mup.u39", 0x4000, 0x4000, CRC(9812fefc) SHA1(bb4e69eba504dae6065094d9668be2b0478b0433))
	ROM_RELOAD(0x8000, 0x4000)

	ROM_REGION(0x800, "kbdcpu", 0)
	ROM_LOAD("kbc.bin", 0x000, 0x400, CRC(72502f02) SHA1(4adb5c55691e1c53a3364d97e64d194be4886b52))
	ROM_CONTINUE(0x000, 0x400)
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY     FULLNAME            FLAGS
COMP( 1985, mupid2,  0,      0,      c2a2,    mupid2, mupid2_state, empty_init, "mupid",    "Post-Mupid C2A2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1990, mupid2i, mupid2, 0,      c2a2,    mupid2, mupid2_state, empty_init, "Infonova", "C2A2",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 198?, ptc100,  mupid2, 0,      c2a2,    mupid2, mupid2_state, empty_init, "Grundig",  "PTC-100",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
