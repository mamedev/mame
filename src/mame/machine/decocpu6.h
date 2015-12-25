// license:BSD-3-Clause
// copyright-holders:David Haywood


#include "emu.h"
#include "cpu/m6502/m6502.h"

class deco_cpu6_device : public m6502_device {
public:
	deco_cpu6_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	class mi_decrypt : public mi_default_normal {
	public:
		virtual ~mi_decrypt() {}
		virtual UINT8 read_sync(UINT16 adr) override;
	};

	virtual void device_start() override;
	virtual void device_reset() override;

};

static const device_type DECO_CPU6 = &device_creator<deco_cpu6_device>;
