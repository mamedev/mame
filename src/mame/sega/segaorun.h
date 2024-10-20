// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega Outrun hardware

***************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "315_5195.h"
#include "machine/adc0804.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "segaic16.h"
#include "segaic16_road.h"
#include "sega16sp.h"
#include "screen.h"


// ======================> segaorun_state

class segaorun_state : public sega_16bit_common_base
{
public:
	// construction/destruction
	segaorun_state(const machine_config &mconfig, device_type type, const char *tag) :
		sega_16bit_common_base(mconfig, type, tag),
		m_mapper(*this, "mapper"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_soundcpu(*this, "soundcpu"),
		m_i8255(*this, "i8255"),
		m_adc(*this, "adc"),
		m_nvram(*this, "nvram"),
		m_watchdog(*this, "watchdog"),
		m_screen(*this, "screen"),
		m_sprites(*this, "sprites"),
		m_segaic16vid(*this, "segaic16vid"),
		m_segaic16road(*this, "segaic16road"),
		m_bankmotor_timer(*this, "bankmotor"),
		m_digital_ports(*this, { { "SERVICE", "UNKNOWN", "COINAGE", "DSW" } }),
		m_adc_ports(*this, "ADC.%u", 0),
		m_bank_motor_direction(*this, "Bank_Motor_Direction"),
		m_bank_motor_speed(*this, "Bank_Motor_Speed"),
		m_vibration_motor(*this, "Vibration_motor"),
		m_start_lamp(*this, "Start_lamp"),
		m_brake_lamp(*this, "Brake_lamp"),
		m_workram(*this, "workram"),
		m_custom_io_r(*this),
		m_custom_io_w(*this),
		m_custom_map(nullptr),
		m_shangon_video(false),
		m_scanline_timer(nullptr),
		m_irq2_gen_timer(nullptr),
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

	ioport_value bankmotor_pos_r();

protected:
	// PPI read/write handlers
	uint8_t unknown_porta_r();
	uint8_t unknown_portb_r();
	uint8_t unknown_portc_r();
	void unknown_porta_w(uint8_t data);
	void unknown_portb_w(uint8_t data);
	void video_control_w(uint8_t data);
	uint8_t bankmotor_limit_r();
	void bankmotor_control_w(uint8_t data);

	// memory mapping
	void memory_mapper(sega_315_5195_mapper_device &mapper, uint8_t index);

	// main CPU read/write handlers
	uint16_t misc_io_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void misc_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void nop_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// video updates
	uint32_t screen_update_outrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_shangon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_segaic16vid->tileram_w(offset,data,mem_mask); }
	void textram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_segaic16vid->textram_w(offset,data,mem_mask); }
	uint16_t sega_road_control_0_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0) { return m_segaic16road->segaic16_road_control_0_r(); }
	void sega_road_control_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_segaic16road->segaic16_road_control_0_w(offset,data,mem_mask); }

	TIMER_DEVICE_CALLBACK_MEMBER(bankmotor_update);

	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void outrun_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	// device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq2_gen_tick);
	TIMER_CALLBACK_MEMBER(scanline_tick);

	// internal helpers
	void update_main_irqs();
	void m68k_reset_callback(int state);

	// custom I/O
	uint16_t outrun_custom_io_r(address_space &space, offs_t offset);
	void outrun_custom_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t shangon_custom_io_r(address_space &space, offs_t offset);
	void shangon_custom_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t analog_r();

	// devices
	required_device<sega_315_5195_mapper_device> m_mapper;
	required_device<m68000_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<z80_device> m_soundcpu;
	required_device<i8255_device> m_i8255;
	required_device<adc0804_device> m_adc;
	optional_device<nvram_device> m_nvram;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<screen_device> m_screen;
	required_device<sega_16bit_sprite_device> m_sprites;
	required_device<segaic16_video_device> m_segaic16vid;
	required_device<segaic16_road_device> m_segaic16road;
	optional_device<timer_device> m_bankmotor_timer;

	// input ports
	required_ioport_array<4> m_digital_ports;
	optional_ioport_array<8> m_adc_ports;

	// outputs
	output_finder<> m_bank_motor_direction;
	output_finder<> m_bank_motor_speed;
	output_finder<> m_vibration_motor;
	output_finder<> m_start_lamp;
	output_finder<> m_brake_lamp;

	// memory
	required_shared_ptr<uint16_t> m_workram;

	// configuration
	read16m_delegate    m_custom_io_r;
	write16s_delegate   m_custom_io_w;
	const uint8_t *     m_custom_map;
	bool                m_shangon_video;

	// internal state
	emu_timer *         m_scanline_timer;
	emu_timer *         m_irq2_gen_timer;
	uint8_t             m_irq2_state;
	uint8_t             m_adc_select;
	uint8_t             m_vblank_irq_state;
	int                 m_bankmotor_pos;
	int                 m_bankmotor_delta;
};
