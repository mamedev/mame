/* Common device stuff for IGS025 / IGS022, should be split into devices for each chip once we know where what part does what */

class igs_025_022_device : public device_t
{
public:
	igs_025_022_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE16_MEMBER( killbld_igs025_prot_w );
	DECLARE_READ16_MEMBER( killbld_igs025_prot_r );
	// use setters instead of making public?
	const UINT8 (*m_kb_source_data)[0xec];
	INT32 m_kb_source_data_offset;
	UINT32 m_kb_game_id;
	UINT16*	m_sharedprotram;

protected:
	virtual void device_config_complete();
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();


	UINT16        m_kb_prot_hold;
	UINT16        m_kb_prot_hilo;
	UINT16        m_kb_prot_hilo_select;

	int           m_kb_cmd;
	int           m_kb_reg;
	int           m_kb_ptr;
	UINT8         m_kb_swap;
	UINT32        m_kb_regs[0x100];



	void killbld_protection_calculate_hilo();
	void killbld_protection_calculate_hold(int y, int z);
	void IGS022_do_dma(UINT16 src, UINT16 dst, UINT16 size, UINT16 mode);
	void IGS022_reset();
	void IGS022_handle_command();


};



extern const device_type IGS025022;
