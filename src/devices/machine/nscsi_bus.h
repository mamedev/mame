// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NSCSI_BUS_H
#define MAME_MACHINE_NSCSI_BUS_H

#pragma once


class nscsi_device_interface;
class nscsi_slot_card_interface;

class nscsi_bus_device : public device_t
{
public:
	nscsi_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto out_bsy_callback() { return m_bsy_handler.bind(); }

	template <typename T> void set_external_device(int id, T &&tag) { m_external_devices[id].set_tag(std::forward<T>(tag)); }

	void ctrl_w(int refid, uint32_t lines, uint32_t mask);
	void data_w(int refid, uint32_t lines);
	void ctrl_wait(int refid, uint32_t lines, uint32_t mask);

	uint32_t ctrl_r() const;
	uint32_t data_r() const;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

private:
	struct dev_t {
		nscsi_device_interface *m_dev;
		uint32_t m_ctrl, m_wait_ctrl;
		uint32_t m_data;
	};

	optional_device_array<nscsi_device_interface, 16> m_external_devices;

	devcb_write_line m_bsy_handler;

	dev_t m_dev[16];
	int m_devcnt;

	uint32_t m_data, m_ctrl;

	void regen_data();
	void regen_ctrl(int refid);
};

class nscsi_connector: public device_t,
					   public device_single_card_slot_interface<nscsi_slot_card_interface>
{
public:
	template <typename T>
	nscsi_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, bool fixed = false)
		: nscsi_connector(mconfig, tag, owner, 0)
	{
		set_options(std::forward<T>(opts), dflt, fixed);
	}
	nscsi_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~nscsi_connector();

	nscsi_device_interface *get_device();

protected:
	virtual void device_start() override ATTR_COLD;
};

class nscsi_slot_card_interface : public device_interface
{
	friend class nscsi_connector;

public:
	nscsi_slot_card_interface(const machine_config &mconfig, device_t &device, const char *nscsi_tag);

private:
	required_device<nscsi_device_interface> m_nscsi;
};

class nscsi_device_interface : public device_interface
{
public:
	// Here because the biggest users are the devices, not the bus
	enum {
		S_INP = 0x0001,
		S_CTL = 0x0002,
		S_MSG = 0x0004,
		S_BSY = 0x0008,
		S_SEL = 0x0010,
		S_REQ = 0x0020,
		S_ACK = 0x0040,
		S_ATN = 0x0080,
		S_RST = 0x0100,
		S_ALL = 0x01ff,

		S_PHASE_DATA_OUT = 0,
		S_PHASE_DATA_IN  = S_INP,
		S_PHASE_COMMAND  = S_CTL,
		S_PHASE_STATUS   = S_CTL|S_INP,
		S_PHASE_MSG_OUT  = S_MSG|S_CTL,
		S_PHASE_MSG_IN   = S_MSG|S_CTL|S_INP,
		S_PHASE_MASK     = S_MSG|S_CTL|S_INP
	};

	// SCSI Messages
	// Here because some controllers interpret messages
	enum {
		SM_COMMAND_COMPLETE              = 0x00,
		SM_EXTENDED_MESSAGE              = 0x01,
		SM_SAVE_DATA_POINTER             = 0x02,
		SM_RESTORE_POINTERS              = 0x03,
		SM_DISCONNECT                    = 0x04,
		SM_INITIATOR_DETECTED_ERROR      = 0x05,
		SM_ABORT                         = 0x06,
		SM_MESSAGE_REJECT                = 0x07,
		SM_NO_OPERATION                  = 0x08,
		SM_MESSAGE_PARITY_ERROR          = 0x09,
		SM_LINKED_COMMAND_COMPLETE       = 0x0a,
		SM_LINKED_COMMAND_COMPLETE_WITH_FLAG = 0x0b,
		SM_BUS_DEVICE_RESET              = 0x0c,
		SM_ABORT_TAG                     = 0x0d,
		SM_CLEAR_QUEUE                   = 0x0e,
		SM_INITIATE_RECOVERY             = 0x0f,
		SM_RELEASE_RECOVERY              = 0x10,
		SM_TERMINATE_IO_PROCESS          = 0x11,
		SM_SIMPLE_QUEUE_TAG              = 0x20,
		SM_HEAD_OF_QUEUE_TAG             = 0x21,
		SM_ORDERED_QUEUE_TAG             = 0x22,
		SM_IGNORE_WIDE_RESIDUE           = 0x23
	};

	void connect_to_bus(nscsi_bus_device *bus, int refid, int default_scsi_id);
	virtual void scsi_ctrl_changed();

protected:
	nscsi_device_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override ATTR_COLD;

	int m_scsi_id;
	int m_scsi_refid;
	nscsi_bus_device *m_scsi_bus;
};

DECLARE_DEVICE_TYPE(NSCSI_BUS,       nscsi_bus_device)
DECLARE_DEVICE_TYPE(NSCSI_CONNECTOR, nscsi_connector)

#endif // MAME_MACHINE_NSCSI_BUS_H
