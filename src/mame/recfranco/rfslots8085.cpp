// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo

/*******************************************************************************************************

  i8085 + projectors based slots by Recreativos Franco
  ----------------------------------------------------

  Recreativos Franco used this hardware from 1987 to 1992 on several machines, including:

    -Baby & Bombo
    -Baby Formula 2
    -Limon y Baby
    -Limon y Baby 100
    -Baby Ajofrin Dakar 3
    -El Tren
    -Baby Derby

     ...Etc.


********************************************************************************************************

  Hardware Notes:
  ---------------


  Recreativos Franco PCB Ref. 53/3297
  .-----------------------------------------------------------------------------------------------------------------.
  |  ....A7.....   ....A8.....   ....A9.....   ....A10....   ....A11....   ....A12....   ....A13....   ....A14....  |
  | .----------.  .----------.  .----------.  .----------.  .----------.  .----------.  .----------.  .----------.  |
  | | ULN2803A |  | ULN2803A |  | ULN2803A |  | ULN2803A |  | ULN2803A |  | ULN2803A |  | ULN2803A |  | ULN2803A |  |
  | '----------'  '----------'  '----------'  '----------'  '----------'  '----------'  '----------'  '----------'  |
  |               .----------.                .----------.                .----------.                .----------.  |
  |               |SN74HC374N|                |SN74HC374N|                |SN74HC374N|                |SN74HC374N|  |
  |               '----------'                '----------'                '----------'                '----------'  |
  | .----------.                .----------.                .----------.                .----------.                |
  | |SN74HC374N|                |SN74HC374N|                |SN74HC374N|                |SN74HC374N|                |
  | '----------'                '----------'                '----------'                '----------'                |
  |                                                                                                                 |
  | .............A2.................  ....A3......  .........A4.............  .......A5............  .....A6......  |
  |                                                                                                                 |
  |                                                                                     .----------.                |
  |                                                                                     | SN74LS0N |                |
  |                                                                                     '----------'                |
  | .----------.                                            .----------.              .----------.                  |
  | |          |                                            | SN7447AN |              |SN74LS138N|                  |
  | '----------'                                            '----------'              '----------'                  |
  | .----------.  .----------.                .----------.                                                          |
  | |SN74HC374N|  |SN74HC374N|                | TC40288P |                                                          |
  | '----------'  '----------'                '----------'                                                          |
  |                                                                                                                 |
  |   .-----------------------.     .-----------------------.     .-----------------------.                         |
  |   | NEC D8155HC           |     | NEC D8279C-2          |     | Toshiba TMP8255AP-5   |                         |
  |   |                       |     |                       |     |                       |                         |
  |   '-----------------------'     '-----------------------'     '-----------------------'                         |
  |                                                                                                                 |
  |   .-------.    .----------.                                                                                     |
  |   |DIPS x6|    |SN74HC132N|                                                                                     |
  |   '-------'    '----------'    .-------------.   .-------------.                                                |
  |     .------.     .-----------. |M1-31/B-1704 |   |TC5517APL    |            .----------.                        |
  |     |NE555C|     |SN74LS125AN| |             |   |             |            |SN74LS138N|                        |
  |     '------'     '-----------' '-------------'   '-------------'            '----------'                        |
  | .------.  BATT         .---------------------.   .-------------.            .----------.                        |
  | |LM339N|  4.8V         |NEC D8085AC          |   |AMD P8212    |            |SN74LS245N|                        |
  | '------'        Xtal   |                     |   |             |            '----------'                        |
  |                5.0688  '---------------------'   '-------------'                                                |
  |                        .---------------------.                           .-------------.                        |
  |                        |SCN8035A             |    .-----------.          | IC44        |                        |
  |                        |                     |    |SN74LS537AN|          |             |                        |
  |                        '---------------------'    '-----------'          '-------------'                        |
  |                            Xtal                                                            Attract Mode         |
  |                           6.000                                                               Switch            |
  |                        .---------------------.                                                                  |
  |                        |AY-38910-A           |    .----------.          .----------.                            |
  |                        |                     |    | SN7486N  |          |  LM380N  |                            |
  |                        '---------------------'    '----------'          '----------'                            |
  |                        .---------------------.                                                                  |
  |                        |AY-38910-A           |                                                                  |
  |                        |                     |                                                                  |
  |                        '---------------------'                                                                  |
  |                                                                                  ......A1........               |
  '-----------------------------------------------------------------------------------------------------------------'

    IC17 = NEC uPD8155HC-2 Static RAM I/O Timer
    IC21 = NEC uPD8279C-2 Programmable Keyboard Display Interface
    IC24 = Toshiba TMP8255AP-5 Programmable Peripheral Interface
    IC32 = 27C128
    IC34 = Toshiba TC5517APL 2.048 Word X 8 Bit CMOS Static RAM
    IC31 = uPD8085AC CPU (Xtal 5.0688 MHz)
    IC33 = AMD P8212 8-Bit IO Port
    IC40 = Signetics SCN8035A MCU (Xtal 6.000 MHz)
    IC44 = 27128
    IC39 = AY-3-8910A
    IC38 = AY-3-8910A


********************************************************************************************************

  Tech notes about Baby & Bombo emulation:

  In the sound CPU code, there is a write operation to PSG Port A. Surprisingly, there is no reference
  to this port in the schematics. Here, the above mentioned Port A access, taken from the debugger:

  [:aysnd0] ':audiocpu' (3D7): warning - read from AY-3-8910A PSG Port A set as output
  [:aysnd0] ':audiocpu' (3D7): warning - read 8910 Port A

  It appears that there is a similarity in the hardware design with other R.F. boards,
  as they share the same Port A lines used for Coin In and Coin Out counters.
  This suggests that the code in question might be a leftover or remnant from that shared design.


*******************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/em_reel.h"
#include "machine/i8155.h"
#include "machine/i8212.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "speaker.h"

#include "bbombo.lh"

namespace
{

class rfslots8085_state : public driver_device
{
public:
	rfslots8085_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "audiocpu")
		, m_kbdc(*this, "kdbc")
		, m_memiot(*this, "memiot")
		, m_ppi(*this, "ppi")
		, m_hopper(*this, "hopper%u", 0U)
		, m_reel(*this, "reel")
		, m_dac(*this, "dac")
		, m_aysnd(*this, "aysnd%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)

	{ }

	enum { STEPS_PER_SYMBOL = 1 };

	void add_em_reels(machine_config &config, int symbols, attotime period);

	void rf53_3297(machine_config &config);

	int reel_opto_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void main_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	// Main MCU Interface
	u8 sid_r();

	// Sound MCU Interface
	u8 sound_io_r(offs_t offset);
	void sound_io_w(offs_t offset, u8 data);
	u8 sound_p1_r();
	void sound_p1_w(u8 data);
	void sound_p2_w(u8 data);

	u8 m_sound_code = 0;

	// I8155 Interface
	void m_miot_pa_w(u8 data);
	void memiot_pb_w(u8 data);
	void rst65_w(u8 state);
	void latch_update();
	u8 m_latch_sel = 0xff;
	u8 m_datalatch = 0x00;
	u8 m_miot_pa = 0;

	// I8255 Interface
	void ppi_pb_w(u8 data);
	void ppi_pc_w(u8 data);
	int findSetBitPosition(u8 value);
	u8 ppi_pc = 0xff;

	// I8279 Interface
	u8 kbd_rl_r();
	void kbd_sl_w(u8 data);
	void disp_w(u8 data);
	void rst55_w(u8 state);
	void output_digit(u8 i, u8 data);
	u8 m_kbd_sl = 0x00;

	// AY-3-8910 (0) Port access
	void ay0_porta_w(u8 data);

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8035_device> m_soundcpu;
	required_device<i8279_device> m_kbdc;
	required_device<i8155_device> m_memiot;
	required_device<i8255_device> m_ppi;
	required_device_array<hopper_device, 2> m_hopper;
	required_device<em_reel_device> m_reel;
	required_device<dac_3bit_binary_weighted_device> m_dac;
	required_device_array<ay8910_device, 2> m_aysnd;
	output_finder<72> m_lamps;
};

#define MAIN_CLOCK      XTAL(5'068'800)
#define SND_CLOCK       XTAL(6'000'000)


/******************************************
*              Machine Start              *
*                                         *
******************************************/

