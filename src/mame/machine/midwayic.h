// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Emulation of various Midway ICs

***************************************************************************/
#ifndef MAME_MACHINE_MIDWAY_IC_H
#define MAME_MACHINE_MIDWAY_IC_H

#pragma once


#include "audio/cage.h"
#include "audio/dcs.h"
#include "cpu/pic16c5x/pic16c5x.h"

/* 1st generation Midway serial PIC - simulation*/

class midway_serial_pic_device : public device_t
{
public:
	// construction/destruction
	midway_serial_pic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_upper(int upper) { m_upper = upper; }

	u8 read();
	void write(u8 data);
	u8 status_r();
	DECLARE_WRITE_LINE_MEMBER( reset_w );

protected:
	midway_serial_pic_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;

	void generate_serial_data(int upper);
	void serial_register_state();

	required_ioport m_io_serial_digit;

	uint8_t   m_data[16]; // reused by other devices
	int     m_upper;

private:
	uint8_t   m_buff;
	uint8_t   m_idx;
	uint8_t   m_status;
	uint8_t   m_bits;
};


// device type definition
DECLARE_DEVICE_TYPE(MIDWAY_SERIAL_PIC, midway_serial_pic_device)

/* 1st generation Midway serial PIC - emulation */

class midway_serial_pic_emu_device : public device_t
{
public:
	// construction/destruction
	midway_serial_pic_emu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 read();
	void write(u8 data);
	u8 status_r();
	DECLARE_WRITE_LINE_MEMBER(reset_w);

protected:
	midway_serial_pic_emu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	required_device<pic16c57_device> m_pic;

	u8 read_c();
	void write_c(u8 data);

	u8 m_command;
	u8 m_data_out;
	u8 m_clk;
	u8 m_status;
};


// device type definition
DECLARE_DEVICE_TYPE(MIDWAY_SERIAL_PIC_EMU, midway_serial_pic_emu_device)



/* 2nd generation Midway serial/NVRAM/RTC PIC */

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

	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:

	void pic_register_state();
	TIMER_CALLBACK_MEMBER( reset_timer );

	uint16_t  m_latch;
	attotime m_latch_expire_time;
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


// device type definition
DECLARE_DEVICE_TYPE(MIDWAY_SERIAL_PIC2, midway_serial_pic2_device)

/* I/O ASIC connected to 2nd generation PIC */

// ======================> midway_ioasic_device

class midway_ioasic_device : public midway_serial_pic2_device
{
public:
	// construction/destruction
	midway_ioasic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_shuffle(uint8_t shuffle) { m_shuffle_type = shuffle; }
	void set_shuffle_default(uint8_t shuffle) { m_shuffle_default = shuffle; }
	void set_auto_ack(uint8_t auto_ack) { m_auto_ack = auto_ack; }
	auto irq_handler() { return m_irq_callback.bind(); }
	auto serial_tx_handler() { return m_serial_tx_cb.bind(); }
	auto aux_output_handler() { return m_aux_output_cb.bind(); }

	void set_shuffle_state(int state);
	void fifo_w(uint16_t data);
	void fifo_full_w(uint16_t data);

	DECLARE_WRITE_LINE_MEMBER(fifo_reset_w);
	uint16_t fifo_r();
	uint16_t fifo_status_r(address_space &space);

	DECLARE_WRITE_LINE_MEMBER(ioasic_input_empty);
	DECLARE_WRITE_LINE_MEMBER(ioasic_output_full);

	uint32_t read(address_space &space, offs_t offset);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t packed_r(address_space &space, offs_t offset, uint32_t mem_mask = ~0);
	void packed_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void cage_irq_handler(uint8_t data);

	void serial_rx_w(u8 data);

	void ioasic_reset();

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void ioasic_register_state();
	void update_ioasic_irq();
	uint16_t get_fifo_status();

	required_ioport m_io_dips;
	required_ioport m_io_system;
	required_ioport m_io_in1;
	required_ioport m_io_in2;

	devcb_write8    m_serial_tx_cb;
	devcb_write32   m_aux_output_cb;

	uint32_t  m_reg[16];
	uint8_t   m_has_dcs;
	uint8_t   m_has_cage;
	cpu_device *m_dcs_cpu;
	uint8_t   m_shuffle_type;
	uint8_t   m_shuffle_default;
	uint8_t   m_shuffle_active;
	const uint8_t *   m_shuffle_map;
	devcb_write8 m_irq_callback;
	uint8_t   m_irq_state;
	uint16_t  m_sound_irq_state;
	uint8_t   m_auto_ack;
	uint8_t   m_force_fifo_full;

	uint16_t  m_fifo[512];
	uint16_t  m_fifo_in;
	uint16_t  m_fifo_out;
	uint16_t  m_fifo_bytes;
	offs_t  m_fifo_force_buffer_empty_pc;

	optional_device<atari_cage_device> m_cage;
	optional_device<dcs_audio_device> m_dcs;
};


// device type definition
DECLARE_DEVICE_TYPE(MIDWAY_IOASIC, midway_ioasic_device)

enum
{
	MIDWAY_IOASIC_STANDARD = 0,
	MIDWAY_IOASIC_BLITZ99,
	MIDWAY_IOASIC_CARNEVIL,
	MIDWAY_IOASIC_CALSPEED,
	MIDWAY_IOASIC_MACE,
	MIDWAY_IOASIC_GAUNTDL,
	MIDWAY_IOASIC_VAPORTRX,
	MIDWAY_IOASIC_SFRUSHRK,
	MIDWAY_IOASIC_HYPRDRIV
};

#endif // MAME_MACHINE_MIDWAY_IC_H
