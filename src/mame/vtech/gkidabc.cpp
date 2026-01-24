// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************************

    Genius KID ABC Fan
    Mis Primeras Lecciones
    Genius Junior Profi

    Other known undumped international versions:
    - Smart Start Elite (English version of Genius Junior Profi / Mis Primeras Lecciones)
	
	Possibly on similar hardware:
	- Animal Friends Toddler Laptop
	- Bob the Builder Laptop
	- Bluey - Game Time Laptop
	- Cars - Lightning McQueen Learning Laptop
	- Cars 2 - Mater Spy Mission Laptop
	- Cars 2 - Hook Laptop (German version of the Cars 2 Mater laptop)
	- Chuggington Traintastic Laptop
	- Chuggington - Laptop (German version)
	- Disney Planes - Learning Laptop
	- Disney Cinderella - Carriage Laptop
	- Disney Princess - Magic Wand Laptop
	- Dora the Explorer - Dora's Laptop (English Dora-branded version of Mr. Laptop)
	- Dora The Explorer - Carnival Time Laptop/Learning Laptop
	- DORA - More to Explore Learning Laptop
	- Girl Fun PC
	- Handy Manny's Construction Laptop
	- Meister Mannys Werkzeugkiste - Laptop (German version of Handy Manny's Construction Laptop)
	- Cars: Genius Flash McQueen (French version of Cars Laptop)
	- Genius Bob l'éponge (French version of SpongeBob Laptop)
	- Genius Copine (French version of Tote and Go Laptop 2009)
	- Genius Dora (French version of Dora's Laptop)
	- Genius Ecriture (French version of Writing Fun Laptop)
	- Genius Ni Hao, Kai Lan (French version of Ni Hao Kai Lan Learning Laptop)
	- Genius Fun (French generic version of Dora's Laptop)
	- Genius Go Diego (French version of Go Diego Go Laptop)
	- Genius Malice (French version of Tote and Go Laptop Plus)
	- Genius Petits Einstein (French Version of Little Einsteins laptop)
	- Genius Récré (French Version of Letter Fun Laptop)
	- Ordinateur cendrillon Baguette magique (French version of Cinderella Carriage Laptop)
	- P'tit Genius (French version of the first tote and go laptop)
	- WallE Ordi Copain Interactif (French Version of WallE Laptop)
	- Jake & The Neverland Pirates - Treasure Hunt Learning Laptop
	- Laptop Deluxe
	- Laptop Voyager
	- Learning Laptop
	- Little Smart Bigtop Laptop
	- Little Smart Mouseland Laptop
	- Little Einsteins - Blast-Off Learning Laptop
	- Kleine Einsteins - Laptop (German version of the Little Einsteins laptop)
	- Lil' SmartTop
	- Marvel Super Hero Squad - Learning Laptop
	- Mickey Mouse Clubhouse - Mousekadoer Laptop
	- Micky Maus Wunderhaus - Laptop (German Version of Mickey Mouse Clubhouse Mousekadoer Laptop)
	- Non-Stop Girl
	- Peppa Pig - Play Smart Laptop (US and UK versions has the same ROM)
	- Play Smart Preschool Laptop
	- Smart Start Learning Laptop
	- Smartwave PC
	- Sofia the First - Learning Laptop
	- SpongeBob Laptop (US and UK versions has the same ROM) (there are German and Dutch versions with the same name, with a Spanish version [El Ordenador de Bob Esponja] which is unconfirmed)
	- Thomas & Friends - Learn & Explore Laptop
	- Tree Fu Tom - Learn & Play Laptop
	- Tote & Go Laptop (2002)
	- Tote & Go Laptop Plus
	- Tote 'n' Go Laptop (2009)
	- Tote & Go Laptop Web
	- Tote & Go Laptop (2014)
	- Toy Story 3 - Buzz Lightyear Star Command Laptop (US and UK versions has the same ROM)
	- Two Smart Laptop
	- Wall.E Learning Laptop
	- Winnie the Pooh - Press 'n Play Laptop
	- Winnie Puuh ABC Laptop (German Version of the Winnie the Pooh Press 'n Play Laptop) (the Meine Freunde Tigger und Puuh branded and the 2010 version seems identical ROM)
	- World Pad PC
	- Yo Gabba Gabba! Learning Laptop
	
    TODO: identify CPU type (16-bit processor internally, but with 8-bit external bus?)
    It might be that the dumped ROMs contain no actual code, only graphics data and
    sound samples.

***************************************************************************************/

#include "emu.h"


namespace {

class gkidabc_state : public driver_device
{
public:
	gkidabc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void gkidabc(machine_config &config);
};


static INPUT_PORTS_START(gkidabc)
INPUT_PORTS_END

void gkidabc_state::gkidabc(machine_config &config)
{
}


ROM_START(gkidabc)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("27-5730-00.bin", 0x00000, 0x20000, CRC(64664708) SHA1(74212c2dec1caa41dbc933b50f857904a8ac623b))
ROM_END

ROM_START(miprimlec)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("27-5482-01.u1", 0x00000, 0x20000, CRC(83aa655b) SHA1(5d7b03f0ff2836e228da77676df03854f87edd26))

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD( "csm10150an.u3", 0x0000, 0x2000, NO_DUMP ) // TSP50C10 (8K bytes of ROM) labeled "67ACLKT VIDEO TECH CSM10150AN"
ROM_END

ROM_START(gjrprofi)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("27-5476-00.u1", 0x00000, 0x20000, CRC(ad1ec838) SHA1(0cf90c02762ace656191a38ae423a4fa0e7484f7))
ROM_END

} // anonymous namespace


COMP(1996, gkidabc,   0, 0, gkidabc, gkidabc, gkidabc_state, empty_init, "VTech", "Genius KID ABC Fan (Germany)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
COMP(1995, miprimlec, 0, 0, gkidabc, gkidabc, gkidabc_state, empty_init, "VTech", "Mis Primeras Lecciones (Spain)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
COMP(1995, gjrprofi,  miprimlec, 0, gkidabc, gkidabc, gkidabc_state, empty_init, "VTech", "Genius Junior Profi (Germany)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
