// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edström
/***************************************************************************************************
 *
 *   Ericsson Information Systems PC "compatibles"
 *
 * The Ericsson PC was the the first original Ericsson design for the office PC market replacing the 
 * Step/One which was an OEM:ed clone of the Matsushita Mybrain 3000 (see myb3k.cpp driver).
 *
 ************************************************************** 
 * Ericsson PC
 *------------
 * Links: https://youtu.be/6uilOdMJc24
 * Form Factor: Desktop
 * CPU: 8088 @ 4.77MHz
 * RAM: 256K
 * Bus: 6x ISA
 * Video: Monchrome or Color 80x25 character mode. 320x200 and 640x400 grahics modes
 * Display: Orange Gas Plasma (GP) display
 * Mass storage: 2 x 5.25" 360K or 1 20Mb HDD
 * On board ports: Beeper,
 * Ports: serial, parallel
 * Internal Options: Up to 640K RAM through add-on RAM card
 * Misc: The hardware was not 100% PC compatible so non BIOS based software would not run. 50.000+ units sold
 *
 * TODO:
 * - Add keyboard, first HLE as in pc.cpp and then LLE when the keyboard controller is dumped
 * - Add the on-board FDC and boot DOS 1.xx
 * - Complete the Ericsson 1070 MDA ISA board and test all the graphics modes inclusing 640x400 (aka HR)
 * - Add the Ericsson 1065 HDC and boot from a hard drive
 *
 * Credits: The driver code is inspired from m24.cpp, myb3k.cpp and genpc.cpp. Information about the EPC has 
 *          been contributed by many, mainly the people at Dalby Computer museum http://www.datormuseum.se/
 *          A dead pcb was donated by rfka01 and rom dumps by ZnaxQue@sweclockers.com 
 *
 ************************************************************************************************************/
/*
 Links:
 ------

 */

#include "emu.h"

#include "cpu/i86/i86.h"
#include "machine/am9517a.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#define LOG_PPI     (1U << 1)
#define LOG_PIT     (1U << 2)
#define LOG_PIC     (1U << 3)
#define LOG_KBD     (1U << 4)
#define LOG_DMA     (1U << 5)
#define LOG_IRQ     (1U << 6)

#define VERBOSE (LOG_PPI|LOG_KBD|LOG_PIT|LOG_DMA|LOG_IRQ)
#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGPPI(...)  LOGMASKED(LOG_PPI,  __VA_ARGS__)
#define LOGPIT(...)  LOGMASKED(LOG_PIT,  __VA_ARGS__)
#define LOGPIC(...)  LOGMASKED(LOG_PIC,  __VA_ARGS__)
#define LOGKBD(...)  LOGMASKED(LOG_KBD,  __VA_ARGS__)
#define LOGDMA(...)  LOGMASKED(LOG_DMA,  __VA_ARGS__)
#define LOGIRQ(...)  LOGMASKED(LOG_IRQ,  __VA_ARGS__)

class epc_state : public driver_device
{
public:
	epc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_isabus(*this, "isabus")
		, m_dma8237a(*this, "dma8237")
		, m_ppi8255(*this, "ppi8255")
		, m_io_dsw(*this, "DSW")
		, m_kbd8251(*this, "kbd8251")
		, m_pic8259(*this, "pic8259")
		, m_pit8253(*this, "pit8253")
		, m_speaker(*this, "speaker")
	{ }

	void epc(machine_config &config);

  
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<i8086_cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<isa8_device> m_isabus;

	// DMA
	DECLARE_WRITE_LINE_MEMBER(dma_tc_w);
	DECLARE_WRITE_LINE_MEMBER(dreq0_ck_w);
	DECLARE_WRITE_LINE_MEMBER( epc_dma_hrq_changed );
	DECLARE_WRITE_LINE_MEMBER( epc_dma8237_out_eop );
	DECLARE_READ8_MEMBER( epc_dma_read_byte );
	DECLARE_WRITE8_MEMBER( epc_dma_write_byte );
	template <int Channel> uint8_t epc_dma8237_io_r(offs_t offset);
	template <int Channel> void epc_dma8237_io_w(offs_t offset, uint8_t data);
	template <int Channel> DECLARE_WRITE_LINE_MEMBER(epc_dack_w);
	required_device<am9517a_device> m_dma8237a;
	uint8_t m_dma_segment[4];
	uint8_t m_dma_active;
	int m_dma_channel;
	bool m_tc;
	bool m_dreq0_ck;

