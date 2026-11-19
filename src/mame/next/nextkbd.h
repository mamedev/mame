// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_NEXT_NEXTKBD_H
#define MAME_NEXT_NEXTKBD_H

#pragma once

#include "sound/dac.h"

class nextkbd_device : public device_t, public device_mixer_interface {
public:
	static constexpr feature_type unemulated_features() { return feature::MICROPHONE; }
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	nextkbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto int_change_wr_callback() { return int_change_cb.bind(); }
	auto int_power_wr_callback() { return int_power_cb.bind(); }
	auto int_nmi_wr_callback() { return int_nmi_cb.bind(); }
	auto sound_drq_callback() { return sound_drq_cb.bind(); }
	auto sound_irq_callback() { return sound_irq_cb.bind(); }

	// 0 = original KMS, 1 = Turbo KMS
	void set_revision(uint8_t rev) { revision = rev; }

	void amap(address_map &map) ATTR_COLD;

	void sound_dma_w(uint8_t data);

	uint8_t status_snd_r();
	uint8_t status_kms_r();
	uint8_t status_dma_r();
	uint8_t status_cmd_r();
	uint32_t cdata_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t kmdata_r(offs_t offset, uint32_t mem_mask = ~0);

	void ctrl_snd_w(uint8_t data);
	void ctrl_kms_w(uint8_t data);
	void ctrl_dma_w(uint8_t data);
	void ctrl_cmd_w(uint8_t data);
	void cdata_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void kmdata_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	DECLARE_INPUT_CHANGED_MEMBER(update);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_tick);
	TIMER_CALLBACK_MEMBER(sound_tick);

private:
	// Big thanks to the previous emulator for that information
	enum {
		C_SOUND_OUT_ENABLE    = 0x80, // rw
		C_SOUND_OUT_REQUEST   = 0x40, // r
		C_SOUND_OUT_UNDERRUN  = 0x20, // rw
		C_SOUND_IN_ENABLE     = 0x08, // rw
		C_SOUND_IN_REQUEST    = 0x04, // r
		C_SOUND_IN_OVERRUN    = 0x02, // rw

		C_KBD_INTERRUPT       = 0x80, // r
		C_KBD_DATA            = 0x40, // r
		C_KBD_OVERRUN         = 0x20, // rw
		C_KBD_NMI             = 0x10, // rw?
		C_KMS_INTERRUPT       = 0x08, // r
		C_KMS_DATA            = 0x04, // r
		C_KMS_OVERRUN         = 0x02, // rw
		C_KMS_WMASK           = 0x22,

		C_SDMA_OUT_PENDING    = 0x80, // r
		C_SDMA_OUT_RUNNING    = 0x40, // r
		C_CDMA_PENDING        = 0x20, // r
		C_CDMA_RUNNING        = 0x10, // r
		C_KMS_ENABLE          = 0x02, // rw
		C_LOOPBACK            = 0x01, // rw
		C_WMASK               = 0x03,

		D_ERROR               = 0x40000000,
		D_USER                = 0x20000000,
		D_MASTER              = 0x10000000,
		D_SECONDARY           = 0x01000000,
		D_VERSION_OLD         = 0x00000000,
		D_VERSION_NEW         = 0x00010000,
		D_VERSION_DIGITAL     = 0x00020000,

		D_KBD_VALID           = 0x00008000,
		D_KBD_KEYDOWN         = 0x00000080
	};

	enum { FIFO_SIZE = 32 };

	devcb_write_line int_change_cb, int_power_cb, int_nmi_cb;
	devcb_write_line sound_drq_cb, sound_irq_cb;
	required_device_array<dac_16bit_r2r_twos_complement_device, 2> dac;
	required_ioport mousex;
	required_ioport mousey;
	required_ioport mousebtn;

	uint8_t revision;

	emu_timer *poll_timer;
	bool nmi_active;

	uint32_t cdata, kmdata, fifo_ir, fifo_iw, fifo_size;
	uint32_t fifo[FIFO_SIZE];
	uint32_t km_address;
	uint32_t prev_mousex, prev_mousey, prev_mousebtn;
	uint16_t modifiers_state;
	uint8_t ctrl_snd, ctrl_kms, ctrl_dma, ctrl_cmd;
	emu_timer *sound_timer;
	uint32_t sound_data, sound_sample, dac_sample;
	uint16_t volume_shift;
	uint8_t sound_bytes, sound_mode, volume_bits, sound_gpo;
	uint8_t volume[2];
	double sound_gain[2];
	bool sound_active, sound_repeat, sound_drq, sound_irq;

	void sound_request();
	void update_sound_irq();
	void sound_enable(uint8_t command);
	void sound_output(uint32_t sample);
	void sound_control(uint8_t data);
	void sound_volume(uint8_t data);
	void update_gain();

	void fifo_push(uint32_t val);
	uint32_t fifo_pop();
	bool fifo_empty() const;

	void update_mouse(bool force_update);
	void send();
	void handle_fifo_command();
	void handle_kbd_command();
	void handle_command();
};

DECLARE_DEVICE_TYPE(NEXTKBD, nextkbd_device)

#endif // MAME_NEXT_NEXTKBD_H
