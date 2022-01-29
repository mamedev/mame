// license:BSD-3-Clause
// copyright-holders:Sandro Ronco


#ifndef MAME_MACHINE_HYPERSCAN_CARD_H
#define MAME_MACHINE_HYPERSCAN_CARD_H

#pragma once

#include "imagedev/memcard.h"

class hyperscan_card_device : public device_t,
				public device_memcard_image_interface
{
public:
	hyperscan_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	int read();
	void write(int state);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return "hyperscan_card"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override;

private:
	uint16_t calc_crc(std::vector<uint8_t> const &data);
	void resp_add_byte(uint8_t data, bool add_parity);
	void make_resp(std::vector<uint8_t> const &data, bool add_crc);
	void check_command();

	int                  m_state;
	int                  m_bit_pos;
	uint8_t              m_data;
	std::vector<uint8_t> m_cmd_buf;
	std::vector<uint8_t> m_resp_buf;
	uint8_t              m_resp_idx;
	attotime             m_last_at;
	uint8_t              m_memory[120];
};

DECLARE_DEVICE_TYPE(HYPERSCAN_CARD, hyperscan_card_device)

#endif // MAME_MACHINE_HYPERSCAN_CARD_H