	// PPI
	required_device<i8255_device> m_ppi8255;
	DECLARE_WRITE8_MEMBER(ppi_portb_w);
	DECLARE_READ8_MEMBER(ppi_portc_r);
	uint8_t m_ppi_portb;
	required_ioport m_io_dsw;

	// Keyboard Controller/USART
	required_device<i8251_device> m_kbd8251;

	// Interrupt Controller
	required_device<pic8259_device> m_pic8259;
	DECLARE_WRITE_LINE_MEMBER(int_w);

	// Timer
	required_device<pit8253_device> m_pit8253;

	// Speaker
	DECLARE_WRITE_LINE_MEMBER(speaker_ck_w);
	void update_speaker();
	required_device<speaker_sound_device> m_speaker;

	void epc_map(address_map &map);
	void epc_io(address_map &map);
};

void epc_state::epc_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void epc_state::epc_io(address_map &map)
{
	map(0x0000, 0x000f).lrw8("dma8237_rw",
		[this](offs_t offset) -> uint8_t
		{
			uint8_t data = m_dma8237a->read(offset);
			LOGDMA("dma8237_r %04x\n", offset);
			return data;
		},
		[this](offs_t offset, uint8_t data)
		{
			LOGDMA("dma8237_w %04x: %02x\n", offset, data);
			m_dma8237a->write(offset, data);
		}
	);

	map(0x0020, 0x0021).mirror(0xe).lrw8("pic8259_rw",
		[this](offs_t offset) -> uint8_t
		{
			uint8_t data = m_pic8259->read(offset);
			LOGPIC("pic8259_r %04x\n", offset);
			return data;
		},
		[this](offs_t offset, uint8_t data)
		{
			LOGPIC("pic8259_w %04x: %02x\n", offset, data);
			m_pic8259->write(offset, data);
		}
	);
	
	map(0x0040, 0x0043).mirror(0xc).lrw8("pit8253_rw",
		[this](offs_t offset) -> uint8_t
		{
			uint8_t data = m_pit8253->read(offset);
			LOGPIT("pit8253_r %04x\n", offset);
			return data;
		},
		[this](offs_t offset, uint8_t data)
		{
			LOGPIT("pit8253_w %04x: %02x\n", offset, data);
			m_pit8253->write(offset, data);
		}
	);

	map(0x0060, 0x0060).lr8("kbd_8251_data_r",
		[this]() -> uint8_t
		{
			uint8_t data = m_kbd8251->data_r();
			LOGKBD("kbd8251_r %02x\n", data);
			return data;
		}
	);

	map(0x0061, 0x0063).lrw8("ppi8255_rw",
		[this](offs_t offset) -> uint8_t
		{
			uint8_t data = m_ppi8255->read(offset);
			LOGPPI("ppi8255_r %04x\n", offset);
			return data;
		},
		[this](offs_t offset, uint8_t data)
		{
			LOGPPI("ppi8255_w %04x: %02x\n", offset, data);
			m_ppi8255->write(offset, data);
		}
	);

	map(0x0070, 0x0070).lw8("i8251_data_w",
		[this](offs_t offset, uint8_t data)
		{
			LOGKBD("kbd8251_w %04x: %02x\n", offset, data);
			m_kbd8251->data_w(data);
		}
	);

	map(0x0071, 0x0071).lrw8("kbd_8251_stat_ctrl_rw",
		[this](offs_t offset) -> uint8_t
		{
			uint8_t stat = m_kbd8251->status_r(); 
			LOGKBD("kbd8251_status_r %04x: %02x\n", offset, stat);
			return stat;
		},
		[this](offs_t offset, uint8_t data)
		{
			LOGKBD("kbd8251_w %04x: %02x\n", offset, data);
			m_kbd8251->control_w(data);
		}
	);

	map(0x0080, 0x0083).mirror(0xc).lw8("dma_segement_w",
		[this](offs_t offset, uint8_t data)
		{
			LOGDMA("dma_segment_w %04x: %02x\n", offset, data);
			m_dma_segment[offset] = data & 0x0f;
		}
	);

	map(0x00a0, 0x00a1).mirror(0xe).lw8("nmi_enable_w",
		[this](offs_t offset, uint8_t data)
		{
			LOGIRQ("nmi_enable_w %04x: %02x\n", offset, data);
		}
	);
}

