// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************
 *
 *  Implementation of ASIC65
 *
 *************************************/

	#include "cpu/tms32010/tms32010.h"

	enum {
	ASIC65_STANDARD,
	ASIC65_STEELTAL,
	ASIC65_GUARDIANS,
	ASIC65_ROMBASED
};

class asic65_device : public device_t
{
public:
	asic65_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// (static) configuration helpers
	static void set_type(device_t &device, int type) { downcast<asic65_device &>(device).m_asic65_type = type; }

	void reset_line(int state);
	void data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void m68k_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t m68k_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void stat_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t stat_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	int get_bio();

	enum
	{
		TIMER_M68K_ASIC65_DEFERRED_W
	};

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	uint8_t   m_asic65_type;
	int     m_command;
	uint16_t  m_param[32];
	uint16_t  m_yorigin;
	uint8_t   m_param_index;
	uint8_t   m_result_index;
	uint8_t   m_reset_state;
	uint8_t   m_last_bank;

	/* ROM-based interface states */
	required_device<cpu_device> m_ourcpu;
	uint8_t   m_tfull;
	uint8_t   m_68full;
	uint8_t   m_cmd;
	uint8_t   m_xflg;
	uint16_t  m_68data;
	uint16_t  m_tdata;

	FILE * m_log;
};

extern const device_type ASIC65;

#define MCFG_ASIC65_ADD(_tag, _type) \
	MCFG_DEVICE_ADD(_tag, ASIC65, 0) \
	asic65_device::set_type(*device, _type);
