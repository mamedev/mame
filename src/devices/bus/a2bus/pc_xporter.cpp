// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    pcxporter.cpp

    Implementation of the Applied Engineering PC Transporter card
    Preliminary version by R. Belmont, additional reverse-engineering by Peter Ferrie
    PC-XT portion adapted from genpc.cpp by Wilbert Pol and Miodrag Milanovic

    The PC Transporter is basically a PC-XT on an Apple II card.
    Features include:
    - V30 CPU @ 4.77 MHz
    - 768K of RAM, which defines the V30 address space from 0x00000 to 0xBFFFF
      and is fully read/writable by the Apple's CPU.
    - Usual XT hardware, mostly inside custom ASICs.  There's a discrete
      NEC uPD71054 (i8254-compatible PIT) though.
    - CGA-compatible video, output to a separate CGA monitor or NTSC-compliant analog
      RGB monitor (e.g. the IIgs RGB monitor).
    - XT-compatible keyboard interface.
    - PC-style floppy controller: supports 360K 5.25" disks and 720K 3.5" disks
    - HDD controller which is redirected to a file on the Apple's filesystem

    The V30 BIOS is downloaded by the Apple; the Apple also writes text to the CGA screen prior to
    the V30's being launched.

    The board was developed by The Engineering Department, a company made up mostly of early Apple
    engineers including Apple /// designer Dr. Wendall Sander and ProDOS creator Dick Huston.

    Software and user documentation at:
    http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/CPU/AE%20PC%20Transporter/

    Notes:
        Registers live at CFxx; access CFFF to clear C800 reservation,
        then read Cn00 to map C800-CFFF first.

        PC RAM from 0xA0000-0xAFFFF is where the V30 BIOS is downloaded,
        plus used for general storage by the system.  This is mirrored at 
        Fxxxx on the V30 so that it can boot.
        RAM from 0xB0000-0xBFFFF is the CGA framebuffer as usual.

		C800-CBFF?: RAM, used as scratchpad space by the software
        CF00: PC memory pointer (bits 0-7)
        CF01: PC memory pointer (bits 8-15)
        CF02: PC memory pointer (bits 16-23)
        CF03: read/write PC memory at the pointer and increment the pointer
        CF04: read/write PC memory at the pointer and *don't* increment the pointer
        CF2C: CGA 6845 register select (port 3D0/3D2/3D4/3D6)
        CF2D: CGA 6845 data read/write (port 3D1/3D3/3D5/3D7)
        CF2E: CGA mode select (port 3D8)
        CF2F: CGA color select (port 3D9)
        CF30: control/flags: bit 4 = 1 to release reset on V30, 5 = 1 to release halt on V30
        CF31: control/flags: bit 4 = 1 to assert reset on V30, 5 = 1 to assert halt on V30

    TODO:
    	- Code at $70b0-$70c5 waits for the V30 to answer FPU presence.
        - What's going on at CF0E/CF0F?
    	- The manual indicates there is no ROM; special drivers installed into ProDOS 8
    	  provide the RAMdisk and A2-accessing-PC-drives functionality.
          
*********************************************************************/

#include "pc_xporter.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_PCXPORTER = &device_creator<a2bus_pcxporter_device>;

static ADDRESS_MAP_START( pc_map, AS_PROGRAM, 16, a2bus_pcxporter_device )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START(pc_io, AS_IO, 16, a2bus_pcxporter_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE8("dma8237", am9517a_device, read, write, 0xffff)
	AM_RANGE(0x0020, 0x002f) AM_DEVREADWRITE8("pic8259", pic8259_device, read, write, 0xffff)
	AM_RANGE(0x0040, 0x004f) AM_DEVREADWRITE8("pit8253", pit8253_device, read, write, 0xffff)
	AM_RANGE(0x0060, 0x006f) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0xffff)
	AM_RANGE(0x0080, 0x008f) AM_WRITE8(pc_page_w, 0xffff)
	AM_RANGE(0x00a0, 0x00a1) AM_WRITE8(nmi_enable_w, 0xffff)
ADDRESS_MAP_END

