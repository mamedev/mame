// license:BSD-3-Clause
// copyright-holders:Curt Coder, Olivier Galibert

#include "emu.h"
#include "stmmu.h"

#define LOG_MODE   (1U << 1) // Shows mode
#define LOG_PORT   (1U << 2) // Shows direct data read/write
#define LOG_DMA    (1U << 3) // Shows dma address and sector count setup
#define LOG_DATA   (1U << 4) // Shows dma-ed data
#define LOG_RAM    (1U << 5) // Shows ram configuration

#define VERBOSE (LOG_RAM)

//#define VERBOSE (LOG_DESC | LOG_COMMAND | LOG_MATCH | LOG_WRITE | LOG_STATE | LOG_LINES | LOG_COMP | LOG_CRC )
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGMODE(...)   LOGMASKED(LOG_MODE,  __VA_ARGS__)
#define LOGPORT(...)   LOGMASKED(LOG_PORT,  __VA_ARGS__)
#define LOGDMA(...)    LOGMASKED(LOG_DMA,   __VA_ARGS__)
#define LOGDATA(...)   LOGMASKED(LOG_DATA,  __VA_ARGS__)
#define LOGRAM(...)    LOGMASKED(LOG_RAM,   __VA_ARGS__)


DEFINE_DEVICE_TYPE(ST_MMU, st_mmu_device, "st_mmu", "Atari ST MMU")

void st_mmu_device::map(address_map &map)
{
	map(0x001, 0x001).rw(FUNC(st_mmu_device::memcfg_r), FUNC(st_mmu_device::memcfg_w));
	map(0x604, 0x605).rw(FUNC(st_mmu_device::data_r), FUNC(st_mmu_device::data_w));
	map(0x606, 0x607).rw(FUNC(st_mmu_device::dma_status_r), FUNC(st_mmu_device::dma_mode_w));
	map(0x609, 0x609).rw(FUNC(st_mmu_device::dma_address_h_r), FUNC(st_mmu_device::dma_address_h_w));
	map(0x60b, 0x60b).rw(FUNC(st_mmu_device::dma_address_m_r), FUNC(st_mmu_device::dma_address_m_w));
	map(0x60d, 0x60d).rw(FUNC(st_mmu_device::dma_address_l_r), FUNC(st_mmu_device::dma_address_l_w));
}

st_mmu_device::st_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ST_MMU, tag, owner, clock),
	m_ram(*this, finder_base::DUMMY_TAG),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_fdc(*this, finder_base::DUMMY_TAG)
{
	m_ram_size = 0;
}

void st_mmu_device::device_start()
{
	save_item(NAME(m_ram_size));
	save_item(NAME(m_dma_address));
	save_item(NAME(m_dma_mode));
	save_item(NAME(m_fifo));
	save_item(NAME(m_block_count));
	save_item(NAME(m_sector_count));
	save_item(NAME(m_fifo_index));
	save_item(NAME(m_memcfg));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_hdc_drq));
	save_item(NAME(m_dma_no_error));

	// Electra demo shows reset does not reconfigure ram
	m_memcfg = 0;
}

void st_mmu_device::device_reset()
{
	m_dma_address = 0;
	m_dma_mode = 0;
	memset(m_fifo, 0, sizeof(m_fifo));
	m_sector_count = 0;
	m_block_count = 0;
	m_fifo_index = 0;
	m_fdc_drq = false;
	m_hdc_drq = false;
	m_dma_no_error = true;
	configure_ram();
}

void st_mmu_device::fdc_transfer()
{
	if(m_dma_mode & MODE_READ_WRITE)
		m_fdc->data_w(fifo_pop());
	else
		fifo_push(m_fdc->data_r());
}

void st_mmu_device::hdc_transfer()
{
	logerror("hdc transfer unimplemented\n");
}

void st_mmu_device::fdc_drq_w(int state)
{
	if(state == m_fdc_drq)
		return;
	m_fdc_drq = state;
	while(m_fdc_drq && (m_dma_mode & MODE_FDC_HDC_ACK))
		fdc_transfer();
}

