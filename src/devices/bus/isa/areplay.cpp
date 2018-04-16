// license:BSD-3-Clause
// copyright-holders:Bavarese
/***************************************************************************
	Action Replay for DOS (DATEL UK 1994/1995).
	Requires 386/486 and DOS 3.2 or above (no Windows).

	Hardware details:
	- 6 DIP switches define hardware address and i/o port,
	  another 6 define the IRQ 			
	DIP switches should only be altered when PC is off. No Plug & Play.

	- 2 x UM 6264 (= two 8 K x 8 CMOS SRAM chips)
	- 1 x AMD 29f010 (= 1 Mbit 128Kb x8 flash memory)

	- 4 rectangular PAL chips with port / address / glue logic 

	Codes printed on PCB (compatible with flash firmware)
		REF1066 
		07/11/94
	An earlier design with REF1033 dated 28/10/93 is known.
---------------------------------------------------------------------------------
	Usage: individual drivers needed for real (AREPLAY.COM) and 
	   protected mode (PROT.EXE; incompatible with EMM386 or QEMM).

	Hard disk space _required_ for temp.files by following sections:

	1. .\AREPLAY.CFG (vital configuration file written by initial SETUP)

    2. file viewer (even if only disk A or B are accessed) -> C:\ARVRAM.DSK 

    3. virtual memory managed by AROS, a complex but largely 
       undocumented operating system (in flash ROM)	-> C:\ARVRAM.RAM

	4. the possibility array of certain trainers 	-> C:\POSSIBIL.ITY 

	If hard disk gets unvailable or isn't present, lockups occur!


	Freezer notes: configuration changes (DOS startup files or 
	  altered DIP switches) usually render a freezed image unusable.
	
	Main problem is that Areplay.com must be at a precise location given
	  by (scratch RAM + offset $10) = (BIOS extension address + $3010);
	  at least with 4.8 software release.

	To debug freeze / unfreeze, set the DEBUG DIP switch.  'Offset' is 
	  the freezed location of Areplay.com. It must be the same as the 
	  offset shown when the Areplay TSR is loaded into memory.

    Always keep a 1:1 copy of CONFIG.SYS, AUTOEXEC.BAT and notes about 
	  DIPs + environment (a write protected boot disk would be ideal). 

	HIMEM.SYS parameters matter. Mess 0.186 with 'ct486' liked:
	  device=HIMEM.SYS /MACHINE:AT /TESTMEM:ON /CPUCLOCK:ON
	Second 8K BIOS chunk must not be detected as available RAM! 
	  (check with Msd.exe supplied by DOS 6.x)

	Software caches like DoubleSpace, Smartdrive are incompatible w.freeze
      because of possible hard disk corruption (a flush is done though).
 ***************************************************************************
	Hardware / logic
    ----------------
	GAL/PAL logic is undumped. Yet well-documented C-64 * Replay 
	  cartridges give an idea of some basic principles (similar 
      ROM/RAM banking, flip-flops).  

    There is no NMI on the ISA card and no attempt was made to
	  hide the card from evil minded programmers.

	A constant Irq heartbeat is generated after the card is
	  correctly initialized (when the Areplay.com driver loaded).

    Cyclic led flashes (on the freezer extension outside the PC case)
 	  indicate activity.

	Byte watch, slow motion, key polling and most other functions are 
      implemented in software (via interrupt hooks).

	Bit 3 in the single read register indicates if the interrupt came 
      from the card or originated elsewhere. The bit is used to 
	  decide whether to invoke Action Replay - or skip functionality 
	  altogether. 
	  Assumption: reset upon each read and set by external logic.

	The BIOS extension segment comprises 16K from C8000 on:
	------------- ROM location set by DIP switches --------------------
	8 K ROM segment (determined by write to port 1, usually $0280)
	+ 4 K RAM bank (1-1F; determined by write to port 2, usually $0281)
	+ 4 K scratch RAM (always present, never banked; for vital variables)
	-------------------------------------------------------------------


	Firmware revisions
	------------------
	First revision had a non-upgradeable ROM <= V4.0 (undumped).

	Newer hardware was equipped with AM29010 EEPROM (*)
 
	Firmware (say V48ROM.ROM with V4 switch) is loaded and banked 
	  in before boot - just like any other BIOS extension. 

	Datel drivers refuse to load if the BIOS area chosen is empty
      or the API level of driver and internal (AR)OS do not match.

	As a rule of thumb, versions 4.1 - 4.8 of the TSR run on V4 hardware 

	Driver revisions known are 
      4.1 (outdated; described in German manual, about 1994),
	  4.8 (widely available, ca. 1995) and 5.0 (when released?)

	Before distribution via mailbox, flash files (labeled VxxROM.ROM) 
      were obfuscated by adding 4,3,2,1 to byte 0,1,2,3. 

	This driver accepts encrypted or unencrypted VxxROM.ROM files.

        TODO: 
	(*) unblock flash upgrades, currently intelflash doesn't cooperate

	- XMS / EMS memory not really tested 

	- compatibility with * official * 5.0 binaries (if they exist)

	- improve banking; add missing / secret bits.
	Bit 7 in read register seems to be added in later V5 hardware.

    No 'magic bits' to disable ROM/RAM or cart were found. Bit 5
      in first write register is set when freezer menu is active.
*********************************************************************************/
#include "emu.h"
#include "areplay.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************
DEFINE_DEVICE_TYPE(ISA8_AREPLAY, isa8_areplay_device, "areplay", "Action Replay for DOS - DATEL UK 1994")

