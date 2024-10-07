// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo
/******************************************************************************************


  Driver for MCS48-based slots by Recreativos Franco.


*******************************************************************************************

  Recreativos Franco PCB "RF-3115"
  .--------------------------------------------------------------------------------------.
  |    ······································································            |
  |      .-----.                                                                        ·|
  |      |4xDSW|                                                                        ·|
  |      '-----'                                                                        ·|
  |  .----------.  .----------. .----------. .----------. .----------. .----------.     ·|
  |  | SN74154N |  | D8243C   | | D8243C   | | D8243C   | | D8243C   | | D8243C   |     ·|
  |  '----------'  '----------' '----------' '----------' '----------' '----------'     ·|
  |                                                                                     ·|
  |                                                        XTAL                          |
  |  .------. .---------. .-----. .--------. .------.   6.0000 MHz  .--------------.     |
  |  |LM380N| |CD4051BCN| |6xDSW| |74LS00PC| |7402PC|               | Intel P8035L |  .-.|
  |  '------' '---------' '-----' '--------' '------'               '--------------'  |·||
  |                     .----------. .----------.                                     |·||
  |  .---------------.  |S5101L-1PC| |S5101L-1PC|   .-----------.   .-----------.     |·||
  |  | Intel D8748   |  '----------' '----------'   | EPROM     |   | AMD P8212 |     '-'|
  |  |               |                              |           |   |           |        |
  |  '---------------' .----------. .----------.    '-----------'   '-----------'       ·|
  |        XTAL        |S5101L-1PC| |S5101L-1PC|                                        ·|
  |     4.0000 MHz     '----------' '----------'                         LED-> O        ·|
  |                                                                                     ·|
  |   BATTERY       RF-3115                                                             ·|
  |                                                                                      |
  '--------------------------------------------------------------------------------------'


*******************************************************************************************

  TODO:

  - Discover and emulate 100 Pts. coin in action, done by an owner hack in mainboard
    with a 74ls164. This 100 Pts. hack accepts this coin and gives back three 25 Pts.
    coins as change, then only plays 25 Pts. as bet.

  - Complete Baby Fruits 25 pts. emulation, partially emulated due to a bad dump of the
    main CPU ROM.

  - Complete Ajofrin emulation, partially emulated due to a bad dump of the main CPU ROM.


******************************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8212.h"
#include "machine/i8243.h"
#include "machine/i8279.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "speaker.h"

#include "babyfrts.lh"

namespace
{

class rfslotsmcs48_state : public driver_device
{
public:
	rfslotsmcs48_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_data_ram(*this, "data_ram", 0x100, ENDIANNESS_LITTLE)
		, m_maincpu(*this, "maincpu")
		, m_ioexp(*this, "ioexp%u", 0U)
		, m_kbdc(*this, "kbdc")  // found leftover code for kdbc controller
		, m_hopper(*this, "hopper")
		, m_sndajcpu(*this, "sndajcpu")
		, m_sndbfcpu(*this, "sndbfcpu")
		, m_aysnd1(*this, "aysnd1")
		, m_aysnd2(*this, "aysnd2")
		, m_outbit(*this, "outbit%u", 0U)
	{ }

	void rf_3115_base(machine_config &config);
	void ajofrin(machine_config &config);
	void babyfrts(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	memory_share_creator<uint8_t> m_data_ram;

	required_device<i8035_device> m_maincpu;
	required_device_array<i8243_device, 5> m_ioexp;
	required_device<i8279_device> m_kbdc;
	required_device<hopper_device> m_hopper;
	optional_device<i8039_device> m_sndajcpu;
	optional_device<i8035_device> m_sndbfcpu;
	optional_device<ay8910_device> m_aysnd1;
	optional_device<ay8910_device> m_aysnd2;

	output_finder<50> m_outbit;

private:
	void main_program_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void soundaj_program_map(address_map &map) ATTR_COLD;
	void soundbf_program_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;

	// Main MCU Interface
	u8 main_io_r(offs_t offset);
	void main_io_w(offs_t offset, u8 data);
	void main_p1_w(u8 data);
	u8 main_p2_r();
	void main_p2_w(u8 data);
	u8 m_int_flag = 0x00;  // to do

	// Sound MCU Interface
	u8 sound_io_r(offs_t offset);
	void sound_io_w(offs_t offset, u8 data);
	void sound_p1_w(u8 data);
	u8 sound_p2_r();
	u8 m_sound_code = 0;

	// I8243 IO Expander x 4
	void proy_1_w(u8 data);   // proy. left
	void proy_2_w(u8 data);   // proy. center
	void proy_3_w(u8 data);   // proy. right
	void exp1_p7_w(u8 data);  // babyfruits: Sound reset + Int
	void aj_exp1_p7_w(u8 data);  // ajofrin: Sound reset + Int

	void exp2_p4_w(u8 data);  // coils and emcounters
	void exp2_p5_w(u8 data);  // game lights
	void exp2_p6_w(u8 data);  // push buttons lights
	void exp2_p7_w(u8 data);  // sound codes
	u8 exp2_p7_r();           // sound handshake

	void exp5_p4_w(u8 data);  // hopper decoder data
	void aj_exp5_p6_w(u8 data);  // ajofrin sound control

	// I8279 Interface
	void kbd_sl_w(u8 data);
	void disp_w(u8 data);
	u8 m_kbd_sl = 0x00;

	// other
	u8 m_hdecode = 0;
	void hopper_decode();
};

#define MAIN_CLOCK      XTAL(6'000'000)
#define SND_CLOCK       XTAL(4'000'000)


/******************************************
*              Machine Start              *
*                                         *
******************************************/

