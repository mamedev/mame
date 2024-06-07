// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    IVL Technologies Karaoke systems

    KaraokeTV Star (c) IVL Technologies

    licensed out as

    KaraokeStation (Japan) (c) Bandai
    Easy Karaoke Groove Station (UK) (c) Easy Karaoke
    KaraokeMicro Star (France) (c) Lexibook

    -------------------------------------

    Easy Karaoke uses

    Clarity 4.3 ARM
    SVI1186
    NV0165  0317
    Sound Vision Inc.

    an overview for 4.1 and 4.2 can be found at
    http://web.archive.org/web/20031212120255fw_/http://www.soundvisioninc.com/OEMProducts/C4datasheet072401.pdf
    Amusingly this datasheet advertises 'MAME Game emulation' as one of the capabilities despite the chip
    clocking in at only 72Mhz

    Support chip is

    IVL
    Technologies
    ICS0253R1.0
    UA1068ABK-RD
    0327 A01491F

    RAM chip is

    IC42S16400-7T

    ROM is

    IVL
    Technologies
    ICS0303-B
    (c)1985-1986
    3415BAI THAI

    --------------

    Cartridges contain:

    1x MX 29LV040TC-90 (Flash ROM)

    1x HC573A

    1x ICSI IC89LV52A-24PQ (80C52 MCU with 8KBytes Flash memory, can be read protected)

    presumably manages a serial protocol to send data to the main unit


    -----------------

    Lexibook's KaraokeMicro Star version uses

    Clarity 4.1 ARM
    SV11180
    NV0093  0246
    Sound Vision Inc.

    ------------------

    KaraokeTV Star also uses the Clarity 4.1 but with "JVR043  0225" numbering

    Packaging also shows 'On-Key Karaoke' logo on box, maybe this is the original US / Canada product name?

    "Karaoke TV Star" appears to be a US product using this technology - advertises 50 built in songs, but
    also a downloadable service.  It has a 2002 date on the box / product.  Another version offers 35 songs

