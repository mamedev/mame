// license:BSD-3-Clause
// copyright-holders:James Wallace
// thanks-to:Martin Buchholz,Juergen Oppermann,Volker Hann, Jan-Ole Christian for original driver
/***************************************************************************

      Poly-Play
      (c) 1985 by VEB Polytechnik Karl-Marx-Stadt

	  Credits:
	  - Original driver written by Martin Buchholz (buchholz@mail.uni-greifswald.de)
	  - Rewrite to add light sequencer by J. Wallace
      - Juergen Oppermann and Volker Hann offered electronic assistance, repair work and ROM dumps.
      - Jan-Ole Christian from the Videogamemuseum in Berlin provided information based on one of the last existing Poly-Play machines.
		He also provided schematics and service manuals for the original driver.

	  NOTES:
	  Hardware is based on the K1520 PC system, with video output coming through a standard Colormat TV.

	  I/O MAP:
	  
	  READ:
	  83  IN1
          used as hardware random number generator

      84  PORT A PIO (IN0)
	      used for inputs (bit 5 unused, bit 6 Bookkeeping (Summe Spiele), bit 7 Coin sensor)

	 
	  85  PORT B PIO
	      bit 0-4 = light sequencer
          bit 5-7 = sound parameter (Needs more investigation)

      86-87 PIO handler
	  
	  WRITE:
	  80        Sound Channel 1
	  81        Sound Channel 2
	  82        40 Hz timer (title screen timing)
	  83        75 Hz timer (general game behaviour)

The Poly-Play has a simple bookmarking system which can be activated
setting Bit 6 of PORTA (Summe Spiele) to low. It reads a double word
from 0c00 and displays it on the screen. The method of controlling this is unknown

TODO:
Verify sound behaviour, IRQ handling.
Fix light sequencer, currently tied directly to PIO
Other versions exist with more games (undumped)
***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "sound/samples.h"
#include "includes/polyplay.h"
#include "polyplay.lh"

WRITE8_MEMBER(polyplay_state::polyplay_sound_w)
{
	process_channel(offset,data);
}

READ8_MEMBER(polyplay_state::polyplay_porta_r)
{
	return m_in0_port->read();
}

WRITE8_MEMBER(polyplay_state::polyplay_portb_w)
{	
	int m_lights = (data & 0x1f);
	
	if (m_lights == 0x1f)
	{
		m_light_state = 0;
		output().set_lamp_value(1, 0);
		output().set_lamp_value(2, 0);
		output().set_lamp_value(3, 0);
		output().set_lamp_value(4, 0);
		m_light_timer->adjust(attotime::never, 0, attotime::never);
	}
	else if (m_lights == 0x1b)
	{
		m_light_timer->adjust(attotime::from_hz(75), 0, attotime::from_hz(75));
	}

}

TIMER_DEVICE_CALLBACK_MEMBER(polyplay_state::polyplay_timer_callback_lights)
{
	//TODO: Find out real system
	m_light_state++;
		
	switch (m_light_state)
	{
		case 0:
		{
			output().set_lamp_value(1, 1);
			output().set_lamp_value(2, 0);
			output().set_lamp_value(3, 0);
			output().set_lamp_value(4, 0);
			break;
		}
		case 3:
		{
			output().set_lamp_value(1, 0);
			output().set_lamp_value(2, 1);
			output().set_lamp_value(3, 0);
			output().set_lamp_value(4, 0);
			break;
		}
		case 6:
		{
			output().set_lamp_value(1, 0);
			output().set_lamp_value(2, 0);
			output().set_lamp_value(3, 1);
			output().set_lamp_value(4, 0);
			break;
		}
		case 9:
		{
			output().set_lamp_value(1, 0);
			output().set_lamp_value(2, 0);
			output().set_lamp_value(3, 0);
			output().set_lamp_value(4, 1);
			break;
		}
		case 12:
		{
			m_light_state=0;
			output().set_lamp_value(1, 1);
			output().set_lamp_value(2, 0);
			output().set_lamp_value(3, 0);
			output().set_lamp_value(4, 0);
			break;
		}
	}
}
	
TIMER_DEVICE_CALLBACK_MEMBER(polyplay_state::polyplay_timer_callback_75)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x4e);
}

INPUT_CHANGED_MEMBER(polyplay_state::coin_inserted)
{
	/* coin insertion causes an IRQ */
	if (oldval)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0x50);
}

