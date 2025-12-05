// license:BSD-3-Clause
// copyright-holders:H.Janetzek
//
// Uses modified code from ympsr60.cpp

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/i86/i186.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/flt_rc.h"
#include "sound/honmeg.h"
#include "sound/hohnerpcm.h"
#include "video/hd61830.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <array>
#include <bitset>

#include "ustudio.lh"

#define LOG_DEV    (1 << 1U)
#define LOG_AUDIO  (1 << 2U)
#define LOG_MATRIX (1 << 3U)
#define LOG_SERIAL (1 << 4U)
#define LOG_LCD    (1 << 5U)
#define LOG_ACIA   (1 << 6U)
#define LOG_RHY    (1 << 7U)

//#define VERBOSE (LOG_DEV | LOG_AUDIO | LOG_MATRIX | LOG_SERIAL | LOG_LCD)
#define VERBOSE (LOG_DEV)
#include "logmacro.h"

namespace
{

static constexpr int DRVIF_MAX_TARGETS = 9;

class ustudio_state : public driver_device
{
public:
    ustudio_state(const machine_config& mconfig, device_type type, const char* tag)
    : driver_device(mconfig, type, tag)
    , m_maincpu(*this, "maincpu")
    , m_meg(*this, "meg")
    , m_meg_filter(*this, "meg_filter_%u", 0U)
    , m_rhythm_pcm(*this, "rhythm_pcm")
    , m_acia(*this, "acia")
    , m_lcdc(*this, "lcdc")
    , m_matrix(*this, "M%u", 0)
    , m_drvif(*this, "K%u", 0)
    , m_led_panels(*this, "K%u_%u", 0U, 0U)
    , m_led_beat(*this, "BEAT") {}

    void ustudio_common(machine_config& config);
    void ustudio_proto(machine_config& config);
    void ustudio(machine_config& config);

protected:
    virtual void machine_start() override;

    void mem_map_proto(address_map& map);
    void mem_map(address_map& map);
    void pcs_map(address_map& map);
    void lcd_map(address_map& map);
    void meg_memory_map(address_map& map);
    void pcm_memory_map(address_map& map);
    
    required_device<i80186_cpu_device> m_maincpu;
    required_device<honmeg_device> m_meg;
    required_device_array<filter_rc_device, 8> m_meg_filter;
    required_device<hohnerpcm_device> m_rhythm_pcm;
    required_device<acia6850_device> m_acia;
    required_device<hd61830_device> m_lcdc;

    required_ioport_array<6> m_matrix;
    required_ioport_array<DRVIF_MAX_TARGETS> m_drvif;
    output_finder<DRVIF_MAX_TARGETS, 8> m_led_panels;
    output_finder<1> m_led_beat;

private:

    std::array<std::bitset<8>, 9> m_serial_register;

    u8 matrix_read_row = 0;

    struct
    {
        u8 clk = 0;
        u8 audio2 = -1;
        u8 audio3 = -1;
        u8 audio4 = -1;
        u8 downbeat = -1;
    } m_serial;

    bool serial_read_panel = false;
    s8 serial_read_count = 127;

    
    void acia_irq_w(int state)
    {
        m_maincpu->drq1_w(state);
        m_maincpu->int1_w(state);
    }
    u8 acia_r(offs_t offset)
    {
        auto data = m_acia->read(offset);
        if (offset == 1)
        {
            LOGMASKED(LOG_ACIA, "rd_acia %d 0x%02x (%c)\n", offset, data, (offset == 1 && data > 0x20 && data < 0x7f) ? (char) data : ' ');
        }
        return data;
    }
    void acia_w(offs_t offset, u8 data)
    {
        LOGMASKED(LOG_ACIA, "wr_acia %d 0x%02x (%c)\n", offset, data, (offset == 1 && data > 0x20 && data < 0x7f) ? (char) data : ' ');
        m_acia->write(offset, data);
    }

    template<size_t I>
    u8 mem_r8_io(offs_t offset)
    {
        LOGMASKED(LOG_DEV, "%zu mem_r8_io %02X (%d)\n", I, offset, offset);
        return 0;
    }

    template<size_t I>
    void mem_w8_io(offs_t offset, u8 data)
    {
        LOGMASKED(LOG_DEV, "%zu mem_w8_io %02X (%d) : %02X\n", I, offset, offset, data);
    }

