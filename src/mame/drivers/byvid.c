/**************************************************************

    PINBALL
    Bally MPU AS-2518-133
    Baby Pacman
    Granny & the Gators
    A blend of arcade video game, and pinball.

ToDo:
- No sound
- No inputs
- Mechanical
- Artwork
- Beeper needs to be replaced by a red LED when artwork gets done.

***************************************************************/


#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "video/tms9928a.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "sound/beep.h"

class by133_state : public driver_device
{
public:
	by133_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_videocpu(*this, "videocpu")
		, m_audiocpu(*this, "audiocpu")
		, m_pia_u7(*this, "pia_u7")
		, m_pia_u10(*this, "pia_u10")
		, m_pia_u11(*this, "pia_u11")
		, m_beep(*this, "beeper")
	{ }

	DECLARE_READ8_MEMBER(sound_data_r);
	DECLARE_WRITE8_MEMBER(sound_data_w);
	DECLARE_READ8_MEMBER(m6803_port2_r);
	DECLARE_WRITE8_MEMBER(m6803_port2_w);
	DECLARE_INPUT_CHANGED_MEMBER(video_test);
	DECLARE_INPUT_CHANGED_MEMBER(sound_test);
	DECLARE_INPUT_CHANGED_MEMBER(activity_test);
	DECLARE_INPUT_CHANGED_MEMBER(self_test);
	DECLARE_READ8_MEMBER(u7_a_r);
	DECLARE_WRITE8_MEMBER(u7_a_w);
	DECLARE_READ8_MEMBER(u7_b_r);
	DECLARE_WRITE8_MEMBER(u7_b_w);
	DECLARE_READ8_MEMBER(u10_a_r);
	DECLARE_WRITE8_MEMBER(u10_a_w);
	DECLARE_READ8_MEMBER(u10_b_r);
	DECLARE_WRITE8_MEMBER(u10_b_w);
	DECLARE_READ8_MEMBER(u11_a_r);
	DECLARE_WRITE8_MEMBER(u11_a_w);
	DECLARE_READ8_MEMBER(u11_b_r);
	DECLARE_WRITE8_MEMBER(u11_b_w);
	DECLARE_WRITE_LINE_MEMBER(u7_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u10_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u11_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u7_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(u10_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(u11_cb2_w);
	TIMER_DEVICE_CALLBACK_MEMBER(u10_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(u11_timer);
private:
	UINT8 m_mpu_to_vid;
	UINT8 m_vid_to_mpu;
	UINT8 m_u7_a;
	UINT8 m_u7_b;
	UINT8 m_u10_a;
	UINT8 m_u10_b;
	bool m_u10_cb2;
	UINT8 m_u11_a;
	UINT8 m_u11_b;
	bool m_u10_timer;
	bool m_u11_timer;
	virtual void machine_reset();
	required_device<m6800_cpu_device> m_maincpu;
	required_device<m6809e_device> m_videocpu;
	required_device<m6803_cpu_device> m_audiocpu;
	required_device<pia6821_device> m_pia_u7;
	required_device<pia6821_device> m_pia_u10;
	required_device<pia6821_device> m_pia_u11;
	optional_device<beep_device> m_beep;
};


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, by133_state ) // U9 MPU
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM // 128x8 in MC6810 U7 MPU
	AM_RANGE(0x0088, 0x008b) AM_DEVREADWRITE("pia_u10", pia6821_device, read, write) // PIA U10 MPU
	AM_RANGE(0x0090, 0x0093) AM_DEVREADWRITE("pia_u11", pia6821_device, read, write) // PIA U11 MPU
	AM_RANGE(0x0200, 0x03ff) AM_RAM // 256x4 in 5101L U8 MPU, battery backed (D4-7 are data, A4-8 are address)
	AM_RANGE(0x1000, 0x17ff) AM_ROM AM_REGION("roms", 0x0000)
	AM_RANGE(0x1800, 0x1fff) AM_ROM AM_REGION("roms", 0x1000)
	AM_RANGE(0x5000, 0x57ff) AM_ROM AM_REGION("roms", 0x0800)
	AM_RANGE(0x5800, 0x5fff) AM_ROM AM_REGION("roms", 0x1800)
	AM_RANGE(0x7000, 0x7fff) AM_ROM AM_REGION("roms", 0x1000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( video_map, AS_PROGRAM, 8, by133_state ) // U8 Vidiot
	AM_RANGE(0x0000, 0x1fff) AM_READWRITE(sound_data_r,sound_data_w)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE("pia_u7", pia6821_device, read, write) // PIA U7 Vidiot
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x0ffe) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0x4001, 0x4001) AM_MIRROR(0x0ffe) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE(0x6000, 0x63ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, by133_state ) // U27 Vidiot
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, by133_state )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_DEVWRITE("dac", dac_device, write_unsigned8) // P10-P17
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(m6803_port2_r, m6803_port2_w) // P20-P24
ADDRESS_MAP_END


