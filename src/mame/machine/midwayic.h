// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Emulation of various Midway ICs

***************************************************************************/

#ifndef __MIDWAY_IC__
#define __MIDWAY_IC__

#include "audio/cage.h"
#include "audio/dcs.h"

/* 1st generation Midway serial PIC */

class midway_serial_pic_device : public device_t
{
public:
	// construction/destruction
	midway_serial_pic_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	midway_serial_pic_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	static void static_set_upper(device_t &device, int upper) { downcast<midway_serial_pic_device &>(device).m_upper = upper; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE_LINE_MEMBER( reset_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	void generate_serial_data(int upper);
	void serial_register_state();

	UINT8   m_data[16]; // reused by other devices
	int     m_upper;
private:
	UINT8   m_buff;
	UINT8   m_idx;
	UINT8   m_status;
	UINT8   m_bits;
	UINT8   m_ormask;
};


// device type definition
extern const device_type MIDWAY_SERIAL_PIC;

#define MCFG_MIDWAY_SERIAL_PIC_UPPER(_upper) \
	midway_serial_pic_device::static_set_upper(*device, _upper);

/* 2nd generation Midway serial/NVRAM/RTC PIC */

// ======================> midway_serial_pic2_device

class midway_serial_pic2_device : public midway_serial_pic_device,
									public device_nvram_interface
{
public:
	// construction/destruction
	midway_serial_pic2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	midway_serial_pic2_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	static void static_set_yearoffs(device_t &device, int yearoffs) { downcast<midway_serial_pic2_device &>(device).m_yearoffs = yearoffs; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( status_r );

	void set_default_nvram(const UINT8 *nvram);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:

	void pic_register_state();
	TIMER_CALLBACK_MEMBER( reset_timer );

	UINT16  m_latch;
	attotime m_latch_expire_time;
	UINT8   m_state;
	UINT8   m_index;
	UINT8   m_total;
	UINT8   m_nvram_addr;
	UINT8   m_buffer[0x10];
	UINT8   m_nvram[0x100];
	UINT8   m_default_nvram[0x100];
	UINT8   m_time_buf[8];
	UINT8   m_time_index;
	UINT8   m_time_just_written;
	UINT16  m_yearoffs;
	emu_timer *m_time_write_timer;
};


// device type definition
extern const device_type MIDWAY_SERIAL_PIC2;

#define MCFG_MIDWAY_SERIAL_PIC2_UPPER   MCFG_MIDWAY_SERIAL_PIC_UPPER

#define MCFG_MIDWAY_SERIAL_PIC2_YEAR_OFFS(_yearoffs) \
	midway_serial_pic2_device::static_set_yearoffs(*device, _yearoffs);

/* I/O ASIC connected to 2nd generation PIC */

// ======================> midway_ioasic_device

class midway_ioasic_device : public midway_serial_pic2_device
{
public:
	// construction/destruction
	midway_ioasic_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void static_set_shuffle(device_t &device, UINT8 shuffle) { downcast<midway_ioasic_device &>(device).m_shuffle_type = shuffle; }
	static void static_set_shuffle_default(device_t &device, UINT8 shuffle) { downcast<midway_ioasic_device &>(device).m_shuffle_default = shuffle; }
	static void static_set_auto_ack(device_t &device, UINT8 auto_ack) { downcast<midway_ioasic_device &>(device).m_auto_ack = auto_ack; }
	template<class _Object> static devcb_base &set_irqhandler_callback(device_t &device, _Object object) { return downcast<midway_ioasic_device &>(device).m_irq_callback.set_callback(object); }

	void set_shuffle_state(int state);
	void fifo_w(UINT16 data);
	void fifo_full_w(UINT16 data);

	DECLARE_WRITE_LINE_MEMBER(fifo_reset_w);
	DECLARE_READ16_MEMBER(fifo_r);
	DECLARE_READ16_MEMBER(fifo_status_r);

	DECLARE_WRITE_LINE_MEMBER(ioasic_input_empty);
	DECLARE_WRITE_LINE_MEMBER(ioasic_output_full);

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( packed_r );
	DECLARE_WRITE32_MEMBER( packed_w );

	DECLARE_WRITE8_MEMBER(cage_irq_handler);

	void ioasic_reset();

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void ioasic_register_state();
	void update_ioasic_irq();


	UINT32  m_reg[16];
	UINT8   m_has_dcs;
	UINT8   m_has_cage;
	device_t *m_dcs_cpu;
	UINT8   m_shuffle_type;
	UINT8   m_shuffle_default;
	UINT8   m_shuffle_active;
	const UINT8 *   m_shuffle_map;
	devcb_write8 m_irq_callback;
	UINT8   m_irq_state;
	UINT16  m_sound_irq_state;
	UINT8   m_auto_ack;
	UINT8   m_force_fifo_full;

	UINT16  m_fifo[512];
	UINT16  m_fifo_in;
	UINT16  m_fifo_out;
	UINT16  m_fifo_bytes;
	offs_t  m_fifo_force_buffer_empty_pc;

	atari_cage_device *m_cage;
	dcs_audio_device *m_dcs;
};


// device type definition
extern const device_type MIDWAY_IOASIC;

#define MCFG_MIDWAY_IOASIC_UPPER    MCFG_MIDWAY_SERIAL_PIC_UPPER

#define MCFG_MIDWAY_IOASIC_YEAR_OFFS MCFG_MIDWAY_SERIAL_PIC2_YEAR_OFFS

#define MCFG_MIDWAY_IOASIC_SHUFFLE(_shuffle) \
	midway_ioasic_device::static_set_shuffle(*device, _shuffle);

#define MCFG_MIDWAY_IOASIC_SHUFFLE_DEFAULT(_shuffle) \
	midway_ioasic_device::static_set_shuffle_default(*device, _shuffle);

#define MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(_write) \
	devcb = &midway_ioasic_device::set_irqhandler_callback(*device, DEVCB_##_write);

#define MCFG_MIDWAY_IOASIC_AUTO_ACK(_ack) \
	midway_ioasic_device::static_set_auto_ack(*device, _ack);


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

#endif
