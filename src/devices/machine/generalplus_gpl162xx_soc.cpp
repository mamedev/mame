// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

	GPL16220A - 320x240 output, 16K words internal RAM
	GPL16230A (GPAC500A) - expands RAM to 28K words, adds SDRAM support, adds NAND Flash/ROM/OTP support, USB support
	GPL16240VA - adds 640x480 output (V = VGA support?)
	GPL16250VA (GPAC800A) - adds '3D' sprite mode (see generalplus_gpl1625x_soc.cpp)

	there is also the 'B' series
	these only have 12K words RAM? but have the extra maths unit at 0x79A0?

	GPL16218B
	GPL16238B (this could be GPAC500B)
	GPL16248VB
	GPL16258VB (this could be GPAC800B) (see generalplus_gpl1625x_soc.cpp)

	die is marked 'GCM394' on some chips

	The GPAC500 / GPAC800 might be slightly customized rather than direct rebadgings

**********************************************************************/

#include "emu.h"
#include "generalplus_gpl162xx_soc.h"


#define LOG_GCM394_SPI            (1U << 5)
#define LOG_GCM394_IO             (1U << 4)
#define LOG_GCM394_SYSDMA         (1U << 3)
#define LOG_GCM394                (1U << 2)
#define LOG_GCM394_UNMAPPED       (1U << 1)

#define VERBOSE             (LOG_GCM394 | LOG_GCM394_IO | LOG_GCM394_UNMAPPED | LOG_GCM394_SYSDMA)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(GCM394, sunplus_gcm394_device, "gcm394", "GeneralPlus GPL16250 System-on-a-Chip")

sunplus_gcm394_base_device::sunplus_gcm394_base_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock, address_map_constructor internal) :
	unsp_20_device(mconfig, type, tag, owner, clock, internal),
	device_mixer_interface(mconfig, *this),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_spg_video(*this, "spgvideo"),
	m_spg_audio(*this, "spgaudio"),
	m_internalrom(*this, "internal"),
	m_mainram(*this, "mainram"),
	m_porta_in(*this, 0),
	m_portb_in(*this, 0),
	m_portc_in(*this, 0),
	m_portd_in(*this, 0),
	m_porta_out(*this),
	m_portb_out(*this),
	m_portc_out(*this),
	m_portd_out(*this),
	m_nand_read_cb(*this, 0),
	m_csbase(0x20000),
	m_cs_space(nullptr),
	m_romtype(0),
	m_space_read_cb(*this, 0),
	m_space_write_cb(*this),
	m_dma_complete_cb(*this),
	m_alt_periodic_irq(false),
	m_boot_mode(0),
	m_cs_callback(*this, DEVICE_SELF, FUNC(sunplus_gcm394_base_device::default_cs_callback))
{
}

sunplus_gcm394_device::sunplus_gcm394_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sunplus_gcm394_base_device(mconfig, GCM394, tag, owner, clock)
{
}




void sunplus_gcm394_base_device::default_cs_callback(uint16_t cs0, uint16_t cs1, uint16_t cs2, uint16_t cs3, uint16_t cs4)
{
	logerror("callback not hooked\n");
}


// **************************************** SYSTEM DMA device *************************************************

uint16_t sunplus_gcm394_base_device::read_dma_params(int channel, int offset)
{
	uint16_t retdata = m_dma_params[offset][channel];
	LOGMASKED(LOG_GCM394_SYSDMA, "%s:sunplus_gcm394_base_device::read_dma_params (channel %01x) %01x returning %04x\n", machine().describe_context(), channel, offset, retdata);
	return retdata;
}

void sunplus_gcm394_base_device::write_dma_params(int channel, int offset, uint16_t data)
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


uint16_t sunplus_gcm394_base_device::system_dma_params_channel0_r(offs_t offset)
{
	return read_dma_params(0, offset);
}


void sunplus_gcm394_base_device::system_dma_params_channel0_w(offs_t offset, uint16_t data)
{
	write_dma_params(0, offset, data);
}

uint16_t sunplus_gcm394_base_device::system_dma_params_channel1_r(offs_t offset)
{
	return read_dma_params(1, offset);
}

void sunplus_gcm394_base_device::system_dma_params_channel1_w(offs_t offset, uint16_t data)
{
	write_dma_params(1, offset, data);
}

uint16_t sunplus_gcm394_base_device::system_dma_params_channel2_r(offs_t offset)
{
	return read_dma_params(2, offset);
}

void sunplus_gcm394_base_device::system_dma_params_channel2_w(offs_t offset, uint16_t data)
{
	write_dma_params(2, offset, data);
}

uint16_t sunplus_gcm394_base_device::system_dma_params_channel3_r(offs_t offset)
{
	return read_dma_params(3, offset);
}

void sunplus_gcm394_base_device::system_dma_params_channel3_w(offs_t offset, uint16_t data)
{
	write_dma_params(3, offset, data);
}




uint16_t sunplus_gcm394_base_device::system_dma_status_r()
{
	LOGMASKED(LOG_GCM394_SYSDMA, "%s:sunplus_gcm394_base_device::system_dma_status_r (7abf)\n", machine().describe_context());

	// bit 0 = channel 0 ready
	// bit 1 = channel 1 ready

	return 0x00ff;
}

void sunplus_gcm394_base_device::trigger_systemm_dma(int channel)
{
	uint16_t mode = m_dma_params[0][channel];
	uint32_t source = m_dma_params[1][channel] | (m_dma_params[4][channel] << 16);
	uint32_t dest = m_dma_params[2][channel] | (m_dma_params[5][channel] << 16) ;
	uint32_t length = m_dma_params[3][channel] | (m_dma_params[6][channel] << 16);
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

	LOGMASKED(LOG_GCM394_SYSDMA, "%s:possible DMA operation with params mode:%04x source:%08x (word offset) dest:%08x (word offset) length:%08x (words) while csbank is %02x\n", machine().describe_context(), mode, source, dest, length, m_membankswitch_7810 );

	// wrlshunt transfers ROM to RAM, all RAM write addresses have 0x800000 in the destination set

	source &= 0x0fffffff;
	length &= 0x0fffffff; // gormiti

	for (int i = 0; i < length; i++)
	{
		uint16_t val;
		if (mode & 0x1000)
		{
			val = (read_space(source) & 0xFF) | (read_space(source) << 8);
			i++;
		}
		else
		{
			val = read_space(source);
		}

		source += sourcedelta;

		if (mode & 0x2000)
		{
			write_space(dest, val & 0xFF);
			dest += destdelta;
			write_space(dest, val >> 8);
		}
		else
		{
			write_space(dest, val);
		}

		dest += destdelta;
	}

	m_dma_complete_cb(1); // allow some driver specific hacks for service modes

	// clear params after operation
	m_dma_params[0][channel] = m_dma_params[0][channel] & 0x00f7;

	m_dma_params[1][channel] = m_dma_params[2][channel] = m_dma_params[3][channel] = m_dma_params[4][channel] = m_dma_params[5][channel] = m_dma_params[6][channel] = 0x0000;
	m_dma_latched[channel] = false;

	//machine().debug_break();
}

void sunplus_gcm394_base_device::system_dma_7abf_unk_w(uint16_t data)
{
	// if this isn't trigger, is it enable? (which could maybe used as similar if DMA only starts to happen if unmasked?)

	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::system_dma_7abf_unk_w %04x\n", machine().describe_context(), data);
}

uint16_t sunplus_gcm394_base_device::system_dma_memtype_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::system_dma_memtype_r\n", machine().describe_context());
	return m_system_dma_memtype;
}

void sunplus_gcm394_base_device::system_dma_memtype_w(uint16_t data)
{
	static char const* const types[16] =
	{
		"Unused / USB",
		"DAC CHA",
		"UART TX",
		"UART RX",
		"SD/MMC",
		"NAND Flash",
		"Serial Interface",
		"DAC CHB",
		"ADC Audo Sample Full",
		"SPI TX",
		"SPI RX",
		"RESERVED (c)",
		"RESERVED (d)",
		"RESERVED (e)",
		"RESERVED (f)"
	};

	m_system_dma_memtype = data;

	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::system_dma_memtype_w %04x (CH3: %s | CH2: %s | CH1: %s | CH0: %s )\n", machine().describe_context(), data,
		types[((m_system_dma_memtype>>12)&0xf)],
		types[((m_system_dma_memtype>>8)&0xf)],
		types[((m_system_dma_memtype>>4)&0xf)],
		types[((m_system_dma_memtype)&0xf)]);

}

uint16_t sunplus_gcm394_base_device::system_7a35_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::system_7a35_r\n", machine().describe_context());
	return machine().rand();
}

uint16_t sunplus_gcm394_base_device::system_7a37_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::system_7a37_r\n", machine().describe_context());
	return machine().rand();
}

uint16_t sunplus_gcm394_base_device::system_7a39_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::system_7a39_r\n", machine().describe_context());
	return machine().rand();
}


uint16_t sunplus_gcm394_base_device::system_7a3a_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::system_7a3a_r\n", machine().describe_context());
	return machine().rand();
}

uint16_t sunplus_gcm394_base_device::system_7a46_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::system_7a46_r\n", machine().describe_context());
	return machine().rand();
}

uint16_t sunplus_gcm394_base_device::system_7a54_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::system_7a54_r\n", machine().describe_context());
	return machine().rand();
}

