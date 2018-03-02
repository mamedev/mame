// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************************************

    PINBALL
    Video Dens S.A., Madrid

    PinMAME used as reference. The Break '86 manual scan available on the net includes
    a mostly illegible schematic.

    Nothing in this driver is confirmed except where noted.

    Ator runs on different hardware (peyper.cpp).

***************************************************************************************/


#include "emu.h"
#include "machine/genpin.h"
#include "machine/timer.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "vd.lh"

class vd_state : public genpin_class
{
public:
	vd_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ8_MEMBER(ack_r);
	DECLARE_WRITE8_MEMBER(col_w);
	DECLARE_WRITE8_MEMBER(disp_w);
	DECLARE_WRITE8_MEMBER(lamp_w) { };
	DECLARE_WRITE8_MEMBER(sol_w) { };
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	void vd(machine_config &config);
	void vd_io(address_map &map);
	void vd_map(address_map &map);
protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override;
private:
	uint8_t m_t_c;
	uint8_t segment[5];
};


READ8_MEMBER(vd_state::ack_r)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE); // guess
	return 0; // this value is not used
}

ADDRESS_MAP_START(vd_state::vd_map)
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x62ff) AM_RAM
	AM_RANGE(0x6700, 0x67ff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(vd_state::vd_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("X0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("X1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("X2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("X3")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("X4")
	AM_RANGE(0x05, 0x05) AM_READ_PORT("X5")
	AM_RANGE(0x20, 0x27) AM_WRITE(lamp_w)
	AM_RANGE(0x28, 0x28) AM_WRITE(sol_w)
	AM_RANGE(0x40, 0x44) AM_WRITE(disp_w)
	AM_RANGE(0x60, 0x60) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x61, 0x61) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x62, 0x62) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0x80, 0x80) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0x81, 0x81) AM_DEVREAD("ay2", ay8910_device, data_r) // probably never read
	AM_RANGE(0x82, 0x82) AM_DEVWRITE("ay2", ay8910_device, data_w)
	AM_RANGE(0xa0, 0xa0) AM_READ(ack_r)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(col_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( break86 )
	PORT_START("DSW1") // "Micro Swicher Nº 1"
	PORT_DIPUNKNOWN(0x01, 0x01) PORT_DIPLOCATION("SW1:8") // used but not described in service manual
	PORT_DIPUNKNOWN(0x02, 0x02) PORT_DIPLOCATION("SW1:7") // used but not described in service manual
	PORT_DIPUNKNOWN(0x04, 0x04) PORT_DIPLOCATION("SW1:6") // used but not described in service manual
	PORT_DIPUNUSED(0x08, 0x08) PORT_DIPLOCATION("SW1:5")
	PORT_DIPNAME(0x30, 0x30, "Power Bumpers") PORT_DIPLOCATION("SW1:4,3") // "Potencia Bu????s" (partly illegible)
	PORT_DIPSETTING(0x00, "4")
	PORT_DIPSETTING(0x10, "6")
	PORT_DIPSETTING(0x20, "8")
	PORT_DIPSETTING(0x30, "10")
	PORT_DIPNAME(0xc0, 0xc0, "Bonus Balls") PORT_DIPLOCATION("SW1:2,1") // "Bonos Bola Extra" - "Nº de Bonos"
	PORT_DIPSETTING(0xc0, "6")
	PORT_DIPSETTING(0x80, "8")
	PORT_DIPSETTING(0x40, "10")
	PORT_DIPSETTING(0x00, "12")

	PORT_START("DSW2") // "Micro Swicher Nº 2"
	PORT_DIPNAME(0x03, 0x03, "Scoring") PORT_DIPLOCATION("SW2:8,7") // "Tanteo"
	PORT_DIPSETTING(0x03, "800k / 1.4M / 2.0M / 2.6M") // = "Bola Extra," "1ª Partida," "2ª Partida," "High Score"
	PORT_DIPSETTING(0x02, "1.0M / 1.6M / 2.2M / 2.8M")
	PORT_DIPSETTING(0x01, "1.2M / 1.8M / 2.4M / 3.0M")
	PORT_DIPSETTING(0x00, "1.4M / 2.0M / 2.6M / 3.2M")
	PORT_DIPNAME(0x04, 0x04, "Balls/Game") PORT_DIPLOCATION("SW2:6") // "Bolas-Partida"
	PORT_DIPSETTING(0x04, "3")
	PORT_DIPSETTING(0x00, "5")
	PORT_DIPNAME(0x18, 0x18, DEF_STR(Coinage)) PORT_DIPLOCATION("SW2:5,4") // "Monederos"
	PORT_DIPSETTING(0x18, "25 Pts. = 1, 100 Pts. = 5")
	PORT_DIPSETTING(0x10, "25 Pts. = 1, 100 Pts. = 6")
	PORT_DIPSETTING(0x08, "25 Pts. = 2, 100 Pts. = 8")
	PORT_DIPSETTING(0x00, "25 Pts. = 1/2, 100 Pts. = 3")
	PORT_DIPNAME(0x20, 0x20, "Lottery") PORT_DIPLOCATION("SW2:3") // "Loteria"
	PORT_DIPSETTING(0x00, DEF_STR(No))
	PORT_DIPSETTING(0x20, DEF_STR(Yes))
	PORT_DIPNAME(0x40, 0x40, "Musica-Reclamo") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x00, DEF_STR(No))
	PORT_DIPSETTING(0x40, DEF_STR(Yes))
	PORT_DIPNAME(0x80, 0x80, "Extra Ball/Score") PORT_DIPLOCATION("SW2:1") // "Bola Extra-Tanteo"
	PORT_DIPSETTING(0x00, DEF_STR(No))
	PORT_DIPSETTING(0x80, DEF_STR(Yes))

	PORT_START("DSW3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED) // SW3 not populated

	PORT_START("X0")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_TILT)

	PORT_START("X1")
	PORT_START("X2")
	PORT_START("X3")
	PORT_START("X4")
	PORT_START("X5")
