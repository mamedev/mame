// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    z180asci.cpp

*********************************************************************/

#include "emu.h"
#include "z180.h"

//#define VERBOSE 1

#include "logmacro.h"


/* 00 ASCI control register A ch 0 */
#define Z180_CNTLA0_MPE         0x80
#define Z180_CNTLA0_RE          0x40
#define Z180_CNTLA0_TE          0x20
#define Z180_CNTLA0_RTS0        0x10
#define Z180_CNTLA0_MPBR_EFR    0x08
#define Z180_CNTLA0_MODE_DATA   0x04
#define Z180_CNTLA0_MODE_PARITY 0x02
#define Z180_CNTLA0_MODE_STOPB  0x01

/* 01 ASCI control register A ch 1 */
#define Z180_CNTLA1_MPE         0x80
#define Z180_CNTLA1_RE          0x40
#define Z180_CNTLA1_TE          0x20
#define Z180_CNTLA1_CKA1D       0x10
#define Z180_CNTLA1_MPBR_EFR    0x08
#define Z180_CNTLA1_MODE        0x07

/* 02 ASCI control register B ch 0 */
#define Z180_CNTLB0_MPBT        0x80
#define Z180_CNTLB0_MP          0x40
#define Z180_CNTLB0_CTS_PS      0x20
#define Z180_CNTLB0_PEO         0x10
#define Z180_CNTLB0_DR          0x08
#define Z180_CNTLB0_SS          0x07

/* 03 ASCI control register B ch 1 */
#define Z180_CNTLB1_MPBT        0x80
#define Z180_CNTLB1_MP          0x40
#define Z180_CNTLB1_CTS_PS      0x20
#define Z180_CNTLB1_PEO         0x10
#define Z180_CNTLB1_DR          0x08
#define Z180_CNTLB1_SS          0x07

/* 04 ASCI status register 0 (all bits read-only except RIE and TIE) */
#define Z180_STAT0_RDRF         0x80
#define Z180_STAT0_OVRN         0x40
#define Z180_STAT0_PE           0x20
#define Z180_STAT0_FE           0x10
#define Z180_STAT0_RIE          0x08
#define Z180_STAT0_DCD0         0x04
#define Z180_STAT0_TDRE         0x02
#define Z180_STAT0_TIE          0x01

/* 05 ASCI status register 1 (all bits read-only except RIE, CTS1E and TIE) */
#define Z180_STAT1_RDRF         0x80
#define Z180_STAT1_OVRN         0x40
#define Z180_STAT1_PE           0x20
#define Z180_STAT1_FE           0x10
#define Z180_STAT1_RIE          0x08
#define Z180_STAT1_CTS1E        0x04
#define Z180_STAT1_TDRE         0x02
#define Z180_STAT1_TIE          0x01

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(Z180ASCI_CHANNEL,   z180asci_channel,  "z180asci_channel",    "Z180 ASCI Channel")


//-------------------------------------------------
//  z180asci_channel - constructor
//-------------------------------------------------

