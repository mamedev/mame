// license:BSD-3-Clause
// copyright-holders:Curt Coder, Olivier Galibert

#ifndef MAME_ATARI_STMMU_H
#define MAME_ATARI_STMMU_H

#include <machine/wd_fdc.h>
#include <cpu/m68000/m68000.h>

// Atari ST family MMU implementation.  Handles things from the GLUE
// too, because the concerns are difficult to separate

// Manages dma and communications with fdc and hd in general.

class st_mmu_device : public device_t {
public:
	st_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_ram(T &&tag) { m_ram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_fdc(T &&tag) { m_fdc.set_tag(std::forward<T>(tag)); }

	void map(address_map &map) ATTR_COLD;
	void set_ram_size(u32 size);

	void fdc_drq_w(int state);
	void hdc_drq_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum {
		STATUS_DRQ          = 0x04,
		STATUS_SECTOR_COUNT = 0x02,
		STATUS_ERROR        = 0x01,
	};

	enum {
		MODE_READ_WRITE   = 0x100,
		MODE_FDC_HDC_ACK  = 0x080,
		MODE_ENABLED      = 0x040,
		MODE_SECTOR_COUNT = 0x010,
		MODE_FDC_HDC_CS   = 0x008,
		MODE_A1           = 0x004,
		MODE_A0           = 0x002,
		MODE_ADDRESS_MASK = 0x006,
	};

	enum {
		SECTOR_SIZE = 512
	};

	required_shared_ptr<u16> m_ram;
	required_device<m68000_device> m_cpu;
	required_device<wd1772_device> m_fdc;

	uint32_t m_ram_size;

	uint32_t m_dma_address;
	uint16_t m_dma_mode;
	uint16_t m_fifo[16];
	uint8_t m_sector_count, m_block_count;
	uint8_t m_fifo_index;
	uint8_t m_memcfg;
	bool m_fdc_drq, m_hdc_drq;
	bool m_dma_no_error;

	uint16_t data_r();
	void data_w(uint16_t data);
	uint16_t dma_status_r();
	void dma_mode_w(uint16_t data);
	uint8_t dma_address_h_r();
	uint8_t dma_address_m_r();
	uint8_t dma_address_l_r();
	void dma_address_h_w(uint8_t data);
	void dma_address_m_w(uint8_t data);
	void dma_address_l_w(uint8_t data);

	uint8_t memcfg_r();
	void memcfg_w(uint8_t data);
	void configure_ram();
	static offs_t remap_128_512(offs_t offset);
	static offs_t remap_128_2048(offs_t offset);
	static offs_t remap_512_128(offs_t offset);
	static offs_t remap_512_2048(offs_t offset);
	static offs_t remap_2048_128(offs_t offset);
	static offs_t remap_2048_512(offs_t offset);
	void ram_mapping(u32 actual, u32 config, u32 adr, u32 radr, u32 off_s, u32 off_u);

	void fdc_transfer();
	void hdc_transfer();

	void fifo_flush();
	u8 fifo_pop();
	void fifo_push(u8 data);

	void fifo_schedule_block_transfer_to_ram();
	void fifo_schedule_block_transfer_from_ram();

	u64 bus_contention(offs_t address, u64 current_time) const;
};

DECLARE_DEVICE_TYPE(ST_MMU, st_mmu_device)

#endif
