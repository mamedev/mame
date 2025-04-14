// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

  SunPlus "GCM394" (based on die pictures)

  Note, these are all the same chip but in different configurations so
  should be tidied up and made to use callbacks

**********************************************************************/

#include "emu.h"
#include "generalplus_gpl16250soc.h"


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
	m_boot_mode(0),
	m_cs_callback(*this, DEVICE_SELF, FUNC(sunplus_gcm394_base_device::default_cs_callback))
{
}

sunplus_gcm394_device::sunplus_gcm394_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sunplus_gcm394_base_device(mconfig, GCM394, tag, owner, clock)
{
}

DEFINE_DEVICE_TYPE(GPAC800, generalplus_gpac800_device, "gpac800", "GeneralPlus GPL16250 System-on-a-Chip (with NAND handling)")

generalplus_gpac800_device::generalplus_gpac800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sunplus_gcm394_base_device(mconfig, GPAC800, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpac800_device::gpac800_internal_map), this))
{
}


DEFINE_DEVICE_TYPE(GP_SPISPI, generalplus_gpspispi_device, "gpac800spi", "GeneralPlus GPL16250 (with SPI handling)")

generalplus_gpspispi_device::generalplus_gpspispi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sunplus_gcm394_base_device(mconfig, GP_SPISPI, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpspispi_device::gpspispi_internal_map), this))
{
}


void generalplus_gpspi_direct_device::ramwrite_w(offs_t offset, uint16_t data)
{
	// TODO: Gross hack, it puts some self-check code in RAM at startup, this replaces those calls with retf.
	if (offset == 0x100 && data == 0xf14c) data = 0x9a90;
	if (offset == 0x00 && data == 0x9311) data = 0x9a90;

	m_mainram[offset] = data;
}

uint16_t generalplus_gpspi_direct_device::ramread_r(offs_t offset)
{
	return m_mainram[offset];
}

uint16_t generalplus_gpspi_direct_device::spi_direct_7b40_r()
{
	return 0xffff; // doesn't care for now
}

uint16_t generalplus_gpspi_direct_device::spi_direct_79f5_r()
{
	return 0xffff; // hangs if returning 0
}

uint16_t generalplus_gpspi_direct_device::spi_direct_7b46_r()
{
	int i = machine().rand();

	if (i & 1) return 0x01;
	else return 0x02;
}

uint16_t generalplus_gpspi_direct_device::spi_direct_79f4_r()
{
	// status bits?
	return machine().rand();
}


uint16_t generalplus_gpspi_direct_device::spi_direct_7af0_r()
{
	return m_7af0;
}

void generalplus_gpspi_direct_device::spi_direct_7af0_w(uint16_t data)
{
	// words read from ROM are written here during the checksum routine in RAM, and must
	// be shifted for the checksum to pass.
	m_7af0 = data >> 8;
}


uint16_t generalplus_gpspi_direct_device::spi_direct_78e8_r()
{
	return machine().rand();
}

void generalplus_gpspi_direct_device::device_start()
{
	sunplus_gcm394_base_device::device_start();
	save_item(NAME(m_7af0));
}

void generalplus_gpspi_direct_device::device_reset()
{
	sunplus_gcm394_base_device::device_reset();
	m_7af0 = 0;
}