MACHINE_CONFIG_FRAGMENT( pcxporter )
	MCFG_CPU_ADD("v30", V30, XTAL_14_31818MHz/2)	// 7.16 MHz as per manual
	MCFG_CPU_PROGRAM_MAP(pc_map)
	MCFG_CPU_IO_MAP(pc_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_14_31818MHz/12) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL_14_31818MHz/12) /* dram refresh */
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(a2bus_pcxporter_device, pc_pit8253_out1_changed))
	MCFG_PIT8253_CLK2(XTAL_14_31818MHz/12) /* pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(a2bus_pcxporter_device, pc_pit8253_out2_changed))

	MCFG_DEVICE_ADD( "dma8237", PCXPORT_DMAC, XTAL_14_31818MHz/2 )
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(a2bus_pcxporter_device, pc_dma_hrq_changed))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(a2bus_pcxporter_device, pc_dma8237_out_eop))
	MCFG_I8237_IN_MEMR_CB(READ8(a2bus_pcxporter_device, pc_dma_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(a2bus_pcxporter_device, pc_dma_write_byte))
	MCFG_I8237_IN_IOR_1_CB(READ8(a2bus_pcxporter_device, pc_dma8237_1_dack_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(a2bus_pcxporter_device, pc_dma8237_2_dack_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(a2bus_pcxporter_device, pc_dma8237_3_dack_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(a2bus_pcxporter_device, pc_dma8237_0_dack_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(a2bus_pcxporter_device, pc_dma8237_1_dack_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(a2bus_pcxporter_device, pc_dma8237_2_dack_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(a2bus_pcxporter_device, pc_dma8237_3_dack_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(a2bus_pcxporter_device, pc_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(a2bus_pcxporter_device, pc_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(a2bus_pcxporter_device, pc_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(a2bus_pcxporter_device, pc_dack3_w))

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("v30", 0), VCC, NULL )

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(a2bus_pcxporter_device, pc_ppi_porta_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(a2bus_pcxporter_device, pc_ppi_portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(a2bus_pcxporter_device, pc_ppi_portc_r))

	MCFG_DEVICE_ADD("isa", ISA16, 0)
	MCFG_ISA16_CPU("^v30")
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE("pic8259", pic8259_device, ir2_w))
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE("pic8259", pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE("pic8259", pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE("pic8259", pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ6_CB(DEVWRITELINE("pic8259", pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE("pic8259", pic8259_device, ir7_w))
	MCFG_ISA_OUT_DRQ1_CB(DEVWRITELINE("dma8237", am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(DEVWRITELINE("dma8237", am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(DEVWRITELINE("dma8237", am9517a_device, dreq3_w))

	MCFG_DEVICE_ADD("pc_kbdc", PC_KBDC, 0)
	MCFG_PC_KBDC_OUT_CLOCK_CB(WRITELINE(a2bus_pcxporter_device, keyboard_clock_w))
	MCFG_PC_KBDC_OUT_DATA_CB(WRITELINE(a2bus_pcxporter_device, keyboard_data_w))
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)
	
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_ISA16_SLOT_ADD("isa", "isa1", pc_isa8_cards, "cga", true)
	MCFG_ISA16_SLOT_ADD("isa", "isa2", pc_isa8_cards, "fdc_xt", true)
MACHINE_CONFIG_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_pcxporter_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcxporter );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_pcxporter_device::a2bus_pcxporter_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_v30(*this, "v30"),
	m_pic8259(*this, "pic8259"),
	m_dma8237(*this, "dma8237"),
	m_pit8253(*this, "pit8253"),
	m_ppi8255(*this, "ppi8255"),
	m_speaker(*this, "speaker"),
	m_isabus(*this, "isa"),
	m_pc_kbdc(*this, "pc_kbdc")
{
}

a2bus_pcxporter_device::a2bus_pcxporter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_PCXPORTER, "Applied Engineering PC Transporter", tag, owner, clock, "a2pcxport", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_v30(*this, "v30"),
	m_pic8259(*this, "pic8259"),
	m_dma8237(*this, "dma8237"),
	m_pit8253(*this, "pit8253"),
	m_ppi8255(*this, "ppi8255"),
	m_speaker(*this, "speaker"),
	m_isabus(*this, "isa"),
	m_pc_kbdc(*this, "pc_kbdc")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_pcxporter_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	memset(m_ram, 0, 768*1024);
	memset(m_regs, 0, 0x400);
	m_offset = 0;
	m_reset_during_halt = false;

	save_item(NAME(m_ram));
	save_item(NAME(m_regs));
	save_item(NAME(m_offset));
	
	m_v30->space(AS_PROGRAM).install_ram(0, 0xaffff, m_ram);
	m_v30->space(AS_PROGRAM).install_rom(0xf0000, 0xfffff, &m_ram[0xa0000]);
	
	m_pcmem_space = &m_v30->space(AS_PROGRAM);
	m_pcio_space = &m_v30->space(AS_IO);
}

void a2bus_pcxporter_device::device_reset()
{
	m_v30->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_reset_during_halt = false;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_pcxporter_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		default:
			printf("Read c0n%x (PC=%x)\n", offset, space.device().safe_pc());
			break;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_pcxporter_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		default:
			printf("Write %02x to c0n%x (PC=%x)\n", data, offset, space.device().safe_pc());
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_pcxporter_device::read_cnxx(address_space &space, UINT8 offset)
{
	// read only to trigger C800?
	return 0xff;
}

void a2bus_pcxporter_device::write_cnxx(address_space &space, UINT8 offset, UINT8 data)
{
	printf("Write %02x to cn%02x (PC=%x)\n", data, offset, space.device().safe_pc());
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_pcxporter_device::read_c800(address_space &space, UINT16 offset)
{
//  printf("Read C800 at %x\n", offset + 0xc800);

	if (offset < 0x400)
	{
		return m_c800_ram[offset];
	}
	else
	{
		UINT8 rv;

		switch (offset)
		{
			case 0x700:
				return m_offset & 0xff;

			case 0x701:
				return (m_offset >> 8) & 0xff;

			case 0x702:
				return (m_offset >> 16) & 0xff;

			case 0x703: // read with increment
				rv = m_ram[m_offset];
				// don't increment if the debugger's reading
				if (!space.debugger_access())
				{
					m_offset++;
				}
				return rv;

			case 0x704: // read w/o increment
				rv = m_ram[m_offset];
				return rv;			
				
			default:
				//printf("Read $C800 at %x\n", offset + 0xc800);
				break;					
		}

		return m_regs[offset];
	}
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_pcxporter_device::write_c800(address_space &space, UINT16 offset, UINT8 data)
{
	if (offset < 0x400)
	{
		m_c800_ram[offset] = data;
	}
	else
	{
		switch (offset)
		{
			case 0x700:
				m_offset &= ~0xff;
				m_offset |= data;
//              printf("offset now %x (PC=%x)\n", m_offset, space.device().safe_pc());
				break;

			case 0x701:
				m_offset &= ~0xff00;
				m_offset |= (data<<8);
//              printf("offset now %x (PC=%x)\n", m_offset, space.device().safe_pc());
				break;

			case 0x702:
				m_offset &= ~0xff0000;
				m_offset |= (data<<16);
//              printf("offset now %x (PC=%x)\n", m_offset, space.device().safe_pc());
				break;

			case 0x703: // write w/increment
				m_ram[m_offset] = data;
				if (m_offset >= 0xb0000 && m_offset <= 0xb7fff) m_pcmem_space->write_byte(m_offset+0x8000, data);
				if (m_offset >= 0xb8000 && m_offset <= 0xbffff) m_pcmem_space->write_byte(m_offset, data);
				m_offset++;
				break;

			case 0x704: // write w/o increment
				m_ram[m_offset] = data;
				if (m_offset >= 0xb0000 && m_offset <= 0xb7fff) m_pcmem_space->write_byte(m_offset+0x8000, data);
				if (m_offset >= 0xb8000 && m_offset <= 0xbffff) m_pcmem_space->write_byte(m_offset, data);
				break;
				
			case 0x72c:	// CGA 6845 register select
				m_pcio_space->write_byte(0x3d6, data);
				m_6845_reg = data;
				break;
			
			case 0x72d:	// CGA 6845 data read/write
				// HACK: adjust the 40 column mode the 6502 sets to
				// be more within specs.
				switch (m_6845_reg)
				{
					case 0:
						if (data == 0x3e)
						{
							data -= 6;
						}
						break;

					case 2:
						if (data == 0x29)
						{
							data += 4;
						}
						break;

					case 3:
						if (data == 0x5)
						{
							data *= 2;
						}
						break;
				}

				m_pcio_space->write_byte(0x3d7, data);
				break;
			
			case 0x72e:	// CGA mode select
				m_pcio_space->write_byte(0x3d8, data);
				break;

			case 0x72f: // CGA color select
				m_pcio_space->write_byte(0x3d9, data);
				break;
				
			case 0x730:	// control 1
				if (data & 0x10) { m_v30->set_input_line(INPUT_LINE_RESET, CLEAR_LINE); m_reset_during_halt = true; }
				if (data & 0x20)
				{ 
					if (m_reset_during_halt)
					{
						m_v30->reset();
						m_reset_during_halt = false;
					}
				
					m_v30->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				}
				break;
				
			case 0x731:	// control 2
				if (data & 0x10) m_v30->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
				if (data & 0x20) m_v30->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
				break;
				
			default:
//				printf("%02x to C800 at %x\n", data, offset + 0xc800);
				m_regs[offset] = data;
				break;
		}
	}
}

READ16_MEMBER(a2bus_pcxporter_device::pc_bios_r)
{
	return m_ram[offset+0xa0000] | (m_ram[offset+0xa0001]<<8);
}

/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/

WRITE8_MEMBER( a2bus_pcxporter_device::pc_page_w)
{
	switch(offset % 4)
	{
	case 1:
		m_dma_offset[2] = data;
		break;
	case 2:
		m_dma_offset[3] = data;
		break;
	case 3:
		m_dma_offset[0] = m_dma_offset[1] = data;
		break;
	}
}


WRITE_LINE_MEMBER( a2bus_pcxporter_device::pc_dma_hrq_changed )
{
	m_v30->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237->hack_w(state);
}


READ8_MEMBER( a2bus_pcxporter_device::pc_dma_read_byte )
{
	if(m_dma_channel == -1)
		return 0xff;
	address_space &spaceio = m_v30->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_offset[m_dma_channel]) << 16) & 0x0F0000;
	return spaceio.read_byte( page_offset + offset);
}


WRITE8_MEMBER( a2bus_pcxporter_device::pc_dma_write_byte )
{
	if(m_dma_channel == -1)
		return;
	address_space &spaceio = m_v30->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_offset[m_dma_channel]) << 16) & 0x0F0000;

	spaceio.write_byte( page_offset + offset, data);
}


READ8_MEMBER( a2bus_pcxporter_device::pc_dma8237_1_dack_r )
{
	return m_isabus->dack_r(1);
}

READ8_MEMBER( a2bus_pcxporter_device::pc_dma8237_2_dack_r )
{
	return m_isabus->dack_r(2);
}


READ8_MEMBER( a2bus_pcxporter_device::pc_dma8237_3_dack_r )
{
	return m_isabus->dack_r(3);
}


WRITE8_MEMBER( a2bus_pcxporter_device::pc_dma8237_1_dack_w )
{
	m_isabus->dack_w(1,data);
}

WRITE8_MEMBER( a2bus_pcxporter_device::pc_dma8237_2_dack_w )
{
	m_isabus->dack_w(2,data);
}


WRITE8_MEMBER( a2bus_pcxporter_device::pc_dma8237_3_dack_w )
{
	m_isabus->dack_w(3,data);
}


WRITE8_MEMBER( a2bus_pcxporter_device::pc_dma8237_0_dack_w )
{
	m_u73_q2 = 0;
	m_dma8237->dreq0_w( m_u73_q2 );
}


WRITE_LINE_MEMBER( a2bus_pcxporter_device::pc_dma8237_out_eop )
{
	m_cur_eop = state == ASSERT_LINE;
	if(m_dma_channel != -1 && m_cur_eop)
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
}

void a2bus_pcxporter_device::pc_select_dma_channel(int channel, bool state)
{
	if(!state) {
		m_dma_channel = channel;
		if(m_cur_eop)
			m_isabus->eop_w(channel, ASSERT_LINE );

	} else if(m_dma_channel == channel) {
		m_dma_channel = -1;
		if(m_cur_eop)
			m_isabus->eop_w(channel, CLEAR_LINE );
	}
}

WRITE_LINE_MEMBER( a2bus_pcxporter_device::pc_dack0_w ) { pc_select_dma_channel(0, state); }
WRITE_LINE_MEMBER( a2bus_pcxporter_device::pc_dack1_w ) { pc_select_dma_channel(1, state); }
WRITE_LINE_MEMBER( a2bus_pcxporter_device::pc_dack2_w ) { pc_select_dma_channel(2, state); }
WRITE_LINE_MEMBER( a2bus_pcxporter_device::pc_dack3_w ) { pc_select_dma_channel(3, state); }

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER(a2bus_pcxporter_device::pc_speaker_set_spkrdata)
{
	m_pc_spkrdata = state ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
}


/*************************************************************
 *
 * pit8253 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER( a2bus_pcxporter_device::pc_pit8253_out1_changed )
{
	/* Trigger DMA channel #0 */
	if ( m_out1 == 0 && state == 1 && m_u73_q2 == 0 )
	{
		m_u73_q2 = 1;
		m_dma8237->dreq0_w( m_u73_q2 );
	}
	m_out1 = state;
}