void rfslotsmcs48_state::machine_start()
{
	m_outbit.resolve();
}


/*********************************************
*           Memory Map Information           *
*                                            *
*********************************************/

void rfslotsmcs48_state::main_program_map(address_map &map)
{
	map(0x000, 0xfff).rom();
}

void rfslotsmcs48_state::main_io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(rfslotsmcs48_state::main_io_r), FUNC(rfslotsmcs48_state::main_io_w));
}


void rfslotsmcs48_state::soundbf_program_map(address_map &map)
{
	map(0x000, 0xfff).rom();
}

void rfslotsmcs48_state::soundaj_program_map(address_map &map)
{
	map(0x000, 0x3ff).rom();
}

void rfslotsmcs48_state::sound_io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(rfslotsmcs48_state::sound_io_r), FUNC(rfslotsmcs48_state::sound_io_w));
}


/*************************************************
*           Main I8035 MPU Interface             *
*                                                *
*************************************************/

u8 rfslotsmcs48_state::main_io_r(offs_t offset)
{
	u8 ret;
	switch (m_maincpu->p2_r() >> 4)
	{
		case 0x3:  // gpkd A0 (data read/write)
		{
			ret = m_kbdc->data_r();
			// logerror("KDBC Data Read Offs:%02X - Data:%02X\n", offset, ret);
			break;
		}
		case 0x7:  // gpkd A1 (stat read / cmd write)
		{
			ret = m_kbdc->status_r();
			// logerror("KDBC Status Read Offs:%02X - Data:%02X\n", offset, ret);
			break;
		}
		case 0xe:  // NVRAM access
		{
			ret = m_data_ram[offset];
			// logerror("Data RAM Read Offs:%02X - Data:%02X\n", offset, ret);
			break;
		}
		default:
			ret = 0xff;
	}
	// logerror("%s: Main I/O Read Offs:%02X - ret:%02x\n", machine().describe_context(), offset, ret);
	return ret;
}

void rfslotsmcs48_state::main_io_w(offs_t offset, u8 data)
{
	switch (m_maincpu->p2_r() >> 4)
	{
		case 0x3:  // gpkd A0 (data read/write)
		{
			m_kbdc->data_w(data);
			// logerror("GPKD DATA Write Offs:%02X - Data:%02X\n", offset, data);
			break;
		}
		case 0x7:  // gpkd A1 (stat read/ cmd write)
		{
			m_kbdc->cmd_w(data);
			// logerror("GPKD CMD Write Offs:%02X - Data:%02X\n", offset, data);
			break;
		}
		case 0xe:  // NVRAM access
		{
			m_data_ram[offset] = data;
			// logerror("Data RAM Write Offs:%02X - Data:%02X\n", offset, data);
			break;
		}
	}

	// logerror("%s: Main I/O Write Offs:%02X - Data: %02X - selector P2:%02x\n", machine().describe_context(), offset, data, m_maincpu->p2_r());
}

