// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    M-Audio Capture and Playback Adapter/A
    MCA ID @6E6C

    Oscillator test microcode is at 0x24700 into ACPADIAG.EXE

***************************************************************************/

#include "emu.h"
#include "macpa.h"
#include "speaker.h"

#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16_MACPA, mca16_macpa_device, "mca16_macpa", "M-Audio Capture and Playback Adapter/A (@6E6C)")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_macpa_device::device_add_mconfig(machine_config &config)
{
	TMS32025(config, m_dsp, XTAL(40'000'000));
	m_dsp->set_addrmap(AS_PROGRAM, &mca16_macpa_device::DSP_map_program);
	m_dsp->set_addrmap(AS_DATA, &mca16_macpa_device::DSP_map_data);
    m_dsp->set_addrmap(AS_IO, &mca16_macpa_device::DSP_map_io);
	m_dsp->hold_in_cb().set(FUNC(mca16_macpa_device::tms_reset_r));
	// m_dsp->hold_ack_out_cb().set(FUNC(mca16_macpa_device::dsp_HOLDA_signal_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DAC_16BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, "lspeaker", 1.0); // uPD6355
	DAC_16BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, "rspeaker", 1.0); // uPD6355
}

//-------------------------------------------------
//  isa8_com_device - constructor
//-------------------------------------------------

mca16_macpa_device::mca16_macpa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_macpa_device(mconfig, MCA16_MACPA, tag, owner, clock)
{
}

