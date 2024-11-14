// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood

#ifndef MAME_JALECO_MS1_GATEARRAY_H
#define MAME_JALECO_MS1_GATEARRAY_H

#pragma once

class megasys1_gatearray_device : public device_t
{
public:
	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_cpuregion_tag(T &&tag) { m_cpuregion.set_tag(std::forward<T>(tag)); }

protected:
	megasys1_gatearray_device(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void install_overlay();
	u16 gatearray_r(offs_t offset, u16 mem_mask = ~0);
	void gatearray_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	bool hs_seq() const;

	virtual void rom_decode() = 0;

	int m_gatearray_hs = 0;
	u16 m_gatearray_hs_ram[0x8]{};

	const u16 *m_gatearray_seq = nullptr;

	required_device<cpu_device> m_cpu;
	required_memory_region m_cpuregion;
};


class megasys1_gatearray_d65006_device : public megasys1_gatearray_device
{
public:
	megasys1_gatearray_d65006_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	megasys1_gatearray_d65006_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void rom_decode() override;

private:
	//                                                         write sequence                return value
	static constexpr u16 jaleco_d65006_unlock_sequence[5]  = { 0x0000,0x0055,0x00aa,0x00ff,  0x835d };
};

class megasys1_gatearray_gs88000_device : public megasys1_gatearray_device
{
public:
	megasys1_gatearray_gs88000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	megasys1_gatearray_gs88000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void rom_decode() override;

	//                                                         write sequence                return value
	static constexpr u16 jaleco_gs88000_unlock_sequence[5] = { 0x00ff,0x0055,0x00aa,0x0000,  0x889e };
};

class megasys1_gatearray_unkarray_device : public megasys1_gatearray_device
{
public:
	megasys1_gatearray_unkarray_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	megasys1_gatearray_unkarray_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void rom_decode() override;
};

// device type definition
//DECLARE_DEVICE_TYPE(MEGASYS1_GATEARRAY, megasys1_gatearray_device)
DECLARE_DEVICE_TYPE(MEGASYS1_GATEARRAY_D65006, megasys1_gatearray_d65006_device)
DECLARE_DEVICE_TYPE(MEGASYS1_GATEARRAY_GS88000, megasys1_gatearray_gs88000_device)
DECLARE_DEVICE_TYPE(MEGASYS1_GATEARRAY_UNKARRAY, megasys1_gatearray_unkarray_device)

#endif  // MAME_JALECO_MS1_GATEARRAY_H
