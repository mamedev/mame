// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli

#pragma once

#include "xbox_usb.h"

class xbox_base_state : public driver_device
{
public:
	xbox_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		nvidia_nv2a(nullptr),
		debug_irq_active(false),
		debug_irq_number(0),
		usb_hack_enabled(false),
		m_maincpu(*this, "maincpu") { }

	uint32_t geforce_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void geforce_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t smbus_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void smbus_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t smbus2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void smbus2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t networkio_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void networkio_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t superio_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void superio_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t superiors232_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void superiors232_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t audio_apu_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void audio_apu_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t audio_ac93_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void audio_ac93_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t dummy_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void dummy_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t ohci_usb_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void ohci_usb_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t ohci_usb2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void ohci_usb2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t network_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void network_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	void smbus_register_device(int address, int(*handler)(xbox_base_state &chs, int command, int rw, int data));
	int smbus_pic16lc(int command, int rw, int data);
	int smbus_cx25871(int command, int rw, int data);
	int smbus_eeprom(int command, int rw, int data);
	void debug_generate_irq(int irq, bool active);
	virtual void hack_eeprom() {};
	virtual void hack_usb() {};

	void vblank_callback(screen_device &screen, bool state);
	uint32_t screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	void xbox_pic8259_1_set_int_line(int state);
	uint8_t get_slave_ack(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void xbox_pit8254_out0_changed(int state);
	void xbox_pit8254_out2_changed(int state);
	void xbox_ohci_usb_interrupt_changed(int state);
	int irq_callback(device_t &device, int irqline);
	void audio_apu_timer(void *ptr, int32_t param);

	struct xbox_devices {
		pic8259_device    *pic8259_1;
		pic8259_device    *pic8259_2;
		bus_master_ide_controller_device    *ide;
	} xbox_base_devs;
	struct smbus_state {
		int status;
		int control;
		int address;
		int data;
		int command;
		int rw;
		int(*devices[128])(xbox_base_state &chs, int command, int rw, int data);
		uint32_t words[256 / 4];
	} smbusst;
	struct apu_state {
		uint32_t memory[0x60000 / 4];
		uint32_t gpdsp_sgaddress; // global processor scatter-gather
		uint32_t gpdsp_sgblocks;
		uint32_t gpdsp_address;
		uint32_t epdsp_sgaddress; // encoder processor scatter-gather
		uint32_t epdsp_sgblocks;
		uint32_t unknown_sgaddress;
		uint32_t unknown_sgblocks;
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
	struct ac97_state {
		uint32_t mixer_regs[0x80 / 4];
		uint32_t controller_regs[0x38 / 4];
	} ac97st;
	struct superio_state
	{
		bool configuration_mode;
		int index;
		int selected;
		uint8_t registers[16][256]; // 256 registers for up to 16 devices, registers 0-0x2f common to all
	} superiost;
	uint8_t pic16lc_buffer[0xff];
	std::unique_ptr<nv2a_renderer> nvidia_nv2a;
	bool debug_irq_active;
	int debug_irq_number;
	bool usb_hack_enabled;
	required_device<cpu_device> m_maincpu;
	ohci_usb_controller *ohci_usb;

private:
	void dump_string_command(int ref, int params, const char **param);
	void dump_process_command(int ref, int params, const char **param);
	void dump_list_command(int ref, int params, const char **param);
	void dump_dpc_command(int ref, int params, const char **param);
	void dump_timer_command(int ref, int params, const char **param);
	void curthread_command(int ref, int params, const char **param);
	void generate_irq_command(int ref, int params, const char **param);
	void nv2a_combiners_command(int ref, int params, const char **param);
	void waitvblank_command(int ref, int params, const char **param);
	void grab_texture_command(int ref, int params, const char **param);
	void grab_vprog_command(int ref, int params, const char **param);
	void vprogdis_command(int ref, int params, const char **param);
	void help_command(int ref, int params, const char **param);
	void xbox_debug_commands(int ref, int params, const char **param);
};

ADDRESS_MAP_EXTERN(xbox_base_map, 32);
ADDRESS_MAP_EXTERN(xbox_base_map_io, 32);
MACHINE_CONFIG_EXTERN(xbox_base);
