// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega System Hang On hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/segaic16.h"
#include "video/segaic16.h"
#include "video/segaic16_road.h"
#include "video/sega16sp.h"


// ======================> segahang_state

class segahang_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segahang_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_subcpu(*this, "subcpu"),
			m_soundcpu(*this, "soundcpu"),
			m_mcu(*this, "mcu"),
			m_i8255_1(*this, "i8255_1"),
			m_i8255_2(*this, "i8255_2"),
			m_sprites(*this, "sprites"),
			m_segaic16vid(*this, "segaic16vid"),
			m_segaic16road(*this, "segaic16road"),
			m_workram(*this, "workram"),
			m_sharrier_video(false),
			m_adc_select(0),
			m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	// PPI read/write callbacks
	DECLARE_WRITE8_MEMBER( video_lamps_w );
	DECLARE_WRITE8_MEMBER( tilemap_sound_w );
	DECLARE_WRITE8_MEMBER( sub_control_adc_w );
	DECLARE_READ8_MEMBER( adc_status_r );

	// main CPU read/write handlers
	DECLARE_READ16_MEMBER( hangon_io_r );
	DECLARE_WRITE16_MEMBER( hangon_io_w );
	DECLARE_READ16_MEMBER( sharrier_io_r );
	DECLARE_WRITE16_MEMBER( sharrier_io_w );

	// Z80 sound CPU read/write handlers
	DECLARE_READ8_MEMBER( sound_data_r );
	DECLARE_WRITE_LINE_MEMBER( sound_irq );

	// I8751-related VBLANK interrupt hanlders
	INTERRUPT_GEN_MEMBER( i8751_main_cpu_vblank );

	// game-specific driver init
	DECLARE_DRIVER_INIT(generic);
	DECLARE_DRIVER_INIT(sharrier);
	DECLARE_DRIVER_INIT(enduror);
	DECLARE_DRIVER_INIT(endurobl);
	DECLARE_DRIVER_INIT(endurob2);

	// video updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// internal types
	typedef delegate<void ()> i8751_sim_delegate;

	// timer IDs
	enum
	{
		TID_INIT_I8751,
		TID_PPI_WRITE
	};

	// driver overrides
	virtual void video_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// I8751 simulations
	void sharrier_i8751_sim();

	// devices
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	optional_device<i8751_device> m_mcu;
	required_device<i8255_device> m_i8255_1;
	required_device<i8255_device> m_i8255_2;
	required_device<sega_16bit_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<segaic16_road_device> m_segaic16road;

	// memory pointers
	required_shared_ptr<UINT16> m_workram;

	// configuration
	bool                    m_sharrier_video;
	i8751_sim_delegate      m_i8751_vblank_hook;

	// internal state
	UINT8                   m_adc_select;
	bool                    m_shadow;
	optional_shared_ptr<UINT16> m_decrypted_opcodes;
	TIMER_DEVICE_CALLBACK_MEMBER(hangon_irq);
};
