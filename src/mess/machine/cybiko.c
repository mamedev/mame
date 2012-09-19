/*

    Cybiko Wireless Inter-tainment System

    (c) 2001-2007 Tim Schuerewegen

    Cybiko Classic (V1)
    Cybiko Classic (V2)
    Cybiko Xtreme

*/

#include "includes/cybiko.h"

#define LOG_LEVEL  1
#define _logerror(level,x)  do { if (LOG_LEVEL > level) logerror x; } while (0)

#define RAMDISK_SIZE (512 * 1024)

/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////

#define MACHINE_STOP(name) \
	static void machine_stop_##name( running_machine &machine)

// machine stop
MACHINE_STOP( cybikov1 );
MACHINE_STOP( cybikov2 );
MACHINE_STOP( cybikoxt );

// state->m_rs232
static void cybiko_rs232_init(running_machine &machine);
static void cybiko_rs232_exit(void);
static void cybiko_rs232_reset(void);

////////////////////////
// DRIVER INIT & EXIT //
////////////////////////

static void init_ram_handler(running_machine &machine, offs_t start, offs_t size, offs_t mirror)
{
	machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_bank(start, start + size - 1, 0, mirror - size, "bank1");
	machine.device("maincpu")->memory().space(AS_PROGRAM).install_write_bank(start, start + size - 1, 0, mirror - size, "bank1");
	machine.root_device().membank( "bank1" )->set_base( machine.device<ram_device>(RAM_TAG)->pointer());
}

DRIVER_INIT_MEMBER(cybiko_state,cybikov1)
{
	_logerror( 0, ("init_cybikov1\n"));
	init_ram_handler(machine(), 0x200000, machine().device<ram_device>(RAM_TAG)->size(), 0x200000);
}

DRIVER_INIT_MEMBER(cybiko_state,cybikov2)
{
	_logerror( 0, ("init_cybikov2\n"));
	init_ram_handler(machine(), 0x200000, machine().device<ram_device>(RAM_TAG)->size(), 0x200000);
}

DRIVER_INIT_MEMBER(cybiko_state,cybikoxt)
{
	_logerror( 0, ("init_cybikoxt\n"));
	init_ram_handler(machine(), 0x400000, machine().device<ram_device>(RAM_TAG)->size(), 0x200000);
}

///////////////////
// MACHINE START //
///////////////////

static emu_file *nvram_system_fopen( running_machine &machine, UINT32 openflags, const char *name)
{
	file_error filerr;
	astring fname(machine.system().name, PATH_SEPARATOR, name, ".nv");
	emu_file *file = global_alloc(emu_file(machine.options().nvram_directory(), openflags));
	filerr = file->open(fname.cstr());
	if (filerr == FILERR_NONE)
		return file;

	global_free(file);
	return NULL;
}

typedef void (nvram_load_func)(running_machine &machine, emu_file *file);

static int nvram_system_load( running_machine &machine, const char *name, nvram_load_func _nvram_load, int required)
{
	emu_file *file;
	file = nvram_system_fopen( machine, OPEN_FLAG_READ, name);
	if (!file)
	{
		if (required) mame_printf_error( "nvram load failed (%s)\n", name);
		return FALSE;
	}
	(*_nvram_load)(machine, file);
	global_free(file);
	return TRUE;
}

typedef void (nvram_save_func)(running_machine &machine, emu_file *file);

static int nvram_system_save( running_machine &machine, const char *name, nvram_save_func _nvram_save)
{
	emu_file *file;
	file = nvram_system_fopen( machine, OPEN_FLAG_CREATE | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE_PATHS, name);
	if (!file)
	{
		mame_printf_error( "nvram save failed (%s)\n", name);
		return FALSE;
	}
	(*_nvram_save)(machine, file);
	global_free(file);
	return TRUE;
}

static void cybiko_pcf8593_load(running_machine &machine, emu_file *file)
{
	device_t *device = machine.device("rtc");
	pcf8593_load(device, file);
}

static void cybiko_pcf8593_save(running_machine &machine, emu_file *file)
{
	device_t *device = machine.device("rtc");
	pcf8593_save(device, file);
}

static void cybiko_at45dbxx_load(running_machine &machine, emu_file *file)
{
	device_t *device = machine.device("flash1");
	at45dbxx_load(device, file);
}

static void cybiko_at45dbxx_save(running_machine &machine, emu_file *file)
{
	device_t *device = machine.device("flash1");
	at45dbxx_save(device, file);
}

