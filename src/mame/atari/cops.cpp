// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, James Wallace
/***************************************************************************

    Nova Laserdisc Games/Atari Games COPS
    (hardware developed by Nova Productions Limited)
    Preliminary driver by Mariusz Wojcieszek, James Wallace

    Cops uses a Sony CD-ROM in addition to the regular setup, purely to play
    Bad Boys by Inner Circle, so there is musical accompaniment to areas
    where the laserdisc audio is muted.

    The different games here have subtly different control PCBs COPS has an Atari
    part number (58-12B), while Revelations simply refers to a Lasermax control PCB
    (Lasermax being the consumer name for the LDP series)

    NOTES: To boot up Revelations, turn the refill key (R) and press button A
    to init NVRAM.

    TODO: There are probably more ROMs for Revelations and related, the disc
    contains full data for a picture based memory game called 'Vision Quest'.
    However, the Vision Quest Laserdisc is slightly different, with Revelations
    specific data seemingly replaced with black level.

    The UK version COPS appears to want to communicate with the LDP in a
    different way, passing illegal commands, need to verify this is the same
    player.

    This should be similar hardware for Street Viper if we get a dump.
***************************************************************************/


#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/ldp1450.h"
#include "machine/mos6551.h"
#include "machine/msm6242.h"
#include "sound/sn76496.h"

#include "speaker.h"

#include "cops.lh"


namespace {

#define LOG_CDROM   1
#define LOG_DACIA   1

#define CMP_REGISTER 0
#define AUX_REGISTER 1

#define MAIN_CLOCK XTAL(4'000'000)

class cops_state : public driver_device
{
public:
	cops_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sn(*this, "snsnd")
		, m_ld(*this, "laserdisc")
		, m_switches(*this, "SW%u", 0U)
		, m_steer(*this, "STEER")
		, m_digits(*this, "digit%u", 0U)
		, m_irq(0)
	{ }

	void revlatns(machine_config &config);
	void base(machine_config &config);
	void cops(machine_config &config);
	void cops_map(address_map &map);
	void revlatns_map(address_map &map);

	void init_cops();

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<sn76489_device> m_sn;
	required_device<sony_ldp1450_device> m_ld;
	required_ioport_array<3> m_switches;
	optional_ioport m_steer;
	output_finder<16> m_digits;

