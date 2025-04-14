// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Acclaim RAX Sound Board

****************************************************************************/

#include "emu.h"
#include "rax.h"
#include "speaker.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

// These are some of the control registers. We don't use them all
enum
{
	IDMA_CONTROL_REG = 0,   // 3fe0
	BDMA_INT_ADDR_REG,      // 3fe1
	BDMA_EXT_ADDR_REG,      // 3fe2
	BDMA_CONTROL_REG,       // 3fe3
	BDMA_WORD_COUNT_REG,    // 3fe4
	PROG_FLAG_DATA_REG,     // 3fe5
	PROG_FLAG_CONTROL_REG,  // 3fe6

	S1_AUTOBUF_REG = 15,    // 3fef
	S1_RFSDIV_REG,          // 3ff0
	S1_SCLKDIV_REG,         // 3ff1
	S1_CONTROL_REG,         // 3ff2
	S0_AUTOBUF_REG,         // 3ff3
	S0_RFSDIV_REG,          // 3ff4
	S0_SCLKDIV_REG,         // 3ff5
	S0_CONTROL_REG,         // 3ff6
	S0_MCTXLO_REG,          // 3ff7
	S0_MCTXHI_REG,          // 3ff8
	S0_MCRXLO_REG,          // 3ff9
	S0_MCRXHI_REG,          // 3ffa
	TIMER_SCALE_REG,        // 3ffb
	TIMER_COUNT_REG,        // 3ffc
	TIMER_PERIOD_REG,       // 3ffd
	WAITSTATES_REG,         // 3ffe
	SYSCONTROL_REG          // 3fff
};


/*************************************
 *
 *  Interface
 *
 *************************************/

void acclaim_rax_device::data_w(uint16_t data)
{
	m_data_in->write(data);
	m_cpu->set_input_line(ADSP2181_IRQL0, ASSERT_LINE);
	machine().scheduler().perfect_quantum(attotime::from_usec(5));
}


uint16_t acclaim_rax_device::data_r()
{
	if (!machine().side_effects_disabled())
		m_adsp_snd_pf0 = 1;
	return m_data_out->read();
}


/*************************************
 *
 *  Internal
 *
 *************************************/

uint16_t acclaim_rax_device::adsp_control_r(offs_t offset)
{
	uint16_t res = 0;

	switch (offset)
	{
		case PROG_FLAG_DATA_REG:
			res = m_adsp_snd_pf0;
			break;
		default:
			res = m_control_regs[offset];
	}

	return res;
}

