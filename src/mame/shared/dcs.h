// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Midway DCS Audio Board

****************************************************************************/

#ifndef MAME_SHARED_DCS_H
#define MAME_SHARED_DCS_H

#pragma once

#include "cpu/adsp2100/adsp2100.h"
#include "machine/bankdev.h"
#include "machine/timer.h"
#include "sound/dmadac.h"


class dcs_audio_device : public device_t, public device_mixer_interface
{
public:
	enum { REV_DCS1, REV_DCS1P5, REV_DCS2, REV_DSIO, REV_DENV };

	template <typename T> void set_maincpu_tag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	// for dcs2 (int dram_in_mb, offs_t polling_offset)
	void set_dram_in_mb(int dram_in_mb) { m_dram_in_mb = dram_in_mb; }
	void set_polling_offset(offs_t polling_offset) { m_polling_offset = polling_offset; }

	void set_auto_ack(int state);

	void set_fifo_callbacks(read16smo_delegate fifo_data_r, read16mo_delegate fifo_status_r, write_line_delegate fifo_reset_w);
	void set_io_callbacks(write_line_delegate output_full_cb, write_line_delegate input_empty_cb);

	uint16_t data_r();
	void ack_w();
	int data2_r();
	int control_r();

	void data_w(uint16_t data);
	void reset_w(int state);

	void fifo_notify(int count, int max);

	void dsio_idma_addr_w(uint32_t data);
	void dsio_idma_data_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dsio_idma_data_r();
	void dmovlay_remap_memory();
	void dmovlay_callback(uint32_t data);
	void denver_postload();
	void install_speedup();

