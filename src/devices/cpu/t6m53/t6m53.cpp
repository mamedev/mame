// license:BSD-3-Clause
// copyright-holders:grubbyplaya
/***************************************************************************

        Toshiba T6M53 ASIC

***************************************************************************/

#include "emu.h"
#include "t6m53.h"
#include "t6m53_dasm.h"

DEFINE_DEVICE_TYPE(T6M53, t6m53_device, "t6m53", "Toshiba T6M53")

#define REG_A m_regs[0x0f0 >> 1]

t6m53_device::t6m53_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
    cpu_device(mconfig, T6M53, tag, owner, clock),
    m_program_config("program", ENDIANNESS_BIG, 8, 16, 0),
    m_ring_out(*this),
    m_tip_out(*this),
    m_ring_in(*this, 0),
    m_tip_in(*this, 0),
    m_stb(*this),
    m_btn_rows(*this, 0),
    m_on_btn(*this, 0)
{
}


device_memory_interface::space_config_vector t6m53_device::memory_space_config() const {
    return space_config_vector {
        std::make_pair(AS_PROGRAM, &m_program_config)
    };
}


void t6m53_device::device_start() {
    std::fill(std::begin(m_regs), std::end(m_regs), 0);
    std::fill(std::begin(m_stack), std::end(m_stack), 0);

    m_program = &space(AS_PROGRAM);

    state_add(T6M53_PC, "PC", m_pc).formatstr("%04X");
    state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%04X").noshow();
    state_add(STATE_GENPCBASE, "CURPC", m_previous_pc).formatstr("%04X").noshow();
    state_add(T6M53_A, "A", REG_A).formatstr("%02X");
    state_add(T6M53_SP, "SP", m_regs[0x118 >> 1]).formatstr("%01X");
    state_add(T6M53_REP, "REP", m_regs[0x110 >> 1]).formatstr("%01X");
    state_add(T6M53_IL, "IL", m_regs[0x100 >> 1]).formatstr("%02X");
    state_add(T6M53_IH, "IH", m_regs[0x102 >> 1]).formatstr("%02X");
    state_add(T6M53_DPL, "DPL", m_regs[0x11c >> 1]).formatstr("%02X");
    state_add(T6M53_DPH, "DPH", m_regs[0x11e >> 1]).formatstr("%02X");

    save_item(NAME(m_regs));
    save_item(NAME(m_pc));
    save_item(NAME(m_stack));
    save_item(NAME(m_lastlow));
    save_item(NAME(m_lasthigh));
    save_item(NAME(m_isel));

    save_item(NAME(m_ftimer));
    save_item(NAME(m_vtimer));
    set_icountptr(m_icount);
    
    set_clock_scale(105.0 / m_divisors[0]);
}


void t6m53_device::device_reset() {
    m_regs[0x10F >> 1] &= 0xB0;
    m_pc = 0;
    m_previous_pc = 0;
    m_lastlow = 0;
    m_lasthigh = 0;
    m_ftimer = 0;
    m_vtimer = 0;
    m_write_repeat = false;
    
    set_clock_scale(105.0 / m_divisors[0]);
}


void t6m53_device::state_import(const device_state_entry &entry) {
    switch (entry.index()) {
        case STATE_GENPC:
        case STATE_GENPCBASE:
        case T6M53_PC:
            break;
        default:
            break;
    }
}


void t6m53_device::state_export(const device_state_entry &entry) {
    switch (entry.index()) {
        case STATE_GENPC:
        case STATE_GENPCBASE:
        case T6M53_PC:
            break;
        default:
            break;
    }
}

std::unique_ptr<util::disasm_interface> t6m53_device::create_disassembler() {
    return std::make_unique<t6m53_disassembler>();
}

uint8_t t6m53_device::reg_r4_raw(uint16_t index, uint8_t &last) {
    if ((index & 0x0f0) == 0x0f0) {
        last = REG_A & 0x0f;
        return last;
    }

    switch (index) {
        case 0x103:
        case 0x10b:
        case 0x10c:
        case 0x10d:
        case 0x10f:
        case 0x110:
        case 0x111:
        case 0x112:
        case 0x113:
        case 0x119:
            return last;

        case 0x117:
            return (m_ring_in() << 1) | m_tip_in();

        default:
            last = BIT(index, 0) ? (m_regs[index >> 1] >> 4) : (m_regs[index >> 1] & 0x0f);
            if (index == 0x109)
                last |= m_on_btn() << 2;

            return last;
    }
}