INPUT_CHANGED_MEMBER( by133_state::video_test )
{
	if(newval)
		m_videocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( by133_state::sound_test )
{
	if(newval)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

// doesn't appear to do anything
INPUT_CHANGED_MEMBER( by133_state::activity_test )
{
	if(newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( by133_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

static INPUT_PORTS_START( by133 )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Video Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, video_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Sound Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, sound_test, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Activity") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, activity_test, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Self Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, self_test, 0)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "S01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "S08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "S09")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "S16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "S17")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S18")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S19")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S20")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S21")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S22")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S23")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "S24")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "S25")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S26")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S27")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S28")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S29")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S31")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "S32")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))
INPUT_PORTS_END


READ8_MEMBER( by133_state::sound_data_r )
{//printf("%X ",m_mpu_to_vid);
	return m_mpu_to_vid;
}

WRITE8_MEMBER( by133_state::sound_data_w )
{
	m_vid_to_mpu = data;
}

READ8_MEMBER( by133_state::m6803_port2_r )
{
	//printf("%X %s\n",m_u7_b,machine().describe_context());
	//machine().scheduler().synchronize();
	return (m_u7_b << 1) | 0;
}

WRITE8_MEMBER( by133_state::m6803_port2_w )
{
	//m_u7_b = data >> 1;
	m_beep->set_frequency(600);
	m_beep->set_state(BIT(data, 0));
}

WRITE_LINE_MEMBER( by133_state::u7_ca2_w )
{
	// comms out
}

WRITE_LINE_MEMBER( by133_state::u10_ca2_w )
{
	// enable digital display
}

WRITE_LINE_MEMBER( by133_state::u11_ca2_w )
{
	// green led
}

