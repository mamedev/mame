// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, R. Belmont
/*
 * apollo.h - APOLLO DN3500/DN3000 driver includes
 *
 *  Created on: May 12, 2010
 *      Author: Hans Ostermeyer
 *
 */

#pragma once

#ifndef APOLLO_H_
#define APOLLO_H_

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "bus/rs232/rs232.h"
#include "machine/terminal.h"
#include "machine/ram.h"
#include "machine/6840ptm.h"
#include "machine/mc68681.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/mc146818.h"
#include "machine/apollo_kbd.h"
#include "machine/clock.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/isa/3c505.h"

#ifndef VERBOSE
#define VERBOSE 0
#endif

#define LOG(x)  { logerror x; logerror ("\n"); apollo_check_log(); }
#define LOG1(x) { if (VERBOSE > 0) LOG(x) }
#define LOG2(x) { if (VERBOSE > 1) LOG(x) }
#define CLOG(x) { machine().logerror ("%s - %s: ", apollo_cpu_context(machine().device(MAINCPU)), tag()); machine().logerror x; machine().logerror ("\n"); apollo_check_log(); }
#define CLOG1(x) { if (VERBOSE > 0) CLOG(x) }
#define CLOG2(x) { if (VERBOSE > 1) CLOG(x) }
#define DLOG(x) { device->logerror ("%s - %s: ", apollo_cpu_context(device->machine().device(MAINCPU)), device->tag()); device->logerror x; device->logerror ("\n"); apollo_check_log(); }
#define DLOG1(x) { if (VERBOSE > 0) DLOG(x) }
#define DLOG2(x) { if (VERBOSE > 1) DLOG(x) }
#define MLOG(x)  { machine().logerror ("%s: ", apollo_cpu_context(machine().device(MAINCPU))); machine().logerror x; machine().logerror ("\n"); apollo_check_log(); }
#define MLOG1(x) { if (VERBOSE > 0) MLOG(x) }
#define MLOG2(x) { if (VERBOSE > 1) MLOG(x) }
#define SLOG(x)  { machine().logerror ("%s: ", apollo_cpu_context(m_maincpu));machine().logerror x; machine().logerror ("\n"); apollo_check_log(); }
#define SLOG1(x) { if (VERBOSE > 0) SLOG(x) }
#define SLOG2(x) { if (VERBOSE > 1) SLOG(x) }

#define  MAINCPU "maincpu"

// Enabling this is >NOT< supported by MESSdev
// Do *not* report any issues on Mametesters if this is enabled!
// #define APOLLO_XXL

/*----------- machine/apollo_dbg.c -----------*/

int apollo_debug_instruction_hook(m68000_base_device *device, offs_t curpc);

/*----------- drivers/apollo.c -----------*/

// return the current CPU context for log file entries
const char *apollo_cpu_context(device_t *cpu);

// enable/disable the FPU
void apollo_set_cpu_has_fpu(m68000_base_device *device, int onoff);

// check for excessive logging
void apollo_check_log();

// return 1 if node is DN3000 or DSP3000, 0 otherwise
int apollo_is_dn3000(void);

// return 1 if node is DN5500 or DSP5500, 0 otherwise
int apollo_is_dn5500(void);

// return 1 if node is DSP3000 or DSP3500, 0 otherwise
int apollo_is_dsp3x00(void);

// get the ram configuration byte
UINT8 apollo_get_ram_config_byte(void);

//apollo_get_node_id - get the node id
UINT32 apollo_get_node_id(void);

	// should be called by the CPU core before executing each instruction
int apollo_instruction_hook(m68000_base_device *device, offs_t curpc);

void apollo_set_cache_status_register(device_t *device,UINT8 mask, UINT8 data);

/*----------- machine/apollo.c -----------*/

#define APOLLO_CONF_TAG "conf"
#define APOLLO_DMA1_TAG "dma8237_1"
#define APOLLO_DMA2_TAG "dma8237_2"
#define APOLLO_KBD_TAG  "kbd"
#define APOLLO_STDIO_TAG "stdio"
#define APOLLO_PIC1_TAG "pic8259_master"
#define APOLLO_PIC2_TAG "pic8259_slave"
#define APOLLO_PTM_TAG  "ptm"
#define APOLLO_RTC_TAG  "rtc"
#define APOLLO_SIO_TAG  "sio"
#define APOLLO_SIO2_TAG "sio2"
#define APOLLO_ETH_TAG  "3c505"
#define APOLLO_NI_TAG  "node_id"
#define APOLLO_ISA_TAG "isabus"

