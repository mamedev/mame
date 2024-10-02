// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Incredible Technologies/Strata system
    (8-bit blitter variant)

**************************************************************************/

#include "machine/gen_latch.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/nvram.h"
#include "video/tlc34076.h"
#include "video/tms34061.h"
#include "emupal.h"
#include "screen.h"


class itech8_state : public driver_device
{
public:
	itech8_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_nvram(*this, "nvram"),
		m_soundlatch(*this, "soundlatch"),
		m_tms34061(*this, "tms34061"),
		m_tlc34076(*this, "tlc34076"),
		m_screen(*this, "screen"),
		m_ticket(*this, "ticket"),
		m_grom(*this, "grom"),
		m_mainbank(*this, "mainbank"),
		m_fixed(*this, "fixed"),
		m_an(*this, { { "AN_C", "AN_D", "AN_E", "AN_F" } }),
		m_p1(*this, "P1"),
		m_p2(*this, "P2"),
		m_visarea(0, 0, 0, 0),
		m_bankxor(0)
	{}

	void rimrockn(machine_config &config);
	void gtg2(machine_config &config);
	void slikshot_lo_noz80(machine_config &config);
	void ninclown(machine_config &config);
	void stratab_hi(machine_config &config);
	void hstennis_lo(machine_config &config);
	void wfortune(machine_config &config);
	void stratab_lo(machine_config &config);
	void hstennis_hi(machine_config &config);

	void init_invbank();
	void init_peggle();
	void init_neckneck();
	void init_arligntn();
	void init_hstennis();

	int special_r();
	ioport_value gtg_mux();

protected:
	static constexpr u32 VRAM_SIZE = 0x40000;
	static constexpr u32 VRAM_MASK = VRAM_SIZE - 1;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<nvram_device> m_nvram;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<tms34061_device> m_tms34061;
	optional_device<tlc34076_device> m_tlc34076;
	required_device<screen_device> m_screen;
	required_device<ticket_dispenser_device> m_ticket;
	required_region_ptr<u8> m_grom;
	optional_memory_bank m_mainbank;
	optional_memory_bank m_fixed;
	optional_ioport_array<4> m_an;
	optional_ioport m_p1;
	optional_ioport m_p2;

	rectangle m_visarea{};

	u8 m_grom_bank = 0;
	u8 m_blitter_int = 0;
	u8 m_tms34061_int = 0;
	u8 m_periodic_int = 0;
	u8 m_pia_porta_data = 0;
	u8 m_pia_portb_data = 0;
	u8 m_blitter_data[16]{};
	u8 m_blit_in_progress = 0;
	u8 m_page_select = 0;
	offs_t m_fetch_offset = 0;
	u8 m_fetch_rle_count = 0;
	u8 m_fetch_rle_value = 0;
	u8 m_fetch_rle_literal = 0;
	emu_timer *m_irq_off_timer = nullptr;
	emu_timer *m_blitter_done_timer = nullptr;
	int m_bankxor = 0;