void generalplus_gpspi_direct_device::spi_direct_78e8_w(uint16_t data)
{
	logerror("%s: spi_direct_78e8_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpspi_direct_device::gpspi_direct_internal_map(address_map& map)
{
	sunplus_gcm394_base_device::base_internal_map(map);

	map(0x000000, 0x0027ff).rw(FUNC(generalplus_gpspi_direct_device::ramread_r), FUNC(generalplus_gpspi_direct_device::ramwrite_w));
	// TODO: RAM is only 0x2800 on this, like earlier SPG2xx models? unmap the extra from the base_internal_map?

	map(0x00780b, 0x00780b).nopw();

	map(0x0078e8, 0x0078e8).rw(FUNC(generalplus_gpspi_direct_device::spi_direct_78e8_r), FUNC(generalplus_gpspi_direct_device::spi_direct_78e8_w));

	map(0x0079f4, 0x0079f4).r(FUNC(generalplus_gpspi_direct_device::spi_direct_79f4_r));
	map(0x0079f5, 0x0079f5).r(FUNC(generalplus_gpspi_direct_device::spi_direct_79f5_r));

	map(0x007af0, 0x007af0).rw(FUNC(generalplus_gpspi_direct_device::spi_direct_7af0_r), FUNC(generalplus_gpspi_direct_device::spi_direct_7af0_w));

	map(0x007b40, 0x007b40).r(FUNC(generalplus_gpspi_direct_device::spi_direct_7b40_r));
//  map(0x007b46, 0x007b46).r(FUNC(generalplus_gpspi_direct_device::spi_direct_7b46_r));
	map(0x007b40, 0x007b40).nopw();
	map(0x007b41, 0x007b41).nopw();
	map(0x007b42, 0x007b42).nopw();

	map(0x007b46, 0x007b46).ram(); // values must be written and read from here, but is there any transformation?
	map(0x007b47, 0x007b47).nopw();
	map(0x007b48, 0x007b48).nopw();

	map(0x007b49, 0x007b49).ram();

	map(0x009000, 0x3fffff).rom().region("spidirect", 0);
}


DEFINE_DEVICE_TYPE(GP_SPI_DIRECT, generalplus_gpspi_direct_device, "gpac800spi_direct", "GeneralPlus GPL16250 (with direct SPI handling)")

generalplus_gpspi_direct_device::generalplus_gpspi_direct_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sunplus_gcm394_base_device(mconfig, GP_SPI_DIRECT, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpspi_direct_device::gpspi_direct_internal_map), this))
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

	// HACKS to get into service mode for debugging

	// note, these patch the code copied to SRAM so the 'PROGRAM ROM' check fails (it passes otherwise)

	address_space& mem = this->space(AS_PROGRAM);

	//if (mem.read_word(0x4368c) == 0x4846)
	//  mem.write_word(0x4368c, 0x4840);    // cars 2 force service mode

	//if (mem.read_word(0x34410) == 0x4846)
	//  mem.write_word(0x34410, 0x4840);    // golden tee force service mode

	// what is it waiting for when we need these? (needed on some service mode screens)
	//if (mem.read_word(0x3f368) == 0x4840)
	//  mem.write_word(0x3f368, 0x4841);    // cars 2 IRQ? wait hack

	//if (mem.read_word(0x4d8d4) == 0x4840)
	//  mem.write_word(0x4d8d4, 0x4841);    // golden tee IRQ? wait hack

	//if (mem.read_word(0x3510f) == 0x4845)
	//  mem.write_word(0x3510f, 0x4840);    // camp rock force service mode

	if (mem.read_word(0x4abe7) == 0x4840)
		mem.write_word(0x4abe7, 0x4841);    // camp rock IRQ? wait hack

	//if (mem.read_word(0x37244) == 0x4845)
	//  mem.write_word(0x37244, 0x4840);    // hannah montana guitar force service mode




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
void sunplus_gcm394_base_device::unkarea_78f0_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78f0_w %04x\n", machine().describe_context(), data); m_78f0 = data; }

uint16_t sunplus_gcm394_base_device::unkarea_78c0_r()
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78c0_r\n", machine().describe_context());
	return machine().rand();
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

