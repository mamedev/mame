// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud
/*****************************************************************************
 *
 * Portable MCS-51 Family Emulator
 * Copyright Steve Ellenoff
 *
 * Chips in the family:
 * 8051 Product Line (8031,8051,8751)
 * 8052 Product Line (8032,8052,8752)
 * 8054 Product Line (8054)
 * 8058 Product Line (8058)
 * 80552 Product Line (80552, 83552, 87552)
 * 80562 Product Line (80562, 83562, 87562)
 *
 * This work is based on:
 * #1) 'Intel(tm) MC51 Microcontroller Family Users Manual' and
 * #2) 8051 simulator by Travis Marlatte
 * #3) Portable UPI-41/8041/8741/8042/8742 emulator V0.1 by Juergen Buchmueller (MAME CORE)
 *
 *****************************************************************************/

/******************************************************************************
 *
 * Notes:
 *
 * The term cycles is used here to really refer to clock oscillations, because 1 machine cycle
 * actually takes 12 oscillations.
 *
 * Read/Write/Modify Instruction -
 *   Data is read from the Port Latch (not the Port Pin!), possibly modified, and
 *   written back to (the pin? and) the latch!
 *
 *   The following all perform this on a port address..
 *   (anl, orl, xrl, jbc, cpl, inc, dec, djnz, mov px.y,c, clr px.y, setb px.y)
 *
 *****************************************************************************/

/* TODO: Various
 * - EA pin - defined by architecture, must implement:
 *   1 means external access, bypassing internal ROM
 * - T0 output clock ?
 *
 * - Implement 80C52 extended serial capabilities
 * - Implement 83C751 in sslam.c
 * - Fix cardline.c
 *      most likely due to different behaviour of I/O pins. The boards
 *      actually use 80CXX, i.e. CMOS versions.
 *      "Normal" 805X will return a 0 if reading from a output port which has
 *      a 0 written to it's latch. At least cardline expects a 1 here.
 * - ADC support for 80552/80562 (controls analog inputs for Arctic Thunder)
 *
 */

#include "emu.h"
#include "i8051.h"
#include "mcs51dasm.h"

#include <tuple>

#define LOG_RX (1U << 1)
#define LOG_TX (1U << 2)

#define VERBOSE (0)

#include "logmacro.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* m_pc vectors */

enum
{
	V_RESET = 0x000,    /* power on address */
	V_IE0   = 0x003,    /* External Interrupt 0 */
	V_TF0   = 0x00b,    /* Timer 0 Overflow */
	V_IE1   = 0x013,    /* External Interrupt 1 */
	V_TF1   = 0x01b,    /* Timer 1 Overflow */
	V_RITI  = 0x023,    /* Serial Receive/Transmit */

	/* 8052 Only Vectors */
	V_TF2   = 0x02b,    /* Timer 2 Overflow */

	/* DS5002FP */
	V_PFI   = 0x02b     /* Power Failure Interrupt */
};

enum serial_state : u8
{
	SIO_IDLE,
	SIO_START_LE,
	SIO_START,
	SIO_DATA0,
	SIO_DATA1,
	SIO_DATA2,
	SIO_DATA3,
	SIO_DATA4,
	SIO_DATA5,
	SIO_DATA6,
	SIO_DATA7,
	SIO_DATA8,
	SIO_STOP,
};

DEFINE_DEVICE_TYPE(I8031, i8031_device, "i8031", "Intel 8031")
DEFINE_DEVICE_TYPE(I8051, i8051_device, "i8051", "Intel 8051")
DEFINE_DEVICE_TYPE(I8751, i8751_device, "i8751", "Intel 8751")
DEFINE_DEVICE_TYPE(AM8753, am8753_device, "am8753", "AMD Am8753")
DEFINE_DEVICE_TYPE(I8344, i8344_device, "i8344", "Intel 8344AH RUPI-44")
DEFINE_DEVICE_TYPE(I8744, i8744_device, "i8744", "Intel 8744H RUPI-44")


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void mcs51_cpu_device::program_map(address_map &map)
{
	if (m_rom_size > 0)
		map(0, m_rom_size - 1).rom().region(DEVICE_SELF, 0);
}

void mcs51_cpu_device::sfr_map(address_map &map)
{
	map(0x80, 0x80).rw(FUNC(mcs51_cpu_device::p0_r  ), FUNC(mcs51_cpu_device::p0_w  ));
	map(0x81, 0x81).rw(FUNC(mcs51_cpu_device::sp_r  ), FUNC(mcs51_cpu_device::sp_w  ));
	map(0x82, 0x83).rw(FUNC(mcs51_cpu_device::dptr_r), FUNC(mcs51_cpu_device::dptr_w));
	map(0x87, 0x87).rw(FUNC(mcs51_cpu_device::pcon_r), FUNC(mcs51_cpu_device::pcon_w));
	map(0x88, 0x88).rw(FUNC(mcs51_cpu_device::tcon_r), FUNC(mcs51_cpu_device::tcon_w));
	map(0x89, 0x89).rw(FUNC(mcs51_cpu_device::tmod_r), FUNC(mcs51_cpu_device::tmod_w));
	map(0x8a, 0x8a).rw(FUNC(mcs51_cpu_device::tl0_r ), FUNC(mcs51_cpu_device::tl0_w ));
	map(0x8b, 0x8b).rw(FUNC(mcs51_cpu_device::tl1_r ), FUNC(mcs51_cpu_device::tl1_w ));
	map(0x8c, 0x8c).rw(FUNC(mcs51_cpu_device::th0_r ), FUNC(mcs51_cpu_device::th0_w ));
	map(0x8d, 0x8d).rw(FUNC(mcs51_cpu_device::th1_r ), FUNC(mcs51_cpu_device::th1_w ));
	map(0x90, 0x90).rw(FUNC(mcs51_cpu_device::p1_r  ), FUNC(mcs51_cpu_device::p1_w  ));
	map(0x98, 0x98).rw(FUNC(mcs51_cpu_device::scon_r), FUNC(mcs51_cpu_device::scon_w));
	map(0x99, 0x99).rw(FUNC(mcs51_cpu_device::sbuf_r), FUNC(mcs51_cpu_device::sbuf_w));
	map(0xa0, 0xa0).rw(FUNC(mcs51_cpu_device::p2_r  ), FUNC(mcs51_cpu_device::p2_w  ));
	map(0xa8, 0xa8).rw(FUNC(mcs51_cpu_device::ie_r  ), FUNC(mcs51_cpu_device::ie_w  ));
	map(0xb0, 0xb0).rw(FUNC(mcs51_cpu_device::p3_r  ), FUNC(mcs51_cpu_device::p3_w  ));
	map(0xb8, 0xb8).rw(FUNC(mcs51_cpu_device::ip_r  ), FUNC(mcs51_cpu_device::ip_w  ));
	map(0xd0, 0xd0).rw(FUNC(mcs51_cpu_device::psw_r ), FUNC(mcs51_cpu_device::psw_w ));
	map(0xe0, 0xe0).rw(FUNC(mcs51_cpu_device::acc_r ), FUNC(mcs51_cpu_device::acc_w ));
	map(0xf0, 0xf0).rw(FUNC(mcs51_cpu_device::b_r   ), FUNC(mcs51_cpu_device::b_w   ));
}