// **************************************** 78xx region with some handling *************************************************

uint16_t sunplus_gcm394_base_device::unkarea_780f_status_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_780f_status_r\n", machine().describe_context());
	return 0x0002;
}

uint16_t sunplus_gcm394_base_device::unkarea_78fb_status_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78fb_status_r\n", machine().describe_context());
	m_78fb ^= 0x0100; // status flag for something?
	return m_78fb;
}

// sets bit 0x0002 then expects it to have cleared
uint16_t sunplus_gcm394_base_device::unkarea_7819_r() { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7819_r\n", machine().describe_context()); return m_7819 & ~ 0x0002; }
void sunplus_gcm394_base_device::unkarea_7819_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7819_w %04x\n", machine().describe_context(), data); m_7819 = data; }

// ****************************************  78xx region stubs *************************************************

uint16_t sunplus_gcm394_base_device::unkarea_782d_r() { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_782d_r\n", machine().describe_context()); return m_782d; }
void sunplus_gcm394_base_device::unkarea_782d_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_782d_w %04x\n", machine().describe_context(), data); m_782d = data; }

uint16_t sunplus_gcm394_base_device::unkarea_7803_r() { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7803_r\n", machine().describe_context()); return m_7803; }
void sunplus_gcm394_base_device::unkarea_7803_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7803_w %04x\n", machine().describe_context(), data); m_7803 = data; }

void sunplus_gcm394_base_device::unkarea_7807_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7807_w %04x\n", machine().describe_context(), data); m_7807 = data; }

void sunplus_gcm394_base_device::waitmode_enter_780c_w(uint16_t data)
{
	// LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::waitmode_enter_780c_w %04x\n", machine().describe_context(), data);
	// must be followed by 6 nops to ensure wait mode is entered
}

// this gets stored / modified / restored before certain memory accesses (
// used extensively during SDRAM checks in jak_gtg and jak_car2

uint16_t sunplus_gcm394_base_device::membankswitch_7810_r()
{
//  LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::membankswitch_7810_r\n", machine().describe_context());
	return m_membankswitch_7810;
}

void sunplus_gcm394_base_device::membankswitch_7810_w(uint16_t data)
{
//  if (m_membankswitch_7810 != data)
//  LOGMASKED(LOG_GCM394,"%s:sunplus_gcm394_base_device::membankswitch_7810_w %04x\n", machine().describe_context(), data);

//  if (m_membankswitch_7810 != data)
//      popmessage("bankswitch %04x -> %04x", m_membankswitch_7810, data);

	m_membankswitch_7810 = data;
}


void sunplus_gcm394_base_device::unkarea_7816_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7816_w %04x\n", machine().describe_context(), data);
	m_7816 = data;
}

void sunplus_gcm394_base_device::unkarea_7817_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7817_w %04x\n", machine().describe_context(), data);
	m_7817 = data;
}

void sunplus_gcm394_base_device::chipselect_csx_memory_device_control_w(offs_t offset, uint16_t data)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::chipselect_csx_memory_device_control_w %04x (782x registers offset %d)\n", machine().describe_context(), data, offset);
	m_782x[offset] = data;


	static const char* const md[] =
	{
		"(00) ROM / SRAM",
		"(01) ROM / SRAM",
		"(10) NOR FLASH",
		"(11) NAND FLASH",
	};

	uint8_t cs_wait =  data & 0x000f;
	uint8_t cs_warat = (data & 0x0030)>>4;
	uint8_t cs_md    = (data & 0x00c0)>>6;
	int cs_size  = (data & 0xff00)>>8;

	logerror("CS%d set to size: %02x (%08x words) md: %01x %s   warat: %01x wait: %01x\n", offset, cs_size, (cs_size+1)*0x10000, cs_md, md[cs_md], cs_warat, cs_wait);

	m_cs_callback(m_782x[0], m_782x[1], m_782x[2], m_782x[3], m_782x[4]);

}

void sunplus_gcm394_base_device::device_post_load()
{
	m_cs_callback(m_782x[0], m_782x[1], m_782x[2], m_782x[3], m_782x[4]);
}

void sunplus_gcm394_base_device::unkarea_7835_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7835_w %04x\n", machine().describe_context(), data); m_7835 = data; }

// IO here?

// Port A

uint16_t sunplus_gcm394_base_device::ioarea_7860_porta_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7860_porta_r\n", machine().describe_context());
	return m_porta_in();
}

void sunplus_gcm394_base_device::ioarea_7860_porta_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7860_porta_w %04x\n", machine().describe_context(), data);
	m_porta_out(data);
}

uint16_t sunplus_gcm394_base_device::ioarea_7861_porta_buffer_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7861_porta_buffer_r\n", machine().describe_context());
	return 0x0000;// 0xffff;// m_7861;
}

void sunplus_gcm394_base_device::ioarea_7861_porta_buffer_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7861_porta_buffer_w %04x\n", machine().describe_context(), data);
}

uint16_t sunplus_gcm394_base_device::ioarea_7862_porta_direction_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7862_porta_direction_r\n", machine().describe_context());
	return m_7862_porta_direction;
}

void sunplus_gcm394_base_device::ioarea_7862_porta_direction_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7862_porta_direction_w %04x\n", machine().describe_context(), data);
	m_7862_porta_direction = data;
}

uint16_t sunplus_gcm394_base_device::ioarea_7863_porta_attribute_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7863_porta_attribute_r\n", machine().describe_context());
	return m_7863_porta_attribute;
}

void sunplus_gcm394_base_device::ioarea_7863_porta_attribute_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7863_porta_attribute_w %04x\n", machine().describe_context(), data);
	m_7863_porta_attribute = data;
}

// Port B

uint16_t sunplus_gcm394_base_device::ioarea_7868_portb_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7868_portb_r\n", machine().describe_context());
	return m_portb_in();
}

uint16_t sunplus_gcm394_base_device::ioarea_7869_portb_buffer_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7869_portb_buffer_r\n", machine().describe_context());
	return machine().rand();
}

void sunplus_gcm394_base_device::ioarea_7869_portb_buffer_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7869_portb_buffer_w %04x\n", machine().describe_context(), data);
	m_portb_out(data); // buffer writes must update output state too, beijuehh requires it for banking
}

void sunplus_gcm394_base_device::ioarea_7868_portb_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7868_portb_w %04x\n", machine().describe_context(), data);
	m_portb_out(data);
}

uint16_t sunplus_gcm394_base_device::ioarea_786a_portb_direction_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_786a_portb_direction_r\n", machine().describe_context());
	return m_786a_portb_direction;
}

void sunplus_gcm394_base_device::ioarea_786a_portb_direction_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_786a_portb_direction_w %04x\n", machine().describe_context(), data);
	m_786a_portb_direction = data;
}

uint16_t sunplus_gcm394_base_device::ioarea_786b_portb_attribute_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_786b_portb_attribute_r\n", machine().describe_context());
	return m_786b_portb_attribute;
}

void sunplus_gcm394_base_device::ioarea_786b_portb_attribute_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_786b_portb_attribute_w %04x\n", machine().describe_context(), data);
	m_786b_portb_attribute = data;
}

// Port C

uint16_t sunplus_gcm394_base_device::ioarea_7870_portc_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7870_portc_r\n", machine().describe_context());
	return m_portc_in();
}

void sunplus_gcm394_base_device::ioarea_7870_portc_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7870_portc_w %04x\n", machine().describe_context(), data);
	m_7870 = data;
	m_portc_out(data);
}

uint16_t sunplus_gcm394_base_device::ioarea_7871_portc_buffer_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7871_portc_buffer_r\n", machine().describe_context());
	return 0xffff;// m_7871;
}

void sunplus_gcm394_base_device::ioarea_7871_portc_buffer_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7871_portc_buffer_w %04x\n", machine().describe_context(), data);
}


uint16_t sunplus_gcm394_base_device::ioarea_7872_portc_direction_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7872_portc_direction_r\n", machine().describe_context());
	return m_7872_portc_direction;
}

void sunplus_gcm394_base_device::ioarea_7872_portc_direction_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7872_portc_direction_w %04x\n", machine().describe_context(), data);
	m_7872_portc_direction = data;
}

uint16_t sunplus_gcm394_base_device::ioarea_7873_portc_attribute_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7873_portc_attribute_r\n", machine().describe_context());
	return m_7873_portc_attribute;
}

void sunplus_gcm394_base_device::ioarea_7873_portc_attribute_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7873_portc_attribute_w %04x\n", machine().describe_context(), data);
	m_7873_portc_attribute = data;
}

// Port D

uint16_t sunplus_gcm394_base_device::ioarea_7878_portd_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7878_portd_r\n", machine().describe_context());
	return m_portd_in();
}

void sunplus_gcm394_base_device::ioarea_7878_portd_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7878_portd_w %04x\n", machine().describe_context(), data);
	//m_7878 = data;
	m_portd_out(data);
}

uint16_t sunplus_gcm394_base_device::ioarea_7879_portd_buffer_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7879_portd_buffer_r\n", machine().describe_context());
	return 0xffff;// m_7871;
}

void sunplus_gcm394_base_device::ioarea_7879_portd_buffer_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_7879_portd_buffer_w %04x\n", machine().describe_context(), data);
	m_portd_out(data); // buffer writes must update output state too, beijuehh requires it for banking
}


