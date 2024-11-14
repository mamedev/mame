// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, R. Belmont
/*
 * apollo.h - APOLLO DN3500/DN3000 driver includes
 *
 *  Created on: May 12, 2010
 *      Author: Hans Ostermeyer
 *
 */

#ifndef MAME_APOLLO_APOLLO_H
#define MAME_APOLLO_APOLLO_H

#pragma once


#include "apollo_kbd.h"

#include "cpu/m68000/m68020.h"
#include "cpu/m68000/m68030.h"
#include "cpu/m68000/m68040.h"

#include "machine/6840ptm.h"
#include "machine/am9517a.h"
#include "machine/clock.h"
#include "machine/mc146818.h"
#include "machine/mc68681.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "machine/terminal.h"

#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/isa/3c505.h"
#include "bus/isa/omti8621.h"

#include "bus/rs232/rs232.h"

#include "diserial.h"
#include "screen.h"

#ifndef VERBOSE
#define VERBOSE 0
#endif

#define LOG(x)  { logerror x; logerror ("\n"); apollo_check_log(); }
#define LOG1(x) { if (VERBOSE > 0) LOG(x) }
#define LOG2(x) { if (VERBOSE > 1) LOG(x) }
#define CLOG(x) { machine().logerror ("%s - %s: ", apollo_cpu_context(machine()), tag()); machine().logerror x; machine().logerror ("\n"); apollo_check_log(); }
#define CLOG1(x) { if (VERBOSE > 0) CLOG(x) }
#define CLOG2(x) { if (VERBOSE > 1) CLOG(x) }
#define DLOG(x) { device->logerror ("%s - %s: ", apollo_cpu_context(device->machine()), device->tag()); device->logerror x; device->logerror ("\n"); apollo_check_log(); }
#define DLOG1(x) { if (VERBOSE > 0) DLOG(x) }
#define DLOG2(x) { if (VERBOSE > 1) DLOG(x) }
#define MLOG(x)  { machine().logerror ("%s: ", apollo_cpu_context(machine())); machine().logerror x; machine().logerror ("\n"); apollo_check_log(); }
#define MLOG1(x) { if (VERBOSE > 0) MLOG(x) }
#define MLOG2(x) { if (VERBOSE > 1) MLOG(x) }
#define SLOG(x)  { machine().logerror ("%s: ", apollo_cpu_context(machine()));machine().logerror x; machine().logerror ("\n"); apollo_check_log(); }
#define SLOG1(x) { if (VERBOSE > 0) SLOG(x) }
#define SLOG2(x) { if (VERBOSE > 1) SLOG(x) }

#define  MAINCPU "maincpu"

// Enabling this is >NOT< supported by MESSdev
// Do *not* report any issues on Mametesters if this is enabled!
// #define APOLLO_XXL

/*----------- drivers/apollo.cpp -----------*/

// return the current CPU context for log file entries
std::string apollo_cpu_context(running_machine &machine);

// enable/disable the FPU
void apollo_set_cpu_has_fpu(m68000_musashi_device *device, int onoff);

// check for excessive logging
void apollo_check_log();

// return 1 if node is DN3000 or DSP3000, 0 otherwise
int apollo_is_dn3000(void);

// return 1 if node is DN5500 or DSP5500, 0 otherwise
int apollo_is_dn5500(void);

// return 1 if node is DSP3000 or DSP3500, 0 otherwise
int apollo_is_dsp3x00(void);

// get the ram configuration byte
uint8_t apollo_get_ram_config_byte(void);

//apollo_get_node_id - get the node id
uint32_t apollo_get_node_id(void);

void apollo_set_cache_status_register(device_t *device,uint8_t mask, uint8_t data);

/*----------- machine/apollo.cpp -----------*/

#define APOLLO_CONF_TAG "conf"
#define APOLLO_DMA1_TAG "dma8237_1"
#define APOLLO_DMA2_TAG "dma8237_2"
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
#define APOLLO_SCREEN_TAG "apollo_screen"
#define APOLLO_KBD_TAG  "kbd"


// forward declaration
class apollo_sio;
class apollo_ni;
class apollo_graphics_15i;
class apollo_kbd_device;

