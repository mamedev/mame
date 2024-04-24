// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB)

****************************************************************************/

#ifndef MAME_HEATHKIT_TLB_H
#define MAME_HEATHKIT_TLB_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/ins8250.h"
#include "machine/mm5740.h"
#include "sound/beep.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class heath_tlb_connector;

class device_heath_tlb_card_interface : public device_interface
{
public:

	// required operation
	virtual void serial_in_w(int state) = 0;

	// optional operations
	virtual void rlsd_in_w(int state) {}
	virtual void dsr_in_w(int state) {}
	virtual void cts_in_w(int state) {}

	// optional SigmaSet operations
	virtual void sigma_ctrl_w(u8 data) {}
	virtual u8 sigma_ctrl_r() { return 0x00; }
	virtual u8 sigma_video_mem_r() { return 0x00; }
	virtual void sigma_video_mem_w(u8 val) {}
	virtual void sigma_io_lo_addr_w(u8 val) {}
	virtual void sigma_io_hi_addr_w(u8 val) {}
	virtual void sigma_window_lo_addr_w(u8 val) {}
	virtual void sigma_window_hi_addr_w(u8 val) {}

protected:
	device_heath_tlb_card_interface(const machine_config &mconfig, device_t &device);

	heath_tlb_connector *const m_slot;
};


/**
 *  Standard Heath Terminal logic board
 */
class heath_tlb_device : public device_t,
						 public device_heath_tlb_card_interface
{
public:
	heath_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// 8250 UART
	virtual void serial_in_w(int state) override;
	virtual void rlsd_in_w(int state) override;
	virtual void dsr_in_w(int state) override;
	virtual void cts_in_w(int state) override;
	void serial_irq_w(int state);

	// Keyboard
	void reset_key_w(int state);
	void right_shift_w(int state);
	void repeat_key_w(int state);
	void break_key_w(int state);

protected:
	heath_tlb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void mem_map(address_map &map);
	void io_map(address_map &map);


	// interrupt related
	virtual void set_irq_line();

	// reset
	void check_for_reset();

	// sound/speaker
	void check_beep_state();
	void key_click_w(u8 data);
	void bell_w(u8 data);
	TIMER_CALLBACK_MEMBER(key_click_off);
	TIMER_CALLBACK_MEMBER(bell_off);

	// keyboard
	u8 kbd_key_r();
	u8 kbd_flags_r();
	u16 translate_mm5740_b(u16 b);
	int mm5740_shift_r();
	int mm5740_control_r();
	void mm5740_data_ready_w(int state);

	// serial port
	void serial_out_b(int data);
	void dtr_out(int data);
	void rts_out(int data);

	// crtc
	void crtc_addr_w(offs_t reg, u8 val);
	virtual u8 crtc_reg_r(offs_t reg);
	virtual void crtc_reg_w(offs_t reg, u8 val);
	void crtc_vsync_w(int val);
	virtual MC6845_UPDATE_ROW(crtc_update_row);

	required_device<cpu_device>     m_maincpu;
	required_device<screen_device>  m_screen;
	required_device<palette_device> m_palette;
	required_device<mc6845_device>  m_crtc;
	required_shared_ptr<u8>         m_p_videoram;
	required_region_ptr<u8>         m_p_chargen;
	required_ioport                 m_config;
	required_device<ins8250_device> m_ace;
	required_device<beep_device>    m_beep;
	required_device<mm5740_device>  m_mm5740;
	required_memory_region          m_kbdrom;
	required_ioport                 m_kbspecial;
	required_device<clock_device>   m_repeat_clock;

	emu_timer                      *m_key_click_timer;
	emu_timer                      *m_bell_timer;

	u8                              m_transchar;
	bool                            m_strobe;
	bool                            m_key_click_active;
	bool                            m_bell_active;
	bool                            m_reset_pending;
	bool                            m_right_shift;
	bool                            m_reset_key;
	bool                            m_keyboard_irq_raised;
	bool                            m_serial_irq_raised;
	bool                            m_break_key_irq_raised;
	bool                            m_allow_vsync_nmi;
};

/**
 *  Heath TLB with Super19 ROM
 */
class heath_super19_tlb_device : public heath_tlb_device
{
public:
	heath_super19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
};

/**
 *  Heath TLB with Superset ROM
 */
