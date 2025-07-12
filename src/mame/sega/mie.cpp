// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "mie.h"
#include "maple-dc.h"

// MIE aka Sega 315-6146, MAPLE-JVS bridge Z80-based MCU
//
// Todos:
// - ports 00-0f is identical to Sega 315-5338A/315-5649 I/O ICs
//   (used in Sega H1, Model 2, etc). devicefication needed.
//
// - ports 10-15 and 20-25 is standard 8250/16xxx UARTs.
//
// - ports a0-af - external I/O 1, in JVS I/Os connected to NEC uPD71053 counter/timer
//
// - ports c0-cf - external I/O 2, not used
//   in JVS I/Os can be connected to (unpopulated) 315-5296 I/O IC, if enabled by DIP switch
//
// - both memory and I/O address spaces can be directly accessed by host system (used in Hikaru)
//
// - Speed is all wrong


DEFINE_DEVICE_TYPE(MIE,     mie_device,     "mie",     "Sega 315-6146 MIE")
DEFINE_DEVICE_TYPE(MIE_JVS, mie_jvs_device, "mie_jvs", "JVS (MIE)")

void mie_device::mie_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0x6fff).r(FUNC(mie_device::read_ff));
	map(0x7000, 0x7002).rw(FUNC(mie_device::control_r), FUNC(mie_device::control_w)).mirror(0x07c0);
	map(0x7003, 0x7003).rw(FUNC(mie_device::lreg_r), FUNC(mie_device::lreg_w)).mirror(0x07c0);
	map(0x7004, 0x7023).rw(FUNC(mie_device::tbuf_r), FUNC(mie_device::tbuf_w)).mirror(0x07c0);
	map(0x7024, 0x703f).r(FUNC(mie_device::read_00)).mirror(0x07c0);
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).ram();
}

void mie_device::mie_port(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).rw(FUNC(mie_device::gpio_r), FUNC(mie_device::gpio_w));
	map(0x08, 0x08).rw(FUNC(mie_device::gpiodir_r), FUNC(mie_device::gpiodir_w));
	map(0x0f, 0x0f).rw(FUNC(mie_device::adc_r), FUNC(mie_device::adc_w));
	map(0x10, 0x10).rw(FUNC(mie_device::jvs_r), FUNC(mie_device::jvs_w));     // ports 1x and 2x is standard UARTs, TODO handle it properly
	map(0x12, 0x12).w(FUNC(mie_device::jvs_dest_w));
	map(0x13, 0x13).w(FUNC(mie_device::jvs_lcr_w));
	map(0x15, 0x15).r(FUNC(mie_device::jvs_status_r));
	map(0x30, 0x30).rw(FUNC(mie_device::irq_enable_r), FUNC(mie_device::irq_enable_w));
	map(0x50, 0x50).rw(FUNC(mie_device::maple_irqlevel_r), FUNC(mie_device::maple_irqlevel_w));
	map(0x70, 0x70).rw(FUNC(mie_device::irq_pending_r), FUNC(mie_device::irq_pending_w));
	map(0x90, 0x90).w(FUNC(mie_device::jvs_control_w));
	map(0x91, 0x91).r(FUNC(mie_device::jvs_sense_r));
}

ROM_START( mie )
	ROM_REGION( 0x800, "mie", 0 )
	ROM_LOAD( "315-6146.bin", 0x000, 0x800, CRC(9b197e35) SHA1(864d14d58732dd4e2ee538ccc71fa8df7013ba06))
ROM_END

const tiny_rom_entry *mie_device::device_rom_region() const
{
	return ROM_NAME(mie);
}

void mie_device::device_add_mconfig(machine_config &config)
{
	Z80(config, cpu, DERIVED_CLOCK(1,1));
	cpu->set_addrmap(AS_PROGRAM, &mie_device::mie_map);
	cpu->set_addrmap(AS_IO, &mie_device::mie_port);
	cpu->set_irq_acknowledge_callback(FUNC(mie_device::irq_callback));
}

mie_jvs_device::mie_jvs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: jvs_host(mconfig, MIE_JVS, tag, owner, clock)
{
}

