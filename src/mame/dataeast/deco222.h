// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_DATAEAST_DECO222
#define MAME_DATAEAST_DECO222

#pragma once

#include "cpu/m6502/m6502d.h"
#include "cpu/m6502/m6502.h"

class deco_222_device : public m6502_device {
public:
	deco_222_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	class mi_decrypt : public mi_default {
	public:
		bool had_written;

		virtual ~mi_decrypt() {}
		virtual uint8_t read_sync(uint16_t adr) override;
	};

	class disassembler : public m6502_disassembler {
	public:
		disassembler() = default;
		virtual ~disassembler() = default;
		virtual u32 interface_flags() const override;
		virtual u8 decrypt8(u8 value, offs_t pc, bool opcode) const override;
	};

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};



class deco_c10707_device : public m6502_device {
public:
	deco_c10707_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	class mi_decrypt : public mi_default {
	public:
		bool had_written;

		virtual ~mi_decrypt() {}
		virtual uint8_t read_sync(uint16_t adr) override;
	};


	class disassembler : public m6502_disassembler {
	public:
		disassembler() = default;
		virtual ~disassembler() = default;
		virtual u32 interface_flags() const override;
		virtual u8 decrypt8(u8 value, offs_t pc, bool opcode) const override;
	};

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


DECLARE_DEVICE_TYPE(DECO_222, deco_222_device)
DECLARE_DEVICE_TYPE(DECO_C10707, deco_c10707_device)

#endif // MAME_DATAEAST_DECO222