void rfslotsmcs48_state::main_p1_w(u8 data)
{
	m_int_flag = BIT(data, 0);
	m_ioexp[0]->cs_w(BIT(data, 5));  // Chip Select IO Expander_1
	m_ioexp[1]->cs_w(BIT(data, 4));  // Chip Select IO Expander_2
	m_ioexp[2]->cs_w(BIT(data, 3));  // Chip Select IO Expander_3
	m_ioexp[3]->cs_w(BIT(data, 2));  // Chip Select IO Expander_4
	m_ioexp[4]->cs_w(BIT(data, 1));  // Chip Select IO Expander_5

	hopper_decode();  // update hopper motor state

	// bit 7 unused or unknown
	// logerror("%s:Main P1 write Data: %02x\n", machine().describe_context(), data);
}

u8 rfslotsmcs48_state::main_p2_r()
{
	u8 opt, ret;
	opt = m_maincpu->p1_r() & 0x3f;
	switch (opt)
	{
		case 0x1f:  // PIA 1 access
		{
			ret = m_ioexp[0]->p2_r();
			// logerror("%s:PIA 1 Data Read Data:%02X\n", machine().describe_context(), ret);
			break;
		}
		case 0x2f:  // PIA 2 access
		{
			ret = m_ioexp[1]->p2_r();
			// logerror("%s:PIA 2 Data Read Data:%02X\n", machine().describe_context(), ret);
			break;
		}
		case 0x37:  // PIA 3 access (nu)
		{
			ret = m_ioexp[2]->p2_r();
			// logerror("%s:PIA 3 Data Read Data:%02X\n", machine().describe_context(),ret);
			break;
		}
		case 0x3b:  // PIA 4 access
		{
			ret = m_ioexp[3]->p2_r();
			// logerror("%s:PIA 4 Data Read Data:%02X\n", machine().describe_context(),ret);
			break;
		}
		case 0x3d:  // PIA 5 access
		{
			ret = m_ioexp[4]->p2_r();
			// logerror("%s:PIA 5 Data Read Data:%02X\n", machine().describe_context(),ret);
			break;
		}
		default:
		{
			ret = 0xff;
			// logerror("%s:Unk. Main P2 Data Read\n", machine().describe_context());  // debug
		}
	}
	// logerror("%s:Main P2 read Data\n", machine().describe_context());
	return ret;
}

void rfslotsmcs48_state::main_p2_w(u8 data)
{
	u8 opt;
	opt = m_maincpu->p1_r() & 0x3f;
	switch (opt)
	{
		case 0x1f:  // PIA 1 access
		{
			m_ioexp[0]->p2_w(data);
			// logerror("%s:PIA 1 Data Write: Data:%02X\n", machine().describe_context(), data);
			break;
		}
		case 0x2f:  // PIA 2 access
		{
			m_ioexp[1]->p2_w(data);
			// logerror("%s:PIA 2 Data Write Data:%02X\n", machine().describe_context(), data);
			break;
		}
		case 0x37:  // PIA 3 access (nu)
		{
			m_ioexp[2]->p2_w(data);
			// logerror("%s:PIA 3 Data Write Data:%02X\n", machine().describe_context(), data);
			break;
		}
		case 0x3b:  // PIA 4 access
		{
			m_ioexp[3]->p2_w(data);
			// logerror("%s:PIA 4 Data Write Data:%02X\n", machine().describe_context(), data);
			break;
		}
		case 0x3d:  // PIA 5 access
		{
			m_ioexp[4]->p2_w(data);
			//logerror("%s:PIA 5 Data Write Data:%02X\n", machine().describe_context(), data);
			break;
		}
		default:
		{
			// logerror("%s:Unk. Main P2 Data Write:%02x\n", machine().describe_context(), data); // debug
		}
	}
	// logerror("Main P2 Write: %02X\n", data);
}


/**************************************************
*          Sound I8035 MPU Interface              *
*                                                 *
**************************************************/

