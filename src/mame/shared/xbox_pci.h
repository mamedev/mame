// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
#ifndef MAME_SHARED_XBOX_PCI_H
#define MAME_SHARED_XBOX_PCI_H

#pragma once

#include "xbox_nv2a.h"
#include "xbox_usb.h"

#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ds128x.h"
#include "machine/am9517a.h"

/*
 * Host
 */

class nv2a_host_device : public pci_host_device {
public:
	template <typename T>
	nv2a_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: nv2a_host_device(mconfig, tag, owner, clock)
	{
		set_ids_host(0x10de02a5, 0, 0);
		set_cpu_tag(std::forward<T>(cpu_tag));
	}
	nv2a_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
			uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	template <typename T> void set_cpu_tag(T &&cpu_tag) { cpu.set_tag(std::forward<T>(cpu_tag)); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<device_memory_interface> cpu;
};

DECLARE_DEVICE_TYPE(NV2A_HOST, nv2a_host_device)

/*
 * Ram
 */

class nv2a_ram_device : public pci_device {
public:
	nv2a_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, int memory_size)
		: nv2a_ram_device(mconfig, tag, owner, clock)
	{
		ram_size = memory_size;
	}
	nv2a_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

protected:
	virtual void device_start() override ATTR_COLD;

	uint32_t config_register_r();
	void config_register_w(uint32_t data);

private:
	int ram_size = 0;
	std::vector<uint32_t> ram;
};

DECLARE_DEVICE_TYPE(NV2A_RAM, nv2a_ram_device)

/*
 * LPC Bus
 */

class lpcbus_host_interface {
public:
	virtual void set_virtual_line(int line, int state) = 0;
	virtual void assign_virtual_line(int line, int device_index) = 0;
	virtual void remap() = 0;
};

class lpcbus_device_interface {
public:
	enum class dma_operation {
		READ = 1,
		WRITE = 2,
		END = 4
	};
	enum class dma_size {
		BYTE = 1,
		WORD = 2,
		DWORD = 4
	};
	virtual void map_extra(address_space *memory_space, address_space *io_space) = 0;
	virtual void set_host(int device_index, lpcbus_host_interface *host) = 0;
	virtual uint32_t dma_transfer(int channel, dma_operation operation, dma_size size, uint32_t data) = 0;
};

