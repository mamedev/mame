// license:BSD-3-Clause
// copyright-holders:David Shah
/***************************************************************************

    rp2a03_vtscr.h

    RP2A03 with VRT VTxx instruction scrambling

***************************************************************************/

#ifndef MAME_NINTENDO_RP2A03_VTSCR_H
#define MAME_NINTENDO_RP2A03_VTSCR_H

#pragma once

#include "cpu/m6502/rp2a03.h"
#include "cpu/m6502/rp2a03d.h"

class rp2a03_vtscr : public rp2a03_core_device {
public:
	rp2a03_vtscr(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_next_scramble(bool scr);
	void set_scramble(bool scr);

protected:
	class mi_decrypt : public mi_default {
	public:

		bool m_scramble_en = false;
		bool m_next_scramble = false;

		virtual ~mi_decrypt() {}
		virtual uint8_t read_sync(uint16_t adr) override;

		uint8_t descramble(uint8_t op);
		bool toggle_scramble(uint8_t op);

	};

	class disassembler : public rp2a03_disassembler {
	public:
		mi_decrypt *mintf;

		disassembler(mi_decrypt *m);
		virtual ~disassembler() = default;
		virtual u32 interface_flags() const override;
		virtual u8 decrypt8(u8 value, offs_t pc, bool opcode) const override;
	};

	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

DECLARE_DEVICE_TYPE(RP2A03_VTSCR, rp2a03_vtscr)

#endif // MAME_NINTENDO_RP2A03_VTSCR_H
