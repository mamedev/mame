// license:BSD-3-Clause
// copyright-holders:Carl

#include "emu.h"
#include "cpu/i86/i286.h"
#include "machine/at.h"
#include "sound/262intf.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "bus/isa/isa_cards.h"

class vis_audio_device : public device_t,
						 public device_isa16_card_interface
{
public:
	vis_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	uint8_t pcm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void dack16_w(int line, uint16_t data) override { if(m_samples < 2) m_sample[m_samples++] = data; else m_isa->drq7_w(CLEAR_LINE); }
	virtual machine_config_constructor device_mconfig_additions() const override;
private:
	required_device<dac_word_interface> m_rdac;
	required_device<dac_word_interface> m_ldac;
	uint16_t m_count;
	uint16_t m_sample[2];
	uint8_t m_index[2]; // unknown indexed registers, volume?
	uint8_t m_data[2][16];
	uint8_t m_mode;
	uint8_t m_stat;
	unsigned int m_sample_byte;
	unsigned int m_samples;
	emu_timer *m_pcm;
};

const device_type VIS_AUDIO = &device_creator<vis_audio_device>;

vis_audio_device::vis_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIS_AUDIO, "vis_pcm", tag, owner, clock, "vis_pcm", __FILE__),
	device_isa16_card_interface(mconfig, *this),
	m_rdac(*this, "rdac"),
	m_ldac(*this, "ldac")
{
}

void vis_audio_device::device_start()
{
	set_isa_device();
	m_isa->set_dma_channel(7, this, false);
	m_isa->install_device(0x0220, 0x022f, read8_delegate(FUNC(vis_audio_device::pcm_r), this), write8_delegate(FUNC(vis_audio_device::pcm_w), this));
	m_isa->install_device(0x0388, 0x038b, read8_delegate(FUNC(ymf262_device::read), subdevice<ymf262_device>("ymf262")), write8_delegate(FUNC(ymf262_device::write), subdevice<ymf262_device>("ymf262")));
	m_pcm = timer_alloc();
	m_pcm->adjust(attotime::never);
}

void vis_audio_device::device_reset()
{
	m_count = 0;
	m_sample_byte = 0;
	m_samples = 0;
	m_mode = 0;
	m_index[0] = m_index[1] = 0;
	m_stat = 0;
}

void vis_audio_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(m_mode & 0x88)
	{
		case 0x80: // 8bit mono
		{
			uint8_t sample = m_sample[m_sample_byte >> 1] >> ((m_sample_byte & 1) * 8);
			m_ldac->write(sample << 8);
			m_rdac->write(sample << 8);
			m_sample_byte++;
			break;
		}
		case 0x00: // 8bit stereo
			m_ldac->write(m_sample[m_sample_byte >> 1] << 8);
			m_rdac->write(m_sample[m_sample_byte >> 1] & 0xff00);
			m_sample_byte += 2;
			break;
		case 0x88: // 16bit mono
			m_ldac->write(m_sample[m_sample_byte >> 1] ^ 0x8000);
			m_rdac->write(m_sample[m_sample_byte >> 1] ^ 0x8000);
			m_sample_byte += 2;
			break;
		case 0x08: // 16bit stereo
			m_ldac->write(m_sample[0] ^ 0x8000);
			m_rdac->write(m_sample[1] ^ 0x8000);
			m_sample_byte += 4;
			break;
	}

	if(m_sample_byte >= 4)
	{
		m_sample_byte = 0;
		m_samples = 0;
		if(m_count)
			m_isa->drq7_w(ASSERT_LINE);
		else
		{
			m_ldac->write(0x8000);
			m_rdac->write(0x8000);
			m_stat = 4;
			m_pcm->adjust(attotime::never);
			m_isa->irq7_w(ASSERT_LINE);
		}
		m_count--;
	}
}

