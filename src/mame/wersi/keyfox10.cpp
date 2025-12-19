// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "cpu/mcs51/i80c52.h"
#include "bus/midi/midi.h"

#define LOG_SERIAL (1U << 1)
#define LOG_PORT   (1U << 2)
#define LOG_SAM    (1U << 3)
#define VERBOSE (LOG_SERIAL | LOG_SAM)
#include "logmacro.h"

namespace {

// SAM8905 DSP state
// Control byte: [X X X X | SSR | IDL | SEL | WR]
//   SSR: 0=44.1kHz, 1=22.05kHz sample rate
//   IDL: 0=normal, 1=all 16 tones output
//   SEL: 0=D-RAM, 1=A-RAM
//   WR:  0=read, 1=write
struct sam8905_state
{
    u8 ctrl = 0;
    u8 addr = 0;
    u16 aram[8 * 32] = {};   // A-RAM: 8 slots × 32 words × 15 bits
    u32 dram[16 * 16] = {};  // D-RAM: 16 slots × 16 words × 19 bits
};

class keyfox10_state : public driver_device
{
public:
    keyfox10_state(const machine_config &mconfig, device_type type, const char *tag)
        : driver_device(mconfig, type, tag)
        , m_maincpu(*this, "maincpu")
        , m_midi_out(*this, "mdout")
    { }

    void keyfox10(machine_config &config);

private:
    void program_map(address_map &map) ATTR_COLD;
    void data_map(address_map &map) ATTR_COLD;

    void port0_w(u8 data);
    void port1_w(u8 data);
    void port2_w(u8 data);
    void port3_w(u8 data);
    u8 port0_r();
    u8 port1_r();
    u8 port2_r();
    u8 port3_r();

    // SAM8905 DSP handlers
    u8 sam8905_r(offs_t offset);
    void sam8905_w(offs_t offset, u8 data);

    void midi_rxd_w(int state) { m_midi_rxd = state; }

    required_device<i80c32_device> m_maincpu;
    required_device<midi_port_device> m_midi_out;
    u8 m_port0 = 0xff;
    u8 m_port1 = 0xff;
    u8 m_port2 = 0xff;
    u8 m_port3 = 0xff;
    u8 m_midi_rxd = 1;  // MIDI RX bit (idle high)

    // SAM8905 DSP - two chips selected via T1 (P3.5)
    // T1=1: Sound generation SAM (index 0)
    // T1=0: Effects SAM (index 1)
    sam8905_state m_sam[2];