void mcs51_cpu_device::intd_map(address_map &map)
{
	map(0x00, m_ram_mask).ram().share(m_internal_ram);
	map(0x80, 0xff).unmaprw();
	sfr_map(map);
}

void mcs51_cpu_device::inti_map(address_map &map)
{
	map(0x00, m_ram_mask).ram().share(m_internal_ram);
}

u8   mcs51_cpu_device::psw_r ()
{
	return m_psw;
}

void mcs51_cpu_device::psw_w (u8 data)
{
	m_psw = (m_psw & 0x01) | (data & 0xfe);
}

u8   mcs51_cpu_device::acc_r ()
{
	return m_acc;
}

u8   mcs51_cpu_device::b_r   ()
{
	return m_b;
}

void mcs51_cpu_device::b_w   (u8 data)
{
	m_b = data;
}

u8   mcs51_cpu_device::sp_r  ()
{
	return m_sp;
}

void mcs51_cpu_device::sp_w  (u8 data)
{
	m_sp = data;
}

u8   mcs51_cpu_device::dptr_r (offs_t offset)
{
	return m_dptr >> (offset*8);
}

void mcs51_cpu_device::dptr_w (offs_t offset, u8 data)
{
	m_dptr = (m_dptr & ~(0xff << (offset*8))) | (data << (offset*8));
}

u8   mcs51_cpu_device::pcon_r()
{
	return m_pcon;
}

void mcs51_cpu_device::pcon_w(u8 data)
{
	m_pcon = data;
}

u8   mcs51_cpu_device::tcon_r()
{
	return m_tcon;
}

void mcs51_cpu_device::tcon_w(u8 data)
{
	m_tcon = data;
}

u8   mcs51_cpu_device::tmod_r()
{
	return m_tmod;
}

void mcs51_cpu_device::tmod_w(u8 data)
{
	m_tmod = data;
}

u8   mcs51_cpu_device::scon_r()
{
	return m_scon;
}

void mcs51_cpu_device::scon_w(u8 data)
{
	u8 old = m_scon;
	m_scon = data;
	if (!BIT(old, 4) && BIT(m_scon, 4))
	{
		LOGMASKED(LOG_RX, "rx enabled SCON 0x%02x\n", m_scon);
		if (!BIT(m_scon, 6, 2))
			logerror("mode 0 serial input is not emulated\n");
		m_uart.rxbit = SIO_IDLE;
	}
}

u8   mcs51_cpu_device::sbuf_r()
{
	return m_sbuf;
}

void mcs51_cpu_device::sbuf_w(u8 data)
{
	m_sbuf = data;
	LOGMASKED(LOG_TX, "tx byte 0x%02x\n", m_sbuf);
	m_uart.data_out = m_sbuf;
	m_uart.txbit = SIO_START;
}

u8   mcs51_cpu_device::ie_r  ()
{
	return m_ie;
}

void mcs51_cpu_device::ie_w  (u8 data)
{
	m_ie = data;
}

u8   mcs51_cpu_device::ip_r  ()
{
	return m_ip;
}

void mcs51_cpu_device::ip_w  (u8 data)
{
	m_ip = data;
	update_irq_prio();
}

u8   mcs51_cpu_device::p0_r  ()
{
	return m_rwm ? m_p0 : (m_p0 | m_forced_inputs[0]) & m_port_in_cb[0]();
}

void mcs51_cpu_device::p0_w  (u8 data)
{
	m_p0 = data;
	m_port_out_cb[0](m_p0);
}

u8   mcs51_cpu_device::p1_r  ()
{
	return m_rwm ? m_p1 : (m_p1 | m_forced_inputs[1]) & m_port_in_cb[1]();
}

void mcs51_cpu_device::p1_w  (u8 data)
{
	m_p1 = data;
	m_port_out_cb[1](m_p1);
}

u8   mcs51_cpu_device::p2_r  ()
{
	return m_rwm ? m_p2 : (m_p2 | m_forced_inputs[2]) & m_port_in_cb[2]();
}

void mcs51_cpu_device::p2_w  (u8 data)
{
	m_p2 = data;
	m_port_out_cb[2](m_p2);
}

u8   mcs51_cpu_device::p3_r  ()
{
	return m_rwm ? m_p3 :
		(m_p3 | m_forced_inputs[3]) & m_port_in_cb[3]()
		& ~(BIT(m_last_line_state, MCS51_INT0_LINE) ? 4 : 0)
		& ~(BIT(m_last_line_state, MCS51_INT1_LINE) ? 8 : 0);
}