// forward declaration
class apollo_sio;
class apollo_ni;

class apollo_state : public driver_device
{
public:
	apollo_state(const machine_config &mconfig, device_type type, const char *tag)
			: driver_device(mconfig, type, tag),
			m_maincpu(*this, MAINCPU),
			m_messram_ptr(*this, "messram"),
			m_dma8237_1(*this, APOLLO_DMA1_TAG),
			m_dma8237_2(*this, APOLLO_DMA2_TAG),
			m_pic8259_master(*this, APOLLO_PIC1_TAG),
			m_pic8259_slave(*this, APOLLO_PIC2_TAG),
			m_ptm(*this, APOLLO_PTM_TAG),
			m_sio(*this, APOLLO_SIO_TAG),
			m_sio2(*this, APOLLO_SIO2_TAG),
			m_rtc(*this, APOLLO_RTC_TAG),
			m_node_id(*this, APOLLO_NI_TAG),
			m_isa(*this, APOLLO_ISA_TAG)
			{ }

	required_device<m68000_base_device> m_maincpu;
	required_shared_ptr<UINT32> m_messram_ptr;

	required_device<am9517a_device> m_dma8237_1;
	required_device<am9517a_device> m_dma8237_2;
	required_device<pic8259_device> m_pic8259_master;
	required_device<pic8259_device> m_pic8259_slave;
	required_device<ptm6840_device> m_ptm;
	required_device<apollo_sio> m_sio;
	optional_device<apollo_sio> m_sio2;
	required_device<mc146818_device> m_rtc;
	required_device<apollo_ni> m_node_id;
	required_device<isa16_device> m_isa;

	DECLARE_WRITE16_MEMBER(apollo_csr_status_register_w);
	DECLARE_READ16_MEMBER(apollo_csr_status_register_r);
	DECLARE_WRITE16_MEMBER(apollo_csr_control_register_w);
	DECLARE_READ16_MEMBER(apollo_csr_control_register_r);
	DECLARE_WRITE8_MEMBER(apollo_dma_1_w);
	DECLARE_READ8_MEMBER(apollo_dma_1_r);
	DECLARE_WRITE8_MEMBER(apollo_dma_2_w);
	DECLARE_READ8_MEMBER(apollo_dma_2_r);
	DECLARE_WRITE8_MEMBER(apollo_dma_page_register_w);
	DECLARE_READ8_MEMBER(apollo_dma_page_register_r);
	DECLARE_WRITE16_MEMBER(apollo_address_translation_map_w);
	DECLARE_READ16_MEMBER(apollo_address_translation_map_r);
	DECLARE_READ8_MEMBER(apollo_dma_read_byte);
	DECLARE_WRITE8_MEMBER(apollo_dma_write_byte);
	DECLARE_READ8_MEMBER(apollo_dma_read_word);
	DECLARE_WRITE8_MEMBER(apollo_dma_write_word);
	DECLARE_WRITE8_MEMBER(apollo_rtc_w);
	DECLARE_READ8_MEMBER(apollo_rtc_r);
	DECLARE_WRITE8_MEMBER(cache_control_register_w);
	DECLARE_READ8_MEMBER(cache_status_register_r);
	DECLARE_WRITE8_MEMBER(task_alias_register_w);
	DECLARE_READ8_MEMBER(task_alias_register_r);
	DECLARE_WRITE16_MEMBER(apollo_node_id_w);
	DECLARE_READ16_MEMBER(apollo_node_id_r);
	DECLARE_WRITE16_MEMBER(latch_page_on_parity_error_register_w);
	DECLARE_READ16_MEMBER(latch_page_on_parity_error_register_r);
	DECLARE_WRITE8_MEMBER(master_req_register_w);
	DECLARE_READ8_MEMBER(master_req_register_r);
	DECLARE_WRITE16_MEMBER(selective_clear_locations_w);
	DECLARE_READ16_MEMBER(selective_clear_locations_r);
	DECLARE_READ32_MEMBER(ram_with_parity_r);
	DECLARE_WRITE32_MEMBER(ram_with_parity_w);
	DECLARE_READ32_MEMBER(apollo_unmapped_r);
	DECLARE_WRITE32_MEMBER(apollo_unmapped_w);
	DECLARE_WRITE32_MEMBER(apollo_rom_w);
	DECLARE_READ16_MEMBER(apollo_atbus_io_r);
	DECLARE_WRITE16_MEMBER(apollo_atbus_io_w);
	DECLARE_READ16_MEMBER(apollo_atbus_memory_r);
	DECLARE_WRITE16_MEMBER(apollo_atbus_memory_w);
	DECLARE_READ16_MEMBER(apollo_atbus_unmap_io_r);
	DECLARE_WRITE16_MEMBER(apollo_atbus_unmap_io_w);
	DECLARE_READ8_MEMBER(apollo_atbus_unmap_r);
	DECLARE_WRITE8_MEMBER(apollo_atbus_unmap_w);
	DECLARE_WRITE8_MEMBER(dn5500_memory_present_register_w);
	DECLARE_READ8_MEMBER(dn5500_memory_present_register_r);
	DECLARE_WRITE8_MEMBER(dn5500_11500_w);
	DECLARE_READ8_MEMBER(dn5500_11500_r);
	DECLARE_WRITE8_MEMBER(dn5500_io_protection_map_w);
	DECLARE_READ8_MEMBER(dn5500_io_protection_map_r);
	DECLARE_READ32_MEMBER(apollo_f8_r);
	DECLARE_WRITE32_MEMBER(apollo_f8_w);
	DECLARE_DRIVER_INIT(dsp3000);
	DECLARE_DRIVER_INIT(dsp5500);
	DECLARE_DRIVER_INIT(dn3500);
	DECLARE_DRIVER_INIT(dn3000);
	DECLARE_DRIVER_INIT(dsp3500);
	DECLARE_DRIVER_INIT(dn5500);
	DECLARE_DRIVER_INIT(apollo);

	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_MACHINE_RESET(apollo);
	DECLARE_MACHINE_START(apollo);

