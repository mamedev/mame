// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    DEC PC11 paper tape reader and punch controller (punch not implemented)

    FIXME: PC11 is a Unibus device with no direct Q-bus counterpart

***************************************************************************/

#include "emu.h"
#include "pc11.h"


//#define LOG_GENERAL (1U <<  0)
#define LOG_DBG     (1U <<  2)

//#define VERBOSE (LOG_GENERAL | LOG_DBG)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGDBG(...)   LOGMASKED(LOG_DBG, __VA_ARGS__)


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DEC_PC11, pc11_device, "pc11", "DEC PC11 controller")


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	REGISTER_RCSR = 0,
	REGISTER_RBUF,
	REGISTER_TCSR,
	REGISTER_TBUF
};

const char* pc11_regnames[] = {
	"RCSR", "RBUF", "TCSR", "TBUF"
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc11_device - constructor
//-------------------------------------------------

pc11_device::pc11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: paper_tape_reader_device(mconfig, DEC_PC11, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_rxvec(070)
	, m_txvec(074)
{
	(void)pc11_regnames;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc11_device::device_start()
{
	m_bus->install_device(0177550, 0177557, read16sm_delegate(*this, FUNC(pc11_device::read)),
		write16sm_delegate(*this, FUNC(pc11_device::write)));

	// resolve callbacks

	// save state
	save_item(NAME(m_rcsr));
	save_item(NAME(m_rbuf));
	save_item(NAME(m_tcsr));
	save_item(NAME(m_tbuf));

	// about 300 cps
	emu_timer *timer = timer_alloc();
	timer->adjust(attotime::from_usec(333), 0, attotime::from_usec(333));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc11_device::device_reset()
{
	LOG("Reset, rxvec %03o txvec %03o\n", m_rxvec, m_txvec);

	m_rcsr = m_rbuf = m_tbuf = 0;
	m_tcsr = CSR_DONE;
	m_rxrdy = m_txrdy = CLEAR_LINE;

	m_bus->birq4_w(CLEAR_LINE);
}

int pc11_device::z80daisy_irq_state()
{
	if (m_rxrdy == ASSERT_LINE || m_txrdy == ASSERT_LINE)
		return Z80_DAISY_INT;
	else
		return 0;
}

int pc11_device::z80daisy_irq_ack()
{
	int vec = -1;

	if (m_rxrdy == ASSERT_LINE)
	{
		m_rxrdy = CLEAR_LINE;
		vec = m_rxvec;
	}
	else if (m_txrdy == ASSERT_LINE)
	{
		m_txrdy = CLEAR_LINE;
		vec = m_txvec;
	}

	if (vec > 0)
		LOGDBG("IRQ ACK vec %03o\n", vec);

	return vec;
}

void pc11_device::z80daisy_irq_reti()
{
}


image_init_result pc11_device::call_load()
{
	/* reader unit */
	m_fd = this;
	m_rcsr &= ~(CSR_ERR | CSR_BUSY);

	LOG("call_load filename %s is_open %d\n", m_fd->filename(), m_fd->is_open());

	return image_init_result::PASS;
}

void pc11_device::call_unload()
{
	/* reader unit */
	m_fd = nullptr;
	m_rcsr |= CSR_ERR;
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t pc11_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset & 0x03)
	{
	case REGISTER_RCSR:
		data = m_rcsr & PTRCSR_IMP;
		break;

	case REGISTER_RBUF:
		data = m_rbuf & 0377;
		m_rcsr &= ~CSR_DONE;
		clear_virq(m_bus->birq4_w, m_rcsr, CSR_IE, m_rxrdy);
		break;
	}

	LOGDBG("R %06o == %06o\n", 0177550 + (offset << 1), data);

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void pc11_device::write(offs_t offset, uint16_t data)
{
	LOGDBG("W %06o <- %06o\n", 0177550 + (offset << 1), data);

	switch (offset & 0x03)
	{
	case REGISTER_RCSR:
		if ((data & CSR_IE) == 0)
		{
			clear_virq(m_bus->birq4_w, 1, 1, m_rxrdy);
		}
		else if ((m_rcsr & (CSR_DONE + CSR_IE)) == CSR_DONE)
		{
			raise_virq(m_bus->birq4_w, 1, 1, m_rxrdy);
		}
		if (data & CSR_GO)
		{
			m_rcsr = (m_rcsr & ~CSR_DONE) | CSR_BUSY;
			clear_virq(m_bus->birq4_w, m_rcsr, CSR_IE, m_rxrdy);
		}
		m_rcsr = ((m_rcsr & ~PTRCSR_WR) | (data & PTRCSR_WR));
		break;

	case REGISTER_RBUF:
		break;
	}
}

void pc11_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	uint8_t reply;

	if (!(m_rcsr & CSR_BUSY))
		return;

	LOGDBG("Timer rcsr %06o id %d param %d m_fd %p\n", m_rcsr, id, param, m_fd);

	m_rcsr = (m_rcsr | CSR_ERR) & ~CSR_BUSY;
	if (m_fd && m_fd->exists() && (m_fd->fread(&reply, 1) == 1))
	{
		m_rcsr = (m_rcsr | CSR_DONE) & ~CSR_ERR;
		m_rbuf = reply;
	}
	raise_virq(m_bus->birq4_w, m_rcsr, CSR_IE, m_rxrdy);
}