void st_mmu_device::hdc_drq_w(int state)
{
	if(state == m_hdc_drq)
		return;
	m_hdc_drq = state;
	while(m_hdc_drq && !(m_dma_mode & MODE_FDC_HDC_ACK))
		hdc_transfer();
}

uint16_t st_mmu_device::data_r()
{
	if(m_dma_mode & MODE_SECTOR_COUNT)
		// Sector count is not readable
		return 0x55;

	else if(m_dma_mode & MODE_FDC_HDC_CS) {
		u8 r = 0x00;
		LOGPORT("hdc_r %02x\n", r);
		return 0;

	} else {
		u8 r = m_fdc->read((m_dma_mode >> 1) & 3);
		LOGPORT("fdc_r %02x\n", r);
		return r;
	}
}

void st_mmu_device::data_w(uint16_t data)
{
	data &= 0xff;
	if(m_dma_mode & MODE_SECTOR_COUNT) {
		LOGDMA("sector_count_w %02x\n", data);
		m_sector_count = data;
		if(data == 0)
			m_dma_no_error = false;
		else if(m_dma_mode & MODE_READ_WRITE)
			fifo_schedule_block_transfer_from_ram();

	} else if(m_dma_mode & MODE_FDC_HDC_CS) {
		LOGPORT("hdc_w %02x\n", data);

	} else {
		LOGPORT("fdc_w %d, %02x\n", (m_dma_mode >> 1) & 3, data);
		m_fdc->write((m_dma_mode >> 1) & 3, data);
	}
}

uint16_t st_mmu_device::dma_status_r()
{
	return (m_dma_no_error ? 1 : 0) | (m_sector_count ? 2 : 0) | (m_fdc_drq ? 4 : 0);
}

void st_mmu_device::dma_mode_w(uint16_t data)
{
	if((m_dma_mode ^ data) & MODE_READ_WRITE)
		fifo_flush();

	m_dma_mode = data;
	LOGMODE("mode %04x %c%c%c%c%c%d\n", m_dma_mode,
			m_dma_mode & MODE_READ_WRITE ? 'w' : 'r',
			m_dma_mode & MODE_FDC_HDC_ACK ? 'f' : 'h',
			m_dma_mode & MODE_ENABLED ? '+' : '-',
			m_dma_mode & MODE_SECTOR_COUNT ? 'C' : '.',
			m_dma_mode & MODE_FDC_HDC_CS ? 'h' : 'f',
			(m_dma_mode >> 1) & 3);

	while(m_fdc_drq && (m_dma_mode & MODE_FDC_HDC_ACK))
		fdc_transfer();
	while(m_hdc_drq && !(m_dma_mode & MODE_FDC_HDC_ACK))
		hdc_transfer();
}

uint8_t st_mmu_device::dma_address_h_r()
{
	return m_dma_address >> 16;
}

uint8_t st_mmu_device::dma_address_m_r()
{
	return m_dma_address >> 8;
}

uint8_t st_mmu_device::dma_address_l_r()
{
	return m_dma_address;
}

void st_mmu_device::dma_address_h_w(uint8_t data)
{
	m_dma_address = (m_dma_address & 0x00ffff) | (data << 16);
	LOGDMA("dma address %06x\n", m_dma_address);
}

void st_mmu_device::dma_address_m_w(uint8_t data)
{
	m_dma_address = (m_dma_address & 0xff00ff) | (data << 8);
}

void st_mmu_device::dma_address_l_w(uint8_t data)
{
	m_dma_address = (m_dma_address & 0xffff00) | data;
}


void st_mmu_device::fifo_flush()
{
	m_fifo_index = 0;
	m_sector_count = 0;
	m_block_count = 0;
	m_dma_no_error = true;
}