void rfslots8085_state::machine_start()
{
	m_lamps.resolve();
}


/*********************************************
*           Memory Map Information           *
*                                            *
*********************************************/

void rfslots8085_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0x9000, 0x9000).rw(m_kbdc, FUNC(i8279_device::data_r), FUNC(i8279_device::data_w));
	map(0x9001, 0x9001).rw(m_kbdc, FUNC(i8279_device::status_r), FUNC(i8279_device::cmd_w));
	map(0xaa00, 0xaaff).rw(m_memiot, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xb000, 0xb003).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void rfslots8085_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
}


void rfslots8085_state::sound_map(address_map &map)
{
	map(0x000, 0xfff).rom();
}

void rfslots8085_state::sound_io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(rfslots8085_state::sound_io_r), FUNC(rfslots8085_state::sound_io_w));
}


/*************************************************
*           Main I8085 CPU Interface             *
*                                                *
*************************************************/

u8 rfslots8085_state::sid_r()
{
	return BIT(m_soundcpu->p2_r(), 7);
}


/**************************************************
*          Sound I8035 MCU Interface              *
*                                                 *
**************************************************/

u8 rfslots8085_state::sound_io_r(offs_t offset)
{
	if(!BIT(m_soundcpu->p2_r(), 5))
	{
		m_aysnd[0]->address_w(offset);
		return m_aysnd[0]->data_r();
	}
	if(!BIT(m_soundcpu->p2_r(), 6))
	{
		m_aysnd[1]->address_w(offset);
		return m_aysnd[1]->data_r();
	}
	// logerror("%s: Audio I/O Read Offs:%02X\n", machine().describe_context(), offset);
	return 0xff;
}

