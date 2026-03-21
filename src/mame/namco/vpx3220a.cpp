// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Micronas Intermetall VPX-3220A Video Pixel Decoder

    Skeleton device

***************************************************************************/

#include "emu.h"
#include "vpx3220a.h"

#define VERBOSE ( LOG_GENERAL )
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VPX3220A, vpx3220a_device, "vpx3220a", "VPX-3220A Video Pixel Decoder")

enum vpx3220a_device::subaddr_t : u8
{
	IDENT = 0x00,
	IFC = 0x20,
	FPRD = 0x26,
	FPWR = 0x27,
	FPDAT = 0x28,
	FPSTA = 0x29,
	AFEND = 0x33,
	REFSIG = 0xd8,
	YMAX = 0xe0,
	YMIN = 0xe1,
	UMAX = 0xe2,
	UMIN = 0xe3,
	VMAX = 0xe4,
	VMIN = 0xe5,
	CBM_BRI = 0xe6,
	CBM_CON = 0xe7,
	FORMAT = 0xe8,
	MISC = 0xea,
	OFIFO = 0xf0,
	OMUX = 0xf1,
	OENA = 0xf2,
	DRIVER_A = 0xf8,
	DRIVER_B = 0xf9,
	UNKNOWN = 0xff
};

enum vpx3220a_device::fpaddr_t : u16
{
	TINT = 0x1c,
	GAIN = 0x20,
	XLG = 0x26,
	HPLL = 0x4b,
	DVCO = 0x58,
	ADJUST = 0x59,
	VBEG1 = 0x88,
	VLINEI1 = 0x89,
	VLINEO1 = 0x8a,
	HBEG1 = 0x8b,
	HLEN1 = 0x8c,
	NPIX1 = 0x8d,
	VBEG2 = 0x8e,
	VLINEI2 = 0x8f,
	VLINEO2 = 0x90,
	HBEG2 = 0x91,
	HLEN2 = 0x92,
	NPIX2 = 0x93,
	ACCREF = 0xa0,
	ACCR = 0xa3,
	ACCB = 0xa4,
	KILVL = 0xa8,
	AGCREF = 0xb2,
	SGAIN = 0xbe,
	VSDT = 0xe7,
	CMDWD = 0xf0,
	INFOWD = 0xf1,
	TVSTNDWR = 0xf2,
	TVSTNDRD = 0xf3
};

vpx3220a_device::vpx3220a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VPX3220A, tag, owner, clock),
	i2c_hle_interface(mconfig, *this, 0x43),
	m_href_cb(*this),
	m_vref_cb(*this),
	m_href_timer(nullptr),
	m_vref_timer(nullptr),
	m_href(0),
	m_vref(0),
	m_ifc(0),
	m_afend(0),
	m_unk_34(0),
	m_unk_3a(0),
	m_refsig(0),
	m_ymax(0),
	m_ymin(0),
	m_umax(0),
	m_umin(0),
	m_vmax(0),
	m_vmin(0),
	m_cbm_bri(0),
	m_cbm_con(0),
	m_format(0),
	m_misc(0),
	m_ofifo(0),
	m_omux(0),
	m_oena(0),
	m_driver_a(0),
	m_driver_b(0),
	m_fpdata(0),
	m_fpaddr(0),
	m_fpsta(0),
	m_want_fp_lsb(false),
	m_accessing_fp(false),
	m_want_fp_data_lsb(false),
	m_fp_tint(0),
	m_fp_gain(0),
	m_fp_xlg(0),
	m_fp_hpll(0),
	m_fp_dvco(0),
	m_fp_adjust(0),
	m_fp_accref(0),
	m_fp_accr(0),
	m_fp_accb(0),
	m_fp_kilvl(0),
	m_fp_agcref(0),
	m_fp_sgain(0),
	m_fp_vsdt(0),
	m_fp_cmdwd(0),
	m_fp_tvstndwr(0)
{
	memset(m_fp_vbeg, 0, sizeof(m_fp_vbeg));
	memset(m_fp_vlinei, 0, sizeof(m_fp_vlinei));
	memset(m_fp_vlineo, 0, sizeof(m_fp_vlineo));
	memset(m_fp_hbeg, 0, sizeof(m_fp_hbeg));
	memset(m_fp_hlen, 0, sizeof(m_fp_hlen));
	memset(m_fp_npix, 0, sizeof(m_fp_npix));
}