uint16_t sunplus_gcm394_base_device::unkarea_7961_r() { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7961_r\n", machine().describe_context()); return m_7961; }
void sunplus_gcm394_base_device::unkarea_7961_w(uint16_t data) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7961_w %04x\n", machine().describe_context(), data); m_7961 = data; }


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

	map(0x0078d0, 0x0078d0).r(FUNC(sunplus_gcm394_base_device::unkarea_78d0_r)); // jak_s500
	map(0x0078d8, 0x0078d8).r(FUNC(sunplus_gcm394_base_device::unkarea_78d8_r)); // jak_tsh


	// ######################################################################################################################################################################################
	// 78fx - unknown
	// ######################################################################################################################################################################################

	map(0x0078f0, 0x0078f0).w(FUNC(sunplus_gcm394_base_device::unkarea_78f0_w));

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


// GPR27P512A   = C2 76
// HY27UF081G2A = AD F1 80 1D
// H27U518S2C   = AD 76

uint16_t generalplus_gpac800_device::nand_7854_r()
{
	// TODO: use actual NAND / Smart Media devices once this is better understood.
	// The games have extensive checks on startup to determine the flash types, but then it appears that
	// certain games (eg jak_tsm) will only function correctly with specific ones, even if the code
	// continues regardless.  Others will bail early if they don't get what they want.

	// I think some unSP core maths bugs are causing severe issues after the initial load for jak_tsm
	// at the moment, possibly the same ones that are causing rendering issues in the jak_gtg bitmap
	// test and seemingly incorrect road data for jak_car2, either that or the hookup here is very
	// non-standard outside of the ident codes

	// real TSM code starts at 4c000


	//logerror("%s:sunplus_gcm394_base_device::nand_7854_r\n", machine().describe_context());

	if (m_nandcommand == 0x90) // read ident
	{
		logerror("%s:sunplus_gcm394_base_device::nand_7854_r   READ IDENT byte %d\n", machine().describe_context(), m_curblockaddr);

		uint8_t data = 0x00;

		if (m_romtype == 0)
		{
			if (m_curblockaddr == 0)
				data = 0xc2;
			else
				data = 0x76;
		}
		else if (m_romtype == 1)
		{
			if (m_curblockaddr == 0)
				data = 0xad;
			else if (m_curblockaddr == 1)
				data = 0x76;
		}
		else
		{
			if (m_curblockaddr == 0)
				data = 0xad;
			else if (m_curblockaddr == 1)
				data = 0xf1;
			else if (m_curblockaddr == 2)
				data = 0x80;
			else if (m_curblockaddr == 3)
				data = 0x1d;
		}

		m_curblockaddr++;

		return data;
	}
	else if (m_nandcommand == 0x00 || m_nandcommand == 0x01 || m_nandcommand  == 0x50)
	{
		//logerror("%s:sunplus_gcm394_base_device::nand_7854_r   READ DATA byte %d\n", machine().describe_context(), m_curblockaddr);

		uint8_t data = m_nand_read_cb(m_effectiveaddress + m_curblockaddr);

		m_curblockaddr++;

		return data;
	}
	else if (m_nandcommand == 0x70) // read status
	{
		logerror("%s:sunplus_gcm394_base_device::nand_7854_r   READ STATUS byte %d\n", machine().describe_context(), m_curblockaddr);

		return 0xffff;
	}
	else
	{
		logerror("%s:sunplus_gcm394_base_device::nand_7854_r   READ UNKNOWN byte %d\n", machine().describe_context(), m_curblockaddr);
		return 0xffff;
	}

	return 0x0000;
}

// 7998

void generalplus_gpac800_device::nand_command_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_command_w %04x\n", machine().describe_context(), data);
	m_nandcommand = data;
}

void generalplus_gpac800_device::nand_addr_low_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_addr_low_w %04x\n", machine().describe_context(), data);
	m_nand_addr_low = data;
	m_curblockaddr = 0;
}

void generalplus_gpac800_device::recalculate_calculate_effective_nand_address()
{
	uint8_t type = m_nand_7856 & 0xf;
	uint8_t shift = 0;
	uint32_t page_offset = 0;

	if (type == 7)
		shift = 4;
	else if (type == 11)
		shift = 5;

	if (m_nandcommand == 0x01)
		page_offset = 256;
	else if (m_nandcommand == 0x50)
		page_offset = 512;

	uint32_t nandaddress = (m_nand_addr_high << 16) | m_nand_addr_low;

	if (m_nand_7850 & 0x4000)
		nandaddress *= 2;

	uint32_t page = type ? nandaddress : /*(m_nand_7850 & 0x4000) ?*/ nandaddress >> 8 /*: nandaddress >> 9*/;
	m_effectiveaddress = (page * 528 + page_offset) << shift;

	logerror("%s: Requested address is %08x, translating to %08x\n", machine().describe_context(), nandaddress, m_effectiveaddress);
}

void generalplus_gpac800_device::nand_addr_high_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_addr_high_w %04x\n", machine().describe_context(), data);
	m_nand_addr_high = data;

	recalculate_calculate_effective_nand_address();

	m_curblockaddr = 0;
}

void generalplus_gpac800_device::nand_dma_ctrl_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_dma_ctrl_w(?) %04x\n", machine().describe_context(), data);
	m_nand_dma_ctrl = data;
}

uint16_t generalplus_gpac800_device::nand_7850_status_r()
{
	// 0x8000 = ready
	return m_nand_7850 | 0x8000;
}

void generalplus_gpac800_device::nand_7850_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_7850_w %04x\n", machine().describe_context(), data);
	m_nand_7850 = data;
}

