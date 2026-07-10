// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_I82378ZB_SIO_H
#define MAME_MACHINE_I82378ZB_SIO_H

#pragma once

#include "pci.h"

#include "bus/isa/isa.h"
#include "cpu/i386/i386.h"
#include "machine/am9517a.h"
#include "machine/at_keybc.h"
#include "machine/idectrl.h"
#include "machine/intelfsh.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "sound/spkrdev.h"

class i82378zb_sio_device : public pci_device
{
public:
	template <typename T>
	i82378zb_sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: i82378zb_sio_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	i82378zb_sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }

	auto boot_state_hook() { return m_boot_state_hook.bind(); }
	auto a20m() { return m_write_a20m.bind(); }
	auto cpureset() { return m_write_cpureset.bind(); }
//	auto pcirst() { return m_write_pcirst.bind(); }

	// X-Bus integrations
	void pc_irq1_w(int state);
	void pc_irq14_w(int state);
	void pc_irq15_w(int state);

	void cpu_a20_w(int state);
	void cpu_reset_w(int state);

	auto rtcale() { return m_rtcale.bind(); }
	auto rtccs_read() { return m_rtccs_read.bind(); }
	auto rtccs_write() { return m_rtccs_write.bind(); }

protected:
	i82378zb_sio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config & config) override;
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual bool map_first() const override { return true; }

	virtual void config_map(address_map &map) override ATTR_COLD;
	void internal_io_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_host_cpu;
	required_device_array<pic8259_device, 2> m_pic;
	required_device_array<am9517a_device, 2> m_dma;
	required_device<pit8254_device> m_pit;

	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	optional_device<intelfsh8_device> m_xbus_flash;
	optional_device<at_keyboard_controller_device> m_xbus_keybc;
	optional_device_array<ide_controller_32_device, 2> m_xbus_ide;

	devcb_write8 m_boot_state_hook;
	devcb_write_line m_write_a20m;
	devcb_write_line m_write_cpureset;

	devcb_write8 m_rtcale;
	devcb_read8 m_rtccs_read;
	devcb_write8 m_rtccs_write;

	struct {
		bool flash_bios;
		bool keyboard;
		bool ide;
	} m_has_xbus;

	void map_bios(address_space *memory_space, uint32_t start, uint32_t end);

	u8 m_pcicon;
	u8 m_pac;
	u8 m_papc;
	u8 m_arbprix;
	u8 m_mcscon;
	u8 m_mcsboh;
	u8 m_mcstoh;
	u8 m_mcstom;
	u8 m_iadcon;
	u8 m_iadrbe;
	u8 m_iadboh;
	u8 m_iadtoh;
	u8 m_icrt;
	u8 m_icd;
	u8 m_ubcsa;
	u8 m_ubcsb;
	u8 m_mar[3];
	u8 m_pirq[4];
	u16 m_bios_timer_base;
	u8 m_smicntl;
	u16 m_smien;
	u32 m_see;
	u8 m_ftmr;
	u16 m_smireq;
	u8 m_ctltmr;
	u8 m_ctltmrh;

	void rtc_index_w(offs_t offset, u8 data);
	u8 rtc_data_r(offs_t offset);
	void rtc_data_w(offs_t offset, u8 data);

	u8 m_port92;
	int m_ext_gatea20;
	int m_fast_gatea20;

	u8 port92_r(offs_t offset);
	void port92_w(offs_t offset, u8 data);
	void emulated_kbreset(int state);
	void emulated_gatea20(int state);
	void fast_gatea20(int state);
	void keyboard_gatea20(int state);

	// Southbridge common stuff
	void at_pit8254_out0_changed(int state);
	void at_pit8254_out1_changed(int state);
	void at_pit8254_out2_changed(int state);
	uint8_t pc_dma8237_0_dack_r();
	uint8_t pc_dma8237_1_dack_r();
	uint8_t pc_dma8237_2_dack_r();
	uint8_t pc_dma8237_3_dack_r();
	uint8_t pc_dma8237_5_dack_r();
	uint8_t pc_dma8237_6_dack_r();
	uint8_t pc_dma8237_7_dack_r();
	void pc_dma8237_0_dack_w(uint8_t data);
	void pc_dma8237_1_dack_w(uint8_t data);
	void pc_dma8237_2_dack_w(uint8_t data);
	void pc_dma8237_3_dack_w(uint8_t data);
	void pc_dma8237_5_dack_w(uint8_t data);
	void pc_dma8237_6_dack_w(uint8_t data);
	void pc_dma8237_7_dack_w(uint8_t data);
	void pc_dack0_w(int state);
	void pc_dack1_w(int state);
	void pc_dack2_w(int state);
	void pc_dack3_w(int state);
	void pc_dack4_w(int state);
	void pc_dack5_w(int state);
	void pc_dack6_w(int state);
	void pc_dack7_w(int state);
	void at_dma8237_out_eop(int state);
	void pc_dma_hrq_changed(int state);
	uint8_t pc_dma_read_byte(offs_t offset);
	void pc_dma_write_byte(offs_t offset, uint8_t data);
	uint8_t pc_dma_read_word(offs_t offset);
	void pc_dma_write_word(offs_t offset, uint8_t data);
	uint8_t at_portb_r();
	void at_portb_w(uint8_t data);
	void at_speaker_set_spkrdata(uint8_t data);
	uint8_t get_slave_ack(offs_t offset);
	void pc_irq3_w(int state);
	void pc_irq4_w(int state);
	void pc_irq5_w(int state);
	void pc_irq6_w(int state);
	void pc_irq7_w(int state);
	void pc_irq8n_w(int state);
	void pc_irq9_w(int state);
	void pc_irq10_w(int state);
	void pc_irq11_w(int state);
	void pc_irq12m_w(int state);

//	void pc_pirqa_w(int state);
//	void pc_pirqb_w(int state);
//	void pc_pirqc_w(int state);
//	void pc_pirqd_w(int state);
//	void pc_mirq0_w(int state);
//	void pc_mirq1_w(int state);
//	void pc_mirq2_w(int state);

	void iochck_w(int state);
	void pc_select_dma_channel(int channel, bool state);
	uint8_t at_page8_r(offs_t offset);
	void at_page8_w(offs_t offset, uint8_t data);
	uint8_t at_dma8237_2_r(offs_t offset);
	void at_dma8237_2_w(offs_t offset, uint8_t data);

	uint8_t m_at_spkrdata = 0;
	uint8_t m_pit_out2 = 0;
	uint8_t m_at_speaker = 0;
	bool m_refresh = false;
	int m_dma_channel = 0;
	bool m_cur_eop = false;
	uint8_t m_dma_offset[2][4];
	uint8_t m_at_pages[0x10]{};
	uint16_t m_dma_high_byte = 0;
	uint8_t m_channel_check = 0;
	bool m_nmi_enabled = false;
};


DECLARE_DEVICE_TYPE(I82378ZB_SIO, i82378zb_sio_device)
//DECLARE_DEVICE_TYPE(I82379AB_SIOA, i82379ab_sio_device)


#endif // MAME_MACHINE_I82378ZB_SIO_H