static void cybiko_sst39vfx_load(running_machine &machine, emu_file *file)
{
	device_t *device = machine.device("flash2");
	sst39vfx_load(device, file);
}

static void cybiko_sst39vfx_save(running_machine &machine, emu_file *file)
{
	device_t *device = machine.device("flash2");
	sst39vfx_save(device, file);
}

static void cybiko_ramdisk_load(running_machine &machine, emu_file *file)
{
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();
#ifdef LSB_FIRST
	UINT8 *temp = (UINT8*)malloc( RAMDISK_SIZE);
	file->read( temp, RAMDISK_SIZE);
	for (int i = 0; i < RAMDISK_SIZE; i += 2)
	{
		ram[i+0] = temp[i+1];
		ram[i+1] = temp[i+0];
	}
	free( temp);
#else
	file->read( ram, RAMDISK_SIZE);
#endif
}

static void cybiko_ramdisk_save(running_machine &machine, emu_file *file)
{
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();
#ifdef LSB_FIRST
	UINT8 *temp = (UINT8*)malloc( RAMDISK_SIZE);
	for (int i = 0; i < RAMDISK_SIZE; i += 2)
	{
		temp[i+0] = ram[i+1];
		temp[i+1] = ram[i+0];
	}
	file->write( temp, RAMDISK_SIZE);
	free( temp);
#else
	file->write( ram, RAMDISK_SIZE);
#endif
}

void cybiko_state::machine_start()
{
	_logerror( 0, ("machine_start_cybikov1\n"));
	// real-time clock
	nvram_system_load( machine(), "rtc", cybiko_pcf8593_load, 0);
	// serial dataflash
	nvram_system_load( machine(), "flash1", cybiko_at45dbxx_load, 1);
	// serial port
	cybiko_rs232_init(machine());
	// other
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(machine_stop_cybikov1),&machine()));
}

MACHINE_START_MEMBER(cybiko_state,cybikov2)
{
	device_t *flash2 = machine().device("flash2");

	_logerror( 0, ("machine_start_cybikov2\n"));
	// real-time clock
	nvram_system_load( machine(), "rtc", cybiko_pcf8593_load, 0);
	// serial dataflash
	nvram_system_load( machine(), "flash1", cybiko_at45dbxx_load, 1);
	// multi-purpose flash
	nvram_system_load( machine(), "flash2", cybiko_sst39vfx_load, 1);
	machine().root_device().membank( "bank2" )->set_base( sst39vfx_get_base(flash2));
	// serial port
	cybiko_rs232_init(machine());
	// other
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(machine_stop_cybikov2),&machine()));
}

MACHINE_START_MEMBER(cybiko_state,cybikoxt)
{
	device_t *flash2 = machine().device("flash2");
	_logerror( 0, ("machine_start_cybikoxt\n"));
	// real-time clock
	nvram_system_load( machine(), "rtc", cybiko_pcf8593_load, 0);
	// multi-purpose flash
	nvram_system_load( machine(), "flash2", cybiko_sst39vfx_load, 1);
	machine().root_device().membank( "bank2" )->set_base( sst39vfx_get_base(flash2));
	// ramdisk
	nvram_system_load( machine(), "ramdisk", cybiko_ramdisk_load, 0);
	// serial port
	cybiko_rs232_init(machine());
	// other
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(machine_stop_cybikoxt),&machine()));
}

///////////////////
// MACHINE RESET //
///////////////////

void cybiko_state::machine_reset()
{
	_logerror( 0, ("machine_reset_cybikov1\n"));
	cybiko_rs232_reset();
}

MACHINE_RESET_MEMBER(cybiko_state,cybikov2)
{
	_logerror( 0, ("machine_reset_cybikov2\n"));
	cybiko_rs232_reset();
}

MACHINE_RESET_MEMBER(cybiko_state,cybikoxt)
{
	_logerror( 0, ("machine_reset_cybikoxt\n"));
	cybiko_rs232_reset();
}

//////////////////
// MACHINE STOP //
//////////////////

MACHINE_STOP( cybikov1 )
{
	_logerror( 0, ("machine_stop_cybikov1\n"));
	// real-time clock
	nvram_system_save( machine, "rtc", cybiko_pcf8593_save);
	// serial dataflash
	nvram_system_save( machine, "flash1", cybiko_at45dbxx_save);
	// serial port
	cybiko_rs232_exit();
}