TIMER_DEVICE_CALLBACK_MEMBER(polyplay_state::polyplay_timer_callback_40)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x4c);
}

WRITE8_MEMBER(polyplay_state::polyplay_timer_40hz)
{
	if (data == 0x03)
	{
		m_timer->reset();
	}
	else if (data == 0xb5)
	{
		m_timer->adjust(attotime::from_hz(40), 0, attotime::from_hz(40));
	}
}

WRITE8_MEMBER(polyplay_state::polyplay_timer_75hz)
{
	if (data == 0x80)
	{
		m_timer2->adjust(attotime::from_hz(75), 0, attotime::from_hz(75));
	}
	else if (data == 0xa5)
	{
		m_timer2->reset();
	}
	else
	{
		logerror("unmapped write to 75hz %x \n",data);
	}
}

READ8_MEMBER(polyplay_state::polyplay_in1_r)
{
	return machine().rand() & 0xff;
}


static ADDRESS_MAP_START( polyplay_map, AS_PROGRAM, 8, polyplay_state )
	AM_RANGE(0x0000, 0x0bff) AM_ROM
	AM_RANGE(0x0c00, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x8fff) AM_ROM
	AM_RANGE(0xe800, 0xebff) AM_ROM AM_REGION("gfx1", 0)
	AM_RANGE(0xec00, 0xf7ff) AM_RAM_WRITE(polyplay_characterram_w) AM_SHARE("characterram")
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( polyplay_io, AS_IO, 8, polyplay_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x81) AM_WRITE(polyplay_sound_w)
	AM_RANGE(0x82, 0x82) AM_WRITE(polyplay_timer_40hz)
	AM_RANGE(0x83, 0x83) AM_READWRITE(polyplay_in1_r,polyplay_timer_75hz)
	AM_RANGE(0x84, 0x87) AM_DEVREADWRITE("z80pio", z80pio_device, read_alt, write_alt)
ADDRESS_MAP_END

static INPUT_PORTS_START( polyplay )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bookkeeping Info") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, polyplay_state,coin_inserted, 0)
INPUT_PORTS_END

