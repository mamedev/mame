// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM RT PC I/O Channel Converter/Controller
 *
 * Sources:
 *  - IBM RT PC Hardware Technical Reference, Volume I (75X0232), Second Edition (September 1986), International Business Machines Corporation.
 *
 * TODO:
 *  - isa bus i/o space should be halfword addressed
 *  - 16-bit system controller dma
 */

#include "emu.h"
#include "rtpc_iocc.h"

#define LOG_DMA     (1U << 1)
#define LOG_WIDEPIO (1U << 2)

//#define VERBOSE (LOG_GENERAL|LOG_DMA)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(RTPC_IOCC, rtpc_iocc_device, "rtpc_iocc", "IBM RT PC I/O Channel Converter/Controller")

rtpc_iocc_device::rtpc_iocc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RTPC_IOCC, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, rsc_bus_interface(mconfig, *this)
	, m_mem_config("mem", ENDIANNESS_LITTLE, 16, 24, 0)
	, m_pio_config("pio", ENDIANNESS_LITTLE, 16, 24, 0)
	, m_out_int(*this)
	, m_out_rst(*this)
	, m_rsc(*this, "^mmu")
	, m_csr(0)
	, m_ccr(0)
	, m_dbr(0)
	, m_dmr(0)
	, m_tcw{}
	, m_adc(0)
	, m_out_int_state(false)
{
}

device_memory_interface::space_config_vector rtpc_iocc_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_mem_config),
		std::make_pair(AS_IO, &m_pio_config)
	};
}

void rtpc_iocc_device::device_start()
{
	save_item(NAME(m_csr));
	save_item(NAME(m_ccr));
	save_item(NAME(m_dbr));
	save_item(NAME(m_dmr));
	save_item(NAME(m_tcw));
	save_item(NAME(m_adc));
	save_item(NAME(m_out_int_state));

	space(AS_PROGRAM).specific(m_mem);
	space(AS_IO).specific(m_pio);
}

void rtpc_iocc_device::device_reset()
{
	m_csr = 0;

	interrupt(false);
}

u8 rtpc_iocc_device::dma_r(offs_t offset)
{
	u8 data = 0;

	if (!BIT(m_dmr, 7 - m_adc))
	{
		// page mode
		u16 const tcw = m_tcw[(m_adc << 6) | BIT(offset, 11, 5)];
		offs_t const addr = BIT(tcw, 0, 13) << 11 | (offset & 0x7ff);

		LOGMASKED(LOG_DMA, "dma_r tcw 0x%04x addr 0x%08x\n", tcw, addr);

		if (tcw & TCW_IOC)
			data = m_mem.read_byte(addr);
		else
			if (!m_rsc->mem_load(addr, data, u8(-1), RSC_N))
				dma_error(m_adc, CSR_DEXK);
	}
	else
		dma_error(m_adc, CSR_INVOP);

	return data;
}

void rtpc_iocc_device::dma_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_DMA, "dma_w offset 0x%04x data 0x%02x\n", offset, data);

	if (!BIT(m_dmr, 7 - m_adc))
	{
		// page mode
		u16 const tcw = m_tcw[(m_adc << 6) | BIT(offset, 11, 5)];
		offs_t const addr = BIT(tcw, 0, 13) << 11 | (offset & 0x7ff);

		LOGMASKED(LOG_DMA, "dma_w tcw 0x%04x addr 0x%08x\n", tcw, addr);

		if (tcw & TCW_IOC)
			m_mem.write_byte(addr, data);
		else
			if (!m_rsc->mem_store(addr, data, u8(-1), RSC_N))
				dma_error(m_adc, CSR_DEXK);
	}
	else
		dma_error(m_adc, CSR_INVOP);
}

