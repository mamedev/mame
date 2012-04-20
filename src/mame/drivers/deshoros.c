/***************************************************************************

Destiny Horoscope (c) 1983 Data East Corporation

driver by Angelo Salese

A fortune-teller machine with 24 characters LED-array and a printer.

TODO:
-Emulate the graphics with genuine artwork display;
-Printer emulation;

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"


class deshoros_state : public driver_device
{
public:
	deshoros_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_io_ram(*this, "io_ram"){ }

	required_shared_ptr<UINT8> m_io_ram;
	char m_led_array[21];
	UINT8 m_bank;
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
	void update_led_array(UINT8 new_data);
};


/*Temporary,to show something on screen...*/

static VIDEO_START( deshoros )
{
	deshoros_state *state = machine.driver_data<deshoros_state>();
	UINT8 i;
	for(i=0;i<20;i++)
		state->m_led_array[i] = 0x20;
	state->m_led_array[20] = 0;
}

static SCREEN_UPDATE_IND16( deshoros )
{
	deshoros_state *state = screen.machine().driver_data<deshoros_state>();
	popmessage("%s",state->m_led_array);
	return 0;
}

/*I don't know it this is 100% correct,might be different...*/
void deshoros_state::update_led_array(UINT8 new_data)
{
	UINT8 i;
	/*scroll the data*/
	for(i=0;i<19;i++)
		m_led_array[i] = m_led_array[i+1];
	/*update the data*/
	m_led_array[19] = new_data;
}


static void answer_bankswitch(running_machine &machine,UINT8 new_bank)
{
	deshoros_state *state = machine.driver_data<deshoros_state>();
	if(state->m_bank!=new_bank)
	{
		UINT8 *ROM = state->memregion("data")->base();
		UINT32 bankaddress;

		state->m_bank = new_bank;
		bankaddress = 0 + 0x6000 * state->m_bank;
		state->membank("bank1")->set_base(&ROM[bankaddress]);
	}
}

READ8_MEMBER(deshoros_state::io_r)
{
	switch(offset)
	{
		case 0x00: return 0xff; //printer read
		case 0x03: return input_port_read(machine(), "KEY0" );
		case 0x04: return input_port_read(machine(), "KEY1" );
		case 0x05: return input_port_read(machine(), "SYSTEM" );
		case 0x0a: return m_io_ram[offset]; //"buzzer" 0 read
		case 0x0b: return m_io_ram[offset]; //"buzzer" 1 read
	}
//  printf("R -> [%02x]\n",offset);

	return m_io_ram[offset];
}