uint16_t sunplus_gcm394_base_device::ioarea_787a_portd_direction_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_787a_portd_direction_r\n", machine().describe_context());
	return m_787a_portd_direction;
}

void sunplus_gcm394_base_device::ioarea_787a_portd_direction_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_787a_portd_direction_w %04x\n", machine().describe_context(), data);
	m_787a_portd_direction = data;
}

uint16_t sunplus_gcm394_base_device::ioarea_787b_portd_attribute_r()
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_787b_portd_attribute_r\n", machine().describe_context());
	return m_787b_portd_attribute;
}

void sunplus_gcm394_base_device::ioarea_787b_portd_attribute_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_IO, "%s:sunplus_gcm394_base_device::ioarea_787b_portd_attribute_w %04x\n", machine().describe_context(), data);
	m_787b_portd_attribute = data;
}

uint16_t sunplus_gcm394_base_device::unkarea_7882_r() { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7882_r\n", machine().describe_context()); return 0xffff;// m_7882;
}
void sunplus_gcm394_base_device::unkarea_7882_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7882_w %04x\n", machine().describe_context(), data); m_7882 = data; }
uint16_t sunplus_gcm394_base_device::unkarea_7883_r() { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7883_r\n", machine().describe_context()); return 0xffff;// m_7883;
}
void sunplus_gcm394_base_device::unkarea_7883_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7883_w %04x\n", machine().describe_context(), data); m_7883 = data; }

void sunplus_gcm394_base_device::unkarea_78a0_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a0_w %04x\n", machine().describe_context(), data); m_78a0 = data; }

uint16_t sunplus_gcm394_base_device::unkarea_78a0_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a0_r\n", machine().describe_context());
	return 0x0000;// machine().rand();
}

uint16_t sunplus_gcm394_base_device::unkarea_78a1_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a1_r\n", machine().describe_context());
	return 0xffff;// machine().rand();
}

void sunplus_gcm394_base_device::unkarea_78a4_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a4_w %04x\n", machine().describe_context(), data); m_78a4 = data; }
void sunplus_gcm394_base_device::unkarea_78a5_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a5_w %04x\n", machine().describe_context(), data); m_78a5 = data; }
void sunplus_gcm394_base_device::unkarea_78a6_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a6_w %04x\n", machine().describe_context(), data); m_78a6 = data; }

void sunplus_gcm394_base_device::unkarea_78a8_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a8_w %04x\n", machine().describe_context(), data); m_78a8 = data; }


void sunplus_gcm394_base_device::unkarea_78b0_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78b0_w %04x\n", machine().describe_context(), data); m_78b0 = data; }
void sunplus_gcm394_base_device::unkarea_78b1_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78b1_w %04x\n", machine().describe_context(), data); m_78b1 = data; }

uint16_t sunplus_gcm394_base_device::unkarea_78b2_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78b2_r\n", machine().describe_context());
	return 0xffff;// m_78b2;
}

void sunplus_gcm394_base_device::unkarea_78b2_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78b2_w %04x\n", machine().describe_context(), data); m_78b2 = data; }

void sunplus_gcm394_base_device::unkarea_78b8_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78b8_w %04x\n", machine().describe_context(), data); m_78b8 = data; }


uint16_t sunplus_gcm394_base_device::unkarea_78f0_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78f0_r\n", machine().describe_context());
	return 0xffff;
}

void sunplus_gcm394_base_device::unkarea_78f0_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78f0_w %04x\n", machine().describe_context(), data); m_78f0 = data; }

uint16_t sunplus_gcm394_base_device::unkarea_78c0_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78c0_r\n", machine().describe_context());
	return machine().rand();
}

uint16_t sunplus_gcm394_base_device::unkarea_78c8_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78c8_r\n", machine().describe_context());
	return 0xffff;
}

uint16_t sunplus_gcm394_base_device::unkarea_78d0_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78d0_r\n", machine().describe_context());
	return machine().rand();
}

uint16_t sunplus_gcm394_base_device::unkarea_78d8_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78d8_r\n", machine().describe_context());
	return machine().rand();
}

// **************************************** 793x uknown region stubs *************************************************

uint16_t sunplus_gcm394_base_device::unkarea_7904_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7904_r\n", machine().describe_context());
	return machine().rand(); // lazertag waits on a bit, status flag for something?

}

uint16_t sunplus_gcm394_base_device::unkarea_7934_r()
{
	// does this return data written, or is it a status flag?
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7934_r\n", machine().describe_context());
	return m_7934;
}

void sunplus_gcm394_base_device::unkarea_7934_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7934_w %04x\n", machine().describe_context(), data);
	m_7934 = data;
}

// value of 7935 is read then written in irq6, nothing happens unless bit 0x0100 was set, which could be some kind of irq source being acked?
uint16_t sunplus_gcm394_base_device::unkarea_7935_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7935_r\n", machine().describe_context());
	return m_7935;
}

void sunplus_gcm394_base_device::unkarea_7935_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7935_w %04x\n", machine().describe_context(), data);
	m_7935 &= ~data;
	//checkirq6();
}

