// license:BSD-3-Clause
// copyright-holders:Chris Toshok
/*
 * apollo_dn300_ring.cpp - the Apollo DN300/DN320 ring network controller
 *
 * Only the registers and the DMA channels are emulated: a transmit is
 * taken from memory and reported as copied, and nothing is ever received.
 *
 *  see also:
 *  - http://www.bitsavers.org/pdf/apollo/002398-04_Domain_Engineering_Handbook_Rev4_Jan87.pdf
 */

#include "emu.h"
#include "apollo_dn300_ring.h"

#define LOG_REGS (1U << 1) // register accesses
#define LOG_DMA  (1U << 2) // DMA bytes

#define VERBOSE (0)
#include "logmacro.h"

#define LOGREGS(...) LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGDMA(...)  LOGMASKED(LOG_DMA, __VA_ARGS__)

//// XMIT_COMMAND
#define XMIT_COMMAND_INTERRUPT_ENABLE 0x4000
#define XMIT_COMMAND_ENABLE 0x2000 // (start the transmit)
#define XMIT_COMAMND_FORCE_TRANSMIT 0x1000
// NOTES from EH87:
//  1. To start a transmit normally, use 6000.
//  2. To force a transmit, use 7000.
//  3. To stop a transmit that has already started, clear the
//     transmit enable bit.
//  4. Writing anything to this register clears the transmit
//     interrupt.


/// RCV_COMMAND
#define RCV_COMMAND_INTERRUPT_ENABLE 0x4000
#define RCV_COMMAND_ENABLE 0x2000 // (start the receive)
// NOTES from EH87:
//  1. To start a receive normally, use 6000.
//  2. To stop a receive that has already started, clear the
//     receive enable bit.


//// XMIT_STATUS
#define XMIT_STATUS_INTERRUPT_PENDING 0x8000
#define XMIT_STATUS_INTERRUPT_ENABLED 0x4000
#define XMIT_STATUS_BUSY              0x2000
#define XMIT_STATUS_DISCONNECTED      0x1000
#define XMIT_STATUS_BIPHASE_ERROR     0x0800
#define XMIT_STATUS_ESB_ERROR         0x0400 // ESB = elastic store buffer
#define XMIT_STATUS_NO_RETURN         0x0200 // a complete pkt frame never arrived
#define XMIT_STATUS_CRC_ERROR         0x0100
#define XMIT_STATUS_ACK_PARITY_ERROR  0x0080 // 0=no error, 1=error detected
#define XMIT_STATUS_EXTERNAL_ERROR    0x0040 // err during DMA, e.g. parity, bus-error
#define XMIT_STATUS_PROTOCOL_ERROR    0x0020 // the pkt hdr with FROM ID never came back
#define XMIT_STATUS_IOCOPY            0x0010 // somebody Intended to COPY -- was willing to rcv
#define XMIT_STATUS_ACK_BYTE_ERRBIT   0x0008 // sombody (anybody!) set the "error detected" bit
#define XMIT_STATUS_COPY              0x0004 // somebody did COPY the pkt
#define XMIT_STATUS_WACK              0x0002
#define XMIT_STATUS_UNDERRUN          0x0001 // DMA didn't keep up with xmit data rate
// NOTES from EH87:
//  1. A successful transmit will have a transmit status of 0014
//  2. A WACK will have a transmit status of 0012


