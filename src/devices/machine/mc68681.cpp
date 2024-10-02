// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, R. Belmont, Joseph Zatarski
/*
    2681 DUART
    68681 DUART
    28C94 QUART
    68340 serial module
    XR68C681 DUART (backwards compatible with 68681 with some improvements)

    Written by Mariusz Wojcieszek
    Updated by Jonathan Gevaryahu AKA Lord Nightmare
    Improved interrupt handling by R. Belmont
    Rewrite and modernization in progress by R. Belmont
    Addition of the duart compatible 68340 serial module support by Edstrom
    Support for the Exar XR68C681 by Joseph Zatarski (July of 2018)

    The main incompatibility between the 2681 and 68681 (Signetics and Motorola each
    manufactured both versions of the chip) is that the 68681 has a R/W input and
    generates a 68000-compatible DTACK signal, instead of using generic RD and WR
    strobes as the 2681 does. The 68681 also adds a programmable interrupt vector,
    with an IACK input replacing IP6.

    The command register addresses should never be read from. Doing so may place
    the baud rate generator into a test mode which drives the parallel outputs with
    internal counters and causes serial ports to operate at uncontrollable rates.

    Exar XR68C681

    The XR68C681 is an improvement upon the MC68681 which supports more baud
    rates, adds an additional MISR (masked ISR) register, and adds a low-power
    standby mode. There may be other differences, but these are the most
    notable.

    The extra baud rates are implemented by an 'X' bit for each channel. The X
    bit chooses between two baud rate tables, in addition to the ACR[7] bit.
    The X bit is changed by additional commands that are written to CRA and
    CRB.

    The MISR is a read only register that takes the place of the 'BRG Test'
    register on the MC68681.

    The low power standby mode is entered and left by a command written to CRA
    or CRB registers. Writing the commands to either register affects the whole
    DUART, not just one channel. Resetting the DUART also leaves low power
    mode.
*/

#include "emu.h"
#include "mc68681.h"

#include <algorithm>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"


static const char *const duart68681_reg_read_names[0x10] =
{
	"MRA", "SRA", "BRG Test", "RHRA", "IPCR", "ISR", "CTU", "CTL", "MRB", "SRB", "1X/16X Test", "RHRB", "IVR", "Input Ports", "Start Counter", "Stop Counter"
};

static const char *const duart68681_reg_write_names[0x10] =
{
	"MRA", "CSRA", "CRA", "THRA", "ACR", "IMR", "CTUR", "CTLR", "MRB", "CSRB", "CRB", "THRB", "IVR", "OPCR", "Set OP Bits", "Reset OP Bits"
};

static const int baud_rate_ACR_0[] =     { 50, 110, 134, 200, 300,  600,   1200,  1050,  2400,   4800, 7200, 9600, 38400, 0, 0, 0 }; /* xr68c681 X=0 */
static const int baud_rate_ACR_0_X_1[] = { 75, 110, 134, 150, 3600, 14400, 28800, 57600, 115200, 4800, 1800, 9600, 19200, 0, 0, 0 };
static const int baud_rate_ACR_1[] =     { 75, 110, 134, 150, 300,  600,   1200,  2000,  2400,   4800, 1800, 9600, 19200, 0, 0, 0 }; /* xr68c681 X=0 */
static const int baud_rate_ACR_1_X_1[] = { 50, 110, 134, 200, 3600, 14400, 28800, 57600, 115200, 4800, 7200, 9600, 38400, 0, 0, 0 };

static const int baud_rate_ACR_0_340[] =     { 50, 110, 134, 200, 300,  600,   1200,  1050,  2400,   4800, 7200, 9600, 38400, 76800, 0, 0 }; /* xr68c681 ACR:7=0 */
static const int baud_rate_ACR_1_340[] =     { 75, 110, 134, 150, 300,  600,   1200,  2000,  2400,   4800, 1800, 9600, 19200, 38400, 0, 0 }; /* xr68c681 ACR:7=1 */

#define INT_INPUT_PORT_CHANGE       0x80
#define INT_DELTA_BREAK_B           0x40
#define INT_RXRDY_FFULLB            0x20
#define INT_TXRDYB                  0x10
#define INT_COUNTER_READY           0x08
#define INT_DELTA_BREAK_A           0x04
#define INT_RXRDY_FFULLA            0x02
#define INT_TXRDYA                  0x01

#define STATUS_RECEIVED_BREAK       0x80
#define STATUS_FRAMING_ERROR        0x40
#define STATUS_PARITY_ERROR         0x20
#define STATUS_OVERRUN_ERROR        0x10
#define STATUS_TRANSMITTER_EMPTY    0x08
#define STATUS_TRANSMITTER_READY    0x04
#define STATUS_FIFO_FULL            0x02
#define STATUS_RECEIVER_READY       0x01

#define MODE_RX_INT_SELECT_BIT      0x40
#define MODE_BLOCK_ERROR            0x20

#define CHANA_TAG   "cha"
#define CHANB_TAG   "chb"
#define CHANC_TAG   "chc"
#define CHAND_TAG   "chd"

// device type definition
DEFINE_DEVICE_TYPE(SCN2681, scn2681_device, "scn2681", "SCN2681 DUART")
DEFINE_DEVICE_TYPE(MC68681, mc68681_device, "mc68681", "MC68681 DUART")
DEFINE_DEVICE_TYPE(SC28C94, sc28c94_device, "sc28c94", "SC28C94 QUART")
DEFINE_DEVICE_TYPE(MC68340_DUART, mc68340_duart_device, "mc68340duart", "MC68340 DUART Device")
DEFINE_DEVICE_TYPE(XR68C681, xr68c681_device, "xr68c681", "XR68C681 DUART")
DEFINE_DEVICE_TYPE(DUART_CHANNEL, duart_channel, "duart_channel", "DUART channel")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

duart_base_device::duart_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_chanA(*this, CHANA_TAG),
	m_chanB(*this, CHANB_TAG),
	m_chanC(*this, CHANC_TAG),
	m_chanD(*this, CHAND_TAG),
	write_irq(*this),
	write_a_tx(*this),
	write_b_tx(*this),
	write_c_tx(*this),
	write_d_tx(*this),
	read_inport(*this, 0),
	write_outport(*this),
	ip3clk(0),
	ip4clk(0),
	ip5clk(0),
	ip6clk(0),
	ACR(0),
	IP_last_state(0)
{
}

scn2681_device::scn2681_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: duart_base_device(mconfig, SCN2681, tag, owner, clock)
{
}

