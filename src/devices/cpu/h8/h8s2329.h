// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8s2329.h

    H8S-2329 family emulation
    (compared to H8S-2319 family: adds DMA, 1 more SCI)

    H8S/2000-based mcus.

    Variant         ROM         RAM         note
    H8S/2320         -           4K
    H8S/2321         -           4K         no DMA
    H8S/2322         -           8K
    H8S/2323        32K          8K
    H8S/2324         -          32K
    H8S/2326       512K          8K
    H8S/2327       128K          8K
    H8S/2328       256K          8K
    H8S/2329       384K         32K

***************************************************************************/

#ifndef MAME_CPU_H8_H8S2329_H
#define MAME_CPU_H8_H8S2329_H

#pragma once

#include "h8s2319.h"
#include "h8_dma.h"

class h8s2321_device : public h8s2319_device {
public:
	h8s2321_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// see h8s2319.h for more
	auto read_port5()  { return m_read_port [PORT_5].bind(); }
	auto write_port5() { return m_write_port[PORT_5].bind(); }
	auto read_port6()  { return m_read_port [PORT_6].bind(); }
	auto write_port6() { return m_write_port[PORT_6].bind(); }

protected:
	h8s2321_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map_delegate, u32 rom_size, u32 ram_size);

	required_device<h8_port_device> m_port5;
	required_device<h8_port_device> m_port6;

	virtual void notify_standby(int state) override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void map_2321(address_map &map) ATTR_COLD;
};

class h8s2320_device : public h8s2321_device {
public:
	h8s2320_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto tend0_cb() { return m_tend_cb[0].bind(); }
	auto tend1_cb() { return m_tend_cb[1].bind(); }

protected:
	required_device<h8s_dma_device> m_dma;
	required_device_array<h8s_dma_channel_device, 2> m_dmac;

	devcb_write_line::array<2> m_tend_cb;

	h8s2320_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 rom_size, u32 ram_size);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void map_2320(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void execute_set_input(int inputnum, int state) override;
};

class h8s2322_device : public h8s2320_device {
public:
	h8s2322_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2323_device : public h8s2320_device {
public:
	h8s2323_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2324_device : public h8s2320_device {
public:
	h8s2324_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2326_device : public h8s2320_device {
public:
	h8s2326_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2327_device : public h8s2320_device {
public:
	h8s2327_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2328_device : public h8s2320_device {
public:
	h8s2328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8s2329_device : public h8s2320_device {
public:
	h8s2329_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(H8S2320, h8s2320_device)
DECLARE_DEVICE_TYPE(H8S2321, h8s2321_device)
DECLARE_DEVICE_TYPE(H8S2322, h8s2322_device)
DECLARE_DEVICE_TYPE(H8S2323, h8s2323_device)
DECLARE_DEVICE_TYPE(H8S2324, h8s2324_device)
DECLARE_DEVICE_TYPE(H8S2326, h8s2326_device)
DECLARE_DEVICE_TYPE(H8S2327, h8s2327_device)
DECLARE_DEVICE_TYPE(H8S2328, h8s2328_device)
DECLARE_DEVICE_TYPE(H8S2329, h8s2329_device)

#endif // MAME_CPU_H8_H8S2329_H