void vpx3220a_device::device_start()
{
	save_item(NAME(m_href));
	save_item(NAME(m_vref));

	save_item(NAME(m_ifc));
	save_item(NAME(m_afend));
	save_item(NAME(m_unk_34));
	save_item(NAME(m_unk_3a));
	save_item(NAME(m_refsig));
	save_item(NAME(m_ymax));
	save_item(NAME(m_ymin));
	save_item(NAME(m_umax));
	save_item(NAME(m_umin));
	save_item(NAME(m_vmax));
	save_item(NAME(m_vmin));
	save_item(NAME(m_cbm_bri));
	save_item(NAME(m_cbm_con));
	save_item(NAME(m_format));
	save_item(NAME(m_misc));
	save_item(NAME(m_ofifo));
	save_item(NAME(m_omux));
	save_item(NAME(m_oena));
	save_item(NAME(m_driver_a));
	save_item(NAME(m_driver_b));

	save_item(NAME(m_fpdata));
	save_item(NAME(m_fpaddr));
	save_item(NAME(m_fpsta));
	save_item(NAME(m_want_fp_lsb));
	save_item(NAME(m_accessing_fp));
	save_item(NAME(m_want_fp_data_lsb));

	save_item(NAME(m_fp_tint));
	save_item(NAME(m_fp_gain));
	save_item(NAME(m_fp_xlg));
	save_item(NAME(m_fp_hpll));
	save_item(NAME(m_fp_dvco));
	save_item(NAME(m_fp_adjust));
	save_item(NAME(m_fp_vbeg));
	save_item(NAME(m_fp_vlinei));
	save_item(NAME(m_fp_vlineo));
	save_item(NAME(m_fp_hbeg));
	save_item(NAME(m_fp_hlen));
	save_item(NAME(m_fp_npix));
	save_item(NAME(m_fp_accref));
	save_item(NAME(m_fp_accr));
	save_item(NAME(m_fp_accb));
	save_item(NAME(m_fp_kilvl));
	save_item(NAME(m_fp_agcref));
	save_item(NAME(m_fp_sgain));
	save_item(NAME(m_fp_vsdt));
	save_item(NAME(m_fp_cmdwd));
	save_item(NAME(m_fp_tvstndwr));

	m_href_timer = timer_alloc(FUNC(vpx3220a_device::href_tick), this);
	m_vref_timer = timer_alloc(FUNC(vpx3220a_device::vref_tick), this);
}

