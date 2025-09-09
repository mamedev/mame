// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM RT PC I/O Channel Converter/Controller
 *
 * Sources:
 *   - IBM RT PC Hardware Technical Reference Volume I, 75X0232, March 1987
 *
 * TODO:
 *   - isa bus i/o space should be halfword addressed
 *   - alternate controllers, region mode dma
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

	set_int(false);
}

u8 rtpc_iocc_device::dma_b_r(offs_t offset)
{
	u8 data = 0;

	if (!BIT(m_dmr, 7 - m_adc))
	{
		u16 const tcw = m_tcw[(m_adc << 6) | (offset >> 11)];
		u32 const real = ((tcw & TCW_PFX) << 11) | (offset & 0x7ff);

		LOGMASKED(LOG_DMA, "dma0 tcw 0x%04x real 0x%08x\n", tcw, real);
		if (tcw & TCW_IOC)
			data = m_mem.read_byte(real);
		else
		{
			if (!m_rsc->mem_load(real, data, RSC_N))
			{
				// on dma exception
				// - assert interrupt (level 2)
				// - csr | DE | ~PER | INTP

				m_csr &= ~CSR_PER;
				m_csr |= (CSR_DE0 >> m_adc) | CSR_DEXK;
				m_out_rst(1);
				m_out_rst(0);
				set_int(true);
			}
		}
	}
	else
		fatalerror("rtpc_iocc_device::dma_b_r() invalid dma operation\n");

	return data;
}

void rtpc_iocc_device::dma_b_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_DMA, "dma0 offset 0x%04x data 0x%02x\n", offset, data);

	if (!BIT(m_dmr, 7 - m_adc))
	{
		u16 const tcw = m_tcw[(m_adc << 6) | (offset >> 11)];
		u32 const real = ((tcw & TCW_PFX) << 11) | (offset & 0x7ff);

		LOGMASKED(LOG_DMA, "dma0 tcw 0x%04x real 0x%08x\n", tcw, real);
		if (tcw & TCW_IOC)
			m_mem.write_byte(real, data);
		else
			m_rsc->mem_store(real, data, RSC_N);
	}
	else
		fatalerror("rtpc_iocc_device::dma_b_w() invalid dma operation\n");
}

u8 rtpc_iocc_device::dma_w_r(offs_t offset)
{
	u16 data = 0;

	if (!BIT(m_dmr, 7 - m_adc))
	{
		u16 const tcw = m_tcw[(m_adc << 6) | (offset >> 10)];
		u32 const real = ((tcw & TCW_PFX) << 11) | (offset & 0x7ff);

		LOGMASKED(LOG_DMA, "dma1 tcw 0x%04x real 0x%08x\n", tcw, real);
		if (tcw & TCW_IOC)
			data = swapendian_int16(m_mem.read_word(real));
		else
			m_rsc->mem_load(real, data, RSC_N);
	}
	else
		fatalerror("rtpc_iocc_device::dma_w_r() invalid dma operation\n");

	return data;
}

void rtpc_iocc_device::dma_w_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_DMA, "dma1 offset 0x%04x data 0x%02x\n", offset, data);

	if (!BIT(m_dmr, 7 - m_adc))
	{
		u16 const tcw = m_tcw[(m_adc << 6) | (offset >> 10)];
		u32 const real = ((tcw & TCW_PFX) << 11) | ((offset & 0x7ff) << 1);

		LOGMASKED(LOG_DMA, "dma1 tcw 0x%04x real 0x%08x\n", tcw, real);
		// FIXME: upper data bits
		if (tcw & TCW_IOC)
			m_mem.write_word(real, swapendian_int16(data));
		else
			m_rsc->mem_store(real, u16(data), RSC_N);
	}
	else
		fatalerror("rtpc_iocc_device::dma_w_w() invalid dma operation\n");
}

#ifdef _MSC_VER
// avoid incorrect MSVC warnings about excessive shift sizes below
#pragma warning(disable:4333)
#endif

template <typename T> bool rtpc_iocc_device::mem_load(u32 address, T &data, rsc_mode const mode)
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

template <typename T> bool rtpc_iocc_device::mem_store(u32 address, T data, rsc_mode const mode)
{
	int const spacenum = (address >> 24) & 15 ? AS_PROGRAM : AS_IO;

	if (spacenum == AS_PROGRAM && (mode & RSC_U) && !(m_ccr & CCR_MMP))
	{
		LOG("store mem protection violation (%s)\n", machine().describe_context());
		m_csr |= CSR_PER | CSR_PD | CSR_PVIO;
		if (mode & RSC_T)
		{
			m_csr |= CSR_EXR;
			return false;
		}
		else
			set_int(true);

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

template <typename T> bool rtpc_iocc_device::mem_modify(u32 address, std::function<T(T)> f, rsc_mode const mode)
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
			set_int(true);

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

template <typename T> bool rtpc_iocc_device::pio_load(u32 address, T &data, rsc_mode const mode)
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

template <typename T> bool rtpc_iocc_device::pio_store(u32 address, T data, rsc_mode const mode)
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
			set_int(true);

		return true;
	}
	else if ((address & REG_MASK) == CSR_ADDR)
	{
		m_csr = 0;
		set_int(false);

		return true;
	}
	else if ((address & REG_MASK) == TCW_BASE)
	{
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

template <typename T> bool rtpc_iocc_device::pio_modify(u32 address, std::function<T(T)> f, rsc_mode const mode)
{
	LOG("modify pio space invalid operation (%s)\n", machine().describe_context());
	m_csr |= CSR_EXR | CSR_PER | CSR_PD | CSR_INVOP;

	return false;
}