mca16_macpa_device::mca16_macpa_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, 0x6e6c),
    m_dsp(*this, "dsp"),
    m_ldac(*this, "ldac"),
    m_rdac(*this, "rdac")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_macpa_device::device_start()
{
	set_mca_device();
    m_shared_ram = std::make_unique<uint16_t[]>(0x2000);
    m_sample_ram = std::make_unique<uint16_t[]>(0x800);
    m_board_is_mapped = 0;
    m_tms_reset = 0;
    m_sample_playback_int_pending = 0;
    
    m_sample_ptrs.adc = 0;
    m_sample_ptrs.dac_l = 0;
    m_sample_ptrs.dac_r = 0;
    m_sample_ptrs.scr = 0;

    m_dacl_enabled = 0;
    m_dacr_enabled = 0;

    m_dacl_int_pending = 0;
    m_dacr_int_pending = 0;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_macpa_device::device_reset()
{
    m_host_to_dsp_int_pending = 0; // Reset condition, page 6-12
    m_dsp_to_host_int_pending = 0; // Reset condition
    m_sample_playback_int_pending = 0; // Reset condition

    m_sample_ptrs.adc = 0;
    m_sample_ptrs.dac_l = 0;
    m_sample_ptrs.dac_r = 0;
    m_sample_ptrs.scr = 0;

    dacl_enable(false);
    dacr_enable(false);

    m_dacl_int_pending = 0;
    m_dacr_int_pending = 0;

    if(m_board_is_mapped) unmap();
}

uint8_t mca16_macpa_device::shared_ram_read8_hi(offs_t address)
{
    LOG("%s: O:%04X\n", FUNCNAME, address);

    // address is in 16-bit words, same as the shared ptr.
    return (m_shared_ram[address % 0x2000] & 0xff00) >> 8;
}

uint8_t mca16_macpa_device::shared_ram_read8_lo(offs_t address)
{
    LOG("%s: O:%04X\n", FUNCNAME, address);

    // address is in 16-bit words, same as the shared ptr.
    return m_shared_ram[address % 0x2000] & 0xff;
}

void mca16_macpa_device::shared_ram_write8_hi(offs_t address, uint8_t data)
{
    LOG("%s: O:%04X D:%02X\n", FUNCNAME, address, data);

    // address is in 16-bit words, same as the shared ptr.
    m_shared_ram[address % 0x2000] &= 0x00ff;
    m_shared_ram[address % 0x2000] |= (((uint16_t)data) << 8);
}

void mca16_macpa_device::shared_ram_write8_lo(offs_t address, uint8_t data)
{
    LOG("%s: O:%04X D:%02X\n", FUNCNAME, address, data);

    // address is in 16-bit words, same as the shared ptr.
    m_shared_ram[address % 0x2000] &= 0xff00;
    m_shared_ram[address % 0x2000] |= data;
}

uint8_t mca16_macpa_device::status_register_r()
{
    // bit 7 - reserved
    // bit 6-5-4: IRQ line configuration, not present on MCA version of the card (handled through POS)
    // bit 3 - reserved
    // bit 2 - reserved
    // bit 1 - HST REQ: 1 if DSP->Host interrupt is pending. reset to 0 on power-up
    // bit 0 - TMS INT: 0 if Host->DSP interrupt is pending. reset to 1 on power-up

    uint8_t data = 0;

    if(m_dsp_to_host_int_pending) data |= 2;
    if(!m_host_to_dsp_int_pending) data |= 1;

    LOG("%s: Host is reading status, %02X\n", FUNCNAME, data);

    return data;    
}

void mca16_macpa_device::command_register_w(uint8_t data)
{
    LOG("%s: D:%04X\n", FUNCNAME, data);

    /*
        Write-only register.

        bit 7 - reserved
        bit 6 - reserved
        bit 5 - reserved
        bit 4 - SPKR EN - Speaker Enable. Latched bit.
            - 1: Speaker enabled.
            - 0: Speaker disabled. Power-up condition.
        bit 3 - TMS INT - TMS Interrupt. Strobed bit.
            - 0: Host->DSP interrupt when 0 is written to this bit.
        bit 2 - HINTENA - Host Interrupt Enable. Latched bit.
            - 1: TMS requests to the host cause a host interrupt.
            - 0: TMS requests to the host do not cause a host interrupt. Power-up condition.
        bit 1 - HREQACK - Host Request Acknowledge. Strobed bit.
            - 0: Clears the DSP->Host interrupt when 0 is written to this bit.
        bit 0 - TMS RES - Resets the TMS320C35. Latched bit.
            - 1: TMS320C35 is in operating state.
            - 0: TMS320C25 is in reset state. Power-up condition.
     */

    m_tms_reset = BIT(data, 0);

    if(m_tms_reset == 0)
    {
        // We're in reset condition. Ignore everything. Reset everything.
        m_host_to_dsp_int_pending = false;
        m_dsp_to_host_int_pending = false;
    }
    else
    {
        if(BIT(data, 4)) LOG("%s: Motherboard speaker enabled\n", FUNCNAME);
        if(BIT(data, 3) == 0) tms_int_w();
        m_hintena = BIT(data, 2);
        if(BIT(data, 1) == 0) hreqack_w();
    }
}

void mca16_macpa_device::hreqack_w()
{
    LOG("%s: Clearing DSP->Host interrupt\n", FUNCNAME);

    m_dsp_to_host_int_pending = false;

    switch(m_mapped_irq)
    {
        case 2: m_mca->ireq_w<2>(m_dsp_to_host_int_pending); break;
        case 3: m_mca->ireq_w<3>(m_dsp_to_host_int_pending); break;
        case 4: m_mca->ireq_w<4>(m_dsp_to_host_int_pending); break;
        case 5: m_mca->ireq_w<5>(m_dsp_to_host_int_pending); break;
        case 6: m_mca->ireq_w<6>(m_dsp_to_host_int_pending); break;
        case 10: m_mca->ireq_w<10>(m_dsp_to_host_int_pending); break;
        case 11: m_mca->ireq_w<11>(m_dsp_to_host_int_pending); break;
        case 12: m_mca->ireq_w<12>(m_dsp_to_host_int_pending); break;
    }
}

void mca16_macpa_device::tms_int_w()
{
    // TMS INT strobed.
    LOG("%s: Asserting Host->DSP interrupt\n", FUNCNAME);

    m_host_to_dsp_int_pending = true;
    m_dsp->set_input_line(TMS32025_INT1, m_host_to_dsp_int_pending);
}

uint16_t mca16_macpa_device::tms_reset_r()
{    
	return !m_tms_reset;
}

uint8_t mca16_macpa_device::io8_r(offs_t offset)
{
	LOG("%s: O:%02X\n", FUNCNAME, offset);
    uint8_t data = 0;
	assert_card_feedback();

    switch(offset)
    {
        // Data - Low Byte
        case 0: data = shared_ram_read8_lo(m_address_latch); break;
        
        // Data - High Byte. Advances address latch by one word.
        case 1: data = shared_ram_read8_hi(m_address_latch); m_address_latch++; break;
        
        // 2, 3, 4, 5 not used
        case 2: data = 0xff; break;
        case 3: data = 0xff; break;
        case 4: data = 0xff; break;
        case 5: data = 0xff; break;

        // Host - Status Register
        case 6: data = status_register_r(); break;
        
        // ACPA-ID, always return 0x6C.
        case 7: data = 0x6c; break;
    }

	return data;
}

void mca16_macpa_device::io8_w(offs_t offset, uint8_t data)
{
	LOG("%s: O:%02X D:%02X\n", FUNCNAME, offset, data);
	assert_card_feedback();

    switch(offset)
    {
        // Data - Low Byte
        case 0: shared_ram_write8_lo(m_address_latch, data); break;
        // Data - High Byte. Advances address latch by one word.
        case 1: shared_ram_write8_hi(m_address_latch, data); m_address_latch++; m_address_latch = m_address_latch % 0x2000; break;
        
        // 2, 3 not used
        case 2: break;
        case 3: break;

        // Address - Low Byte
        case 4: m_address_latch &= 0xff00; m_address_latch |= data; break;
        // Address - High Byte
        case 5: m_address_latch &= 0x00ff; m_address_latch |= ((uint16_t)data << 8); break;

        // Host - Command Register
        case 6: command_register_w(data); break;
        
        // not used
        case 7: break;
    }
}

uint8_t mca16_macpa_device::pos_r(offs_t offset)
{
	LOG("%s\n", FUNCNAME);

	switch(offset)
	{
		case 0:
			// Adapter Identification b0-b7
			return get_card_id() & 0xFF;
		case 1:
			// Adapter Identification b8-b15
			return (get_card_id() & 0xFF00) >> 8;
		case 2:
			// Option Select Data 1
            return m_option_select[MCABus::POS::OPTION_SELECT_DATA_1];
			break;
		case 3:
			// Option Select Data 2
            return m_option_select[MCABus::POS::OPTION_SELECT_DATA_2];
			break;
		case 4:
			// Option Select Data 3
            return m_option_select[MCABus::POS::OPTION_SELECT_DATA_3];
			break;
		case 5:
			// Option Select Data 4
            return m_option_select[MCABus::POS::OPTION_SELECT_DATA_4];
			break;
		case 6:
			// Subaddress Extension Low
			return 0xff;
			break;
		case 7:
			// Subaddress Extension High
			return 0xff;
			break;
		default:
			break;
	}

	return 0xFF;
}

void mca16_macpa_device::pos_w(offs_t offset, uint8_t data)
{
	LOG("%s\n", FUNCNAME);

	switch(offset)
	{
		case 0:
			// Adapter Identification b0-b7
		case 1:
			// Adapter Identification b8-b15
			break;
		case 2:
			// Option Select Data 1
            update_pos(data);
			break;
		case 3:
			// Option Select Data 2 - not used
			break;
		case 4:
			// Option Select Data 3 - not used
			break;
		case 5:
			// Option Select Data 4 - not used
			break;
		case 6:
			// Subaddress Extension Low - not used
			break;
		case 7:
			// Subaddress Extension High - not used
			break;
		default:
			break;
	}
}

void mca16_macpa_device::update_pos(uint8_t data)
{
    // This card only uses one POS register.
    // b7-b6: reserved
    // b5-b3: IR2/IR1/IR0
    // b2-b1: AS1/AS0
    // b0: BDEN

    uint8_t irq;
    uint16_t io_base;
    bool board_enable = m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] & 1;

    switch((m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] & 0b00000110) >> 1)
    {
        case 0: io_base = 0xfdc0; break;
        case 1: io_base = 0xfdc8; break;
        case 2: io_base = 0xfdd0; break;
        case 3: io_base = 0xfdd8; break;
    };

    switch((m_option_select[MCABus::POS::OPTION_SELECT_DATA_1] & 0b00111000) >> 3)
    {
        case 0: irq = 3; break;
        case 1: irq = 4; break;
        case 2: irq = 5; break;
        case 3: irq = 6; break;
        case 4: irq = 2; break;
        case 5: irq = 10; break;
        case 6: irq = 11; break;
        case 7: irq = 12; break;
    }

    if(m_board_is_mapped) unmap();
    m_mapped_io = io_base;
    m_mapped_irq = irq;
    if(board_enable) remap();
}