void rfslots8085_state::sound_io_w(offs_t offset, u8 data)
{
	if(!BIT(m_soundcpu->p2_r(), 5))
	{
		m_aysnd[0]->address_w(offset);
		m_aysnd[0]->data_w(data);
	}
	if(!BIT(m_soundcpu->p2_r(), 6))
	{
		m_aysnd[1]->address_w(offset);
		m_aysnd[1]->data_w(data);
	}
}

u8 rfslots8085_state::sound_p1_r()
{
//  P1.0 .. P1.3 (input: Sound Code) <-  i8255(w):Pb.0 .. Pb.3

	// logerror("Sound CPU: Get sound code:%02x\n", m_sound_code & 0x0f );
	return m_sound_code & 0x0f;  // lower nibble
}

void rfslots8085_state::sound_p1_w(u8 data)
{
//  P1.4 .. P1.6 -> DAC

	m_dac->write((data >> 4) & 0x07);
	// logerror("Sound CPU: Send Out DAC signals:%02x\n", m_sound_code & 0x0f );
}

void rfslots8085_state::sound_p2_w(u8 data)
{
// P2.4 -> AY-3-8910 /RESET

	if(!BIT(data, 4))
	{
		m_aysnd[0]->reset_w();
		m_aysnd[1]->reset_w();
	}
}


/*********************************************
*         I8155 MEMORY / PPI / TIMER         *
*                                            *
*********************************************/

int rfslots8085_state::findSetBitPosition(u8 value)
{
	if (value == 0)
		return -1;

	u8 position = 1;
	while ((value & 1) == 0)
	{
		value >>= 1;
		position++;
	}
	return position;
}