mc68681_device::mc68681_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: duart_base_device(mconfig, type, tag, owner, clock),
	m_read_vector(false)
{
}

mc68681_device::mc68681_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68681_device(mconfig, MC68681, tag, owner, clock)
{
}

sc28c94_device::sc28c94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: duart_base_device(mconfig, SC28C94, tag, owner, clock)
{
}

//--------------------------------------------------------------------------------------------------------------------
// The read and write methods are meant to catch all differences in the register model between 68681 and 68340
// serial module. Eg some registers are (re)moved into the 68340 like the vector register. There are also no counter
// in the 68340. The CSR clock register is also different for the external clock modes. The implementation assumes
// that the code knows all of this and will not warn if those registers are accessed as it could be ported code.
// TODO: A lot of subtle differences and also detect misuse of unavailable registers as they should be ignored
//--------------------------------------------------------------------------------------------------------------------
mc68340_duart_device::mc68340_duart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: duart_base_device(mconfig, type, tag, owner, clock)
{
}

mc68340_duart_device::mc68340_duart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68340_duart_device(mconfig, MC68340_DUART, tag, owner, clock)
{
}

xr68c681_device::xr68c681_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc68681_device(mconfig, XR68C681, tag, owner, clock),
	m_XTXA(false),
	m_XRXA(false),
	m_XTXB(false),
	m_XRXB(false)
{
}

//-------------------------------------------------
//  set_clocks - configuration helper to set
//  the external clocks
//-------------------------------------------------

void duart_base_device::set_clocks(int clk3, int clk4, int clk5, int clk6)
{
	ip3clk = clk3;
	ip4clk = clk4;
	ip5clk = clk5;
	ip6clk = clk6;
}

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

void duart_base_device::device_start()
{
	duart_timer = timer_alloc(FUNC(duart_base_device::duart_timer_callback), this);

	save_item(NAME(ACR));
	save_item(NAME(IMR));
	save_item(NAME(ISR));
	save_item(NAME(OPCR));
	save_item(NAME(OPR));
	save_item(NAME(CTR));
	save_item(NAME(IPCR));
	save_item(NAME(IP_last_state));
	save_item(NAME(half_period));
	save_item(NAME(m_irq_state));
}

void mc68681_device::device_start()
{
	duart_base_device::device_start();

	save_item(NAME(m_read_vector));
	save_item(NAME(IVR));
}

