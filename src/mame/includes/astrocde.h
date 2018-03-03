// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Mike Coates, Frank Palazzolo, Aaron Giles
/***************************************************************************

    Bally Astrocade-based hardware

***************************************************************************/

#include "machine/bankdev.h"
#include "machine/gen_latch.h"
#include "sound/astrocde.h"
#include "sound/samples.h"
#include "sound/votrax.h"
#include "screen.h"

#define ASTROCADE_CLOCK     (XTAL(14'318'181)/2)

#define AC_SOUND_PRESENT    (0x01)
#define AC_LIGHTPEN_INTS    (0x02)
#define AC_STARS            (0x04)
#define AC_MONITOR_BW       (0x08)


class astrocde_state : public driver_device
{
public:
	enum
	{
		TIMER_INTERRUPT_OFF,
		TIMER_SCANLINE
	};

	astrocde_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_samples(*this, "samples"),
		m_votrax(*this, "votrax"),
		m_astrocade_sound1(*this, "astrocade1"),
		m_videoram(*this, "videoram"),
		m_protected_ram(*this, "protected_ram"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_bank4000(*this, "bank4000"),
		m_bank8000(*this, "bank8000"),
		m_p1handle(*this, "P1HANDLE"),
		m_p2handle(*this, "P2HANDLE"),
		m_p3handle(*this, "P3HANDLE"),
		m_p4handle(*this, "P4HANDLE"),
		m_keypad0(*this, "KEYPAD0"),
		m_keypad1(*this, "KEYPAD1"),
		m_keypad2(*this, "KEYPAD2"),
		m_keypad3(*this, "KEYPAD3"),
		m_p1_knob(*this, "P1_KNOB"),
		m_p2_knob(*this, "P2_KNOB"),
		m_p3_knob(*this, "P3_KNOB"),
		m_p4_knob(*this, "P4_KNOB"),
		m_trackball(*this, { { "TRACKX2", "TRACKY2", "TRACKX1", "TRACKY1" } }),
		m_joystick(*this, { { "MOVEX", "MOVEY" } }),
		m_interrupt_scanline(0xff)
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	optional_device<samples_device> m_samples;
	optional_device<votrax_sc01_device> m_votrax;
	optional_device<astrocade_device> m_astrocade_sound1;
	optional_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_protected_ram;
	required_device<screen_device> m_screen;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<address_map_bank_device> m_bank4000;
	optional_memory_bank m_bank8000;
	optional_ioport m_p1handle;
	optional_ioport m_p2handle;
	optional_ioport m_p3handle;
	optional_ioport m_p4handle;
	optional_ioport m_keypad0;
	optional_ioport m_keypad1;
	optional_ioport m_keypad2;
	optional_ioport m_keypad3;
	optional_ioport m_p1_knob;
	optional_ioport m_p2_knob;
	optional_ioport m_p3_knob;
	optional_ioport m_p4_knob;
	optional_ioport_array<4> m_trackball;
	optional_ioport_array<2> m_joystick;

	uint8_t m_video_config;
	uint8_t m_sparkle[4];
	char m_totalword[256];
	char *m_totalword_ptr;
	char m_oldword[256];
	int m_plural;
	uint8_t m_port_1_last;
	uint8_t m_port_2_last;
	uint8_t m_ram_write_enable;
	uint8_t m_input_select;
	std::unique_ptr<uint8_t[]> m_sparklestar;
	uint8_t m_interrupt_enabl;
	uint8_t m_interrupt_vector;
	uint8_t m_interrupt_scanline;
	uint8_t m_vertical_feedback;
	uint8_t m_horizontal_feedback;
	emu_timer *m_scanline_timer;
	emu_timer *m_intoff_timer;
	uint8_t m_colors[8];
	uint8_t m_colorsplit;
	uint8_t m_bgdata;
	uint8_t m_vblank;
	uint8_t m_video_mode;
	uint8_t m_funcgen_expand_color[2];
	uint8_t m_funcgen_control;
	uint8_t m_funcgen_expand_count;
	uint8_t m_funcgen_rotate_count;
	uint8_t m_funcgen_rotate_data[4];
	uint8_t m_funcgen_shift_prev_data;
	uint8_t m_funcgen_intercept;
	uint16_t m_pattern_source;
	uint8_t m_pattern_mode;
	uint16_t m_pattern_dest;
	uint8_t m_pattern_skip;
	uint8_t m_pattern_width;
	uint8_t m_pattern_height;
	std::unique_ptr<uint16_t[]> m_profpac_videoram;
	uint16_t m_profpac_palette[16];
	uint8_t m_profpac_colormap[4];
	uint8_t m_profpac_intercept;
	uint8_t m_profpac_vispage;
	uint8_t m_profpac_readpage;
	uint8_t m_profpac_readshift;
	uint8_t m_profpac_writepage;
	uint8_t m_profpac_writemode;
	uint16_t m_profpac_writemask;
	uint8_t m_profpac_vw;
	DECLARE_WRITE8_MEMBER(protected_ram_enable_w);
	DECLARE_READ8_MEMBER(protected_ram_r);
	DECLARE_WRITE8_MEMBER(protected_ram_w);
	DECLARE_WRITE8_MEMBER(seawolf2_lamps_w);
	DECLARE_WRITE8_MEMBER(seawolf2_sound_1_w);
	DECLARE_WRITE8_MEMBER(seawolf2_sound_2_w);
	DECLARE_WRITE8_MEMBER(ebases_trackball_select_w);
	DECLARE_WRITE8_MEMBER(ebases_coin_w);
	DECLARE_READ8_MEMBER(spacezap_io_r);
	DECLARE_READ8_MEMBER(wow_io_r);
	DECLARE_READ8_MEMBER(gorf_io_1_r);
	DECLARE_READ8_MEMBER(gorf_io_2_r);
	DECLARE_READ8_MEMBER(robby_io_r);
	DECLARE_READ8_MEMBER(profpac_io_1_r);
	DECLARE_READ8_MEMBER(profpac_io_2_r);
	DECLARE_WRITE8_MEMBER(profpac_banksw_w);
	DECLARE_WRITE8_MEMBER(demndrgn_banksw_w);
	DECLARE_READ8_MEMBER(demndrgn_io_r);
	DECLARE_WRITE8_MEMBER(demndrgn_sound_w);
	DECLARE_WRITE8_MEMBER(tenpindx_lamp_w);
	DECLARE_WRITE8_MEMBER(tenpindx_counter_w);
	DECLARE_WRITE8_MEMBER(tenpindx_lights_w);
	DECLARE_READ8_MEMBER(astrocade_data_chip_register_r);
	DECLARE_WRITE8_MEMBER(astrocade_data_chip_register_w);
	DECLARE_WRITE8_MEMBER(astrocade_funcgen_w);
	DECLARE_WRITE8_MEMBER(astrocade_pattern_board_w);
	DECLARE_WRITE8_MEMBER(profpac_page_select_w);
	DECLARE_READ8_MEMBER(profpac_intercept_r);
	DECLARE_WRITE8_MEMBER(profpac_screenram_ctrl_w);
	DECLARE_READ8_MEMBER(profpac_videoram_r);
	DECLARE_WRITE8_MEMBER(profpac_videoram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(ebases_trackball_r);
	DECLARE_CUSTOM_INPUT_MEMBER(demndragn_joystick_r);
	DECLARE_INPUT_CHANGED_MEMBER(spacezap_monitor);
	DECLARE_DRIVER_INIT(profpac);
	DECLARE_DRIVER_INIT(spacezap);
	DECLARE_DRIVER_INIT(robby);
	DECLARE_DRIVER_INIT(wow);
	DECLARE_DRIVER_INIT(tenpindx);
	DECLARE_DRIVER_INIT(seawolf2);
	DECLARE_DRIVER_INIT(demndrgn);
	DECLARE_DRIVER_INIT(ebases);
	DECLARE_DRIVER_INIT(gorf);
	DECLARE_DRIVER_INIT(astrocde);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(astrocde);
	DECLARE_VIDEO_START(profpac);
	DECLARE_PALETTE_INIT(profpac);
	uint32_t screen_update_astrocde(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_profpac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_callback);
	inline int mame_vpos_to_astrocade_vpos(int scanline);
	void init_savestate();
	void astrocade_trigger_lightpen(uint8_t vfeedback, uint8_t hfeedback);
	inline void increment_source(uint8_t curwidth, uint8_t *u13ff);
	inline void increment_dest(uint8_t curwidth);
	void execute_blit();
	void init_sparklestar();
	virtual void machine_start() override;

	DECLARE_READ8_MEMBER( votrax_speech_r );
	CUSTOM_INPUT_MEMBER( votrax_speech_status_r );

	void astrocade_base(machine_config &config);
	void astrocade_16color_base(machine_config &config);
	void astrocade_mono_sound(machine_config &config);
	void astrocade_stereo_sound(machine_config &config);
	void spacezap(machine_config &config);
	void gorf(machine_config &config);
	void seawolf2(machine_config &config);
	void profpac(machine_config &config);
	void robby(machine_config &config);
	void ebases(machine_config &config);
	void wow(machine_config &config);
	void tenpindx(machine_config &config);
	void demndrgn(machine_config &config);
	void bank4000_map(address_map &map);
	void demndrgn_map(address_map &map);
	void ebases_map(address_map &map);
	void port_map(address_map &map);
	void port_map_16col_pattern(address_map &map);
	void port_map_16col_pattern_nosound(address_map &map);
	void port_map_16col_pattern_tenpindx(address_map &map);
	void port_map_mono_pattern(address_map &map);
	void port_map_stereo_pattern(address_map &map);
	void profpac_bank4000_map(address_map &map);
	void profpac_map(address_map &map);
	void robby_map(address_map &map);
	void seawolf2_map(address_map &map);
	void spacezap_map(address_map &map);
	void tenpin_sub_io_map(address_map &map);
	void tenpin_sub_map(address_map &map);
	void wow_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