void rfslots8085_state::latch_update()
{
	u8 m_upperp = 0;
	u8 m_rightp = 0;
	u8 m_centp = 0;
	u8 m_leftp = 0;

	switch( m_latch_sel )
	{
		case 0xfe:  // 01h - bit 0
		{
			// pa0: upper proj
			m_lamps[0] = m_lamps[10] = 0;
			if(m_datalatch != 0)
			{
				m_lamps[0] = findSetBitPosition(m_datalatch);
				m_lamps[10] = 1;
				// logerror("i8155: Latch 0 upper proj.  Data Latch:%02x -lamp0:%02x\n\n", m_datalatch, m_lamps[0]);
			}
			break;
		}
		case 0xfd:  // 02h - bit 1
		{
			// pa1: right+up ((high nibble -> Completes upper projector - lamps0)(lower nibble -> Completes right projector - lamps2))
			m_rightp = bitswap(m_datalatch, 7, 5, 1, 4, 3, 2, 1, 0) & 0x0f ;
			if(m_rightp != 0)
			{
				m_lamps[3] = findSetBitPosition(m_rightp) + 8;
				m_lamps[13] = 1;
				// logerror("i8155: Latch 3 right proj(extra).  Data Latch:%02x -lamp3:%02x\n\n", m_datalatch, m_lamps[3]);
			}
			m_upperp = (bitswap(m_datalatch, 7, 6, 5, 4, 6, 2, 3, 0) & 0xf0) >> 4;
			if(m_upperp != 0)
			{
				m_lamps[0] = findSetBitPosition(m_upperp) + 8;
				m_lamps[10] = 1;
				// logerror("i8155: Latch 0 upper proj(extra).  Data Latch:%02x -lamp0:%02x\n\n", m_datalatch, m_lamps[0]);
			}
			break;
		}
		case 0xfb:  // 04h - bit 2
		{
			// pa2: right proj
			m_lamps[3] = m_lamps[13] = 0;
			if(m_datalatch != 0)
			{
				m_lamps[3] = findSetBitPosition(m_datalatch);
				m_lamps[13] = 1;
				// logerror("i8155: Latch 3 right proj.  Data Latch:%02x -lamp3:%02x\n\n", m_datalatch, m_lamps[3]);
			}
			break;
		}
		case 0xf7:  // 08h - bit 3
		{
			// pa3: left proj (lamps1)
			m_lamps[1] = m_lamps[11] = 0;
			if(m_datalatch != 0)
			{
				m_lamps[1] = findSetBitPosition(m_datalatch);
				m_lamps[11] = 1;
				// logerror("i8155: Latch 1 left proj.  Data Latch:%02x -lamp3:%02x\n\n", m_datalatch, m_lamps[1]);
			}
			break;
		}
		case 0xef:  // 10h - bit 4
		{
			// pa4: cent proj (lamps2)
			m_lamps[2] = m_lamps[12] = 0;
			if(m_datalatch != 0)
			{
				m_lamps[2] = findSetBitPosition(m_datalatch);
				m_lamps[12] = 1;
				// logerror("i8155: Latch 2 left proj.  Data Latch:%02x -lamp3:%02x\n\n", m_datalatch, m_lamps[2]);
			}
			// logerror("%s:i8155: Latch 4 cent proj.  Data:%02x\n\n",machine().describe_context(), m_datalatch);
			break;
		}
		case 0xdf:  // 20h - bit 5 (high nibble -> Completes left projector - lamps1)(lower nibble -> Completes center projector - lamps2)
		{
			// pa5: left+cent (extra projector lamps for lottery game)
			m_leftp = (bitswap(m_datalatch, 7, 6, 5, 4, 6, 2, 3, 0) & 0xf0) >> 4;
			if(m_leftp != 0)
			{
				m_lamps[1] = findSetBitPosition(m_leftp) + 8;
				m_lamps[11] = 1;
				// logerror("i8155: Latch 1 left proj.(extra)  Data Latch:%02x -lamp3:%02x\n\n", m_datalatch, m_lamps[1]);
			}

			m_centp = bitswap(m_datalatch, 7, 5, 1, 4, 3, 2, 1, 0) & 0x0f;
			if(m_centp != 0)
			{
				m_lamps[2] = findSetBitPosition(m_centp) + 8;
				m_lamps[12] = 1;
				// logerror("i8155: Latch 2 left proj.(extra)  Data Latch:%02x -lamp3:%02x\n\n", m_datalatch, m_lamps[2]);
			}
			break;
		}
		case 0xbf:  // 40h - bit 6 (lamp 20-27)
		{
			/*
			pa6: coils - leds

			bit 0: Led Avances                 Advances Led
			bit 1: Bob. desvio 25 enclave      Diverter 25 coil lock
			bit 2: Led Premios                 Prizes Led
			bit 3: n.u.
			bit 4: Bob. desvio 25 desenclave   Diverter 25 coil unlock
			bit 5: Bob. desvio 100 desenclave  Diverter 100 coil unlock
			bit 6: Bob. desvio 100 enclave     Diverter 100 coil lock
			bit 7: Led Creditos                Credits Led
			*/
			for(u8 i = 0; i < 8 ; i++)
				m_lamps[20 + i] = BIT(m_datalatch, i);
			// logerror("%s:i8155: Latch 6 Update  Data:%02x\n\n",machine().describe_context(), m_datalatch);
			break;
		}
		case 0x7f: // 80 - bit 7 (lamp 28-35)
		{
			/*
			pa7: game lamps (* = mismatch with bformula2 schematics)

			bit 0: Lamp. Boton Doblar          Lamp. Button Double
			bit 1: Lamp. Boton Arranque *      Lamp. Button Start
			bit 2: Lamp. Boton Lento           Lamp. Button Slow
			bit 3: Lamp. Boton Pulse Der. *    Lamp. Button Push Right
			bit 4: Lamp. Boton Cobrar *        Lamp. Button Push Cash Out
			bit 5: Lamp. Boton Pulse Izq. *    Lamp. Button Push Left
			bit 6: Lamp. Boton Recupere *      Lamp. Button Recover
			bit 7: Lamp. Boton Loteria *       Lamp. Button Lottery
			*/
			for(u8 i = 0; i < 8 ; i++)
				m_lamps[28 + i] = BIT(m_datalatch, i);
			// logerror("%s:i8155: Latch 7 Update  Data:%02x\n\n",machine().describe_context(), m_datalatch);
			break;
		}
	}
}


void rfslots8085_state::m_miot_pa_w(u8 data)
{
	u8 bits_up = (m_miot_pa ^ data) & data;

	if(bits_up != 0)
	{
		m_latch_sel = m_miot_pa;
		// logerror("%s:i8155: Port A : Latch Select:%02x - Bits_up:%02x\n", machine().describe_context(), m_latch_sel, bits_up);
		latch_update();
	}

	//logerror("%s:i8155: Port A : Latch Select:%02x - from data:%02x\n", machine().describe_context(), m_latch_sel, data);
	m_miot_pa = data;
}

