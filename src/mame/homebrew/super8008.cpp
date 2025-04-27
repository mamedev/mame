// license:BSD-3-Clause copyright-holders:Jeremy English
//
// Scott's 8008 Supercomputer a.k.a. Master Blaster
// by Dr. Scott M. Baker
//
// Schematics:
// https://github.com/sbelectronics/8008-super
//
// ROM Source Code:
// https://github.com/sbelectronics/h8/tree/master/h8-8008
//
// Demo:
// https://youtu.be/wurKTPdPhrI?si=aerTbgHIFm_8YwU2
//
// Write up:
// https://www.smbaker.com/master-blaster-an-8008-supercomputer
//
// MAME driver for Jim Loos 8008-SBC
// https://github.com/jim11662418/8008-SBC
//
//
// This computer is based on Jim Loos 8008-SBC:
// https://github.com/jim11662418/8008-SBC
//
// The Mame driver for the 8008-SBC is named sbc8008.

#include "emu.h"

#include "bus/super8008/super8008.h"
#include "bus/rs232/rs232.h"
#include "cpu/i8008/i8008.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/ram.h"

#include "super8008.lh"


namespace
{

const uint8_t NUM_BLASTERS = 7;
const uint8_t MMAP_MASK = 3;

#define SUPER8008_BUS_TAG        "master_blaster"

struct memory_map_info
{
	uint8_t mmap_index;
	uint8_t mmap_value;
	uint8_t ra12;
	uint8_t ra13;
	uint8_t ext_cs;
	uint8_t rom_cs;
	uint16_t address;
};



// State class - derives from driver_device
class super8008_state : public driver_device
{
public:
	super8008_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_rom(*this, "rom")
		, m_rs232(*this, "rs232")
		, m_leds(*this, "led%u", 0U)
		, m_run_led(*this, "run_led")
		, m_uart(*this, "uart")
		, m_bus(*this, SUPER8008_BUS_TAG)
		, m_slots(*this, SUPER8008_BUS_TAG ":%u", 1U)
		, m_reg1_view(*this, "reg1_view")
		, m_reg2_view(*this, "reg2_view")
		, m_reg3_view(*this, "reg3_view")
		, m_reg4_view(*this, "reg4_view")
		, m_reg_rom_banks(*this, "reg%u_rom_bank", 0U)
		, m_reg_ram_banks(*this, "reg%u_ram_bank", 0U)
	{ }

	// This function sets up the machine configuration
	void super8008(machine_config &config);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_device<rs232_port_device> m_rs232;
	output_finder<8> m_leds;
	output_finder<> m_run_led;

	required_device<i8251_device> m_uart;
	uint8_t mmap[4] = {0}; //used to simulate the 74670 4x4 memory mapper
	optional_device<super8008_bus_device> m_bus;
	optional_device_array<super8008_slot_device, 8> m_slots;

	memory_view m_reg1_view;
	memory_view m_reg2_view;
	memory_view m_reg3_view;
	memory_view m_reg4_view;
	memory_view *const m_reg_views[4] =
		{ &m_reg1_view, &m_reg2_view, &m_reg3_view, &m_reg4_view };

	required_memory_bank_array<4> m_reg_rom_banks;
	required_memory_bank_array<4> m_reg_ram_banks;

	uint8_t m_take_state = 0;

	virtual void machine_start() override ATTR_COLD;

	// address maps for program memory and io memory
	void super8008_mem(address_map &map);
	void super8008_io(address_map &map);

	uint8_t port_1_read();
	void serial_leds(uint8_t data);

	uint8_t ext_read(offs_t offset);
	void ext_write(offs_t offset, uint8_t data);
	void ext_take_w(offs_t offset, uint8_t data);
	void ext_int_w(offs_t offset, uint8_t data);
	void ext_req_w(offs_t offset, uint8_t data);
	void ext_reset_w(offs_t offset, uint8_t data);

