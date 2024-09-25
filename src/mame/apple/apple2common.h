// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    apple2common.h

    Apple II stuff shared between apple2/apple2e/apple2gs.

*********************************************************************/
#ifndef MAME_APPLE_APPLE2_COMMON_H
#define MAME_APPLE_APPLE2_COMMON_H

#pragma once

#include "cpu/g65816/g65816.h"

// ======================> apple2_common_device

class apple2_common_device : public device_t
{
public:
	// construction/destruction
	apple2_common_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_GS_cputag(T &&tag) { m_GScpu.set_tag(std::forward<T>(tag)); }

	offs_t dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);
	offs_t dasm_override_GS(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_validity_check(validity_checker &valid) const override;

private:
	optional_device<g65816_device> m_GScpu;

	double m_x_calibration = 0, m_y_calibration = 0;
	double m_joystick_x1_time = 0;
	double m_joystick_y1_time = 0;
	double m_joystick_x2_time = 0;
	double m_joystick_y2_time = 0;

	offs_t com_2byte_op(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname);
	offs_t com_3byte_op(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname);
	offs_t com_long_op(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname);
};


// device type definition
DECLARE_DEVICE_TYPE(APPLE2_COMMON, apple2_common_device)


#endif // MAME_APPLE_APPLE2_COMMON_H