class heath_superset_tlb_device : public heath_tlb_device
{
public:
	heath_superset_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void mem_map(address_map &map);
	void io_map(address_map &map);

	virtual MC6845_UPDATE_ROW(crtc_update_row) override;

	void dtr_internal(int data);
	void out1_internal(int data);
	void out2_internal(int data);

	virtual u8 crtc_reg_r(offs_t reg) override;
	virtual void crtc_reg_w(offs_t reg, u8 val) override;

	u8   m_selected_char_set;
	bool m_reverse_video_disabled;
};

/**
 *  Heath TLB with Watzman (HUG) ROM
 */
class heath_watz_tlb_device : public heath_tlb_device
{
public:
	heath_watz_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
};

/**
 *  Heath TLB with Ultra ROM
 */
class heath_ultra_tlb_device : public heath_tlb_device
{
public:
	heath_ultra_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	void mem_map(address_map &map);
};

/**
 *  Heath TLB plus Northwest Digital Systems GP-19
 */
class heath_gp19_tlb_device : public heath_tlb_device
{
public:
	heath_gp19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void mem_map(address_map &map);
	void io_map(address_map &map);

	void latch_u5_w(u8 data);

	virtual MC6845_UPDATE_ROW(crtc_update_row) override;

	bool m_char_gen_a11;
	bool m_graphic_mode;
	bool m_col_132;
	bool m_reverse_video;
};

/**
 *  Heath TLB plus Cleveland Codonics Imaginator-100
 */
class heath_imaginator_tlb_device : public heath_tlb_device
{
public:
	heath_imaginator_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void mem_map(address_map &map);
	void io_map(address_map &map);

	virtual MC6845_UPDATE_ROW(crtc_update_row) override;
	void crtc_hsync_w(int val);

	IRQ_CALLBACK_MEMBER(irq_ack_cb);

	void tap_6000h();
	void tap_8000h();

	void allow_tlb_intr();
	void allow_hsync_intr();

	void display_enable_w(offs_t reg, u8 val);
	void config_irq_w(offs_t reg, u8 val);
	void nop_w(offs_t reg, u8 val);
	virtual void set_irq_line() override;

	required_memory_bank            m_mem_bank;
	required_shared_ptr<u8>         m_p_graphic_ram;
	memory_passthrough_handler      m_tap_6000h;

	u8                              m_im2_val;

	bool                            m_alphanumeric_mode_active;
	bool                            m_graphics_mode_active;

	bool                            m_allow_tlb_interrupts;
	bool                            m_allow_imaginator_interrupts;

	bool                            m_hsync_irq_raised;
};

/**
 *  Heath TLB plus SigmaSoft Interactive Graphics Controller
 */
class heath_igc_tlb_device : public heath_tlb_device
{
public:
	heath_igc_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void sigma_ctrl_w(u8 data) override;
	virtual u8 sigma_ctrl_r() override;

	virtual u8 sigma_video_mem_r() override;
	virtual void sigma_video_mem_w(u8 val) override;

	virtual void sigma_io_lo_addr_w(u8 val) override;
	virtual void sigma_io_hi_addr_w(u8 val) override;

	virtual void sigma_window_lo_addr_w(u8 val) override;
	virtual void sigma_window_hi_addr_w(u8 val) override;

protected:
	heath_igc_tlb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual MC6845_UPDATE_ROW(crtc_update_row) override;

	std::unique_ptr<u8[]>   m_p_graphic_ram;

	// Control Register
	bool                    m_pixel_video_enabled;
	bool                    m_character_video_disabled;
	bool                    m_video_invert_enabled;
	bool                    m_alternate_character_set_enabled;
	bool                    m_video_fill_enabled;
	bool                    m_read_address_increment_disabled;
	u8                      m_memory_bank_select;

	u8                      m_data_reg;

	u16                     m_io_address;
	u16                     m_window_address;
};

/**
 *  Heath TLB with the super19 ROM plus SigmaSoft Interactive Graphics Controller
 */
class heath_igc_super19_tlb_device : public heath_igc_tlb_device
{
public:
	heath_igc_super19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
};

/**
 *  Heath TLB with the ultrarom plus SigmaSoft Interactive Graphics Controller
 */
class heath_igc_ultra_tlb_device : public heath_igc_tlb_device
{
public:
	heath_igc_ultra_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	void mem_map(address_map &map);
};

/**
 *  Heath TLB with the watzman ROM plus SigmaSoft Interactive Graphics Controller
 */