void rfslots8085_state::memiot_pb_w(u8 data)
{
	// logerror("%s:i8155: Port B Write: Data to latch:%02x\n",machine().describe_context(), data);
	m_datalatch = data;
}

void rfslots8085_state::rst65_w(u8 state)
{
	// i8155 timer out goes low when timer finishes counting.
	// i8085 rst_65 interrupt line is active on rising edge signal.
	// So, timer out signal from i8155 must be inverted, as shown in schematics.
	// Timer has 2.1 ms period.

	m_maincpu->set_input_line(I8085_RST65_LINE, state ? CLEAR_LINE : ASSERT_LINE);
	//logerror("I8155: irq state: %s - State:%02x - Time:%s\n", state ? "Clear Line":"Assert Line", state, machine().time().as_string());
}


/*********************************************
*                 I8255 PPI                  *
*                                            *
*********************************************/

void rfslots8085_state::ppi_pb_w(u8 data)
{
	// PPI Port B Data

	// Bit 0 .. Bit 3:   Sound Code
	// Bit 4: Sound cpu: /INT
	// Bit 5: n.u.
	// Bit 6: Data A (74LS138 Decoder).
	// Bit 7: Data B (74LS138 Decoder).

	m_soundcpu->set_input_line(INPUT_LINE_IRQ0, BIT(data, 4) ? CLEAR_LINE : ASSERT_LINE);
	m_sound_code = (data & 0x03f);
	//logerror("%s:i8255: Port B Write Sound code:%02x\n", machine().describe_context(), m_sound_code);

	// decode motors

	if(!BIT(ppi_pc, 0))  // Enable from port C
	{
		u8 deco = 0;
		deco = ((data >> 6) & 0x03);           // Get A,B decoder signals from PPI Port B, bits 5, 6.
		deco = deco + ((BIT(ppi_pc, 1) * 4));  // Get C decoder signal from port C

		if(deco == 2)  // hopper motor 25 Pts.
			m_lamps[52] = 1;

		if(deco == 5)  // hopper motor 100 Pts.
			m_lamps[53] = 1;

		if(deco == 7)  // bingo roller motor
		{
			m_lamps[54] = 1;
			m_lamps[18] = 0;
			m_reel->set_state(1);
		}

		m_hopper[0]->motor_w(m_lamps[52]);
		m_hopper[1]->motor_w(m_lamps[53]);

		// logerror("%s:i8255: Port B Write data:%02x - Deco:%02x\n", machine().describe_context(), data, deco);
	}
}


