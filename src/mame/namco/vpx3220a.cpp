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

vpx3220a_device::vpx3220a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VPX3220A, tag, owner, clock)
	, i2c_hle_interface(mconfig, *this, 0x43)
	, m_ifc(0)
	, m_afend(0)
	, m_unk_34(0)
	, m_unk_3a(0)
	, m_refsig(0)
	, m_ymax(0)
	, m_ymin(0)
	, m_umax(0)
	, m_umin(0)
	, m_vmax(0)
	, m_vmin(0)
	, m_cbm_bri(0)
	, m_cbm_con(0)
	, m_format(0)
	, m_misc(0)
	, m_ofifo(0)
	, m_omux(0)
	, m_oena(0)
	, m_driver_a(0)
	, m_driver_b(0)
	, m_fpaddr(0)
	, m_fpsta(0)
	, m_fp_lsb(false)
	, m_fp_tint(0)
	, m_fp_gain(0)
	, m_fp_xlg(0)
	, m_fp_hpll(0)
	, m_fp_dvco(0)
	, m_fp_adjust(0)
	, m_fp_accref(0)
	, m_fp_accr(0)
	, m_fp_accb(0)
	, m_fp_kilvl(0)
	, m_fp_agcref(0)
	, m_fp_sgain(0)
	, m_fp_vsdt(0)
	, m_fp_cmdwd(0)
	, m_fp_infowd(0)
	, m_fp_tvstndwr(0)
	, m_fp_tvstndrd(0)
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

	save_item(NAME(m_fpaddr));
	save_item(NAME(m_fpsta));
	save_item(NAME(m_fp_lsb));

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
	save_item(NAME(m_fp_infowd));
	save_item(NAME(m_fp_tvstndwr));
	save_item(NAME(m_fp_tvstndrd));
}

void vpx3220a_device::device_reset()
{
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
	m_fpaddr = 0;
	m_fpsta = 0;
	m_fp_lsb = false;

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
	m_fp_infowd = 0;
	m_fp_tvstndwr = 979;
	m_fp_tvstndrd = 0;
}

u8 vpx3220a_device::read_data(u16 offset)
{
	switch (offset)
	{
	case FORMAT:
		LOG("%s: Read Format Selection / Alpha Keyer / Contrast Brightness: %04x\n", machine().describe_context(), offset);
		return m_format;

	case FPDAT:
		switch (m_fpaddr)
		{
		default:
			LOG("%s: Unknown Fast Processor Register Read: %04x\n", machine().describe_context(), m_fpaddr);
			return 0;

		case TVSTNDWR: {
			static char const *STANDARD_NAMES[8] =
			{
				"PAL B/G/H/I",
				"NTSC M",
				"SECAM",
				"NTSC 44",
				"PAL M",
				"PAL N",
				"PAL 60",
				"NTSC Comb"
			};
			LOG("%s: TV Standard (Write) Read: %04x\n", machine().describe_context(), m_fp_tvstndwr);
			LOG("%s:     Manual/Automatic:                 %s\n", machine().describe_context(), BIT(m_fp_tvstndwr, 0) ? "Manual" : "Automatic");
			LOG("%s:     TV Standard for Manual Selection: %s\n", machine().describe_context(), STANDARD_NAMES[BIT(m_fp_tvstndwr, 1, 3)]);
			LOG("%s:     Manual TV Standard Latch:         %d\n", machine().describe_context(), BIT(m_fp_tvstndwr, 4));
			LOG("%s:     Composite/S-VHS Select:           %s\n", machine().describe_context(), BIT(m_fp_tvstndwr, 5) ? "S-HVS" : "Composite");
			LOG("%s:     Search-Result Threshold:          %d\n", machine().describe_context(), BIT(m_fp_tvstndwr, 6, 4));
			return m_fp_tvstndwr;
			} break;
		/*case TINT:
		case GAIN:
		case XLG:
		case HPLL:
		case DVCO:
		case ADJUST:
		case VBEG1:
		case VLINEI1:
		case VLINEO1:
		case HBEG1:
		case HLEN1:
		case NPIX1:
		case VBEG2:
		case VLINEI2:
		case VLINEO2:
		case HBEG2:
		case HLEN2:
		case NPIX2:
		case ACCREF:
		case ACCR:
		case ACCB:
		case KILVL:
		case AGCREF:
		case SGAIN:
		case VSDT:
		case CMDWD:
		case INFOWD:
		case TVSTNDWR:
		case TVSTNDRD:*/
		}
		break;

	case FPSTA:
		return m_fpsta;

	default:
		LOG("%s: Read Unknown Sub-Address: %04x (FPAddr %04x)\n", machine().describe_context(), offset, m_fpaddr);
	}
	return 0;
}

void vpx3220a_device::write_data(u16 offset, u8 data)
{
	switch (offset)
	{
	case IFC: {
		static char const *IF_NAMES[4] = { "12 dB", "Reserved", "6 dB", "0 dB" };
		LOG("%s: Write Chroma Processing: %02x\n", machine().describe_context(), data);
		LOG("%s:     IF Compensation: %s\n", machine().describe_context(), IF_NAMES[data & 3]);
		} break;

	case FPRD:
		LOG("%s: Write Fast Processor Register Read: %02x\n", machine().describe_context(), data);
		m_fpaddr <<= 8;
		m_fpaddr |= data;
		m_fpsta = (1 << 1);
		m_fp_lsb = !m_fp_lsb;
		break;

	case FPWR:
		LOG("%s: Write Fast Processor Register Write: %02x\n", machine().describe_context(), data);
		m_fpaddr <<= 8;
		m_fpaddr |= data;
		m_fpsta = (1 << 0);
		m_fp_lsb = !m_fp_lsb;
		break;

	case AFEND: {
		static char const *LUMA_NAMES[4] = { "VIN3", "VIN2", "VIN1", "Reserved" };
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
		static char const *FORMAT_NAMES[8] =
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
		static char const *SHUFFLE_NAMES[8] =
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
		static char const *PORT_MODE_NAMES[4] =
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
		LOG("%s: Write Unknown Sub-Address: %02x\n", machine().describe_context(), data);
		break;
	}
}
