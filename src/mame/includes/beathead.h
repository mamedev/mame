/*************************************************************************

    Atari "Stella on Steroids" hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "cpu/asap/asap.h"
#include "audio/atarijsa.h"

class beathead_state : public atarigen_state
{
public:
	beathead_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config),
		  m_maincpu(*this, "maincpu"),
		  m_nvram(*this, "nvram") { }

	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	virtual bool video_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect);

	required_device<asap_device> m_maincpu;

	required_shared_ptr<UINT32>	m_nvram;

	UINT32 *		m_videoram;
	UINT32 *		m_paletteram;

	UINT32 *		m_vram_bulk_latch;
	UINT32 *		m_palette_select;

	UINT32			m_finescroll;
	offs_t			m_vram_latch_offset;

	offs_t			m_hsyncram_offset;
	offs_t			m_hsyncram_start;
	UINT8			m_hsyncram[0x800];

	UINT32 *		m_ram_base;
	UINT32 *		m_rom_base;

	double			m_hblank_offset;

	UINT8			m_irq_line_state;
	UINT8			m_irq_enable[3];
	UINT8			m_irq_state[3];

	UINT8			m_eeprom_enabled;

	UINT32 *		m_speedup_data;
	UINT32 *		m_movie_speedup_data;

	// in drivers/beathead.c
	void update_interrupts();
	DECLARE_WRITE32_MEMBER( interrupt_control_w );
	DECLARE_READ32_MEMBER( interrupt_control_r );
	DECLARE_WRITE32_MEMBER( eeprom_data_w );
	DECLARE_WRITE32_MEMBER( eeprom_enable_w );
	DECLARE_READ32_MEMBER( input_2_r );
	DECLARE_READ32_MEMBER( sound_data_r );
	DECLARE_WRITE32_MEMBER( sound_data_w );
	DECLARE_WRITE32_MEMBER( sound_reset_w );
	DECLARE_WRITE32_MEMBER( coin_count_w );
	DECLARE_READ32_MEMBER( speedup_r );
	DECLARE_READ32_MEMBER( movie_speedup_r );

	// in video/beathead.c
	DECLARE_WRITE32_MEMBER( vram_transparent_w );
	DECLARE_WRITE32_MEMBER( vram_bulk_w );
	DECLARE_WRITE32_MEMBER( vram_latch_w );
	DECLARE_WRITE32_MEMBER( vram_copy_w );
	DECLARE_WRITE32_MEMBER( finescroll_w );
	DECLARE_WRITE32_MEMBER( palette_w );
	DECLARE_READ32_MEMBER( hsync_ram_r );
	DECLARE_WRITE32_MEMBER( hsync_ram_w );
};
