// license:BSD-3-Clause
// copyright-holders:Carl,psxAuthor,R. Belmont
#ifndef MAME_BUS_PSX_MEMCARD_H
#define MAME_BUS_PSX_MEMCARD_H

#pragma once


class psx_controller_port_device;


class psxcard_device :  public device_t,
						public device_image_interface
{
public:
	psxcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual iodevice_t image_type() const noexcept override { return IO_MEMCARD; }

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "mc"; }

	virtual image_init_result call_load() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;

	void disable(bool state) { m_disabled = state; if(state) unload(); }

	void clock_w(bool state) { if(!m_clock && !m_sel && state && !m_pad) do_card(); m_clock = state; }
	void sel_w(bool state);
	bool rx_r() { return m_rx; }
	bool ack_r() { return m_ack; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void read_card(const unsigned short addr, unsigned char *buf);
	void write_card(const unsigned short addr, unsigned char *buf);
	unsigned char checksum_data(const unsigned char *buf, const unsigned int sz);
	void do_card();
	bool transfer(uint8_t to, uint8_t *from);
	void ack_timer(void *ptr, int param);

	unsigned char pkt[0x8b], pkt_ptr, pkt_sz, cmd;
	unsigned short addr;
	int state;
	bool m_disabled;

	uint8_t m_odata;
	uint8_t m_idata;
	int m_bit;
	int m_count;
	bool m_pad;

	bool m_clock;
	bool m_sel;
	bool m_ack;
	bool m_rx;

	emu_timer *m_ack_timer;
	psx_controller_port_device *m_owner;
};

// device type definition
DECLARE_DEVICE_TYPE(PSXCARD, psxcard_device)

#endif // MAME_BUS_PSX_MEMCARD_H