void xr68c681_device::device_start()
{
	mc68681_device::device_start();

	save_item(NAME(m_XTXA));
	save_item(NAME(m_XRXA));
	save_item(NAME(m_XTXB));
	save_item(NAME(m_XRXB));
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

void duart_base_device::device_reset()
{
	ACR = 0;  /* Interrupt Vector Register */
	IMR = 0;  /* Interrupt Mask Register */
	ISR = 0;  /* Interrupt Status Register */
	OPCR = 0; /* Output Port Conf. Register */
	OPR = 0;  /* Output Port Register */
	CTR.d = 0;  /* Counter/Timer Preset Value */
	// "reset clears internal registers (SRA, SRB, IMR, ISR, OPR, OPCR) puts OP0-7 in the high state, stops the counter/timer, and puts channels a/b in the inactive state"
	IPCR = 0;

	m_irq_state = false;
	write_irq(CLEAR_LINE);
	write_outport(OPR ^ 0xff);
}

void mc68681_device::device_reset()
{
	duart_base_device::device_reset();

	IVR = 0x0f;  /* Interrupt Vector Register */
	m_read_vector = false;
}

void xr68c681_device::device_reset()
{
	mc68681_device::device_reset();

	m_XTXA = m_XRXA = m_XTXB = m_XRXB = false;
}

void duart_base_device::device_add_mconfig(machine_config &config)
{
	DUART_CHANNEL(config, CHANA_TAG, 0);
	DUART_CHANNEL(config, CHANB_TAG, 0);
}

void sc28c94_device::device_add_mconfig(machine_config &config)
{
	DUART_CHANNEL(config, CHANA_TAG, 0);
	DUART_CHANNEL(config, CHANB_TAG, 0);
	DUART_CHANNEL(config, CHANC_TAG, 0);
	DUART_CHANNEL(config, CHAND_TAG, 0);
}

void mc68340_duart_device::device_add_mconfig(machine_config &config)
{
	DUART_CHANNEL(config, CHANA_TAG, 0);
	DUART_CHANNEL(config, CHANB_TAG, 0);
}

void duart_base_device::update_interrupts()
{
	/* update SR state and update interrupt ISR state for the following bits:
	SRn: bits 7-4: handled elsewhere.
	SRn: bit 3 (TxEMTn) (we can assume since we're not actually emulating the delay/timing of sending bits, that as long as TxRDYn is set, TxEMTn is also set since the transmit byte has 'already happened', therefore TxEMTn is always 1 assuming tx is enabled on channel n and the MSR2n mode is 0 or 2; in mode 1 it is explicitly zeroed, and mode 3 is undefined)
	SRn: bit 2 (TxRDYn) (we COULD assume since we're not emulating delay and timing output, that as long as tx is enabled on channel n, TxRDY is 1 for channel n and the MSR2n mode is 0 or 2; in mode 1 it is explicitly zeroed, and mode 3 is undefined; however, tx_ready is already nicely handled for us elsewhere, so we can use that instead for now, though we may need to retool that code as well)
	SRn: bit 1 (FFULLn) (this bit we actually emulate; if the receive fifo for channel n is full, this bit is 1, otherwise it is 0. the receive fifo should be three words long.)
	SRn: bit 0 (RxRDYn) (this bit we also emulate; the bit is always asserted if the receive fifo is not empty)
	ISR: bit 7: Input Port change; this should be handled elsewhere, on the input port handler
	ISR: bit 6: Delta Break B; this should be handled elsewhere, on the data receive handler
	ISR: bit 5: RxRDYB/FFULLB: this is handled here; depending on whether MSR1B bit 6 is 0 or 1, this bit holds the state of SRB bit 0 or bit 1 respectively
	ISR: bit 4: TxRDYB: this is handled here; it mirrors SRB bit 2
	ISR: bit 3: Counter ready; this should be handled by the timer generator
	ISR: bit 2: Delta Break A; this should be handled elsewhere, on the data receive handler
	ISR: bit 1: RxRDYA/FFULLA: this is handled here; depending on whether MSR1A bit 6 is 0 or 1, this bit holds the state of SRA bit 0 or bit 1 respectively
	ISR: bit 0: TxRDYA: this is handled here; it mirrors SRA bit 2
	*/
	if ((ISR & IMR) != 0)
	{
		if (!m_irq_state)
		{
			m_irq_state = true;
			LOG("Interrupt line active (IMR & ISR = %02X)\n", (ISR & IMR));
			write_irq(ASSERT_LINE);
		}
	}
	else
	{
		if (m_irq_state)
		{
			m_irq_state = false;
			LOG("Interrupt line not active (IMR & ISR = %02X)\n", ISR & IMR);
			write_irq(CLEAR_LINE);
		}
	}
	if (OPCR & 0xf0)
	{
		if (BIT(OPCR, 4))
		{
			if (BIT(ISR, 1))
				OPR |= 0x10;
			else
				OPR &= ~0x10;
		}
		if (BIT(OPCR, 5))
		{
			if (BIT(ISR, 5))
				OPR |= 0x20;
			else
				OPR &= ~0x20;
		}
		if (BIT(OPCR, 6))
		{
			if (BIT(ISR, 0))
				OPR |= 0x40;
			else
				OPR &= ~0x40;
		}
		if (BIT(OPCR, 7))
		{
			if (BIT(ISR, 4))
				OPR |= 0x80;
			else
				OPR &= ~0x80;
		}
		write_outport(OPR ^ 0xff);
	}
}

void mc68681_device::update_interrupts()
{
	duart_base_device::update_interrupts();

	if (!irq_pending())
		m_read_vector = false;  // clear IACK too
}

uint8_t mc68681_device::get_irq_vector()
{
	if (!machine().side_effects_disabled())
		m_read_vector = true;

	return IVR;
}

uint32_t duart_base_device::get_ct_rate()
{
	uint32_t rate = 0;

	if (ACR & 0x40)
	{
		// Timer mode
		switch ((ACR >> 4) & 3)
		{
		case 0: // IP2
		case 1: // IP2 / 16
			//logerror( "68681 (%s): Unhandled timer/counter mode %d\n", duart68681->tag(), (duart68681->ACR >> 4) & 3);
			rate = clock();
			break;
		case 2: // X1/CLK
			rate = clock();
			break;
		case 3: // X1/CLK / 16
			rate = clock() / 16;
			break;
		}
	}
	else
	{
		// Counter mode
		switch ((ACR >> 4) & 3)
		{
		case 0: // IP2
			//logerror( "68681 (%s): Unhandled timer/counter mode %d\n", device->tag(), (duart68681->ACR >> 4) & 3);
			rate = clock();
			break;
		case 1: // TxCA
			rate = m_chanA->get_tx_rate();
			break;
		case 2: // TxCB
			rate = m_chanB->get_tx_rate();
			break;
		case 3: // X1/CLK / 16
			rate = clock() / 16;
			break;
		}
	}

	return rate;
}

uint16_t duart_base_device::get_ct_count()
{
	uint32_t clock = get_ct_rate();
	return duart_timer->remaining().as_ticks(clock);
}

void duart_base_device::start_ct(int count)
{
	uint32_t clock = get_ct_rate();
	duart_timer->adjust(attotime::from_ticks(count, clock), 0);
}

TIMER_CALLBACK_MEMBER(duart_base_device::duart_timer_callback)
{
	if (ACR & 0x40)
	{
		// Timer mode
		half_period ^= 1;

		// timer output to bit 3?
		if ((OPCR & 0xc) == 0x4)
		{
			OPR ^= 0x8;
			write_outport(OPR ^ 0xff);
		}

		// timer driving any serial channels?
		uint8_t csr = m_chanA->get_chan_CSR();

		if ((csr & 0x0f) == 0x0d)   // tx is timer driven
		{
			m_chanA->tx_16x_clock_w(half_period);
		}
		if ((csr & 0xf0) == 0xd0)   // rx is timer driven
		{
			m_chanA->rx_16x_clock_w(half_period);
		}

		csr = m_chanB->get_chan_CSR();
		if ((csr & 0x0f) == 0x0d)   // tx is timer driven
		{
			m_chanB->tx_16x_clock_w(half_period);
		}
		if ((csr & 0xf0) == 0xd0)   // rx is timer driven
		{
			m_chanB->rx_16x_clock_w(half_period);
		}

		if (!half_period)
			set_ISR_bits(INT_COUNTER_READY);

		int count = std::max(CTR.w.l, uint16_t(1));
		start_ct(count);
	}
	else
	{
		// assert OP3 counter ready
		if ((OPCR & 0xc) == 0x4)
			OPR |= 0x8;

		// Counter mode
		set_ISR_bits(INT_COUNTER_READY);
		start_ct(0xffff);
	}

}

uint8_t mc68681_device::read(offs_t offset)
{
	if (offset == 0x0c)
		return IVR;

	uint8_t r = duart_base_device::read(offset);

	if (offset == 0x0d)
	{
		// bit 6 is /IACK (note the active-low)
		if (m_read_vector)
			r &= ~0x40;
		else
			r |= 0x40;
	}

	return r;
}

uint8_t mc68340_duart_device::read(offs_t offset)
{
	uint8_t r = 0;

	switch (offset)
	{
	case 0x00: /* MR1A - does not share register address with MR2A */
		r = m_chanA->read_MR1();
		break;
	case 0x08: /* MR1B - does not share register address with MR2B */
		r = m_chanB->read_MR1();
		break;
	case 0x10: /* MR2A - does not share register address with MR1A */
		r = m_chanA->read_MR2();
		break;
	case 0x11: /* MR2B - does not share register address with MR1B */
		r = m_chanB->read_MR2();
		break;
	default:
		r = duart_base_device::read(offset);
	}
	return r;
}

uint8_t sc28c94_device::read(offs_t offset)
{
	uint8_t r = 0;
	offset &= 0x3f;

	if (offset < 0x10)
	{
		return duart_base_device::read(offset);
	}

	switch (offset)
	{
	case 0x10: /* MR1A/MR2C */
	case 0x11: /* SRC */
	case 0x13: /* Rx Holding Register C */
		r = m_chanC->read_chan_reg(offset & 3);
		break;

	case 0x18: /* MR1D/MR2D */
	case 0x19: /* SRD */
	case 0x1b: /* RHRD */
		r = m_chanD->read_chan_reg(offset & 3);
		break;
	}

	return r;
}

uint8_t xr68c681_device::read(offs_t offset)
{
	if (offset == 0x02)
	{
		LOG("Reading XR68C681 (%s) reg 0x02 (MISR)\n", tag());
		return ISR & IMR;
	}
	else
		return mc68681_device::read(offset);
}

uint8_t duart_base_device::read(offs_t offset)
{
	uint8_t r = 0xff;

	offset &= 0xf;

	LOG("Reading 68681 (%s) reg %x (%s)\n", tag(), offset, duart68681_reg_read_names[offset]);

	switch (offset)
	{
	case 0x00: /* MR1A/MR2A */
	case 0x01: /* SRA */
	case 0x03: /* Rx Holding Register A */
		r = m_chanA->read_chan_reg(offset & 3);
		break;

	case 0x04: /* IPCR */
		r = IPCR;

		// reading this clears all the input change bits
		IPCR &= 0x0f;
		clear_ISR_bits(INT_INPUT_PORT_CHANGE);
		break;

	case 0x05: /* ISR */
		r = ISR;
		break;

	case 0x06: /* CUR */
		r = get_ct_count() >> 8;
		break;

	case 0x07: /* CLR */
		r = get_ct_count() & 0xff;
		break;

	case 0x08: /* MR1B/MR2B */
	case 0x09: /* SRB */
	case 0x0b: /* RHRB */
		r = m_chanB->read_chan_reg(offset & 3);
		break;

	case 0x0a: /* 1X/16X Test */
		r = 0x61;   // the old 68681 returned this and it makes Apollo happy
		break;

	case 0x0d: /* IP */
		if (!read_inport.isunset())
		{
			r = read_inport();  // TODO: go away
		}
		else
		{
			r = IP_last_state;
		}

		r |= 0x80;  // bit 7 is always set
		break;

	case 0x0e: /* Start counter command */
	{
		if (ACR & 0x40)
		{
			// Reset the timer
			half_period = 0;
		}

		int count = std::max(CTR.w.l, uint16_t(1));
		start_ct(count);
		break;
	}

	case 0x0f: /* Stop counter command */

		// Stop the counter only
		if (!(ACR & 0x40))
		{
			duart_timer->enable(false);

			// clear OP3 counter ready
			if ((OPCR & 0xc) == 0x4)
				OPR &= ~0x8;
		}

		clear_ISR_bits(INT_COUNTER_READY);
		break;

	default:
		LOG("Reading unhandled 68681 reg %x\n", offset);
		break;
	}
	LOG("returned %02x\n", r);

	return r;
}

void mc68681_device::write(offs_t offset, uint8_t data)
{
	if (offset == 0x0c)
		IVR = data;
	else
		duart_base_device::write(offset, data);
}

void mc68340_duart_device::write(offs_t offset, uint8_t data)
{
	//printf("Duart write %02x -> %02x\n", data, offset);

	switch(offset)
	{
	case 0x00: /* MR1A - does not share register address with MR2A */
		m_chanA->write_MR1(data);
		break;
	case 0x08: /* MR1B - does not share register address with MR2B */
		m_chanB->write_MR1(data);
		break;
	case 0x10: /* MR2A - does not share register address with MR1A */
		m_chanA->write_MR2(data);
		break;
	case 0x11: /* MR2B - does not share register address with MR1B */
		m_chanB->write_MR2(data);
		break;
	default:
		duart_base_device::write(offset, data);
	}
}

void sc28c94_device::write(offs_t offset, uint8_t data)
{
	offset &= 0x3f;

	if (offset < 0x10)
	{
		duart_base_device::write(offset, data);
	}

	switch (offset)
	{
	case 0x10: /* MRC */
	case 0x11: /* CSRC */
	case 0x12: /* CRC */
	case 0x13: /* THRC */
		m_chanC->write_chan_reg(offset&3, data);
		break;

	case 0x18: /* MRD */
	case 0x19: /* CSRD */
	case 0x1a: /* CRD */
	case 0x1b: /* THRD */
		m_chanD->write_chan_reg(offset&3, data);
		break;
	}
}

void xr68c681_device::write(offs_t offset, uint8_t data)
{
	if (offset == 0x02) /* CRA */
	{
		switch (data >> 4)
		{
		case 0x08: /* set RX extend bit */
			m_XRXA = true;
			m_chanA->baud_updated();
			data &= 0x0f; /* disable command before we send it off to 68681 */
			break;

		case 0x09: /* clear RX extend bit */
			m_XRXA = false;
			m_chanA->baud_updated();
			data &= 0x0f;
			break;

		case 0x0a: /* set TX extend bit */
			m_XTXA = true;
			m_chanA->baud_updated();
			data &= 0x0f;
			break;

		case 0x0b: /* clear TX extend bit */
			m_XTXA = false;
			m_chanA->baud_updated();
			data &= 0x0f;
			break;

		case 0x0c: /* enter low power mode TODO: unimplemented */
		case 0x0d: /* leave low power mode */
		case 0x0e: /* reserved */
		case 0x0f: /* reserved */
			data &= 0x0f;
			break;
		}
	}
	else if (offset == 0x0a) /* CRB */
	{
		switch (data >> 4)
		{
		case 0x08: /* set RX extend bit */
			m_XRXB = true;
			m_chanB->baud_updated();
			data &= 0x0f;
			break;

		case 0x09: /* clear RX extend bit */
			m_XRXB = false;
			m_chanB->baud_updated();
			data &= 0x0f;
			break;

		case 0x0a: /* set TX extend bit */
			m_XTXB = true;
			m_chanB->baud_updated();
			data &= 0x0f;
			break;

		case 0x0b: /* clear TX extend bit */
			m_XTXB = false;
			m_chanB->baud_updated();
			data &= 0x0f;
			break;

		case 0x0c: /* enter low power mode TODO: unimplemented */
		case 0x0d: /* leave low power mode */
		case 0x0e: /* reserved */
		case 0x0f: /* reserved */
			data &= 0x0f;
			break;
		}
	}

	mc68681_device::write(offset, data); /* pass on 68681 command */
}

void duart_base_device::write(offs_t offset, uint8_t data)
{
	offset &= 0x0f;
	LOG("Writing 68681 (%s) reg %x (%s) with %02x\n", tag(), offset, duart68681_reg_write_names[offset], data);
	switch (offset)
	{
	case 0x00: /* MRA */
	case 0x01: /* CSRA */
	case 0x02: /* CRA */
	case 0x03: /* THRA */
		m_chanA->write_chan_reg(offset&3, data);
		break;

	case 0x04: /* ACR */
	{
		uint8_t old_acr = ACR;
		ACR = data;

		//       bits 6-4: Counter/Timer Mode And Clock Source Select
		//       bits 3-0: IP3-0 Change-Of-State Interrupt Enable
		if ((old_acr ^ data) & 0x40)
		{
			if (data & 0x40)
			{
				// Entering timer mode
				uint16_t count = std::max(CTR.w.l, uint16_t(1));
				half_period = 0;

				start_ct(count);
			}
			else
			{
				// Leaving timer mode (TODO: is this correct?)
				duart_timer->adjust(attotime::never);
			}
		}

		// check for pending input port delta interrupts
		if ((((IPCR>>4) & data) & 0x0f) != 0)
			set_ISR_bits(INT_INPUT_PORT_CHANGE);

		m_chanA->baud_updated();
		m_chanB->baud_updated();
		m_chanA->update_interrupts();
		m_chanB->update_interrupts();
		break;
	}
	case 0x05: /* IMR */
		IMR = data;
		update_interrupts();
		break;

	case 0x06: /* CTUR */
		CTR.b.h = data;
		break;

	case 0x07: /* CTLR */
		CTR.b.l = data;
		break;

	case 0x08: /* MRB */
	case 0x09: /* CSRB */
	case 0x0a: /* CRB */
	case 0x0b: /* THRB */
		m_chanB->write_chan_reg(offset&3, data);
		break;

	case 0x0d: /* OPCR */
		if (((data & 0xf) != 0x00) && ((data & 0xc) != 0x4))
			logerror("68681 (%s): Unhandled OPCR value: %02x\n", tag(), data);
		OPCR = data;
		break;

	case 0x0e: /* Set Output Port Bits */
		OPR |= data;
		write_outport(OPR ^ 0xff);
		break;

	case 0x0f: /* Reset Output Port Bits */
		OPR &= ~data;
		write_outport(OPR ^ 0xff);
		break;
	}
}

void duart_base_device::ip0_w(int state)
{
	uint8_t newIP = (IP_last_state & ~0x01) | ((state == ASSERT_LINE) ? 1 : 0);

	if (newIP != IP_last_state)
	{
		IPCR &= ~0x0f;
		IPCR |= (newIP & 0x0f);
		IPCR |= 0x10;

		if (ACR & 1)
			set_ISR_bits(INT_INPUT_PORT_CHANGE);
	}

	IP_last_state = newIP;
}

void duart_base_device::ip1_w(int state)
{
	uint8_t newIP = (IP_last_state & ~0x02) | ((state == ASSERT_LINE) ? 2 : 0);

	if (newIP != IP_last_state)
	{
		IPCR &= ~0x0f;
		IPCR |= (newIP & 0x0f);
		IPCR |= 0x20;

		if (ACR & 2)
			set_ISR_bits(INT_INPUT_PORT_CHANGE);
	}

	IP_last_state = newIP;
}

void duart_base_device::ip2_w(int state)
{
	uint8_t newIP = (IP_last_state & ~0x04) | ((state == ASSERT_LINE) ? 4 : 0);

	if (newIP != IP_last_state)
	{
		IPCR &= ~0x0f;
		IPCR |= (newIP & 0x0f);
		IPCR |= 0x40;

		if (ACR & 4)
			set_ISR_bits(INT_INPUT_PORT_CHANGE);
	}

	IP_last_state = newIP;
}

void duart_base_device::ip3_w(int state)
{
	uint8_t newIP = (IP_last_state & ~0x08) | ((state == ASSERT_LINE) ? 8 : 0);

	if (newIP != IP_last_state)
	{
		IPCR &= ~0x0f;
		IPCR |= (newIP & 0x0f);
		IPCR |= 0x80;

		if (ACR & 8)
			set_ISR_bits(INT_INPUT_PORT_CHANGE);
	}

	IP_last_state = newIP;
}

void duart_base_device::ip4_w(int state)
{
	uint8_t newIP = (IP_last_state & ~0x10) | ((state == ASSERT_LINE) ? 0x10 : 0);
// TODO: special mode for ip4 (Ch. A Rx clock)
	IP_last_state = newIP;
}

void duart_base_device::ip5_w(int state)
{
	uint8_t newIP = (IP_last_state & ~0x20) | ((state == ASSERT_LINE) ? 0x20 : 0);
// TODO: special mode for ip5 (Ch. B Tx clock)
	IP_last_state = newIP;
}

void duart_base_device::ip6_w(int state)
{
	uint8_t newIP = (IP_last_state & ~0x40) | ((state == ASSERT_LINE) ? 0x40 : 0);
// TODO: special mode for ip6 (Ch. B Rx clock)
	IP_last_state = newIP;
}

duart_channel *duart_base_device::get_channel(int chan)
{
	if (chan == 0)
	{
		return m_chanA;
	}

	return m_chanB;
}

int duart_base_device::calc_baud(int ch, bool rx, uint8_t data)
{
	int baud_rate;

	if (BIT(ACR, 7) == 0)
	{
		baud_rate = baud_rate_ACR_0[data & 0x0f];

		if (ch == 0)
		{
			if ((data & 0xf) == 0xe)
			{
				baud_rate = ip3clk/16;
			}
			else if ((data & 0xf) == 0xf)
			{
				baud_rate = ip3clk;
			}
		}
		else if (ch == 1)
		{
			if ((data & 0xf) == 0xe)
			{
				baud_rate = ip5clk/16;
			}
			else if ((data & 0xf) == 0xf)
			{
				baud_rate = ip5clk;
			}
		}
	}
	else
	{
		baud_rate = baud_rate_ACR_1[data & 0x0f];
	}

	if ((baud_rate == 0) && ((data & 0xf) != 0xd))
	{
		LOG("Unsupported transmitter clock: channel %d, clock select = %02x\n", ch, data);
	}

	//printf("%s ch %d setting baud to %d\n", tag(), ch, baud_rate);
	return baud_rate;
}

int mc68340_duart_device::calc_baud(int ch, bool rx, uint8_t data)
{
	int baud_rate;

	if (BIT(ACR, 7) == 0)
	{
		baud_rate = baud_rate_ACR_0_340[data & 0x0f];

		if (ch == 0)
		{
			if ((data & 0xf) == 0xe)
			{
				baud_rate = ip3clk/16;
			}
			else if ((data & 0xf) == 0xf)
			{
				baud_rate = ip3clk;
			}
		}
		else if (ch == 1)
		{
			if ((data & 0xf) == 0xe)
			{
				baud_rate = ip5clk/16;
			}
			else if ((data & 0xf) == 0xf)
			{
				baud_rate = ip5clk;
			}
		}
	}
	else
	{
		baud_rate = baud_rate_ACR_1_340[data & 0x0f];

		if (ch == 0)
		{
			if ((data & 0xf) == 0xe)
			{
				baud_rate = ip3clk/16;
			}
			else if ((data & 0xf) == 0xf)
			{
				baud_rate = ip3clk;
			}
		}
		else if (ch == 1)
		{
			if ((data & 0xf) == 0xe)
			{
				baud_rate = ip5clk/16;
			}
			else if ((data & 0xf) == 0xf)
			{
				baud_rate = ip5clk;
			}
		}
	}

	if ((baud_rate == 0) && ((data & 0xf) != 0xd))
	{
		LOG("Unsupported transmitter clock: channel %d, clock select = %02x\n", ch, data);
	}

	//printf("%s ch %d setting baud to %d\n", tag(), ch, baud_rate);
	return baud_rate;
}

int xr68c681_device::calc_baud(int ch, bool rx, uint8_t data)
{
	int baud_rate;

	baud_rate = baud_rate_ACR_0[data & 0x0f];

	if (ch == 0)
	{
		if ((data & 0x0f) == 0x0e)
			baud_rate = ip3clk/16;
		else if ((data & 0x0f) == 0x0f)
			baud_rate = ip3clk;
		else if ((rx && m_XRXA) || (!rx && m_XTXA)) /* X = 1 */
			baud_rate = BIT(ACR, 7) == 0 ? baud_rate_ACR_0_X_1[data & 0x0f] : baud_rate_ACR_1_X_1[data & 0x0f];
		else /* X = 0 */
			baud_rate = BIT(ACR, 7) == 0 ? baud_rate_ACR_0[data & 0x0f] : baud_rate_ACR_1[data & 0x0f];
	}
	else if (ch == 1)
	{
		if ((data & 0x0f) == 0x0e)
			baud_rate = ip5clk/16;
		else if ((data & 0x0f) == 0x0f)
			baud_rate = ip5clk;
		else if ((rx && m_XRXB) || (!rx && m_XTXB)) /* X = 1 */
			baud_rate = BIT(ACR, 7) == 0 ? baud_rate_ACR_0_X_1[data & 0x0f] : baud_rate_ACR_1_X_1[data & 0x0f];
		else /* X = 0 */
			baud_rate = BIT(ACR, 7) == 0 ? baud_rate_ACR_0[data & 0x0f] : baud_rate_ACR_1[data & 0x0f];
	}

	if ((baud_rate == 0) && ((data & 0xf) != 0xd))
	{
		LOG("Unsupported transmitter clock: channel %d, clock select = %02x\n", ch, data);
	}

	//printf("%s ch %d setting baud to %d\n", tag(), ch, baud_rate);
	return baud_rate;
}

void duart_base_device::clear_ISR_bits(int mask)
{
	if (ISR & mask)
	{
		ISR &= ~mask;
		update_interrupts();
	}
}

void duart_base_device::set_ISR_bits(int mask)
{
	if (!(ISR & mask))
	{
		ISR |= mask;
		update_interrupts();
	}
}

// DUART channel class stuff

duart_channel::duart_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DUART_CHANNEL, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, MR1(0)
	, MR2(0)
	, SR(0)
	, rx_enabled(0)
	, rx_fifo_num(0)
	, m_tx_data_in_buffer(false)
	, m_tx_break(false)
	, m_bits_transmitted(255)
{
	std::fill_n(&rx_fifo[0], MC68681_RX_FIFO_SIZE + 1, 0);
}

void duart_channel::device_start()
{
	m_uart = downcast<duart_base_device *>(owner());
	m_ch = m_uart->get_ch(this);    // get our channel number

	save_item(NAME(CR));
	save_item(NAME(CSR));
	save_item(NAME(MR1));
	save_item(NAME(MR2));
	save_item(NAME(MR_ptr));
	save_item(NAME(SR));
	save_item(NAME(rx_baud_rate));
	save_item(NAME(tx_baud_rate));
	save_item(NAME(rx_enabled));
	save_item(NAME(rx_fifo));
	save_item(NAME(rx_fifo_read_ptr));
	save_item(NAME(rx_fifo_write_ptr));
	save_item(NAME(rx_fifo_num));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_tx_data_in_buffer));
	save_item(NAME(m_tx_break));
	save_item(NAME(m_bits_transmitted));
}