WRITE_LINE_MEMBER( a2bus_pcxporter_device::pc_pit8253_out2_changed )
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
}


/**********************************************************
 *
 * PPI8255 interface
 *
 *
 * PORT A (input)
 *
 * Directly attached to shift register which stores data
 * received from the keyboard.
 *
 * PORT B (output)
 * 0 - PB0 - TIM2GATESPK - Enable/disable counting on timer 2 of the 8253
 * 1 - PB1 - SPKRDATA    - Speaker data
 * 2 - PB2 -             - Enable receiving data from the keyboard when keyboard is not locked.
 * 3 - PB3 -             - Dipsswitch set selector
 * 4 - PB4 - ENBRAMPCK   - Enable ram parity check
 * 5 - PB5 - ENABLEI/OCK - Enable expansion I/O check
 * 6 - PB6 -             - Connected to keyboard clock signal
 *                         0 = ignore keyboard signals
 *                         1 = accept keyboard signals
 * 7 - PB7 -             - Clear/disable shift register and IRQ1 line
 *                         0 = normal operation
 *                         1 = clear and disable shift register and clear IRQ1 flip flop
 *
 * PORT C
 * 0 - PC0 -         - Dipswitch 0/4 SW1
 * 1 - PC1 -         - Dipswitch 1/5 SW1
 * 2 - PC2 -         - Dipswitch 2/6 SW1
 * 3 - PC3 -         - Dipswitch 3/7 SW1
 * 4 - PC4 - SPK     - Speaker/cassette data
 * 5 - PC5 - I/OCHCK - Expansion I/O check result
 * 6 - PC6 - T/C2OUT - Output of 8253 timer 2
 * 7 - PC7 - PCK     - Parity check result
 *
 * IBM5150 SW1:
 * 0   - OFF - One or more floppy drives
 *       ON  - Diskless operation
 * 1   - OFF - 8087 present
 *       ON  - No 8087 present
 * 2+3 - Used to determine on board memory configuration
 *       OFF OFF - 64KB
 *       ON  OFF - 48KB
 *       OFF ON  - 32KB
 *       ON  ON  - 16KB
 * 4+5 - Used to select display
 *       OFF OFF - Monochrome
 *       ON  OFF - CGA, 80 column
 *       OFF ON  - CGA, 40 column
 *       ON  ON  - EGA/VGA display
 * 6+7 - Used to select number of disk drives
 *       OFF OFF - four disk drives
 *       ON  OFF - three disk drives
 *       OFF ON  - two disk drives
 *       ON  ON  - one disk drive
 *
 **********************************************************/
