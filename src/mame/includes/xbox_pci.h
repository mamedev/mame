// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
#ifndef MAME_INCLUDES_XBOX_PCI_H
#define MAME_INCLUDES_XBOX_PCI_H

#pragma once

#include "machine/pit8253.h"
#include "xbox_nv2a.h"
#include "xbox_usb.h"

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
	virtual void device_start() override;
	virtual void device_reset() override;

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

	virtual void config_map(address_map &map) override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

protected:
	virtual void device_start() override;

	uint32_t config_register_r();
	void config_register_w(uint32_t data);

private:
	int ram_size;
	std::vector<uint32_t> ram;
};

DECLARE_DEVICE_TYPE(NV2A_RAM, nv2a_ram_device)

/*
 * LPC Bus
 */

class lpcbus_host_interface {
public:
	virtual void set_virtual_line(int line, int state) = 0;
	virtual void remap() = 0;
};

class lpcbus_device_interface {
public:
	virtual void map_extra(address_space *memory_space, address_space *io_space) = 0;
	virtual void set_host(int index, lpcbus_host_interface *host) = 0;
};

class mcpx_isalpc_device : public pci_device, public lpcbus_host_interface {
public:
	mcpx_isalpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id);
	mcpx_isalpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto smi() { return m_smi_callback.bind(); }
	auto interrupt_output() { return m_interrupt_output.bind(); }
	auto boot_state_hook() { return m_boot_state_hook.bind(); }

	uint32_t acknowledge();
	void debug_generate_irq(int irq, int state);

	virtual void set_virtual_line(int line, int state) override;
	virtual void remap() override;

	uint32_t acpi_r(offs_t offset, uint32_t mem_mask = ~0);
	void acpi_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void boot_state_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(irq1);
	DECLARE_WRITE_LINE_MEMBER(irq3);
	DECLARE_WRITE_LINE_MEMBER(irq11);
	DECLARE_WRITE_LINE_MEMBER(irq10);
	DECLARE_WRITE_LINE_MEMBER(irq14);
	DECLARE_WRITE_LINE_MEMBER(irq15);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	DECLARE_WRITE_LINE_MEMBER(interrupt_ouptut_changed);
	uint8_t get_slave_ack(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(pit8254_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(pit8254_out1_changed);
	DECLARE_WRITE_LINE_MEMBER(pit8254_out2_changed);

private:
	void internal_io_map(address_map &map);
	void lpc_io(address_map &map);
	void update_smi_line();
	void speaker_set_spkrdata(uint8_t data);

	uint8_t portb_r();
	void portb_w(uint8_t data);

	devcb_write_line m_smi_callback;
	devcb_write_line m_interrupt_output;
	devcb_write8 m_boot_state_hook;
	required_device<pic8259_device> pic8259_1;
	required_device<pic8259_device> pic8259_2;
	required_device<pit8254_device> pit8254;

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
	uint8_t m_speaker;
	bool m_refresh;
	uint8_t m_pit_out2;
	uint8_t m_spkrdata;
	uint8_t m_channel_check;
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
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void config_map(address_map &map) override;

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
	void smbus_io0(address_map &map);
	void smbus_io1(address_map &map);
	void smbus_io2(address_map &map);
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
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_config_complete() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	virtual void config_map(address_map &map) override;

private:
	ohci_usb_controller *ohci_usb;
	devcb_write_line m_interrupt_handler;
	emu_timer *timer;
	required_device<cpu_device> maincpu;
	std::function<void(void)> hack_callback;
	void ohci_mmio(address_map &map);
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
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void eth_mmio(address_map &map);
	void eth_io(address_map &map);
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
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	virtual void config_map(address_map &map) override;

private:
	required_device<device_memory_interface> cpu;
	// APU contains 3 dsps: voice processor (VP) global processor (GP) encode processor (EP)
	struct apu_state {
		uint32_t memory[0x60000 / 4];
		uint32_t gpdsp_sgaddress; // global processor scatter-gather
		uint32_t gpdsp_sgblocks;
		uint32_t gpdsp_address;
		uint32_t epdsp_sgaddress; // encoder processor scatter-gather
		uint32_t epdsp_sgblocks;
		uint32_t epdsp_sgaddress2;
		uint32_t epdsp_sgblocks2;
		int voice_number;
		uint32_t voices_heap_blockaddr[1024];
		uint64_t voices_active[4]; //one bit for each voice: 1 playing 0 not
		uint32_t voicedata_address;
		int voices_frequency[256]; // sample rate
		int voices_position[256]; // position in samples * 1000
		int voices_position_start[256]; // position in samples * 1000
		int voices_position_end[256]; // position in samples * 1000
		int voices_position_increment[256]; // position increment every 1ms * 1000
		emu_timer *timer;
		address_space *space;
	} apust;
	void apu_mmio(address_map &map);
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
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void config_map(address_map &map) override;

private:
	struct ac97_state {
		uint32_t mixer_regs[0x84 / 4];
		uint32_t controller_regs[0x40 / 4];
	} ac97st;
	void ac97_mmio(address_map &map);
	void ac97_io0(address_map &map);
	void ac97_io1(address_map &map);
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

	auto pri_interrupt_handler() { return m_pri_interrupt_handler.bind(); }
	auto sec_interrupt_handler() { return m_sec_interrupt_handler.bind(); }

	void class_rev_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t pri_read_cs1_r();
	void pri_write_cs1_w(uint8_t data);
	uint8_t sec_read_cs1_r();
	void sec_write_cs1_w(uint8_t data);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

private:
	required_device<bus_master_ide_controller_device> m_pri;
	required_device<bus_master_ide_controller_device> m_sec;
	devcb_write_line m_pri_interrupt_handler;
	devcb_write_line m_sec_interrupt_handler;
	void ide_pri_command(address_map &map);
	void ide_pri_control(address_map &map);
	void ide_sec_command(address_map &map);
	void ide_sec_control(address_map &map);
	void ide_io(address_map &map);
	DECLARE_WRITE_LINE_MEMBER(ide_pri_interrupt);
	DECLARE_WRITE_LINE_MEMBER(ide_sec_interrupt);
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
	virtual void device_start() override;
	virtual void device_reset() override;
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
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	nv2a_renderer *nvidia_nv2a;
	required_device<device_memory_interface> cpu;
	devcb_write_line m_interrupt_handler;
	address_space *m_program;
	void nv2a_mmio(address_map &map);
	void nv2a_mirror(address_map &map);
};

DECLARE_DEVICE_TYPE(NV2A_GPU, nv2a_gpu_device)

#endif // MAME_INCLUDES_XBOX_PCI_H
