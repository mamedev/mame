// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Namco C139 - Serial I/F Controller

***************************************************************************/
#ifndef MAME_NAMCO_NAMCO_C139_H
#define MAME_NAMCO_NAMCO_C139_H

#pragma once




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> namco_c139_device

class namco_c139_device : public device_t
{
public:
	// construction/destruction
	namco_c139_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_cb() { return m_irq_cb.bind(); }

	// I/O operations
	void data_map(address_map &map) ATTR_COLD;
	void regs_map(address_map &map) ATTR_COLD;

	uint16_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t reg_r(offs_t offset);
	void reg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void sci_de_hack(uint8_t data);


protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	devcb_write_line m_irq_cb;

private:
	uint16_t m_ram[0x2000];
	uint16_t m_reg[0x0010];

	std::string m_localhost;
	std::string m_localport;
	std::string m_remotehost;
	std::string m_remoteport;

	emu_timer *m_tick_timer;

	class context;
	std::unique_ptr<context> m_context;

	uint8_t m_buffer[0x200];

	uint16_t m_linktimer;
	uint8_t m_linkid;
	uint8_t m_txsize;
	uint8_t m_txblock;
	uint8_t m_reg_f3;

	TIMER_CALLBACK_MEMBER(tick_timer_callback);

	void comm_tick();
	void read_data(unsigned data_size);
	unsigned read_frame(unsigned data_size);
	unsigned find_sync_bit(unsigned tx_offset, unsigned tx_mask);
	void send_data(unsigned data_size);
	void send_frame(unsigned data_size);
};


// device type definition
DECLARE_DEVICE_TYPE(NAMCO_C139, namco_c139_device)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_NAMCO_NAMCO_C139_H
