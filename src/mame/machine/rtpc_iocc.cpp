// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM RT PC I/O Channel Converter/Controller
 *
 * Sources:
 *   - http://bitsavers.org/pdf/ibm/pc/rt/75X0232_RT_PC_Technical_Reference_Volume_1_Jun87.pdf
 *
 * TODO:
 *   - bus spaces should be little endian
 *   - alternate controllers, region mode dma
 *   - improve rsc interface
 *   - state saving
 *   - refactoring and cleanup
 */

#include "emu.h"
#include "rtpc_iocc.h"

#define LOG_GENERAL (1U << 0)
#define LOG_DMA     (1U << 1)
#define LOG_WIDEPIO (1U << 2)

//#define VERBOSE (LOG_GENERAL|LOG_DMA)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(RTPC_IOCC, rtpc_iocc_device, "rtpc_iocc", "RT PC I/O Channel Converter/Controller")

rtpc_iocc_device::rtpc_iocc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RTPC_IOCC, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, rsc_bus_interface(mconfig, *this)
	, m_mem_config("mem", ENDIANNESS_BIG, 16, 24, 0)
	, m_pio_config("pio", ENDIANNESS_BIG, 16, 24, 0)
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
	m_out_int.resolve_safe();
	m_out_rst.resolve_safe();

	save_item(NAME(m_csr));
	save_item(NAME(m_ccr));
	save_item(NAME(m_dbr));
	save_item(NAME(m_dmr));
	save_item(NAME(m_tcw));
	save_item(NAME(m_adc));
	save_item(NAME(m_out_int_state));
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
			data = space(AS_PROGRAM).read_byte(real);
		else
		{
			if (!m_rsc->load(real, data, RSC_N))
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
			space(AS_PROGRAM).write_byte(real, data);
		else
			m_rsc->store(real, data, RSC_N);
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
			data = space(AS_PROGRAM).read_word(real);
		else
			m_rsc->load(real, data, RSC_N);
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
			space(AS_PROGRAM).write_word(real, data);
		else
			m_rsc->store(real, u16(data), RSC_N);
	}
	else
		fatalerror("rtpc_iocc_device::dma_w_w() invalid dma operation\n");
}

template <typename T> bool rtpc_iocc_device::load(u32 address, T &data, rsc_mode const mode)
{
	unsigned const s = (address >> 24) & 15 ? AS_PROGRAM : AS_IO;

	if (s == AS_PROGRAM && (mode & RSC_U) && !(m_ccr & CCR_MMP))
	{
		LOG("load mem protection violation (%s)\n", machine().describe_context());
		m_csr |= CSR_EXR | CSR_PER | CSR_PD | CSR_PVIO;

		return false;
	}

	if (s == AS_IO)
	{
		if ((mode & RSC_U) && (address == 0xf001'0800U || !(m_ccr & CCR_IMP)))
		{
			LOG("load pio protection violation (%s)\n", machine().describe_context());
			m_csr |= CSR_EXR | CSR_PER | CSR_PD | CSR_PVIO;

			return false;
		}
	}

	switch (sizeof(T))
	{
	case 1: data = space(s).read_byte(address); break;
	case 2:
		if (s == 2 && target_size(address) < sizeof(T))
		{
			LOGMASKED(LOG_WIDEPIO, "load pio w<-b 0x%08x (%s)\n", address, machine().describe_context());
			data = u16(space(s).read_byte(address)) << 8;
			data |= space(s).read_byte(address);
		}
		else
			data = space(s).read_word(address);
		break;
	case 4:
		if (s == 2 && target_size(address) < sizeof(T))
		{
			if (target_size(address) == 1)
			{
				LOGMASKED(LOG_WIDEPIO, "load pio d<-b 0x%08x (%s)\n", address, machine().describe_context());
				data = u32(space(s).read_byte(address)) << 24;
				data |= u32(space(s).read_byte(address)) << 16;
				data |= u32(space(s).read_byte(address)) << 8;
				data |= space(s).read_byte(address);
			}
			else
			{
				LOGMASKED(LOG_WIDEPIO, "load pio d<-w 0x%08x (%s)\n", address, machine().describe_context());
				data = u32(space(s).read_word(address)) << 16;
				data |= space(s).read_word(address);
			}
		}
		else
			data = space(s).read_dword(address);
		break;
	}

	return true;
}

#ifdef _MSC_VER
// avoid incorrect MSVC warnings about excessive shift sizes below
#pragma warning(disable:4333)
#endif

template <typename T> bool rtpc_iocc_device::store(u32 address, T data, rsc_mode const mode)
{
	unsigned const spacenum = (address >> 24) & 15 ? AS_PROGRAM : AS_IO;

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

	if (spacenum == AS_IO && (mode & RSC_U))
	{
		if (address == 0xf001'0800U || !(m_ccr & CCR_IMP))
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
	}

	switch (sizeof(T))
	{
	case 1: space(spacenum).write_byte(address, data); break;
	case 2:
		if (spacenum == AS_IO && target_size(address) < sizeof(T))
		{
			LOGMASKED(LOG_WIDEPIO, "store pio w->b 0x%08x data 0x%04x (%s)\n", address, data, machine().describe_context());
			space(spacenum).write_byte(address, u8(data >> 8));
			space(spacenum).write_byte(address, u8(data >> 0));
		}
		else
			space(spacenum).write_word(address, data);
		break;
	case 4:
		if (spacenum == AS_IO && target_size(address) < sizeof(T))
		{
			if (target_size(address) == 1)
			{
				LOGMASKED(LOG_WIDEPIO, "store pio d->b 0x%08x data 0x%08x (%s)\n", address, data, machine().describe_context());
				space(spacenum).write_byte(address, u8(data >> 24));
				space(spacenum).write_byte(address, u8(data >> 16));
				space(spacenum).write_byte(address, u8(data >> 8));
				space(spacenum).write_byte(address, u8(data >> 0));
			}
			else
			{
				LOGMASKED(LOG_WIDEPIO, "store pio d->w 0x%08x data 0x%08x (%s)\n", address, data, machine().describe_context());
				space(spacenum).write_word(address, u16(data >> 16));
				space(spacenum).write_word(address, u16(data >> 0));
			}
		}
		else
			space(spacenum).write_dword(address, data);
		break;
	}

	return true;
}

template <typename T> bool rtpc_iocc_device::modify(u32 address, std::function<T(T)> f, rsc_mode const mode)
{
	unsigned const spacenum = (address >> 24) & 15 ? AS_PROGRAM : AS_IO;

	if (spacenum == AS_PROGRAM && (mode & RSC_U) && !(m_ccr & CCR_MMP))
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

	if (spacenum == AS_IO)
	{
		LOG("modify pio space invalid operation (%s)\n", machine().describe_context());
		m_csr |= CSR_EXR | CSR_PER | CSR_PD | CSR_INVOP;

		return false;
	}

	switch (sizeof(T))
	{
	case 1: space(spacenum).write_byte(address, f(space(spacenum).read_byte(address))); break;
	case 2: space(spacenum).write_word(address, f(space(spacenum).read_word(address))); break;
	case 4: space(spacenum).write_dword(address, f(space(spacenum).read_dword(address))); break;
	}

	return true;
}