void mcs51_cpu_device::p3_w  (u8 data)
{
	m_p3 = data;
	// P3.1 = SFR(P3) & TxD
	if (!m_uart.txd)
		m_port_out_cb[3](m_p3 & ~0x02);
	else
		m_port_out_cb[3](m_p3);
}

u8   mcs51_cpu_device::tl0_r ()
{
	return m_tl0;
}

void mcs51_cpu_device::tl0_w (u8 data)
{
	m_tl0 = data;
}

u8   mcs51_cpu_device::tl1_r ()
{
	return m_tl1;
}

void mcs51_cpu_device::tl1_w (u8 data)
{
	m_tl1 = data;
}

u8   mcs51_cpu_device::th0_r ()
{
	return m_th0;
}

void mcs51_cpu_device::th0_w (u8 data)
{
	m_th0 = data;
}

u8   mcs51_cpu_device::th1_r ()
{
	return m_th1;
}

void mcs51_cpu_device::th1_w (u8 data)
{
	m_th1 = data;
}


mcs51_cpu_device::mcs51_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, int io_width)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(mcs51_cpu_device::program_map), this))
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_intd_config("internal_direct", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(mcs51_cpu_device::intd_map), this))
	, m_inti_config("internal_indirect", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(mcs51_cpu_device::inti_map), this))
	, m_pc(0)
	, m_has_pd(false)
	, m_inst_cycles(0)
	, m_rom_size(program_width > 0 ? 1 << program_width : 0)
	, m_ram_mask((io_width == 8) ? 0xff : 0x7f)
	, m_num_interrupts(5)
	, m_internal_ram(*this, "internal_ram")
	, m_port_in_cb(*this, 0xff)
	, m_port_out_cb(*this)
	, m_rtemp(0)
{
	/* default to standard cmos interfacing */
	for (auto & elem : m_forced_inputs)
		elem = 0;

	for(int i=0; i != 8; i++) {
		m_port_in_cb[i].bind().set([this, i]() { return port_default_r(i); });
		m_port_out_cb[i].bind().set([this, i](u8 data) { port_default_w(i, data); });
	}
}


u8 mcs51_cpu_device::port_default_r(int port)
{
	if(!machine().side_effects_disabled())
		logerror("read of un-hooked port %d (PC=%X)\n", port, m_ppc);
	return 0xff;
}

void mcs51_cpu_device::port_default_w(int port, u8 data)
{
	logerror("write of un-hooked port %d %02x\n", port, data);
}

i8031_device::i8031_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mcs51_cpu_device(mconfig, I8031, tag, owner, clock, 0, 7)
{
}

i8051_device::i8051_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mcs51_cpu_device(mconfig, I8051, tag, owner, clock, 12, 7)
{
}

i8751_device::i8751_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mcs51_cpu_device(mconfig, I8751, tag, owner, clock, 12, 7)
{
}

am8753_device::am8753_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mcs51_cpu_device(mconfig, AM8753, tag, owner, clock, 13, 7)
{
}


i8344_device::i8344_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mcs51_cpu_device(mconfig, I8344, tag, owner, clock, 0, 8)
{
}

i8744_device::i8744_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mcs51_cpu_device(mconfig, I8744, tag, owner, clock, 12, 8)
{
}


device_memory_interface::space_config_vector mcs51_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config),
		std::make_pair(AS_INTD,    &m_intd_config),
		std::make_pair(AS_INTI,    &m_inti_config)
	};
}

void mcs51_cpu_device::clear_current_irq()
{
	if (m_cur_irq_prio >= 0)
		m_irq_active &= ~(1 << m_cur_irq_prio);
	if (m_irq_active & 4)
		m_cur_irq_prio = 2;
	else if (m_irq_active & 2)
		m_cur_irq_prio = 1;
	else if (m_irq_active & 1)
		m_cur_irq_prio = 0;
	else
		m_cur_irq_prio = -1;
	LOG("New: %d %02x\n", m_cur_irq_prio, m_irq_active);
}

/* Generate an external ram address for read/writing using indirect addressing mode */

/*The lowest 8 bits of the address are passed in (from the R0/R1 register), however
  the hardware can be configured to set the rest of the address lines to any available output port pins, which
  means the only way we can implement this is to allow the driver to setup a callback to generate the
  address as defined by the specific hardware setup. We'll assume the address won't be bigger than 32 bits

  Couriersud, October 2008:
  There is no way external hardware can distinguish between 8bit access and 16 bit access.
  During 16bit access the high order byte of the address is output on port 2. We therefore
  assume that most hardware will use port 2 for 8bit access as well.

  On configurations where 8 bit access in conjunction with other ports is used,
  it is up to the driver to use mirror() to mask out the high level address and
  provide it's own mapping.
*/

offs_t mcs51_cpu_device::external_ram_iaddr(offs_t offset, offs_t mem_mask)
{
	if (mem_mask == 0x00ff)
		return (offset & 0x00ff) | (m_p2 << 8);
	return offset;
}

void mcs51_cpu_device::transmit(int state)
{
	if (m_uart.txd != state)
	{
		m_uart.txd = state;

		// P3.1 = SFR(P3) & TxD
		if (BIT(m_p3, 1))
		{
			if (state)
				m_port_out_cb[3](m_p3);
			else
				m_port_out_cb[3](m_p3 & ~0x02);
		}
	}
}

void mcs51_cpu_device::handle_8bit_uart_clock(int source)
{
	if (source == 1)
	{
		m_uart.tx_clk += !m_uart.smod_div;
		m_uart.rx_clk += !m_uart.smod_div;
	}
}

