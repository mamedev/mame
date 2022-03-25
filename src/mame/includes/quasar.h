// license:BSD-3-Clause
// copyright-holders:Mike Coates, Pierpaolo Prazzoli
/***************************************************************************

    Zaccaria Quasar

****************************************************************************/
#ifndef MAME_INCLUDES_QUASAR_H
#define MAME_INCLUDES_QUASAR_H

#pragma once

#include "includes/cvs.h"

class quasar_state : public cvs_state
{
public:
	quasar_state(const machine_config &mconfig, device_type type, const char *tag)
		: cvs_state(mconfig, type, tag)
	{ }

	void quasar(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	void video_page_select_w(offs_t offset, uint8_t data);
	void io_page_select_w(offs_t offset, uint8_t data);
	void quasar_video_w(offs_t offset, uint8_t data);
	uint8_t quasar_IO_r();
	void quasar_bullet_w(offs_t offset, uint8_t data);
	void quasar_sh_command_w(uint8_t data);
	uint8_t quasar_sh_command_r();
	DECLARE_READ_LINE_MEMBER(audio_t1_r);
	void quasar_palette(palette_device &palette) const;
	uint32_t screen_update_quasar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(quasar_interrupt);

	void quasar_program(address_map &map);
	void quasar_data(address_map &map);
	void quasar_io(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);

	std::unique_ptr<uint8_t[]>    m_effectram;
	uint8_t      m_effectcontrol = 0;
	uint8_t      m_page = 0;
	uint8_t      m_io_page = 0;
};

#endif // MAME_INCLUDES_QUASAR_H