void t6m53_device::reg_w4_raw(uint16_t index, uint8_t data) {
    data &= 0x0f;

    if ((index & 0x0f0) == 0x0f0) {
        REG_A = (REG_A & 0xf0) | data;
        return;
    }

    switch (index) {
        case 0x102:
            data &= 1;
            break;

        case 0x107:
            data &= 0x07;
            break;

        case 0x108:
        case 0x114:
        case 0x115:
            return;

        case 0x109:
            if ((data & 8) != (m_isel & 8)) {
                const uint8_t temp = (m_regs[0x101 >> 1] & 0xf0) | (m_regs[0x102 >> 1] & 1);

                m_regs[0x101 >> 1] = (m_regs[0x101 >> 1] & 0x0f) | (m_regs[0x0f2 >> 1] & 0xf0);
                m_regs[0x102 >> 1] = (m_regs[0x102 >> 1] & 0xf0) | (m_regs[0x0f2 >> 1] & 1);
                m_regs[0x0f2 >> 1] = temp;
                m_isel = data;
            }
            break;

        case 0x10d:
            set_clock_scale(105.0 / m_divisors[(data >> 1)]);
            break;

        case 0x10f:
            if ((data & 1) && !(m_regs[0x10F >> 1] & 0x10) && !(m_regs[0x10D >> 1] & 0x10))
                m_regs[0x11a >> 1] = m_regs[0x0f4 >> 1];

            if (data & 4) {
                reg_w8(0x11c, 0xaa);
                reg_w8(0x11e, 0xaa);
            }
            break;

        case 0x110:
            m_write_repeat = true;
            break;

        case 0x116:
            m_regs[0x116 >> 1] = (m_regs[0x116 >> 1] & 0xf0) | (data & 2);
            m_stb(!(data & 2));
            return;

        case 0x117:
            m_tip_out(BIT(data, 0));
            m_ring_out(BIT(data, 1));
            break;

        case 0x11a:
        case 0x11b:
            index = (index - 0x11a) + 0x0f4;
            break;
    }

    if (index & 1)
        m_regs[index >> 1] = (m_regs[index >> 1] & 0x0f) | (data << 4);
    else
        m_regs[index >> 1] = (m_regs[index >> 1] & 0xf0) | (data & 0x0f);
}


uint8_t t6m53_device::reg_r8(uint16_t index) {
    if ((index & 0x0f0) == 0x0f0) {
        m_lastlow = REG_A & 0x0f;
        m_lasthigh = REG_A >> 4;
        return REG_A;
    } else if ((index & 0x1e1) == 0x101) {
        return reg_r4_raw(index, m_lastlow) | (m_lasthigh << 4);
    }

    return reg_r4_raw(index, m_lastlow) | (reg_r4_raw((index & 0x1f0) | ((index + 1) & 0x0f), m_lasthigh) << 4);
}


void t6m53_device::reg_w8(uint16_t index, uint8_t data) {
    m_lastlow = data & 0x0f;
    m_lasthigh = data >> 4;

    if ((index & 0x0f0) == 0x0f0) {
        REG_A = data;
    } else if ((index & 0x1e1) == 0x101) {
        reg_w4(index, m_lastlow);
    } else {
        reg_w4(index, m_lastlow);
        reg_w4((index & 0x1f0) | ((index + 1) & 0x0f), m_lasthigh);
    }
}


inline uint16_t t6m53_device::get_i() const {
    return ((m_regs[0x102 >> 1] & 1) << 8) | m_regs[0x100 >> 1];
}


inline void t6m53_device::set_i(uint16_t value) {
    m_regs[0x100 >> 1] = value & 0xff;
    m_regs[0x102 >> 1] = (m_regs[0x102 >> 1] & 0xf0) | !!(value & 0x100);
}


inline uint16_t t6m53_device::get_dp() const {
    return (m_regs[0x11e >> 1] << 8) | m_regs[0x11c >> 1];
}


inline void t6m53_device::set_dp(uint16_t value) {
    m_regs[0x11c >> 1] = value & 0xff;
    m_regs[0x11e >> 1] = value >> 8;
}


inline uint16_t t6m53_device::add4(uint16_t x, uint8_t y) const {
    return (x & 0xfff0) | ((x + y) & 0x0f);
}


