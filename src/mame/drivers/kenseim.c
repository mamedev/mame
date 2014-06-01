/* Kensei Mogura
  aka Street Fighter II Whac-a-mole game */

/*
  this game uses a CPS1 board as the 'Video board'
  the main game is driven by an additional PCB containing a TMPZ84C011 and MB89363B
  moles + hammer are physical non-video parts

  https://www.youtube.com/watch?v=wk71y7OGU-k
  https://www.youtube.com/watch?v=mV00MMyBBXM
  https://www.youtube.com/watch?v=yQpMvRL0FfM



  Additional PCB  (todo, ascii layout)

  --------------------------
  |
  |
  | KENSEI MOGURA
  |  9401-TS280
  | TOGO JAPAN
  |

*/

// note: I've kept this code out of cps1.c as there is likely to be a substantial amount of game specific code here ones all the extra hardware is emulated

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "includes/cps1.h"
#include "machine/i8255.h"
#include "kenseim.lh"

class kenseim_state : public cps_state
{
public:
	kenseim_state(const machine_config &mconfig, device_type type, const char *tag)
		: cps_state(mconfig, type, tag) { }

	/* kenseim */
	DECLARE_READ16_MEMBER(cps1_kensei_r);
	DECLARE_WRITE16_MEMBER(cps1_kensei_w);
	DECLARE_READ16_MEMBER(kensei_dsw_r);
	DECLARE_DRIVER_INIT(kenseim);

	DECLARE_READ8_MEMBER(porta_default_r) { logerror("%s read port A but no handler assigned\n", machine().describe_context()); return 0x00; }
	DECLARE_READ8_MEMBER(portb_default_r) { logerror("%s read port B but no handler assigned\n", machine().describe_context()); return 0x00; }
//	DECLARE_READ8_MEMBER(portc_default_r) { logerror("%s read port C but no handler assigned\n", machine().describe_context()); return 0xff; }
	DECLARE_READ8_MEMBER(portc_r);
//	DECLARE_READ8_MEMBER(portd_default_r) { logerror("%s read port D but no handler assigned\n", machine().describe_context()); return 0xff; }
	DECLARE_READ8_MEMBER(portd_r);
	DECLARE_READ8_MEMBER(porte_default_r) { logerror("%s read port E but no handler assigned\n", machine().describe_context()); return 0xff; }

	DECLARE_WRITE8_MEMBER(porta_default_w) { logerror("%s write %02x to port A but no handler assigned\n", machine().describe_context(), data); }
	DECLARE_WRITE8_MEMBER(portb_default_w) { logerror("%s write %02x to port B but no handler assigned\n", machine().describe_context(), data); }
	DECLARE_WRITE8_MEMBER(portc_default_w) { logerror("%s write %02x to port C but no handler assigned\n", machine().describe_context(), data); }
	DECLARE_WRITE8_MEMBER(portd_default_w) { logerror("%s write %02x to port D but no handler assigned\n", machine().describe_context(), data); }
	DECLARE_WRITE8_MEMBER(porte_default_w) { logerror("%s write %02x to port E but no handler assigned\n", machine().describe_context(), data); }

	DECLARE_READ8_MEMBER(i8255_porta_default_r) { logerror("%s i8255 read port A but no handler assigned\n", machine().describe_context()); return 0xff; }
	DECLARE_READ8_MEMBER(i8255_portb_default_r) { logerror("%s i8255 read port B but no handler assigned\n", machine().describe_context()); return 0xff; }
	DECLARE_READ8_MEMBER(i8255_portc_default_r) { logerror("%s i8255 read port C but no handler assigned\n", machine().describe_context()); return 0xff; }

	DECLARE_WRITE8_MEMBER(i8255_porta_default_w) { logerror("%s i8255 write %02x to port A but no handler assigned\n", machine().describe_context(), data); } // maybe molesa output? (6-bits?)
	DECLARE_WRITE8_MEMBER(i8255_portb_default_w) { logerror("%s i8255 write %02x to port B but no handler assigned\n", machine().describe_context(), data); } // maybe molesb output? (6-bits?)
//	DECLARE_WRITE8_MEMBER(i8255_portc_default_w) { logerror("%s i8255 write %02x to port C but no handler assigned\n", machine().describe_context(), data); } // leds??
	DECLARE_WRITE8_MEMBER(i8255_portc_w);