void duart_channel::device_reset()
{
	write_CR(0x10); // reset MR
	write_CR(0x20); // reset Rx
	write_CR(0x30); // reset Tx
	write_CR(0x40); // reset errors

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);

	tx_baud_rate = rx_baud_rate = 0;
	CSR = 0;
	m_tx_break = false;
}

// serial device virtual overrides
void duart_channel::rcv_complete()
{
	receive_register_extract();

	//printf("%s ch %d rcv complete\n", tag(), m_ch);

	if (rx_enabled)
	{
		uint8_t errors = 0;
		if (is_receive_framing_error())
			errors |= STATUS_FRAMING_ERROR;
		if (is_receive_parity_error())
			errors |= STATUS_PARITY_ERROR;
		rx_fifo_push(get_received_char(), errors);
	}
}

void duart_channel::rx_fifo_push(uint8_t data, uint8_t errors)
{
	if (rx_fifo_num == (MC68681_RX_FIFO_SIZE + 1))
	{
		logerror("68681: FIFO overflow\n");
		SR |= STATUS_OVERRUN_ERROR;
		// In case of overrun the FIFO tail entry is overwritten
		// Back rx_fifo_write_ptr up by one position
		if (rx_fifo_write_ptr)
			rx_fifo_write_ptr--;
		else
			rx_fifo_write_ptr = MC68681_RX_FIFO_SIZE;
	}

	rx_fifo[rx_fifo_write_ptr++] = data | (errors << 8);
	if (rx_fifo_write_ptr > MC68681_RX_FIFO_SIZE)
		rx_fifo_write_ptr = 0;

	if (rx_fifo_num <= MC68681_RX_FIFO_SIZE)
	{
		rx_fifo_num++;
	}

	if (rx_fifo_num == 1)
	{
		SR |= STATUS_RECEIVER_READY;
		if (!(MR1 & MODE_BLOCK_ERROR))
			SR &= ~(STATUS_RECEIVED_BREAK | STATUS_FRAMING_ERROR | STATUS_PARITY_ERROR);
		SR |= errors;
	}
	if (rx_fifo_num == MC68681_RX_FIFO_SIZE)
		SR |= STATUS_FIFO_FULL;
	update_interrupts();
}

