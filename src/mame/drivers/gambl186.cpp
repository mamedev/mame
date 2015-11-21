// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese, Roberto Fresca, Peter Ferrie
/***********************************************************************************

  Multi Game - EGD, 1997
  Poker - Roulette - Black Jack - Bingo

  Game base is Bingo. Casino 10 (poker), Roulette, and Black Jack,
  could be enabled/disabled through DIP switches.

************************************************************************************

80186xl20   Xtal 40Mhz
At89c52 (not dumped) with external 32K ram?? Xtal 11.xxx18Mhz
Cirrus Logic CL-GD5428-80QC-A video chip with 416c256 ram near it Xtal 14.x818 Mhaz

U9-U10 programm rom common to both pcb
u11-u12 Program rom/GFX
U13-U14 256Kramx8 (32Kbyte x16)

U23 Nec D71055C
U28 Nec D71055C
U22 Nec D71055C
U42 Nec D71051C
U500 Nec D71051C (not present on board "1")

U3 Max691cpe

U300 Nec D7759GC (10Mhz xtal near it)

In order to get the game to run, follow these steps:
- enable service mode
- run all of the tests (press '1' repeatedly to progress)
- at the "cancellare la memoria" prompt, press 'X' to allow it
- when the line of '*' appears, press the buttons in the following sequence:
    1 1 X X 1 1 1 X X X 1 1 X 1 1 1 1 X 1 1 1 1 1 X 1 1 1 1 1 1 X
(This is the "password" for the EEPROM.  You have three attempts to get it correct, otherwise the service mode will restart)
- if accepted, disable service mode DIP switch
- reset machine (press 'F3') (watchdog is not implemented)
- if another password prompt appears, just press 'X' 9 times

TODO:
- fix the poker game (casino 10). EEPROM behaviour still buggy.
- sound;
- proper 3x D71055C emulation.

***********************************************************************************/



#include "emu.h"
#include "cpu/i86/i186.h"
#include "video/clgd542x.h"
#include "sound/upd7759.h"
#include "machine/nvram.h"


class gambl186_state : public driver_device
{
public:
	gambl186_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_upd7759(*this, "7759") { }

	required_device<cpu_device> m_maincpu;
	optional_device<upd7759_device> m_upd7759;
	int m_comms_state;
	int m_comms_ind;
	UINT8 m_comms_data[1002];
	int m_comms_cmd;
	int m_comms_expect;
	int m_comms_blocks;
	bool m_comms_ack;

	virtual void machine_start();
	DECLARE_READ16_MEMBER(comms_r);
	DECLARE_WRITE16_MEMBER(comms_w);
	DECLARE_WRITE16_MEMBER(data_bank_w);
	DECLARE_READ16_MEMBER(upd_r);
	DECLARE_WRITE16_MEMBER(upd_w);
};

void gambl186_state::machine_start()
{
	membank("data_bank")->configure_entries(0, 4, memregion("data")->base(), 0x40000);
}

static const UINT8 password[] = {5, 2, 0, 3, 0, 0, 2, 4, 5, 6, 0x16};

READ16_MEMBER(gambl186_state::comms_r)
{
	UINT16 retval = 0;

	if ((offset == 0) && ACCESSING_BITS_0_7) //port 680 == data
	{
		if (m_comms_state == 0x16) //read mode, just in case
		{
			if (!m_comms_ind && (m_comms_cmd == 0xff))
			{
				m_comms_cmd = m_comms_data[1];

				switch (m_comms_cmd)
				{
					case 0:
					{
						m_comms_expect = 4;
						break;
					}

					case 1:
					{
						m_comms_expect = 12;
						m_comms_blocks = 2;
						break;
					}

					case 2:
					{
						m_comms_expect = 408;
						m_comms_blocks = 4;
						break;
					}

					case 3:
					{
						m_comms_expect = 7;
						m_comms_blocks = 3;
						break;
					}

					case 4:
					{
						m_comms_expect = 2;
						m_comms_blocks = 2;
						break;
					}

					case 5:
					{
						m_comms_expect = 13;
						m_comms_blocks = 4;
						break;
					}

					case 6:
					{
						m_comms_expect = 1003;
						break;
					}

					default: //unknown
					{
						m_comms_expect = 1;
					}
				}
			}

			if (m_comms_ind < sizeof(m_comms_data))
			{
				if (m_comms_expect && !--m_comms_expect)
				{
					m_comms_ack = true;

					if (m_comms_ind)
					{
						int i, sum;

						for (i = 1, sum = 0; i < m_comms_ind; sum += m_comms_data[i++]);
						m_comms_data[m_comms_ind] = (unsigned char) sum;

						switch (m_comms_cmd)
						{
							case 1:
							{
								if (m_comms_blocks == 2)
								{
									m_comms_expect = 5;
								}
								else
								{
									m_comms_data[m_comms_ind] += 5; //compensate for ack
								}

								break;
							}

							case 2:
							{
								if (m_comms_blocks == 4)
								{
									m_comms_expect = 5;
								}
								else
								{
									m_comms_data[m_comms_ind] += 5; //compensate for ack
									m_comms_expect = 3;
								}

								break;
							}

							case 3:
							{
								if (m_comms_blocks == 3)
								{
									m_comms_expect = 3;
								}
								else
								{
									m_comms_data[m_comms_ind] += 5; //compensate for ack
									m_comms_expect = 5;
								}

								break;
							}

							case 5:
							{
								m_comms_expect = 3;

								if (m_comms_blocks < 4)
								{
									m_comms_data[m_comms_ind] += 5; //compensate for ack

									if (m_comms_blocks == 2)
									{
										m_comms_expect = 2;
									}
								}

								break;
							}

							default:
							{
							}
						}
					}
					else if (m_comms_cmd == 4)
					{
						if (!memcmp(m_comms_data, password, sizeof(password)))
						{
							m_comms_data[1] = 0x55;
							m_comms_data[2] = 0x55;
						}

						m_comms_expect = 2;
						m_comms_ack = false;
					}

					if (!m_comms_blocks || !--m_comms_blocks)
					{
						m_comms_cmd = 0xff;
					}
				}

				retval = m_comms_data[m_comms_ind++];
			}
		}
	}
	else if (offset == 1) //port 681 == status
	{
		if (m_comms_state == 0x16) //read mode
		{
			retval = 2; //read ready
		}
		else if (m_comms_state == 0x31) //write mode
		{
			retval = 4; //write ready
		}
	}

	return retval;
}

