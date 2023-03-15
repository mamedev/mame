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
#include "screen.h"


class itech8_state : public driver_device
{
public:
	itech8_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_subcpu(*this, "sub"),
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
		m_fakex(*this, "FAKEX"),
		m_fakey(*this, "FAKEY"),
		m_p1(*this, "P1"),
		m_p2(*this, "P2"),
		m_visarea(0, 0, 0, 0),
		m_bankxor(0)
	{}

	void rimrockn(machine_config &config);
	void gtg2(machine_config &config);
	void slikshot_lo(machine_config &config);
	void slikshot_lo_noz80(machine_config &config);
	void ninclown(machine_config &config);
	void sstrike(machine_config &config);
	void stratab_hi(machine_config &config);
	void hstennis_lo(machine_config &config);
	void wfortune(machine_config &config);
	void stratab_lo(machine_config &config);
	void slikshot_hi(machine_config &config);
	void hstennis_hi(machine_config &config);

	void init_invbank();
	void init_peggle();
	void init_slikshot();
	void init_neckneck();
	void init_arligntn();
	void init_hstennis();

	DECLARE_READ_LINE_MEMBER(special_r);
	DECLARE_CUSTOM_INPUT_MEMBER(gtg_mux);

protected:
	static constexpr uint32_t YBUFFER_COUNT = 15;
	static constexpr uint32_t VRAM_SIZE = 0x40000;
	static constexpr uint32_t VRAM_MASK = VRAM_SIZE - 1;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<cpu_device> m_subcpu;
	required_device<nvram_device> m_nvram;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<tms34061_device> m_tms34061;
	required_device<tlc34076_device> m_tlc34076;
	required_device<screen_device> m_screen;
	required_device<ticket_dispenser_device> m_ticket;
	required_region_ptr<uint8_t> m_grom;
	optional_memory_bank m_mainbank;
	optional_memory_bank m_fixed;
	optional_ioport_array<4> m_an;
	optional_ioport m_fakex;
	optional_ioport m_fakey;
	optional_ioport m_p1;
	optional_ioport m_p2;

	rectangle m_visarea{};

	uint8_t m_grom_bank = 0;
	uint8_t m_blitter_int = 0;
	uint8_t m_tms34061_int = 0;
	uint8_t m_periodic_int = 0;
	uint8_t m_pia_porta_data = 0;
	uint8_t m_pia_portb_data = 0;
	uint8_t m_z80_ctrl = 0;
	uint8_t m_z80_port_val = 0;
	uint8_t m_z80_clear_to_send = 0;
	uint16_t m_sensor0 = 0;
	uint16_t m_sensor1 = 0;
	uint16_t m_sensor2 = 0;
	uint16_t m_sensor3 = 0;
	uint8_t m_curvx = 0;
	uint8_t m_curvy = 0;
	uint8_t m_curx = 0;
	int8_t m_xbuffer[YBUFFER_COUNT]{};
	int8_t m_ybuffer[YBUFFER_COUNT]{};
	int m_ybuffer_next = 0;
	int m_curxpos = 0;
	int m_last_ytotal = 0;
	uint8_t m_crosshair_vis = 0;
	uint8_t m_blitter_data[16]{};
	uint8_t m_blit_in_progress = 0;
	uint8_t m_page_select = 0;
	offs_t m_fetch_offset = 0;
	uint8_t m_fetch_rle_count = 0;
	uint8_t m_fetch_rle_value = 0;
	uint8_t m_fetch_rle_literal = 0;
	emu_timer *m_irq_off_timer = nullptr;
	emu_timer *m_behind_beam_update_timer = nullptr;
	emu_timer *m_blitter_done_timer = nullptr;
	int m_bankxor = 0;

