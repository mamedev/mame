// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese, Roberto Fresca, Peter Ferrie
/***********************************************************************************

  Multi Game - EGD, 1997
  Poker - Roulette - Black Jack - Bingo

  Game base is Bingo. Casino 10 (poker), Roulette, and Black Jack,
  could be enabled/disabled through DIP switches.

************************************************************************************

80186xl20   Xtal 40MHz
At89c52 (not dumped) with external 32K ram?? Xtal 11.0592MHz
Cirrus Logic CL-GD5428-80QC-A video chip with 416c256 ram near it Xtal 14.381818 MHz

U9-U10 program rom common to both pcb
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
- proper 3x D71055C emulation
- hopper and ticket dispenser
- proper touch screen
- in gambl186c the link test can't be skipped and it doesn't seem to like current connection emulation

***********************************************************************************/



#include "emu.h"

#include "cpu/i86/i186.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"
#include "sound/upd7759.h"
#include "video/clgd542x.h"

#include "speaker.h"


namespace {

class gambl186_state : public driver_device
{
public:
	gambl186_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_upd7759(*this, "7759"),
		m_data_bank(*this, "data_bank") { }

	void gambl186(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<upd7759_device> m_upd7759;
	required_memory_bank m_data_bank;

	uint16_t m_comms_state;
	uint16_t m_comms_ind;
	uint8_t m_comms_data[1002];
	uint8_t m_comms_cmd;
	uint16_t m_comms_expect;
	uint8_t m_comms_blocks;
	bool m_comms_ack;

	uint16_t comms_r(offs_t offset, uint16_t mem_mask = ~0);
	void comms_w(offs_t offset, uint16_t data);
	void data_bank_w(uint16_t data);
	void upd_w(uint16_t data);
	void gambl186_io(address_map &map);
	void gambl186_map(address_map &map);
};

void gambl186_state::machine_start()
{
	m_data_bank->configure_entries(0, 4, memregion("data")->base(), 0x40000);

	save_item(NAME(m_comms_state));
	save_item(NAME(m_comms_ind));
	save_item(NAME(m_comms_data));
	save_item(NAME(m_comms_cmd));
	save_item(NAME(m_comms_expect));
	save_item(NAME(m_comms_blocks));
	save_item(NAME(m_comms_ack));
}

static const uint8_t password[] = {5, 2, 0, 3, 0, 0, 2, 4, 5, 6, 0x16};

uint16_t gambl186_state::comms_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t retval = 0;

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

void gambl186_state::comms_w(offs_t offset, uint16_t data)
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

