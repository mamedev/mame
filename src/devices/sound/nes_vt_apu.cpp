// license:BSD-3-Clause
// copyright-holders:David Shah
/*****************************************************************************

  MAME/MESS VTxx APU CORE

 *****************************************************************************/

#include "nes_vt_apu.h"
#include "emu.h"

#define VTXX_NTSC_XTAL XTAL(21'477'272)
#define VTXX_NTSC_APU_CLOCK (VTXX_NTSC_XTAL/12)

DEFINE_DEVICE_TYPE(NES_VT_APU, nesapu_vt_device, "nesapu_vt", "VTxx APU")
DEFINE_DEVICE_TYPE(NES_VT_APU_SLAVE, nesapu_vt_slave_device, "nesapu_vt_slave", "VTxx APU (slave)")

nesapu_vt_device::nesapu_vt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nesapu_device(mconfig, tag, NES_VT_APU, owner, clock),
	  m_xop2(*this, "nesapu_vt_slave"),
	  m_rom_read_cb(*this)
{

}

void nesapu_vt_device::device_start()
{
	apu_init();

	if(!m_xop2->started())
		throw device_missing_dependencies();
	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_apu_vt.vt33_pcm[i].regs), i);
		save_item(NAME(m_apu_vt.vt33_pcm[i].address), i);
		save_item(NAME(m_apu_vt.vt33_pcm[i].volume), i);
		save_item(NAME(m_apu_vt.vt33_pcm[i].enabled), i);
		save_item(NAME(m_apu_vt.vt33_pcm[i].playing), i);
	}

	save_item(NAME(m_apu_vt.vt03_pcm.phaseacc));
	save_item(NAME(m_apu_vt.vt03_pcm.regs));
	save_item(NAME(m_apu_vt.vt03_pcm.address));
	save_item(NAME(m_apu_vt.vt03_pcm.length));
    save_item(NAME(m_apu_vt.vt03_pcm.remaining_bytes));
	save_item(NAME(m_apu_vt.vt03_pcm.enabled));
	save_item(NAME(m_apu_vt.vt03_pcm.vol));

	save_item(NAME(m_apu_vt.extra_regs));
	save_item(NAME(m_apu_vt.vt3x_sel_channel));
	save_item(NAME(m_apu_vt.use_vt03_pcm));
	save_item(NAME(m_apu_vt.use_vt3x_pcm));

}

nesapu_vt_slave_device::nesapu_vt_slave_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nesapu_device(mconfig, tag, NES_VT_APU_SLAVE, owner, clock)
{

}

void nesapu_vt_slave_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	//TODO: any tweaks needed here:
	nesapu_device::sound_stream_update(stream, inputs, outputs, samples);
}

void nesapu_vt_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	std::unique_ptr<stream_sample_t[]> pbuf = std::make_unique<stream_sample_t[]>(samples);
	std::unique_ptr<stream_sample_t[]> sbuf = std::make_unique<stream_sample_t[]>(samples);

	stream_sample_t *pptr = pbuf.get(), *sptr = sbuf.get();

	// Dual legacy sound generators
	nesapu_device::sound_stream_update(stream, inputs, &pptr, samples);
	m_xop2->sound_stream_update(stream, inputs, &sptr, samples);

	int accum;
	memset( outputs[0], 0, samples*sizeof(*outputs[0]) );

	for(int i = 0; i < samples; i++)
	{
		accum = 0;
		accum += pbuf[i] >> 8;
		accum += sbuf[i] >> 8; //TODO: mixing between generators?
		accum += vt03_pcm(&m_apu_vt.vt03_pcm);
		accum += vt3x_pcm(&(m_apu_vt.vt33_pcm[0]));
		accum += vt3x_pcm(&(m_apu_vt.vt33_pcm[1]));

		/* 8-bit clamps */
		if (accum > 127)
			accum = 127;
		else if (accum < -128)
			accum = -128;

		*(outputs[0]++)=accum<<8;

	}
}

s8 nesapu_vt_device::vt03_pcm(apu_vt_t::vt03_pcm_t *ch) {
    if (ch->enabled) {
    	int freq = dpcm_clocks[ch->regs[0] & 0x0F];
		ch->phaseacc -= 4;
		while (ch->phaseacc < 0)
		{
			ch->phaseacc += freq;
	        u8 data = u8(m_mem_read_cb(ch->address));
	        logerror("pcm fetch %04x %d\n", ch->address, int(data));
	        ch->vol = data;
	        ch->address++;
	        ch->remaining_bytes--;
	        if (ch->remaining_bytes == 0) {
	            if (ch->regs[0] & 0x40) {
	                reset_vt03_pcm(ch);
	            } else {
	                ch->enabled = false;
	            }
	        }
	    }
        return ch->vol - 128;
    } else {
        return 0;
    }
}

s8 nesapu_vt_device::vt3x_pcm(apu_vt_t::vt3x_pcm_t *ch) {
	if (ch->enabled && ch->playing) {
		uint8_t sample = m_rom_read_cb(ch->address);
		if (sample == 0xFF) {
			ch->playing = false;
		} else {
			ch->address++;
			return s8(s8(sample) * s16(ch->volume) / 255);
		}
	}
	return 0;
}

void nesapu_vt_device::start_vt3x_pcm(apu_vt_t::vt3x_pcm_t *ch) {
	ch->enabled = true;
	ch->playing = true;
	ch->address = (uint32_t(ch->regs[3] & 0x0F) << 21) | (uint32_t(ch->regs[2] & 0x7F) << 14) || (uint32_t(ch->regs[1]) << 6);
}