	void memory_mapper_w(offs_t offset, uint8_t data);
	memory_map_info get_mmap_info(offs_t offset);
	memory_map_info get_mmap_info(uint8_t idx);
	void select_memory_view(uint8_t index);

	//void log_mmap_info(std::string context, std::string message, offs_t offset, memory_map_info *mmap_info);
};


void super8008_state::machine_start()
{
	m_leds.resolve();
	m_run_led.resolve();

	for(int i = 0; i < 4; i++)
	{
		m_reg_views[i]->select(0);
		m_reg_rom_banks[i]->configure_entries(0, 4, m_rom->base(), 0x1000);
		m_reg_ram_banks[i]->configure_entries(0, 4, m_ram->pointer(), 0x1000);
		m_reg_rom_banks[i]->set_entry(0);
		m_reg_ram_banks[i]->set_entry(0);
	}

	m_run_led = 1;
}

memory_map_info super8008_state::get_mmap_info(offs_t offset)
{
	memory_map_info mmap_info;

	mmap_info.mmap_index = (BIT(offset, 13) << 1) | BIT(offset, 12);
	mmap_info.mmap_value = mmap[mmap_info.mmap_index & MMAP_MASK];
	mmap_info.ra12   = BIT(mmap_info.mmap_value, 0);
	mmap_info.ra13   = BIT(mmap_info.mmap_value, 1);
	mmap_info.ext_cs = BIT(mmap_info.mmap_value, 2);
	mmap_info.rom_cs = BIT(mmap_info.mmap_value, 3);
	mmap_info.address= (offset & 0x0fff) | ((mmap_info.ra13 << 13) | (mmap_info.ra12 << 12));

	return mmap_info;
}

memory_map_info super8008_state::get_mmap_info(uint8_t idx)
{
	offs_t offset = (idx & 0x3) << 12;
	return get_mmap_info(offset);
}

// void super8008_state::log_mmap_info(std::string context, std::string message, offs_t offset, memory_map_info *mmap_info)
// {
// 	logerror("%s:%s ($%04X) mmap_index %X mmap_value %X, ra12 %d, ra13 %d, ext_cs %d, rom_cs %d address %04X\n",
// 		context,
// 		message,
// 		offset,
// 		mmap_info->mmap_index,
// 		mmap_info->mmap_value,
// 		mmap_info->ra12,
// 		mmap_info->ra13,
// 		mmap_info->ext_cs,
// 		mmap_info->rom_cs,
// 		mmap_info->address);
// }

uint8_t super8008_state::ext_read(offs_t offset)
{
	memory_map_info mmap_info = get_mmap_info(offset);
	//log_mmap_info(machine().describe_context(), "external read", offset, &mmap_info);
	return m_bus->ext_read(mmap_info.address);
}


//Does takew also write to master's memory?  That is the only way the conway code would work.
//
// The PLD has the following:
//
//   /* RAM is selected if not ROM and not EXTERNAL,
//   * ... or if EXTERN and TAKEW and we're about to write
//   */
//   RAMCS = (!ROMCS & !EXTCS) # (EXTCS & TAKEW & EXT_WRP);
//
// One thing to be aware of, the input for EXTCS, TAKEW is inverted.
//
// Notes about EXT_WRP from the IO PLD:
//
//   /* Set EXT_WRP ahead of the eventual MEMWR that will occur. This is used
//    * by blaster when we want to take all blasters on write. It will be gated by
//    * blaster by EXT_CS, so there's no need to factor in EXT_CS here.
//    *
//    * Write cycle occurs during T3 and PCW when SYNC is low. MW is toggled during
//    * this cycle when the O2 clock is high. EXT_WRP should come on before the O2
//    * clock
//    */
//

void super8008_state::ext_write(offs_t offset, uint8_t data)
{
	memory_map_info mmap_info = get_mmap_info(offset);
	m_bus->ext_write(mmap_info.address, data);
	//log_mmap_info(machine().describe_context(), "external write", offset, &mmap_info);
	//TAKEW logic
	if (!BIT(m_take_state, 7))
	{
		//log_mmap_info(machine().describe_context(), "takew", offset, &mmap_info);
		m_ram->pointer()[mmap_info.address] = data;
	}
}

// Comment from the PLD file:
//
//   simulated SR flip-flop made up of cross-connected NAND gates.
//   the flip-flop is set when the reset signal from the DS1233 goes low
//   (power-on-reset) and cleared when input port 1 is accessed.
//   when set, the flip-flop forces all memory accesses to select the
//   EPROM. when reset, the flip-flop permits the normal memory map.

uint8_t super8008_state::port_1_read()
{
	return m_bus->ext_run();
}

void super8008_state::serial_leds(uint8_t data)
{
	for(int i = 0; i < 8; i++)
	{
		m_leds[i] = BIT(data, 7-i);
	}
}

// take, int and req are tied to one of the seven blasters
// reset is shared between all blasters
//
// reset - boostrap all of the blasters
// run   - check to see if blaster is running or halted
// int   - interrupt the blaster out of the halted state to the running state
// take  - connect blasters memory to the bus
// req   - ask blaster to go into the halt state (not used in the demos)
//
// reset, run, and int are routed to blaster's cpu
// take is routed to buffers around blasters's 16K of RAM
// ext_cs from the memory mapper is routed to blaster's 16K RAM's cs


void super8008_state::ext_take_w(offs_t offset, uint8_t data)
{
	m_take_state = data;
	m_bus->ext_take(data);
}

void super8008_state::ext_int_w(offs_t offset, uint8_t data)
{
	m_bus->ext_int();
}

void super8008_state::ext_req_w(offs_t offset, uint8_t data)
{
	m_bus->ext_req();
}

void super8008_state::ext_reset_w(offs_t offset, uint8_t data)
{
	m_bus->ext_reset();
}

void super8008_state::select_memory_view(uint8_t index)
{
	memory_map_info mmap_info = get_mmap_info(index);
	//log_mmap_info(machine().describe_context(), "select_memory_view", index, &mmap_info);
	uint8_t bank_index = ((mmap_info.ra13 << 1) | mmap_info.ra12);

	if (!mmap_info.rom_cs)
	{
		m_reg_views[mmap_info.mmap_index]->select(0);
		m_reg_rom_banks[mmap_info.mmap_index]->set_entry(bank_index);
	}
	else if (!mmap_info.ext_cs)
	{
		m_reg_views[mmap_info.mmap_index]->select(1);
	}
	else
	{
		m_reg_views[mmap_info.mmap_index]->select(2);
		m_reg_ram_banks[mmap_info.mmap_index]->set_entry(bank_index);
	}
}

void super8008_state::memory_mapper_w(offs_t offset, uint8_t data)
{
	mmap[offset] = data ;
	select_memory_view(offset);
	//logerror("super-8008 memory mapper write ($%04X) masked ($%04X) data %04X \n", offset, index, mmap[index]);
}

void super8008_state::super8008_mem(address_map &map)
{
	// Comment from monitor.asm
	//
	//   when the reset pushbutton is pressed, the flip-flop is set which generates an interrupt
	//   and clears the address latches. thus, the first instruction is thus always fetched from
	//   address 0. the instruction at address 0 must be a single byte transfer instruction in
	//   order to set the program counter. i.e., it must be one of the RST opcodes.
	//

	//A 74670 4x4 register file is used for the memory mapper.  This
	//code is setting the memory view for all possible combinations
	//of the 4x4 register.

	uint16_t start = 0x0000;
	uint16_t end = 0x0fff;
	for(uint8_t i = 0; i < 4; i++)
	{
		map(start, end).view(*m_reg_views[i]);
		(*m_reg_views[i])[0](start, end).bankr(m_reg_rom_banks[i]);
		(*m_reg_views[i])[1](0x0000, 0x3fff).rw(
			FUNC(super8008_state::ext_read),
			FUNC(super8008_state::ext_write));
		(*m_reg_views[i])[2](start, end).bankrw(m_reg_ram_banks[i]);
		start += 0x1000;
		end += 0x1000;
	}
}


//For checking the status master performs a read (IN) on port 0 with device 0 or 1 and it is a read.  This is the same
//interupt that will take use out of start.
//and device 0 is for dipswitches
//    device 1 is the external status reuquest
// this triggers a buffer that sets the run byte on the bus
//
// On master the run byte on the bus will set running based on the jumper (board 0 jumped and ext_run0 low => running low)
// running also sets the running led on blaster
// running is set from the pld that is monitoring the state flag.

//TODO(jhe) once all of this is working sort these
void super8008_state::super8008_io(address_map &map)
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

