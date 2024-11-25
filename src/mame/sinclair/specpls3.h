// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/specpls3.h
 *
 ****************************************************************************/

#ifndef MAME_SINCLAIR_SPECPLS3_H
#define MAME_SINCLAIR_SPECPLS3_H

#include "spectrum.h"
#include "spec128.h"

#include "imagedev/floppy.h"
#include "machine/upd765.h"

INPUT_PORTS_EXTERN( spec_plus );

class specpls3_state : public spectrum_128_state
{
public:
	specpls3_state(const machine_config &mconfig, device_type type, const char *tag) :
		spectrum_128_state(mconfig, type, tag),
		m_upd765(*this, "upd765"),
		m_flop(*this, "upd765:%u", 0U)
	{ }

	void spectrum_plus2(machine_config &config);
	void spectrum_plus3(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void plus3_update_memory() override;
	virtual bool is_contended(offs_t offset) override;

private:
	void rom_w(offs_t offset, uint8_t data);
	uint8_t rom_r(offs_t offset);
	void port_3ffd_w(offs_t offset, uint8_t data);
	uint8_t port_3ffd_r();
	uint8_t port_2ffd_r();
	void port_7ffd_w(offs_t offset, uint8_t data);
	void port_1ffd_w(offs_t offset, uint8_t data);
	void plus3_us_w(uint8_t data);

	void plus3_io(address_map &map) ATTR_COLD;
	void plus3_mem(address_map &map) ATTR_COLD;

	static void floppy_formats(format_registration &fr);

	optional_device<upd765a_device> m_upd765;
	optional_device_array<floppy_connector, 2> m_flop;
};

#endif // MAME_SINCLAIR_SPECPLS3_H
