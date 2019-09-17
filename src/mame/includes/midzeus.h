// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/

#define MIDZEUS_VIDEO_CLOCK     XTAL(66'666'700)

#include "machine/timekpr.h"
#include "emupal.h"
#include "screen.h"
#include "machine/midwayic.h"
#include "video/poly.h"

/*************************************
 *
 *  Type definitions
 *
 *************************************/

struct mz_poly_extra_data
{
	const void *    palbase;
	const void *    texbase;
	uint16_t          solidcolor;
	uint16_t          voffset;
	int16_t           zoffset;
	uint16_t          transcolor;
	uint16_t          texwidth;
	uint16_t          color;
	uint32_t          alpha;
	uint32_t          ctrl_word;
	bool            blend_enable;
	bool            depth_test_enable;
	bool            depth_write_enable;
	uint32_t          blend;
	uint8_t           (*get_texel)(const void *, int, int, int);
};


class midzeus_state;

class midzeus_renderer : public poly_manager<float, mz_poly_extra_data, 4, 10000>
{
public:
	midzeus_renderer(midzeus_state &state);

	void render_poly(int32_t scanline, const extent_t& extent, const mz_poly_extra_data& object, int threadid);
	void render_poly_solid_fixedz(int32_t scanline, const extent_t& extent, const mz_poly_extra_data& object, int threadid);

	void zeus_draw_quad(int long_fmt, const uint32_t *databuffer, uint32_t texdata, bool logit);
	void zeus_draw_debug_quad(const rectangle& rect, const vertex_t* vert);

private:
	midzeus_state& m_state;
};

typedef midzeus_renderer::vertex_t poly_vertex;


