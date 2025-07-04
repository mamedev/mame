// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

NEC PC-9801-86 sound card
NEC PC-9801-SpeakBoard sound card
Mad Factory Otomi-chan Kai sound card

Similar to PC-9801-26, this one has YM2608 instead of YM2203 and an
additional DAC port

SpeakBoard sound card seems to be derived design from -86, with an additional
OPNA mapped at 0x58*

Otomi-chan Kai is a doujinshi sound card based off SpeakBoard design.
It uses YM3438 OPL2C mapped at 0x78*, and anything that uses the nax.exe sound driver
expects this to be installed as default (cfr. datsumj).
To fallback to a regular -26/-86 board user needs to add parameter switches "-2" or "-3"
respectively, cfr. "nax -?" for additional details.

TODO:
- Test all pcm modes;
- Fix PCM overflow bug properly (CPUENB signal yield host CPU until DAC catches up?)
- Make volume work;
- Recording;
- SpeakBoard: no idea about software that uses this, also board shows a single YM2608B?
    "-86 only supports ADPCM instead of PCM, while SpeakBoard has OPNA + 256 Kbit RAM";
- Otomi-chan Kai: find a manual (マニュアル), it's mentioned with nax usage.
    Very low-res scan of the PCB sports a 4-bit dip-sw bank at very least;
- Otomi-chan Kai: unknown ID port readback;
- verify sound irq;

**************************************************************************************************/

#include "emu.h"
#include "bus/cbus/pc9801_86.h"
#include "speaker.h"

#define QUEUE_SIZE 32768

#define LOG_DAC        (1U << 1)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGDAC(...)    LOGMASKED(LOG_DAC, __VA_ARGS__)


DEFINE_DEVICE_TYPE(PC9801_86, pc9801_86_device, "pc9801_86", "NEC PC-9801-86")
DEFINE_DEVICE_TYPE(PC9801_SPEAKBOARD, pc9801_speakboard_device, "pc9801_spb", "NEC PC-9801 SpeakBoard")
DEFINE_DEVICE_TYPE(OTOMICHAN_KAI, otomichan_kai_device, "pc98_otomichan_kai", "MAD Factory Otomi-chan Kai") // 音美(おとみ)ちゃん改

pc9801_86_device::pc9801_86_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_bus(*this, DEVICE_SELF_OWNER)
	, m_opna(*this, "opna")
	, m_irqs(*this, "irqs")
	, m_ldac(*this, "ldac")
	, m_rdac(*this, "rdac")
	, m_queue(QUEUE_SIZE)
	, m_joy(*this, "joy_port")
{
}

pc9801_86_device::pc9801_86_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_86_device(mconfig, PC9801_86, tag, owner, clock)
{

}



void pc9801_86_device::pc9801_86_config(machine_config &config)
{
	// TODO: "SecondBus86" PCB contents differs from current hookup
	// XTAL 15.9744 (X1)
	// HD641180X0 MCU (U7) (!)
	// YM2608B (U11)
	// CS4231A (U15)
	// OPL4 YMF278 + YRW801 (U21 + U22)
	// TC55257CFL-10 (U15)
	// unknown chip (most likely surface scratched) U3)

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set([this](int state) { m_bus->int_w<5>(state); });

	SPEAKER(config, "speaker", 2).front();
	YM2608(config, m_opna, 7.987_MHz_XTAL); // actually YM2608B
	// shouldn't have one
//  m_opna->set_addrmap(0, &pc9801_86_device::opna_map);
	m_opna->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_opna->port_a_read_callback().set([this] () {
		if((m_joy_sel & 0xc0) == 0x80)
			return m_joy->read();

		return (u8)0xff;
	});
	m_opna->port_b_write_callback().set([this] (u8 data) {
		m_joy_sel = data;
	});
	// TODO: confirm mixing
	m_opna->add_route(0, "speaker", 0.75, 0);
	m_opna->add_route(0, "speaker", 0.75, 1);
	m_opna->add_route(1, "speaker", 1.00, 0);
	m_opna->add_route(2, "speaker", 1.00, 1);

	// 2x burr brown pcm61p
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_ldac, 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_rdac, 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	// NOTE: 1x DE-9 port only
	MSX_GENERAL_PURPOSE_PORT(config, m_joy, msx_general_purpose_port_devices, "joystick");
}

