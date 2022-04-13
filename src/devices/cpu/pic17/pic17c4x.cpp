// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Microchip PIC17C4X microcontrollers

    Comparative list of features:

    Clock frequency     33 MHz maximum (PIC17C42: 25 MHz maximum)
    EPROM (PIC17C42(A)) 2K (mask ROM on PIC17CR42)
    EPROM (PIC17C43)    4K (mask ROM on PIC17CR43)
    EPROM (PIC17C44)    8K
    RAM (PIC17C42(A))   232 bytes
    RAM (PIC17C43/44)   454 bytes
    Pin count           40 (PDIP, CERDIP) or 44 (PLCC, MQFP, TQFP)
    Port pins           33 (19 shared with external bus)
    Interrupt sources   11
    8-bit timers        2 (cascadable)
    16-bit timers       16 (including TMR0)
    Capture inputs      2
    PWM outputs         2
    USART               1

    The original PIC17C42 lacks a hardware multiplier and has a few
    CPU core bugs that were corrected in PIC17C42A and the rest of
    the family.

**********************************************************************/

#include "emu.h"
#include "pic17c4x.h"

// device type definitions
DEFINE_DEVICE_TYPE(PIC17C43, pic17c43_device, "pic17c43", "Microchip PIC17C43")
DEFINE_DEVICE_TYPE(PIC17C44, pic17c44_device, "pic17c44", "Microchip PIC17C44")

pic17c4x_device::pic17c4x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 rom_size, address_map_constructor data_map)
	: pic17_cpu_device(mconfig, type, tag, owner, clock, rom_size, data_map)
	, m_port_out_cb(*this)
	, m_rain(0x3f)
	, m_lata(0)
	, m_rbpu(false)
	, m_ddrb(0)
	, m_latb(0)
	, m_tmr8bit{0, 0}
	, m_pr8bit{0, 0}
	, m_tmr3(0)
	, m_ca16bit{0, 0}
	, m_tcon1(0)
	, m_tcon2(0)
	, m_rcsta(0)
	, m_txsta(0)
	, m_spbrg(0)
	, m_pir(0)
	, m_pie(0)
{
}

pic17c43_device::pic17c43_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 rom_size)
	: pic17c4x_device(mconfig, type, tag, owner, clock, rom_size, address_map_constructor(FUNC(pic17c43_device::data_map), this))
{
}

pic17c43_device::pic17c43_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic17c43_device(mconfig, PIC17C43, tag, owner, clock, 0x1000)
{
}

pic17c44_device::pic17c44_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic17c43_device(mconfig, PIC17C44, tag, owner, clock, 0x2000)
{
}