	// common
	void generate_tms34061_interrupt(int state);
	void nmi_ack_w(u8 data);
	void blitter_bank_w(offs_t offset, u8 data);
	void rimrockn_bank_w(u8 data);
	void pia_portb_out(u8 data);
	void gtg2_sound_data_w(u8 data);
	void grom_bank_w(u8 data);
	void palette_w(offs_t offset, u8 data);
	void page_w(u8 data);
	u8 blitter_r(offs_t offset);
	void blitter_w(offs_t offset, u8 data);
	void tms34061_w(offs_t offset, u8 data);
	u8 tms34061_r(offs_t offset);
	void pia_porta_out(u8 data);
	void ym2203_portb_out(u8 data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	u32 screen_update_2layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_2page(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_2page_large(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void generate_nmi(int state);
	void ninclown_irq(int state);
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_CALLBACK_MEMBER(blitter_done);

	inline u8 fetch_next_raw();
	inline void consume_raw(int count);
	inline u8 fetch_next_rle();
	inline void consume_rle(int count);
	void perform_blit();
	void update_interrupts(int periodic, int tms34061, int blitter);

	// ninja clowns
	u16 rom_constant_r(offs_t offset);
	u8 ninclown_palette_r(offs_t offset);
	void ninclown_palette_w(offs_t offset, u8 data);

	void itech8_sound_ym2203(machine_config &config);
	void itech8_sound_ym2608b(machine_config &config);
	void itech8_sound_ym3812(machine_config &config);
	void itech8_sound_ym3812_external(machine_config &config);
	void itech8_core_devices(machine_config &config);
	void itech8_core_lo(machine_config &config);
	void itech8_core_hi(machine_config &config);
	void common_hi_map(address_map &map) ATTR_COLD;
	void common_lo_map(address_map &map) ATTR_COLD;
	void gtg2_map(address_map &map) ATTR_COLD;
	void ninclown_map(address_map &map) ATTR_COLD;
	void rimrockn_map(address_map &map) ATTR_COLD;
	void sound2203_map(address_map &map) ATTR_COLD;
	void sound2608b_map(address_map &map) ATTR_COLD;
	void sound3812_external_map(address_map &map) ATTR_COLD;
	void sound3812_map(address_map &map) ATTR_COLD;
};

// with sensor hardware
class slikshot_state : public itech8_state
{
public:
	slikshot_state(const machine_config &mconfig, device_type type, const char *tag) :
		itech8_state(mconfig, type, tag),
		m_subcpu(*this, "sub"),
		m_fakex(*this, "FAKEX"),
		m_fakey(*this, "FAKEY")
	{
	}

	void slikshot_lo(machine_config &config);
	void slikshot_hi(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	optional_device<cpu_device> m_subcpu;
	optional_ioport m_fakex;
	optional_ioport m_fakey;

	static constexpr u32 YBUFFER_COUNT = 15;
	u8 m_z80_ctrl = 0;
	u8 m_z80_port_val = 0;
	u8 m_z80_clear_to_send = 0;
	u16 m_sensor0 = 0;
	u16 m_sensor1 = 0;
	u16 m_sensor2 = 0;
	u16 m_sensor3 = 0;
	u8 m_curvx = 0;
	u8 m_curvy = 0;
	u8 m_curx = 0;
	s8 m_xbuffer[YBUFFER_COUNT]{};
	s8 m_ybuffer[YBUFFER_COUNT]{};
	u8 m_ybuffer_next = 0;
	s32 m_curxpos = 0;
	s32 m_last_ytotal = 0;
	u8 m_crosshair_vis = 0;

	//----------- defined in itech/itech8_m.cpp -----------

	u8 z80_port_r();
	void z80_port_w(u8 data);

	u8 z80_r();
	u8 z80_control_r();
	void z80_control_w(u8 data);

	void inters_to_vels(u16 inter1, u16 inter2, u16 inter3, u8 beams,
							u8 &xres, u8 &vxres, u8 &vyres);
	void vels_to_inters(u8 x, u8 vx, u8 vy,
							u16 &inter1, u16 &inter2, u16 &inter3, u8 &beams);
	void inters_to_words(u16 inter1, u16 inter2, u16 inter3, u8 &beams,
							u16 &word1, u16 &word2, u16 &word3);

	void words_to_sensors(u16 word1, u16 word2, u16 word3, u8 beams,
							u16 &sens0, u16 &sens1, u16 &sens2, u16 &sens3);
	void compute_sensors();
	TIMER_CALLBACK_MEMBER(delayed_z80_control_w);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_hi_map(address_map &map) ATTR_COLD;
	void mem_lo_map(address_map &map) ATTR_COLD;
	void z80_io_map(address_map &map) ATTR_COLD;
	void z80_mem_map(address_map &map) ATTR_COLD;
};

// with additional timer
class sstrike_state : public slikshot_state
{
public:
	sstrike_state(const machine_config &mconfig, device_type type, const char *tag) :
		slikshot_state(mconfig, type, tag)
	{
	}

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	emu_timer *m_behind_beam_update_timer = nullptr;
	TIMER_CALLBACK_MEMBER(behind_the_beam_update);
};

// different palette behavior
class grmatch_state : public itech8_state
{
public:
	grmatch_state(const machine_config &mconfig, device_type type, const char *tag) :
		itech8_state(mconfig, type, tag),
		m_palette(*this, "palette_%u", 0U),
		m_palette_timer(nullptr)
	{
	}

	void grmatch(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device_array<palette_device, 2> m_palette;
	emu_timer *m_palette_timer = nullptr;
	u8 m_palcontrol = 0U;
	u8 m_xscroll = 0U;

	void palette_w(u8 data);
	void xscroll_w(u8 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(palette_update);

	void grmatch_map(address_map &map) ATTR_COLD;
};