class heath_igc_watz_tlb_device : public heath_igc_tlb_device
{
public:
	heath_igc_watz_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
};

// Options for the standard H89 class of systems
DECLARE_DEVICE_TYPE(HEATH_TLB,         heath_tlb_device)
DECLARE_DEVICE_TYPE(HEATH_GP19,        heath_gp19_tlb_device)
DECLARE_DEVICE_TYPE(HEATH_IMAGINATOR,  heath_imaginator_tlb_device)
DECLARE_DEVICE_TYPE(HEATH_SUPER19,     heath_super19_tlb_device)
DECLARE_DEVICE_TYPE(HEATH_SUPERSET,    heath_superset_tlb_device)
DECLARE_DEVICE_TYPE(HEATH_WATZ,        heath_watz_tlb_device)
DECLARE_DEVICE_TYPE(HEATH_ULTRA,       heath_ultra_tlb_device)

// Options for the H89 with the SigmaSoft IGC graphics installed
DECLARE_DEVICE_TYPE(HEATH_IGC,         heath_igc_tlb_device)
DECLARE_DEVICE_TYPE(HEATH_IGC_SUPER19, heath_igc_super19_tlb_device)
DECLARE_DEVICE_TYPE(HEATH_IGC_ULTRA,   heath_igc_ultra_tlb_device)
DECLARE_DEVICE_TYPE(HEATH_IGC_WATZ,    heath_igc_watz_tlb_device)

/**
 * Connector for the Terminal Logic Board in an H-89 class computer
 */
class heath_tlb_connector : public device_t,
							public device_single_card_slot_interface<device_heath_tlb_card_interface>
{
public:

	template <typename T>
	heath_tlb_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, bool fixed = false) :
		 heath_tlb_connector(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}

	heath_tlb_connector(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~heath_tlb_connector();

	// computer interface
	auto serial_data_callback() { return m_write_sd.bind(); }
	auto dtr_callback() { return m_dtr_cb.bind(); }
	auto rts_callback() { return m_rts_cb.bind(); }
	auto reset_cb() { return m_reset.bind(); }

	// card interface
	void serial_in_w(int state) { if (m_tlb) m_tlb->serial_in_w(state); }
	void rlsd_in_w(int state) { if (m_tlb) m_tlb->rlsd_in_w(state); }
	void dsr_in_w(int state) { if (m_tlb) m_tlb->dsr_in_w(state); }
	void cts_in_w(int state) { if (m_tlb) m_tlb->cts_in_w(state); }

	// signals out from the tlb
	void serial_out_b(int data) { m_write_sd(data); }
	void dtr_out(int data) { m_dtr_cb(data); }
	void rts_out(int data) { m_rts_cb(data); }
	void reset_out(int data) { m_reset(data); }

	// optional SigmaSet IGC operations
	u8 sigma_ctrl_r() { return (m_tlb) ? m_tlb->sigma_ctrl_r() : 0x00; }
	void sigma_ctrl_w(u8 data) { if (m_tlb) m_tlb->sigma_ctrl_w(data); }

	u8 sigma_video_mem_r() { return (m_tlb) ? m_tlb->sigma_video_mem_r() : 0x00; }
	void sigma_video_mem_w(u8 val) { if (m_tlb) m_tlb->sigma_video_mem_w(val); }

	void sigma_io_lo_addr_w(u8 val) { if (m_tlb) m_tlb->sigma_io_lo_addr_w(val); }
	void sigma_io_hi_addr_w(u8 val) { if (m_tlb) m_tlb->sigma_io_hi_addr_w(val); }

	void sigma_window_lo_addr_w(u8 val) { if (m_tlb) m_tlb->sigma_window_lo_addr_w(val); }
	void sigma_window_hi_addr_w(u8 val) { if (m_tlb) m_tlb->sigma_window_hi_addr_w(val); }

protected:
	virtual void device_start() override;

	devcb_write_line                 m_write_sd;
	devcb_write_line                 m_dtr_cb;
	devcb_write_line                 m_rts_cb;
	devcb_write_line                 m_reset;

	device_heath_tlb_card_interface *m_tlb;
};


// device type definition
DECLARE_DEVICE_TYPE(HEATH_TLB_CONNECTOR, heath_tlb_connector)

#endif // MAME_HEATHKIT_TLB_H