// TODO: alternate controller: check ch8 enable
template <unsigned Channel> u16 rtpc_iocc_device::alt_r(offs_t offset, u16 mem_mask)
{
	bool const region = (Channel == 8) || BIT(m_dmr, 7 - Channel);

	u16 data = 0;

	// retrieve translation control word
	u16 const tcw = region
		? m_tcw[512 + BIT(offset, 15, 9)]
		: m_tcw[(Channel << 6) | BIT(offset, 11, 6)];

	if (!(tcw & TCW_IOC))
	{
		// translate to system address
		offs_t address = region
			? BIT(tcw, 4, 9) << 15 | (offset & 0x7fff)  // region mode: 9-bit prefix + 15-bit displacement
			: BIT(tcw, 0, 13) << 11 | (offset & 0x7ff); // page mode: 13-bit prefix + 11-bit displacement

		rsc_mode mode = rsc_mode::RSC_N;

		// region mode virtual address?
		if (region && (tcw & TCW_VIR))
		{
			address |= 0xe000'0000U;
			mode = rsc_mode::RSC_T;
		}

		if (!m_rsc->mem_load(address, data, mem_mask, mode) && !machine().side_effects_disabled())
			dma_error(Channel, CSR_DEXK);

		data = swapendian_int16(data);

		LOGMASKED(LOG_DMA, "alt_r %s 0x%06x tcw 0x%04x address 0x%08x data 0x%04x mask 0x%04x\n",
			region ? "region" : "page", offset, tcw, address, data, mem_mask);
	}
	else
		// I/O channel accesses are passed through without translation
		data = m_mem.read_word(offset, mem_mask);

	return data;
}

template <unsigned Channel> void rtpc_iocc_device::alt_w(offs_t offset, u16 data, u16 mem_mask)
{
	bool const region = (Channel == 8) || BIT(m_dmr, 7 - Channel);

	// retrieve translation control word
	u16 const tcw = region
		? m_tcw[512 + BIT(offset, 15, 9)]
		: m_tcw[(Channel << 6) | BIT(offset, 11, 6)];

	if (!(tcw & TCW_IOC))
	{
		// translate to system address
		offs_t address = region
			? BIT(tcw, 4, 9) << 15 | (offset & 0x7fff)  // region mode: 9-bit prefix + 15-bit displacement
			: BIT(tcw, 0, 13) << 11 | (offset & 0x7ff); // page mode: 13-bit prefix + 11-bit displacement

		rsc_mode mode = rsc_mode::RSC_N;

		// region mode virtual address?
		if (region && (tcw & TCW_VIR))
		{
			address |= 0xe000'0000U;
			mode = rsc_mode::RSC_T;
		}

		LOGMASKED(LOG_DMA, "alt_w %s 0x%06x tcw 0x%04x address 0x%08x data 0x%04x mask 0x%04x\n",
			region ? "region" : "page", offset, tcw, address, data, mem_mask);

		if (!m_rsc->mem_store(address, swapendian_int16(data), mem_mask, mode) && !machine().side_effects_disabled())
			dma_error(Channel, CSR_DEXK);
	}
	else
		// I/O channel accesses are passed through without translation
		m_mem.write_word(offset, data, mem_mask);
}

template u16 rtpc_iocc_device::alt_r<0>(offs_t offset, u16 mem_mask);
template u16 rtpc_iocc_device::alt_r<1>(offs_t offset, u16 mem_mask);
template u16 rtpc_iocc_device::alt_r<2>(offs_t offset, u16 mem_mask);
template u16 rtpc_iocc_device::alt_r<3>(offs_t offset, u16 mem_mask);
template u16 rtpc_iocc_device::alt_r<5>(offs_t offset, u16 mem_mask);
template u16 rtpc_iocc_device::alt_r<6>(offs_t offset, u16 mem_mask);
template u16 rtpc_iocc_device::alt_r<7>(offs_t offset, u16 mem_mask);
template u16 rtpc_iocc_device::alt_r<8>(offs_t offset, u16 mem_mask);

template void rtpc_iocc_device::alt_w<0>(offs_t offset, u16 data, u16 mem_mask);
template void rtpc_iocc_device::alt_w<1>(offs_t offset, u16 data, u16 mem_mask);
template void rtpc_iocc_device::alt_w<2>(offs_t offset, u16 data, u16 mem_mask);
template void rtpc_iocc_device::alt_w<3>(offs_t offset, u16 data, u16 mem_mask);
template void rtpc_iocc_device::alt_w<5>(offs_t offset, u16 data, u16 mem_mask);
template void rtpc_iocc_device::alt_w<6>(offs_t offset, u16 data, u16 mem_mask);
template void rtpc_iocc_device::alt_w<7>(offs_t offset, u16 data, u16 mem_mask);
template void rtpc_iocc_device::alt_w<8>(offs_t offset, u16 data, u16 mem_mask);

bool rtpc_iocc_device::translate(int spacenum, offs_t &address, address_space *&target_space) const
{
	target_space = &space(spacenum);

	address &= 0x00ff'ffffU;

	return true;
}