*******************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class ivl_karaoke_state : public driver_device
{
public:
	ivl_karaoke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void ivl_karaoke_base(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint32_t a000004_r();

	void arm_map(address_map &map);
};

class easy_karaoke_cartslot_state : public ivl_karaoke_state
{
public:
	easy_karaoke_cartslot_state(const machine_config &mconfig, device_type type, const char *tag)
		: ivl_karaoke_state(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
	{ }

	void easy_karaoke(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
};

uint32_t ivl_karaoke_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void ivl_karaoke_state::machine_start()
{

}

void easy_karaoke_cartslot_state::machine_start()
{
	ivl_karaoke_state::machine_start();

	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		m_cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	}
}

void ivl_karaoke_state::machine_reset()
{
	m_maincpu->set_state_int(ARM7_R15, 0x04000000);
}

DEVICE_IMAGE_LOAD_MEMBER(easy_karaoke_cartslot_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( ivl_karaoke )
INPUT_PORTS_END

uint32_t ivl_karaoke_state::a000004_r()
{
	return machine().rand();
}

void ivl_karaoke_state::arm_map(address_map &map)
{
	map(0x00000000, 0x007fffff).ram();
	map(0x04000000, 0x047fffff).rom().region("maincpu", 0);
	map(0x0a000004, 0x0a000007).r(FUNC(ivl_karaoke_state::a000004_r));
}


void ivl_karaoke_state::ivl_karaoke_base(machine_config &config)
{
	ARM9(config, m_maincpu, 72000000); // ARM 720 core
	m_maincpu->set_addrmap(AS_PROGRAM, &ivl_karaoke_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(ivl_karaoke_state::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

void easy_karaoke_cartslot_state::easy_karaoke(machine_config &config)
{
	ivl_karaoke_base(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "easy_karaoke_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(easy_karaoke_cartslot_state::cart_load));
	SOFTWARE_LIST(config, "cart_list").set_original("easy_karaoke_cart");
}

/*
The 'karatvst' set has the following 50 songs built in, there don't appear to be any downloaded songs in this NAND dump

ABC                                         Jackson 5
All I Have To Give                          Backstreet Boys
Always On My Mind                           Elvis Presley
America The Beautiful                       Standard
Baby Love                                   Supremes
...Baby One More Time                       Britney Spears
Born To Make You Happy                      Britney Spears
Brick House                                 The Commodores
Bye Bye Bye                                 N'Sync
Dancing Queen                               Abba
Don't Let Me Get Me                         pink
Drive (For Daddy Gene)                      Alan Jackson
Fallin'                                     Alicia Keys
Girlfriend                                  N'Sync
Goodbye Earl                                Dixie Chicks
Hit 'Em Up Style                            Blu Cantrell
I Believe I Can Fly                         R. Kelly
I Heard It Through the Grapevine            Marvin Gaye
I Should Be Sleeping                        Emerson Drive
I Wanna Know                                Joe
I Want It That Way                          Backstreet Boys
If You're Gone                              Matchbox 20
It's A Great Day To Be Alive                Travis Tritt
Lady Marmalade                              Christina Aguilera
Love Shack                                  B52's
Me And Bobby McGee                          Kris Kristofferson
My Girl                                     Temptations
My Guy                                      Mary Wells
New York New York                           Frank Sinatra
No More Drama                               Mary J. Blige
One Fine Day                                Chiffons
Oops...I Did It Again                       Britney Spears
Over The Rainbow                            Judy Garland
Overprotected                               Britney Spears
Stand By Your Man                           Tammy Wynette
Star Spangled Banner                        Public Domain
Stop In The Name Of Love                    Supremes
Stronger                                    Britney Spears
Super Freak                                 Rick James
Superman                                    Five For Fighting
That's The Way (I Like It)                  KC And The Sunshine Band
The Greatest Love Of All                    Whitney Houston
The Loco-Motion                             Kylie Minogue
The One                                     Backstreet Boys
There Is No Arizona                         Jamie O'Neal
We Wish You A Merry Xmas                    Traditional
What's Going On                             Marvin Gaye
Wild Thing                                  The Troggs
Wrapped Around                              Brad Paisley
You Can't Hurry Love                        The Supremes

*/
ROM_START( karatvst )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "karaoke37vf010.bin", 0x000000, 0x20000, CRC(9d3020e4) SHA1(bce5d42ecff88b310a43599c9e47cba920c6b6e1) ) // bootloader

	ROM_REGION( 0x840000, "nand", ROMREGION_ERASEFF ) // NAND with main program, graphics, built in songs (and potentially user downloads)
	ROM_LOAD( "karaoketc58v64bft.bin", 0x000000, 0x840000, CRC(8cf42f20) SHA1(fae09ab08035e8c894fe00dcb23746ea78391d7f) )
ROM_END

/*

song list is unknown for the Japanese version as the titles aren't stored as ASCII in the flash ROM

*/

ROM_START( bkarast )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "sst37vf010.u9", 0x000000, 0x20000, CRC(a7c69fbb) SHA1(28ef698e63e76d9461b71649e4ee9c8f252f82e2) ) // bootloader

	ROM_REGION( 0x840000, "nand", ROMREGION_ERASEFF ) // NAND with main program, graphics, built in songs (and potentially user downloads)
	ROM_LOAD( "tc58v64bft_withspare.u8", 0x000000, 0x840000, CRC(7770996a) SHA1(cb6df756f88c1f5ff4bd202e7758586c03aff00e) )
ROM_END

/*
The 'easykara' set has the following 10 songs built in.

One Step Closer                             S Club Juniors
S Club Party                                S Club 7
Automatic High                              S Club Juniors
Don't Stop Movin'                           S Club 7
Get the Party Started                       Pink
Feel                                        Robbie Williams
Complicated                                 Avril Lavigne
One love                                    Blue
If you're not the one                       Daniel Bedingfield
Sound of the Underground                    Girls Aloud

*/
ROM_START( easykara )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ics0303-b.bin", 0x000000, 0x400000, CRC(43d86ae8) SHA1(219dcbf72b92d1b7e00f78f237194ab47dc08f1b) )
ROM_END

/*
The 'karams' set has the following 50 songs built in.

...Baby One More Time                       Britney Spears
Alexandrie Alexandra                        Claude François
All By Myself                               Celine Dion
Allumer le feu                              Johnny Hallyday
Alors regarde                               Patrick Bruel
Always On My Mind                           Elvis Presley (R)
Au soleil                                   Jennifer
Auprès de ma blonde                         Enfant
Believe                                     Cher
Bye Bye Bye                                 'N Sync
Can't Get You Out Of My Head                Kylie Minogue
Cette année là                              Claude François
Don't Let Me Get Me                         Pink
Déshabillez-moi                             Juliette Greco
Elle te rend dingue                         Nuttea
Embrasse-moi idiot                          Forban
Fallin'                                     Alicia Keys
Fame                                        Irene Cara
Femmes je vous aime                         Julien Clerc
Frère Jacques                               Enfant
I Love Rock and Roll                        Joan Jett
I Will Survive                              Gloria Gaynor
I'll Be There                               Mariah Carey
Il était un petit navire                    Enfant
It's Raining Men                            Geri Halliwell
Juste quelqu'un de bien                     Enzo Enzo
La Bohème                                   Charles Aznavour
La Cucaracha                                Standard
La Marseillaise                             Popular
La musique                                  Star Academy 1
Lady Marmalade                              Christina Aguilera
Laissons entrer le soleil                   A la recherche de la Nouvelle Star
Le bon roi Dagobert                         Enfant
Le pénitencier                              Johnny Halliday
London Bridge                               Children
Magnolias forever                           Claude François
My Girl                                     Temptations
New York New York                           Frank Sinatra
Noir c'est noir                             Johnny Hallyday
Oops!...I Did It Again                      Britney Spears
Pour le plaisir                             Herbert Léonard
Qui est l'exemple?                          Rohf
Silent Night                                Christmas
That's The Way (I Like It)                  KC And The Sunshine Band
That's The Way It Is                        Celine Dion
The Loco-Motion                             Kylie Minogue
Toute seule                                 Lorie
Vieille canaille                            Gainsbourg
We Wish You A Merry Christmas               Standard
When The Saints Go Marchin' In              Louis Armstrong

*/

ROM_START( karams )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ics0300-a.u9", 0x000000, 0x800000, CRC(32a7a429) SHA1(ed219bc9201b45f67c5e7dbe3fb3db70823c59f0) )
ROM_END

} // anonymous namespace

// This is the original US release, there's no cartridge slot, but it has a NAND Flash inside, and in addition to 50 built-in songs, advertises
// use of a (now defunct) www.onkeysongs.com service for downloading additional songs to the microphone via bundled PC software.
CONS( 2002, karatvst,      0,              0,      ivl_karaoke_base, ivl_karaoke, ivl_karaoke_state, empty_init, "IVL Technologies", "KaraokeTV Star (US, with 50 songs)", MACHINE_IS_SKELETON )
// Bandai's Japanese release also lacks a cartridge slot, relying on downloads for additional songs. It also comes with a CD containing the PC-side software.  The external microphone design differs slightly.
CONS( 2002, bkarast,       karatvst,       0,      ivl_karaoke_base, ivl_karaoke, ivl_karaoke_state, empty_init, "IVL Technologies (Bandai license)", "Karaoke Station (Japan)", MACHINE_IS_SKELETON )


// There is also a 35 song US version
// The "Memorex Star Singer Karaoke / MKS4001" is also made by IVL and boasts 50 built in songs, the casing is different, so it could differ from the standard version.

// The European releases take cartridges rather than relying on a download service
CONS( 2004, easykara,      karatvst,       0,      easy_karaoke, ivl_karaoke, easy_karaoke_cartslot_state, empty_init, "IVL Technologies (Easy Karaoke license)", "Easy Karaoke Groove Station (UK)", MACHINE_IS_SKELETON )
CONS( 2003, karams,        karatvst,       0,      easy_karaoke, ivl_karaoke, easy_karaoke_cartslot_state, empty_init, "IVL Technologies (Lexibook license)",     "KaraokeMicro Star (France)", MACHINE_IS_SKELETON )
