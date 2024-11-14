// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_DBDMA_H
#define MAME_APPLE_DBDMA_H

#pragma once


// ======================> dbdma_device

class dbdma_device :  public device_t
{
public:
	// construction/destruction
	dbdma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, address_space *space)
		: dbdma_device(mconfig, tag, owner, clock)
	{
		set_address_space(space);
	}

	dbdma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u32 dma_read(u32 offset);
	void dma_write(u32 offset, u32 data);

	auto irq_callback() { return write_irq.bind(); }

	void set_address_space(address_space *space) { m_pci_memory = space; }

	void map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_write_line write_irq;

	address_space *m_pci_memory;

	u16 m_status;
	u32 m_command_pointer;
	u32 m_intselect;
	u32 m_branchselect;
	u32 m_waitselect;
	u32 m_xfer_word;
	u32 m_opcode, m_address, m_cmdDep, m_statusCount;
	u16 m_currentXfer, m_xferLimit;

	void control_w(u32 data);
	u32 status_r();
	u32 cmdpointer_r();
	void cmdpointer_w(u32 data);
	u32 intselect_r();
	void intselect_w(u32 data);
	u32 branchselect_r();
	void branchselect_w(u32 data);
	u32 waitselect_r();
	void waitselect_w(u32 data);

	void step_program();
	void new_command();
};

// device type definition
DECLARE_DEVICE_TYPE(DBDMA_CHANNEL, dbdma_device)

#endif // MAME_APPLE_DBDMA_H