z180asci_channel::z180asci_channel(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	device_buffered_serial_interface(mconfig, *this),
	m_tx_handler(*this)
{
	m_asci_cntla = 0;
	m_asci_cntlb = 7; // SS2,SS1 and SS0 are 1 after reset
	m_asci_stat = 2; // set TDRE=1 on reset
	m_asci_tdr = 0;
	m_asci_rdr = 0;
	m_id = 0;
	m_irq = 0;
}

z180asci_channel::z180asci_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z180asci_channel(mconfig, Z180ASCI_CHANNEL, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void z180asci_channel::device_resolve_objects()
{
	// resolve callbacks
	m_tx_handler.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z180asci_channel::device_start()
{
	save_item(NAME(m_asci_cntla));
	save_item(NAME(m_asci_cntlb));
	save_item(NAME(m_asci_stat));
	save_item(NAME(m_asci_tdr));
	save_item(NAME(m_asci_rdr));

	transmit_register_reset();
	receive_register_reset();

	cpu_device *cpu = reinterpret_cast<cpu_device *>(owner());
	if (m_id==0) {
		cpu->state_add(Z180_CNTLA0,     "CNTLA0",    m_asci_cntla);
		cpu->state_add(Z180_CNTLB0,     "CNTLB0",    m_asci_cntlb);
		cpu->state_add(Z180_STAT0,      "STAT0",     m_asci_stat);
		cpu->state_add(Z180_TDR0,       "TDR0",      m_asci_tdr);
		cpu->state_add(Z180_RDR0,       "RDR0",      m_asci_rdr);
	} else {
		cpu->state_add(Z180_CNTLA1,     "CNTLA1",    m_asci_cntla);
		cpu->state_add(Z180_CNTLB1,     "CNTLB1",    m_asci_cntlb);
		cpu->state_add(Z180_STAT1,      "STAT1",     m_asci_stat);
		cpu->state_add(Z180_TDR1,       "TDR1",      m_asci_tdr);
		cpu->state_add(Z180_RDR1,       "RDR1",      m_asci_rdr);
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z180asci_channel::device_reset()
{
	m_asci_cntla = 0;
	m_asci_cntlb = 7; // SS2,SS1 and SS0 are 1 after reset
	m_asci_tdr = 0;
	m_asci_rdr = 0;
	if (m_id==0) {
		m_asci_cntla = (m_asci_cntla & Z180_CNTLA0_MPBR_EFR) | Z180_CNTLA0_RTS0;
		m_asci_cntlb = (m_asci_cntlb & (Z180_CNTLB0_MPBT | Z180_CNTLB0_CTS_PS)) | 0x07;
		m_asci_stat = m_asci_stat & (Z180_STAT0_DCD0 | Z180_STAT0_TDRE);
	} else {
		m_asci_cntla = (m_asci_cntla & Z180_CNTLA1_MPBR_EFR) | Z180_CNTLA1_CKA1D;
		m_asci_cntlb = (m_asci_cntlb & Z180_CNTLB1_MPBT) | 0x07;
		m_asci_stat = Z180_STAT1_TDRE;
	}
}

uint8_t z180asci_channel::cntla_r()
{
	uint8_t data = m_asci_cntla;
	LOG("Z180 CNTLA%d rd $%02x\n", m_id, data);
	return data;
}

uint8_t z180asci_channel::cntlb_r()
{
	uint8_t data = m_asci_cntlb;
	LOG("Z180 CNTLB%d rd $%02x\n", m_id, data);
	return data;
}

uint8_t z180asci_channel::stat_r()
{
	uint8_t data = m_asci_stat;
	LOG("Z180 STAT%d  rd $%02x\n", m_id, data);
	return data;
}

uint8_t z180asci_channel::tdr_r()
{
	uint8_t data = m_asci_tdr;
	LOG("Z180 TDR%d   rd $%02x\n", m_id, data);
	return data;
}

uint8_t z180asci_channel::rdr_r()
{
	uint8_t data = m_asci_rdr;
	LOG("Z180 RDR%d   rd $%02x\n", m_id, data);
	m_asci_stat &= ~0x80;

	return data;
}

void z180asci_channel::cntla_w(uint8_t data)
{
	LOG("Z180 CNTLA%d wr $%02x\n", m_id, data);
	m_asci_cntla = data;
	set_data_frame(1, (data & 4) ? 8 : 7, (data & 2) ? PARITY_ODD : PARITY_NONE, (data & 2) ? STOP_BITS_2 : STOP_BITS_1);
}

void z180asci_channel::cntlb_w(uint8_t data)
{
	LOG("Z180 CNTLB%d wr $%02x\n", m_id, data);
	m_asci_cntlb = data;

	int divisor = 1<<(m_asci_cntlb & 0x07);
	divisor *= ((m_asci_cntlb & 0x20) ? 30 : 10);
	divisor *= ((m_asci_cntlb & 0x08) ? 64 : 16);

	set_tra_rate(clock(), divisor);
	set_rcv_rate(clock(), divisor);
}

void z180asci_channel::stat_w(uint8_t data)
{
	if (m_id==0) {
		LOG("Z180 STAT0  wr $%02x ($%02x)\n", data,  data & (Z180_STAT0_RIE | Z180_STAT0_TIE));
		m_asci_stat = (m_asci_stat & ~(Z180_STAT0_RIE | Z180_STAT0_TIE)) | (data & (Z180_STAT0_RIE | Z180_STAT0_TIE));
	} else {
		LOG("Z180 STAT1  wr $%02x ($%02x)\n", data,  data & (Z180_STAT1_RIE | Z180_STAT1_CTS1E | Z180_STAT1_TIE));
		m_asci_stat = (m_asci_stat & ~(Z180_STAT1_RIE | Z180_STAT1_CTS1E | Z180_STAT1_TIE)) | (data & (Z180_STAT1_RIE | Z180_STAT1_CTS1E | Z180_STAT1_TIE));
	}
}

void z180asci_channel::tdr_w(uint8_t data)
{
	LOG("Z180 TDR%d   wr $%02x\n", m_id, data);
	m_asci_tdr = data;
	m_asci_stat &= ~0x02; // clear TDRE
	transmit_byte(m_asci_tdr);
}

void z180asci_channel::tra_callback()
{
	m_tx_handler(transmit_register_get_data_bit());
}

void z180asci_channel::tra_complete()
{
	device_buffered_serial_interface::tra_complete();
	m_asci_stat |= 0x02; // set TDRE
	if (m_asci_stat & Z180_STAT0_TIE) {
		m_irq = 1;
	}
}

void z180asci_channel::rdr_w(uint8_t data)
{
	LOG("Z180 RDR%d   wr $%02x\n", m_id, data);
	m_asci_rdr = data;
}

void z180asci_channel::received_byte(u8 byte)
{
	m_asci_stat |= 0x80;
	m_asci_rdr = byte;
	if (m_asci_stat & Z180_STAT0_RIE) {
		m_irq = 1;
	}
}

DECLARE_WRITE_LINE_MEMBER( z180asci_channel::write_rx )
{
	device_buffered_serial_interface::rx_w(state);
}

int z180asci_channel::check_interrupt()
{
	return m_irq;
}
void z180asci_channel::clear_interrupt()
{
	m_irq = 0;
}