//// RCV_STATUS
#define RCV_STATUS_INTERRUPT_PENDING 0x8000
#define RCV_STATUS_INTERRUPT_ENABLED 0x4000
#define RCV_STATUS_BUSY              0x2000
#define RCV_STATUS_DISCONNECTED      0x1000
#define RCV_STATUS_BIPHASE_ERROR     0x0800
#define RCV_STATUS_ESB_ERROR         0x0400 // ESB = elastic store buffer
#define RCV_STATUS_TIMEOUT           0x0200 // timeout (the hdr of a msg was seen, but it never ended)
#define RCV_STATUS_CRC_ERROR         0x0100
#define RCV_STATUS_ACK_PARITY_ERROR  0x0080 // 0=no error, 1=error detected
#define RCV_STATUS_EXTERNAL_ERROR    0x0040 // err during DMA, e.g. parity, bus-error
#define RCV_STATUS_DMA_END_OF_RANGE  0x0020
#define RCV_STATUS_IOCOPY            0x0010 // somebody before me Intended to COPY
#define RCV_STATUS_ACK_BYTE_ERRBIT   0x0008 // sombody before me set the "error detected" bit
#define RCV_STATUS_COPY 			 0x0004 // somebody before me did COPY the pkt
#define RCV_STATUS_WACK              0x0002 // somebody before me WACKed the pkt
#define RCV_STATUS_OVERRUN           0x0001 // DMA didn't keep up with rcv data rate


////  DIAGNOSTIC_STATUS
#define DIAG_STATUS_INTERRUPT_PENDING  0x8000 // bad_pkt_cnt_overflow interrupt
#define DIAG_STATUS_INTERRUPT_ENABLED  0x4000 // bad_pkt_cnt_overflow interrupt
#define DIAG_STATUS_CONNECTED_TO_NET   0x2000
#define DIAG_STATUS_STICKY_BIPHASE_ERR 0x1000 // error seen since bit was cleared
#define DIAG_STATUS_DELAY_ON           0x0800 // the delay is enabled
#define DIAG_STATUS_STICKY_GOOD_SEEN   0x0400 // good pkt seen since bit was cleared
#define DIAG_STATUS_STICKY_ESB_ERR     0x0200 // error seen since bit was cleared
#define DIAG_STATUS_BAD_PACKET_COUNT   0x1FF  // 9-bit counter for 1st detecting errs
// NOTES from EH87:
//  1. Counter is number of times this node found an
//     error in a packet going by (regardless of packet
//     target node ID), found the error bit in the
//     ackbyte clear, and so was the first to set the
//     error bit to one.
//  2. The bad_pkg_cnt interrupt occurs when the counter
//     counts from 255 to 256 (i.e. first uses its
//     highest order bit).
//  3. The counter sticks at 511 if more than 511 errors
//     are seen.
//  4. Writing anything to the diagnostic command
//     register (word) (see below) clears interrupt and
//     all sticky bits.


// DIAGNOSTIC_COMMAND
#define DIAG_COMMAND_DMA_TEST         0x8000
#define DIAG_COMMAND_ENABLE_INTERRUPT 0x4000
#define DIAG_COMMAND_CONNECT          0x2000
#define DIAG_COMMAND_DISCONNECT       0x1000
#define DIAG_COMMAND_DELAY_OFF        0x0800
#define DIAG_COMMAND_DELAY_ON         0x0400
#define DIAG_COMMAND_SNOOP            0x0200
// NOTES from EH87:
//  Writing anything to the register (word) clears interrupt
//  and all sticky bits in diagnostic status register


// TMASK
#define TMASK_BROADCAST           0x80
#define TMASK_HARDWARE_DIAGNOSTIC 0x40
#define TMASK_THANK_YOU           0x20
#define TMASK_PLEASE              0x10
#define TMASK_PAGING              0x08
#define TMASK_USER                0x04
#define TMASK_SOFTWARE_DIAGNOSTIC 0x02
#define TMASK_XTYPE3              0x01
// NOTES from EH87:
//  Except for BROADCAST, these bits are software defined.

#define PULSE_DRQ(cb) do { cb(true); cb(false); } while (0)

DEFINE_DEVICE_TYPE(APOLLO_DN300_RING_CTRLR, apollo_dn300_ring_ctrlr_device, "apollo_dn300_ring", "Apollo DN300 ring network controller")