				m_comms_data[++m_comms_ind] = (uint8_t) data;
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

void gambl186_state::data_bank_w(uint16_t data)
{
	m_data_bank->set_entry(data & 3);
	if (data & 0xfffc)
		popmessage("warning: set %04x to data bank", data);
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
void gambl186_state::upd_w(uint16_t data)
{
//// FIXME
//  m_upd7759->reset_w(0);
//  m_upd7759->reset_w(1);

//  if (ACCESSING_BITS_0_7) m_upd7759->port_w(data & 0xff);
//  if (ACCESSING_BITS_8_15) m_upd7759->port_w((data >> 8) & 0xff);
	data = (data >> 8);
	popmessage("sample index: %02x", data);

//  m_upd7759->start_w(0);
//  m_upd7759->start_w(1);
}

void gambl186_state::gambl186_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram().share("nvram");
	map(0x40000, 0x7ffff).bankr(m_data_bank);
	map(0xa0000, 0xbffff).rw("vga", FUNC(cirrus_gd5428_device::mem_r), FUNC(cirrus_gd5428_device::mem_w));
	map(0xc0000, 0xfffff).rom().region("ipl", 0);
}

void gambl186_state::gambl186_io(address_map &map)
{
	map(0x03b0, 0x03bf).rw("vga", FUNC(cirrus_gd5428_device::port_03b0_r), FUNC(cirrus_gd5428_device::port_03b0_w));
	map(0x03c0, 0x03cf).rw("vga", FUNC(cirrus_gd5428_device::port_03c0_r), FUNC(cirrus_gd5428_device::port_03c0_w));
	map(0x03d0, 0x03df).rw("vga", FUNC(cirrus_gd5428_device::port_03d0_r), FUNC(cirrus_gd5428_device::port_03d0_w));
	map(0x0400, 0x0401).w(FUNC(gambl186_state::upd_w));      // upd7759 sample index/input
	map(0x0500, 0x0501).portr("IN0");
	map(0x0502, 0x0503).portr("IN1");
	map(0x0504, 0x0505).portr("IN2");  // Seems to writes more upd7759 params in MSB...

	//map(0x0500, 0x050f).r(FUNC(gambl186_state::unk_r));
	map(0x0580, 0x0581).portr("DSW1");
	map(0x0582, 0x0583).portr("JOY");
	map(0x0584, 0x0585).portr("DSW0").nopw(); // Watchdog: bit 8
//  map(0x0600, 0x0603).nopw(); // lamps
	map(0x0680, 0x0683).rw(FUNC(gambl186_state::comms_r), FUNC(gambl186_state::comms_w));
	map(0x0700, 0x0701).w(FUNC(gambl186_state::data_bank_w));
}



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

static INPUT_PORTS_START( gambl186c )
	PORT_INCLUDE( gambl186)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Language) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x2000, DEF_STR( French ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( English ) )
INPUT_PORTS_END

void gambl186_state::gambl186(machine_config &config)
{
	I80186(config, m_maincpu, XTAL(40'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &gambl186_state::gambl186_map);
	m_maincpu->set_addrmap(AS_IO, &gambl186_state::gambl186_io);

	AT89C52(config, "mcu", XTAL(11'059'200)).set_disable(); // no dump available

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800),900,0,640,526,0,480);
	screen.set_screen_update("vga", FUNC(cirrus_gd5428_device::screen_update));

	cirrus_gd5428_device &vga(CIRRUS_GD5428(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x200000);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	UPD7759(config, m_upd7759);
	m_upd7759->add_route(ALL_OUTPUTS, "mono", 0.75);
}



ROM_START( gambl186 ) // has Casino 10, Roulette, 21 Blackjack and Bingo 10 selection
	ROM_REGION( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "ie398.u11", 0x00000, 0x80000, CRC(86ad7cab) SHA1(b701c3701db630d218a9b1700f216f795a1b1272) )
	ROM_LOAD16_BYTE( "io398.u12", 0x00001, 0x80000, CRC(0a036f34) SHA1(63d0b87c7d4c902413f28c0b55d78e5fda511f4f) )

	ROM_REGION16_LE( 0x40000, "ipl", 0 )
	ROM_LOAD16_BYTE( "se403p.u9",  0x00000, 0x20000, CRC(1021cc20) SHA1(d9bb67676b05458ff813d608431ff06946ab7721) )
	ROM_LOAD16_BYTE( "so403p.u10", 0x00001, 0x20000, CRC(af9746c9) SHA1(3f1ab8110cc5eadec661181779799693ad695e21) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "89c52.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x20000, "upd", 0 ) // upd7759 sound samples
	ROM_LOAD( "347.u302", 0x00000, 0x20000, CRC(7ce8f490) SHA1(2f856e31d189e9d46ba6b322133d99133e0b52ac) )
ROM_END

ROM_START( gambl186a ) // has Casino 10, Roulette, 21 Blackjack and Bingo 10 selection
	ROM_REGION( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "ie399.u11", 0x00000, 0x80000, CRC(2a7bce20) SHA1(fbabaaa0d72b5dfccd33f5194d13009bdc44b5a7) )
	ROM_LOAD16_BYTE( "io399.u12", 0x00001, 0x80000, CRC(9212f52b) SHA1(d970c59c1e0f5f7e94c1b632398bcfae278c143d) )

	ROM_REGION16_LE( 0x40000, "ipl", 0 )
	ROM_LOAD16_BYTE( "se403p.u9",  0x00000, 0x20000, CRC(1021cc20) SHA1(d9bb67676b05458ff813d608431ff06946ab7721) )
	ROM_LOAD16_BYTE( "so403p.u10", 0x00001, 0x20000, CRC(af9746c9) SHA1(3f1ab8110cc5eadec661181779799693ad695e21) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "89c52.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x20000, "upd", 0 ) // upd7759 sound samples
	ROM_LOAD( "347.u302", 0x00000, 0x20000, CRC(7ce8f490) SHA1(2f856e31d189e9d46ba6b322133d99133e0b52ac) )
ROM_END


ROM_START( gambl186b )
	ROM_REGION( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "ie3.7.8.bin", 0x00000, 0x80000, CRC(cc27886c) SHA1(cb27af74dffe86c564ba8a0ad711f4232330cf1b) )
	ROM_LOAD16_BYTE( "io3.7.8.bin", 0x00001, 0x80000, CRC(c69bf3ad) SHA1(eb612e903a9b184c2dd363e081ee8f650a4f2f90) )

	ROM_REGION16_LE( 0x40000, "ipl", 0 )
	ROM_LOAD16_BYTE( "se3.8.6t.bin", 0x00000, 0x20000, CRC(158bd3a3) SHA1(846f382f145f8c4c36bd75fef12717b41e91c70b) )
	ROM_LOAD16_BYTE( "so3.8.6t.bin", 0x00001, 0x20000, CRC(4bd275d3) SHA1(6b84f54e723408b06b71e89f7de6a8014fd9ecfd) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "89c52.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x20000, "upd", 0 ) // upd7759 sound samples
	ROM_LOAD( "sound3.9.8.bin", 0x00000, 0x20000, CRC(1d6d1743) SHA1(df0e8d311ccaf77fb5dfc341124a11051154e79c) )
ROM_END


ROM_START( gambl186c ) // labels unreadable or hand written with just a number. Has Super Lolo, Morpion, Solitaire and Jumping Frog selection
	ROM_REGION( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "3", 0x00000, 0x80000, CRC(782d6aa2) SHA1(8b8cfc41a10c79f43c56c90ed6dc280a54da2a18) )
	ROM_LOAD16_BYTE( "4", 0x00001, 0x80000, CRC(75053488) SHA1(a63b312325cc1bd73283f5387219e58c7ba368d8) )

	ROM_REGION16_LE( 0x40000, "ipl", 0 )
	ROM_LOAD16_BYTE( "1", 0x00000, 0x20000, CRC(3478d935) SHA1(439fa07d6eb0e77356d1fd1713aeed5580a2dccb) )
	ROM_LOAD16_BYTE( "2", 0x00001, 0x20000, CRC(6f708e48) SHA1(8e9157599cf340713b67b6e03fa60c7a536175c0) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "89c52.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x20000, "upd", 0 ) // upd7759 sound samples
	ROM_LOAD( "5", 0x00000, 0x20000, CRC(1d6d1743) SHA1(df0e8d311ccaf77fb5dfc341124a11051154e79c) )
ROM_END


ROM_START( gambl186d ) // labels unreadable or hand written with just a number. Has Super Lolo, Morpion, Solitaire and Jumping Frog selection
	ROM_REGION( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "3", 0x00000, 0x80000, CRC(782d6aa2) SHA1(8b8cfc41a10c79f43c56c90ed6dc280a54da2a18) )
	ROM_LOAD16_BYTE( "4", 0x00001, 0x80000, CRC(75053488) SHA1(a63b312325cc1bd73283f5387219e58c7ba368d8) )

	ROM_REGION16_LE( 0x40000, "ipl", 0 )
	ROM_LOAD16_BYTE( "1", 0x00000, 0x20000, CRC(97a900cd) SHA1(1dd65080fe366f74c63456f7ecc0121252bea6e2) )
	ROM_LOAD16_BYTE( "2", 0x00001, 0x20000, CRC(b5e61416) SHA1(55d64250cd9855405b76f9a7027f89edb27dadf1) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "89c52.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x20000, "upd", 0 ) // upd7759 sound samples
	ROM_LOAD( "5", 0x00000, 0x20000, CRC(1d6d1743) SHA1(df0e8d311ccaf77fb5dfc341124a11051154e79c) )
ROM_END

} // anonymous namespace


// version numbering isn't clear, rom labels don't agree with test mode display.
GAME( 1997, gambl186,  0,        gambl186, gambl186,  gambl186_state, empty_init, ROT0, "EGD", "Multi Game (Italian, Versione 4.0.3 - 1.5.7, 05-FEV-99(397)) (V398?)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Versione 4.0.3 (1.5.7), csmb15A, CSMB_0015A (IT), - 05-FEV-99(397)
GAME( 1997, gambl186a, gambl186, gambl186, gambl186,  gambl186_state, empty_init, ROT0, "EGD", "Multi Game (Italian, Versione 4.0.3 - 1.5.7, 05-FEV-99(397)) (V399?)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // same?
GAME( 1997, gambl186b, gambl186, gambl186, gambl186,  gambl186_state, empty_init, ROT0, "EGD", "Multi Game (Italian, Versione 3.8.6T - 1.5.6, 25-AUG-97) (V378?)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Versione 3.8.6T (1.5.6), mult5_it, CSMB-0000F (IT), 25-AUG-97
GAME( 2000, gambl186c, gambl186, gambl186, gambl186c, gambl186_state, empty_init, ROT0, "EGD", "Multi Game (French / English, Version Soft 4.4.8T - 4.0.5, 26-OCT-00)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Version Soft 4.4.8T (4.0.5), CSMB-0020E (HX) - hdpr-hx - Oct 26 2000 - 15:01:24
GAME( 2000, gambl186d, gambl186, gambl186, gambl186,  gambl186_state, empty_init, ROT0, "EGD", "Multi Game (English, Version Soft 4.1.2T - 1.5.7, 16-MAY-00(397))",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Version 4.1.2T (1.5.7), carapor2, SLC_PORT_2_VII (POR) - 16-MAY-00(397)