void st_mmu_device::fifo_push(u8 data)
{
	if(m_fifo_index == 32 || !m_sector_count) {
		m_dma_no_error = false;
		return;
	}

	int idx = m_fifo_index >> 1;
	if(m_fifo_index & 1)
		m_fifo[idx] = data | (m_fifo[idx] & 0xff00);
	else
		m_fifo[idx] = (data << 8) | (m_fifo[idx] & 0x00ff);
	m_fifo_index ++;
	if(m_fifo_index == 16)
		fifo_schedule_block_transfer_to_ram();
}

u8 st_mmu_device::fifo_pop()
{
	if(m_fifo_index == 32) {
		m_dma_no_error = false;
		return 0x00;
	}

	int idx = m_fifo_index >> 1;
	u8 r;
	if(m_fifo_index & 1)
		r = m_fifo[idx];
	else
		r = m_fifo[idx] >> 8;
	m_fifo_index ++;
	if(m_fifo_index == 16)
		fifo_schedule_block_transfer_from_ram();
	return r;
}

void st_mmu_device::fifo_schedule_block_transfer_to_ram()
{
	if(VERBOSE & LOG_DATA) {
		std::string l = util::string_format("r %06x =", m_dma_address);
		for(int i = 0; i != 8; i++)
			l += util::string_format(" %04x", m_fifo[i]);
		logerror("%s\n", l);
	}

	// Synchronous for now
	for(int i=0; i != 8; i++) {
		m_ram[(m_dma_address & 0x3ffffe) >> 1] = m_fifo[i];
		m_dma_address += 2;
	}
	memcpy(m_fifo, m_fifo + 8, 8*2);
	m_fifo_index -= 16;
	m_block_count ++;
	if(m_block_count == 32) {
		m_block_count = 0;
		m_sector_count --;
	}
}


void st_mmu_device::fifo_schedule_block_transfer_from_ram()
{
	if(!m_sector_count)
		return;

	// Synchronous for now
	if(m_fifo_index == 0) {
		int limit = m_sector_count > 1 || m_block_count != 31 ? 16 : 8;
		for(int i=0; i != limit; i++) {
			m_fifo[i] = m_ram[(m_dma_address & 0x3ffffe) >> 1];
			m_dma_address += 2;
		}
		m_block_count += limit == 16 ? 2 : 1;
		if(VERBOSE & LOG_DATA) {
			std::string l = util::string_format("w %06x =", m_dma_address-2*limit);
			for(int i = 0; i != 8; i++)
				l += util::string_format(" %04x", m_fifo[i]);
			logerror("%s\n", l);
			if(limit == 16) {
				std::string l = util::string_format("w %06x =", m_dma_address-16);
				for(int i = 0; i != 8; i++)
					l += util::string_format(" %04x", m_fifo[i+8]);
				logerror("%s\n", l);
			}
		}
	} else {
		memcpy(m_fifo, m_fifo + 8, 8*2);
		for(int i=0; i != 8; i++) {
			m_fifo[8 + i] = m_ram[(m_dma_address & 0x3ffffe) >> 1];
			m_dma_address += 2;
		}
		m_block_count ++;
		m_fifo_index -= 16;
		if(VERBOSE & LOG_DATA) {
			std::string l = util::string_format("w %06x =", m_dma_address-16);
			for(int i = 0; i != 8; i++)
				l += util::string_format(" %04x", m_fifo[i+8]);
			logerror("%s\n", l);
		}
	}

	if(m_block_count == 32) {
		m_block_count = 0;
		m_sector_count --;
	}
}


uint8_t st_mmu_device::memcfg_r()
{
	return m_memcfg;
}

void st_mmu_device::memcfg_w(uint8_t data)
{
	m_memcfg = data;
	configure_ram();
}

void st_mmu_device::set_ram_size(u32 size)
{
	m_ram_size = size;
}

// Install a bank of real size (in K) actual and configured size
// config, in memory map at address adr, in memory block at offset
// radr, skipping off_s bytes at the start in supervisor mode and
// off_u in user mode.

