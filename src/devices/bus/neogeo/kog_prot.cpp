// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#include "emu.h"
#include "kog_prot.h"



extern const device_type KOG_PROT = &device_creator<kog_prot_device>;


kog_prot_device::kog_prot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, KOG_PROT, "NeoGeo Protection (King of Gladiator)", tag, owner, clock, "kog_prot", __FILE__),
	m_jumper(*this, "JUMPER")
{
}


void kog_prot_device::device_start()
{
}

void kog_prot_device::device_reset()
{
}

READ16_MEMBER(kog_prot_device::read_jumper)
{
	return ioport("JUMPER")->read();
}

void kog_prot_device::kog_install_protection(cpu_device* maincpu)
{
	/* overlay cartridge ROM */
	maincpu->space(AS_PROGRAM).install_read_handler(0x0ffffe, 0x0fffff, read16_delegate(FUNC(kog_prot_device::read_jumper), this));
}


/* The King of Gladiator (The King of Fighters '97 bootleg) */


/* The protection patching here may be incomplete
   Thanks to Razoola for the info */

void kog_prot_device::kog_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	/* the protection chip does some *very* strange things to the rom */
	UINT8 *src = cpurom;
	dynamic_buffer dst( 0x600000 );
	UINT16 *rom = (UINT16 *)cpurom;
	int i;
	static const int sec[] = { 0x3, 0x8, 0x7, 0xC, 0x1, 0xA, 0x6, 0xD };

	for (i = 0; i < 8; i++){
		memcpy (&dst[i * 0x20000], src + sec[i] * 0x20000, 0x20000);
	}

	memcpy (&dst[0x0007A6], src + 0x0407A6, 0x000006);
	memcpy (&dst[0x0007C6], src + 0x0407C6, 0x000006);
	memcpy (&dst[0x0007E6], src + 0x0407E6, 0x000006);
	memcpy (&dst[0x090000], src + 0x040000, 0x004000);
	memcpy (&dst[0x100000], src + 0x200000, 0x400000);
	memcpy (src, &dst[0], 0x600000);

	for (i = 0x90000/2; i < 0x94000/2; i++){
		if (((rom[i]&0xFFBF) == 0x4EB9 || rom[i] == 0x43F9) && !rom[i + 1])
			rom[i + 1] = 0x0009;

		if (rom[i] == 0x4EB8)
			rom[i] = 0x6100;
	}

	rom[0x007A8/2] = 0x0009;
	rom[0x007C8/2] = 0x0009;
	rom[0x007E8/2] = 0x0009;
	rom[0x93408/2] = 0xF168;
	rom[0x9340C/2] = 0xFB7A;
	rom[0x924AC/2] = 0x0009;
	rom[0x9251C/2] = 0x0009;
	rom[0x93966/2] = 0xFFDA;
	rom[0x93974/2] = 0xFFCC;
	rom[0x93982/2] = 0xFFBE;
	rom[0x93990/2] = 0xFFB0;
	rom[0x9399E/2] = 0xFFA2;
	rom[0x939AC/2] = 0xFF94;
	rom[0x939BA/2] = 0xFF86;
	rom[0x939C8/2] = 0xFF78;
	rom[0x939D4/2] = 0xFA5C;
	rom[0x939E0/2] = 0xFA50;
	rom[0x939EC/2] = 0xFA44;
	rom[0x939F8/2] = 0xFA38;
	rom[0x93A04/2] = 0xFA2C;
	rom[0x93A10/2] = 0xFA20;
	rom[0x93A1C/2] = 0xFA14;
	rom[0x93A28/2] = 0xFA08;
	rom[0x93A34/2] = 0xF9FC;
	rom[0x93A40/2] = 0xF9F0;
	rom[0x93A4C/2] = 0xFD14;
	rom[0x93A58/2] = 0xFD08;
	rom[0x93A66/2] = 0xF9CA;
	rom[0x93A72/2] = 0xF9BE;

}


static INPUT_PORTS_START( kog )
	/* a jumper on the pcb overlays a ROM address, very strange but that's how it works. */
	PORT_START("JUMPER")
	PORT_DIPNAME( 0x0001, 0x0001, "Title Language" ) PORT_DIPLOCATION("CART-JUMPER:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, "Non-English" )
	PORT_BIT( 0x00fe, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor kog_prot_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( kog );
}
