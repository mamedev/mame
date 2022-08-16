// license:BSD-3-Clause
// copyright-holders:Paul Arnold
/***************************************************************************
    Philips SCC66470 Video and System controller.

	This does not render the image to the display. It is up to the user
	to provide their own screen_update function. Pixel (palette offset) data
	can be obtained by calling line( line number ) for each line.
	Some boards have multiple video sources, the source being displayed being
	selected based on pixel value...is there a nice way of doing this other
	than leaving it to the board driver ?

	Todo:
		Add support for mosaic and RLE screens.
		Add remaining pixac operations. Only BCOLOUR1/BCOLOUR2 are supported.
		Add interlaced support.
		Add bep ?
		Verify number of cycles for each access.
***************************************************************************/

#include "emu.h"
#include "scc66470.h"
#include "screen.h"

#define CSR_REG  (m_csr)
#define DCR_REG  (m_dcr)
#define DCR2_REG (m_dcr2)

#define DCR_DE  15
#define DCR_CF1 14
#define DCR_CF2 13
#define DCR_FD  12
#define DCR_SM  11
#define DCR_SS  10
#define DCR_LS  9
#define DCR_CM  8
#define DCR_FG  7
#define DCR_DF  6
#define DCR_IC  5
#define DCR_DC  4

#define CSR_DI1 15
#define CSR_DI2 14
#define CSR_EW  10
#define CSR_DD1 9
#define CSR_DD2 8
#define CSR_DM1 7
#define CSR_DM2 6
#define CSR_TD  5
#define CSR_CG  4
#define CSR_DD  3
#define CSR_ED  2
#define CSR_ST  1
#define CSR_BE  0

#define DCR2_OM1 14
#define DCR2_OM2 13
#define DCR2_ID  12
#define DCR2_MF1 11
#define DCR2_MF2 10
#define DCR2_FT1 9
#define DCR2_FT2 8

#define CSR_R_DA  0x80
#define CSR_R_PA  0x20
#define CSR_R_IT2 0x04
#define CSR_R_IT1 0x02

#define SCC_INS_STOP             (  0 << 12 )
#define SCC_INS_NOP              (  1 << 12 )
#define SCC_INS_LOAD_DCP         (  2 << 12 )
#define SCC_INS_LOAD_DCP_STOP    (  3 << 12 )
#define SCC_INS_LOAD_VSR         (  4 << 12 )
#define SCC_INS_LOAD_VSR_STOP    (  5 << 12 )
#define SCC_INS_INTERRUPT        (  6 << 12 )
#define SCC_INS_LOAD_BORDER      ( 14 << 11 )
#define SCC_INS_LOAD_BORDER_DSP  ( 15 << 11 )
#define SCC_INS_BEP_CONTROL      (  1 << 15 )

#define PIXAC_4N  15
#define PIXAC_COL 14
#define PIXAC_EXC 13
#define PIXAC_CPY 12
#define PIXAC_CMP 11
#define PIXAC_RTL 10
#define PIXAC_SHK 9
#define PIXAC_ZOM 8
#define PIXAC_INV 3
#define PIXAC_BIT 2
#define PIXAC_TT  1
#define PIXAC_NI  0

struct horizontal_settings
{
	uint32_t pixels;
	uint32_t border;
};

static const horizontal_settings h_table[] =
{
	               //cf1 cf2 ss st
	{ 512,  64 },  // 0   0   0  0
	{ 512,  64 },  // 0   0   0  1
	{ 512,   0 },  // 0   0   1  0
	{ 512,   0 },  // 0   0   1  1

	{ 640, 128 },  // 0   1   0  0
	{ 640, 128 },  // 0   1   0  1
	{ 640,   0 },  // 0   1   1  0
	{ 640,   0 },  // 0   1   1  1

	{ 720,  80 },  // 1   0   0  0
	{ 720,  80 },  // 1   0   0  1

	{ 720,   0 },  // 1   0   1  0
	{ 720,   0 },  // 1   0   1  1

	{ 768, 128 },  // 1   1   0  0
	{ 720,  80 },  // 1   1   0  1

	{ 768,   0 },  // 1   1   1  0

	{ 768,  48 },  // 1   1   1  1
};

