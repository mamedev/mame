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
	dcs_audio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int rev = 1);

	// for dcs2 (int dram_in_mb, offs_t polling_offset)
	static void static_set_dram_in_mb(device_t &device, int dram_in_mb) { downcast<dcs_audio_device &>(device).m_dram_in_mb = dram_in_mb; }
	static void static_set_polling_offset(device_t &device, offs_t polling_offset) { downcast<dcs_audio_device &>(device).m_polling_offset = polling_offset; }

	void set_auto_ack(int state);

	void set_fifo_callbacks(read16_delegate fifo_data_r, read16_delegate fifo_status_r, write_line_delegate fifo_reset_w);
	void set_io_callbacks(write_line_delegate output_full_cb, write_line_delegate input_empty_cb);

	UINT16 data_r();
	void ack_w();
	int data2_r();
	int control_r();

	void data_w(UINT16 data);
	void reset_w(int state);

	void fifo_notify(int count, int max);

	DECLARE_WRITE32_MEMBER( dsio_idma_addr_w );
	DECLARE_WRITE32_MEMBER( dsio_idma_data_w );
	DECLARE_READ32_MEMBER( dsio_idma_data_r );

	// non public
	void dcs_boot();
	TIMER_CALLBACK_MEMBER( dcs_reset );
	void dcs_register_state();
	DECLARE_READ16_MEMBER( dcs_dataram_r );
	DECLARE_WRITE16_MEMBER( dcs_dataram_w );
	DECLARE_WRITE16_MEMBER( dcs_data_bank_select_w );
	DECLARE_WRITE16_MEMBER( dcs_data_bank_select2_w );
	inline void sdrc_update_bank_pointers();
	void sdrc_remap_memory();
	void sdrc_reset();
	DECLARE_READ16_MEMBER( sdrc_r );
	DECLARE_WRITE16_MEMBER( sdrc_w );
	void dsio_reset();
	DECLARE_READ16_MEMBER( dsio_r );
	DECLARE_WRITE16_MEMBER( dsio_w );
	void denver_reset();
	DECLARE_READ16_MEMBER( denver_r );
	DECLARE_WRITE16_MEMBER( denver_w );
	DECLARE_READ16_MEMBER( latch_status_r );
	DECLARE_READ16_MEMBER( fifo_input_r );
	void dcs_delayed_data_w(UINT16 data);
	TIMER_CALLBACK_MEMBER( dcs_delayed_data_w_callback );
	DECLARE_WRITE16_MEMBER( input_latch_ack_w );
	DECLARE_READ16_MEMBER( input_latch_r );
	DECLARE_READ32_MEMBER( input_latch32_r );
	TIMER_CALLBACK_MEMBER( latch_delayed_w );
	DECLARE_WRITE16_MEMBER( output_latch_w );
	DECLARE_WRITE32_MEMBER( output_latch32_w );
	void delayed_ack_w();
	TIMER_CALLBACK_MEMBER( delayed_ack_w_callback );
	TIMER_CALLBACK_MEMBER( output_control_delayed_w );
	DECLARE_WRITE16_MEMBER( output_control_w );
	DECLARE_READ16_MEMBER( output_control_r );
	void update_timer_count();
	TIMER_DEVICE_CALLBACK_MEMBER( internal_timer_callback );
	void reset_timer();
	DECLARE_WRITE_LINE_MEMBER(timer_enable_callback);
	DECLARE_READ16_MEMBER( adsp_control_r );
	DECLARE_WRITE16_MEMBER( adsp_control_w );
	TIMER_DEVICE_CALLBACK_MEMBER( dcs_irq );
	TIMER_DEVICE_CALLBACK_MEMBER( sport0_irq );
	void recompute_sample_rate();
	WRITE32_MEMBER(sound_tx_callback);
	DECLARE_READ16_MEMBER( dcs_polling_r );
	DECLARE_WRITE16_MEMBER( dcs_polling_w );
	TIMER_DEVICE_CALLBACK_MEMBER( transfer_watchdog_callback );
	TIMER_CALLBACK_MEMBER( s1_ack_callback2 );
	TIMER_CALLBACK_MEMBER( s1_ack_callback1 );
	int preprocess_stage_1(UINT16 data);
	TIMER_CALLBACK_MEMBER( s2_ack_callback );
	int preprocess_stage_2(UINT16 data);
	int preprocess_write(UINT16 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

protected:
	struct sdrc_state
	{
		UINT16      reg[4];
		UINT8       seed;
	};


	struct dsio_state
	{
		UINT16      reg[4];
		UINT8       start_on_next_write;
		UINT16      channelbits;
	};


	struct hle_transfer_state
	{
		UINT8       hle_enabled;
		INT32       dcs_state;
		INT32       state;
		INT32       start;
		INT32       stop;
		INT32       type;
		INT32       temp;
		INT32       writes_left;
		UINT16      sum;
		INT32       fifo_entries;
		timer_device *watchdog;
	};

	adsp21xx_device *m_cpu;
	address_space *m_program;
	address_space *m_data;
	UINT8       m_rev;
	offs_t      m_polling_offset;
	UINT32      m_polling_count;

	/* sound output */
	UINT8       m_channels;
	UINT16      m_size;
	UINT16      m_incs;
	dmadac_sound_device *m_dmadac[6];
	timer_device *m_reg_timer;
	timer_device *m_sport_timer;
	timer_device *m_internal_timer;
	INT32       m_ireg;
	UINT16      m_ireg_base;
	UINT16      m_control_regs[32];

	/* memory access/booting */
	UINT16 *    m_bootrom;
	UINT32      m_bootrom_words;
	UINT16 *    m_sounddata;
	UINT32      m_sounddata_words;
	UINT32      m_sounddata_banks;
	UINT16      m_sounddata_bank;

	/* I/O with the host */
	UINT8       m_auto_ack;
	UINT16      m_latch_control;
	UINT16      m_input_data;
	UINT16      m_output_data;
	UINT16      m_pre_output_data;
	UINT16      m_output_control;
	UINT64      m_output_control_cycles;
	UINT8       m_last_output_full;
	UINT8       m_last_input_empty;
	UINT16      m_progflags;

	write_line_delegate m_output_full_cb;
	write_line_delegate m_input_empty_cb;

	read16_delegate m_fifo_data_r;
	read16_delegate m_fifo_status_r;
	write_line_delegate m_fifo_reset_w;

	/* timers */
	UINT8       m_timer_enable;
	UINT8       m_timer_ignore;
	UINT64      m_timer_start_cycles;
	UINT32      m_timer_start_count;
	UINT32      m_timer_scale;
	UINT32      m_timer_period;
	UINT32      m_timers_fired;

	UINT16 *m_sram;
	UINT16 *m_polling_base;
	UINT32 *m_internal_program_ram;
	UINT32 *m_external_program_ram;

	sdrc_state m_sdrc;
	dsio_state m_dsio;
	hle_transfer_state m_transfer;

	int m_dram_in_mb;
};


// dcs_audio_2k_device

class dcs_audio_2k_device : public dcs_audio_device
{
public:
	// construction/destruction
	dcs_audio_2k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	dcs_audio_2k_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	dcs_audio_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	dcs_audio_wpc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	dcs2_audio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	// device-level overrides
	virtual void device_start() override;
};

// dcs2_audio_2115_device

class dcs2_audio_2115_device : public dcs2_audio_device
{
public:
	// construction/destruction
	dcs2_audio_2115_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	dcs2_audio_2104_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	dcs2_audio_dsio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	dcs2_audio_denver_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

};

// device type definition
extern const device_type DCS2_AUDIO_DENVER;

#endif