template <typename T> bool rtpc_iocc_device::mem_load(offs_t address, T &data, rsc_mode const mode)
{
	if ((mode & RSC_U) && !(m_ccr & CCR_MMP))
	{
		LOG("load mem protection violation (%s)\n", machine().describe_context());
		m_csr |= CSR_EXR | CSR_PER | CSR_PD | CSR_PVIO;

		return false;
	}

	switch (sizeof(T))
	{
	case 1: data = m_mem.read_byte(address); break;
	case 2: data = swapendian_int16(m_mem.read_word(address)); break;
	case 4:
		data = u32(swapendian_int16(m_mem.read_word(address + 0))) << 16;
		data |= swapendian_int16(m_mem.read_word(address + 2));
		break;
	}

	return true;
}

template <typename T> bool rtpc_iocc_device::mem_store(offs_t address, T data, rsc_mode const mode)
{
	if ((mode & RSC_U) && !(m_ccr & CCR_MMP))
	{
		LOG("store mem protection violation (%s)\n", machine().describe_context());
		m_csr |= CSR_PER | CSR_PD | CSR_PVIO;
		if (mode & RSC_T)
		{
			m_csr |= CSR_EXR;
			return false;
		}
		else
			interrupt(true);

		return true;
	}

	switch (sizeof(T))
	{
	case 1: m_mem.write_byte(address, data); break;
	case 2: m_mem.write_word(address, swapendian_int16(data)); break;
	case 4:
		m_mem.write_word(address + 0, swapendian_int16(data >> 16));
		m_mem.write_word(address + 2, swapendian_int16(data >> 0));
		break;
	}

	return true;
}

template <typename T> bool rtpc_iocc_device::mem_modify(offs_t address, std::function<T(T)> f, rsc_mode const mode)
{
	if ((mode & RSC_U) && !(m_ccr & CCR_MMP))
	{
		LOG("modify mem protection violation (%s)\n", machine().describe_context());
		m_csr |= CSR_PER | CSR_PD | CSR_PVIO;
		if (mode & RSC_T)
		{
			m_csr |= CSR_EXR;
			return false;
		}
		else
			interrupt(true);

		return true;
	}

	switch (sizeof(T))
	{
	case 1: m_mem.write_byte(address, f(m_mem.read_byte(address))); break;
	case 2: m_mem.write_word(address, swapendian_int16(f(swapendian_int16(m_mem.read_word(address))))); break;
	case 4:
		{
			T data = u32(swapendian_int16(m_mem.read_word(address + 0))) << 16;
			data |= swapendian_int16(m_mem.read_word(address + 2));

			data = f(data);

			m_mem.write_word(address + 0, swapendian_int16(data >> 16));
			m_mem.write_word(address + 2, swapendian_int16(data >> 0));
		}
		break;
	}

	return true;
}

template <typename T> bool rtpc_iocc_device::pio_load(offs_t address, T &data, rsc_mode const mode)
{
	if ((mode & RSC_U) && (address == CSR_ADDR || !(m_ccr & CCR_IMP)))
	{
		LOG("load pio protection violation (%s)\n", machine().describe_context());
		m_csr |= CSR_EXR | CSR_PER | CSR_PD | CSR_PVIO;

		return false;
	}
	else if ((address & REG_MASK) == CSR_ADDR)
	{
		data = m_csr | CSR_RSV;

		return true;
	}
	else if ((address & REG_MASK) == TCW_BASE)
	{
		// the multiply creates a halfword smear needed to pass POST
		data = m_tcw[(address & ~REG_MASK) >> 1] * 0x0001'0001U;

		return true;
	}

	switch (sizeof(T))
	{
	case 1: data = m_pio.read_byte(address); break;
	case 2:
		switch (m_pio.lookup_read_word_flags(address) & PIO_SIZE)
		{
		case PIO_B:
			data = u16(m_pio.read_byte(address)) << 8;
			data |= m_pio.read_byte(address);
			LOGMASKED(LOG_WIDEPIO, "load pio w<-b 0x%08x data 0x%04x (%s)\n", address, data, machine().describe_context());
			break;
		case PIO_W:
			data = swapendian_int16(m_pio.read_word(address));
			break;
		}
		break;
	case 4:
		switch (m_pio.lookup_read_word_flags(address) & PIO_SIZE)
		{
		case PIO_B:
			data = u32(m_pio.read_byte(address)) << 24;
			data |= u32(m_pio.read_byte(address)) << 16;
			data |= u32(m_pio.read_byte(address)) << 8;
			data |= u32(m_pio.read_byte(address)) << 0;
			LOGMASKED(LOG_WIDEPIO, "load pio d<-b 0x%08x data 0x%08x (%s)\n", address, data, machine().describe_context());
			break;
		case PIO_W:
			data = u32(swapendian_int16(m_pio.read_word(address))) << 16;
			data |= swapendian_int16(m_pio.read_word(address));
			LOGMASKED(LOG_WIDEPIO, "load pio d<-w 0x%08x data 0x%08x(%s)\n", address, data, machine().describe_context());
			break;
		}
		break;
	}

	return true;
}