	uint8_t get_rev() { return m_rev; } // TODO(RH): This can be done better, and shouldn't be necessary.
	cpu_device *get_cpu() { return m_cpu; } // TODO(RH): Same.

protected:
	// construction/destruction
	dcs_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int rev, int outputs);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void add_mconfig_dcs(machine_config &config) ATTR_COLD;

	struct sdrc_state
	{
		uint16_t      reg[4];
		uint8_t       seed;
	};


	struct dsio_state
	{
		uint16_t      reg[4];
		uint8_t       start_on_next_write;
		uint16_t      channelbits;
	};


	struct hle_transfer_state
	{
		uint8_t       hle_enabled;
		int32_t       dcs_state;
		int32_t       state;
		int32_t       start;
		int32_t       stop;
		int32_t       type;
		int32_t       temp;
		int32_t       writes_left;
		uint16_t      sum;
		int32_t       fifo_entries;
		timer_device *watchdog;
	};

	optional_device<device_execute_interface> m_maincpu;

	required_device<timer_device> m_reg_timer;
	optional_device<timer_device> m_sport0_timer;
	required_device<timer_device> m_internal_timer;

	optional_device<address_map_bank_device> m_ram_map;

	optional_region_ptr<uint16_t> m_bootrom;
	optional_shared_ptr<uint32_t> m_internal_program_ram;
	optional_shared_ptr<uint32_t> m_external_program_ram;
	optional_shared_ptr<uint16_t> m_iram;

	optional_memory_bank    m_data_bank;
	memory_bank_creator     m_rom_page;
	memory_bank_creator     m_dram_page;

	adsp21xx_device *m_cpu;
	address_space *m_program;
	address_space *m_data;
	uint8_t       m_rev;
	offs_t        m_polling_offset;
	uint32_t      m_polling_count;
	// sound output
	uint8_t       m_channels;
	uint16_t      m_size;
	uint16_t      m_incs;
	dmadac_sound_device *m_dmadac[6];
	int32_t       m_ireg;
	uint16_t      m_ireg_base;
	uint16_t      m_control_regs[32];

	// memory access
	uint16_t *    m_sounddata;
	std::unique_ptr<uint16_t[]> m_sounddata_ptr;
	uint32_t      m_sounddata_words;
	uint32_t      m_sounddata_banks;
	uint16_t      m_sounddata_bank;

	// I/O with the host
	uint8_t       m_auto_ack;
	uint16_t      m_latch_control;
	uint16_t      m_input_data;
	uint16_t      m_output_data;
	uint16_t      m_pre_output_data;
	uint16_t      m_output_control;
	uint64_t      m_output_control_cycles;
	uint8_t       m_last_output_full;
	uint8_t       m_last_input_empty;
	uint16_t      m_progflags;
	emu_timer *   m_s1_ack_timer;
	emu_timer *   m_s1_ack2_timer;
	emu_timer *   m_s2_ack_timer;

	write_line_delegate m_output_full_cb;
	write_line_delegate m_input_empty_cb;

	read16smo_delegate  m_fifo_data_r;
	read16mo_delegate   m_fifo_status_r;
	write_line_delegate m_fifo_reset_w;

	/* timers */
	uint8_t       m_timer_enable;
	bool          m_timer_ignore;
	uint64_t      m_timer_start_cycles;
	uint32_t      m_timer_start_count;
	uint32_t      m_timer_scale;
	uint32_t      m_timer_period;
	uint32_t      m_timers_fired;

	std::unique_ptr<uint16_t[]> m_sram;
	uint16_t m_polling_value;
	uint32_t m_polling32_value;

	int32_t m_dmovlay_val;

	sdrc_state m_sdrc;
	dsio_state m_dsio;
	hle_transfer_state m_transfer;

	int m_dram_in_mb;

	// non public
	void dcs_boot();
	TIMER_CALLBACK_MEMBER( dcs_reset );
	void dcs_register_state();
	uint16_t dcs_dataram_r(offs_t offset);
	void dcs_dataram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dcs_data_bank_select_w(uint16_t data);
	void dcs_data_bank_select2_w(uint16_t data);
	inline void sdrc_update_bank_pointers();
	void sdrc_remap_memory();
	void sdrc_reset();
	uint16_t sdrc_r(offs_t offset);
	void sdrc_w(offs_t offset, uint16_t data);
	void dsio_reset();
	uint16_t dsio_r(offs_t offset);
	void dsio_w(offs_t offset, uint16_t data);
	void denver_alloc_dmadac();
	void denver_reset();
	uint16_t denver_r(offs_t offset);
	void denver_w(offs_t offset, uint16_t data);
	uint16_t latch_status_r(address_space &space);
	uint16_t fifo_input_r();
	void dcs_delayed_data_w(uint16_t data);
	TIMER_CALLBACK_MEMBER( dcs_delayed_data_w_callback );
	void input_latch_ack_w(uint16_t data);
	uint16_t input_latch_r();
	uint32_t input_latch32_r();
	TIMER_CALLBACK_MEMBER( latch_delayed_w );
	void output_latch_w(uint16_t data);
	void output_latch32_w(uint32_t data);
	void delayed_ack_w();
	TIMER_CALLBACK_MEMBER( delayed_ack_w_callback );
	TIMER_CALLBACK_MEMBER( output_control_delayed_w );
	void output_control_w(uint16_t data);
	uint16_t output_control_r();
	void update_timer_count();
	TIMER_DEVICE_CALLBACK_MEMBER( internal_timer_callback );
	void reset_timer();
	void timer_enable_callback(int state);
	uint16_t adsp_control_r(offs_t offset);
	void adsp_control_w(offs_t offset, uint16_t data);
	TIMER_DEVICE_CALLBACK_MEMBER( dcs_irq );
	TIMER_DEVICE_CALLBACK_MEMBER( sport0_irq );
	void recompute_sample_rate();
	void sound_tx_callback(offs_t offset, uint32_t data);
	uint16_t dcs_polling_r(address_space &space);
	void dcs_polling_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t dcs_polling32_r(address_space &space);
	void dcs_polling32_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	TIMER_DEVICE_CALLBACK_MEMBER( transfer_watchdog_callback );
	TIMER_CALLBACK_MEMBER( s1_ack_callback2 );
	TIMER_CALLBACK_MEMBER( s1_ack_callback1 );
	int preprocess_stage_1(uint16_t data);
	TIMER_CALLBACK_MEMBER( s2_ack_callback );
	int preprocess_stage_2(uint16_t data);
	int preprocess_write(uint16_t data);

	void dcs2_2104_data_map(address_map &map) ATTR_COLD;
	void dcs2_2104_program_map(address_map &map) ATTR_COLD;
	void dcs2_2115_data_map(address_map &map) ATTR_COLD;
	void dcs2_2115_program_map(address_map &map) ATTR_COLD;
	void dcs_2k_data_map(address_map &map) ATTR_COLD;
	void dcs_2k_program_map(address_map &map) ATTR_COLD;
	void dcs_2k_uart_data_map(address_map &map) ATTR_COLD;
	void dcs_8k_data_map(address_map &map) ATTR_COLD;
	void dcs_8k_program_map(address_map &map) ATTR_COLD;
	void dcs_wpc_program_map(address_map &map) ATTR_COLD;
	void denver_data_map(address_map &map) ATTR_COLD;
	void denver_io_map(address_map &map) ATTR_COLD;
	void denver_program_map(address_map &map) ATTR_COLD;
	void denver_rambank_map(address_map &map) ATTR_COLD;
	void dsio_data_map(address_map &map) ATTR_COLD;
	void dsio_io_map(address_map &map) ATTR_COLD;
	void dsio_program_map(address_map &map) ATTR_COLD;
	void dsio_rambank_map(address_map &map) ATTR_COLD;
};


