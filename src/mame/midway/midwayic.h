// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Emulation of various Midway ICs

***************************************************************************/
#ifndef MAME_MIDWAY_MIDWAYIC_H
#define MAME_MIDWAY_MIDWAYIC_H

#pragma once

#include "cage.h"
#include "dcs.h"

#include "cpu/pic16c5x/pic16c5x.h"


// 1st generation Midway serial PIC - simulation

class midway_serial_pic_device : public device_t
{
public:
	// construction/destruction
	midway_serial_pic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_upper(int upper) { m_upper = upper; }

	u8 read();
	void write(u8 data);
	u8 status_r();
	void reset_w(int state);

protected:
	midway_serial_pic_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void generate_serial_data(int upper);
	void serial_register_state();

	required_ioport m_io_serial_digit;

	uint8_t m_data[16]; // reused by other devices
	int     m_upper;

private:
	uint8_t m_buff;
	uint8_t m_idx;
	uint8_t m_status;
	uint8_t m_bits;
};


// 1st generation Midway serial PIC - emulation

class midway_serial_pic_emu_device : public device_t
{
public:
	// construction/destruction
	midway_serial_pic_emu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 read();
	void write(u8 data);
	u8 status_r();
	void reset_w(int state);

protected:
	midway_serial_pic_emu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<pic16c57_device> m_pic;

	u8 read_c();
	void write_c(u8 data);

	u8 m_command;
	u8 m_data_out;
	u8 m_clk;
	u8 m_status;
};



// 2nd generation Midway serial/NVRAM/RTC PIC

// ======================> midway_serial_pic2_device

class midway_serial_pic2_device : public midway_serial_pic_device, public device_nvram_interface
{
public:
	// construction/destruction
	midway_serial_pic2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_yearoffs(int yearoffs) { m_yearoffs = yearoffs; }

	u8 read();
	void write(u8 data);
	u8 status_r();

	void set_default_nvram(const uint8_t *nvram);

protected:
	midway_serial_pic2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:

	void pic_register_state();
	TIMER_CALLBACK_MEMBER(reset_timer);

	uint16_t  m_latch;
	attotime  m_latch_expire_time;
	uint8_t   m_state;
	uint8_t   m_index;
	uint8_t   m_total;
	uint8_t   m_nvram_addr;
	uint8_t   m_buffer[0x10];
	uint8_t   m_nvram[0x100];
	uint8_t   m_default_nvram[0x100];
	uint8_t   m_time_buf[8];
	uint8_t   m_time_index;
	uint8_t   m_time_just_written;
	uint16_t  m_yearoffs;
	emu_timer *m_time_write_timer;
};


// I/O ASIC connected to 2nd generation PIC

// ======================> midway_ioasic_device

class midway_ioasic_device : public midway_serial_pic2_device
{
public:
	enum
	{
		SHUFFLE_STANDARD = 0,
		SHUFFLE_BLITZ99,
		SHUFFLE_CARNEVIL,
		SHUFFLE_CALSPEED,
		SHUFFLE_MACE,
		SHUFFLE_GAUNTDL,
		SHUFFLE_VAPORTRX,
		SHUFFLE_SFRUSHRK,
		SHUFFLE_HYPRDRIV
	};

	// construction/destruction
	midway_ioasic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<unsigned Port> auto in_port_cb() { return m_input_cb[Port].bind(); }
	template <typename T> void set_cage_tag(T &&tag) { m_cage.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_dcs_tag(T &&tag) { m_dcs.set_tag(std::forward<T>(tag)); }
	void set_shuffle(uint8_t shuffle) { m_shuffle_type = shuffle; }
	void set_shuffle_default(uint8_t shuffle) { m_shuffle_default = shuffle; }
	void set_auto_ack(uint8_t auto_ack) { m_auto_ack = auto_ack; }
	auto irq_handler() { return m_irq_callback.bind(); }
	auto serial_tx_handler() { return m_serial_tx_cb.bind(); }
	auto aux_output_handler() { return m_aux_output_cb.bind(); }

	void set_shuffle_state(int state);
	void fifo_w(uint16_t data);
	void fifo_full_w(uint16_t data);

	void fifo_reset_w(int state);
	uint16_t fifo_r();
	uint16_t fifo_status_r(address_space &space);

	void ioasic_input_empty(int state);
	void ioasic_output_full(int state);

	uint32_t read(address_space &space, offs_t offset);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t packed_r(address_space &space, offs_t offset, uint32_t mem_mask = ~0);
	void packed_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void cage_irq_handler(uint8_t data);

	void serial_rx_w(u8 data);

	void ioasic_reset();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	void ioasic_register_state();
	void update_ioasic_irq();
	uint16_t get_fifo_status();

	optional_device<atari_cage_device> m_cage;
	optional_device<dcs_audio_device> m_dcs;

	devcb_write8 m_irq_callback;

	devcb_read32::array<4> m_input_cb;

	devcb_write8    m_serial_tx_cb;
	devcb_write32   m_aux_output_cb;

	uint32_t  m_reg[16];
	cpu_device *m_dcs_cpu;
	uint8_t   m_shuffle_type;
	uint8_t   m_shuffle_default;
	uint8_t   m_shuffle_active;
	const uint8_t *m_shuffle_map;
	uint8_t   m_irq_state;
	uint16_t  m_sound_irq_state;
	uint8_t   m_auto_ack;
	uint8_t   m_force_fifo_full;

	uint16_t  m_fifo[512];
	uint16_t  m_fifo_in;
	uint16_t  m_fifo_out;
	uint16_t  m_fifo_bytes;
	offs_t  m_fifo_force_buffer_empty_pc;
};


// device type declarations
DECLARE_DEVICE_TYPE(MIDWAY_SERIAL_PIC, midway_serial_pic_device)
DECLARE_DEVICE_TYPE(MIDWAY_SERIAL_PIC_EMU, midway_serial_pic_emu_device)
DECLARE_DEVICE_TYPE(MIDWAY_SERIAL_PIC2, midway_serial_pic2_device)
DECLARE_DEVICE_TYPE(MIDWAY_IOASIC, midway_ioasic_device)

#endif // MAME_MIDWAY_MIDWAYIC_H
