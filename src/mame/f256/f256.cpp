// license:BSD-3-Clause
// copyright-holders:Daniel Tremblay
#include "emu.h"
#include "f256.h"

#include "cpu/m6502/w65c02s.h"
#include "tiny_vicky.h"

/**
 *
 * F256K - WDC65C02 Processor running at 6.25MHz
 *    512KB RAM managed with slots of 8KB in MMU located at address $0000.  Slots 0 to $3F
 *    512KB Flash                                                           Slots $40 to $7F
 *    All I/O are mapped to slot 6 ($C000-$DFFF) - there are 4 I/O maps, switched using address $0001.
 *    Sound Chips: OPL3, PSG, SN74689, CODEC
 *    Keyboard: mechanical switch in a matrix - controlled by VIA6522 chip.
 *    Joysticks: 2 Atari-type ports and 2 S/NES ports
 *    Mouse: over PS/2 - which can also be used for PS/2 Keyboard
 *    IEC: Commodore Floppy Drive Controller
 *
 */

f256_state::f256_state(const machine_config &mconfig, device_type type, const char *tag) :
    driver_device(mconfig, type, tag)
    , m_maincpu(*this, MAINCPU_TAG)
    , m_ram(*this, RAM_TAG)
    , m_iopage(*this, "iopage%u", 0U)
    , m_rom(*this, ROM_TAG)
    , m_font(*this, FONT_TAG)
    , m_screen(*this, SCREEN_TAG)
    , m_rtc(*this, "rtc")
    , m_keyboard(*this, "ROW%u", 0)  // this, with the 8 array, requires 8 ROW of INPUTs
    , m_via6522_0(*this, "via6522_0")
	, m_via6522_1(*this, "via6522_1")

    , m_sn0(*this, "sn76489_0")
    , m_sn1(*this, "sn76489_1")
    , m_opl3(*this, "ymf262")
    , m_sid0(*this, "sid_0")
    , m_sid1(*this, "sid_1")
    , m_joy1(*this, "JOY1")
    , m_joy2(*this, "JOY2")

    , m_video(*this, "tiny_vicky")
    //, m_iec(*this, "iec_bus")
    //, m_iec_data_out(1)
    //, m_iec_srq(1)
    , m_ps2_keyboard(*this, "ps2_kbd")
    , m_mouse(*this, "ps_mouse")
    , m_uart(*this, "uart")
    , m_sdcard(*this, "sdcard")
	, m_spi_clock_state(false)
	, m_spi_clock_sysclk(false)
	, m_spi_clock_cycles(0)
{
}