    template<size_t I>
    u16 mem_r_io(offs_t offset)
    {
        LOGMASKED(LOG_DEV, "%zu mem_r_io %04X (%d)\n", I, offset, offset);
        return 0;
    }

    template<size_t I>
    void mem_w_io(offs_t offset, u16 data)
    {
        LOGMASKED(LOG_DEV, "%zu mem_w_io %04X (%d) : %04X\n", I, offset, offset, data);
    }
    u8 lcd_r(offs_t offset)
    {
        if (offset == 2)
        {
            auto val = m_lcdc->status_r();
            LOGMASKED(LOG_LCD, "lcd_r_stat %04X (%d) : %02x\n", offset, offset, val);
            return val;
        }

        if (offset == 0)
        {
            auto val = m_lcdc->data_r();
            LOGMASKED(LOG_LCD, "lcd_r_data %04X (%d) : %02x\n", offset, offset, val);
            return val;  // << 8;
        }

        LOGMASKED(LOG_DEV, "lcd_r %04X (%d) ERROR\n", offset, offset);
        return 0;
    }

    void lcd_w(offs_t offset, u8 data)
    {
        if (offset == 0)
        {
            LOGMASKED(LOG_LCD, "lcd_w_data %04X (%d) : %02X\n", offset, offset, data);
            m_lcdc->data_w(data);
        }
        else if (offset == 2)
        {
            LOGMASKED(LOG_LCD, "lcd_w_ctrl %04X (%d) : %02X\n", offset, offset, data);
            m_lcdc->control_w(data);
        }
        else
        {
            LOGMASKED(LOG_DEV, "lcd_w %04X (%d) : %04X ERROR\n", offset, offset, data);
        }
    }

    void meg_w(address_space& space, offs_t offset, u16 data, u16 mem_mask)
    {
        m_meg->write(offset, data >> 1);
    }

    u16 meg_r(address_space& space, offs_t offset)
    {
        auto val = m_meg->read(offset);
        return val << 1;
    }

    void pcs0_w(offs_t offset, u8 data)
    {
        matrix_read_row = ~data;
        LOGMASKED(LOG_MATRIX, "w pcs0 matrix %02X (%d) => %02X, %02X\n", offset, offset, data, matrix_read_row);
    }

    u8 pcs0_r(offs_t offset)
    {
        // NB: This is intentionally not a switch
        u8 val = 0xff;
        if (matrix_read_row & 0x01)
        {
            LOGMASKED(LOG_MATRIX, "r pcs0 matrix invalid row 0x01\n");
        }
        if (matrix_read_row & 0x02)
        {
            LOGMASKED(LOG_MATRIX, "r pcs0 matrix row:0 %02X\n", m_matrix[0]->read());
            val &= m_matrix[0]->read();
        }
        if (matrix_read_row & 0x04)
        {
            LOGMASKED(LOG_MATRIX, "r pcs0 matrix row:1 %02X\n", m_matrix[1]->read());
            val &= m_matrix[1]->read();
        }
        if (matrix_read_row & 0x08)
        {
            LOGMASKED(LOG_MATRIX, "r pcs0 matrix row:2 %02X\n", m_matrix[2]->read());
            val &= m_matrix[2]->read();
        }
        if (matrix_read_row & 0x10)
        {
            LOGMASKED(LOG_MATRIX, "r pcs0 matrix row:3 %02X\n", m_matrix[3]->read());
            val &= m_matrix[3]->read();
        }
        if (matrix_read_row & 0x20)
        {
            LOGMASKED(LOG_MATRIX, "r pcs0 matrix row:4 %02X\n", m_matrix[4]->read());
            val &= m_matrix[4]->read();
        }
        if (matrix_read_row & 0x40)
        {
            LOGMASKED(LOG_MATRIX, "r pcs0 matrix invalid row 0x40\n");
        }
        if (matrix_read_row & 0x80)
        {
            LOGMASKED(LOG_MATRIX, "r pcs0 matrix row:5 %02X\n", m_matrix[5]->read());
            val &= m_matrix[5]->read();
        }
        LOGMASKED(LOG_MATRIX, "r pcs0 matrix %02X (%d) <= %02X\n", offset, offset, val);

        return val;
    }

