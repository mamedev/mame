// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_NEC_PC88VA_SGP_H
#define MAME_NEC_PC88VA_SGP_H

#pragma once

class pc88va_sgp_device :
	public device_t,
	public device_memory_interface
{
public:
	pc88va_sgp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename... T> pc88va_sgp_device& set_map(T &&... args) { set_addrmap(AS_DATA, std::forward<T>(args)...); return *this; }

	void sgp_io(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_config_complete() override;

	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_data_config;
	address_space *m_data;

	u16 m_vdp_address[2]{};
	u32 m_work_address = 0;
	u16 m_color_code;

	struct BufferArea {
		u8 start_dot = 0;
		u8 pixel_mode = 0;
		u16 hsize = 0;
		u16 vsize = 0;
		u16 fb_pitch = 0;
		u32 address = 0;
	};

	BufferArea m_src, m_dst;

	void vdp_address_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 status_r();
	void control_w(u8 data);
	void trigger_w(u8 data);

	void start_exec();

	void execute_blit(u16 draw_mode, bool is_patblt);

	typedef u16 (pc88va_sgp_device::*rop_func)(u16 src, u16 dst);

	static const rop_func rop_table[16];
	u16 rop_0_fill_0(u16 src, u16 dst);
	u16 rop_1_s_and_d(u16 src, u16 dst);
	u16 rop_2_ns_and_d(u16 src, u16 dst);
	u16 rop_3_d(u16 src, u16 dst);
	u16 rop_4_s_and_nd(u16 src, u16 dst);
	u16 rop_5_s(u16 src, u16 dst);
	u16 rop_6_s_xor_d(u16 src, u16 dst);
	u16 rop_7_s_or_d(u16 src, u16 dst);
	u16 rop_8_ns_or_d(u16 src, u16 dst);
	u16 rop_9_n_s_xor_d(u16 src, u16 dst);
	u16 rop_A_n_s(u16 src, u16 dst);
	u16 rop_B_n_s_or_d(u16 src, u16 dst);
	u16 rop_C_nd(u16 src, u16 dst);
	u16 rop_D_s_or_nd(u16 src, u16 dst);
	u16 rop_E_n_s_and_d(u16 src, u16 dst);
	u16 rop_F_fill_1(u16 src, u16 dst);

	typedef bool (pc88va_sgp_device::*tpmod_func)(u16 src, u16 dst);
	static const tpmod_func tpmod_table[4];
	bool tpmod_0_always(u16 src, u16 dst);
	bool tpmod_1_src(u16 src, u16 dst);
	bool tpmod_2_dst(u16 src, u16 dst);
	bool tpmod_3_never(u16 src, u16 dst);
};


DECLARE_DEVICE_TYPE(PC88VA_SGP, pc88va_sgp_device)

#endif  // MAME_NEC_PC88VA_SGP_H