void pc9801_86_device::device_add_mconfig(machine_config &config)
{
	pc9801_86_config(config);
}

// helper for derived devices to account for the different master OPNA sound mixing
void pc9801_86_device::opna_reset_routes_config(machine_config &config)
{
	m_opna->reset_routes();
	m_opna->add_route(0, "speaker", 0.125, 0);
	m_opna->add_route(0, "speaker", 0.125, 1);
	m_opna->add_route(1, "speaker", 0.50, 0);
	m_opna->add_route(2, "speaker", 0.50, 1);
}

// to load a different bios for slots:
// -cbusX pc9801_86,bios=N
ROM_START( pc9801_86 )
	ROM_REGION( 0x4000, "sound_bios", ROMREGION_ERASEFF )
	// following roms are unchecked and of dubious quality
	// we currently mark bios names based off where they originally belonged to, lacking of a better info
	// supposedly these are -86 roms according to eikanwa2 sound card detection,
	// loading a -26 rom in a -86 environment causes an hang there.
	// TODO: several later machines (i.e. CanBe) really has an internal -86 with sound BIOS data coming directly from the machine ROM
	// it also sports different ID mapping at $a460
	ROM_SYSTEM_BIOS( 0,  "86rx",    "nec86rx" )
	ROMX_LOAD( "sound_rx.rom",    0x0000, 0x4000, BAD_DUMP CRC(fe9f57f2) SHA1(d5dbc4fea3b8367024d363f5351baecd6adcd8ef), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1,  "86mu",    "epson86mu" )
	ROMX_LOAD( "sound_486mu.rom", 0x0000, 0x4000, BAD_DUMP CRC(6cdfa793) SHA1(4b8250f9b9db66548b79f961d61010558d6d6e1c), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *pc9801_86_device::device_rom_region() const
{
	return ROM_NAME( pc9801_86 );
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( pc9801_86 )
	// Single 8-bit DSW bank
	PORT_START("OPNA_DSW")
	PORT_DIPNAME( 0x01, 0x00, "PC-9801-86: Port Base" ) PORT_DIPLOCATION("OPNA_SW:!1")
	PORT_DIPSETTING(    0x00, "0x188" )
	PORT_DIPSETTING(    0x01, "0x288" )
	PORT_DIPNAME( 0x02, 0x00, "PC-9801-86: Enable sound ROM") PORT_DIPLOCATION("OPNA_SW:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) ) // hardwired at 0xcc000
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPNAME( 0x0c, 0x00, "PC-9801-86: Interrupt level") PORT_DIPLOCATION("OPNA_SW:!3,!4")
	PORT_DIPSETTING(    0x0c, "IRQ 0" )
	PORT_DIPSETTING(    0x08, "IRQ 4" )
	PORT_DIPSETTING(    0x00, "IRQ 5" )
	PORT_DIPSETTING(    0x04, "IRQ 6" )
	PORT_DIPNAME( 0x10, 0x00, "PC-9801-86: Interrupt enable") PORT_DIPLOCATION("OPNA_SW:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	// TODO: how HW really read this?
	PORT_DIPNAME( 0xe0, 0x80, "PC-9801-86: ID number") PORT_DIPLOCATION("OPNA_SW:!6,!7,!8")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x60, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xa0, "5" )
	PORT_DIPSETTING(    0xc0, "6" )
	PORT_DIPSETTING(    0xe0, "7" )
INPUT_PORTS_END

ioport_constructor pc9801_86_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_86 );
}

// only for derived designs?
void pc9801_86_device::opna_map(address_map &map)
{
	// TODO: confirm it really is ROMless
	// TODO: confirm size
	map(0x000000, 0x1fffff).ram();
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void pc9801_86_device::device_validity_check(validity_checker &valid) const
{
}

u16 pc9801_86_device::read_io_base()
{
	return ((ioport("OPNA_DSW")->read() & 1) << 8) + 0x188;
}

void pc9801_86_device::device_start()
{
	// TODO: uninstall option from dip
	m_bus->program_space().install_rom(
		0xcc000,
		0xcffff,
		memregion(this->subtag("sound_bios").c_str())->base()
	);
	// TODO: who wins if 2+ PC-9801-86 or mixed -73/-86 are mounted?
	m_bus->install_device(0xa460, 0xa46f, *this, &pc9801_86_device::io_map);
	m_bus->install_io(0xa66c, 0xa66f,
		read8sm_delegate(*this, [this](offs_t o){ return o == 2 ? m_pcm_mute : 0xff; }, "pcm_mute_r"),
		write8sm_delegate(*this, [this](offs_t o, u8 d){
			if(o == 2)
			{
				m_pcm_mute = d;
				m_ldac->set_output_gain(ALL_OUTPUTS, BIT(m_pcm_mute, 0) ? 0.0 : 1.0);
				m_rdac->set_output_gain(ALL_OUTPUTS, BIT(m_pcm_mute, 0) ? 0.0 : 1.0);
			}
		}, "pcm_mute_w")
	);

	m_io_base = 0;

	m_dac_timer = timer_alloc(FUNC(pc9801_86_device::dac_tick), this);
	save_item(NAME(m_count));
	save_item(NAME(m_queue));
	save_item(NAME(m_irq_rate));
}

void pc9801_86_device::device_reset()
{
	u16 current_io = read_io_base();
	m_bus->flush_install_io(
		this->tag(),
		m_io_base,
		current_io,
		7,
		read8sm_delegate(*this, FUNC(pc9801_86_device::opna_r)),
		write8sm_delegate(*this, FUNC(pc9801_86_device::opna_w))
	);
	m_io_base = current_io;

	m_mask = 0;
	m_head = m_tail = m_count = 0;
	m_pcmirq = m_init = false;
	m_irq_rate = 0;
	m_pcm_ctrl = m_pcm_mode = 0;
	// Starts off with DACs muted (os2warp3 will burp a lot while initializing OS)
	m_pcm_mute = 0x01;
	m_ldac->set_output_gain(ALL_OUTPUTS, 0.0);
	m_rdac->set_output_gain(ALL_OUTPUTS, 0.0);

	m_pcm_clk = false;
	memset(&m_queue[0], 0, QUEUE_SIZE);
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


u8 pc9801_86_device::opna_r(offs_t offset)
{
	if((offset & 1) == 0)
		return m_opna->read(offset >> 1);
	else // odd
	{
		logerror("%s: Read to undefined port [%02x]\n", machine().describe_context(), offset + m_io_base);
		return 0xff;
	}
}

void pc9801_86_device::opna_w(offs_t offset, u8 data)
{
	if((offset & 1) == 0)
		m_opna->write(offset >> 1,data);
	else // odd
		logerror("%s: Write to undefined port [%02x] %02x\n", machine().describe_context(), offset + m_io_base, data);
}

u8 pc9801_86_device::pcm_control_r()
{
	return m_pcm_ctrl | (m_pcmirq ? 0x10 : 0);
}

void pc9801_86_device::pcm_control_w(u8 data)
{
	const u32 rate = (25.4_MHz_XTAL).value() / 16;
	const int divs[8] = {36, 48, 72, 96, 144, 192, 288, 384};

	LOGDAC("$a468 FIFO Control %02x\n", data);
	if(((data & 7) != (m_pcm_ctrl & 7)) || !m_init)
	{
		LOGDAC("\tclk rate %01x (%d)\n", data, divs[data & 7]);
		m_dac_timer->adjust(attotime::from_ticks(divs[data & 7], rate), 0, attotime::from_ticks(divs[data & 7], rate));
	}
	if(data & 8)
	{
		LOGDAC("\tFIFO reset\n");
		m_head = m_tail = m_count = 0;
	}
	if(!(data & 0x10))
	{
		LOGDAC("\tIRQ clear\n");
		//m_bus->int_w<5>(m_fmirq ? ASSERT_LINE : CLEAR_LINE);
		if(!(queue_count() < m_irq_rate) || !(data & 0x80))
		{
			//TODO: this needs research
			m_pcmirq = false;
			m_irqs->in_w<1>(CLEAR_LINE);
		}
	}
	m_init = true;
	m_pcm_ctrl = data & ~0x10;
}

// $a460 base
void pc9801_86_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).rw(FUNC(pc9801_86_device::id_r), FUNC(pc9801_86_device::mask_w));
//  map(0x02, 0x02) μPD6380 for PC9801-73 control
//  map(0x04, 0x04) μPD6380 for PC9801-73 data
	map(0x06, 0x06).lrw8(
		NAME([this] () {
			u8 res = 0;
			// FIFO full
			res |= (queue_count() == QUEUE_SIZE) << 7;
			// FIFO empty
			res |= !queue_count() << 6;
			// recording overflow
			res |= 0 << 5;
			// DAC clock
			res |= m_pcm_clk << 0;
			return res;
		}),
		NAME([this] (u8 data) {
			const u8 line_select = data >> 5;
			m_vol[line_select] = data & 0x0f;
			logerror("$a466 volume control [%02x] %02x\n", line_select, data & 0xf);
		})
	);
	map(0x08, 0x08).rw(FUNC(pc9801_86_device::pcm_control_r), FUNC(pc9801_86_device::pcm_control_w));
	map(0x0a, 0x0a).lrw8(
		NAME([this] () {
			return m_pcm_mode;
		}),
		NAME([this] (u8 data) {
			if(m_pcm_ctrl & 0x20)
			{
				// TODO: may fall over with the irq logic math
				// queue_count() < (0xff + 1) << 7 = 0x8000 -> always true
				if (data == 0xff)
					popmessage("pc9801_86: $a46a irq_rate == 0xff");
				m_irq_rate = (data + 1) * 128;
				LOGDAC("$a468 irq_rate %d (%02x)\n", m_irq_rate, data);
			}
			else
			{
				m_pcm_mode = data;
				LOGDAC("$a468 pcm_mode %02x\n", data);
				LOGDAC("\tclock %d quantization %d-bit output %d mode %d\n"
					, BIT(data, 7)
					, BIT(data, 6) ? 16 : 8
					// 3 = stereo, 2 Left only, 1 Right only, 0 = No PCM output
					, (data >> 4) & 3
					// TODO: unknown purpose, normally 2, apparently set by AVSDRV differently
					, data & 3
				);
			}
		})
	);
	map(0x0c, 0x0c).lrw8(
		NAME([this] () {
			// TODO: recording mode
			(void)this;
			return 0;
		}),
		NAME([this] (u8 data) {
			// HACK: on overflow make sure to single step the FIFO enough to claim some space back
			// os2warp3 initializes the full buffer with 0x00 then quickly pretends
			// that DAC already catched up by the time the actual startup/shutdown chimes are sent.
			if (queue_count() == QUEUE_SIZE)
			{
				dac_transfer();
				logerror("Warning: $a46c write with FIFO overflow %02x\n", m_pcm_mode);
			}

			if(queue_count() < QUEUE_SIZE)
			{
				m_queue[m_head++] = data;
				m_head %= QUEUE_SIZE;
				m_count++;
			}
		})
	);
}

/*
 * xxxx ---- ID
 * 0000 ---- PC-90DO+ built-in
 * 0001 ---- PC-98GS built-in
 * 0010 ---- PC-9801-73/-76, I/O base $188
 * 0011 ---- PC-9801-73/-76, I/O base $288
 * 0100 ---- PC-9801-86, I/O base $188, PC-9821 Multi/A Mate/early CanBe (Ce/Cs2/Ce2)
 * 0101 ---- PC-9801-86, I/O base $288
 * 0110 ---- PC-9821Nf/Np built-in
 * 0111 ---- PC-9821 X Mate (as PC-9821XE10-B?)
 * 1000 ---- PC-9821 later CanBe/Na7/Nx built-in
 * 1111 ---- <unsupported or PC-9801-26>
 * ---- --x- (1) OPNA force volume to 0
 * ---- ---x select OPN base
 * ---- ---1 OPNA
 * ---- ---0 OPN
 */
u8 pc9801_86_device::id_r()
{
	// either a -86 or 9821 MATE A uses this id (built-in)
	const u8 id_port = ((ioport("OPNA_DSW")->read() & 1) << 4) | 0x40;
	return id_port | m_mask;
}

void pc9801_86_device::mask_w(u8 data)
{
	m_mask = data & 3;
	// TODO: bit 1 totally cuts off OPNA output
	logerror("%s: OPNA mask setting %02x\n", machine().describe_context(), data);
}

int pc9801_86_device::queue_count()
{
	return m_count;
}

u8 pc9801_86_device::queue_pop()
{
	u8 ret = m_queue[m_tail++];
	m_tail %= QUEUE_SIZE;
	// TODO: dangel resets the fifo after filling it completely so maybe it expects an underflow
	// this breaks win95, that expects FIFO empty flags to stay consistant
	//m_count = (m_count - 1) % QUEUE_SIZE;
	m_count = std::max(m_count - 1, 0);
	return ret;
}

void pc9801_86_device::dac_transfer()
{
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
		case 0x30: { // 16bit stereo
			int16_t lsample = queue_pop() << 8;
			lsample |= queue_pop();
			int16_t rsample = queue_pop() << 8;
			rsample |= queue_pop();
			m_ldac->write(lsample);
			m_rdac->write(rsample);
		} break;
		case 0x20: { // 16bit left only
			int16_t lsample = queue_pop() << 8;
			lsample |= queue_pop();
			m_ldac->write(lsample);
		}   break;
		case 0x10: { // 16bit right only
			int16_t rsample = queue_pop() << 8;
			rsample |= queue_pop();
			m_rdac->write(rsample);
		}   break;
	}
}

