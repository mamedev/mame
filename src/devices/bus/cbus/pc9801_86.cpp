// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-86 sound card
    NEC PC-9801-SpeakBoard sound card

    Similar to PC-9801-26, this one has YM2608 instead of YM2203 and an
    additional DAC port
    SpeakBoard sound card seems to be derived design from -86, with an additional
    OPNA mapped at 0x58*

    TODO:
    - Test all pcm modes
    - Make volume work
    - Recording
    - SpeakBoard: no idea about software that uses this, also board shows a single YM2608B?
      "-86 only supports ADPCM instead of PCM, while SpeakBoard has OPNA + 256 Kbit RAM"
      Sounds like a sound core flaw since OPNA requires a rom region in any case;
    - verify sound irq;

***************************************************************************/

#include "emu.h"
#include "bus/cbus/pc9801_86.h"
#include "sound/volt_reg.h"
#include "speaker.h"

#define QUEUE_SIZE 32768

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(PC9801_86, pc9801_86_device, "pc9801_86", "pc9801_86")

WRITE_LINE_MEMBER(pc9801_86_device::sound_irq)
{
	m_fmirq = state ? true : false;
	/* TODO: seems to die very often */
	m_bus->int_w<5>(state || (m_pcmirq ? ASSERT_LINE : CLEAR_LINE));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pc9801_86_device::pc9801_86_config(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	YM2608(config, m_opna, 7.987_MHz_XTAL);
	m_opna->irq_handler().set(FUNC(pc9801_86_device::sound_irq));
	m_opna->set_flags(AY8910_SINGLE_OUTPUT);
	m_opna->port_a_read_callback().set(FUNC(pc9801_86_device::opn_porta_r));
	//m_opna->port_b_read_callback().set(FUNC(pc8801_state::opn_portb_r));
	//m_opna->port_a_write_callback().set(FUNC(pc8801_state::opn_porta_w));
	m_opna->port_b_write_callback().set(FUNC(pc9801_86_device::opn_portb_w));
	m_opna->add_route(0, "lspeaker", 1.00);
	m_opna->add_route(0, "rspeaker", 1.00);
	m_opna->add_route(1, "lspeaker", 1.00);
	m_opna->add_route(2, "rspeaker", 1.00);

	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_ldac, 0).add_route(ALL_OUTPUTS, "lspeaker", 1.0); // burr brown pcm61p
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_rdac, 0).add_route(ALL_OUTPUTS, "rspeaker", 1.0); // burr brown pcm61p
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "ldac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "ldac", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "rdac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "rdac", -1.0, DAC_VREF_NEG_INPUT);
}

void pc9801_86_device::device_add_mconfig(machine_config &config)
{
	pc9801_86_config(config);
}

// to load a different bios for slots:
// -cbusX pc9801_86,bios=N
ROM_START( pc9801_86 )
	ROM_REGION( 0x4000, "sound_bios", ROMREGION_ERASEFF )
	// following roms are unchecked and of dubious quality
	// we currently mark bios names based off where they originally belonged to, lacking of a better info
	// supposedly these are -86 roms according to eikanwa2 sound card detection,
	// loading a -26 rom in a -86 environment causes an hang there.
	ROM_SYSTEM_BIOS( 0,  "86rx",    "nec86rx" )
	ROMX_LOAD( "sound_rx.rom",    0x0000, 0x4000, BAD_DUMP CRC(fe9f57f2) SHA1(d5dbc4fea3b8367024d363f5351baecd6adcd8ef), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1,  "86mu",    "nec86mu" )
	ROMX_LOAD( "sound_486mu.rom", 0x0000, 0x4000, BAD_DUMP CRC(6cdfa793) SHA1(4b8250f9b9db66548b79f961d61010558d6d6e1c), ROM_BIOS(1) )

	// RAM
	ROM_REGION( 0x100000, "opna", ROMREGION_ERASE00 )
ROM_END

