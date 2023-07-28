// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    3COM 3C523 EtherLink/MC 
	MCA ID @6042

    i82586-based LAN card.

    POS options:
    - POS 0
    -- bit 0: Enable/Disable
    --- XXXXXXX0b Disable	- When disabled, 82586 is held in Reset.
    --- XXXXXXX1b Enable
    -- bits 2-1: I/O ports
    --- XXXXX00Xb 0300h-0307h
    --- XXXXX01Xb 1300h-1307h
    --- XXXXX10Xb 2300h-2307h
    --- XXXXX11Xb 3300h-3307h
    -- bits 4-3: MMIO base - 16K of packet buffer SRAM and 8K of ROM, if present.
    --- XXX00XXXb C0000h-C5FFFh
    --- XXX01XXXb C8000h-CDFFFh
    --- XXX10XXXb D0000h-D5FFFh
    --- XXX11XXXb D8000h-DDFFFh
    -- bit 5: Transceiver Type
    --- XX0XXXXXb On-Board (BNC or RJ45)
    --- XX1XXXXXb External (AUI)
	-- bit 6-7: IRQ selection (read-only)
	--- 00XXXXXXb IRQ 12
	--- 01XXXXXXb IRQ 7
	--- 10XXXXXXb IRQ 3
	--- 11XXXXXXb IRQ 9
    
    - POS 1
    -- bits 3-0: IRQ
    --- XXXX0100b IRQ 3
    --- XXXX0010b IRQ 7
    --- XXXX1000b IRQ 9
    --- XXXX0001b IRQ 12

	https://ardent-tool.com/NIC/3c523_Technical_Reference.pdf

***************************************************************************/

#include "emu.h"
#include "3c523.h"


//#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16_3C523, mca16_3c523_device, "mca_3c523", "3COM EtherLink/MC (@6042)")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_3c523_device::device_add_mconfig(machine_config &config)
{
	I82586(config, m_net, 20_MHz_XTAL/2);
	m_net->set_addrmap(0, &mca16_3c523_device::map_main);
	m_net->out_irq_cb().set(FUNC(mca16_3c523_device::irq_w));

    RAM(config, m_ram);
	m_ram->set_default_size("16KiB");
}

void mca16_3c523_device::map_main(address_map &map)
{
	// i82586 upper 8 address lines are ignored
	map.global_mask(0x00ffff);

	// suppress logging when sizing RAM
	map(0xc000, 0xffff).rw(FUNC(mca16_3c523_device::packet_buffer_r), FUNC(mca16_3c523_device::packet_buffer_w));
}

//-------------------------------------------------
//  mca16_3c523_device - constructor
//-------------------------------------------------

mca16_3c523_device::mca16_3c523_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_3c523_device(mconfig, MCA16_3C523, tag, owner, clock)
{
}

mca16_3c523_device::mca16_3c523_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, 0x6042),
    m_net(*this, "net"),
	m_ram(*this, "ram")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_3c523_device::device_start()
{
	set_mca_device();
	m_is_mapped = 0;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_3c523_device::device_reset()
{
	reset_option_select();	// manual: RESET clears POS registers and host register

	if(m_is_mapped) unmap();
    m_is_mapped = 0;

	m_cur_io_base = 0;
	m_cur_irq = 0;
	m_cur_mem_base = 0;

	m_reset = 0;
	m_channel_attention = 0;
	m_loopback_enabled = 0;
	m_interrupt_flag = 0;
	m_interrupt_fired = 0;
	m_interrupts_enabled = 0;
	m_ram_bank = 0;

}

void mca16_3c523_device::unmap()
{
	m_mca->unmap_device(m_cur_io_base, m_cur_io_base+0xf);
	m_mca->unmap_readwrite(m_cur_mem_base, m_cur_mem_base+0x3fff);
	
	m_is_mapped = false;
}

void mca16_3c523_device::remap()
{
	if(BIT(m_option_select[MCABus::POS::OPTION_SELECT_DATA_1], 0) == 0)
	{
		// do not map
		LOG("%s: disabled via POS, not mapping\n", FUNCNAME);
		return;
	}

	uint16_t pos_io_base = get_pos_io_base();
	uint8_t pos_irq = get_pos_irq();
	offs_t pos_mem_base = get_pos_mem_base();

	LOG("%s: %02X %02X Installing to %04Xh IRQ%d memory %06X\n", FUNCNAME, 
		m_option_select[MCABus::POS::OPTION_SELECT_DATA_1], m_option_select[MCABus::POS::OPTION_SELECT_DATA_2],
	 	pos_io_base, pos_irq, pos_mem_base);

	LOG("%s: Installing I/O\n", FUNCNAME);
	m_mca->install_device(pos_io_base, pos_io_base+0xf,
		read8sm_delegate(*this, FUNC(mca16_3c523_device::io8_r)), write8sm_delegate(*this, FUNC(mca16_3c523_device::io8_w)));
	LOG("%s: Installing MMIO space\n", FUNCNAME);
	m_mca->install_memory(pos_mem_base, pos_mem_base+0x3fff,
			read8sm_delegate(*this, FUNC(mca16_3c523_device::shared_ram_r)), write8sm_delegate(*this, FUNC(mca16_3c523_device::shared_ram_w)));

	m_cur_io_base = pos_io_base;
	m_cur_irq = pos_irq;
	m_cur_mem_base = pos_mem_base;

	LOG("%s: Updating POS DATA 1 for new IRQ\n", FUNCNAME);
	m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] &= 0x3f;
	switch(m_cur_irq)
	{
		case 12: 	m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] |= 0b00000000; break;
		case 7:		m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] |= 0b01000000; break;
		case 3:		m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] |= 0b10000000; break;
		case 9:		m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] |= 0b11000000; break;
	}	

	m_is_mapped = true;
}