template <typename T> bool rtpc_iocc_device::pio_store(offs_t address, T data, rsc_mode const mode)
{
	if ((mode & RSC_U) && (address == CSR_ADDR || !(m_ccr & CCR_IMP)))
	{
		LOG("store pio protection violation (%s)\n", machine().describe_context());
		m_csr |= CSR_PER | CSR_PD | CSR_PVIO;
		if (mode & RSC_T)
		{
			m_csr |= CSR_EXR;
			return false;
		}
		else
			interrupt(true);

		return true;
	}
	else if ((address & REG_MASK) == CSR_ADDR)
	{
		m_csr = 0;
		interrupt(false);

		return true;
	}
	else if ((address & REG_MASK) == TCW_BASE)
	{
		logerror("tcw[0x%03x] = 0x%04x\n", (address & ~REG_MASK) >> 1, data);
		m_tcw[(address & ~REG_MASK) >> 1] = data;

		return true;
	}

	switch (sizeof(T))
	{
	case 1: m_pio.write_byte(address, data); break;
	case 2:
		switch (m_pio.lookup_write_word_flags(address) & PIO_SIZE)
		{
		case PIO_B:
			LOGMASKED(LOG_WIDEPIO, "store pio w->b 0x%08x data 0x%04x (%s)\n", address, data, machine().describe_context());
			m_pio.write_byte(address, data >> 8);
			m_pio.write_byte(address, data >> 0);
			break;
		case PIO_W:
			m_pio.write_word(address, swapendian_int16(data));
			break;
		}
		break;
	case 4:
		switch (m_pio.lookup_write_word_flags(address) & PIO_SIZE)
		{
		case PIO_B:
			// HACK: suppress excessive logging from frequent delay register word writes
			if (address != 0xf000'80e0U)
				LOGMASKED(LOG_WIDEPIO, "store pio d->b 0x%08x data 0x%08x (%s)\n", address, data, machine().describe_context());
			m_pio.write_byte(address, data >> 24);
			m_pio.write_byte(address, data >> 16);
			m_pio.write_byte(address, data >> 8);
			m_pio.write_byte(address, data >> 0);
			break;
		case PIO_W:
			LOGMASKED(LOG_WIDEPIO, "store pio d->w 0x%08x data 0x%08x (%s)\n", address, data, machine().describe_context());
			m_pio.write_word(address, swapendian_int16(data >> 16));
			m_pio.write_word(address, swapendian_int16(data >> 0));
			break;
		}
		break;
	}

	return true;
}

template <typename T> bool rtpc_iocc_device::pio_modify(offs_t address, std::function<T(T)> f, rsc_mode const mode)
{
	LOG("modify pio space invalid operation (%s)\n", machine().describe_context());
	m_csr |= CSR_EXR | CSR_PER | CSR_PD | CSR_INVOP;

	return false;
}

void rtpc_iocc_device::interrupt(bool state)
{
	if (state != m_out_int_state)
	{
		if (state)
			m_csr |= CSR_INTP;

		m_out_int_state = state;
		m_out_int(!m_out_int_state);
	}
}

void rtpc_iocc_device::dma_error(unsigned channel, u32 type)
{
	static constexpr u32 map[] = { CSR_DE0, CSR_DE1, CSR_DE2, CSR_DE3, 0, CSR_DE5, CSR_DE6, CSR_DE7, CSR_DE8 };

	m_csr &= ~(CSR_PER | CSR_PD);
	m_csr |= map[channel] | type;

	// reset dma controllers
	m_out_rst(1);
	m_out_rst(0);

	interrupt(true);
}