MACHINE_STOP( cybikov2 )
{
	_logerror( 0, ("machine_stop_cybikov2\n"));
	// real-time clock
	nvram_system_save( machine, "rtc", cybiko_pcf8593_save);
	// serial dataflash
	nvram_system_save( machine, "flash1", cybiko_at45dbxx_save);
	// multi-purpose flash
	nvram_system_save( machine, "flash2", cybiko_sst39vfx_save);
	// serial port
	cybiko_rs232_exit();
}

MACHINE_STOP( cybikoxt )
{
	_logerror( 0, ("machine_stop_cybikoxt\n"));
	// real-time clock
	nvram_system_save( machine, "rtc", cybiko_pcf8593_save);
	// multi-purpose flash
	nvram_system_save( machine, "flash2", cybiko_sst39vfx_save);
	// ramdisk
	nvram_system_save( machine, "ramdisk", cybiko_ramdisk_save);
	// serial port
	cybiko_rs232_exit();
}

///////////
// RS232 //
///////////


static void cybiko_rs232_init(running_machine &machine)
{
	cybiko_state *state = machine.driver_data<cybiko_state>();
	_logerror( 0, ("cybiko_rs232_init\n"));
	memset( &state->m_rs232, 0, sizeof( state->m_rs232));
//  machine.scheduler().timer_pulse(TIME_IN_HZ( 10), FUNC(rs232_timer_callback));
}

static void cybiko_rs232_exit()
{
	_logerror( 0, ("cybiko_rs232_exit\n"));
}

static void cybiko_rs232_reset()
{
	_logerror( 0, ("cybiko_rs232_reset\n"));
}

void cybiko_state::cybiko_rs232_write_byte( int data )
{
//  printf( "%c", data);
}

void cybiko_state::cybiko_rs232_pin_sck( int data )
{
	_logerror( 3, ("cybiko_rs232_pin_sck (%d)\n", data));
	// clock high-to-low
	if ((m_rs232.pin.sck == 1) && (data == 0))
	{
		// transmit
		if (m_rs232.pin.txd) m_rs232.tx_byte = m_rs232.tx_byte | (1 << m_rs232.tx_bits);
		m_rs232.tx_bits++;
		if (m_rs232.tx_bits == 8)
		{
			m_rs232.tx_bits = 0;
			cybiko_rs232_write_byte(m_rs232.tx_byte);
			m_rs232.tx_byte = 0;
		}
		// receive
		m_rs232.pin.rxd = (m_rs232.rx_byte >> m_rs232.rx_bits) & 1;
		m_rs232.rx_bits++;
		if (m_rs232.rx_bits == 8)
		{
			m_rs232.rx_bits = 0;
			m_rs232.rx_byte = 0;
		}
	}
	// save sck
	m_rs232.pin.sck = data;
}

void cybiko_state::cybiko_rs232_pin_txd( int data )
{
	_logerror( 3, ("cybiko_rs232_pin_txd (%d)\n", data));
	m_rs232.pin.txd = data;
}

int cybiko_state::cybiko_rs232_pin_rxd()
{
	_logerror( 3, ("cybiko_rs232_pin_rxd\n"));
	return m_rs232.pin.rxd;
}

int cybiko_state::cybiko_rs232_rx_queue()
{
	return 0;
}

/////////////////////////
// READ/WRITE HANDLERS //
/////////////////////////

READ16_MEMBER( cybiko_state::cybiko_lcd_r )
{
	UINT16 data = 0;
	if (ACCESSING_BITS_8_15) data |= (m_crtc->reg_idx_r(space, offset) << 8);
	if (ACCESSING_BITS_0_7) data |= (m_crtc->reg_dat_r(space, offset) << 0);
	return data;
}

