// license:LGPL-2.1+
// copyright-holders:Angelo Salese
#ifndef MAME_NICHIBUTSU_NB1414M4_H
#define MAME_NICHIBUTSU_NB1414M4_H

#pragma once

#include "tilemap.h"

class nb1414m4_device : public device_t, public device_video_interface
{
public:
	nb1414m4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void exec(uint16_t mcu_cmd, uint8_t *vram, uint16_t &scrollx, uint16_t &scrolly, tilemap_t *tilemap);
	void vblank_trigger();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void dma(uint16_t src, uint16_t dst, uint16_t size, uint8_t condition, uint8_t *vram);
	void fill(uint16_t dst, uint8_t tile, uint8_t pal, uint8_t *vram);
	void insert_coin_msg(uint8_t *vram);
	void credit_msg(uint8_t *vram);
	void kozure_score_msg(uint16_t dst, uint8_t src_base, uint8_t *vram);
	void _0200(uint16_t mcu_cmd, uint8_t *vram);
	void _0600(uint8_t is2p, uint8_t *vram);
	void _0e00(uint16_t mcu_cmd, uint8_t *vram);

	required_region_ptr<uint8_t> m_data;
	uint16_t m_previous_0200_command = 0;
	uint64_t m_previous_0200_command_frame = 0;
	bool m_in_game = false;
};

DECLARE_DEVICE_TYPE(NB1414M4, nb1414m4_device)

#endif // MAME_NICHIBUTSU_NB1414M4_H