WRITE8_MEMBER(deshoros_state::io_w)
{
	switch(offset)
	{
		case 0x00: /*Printer data*/						return;
		case 0x02: update_led_array(data);              return;
		case 0x05: coin_lockout_w(machine(), 0,m_io_ram[offset] & 1);return;
		case 0x06: /*Printer IRQ enable*/   		    return;
//      case 0x0a: "buzzer" 0 write
//      case 0x0b: "buzzer" 1 write
		case 0x0c: answer_bankswitch(machine(),data&0x03); return; //data & 0x10 enabled too,dunno if it is worth to shift the data...
	}
	m_io_ram[offset] = data;
//  printf("%02x -> [%02x]\n",data,offset);
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, deshoros_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x900f) AM_READWRITE(io_r,io_w) AM_SHARE("io_ram") //i/o area
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( deshoros )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Key Male") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 )  PORT_NAME("Key 3")  PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )  PORT_NAME("Key 2")  PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )  PORT_NAME("Key 1")  PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Key Female") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 )  PORT_NAME("Key 6")  PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 )  PORT_NAME("Key 5")  PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )  PORT_NAME("Key 4")  PORT_CODE(KEYCODE_4_PAD)
	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Key 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON9 )  PORT_NAME("Key 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON8 )  PORT_NAME("Key 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 )  PORT_NAME("Key 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Key Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("Key Cancel") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_DIPNAME( 0x01,   0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02,   0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04,   0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08,   0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10,   0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x10, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_DIPNAME( 0xc0,   0x00, "Operation Mode" )
	PORT_DIPSETTING(      0x00, "Normal Mode" )
	PORT_DIPSETTING(      0x80, "Test Mode" )
	PORT_DIPSETTING(      0xc0, "I/O Test" )
	//                    0x40, Normal Mode again
INPUT_PORTS_END

/*Is it there an IRQ mask?*/
static INTERRUPT_GEN( deshoros_irq )
{
	cputag_set_input_line(device->machine(), "maincpu", M6809_IRQ_LINE, HOLD_LINE);
}

static MACHINE_RESET( deshoros )
{
	deshoros_state *state = machine.driver_data<deshoros_state>();
	state->m_bank = -1;
}

static MACHINE_CONFIG_START( deshoros, deshoros_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809,2000000)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT("screen",deshoros_irq)

	MCFG_MACHINE_RESET(deshoros)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(48*8, 16*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0*8, 16*8-1)
	MCFG_SCREEN_UPDATE_STATIC(deshoros)
	MCFG_PALETTE_LENGTH(16)

	MCFG_VIDEO_START(deshoros)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( deshoros )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ag12-4",   0xc000, 0x2000, CRC(03b2c850) SHA1(4e2c49a8d80bc559d0f406caddddb85bc107aac0) )
	ROM_LOAD( "ag13-4",   0xe000, 0x2000, CRC(36959ef6) SHA1(9b3ed44416fcda6a8e89d11ad6e713abd4f63d83) )

	ROM_REGION( 0x18000, "data", 0 ) //answers data
	ROM_LOAD( "ag00",   0x00000, 0x2000, CRC(77f5bce0) SHA1(20b5257710c5e848893fec107f0d87a473a4ba24) )
	ROM_LOAD( "ag01",   0x02000, 0x2000, CRC(c08e6a74) SHA1(88679ed8bd2b6b8698258baddf8433c0f60a1b64) )
	ROM_LOAD( "ag02",   0x04000, 0x2000, CRC(687c72b5) SHA1(3f2768c9b6247e96d11b4159f6f5c0dfeb2c5075) )
	ROM_LOAD( "ag03",   0x06000, 0x2000, CRC(535dbe83) SHA1(29336539c57d1fa7d42a0ce01884b29e1707e9ad) )
	ROM_LOAD( "ag04",   0x08000, 0x2000, CRC(e6ae8eb7) SHA1(d0e20e438dcfeac9d844d1fd98701a443ea5e4f7) )
	ROM_LOAD( "ag05",   0x0a000, 0x2000, CRC(c2485e40) SHA1(03f6d7c63a45d430a7965e28aaf07e053ecac7a1) )
	ROM_LOAD( "ag06",   0x0c000, 0x2000, CRC(e6e0bbd1) SHA1(fe693d038b05ae18a3c0cfb25a4649dbb10ab2c7) )
	ROM_LOAD( "ag07",   0x0e000, 0x2000, CRC(a62d879d) SHA1(94d07e774df4c9e4e34ae386714372b53b255530) )
	ROM_LOAD( "ag08",   0x10000, 0x2000, CRC(f5822738) SHA1(afe53e875057317033cdd5f4b7614c96cd11193b) )
	ROM_LOAD( "ag09",   0x12000, 0x2000, CRC(ad3c9f2c) SHA1(f665efb65c072a3d3d2e19844ebe0b352c0251d3) )
	ROM_LOAD( "ag10",   0x14000, 0x2000, CRC(c498754a) SHA1(90e215e8e41d32237d1f4b074d93e20eade92e4e) )
	ROM_LOAD( "ag11",   0x16000, 0x2000, CRC(5f7bf9f9) SHA1(281f89c0bccfcc2bdc1d4d0a5b9cc9a8ab2e7869) )
ROM_END

GAME( 1983, deshoros,  0,       deshoros,  deshoros,  0, ROT0, "Data East Corporation", "Destiny Horoscope", GAME_NO_SOUND | GAME_NOT_WORKING )
