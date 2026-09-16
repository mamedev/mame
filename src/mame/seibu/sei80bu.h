// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, R. Belmont
/***************************************************************************

    SEI80BU Z80 ROM decryptor

***************************************************************************/
#ifndef MAME_SEIBU_SEI80BU_H
#define MAME_SEIBU_SEI80BU_H

#pragma once

#include "dirom.h"


class sei80bu_device : public device_t, public device_rom_interface<16>
{
public:
	sei80bu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 data_r(offs_t offset);
	u8 opcode_r(offs_t offset);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD { }
};

DECLARE_DEVICE_TYPE(SEI80BU, sei80bu_device)

#endif // MAME_SEIBU_SEI80BU_H