void mcs51_cpu_device::transmit_receive(int source)
{
	int mode = (BIT(m_scon, SCON_SM0) << 1) | BIT(m_scon, SCON_SM1);

	if (source == 1) /* timer1 */
		m_uart.smod_div = (m_uart.smod_div + 1) & !BIT(m_pcon, PCON_SMOD);

	switch (mode)
	{
		// 8 bit shifter - rate set by clock freq / 12
		case 0:
			if (source == 0)
			{
				// TODO: mode 0 serial input is unemulated
				// FIXME: output timing is highly simplified and incorrect
				switch (m_uart.txbit)
				{
				case SIO_IDLE:
					break;
				case SIO_START:
					m_p3 |= 0x03;
					m_port_out_cb[3](m_p3);
					m_uart.txbit = SIO_DATA0;
					break;
				case SIO_DATA0: case SIO_DATA1: case SIO_DATA2: case SIO_DATA3:
				case SIO_DATA4: case SIO_DATA5: case SIO_DATA6: case SIO_DATA7:
					m_p3 &= ~0x03;
					if (BIT(m_uart.data_out, m_uart.txbit - SIO_DATA0))
						m_p3 |= 1U << 0;
					m_port_out_cb[3](m_p3);

					if (m_uart.txbit == SIO_DATA7)
					{
						set_ti(1);
						m_uart.txbit = SIO_STOP;
					}
					else
						m_uart.txbit++;
					break;
				case SIO_STOP:
					m_p3 |= 0x03;
					m_port_out_cb[3](m_p3);
					m_uart.txbit = SIO_IDLE;
					break;
				}
			}
			return;
		//8 bit uart ( + start,stop bit ) - baud set by timer1 or timer2
		case 1:
		case 3:
			handle_8bit_uart_clock(source);
			break;
		//9 bit uart
		case 2:
			m_uart.rx_clk += (source == 0) ? (BIT(m_pcon, PCON_SMOD) ? 6 : 3) : 0; /* clock / 12 * 3 / 8 (16) = clock / 32 (64)*/
			m_uart.tx_clk += (source == 0) ? (BIT(m_pcon, PCON_SMOD) ? 6 : 3) : 0; /* clock / 12 */
			break;
	}

	// transmit
	if (m_uart.tx_clk >= 16)
	{
		m_uart.tx_clk &= 0x0f;

		switch (m_uart.txbit)
		{
		case SIO_IDLE:
			transmit(1);
			break;
		case SIO_START:
			LOGMASKED(LOG_TX, "tx start bit (%s)\n", machine().time().to_string());
			transmit(0);
			m_uart.txbit = SIO_DATA0;
			break;
		case SIO_DATA0: case SIO_DATA1: case SIO_DATA2: case SIO_DATA3:
		case SIO_DATA4: case SIO_DATA5: case SIO_DATA6: case SIO_DATA7:
			LOGMASKED(LOG_TX, "tx bit %d data %d (%s)\n", m_uart.txbit - SIO_DATA0, BIT(m_uart.data_out, m_uart.txbit - SIO_DATA0), machine().time().to_string());
			transmit(BIT(m_uart.data_out, m_uart.txbit - SIO_DATA0));

			// mode 1 has no data8/parity bit
			if (mode == 1 && m_uart.txbit == SIO_DATA7)
				m_uart.txbit = SIO_STOP;
			else
				m_uart.txbit++;
			break;
		case SIO_DATA8: // data8/parity bit
			LOGMASKED(LOG_TX, "tx bit %d data %d (%s)\n", m_uart.txbit - SIO_DATA0, BIT(m_scon, SCON_TB8), machine().time().to_string());
			transmit(BIT(m_scon, SCON_TB8));
			m_uart.txbit = SIO_STOP;
			break;
		case SIO_STOP:
			LOGMASKED(LOG_TX, "tx stop bit (%s)\n", machine().time().to_string());
			transmit(1);
			set_ti(1);
			m_uart.txbit = SIO_IDLE;
			break;
		}
	}

	// receive
	if (m_uart.rx_clk >= 16 || m_uart.rxbit < SIO_START)
	{
		m_uart.rx_clk &= 0x0f;

		if (BIT(m_scon, SCON_REN))
		{
			// directly read RXD input
			int const data = BIT(m_port_in_cb[3](), 0);

			switch (m_uart.rxbit)
			{
			case SIO_IDLE:
				if (data)
					m_uart.rxbit = SIO_START_LE;
				break;
			case SIO_START_LE: // start bit leading edge
				if (!data)
				{
					LOGMASKED(LOG_RX, "rx start leading edge (%s)\n", machine().time().to_string());
					m_uart.rxbit = SIO_START;
					m_uart.rx_clk = 8;
				}
				break;
			case SIO_START:
				if (!data)
				{
					LOGMASKED(LOG_RX, "rx start bit (%s)\n", machine().time().to_string());

					m_uart.data_in = 0;
					m_uart.rxbit = SIO_DATA0;
				}
				else
					// false start bit
					m_uart.rxbit = SIO_START_LE;
				break;
			case SIO_DATA0: case SIO_DATA1: case SIO_DATA2: case SIO_DATA3:
			case SIO_DATA4: case SIO_DATA5: case SIO_DATA6: case SIO_DATA7:
				LOGMASKED(LOG_RX, "rx bit %d data %d (%s)\n", m_uart.rxbit - SIO_DATA0, data, machine().time().to_string());
				if (data)
					m_uart.data_in |= 1U << (m_uart.rxbit - SIO_DATA0);

				// mode 1 has no data8/parity bit
				if (mode == 1 && m_uart.rxbit == SIO_DATA7)
					m_uart.rxbit = SIO_STOP;
				else
					m_uart.rxbit++;
				break;
			case SIO_DATA8: // data8/parity bit
				LOGMASKED(LOG_RX, "rx bit %d data %d (%s)\n", m_uart.rxbit - SIO_DATA0, data, machine().time().to_string());
				m_uart.rxb8 = data;
				m_uart.rxbit = SIO_STOP;
				break;
			case SIO_STOP:
				if (!BIT(m_scon, SCON_RI))
				{
					switch (mode)
					{
					case 1:
						LOGMASKED(LOG_RX, "rx byte 0x%02x stop %d (%s)\n", m_uart.data_in, data, machine().time().to_string());
						m_sbuf = m_uart.data_in;
						if (!BIT(m_scon, SCON_SM2))
						{
							// RB8 contains stop bit
							set_rb8(data);
							set_ri(1);
						}
						else if (data)
							// RI if valid stop bit
							set_ri(1);
						break;
					case 2:
					case 3:
						LOGMASKED(LOG_RX, "rx byte 0x%02x RB8 %d stop %d (%s)\n", m_uart.data_in, m_uart.rxb8, data, machine().time().to_string());
						m_sbuf = m_uart.data_in;
						set_rb8(m_uart.rxb8);

						// no RI if SM2 && !RB8
						if (!BIT(m_scon, SCON_SM2) || BIT(m_scon, SCON_RB8))
							set_ri(1);
						break;
					}
				}
				else
					LOGMASKED(LOG_RX, "rx overrun discarding data 0x%02x\n", m_uart.data_in);

				// next state depends on stop bit validity
				if (data)
					m_uart.rxbit = SIO_START_LE;
				else
					m_uart.rxbit = SIO_IDLE;
				break;
			}
		}
	}
}


