// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_ROMP_RSC_H
#define MAME_CPU_ROMP_RSC_H

#pragma once

/*
 * ROMP Storage Channel (the 32-bit, packet-switched, synchronous, pipelined
 * interface between ROMP CPU, MMU and I/O Channel Converter/Controller).
 *
 * TODO:
 *   - improve registration/decoding
 *   - advanced/enhanced variations
 */

class rsc_bus_interface
	: public device_interface
{
public:
	rsc_bus_interface(machine_config const &mconfig, device_t &device)
		: rsc_bus_interface(mconfig, device, "rsc_bus_interface")
	{
	}
	virtual ~rsc_bus_interface() = default;

	enum rsc_mode : unsigned
	{
		RSC_N   = 0,
		RSC_T   = 1, // translate mode
		RSC_U   = 2, // unprivileged state
		RSC_UT  = 3,
		RSC_P   = 4, // memory protect
		RSC_PT  = 5,
		RSC_PU  = 6,
		RSC_PUT = 7,
	};

	virtual bool mem_load(u32 address, u8 &data, rsc_mode const mode = RSC_N, bool sp = false) = 0;
	virtual bool mem_load(u32 address, u16 &data, rsc_mode const mode = RSC_N, bool sp = false) = 0;
	virtual bool mem_load(u32 address, u32 &data, rsc_mode const mode = RSC_N, bool sp = false) = 0;

	virtual bool mem_store(u32 address, u8 data, rsc_mode const mode = RSC_N, bool sp = false) = 0;
	virtual bool mem_store(u32 address, u16 data, rsc_mode const mode = RSC_N, bool sp = false) = 0;
	virtual bool mem_store(u32 address, u32 data, rsc_mode const mode = RSC_N, bool sp = false) = 0;

	virtual bool mem_modify(u32 address, std::function<u8(u8)> f, rsc_mode const mode = RSC_N) = 0;
	virtual bool mem_modify(u32 address, std::function<u16(u16)> f, rsc_mode const mode = RSC_N) = 0;
	virtual bool mem_modify(u32 address, std::function<u32(u32)> f, rsc_mode const mode = RSC_N) = 0;

	virtual bool pio_load(u32 address, u8 &data, rsc_mode const mode = RSC_N) = 0;
	virtual bool pio_load(u32 address, u16 &data, rsc_mode const mode = RSC_N) = 0;
	virtual bool pio_load(u32 address, u32 &data, rsc_mode const mode = RSC_N) = 0;

	virtual bool pio_store(u32 address, u8 data, rsc_mode const mode = RSC_N) = 0;
	virtual bool pio_store(u32 address, u16 data, rsc_mode const mode = RSC_N) = 0;
	virtual bool pio_store(u32 address, u32 data, rsc_mode const mode = RSC_N) = 0;

	virtual bool pio_modify(u32 address, std::function<u8(u8)> f, rsc_mode const mode = RSC_N) = 0;
	virtual bool pio_modify(u32 address, std::function<u16(u16)> f, rsc_mode const mode = RSC_N) = 0;
	virtual bool pio_modify(u32 address, std::function<u32(u32)> f, rsc_mode const mode = RSC_N) = 0;

protected:
	rsc_bus_interface(machine_config const &mconfig, device_t &device, char const *type)
		: device_interface(device, type)
	{
	}
};

class rsc_cpu_interface
	: public rsc_bus_interface
{
public:
	rsc_cpu_interface(machine_config const &mconfig, device_t &device)
		: rsc_bus_interface(mconfig, device, "rsc_cpu_interface")
	{
	}
	virtual ~rsc_cpu_interface() = default;

	// cpu interface
	virtual bool fetch(u32 address, u16 &data, rsc_mode const mode = RSC_N) = 0;
	virtual bool translate(u32 &address) const = 0;

	// rsc_bus_interface overrides
	virtual bool mem_load(u32 address, u8 &data, rsc_mode const mode = RSC_N, bool sp = true) override = 0;
	virtual bool mem_load(u32 address, u16 &data, rsc_mode const mode = RSC_N, bool sp = true) override = 0;
	virtual bool mem_load(u32 address, u32 &data, rsc_mode const mode = RSC_N, bool sp = true) override = 0;

	virtual bool mem_store(u32 address, u8 data, rsc_mode const mode = RSC_N, bool sp = true) override = 0;
	virtual bool mem_store(u32 address, u16 data, rsc_mode const mode = RSC_N, bool sp = true) override = 0;
	virtual bool mem_store(u32 address, u32 data, rsc_mode const mode = RSC_N, bool sp = true) override = 0;
};

#endif // MAME_CPU_ROMP_RSC_H