void pic17c4x_device::data_map(address_map &map)
{
	core_data_map(map);

	// TODO: add handlers for more of these SFRs
	map(0x010, 0x010).rw(FUNC(pic17c4x_device::porta_r), FUNC(pic17c4x_device::porta_w));
	map(0x011, 0x011).rw(FUNC(pic17c4x_device::ddrb_r), FUNC(pic17c4x_device::ddrb_w));
	map(0x012, 0x012).rw(FUNC(pic17c4x_device::portb_r), FUNC(pic17c4x_device::portb_w));
	map(0x013, 0x013).rw(FUNC(pic17c4x_device::rcsta_r), FUNC(pic17c4x_device::rcsta_w));
	//map(0x014, 0x014).r(FUNC(pic17c4x_device::rcreg_r));
	map(0x015, 0x015).rw(FUNC(pic17c4x_device::txsta_r), FUNC(pic17c4x_device::txsta_w));
	map(0x016, 0x016).w(FUNC(pic17c4x_device::txreg_w));
	map(0x017, 0x017).rw(FUNC(pic17c4x_device::spbrg_r), FUNC(pic17c4x_device::spbrg_w));
	//map(0x110, 0x110).rw(FUNC(pic17c4x_device::ddrc_r), FUNC(pic17c4x_device::ddrc_w));
	//map(0x111, 0x111).rw(FUNC(pic17c4x_device::portc_r), FUNC(pic17c4x_device::portc_w));
	//map(0x112, 0x112).rw(FUNC(pic17c4x_device::ddrd_r), FUNC(pic17c4x_device::ddrd_w));
	//map(0x113, 0x113).rw(FUNC(pic17c4x_device::portd_r), FUNC(pic17c4x_device::portd_w));
	//map(0x114, 0x114).rw(FUNC(pic17c4x_device::ddre_r), FUNC(pic17c4x_device::ddre_w));
	//map(0x115, 0x115).rw(FUNC(pic17c4x_device::porte_r), FUNC(pic17c4x_device::porte_w));
	map(0x116, 0x116).rw(FUNC(pic17c4x_device::pir_r), FUNC(pic17c4x_device::pir_w));
	map(0x117, 0x117).rw(FUNC(pic17c4x_device::pie_r), FUNC(pic17c4x_device::pie_w));
	map(0x210, 0x210).rw(FUNC(pic17c4x_device::tmr1_r), FUNC(pic17c4x_device::tmr1_w));
	map(0x211, 0x211).rw(FUNC(pic17c4x_device::tmr2_r), FUNC(pic17c4x_device::tmr2_w));
	map(0x212, 0x212).rw(FUNC(pic17c4x_device::tmr3l_r), FUNC(pic17c4x_device::tmr3l_w));
	map(0x213, 0x213).rw(FUNC(pic17c4x_device::tmr3h_r), FUNC(pic17c4x_device::tmr3h_w));
	map(0x214, 0x214).rw(FUNC(pic17c4x_device::pr1_r), FUNC(pic17c4x_device::pr1_w));
	map(0x215, 0x215).rw(FUNC(pic17c4x_device::pr2_r), FUNC(pic17c4x_device::pr2_w));
	map(0x216, 0x216).rw(FUNC(pic17c4x_device::pr3l_ca1l_r), FUNC(pic17c4x_device::pr3l_ca1l_w));
	map(0x217, 0x217).rw(FUNC(pic17c4x_device::pr3h_ca1h_r), FUNC(pic17c4x_device::pr3h_ca1h_w));
	//map(0x310, 0x310).rw(FUNC(pic17c4x_device::pw1dcl_r), FUNC(pic17c4x_device::pw1dcl_w));
	//map(0x311, 0x311).rw(FUNC(pic17c4x_device::pw2dcl_r), FUNC(pic17c4x_device::pw2dcl_w));
	//map(0x312, 0x312).rw(FUNC(pic17c4x_device::pw1dch_r), FUNC(pic17c4x_device::pw1dch_w));
	//map(0x313, 0x313).rw(FUNC(pic17c4x_device::pw2dch_r), FUNC(pic17c4x_device::pw2dch_w));
	map(0x314, 0x314).r(FUNC(pic17c4x_device::ca2l_r));
	map(0x315, 0x315).r(FUNC(pic17c4x_device::ca2h_r));
	map(0x316, 0x316).rw(FUNC(pic17c4x_device::tcon1_r), FUNC(pic17c4x_device::tcon1_w));
	map(0x317, 0x317).rw(FUNC(pic17c4x_device::tcon2_r), FUNC(pic17c4x_device::tcon2_w));
}

void pic17c43_device::data_map(address_map &map)
{
	pic17c4x_device::data_map(map);

	// RAM: 454 bytes
	map(0x01a, 0x0ff).ram();
	map(0x120, 0x1ff).ram();
}

u8 pic17c4x_device::porta_r()
{
	// RA2 and RA3 are open-drain I/O
	// TODO: RA4 and RA5 carry USART signals
	u8 porta = (m_rain & 0x33) | (m_rain & m_lata & 0x0c);
	if (m_rbpu)
		porta |= 0x80;
	return porta;
}

