// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Ernesto Corvi,Andrew Prime,Zsolt Vasvari
// thanks-to:Fuzz
/*************************************************************************

    Neo-Geo hardware

*************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2610intf.h"
#include "machine/gen_latch.h"
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
		m_upd4990a(*this, "upd4990a"),
		m_ym(*this, "ymsnd"),
		m_sprgen(*this, "spritegen"),
		m_save_ram(*this, "saveram"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_memcard(*this, "memcard"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_region_maincpu(*this, "maincpu"),
		m_region_sprites(*this, "sprites"),
		m_region_fixed(*this, "fixed"),
		m_region_fixedbios(*this, "fixedbios"),
		m_region_mainbios(*this, "mainbios"),
		m_region_audiobios(*this, "audiobios"),
		m_region_audiocpu(*this, "audiocpu"),
		m_bank_audio_main(*this, "audio_main"),
		m_dsw(*this, "DSW"),
		m_trackx(*this, "TRACK_X"),
		m_tracky(*this, "TRACK_Y"),
		m_edge(*this, "edge"),
		m_ctrl1(*this, "ctrl1"),
		m_ctrl2(*this, "ctrl2"),
		m_use_cart_vectors(0),
		m_use_cart_audio(0),
		m_slot1(*this, "cslot1"),
		m_slot2(*this, "cslot2"),
		m_slot3(*this, "cslot3"),
		m_slot4(*this, "cslot4"),
		m_slot5(*this, "cslot5"),
		m_slot6(*this, "cslot6")
	{ }

	uint16_t memcard_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void memcard_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void audio_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t audio_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t audio_cpu_bank_select_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void audio_cpu_enable_nmi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t unmapped_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t paletteram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void paletteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t video_register_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void video_register_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t in0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t in1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	ioport_value get_memcard_status(ioport_field &field, void *param);
	ioport_value get_audio_result(ioport_field &field, void *param);

	void display_position_interrupt_callback(void *ptr, int32_t param);
	void display_position_vblank_callback(void *ptr, int32_t param);
	void vblank_interrupt_callback(void *ptr, int32_t param);

	// MVS-specific
	void save_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	ioport_value kizuna4p_start_r(ioport_field &field, void *param);

	uint32_t screen_update_neogeo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void io_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void system_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t banked_vectors_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void write_banksel(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void write_bankprot(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void write_bankprot_pvc(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void write_bankprot_ms5p(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void write_bankprot_kf2k3bl(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void write_bankprot_kof10th(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t read_lorom_kof10th(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void init_neogeo();

protected:
	void common_machine_start();

	void set_outputs();

	// device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void neogeo_postload();

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	// MVS-specific devices
	optional_device<upd4990a_device> m_upd4990a;
	optional_device<ym2610_device> m_ym;
	required_device<neosprite_optimized_device> m_sprgen;
	optional_shared_ptr<uint16_t> m_save_ram;

	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_device<ng_memcard_device> m_memcard;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

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

	// configuration
	enum {NEOGEO_MVS, NEOGEO_AES, NEOGEO_CD} m_type;

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

	const pen_t *m_bg_pen;
	uint8_t      m_vblank_level;
	uint8_t      m_raster_level;

	int m_use_cart_vectors;
	int m_use_cart_audio;

	void set_slot_idx(int slot);

	// cart slots
	void init_cpu();
	void init_audio();
	void init_ym();
	void init_sprites();
	// temporary helper to restore memory banking while bankswitch is handled in the driver...
	uint32_t m_bank_base;

	optional_device<neogeo_cart_slot_device> m_slot1;
	optional_device<neogeo_cart_slot_device> m_slot2;
	optional_device<neogeo_cart_slot_device> m_slot3;
	optional_device<neogeo_cart_slot_device> m_slot4;
	optional_device<neogeo_cart_slot_device> m_slot5;
	optional_device<neogeo_cart_slot_device> m_slot6;

	int m_curr_slot;
	neogeo_cart_slot_device* m_slots[6];

private:
	void update_interrupts();
	void create_interrupt_timers();
	void start_interrupt_timers();
	void acknowledge_interrupt(uint16_t data);

	void adjust_display_position_interrupt_timer();
	void set_display_position_interrupt_control(uint16_t data);
	void set_display_counter_msb(uint16_t data);
	void set_display_counter_lsb(uint16_t data);
	void set_video_control(uint16_t data);

	void create_rgb_lookups();
	void set_pens();
	void set_screen_shadow(int data);
	void set_palette_bank(int data);

	void audio_cpu_check_nmi();
	void set_save_ram_unlock(uint8_t data);
	void set_output_latch(uint8_t data);
	void set_output_data(uint8_t data);

	// internal state
	bool       m_recurse;
	bool       m_audio_cpu_nmi_enabled;
	bool       m_audio_cpu_nmi_pending;

	// MVS-specific state
	uint8_t      m_save_ram_unlocked;
	uint8_t      m_output_data;
	uint8_t      m_output_latch;
	uint8_t      m_el_value;
	uint8_t      m_led1_value;
	uint8_t      m_led2_value;

	emu_timer  *m_display_position_interrupt_timer;
	emu_timer  *m_display_position_vblank_timer;
	emu_timer  *m_vblank_interrupt_timer;
	uint32_t     m_display_counter;
	uint8_t      m_vblank_interrupt_pending;
	uint8_t      m_display_position_interrupt_pending;
	uint8_t      m_irq3_pending;
	uint8_t      m_display_position_interrupt_control;

	uint16_t get_video_control();

	// color/palette related
	std::vector<uint16_t> m_paletteram;
	uint8_t        m_palette_lookup[32][4];
	int          m_screen_shadow;
	int          m_palette_bank;
};


class aes_state : public neogeo_state
{
	public:
		aes_state(const machine_config &mconfig, device_type type, const char *tag)
			: neogeo_state(mconfig, type, tag)
			, m_io_in2(*this, "IN2")
	{}

	uint16_t aes_in2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void aes_jp1(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void machine_start_aes();

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

	void write_bankpvc(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void select_bios(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	void init_ms5pcb();
	void init_svcpcb();
	void init_kf2k3pcb();
	void init_vliner();

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