uint16_t sunplus_gcm394_base_device::unkarea_7936_r() { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7936_r\n", machine().describe_context()); return 0x0000; }
void sunplus_gcm394_base_device::unkarea_7936_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7936_w %04x\n", machine().describe_context(), data); m_7936 = data; }

// **************************************** 794x SPI *************************************************

// these are related to the accelerometer values on jak_g500 (8-bit signed) and also the SPI reads for bkrankp
uint16_t sunplus_gcm394_base_device::spi_7944_rxdata_r()
{
	LOGMASKED(LOG_GCM394_SPI, "%s:sunplus_gcm394_base_device::spi_7944_rxdata_r\n", machine().describe_context());
	return machine().rand();
}

uint16_t sunplus_gcm394_base_device::spi_7945_misc_control_reg_r()
{
	LOGMASKED(LOG_GCM394_SPI, "%s:sunplus_gcm394_base_device::spi_7945_misc_control_reg_r\n", machine().describe_context());
	return machine().rand();// &0x0007;
}

void sunplus_gcm394_base_device::spi_7942_txdata_w(uint16_t data)
{
	LOGMASKED(LOG_GCM394_SPI, "%s:sunplus_gcm394_base_device::spi_7942_txdata_w %04x\n", machine().describe_context(), data);
}

// **************************************** 796x unknown *************************************************

void sunplus_gcm394_base_device::unkarea_7960_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7960_w %04x\n", machine().describe_context(), data); m_7960 = data; }

// 7961 and 7962 are used by dressmtv when detecting battery status, adc?
uint16_t sunplus_gcm394_base_device::unkarea_7961_r() { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7961_r\n", machine().describe_context()); return 0xffff; /* return m_7961; */ }
void sunplus_gcm394_base_device::unkarea_7961_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7961_w %04x\n", machine().describe_context(), data); m_7961 = data; }

uint16_t sunplus_gcm394_base_device::unkarea_7962_r() { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7962_r\n", machine().describe_context()); return 0xffff; }

// **************************************** fallthrough logger etc. *************************************************

uint16_t sunplus_gcm394_base_device::unk_r(offs_t offset)
{
	switch (offset)
	{
	default:
		LOGMASKED(LOG_GCM394_UNMAPPED, "%s:sunplus_gcm394_base_device::unk_r @ 0x%04x\n", machine().describe_context(), offset + 0x7000);
		return 0x0000;
	}

	return 0x0000;
}

void sunplus_gcm394_base_device::unk_w(offs_t offset, uint16_t data)
{

	switch (offset)
	{
	default:
		LOGMASKED(LOG_GCM394_UNMAPPED, "%s:sunplus_gcm394_base_device::unk_w @ 0x%04x (data 0x%04x)\n", machine().describe_context(), offset + 0x7000, data);
		break;
	}
}

// GPAC500B register list

// 7000 - Tx3_X_Position
// 7001 - Tx3_Y_Position
// 7002 - Tx3_X_Offset
// 7003 - Tx3_Y_Offset
// 7004 - Tx3_Attribute
// 7005 - Tx3_Control
// 7006 - Tx3_N_PTR
// 7007 - Tx3_A_PTR
// 7008 - Tx4_X_Position
// 7009 - Tx4_Y_Position
// 700a - Tx4_X_Offset
// 700b - Tx4_Y_Offset
// 700c - Tx4_Attribute
// 700d - Tx4_Control
// 700e - Tx4_N_PTR
// 700f - Tx4_A_PTR
// 7010 - Tx1_X_Position
// 7011 - Tx1_Y_Position
// 7012 - Tx1_Attribute
// 7013 - Tx1_Control
// 7014 - Tx1_N_PTR
// 7015 - Tx1_A_PTR
// 7016 - Tx2_X_Position
// 7017 - Tx2_Y_Position
// 7018 - Tx2_Attribute
// 7019 - Tx2_Control 
// 701a - Tx2_N_PTR
// 701b - Tx2_A_PTR
// 701c - VComValue
// 701d - VComOffset
// 701e - VComStep
//
// 7020 - Segment_Tx1
// 7021 - Segment_Tx2
// 7022 - Segment_sp
// 7023 - Segment_Tx3
// 7024 - Segment_Tx4
//
// 7028 - Tx4_Cosine
// 7029 - Tx4_Sine 
// 702a - Blending
// 702b - Segment_Tx1H
// 702c - Segment_Tx2H
// 702d - Segment_spH
// 702e - Segment_Tx3H
// 702f - Segment_Tx4H
// 7030 - Fade_Control
//
// 7036 - IRQTMV
// 7037 - IRQTMH
// 7038 - Line_Counter
// 7039 - LightPen_Control 
// 703a - Palette_Control
//
// 703c - TV_Control
//
// 703e - LPHPosition
// 703f - LPVPosition
//
// 7042 - SControl
//
// 7050 - TFT_Ctrl    (bit 0 is TFT enable, which presumable changes the mappings at 7062/7063 from PPU to TFT)
// 7051 - TFT_V_Width       or STN_COM_Clip
// 7052 - TFT_VSync_Setup
// 7053 - TFT_V_Start
// 7054 - TFT_V_End
// 7055 - TFT_H_Width
// 7056 - TFT_HSync_Setup
// 7057 - TFT_H_ Start
// 7058 - TFT_H_ End
// 7059 - TFT_RGB_Ctrl
// 705a - TFT_Status
// 705b - TFT_MemMode_WriteCMD
// 705c - TFT_MemMode_ReadCMD 
//
// 705f - STN_Ctrl1   (bit 0 is STN enable, which presumably changes the mappings at 708x and 7051 from TV/TFT to STN regs)
//
// 7062 - PPU_IRQ_EN     or TFT_INT_EN
// 7063 - PPU_IRQ_Status or TFT_INT_CLR
//
// 706c - TFT_V_Show_Start
// 706d - TFT_V_Show_End
// 706e - TFT_H_Show_Start
// 706f - TFT_H_Show_End
//
// 7070 - SPDMA_Source
// 7071 - SPDMA_Target
// 7072 - SPDMA_Number 
// 7073 - HB_Ctrl
// 7074 - HB_GO
//
// 7078 - FBI_Addr
// 7079 - FBI_AddrH
// 707a - FBO_AddrL
// 707b - FBO_AddrH
// 707c - FB_PPU_GO
// 707d - BLD_Color
// 707e - PPU_RAM_Bank
// 707f - PPU_Enable
//
// 7080 - TV_Saturation  or STN_SEG
// 7081 - TV_Hue         or STN_COM  
// 7082 - TV_Brightness  or STN_PIC_COM 
// 7083 - TV_Sharpness   or STN_CPWAIT    
// 7084 - TV_Y_Gain      or STN_Ctrl2     
// 7085 - TV_Y_Delay     or STN_GTG_SEG  
// 7086 - TV_V_Position  or STN_GTG_COM  
// 7087 - TV_H_Position  or STN_SEG_Clip 
// 7088 - TV_VideoDAC
// 
// 70b0 - Tx1_AttributeH
// 70b1 - Tx2_AttributeH
// 70b2 - Tx3_AttributeH
// 70b3 - Tx4_AttributeH
// 70b4 - Tx1_N_PTRH
// 70b5 - Tx1_A_PTRH
// 70b6 - Tx2_N_PTRH
// 70b7 - Tx2_A_PTRH
// 70b8 - Tx3_N_PTRH
// 70b9 - Tx3_A_PTRH
// 70ba - Tx4_N_PTRH
// 70bb - Tx4_A_PTRH
// 
// 70d8 - BLD_Enable 
//
// 70e0 - Random0
// 70e1 - Random1
//
// 7100 to 71ff - Tx_Hvoffset
// 7200 to 72ff - HCMValue
// 7300 to 73ff - Palette0 / Palette1 / Palette2 / Palette3 (4 banks)
// 7400 to 77ff - Spriteram (2 banks) (one bank is 7400-77ff, other bank is 7400-74ff)
//
// 7800 - BodyID
//
// 7803 - SYS_CTRL
// 7804 - CLK_Ctrl0
// 7805 - CLK_Ctrl1
// 7806 - Reset_Flag
// 7807 - Clock_Ctrl
// 7808 - LVR_Ctrl
// 780a - Watchdog_Ctrl
// 780b - Watchdog_Clear
// 780c - WAIT
// 780d - HALT
// 780e - SLEEP
// 780f - Power_State
//
// 7810 - BankSwitch_Ctrl 
//
// 7817 - PLLChange
// 7818 - PLLCLKWait
// 7819 - Cache_Ctrl
// 781a - Cache_HitRate
//
// 781f - IO_SR_SMT
//
// 7820 - MCS0_Ctrl
// 7821 - MCS1_Ctrl
// 7822 - MCS2_Ctrl 
// 7823 - MCS3_Ctrl
// 7824 - MCS4_Ctrl
// 7825 - PSRAM_Ctrl
// 7826 - MCS_BYTE_SEL
// 7827 - MCS3_WETimingCtrl
// 7828 - MCS4_WETimingCtrl
// 7829 - MCS3_RDTimingCtrl
// 782a - MCS4_RDTimingCtrl
// 782b - MCS3_TimingCtrl
// 782c - MCS4_TimingCtrl
// 782d - RAW_WAR
// 782e - NOR_WHold
//
// 7835 - MCS0_Page
// 7836 - MCS1_Page
// 7837 - MCS2_Page
// 7838 - MCS3_Page 
// 7839 - MCS4_Page
//
// 7840 - Mem_Ctrl
// 7841 - Addr_Ctrl
//
// 787e - MCS_Drv
// 787f - MCS_Dly
//
// 7888 - MEM_Drv
// 7889 - MEM_Dly0
// 788a - MEM_Dly1
// 788b - MEM_Dly2
// 788c - MEM_Dly3
// 788d - MEM_Dly4
// 788e - MEM_Dly5
// 788f - MEM_Dly6
//
// 78a0 - INT_Status1
// 78a1 - INT_Status2
//
// 78a3 - INT_Status3
// 78a4 - INT_Priority1
// 78a5 - INT_Priority2
// 78a6 - INT_Priority3
//
// 78a8 - MINT_Ctrl
//
// 78b0 - TimeBaseA_Ctrl
// 78b1 - TimeBaseB_Ctrl
// 78b2 - TimeBaseC_Ctrl
// 
// 78b8 - TimeBase_Reset
//
// 78c0 - TimerA_Ctrl
// 78c1 - TimerA_CCCtrl
// 78c2 - TimerA_Preload
// 78c3 - TimerA_CCReg
// 78c4 - TimerA_UpCount
//
// 78c8 - TimerB_Ctrl
// 78c9 - TimerB_CCCtrl
// 78ca - TimerB_Preload
// 78cb - TimerB_CCReg
// 78cc - TimerB_UpCount
//
// 78d0 - TimerC_Ctrl
// 78d1 - TimerC_CCCtrl
// 78d2 - TimerC_Preload
// 78d3 - TimerC_CCReg
// 78d4 - TimerC_UpCount
//
// 78d8 - TimerD_Ctrl
// 78da - TimerD_Preload
// 78dc - TimerD_UpCount
//
// 78e0 - TimerE_Ctrl
// 78e2 - TimerE_Preload
// 78e4 - TimerE_UpCount
// 
// 78e8 - TimerF_Ctrl
// 78ea - TimerF_Preload
// 78ec - TimerF_UpCount 
//
// 78f0 - CHA_Ctrl
// 78f1 - CHA_Data
// 78f2 - CHA_FIFO
//
// 78f8 - CHB_Ctrl
// 78f9 - CHB_Data
// 78fa - CHB_FIFO
// 78fb - DAC_PGA
//
// 78ff - IISEN
//
// 7920 - Second
// 7921 - Minute
// 7922 - Hour
// 7924 - Alarm_Second
// 7925 - Alarm_Minute
// 7926 - Alarm_Hour
//
// 7934 - RTC_Ctrl
// 7935 - RTC_INT_Status
// 7936 - RTC_INT_Ctrl
// 7937 - RTC_Busy 
//
// 7940 - SPI_Ctrl
// 7941 - SPI_TXStatus
// 7942 - SPI_TXData
// 7943 - SPI_RXStatus
// 7944 - SPI_RXData
// 7945 - SPI_Misc
//
// 79a0 - DividendH
// 79a1 - Dividend
// 79a2 - DivisorH
// 79a3 - Divisor
// 79a4 - QuotientH
// 79a5 - Quotient
// 79a6 - RemainderH
// 79a7 - Remainder
// 79a8 - Divider_Status 
//
// 79e0 - SD2_DataTX
// 79e1 - SD2_DataRX
// 79e2 - SD2_CMD
// 79e3 - SD2_ArgL
// 79e4 - SD2_ArgH
// 79e5 - SD2_RespL
// 79e6 - SD2_RespH
// 79e7 - SD2_Status
// 79e8 - SD2_Ctrl
// 79e9 - SD2_BLKLEN
// 79ea - SD2_INT
//
// 7a80 - DMA_Ctrl0
// 7a81 - DMA_SRC_AddrL0
// 7a82 - DMA_TAR_AddrL0
// 7a83 - DMA_TCountL0
// 7a84 - DMA_SRC_AddrH0
// 7a85 - DMA_TAR_AddrH0
// 7a86 - DMA_TCountH0
// 7a87 - DMA_MISC0
// 7a88 - DMA_Ctrl1
// 7a89 - DMA_SRC_AddrL1
// 7a8a - DMA_TAR_AddrL1
// 7a8b - DMA_TCountL1
// 7a8c - DMA_SRC_AddrH1
// 7a8d - DMA_TAR_AddrH1
// 7a8e - DMA_TCountH1
// 7a8f - DMA_MISC1
//
// 7ab0 - DMA_SPRISIZE0
// 7ab1 - DMA_SPRISIZE1 
//
// 7abd - DMA_LineLength
// 7abe - DMA_SS
// 7abf - DMA_INT
// 
// 7af0 - Byte_Swap
// 7af1 - Nibble_Swap
// 7af2 - TwoBit_Swap
// 7af3 - Bit_Reverse
//
// (Sound)
// 7b80 - Channel Enable
// 7b81 - Main volume
// 7b82 - Channel FIQ Enable
// 7b83 - Channel FIQ Status
// 7b84 - Beat base counter
// 7b85 - Beat counter
// 7b86 - Envelope interval select (Ch 3-0)
// 7b87 - Envelope interval select (Ch 7-4)
// 7b88 - Envelope interval select (Ch 11-8)
// 7b89 - Envelope interval select (Ch 15-12)
// 7b8a - Envelope fast ramp down
// 7b8b - Stop channel status
//
// 7b8d - Control Flags
// 7b8e - Compressor Control
// 7b8f - Channel status
// 7b90 - Left channel mixer input
// 7b91 - Right channel mixer input
// 7b92 - Left channel mixer output
// 7b93 - Right channel mixer output
// 7b94 - Channel Repeat Enable control
// 7b95 - Channel Env Mode
// 7b96 - Channel Tone Release Control
// 7b97 - Channel Env Irq Status
// 7b98 - Channel Pitch Bend Enable
//
// 7b9a - Attack/Release Time Control
//
// 7b9f - Wave Table Bank Address
// 
// 7ba5 - SPU_CtrPW
// 
// 7bb2 - SPU_CtrPWaveOutL 
// 7bb3 - SPU_CtrPWaveOutR
//
// 7bb6 - SPU_CtrChToneRelease_H
//
// 7c00 - 7cff Sound Attribute
// 7e00 - 7eff Sound Phase


void sunplus_gcm394_base_device::base_internal_map(address_map &map)
{
	map(0x000000, 0x006fff).ram().share("mainram");
	map(0x007000, 0x007fff).rw(FUNC(sunplus_gcm394_base_device::unk_r), FUNC(sunplus_gcm394_base_device::unk_w)); // catch unhandled

	// ######################################################################################################################################################################################
	// 70xx region = video hardware
	// ######################################################################################################################################################################################

	// note, tilemaps are at the same address offsets in video device as spg2xx (but unknown devices are extra)

	map(0x007000, 0x007007).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap2_regs_r), FUNC(gcm394_base_video_device::tmap2_regs_w)); // written with other unknown_video_device1 LSB/MSB regs below (roz layer or line layer?)
	map(0x007008, 0x00700f).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap3_regs_r), FUNC(gcm394_base_video_device::tmap3_regs_w)); // written with other unknown_video_device2 LSB/MSB regs below (roz layer or line layer?)

	map(0x007010, 0x007015).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap0_regs_r), FUNC(gcm394_base_video_device::tmap0_regs_w));
	map(0x007016, 0x00701b).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap1_regs_r), FUNC(gcm394_base_video_device::tmap1_regs_w));

	map(0x00701c, 0x00701c).w(m_spg_video, FUNC(gcm394_base_video_device::video_701c_w)); // these 3 are written together in paccon
	map(0x00701d, 0x00701d).w(m_spg_video, FUNC(gcm394_base_video_device::video_701d_w));
	map(0x00701e, 0x00701e).w(m_spg_video, FUNC(gcm394_base_video_device::video_701e_w));

	// tilebase LSBs
	map(0x007020, 0x007020).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap0_tilebase_lsb_r), FUNC(gcm394_base_video_device::tmap0_tilebase_lsb_w));           // tilebase, written with other tmap0 regs
	map(0x007021, 0x007021).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap1_tilebase_lsb_r), FUNC(gcm394_base_video_device::tmap1_tilebase_lsb_w));           // tilebase, written with other tmap1 regs
	map(0x007022, 0x007022).rw(m_spg_video, FUNC(gcm394_base_video_device::sprite_7022_gfxbase_lsb_r), FUNC(gcm394_base_video_device::sprite_7022_gfxbase_lsb_w)); // sprite tilebase written as 7022, 702d and 7042 group
	map(0x007023, 0x007023).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap2_tilebase_lsb_r), FUNC(gcm394_base_video_device::tmap2_tilebase_lsb_w));           // written with other tmap2 regs (roz layer or line layer?)
	map(0x007024, 0x007024).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap3_tilebase_lsb_r), FUNC(gcm394_base_video_device::tmap3_tilebase_lsb_w));           // written with other tmap3 regs (roz layer or line layer?)

	map(0x00702a, 0x00702a).w(m_spg_video, FUNC(gcm394_base_video_device::video_702a_w)); // blend level control

	// tilebase MSBs
	map(0x00702b, 0x00702b).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap0_tilebase_msb_r), FUNC(gcm394_base_video_device::tmap0_tilebase_msb_w));           // written with other tmap0 regs
	map(0x00702c, 0x00702c).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap1_tilebase_msb_r), FUNC(gcm394_base_video_device::tmap1_tilebase_msb_w));           // written with other tmap1 regs
	map(0x00702d, 0x00702d).rw(m_spg_video, FUNC(gcm394_base_video_device::sprite_702d_gfxbase_msb_r), FUNC(gcm394_base_video_device::sprite_702d_gfxbase_msb_w)); // sprites, written as 7022, 702d and 7042 group
	map(0x00702e, 0x00702e).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap2_tilebase_msb_r), FUNC(gcm394_base_video_device::tmap2_tilebase_msb_w));           // written with other tmap2 regs (roz layer or line layer?)
	map(0x00702f, 0x00702f).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap3_tilebase_msb_r), FUNC(gcm394_base_video_device::tmap3_tilebase_msb_w));           // written with other tmap3 regs (roz layer or line layer?)

	map(0x007036, 0x007036).w(m_spg_video, FUNC(gcm394_base_video_device::split_irq_xpos_w));
	map(0x007037, 0x007037).w(m_spg_video, FUNC(gcm394_base_video_device::split_irq_ypos_w));


	map(0x007030, 0x007030).rw(m_spg_video, FUNC(gcm394_base_video_device::video_7030_brightness_r), FUNC(gcm394_base_video_device::video_7030_brightness_w));
	map(0x007038, 0x007038).r(m_spg_video, FUNC(gcm394_base_video_device::video_curline_r));
	map(0x00703a, 0x00703a).rw(m_spg_video, FUNC(gcm394_base_video_device::video_703a_palettebank_r), FUNC(gcm394_base_video_device::video_703a_palettebank_w));
	map(0x00703c, 0x00703c).rw(m_spg_video, FUNC(gcm394_base_video_device::video_703c_tvcontrol1_r), FUNC(gcm394_base_video_device::video_703c_tvcontrol1_w)); // TV Control 1

	map(0x007042, 0x007042).rw(m_spg_video, FUNC(gcm394_base_video_device::sprite_7042_extra_r), FUNC(gcm394_base_video_device::sprite_7042_extra_w)); // maybe sprites,  written as 7022, 702d and 7042 group

	map(0x007051, 0x007051).r(m_spg_video, FUNC(gcm394_base_video_device::video_7051_r)); // wrlshunt checks this (doesn't exist on older SPG?)

	map(0x007062, 0x007062).rw(m_spg_video, FUNC(gcm394_base_video_device::videoirq_source_enable_r), FUNC(gcm394_base_video_device::videoirq_source_enable_w));
	map(0x007063, 0x007063).rw(m_spg_video, FUNC(gcm394_base_video_device::video_7063_videoirq_source_r), FUNC(gcm394_base_video_device::video_7063_videoirq_source_ack_w));

	// note, 70 / 71 / 72 are the same offsets used for DMA as in spg2xx video device
	map(0x007070, 0x007070).w(m_spg_video, FUNC(gcm394_base_video_device::video_dma_source_w));                                                      // video dma, not system dma? (sets pointers to ram buffers)
	map(0x007071, 0x007071).w(m_spg_video, FUNC(gcm394_base_video_device::video_dma_dest_w));                                                        // sets pointers to 7300, 7400 ram areas below
	map(0x007072, 0x007072).rw(m_spg_video, FUNC(gcm394_base_video_device::video_dma_size_busy_r), FUNC(gcm394_base_video_device::video_dma_size_trigger_w));     //

	// these don't exist on older SPG
	map(0x00707c, 0x00707c).r(m_spg_video, FUNC(gcm394_base_video_device::video_707c_r)); // wrlshunt polls this waiting for 0x8000, is this some kind of manual port-based data upload?

	map(0x00707e, 0x00707e).w(m_spg_video, FUNC(gcm394_base_video_device::video_707e_spritebank_w));                                                         // written around same time as DMA, seems to select alt sprite bank
	map(0x00707f, 0x00707f).rw(m_spg_video, FUNC(gcm394_base_video_device::video_707f_r), FUNC(gcm394_base_video_device::video_707f_w));

	// another set of registers for something?
	map(0x007080, 0x007080).w(m_spg_video, FUNC(gcm394_base_video_device::video_7080_w));
	map(0x007081, 0x007081).w(m_spg_video, FUNC(gcm394_base_video_device::video_7081_w));
	map(0x007082, 0x007082).w(m_spg_video, FUNC(gcm394_base_video_device::video_7082_w));
	map(0x007083, 0x007083).rw(m_spg_video, FUNC(gcm394_base_video_device::video_7083_r), FUNC(gcm394_base_video_device::video_7083_w));
	map(0x007084, 0x007084).w(m_spg_video, FUNC(gcm394_base_video_device::video_7084_w));
	map(0x007085, 0x007085).w(m_spg_video, FUNC(gcm394_base_video_device::video_7085_w));
	map(0x007086, 0x007086).w(m_spg_video, FUNC(gcm394_base_video_device::video_7086_w));
	map(0x007087, 0x007087).w(m_spg_video, FUNC(gcm394_base_video_device::video_7087_w));
	map(0x007088, 0x007088).w(m_spg_video, FUNC(gcm394_base_video_device::video_7088_w));

	map(0x0070e0, 0x0070e0).r(m_spg_video, FUNC(gcm394_base_video_device::video_70e0_prng_r)); // gormiti checks this

	// ######################################################################################################################################################################################
	// 73xx-77xx = video ram
	// ######################################################################################################################################################################################

	map(0x007100, 0x0071ff).ram().share("rowscroll"); // based on jak_s500
	map(0x007200, 0x0072ff).ram().share("rowzoom"); // ^^

	map(0x007300, 0x0073ff).rw(m_spg_video, FUNC(gcm394_base_video_device::palette_r), FUNC(gcm394_base_video_device::palette_w));

	map(0x007400, 0x0077ff).rw(m_spg_video, FUNC(gcm394_base_video_device::spriteram_r), FUNC(gcm394_base_video_device::spriteram_w));

	// ######################################################################################################################################################################################
	// 78xx region = system regs?
	// ######################################################################################################################################################################################

	map(0x007803, 0x007803).rw(FUNC(sunplus_gcm394_base_device::unkarea_7803_r), FUNC(sunplus_gcm394_base_device::unkarea_7803_w));

	map(0x007807, 0x007807).w(FUNC(sunplus_gcm394_base_device::unkarea_7807_w));
	// 7808

	// 780a

	map(0x00780c, 0x00780c).w(FUNC(sunplus_gcm394_base_device::waitmode_enter_780c_w));

	map(0x00780f, 0x00780f).r(FUNC(sunplus_gcm394_base_device::unkarea_780f_status_r));

	map(0x007810, 0x007810).rw(FUNC(sunplus_gcm394_base_device::membankswitch_7810_r), FUNC(sunplus_gcm394_base_device::membankswitch_7810_w));  // 7810 Bank Switch Control Register  (P_BankSwitch_Ctrl) (maybe)

	map(0x007819, 0x007819).rw(FUNC(sunplus_gcm394_base_device::unkarea_7819_r), FUNC(sunplus_gcm394_base_device::unkarea_7819_w));

	map(0x007816, 0x007816).w(FUNC(sunplus_gcm394_base_device::unkarea_7816_w));
	map(0x007817, 0x007817).w(FUNC(sunplus_gcm394_base_device::unkarea_7817_w));

	// ######################################################################################################################################################################################
	// 782x region = memory config / control
	// ######################################################################################################################################################################################
																										 // wrlshunt                                                               | smartfp
	map(0x007820, 0x007824).w(FUNC(sunplus_gcm394_base_device::chipselect_csx_memory_device_control_w)); // 7f8a (7f8a before DMA from ROM to RAM, 008a after DMA from ROM to RAM) | 3f04      7820 Chip Select (CS0) Memory Device Control (P_MC50_Ctrl)
																										 // 7f47                                                                   | 0044      7821 Chip Select (CS1) Memory Device Control (P_MC51_Ctrl)
																										 // 0047                                                                   | 1f44      7822 Chip Select (CS2) Memory Device Control (P_MC52_Ctrl)
																										 // 0047                                                                   | 0044      7823 Chip Select (CS3) Memory Device Control (P_MC53_Ctrl)
																										 // 0047                                                                   | 0044      7824 Chip Select (CS4) Memory Device Control (P_MC54_Ctrl)

	map(0x00782d, 0x00782d).rw(FUNC(sunplus_gcm394_base_device::unkarea_782d_r), FUNC(sunplus_gcm394_base_device::unkarea_782d_w)); // on startup
	// 782f

	map(0x007835, 0x007835).w(FUNC(sunplus_gcm394_base_device::unkarea_7835_w));

	// 783a
	// 783b
	// 783c
	// 783d
	// 783e

	// 7840 - accessed by code in RAM when changing bank in tkmag220
	// 7841 - ^^

	// ######################################################################################################################################################################################
	// 786x - 787x - IO related?
	// ######################################################################################################################################################################################

	map(0x007860, 0x007860).rw(FUNC(sunplus_gcm394_base_device::ioarea_7860_porta_r), FUNC(sunplus_gcm394_base_device::ioarea_7860_porta_w)); //    7860  I/O PortA Data Register
	map(0x007861, 0x007861).rw(FUNC(sunplus_gcm394_base_device::ioarea_7861_porta_buffer_r), FUNC(sunplus_gcm394_base_device::ioarea_7861_porta_buffer_w)); // 7861  I/O PortA Buffer Register
	map(0x007862, 0x007862).rw(FUNC(sunplus_gcm394_base_device::ioarea_7862_porta_direction_r), FUNC(sunplus_gcm394_base_device::ioarea_7862_porta_direction_w));  // 7862  I/O PortA Direction Register
	map(0x007863, 0x007863).rw(FUNC(sunplus_gcm394_base_device::ioarea_7863_porta_attribute_r), FUNC(sunplus_gcm394_base_device::ioarea_7863_porta_attribute_w)); //    7863  I/O PortA Attribute Register

	map(0x007868, 0x007868).rw(FUNC(sunplus_gcm394_base_device::ioarea_7868_portb_r), FUNC(sunplus_gcm394_base_device::ioarea_7868_portb_w)); // on startup   // 7868  I/O PortB Data Register
	map(0x007869, 0x007869).rw(FUNC(sunplus_gcm394_base_device::ioarea_7869_portb_buffer_r), FUNC(sunplus_gcm394_base_device::ioarea_7869_portb_buffer_w)); //  7869  I/O PortB Buffer Register   // jak_s500
	map(0x00786a, 0x00786a).rw(FUNC(sunplus_gcm394_base_device::ioarea_786a_portb_direction_r), FUNC(sunplus_gcm394_base_device::ioarea_786a_portb_direction_w)); // 786a  I/O PortB Direction Register
	map(0x00786b, 0x00786b).rw(FUNC(sunplus_gcm394_base_device::ioarea_786b_portb_attribute_r), FUNC(sunplus_gcm394_base_device::ioarea_786b_portb_attribute_w)); // 786b  I/O PortB Attribute Register
	// 786c  I/O PortB Latch / Wakeup

	map(0x007870, 0x007870).rw(FUNC(sunplus_gcm394_base_device::ioarea_7870_portc_r) ,FUNC(sunplus_gcm394_base_device::ioarea_7870_portc_w)); // 7870  I/O PortC Data Register
	map(0x007871, 0x007871).rw(FUNC(sunplus_gcm394_base_device::ioarea_7871_portc_buffer_r), FUNC(sunplus_gcm394_base_device::ioarea_7871_portc_buffer_w)); // 7871  I/O PortC Buffer Register
	map(0x007872, 0x007872).rw(FUNC(sunplus_gcm394_base_device::ioarea_7872_portc_direction_r), FUNC(sunplus_gcm394_base_device::ioarea_7872_portc_direction_w)); // 7872  I/O PortC Direction Register
	map(0x007873, 0x007873).rw(FUNC(sunplus_gcm394_base_device::ioarea_7873_portc_attribute_r), FUNC(sunplus_gcm394_base_device::ioarea_7873_portc_attribute_w)); // 7873  I/O PortC Attribute Register

	// 7874 (data 0x1249) (bkrankp data 0x36db)

	map(0x007878, 0x007878).rw(FUNC(sunplus_gcm394_base_device::ioarea_7878_portd_r) ,FUNC(sunplus_gcm394_base_device::ioarea_7878_portd_w)); // 7878  I/O PortD Data Register
	map(0x007879, 0x007879).rw(FUNC(sunplus_gcm394_base_device::ioarea_7879_portd_buffer_r), FUNC(sunplus_gcm394_base_device::ioarea_7879_portd_buffer_w)); // 7879  I/O PortD Buffer Register
	map(0x00787a, 0x00787a).rw(FUNC(sunplus_gcm394_base_device::ioarea_787a_portd_direction_r), FUNC(sunplus_gcm394_base_device::ioarea_787a_portd_direction_w)); // 787a  I/O PortD Direction Register
	map(0x00787b, 0x00787b).rw(FUNC(sunplus_gcm394_base_device::ioarea_787b_portd_attribute_r), FUNC(sunplus_gcm394_base_device::ioarea_787b_portd_attribute_w)); // 787b  I/O PortD Attribute Register

	// 787c (data 0x1249) (bkrankp data 0x36db)
	// 787e (data 0x1249) (bkrankp data 0x36db)

	// 7880

	map(0x007882, 0x007882).rw(FUNC(sunplus_gcm394_base_device::unkarea_7882_r), FUNC(sunplus_gcm394_base_device::unkarea_7882_w));
	map(0x007883, 0x007883).rw(FUNC(sunplus_gcm394_base_device::unkarea_7883_r), FUNC(sunplus_gcm394_base_device::unkarea_7883_w));

	// 0x7888 (data 0x1249) (bkrankp data 0x36db), written with 7874 / 787c / 787e above

	// ######################################################################################################################################################################################
	// 78ax - interrupt controller?
	// ######################################################################################################################################################################################

	map(0x0078a0, 0x0078a0).rw(FUNC(sunplus_gcm394_base_device::unkarea_78a0_r), FUNC(sunplus_gcm394_base_device::unkarea_78a0_w));
	map(0x0078a1, 0x0078a1).r(FUNC(sunplus_gcm394_base_device::unkarea_78a1_r));

	map(0x0078a4, 0x0078a4).w(FUNC(sunplus_gcm394_base_device::unkarea_78a4_w));
	map(0x0078a5, 0x0078a5).w(FUNC(sunplus_gcm394_base_device::unkarea_78a5_w));
	map(0x0078a6, 0x0078a6).w(FUNC(sunplus_gcm394_base_device::unkarea_78a6_w));

	map(0x0078a8, 0x0078a8).w(FUNC(sunplus_gcm394_base_device::unkarea_78a8_w));

	// ######################################################################################################################################################################################
	// 78bx - timer control?
	// ######################################################################################################################################################################################

	map(0x0078b0, 0x0078b0).w(FUNC(sunplus_gcm394_base_device::unkarea_78b0_w));  // 78b0 TimeBase A Control Register (P_TimeBaseA_Ctrl)
	map(0x0078b1, 0x0078b1).w(FUNC(sunplus_gcm394_base_device::unkarea_78b1_w));  // 78b1 TimeBase B Control Register (P_TimeBaseB_Ctrl)
	map(0x0078b2, 0x0078b2).rw(FUNC(sunplus_gcm394_base_device::unkarea_78b2_r), FUNC(sunplus_gcm394_base_device::unkarea_78b2_w));  // 78b2 TimeBase C Control Register (P_TimeBaseC_Ctrl)

	map(0x0078b8, 0x0078b8).w(FUNC(sunplus_gcm394_base_device::unkarea_78b8_w));  // 78b8 TimeBase Counter Reset Register  (P_TimeBase_Reset)

	map(0x0078c0, 0x0078c0).r(FUNC(sunplus_gcm394_base_device::unkarea_78c0_r)); // beijuehh

	map(0x0078c8, 0x0078c8).r(FUNC(sunplus_gcm394_base_device::unkarea_78c8_r)); // dressmtv

	map(0x0078d0, 0x0078d0).r(FUNC(sunplus_gcm394_base_device::unkarea_78d0_r)); // jak_s500
	map(0x0078d8, 0x0078d8).r(FUNC(sunplus_gcm394_base_device::unkarea_78d8_r)); // jak_tsh


	// ######################################################################################################################################################################################
	// 78fx - unknown
	// ######################################################################################################################################################################################

	map(0x0078f0, 0x0078f0).rw(FUNC(sunplus_gcm394_base_device::unkarea_78f0_r), FUNC(sunplus_gcm394_base_device::unkarea_78f0_w));

	map(0x0078fb, 0x0078fb).r(FUNC(sunplus_gcm394_base_device::unkarea_78fb_status_r));

	// ######################################################################################################################################################################################
	// 793x - misc?
	// ######################################################################################################################################################################################

	map(0x007904, 0x007904).r(FUNC(sunplus_gcm394_base_device::unkarea_7904_r)); // lazertag after a while

	// possible rtc?
	map(0x007934, 0x007934).rw(FUNC(sunplus_gcm394_base_device::unkarea_7934_r), FUNC(sunplus_gcm394_base_device::unkarea_7934_w));
	map(0x007935, 0x007935).rw(FUNC(sunplus_gcm394_base_device::unkarea_7935_r), FUNC(sunplus_gcm394_base_device::unkarea_7935_w));
	map(0x007936, 0x007936).rw(FUNC(sunplus_gcm394_base_device::unkarea_7936_r), FUNC(sunplus_gcm394_base_device::unkarea_7936_w));

	// ######################################################################################################################################################################################
	// 794x - SPI
	// ######################################################################################################################################################################################

	//7940 P_SPI_Ctrl     - SPI Control Register
	//7941 P_SPI_TXStatus - SPI Transmit Status Register
	map(0x007942, 0x007942).w(FUNC(sunplus_gcm394_base_device::spi_7942_txdata_w)); //7942 P_SPI_TXData   - SPI Transmit FIFO Register
	//7943 P_SPI_RXStatus - SPI Receive Status Register
	map(0x007944, 0x007944).r(FUNC(sunplus_gcm394_base_device::spi_7944_rxdata_r));           // 7944 P_SPI_RXData - SPI Receive FIFO Register    (jak_s500 accelerometer)   (also the SPI ROM DMA input port for bkrankp?)
	map(0x007945, 0x007945).r(FUNC(sunplus_gcm394_base_device::spi_7945_misc_control_reg_r)); // 7945 P_SPI_Misc   - SPI Misc Control Register    (jak_s500 accelerometer)

	// ######################################################################################################################################################################################
	// 796x - unknown
	// ######################################################################################################################################################################################

	// possible adc?
	map(0x007960, 0x007960).w(FUNC(sunplus_gcm394_base_device::unkarea_7960_w));
	map(0x007961, 0x007961).rw(FUNC(sunplus_gcm394_base_device::unkarea_7961_r), FUNC(sunplus_gcm394_base_device::unkarea_7961_w));
	map(0x007962, 0x007962).r(FUNC(sunplus_gcm394_base_device::unkarea_7962_r));

	// ######################################################################################################################################################################################
	// 7axx region = system (including dma)
	// ######################################################################################################################################################################################

	map(0x007a35, 0x007a35).r(FUNC(sunplus_gcm394_base_device::system_7a35_r)); // wlsair60
	map(0x007a37, 0x007a37).r(FUNC(sunplus_gcm394_base_device::system_7a37_r)); // wlsair60
	map(0x007a39, 0x007a39).r(FUNC(sunplus_gcm394_base_device::system_7a39_r)); // wlsair60
	map(0x007a3a, 0x007a3a).r(FUNC(sunplus_gcm394_base_device::system_7a3a_r)); // ?
	map(0x007a46, 0x007a46).r(FUNC(sunplus_gcm394_base_device::system_7a46_r)); // wlsair60
	map(0x007a54, 0x007a54).r(FUNC(sunplus_gcm394_base_device::system_7a54_r)); // wlsair60

	map(0x007a80, 0x007a87).rw(FUNC(sunplus_gcm394_base_device::system_dma_params_channel0_r), FUNC(sunplus_gcm394_base_device::system_dma_params_channel0_w));
	map(0x007a88, 0x007a8f).rw(FUNC(sunplus_gcm394_base_device::system_dma_params_channel1_r), FUNC(sunplus_gcm394_base_device::system_dma_params_channel1_w)); // jak_tsm writes here
	map(0x007a90, 0x007a97).rw(FUNC(sunplus_gcm394_base_device::system_dma_params_channel2_r), FUNC(sunplus_gcm394_base_device::system_dma_params_channel2_w)); // bkrankp writes here (is this on all types or just SPI?)
	map(0x007a98, 0x007a9f).rw(FUNC(sunplus_gcm394_base_device::system_dma_params_channel3_r), FUNC(sunplus_gcm394_base_device::system_dma_params_channel3_w)); // not seen, but probably

	map(0x007abe, 0x007abe).rw(FUNC(sunplus_gcm394_base_device::system_dma_memtype_r), FUNC(sunplus_gcm394_base_device::system_dma_memtype_w)); // 7abe - written with DMA stuff (source type for each channel so that device handles timings properly?)
	map(0x007abf, 0x007abf).rw(FUNC(sunplus_gcm394_base_device::system_dma_status_r), FUNC(sunplus_gcm394_base_device::system_dma_7abf_unk_w));

	// ######################################################################################################################################################################################
	// 7bxx-7fxx = audio
	// ######################################################################################################################################################################################

	map(0x007b80, 0x007bbf).rw(m_spg_audio, FUNC(sunplus_gcm394_audio_device::control_r), FUNC(sunplus_gcm394_audio_device::control_w));
	map(0x007c00, 0x007dff).rw(m_spg_audio, FUNC(sunplus_gcm394_audio_device::audio_r), FUNC(sunplus_gcm394_audio_device::audio_w));
	map(0x007e00, 0x007fff).rw(m_spg_audio, FUNC(sunplus_gcm394_audio_device::audio_phase_r), FUNC(sunplus_gcm394_audio_device::audio_phase_w));

}