	DECLARE_READ8_MEMBER(i8255_portd_default_r) { logerror("%s i8255 read port D but no handler assigned\n", machine().describe_context()); return 0xff; }
	DECLARE_READ8_MEMBER(i8255_porte_default_r) { logerror("%s i8255 read port E but no handler assigned\n", machine().describe_context()); return 0xff; }
	DECLARE_READ8_MEMBER(i8255_portf_default_r) { logerror("%s i8255 read port F but no handler assigned\n", machine().describe_context()); return 0xff; }

	DECLARE_WRITE8_MEMBER(i8255_portd_default_w) { logerror("%s i8255 write %02x to port D but no handler assigned\n", machine().describe_context(), data); }
	DECLARE_WRITE8_MEMBER(i8255_porte_default_w) { logerror("%s i8255 write %02x to port E but no handler assigned\n", machine().describe_context(), data); }
	DECLARE_WRITE8_MEMBER(i8255_portf_default_w) { logerror("%s i8255 write %02x to port F but no handler assigned\n", machine().describe_context(), data); }


	UINT32 m_led_serial_data;
	int m_led_clock;
	int m_led_latch;
	void set_leds(UINT32 ledstates);
};

void kenseim_state::set_leds(UINT32 ledstates)
{
	for (int i=0; i<20; i++)
		output_set_lamp_value(i+1, ((ledstates & (1 << i)) != 0));	
}

// could be wrong
WRITE8_MEMBER(kenseim_state::i8255_portc_w)
{
	// I'm guessing these are the 20 'power meter' LEDs, 10 for each player? (it writes 42 tiles, with the last write being some terminator?)

//	printf("%s i8255 write %02x to port C but no handler assigned (serial data?)\n", machine().describe_context(), data);

	if (data & 0x08)
	{

		if (data & 0x02)
		{
			if (data & 0x04)
			{
				// send and reset? maybe?
				//printf("led write reset?\n");
				m_led_latch = 0;
				set_leds(m_led_serial_data);
				m_led_serial_data = 0;
			}
			else if (!(m_led_clock & 0x02))
			{
				//printf("write data bit %d\n", m_led_latch & 1);
				m_led_serial_data = (m_led_serial_data << 1) | (m_led_latch & 1);
			}

		}
		else
		{
			m_led_latch = data & 0x5;
			//printf("set latch %02x\n", m_led_latch);
		}

		m_led_clock = data & 0x02;
	}

}


READ8_MEMBER(kenseim_state::portd_r)
{
	// comms port maybe? checks for 0x10 (bit 4,a) to be clear in a tight loop (092B) then for bit 0x80 to be set in another tight loop  (0933) then at (0947) it checks that bits 0xe0 aren't set.
	logerror("%s read port D\n", machine().describe_context());
	return 0x00;// rand();// 0x80;
}

READ8_MEMBER(kenseim_state::portc_r)
{
	// bits 0x09 checked at 1171
	logerror("%s read port C\n", machine().describe_context());

	return 0x00;//
	//return 0x09;// rand();
}


READ16_MEMBER(kenseim_state::cps1_kensei_r)
{
	//
	static int i = 0;
	
	int ret;

	ret = ((i & 0xf0) >> 4) | ((i & 0x0f) << 8);

	if (mem_mask & 0xff00) i++;

	logerror("%s cps1_kensei_r offs %04x, (%04x) (68k reading command port %04x)\n", machine().describe_context(), offset *2, mem_mask, ret);

	return ret | 0xf0f0;
}

WRITE16_MEMBER(kenseim_state::cps1_kensei_w)
{
	if (mem_mask == 0xff00)
	{

		data >>= 8;;

		logerror("%s cps1_kensei_w offs %04x, %02x (from 68k?)\n", machine().describe_context(), offset * 2, data);

		if ((data != 0x02) && (data != 0x03) && (data != 0x04) && (data != 0x05) && (data != 0x83))
			logerror("  ^^ (unknown?)\n");
	}
	else
	{
		logerror("%s cps1_kensei_w offs %04x, %04x (%04x) (other byte)\n", machine().describe_context(), offset * 2, data, mem_mask);
	}
}