void mca16_macpa_device::remap()
{
    m_mca->install_device(m_mapped_io, m_mapped_io+7,
        read8sm_delegate(*this, FUNC(mca16_macpa_device::io8_r)),
        write8sm_delegate(*this, FUNC(mca16_macpa_device::io8_w)));
}

void mca16_macpa_device::unmap()
{
    m_mca->unmap_device(m_mapped_io, m_mapped_io+7);    
}

//
// DSP
//

// Memory decoding is really, really weird.
// There's also internal on-chip memory in here somewhere.

void mca16_macpa_device::DSP_map_io(address_map &map)
{
    // Page 6-20 of ACPA_Techref.pdf

    // All I/O address space points to one register.
    map(0x0000, 0xffff).rw(FUNC(mca16_macpa_device::dsp_status_register_r), FUNC(mca16_macpa_device::dsp_command_register_w));
}

void mca16_macpa_device::DSP_map_program(address_map &map)
{
    // Page 6-20 of ACPA_Techref.pdf
    // Program space contains:
    // - 8 blocks of Shared Memory (A through H)

    map(0x0000, 0x1fff).rw(FUNC(mca16_macpa_device::shared_ram_r), FUNC(mca16_macpa_device::shared_ram_w));
    map(0x2000, 0x3fff).rw(FUNC(mca16_macpa_device::shared_ram_r), FUNC(mca16_macpa_device::shared_ram_w));
    map(0x4000, 0x5fff).rw(FUNC(mca16_macpa_device::shared_ram_r), FUNC(mca16_macpa_device::shared_ram_w));
    map(0x6000, 0x7fff).rw(FUNC(mca16_macpa_device::shared_ram_r), FUNC(mca16_macpa_device::shared_ram_w));
    map(0x8000, 0x9fff).rw(FUNC(mca16_macpa_device::shared_ram_r), FUNC(mca16_macpa_device::shared_ram_w));
    map(0xa000, 0xbfff).rw(FUNC(mca16_macpa_device::shared_ram_r), FUNC(mca16_macpa_device::shared_ram_w));
    map(0xc000, 0xdfff).rw(FUNC(mca16_macpa_device::shared_ram_r), FUNC(mca16_macpa_device::shared_ram_w));
    map(0xe000, 0xffff).rw(FUNC(mca16_macpa_device::shared_ram_r), FUNC(mca16_macpa_device::shared_ram_w));
}
void mca16_macpa_device::DSP_map_data(address_map &map)
{
    // Page 6-20 of ACPA_Techref.pdf
    // Data space contains:
    // - 4 blocks of Shared Memory (A through D)
    // - 16 blocks of Sample Memory (A through P)

    map(0x0000, 0x1fff).rw(FUNC(mca16_macpa_device::shared_ram_r), FUNC(mca16_macpa_device::shared_ram_w));
    map(0x2000, 0x3fff).rw(FUNC(mca16_macpa_device::shared_ram_r), FUNC(mca16_macpa_device::shared_ram_w));
    map(0x4000, 0x47ff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0x4800, 0x4fff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0x5000, 0x57ff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0x5800, 0x5fff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0x6000, 0x67ff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0x6800, 0x6fff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0x7000, 0x77ff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0x7800, 0x7fff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0x8000, 0x9fff).rw(FUNC(mca16_macpa_device::shared_ram_r), FUNC(mca16_macpa_device::shared_ram_w));
    map(0xa000, 0xbfff).rw(FUNC(mca16_macpa_device::shared_ram_r), FUNC(mca16_macpa_device::shared_ram_w));
    map(0xc000, 0xc7ff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0xc800, 0xcfff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0xd000, 0xd7ff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0xd800, 0xdfff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0xe000, 0xe7ff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0xe800, 0xefff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0xf000, 0xf7ff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
    map(0xf800, 0xffff).rw(FUNC(mca16_macpa_device::sample_ram_r), FUNC(mca16_macpa_device::sample_ram_w));
}

u16 mca16_macpa_device::shared_ram_r(offs_t offset)
{
	return m_shared_ram[offset];
}

void mca16_macpa_device::shared_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_8_15 && ACCESSING_BITS_0_7)
		m_shared_ram[offset] = data;
}