void mcs51_cpu_device::update_timer_t0(int cycles)
{
	int mode = (BIT(m_tmod, TMOD_M0_1) << 1) | BIT(m_tmod, TMOD_M0_0);
	u32 count;

	if (BIT(m_tcon, TCON_TR0))
	{
		u32 delta;

		/* counter / external input */
		delta = BIT(m_tmod, TMOD_CT0) ? m_t0_cnt : cycles;
		/* taken, reset */
		m_t0_cnt = 0;
		/* TODO: Not sure about IE0. The manual specifies INT0=high,
		 * which in turn means CLEAR_LINE.
		 * IE0 may be edge triggered depending on IT0 */
		if (BIT(m_tmod, TMOD_GATE0) && !BIT(m_tcon, TCON_IE0))
			delta = 0;

		switch (mode)
		{
			case 0: /* 13 Bit Timer Mode */
				count = ((m_th0 << 5) | (m_tl0 & 0x1f));
				count += delta;
				if (count & 0xffffe000) /* Check for overflow */
					set_tf0(1);
				m_th0 = (count >> 5) & 0xff;
				m_tl0 = count & 0x1f;
				break;
			case 1: /* 16 Bit Timer Mode */
				count = ((m_th0 << 8) | m_tl0);
				count += delta;
				if (count & 0xffff0000) /* Check for overflow */
					set_tf0(1);
				m_th0 = (count >> 8) & 0xff;
				m_tl0 = count & 0xff;
				break;
			case 2: /* 8 Bit Autoreload */
				count = ((u32)m_tl0) + delta;
				if (count & 0xffffff00) /* Check for overflow */
				{
					set_tf0(1);
					count += m_th0; /* Reload timer */
				}
				/* Update new values of the counter */
				m_tl0 =  count & 0xff;
				break;
			case 3:
				/* Split Timer 1 */
				count = ((u32)m_tl0) + delta;
				if (count & 0xffffff00) /* Check for overflow */
					set_tf0(1);
				m_tl0 = count & 0xff; /* Update new values of the counter */
				break;
		}
	}
	if (BIT(m_tcon, TCON_TR1))
	{
		switch (mode)
		{
		case 3:
			/* Split Timer 2 */
			count = ((u32)m_th0) + cycles; /* No gate control or counting !*/
			if (count & 0xffffff00) /* Check for overflow */
				set_tf1(1);
			m_th0 = count & 0xff; /* Update new values of the counter */
			break;
		}
	}
}

/* From the DS5002FP User Manual
When Timer 1 is selected for operation in Mode 3, it stops counting and holds its current value. This
action is the same as setting TR1 = 0. When Timer 0 is selected in Mode 3, Timer 1???s control bits are
stolen as described above. As a result, Timer 1???s functions are limited in this MODE. It is forced to
operate as a timer whose clock in-put is 12 tCLK and it cannot generate an interrupt on overflow. In
addition, it also cannot be used with the GATE function. However, it can be started and stopped by
switching it into or out of Mode 3 or it can be assigned as a baud rate generator for the serial port.
*/

/* Intel documentation:
 *  Timer 1 may still be used in modes 0, 1, and 2, while timer 0
 * is in mode 3. With one important exception:  No interrupts
 * will be generated by timer 1 while timer 0 is using the TF1
 * overflow flag
 */

void mcs51_cpu_device::update_timer_t1(int cycles)
{
	u8 mode = (BIT(m_tmod, TMOD_M1_1) << 1) | BIT(m_tmod, TMOD_M1_0);
	u8 mode_0 = (BIT(m_tmod, TMOD_M0_1) << 1) | BIT(m_tmod, TMOD_M0_0);
	u32 count;

	if (mode_0 != 3)
	{
		if (BIT(m_tcon, TCON_TR1))
		{
			u32 delta;
			u32 overflow = 0;

			/* counter / external input */
			delta = BIT(m_tmod, TMOD_CT1) ? m_t1_cnt : cycles;
			/* taken, reset */
			m_t1_cnt = 0;
			/* TODO: Not sure about IE0. The manual specifies INT0=high,
			 * which in turn means CLEAR_LINE. Change to access last_state?
			 * IE0 may be edge triggered depending on IT0 */
			if (BIT(m_tmod, TMOD_GATE1) && !BIT(m_tcon, TCON_IE1))
				delta = 0;

			switch (mode)
			{
				case 0: /* 13 Bit Timer Mode */
					count = ((m_th1 << 5) | (m_tl1 & 0x1f));
					count += delta;
					overflow = count & 0xffffe000; /* Check for overflow */
					m_th1 = (count >> 5) & 0xff;
					m_tl1 = count & 0x1f;
					break;
				case 1: /* 16 Bit Timer Mode */
					count = ((m_th1 << 8) | m_tl1);
					count += delta;
					overflow = count & 0xffff0000; /* Check for overflow */
					m_th1 = (count >> 8) & 0xff;
					m_tl1 = count & 0xff;
					break;
				case 2: /* 8 Bit Autoreload */
					count = ((u32)m_tl1) + delta;
					overflow = count & 0xffffff00; /* Check for overflow */
					if (overflow)
						count += m_th1; /* Reload timer */
					/* Update new values of the counter */
					m_tl1 =  count & 0xff;
					break;
				case 3:
					/* do nothing */
					break;
			}
			if (overflow)
			{
				set_tf1(1);
				transmit_receive(1);
			}
		}
	}
	else
	{
		u32 delta;
		u32 overflow = 0;

		delta =  cycles;
		/* taken, reset */
		m_t1_cnt = 0;
		switch (mode)
		{
			case 0: /* 13 Bit Timer Mode */
				count = ((m_th1 << 5) | (m_tl1 & 0x1f));
				count += delta;
				overflow = count & 0xffffe000; /* Check for overflow */
				m_th1 = (count >> 5) & 0xff;
				m_tl1 = count & 0x1f;
				break;
			case 1: /* 16 Bit Timer Mode */
				count = ((m_th1 << 8) | m_tl1);
				count += delta;
				overflow = count & 0xffff0000; /* Check for overflow */
				m_th1 = (count >> 8) & 0xff;
				m_tl1 = count & 0xff;
				break;
			case 2: /* 8 Bit Autoreload */
				count = ((u32)m_tl1) + delta;
				overflow = count & 0xffffff00; /* Check for overflow */
				if (overflow)
					count += m_th1; /* Reload timer */
				/* Update new values of the counter */
				m_tl1 = count & 0xff;
				break;
			case 3:
				/* do nothing */
				break;
		}
		if (overflow)
		{
			transmit_receive(1);
		}
	}
}

