// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/* Common device stuff for IGS025 / IGS022, should be split into devices for each chip once we know where what part does what */



// used to connect the 022
typedef device_delegate<void (void)> igs025_execute_external;

#define MCFG_IGS025_SET_EXTERNAL_EXECUTE( _class, _method) \
	igs025_device::set_external_cb(*device, igs025_execute_external(&_class::_method, #_class "::" #_method, NULL, (_class *)0));

class igs025_device : public device_t
{
public:
	igs025_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER( killbld_igs025_prot_r );
	// use setters instead of making public?
	const UINT8 (*m_kb_source_data)[0xec];
	UINT32 m_kb_game_id;
	UINT32 m_kb_region;


	igs025_execute_external m_execute_external;
	static void set_external_cb(device_t &device,igs025_execute_external newcb);

	DECLARE_WRITE16_MEMBER( olds_w );
	DECLARE_WRITE16_MEMBER( drgw2_d80000_protection_w );
	DECLARE_WRITE16_MEMBER( killbld_igs025_prot_w);


protected:
	virtual void device_config_complete() override;
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;


	UINT16        m_kb_prot_hold;
	UINT16        m_kb_prot_hilo;
	UINT16        m_kb_prot_hilo_select;

	int           m_kb_cmd;
	int           m_kb_reg;
	int           m_kb_ptr;
	UINT8         m_kb_swap;

	void killbld_protection_calculate_hilo();
	void killbld_protection_calculate_hold(int y, int z);

	void no_callback_setup(void);


	UINT16        m_olds_bs;
	UINT16        m_kb_cmd3;

};



extern const device_type IGS025;