/*
    Manufacturer: Fujitsu
    Part Number: MB89363 / MB89363B / MB89363R
    Package: Surface Mount QFP80 / QFP64P (MB89363R)
    Description: 8-bit x 3 x 2 (6 x 8-bit) parallel data I/O port VLSI chip
                 Parallel Communication Interface
                 Extended I/O

    Note: MB89363B is compatible with 8255

    Pin Assignment:
                                          +5v                                          
                         P P P P P P P P P V   P P P P P P P P P                       
                     N N 5 4 4 4 4 4 4 4 4 C N 1 1 1 1 1 1 1 1 2 N N                   
                     C C 3 0 1 2 3 4 5 6 7 C C 7 6 5 4 3 2 1 0 3 C C                   
                                                                                       
                     | | ^ ^ ^ ^ ^ ^ ^ ^ ^ | | ^ ^ ^ ^ ^ ^ ^ ^ ^ | |                   
                     | | | | | | | | | | | | | | | | | | | | | | | |                   
                     | | v v v v v v v v v | | v v v v v v v v v | |                   
                .-------------------------------------------------------.              
                |    6 6 6 6 6 5 5 5 5 5 5 5 5 5 5 4 4 4 4 4 4 4 4 4    |              
                |    4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1    |              
      P52   <-> | 65                                                 40 | <->    P22   
      P51   <-> | 66                                                 39 | <->    P21   
      P50   <-> | 67                                                 38 | <->    P20   
      P54   <-> | 68                                                 37 | <->    P24   
      P55   <-> | 69                                                 36 | <->    P25   
      P56   <-> | 70                                                 35 | <->    P26   
      P57   <-> | 71                                                 34 | <->    P27   
       NC   --- | 72                   MB89363B                      33 | ---    NC    
       NC   --- | 73                                                 32 | <--    RSLCT1
      GND   --> | 74                                                 31 | <--    RSLCT0
      CS2   --> | 75                                                 30 | <--    GND   
        R   --> | 76                                                 29 | <--    CS1   
      P30   <-> | 77                                                 28 | <->    P00   
      P31   <-> | 78                                                 27 | <->    P01   
      P32   <-> | 79                                                 26 | <->    P02   
      P33   <-> | 80                                                 25 | <->    P03   
                 \                     1 1 1 1 1 1 1 1 1 1 2 2 2 2 2    |              
                  \  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4    |              
                   -----------------------------------------------------'              
                     ^ ^ ^ ^ ^ ^ | | ^ | ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ |                   
                     | | | | | | | | | | | | | | | | | | | | | | | |                   
                     v v v v | | | | | | | v v v v v v v v v v v v |                   
                                                                                       
                     P P P P W R N N R N O D D D D D D D D P P P P N                   
                     3 3 3 3   S C C H C U B B B B B B B B 0 0 0 0 C                   
                     4 5 6 7   T     /   S 0 1 2 3 4 5 6 7 7 6 5 4                     
                                     R   /                                             
                                     L   I                                             
                                         N                                             
                                         S                                             

    Block Diagram / Pin Descriptions:
    http://www.mess.org/_media/datasheets/fujitsu/mb89363b_partial.pdf

    D.C. Characteristics:
    (Recommended operating conditions unless otherwise noted)
    (VCC = +5V +- 10%, GND = 0V, TA = -40o C to 85o C)
                                         Value
    Parameter            Symbol      Min       Max            Unit      Test Condition
    ----------------------------------------------------------------------------------
    Input Low Voltage    ViL         -0.3      0.8            V
    Input High Voltage   ViH         2.2       VCC +0.3       V
    Output Low Voltage   VoL         -         0.4            V         IoL = 2.5mA
    Output High Voltage  VoH         3.0       -              V         IoH =-2.5mA

    Sources:
    http://www.emb-tech.co.jp/pc104/96dio.pdf
    http://www.pb5800.com/resources/2350ser01.pdf
    http://www.diagramasde.com/diagramas/otros2/TS-850S%20Service%20Manual%20.pdf
*/

