// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Intel 82355 Bus Master Interface Controller (BMIC)

**********************************************************************/

#ifndef MAME_MACHINE_I82355_H
#define MAME_MACHINE_I82355_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> i82355_device

class i82355_device : public device_t
{
public:
	// construction/destruction
	i82355_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto eint_callback() { return m_eint_callback.bind(); }
	auto lint_callback() { return m_lint_callback.bind(); }

	// local CPU access
	u8 local_r(offs_t offset);
	void local_w(offs_t offset, u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// helpers
	void set_local_interrupt(u8 flag);
	void clear_local_interrupt(u8 flag);
	void set_system_interrupt(u8 flag);
	void clear_system_interrupt(u8 flag);
	void global_config(u8 flag);
	void identify_board();
	void peek_poke_control(u8 flag);
	void transfer_config(int channel, u8 flag);
	void transfer_strobe(int channel);
	u8 local_register_read(u8 reg);
	void local_register_write(u8 reg, u8 data);

	// callbacks
	devcb_write_line m_eint_callback;
	devcb_write_line m_lint_callback;

	// internal state
	u8 m_local_index;
	u8 m_local_status;
	PAIR m_id;
	u8 m_global_config;
	u8 m_system_interrupt;
	bool m_semaphore_flag[2];
	bool m_semaphore_etest[2];
	bool m_semaphore_ltest[2];
	u8 m_local_doorbell_status;
	u8 m_local_doorbell_enable;
	u8 m_eisa_doorbell_status;
	u8 m_eisa_doorbell_enable;
	u8 m_mailbox[16];
	PAIR m_peek_poke_data;
	PAIR m_peek_poke_address;
	u8 m_peek_poke_control;
	u8 m_peek_poke_status;
	u8 m_io_decode_base[2];
	u8 m_io_decode_control[2];
	u8 m_transfer_config[2];
	u8 m_transfer_status[2];
	PAIR m_base_count[2];
	PAIR m_base_address[2];
	PAIR m_current_count[2];
	PAIR m_current_address[2];
	PAIR16 m_tbi_base[2];
	PAIR16 m_tbi_current[2];
};

// device type declaration
DECLARE_DEVICE_TYPE(I82355, i82355_device)

#endif // MAME_MACHINE_I82355_H
