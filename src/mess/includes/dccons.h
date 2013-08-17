#include "imagedev/chd_cd.h"
#include "machine/gdrom.h"

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
	DECLARE_WRITE_LINE_MEMBER(aica_irq);
	void gdrom_raise_irq();
	void gdrom_set_status(UINT8 flag,bool state);
	void gdrom_set_error(UINT8 flag,bool state);

	TIMER_CALLBACK_MEMBER( atapi_xfer_end );
	TIMER_CALLBACK_MEMBER( atapi_cmd_exec );
	UINT8 cur_atapi_cmd;
	void atapi_cmd_nop();
	void atapi_cmd_packet();
	void atapi_cmd_identify_packet();
	void atapi_cmd_set_features();

	READ32_MEMBER( atapi_r );
	WRITE32_MEMBER( atapi_w );
	void dreamcast_atapi_init();
	void dreamcast_atapi_reset();
	inline int decode_reg32_64(UINT32 offset, UINT64 mem_mask, UINT64 *shift);
	READ64_MEMBER( dc_mess_gdrom_r );
	WRITE64_MEMBER( dc_mess_gdrom_w );
	READ32_MEMBER( dc_mess_g1_ctrl_r );
	WRITE32_MEMBER( dc_mess_g1_ctrl_w );

private:
	UINT64 PDTRA, PCTRA;

	UINT8 *atapi_regs;
	emu_timer *atapi_timer,*atapi_cmd_timer;
	gdrom_device *gdrom;
	UINT8 *atapi_data;
	int atapi_data_ptr, atapi_data_len, atapi_xferlen, atapi_xferbase, atapi_cdata_wait, atapi_xfermod;
	UINT8 xfer_mode;
};