mie_device::mie_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: maple_device(mconfig, MIE, tag, owner, clock)
	, cpu(*this, "mie")
	, jvs(*this, finder_base::DUMMY_TAG)
	, gpio_port(*this, {finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG, finder_base::DUMMY_TAG})
{
}

void mie_device::device_start()
{
	maple_device::device_start();
	timer = timer_alloc(FUNC(mie_device::update_irq_reply), this);
	m_space = &cpu->space(AS_PROGRAM);

	save_item(NAME(gpiodir));
	save_item(NAME(gpio_val));
	save_item(NAME(irq_enable));
	save_item(NAME(irq_pending));
	save_item(NAME(maple_irqlevel));

	// patch out MIE RAM test
	// TODO: figure out why SH4 code doesn't wait long enough for internal firmware's RAM test completed in the case of reset
	uint32_t *rom = (uint32_t*)memregion("mie")->base();
	rom[0x144/4] = 0x0001d8c3;
}

void mie_device::device_reset()
{
	lreg = 0;
	control = 0;
	jvs_control = 0;
	jvs_dest = 0;
	jvs_rpos = 0;
	memset(gpio_val, 0, sizeof(gpio_val));
	gpiodir = 0xff;
	maple_irqlevel = 0xff;
	irq_enable = irq_pending = 0;
	cpu->reset();
	timer->adjust(attotime::never);
	memset(tbuf, 0, sizeof(tbuf));
}

uint8_t mie_device::control_r(offs_t offset)
{
	return control >> (8*offset);
}

void mie_device::control_w(offs_t offset, uint8_t data)
{
	uint32_t prev_control = control;
	int shift = offset*8;
	control = (control & ~(255 << shift)) | (data << shift);

	if((!(prev_control & CTRL_TXB) && (control & CTRL_TXB)) ||
		(!(prev_control & CTRL_CTXB) && (control & CTRL_CTXB))) {
		control &= ~(CTRL_TFB|CTRL_RXB|CTRL_RFB|CTRL_BFOV|CTRL_EMP);
		reply_size = lreg+1;
		if(reply_size > TBUF_SIZE)
			reply_size = TBUF_SIZE;
		memcpy(reply_buffer, tbuf, reply_size*4);
		reply_partial = !(control & CTRL_ENDP);

		timer->adjust(attotime::from_usec(20));
	}
}

TIMER_CALLBACK_MEMBER(mie_device::update_irq_reply)
{
	if(control & CTRL_RXB) {
		control &= ~CTRL_RXB;
		control |= CTRL_RFB;
		raise_irq(maple_irqlevel);
	}
	if(control & (CTRL_TXB|CTRL_CTXB)) {
		reply_ready(0);
		lreg -= reply_size;
		if(reply_partial) {
			control &= ~CTRL_CTXB;
			control |= CTRL_EMP;
		} else {
			control &= ~(CTRL_TXB|CTRL_CTXB);
			control |= CTRL_TFB|CTRL_EMP;
		}
	}
	if(control & CTRL_HRES) {
		raise_irq(maple_irqlevel);
	}
}

void mie_device::maple_w(const uint32_t *data, uint32_t in_size)
{
	memcpy(tbuf, data, in_size*4);
	lreg = in_size-1;
	// currently not known how/then CTRL_HRES is cleared after reset, lets clear it at packet receive
	control &= ~(CTRL_HRES|CTRL_TXB|CTRL_TFB|CTRL_RFB|CTRL_BFOV);
	control |= CTRL_RXB;

	timer->adjust(attotime::from_usec(20));
}

uint8_t mie_device::read_ff()
{
	return 0xff;
}

uint8_t mie_device::read_00()
{
	return 0x00;
}

//uint8_t mie_device::read_78xx(offs_t offset)
//{
	// Internal rom tests (7800) & 80 and jumps to 8010 if non-zero
	// What we return is what a memdump sees on a naomi2 board
//  return offset & 4 ? 0xff : 0x00;
//}

uint8_t mie_device::gpio_r(offs_t offset)
{
	if(gpiodir & (1 << offset))
		return gpio_port[offset] ? gpio_port[offset]->read() : 0xff;
	else
		return gpio_val[offset];
}

void mie_device::gpio_w(offs_t offset, uint8_t data)
{
	gpio_val[offset] = data;
	if(!(gpiodir & (1 << offset)) && gpio_port[offset])
		gpio_port[offset]->write(data, 0xff);
}