WRITE_LINE_MEMBER( by133_state::u7_cb2_w )
{
	// red led
	m_beep->set_frequency(950);
	m_beep->set_state(state);
	//address_space &space = m_audiocpu->space(AS_PROGRAM);
	//m_audiocpu->m6801_io_r(space, 3, ((m_u7_b << 1) | (UINT8)state) );
	m_audiocpu->set_input_line(M6801_TIN_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( by133_state::u10_cb2_w )
{
	// lamp strobe #1
	m_u10_cb2 = state;
}

WRITE_LINE_MEMBER( by133_state::u11_cb2_w )
{
	// solenoid-sound selector
}

READ8_MEMBER( by133_state::u7_a_r )
{
	return m_u7_a;
}

WRITE8_MEMBER( by133_state::u7_a_w )
{
	m_u7_a = data;
}

READ8_MEMBER( by133_state::u7_b_r )
{
	return m_u7_b;
}

WRITE8_MEMBER( by133_state::u7_b_w )
{
	//machine().scheduler().synchronize();
	m_u7_b = data;
}

READ8_MEMBER( by133_state::u10_a_r )
{
	return m_u10_a;
}

WRITE8_MEMBER( by133_state::u10_a_w )
{
	m_u10_a = data;
	if (BIT(m_u11_a, 2) == 0)
		m_mpu_to_vid = data ^ 0x0f;
}

READ8_MEMBER( by133_state::u10_b_r )
{
	if (BIT(m_u11_a, 3) == 0)
		return ~m_u7_a & 0x03;

	if (BIT(m_u11_a, 1) == 0)
		return m_vid_to_mpu;

	UINT8 data = 0;

	if (BIT(m_u10_a, 5))
		data |= ioport("DSW0")->read();

	if (BIT(m_u10_a, 6))
		data |= ioport("DSW1")->read();

	if (BIT(m_u10_a, 7))
		data |= ioport("DSW2")->read();

	if (m_u10_cb2)
		data |= ioport("DSW3")->read();

	return data;
}

WRITE8_MEMBER( by133_state::u10_b_w )
{
	m_u10_b = data;
}

READ8_MEMBER( by133_state::u11_a_r )
{
	return m_u11_a;
}

WRITE8_MEMBER( by133_state::u11_a_w )
{
	m_u11_a = data;
	m_pia_u7->ca1_w(BIT(data, 1));
  	m_pia_u7->ca2_w(BIT(data, 2));
}

READ8_MEMBER( by133_state::u11_b_r )
{
	return m_u11_b;
}

WRITE8_MEMBER( by133_state::u11_b_w )
{
	m_u11_b = data;
}

// zero-cross detection
TIMER_DEVICE_CALLBACK_MEMBER( by133_state::u10_timer )
{
	m_u10_timer ^= 1;
	m_pia_u10->cb1_w(m_u10_timer);
}	

// 555 timer for display refresh
TIMER_DEVICE_CALLBACK_MEMBER( by133_state::u11_timer )
{
	m_u11_timer ^= 1;
	m_pia_u11->ca1_w(m_u11_timer);
}

void by133_state::machine_reset()
{
	m_u7_a = 0;
	m_u7_b = 1; // select mode 2 of mc6803 on /reset
	m_u10_a = 0;
	m_u10_b = 0;
	m_u10_cb2 = 0;
	m_u11_a = 0;
	m_u11_b = 0;
	m_mpu_to_vid = 0;
	m_vid_to_mpu = 0;
	m_beep->set_state(0);
}

static MACHINE_CONFIG_START( by133, by133_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_3_579545MHz/4) // no xtal, just 2 chips
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_CPU_ADD("videocpu", M6809E, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(video_map)

	MCFG_CPU_ADD("audiocpu", M6803, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)

	MCFG_DEVICE_ADD("pia_u7", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(by133_state, u7_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(by133_state, u7_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(by133_state, u7_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(by133_state, u7_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(by133_state, u7_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(by133_state, u7_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("videocpu", m6809e_device, firq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("videocpu", m6809e_device, firq_line))

	MCFG_DEVICE_ADD("pia_u10", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(by133_state, u10_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(by133_state, u10_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(by133_state, u10_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(by133_state, u10_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(by133_state, u10_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(by133_state, u10_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("babypac1", by133_state, u10_timer, attotime::from_hz(120)) // mains freq

	MCFG_DEVICE_ADD("pia_u11", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(by133_state, u11_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(by133_state, u11_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(by133_state, u11_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(by133_state, u11_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(by133_state, u11_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(by133_state, u11_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("babypac2", by133_state, u11_timer, attotime::from_hz(634)) // 555 timer

	/* video hardware */
	MCFG_DEVICE_ADD( "tms9928a", TMS9928A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(DEVWRITELINE("videocpu", m6809e_device, irq_line))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SPEAKER_STANDARD_MONO("beee")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "beee", 0.10)
MACHINE_CONFIG_END

/*-----------------------------------------------------
/ Baby Pacman (Video/Pinball Combo) (BY133-891:  10/82)
/-----------------------------------------------------*/
ROM_START(babypac)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "891-u2.732", 0x0000, 0x1000, CRC(7f7242d1) SHA1(213a697bb7fc69f93ea04621f0fcfdd796f35196))
	ROM_LOAD( "891-u6.732", 0x1000, 0x1000, CRC(6136d636) SHA1(c01a0a2fcad3bdabd649128e012ab558b1c90cd3) )

	ROM_REGION(0x10000, "videocpu", 0)
	ROM_LOAD( "891-16-u09.764", 0x8000, 0x2000, CRC(781e90e9) SHA1(940047cc875ae531a825af069bb650d59c9495a6))
	ROM_LOAD( "891-11-u10.764", 0xa000, 0x2000, CRC(28f4df8b) SHA1(bd6a3598c2c90b5a3a59327616d2f5b9940d98bc))
	ROM_LOAD( "891-05-u11.764", 0xc000, 0x2000, CRC(0a5967a4) SHA1(26d56ddea3f39d41e382449007bf7ba113c0285f))
	ROM_LOAD( "891-06-u12.764", 0xe000, 0x2000, CRC(58cfe542) SHA1(e024d14019866bd460d1da6b901f9b786a76a181))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD( "891-12-u29.764", 0xe000, 0x2000, CRC(0b57fd5d) SHA1(43a03e6d16c87c3305adb04722484f992f23a1bd))
ROM_END

ROM_START(babypac2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "891-u2.732", 0x0000, 0x1000, CRC(7f7242d1) SHA1(213a697bb7fc69f93ea04621f0fcfdd796f35196))
	ROM_LOAD( "891-u6.732", 0x1000, 0x1000, CRC(6136d636) SHA1(c01a0a2fcad3bdabd649128e012ab558b1c90cd3) )

	ROM_REGION(0x10000, "videocpu", 0)
	ROM_LOAD( "891-13-u09.764", 0x8000, 0x2000, CRC(7fa570f3) SHA1(423ad9266b1ded00fa52ce4180d518874142a203))
	ROM_LOAD( "891-11-u10.764", 0xa000, 0x2000, CRC(28f4df8b) SHA1(bd6a3598c2c90b5a3a59327616d2f5b9940d98bc))
	ROM_LOAD( "891-05-u11.764", 0xc000, 0x2000, CRC(0a5967a4) SHA1(26d56ddea3f39d41e382449007bf7ba113c0285f))
	ROM_LOAD( "891-06-u12.764", 0xe000, 0x2000, CRC(58cfe542) SHA1(e024d14019866bd460d1da6b901f9b786a76a181))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD( "891-12-u29.764", 0xe000, 0x2000, CRC(0b57fd5d) SHA1(43a03e6d16c87c3305adb04722484f992f23a1bd))
ROM_END

/*-----------------------------------------------------------------
/ Granny and the Gators (Video/Pinball Combo) - (BY35-???: 01/84)
/----------------------------------------------------------------*/
ROM_START(granny)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "cpu_u2.532", 0x0000, 0x1000, CRC(d45bb956) SHA1(86a6942ff9fe38fa109ecde40dc2dd19adf938a9))
	ROM_LOAD( "cpu_u6.532", 0x1000, 0x1000, CRC(306aa673) SHA1(422c3d9decf9214a18edb536c2077bf52b272e7d) )

	ROM_REGION(0x10000, "videocpu", 0)
	ROM_LOAD( "vid_u4.764", 0x4000, 0x2000, CRC(3a3d4c6b) SHA1(a6c27eee178a4bde67004e11f6ddf3b6414571dd))
	ROM_LOAD( "vid_u5.764", 0x6000, 0x2000, CRC(78bcb0fb) SHA1(d9dc1cc1bef063d5fbdbf2d1daf793234a9c55a0))
	ROM_LOAD( "vid_u6.764", 0x8000, 0x2000, CRC(8d8220a6) SHA1(64aa7d6ef2702c1b9afc61528434caf56cb91396))
	ROM_LOAD( "vid_u7.764", 0xa000, 0x2000, CRC(aa71cf29) SHA1(b69cd4060f5d4d2a7f85d901552cdc987013fde2))
	ROM_LOAD( "vid_u8.764", 0xc000, 0x2000, CRC(a442bc01) SHA1(2c01123dc5799561ae9e7c5d6db588b82b5ae59c))
	ROM_LOAD( "vid_u9.764", 0xe000, 0x2000, CRC(6b67a1f7) SHA1(251c2b941898363bbd6ee1a94710e2b2938ec851))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD( "cs_u3.764", 0xe000, 0x2000, CRC(0a39a51d) SHA1(98342ba38e48578ce9870f2ee85b553d46c0e35f))
ROM_END


GAME( 1982, babypac,  0,        by133,  by133, driver_device,  0,  ROT90, "Dave Nutting Associates / Bally", "Baby Pac-Man (set 1)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1982, babypac2, babypac,  by133,  by133, driver_device,  0,  ROT90, "Dave Nutting Associates / Bally", "Baby Pac-Man (set 2)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1984, granny,   0,        by133,  by133, driver_device,  0,  ROT0,  "Bally", "Granny and the Gators", GAME_IS_SKELETON_MECHANICAL)
