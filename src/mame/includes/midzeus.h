// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/

#define MIDZEUS_VIDEO_CLOCK     XTAL(66'666'700)

#include "machine/timekpr.h"
#include "emupal.h"
#include "screen.h"
#include "machine/midwayic.h"

class midzeus_state : public driver_device
{
public:
	midzeus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_nvram(*this, "nvram"),
		m_ram_base(*this, "ram_base"),
		m_firewire(*this, "firewire"),
		m_tms32031_control(*this, "tms32031_ctl"),
		m_zeusbase(*this, "zeusbase"),
		m_m48t35(*this, "m48t35"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ioasic(*this, "ioasic"),
		m_io_analog(*this, "ANALOG%u", 0U),
		m_io_gun_x(*this, "GUNX%u", 1U),
		m_io_gun_y(*this, "GUNY%u", 1U),
		m_io_trackx(*this, "TRACKX1"),
		m_io_tracky(*this, "TRACKY1"),
		m_io_49way_x(*this, "49WAYX"),
		m_io_49way_y(*this, "49WAYY"),
		m_io_keypad(*this, "KEYPAD"),
		m_digits(*this, "digit%u", 0U)
	{ }

	//static constexpr XTAL CPU_CLOCK = XTAL(60'000'000);
	static constexpr int BEAM_DY = 3;
	static constexpr int BEAM_DX = 3;
	static constexpr int BEAM_XOFFS = 40; // table in the code indicates an offset of 20 with a beam height of 7

	required_shared_ptr<uint32_t> m_nvram;
	required_shared_ptr<uint32_t> m_ram_base;
	optional_shared_ptr<uint32_t> m_firewire;
	required_shared_ptr<uint32_t> m_tms32031_control;
	optional_shared_ptr<uint32_t> m_zeusbase;

	DECLARE_WRITE32_MEMBER(cmos_w);
	DECLARE_READ32_MEMBER(cmos_r);
	DECLARE_WRITE32_MEMBER(cmos_protect_w);
	DECLARE_READ32_MEMBER(zpram_r);
	DECLARE_WRITE32_MEMBER(zpram_w);
	DECLARE_READ32_MEMBER(disk_asic_r);
	DECLARE_WRITE32_MEMBER(disk_asic_w);
	DECLARE_READ32_MEMBER(disk_asic_jr_r);
	DECLARE_WRITE32_MEMBER(disk_asic_jr_w);
	DECLARE_READ32_MEMBER(firewire_r);
	DECLARE_WRITE32_MEMBER(firewire_w);
	DECLARE_READ32_MEMBER(tms32031_control_r);
	DECLARE_WRITE32_MEMBER(tms32031_control_w);
	DECLARE_WRITE32_MEMBER(keypad_select_w);
	DECLARE_READ32_MEMBER(analog_r);
	DECLARE_WRITE32_MEMBER(analog_w);
	DECLARE_WRITE32_MEMBER(invasn_gun_w);
	DECLARE_READ32_MEMBER(invasn_gun_r);
	DECLARE_READ32_MEMBER(zeus_r);
	DECLARE_WRITE32_MEMBER(zeus_w);
	DECLARE_CUSTOM_INPUT_MEMBER(custom_49way_r);
	DECLARE_CUSTOM_INPUT_MEMBER(keypad_r);
	DECLARE_READ32_MEMBER(grid_keypad_r);
	DECLARE_READ32_MEMBER(trackball_r);
	void init_invasn();
	void init_mk4();
	DECLARE_MACHINE_START(midzeus);
	DECLARE_MACHINE_RESET(midzeus);
	DECLARE_VIDEO_START(midzeus);
	uint32_t screen_update_midzeus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(display_irq);
	TIMER_CALLBACK_MEMBER(display_irq_off);
	TIMER_CALLBACK_MEMBER(invasn_gun_callback);
	void midzeus(machine_config &config);
	void invasn(machine_config &config);
	void mk4(machine_config &config);
	void zeus_map(address_map &map);
protected:
	optional_device<timekeeper_device> m_m48t35;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	required_device<midway_ioasic_device> m_ioasic;
	optional_ioport_array<4> m_io_analog;
	optional_ioport_array<2> m_io_gun_x;
	optional_ioport_array<2> m_io_gun_y;
	optional_ioport m_io_trackx;
	optional_ioport m_io_tracky;
	optional_ioport m_io_49way_x;
	optional_ioport m_io_49way_y;
	optional_ioport m_io_keypad;
	output_finder<7> m_digits;
	emu_timer *m_display_irq_off_timer;
	uint8_t            crusnexo_leds_select;
	uint32_t           disk_asic[0x10];
	uint32_t           disk_asic_jr[0x10];

	uint8_t cmos_protected;

	emu_timer *timer[2];
private:
	uint32_t           gun_control;
	uint8_t            gun_irq_state;
	emu_timer *        gun_timer[2];
	int32_t            gun_x[2], gun_y[2];
	uint8_t            keypad_select;
	void exit_handler();
	void zeus_pointer_w(uint32_t which, uint32_t data, bool logit);
	void zeus_register16_w(offs_t offset, uint16_t data, bool logit);
	void zeus_register32_w(offs_t offset, uint32_t data, bool logit);
	void zeus_register_update(offs_t offset);
	int zeus_fifo_process(const uint32_t *data, int numwords);
	void zeus_draw_model(uint32_t texdata, bool logit);

	void log_fifo_command(const uint32_t *data, int numwords, const char *suffix);
	void log_waveram(uint32_t length_and_base);
	void update_gun_irq();
};
