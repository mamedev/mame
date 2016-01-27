// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, James Wallace
/***************************************************************************

    Nova 'LaserMax'/Atari Games Cops
    (hardware developed by Nova Productions Limited)
    Preliminary driver by Mariusz Wojcieszek, James Wallace

    Cops uses a Sony CD-ROM in addition to the regular setup, purely to play
    Bad Boys by Inner Circle, so there is muscial accompaniment to areas
    where the laserdisc audio is muted.

    NOTES: To boot up Revelations, turn the refill key (R) and press button A.
    TODO: There are probably more ROMs for Revelations, the disc contains
    full data for a picture based memory game called 'Vision Quest'.

    LaserMax memory map needs sorting out, Cops uses a subset of what's
    actually available

    The UK version COPS appears to want to communicate with the LDP in a
    different way.
***************************************************************************/


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "sound/sn76496.h"

//#include "machine/mos6551.h"

#include "cops.lh"

#define LOG_CDROM   0
#define LOG_DACIA   0

#define CMP_REGISTER 0
#define AUX_REGISTER 1

#define MAIN_CLOCK XTAL_4MHz

class cops_state : public driver_device
{
public:
	cops_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_sn(*this, "snsnd"),
			m_irq(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<sn76489_device> m_sn;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

public:
	DECLARE_WRITE8_MEMBER(io1_w);
	DECLARE_READ8_MEMBER(io1_r);
	DECLARE_WRITE8_MEMBER(io2_w);
	DECLARE_READ8_MEMBER(io2_r);
	DECLARE_WRITE_LINE_MEMBER(via1_irq);
	DECLARE_WRITE_LINE_MEMBER(via2_irq);
	void dacia_receive(UINT8 data);
	void update_dacia_irq();
	DECLARE_WRITE8_MEMBER(dacia_w);
	DECLARE_READ8_MEMBER(dacia_r);
	DECLARE_WRITE8_MEMBER(via1_b_w);
	DECLARE_WRITE8_MEMBER(via1_cb1_w);
	DECLARE_WRITE8_MEMBER(cdrom_data_w);
	DECLARE_WRITE8_MEMBER(cdrom_ctrl_w);
	DECLARE_READ8_MEMBER(cdrom_data_r);
	DECLARE_DRIVER_INIT(cops);
	int m_irq;

	UINT8 m_lcd_addr_l, m_lcd_addr_h;
	UINT8 m_lcd_data_l, m_lcd_data_h;

	UINT8 m_dacia_irq1_reg;
	UINT8 m_dacia_rts1;
	UINT8 m_dacia_dtr1;
	UINT8 m_parity_1;
	UINT8 m_parity_mode_1;
	UINT8 m_bpc_1;
	int m_dacia_ic_div_1;
	UINT8 m_dacia_echo1;
	UINT8 m_dacia_stp_1;
	UINT8 m_dacia_reg1;
	UINT8 m_dacia_fe1;
	UINT8 m_dacia_cmp1;
	UINT8 m_dacia_cmpval1;

	UINT8 m_dacia_cts;
	UINT8 m_dacia_dcd;
	UINT8 m_dacia_trans;

	UINT8 m_dacia_receiver_data;
	UINT8 m_dacia_receiver_full;

	UINT8 m_cdrom_ctrl;
	UINT8 m_cdrom_data;

	UINT8 m_sn_data;
	UINT8 m_sn_cb1;

	// LDP-1450
	UINT8 m_ld_command_to_send[5];
	UINT8 m_ld_command_total_bytes;
	UINT8 m_ld_command_current_byte;
	UINT8 m_ld_frame[5];
	UINT8 m_ld_frame_index;
	emu_timer *m_ld_timer;
	TIMER_CALLBACK_MEMBER(ld_timer_callback);
	enum LD_INPUT_STATE
	{
		LD_INPUT_GET_COMMAND = 0,
		LD_INPUT_TEXT_COMMAND,
		LD_INPUT_TEXT_GET_X,
		LD_INPUT_TEXT_GET_Y,
		LD_INPUT_TEXT_GET_MODE,
		LD_INPUT_TEXT_GET_STRING,
		LD_INPUT_TEXT_GET_SET_WINDOW
	} m_ld_input_state;

	UINT8 generate_isr();
	void laserdisc_w(UINT8 data);
	void laserdisc_response_w(UINT8 data);
	DECLARE_PALETTE_INIT( cops );
};

const int timer_divide_select[16] =
{
	73728,
	33538,
	27408,
	24576,
	12288,
	6144,
	3072,
	2048,
	1536,
	1024,
	768,
	512,
	384,
	192,
	96,
	16
};

void cops_state::video_start()
{
}

UINT32 cops_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

/*************************************
 *
 * Sony CDU33A-02 CDROM
 *
 *************************************/

WRITE8_MEMBER(cops_state::cdrom_data_w)
{
	const char *regs[4] = { "CMD", "PARAM", "WRITE", "CTRL" };
	m_cdrom_data = BITSWAP8(data,0,1,2,3,4,5,6,7);
	UINT8 reg = ((m_cdrom_ctrl & 4) >> 1) | ((m_cdrom_ctrl & 8) >> 3);
	if (LOG_CDROM) logerror("%s:cdrom_data_w(reg = %s, data = %02x)\n", machine().describe_context(), regs[reg & 0x03], m_cdrom_data);
}

WRITE8_MEMBER(cops_state::cdrom_ctrl_w)
{
	if (LOG_CDROM) logerror("%s:cdrom_ctrl_w(%02x)\n", machine().describe_context(), data);
	m_cdrom_ctrl = data;
}

READ8_MEMBER(cops_state::cdrom_data_r)
{
	const char *regs[4] = { "STATUS", "RESULT", "READ", "FIFOST" };
	UINT8 reg = ((m_cdrom_ctrl & 4) >> 1) | ((m_cdrom_ctrl & 8) >> 3);
	if (LOG_CDROM) logerror("%s:cdrom_data_r(reg = %s)\n", machine().describe_context(), regs[reg & 0x03]);
	return machine().rand()&0xff;
}
/*************************************
 *
 * LDP-1450 Laserdisc
 *
 *************************************/

TIMER_CALLBACK_MEMBER(cops_state::ld_timer_callback)
{
	m_dacia_receiver_full = 1;

	if ( m_ld_command_current_byte < m_ld_command_total_bytes )
	{
		dacia_receive(m_ld_command_to_send[m_ld_command_current_byte]);
		m_ld_command_current_byte++;
		if ( m_ld_command_current_byte == m_ld_command_total_bytes )
		{
			m_ld_command_current_byte = m_ld_command_total_bytes = 0;
		}
	}
}

void cops_state::laserdisc_response_w(UINT8 data)
{
	if ( m_ld_command_total_bytes >= 5 )
	{
		logerror( "LD Overflow!\n" );
	}
	m_ld_command_to_send[m_ld_command_total_bytes++] = data;
}

void cops_state::laserdisc_w(UINT8 data)
{
	switch( m_ld_input_state )
	{
		case LD_INPUT_TEXT_GET_X:
			m_ld_input_state = LD_INPUT_TEXT_GET_Y;
			break;
		case LD_INPUT_TEXT_GET_Y:
			m_ld_input_state = LD_INPUT_TEXT_GET_MODE;
			break;
		case LD_INPUT_TEXT_GET_MODE:
			m_ld_input_state = LD_INPUT_GET_COMMAND;
			break;
		case LD_INPUT_TEXT_GET_SET_WINDOW:
			m_ld_input_state = LD_INPUT_GET_COMMAND;
			break;
		case LD_INPUT_TEXT_GET_STRING:
			if ( data == 0x1a )
			{
				m_ld_input_state = LD_INPUT_GET_COMMAND;
			}
			break;
		case LD_INPUT_TEXT_COMMAND:
		case LD_INPUT_GET_COMMAND:
			{
				switch( data )
				{
					case 0x00: /* text handling (start gotoxy) */
						if ( m_ld_input_state == LD_INPUT_TEXT_COMMAND )
						{
							m_ld_input_state = LD_INPUT_TEXT_GET_X;
						}
						break;
					case 0x01: /* text handling (end of text)*/
						if ( m_ld_input_state == LD_INPUT_TEXT_COMMAND )
						{
							m_ld_input_state = LD_INPUT_TEXT_GET_STRING;
						}
						break;
					case 0x02: /* text 'set window' command */
						if ( m_ld_input_state == LD_INPUT_TEXT_COMMAND )
						{
							m_ld_input_state = LD_INPUT_TEXT_GET_SET_WINDOW;
						}
						break;
					case 0x1a: /* text sent */
						break;
					case 0x24: /* Audio On */
						laserdisc_response_w(0x0a);
						break;
					case 0x26: /* Video off */
						laserdisc_response_w(0x0a);
						break;
					case 0x27: /* Video on */
						laserdisc_response_w(0x0a);
						break;
					case 0x30: /* Digit */
					case 0x31:
					case 0x32:
					case 0x33:
					case 0x34:
					case 0x35:
					case 0x36:
					case 0x37:
					case 0x38:
					case 0x39:
						if ( m_ld_frame_index >= 5 )
						{
							m_ld_frame_index = 0;
						}
						m_ld_frame[m_ld_frame_index++] = data;
						laserdisc_response_w(0x0a);
						break;
					case 0x3a: /* Play (answer should have delay) */
						laserdisc_response_w(0x0a);
						break;
					case 0x3f: /* Stop */
						laserdisc_response_w(0x0a);
						break;
					case 0x40: /* Enter */
						laserdisc_response_w(0x0a);
						break;
					case 0x43: /* Search */
						laserdisc_response_w(0x0a);
						break;
					case 0x46: /* Channel 1 on */
						laserdisc_response_w(0x0a);
						break;
					case 0x47: /* Channel 1 off */
						laserdisc_response_w(0x0a);
						break;
					case 0x48: /* Channel 2 on */
						laserdisc_response_w(0x0a);
						break;
					case 0x49: /* Channel 2 off */
						laserdisc_response_w(0x0a);
						break;
					case 0x4f: /* Still */
						laserdisc_response_w(0x0a);
						break;
					case 0x55: /* 'frame mode' (unknown function) */
						break;
					case 0x56: /* C. L. (Reset) */
						m_ld_input_state = LD_INPUT_GET_COMMAND;
						laserdisc_response_w(0x0a);
						break;
					case 0x60: /* Addr Inq (get current frame number) */
						for (auto & elem : m_ld_frame)
						{
							laserdisc_response_w(elem);
						}
						break;
					case 0x80: /* text start */
						m_ld_input_state = LD_INPUT_TEXT_COMMAND;
						break;
					case 0x81: /* Turn on text */
						break;
					case 0x82: /* Turn off text */
						break;
					default:
						logerror("Laserdisc command %02x\n", data);
						break;
				}
			}
			break;
	}
}

/*************************************
 *
 * 6552 DACIA
 *
 *************************************/