// dcs_audio_2k_device

class dcs_audio_2k_device : public dcs_audio_device
{
public:
	// construction/destruction
	dcs_audio_2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// dcs_audio_2k_uart_device

class dcs_audio_2k_uart_device : public dcs_audio_device
{
public:
	// construction/destruction
	dcs_audio_2k_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// dcs_audio_8k_device

class dcs_audio_8k_device : public dcs_audio_device
{
public:
	// construction/destruction
	dcs_audio_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// dcs_audio_wpc_device

class dcs_audio_wpc_device : public dcs_audio_device
{
public:
	// construction/destruction
	dcs_audio_wpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void dcs_wpc_data_map(address_map &map) ATTR_COLD;
};


// dcs2_audio_device

class dcs2_audio_device : public dcs_audio_device
{
protected:
	// construction/destruction
	dcs2_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int outputs);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	void add_mconfig_dcs2(machine_config &config) ATTR_COLD;
};


// dcs2_audio_2115_device

class dcs2_audio_2115_device : public dcs2_audio_device
{
public:
	// construction/destruction
	dcs2_audio_2115_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// dcs2_audio_2104_device

class dcs2_audio_2104_device : public dcs2_audio_device
{
public:
	// construction/destruction
	dcs2_audio_2104_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// dcs2_audio_dsio_device

class dcs2_audio_dsio_device : public dcs2_audio_device
{
public:
	// construction/destruction
	dcs2_audio_dsio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// dcs2_audio_denver_device types

class dcs2_audio_denver_device : public dcs2_audio_device
{
protected:
	// construction/destruction
	dcs2_audio_denver_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int outputs);

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class dcs2_audio_denver_5ch_device : public dcs2_audio_denver_device
{
public:
	// construction/destruction
	dcs2_audio_denver_5ch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class dcs2_audio_denver_2ch_device : public dcs2_audio_denver_device
{
public:
	// construction/destruction
	dcs2_audio_denver_2ch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type declarations
DECLARE_DEVICE_TYPE(DCS_AUDIO_2K, dcs_audio_2k_device)
DECLARE_DEVICE_TYPE(DCS_AUDIO_2K_UART, dcs_audio_2k_uart_device)
DECLARE_DEVICE_TYPE(DCS_AUDIO_8K, dcs_audio_8k_device)
DECLARE_DEVICE_TYPE(DCS_AUDIO_WPC, dcs_audio_wpc_device)
DECLARE_DEVICE_TYPE(DCS2_AUDIO_2115, dcs2_audio_2115_device)
DECLARE_DEVICE_TYPE(DCS2_AUDIO_2104, dcs2_audio_2104_device)
DECLARE_DEVICE_TYPE(DCS2_AUDIO_DSIO, dcs2_audio_dsio_device)
DECLARE_DEVICE_TYPE(DCS2_AUDIO_DENVER_5CH, dcs2_audio_denver_5ch_device)
DECLARE_DEVICE_TYPE(DCS2_AUDIO_DENVER_2CH, dcs2_audio_denver_2ch_device)

#endif // MAME_SHARED_DCS_H