static ADDRESS_MAP_START( kenseim_map, AS_PROGRAM, 8, kenseim_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM

//	AM_RANGE(0x8000, 0x81ff) AM_RAM // ? size unknown, code just wipes ram until the compare fails

	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kenseim_io_map, AS_IO, 8, kenseim_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("gamecpu_ctc", z80ctc_device, read, write)

	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("i8255",   i8255_device, read, write)
	AM_RANGE(0x24, 0x27) AM_DEVREADWRITE("i8255_2", i8255_device, read, write)

ADDRESS_MAP_END


static const z80_daisy_config daisy_chain_gamecpu[] =
{
	{ "gamecpu_ctc" },
	{ NULL }
};

READ16_MEMBER(kenseim_state::kensei_dsw_r)
{


	static const char *const dswname[] = { "IN0", "DSWA", "DSWB", "DSWC" };
	
	if (offset > 0)
	{
		//logerror("%s kensei_dsw_r offs %04x, (%04x)\n", machine().describe_context(), offset *2, mem_mask);

		int in = ioport(dswname[offset])->read();
		return (in << 8) | 0xff;
	}
	else
	{  // connected to the other board instead of IN0? (or at least some bits are)

		static int togglecount = 0;
		
		togglecount++;



		int in = 0x00;
		in |= 0x40; // don't want cps1 test mode (leftover)
		
		if (togglecount == 3)
		{
			in |= 0x10; // won't read commands otherwise?
			togglecount = 0;
		}	
		
		
		//in |= 0x20;

		logerror("%s kensei_dsw_r offs %04x (comms?), (%04x) (returning %02x)\n", machine().describe_context(), offset *2, mem_mask, in);


		return (in << 8) | 0xff;

	}
}


static MACHINE_CONFIG_DERIVED_CLASS( kenseim, cps1_12MHz, kenseim_state )

	MCFG_CPU_ADD("gamecpu", TMPZ84C011, XTAL_16MHz/2) // tmpz84c011 - divider unknown
	MCFG_CPU_PROGRAM_MAP(kenseim_map)
	MCFG_CPU_IO_MAP(kenseim_io_map)
	MCFG_TMPZ84C011_PORTA_WRITE_CALLBACK(WRITE8(kenseim_state, porta_default_w))
	MCFG_TMPZ84C011_PORTB_WRITE_CALLBACK(WRITE8(kenseim_state, portb_default_w))
	MCFG_TMPZ84C011_PORTC_WRITE_CALLBACK(WRITE8(kenseim_state, portc_default_w))
	MCFG_TMPZ84C011_PORTD_WRITE_CALLBACK(WRITE8(kenseim_state, portd_default_w))
	MCFG_TMPZ84C011_PORTE_WRITE_CALLBACK(WRITE8(kenseim_state, porte_default_w))	
	MCFG_TMPZ84C011_PORTA_READ_CALLBACK(READ8(kenseim_state, porta_default_r))
	MCFG_TMPZ84C011_PORTB_READ_CALLBACK(READ8(kenseim_state, portb_default_r))
	MCFG_TMPZ84C011_PORTC_READ_CALLBACK(READ8(kenseim_state, portc_r))
	MCFG_TMPZ84C011_PORTD_READ_CALLBACK(READ8(kenseim_state, portd_r))
	MCFG_TMPZ84C011_PORTE_READ_CALLBACK(READ8(kenseim_state, porte_default_r))
	MCFG_CPU_CONFIG(daisy_chain_gamecpu)

	MCFG_DEVICE_ADD("gamecpu_ctc", Z80CTC, XTAL_16MHz/2 ) // part of the tmpz84?
	MCFG_Z80CTC_INTR_CB(INPUTLINE("gamecpu", INPUT_LINE_IRQ0))
	
	// the MB89363B seems to be 2 * i8255?
	MCFG_DEVICE_ADD("i8255", I8255, 0) // MB89363B!
	MCFG_I8255_IN_PORTA_CB(READ8(kenseim_state, i8255_porta_default_r))
	MCFG_I8255_IN_PORTB_CB(READ8(kenseim_state, i8255_portb_default_r))
	MCFG_I8255_IN_PORTC_CB(READ8(kenseim_state, i8255_portc_default_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(kenseim_state, i8255_porta_default_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(kenseim_state, i8255_portb_default_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(kenseim_state, i8255_portc_w))

	MCFG_DEVICE_ADD("i8255_2", I8255, 0) // MB89363B!
	MCFG_I8255_IN_PORTA_CB(READ8(kenseim_state, i8255_portd_default_r))
	MCFG_I8255_IN_PORTB_CB(READ8(kenseim_state, i8255_porte_default_r))
	MCFG_I8255_IN_PORTC_CB(READ8(kenseim_state, i8255_portf_default_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(kenseim_state, i8255_portd_default_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(kenseim_state, i8255_porte_default_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(kenseim_state, i8255_portf_default_w))


	MCFG_QUANTUM_PERFECT_CPU("maincpu")
MACHINE_CONFIG_END


static INPUT_PORTS_START( kenseim )
	// the regular CPS1 input ports are used for comms with the extra board
	PORT_START("IN0") 
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	// most of the regular CPS1 dips are unused  
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "CPSA SW(A):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "CPSA SW(A):2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "CPSA SW(A):3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "CPSA SW(A):4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "CPSA SW(A):5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "CPSA SW(A):6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "CPSA SW(A):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "CPSA SW(A):8" )

	PORT_START("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "CPSA SW(B):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "CPSA SW(B):2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "CPSA SW(B):3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "CPSA SW(B):4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "CPSA SW(B):5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "CPSA SW(B):6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "CPSA SW(B):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "CPSA SW(B):8" )

	PORT_START("DSWC")                         
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "CPSA SW(C):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "CPSA SW(C):2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "CPSA SW(C):3" )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("CPSA SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("CPSA SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("CPSA SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "CPSA SW(C):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "CPSA SW(C):8" )

	// the extra board has 2 dip banks used for most game options
	PORT_START("DSW1")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPUNUSED( 0x01, 0x01 )
	PORT_DIPUNUSED( 0x02, 0x02 )
	PORT_DIPUNUSED( 0x04, 0x04 )
	PORT_DIPUNUSED( 0x08, 0x08 )
	PORT_DIPUNUSED( 0x10, 0x10 )
	PORT_DIPUNUSED( 0x20, 0x20 )
	PORT_DIPUNUSED( 0x40, 0x40 )
	PORT_DIPUNUSED( 0x80, 0x80 )
INPUT_PORTS_END

ROM_START( kenseim )
	ROM_REGION( 0x400000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "knm_23.8f",  0x000000, 0x80000, CRC(f8368900) SHA1(07a8e9fffcf7be6cb154b60a0559211bc7127c5d) )
	ROM_LOAD16_WORD_SWAP( "knm_21.6f",  0x100000, 0x80000, CRC(a8025e91) SHA1(24cd3f34ae96947a1101e5f5cb6cf0d1c1d66dc0) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "knm_01.3a",  0x000000, 0x80000, CRC(923f0c0c) SHA1(2569543ba33900d1e9c7c3981c8fe1cb40743546) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_02.4a",  0x000002, 0x80000, CRC(fa694f67) SHA1(b1ffbeaba71619e9b52f1f50abc7dafe2f3332b1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_03.5a",  0x000004, 0x80000, CRC(af7af02c) SHA1(ce2e0c696b50e4806f25fc69bf4455048c9fa396) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_04.6a",  0x000006, 0x80000, CRC(607a9af4) SHA1(78862e37c1fa727d9e36099e87ee17dfa9d2498f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_05.7a",  0x200000, 0x80000, CRC(d877eee9) SHA1(d63e123fa6c1f9927cec3cf93474f31729348fd5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_06.8a",  0x200002, 0x80000, CRC(8821a281) SHA1(216305421783baa20994eec33e26537f69f34fcb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_07.9a",  0x200004, 0x80000, CRC(00306d09) SHA1(581c4ba6f9eb3050d6bf989016532457314441e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_08.10a", 0x200006, 0x80000, CRC(4a329d16) SHA1(60d66cec8c226ef49890d8b2cd82d798dfefa049) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_10.3c", 0x400000, 0x80000, CRC(ca93a942) SHA1(1f293617e6f202054690035ebe6b6e45ffe68cc9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_11.4c", 0x400002, 0x80000, CRC(a91f3091) SHA1(7cddcd30aa6a561ce297b877611ffabfac10be28) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_12.5c", 0x400004, 0x80000, CRC(5da8303a) SHA1(de30149e323f7892bb9967a98a0d3cd9c261dc69) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "knm_13.6c", 0x400006, 0x80000, CRC(889bb671) SHA1(c7952ed801343e79c06be8ed765a293e7322307b) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "knm_09.12a",  0x00000, 0x08000, CRC(15394dd7) SHA1(d96413cc8fa6cd3cfdafb2ab6305e41cfd2b8874) )
	ROM_CONTINUE(            0x10000, 0x18000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "knm_18.11c",  0x00000, 0x20000, CRC(9e3e4773) SHA1(6e750a9610fabc4bf4964b5a754414d612d43dec) )
	ROM_LOAD( "knm_19.12c",  0x20000, 0x20000, CRC(d6c4047f) SHA1(1259a3cbfc14c348ce4bd87b5de5e97ad252f7fb) )

	// to do, verify these are the correct plds

	ROM_REGION( 0x0200, "aboardplds", ROMREGION_ERASE00 )
	ROM_LOAD( "buf1",         0x0000, 0x0117, CRC(eb122de7) SHA1(b26b5bfe258e3e184f069719f9fd008d6b8f6b9b) )
	ROM_LOAD( "ioa1",         0x0000, 0x0117, CRC(59c7ee3b) SHA1(fbb887c5b4f5cb8df77cec710eaac2985bc482a6) )
	ROM_LOAD( "prg1",         0x0000, 0x0117, CRC(f1129744) SHA1(a5300f301c1a08a7da768f0773fa0fe3f683b237) )
	ROM_LOAD( "rom1",         0x0000, 0x0117, CRC(41dc73b9) SHA1(7d4c9f1693c821fbf84e32dd6ef62ddf14967845) )
	ROM_LOAD( "sou1",         0x0000, 0x0117, CRC(84f4b2fe) SHA1(dcc9e86cc36316fe42eace02d6df75d08bc8bb6d) )

	ROM_REGION( 0x0200, "bboardplds", ROMREGION_ERASE00 )
	ROM_LOAD( "knm10b.1a",    0x0000, 0x0117, NO_DUMP )
	ROM_LOAD( "iob1.12d",     0x0000, 0x0117, CRC(3abc0700) SHA1(973043aa46ec6d5d1db20dc9d5937005a0f9f6ae) )
	ROM_LOAD( "bprg1.11d",    0x0000, 0x0117, CRC(31793da7) SHA1(400fa7ac517421c978c1ee7773c30b9ed0c5d3f3) )

	ROM_REGION( 0x0200, "cboardplds", ROMREGION_ERASE00 )
	ROM_LOAD( "ioc1.ic7",     0x0000, 0x0117, CRC(0d182081) SHA1(475b3d417785da4bc512cce2b274bb00d4cc6792) )
	ROM_LOAD( "c632.ic1",     0x0000, 0x0117, CRC(0fbd9270) SHA1(d7e737b20c44d41e29ca94be56114b31934dde81) )

	ROM_REGION( 0x08000, "gamecpu", 0 )
	ROM_LOAD( "kensei_mogura_ver1.0.u2",  0x00000, 0x08000, CRC(725cfcfc) SHA1(5a4c6e6efe2ddb38bec3218e55a746ea0146209f) )
ROM_END

DRIVER_INIT_MEMBER(kenseim_state,kenseim)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x800000, 0x800007, read16_delegate(FUNC(kenseim_state::cps1_kensei_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x800018, 0x80001f, read16_delegate(FUNC(kenseim_state::kensei_dsw_r),this));

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x800030, 0x800037, write16_delegate(FUNC(kenseim_state::cps1_kensei_w),this));

	DRIVER_INIT_CALL(cps1);

	m_led_serial_data = 0;
	m_led_clock = 0;
	m_led_latch = 0;

} 


 // 1994.04.18 is from extra PCB rom, Siguma or Sigma? (Siguma is in the ROM)
 // the CPS1 board roms contain "M O G U R A   9 2 0 9 2 4" strings suggesting that part of the code was developed earlier
 GAMEL( 1994, kenseim,       0,        kenseim, kenseim,      kenseim_state,   kenseim,     ROT0,   "Sigma / Togo / Capcom", "Kensei Mogura (1994.04.18, Ver 1.00)", GAME_NOT_WORKING, layout_kenseim )