class midzeus_state : public driver_device
{
	friend class midzeus_renderer;

public:
	midzeus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_nvram(*this, "nvram"),
		m_ram_base(*this, "ram_base"),
		m_firewire(*this, "firewire"),
		m_tms32031_control(*this, "tms32031_ctl"),
		m_zeusbase(*this, "zeusbase"),
		m_m48t35(*this, "m48t35"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ioasic(*this, "ioasic"),
		m_io_analog(*this, "ANALOG%u", 0U),
		m_io_gun_x(*this, "GUNX%u", 1U),
		m_io_gun_y(*this, "GUNY%u", 1U),
		m_io_trackx(*this, "TRACKX1"),
		m_io_tracky(*this, "TRACKY1"),
		m_io_49way_x(*this, "49WAYX"),
		m_io_49way_y(*this, "49WAYY"),
		m_io_keypad(*this, "KEYPAD"),
		m_digits(*this, "digit%u", 0U)
	{ }

	//static constexpr XTAL CPU_CLOCK = XTAL(60'000'000);
	static constexpr int BEAM_DY = 3;
	static constexpr int BEAM_DX = 3;
	static constexpr int BEAM_XOFFS = 40; // table in the code indicates an offset of 20 with a beam height of 7

	required_shared_ptr<uint32_t> m_nvram;
	required_shared_ptr<uint32_t> m_ram_base;
	optional_shared_ptr<uint32_t> m_firewire;
	required_shared_ptr<uint32_t> m_tms32031_control;
	optional_shared_ptr<uint32_t> m_zeusbase;

	DECLARE_WRITE32_MEMBER(cmos_w);
	DECLARE_READ32_MEMBER(cmos_r);
	DECLARE_WRITE32_MEMBER(cmos_protect_w);
	DECLARE_READ32_MEMBER(zpram_r);
	DECLARE_WRITE32_MEMBER(zpram_w);
	DECLARE_READ32_MEMBER(disk_asic_r);
	DECLARE_WRITE32_MEMBER(disk_asic_w);
	DECLARE_READ32_MEMBER(disk_asic_jr_r);
	DECLARE_WRITE32_MEMBER(disk_asic_jr_w);
	DECLARE_READ32_MEMBER(firewire_r);
	DECLARE_WRITE32_MEMBER(firewire_w);
	DECLARE_READ32_MEMBER(tms32031_control_r);
	DECLARE_WRITE32_MEMBER(tms32031_control_w);
	DECLARE_WRITE32_MEMBER(keypad_select_w);
	DECLARE_READ32_MEMBER(analog_r);
	DECLARE_WRITE32_MEMBER(analog_w);
	DECLARE_WRITE32_MEMBER(invasn_gun_w);
	DECLARE_READ32_MEMBER(invasn_gun_r);
	DECLARE_READ32_MEMBER(zeus_r);
	DECLARE_WRITE32_MEMBER(zeus_w);
	DECLARE_CUSTOM_INPUT_MEMBER(custom_49way_r);
	DECLARE_CUSTOM_INPUT_MEMBER(keypad_r);
	DECLARE_READ32_MEMBER(grid_keypad_r);
	DECLARE_READ32_MEMBER(trackball_r);
	void init_invasn();
	void init_mk4();
	DECLARE_MACHINE_START(midzeus);
	DECLARE_MACHINE_RESET(midzeus);
	DECLARE_VIDEO_START(midzeus);
	uint32_t screen_update_midzeus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(display_irq);
	TIMER_CALLBACK_MEMBER(display_irq_off);
	TIMER_CALLBACK_MEMBER(invasn_gun_callback);
	void midzeus(machine_config &config);
	void invasn(machine_config &config);
	void mk4(machine_config &config);
	void zeus_map(address_map &map);
protected:
	optional_device<timekeeper_device> m_m48t35;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	required_device<midway_ioasic_device> m_ioasic;
	optional_ioport_array<4> m_io_analog;
	optional_ioport_array<2> m_io_gun_x;
	optional_ioport_array<2> m_io_gun_y;
	optional_ioport m_io_trackx;
	optional_ioport m_io_tracky;
	optional_ioport m_io_49way_x;
	optional_ioport m_io_49way_y;
	optional_ioport m_io_keypad;
	output_finder<7> m_digits;
	emu_timer *m_display_irq_off_timer;
	uint8_t            crusnexo_leds_select;
	uint32_t           disk_asic[0x10];
	uint32_t           disk_asic_jr[0x10];

	uint8_t cmos_protected;

	emu_timer *timer[2];
private:
	uint32_t           gun_control;
	uint8_t            gun_irq_state;
	emu_timer *        gun_timer[2];
	int32_t            gun_x[2], gun_y[2];
	uint8_t            keypad_select;
	void exit_handler();
	void zeus_pointer_w(uint32_t which, uint32_t data, bool logit);
	void zeus_register16_w(offs_t offset, uint16_t data, bool logit);
	void zeus_register32_w(offs_t offset, uint32_t data, bool logit);
	void zeus_register_update(offs_t offset);
	int zeus_fifo_process(const uint32_t *data, int numwords);
	void zeus_draw_model(uint32_t texdata, bool logit);

	void log_fifo_command(const uint32_t *data, int numwords, const char *suffix);
	void log_waveram(uint32_t length_and_base);
	void update_gun_irq();

	void *waveram0_ptr_from_block_addr(uint32_t addr);
	void *waveram0_ptr_from_expanded_addr(uint32_t addr);
	void *waveram1_ptr_from_expanded_addr(uint32_t addr);
	void *waveram0_ptr_from_texture_addr(uint32_t addr, int width);
	void waveram_plot_depth(int y, int x, uint16_t color, uint16_t depth);
	void waveram_plot(int y, int x, uint16_t color);
	void waveram_plot_check_depth(int y, int x, uint16_t color, uint16_t depth);
	void waveram_plot_check_depth_nowrite(int y, int x, uint16_t color, uint16_t depth);

	std::unique_ptr<midzeus_renderer> m_poly;
	uint8_t m_log_fifo;

	uint32_t m_zeus_fifo[20];
	uint8_t m_zeus_fifo_words;
	int16_t m_zeus_matrix[3][3];
	int32_t m_zeus_point[3];
	int16_t m_zeus_light[3];
	void *m_zeus_renderbase;
	uint32_t m_zeus_palbase;
	uint32_t m_zeus_unkbase;
	int m_zeus_enable_logging;
	uint32_t m_zeus_objdata;
	rectangle m_zeus_cliprect;

	std::unique_ptr<uint32_t[]> m_waveram[2];
	int m_yoffs;
	int m_texel_width;
	int m_is_mk4b;
};