void acclaim_rax_device::adsp_control_w(offs_t offset, uint16_t data)
{
	m_control_regs[offset] = data;

	switch (offset)
	{
		case 0x1:
			m_control_regs[BDMA_INT_ADDR_REG] = data & 0x3fff;
			break;
		case 0x2:
			m_control_regs[BDMA_EXT_ADDR_REG] = data & 0x3fff;
			break;
		case 0x3:
			m_control_regs[BDMA_CONTROL_REG] = data & 0xff0f;
			break;

		case 0x4:
		{
			m_control_regs[BDMA_WORD_COUNT_REG] = data & 0x3fff;

			uint8_t const *const adsp_rom = &m_rom[m_rom_bank * 0x400000];

			uint32_t const page = (m_control_regs[BDMA_CONTROL_REG] >> 8) & 0xff;
			uint32_t const dir = BIT(m_control_regs[BDMA_CONTROL_REG], 2);
			uint32_t const type = m_control_regs[BDMA_CONTROL_REG] & 3;
			uint32_t src_addr = (page << 14) | m_control_regs[BDMA_EXT_ADDR_REG];

			uint32_t count = m_control_regs[BDMA_WORD_COUNT_REG];

			address_space* addr_space = (type == 0 ? m_program : m_data);

			if (!dir)
			{
				if (type == 0)
				{
					while (count)
					{
						uint32_t const src_dword = (adsp_rom[src_addr + 0] << 16) | (adsp_rom[src_addr + 1] << 8) | adsp_rom[src_addr + 2];

						addr_space->write_dword(m_control_regs[BDMA_INT_ADDR_REG], src_dword);

						src_addr += 3;
						++m_control_regs[BDMA_INT_ADDR_REG];
						--count;
					}
				}
				else if (type == 1)
				{
					while (count)
					{
						uint16_t const src_word = (adsp_rom[src_addr + 0] << 8) | adsp_rom[src_addr + 1];

						addr_space->write_word(m_control_regs[BDMA_INT_ADDR_REG], src_word);

						src_addr += 2;
						++m_control_regs[BDMA_INT_ADDR_REG];
						--count;
					}
				}
				else
				{
					int const shift = type == 2 ? 8 : 0;

					while (count)
					{
						uint16_t const src_word = adsp_rom[src_addr] << shift;

						addr_space->write_word(m_control_regs[BDMA_INT_ADDR_REG], src_word);

						++src_addr;
						++m_control_regs[BDMA_INT_ADDR_REG];
						--count;
					}
				}
			}
			else
			{
				// TODO: last stage in Batman Forever!?
				// page = 0, dir = 1, type = 1, src_addr = 0xfd
				fatalerror("%s DMA to byte memory!",this->tag());
			}

			attotime const word_period = attotime::from_hz(m_cpu->unscaled_clock());
			attotime const period = word_period * (data & 0x3fff) * 1;
			m_dma_timer->adjust(period, src_addr, period);

			break;
		}

		case S1_AUTOBUF_REG:
			// autobuffer off: nuke the timer, and disable the DAC
			if (BIT(~data, 1))
			{
				m_dmadac[1]->enable(0);
			}
			break;

		case S0_AUTOBUF_REG:
			// autobuffer off: nuke the timer, and disable the DAC
			if (BIT(~data, 1))
			{
				m_dmadac[0]->enable(0);
				m_reg_timer->reset();
			}
			break;

		case S1_CONTROL_REG:
			if (((data >> 4) & 3) == 2)
				fatalerror("RAX: Oh no!, the data is compressed with u-law encoding\n");
			if (((data >> 4) & 3) == 3)
				fatalerror("RAX: Oh no!, the data is compressed with A-law encoding\n");
			break;

		case PROG_FLAG_DATA_REG:
			logerror("PFLAGS: %x\n", data);
			break;
		case PROG_FLAG_CONTROL_REG:
			logerror("PFLAG CTRL: %x\n", data);
			break;
		default:
			logerror("Unhandled register: %x %x\n", 0x3fe0 + offset, data);
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(acclaim_rax_device::dma_timer_callback)
{
	// Update external address count and page
	m_control_regs[BDMA_WORD_COUNT_REG] = 0;
	m_control_regs[BDMA_EXT_ADDR_REG] = param & 0x3fff;
	m_control_regs[BDMA_CONTROL_REG] &= ~0xff00;
	m_control_regs[BDMA_CONTROL_REG] |= ((param >> 14) & 0xff) << 8;

	if (BIT(m_control_regs[BDMA_CONTROL_REG], 3))
		m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	else
		m_cpu->pulse_input_line(ADSP2181_BDMA, m_cpu->minimum_quantum_time());

	timer.adjust(attotime::never);
}


void acclaim_rax_device::update_data_ram_bank()
{
	if (m_dmovlay_val == 0)
		m_adsp_data_bank->set_entry(0);
	else
		m_adsp_data_bank->set_entry(1 + m_data_bank);
}

void acclaim_rax_device::ram_bank_w(uint16_t data)
{
	// Note: The PCB has two unstuffed RAM locations
	m_data_bank = data & 3;
	update_data_ram_bank();
}

void acclaim_rax_device::rom_bank_w(uint16_t data)
{
	m_rom_bank = data;
}

uint16_t acclaim_rax_device::host_r()
{
	if (!machine().side_effects_disabled())
		m_cpu->set_input_line(ADSP2181_IRQL0, CLEAR_LINE);
	return m_data_in->read();
}

void acclaim_rax_device::host_w(uint16_t data)
{
	m_data_out->write(data);
	m_adsp_snd_pf0 = 0;
}


/*************************************
 *
 *  CPU memory map & config
 *
 *************************************/

void acclaim_rax_device::adsp_program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).ram().share(m_adsp_pram);
}

void acclaim_rax_device::adsp_data_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).bankrw(m_adsp_data_bank);
	map(0x2000, 0x3fdf).ram(); // Internal RAM
	map(0x3fe0, 0x3fff).rw(FUNC(acclaim_rax_device::adsp_control_r), FUNC(acclaim_rax_device::adsp_control_w));
}