class mcpx_isalpc_device : public pci_device, public lpcbus_host_interface {
public:
	mcpx_isalpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id);
	mcpx_isalpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> void set_dma_space(T &&bmtag, int bmspace) { m_dma_space.set_tag(std::forward<T>(bmtag), bmspace); }
	template <bool R> void set_dma_space(const address_space_finder<R> &finder) { m_dma_space.set_tag(finder); }

	auto smi() { return m_smi_callback.bind(); }
	auto interrupt_output() { return m_interrupt_output.bind(); }
	auto boot_state_hook() { return m_boot_state_hook.bind(); }

	uint32_t acknowledge();
	void debug_generate_irq(int irq, int state);

	virtual void set_virtual_line(int line, int state) override;
	virtual void assign_virtual_line(int line, int device_index) override;
	virtual void remap() override;

	uint32_t acpi_r(offs_t offset, uint32_t mem_mask = ~0);
	void acpi_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void boot_state_w(uint8_t data);

	uint8_t dma_page_r(offs_t offset);
	void dma_page_w(offs_t offset, uint8_t data);
	uint8_t dma_read_byte(offs_t offset);
	void dma_write_byte(offs_t offset, uint8_t data);
	uint8_t dma_read_word(offs_t offset);
	void dma_write_word(offs_t offset, uint8_t data);
	void set_dma_channel(int channel, bool state);
	void send_dma_byte(int channel, uint8_t value);
	void send_dma_word(int channel, uint16_t value);
	uint8_t get_dma_byte(int channel);
	uint16_t get_dma_word(int channel);
	void signal_dma_end(int channel, int tc);
	uint8_t dma2_r(offs_t offset) { return m_dma8237_2->read(offset / 2); }
	void dma2_w(offs_t offset, uint8_t data) { m_dma8237_2->write(offset / 2, data); }
	uint8_t dma1_ior0_r() { return get_dma_byte(0); }
	uint8_t dma1_ior1_r() { return get_dma_byte(1); }
	uint8_t dma1_ior2_r() { return get_dma_byte(2); }
	uint8_t dma1_ior3_r() { return get_dma_byte(3); }
	uint8_t dma2_ior1_r() { uint16_t const result = get_dma_word(5); m_dma_high_byte = result >> 8; return result; }
	uint8_t dma2_ior2_r() { uint16_t const result = get_dma_word(6); m_dma_high_byte = result >> 8; return result; }
	uint8_t dma2_ior3_r() { uint16_t const result = get_dma_word(7); m_dma_high_byte = result >> 8; return result; }
	void dma1_iow0_w(uint8_t data) { send_dma_byte(0, data); }
	void dma1_iow1_w(uint8_t data) { send_dma_byte(1, data); }
	void dma1_iow2_w(uint8_t data) { send_dma_byte(2, data); }
	void dma1_iow3_w(uint8_t data) { send_dma_byte(3, data); }
	void dma2_iow1_w(uint8_t data) { send_dma_word(5, (m_dma_high_byte << 8) | data); }
	void dma2_iow2_w(uint8_t data) { send_dma_word(6, (m_dma_high_byte << 8) | data); }
	void dma2_iow3_w(uint8_t data) { send_dma_word(7, (m_dma_high_byte << 8) | data); }

	void irq1(int state);
	void irq3(int state);
	void irq11(int state);
	void irq10(int state);
	void irq14(int state);
	void irq15(int state);
	void dma2_hreq_w(int state) { m_dma8237_2->hack_w(state); }
	void dma1_eop_w(int state) { m_dma_eop = state; if (m_dma_channel != -1) signal_dma_end(m_dma_channel, state & 1); }
	void dma1_dack0_w(int state) { set_dma_channel(0, state); }
	void dma1_dack1_w(int state) { set_dma_channel(1, state); }
	void dma1_dack2_w(int state) { set_dma_channel(2, state); }
	void dma1_dack3_w(int state) { set_dma_channel(3, state); }
	void dma2_dack0_w(int state) { m_dma8237_1->hack_w(state ? 0 : 1); }
	void dma2_dack1_w(int state) { set_dma_channel(5, state); }
	void dma2_dack2_w(int state) { set_dma_channel(6, state); }
	void dma2_dack3_w(int state) { set_dma_channel(7, state); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	uint8_t get_slave_ack(offs_t offset);
	void interrupt_ouptut_changed(int state);
	void pit8254_out0_changed(int state);
	void pit8254_out1_changed(int state);
	void pit8254_out2_changed(int state);

private:
	void internal_io_map(address_map &map) ATTR_COLD;
	void lpc_io(address_map &map) ATTR_COLD;
	void update_smi_line();
	void speaker_set_spkrdata(uint8_t data);

	uint8_t portb_r();
	void portb_w(uint8_t data);

	devcb_write_line m_smi_callback;
	devcb_write_line m_interrupt_output;
	devcb_write8 m_boot_state_hook;
	required_device<pic8259_device> pic8259_1;
	required_device<pic8259_device> pic8259_2;
	required_device<am9517a_device> m_dma8237_1;
	required_device<am9517a_device> m_dma8237_2;
	required_device<pit8254_device> pit8254;
	required_address_space m_dma_space;

	uint16_t m_pm1_status;
	uint16_t m_pm1_enable;
	uint16_t m_pm1_control;
	uint16_t m_pm1_timer;
	uint16_t m_gpe0_status;
	uint16_t m_gpe0_enable;
	uint16_t m_global_smi_control;
	uint8_t m_smi_command_port;
	uint8_t m_gpio_mode[26];
	lpcbus_device_interface *lpcdevices[16];
	int m_lineowners[8];
	uint8_t m_speaker;
	bool m_refresh;
	uint8_t m_pit_out2;
	uint8_t m_spkrdata;
	uint8_t m_channel_check;
	int m_dma_eop;
	uint8_t m_dma_page[0x10];
	uint8_t m_dma_high_byte;
	int m_dma_channel;
	offs_t m_page_offset;
};

DECLARE_DEVICE_TYPE(MCPX_ISALPC, mcpx_isalpc_device)

/*
 * SMBus
 */

class smbus_interface {
public:
	virtual int execute_command(int command, int rw, int data) = 0;
};