INPUT_PORTS_END

static INPUT_PORTS_START( papillon )
	PORT_INCLUDE( break86 ) // differences unknown
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER( vd_state::irq )
{
	if (m_t_c > 40)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
	else
		m_t_c++;
}

WRITE8_MEMBER( vd_state::disp_w )
{
	segment[offset] = data;
#if 0 // probably not how this works
	if (!offset)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
#endif
}

WRITE8_MEMBER( vd_state::col_w )
{
	if (data != 0x3f)
	{
		data &= 7;
		output().set_digit_value(data + 11, segment[0]);
		output().set_digit_value(data + 21, segment[1]);
		output().set_digit_value(data + 31, segment[2]);
		output().set_digit_value(data + 41, segment[3]);
		output().set_digit_value(data + 51, segment[4]);
	}
}

void vd_state::machine_reset()
{
	m_t_c = 0;
}

MACHINE_CONFIG_START(vd_state::vd)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(vd_map)
	MCFG_CPU_IO_MAP(vd_io)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", vd_state, irq, attotime::from_hz(484))

	/* Sound */
	genpin_audio(config);
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ay1", AY8910, 2000000) //?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.33/3)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW2"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW1"))
	MCFG_SOUND_ADD("ay2", AY8910, 2000000) //?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.33/3)
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW3")) //?

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_vd)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Break '86 (1986)
/
/ The title of this game is somewhat uncertain. The backglass says
/ only "Break," the flyer also calls it "Super Break" and "Super
/ Break '86", and the service manual's title page has "Modbreak."
/-------------------------------------------------------------------*/
ROM_START(break86)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("break1.cpu", 0x0000, 0x2000, CRC(c187d263) SHA1(1790566799ccc41cd5445936e86f945150e24e8a))
	ROM_LOAD("break2.cpu", 0x2000, 0x2000, CRC(ed8f84ab) SHA1(ff5d7e3c373ca345205e8b92c6ce7b02f36a3d95))
	ROM_LOAD("break3.cpu", 0x4000, 0x2000, CRC(3cdfedc2) SHA1(309fd04c81b8facdf705e6297c0f4d507957ae1f))
ROM_END

/*-------------------------------------------------------------------
/ Papillon (1986)
/-------------------------------------------------------------------*/
ROM_START(papillon)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u4.dat", 0x0000, 0x2000, CRC(e57bfcdd) SHA1(d0d5c798552a2436693dfee0e2ebf4b6f465b194))
	ROM_LOAD("u5.dat", 0x2000, 0x2000, CRC(6d2ef02a) SHA1(0b67b2edd85624531630c162ae31af8078be01e3))
	ROM_LOAD("u6.dat", 0x4000, 0x2000, CRC(6b2867b3) SHA1(720fe8a65b447e839b0eb9ea21e0b3cb0e50cf7a))
ROM_END


GAME(1986, break86,  0,    vd,  break86,  vd_state, 0,  ROT0,  "Video Dens", "Break '86", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, papillon, 0,    vd,  papillon, vd_state, 0,  ROT0,  "Video Dens", "Papillon",  MACHINE_IS_SKELETON_MECHANICAL)
