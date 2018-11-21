// license:BSD-3-Clause
// copyright-holders:Sven Schnelle

#ifndef MAME_BUS_HPDIO_98643_H
#define MAME_BUS_HPDIO_98643_H

#pragma once

#include "hp_dio.h"
#include "machine/am79c90.h"

namespace bus { namespace hp_dio {

static constexpr int REG_SWITCHES_REMOTE = 0x80;

static constexpr int REG_SWITCHES_SELECT_CODE_MASK = 0x1f;
static constexpr int REG_SWITCHES_SELECT_CODE_SHIFT = 0x00;

static constexpr int REG_SWITCHES_INT_LEVEL_MASK = 0x03;
static constexpr int REG_SWITCHES_INT_LEVEL_SHIFT = 0x05;

static constexpr uint16_t REG_ID = 0x15;

static constexpr uint16_t REG_STATUS_ACK = 0x04;

static constexpr uint16_t REG_SC_REV = 0x01;
static constexpr uint16_t REG_SC_LOCK = 0x08;
static constexpr uint16_t REG_SC_IP = 0x40;
static constexpr uint16_t REG_SC_IE = 0x80;

class dio16_98643_device :
		public device_t,
		public device_dio16_card_interface
{
public:
	// construction/destruction
	dio16_98643_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	dio16_98643_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);


	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<am7990_device> m_lance;
	required_ioport m_switches;

	void addrmap(address_map &map);
	void update_int();
	int get_irq_line();

	READ16_MEMBER(sc_r);
	WRITE16_MEMBER(sc_w);
	READ16_MEMBER(id_r);
	WRITE16_MEMBER(id_w);
	READ16_MEMBER(novram_r);
	WRITE16_MEMBER(novram_w);

	DECLARE_WRITE_LINE_MEMBER(lance_int_w);
	DECLARE_WRITE16_MEMBER(lance_dma_out);
	DECLARE_READ16_MEMBER(lance_dma_in);

	uint16_t m_novram[32] = {
		0xfff0, 0xfff0, 0xfff0, 0xfff0,
		0xfff0, 0xfff8, 0xfff0, 0xfff0,
		0xfff0, 0xfff9, 0xfff0, 0xfff6,
		0xfffa, 0xfff1, 0xffff, 0xfff1,
		0xfff0, 0xfff0, 0xfff0, 0xfff0,
		0xfff0, 0xfff0, 0xfff0, 0xfff0,
		0xfff0, 0xfff0, 0xfff0, 0xfff0,
		0xfff0, 0xfff0, 0xfffa, 0xfff9,
	};

	std::array<uint16_t, 8192> m_ram;

	uint16_t m_sc;
	bool m_installed_io;
};

} // namespace bus::hp_dio
} // namespace bus

// device type definition
DECLARE_DEVICE_TYPE_NS(HPDIO_98643, bus::hp_dio, dio16_98643_device)

#endif // MAME_BUS_HPDIO_98643_H