void epc_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->size() - 1, m_ram->pointer());

	std::fill_n(&m_dma_segment[0], 4, 0);
	m_dma_active = 0;
	m_tc = false;
	m_dreq0_ck = true;

	save_item(NAME(m_dma_segment));
	save_item(NAME(m_dma_active));
	save_item(NAME(m_tc));
	save_item(NAME(m_ppi_portb));
	save_item(NAME(m_dreq0_ck));
}

void epc_state::machine_reset()
{
	m_ppi_portb = 0;
}

template <int Channel>
uint8_t epc_state::epc_dma8237_io_r(offs_t offset)
{
	return m_isabus->dack_r(Channel);
}

template <int Channel>
void epc_state::epc_dma8237_io_w(offs_t offset, uint8_t data)
{
	m_isabus->dack_w(Channel, data);
}

template <int Channel>
WRITE_LINE_MEMBER(epc_state::epc_dack_w)
{
	m_isabus->dack_line_w(Channel, state);

	if (!state)
	{
		m_dma_active |= 1 << Channel;
		if (Channel == 0)
			m_dma8237a->dreq0_w(0);
		if (m_tc)
			m_isabus->eop_w(Channel, ASSERT_LINE);
	}
	else
	{
		m_dma_active &= ~(1 << Channel);
		if (m_tc)
			m_isabus->eop_w(Channel, CLEAR_LINE);
	}

	// Update active channel, if there is one
	m_dma_channel = -1;
	for (int channel = 0; channel < 4; channel++)
	{
		if (BIT(m_dma_active, channel))
		{
			m_dma_channel = channel;
			break;
		}
	}
}

WRITE_LINE_MEMBER(epc_state::dma_tc_w)
{
	m_tc = (state == ASSERT_LINE);
	for (int channel = 0; channel < 4; channel++)
		if (BIT(m_dma_active, channel))
			m_isabus->eop_w(channel, state);
}

WRITE_LINE_MEMBER(epc_state::dreq0_ck_w)
{
	if (state && !m_dreq0_ck && !BIT(m_dma_active, 0))
		m_dma8237a->dreq0_w(1);

	m_dreq0_ck = state;
}

// TODO: Check pcb or schematics to hook up speaker correctly
WRITE_LINE_MEMBER(epc_state::speaker_ck_w)
{
#if 0
	if (state)
		m_ctrlport_b |= 0x20;
	else
		m_ctrlport_b &= 0xdf;
#endif
	update_speaker();
}

void epc_state::update_speaker()
{
#if 0
	if (BIT(m_ctrlport_a, 1) && BIT(m_ctrlport_b, 5))
	{
		m_speaker->level_w(1);
		m_ctrlport_b &= 0xef;
	}
	else
	{
		m_speaker->level_w(0);
		m_ctrlport_b |= 0x10;
	}
#endif
}