    void pcs1_cartridge_w(offs_t offset, u8 data)
    {
        LOGMASKED(LOG_DEV, "w pcs1 cart   %04X (%d) : %02X\n", offset, offset, data);
    }
    void pcs1_audio_w(offs_t offset, u8 data)
    {
        LOGMASKED(LOG_AUDIO, "w pcs1 audio  %04X (%d) : %02X\n", offset, offset, data);
    }

    void pcs2_audio_w(offs_t offset, u8 data)
    {
        LOGMASKED(LOG_AUDIO, "w pcs2 audio  %04X (%d) : %02X\n", offset, offset, data);
    }

    void pcs3_w(offs_t offset, u8 data)
    {
        LOGMASKED(LOG_DEV, "w pcs3         %04X (%d) : %02X\n", offset, offset, data);
    }

    void pcs3_audio_w(offs_t offset, u8 data)
    {
        LOGMASKED(LOG_AUDIO, "w pcs3 audio  %04X (%d) : %02X\n", offset, offset, data);
    }

    void pcs4_rhythm_w(offs_t offset, u8 data)
    {
        LOGMASKED(LOG_RHY, "w pcs4 rhythm  %04X (%d) : %02X\n", offset, offset, data);
        m_rhythm_pcm->write(offset, data);
    }

    void pcs4_cartridge_w(offs_t offset, u8 data)
    {
        LOGMASKED(LOG_DEV, "w pcs4 cartridge %04X (%d) : %02X\n", offset, offset, data);
    }

    u8   pcs4_cartridge_r(offs_t offset)
    {
        LOGMASKED(LOG_DEV, "r pcs4 cartridge %04X (%d) : %02X\n", offset, offset);
        return 0xff;
    }

    void write_serial(const u8 data)
    {
        auto bit0 = (data & 0x1);
        for (auto i = 0; i < m_serial_register.size(); ++i)
        {
            auto& r    = m_serial_register[i];
            auto  bit7 = r[7] ? 1 : 0;
            r <<= 1;
            r[0] = bit0;
            bit0 = bit7;

            if (m_led_panels[i])
            {
                for (auto bit = 0; bit < 8; ++bit)
                {
                    m_led_panels[i][bit] = !r[bit];
                }
            }
        }
    };

