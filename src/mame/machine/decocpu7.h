// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_DECOCPU7_H
#define MAME_MACHINE_DECOCPU7_H

#pragma once

#include "cpu/m6502/m6502.h"

class deco_cpu7_device : public m6502_device {
public:
	deco_cpu7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	class mi_decrypt : public mi_default_normal {
	public:
		bool had_written;

		virtual ~mi_decrypt() {}
		virtual uint8_t read_sync(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
	};

	virtual void device_start() override;
	virtual void device_reset() override;

};

DECLARE_DEVICE_TYPE(DECO_CPU7, deco_cpu7_device)

#endif // MAME_MACHINE_DECOCPU7_H