    int sam_chip_select() const { return BIT(m_port3, 5) ? 0 : 1; }
};

void keyfox10_state::program_map(address_map &map)
{
    // 80C32 has 16-bit address space; ROM banking for 128KB ROM TBD
    map(0x0000, 0xffff).rom().region("program", 0);
}

void keyfox10_state::data_map(address_map &map)
{
    map(0x0000, 0x1fff).ram();  // 8KB RAM
    map(0x8000, 0x800f).rw(FUNC(keyfox10_state::sam8905_r), FUNC(keyfox10_state::sam8905_w));  // SAM8905 DSP
}

u8 keyfox10_state::port0_r()
{
    LOGMASKED(LOG_PORT, "P0 read: 0x%02X\n", m_port0);
    return m_port0;
}

u8 keyfox10_state::port1_r()
{
    LOGMASKED(LOG_PORT, "P1 read: 0x%02X\n", m_port1);
    return m_port1;
}

u8 keyfox10_state::port2_r()
{
    LOGMASKED(LOG_PORT, "P2 read: 0x%02X\n", m_port2);
    return m_port2;
}

u8 keyfox10_state::port3_r()
{
    // P3.0 = MIDI RXD input
    u8 data = (m_port3 & 0xfe) | (m_midi_rxd & 0x01);
    LOGMASKED(LOG_PORT, "P3 read: 0x%02X\n", data);
    return data;
}

void keyfox10_state::port0_w(u8 data)
{
    if (m_port0 != data)
        LOGMASKED(LOG_PORT, "P0 write: 0x%02X\n", data);
    m_port0 = data;
}

void keyfox10_state::port1_w(u8 data)
{
    if (m_port1 != data)
        LOGMASKED(LOG_PORT, "P1 write: 0x%02X\n", data);
    m_port1 = data;
}

void keyfox10_state::port2_w(u8 data)
{
    if (m_port2 != data)
        LOGMASKED(LOG_PORT, "P2 write: 0x%02X\n", data);
    m_port2 = data;
}

void keyfox10_state::port3_w(u8 data)
{
    if (m_port3 != data)
        LOGMASKED(LOG_PORT, "P3 write: 0x%02X\n", data);
    // P3.1 = TXD output to MIDI
    if ((m_port3 ^ data) & 0x02)
        m_midi_out->write_txd(BIT(data, 1));
    m_port3 = data;
}

u8 keyfox10_state::sam8905_r(offs_t offset)
{
    int chip = sam_chip_select();
    sam8905_state &sam = m_sam[chip];
    const char *chip_name = chip ? "FX" : "SND";

    u8 a2 = BIT(offset, 2);
    u8 a1 = BIT(offset, 1);
    u8 a0 = BIT(offset, 0);
    bool sel_aram = BIT(sam.ctrl, 1);  // SEL bit

    if (a2)
    {
        // A2=1: Control register (write-only, return 0xff)
        LOGMASKED(LOG_SAM, "SAM[%s] read ctrl (invalid)\n", chip_name);
        return 0xff;
    }

    if (a1 == 0 && a0 == 0)
    {
        // A2=0, A1=0, A0=0: Read interrupt status
        LOGMASKED(LOG_SAM, "SAM[%s] read interrupt status\n", chip_name);
        return 0x00;  // TODO: implement interrupt status
    }

    // Data read from A-RAM or D-RAM
    if (sel_aram)
    {
        // A-RAM: 15-bit words, accessed via 2 bytes
        u16 word = sam.aram[sam.addr % 256];
        if (a1 == 0 && a0 == 1)
        {
            // LSB
            LOGMASKED(LOG_SAM, "SAM[%s] A-RAM[0x%02X] read LSB = 0x%02X\n", chip_name, sam.addr, word & 0xff);
            return word & 0xff;
        }
        else if (a1 == 1 && a0 == 0)
        {
            // MSB (bit 7 undefined)
            LOGMASKED(LOG_SAM, "SAM[%s] A-RAM[0x%02X] read MSB = 0x%02X\n", chip_name, sam.addr, (word >> 8) & 0x7f);
            return (word >> 8) & 0x7f;
        }
    }
    else
    {
        // D-RAM: 19-bit words, accessed via 3 bytes
        u32 word = sam.dram[sam.addr % 256];
        if (a1 == 0 && a0 == 1)
        {
            // LSB
            LOGMASKED(LOG_SAM, "SAM[%s] D-RAM[0x%02X] read LSB = 0x%02X\n", chip_name, sam.addr, word & 0xff);
            return word & 0xff;
        }
        else if (a1 == 1 && a0 == 0)
        {
            // NSB (next significant byte)
            LOGMASKED(LOG_SAM, "SAM[%s] D-RAM[0x%02X] read NSB = 0x%02X\n", chip_name, sam.addr, (word >> 8) & 0xff);
            return (word >> 8) & 0xff;
        }
        else if (a1 == 1 && a0 == 1)
        {
            // MSB (5 high bits undefined)
            LOGMASKED(LOG_SAM, "SAM[%s] D-RAM[0x%02X] read MSB = 0x%02X\n", chip_name, sam.addr, (word >> 16) & 0x07);
            return (word >> 16) & 0x07;
        }
    }

    LOGMASKED(LOG_SAM, "SAM[%s] read: [0x%04X] (unhandled)\n", chip_name, 0x8000 + offset);
    return 0xff;
}

void keyfox10_state::sam8905_w(offs_t offset, u8 data)
{
    int chip = sam_chip_select();
    sam8905_state &sam = m_sam[chip];
    const char *chip_name = chip ? "FX" : "SND";

    u8 a2 = BIT(offset, 2);
    u8 a1 = BIT(offset, 1);
    u8 a0 = BIT(offset, 0);
    bool sel_aram = BIT(sam.ctrl, 1);  // SEL bit

    if (a2)
    {
        // A2=1: Write control byte
        sam.ctrl = data;
        LOGMASKED(LOG_SAM, "SAM[%s] ctrl = 0x%02X (SSR=%d IDL=%d SEL=%s WR=%s)\n",
            chip_name, data,
            BIT(data, 3),  // SSR
            BIT(data, 2),  // IDL
            BIT(data, 1) ? "A-RAM" : "D-RAM",  // SEL
            BIT(data, 0) ? "write" : "read");  // WR
        return;
    }

    if (a1 == 0 && a0 == 0)
    {
        // A2=0, A1=0, A0=0: Write address selection byte
        sam.addr = data;
        LOGMASKED(LOG_SAM, "SAM[%s] addr = 0x%02X\n", chip_name, data);
        return;
    }

    // Data write to A-RAM or D-RAM
    if (sel_aram)
    {
        // A-RAM: 15-bit words, accessed via 2 bytes
        if (a1 == 0 && a0 == 1)
        {
            // LSB
            sam.aram[sam.addr % 256] = (sam.aram[sam.addr % 256] & 0x7f00) | data;
            LOGMASKED(LOG_SAM, "SAM[%s] A-RAM[0x%02X] write LSB = 0x%02X\n", chip_name, sam.addr, data);
        }
        else if (a1 == 1 && a0 == 0)
        {
            // MSB (bit 7 undefined, mask to 7 bits)
            sam.aram[sam.addr % 256] = (sam.aram[sam.addr % 256] & 0x00ff) | ((data & 0x7f) << 8);
            LOGMASKED(LOG_SAM, "SAM[%s] A-RAM[0x%02X] write MSB = 0x%02X -> word = 0x%04X\n",
                chip_name, sam.addr, data, sam.aram[sam.addr % 256]);
        }
    }
    else
    {
        // D-RAM: 19-bit words, accessed via 3 bytes
        if (a1 == 0 && a0 == 1)
        {
            // LSB
            sam.dram[sam.addr % 256] = (sam.dram[sam.addr % 256] & 0x07ff00) | data;
            LOGMASKED(LOG_SAM, "SAM[%s] D-RAM[0x%02X] write LSB = 0x%02X\n", chip_name, sam.addr, data);
        }
        else if (a1 == 1 && a0 == 0)
        {
            // NSB
            sam.dram[sam.addr % 256] = (sam.dram[sam.addr % 256] & 0x0700ff) | (data << 8);
            LOGMASKED(LOG_SAM, "SAM[%s] D-RAM[0x%02X] write NSB = 0x%02X\n", chip_name, sam.addr, data);
        }
        else if (a1 == 1 && a0 == 1)
        {
            // MSB (5 high bits undefined, mask to 3 bits)
            sam.dram[sam.addr % 256] = (sam.dram[sam.addr % 256] & 0x00ffff) | ((data & 0x07) << 16);
            LOGMASKED(LOG_SAM, "SAM[%s] D-RAM[0x%02X] write MSB = 0x%02X -> word = 0x%05X\n",
                chip_name, sam.addr, data, sam.dram[sam.addr % 256]);
        }
    }
}

static INPUT_PORTS_START(keyfox10)
INPUT_PORTS_END

void keyfox10_state::keyfox10(machine_config &config)
{
    I80C32(config, m_maincpu, 16_MHz_XTAL);
    m_maincpu->set_addrmap(AS_PROGRAM, &keyfox10_state::program_map);
    m_maincpu->set_addrmap(AS_DATA, &keyfox10_state::data_map);
    m_maincpu->port_in_cb<0>().set(FUNC(keyfox10_state::port0_r));
    m_maincpu->port_in_cb<1>().set(FUNC(keyfox10_state::port1_r));
    m_maincpu->port_in_cb<2>().set(FUNC(keyfox10_state::port2_r));
    m_maincpu->port_in_cb<3>().set(FUNC(keyfox10_state::port3_r));
    m_maincpu->port_out_cb<0>().set(FUNC(keyfox10_state::port0_w));
    m_maincpu->port_out_cb<1>().set(FUNC(keyfox10_state::port1_w));
    m_maincpu->port_out_cb<2>().set(FUNC(keyfox10_state::port2_w));
    m_maincpu->port_out_cb<3>().set(FUNC(keyfox10_state::port3_w));

    // MIDI
    midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
    mdin.rxd_handler().set(FUNC(keyfox10_state::midi_rxd_w));
    MIDI_PORT(config, "mdout", midiout_slot, "midiout");
}

ROM_START(keyfox10)
    ROM_REGION(0x20000, "program", 0)
    ROM_LOAD("kf10_ic27_v2.bin", 0x0000, 0x20000, CRC(c04e40a9) SHA1(668d40761658f1863ab028ed317141839d1075ac))
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME      FLAGS
SYST( 1990, keyfox10, 0,      0,      keyfox10, keyfox10, keyfox10_state, empty_init, "Wersi", "Keyfox 10",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
