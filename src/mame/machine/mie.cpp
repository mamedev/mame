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


const device_type MIE = &device_creator<mie_device>;
const device_type MIE_JVS = &device_creator<mie_jvs_device>;

static ADDRESS_MAP_START( mie_map, AS_PROGRAM, 8, mie_device)
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x6fff) AM_READ(read_ff)
	AM_RANGE(0x7000, 0x7002) AM_READWRITE(control_r, control_w) AM_MIRROR(0x07c0)
	AM_RANGE(0x7003, 0x7003) AM_READWRITE(lreg_r, lreg_w) AM_MIRROR(0x07c0)
	AM_RANGE(0x7004, 0x7023) AM_READWRITE(tbuf_r, tbuf_w) AM_MIRROR(0x07c0)
	AM_RANGE(0x7024, 0x703f) AM_READ(read_00) AM_MIRROR(0x07c0)
	AM_RANGE(0x7800, 0x7fff) AM_READ(read_78xx)
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mie_port, AS_IO, 8, mie_device)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x07) AM_READWRITE(gpio_r, gpio_w)
	AM_RANGE(0x08, 0x08) AM_READWRITE(gpiodir_r, gpiodir_w)
	AM_RANGE(0x0f, 0x0f) AM_READWRITE(adc_r, adc_w)
	AM_RANGE(0x10, 0x10) AM_READWRITE(jvs_r, jvs_w)     // ports 1x and 2x is standard UARTs, TODO handle it properly
	AM_RANGE(0x12, 0x12) AM_WRITE(jvs_dest_w)
	AM_RANGE(0x13, 0x13) AM_WRITE(jvs_lcr_w)
	AM_RANGE(0x15, 0x15) AM_READ(jvs_status_r)
	AM_RANGE(0x30, 0x30) AM_READWRITE(irq_enable_r, irq_enable_w)
	AM_RANGE(0x50, 0x50) AM_READWRITE(maple_irqlevel_r, maple_irqlevel_w)
	AM_RANGE(0x70, 0x70) AM_READWRITE(irq_pending_r, irq_pending_w)
	AM_RANGE(0x90, 0x90) AM_WRITE(jvs_control_w)
	AM_RANGE(0x91, 0x91) AM_READ(jvs_sense_r)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( mie )
	MCFG_CPU_ADD("mie", Z80, DERIVED_CLOCK(1,1))
	MCFG_CPU_PROGRAM_MAP(mie_map)
	MCFG_CPU_IO_MAP(mie_port)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(DEVICE_SELF, mie_device,irq_callback)
MACHINE_CONFIG_END

ROM_START( mie )
	ROM_REGION( 0x800, "mie", 0 )
	ROM_LOAD( "315-6146.bin", 0x000, 0x800, CRC(9b197e35) SHA1(864d14d58732dd4e2ee538ccc71fa8df7013ba06))
ROM_END

void mie_device::static_set_gpio_name(device_t &device, int entry, const char *name)
{
	mie_device &mie = downcast<mie_device &>(device);
	mie.gpio_name[entry] = name;
}

void mie_device::static_set_jvs_name(device_t &device, const char *name)
{
	mie_device &mie = downcast<mie_device &>(device);
	mie.jvs_name = name;
}

const rom_entry *mie_device::device_rom_region() const
{
	return ROM_NAME(mie);
}

machine_config_constructor mie_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(mie);
}

mie_jvs_device::mie_jvs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: jvs_host(mconfig, MIE_JVS, "JVS (MIE)", tag, owner, clock, "mie_jvs", __FILE__)
{
}

mie_device::mie_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: maple_device(mconfig, MIE, "Sega 315-6146 MIE", tag, owner, clock, "mie", __FILE__)
{
	memset(gpio_name, 0, sizeof(gpio_name));
	jvs_name = 0;
	cpu = 0;
	jvs = 0;
}