WRITE16_MEMBER( cybiko_state::cybiko_lcd_w )
{
	if (ACCESSING_BITS_8_15) m_crtc->reg_idx_w(space, offset, (data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7) m_crtc->reg_dat_w(space, offset, (data >> 0) & 0xff);
}

int cybiko_state::cybiko_key_r( offs_t offset, int mem_mask)
{
	static const char *const keynames[] = { "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10", "A11", "A12", "A13", "A14", "A15" };
	UINT16 data = 0xFFFF;
	for (UINT8 i = 0; i < 15; i++)
	{
		if (!BIT(offset, i))
			data &= ~ioport(keynames[i])->read_safe(0);
	}
	if (data != 0xFFFF)
	{
		_logerror( 1, ("cybiko_key_r (%08X/%04X) %04X\n", offset, mem_mask, data));
	}
	return data;
}

READ16_MEMBER( cybiko_state::cybikov1_key_r )
{
	return cybiko_key_r(offset, mem_mask);
}

READ16_MEMBER( cybiko_state::cybikov2_key_r )
{
	UINT16 data = cybiko_key_r(offset, mem_mask);
	if (!BIT(offset, 0))
		data |= 0x0002; // or else [esc] does not work in "lost in labyrinth"
	return data;
}

READ16_MEMBER( cybiko_state::cybikoxt_key_r )
{
	return cybiko_key_r(offset, mem_mask);
}

READ8_MEMBER( cybiko_state::cybikov1_io_reg_r )
{
	UINT8 data = 0;
	_logerror( 2, ("cybikov1_io_reg_r (%08X)\n", offset));
	switch (offset)
	{
		// keyboard
		case H8S_IO_PORT1 :
		{
			if (!BIT(ioport("A1")->read(), 1))
				data |= (1 << 3); // "esc" key
		}
		break;
		// serial dataflash
		case H8S_IO_PORT3 :
		{
			if (at45dbxx_pin_so(m_flash1))
				data |= H8S_P3_RXD1;
		}
		break;
		// rs232
		case H8S_IO_PORT5 :
		{
			if (cybiko_rs232_pin_rxd())
				data |= H8S_P5_RXD2;
		}
		break;
		// real-time clock
		case H8S_IO_PORTF :
		{
			data = H8S_PF_PF2;
			if (pcf8593_pin_sda_r(m_rtc))
				data |= H8S_PF_PF0;
		}
		break;
		// serial 2
		case H8S_IO_SSR2 :
		{
			if (cybiko_rs232_rx_queue())
				data |= H8S_SSR_RDRF;
		}
		break;
	}
	return data;
}

READ8_MEMBER( cybiko_state::cybikov2_io_reg_r )
{
	UINT8 data = 0;
	_logerror( 2, ("cybikov2_io_reg_r (%08X)\n", offset));
	switch (offset)
	{
		// keyboard
		case H8S_IO_PORT1 :
		{
			if (!BIT(ioport("A1")->read(), 1))
				data |= (1 << 3); // "esc" key
		}
		break;
		// serial dataflash
		case H8S_IO_PORT3 :
		{
			if (at45dbxx_pin_so(m_flash1))
				data |= H8S_P3_RXD1;
		}
		break;
		// rs232
		case H8S_IO_PORT5 :
		{
			if (cybiko_rs232_pin_rxd())
				data |= H8S_P5_RXD2;
		}
		break;
		// real-time clock
		case H8S_IO_PORTF :
		{
			data = H8S_PF_PF2;
			if (pcf8593_pin_sda_r(m_rtc))
				data |= H8S_PF_PF0;
		}
		break;
		// serial 2
		case H8S_IO_SSR2 :
		{
			if (cybiko_rs232_rx_queue())
				data |= H8S_SSR_RDRF;
		}
		break;
	}
	return data;
}

READ8_MEMBER( cybiko_state::cybikoxt_io_reg_r )
{
	UINT8 data = 0;
	_logerror( 2, ("cybikoxt_io_reg_r (%08X)\n", offset));
	switch (offset)
	{
		// rs232
		case H8S_IO_PORT3 :
		{
			if (cybiko_rs232_pin_rxd())
				data |= H8S_P3_RXD1;
		}
		break;
		// ...
		case H8S_IO_PORTA :
		{
			data |= (1 << 6); // recharge batteries (xtreme)
			data |= (1 << 7); // on/off key (xtreme)
		}
		break;
		// real-time clock
		case H8S_IO_PORTF :
		{
			if (pcf8593_pin_sda_r(m_rtc))
				data |= H8S_PF_PF6;
		}
		break;
		// serial 1
		case H8S_IO_SSR1 :
		{
			if (cybiko_rs232_rx_queue())
				data |= H8S_SSR_RDRF;
		}
		break;
	}
	return data;
}

WRITE8_MEMBER( cybiko_state::cybikov1_io_reg_w )
{
	_logerror( 2, ("cybikov1_io_reg_w (%08X/%02X)\n", offset, data));
	switch (offset)
	{
		// speaker
		case H8S_IO_P1DR :
		{
			speaker_level_w(m_speaker, (data & H8S_P1_TIOCB1) ? 1 : 0);
		}
		break;
		// serial dataflash
		case H8S_IO_P3DR :
		{
			at45dbxx_pin_cs (m_flash1, (data & H8S_P3_SCK0) ? 0 : 1);
			at45dbxx_pin_si (m_flash1, (data & H8S_P3_TXD1) ? 1 : 0);
			at45dbxx_pin_sck(m_flash1, (data & H8S_P3_SCK1) ? 1 : 0);
		}
		break;
		// rs232
		case H8S_IO_P5DR :
		{
			cybiko_rs232_pin_txd((data & H8S_P5_TXD2) ? 1 : 0);
			cybiko_rs232_pin_sck((data & H8S_P5_SCK2) ? 1 : 0);
		}
		break;
		// real-time clock
		case H8S_IO_PFDR :
		{
			pcf8593_pin_scl(m_rtc, (data & H8S_PF_PF1) ? 1 : 0);
		}
		break;
		// real-time clock
		case H8S_IO_PFDDR :
		{
			pcf8593_pin_sda_w(m_rtc, (data & H8S_PF_PF0) ? 0 : 1);
		}
		break;
	}
}

WRITE8_MEMBER( cybiko_state::cybikov2_io_reg_w )
{
	_logerror( 2, ("cybikov2_io_reg_w (%08X/%02X)\n", offset, data));
	switch (offset)
	{
		// speaker
		case H8S_IO_P1DR :
		{
			speaker_level_w(m_speaker, (data & H8S_P1_TIOCB1) ? 1 : 0);
		}
		break;
		// serial dataflash
		case H8S_IO_P3DR :
		{
			at45dbxx_pin_cs (m_flash1, (data & H8S_P3_SCK0) ? 0 : 1);
			at45dbxx_pin_si (m_flash1, (data & H8S_P3_TXD1) ? 1 : 0);
			at45dbxx_pin_sck(m_flash1, (data & H8S_P3_SCK1) ? 1 : 0);
		}
		break;
		// rs232
		case H8S_IO_P5DR :
		{
			cybiko_rs232_pin_txd((data & H8S_P5_TXD2) ? 1 : 0);
			cybiko_rs232_pin_sck((data & H8S_P5_SCK2) ? 1 : 0);
		}
		break;
		// real-time clock
		case H8S_IO_PFDR :
		{
			pcf8593_pin_scl(m_rtc, (data & H8S_PF_PF1) ? 1 : 0);
		}
		break;
		// real-time clock
		case H8S_IO_PFDDR :
		{
			pcf8593_pin_sda_w(m_rtc, (data & H8S_PF_PF0) ? 0 : 1);
		}
		break;
	}
}

WRITE8_MEMBER( cybiko_state::cybikoxt_io_reg_w )
{
	_logerror( 2, ("cybikoxt_io_reg_w (%08X/%02X)\n", offset, data));
	switch (offset)
	{
		// speaker
		case H8S_IO_P1DR :
		{
			speaker_level_w(m_speaker, (data & H8S_P1_TIOCB1) ? 1 : 0);
		}
		break;
		// rs232
		case H8S_IO_P3DR :
		{
			cybiko_rs232_pin_txd((data & H8S_P3_TXD1) ? 1 : 0);
			cybiko_rs232_pin_sck((data & H8S_P3_SCK1) ? 1 : 0);
		}
		break;
		// real-time clock
		case H8S_IO_PFDR :
		{
			pcf8593_pin_scl(m_rtc, (data & H8S_PF_PF1) ? 1 : 0);
		}
		break;
		// real-time clock
		case H8S_IO_PFDDR :
		{
			pcf8593_pin_sda_w(m_rtc, (data & H8S_PF_PF6) ? 0 : 1);
		}
		break;
	}
}

// Cybiko Xtreme writes following byte pairs to 0x200003/0x200000
// 00/01, 00/C0, 0F/32, 0D/03, 0B/03, 09/50, 07/D6, 05/00, 04/00, 20/00, 23/08, 27/01, 2F/08, 2C/02, 2B/08, 28/01
// 04/80, 05/02, 00/C8, 00/C8, 00/C0, 1B/2C, 00/01, 00/C0, 1B/6C, 0F/10, 0D/07, 0B/07, 09/D2, 07/D6, 05/00, 04/00,
// 20/00, 23/08, 27/01, 2F/08, 2C/02, 2B/08, 28/01, 37/08, 34/04, 33/08, 30/03, 04/80, 05/02, 1B/6C, 00/C8
WRITE16_MEMBER( cybiko_state::cybiko_usb_w )
{
	if (ACCESSING_BITS_8_15) _logerror( 2, ("[%08X] <- %02X\n", 0x200000 + offset * 2 + 0, (data >> 8) & 0xFF));
	if (ACCESSING_BITS_0_7) _logerror( 2, ("[%08X] <- %02X\n", 0x200000 + offset * 2 + 1, (data >> 0) & 0xFF));
}