inline uint16_t t6m53_device::bcd(uint8_t x) {
    return (x >> 4) * 10 + (x & 0x0f);
}

uint8_t t6m53_device::adc(int bits, uint8_t x, uint8_t y, bool &carry, uint16_t op) {
    if (bits > 0) {
        if (op & 0x0800) {
            const int sum = bcd(x) + bcd(y) + carry;
            carry = sum > ((bits == 4) ? 9 : 99);

            if (bits == 4)
                return ((sum / 10) % 10) << 4 | (sum % 10);

            return ((sum / 100) << 8) | (((sum / 10) % 10) << 4) | (sum % 10);
        }

        const int sum = x + y + carry;
        carry = (sum >> bits) & 1;
        return sum;
    }

    if (op & 0x0800) {
        int sum = bcd(x) - bcd(y) - carry;
        carry = sum < 0;
        sum = (bits == -4) ? (sum + 10) % 10 : (sum + 100) % 100;
        return ((sum / 10) % 10) << 4 | (sum % 10);
    }

    const int sum = x - y - carry;
    carry = (sum >> -bits) & 1;
    return sum;
}


uint16_t t6m53_device::jyx(uint16_t op, uint16_t offset) const {
    const uint16_t YX = ((op & 0xf) << 4) | ((op >> 4) & 0xf);
    const uint16_t effective_offset = BIT(op, 8) ? offset : offset << 1;
    return add4(((get_i() & 0x100) ^ 0x100) | YX, effective_offset);
}


uint16_t t6m53_device::wyx(uint16_t op, uint16_t offset) const {
    const uint16_t YX = ((op & 0xf) << 4) | ((op >> 4) & 0xf);
    return ((op & 0x020f) == 0x020f) ? get_i() : add4(((op >> 1) & 0x100) | YX, offset);
}


uint16_t t6m53_device::wyxs(uint16_t op, uint16_t offset) const {
    return ((op & 0x020f) == 0x020f) ? get_i() : wyxs_no_i(op, offset);
}

uint16_t t6m53_device::wyxs_no_i(uint16_t op, uint16_t offset) const {
    const uint16_t YX = ((op & 0xf) << 4) | ((op >> 4) & 0xf);
    const uint16_t effective_offset = BIT(op, 8) ? offset : offset << 1;
    return add4(((op >> 1) & 0x100) | YX, effective_offset);
}

uint16_t t6m53_device::ef(uint16_t op, uint16_t offset) const {
    return add4(0x100 | ((op >> 7) & 0x20) | ((op >> 4) & 0x1f), offset);
}


void t6m53_device::jump_call(bool is_cond, bool condition) {
    const uint16_t addr = m_program->read_word(m_pc);

    if (is_cond && !condition)
        return;

    if (addr & 0x8000) {
        const uint8_t sp = reg_r4(0x118);
        if (sp < 8) m_stack[sp] = m_pc + 2;
        reg_w4(0x118, sp + 1);
    }

    m_pc = (addr & 0x7fff) << 1;
    add_cycles(2);
}