    void pcs5_serial_w(offs_t offset, u8 data)
    {
        if (offset != 0)
        {
            return;
        }
        auto audio_4 = BIT(data, 3) == 0 ? 1 : 0;
        auto audio_3 = BIT(data, 5) == 0 ? 1 : 0;
        auto audio_2 = BIT(data, 4) == 0 ? 1 : 0;

        auto serial_clk = BIT(data, 7);  // == 0 ? 1 : 0;
        auto serial_oe = BIT(data, 1) == 0 ? 1 : 0;
        auto serial_data = BIT(data, 0) == 0 ? 1 : 0;

        // if (serial_read_panel)
        // {
        //     LOGMASKED(LOG_SERIAL, "w pcs5 serial %04X (%d) : %04X clk:%d oe:%d val:%d\n",
        //                offset, offset, data, serial_clk, serial_oe, serial_data);
        // }
        if (serial_clk == 1 && m_serial.clk == 0)
        {
            if (!serial_oe)
            {
                serial_read_count = 0;
                serial_read_panel = true;
            }
            else if (serial_read_count < 72)
            {
                // TODO this actually also write to m_serial_register
                // to compare with serial-panel input.
                auto pos = serial_read_count;
                if (pos >= 0 && m_drvif[pos / 8]->read() == 1 << (pos % 8))
                {
                    m_maincpu->int2_w(0);
                    serial_read_count = 127;
                }
                else
                {
                    serial_read_count++;
                }
            }
            else
            {
                // LOGMASKED(LOG_SERIAL, "w pcs5 serial %04X (%d) : %04X clk:%d oe:%d val:%d\n", offset, offset, data, serial_clk,
                // serial_oe, serial_data);
                write_serial(serial_data);
            }
        }

        if (serial_read_panel && serial_read_count == 127)
        {
            if (!serial_oe)
            {
                m_maincpu->int2_w(1);
                serial_read_panel = false;
            }
        }

        m_serial.clk = serial_clk;

        if ((audio_2 != m_serial.audio2) || (audio_3 != m_serial.audio3) || (audio_4 != m_serial.audio4))
        {
            m_serial.audio2 = audio_2;
            m_serial.audio3 = audio_3;
            m_serial.audio4 = audio_4;
            LOGMASKED(LOG_AUDIO, "w pcs5 audio  %04X (%d) : %04X - %d %d %d\n", offset, offset, data, audio_2, audio_3, audio_4);
        }

        auto downbeat = BIT(data, 6);
        if (downbeat != m_serial.downbeat)
        {
            // LOGMASKED(LOG_DEV, "w pcs5 beat   %04X (%d) : %d\n", offset, offset, downbeat);
            m_led_beat[0] = downbeat;
        }
        m_serial.downbeat = downbeat;
    }

public:
    INPUT_CHANGED_MEMBER(drvif_changed);
};

void ustudio_state::meg_memory_map(address_map& map)
{
    // map(0x0000, 0x0fff).rom().region("wf90", 0);
    map(0x0000, 0x0fff).rom().region("wf5000", 0);
}

void ustudio_state::pcm_memory_map(address_map& map)
{
    map(0x0000, 0xffff).rom().region("ry5000", 0);
}


void ustudio_state::pcs_map(address_map& map) {
    // PCS0
    map(0x38800, 0x3887f).rw(FUNC(ustudio_state::pcs0_r), FUNC(ustudio_state::pcs0_w));
    // PCS1
    map(0x38880, 0x388ff).w(FUNC(ustudio_state::pcs1_audio_w)).umask16(0x00ff);
    map(0x38880, 0x388ff).w(FUNC(ustudio_state::pcs1_cartridge_w)).umask16(0xff00);
    // PCS2
    map(0x38900, 0x3897f).w(FUNC(ustudio_state::pcs2_audio_w)).umask16(0x00ff);
    map(0x38900, 0x3897f).rw(FUNC(ustudio_state::acia_r), FUNC(ustudio_state::acia_w)).umask16(0xff00);
    // PCS3
    map(0x38980, 0x389ff).w(FUNC(ustudio_state::pcs3_audio_w)).umask16(0x00ff);
    map(0x38980, 0x389ff).w(FUNC(ustudio_state::pcs3_w)).umask16(0xff00);
    // PCS4
    map(0x38a00, 0x38a7f).w(FUNC(ustudio_state::pcs4_rhythm_w)).umask16(0x00ff);
    map(0x38a00, 0x38a7f).rw(FUNC(ustudio_state::pcs4_cartridge_r), FUNC(ustudio_state::pcs4_cartridge_w)).umask16(0xff00);
    // PCS5
    map(0x38a80, 0x38a83).w(FUNC(ustudio_state::pcs5_serial_w));
    map(0x38a81, 0x38a83).rw(FUNC(ustudio_state::lcd_r), FUNC(ustudio_state::lcd_w));

    // PCS6 connects to unused connector on audio-board - might be for debugging
    map(0x38b00, 0x38bff).rw(FUNC(ustudio_state::mem_r8_io<6>), FUNC(ustudio_state::mem_w8_io<6>));
}

void ustudio_state::mem_map_proto(address_map& map)
{
    // MCS0 128kb
    map(0x00000, 0x1ffff).rom().region("program0", 0);
    // MCS1 64kb
    map(0x20000, 0x2ffff).rom().region("program1", 0);
    // MCS2 32kb (4x8kb)
    map(0x30000, 0x37fff).ram();

    map(0x38000, 0x387ff).rw(FUNC(ustudio_state::meg_r), FUNC(ustudio_state::meg_w));

    pcs_map(map);

    map(0xf0000, 0xfffff).rom().region("mos", 0);  // 64kb
}

void ustudio_state::mem_map(address_map& map)
{
    // MCS0 64kb
    map(0x00000, 0x0ffff).rom().region("program0", 0);
    // MCS1 64kb
    map(0x20000, 0x2ffff).rom().region("program1", 0);
    // MCS2 32kb
    map(0x30000, 0x37fff).ram();

    map(0x38000, 0x387ff).rw(FUNC(ustudio_state::meg_r), FUNC(ustudio_state::meg_w));

    pcs_map(map);

    map(0xf0000, 0xfffff).rom().region("mos", 0);  // 64kb
}

void ustudio_state::lcd_map(address_map& map)
{
    map.global_mask(0x07ff);
    map(0x0000, 0x07ff).ram();
}

void ustudio_state::drvif_changed(ioport_field& field, u32 param, ioport_value oldval, ioport_value newval)
{
    if (m_drvif[param]->read())
    {
        m_maincpu->int2_w(1);
    }
    else
    {
        m_maincpu->int2_w(0);
    }
}

// clang-format off
#define DRVIF_PORT(num, sw1, sw2, sw3, sw4, sw5, sw6, sw7, sw8) \
    PORT_START("K" #num) \
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(sw1) PORT_CHANGED_MEMBER(DEVICE_SELF, ustudio_state, drvif_changed, num) \
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(sw2) PORT_CHANGED_MEMBER(DEVICE_SELF, ustudio_state, drvif_changed, num) \
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(sw3) PORT_CHANGED_MEMBER(DEVICE_SELF, ustudio_state, drvif_changed, num) \
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(sw4) PORT_CHANGED_MEMBER(DEVICE_SELF, ustudio_state, drvif_changed, num) \
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(sw5) PORT_CHANGED_MEMBER(DEVICE_SELF, ustudio_state, drvif_changed, num) \
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(sw6) PORT_CHANGED_MEMBER(DEVICE_SELF, ustudio_state, drvif_changed, num) \
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(sw7) PORT_CHANGED_MEMBER(DEVICE_SELF, ustudio_state, drvif_changed, num) \
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(sw8) PORT_CHANGED_MEMBER(DEVICE_SELF, ustudio_state, drvif_changed, num)

#define MATRIX_PORT(num, sw1, sw2, sw3, sw4, sw5, sw6, sw7, sw8)    \
    PORT_START("M" #num) \
    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw1) \
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw2) \
    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw3) \
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw4) \
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw5) \
    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw6) \
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw7) \
    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw8)

