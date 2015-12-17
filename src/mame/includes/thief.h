// license:BSD-3-Clause
// copyright-holders:Victor Trucco, Mike Balfour, Phil Stroffolino
#include "sound/samples.h"
#include "video/tms9927.h"

struct coprocessor_t {
	std::unique_ptr<UINT8[]> context_ram;
	UINT8 bank;
	std::unique_ptr<UINT8[]> image_ram;
	UINT8 param[0x9];
};

class thief_state : public driver_device
{
public:
	thief_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_tms(*this, "tms"),
		m_palette(*this, "palette") { }

	UINT8 *m_videoram;
	UINT8 m_input_select;
	UINT8 m_read_mask;
	UINT8 m_write_mask;
	UINT8 m_video_control;
	coprocessor_t m_coprocessor;
	DECLARE_WRITE8_MEMBER(thief_input_select_w);
	DECLARE_READ8_MEMBER(thief_io_r);
	DECLARE_READ8_MEMBER(thief_context_ram_r);
	DECLARE_WRITE8_MEMBER(thief_context_ram_w);
	DECLARE_WRITE8_MEMBER(thief_context_bank_w);
	DECLARE_WRITE8_MEMBER(thief_video_control_w);
	DECLARE_WRITE8_MEMBER(thief_color_map_w);
	DECLARE_WRITE8_MEMBER(thief_color_plane_w);
	DECLARE_READ8_MEMBER(thief_videoram_r);
	DECLARE_WRITE8_MEMBER(thief_videoram_w);
	DECLARE_WRITE8_MEMBER(thief_blit_w);
	DECLARE_READ8_MEMBER(thief_coprocessor_r);
	DECLARE_WRITE8_MEMBER(thief_coprocessor_w);
	DECLARE_WRITE8_MEMBER(tape_control_w);
	DECLARE_DRIVER_INIT(thief);
	virtual void video_start() override;
	UINT32 screen_update_thief(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(thief_interrupt);
	UINT16 fetch_image_addr( coprocessor_t &thief_coprocessor );
	void tape_set_audio( int track, int bOn );
	void tape_set_motor( int bOn );
	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<tms9927_device> m_tms;
	required_device<palette_device> m_palette;
};
