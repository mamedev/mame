// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "generalplus_gpl_dma.h"

#define LOG_GCM394_SYSDMA (1U << 1)

#define VERBOSE     (LOG_GCM394_SYSDMA)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(GPL_DMA, gpl_dma_device, "gpl_dma", "Generalplus GPL162xx / GPL951xx System DMA")

gpl_dma_device::gpl_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GPL_DMA, tag, owner, clock),
	m_space_read_cb(*this, 0),
	m_space_write_cb(*this),
	m_dma_complete_cb(*this)
{
}

void gpl_dma_device::device_start()
{
	save_item(NAME(m_dma_params));
	save_item(NAME(m_dma_latched));
	save_item(NAME(m_system_dma_memtype));
	save_item(NAME(m_dma_status));
}

void gpl_dma_device::device_reset()
{
	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 8; i++)
		{
			m_dma_params[i][j] = 0x0000;
		}
		m_dma_latched[j] = 0;
	}

	m_system_dma_memtype = 0x0000;
	m_dma_status = 0x0000;
}


// **************************************** SYSTEM DMA device *************************************************

// note, GPL162xx has 4 channels, GPL951xx only has 2
// sources also differ

u16 gpl_dma_device::read_dma_params(int channel, int offset)
{
	u16 retdata = m_dma_params[offset][channel];
	LOGMASKED(LOG_GCM394_SYSDMA, "%s:sunplus_gcm394_base_device::read_dma_params (channel %01x) %01x returning %04x\n", machine().describe_context(), channel, offset, retdata);
	return retdata;
}

void gpl_dma_device::write_dma_params(int channel, int offset, u16 data)
{
	LOGMASKED(LOG_GCM394_SYSDMA, "%s:sunplus_gcm394_base_device::write_dma_params (channel %01x) %01x %04x\n", machine().describe_context(), channel, offset, data);

	m_dma_params[offset][channel] = data;

	// TODO: very likely DMA happens whenever the length is not 0, as long as it's been enabled previously
	// jak_prft doesn't rewrite the offset 0 register between requests, and instead writes the length
	// as the final thing for each new request.  other games do not write the length last, but turn off
	// register 0 before writing params, and enable it again afterwards
	// if that's the case, this code can be refactored to work on 'length' instead of the m_dma_latched

	if (offset == 3)
	{
		m_dma_latched[channel] = true;

		if (m_dma_params[0][channel] & 1)
			trigger_systemm_dma(channel);
	}

	if (offset == 0 && (data & 1))
	{
		if (m_dma_latched[channel])
			trigger_systemm_dma(channel);
	}

}


u16 gpl_dma_device::system_dma_params_channel0_r(offs_t offset)
{
	return read_dma_params(0, offset);
}


void gpl_dma_device::system_dma_params_channel0_w(offs_t offset, u16 data)
{
	write_dma_params(0, offset, data);
}

u16 gpl_dma_device::system_dma_params_channel1_r(offs_t offset)
{
	return read_dma_params(1, offset);
}

void gpl_dma_device::system_dma_params_channel1_w(offs_t offset, u16 data)
{
	write_dma_params(1, offset, data);
}

u16 gpl_dma_device::system_dma_params_channel2_r(offs_t offset)
{
	return read_dma_params(2, offset);
}

void gpl_dma_device::system_dma_params_channel2_w(offs_t offset, u16 data)
{
	write_dma_params(2, offset, data);
}

u16 gpl_dma_device::system_dma_params_channel3_r(offs_t offset)
{
	return read_dma_params(3, offset);
}

void gpl_dma_device::system_dma_params_channel3_w(offs_t offset, u16 data)
{
	write_dma_params(3, offset, data);
}

// P_DMA_INT
//
// 15
// 14
// 13
// 12
//
// 11  CH3BY - channel busy flags (read only)
// 10  CH2BY
//  9  CH1BY
//  8  CH0BY
//
//  7  CH3TOIF - channel timeout flags
//  6  CH2TOIF
//  5  CH1TOIF
//  4  CH0TOIF
//
//  3  CH3IF - channel complete interrupt flags (write to clear)
//  2  CH2IF
//  1  CH1IF
//  0  CH0IF

u16 gpl_dma_device::system_dma_status_r()
{
	LOGMASKED(LOG_GCM394_SYSDMA, "%s:sunplus_gcm394_base_device::system_dma_status_r (7abf)\n", machine().describe_context());
	return m_dma_status;
}

void gpl_dma_device::system_dma_status_w(u16 data)
{
	// writes to the low 4 bits clear the channel complete interrupt flags
	LOGMASKED(LOG_GCM394_SYSDMA, "%s:sunplus_gcm394_base_device::system_dma_status_w %04x\n", machine().describe_context(), data);
	m_dma_status &= ~data;
}

// m_dma_params[0] is P_DMA_Ctrl
//
// 15  WriteReq  (0 = request data out, 1 = request data in)
// 14  TM        (0 = single transfer mode, 1 = demand transfer mode)
// 13  TARByte   (0 = 16-bit target, 1 = 8-bit target)
// 12  SRCByte   (0 = 16-bit source, 1 = 8-bit source)