class mcpx_smbus_device : public pci_device {
public:
	mcpx_smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id);
	mcpx_smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto interrupt_handler() { return m_interrupt_handler.bind(); }

	uint32_t smbus0_r(offs_t offset, uint32_t mem_mask = ~0);
	void smbus0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t smbus1_r(offs_t offset, uint32_t mem_mask = ~0);
	void smbus1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	devcb_write_line m_interrupt_handler;
	struct smbus_state {
		int status;
		int control;
		int address;
		int data;
		int command;
		int rw;
		smbus_interface *devices[128];
		uint32_t words[256 / 4];
	} smbusst[2];
	void smbus_io0(address_map &map) ATTR_COLD;
	void smbus_io1(address_map &map) ATTR_COLD;
	void smbus_io2(address_map &map) ATTR_COLD;
	uint32_t smbus_read(int bus, offs_t offset, uint32_t mem_mask);
	void smbus_write(int bus, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint8_t minimum_grant_r() { return 3; }
	uint8_t maximum_latency_r() { return 1; }
};

DECLARE_DEVICE_TYPE(MCPX_SMBUS, mcpx_smbus_device)

/*
 * OHCI USB Controller
 */
class usb_function_device;
class mcpx_ohci_device : public pci_device {
public:
	mcpx_ohci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id);
	mcpx_ohci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void set_hack_callback(std::function<void(void)> hack) { hack_callback = hack; }
	void plug_usb_device(int port, device_usb_ohci_function_interface *function);

	auto interrupt_handler() { return m_interrupt_handler.bind(); }

	uint32_t ohci_r(offs_t offset);
	void ohci_w(offs_t offset, uint32_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_config_complete() override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(usb_update);

private:
	ohci_usb_controller *ohci_usb;
	devcb_write_line m_interrupt_handler;
	emu_timer *timer;
	required_device<cpu_device> maincpu;
	std::function<void(void)> hack_callback;
	void ohci_mmio(address_map &map) ATTR_COLD;
	struct dev_t {
		device_usb_ohci_function_interface *dev;
		int port;
	} connecteds[4];
	int connecteds_count;
	uint8_t minimum_grant_r() { return 3; }
	uint8_t maximum_latency_r() { return 1; }
};

DECLARE_DEVICE_TYPE(MCPX_OHCI, mcpx_ohci_device)

/*
 * Ethernet
 */

