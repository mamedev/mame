// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway V-Unit games

**************************************************************************/

#include "video/poly.h"
#include "audio/dcs.h"
#include "machine/midwayic.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "machine/adc0844.h"
#include "screen.h"

#define MIDVUNIT_VIDEO_CLOCK    33000000

struct midvunit_object_data
{
	uint16_t *    destbase;
	uint8_t *     texbase;
	uint16_t      pixdata;
	uint8_t       dither;
};

class midvunit_state;

class midvunit_renderer : public poly_manager<float, midvunit_object_data, 2, 4000>
{
public:
	midvunit_renderer(midvunit_state &state);
	void process_dma_queue();
	void make_vertices_inclusive(vertex_t *vert);

private:
	midvunit_state &m_state;

	void render_flat(int32_t scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
	void render_tex(int32_t scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
	void render_textrans(int32_t scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
	void render_textransmask(int32_t scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
};

class midvunit_state : public driver_device
{
public:
	enum
	{
		TIMER_SCANLINE
	};

	midvunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_nvram(*this, "nvram"),
			m_ram_base(*this, "ram_base"),
			m_fastram_base(*this, "fastram_base"),
			m_tms32031_control(*this, "32031_control"),
			m_midvplus_misc(*this, "midvplus_misc"),
			m_videoram(*this, "videoram", 32),
			m_textureram(*this, "textureram") ,
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_adc(*this, "adc"),
		m_midway_serial_pic(*this, "serial_pic"),
		m_midway_serial_pic2(*this, "serial_pic2"),
		m_midway_ioasic(*this, "ioasic"),
		m_dcs(*this, "dcs"),
		m_generic_paletteram_32(*this, "paletteram") { }

	optional_shared_ptr<uint32_t> m_nvram;
	required_shared_ptr<uint32_t> m_ram_base;
	optional_shared_ptr<uint32_t> m_fastram_base;
	required_shared_ptr<uint32_t> m_tms32031_control;
	optional_shared_ptr<uint32_t> m_midvplus_misc;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint32_t> m_textureram;

	uint8_t m_cmos_protected;
	uint16_t m_control_data;
	uint8_t m_adc_shift;
	uint16_t m_last_port0;
	uint8_t m_shifter_state;
	timer_device *m_timer[2];
	double m_timer_rate;
	uint16_t m_bit_index;
	int m_lastval;
	uint32_t *m_generic_speedup;
	uint16_t m_video_regs[16];
	uint16_t m_dma_data[16];
	uint8_t m_dma_data_index;
	uint16_t m_page_control;
	uint8_t m_video_changed;
	emu_timer *m_scanline_timer;
	std::unique_ptr<midvunit_renderer> m_poly;
	uint8_t m_galil_input_index;
	uint8_t m_galil_input_length;
	const char *m_galil_input;
	uint8_t m_galil_output_index;
	char m_galil_output[450];
	uint32_t m_wheel_board_output;
	uint32_t m_wheel_board_last;
	uint32_t m_wheel_board_u8_latch;
	DECLARE_WRITE32_MEMBER(midvunit_dma_queue_w);
	DECLARE_READ32_MEMBER(midvunit_dma_queue_entries_r);
	DECLARE_READ32_MEMBER(midvunit_dma_trigger_r);
	DECLARE_WRITE32_MEMBER(midvunit_page_control_w);
	DECLARE_READ32_MEMBER(midvunit_page_control_r);
	DECLARE_WRITE32_MEMBER(midvunit_video_control_w);
	DECLARE_READ32_MEMBER(midvunit_scanline_r);
	DECLARE_WRITE32_MEMBER(midvunit_videoram_w);
	DECLARE_READ32_MEMBER(midvunit_videoram_r);
	DECLARE_WRITE32_MEMBER(midvunit_paletteram_w);
	DECLARE_WRITE32_MEMBER(midvunit_textureram_w);
	DECLARE_READ32_MEMBER(midvunit_textureram_r);
	DECLARE_READ32_MEMBER(port0_r);
	DECLARE_READ32_MEMBER(adc_r);
	DECLARE_WRITE32_MEMBER(adc_w);
	DECLARE_WRITE32_MEMBER(midvunit_cmos_protect_w);
	DECLARE_WRITE32_MEMBER(midvunit_cmos_w);
	DECLARE_READ32_MEMBER(midvunit_cmos_r);
	DECLARE_WRITE32_MEMBER(midvunit_control_w);
	DECLARE_WRITE32_MEMBER(crusnwld_control_w);
	DECLARE_WRITE32_MEMBER(midvunit_sound_w);
	DECLARE_READ32_MEMBER(tms32031_control_r);
	DECLARE_WRITE32_MEMBER(tms32031_control_w);
	DECLARE_READ32_MEMBER(crusnwld_serial_status_r);
	DECLARE_READ32_MEMBER(crusnwld_serial_data_r);
	DECLARE_WRITE32_MEMBER(crusnwld_serial_data_w);
	DECLARE_READ32_MEMBER(bit_data_r);
	DECLARE_WRITE32_MEMBER(bit_reset_w);
	DECLARE_READ32_MEMBER(offroadc_serial_status_r);
	DECLARE_READ32_MEMBER(offroadc_serial_data_r);
	DECLARE_WRITE32_MEMBER(offroadc_serial_data_w);
	DECLARE_READ32_MEMBER(midvplus_misc_r);
	DECLARE_WRITE32_MEMBER(midvplus_misc_w);
	DECLARE_WRITE8_MEMBER(midvplus_xf1_w);
	DECLARE_READ32_MEMBER(generic_speedup_r);
	DECLARE_READ32_MEMBER(midvunit_wheel_board_r);
	DECLARE_WRITE32_MEMBER(midvunit_wheel_board_w);
	DECLARE_DRIVER_INIT(crusnu40);
	DECLARE_DRIVER_INIT(crusnu21);
	DECLARE_DRIVER_INIT(crusnwld);
	DECLARE_DRIVER_INIT(wargods);
	DECLARE_DRIVER_INIT(offroadc);
	DECLARE_DRIVER_INIT(crusnusa);
	void set_input(const char *s);
	void init_crusnwld_common(offs_t speedup);
	void init_crusnusa_common(offs_t speedup);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_MACHINE_RESET(midvplus);
	uint32_t screen_update_midvunit(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_timer_cb);
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<adc0844_device> m_adc;
	optional_device<midway_serial_pic_device> m_midway_serial_pic;
	optional_device<midway_serial_pic2_device> m_midway_serial_pic2;
	optional_device<midway_ioasic_device> m_midway_ioasic;
	required_device<dcs_audio_device> m_dcs;
	required_shared_ptr<uint32_t> m_generic_paletteram_32;
	void postload();

	void midvcommon(machine_config &config);
	void crusnwld(machine_config &config);
	void midvplus(machine_config &config);
	void offroadc(machine_config &config);
	void midvunit(machine_config &config);
	void midvplus_map(address_map &map);
	void midvunit_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
