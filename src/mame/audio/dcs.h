// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Midway DCS Audio Board

****************************************************************************/

#ifndef __DCS_H__
#define __DCS_H__

#include "cpu/adsp2100/adsp2100.h"
#include "sound/dmadac.h"

#define MCFG_DCS2_AUDIO_DRAM_IN_MB(_dram_in_mb) \
	dcs_audio_device::static_set_dram_in_mb(*device, _dram_in_mb);

#define MCFG_DCS2_AUDIO_POLLING_OFFSET(_polling_offset) \
	dcs_audio_device::static_set_polling_offset(*device, _polling_offset);


class dcs_audio_device : public device_t
{
public:
	// construction/destruction
	dcs_audio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source, int rev = 1);

	// for dcs2 (int dram_in_mb, offs_t polling_offset)
	static void static_set_dram_in_mb(device_t &device, int dram_in_mb) { downcast<dcs_audio_device &>(device).m_dram_in_mb = dram_in_mb; }
	static void static_set_polling_offset(device_t &device, offs_t polling_offset) { downcast<dcs_audio_device &>(device).m_polling_offset = polling_offset; }

	void set_auto_ack(int state);

	void set_fifo_callbacks(read16_delegate fifo_data_r, read16_delegate fifo_status_r, write_line_delegate fifo_reset_w);
	void set_io_callbacks(write_line_delegate output_full_cb, write_line_delegate input_empty_cb);

	uint16_t data_r();
	void ack_w();
	int data2_r();
	int control_r();

	void data_w(uint16_t data);
	void reset_w(int state);

	void fifo_notify(int count, int max);

	void dsio_idma_addr_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void dsio_idma_data_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t dsio_idma_data_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void dmovlay_remap_memory();
	void dmovlay_callback(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);

	// non public
	void dcs_boot();
	void dcs_reset(void *ptr, int32_t param);
	void dcs_register_state();
	uint16_t dcs_dataram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dcs_dataram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dcs_data_bank_select_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dcs_data_bank_select2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	inline void sdrc_update_bank_pointers();
	void sdrc_remap_memory();
	void sdrc_reset();
	uint16_t sdrc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sdrc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dsio_reset();
	uint16_t dsio_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsio_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void denver_reset();
	uint16_t denver_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void denver_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t latch_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t fifo_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dcs_delayed_data_w(uint16_t data);
	void dcs_delayed_data_w_callback(void *ptr, int32_t param);
	void input_latch_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t input_latch_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t input_latch32_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void latch_delayed_w(void *ptr, int32_t param);
	void output_latch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void output_latch32_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void delayed_ack_w();
	void delayed_ack_w_callback(void *ptr, int32_t param);
	void output_control_delayed_w(void *ptr, int32_t param);
	void output_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t output_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void update_timer_count();
	void internal_timer_callback(timer_device &timer, void *ptr, int32_t param);
	void reset_timer();
	void timer_enable_callback(int state);
	uint16_t adsp_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void adsp_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dcs_irq(timer_device &timer, void *ptr, int32_t param);
	void sport0_irq(timer_device &timer, void *ptr, int32_t param);
	void recompute_sample_rate();
	void sound_tx_callback(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t dcs_polling_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dcs_polling_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void transfer_watchdog_callback(timer_device &timer, void *ptr, int32_t param);
	void s1_ack_callback2(void *ptr, int32_t param);
	void s1_ack_callback1(void *ptr, int32_t param);
	int preprocess_stage_1(uint16_t data);
	void s2_ack_callback(void *ptr, int32_t param);
	int preprocess_stage_2(uint16_t data);
	int preprocess_write(uint16_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

protected:
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

	adsp21xx_device *m_cpu;
	address_space *m_program;
	address_space *m_data;
	uint8_t       m_rev;
	offs_t      m_polling_offset;
	uint32_t      m_polling_count;

	/* sound output */
	uint8_t       m_channels;
	uint16_t      m_size;
	uint16_t      m_incs;
	dmadac_sound_device *m_dmadac[6];
	timer_device *m_reg_timer;
	timer_device *m_sport_timer;
	timer_device *m_internal_timer;
	int32_t       m_ireg;
	uint16_t      m_ireg_base;
	uint16_t      m_control_regs[32];

	/* memory access/booting */
	uint16_t *    m_bootrom;
	uint32_t      m_bootrom_words;
	uint16_t *    m_sounddata;
	uint32_t      m_sounddata_words;
	uint32_t      m_sounddata_banks;
	uint16_t      m_sounddata_bank;

	optional_memory_bank    m_data_bank;
	memory_bank *           m_rom_page;
	memory_bank *           m_dram_page;

	/* I/O with the host */
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

	write_line_delegate m_output_full_cb;
	write_line_delegate m_input_empty_cb;

	read16_delegate m_fifo_data_r;
	read16_delegate m_fifo_status_r;
	write_line_delegate m_fifo_reset_w;

	/* timers */
	uint8_t       m_timer_enable;
	uint8_t       m_timer_ignore;
	uint64_t      m_timer_start_cycles;
	uint32_t      m_timer_start_count;
	uint32_t      m_timer_scale;
	uint32_t      m_timer_period;
	uint32_t      m_timers_fired;

	uint16_t *m_sram;
	uint16_t *m_polling_base;
	uint32_t *m_internal_program_ram;
	uint32_t *m_external_program_ram;

	int m_dmovlay_val;

	sdrc_state m_sdrc;
	dsio_state m_dsio;
	hle_transfer_state m_transfer;

	int m_dram_in_mb;

	optional_shared_ptr<uint16_t> m_iram;
};


// dcs_audio_2k_device

class dcs_audio_2k_device : public dcs_audio_device
{
public:
	// construction/destruction
	dcs_audio_2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

};

// device type definition
extern const device_type DCS_AUDIO_2K;

// dcs_audio_2k_uart_device

class dcs_audio_2k_uart_device : public dcs_audio_device
{
public:
	// construction/destruction
	dcs_audio_2k_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

};

// device type definition
extern const device_type DCS_AUDIO_2K_UART;

// dcs_audio_8k_device

class dcs_audio_8k_device : public dcs_audio_device
{
public:
	// construction/destruction
	dcs_audio_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

};

// device type definition
extern const device_type DCS_AUDIO_8K;

// dcs_audio_wpc_device

class dcs_audio_wpc_device : public dcs_audio_device
{
public:
	// construction/destruction
	dcs_audio_wpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};

// device type definition
extern const device_type DCS_AUDIO_WPC;


// dcs2_audio_device

class dcs2_audio_device : public dcs_audio_device
{
public:
	// construction/destruction
	dcs2_audio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

protected:
	// device-level overrides
	virtual void device_start() override;
};

// dcs2_audio_2115_device

class dcs2_audio_2115_device : public dcs2_audio_device
{
public:
	// construction/destruction
	dcs2_audio_2115_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

};

// device type definition
extern const device_type DCS2_AUDIO_2115;

// dcs2_audio_2104_device

class dcs2_audio_2104_device : public dcs2_audio_device
{
public:
	// construction/destruction
	dcs2_audio_2104_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

};

// device type definition
extern const device_type DCS2_AUDIO_2104;

// dcs2_audio_dsio_device

class dcs2_audio_dsio_device : public dcs2_audio_device
{
public:
	// construction/destruction
	dcs2_audio_dsio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

};

// device type definition
extern const device_type DCS2_AUDIO_DSIO;

// dcs2_audio_denver_device

class dcs2_audio_denver_device : public dcs2_audio_device
{
public:
	// construction/destruction
	dcs2_audio_denver_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

};

// device type definition
extern const device_type DCS2_AUDIO_DENVER;

#endif
