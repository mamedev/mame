// license:BSD-3-Clause
// copyright-holders:Howie Cohen, Yochizo
// thanks-to:Richard Bush
#include "includes/seta.h"

class taitox_state : public seta_state
{
public:
	taitox_state(const machine_config &mconfig, device_type type, std::string tag)
		: seta_state(mconfig, type, tag) { }

	DECLARE_READ16_MEMBER(superman_dsw_input_r);
	DECLARE_READ16_MEMBER(daisenpu_input_r);
	DECLARE_WRITE16_MEMBER(daisenpu_input_w);
	DECLARE_WRITE16_MEMBER(kyustrkr_input_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_DRIVER_INIT(kyustrkr);
	DECLARE_MACHINE_START(taitox);
	DECLARE_MACHINE_START(superman);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);

	// superman c-chip
	UINT16 m_current_bank;
	UINT8 m_cc_port;
	DECLARE_READ16_MEMBER( cchip1_ctrl_r );
	DECLARE_READ16_MEMBER( cchip1_ram_r );
	DECLARE_WRITE16_MEMBER( cchip1_ctrl_w );
	DECLARE_WRITE16_MEMBER( cchip1_bank_w );
	DECLARE_WRITE16_MEMBER( cchip1_ram_w );
};
