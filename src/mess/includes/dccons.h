class dc_cons_state : public dc_state
{
public:
	dc_cons_state(const machine_config &mconfig, device_type type, const char *tag)
		: dc_state(mconfig, type, tag)
		{ }

	DECLARE_DRIVER_INIT(dc);
	DECLARE_DRIVER_INIT(dcus);
	DECLARE_DRIVER_INIT(dcjp);
	
	DECLARE_READ64_MEMBER(dcus_idle_skip_r);
	DECLARE_READ64_MEMBER(dcjp_idle_skip_r);
	
	DECLARE_MACHINE_RESET(dc_console);
	DECLARE_READ64_MEMBER(dc_pdtra_r);
	DECLARE_WRITE64_MEMBER(dc_pdtra_w);
	DECLARE_READ64_MEMBER(dc_arm_r);
	DECLARE_WRITE64_MEMBER(dc_arm_w);
	DECLARE_WRITE64_MEMBER(ta_texture_directpath0_w);
	DECLARE_WRITE64_MEMBER(ta_texture_directpath1_w);	
private:	
	UINT64 PDTRA, PCTRA;
	
};
