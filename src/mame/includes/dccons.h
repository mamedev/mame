// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
#include "imagedev/chd_cd.h"
#include "machine/gdrom.h"
#include "machine/ataintf.h"
#include "machine/intelfsh.h"

class dc_cons_state : public dc_state
{
public:
	dc_cons_state(const machine_config &mconfig, device_type type, const char *tag)
		: dc_state(mconfig, type, tag),
		m_ata(*this, "ata")
//        m_dcflash(*this, "dcflash")
		{ }

	required_device<ata_interface_device> m_ata;
//  required_device<macronix_29lv160tmc_device> m_dcflash;

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
	DECLARE_WRITE_LINE_MEMBER(sh4_aica_irq);
	DECLARE_WRITE_LINE_MEMBER(ata_interrupt);

	TIMER_CALLBACK_MEMBER( atapi_xfer_end );

	void dreamcast_atapi_init();
	DECLARE_READ32_MEMBER( dc_mess_g1_ctrl_r );
	DECLARE_WRITE32_MEMBER( dc_mess_g1_ctrl_w );
//  DECLARE_READ8_MEMBER( dc_flash_r );
//  DECLARE_WRITE8_MEMBER( dc_flash_w );

private:
	UINT64 PDTRA, PCTRA;
	emu_timer *atapi_timer;
	int atapi_xferlen, atapi_xferbase;
};