// sound cpu
u8 rfslotsmcs48_state::sound_io_r(offs_t offset)
{
	logerror("%s: Audio I/O Read Offs:%02X\n", machine().describe_context(), offset);
	if(!BIT(m_sndbfcpu->p1_r(), 0))
	{
		m_aysnd1->address_w(offset);
		return m_aysnd1->data_r();
	}
	if(!BIT(m_sndbfcpu->p1_r(), 1))
	{
		m_aysnd2->address_w(offset);
		return m_aysnd2->data_r();
	}
	return 0xff;
}

void rfslotsmcs48_state::sound_io_w(offs_t offset, u8 data)
{
	if(!BIT(m_sndbfcpu->p1_r(), 0))
	{
		m_aysnd1->address_w(offset);
		m_aysnd1->data_w(data);
		// logerror("%s:m_aysnd1: read:%02X\n", machine().describe_context(), m_aysnd1->data_r());
	}
	if(!BIT(m_sndbfcpu->p1_r(), 1))
	{
		m_aysnd2->address_w(offset);
		m_aysnd2->data_w(data);
		// logerror("%s:m_aysnd2: read:%02X\n", machine().describe_context(), m_aysnd2->data_r());
	}
	// logerror("%s: Audio I/O Write Offs:%02X - Data: %02X p1 select:%02x\n", machine().describe_context(), offset, data, m_sndbfcpu->p1_r());

}

void rfslotsmcs48_state::sound_p1_w(u8 data)
{
	if(!BIT(data, 2))
	{
		m_aysnd1->reset_w();
		m_aysnd2->reset_w();
	}
}

u8 rfslotsmcs48_state::sound_p2_r()
{
	logerror("Sound CPU: Get sound code:%02x\n", m_sound_code);
	return (m_sound_code << 4) & 0xf0;  // upper nibble (to p2.4 - p2.7)
}


/*************************************************
*    I8243 IO Expander Interface x 4             *
*   Access:                                      *
*   P1 -> Enable PIA Access Bits 5, 4, 3, 2, 1   *
*                                                *
*************************************************/

u8 rfslotsmcs48_state::exp2_p7_r()
{
	logerror("%s:Pia 2 - Port 7: Read Sound Handshake:%02X\n", machine().describe_context(), m_sndbfcpu->p2_r());
	return (m_sndbfcpu->p2_r() >> 4) & 0x0f;
}

void rfslotsmcs48_state::proy_1_w(u8 data)  // left projector
{
	m_outbit[0] = data;
}

void rfslotsmcs48_state::proy_2_w(u8 data)  // center projector
{
	m_outbit[1] = data;
}

void rfslotsmcs48_state::proy_3_w(u8 data)  // right projector
{
	m_outbit[2] = data;
}

void rfslotsmcs48_state::exp1_p7_w(u8 data)  // sound reset + int
{
	if(!BIT(data,0))
		logerror("Sent Reset to Sound CPU - Now testing...\n");

	m_sndbfcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);
	m_sndbfcpu->set_input_line(INPUT_LINE_IRQ0, BIT(data, 1) ? CLEAR_LINE : ASSERT_LINE);
}


void rfslotsmcs48_state::aj_exp1_p7_w(u8 data)  // sound code
{
	m_sound_code = data;
	logerror("Pia 2 - P7: Set sound code:%02x\n", m_sound_code);
}

void rfslotsmcs48_state::aj_exp5_p6_w(u8 data)  // sound reset + int
{
	m_sndajcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 2) ? CLEAR_LINE : ASSERT_LINE);
	m_sndajcpu->set_input_line(INPUT_LINE_IRQ0, BIT(data, 3) ? CLEAR_LINE : ASSERT_LINE);
}


void rfslotsmcs48_state::exp2_p4_w(u8 data)  // coils and emcounters
{
	m_outbit[3] = !BIT(data, 0);  // coin lock
	m_outbit[4] = BIT(data, 1);   // unused
	m_outbit[5] = BIT(data, 2);   // coin-in counter
	m_outbit[6] = BIT(data, 3);   // coin-out counter

	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));  // EM counter: Coin In
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3));  // EM counter: Coin Out

	m_outbit[16] = machine().bookkeeping().coin_counter_get_count(0);
	m_outbit[17] = machine().bookkeeping().coin_counter_get_count(1);
}

void rfslotsmcs48_state::exp2_p5_w(u8 data)  // game lights
{
	m_outbit[7] = BIT(data, 0);   // insert coin lamp
	m_outbit[8] = BIT(data, 1);   // fault lamp
	m_outbit[9] = BIT(data, 2);   // start lamp
	m_outbit[10] = BIT(data, 3);  // unused
}

