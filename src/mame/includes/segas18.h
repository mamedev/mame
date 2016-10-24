// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega System 16A/16B/18/Outrun/Hang On/X-Board/Y-Board hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/segaic16.h"
#include "machine/315_5296.h"
#include "video/315_5313.h"
#include "video/segaic16.h"
#include "video/sega16sp.h"


// ======================> segas18_state

class segas18_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segas18_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag),
			m_mapper(*this, "mapper"),
			m_maincpu(*this, "maincpu"),
			m_maincpu_region(*this, "maincpu"),
			m_soundcpu(*this, "soundcpu"),
			m_mcu(*this, "mcu"),
			m_vdp(*this, "gen_vdp"),
			m_io(*this, "io"),
			m_nvram(*this, "nvram"),
			m_sprites(*this, "sprites"),
			m_segaic16vid(*this, "segaic16vid"),
			m_gfxdecode(*this, "gfxdecode"),
			m_soundlatch(*this, "soundlatch"),
			m_workram(*this, "workram"),
			m_romboard(ROM_BOARD_INVALID),
			m_grayscale_enable(false),
			m_vdp_enable(false),
			m_vdp_mixing(0),
			m_mcu_data(0),
			m_lghost_value(0),
			m_lghost_select(0)
	{
		memset(m_wwally_last_x, 0, sizeof(m_wwally_last_x));
		memset(m_wwally_last_y, 0, sizeof(m_wwally_last_y));
	}

	// driver init
	void init_ddcrew();
	void init_lghost();
	void init_generic_shad();
	void init_generic_5874();
	void init_wwally();
	void init_generic_5987();
	void init_hamaway();

	// memory mapping
	void memory_mapper(sega_315_5195_mapper_device &mapper, uint8_t index);
	uint8_t mapper_sound_r();
	void mapper_sound_w(uint8_t data);

	// read/write handlers
	void rom_5874_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rom_5987_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rom_837_7525_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void misc_outputs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t misc_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void misc_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void soundbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void ym3438_irq_handler(int state);

	// custom I/O
	uint16_t ddcrew_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t lghost_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void lghost_gun_recoil_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lghost_custom_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t wwally_custom_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void wwally_custom_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// video rendering
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vdp_sndirqline_callback_s18(int state);
	void vdp_lv6irqline_callback_s18(int state);
	void vdp_lv4irqline_callback_s18(int state);

	uint16_t genesis_vdp_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_vdp->vdp_r(space, offset, mem_mask); }
	void genesis_vdp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_vdp->vdp_w(space, offset, data, mem_mask); }
	void tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_segaic16vid->tileram_w(space, offset, data, mem_mask); }
	void textram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { m_segaic16vid->textram_w(space, offset, data, mem_mask); }

	void set_grayscale(int state);
	void set_vdp_enable(int state);

protected:
	// timer IDs
	enum
	{
		TID_INITIAL_BOOST
	};

	// rom board types
	enum segas18_rom_board
	{
		ROM_BOARD_INVALID,
		ROM_BOARD_171_SHADOW,   // 171-???? -- used by shadow dancer
		ROM_BOARD_171_5874,     // 171-5874
		ROM_BOARD_171_5987,     // 171-5987
		ROM_BOARD_837_7525      // Hammer Away proto
	};

	// device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// internal helpers
	void init_generic(segas18_rom_board rom_board);
	void set_vdp_mixing(uint8_t mixing);
	void draw_vdp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);

	// devices
	required_device<sega_315_5195_mapper_device> m_mapper;
	required_device<m68000_device> m_maincpu;
	required_memory_region m_maincpu_region;
	required_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device<sega315_5313_device> m_vdp;
	required_device<sega_315_5296_device> m_io;
	required_device<nvram_device> m_nvram;
	required_device<sega_sys16b_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory pointers
	required_shared_ptr<uint16_t> m_workram;

	// configuration
	segas18_rom_board   m_romboard;
	read16_delegate     m_custom_io_r;
	write16_delegate    m_custom_io_w;

	// internal state
	int                 m_grayscale_enable;
	int                 m_vdp_enable;
	uint8_t               m_vdp_mixing;
	bitmap_ind16        m_temp_bitmap;
	uint8_t               m_mcu_data;

	// game-specific state
	uint8_t               m_wwally_last_x[3];
	uint8_t               m_wwally_last_y[3];
	uint8_t               m_lghost_value;
	uint8_t               m_lghost_select;
};
