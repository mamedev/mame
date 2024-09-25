// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_NEXT_NEXTKBD_H
#define MAME_NEXT_NEXTKBD_H

#pragma once


class nextkbd_device : public device_t {
public:
	nextkbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_change_wr_callback() { return int_change_cb.bind(); }
	auto int_power_wr_callback() { return int_power_cb.bind(); }
	auto int_nmi_wr_callback() { return int_nmi_cb.bind(); }

	void amap(address_map &map) ATTR_COLD;

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
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_tick);

private:
	// Big thanks to the previous emulator for that information
	enum {
		C_SOUND_OUT_ENABLE    = 0x80, // rw
		C_SOUND_OUT_REQUEST   = 0x40, // r
		C_SOUND_OUT_UNDERRUN  = 0x20, // rw
		C_SOUND_IN_ENABLE     = 0x08, // rw
		C_SOUND_IN_REQUEST    = 0x04, // r
		C_SOUND_IN_OVERRUN    = 0x02, // rw
		C_SOUND_WMASK         = 0xaa,

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
	required_ioport mousex;
	required_ioport mousey;
	required_ioport mousebtn;

	emu_timer *poll_timer;
	bool nmi_active;

	uint32_t cdata, kmdata, fifo_ir, fifo_iw, fifo_size;
	uint32_t fifo[FIFO_SIZE];
	uint32_t km_address;
	uint32_t prev_mousex, prev_mousey, prev_mousebtn;
	uint16_t modifiers_state;
	uint8_t ctrl_snd, ctrl_kms, ctrl_dma, ctrl_cmd;

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