void rfslotsmcs48_state::exp2_p6_w(u8 data)  // push buttons lights
{
	m_outbit[11] = BIT(data, 0);  // stop left lamp
	m_outbit[12] = BIT(data, 1);  // stop center lamp
	m_outbit[13] = BIT(data, 2);  // stop right lamp
	m_outbit[14] = BIT(data, 3);  // unused
}

void rfslotsmcs48_state::exp2_p7_w(u8 data)  // sound codes
{
	m_sound_code = data;
	logerror("Set sound code:%02x\n", m_sound_code);
}

void rfslotsmcs48_state::exp5_p4_w(u8 data)  // 1-16 Selector
{
	m_hdecode = data;
	hopper_decode();  // update hopper motor state
}


/**********************************************
*      I8279 Keyboard-Disply Interface        *
*      Socket empty - Some leftover code      *
**********************************************/

void rfslotsmcs48_state::kbd_sl_w(u8 data)
{
//  Scan Line
	m_kbd_sl = data;
	// logerror("I8279: Scan Line: %02X\n", data);
}

void rfslotsmcs48_state::disp_w(u8 data)
{
//  code initializes a i8279 but there is no display on this system.
}


/*************************************************
*  Other                                         *
*  Hopper decoder                                *
*************************************************/

void rfslotsmcs48_state::hopper_decode()
{
/*
  Validation at bit level to enhance this function documentation.

  g1 = p4.1
  g2 = p4.0
   a = p4.2
   b = fixed to 1
   c = p4.3
   d = p1.6

*/
	u8 res = 0xff;
	if(!BIT(m_hdecode, 0) & (!BIT(m_hdecode, 1)))  // g1&g2=0
	{
		u8 a, b, c, d;
		d = BIT(m_maincpu->p1_r(), 6);
		c = BIT(m_hdecode, 3);
		b = 1;
		a = BIT(m_hdecode, 2);
		res = a + (b * 2) + (c * 4) + (d * 8);
	}

	if(res == 0x03)
	{
		m_hopper->motor_w(1);
		m_outbit[15] = 1;
		logerror("Hopper paying....\n");
	}
	else
	{
		m_hopper->motor_w(0);
		m_outbit[15] = 0;
	}
	// logerror("Hopper decode:%02x\n", res);
}


/*************************************************
*                Input Ports                     *
*                                                *
*************************************************/

static INPUT_PORTS_START(babyfrts)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1)  PORT_NAME("Stop/Advance 1")  // left button
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2)  PORT_NAME("Stop/Advance 2")  // central button
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3)  PORT_NAME("Stop/Advance 3")  // right button
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Juegue/Start")         // start button

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )  // ctrl 1 (display) ?  \   :
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )  // ctrl 2 (display) ?   \  : from schematics: these lines are tied to an i8279
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )  // ctrl 3 (display) ?   /  : not present in the real hardware.
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )  // ctrl 4 (display) ?  /   :

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM )  PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1)    PORT_NAME("Coin In")  PORT_IMPULSE(5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SWA")  // switches order from schematics...
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown))  PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown))  PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown))  PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown))  PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x08, DEF_STR(Off))

	PORT_START("SWB")  // switches order from schematics...
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown))  PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown))  PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown))  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown))  PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x08, DEF_STR(Off))
INPUT_PORTS_END


static INPUT_PORTS_START(ajofrin)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)  // unknown
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)  // unknown
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)  // unknown
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)  // unknown

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-1") PORT_CODE(KEYCODE_5)  // unknown
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-2") PORT_CODE(KEYCODE_6)  // unknown
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-3") PORT_CODE(KEYCODE_7)  // unknown
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-4") PORT_CODE(KEYCODE_8)  // unknown

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-3") PORT_CODE(KEYCODE_A)  // unknown
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-3") PORT_CODE(KEYCODE_S)  // unknown
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)  // unknown
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)  // unknown

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-1") PORT_CODE(KEYCODE_G)  // unknown
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-2") PORT_CODE(KEYCODE_H)  // unknown
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-3") PORT_CODE(KEYCODE_J)  // unknown
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-4") PORT_CODE(KEYCODE_K)  // unknown

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-1") PORT_CODE(KEYCODE_Z)  // unknown
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-2") PORT_CODE(KEYCODE_X)  // unknown
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-3") PORT_CODE(KEYCODE_C)  // unknown
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-4") PORT_CODE(KEYCODE_V)  // unknown

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-1") PORT_CODE(KEYCODE_B)  // unknown
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-2") PORT_CODE(KEYCODE_N)  // unknown
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-3") PORT_CODE(KEYCODE_M)  // unknown
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-4") PORT_CODE(KEYCODE_L)  // unknown
INPUT_PORTS_END