/**********************************************************
 *
 * PPI8255 interface
 *
 *
 * PORT A (not used)
 *
 * Reads of port A is shadowed by 8251As read register! 
 * The keyboard connection is thereby bidirectional enabling 
 * keyboard led control.
 *
 * PORT B (output)
 * 0 - PB0 -             - Control signal for the sound generator (short beeps)
 * 1 - PB1 -             - Control signal for the sound generator
 * 2 - PB2 -             - Unused
 * 3 - PB3 -             - Data select for the configuration switches 0=SW1-4 1=SW5-8
 * 4 - PB4 - *           - Enable ram parity check
 * 5 - PB5 - *           - Enable expansion I/O check
 * 6 - PB6 - *           - Keyboard reset
 * 7 - PB7 -             - Reset keyboard interrupt
 *
 * PORT C
 * 0 - PC0 -         - Dipswitch SW 1/5 PB3=0/PB3=1
 * 1 - PC1 -         - Dipswitch SW 2/6 PB3=0/PB3=1
 * 2 - PC2 -         - Dipswitch SW 3/7 PB3=0/PB3=1
 * 3 - PC3 -         - Dipswitch SW 4/8 PB3=0/PB3=1
 * 4 - PC4 - SPK     - Speaker/cassette data
 * 5 - PC5 - I/OCHCK - Expansion I/O check result
 * 6 - PC6 - T/C2OUT - Output of 8253 timer 2
 * 7 - PC7 - PCK     - Parity check result
 *
 * Ericsson PC SW:
 * 1   - Not used. Must be set to OFF
 * 2   - OFF - 8087 present
 *       ON  - No 8087 present *)
 * 3   - Not Used. Don't care but OFF *)
 * 4   - Not Used. Must be set to ON
 * 5+6 - Used to select display
 *       OFF OFF - Monochrome HR graphics monitor 3111 installed + 1020 color secondary monitor
 *       ON  OFF - Monochrome HR graphics monitor 3111 installed + optional 1020 color main monitor *)
 *       OFF ON  - Not used
 *       ON  ON  - Not used
 * 7+8 - Used to select number of disk drives
 *       OFF OFF - Not used
 *       ON  OFF - Not used
 *       OFF ON  - two disk drives, system units 1030-1 and 1030-2
 *       ON  ON  - one disk drive, system units 1030-3, 1030-4, 1031-1 and 1031-2
 *
 *           *)  - Factory settings
 *
 **********************************************************/

READ8_MEMBER( epc_state::ppi_portc_r )
{
	uint8_t data;

	// Read 4 configurations dip switches depending on PB3
	data = (m_io_dsw->read() >> ((m_ppi_portb & 0x08) ? 4 : 0) & 0x0f);

	// TODO: verify what PC4-PC7 is used for, if anything

	LOGPPI("PPI Port C read: %02x\n", data);

	return data;
}

WRITE8_MEMBER( epc_state::ppi_portb_w )
{
	LOGPPI("PPI Port B write: %02x\n", data);
	LOGPPI(" PB0 - Enable beeper             : %d\n", (data & 0x01)  ? 1 : 0);
	LOGPPI(" PB1 - Beeper data               : %d\n", (data & 0x02)  ? 1 : 0);
	LOGPPI(" PB2 - Unused                    : %d\n", (data & 0x04)  ? 1 : 0);
	LOGPPI(" PB3 - Port C dip switch select  : %d\n", (data & 0x08)  ? 1 : 0);
	LOGPPI(" PB4 - RAM parity enable         : %d\n", (data & 0x10)  ? 1 : 0);
	LOGPPI(" PB5 - ISA error checking enable : %d\n", (data & 0x20)  ? 1 : 0);
	LOGPPI(" PB6 - Reset keyboard            : %d\n", (data & 0x40)  ? 1 : 0);
	LOGPPI(" PB7 - Reset keyboard interrupt  : %d\n", (data & 0x80)  ? 1 : 0);

	m_ppi_portb = data;
}

WRITE_LINE_MEMBER(epc_state::int_w)
{
	m_maincpu->set_input_line(0, state);
}

static void epc_isa8_cards(device_slot_interface &device)
{
	// device.option_add("epc_hdc1065", ISA8_EPC_HDC1065);
	// device.option_add("epc_mb1080", ISA8_EPC_MB1080);
}