void t6m53_device::execute_op(uint16_t op, int &repeat, uint16_t offset, bool is_repeat, bool &carry) {
    const bool s = BIT(op, 8);
    const uint8_t s_mask = s ? 0x0f : 0xff;
    const uint8_t SB = s ? 4 : 8;
    const uint8_t B = (BIT(op, 10) << 1) | BIT(op, 8);

    const auto reg_rS = [&](uint16_t addr) -> uint8_t { return s ? reg_r4(addr) : reg_r8(addr); };
    const auto reg_wS = [&](uint16_t addr, uint8_t value) { if (s) reg_w4(addr, value); else reg_w8(addr, value); };
    const auto do_m_jump_call = [&](bool cond) {
        if (repeat)
            return;

        if (op & 0x0400) {
            if (BIT(op, 9) == cond)
                jump_call(true, true);
            else
                m_pc += 2;
        } else if (op & 0x0200) {
            jump_call(false, true);
        }
    };

    if (op < 0x0200) {
        const uint16_t DPsave = get_dp();

        if (op & 0x04) {
            if (!repeat)
                add_cycles(1);

            if (op & 0x02)
                m_program->write_byte(get_dp(), s ? reg_r4(get_i()) | (m_lasthigh << 4) : reg_r8(get_i()));
            else
                reg_wS(get_i(), m_program->read_byte(get_dp()));

            op & 0x01 ? set_dp(get_dp() - 1) : set_dp(get_dp() + 1);
            if (is_repeat) set_i(add4(get_i(), 2 - s));
        }

        if ((op & 0x18) == 0x18)
            reg_w8(0x0f0, reg_r8(0x0f0) >> (((op & 4) && DPsave < 0x4000) ? 2 : 1));

        if (op & 0x20)
            m_regs[0x114 >> 1] = m_btn_rows(m_regs[0x112 >> 1]);

        if (op & 0x40) {
            uint8_t sp = m_regs[0x118 >> 1] & 0x0f;
            if (sp > 0) {
                reg_w4_raw(0x118, --sp);
                m_pc = sp < 8 ? m_stack[sp] : 0x0000;
            }
        }

        if (op & 0x80) 
            m_pc = (m_regs[0x106 >> 1] << 9) | (m_regs[0x104 >> 1] << 1);
    } else if (op < 0x0400) {
        reg_wS(jyx(op, offset), reg_rS(get_i()));
        if (is_repeat) set_i(add4(get_i(), 2 - s));
    } else if (op < 0x0600) {
        reg_wS(get_i(), reg_rS(jyx(op, offset)));
        if (is_repeat) set_i(add4(get_i(), 2 - s));
    } else if (op < 0x0800) {
        const uint8_t temp = reg_rS(jyx(op, offset));
        reg_wS(jyx(op, offset), reg_rS(get_i()));
        reg_wS(get_i(), temp);
        if (is_repeat) set_i(add4(get_i(), 2 - s));
    } else if (op < 0x0c00) {
        reg_w4(0x102, op >> 8);
        reg_w8(0x100, op);
        if (op & 0x0200)
            jump_call(false, true);
    } else if (op < 0x0e00) {
        reg_wS(get_i(), op);
        set_i(add4(get_i(), 2 - s));
    } else if (op < 0x1000) {
        reg_w4(ef(op, offset), op);
    } else if (op < 0x1400) {
        reg_wS(0x0f0, reg_rS(wyxs_no_i(op, offset)));
    } else if (op < 0x1800) {
        const uint8_t temp = reg_rS(0x0f0);
        reg_wS(0x0f0, reg_rS(wyxs_no_i(op, offset)));
        reg_wS(wyxs_no_i(op, offset), temp);
    } else if (op < 0x1c00) {
        reg_wS(wyxs_no_i(op, offset), reg_rS(0x0f0));
    } else if (op < 0x1e00) {
        reg_wS(0x0f0, op);
    } else if (op < 0x2000) {
        reg_w4(ef(op, offset), op);
    } else if (op < 0x2800) {
        const uint16_t addr = wyx(op, offset);
        reg_w4(addr, reg_r4(addr) ^ (1 << B));
    } else if (op < 0x3000) {
        const uint16_t addr = wyx(op, offset);
        reg_w4(addr, reg_r4(addr) & ~(1 << B));
    } else if (op < 0x3800) {
        const uint16_t addr = wyx(op, offset);
        reg_w4(addr, reg_r4(addr) | (1 << B));
    } else if (op < 0x3c00) {
        const uint16_t addr = ((op & 0x020f) == 0x020f) ? get_i() : wyxs(op, offset);
        reg_wS(addr, ~reg_rS(addr));
        if (is_repeat && (op & 0x020f) == 0x020f)
            set_i(add4(get_i(), 2 - s));
    } else if (op < 0x3e00) {
        reg_wS(get_i(), reg_rS(get_i()) ^ reg_rS(jyx(op, offset)));
        if (is_repeat) set_i(add4(get_i(), 2 - s));
    } else if (op < 0x4000) {
        reg_wS(get_i(), reg_rS(get_i()) | reg_rS(jyx(op, offset)));
        if (is_repeat) set_i(add4(get_i(), 2 - s));
    } else if (op < 0x6000) {
        const uint8_t x = reg_r4(ef(op, offset));
        const uint8_t y = op & 0x0f;
        carry = (op & 0x0800) ? (x < y || (carry && x == y)) : (carry || x != y);
        if (!repeat)
            carry = !(op & 0x0800) ^ !carry;
        do_m_jump_call(carry);
    } else if (op < 0x7000) {
        const uint8_t x = reg_rS(get_i());
        const uint8_t y = op & s_mask;
        carry = (op & 0x0800) ? (x < y || (carry && x == y)) : (carry || x != y);
        if (!repeat)
            carry = !(op & 0x0800) ^ !carry;
        do_m_jump_call(carry);
        set_i(add4(get_i(), 2 - s));
    } else if (op < 0x8000) {
        const uint8_t x = reg_rS(0x0f0);
        const uint8_t y = op & s_mask;
        carry = (op & 0x0800) ? (x < y || (carry && x == y)) : (carry || x != y);
        if (!repeat)
            carry = !(op & 0x0800) ^ !carry;
        do_m_jump_call(carry);
    } else if (op < 0xa000) {
        reg_wS(get_i(), adc(op & 0x1000 ? -SB : SB, reg_rS(get_i()), reg_rS(jyx(op, offset)), carry, op));
        if (is_repeat) set_i(add4(get_i(), 2 - s));
        do_m_jump_call(carry);
    } else if (op < 0xb000) {
        bool test = true;

        do {
            test &= BIT(reg_r4(wyx(op, offset)), B) == BIT(op, 11);
        } while (repeat && repeat--);

        if (test)
            jump_call(true, true);
        else
            m_pc += 2;
    } else if (op < 0xc000) {
        const uint8_t x = reg_rS(get_i());
        const uint8_t y = reg_rS(jyx(op, offset));
        carry = (op & 0x0800) ? (x < y || (carry && x == y)) : (carry || x != y);
        if (!repeat)
            carry = !(op & 0x0800) ^ !carry;
        do_m_jump_call(carry);
        if (is_repeat) set_i(add4(get_i(), 2 - s));
    } else if (op < 0xe000) {
        const uint16_t addr = ef(op, offset);
        reg_w4(addr, adc(4, reg_r4(addr), op & 0x0f, carry, op));
        do_m_jump_call(carry);
    } else if (op < 0xf000) {
        reg_wS(get_i(), adc(SB, reg_rS(get_i()), op & s_mask, carry, op));
        if (is_repeat) set_i(add4(get_i(), 2 - s));
        do_m_jump_call(carry);
    } else {
        reg_wS(0x0f0, adc(SB, reg_rS(0x0f0), op & s_mask, carry, op));
        do_m_jump_call(carry);
    }
}