const tiny_rom_entry *pc9801_86_device::device_rom_region() const
{
	return ROM_NAME( pc9801_86 );
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( pc9801_86 )
	PORT_INCLUDE( pc9801_joy_port )

	PORT_START("OPNA_DSW")
	PORT_CONFNAME( 0x01, 0x01, "PC-9801-86: Port Base" )
	PORT_CONFSETTING(    0x00, "0x088" )
	PORT_CONFSETTING(    0x01, "0x188" )
INPUT_PORTS_END

ioport_constructor pc9801_86_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_86 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc9801_86_device - constructor
//-------------------------------------------------

pc9801_86_device::pc9801_86_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_snd_device(mconfig, type, tag, owner, clock),
		m_bus(*this, DEVICE_SELF_OWNER),
		m_opna(*this, "opna"),
		m_ldac(*this, "ldac"),
		m_rdac(*this, "rdac"),
		m_queue(QUEUE_SIZE)
{
}

pc9801_86_device::pc9801_86_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_86_device(mconfig, PC9801_86, tag, owner, clock)
{

}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void pc9801_86_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------


void pc9801_86_device::device_start()
{
	m_bus->program_space().install_rom(0xcc000,0xcffff,memregion(this->subtag("sound_bios").c_str())->base());
	m_bus->install_io(0xa460, 0xa463, read8_delegate(*this, FUNC(pc9801_86_device::id_r)), write8_delegate(*this, FUNC(pc9801_86_device::mask_w)));
	m_bus->install_io(0xa464, 0xa46f, read8_delegate(*this, FUNC(pc9801_86_device::pcm_r)), write8_delegate(*this, FUNC(pc9801_86_device::pcm_w)));
	m_bus->install_io(0xa66c, 0xa66f, read8_delegate(*this, [this](address_space &s, offs_t o, u8 mm){ return o == 2 ? m_pcm_mute : 0xff; }, "pc9801_86_mute_r"),
								   write8_delegate(*this, [this](address_space &s, offs_t o, u8 d, u8 mm){ if(o == 2) m_pcm_mute = d; }, "pc9801_86_mute_w"));

	m_dac_timer = timer_alloc();
	save_item(NAME(m_count));
	save_item(NAME(m_queue));
	save_item(NAME(m_irq_rate));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc9801_86_device::device_reset()
{
	uint16_t port_base = (ioport("OPNA_DSW")->read() & 1) << 8;
	m_bus->io_space().unmap_readwrite(0x0088, 0x008f, 0x100);
	m_bus->install_io(port_base + 0x0088, port_base + 0x008f, read8_delegate(*this, FUNC(pc9801_86_device::opna_r)), write8_delegate(*this, FUNC(pc9801_86_device::opna_w)));

	m_mask = 0;
	m_head = m_tail = m_count = 0;
	m_fmirq = m_pcmirq = m_init = false;
	m_irq_rate = 0;
	m_pcm_ctrl = m_pcm_mode = 0;
	m_pcm_mute = 0;
	m_pcm_clk = false;
	memset(&m_queue[0], 0, QUEUE_SIZE);
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


READ8_MEMBER(pc9801_86_device::opna_r)
{
	if((offset & 1) == 0)
		return m_opna->read(offset >> 1);
	else // odd
	{
		logerror("PC9801-86: Read to undefined port [%02x]\n",offset+0x188);
		return 0xff;
	}
}

WRITE8_MEMBER(pc9801_86_device::opna_w)
{
	if((offset & 1) == 0)
		m_opna->write(offset >> 1,data);
	else // odd
		logerror("PC9801-86: Write to undefined port [%02x] %02x\n",offset+0x188,data);
}

READ8_MEMBER(pc9801_86_device::id_r)
{
	return 0x40 | m_mask;
}

WRITE8_MEMBER(pc9801_86_device::mask_w)
{
	m_mask = data & 1;
}

READ8_MEMBER(pc9801_86_device::pcm_r)
{
	if((offset & 1) == 0)
	{
		switch(offset >> 1)
		{
			case 1:
				return ((queue_count() == QUEUE_SIZE) ? 0x80 : 0) |
						(!queue_count() ? 0x40 : 0) | (m_pcm_clk ? 1 : 0);
			case 2:
				return m_pcm_ctrl | (m_pcmirq ? 0x10 : 0);
			case 3:
				return m_pcm_mode;
			case 4:
				return 0;
		}
	}
	else // odd
		logerror("PC9801-86: Read to undefined port [%02x]\n",offset+0xa464);
	return 0xff;
}

WRITE8_MEMBER(pc9801_86_device::pcm_w)
{
	const u32 rate = (25.4_MHz_XTAL).value() / 16;
	const int divs[8] = {36, 48, 72, 96, 144, 192, 288, 384};
	if((offset & 1) == 0)
	{
		switch(offset >> 1)
		{
			case 1:
				m_vol[data >> 5] = data & 0x0f;
				break;
			case 2:
				if(((data & 7) != (m_pcm_ctrl & 7)) || !m_init)
					m_dac_timer->adjust(attotime::from_ticks(divs[data & 7], rate), 0, attotime::from_ticks(divs[data & 7], rate));
				if(data & 8)
					m_head = m_tail = m_count = 0;
				if(!(data & 0x10))
				{
					m_bus->int_w<5>(m_fmirq ? ASSERT_LINE : CLEAR_LINE);
					if(!(queue_count() < m_irq_rate) || !(data & 0x80))
						m_pcmirq = false; //TODO: this needs research
				}
				m_init = true;
				m_pcm_ctrl = data & ~0x10;
				break;
			case 3:
				if(m_pcm_ctrl & 0x20)
					m_irq_rate = (data + 1) * 128;
				else
					m_pcm_mode = data;
				break;
			case 4:
				if(queue_count() < QUEUE_SIZE)
				{
					m_queue[m_head++] = data;
					m_head %= QUEUE_SIZE;
					m_count++;
				}
				break;
		}
	}
	else // odd
		logerror("PC9801-86: Write to undefined port [%02x] %02x\n",offset+0xa464,data);
}

int pc9801_86_device::queue_count()
{
	return m_count;
}

uint8_t pc9801_86_device::queue_pop()
{
	uint8_t ret = m_queue[m_tail++];
	m_tail %= QUEUE_SIZE;
	m_count = (m_count - 1) % QUEUE_SIZE; // dangel resets the fifo after filling it completely so maybe it expects an underflow
	return ret;
}

void pc9801_86_device::device_timer(emu_timer& timer, device_timer_id id, int param, void* ptr)
{
	int16_t lsample, rsample;

	m_pcm_clk = !m_pcm_clk;
	if((m_pcm_ctrl & 0x40) || !(m_pcm_ctrl & 0x80))
		return;

	switch(m_pcm_mode & 0x70)
	{
		case 0x70: // 8bit stereo
			m_ldac->write(queue_pop() << 8);
			m_rdac->write(queue_pop() << 8);
			break;
		case 0x60: // 8bit left only
			m_ldac->write(queue_pop() << 8);
			break;
		case 0x50: // 8bit right only
			m_rdac->write(queue_pop() << 8);
			break;
		case 0x30: // 16bit stereo
			lsample = queue_pop() << 8;
			lsample |= queue_pop();
			rsample = queue_pop() << 8;
			rsample |= queue_pop();
			m_ldac->write(lsample);
			m_rdac->write(rsample);
			break;
		case 0x20: // 16bit left only
			lsample = queue_pop() << 8;
			lsample |= queue_pop();
			m_ldac->write(lsample);
			break;
		case 0x10: // 16bit right only
			rsample = queue_pop() << 8;
			rsample |= queue_pop();
			m_rdac->write(rsample);
			break;
	}
	if((queue_count() < m_irq_rate) && (m_pcm_ctrl & 0x20))
	{
		m_pcmirq = true;
		m_bus->int_w<5>(ASSERT_LINE);
	}
}

//**************************************************************************
//  SpeakBoard device section
//**************************************************************************

DEFINE_DEVICE_TYPE(PC9801_SPEAKBOARD, pc9801_speakboard_device, "pc9801_spb", "NEC PC9801 SpeakBoard")

ROM_START( pc9801_spb )
	ROM_REGION( 0x4000, "sound_bios", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "spb lh5764 ic21_pink.bin",    0x0001, 0x2000, CRC(5bcefa1f) SHA1(ae88e45d411bf5de1cb42689b12b6fca0146c586) )
	ROM_LOAD16_BYTE( "spb lh5764 ic22_green.bin",   0x0000, 0x2000, CRC(a7925ced) SHA1(3def9ee386ab6c31436888261bded042cd64a0eb) )

	// RAM
	ROM_REGION( 0x100000, "opna", ROMREGION_ERASE00 )

	ROM_REGION( 0x100000, "opna_slave", ROMREGION_ERASE00 )
ROM_END

const tiny_rom_entry *pc9801_speakboard_device::device_rom_region() const
{
	return ROM_NAME( pc9801_spb );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc9801_speakboard_device - constructor
//-------------------------------------------------

pc9801_speakboard_device::pc9801_speakboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_86_device(mconfig, PC9801_SPEAKBOARD, tag, owner, clock),
	m_opna_slave(*this, "opna_slave")
{
}

void pc9801_speakboard_device::device_add_mconfig(machine_config &config)
{
	pc9801_86_config(config);

	m_opna->reset_routes();
	m_opna->add_route(0, "lspeaker", 0.50);
	m_opna->add_route(0, "rspeaker", 0.50);
	m_opna->add_route(1, "lspeaker", 0.50);
	m_opna->add_route(2, "rspeaker", 0.50);

	YM2608(config, m_opna_slave, 7.987_MHz_XTAL);
	m_opna_slave->set_flags(AY8910_SINGLE_OUTPUT);
	m_opna_slave->add_route(0, "lspeaker", 0.50);
	m_opna_slave->add_route(0, "rspeaker", 0.50);
	m_opna_slave->add_route(1, "lspeaker", 0.50);
	m_opna_slave->add_route(2, "rspeaker", 0.50);
}

void pc9801_speakboard_device::device_start()
{
	pc9801_86_device::device_start();

	m_bus->install_io(0x0588, 0x058f, read8_delegate(*this, FUNC(pc9801_speakboard_device::opna_slave_r)), write8_delegate(*this, FUNC(pc9801_speakboard_device::opna_slave_w)));
}

void pc9801_speakboard_device::device_reset()
{
	pc9801_86_device::device_reset();
}

READ8_MEMBER(pc9801_speakboard_device::opna_slave_r)
{
	if((offset & 1) == 0)
		return m_opna_slave->read(offset >> 1);
	else // odd
	{
		logerror("PC9801-SPB: Read to undefined port [%02x]\n",offset+0x588);
		return 0xff;
	}
}

WRITE8_MEMBER(pc9801_speakboard_device::opna_slave_w)
{
	if((offset & 1) == 0)
		m_opna_slave->write(offset >> 1,data);
	else // odd
		logerror("PC9801-SPB: Write to undefined port [%02x] %02x\n",offset+0x588,data);
}