static const gfx_layout charlayout_1_bit =
{
	8,8,    /* 8*8 characters */
	128,    /* 128 characters */
	1,      /* 1 bit per pixel */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout charlayout_3_bit =
{
	8,8,    /* 8*8 characters */
	128,    /* 128 characters */
	3,      /* 3 bit per pixel */
	{ 0, 128*8*8, 128*8*8 + 128*8*8 },    /* offset for each bitplane */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( polyplay )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout_1_bit, 0, 1 )
	GFXDECODE_ENTRY( nullptr,0xec00, charlayout_3_bit, 2, 1 )
GFXDECODE_END


void polyplay_state::machine_reset()
{
	for (int i =0; i < 2; i++)
	{
		m_channel_const[i] = 0;
		play_channel(i,0);
	}
	m_timer = machine().device<timer_device>("timer");
	m_timer2 = machine().device<timer_device>("timer2");
	m_light_timer = machine().device<timer_device>("lighttimer");
	m_light_state=0;
}

static MACHINE_CONFIG_START( polyplay, polyplay_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, POLYPLAY_MAIN_CLOCK/4) /* U880 */
	MCFG_CPU_PROGRAM_MAP(polyplay_map)
	MCFG_CPU_IO_MAP(polyplay_io)
	MCFG_TIMER_DRIVER_ADD("timer", polyplay_state, polyplay_timer_callback_40)
	MCFG_TIMER_DRIVER_ADD("timer2", polyplay_state, polyplay_timer_callback_75)
	MCFG_TIMER_DRIVER_ADD("lighttimer", polyplay_state, polyplay_timer_callback_lights)
	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(polyplay_state, screen_update_polyplay)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", polyplay)
	MCFG_PALETTE_ADD("palette", 10)
	MCFG_PALETTE_INIT_OWNER(polyplay_state, polyplay)

	/* devices */
	MCFG_DEVICE_ADD("z80pio", Z80PIO, POLYPLAY_MAIN_CLOCK/4)
	MCFG_Z80PIO_IN_PA_CB(READ8(polyplay_state, polyplay_porta_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(polyplay_state, polyplay_portb_w))
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(2)
	MCFG_SAMPLES_START_CB(polyplay_state, sh_start)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

ROM_START( polyplay )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu_0000.37",       0x0000, 0x0400, CRC(87884c5f) SHA1(849c6b3f40496c694a123d6eec268a7128c037f0) )//OS
	ROM_LOAD( "cpu_0400.36",       0x0400, 0x0400, CRC(d5c84829) SHA1(baa8790e77db66e1e543b3a0e5390cc71256de2f) )//GAME
	ROM_LOAD( "cpu_0800.35",       0x0800, 0x0400, CRC(5f36d08e) SHA1(08ecf8143e818a9844b4f168e68629d6d4481a8a) )//MENU
	ROM_LOAD( "2_-_1000.14",       0x1000, 0x0400, CRC(950dfcdb) SHA1(74170d5c99d1ea61fe37d1fe023dca96efb1ca69) )//Abfahrtslauf 
	ROM_LOAD( "2_-_1400.10",       0x1400, 0x0400, CRC(829f74ca) SHA1(4df9d3c24e1bc4c2c953dce9530e43a00ecf67fc) )//Abfahrtslauf 
	ROM_LOAD( "2_-_1800.6",        0x1800, 0x0400, CRC(b69306f5) SHA1(66d7c3cf76782a5b6eafa3e1513ecc9a9df0e0e1) )//Abfahrtslauf 
	ROM_LOAD( "2_-_1c00.2",        0x1c00, 0x0400, CRC(aede2280) SHA1(0a01394ab70d07d666e955c87a08cb4d4945767e) )//Hirschjagd
	ROM_LOAD( "2_-_2000.15",       0x2000, 0x0400, CRC(6c7ad0d8) SHA1(df959d1e43fde96b5e21e3c53b397209a98ea423) )//Hirschjagd
	ROM_LOAD( "2_-_2400.11",       0x2400, 0x0400, CRC(bc7462f0) SHA1(01ca680c74b92b9ba5a85f98e0933ef1e754bfc1) )//Hirschjagd
	ROM_LOAD( "2_-_2800.7",        0x2800, 0x0400, CRC(9ccf1958) SHA1(6bdf04d7796074af7327fab6717b52736540f97c) )//Hase und Wolf
	ROM_LOAD( "2_-_2c00.3",        0x2c00, 0x0400, CRC(21827930) SHA1(71d27d68f6973a59996102381f8754d9b353c65a) )//Hase und Wolf
	ROM_LOAD( "2_-_3000.16",       0x3000, 0x0400, CRC(b3b3c0ec) SHA1(a94cd9794d59ea2f9ddd8bef86e6e3a269b276ad) )//Hase und Wolf
	ROM_LOAD( "2_-_3400.12",       0x3400, 0x0400, CRC(bd416cd0) SHA1(57391cc4a417468455b45014969067629fd629b8) )//Hase und Wolf
	ROM_LOAD( "2_-_3800.8",        0x3800, 0x0400, CRC(1c470b7c) SHA1(f7c71ee1752ecd4f30a35f14ee392b37febefb9c) )//Hase und Wolf
	ROM_LOAD( "2_-_3c00.4",        0x3c00, 0x0400, CRC(b8354a19) SHA1(58ea7798ecc1be987b1217f4078c7cb366622dd3) )//Hase und Wolf
	ROM_LOAD( "2_-_4000.17",       0x4000, 0x0400, CRC(1e01041e) SHA1(ff63e4bb924d1c26e445a28c5f8cbc696b4b9f5a) )//Schmetterlingsfang
	ROM_LOAD( "2_-_4400.13",       0x4400, 0x0400, CRC(fe4d8959) SHA1(233f97956f4c819558d5d38034d92edc0e86a0de) )//Schmetterlingsfang
	ROM_LOAD( "2_-_4800.9",        0x4800, 0x0400, CRC(c45f1d9d) SHA1(f3373f1f5a3c6099fd38e65f66e024ef042a984c) )//Schmetterlingsfang
	ROM_LOAD( "2_-_4c00.5",        0x4c00, 0x0400, CRC(26950ad6) SHA1(881f5f0f4806ba6f21d0b28a70fc43363d51419b) )//Schmetterlingsfang
	ROM_LOAD( "1_-_5000.30",       0x5000, 0x0400, CRC(9f5e2ba1) SHA1(58c696afbda8932f5e401b0a82b2de5cdfc2d1fb) )//Schiessbude
	ROM_LOAD( "1_-_5400.26",       0x5400, 0x0400, CRC(b5f9a780) SHA1(eb785b7668f6af0a9df84cbd1905173869377e6c) )//Schiessbude
	ROM_LOAD( "1_-_5800.22",       0x5800, 0x0400, CRC(d973ad12) SHA1(81cc5e19e83f2e5b10b885583c250a2ff66bafe5) )//Schiessbude
	ROM_LOAD( "1_-_5c00.18",       0x5c00, 0x0400, CRC(9c22ea79) SHA1(e25ed745589a83e297dba936a6e5979f1b31b2d5) )//Schiessbude
	ROM_LOAD( "1_-_6000.31",       0x6000, 0x0400, CRC(245c49ca) SHA1(12e5a032327fb45b2a240aff11b0c5d1798932f4) )//Autorennen
	ROM_LOAD( "1_-_6400.27",       0x6400, 0x0400, CRC(181e427e) SHA1(6b65409cd8410e632093662f5de2989dd9134620) )//Autorennen
	ROM_LOAD( "1_-_6800.23",       0x6800, 0x0400, CRC(8a6c1f97) SHA1(bf9d4dda8ac933a4a700f52540dcd1197f0a64eb) )//Autorennen
	ROM_LOAD( "1_-_6c00.19",       0x6c00, 0x0400, CRC(77901dc9) SHA1(b1132e06011aa8f7a95c43f447cd422f01139bb1) )//Autorennen
	ROM_LOAD( "1_-_7000.32",       0x7000, 0x0400, CRC(83ffbe57) SHA1(1e06408f7b4c9a4e5cadab58f6efbc03a5bedc1e) )//Autorennen
	ROM_LOAD( "1_-_7400.28",       0x7400, 0x0400, CRC(e2a66531) SHA1(1c9eb54e9c8a13f26335d8fb79fe5e39c28b3255) )//Merkspiel
	ROM_LOAD( "1_-_7800.24",       0x7800, 0x0400, CRC(1d0803ef) SHA1(15a1996f9262f26cf531f329e086b10b3c25ce92) )//Merkspiel
	ROM_LOAD( "1_-_7c00.20",       0x7c00, 0x0400, CRC(17dfa7e4) SHA1(afb471dc6cb2faccfb4305540f75162fcee3d622) )//Merkspiel
	ROM_LOAD( "1_-_8000.33",       0x8000, 0x0400, CRC(6ee02375) SHA1(fbf797b655639ee442804a30fd3a06bbf261999a) )//Wasserrohrbruch
	ROM_LOAD( "1_-_8400.29",       0x8400, 0x0400, CRC(9db09598) SHA1(8eb385542a617b23caad3ce7bbdd9714c1dd684f) )//Wasserrohrbruch
	ROM_LOAD( "1_-_8800.25",       0x8800, 0x0400, CRC(ca2f963f) SHA1(34295f02bfd1bca141d650bbbbc1989e01c67b2f) )//Wasserrohrbruch
	ROM_LOAD( "1_-_8c00.21",       0x8c00, 0x0400, CRC(0c7dec2d) SHA1(48d776b97c1eca851f89b0c5df4d5765d9aa0319) )//Wasserrohrbruch

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "char.1",            0x0000, 0x0400, CRC(5242dd6b) SHA1(ba8f317df62fe4360757333215ce3c8223c68c4e) )
ROM_END


/* game driver */
GAMEL( 1985, polyplay, 0, polyplay, polyplay, driver_device, 0, ROT0, "VEB Polytechnik Karl-Marx-Stadt", "Poly-Play", 0,layout_polyplay )
