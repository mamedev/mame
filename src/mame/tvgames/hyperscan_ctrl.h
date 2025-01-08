// license:BSD-3-Clause
// copyright-holders:Sandro Ronco


#ifndef MAME_TVGAMES_HYPERSCAN_CTRL_H
#define MAME_TVGAMES_HYPERSCAN_CTRL_H

#pragma once

class hyperscan_ctrl_device : public device_t
{
public:
	hyperscan_ctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(offs_t offset);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport_array<4> m_inputs;
	uint8_t m_input_data[4];
};

DECLARE_DEVICE_TYPE(HYPERSCAN_CTRL, hyperscan_ctrl_device)

#endif // MAME_TVGAMES_HYPERSCAN_CTRL_H
