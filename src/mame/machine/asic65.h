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
	asic65_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// (static) configuration helpers
	static void set_type(device_t &device, int type) { downcast<asic65_device &>(device).m_asic65_type = type; }

	void reset_line(int state);
	DECLARE_WRITE16_MEMBER( data_w );
	DECLARE_READ16_MEMBER( read );
	DECLARE_READ16_MEMBER( io_r );

	WRITE16_MEMBER( m68k_w );
	READ16_MEMBER( m68k_r );
	WRITE16_MEMBER( stat_w );
	READ16_MEMBER( stat_r );
	READ16_MEMBER( get_bio );

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
	UINT8   m_asic65_type;
	int     m_command;
	UINT16  m_param[32];
	UINT16  m_yorigin;
	UINT8   m_param_index;
	UINT8   m_result_index;
	UINT8   m_reset_state;
	UINT8   m_last_bank;

	/* ROM-based interface states */
	required_device<cpu_device> m_ourcpu;
	UINT8   m_tfull;
	UINT8   m_68full;
	UINT8   m_cmd;
	UINT8   m_xflg;
	UINT16  m_68data;
	UINT16  m_tdata;

	FILE * m_log;
};

extern const device_type ASIC65;

#define MCFG_ASIC65_ADD(_tag, _type) \
	MCFG_DEVICE_ADD(_tag, ASIC65, 0) \
	asic65_device::set_type(*device, _type);
