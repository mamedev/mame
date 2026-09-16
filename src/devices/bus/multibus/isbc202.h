// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    isbc202.h

    Intel iSBC-202 SSDD 8" floppy disk controller

*********************************************************************/

#ifndef MAME_BUS_MULTIBUS_ISBC202_H
#define MAME_BUS_MULTIBUS_ISBC202_H

#pragma once

#include "multibus.h"
#include "machine/i3001.h"
#include "machine/i3002.h"
#include "machine/fdc_pll.h"
#include "imagedev/floppy.h"

class isbc202_device : public cpu_device,
					   public device_multibus_interface
{
public:
	// Construction/destruction
	isbc202_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~isbc202_device();

	// Access to I/O space by CPU
	uint8_t io_r(address_space &space, offs_t offset);
	void io_w(address_space &space, offs_t offset, uint8_t data);

	void co_w(int state);

	uint8_t px_r();

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	required_device<i3001_device> m_mcu;
	required_device_array<i3002_device , 4> m_cpes;
	required_device_array<floppy_connector , 4> m_drives;

	address_space_config m_program_config;
	memory_access<9, 2 , -2 , ENDIANNESS_BIG>::cache m_cache;

	address_space *m_mem_space;

	int m_icount;
	uint8_t m_flags;
	uint8_t m_regs[ i3002_device::REG_COUNT ];
	uint16_t m_microcode_addr;
	uint32_t m_code_word;
	uint8_t m_ac;
	uint8_t m_fc;
	bool m_fc32;
	bool m_fc10;
	enum {              // m_in_sel     Input on AC[0]
		  IN_SEL_AC0,   // 0            AC[0]
		  IN_SEL_CO,    // 1            CO
		  IN_SEL_START, // 2            START
		  IN_SEL_F,     // 3            F
		  IN_SEL_AZ,    // 4            AZ
		  IN_SEL_INDEX, // 5            INDEX
		  IN_SEL_XFERQ, // 6            XFERREQ
		  IN_SEL_TIMEOUT,//7            TIMEOUT
		  IN_SEL_COUNT
	};
	uint8_t m_in_sel;
	uint8_t m_out_sel;
	uint8_t m_slk;
	uint8_t m_mask;
	uint8_t m_kbus;
	bool m_inputs[ IN_SEL_COUNT ];
	// IC identifiers have this form: x-Ayy
	// where x is either C (for channel board) or I (for interface board)
	// and yy is the numeric IC ID.
	uint8_t m_op_us;    // C-A44
	floppy_image_device *m_current_drive;
	uint8_t m_px_s1s0;  // C-A16-11 & C-A16-13
	uint8_t m_cmd;      // C-A8
	bool m_2nd_pass;
	bool m_cpu_rd;
	uint8_t m_ready_in;
	uint8_t m_ready_ff; // I-A44 & I-A43
	bool m_gate_lower;  // I-A58-9
	bool m_irq;         // C-A37-9
	uint8_t m_data_low_out; // C-A43
	uint8_t m_data_low_in;  // C-A25
	uint8_t m_cpu_data;
	uint8_t m_addr_low_out; // C-A41
	bool m_mem_wrt;     // I-A58-4
	bool m_wrt_inh;     // I-A46-13
	bool m_direction;   // I-A58-11
	bool m_ibus_cached;
	uint8_t m_ibus;
	uint16_t m_crc;     // I-A62
	bool m_crc_enabled; // I-A58-13
	bool m_crc_out;     // C-A16-2
	bool m_reading;     // I-A46-9
	bool m_writing;     // C-A16-4
	uint16_t m_data_sr; // C-A36 & C-A29 (MSB), next byte in LSB
	bool m_last_data_bit;   // I-A48-6
	uint16_t m_clock_sr;    // C-A34 & C-A27 (MSB), next byte in LSB
	attotime m_last_f_time;
	bool m_clock_gate;  // I-A10-8
	bool m_amwrt;       // I-A58-6
	bool m_dlyd_amwrt;

	// PLL
	fdc_pll_t m_pll;

	// Timers
	emu_timer *m_timeout_timer;
	emu_timer *m_byte_timer;
	emu_timer *m_f_timer;

	TIMER_CALLBACK_MEMBER(timeout_tick);
	TIMER_CALLBACK_MEMBER(byte_tick);
	TIMER_CALLBACK_MEMBER(f_tick);

	void set_output();
	unsigned selected_drive() const;
	unsigned drive_idx(floppy_image_device *drive);
	void floppy_ready_cb(floppy_image_device *floppy , int state);
	void floppy_index_cb(floppy_image_device *floppy , int state);
	uint8_t dbus_r() const;
	uint8_t mbus_r() const;
	uint8_t abus_r() const;
	uint8_t ibus_r();
	void set_start(uint8_t off , bool read);
	void set_rd_wr(bool new_rd , bool new_wr);
	uint8_t aligned_rd_data(uint16_t sr);
	void rd_bits(unsigned n);
	void write_byte();
	bool update_crc(bool bit);
};

// device type declaration
DECLARE_DEVICE_TYPE(ISBC202, isbc202_device)

#endif /* MAME_BUS_MULTIBUS_ISBC202_H */