	IRQ_CALLBACK_MEMBER(apollo_irq_acknowledge);
	IRQ_CALLBACK_MEMBER(apollo_pic_acknowledge);
	void apollo_bus_error();
	DECLARE_WRITE8_MEMBER( apollo_kbd_putchar );
	DECLARE_READ_LINE_MEMBER( apollo_kbd_is_german );
	DECLARE_WRITE_LINE_MEMBER( apollo_dma8237_out_eop );
	DECLARE_WRITE_LINE_MEMBER( apollo_dma_1_hrq_changed );
	DECLARE_WRITE_LINE_MEMBER( apollo_dma_2_hrq_changed );
	DECLARE_WRITE_LINE_MEMBER( apollo_pic8259_master_set_int_line );
	DECLARE_WRITE_LINE_MEMBER( apollo_pic8259_slave_set_int_line );
	DECLARE_WRITE_LINE_MEMBER( sio_irq_handler );
	DECLARE_WRITE8_MEMBER( sio_output );
	DECLARE_WRITE_LINE_MEMBER( sio2_irq_handler );
	DECLARE_WRITE_LINE_MEMBER( apollo_ptm_irq_function );
	DECLARE_WRITE_LINE_MEMBER( apollo_ptm_timer_tick );
	DECLARE_READ8_MEMBER( apollo_pic8259_get_slave_ack );

	DECLARE_READ8_MEMBER(pc_dma8237_0_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_1_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_2_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_3_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_5_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_6_dack_r);
	DECLARE_READ8_MEMBER(pc_dma8237_7_dack_r);
	DECLARE_WRITE8_MEMBER(pc_dma8237_0_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_1_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_2_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_3_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_5_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_6_dack_w);
	DECLARE_WRITE8_MEMBER(pc_dma8237_7_dack_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack0_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack1_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack2_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack3_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack4_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack5_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack6_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack7_w);
	TIMER_CALLBACK_MEMBER( apollo_rtc_timer );

	void apollo_pic_set_irq_line(int irq, int state);
	void select_dma_channel(int channel, bool state);

	DECLARE_WRITE_LINE_MEMBER(apollo_reset_instr_callback);
	DECLARE_READ32_MEMBER(apollo_instruction_hook);

private:
	UINT32 ptm_counter;
	UINT8 sio_output_data;
	int m_dma_channel;
	bool m_cur_eop;
	emu_timer *m_dn3000_timer;
};

MACHINE_CONFIG_EXTERN( apollo );
MACHINE_CONFIG_EXTERN( apollo_terminal );