// device type definition
DEFINE_DEVICE_TYPE(SCC66470, scc66470_device, "scc66470", "Philips SCC66470")

//-------------------------------------------------
//  scc66470_device - constructor
//-------------------------------------------------

scc66470_device::scc66470_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SCC66470, tag, owner, clock),
		device_memory_interface(mconfig, *this),
		device_video_interface(mconfig, *this),
		m_irqcallback(*this),
		m_space_config("videoram", ENDIANNESS_BIG, 16, 21, 0, address_map_constructor(FUNC(scc66470_device::scc66470_vram), this))
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void scc66470_device::device_start()
{
	m_irqcallback.resolve_safe();

	m_ica_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(scc66470_device::process_ica), this));

	m_dca_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(scc66470_device::process_dca), this));

	save_item(NAME(m_csr));
	save_item(NAME(m_dcr));
	save_item(NAME(m_vsr));
	save_item(NAME(m_bcr));
	save_item(NAME(m_dcr2));
	save_item(NAME(m_dcp));
	save_item(NAME(m_swm));
	save_item(NAME(m_stm));
	save_item(NAME(m_reg_a));
	save_item(NAME(m_reg_b));
	save_item(NAME(m_pcr));
	save_item(NAME(m_mask));
	save_item(NAME(m_shift));
	save_item(NAME(m_index));
	save_item(NAME(m_fc));
	save_item(NAME(m_bc));
	save_item(NAME(m_tc));
	save_item(NAME(m_csr_r));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void scc66470_device::device_reset()
{
	m_working_dcp = 0;

	m_csr = 0;
	m_dcr = 0;
	m_vsr = 0;
	m_bcr = 0;
	m_dcr2 = 0;
	m_dcp = 0;
	m_swm = 0;
	m_stm = 0;
	m_reg_a = 0;
	m_reg_b = 0;
	m_pcr = 0;
	m_mask = 0;
	m_shift = 0;
	m_index = 0;
	m_fc = 0;
	m_bc = 0;
	m_tc = 0;
	m_csr_r = 0;

	m_ica_timer->adjust(screen().time_until_pos(0, 0));
	m_dca_timer->adjust(screen().time_until_pos(32, 784));
}

// default address map
void scc66470_device::scc66470_vram(address_map &map)
{
	if(!has_configured_map(0))
	{
		map(0x00000000, 0x0017ffff).ram();
	}
}

device_memory_interface::space_config_vector scc66470_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


void scc66470_device::set_vectors(uint16_t *src)
{
	for(int i = 0 ; i < 4 ; i++)
	{
		dram_w(i, *src++, 0xffff);
	}
}


void scc66470_device::dram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	space().write_word(offset<<1, data, mem_mask);
}

uint16_t scc66470_device::dram_r(offs_t offset, uint16_t mem_mask)
{
	return space().read_word(offset<<1, mem_mask);
}

inline uint8_t scc66470_device::dram_byte_r(offs_t offset)
{
	return space().read_byte(offset);
}

void scc66470_device::csr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_csr);
}

void scc66470_device::dcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dcr);
}

void scc66470_device::vsr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vsr);
}

void scc66470_device::bcr_w(offs_t offset, uint8_t data)
{
	m_bcr = data;
}

void scc66470_device::dcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dcr2);
}

void scc66470_device::dcp_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dcp);
}

void scc66470_device::swm_w(offs_t offset, uint8_t data)
{
	m_swm = data | data << 8;
}

void scc66470_device::stm_w(offs_t offset, uint8_t data)
{
	m_stm = data;
}

void scc66470_device::reg_a_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_reg_a);
}

void scc66470_device::reg_b_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_reg_b);
	perform_pixac_op();
}

void scc66470_device::pcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pcr);

	if(!BIT(m_pcr, PIXAC_COL) || !BIT(m_pcr, PIXAC_4N) || !BIT(m_pcr, PIXAC_BIT))
	{
		logerror("unsuppported pixac %x\n", m_pcr);
	}
}

