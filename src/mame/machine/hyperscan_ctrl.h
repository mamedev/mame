// license:BSD-3-Clause
// copyright-holders:Sandro Ronco


#ifndef MAME_MACHINE_HYPERSCAN_CTRL_H
#define MAME_MACHINE_HYPERSCAN_CTRL_H

#pragma once

class hyperscan_ctrl_device : public device_t
{
public:
	hyperscan_ctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(offs_t offset);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

private:
	required_ioport_array<4> m_inputs;
	uint8_t m_input_data[4];
};

DECLARE_DEVICE_TYPE(HYPERSCAN_CTRL, hyperscan_ctrl_device)

#endif // MAME_MACHINE_HYPERSCAN_CTRL_H