/*********************************************
*              Machine Drivers               *
*                                            *
*********************************************/

void rfslotsmcs48_state::rf_3115_base(machine_config &config)
{
	// Main PCB
	I8035(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &rfslotsmcs48_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO , &rfslotsmcs48_state::main_io_map);

	m_maincpu->p1_out_cb().set(FUNC(rfslotsmcs48_state::main_p1_w));
	m_maincpu->p2_in_cb().set(FUNC(rfslotsmcs48_state::main_p2_r));
	m_maincpu->p2_out_cb().set(FUNC(rfslotsmcs48_state::main_p2_w));

	m_maincpu->prog_out_cb().set(m_ioexp[0], FUNC(i8243_device::prog_w));
	m_maincpu->prog_out_cb().append(m_ioexp[1], FUNC(i8243_device::prog_w));
	m_maincpu->prog_out_cb().append(m_ioexp[2], FUNC(i8243_device::prog_w));
	m_maincpu->prog_out_cb().append(m_ioexp[3], FUNC(i8243_device::prog_w));
	m_maincpu->prog_out_cb().append(m_ioexp[4], FUNC(i8243_device::prog_w));

	I8243(config, m_ioexp[0]);  // PIA 1
	I8243(config, m_ioexp[1]);  // PIA 2
	I8243(config, m_ioexp[2]);  // PIA 3
	I8243(config, m_ioexp[3]);  // PIA 4
	I8243(config, m_ioexp[4]);  // PIA 5

	I8279(config, m_kbdc, MAIN_CLOCK / 3);  // 2 MHz. (Derived from Main CPU T0 line, that gives Main Clock / 3  frequency.
	m_kbdc->out_sl_callback().set(FUNC(rfslotsmcs48_state::kbd_sl_w));  // scan SL lines
	m_kbdc->out_disp_callback().set(FUNC(rfslotsmcs48_state::disp_w));  // display A&B
	m_kbdc->in_shift_callback().set_constant(0);
	m_kbdc->in_ctrl_callback().set_constant(0);

	I8212(config, "i8212_2");

	NVRAM(config, "data_ram", nvram_device::DEFAULT_ALL_0);

	// Hopper device
	HOPPER(config, m_hopper, attotime::from_msec(100));

	SPEAKER(config, "mono").front_center();
}