	// screen updates
	[[maybe_unused]] uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io1_w(offs_t offset, uint8_t data);
	uint8_t io1_r(offs_t offset);
	uint8_t io1_lm_r(offs_t offset);
	void io2_w(offs_t offset, uint8_t data);
	uint8_t io2_r(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(dacia_irq);
	[[maybe_unused]] DECLARE_WRITE_LINE_MEMBER(ld_w);
	DECLARE_WRITE_LINE_MEMBER(via1_irq);
	DECLARE_WRITE_LINE_MEMBER(via2_irq);
	void dacia_receive(uint8_t data);
	void update_dacia_irq();
	void dacia_w(offs_t offset, uint8_t data);
	uint8_t dacia_r(offs_t offset);
	void via1_b_w(uint8_t data);
	void via1_cb1_w(uint8_t data);
	void cdrom_data_w(uint8_t data);
	void cdrom_ctrl_w(uint8_t data);
	uint8_t cdrom_data_r();
	int m_irq;

	uint8_t m_lcd_addr_l, m_lcd_addr_h;
	uint8_t m_lcd_data_l, m_lcd_data_h;

	uint8_t m_dacia_irq1_reg;
	uint8_t m_dacia_rts1;
	uint8_t m_dacia_dtr1;

	uint8_t m_parity_1;
	uint8_t m_parity_mode_1;
	uint8_t m_bpc_1;
	int m_dacia_ic_div_1;
	uint8_t m_dacia_echo1;
	uint8_t m_dacia_stp_1;
	uint8_t m_dacia_reg1;
	uint8_t m_dacia_fe1;
	uint8_t m_dacia_cmp1;
	uint8_t m_dacia_cmpval1;

	uint8_t m_dacia_irq2_reg;
	uint8_t m_dacia_rts2;
	uint8_t m_dacia_dtr2;

	uint8_t m_parity_2;
	uint8_t m_parity_mode_2;
	uint8_t m_bpc_2;
	int m_dacia_ic_div_2;
	uint8_t m_dacia_echo2;
	uint8_t m_dacia_stp_2;
	uint8_t m_dacia_reg2;
	uint8_t m_dacia_fe2;
	uint8_t m_dacia_cmp2;
	uint8_t m_dacia_cmpval2;

	uint8_t m_dacia_cts;
	uint8_t m_dacia_dcd;
	uint8_t m_dacia_trans;
	uint8_t m_dacia_trans2;

	uint8_t m_dacia_receiver_data;
	uint8_t m_dacia_receiver_full;

	uint8_t m_dacia_receiver_data2;
	uint8_t m_dacia_receiver_full2;

	uint8_t m_cdrom_ctrl;
	uint8_t m_cdrom_data;

	uint8_t m_sn_data;
	uint8_t m_sn_cb1;

	// LDP-1450
	emu_timer *m_ld_timer;
	TIMER_CALLBACK_MEMBER(ld_timer_callback);

	uint8_t m_ld_command_to_send[8];
	uint8_t m_ld_command_current_byte;
	uint8_t m_ldcount=0;
	uint8_t m_lddata;
	uint8_t generate_isr();
	uint8_t generate_isr2();
//  void laserdisc_w(uint8_t data);
	void laserdisc_response_w(uint8_t data);
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

uint32_t cops_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

/*************************************
 *
 * Sony CDU33A-02 CDROM
 *
 *************************************/

void cops_state::cdrom_data_w(uint8_t data)
{
	const char *regs[4] = { "CMD", "PARAM", "WRITE", "CTRL" };
	m_cdrom_data = bitswap<8>(data,0,1,2,3,4,5,6,7);
	uint8_t reg = ((m_cdrom_ctrl & 4) >> 1) | ((m_cdrom_ctrl & 8) >> 3);
	if (LOG_CDROM) logerror("%s:cdrom_data_w(reg = %s, data = %02x)\n", machine().describe_context(), regs[reg & 0x03], m_cdrom_data);
}

void cops_state::cdrom_ctrl_w(uint8_t data)
{
	if (LOG_CDROM) logerror("%s:cdrom_ctrl_w(%02x)\n", machine().describe_context(), data);
	m_cdrom_ctrl = data;
}

uint8_t cops_state::cdrom_data_r()
{
	const char *regs[4] = { "STATUS", "RESULT", "READ", "FIFOST" };
	uint8_t reg = ((m_cdrom_ctrl & 4) >> 1) | ((m_cdrom_ctrl & 8) >> 3);
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
	int m_ld_command_total_bytes =8;

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

WRITE_LINE_MEMBER(cops_state::ld_w)
{
	m_lddata <<= 1;

	if ( state ) m_lddata |= 1;

	if ( ++m_ldcount >= 8 )
	{
		m_ldcount = 0;
		m_lddata  = 0;
		printf("LDBYTE %d", m_lddata);
	}

	printf("LDBIT %d",state);
}
/*void cops_state::laserdisc_w(uint8_t data)
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
            }
            break;
    }
}*/

/*************************************
 *
 * 6552 DACIA
 *
 *************************************/

void cops_state::update_dacia_irq()
{
	uint8_t isr = generate_isr();
	//remove bits
	isr &= ~m_dacia_irq1_reg;
	m_maincpu->set_input_line(INPUT_LINE_NMI, isr? ASSERT_LINE:CLEAR_LINE);

	uint8_t isr2 = generate_isr2();
	//remove bits
	isr2 &= ~m_dacia_irq2_reg;
	m_maincpu->set_input_line(INPUT_LINE_NMI, isr2? ASSERT_LINE:CLEAR_LINE);

}

void cops_state::dacia_receive(uint8_t data)
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

uint8_t cops_state::generate_isr()
{
	uint8_t isr =0;

	isr |= m_dacia_receiver_full;
	isr |= (m_dacia_cmp1 << 1);
	isr |= (m_dacia_trans <<4);

	if (isr)
	{
		isr |= 0x40;
	}
	return isr;
}

uint8_t cops_state::generate_isr2()
{
	uint8_t isr2 =0;

	isr2 |= m_dacia_receiver_full2;
	isr2 |= (m_dacia_cmp2 << 1);
	isr2 |= (m_dacia_trans2 <<4);

	if (isr2)
	{
		isr2 |= 0x40;
	}
	return isr2;
}

uint8_t cops_state::dacia_r(offs_t offset)
{
	switch(offset & 0x07)
	{
		case 0: /* ISR1: Interrupt Status Register */
		{
			uint8_t isr = generate_isr();
			m_dacia_trans =0;
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			return isr;
		}

		case 1: /* CSR1: Control Status Register */
		{
			uint8_t csr =0;
			csr |= m_dacia_rts1;
			csr |= (m_dacia_dtr1 << 1);
			csr |= (m_dacia_cts <<4);
			csr |= (m_dacia_fe1 <<7);
			if (LOG_DACIA) logerror("CSR1 %02x\n",csr);
			return csr;
		}

		case 3: /* RDR1: Receive data register */
		{
			m_dacia_receiver_full = 0;
			m_dacia_fe1=0;

//          if (LOG_DACIA) logerror("RDR1 %02x\n",m_dacia_receiver_data);
			if (LOG_DACIA) logerror("RDR1 %02x\n",m_ld->status_r());
			return m_ld->status_r();
		}
		case 4: /* ISR2: Interrupt Status Register */
		{
			uint8_t isr2 = generate_isr2();
			m_dacia_trans2 =0;
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			return isr2;
		}

		case 5: /* CSR2: Control Status Register */
		{
			uint8_t csr2 =0;
			csr2 |= m_dacia_rts2;
			csr2 |= (m_dacia_dtr2 << 1);
			csr2 |= (m_dacia_cts <<4);
			csr2 |= (m_dacia_fe2 <<7);
			if (LOG_DACIA) logerror("CSR2 %02x\n",csr2);
			return csr2;
		}

		case 7: /* RDR2: Receive data register */
			m_dacia_receiver_full2 = 0;
			m_dacia_fe2=0;

			if (LOG_DACIA) logerror("RDR2 %02x\n",m_ld->status_r());
			return m_ld->status_r();


			default:
			if (LOG_DACIA) logerror("%s:dacia_r(%02x)\n", machine().describe_context(), offset);
			return 0;
	}
}

void cops_state::dacia_w(offs_t offset, uint8_t data)
{
	switch(offset & 0x07)
	{
		case 0: /* IRQ enable Register 1 */
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

		case 1: /* Control / Format Register 1 */
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
				if (LOG_DACIA) logerror("DACIA TIME %02d\n", (XTAL(3'686'400) / m_dacia_ic_div_1).value());

//              m_ld_timer->adjust(attotime::from_hz(XTAL(3'686'400) / m_dacia_ic_div_1), 0, attotime::from_hz(XTAL(3'686'400) / m_dacia_ic_div_1));

				if (LOG_DACIA) logerror("DACIA Ctrl Register: %02x\n", data);

			}
			break;

		case 2: /* Compare / Aux Ctrl Register 1 */
			if (m_dacia_reg1 == CMP_REGISTER)
			{
				m_dacia_cmp1 = 1;
				m_dacia_cmpval1 = data;
				if (LOG_DACIA) logerror("DACIA Compare mode: %02x \n", data);
//              update_dacia_irq();
			}
			else
			{
				if (LOG_DACIA) logerror("DACIA Aux ctrl: %02x \n", data);
			}
			[[fallthrough]]; // FIXME: really?
		case 3: /* Transmit Data Register 1 */
			if (LOG_DACIA) logerror("DACIA Transmit: %02x %c\n", data, (char)data);
			m_ld->command_w(data);
			break;

		case 4: /* IRQ enable Register 2 */
			m_dacia_irq2_reg &= ~0x80;

			if (data & 0x80) //enable bits
			{
				m_dacia_irq2_reg |= (data & 0x7f);
			}
			else // disable bits
			{
				m_dacia_irq2_reg &= ~(data & 0x7f);
			}
			if (LOG_DACIA) logerror("DACIA IRQ 2 Register: %02x\n", m_dacia_irq2_reg);
			update_dacia_irq();
			break;

		case 5: /* Control / Format Register 2 */
			if (data & 0x80) //Format Register
			{
				m_dacia_rts2 = (data & 0x01);
				m_dacia_dtr2 = (data & 0x02 ? 1:0);
				m_parity_2 = (data & 0x04);
				m_parity_mode_2 = ((data & 0x18) >> 3);
				m_bpc_2 = ((data & 0x60) >> 5) +5;
				if (LOG_DACIA) logerror("DACIA Format Register 2: %02x\n", data);
			}
			else // Control register
			{
				m_dacia_ic_div_2 = timer_divide_select[data & 0x15];
				m_dacia_echo2 = (data & 0x10);
				m_dacia_stp_2 = (data & 0x20 ? 2:1);
				if (data & 0x40)
				{
					m_dacia_reg2 = AUX_REGISTER;
				}
				else
				{
					m_dacia_reg2 = CMP_REGISTER;
				}
				if (LOG_DACIA) logerror("DACIA TIME 2 %02d\n", (XTAL(3'686'400) / m_dacia_ic_div_1).value());

				m_ld_timer->adjust(attotime::from_hz(XTAL(3'686'400) / m_dacia_ic_div_2), 0, attotime::from_hz(XTAL(3'686'400) / m_dacia_ic_div_2));

				if (LOG_DACIA) logerror("DACIA Ctrl Register 2: %02x\n", data);

			}
			break;

		case 6: /* Compare / Aux Ctrl Register 2 */
			if (m_dacia_reg2 == CMP_REGISTER)
			{
				m_dacia_cmp2 =1;
				m_dacia_cmpval2=data;
				if (LOG_DACIA) logerror("DACIA Compare mode 2: %02x \n", data);
//              update_dacia_irq();
			}
			else
			{
				if (LOG_DACIA) logerror("DACIA Aux ctrl 2: %02x \n", data);
			}
			[[fallthrough]]; // FIXME: really?
		case 7: /* Transmit Data Register 2 */
			if (LOG_DACIA) logerror("DACIA Transmit 2: %02x %c\n", data, (char)data);

		//  for (int i=0; i <8; i++)
			{
	//          m_ld_command_to_send[i] = data & (1<<i);
			}
//          m_ld->command_w(data);
			break;
	}
}
/*************************************
 *
 * I/O
 *
 *************************************/

uint8_t cops_state::io1_r(offs_t offset)
{
	switch( offset & 0x0f )
	{
		case 0x08:  /* SW0 */
			return m_switches[0]->read();
		case 0x09:  /* SW1 */
			return m_switches[1]->read();
		case 0x0a:  /* SW2 */
			return m_switches[2]->read();
		default:
			logerror("Unknown io1_r, offset = %03x\n", offset);
			return 0;
	}
}

uint8_t cops_state::io1_lm_r(offs_t offset)
{
	switch( offset & 0x0f )
	{
		case 0x07:  /* WDI */
			return 1;
		case 0x08:  /* SW0 */
			return m_switches[0]->read();
		case 0x09:  /* SW1 */
			return m_switches[1]->read();
		case 0x0a:  /* SW2 */
			return m_switches[2]->read();
		default:
			logerror("Unknown io1_r, offset = %03x\n", offset);
			return 0;
	}
}

void cops_state::io1_w(offs_t offset, uint8_t data)
{
	switch (offset & 0x0f)
	{
		case 0x00: /* WOP0 Alpha display*/
			m_lcd_addr_l = data;
			break;
		case 0x01: /* WOP1 Alpha display*/
			m_lcd_addr_h = data;
			{
				// update display
				constexpr uint16_t addrs_table[] = {
						0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0002, 0x0001, 0x0080,
						0x1000, 0x0800, 0x0400, 0x2000, 0x4000, 0x0200, 0x0100, 0x8000 };
				const uint16_t addr = m_lcd_addr_l | (m_lcd_addr_h << 8);
				for (int i = 0; i < 16; i++)
				{
					if (addr == addrs_table[i])
					{
						const uint16_t display_data = m_lcd_data_l | (m_lcd_data_h << 8);
						m_digits[i] = bitswap<16>(display_data, 4, 5, 12, 1, 0, 11, 10, 6, 7, 2, 9, 3, 15, 8, 14, 13);
						break;
					}
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

uint8_t cops_state::io2_r(offs_t offset)
{
	switch( offset & 0x0f )
	{
		case 0x03:
			return m_steer->read();
		default:
			logerror("Unknown io2_r, offset = %02x\n", offset);
			return 0;
	}
}

void cops_state::io2_w(offs_t offset, uint8_t data)
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
 *   PA3   Shake motor?
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

void cops_state::via1_b_w(uint8_t data)
{
	m_sn_data = bitswap<8>(data,0,1,2,3,4,5,6,7);
	if (m_sn_cb1)
	{
		m_sn->write(m_sn_data);
	}
}

void cops_state::via1_cb1_w(uint8_t data)
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

void cops_state::cops_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x9fff).rom().region("program", 0);
	map(0xa000, 0xafff).rw(FUNC(cops_state::io1_r), FUNC(cops_state::io1_w));
	map(0xb000, 0xb00f).m("via6522_1", FUNC(via6522_device::map));  /* VIA 1 */
	map(0xb800, 0xb80f).m("via6522_2", FUNC(via6522_device::map));  /* VIA 2 */
	map(0xc000, 0xcfff).rw(FUNC(cops_state::io2_r), FUNC(cops_state::io2_w));
	map(0xd000, 0xd007).rw(FUNC(cops_state::dacia_r), FUNC(cops_state::dacia_w));
	map(0xd800, 0xd80f).m("via6522_3", FUNC(via6522_device::map));  /* VIA 3 */
	map(0xe000, 0xffff).bankr("sysbank1");
}

void cops_state::revlatns_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x9fff).rom().region("program", 0);
	map(0xa000, 0xafff).rw(FUNC(cops_state::io1_lm_r), FUNC(cops_state::io1_w));
	map(0xb000, 0xb00f).m("via6522_1", FUNC(via6522_device::map));  /* VIA 1 */
	map(0xc000, 0xc00f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0xd000, 0xd007).rw(FUNC(cops_state::dacia_r), FUNC(cops_state::dacia_w));
	map(0xe000, 0xffff).bankr("sysbank1");
}

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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )
INPUT_PORTS_END

void cops_state::machine_start()
{
	m_digits.resolve();

	m_ld_timer = timer_alloc(FUNC(cops_state::ld_timer_callback), this);

	m_ld_timer->adjust(attotime::from_hz(167*5), 0, attotime::from_hz(167*5));

	m_dacia_cmpval1 = m_dacia_cmpval2 = 0;
	m_ld_command_current_byte = 0;
	std::fill(std::begin(m_ld_command_to_send), std::end(m_ld_command_to_send), 0);
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
}


void cops_state::init_cops()
{
	//The hardware is designed and programmed to use multiple system ROM banks, but for some reason it's hardwired to bank 2.
	//For documentation's sake, here's the init
	uint8_t *rom = memregion("system")->base();
	membank("sysbank1")->configure_entries(0, 4, &rom[0x0000], 0x2000);
	membank("sysbank1")->set_entry(2);
}

void cops_state::base(machine_config &config)
{
	M6502(config, m_maincpu, MAIN_CLOCK/2);

	SONY_LDP1450(config, m_ld, 9600);
	m_ld->set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_SELF_RENDER);
	screen.set_raw(XTAL(14'318'181)*2, 910, 0, 704, 525, 44, 524);
	screen.set_screen_update("laserdisc", FUNC(laserdisc_device::screen_update));