// In the ST (non-e), the address is split into a column and row of 7
// to 9 bits (128K to 2M).  Smaller chips just do not have the top
// column and row bits.  The address is split in the middle, with
// column in the high bits and row in the low.  As a result a mismatch
// between configuration and actual chip size requires breaking up and
// rebuilding the address.

// In the STE it's the same but the column and row bits are
// interleaved, ensuring a linear behaviour at all times.

offs_t st_mmu_device::remap_128_512(offs_t offset)
{
	return (offset & 0xff) | ((offset & 0xff00) << 1);
}

offs_t st_mmu_device::remap_128_2048(offs_t offset)
{
	return (offset & 0xff) | ((offset & 0xff00) << 2);
}

offs_t st_mmu_device::remap_512_128(offs_t offset)
{
	return (offset & 0xff) | ((offset & 0x1fe00) >> 1);
}

offs_t st_mmu_device::remap_512_2048(offs_t offset)
{
	return (offset & 0x1ff) | ((offset & 0x3fe00) << 1);
}

offs_t st_mmu_device::remap_2048_128(offs_t offset)
{
	return (offset & 0xff) | ((offset & 0x1fc00) >> 2);
}

offs_t st_mmu_device::remap_2048_512(offs_t offset)
{
	return (offset & 0x1ff) | ((offset & 0x7fc00) >> 1);
}

