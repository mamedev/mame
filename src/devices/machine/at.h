// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic, Carl
#ifndef MAME_MACHINE_AT_H
#define MAME_MACHINE_AT_H

#include "machine/mc146818.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/am9517a.h"
#include "machine/at_keybc.h"
#include "bus/isa/isa.h"
#include "sound/spkrdev.h"
#include "softlist.h"

class at_mb_device : public device_t
{
public:
	at_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void map(address_map &map) ATTR_COLD;

	auto kbd_clk() { return m_keybc.lookup()->kbd_clk(); }
	auto kbd_data() { return m_keybc.lookup()->kbd_data(); }

	uint8_t page8_r(offs_t offset);
	void page8_w(offs_t offset, uint8_t data);
	void kbd_clk_w(int state);
	void kbd_data_w(int state);
	uint8_t portb_r();
	void portb_w(uint8_t data);
	void rtcas_nmi_w(uint8_t data);
	void iochck_w(int state);

	void shutdown(int state);

	uint32_t a20_286(bool state);

	void at_softlists(machine_config &config);
protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void set_dma_channel(int channel, int state);
	void speaker_set_spkrdata(uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_device<isa16_device> m_isabus;
	required_device<pic8259_device> m_pic8259_slave;
	required_device<am9517a_device> m_dma8237_1;
	required_device<am9517a_device> m_dma8237_2;
	required_device<pit8254_device> m_pit8254;
	required_device<speaker_sound_device> m_speaker;
	required_device<mc146818_device> m_mc146818;
	optional_device<at_keyboard_controller_device> m_keybc; // removed in mtouchxl.cpp and vis.cpp

	uint8_t m_at_spkrdata = 0;
	uint8_t m_pit_out2 = 0;
	int m_dma_channel = 0;
	bool m_cur_eop = false, m_cur_eop2 = false;
	uint8_t m_dma_offset[2][4]{};
	uint8_t m_at_pages[0x10]{};
	uint16_t m_dma_high_byte = 0;
	uint8_t m_at_speaker = 0;
	uint8_t m_channel_check = 0;
	uint8_t m_nmi_enabled = 0;

	void pit8254_out2_changed(int state);

	void dma8237_out_eop(int state);
	void dma8237_2_out_eop(int state);
	uint8_t dma8237_0_dack_r();
	uint8_t dma8237_1_dack_r();
	uint8_t dma8237_2_dack_r();
	uint8_t dma8237_3_dack_r();
	uint8_t dma8237_5_dack_r();
	uint8_t dma8237_6_dack_r();
	uint8_t dma8237_7_dack_r();
	void dma8237_0_dack_w(uint8_t data);
	void dma8237_1_dack_w(uint8_t data);
	void dma8237_2_dack_w(uint8_t data);
	void dma8237_3_dack_w(uint8_t data);
	void dma8237_5_dack_w(uint8_t data);
	void dma8237_6_dack_w(uint8_t data);
	void dma8237_7_dack_w(uint8_t data);
	void dack0_w(int state);
	void dack1_w(int state);
	void dack2_w(int state);
	void dack3_w(int state);
	void dack4_w(int state);
	void dack5_w(int state);
	void dack6_w(int state);
	void dack7_w(int state);
	uint8_t get_slave_ack(offs_t offset);
	void dma_hrq_changed(int state);

	uint8_t dma_read_byte(offs_t offset);
	void dma_write_byte(offs_t offset, uint8_t data);
	uint8_t dma_read_word(offs_t offset);
	void dma_write_word(offs_t offset, uint8_t data);
};

DECLARE_DEVICE_TYPE(AT_MB, at_mb_device)


#endif // MAME_MACHINE_AT_H