void pic17c4x_device::porta_w(u8 data)
{
	// RA0 and RA1 have no output drivers
	// RA4 and RA5 can only be output from USART
	m_lata = (m_lata & 0x30) | (data & 0x0c);
	m_port_out_cb[0](m_lata);

	if (m_rbpu != BIT(data, 7))
	{
		m_rbpu = BIT(data, 7);
		if (m_ddrb != 0)
			m_port_out_cb[1](m_rbpu ? (m_latb | m_ddrb) : (m_latb & m_ddrb));
	}
}

u8 pic17c4x_device::ddrb_r()
{
	return m_ddrb;
}

void pic17c4x_device::ddrb_w(u8 data)
{
	if (m_ddrb != data)
	{
		m_ddrb = data;
		m_port_out_cb[1](m_rbpu ? (m_latb | m_ddrb) : (m_latb & m_ddrb));
	}
}

u8 pic17c4x_device::portb_r()
{
	return m_latb | (m_rbpu ? m_ddrb : 0);
}

void pic17c4x_device::portb_w(u8 data)
{
	m_latb = data;
	m_port_out_cb[1](m_rbpu ? (m_latb | m_ddrb) : (m_latb & m_ddrb));
}

u8 pic17c4x_device::rcsta_r()
{
	return m_rcsta;
}

void pic17c4x_device::rcsta_w(u8 data)
{
	// FERR, OERR and RX9D are read-only; bit 3 is unimplemented
	m_rcsta = (data & 0xf0) | (m_rcsta & 0x07);
}

u8 pic17c4x_device::txsta_r()
{
	return m_txsta;
}

void pic17c4x_device::txsta_w(u8 data)
{
	// TRMT is read-only; bits 2 & 3 are unimplemented
	m_txsta = (data & 0xf1) | (m_txsta & 0x02);
}

void pic17c4x_device::txreg_w(u8 data)
{
	logerror("%s: Writing %02X to TXREG\n", machine().describe_context(), data);

	// Clear TXIF
	m_pir &= 0xfd;
	if ((m_pir & m_pie) == 0)
		set_peif(false);
}

u8 pic17c4x_device::spbrg_r()
{
	return m_spbrg;
}

void pic17c4x_device::spbrg_w(u8 data)
{
	m_spbrg = data;
}

u8 pic17c4x_device::tmr1_r()
{
	return m_tmr8bit[0];
}

void pic17c4x_device::tmr1_w(u8 data)
{
	m_tmr8bit[0] = data;
}

u8 pic17c4x_device::tmr2_r()
{
	return m_tmr8bit[1];
}

void pic17c4x_device::tmr2_w(u8 data)
{
	m_tmr8bit[1] = data;
}

u8 pic17c4x_device::tmr3l_r()
{
	return m_tmr3 & 0x00ff;
}

void pic17c4x_device::tmr3l_w(u8 data)
{
	m_tmr3 = (m_tmr3 & 0xff00) | data;
}

u8 pic17c4x_device::tmr3h_r()
{
	return m_tmr3 >> 8;
}

void pic17c4x_device::tmr3h_w(u8 data)
{
	m_tmr3 = u16(data) << 8 | (m_tmr3 & 0x00ff);
}

u8 pic17c4x_device::pr1_r()
{
	return m_pr8bit[0];
}

void pic17c4x_device::pr1_w(u8 data)
{
	m_pr8bit[0] = data;
}

u8 pic17c4x_device::pr2_r()
{
	return m_pr8bit[1];
}

void pic17c4x_device::pr2_w(u8 data)
{
	m_pr8bit[1] = data;
}

u8 pic17c4x_device::pr3l_ca1l_r()
{
	return m_ca16bit[0] & 0x00ff;
}

void pic17c4x_device::pr3l_ca1l_w(u8 data)
{
	m_ca16bit[0] = (m_ca16bit[0] & 0xff00) | data;
}

