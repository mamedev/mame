// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "generalplus_gpl_chx.h"

// System DMA memory mode can be configured to
// 1 (CHA) or 7 (CHB) but the software we've seen doesn't appear to do that
// so it's unclear how that relates to any of this

#define LOG_CHX (1U << 1)

#define VERBOSE     (LOG_CHX)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(GPL_CHX, gpl_chx_device, "gpl_chx", "Generalplus GPL162xx / GPL951xx CHA/CHB Sound")

gpl_chx_device::gpl_chx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GPL_CHX, tag, owner, clock),
	m_cha_output_cb(*this),
	m_chb_output_cb(*this),
	m_updateirqs_cb(*this)
{
}

void gpl_chx_device::device_start()
{
	save_item(NAME(m_cha_ctrl));
	save_item(NAME(m_chb_ctrl));
	save_item(NAME(m_cha_fifo_reg));
	save_item(NAME(m_chb_fifo_reg));
	save_item(NAME(m_cha_fifo));
	save_item(NAME(m_chb_fifo));
	save_item(NAME(m_cha_fifo_readpos));
	save_item(NAME(m_cha_fifo_writepos));
	save_item(NAME(m_cha_fifo_entries));
	save_item(NAME(m_chb_fifo_readpos));
	save_item(NAME(m_chb_fifo_writepos));
	save_item(NAME(m_chb_fifo_entries));
}

void gpl_chx_device::device_reset()
{
	m_cha_ctrl = 0;
	m_chb_ctrl = 0;

	for (int i = 0; i < 16; i++)
	{
		m_cha_fifo[i] = 0;
		m_chb_fifo[i] = 0;
	}

	m_cha_fifo_reg = 0;
	m_chb_fifo_reg = 0;

	m_cha_fifo_readpos = 0;
	m_cha_fifo_writepos = 0;
	m_cha_fifo_entries = 0;

	m_chb_fifo_readpos = 0;
	m_chb_fifo_writepos = 0;
	m_chb_fifo_entries = 0;
}


// CHA (for sound output)

// P_CHA_Ctrl
//
// 15  FEMI/C - FIFO Empty IRQ Flag - Write to clear
// 14  FEMIEN - FIFO Empty Interrupt Enable
// 13  CHAEN  - CHA Enable
// 12  DACBEN
//
// 11  SIGNEN - used signed data
// 10  AMP_PE - Positive-side push-pull amp enable
//  9  AMP_NE - Negative-side push-pull amp enable
//  8
//
//  7  ONE_DAC - Mix CHA and CHB data to CHA
//  6  GAIN[3]
//  5  GAIN[2]
//  4  GAIN[1]
//
//  3  GAIN[0]
//  2  CASCADE1 - External signal1 (ACIN) mixing enable
//  1  CASCADE0 - External signal0 (ACIN) mixing enable
//  0

u16 gpl_chx_device::cha_ctrl_r()
{
	LOGMASKED(LOG_CHX, "%s: gpl_chx_device::cha_ctrl_r\n", machine().describe_context());
	return m_cha_ctrl;
}

void gpl_chx_device::cha_ctrl_w(u16 data)
{
	LOGMASKED(LOG_CHX, "%s: gpl_chx_device::cha_ctrl_w %04x\n", machine().describe_context(), data);

	if (data & 0x8000)
	{
		// clear flag
		data &= 0x7fff;
		m_updateirqs_cb(1);
	}

	m_cha_ctrl = data;

	//check_cha_fifo_empty();
}

// P_CHA_Data
//
// 15-0  CHADATA

u16 gpl_chx_device::cha_data_r()
{
	LOGMASKED(LOG_CHX, "%s: gpl_chx_device::cha_data_r\n", machine().describe_context());
	return 0xffff;
}

void gpl_chx_device::cha_data_w(u16 data)
{
	LOGMASKED(LOG_CHX, "%s: gpl_chx_device::cha_data_w %04x\n", machine().describe_context(), data);

	if (m_cha_fifo_entries < 16)
	{
		m_cha_fifo[m_cha_fifo_writepos] = data;
		m_cha_fifo_writepos++;
		m_cha_fifo_writepos &= 0xf;

		m_cha_fifo_entries++;
	}
	else
	{
		// trying to overflow the FIFO
	}
}

// P_CHA_FIFO
//
// 15  FFUL - CHA FIFO full flag
// 14  FFUNRN - CHA FIFO under run flag
// 13
// 12
//
// 11
// 10
//  9
//  8  FRST - FIFO Reset
//
//  7  CHAFEILV[3] - CHA FIFO Empty Interrupt Level
//  6  CHAFEILV[2]
//  5  CHAFEILV[1]
//  4  CHAFEILV[0]
//
//  3  CHAFINX[3] - CHA FIFO Used
//  2  CHAFINX[2]
//  1  CHAFINX[1]
//  0  CHAFINX[0]

u16 gpl_chx_device::cha_fifo_r()
{
	LOGMASKED(LOG_CHX, "%s: gpl_chx_device::cha_fifo_r\n", machine().describe_context());
	return (m_cha_fifo_reg & 0xfff0) | (m_cha_fifo_entries & 0x000f);
}