void scc66470_device::mask_w(offs_t offset, uint8_t data)
{
	m_mask = data;
}

void scc66470_device::shift_w(offs_t offset, uint8_t data)
{
	m_shift = data;
}

void scc66470_device::index_w(offs_t offset, uint8_t data)
{
	m_index = data;
}

void scc66470_device::fc_w(offs_t offset, uint8_t data)
{
	m_fc = data;
}

void scc66470_device::bc_w(offs_t offset, uint8_t data)
{
	m_bc = data;
}

void scc66470_device::tc_w(offs_t offset, uint8_t data)
{
	m_tc = data;
}

uint8_t scc66470_device::csr_r(offs_t offset)
{
	uint8_t val = m_csr_r;

	if(!machine().side_effects_disabled())
	{
		m_csr_r &= ~(CSR_R_IT1 | CSR_R_IT2);

		if(!m_irqcallback.isnull())
		{
			m_irqcallback(CLEAR_LINE);
		}
	}

	int scanline = screen().vpos();

	if(scanline >= total_height() - height())
	{
		val |= CSR_R_DA;
	}

	return val;
}

uint16_t scc66470_device::reg_b_r(offs_t offset, uint16_t mem_mask)
{
	return m_reg_b & mem_mask;
}

int scc66470_device::dram_dtack_cycles()
{
	const int slot_cycle = (int)(machine().time().as_ticks(clock()) & 0xf);

	if(slot_cycle == 9)
	{
		return 2;
	}
	else if(slot_cycle < 9)
	{
		return 2 + 9 - slot_cycle;
	}
	else
	{
		return 2 + 9 + 16 - slot_cycle;
	}
}

void scc66470_device::map(address_map &map)
{
	map(0x000000, 0x17ffff).rw(FUNC(scc66470_device::dram_r), FUNC(scc66470_device::dram_w));
	map(0x1fffe0, 0x1fffe1).w(FUNC(scc66470_device::csr_w));
	map(0x1fffe1, 0x1fffe1).r(FUNC(scc66470_device::csr_r));
	map(0x1fffe2, 0x1fffe3).w(FUNC(scc66470_device::dcr_w));
	map(0x1fffe4, 0x1fffe5).w(FUNC(scc66470_device::vsr_w));
	map(0x1fffe7, 0x1fffe7).w(FUNC(scc66470_device::bcr_w));
	map(0x1fffe8, 0x1fffe9).w(FUNC(scc66470_device::dcr2_w));
	map(0x1fffea, 0x1fffeb).w(FUNC(scc66470_device::dcp_w));
	map(0x1fffec, 0x1fffec).w(FUNC(scc66470_device::swm_w));
	map(0x1fffef, 0x1fffef).w(FUNC(scc66470_device::stm_w));
	map(0x1ffff0, 0x1ffff1).w(FUNC(scc66470_device::reg_a_w));
	map(0x1ffff2, 0x1ffff3).rw(FUNC(scc66470_device::reg_b_r), FUNC(scc66470_device::reg_b_w));
	map(0x1ffff4, 0x1ffff5).w(FUNC(scc66470_device::pcr_w));
	map(0x1ffff7, 0x1ffff7).w(FUNC(scc66470_device::mask_w));
	map(0x1ffff8, 0x1ffff8).w(FUNC(scc66470_device::shift_w));
	map(0x1ffffb, 0x1ffffb).w(FUNC(scc66470_device::index_w));
	map(0x1ffffc, 0x1ffffc).w(FUNC(scc66470_device::fc_w));
	map(0x1ffffd, 0x1ffffd).w(FUNC(scc66470_device::bc_w));
	map(0x1ffffe, 0x1ffffe).w(FUNC(scc66470_device::tc_w));
}