void generalplus_gpac800_device::nand_7856_type_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_7856_type_w %04x\n", machine().describe_context(), data);
	m_nand_7856 = data;

	recalculate_calculate_effective_nand_address();

	m_curblockaddr = 0;
}

void generalplus_gpac800_device::nand_7857_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_7857_w %04x\n", machine().describe_context(), data);
	m_nand_7857 = data;
}

void generalplus_gpac800_device::nand_785b_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_785b_w %04x\n", machine().describe_context(), data);
	m_nand_785b = data;
}

void generalplus_gpac800_device::nand_785c_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_785c_w %04x\n", machine().describe_context(), data);
	m_nand_785c = data;
}

void generalplus_gpac800_device::nand_785d_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_785d_w %04x\n", machine().describe_context(), data);
	m_nand_785d = data;
}

// [:maincpu] ':maincpu' (00146D)  jak_tsm
uint16_t generalplus_gpac800_device::nand_785e_r()
{
	return 0x0000;
}

//[:maincpu] ':maincpu' (001490)  jak_tsm
uint16_t generalplus_gpac800_device::nand_ecc_low_byte_error_flag_1_r()
{
	return 0x0000;
}

/*
UNMAPPED reads  writes

jak_tsm uses these (all iniitalized near start)
unclear if these are specific to the GPAC800 type, or present in the older types

[:maincpu] ':maincpu' (00043F):sunplus_gcm394_base_device::unk_w @ 0x780a (data 0x0000)
[:maincpu] ':maincpu' (000442):sunplus_gcm394_base_device::unk_w @ 0x7808 (data 0x0000)
[:maincpu] ':maincpu' (000445):sunplus_gcm394_base_device::unk_w @ 0x782f (data 0x0002)
[:maincpu] ':maincpu' (000449):sunplus_gcm394_base_device::unk_w @ 0x783d (data 0x05d9)
[:maincpu] ':maincpu' (00044D):sunplus_gcm394_base_device::unk_w @ 0x783c (data 0x0a57)
[:maincpu] ':maincpu' (000451):sunplus_gcm394_base_device::unk_w @ 0x783b (data 0x2400)
[:maincpu] ':maincpu' (000454):sunplus_gcm394_base_device::unk_w @ 0x783e (data 0x0002)
[:maincpu] ':maincpu' (000458):sunplus_gcm394_base_device::unk_w @ 0x783a (data 0x3011)
[:maincpu] ':maincpu' (00045B):sunplus_gcm394_base_device::unk_w @ 0x7874 (data 0x0000)
[:maincpu] ':maincpu' (00045D):sunplus_gcm394_base_device::unk_w @ 0x787c (data 0x0000)
[:maincpu] ':maincpu' (00045F):sunplus_gcm394_base_device::unk_w @ 0x7888 (data 0x0000)
[:maincpu] ':maincpu' (000461):sunplus_gcm394_base_device::unk_w @ 0x787e (data 0x0000)

jak_car2 uses these

[:maincpu] ':maincpu' (004056):sunplus_gcm394_base_device::unk_w @ 0x782f (data 0x0002)
[:maincpu] ':maincpu' (004059):sunplus_gcm394_base_device::unk_w @ 0x783d (data 0x05d9)
[:maincpu] ':maincpu' (00405C):sunplus_gcm394_base_device::unk_w @ 0x783c (data 0x0a57)
[:maincpu] ':maincpu' (00405F):sunplus_gcm394_base_device::unk_w @ 0x783b (data 0x2400)
[:maincpu] ':maincpu' (004062):sunplus_gcm394_base_device::unk_w @ 0x783e (data 0x0002)
[:maincpu] ':maincpu' (004065):sunplus_gcm394_base_device::unk_w @ 0x783a (data 0x3011)
[:maincpu] ':maincpu' (004069):sunplus_gcm394_base_device::unk_r @ 0x7880
[:maincpu] ':maincpu' (00406F):sunplus_gcm394_base_device::unk_w @ 0x7874 (data 0x1249)
[:maincpu] ':maincpu' (004071):sunplus_gcm394_base_device::unk_w @ 0x787c (data 0x1249)
[:maincpu] ':maincpu' (004073):sunplus_gcm394_base_device::unk_w @ 0x7888 (data 0x1249)
[:maincpu] ':maincpu' (004075):sunplus_gcm394_base_device::unk_w @ 0x787e (data 0x1249)
[:maincpu] ':maincpu' (004088):sunplus_gcm394_base_device::unk_w @ 0x7841 (data 0x000f)
[:maincpu] ':maincpu' (00408F):sunplus_gcm394_base_device::unk_w @ 0x780a (data 0x0000)
[:maincpu] ':maincpu' (004092):sunplus_gcm394_base_device::unk_w @ 0x7808 (data 0x0002)

[:maincpu] ':maincpu' (03000A):sunplus_gcm394_base_device::unk_w @ 0x7874 (data 0x36db)
[:maincpu] ':maincpu' (03000C):sunplus_gcm394_base_device::unk_w @ 0x787c (data 0x36db)
[:maincpu] ':maincpu' (03000E):sunplus_gcm394_base_device::unk_w @ 0x7888 (data 0x36db)
[:maincpu] ':maincpu' (030010):sunplus_gcm394_base_device::unk_w @ 0x787e (data 0x36db)
[:maincpu] ':maincpu' (030013):sunplus_gcm394_base_device::unk_w @ 0x787f (data 0x0010)
[:maincpu] ':maincpu' (03001D):sunplus_gcm394_base_device::unk_w @ 0x7804 (data 0x1c7f)
[:maincpu] ':maincpu' (030023):sunplus_gcm394_base_device::unk_w @ 0x7805 (data 0xcdf0)
[:maincpu] ':maincpu' (03E645):sunplus_gcm394_base_device::unk_w @ 0x7861 (data 0x1f66)
[:maincpu] ':maincpu' (03E64C):sunplus_gcm394_base_device::unk_w @ 0x786b (data 0x0000)
[:maincpu] ':maincpu' (03E64F):sunplus_gcm394_base_device::unk_w @ 0x7869 (data 0x0000)
[:maincpu] ':maincpu' (03E652):sunplus_gcm394_base_device::unk_w @ 0x786a (data 0x0000)
[:maincpu] ':maincpu' (03E65B):sunplus_gcm394_base_device::unk_w @ 0x7966 (data 0x0001)
[:maincpu] ':maincpu' (03CBD0):sunplus_gcm394_base_device::unk_w @ 0x7871 (data 0x0000)

-- this one seems like a common alt type of DMA, used in both hw types as it polls 707c status before doing it
[:maincpu] ':maincpu' (03B4C7):sunplus_gcm394_base_device::unk_w @ 0x707c (data 0x0001)
-- also video / alt dma?
[:maincpu] ':maincpu' (068C15):sunplus_gcm394_base_device::unk_r @ 0x707e

beambox sets things up with different values (ultimately stalls on some check, maybe seeprom?)

[:maincpu] ':maincpu' (00043F):sunplus_gcm394_base_device::unk_w @ 0x780a (data 0x0000)
[:maincpu] ':maincpu' (000442):sunplus_gcm394_base_device::unk_w @ 0x7808 (data 0x0000)
[:maincpu] ':maincpu' (000445):sunplus_gcm394_base_device::unk_w @ 0x782f (data 0x0002)
[:maincpu] ':maincpu' (000449):sunplus_gcm394_base_device::unk_w @ 0x783d (data 0x05d9)
[:maincpu] ':maincpu' (00044D):sunplus_gcm394_base_device::unk_w @ 0x783c (data 0x0f58)
[:maincpu] ':maincpu' (000451):sunplus_gcm394_base_device::unk_w @ 0x783b (data 0x2400)
[:maincpu] ':maincpu' (000454):sunplus_gcm394_base_device::unk_w @ 0x783e (data 0x0002)
[:maincpu] ':maincpu' (000458):sunplus_gcm394_base_device::unk_w @ 0x783a (data 0x4011)
[:maincpu] ':maincpu' (00045C):sunplus_gcm394_base_device::unk_w @ 0x7874 (data 0x2492)   -- note pair of 4, but different values to above games
[:maincpu] ':maincpu' (00045E):sunplus_gcm394_base_device::unk_w @ 0x787c (data 0x2492)
[:maincpu] ':maincpu' (000460):sunplus_gcm394_base_device::unk_w @ 0x7888 (data 0x2492)
[:maincpu] ':maincpu' (000462):sunplus_gcm394_base_device::unk_w @ 0x787e (data 0x2492)

vbaby code is very differet, attempts to load NAND block manually, not with DMA

*/