#define MATRIX_PORT2(num, sw1, sw2, sw3, sw4, sw5, sw6, sw7, sw8)    \
    PORT_START("M" #num) \
    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw1) \
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw2) \
    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw3) \
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw4) \
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(sw5) \
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(sw6) \
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(sw7) \
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(sw8)

static INPUT_PORTS_START(ustudio)
    DRVIF_PORT(0, "K0_0", "K0_1", "K0_2", "K0_3", "K0_4", "K0_5", "K0_6", "K0_7")
    DRVIF_PORT(1, "K1_0", "K1_1", "K1_2", "K1_3", "K1_4", "K1_5", "K1_6", "K1_7")
    DRVIF_PORT(2, "K2_0", "K2_1", "K2_2", "K2_3", "K2_4", "K2_5", "K2_6", "K2_7")
    DRVIF_PORT(3, "K3_0", "K3_1", "K3_2", "K3_3", "K3_4", "K3_5", "K3_6", "K3_7")
    DRVIF_PORT(4, "K4_0", "K4_1", "K4_2", "K4_3", "K4_4", "K4_5", "K4_6", "K4_7")
    DRVIF_PORT(5, "Erase", "B", "Record", "Mode", "Play", "Store", "Next", "Read")
    DRVIF_PORT(6, "C3", "C6", "C4", "C2", "A", "C5", "*", "C1")
    DRVIF_PORT(7, "m9", "m5", "m2", "m4", "m7", "m5", "m1", "m8")
    DRVIF_PORT(8, "Split", "m12", "Tremolo", "m6", "unused", "m11", "unused", "m10")

    MATRIX_PORT(0, "M0_0", "M0_1", "M0_2", "M0_3", "M0_4", "M0_5", "M0_6", "M0_7")
    MATRIX_PORT(1, "M1_0", "M1_1", "M1_2", "M1_3", "M1_4", "M1_5", "M1_6", "M1_7")
    MATRIX_PORT(2, "M2_0", "M2_1", "M2_2", "M2_3", "M2_4", "M2_5", "M2_6", "M2_7")
    MATRIX_PORT(3, "M3_0", "M3_1", "M3_2", "M3_3", "M3_4", "M3_5", "M3_6", "M3_7")
    MATRIX_PORT(4, "M4_0", "M4_1", "M4_2", "M4_3", "M4_4", "M4_5", "M4_6", "M4_7")
    MATRIX_PORT2(5, "M5_0", "M5_1", "M5_2", "M5_3", "M5_4", "Foot1", "Foot2", "Sustain")