int scc66470_device::pixac_trigger()
{
	if(BIT(m_pcr, PIXAC_COL))
	{
		return 1;
	}
	else if(BIT(m_pcr, PIXAC_CPY) && !BIT(m_pcr, PIXAC_TT))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void scc66470_device::perform_pixac_op()
{
	if(BIT(m_pcr, PIXAC_COL) && BIT(m_pcr, PIXAC_BIT))
	{
		if(BIT(m_pcr, PIXAC_4N))
		{
			uint16_t result = m_reg_b;
			m_index &= 0xf;

			if(BIT(m_reg_a, 15 - m_index))
			{
				result = (result & 0xff) | m_fc << 8;
			}
			else if(!BIT(m_pcr, PIXAC_TT))
			{
				result = (result & 0xff) | m_bc << 8;
			}

			m_index++;

			m_index &= 0xf;

			if(BIT(m_reg_a, 15 - m_index))
			{
				result = (result & 0xff00) | m_fc;
			}
			else if(!BIT(m_pcr, PIXAC_TT))
			{
				result = (result & 0xff00) | m_bc;
			}

			m_index++;

			m_reg_b = (m_reg_b & ~m_swm) | (result & m_swm);
		}
	}
}


uint16_t scc66470_device::ipa_r(offs_t offset, uint16_t mem_mask)
{
	if(offset < 0x180000 / 2)
	{
		if(pixac_trigger())
		{
			reg_b_w(0, dram_r(offset, 0xffff), 0xffff);
		}
	}

	return 0;
}

void scc66470_device::ipa_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(offset < 0x180000 / 2)
	{
		if(pixac_trigger())
		{
			dram_w(offset, m_reg_b, 0xffff);
		}
	}
}