void mie_device::device_start()
{
	maple_device::device_start();
	cpu = subdevice<z80_device>("mie");
	timer = timer_alloc(0);
	jvs = machine().device<mie_jvs_device>(jvs_name);

	for (int i = 0; i < ARRAY_LENGTH(gpio_name); i++)
	{
		gpio_port[i] = gpio_name[i] ? ioport(gpio_name[i]) : NULL;
	}

	save_item(NAME(gpiodir));
	save_item(NAME(gpio_val));
	save_item(NAME(irq_enable));
	save_item(NAME(irq_pending));
	save_item(NAME(maple_irqlevel));

	// patch out MIE RAM test
	// TODO: figure out why SH4 code doesn't wait long enough for internal firmware's RAM test completed in the case of reset
	UINT32 *rom = (UINT32*)memregion("mie")->base();
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

READ8_MEMBER(mie_device::control_r)
{
	return control >> (8*offset);
}

WRITE8_MEMBER(mie_device::control_w)
{
	UINT32 prev_control = control;
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

void mie_device::device_timer(emu_timer &_timer, device_timer_id id, int param, void *ptr)
{
	timer->adjust(attotime::never);
	if(control & CTRL_RXB) {
		control &= ~CTRL_RXB;
		control |= CTRL_RFB;
		raise_irq(maple_irqlevel);
	}
	if(control & (CTRL_TXB|CTRL_CTXB)) {
		reply_ready();
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

void mie_device::maple_w(const UINT32 *data, UINT32 in_size)
{
	memcpy(tbuf, data, in_size*4);
	lreg = in_size-1;
	// currently not known how/then CTRL_HRES is cleared after reset, lets clear it at packet receive
	control &= ~(CTRL_HRES|CTRL_TXB|CTRL_TFB|CTRL_RFB|CTRL_BFOV);
	control |= CTRL_RXB;

	timer->adjust(attotime::from_usec(20));
}

READ8_MEMBER(mie_device::read_ff)
{
	return 0xff;
}

READ8_MEMBER(mie_device::read_00)
{
	return 0x00;
}

READ8_MEMBER(mie_device::read_78xx)
{
	// Internal rom tests (7800) & 80 and jumps to 8010 if non-zero
	// What we return is what a memdump sees on a naomi2 board
	return offset & 4 ? 0xff : 0x00;
}

READ8_MEMBER(mie_device::gpio_r)
{
	if(gpiodir & (1 << offset))
		return gpio_port[offset] ? gpio_port[offset]->read() : 0xff;
	else
		return gpio_val[offset];
}

WRITE8_MEMBER(mie_device::gpio_w)
{
	gpio_val[offset] = data;
	if(!(gpiodir & (1 << offset)) && gpio_port[offset])
		gpio_port[offset]->write(data, 0xff);
}

READ8_MEMBER(mie_device::gpiodir_r)
{
	return gpiodir;
}

WRITE8_MEMBER(mie_device::gpiodir_w)
{
	gpiodir = data;
}

READ8_MEMBER(mie_device::adc_r)
{
	return 0;
}

WRITE8_MEMBER(mie_device::adc_w)
{
}

READ8_MEMBER(mie_device::irq_enable_r)
{
	return irq_enable;
}

WRITE8_MEMBER(mie_device::irq_enable_w)
{
	irq_enable = data;
	recalc_irq();
}

READ8_MEMBER(mie_device::maple_irqlevel_r)
{
	return maple_irqlevel;
}

WRITE8_MEMBER(mie_device::maple_irqlevel_w)
{
	maple_irqlevel = data;
}

READ8_MEMBER(mie_device::irq_pending_r)
{
	return irq_pending;
}

WRITE8_MEMBER(mie_device::irq_pending_w)
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

READ8_MEMBER(mie_device::tbuf_r)
{
	return tbuf[offset >> 2] >> (8*(offset & 3));
}

WRITE8_MEMBER(mie_device::tbuf_w)
{
	int shift = (offset & 3)*8;
	tbuf[offset >> 2] = (tbuf[offset >> 2] & ~(255 << shift)) | (data << shift);
}

READ8_MEMBER(mie_device::lreg_r)
{
	return lreg;
}

WRITE8_MEMBER(mie_device::lreg_w)
{
	lreg = data;
}

READ8_MEMBER(mie_device::jvs_r)
{
	if (jvs_lcr & 0x80)
		return 0;

	const UINT8 *buf;
	UINT32 size;
	jvs->get_encoded_reply(buf, size);
	if(jvs_rpos >= size)
		return 0;
	return buf[jvs_rpos++];
}

WRITE8_MEMBER(mie_device::jvs_w)
{
	if (jvs_lcr & 0x80)
		return;

	jvs->push(data);
}

WRITE8_MEMBER(mie_device::jvs_dest_w)
{
	jvs_dest = data;
}

READ8_MEMBER(mie_device::jvs_status_r)
{
	// 01 = ready for reading
	// 20 = ready for writing
	// 40 = sending done
	const UINT8 *buf;
	UINT32 size;
	jvs->get_encoded_reply(buf, size);
	return 0x60 | (jvs_rpos < size ? 1 : 0);
}

WRITE8_MEMBER(mie_device::jvs_control_w)
{
	if((jvs_control & 1) && !(data & 1)) {
		jvs->commit_encoded();
		jvs_rpos = 0;
	}
	jvs_control = data;
}

WRITE8_MEMBER(mie_device::jvs_lcr_w)
{
	jvs_lcr = data;
}

READ8_MEMBER(mie_device::jvs_sense_r)
{
	return 0x8c | (jvs->get_address_set_line() ? 2 : 0) | (jvs->get_presence_line() ? 0 : 1);
}

void mie_device::maple_reset()
{
	control &= ~(CTRL_RXB|CTRL_TXB|CTRL_TFB|CTRL_RFB|CTRL_BFOV);
	control |= CTRL_HRES;

	timer->adjust(attotime::from_usec(20));
}