INPUT_PORTS_END
// clang-format on

void ustudio_state::ustudio_common(machine_config& config)
{
    // NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

    // Midi interface
    ACIA6850(config, m_acia);
    clock_device& midiclock(CLOCK(config, "midiclock", 8_MHz_XTAL / 16));
    midiclock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
    midiclock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

    m_acia->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
    m_acia->irq_handler().set(FUNC(ustudio_state::acia_irq_w));

    midi_port_device& mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
    mdin.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
    MIDI_PORT(config, "mdout", midiout_slot, "midiout");

    // Display hardware
    HD61830(config, m_lcdc, 12_MHz_XTAL);
    m_lcdc->set_addrmap(0, &ustudio_state::lcd_map);
    m_lcdc->set_screen("screen");

    screen_device& screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
    screen.set_refresh_hz(60);
    screen.set_screen_update(m_lcdc, FUNC(hd61830_device::screen_update));
    screen.set_size(240, 64);

    // Somehow both roms I got have the displayed area appears shifted to the left
    // There are images (on audiofanzine.fr) where the display looks correct.
    // Need to further investigate - and hopefully fix the rom. For now this makes
    // it show the intended screen area:
    screen.set_visarea(0, 260 - 1, 0, 64 - 1);
    screen.set_palette("palette");
    PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

    config.set_default_layout(layout_ustudio);

    // Sound hardware
    SPEAKER(config, "mono").front_center();


    HONMEG(config, m_meg, 1_MHz_XTAL);
    m_meg->set_addrmap(0, &ustudio_state::meg_memory_map);

    for (int i = 0; i < 8; i++)
    {
        // TODO calculate filter values from decoupling cap / op-amp impedence
        FILTER_RC(config, m_meg_filter[i]);
        m_meg_filter[i]->add_route(0, "mono", 1.0);
        m_meg_filter[i]->set_rc(filter_rc_device::HIGHPASS, 2000, 0, 0, CAP_U(2));

        m_meg->add_route(i, m_meg_filter[i], 1.00);
    }

    // m_meg->add_route(0, "meg_filter", 1.00);
	HOHNERPCM(config, m_rhythm_pcm, 4_MHz_XTAL);
    m_rhythm_pcm->set_addrmap(0, &ustudio_state::pcm_memory_map);
	m_rhythm_pcm->add_route(0, "mono", 1.0);
	m_rhythm_pcm->add_route(1, "mono", 1.0);

}

void ustudio_state::ustudio_proto(machine_config& config)
{
    I80186(config, m_maincpu, 16_MHz_XTAL);
    m_maincpu->set_addrmap(AS_PROGRAM, &ustudio_state::mem_map_proto);
    ustudio_common(config);
}

void ustudio_state::ustudio(machine_config& config)
{
    I80186(config, m_maincpu, 16_MHz_XTAL);
    m_maincpu->set_addrmap(AS_PROGRAM, &ustudio_state::mem_map);
    ustudio_common(config);
}

void ustudio_state::machine_start()
{
    m_led_panels.resolve();
    m_led_beat.resolve();

}