WRITE16_MEMBER(gambl186_state::comms_w)
{
	if (offset == 0)
	{
		if ((m_comms_state == 0x31) && (m_comms_ind < 1000))
		{
			if (!m_comms_ack || (data == 0x15)) //validation failure
			{
				if (m_comms_cmd == 6) //1000 bytes transfer
				{
					data = ~data;
				}
				else if (m_comms_ack)
				{
					m_comms_cmd = 0xfe;
					m_comms_expect = 2;
					data = 5;
				}

				m_comms_data[++m_comms_ind] = (UINT8) data;
			}

			m_comms_ack = false;
		}
	}
	else if (offset == 1)
	{
		if (m_comms_state != data) //detect transition
		{
			m_comms_ind = 0;

			if (data == 0x4e) //reset
			{
				m_comms_data[0] = 5; //operation complete
				m_comms_cmd = 0xff; //none
				m_comms_expect = 0;
				m_comms_blocks = 0;
				m_comms_ack = false;
			}
		}

		m_comms_state = data;
	}
}

WRITE16_MEMBER( gambl186_state::data_bank_w)
{
	membank("data_bank")->set_entry(data & 3);
	if(data & 0xfffc)
		popmessage("warning: set %04x to data bank",data);
}


/* Preliminary sound through UPD7759

   port 400h writes the sample index/input
   port 504h writes the commands...

   504h xxxx ---- ---- ----

   Sound event:
   504h: 2000
   504h: e000
   400h: xxyy <--- xx and yy are the sample number/index
   504h: c000
   504h: e000
   504h: 2000
*/
WRITE16_MEMBER(gambl186_state::upd_w)
{
//// FIXME
//  m_upd7759->reset_w(0);
//  m_upd7759->reset_w(1);

//  if (mem_mask&0x00ff) m_upd7759->port_w(space, 0, data & 0xff);
//  if (mem_mask&0xff00) m_upd7759->port_w(space, 0, (data >> 8) & 0xff);
	data = (data >> 8);
	popmessage("sample index: %02x", data);

//  m_upd7759->start_w(0);
//  m_upd7759->start_w(1);
}