	void cops_state::update_dacia_irq()
{
	UINT8 isr = generate_isr();
	//remove bits
	isr &= ~m_dacia_irq1_reg;
	m_maincpu->set_input_line(INPUT_LINE_NMI, isr? ASSERT_LINE:CLEAR_LINE);
}

void cops_state::dacia_receive(UINT8 data)
{
	if (m_dacia_cmp1)
	{
		if (m_dacia_cmpval1 == data)
		{
			m_dacia_receiver_data = data;
			m_dacia_receiver_full = 1;
			update_dacia_irq();
			m_dacia_cmp1 =0;
			m_dacia_cts =1;
			m_dacia_trans =1;
		}
	}
	else
	{
		m_dacia_receiver_data = data;
		m_dacia_receiver_full = 1;
		update_dacia_irq();
		m_dacia_cts =1;
		m_dacia_trans =1;
	}
}

UINT8 cops_state::generate_isr()
{
	UINT8 isr =0;

	isr |= m_dacia_receiver_full;
	isr |= (m_dacia_cmp1 << 1);
	isr |= (m_dacia_trans <<4);

	if (isr)
	{
		isr |= 0x40;
	}
	return isr;
}

READ8_MEMBER(cops_state::dacia_r)
{
	switch(offset & 0x07)
	{
		case 0: /* ISR1: Interrupt Status Register */
		{
			UINT8 isr = generate_isr();
			m_dacia_trans =0;
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			return isr;
		}

		case 1: /* CSR1: Control Status Register */
		{
			UINT8 csr =0;
			csr |= m_dacia_rts1;
			csr |= (m_dacia_dtr1 << 1);
			csr |= (m_dacia_cts <<4);
			csr |= (m_dacia_fe1 <<7);
			if (LOG_DACIA) logerror("CSR1 %02x\n",csr);
			return csr;
		}

		case 3: /* RDR1: Receive data register */
			m_dacia_receiver_full = 0;
			m_dacia_fe1=0;
			if (LOG_DACIA) logerror("RDR1 %02x\n",m_dacia_receiver_data);
			return m_dacia_receiver_data;
		default:
			if (LOG_DACIA) logerror("%s:dacia_r(%02x)\n", machine().describe_context(), offset);
			return 0;
	}
}

WRITE8_MEMBER(cops_state::dacia_w)
{
	switch(offset & 0x07)
	{
		case 0: /* IRQ enable Register 1 */
		{
			m_dacia_irq1_reg &= ~0x80;

			if (data & 0x80) //enable bits
			{
				m_dacia_irq1_reg |= (data & 0x7f);
			}
			else // disable bits
			{
				m_dacia_irq1_reg &= ~(data & 0x7f);
			}
			if (LOG_DACIA) logerror("DACIA IRQ 1 Register: %02x\n", m_dacia_irq1_reg);
			update_dacia_irq();
			break;
		}

		case 1: /* Control / Format Register 1 */
		{
			if (data & 0x80) //Format Register
			{
				m_dacia_rts1 = (data & 0x01);
				m_dacia_dtr1 = (data & 0x02 ? 1:0);
				m_parity_1 = (data & 0x04);
				m_parity_mode_1 = ((data & 0x18) >> 3);
				m_bpc_1 = ((data & 0x60) >> 5) +5;
				if (LOG_DACIA) logerror("DACIA Format Register: %02x\n", data);
			}
			else // Control register
			{
				m_dacia_ic_div_1 = timer_divide_select[data & 0x15];
				m_dacia_echo1 = (data & 0x10);
				m_dacia_stp_1 = (data & 0x20 ? 2:1);
				if (data & 0x40)
				{
					m_dacia_reg1 = AUX_REGISTER;
				}
				else
				{
					m_dacia_reg1 = CMP_REGISTER;
				}
				if (LOG_DACIA) logerror("DACIA TIME %02d\n", XTAL_3_6864MHz / m_dacia_ic_div_1);

				m_ld_timer->adjust(attotime::from_hz(XTAL_3_6864MHz / m_dacia_ic_div_1), 0, attotime::from_hz(XTAL_3_6864MHz / m_dacia_ic_div_1));

				if (LOG_DACIA) logerror("DACIA Ctrl Register: %02x\n", data);

			}
			break;
		}
		case 2: /* Compare / Aux Ctrl Register 1 */
		{
			if (m_dacia_reg1 == CMP_REGISTER)
			{
				m_dacia_cmp1 =1;
				m_dacia_cmpval1=data;
				if (LOG_DACIA) logerror("DACIA Compare mode: %02x \n", data);
//              update_dacia_irq();
			}
			else
			{
				if (LOG_DACIA) logerror("DACIA Aux ctrl: %02x \n", data);
			}
		}
		case 3: /* Transmit Data Register 1 */
			if (LOG_DACIA) logerror("DACIA Transmit: %02x %c\n", data, (char)data);
			laserdisc_w(data);
			break;
		default:
			if (LOG_DACIA) logerror("%s:dacia_w(%02x,%02x)\n", machine().describe_context(), offset, data);
			break;
	}
}

/*************************************
 *
 * I/O
 *
 *************************************/

READ8_MEMBER(cops_state::io1_r)
{
	switch( offset & 0x0f )
	{
		case 0x08:  /* SW0 */
			return ioport("SW0")->read();
		case 0x09:  /* SW1 */
			return ioport("SW1")->read();
		case 0x0a:  /* SW2 */
			return ioport("SW2")->read();
		default:
			logerror("Unknown io1_r, offset = %03x\n", offset);
			return 0;
	}
}

WRITE8_MEMBER(cops_state::io1_w)
{
	int i;
	char output_name[16];
	UINT16 display_data;

	switch( offset & 0x0f )
	{
		case 0x00: /* WOP0 Alpha display*/
			m_lcd_addr_l = data;
			break;
		case 0x01: /* WOP1 Alpha display*/
			m_lcd_addr_h = data;
			{
				// update display
				const UINT16 addrs_table[] = { 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0002, 0x0001, 0x0080,
												0x1000, 0x0800, 0x0400, 0x2000, 0x4000, 0x0200, 0x0100, 0x8000 };
				UINT16 addr = m_lcd_addr_l | (m_lcd_addr_h << 8);
				for (i = 0; i < 16; i++ )
				{
					if (addr == addrs_table[i])
					{
						break;
					}
				}

				if (i < 16)
				{
					sprintf(output_name, "digit%d", i);
					display_data = m_lcd_data_l | (m_lcd_data_h << 8);
					display_data = BITSWAP16(display_data, 4, 5, 12, 1, 0, 11, 10, 6, 7, 2, 9, 3, 15, 8, 14, 13);
					output().set_value(output_name, display_data);
				}
			}
			break;
		case 0x02: /* WOP2 Alpha display*/
			m_lcd_data_l = data;
			break;
		case 0x03: /* WOP3 Alpha display*/
			m_lcd_data_h = data;
			break;
		case 0x04: /* WOP4 */
			output().set_value("Offroad Right 4 Lamp", data & 0x80);
			output().set_value("Offroad Right 3 Lamp", data & 0x40);
			output().set_value("Offroad Right 2 Lamp", data & 0x20);
			output().set_value("Offroad Right 1 Lamp", data & 0x10);
			output().set_value("Offroad Left 4 Lamp", data & 0x08);
			output().set_value("Offroad Left 3 Lamp", data & 0x04);
			output().set_value("Offroad Left 2 Lamp", data & 0x02);
			output().set_value("Offroad Left 1 Lamp", data & 0x01);
			break;
		case 0x05: /* WOP5 */
			output().set_value("Damage Lamp", data & 0x80);
			output().set_value("Stop Lamp", data & 0x40);
			output().set_value("Gun Active Right Lamp", data & 0x20);
			output().set_value("Vest Hit 2 Lamp", data & 0x10);
			output().set_value("Vest Hit 3 Lamp", data & 0x04);
			output().set_value("Gun Active Left Lamp", data & 0x02);
			output().set_value("Vest Hit 1 Lamp", data & 0x01);
			break;
		case 0x06: /* WOP6 */
			logerror("WOP6: data = %02x\n", data);
			break;
		case 0x07: /* WOP? */
			logerror("WOP7: data = %02x\n", data);
			break;
		case 0x08: /* WOP0 */
			logerror("WOP8: data = %02x\n", data);
			break;
		default:
			logerror("Unknown io1_w, offset = %03x, data = %02x\n", offset, data);
			break;
	}
}

READ8_MEMBER(cops_state::io2_r)
{
	switch( offset & 0x0f )
	{
		case 0x03:
			return ioport("STEER")->read();
		default:
			logerror("Unknown io2_r, offset = %02x\n", offset);
			return 0;
	}
}

WRITE8_MEMBER(cops_state::io2_w)
{
	switch( offset & 0x0f )
	{
		case 0x02:
			output().set_value("Flash Red Lamp", data & 0x01);
			output().set_value("Flash Blue Lamp", data & 0x80);
			if ( data & ~0x91 ) logerror("Unknown io2_w, offset = %02x, data = %02x\n", offset, data);
			break;
		case 0x04:
			output().set_value("Bullet Lamp 6", data & 0x20);
			output().set_value("Bullet Lamp 5", data & 0x10);
			output().set_value("Bullet Lamp 4", data & 0x08);
			output().set_value("Bullet Lamp 3", data & 0x04);
			output().set_value("Bullet Lamp 2", data & 0x02);
			output().set_value("Bullet Lamp 1", data & 0x01);
			if ( data & ~0x3f ) logerror("Unknown io2_w, offset = %02x, data = %02x\n", offset, data);
			break;
		default:
			logerror("Unknown io2_w, offset = %02x, data = %02x\n", offset, data);
			break;
	}
}

/*************************************
 *
 *  VIA 1 (U18)
 *   PA0-2 Steer
 *   PA3   ?
 *   PA4-6 Fade?
 *   PA7   STK (system rom banking)
 *   PB0-7 SN76489 data bus
 *   CA1-2 n.c.
 *   CB1   /WE SN76489
 *   IRQ   IRQ
 *
 *************************************/

WRITE_LINE_MEMBER(cops_state::via1_irq)
{
	if ( state == ASSERT_LINE )
	{
		m_irq |= 1;
	}
	else
	{
		m_irq &= ~1;
	}
	m_maincpu->set_input_line(M6502_IRQ_LINE, m_irq ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(cops_state::via1_b_w)
{
	m_sn_data = BITSWAP8(data,0,1,2,3,4,5,6,7);
	if (m_sn_cb1)
	{
		m_sn->write(space,0,m_sn_data);
	}
}

WRITE8_MEMBER(cops_state::via1_cb1_w)
{
	m_sn_cb1 = data;
}

/*************************************
 *
 *  VIA 2 (U27)
 *   PA0-7 GUN
 *   PB0-7 GUN
 *   IRQ   IRQ
 *
 *************************************/

WRITE_LINE_MEMBER(cops_state::via2_irq)
{
	if ( state == ASSERT_LINE )
	{
		m_irq |= 2;
	}
	else
	{
		m_irq &= ~2;
	}
	m_maincpu->set_input_line(M6502_IRQ_LINE, m_irq ? ASSERT_LINE : CLEAR_LINE);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( cops_map, AS_PROGRAM, 8, cops_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x9fff) AM_ROM AM_REGION("program", 0)
	AM_RANGE(0xa000, 0xafff) AM_READWRITE(io1_r, io1_w)
	AM_RANGE(0xb000, 0xb00f) AM_DEVREADWRITE("via6522_1", via6522_device, read, write)  /* VIA 1 */
	AM_RANGE(0xb800, 0xb80f) AM_DEVREADWRITE("via6522_2", via6522_device, read, write)  /* VIA 2 */
	AM_RANGE(0xc000, 0xcfff) AM_READWRITE(io2_r, io2_w)
//  AM_RANGE(0xd000, 0xd003) AM_DEVREADWRITE("acia6551_1", mos6551_device, read, write )
//  AM_RANGE(0xd004, 0xd007) AM_DEVREADWRITE("acia6551_2", mos6551_device, read, write )
	AM_RANGE(0xd000, 0xd007) AM_READWRITE(dacia_r, dacia_w)
	AM_RANGE(0xd800, 0xd80f) AM_DEVREADWRITE("via6522_3", via6522_device, read, write)  /* VIA 3 */
	AM_RANGE(0xe000, 0xffff) AM_ROMBANK("sysbank1")
ADDRESS_MAP_END

static INPUT_PORTS_START( cops )
	PORT_START("SW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Switch A") PORT_CODE(KEYCODE_A) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Switch C") PORT_CODE(KEYCODE_C) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PROGRAM") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Switch B") PORT_CODE(KEYCODE_B) PORT_IMPULSE(1)

	PORT_START("SW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // N.C.
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Gas pedal
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("20P LEVEL") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("100P LEVEL") PORT_CODE(KEYCODE_W)

	PORT_START("SW2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
INPUT_PORTS_END

static INPUT_PORTS_START( revlatns )
	PORT_START("SW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("COLLECT")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN5 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Continue")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")

	PORT_START("SW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("20P LEVEL") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("*")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("100P LEVEL") PORT_CODE(KEYCODE_W)

	PORT_START("SW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("50p")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("10p")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("100p")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )

INPUT_PORTS_END

void cops_state::machine_start()
{
	m_ld_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cops_state::ld_timer_callback),this));
	m_dacia_ic_div_1 = timer_divide_select[0];

	m_ld_timer->adjust(attotime::from_hz(167*5), 0, attotime::from_hz(167*5));
}

void cops_state::machine_reset()
{
	m_irq = 0;
	m_lcd_addr_l = m_lcd_addr_h = 0;
	m_lcd_data_l = m_lcd_data_h = 0;

	m_dacia_cts = 0;
	m_dacia_dcd = 0;

	m_dacia_irq1_reg = 0x80;
	m_dacia_rts1 = 1;
	m_dacia_dtr1 = 1;
	m_dacia_fe1 = 1;
	m_dacia_receiver_full = 1;
	m_ld_input_state = LD_INPUT_GET_COMMAND;
	m_ld_command_current_byte = m_ld_command_total_bytes = 0;
	m_ld_frame_index = 0;
}


PALETTE_INIT_MEMBER( cops_state,cops )
{
}

DRIVER_INIT_MEMBER(cops_state,cops)
{
	//The hardware is designed and programmed to use multiple system ROM banks, but for some reason it's hardwired to bank 2.
	//For documentation's sake, here's the init
	UINT8 *rom = memregion("system")->base();
	membank("sysbank1")->configure_entries(0, 4, &rom[0x0000], 0x2000);
	membank("sysbank1")->set_entry(2);
}

static MACHINE_CONFIG_START( cops, cops_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6502,MAIN_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(cops_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(cops_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(cops_state,cops)

	/* via */
	MCFG_DEVICE_ADD("via6522_1", VIA6522, 0)
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(cops_state, via1_irq))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(cops_state, via1_b_w))
	MCFG_VIA6522_CB1_HANDLER(WRITE8(cops_state, via1_cb1_w))

	MCFG_DEVICE_ADD("via6522_2", VIA6522, 0)
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(cops_state, via2_irq))

	MCFG_DEVICE_ADD("via6522_3", VIA6522, 0)
	MCFG_VIA6522_READPA_HANDLER(READ8(cops_state, cdrom_data_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(cops_state, cdrom_data_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(cops_state, cdrom_ctrl_w))

	/* acia */
//  MCFG_MOS6551_ADD("acia6551_1", XTAL_1_8432MHz, NULL)
//  MCFG_MOS6551_ADD("acia6551_2", XTAL_1_8432MHz, NULL)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

		/* TODO: Verify clock */
	MCFG_SOUND_ADD("snsnd", SN76489, MAIN_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cops )
	ROM_REGION( 0x8000, "program", 0 )
	ROM_LOAD( "cops_prg.dat", 0x0000, 0x8000, CRC(a5c02366) SHA1(b135d72fcfe737a113c984b0b8dd78428f248414) )

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "cops_sys.dat", 0x0000, 0x8000, CRC(0060e5d0) SHA1(b8c9f6fde6a315e33fa7946e5d3bb4ea2fbe76a8) )

	DISK_REGION( "audiocd" )
		DISK_IMAGE_READONLY( "copscd", 0, NO_DUMP )

	DISK_REGION( "laserdisc" )
		DISK_IMAGE_READONLY( "cops", 0, NO_DUMP )
ROM_END

ROM_START( copsuk )
	ROM_REGION( 0x8000, "program", 0 )
	ROM_LOAD( "cops1b_uk.bin", 0x0000, 0x8000, CRC(f095ee95) SHA1(86bb517331d81ae3a8f3b87df67c321013c6aae4) )

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "cops_sys.dat", 0x0000, 0x8000, CRC(0060e5d0) SHA1(b8c9f6fde6a315e33fa7946e5d3bb4ea2fbe76a8) )

	DISK_REGION( "audiocd" )
		DISK_IMAGE_READONLY( "copscd", 0, NO_DUMP )

	DISK_REGION( "laserdisc" )
		DISK_IMAGE_READONLY( "cops", 0, NO_DUMP )
ROM_END

ROM_START( revlatns )
	ROM_REGION( 0x8000, "program", 0 )
	ROM_LOAD( "revelations_prog.bin", 0x0000, 0x8000, CRC(5ab41ac3) SHA1(0f7027551da17011576cf077e2f199729bb10482) )

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "revelations_sys.bin", 0x0000, 0x8000, CRC(43e5e3ec) SHA1(fa44b102b5aa7ad2421c575abdc67f1c29f23bc1) )

	DISK_REGION( "laserdisc" )
		DISK_IMAGE_READONLY( "revlatns", 0, NO_DUMP )
ROM_END


GAMEL( 1994, cops,      0,   cops,  cops,      cops_state, cops,       ROT0, "Atari Games",                     "Cops (USA)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_cops )
GAMEL( 1994, copsuk,    cops,cops,  cops,      cops_state, cops,       ROT0, "Nova Productions / Deith Leisure","Cops (UK)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_cops )
GAMEL( 1994, revlatns,  0,   cops,  revlatns,  cops_state, cops,       ROT0, "Nova Productions",                "Revelations",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_cops )
