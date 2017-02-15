// license:BSD-3-Clause
// copyright-holders:David Haywood


#include "cpu/m6502/m6502.h"

class deco_cpu6_device : public m6502_device {
public:
	deco_cpu6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	class mi_decrypt : public mi_default_normal {
	public:
		virtual ~mi_decrypt() {}
		virtual uint8_t read_sync(uint16_t adr) override;
	};

	virtual void device_start() override;
	virtual void device_reset() override;

};

static const device_type DECO_CPU6 = &device_creator<deco_cpu6_device>;