void acclaim_rax_device::adsp_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).w(FUNC(acclaim_rax_device::ram_bank_w));
	map(0x0001, 0x0001).w(FUNC(acclaim_rax_device::rom_bank_w));
	map(0x0003, 0x0003).rw(FUNC(acclaim_rax_device::host_r), FUNC(acclaim_rax_device::host_w));
}


void acclaim_rax_device::device_start()
{
	m_program = &m_cpu->space(AS_PROGRAM);
	m_data = &m_cpu->space(AS_DATA);

	// 1 bank for internal
	m_banked_ram = make_unique_clear<uint16_t[]>(0x2000 * 5);
	m_adsp_data_bank->configure_entries(0, 5, &m_banked_ram[0], 0x2000 * sizeof(uint16_t));

	save_item(NAME(m_adsp_snd_pf0));
	save_item(NAME(m_adsp_regs.bdma_internal_addr));
	save_item(NAME(m_adsp_regs.bdma_external_addr));
	save_item(NAME(m_adsp_regs.bdma_control));
	save_item(NAME(m_adsp_regs.bdma_word_count));
	save_item(NAME(m_control_regs));
	save_item(NAME(m_size));
	save_item(NAME(m_incs));
	save_item(NAME(m_ireg));
	save_item(NAME(m_ireg_base));
	save_item(NAME(m_data_bank));
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_dmovlay_val));
}

void acclaim_rax_device::device_reset()
{
	// Load 32 program words (96 bytes) via BDMA
	for (int i = 0; i < 32; i ++)
	{
		uint32_t word = m_rom[i*3 + 0] << 16;
		word |= m_rom[i*3 + 1] << 8;
		word |= m_rom[i*3 + 2];

		m_adsp_pram[i] = word;
	}

	m_adsp_snd_pf0 = 1;
	m_rom_bank = 0;

	// initialize our state structure and install the transmit callback
	m_size[0] = 0;
	m_incs[0] = 0;
	m_ireg[0] = 0;

	// initialize the ADSP control regs
	memset(m_control_regs, 0, sizeof(m_control_regs));

	m_dmovlay_val = 0;
	m_data_bank = 0;
	update_data_ram_bank();
}

void acclaim_rax_device::device_post_load()
{
	update_data_ram_bank();
}


void acclaim_rax_device::adsp_irq(int which)
{
	if (which != 0)
		return;

	// get the index register
	int reg = m_cpu->state_int(ADSP2100_I0 + m_ireg[which]);

	// copy the current data into the buffer
	int const count = m_size[which] / (4 * (m_incs[which] ? m_incs[which] : 1));

	int16_t buffer[0x100]{};

	for (uint32_t i = 0; i < count; i++)
	{
		buffer[i] = m_data->read_word(reg);
		reg += m_incs[which];
	}

	for (int i = 0; i < 2; i++)
	{
		m_dmadac[i]->flush();
		m_dmadac[i]->transfer(i, 1, 2, count/2, buffer);
	}

	// check for wrapping
	if (reg >= m_ireg_base[which] + m_size[which])
	{
		// reset the base pointer
		reg = m_ireg_base[which];
	}

	m_cpu->set_state_int(ADSP2100_I0 + m_ireg[which], reg);
}

TIMER_DEVICE_CALLBACK_MEMBER(acclaim_rax_device::adsp_irq0)
{
	adsp_irq(0);
}



void acclaim_rax_device::recompute_sample_rate(int which)
{
	// calculate how long until we generate an interrupt

	// frequency the time per each bit sent
	attotime sample_period = attotime::from_hz(m_cpu->unscaled_clock()) * (1 * (m_control_regs[which ? S1_SCLKDIV_REG : S0_SCLKDIV_REG] + 1));

	// now put it down to samples, so we know what the channel frequency has to be
	sample_period = sample_period * (16 * 1);
	for (auto &dmadac : m_dmadac)
	{
		dmadac->set_frequency(sample_period.as_hz());
		dmadac->enable(1);
	}

	// fire off a timer which will hit every half-buffer
	if (m_incs[which])
	{
		attotime const period = (sample_period * m_size[which]) / (4 * 2 * m_incs[which]);
		m_reg_timer->adjust(period, 0, period);
	}
}