void duart_channel::tra_complete()
{
	if (!(SR & STATUS_TRANSMITTER_READY))
	{
		transmit_register_setup(m_tx_data);
		m_bits_transmitted = 0;
		m_tx_data_in_buffer = false;
	}
	else
	{
		SR |= STATUS_TRANSMITTER_EMPTY;
		update_interrupts();
	}
}

void duart_channel::tra_callback()
{
	// don't actually send in loopback mode
	if ((MR2 & 0xc0) != 0x80)
	{
		int bit = transmit_register_get_data_bit();
		//printf("%s ch %d transmit %d\n", tag(), m_ch, bit);
		if (m_ch == 0)
		{
			m_uart->write_a_tx(bit);
		}
		else if (m_ch == 1)
		{
			m_uart->write_b_tx(bit);
		}
		else if (m_ch == 2)
		{
			m_uart->write_c_tx(bit);
		}
		else if (m_ch == 3)
		{
			m_uart->write_d_tx(bit);
		}
	}
	else
		// loop back transmitted bit
		rx_w(transmit_register_get_data_bit());

	// TxRDY is not set until the end of start bit time
	if (++m_bits_transmitted > 1 && !m_tx_data_in_buffer)
	{
		SR |= STATUS_TRANSMITTER_READY;
		update_interrupts();
	}
}