void sunplus_gcm394_base_device::gcm394_internal_map(address_map& map)
{
	sunplus_gcm394_base_device::base_internal_map(map);

	// no internal ROM on this model?

	map(0x08000, 0x0ffff).r(FUNC(sunplus_gcm394_base_device::internalrom_lower32_r)).nopw();

	map(0x10000, 0x01ffff).nopr();

	map(0x020000, 0x1fffff).rw(FUNC(sunplus_gcm394_base_device::cs_space_r), FUNC(sunplus_gcm394_base_device::cs_space_w));
	map(0x200000, 0x3fffff).rw(FUNC(sunplus_gcm394_base_device::cs_bank_space_r), FUNC(sunplus_gcm394_base_device::cs_bank_space_w));
}

uint16_t sunplus_gcm394_base_device::cs_space_r(offs_t offset)
{
	return m_cs_space->read_word(offset);
}

void sunplus_gcm394_base_device::cs_space_w(offs_t offset, uint16_t data)
{
	m_cs_space->write_word(offset, data);
}
uint16_t sunplus_gcm394_base_device::cs_bank_space_r(offs_t offset)
{
	int bank = m_membankswitch_7810 & 0x3f;
	int realoffset = offset + (bank * 0x200000) - m_csbase;

	if (realoffset < 0)
	{
		logerror("read real offset < 0\n");
		return 0;
	}

	return m_cs_space->read_word(realoffset);
}

