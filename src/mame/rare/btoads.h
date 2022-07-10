// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    BattleToads

    Common definitions

*************************************************************************/

#include "cpu/tms34010/tms34010.h"
#include "cpu/z80/z80.h"
#include "video/tlc34076.h"
#include "sound/bsmt2000.h"
#include "machine/nvram.h"
#include "emupal.h"
#include "screen.h"

class btoads_state : public driver_device
{
public:
	btoads_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram_fg0(*this, "vram_fg0", 0x200000, ENDIANNESS_LITTLE),
		m_vram_fg1(*this, "vram_fg1", 0x200000, ENDIANNESS_LITTLE),
		m_vram_fg_data(*this, "vram_fg_data"),
		m_vram_bg0(*this, "vram_bg0", 0x400000, ENDIANNESS_LITTLE),
		m_vram_bg1(*this, "vram_bg1", 0x400000, ENDIANNESS_LITTLE),
		m_sprite_scale(*this, "sprite_scale"),
		m_sprite_control(*this, "sprite_control"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bsmt(*this, "bsmt"),
		m_tlc34076(*this, "tlc34076"),
		m_screen(*this, "screen") { }

	DECLARE_READ_LINE_MEMBER( main_to_sound_r );
	DECLARE_READ_LINE_MEMBER( sound_to_main_r );

	void btoads(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void video_start() override;

	TIMER_CALLBACK_MEMBER(delayed_sound);
	TIMER_CALLBACK_MEMBER(audio_sync) { }

private:
	// timer IDs
	enum
	{
		TIMER_ID_NOP,
		TIMER_ID_DELAYED_SOUND
	};

	// shared pointers
	memory_share_creator<uint8_t> m_vram_fg0;
	memory_share_creator<uint8_t> m_vram_fg1;
	required_shared_ptr<uint32_t> m_vram_fg_data;
	memory_share_creator<uint16_t> m_vram_bg0;
	memory_share_creator<uint16_t> m_vram_bg1;
	required_shared_ptr<uint32_t> m_sprite_scale;
	required_shared_ptr<uint32_t> m_sprite_control;

	// state
	uint8_t m_main_to_sound_data = 0;
	uint8_t m_main_to_sound_ready = 0;
	uint8_t m_sound_to_main_data = 0;
	uint8_t m_sound_to_main_ready = 0;
	uint8_t m_sound_int_state = 0;
	uint8_t *m_vram_fg_draw = nullptr;
	uint8_t *m_vram_fg_display = nullptr;
	int32_t m_xscroll0 = 0;
	int32_t m_yscroll0 = 0;
	int32_t m_xscroll1 = 0;
	int32_t m_yscroll1 = 0;
	uint8_t m_screen_control = 0;
	uint16_t m_sprite_source_offs = 0;
	uint8_t *m_sprite_dest_base = nullptr;
	uint16_t m_sprite_dest_offs = 0;
	uint16_t m_misc_control = 0;
	int m_xcount = 0;
	std::unique_ptr<uint8_t[]> m_nvram_data;
	emu_timer *m_delayed_sound_timer;
	emu_timer *m_audio_sync_timer;

	// in drivers/btoads
	void nvram_w(offs_t offset, uint8_t data);
	uint8_t nvram_r(offs_t offset);
	void main_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t main_sound_r();
	void sound_data_w(uint8_t data);
	uint8_t sound_data_r();
	uint8_t sound_ready_to_send_r();
	uint8_t sound_data_ready_r();
	void sound_int_state_w(uint8_t data);
	uint8_t bsmt_ready_r();
	void bsmt2000_port_w(offs_t offset, uint8_t data);

	// in video/btoads
	void misc_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void display_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void scroll0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void scroll1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vram_bg0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vram_bg1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vram_bg0_r(offs_t offset);
	uint16_t vram_bg1_r(offs_t offset);
	void vram_fg_display_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vram_fg_draw_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vram_fg_display_r(offs_t offset);
	uint16_t vram_fg_draw_r(offs_t offset);
	void render_sprite_row(uint16_t *sprite_source, uint32_t address);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);

	// devices
	required_device<tms34020_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<bsmt2000_device> m_bsmt;
	required_device<tlc34076_device> m_tlc34076;
	required_device<screen_device> m_screen;

	void main_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
};