// all tilemap registers etc. appear to be in the same place as the above system, including the 'extra' ones not on the earlier models
// so it's likely this is built on top of that just with NAND support
void generalplus_gpac800_device::gpac800_internal_map(address_map& map)
{
	sunplus_gcm394_base_device::base_internal_map(map);

	// 785x = NAND device
	map(0x007850, 0x007850).rw(FUNC(generalplus_gpac800_device::nand_7850_status_r), FUNC(generalplus_gpac800_device::nand_7850_w)); // NAND Control Reg
	map(0x007851, 0x007851).w(FUNC(generalplus_gpac800_device::nand_command_w)); // NAND Command Reg
	map(0x007852, 0x007852).w(FUNC(generalplus_gpac800_device::nand_addr_low_w)); // NAND Low Address Reg
	map(0x007853, 0x007853).w(FUNC(generalplus_gpac800_device::nand_addr_high_w)); // NAND High Address Reg
	map(0x007854, 0x007854).r(FUNC(generalplus_gpac800_device::nand_7854_r)); // NAND Data Reg
	map(0x007855, 0x007855).w(FUNC(generalplus_gpac800_device::nand_dma_ctrl_w)); // NAND DMA / INT Control
	map(0x007856, 0x007856).w(FUNC(generalplus_gpac800_device::nand_7856_type_w)); // usually 0x0021?
	map(0x007857, 0x007857).w(FUNC(generalplus_gpac800_device::nand_7857_w));

	// most of these are likely ECC stuff for testing the ROM?
	map(0x00785b, 0x00785b).w(FUNC(generalplus_gpac800_device::nand_785b_w));
	map(0x00785c, 0x00785c).w(FUNC(generalplus_gpac800_device::nand_785c_w));
	map(0x00785d, 0x00785d).w(FUNC(generalplus_gpac800_device::nand_785d_w));
	map(0x00785e, 0x00785e).r(FUNC(generalplus_gpac800_device::nand_785e_r)); // also ECC status related?
	map(0x00785f, 0x00785f).r(FUNC(generalplus_gpac800_device::nand_ecc_low_byte_error_flag_1_r)); // ECC Low Byte Error Flag 1 (maybe)

	// 128kwords internal ROM
	//map(0x08000, 0x0ffff).rom().region("internal", 0); // lower 32kwords of internal ROM is visible / shadowed depending on boot pins and register
	map(0x08000, 0x0ffff).r(FUNC(generalplus_gpac800_device::internalrom_lower32_r)).nopw();
	map(0x10000, 0x27fff).rom().region("internal", 0x10000); // upper 96kwords of internal ROM is always visible
	map(0x28000, 0x2ffff).noprw(); // reserved
	// 0x30000+ is CS access

	map(0x030000, 0x1fffff).rw(FUNC(generalplus_gpac800_device::cs_space_r), FUNC(generalplus_gpac800_device::cs_space_w));
	map(0x200000, 0x3fffff).rw(FUNC(generalplus_gpac800_device::cs_bank_space_r), FUNC(generalplus_gpac800_device::cs_bank_space_w));
}


uint16_t generalplus_gpspispi_device::spi_unk_7943_r()
{
	return 0x0007;
}

void generalplus_gpspispi_device::gpspispi_internal_map(address_map& map)
{
	sunplus_gcm394_base_device::base_internal_map(map);

	map(0x007943, 0x007943).r(FUNC(generalplus_gpspispi_device::spi_unk_7943_r));

	map(0x008000, 0x00ffff).rom().region("internal", 0);
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

void generalplus_gpac800_device::device_reset()
{
	sunplus_gcm394_base_device::device_reset();

	m_nand_addr_low = 0x0000;
	m_nand_addr_high = 0x0000;
	m_nand_dma_ctrl = 0x0000;
	m_nand_7850 = 0x0000;
	m_nand_785d = 0x0000;
	m_nand_785c = 0x0000;
	m_nand_785b = 0x0000;
	m_nand_7856 = 0x0000;
	m_nand_7857 = 0x0000;
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
	set_state_unsynced(UNSP_IRQ6_LINE, ASSERT_LINE);
	//set_state_unsynced(UNSP_IRQ4_LINE, ASSERT_LINE);

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