void mcs51_cpu_device::update_timer_t2(int cycles)
{
}

/* Check and update status of serial port */
void mcs51_cpu_device::update_irq_prio()
{
	for (int i = 0; i < 8; i++)
		m_irq_prio[i] = ((m_ip >> i) & 1) | (((m_iph >> i) & 1) << 1);
}

/***********************************************************************************
 Check for pending Interrupts and process - returns # of cycles used for the int

 Note about priority & interrupting interrupts..
 1) A high priority interrupt cannot be interrupted by anything!
 2) A low priority interrupt can ONLY be interrupted by a high priority interrupt
 3) If more than 1 Interrupt Flag is set (ie, 2 simultaneous requests occur),
    the following logic works as follows:
    1) If two requests come in of different priority levels, the higher one is selected..
    2) If the requests are of the same level, an internal order is used:
        a) IEO
        b) TFO
        c) IE1
        d) TF1
        e) RI+TI
        f) TF2+EXF2
 **********************************************************************************/

void mcs51_cpu_device::irqs_complete_and_mask(u8 &ints, u8 int_mask)
{
	ints &= int_mask;
}

bool mcs51_cpu_device::manage_idle_on_interrupt(u8 ints)
{
	return false;
}

void mcs51_cpu_device::check_irqs()
{
	u8 ints = (BIT(m_tcon, TCON_IE0) | (BIT(m_tcon, TCON_TF0) << 1) | (BIT(m_tcon, TCON_IE1) << 2) | (BIT(m_tcon, TCON_TF1) << 3) | ((BIT(m_scon, SCON_RI) | BIT(m_scon, SCON_TI)) << 4));
	u8 int_vec = 0;
	int priority_request = -1;

	//If All Interrupts Disabled or no pending abort..
	u8 int_mask = BIT(m_ie, IE_A) ? m_ie : 0x00;

	irqs_complete_and_mask(ints, int_mask);

	if (!ints)
		return;

	if(manage_idle_on_interrupt(ints))
		return;

	for (int i = 0; i < m_num_interrupts; i++)
	{
		if (ints & (1 << i))
		{
			if (m_irq_prio[i] > priority_request)
			{
				priority_request = m_irq_prio[i];
				int_vec = (i << 3) | 3;
			}
		}
	}

	/* Skip the interrupt request if currently processing interrupt
	 * and the new request does not have a higher priority
	 */
	LOG("Request: %d\n", priority_request);
	if (m_irq_active && (priority_request <= m_cur_irq_prio))
	{
		LOG("higher or equal priority irq (%u) in progress already, skipping ...\n", m_cur_irq_prio);
		return;
	}

	// indicate we took the external IRQ
	if (int_vec == V_IE0)
	{
		// Hack to work around polling latency issue with JB INT0
		if (m_last_op == 0x20 && m_last_bit == 0xb2)
			m_pc = m_ppc + 3;
		standard_irq_callback(0, m_pc);
	}
	else if (int_vec == V_IE1)
	{
		// Hack to work around polling latency issue with JB INT1
		if (m_last_op == 0x20 && m_last_bit == 0xb3)
			m_pc = m_ppc + 3;
		standard_irq_callback(1, m_pc);
	}

	//Save current pc to stack, set pc to new interrupt vector
	push_pc();
	m_pc = int_vec;

	/* interrupts take 24 cycles */
	m_inst_cycles += 2;

	//Set current Irq & Priority being serviced
	m_cur_irq_prio = priority_request;
	m_irq_active |= (1 << priority_request);

	LOG("Take: %d %02x\n", m_cur_irq_prio, m_irq_active);

	//Clear any interrupt flags that should be cleared since we're servicing the irq!
	switch (int_vec)
	{
		case V_IE0:
			//External Int Flag only cleared when configured as Edge Triggered..
			if (BIT(m_tcon, TCON_IT0)) /* for some reason having this, breaks alving dmd games */
				set_ie0(0);
			break;
		case V_TF0:
			//Timer 0 - Always clear Flag
			set_tf0(0);
			break;
		case V_IE1:
			//External Int Flag only cleared when configured as Edge Triggered..
			if (BIT(m_tcon, TCON_IT1)) /* for some reason having this, breaks alving dmd games */
				set_ie1(0);
			break;
		case V_TF1:
			//Timer 1 - Always clear Flag
			set_tf1(0);
			break;
		case V_RITI:
			/* no flags are cleared, TI and RI remain set until reset by software */
			break;
		/* I8052 specific */
		case V_TF2:
			/* no flags are cleared according to manual */
			break;
		/* DS5002FP specific */
		/* case V_PFI:
		 *  no flags are cleared, PFW is reset by software
		 *  This has the same vector as V_TF2.
		 */
	}
}

