// license:BSD-3-Clause
// copyright-holders:Jeremy English
/*********************************************************************

	Blaster for Scott's 8008 Supercomputer a.k.a. Master Blaster
	by Dr. Scott M. Baker

	Schematics:
	https://github.com/sbelectronics/8008-super

	ROM Source Code:
	https://github.com/sbelectronics/h8/tree/master/h8-8008

	Demo:
	https://youtu.be/wurKTPdPhrI?si=aerTbgHIFm_8YwU2

	Write up:
	https://www.smbaker.com/master-blaster-an-8008-supercomputer

	This computer is based on Jim Loos 8008-SBC:
	https://github.com/jim11662418/8008-SBC

*********************************************************************/

#include "emu.h"

#include "super8008_blaster.h"

#include "cpu/i8008/i8008.h"
#include "machine/clock.h"
#include "machine/ram.h"

namespace {

class super8008_blaster_device : public device_t, public device_super8008_card_interface
{
public:
	// construction/destruction
	super8008_blaster_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void ext_write( offs_t offset, uint8_t data) override;
	virtual uint8_t ext_read(offs_t offset) override;
	virtual void ext_int() override;
	virtual void ext_reset() override;
	virtual void ext_req() override;
	virtual void ext_take(int state) override;
	virtual uint8_t ext_run() override;

	void blaster_mem(address_map &map);
	void blaster_io(address_map &map);

	int handle_irq(device_t &device, int data);

private:

	uint8_t m_take_state;
	required_device<i8008_device> m_maincpu;
	memory_share_creator<uint8_t> m_ram;
	memory_view m_view;

	required_ioport m_ext_take;
	required_ioport m_ext_run;
	required_ioport m_ext_req;
	required_ioport m_ext_int;

	//TODO(jhe) I'm not sure how to dynamically bind these based on the
	//jumpers.  I'm binding all of them for now and setting values based on the
	//TAKE jumper.
	output_finder<8, 8> m_leds;


	void serial_leds(uint8_t data);
};

super8008_blaster_device::super8008_blaster_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SUPER8008_BLASTER, tag, owner, clock)
	, device_super8008_card_interface(mconfig, *this)
	, m_maincpu(*this, "blaster_cpu")
	, m_ram(*this, "ram", 0x4000, ENDIANNESS_BIG)
	, m_view(*this, "view")
	, m_ext_take(*this, "EXT_TAKE")
	, m_ext_run(*this, "EXT_RUN")
	, m_ext_req(*this, "EXT_REQ")
	, m_ext_int(*this, "EXT_INT")
	, m_leds(*this, "led%u_%u", 0U, 0U) //First index is the blaster number, second is the individual led
{
}

void super8008_blaster_device::device_start()
{
	m_leds.resolve();
	//save_item(NAME(m_leds));
	save_item(NAME(m_take_state));
}

void super8008_blaster_device::ext_int()
{
	m_maincpu->set_input_line(0, HOLD_LINE);
}

void super8008_blaster_device::device_reset()
{
	//Power-on-reset triggers the interrupt
	ext_int();
}

void super8008_blaster_device::device_post_load()
{
	//Power-on-reset triggers the interrupt
	ext_int();
}


//The /Take signal controls which buffer on blaster has access to the 16kb of
//ram.  If an internal read happens, while blaster is taken, the local 8008
//will get back 0 since the data lines will be tied low.  This will HALT the
//internal CPU.

void super8008_blaster_device::blaster_mem(address_map &map)
{
	map(0x0000, 0x3fff).view(m_view);
	m_view[0](0x0000, 0x3fff).ram().share("ram");
	m_view[1](0x0000, 0x3fff).noprw();
}

IRQ_CALLBACK_MEMBER(super8008_blaster_device::handle_irq)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0xc0;//return a NOP
}

void super8008_blaster_device::device_add_mconfig(machine_config &config)
{

	I8008(config, m_maincpu, XTAL(1'000'000)/2);

	m_maincpu->set_addrmap(AS_PROGRAM, &super8008_blaster_device::blaster_mem);
	m_maincpu->set_irq_acknowledge_callback(FUNC(super8008_blaster_device::handle_irq));
	m_maincpu->set_addrmap(AS_IO, &super8008_blaster_device::blaster_io);
}