void duart_channel::update_interrupts()
{
	// Handle the TxEMT and TxRDY bits based on mode
	switch (MR2 & 0xc0) // what mode are we in?
	{
	case 0x00: // normal mode
		break;
	case 0x40: // automatic echo mode
		SR &= ~STATUS_TRANSMITTER_READY;
		break;
	case 0x80: // local loopback mode
		break;
	case 0xc0: // remote loopback mode
		SR &= ~STATUS_TRANSMITTER_READY;
		break;
	}

	// now handle the ISR bits
	if (SR & STATUS_TRANSMITTER_READY)
	{
		if (m_ch == 0)
			m_uart->set_ISR_bits(INT_TXRDYA);
		else
			m_uart->set_ISR_bits(INT_TXRDYB);
	}
	else
	{
		if (m_ch == 0)
			m_uart->clear_ISR_bits(INT_TXRDYA);
		else
			m_uart->clear_ISR_bits(INT_TXRDYB);
	}
	//logerror("DEBUG: 68681 int check: before receiver test, SR%c is %02X, ISR is %02X\n", (ch+0x41), duart68681->channel[ch].SR, duart68681->ISR);
	if (MR1 & MODE_RX_INT_SELECT_BIT)
	{
		if (SR & STATUS_FIFO_FULL)
		{
			m_uart->set_ISR_bits((m_ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB);
		}
		else
		{
			m_uart->clear_ISR_bits((m_ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB);
		}
	}
	else
	{
		if (SR & STATUS_RECEIVER_READY)
		{
			m_uart->set_ISR_bits((m_ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB);
		}
		else
		{
			m_uart->clear_ISR_bits((m_ch == 0) ? INT_RXRDY_FFULLA : INT_RXRDY_FFULLB);
		}
	}

	//logerror("DEBUG: 68681 int check: after receiver test, SR%c is %02X, ISR is %02X\n", (ch+0x41), duart68681->channel[ch].SR, duart68681->ISR);
}

uint8_t duart_channel::read_rx_fifo()
{
	uint8_t rv;

	//printf("read_rx_fifo: rx_fifo_num %d\n", rx_fifo_num);

	if (rx_fifo_num == 0)
	{
		LOG("68681 channel: rx fifo underflow\n");
		update_interrupts();
		return 0;
	}

	rv = rx_fifo[rx_fifo_read_ptr++];
	if (rx_fifo_read_ptr > MC68681_RX_FIFO_SIZE)
	{
		rx_fifo_read_ptr = 0;
	}

	rx_fifo_num--;
	if (rx_fifo_num == (MC68681_RX_FIFO_SIZE - 1))
	{
		SR &= ~STATUS_FIFO_FULL;
	}

	if (!(MR1 & MODE_BLOCK_ERROR))
		SR &= ~(STATUS_RECEIVED_BREAK | STATUS_FRAMING_ERROR | STATUS_PARITY_ERROR);
	if (rx_fifo_num == 0)
		SR &= ~STATUS_RECEIVER_READY;
	else
		SR |= rx_fifo[rx_fifo_read_ptr] >> 8;
	update_interrupts();

	//printf("Rx read %02x\n", rv);

	return rv;
}

uint8_t duart_channel::read_chan_reg(int reg)
{
	uint8_t rv = 0xff;

	switch (reg)
	{
	case 0: // MR1/MR2
		if (MR_ptr == 0)
		{
			rv = MR1;
			MR_ptr = 1;
		}
		else
		{
			rv = MR2;
		}
		break;

	case 1: // SRA
		rv = SR;
		break;

	case 2: // CSRA: reading this is prohibited
		break;

	case 3: // Rx holding register A
		rv = read_rx_fifo();
		break;
	}

	return rv;
}

void duart_channel::write_chan_reg(int reg, uint8_t data)
{
	switch (reg)
	{
	case 0x00: /* MRA */
		write_MR(data);
		break;

	case 0x01: /* CSR */
		CSR = data;
		tx_baud_rate = m_uart->calc_baud(m_ch, false, data & 0xf);
		rx_baud_rate = m_uart->calc_baud(m_ch, true, (data>>4) & 0xf);
	//printf("%s ch %d CSR %02x Tx baud %d Rx baud %d\n", tag(), m_ch, data, tx_baud_rate, rx_baud_rate);
		set_rcv_rate(rx_baud_rate);
		set_tra_rate(tx_baud_rate);
		break;

	case 0x02: /* CR */
		write_CR(data);
		break;

	case 0x03: /* THR */
		write_TX(data);
		break;
	}
}

void duart_channel::write_MR(uint8_t data)
{
	if (MR_ptr == 0)
	{
		MR1 = data;
		MR_ptr = 1;
	}
	else
	{
		MR2 = data;
	}
	recalc_framing();
	update_interrupts();
}

void duart_channel::recalc_framing()
{
	parity_t parity = PARITY_NONE;
	switch ((MR1 >> 3) & 3)
	{
	case 0: // with parity
		if (MR1 & 4)
			parity = PARITY_ODD;
		else
			parity = PARITY_EVEN;
		break;

	case 1: // force parity
		if (MR1 & 4)
			parity = PARITY_MARK;
		else
			parity = PARITY_SPACE;
		break;

	case 2: // no parity
		parity = PARITY_NONE;
		break;

	case 3: // multidrop mode
		// fatalerror("68681: multidrop parity not supported\n");
		// Apollo DEX CPU will test this; omit to abort the emulation
		logerror("68681: multidrop parity not supported\n");
		break;
	}

	stop_bits_t stopbits = STOP_BITS_0;
	switch ((MR2 >> 2) & 3)
	{
	case 0:
	case 1:
		stopbits = STOP_BITS_1;
		break;

	case 2: // "1.5 async, 2 sync"
		stopbits = STOP_BITS_1_5;
		break;

	case 3:
		stopbits = STOP_BITS_2;
		break;
	}

	//printf("%s ch %d MR1 %02x MR2 %02x => %d bits / char, %d stop bits, parity %d\n", tag(), m_ch, MR1, MR2, (MR1 & 3)+5, stopbits, parity);

	set_data_frame(1, (MR1 & 3) + 5, parity, stopbits);
}

void duart_channel::write_CR(uint8_t data)
{
	CR = data;

	switch ((data >> 4) & 0x07)
	{
	case 0: /* No command */
		break;
	case 1: /* Reset MR pointer. Causes the channel MR pointer to point to MR1 */
		MR_ptr = 0;
		break;
	case 2: /* Reset channel receiver (disable receiver and flush fifo) */
		rx_enabled = 0;
		SR &= ~STATUS_RECEIVER_READY;
		SR &= ~(STATUS_RECEIVED_BREAK | STATUS_FRAMING_ERROR | STATUS_PARITY_ERROR);
		SR &= ~STATUS_OVERRUN_ERROR; // is this correct?
		rx_fifo_read_ptr = 0;
		rx_fifo_write_ptr = 0;
		rx_fifo_num = 0;
		receive_register_reset();
		break;
	case 3: /* Reset channel transmitter */
		SR &= ~(STATUS_TRANSMITTER_READY | STATUS_TRANSMITTER_EMPTY);
		if (m_ch == 0)
		{
			m_uart->write_a_tx(1);
			m_uart->clear_ISR_bits(INT_TXRDYA);
		}
		else
		{
			m_uart->write_b_tx(1);
			m_uart->clear_ISR_bits(INT_TXRDYB);
		}
		transmit_register_reset();
		m_bits_transmitted = 255;
		m_tx_data_in_buffer = false;
		break;
	case 4: /* Reset Error Status */
		SR &= ~(STATUS_RECEIVED_BREAK | STATUS_FRAMING_ERROR | STATUS_PARITY_ERROR | STATUS_OVERRUN_ERROR);
		break;
	case 5: /* Reset Channel break change interrupt */
		if (m_ch == 0)
			m_uart->clear_ISR_bits(INT_DELTA_BREAK_A);
		else
			m_uart->clear_ISR_bits(INT_DELTA_BREAK_B);
		break;
	case 6: /* Start Tx break */
		if (!m_tx_break)
		{
			m_tx_break = true;
			if ((MR2 & 0xc0) == 0x80)
			{
				// Local loopback mode: set delta break in ISR, simulate break rx
				if (m_ch == 0)
					m_uart->set_ISR_bits(INT_DELTA_BREAK_A);
				else
					m_uart->set_ISR_bits(INT_DELTA_BREAK_B);
				rx_fifo_push(0 , STATUS_RECEIVED_BREAK);
			}
			// TODO: Actually send break signal
		}
		break;
	case 7: /* Stop tx break */
		if (m_tx_break)
		{
			m_tx_break = false;
			if ((MR2 & 0xc0) == 0x80)
			{
				// Local loopback mode: set delta break in ISR
				if (m_ch == 0)
					m_uart->set_ISR_bits(INT_DELTA_BREAK_A);
				else
					m_uart->set_ISR_bits(INT_DELTA_BREAK_B);
			}
		}
		break;
	default:
		LOG("68681: Unhandled command (%x) in CR%d\n", (data >> 4) & 0x07, m_ch);
		break;
	}

	if (BIT(data, 0))
	{
		rx_enabled = 1;
	}
	if (BIT(data, 1))
	{
		rx_enabled = 0;
		SR &= ~STATUS_RECEIVER_READY;
	}

	if (!(SR & STATUS_TRANSMITTER_READY) && BIT(data, 2))
	{
		SR |= STATUS_TRANSMITTER_READY | STATUS_TRANSMITTER_EMPTY;
		m_tx_data_in_buffer = false;
		if (m_ch == 0)
			m_uart->set_ISR_bits(INT_TXRDYA);
		else
			m_uart->set_ISR_bits(INT_TXRDYB);
	}
	if (BIT(data, 3))
	{
		SR &= ~(STATUS_TRANSMITTER_READY | STATUS_TRANSMITTER_EMPTY);
		m_tx_data_in_buffer = false;
		if (m_ch == 0)
			m_uart->clear_ISR_bits(INT_TXRDYA);
		else
			m_uart->clear_ISR_bits(INT_TXRDYB);
	}

	update_interrupts();
}

void duart_channel::write_TX(uint8_t data)
{
	if (!(SR & STATUS_TRANSMITTER_READY))
	{
		logerror("write_tx transmitter not ready (data 0x%02x discarded)\n", data);
		return;
	}

	SR &= ~(STATUS_TRANSMITTER_READY | STATUS_TRANSMITTER_EMPTY);
	if (!is_transmit_register_empty())
	{
		m_tx_data = data;
		m_tx_data_in_buffer = true;
	}
	else
	{
		transmit_register_setup(data);
		m_bits_transmitted = 0;
	}

	update_interrupts();
}

void duart_channel::baud_updated()
{
	write_chan_reg(1, CSR);
}

uint8_t duart_channel::get_chan_CSR()
{
	return CSR;
}

void duart_channel::tx_16x_clock_w(bool state)
{
	if (state)
	{
		m_tx_prescaler--;
		if (m_tx_prescaler == 8)
		{
			tx_clock_w(true);
		}
		else if (m_tx_prescaler == 0)
		{
			m_tx_prescaler = 16;
			tx_clock_w(false);
		}
	}
}

void duart_channel::rx_16x_clock_w(bool state)
{
	if (!is_receive_register_synchronized())
	{
		// Skip over the start bit once synchonization is achieved
		m_rx_prescaler = 32;
		rx_clock_w(true);
	}
	else if (state)
	{
		m_rx_prescaler--;
		if (m_rx_prescaler == 8)
		{
			rx_clock_w(false);
		}
		else if (m_rx_prescaler == 0)
		{
			m_rx_prescaler = 16;
			rx_clock_w(true);
		}
	}
}
