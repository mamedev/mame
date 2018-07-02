// license:BSD-3-Clause
// copyright-holders:Mike Coates, Pierpaolo Prazzoli
/***************************************************************************

    Zaccaria Quasar

****************************************************************************/

#include "includes/cvs.h"

class quasar_state : public cvs_state
{
public:
	quasar_state(const machine_config &mconfig, device_type type, const char *tag)
		: cvs_state(mconfig, type, tag) { }

	void quasar(machine_config &config);

private:
	std::unique_ptr<uint8_t[]>    m_effectram;
	uint8_t      m_effectcontrol;
	uint8_t      m_page;
	uint8_t      m_io_page;
	DECLARE_WRITE8_MEMBER(video_page_select_w);
	DECLARE_WRITE8_MEMBER(io_page_select_w);
	DECLARE_WRITE8_MEMBER(quasar_video_w);
	DECLARE_READ8_MEMBER(quasar_IO_r);
	DECLARE_WRITE8_MEMBER(quasar_bullet_w);
	DECLARE_WRITE8_MEMBER(quasar_sh_command_w);
	DECLARE_READ8_MEMBER(quasar_sh_command_r);
	DECLARE_READ_LINE_MEMBER(audio_t1_r);
	DECLARE_MACHINE_START(quasar);
	DECLARE_MACHINE_RESET(quasar);
	DECLARE_VIDEO_START(quasar);
	DECLARE_PALETTE_INIT(quasar);
	uint32_t screen_update_quasar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(quasar_interrupt);
	void quasar(address_map &map);
	void quasar_data(address_map &map);
	void quasar_io(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);
};