void rfslots8085_state::ppi_pc_w(u8 data)
{
	// PPI Port C Data(w) on Lower nibble

	// Bit 0: Decoder enable (4-5) /E2 /E3
	// Bit 1: Decoder data C (A-B Signals form Port B)
	// Bit 2: Sel Latch 9
	// Bit 3: Sel Latch 8

	// Rising edge detection on latch selectors 8 or 9 (Only Bits PC.2, PC.3)
	// First detect a change on bit, then detects if it´s a rising edge.

	u8 bits_up = ((ppi_pc & 0x0c) ^ (data & 0x0c)) & (data & 0x0c);
	//logerror("%s:i8255: Port C Write data:%02x - bits up:%02x\n", machine().describe_context(), data, bits_up);

	// Latch 8 Update (lamp 36-43) ( * = mismatch with bformula2 schematics)

	// Bit 0: Lamp "Pulse" central    *  (ex Coin lock coil 100 Pts.)
	// Bit 1: Fault lamp
	// Bit 2: n.u.                    *  (ex "NADA")
	// Bit 3: Coin lock coil 100 Pts. *  (ex "DOBLE")
	// Bit 4: Coin lock coil 25 Pts.  *  (ex "PULSE CENTRAL")
	// Bit 5: Lamp "DOBLE"            *  (ex n.u.
	// Bit 6: Lamp "NADA"             *  (ex "DEPOSITE MONEDA")
	// Bit 7: Lamp "DEPOSITE MONEDA"  *  (ex Coin lock coil 25 Pts.)

	if(BIT(bits_up, 3))  // Rising edge on PC.3 -> Latched data  belongs to latch 9
	{
		for(u8 i = 0; i < 8; i++)
			m_lamps[36 + i] = BIT(m_datalatch, i);
		// logerror("%s:i8255: Latch 8 Update  Data:%02x\n\n",machine().describe_context(), m_datalatch);
	}

	// Latch 9 Update - lamps 44-51 ( * = mismatch with bformula2 schematics)

	// Bit 0: Coin in 25 Pts counter.   *
	// Bit 1: Coin Out 25 Pts. counter  *
	// Bit 2: Coin in 100 Pts. counter  *
	// Bit 3: Coin Out 100 Pts. counter *
	// Bit 4: Topper ? lamp48   *
	// Bit 5: Topper ? lamp49   *
	// Bit 6: Topper ? lamp50   *
	// Bit 7: Topper ? lamp51   *

	if(BIT(bits_up, 2)) // Rising edge on PC.2 -> Latched data  belongs to latch 8
	{
		for(u8 i = 0; i < 8; i++)
			m_lamps[44 + i] = BIT(m_datalatch, i);
		machine().bookkeeping().coin_counter_w(0, BIT(m_datalatch, 0));  // Coin in 25 Pts. E.M. Counter
		machine().bookkeeping().coin_counter_w(1, BIT(m_datalatch, 2));  // Coin in 100 Pts. E.M. Counter
		machine().bookkeeping().coin_counter_w(2, BIT(m_datalatch, 1));  // Coin Out 25 Pts. E.M. Counter
		machine().bookkeeping().coin_counter_w(3, BIT(m_datalatch, 3));  // Coin Out 100 Pts. E.M. Counter

		m_lamps[63] = machine().bookkeeping().coin_counter_get_count(0);  // Coin in 25 Pts. E.M. Counter
		m_lamps[64] = machine().bookkeeping().coin_counter_get_count(1);  // Coin in 100 Pts. E.M. Counter
		m_lamps[65] = machine().bookkeeping().coin_counter_get_count(2);  // Coin Out 25 Pts. E.M. Counter
		m_lamps[66] = machine().bookkeeping().coin_counter_get_count(3);  // Coin Out 100 Pts. E.M. Counter

		// logerror("%s:i8255: Latch 9 Update  Data:%02x\n\n",machine().describe_context(), m_datalatch);
	}

	if(BIT(data, 0))  // Decoder disabled -> All motors off
	{
		m_hopper[0]->motor_w(0);
		m_hopper[1]->motor_w(0);
		m_reel->set_state(0);
		m_lamps[52] = 0;
		m_lamps[53] = 0;
		m_lamps[54] = 0;
	}

	ppi_pc = data;  // to the next rise/fall bit detection
}


/*********************************************
*      I8279 Keyboard-Disply Interface       *
*                                            *
*********************************************/

void rfslots8085_state::kbd_sl_w(u8 data)
{

//  Scan Line
	m_kbd_sl = data & 0x07;
	// logerror("I8279: Scan Line: %02X\n", m_kbd_sl);
}

u8 rfslots8085_state::kbd_rl_r()
{
//  Bingo Roller animator
	if(m_lamps[54] == 1)
	{
		m_lamps[18] = 109 * (1 + ( sin( (m_reel->get_pos() * 0.0628) - 1.57079)));  // layout scale
		m_lamps[19] = m_reel->get_pos() % 3;
//      logerror("%s:  Anim. Bingo Roller: L18:%d - L19:%02x - pos:%d\n", machine().time().as_string(), m_lamps[18], m_lamps[19], m_reel->get_pos());
	}

//  Keyboard read (only scan line 0 is used)
	u8 ret = 0xff;
	if((m_kbd_sl & 0x07) == 6)
		ret = ioport("IN0")->read();
	return ret;
}

void rfslots8085_state::disp_w(u8 data)
{
//  Display data
	data = bitswap(data, 4, 5, 6, 7, 0, 1, 2, 3);
	output_digit(m_kbd_sl, data >> 4);
	// logerror("I8279: Data Display: %02X\n", data);
}

void rfslots8085_state::output_digit(u8 i, u8 data)
{
//  Segment decode
	static const u8 led_map[16] =
		{ 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0x00 };

//  Show layout
//  The i8279 is configured to display 8 digits, so we need to use 8 "m_lamps" elements.
//  The actual hardware only has two 7-segment displays.

	m_lamps[55 + i] = led_map[data & 0x0f];  // lamps 55 to 62
}

void rfslots8085_state::rst55_w(u8 state)
{
//  KBD Interrupt
//  On each INT main cpu send commands:
//  0x3F -> CMD 1: (Set Clock divider) Value 0x1f: The highest divider, set clock to 64.52 Khz
//  0x04 -> CMD 0: (Progam Display and Kbd modes) Display Value=00: Display 8 digits, 8 bit char left entry. Kbd Value=04: Encoded Scan Sensor Matrix.
//  0x46 -> CMD 2: Read sensor RAM. Value=06: Start reading from address 0x06.
//  Then reads data (once) from data port.

	m_maincpu->set_input_line(I8085_RST55_LINE, state ? ASSERT_LINE : CLEAR_LINE);
	// logerror("I8279: irq state: %s - state:%02x time:%s\n", state ? "Assert Line":"Clear Line", state, machine().time().as_string());
}