void st_mmu_device::ram_mapping(u32 actual, u32 config, u32 adr, u32 radr, u32 off_s, u32 off_u)
{
	auto &ss = m_cpu->space(AS_PROGRAM);
	auto &su = m_cpu->space(m68000_device::AS_USER_PROGRAM);
	switch(config) {
	case  128:
		switch(actual) {
		case    0: {
			ss.unmap_readwrite(adr + off_s, adr + 0x1ffff);
			su.unmap_readwrite(adr + off_u, adr + 0x1ffff);
			break;
		}
		case  128: {
			ss.install_ram(adr + off_s, adr + 0x1ffff, m_ram + radr/2 + off_s/2);
			su.install_ram(adr + off_u, adr + 0x1ffff, m_ram + radr/2 + off_u/2);
			break;
		}
		case  512: {
			ss.install_read_handler (adr + off_s, adr + 0x1ffff, read16sm_delegate(*this, [this, off_s, radr](offs_t offset) { return m_ram[remap_128_512(offset + off_s/2) + radr/2]; }, "128-512"));
			ss.install_write_handler(adr + off_s, adr + 0x1ffff, write16s_delegate(*this, [this, off_s, radr](offs_t offset, offs_t data, offs_t mem_mask) { COMBINE_DATA(&m_ram[remap_128_512(offset + off_s/2) + radr/2]); }, "128-512"));
			su.install_read_handler (adr + off_u, adr + 0x1ffff, read16sm_delegate(*this, [this, off_u, radr](offs_t offset) { return m_ram[remap_128_512(offset + off_u/2) + radr/2]; }, "128-512"));
			su.install_write_handler(adr + off_u, adr + 0x1ffff, write16s_delegate(*this, [this, off_u, radr](offs_t offset, offs_t data, offs_t mem_mask) { COMBINE_DATA(&m_ram[remap_128_512(offset + off_u/2) + radr/2]); }, "128-512"));
			break;
		}
		case 2048: {
			ss.install_read_handler (adr + off_s, adr + 0x1ffff, read16sm_delegate(*this, [this, off_s, radr](offs_t offset) { return m_ram[remap_128_2048(offset + off_s/2) + radr/2]; }, "128-2048"));
			ss.install_write_handler(adr + off_s, adr + 0x1ffff, write16s_delegate(*this, [this, off_s, radr](offs_t offset, offs_t data, offs_t mem_mask) { COMBINE_DATA(&m_ram[remap_512_128(offset + off_s/2) + radr/2]); }, "128-2048"));
			su.install_read_handler (adr + off_u, adr + 0x1ffff, read16sm_delegate(*this, [this, off_u, radr](offs_t offset) { return m_ram[remap_128_2048(offset + off_u/2) + radr/2]; }, "128-2048"));
			su.install_write_handler(adr + off_u, adr + 0x1ffff, write16s_delegate(*this, [this, off_u, radr](offs_t offset, offs_t data, offs_t mem_mask) { COMBINE_DATA(&m_ram[remap_512_128(offset + off_u/2) + radr/2]); }, "128-2048"));
			break;
		}
		}
		break;
	case  512:
		switch(actual) {
		case    0: {
			ss.unmap_readwrite(adr + off_s, adr + 0x7ffff);
			su.unmap_readwrite(adr + off_u, adr + 0x7ffff);
			break;
		}
		case  128: {
			ss.install_read_handler (adr + off_s, adr + 0x7ffff, read16sm_delegate(*this, [this, off_s, radr](offs_t offset) { return m_ram[remap_512_128(offset + off_s/2) + radr/2]; }, "512-128"));
			ss.install_write_handler(adr + off_s, adr + 0x7ffff, write16s_delegate(*this, [this, off_s, radr](offs_t offset, offs_t data, offs_t mem_mask) { COMBINE_DATA(&m_ram[remap_512_128(offset + off_s/2) + radr/2]); }, "512-128"));
			su.install_read_handler (adr + off_u, adr + 0x7ffff, read16sm_delegate(*this, [this, off_u, radr](offs_t offset) { return m_ram[remap_512_128(offset + off_u/2) + radr/2]; }, "512-128"));
			su.install_write_handler(adr + off_u, adr + 0x7ffff, write16s_delegate(*this, [this, off_u, radr](offs_t offset, offs_t data, offs_t mem_mask) { COMBINE_DATA(&m_ram[remap_512_128(offset + off_u/2) + radr/2]); }, "512-128"));
			break;
		}
		case  512: {
			ss.install_ram(adr + off_s, adr + 0x7ffff, m_ram + radr/2 + off_s/2);
			su.install_ram(adr + off_u, adr + 0x7ffff, m_ram + radr/2 + off_u/2);
			break;
		}
		case 2048: {
			ss.install_read_handler (adr + off_s, adr + 0x7ffff, read16sm_delegate(*this, [this, off_s, radr](offs_t offset) { return m_ram[remap_512_2048(offset + off_s/2) + radr/2]; }, "512-2048"));
			ss.install_write_handler(adr + off_s, adr + 0x7ffff, write16s_delegate(*this, [this, off_s, radr](offs_t offset, offs_t data, offs_t mem_mask) { COMBINE_DATA(&m_ram[remap_512_128(offset + off_s/2) + radr/2]); }, "512-2048"));
			su.install_read_handler (adr + off_u, adr + 0x7ffff, read16sm_delegate(*this, [this, off_u, radr](offs_t offset) { return m_ram[remap_512_2048(offset + off_u/2) + radr/2]; }, "512-2048"));
			su.install_write_handler(adr + off_u, adr + 0x7ffff, write16s_delegate(*this, [this, off_u, radr](offs_t offset, offs_t data, offs_t mem_mask) { COMBINE_DATA(&m_ram[remap_512_128(offset + off_u/2) + radr/2]); }, "512-2048"));
			break;
		}
		}
		break;
	case 2048:
		switch(actual) {
		case    0: {
			ss.unmap_readwrite(adr + off_s, adr + 0x1fffff);
			su.unmap_readwrite(adr + off_u, adr + 0x1fffff);
			break;
		}
		case  128: {
			ss.install_read_handler (adr + off_s, adr + 0x1fffff, read16sm_delegate(*this, [this, off_s, radr](offs_t offset) { return m_ram[remap_2048_128(offset + off_s/2) + radr/2]; }, "2048-128"));
			ss.install_write_handler(adr + off_s, adr + 0x1fffff, write16s_delegate(*this, [this, off_s, radr](offs_t offset, offs_t data, offs_t mem_mask) { COMBINE_DATA(&m_ram[remap_2048_128(offset + off_s/2) + radr/2]); }, "2048-128"));
			su.install_read_handler (adr + off_u, adr + 0x1fffff, read16sm_delegate(*this, [this, off_u, radr](offs_t offset) { return m_ram[remap_2048_128(offset + off_u/2) + radr/2]; }, "2048-128"));
			su.install_write_handler(adr + off_u, adr + 0x1fffff, write16s_delegate(*this, [this, off_u, radr](offs_t offset, offs_t data, offs_t mem_mask) { COMBINE_DATA(&m_ram[remap_2048_128(offset + off_u/2) + radr/2]); }, "2048-128"));
			break;
		}
		case  512: {
			ss.install_read_handler (adr + off_s, adr + 0x1fffff, read16sm_delegate(*this, [this, off_s, radr](offs_t offset) { return m_ram[remap_2048_512(offset + off_s/2) + radr/2]; }, "2048-512"));
			ss.install_write_handler(adr + off_s, adr + 0x1fffff, write16s_delegate(*this, [this, off_s, radr](offs_t offset, offs_t data, offs_t mem_mask) { COMBINE_DATA(&m_ram[remap_2048_512(offset + off_s/2) + radr/2]); }, "2048-512"));
			su.install_read_handler (adr + off_u, adr + 0x1fffff, read16sm_delegate(*this, [this, off_u, radr](offs_t offset) { return m_ram[remap_2048_512(offset + off_u/2) + radr/2]; }, "2048-512"));
			su.install_write_handler(adr + off_u, adr + 0x1fffff, write16s_delegate(*this, [this, off_u, radr](offs_t offset, offs_t data, offs_t mem_mask) { COMBINE_DATA(&m_ram[remap_2048_512(offset + off_u/2) + radr/2]); }, "2048-512"));
			break;
		}
		case 2048: {
			ss.install_ram(adr + off_s, adr + 0x1fffff, m_ram + radr/2 + off_s/2);
			su.install_ram(adr + off_u, adr + 0x1fffff, m_ram + radr/2 + off_u/2);
			break;
		}
		}
		break;
	}
}