void nesapu_vt_device::reset_vt03_pcm(apu_vt_t::vt03_pcm_t *ch) {
	logerror("reset vt03 pwm\n");
    ch->address = ((~m_apu_vt.extra_regs[0]) & 0x03) << 14 | m_apu_vt.vt03_pcm.regs[2] << 6;
    ch->remaining_bytes = m_apu_vt.vt03_pcm.regs[3] << 4;
    ch->length = m_apu_vt.vt03_pcm.regs[3] << 4;
    ch->enabled = true;
}

void nesapu_vt_device::vt_apu_write(uint8_t address, uint8_t data) {

	if(address == 0x35 && !m_apu_vt.use_vt3x_pcm)
	{
		//When VT3x PCM disabled, 4035 controls XOP2
		m_xop2->write(0x15, data & 0x0F);
        m_apu_vt.extra_regs[0x05] = data;
    } else if (address >= 0x10 && address <= 0x13) {
        m_apu_vt.vt03_pcm.regs[address - 0x10] = data;
		if (m_apu_vt.use_vt3x_pcm && address == 0x12) {
			m_apu_vt.vt33_pcm[m_apu_vt.vt3x_sel_channel].regs[1] = data;
		}
	} else if(address >= 0x30 && address <= 0x36) {
        if (address == 0x30) {
            m_apu_vt.use_vt03_pcm = (data & 0x10) != 0;
		} else if (address == 0x33) {
			m_apu_vt.use_vt3x_pcm = (data & 0x80);
			m_apu_vt.vt33_pcm[0].enabled = (data & 0x10);
			m_apu_vt.vt33_pcm[1].enabled = (data & 0x08);
		} else if (address == 0x31) {
			m_apu_vt.vt3x_sel_channel = 0;
			m_apu_vt.vt33_pcm[0].volume = data;
			m_apu_vt.vt33_pcm[m_apu_vt.vt3x_sel_channel].regs[0] = data;
		} else if (address == 0x32) {
			m_apu_vt.vt3x_sel_channel = 1;
			m_apu_vt.vt33_pcm[1].volume = data;
			m_apu_vt.vt33_pcm[m_apu_vt.vt3x_sel_channel].regs[0] = data;
		} else if (address == 0x35) {
			m_apu_vt.vt33_pcm[m_apu_vt.vt3x_sel_channel].regs[2] = data;
		} else if (address == 0x36) {
			m_apu_vt.vt33_pcm[m_apu_vt.vt3x_sel_channel].regs[3] = data;
			start_vt3x_pcm(&(m_apu_vt.vt33_pcm[m_apu_vt.vt3x_sel_channel]));
		}

		m_apu_vt.extra_regs[address - 0x30] = data;
	} else if (address == 0x15) {
		uint8_t nes_val = data;
		if(m_apu_vt.use_vt03_pcm || m_apu_vt.use_vt3x_pcm)
		{
            if (m_apu_vt.use_vt03_pcm) {
            	if (nes_val & 0x10)
                	reset_vt03_pcm(&m_apu_vt.vt03_pcm);
                else
                	m_apu_vt.vt03_pcm.enabled = false;
            }

			nes_val &= 0x0F;
		}
		nesapu_device::write(0x15, nes_val);
	}
}

uint8_t nesapu_vt_device::vt_apu_read(uint8_t address) {
	if(address >= 0x30 && address <= 0x36)
	{
		return m_apu_vt.extra_regs[address - 0x30];
	} else if (address >= 0x10 && address <= 0x13) {
		return m_apu_vt.vt03_pcm.regs[address - 0x10];
	} else if (address == 0x15) {
		uint8_t base = nesapu_device::read(0x15);
		if(m_apu_vt.use_vt03_pcm)
		{
			base &= 0x4F;
			base |= (m_apu_vt.vt03_pcm.enabled << 4);
			// TODO: IRQ status
		}
		return base;
	}
	return 0x00;
}



u8 nesapu_vt_device::read(offs_t address)
{
	if (address <= 0x0F) {
		return nesapu_device::read(address);
	} else if (address >= 0x10 && address <= 0x13) {
		if(!m_apu_vt.use_vt03_pcm)
			return nesapu_device::read(address);
		else
			return vt_apu_read(address);
	} else if (address >= 0x20 && address <= 0x2F) {
		return m_xop2->read(address - 0x20);
	} else if (address == 0x15 || (address >= 0x30 && address <= 0x36)) {
		return vt_apu_read(address);
	} else {
		logerror("nesapu_vt read %04x\n", 0x4000 + address);
		return 0x00;
	}
}

void nesapu_vt_device::write(offs_t address, u8 value)
{
	logerror("nesapu_vt write %04x %02x\n", 0x4000 + address, value);
	if (address <= 0x0F) {
		nesapu_device::write(address, value);
	} else if (address >= 0x10 && address <= 0x13) {
		//PCM registers affect both new and legacy APU
		if (m_apu_vt.use_vt03_pcm || m_apu_vt.use_vt3x_pcm)
			vt_apu_write(address, value);
		else
			nesapu_device::write(address, value);
	} else if (address >= 0x20 && address <= 0x2F) {
		m_xop2->write(address - 0x20, value);
	} else if (address == 0x15 || (address >= 0x30 && address <= 0x36)) {
		vt_apu_write(address, value);
	} else {
		logerror("nesapu_vt write %04x %02x\n", 0x4000 + address, value);
	}
}

MACHINE_CONFIG_START(nesapu_vt_device::device_add_mconfig)
	MCFG_DEVICE_ADD("nesapu_vt_slave", NES_VT_APU_SLAVE, VTXX_NTSC_APU_CLOCK)
MACHINE_CONFIG_END