static ADDRESS_MAP_START( gambl186_map, AS_PROGRAM, 16, gambl186_state )
	AM_RANGE(0x00000, 0x0ffff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x40000, 0x7ffff) AM_ROMBANK("data_bank")
	AM_RANGE(0xa0000, 0xbffff) AM_DEVREADWRITE8("vga", cirrus_gd5428_device, mem_r, mem_w, 0xffff)
	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION("ipl",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gambl186_io, AS_IO, 16, gambl186_state )
	AM_RANGE(0x03b0, 0x03bf) AM_DEVREADWRITE8("vga", cirrus_gd5428_device, port_03b0_r, port_03b0_w, 0xffff)
	AM_RANGE(0x03c0, 0x03cf) AM_DEVREADWRITE8("vga", cirrus_gd5428_device, port_03c0_r, port_03c0_w, 0xffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("vga", cirrus_gd5428_device, port_03d0_r, port_03d0_w, 0xffff)
	AM_RANGE(0x0400, 0x0401) AM_WRITE(upd_w)      // upd7759 sample index/input
	AM_RANGE(0x0500, 0x0501) AM_READ_PORT("IN0")
	AM_RANGE(0x0502, 0x0503) AM_READ_PORT("IN1")
	AM_RANGE(0x0504, 0x0505) AM_READ_PORT("IN2")  // Seems to writes more upd7759 params in MSB...

	//AM_RANGE(0x0500, 0x050f) AM_READ(unk_r)
	AM_RANGE(0x0580, 0x0581) AM_READ_PORT("DSW1")
	AM_RANGE(0x0582, 0x0583) AM_READ_PORT("JOY")
	AM_RANGE(0x0584, 0x0585) AM_READ_PORT("DSW0") AM_WRITENOP // Watchdog: bit 8
//  AM_RANGE(0x0600, 0x0603) AM_WRITENOP // lamps
	AM_RANGE(0x0680, 0x0683) AM_READWRITE(comms_r, comms_w)
	AM_RANGE(0x0700, 0x0701) AM_WRITE(data_bank_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( gambl186 )
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("-")  PORT_CODE(KEYCODE_2)  // Unknown meaning

	PORT_START("IN1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN5 )  PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN6 )  PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Key")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("- Aux")  PORT_CODE(KEYCODE_3)  // Unknown meaning
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("JOY")
	PORT_BIT( 0x01ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE(0x1000, IP_ACTIVE_LOW )               PORT_DIPLOCATION("SW1:4")
	PORT_DIPNAME( 0x2000, 0x0000, "Casino 10 Game" )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Bookkeeping" )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "SW1-1" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0100, 0x0100, "SW2-4" )            PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "SW2-3" )            PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "SW2-2" )            PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "VGA Mode" )         PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x0800, "640x480" )
	PORT_DIPSETTING(    0x0000, "640x240" )
	PORT_DIPNAME( 0x1000, 0x0000, "Roulette Game" )    PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "SW2-6" )            PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, "Black Jack Game" )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "SW2-8" )            PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

INPUT_PORTS_END



static MACHINE_CONFIG_START( gambl186, gambl186_state )
	MCFG_CPU_ADD("maincpu", I80186, XTAL_40MHz)
	MCFG_CPU_PROGRAM_MAP(gambl186_map)
	MCFG_CPU_IO_MAP(gambl186_io)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_FRAGMENT_ADD( pcvideo_cirrus_gd5428 )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("7759", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

MACHINE_CONFIG_END



ROM_START( gambl186 )
	ROM_REGION( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "ie398.u11", 0x00000, 0x80000, CRC(86ad7cab) SHA1(b701c3701db630d218a9b1700f216f795a1b1272) )
	ROM_LOAD16_BYTE( "io398.u12", 0x00001, 0x80000, CRC(0a036f34) SHA1(63d0b87c7d4c902413f28c0b55d78e5fda511f4f) )

	ROM_REGION( 0x40000, "ipl", 0 )
	ROM_LOAD16_BYTE( "se403p.u9",  0x00000, 0x20000, CRC(1021cc20) SHA1(d9bb67676b05458ff813d608431ff06946ab7721) )
	ROM_LOAD16_BYTE( "so403p.u10", 0x00001, 0x20000, CRC(af9746c9) SHA1(3f1ab8110cc5eadec661181779799693ad695e21) )

	ROM_REGION( 0x20000, "upd", 0 ) // upd7759 sound samples
	ROM_LOAD( "347.u302", 0x00000, 0x20000, CRC(7ce8f490) SHA1(2f856e31d189e9d46ba6b322133d99133e0b52ac) )
ROM_END

ROM_START( gambl186a )
	ROM_REGION( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "ie399.u11", 0x00000, 0x80000, CRC(2a7bce20) SHA1(fbabaaa0d72b5dfccd33f5194d13009bdc44b5a7) )
	ROM_LOAD16_BYTE( "io399.u12", 0x00001, 0x80000, CRC(9212f52b) SHA1(d970c59c1e0f5f7e94c1b632398bcfae278c143d) )

	ROM_REGION( 0x40000, "ipl", 0 )
	ROM_LOAD16_BYTE( "se403p.u9",  0x00000, 0x20000, CRC(1021cc20) SHA1(d9bb67676b05458ff813d608431ff06946ab7721) )
	ROM_LOAD16_BYTE( "so403p.u10", 0x00001, 0x20000, CRC(af9746c9) SHA1(3f1ab8110cc5eadec661181779799693ad695e21) )

	ROM_REGION( 0x20000, "upd", 0 ) // upd7759 sound samples
	ROM_LOAD( "347.u302", 0x00000, 0x20000, CRC(7ce8f490) SHA1(2f856e31d189e9d46ba6b322133d99133e0b52ac) )
ROM_END


/*    YEAR  NAME       PARENT    MACHINE   INPUT     STATE          INIT  ROT     COMPANY    FULLNAME             FLAGS... */
GAME( 1997, gambl186,  0,        gambl186, gambl186, driver_device, 0,    ROT0,  "EGD",     "Multi Game (V398)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 199?, gambl186a, gambl186, gambl186, gambl186, driver_device, 0,    ROT0,  "EGD",     "Multi Game (V399)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
