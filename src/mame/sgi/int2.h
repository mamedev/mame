// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SGI_INT2_H
#define MAME_SGI_INT2_H

#pragma once

#include "machine/input_merger.h"
#include "machine/pit8253.h"

class sgi_int2_device
	: public device_t
{
public:
	sgi_int2_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	template <unsigned N> auto write_led() { return m_led[N].bind(); }
	auto write_poweroff() { return m_poweroff.bind(); }
	template <unsigned N> auto write_intr() { return m_intr[N].bind(); }

	enum lio_int : unsigned
	{
		LIO0_GIO0     = 0, // gio0/fifo full
		LIO0_PARALLEL = 1,
		LIO0_SCSI     = 2,
		LIO0_ETHERNET = 3,
		LIO0_GDMA     = 4, // graphics dma done
		LIO0_DUART    = 5,
		LIO0_GIO1     = 6, // gio1/ge/second hpc1
		LIO0_VME0     = 7,

		LIO1_GR1MODE  = 1,
		LIO1_VME1     = 3,
		LIO1_DSP      = 4, // hpc
		LIO1_ACFAIL   = 5,
		LIO1_VIDEO    = 6, // video option
		LIO1_GIO2     = 7, // gio2/vertical retrace
	};
	// interrupt request inputs
	template <unsigned N> void lio0_w(int state);
	template <unsigned N> void lio1_w(int state);
	template <unsigned N> void vme_w(int state);

	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// read handlers
	template <unsigned N> u8 lstatus_r() { return m_lstatus[N]; }
	template <unsigned N> u8 lmask_r() { return m_lmask[N]; }
	u8 vstatus_r() { return m_vstatus; }
	template <unsigned N> u8 vmask_r() { return m_vmask[N]; }
	u8 config_r() { return m_config; }

	// write handlers
	template <unsigned N> void lmask_w(u8 data);
	template <unsigned N> void vmask_w(u8 data);
	void config_w(u8 data);
	void tclear_w(u8 data);

	void lio_update();

private:
	required_device<pit8254_device> m_pit;
	required_device_array<input_merger_any_high_device, 2> m_vme;

	devcb_write_line::array<4> m_led;
	devcb_write_line m_poweroff;
	devcb_write_line::array<6> m_intr;
	bool m_intr_state[6];

	u8 m_lstatus[2];
	u8 m_lmask[2];
	u8 m_vstatus;
	u8 m_vmask[2];
	u8 m_config;
};

DECLARE_DEVICE_TYPE(SGI_INT2, sgi_int2_device)

#endif // MAME_SGI_INT2_H