apollo_dn300_ring_ctrlr_device::apollo_dn300_ring_ctrlr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	device_t(mconfig, APOLLO_DN300_RING_CTRLR, tag, owner, clock),
	irq_cb(*this),
	rcv_header_drq_wr_cb(*this),
	rcv_data_drq_wr_cb(*this),
	transmit_data_drq_wr_cb(*this),
	m_xmit_status(0),
	m_xmit_command(0),
	m_rcv_status(0),
	m_rcv_command(0),
	m_tmask(0),
	m_diag_status(0),
	m_ring_id_msb(0),
	m_ring_id_lsb(0),
	m_cursor(0)
{
	// the node ID, 12345 (hex)
	m_id[0] = 0x00;
	m_id[1] = 0x01;
	m_id[2] = 0x23;
	m_id[3] = 0x45;
}

void
apollo_dn300_ring_ctrlr_device::device_start()
{
}

void
apollo_dn300_ring_ctrlr_device::device_reset()
{
}

void
apollo_dn300_ring_ctrlr_device::map(address_map &map)
{
	map(0x0,0x1).rw(FUNC(apollo_dn300_ring_ctrlr_device::xmit_status_r), FUNC(apollo_dn300_ring_ctrlr_device::xmit_command_w));
	map(0x2,0x3).rw(FUNC(apollo_dn300_ring_ctrlr_device::rcv_status_r), FUNC(apollo_dn300_ring_ctrlr_device::rcv_command_w));
	map(0x4,0x4).rw(FUNC(apollo_dn300_ring_ctrlr_device::tmask_r), FUNC(apollo_dn300_ring_ctrlr_device::tmask_w));
	map(0x6,0x7).rw(FUNC(apollo_dn300_ring_ctrlr_device::diag_status_r), FUNC(apollo_dn300_ring_ctrlr_device::diag_command_w));
	map(0x8,0xB).rw(FUNC(apollo_dn300_ring_ctrlr_device::ring_id_r), FUNC(apollo_dn300_ring_ctrlr_device::ring_id_w));

	map(0x10,0x16).r(FUNC(apollo_dn300_ring_ctrlr_device::id_r));
}

void
apollo_dn300_ring_ctrlr_device::xmit_command_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGREGS("writing xmit_command at offset %02x = %04x & %08x\n", offset, data, mem_mask);

	uint16_t old_command = m_xmit_command;
	m_xmit_command = data;
	if ((old_command & XMIT_COMMAND_ENABLE) == 0 && (m_xmit_command & XMIT_COMMAND_ENABLE) != 0) {
		// starting a transmit.  we'll do it synchronously.

		m_xmit_status = XMIT_STATUS_BUSY;

		m_cursor = 0;
		for (int i = 0; i < 0x14; i ++) {
			PULSE_DRQ(transmit_data_drq_wr_cb);
		}

		for (int i = 0; i < 0x14; i ++) {
			LOGDMA("pkt[%02x] = %02x\n", i, m_buffer[i]);
		}

		// skip the WACK and just say someone got it?

		m_xmit_status = XMIT_STATUS_IOCOPY | XMIT_STATUS_COPY;
	}
}

uint16_t
apollo_dn300_ring_ctrlr_device::xmit_status_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_xmit_status & mem_mask;
	LOGREGS("reading xmit_status at offset %02x & %08x = %04x\n", offset, mem_mask, data);
	return data;
}

void
apollo_dn300_ring_ctrlr_device::rcv_command_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGREGS("writing rcv_command at offset %02x = %04x & %08x\n", offset, data, mem_mask);
	uint16_t old_command = m_rcv_command;
	m_rcv_command = data;
	if ((old_command & RCV_COMMAND_ENABLE) == 0 && (m_rcv_command & RCV_COMMAND_ENABLE) != 0) {
		// receiving
	}
}

uint16_t
apollo_dn300_ring_ctrlr_device::rcv_status_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_rcv_status & mem_mask;
	LOGREGS("reading rcv_status at offset %02x & %08x = %04x\n", offset, mem_mask, data);
	return data;
}