TIMER_CALLBACK_MEMBER(pc9801_86_device::dac_tick)
{
	m_pcm_clk = !m_pcm_clk;
	if((m_pcm_ctrl & 0x40) || !(m_pcm_ctrl & 0x80))
		return;

	// TODO: verify underflow
	// should leave the DACs in whatever state they are or ...?
	if (!queue_count())
		return;

	dac_transfer();
	if((queue_count() < m_irq_rate) && (m_pcm_ctrl & 0x20))
	{
		//LOGDAC("\tIRQ set\n");
		m_pcmirq = true;
		// win95 expects edge triggers
		m_irqs->in_w<1>(CLEAR_LINE);
		m_irqs->in_w<1>(ASSERT_LINE);
	}
}

//**************************************************************************
//
//  SpeakBoard device section
//
//**************************************************************************

ROM_START( pc9801_spb )
	ROM_REGION( 0x4000, "sound_bios", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "spb lh5764 ic21_pink.bin",    0x0001, 0x2000, CRC(5bcefa1f) SHA1(ae88e45d411bf5de1cb42689b12b6fca0146c586) )
	ROM_LOAD16_BYTE( "spb lh5764 ic22_green.bin",   0x0000, 0x2000, CRC(a7925ced) SHA1(3def9ee386ab6c31436888261bded042cd64a0eb) )
