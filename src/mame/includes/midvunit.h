/*************************************************************************

    Driver for Midway V-Unit games

**************************************************************************/

#include "video/polynew.h"

#define MIDVUNIT_VIDEO_CLOCK	33000000

struct midvunit_object_data
{
	UINT16 *	destbase;
	UINT8 *		texbase;
	UINT16		pixdata;
	UINT8		dither;
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

	void render_flat(INT32 scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
	void render_tex(INT32 scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
	void render_textrans(INT32 scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
	void render_textransmask(INT32 scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
};

class midvunit_state : public driver_device
{
public:
	midvunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram"),
		  m_ram_base(*this, "ram_base"),
		  m_fastram_base(*this, "fastram_base"),
		  m_tms32031_control(*this, "32031_control"),
		  m_midvplus_misc(*this, "midvplus_misc"),
		  m_videoram(*this, "videoram", 32),
		  m_textureram(*this, "textureram") { }

	optional_shared_ptr<UINT32>	m_nvram;
	required_shared_ptr<UINT32> m_ram_base;
	optional_shared_ptr<UINT32> m_fastram_base;
	required_shared_ptr<UINT32> m_tms32031_control;
	optional_shared_ptr<UINT32> m_midvplus_misc;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT32> m_textureram;

	UINT8 m_cmos_protected;
	UINT16 m_control_data;
	UINT8 m_adc_data;
	UINT8 m_adc_shift;
	UINT16 m_last_port0;
	UINT8 m_shifter_state;
	timer_device *m_timer[2];
	double m_timer_rate;
	UINT16 m_bit_index;
	int m_lastval;
	UINT32 *m_generic_speedup;
	UINT16 m_video_regs[16];
	UINT16 m_dma_data[16];
	UINT8 m_dma_data_index;
	UINT16 m_page_control;
	UINT8 m_video_changed;
	emu_timer *m_scanline_timer;
	midvunit_renderer *m_poly;
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
	DECLARE_READ32_MEMBER(midvunit_adc_r);
	DECLARE_WRITE32_MEMBER(midvunit_adc_w);
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
	DECLARE_READ32_MEMBER(generic_speedup_r);
	DECLARE_DRIVER_INIT(crusnu40);
	DECLARE_DRIVER_INIT(crusnu21);
	DECLARE_DRIVER_INIT(crusnwld);
	DECLARE_DRIVER_INIT(wargods);
	DECLARE_DRIVER_INIT(offroadc);
	DECLARE_DRIVER_INIT(crusnusa);
	void init_crusnwld_common(offs_t speedup);
	void init_crusnusa_common(offs_t speedup);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_MACHINE_RESET(midvplus);
	UINT32 screen_update_midvunit(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(adc_ready);
	TIMER_CALLBACK_MEMBER(scanline_timer_cb);
};
