// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari "Stella on Steroids" hardware

***************************************************************************/

#include "machine/atarigen.h"
#include "cpu/asap/asap.h"
#include "audio/atarijsa.h"

class beathead_state : public atarigen_state
{
public:
	beathead_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_jsa(*this, "jsa"),
			m_nvram(*this, "nvram"),
			m_videoram(*this, "videoram"),
			m_vram_bulk_latch(*this, "vram_bulk_latch"),
			m_palette_select(*this, "palette_select"),
			m_ram_base(*this, "ram_base"),
			m_rom_base(*this, "rom_base") { }

	virtual void machine_reset() override;

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<asap_device> m_maincpu;
	required_device<atari_jsa_iii_device> m_jsa;

	required_shared_ptr<uint32_t> m_nvram;

	required_shared_ptr<uint32_t> m_videoram;

	required_shared_ptr<uint32_t> m_vram_bulk_latch;
	required_shared_ptr<uint32_t> m_palette_select;

	uint32_t          m_finescroll;
	offs_t          m_vram_latch_offset;

	offs_t          m_hsyncram_offset;
	offs_t          m_hsyncram_start;
	uint8_t           m_hsyncram[0x800];

	required_shared_ptr<uint32_t> m_ram_base;
	required_shared_ptr<uint32_t> m_rom_base;

	attotime        m_hblank_offset;

	uint8_t           m_irq_line_state;
	uint8_t           m_irq_enable[3];
	uint8_t           m_irq_state[3];

	uint8_t           m_eeprom_enabled;

	uint32_t *        m_speedup_data;
	uint32_t *        m_movie_speedup_data;

	// in drivers/beathead.c
	virtual void update_interrupts() override;
	void interrupt_control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t interrupt_control_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void eeprom_data_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void eeprom_enable_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void sound_reset_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void coin_count_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t movie_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

	// in video/beathead.c
	void vram_transparent_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void vram_bulk_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void vram_latch_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void vram_copy_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void finescroll_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t hsync_ram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void hsync_ram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void init_beathead();
	void scanline_callback(timer_device &timer, void *ptr, int32_t param);
};
