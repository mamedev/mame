// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood

#ifndef MAME_JALECO_MS1_GATEARRAY_H
#define MAME_JALECO_MS1_GATEARRAY_H

#pragma once

class megasys1_gatearray_device : public device_t
{
public:
	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	void set_rom(u8* rom, u32 size) { m_rom = rom; m_romsize = size; }

protected:
	megasys1_gatearray_device(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	void install_gatearray_overlay();
	u16 gatearray_r(offs_t offset, u16 mem_mask = ~0);
	void gatearray_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	// System A only
	int m_mcu_hs = 0;
	u16 m_mcu_hs_ram[0x8]{};

	const u16* m_gatearray_seq = nullptr;

	required_device<cpu_device> m_cpu;
	u8* m_rom;
	u32 m_romsize;
	bool m_has_decoded;

	virtual void rom_decode() = 0;
};


class megasys1_gatearray_d65006_device : public megasys1_gatearray_device
{
public:
	megasys1_gatearray_d65006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	megasys1_gatearray_d65006_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void rom_decode() override;

private:
	//                                     write sequence                return value
	static constexpr u16 jaleco_d65006_unlock_sequence[5]  = { 0x0000,0x0055,0x00aa,0x00ff,  0x835d };

};

class megasys1_gatearray_gs88000_device : public megasys1_gatearray_device
{
public:
	megasys1_gatearray_gs88000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	megasys1_gatearray_gs88000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void rom_decode() override;

	//                                     write sequence                return value
	static constexpr u16 jaleco_gs88000_unlock_sequence[5] = { 0x00ff,0x0055,0x00aa,0x0000,  0x889e };

private:

};

class megasys1_gatearray_unkarray_device : public megasys1_gatearray_device
{
public:
	megasys1_gatearray_unkarray_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	megasys1_gatearray_unkarray_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void rom_decode() override;

private:

};

// device type definition
//DECLARE_DEVICE_TYPE(MEGASYS1_GATEARRAY, megasys1_gatearray_device)
DECLARE_DEVICE_TYPE(MEGASYS1_GATEARRAY_D65006, megasys1_gatearray_d65006_device)
DECLARE_DEVICE_TYPE(MEGASYS1_GATEARRAY_GS88000, megasys1_gatearray_gs88000_device)
DECLARE_DEVICE_TYPE(MEGASYS1_GATEARRAY_UNKARRAY, megasys1_gatearray_unkarray_device)

#endif  // MAME_JALECO_MS1_GATEARRAY_H