void rfslotsmcs48_state::babyfrts(machine_config &config)
{
	rf_3115_base(config);

	// Sound PCB
	I8035(config, m_sndbfcpu, MAIN_CLOCK);
	m_sndbfcpu->set_addrmap(AS_PROGRAM, &rfslotsmcs48_state::soundbf_program_map);
	m_sndbfcpu->set_addrmap(AS_IO,      &rfslotsmcs48_state::sound_io_map);
	m_sndbfcpu->p1_out_cb().set(FUNC(rfslotsmcs48_state::sound_p1_w));
	m_sndbfcpu->p2_in_cb().set(FUNC(rfslotsmcs48_state::sound_p2_r));

//  I8243, m_ioexp[0];  PIA 1: fruits projectors
	m_ioexp[0]->p4_out_cb().set(FUNC(rfslotsmcs48_state::proy_1_w));   // left
	m_ioexp[0]->p5_out_cb().set(FUNC(rfslotsmcs48_state::proy_2_w));   // center
	m_ioexp[0]->p6_out_cb().set(FUNC(rfslotsmcs48_state::proy_3_w));   // right
	m_ioexp[0]->p7_out_cb().set(FUNC(rfslotsmcs48_state::exp1_p7_w));  // sound Reset + Int

//  I8243, m_ioexp[1];  PIA 2
	m_ioexp[1]->p4_out_cb().set(FUNC(rfslotsmcs48_state::exp2_p4_w));  // coils and EM counters
	m_ioexp[1]->p5_out_cb().set(FUNC(rfslotsmcs48_state::exp2_p5_w));  // game lights
	m_ioexp[1]->p6_out_cb().set(FUNC(rfslotsmcs48_state::exp2_p6_w));  // push buttons lights
	m_ioexp[1]->p7_out_cb().set(FUNC(rfslotsmcs48_state::exp2_p7_w));  // sound codes
	m_ioexp[1]->p7_in_cb().set(FUNC(rfslotsmcs48_state::exp2_p7_r));   // sound handshake

//  I8243, m_ioexp[3];  PIA 4
	m_ioexp[3]->p4_in_cb().set_ioport("IN0");
	m_ioexp[3]->p5_in_cb().set_ioport("IN1");
	m_ioexp[3]->p6_in_cb().set_ioport("IN4");  // SWA
	m_ioexp[3]->p7_in_cb().set_ioport("IN2");

//  I8243, m_ioexp[4];  PIA 5
	m_ioexp[4]->p4_out_cb().set(FUNC(rfslotsmcs48_state::exp5_p4_w));  // Selector 1-16
	m_ioexp[4]->p5_in_cb().set_ioport("IN5");  // SWB
	m_ioexp[4]->p7_in_cb().set_ioport("IN3");

	// video layout
	config.set_default_layout(layout_babyfrts);

	AY8910(config, m_aysnd1, MAIN_CLOCK / 3).add_route(ALL_OUTPUTS, "mono", 0.50); // Derived from soundcpu->T0 Clock (soundcpu/3)
	AY8910(config, m_aysnd2, MAIN_CLOCK / 3).add_route(ALL_OUTPUTS, "mono", 0.50); // Derived from soundcpu->T0 Clock (soundcpu/3)
}


void rfslotsmcs48_state::ajofrin(machine_config &config)
{
	rf_3115_base(config);

	I8039(config, m_sndajcpu, 4_MHz_XTAL);
	m_sndajcpu->set_addrmap(AS_PROGRAM, &rfslotsmcs48_state::soundaj_program_map);
	m_sndajcpu->set_addrmap(AS_IO,      &rfslotsmcs48_state::sound_io_map);
	m_sndajcpu->p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	m_sndajcpu->p2_in_cb().set(FUNC(rfslotsmcs48_state::sound_p2_r));

//  I8243, m_ioexp[0];  PIA 1
	m_ioexp[0]->p4_out_cb().set(FUNC(rfslotsmcs48_state::proy_1_w));  // to verify left projector
	m_ioexp[0]->p5_out_cb().set(FUNC(rfslotsmcs48_state::proy_2_w));  // to verify center projector
	m_ioexp[0]->p6_out_cb().set(FUNC(rfslotsmcs48_state::proy_3_w));  // to verify right projector - There is an extra projector. To be found.
	m_ioexp[0]->p7_out_cb().set(FUNC(rfslotsmcs48_state::aj_exp1_p7_w));  // sound + int to verify

//  I8243, m_ioexp[1];  PIA 2
	m_ioexp[1]->p4_out_cb().set(FUNC(rfslotsmcs48_state::exp2_p4_w));  // coils and EM counters - idem bfr
	m_ioexp[1]->p5_out_cb().set(FUNC(rfslotsmcs48_state::exp2_p5_w));  // game lights - idem bfr

//  I8243, m_ioexp[2];  PIA 3
	m_ioexp[2]->p7_in_cb().set_ioport("IN0");

//  I8243, m_ioexp[3];  PIA 4
	m_ioexp[3]->p4_in_cb().set_ioport("IN1");
	m_ioexp[3]->p5_in_cb().set_ioport("IN2");
	m_ioexp[3]->p6_in_cb().set_ioport("IN3");
	m_ioexp[3]->p7_in_cb().set_ioport("IN4");

//  I8243, m_ioexp[4];  PIA 5
	m_ioexp[4]->p5_in_cb().set_ioport("IN5");
	m_ioexp[4]->p6_out_cb().set(FUNC(rfslotsmcs48_state::aj_exp5_p6_w));

	// video layout
	// config.set_default_layout(layout_ajofrin);

	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "mono", 2.0);
}