void st_mmu_device::configure_ram()
{
	u32 b0 = 0, b1 = 0;
	switch(m_ram_size) {
	case  256*1024: b0 =  128; b1 =  128; break;
	case  512*1024: b0 =  512; b1 =    0; break;
	case 1024*1024: b0 =  512; b1 =  512; break;
	case 2048*1024: b0 = 2048; b1 =    0; break;
	case 4096*1024: b0 = 2048; b1 = 2048; break;
	}

	const u32 bank_sizes[4] = { 128, 512, 2048, 0 };
	u32 c0 = bank_sizes[(m_memcfg >> 2) & 3];
	u32 c1 = bank_sizes[m_memcfg & 3];

	logerror("ram banks %d/%d configured %d/%d\n", b0, b1, c0, c1);

	constexpr u32 st_s = 0x000008;
	constexpr u32 st_u = 0x000800;

	ram_mapping(b0, c0, 0, 0, st_s, st_u);
	ram_mapping(b1, c1, 1024*c0, 1024*b0, 0, 0);

	u32 ub = 1024*(c0+c1);
	if(ub < 0x400000) {
		m_cpu->space(AS_PROGRAM).nop_readwrite(ub, 0x3fffff);
		m_cpu->space(m68000_device::AS_USER_PROGRAM).nop_readwrite(ub, 0x3fffff);
	}

	m_cpu->space(AS_PROGRAM).install_readwrite_before_time(st_s, 0x3fffff, ws_time_delegate(*this, FUNC(st_mmu_device::bus_contention)));
	m_cpu->space(m68000_device::AS_USER_PROGRAM).install_readwrite_before_time(st_u, 0x3fffff, ws_time_delegate(*this, FUNC(st_mmu_device::bus_contention)));
}

u64 st_mmu_device::bus_contention(offs_t address, u64 current_time) const
{
	if(current_time & 3)
		current_time = (current_time | 3) + 1;
	return current_time;
}