void vpx3220a_device::device_reset()
{
	m_href = 0;
	m_vref = 0;
	m_ifc = 0x03;
	m_afend = 0x0d;
	m_unk_34 = 0x00;
	m_unk_3a = 0x00;
	m_refsig = 0x00;
	m_ymax = 0xff;
	m_ymin = 0x00;
	m_umax = 0x7f;
	m_umin = 0x80;
	m_vmax = 0x7f;
	m_vmin = 0x80;
	m_cbm_bri = 0x00;
	m_cbm_con = 0x20;
	m_format = 0xf8;
	m_misc = 0x00;
	m_ofifo = 0x0a;
	m_omux = 0x00;
	m_oena = 0x00;
	m_driver_a = 0x12;
	m_driver_b = 0x24;

	m_fpdata = 0;
	m_fpaddr = 0;
	m_fpsta = 0;
	m_want_fp_lsb = false;
	m_accessing_fp = false;
	m_want_fp_data_lsb = false;

	m_fp_tint = 0;
	m_fp_gain = 0;
	m_fp_xlg = 0;
	m_fp_hpll = 664;
	m_fp_dvco = 0;
	m_fp_adjust = 0;
	m_fp_vbeg[0] = 12;
	m_fp_vlinei[0] = 1;
	m_fp_vlineo[0] = 1;
	m_fp_hbeg[0] = 0;
	m_fp_hlen[0] = 704;
	m_fp_npix[0] = 704;
	m_fp_vbeg[1] = 17;
	m_fp_vlinei[1] = 500;
	m_fp_vlineo[1] = 500;
	m_fp_hbeg[1] = 0;
	m_fp_hlen[1] = 704;
	m_fp_npix[1] = 704;
	m_fp_accref = 2070;
	m_fp_accr = 0;
	m_fp_accb = 0;
	m_fp_kilvl = 30;
	m_fp_agcref = 768;
	m_fp_sgain = 27;
	m_fp_vsdt = 523;
	m_fp_cmdwd = 114;
	m_fp_tvstndwr = 979;

	m_href_timer->adjust(attotime::never);
	m_vref_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(vpx3220a_device::href_tick)
{
	m_href = !m_href;
	m_href_cb(m_href);
	m_href_timer->adjust(m_href ? attotime::from_usec(52) : (attotime::from_hz(15734) - attotime::from_usec(52)), 0);
}

TIMER_CALLBACK_MEMBER(vpx3220a_device::vref_tick)
{
	m_vref = !m_vref;
	m_vref_cb(m_vref);
	m_vref_timer->adjust(attotime::from_hz(15734) * (m_vref ? 45 : 480), 0);
}

u8 vpx3220a_device::read_data(u16 offset)
{
	if (m_want_fp_data_lsb)
	{
		LOG("%s: Fast Processor Data Read (LSB): %02x\n", machine().describe_context(), (u8)m_fpdata);
		m_accessing_fp = false;
		m_want_fp_data_lsb = false;
		return (u8)m_fpdata;
	}

	switch (offset)
	{
	case FORMAT:
		LOG("%s: Read Format Selection / Alpha Keyer / Contrast Brightness: %04x\n", machine().describe_context(), offset);
		return m_format;

	case FPDAT:
		if (!m_want_fp_data_lsb)
		{
			LOG("%s: Fast Processor Data Read (MSB): %02x\n", machine().describe_context(), m_fpdata >> 8);
			m_want_fp_data_lsb = true;
			return (u8)(m_fpdata >> 8);
		}
		return 0;

	case FPSTA:
		return m_fpsta;

	default:
		LOG("%s: Read Unknown Sub-Address: %04x (FPAddr %04x)\n", machine().describe_context(), offset, m_fpaddr);
	}
	return 0;
}

void vpx3220a_device::write_data(u16 offset, u8 data)
{
	if (m_want_fp_lsb)
	{
		m_fpaddr <<= 8;
		m_fpaddr |= data;
		if (BIT(m_fpsta, 0))
		{
			LOG("%s: Write Fast Processor Register Write Address (LSB): %02x\n", machine().describe_context(), data);
		}
		else if (BIT(m_fpsta, 1))
		{
			LOG("%s: Write Fast Processor Register Read Address (LSB): %02x\n", machine().describe_context(), data);
			m_fpdata = get_fp_reg();
		}
		m_want_fp_lsb = false;
		m_accessing_fp = true;
		m_want_fp_data_lsb = false;
		return;
	}

	if (m_accessing_fp)
	{
		if (!m_want_fp_data_lsb)
		{
			if (BIT(m_fpsta, 0))
			{
				LOG("%s: Write Fast Processor Data MSB: %02x\n", machine().describe_context(), data);
				m_fpdata <<= 8;
				m_fpdata |= data;
			}
			m_want_fp_data_lsb = true;
		}
		else
		{
			if (BIT(m_fpsta, 0))
			{
				m_fpdata <<= 8;
				m_fpdata |= data;
				set_fp_reg();
			}
			m_accessing_fp = false;
			m_want_fp_data_lsb = false;
		}
		return;
	}

	switch (offset)
	{
	case IFC: {
		static char const *const IF_NAMES[4] = { "12 dB", "Reserved", "6 dB", "0 dB" };
		LOG("%s: Write Chroma Processing: %02x\n", machine().describe_context(), data);
		LOG("%s:     IF Compensation: %s\n", machine().describe_context(), IF_NAMES[data & 3]);
		} break;

	case FPRD:
		LOG("%s: Write Fast Processor Register Read Address (MSB): %02x\n", machine().describe_context(), data);
		m_fpaddr <<= 8;
		m_fpaddr |= data;
		m_fpsta = (1 << 1);
		m_want_fp_lsb = true;
		break;

	case FPWR:
		LOG("%s: Write Fast Processor Register Write Address (MSB): %02x\n", machine().describe_context(), data);
		m_fpaddr <<= 8;
		m_fpaddr |= data;
		m_fpsta = (1 << 0);
		m_want_fp_lsb = true;
		break;

	case AFEND: {
		static char const *const LUMA_NAMES[4] = { "VIN3", "VIN2", "VIN1", "Reserved" };
		LOG("%s: Write Analog Front-End Register: %02x\n", machine().describe_context(), data);
		LOG("%s:     Input Selector Luma ADC:   %s\n", machine().describe_context(), LUMA_NAMES[data & 3]);
		LOG("%s:     Input Selector Luma ADC:   %s\n", machine().describe_context(), LUMA_NAMES[data & 3]);
		LOG("%s:     Input Selector Chroma ADC: %s\n", machine().describe_context(), BIT(data, 2) ? "CIN" : "VIN1");
		LOG("%s:     Clamp Chroma ADC:          %d\n", machine().describe_context(), BIT(~data, 3));
		LOG("%s:     Standby Luma ADC:          %d\n", machine().describe_context(), BIT(data, 6));
		LOG("%s:     Standby Chroma ADC:        %d\n", machine().describe_context(), BIT(data, 7));
		} break;

	case REFSIG:
		LOG("%s: Write Href/Vref Control Register: %02x\n", machine().describe_context(), data);
		LOG("%s:     HREF Polarity:    %s\n", machine().describe_context(), BIT(data, 1) ? "Active Low" : "Active High");
		LOG("%s:     VREF Polarity:    %s\n", machine().describe_context(), BIT(data, 2) ? "Active Low" : "Active High");
		LOG("%s:     VREF Pulse Width: %d\n", machine().describe_context(), BIT(data, 3, 3) + 2);
		LOG("%s:     PREF Select:      %s\n", machine().describe_context(), BIT(data, 6) ? "PIntr" : "Odd/Even flag");
		LOG("%s:     PREF Polarity:    %s\n", machine().describe_context(), BIT(data, 7) ? "Inverted" : "Unchanged");
		break;

	case YMAX:
		LOG("%s: Write Alpha Keyer (YMAX): %02x\n", machine().describe_context(), data);
		break;
	case YMIN:
		LOG("%s: Write Alpha Keyer (YMIN): %02x\n", machine().describe_context(), data);
		break;

	case UMAX:
		LOG("%s: Write Alpha Keyer (UMAX): %02x\n", machine().describe_context(), data);
		break;
	case UMIN:
		LOG("%s: Write Alpha Keyer (UMIN): %02x\n", machine().describe_context(), data);
		break;

	case VMAX:
		LOG("%s: Write Alpha Keyer (VMAX): %02x\n", machine().describe_context(), data);
		break;
	case VMIN:
		LOG("%s: Write Alpha Keyer (VMIN): %02x\n", machine().describe_context(), data);
		break;

	case CBM_BRI:
		LOG("%s: Write Contrast Brightness 1: %02x\n", machine().describe_context(), data);
		break;
	case CBM_CON:
		LOG("%s: Write Contrast Brightness 2: %02x\n", machine().describe_context(), data);
		break;

	case FORMAT: {
		static char const *const FORMAT_NAMES[8] =
		{
			"YUV 4:2:2, YUV 4:2:2 ITUR",
			"YUV 4:4:4",
			"YUV 4:1:1",
			"YUV 4:1:1 DPCM",
			"RGB 888",
			"RGB 888 (Inverse Gamma)",
			"RGB 565 (Inverse Gamma)",
			"RGB 565 (Inverse Gamma) + Alpha Key"
		};
		LOG("%s: Write Format Selection / Alpha Keyer / Contrast Brightness: %02x\n", machine().describe_context(), data);
		LOG("%s:     Format:                      %s\n", machine().describe_context(), FORMAT_NAMES[data & 7]);
		LOG("%s:     Chroma Output Stream Format: %s\n", machine().describe_context(), BIT(data, 3) ? "Unsigned" : "Signed");
		LOG("%s:     Contrast Brightness Clamp:   %d\n", machine().describe_context(), BIT(data, 4) ? 16 : 32);
		LOG("%s:     Gamma Round Dither Enable:   %d\n", machine().describe_context(), BIT(data, 5));
		LOG("%s:     Alpha Key Polarity:          %s\n", machine().describe_context(), BIT(data, 6) ? "Active Low" : "Active High");
		LOG("%s:     Alpha Key Median Filter:     %d\n", machine().describe_context(), BIT(data, 7));
		} break;

	case MISC:
		LOG("%s: Write Diverse Settings: %02x\n", machine().describe_context(), data);
		LOG("%s:     LLC2 to ALPHA/TDO pin:     %d\n", machine().describe_context(), BIT(data, 3));
		LOG("%s:     LLC2 polarity:             %d\n", machine().describe_context(), BIT(data, 4));
		LOG("%s:     Output FIFO Pointer Reset: %d\n", machine().describe_context(), BIT(data, 5) ? "VRF=0" : "VACT(intern) positive edge");
		break;

	case OFIFO: {
		static char const *const SHUFFLE_NAMES[8] =
		{
			"000 Out[23:0] = In[23:0]",
			"001",
			"010 Out[23:0] = In[7:0, 23:8]",
			"011 Out[23:0] = In[15:0, 23:16]",
			"100 Out[23:0] = In[15:8, 23:16, 7:0]",
			"101",
			"110 Out[23:0] = In[7:0, 15:8, 23:16]",
			"111 Out[23:0] = In[23:16, 7:0, 15:8]"
		};
		LOG("%s: Write Output FIFO: %02x\n", machine().describe_context(), data);
		LOG("%s:     Half-Full Level: %d\n", machine().describe_context(), data & 0x1f);
		LOG("%s:     Bus Shuffler:    %s\n", machine().describe_context(), SHUFFLE_NAMES[BIT(data, 5, 3)]);
		} break;

	case OMUX: {
		static char const *const PORT_MODE_NAMES[4] =
		{
			"Parallel Out, 'Single Clock' A = FifoOut[23:16], B = FifoOut[15:8]",
			"'Double Clock' A = FifoOut[23:16] / FifoOut[15:8], B = FifoOut[7:0]",
			"Reserved (10)",
			"Reserved (11)"
		};
		LOG("%s: Write Output Multiplexer: %02x\n", machine().describe_context(), data);
		LOG("%s:     Port Mode:       %s\n", machine().describe_context(), PORT_MODE_NAMES[data & 3]);
		if (BIT(data, 3))
		{
			LOG("%s:     Data Reset:      %s\n", machine().describe_context(), BIT(data, 2) ? "Set Output to 0 during VACT(/FE#) = 0" : "Reserved");
		}
		else
		{
			LOG("%s:     Clock Slope:     %s\n", machine().describe_context(), BIT(data, 2) ? "Negative Edge" : "Positive Edge");
		}
		LOG("%s:     Clock Source:    %s\n", machine().describe_context(), BIT(data, 3) ? "Internal Source (PIXCLK Output)" : "External Source (PIXCLK Input)");
		LOG("%s:     /FE Delay:       %d cycles\n", machine().describe_context(), BIT(data, 4, 2));
		if (!BIT(data, 3))
		{
			LOG("%s:     Disable /FE LPF: %d\n", machine().describe_context(), BIT(data, 6));
		}
		LOG("%s:     Enable HLEN:     %d\n", machine().describe_context(), BIT(data, 7));
		} break;

	case OENA:
		LOG("%s: Write Output Enable: %02x\n", machine().describe_context(), data);
		LOG("%s:     Enable Video Port A:             %d\n", machine().describe_context(), BIT(data, 0));
		LOG("%s:     Enable Video Port B:             %d\n", machine().describe_context(), BIT(data, 1));
		LOG("%s:     Enable Controls:                 %d\n", machine().describe_context(), BIT(data, 3));
		LOG("%s:     Enable LLC-Clock to /HF:         %d\n", machine().describe_context(), BIT(data, 4));
		LOG("%s:     Enable FSY-Data to /HF:          %d\n", machine().describe_context(), BIT(data, 5));
		LOG("%s:     Synchronize HREF/VREF to PIXCLK: %d\n", machine().describe_context(), BIT(data, 6));
		LOG("%s:     Disable OEQ Pin:                 %d\n", machine().describe_context(), BIT(data, 7));
		break;

	case DRIVER_A:
		LOG("%s: Write Pad Driver Strength - TTL Output Pads A: %02x\n", machine().describe_context(), data);
		LOG("%s:     Port A Driver Strength:         %d\n", machine().describe_context(), BIT(data, 0, 3));
		LOG("%s:     PIXCLK/HF#/FE# Driver Strength: %d\n", machine().describe_context(), BIT(data, 3, 3));
		LOG("%s:     Add'l PIXCLK Driver Strength:   %d\n", machine().describe_context(), BIT(data, 6, 2));
		break;

	case DRIVER_B:
		LOG("%s: Write Pad Driver Strength - TTL Output Pads B: %02x\n", machine().describe_context(), data);
		LOG("%s:     Port B/C Driver Strength:             %d\n", machine().describe_context(), BIT(data, 0, 3));
		LOG("%s:     HREF/VREF/PREF/ALPHA Driver Strength: %d\n", machine().describe_context(), BIT(data, 3, 3));
		break;

	default:
		LOG("%s: Write Unknown Sub-Address: %02x = %02x\n", machine().describe_context(), offset, data);
		break;
	}
}

u16 vpx3220a_device::get_fp_reg()
{
	switch (m_fpaddr)
	{
	default:
		LOG("%s: Unknown Fast Processor Register Read: %04x\n", machine().describe_context(), m_fpaddr);
		return 0;
	case TINT:
	{
		static const float VAL2DEG = (1.f / 1024.f) * (180.f / 3.14159265f);
		LOG("%s: NTSC Tint Angle Read: %04x (%d)\n", machine().describe_context(), m_fp_tint, (int)(m_fp_tint * VAL2DEG));
		return m_fp_tint;
	}
	case GAIN:
		LOG("%s: AGC Gain Value Read: %04x\n", machine().describe_context(), m_fp_sgain);
		return m_fp_sgain;
	case XLG:
	{
		const u16 data = (m_fp_xlg == 100) ? 0x0fff : 0x0000;
		LOG("%s: Line-Locked Mode Status Read: %04x\n", machine().describe_context(), data);
		return data;
	}
	case HPLL:
		LOG("%s: Horizontal PLL Gain Read: %04x\n", machine().describe_context(), m_fp_hpll);
		LOG("%s:         Integrating Gain: %02x\n", machine().describe_context(), BIT(m_fp_hpll, 0, 5));
		LOG("%s:        Proportional Gain: %02x\n", machine().describe_context(), BIT(m_fp_hpll, 5, 5));
		return m_fp_hpll;
	case DVCO:
		LOG("%s: Oscillator Frequency Center Adjust Read: %04x\n", machine().describe_context(), m_fp_dvco);
		return m_fp_dvco;
	case ADJUST:
		LOG("%s: Oscillator Frequency Center Auto-Adjustment Read: %04x\n", machine().describe_context(), m_fp_dvco);
		return m_fp_dvco;
	case VBEG1:
		LOG("%s: Window #1 Vertical Begin Read: %04x\n", machine().describe_context(), m_fp_vbeg[0]);
		LOG("%s:             Field Line Number: %d\n", machine().describe_context(), BIT(m_fp_vbeg[0], 0, 9));
		LOG("%s:             Sharpness Control: %d%d%d\n", machine().describe_context(), BIT(m_fp_vbeg[0], 11), BIT(m_fp_vbeg[0], 10), BIT(m_fp_vbeg[0], 9));
		return m_fp_vbeg[0];
	case VLINEI1:
	{
		static const char *const FIELD_NAMES[4] = { "Window Disabled", "Window Odd Only", "Window Even Only", "Window Both" };
		LOG("%s: Window #1 Vertical Lines In Read: %04x\n", machine().describe_context(), m_fp_vlinei[0]);
		LOG("%s:            Number of Input Lines: %d\n", machine().describe_context(), BIT(m_fp_vlinei[0], 0, 9));
		LOG("%s:                       Field Flag: %s\n", machine().describe_context(), FIELD_NAMES[BIT(m_fp_vlinei[0], 10, 2)]);
		return m_fp_vlinei[0];
	}
	case VLINEO1:
		LOG("%s: Window #1 Vertical Lines Out Read: %04x (%d)\n", machine().describe_context(), m_fp_vlineo[0], BIT(m_fp_vlineo[0], 0, 9));
		return m_fp_vlineo[0];
	case HBEG1:
		LOG("%s: Window #1 Horizontal Window Start Read: %04x (%d)\n", machine().describe_context(), m_fp_hbeg[0], BIT(m_fp_hbeg[0], 0, 11));
		return m_fp_hbeg[0];
	case HLEN1:
		LOG("%s: Window #1 Horizontal Window Length Read: %04x (%d)\n", machine().describe_context(), m_fp_hlen[0], BIT(m_fp_hlen[0], 0, 11));
		return m_fp_hlen[0];
	case NPIX1:
		LOG("%s: Window #1 Active Pixels Per Line Read: %04x (%d)\n", machine().describe_context(), m_fp_npix[0], BIT(m_fp_npix[0], 0, 11));
		return m_fp_npix[0];
	case VBEG2:
		LOG("%s: Window #2 Vertical Begin Read: %04x\n", machine().describe_context(), m_fp_vbeg[1]);
		LOG("%s:             Field Line Number: %d\n", machine().describe_context(), BIT(m_fp_vbeg[1], 0, 9));
		LOG("%s:             Sharpness Control: %d%d%d\n", machine().describe_context(), BIT(m_fp_vbeg[1], 11), BIT(m_fp_vbeg[1], 10), BIT(m_fp_vbeg[1], 9));
		return m_fp_vbeg[1];
	case VLINEI2:
	{
		static const char *const FIELD_NAMES[4] = { "Window Disabled", "Window Odd Only", "Window Even Only", "Window Both" };
		LOG("%s: Window #2 Vertical Lines In Read: %04x\n", machine().describe_context(), m_fp_vlinei[1]);
		LOG("%s:            Number of Input Lines: %d\n", machine().describe_context(), BIT(m_fp_vlinei[1], 0, 9));
		LOG("%s:                       Field Flag: %s\n", machine().describe_context(), FIELD_NAMES[BIT(m_fp_vlinei[1], 10, 2)]);
		return m_fp_vlinei[1];
	}
	case VLINEO2:
		LOG("%s: Window #2 Vertical Lines Out Read: %04x (%d)\n", machine().describe_context(), m_fp_vlineo[1], BIT(m_fp_vlineo[1], 0, 9));
		return m_fp_vlineo[1];
	case HBEG2:
		LOG("%s: Window #2 Horizontal Window Start Read: %04x (%d)\n", machine().describe_context(), m_fp_hbeg[1], BIT(m_fp_hbeg[1], 0, 11));
		return m_fp_hbeg[1];
	case HLEN2:
		LOG("%s: Window #2 Horizontal Window Length Read: %04x (%d)\n", machine().describe_context(), m_fp_hlen[1], BIT(m_fp_hlen[1], 0, 11));
		return m_fp_hlen[1];
	case NPIX2:
		LOG("%s: Window #2 Active Pixels Per Line Read: %04x (%d)\n", machine().describe_context(), m_fp_npix[1], BIT(m_fp_npix[1], 0, 11));
		return m_fp_npix[1];
	case ACCREF:
		LOG("%s: ACC Reference/Saturation Control Write: %04x\n", machine().describe_context(), m_fp_accref);
		return m_fp_accref;
	case ACCR:
		LOG("%s: ACC Multiplier for SECAM Dr Adjust Read: %04x\n", machine().describe_context(), m_fp_accr);
		return m_fp_accr;
	case ACCB:
		LOG("%s: ACC Multiplier for SECAM Db Adjust Read: %04x\n", machine().describe_context(), m_fp_accb);
		return m_fp_accb;
	case KILVL:
		LOG("%s: Amplitude Color Killer Level Read: %04x\n", machine().describe_context(), m_fp_kilvl);
		return m_fp_kilvl;
	case AGCREF:
		LOG("%s: AGC Sync Amplitude Reference Read: %04x\n", machine().describe_context(), m_fp_agcref);
		return m_fp_agcref;
	case SGAIN:
		LOG("%s: AGC Start Value for Vertical Lock Read: %04x\n", machine().describe_context(), m_fp_sgain);
		return m_fp_sgain;
	case VSDT:
		LOG("%s: Vertical Locking Standard Read: %04x\n", machine().describe_context(), m_fp_vsdt);
		LOG("%s:  Vertical Standard Lock Enable: %d\n", machine().describe_context(), BIT(m_fp_vsdt, 0));
		LOG("%s:       Expected Lines Per Field: %d\n", machine().describe_context(), BIT(m_fp_vsdt, 1, 11));
		return m_fp_vsdt;
	case CMDWD:
		LOG("%s: Control/Latching Register Read: %04x\n", machine().describe_context(), m_fp_cmdwd & 0x100);
		return m_fp_cmdwd & 0x100;
	case INFOWD:
		LOG("%s: Internal Info Word Read: %04x\n", machine().describe_context(), 0x038);
		LOG("%s:             TV Standard: NTSC Comb\n", machine().describe_context());
		LOG("%s:           Line Standard: 525/60\n", machine().describe_context());
		return 0x038;
	case TVSTNDRD:
		LOG("%s: TV Standard Read: %04x\n", machine().describe_context(), 0x7de);
		LOG("%s:                 VACT Suppress: Enabled\n", machine().describe_context());
		LOG("%s:           Recognition Routine: Running\n", machine().describe_context());
		LOG("%s:                   TV Standard: NTSC Comb\n", machine().describe_context());
		LOG("%s:                         Video: Present\n", machine().describe_context());
		LOG("%s:  Video Recognition Confidence: Maximum\n", machine().describe_context());
		LOG("%s:                 Line Standard: 525/60\n", machine().describe_context());
		return 0x7de;
	}
}

void vpx3220a_device::set_fp_reg()
{
	switch (m_fpaddr)
	{
	case TINT:
	{
		static const float VAL2DEG = (1.f / 1024.f) * (180.f / 3.14159265f);
		LOG("%s: NTSC Tint Angle Write: %04x (%d)\n", machine().describe_context(), m_fpdata, (int)(m_fpdata * VAL2DEG));
		m_fp_tint = m_fpdata;
		break;
	}
	case GAIN:
		LOG("%s: AGC Gain Value Write (Read-Only): %04x\n", machine().describe_context(), m_fpdata);
		break;
	case XLG:
		LOG("%s: Line-Locked Mode Command Write: %04x (%s Lock)\n", machine().describe_context(), m_fpdata, m_fpdata == 100 ? "Enable" : "Disable");
		m_fp_xlg = m_fpdata;
		break;
	case HPLL:
		LOG("%s: Horizontal PLL Gain Write: %04x\n", machine().describe_context(), m_fpdata);
		LOG("%s:          Integrating Gain: %02x\n", machine().describe_context(), BIT(m_fpdata, 0, 5));
		LOG("%s:         Proportional Gain: %02x\n", machine().describe_context(), BIT(m_fpdata, 5, 5));
		m_fp_hpll = m_fpdata;
		break;
	case DVCO:
		LOG("%s: Oscillator Frequency Center Adjust Write: %04x\n", machine().describe_context(), m_fpdata);
		m_fp_dvco = m_fpdata;
		break;
	case ADJUST:
		LOG("%s: Oscillator Frequency Center Adjust for Line-Locked Write (Read-Only): %04x\n", machine().describe_context(), m_fpdata);
		break;
	case VBEG1:
		LOG("%s: Window #1 Vertical Begin Write: %04x\n", machine().describe_context(), m_fpdata);
		LOG("%s:              Field Line Number: %d\n", machine().describe_context(), BIT(m_fpdata, 0, 9));
		LOG("%s:              Sharpness Control: %d%d%d\n", machine().describe_context(), BIT(m_fpdata, 11), BIT(m_fpdata, 10), BIT(m_fpdata, 9));
		m_fp_vbeg[0] = m_fpdata;
		break;
	case VLINEI1:
	{
		static const char *const FIELD_NAMES[4] = { "Window Disabled", "Window Odd Only", "Window Even Only", "Window Both" };
		LOG("%s: Window #1 Vertical Lines In Write: %04x\n", machine().describe_context(), m_fpdata);
		LOG("%s:             Number of Input Lines: %d\n", machine().describe_context(), BIT(m_fpdata, 0, 9));
		LOG("%s:                        Field Flag: %s\n", machine().describe_context(), FIELD_NAMES[BIT(m_fpdata, 10, 2)]);
		m_fp_vlinei[0] = m_fpdata;
		break;
	}
	case VLINEO1:
		LOG("%s: Window #1 Vertical Lines Out Write: %04x (%d)\n", machine().describe_context(), m_fpdata, BIT(m_fpdata, 0, 9));
		m_fp_vlineo[0] = m_fpdata;
		break;
	case HBEG1:
		LOG("%s: Window #1 Horizontal Window Start Write: %04x (%d)\n", machine().describe_context(), m_fpdata, BIT(m_fpdata, 0, 11));
		m_fp_hbeg[0] = m_fpdata;
		break;
	case HLEN1:
		LOG("%s: Window #1 Horizontal Window Length Write: %04x (%d)\n", machine().describe_context(), m_fpdata, BIT(m_fpdata, 0, 11));
		m_fp_hlen[0] = m_fpdata;
		break;
	case NPIX1:
		LOG("%s: Window #1 Active Pixels Per Line Write: %04x (%d)\n", machine().describe_context(), m_fpdata, BIT(m_fpdata, 0, 11));
		m_fp_npix[0] = m_fpdata;
		break;
	case VBEG2:
		LOG("%s: Window #2 Vertical Begin Write: %04x\n", machine().describe_context(), m_fpdata);
		LOG("%s:              Field Line Number: %d\n", machine().describe_context(), BIT(m_fpdata, 0, 9));
		LOG("%s:              Sharpness Control: %d%d%d\n", machine().describe_context(), BIT(m_fpdata, 11), BIT(m_fpdata, 10), BIT(m_fpdata, 9));
		m_fp_vbeg[1] = m_fpdata;
		break;
	case VLINEI2: {
		static const char *const FIELD_NAMES[4] = { "Window Disabled", "Window Odd Only", "Window Even Only", "Window Both" };
		LOG("%s: Window #2 Vertical Lines In Write: %04x\n", machine().describe_context(), m_fpdata);
		LOG("%s:             Number of Input Lines: %d\n", machine().describe_context(), BIT(m_fpdata, 0, 9));
		LOG("%s:                        Field Flag: %s\n", machine().describe_context(), FIELD_NAMES[BIT(m_fpdata, 10, 2)]);
		m_fp_vlinei[1] = m_fpdata;
		} break;
	case VLINEO2:
		LOG("%s: Window #2 Vertical Lines Out Write: %04x (%d)\n", machine().describe_context(), m_fpdata, BIT(m_fpdata, 0, 9));
		m_fp_vlineo[1] = m_fpdata;
		break;
	case HBEG2:
		LOG("%s: Window #2 Horizontal Window Start Write: %04x (%d)\n", machine().describe_context(), m_fpdata, BIT(m_fpdata, 0, 11));
		m_fp_hbeg[1] = m_fpdata;
		break;
	case HLEN2:
		LOG("%s: Window #2 Horizontal Window Length Write: %04x (%d)\n", machine().describe_context(), m_fpdata, BIT(m_fpdata, 0, 11));
		m_fp_hlen[1] = m_fpdata;
		break;
	case NPIX2:
		LOG("%s: Window #2 Active Pixels Per Line Write: %04x (%d)\n", machine().describe_context(), m_fpdata, BIT(m_fpdata, 0, 11));
		m_fp_npix[1] = m_fpdata;
		break;
	case ACCREF:
		LOG("%s: ACC Reference/Saturation Control Write: %04x\n", machine().describe_context(), m_fpdata);
		m_fp_accref = m_fpdata;
		break;
	case ACCR:
		LOG("%s: ACC Multiplier for SECAM Dr Adjust Write: %04x\n", machine().describe_context(), m_fpdata);
		m_fp_accr = m_fpdata;
		break;
	case ACCB:
		LOG("%s: ACC Multiplier for SECAM Db Adjust Write: %04x\n", machine().describe_context(), m_fpdata);
		m_fp_accb = m_fpdata;
		break;
	case KILVL:
		LOG("%s: Amplitude Color Killer Level Write: %04x\n", machine().describe_context(), m_fpdata);
		m_fp_kilvl = m_fpdata;
		break;
	case AGCREF:
		LOG("%s: AGC Sync Amplitude Reference Write: %04x\n", machine().describe_context(), m_fpdata);
		m_fp_agcref = m_fpdata;
		break;
	case SGAIN:
		LOG("%s: AGC Start Value for Vertical Lock Write: %04x\n", machine().describe_context(), m_fpdata);
		m_fp_sgain = m_fpdata;
		break;
	case VSDT:
		LOG("%s: Vertical Locking Standard Write: %04x\n", machine().describe_context(), m_fpdata);
		LOG("%s:   Vertical Standard Lock Enable: %d\n", machine().describe_context(), BIT(m_fpdata, 0));
		LOG("%s:        Expected Lines Per Field: %d\n", machine().describe_context(), BIT(m_fpdata, 1, 11));
		m_fp_vsdt = m_fpdata;
		break;
	case CMDWD: {
		static const char *const SYNC_TIMING_NAMES[4] = { "Open", "Forced", "Scan [2]", "Scan [3]" };
		LOG("%s: Control/Latching Register Write: %04x\n", machine().describe_context(), m_fpdata);
		LOG("%s:                  Transport Rate: %s\n", machine().describe_context(), BIT(m_fpdata, 0) ? "13.5 MHz" : "20.25 MHz");
		LOG("%s:            Latch Transport Rate: %d\n", machine().describe_context(), BIT(m_fpdata, 1));
		LOG("%s:                Sync Timing Mode: %s\n", machine().describe_context(), SYNC_TIMING_NAMES[BIT(m_fpdata, 2, 2)]);
		LOG("%s:               Latch Timing Mode: %d\n", machine().describe_context(), BIT(m_fpdata, 4));
		LOG("%s:                 Latch Window #1: %d\n", machine().describe_context(), BIT(m_fpdata, 5));
		LOG("%s:                 Latch Window #2: %d\n", machine().describe_context(), BIT(m_fpdata, 6));
		LOG("%s:                   Odd/Even Mode: %s\n", machine().describe_context(), BIT(m_fpdata, 8) ? "Follow Input" : "Always Toggle");
		const u32 old_cmdwd = m_fp_cmdwd;
		m_fp_cmdwd = m_fpdata & 0x018e;
		if (BIT(old_cmdwd, 2, 2) == 0 && BIT(m_fp_cmdwd, 2, 2) != 0)
		{
			m_href_timer->adjust(attotime::never);
			m_vref_timer->adjust(attotime::never);
		}
		else if (BIT(m_fp_cmdwd, 2, 2) == 0 && (BIT(old_cmdwd, 2, 2) != 0 || m_href_timer->remaining() == attotime::never || m_vref_timer->remaining() == attotime::never))
		{
			m_href_timer->adjust(attotime::from_hz(15734) - attotime::from_usec(52), 0);
			m_vref_timer->adjust(attotime::from_hz(15734) * 480, 0);
		}
		} break;
	case INFOWD:
		LOG("%s: Info Word Write (Read-Only): %04x\n", machine().describe_context(), m_fpdata);
		break;
	case TVSTNDWR:
		static const char *const STANDARD_NAMES[8] = { "PAL B,G,H,I", "NTSC M", "SECAM", "NTSC 44", "PAL M", "PAL N", "PAL 60", "NTSC Comb" };
		LOG("%s: TV Standard Write: %04x\n", machine().describe_context(), m_fpdata);
		LOG("%s:    Selection Mode: %s\n", machine().describe_context(), BIT(m_fpdata, 0) ? "Manual" : "Automatic");
		LOG("%s:       TV Standard: %s\n", machine().describe_context(), STANDARD_NAMES[BIT(m_fpdata, 1, 3)]);
		LOG("%s:    Latch Standard: %d\n", machine().describe_context(), BIT(m_fpdata, 4));
		LOG("%s:       Mode Select: %d\n", machine().describe_context(), BIT(m_fpdata, 5) ? "S-VHS" : "Composite");
		LOG("%s:  Search Threshold: %d%d%d%d\n", machine().describe_context(), BIT(m_fpdata, 9), BIT(m_fpdata, 8), BIT(m_fpdata, 7), BIT(m_fpdata, 6));
		m_fp_tvstndwr = m_fpdata & 0x03ef;
		break;
	case TVSTNDRD:
		LOG("%s: TV Standard Info Write (Read-Only): %04x\n", machine().describe_context(), m_fpdata);
		break;
	}
}
