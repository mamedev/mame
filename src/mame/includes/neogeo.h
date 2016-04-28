// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Ernesto Corvi,Andrew Prime,Zsolt Vasvari
// thanks-to:Fuzz
/*************************************************************************

    Neo-Geo hardware

*************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2610intf.h"
#include "machine/upd1990a.h"
#include "machine/ng_memcard.h"
#include "video/neogeo_spr.h"

#include "bus/neogeo/slot.h"
#include "bus/neogeo/carts.h"
#include "bus/neogeo_ctrl/ctrl.h"


// On scanline 224, /VBLANK goes low 56 mclks (14 pixels) from the rising edge of /HSYNC.
// Two mclks after /VBLANK goes low, the hardware sets a pending IRQ1 flip-flop.
#define NEOGEO_VBLANK_IRQ_HTIM (attotime::from_ticks(56+2, NEOGEO_MASTER_CLOCK))


class neogeo_state : public driver_device
{
public:
	neogeo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_region_maincpu(*this, "maincpu"),
		m_region_sprites(*this, "sprites"),
		m_region_fixed(*this, "fixed"),
		m_region_fixedbios(*this, "fixedbios"),
		m_region_mainbios(*this, "mainbios"),
		m_region_audiobios(*this, "audiobios"),
		m_region_audiocpu(*this, "audiocpu"),
		m_bank_audio_main(*this, "audio_main"),
		m_upd4990a(*this, "upd4990a"),
		m_ym(*this, "ymsnd"),
		m_save_ram(*this, "saveram"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_memcard(*this, "memcard"),
		m_dsw(*this, "DSW"),
		m_trackx(*this, "TRACK_X"),
		m_tracky(*this, "TRACK_Y"),
		m_edge(*this, "edge"),
		m_ctrl1(*this, "ctrl1"),
		m_ctrl2(*this, "ctrl2"),
		m_sprgen(*this, "spritegen"),
		m_use_cart_vectors(0),
		m_use_cart_audio(0),
		m_slot1(*this, "cslot1"),
		m_slot2(*this, "cslot2"),
		m_slot3(*this, "cslot3"),
		m_slot4(*this, "cslot4"),
		m_slot5(*this, "cslot5"),
		m_slot6(*this, "cslot6")
	{ }

	DECLARE_READ16_MEMBER(memcard_r);
	DECLARE_WRITE16_MEMBER(memcard_w);
	DECLARE_WRITE8_MEMBER(audio_command_w);
	DECLARE_READ8_MEMBER(audio_command_r);
	DECLARE_READ8_MEMBER(audio_cpu_bank_select_r);
	DECLARE_WRITE8_MEMBER(audio_cpu_enable_nmi_w);
	DECLARE_READ16_MEMBER(unmapped_r);
	DECLARE_READ16_MEMBER(paletteram_r);
	DECLARE_WRITE16_MEMBER(paletteram_w);
	DECLARE_READ16_MEMBER(video_register_r);
	DECLARE_WRITE16_MEMBER(video_register_w);
	DECLARE_READ16_MEMBER(in0_r);
	DECLARE_READ16_MEMBER(in1_r);

	DECLARE_CUSTOM_INPUT_MEMBER(get_memcard_status);
	DECLARE_CUSTOM_INPUT_MEMBER(get_audio_result);

	TIMER_CALLBACK_MEMBER(display_position_interrupt_callback);
	TIMER_CALLBACK_MEMBER(display_position_vblank_callback);
	TIMER_CALLBACK_MEMBER(vblank_interrupt_callback);

	// MVS-specific
	DECLARE_WRITE16_MEMBER(save_ram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(kizuna4p_start_r);

	UINT32 screen_update_neogeo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_DRIVER_INIT(mvs);

	// NEW IMPLEMENTATION!!!
	void set_slot_idx(int slot);
	void neogeo_postload();
	
	DECLARE_WRITE8_MEMBER(io_control_w);
	DECLARE_WRITE8_MEMBER(system_control_w);
	DECLARE_READ16_MEMBER(banked_vectors_r);
	DECLARE_WRITE16_MEMBER(write_banksel);
	DECLARE_WRITE16_MEMBER(write_bankprot);
	DECLARE_WRITE16_MEMBER(write_bankprot_pvc);
	DECLARE_WRITE16_MEMBER(write_bankprot_ms5p);
	DECLARE_WRITE16_MEMBER(write_bankprot_kf2k3bl);
	DECLARE_WRITE16_MEMBER(write_bankprot_kof10th);
	DECLARE_READ16_MEMBER(read_lorom_kof10th);
	
	DECLARE_DRIVER_INIT(neogeo);

protected:
	void common_machine_start();

	void update_interrupts();
	void create_interrupt_timers();
	void start_interrupt_timers();
	void acknowledge_interrupt(UINT16 data);

	void adjust_display_position_interrupt_timer();
	void set_display_position_interrupt_control(UINT16 data);
	void set_display_counter_msb(UINT16 data);
	void set_display_counter_lsb(UINT16 data);
	void set_video_control(UINT16 data);

	void create_rgb_lookups();
	void set_pens();
	void set_screen_shadow(int data);
	void set_palette_bank(int data);

	void audio_cpu_check_nmi();
	void set_save_ram_unlock(UINT8 data);
	void set_outputs();
	void set_output_latch(UINT8 data);
	void set_output_data(UINT8 data);

	// device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;

	// memory
	optional_memory_region m_region_maincpu;
	optional_memory_region m_region_sprites;
	optional_memory_region m_region_fixed;
	optional_memory_region m_region_fixedbios;
	optional_memory_region m_region_mainbios;
	optional_memory_region m_region_audiobios;
	optional_memory_region m_region_audiocpu;
	optional_memory_bank   m_bank_audio_main; // optional because of neocd
	memory_bank           *m_bank_audio_cart[4];
	memory_bank           *m_bank_cartridge;
	
	// MVS-specific devices
	optional_device<upd4990a_device> m_upd4990a;
	optional_device<ym2610_device> m_ym;
	optional_shared_ptr<UINT16> m_save_ram;

	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_device<ng_memcard_device> m_memcard;

	// configuration
	enum {NEOGEO_MVS, NEOGEO_AES, NEOGEO_CD} m_type;

	// internal state
	bool       m_recurse;
	bool       m_audio_cpu_nmi_enabled;
	bool       m_audio_cpu_nmi_pending;

	// MVS-specific state
	UINT8      m_save_ram_unlocked;
	UINT8      m_output_data;
	UINT8      m_output_latch;
	UINT8      m_el_value;
	UINT8      m_led1_value;
	UINT8      m_led2_value;

	optional_ioport m_dsw;
	optional_ioport m_trackx;
	optional_ioport m_tracky;
	optional_device<neogeo_ctrl_edge_port_device> m_edge;
	optional_device<neogeo_control_port_device> m_ctrl1;
	optional_device<neogeo_control_port_device> m_ctrl2;

	// video hardware, including maincpu interrupts
	// TODO: make into a device
	virtual void video_start() override;
	virtual void video_reset() override;

	emu_timer  *m_display_position_interrupt_timer;
	emu_timer  *m_display_position_vblank_timer;
	emu_timer  *m_vblank_interrupt_timer;
	UINT32     m_display_counter;
	UINT8      m_vblank_interrupt_pending;
	UINT8      m_display_position_interrupt_pending;
	UINT8      m_irq3_pending;
	UINT8      m_display_position_interrupt_control;
	UINT8      m_vblank_level;
	UINT8      m_raster_level;

	required_device<neosprite_optimized_device> m_sprgen;
	UINT16 get_video_control();

	// color/palette related
	std::vector<UINT16> m_paletteram;
	UINT8        m_palette_lookup[32][4];
	const pen_t *m_bg_pen;
	int          m_screen_shadow;
	int          m_palette_bank;


	int m_use_cart_vectors;
	int m_use_cart_audio;

	// cart slots
	void init_cpu();
	void init_audio();
	void init_ym();
	void init_sprites();
	// temporary helper to restore memory banking while bankswitch is handled in the driver...
	UINT32 m_bank_base;
	
	optional_device<neogeo_cart_slot_device> m_slot1;
	optional_device<neogeo_cart_slot_device> m_slot2;
	optional_device<neogeo_cart_slot_device> m_slot3;
	optional_device<neogeo_cart_slot_device> m_slot4;
	optional_device<neogeo_cart_slot_device> m_slot5;
	optional_device<neogeo_cart_slot_device> m_slot6;
	
	int m_curr_slot;
	neogeo_cart_slot_device* m_slots[6];
};


class aes_state : public neogeo_state
{
	public:
		aes_state(const machine_config &mconfig, device_type type, const char *tag)
			: neogeo_state(mconfig, type, tag)
			, m_io_in2(*this, "IN2")
	{}
	
	DECLARE_READ16_MEMBER(aes_in2_r);
	
	DECLARE_INPUT_CHANGED_MEMBER(aes_jp1);
	
	DECLARE_MACHINE_START(aes);
	
protected:
	required_ioport m_io_in2;
};


#include "bus/neogeo/prot_pcm2.h"
#include "bus/neogeo/prot_cmc.h"
#include "bus/neogeo/prot_pvc.h"

class neopcb_state : public neogeo_state
{
	public:
		neopcb_state(const machine_config &mconfig, device_type type, const char *tag)
			: neogeo_state(mconfig, type, tag)
		, m_cmc_prot(*this, "cmc50")
		, m_pcm2_prot(*this, "pcm2")
		, m_pvc_prot(*this, "pvc")
	{}
	
	// device overrides
	virtual void machine_start() override;

	DECLARE_WRITE16_MEMBER(write_bankpvc);

	DECLARE_INPUT_CHANGED_MEMBER(select_bios);

	DECLARE_DRIVER_INIT(ms5pcb);
	DECLARE_DRIVER_INIT(svcpcb);
	DECLARE_DRIVER_INIT(kf2k3pcb);
	DECLARE_DRIVER_INIT(vliner);

	void install_common();
	void install_banked_bios();
	void neopcb_postload();
	// non-carts
	void svcpcb_gfx_decrypt();
	void svcpcb_s1data_decrypt();
	void kf2k3pcb_gfx_decrypt();
	void kf2k3pcb_decrypt_s1data();
	void kf2k3pcb_sp1_decrypt();
	
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<pvc_prot_device> m_pvc_prot;
};


/*----------- defined in drivers/neogeo.c -----------*/

MACHINE_CONFIG_EXTERN( neogeo_base );
MACHINE_CONFIG_EXTERN( neogeo_arcade );
INPUT_PORTS_EXTERN(neogeo);
INPUT_PORTS_EXTERN(aes);