/*********************************************
*                  Rom Load                  *
*                                            *
*********************************************/

/* Baby Fruits 100 Pst.
   It's an upgrade of "babyfrts25" that accepts also 100 Pts coins instead of just 25 Pts ones.

   Standard RF-3115 PCB but:
    -Two S5101 static RAM chips (instead of 4, two empty sockets)
    -Four NEC D8243C (instead of 5, one empty socket in the middle).
    -No D8748 (the socket is empty).

   Also the board uses an external PCB (RF 53/3131) for sound
   with the following layout:

     .----------------------------------.
     |    .-------------------------.   |
     |    |·························|   |
     |    '-------------------------'   |
     |             Xtal                 |
     |            6.000 MHz             |
     |           .--------------------. |
     | .-------. |  AM8035PC          | |
     | |DM7486N| |                    | |
     | '-------' '--------------------' |
     |    .-----------.  .-----------.  |
     |    |Intel D8212|  |2532 EPROM |  |
     |    |           |  |           |  |
     |    '-----------'  '-----------'  |
     |          .--------------------.  |
     |          | GI AY-3-8910       |  |
     |          |                    |  |
     |          '--------------------'  |
     |          .--------------------.  |
     |          | GI AY-3-8910       |  |
     |          |                    |  |
     |          '--------------------'  |
     | .-------.                        |
     | |· · · ·|                        |
     | '-------'                        |
     '----------------------------------'

*/
ROM_START(babyfrts)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("bbr_cpu_100pts.bin",     0x0000, 0x1000, CRC(22c47a4d) SHA1(051c82b20a823047119a6d5460c07df601e44a2e))

	ROM_REGION(0x1000, "sndbfcpu", 0)
	ROM_LOAD( "bbs_100pts_sonido.bin", 0x0000, 0x1000, CRC(c84e20d3) SHA1(ab9457d3877a10bc54485a4954ef5b4d006ed3be))
ROM_END

// Same PCBs configuration as "babyfrts"
ROM_START(babyfrts25)
	ROM_REGION(0x1000, "maincpu", 0)  // too many illegal opcodes.
	ROM_LOAD("baby_8.3_av_25pts.bin",  0x0000, 0x1000, BAD_DUMP CRC(af45e046) SHA1(d4896f428a2061ad6bc12eed7d56eca4182d237d))

	ROM_REGION(0x1000, "sndbfcpu", 0)
	ROM_LOAD( "bbs_25pts_sonido.bin",  0x0000, 0x1000, CRC(384dc9b4) SHA1(0a3ab8a7dfba958858b06d23850d1a8a2b9a348f))
ROM_END


// Found just one PCB, may be missing some more boards (extra sound, I/O, etc.)
ROM_START(ajofrin)
	ROM_REGION(0x1000, "maincpu", 0)  // underdumped and duped halves. bad higher address pin
	ROM_LOAD("ajofr_90percent_4k.bin", 0x0000, 0x1000, BAD_DUMP CRC(9e1fd7fe) SHA1(e7a5a2a10d17537edb039ac0f53358ee35465c90))

	ROM_REGION(0x0400, "sndajcpu", 0)
	ROM_LOAD( "ajo_d8748.bin",         0x0000, 0x0400, CRC(414dc0e3) SHA1(016ed2aa36b36a637163ac7cba0a944a258c02a4))
ROM_END


} // anonymous namespace


/*********************************************
*              Game Drivers                  *
*                                            *
*********************************************/

//    YEAR  NAME        PARENT    MACHINE   INPUT     CLASS               INIT        ROT    COMPANY               FULLNAME                        FLAGS
GAME( 1981, babyfrts,   0,        babyfrts, babyfrts, rfslotsmcs48_state, empty_init, ROT0, "Recreativos Franco", "Baby Fruits (100 pts version)", MACHINE_MECHANICAL )
GAME( 1981, babyfrts25, babyfrts, babyfrts, babyfrts, rfslotsmcs48_state, empty_init, ROT0, "Recreativos Franco", "Baby Fruits (25 pts version)",  MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 1981, ajofrin,    0,        ajofrin,  ajofrin,  rfslotsmcs48_state, empty_init, ROT0, "Recreativos Franco", "Ajofrin City",                  MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
