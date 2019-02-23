// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    apple2common.h

    Apple II stuff shared between apple2/apple2e/apple2gs.

*********************************************************************/
#ifndef MAME_MACHINE_APPLE2_COMMON_H
#define MAME_MACHINE_APPLE2_COMMON_H

#pragma once


// ======================> apple2_common_device

class apple2_common_device : public device_t
{
public:
	// construction/destruction
	apple2_common_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	offs_t dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	double m_x_calibration, m_y_calibration;

	double m_joystick_x1_time;
	double m_joystick_y1_time;
	double m_joystick_x2_time;
	double m_joystick_y2_time;

	offs_t com_2byte_op(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname);
	offs_t com_3byte_op(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname);
};


// device type definition
DECLARE_DEVICE_TYPE(APPLE2_COMMON, apple2_common_device)


#endif // MAME_MACHINE_APPLE2_COMMON_H