void epc_state::epc(machine_config &config)
{
	I8088(config, m_maincpu, XTAL(14'318'181) / 3.0); // It's a TEW osc marked 14'3181, but close enough
	m_maincpu->set_addrmap(AS_PROGRAM, &epc_state::epc_map);
	m_maincpu->set_addrmap(AS_IO, &epc_state::epc_io);

	// DMA
	AM9517A(config, m_dma8237a, XTAL(14'318'181) / 3.0);
	m_dma8237a->out_hreq_callback().set(FUNC(epc_state::epc_dma_hrq_changed));
	m_dma8237a->out_eop_callback().set(FUNC(epc_state::dma_tc_w));
	m_dma8237a->in_memr_callback().set(FUNC(epc_state::epc_dma_read_byte));
	m_dma8237a->out_memw_callback().set(FUNC(epc_state::epc_dma_write_byte));
	m_dma8237a->in_ior_callback<1>().set(FUNC(epc_state::epc_dma8237_io_r<1>));
	m_dma8237a->in_ior_callback<2>().set(FUNC(epc_state::epc_dma8237_io_r<2>));
	m_dma8237a->in_ior_callback<3>().set(FUNC(epc_state::epc_dma8237_io_r<3>));
	m_dma8237a->out_iow_callback<0>().set(FUNC(epc_state::epc_dma8237_io_w<0>));
	m_dma8237a->out_iow_callback<1>().set(FUNC(epc_state::epc_dma8237_io_w<1>));
	m_dma8237a->out_iow_callback<2>().set(FUNC(epc_state::epc_dma8237_io_w<2>));
	m_dma8237a->out_iow_callback<3>().set(FUNC(epc_state::epc_dma8237_io_w<3>));
	m_dma8237a->out_dack_callback<0>().set(FUNC(epc_state::epc_dack_w<0>));
	m_dma8237a->out_dack_callback<1>().set(FUNC(epc_state::epc_dack_w<1>));
	m_dma8237a->out_dack_callback<2>().set(FUNC(epc_state::epc_dack_w<2>));
	m_dma8237a->out_dack_callback<3>().set(FUNC(epc_state::epc_dack_w<3>));

	// Keyboard USART
	I8251( config, m_kbd8251, XTAL(15'974'400) / 8 ); // Clock needs verifying

	// Interrupt Controller
	PIC8259(config, m_pic8259);
	m_pic8259->in_sp_callback().set_constant(1);
	m_pic8259->out_int_callback().set(FUNC(epc_state::int_w));

	// Parallel port
	I8255A(config, m_ppi8255);
	m_ppi8255->out_pa_callback().set([this] (uint8_t data) { LOGPPI("PPI: write %02x to unused Port A\n", data); } ); // Port A us not used
	m_ppi8255->out_pb_callback().set(FUNC(epc_state::ppi_portb_w));
	m_ppi8255->in_pc_callback().set(FUNC(epc_state::ppi_portc_r));

	// Timer
	PIT8253(config, m_pit8253);
	m_pit8253->set_clk<0>((XTAL(14'318'181) / 3.0) / 2.0 ); // Clocked at half the CPU speed
	m_pit8253->set_clk<1>((XTAL(14'318'181) / 3.0) / 2.0 );
	m_pit8253->set_clk<2>((XTAL(14'318'181) / 3.0) / 2.0 );
	m_pit8253->out_handler<0>().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	m_pit8253->out_handler<1>().set(FUNC(epc_state::dreq0_ck_w));
	m_pit8253->out_handler<2>().set(FUNC(epc_state::speaker_ck_w));

	// Speaker
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	// ISA bus
	ISA8(config, m_isabus,  XTAL(14'318'181) / 3.0);
	m_isabus->set_memspace(m_maincpu, AS_PROGRAM);
	m_isabus->set_iospace(m_maincpu, AS_IO);
	//m_isabus->irq2_callback().set(m_pic8259, FUNC(pic8259_device::ir2_w)); // Reserved in service manual
	m_isabus->irq3_callback().set(m_pic8259, FUNC(pic8259_device::ir3_w));
	m_isabus->irq4_callback().set(m_pic8259, FUNC(pic8259_device::ir4_w));
	m_isabus->irq5_callback().set(m_pic8259, FUNC(pic8259_device::ir5_w));
	m_isabus->irq6_callback().set(m_pic8259, FUNC(pic8259_device::ir6_w));
	m_isabus->irq7_callback().set(m_pic8259, FUNC(pic8259_device::ir7_w));
	m_isabus->drq1_callback().set(m_dma8237a, FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_dma8237a, FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_dma8237a, FUNC(am9517a_device::dreq3_w));
	// m_isabus->iochck_callback().set(FUNC(epc_state::chck_w)); // Need schematics to hook up

	//ISA8_SLOT(config, "isa1", 0, m_isabus, epc_isa8_cards, nullptr, false);
	// This is wrong and will be replaced once the Ericsson Monochrome HR Graphics Board 1070
	// is implemented and working but works for simple output meanwhile
	ISA8_SLOT(config, "isa1", 0, m_isabus, pc_isa8_cards, "epc_mda", false); 

	ISA8_SLOT(config, "isa2", 0, m_isabus, epc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa3", 0, m_isabus, epc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa4", 0, m_isabus, epc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa5", 0, m_isabus, epc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa6", 0, m_isabus, epc_isa8_cards, nullptr, false);
	
	// System board has 128kB memory with parity, expansion can be achieved through the
	// 128kB Memory Expansion Board 1090 and/or the 128kB Multifunction Board MB1080-001
	// and/or the 384kB MB1080-002. The MB1080 DRAM might need to be dynamically added as
	// base address and also a video memory hole is configuarable. 
	RAM(config, m_ram).set_default_size("128K").set_extra_options("256K, 384K, 512K, 640K");

}