static INPUT_PORTS_START( super8008_blaster_jumpers )
	PORT_START("EXT_TAKE")
	PORT_CONFNAME( 0xff, 0, "External Take" )
	PORT_CONFSETTING( 0, "0" )
	PORT_CONFSETTING( 1, "1" )
	PORT_CONFSETTING( 2, "2" )
	PORT_CONFSETTING( 3, "3" )
	PORT_CONFSETTING( 4, "4" )
	PORT_CONFSETTING( 5, "5" )
	PORT_CONFSETTING( 6, "6" )
	PORT_START("EXT_RUN")
	PORT_CONFNAME( 0xff, 0, "External Run" )
	PORT_CONFSETTING( 0, "0" )
	PORT_CONFSETTING( 1, "1" )
	PORT_CONFSETTING( 2, "2" )
	PORT_CONFSETTING( 3, "3" )
	PORT_CONFSETTING( 4, "4" )
	PORT_CONFSETTING( 5, "5" )
	PORT_CONFSETTING( 6, "6" )
	PORT_START("EXT_REQ")
	PORT_CONFNAME( 0xff, 0, "External Request" )
	PORT_CONFSETTING( 0, "0" )
	PORT_CONFSETTING( 1, "1" )
	PORT_CONFSETTING( 2, "2" )
	PORT_CONFSETTING( 3, "3" )
	PORT_CONFSETTING( 4, "4" )
	PORT_CONFSETTING( 5, "5" )
	PORT_CONFSETTING( 6, "6" )
	PORT_START("EXT_INT")
	PORT_CONFNAME( 0xff, 0, "External Interrupt" )
	PORT_CONFSETTING( 0, "0" )
	PORT_CONFSETTING( 1, "1" )
	PORT_CONFSETTING( 2, "2" )
	PORT_CONFSETTING( 3, "3" )
	PORT_CONFSETTING( 4, "4" )
	PORT_CONFSETTING( 5, "5" )
	PORT_CONFSETTING( 6, "6" )
INPUT_PORTS_END

ioport_constructor super8008_blaster_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( super8008_blaster_jumpers );
}

// The /takew signal can be setup based on a jumper on blaster.  The jumper allows
// for either the ext_take7 line or LED7 line to set /takew.
//
// From the blog post:
//  "For example, I can set TAKE=1 and assert /TAKEW. This will cause a read to
//   come from Blaster#1, but a write to go out to all Blasters."
//
// TODO: Add a configurable jumper.  I don't need it now since the demo code is written
//       for the jumper to be in the EXT_TAKE7 configuration.

void super8008_blaster_device::ext_write(offs_t offset, uint8_t data)
{
	int jumper = m_ext_take->read();

	if (!BIT(m_take_state, jumper) || !BIT(m_take_state, 7))
	{
		if (0 <= offset && offset < 0x4000)
		{
			m_ram[offset] = data;
		}
	}
}

uint8_t super8008_blaster_device::ext_read(offs_t offset)
{
	int jumper = m_ext_take->read();
	//Selected when the ext_take line is low
	if (!BIT(m_take_state, jumper) && 0 <= offset && offset < 0x4000)
	{
		return m_ram[offset];
	}
	return 0;
}

//From the IO PLD
// /* memory and I/O strobes */
// MEMRD = (t3 & (pci # pcr) & SYNC) # (TAKEN & EXT_RD);
// MEMWR = (t3 & pcw & !SYNC & PHASE2) # (TAKEN & EXT_WR);
// TAKEN = (EXT_CS & EXT_TAKEOVER) # (EXT_CS & TAKEW);

void super8008_blaster_device::ext_take(int state)
{
	int jumper = m_ext_take->read();
	m_take_state = state;
	if (!BIT(m_take_state, jumper))
	{
		m_view.select(1);
	}
	else
	{
		m_view.select(0);
	}
}

void super8008_blaster_device::serial_leds(uint8_t data)
{
	int id = m_ext_take->read();
	if (0 <= id && id < 8) {
		for(int i = 0; i < 8; i++){
			m_leds[id][i] = BIT(data, 7-i);
		}
	}
}

uint8_t super8008_blaster_device::ext_run()
{
	//This is shifted by the ext_run jumper
	int id = m_ext_run->read();
	return m_maincpu->halt_r() << id;
}

void super8008_blaster_device::blaster_io(address_map &map)
{
	// Description of IO ports from monitor.asm
	//
	// serial I/O at 2400 bps N-8-1
	//
	// INPORT      equ 0           ; serial input port address
	// OUTPORT     equ 08H         ; serial output port address
	//
	// out 10                      ; clear the EPROM bank switch address outputs A13 and A14
	// out 09                      ; turn off orange LEDs
	// out 08                      ; set serial output high (mark)
	// in 1                        ; reset the bootstrap flip-flop internal to GAL22V10 #2


	map.global_mask(0xff);  // use 8-bit ports
	map.unmap_value_high(); // unmapped addresses return 0xff
	map(0x08, 0x08).w(FUNC(super8008_blaster_device::serial_leds));
}

void super8008_blaster_device::ext_reset()
{
	m_maincpu->reset();
}

void super8008_blaster_device::ext_req()
{

}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(SUPER8008_BLASTER, device_super8008_card_interface, super8008_blaster_device, "super8008_blaster", "Blaster Card")