u16 mca16_macpa_device::sample_ram_r(offs_t offset)
{
	return m_sample_ram[offset];
}

void mca16_macpa_device::sample_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_8_15 && ACCESSING_BITS_0_7)
		m_sample_ram[offset] = data;
}

// These are local registers that only the DSP can see.    
uint16_t mca16_macpa_device::dsp_status_register_r(offs_t offset)
{
    // Any DSP I/O space read will go to the status register.

    /*
        bit 7 - reserved
        bit 6 - reserved
        bit 5 - reserved
        bit 4 - reserved
        bit 3 - HST REQ     - Host->DSP interrupt.
            1: Interrupt pending.
            0: No interrupt pending, power-up condition.
        bit 2 - ADC INT     - ADC->DSP interrupt.
            1: Interrupt pending.
            0: No interrupt pending, power-up condition.
        bit 1 - DACL INT    - DAC Left->DSP interrupt.
            1: Interrupt pending.
            0: No interrupt pending, power-up condition.
        bit 0 - DACR INT    - DAC Right->DSP interrupt.
            1: Interrupt pending.
            0: No interrupt pending, power-up condition.
    */

    uint16_t data = 0;

    if(m_tms_reset == 0)
    {
        // We're in the reset condition. All interrupts disabled.
        data = 0;
    }
    else
    {
        if(m_host_to_dsp_int_pending) data |= (1 << 3);
        if(m_dacl_int_pending) data |= (1 << 1);
        if(m_dacr_int_pending) data |= (1 << 0);
    }

    LOG("%s: D:%04X\n", FUNCNAME, data);

    return data;
}