void sunplus_gcm394_base_device::cs_bank_space_w(offs_t offset, uint16_t data)
{
	int bank = m_membankswitch_7810 & 0x3f;
	int realoffset = offset + (bank * 0x200000) - m_csbase;

	if (realoffset < 0)
	{
		logerror("write real offset < 0\n");
		return;
	}

	m_cs_space->write_word(realoffset, data);
}



uint16_t sunplus_gcm394_base_device::internalrom_lower32_r(offs_t offset)
{
	if (m_boot_mode == 0)
	{
		uint16_t* introm = (uint16_t*)m_internalrom->base();
		return introm[offset];
	}
	else
	{
		if (!m_cs_space)
			return 0x0000;

		uint16_t val = m_cs_space->read_word(offset+0x8000);
		return val;
	}
}


void sunplus_gcm394_base_device::device_start()
{
	unsp_20_device::device_start();

	m_cs_callback.resolve();

	m_unk_timer = timer_alloc(FUNC(sunplus_gcm394_base_device::unknown_tick), this);
	m_unk_timer->adjust(attotime::never);

	save_item(NAME(m_dma_params));
	save_item(NAME(m_7803));
	save_item(NAME(m_7807));
	save_item(NAME(m_membankswitch_7810));
	save_item(NAME(m_7816));
	save_item(NAME(m_7817));
	save_item(NAME(m_7819));
	save_item(NAME(m_782x));
	save_item(NAME(m_782d));
	save_item(NAME(m_7835));
	save_item(NAME(m_7860));
	save_item(NAME(m_7861));
	save_item(NAME(m_7862_porta_direction));
	save_item(NAME(m_7863_porta_attribute));

	save_item(NAME(m_786a_portb_direction));
	save_item(NAME(m_786b_portb_attribute));

	save_item(NAME(m_7870));
	//save_item(NAME(m_7871));
	save_item(NAME(m_7872_portc_direction));
	save_item(NAME(m_7873_portc_attribute));
	save_item(NAME(m_7882));
	save_item(NAME(m_7883));
	save_item(NAME(m_78a0));
	save_item(NAME(m_78a4));
	save_item(NAME(m_78a5));
	save_item(NAME(m_78a6));
	save_item(NAME(m_78a8));
	save_item(NAME(m_78b0));
	save_item(NAME(m_78b1));
	save_item(NAME(m_78b2));
	save_item(NAME(m_78b8));
	save_item(NAME(m_78f0));
	save_item(NAME(m_78fb));
	save_item(NAME(m_7934));
	save_item(NAME(m_7935));
	save_item(NAME(m_7936));
	save_item(NAME(m_7960));
	save_item(NAME(m_7961));
	save_item(NAME(m_system_dma_memtype));
	save_item(NAME(m_csbase));
	save_item(NAME(m_romtype));
}