WRITE_LINE_MEMBER( a2bus_pcxporter_device::keyboard_clock_w )
{
	if (!m_ppi_keyboard_clear && !state && !m_ppi_shift_enable)
	{
		m_ppi_shift_enable = m_ppi_shift_register & 0x01;

		m_ppi_shift_register >>= 1;
		m_ppi_shift_register |= m_ppi_data_signal << 7;

		m_pic8259->ir1_w(m_ppi_shift_enable);
		m_pc_kbdc->data_write_from_mb(!BIT(m_ppi_portb, 2) && !m_ppi_shift_enable);
	}
}


WRITE_LINE_MEMBER( a2bus_pcxporter_device::keyboard_data_w )
{
	m_ppi_data_signal = state;
}

READ8_MEMBER (a2bus_pcxporter_device::pc_ppi_porta_r)
{
	int data = 0xFF;
	/* KB port A */
	if (m_ppi_keyboard_clear)
	{
		/*   0  0 - no floppy drives
		 *   1  Not used
		 * 2-3  The number of memory banks on the system board
		 * 4-5  Display mode
		 *      11 = monochrome
		 *      10 - color 80x25
		 *      01 - color 40x25
		 * 6-7  The number of floppy disk drives
		 */
		data = 0x30;
	}
	else
	{
		data = m_ppi_shift_register;
	}
//	PIO_LOG(1,"PIO_A_r",("$%02x\n", data));
	return data;
}


