// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#ifndef MAME_CPU_UPD777_UPD777_H
#define MAME_CPU_UPD777_UPD777_H

#pragma once

#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"


class upd777_cpu_device : public cpu_device
{
public:
	upd777_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	enum {
		UPD777_PC = 0,
		UPD777_A1,
		UPD777_A2,
		UPD777_A3,
		UPD777_A4,
		UPD777_SKIP,
		UPD777_H,
		UPD777_L,
		UPD777_ADDR_STACK0,
		UPD777_ADDR_STACK1,
		UPD777_ADDR_STACK2,
		UPD777_ADDR_STACK_POS,
	};

	u16* get_prgregion() { return &m_prgregion[0]; }
	u8* get_patregion() { return &m_patregion[0]; }

	auto in_cb() { return m_port_in.bind(); }

protected:
	upd777_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor data);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual space_config_vector memory_space_config() const override;

	bool get_vbl_state();
	bool get_hbl_4_state();


	void internal_map(address_map &map);
	void internal_data_map(address_map &map);

	required_shared_ptr<u8> m_datamem;

private:
	void increment_pc();
	u16 fetch();
	void do_op();


	u8 get_l() const;
	void set_l(int l);
	u8 get_h_shifted() const;
	u8 get_h() const;
	void set_h(int h);
	void set_a11(int a11);
	void set_new_pc(int newpc);

	void set_frs(u8 frs);
	void set_fls(u8 fls);
	void set_mode(u8 mode);

	u8 get_m_data();
	void set_m_data(u8 data);

	u8 get_a1() const;
	u8 get_a2() const;
	u8 get_a3() const;
	u8 get_a4() const;
	u8 get_a1_or_a2(int reg) const;

	void set_a1(u8 data);
	void set_a2(u8 data);
	void set_a3(u8 data);
	void set_a4(u8 data);
	void set_a1_or_a2(int reg, u8 value);

	void set_disp(u8 data);
	void set_gpe(u8 data);
	void set_kie(u8 data);
	void set_sme(u8 data);

	u8 get_kie() const;
	u8 get_sme() const;

	u8 read_data_mem(u8 addr);
	void write_data_mem(u8 addr, u8 data);
	void push_to_stack(u16 addr);
	u16 pull_from_stack();

	address_space_config m_space_config;
	address_space_config m_data_config;

	memory_access<11, 1, -1, ENDIANNESS_BIG>::specific m_space;
	memory_access<8, 0, 0, ENDIANNESS_BIG>::specific m_data;

	u32 m_ppc;
	u32 m_pc;
	s32 m_icount;
	u8 m_skip;
	u8 m_a[4]; // A1 to A4
	u8 m_h;
	u8 m_l;
	u16 m_stack[3]; // 3 11-bit registers
	u8 m_stackpos;

	u8 m_frs;
	u8 m_fls;

	u8 m_mode;

	u8 m_stb;

	// what are these, used by H<->X opcode
	u8 m_ldash;
	u8 m_x4;

	// single bit enable registers?
	u8 m_disp;
	u8 m_gpe;
	u8 m_kie;
	u8 m_sme;

	///////////////////////////////////// VIDEO RELATED

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void push_to_line_buffer(u8 h, u8 m1, u8 m2, u8 m3, u8 m4);

	void palette_init(palette_device &palette) const;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_region_ptr<uint16_t> m_prgregion;
	required_region_ptr<uint8_t> m_patregion;

	devcb_read8 m_port_in;
};

DECLARE_DEVICE_TYPE(UPD777, upd777_cpu_device)

#endif // MAME_CPU_UPD777_UPD777_H