	/* via */
	via6522_device &via1(MOS6522(config, "via6522_1", MAIN_CLOCK/2));
	via1.irq_handler().set(FUNC(cops_state::via1_irq));
	via1.writepb_handler().set(FUNC(cops_state::via1_b_w));
	via1.cb1_handler().set(FUNC(cops_state::via1_cb1_w));

	SPEAKER(config, "mono").front_center();

	/* acia (really a 65C52)*/

	/* TODO: Verify clock */
	SN76489(config, m_sn, MAIN_CLOCK/2);
	m_sn->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void cops_state::cops(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &cops_state::cops_map);

	via6522_device &via2(MOS6522(config, "via6522_2", MAIN_CLOCK/2));
	via2.irq_handler().set(FUNC(cops_state::via2_irq));

	via6522_device &via3(MOS6522(config, "via6522_3", MAIN_CLOCK/2));
	via3.readpa_handler().set(FUNC(cops_state::cdrom_data_r));
	via3.writepa_handler().set(FUNC(cops_state::cdrom_data_w));
	via3.writepb_handler().set(FUNC(cops_state::cdrom_ctrl_w));
}

void cops_state::revlatns(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &cops_state::revlatns_map);

	MSM6242(config, "rtc", XTAL(32'768));
}

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
	DISK_IMAGE_READONLY( "copsld", 0, NO_DUMP )
ROM_END

ROM_START( revlatns )
	ROM_REGION( 0x8000, "program", 0 )
	ROM_LOAD( "revelations_prog.bin", 0x0000, 0x8000, CRC(5ab41ac3) SHA1(0f7027551da17011576cf077e2f199729bb10482) )

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "revelations_sys.bin", 0x0000, 0x8000, CRC(43e5e3ec) SHA1(fa44b102b5aa7ad2421c575abdc67f1c29f23bc1) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "revlatns", 0, NO_DUMP )
ROM_END

} // Anonymous namespace


GAMEL( 1994, cops,     0,    cops,     cops,     cops_state, init_cops, ROT0, "Atari Games",                      "Cops (USA)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_cops )
GAMEL( 1994, copsuk,   cops, cops,     cops,     cops_state, init_cops, ROT0, "Nova Productions / Deith Leisure", "Cops (UK)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_cops )
GAMEL( 1994, revlatns, 0,    revlatns, revlatns, cops_state, init_cops, ROT0, "Nova Productions",                 "Revelations", MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_cops )
