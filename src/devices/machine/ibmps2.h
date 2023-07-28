// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic, Carl
#ifndef MAME_MACHINE_PS2_H
#define MAME_MACHINE_PS2_H

#include "machine/timer.h"
#include "machine/am9517a.h"
#include "machine/at_keybc.h"
#include "machine/ins8250.h"
#include "machine/mc146818.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/pc_lpt.h"
#include "machine/ibm72x7377.h"
#include "bus/mca/mca.h"
#include "bus/mca/ibm72x8299.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "sound/spkrdev.h"

class ps2_mb_device : public device_t
{
public:
	ps2_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
		uint16_t sram_size, uint16_t planar_id, bool supports_fast_refresh)
		: ps2_mb_device(mconfig, tag, owner, clock)
	{
		m_sram_size = sram_size;
		m_planar_id = planar_id;
		m_supports_fast_refresh = supports_fast_refresh;
	}
	ps2_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map);

	// Keyboard Controller
	auto 	kbd_clk() { return subdevice<ps2_keyboard_controller_device>("keybc")->kbd_clk(); }
	auto 	kbd_data() { return subdevice<ps2_keyboard_controller_device>("keybc")->kbd_data(); }
	void 	write_rtc(offs_t offset, uint8_t data);
	
    void    post_debug_w(offs_t offset, uint8_t data);

    uint8_t memory_control_r();
    void    memory_control_w(uint8_t data);

    uint8_t nvram_r(offs_t offset);
    void    nvram_w(offs_t offset, uint8_t data);

	virtual uint8_t 	planar_pos_r(offs_t offset);
	virtual void 		planar_pos_w(offs_t offset, uint8_t data);

	mca16_device * get_mca_bus() { return m_mcabus; }
	mca16_slot_device * get_mca_slot(uint8_t slotnum) { return m_mcaslot[slotnum]; }

	uint16_t get_planar_id() { return m_planar_id; }

	template<typename R, typename W> void install_memory(offs_t start, offs_t end, R rhandler, W whandler)
	{
		m_mcabus->install_memory(start, end, rhandler, whandler);
	}

	template <typename T> void set_cpu_tag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_ram_tag(T &&tag) { m_ram.set_tag(std::forward<T>(tag)); }

	void iochck_w(int state);
	void shutdown(int state);

	virtual IRQ_CALLBACK_MEMBER(inta_cb);

protected:
	required_device<cpu_device> m_maincpu;
    required_device<ram_device> m_ram;
	optional_device<nvram_device> m_nvram;
	required_device<mca16_device> m_mcabus;
	required_device<pic8259_device> m_pic8259_master;
	required_device<pic8259_device> m_pic8259_slave;
	optional_device<ibm72x7377_device> m_dmac;
	required_device<speaker_sound_device> m_speaker;
	required_device<mc146818_device> m_mc146818;
	required_device<ps2_keyboard_controller_device> m_keybc;
    required_device<pc_kbdc_device> m_ps2_con;
	required_device<pc_kbdc_device> m_aux_con;
	required_device<timer_device> m_timer_slow_refresh;
	optional_device<timer_device> m_timer_fast_refresh;
	optional_device<ibm72x8299_device> m_io_controller;

	optional_device_array<mca16_slot_device, 8> m_mcaslot;

	required_memory_region m_bios;

	void add_southbridge_72x8299(machine_config &config);
	void add_dmac_72x7377(machine_config &config);

	void ps2_32_map(address_map& map);
	void ps2_32_io(address_map& map);

	void ps2_16_map(address_map& map);
	void ps2_16_io(address_map& map);

	void device_start() override;
	void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
    virtual void device_config_complete() override;

	void add_mca_common(machine_config &config);
	void add_mca16(machine_config &config);
	void add_mca32(machine_config &config);

	void speaker_set_spkrdata(uint8_t data);

	uint8_t get_slave_ack(offs_t offset);
	uint8_t keybc_data_r();

	// System Control Ports A and B
	uint8_t	porta_r();
	void	porta_w(uint8_t data);

	virtual uint8_t portb_r();
	virtual void portb_w(uint8_t data);

	virtual void irq0_latch_reset();

	uint16_t m_sram_size;
	uint16_t m_planar_id;

	std::unique_ptr<uint8_t[]> m_sram;
    uint16_t m_nvram_index;

	uint8_t m_at_spkrdata;
	uint8_t m_pit_out2;
	bool m_write_gate_2;
	
	uint8_t m_at_speaker;
	uint8_t m_channel_check;
	uint8_t m_nmi_enabled;
	uint8_t m_parity_check_enabled;
	uint8_t m_channel_check_enabled;
	uint8_t m_memory_control;
	bool m_gate_a20;
	bool m_nmi_flag;
	bool m_alt_hot_reset;
	bool m_refresh_bit;
	bool m_refresh_state;
	bool m_supports_fast_refresh;

	void pit8254_out2_changed(int state);
	void irq0_w(int state);
	void watchdog_w(int state);
	void gate_a20_w(int state);
	void hot_reset_w(int state);
	void keybc_irq_latch_w(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(refresh_cb);
};

#endif // MAME_MACHINE_PS2_H