u8 pic17c4x_device::pr3h_ca1h_r()
{
	return m_ca16bit[0] >> 8;
}

void pic17c4x_device::pr3h_ca1h_w(u8 data)
{
	m_ca16bit[0] = u16(data) << 8 | (m_ca16bit[0] & 0x00ff);
}

u8 pic17c4x_device::ca2l_r()
{
	return m_ca16bit[1] & 0x00ff;
}

u8 pic17c4x_device::ca2h_r()
{
	return m_ca16bit[1] >> 8;
}

u8 pic17c4x_device::tcon1_r()
{
	return m_tcon1;
}

void pic17c4x_device::tcon1_w(u8 data)
{
	m_tcon1 = data;
}

u8 pic17c4x_device::tcon2_r()
{
	return m_tcon2;
}

void pic17c4x_device::tcon2_w(u8 data)
{
	// CA1OVF and CA2OVF are read-only
	m_tcon2 = (m_tcon2 & 0xc0) | (data & 0x3f);
}

u8 pic17c4x_device::pir_r()
{
	return m_pir;
}

void pic17c4x_device::pir_w(u8 data)
{
	// TXIF and RCIF are read-only
	m_pir = (data & 0xfc) | (m_pir & 0x03);
	set_peif((m_pie & m_pir) != 0);
}

u8 pic17c4x_device::pie_r()
{
	return m_pie;
}

void pic17c4x_device::pie_w(u8 data)
{
	m_pie = data;
	set_peif((m_pie & m_pir) != 0);
}

void pic17c4x_device::increment_timers()
{
	pic17_cpu_device::increment_timers();

	// Advance TMR1
	if (BIT(m_tcon2, 0) && !BIT(m_tcon1, 0))
	{
		if (BIT(m_tcon1, 3))
		{
			// T16 mode: TMR1 and TMR2 cascaded
			if (m_tmr8bit[0] == m_pr8bit[0] && m_tmr8bit[1] == m_pr8bit[1])
			{
				// Roll over and set TMR1IF
				m_tmr8bit[0] = 0;
				m_tmr8bit[1] = 0;
				m_pir |= 0x10;
				if (BIT(m_pie, 4))
					set_peif(true);
			}
			else
			{
				++m_tmr8bit[0];
				if (m_tmr8bit[0] == 0 && BIT(m_tcon2, 1))
					++m_tmr8bit[1];
			}
		}
		else
		{
			if (m_tmr8bit[0] == m_pr8bit[0])
			{
				// Roll over and set TMR1IF
				m_tmr8bit[0] = 0;
				m_pir |= 0x10;
				if (BIT(m_pie, 4))
					set_peif(true);
			}
			else
				++m_tmr8bit[0];
		}
	}

	// Advance TMR2 (as 8-bit counter only)
	if (BIT(m_tcon2, 1) && (m_tcon1 & 0x0a) == 0)
	{
		if (m_tmr8bit[1] == m_pr8bit[1])
		{
			// Roll over and set TMR2IF
			m_tmr8bit[1] = 0;
			m_pir |= 0x20;
			if (BIT(m_pie, 5))
				set_peif(true);
		}
		else
			++m_tmr8bit[1];
	}

	// Advance TMR3
	if (BIT(m_tcon2, 2) && !BIT(m_tcon1, 2))
	{
		// Period register is not used to determine overflow when CA1 mode is selected
		if (m_tmr3 == (BIT(m_tcon2, 3) ? 0xffff : m_ca16bit[0]))
		{
			// Roll over and set TMR3IF
			m_tmr3 = 0;
			m_pir |= 0x40;
			if (BIT(m_pie, 6))
				set_peif(true);
		}
		else
			++m_tmr3;
	}
}

void pic17c4x_device::device_resolve_objects()
{
	// Resolve callback objects
	m_port_out_cb.resolve_all_safe();
}