class mcpx_eth_device : public pci_device {
public:
	mcpx_eth_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t eth_r();
	void eth_w(uint32_t data);
	uint32_t eth_io_r();
	void eth_io_w(uint32_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void eth_mmio(address_map &map) ATTR_COLD;
	void eth_io(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(MCPX_ETH, mcpx_eth_device)

/*
 * Audio Processing Unit
 */

class mcpx_apu_device : public pci_device {
public:
	template <typename T>
	mcpx_apu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id, T &&cpu_tag)
		: mcpx_apu_device(mconfig, tag, owner, clock)
	{
		set_ids(0x10de01b0, 0xc2, 0x040100, subsystem_id);
		set_cpu_tag(std::forward<T>(cpu_tag));
	}
	mcpx_apu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> void set_cpu_tag(T &&cpu_tag) { cpu.set_tag(std::forward<T>(cpu_tag)); }

	uint32_t apu_r(offs_t offset, uint32_t mem_mask = ~0);
	void apu_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(audio_update);

private:
	required_device<device_memory_interface> cpu;
	// APU contains 3 dsps: voice processor (VP) global processor (GP) encode processor (EP)
	struct apu_state {
		uint32_t memory[0x60000 / 4]{};
		uint32_t gpdsp_sgaddress = 0; // global processor scatter-gather
		uint32_t gpdsp_sgblocks = 0;
		uint32_t gpdsp_address = 0;
		uint32_t epdsp_sgaddress = 0; // encoder processor scatter-gather
		uint32_t epdsp_sgblocks = 0;
		uint32_t epdsp_sgaddress2 = 0;
		uint32_t epdsp_sgblocks2 = 0;
		int voice_number = 0;
		uint32_t voices_heap_blockaddr[1024]{};
		uint64_t voices_active[4]{}; //one bit for each voice: 1 playing 0 not
		uint32_t voicedata_address = 0;
		int voices_frequency[256]{}; // sample rate
		int voices_position[256]{}; // position in samples * 1000
		int voices_position_start[256]{}; // position in samples * 1000
		int voices_position_end[256]{}; // position in samples * 1000
		int voices_position_increment[256]{}; // position increment every 1ms * 1000
		emu_timer *timer = nullptr;
		address_space *space = nullptr;
	} apust;
	void apu_mmio(address_map &map) ATTR_COLD;
	uint8_t minimum_grant_r() { return 1; }
	uint8_t maximum_latency_r() { return 0xc; }
};

DECLARE_DEVICE_TYPE(MCPX_APU, mcpx_apu_device)

/*
 * AC97 Audio Controller
 */

class mcpx_ac97_audio_device : public pci_device {
public:
	mcpx_ac97_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id);
	mcpx_ac97_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t ac97_audio_r(offs_t offset, uint32_t mem_mask = ~0);
	void ac97_audio_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ac97_audio_io0_r();
	void ac97_audio_io0_w(uint32_t data);
	uint32_t ac97_audio_io1_r();
	void ac97_audio_io1_w(uint32_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	struct ac97_state {
		uint32_t mixer_regs[0x84 / 4];
		uint32_t controller_regs[0x40 / 4];
	} ac97st;
	void ac97_mmio(address_map &map) ATTR_COLD;
	void ac97_io0(address_map &map) ATTR_COLD;
	void ac97_io1(address_map &map) ATTR_COLD;
	uint8_t minimum_grant_r() { return 2; }
	uint8_t maximum_latency_r() { return 5; }
};

DECLARE_DEVICE_TYPE(MCPX_AC97_AUDIO, mcpx_ac97_audio_device)

/*
 * AC97 Modem Controller
 */

class mcpx_ac97_modem_device : public pci_device {
public:
	mcpx_ac97_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(MCPX_AC97_MODEM, mcpx_ac97_modem_device)

/*
 * IDE Controller
 */

class mcpx_ide_device : public pci_device {
public:
	mcpx_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id);
	mcpx_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> void set_bus_master_space(T &&bmtag, int bmspace)
	{
		m_pri.lookup()->set_bus_master_space(bmtag, bmspace);
		m_sec.lookup()->set_bus_master_space(bmtag, bmspace);
	}
	template <bool R> void set_bus_master_space(const address_space_finder<R> &finder)
	{
		m_pri.lookup()->set_bus_master_space(finder);
		m_sec.lookup()->set_bus_master_space(finder);
	}

	auto pri_interrupt_handler() { return m_pri_interrupt_handler.bind(); }
	auto sec_interrupt_handler() { return m_sec_interrupt_handler.bind(); }

	void class_rev_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t pri_read_cs1_r();
	void pri_write_cs1_w(uint8_t data);
	uint8_t sec_read_cs1_r();
	void sec_write_cs1_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	required_device<bus_master_ide_controller_device> m_pri;
	required_device<bus_master_ide_controller_device> m_sec;
	devcb_write_line m_pri_interrupt_handler;
	devcb_write_line m_sec_interrupt_handler;
	void ide_pri_command(address_map &map) ATTR_COLD;
	void ide_pri_control(address_map &map) ATTR_COLD;
	void ide_sec_command(address_map &map) ATTR_COLD;
	void ide_sec_control(address_map &map) ATTR_COLD;
	void ide_io(address_map &map) ATTR_COLD;
	void ide_pri_interrupt(int state);
	void ide_sec_interrupt(int state);
	uint8_t minimum_grant_r() { return 3; }
	uint8_t maximum_latency_r() { return 1; }
};

DECLARE_DEVICE_TYPE(MCPX_IDE, mcpx_ide_device)


/*
 * AGP Bridge
 */

class nv2a_agp_device : public agp_bridge_device {
public:
	nv2a_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t main_id, uint32_t revision)
		: nv2a_agp_device(mconfig, tag, owner, clock)
	{
		set_ids_bridge(main_id, revision);
	}
	nv2a_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void config_map(address_map& map) override;

	uint32_t unknown_r(offs_t offset, uint32_t mem_mask = ~0);
	void unknown_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(NV2A_AGP, nv2a_agp_device)

/*
 * NV2A 3D Accelerator
 */

class nv2a_gpu_device : public agp_device {
public:
	template <typename T>
	nv2a_gpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: nv2a_gpu_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}
	nv2a_gpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> void set_cpu_tag(T &&cpu_tag) { cpu.set_tag(std::forward<T>(cpu_tag)); }
	nv2a_renderer *debug_get_renderer() { return nvidia_nv2a; }

	auto interrupt_handler() { return m_interrupt_handler.bind(); }

	uint32_t geforce_r(offs_t offset, uint32_t mem_mask = ~0);
	void geforce_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t nv2a_mirror_r(offs_t offset, uint32_t mem_mask = ~0);
	void nv2a_mirror_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	nv2a_renderer *nvidia_nv2a;
	required_device<device_memory_interface> cpu;
	devcb_write_line m_interrupt_handler;
	address_space *m_program;
	void nv2a_mmio(address_map &map) ATTR_COLD;
	void nv2a_mirror(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(NV2A_GPU, nv2a_gpu_device)

#endif // MAME_SHARED_XBOX_PCI_H