static MACHINE_CONFIG_FRAGMENT( vis_pcm_config )
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ymf262", YMF262, XTAL_14_31818MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(2, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(3, "rspeaker", 1.00)
	MCFG_SOUND_ADD("ldac", DAC_16BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0) // unknown DAC
	MCFG_SOUND_ADD("rdac", DAC_16BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "ldac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "ldac", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "rdac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "rdac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END

machine_config_constructor vis_audio_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vis_pcm_config );
}

uint8_t vis_audio_device::pcm_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	switch(offset)
	{
		case 0x00:
			return m_mode;
		case 0x02:
			return m_data[0][m_index[0]];
		case 0x04:
			return m_data[1][m_index[1]];
		case 0x09:
			m_isa->irq7_w(CLEAR_LINE);
			return m_stat;
		case 0x0c:
			return m_count & 0xff;
		case 0x0e:
			return m_count >> 8;
		default:
			logerror("unknown pcm read %04x\n", offset);
			break;
	}
	return 0;
}

void vis_audio_device::pcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	switch(offset)
	{
		case 0x00:
			m_mode = data;
			break;
		case 0x02:
			m_data[0][m_index[0] & 0xf] = data;
			break;
		case 0x03:
			m_index[0] = data;
			break;
		case 0x04:
			m_data[1][m_index[1] & 0xf] = data;
			break;
		case 0x05:
			m_index[1] = data;
			break;
		case 0x0c:
			m_count = (m_count & 0xff00) | data;
			break;
		case 0x0e:
			m_count = (m_count & 0xff) | (data << 8);
			break;
		default:
			logerror("unknown pcm write %04x %02x\n", offset, data);
			break;
	}
	if((m_mode & 0x10) && m_count)
	{
		const int rates[] = {44100, 22050, 11025, 5512};
		m_samples = 0;
		m_sample_byte = 0;
		m_stat = 0;
		m_isa->drq7_w(ASSERT_LINE);
		attotime rate = attotime::from_hz(rates[(m_mode >> 5) & 3]);
		m_pcm->adjust(rate, 0, rate);
	}
}

class vis_state : public driver_device
{
public:
	vis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic1(*this, "mb:pic8259_slave"),
		m_vga(*this, "vga")
		{ }
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic1;
	required_device<vga_device> m_vga;

	uint8_t sysctl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sysctl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t unk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void unk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t unk2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t unk3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pad_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pad_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t vga_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vga_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
protected:
	void machine_reset() override;
private:
	uint8_t m_sysctl;
	uint8_t m_unkidx;
	uint8_t m_unk[16];
	uint8_t m_pad[4];
	uint8_t m_crtcidx;
	uint8_t m_gfxidx;
};

void vis_state::machine_reset()
{
	m_sysctl = 0;
}

//chipset registers?
uint8_t vis_state::unk_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	if(offset)
		return m_unk[m_unkidx];
	return 0;
}

void vis_state::unk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if(offset)
		m_unk[m_unkidx] = data;
	else
		m_unkidx = data & 0xf;
}

uint8_t vis_state::unk2_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0x40;
}

//memory card reader?
uint8_t vis_state::unk3_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0x00;
}

uint8_t vis_state::pad_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	if(offset == 2)
		return 0xde;
	return 0;
}

void vis_state::pad_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	switch(offset)
	{
		case 1:
			if(data == 0x10)
				m_pic1->ir1_w(CLEAR_LINE);
			else if(data == 0x16)
				m_pic1->ir1_w(ASSERT_LINE);
	}
	m_pad[offset] = data;
}

uint8_t vis_state::vga_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	if(offset < 0x10)
		return m_vga->port_03b0_r(space, offset, mem_mask);
	else if(offset < 0x20)
		return m_vga->port_03c0_r(space, offset - 0x10, mem_mask);
	else
		return m_vga->port_03d0_r(space, offset - 0x20, mem_mask);
}