void pic17c4x_device::device_start()
{
	pic17_cpu_device::device_start();

	// Register debug state
	state_add(PIC17_LATA, "LATA", m_lata).mask(0x3c);
	state_add(PIC17_DDRB, "DDRB", m_ddrb);
	state_add(PIC17_LATB, "LATB", m_latb);
	state_add(PIC17_TMR1, "TMR1", m_tmr8bit[0]);
	state_add(PIC17_TMR2, "TMR2", m_tmr8bit[1]);
	state_add(PIC17_TMR3, "TMR3", m_tmr3);
	state_add(PIC17_PR1, "PR1", m_pr8bit[0]);
	state_add(PIC17_PR2, "PR2", m_pr8bit[1]);
	state_add(PIC17_CA1, "CA1", m_ca16bit[0]);
	state_add(PIC17_CA2, "CA2", m_ca16bit[1]);
	state_add(PIC17_TCON1, "TCON1", m_tcon1);
	state_add(PIC17_TCON2, "TCON2", m_tcon2);
	state_add(PIC17_RCSTA, "RCSTA", m_rcsta).mask(0xf7);
	state_add(PIC17_TXSTA, "TXSTA", m_txsta).mask(0xf3);
	state_add(PIC17_SPBRG, "SPBRG", m_spbrg);
	state_add(PIC17_PIE, "PIE", m_pie);
	state_add(PIC17_PIR, "PIR", m_pir);

	// Save state
	save_item(NAME(m_lata));
	save_item(NAME(m_rbpu));
	save_item(NAME(m_ddrb));
	save_item(NAME(m_latb));
	save_item(NAME(m_tmr8bit));
	save_item(NAME(m_pr8bit));
	save_item(NAME(m_tmr3));
	save_item(NAME(m_ca16bit));
	save_item(NAME(m_tcon1));
	save_item(NAME(m_tcon2));
	save_item(NAME(m_rcsta));
	save_item(NAME(m_txsta));
	save_item(NAME(m_spbrg));
	save_item(NAME(m_pie));
	save_item(NAME(m_pir));
}

void pic17c4x_device::device_reset()
{
	pic17_cpu_device::device_reset();

	// Reset ports
	m_lata = 0x3c;
	m_rbpu = false;
	m_ddrb = 0xff;
	m_port_out_cb[0](0x3c);
	m_port_out_cb[1](0xff);

	// Reset timers
	//m_pw2dcl &= 0xc0;
	m_tcon1 = 0;
	m_tcon2 = 0;

	// Reset USART
	m_rcsta &= 0x01;
	m_txsta = (m_txsta & 0x01) | 0x02;

	// Reset peripheral interrupts
	m_pir = 0x02;
	m_pie = 0;
}

void pic17c4x_device::execute_set_input(int linenum, int state)
{
	switch (linenum)
	{
	case RA0_LINE:
		if (state != CLEAR_LINE && BIT(m_rain, 0))
		{
			// Falling edge
			m_rain &= 0x3e;
			int_edge(false);
		}
		else if (state == CLEAR_LINE && !BIT(m_rain, 0))
		{
			// Rising edge
			m_rain |= 0x01;
			int_edge(true);
		}
		break;

	case RA1_LINE:
		if (state != CLEAR_LINE && BIT(m_rain, 1))
		{
			// Falling edge
			m_rain &= 0x3d;
			t0cki_edge(false);
		}
		else if (state == CLEAR_LINE && !BIT(m_rain, 1))
		{
			// Rising edge
			m_rain |= 0x02;
			t0cki_edge(true);
		}
		break;

	case RA2_LINE:
	case RA3_LINE:
	case RA4_LINE:
	case RA5_LINE:
		if (state == ASSERT_LINE)
			m_rain &= ~(1 << (linenum - RA0_LINE));
		else
			m_rain |= 1 << (linenum - RA0_LINE);
		break;
	}
}