bool scc66470_device::display_enabled()
{
	if(BIT(DCR_REG, DCR_DE))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void scc66470_device::set_vsr(uint32_t vsr)
{
	m_dcr = (m_dcr  & 0xfff0) | ((vsr >> 16) & 0x0f);
	m_vsr = vsr;
}

uint32_t scc66470_device::get_vsr()
{
	return ((m_dcr & 0xf) << 16) | m_vsr;
}

uint32_t scc66470_device::get_dcp()
{
	return ((m_dcr2 & 0x0f) << 16) | m_dcp;
}

void scc66470_device::set_dcp(uint32_t dcp)
{
	m_dcr2 = (m_dcr2 & 0xfff0) | (dcp & 0x0f);
	m_dcp = dcp;
}

void scc66470_device::line(int line, uint8_t *line_buffer, unsigned line_buffer_size)
{
	if(line_buffer_size == width())
	{
		if(display_enabled() && line >= (total_height() - height()))
		{
			uint8_t bc = m_bcr;

			if(BIT(DCR_REG, DCR_CM))  //4 bits/pixel
			{
				bc = bc >> 4;
			}

			line -= total_height() - height();

			if(line < border_height() || line >= (height() - border_height()))
			{
				std::fill_n(line_buffer, width(), bc);

				if(line == height() - 1)
				{
					set_vsr(0);
				}
			}
			else
			{
				unsigned vsr = get_vsr() & 0xfffff;

				if(vsr)
				{
					line_buffer = std::fill_n(line_buffer, border_width(), bc);

					if(BIT(DCR_REG, DCR_CM))  //4 bits/pixel
					{
						if(!BIT(DCR2_REG, DCR2_FT1))
						{
							for(int i = 0 ; i < width() - (border_width() * 2) ; i += 2)
							{
								uint8_t pixels = dram_byte_r(vsr++);
								*line_buffer++ = (pixels >> 4) & 0x0f;
								*line_buffer++ = (pixels) & 0x0f;
								vsr &= 0xfffff;
							}
						}
					}
					else
					{
						if(!BIT(DCR2_REG, DCR2_FT1))
						{
							for(int i = 0 ; i < width() - (border_width() * 2) ; i += 2)
							{
								const uint8_t pixel = dram_byte_r(vsr++);
								*line_buffer++ = pixel;
								*line_buffer++ = pixel;
								vsr &= 0xfffff;
							}
						}
					}

					std::fill_n(line_buffer, border_width(), bc);

					if(BIT(DCR_REG, DCR_LS))
					{
						vsr = get_vsr() + 512;

						if(BIT(DCR_REG, DCR_IC))
						{
							m_working_dcp = vsr - 64;
						}
						else
						{
							m_working_dcp = vsr - 16;
						}
					}
					else
					{
						if(!BIT(DCR2_REG, DCR2_ID))
						{
							if(BIT(DCR_REG, DCR_DC))
							{
								m_working_dcp = vsr;

								if(BIT(DCR_REG, DCR_IC))
								{
									vsr += 64;
								}
								else
								{
									vsr += 16;
								}
							}
						}
					}

					set_vsr(vsr);
				}
				else
				{
					std::fill_n(line_buffer, line_buffer_size, 0);
				}
			}
		}
		else
		{
			std::fill_n(line_buffer, line_buffer_size, 0);
		}
	}
	else
	{
		std::fill_n(line_buffer, line_buffer_size, 0);
	}
}

TIMER_CALLBACK_MEMBER(scc66470_device::process_ica)
{
	uint32_t ctrl = 0x400 / 2;

	if((BIT(DCR_REG, DCR_IC) || BIT(DCR_REG, DCR_DC)) && dram_r(ctrl, 0xffff) != 0)
	{
		bool stop = false;
		while(!stop)
		{
			uint16_t cmd = dram_r(ctrl++, 0xffff);
			uint16_t data = dram_r(ctrl++, 0xffff);

			ctrl &= 0xffffe;

			switch(cmd & 0xff00)
			{
				case SCC_INS_STOP:
					set_vsr((ctrl - 2) * 2);
					stop = true;
					break;

				case SCC_INS_NOP:
					break;

				case SCC_INS_LOAD_DCP:
					set_dcp(((cmd & 0xf) << 16) | (data & 0xfc));
					break;

				case SCC_INS_LOAD_DCP_STOP:
					set_dcp(((cmd & 0xf) << 16) | (data & 0xfc));
					stop = true;
					break;

				case SCC_INS_LOAD_VSR:
					ctrl = (((cmd & 0xf) << 16) | data) / 2;
					ctrl &= 0xffffe;
					break;

				case SCC_INS_LOAD_VSR_STOP:
					ctrl = ((cmd & 0xf) << 16) | data;
					ctrl &= 0xffffe;
					set_vsr(ctrl);
					stop = true;
					break;

				case SCC_INS_INTERRUPT:
					m_csr_r |= CSR_R_IT1;

					if(!BIT(CSR_REG, CSR_DI1))
					{
						if(!m_irqcallback.isnull())
						{
							m_irqcallback(ASSERT_LINE);
						}
					}

					break;

				default:
					if((cmd & 0xf800) == SCC_INS_LOAD_BORDER)
					{
						m_bcr = cmd & 0xff;
					}
					else if((cmd & 0xf800) == SCC_INS_LOAD_BORDER_DSP)
					{
						m_bcr = cmd & 0xff;
						m_dcr2 = (m_dcr2 & 0x90ff) | (data & 0x6f) << 8;
						m_dcr = (m_dcr & 0xfaff) | (data & 0x0500);
					}
					else if(cmd & SCC_INS_BEP_CONTROL)
					{
						//need to implement ?
					}
					else
					{
						logerror("Unknown ica/dca instruction %x %x %x\n",cmd,data,ctrl);
					}
			}
		}
	}

	m_working_dcp = 0;

	if(BIT(DCR_REG, DCR_DC) && BIT(DCR2_REG, DCR2_ID))
	{
		m_working_dcp = get_dcp();
	}

	m_ica_timer->adjust(screen().time_until_pos(0, 0));
}

TIMER_CALLBACK_MEMBER(scc66470_device::process_dca)
{
	uint32_t ctrl = (m_working_dcp / 2) & 0xffffe;

	if(BIT(DCR_REG, DCR_DC) && ctrl)
	{
		bool stop = false;
		bool new_dcp = false;
		uint32_t count;

		if(!BIT(DCR_REG, DCR_IC))
		{
			count = 16;
		}
		else
		{
			count = 64;
		}

		while(!stop && count)
		{
			uint16_t cmd = dram_r(ctrl++, 0xffff);
			uint16_t data = dram_r(ctrl++, 0xffff);

			ctrl &= 0xffffe;

			count -= 4;

			switch(cmd & 0xff00)
			{
				case SCC_INS_STOP:
					stop = true;
					break;

				case SCC_INS_NOP:
					break;

				case SCC_INS_LOAD_DCP:
					m_working_dcp = ((cmd & 0xf) << 16) | (data & 0xfc);
					set_dcp(m_working_dcp);
					ctrl = m_working_dcp / 2;
					ctrl &= 0xffffe;
					break;

				case SCC_INS_LOAD_DCP_STOP:
					m_working_dcp = ((cmd & 0xf) << 16) | (data & 0xfc);
					set_dcp(m_working_dcp);
					stop = true;
					new_dcp = true;
					break;

				case SCC_INS_LOAD_VSR:
					set_vsr(((cmd & 0xf) << 16) | data);
					break;

				case SCC_INS_LOAD_VSR_STOP:
					set_vsr(((cmd & 0xf) << 16) | data);
					stop = true;
					break;

				case SCC_INS_INTERRUPT:
					m_csr_r |= CSR_R_IT1;

					if(!BIT(CSR_REG, CSR_DI1))
					{
						if(!m_irqcallback.isnull())
						{
							m_irqcallback(ASSERT_LINE);
						}
					}

					break;

				default:
					if((cmd & 0xf800) == SCC_INS_LOAD_BORDER)
					{
						m_bcr = cmd & 0xff;
					}
					else if((cmd & 0xf800) == SCC_INS_LOAD_BORDER_DSP)
					{
						m_bcr = cmd & 0xff;
						m_dcr2 = (m_dcr2 & 0x90ff) | (data & 0x6f) << 8;
						m_dcr = (m_dcr & 0xfaff) | (data & 0x0500);
					}
					else if(cmd & SCC_INS_BEP_CONTROL)
					{
						//need to implement ?
					}
					else
					{
						logerror("Unknown ica instruction %x %x %x\n",cmd,data,ctrl);
					}
			}
		}

		ctrl *= 2;

		if(!new_dcp)
		{
			ctrl += count;
		}

		if(BIT(DCR_REG, DCR_DC) && BIT(DCR2_REG, DCR2_ID))
		{
			m_working_dcp = ctrl;
		}
		else
		{
			m_working_dcp = 0;
		}
	}

	int scanline = screen().vpos();

	if(scanline == total_height() - border_height() - 1)
	{
		m_csr_r ^= CSR_R_PA;

		m_dca_timer->adjust(screen().time_until_pos(total_height() - height() + border_height(), 784));
	}
	else
	{
		m_dca_timer->adjust(screen().time_until_pos(scanline + 1, 784));
	}
}

unsigned scc66470_device::border_width()
{
	const unsigned hoffset = (BIT(DCR_REG, DCR_CF1) << 3) | (BIT(DCR_REG, DCR_CF2) << 2) | (BIT(DCR_REG, DCR_SS) << 1) | BIT(CSR_REG, CSR_ST);
	return h_table[ hoffset ].border / 2;
}

int scc66470_device::border_height()
{
	int height = 0;

	if(!BIT(DCR_REG, DCR_FD) && BIT(CSR_REG, CSR_ST))
	{
		height = 20;
	}

	if(!BIT(DCR_REG, DCR_SS))
	{
		height += 15;
	}

	return height;
}

unsigned scc66470_device::width()
{
	const unsigned hoffset = (BIT(DCR_REG, DCR_CF1) << 3) | (BIT(DCR_REG, DCR_CF2) << 2) | (BIT(DCR_REG, DCR_SS) << 1) | BIT(CSR_REG, CSR_ST);
	return h_table[ hoffset ].pixels;
}

unsigned scc66470_device::height()
{
	if(BIT(DCR_REG, DCR_FD))
	{
		return 240;
	}
	else
	{
		return 280;
	}
}

unsigned scc66470_device::total_height()
{
	if(BIT(DCR_REG, DCR_FD))
	{
		return 262;
	}
	else
	{
		return 312;
	}
}