void mca16_macpa_device::dsp_command_register_w(offs_t offset, uint16_t data)
{
    // Any DSP I/O space write will go to the command register.

    LOG("%s: D:%04X\n", FUNCNAME, data);

    /*
        bit 9 - MIKE GAIN   - Select microphone gain amount.
            1: +33dB
            0: +45dB, power-up condition.
        bit 8 - AD CHAN     - Select the right channel sampled at 88.2KHz or stereo at 44.1KHz.
            1: Right and left channels sampled at 44.1KHz each.
            0: Right channel sampled at 88.2KHz, power-up condition.
        bit 7 - BLK RST     - Forces sample memory to the first location of block 0. Strobed bit.
            0: When written, resets sample memory to the first location of block 0.
        bit 6 - HST REQ     - Raises a DSP->Host interrupt. Strobed bit.
            0: Requests service from the host, if HINTENA is asserted.
        bit 5 - TMS IAK     - Acknowledges a Host->DSP interrupt. Strobed bit.
            0: Clears the Host->DSP interrupt latch.
        bit 4 - SPB IAK     - Acknowledges a sample playback request interrupt.
            0: Clears the sample playback interrupt latch.
        bit 3 - INP SEL     - Select Right input source.
            1: Line selected
            0: Mike selected, power-up condition.
        bit 2 - ADC EN      - Enable the ADC.
            1: ADC enabled
            0: ADC disabled, power-up condition.
        bit 1 - DACL EN     - Enable the left DAC.
            1: DACL enabled
            0: DACL disabled, power-up condition.
        bit 0 - DACR EN     - Enable the right DAC.
            1: DACR enabled
            0: DACL disabled, power-up condition.
    */

    if(m_tms_reset == 0)
    {
        // We're in the reset condition.

        // Disable ADC.
        // Disable both DACs.
        // Input is Mike.
        // Right channel is sampled at 88.2KHz.
        // Mike Gain is +45dB.
    }
    else
    {
        if(!BIT(data, 7))
        {
            m_sample_ptrs.adc = 0;
            m_sample_ptrs.dac_l = 0;
            m_sample_ptrs.dac_r = 0;
            m_sample_ptrs.scr = 0;            
        }

        if(!BIT(data, 6))
        {
            if(m_hintena)
            {
                LOG("%s: Raising DSP->Host interrupt, IRQ %d\n", FUNCNAME, m_mapped_irq);
                m_dsp_to_host_int_pending = true;
                
                switch(m_mapped_irq)
                {
                    case 2: m_mca->ireq_w<2>(m_dsp_to_host_int_pending); break;
                    case 3: m_mca->ireq_w<3>(m_dsp_to_host_int_pending); break;
                    case 4: m_mca->ireq_w<4>(m_dsp_to_host_int_pending); break;
                    case 5: m_mca->ireq_w<5>(m_dsp_to_host_int_pending); break;
                    case 6: m_mca->ireq_w<6>(m_dsp_to_host_int_pending); break;
                    case 10: m_mca->ireq_w<10>(m_dsp_to_host_int_pending); break;
                    case 11: m_mca->ireq_w<11>(m_dsp_to_host_int_pending); break;
                    case 12: m_mca->ireq_w<12>(m_dsp_to_host_int_pending); break;
                }
            }
            else
            {
                LOG("%s: DSP->Host IRQ is masked, not raising.\n", FUNCNAME);
            }
        }

        if(!BIT(data, 5))
        {
            LOG("%s: Clearing Host->DSP interrupt\n", FUNCNAME);
            m_host_to_dsp_int_pending = false;
            m_dsp->set_input_line(TMS32025_INT1, m_host_to_dsp_int_pending);
        }

        if(!BIT(data, 4))
        {
            m_dacl_int_pending = false;
            m_dacr_int_pending = false;
            m_dsp->set_input_line(TMS32025_INT0, false);
        }

        LOG("%s: DAC L/R now %d/%d. ADC now %d\n", FUNCNAME, BIT(data, 1), BIT(data, 0), BIT(data, 2));

        // Page 6-24 of the ACPA manual to see how to hook up the DAC.
        // Sample Memory is divided up into 256-word blocks. Each peripheral has two blocks used as a ring buffer.
        // - The DSP fills block 0 for the appropriate peripheral.
        // - When the Enable bit is asserted, the DAC will immediately receive an INT0 from the peripheral.
        // - The status register will show which peripheral sent the interrupt.
        // - Each time the peripheral reaches the end of its 256-word block, INT0 is asserted.
        // - The BIO pin shows which block is being used by the peripheral.
        // - BIO 0 = The peripheral is using Block 0, the DSP must write Block 1.
        // - BIO 1 = The peripheral is using Block 1, the DSP must write Block 0.
        // - The INT0 is cleared by strobing 0 into the SPB IAK bit of the DSP command register.

        dacr_enable(BIT(data, 0));
        dacl_enable(BIT(data, 1));

        // Assert INT0 if required.
        m_dsp->set_input_line(TMS32025_INT0, m_dacr_int_pending || m_dacl_int_pending);
    }
}

void mca16_macpa_device::dacr_enable(bool enable)
{
    if(m_dacr_enabled && enable) return;    // already enabled
    if(!m_dacr_enabled && !enable) return;  // already disabled
    
    if(!m_dacr_enabled && enable)           // off -> on transition
    {        
        // What is the timing source for the DAC?
        m_dacr_int_pending = true;
        m_dacr_enabled = true;
    }
    else if(m_dacr_enabled && !enable)      // on -> off transition
    {
        m_dacr_int_pending = false;
        m_dacr_enabled = false;        
    }
}

void mca16_macpa_device::dacl_enable(bool enable)
{
    if(m_dacl_enabled && enable) return; // already enabled
}