ROM_END

const tiny_rom_entry *pc9801_speakboard_device::device_rom_region() const
{
	return ROM_NAME( pc9801_spb );
}

pc9801_speakboard_device::pc9801_speakboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_86_device(mconfig, PC9801_SPEAKBOARD, tag, owner, clock)
	, m_opna_slave(*this, "opna_slave")
{
}

void pc9801_speakboard_device::device_add_mconfig(machine_config &config)
{
	pc9801_86_config(config);
	opna_reset_routes_config(config);
	// TODO: confirm RAM mapping configuration (shared? not present on either chip? misc?)
	m_opna->set_addrmap(0, &pc9801_speakboard_device::opna_map);

	YM2608(config, m_opna_slave, 7.987_MHz_XTAL);
	m_opna_slave->set_addrmap(0, &pc9801_speakboard_device::opna_map);
	m_opna_slave->add_route(0, "speaker", 0.50, 0);
	m_opna_slave->add_route(0, "speaker", 0.50, 1);
	m_opna_slave->add_route(1, "speaker", 0.50, 0);
	m_opna_slave->add_route(2, "speaker", 0.50, 1);
}

void pc9801_speakboard_device::device_start()
{
	pc9801_86_device::device_start();

	m_bus->install_io(0x0588, 0x058f, read8sm_delegate(*this, FUNC(pc9801_speakboard_device::opna_slave_r)), write8sm_delegate(*this, FUNC(pc9801_speakboard_device::opna_slave_w)));
}