uint8_t mca16_3c523_device::get_pos_irq()
{
	LOG("%s: OPTION_SELECT_DATA_2 %02X\n", FUNCNAME, m_option_select[MCABus::POS::OPTION_SELECT_DATA_2]);
	switch(m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] & 0x0F)
	{
		case 1: return 12;
		case 2: return 7;
		case 4: return 3;
		case 8: return 9;
		default: return 0; // invalid
	}
}

offs_t mca16_3c523_device::get_pos_mem_base()
{
	return 0xC0000 + (0x8000 * ((m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] >> 3) & 3));
}

uint16_t mca16_3c523_device::get_pos_io_base()
{
	return 0x300 + (0x1000 * ((m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] >> 1) & 3));
}

bool mca16_3c523_device::map_has_changed()
{
	uint16_t pos_io_base = get_pos_io_base();
	uint8_t pos_irq = get_pos_irq();
	offs_t pos_mem_base = get_pos_mem_base();

	return (pos_io_base != m_cur_io_base) || (pos_irq != m_cur_irq) || (pos_mem_base != m_cur_mem_base);
}

void mca16_3c523_device::update_pos_data_1(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] &= 0xc0;
	m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] |= (data & 0x3f); // bits 6 and 7 are read-only
	if(map_has_changed())
	{
		if(m_is_mapped) unmap();
		remap();
	}

	update_reset();
}

void mca16_3c523_device::update_pos_data_2(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_2] = data;
	if(map_has_changed())
	{
		if(m_is_mapped) unmap();
		remap();
	}
}

void mca16_3c523_device::update_pos_data_3(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_3] = data;
	if(map_has_changed())
	{
		if(m_is_mapped) unmap();
		remap();
	}
}

void mca16_3c523_device::update_pos_data_4(uint8_t data)
{
    m_option_select[MCABus::POS::OPTION_SELECT_DATA_4] = data;
	if(map_has_changed())
	{
		if(m_is_mapped) unmap();
		remap();
	}
}

/* 3C523 I/O registers */

/* 
	Control Register
	b7: /RST	Reset, active low. When asserted, the 82586 is in Reset. Set to 0 on power-up.
	b6: CA		Channel Attention. The 82586 responds when this goes from high to low.
	b5: LBK		Loopback Enable, sets the encoder to loopback mode for testing.
	b4: always 0
	b3: INT		Current interrupt state. Not latched, so always reflects the IRQ line.
	b2: INTE	Assert to enable interrupts. Should always be 1.
	b1: BS1	 	\ RAM Bank Select, always set both bits to 1 for a 16K card
	b0: BS0		/
*/

uint8_t mca16_3c523_device::control_register_r()
{
	uint8_t data = 0;

	data |= (m_reset 				? 0x80 : 0x00);
	data |= (m_channel_attention 	? 0x40 : 0x00);
	data |= (m_loopback_enabled 	? 0x20 : 0x00);
	data |= (m_interrupt_flag 		? 0x08 : 0x00);
	data |= (m_interrupts_enabled 	? 0x04 : 0x00);
	data |= m_ram_bank;				// 0-3 are all valid values

	return data;
}

void mca16_3c523_device::control_register_w(uint8_t data)
{
	LOG("%s: writing %02X\n", FUNCNAME, data);

	m_reset 				= BIT(data, 7);
	m_channel_attention 	= BIT(data, 6);
	m_loopback_enabled		= BIT(data, 5);
	// INT is r/o
	m_interrupts_enabled 	= BIT(data, 2);
	m_ram_bank 				= data & 0x03;

	m_net->ca(m_channel_attention);
	update_reset();
}

