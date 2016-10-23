// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/* Common device stuff for IGS025 / IGS022, should be split into devices for each chip once we know where what part does what */



// used to connect the 022
typedef device_delegate<void (void)> igs025_execute_external;

#define MCFG_IGS025_SET_EXTERNAL_EXECUTE( _class, _method) \
	igs025_device::set_external_cb(*device, igs025_execute_external(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));

class igs025_device : public device_t
{
public:
	igs025_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t killbld_igs025_prot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	// use setters instead of making public?
	const uint8_t (*m_kb_source_data)[0xec];
	uint32_t m_kb_game_id;
	uint32_t m_kb_region;


	igs025_execute_external m_execute_external;
	static void set_external_cb(device_t &device,igs025_execute_external newcb);

	void olds_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void drgw2_d80000_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void killbld_igs025_prot_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);


protected:
	virtual void device_config_complete() override;
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;


	uint16_t        m_kb_prot_hold;
	uint16_t        m_kb_prot_hilo;
	uint16_t        m_kb_prot_hilo_select;

	int           m_kb_cmd;
	int           m_kb_reg;
	int           m_kb_ptr;
	uint8_t         m_kb_swap;

	void killbld_protection_calculate_hilo();
	void killbld_protection_calculate_hold(int y, int z);

	void no_callback_setup(void);


	uint16_t        m_olds_bs;
	uint16_t        m_kb_cmd3;

};



extern const device_type IGS025;