void pc9801_speakboard_device::device_reset()
{
	pc9801_86_device::device_reset();
}

u8 pc9801_speakboard_device::opna_slave_r(offs_t offset)
{
	if((offset & 1) == 0)
		return m_opna_slave->read(offset >> 1);
	else // odd
	{
		logerror("%s: Read to undefined port [%02x]\n", machine().describe_context(), offset + 0x588);
		return 0xff;
	}
}

void pc9801_speakboard_device::opna_slave_w(offs_t offset, u8 data)
{
	if((offset & 1) == 0)
		m_opna_slave->write(offset >> 1,data);
	else // odd
		logerror("%s: Write to undefined port [%02x] %02x\n", machine().describe_context(), offset + 0x588, data);
}

//**************************************************************************
//
//  Otomi-chan Kai device section
//
//**************************************************************************

otomichan_kai_device::otomichan_kai_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_86_device(mconfig, OTOMICHAN_KAI, tag, owner, clock)
	, m_opn2c(*this, "opn2c")
{
}

ROM_START( pc98_otomichan_kai )
	ROM_REGION( 0x4000, "sound_bios", ROMREGION_ERASEFF )
	// TODO: "compatible" with SpeakBoard, does it even uses a ROM altogether? low-res PCB pic doesn't help at all.
	ROM_LOAD16_BYTE( "spb lh5764 ic21_pink.bin",    0x0001, 0x2000, BAD_DUMP CRC(5bcefa1f) SHA1(ae88e45d411bf5de1cb42689b12b6fca0146c586) )
	ROM_LOAD16_BYTE( "spb lh5764 ic22_green.bin",   0x0000, 0x2000, BAD_DUMP CRC(a7925ced) SHA1(3def9ee386ab6c31436888261bded042cd64a0eb) )