void mca16_3c523_device::update_reset()
{
	// The controller is held in reset if POS[0].0 is 0 or the control register's reset bit is 0.
	bool is_in_reset = (BIT(m_option_select[MCABus::POS::OPTION_SELECT_DATA_1], 0) == 0) || (m_reset == 0);
	LOG("%s: 82586 reset line %d. POS reset = %d, ctrl reg reset = %d\n", FUNCNAME, is_in_reset, BIT(m_option_select[MCABus::POS::OPTION_SELECT_DATA_1], 0), m_reset);
	m_net->reset_w(is_in_reset);
}

uint8_t mca16_3c523_device::io8_r(offs_t offset)
{
	// The adapter uses 8 I/O ports.
	// Ports 0-5 are read-only and contain the MAC address.
	// Port 6 is the actual control register.
	// Port 7 is a revision register.

	LOG("%s: O:%02X\n", FUNCNAME, offset);

	switch(offset)
	{
		case 0: return 0x00;	// MAC 0
		case 1: return 0xc0;	// MAC 1
		case 2: return 0x75;	// MAC 2
		case 3: return 0x72;	// MAC 3
		case 4: return 0xb8;	// MAC 4
		case 5: return 0x9e;	// MAC 5
		case 6: return control_register_r();
		case 7: return 0x0f;	// revision
		default: return 0xff;
	}
}

void mca16_3c523_device::io8_w(offs_t offset, uint8_t data)
{
	LOG("%s: O:%02X\n", FUNCNAME, offset);

	// All other registers are read-only.
	if(offset == 6) control_register_w(data);
}

void mca16_3c523_device::irq_w(int state)
{
	bool asserting_now = (m_interrupt_flag == 0) && (state == 1);
	m_interrupt_flag = state; // This flag is not affected by the INTE bit.

	if(m_interrupt_fired && asserting_now)
	{
		LOG("%s: *** Still waiting for IRQ ack...\n", FUNCNAME);
	}

	LOG("%s: state %d, ints enabled %d, int flag %d\n", FUNCNAME, state, m_interrupts_enabled, m_interrupts_enabled);
	if(state && !m_interrupt_fired)
	{
		if(m_interrupts_enabled)
		{
			LOG("%s: firing IRQ %d\n", FUNCNAME, m_cur_irq);
			// Going from cleared to asserted with interrupts enabled, trigger the IRQ.
			switch(m_cur_irq)
			{
				case 3: 	m_mca->ireq_w<3>(state); break;
				case 7: 	m_mca->ireq_w<7>(state); break;
				case 9:	 	m_mca->ireq_w<9>(state); break;
				case 12: 	m_mca->ireq_w<12>(state); break;
				default: break;
			}
			m_interrupt_fired = true;	// An interrupt is actually in progress.
		}
		// Going from cleared to asserted with interrupts disabled, do nothing.
	}
	else if(!state)
	{
		// Going from asserted to cleared. Clear in all cases.
		switch(m_cur_irq)
		{
			case 3: 	m_mca->ireq_w<3>(state); break;
			case 7: 	m_mca->ireq_w<7>(state); break;
			case 9:	 	m_mca->ireq_w<9>(state); break;
			case 12: 	m_mca->ireq_w<12>(state); break;
			default: break;
		}
		m_interrupt_fired = false;		// An interrupt is not actually in progress.
	}
}

uint8_t mca16_3c523_device::shared_ram_r(offs_t offset)
{
	uint8_t data = m_ram->read(offset);
	return data;
}

void mca16_3c523_device::shared_ram_w(offs_t offset, uint8_t data)
{
	//LOG("%s O:%02X D:%02X\n", FUNCNAME, offset, data);
	m_ram->write(offset, data);
}

uint16_t mca16_3c523_device::packet_buffer_r(offs_t offset)
{
	// All 82586 memory cycles are 16-bit on the 3C523.
	uint16_t data = (m_ram->read(offset*2)) | (m_ram->read((offset*2) + 1) << 8);

	//LOG("packet_buffer_r O:%04X (P:%06X) D:%04X\n", offset, (offset*2)+0xC0000, data);

	return data;
}

void mca16_3c523_device::packet_buffer_w(offs_t offset, uint16_t data)
{
	//LOG("packet_buffer_w O:%04X (P:%06X) D:%04X\n", offset, (offset*2)+0xC0000, data);

	// All 82586 memory cycles are 16-bit on the 3C523.
	m_ram->write((offset*2), data & 0xFF);
	m_ram->write((offset*2)+1, (data & 0xFF00) >> 8);
}