READ8_MEMBER ( a2bus_pcxporter_device::pc_ppi_portc_r )
{
	int data=0xff;

	data&=~0x80; // no parity error
	data&=~0x40; // no error on expansion board
	/* KB port C: equipment flags */
	if (m_ppi_portc_switch_high)
	{
		/* read hi nibble of S2 */
		data = (data & 0xf0) | ((0x3) & 0x0f);
//		PIO_LOG(1,"PIO_C_r (hi)",("$%02x\n", data));
	}
	else
	{
		/* read lo nibble of S2 */
		data = (data & 0xf0) | (0x0 & 0x0f);
//		PIO_LOG(1,"PIO_C_r (lo)",("$%02x\n", data));
	}

	if ( m_ppi_portb & 0x01 )
	{
		data = ( data & ~0x10 ) | ( m_pit_out2 ? 0x10 : 0x00 );
	}
	data = ( data & ~0x20 ) | ( m_pit_out2 ? 0x20 : 0x00 );

	return data;
}


WRITE8_MEMBER( a2bus_pcxporter_device::pc_ppi_portb_w )
{
	/* PPI controller port B*/
	m_ppi_portb = data;
	m_ppi_portc_switch_high = data & 0x08;
	m_ppi_keyboard_clear = data & 0x80;
	m_ppi_keyb_clock = data & 0x40;
	m_pit8253->write_gate2(BIT(data, 0));
	pc_speaker_set_spkrdata( data & 0x02 );

	/* If PB7 is set clear the shift register and reset the IRQ line */
	if ( m_ppi_keyboard_clear )
	{
		m_ppi_shift_register = 0;
		m_ppi_shift_enable = 0;
		m_pic8259->ir1_w(m_ppi_shift_enable);
	}

	m_pc_kbdc->data_write_from_mb(!BIT(m_ppi_portb, 2) && !m_ppi_shift_enable);
	m_ppi_clock_signal = ( m_ppi_keyb_clock ) ? 1 : 0;
	m_pc_kbdc->clock_write_from_mb(m_ppi_clock_signal);
}


/**********************************************************
 *
 * NMI handling
 *
 **********************************************************/

WRITE8_MEMBER( a2bus_pcxporter_device::nmi_enable_w )
{
	m_nmi_enabled = BIT(data,7);
	m_isabus->set_nmi_state(m_nmi_enabled);
}



