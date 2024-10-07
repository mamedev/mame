// license:BSD-3-Clause
// copyright-holders:David Haywood,hap
/***************************************************************************

    Toshiba TMPZ84C011, MPUZ80/TLCS-Z80 ASSP Family
    Z80 CPU, CTC, CGC, I/O8x5

***************************************************************************/

#ifndef MAME_CPU_Z80_TMPZ84C011_H
#define MAME_CPU_Z80_TMPZ84C011_H

#pragma once

#include "z80.h"
#include "machine/z80ctc.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

// For daisy chain configuration, insert this:
#define TMPZ84C011_DAISY_INTERNAL { "tmpz84c011_ctc" }

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class tmpz84c011_device : public z80_device
{
public:
	tmpz84c011_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto zc0_callback() { return m_zc0_cb.bind(); }
	auto zc1_callback() { return m_zc1_cb.bind(); }
	auto zc2_callback() { return m_zc2_cb.bind(); }

	auto out_pa_callback() { return m_outportsa.bind(); }
	auto out_pb_callback() { return m_outportsb.bind(); }
	auto out_pc_callback() { return m_outportsc.bind(); }
	auto out_pd_callback() { return m_outportsd.bind(); }
	auto out_pe_callback() { return m_outportse.bind(); }

	auto in_pa_callback() { return m_inportsa.bind(); }
	auto in_pb_callback() { return m_inportsb.bind(); }
	auto in_pc_callback() { return m_inportsc.bind(); }
	auto in_pd_callback() { return m_inportsd.bind(); }
	auto in_pe_callback() { return m_inportse.bind(); }

	// CTC public interface
	void trg0(int state) { m_ctc->trg0(state); }
	void trg1(int state) { m_ctc->trg1(state); }
	void trg2(int state) { m_ctc->trg2(state); }
	void trg3(int state) { m_ctc->trg3(state); }

	/////////////////////////////////////////////////////////

	uint8_t tmpz84c011_pa_r() { return (m_inportsa() & ~m_pio_dir[0]) | (m_pio_latch[0] & m_pio_dir[0]); }
	uint8_t tmpz84c011_pb_r() { return (m_inportsb() & ~m_pio_dir[1]) | (m_pio_latch[1] & m_pio_dir[1]); }
	uint8_t tmpz84c011_pc_r() { return (m_inportsc() & ~m_pio_dir[2]) | (m_pio_latch[2] & m_pio_dir[2]); }
	uint8_t tmpz84c011_pd_r() { return (m_inportsd() & ~m_pio_dir[3]) | (m_pio_latch[3] & m_pio_dir[3]); }
	uint8_t tmpz84c011_pe_r() { return (m_inportse() & ~m_pio_dir[4]) | (m_pio_latch[4] & m_pio_dir[4]); }
	void tmpz84c011_pa_w(uint8_t data) { m_pio_latch[0] = data; m_outportsa(data & m_pio_dir[0]); }
	void tmpz84c011_pb_w(uint8_t data) { m_pio_latch[1] = data; m_outportsb(data & m_pio_dir[1]); }
	void tmpz84c011_pc_w(uint8_t data) { m_pio_latch[2] = data; m_outportsc(data & m_pio_dir[2]); }
	void tmpz84c011_pd_w(uint8_t data) { m_pio_latch[3] = data; m_outportsd(data & m_pio_dir[3]); }
	void tmpz84c011_pe_w(uint8_t data) { m_pio_latch[4] = data; m_outportse(data & m_pio_dir[4]); }

	uint8_t tmpz84c011_dir_pa_r() { return m_pio_dir[0]; }
	uint8_t tmpz84c011_dir_pb_r() { return m_pio_dir[1]; }
	uint8_t tmpz84c011_dir_pc_r() { return m_pio_dir[2]; }
	uint8_t tmpz84c011_dir_pd_r() { return m_pio_dir[3]; }
	uint8_t tmpz84c011_dir_pe_r() { return m_pio_dir[4]; }
	void tmpz84c011_dir_pa_w(uint8_t data) { m_pio_dir[0] = data; }
	void tmpz84c011_dir_pb_w(uint8_t data) { m_pio_dir[1] = data; }
	void tmpz84c011_dir_pc_w(uint8_t data) { m_pio_dir[2] = data; }
	void tmpz84c011_dir_pd_w(uint8_t data) { m_pio_dir[3] = data; }
	void tmpz84c011_dir_pe_w(uint8_t data) { m_pio_dir[4] = data; }

	void tmpz84c011_internal_io_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	const address_space_config m_io_space_config;

	virtual space_config_vector memory_space_config() const override;

private:
	// devices/pointers
	required_device<z80ctc_device> m_ctc;

	// internal state
	uint8_t m_pio_dir[5];
	uint8_t m_pio_latch[5];

	// callbacks
	devcb_write8 m_outportsa;
	devcb_write8 m_outportsb;
	devcb_write8 m_outportsc;
	devcb_write8 m_outportsd;
	devcb_write8 m_outportse;

	devcb_read8 m_inportsa;
	devcb_read8 m_inportsb;
	devcb_read8 m_inportsc;
	devcb_read8 m_inportsd;
	devcb_read8 m_inportse;

	devcb_write_line m_zc0_cb;
	devcb_write_line m_zc1_cb;
	devcb_write_line m_zc2_cb;

	void zc0_cb_trampoline_w(int state) { m_zc0_cb(state); }
	void zc1_cb_trampoline_w(int state) { m_zc1_cb(state); }
	void zc2_cb_trampoline_w(int state) { m_zc2_cb(state); }
};


// device type definition
DECLARE_DEVICE_TYPE(TMPZ84C011, tmpz84c011_device)


#endif // MAME_CPU_Z80_TMPZ84C011_H