void sunplus_gcm394_base_device::device_reset()
{
	unsp_20_device::device_reset();

	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < 7; i++)
		{
			m_dma_params[i][j] = 0x0000;
		}
		m_dma_latched[j] = false;
	}

	// 78xx unknown

	m_78fb = 0x0000;
	m_782d = 0x0000;

	m_7807 = 0x0000;

	m_membankswitch_7810 = 0x0001;

	m_7816 = 0x0000;
	m_7817 = 0x0000;

	m_7819 = 0x0000;

	m_782x[0] = 0x0000;
	m_782x[1] = 0x0000;
	m_782x[2] = 0x0000;
	m_782x[3] = 0x0000;
	m_782x[4] = 0x0000;

	m_7835 = 0x0000;

	m_7860 = 0x0000;

	m_7861 = 0x0000;

	m_7862_porta_direction = 0x0000;
	m_7863_porta_attribute = 0x0000;

	m_786a_portb_direction = 0x0000;
	m_786b_portb_attribute = 0x0000;

	m_7870 = 0x0000;

	//m_7871 = 0x0000;

	m_7872_portc_direction = 0x0000;
	m_7873_portc_attribute = 0x0000;

	m_7882 = 0x0000;
	m_7883 = 0x0000;

	m_78a0 = 0x0000;

	m_78a4 = 0x0000;
	m_78a5 = 0x0000;
	m_78a6 = 0x0000;

	m_78a8 = 0x0000;

	m_78b0 = 0x0000;
	m_78b1 = 0x0000;
	m_78b2 = 0x0000;

	m_78b8 = 0x0000;
	m_78f0 = 0x0000;

	// 79xx unknown

	m_7934 = 0x0000;
	m_7935 = 0x0000;
	m_7936 = 0x0000;

	m_7960 = 0x0000;
	m_7961 = 0x0000;

	m_system_dma_memtype = 0x0000;

	m_unk_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));

	m_spg_video->set_video_spaces(this->space(AS_PROGRAM), *m_cs_space, m_csbase);
	m_spg_video->reset();
}