	map(0x01, 0x01).r(FUNC(super8008_state::port_1_read));//Signals start (come out of the start state)
	map(0x02, 0x02).r(m_uart, FUNC(i8251_device::read));
	map(0x03, 0x03).r(m_uart, FUNC(i8251_device::status_r));
	map(0x08, 0x08).w(FUNC(super8008_state::serial_leds));
	map(0x12, 0x12).w(m_uart, FUNC(i8251_device::write));
	map(0x13, 0x13).w(m_uart, FUNC(i8251_device::control_w));

	//A 74670 4x4 register file is used for the memory mapper
	//When not in start mode, address lines A12 and A13 are used
	//to reference which of the 4 registers contains the A12 and A13
	//lines of the ram or rom.  The ram and rom cs are the high bit
	//and bit 3 is used for issuing an external chip select.
	map(0x0c, 0x0f).w(FUNC(super8008_state::memory_mapper_w));

	map(0x14, 0x14).w(FUNC(super8008_state::ext_take_w));
	map(0x15, 0x15).w(FUNC(super8008_state::ext_int_w));
	map(0x16, 0x16).w(FUNC(super8008_state::ext_req_w));
	map(0x17, 0x17).w(FUNC(super8008_state::ext_reset_w));
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD"  , 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD"  , 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY"  , 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
	//DEVICE_INPUT_DEFAULTS( "TERM_CONF", 0x080, 0x080 ) // Auto LF on CR
DEVICE_INPUT_DEFAULTS_END

#define SUPER8008_SETUP_BLASTER_JUMPERS(_num) \
	DEVICE_INPUT_DEFAULTS_START(blaster_input_ ##_num) \
		DEVICE_INPUT_DEFAULTS("EXT_TAKE", 0xff, _num) \
		DEVICE_INPUT_DEFAULTS("EXT_RUN" , 0xff, _num) \
		DEVICE_INPUT_DEFAULTS("EXT_REQ" , 0xff, _num) \
		DEVICE_INPUT_DEFAULTS("EXT_INT" , 0xff, _num) \
	DEVICE_INPUT_DEFAULTS_END

SUPER8008_SETUP_BLASTER_JUMPERS(0)
SUPER8008_SETUP_BLASTER_JUMPERS(1)
SUPER8008_SETUP_BLASTER_JUMPERS(2)
SUPER8008_SETUP_BLASTER_JUMPERS(3)
SUPER8008_SETUP_BLASTER_JUMPERS(4)
SUPER8008_SETUP_BLASTER_JUMPERS(5)
SUPER8008_SETUP_BLASTER_JUMPERS(6)


void super8008_state::super8008(machine_config &config)
{
	RAM(config, m_ram).set_default_size("32K");

	//500khz
	I8008(config, m_maincpu, XTAL(1'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &super8008_state::super8008_mem);
	m_maincpu->set_addrmap(AS_IO, &super8008_state::super8008_io);

	config.set_default_layout(layout_super8008);

	i8251_device &uart(I8251(config, m_uart, 0));

	//Schematic says our clock is 4.9152 / 4
	//
	//Kludge to get the terminal to work at 9600 baud. The monitor configures
	//for 16x so 16*9600hz = 153.600 khz
	clock_device &uart_clock(CLOCK(config, "uart_clock", 153600));//4.9152_MHz_XTAL / 4));
	uart_clock.signal_handler().set(m_uart, FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append(m_uart, FUNC(i8251_device::write_rxc));

	uart.txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));

	// To provide a console, configure a "default terminal" to connect to the serial port
	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	m_rs232->rxd_handler().set(m_uart, FUNC(i8251_device::write_rxd));
	m_rs232->dsr_handler().set(m_uart, FUNC(i8251_device::write_dsr));
	m_rs232->cts_handler().set(m_uart, FUNC(i8251_device::write_cts));
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal)); // must be below the DEVICE_INPUT_DEFAULTS_START block

	SUPER8008_BUS(config, m_bus, 153600);

	SUPER8008_SLOT(config, m_slots[0], super8008_bus_devices, "super8008_blaster");
	SUPER8008_SLOT(config, m_slots[1], super8008_bus_devices, "super8008_blaster");
	SUPER8008_SLOT(config, m_slots[2], super8008_bus_devices, "super8008_blaster");
	SUPER8008_SLOT(config, m_slots[3], super8008_bus_devices, "super8008_blaster");
	SUPER8008_SLOT(config, m_slots[4], super8008_bus_devices, "super8008_blaster");
	SUPER8008_SLOT(config, m_slots[5], super8008_bus_devices, "super8008_blaster");
	SUPER8008_SLOT(config, m_slots[6], super8008_bus_devices, "super8008_blaster");

	m_slots[0]->set_option_device_input_defaults("super8008_blaster", DEVICE_INPUT_DEFAULTS_NAME(blaster_input_0));
	m_slots[1]->set_option_device_input_defaults("super8008_blaster", DEVICE_INPUT_DEFAULTS_NAME(blaster_input_1));
	m_slots[2]->set_option_device_input_defaults("super8008_blaster", DEVICE_INPUT_DEFAULTS_NAME(blaster_input_2));
	m_slots[3]->set_option_device_input_defaults("super8008_blaster", DEVICE_INPUT_DEFAULTS_NAME(blaster_input_3));
	m_slots[4]->set_option_device_input_defaults("super8008_blaster", DEVICE_INPUT_DEFAULTS_NAME(blaster_input_4));
	m_slots[5]->set_option_device_input_defaults("super8008_blaster", DEVICE_INPUT_DEFAULTS_NAME(blaster_input_5));
	m_slots[6]->set_option_device_input_defaults("super8008_blaster", DEVICE_INPUT_DEFAULTS_NAME(blaster_input_6));
}

ROM_START(super8008)
	ROM_REGION(0x10000, "rom",0) //For the addresses to makes since, setup a huge rom chip and load the roms to the cooresponding machine addresses
	//         Name                     offset  Length   hash
	ROM_LOAD("monitor-8251-master.bin", 0x0000, 0x14b5, CRC(8cfd849e) SHA1(ea40b0066823df6c1dd896e5425285dddd3432e3))
	ROM_LOAD("scelbal-8251-master.bin", 0x2000, 0x2000, CRC(51f98937) SHA1(83705305c24313cd81e14d1e3cefb3422a9e8118))
ROM_END

} // anonymous namespace


// This ties everything together
//    YEAR  NAME            PARENT    COMPAT    MACHINE        INPUT          CLASS             INIT           COMPANY               FULLNAME       FLAGS
COMP( 2024, super8008,          0,        0,    super8008,         0,         super8008_state,  empty_init,    "Dr. Scott M. Baker", "8008-Super",  MACHINE_NO_SOUND_HW )