uint8_t mie_device::gpiodir_r()
{
	return gpiodir;
}

void mie_device::gpiodir_w(uint8_t data)
{
	// refresh output ports if direction changed
	u8 r_to_w = (data ^ gpiodir) & ~data;
	gpiodir = data;
	for (unsigned i = 0; i < 7; i++)
		if (BIT(r_to_w, i) && gpio_port[i])
			gpio_port[i]->write(gpio_val[i], 0xff);
}

uint8_t mie_device::adc_r()
{
	return 0;
}

void mie_device::adc_w(uint8_t data)
{
}

uint8_t mie_device::irq_enable_r()
{
	return irq_enable;
}

void mie_device::irq_enable_w(uint8_t data)
{
	irq_enable = data;
	recalc_irq();
}

uint8_t mie_device::maple_irqlevel_r()
{
	return maple_irqlevel;
}

void mie_device::maple_irqlevel_w(uint8_t data)
{
	maple_irqlevel = data;
}

uint8_t mie_device::irq_pending_r()
{
	return irq_pending;
}

void mie_device::irq_pending_w(uint8_t data)
{
	irq_pending = data;
	recalc_irq();
}

void mie_device::recalc_irq()
{
	cpu->set_input_line(0, irq_enable & irq_pending & 0x7f ? ASSERT_LINE : CLEAR_LINE);
}

IRQ_CALLBACK_MEMBER(mie_device::irq_callback)
{
	if(!(irq_enable & irq_pending & 0x7f))
		throw emu_fatalerror("MIE irq callback called with enable=%02x, pending=%02x", irq_enable, irq_pending);

	int level = -1;
	for(level = 0; level < 7; level++)
		if((irq_enable & irq_pending) & (1 << level))
			break;

	irq_pending &= ~(1 << level);
	recalc_irq();
	return 0xf2+2*level;
}

void mie_device::raise_irq(int level)
{
	if(level>=0 && level<7) {
		irq_pending |= 1<<level;
		recalc_irq();
	}
}

uint8_t mie_device::tbuf_r(offs_t offset)
{
	return tbuf[offset >> 2] >> (8*(offset & 3));
}

void mie_device::tbuf_w(offs_t offset, uint8_t data)
{
	int shift = (offset & 3)*8;
	tbuf[offset >> 2] = (tbuf[offset >> 2] & ~(255 << shift)) | (data << shift);
}

uint8_t mie_device::lreg_r()
{
	return lreg;
}

void mie_device::lreg_w(uint8_t data)
{
	lreg = data;
}

uint8_t mie_device::jvs_r()
{
	if(jvs_lcr & 0x80)
		return 0;

	const uint8_t *buf;
	uint32_t size;
	jvs->get_encoded_reply(buf, size);
	if(jvs_rpos >= size)
		return 0;
	return buf[jvs_rpos++];
}

void mie_device::jvs_w(uint8_t data)
{
	if(jvs_lcr & 0x80)
		return;

	jvs->push(data);
}

void mie_device::jvs_dest_w(uint8_t data)
{
	jvs_dest = data;
}

uint8_t mie_device::jvs_status_r()
{
	// 01 = ready for reading
	// 20 = ready for writing
	// 40 = sending done
	const uint8_t *buf;
	uint32_t size;
	jvs->get_encoded_reply(buf, size);
	return 0x60 | (jvs_rpos < size ? 1 : 0);
}

void mie_device::jvs_control_w(uint8_t data)
{
	if((jvs_control & 1) && !(data & 1)) {
		jvs->commit_encoded();
		jvs_rpos = 0;
	}
	jvs_control = data;
}

void mie_device::jvs_lcr_w(uint8_t data)
{
	jvs_lcr = data;
}

uint8_t mie_device::jvs_sense_r()
{
	return 0x8c | (jvs->get_address_set_line() ? 2 : 0) | (jvs->get_presence_line() ? 0 : 1);
}

void mie_device::maple_reset()
{
	control &= ~(CTRL_RXB|CTRL_TXB|CTRL_TFB|CTRL_RFB|CTRL_BFOV);
	control |= CTRL_HRES;

	timer->adjust(attotime::from_usec(20));
}