	// common
	DECLARE_WRITE_LINE_MEMBER(generate_tms34061_interrupt);
	void nmi_ack_w(uint8_t data);
	void blitter_bank_w(offs_t offset, uint8_t data);
	void rimrockn_bank_w(uint8_t data);
	void pia_portb_out(uint8_t data);
	void gtg2_sound_data_w(uint8_t data);
	void grom_bank_w(uint8_t data);
	void palette_w(offs_t offset, uint8_t data);
	void page_w(u8 data);
	uint8_t blitter_r(offs_t offset);
	void blitter_w(offs_t offset, uint8_t data);
	void tms34061_w(offs_t offset, uint8_t data);
	uint8_t tms34061_r(offs_t offset);
	void pia_porta_out(uint8_t data);
	void ym2203_portb_out(uint8_t data);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(slikshot);
	DECLARE_MACHINE_START(sstrike);

	uint32_t screen_update_2layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_slikshot(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_2page(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_2page_large(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(generate_nmi);
	DECLARE_WRITE_LINE_MEMBER(ninclown_irq);
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_CALLBACK_MEMBER(behind_the_beam_update);
	TIMER_CALLBACK_MEMBER(blitter_done);

	inline uint8_t fetch_next_raw();
	inline void consume_raw(int count);
	inline uint8_t fetch_next_rle();
	inline void consume_rle(int count);
	void perform_blit();
	void update_interrupts(int periodic, int tms34061, int blitter);

	/*----------- defined in machine/itech8.cpp -----------*/

	uint8_t slikz80_port_r();
	void slikz80_port_w(uint8_t data);

	uint8_t slikshot_z80_r();
	uint8_t slikshot_z80_control_r();
	void slikshot_z80_control_w(uint8_t data);

	void inters_to_vels(uint16_t inter1, uint16_t inter2, uint16_t inter3, uint8_t beams,
							uint8_t *xres, uint8_t *vxres, uint8_t *vyres);
	void vels_to_inters(uint8_t x, uint8_t vx, uint8_t vy,
							uint16_t *inter1, uint16_t *inter2, uint16_t *inter3, uint8_t *beams);
	void inters_to_words(uint16_t inter1, uint16_t inter2, uint16_t inter3, uint8_t *beams,
							uint16_t *word1, uint16_t *word2, uint16_t *word3);

	void words_to_sensors(uint16_t word1, uint16_t word2, uint16_t word3, uint8_t beams,
							uint16_t *sens0, uint16_t *sens1, uint16_t *sens2, uint16_t *sens3);
	void compute_sensors();
	TIMER_CALLBACK_MEMBER( delayed_z80_control_w );

	// ninja clowns
	uint16_t rom_constant_r(offs_t offset);
	uint8_t ninclown_palette_r(offs_t offset);
	void ninclown_palette_w(offs_t offset, uint8_t data);

	void itech8_sound_ym2203(machine_config &config);
	void itech8_sound_ym2608b(machine_config &config);
	void itech8_sound_ym3812(machine_config &config);
	void itech8_sound_ym3812_external(machine_config &config);
	void itech8_core_devices(machine_config &config);
	void itech8_core_lo(machine_config &config);
	void itech8_core_hi(machine_config &config);
	void common_hi_map(address_map &map);
	void common_lo_map(address_map &map);
	void gtg2_map(address_map &map);
	void ninclown_map(address_map &map);
	void rimrockn_map(address_map &map);
	void slikshot_map(address_map &map);
	void slikz80_io_map(address_map &map);
	void slikz80_mem_map(address_map &map);
	void sound2203_map(address_map &map);
	void sound2608b_map(address_map &map);
	void sound3812_external_map(address_map &map);
	void sound3812_map(address_map &map);
	void sstrike_map(address_map &map);
};

class grmatch_state : public itech8_state
{
public:
	grmatch_state(const machine_config &mconfig, device_type type, const char *tag) :
		itech8_state(mconfig, type, tag),
		m_palette_timer(nullptr)
	{
	}

	void grmatch(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void palette_w(uint8_t data);
	void xscroll_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(palette_update);

	emu_timer *m_palette_timer = nullptr;
	uint8_t m_palcontrol = 0U;
	uint8_t m_xscroll = 0U;
	rgb_t m_palette[2][16]{};

	void grmatch_map(address_map &map);
};