void
apollo_dn300_ring_ctrlr_device::tmask_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	LOGREGS("writing tmask at offset %02x = %02x & %08x\n", offset, data, mem_mask);
}

uint8_t
apollo_dn300_ring_ctrlr_device::tmask_r(offs_t offset, uint8_t mem_mask)
{
	uint8_t data = m_tmask & mem_mask;
	LOGREGS("reading tmask at offset %02x & %08x = %02x\n", offset, mem_mask, data);
	return data;
}

void
apollo_dn300_ring_ctrlr_device::diag_command_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGREGS("writing diag_command at offset %02x = %04x & %08x\n", offset, data, mem_mask);

	// writing anything to the register clears the interrupt and all sticky bits
	// in the diagnostic status register.
	// TODO: does this clear DIAG_STATUS_INTERRUPT_PENDING too?
	m_diag_status &= ~(
		DIAG_STATUS_INTERRUPT_ENABLED |
		DIAG_STATUS_STICKY_BIPHASE_ERR |
		DIAG_STATUS_STICKY_GOOD_SEEN |
		DIAG_STATUS_STICKY_ESB_ERR
	);
}

uint16_t
apollo_dn300_ring_ctrlr_device::diag_status_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_diag_status & mem_mask;
	LOGREGS("reading diag_status at offset %02x & %08x = %04x\n", offset, mem_mask, data);
	return data;
}

void
apollo_dn300_ring_ctrlr_device::ring_id_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGREGS("writing ring_id at offset %02x = %04x & %08x\n", offset, data, mem_mask);
}

uint16_t
apollo_dn300_ring_ctrlr_device::ring_id_r(offs_t offset, uint16_t mem_mask)
{
	uint8_t data = (offset == 0 ? m_ring_id_msb : m_ring_id_lsb) & mem_mask;
	LOGREGS("reading ring_id at offset %02x & %08x = %04x\n", offset, mem_mask, data);
	return data;
}

uint8_t
apollo_dn300_ring_ctrlr_device::id_r(offs_t offset, uint8_t mem_mask)
{
	uint8_t data = m_id[offset / 2] & mem_mask;
	LOGREGS("reading node_id at offset %02x & %08x = %02x\n", offset, mem_mask, data);
	return data;
}

uint8_t
apollo_dn300_ring_ctrlr_device::rcv_header_read_byte(offs_t offset)
{
	uint8_t data = m_buffer[m_cursor++];
	LOGDMA("rcv_header_read_byte offset %02x = %02x\n", offset, data);
	return data;
}

void
apollo_dn300_ring_ctrlr_device::rcv_header_write_byte(offs_t offset, uint8_t data)
{
	LOGDMA("rcv_header_write_byte at offset %02x = %02x\n", offset, data);
	m_buffer[m_cursor++] = data;
}

uint8_t
apollo_dn300_ring_ctrlr_device::rcv_data_read_byte(offs_t offset)
{
	uint8_t data = m_buffer[m_cursor++];
	LOGDMA("rcv_data_read_byte offset %02x = %02x\n", offset, data);
	return data;
}

void
apollo_dn300_ring_ctrlr_device::rcv_data_write_byte(offs_t offset, uint8_t data)
{
	LOGDMA("rcv_data_write_byte at offset %02x = %02x\n", offset, data);
	m_buffer[m_cursor++] = data;
}

uint8_t
apollo_dn300_ring_ctrlr_device::transmit_read_byte(offs_t offset)
{
	uint8_t data = m_buffer[m_cursor++];
	LOGDMA("transmit_read_byte offset %02x = %02x\n", offset, data);
	return data;
}

void
apollo_dn300_ring_ctrlr_device::transmit_write_byte(offs_t offset, uint8_t data)
{
	LOGDMA("transmit_write_byte at offset %02x = %02x (cursor %d)\n", offset, data, m_cursor);
	m_buffer[m_cursor++] = data;
}