IRQ_CALLBACK_MEMBER(sunplus_gcm394_base_device::irq_vector_cb)
{
	//logerror("irq_vector_cb %d\n", irqline);

	if (irqline == UNSP_IRQ6_LINE)
		set_state_unsynced(UNSP_IRQ6_LINE, CLEAR_LINE);

	if (irqline == UNSP_IRQ4_LINE)
		set_state_unsynced(UNSP_IRQ4_LINE, CLEAR_LINE);

	return 0;
}


void sunplus_gcm394_base_device::checkirq6()
{
/*
    if (m_7935 & 0x0100)
        set_state_unsynced(UNSP_IRQ6_LINE, ASSERT_LINE);
    else
        set_state_unsynced(UNSP_IRQ6_LINE, CLEAR_LINE);
*/
}

/* the IRQ6 interrupt on Wrlshunt
   reads 78a1, checks bit 0400
   if it IS set, just increases value in RAM at 1a2e  (no ack)
   if it ISN'T set, then read/write 78b2 (ack something?) then increase value in RAM  at 1a2e
   (smartfp doesn't touch these addresses? but instead reads/writes 7935, alt ack?)

   wrlshunt also has an IRQ4
   it always reads/writes 78c0 before executing payload (ack?)
   payload is a lot of manipulation of values in RAM, no registers touched

   wrlshunt also has FIQ
   no ack mechanism, for sound timer maybe (as it appears to be on spg110)


   ----

   IRQ5 is video IRQ
   in both games writing 0x0001 to 7063 seems to be an ack mechanism
   wrlshunt also checks bit 0x0040 of 7863 and will ack that too with alt code paths

   7863 is therefore some kind of 'video irq source' ?

*/


TIMER_CALLBACK_MEMBER(sunplus_gcm394_base_device::unknown_tick)
{
	m_7935 |= 0x0100;

	if (m_alt_periodic_irq)
		set_state_unsynced(UNSP_IRQ4_LINE, ASSERT_LINE);
	else
		set_state_unsynced(UNSP_IRQ6_LINE, ASSERT_LINE);

	//  checkirq6();
}


void sunplus_gcm394_base_device::audioirq_w(int state)
{
	//set_state_unsynced(UNSP_IRQ5_LINE, state);
}

void sunplus_gcm394_base_device::videoirq_w(int state)
{
	set_state_unsynced(UNSP_IRQ5_LINE, state);
}

uint16_t sunplus_gcm394_base_device::read_space(uint32_t offset)
{
	address_space& space = this->space(AS_PROGRAM);
	uint16_t val;
	if (offset < m_csbase)
	{
		val = space.read_word(offset);
	}
	else
	{
		val = m_cs_space->read_word(offset-m_csbase);
	}

	return val;
}



void sunplus_gcm394_base_device::write_space(uint32_t offset, uint16_t data)
{
	address_space& space = this->space(AS_PROGRAM);
	if (offset < m_csbase)
	{
		space.write_word(offset, data);
	}
	else
	{
		m_cs_space->write_word(offset-m_csbase, data);
	}
}



void sunplus_gcm394_base_device::device_add_mconfig(machine_config &config)
{
	SUNPLUS_GCM394_AUDIO(config, m_spg_audio, DERIVED_CLOCK(1, 1));
	m_spg_audio->write_irq_callback().set(FUNC(sunplus_gcm394_base_device::audioirq_w));
	m_spg_audio->space_read_callback().set(FUNC(sunplus_gcm394_base_device::read_space));

	m_spg_audio->add_route(0, *this, 1.0, 0);
	m_spg_audio->add_route(1, *this, 1.0, 1);

	GCM394_VIDEO(config, m_spg_video, DERIVED_CLOCK(1, 1), DEVICE_SELF, m_screen);
	m_spg_video->write_video_irq_callback().set(FUNC(sunplus_gcm394_base_device::videoirq_w));
	m_spg_video->space_read_callback().set(FUNC(sunplus_gcm394_base_device::read_space));
}


