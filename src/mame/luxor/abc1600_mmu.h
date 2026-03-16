// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Memory Access Controller emulation

**********************************************************************/

#ifndef MAME_LUXOR_ABC1600_MMU_H
#define MAME_LUXOR_ABC1600_MMU_H

#pragma once

#include "cpu/m68000/m68008.h"
#include "machine/watchdog.h"

class abc1600_mmu_device : public device_t, public device_memory_interface, public device_state_interface
{
public:
    abc1600_mmu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_cpu(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	template <int N> auto in_tren_cb() { return m_read_tren[N].bind(); }
	template <int N> auto out_tren_cb() { return m_write_tren[N].bind(); }

    void partst_w(int state) { m_partst = state; }
	void rstbut_w(int state) { m_rstbut = state; }

    void dmamap_w(offs_t offset, uint8_t data);

    uint8_t dma0_mreq_r(offs_t offset) { return dma_mreq_r(0, DMAMAP_R0_LO, offset); }
	void dma0_mreq_w(offs_t offset, uint8_t data) { dma_mreq_w(0, DMAMAP_R0_LO, offset, data); }
	uint8_t dma0_iorq_r(offs_t offset) { return dma_iorq_r(DMAMAP_R0_LO, offset); }
	void dma0_iorq_w(offs_t offset, uint8_t data) { dma_iorq_w(DMAMAP_R0_LO, offset, data); }

	uint8_t dma1_mreq_r(offs_t offset) { return dma_mreq_r(1, DMAMAP_R1_LO, offset); }
	void dma1_mreq_w(offs_t offset, uint8_t data) { dma_mreq_w(1, DMAMAP_R1_LO, offset, data); }
	uint8_t dma1_iorq_r(offs_t offset) { return dma_iorq_r(DMAMAP_R1_LO, offset); }
	void dma1_iorq_w(offs_t offset, uint8_t data) { dma_iorq_w(DMAMAP_R1_LO, offset, data); }

	uint8_t dma2_mreq_r(offs_t offset) { return dma_mreq_r(2, DMAMAP_R2_LO, offset); }
	void dma2_mreq_w(offs_t offset, uint8_t data) { dma_mreq_w(2, DMAMAP_R2_LO, offset, data); }
	uint8_t dma2_iorq_r(offs_t offset) { return dma_iorq_r(DMAMAP_R2_LO, offset); }
	void dma2_iorq_w(offs_t offset, uint8_t data) { dma_iorq_w(DMAMAP_R2_LO, offset, data); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
    
    // device_memory_interface implementation
	space_config_vector memory_space_config() const override ATTR_COLD;

    // optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	static constexpr int DMAOK = 0x04;

	enum {
		AS_BOOT = 7,
        AS_MAC = 8
	};

    enum {
		DMAMAP_R2_LO = 0,
		DMAMAP_R2_HI,
		DMAMAP_R1_LO = 4,
		DMAMAP_R1_HI,
		DMAMAP_R0_LO,
		DMAMAP_R0_HI
	};

	class mmu : public m68008_device::mmu8
    {
        friend class abc1600_mmu_device;

    public:
		mmu(m68008_device *maincpu, address_space &boot, address_space &program, address_space &cpu_space, address_space &mac_space, u8 *segment_ram, u16 *page_ram);
		virtual ~mmu() = default;

        virtual u8 read_program(offs_t addr) override;
        virtual void write_program(offs_t addr, u8 data) override;
        virtual u8 read_data(offs_t addr) override;
        virtual void write_data(offs_t addr, u8 data) override;
        virtual u8 read_cpu(offs_t addr) override;
        virtual void set_super(bool super) override;
        virtual bool translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

    private:
		m68008_device *m_maincpu;
		u8 *m_segment_ram;
		u16 *m_page_ram;
        address_space &m_a_boot, &m_a_program, &m_a_cpu_space, &m_a_mac_space;
		memory_access<17, 0, 0, ENDIANNESS_BIG>::specific m_boot;
		memory_access<21, 0, 0, ENDIANNESS_BIG>::specific m_program;
		memory_access<20, 0, 0, ENDIANNESS_BIG>::specific m_cpu_space;
		memory_access<20, 0, 0, ENDIANNESS_BIG>::specific m_s_program;

        void reset();
        offs_t get_physical_address(offs_t logical, int task, bool &nonx, bool &wp);
        u8 mmu_read(offs_t logical, int intention);
        void mmu_write(offs_t logical, u8 data);

        bool m_super = false;
        bool m_boote = 0;
        bool m_magic = 0;
        int m_task = 0;
        u8 m_cause = 0;
    };

    devcb_read8::array<3> m_read_tren;
	devcb_write8::array<3> m_write_tren;

	required_device<m68008_device> m_maincpu;
	std::unique_ptr<mmu> m_mmu;
	address_space_config m_boot_config, m_program_config, m_cpu_space_config, m_s_program_config;
	required_device<watchdog_timer_device> m_watchdog;
	memory_share_creator<u8> m_segment_ram;
	memory_share_creator<u16> m_page_ram;

    void boot_map(address_map &map) ATTR_COLD;
	void mac_map(address_map &map) ATTR_COLD;

	uint8_t cause_r();
	void task_w(offs_t offset, uint8_t data);
	uint8_t segment_r(offs_t offset);
	void segment_w(offs_t offset, uint8_t data);
	uint8_t page_lo_r(offs_t offset);
	void page_lo_w(offs_t offset, uint8_t data);
	uint8_t page_hi_r(offs_t offset);
	void page_hi_w(offs_t offset, uint8_t data);

    offs_t get_dma_address(int index, offs_t logical, bool &rw);
	uint8_t dma_mreq_r(int index, int dmamap, offs_t offset);
	void dma_mreq_w(int index, int dmamap, offs_t offset, uint8_t data);
	uint8_t dma_iorq_r(int dmamap, offs_t offset);
	void dma_iorq_w(int dmamap, offs_t offset, uint8_t data);

    bool m_rstbut = 0;
    bool m_partst = 0;

    u8 m_dmamap[8];
};

DECLARE_DEVICE_TYPE(LUXOR_ABC1600_MMU, abc1600_mmu_device)

#endif // MAME_LUXOR_ABC1600_MMU_H
