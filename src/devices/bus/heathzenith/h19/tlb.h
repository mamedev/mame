// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB)

****************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_H19_TLB_H
#define MAME_BUS_HEATHZENITH_H19_TLB_H

#pragma once

class heath_tlb_connector;

class device_heath_tlb_card_interface : public device_interface
{
public:

	// required operation
	virtual void serial_in_w(int state) = 0;

	// optional operations
	virtual void rlsd_in_w(int state) {}
	virtual void dsr_in_w(int state) {}
	virtual void cts_in_w(int state) {}

	// optional SigmaSet operations
	virtual void sigma_w(u8 offset, u8 data) {}
	virtual u8 sigma_r(u8 offset) { return 0x00; }

protected:
	device_heath_tlb_card_interface(const machine_config &mconfig, device_t &device);

	heath_tlb_connector *const m_slot;
};


// Options for the standard H89 class of systems
DECLARE_DEVICE_TYPE(HEATH_TLB,         device_heath_tlb_card_interface)
DECLARE_DEVICE_TYPE(HEATH_GP19,        device_heath_tlb_card_interface)
DECLARE_DEVICE_TYPE(HEATH_IMAGINATOR,  device_heath_tlb_card_interface)
DECLARE_DEVICE_TYPE(HEATH_SUPER19,     device_heath_tlb_card_interface)
DECLARE_DEVICE_TYPE(HEATH_SUPERSET,    device_heath_tlb_card_interface)
DECLARE_DEVICE_TYPE(HEATH_WATZ,        device_heath_tlb_card_interface)
DECLARE_DEVICE_TYPE(HEATH_ULTRA,       device_heath_tlb_card_interface)

// Options for the H89 with the SigmaSoft IGC graphics installed
DECLARE_DEVICE_TYPE(HEATH_IGC,         device_heath_tlb_card_interface)
DECLARE_DEVICE_TYPE(HEATH_IGC_SUPER19, device_heath_tlb_card_interface)
DECLARE_DEVICE_TYPE(HEATH_IGC_ULTRA,   device_heath_tlb_card_interface)
DECLARE_DEVICE_TYPE(HEATH_IGC_WATZ,    device_heath_tlb_card_interface)


/**
 * Connector for the Terminal Logic Board in an H-89 class computer
 */
class heath_tlb_connector : public device_t,
							public device_single_card_slot_interface<device_heath_tlb_card_interface>
{
public:

	template <typename T>
	heath_tlb_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, bool fixed = false) :
		 heath_tlb_connector(mconfig, tag, owner, 0)
	{
		set_options(std::forward<T>(opts), dflt, fixed);
	}

	heath_tlb_connector(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~heath_tlb_connector();

	// computer interface
	auto serial_data_callback() { return m_write_sd.bind(); }
	auto dtr_callback() { return m_dtr_cb.bind(); }
	auto rts_callback() { return m_rts_cb.bind(); }
	auto reset_cb() { return m_reset.bind(); }

	// card interface
	void serial_in_w(int state) { if (m_tlb) m_tlb->serial_in_w(state); }
	void rlsd_in_w(int state) { if (m_tlb) m_tlb->rlsd_in_w(state); }
	void dsr_in_w(int state) { if (m_tlb) m_tlb->dsr_in_w(state); }
	void cts_in_w(int state) { if (m_tlb) m_tlb->cts_in_w(state); }

	// signals out from the tlb
	void serial_out_b(int data) { m_write_sd(data); }
	void dtr_out(int data) { m_dtr_cb(data); }
	void rts_out(int data) { m_rts_cb(data); }
	void reset_out(int data) { m_reset(data); }

	// optional SigmaSet IGC operations
	u8 sigma_r(u8 offset) { return (m_tlb) ? m_tlb->sigma_r(offset) : 0x00; }
	void sigma_w(u8 offset, u8 data) { if (m_tlb) m_tlb->sigma_w(offset, data); }

protected:
	virtual void device_start() override ATTR_COLD;

	devcb_write_line                 m_write_sd;
	devcb_write_line                 m_dtr_cb;
	devcb_write_line                 m_rts_cb;
	devcb_write_line                 m_reset;

	device_heath_tlb_card_interface *m_tlb;
};


// device type declaration
DECLARE_DEVICE_TYPE(HEATH_TLB_CONNECTOR, heath_tlb_connector)

#endif // MAME_BUS_HEATHZENITH_H19_TLB_H