WRITE_LINE_MEMBER( epc_state::epc_dma_hrq_changed )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237a->hack_w(state);
}


READ8_MEMBER( epc_state::epc_dma_read_byte )
{
	if ((m_dma_active & 0x0f) == 0)
		return 0xff;

	address_space &spaceio = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_segment[m_dma_channel]) << 16) & 0x0F0000;
	return spaceio.read_byte( page_offset + offset);
}


WRITE8_MEMBER( epc_state::epc_dma_write_byte )
{
	if(m_dma_channel == -1)
		return;
	address_space &spaceio = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_segment[m_dma_channel]) << 16) & 0x0F0000;

	spaceio.write_byte( page_offset + offset, data);
}


static INPUT_PORTS_START( epc_ports )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Not used")
	PORT_DIPSETTING(    0x00, "ON - Don't use")
	PORT_DIPSETTING(    0x01, "OFF - Factory Setting")
	PORT_DIPNAME( 0x02, 0x00, "8087 installed")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x02, DEF_STR(Yes) )
	PORT_DIPNAME( 0x04, 0x04, "Not used")
	PORT_DIPSETTING(    0x00, "ON - Don't care")
	PORT_DIPSETTING(    0x04, "OFF - Factory Setting")
	PORT_DIPNAME( 0x08, 0x00, "Not used")
	PORT_DIPSETTING(    0x00, "ON - Factory Setting")
	PORT_DIPSETTING(    0x08, "OFF - Don't use")
	PORT_DIPNAME( 0x30, 0x30, "Main monitor")
	PORT_DIPSETTING(    0x00, "Not used" )
	PORT_DIPSETTING(    0x10, "Optional 1020 color" )
	PORT_DIPSETTING(    0x20, "Not used" )
	PORT_DIPSETTING(    0x30, "3111 HR Monochrome" )
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "Not used" )
	PORT_DIPSETTING(    0xc0, "Not used" )
INPUT_PORTS_END

ROM_START( epc )
	ROM_REGION(0x10000,"bios", 0)
	ROM_DEFAULT_BIOS("p860110")
	ROM_SYSTEM_BIOS(0, "p840705", "P840705")
	ROMX_LOAD("ericsson_8088.bin", 0xe000, 0x2000, CRC(3953c38d) SHA1(2bfc1f1d11d0da5664c3114994fc7aa3d6dd010d), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "p860110", "P860110")
	ROMX_LOAD("epcbios1.bin",  0xe000, 0x02000, CRC(79a83706) SHA1(33528c46a24d7f65ef5a860fbed05afcf797fc55), ROM_BIOS(1))
	ROMX_LOAD("epcbios2.bin",  0xa000, 0x02000, CRC(3ca764ca) SHA1(02232fedef22d31a641f4b65933b9e269afce19e), ROM_BIOS(1))
	ROMX_LOAD("epcbios3.bin",  0xc000, 0x02000, CRC(70483280) SHA1(b44b09da94d77b0269fc48f07d130b2d74c4bb8f), ROM_BIOS(1))
ROM_END


COMP( 1985, epc,     0,      0,      epc,     epc_ports, epc_state, empty_init,    "Ericsson Information System",     "Ericsson PC" ,          MACHINE_NOT_WORKING )
//COMP( 1985, eppc,   ibm5150, 0,  pccga,         pccga,  pc_state, empty_init,    "Ericsson Information System",     "Ericsson Portable PC",  MACHINE_NOT_WORKING )