class apollo_state : public driver_device
{
public:
	apollo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, MAINCPU),
		m_ram(*this, RAM_TAG),
		m_messram_ptr(*this, RAM_TAG),
		m_dma8237_1(*this, APOLLO_DMA1_TAG),
		m_dma8237_2(*this, APOLLO_DMA2_TAG),
		m_pic8259_master(*this, APOLLO_PIC1_TAG),
		m_pic8259_slave(*this, APOLLO_PIC2_TAG),
		m_ptm(*this, APOLLO_PTM_TAG),
		m_sio(*this, APOLLO_SIO_TAG),
		m_sio2(*this, APOLLO_SIO2_TAG),
		m_rtc(*this, APOLLO_RTC_TAG),
		m_node_id(*this, APOLLO_NI_TAG),
		m_isa(*this, APOLLO_ISA_TAG),
		m_graphics(*this, APOLLO_SCREEN_TAG),
		m_keyboard(*this, APOLLO_KBD_TAG),
		m_internal_leds(*this, "internal_led_%u", 1U),
		m_external_leds(*this, "external_led_%c", unsigned('a'))
	{ }

	void dn3500(machine_config &config);
	void dn5500_19i(machine_config &config);
	void dn3000(machine_config &config);
	void dn3000_15i(machine_config &config);
	void dn3000_19i(machine_config &config);
	void dn3500_15i(machine_config &config);
	void dsp3000(machine_config &config);
	void dsp3500(machine_config &config);
	void dsp5500(machine_config &config);
	void dn5500(machine_config &config);
	void dn5500_15i(machine_config &config);
	void dn3500_19i(machine_config &config);

	void init_dsp3000();
	void init_dsp5500();
	void init_dn3500();
	void init_dn3000();
	void init_dsp3500();
	void init_dn5500();
	void init_apollo();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68000_musashi_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_shared_ptr<uint32_t> m_messram_ptr;

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
	optional_device<apollo_graphics_15i> m_graphics;
	optional_device<apollo_kbd_device> m_keyboard;
	output_finder<4> m_internal_leds;
	output_finder<4> m_external_leds;

	void apollo_csr_status_register_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t apollo_csr_status_register_r(offs_t offset, uint16_t mem_mask = ~0);
	void apollo_csr_control_register_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t apollo_csr_control_register_r(offs_t offset, uint16_t mem_mask = ~0);
	void apollo_dma_1_w(offs_t offset, uint8_t data);
	uint8_t apollo_dma_1_r(offs_t offset);
	void apollo_dma_2_w(offs_t offset, uint8_t data);
	uint8_t apollo_dma_2_r(offs_t offset);
	void apollo_dma_page_register_w(offs_t offset, uint8_t data);
	uint8_t apollo_dma_page_register_r(offs_t offset);
	void apollo_address_translation_map_w(offs_t offset, uint16_t data);
	uint16_t apollo_address_translation_map_r(offs_t offset);
	uint8_t apollo_dma_read_byte(offs_t offset);
	void apollo_dma_write_byte(offs_t offset, uint8_t data);
	uint8_t apollo_dma_read_word(offs_t offset);
	void apollo_dma_write_word(offs_t offset, uint8_t data);
	void apollo_rtc_w(offs_t offset, uint8_t data);
	uint8_t apollo_rtc_r(offs_t offset);
	void cache_control_register_w(offs_t offset, uint8_t data);
	uint8_t cache_status_register_r(offs_t offset);
	void task_alias_register_w(offs_t offset, uint8_t data);
	uint8_t task_alias_register_r(offs_t offset);
	void latch_page_on_parity_error_register_w(offs_t offset, uint16_t data);
	uint16_t latch_page_on_parity_error_register_r(offs_t offset);
	void master_req_register_w(offs_t offset, uint8_t data);
	uint8_t master_req_register_r(offs_t offset);
	void selective_clear_locations_w(offs_t offset, uint16_t data);
	uint16_t selective_clear_locations_r(offs_t offset);
	uint32_t ram_with_parity_r(offs_t offset, uint32_t mem_mask = ~0);
	void ram_with_parity_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t apollo_unmapped_r(offs_t offset, uint32_t mem_mask = ~0);
	void apollo_unmapped_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void apollo_rom_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t apollo_atbus_io_r(offs_t offset, uint16_t mem_mask = ~0);
	void apollo_atbus_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t apollo_atbus_memory_r(offs_t offset, uint16_t mem_mask = ~0);
	void apollo_atbus_memory_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t apollo_atbus_unmap_io_r(offs_t offset, uint16_t mem_mask = ~0);
	void apollo_atbus_unmap_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t apollo_atbus_unmap_r(offs_t offset, uint8_t mem_mask = ~0);
	void apollo_atbus_unmap_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	void dn5500_memory_present_register_w(offs_t offset, uint8_t data);
	uint8_t dn5500_memory_present_register_r(offs_t offset);
	void dn5500_11500_w(offs_t offset, uint8_t data);
	uint8_t dn5500_11500_r(offs_t offset);
	void dn5500_io_protection_map_w(offs_t offset, uint8_t data);
	uint8_t dn5500_io_protection_map_r(offs_t offset);

	DECLARE_MACHINE_RESET(apollo);
	DECLARE_MACHINE_START(apollo);

	void cpu_space_map(address_map &map) ATTR_COLD;
	u16 apollo_irq_acknowledge(offs_t offset);
	u16 apollo_pic_get_vector();
	void apollo_bus_error();
	int apollo_kbd_is_german();
	void apollo_dma8237_out_eop(int state);
	void apollo_dma_1_hrq_changed(int state);
	void apollo_dma_2_hrq_changed(int state);
	void apollo_pic8259_master_set_int_line(int state);
	void apollo_pic8259_slave_set_int_line(int state);
	void sio_irq_handler(int state);
	void sio_output(uint8_t data);
	void sio2_irq_handler(int state);
	void apollo_ptm_irq_function(int state);
	void apollo_ptm_timer_tick(int state);
	uint8_t apollo_pic8259_get_slave_ack(offs_t offset);
	void apollo_rtc_irq_function(int state);

	uint8_t pc_dma8237_0_dack_r();
	uint8_t pc_dma8237_1_dack_r();
	uint8_t pc_dma8237_2_dack_r();
	uint8_t pc_dma8237_3_dack_r();
	uint8_t pc_dma8237_5_dack_r();
	uint8_t pc_dma8237_6_dack_r();
	uint8_t pc_dma8237_7_dack_r();
	void pc_dma8237_0_dack_w(uint8_t data);
	void pc_dma8237_1_dack_w(uint8_t data);
	void pc_dma8237_2_dack_w(uint8_t data);
	void pc_dma8237_3_dack_w(uint8_t data);
	void pc_dma8237_5_dack_w(uint8_t data);
	void pc_dma8237_6_dack_w(uint8_t data);
	void pc_dma8237_7_dack_w(uint8_t data);
	void pc_dack0_w(int state);
	void pc_dack1_w(int state);
	void pc_dack2_w(int state);
	void pc_dack3_w(int state);
	void pc_dack4_w(int state);
	void pc_dack5_w(int state);
	void pc_dack6_w(int state);
	void pc_dack7_w(int state);

	void apollo_pic_set_irq_line(int irq, int state);
	void select_dma_channel(int channel, bool state);

	void apollo_reset_instr_callback(int state);

	void common(machine_config &config);
	void apollo(machine_config &config);
	void apollo_terminal(machine_config &config);

	void dn3000_map(address_map &map) ATTR_COLD;
	void dn3500_map(address_map &map) ATTR_COLD;
	void dn5500_map(address_map &map) ATTR_COLD;
	void dsp3000_map(address_map &map) ATTR_COLD;
	void dsp3500_map(address_map &map) ATTR_COLD;
	void dsp5500_map(address_map &map) ATTR_COLD;

	uint32_t ptm_counter = 0U;
	uint8_t sio_output_data = 0U;
	int m_dma_channel = 0;
	bool m_cur_eop = false;
};

