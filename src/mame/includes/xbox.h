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

	DECLARE_READ32_MEMBER(geforce_r);
	DECLARE_WRITE32_MEMBER(geforce_w);
	DECLARE_READ32_MEMBER(smbus_r);
	DECLARE_WRITE32_MEMBER(smbus_w);
	DECLARE_READ32_MEMBER(smbus2_r);
	DECLARE_WRITE32_MEMBER(smbus2_w);
	DECLARE_READ32_MEMBER(networkio_r);
	DECLARE_WRITE32_MEMBER(networkio_w);
	DECLARE_READ8_MEMBER(superio_read);
	DECLARE_WRITE8_MEMBER(superio_write);
	DECLARE_READ8_MEMBER(superiors232_read);
	DECLARE_WRITE8_MEMBER(superiors232_write);
	DECLARE_READ32_MEMBER(audio_apu_r);
	DECLARE_WRITE32_MEMBER(audio_apu_w);
	DECLARE_READ32_MEMBER(audio_ac93_r);
	DECLARE_WRITE32_MEMBER(audio_ac93_w);
	DECLARE_READ32_MEMBER(dummy_r);
	DECLARE_WRITE32_MEMBER(dummy_w);
	DECLARE_READ32_MEMBER(ohci_usb_r);
	DECLARE_WRITE32_MEMBER(ohci_usb_w);
	DECLARE_READ32_MEMBER(ohci_usb2_r);
	DECLARE_WRITE32_MEMBER(ohci_usb2_w);
	DECLARE_READ32_MEMBER(network_r);
	DECLARE_WRITE32_MEMBER(network_w);

	void smbus_register_device(int address, int(*handler)(xbox_base_state &chs, int command, int rw, int data));
	int smbus_pic16lc(int command, int rw, int data);
	int smbus_cx25871(int command, int rw, int data);
	int smbus_eeprom(int command, int rw, int data);
	void debug_generate_irq(int irq, bool active);
	virtual void hack_eeprom() {};
	virtual void hack_usb() {};

	void vblank_callback(screen_device &screen, bool state);
	UINT32 screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	DECLARE_WRITE_LINE_MEMBER(xbox_pic8259_1_set_int_line);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(xbox_pit8254_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(xbox_pit8254_out2_changed);
	DECLARE_WRITE_LINE_MEMBER(xbox_ohci_usb_interrupt_changed);
	IRQ_CALLBACK_MEMBER(irq_callback);
	TIMER_CALLBACK_MEMBER(audio_apu_timer);

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
		UINT32 words[256 / 4];
	} smbusst;
	struct apu_state {
		UINT32 memory[0x60000 / 4];
		UINT32 gpdsp_sgaddress; // global processor scatter-gather
		UINT32 gpdsp_sgblocks;
		UINT32 gpdsp_address;
		UINT32 epdsp_sgaddress; // encoder processor scatter-gather
		UINT32 epdsp_sgblocks;
		UINT32 unknown_sgaddress;
		UINT32 unknown_sgblocks;
		int voice_number;
		UINT32 voices_heap_blockaddr[1024];
		UINT64 voices_active[4]; //one bit for each voice: 1 playing 0 not
		UINT32 voicedata_address;
		int voices_frequency[256]; // sample rate
		int voices_position[256]; // position in samples * 1000
		int voices_position_start[256]; // position in samples * 1000
		int voices_position_end[256]; // position in samples * 1000
		int voices_position_increment[256]; // position increment every 1ms * 1000
		emu_timer *timer;
		address_space *space;
	} apust;
	struct ac97_state {
		UINT32 mixer_regs[0x80 / 4];
		UINT32 controller_regs[0x38 / 4];
	} ac97st;
	struct superio_state
	{
		bool configuration_mode;
		int index;
		int selected;
		UINT8 registers[16][256]; // 256 registers for up to 16 devices, registers 0-0x2f common to all
	} superiost;
	UINT8 pic16lc_buffer[0xff];
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
