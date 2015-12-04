// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega Outrun hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/segaic16.h"
#include "video/segaic16.h"
#include "video/segaic16_road.h"
#include "video/sega16sp.h"


// ======================> segaorun_state

class segaorun_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segaorun_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag),
		m_mapper(*this, "mapper"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_soundcpu(*this, "soundcpu"),
		m_i8255(*this, "i8255"),
		m_nvram(*this, "nvram"),
		m_sprites(*this, "sprites"),
		m_segaic16vid(*this, "segaic16vid"),
		m_segaic16road(*this, "segaic16road"),
		m_bankmotor_timer(*this, "bankmotor"),
		m_digital_ports(*this, digital_ports),
		m_adc_ports(*this, "ADC"),
		m_workram(*this, "workram"),
		m_custom_map(nullptr),
		m_shangon_video(false),
		m_scanline_timer(nullptr),
		m_irq2_state(0),
		m_adc_select(0),
		m_vblank_irq_state(0),
		m_bankmotor_pos(0x8000),
		m_bankmotor_delta(0)
	{ }

	// PPI read/write handlers
	DECLARE_READ8_MEMBER( unknown_porta_r );
	DECLARE_READ8_MEMBER( unknown_portb_r );
	DECLARE_READ8_MEMBER( unknown_portc_r );
	DECLARE_WRITE8_MEMBER( unknown_porta_w );
	DECLARE_WRITE8_MEMBER( unknown_portb_w );
	DECLARE_WRITE8_MEMBER( video_control_w );
	DECLARE_READ8_MEMBER( bankmotor_limit_r );
	DECLARE_WRITE8_MEMBER( bankmotor_control_w );

	// memory mapping
	void memory_mapper(sega_315_5195_mapper_device &mapper, UINT8 index);
	UINT8 mapper_sound_r();
	void mapper_sound_w(UINT8 data);

	// main CPU read/write handlers
	DECLARE_READ16_MEMBER( misc_io_r );
	DECLARE_WRITE16_MEMBER( misc_io_w );
	DECLARE_WRITE16_MEMBER( nop_w );

	// Z80 sound CPU read/write handlers
	DECLARE_READ8_MEMBER( sound_data_r );

	// game-specific driver init
	DECLARE_DRIVER_INIT(generic);
	DECLARE_DRIVER_INIT(outrun);
	DECLARE_DRIVER_INIT(outrunb);
	DECLARE_DRIVER_INIT(shangon);

	// video updates
	UINT32 screen_update_outrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_shangon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE16_MEMBER( tileram_w ) { m_segaic16vid->tileram_w(space,offset,data,mem_mask); };
	DECLARE_WRITE16_MEMBER( textram_w ) { m_segaic16vid->textram_w(space,offset,data,mem_mask); };
	DECLARE_READ16_MEMBER( sega_road_control_0_r ) { return m_segaic16road->segaic16_road_control_0_r(space,offset,mem_mask); };
	DECLARE_WRITE16_MEMBER( sega_road_control_0_w ) { m_segaic16road->segaic16_road_control_0_w(space,offset,data,mem_mask); };

	CUSTOM_INPUT_MEMBER( bankmotor_pos_r );
	TIMER_DEVICE_CALLBACK_MEMBER(bankmotor_update);

protected:
	// timer IDs
	enum
	{
		TID_SCANLINE,
		TID_IRQ2_GEN,
		TID_SOUND_WRITE
	};

	// device overrides
	virtual void machine_reset();
	virtual void video_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// internal helpers
	void update_main_irqs();
	DECLARE_WRITE_LINE_MEMBER(m68k_reset_callback);

	// custom I/O
	DECLARE_READ16_MEMBER( outrun_custom_io_r );
	DECLARE_WRITE16_MEMBER( outrun_custom_io_w );
	DECLARE_READ16_MEMBER( shangon_custom_io_r );
	DECLARE_WRITE16_MEMBER( shangon_custom_io_w );

	// devices
	required_device<sega_315_5195_mapper_device> m_mapper;
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	required_device<i8255_device> m_i8255;
	optional_device<nvram_device> m_nvram;
	required_device<sega_16bit_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<segaic16_road_device> m_segaic16road;
	optional_device<timer_device> m_bankmotor_timer;

	// input ports
	DECLARE_IOPORT_ARRAY(digital_ports);
	required_ioport_array<4> m_digital_ports;
	optional_ioport_array<8> m_adc_ports;

	// memory
	required_shared_ptr<UINT16> m_workram;

	// configuration
	read16_delegate     m_custom_io_r;
	write16_delegate    m_custom_io_w;
	const UINT8 *       m_custom_map;
	bool                m_shangon_video;

	// internal state
	emu_timer *         m_scanline_timer;
	UINT8               m_irq2_state;
	UINT8               m_adc_select;
	UINT8               m_vblank_irq_state;
	int                 m_bankmotor_pos;
	int                 m_bankmotor_delta;
};