ROM_START(ustudio_proto)
    ROM_REGION16_LE(0x10000, "mos", 0)
    ROM_LOAD16_BYTE("us30-1.bin", 0x00000, 0x8000, CRC(ce83c02a) SHA1(ff4fbf8861fd0305d9dccd0a72eaf09f0691dd73))
    ROM_LOAD16_BYTE("us30-6.bin", 0x00001, 0x8000, CRC(ea0b49b1) SHA1(3fa70c0221c156ef551d57cf69f25f9fbb63a1ed))

    ROM_REGION16_LE(0x20000, "program0", 0)
    ROM_LOAD16_BYTE("us30-3.bin", 0x00000, 0x10000, CRC(3581e237) SHA1(79cca8b188340300ca8a9335a21576365ba31567))
    ROM_LOAD16_BYTE("us30-4.bin", 0x00001, 0x10000, CRC(c3d78418) SHA1(a803bf918b499197aa23c3d989459941aa98138e))

    ROM_REGION16_LE(0x10000, "program1", 0)
    ROM_LOAD16_BYTE("us30-2.bin", 0x00000, 0x8000, CRC(209d5ff9) SHA1(ccc24b4391521f680a8e281da9a0e849e2e46192))
    ROM_LOAD16_BYTE("us30-5.bin", 0x00001, 0x8000, CRC(471bcbc3) SHA1(162643546a63aa0129802426e2682e6840157df9))

    ROM_REGION(0x1000, "wf5000", 0)
    ROM_LOAD("wf5000.bin", 0x0000, 0x1000, CRC(fb733373) SHA1(0afa383a131acb4aa8ea731d409ac4c5682194fa))

    // ROM_REGION(0x1000, "wf90", 0)
    // ROM_LOAD("wf90.bin", 0x0000, 0x1000, CRC(730cfa8e) SHA1(300650aa1dc822d1e09d71b4456f30285346f220))

    ROM_REGION(0x10000, "ry5000", 0)
    ROM_LOAD("ry5000-1.bin", 0x0000, 0x8000, CRC(9ec93b9a) SHA1(83c7950b0290f4dd2c2fea028e4351b1932a3045))
    ROM_LOAD("ry5000-2.bin", 0x8000, 0x8000, CRC(36b6d440) SHA1(6d025d95597ac041d1686bf70b266989aeac97c0))
ROM_END

ROM_START(ustudio)
    ROM_REGION16_LE(0x10000, "mos", 0)
    ROM_LOAD16_BYTE("us30-1.bin", 0x00000, 0x8000, CRC(b97674c6) SHA1(1deb97fac8e8bab3e3b21fff67d0b453d38d0884))
    ROM_LOAD16_BYTE("us30-6.bin", 0x00001, 0x8000, CRC(ad0fa64b) SHA1(fddacd8cf4df4f285b8ff53fd8c8b443417b0fff))

    ROM_REGION16_LE(0x10000, "program0", 0)
    ROM_LOAD16_BYTE("us30-3.bin", 0x00000, 0x8000, CRC(f531cd71) SHA1(868ce1450c47eba2fa671e08ae858cd3038fb0bc))
    ROM_LOAD16_BYTE("us30-4.bin", 0x00001, 0x8000, CRC(f88ad192) SHA1(a31c77c70491b8e89ecb3255668031f049f3b3b8))

    ROM_REGION16_LE(0x10000, "program1", 0)
    ROM_LOAD16_BYTE("us30-2.bin", 0x00000, 0x8000, CRC(76bd1717) SHA1(c8909ff1f4cb27d6b0948df5270fa0c23aaeb7dd))
    ROM_LOAD16_BYTE("us30-5.bin", 0x00001, 0x8000, CRC(02692298) SHA1(d2319d24796f76e929550d4ba69b95bd362dfb53))

    ROM_REGION(0x1000, "wf5000", 0)
    ROM_LOAD("wf5000.bin", 0x0000, 0x1000, CRC(fb733373) SHA1(0afa383a131acb4aa8ea731d409ac4c5682194fa))

    // ROM_REGION(0x1000, "wf90", 0)
    // ROM_LOAD("wf90.bin", 0x0000, 0x1000, CRC(730cfa8e) SHA1(300650aa1dc822d1e09d71b4456f30285346f220))

    ROM_REGION(0x10000, "ry5000", 0)
    ROM_LOAD("ry5000-1.bin", 0x0000, 0x8000, CRC(9ec93b9a) SHA1(83c7950b0290f4dd2c2fea028e4351b1932a3045))
    ROM_LOAD("ry5000-2.bin", 0x8000, 0x8000, CRC(36b6d440) SHA1(6d025d95597ac041d1686bf70b266989aeac97c0))
ROM_END

}

// Both ROMS state V1.1 but the hardware of the first looked more to be a prototype,
// with cables directly soldered to RCA ouput jacks, not on a PCB.
SYST(1987, ustudio_proto, 0, 0, ustudio_proto, ustudio, ustudio_state, empty_init, "Hohner", "uStudio30 v1.1 prototype", MACHINE_CLICKABLE_ARTWORK)
// | MACHINE_IMPERFECT_SOUND)

SYST(1987, ustudio, 0, 0, ustudio, ustudio, ustudio_state, empty_init, "Hohner", "uStudio30 v1.1", MACHINE_CLICKABLE_ARTWORK)