void mcs51_cpu_device::burn_cycles(int cycles)
{
	while (cycles--)
	{
		m_icount--;

		// update timers
		update_timer_t0(1);
		update_timer_t1(1);
		update_timer_t2(1);

		// check and update status of serial port
		transmit_receive(0);
	}
}

void mcs51_cpu_device::handle_irq(int irqline, int state, u32 new_state, u32 tr_state)
{
	switch (irqline)
	{
		//External Interrupt 0
		case MCS51_INT0_LINE:
			//Line Asserted?
			if (state != CLEAR_LINE)
			{
				//Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT0 active lo!
				if (BIT(m_tcon, TCON_IT0))
				{
					if (BIT(tr_state, MCS51_INT0_LINE))
						set_ie0(1);
				}
				else
				{
					set_ie0(1); //Nope, just set it..
				}
			}
			else
			{
				if (!BIT(m_tcon, TCON_IT0)) /* clear if level triggered */
					set_ie0(0);
			}

			break;

		//External Interrupt 1
		case MCS51_INT1_LINE:
			//Line Asserted?
			if (state != CLEAR_LINE)
			{
				//Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT1 active lo!
				if (BIT(m_tcon, TCON_IT1))
				{
					if (BIT(tr_state, MCS51_INT1_LINE))
						set_ie1(1);
				}
				else
					set_ie1(1); //Nope, just set it..
			}
			else
			{
				if (!BIT(m_tcon, TCON_IT1)) /* clear if level triggered */
					set_ie1(0);
			}
			break;

		case MCS51_T0_LINE:
			if (BIT(tr_state, MCS51_T0_LINE) && BIT(m_tcon, TCON_TR0))
				m_t0_cnt++;
			break;

		case MCS51_T1_LINE:
			if (BIT(tr_state, MCS51_T1_LINE) && BIT(m_tcon, TCON_TR1))
				m_t1_cnt++;
			break;
	}
}

void mcs51_cpu_device::execute_set_input(int irqline, int state)
{
	/* From the manual:
	 *
	 * <cite>In operation all the interrupt flags are latched into the
	 * interrupt control system during State 5 of every machine cycle.
	 * The samples are polled during the following machine cycle.</cite>
	 *
	 * ==> Since we do not emulate sub-states, this assumes that the signal is present
	 * for at least one cycle (12 states)
	 *
	 */
	u32 new_state = (m_last_line_state & ~(1 << irqline)) | ((state != CLEAR_LINE) << irqline);
	/* detect 0->1 transitions */
	u32 tr_state = (~m_last_line_state) & new_state;

	handle_irq(irqline, state, new_state, tr_state);
	m_last_line_state = new_state;
}

void mcs51_cpu_device::handle_ta_window()
{
}

/* Execute cycles */
void mcs51_cpu_device::execute_run()
{
	do
	{
		// check interrupts
		check_irqs();

		// if in powerdown and external IRQ did not wake us up, just return
		if (m_has_pd && BIT(m_pcon, PCON_PD))
		{
			debugger_wait_hook();
			m_icount = 0;
			return;
		}

		// if not idling, process next opcode
		if (!(m_has_pd && BIT(m_pcon, PCON_IDL) && !BIT(m_pcon, PCON_PD)))
		{
			m_ppc = m_pc;
			debugger_instruction_hook(m_pc);
			u8 op = m_program.read_byte(m_pc++);

			m_inst_cycles += mcs51_cycles[op];
			execute_op(op);
		}
		else
			m_inst_cycles++;

		// burn the cycles
		burn_cycles(m_inst_cycles);

		handle_ta_window();

		m_inst_cycles = 0;

	} while (m_icount > 0);
}


/****************************************************************************
 * MCS51/8051 Section
 ****************************************************************************/