/*----------- machine/apollo_config.cpp -----------*/

// configuration bit definitions

#define APOLLO_CONF_SERVICE_MODE 0x0001
#define APOLLO_CONF_DISPLAY      0x001e
#define APOLLO_CONF_8_PLANES     0x0002
#define APOLLO_CONF_4_PLANES     0x0004
#define APOLLO_CONF_MONO_15I     0x0008
#define APOLLO_CONF_MONO_19I     0x0010
#define APOLLO_CONF_GERMAN_KBD   0x0020
#define APOLLO_CONF_30_YEARS_AGO 0x0040
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

/*----------- machine/apollo_csr.cpp -----------*/

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

uint16_t apollo_csr_get_control_register(void);
uint16_t apollo_csr_get_status_register(void);
void apollo_csr_set_status_register(uint16_t mask, uint16_t data);

/*----------- machine/apollo_sio.cpp -----------*/

class apollo_sio: public duart_base_device
{
public:
	apollo_sio(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_csrb = 0U;
};

DECLARE_DEVICE_TYPE(APOLLO_SIO, apollo_sio)

/*----------- machine/apollo_ni.cpp -----------*/


/*** Apollo Node ID device ***/

class apollo_ni: public device_t, public device_image_interface
{
public:
	// construction/destruction
	apollo_ni(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~apollo_ni();

	// image-level overrides
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool support_command_line_image_creation() const noexcept override { return true; }
	virtual const char *file_extensions() const noexcept override { return "ani,bin"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;
	virtual const char *image_type_name() const noexcept override { return "node_id"; }
	virtual const char *image_brief_type_name() const noexcept override { return "ni"; }

	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t read(offs_t offset, uint16_t mem_mask = ~0);

	void set_node_id_from_disk();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	optional_device<omti8621_apollo_device> m_wdc;
	void set_node_id(uint32_t node_id);
	uint32_t m_node_id = 0U;
};

// device type definition
DECLARE_DEVICE_TYPE(APOLLO_NI, apollo_ni)

/*----------- video/apollo.cpp -----------*/

class apollo_graphics_15i : public device_t
{
public:
	apollo_graphics_15i(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~apollo_graphics_15i();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// monochrome control
	uint8_t  apollo_mcr_r(offs_t offset);
	void apollo_mcr_w(offs_t offset, uint8_t data);

	// monochrome and color memory
	uint16_t apollo_mem_r(offs_t offset, uint16_t mem_mask = ~0);
	void apollo_mem_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// color control
	uint8_t  apollo_ccr_r(offs_t offset);
	void apollo_ccr_w(offs_t offset, uint8_t data);

	uint16_t apollo_mgm_r(offs_t offset, uint16_t mem_mask = ~0);
	void apollo_mgm_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t apollo_cgm_r(offs_t offset, uint16_t mem_mask = ~0);
	void apollo_cgm_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void vblank_state_changed(screen_device &screen, bool vblank_state);

	int is_mono() { return m_n_planes == 1; }

protected:
	required_device<screen_device> m_screen;

	apollo_graphics_15i(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

protected:
	class lut_fifo;
	class bt458;

	const char *cr_text(offs_t offset, uint8_t data, uint8_t rw);

	void increment_h_clock();
	void increment_v_clock();
	void increment_p_clock();

	void log_cr1(const char * text);
	void set_cr1(uint8_t data);
	void set_cr3a(uint8_t data);
	void set_cr3b(uint8_t data);
	void set_lut_cr(uint8_t data);

	void register_vblank_callback();

	uint32_t set_msb0(uint32_t value, uint8_t data)
	{
		return (value & 0xffffff00) | data;
	}
	uint32_t set_lsb0(uint32_t value, uint8_t data)
	{
		return (value & 0xffff00ff) | (data << 8);
	}
	uint32_t set_msb1(uint32_t value, uint8_t data)
	{
		return (value & 0xff00ffff) | (data << 16);
	}
	uint32_t set_lsb1(uint32_t value, uint8_t data)
	{
		return (value & 0x00ffffff) | (data << 24);
	}
	uint8_t get_msb1(uint32_t value)
	{
		return (value >> 16) & 0xff;
	}
	uint8_t get_lsb1(uint32_t value)
	{
		return (value >> 24) & 0xff;
	}

	void set_status_rmw();
	uint16_t rop(uint16_t dest_data, uint16_t src_data, uint8_t plane);
	void set_source_data(uint32_t offset);
	uint32_t get_source_data(uint8_t plane);
	void blt(uint32_t dest_addr, uint16_t mem_mask);

	uint8_t get_pixel(uint32_t offset, uint16_t mask);
	uint8_t c4p_read_adc(uint8_t data);
	uint8_t c8p_read_adc(uint8_t data);

	void screen_update1(bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
	uint16_t m_n_planes = 0U;
	uint16_t m_width = 0U;
	uint16_t m_height = 0U;
	uint16_t m_buffer_width = 0U;
	uint16_t m_buffer_height = 0U;

	uint8_t m_sr = 0U;
	uint8_t m_device_id = 0U;
	uint16_t m_write_enable_register = 0U;
	uint32_t m_rop_register = 0U;
	uint16_t m_diag_mem_request = 0U;
	uint8_t m_cr0 = 0U;
	uint8_t m_cr1 = 0U;
	uint8_t m_cr2 = 0U;
	uint8_t m_cr2b = 0U;
	uint8_t m_cr2_s_data = 0U;
	uint8_t m_cr2_s_plane = 0U;
	uint8_t m_cr2_d_plane = 0U;
	uint8_t m_cr3a = 0U;
	uint8_t m_cr3b = 0U;
	uint8_t m_ad_result = 0U;
	uint8_t m_ad_pending = 0U;

	uint8_t m_lut_control = 0U;
	uint8_t m_lut_data = 0U;

	uint8_t m_update_flag = 0U;
	uint8_t m_update_pending = 0U;

	uint8_t m_blt_cycle_count = 0U;
	uint32_t m_image_offset = 0U;
	uint32_t m_guard_latch[8]{};

	int m_h_clock = 0;
	int m_v_clock = 0;
	int m_p_clock = 0;
	int m_data_clock = 0;

	std::unique_ptr<uint16_t[]> m_image_memory{};
	int m_image_plane_size = 0;
	int m_image_memory_size = 0;

	uint32_t m_color_lookup_table[16]{};

	std::unique_ptr<lut_fifo> m_lut_fifo;
	std::unique_ptr<bt458> m_bt458;
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

	void put(const uint8_t data)
	{
		if (!is_full())
		{
			m_data[m_put_index] = data;
			m_put_index = (m_put_index + 1) % m_size;
		}
	}

	uint8_t get()
	{
		uint8_t data = is_empty() ? 0xff : m_data[m_get_index];
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
	uint16_t m_size = 0U;
	uint16_t m_get_index = 0U;
	uint16_t m_put_index = 0U;
	uint8_t m_data[LUT_FIFO_SIZE]{};
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
	uint8_t read(uint8_t c10);
	void write(uint8_t data, uint8_t c10);
	uint32_t get_rgb(uint8_t index);

private:
	running_machine &machine() const
	{
		assert(m_machine != nullptr);
		return *m_machine;
	}

	uint8_t m_color_counter = 0U;
	uint8_t m_red = 0U;
	uint8_t m_green = 0U;

	uint8_t m_address_register = 0U;
	uint32_t m_color_palette_RAM[256]{};
	uint32_t m_overlay_color[4]{};
	uint8_t m_read_mask_register = 0U;
	uint8_t m_blink_mask_register = 0U;
	uint8_t m_command_register = 0U;
	uint8_t m_control_test_register = 0U;

	running_machine *m_machine;
};

DECLARE_DEVICE_TYPE(APOLLO_GRAPHICS, apollo_graphics_15i)


class apollo_graphics_19i : public apollo_graphics_15i
{
public:
	apollo_graphics_19i(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal state
};

DECLARE_DEVICE_TYPE(APOLLO_MONO19I, apollo_graphics_19i)

#ifdef APOLLO_XXL

/*----------- machine/apollo_stdio.cpp -----------*/


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apollo_stdio_device

class apollo_stdio_device: public device_t, public device_serial_interface
{
public:
	// construction/destruction
	apollo_stdio_device(const machine_config &mconfig, const char *tag,
			device_t *owner, uint32_t clock);

	auto tx_cb() { return m_tx_w.bind(); }

private:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// serial overrides
	virtual void rcv_complete() override; // Rx completed receiving byte
	virtual void tra_complete() override; // Tx completed sending byte
	virtual void tra_callback() override; // Tx send bit

	TIMER_CALLBACK_MEMBER( poll_timer );
	void xmit_char(uint8_t data);

	static const int XMIT_RING_SIZE = 64;

	uint8_t m_xmitring[XMIT_RING_SIZE]{};
	int m_xmit_read = 0, m_xmit_write = 0;
	bool m_tx_busy = false;

	emu_timer* m_poll_timer = nullptr;

	devcb_write_line m_tx_w;
};

// device type definition
DECLARE_DEVICE_TYPE(APOLLO_STDIO, apollo_stdio_device)
#endif /* APOLLO_XXL */

#endif // MAME_APOLLO_APOLLO_H