void t6m53_device::add_cycles(int cycles) {
    m_icount -= cycles * 4;

    if ((m_regs[0x10d >> 1] & 0x10) || (m_regs[0x10f >> 1] & 0xc0)) {
        m_vtimer = 0;
        return;
    }

    m_ftimer += cycles * 10;
    if (m_ftimer >= 262144) {
        m_ftimer -= 262144;
        m_regs[0x108 >> 1] = add4(m_regs[0x108 >> 1], 1);
    }

    if (m_regs[0x10f >> 1] & 0x10) {
        int speed = 40960 >> ((m_regs[0x10b >> 1] >> 2) & 0x0c);
        m_vtimer += cycles * speed;
        if (m_vtimer >= 262144) {
            m_vtimer -= 262144;
            if (--m_regs[0x11a >> 1] == 0xff) {
                m_regs[0x11a >> 1] = m_regs[0x0f4 >> 1];
                m_regs[0x109 >> 1] |= 0x20;
            }
        }
    } else {
        m_vtimer = 0;
    }
}

void t6m53_device::execute_run() {
    const bool call_debugger = debugger_enabled();

    while (m_icount > 0) {
        if (m_on_btn() && (m_regs[0x10e >> 1] & 0x0c) == 0x0c) {
                device_reset();
                continue;
        }

        if (m_regs[0x10f >> 1] & 0xc0) {
            add_cycles(8);
            continue;
        }

        const uint16_t pc_save = m_previous_pc = m_pc;
        if (call_debugger)
            debugger_instruction_hook(m_pc);

        const uint8_t rep = repeat_count();
        bool carry = false;
        add_cycles(1);

        for (int repeat = rep, offset = 0; repeat >= 0; --repeat, ++offset) {
            m_pc = pc_save;
            m_write_repeat = false;

            uint16_t op = m_program->read_word(m_pc);
            add_cycles(1);

            m_pc += 2;
            execute_op(op, repeat, offset, rep != 0, carry);

            if (m_write_repeat) {
                if (repeat) {
                    repeat = repeat_count();
                    if (!repeat)
                        repeat = 0x10;
                }
            } else {
                reg_w4_raw(0x110, repeat);
            }
        }
    }
}