// 11  TD[1]     (00 = Memory to Memory, 01 = Memory to I/O, 10 = I/O to Memory, 11 = invalid)
// 10  TD[0]
//  9  RS        (1 = reset channel)
//  8  CIE       (1 = enable DMA interrupt)

//  7  SF        (0 = increase/decrease source enabled, 1 = no increment)
//  6  DF        (0 = increase/decrease dest enabled, 1 = no increment)
//  5  SD        (0 = increase source, 1 = decrease source)
//  4  DD        (0 = increase dest, 1 = decrease dest)

//  3  DBF(r)/NORM_I(w)    (DBF: 1 = DMA Double Buffer Full, NORM_I: DMA Normal Interrupt mode)
//  2  MODE      (0 = Software Mode/Auto Mode, 1 = External Mode request)
//  1  BS(r/o)   (0 = DMA idle, 1 = DMA busy)
//  0  CE        (0 = Channel Disabled, 1 = Channel Enabled)

void gpl_dma_device::trigger_systemm_dma(int channel)
{
	u16 mode = m_dma_params[0][channel];
	u32 source = m_dma_params[1][channel] | (m_dma_params[4][channel] << 16);
	u32 dest = m_dma_params[2][channel] | (m_dma_params[5][channel] << 16) ;
	u32 length = m_dma_params[3][channel] | (m_dma_params[6][channel] << 16);
	int sourcedelta = 0;
	int destdelta = 0;

	if ((mode & 0xa0) == 0x00)
		sourcedelta = 1;
	else if ((mode & 0xa0) == 0x20)
		sourcedelta = -1;

	if ((mode & 0x50) == 0x00)
		destdelta = 1;
	else if ((mode & 0x50) == 0x10)
		destdelta = -1;

	static const char* tmode_names[4] = { "Memory to Memory", "Memory to IO", "IO to Memory", "Reserved" };
	u8 td = (mode & 0x0c00) >> 10;

	LOGMASKED(LOG_GCM394_SYSDMA, "%s:possible DMA operation with params mode:%04x (TD is %s) source:%08x (word offset) dest:%08x (word offset) length:%08x (words)\n", machine().describe_context(), mode, tmode_names[td], source, dest, length );

	// wrlshunt transfers ROM to RAM, all RAM write addresses have 0x800000 in the destination set

	source &= 0x0fffffff;
	length &= 0x0fffffff; // gormiti

	for (int i = 0; i < length; i++)
	{
		u16 val;
		if (mode & 0x1000)
		{
			val = (m_space_read_cb(source) & 0xff) | (m_space_read_cb(source) << 8);
			i++;
		}
		else
		{
			val = m_space_read_cb(source);
		}

		source += sourcedelta;

		if (mode & 0x2000)
		{
			m_space_write_cb(dest, val & 0xFF);
			dest += destdelta;
			m_space_write_cb(dest, val >> 8);
		}
		else
		{
			m_space_write_cb(dest, val);
		}

		dest += destdelta;
	}

	m_dma_complete_cb(1); // allow some driver specific hacks for service modes

	// clear params after operation
	m_dma_params[0][channel] = m_dma_params[0][channel] & 0x00f7;

	m_dma_params[1][channel] = m_dma_params[2][channel] = m_dma_params[3][channel] = m_dma_params[4][channel] = m_dma_params[5][channel] = m_dma_params[6][channel] = 0x0000;
	m_dma_latched[channel] = false;

	m_dma_status |= 1 << channel; // the flag seems to be set even if IRQs are disabled

	//machine().debug_break();
}



u16 gpl_dma_device::system_dma_memtype_r()
{
	LOGMASKED(LOG_GCM394_SYSDMA, "%s:sunplus_gcm394_base_device::system_dma_memtype_r\n", machine().describe_context());
	return m_system_dma_memtype;
}

void gpl_dma_device::system_dma_memtype_w(u16 data)
{
	// these sources differ between models
	static char const* const types[16] =
	{
		"Unused / USB",
		"DAC CHA",
		"UART TX",
		"UART RX",
		"SDC/MMC 1", // reserved (GPL951xx)
		"NAND Flash",
		"Serial Interface", // ADC Auto Sample Full (GPL951xx)
		"DAC CHB",
		"ADC Auto Sample Full", // MIC ADC Auto Sample Full (GPL951xx)
		"SPI TX",
		"SPI RX",
		"USB ISO Out", // reserved (GPL951xx)
		"USB ISO In", // reserved (GPL951xx)
		"SDC/MMC 2", // reserved (GPL951xx)
		"SPUL", // SPI1 TX (GP951xx)
		"SPUH" // SPI1 RX (GP951xx)
	};

	m_system_dma_memtype = data;

	LOGMASKED(LOG_GCM394_SYSDMA, "%s:sunplus_gcm394_base_device::system_dma_memtype_w %04x (CH3: %s | CH2: %s | CH1: %s | CH0: %s )\n", machine().describe_context(), data,
		types[((m_system_dma_memtype>>12)&0xf)],
		types[((m_system_dma_memtype>>8)&0xf)],
		types[((m_system_dma_memtype>>4)&0xf)],
		types[((m_system_dma_memtype)&0xf)]);

}