/*********************************************
*               AY-3-8910 PSG                *
*                                            *
*********************************************/

void rfslots8085_state::ay0_porta_w(u8 data)
{
	// logerror("ay8910: Port A Write - data:%02x\n", data);  // unknown hardware variation. read notes
}


/*************************************************
*                Bingo Roller                    *
*************************************************/

void rfslots8085_state::add_em_reels(machine_config &config, int symbols, attotime period)
{
	std::set<uint16_t> detents;
	for(int i = 0; i < symbols; i++)
		detents.insert(i * STEPS_PER_SYMBOL);

	EM_REEL(config, m_reel, symbols * STEPS_PER_SYMBOL, detents, period);
	m_reel->set_direction(em_reel_device::dir::FORWARD);
}

int rfslots8085_state::reel_opto_r()
{
	uint16_t const pos = m_reel->get_pos();
	//logerror("%s:  Anim. Bingo Roller: Lee input %d\n", machine().time().as_string(), m_lamps[18]);
	return (pos == 99);
}


/**********************************************
*                 Input Ports                 *
*                                             *
**********************************************/

static INPUT_PORTS_START(rf53_3297)
	PORT_START("IN0")  // kbdc-RL port - scan line 6
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Arranque/Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )  PORT_NAME("Moneda/Coin 25 Pts")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Recupere/Recover (Change)") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Loteria/Lottery")           PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_NAME("sw hopper out 100 Pts")     PORT_READ_LINE_DEVICE_MEMBER("hopper1", ticket_dispenser_device, line_r)  // hopper 100 pts
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_NAME("sw hopper out 25 Pts")      PORT_READ_LINE_DEVICE_MEMBER("hopper0", ticket_dispenser_device, line_r)  // hopper 25 pts
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )  PORT_NAME("Moneda/Coin 100 Pts")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Falta/Fault")               PORT_CODE(KEYCODE_8)

	PORT_START("IN1")  // ppi - port A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Pulse Cent./ Central Push")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Pulse Izq./ Left Push")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Pulse Der./ Right Push")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Doblar/Double")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Cobrar/PayOut")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )      PORT_NAME("Lento/Slow")

	PORT_START("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM )       PORT_NAME("sw bingo roller")         PORT_READ_LINE_MEMBER(rfslots8085_state, reel_opto_r) // bingo roller
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("sw hopper load 100 Pts.") PORT_TOGGLE  PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("sw hopper load 25 Pts.")  PORT_TOGGLE  PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR ) PORT_NAME("Door & hopper out")       PORT_TOGGLE

	PORT_START("DSW") // 1 x 6-dips bank
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown))  PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPNAME(0x02, 0x02, "Test Mode")       PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown))  PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown))  PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown))  PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown))  PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x20, DEF_STR(Off))
INPUT_PORTS_END


/*********************************************
*              Machine Drivers               *
*                                            *
*********************************************/