void vis_state::vga_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	switch(offset)
	{
		case 0x1e:
			m_gfxidx = data;
			break;
		case 0x1f:
			if(m_gfxidx == 0x05)
				data |= 0x40;
			break;
		case 0x04:
		case 0x24:
			m_crtcidx = data;
			break;
		case 0x05:
		case 0x25:
			if(m_crtcidx == 0x14)
				data |= 0x40;
			break;
	}
	if(offset < 0x10)
		m_vga->port_03b0_w(space, offset, data, mem_mask);
	else if(offset < 0x20)
		m_vga->port_03c0_w(space, offset - 0x10, data, mem_mask);
	else
		m_vga->port_03d0_w(space, offset - 0x20, data, mem_mask);
}

uint8_t vis_state::sysctl_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_sysctl;
}

void vis_state::sysctl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if(BIT(data, 0) && !BIT(m_sysctl, 0))
		m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	//m_maincpu->set_input_line(INPUT_LINE_A20, BIT(data, 1) ? CLEAR_LINE : ASSERT_LINE);
	m_sysctl = data;
}

static ADDRESS_MAP_START( at16_map, AS_PROGRAM, 16, vis_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x09ffff) AM_RAM
	AM_RANGE(0x0a0000, 0x0bffff) AM_DEVREADWRITE8("vga", vga_device, mem_r, mem_w, 0xffff)
	AM_RANGE(0x0d8000, 0x0fffff) AM_ROM AM_REGION("bios", 0xd8000)
	AM_RANGE(0x100000, 0x15ffff) AM_RAM
	AM_RANGE(0x300000, 0x3fffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0xff0000, 0xffffff) AM_ROM AM_REGION("bios", 0xf0000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( at16_io, AS_IO, 16, vis_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0026, 0x0027) AM_READWRITE8(unk_r, unk_w, 0xffff)
	AM_RANGE(0x006a, 0x006b) AM_READ8(unk2_r, 0x00ff)
	AM_RANGE(0x0092, 0x0093) AM_READWRITE8(sysctl_r, sysctl_w, 0x00ff)
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE("mb", at_mb_device, map)
	AM_RANGE(0x023c, 0x023f) AM_READWRITE8(pad_r, pad_w, 0xffff)
	AM_RANGE(0x031a, 0x031b) AM_READ8(unk3_r, 0x00ff)
	AM_RANGE(0x03b0, 0x03df) AM_READWRITE8(vga_r, vga_w, 0xffff)
ADDRESS_MAP_END

static SLOT_INTERFACE_START(vis_cards)
	SLOT_INTERFACE("visaudio", VIS_AUDIO)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( vis, vis_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, XTAL_12MHz )
	MCFG_CPU_PROGRAM_MAP(at16_map)
	MCFG_CPU_IO_MAP(at16_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)
	MCFG_80286_SHUTDOWN(DEVWRITELINE("mb", at_mb_device, shutdown))

	MCFG_DEVICE_ADD("mb", AT_MB, 0)

	MCFG_ISA16_SLOT_ADD("mb:isabus", "mcd", pc_isa16_cards, "mcd", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus", "visaudio", vis_cards, "visaudio", true)
	MCFG_FRAGMENT_ADD(pcvideo_vga)
MACHINE_CONFIG_END

ROM_START(vis)
	ROM_REGION(0x100000,"bios", 0)
	ROM_LOAD( "p513bk0b.bin", 0x00000, 0x80000, CRC(364e3f74) SHA1(04260ef1e65e482c9c49d25ace40e22487d6aab9))
	ROM_LOAD( "p513bk1b.bin", 0x80000, 0x80000, CRC(e18239c4) SHA1(a0262109e10a07a11eca43371be9978fff060bc5))
ROM_END

COMP ( 1992, vis,  0, 0, vis, 0, driver_device, 0, "Tandy/Memorex", "Video Information System MD-2500", MACHINE_NOT_WORKING )