void mcs51_cpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_program);
	space(AS_DATA).specific(m_data);
	space(AS_INTD).specific(m_intd);
	space(AS_INTI).specific(m_inti);

	/* Save states */
	save_item(NAME(m_ppc));
	save_item(NAME(m_pc));
	save_item(NAME(m_rwm));
	save_item(NAME(m_last_line_state));
	save_item(NAME(m_t0_cnt));
	save_item(NAME(m_t1_cnt));
	save_item(NAME(m_t2_cnt));
	save_item(NAME(m_t2ex_cnt));
	save_item(NAME(m_cur_irq_prio));
	save_item(NAME(m_irq_active));
	save_item(NAME(m_irq_prio));
	save_item(NAME(m_last_op));
	save_item(NAME(m_last_bit));

	save_item(NAME(m_dptr));
	save_item(NAME(m_acc));
	save_item(NAME(m_psw));
	save_item(NAME(m_b));
	save_item(NAME(m_sp));
	save_item(NAME(m_pcon));
	save_item(NAME(m_tcon));
	save_item(NAME(m_tmod));
	save_item(NAME(m_scon));
	save_item(NAME(m_sbuf));
	save_item(NAME(m_ie));
	save_item(NAME(m_ip));
	save_item(NAME(m_iph));

	save_item(NAME(m_uart.data_out));
	save_item(NAME(m_uart.data_in));
	save_item(NAME(m_uart.txbit));
	save_item(NAME(m_uart.txd));
	save_item(NAME(m_uart.rxbit));
	save_item(NAME(m_uart.rxb8));
	save_item(NAME(m_uart.smod_div));
	save_item(NAME(m_uart.rx_clk));
	save_item(NAME(m_uart.tx_clk));

	state_add( MCS51_PC,   "PC",   m_pc  ).formatstr("%04X");
	state_add( MCS51_SP,   "SP",   m_sp  ).formatstr("%02X");
	state_add( MCS51_PSW,  "PSW",  m_psw ).formatstr("%02X");
	state_add( MCS51_ACC,  "A",    m_acc ).formatstr("%02X");
	state_add( MCS51_B,    "B",    m_b   ).formatstr("%02X");
	state_add( MCS51_DPTR, "DPTR", m_dptr).formatstr("%04X");
	state_add( MCS51_IE,   "IE",   m_ie  ).formatstr("%02X");
	state_add( MCS51_IP,   "IP",   m_ip  ).formatstr("%02X");
	state_add<u8>( MCS51_P0,  "P0", [this](){ auto dis = machine().disable_side_effects(); return p0_r(); }, [this](u8 p){ p0_w(p); }).formatstr("%02X");
	state_add<u8>( MCS51_P1,  "P1", [this](){ auto dis = machine().disable_side_effects(); return p1_r(); }, [this](u8 p){ p1_w(p); }).formatstr("%02X");
	state_add<u8>( MCS51_P2,  "P2", [this](){ auto dis = machine().disable_side_effects(); return p2_r(); }, [this](u8 p){ p2_w(p); }).formatstr("%02X");
	state_add<u8>( MCS51_P3,  "P3", [this](){ auto dis = machine().disable_side_effects(); return p3_r(); }, [this](u8 p){ p3_w(p); }).formatstr("%02X");
	state_add<u8>( MCS51_R0,  "R0", [this](){ return r_reg(0); }, [this](u8 r){ set_reg(0, r); }).formatstr("%02X");
	state_add<u8>( MCS51_R1,  "R1", [this](){ return r_reg(1); }, [this](u8 r){ set_reg(1, r); }).formatstr("%02X");
	state_add<u8>( MCS51_R2,  "R2", [this](){ return r_reg(2); }, [this](u8 r){ set_reg(2, r); }).formatstr("%02X");
	state_add<u8>( MCS51_R3,  "R3", [this](){ return r_reg(3); }, [this](u8 r){ set_reg(3, r); }).formatstr("%02X");
	state_add<u8>( MCS51_R4,  "R4", [this](){ return r_reg(4); }, [this](u8 r){ set_reg(4, r); }).formatstr("%02X");
	state_add<u8>( MCS51_R5,  "R5", [this](){ return r_reg(5); }, [this](u8 r){ set_reg(5, r); }).formatstr("%02X");
	state_add<u8>( MCS51_R6,  "R6", [this](){ return r_reg(6); }, [this](u8 r){ set_reg(6, r); }).formatstr("%02X");
	state_add<u8>( MCS51_R7,  "R7", [this](){ return r_reg(7); }, [this](u8 r){ set_reg(7, r); }).formatstr("%02X");
	state_add<u8>( MCS51_RB,  "RB", [this](){ return (m_psw & 0x18)>>3; }, [this](u8 rb){ set_rs(rb); }).mask(0x03).formatstr("%02X");
	state_add( MCS51_TCON, "TCON", m_tcon).formatstr("%02X");
	state_add( MCS51_TMOD, "TMOD", m_tmod).formatstr("%02X");
	state_add( MCS51_TL0,  "TL0",  m_tl0).formatstr("%02X");
	state_add( MCS51_TH0,  "TH0",  m_th0).formatstr("%02X");
	state_add( MCS51_TL1,  "TL1",  m_tl1).formatstr("%02X");
	state_add( MCS51_TH1,  "TH1",  m_th1).formatstr("%02X");

	state_add( STATE_GENPC, "GENPC", m_pc).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_rtemp).formatstr("%8s").noshow();

	m_sbuf = 0;

	set_icountptr(m_icount);
}

void mcs51_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				m_psw & 0x80 ? 'C':'.',
				m_psw & 0x40 ? 'A':'.',
				m_psw & 0x20 ? 'F':'.',
				m_psw & 0x10 ? '0':'.',
				m_psw & 0x08 ? '1':'.',
				m_psw & 0x04 ? 'V':'.',
				m_psw & 0x02 ? '?':'.',
				m_psw & 0x01 ? 'P':'.');
			break;
	}
}

/* Reset registers to the initial values */
void mcs51_cpu_device::device_reset()
{
	m_last_line_state = 0;
	m_t0_cnt = 0;
	m_t1_cnt = 0;
	m_t2_cnt = 0;
	m_t2ex_cnt = 0;

	/* Flag as NO IRQ in Progress */
	m_irq_active = 0;
	m_cur_irq_prio = -1;
	m_last_op = 0;
	m_last_bit = 0;

	/* these are all defined reset states */
	m_rwm = 0;
	m_ppc = m_pc;
	m_pc = 0;
	m_sp = 7;
	m_psw = 0;
	m_acc = 0;
	m_dptr = 0;
	m_b = 0;
	m_ip = 0;
	m_iph = 0;
	m_ie = 0;
	m_scon = 0;
	m_tcon = 0;
	m_tmod = 0;
	m_pcon = 0;
	m_th1 = 0;
	m_th0 = 0;
	m_tl1 = 0;
	m_tl0 = 0;

	/* set the port configurations to all 1's */
	p3_w(0xff);
	p2_w(0xff);
	p1_w(0xff);
	p0_w(0xff);

	m_uart.data_out = 0;
	m_uart.data_in = 0;
	m_uart.rx_clk = 0;
	m_uart.tx_clk = 0;
	m_uart.txbit = SIO_IDLE;
	m_uart.txd = 1;
	m_uart.rxbit = SIO_IDLE;
	m_uart.rxb8 = 0;
	m_uart.smod_div = 0;

	update_irq_prio();
}


std::unique_ptr<util::disasm_interface> mcs51_cpu_device::create_disassembler()
{
	return std::make_unique<i8051_disassembler>();
}

std::unique_ptr<util::disasm_interface> i8344_device::create_disassembler()
{
	return std::make_unique<rupi44_disassembler>();
}

std::unique_ptr<util::disasm_interface> i8744_device::create_disassembler()
{
	return std::make_unique<rupi44_disassembler>();
}
