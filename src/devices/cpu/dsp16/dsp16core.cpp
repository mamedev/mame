// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    WE|AT&T DSP16 series core state

***************************************************************************/

#include "emu.h"
#include "dsp16core.h"


/***********************************************************************
    setup
***********************************************************************/

void dsp16_device_base::core_state::register_save_items(device_t &host)
{
	host.save_item(NAME(xaau_pc));
	host.save_item(NAME(xaau_pt));
	host.save_item(NAME(xaau_pr));
	host.save_item(NAME(xaau_pi));
	host.save_item(NAME(xaau_i));

	host.save_item(NAME(yaau_r));
	host.save_item(NAME(yaau_rb));
	host.save_item(NAME(yaau_re));
	host.save_item(NAME(yaau_j));
	host.save_item(NAME(yaau_k));

	host.save_item(NAME(dau_x));
	host.save_item(NAME(dau_y));
	host.save_item(NAME(dau_p));
	host.save_item(NAME(dau_a));
	host.save_item(NAME(dau_c));
	host.save_item(NAME(dau_auc));
	host.save_item(NAME(dau_psw));
	host.save_item(NAME(dau_temp));
}