void gpl_chx_device::cha_fifo_w(u16 data)
{
	LOGMASKED(LOG_CHX, "%s: gpl_chx_device::cha_fifo_w %04x\n", machine().describe_context(), data);
	m_cha_fifo_reg = (m_cha_fifo_reg & 0xff0f) | (data & 0x00f0);

	if (data & 0x0100)
	{
		m_cha_fifo_readpos = 0;
		m_cha_fifo_writepos = 0;
		m_cha_fifo_entries = 0;
	}
}

// P_CHB_Ctrl
// this is different to CHA_Ctrl
//
// 15  FEMIF/C  - FIFO Empty Interrupt Flag - write to clear
// 14  FEMIEN   - FIFO Empty Interrupt Enable
// 13  CHBEN    - CHB Enable
// 12  SSF      - CHB service Frequency (0 = different to CHAA, 1 = the same)
//
// 11  CHACFG   - CHB uses CHA config (0 = CHB config, 1 = CHA config)
// 10  MONO     - Mono mode (0 = Stereo, 1 = Mono)
//  9
//  8
//
//  7
//  6
//  5
//  4
//
//  3
//  2
//  1
//  0

u16 gpl_chx_device::chb_ctrl_r()
{
	LOGMASKED(LOG_CHX, "%s: gpl_chx_device::chb_ctrl_r\n", machine().describe_context());
	return m_chb_ctrl;
}

void gpl_chx_device::chb_ctrl_w(u16 data)
{
	LOGMASKED(LOG_CHX, "%s: gpl_chx_device::chb_ctrl_w %04x\n", machine().describe_context(), data);

	if (data & 0x8000)
	{
		// clear flag
		data &= 0x7fff;
		m_updateirqs_cb(1);
	}

	m_chb_ctrl = data;

	//check_chb_fifo_empty();
}

// P_CHB_Data
//
// 15-0  CHBDATA

u16 gpl_chx_device::chb_data_r()
{
	LOGMASKED(LOG_CHX, "%s: gpl_chx_device::chb_data_r\n", machine().describe_context());
	return 0xffff;
}

void gpl_chx_device::chb_data_w(u16 data)
{
	LOGMASKED(LOG_CHX, "%s: gpl_chx_device::chb_data_w %04x\n", machine().describe_context(), data);

	if (m_chb_fifo_entries < 16)
	{
		m_chb_fifo[m_cha_fifo_writepos] = data;
		m_chb_fifo_writepos++;
		m_chb_fifo_writepos &= 0xf;

		m_chb_fifo_entries++;
	}
	else
	{
		// trying to overflow the FIFO
	}
}

// P_CHB_FIFO
//
// same as CHA_FIFO but for CHAB

u16 gpl_chx_device::chb_fifo_r()
{
	LOGMASKED(LOG_CHX, "%s: gpl_chx_device::chb_fifo_r\n", machine().describe_context());
	return (m_chb_fifo_reg & 0xfff0) | (m_chb_fifo_entries & 0x000f);
}

void gpl_chx_device::chb_fifo_w(u16 data)
{
	LOGMASKED(LOG_CHX, "%s: gpl_chx_device::chb_fifo_w %04x\n", machine().describe_context(), data);
	m_chb_fifo_reg = (m_chb_fifo_reg & 0xff0f) | (data & 0x00f0);

	if (data & 0x0100)
	{
		m_chb_fifo_readpos = 0;
		m_chb_fifo_writepos = 0;
		m_chb_fifo_entries = 0;
	}
}

void gpl_chx_device::check_cha_fifo_empty()
{
	u8 emptyamount = (m_cha_fifo_reg & 0x00f0) >> 4;

	if (m_cha_fifo_entries <= emptyamount)
	{
		if (m_cha_ctrl & 0x4000)
		{
			m_cha_ctrl |= 0x8000;
			m_updateirqs_cb(1);
		}
	}
}

void gpl_chx_device::process_cha_fifo()
{
	if (!(m_cha_ctrl & 0x2000))
		return;

	// should we be calling cha_data_r to do this?

	if (m_cha_fifo_entries > 0)
	{
		u16 retdata = m_cha_fifo[m_cha_fifo_readpos];
		m_cha_fifo_entries--;
		m_cha_fifo_readpos++;
		m_cha_fifo_readpos &= 0xf;
		m_cha_output_cb(retdata);
	}
	else
	{
		// trying to underflow the FIFO?
	}

	check_cha_fifo_empty();
}

void gpl_chx_device::check_chb_fifo_empty()
{
	u8 emptyamount = (m_chb_fifo_reg & 0x00f0) >> 4;

	if (m_chb_fifo_entries <= emptyamount)
	{
		if (m_chb_ctrl & 0x4000)
		{
			m_chb_ctrl |= 0x8000;
			m_updateirqs_cb(1);
		}
	}
}

void gpl_chx_device::process_chb_fifo()
{
	if (!(m_chb_ctrl & 0x2000))
		return;

	// should we be calling chb_data_r to do this?

	if (m_chb_fifo_entries > 0)
	{
		u16 retdata = m_chb_fifo[m_chb_fifo_readpos];
		m_chb_fifo_entries--;
		m_chb_fifo_readpos++;
		m_chb_fifo_readpos &= 0xf;
		m_chb_output_cb(retdata);
	}
	else
	{
		// trying to underflow the FIFO?
	}

	check_chb_fifo_empty();
}