/*----------- machine/apollo_config.c -----------*/

// configuration bit definitions

#define APOLLO_CONF_SERVICE_MODE 0x0001
#define APOLLO_CONF_DISPLAY      0x001e
#define APOLLO_CONF_8_PLANES     0x0002
#define APOLLO_CONF_4_PLANES     0x0004
#define APOLLO_CONF_MONO_15I     0x0008
#define APOLLO_CONF_MONO_19I     0x0010
#define APOLLO_CONF_GERMAN_KBD   0x0020
#define APOLLO_CONF_20_YEARS_AGO 0x0040
#define APOLLO_CONF_25_YEARS_AGO 0x0080
#define APOLLO_CONF_NODE_ID      0x0100
#define APOLLO_CONF_IDLE_SLEEP   0x0200
#define APOLLO_CONF_TRAP_TRACE   0x0400
#define APOLLO_CONF_FPU_TRACE    0x0800
#define APOLLO_CONF_DISK_TRACE   0x1000
#define APOLLO_CONF_NET_TRACE    0x2000

// check configuration setting
int apollo_config(int mask);

INPUT_PORTS_EXTERN(apollo_config);

/*----------- machine/apollo_csr.c -----------*/

#define APOLLO_CSR_SR_SERVICE            0x0001
#define APOLLO_CSR_SR_ATBUS_IO_TIMEOUT   0x0002
#define APOLLO_CSR_SR_FP_TRAP            0x0004
#define APOLLO_CSR_SR_INTERRUPT_PENDING  0x0008 // DN3000 only
#define APOLLO_CSR_SR_PARITY_BYTE_MASK   0x00f0
#define APOLLO_CSR_SR_CPU_TIMEOUT        0x0100
#define APOLLO_CSR_SR_ATBUS_MEM_TIMEOUT  0x2000
#define APOLLO_CSR_SR_BIT15              0x8000
#define APOLLO_CSR_SR_CLEAR_ALL          0x3ffe

#define APOLLO_CSR_CR_INTERRUPT_ENABLE   0x0001
#define APOLLO_CSR_CR_RESET_DEVICES       0x0002
#define APOLLO_CSR_CR_FPU_TRAP_ENABLE    0x0004
#define APOLLO_CSR_CR_FORCE_BAD_PARITY   0x0008
#define APOLLO_CSR_CR_PARITY_BYTE_MASK   0x00f0

UINT16 apollo_csr_get_control_register(void);
UINT16 apollo_csr_get_status_register(void);
void apollo_csr_set_status_register(UINT16 mask, UINT16 data);

/*----------- machine/apollo_sio.c -----------*/

#define MCFG_APOLLO_SIO_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, APOLLO_SIO, _clock)