void f256_state::f256k(machine_config &config)
{
    W65C02S(config, m_maincpu, MASTER_CLOCK/4);
    m_maincpu->set_addrmap(AS_PROGRAM, &f256_state::program_map);

    RAM(config, m_ram).set_default_size("512k").set_default_value(0x0);
    for (auto &iopage : m_iopage) 
    {
        RAM(config, iopage).set_default_size("8k").set_default_value(0x0);
    }

    SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
    m_screen->set_refresh_hz(60); // Refresh rate (e.g., 60Hz)
    m_screen->set_size(800,525);
    //m_screen->set_visarea(160, 799, 45, 524);  // this is how it should reall work, but the screen ends up offset
    m_screen->set_visarea(0, 639, 0, 479);
    m_screen->set_screen_update(m_video, FUNC(tiny_vicky_video_device::screen_update));
    
    BQ4802(config, m_rtc, MASTER_CLOCK / 1000);  // RTC clock in kHz
    //set interrupt handler for the RTC
    m_rtc->int_handler().set(FUNC(f256_state::rtc_interrupt_handler));
    
    TINY_VICKY(config, m_video, MASTER_CLOCK);
    m_video->sof_irq_handler().set(FUNC(f256_state::sof_interrtupt));
    m_video->sol_irq_handler().set(FUNC(f256_state::sol_interrtupt));

    W65C22S(config, m_via6522_0, MASTER_CLOCK / 4);  // Atari Joysticks
	m_via6522_0->readpa_handler().set(FUNC(f256_state::via0_system_porta_r));
	m_via6522_0->readpb_handler().set(FUNC(f256_state::via0_system_portb_r));
	m_via6522_0->writepa_handler().set(FUNC(f256_state::via0_system_porta_w));
	m_via6522_0->writepb_handler().set(FUNC(f256_state::via0_system_portb_w));
    m_via6522_0->ca2_handler().set(FUNC(f256_state::via0_ca2_write));
    m_via6522_0->cb2_handler().set(FUNC(f256_state::via0_cb2_write));
    m_via6522_0->irq_handler().set(FUNC(f256_state::via0_interrupt));

    // initialize the PS2 mouse
    PC_KBDC(config, m_ps2_keyboard, XTAL(32'768));
    HLE_PS2_MOUSE(config, m_mouse, XTAL(32'768));
    m_mouse->set_pc_kbdc(m_ps2_keyboard);

    MOS6522(config, m_via6522_1, MASTER_CLOCK / 4);  // Keyboard XTAL(14'318'181)/14)
    m_via6522_1->readpa_handler().set(FUNC(f256_state::via1_system_porta_r));
	m_via6522_1->readpb_handler().set(FUNC(f256_state::via1_system_portb_r));
	m_via6522_1->writepa_handler().set(FUNC(f256_state::via1_system_porta_w));
	m_via6522_1->writepb_handler().set(FUNC(f256_state::via1_system_portb_w));
    m_via6522_1->ca2_handler().set(FUNC(f256_state::via1_ca2_write));
    m_via6522_1->cb2_handler().set(FUNC(f256_state::via1_cb2_write));
    m_via6522_1->irq_handler().set(FUNC(f256_state::via1_interrupt));

    // Mix PSG
    SN76489(config, m_sn0, MUSIC_CLOCK / 4);
    m_sn0->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
    m_sn0->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
    SN76489(config, m_sn1, MUSIC_CLOCK / 4);
    m_sn1->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
    m_sn1->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

    YMF262(config, m_opl3, MUSIC_CLOCK);
    m_opl3->add_route(0, "lspeaker", 1.0);
	m_opl3->add_route(1, "rspeaker", 1.0);
	m_opl3->add_route(2, "lspeaker", 1.0);
	m_opl3->add_route(3, "rspeaker", 1.0);

    // The SIDs are very noisy
    MOS6581(config, m_sid0, MUSIC_CLOCK/14);
    m_sid0->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
    m_sid0->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
    MOS6581(config, m_sid1, MUSIC_CLOCK/14);
    m_sid1->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
    m_sid1->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

    SPEAKER(config, "lspeaker").front_left();
    SPEAKER(config, "rspeaker").front_right();

    // Add an SD card device
    SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->set_prefer_sdhc();
    m_sdcard->spi_miso_callback().set([this](int state) {
        //m_in_bit = state;
        m_in_latch <<= 1;
        m_in_latch |= state;
    });

    
    NS16550(config, m_uart, MASTER_CLOCK);

    // The IEC interface is not yet implemented
    // cbm_iec_slot_device::add(config, m_iec, "c1581");
	// m_iec->srq_callback().set(FUNC(f256_state::iec_srq_w));
	// m_iec->data_callback().set(FUNC(f256_state::iec_data_w));
    // m_iec->atn_callback().set(FUNC(f256_state::iec_atn_w));
    // m_iec->clk_callback().set(FUNC(f256_state::iec_clk_w));
}
f256_state::~f256_state()
{
}
/*
    Memory map
    $00:0000 - $07:FFFF	RAM
    $08:0000 - $0F:FFFF	Flash
    $10:0000 - $13:FFFF	Expansion Memory - NOT IMPLEMENTED
    $14:0000 - $1F:FFFF	Reserved
*/
void f256_state::program_map(address_map &map)
{
    // the address range 0:F
    // 0: LUT Edit $80 allows writing to 8 to F, LUT Select 0 to 3
    map(0x0000, 0x000F).rw(FUNC(f256_state::lut_r), FUNC(f256_state::lut_w));
    map(0x0010, 0xFFFF).rw(FUNC(f256_state::mem_r), FUNC(f256_state::mem_w));
};

void f256_state::data_map(address_map &map)
{
    map(0x0000, 0x1FFF).ram().share("iopage0");
    map(0x0000, 0x1FFF).ram().share("iopage1");
    map(0x0000, 0x1FFF).ram().share("iopage2");
    map(0x0000, 0x1FFF).ram().share("iopage3");
}

u8   f256_state::lut_r(offs_t offset)
{
    // addresses 2 to 7 are always read from RAM
    if (offset == 0)
    {
        return m_mmu_reg;
    }
    else if (offset == 1)
    {
        return m_ioreg;
    }
    else if (offset < 8)
    {
        return m_ram->read(offset);
    }
    else
    {
        // if we are not in edit mode, return RAM data
        if ((m_mmu_reg & 0x80) != 0)
        {
            uint8_t mmu = (m_mmu_reg >> 4) & 0x3;  // use the top nibble
            return mmu_lut[(mmu & 0x3) * 8 + (offset-8)];
        }
        else
        {
            return m_ram->read(offset);
        }
    }
}
void f256_state::lut_w(offs_t offset, u8 data)
{
    // addresses 2:7 are always RAM
    if (offset == 0)
    {
        m_mmu_reg = data;
    }
    else if (offset == 1)
    {
        m_ioreg = data;
    }
    else if (offset < 8)
    {
        m_ram->write(offset, data);
    }
    else
    {
        // bit 7 of mmu0 determines if the MMU is writable
        if ((m_mmu_reg & 0x80) == 0)
        {
            m_ram->write(offset, data);
        }
        else
        {
            uint8_t mmu = (m_mmu_reg >> 4) & 0x3;  // use the top nibble
            mmu_lut[mmu * 8 + (offset - 8)] = data;
        }
    }
}

// offsets are 0x10 based
u8   f256_state::mem_r(offs_t offset)
{

    // find which slot to read
    uint8_t mmu = m_mmu_reg & 3;
    uint16_t adj_addr = offset + 0x10;
    uint8_t slot = adj_addr >> 13;
    uint16_t low_addr = adj_addr & 0x1FFF;
    uint8_t bank = mmu_lut[mmu * 8 + slot];

    // fslot < 0x40 is RAM, greater is FLASH/ROM
    if (bank < 0x40)
    {
        // Slot 6 is where I/O devices are located, when IO_DISABLE is 0
        if (slot == 6 && (m_ioreg & 0x4) == 0)
        {
            switch (m_ioreg & 0x3)
            {
                case 0:
                    // here we have a number of devices to read
                    if (adj_addr >= 0xD018 && adj_addr < 0xD01C)
                    {
                        // return Vicky's scan line and colum
                        uint16_t line = m_screen->hpos();
                        //uint16_t column = m_video->column();
                        logerror("Scanline Addr: %04X, Line: %04X\n", adj_addr, line);
                        switch (adj_addr - 0xD018)
                        {
                            case 0:
                                return 0; //column & 0xFF;
                            case 1:
                                return 0; // (column >> 8);
                            case 2:
                                return line & 0xFF;
                            case 3:
                                return (line >> 8);
                        }

                    }
                    else if (adj_addr >= 0xD400 && adj_addr < 0xD580)
                    {
                        // SID
                        return 0;
                    }
                    else if (adj_addr >= 0xD580 && adj_addr < 0xD583)
                    {
                        // OPL3
                        return 0;
                    }
                    else if (adj_addr >= 0xD600 && adj_addr < 0xD620)
                    {
                        // PSG - left channel D600, right channel D610 - both D608
                        return 0;
                    }
                    else if (adj_addr >= 0xD620 && adj_addr < 0xD630)
                    {
                        // logerror("CODEC Read Addr: %04X\n", adj_addr);
                        uint16_t base = adj_addr - 0xD620;
                        return m_codec[base];
                    }
                    else if (adj_addr >= 0xD630 && adj_addr < 0xD640)
                    {
                        // UART
                        uint8_t v_uart = m_uart->ins8250_r(adj_addr - 0xD630);
                        logerror("UART Read %X %02X\n", adj_addr, v_uart);
                        return v_uart;
                    }
                    else if (adj_addr >= 0xD640 && adj_addr < 0xD64F)
                    {
                        // PS2
                        logerror("PS/2 Read %04X\n", adj_addr);
                        switch(adj_addr - 0xD640)
                        {
                            case 0:
                            case 1:
                                return m_ps2[adj_addr - 0xD640];
                            case 2:
                            {
                                // Read from the keyboard fifo
                                if (kbPacketCntr > kbQLen)
                                {
                                    return 0;
                                }
                                uint8_t kbval = kbFifo[kbPacketCntr++];
                                if (kbPacketCntr == kbQLen)
                                {
                                    kbPacketCntr = 0;
                                    kbQLen = 0;
                                    memset(kbFifo, 0, 6);
                                }
                                return kbval;
                            }
                            case 3:
                            {
                                // Read from the mouse fifo
                                if (msPacketCntr> msQLen)
                                {
                                    return 0;
                                }
                                uint8_t msval = msFifo[msPacketCntr++];
                                if (msPacketCntr == msQLen)
                                {
                                    msPacketCntr = 0;
                                    msQLen = 0;
                                    memset(msFifo, 0, 3);
                                }
                                return msval;
                            }
                            case 4:
                                K_AK = false;
                                M_AK = false;
                                return ((K_AK ? 0x80:0) + (M_AK ? 0x20 : 0) + (msQLen == 0 ? 2 : 0) + (kbQLen == 0? 1 : 0));
                        }

                        return m_ps2[adj_addr - 0xD640];
                    }
                    else if (adj_addr >= 0xD650 && adj_addr < 0xD660)
                    {
                        // Timers
                        switch (adj_addr)
                        {
                            case 0xD650:
                                return m_timer0_eq;
                            case 0xD651:
                                return m_timer0_val & 0xFF;
                            case 0xD652:
                                return (m_timer0_val >> 8) & 0xFF;
                            case 0xD653:
                                return (m_timer0_val >> 16) & 0xFF;
                            case 0xD658:
                                return m_timer1_eq;
                            case 0xD659:
                                return m_timer1_val & 0xFF;
                            case 0xD65A:
                                return (m_timer1_val >> 8) & 0xFF;
                            case 0xD65B:
                                return (m_timer1_val >> 16) & 0xFF;
                        }
                        return m_iopage[0]->read(adj_addr - 0xC000);
                    }
                    else if (adj_addr >= 0xD660 && adj_addr < 0xD670)
                    {
                        // Interrupt Registers
                        logerror("Interrupt Registers Read: %04X\n", adj_addr);
                        switch (adj_addr)
                        {
                            case 0xD660:
                                return m_interrupt_reg[0]; // int_pending_0
                            case 0xD661:
                                return m_interrupt_reg[1]; // int_pending_1
                            case 0xD662:
                                return m_interrupt_reg[2]; // int_pending_2
                            case 0xD663:
                                return 0;
                            case 0xD664:
                                return m_interrupt_polarity[0]; // int_polarity_0
                            case 0xD665:
                                return m_interrupt_polarity[1]; // int_polarity_1
                            case 0xD666:
                                return m_interrupt_polarity[2]; // int_polarity_2
                            case 0xD667:
                                return 0;
                            case 0xD668:
                                return m_interrupt_edge[0]; // int_edge_0
                            case 0xD669:
                                return m_interrupt_edge[1]; // int_edge_1
                            case 0xD66A:
                                return m_interrupt_edge[2]; // int_edge_2
                            case 0xD66B:
                                return 0;
                            case 0xD66C:
                                return m_interrupt_masks[0];
                            case 0xD66D:
                                return m_interrupt_masks[1];
                            case 0xD66E:
                                return m_interrupt_masks[2];
                            case 0xD66F:
                                return 0;

                        }
                    }
                    else if (adj_addr >= 0xD680 && adj_addr < 0xD682)
                    {
                        //IEC
                        logerror("Reading from IEC Reg: %X", adj_addr - 0xD680);
                        switch (adj_addr - 0xD680)
                        {
                            case 0:
                                logerror(", data: %02X\n", m_iec_in);
                                // gather the IEC bus values
                                // m_iec_in =
                                //     (m_iec->srq_r() << 7) +
                                //     (m_iec->atn_r() << 4) +
                                //     (m_iec->clk_r() << 1) +
                                //     (m_iec->data_r());
                                logerror(", data: %02X\n", m_iec_in);
                                return m_iec_in;
                            case 1:
                                logerror(", data: %02X\n", m_iec_out);
                                return m_iec_out;
                        }
                    }
                    else if (adj_addr >= 0xD690 && adj_addr < 0xD6A0)
                    {
                        // RTC
                        logerror("RTC Read %04X\n", adj_addr);
                        return m_rtc->read(adj_addr - 0xDC90);
                    }
                    else if (adj_addr >= 0xD6A0 && adj_addr < 0xD6C0)
                    {
                        // System Control Registers
                        // D6A0 - buzzer and LED controls - including RESET bit
                        // D6A1 - sound mixer
                        // D6A2 - Set to 0xDE to enable software reset
                        // D6A3 - Set to 0xAD to enable software reset
                        // D6A4 - D6A6 : Random Number generator
                        // D6A7 - Macine ID - For the F256, the machine ID will be 0x02. For the F256k, the machine ID will be 0x12.
                        logerror("System Register Read %04X\n", adj_addr);
                        switch (adj_addr){
                            case 0xD6A0:
                                return m_sdcard->get_card_present() ? 0x10:0;
                            case 0xD6A1:
                                return m_iopage[0]->read(0xD6A1 - 0xC000);
                            case 0xD6A4:
                                if (m_rng_enabled)
                                {
                                    return get_random() & 0xFF;
                                }
                                else
                                {
                                    return m_seed & 0xFF;
                                }
                            case 0xD6A5:
                                if (m_rng_enabled)
                                {
                                    return get_random() & 0xFF;
                                }
                                else
                                {
                                    return (m_seed >> 8) & 0xFF;
                                }
                            case 0xD6A6:
                                return m_rng_enabled ? 0x80: 0;
                            case 0xD6A7:
                                return 0x12;  // F256K ID
                            case 0XD6A8:
                                return 'A';
                            case 0XD6A9:
                                return '0';
                            case 0XD6AA:
                                return 1;
                            case 0XD6AB:
                                return 1;
                            case 0XD6AC:
                                return 0;
                            case 0XD6AD:
                                return 0x14;
                            case 0XD6AE:
                                return 0;
                            case 0XD6AF:
                                return 0;
                        }

                    }
                    // mouse registers
                    // else if (adj_addr >= 0xD6E0 && adj_addr < 0xD6E9)
                    // {
                    //     switch (adj_addr)
                    //     {
                    //         case 0xD6E0:
                    //             return (m_mouse_mode << 1) + m_mouse_enabled ? 1 : 0;
                    //             break;
                    //         case 0xD6E2:
                    //             return m_mouse_x & 0xFF;
                    //         case 0xD6E3:
                    //             return (m_mouse_x >> 8) & 0xFF;
                    //         case 0xD6E4:
                    //             return m_mouse_y & 0xFF;
                    //         case 0xD6E5:
                    //             return (m_mouse_y >> 8) & 0xFF;

                    //         default:
                    //             break;
                    //     }
                    // }
                    else if (adj_addr >= 0xD880 && adj_addr < 0xD8C0)
                    {
                        // NES
                        return 0xFF;
                    }
                    else if (adj_addr >= 0xDB00 && adj_addr < 0xDB10)
                    {
                        // VIA1 - Keyboard for F256K
                        return m_via6522_1->read(adj_addr - 0xDB00);
                    }
                    else if (adj_addr >= 0xDC00 && adj_addr < 0xDC10)
                    {
                        // VIA0 - Atari Joystick
                        return m_via6522_0->read(adj_addr - 0xDC00);
                    }
                    else if (adj_addr >= 0xDD00 && adj_addr < 0xDD20)
                    {
                        // SD Card
                        //m_sdcard->(adj_addr - 0xDD00);
                        // logerror("Reading SD Card: %04X\n", adj_addr);
                        switch (adj_addr)
                        {
                            case 0xDD00:
                            {
                                // bit 7 is the busy state
                                u8 spi_reg = (m_spi_clock_cycles > 0 ? 0x80 : 0x00) + (spi_sd_enabled ? 1 : 0); // TODO add clock bits
                                //logerror("Read SD 0: %02X\n", spi_reg);
                                return spi_reg;
                            }
                            case 0xDD01:
                                //logerror("Read SD 1: %02X\n", m_in_latch);
                                return m_in_latch;
                            default:
                                return 0xFF;
                        }
                    }
                    else if (adj_addr >= 0xDE00 && adj_addr < 0xDE20)
                    {
                        // Math Coprocessor
                        switch (adj_addr - 0xDE10)
                        {
                            case 0:
                                return m_multiplication_result & 0xFF;
                            case 1:
                                return (m_multiplication_result >> 8) & 0xFF;
                            case 2:
                                return (m_multiplication_result >> 16) & 0xFF;
                            case 3:
                                return (m_multiplication_result >> 24) & 0xFF;
                            case 4:
                                return m_division_result & 0xFF;
                            case 5:
                                return (m_division_result >> 8) & 0xFF;
                            case 6:
                                return m_division_remainder & 0xFF;
                            case 7:
                                return (m_division_remainder >> 8) & 0xFF;
                            case 8:
                                return m_addition_result & 0xFF;
                            case 9:
                                return (m_addition_result >> 8) & 0xFF;
                            case 0xA:
                                return (m_addition_result >> 16) & 0xFF;
                            case 0xB:
                                return (m_addition_result >> 24) & 0xFF;
                        }
                        return m_iopage[0]->read(adj_addr - 0xC000);
                    }
                    else if (adj_addr >= 0xDF00 && adj_addr < 0xE000)
                    {
                        // DMA
                        if (adj_addr == 0xDF01)
                        {
                            return m_dma_status;
                        }
                        else
                        {
                            return m_iopage[0]->read(adj_addr - 0xC000);
                        }
                    }
                    // Stick everything else in Vicky
                    // (adj_addr >= 0xC000 && adj_addr < 0xD400) ||  // gamma, mouse graphics, vicky registers, bitmaps, tiles
                    // (adj_addr >= 0xD800 && adj_addr < 0xD880) ||  // text colors
                    // (adj_addr >= 0xD900 && adj_addr < 0xDB00)     // sprite registers
                    return m_iopage[0]->read(adj_addr - 0xC000);
                case 1:
                    return m_iopage[1]->read(adj_addr - 0xC000);
                case 2:
                    return m_iopage[2]->read(adj_addr - 0xC000);
                case 3:
                    return m_iopage[3]->read(adj_addr - 0xC000);
            }
        }
        offs_t address = (bank << 13) + low_addr;
        return m_ram->read(address);
    }
    else if (bank < 0x80)
    {
        offs_t address = (bank << 13) + low_addr;
        return m_rom->as_u8(address);
    }
    else if (bank < 0xA0)
    {
        // this is now trying to read expansion RAM/Flash - NOT IMPLEMENTED
    }
    // Invalid area of memory
    return 0;
}
void f256_state::mem_w(offs_t offset, u8 data)
{
    // find which slot to write
    uint8_t mmu = m_mmu_reg & 3;
    uint16_t adj_addr = offset + 0x10;
    uint8_t slot = adj_addr >> 13;
    uint16_t low_addr = adj_addr & 0x1FFF;
    uint8_t old, combo;

    uint8_t bank = mmu_lut[mmu * 8 + slot];
    if (bank < 0x40)
    {
        // Slot 6 is where I/O devices are located, when IO_DISABLE is 0
        if (slot == 6)
        {
            // if IO_DISABLED is 1, then slot 6 is regular RAM
            if ((m_ioreg & 0x4) == 0)
            {
                switch (m_ioreg & 0x3)
                {
                    case 0:
                        // here we have a number of devices to write
                        if (adj_addr == 0xD001)
                        {
                            logerror("Change Resolution %04X %02X\n", adj_addr, data);
                            if ((data & 0x1) != 0 )
                            {
                                m_screen->set_refresh_hz(70);
                                m_screen->set_visarea(0, 639, 0, 399);
                                m_screen->set_size(800,450);
                            }
                            else
                            {
                                m_screen->set_refresh_hz(60);
                                m_screen->set_visarea(0, 639, 0, 479);
                                m_screen->set_size(800,525);
                            }
                            m_iopage[0]->write(0xD001 - 0xC000, data);
                        }
                        else if (adj_addr >= 0xD400 && adj_addr < 0xD419)
                        {
                            // SID 0
                            m_sid0->write(adj_addr - 0xD400, data);

                        }
                        else if (adj_addr >= 0xD500 && adj_addr < 0xD519)
                        {
                            // SID 1
                            m_sid1->write(adj_addr - 0xD500, data);
                        }
                        else if (adj_addr >= 0xD580 && adj_addr < 0xD583)
                        {
                            // OPL3
                            switch(adj_addr)
                            {
                                case 0xD580:
                                    m_opl3_reg = data;
                                    m_opl3->address_w(data);
                                    break;
                                case 0xD581:
                                    //m_opl3->write(m_opl3_reg, data);
                                    if (m_opl3_reg > 0xFF) { m_opl3->data_hi_w(data);} else { m_opl3->data_w(data);}
                                    break;
                                case 0xD582:
                                    m_opl3_reg = 0x100 + data;
                                    m_opl3->address_hi_w(data);
                                    break;
                            }
                        }
                        else if (adj_addr == 0xD600)
                        {
                            // PSG
                            logerror("PSG Left Write: %02X\n", data);
                            m_sn0->write(data);
                        }
                        else if (adj_addr == 0xD608)
                        {
                            // PSG
                            logerror("PSG Both Write: %02X\n", data);
                            m_sn0->write(data);
                            m_sn1->write(data);
                        }
                        else if (adj_addr == 0xD610)
                        {
                            // PSG
                            logerror("PSG Right Write: %02X\n", data);
                            m_sn1->write(data);
                        }
                        else if (adj_addr >= 0xD620 && adj_addr < 0xD630)
                        {
                            // Codec
                            logerror("CODEC Write %04X %02X\n", adj_addr, data);
                            uint16_t base = adj_addr-0xD620;
                            m_codec[base] = data;
                            // the program is telling the codec to start
                            if ((base == 2) && ((data & 1) == 1))
                            {
                                // start a timer that will reset the value to zero
                                this->machine().scheduler().timer_set(attotime::from_msec(100), timer_expired_delegate(FUNC(f256_state::codec_done), this), 1);   // timer_alloc(timer_expired_delegate(FUNC(f256_state::timer), this));
                            }
                        }
                        else if (adj_addr >= 0xD630 && adj_addr < 0xD640)
                        {
                            // UART
                            logerror("UART Writing %X %02X\n", adj_addr, data);
                            m_uart->ins8250_w(adj_addr - 0xD630, data);
                        }
                        else if (adj_addr >= 0xD640 && adj_addr < 0xD64F)
                        {
                            // PS/2 Keyboard
                            logerror("PS/2 Write %04X %02X\n", adj_addr, data);
                            uint16_t delta = adj_addr-0xD640;
                            m_ps2[delta] = data;
                            // Only addresses 0 and 1 are writable
                            if (delta == 0)
                            {
                                switch (data)
                                {
                                    case 0:
                                        if (isK_WR)
                                        {
                                            // write out the byte in data[1] to keyboard
                                            isK_WR = false;
                                            K_AK = true;
                                            m_ps2_keyboard->data_write_from_kb(data);
                                        }
                                        if (isM_WR)
                                        {
                                            // write out the byte in data[1] to mouse
                                            isM_WR = false;
                                            M_AK = true;
                                            m_mouse->data_write(data);
                                        }
                                        break;
                                    case 2:
                                        isK_WR = true;
                                        break;
                                    case 8:
                                        isM_WR = true;
                                        break;
                                    case 0x10: // clear keyboard fifo
                                        memset(kbFifo, 0 , 6);
                                        break;
                                    case 0x20: // clear mouse fifo
                                        memset(msFifo, 0, 3);
                                        break;
                                }
                            }
                            else if (delta == 1)
                            {

                            }
                        }
                        else if (adj_addr >= 0xD650 && adj_addr < 0xD660)
                        {
                            // Timers
                            logerror("Writing to Timer Register: %X, %02X\n", adj_addr, data);
                            m_iopage[0]->write(adj_addr - 0xC000, data);
                            switch(adj_addr)
                            {
                                case 0xD650:
                                    // Timer0 is based on Master Clock and causes some slow down.
                                    // I'm computing the next period and avoiding increments by one.
                                    if ((data & 0x1) == 1)
                                    {

                                        uint32_t timer0_cmp = m_iopage[0]->read(0xD655 - 0xC000) +
                                            (m_iopage[0]->read(0xD656 - 0xC000) << 8) +
                                            (m_iopage[0]->read(0xD657 - 0xC000) << 16);
                                        logerror("Start Timer0: %06X\n", timer0_cmp);
                                        attotime period = attotime::from_double((double)(timer0_cmp - m_timer0_load)/(double)25'175'000);
                                        m_timer0->adjust(period, 0, period);
                                    }
                                    else
                                    {
                                        logerror("Stop Timer0\n");
                                        m_timer0->adjust(attotime::never);
                                    }

                                    if ((data & 0x2) != 0)
                                    {
                                        m_timer0_val = 0;
                                    }
                                    if ((data & 0x4) != 0)
                                    {
                                        m_timer0_val = m_timer0_load;
                                    }
                                    break;
                                case 0xD651:
                                case 0xD652:
                                case 0xD653:
                                    // writing to these registers sets the load value
                                    m_timer0_load = m_iopage[0]->read(0xD651 - 0xC000) + (m_iopage[0]->read(0xD652 - 0xC000) << 8) +
                                            (m_iopage[0]->read(0xD653 - 0xC000) << 16);
                                    break;
                                case 0xD658:
                                    // Timer1 is based on the Start of Frame - so it's very slow
                                    if ((data & 0x1) == 1)
                                    {
                                        logerror("Start Timer1 %X, %X\n", data, m_timer1_val);
                                        // Get the frame frequency from video
                                        int frame_freq = (m_iopage[0]->read(0xD001 - 0xC000) & 1) == 1? 70: 60;
                                        m_timer1->adjust(attotime::from_hz(XTAL(frame_freq)), 0, attotime::from_hz(XTAL(frame_freq)));
                                    }
                                    else
                                    {
                                        logerror("Stop Timer1\n");
                                        m_timer1->adjust(attotime::never);
                                    }

                                    if ((data & 0x2) != 0)
                                    {
                                        logerror("Timer1 value = 0\n");
                                        m_timer1_val = 0;
                                    }
                                    if ((data & 0x4) != 0)
                                    {
                                        m_timer1_val = m_timer1_load;
                                        logerror("Timer1 value = %06X\n", m_timer1_val);
                                    }
                                    break;
                                case 0xD659:
                                case 0xD65A:
                                case 0xD65B:
                                    // writing to these registers sets the load value
                                    m_timer1_load = m_iopage[0]->read(0xD659 - 0xC000) + (m_iopage[0]->read(0xD65A - 0xC000) << 8) +
                                            (m_iopage[0]->read(0xD65B - 0xC000) << 16);
                                    break;
                            }
                        }
                        else if (adj_addr >= 0xD660 && adj_addr < 0xD670)
                        {
                            // Interrupt Registers
                            logerror("Interrupt Register Write: %04X with %02X\n", adj_addr, data);
                            switch (adj_addr)
                            {
                                case 0xD660:
                                    // int_pending_0
                                    old = m_interrupt_reg[0];
                                    combo = old & data;
                                    if (combo > 0)
                                    {
                                        m_interrupt_reg[0] = old & ~combo;
                                    }

                                    break;
                                case 0xD661:
                                    // int_pending_1
                                    old = m_interrupt_reg[1];
                                    combo = old & data;
                                    if (combo > 0)
                                    {
                                        m_interrupt_reg[1] = old & ~combo;
                                    }
                                    break;
                                case 0xD662:
                                    // int_pending_2
                                    old = m_interrupt_reg[2];
                                    combo = old & data;
                                    if (combo > 0)
                                    {
                                        m_interrupt_reg[2] = old & ~combo;
                                    }
                                    break;
                                case 0xD663:
                                    break;
                                case 0xD664:
                                    // int_polarity_0
                                    m_interrupt_polarity[0] = data;
                                    break;
                                case 0xD665:
                                    // int_polarity_1
                                    m_interrupt_polarity[1] = data;
                                    break;
                                case 0xD666:
                                    // int_polarity_2
                                    m_interrupt_polarity[2] = data;
                                    break;
                                case 0xD667:
                                    break;
                                case 0xD668:
                                    // int_edge_0
                                    m_interrupt_edge[0] = data;
                                    break;
                                case 0xD669:
                                    // int_edge_1
                                    m_interrupt_edge[1] = data;
                                    break;
                                case 0xD66A:
                                    // int_edge_2
                                    m_interrupt_edge[2] = data;
                                    break;
                                case 0xD66B:
                                    break;
                                case 0xD66C:
                                    m_interrupt_masks[0] = data;
                                    break;
                                case 0xD66D:
                                    m_interrupt_masks[1] = data;
                                    break;
                                case 0xD66E:
                                    m_interrupt_masks[2] = data;
                                    break;
                                case 0xD66F:
                                    break;

                            }
                            if (m_interrupt_reg[0] == 0 && m_interrupt_reg[1] == 0 && m_interrupt_reg[2] == 0)
                            {
                                logerror("Clearing Interrupt Line\n");
                                m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
                            }
                        }
                        //IEC  - 0xD680 is not writable
                        else if (adj_addr >= 0xD680 && adj_addr < 0xD682)
                        {
                            logerror("Writing to IEC reg %X %02X\n", adj_addr, data);
                            if (adj_addr == 0xD681)
                            {
                                m_iec_out = data;
                                // Bit 6 is RESET
                                // m_iec->host_reset_w((data & 0x40) >> 6);

                                // // distribute these to the IEC bus
                                // m_iec->host_clk_w((data & 2) >> 1);
                                // m_iec->host_data_w(data & 1);
                                // m_iec->host_atn_w((data & 0x10) >> 4);
                                // m_iec->host_srq_w((data & 0x80) >> 7);

                                // fake the bus
                                m_iec_in &= 0xFE;
                                m_iec_in |= (data & 1);
                            }
                        }
                        else if (adj_addr >= 0xD690 && adj_addr < 0xD6A0)
                        {
                            // RTC
                            logerror("RTC Write %04X %02X\n", adj_addr, data);
                            m_rtc->write(adj_addr - 0xDC90, data);
                        }
                        else if (adj_addr >= 0xD6A0 && adj_addr < 0xD6A7)
                        {
                            // RNG
                            logerror("RNG Write %04X %02X\n", adj_addr, data);
                            switch (adj_addr)
                            {
                                case 0xD6A1:
                                    // mix the PSG or SID based on the value
                                    m_iopage[0]->write(0xD6A1 - 0xC000, data);
                                    if ((data & 4) == 0)
                                    {
                                        // PSG mix - both outputs to both speakers
                                        m_sn0->reset_routes();
                                        m_sn1->reset_routes();
                                        m_sn0->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
                                        m_sn0->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
                                        m_sn1->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
                                        m_sn1->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
                                    }
                                    else
                                    {
                                        // PSG0 to left, PSG1 to right
                                        m_sn0->reset_routes();
                                        m_sn1->reset_routes();
                                        m_sn0->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
                                        m_sn1->add_route(ALL_OUTPUTS, "rspeaker", 1.0);
                                    }
                                    if ((data & 8) == 0)
                                    {
                                        // SID mix -
                                        m_sid0->reset_routes();
                                        m_sid1->reset_routes();
                                        m_sid0->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
                                        m_sid0->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
                                        m_sid1->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
                                        m_sid1->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
                                    }
                                    else
                                    {
                                        m_sid0->reset_routes();
                                        m_sid1->reset_routes();
                                        m_sid0->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
                                        m_sid1->add_route(ALL_OUTPUTS, "rspeaker", 1.0);
                                    }
                                    break;
                                case 0xD6A4:
                                    m_seed &= 0xFF00; // zero the low byte
                                    m_seed |= data;   // set the low byte
                                    break;
                                case 0xD6A5:
                                    m_seed &= 0xFF;   // zero the high byte
                                    m_seed |= (data << 8);  // set the high byte
                                    break;
                                case 0xD6A6:
                                    m_rng_enabled = (data & 1);
                                    if ((data & 2) != 0)
                                    {
                                        srand(m_seed);
                                    }
                                    break;
                            }
                        }
                        // mouse registers
                        // else if (adj_addr >= 0xD6E0 && adj_addr < 0xD6E9)
                        // {
                        //     switch (adj_addr)
                        //     {
                        //         case 0xD6E0:
                        //             m_mouse_enabled = (data & 1) != 0;
                        //             m_mouse_mode = (data >> 1) & 1;
                        //             break;
                        //         case 0xD6E2:
                        //             // keep the high byte
                        //             m_mouse_x &= 0xFF00;
                        //             m_mouse_x |= data;
                        //             break;
                        //         case 0xD6E3:
                        //             // keep the low byte
                        //             m_mouse_x &= 0xFF;
                        //             m_mouse_x |= data << 8;
                        //             break;
                        //         case 0xD6E4:
                        //             // keep the high byte
                        //             m_mouse_y &= 0xFF00;
                        //             m_mouse_y |= data;
                        //             break;
                        //         case 0xD6E5:
                        //             // keep the low byte
                        //             m_mouse_y &= 0xFF;
                        //             m_mouse_y |= data << 8;
                        //             break;

                        //         default:
                        //             break;
                        //     }
                        // }
                        else if (adj_addr >= 0xD880 && adj_addr < 0xD8C0)
                        {
                            // NES - only address 0xD8800 is writable
                        }
                        else if (adj_addr >= 0xDB00 && adj_addr < 0xDC00)
                        {
                            // VIA1 - Keyboard for F256K
                            m_via6522_1->write(adj_addr - 0xDB00, data);
                        }
                        else if (adj_addr >= 0xDC00 && adj_addr < 0xDD00)
                        {
                            // VIA0 - Atari Joystick
                            m_via6522_0->write(adj_addr - 0xDC00, data);
                        }
                        else if (adj_addr >= 0xDD00 && adj_addr < 0xDD20)
                        {
                            // SD Card
                            switch(adj_addr - 0xDD00)
                            {
                                case 0:
                                    // When bit is set, clock is 400kHz - 0= 12.5 MHz
                                    //logerror("Write SD 0: %02X\n", data);
                                    m_spi_clock_sysclk = bool(BIT(data, 1));
                                    spi_sd_enabled = BIT(data, 0);
                                    break;
                                case 1:
                                    if (m_spi_clock_cycles == 0)
                                    {
                                        //logerror("Write SD 1: %02X\n", data);
                                        m_out_latch = data;
                                        if (spi_sd_enabled)
                                        {
                                            m_spi_clock_cycles = 8;
                                            m_sdcard->spi_ss_w(spi_sd_enabled);
                                            if (m_spi_clock_sysclk)
                                                m_spi_clock->adjust(attotime::from_hz(XTAL(400'000)), 0, attotime::from_hz(XTAL(400'000)));
                                            else
                                                m_spi_clock->adjust(attotime::from_hz(XTAL(12'500'000)), 0, attotime::from_hz(XTAL(12'500'000)));
                                        }
                                    }
                                    else
                                    {
                                        logerror("SD card is busy - refusing to write\n");
                                    }
                                    break;
                            }

                            // - F256K2c
                            // $DD00 - $DD1F - SDCARD0
                            // $DD20 - $DD3F - SDCARD1 *** This one has moved ***
                            // $DD40 - $DD5F - SPLASH LCD (SPI Port)
                            // $DD60 - $DD7F - Wiznet Copper SPI Interface
                            // $DD80 - $DD9F - Wiznet WIFI UART interface (115K or 2M)
                            // $DDA0 - $DDBF - MIDI UART (Fixed @ 31,250Baud)
                            // $DDC0 - $DDDF - Master SPI Interface to Supervisor (RP2040)*
                        }
                        else if (adj_addr >= 0xDE00 && adj_addr < 0xDE20)
                        {
                            // Math Coprocessor - 4 blocks
                            u8 block = (adj_addr - 0xDE00) >> 2;
                            if (adj_addr < 0xDE10)
                            {
                                m_iopage[0]->write(adj_addr - 0xC000, data);
                            }
                            switch (block)
                            {
                                case 0:
                                    unsignedMultiplier(0xDE00 - 0xC000);
                                    break;
                                case 1:
                                    unsignedDivider(0xDE04 - 0xC000);
                                    break;
                                case 2:
                                case 3:
                                    unsignedAdder(0xDE08 - 0xC000);
                                    break;
                            }
                        }
                        else if (adj_addr >= 0xDF00 && adj_addr < 0xE000)
                        {
                            // DMA
                            logerror("DMA Write %04X %02X\n", adj_addr, data);
                            m_iopage[0]->write(adj_addr - 0xC000, data);
                            if ((adj_addr - 0xDF00) == 0)
                            {
                                    // control register - when start and enabled are set start DMA operation
                                    if ((data & 0x81) == 0x81)
                                    {
                                        bool fill = data & 0x4;
                                        bool tfr_2d = data & 0x2;
                                        // set to busy
                                        m_dma_status = 0x80;
                                        if (fill)
                                        {
                                            if (tfr_2d)
                                                perform2DFillDMA();
                                            else
                                                performLinearFillDMA();
                                        }
                                        else
                                        {
                                            if (tfr_2d)
                                                perform2DDMA();
                                            else
                                                performLinearDMA();
                                        }
                                        // set to not busy
                                        m_dma_status = 0x0;
                                        // check if an interrupt needs to be raised
                                        if ((data & 0x8) != 0)
                                        {
                                            dma_interrupt_handler(1);
                                        }
                                    }
                            }
                        }
                        // stick everything else in Vicky
                            // (adj_addr >= 0xC000 && adj_addr < 0xD400) ||  // gamma, mouse graphics, vicky registers, bitmaps, tiles
                            // (adj_addr >= 0xD800 && adj_addr < 0xD880) ||  // text colors
                            // (adj_addr >= 0xD900 && adj_addr < 0xDB00)     // sprite registers
                        else
                        {
                            m_iopage[0]->write(adj_addr - 0xC000, data);
                        }

                        break;
                    case 1:
                        m_iopage[1]->write(adj_addr - 0xC000, data);
                        break;
                    case 2:
                        m_iopage[2]->write(adj_addr - 0xC000, data);
                        break;
                    case 3:
                        m_iopage[3]->write(adj_addr - 0xC000, data);
                        break;
                }
            }
            else
            {
                offs_t address = (bank << 13) + low_addr;
                m_ram->write(address, data);
            }
        }
        else
        {
            offs_t address = (bank << 13) + low_addr;
            m_ram->write(address, data);
        }
    }
}
void f256_state::codec_done(s32 param)
{
    m_codec[2] = 0;
}
void f256_state::reset_mmu()
{
    logerror("reset_mmu\n");
    for (int i =0; i < 32; i++)
    {
        if (i % 8 == 7)
        {
            mmu_lut[i]= 0x7f;
        }
        else
        {
            mmu_lut[i] = i % 8;
        }
    }
}


//-------------------------------------------------
//  IEC Methods
//-------------------------------------------------
inline void f256_state::update_iec()
{
	// int fsdir = m_mmu->fsdir_r();

	// fast serial data in
	//int data_in = m_iec->data_r();

	// m_cia1->sp_w(fsdir || data_in);

	// fast serial data out
	//int data_out = !m_iec_data_out;

	//if (fsdir) data_out &= m_sp1;

	//m_iec->host_data_w(data_out);

	// fast serial clock in
	// m_cia1->cnt_w(fsdir || m_iec_srq);

	// fast serial clock out
	//int srq_out = m_iec_srq;

	// if (fsdir) srq_out &= m_cnt1;

	//m_iec->host_srq_w(srq_out);
}

void f256_state::iec_srq_w(int state)
{
    logerror("Event iec_srq_w: %X\n", state);

    m_iec_srq = state;

    if (state && ((m_interrupt_masks[2] & 0x8) == 0))
    {
        m_interrupt_reg[2] |= 0x8;
        m_maincpu->set_input_line((m_iec_out & 0x20) !=0 ? M6502_NMI_LINE:M6502_IRQ_LINE, state);
    }

	update_iec();
}
void f256_state::iec_data_w(int state)
{
    logerror("Event iec_data_w: %X\n", state);

    if (state && ((m_interrupt_masks[2] & 0x1) == 0))
    {
        m_interrupt_reg[2] |= 0x1;
        m_maincpu->set_input_line((m_iec_out & 0x20) !=0 ? M6502_NMI_LINE:M6502_IRQ_LINE, state);
    }
    update_iec();
}
void f256_state::iec_atn_w(int state)
{
    logerror("Event iec_atn_w: %X\n", state);
    if (state && ((m_interrupt_masks[2] & 0x4) == 0))
    {
        m_interrupt_reg[2] |= 0x4;
        m_maincpu->set_input_line((m_iec_out & 0x20) !=0 ? M6502_NMI_LINE:M6502_IRQ_LINE, state);
    }
}
void f256_state::iec_clk_w(int state)
{
    logerror("Event iec_clk_w: %X\n", state);
    if (state && ((m_interrupt_masks[2] & 0x2) == 0))
    {
        m_interrupt_reg[2] |= 0x2;
        m_maincpu->set_input_line((m_iec_out & 0x20) !=0 ? M6502_NMI_LINE:M6502_IRQ_LINE, state);
    }
}
//-------------------------------------------------
//  Math Coprocessor Methods
//-------------------------------------------------
void f256_state::unsignedMultiplier(int baseAddr)
{
    uint16_t acc1 = (m_iopage[0]->read(baseAddr + 1) << 8) + m_iopage[0]->read(baseAddr);
    uint16_t acc2 = (m_iopage[0]->read(baseAddr + 3) << 8) + m_iopage[0]->read(baseAddr + 2);
    m_multiplication_result = acc1 * acc2;
}

void f256_state::unsignedDivider(int baseAddr)
{
    uint16_t acc1 = (m_iopage[0]->read(baseAddr + 1) << 8) + m_iopage[0]->read(baseAddr);
    uint16_t acc2 = (m_iopage[0]->read(baseAddr + 3) << 8) + m_iopage[0]->read(baseAddr + 2);
    if (acc1 != 0)
    {
        m_division_result= acc2 / acc1;
        m_division_remainder = acc2 % acc1;
    }
}

void f256_state::unsignedAdder(int baseAddr)
{
    int acc1 = (m_iopage[0]->read(baseAddr + 3) << 24) + (m_iopage[0]->read(baseAddr + 2) << 16) +
        (m_iopage[0]->read(baseAddr + 1) << 8) + m_iopage[0]->read(baseAddr);
    int acc2 = (m_iopage[0]->read(baseAddr + 7) << 24) + (m_iopage[0]->read(baseAddr + 6) << 16) +
        (m_iopage[0]->read(baseAddr + 5) << 8) + m_iopage[0]->read(baseAddr + 4);
    m_addition_result = acc1 + acc2;
}

//-------------------------------------------------
//  DMA Methods
//-------------------------------------------------
uint8_t f256_state::get_random()
{
    uint8_t m_random = rand();
    return m_random & 0xFF;
}
//-------------------------------------------------
//  DMA Methods
//-------------------------------------------------
void f256_state::perform2DFillDMA()
{

    uint8_t fill_byte = m_iopage[0]->read(0xDF01 - 0xC000);
    uint32_t dest_addr = ((m_iopage[0]->read(0xDF0A) & 0x7) << 16) + (m_iopage[0]->read(0xDF09) << 8) + m_iopage[0]->read(0xDF08);
    uint16_t width_2D = (m_iopage[0]->read(0xDF0D - 0xC000) << 8) + m_iopage[0]->read(0xDF0C - 0xC000);
    uint16_t height_2D = (m_iopage[0]->read(0xDF0F - 0xC000) << 8) + m_iopage[0]->read(0xDF0E - 0xC000);
    uint16_t dest_stride = (m_iopage[0]->read(0xDF13 - 0xC000) << 8) + m_iopage[0]->read(0xDF12 - 0xC000);
    //logerror("2D Fill DMA: DEST: %X, W: %X, H: %X\n", dest_addr, width_2D, height_2D);
    for (int y = 0; y < height_2D; y++)
    {
        for (int x = 0; x < width_2D; x++)
        {
            m_ram->write(dest_addr + x + y * dest_stride, fill_byte);
        }
    }
}
void f256_state::performLinearFillDMA()
{
    uint8_t fill_byte = m_iopage[0]->read(0xDF01 - 0xC000);
    uint32_t dest_addr = ((m_iopage[0]->read(0xDF0A) & 0x7) << 16) + (m_iopage[0]->read(0xDF09) << 8) + m_iopage[0]->read(0xDF08);
    uint32_t count = ((m_iopage[0]->read(0xDF0E) & 0x7) << 16) + (m_iopage[0]->read(0xDF0D) << 8) + m_iopage[0]->read(0xDF0C);
    //logerror("Linear Fill DMA DEST: %X, LEN: %X\n", dest_addr, count);
    memset(m_ram->pointer() + dest_addr, fill_byte, count);
}
void f256_state::perform2DDMA()
{
    uint32_t src_addr = ((m_iopage[0]->read(0xDF06) & 0x7) << 16) + (m_iopage[0]->read(0xDF05) << 8) + m_iopage[0]->read(0xDF04);
    uint32_t dest_addr = ((m_iopage[0]->read(0xDF0A) & 0x7) << 16) + (m_iopage[0]->read(0xDF09) << 8) + m_iopage[0]->read(0xDF08);
    uint16_t width_2D = (m_iopage[0]->read(0xDF0D - 0xC000) << 8) + m_iopage[0]->read(0xDF0C - 0xC000);
    uint16_t height_2D = (m_iopage[0]->read(0xDF0F - 0xC000) << 8) + m_iopage[0]->read(0xDF0E - 0xC000);
    uint16_t src_stride = (m_iopage[0]->read(0xDF11 - 0xC000) << 8) + m_iopage[0]->read(0xDF10 - 0xC000);
    uint16_t dest_stride = (m_iopage[0]->read(0xDF13 - 0xC000) << 8) + m_iopage[0]->read(0xDF12 - 0xC000);
    //logerror("2D Copy DMA, SRC: %X, DEST: %X, W: %X H: %X, SRC_STR: %X, DEST_STR: %X\n", src_addr, dest_addr,
    //    width_2D, height_2D, src_stride, dest_stride);
    for (int y = 0; y < height_2D; y++)
    {
        for (int x = 0; x < width_2D; x++)
        {
            uint8_t src_byte = m_ram->read(src_addr + x + y * src_stride);
            m_ram->write(dest_addr + x + y * dest_stride, src_byte);
        }
    }
}
void f256_state::performLinearDMA()
{
    uint32_t src_addr = ((m_iopage[0]->read(0xDF06) & 0x7) << 16) + (m_iopage[0]->read(0xDF05) << 8) + m_iopage[0]->read(0xDF04);
    uint32_t dest_addr = ((m_iopage[0]->read(0xDF0A) & 0x7) << 16) + (m_iopage[0]->read(0xDF09) << 8) + m_iopage[0]->read(0xDF08);
    uint32_t count = ((m_iopage[0]->read(0xDF0E) & 0x7) << 16) + (m_iopage[0]->read(0xDF0D) << 8) + m_iopage[0]->read(0xDF0C);
    //logerror("Linear Copy DMA SRC: %X, DEST: %X, LEN: %X\n", src_addr, dest_addr, count);
    memcpy(m_ram->pointer() + dest_addr, m_ram->pointer() + src_addr, count);
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------
void f256_state::device_start()
{
	driver_device::device_start();
    reset_mmu();
    // TODO: Copy the font from file to IO Page 1
    //memcpy(m_iopage[1], m_font, 0x800);
    for (int i=0;i<0x800;i++)
    {
        m_iopage[1]->write(i, m_font->as_u8(i));
    }
    // Copy the gamma correction table
    uint8_t gamma_1_8[] = {
        0x00, 0x0b, 0x11, 0x15, 0x19, 0x1c, 0x1f, 0x22, 0x25, 0x27, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34,
        0x36, 0x38, 0x3a, 0x3c, 0x3d, 0x3f, 0x41, 0x43, 0x44, 0x46, 0x47, 0x49, 0x4a, 0x4c, 0x4d, 0x4f,
        0x50, 0x51, 0x53, 0x54, 0x55, 0x57, 0x58, 0x59, 0x5b, 0x5c, 0x5d, 0x5e, 0x60, 0x61, 0x62, 0x63,
        0x64, 0x65, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75,
        0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x84,
        0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93,
        0x94, 0x95, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0x9f, 0xa0,
        0xa1, 0xa2, 0xa3, 0xa3, 0xa4, 0xa5, 0xa6, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xaa, 0xab, 0xac, 0xad,
        0xad, 0xae, 0xaf, 0xb0, 0xb0, 0xb1, 0xb2, 0xb3, 0xb3, 0xb4, 0xb5, 0xb6, 0xb6, 0xb7, 0xb8, 0xb8,
        0xb9, 0xba, 0xbb, 0xbb, 0xbc, 0xbd, 0xbd, 0xbe, 0xbf, 0xbf, 0xc0, 0xc1, 0xc2, 0xc2, 0xc3, 0xc4,
        0xc4, 0xc5, 0xc6, 0xc6, 0xc7, 0xc8, 0xc8, 0xc9, 0xca, 0xca, 0xcb, 0xcc, 0xcc, 0xcd, 0xce, 0xce,
        0xcf, 0xd0, 0xd0, 0xd1, 0xd2, 0xd2, 0xd3, 0xd4, 0xd4, 0xd5, 0xd6, 0xd6, 0xd7, 0xd7, 0xd8, 0xd9,
        0xd9, 0xda, 0xdb, 0xdb, 0xdc, 0xdc, 0xdd, 0xde, 0xde, 0xdf, 0xe0, 0xe0, 0xe1, 0xe1, 0xe2, 0xe3,
        0xe3, 0xe4, 0xe4, 0xe5, 0xe6, 0xe6, 0xe7, 0xe7, 0xe8, 0xe9, 0xe9, 0xea, 0xea, 0xeb, 0xec, 0xec,
        0xed, 0xed, 0xee, 0xef, 0xef, 0xf0, 0xf0, 0xf1, 0xf1, 0xf2, 0xf3, 0xf3, 0xf4, 0xf4, 0xf5, 0xf5,
        0xf6, 0xf7, 0xf7, 0xf8, 0xf8, 0xf9, 0xf9, 0xfa, 0xfb, 0xfb, 0xfc, 0xfc, 0xfd, 0xfd, 0xfe, 0xff
    };

    memcpy(m_iopage[0]->pointer(), gamma_1_8, 256);
    memcpy(m_iopage[0]->pointer() + 0x400, gamma_1_8, 256);
    memcpy(m_iopage[0]->pointer() + 0x800, gamma_1_8, 256);
    m_video->set_videoram(m_ram->pointer(), m_iopage[0]->pointer(), m_iopage[1]->pointer(), m_iopage[2]->pointer(), m_iopage[3]->pointer());
    m_video->start();

    // set the current time on the RTC device
    time_t now;
    time(&now);
    system_time stnow = system_time(now);

    m_rtc->set_current_time(stnow);

    // Initialize the VIA0
    m_via6522_0->write(via6522_device::VIA_DDRB, 0xFF);  // DDRB
    m_via6522_0->write(via6522_device::VIA_DDRA, 0xFF);  // DDRA
    m_via6522_0->write(via6522_device::VIA_PB,   0xFF);  // JOYSTICK 2
    m_via6522_0->write(via6522_device::VIA_PA,   0xFF);  // JOYSTICK 1
    m_via6522_0->write(via6522_device::VIA_DDRB, 0);     // DDRB
    m_via6522_0->write(via6522_device::VIA_DDRA, 0);     // DDRA

    // Initialize the VIA1
    m_via6522_1->write(via6522_device::VIA_PB, 0);
    m_via6522_1->write(via6522_device::VIA_PA, 0);
    m_via6522_1->write(via6522_device::VIA_DDRB, 0);     // DDRB
    m_via6522_1->write(via6522_device::VIA_DDRA, 0);     // DDRA

    // Initialize SD Card / SPI clock
    m_spi_clock = timer_alloc(FUNC(f256_state::spi_clock), this);
    save_item(NAME(m_spi_clock_state));
	save_item(NAME(m_spi_clock_sysclk));
	save_item(NAME(m_spi_clock_cycles));
	save_item(NAME(m_in_bit));
	save_item(NAME(m_in_latch));
	save_item(NAME(m_out_latch));
    save_item(NAME(m_iec_srq));

    m_timer0 = timer_alloc(FUNC(f256_state::timer0), this);
    m_timer1 = timer_alloc(FUNC(f256_state::timer1), this);
}

//-------------------------------------------------
//  device_reset
//-------------------------------------------------
void f256_state::device_reset()
{
	driver_device::device_reset();
    reset_mmu();
    m_via6522_0->reset();
	m_via6522_1->reset();

    m_opl3->reset();
    m_sdcard->reset();
    m_spi_clock->adjust(attotime::never);
	m_spi_clock_cycles = 0;
    m_in_bit = 0;
	m_spi_clock_state = false;
    spi_sd_enabled = 0;
    m_mouse->reset();

    m_sid0->reset();
    m_sid1->reset();
    m_sn0->reset();
    m_sn1->reset();
    //m_iec->reset();
    m_uart->reset();

    m_timer0_load = 0;
    m_timer0_val = 0;
    m_timer1_load = 0;
    m_timer1_val = 0;
}

//-------------------------------------------------
//  Interrupts
//-------------------------------------------------
void f256_state::sof_interrtupt(int state)
{
    if (state) // && ((m_interrupt_masks[1] & 0x01) == 0))
    {
        //logerror("SOF INTERRUPT: %02X\n", state);
        m_interrupt_reg[0] |= 0x01;
        m_maincpu->set_input_line(M6502_IRQ_LINE, state);
    }
}
void f256_state::sol_interrtupt(int state)
{
    if (state && ((m_interrupt_masks[1] & 0x02) == 0))
    {
        logerror("SOL INTERRUPT: %02X\n", state);
        m_interrupt_reg[0] |= 0x02;
        m_maincpu->set_input_line(M6502_IRQ_LINE, state);
    }
}
void f256_state::rtc_interrupt_handler(int state)
{
    // this is really odd: if I set state==1, then the interrupt gets only called once.
    if (state == 0 && ((m_interrupt_masks[1] & 0x10) == 0))
    {
        logerror("RTC INTERRUPT: %02X:%02X:%02X\n", m_rtc->read(4), m_rtc->read(2), m_rtc->read(0));
        m_interrupt_reg[1] |= 0x10;
        m_maincpu->set_input_line(M6502_IRQ_LINE, state);
    }
}

void f256_state::via0_interrupt(int state)
{
    // if a joystick button is pressed, set the VIA0 interrupt if the mask allows if
    if (state && ((m_interrupt_masks[1] & 0x20) == 0))
    {
        logerror("VIA0 INTERRUPT: %02X\n", state);
        m_interrupt_reg[1] |= 0x20;
        m_maincpu->set_input_line(M6502_IRQ_LINE, state);
    }
}
void f256_state::via1_interrupt(int state)
{
    // if a keyboard button is pressed, set the VIA1 interrupt if the mask allows if
    if (state && ((m_interrupt_masks[1] & 0x40) == 0))
    {
        logerror("VIA1 INTERRUPT: %02X\n", state);
        m_interrupt_reg[1] |= 0x40;
        m_maincpu->set_input_line(M6502_IRQ_LINE, state);
    }
}
void f256_state::dma_interrupt_handler(int state)
{
    // if (state && ((m_interrupt_masks[1] & 0x10) == 0))
    // {
    //     logerror("DMA Interrupt Not implemented!");
    //     m_interrupt_reg[1] |= 0x10;
    //     m_maincpu->set_input_line(M6502_IRQ_LINE, state);
    // }
}
void f256_state::timer0_interrupt_handler(int state)
{
    if (state && ((m_interrupt_masks[0] & 0x10) == 0))
    {
        logerror("TIMER0 INTERRUPT: %02X\n", state);
        m_interrupt_reg[0] |= 0x10;
        m_maincpu->set_input_line(M6502_IRQ_LINE, state);
    }
}
void f256_state::timer1_interrupt_handler(int state)
{
    logerror("Timer1 interrupt handler %d\n", state);
    if (state && ((m_interrupt_masks[0] & 0x20) == 0))
    {
        logerror("TIMER1 INTERRUPT: %02X\n", state);
        m_interrupt_reg[0] |= 0x20;
        m_maincpu->set_input_line(M6502_IRQ_LINE, state);
    }
}
TIMER_CALLBACK_MEMBER(f256_state::spi_clock)
{

	if (m_spi_clock_cycles > 0)
	{

		if (m_spi_clock_state)
		{
			m_sdcard->spi_clock_w(1);
			m_spi_clock_cycles--;
		}
		else
		{
			m_sdcard->spi_mosi_w(BIT(m_out_latch, 7));
			m_sdcard->spi_clock_w(0);
			m_out_latch <<= 1;
		}
        // toggle the clock signal
		m_spi_clock_state = !m_spi_clock_state;
	}
	else
	{
		m_spi_clock_state = false;
		m_spi_clock->adjust(attotime::never);
	}
}

// This is the optimized function for Timer0
TIMER_CALLBACK_MEMBER(f256_state::timer0)
{
    logerror("Timer0 reached value: %06X\n", m_timer0_load);
    uint8_t reg_t0 = m_iopage[0]->read(0xD650 - 0xC000);
    if ((reg_t0 & 0x80) !=0)
    {
        timer0_interrupt_handler(1);
    }
}
// TIMER_CALLBACK_MEMBER(f256_state::timer0)
// {
//     uint8_t reg_t0 = m_iopage[0]->read(0xD650 - 0xC000);
//     uint32_t cmp = m_iopage[0]->read(0xD655 - 0xC000) + (m_iopage[0]->read(0xD656 - 0xC000) << 8) +
//             (m_iopage[0]->read(0xD657 - 0xC000) << 16);

//     // if timer as reached value, then execute the action
//     if (m_timer0_eq == 1)
//     {
//         int8_t action = m_iopage[0]->read(0xD654 - 0xC000);
//         if (action & 1)
//         {
//             logerror("TIMER0 Cleared\n");
//             m_timer0_val = 0;
//         }
//         else
//         {
//             m_timer0_val = m_timer0_load;
//             logerror("TIMER0 Reloaded with: %X\n", m_timer0_val);
//         }
//         m_timer0_eq = 0;
//     }
//     else
//     {
//         if ((reg_t0 & 8) != 0)
//         {
//             // up
//             m_timer0_val++;

//             // it's a 24 bit register
//             if (m_timer0_val == 0x100'0000)
//             {
//                 m_timer0_val = 0;
//             }

//             if (m_timer0_val == cmp)
//             {
//                 m_timer0_eq = 1;
//                 logerror("TIMER0 up value reached %X\n", cmp);
//             }

//         }
//         else
//         {
//             // down
//             m_timer0_val--;
//             // roll over to 24 bits
//             if (m_timer0_val == 0xFFFF'FFFF)
//             {
//                 m_timer0_val = 0xFF'FFFF;
//             }
//             if (m_timer0_val == cmp)
//             {
//                 m_timer0_eq = 1;
//                 logerror("TIMER0 down value reached %X\n", cmp);
//             }
//         }
//         if (m_timer0_eq == 1 && (reg_t0 & 0x80) !=0)
//         {
//             timer0_interrupt_handler(1);
//         }
//     }
// }


// This timer is much slower than Timer0, so we can use single increments
TIMER_CALLBACK_MEMBER(f256_state::timer1)
{
    uint8_t reg_t1 = m_iopage[0]->read(0xD658 - 0xC000);
    uint32_t cmp = m_iopage[0]->read(0xD65D - 0xC000) + (m_iopage[0]->read(0xD65E - 0xC000) << 8) +
            (m_iopage[0]->read(0xD65F - 0xC000) << 16);
    logerror("TIMER1 event %06X CMP: %06X\n", m_timer1_val, cmp);
    // if timer as reached value, then execute the action
    if (m_timer1_eq == 1)
    {
        int8_t action = m_iopage[0]->read(0xD65C - 0xC000);
        if (action & 1)
        {
            logerror("TIMER1 Cleared\n");
            m_timer1_val = 0;
        }
        else
        {
            m_timer1_val = m_timer1_load;
            logerror("TIMER1 Reloaded with %X\n", m_timer1_val);
        }
        m_timer1_eq = 0;
    }
    else
    {
        if ((reg_t1 & 8) != 0)
        {
            // up
            m_timer1_val++;

            // it's a 24 bit register
            if (m_timer1_val == 0x100'0000)
            {
                m_timer1_val = 0;
            }

            if (m_timer1_val == cmp)
            {
                m_timer1_eq = 1;
                logerror("TIMER1 up value reached %X\n", cmp);
            }

        }
        else
        {
            // down
            m_timer1_val--;
            // roll over to 24 bits
            if (m_timer1_val == 0xFFFF'FFFF)
            {
                m_timer1_val = 0xFF'FFFF;
            }
            if (m_timer1_val == cmp)
            {
                m_timer1_eq = 1;
                logerror("TIMER1 up value reached %X\n", cmp);
            }
        }
        if (m_timer1_eq == 1 && (reg_t1 & 0x80) !=0)
        {
            timer1_interrupt_handler(1);
        }
    }
}

//-------------------------------------------------
//  VIA0 - JOYSTICK
//-------------------------------------------------
u8 f256_state::via0_system_porta_r()
{
    //logerror("VIA #0 Port A Read ioport JOY2: %02X\n", data);
    return m_joy2->read();
}
u8 f256_state::via0_system_portb_r()
{
    //logerror("VIA #0 Port B Read ioport JOY1: %02X\n", m_via_joy1);
    return m_via_joy1;
}
void f256_state::via0_system_porta_w(u8 data)
{
    //logerror("VIA #0 Port A Write: %02X\n", data);
    // writing should only be done if DDR allows it
    m_via6522_0->write_pa(data);
}
void f256_state::via0_system_portb_w(u8 data)
{
    //logerror("VIA #0 Port B Write: %02X\n", data);
    // writing should only be done if DDR allows it
    m_via6522_0->write_pb(data);
}
void f256_state::via0_ca2_write(u8 value)
{
    //logerror("Write to VIA0 - CA2 %02X\n", value);
    m_via6522_0->write_ca2(value);
}
void f256_state::via0_cb2_write(u8 value)
{
    //logerror("Write to VIA0 - CB2 %02X\n", value);
    m_via6522_0->write_cb2(value);
}

static INPUT_PORTS_START(f256k_mouse)
    PORT_START("MOUSE_X")
    PORT_BIT(0xff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

    PORT_START("MOUSE_Y")
    PORT_BIT(0xff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

    PORT_START("MOUSE_BUTTONS")
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left Button")
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right Button")
INPUT_PORTS_END

static INPUT_PORTS_START(f256k_joysticks)
    PORT_START("JOY1") /* Atari Joystick 1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(1) PORT_NAME("Atari Joystick P1 Up")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(1) PORT_NAME("Atari Joystick P1 Down")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(1) PORT_NAME("Atari Joystick P1 Left")
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1) PORT_NAME("Atari Joystick P1 Right")
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)                  PORT_PLAYER(1) PORT_NAME("Atari Joystick P1 Button 1")
    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)                  PORT_PLAYER(1) PORT_NAME("Atari Joystick P1 Button 2")
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3)                  PORT_PLAYER(1) PORT_NAME("Atari Joystick P1 Button 3")

	PORT_START("JOY2") /* Atari Joystick 2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(2) PORT_NAME("Atari Joystick P2 Up")
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(2) PORT_NAME("Atari Joystick P2 Down")
    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(2) PORT_NAME("Atari Joystick P2 Left")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2) PORT_NAME("Atari Joystick P2 Right")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)                  PORT_PLAYER(2) PORT_NAME("Atari Joystick P2 Button 1")
    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)                  PORT_PLAYER(2) PORT_NAME("Atari Joystick P2 Button 2")
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3)                  PORT_PLAYER(2) PORT_NAME("Atari Joystick P2 Button 3")
INPUT_PORTS_END

//-------------------------------------------------
//  VIA1 - F256K Keyboard
//-------------------------------------------------
u8 f256_state::via1_system_porta_r()
{
    //logerror("VIA1 Read Port A - %02X\n", m_via_keyboard_port_a);
    return m_via_keyboard_port_a;
}
u8 f256_state::via1_system_portb_r()
{
    //logerror("VIA1 Read Port B - %02X\n", m_via_keyboard_port_b);
    return m_via_keyboard_port_b;
}
// Read keyboard as rows
void f256_state::via1_system_porta_w(u8 data)
{
    //logerror("VIA1 Write Port A - %02X\n", data);

    m_via_keyboard_port_a = data;
    m_via_keyboard_port_b = 0xFF;
    // scan each keyboard row
    u8 joy1 = m_joy1->read();
    m_via_joy1 = joy1 | 0x80;
    for (int r = 0; r < 8; r++)
    {
        //if (BIT(m_via_keyboard_port_a,r) == 0)
        if (BIT(data, r) == 0)
        {
            uint16_t kbval = m_keyboard[r]->read();
            m_via_keyboard_port_b &= (kbval & 0xFF);
            if (r == 6 || r == 0)
            {
                if (BIT(kbval,8) == 0)
                {
                    m_via_joy1 = joy1;
                    //logerror("row: %d, kbval: %02X, joy1: %02X, porta: %02X, portb: %02X\n", r, kbval, m_via_joy1, m_via_keyboard_port_a, m_via_keyboard_port_b);
                }
            }
        }
    }
    m_via6522_1->write_pa(m_via_keyboard_port_a);
    m_via6522_0->write_pb(m_via_joy1);
}
// Read keyboard as columns
void f256_state::via1_system_portb_w(u8 data)
{
    m_via_keyboard_port_b = data;
    m_via6522_1->write_pb(data);
}
void f256_state::via1_ca2_write(u8 value)
{
    m_via6522_1->write_ca2(value);
}
void f256_state::via1_cb2_write(u8 value)
{
    m_via6522_1->write_cb2(value);
}

static INPUT_PORTS_START(f256k)
    PORT_INCLUDE( f256k_joysticks )
    PORT_INCLUDE( f256k_mouse )

	PORT_START("ROW0")
    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL")     PORT_CODE(KEYCODE_DEL)
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER")   PORT_CODE(KEYCODE_ENTER)
    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT)
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F7")      PORT_CODE(KEYCODE_F7)
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1")      PORT_CODE(KEYCODE_F1)
    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")      PORT_CODE(KEYCODE_F3)
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5")      PORT_CODE(KEYCODE_F5)
    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)   PORT_CODE(KEYCODE_UP)
    PORT_BIT(0x100,IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN)

    PORT_START("ROW1")
    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")       PORT_CODE(KEYCODE_3)
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")       PORT_CODE(KEYCODE_W)
    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")       PORT_CODE(KEYCODE_A)
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")       PORT_CODE(KEYCODE_4)
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")       PORT_CODE(KEYCODE_Z)
    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")       PORT_CODE(KEYCODE_S)
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")       PORT_CODE(KEYCODE_E)
    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L SHIFT") PORT_CODE(KEYCODE_LSHIFT)

    PORT_START("ROW2")
    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)

    PORT_START("ROW3")
    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)

    PORT_START("ROW4")
    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)

    PORT_START("ROW5")
    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)

    PORT_START("ROW6")
    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=")         PORT_CODE(KEYCODE_EQUALS)
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]")         PORT_CODE(KEYCODE_CLOSEBRACE)
    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("'")         PORT_CODE(KEYCODE_QUOTE)
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("HOME")      PORT_CODE(KEYCODE_HOME)
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R SHIFT")   PORT_CODE(KEYCODE_RSHIFT)
    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ALT")       PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB")       PORT_CODE(KEYCODE_TAB)
    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/")         PORT_CODE(KEYCODE_SLASH)
    PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT)

    PORT_START("ROW7")
    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")      PORT_CODE(KEYCODE_1)
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BKSP")   PORT_CODE(KEYCODE_BACKSPACE)
    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")   PORT_CODE(KEYCODE_LCONTROL)
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")      PORT_CODE(KEYCODE_2)
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE")  PORT_CODE(KEYCODE_SPACE)
    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FOENIX") PORT_CODE(KEYCODE_LWIN)
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")      PORT_CODE(KEYCODE_Q)
    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RUN/STOP") PORT_CODE(KEYCODE_STOP)
INPUT_PORTS_END

ROM_START(f256k)
	ROM_REGION(0x10'0000,ROM_TAG,0)
    // Offsets are based on the REGION base address - offset by 0x10 because of map.
    ROM_LOAD("xdev.bin",             0x08'2000, 0x2000, CRC(5cee0cb0) SHA1(a5fb10ad914069f506847150bdd387371e73f1de))
    ROM_LOAD("sb01.bin",             0x08'4000, 0x2000, CRC(21f06e73) SHA1(bbeefb52d4b126b61367169c21599180f3358af7))
    ROM_LOAD("sb02.bin",             0x08'6000, 0x2000, CRC(6ed611b9) SHA1(4a03aa286f6274e6974a3cecdedad651a58f5fb1))
    ROM_LOAD("sb03.bin",             0x08'8000, 0x2000, CRC(653f849d) SHA1(65942d98f26b86499e6359170aa2d0c6e16124ff))
    ROM_LOAD("sb04.bin",             0x08'A000, 0x2000, CRC(f4aa6049) SHA1(11f02fee6ec412f0c96b27b0b149f72cf1770d15))
    ROM_LOAD("dos.bin",              0x08'C000, 0x2000, CRC(f3673c4e) SHA1(9c6b70067d7195d4a6bbd7f379b8e5382bf8cc1b))
    ROM_LOAD("pexec.bin",            0x08'E000, 0x2000, CRC(937c1374) SHA1(40566a51d2ef7321a42fe926b03dee3571c78202))
	ROM_LOAD("3b.bin",               0x0F'6000, 0x2000, CRC(7c5d2f27) SHA1(bd1ece74b02a210cfe5a1ed15a0febefc39a1861))
    ROM_LOAD("3c.bin",               0x0F'8000, 0x2000, CRC(2e2295d1) SHA1(9049b83d4506b49701669c335ded2879c7992751))
    ROM_LOAD("3d.bin",               0x0F'A000, 0x2000, CRC(97743cb7) SHA1(693fa7762528eca6a75c9ea30a603dadc4d55cf9))
    ROM_LOAD("3e.bin",               0x0F'C000, 0x2000, CRC(9012398f) SHA1(4ae1e37aa3ad4c2b498bf1797d591d7fa25a9d43))
    ROM_LOAD("3f.bin",               0x0F'E000, 0x2000, CRC(b9ddda5e) SHA1(2f21ef84a269cc2ed25c6441c9451f61dbb5b285))
    ROM_LOAD("docs_superbasic1.bin", 0x09'6000, 0x2000, CRC(ad6398cd) SHA1(d926ae72f8f3af2a0b15ac165bc680db2e647740))
    ROM_LOAD("docs_superbasic2.bin", 0x09'8000, 0x2000, CRC(3cf07824) SHA1(b92e88a99ccf51461f45d317e3e555c5d62792eb))
    ROM_LOAD("docs_superbasic3.bin", 0x09'A000, 0x2000, CRC(838cb5df) SHA1(103b182ad76c185c4a779f4865c48c5fc71e2a14))
    ROM_LOAD("docs_superbasic4.bin", 0x09'C000, 0x2000, CRC(bf7841b9) SHA1(7dcbf77c46d680a1c47ac11ed871b832a1479e8e))
    ROM_LOAD("help.bin",             0x09'4000, 0x2000, CRC(b7d63466) SHA1(bd1dafb5849dee61fd48ece16a409e56de62f464))

    // Load the file manager application
    ROM_LOAD("fm.00",              0x0A'0000, 0x2000, CRC(f877c712) SHA1(18d7b2a484ef6dc2ee4a83b6a4dd28ffc3ffe26f))
    ROM_LOAD("fm.01",              0x0A'2000, 0x2000, CRC(de843f4b) SHA1(f132ebf997a57fd15fcaa51ab9ff9a4aa40183a0))
    ROM_LOAD("fm.02",              0x0A'4000, 0x2000, CRC(9c90cf7d) SHA1(0ed0a09f7bb1eeb0a99141e9d4625d027448b7ea))
    ROM_LOAD("fm.03",              0x0A'6000, 0x2000, CRC(7dfe0934) SHA1(48d2aa2f4c926f7a81e9eab312883e4e573e7cd9))
    ROM_LOAD("fm.04",              0x0A'8000, 0x2000, CRC(3bc5832d) SHA1(5a8491e4c6e4e56e4017ece97615f7eb5aa928e3))
    ROM_LOAD("fm.05",              0x0A'A000, 0x2000, CRC(48dcadc7) SHA1(52e051d5e8446deddfc469f7aed0f18fb2483715))
    ROM_LOAD("fm.06",              0x0A'C000, 0x2000, CRC(035c3c8a) SHA1(649601087b7b95a4f432f362428e98291648434c))
    ROM_LOAD("fm.07",              0x0A'E000, 0x2000, CRC(76f749d4) SHA1(a3e7e881f4c4fd39c94385de2c8d7546a2239d96))

    ROM_REGION(0x0800,FONT_TAG,0)
    ROM_LOAD("f256jr_font_micah_jan25th.bin", 0x0000, 0x0800, CRC(6d66da85) SHA1(377dc27ff3a4ae2d80d740b2d16373f8e639eef6))
ROM_END

//    YEAR  NAME   PARENT COMPAT  MACHINE    INPUT    CLASS        INIT        COMPANY              FULLNAME                        FLAGS
COMP( 2024, f256k,    0,      0,    f256k,   f256k,   f256_state, empty_init, "Stefany Allaire", "F256K 8-bit Retro System",    MACHINE_UNOFFICIAL  )
