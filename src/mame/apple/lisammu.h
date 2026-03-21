// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// The LISA MMU
#ifndef MAME_APPLE_LISAMMU_H
#define MAME_APPLE_LISAMMU_H

#pragma once

#include "lisavideo.h"

#include "cpu/m68000/m68000.h"


class lisa_mmu_device : public device_t, public device_memory_interface {
public:
	enum {
		AS_RAM = AS_PROGRAM,
		AS_SPECIAL_IO = AS_DATA
	};

	lisa_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_cpu(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_video(T &&tag) { m_video.set_tag(std::forward<T>(tag)); }

	auto write_parity_err() { return m_parity_err_cb.bind(); }

	u16 sor_r(offs_t adr);
	void sor_w(offs_t adr, u16 data, u16 mem_mask);
	u16 slr_r(offs_t adr);
	void slr_w(offs_t adr, u16 data, u16 mem_mask);
	u16 status_r();

	u16 parity_error_address_r();

	void setup_1();
	void setup_0();
	void seg1_1();
	void seg1_0();
	void seg2_1();
	void seg2_0();
	void diag1_0();
	void diag1_1();
	void diag2_0();
	void diag2_1();
	void serr_0();
	void serr_1();
	void herr_0();
	void herr_1();

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	space_config_vector memory_space_config() const override ATTR_COLD;

private:
	enum : u8 {
		INVALID,
		MAIN_RO_STACK,
		MAIN_RO,
		MAIN_RW_STACK,
		MAIN_RW,
		IO,
		SPECIAL
	};

	static const u8 mode_table[16];

	class mmu : public m68000_device::mmu {
	public:
		mmu(m68000_device *maincpu, address_space &ram, address_space &io, address_space &special_io, address_space &cpu_space);
		virtual ~mmu() = default;

		u16 read_program(offs_t addr, u16 mem_mask) override;
		void write_program(offs_t addr, u16 data, u16 mem_mask) override;
		u16 read_data(offs_t addr, u16 mem_mask) override;
		void write_data(offs_t addr, u16 data, u16 mem_mask) override;
		u16 read_cpu(offs_t addr, u16 mem_mask) override;
		void set_super(bool super) override;
		bool translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

		void set_seg(u8 seg);
		void set_setup(bool setup);

		void setup_entry(int entry, u16 sor, u16 slr);

	private:
		m68000_device *m_maincpu;
		address_space &m_a_ram, &m_a_io, &m_a_special_io, &m_a_cpu_space;
		memory_access<24, 1, 0, ENDIANNESS_BIG>::specific m_ram, m_io, m_special_io, m_cpu_space;

		std::array<u32, 128*(4+1)> m_base_address;
		std::array<u32, 128*(4+1)> m_limit_address;
		std::array<u8, 128*(4+1)> m_mode;

		u32 m_current_base;
		u8 m_seg;
		bool m_setup, m_super;

		inline u16 read(offs_t addr, u16 mem_mask);
		inline void write(offs_t addr, u16 data, u16 mem_mask);
		void recompute_slot();
	};

	devcb_write_line m_parity_err_cb;

	required_device<m68000_device> m_maincpu;
	required_device<lisa_video_device> m_video;
	std::unique_ptr<mmu> m_mmu;
	memory_passthrough_handler m_mph_hard_error_write, m_mph_hard_error_read;

	address_space_config m_ram_config, m_io_config, m_special_io_config, m_cpu_space_config;

	std::array<u16, 128*4> m_sor, m_slr;

	u32 m_hard_error_address;
	u8 m_seg;
	bool m_hard_error, m_hard_error_mask, m_hard_error_force;

	void default_autovectors_map(address_map &map);
	void setup_or_clear_hard_error(offs_t address);
	void test_hard_error(offs_t address);
};

DECLARE_DEVICE_TYPE(LISAMMU, lisa_mmu_device)

#endif