void rfslots8085_state::rf53_3297(machine_config &config)
{
	I8085A(config, m_maincpu, MAIN_CLOCK);  // 6 MHz xtal on the technical manual, but 5.0688 MHz on the real PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &rfslots8085_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &rfslots8085_state::main_io_map);
	m_maincpu->in_sid_func().set(FUNC(rfslots8085_state::sid_r));

	I8035(config, m_soundcpu, SND_CLOCK );
	m_soundcpu->set_addrmap(AS_PROGRAM, &rfslots8085_state::sound_map);
	m_soundcpu->set_addrmap(AS_IO, &rfslots8085_state::sound_io_map);
	m_soundcpu->p1_in_cb().set(FUNC(rfslots8085_state::sound_p1_r));
	m_soundcpu->p1_out_cb().set(FUNC(rfslots8085_state::sound_p1_w));
	m_soundcpu->p2_out_cb().set(FUNC(rfslots8085_state::sound_p2_w));
	m_soundcpu->t1_in_cb().set([this]() { return BIT(rfslots8085_state::m_sound_code, 5); });

	I8155(config, m_memiot, MAIN_CLOCK / 2);  // derived from main cpu clock out (half XTAL frec derived from internal flip flop)
	m_memiot->out_pa_callback().set(FUNC(rfslots8085_state::m_miot_pa_w));
	m_memiot->out_pb_callback().set(FUNC(rfslots8085_state::memiot_pb_w));
	m_memiot->out_to_callback().set(FUNC(rfslots8085_state::rst65_w));
	m_memiot->in_pc_callback().set_ioport("DSW");

	I8255(config, m_ppi);
	m_ppi->out_pb_callback().set(FUNC(rfslots8085_state::ppi_pb_w));
	m_ppi->out_pc_callback().set(FUNC(rfslots8085_state::ppi_pc_w));
	m_ppi->in_pa_callback().set_ioport("IN1");
	m_ppi->in_pc_callback().set_ioport("IN2");

	I8212(config, "i8212");

	I8279(config, m_kbdc, SND_CLOCK / 3);  // derived from sound cpu line T0
	m_kbdc->out_sl_callback().set(FUNC(rfslots8085_state::kbd_sl_w));  // scan SL lines
	m_kbdc->out_disp_callback().set(FUNC(rfslots8085_state::disp_w));  // display A&B
	m_kbdc->in_rl_callback().set(FUNC(rfslots8085_state::kbd_rl_r));   // kbd RL lines
	m_kbdc->out_irq_callback().set(FUNC(rfslots8085_state::rst55_w));  // irq line

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	HOPPER(config, m_hopper[0], attotime::from_msec(150));  // hopper motor 25 Pts.
	HOPPER(config, m_hopper[1], attotime::from_msec(150));  // hopper motor 100 Pts.

	add_em_reels(config, 100, attotime::from_double(2));

	config.set_default_layout(layout_bbombo);

	SPEAKER(config, "mono").front_center();

	DAC_3BIT_BINARY_WEIGHTED(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 2.0);

	AY8910(config, m_aysnd[0], SND_CLOCK / 3).add_route(ALL_OUTPUTS, "mono", 0.50);  // derived from sound cpu line T0
	m_aysnd[0]->port_a_write_callback().set(FUNC(rfslots8085_state::ay0_porta_w));

	AY8910(config, m_aysnd[1], SND_CLOCK / 3).add_route(ALL_OUTPUTS, "mono", 0.50);  // derived from sound cpu line T0
}


/*********************************************
*                  Rom Load                  *
*                                            *
*********************************************/

// Baby & Bombo
ROM_START(bbombo)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("m1-31_b_1704.ic32", 0x0000, 0x4000, CRC(a74a85b7) SHA1(f562495a6b97f34165cc9fd5c750664701cac21f))

	ROM_REGION(0x1000, "audiocpu", 0)
	ROM_LOAD("8a.ic44", 0x0000, 0x1000, CRC(51b564b6) SHA1(8992a5cb4dff8c6b38b77a7e0199a71f2969b496))
	ROM_IGNORE(                 0x3000) // 0xff filled and it's outside of the 8035's global address mask (fff)
ROM_END

/*
  Baby Formula 2.
  Uses also the following Recreativos Franco boards:
   -53/3378 for hoppers.
   -53/3379 for the mechanical "bingo roller".
   -53/3385 and 53/3380 for displays.

  A complete manual with schematics can be downloaded from https://www.recreativas.org/manuales
*/
ROM_START(bformula2)
	ROM_REGION(0x8000, "maincpu", 0)  // ROM lines D2 and D5 are stuck to 0. the device needs to be redumped.
	ROM_LOAD("recreativos_franco_m-000031-b_001744_90_pc.ic32", 0x0000, 0x8000, BAD_DUMP CRC(b593c6bc) SHA1(c01b61b4ea60b9f2bca3200b4b90af1c4027f9df))

	ROM_REGION(0x8000, "audiocpu", 0)  // ROM lines D2 and D5 are stuck to 0. the device needs to be redumped.
	ROM_LOAD("recreativos_franco_m-000031-b_001744_90_pa.ic44", 0x0000, 0x8000, BAD_DUMP CRC(3c4ae129) SHA1(e68e7a403596a9f2d34e6b8d2631d2716010e982))
ROM_END

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*                                            *
*********************************************/

//   YEAR   NAME       PARENT MACHINE    INPUT      CLASS               INIT        ROT   COMPANY               FULLNAME           FLAGS
GAME(1987?, bbombo,    0,     rf53_3297, rf53_3297, rfslots8085_state,  empty_init, ROT0, "Recreativos Franco", "Baby & Bombo",    MACHINE_MECHANICAL)  // Date "25-05-87" engraved on the PCB
GAME(1988,  bformula2, 0,     rf53_3297, rf53_3297, rfslots8085_state,  empty_init, ROT0, "Recreativos Franco", "Baby Formula 2",  MACHINE_NOT_WORKING | MACHINE_MECHANICAL)  // Year from legal registry date