//-------------------------------------------------
//  ROM( action replay )  V4.8 or V5.0
//-------------------------------------------------

// - official V4.8 firmware = CRC(6c4a64ce)
// - encoded V5.0 beta firmware  = CRC(0ff4d5ef)
ROM_START( areplay )
	ROM_REGION(0x20000,"v48rom.rom",0)
	ROM_LOAD( "v48rom.rom", 0x0000, 0x20000, CRC(6c4a64ce) SHA1(fac68327a4c8b3cb03def2a69e5930c845a09047) )

	ROM_REGION(0x40000,"v50rom.rom",0)
	ROM_LOAD( "v50rom.rom", 0x0000, 0x20000, CRC(0ff4d5ef) SHA1(eb4cd4f71ebf6d604b8e7924141389585b92ff5e)  )
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------
const tiny_rom_entry *isa8_areplay_device::device_rom_region() const
{
	return ROM_NAME( areplay );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
MACHINE_CONFIG_START(isa8_areplay_device::device_add_mconfig)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("heartbeat", isa8_areplay_device, heartbeat_timer, attotime::from_hz(18))
MACHINE_CONFIG_END
//**************************************************************************

//-------------------------------------------------
//  isa8_areplay_device - constructor
//-------------------------------------------------
isa8_areplay_device::isa8_areplay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_AREPLAY, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void isa8_areplay_device::device_start()
{
	set_isa_device();

	one_shot_timer = timer_alloc(0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void isa8_areplay_device::device_reset()
{
   uint32_t bank_start;
   uint32_t bank_end;

	uint8_t *dest;
	uint16_t io_port_end;

	m_write_port1 = 0; // bit 5 set = no IRQs --------- correct?

	m_timer_fired = true;
	force_irq_to(false); // side effect on global variable: m_is_heartbeat_present = false

	m_current_irq_selected = 5;

	m_current_rom_page = 0; 
	m_current_RAM_PAGE = INITIAL_RAM_BANK_NO; 

	int rom = ioport("ROM_ADDRESS")->read();
	switch(rom)
        {
		default:
			m_current_rom_start = 0xc8000 + (rom * 0x4000);
			break;

		case 3:
			m_current_rom_start = 0xd2000; 
			break;

		case 4:
		case 5:
		case 6:
			m_current_rom_start = 0xd4000 + ( (rom - 4) * 0x4000);
			break;
	}

	int port = ioport("PORT")->read();

	uint16_t  io_port = 0x280; // default
	if( (port >= 0) && (port <= 3) )
	         io_port = 0x280 + (port * 0x10);

	int i_no = ioport("IRQ")->read();
	if( (i_no  >= 2) && (i_no  <= 7) )
	         m_current_irq_selected = i_no;

	// Unmap all 16K (8 K + two independent RAM banks a 4 K):
	m_isa->unmap_bank(m_current_rom_start, m_current_rom_start + SIZE_ROM_WINDOW + (SIZE_RAM_BANK * 2) - 1 ); 
	
	char str[40];
	sprintf(str, "v%drom.rom", 48 + (2 * ioport("COMPATIBILITY")->read()) ); // load "v48rom.rom" 

	m_isa->install_rom(this, m_current_rom_start, m_current_rom_start + SIZE_ROM_WINDOW - 1, str, str);

	memcpy(&m_banked_flash[0], memregion(str)->base(), sizeof(m_banked_flash)); // copy to memory.

	if(m_banked_flash[0] != 0x55) // if blob is unbootable, decode and keep copy in banked_flash -
	{
		uint8_t table[4];
		table[0]=4; table[1]=3; table[2]=2; table[3]=1;

		for(uint32_t i = 0; i < sizeof(m_banked_flash); i++) 
			m_banked_flash[i] -= table[i & 3];
	} 

	dest = machine().root_device().memregion("isa")->base() + m_current_rom_start - 0xc0000;
	memcpy(dest, &m_banked_flash[0], SIZE_ROM_WINDOW); // place bootable BIOS extension (8 K)

	// RAM range: install (4 K chunks)
	bank_start = m_current_rom_start + SIZE_ROM_WINDOW;
	bank_end   = bank_start + SIZE_RAM_BANK -1;

	m_isa->install_bank(bank_start, 
			    bank_end, "banked_ram", &m_board_ram[INITIAL_RAM_BANK_NO * SIZE_RAM_BANK]);

	// Scratch RAM has a seperate space never banked out. Reserve an extra space -
	bank_start = bank_end + 1;
	bank_end = bank_start + SIZE_RAM_BANK -1;

	m_isa->install_bank(bank_start, 
                    bank_end , "banked_ram", &m_board_ram[SCRATCH_RAM_BANK_NO * SIZE_RAM_BANK]);

	// Protect entire bank to deter Himem.sys from detecting free memory - 
	m_isa->writeprotect_bank(bank_start, bank_end);

	// If only address 0-3 is covered and port changes occur, Unfreeze hangs (one cause of several)
	if( ioport("DEBUG")->read()) // [DEBUG]: cover complete I-O range from 0280 - 02b3. 
	{	
		io_port = 0x280;
		io_port_end = io_port + 0x33;
	}
	else
	{	// cover only offset 0 - 3 (+ io_port)
		io_port_end = io_port + 3;
	}
	m_isa->install_device(io_port, io_port_end, read8_delegate(FUNC(isa8_areplay_device::ar_read), this), write8_delegate(FUNC(isa8_areplay_device::ar_write), this));

	if(dest[0]==0x55) // if 'bootable' signature present...
	{
		if ( (ioport("FREEZE_BUTTON")->read() > 0) || (ioport("SLOMO_SWITCH")->read() > 0) )
			popmessage("FREEZE and SLOMO should be disabled before startup!\n[Action Replay]");
		else
			popmessage("Action Replay %1.1f from %x to %x / PORT %x - %x/ IRQ %x", 4.8f + (0.2f * ioport("COMPATIBILITY")->read()),
						m_current_rom_start, bank_end, 
						io_port, io_port_end, 
						m_current_irq_selected 
				   );
	}
}


// ----------- ONE READ REGISTER (bits must match when probing for ports. Expected value of bit 7 varies with firmware)
// bit 0 : (unkown). Must be 1 at startup 
// bit 1 : FREEZE BUTTON (if bit is 0 then 1 is poked to scratch RAM offset + 0xD0 on firmware version 5) * active low *
// bit 2 : (unkown). Must be 0 at startup 
// bit 3 : ACTION_REPLAY_IRQ. Must be 0 at startup!			  				  * active high *
// bit 4 : (unknown). Must be 1 at startup 
// bit 5 : (unknown). Must be 0 at startup 
// bit 6 : SLOW MOTION SWITCH. Must be 0 at startup							  * active high *
// bit 7 : (unknown). Must be 1 at startup of V5.0 firmware and 0 at startup of V4.8. 
READ8_MEMBER(isa8_areplay_device::ar_read)
{
	offset = offset & 1;

	int data = 1 | 16; // unknown bits 0 and 4 must be 1 at all times

	if ( ioport("FREEZE_BUTTON")->read() == 0 ) // * active low *  [bit 1]
		data |= FREEZE_BUTTON_BIT;

	if (m_is_heartbeat_present) 		 // * active high * [bit 3] 
	{	
		force_irq_to(false); // sets 'm_is_heartbeat_present' to false
		data |= ACTION_REPLAY_IRQ; // status bit needed for sensing of IRQ (1 = AR activity) 
	}

	if (ioport("SLOMO_SWITCH")->read() > 0) // ? active high ?  [bit 6]  0x40
		data |= SLOMO_SWITCH_BIT;

	if (ioport("COMPATIBILITY")->read() > 0)
		data |= 0x80;  // V5.0 'port sensing' appears to need it

	return data;
}

// ----------- TWO WRITE EGISTERS :

// --- WRITE PORT #1: [ROM] bank select (always in 8 K chunks; $1FFF).   
// Used to communicate with the AM29010 flash chip (access patterns: see datasheet)

// bit 0 : (set to 1 by flash routine, else 0) WRITE ENABLE ?
// bit 1 : (unknown); leftover bits from address?   Usually 0.

// bit 2 : A14  [ bits 2 - 4 select one 8 K bank in 128 K ROM space ]
// bit 3 : A15  [ bits 2 - 4 select one 8 K bank in 128 K ROM space ]
// bit 4 : A16  [ bits 2 - 4 select one 8 K bank in 128 K ROM space ]
// bit 5 : A17. Usually 0. Sense AM29010 flash 'reset' (0xf0) ?   

// bit 6 : (unknown); leftover bits from address?   Usually 0.
// bit 7 : LED (bit inverted by firmware when Action Replay is ready)

// --------------------------------------------------------------------------------
// --- WRITE PORT #2: [RAM] bank select

// bits 0...4 : [RAM] bank select (5 bits address 1 out of 32) pages of 4 K size.
// bits 5 + 6 : (unknown)
// Bit 7 : INTERRUPT ENABLE * active high *. Needed for IRQ sensing (pokes data 0x00, then 0x80)
// 			  Bit 7 = 0 : "disable interrupts" 
//			  Bit 7 = 1 : "enable interrupts "

WRITE8_MEMBER(isa8_areplay_device::ar_write)
{
   uint32_t bank_start;
   uint32_t bank_end;

   static bool old_irq_state;
   static bool old_led_state;

   static uint16_t old_tsr_start, old_bank, old_port;
   uint16_t tsr_start, bank, port;

   // Debug freezes which crash the host (usually because start addresses no longer match):
   if( ioport("DEBUG")->read())
   {	
		uint8_t *scratch = machine().root_device().memregion("isa")->base() + m_current_rom_start - 0xc0000 + 3 * SIZE_RAM_BANK;
		memcpy(&m_post_mortem_ram[0], scratch, 0x20); // make post mortem dump 

		tsr_start= m_post_mortem_ram[0x12] | (m_post_mortem_ram[0x13] << 8);
		bank   = m_post_mortem_ram[7];
		port   = m_post_mortem_ram[0] | (m_post_mortem_ram[1] << 8); // locations valid for 4.8

		if( (bank != old_bank ) || 
		    (tsr_start != old_tsr_start) ||
		    (port != old_port)
		  )
		{	
			if(tsr_start > 0)
				popmessage("Tsr start address %x\n[Action Replay]",tsr_start);

			if( (old_tsr_start > 0) && 
			    (old_bank > 0) && 
			    (old_port > 0)
			   )
			{
				popmessage("Change detected: OFFSET: %x -> %x\nROM BANK: %x -> %x...\nPORT %x -> %x\n[Action Replay]", 
					old_tsr_start, tsr_start,
					old_bank,   bank, 
					old_port,   port
					  );
			}

			old_tsr_start = tsr_start;
			old_bank = bank;
			old_port = port;
	  }
   }

   offset = offset & 1;

   switch(offset)
   {
	// ****************
	//  WRITE PORT #1
	// ****************
	case 0:  // (EEP)ROM bank select ( 1FFF = 8 K chunks).  Range: 0...15 (allows 128 K)

		m_write_port1 = data;

		bank_end = m_current_rom_start + SIZE_ROM_WINDOW - 1;
		if(data & WRITE_ENABLE_BIT) // detect write to EEPROM area
		{
			m_isa->unmap_bank(m_current_rom_start, bank_end);
			m_isa->install_bank(m_current_rom_start, bank_end,
				    "banked_flash", &m_banked_flash[ 16 * SIZE_ROM_WINDOW ]
				          );

			if(data == 0x01 ) // attempt to query manufacturer ID ?
			{
				popmessage("EEPROM updates are unemulated!\n[Action Replay]");
				m_isa->install_bank(m_current_rom_start, bank_end,
					    "banked_flash", &m_banked_flash[ m_current_rom_page * SIZE_ROM_WINDOW ]
					 	   );
			}
		}
		else
		{       // ROM PAGE 0 * IS * USED. MSB BITS (11000011) ARE ALWAYS ZERO:
			m_current_rom_page = (data & 0x3c) >> 2; // bits 2...5 

			m_isa->unmap_bank(m_current_rom_start, bank_end);
			m_isa->install_bank(m_current_rom_start, bank_end,
					    "banked_flash", &m_banked_flash[ m_current_rom_page * SIZE_ROM_WINDOW ]
				           );

			m_isa->writeprotect_bank(m_current_rom_start, bank_end); // *** write protect ROM bank ****
		}

		if( ((data & LED_ENABLE_BIT) > 0) && (old_led_state == false) )
				if( ioport("DEBUG")->read() != 0 ) printf(" [Action Replay LED 0 > 1] ");

		if( ((data & LED_ENABLE_BIT) == 0) && (old_led_state == true) )
				if( ioport("DEBUG")->read() != 0 ) printf(" [Action Replay LED 1 > 0] ");

		old_led_state = (data & LED_ENABLE_BIT) ? true : false; // keep position of statement 

		break; // (end of) case 0

	// ****************
	//  WRITE PORT #2
	// ****************
	case 1:  // RAM bank select ( FFF = 4K size). Range: 0...31 (128 K possible; amount probed by firmware)

		 if(old_irq_state == false) // Trigger IRQ only if 0 => 1 transition 
		 {
                  if( ((data & PORT2_IRQ_ENABLE) > 0 )  )
		  {
		     if(m_timer_fired == true)
		     {	
			if(m_is_heartbeat_present == false) 
			   force_irq_to(true);
		     } 
		  }
		 } // old irq state was ZERO.  
		
		if( (data & PORT2_IRQ_ENABLE) == 0 ) // driver reactivates stuck IRQ by forcing the bit to 0, then 1
			force_irq_to(false);

		old_irq_state = (data & PORT2_IRQ_ENABLE) ? true : false; // keep position of statement 

		bank_start = m_current_rom_start + SIZE_ROM_WINDOW;
		bank_end   = bank_start + SIZE_RAM_BANK - 1;

		m_isa->unmap_bank(bank_start, bank_end);

		m_current_RAM_PAGE = data & 0x1f; // Bits 0..4 

		m_isa->install_bank(bank_start, bank_end , 
				     "banked_ram", &m_board_ram[ SIZE_RAM_BANK * m_current_RAM_PAGE ]
			           );
		break; // (end of) case 1 : RAM bank 
    }
}

void isa8_areplay_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	switch (tid)
	{
		default:

if( ioport("DEBUG")->read() != 0 )
	printf("\nONE SHOT  **** stopped ****!");
			m_timer_fired = true;

			force_irq_to(false);
			break; 

	} // switch (timer ID)
}


// Subroutine 
// => side effect on global variable: 	m_is_heartbeat_present = flag; 

void isa8_areplay_device::force_irq_to(bool flag)
{	
	m_is_heartbeat_present = flag; // flag indicates the IRQ is generated by Action Replay

	if(flag == false)
		one_shot_timer->adjust(attotime::never); 
	  else
		one_shot_timer->adjust(attotime::from_msec(ONE_SHOT_TIMER_DELAY_MS)); 
	raise_processor_interrupt(m_current_irq_selected, flag);
}

TIMER_DEVICE_CALLBACK_MEMBER(isa8_areplay_device::heartbeat_timer)
{
	if(!m_is_heartbeat_present)
	{	
		m_timer_fired = false;

		force_irq_to(true); // side effect: m_is_heartbeat_present = true
	}
}

void isa8_areplay_device::raise_processor_interrupt(int ref, bool state)
{
			switch(ref)
	      		{
			case 2:
				m_isa->irq2_w(state ? ASSERT_LINE : CLEAR_LINE);
				break;
	
			case 3:
				m_isa->irq3_w(state ? ASSERT_LINE : CLEAR_LINE);
				break;

			case 4:
				m_isa->irq4_w(state ? ASSERT_LINE : CLEAR_LINE);
				break;

			case 5:
				m_isa->irq5_w(state ? ASSERT_LINE : CLEAR_LINE);
				break;

			case 6:
				m_isa->irq6_w(state ? ASSERT_LINE : CLEAR_LINE);
				break;

			case 7:
				m_isa->irq7_w(state ? ASSERT_LINE : CLEAR_LINE);
				break;
			}
}

static INPUT_PORTS_START( areplay_dsw )
	PORT_START("FREEZE_BUTTON")
	PORT_DIPNAME(0x01, 0x00, "Freeze (push to activate Action Replay)") PORT_IMPULSE(4)
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))

	PORT_START("SLOMO_SWITCH")
	PORT_DIPNAME(0x01, 0x00, "Slow motion switch (switch OFF at startup)") PORT_TOGGLE
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))

	PORT_START("ROM_ADDRESS")
	PORT_DIPNAME( 0xff, 0x00, "16K ROM bank (disable BIOS shadowing; exclude in EMM386)")
	PORT_DIPSETTING(    0, "C8000 - CBFFF (at times occupied by graphics o.HD contr.)" )
	PORT_DIPSETTING(    1, "CC000 - CFFFF" ) 
	PORT_DIPSETTING(    2, "D0000 - D3FFF" )
	PORT_DIPSETTING(    3, "D2000 - D5FFF" ) 
	PORT_DIPSETTING(    4, "D4000 - D7FFF" ) 
	PORT_DIPSETTING(    5, "D8000 - DBFFF" ) 
	PORT_DIPSETTING(    6, "DC000 - DFFFF" )  

	PORT_START("PORT")
	PORT_DIPNAME( 0xff, 0x00, "I/O port")
	PORT_DIPSETTING(    0, "0x280" ) 
	PORT_DIPSETTING(    1, "0x290" )
	PORT_DIPSETTING(    2, "0x2A0" )
	PORT_DIPSETTING(    3, "0x2B0" )

	PORT_START("IRQ")
	PORT_DIPNAME( 0xff, 5, "Interrupt number")
	PORT_DIPSETTING(    2, "2 (sometimes usable; timer irq / occupied by 8259A)" ) 
	PORT_DIPSETTING(    3, "3 (free if not occupied by COM-2 or COM-4)" )
	PORT_DIPSETTING(    4, "4 (do not use if COM-1 or COM-3 present)" )
	PORT_DIPSETTING(    5, "5 (recommended; usually LPT-2)" )
	PORT_DIPSETTING(    6, "6 (do not use if FDC controller present)" )
	PORT_DIPSETTING(    7, "7 (recommended; usually LPT-1)" )

	PORT_START("COMPATIBILITY")
	PORT_DIPNAME(0xff, 0x00, "Hardware compatibility") PORT_TOGGLE
	PORT_DIPSETTING(    0, "4.8 (AMD flash)" ) 
	PORT_DIPSETTING(    1, "5.0 (AMD flash w.different I/O)" )

	PORT_START("DEBUG")
	PORT_DIPNAME(0xff, 0x00, "* DEBUGGING ON *") PORT_TOGGLE
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
INPUT_PORTS_END

//-------------------------------------------------
//  isa8_areplay_device - constructor
//-------------------------------------------------
ioport_constructor isa8_areplay_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( areplay_dsw );
}