ROM_END

const tiny_rom_entry *otomichan_kai_device::device_rom_region() const
{
	return ROM_NAME( pc98_otomichan_kai );
}

void otomichan_kai_device::device_add_mconfig(machine_config &config)
{
	pc9801_86_config(config);
	opna_reset_routes_config(config);
	// TODO: confirm if this has OPNA RAM
	m_opna->set_addrmap(0, &otomichan_kai_device::opna_map);

	YM3438(config, m_opn2c, 7.987_MHz_XTAL);
	m_opn2c->add_route(0, "speaker", 0.50, 0);
	m_opn2c->add_route(1, "speaker", 0.50, 1);
}

u8 otomichan_kai_device::id_r()
{
	// no ID, unconfirmed if it has mask
	return 0xf0 | m_mask;
}

void otomichan_kai_device::device_start()
{
	pc9801_86_device::device_start();

	m_bus->install_io(0x0788, 0x078f, read8sm_delegate(*this, FUNC(otomichan_kai_device::opn2c_r)), write8sm_delegate(*this, FUNC(otomichan_kai_device::opn2c_w)));
}

void otomichan_kai_device::device_reset()
{
	pc9801_86_device::device_reset();
}

u8 otomichan_kai_device::opn2c_r(offs_t offset)
{
	if((offset & 1) == 0)
		return m_opn2c->read(offset >> 1);
	else // odd
	{
		logerror("%s: Read to undefined port [%02x]\n", machine().describe_context(), offset + 0x788);
		return 0xff;
	}
}

void otomichan_kai_device::opn2c_w(offs_t offset, u8 data)
{
	if((offset & 1) == 0)
		m_opn2c->write(offset >> 1, data);
	else // odd
		logerror("%s: Write to undefined port [%02x] %02x\n", machine().describe_context(), offset + 0x788, data);
}
