// license:BSD-3-Clause
// copyright-holders:Dave Rand
/**********************************************************************

    Xerox 820-II Shugart SASI host-adapter daughterboard (9R80758)

**********************************************************************/

#include "emu.h"
#include "sasi.h"

#include "bus/nscsi/devices.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(X820_SASI_HOST, x820_sasi_host_device, "x820_sasi_host", "Xerox 820-II SASI host adapter")
DEFINE_DEVICE_TYPE(XEROX820_SASI, xerox820_sasi_device, "xerox820_sasi", "Xerox 820-II SASI host adapter (8\" + ST-506)")


//**************************************************************************
//  x820_sasi_host_device - line-level Z80PIO <-> nscsi bridge
//**************************************************************************

x820_sasi_host_device::x820_sasi_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, X820_SASI_HOST, tag, owner, clock)
	, nscsi_device_interface(mconfig, *this)
	, m_ack_timer(nullptr)
{
}

void x820_sasi_host_device::device_start()
{
	m_ack_timer = timer_alloc(FUNC(x820_sasi_host_device::ack_off), this);
}

void x820_sasi_host_device::device_reset()
{
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, nscsi_device_interface::S_ALL);
	m_scsi_bus->data_w(m_scsi_refid, 0);
	// register for target-driven line changes so scsi_ctrl_changed() fires
	// (the bus only dispatches to devices whose wait mask overlaps)
	constexpr u32 target_mask =
		nscsi_device_interface::S_BSY |
		nscsi_device_interface::S_REQ |
		nscsi_device_interface::S_MSG |
		nscsi_device_interface::S_CTL |
		nscsi_device_interface::S_INP;
	m_scsi_bus->ctrl_wait(m_scsi_refid, target_mask, target_mask);
}

void x820_sasi_host_device::scsi_ctrl_changed()
{
	// the boot ROM polls port B; no edge-triggered host logic to drive
}

TIMER_CALLBACK_MEMBER(x820_sasi_host_device::ack_off)
{
	// drop ACK and release the data bus (essential on target-driven phases:
	// a stale host byte would OR with the target's data)
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, nscsi_device_interface::S_ACK);
	m_scsi_bus->data_w(m_scsi_refid, 0);
}

uint8_t x820_sasi_host_device::data_r()
{
	const uint8_t v = m_scsi_bus->data_r();
	// PARDY pulses U11 on a port-A read too: ACK each byte taken during a
	// connected transfer phase (never during selection)
	if (!machine().side_effects_disabled() && (m_scsi_bus->ctrl_r() & S_BSY))
	{
		m_scsi_bus->ctrl_w(m_scsi_refid, S_ACK, S_ACK);
		m_ack_timer->adjust(SASI_PULSE);
	}
	return v;
}

void x820_sasi_host_device::data_w(uint8_t data)
{
	m_scsi_bus->data_w(m_scsi_refid, data);
	if (m_scsi_bus->ctrl_r() & S_BSY)
	{
		m_scsi_bus->ctrl_w(m_scsi_refid, S_ACK, S_ACK);
		m_ack_timer->adjust(SASI_PULSE);
	}
}

uint8_t x820_sasi_host_device::ctrl_r()
{
	/*
	    bit     description
	    0       NBSY
	    1       NMSG
	    2       NC/D
	    3       NREQ
	    4       NI/O
	    6       LS74 Q (U11; 0 = no parity error, the boot's f693 gate)
	*/
	const u32 ctrl = m_scsi_bus->ctrl_r();
	uint8_t data = 0;
	if (ctrl & S_BSY) data |= 0x01;
	if (ctrl & S_MSG) data |= 0x02;
	if (ctrl & S_CTL) data |= 0x04;
	if (ctrl & S_REQ) data |= 0x08;
	if (ctrl & S_INP) data |= 0x10;
	return data;
}

void x820_sasi_host_device::ctrl_w(uint8_t data)
{
	/*
	    bit     description
	    5       NSEL
	    7       NRST (init writes 0x80 then 0x00 = RST pulse)
	*/
	m_scsi_bus->ctrl_w(m_scsi_refid,
			(BIT(data, 5) ? S_SEL : 0) | (BIT(data, 7) ? S_RST : 0),
			S_SEL | S_RST);
	// release the data bus when SEL drops so the target can drive COMMAND
	if (!BIT(data, 5))
		m_scsi_bus->data_w(m_scsi_refid, 0);
}


//**************************************************************************
//  xerox820_sasi_device
//**************************************************************************

xerox820_sasi_device::xerox820_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XEROX820_SASI, tag, owner, clock)
	, device_xerox820_dbslot_card_interface(mconfig, *this)
	, m_pio(*this, "u8pio")
	, m_host(*this, "host")
	, m_sasibus(*this, "sasi")
{
}


void xerox820_sasi_device::device_add_mconfig(machine_config &config)
{
	Z80PIO(config, m_pio, 20_MHz_XTAL / 8);
	m_pio->out_int_callback().set([this](int state) { m_slot->int_w(state); });
	m_pio->in_pa_callback().set(m_host, FUNC(x820_sasi_host_device::data_r));
	m_pio->out_pa_callback().set(m_host, FUNC(x820_sasi_host_device::data_w));
	m_pio->in_pb_callback().set(m_host, FUNC(x820_sasi_host_device::ctrl_r));
	m_pio->out_pb_callback().set(m_host, FUNC(x820_sasi_host_device::ctrl_w));

	// SASI bus: SA1403D controller at target id 0 (SEL with DB0 = controller
	// id), host adapter bridged through the u8 PIO.  Floppies on LUN 0-2 and
	// the rigid disk on LUN 3, per the 9R80758 unit wiring.
	NSCSI_BUS(config, m_sasibus);
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, "sa1403d");
	X820_SASI_HOST(config, m_host);
	m_sasibus->set_external_device(7, m_host);
}

void xerox820_sasi_device::device_start()
{
}


//-------------------------------------------------
//  io_r / io_w - Z80 ports 0x10-0x13 (u8 PIO, register-select ordering)
//-------------------------------------------------

uint8_t xerox820_sasi_device::io_r(offs_t offset)
{
	return m_pio->read_alt(offset & 0x03);
}

void xerox820_sasi_device::io_w(offs_t offset, uint8_t data)
{
	m_pio->write_alt(offset & 0x03, data);
}


//-------------------------------------------------
//  Z80 daisy chain - the u8 PIO participates
//-------------------------------------------------

int xerox820_sasi_device::z80daisy_irq_state()
{
	return static_cast<device_z80daisy_interface &>(*m_pio).z80daisy_irq_state();
}

int xerox820_sasi_device::z80daisy_irq_ack()
{
	return static_cast<device_z80daisy_interface &>(*m_pio).z80daisy_irq_ack();
}

void xerox820_sasi_device::z80daisy_irq_reti()
{
	static_cast<device_z80daisy_interface &>(*m_pio).z80daisy_irq_reti();
}