void acclaim_rax_device::adsp_sound_tx_callback(offs_t offset, uint32_t data)
{
	int const which = offset;

	if (which != 0)
		return;

	int const autobuf_reg = which ? S1_AUTOBUF_REG : S0_AUTOBUF_REG; // "which" must equal 0 here, invalid test: Coverity 315932

	// check if SPORT1 is enabled
	if (m_control_regs[SYSCONTROL_REG] & (which ? 0x0800 : 0x1000)) // bit 11 // invalid test here too
	{
		// we only support autobuffer here (which is what this thing uses), bail if not enabled
		if (m_control_regs[autobuf_reg] & 0x0002) // bit 1
		{
			// get the autobuffer registers
			m_ireg[which] = (m_control_regs[autobuf_reg] >> 9) & 7;
			int mreg = (m_control_regs[autobuf_reg] >> 7) & 3;
			mreg |= m_ireg[which] & 0x04; // msb comes from ireg
			int const lreg = m_ireg[which];

			// now get the register contents in a more legible format
			// we depend on register indexes to be continuous (which is the case in our core)
			uint16_t source = m_cpu->state_int(ADSP2100_I0 + m_ireg[which]);
			m_incs[which] = m_cpu->state_int(ADSP2100_M0 + mreg);
			m_size[which] = m_cpu->state_int(ADSP2100_L0 + lreg);

			// get the base value, since we need to keep it around for wrapping
			source -= m_incs[which];

			// make it go back one so we dont lose the first sample
			m_cpu->set_state_int(ADSP2100_I0 + m_ireg[which], source);

			// save it as it is now
			m_ireg_base[which] = source;

			// recompute the sample rate and timer
			recompute_sample_rate(which);
			return;
		}
		else
			logerror( "ADSP SPORT1: trying to transmit and autobuffer not enabled!\n" );
	}

	// if we get there, something went wrong. Disable playing
	for (auto &dmadac : m_dmadac)
		dmadac->enable(0);

	// remove timer
	m_reg_timer->reset();
}

void acclaim_rax_device::dmovlay_callback(uint32_t data)
{
	if (data > 1)
	{
		fatalerror("dmovlay_callback: Error! dmovlay called with value = %X\n", data);
	}
	else
	{
		m_dmovlay_val = data;
		update_data_ram_bank();
	}
}


DEFINE_DEVICE_TYPE(ACCLAIM_RAX, acclaim_rax_device, "rax_audio", "Acclaim RAX")

//-------------------------------------------------
//  acclaim_rax_device - constructor
//-------------------------------------------------

acclaim_rax_device::acclaim_rax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACCLAIM_RAX, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_cpu(*this, "adsp")
	, m_dmadac(*this, { "dacl", "dacr" })
	, m_reg_timer(*this, "adsp_reg_timer")
	, m_dma_timer(*this, "adsp_dma_timer")
	, m_adsp_pram(*this, "adsp_pram")
	, m_adsp_data_bank(*this, "databank")
	, m_rom(*this, DEVICE_SELF)
	, m_data_in(*this, "data_in")
	, m_data_out(*this, "data_out")
{

}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void acclaim_rax_device::device_add_mconfig(machine_config &config)
{
	ADSP2181(config, m_cpu, XTAL(16'670'000));
	m_cpu->sport_tx().set(FUNC(acclaim_rax_device::adsp_sound_tx_callback)); // callback for serial transmit
	m_cpu->dmovlay().set(FUNC(acclaim_rax_device::dmovlay_callback)); // callback for adsp 2181 dmovlay instruction
	m_cpu->set_addrmap(AS_PROGRAM, &acclaim_rax_device::adsp_program_map);
	m_cpu->set_addrmap(AS_DATA, &acclaim_rax_device::adsp_data_map);
	m_cpu->set_addrmap(AS_IO, &acclaim_rax_device::adsp_io_map);

	TIMER(config, m_reg_timer).configure_generic(FUNC(acclaim_rax_device::adsp_irq0));
	TIMER(config, m_dma_timer).configure_generic(FUNC(acclaim_rax_device::dma_timer_callback));

	GENERIC_LATCH_16(config, m_data_in);
	GENERIC_LATCH_16(config, m_data_out);

	SPEAKER(config, "speaker", 2).front();

	DMADAC(config, m_dmadac[0]).add_route(ALL_OUTPUTS, *this, 1.0, 0);
	DMADAC(config, m_dmadac[1]).add_route(ALL_OUTPUTS, *this, 1.0, 1);
}
