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
#include "machine/timer.h"
#include "machine/watchdog.h"
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
		m_watchdog(*this, "watchdog"),
		m_sprites(*this, "sprites"),
		m_segaic16vid(*this, "segaic16vid"),
		m_segaic16road(*this, "segaic16road"),
		m_bankmotor_timer(*this, "bankmotor"),
		m_digital_ports(*this, { { "SERVICE", "UNKNOWN", "COINAGE", "DSW" } }),
		m_adc_ports(*this, "ADC.%u", 0),
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

	void shangon_fd1089b(machine_config &config);
	void outrun_fd1094(machine_config &config);
	void outrundx(machine_config &config);
	void shangon(machine_config &config);
	void outrun_fd1089a(machine_config &config);
	void outrun(machine_config &config);
	void outrun_base(machine_config &config);

	// game-specific driver init
	void init_generic();
	void init_outrun();
	void init_outrunb();
	void init_shangon();

	CUSTOM_INPUT_MEMBER( bankmotor_pos_r );

protected:
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
	void memory_mapper(sega_315_5195_mapper_device &mapper, uint8_t index);

	// main CPU read/write handlers
	DECLARE_READ16_MEMBER( misc_io_r );
	DECLARE_WRITE16_MEMBER( misc_io_w );
	DECLARE_WRITE16_MEMBER( nop_w );

	// video updates
	uint32_t screen_update_outrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_shangon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE16_MEMBER( tileram_w ) { m_segaic16vid->tileram_w(space,offset,data,mem_mask); };
	DECLARE_WRITE16_MEMBER( textram_w ) { m_segaic16vid->textram_w(space,offset,data,mem_mask); };
	DECLARE_READ16_MEMBER( sega_road_control_0_r ) { return m_segaic16road->segaic16_road_control_0_r(space,offset,mem_mask); };
	DECLARE_WRITE16_MEMBER( sega_road_control_0_w ) { m_segaic16road->segaic16_road_control_0_w(space,offset,data,mem_mask); };

	TIMER_DEVICE_CALLBACK_MEMBER(bankmotor_update);

	void decrypted_opcodes_map(address_map &map);
	void outrun_map(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);
	void sub_map(address_map &map);

	// timer IDs
	enum
	{
		TID_SCANLINE,
		TID_IRQ2_GEN,
		TID_SOUND_WRITE
	};

	// device overrides
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

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
	required_device<watchdog_timer_device> m_watchdog;
	required_device<sega_16bit_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<segaic16_road_device> m_segaic16road;
	optional_device<timer_device> m_bankmotor_timer;

	// input ports
	required_ioport_array<4> m_digital_ports;
	optional_ioport_array<8> m_adc_ports;

	// memory
	required_shared_ptr<uint16_t> m_workram;

	// configuration
	read16_delegate     m_custom_io_r;
	write16_delegate    m_custom_io_w;
	const uint8_t *       m_custom_map;
	bool                m_shangon_video;

	// internal state
	emu_timer *         m_scanline_timer;
	emu_timer *         m_irq2_gen_timer;
	uint8_t               m_irq2_state;
	uint8_t               m_adc_select;
	uint8_t               m_vblank_irq_state;
	int                 m_bankmotor_pos;
	int                 m_bankmotor_delta;
};
