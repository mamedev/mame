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
#include "machine/pcecommn.h"

#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "video/huc6260.h"
#include "video/huc6270.h"
#include "cpu/h6280/h6280.h"
#include "sound/c6280.h"
#include "screen.h"
#include "speaker.h"


class paranoia_state : public pce_common_state
{
public:
	paranoia_state(const machine_config &mconfig, device_type type, const char *tag)
		: pce_common_state(mconfig, type, tag) { }

	DECLARE_WRITE8_MEMBER(i8085_d000_w);
	DECLARE_READ8_MEMBER(z80_io_01_r);
	DECLARE_READ8_MEMBER(z80_io_02_r);
	DECLARE_WRITE8_MEMBER(z80_io_17_w);
	DECLARE_WRITE8_MEMBER(z80_io_37_w);
	DECLARE_WRITE8_MEMBER(i8155_a_w);
	DECLARE_WRITE8_MEMBER(i8155_b_w);
	DECLARE_WRITE8_MEMBER(i8155_c_w);
	DECLARE_WRITE_LINE_MEMBER(i8155_timer_out);
	void paranoia(machine_config &config);
	void paranoia_8085_io_map(address_map &map);
	void paranoia_8085_map(address_map &map);
	void paranoia_z80_io_map(address_map &map);
	void paranoia_z80_map(address_map &map);
	void pce_io(address_map &map);
	void pce_mem(address_map &map);
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
	map(0x1FE800, 0x1FEBFF).rw("c6280", FUNC(c6280_device::c6280_r), FUNC(c6280_device::c6280_w));
	map(0x1FEC00, 0x1FEFFF).rw(m_maincpu, FUNC(h6280_device::timer_r), FUNC(h6280_device::timer_w));
	map(0x1FF000, 0x1FF3FF).rw(this, FUNC(paranoia_state::pce_joystick_r), FUNC(paranoia_state::pce_joystick_w));
	map(0x1FF400, 0x1FF7FF).rw(m_maincpu, FUNC(h6280_device::irq_status_r), FUNC(h6280_device::irq_status_w));
}

void paranoia_state::pce_io(address_map &map)
{
	map(0x00, 0x03).rw("huc6270", FUNC(huc6270_device::read), FUNC(huc6270_device::write));
}

WRITE8_MEMBER(paranoia_state::i8085_d000_w)
{
	//logerror( "D000 (8085) write %02x\n", data );
}

void paranoia_state::paranoia_8085_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x80ff).rw("i8155", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x8100, 0x8107).rw("i8155", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xd000, 0xd000).w(this, FUNC(paranoia_state::i8085_d000_w));
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

READ8_MEMBER(paranoia_state::z80_io_01_r)
{
	return 0;
}

READ8_MEMBER(paranoia_state::z80_io_02_r)
{
	return 0;
}

WRITE8_MEMBER(paranoia_state::z80_io_17_w)
{
}

WRITE8_MEMBER(paranoia_state::z80_io_37_w)
{
}

void paranoia_state::paranoia_z80_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(this, FUNC(paranoia_state::z80_io_01_r));
	map(0x02, 0x02).r(this, FUNC(paranoia_state::z80_io_02_r));
	map(0x17, 0x17).w(this, FUNC(paranoia_state::z80_io_17_w));
	map(0x37, 0x37).w(this, FUNC(paranoia_state::z80_io_37_w));
}

WRITE8_MEMBER(paranoia_state::i8155_a_w)
{
	//logerror("i8155 Port A: %02X\n", data);
}

WRITE8_MEMBER(paranoia_state::i8155_b_w)
{
	//logerror("i8155 Port B: %02X\n", data);
}

WRITE8_MEMBER(paranoia_state::i8155_c_w)
{
	//logerror("i8155 Port C: %02X\n", data);
}

WRITE_LINE_MEMBER(paranoia_state::i8155_timer_out)
{
	//m_subcpu->set_input_line(I8085_RST55_LINE, state ? CLEAR_LINE : ASSERT_LINE );
	//logerror("Timer out %d\n", state);
}

MACHINE_CONFIG_START(paranoia_state::paranoia)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", H6280, PCE_MAIN_CLOCK/3)
	MCFG_DEVICE_PROGRAM_MAP(pce_mem)
	MCFG_DEVICE_IO_MAP(pce_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_DEVICE_ADD("sub", I8085A, 18000000/3)
	MCFG_DEVICE_PROGRAM_MAP(paranoia_8085_map)
	MCFG_DEVICE_IO_MAP(paranoia_8085_io_map)

	MCFG_DEVICE_ADD("sub2", Z80, 18000000/6)
	MCFG_DEVICE_PROGRAM_MAP(paranoia_z80_map)
	MCFG_DEVICE_IO_MAP(paranoia_z80_io_map)

	MCFG_DEVICE_ADD("i8155", I8155, 1000000 /*?*/)
	MCFG_I8155_OUT_PORTA_CB(WRITE8(*this, paranoia_state, i8155_a_w))
	MCFG_I8155_OUT_PORTB_CB(WRITE8(*this, paranoia_state, i8155_b_w))
	MCFG_I8155_OUT_PORTC_CB(WRITE8(*this, paranoia_state, i8155_c_w))
	MCFG_I8155_OUT_TIMEROUT_CB(WRITELINE(*this, paranoia_state, i8155_timer_out))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PCE_MAIN_CLOCK, huc6260_device::WPF, 64, 64 + 1024 + 64, huc6260_device::LPF, 18, 18 + 242)
	MCFG_SCREEN_UPDATE_DRIVER( pce_common_state, screen_update )
	MCFG_SCREEN_PALETTE("huc6260")

	MCFG_DEVICE_ADD( "huc6260", HUC6260, PCE_MAIN_CLOCK )
	MCFG_HUC6260_NEXT_PIXEL_DATA_CB(READ16("huc6270", huc6270_device, next_pixel))
	MCFG_HUC6260_TIME_TIL_NEXT_EVENT_CB(READ16("huc6270", huc6270_device, time_until_next_event))
	MCFG_HUC6260_VSYNC_CHANGED_CB(WRITELINE("huc6270", huc6270_device, vsync_changed))
	MCFG_HUC6260_HSYNC_CHANGED_CB(WRITELINE("huc6270", huc6270_device, hsync_changed))
	MCFG_DEVICE_ADD( "huc6270", HUC6270, 0 )
	MCFG_HUC6270_VRAM_SIZE(0x10000)
	MCFG_HUC6270_IRQ_CHANGED_CB(INPUTLINE("maincpu", 0))

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	MCFG_DEVICE_ADD("c6280", C6280, PCE_MAIN_CLOCK/6)
	MCFG_C6280_CPU("maincpu")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

MACHINE_CONFIG_END

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

GAME( 1990, paranoia, 0, paranoia, paranoia, paranoia_state, init_pce_common, ROT0, "Naxat Soft", "Paranoia", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