#define MCFG_APOLLO_SIO_IRQ_CALLBACK(_cb) \
	devcb = &apollo_sio::set_irq_cb(*device, DEVCB_##_cb);

#define MCFG_APOLLO_SIO_A_TX_CALLBACK(_cb) \
	devcb = &apollo_sio::set_a_tx_cb(*device, DEVCB_##_cb);

#define MCFG_APOLLO_SIO_B_TX_CALLBACK(_cb) \
	devcb = &apollo_sio::set_b_tx_cb(*device, DEVCB_##_cb);

#define MCFG_APOLLO_SIO_OUTPORT_CALLBACK(_cb) \
	devcb = &apollo_sio::set_outport_cb(*device, DEVCB_##_cb);

class apollo_sio: public mc68681_device
{
public:
	apollo_sio(const machine_config &mconfig, const char *tag,
			device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	virtual void device_reset();

private:
		UINT8 m_csrb;
		UINT8 m_ip6;
};

extern const device_type APOLLO_SIO;

/*----------- machine/apollo_ni.c -----------*/

#define MCFG_APOLLO_NI_ADD(_tag, _xtal) \
	MCFG_DEVICE_ADD(_tag, APOLLO_NI, _xtal)

/*** Apollo Node ID device ***/

class apollo_ni: public device_t, public device_image_interface
{
public:
	// construction/destruction
	apollo_ni(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~apollo_ni();

	virtual iodevice_t image_type() const { return IO_ROM; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 1; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return NULL; }
	virtual const char *file_extensions() const { return "ani,bin"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	DECLARE_WRITE16_MEMBER(write);
	DECLARE_READ16_MEMBER(read);

	// image-level overrides
	virtual bool call_load();
	virtual bool call_create(int format_type, option_resolution *format_options);
	virtual void call_unload();

	void set_node_id_from_disk();

protected:
	// device-level overrides
	virtual void device_config_complete() ;
	virtual void device_start();
	virtual void device_reset();

private:
	void set_node_id(UINT32 node_id);
	UINT32 m_node_id;
};

// device type definition
extern const device_type APOLLO_NI;

/*----------- video/apollo.c -----------*/

#define APOLLO_SCREEN_TAG "apollo_screen"

class apollo_graphics_15i : public device_t
{
public:
	apollo_graphics_15i(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	apollo_graphics_15i(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, device_type type, const char *name, const char *shortname, const char *source);
	~apollo_graphics_15i();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// monochrome control
	DECLARE_READ8_MEMBER( apollo_mcr_r );
	DECLARE_WRITE8_MEMBER( apollo_mcr_w );

	// monochrome and color memory
	DECLARE_READ16_MEMBER( apollo_mem_r );
	DECLARE_WRITE16_MEMBER( apollo_mem_w );

	// color control
	DECLARE_READ8_MEMBER( apollo_ccr_r );
	DECLARE_WRITE8_MEMBER( apollo_ccr_w );

	DECLARE_READ16_MEMBER( apollo_mgm_r );
	DECLARE_WRITE16_MEMBER( apollo_mgm_w );

	DECLARE_READ16_MEMBER( apollo_cgm_r );
	DECLARE_WRITE16_MEMBER( apollo_cgm_w );

	void vblank_state_changed(screen_device &screen, bool vblank_state);

	int is_mono() { return m_n_planes == 1; }

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
protected:
	class lut_fifo;
	class bt458;

	const char *cr_text(offs_t offset, UINT8 data, UINT8 rw);

	void increment_h_clock();
	void increment_v_clock();
	void increment_p_clock();

	void log_cr1(const char * text);
	void set_cr1(UINT8 data);
	void set_cr3a(UINT8 data);
	void set_cr3b(UINT8 data);
	void set_lut_cr(UINT8 data);

	void register_vblank_callback();

	UINT32 set_msb0(UINT32 value, UINT8 data)
	{
		return (value & 0xffffff00) | data;
	}
	UINT32 set_lsb0(UINT32 value, UINT8 data)
	{
		return (value & 0xffff00ff) | (data << 8);
	}
	UINT32 set_msb1(UINT32 value, UINT8 data)
	{
		return (value & 0xff00ffff) | (data << 16);
	}
	UINT32 set_lsb1(UINT32 value, UINT8 data)
	{
		return (value & 0x00ffffff) | (data << 24);
	}
	UINT8 get_msb1(UINT32 value)
	{
		return (value >> 16) & 0xff;
	}
	UINT8 get_lsb1(UINT32 value)
	{
		return (value >> 24) & 0xff;
	}

	void set_status_rmw();
	UINT16 rop(UINT16 dest_data, UINT16 src_data, UINT8 plane);
	void set_source_data(UINT32 offset);
	UINT32 get_source_data(UINT8 plane);
	void blt(UINT32 dest_addr, UINT16 mem_mask);

	UINT8 get_pixel(UINT32 offset, UINT16 mask);
	UINT8 c4p_read_adc(UINT8 data);
	UINT8 c8p_read_adc(UINT8 data);

	void screen_update1(bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
	UINT16 m_n_planes;
	UINT16 m_width;
	UINT16 m_height;
	UINT16 m_buffer_width;
	UINT16 m_buffer_height;

	UINT8 m_sr;
	UINT8 m_device_id;
	UINT16 m_write_enable_register;
	UINT32 m_rop_register;
	UINT16 m_diag_mem_request;
	UINT8 m_cr0;
	UINT8 m_cr1;
	UINT8 m_cr2;
	UINT8 m_cr2b;
	UINT8 m_cr2_s_data;
	UINT8 m_cr2_s_plane;
	UINT8 m_cr2_d_plane;
	UINT8 m_cr3a;
	UINT8 m_cr3b;
	UINT8 m_ad_result;
	UINT8 m_ad_pending;

	UINT8 m_lut_control;
	UINT8 m_lut_data;

	UINT8 m_update_flag;
	UINT8 m_update_pending;

	UINT8 m_blt_cycle_count;
	UINT32 m_image_offset;
	UINT32 m_guard_latch[8];

	int m_h_clock;
	int m_v_clock;
	int m_p_clock;
	int m_data_clock;

	UINT16 *m_image_memory;
	int m_image_plane_size;
	int m_image_memory_size;

	UINT32 m_color_lookup_table[16];

	lut_fifo *m_lut_fifo;
	bt458 *m_bt458;
};


#define LUT_FIFO_SIZE   1024


//**************************************************************************
// class LUT Fifo
//**************************************************************************

class apollo_graphics_15i::lut_fifo
{
public:
	lut_fifo()
	{
		reset();
	}

	void reset()
	{
		m_size = LUT_FIFO_SIZE;
		m_get_index = 0;
		m_put_index = 0;
	}

	void put(const UINT8 data)
	{
		if (!is_full())
		{
			m_data[m_put_index] = data;
			m_put_index = (m_put_index + 1) % m_size;
		}
	}

	UINT8 get()
	{
		UINT8 data = is_empty() ? 0xff : m_data[m_get_index];
		m_get_index = (m_get_index + 1) % m_size;
		return data;
	}

	int is_empty()
	{
		return m_get_index == m_put_index;
	}

	int is_full()
	{
		return ((m_put_index + 1) % m_size) == m_get_index;
	}

private:
	UINT16 m_size;
	UINT16 m_get_index;
	UINT16 m_put_index;
	UINT8 m_data[LUT_FIFO_SIZE];
};

//**************************************************************************
//  class Brooktree Bt458
//**************************************************************************

class apollo_graphics_15i::bt458
{
public:
	bt458(running_machine &running_machine);
	void start();
	void reset();
	UINT8 read(UINT8 c10);
	void write(UINT8 data, UINT8 c10);
	UINT32 get_rgb(UINT8 index);

private:
	running_machine &machine() const
	{
		assert(m_machine != NULL);
		return *m_machine;
	}

	UINT8 m_color_counter;
	UINT8 m_red;
	UINT8 m_green;

	UINT8 m_address_register;
	UINT32 m_color_palette_RAM[256];
	UINT32 m_overlay_color[4];
	UINT8 m_read_mask_register;
	UINT8 m_blink_mask_register;
	UINT8 m_command_register;
	UINT8 m_control_test_register;

	running_machine *m_machine;
};

extern const device_type APOLLO_GRAPHICS;

#define MCFG_APOLLO_GRAPHICS_ADD( _tag) \
	MCFG_FRAGMENT_ADD(apollo_graphics) \
	MCFG_DEVICE_ADD(_tag, APOLLO_GRAPHICS, 0)

MACHINE_CONFIG_EXTERN( apollo_graphics );

class apollo_graphics_19i : public apollo_graphics_15i
{
public:
	apollo_graphics_19i(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
};

extern const device_type APOLLO_MONO19I;

#define MCFG_APOLLO_MONO19I_ADD(_tag) \
	MCFG_FRAGMENT_ADD(apollo_mono19i) \
	MCFG_DEVICE_ADD(_tag, APOLLO_MONO19I, 0)

MACHINE_CONFIG_EXTERN( apollo_mono19i );

#ifdef APOLLO_XXL

/*----------- machine/apollo_stdio.c -----------*/

//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_APOLLO_STDIO_TX_CALLBACK(_cb) \
	devcb = &apollo_stdio_device::set_tx_cb(*device, DEVCB_##_cb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apollo_stdio_device

class apollo_stdio_device: public device_t, public device_serial_interface
{
public:
	// construction/destruction
	apollo_stdio_device(const machine_config &mconfig, const char *tag,
			device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_tx_cb(device_t &device, _Object object)
	{
		return downcast<apollo_stdio_device &> (device).m_tx_w.set_callback(object);
	}

	devcb_write_line m_tx_w;

private:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// serial overrides
	virtual void rcv_complete(); // Rx completed receiving byte
	virtual void tra_complete(); // Tx completed sending byte
	virtual void tra_callback(); // Tx send bit

	TIMER_CALLBACK_MEMBER( poll_timer );
	void xmit_char(UINT8 data);

	static const int XMIT_RING_SIZE = 64;

	UINT8 m_xmitring[XMIT_RING_SIZE];
	int m_xmit_read, m_xmit_write;
	bool m_tx_busy;

	emu_timer* m_poll_timer;
};

// device type definition
extern const device_type APOLLO_STDIO;
#endif /* APOLLO_XXL */

#endif /* APOLLO_H_ */
