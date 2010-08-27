/******************************************************************************

    mamedriv.c

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    The list of all available drivers. Drivers have to be included here to be
    recognized by the executable.

    To save some typing, we use a hack here. This file is recursively #included
    twice, with different definitions of the DRIVER() macro. The first one
    declares external references to the drivers; the second one builds an array
    storing all the drivers.

******************************************************************************/

#include "emu.h"

#ifndef DRIVER_RECURSIVE

#define DRIVER_RECURSIVE

/* step 1: declare all external references */
#define DRIVER(NAME) GAME_EXTERN(NAME);
#include "mamedriv.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &GAME_NAME(NAME),
const game_driver * const drivers[] =
{
#include "mamedriv.c"
	0	/* end of array */
};

#else	/* DRIVER_RECURSIVE */

	/* "Pacman hardware" games */
	DRIVER( puckman )	/* (c) 1980 Namco */
	DRIVER( puckmana )	/* (c) 1980 Namco */
	DRIVER( puckmanf )	/* hack */
	DRIVER( puckmanh )	/* hack */
	DRIVER( pacman )	/* (c) 1980 Midway */
	DRIVER( pacmanf )	/* hack */
	DRIVER( puckmod )	/* (c) 1981 Namco */
	DRIVER( pacmod )	/* (c) 1981 Midway */
	DRIVER( newpuc2 )	/* hack */
	DRIVER( newpuc2b )	/* hack */
	DRIVER( newpuckx )	/* hack */
	DRIVER( pacheart )	/* hack */
	DRIVER( hangly )	/* hack */
	DRIVER( hangly2 )	/* hack */
	DRIVER( hangly3 )	/* hack */
	DRIVER( popeyeman )	/* hack */
	DRIVER( piranhah )	/* hack */
	DRIVER( abscam )	/* hack */
	DRIVER( crush )		/* (c) 1981 Kural Samno Electric Ltd */
	DRIVER( crushbl )	/* bootleg */
	DRIVER( crushbl2 )	/* bootleg 2*/
	DRIVER( crush2 )	/* (c) 1981 Kural Esco Electric Ltd - bootleg? */
	DRIVER( crush3 )	/* Kural Electric Ltd - bootleg? */
	DRIVER( crush4 )	/* Kural TWT */
	DRIVER( maketrax )	/* (c) 1981 Williams, high score table says KRL (fur Kural) */
	DRIVER( maketrxb )	/* (c) 1981 [Kural] (Williams license) */
	DRIVER( korosuke )	/* (c) 1981 Kural Electric */
	DRIVER( mbrush )	/* 1981 bootleg */
	DRIVER( paintrlr )	/* 1981 bootleg */
	DRIVER( crushs )	/* 1981 bootleg */
	DRIVER( pacplus )	/* (c) 1982 Namco (Midway license) */
	DRIVER( joyman )	/* hack */
	DRIVER( ctrpllrp )	/* hack */
	DRIVER( eyes )		/* (c) 1982 Digitrex Techstar + "Rockola presents" */
	DRIVER( eyes2 )		/* (c) 1982 Techstar + "Rockola presents" */
	DRIVER( eyeszac )	/* (c) 1982 Zaccaria / bootleg */
	DRIVER( mrtnt )		/* (c) 1983 Telko */
	DRIVER( gorkans )	/* (c) 1984 Techstar */
	DRIVER( eggor )		/* (c) 1983 Telko */
	DRIVER( jumpshot )	/* (c) 1985 Bally Midway */
	DRIVER( jumpshotp )	/* (c) 1985 Bally Midway */
	DRIVER( shootbul )	/* (c) 1985 Bally Midway */
	DRIVER( piranha )	/* GL */
	DRIVER( piranhao )	/* GL */
	DRIVER( nmouse )	/* (c) 1981 Amenip (Palcom Queen River) */
	DRIVER( nmouseb )	/* (c) 1981 Amenip Nova Games Ltd. */
	DRIVER( mspacman )	/* (c) 1981 Midway */ /* made by General Computer */
	DRIVER( mspacmnf )	/* hack */
	DRIVER( mspacmat )	/* hack */
	DRIVER( woodpeck )	/* (c) 1981 Amenip (Palcom Queen River) */
	DRIVER( woodpeca )	/* (c) 1981 Amenip Nova Games Ltd. */
	DRIVER( mspacmab )	/* bootleg */
	DRIVER( mspacmbe )	/* bootleg */
	DRIVER( pacgal )	/* hack */
	DRIVER( mspacpls )	/* hack */
	DRIVER( ponpoko )	/* (c) 1982 Sigma Ent. Inc. */
	DRIVER( ponpokov )	/* (c) 1982 Sigma Ent. Inc. + Venture Line license */
	DRIVER( lizwiz )	/* (c) 1985 Techstar + "Sunn presents" */
	DRIVER( alibaba )	/* (c) 1982 Sega */
	DRIVER( dremshpr )	/* (c) 1982 Sanritsu */
	DRIVER( vanvan )	/* (c) 1983 Sanritsu */
	DRIVER( vanvank )	/* (c) 1983 Karateco (bootleg?) */
	DRIVER( vanvanb )	/* (c) 1983 Karateco (bootleg?) */
	DRIVER( bwcasino )	/* (c) 1983 Epos Coropration */
	DRIVER( acitya )	/* (c) 1983 Epos Corporation */
	DRIVER( theglobp )	/* (c) 1983 Epos Corporation */
	DRIVER( sprglobp )	/* (c) 1983 Epos Corporation */
	DRIVER( sprglbpg )	/* German bootleg */
	DRIVER( beastf )	/* (c) 1984 Epos Corporation */
	DRIVER( rocktrv2 )	/* (c) 1986 Triumph Software Inc */
	DRIVER( bigbucks )	/* (c) 1986 Dynasoft Inc. */
	DRIVER( mschamp )	/* hack by RAYGLO MFG "Zola-Puc" (c) 1992/1993 */
	DRIVER( mschamps )	/* hack (hack of the Rayglo hack?) */
	DRIVER( cannonbp )	/* hack */

	/* S2650 Pacman Kits */
	DRIVER( drivfrcp )	/* (c) 1984 Shinkai Inc. (Magic Eletronics Inc. license) */
	DRIVER( 8bpm )		/* (c) 1985 Seatongrove Ltd (Magic Eletronics USA license) */
	DRIVER( porky )		/* (c) 1985 Shinkai Inc. (Magic Eletronics Inc. license) */

	DRIVER( pengo )		/* 834-0386 (c) 1982 Sega */
	DRIVER( pengo2 )	/* 834-0386 (c) 1982 Sega */
	DRIVER( pengo2u )	/* 834-0386 (c) 1982 Sega */
	DRIVER( pengo3u )	/* 834-0386 (c) 1982 Sega */
	DRIVER( pengo4 )	/* 834-0386 (c) 1982 Sega */
	DRIVER( pengob )	/* bootleg */
	DRIVER( penta )		/* bootleg */
	DRIVER( jrpacmbl )	/* bootleg on Pengo HW */

	DRIVER( jrpacman )	/* (c) 1983 Midway */

	/* Epos games */
	DRIVER( megadon )	/* (c) 1982 */
	DRIVER( catapult )	/* (c) 1982 */
	DRIVER( suprglob )	/* (c) 1983 */
	DRIVER( theglob )	/* (c) 1983 */
	DRIVER( theglob2 )	/* (c) 1983 */
	DRIVER( theglob3 )	/* (c) 1983 */
	DRIVER( igmo )		/* (c) 1984 */
	DRIVER( dealer )	/* (c) 198? */
	DRIVER( revenger )	/* (c) 1984 */

	/* "Galaxian hardware" games */
	DRIVER( galaxian )	/* (c) Namco */
	DRIVER( galaxiana )	/* (c) Namco */
	DRIVER( galaxianm )	/* (c) Midway */
	DRIVER( galaxianmo )/* (c) Midway */
	DRIVER( galaxiant )	/* (c) Taito */
	DRIVER( superg )	/* hack */
	DRIVER( galapx )	/* hack */
	DRIVER( moonaln )	/* [Nichibutsu] (Karateco license) or hack */
	DRIVER( galap1 )	/* hack */
	DRIVER( galap4 )	/* hack */
	DRIVER( galturbo )	/* hack */
	DRIVER( swarm )		/* hack */
	DRIVER( zerotime )	/* hack */
	DRIVER( starfght )	/* hack */
	DRIVER( galaxbsf )	/* hack */
	DRIVER( kamakazi3 )	/* Video Games (UK) Ltd., hack or bootleg? */
	DRIVER( tst_galx )	/* Galaxian Test ROM */
	DRIVER( gmgalax )	/* bootleg */
	DRIVER( pisces )	/* Subelectro */
	DRIVER( piscesb )	/* bootleg */
	DRIVER( omni )		/* bootleg */
	DRIVER( uniwars )	/* (c) Irem */
	DRIVER( gteikoku )	/* (c) Irem */
	DRIVER( gteikokb )	/* bootleg */
	DRIVER( gteikob2 )	/* bootleg */
	DRIVER( spacbatt )	/* bootleg */
	DRIVER( spacbat2 )	/* bootleg */
	DRIVER( spacempr )	/* bootleg */
	DRIVER( skyraidr )	/* bootleg */
	DRIVER( batman2 )	/* bootleg */
	DRIVER( warofbug )	/* (c) 1981 Armenia */
	DRIVER( warofbugg )	/* German Version */
	DRIVER( redufo )	/* (c) Artic */
	DRIVER( redufob )	/* bootleg */
	DRIVER( exodus )	/* Subelectro - bootleg? */
	DRIVER( streakng )	/* [1980] Shoei */
	DRIVER( pacmanbl )	/* bootleg */
	DRIVER( pacmanbla )	/* bootleg */
	DRIVER( phoenxp2 )	/* bootleg */
	DRIVER( atlantisb )	/* bootleg */
	DRIVER( tenspot )	/* (c) 1982 Thomas Automatics */
	DRIVER( devilfsg )	/* (c) 1984 Vision / Artic (bootleg?) */
	DRIVER( zigzag )	/* (c) 1982 LAX */
	DRIVER( zigzag2 )	/* (c) 1982 LAX */
	DRIVER( jumpbug )	/* (c) 1981 Rock-ola */
	DRIVER( jumpbugb )	/* (c) 1981 Sega */
	DRIVER( levers )	/* (c) 1983 Rock-ola */
	DRIVER( azurian )	/* (c) 1982 Rait Electronics Ltd */
	DRIVER( orbitron )	/* (c) 1982 Signatron USA (Arcade Tv Game List - P.160, Left, 22 from top) */
	DRIVER( mooncrgx )	/* bootleg */
	DRIVER( mooncrst )	/* (c) 1980 Nichibutsu */
	DRIVER( mooncrstu )	/* (c) 1980 Nichibutsu USA */
	DRIVER( mooncrsto )	/* (c) 1980 Nichibutsu */
	DRIVER( mooncrstg )	/* (c) 1980 Gremlin */
	DRIVER( mooncrstuk )/* (c) 1980 Nichibutsu UK */
	DRIVER( smooncrs )	/* Gremlin */
	DRIVER( mooncmw )	/* bootleg */
	DRIVER( mooncrsb )	/* bootleg */
	DRIVER( mooncrs2 )	/* bootleg */
	DRIVER( spcdrag )	/* bootleg */
	DRIVER( stera )		/* bootleg */
	DRIVER( spcdraga )	/* bootleg */
	DRIVER( mooncrs3 )	/* bootleg */
	DRIVER( fantazia )	/* bootleg */
	DRIVER( moonqsr )	/* (c) 1980 Nichibutsu */
	DRIVER( mshuttle )	/* (c) 1981 Nichibutsu */
	DRIVER( mshuttlej )	/* (c) 1981 Nichibutsu */
	DRIVER( mshuttlej2 )/* (c) 1981 Nichibutsu */
	DRIVER( moonal2 )	/* [1980] Nichibutsu */
	DRIVER( moonal2b )	/* [1980] Nichibutsu */
	DRIVER( supergx )	/* [1980] Nichibutsu */
	DRIVER( eagle )		/* [1980] Centuri */
	DRIVER( eagle2 )	/* [1980] Centuri */
	DRIVER( eagle3 )	/* [1980] Centuri */
	DRIVER( spctbird )	/* (c) Fortrek */
	DRIVER( skybase )	/* (c) 1982 Omori Electric Co., Ltd. */
	DRIVER( checkman )	/* (c) 1982 Zilec-Zenitone */
	DRIVER( checkmanj )	/* (c) 1982 Jaleco (Zenitone/Zilec in ROM CM4, and the programmer names) */
	DRIVER( dingo )		/* (c) 1983 Ashby Computers and Graphics LTD. + Jaleco license */
	DRIVER( dingoe )	/* (c) 1983 Ashby Computers and Graphics LTD. + Jaleco license */
	DRIVER( blkhole )	/* TDS (Tokyo Denshi Sekkei) */
	DRIVER( kingball )	/* (c) 1980 Namco */
	DRIVER( kingballj )	/* (c) 1980 Namco */
	DRIVER( scorpionmc )/* bootleg */
	DRIVER( frogg )		/* bootleg */
	DRIVER( 4in1 )		/* (c) 1981 Armenia / Food and Fun */
	DRIVER( bagmanmc )	/* bootleg */
	DRIVER( dkongjrm )	/* bootleg */
	DRIVER( ozon1 )		/* (c) 1983 Proma */
	DRIVER( ladybugg )	/* bootleg */
	DRIVER( vpool )		/* bootleg */
	DRIVER( drivfrcg )	/* [1984] Shinkai (Magic Electronics USA license) */
	DRIVER( drivfrcb )	/* bootleg */
	DRIVER( bongo )		/* (c) 1983 Jetsoft */
	DRIVER( hunchbkg )	/* (c) 1983 Century */
	DRIVER( harem )		/* [1983] I.G.R. */
	DRIVER( tazzmang )	/* bootleg */
	DRIVER( racknrol )	/* (c) 1986 Status (Shinkia license) (Senko Kit) */
	DRIVER( hexpool )	/* (c) 1986 Shinkia (Senko Kit) */
	DRIVER( hexpoola )	/* (c) 1986 Shinkia (Senko Kit) */
	DRIVER( trvchlng )	/* [1985] Joyland (Senko license) */
	DRIVER( luctoday )	/* 1980 Sigma */
	DRIVER( chewing )	/* ?? unknown */
	DRIVER( catacomb )	/* 1982 MTM Games */
	DRIVER( ckongg )	/* 1981 bootleg */
	DRIVER( ckongmc )	/* 1981 bootleg */
	DRIVER( porter )	/* 1982 bootleg (Arcade TV Game List - P.98, Left, 15 from bottom) */
	DRIVER( vstars )	/* 19?? Competitive Video? */
	DRIVER( tdpgal )	/* 1983 Design Labs / Thomas Automatics */

	/* Has some similarities with Moon Cresta but Board is very different */
	DRIVER( rockclim )	/* (c)1981 Taito */

	/* "Scramble hardware" (and variations) games */
	DRIVER( scramble )	/* GX387 (c) 1981 Konami */
	DRIVER( scrambles )	/* GX387 (c) 1981 Stern */
	DRIVER( scramblebf )/* bootleg */
	DRIVER( scramblb )	/* bootleg */
	DRIVER( scramb2 )	/* bootleg */
	DRIVER( explorer )	/* bootleg */
	DRIVER( strfbomb )	/* bootleg */
	DRIVER( atlantis )	/* (c) 1981 Comsoft */
	DRIVER( atlantis2 )	/* (c) 1981 Comsoft */
	DRIVER( theend )	/* (c) 1980 Konami */
	DRIVER( theends )	/* (c) 1980 Stern */
	DRIVER( omega )		/* bootleg */
	DRIVER( ckongs )	/* bootleg */
	DRIVER( froggers )	/* bootleg */
	DRIVER( frogf )		/* bootleg */
	DRIVER( amidars )	/* GX337 (c) 1982 Konami */
	DRIVER( triplep )	/* (c) 1982 KKI */	/* made by Sanritsu? */
	DRIVER( knockout )	/* (c) 1982 KKK */
	DRIVER( mariner )	/* (c) 1981 Amenip */
	DRIVER( 800fath )	/* (c) 1981 Amenip + U.S. Billiards license */
	DRIVER( mars )		/* (c) 1981 Artic */
	DRIVER( devilfsh )	/* (c) 1982 Artic */
	DRIVER( newsin7 )	/* (c) 1983 ATW USA, Inc. */
	DRIVER( mrkougar )	/* (c) 1984 ATW */
	DRIVER( mrkougar2 )	/* (c) 1984 ATW */
	DRIVER( mrkougb )	/* bootleg */
	DRIVER( mrkougb2 )	/* bootleg */
	DRIVER( hotshock )	/* (c) 1982 E.G. Felaco (Domino license) */
	DRIVER( hotshockb )	/* bootleg */
	DRIVER( conquer )	/* (c) 1982 ???? */
	DRIVER( hunchbks )	/* (c) 1983 Century */
	DRIVER( hncholms )	/* (c) 1984 Century */
	DRIVER( cavelon )	/* (c) 1983 Jetsoft */
	DRIVER( sfx )		/* (c) 1983 Nichibutsu */
	DRIVER( skelagon )	/* (c) 1983 Nichibutsu USA */
	DRIVER( monsterz )	/* (c) 1982 Nihon (Arcade TV Game List - P.102, Left, 20 from top) */
	DRIVER( mimonscr )	/* (c) 1982 Universal Video Games (US Copyright Office info - http://cocatalog.loc.gov)*/
	DRIVER( mimonkey )	/* (c) 1982 bootleg */
	DRIVER( mimonsco )	/* (c) 1982 bootleg */
	DRIVER( scobra )	/* GX316 (c) 1981 Konami */
	DRIVER( scobras )	/* GX316 (c) 1981 Stern */
	DRIVER( scobrase )	/* GX316 (c) 1981 Stern */
	DRIVER( scobrab )	/* GX316 (c) 1981 Karateco (bootleg?) */
	DRIVER( suprheli )	/* bootleg */
	DRIVER( stratgyx )	/* GX306 (c) 1981 Konami */
	DRIVER( stratgys )	/* GX306 (c) 1981 Stern */
	DRIVER( strongx )	/* bootleg */
	DRIVER( armorcar )	/* (c) 1981 Stern */
	DRIVER( armorcar2 )	/* (c) 1981 Stern */
	DRIVER( moonwar )	/* (c) 1981 Stern */
	DRIVER( moonwara )	/* (c) 1981 Stern */
	DRIVER( spdcoin )	/* (c) 1984 Stern */
	DRIVER( darkplnt )	/* (c) 1982 Stern */
	DRIVER( tazmania )	/* (c) 1982 Stern */
	DRIVER( tazmani2 )	/* (c) 1982 Stern */
	DRIVER( calipso )	/* (c) 1982 Tago */
	DRIVER( anteater )	/* (c) 1982 Tago */
	DRIVER( anteatg )	/* (c) 1983 TV-Tuning (F.E.G. license) */
	DRIVER( anteatgb )	/* (c) 1983 Free Enterprise Games */
	DRIVER( rescue )	/* (c) 1982 Stern */
	DRIVER( aponow )	/* bootleg */
	DRIVER( minefld )	/* (c) 1983 Stern */
	DRIVER( losttomb )	/* (c) 1982 Stern */
	DRIVER( losttombh )	/* (c) 1982 Stern */
	DRIVER( superbon )	/* bootleg */
	DRIVER( hustler )	/* GX343 (c) 1981 Konami */
	DRIVER( billiard )	/* bootleg */
	DRIVER( hustlerb )	/* bootleg */
	DRIVER( hustlerb2 )	/* bootleg */
	DRIVER( frogger )	/* GX392 (c) 1981 Konami */
	DRIVER( froggers1 )	/* (c) 1981 Sega */
	DRIVER( froggers2 )	/* 834-0068 (c) 1981 Sega */
	DRIVER( froggermc )	/* 800-3110 (c) 1981 Sega */
	DRIVER( amidar )	/* GX337 (c) 1981 Konami */
	DRIVER( amidaru )	/* GX337 (c) 1982 Konami + Stern license */
	DRIVER( amidaro )	/* GX337 (c) 1982 Konami + Olympia license */
	DRIVER( amidarb )	/* bootleg */
	DRIVER( amigo )		/* bootleg */
	DRIVER( turtles )	/* (c) 1981 Stern */
	DRIVER( turpin )	/* (c) 1981 Sega */
	DRIVER( 600 )		/* GX353 (c) 1981 Konami */
	DRIVER( flyboy )	/* (c) 1982 Kaneko */
	DRIVER( flyboyb )	/* bootleg */
	DRIVER( fastfred )	/* 136009   (c) 1982 Atari */
	DRIVER( jumpcoas )	/* (c) 1983 Kaneko */
	DRIVER( jumpcoast )	/* (c) 1983 Taito */
	DRIVER( boggy84 )	/* (c) 1984 Kaneko */
	DRIVER( boggy84b )	/* bootleg */
	DRIVER( redrobin )	/* (c) 1986 Elettronolo */
	DRIVER( imago )		/* (c) 1984 Acom */
	DRIVER( imagoa )	/* (c) 1983 Acom */
	DRIVER( scorpion )	/* (c) 1982 Zaccaria */
	DRIVER( scorpiona )	/* (c) 1982 Zaccaria */
	DRIVER( scorpionb )	/* (c) 1982 Zaccaria */
	DRIVER( aracnis )	/* bootleg */
	DRIVER( ad2083 )	/* (c) 1983 Midcoin */
	DRIVER( turpins )	/* (c) 1981 bootleg */

	/* "Crazy Climber hardware" games */
	DRIVER( cclimber )	/* (c) 1980 Nichibutsu */
	DRIVER( cclimberj )	/* (c) 1980 Nichibutsu */
	DRIVER( ccboot )	/* bootleg */
	DRIVER( ccboot2 )	/* bootleg */

	DRIVER( ckong )		/* (c) 1981 (Falcon?) */
	DRIVER( ckongo )	/* bootleg (Orca) */
	DRIVER( ckongalc )	/* bootleg (Alca) */
	DRIVER( bigkong )	/* bootleg */
	DRIVER( monkeyd )	/* bootleg */

	DRIVER( ckongpt2 )		/* (c) 1981 Falcon */
	DRIVER( ckongpt2a )		/* (c) 1981 Falcon */
	DRIVER( ckongpt2j )		/* (c) 1981 Falcon */
	DRIVER( ckongpt2jeu )	/* bootleg (Jeutel) */
	DRIVER( ckongpt2b )		/* bootleg */

	DRIVER( rpatrol )	/* Orca, from a suspicious set */
	DRIVER( rpatrolb )	/* bootleg */
	DRIVER( silvland )	/* Falcon */
	DRIVER( yamato )	/* (c) 1983 Sega */
	DRIVER( yamato2 )	/* (c) 1983 Sega */
	DRIVER( toprollr )	/* (c) 1983 Jaleco */
	DRIVER( swimmer )	/* (c) 1982 Tehkan */
	DRIVER( swimmera )	/* (c) 1982 Tehkan */
	DRIVER( swimmerb )	/* (c) 1982 Tehkan */
	DRIVER( guzzler )	/* (c) 1983 Tehkan */
	DRIVER( cannonb )	/* (c) 1985 Soft */
	DRIVER( cannonb2 )	/* (c) 1985 TV Game Gruenberg */
	DRIVER( cannonb3 )	/* (c) 1985 Soft */
	DRIVER( bagmanf )	/* bootleg on Falcon FCK-00 (Falcon Crazy Kong) PCB */

	/* Nichibutsu games */
	DRIVER( gomoku )	/* (c) 1981 */
	DRIVER( wiping )	/* (c) 1982 */
	DRIVER( rugrats )	/* (c) 1983 */
	DRIVER( dacholer )	/* (c) 1983 */
	DRIVER( kickboy )	/* (c) 1983 */
	DRIVER( friskyt )	/* (c) 1981 */
	DRIVER( friskyta )	/* (c) 1981 */
	DRIVER( radrad )	/* (c) 1982 Nichibutsu USA */
	DRIVER( seicross )	/* (c) 1984 + Alice */
	DRIVER( sectrzon )	/* (c) 1984 + Alice */
	DRIVER( firebatl )	/* (c) 1984 Taito */
	DRIVER( clshroad )	/* (c) 1986 Wood Place Inc. */
	DRIVER( clshroads )	/* (c) 1986 Wood Place Inc. (Status Games license) */
	DRIVER( clshroadd )	/* (c) 1986 Wood Place Inc. (Data East license) */
	DRIVER( tubep )		/* (c) 1984 + Fujitek */
	DRIVER( tubepb )	/* (c) 1984 + bootleg */
	DRIVER( rjammer )	/* (c) 1984 + Alice */
	DRIVER( magmax )	/* (c) 1985 */
	DRIVER( cop01 )		/* (c) 1985 */
	DRIVER( cop01a )	/* (c) 1985 */
	DRIVER( mightguy )	/* (c) 1986 */
	DRIVER( terracre )	/* (c) 1985 */
	DRIVER( terracrea )	/* (c) 1985 */
	DRIVER( terracren )	/* (c) 1985 */
	DRIVER( amazon )	/* (c) 1986 */
	DRIVER( amatelas )	/* (c) 1986 */
	DRIVER( horekid )	/* (c) 1987 */
	DRIVER( horekidb )	/* bootleg */
	DRIVER( boobhack )	/* bootleg */
	DRIVER( galivan )	/* (c) 1985 */
	DRIVER( galivan2 )	/* (c) 1985 */
	DRIVER( dangar )	/* (c) 1986 */
	DRIVER( dangar2 )	/* (c) 1986 */
	DRIVER( dangarb )	/* bootleg */
	DRIVER( ninjemak )	/* (c) 1986 (US?) */
	DRIVER( youma )		/* (c) 1986 (Japan) */
	DRIVER( youma2 )	/* (c) 1986 (Japan) */
	DRIVER( youmab )	/* bootleg */
	DRIVER( youmab2 )	/* bootleg */
	DRIVER( legion )	/* (c) 1986 */
	DRIVER( legiono )	/* (c) 1986 */
	DRIVER( terraf )	/* (c) 1987 */
	DRIVER( terrafb )	/* (c) 1987 */
	DRIVER( terrafa )	/* (c) 1987 */
	DRIVER( terrafu )	/* (c) 1987 Nichibutsu USA */
	DRIVER( kodure )	/* (c) 1987 (Japan) */
	DRIVER( armedf )	/* (c) 1988 */
	DRIVER( armedff )	/* (c) 1988 Fillmore license */
	DRIVER( cclimbr2 )	/* (c) 1988 (Japan) */
	DRIVER( cclimbr2a )	/* (c) 1988 (Japan) */
	DRIVER( skyrobo )	/* (c) 1989 */
	DRIVER( bigfghtr )	/* (c) 1989 */

	/* Nichibutsu Mahjong games */
	DRIVER( hyhoo )		/* (c) 1987 */
	DRIVER( hyhoo2 )	/* (c) 1987 */

	DRIVER( jangou )	/* (c) 1983 */
	DRIVER( macha )		/* (c) 1983 Logitec */
	/* Jangou Night   *//* (c) 1983 */
	DRIVER( jngolady )	/* (c) 1984 */
	DRIVER( cntrygrl )	/* (c) 1984 Royal Denshi */
	DRIVER( cntrygrla ) /* (c) 1984 */
	DRIVER( roylcrdn )	/* (c) 1985 */
	DRIVER( fruitbun )	/* (c) 1984 */
	DRIVER( luckygrl )	/* (c) 1984 Wing? */
	DRIVER( nightgal )	/* (c) 1984 */
	DRIVER( ngtbunny )	/* (c) 1984 */
	DRIVER( royalngt )	/* (c) 1984 Royal Denshi */
	DRIVER( royalqn )
	DRIVER( sexygal  )	/* (c) 1985 */
	DRIVER( sweetgal )	/* (c) 1985 */
	DRIVER( ngalsumr )	/* (c) 1985 */

	DRIVER( pastelg )	/* (c) 1985 */
	DRIVER( 3ds )		/* (c) 1985 */

	DRIVER( crystalg )	/* (c) 1986 */
	DRIVER( crystal2 )	/* (c) 1986 */
	DRIVER( nightlov )	/* (c) 1986 */
	DRIVER( citylove )	/* (c) 1986 */
	DRIVER( mcitylov )	/* (c) 1986 */
	DRIVER( apparel )	/* (c) 1986 Central Denshi */
	DRIVER( secolove )	/* (c) 1986 */
	DRIVER( barline )	/* (c) 1986?*/
	DRIVER( housemnq )	/* (c) 1987 */
	DRIVER( housemn2 )	/* (c) 1987 */
	DRIVER( livegal )	/* (c) 1987 */
	DRIVER( seiha )		/* (c) 1987 */
	DRIVER( seiham )	/* (c) 1987 */
	DRIVER( mjgaiden )	/* (c) 1987 Central Denshi */
	DRIVER( bijokkoy )	/* (c) 1987 */
	DRIVER( iemoto )	/* (c) 1987 */
	DRIVER( iemotom )	/* (c) 1987 */
	DRIVER( ryuuha )	/* (c) 1987 */
	DRIVER( ojousan )	/* (c) 1987 */
	DRIVER( ojousanm )	/* (c) 1987 */
	DRIVER( bijokkog )	/* (c) 1988 */
	DRIVER( orangec )	/* (c) 1988 Daiichi Denshi */
	DRIVER( orangeci )	/* (c) 1988 Daiichi Denshi */
	DRIVER( vipclub )	/* (c) 1988 Daiichi Denshi */
	DRIVER( korinai )	/* (c) 1988 */
	DRIVER( korinaim )	/* (c) 1988 */
	DRIVER( kaguya )	/* (c) 1988 MIKI SYOUJI */
	DRIVER( kaguya2 )	/* (c) 1989 MIKI SYOUJI */
	DRIVER( kaguya2f )	/* (c) 1989 MIKI SYOUJI */
	DRIVER( otonano )	/* (c) 1988 Apple */
	DRIVER( kanatuen )	/* (c) 1988 Panac */
	DRIVER( kyuhito )	/* (c) 1988 ROLLER TRON */
	DRIVER( mjsikaku )	/* (c) 1988 */
	DRIVER( mjsikakb )	/* (c) 1988 */
	DRIVER( mjsikakc )	/* (c) 1988 */
	DRIVER( mjsikakd )	/* (c) 1988 */
	DRIVER( mmsikaku )	/* (c) 1988 */
	DRIVER( mjcamera )	/* (c) 1988 MIKI SYOUJI */
	DRIVER( mmcamera )	/* (c) 1988 MIKI SYOUJI */

	DRIVER( msjiken )	/* (c) 1988 */
	DRIVER( hanamomo )	/* (c) 1988 */
	DRIVER( hanamomb )	/* (c) 1988 */
	DRIVER( telmahjn )	/* (c) 1988 */
	DRIVER( taiwanmb )	/* (c) 1988 */
	DRIVER( gionbana )	/* (c) 1989 */
	DRIVER( idhimitu )	/* (c) 1989 Digital Soft */
	DRIVER( mgion )		/* (c) 1989 */
	DRIVER( mjcamerb )	/* (c) 1989 MIKI SYOUJI */
	DRIVER( omotesnd )	/* (c) 1989 */
	DRIVER( abunai )	/* (c) 1989 */
	DRIVER( mjfocus )	/* (c) 1989 */
	DRIVER( mjfocusm )	/* (c) 1989 */
	DRIVER( peepshow )	/* (c) 1989 AC */
	DRIVER( scandal )	/* (c) 1989 */
	DRIVER( scandalm )	/* (c) 1989 */
	DRIVER( mgmen89 )	/* (c) 1989 */
	DRIVER( mjnanpas )	/* (c) 1989 BROOKS */
	DRIVER( mjnanpaa )	/* (c) 1989 BROOKS */
	DRIVER( mjnanpau )	/* (c) 1989 BROOKS */
	DRIVER( pairsnb )	/* (c) 1989 */
	DRIVER( pairsten )	/* (c) 1989 System Ten */
	DRIVER( bananadr )	/* (c) 1989 DIGITAL SOFT */
	DRIVER( mladyhtr )	/* (c) 1990 */
	DRIVER( chinmoku )	/* (c) 1990 */
	DRIVER( maiko )		/* (c) 1990 */
	DRIVER( mmaiko )	/* (c) 1990 */
	DRIVER( hnageman )	/* (c) 1990 Nichibutsu/AV Japan */
	DRIVER( hnxmasev )	/* (c) 1990 Nichibutsu/AV Japan */
	DRIVER( club90s )	/* (c) 1990 */
	DRIVER( club90sa )	/* (c) 1990 */
	DRIVER( lovehous )	/* (c) 1990 */
	DRIVER( hanaoji )	/* (c) 1991 */

	DRIVER( pstadium )	/* (c) 1990 */
	DRIVER( triplew1 )	/* (c) 1989 */
	DRIVER( triplew2 )	/* (c) 1990 */
	DRIVER( ntopstar )	/* (c) 1990 */
	DRIVER( mjlstory )	/* (c) 1991 */
	DRIVER( vanilla )	/* (c) 1991 */
	DRIVER( finalbny )	/* (c) 1991 */
	DRIVER( qmhayaku )	/* (c) 1991 */
	DRIVER( mjgottub )	/* (c) 1991 */
	DRIVER( galkoku )	/* (c) 1989 Nichibutsu/T.R.TEC */
	DRIVER( hyouban )	/* (c) 1989 Nichibutsu/T.R.TEC */
	DRIVER( galkaika )	/* (c) 1989 Nichibutsu/T.R.TEC */
	DRIVER( tokyogal )	/* (c) 1989 */
	DRIVER( tokimbsj )	/* (c) 1989 */
	DRIVER( mcontest )	/* (c) 1989 */
	DRIVER( uchuuai )	/* (c) 1989 */
	DRIVER( av2mj1bb )	/* (c) 1991 MIKI SYOUJI/AV JAPAN */
	DRIVER( av2mj2rg )	/* (c) 1991 MIKI SYOUJI/AV JAPAN */

	DRIVER( ohpaipee )	/* (c) 1990 Nichibutsu */
	DRIVER( togenkyo )	/* (c) 1990 Nichibutsu */

	DRIVER( mjuraden )	/* (c) 1992 Nichibutsu/Yubis */
	DRIVER( koinomp )	/* (c) 1992 */
	DRIVER( patimono )	/* (c) 1992 */
	DRIVER( janbari )	/* (c) 1992 Nichibutsu/Yubis/AV JAPAN */
	DRIVER( mjanbari )	/* (c) 1992 Nichibutsu/Yubis/AV JAPAN */
	DRIVER( ultramhm )	/* (c) 1993 Apple */
	DRIVER( gal10ren )	/* (c) 1993 FUJIC */
	DRIVER( renaiclb )	/* (c) 1993 FUJIC */
	DRIVER( mjlaman )	/* (c) 1993 Nichibutsu/AV JAPAN */
	DRIVER( mkeibaou )	/* (c) 1993 */
	DRIVER( pachiten )	/* (c) 1993 Nichibutsu/MIKI SYOUJI/AV JAPAN */
	DRIVER( sailorws )	/* (c) 1993 */
	DRIVER( sailorwa )	/* (c) 1993 */
	DRIVER( sailorwr )	/* (c) 1993 */
	DRIVER( wcatcher )	/* (c) 1993 */
	DRIVER( jituroku )	/* (c) 1993 Windom */
	DRIVER( yosimoto )	/* (c) 1994 Nichibutsu/Yoshimoto Kougyou */
	DRIVER( psailor1 )	/* (c) 1994 SPHINX */
	DRIVER( psailor2 )	/* (c) 1994 SPHINX */
	DRIVER( otatidai )	/* (c) 1995 SPHINX */
	DRIVER( ngpgal )	/* (c) 1991 */
	DRIVER( mjgottsu )	/* (c) 1991 */
	DRIVER( bakuhatu )	/* (c) 1991 */
	DRIVER( cmehyou )	/* (c) 1992 Nichibutsu/Kawakusu */
	DRIVER( mmehyou )	/* (c) 1992 Nichibutsu/Kawakusu */
	DRIVER( mjkoiura )	/* (c) 1992 */
	DRIVER( mkoiuraa )	/* (c) 1992 */
	DRIVER( imekura )	/* (c) 1994 SPHINX/AV JAPAN */
	DRIVER( mscoutm )	/* (c) 1994 SPHINX/AV JAPAN */
	DRIVER( mjegolf )	/* (c) 1994 FUJIC/AV JAPAN */

	DRIVER( mhhonban )	/* (c) 199? */
	DRIVER( 4psimasy )	/* (c) 1994 */
	DRIVER( musobana )	/* (c) 1995 */
	DRIVER( niyanpai )	/* (c) 1996 */

	/* "Phoenix hardware" (and variations) games */
	DRIVER( safarir )	/* Shin Nihon Kikaku (SNK) */
	DRIVER( phoenix )	/* (c) 1980 Amstar */
	DRIVER( phoenixa )	/* (c) 1980 Amstar + Centuri license */
	DRIVER( phoenixb )	/* (c) 1980 Amstar + Centuri license */
	DRIVER( phoenixt )	/* (c) 1980 Taito */
	DRIVER( phoenixj )	/* (c) 1980 Taito */
	DRIVER( phoenix3 )	/* bootleg */
	DRIVER( phoenixc )	/* bootleg */
	DRIVER( condor )	/* bootleg */
	DRIVER( fenix )		/* bootleg */
	DRIVER( falcon )	/* bootleg */
	DRIVER( vautour )	/* bootleg (Jeutel) */
	DRIVER( falconz )	/* bootleg */
	DRIVER( vautourz )	/* bootleg */
	DRIVER( griffon )	/* bootleg (Videotron) */
	DRIVER( nextfase )	/* bootleg */
	DRIVER( pleiads )	/* (c) 1981 Tehkan */
	DRIVER( pleiadbl )	/* bootleg */
	DRIVER( pleiadce )	/* (c) 1981 Centuri + Tehkan */
	DRIVER( capitol )	/* bootleg? */
	DRIVER( survival )	/* (c) 1982 Rock-ola */
	DRIVER( naughtyb )	/* (c) 1982 Jaleco */
	DRIVER( naughtyba )	/* bootleg */
	DRIVER( naughtybc )	/* (c) 1982 Jaleco + Cinematronics */
	DRIVER( popflame )	/* (c) 1982 Jaleco */
	DRIVER( popflamea )	/* (c) 1982 Jaleco */
	DRIVER( popflameb )	/* (c) 1982 Jaleco */
	DRIVER( popflamen )	/* (c) 1982 Jaleco */
	DRIVER( trvmstr )	/* (c) 1985 Enerdyne Technologies Inc. */
	DRIVER( trvmstra )	/* (c) 1985 Enerdyne Technologies Inc. */
	DRIVER( trvmstrb )	/* (c) 1985 Enerdyne Technologies Inc. */
	DRIVER( trvmstrc )	/* (c) 1985 Enerdyne Technologies Inc. */
	DRIVER( trvgns )	/* bootleg */



	/* Namco games (plus some intruders on similar hardware) */
	DRIVER( geebee )	/* [1978] Namco */
	DRIVER( geebeeb )	/* [1978] Namco (F.lli Bertolino license) */
	DRIVER( geebeeg )	/* [1978] Gremlin */
	DRIVER( bombbee )	/* [1979] Namco */
	DRIVER( cutieq )	/* (c) 1979 Namco */
	DRIVER( navarone )	/* (c) 1980 Namco */
	DRIVER( kaitein )	/* [1980] Namco */
	DRIVER( kaitei )	/* [1980] K.K. Tokki */
	DRIVER( sos )		/* [1980] Namco */
	DRIVER( tankbatt )	/* (c) 1980 Namco */
	DRIVER( warpwarp )	/* (c) 1981 Namco */
	DRIVER( warpwarpr )	/* (c) 1981 [Namco] (Rock-ola license) */
	DRIVER( warpwarpr2 )/* (c) 1981 [Namco] (Rock-ola license) */
	DRIVER( rallyx )	/* (c) 1980 Namco */
	DRIVER( rallyxa )	/* (c) 1980 Namco */
	DRIVER( rallyxm )	/* (c) 1980 Midway */
	DRIVER( nrallyx )	/* (c) 1981 Namco */
	DRIVER( nrallyxb )	/* (c) 1981 Namco */
	DRIVER( jungler )	/* GX327 (c) 1981 Konami */
	DRIVER( junglers )	/* GX327 (c) 1981 Stern */
	DRIVER( tactcian )	/* GX335 (c) 1982 Sega */
	DRIVER( tactcian2 )	/* GX335 (c) 1981 Sega */
	DRIVER( locomotn )	/* GX359 (c) 1982 Konami + Centuri license */
	DRIVER( gutangtn )	/* GX359 (c) 1982 Konami + Sega license */
	DRIVER( cottong )	/* bootleg */
	DRIVER( locoboot )	/* bootleg */
	DRIVER( commsega )	/* (c) 1983 Sega */
	/* the following ones all have a custom I/O chip */
	DRIVER( bosco )		/* (c) 1981 */
	DRIVER( boscoo )	/* (c) 1981 */
	DRIVER( boscoo2 )	/* (c) 1981 */
	DRIVER( boscomd )	/* (c) 1981 Midway */
	DRIVER( boscomdo )	/* (c) 1981 Midway */
	DRIVER( galaga )	/* (c) 1981 */
	DRIVER( galagao )	/* (c) 1981 */
	DRIVER( galagamw )	/* (c) 1981 Midway */
	DRIVER( galagamk )	/* (c) 1981 Midway */
	DRIVER( galagamf )	/* (c) 1981 Midway / hack */
	DRIVER( gallag )	/* bootleg */
	DRIVER( gatsbee )	/* (c) 1984 Uchida / hack */
	DRIVER( digdug )	/* (c) 1982 */
	DRIVER( digdug1 )	/* (c) 1982 */
	DRIVER( digdugat )	/* 136007   (c) 1982 Atari */
	DRIVER( digdugat1 )	/* 136007   (c) 1982 Atari */
	DRIVER( dzigzag )	/* bootleg */
	DRIVER( digsid )	/* bootleg */
	DRIVER( xevious )	/* (c) 1982 */
	DRIVER( xeviousa )	/* 136018   (c) 1982 + Atari license */
	DRIVER( xeviousb )	/* 136018   (c) 1982 + Atari license */
	DRIVER( xeviousc )	/* 136018   (c) 1982 + Atari license */
	DRIVER( xevios )	/* bootleg */
	DRIVER( battles )	/* bootleg */
	DRIVER( sxevious )	/* (c) 1984 */
	DRIVER( sxeviousj )	/* (c) 1984 */
	DRIVER( superpac )	/* (c) 1982 */
	DRIVER( superpacm )	/* (c) 1982 Midway */
	DRIVER( pacnpal )	/* (c) 1983 */
	DRIVER( pacnpal2 )	/* (c) 1983 */
	DRIVER( pacnchmp )	/* (c) 1983 */
	DRIVER( phozon )	/* (c) 1983 */
	DRIVER( mappy )		/* (c) 1983 */
	DRIVER( mappyj )	/* (c) 1983 */
	DRIVER( digdug2 )	/* (c) 1985 */
	DRIVER( digdug2o )	/* (c) 1985 */
	DRIVER( todruaga )	/* (c) 1984 */
	DRIVER( todruagao )	/* (c) 1984 */
	DRIVER( todruagas )	/* (c) 1984 pcb manufactured by Sidam */
	DRIVER( motos )		/* (c) 1985 */
	DRIVER( grobda )	/* (c) 1984 */
	DRIVER( grobda2 )	/* (c) 1984 */
	DRIVER( grobda3 )	/* (c) 1984 */
	DRIVER( gaplus )	/* (c) 1984 */
	DRIVER( gaplusa )	/* (c) 1984 */
	DRIVER( gapluso )	/* (c) 1984 */
	DRIVER( galaga3 )	/* (c) 1984 */
	DRIVER( galaga3m )	/* (c) 1984 */
	DRIVER( galaga3a )	/* (c) 1984 */
	/* Libble Rabble board (first Japanese game using a 68000) */
	DRIVER( liblrabl )	/* (c) 1983 */
	DRIVER( toypop )	/* (c) 1986 */
	/* Z8000 games */
	DRIVER( polepos )	/* (c) 1982  */
	DRIVER( poleposa )	/* 136014   (c) 1982 + Atari license */
	DRIVER( polepos1 )	/* 136014   (c) 1982 Atari */
	DRIVER( topracer )	/* bootleg */
	DRIVER( topracera )	/* bootleg */
	DRIVER( topracern )	/* bootleg */
	DRIVER( polepos2 )	/* (c) 1983 */
	DRIVER( polepos2a )	/* 136014   (c) 1983 + Atari license */
	DRIVER( polepos2b )	/* bootleg */
	DRIVER( polepos2bi )/* bootleg */
	/* no custom I/O in the following, HD63701 (or compatible) microcontroller instead */
	DRIVER( pacland )	/* (c) 1984 */
	DRIVER( paclandj )	/* (c) 1984 */
	DRIVER( paclandjo )	/* (c) 1984 */
	DRIVER( paclandjo2 )/* (c) 1984 */
	DRIVER( paclandm )	/* (c) 1984 Midway */
	DRIVER( drgnbstr )	/* (c) 1984 */
	DRIVER( skykid )	/* (c) 1985 */
	DRIVER( skykido )	/* (c) 1985 */
	DRIVER( skykidd )	/* (c) 1985 */
	DRIVER( skykids )	/* (c) 1985 */
	DRIVER( aliensec )	/* (c) 1985 */
	DRIVER( baraduke )	/* (c) 1985 */
	DRIVER( metrocrs )	/* (c) 1985 */
	DRIVER( metrocrsa )	/* (c) 1985 */

	/* Namco System 86 games */
	DRIVER( hopmappy )	/* (c) 1986 */
	DRIVER( skykiddx )	/* (c) 1986 */
	DRIVER( skykiddxo )	/* (c) 1986 */
	DRIVER( roishtar )	/* (c) 1986 */
	DRIVER( genpeitd )	/* (c) 1986 */
	DRIVER( rthunder )	/* (c) 1986 new version */
	DRIVER( rthundero )	/* (c) 1986 old version */
	DRIVER( wndrmomo )	/* (c) 1987 */

	/* Thunder Ceptor HW */
	DRIVER( tceptor )	/* (c) 1986 */
	DRIVER( tceptor2 )	/* (c) 1986 */

	DRIVER( kungfur )	/* (c) 1987 */

	/* Namco System 1 games */
	DRIVER( shadowld )	/* (c) 1987 */
	DRIVER( youkaidk )	/* (c) 1987 (Japan new version) */
	DRIVER( youkaidko )	/* (c) 1987 (Japan old version) */
	DRIVER( dspirit )	/* (c) 1987 (Japan new version) */
	DRIVER( dspirito )	/* (c) 1987 (Japan old version) */
	DRIVER( dspirita )	/* (c) 1987 (Atari license) */
	DRIVER( blazer )	/* (c) 1987 (Japan) */
	DRIVER( quester )	/* (c) 1987 (Japan) */
	DRIVER( pacmania )	/* (c) 1987 */
	DRIVER( pacmaniaj )	/* (c) 1987 (Japan) */
	DRIVER( galaga88 )	/* (c) 1987 */
	DRIVER( galaga88j )	/* (c) 1987 (Japan) */
	DRIVER( ws )		/* (c) 1988 (Japan) */
	DRIVER( berabohm )	/* (c) 1988 (Japan) */
	DRIVER( berabohmo )	/* (c) 1988 (Japan) */
				/* 1988 Alice in Wonderland (English version of Marchen maze) */
	DRIVER( mmaze )		/* (c) 1988 (Japan) */
	DRIVER( bakutotu )	/* (c) 1988 (Japan) */
	DRIVER( wldcourt )	/* (c) 1988 (Japan) */
	DRIVER( splatter )	/* (c) 1988 (World) */
	DRIVER( splattero )	/* (c) 1988 (World) */
	DRIVER( splatterj )	/* (c) 1988 (Japan) */
	DRIVER( faceoff )	/* (c) 1988 (Japan) */
	DRIVER( rompers )	/* (c) 1989 (Japan) */
	DRIVER( romperso )	/* (c) 1989 (Japan) */
	DRIVER( blastoff )	/* (c) 1989 (Japan) */
	DRIVER( ws89 )		/* (c) 1989 (Japan) */
	DRIVER( dangseed )	/* (c) 1989 (Japan) */
	DRIVER( ws90 )		/* (c) 1990 (Japan) */
	DRIVER( pistoldm )	/* (c) 1990 (Japan) */
	DRIVER( boxyboy )	/* (c) 1990 (US) */
	DRIVER( soukobdx )	/* (c) 1990 (Japan) */
	DRIVER( puzlclub )	/* (c) 1990 (Japan) */
	DRIVER( tankfrce )	/* (c) 1991 (US, 2 Player) */
	DRIVER( tankfrce4 )	/* (c) 1991 (US, 4 Player) */
	DRIVER( tankfrcej )	/* (c) 1991 (Japan) */

	/* Namco System 2 games */
	DRIVER( finallap )	/* 87.12 Final Lap */
	DRIVER( finallapd )	/* 87.12 Final Lap */
	DRIVER( finallapc )	/* 87.12 Final Lap */
	DRIVER( finallapjc )/* 87.12 Final Lap */
	DRIVER( finallapjb )/* 87.12 Final Lap */
	DRIVER( assault )	/* (c) 1988 */
	DRIVER( assaultj )	/* (c) 1988 (Japan) */
	DRIVER( assaultp )	/* (c) 1988 (Japan) */
	DRIVER( metlhawk )	/* (c) 1988 */
	DRIVER( metlhawkj )	/* (c) 1988 */
	DRIVER( ordyne )	/* (c) 1988 */
	DRIVER( ordynej )	/* (c) 1988 */
	DRIVER( mirninja )	/* (c) 1988 (Japan) */
	DRIVER( phelios )	/* (c) 1988 (Japan) */
	DRIVER( dirtfoxj )	/* (c) 1989 (Japan) */
	DRIVER( fourtrax )	/* 89.11 */
	DRIVER( valkyrie )	/* (c) 1989 (Japan) */
	DRIVER( finehour )	/* (c) 1989 (Japan) */
	DRIVER( burnforc )	/* (c) 1989 (Japan) */
	DRIVER( burnforco )	/* (c) 1989 (Japan) */
	DRIVER( marvland )	/* (c) 1989 (US) */
	DRIVER( marvlandj )	/* (c) 1989 (Japan) */
	DRIVER( kyukaidk )	/* (c) 1990 (Japan) */
	DRIVER( kyukaidko )	/* (c) 1990 (Japan) */
	DRIVER( dsaber )	/* (c) 1990 */
	DRIVER( dsaberj )	/* (c) 1990 (Japan) */
	DRIVER( finalap2 )	/* 90.8  Final Lap 2 */
	DRIVER( finalap2j )	/* 90.8  Final Lap 2 (Japan) */
	DRIVER( gollygho )	/* 91.7  Golly Ghost */
	DRIVER( rthun2 )	/* (c) 1990 */
	DRIVER( rthun2j )	/* (c) 1990 (Japan) */
	DRIVER( sgunner )	/* (c) 1990 */
	DRIVER( sgunnerj )	/* (c) 1990 */
	DRIVER( sgunner2 )	/* (c) 1991 (US) */
	DRIVER( sgunner2j )	/* (c) 1991 (Japan) */
	DRIVER( cosmogng )	/* (c) 1991 (US) */
	DRIVER( cosmogngj )	/* (c) 1991 (Japan) */
	DRIVER( bubbletr )	/* (c) 1992 */
	DRIVER( finalap3 )	/* 92.9  Final Lap 3 */
	DRIVER( finalap3a )	/* 92.9  Final Lap 3 */
	DRIVER( finalap3j )	/* 92.9  Final Lap 3 */
	DRIVER( luckywld )	/* (c) 1992 */
	DRIVER( luckywldj )	/* (c) 1992 */
	DRIVER( suzuka8h )	/* (c) 1992 (World) */
	DRIVER( suzuka8hj )	/* (c) 1992 (Japan) */
	DRIVER( sws )		/* (c) 1992 (Japan) */
	DRIVER( sws92 )		/* (c) 1992 (Japan) */
	DRIVER( sws92g )	/* (c) 1992 (Japan) */
	DRIVER( suzuk8h2 )	/* (c) 1993 (World) */
	DRIVER( sws93 )		/* (c) 1993 (Japan) */

	/* Namco FL games */
	DRIVER( speedrcr )	/* (c) 1995 */
	DRIVER( finalapr )	/* (c) 1995 */
	DRIVER( finalaprj )	/* (c) 1995 */
	DRIVER( finalapro )	/* (c) 1995 */

	/* Namco NA-1 / NA-2 System games */
	DRIVER( bkrtmaq )	/* (c) 1992 (Japan) */
	DRIVER( cgangpzl )	/* (c) 1992 (US) */
	DRIVER( cgangpzlj )	/* (c) 1992 (Japan) */
	DRIVER( exvania )	/* (c) 1992 (Japan) */
	DRIVER( fghtatck )	/* (c) 1992 (US) */
	DRIVER( fa )		/* (c) 1992 (Japan) */
	DRIVER( knckhead )	/* (c) 1992 (World) */
	DRIVER( knckheadj )	/* (c) 1992 (Japan) */
	DRIVER( swcourt )	/* (c) 1992 (Japan) */
	DRIVER( swcourtj )	/* (c) 1992 (Japan) */
	DRIVER( emeralda )	/* (c) 1993 (Japan) */
	DRIVER( emeraldaj )	/* (c) 1993 (Japan) */
	DRIVER( emeraldaja )	/* (c) 1993 (Japan) */
	DRIVER( numanath )	/* (c) 1993 (World) */
	DRIVER( numanathj )	/* (c) 1993 (Japan) */
	DRIVER( quiztou )	/* (c) 1993 (Japan) */
	DRIVER( tinklpit )	/* (c) 1993 (Japan) */
	DRIVER( xday2 )		/* (c) 1995 (Japan) */

	/* Namco NB-1 / NB-2 System games */
	DRIVER( nebulray )	/* (c) 1994 (World) */
	DRIVER( nebulrayj )	/* (c) 1994 (Japan) */
	DRIVER( ptblank )	/* (c) 1994 (GN2 World) */
	DRIVER( gunbuletj )	/* (c) 1994 (GN1 Japan) */
	DRIVER( gunbuletw )	/* (c) 1994 (GN3 World) */
	DRIVER( gslugrsj )	/* (c) 1993 (Japan) */
	DRIVER( gslgr94u )	/* (c) 1994 */
	DRIVER( gslgr94j )	/* (c) 1994 (Japan) */
	DRIVER( sws95 )		/* (c) 1995 (Japan) */
	DRIVER( sws96 )		/* (c) 1996 (Japan) */
	DRIVER( sws97 )		/* (c) 1997 (Japan) */
	DRIVER( vshoot )	/* (c) 1994 */
	DRIVER( outfxies )	/* (c) 1994 (World) */
	DRIVER( outfxiesj )	/* (c) 1994 (Japan) */
	DRIVER( machbrkr )	/* (c) 1995 (Japan) */

	/* Namco ND-1 games */
	DRIVER( ncv1 )		/* (c) 1995 */
	DRIVER( ncv1j )		/* (c) 1995 (Japan) */
	DRIVER( ncv1j2 )	/* (c) 1995 (Japan) */
	DRIVER( ncv2 )		/* (c) 1996 */
	DRIVER( ncv2j )		/* (c) 1996 (Japan) */

	/* Namco System 21 games */
	DRIVER( winrun )	/* (c) 1988 */
				/* 1989, Winning Run Suzuka Grand Prix */
	DRIVER( winrun91 )	/* (c) 1991 */
	DRIVER( solvalou )	/* (c) 1991 (Japan) */
	DRIVER( starblad )	/* (c) 1991 */
	DRIVER( driveyes )	/* (c) 1992? */
				/* 1992, ShimDrive */
	DRIVER( aircomb )	/* (c) 1992 (US) */
	DRIVER( aircombj )	/* (c) 1992 (Japan) */
	DRIVER( cybsled )	/* (c) 1993 */
	DRIVER( cybsledj )	/* (c) 1993 */

	/* Galaxian 3 HW */
	DRIVER( gal3 )		/* (c) 1992 (Arcade TV Game List - P.73, Right, 6 from top) */

	/* Namco System 22 games */
	DRIVER( ridgerac )	/* (c) 1993 */
	DRIVER( ridgeracb )	/* (c) 1993 */
	DRIVER( ridgeracj )	/* (c) 1993 */
	DRIVER( ridgerac3 )	/* (c) 1993 */
	DRIVER( rrf )		/* (c) 1993 */
	DRIVER( acedrvrw )	/* (c) 1994 */
	DRIVER( alpinerd )	/* (c) 1994 */
	DRIVER( alpinerc )	/* (c) 1994 */
	DRIVER( ridgera2 )	/* (c) 1994 */
	DRIVER( ridgera2j )	/* (c) 1994 */
	DRIVER( ridgera2ja )/* (c) 1994 */
	DRIVER( raveracw )	/* (c) 1995 */
	DRIVER( raveracj )	/* (c) 1995 */
	DRIVER( raveracja )	/* (c) 1995 */
	DRIVER( airco22b )	/* (c) 1995 */
	DRIVER( alpinr2b )	/* (c) 1995 */
	DRIVER( alpinr2a )	/* (c) 1995 */
	DRIVER( cybrcomm )	/* (c) 1995 */
	DRIVER( cybrcycc )	/* (c) 1995 */
	DRIVER( timecris )	/* (c) 1995 */
	DRIVER( timecrisa )	/* (c) 1995 */
	DRIVER( dirtdash )	/* (c) 1995 */
	DRIVER( victlapw )	/* (c) 1996 */
	DRIVER( alpinesa )	/* (c) 1996 */
	DRIVER( propcycl )	/* (c) 1996 */
	DRIVER( tokyowar )	/* (c) 1996 */
	DRIVER( aquajet )	/* (c) 1996 */

	/* Namco System 23 */
	DRIVER( finlflng )	/* July 1997 */
	DRIVER( motoxgo )	/* November 1997 */
	DRIVER( rapidrvr )	/* December 1997 */
	DRIVER( rapidrvr2 )	/* December 1997 */
	DRIVER( downhill )	/* March 1998 */
	DRIVER( timecrs2 )	/* April 1998 */
	DRIVER( timecrs2b )	/* April 1998 */
	DRIVER( timecrs2c )	/* April 1998 */
	DRIVER( panicprk )	/* May 1998 */
				/* Race On - September 1998 */
	DRIVER( gunwars )	/* September 1998 */
				/* Angler King - March 1999 */
	DRIVER( 500gp )		/* March 1999 */
	DRIVER( finfurl2 )	/* March 1999 */
	DRIVER( finfurl2j )	/* March 1999 */
				/* Guitar Jam - July 1999 */
	DRIVER( crszone )	/* Crisis Zone - December 1999 */

	/* Namco System 246 */
	DRIVER( sys246 )
	DRIVER( dragchrn )
	DRIVER( netchu02 )
	DRIVER( soulclb2 )
	DRIVER( soulcl2a )
	DRIVER( tekken4 )
	DRIVER( tekken4a )
	DRIVER( tekken4b )
	DRIVER( timecrs3 )
	DRIVER( zgundm )
	DRIVER( fghtjam )
	DRIVER( sukuinuf )
	DRIVER( zgundmdx )
	DRIVER( gundzaft )
	DRIVER( soulclb3 )

	/* Namco System 256 */
	DRIVER( sys256 )
	DRIVER( tekken51 )
	DRIVER( kinniku )
	DRIVER( taiko9 )
	DRIVER( taiko10 )

	/* Misc Namco games */
	DRIVER( 25pacman )	/* (c) 2005 Ver 2.0 - Same hardware as 20pacgal */
	DRIVER( 20pacgal )	/* (c) 2000 Ver 1.08 */
	DRIVER( 20pacgalr4 )	/* (c) 2000 Ver 1.04 */
	DRIVER( 20pacgalr3 )	/* (c) 2000 Ver 1.03 */
	DRIVER( 20pacgalr2 )	/* (c) 2000 Ver 1.02 */
	DRIVER( 20pacgalr1 )	/* (c) 2000 Ver 1.01 */
	DRIVER( 20pacgalr0 )	/* (c) 2000 Ver 1.00 */
	DRIVER( turrett )	/* (c) 2001 licensed from Dell Electronics */

	/* Universal games */
	DRIVER( cosmicg )	/* 7907 (c) 1979 */
	DRIVER( cosmica )	/* 7910-AII (c) [1979] */
	DRIVER( cosmica1 )	/* 7910-A (c) [1979] */
	DRIVER( cosmica2 )	/* 7910 (c) 1979 */
	DRIVER( nomnlnd )	/* 8003 (c) [1980?] */
	DRIVER( nomnlndg )	/* 8003 (c) [1980?] + Gottlieb */
	DRIVER( magspot )	/* ???? (c) [1980] */
	DRIVER( magspot2 )	/* 8013 (c) [1980] */
	DRIVER( panic )		/* 8015 (c) 1980 */
	DRIVER( panic2 )	/* 8015 (c) 1980 */
	DRIVER( panic3 )	/* 8015 (c) 1980 */
	DRIVER( panich )	/* 8015 (c) 1980 */
	DRIVER( panicger )	/* 8015 (c) 1980 */
	DRIVER( devzone )	/* 8022 (c) [1980] */
	DRIVER( devzone2 )	/* 8022 (c) [1980] */
	DRIVER( cheekyms )	/* 8004 (c) [1980?] */
	DRIVER( zerohour )	/* 8011 (c) Universal */
	DRIVER( redclash )	/* (c) 1981 Tehkan */
	DRIVER( redclasha )	/* (c) 1981 Tehkan */
	DRIVER( redclashk )	/* (c) Kaneko (bootleg?) */
	DRIVER( cavenger )	/* 8106 (c) 1981 */
	DRIVER( ladybug )	/* 8110 (c) 1981 */
	DRIVER( ladybugb )	/* bootleg */
	DRIVER( ladybgb2 )	/* bootleg */
	DRIVER( snapjack )	/* ???? (c) */
	DRIVER( sraider )	/* ???? (c) 1982 Universal */
	DRIVER( dorodon )	/* Falcon */
	DRIVER( dorodon2 )	/* Falcon */
	DRIVER( mrdo )		/* 8201 (c) 1982 */
	DRIVER( mrdoy )		/* 8201 (c) 1982 */
	DRIVER( mrdot )		/* 8201 (c) 1982 + Taito license */
	DRIVER( mrdofix )	/* 8201 (c) 1982 + Taito license */
	DRIVER( mrlo )		/* bootleg */
	DRIVER( mrdu )		/* bootleg */
	DRIVER( yankeedo )	/* bootleg */
	DRIVER( docastle )	/* 8302 (c) 1983 */
	DRIVER( docastle2 )	/* 8302 (c) 1983 */
	DRIVER( docastleo )	/* 8302 (c) 1983 */
	DRIVER( douni )		/* 8302 (c) 1983 */
	DRIVER( dorunrun )	/* ???? (c) 1984 */
	DRIVER( dorunrun2 )	/* ???? (c) 1984 */
	DRIVER( dorunrunc )	/* ???? (c) 1984 */
	DRIVER( dorunrunca )/* ???? (c) 1984 */
	DRIVER( spiero )	/* ???? (c) 1987 */
	DRIVER( dowild )	/* ???? (c) 1984 */
	DRIVER( jjack )		/* ???? (c) 1984 */
	DRIVER( kickridr )	/* ???? (c) 1984 */
	DRIVER( idsoccer )	/* ???? (c) 1985 */
	DRIVER( idsoccera )	/* ???? (c) 1985 */
	DRIVER( asoccer )   /* 8461 (c) 1987 */
	DRIVER( superdq )	/* ???? (c) 1984 */
	DRIVER( superdqs )	/* ???? (c) 1984 */
	DRIVER( superdqa )	/* ???? (c) 1984 */

	/* Nintendo games */
	DRIVER( spacefev )	/* (c) 1979 Nintendo */
	DRIVER( spacefevo )	/* (c) 1979 Nintendo */
	DRIVER( spacefevo2 )/* (c) 1979 Nintendo */
	DRIVER( highsplt )	/* (c) 1979 Nintendo */
	DRIVER( highsplta )	/* (c) 1979 Nintendo */
	DRIVER( highspltb )	/* (c) 1979 Nintendo */
	DRIVER( spacelnc )	/* (c) 1979 Nintendo */
	DRIVER( sheriff )	/* (c) 1979 Nintendo */
	DRIVER( bandido )	/* (c) 1980 Exidy */
	DRIVER( helifire )	/* (c) 1980 Nintendo */
	DRIVER( helifirea )	/* (c) 1980 Nintendo */
	DRIVER( radarscp )	/* (c) 1980 Nintendo */
	DRIVER( radarscp1 )	/* (c) 1980 Nintendo */
	DRIVER( dkong )		/* (c) 1981 Nintendo of America */
	DRIVER( dkongo )	/* (c) 1981 Nintendo */
	DRIVER( dkongj )	/* (c) 1981 Nintendo */
	DRIVER( dkongjo )	/* (c) 1981 Nintendo */
	DRIVER( dkongjo1 )	/* (c) 1981 Nintendo */
	DRIVER( dkongf )	/* hack from Jeff's Romhack */
	DRIVER( dkongx )	/* hack */
	DRIVER( dkongx11 )	/* hack */
	DRIVER( dkongjr )	/* (c) 1982 Nintendo of America */
	DRIVER( dkongjrj )	/* (c) 1982 Nintendo */
	DRIVER( dkongjnrj )	/* (c) 1982 Nintendo */
	DRIVER( dkongjrb )	/* bootleg */
	DRIVER( jrking )	/* bootleg */
	DRIVER( dkongjre )	/* (c) 1982 Nintendo of America */
	DRIVER( dkong3 )	/* (c) 1983 Nintendo of America */
	DRIVER( dkong3j )	/* (c) 1983 Nintendo */
	DRIVER( dkong3b )	/* bootleg */
	DRIVER( mario )		/* (c) 1983 Nintendo of America */
	DRIVER( marioe )	/* (c) 1983 Nintendo of America */
	DRIVER( marioo )	/* (c) 1983 Nintendo of America */
	DRIVER( marioj )	/* (c) 1983 Nintendo */
	DRIVER( masao )		/* bootleg */
	DRIVER( pestplce )	/* bootleg on donkey kong hw */
	DRIVER( spclforc )	/* (c) 1985 Senko Industries (Magic Eletronics Inc. license) */
	DRIVER( spcfrcii )	/* (c) 1985 Senko Industries (Magic Eletronics Inc. license) */
	DRIVER( 8ballact )	/* (c) 1984 Seatongrove Ltd (Magic Eletronics USA license) */
	DRIVER( 8ballact2 )	/* (c) 1984 Seatongrove Ltd (Magic Eletronics USA license) */
	DRIVER( shootgal )	/* (c) 1984 Seatongrove Ltd (Zaccaria license) */
	DRIVER( drakton )	/* (c) 1984 Epos Corporation */
	DRIVER( drktnjr )	/* (c) 1984 Epos Corporation */
	DRIVER( strtheat )	/* (c) 1985 Epos Corporation */
	DRIVER( hunchbkd )	/* (c) 1983 Century */
	DRIVER( sbdk )		/* (c) 1984 Century */
	DRIVER( herbiedk )	/* (c) 1984 CVS */
	DRIVER( herodk )	/* (c) 1984 Seatongrove + Crown license */
	DRIVER( herodku )	/* (c) 1984 Seatongrove + Crown license */
	DRIVER( skyskipr )	/* (c) 1981 */
	DRIVER( popeye )	/* (c) 1982 */
	DRIVER( popeyeu )	/* (c) 1982 */
	DRIVER( popeyef )	/* (c) 1982 */
	DRIVER( popeyebl )	/* bootleg */
	DRIVER( punchout )	/* (c) 1984 */
	DRIVER( punchita )	/* bootleg */
	DRIVER( spnchout )	/* (c) 1984 */
	DRIVER( spnchoutj )	/* (c) 1984 (Japan) */
	DRIVER( armwrest )	/* (c) 1985 */

	/* Nintendo Playchoice 10 games */
	DRIVER( playch10 )
	DRIVER( pc_tenis )	/* (c) 1983 Nintendo */
	DRIVER( pc_mario )	/* (c) 1983 Nintendo */
	DRIVER( pc_bball )	/* (c) 1984 Nintendo of America */
	DRIVER( pc_bfght )	/* (c) 1984 Nintendo */
	DRIVER( pc_ebike )	/* (c) 1984 Nintendo */
	DRIVER( pc_golf )	/* (c) 1984 Nintendo */
	DRIVER( pc_kngfu )	/* (c) 1984 Irem (Nintendo license) */
	DRIVER( pc_1942 )	/* (c) 1985 Capcom */
	DRIVER( pc_smb )	/* (c) 1985 Nintendo */
	DRIVER( pc_vball )	/* (c) 1986 Nintendo */
	DRIVER( pc_duckh )	/* (c) 1984 Nintendo */
	DRIVER( pc_hgaly )	/* (c) 1984 Nintendo */
	DRIVER( pc_wgnmn )	/* (c) 1984 Nintendo */
	DRIVER( pc_grdus )	/* (c) 1986 Konami */
	DRIVER( pc_grdue )	/* (c) 1986 Konami */
	DRIVER( pc_tkfld )	/* (c) 1987 Konami (Nintendo of America license) */
	DRIVER( pc_pwrst )	/* (c) 1986 Nintendo */
	DRIVER( pc_trjan )	/* (c) 1986 Capcom USA (Nintendo of America license) */
	DRIVER( pc_cvnia )	/* (c) 1987 Konami (Nintendo of America license) */
	DRIVER( pc_dbldr )	/* (c) 1987 Konami (Nintendo of America license) */
	DRIVER( pc_rnatk )	/* (c) 1987 Konami (Nintendo of America license) */
	DRIVER( pc_rygar )	/* (c) 1987 Tecmo (Nintendo of America license) */
	DRIVER( pc_cntra )	/* (c) 1988 Konami (Nintendo of America license) */
	DRIVER( pc_goons )	/* (c) 1986 Konami */
	DRIVER( pc_mtoid )	/* (c) 1986 Nintendo */
	DRIVER( pc_radrc )	/* (c) 1987 Square */
	DRIVER( pc_miket )	/* (c) 1987 Nintendo */
	DRIVER( pc_rcpam )	/* (c) 1987 Rare */
	DRIVER( pc_ngaid )	/* (c) 1989 Tecmo (Nintendo of America license) */
	DRIVER( pc_tmnt )	/* (c) 1989 Konami (Nintendo of America license) */
	DRIVER( pc_ftqst )	/* (c) 1989 Sunsoft (Nintendo of America license) */
	DRIVER( pc_bstar )	/* (c) 1989 SNK (Nintendo of America license) */
	DRIVER( pc_tbowl )	/* (c) 1989 Tecmo (Nintendo of America license) */
	DRIVER( pc_drmro )	/* (c) 1990 Nintendo */
	DRIVER( pc_ynoid )	/* (c) 1990 Capcom USA (Nintendo of America license) */
	DRIVER( pc_rrngr )	/* (c) Capcom USA (Nintendo of America license) */
	DRIVER( pc_ddrgn )	/* (c) 1988 Technos */
	DRIVER( pc_gntlt )	/* (c) 1985 Atari/Tengen (Nintendo of America license) */
	DRIVER( pc_smb2 )	/* (c) 1988 Nintendo */
	DRIVER( pc_smb3 )	/* (c) 1988 Nintendo */
	DRIVER( pc_mman3 )	/* (c) 1990 Capcom USA (Nintendo of America license) */
	DRIVER( pc_radr2 )	/* (c) 1990 Square (Nintendo of America license) */
	DRIVER( pc_suprc )	/* (c) 1990 Konami (Nintendo of America license) */
	DRIVER( pc_tmnt2 )	/* (c) 1990 Konami (Nintendo of America license) */
	DRIVER( pc_wcup )	/* (c) 1990 Technos (Nintendo license) */
	DRIVER( pc_ngai2 )	/* (c) 1990 Tecmo (Nintendo of America license) */
	DRIVER( pc_ngai3 )	/* (c) 1991 Tecmo (Nintendo of America license) */
	DRIVER( pc_pwbld )	/* (c) 1991 Taito (Nintendo of America license) */
	DRIVER( pc_rkats )	/* (c) 1991 Atlus (Nintendo of America license) */
	DRIVER( pc_pinbt )	/* (c) 1988 Rare (Nintendo of America license) */
	DRIVER( pc_cshwk )	/* (c) 1989 Rare (Nintendo of America license) */
	DRIVER( pc_sjetm )	/* (c) 1990 Rare */
	DRIVER( pc_moglf )	/* (c) 1991 Nintendo */

	/* Nintendo VS games */
	DRIVER( btlecity )	/* (c) 1985 Namco */
	DRIVER( starlstr )	/* (c) 1985 Namco */
	DRIVER( cstlevna )	/* (c) 1987 Konami */
	DRIVER( cluclu )	/* (c) 1984 Nintendo */
	DRIVER( drmario )	/* (c) 1990 Nintendo */
	DRIVER( duckhunt )	/* (c) 1985 Nintendo */
	DRIVER( excitebk )	/* (c) 1984 Nintendo */
	DRIVER( excitebkj )	/* (c) 1984 Nintendo */
	DRIVER( goonies )	/* (c) 1986 Konami */
	DRIVER( hogalley )	/* (c) 1985 Nintendo */
	DRIVER( iceclimb )	/* (c) 1984 Nintendo */
	DRIVER( iceclimbj )	/* (c) 1984 Nintendo */
	DRIVER( ladygolf )	/* (c) 1984 Nintendo */
	DRIVER( machridr )	/* (c) 1985 Nintendo */
	DRIVER( machridrj )	/* (c) 1985 Nintendo */
	DRIVER( rbibb )		/* (c) 1987 Namco */
	DRIVER( rbibba )	/* (c) 1987 Namco */
	DRIVER( suprmrio )	/* (c) 1986 Nintendo */
	DRIVER( suprmrioa )	/* (c) 1986 Nintendo */
	DRIVER( suprmriobl ) /* bootleg */
	DRIVER( suprmriobl2 )/* bootleg */
	DRIVER( skatekds )	/* (c) 1988 (hack) */
	DRIVER( vsskykid )	/* (c) 1986 Namco */
	DRIVER( tkoboxng )	/* (c) 1987 Data East */
	DRIVER( smgolf )	/* (c) 1984 Nintendo */
	DRIVER( smgolfj )	/* (c) 1984 Nintendo */
	DRIVER( vspinbal )	/* (c) 1984 Nintendo */
	DRIVER( vspinbalj )	/* (c) 1984 Nintendo */
	DRIVER( vsslalom )	/* (c) 1986 Nintendo */
	DRIVER( vssoccer )	/* (c) 1985 Nintendo */
	DRIVER( vssoccerj )	/* (c) 1985 Nintendo */
	DRIVER( vsgradus )	/* (c) 1986 Konami */
	DRIVER( platoon )	/* (c) 1987 Ocean */
	DRIVER( vstetris )	/* (c) 1988 Atari */
	DRIVER( mightybj )	/* (c) 1986 Tecmo */
	DRIVER( jajamaru )	/* (c) 1985 Jaleco */
	DRIVER( topgun )	/* (c) 1987 Konami */
	DRIVER( bnglngby )	/* (c) 1985 Nintendo / Broderbund Software Inc. */
	DRIVER( vstennis )	/* (c) 1984 Nintendo */
	DRIVER( vstennisj )	/* (c) 1984 Nintendo */
	DRIVER( wrecking )	/* (c) 1984 Nintendo */
	DRIVER( balonfgt )	/* (c) 1984 Nintendo */
	DRIVER( vsmahjng )	/* (c) 1984 Nintendo */
	DRIVER( vsbball )	/* (c) 1984 Nintendo */
	DRIVER( vsbballj )	/* (c) 1984 Nintendo */
	DRIVER( vsbballja )	/* (c) 1984 Nintendo */
	DRIVER( vsbballjb )	/* (c) 1984 Nintendo */
	DRIVER( iceclmrj )	/* (c) 1984 Nintendo */
	DRIVER( vsgshoe )	/* (c) 1986 Nintendo */
	DRIVER( supxevs )	/* (c) 1986 Nintendo */
	DRIVER( vsfdf )		/* (c) 1986 Sunsoft */
	DRIVER( smgolfb )	/* (c) 1985 Nintendo */

	/* NES-based hacks */
	DRIVER( multigam )	/* 1992 */
	DRIVER( multigmb )	/* 1992 */
	DRIVER( multigm2 )	/* 1992 */
	DRIVER( multigm3 )	/* 19?? */
	DRIVER( multigmt )
	DRIVER( supergm3 )  /* 1996 */
	DRIVER( cham24 )	/* 2002 */

	/* Nintendo Super System Games */
	DRIVER( nss )
	DRIVER( nss_ssoc )	/* 1992 Human Inc. */
	DRIVER( nss_actr )	/* 1992 Enix */
	DRIVER( nss_con3 )	/* 1992 Konami */
	DRIVER( nss_adam )	/* 1992 Ocean */
	DRIVER( nss_aten )	/* 1992 Absolute Entertainment Inc. */
	DRIVER( nss_rob3 )	/* 1992 Ocean */
	DRIVER( nss_ncaa )	/* 1992 Sculptured Software Inc. */
	DRIVER( nss_skin )	/* 1992 Irem */
	DRIVER( nss_lwep )	/* 1992 Nintendo */
	DRIVER( nss_smw )	/* 1991 Nintendo */
	DRIVER( nss_fzer )	/* 1991 Nintendo */
	DRIVER( nss_sten )	/* 199? Nintendo */

	/* Super Famicom Box Games */
	DRIVER( sfcbox )
	//pss61 - Super Mario Kart / Super Mario Collection / Star Fox
	DRIVER( pss62 )		/* 199? T & E Soft / I'Max */
	//pss63 - Super Donkey Kong / Super Tetris 2 + Bombliss
	//pss64 - Super Donkey Kong / Super Bomberman 2



	/* SNES-based hacks */
	DRIVER( kinstb )	/* bootleg */
	DRIVER( ffight2b )	/* bootleg */
	DRIVER( sblast2b )	/* bootleg */
	DRIVER( iron )		/* bootleg */
	DRIVER( endless )	/* bootleg */
	DRIVER( denseib )	/* bootleg */


	/* Midway 8080 b/w games */
	DRIVER( seawolf )	/* 596 [1976] */
	DRIVER( gunfight )	/* 597 [1975] */
						/* 603 - Top Gun [1976] */
	DRIVER( tornbase )	/* 605 [1976] */
	DRIVER( 280zzzap )	/* 610 [1976] */
	DRIVER( maze )		/* 611 [1976] */
	DRIVER( boothill )	/* 612 [1977] */
	DRIVER( checkmat )	/* 615 [1977] */
	DRIVER( desertgu )	/* 618 [1977] */
	DRIVER( roadrunm )	/* 618 [1977] */
	DRIVER( dplay )		/* 619 [1977] */
	DRIVER( lagunar )	/* 622 [1977] */
	DRIVER( gmissile )	/* 623 [1977] */
	DRIVER( m4 )		/* 626 [1977] */
	DRIVER( clowns )	/* 630 [1978] */
	DRIVER( clowns1 )	/* 630 [1978] */
	DRIVER( spacwalk )	/* 640 [1978] */
	DRIVER( einning )	/* 642 [1978] Midway */
	DRIVER( shuffle )	/* 643 [1978] */
	DRIVER( dogpatch )	/* 644 [1977] */
	DRIVER( spcenctr )	/* 645 (c) 1980 Midway */
	DRIVER( phantom2 )	/* 652 [1979] */
	DRIVER( bowler )	/* 730 [1978] Midway */
	DRIVER( invaders )	/* 739 [1979] */
	DRIVER( blueshrk )	/* 742 [1978] */
	DRIVER( invad2ct )	/* 851 (c) 1980 Midway */
	DRIVER( invadpt2 )	/* 852 [1980] Taito */
	DRIVER( invaddlx )	/* 852 [1980] Midway */
	DRIVER( vortex )	/* 852 [1980] Zilec Electronics Ltd. */
	DRIVER( moonbase )	/* Taito */
	DRIVER( moonbasea )	/* Zeta - Nichibutsu */
						/* 870 - Space Invaders Deluxe cocktail */
	DRIVER( searthin )	/* bootleg */
	DRIVER( searthina )	/* bootleg */
	DRIVER( invadrmr )	/* (c) 1978 Model Racing */
	DRIVER( modelr )	/* (c) 19?? Model Racing */
	DRIVER( spaceatt )	/* (c) 1978 Video Games GMBH */
	DRIVER( spaceat2 )	/* (c) 1980 Zenitone-Microsec Ltd */
	DRIVER( sinvzen )	/* Zenitone-Microsec Ltd */
	DRIVER( superinv )	/* bootleg */
	DRIVER( invasion )	/* Sidam */
	DRIVER( invasiona )	/* bootleg */
	DRIVER( invasionb )	/* bootleg */
	DRIVER( invasionrz ) /* bootleg */
	DRIVER( invasionrza )/* bootleg */
	DRIVER( sstrangr )	/* (c) 1978 Yachiyo Electronics, Ltd. */
	DRIVER( sstrangr2 )	/* (c) 1979 Yachiyo Electronics, Ltd. */
	DRIVER( sinvemag )	/* Zenitone-Microsec Ltd. */
	DRIVER( jspecter )	/* (c) 1979 Jatre */
	DRIVER( jspecter2 )	/* (c) 1979 Jatre */
	DRIVER( invrvnge )	/* Zenitone-Microsec Ltd. */
	DRIVER( invrvngea )	/* Zenitone-Microsec Ltd. (Dutchford license) */
	DRIVER( galxwars )	/* (c) 1979 Universal */
	DRIVER( galxwars2 )	/* (c) 1979 Universal */
	DRIVER( galxwarst )	/* (c) 1979 Taito? */
	DRIVER( starw )		/* bootleg */
	DRIVER( lrescue )	/* LR  (c) 1979 Taito */
	DRIVER( mlander )	/* (c) 1980 Leisure Time Electronics */
	DRIVER( lrescuem )	/* Model Racing bootleg */
	DRIVER( grescue )	/* bootleg? */
	DRIVER( desterth )	/* bootleg */
	DRIVER( cosmicmo )	/* Universal */
	DRIVER( cosmicm2 )	/* Universal */
	DRIVER( rollingc )	/* Nichibutsu */
	DRIVER( ozmawars )	/* Shin Nihon Kikaku (SNK) */
	DRIVER( ozmawars2 )	/* Shin Nihon Kikaku (SNK) */
	DRIVER( solfight )	/* bootleg */
	DRIVER( spaceph )	/* Zilec Games */
	DRIVER( schaser )	/* RT  Taito */
	DRIVER( schasercv )	/* RT  Taito */
	DRIVER( lupin3 )	/* LP  (c) 1980 Taito */
	DRIVER( lupin3a )	/* LP  (c) 1980 Taito */
	DRIVER( intruder )	/* (c) 1980 GamePlan (Taito) */
	DRIVER( spclaser )	/* (c) 1980 GamePlan (Taito) */
	DRIVER( laser )		/* (c) 1980 Leisure Time Electronics Inc. */
	DRIVER( spcewarl )	/* (c) 1979 Leijac Corporation (Konami) */
	DRIVER( polaris )	/* PS  (c) 1980 Taito */
	DRIVER( polarisa )	/* PS  (c) 1980 Taito */
	DRIVER( polariso )	/* PS  (c) 1980 Taito */
	DRIVER( ballbomb )	/* TN  (c) 1980 Taito */
	DRIVER( indianbt )	/* (c) 1980 Taito */
	DRIVER( steelwkr )	/* (c) 1980 Taito */
	DRIVER( m79amb )	/* (c) 1977 RamTek */
	DRIVER( alieninv )	/* Margamatics */
	DRIVER( alieninvp2 )	/* bootleg */
	DRIVER( tst_invd )	/* Test ROM */
	DRIVER( sitv )		/* (c) 1979 Taito */
	DRIVER( sicv )		/* (c) 1979 Taito */
	DRIVER( sisv )		/* (c) 1978 Taito */
	DRIVER( sisv2 )		/* (c) 1978 Taito */
	DRIVER( spacewr3 )	/* bootleg */
	DRIVER( invaderl )	/* bootleg */
	DRIVER( invader4 )	/* bootleg */
	DRIVER( yosakdon )	/* (c) 1979 Yosaku To Donbei */
	DRIVER( yosakdona )	/* (c) 1979 Yosaku To Donbei */
	DRIVER( spceking )	/* (c) 1978 Leijac Corporation (Konami) */
	DRIVER( spcewars )	/* (c) 1978 Sanritsu */
	DRIVER( astropal )	/* (c) 19?? Sidam */
	DRIVER( cosmo )		/* TDS+Mints */
	DRIVER( darthvdr )	/* bootleg */

	DRIVER( sspeedr )	/* 1979 Midway */

	/* Similar but with a M6800 instead of an 8080 */
	DRIVER( sflush )	/* (c)1979 Taito */

	/* Meadows S2650 games */
	DRIVER( lazercmd )	/* [1976?] */
	DRIVER( bbonk )		/* [1976?] */
	DRIVER( deadeye )	/* [1978?] */
	DRIVER( bowl3d )	/* [1978?] */
	DRIVER( gypsyjug )	/* [1978?] */
	DRIVER( minferno )	/* [1978?] */
	DRIVER( medlanes )	/* [1977?] */

	/* CVS games */
	DRIVER( cosmos )	/* (c) 1981 Century */
	DRIVER( darkwar )	/* (c) 1981 Century */
	DRIVER( spacefrt )	/* (c) 1981 Century */
	DRIVER( 8ball )		/* (c) 1982 Century */
	DRIVER( 8ball1 )	/* (c) 1982 Century */
	DRIVER( logger )	/* (c) 1982 Century */
	DRIVER( dazzler )	/* (c) 1982 Century */
	DRIVER( diggerc )	/* (c) 1982 Century */
	DRIVER( wallst )	/* (c) 1982 Century */
	DRIVER( radarzon )	/* (c) 1982 Century */
	DRIVER( radarzon1 )	/* (c) 1982 Century */
	DRIVER( radarzont )	/* (c) 1982 Tuni Electro Service */
	DRIVER( outline )	/* (c) 1982 Century */
	DRIVER( goldbug )	/* (c) 1982 Century */
	DRIVER( heartatk )	/* (c) 1983 Century Electronics */
	DRIVER( hunchbak )	/* (c) 1983 Century */
	DRIVER( hunchbaka )	/* (c) 1983 Century */
	DRIVER( superbik )	/* (c) 1983 Century */
	DRIVER( raiders )	/* (c) 1983 Century Electronics UK */
	DRIVER( hero )		/* (c) 1983 Seatongrove (c) 1984 CVS */
	DRIVER( huncholy )	/* (c) 1984 Seatongrove (c) CVS */

	/* Zaccaria Hardware (similar to CVS) */
	DRIVER( quasar )	/* (c) 1980 Zelco Games Italy */
	DRIVER( quasara )	/* (c) 1980 Zelco Games Italy */

	/* Midway "Astrocade" games */
	DRIVER( seawolf2 )	/* (c) 1978 */
	DRIVER( spacezap )	/* (c) 1980 */
	DRIVER( ebases )	/* (c) 1980 */
	DRIVER( wow )		/* (c) 1980 */
	DRIVER( wowg )		/* (c) 1980 */
	DRIVER( gorf )		/* (c) 1981 */
	DRIVER( gorfpgm1 )	/* (c) 1981 */
	DRIVER( gorfpgm1g )	/* (c) 1981 */
	DRIVER( robby )		/* (c) 1981 Bally Midway */
	DRIVER( demndrgn )	/* (c) 1982 Bally Midway */
	DRIVER( profpac )	/* (c) 1983 Bally Midway */
	DRIVER( tenpindx )	/* (c) 1983 Bally Midway */

	/* Bally Midway MCR games */
	/* MCR1 */
	DRIVER( solarfox )	/* (c) 1981 */
	DRIVER( kick )		/* (c) 1981 */
	DRIVER( kickman )	/* (c) 1981 */
	DRIVER( kickc )		/* (c) 1981 */
	/* MCR2 */
	DRIVER( shollow )	/* (c) 1981 */
	DRIVER( shollow2 )	/* (c) 1981 */
	DRIVER( tron )		/* (c) 1982 */
	DRIVER( tron2 )		/* (c) 1982 */
	DRIVER( tron3 )		/* (c) 1982 */
	DRIVER( tron4 )		/* (c) 1982 */
	DRIVER( kroozr )	/* (c) 1982 */
	DRIVER( domino )	/* (c) 1982 */
	DRIVER( wacko )		/* (c) 1982 */
	DRIVER( twotiger )	/* (c) 1984 */
	DRIVER( twotigerc )	/* (c) 1984 */
	/* MCR2 + MCR3 sprites */
	DRIVER( journey )	/* (c) 1983 */
	/* MCR3 */
	DRIVER( tapper )	/* (c) 1983 */
	DRIVER( tappera )	/* (c) 1983 */
	DRIVER( sutapper )	/* (c) 1983 */
	DRIVER( rbtapper )	/* (c) 1984 */
	DRIVER( timber )	/* (c) 1984 */
	DRIVER( dotron )	/* (c) 1983 */
	DRIVER( dotrona )	/* (c) 1983 */
	DRIVER( dotrone )	/* (c) 1983 */
	DRIVER( nflfoot )	/* (c) 1983 + laserdisk */
	DRIVER( demoderb )	/* (c) 1984 */
	DRIVER( demoderm )	/* (c) 1984 */
	DRIVER( sarge )		/* (c) 1985 */
	DRIVER( rampage )	/* (c) 1986 */
	DRIVER( rampage2 )	/* (c) 1986 */
	DRIVER( powerdrv )	/* (c) 1986 */
	DRIVER( stargrds )	/* (c) 1987 */
	DRIVER( maxrpm )	/* (c) 1986 */
	DRIVER( spyhunt )	/* (c) 1983 */
	DRIVER( spyhuntp )	/* (c) 1983 */
	DRIVER( turbotag )	/* (c) 1985 */
	DRIVER( crater )	/* (c) 1984 */
	/* MCR 68000 */
	DRIVER( zwackery )	/* (c) 1984 */
	DRIVER( xenophob )	/* (c) 1987 */
	DRIVER( spyhunt2 )	/* (c) 1987 */
	DRIVER( spyhunt2a )	/* (c) 1987 */
	DRIVER( intlaser )	/* (c) 1987 */
	DRIVER( blasted )	/* (c) 1988 */
	DRIVER( archrivl )	/* (c) 1989 */
	DRIVER( archrivl2 )	/* (c) 1989 */
	DRIVER( trisport )	/* (c) 1989 */
	DRIVER( pigskin )	/* (c) 1990 */

	/* Bally / Sente games */
	DRIVER( sentetst )
	DRIVER( cshift )	/* (c) 1984 */
	DRIVER( gghost )	/* (c) 1984 */
	DRIVER( hattrick )	/* (c) 1984 */
	DRIVER( otwalls )	/* (c) 1984 */
	DRIVER( snakepit )	/* (c) 1984 */
	DRIVER( snakjack )	/* (c) 1984 */
	DRIVER( stocker )	/* (c) 1984 */
	DRIVER( triviag1 )	/* (c) 1984 */
	DRIVER( trivia12 )	/* (c) 1984 */
	DRIVER( triviag2 )	/* (c) 1984 */
	DRIVER( triviasp )	/* (c) 1984 */
	DRIVER( triviayp )	/* (c) 1984 */
	DRIVER( triviabb )	/* (c) 1984 */
	DRIVER( triviaes )	/* (c) 1984 */
	DRIVER( gimeabrk )	/* (c) 1985 */
	DRIVER( minigolf )	/* (c) 1985 */
	DRIVER( minigolf2 )	/* (c) 1985 */
	DRIVER( toggle )	/* (c) 1985 */
	DRIVER( nametune )	/* (c) 1986 */
	DRIVER( nametune2 )	/* (c) 1986 */
	DRIVER( nstocker )	/* (c) 1986 */
	DRIVER( nstocker2 )	/* (c) 1986 */
	DRIVER( sfootbal )	/* (c) 1986 */
	DRIVER( spiker )	/* (c) 1986 */
	DRIVER( stompin )	/* (c) 1986 */
	DRIVER( rescraid )	/* (c) 1987 */
	DRIVER( rescraida )	/* (c) 1987 */
	DRIVER( grudge )	/* prototype - Bally/Midway */
	DRIVER( shrike )	/* (c) 1987 */
	DRIVER( gridlee )	/* [1983 Videa] prototype - no copyright notice */

	/* Irem games */
	/* trivia: IREM means "International Rental Electronics Machines" */
	DRIVER( andromed )	/* (c) 1979 */
	DRIVER( ipminvad )	/* M10 no copyright notice (Arcade TV Game List - P.67, Left, 26 from top) */
	DRIVER( ipminvad1 )	/* M10 incomplete dump */
	DRIVER( ptrmj )		/* M14 (c) 1979  */
	DRIVER( skychut )	/* Irem [1980] */
	DRIVER( spacbeam )	/* M15 no copyright notice */
	DRIVER( greenber )	/* Irem */
	DRIVER( headoni )	/* Irem no copyright notice (1979 - Arcade TV Game List - P.98, Left, 7 from top ) */

	DRIVER( panther )	/* M27 no copyright notice */
	DRIVER( redalert )	/* M27 (c) 1981 + "GDI presents" */
	DRIVER( demoneye )	/* M27 (c) 1981 */
	DRIVER( ww3 )		/* M27 (c) 1981 */

	DRIVER( olibochu )	/* M47 (c) 1981 + "GDI presents" */
	DRIVER( mpatrol )	/* M52 (c) 1982 */
	DRIVER( mpatrolw )	/* M52 (c) 1982 + Williams license */
	DRIVER( alpha1v )	/* Vision Electronics */
	DRIVER( troangel )	/* (c) 1983 */
	DRIVER( newtangl )	/* (c) 1983 */
	DRIVER( 10yard )	/* (c) 1983 */
	DRIVER( 10yardj )	/* (c) 1983 */
	DRIVER( vs10yard )	/* (c) 1983/1984 */
	DRIVER( vs10yardj )	/* (c) 1983/1984 */
	DRIVER( vs10yardu ) /* (c) 1983/1984 */
	DRIVER( 10yard85 )	/* (c) 1985 */
	DRIVER( travrusa )	/* (c) 1983 */
	DRIVER( motorace )	/* (c) 1983 Williams license */
	DRIVER( shtrider )	/* (c) 1984 Seibu Kaihatsu */
	DRIVER( shtridera )	/* (c) 1984 Seibu Kaihatsu */
	DRIVER( wilytowr )	/* M63 (c) 1984 */
	DRIVER( atomboy )	/* M63 (c) 1985 Irem + Memetron license */
	DRIVER( atomboya )	/* M63 (c) 1985 Irem + Memetron license */
	DRIVER( fghtbskt )	/* (c) 1984 Paradise Co. Ltd. */
	/* M62 */
	DRIVER( kungfum )	/* (c) 1984 */
	DRIVER( kungfumd )	/* (c) 1984 + Data East license */
	DRIVER( spartanx )	/* (c) 1984 (Japan) */
	DRIVER( kungfub )	/* bootleg */
	DRIVER( kungfub2 )	/* bootleg */
	DRIVER( battroad )	/* (c) 1984 */
	DRIVER( ldrun )		/* (c) 1984 licensed from Broderbund */
	DRIVER( ldruna )	/* (c) 1984 licensed from Broderbund */
	DRIVER( ldrun2 )	/* (c) 1984 licensed from Broderbund */
	DRIVER( ldrun3 )	/* (c) 1985 licensed from Broderbund */
	DRIVER( ldrun3j )	/* (c) 1985 licensed from Broderbund */
	DRIVER( ldrun4 )	/* (c) 1986 licensed from Broderbund */
	DRIVER( lotlot )	/* (c) 1985 licensed from Tokuma Shoten */
	DRIVER( kidniki )	/* (c) 1986 + Data East USA license */
	DRIVER( kidnikiu )	/* (c) 1986 + Data East USA license */
	DRIVER( yanchamr )	/* (c) 1986 (Japan) */
	DRIVER( lithero )	/* bootleg */
	DRIVER( spelunkr )	/* (c) 1985 licensed from Broderbund */
	DRIVER( spelunkrj )	/* (c) 1985 licensed from Broderbund */
	DRIVER( spelunk2 )	/* (c) 1986 licensed from Broderbund */
	DRIVER( horizon )	/* (c) 1985 */
	DRIVER( youjyudn )	/* (c) 1986 (Japan) */

	DRIVER( vigilant )	/* (c) 1988 (World) */
	DRIVER( vigilantu )	/* (c) 1988 (US) */
	DRIVER( vigilantj )	/* (c) 1988 (Japan) */
	DRIVER( kikcubic )	/* (c) 1988 (Japan) */
	DRIVER( kikcubicb )	/* bootleg */
	DRIVER( buccanrs )	/* (c) 1989 Duintronic */
	DRIVER( buccanrsa )	/* (c) 1989 Duintronic */
	/* M72 (and derivatives) */
	DRIVER( rtype )		/* (c) 1987 (Japan) */
	DRIVER( rtypej )	/* (c) 1987 (Japan) */
	DRIVER( rtypejp )	/* (c) 1987 (Japan) */
	DRIVER( rtypeu )	/* (c) 1987 + Nintendo USA license (US) */
	DRIVER( rtypeb )	/* bootleg  */
	DRIVER( bchopper )	/* (c) 1987 */
	DRIVER( mrheli )	/* (c) 1987 (Japan) */
	DRIVER( nspirit )	/* (c) 1988 */
	DRIVER( nspiritj )	/* (c) 1988 (Japan) */
	DRIVER( imgfight )	/* (c) 1988 (Japan) */
	DRIVER( imgfighto )	/* (c) 1988 (Japan) */
	DRIVER( loht )		/* (c) 1989 */
	DRIVER( lohtj )		/* (c) 1989 (Japan) */
	DRIVER( lohtb )		/* (c) 1989 (bootleg) */
	DRIVER( lohtb2 )	/* (c) 1989 (bootleg) */
	DRIVER( xmultipl )	/* (c) 1989 (Japan) */
	DRIVER( xmultiplm72 )	/* (c) 1989 (Japan) */
	DRIVER( dbreed )	/* (c) 1989 */
	DRIVER( dbreedm72 )	/* (c) 1989 */
	DRIVER( rtype2 )	/* (c) 1989 */
	DRIVER( rtype2j )	/* (c) 1989 (Japan) */
	DRIVER( rtype2jc )	/* (c) 1989 (Japan) */
	DRIVER( majtitle )	/* (c) 1990 (World) */
	DRIVER( majtitlej )	/* (c) 1990 (Japan) */
	DRIVER( hharry )	/* (c) 1990 (World) */
	DRIVER( hharryu )	/* (c) 1990 Irem America (US) */
	DRIVER( dkgensan )	/* (c) 1990 (Japan) */
	DRIVER( dkgensanm72 )	/* (c) 1990 (Japan) */
	DRIVER( poundfor )	/* (c) 1990 (World) */
	DRIVER( poundforj )	/* (c) 1990 (Japan) */
	DRIVER( poundforu )	/* (c) 1990 Irem America (US) */
	DRIVER( airduel )	/* (c) 1990 (Japan) */
	DRIVER( cosmccop )	/* (c) 1991 (World) */
	DRIVER( gallop )	/* (c) 1991 (Japan) */
	DRIVER( kengo )		/* (c) 1991 */
	/* not M72, but same sound hardware */
	DRIVER( sichuan2 )	/* (c) 1989 Tamtex */
	DRIVER( sichuan2a )	/* (c) 1989 Tamtex */
	DRIVER( shisen )	/* (c) 1989 Tamtex */
	DRIVER( matchit )	/* (c) 1989 Tamtex */
	/* M90 */
	DRIVER( hasamu )	/* (c) 1991 Irem (Japan) */
	DRIVER( dynablst )	/* (c) 1991 Irem (World) */
	DRIVER( dynablstb )	/* bootleg */
	DRIVER( bombrman )	/* (c) 1991 Irem (Japan) */
	DRIVER( atompunk )	/* (c) 1991 Irem America (licensed from Hudson Soft) */
	/* M97 */
	DRIVER( bbmanw )	/* (c) 1992 Irem (World) */
	DRIVER( bbmanwj )	/* (c) 1992 Irem (Japan) */
	DRIVER( bomblord )	/* bootleg */
	DRIVER( newapunk )	/* (c) 1992 Irem America (US) */
	DRIVER( quizf1 )	/* (c) 1992 Irem (Japan) */
	DRIVER( riskchal )	/* (c) 1993 Irem */
	DRIVER( gussun )	/* (c) 1993 Irem (Japan) */
	DRIVER( matchit2 )	/* (c) 1993 Tamtex */
	DRIVER( shisen2 )	/* (c) 1993 Tamtex */
	/* M92 */
	DRIVER( gunforce )	/* (c) 1991 Irem (World) */
	DRIVER( gunforceu )	/* (c) 1991 Irem America (US) */
	DRIVER( gunforcej )	/* (c) 1991 Irem (Japan) */
	DRIVER( bmaster )	/* (c) 1991 Irem */
	DRIVER( crossbld )	/* (c) 1991 Irem (Japan) */
	DRIVER( lethalth )	/* (c) 1991 Irem (World) */
	DRIVER( thndblst )	/* (c) 1991 Irem (Japan) */
	DRIVER( uccops )	/* (c) 1992 Irem (World) */
	DRIVER( uccopsu )	/* (c) 1992 Irem (US) */
	DRIVER( uccopsar )	/* (c) 1992 Irem (World) */
	DRIVER( uccopsj )	/* (c) 1992 Irem (Japan) */
	DRIVER( mysticri )	/* (c) 1992 Irem (World) */
	DRIVER( mysticrib )	/* bootleg? */
	DRIVER( gunhohki )	/* (c) 1992 Irem (Japan) */
	DRIVER( majtitl2 )	/* (c) 1992 Irem (World) */
	DRIVER( majtitl2j )	/* (c) 1992 Irem (Japan) */
	DRIVER( skingame )	/* (c) 1992 Irem America (US) */
	DRIVER( skingame2 )	/* (c) 1992 Irem America (US) */
	DRIVER( hook )		/* (c) 1992 Irem (World) */
	DRIVER( hooku )		/* (c) 1992 Irem America (US) */
	DRIVER( hookj )		/* (c) 1992 Irem (Japan) */
	DRIVER( ppan )		/* bootleg */
	DRIVER( rtypeleo )	/* (c) 1992 Irem (World) */
	DRIVER( rtypeleoj )	/* (c) 1992 Irem (Japan) */
	DRIVER( inthunt )	/* (c) 1993 Irem (World) */
	DRIVER( inthuntu )	/* (c) 1993 Irem (US) */
	DRIVER( kaiteids )	/* (c) 1993 Irem (Japan) */
	DRIVER( nbbatman )	/* (c) 1993 Irem America (US) */
	DRIVER( leaguemn )	/* (c) 1993 Irem (Japan) */
	DRIVER( ssoldier )	/* (c) 1993 Irem America (US) */
	DRIVER( psoldier )	/* (c) 1993 Irem (Japan) */
	DRIVER( dsoccr94j )	/* (c) 1994 Irem (Japan) */
	DRIVER( gunforc2 )	/* (c) 1994 Irem */
	DRIVER( geostorm )	/* (c) 1994 Irem (Japan) */
	/* M107 */
	DRIVER( firebarr )	/* (c) 1993 Irem (Japan) */
	DRIVER( dsoccr94 )	/* (c) 1994 Irem (Data East Corporation license) */
	DRIVER( kftgoal )	/* (c) 1994 Jaleco */
	DRIVER( wpksoc )	/* (c) 1995 Jaleco */

	/* Gottlieb/Mylstar games (Gottlieb became Mylstar in 1983) */
	DRIVER( reactor )	/* GV-100 (c) 1982 Gottlieb */
	DRIVER( mplanets )	/* GV-102 (c) 1983 Gottlieb */
	DRIVER( mplanetsuk )/* GV-102 (c) 1983 Gottlieb */
	DRIVER( qbert )		/* GV-103 (c) 1982 Gottlieb */
	DRIVER( qberta )	/* GV-103 (c) 1982 Gottlieb */
	DRIVER( qbertj )	/* GV-103 (c) 1982 Gottlieb + Konami license */
	DRIVER( myqbert )	/* GV-103?(c) 1982 Gottlieb */
	DRIVER( qberttst )	/* GV-103 (c) 1982 Gottlieb */
	DRIVER( qbtrktst )	/* GV-103 (c) 1982 Gottlieb */
	DRIVER( insector )	/* GV-??? (c) 1982 Gottlieb - never released */
	DRIVER( tylz )		/* GV-??? (c) 1982 Gottlieb - never released */
	DRIVER( argusg )	/* GV-??? (c) 1982 Gottlieb - never released */
	DRIVER( krull )		/* GV-105 (c) 1983 Gottlieb */
	DRIVER( kngtmare )	/* GV-??? (c) 1983 Gottlieb - never released */
	DRIVER( sqbert )	/* GV-??? (c) 1983 Mylstar - never released */
	DRIVER( mach3 )		/* GV-109 (c) 1983 Mylstar */
	DRIVER( cobram3 )	/* ...... (c) 1984 Data East */
	DRIVER( usvsthem )	/* GV-??? (c) 198? Mylstar */
	DRIVER( 3stooges )	/* GV-113 (c) 1984 Mylstar */
	DRIVER( qbertqub )	/* GV-119 (c) 1983 Mylstar */
	DRIVER( screwloo )	/* GV-123 (c) 1983 Mylstar - never released */
	DRIVER( curvebal )	/* GV-134 (c) 1984 Mylstar */
	DRIVER( vidvince )	/* GV-??? (c) 1984 Mylstar - never released  */
	DRIVER( wizwarz )	/* GV-??? (c) 1984 Mylstar - never released */

	/* Taito "Qix hardware" games */
	DRIVER( qix )		/* LK  (c) 1981 Taito America Corporation */
	DRIVER( qixa )		/* LK  (c) 1981 Taito America Corporation */
	DRIVER( qixb )		/* LK  (c) 1981 Taito America Corporation */
	DRIVER( qixo )		/* LK  (c) 1981 Taito America Corporation */
	DRIVER( qix2 )		/* ??  (c) 1981 Taito America Corporation */
	DRIVER( sdungeon )	/* SD  (c) 1981 Taito America Corporation */
	DRIVER( elecyoyo )	/* YY  (c) 1982 Taito America Corporation */
	DRIVER( elecyoyo2 )	/* YY  (c) 1982 Taito America Corporation */
	DRIVER( kram )		/* KS  (c) 1982 Taito America Corporation */
	DRIVER( kram2 )		/* KS  (c) 1982 Taito America Corporation */
	DRIVER( kram3 )		/* KS  (c) 1982 Taito America Corporation */
	DRIVER( zookeep )	/* ZA  (c) 1982 Taito America Corporation */
	DRIVER( zookeep2 )	/* ZA  (c) 1982 Taito America Corporation */
	DRIVER( zookeep3 )	/* ZA  (c) 1982 Taito America Corporation */
	DRIVER( slither )	/* (c) 1982 Century II */
	DRIVER( slithera )	/* (c) 1982 Century II */
	DRIVER( complexx )	/* CX  (c) 1984 Taito America Corporation */

	/* Taito SJ System games */
	DRIVER( spaceskr )	/* EB  (c) 1981 Taito Corporation */
	DRIVER( spacecr )	/* CG  (c) 1981 Taito Corporation */
	DRIVER( junglek )	/* KN  (c) 1982 Taito Corporation */
	DRIVER( junglekj2 )	/* KN  (c) 1982 Taito Corporation */
	DRIVER( jungleh )	/* KN  (c) 1982 Taito America Corporation */
	DRIVER( junglehbr )	/* KN  (c) 1982 Taito do Brasil */
	DRIVER( piratpet )	/* KN  (c) 1982 Taito America Corporation */
	DRIVER( alpine )	/* RH  (c) 1982 Taito Corporation */
	DRIVER( alpinea )	/* RH  (c) 1982 Taito Corporation */
	DRIVER( timetunl )	/* UN  (c) 1982 Taito Corporation */
	DRIVER( wwestern )	/* WW  (c) 1982 Taito Corporation */
	DRIVER( wwestern1 )	/* WW  (c) 1982 Taito Corporation */
	DRIVER( frontlin )	/* FL  (c) 1982 Taito Corporation */
	DRIVER( elevator )	/* EA  (c) 1983 Taito Corporation */
	DRIVER( elevatorb )	/* bootleg */
	DRIVER( tinstar )	/* A10 (c) 1983 Taito Corporation */
	DRIVER( tinstar2 )	/* A10 (c) 1983 Taito Corporation */
	DRIVER( waterski )	/* A03 (c) 1983 Taito Corporation */
	DRIVER( bioatack )	/* AA8 (c) 1983 Taito Corporation + Fox Video Games license */
	DRIVER( hwrace )	/* AC4 (c) 1983 Taito Corporation */
	DRIVER( sfposeid )	/* A14 (c) 1984 Taito Corporation */
	DRIVER( kikstart )	/* A20 (c) 1984 Taito Corporation */

	/* other Taito games */
	DRIVER( fgoal )		/* TF  (c) 1979 Taito Corporation */
	DRIVER( fgoala )	/* MF  (c) 1979 Taito Corporation */
	DRIVER( crbaloon )	/* CL  (c) 1980 Taito Corporation */
	DRIVER( crbaloon2 )	/* CL  (c) 1980 Taito Corporation */
	DRIVER( sbowling )	/* KB  (c) 1982 Taito Corporation */
	DRIVER( grchamp )	/* GM  (c) 1981 Taito Corporation */
	DRIVER( marinedt )	/* MG  (c) 1981 Taito Corporation */
	DRIVER( changela )	/* ??? (c) 1983 Taito Corporation */
	DRIVER( fspiderb )	/* bootleg */
	DRIVER( jollyjgr )	/* KD  (c) 1982 Taito Corporation */
	DRIVER( bking )		/* DM  (c) 1982 Taito Corporation */
	DRIVER( bking2 )	/* AD6 (c) 1983 Taito Corporation */
	DRIVER( bking3 )	/* A24 (c) 1984 Taito Corporation */
	DRIVER( chaknpop )	/* A04 (c) 1983 Taito Corporation */
	DRIVER( josvolly )	/* AA  (c) 1983 Taito Corporation */
	DRIVER( gsword )	/* AC  (c) 1984 Taito Corporation */
	DRIVER( gsword2 )	/* AC  (c) 1984 Taito Corporation */
	DRIVER( cyclemb )	/* P0  (c) 1984 Taito Corporation [+ Seta] */
	DRIVER( pitnrun )	/* PR  (c) 1984 Taito Corporation */
	DRIVER( pitnruna )	/* PR  (c) 1984 Taito Corporation */
	DRIVER( lkage )		/* A54 (c) 1984 Taito Corporation */
	DRIVER( lkageo )	/* A54 (c) 1984 Taito Corporation */
	DRIVER( lkageoo )	/* A54 (c) 1984 Taito Corporation */
	DRIVER( lkageb )	/* bootleg */
	DRIVER( lkageb2 )	/* bootleg */
	DRIVER( lkageb3 )	/* bootleg */
	DRIVER( msisaac )	/* A34 (c) 1985 Taito Corporation */
	DRIVER( retofinv )	/* A37 (c) 1985 Taito Corporation */
	DRIVER( retofinv1 )	/* bootleg */
	DRIVER( retofinv2 )	/* bootleg */
	DRIVER( fightrol )	/* (c) 1983 Taito */
	DRIVER( rollace )	/* (c) 1983 Williams */
	DRIVER( rollace2 )	/* (c) 1983 Williams */
	DRIVER( lgp )		/* (c) 1983 Taito */
	DRIVER( vsgongf )	/* (c) 1984 Kaneko */
	DRIVER( ringfgt )	/* (c) 1984 Taito */
	DRIVER( ringfgt2 )	/* (c) 1984 Taito */
	DRIVER( fieldday )	/* A23 (c) 1984 Taito */
	DRIVER( undoukai )	/* A17 (c) 1984 Taito */
	DRIVER( 40love )	/* A30 (c) 1984 Taito */
	DRIVER( tsamurai )	/* A35 (c) 1985 Taito */
	DRIVER( tsamurai2 )	/* A35 (c) 1985 Taito */
	DRIVER( tsamuraih )	/* A35 (c) 1985 Taito */
	DRIVER( ladymstr )	/* A49 (c) 1985 Taito */
	DRIVER( nunchaku )	/* ??? (c) 1985 Taito */
	DRIVER( yamagchi )	/* A38 (c) 1985 Taito */
	DRIVER( m660 )		/* ??? (c) 1986 Taito America Corporation */
	DRIVER( m660j )		/* ??? (c) 1986 Taito Corporation (Japan) */
	DRIVER( m660b )		/* bootleg */
	DRIVER( alphaxz )	/* AZ  (c) 1986 Ed/Wood Place */
	DRIVER( buggychl )	/* A22 (c) 1984 Taito Corporation */
	DRIVER( buggychlt )	/* A22 (c) 1984 Taito Corporation + Tefri license */
	DRIVER( ssrj )		/* A40 (c) 1985 Taito Corporation */
	DRIVER( bigevglf )	/* A67 (c) 1986 Taito America Corporation (US) */
	DRIVER( bigevglfj )	/* A67 (c) 1986 Taito Corporation (Japan) */
	DRIVER( flstory )	/* A45 (c) 1985 Taito Corporation */
	DRIVER( flstoryj )	/* A45 (c) 1985 Taito Corporation (Japan) */
	DRIVER( onna34ro )	/* A52 (c) 1985 Taito Corporation (Japan) */
	DRIVER( onna34roa )	/* A52 (c) 1985 Taito Corporation (Japan) */
	DRIVER( victnine )	/* A16 (c) 1985 Taito Corporation (Japan) */
	DRIVER( gladiatr )	/* QB  (c) 1986 Taito America Corporation (US) */
	DRIVER( ogonsiro )	/* QB  (c) 1986 Taito Corporation (Japan) */
	DRIVER( greatgur )	/* QB  (c) 1986 Taito Corporation (Japan?) */
	DRIVER( ppking )	/* QO  (c) 1985 Taito America Corporation (US) */
	DRIVER( nycaptor )	/* A50 (c) 1985 Taito Corporation */
	DRIVER( cyclshtg )	/* A97 (c) 1986 Taito Corporation */
	DRIVER( bronx )		/* bootleg */
	DRIVER( colt )		/* bootleg */
	DRIVER( ksayakyu )	/* ??? (c) 1985 Taito Corporation */
	DRIVER( benberob )	/* A26 */
	DRIVER( halleys )	/* A62 (c) 1986 Taito America Corporation + Coin It (US) */
	DRIVER( halleysc )	/* A62 (c) 1986 Taito Corporation (Japan) */
	DRIVER( halleycj )	/* A62 (c) 1986 Taito Corporation (Japan) */
	DRIVER( halley87 )	/* A62 (c) 1986 Taito Corporation (Japan) */
	DRIVER( lsasquad )	/* A64 (c) 1986 Taito Corporation / Taito America (dip switch) */
	DRIVER( storming )	/* A64 (c) 1986 Taito Corporation */
	DRIVER( daikaiju )	/* A74 (c) 1986 Taito */
	DRIVER( tokio )		/* A71 1986 */
	DRIVER( tokioo )	/* A71 1986 */
	DRIVER( tokiou )	/* A71 1986 */
	DRIVER( tokiob )	/* bootleg */
	DRIVER( bublbobl )	/* A78 (c) 1986 Taito Corporation */
	DRIVER( bublbobl1 )	/* A78 (c) 1986 Taito Corporation */
	DRIVER( bublboblr )	/* A78 (c) 1986 Taito America Corporation + Romstar license */
	DRIVER( bublboblr1 )/* A78 (c) 1986 Taito America Corporation + Romstar license */
	DRIVER( boblbobl )	/* bootleg */
	DRIVER( sboblboa )	/* bootleg */
	DRIVER( sboblbob )	/* bootleg */
	DRIVER( bub68705 )	/* bootleg */
	DRIVER( dland )		/* bootleg */
	DRIVER( missb2 )	/* bootleg on enhanced hardware */
	DRIVER( kikikai )	/* A85 (c) 1986 Taito Corporation */
	DRIVER( knightb )	/* bootleg */
	DRIVER( kicknrun )	/* A87 (c) 1986 Taito Corporation */
	DRIVER( kicknrunu )	/* A87 (c) 1986 Taito Corporation */
	DRIVER( mexico86 )	/* bootleg (Micro Research) */
	DRIVER( darius )	/* A96 (c) 1986 Taito Corporation Japan (World) */
	DRIVER( dariusj )	/* A96 (c) 1986 Taito Corporation (Japan) */
	DRIVER( dariuso )	/* A96 (c) 1986 Taito Corporation (Japan) */
	DRIVER( dariuse )	/* A96 (c) 1986 Taito Corporation (Japan) */
	DRIVER( rastan )	/* B04 (c) 1987 Taito Corporation Japan (World) */
	DRIVER( rastanu )	/* B04 (c) 1987 Taito America Corporation (US) */
	DRIVER( rastanu2 )	/* B04 (c) 1987 Taito America Corporation (US) */
	DRIVER( rastsaga )	/* B04 (c) 1987 Taito Corporation (Japan)*/
	DRIVER( rastsaga1 )	/* B04 (c) 1987 Taito Corporation (Japan)*/
	DRIVER( topspeed )	/* B14 (c) 1987 Taito Corporation Japan (World) */
	DRIVER( topspeedu )	/* B14 (c) 1987 Taito America Corporation (US) */
	DRIVER( fullthrl )	/* B14 (c) 1987 Taito Corporation (Japan) */
	DRIVER( opwolf )	/* B20 (c) 1987 Taito Corporation Japan (World) */
	DRIVER( opwolfa )	/* B20 (c) 1987 Taito Corporation Japan (World) */
	DRIVER( opwolfj )	/* B20 (c) 1987 Taito Corporation (Japan) */
	DRIVER( opwolfu )	/* B20 (c) 1987 Taito America Corporation (US) */
	DRIVER( opwolfb )	/* bootleg */
	DRIVER( othunder )	/* B67 (c) 1988 Taito Corporation Japan (World) */
	DRIVER( othunderu )	/* B67 (c) 1988 Taito America Corporation (US) */
	DRIVER( othunderuo )	/* B67 (c) 1988 Taito America Corporation (US) */
	DRIVER( othunderj )	/* B67 (c) 1988 Taito Corporation (Japan) */
	DRIVER( rainbow )	/* B22 (c) 1987 Taito Corporation */
	DRIVER( rainbowo )	/* B22 (c) 1987 Taito Corporation */
	DRIVER( rainbowe )	/* B39 (c) 1988 Taito Corporation */
	DRIVER( jumping )	/* bootleg */
	DRIVER( arkanoid )	/* A75 (c) 1986 Taito Corporation Japan (World) */
	DRIVER( arkanoidu )	/* A75 (c) 1986 Taito America Corporation + Romstar license (US) */
	DRIVER( arkanoiduo )	/* A75 (c) 1986 Taito America Corporation + Romstar license (US) */
	DRIVER( arkanoidj )	/* A75 (c) 1986 Taito Corporation (Japan) */
	DRIVER( arkmcubl )	/* bootleg */
	DRIVER( ark1ball )	/* bootleg */
	DRIVER( arkangc )	/* bootleg */
	DRIVER( arkangc2 )	/* bootleg */
	DRIVER( arkblock )	/* bootleg */
	DRIVER( arkbloc2 )	/* bootleg */
	DRIVER( arkbloc3 )	/* bootleg */
	DRIVER( arkgcbl )	/* bootleg */
	DRIVER( arkgcbla )	/* bootleg */
	DRIVER( paddle2 )	/* bootleg */
	DRIVER( block2 )	/* bootleg */
	DRIVER( arkatayt )	/* bootleg */
	DRIVER( arktayt2 )	/* bootleg */
	DRIVER( arkatour )	/* ??? (c) 1987 Taito America Corporation + Romstar license (US) */
	DRIVER( tetrsark )	/* ??? (c) D.R.Korea */
	DRIVER( hexa )		/* D. R. Korea */
	DRIVER( sqix )		/* B03 1987 */
	DRIVER( sqixr1 )	/* B03 1987 */
	DRIVER( sqixu )		/* B03 1987 */
	DRIVER( sqixb1 )	/* bootleg? but (c) 1987 */
	DRIVER( sqixb2 )	/* bootleg? but (c) 1987 */
	DRIVER( perestro )	/* (c) 1994 Promat */
	DRIVER( perestrof )	/* (c) 1993 Promat / Fuuki */
	DRIVER( pbillian )	/* (c) 1986 Taito */
	DRIVER( hotsmash )	/* B18 (c) 1987 Taito */
	DRIVER( exzisus )	/* B12 (c) 1987 Taito Corporation (Japan) */
	DRIVER( exzisusa )	/* B23 (c) 1987 Taito Corporation (Japan) */
	DRIVER( minivadr )	/* D26 cabinet test board */
	DRIVER( volfied )	/* C04 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( volfiedu )	/* C04 (c) 1989 Taito America Corporation (US) */
	DRIVER( volfiedj )	/* C04 (c) 1989 Taito Corporation (Japan) */
	DRIVER( volfiedjo )	/* C04 (c) 1989 Taito Corporation (Japan) */
	DRIVER( bonzeadv )	/* B41 (c) 1988 Taito Corporation Japan (World) */
	DRIVER( bonzeadvo )	/* B41 (c) 1988 Taito Corporation Japan (World) */
	DRIVER( bonzeadvu )	/* B41 (c) 1988 Taito America Corporation (US) */
	DRIVER( jigkmgri )	/* B41 (c) 1988 Taito Corporation (Japan)*/
	DRIVER( asuka )		/* B68 (c) 1988 Taito Corporation (World) */
	DRIVER( asukaj )	/* B68 (c) 1988 Taito Corporation (Japan) */
	DRIVER( mofflott )	/* C17 (c) 1989 Taito Corporation (Japan) */
	DRIVER( cadash )	/* C21 (c) 1989 Taito Corporation Japan */
	DRIVER( cadashj )	/* C21 (c) 1989 Taito Corporation */
	DRIVER( cadashu )	/* C21 (c) 1989 Taito America Corporation */
	DRIVER( cadashi )	/* C21 (c) 1989 Taito Corporation Japan */
	DRIVER( cadashf )	/* C21 (c) 1989 Taito Corporation Japan */
	DRIVER( cadashg )	/* C21 (c) 1989 Taito Corporation Japan */
	DRIVER( parentj )	/* C42 (c) 199? Taito */
	DRIVER( galmedes )	/* (c) 1992 Visco (Japan) */
	DRIVER( earthjkr )	/* (c) 1993 Visco (Japan) */
	DRIVER( eto )		/* (c) 1994 Visco (Japan) */
	DRIVER( wgp )		/* C32 (c) 1989 Taito America Corporation (US) */
	DRIVER( wgpj )		/* C32 (c) 1989 Taito Corporation (Japan) */
	DRIVER( wgpjoy )	/* C32 (c) 1989 Taito Corporation (Japan) */
	DRIVER( wgpjoya )	/* C32 (c) 1989 Taito Corporation (Japan) */
	DRIVER( wgp2 )		/* C73 (c) 1990 Taito Corporation (Japan) */
	DRIVER( galastrm )	/* C99 (c) 1992 Taito Corporation */
	DRIVER( slapshot )	/* D71 (c) 1994 Taito Corporation (Japan) */
	DRIVER( opwolf3 )	/* D74 (c) 1994 Taito */
	DRIVER( opwolf3u )	/* D74 (c) 1994 Taito */
	DRIVER( scessjoe )	/* ??? (c) 1990 Wave / Taito */
	DRIVER( ashnojoe )	/* ??? (c) 1990 Wave / Taito */

	/* Taito multi-screen games */
	DRIVER( ninjaw )	/* B31 (c) 1987 Taito Corporation Japan (World) */
	DRIVER( ninjawj )	/* B31 (c) 1987 Taito Corporation (Japan) */
	DRIVER( darius2 )	/* C07 (c) 1989 Taito Corporation (Japan) */
	DRIVER( darius2d )	/* C07 (c) 1989 Taito Corporation (Japan) */
	DRIVER( darius2do )	/* C07 (c) 1989 Taito Corporation (Japan) */
	DRIVER( warriorb )	/* D24 (c) 1991 Taito Corporation (Japan) */

	/* Taito "X"-system games */
	DRIVER( superman )	/* B61 (c) 1988 Taito Corporation */
	DRIVER( supermanj )	/* B61 (c) 1988 Taito Corporation */
	DRIVER( twinhawk )	/* B87 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( twinhawku )	/* B87 (c) 1989 Taito America Corporation (US) */
	DRIVER( daisenpu )	/* B87 (c) 1989 Taito Corporation (Japan) */
	DRIVER( gigandes )	/* (c) 1989 East Technology */
	DRIVER( gigandesj )	/* (c) 1989 East Technology */
	DRIVER( kyustrkr )	/* (c) 1989 East Technology */
	DRIVER( ballbros )	/* no copyright notice */

	/* Taito "tnzs" (Seta) hardware */
	DRIVER( plumppop )	/* A98 (c) 1987 Taito Corporation (Japan) */
	DRIVER( jpopnics )  /* (c)1992 NICs, based on Plump Pop code */
	DRIVER( extrmatn )	/* B06 (c) 1987 Taito Corporation Japan (World) */
	DRIVER( extrmatnu )	/* B06 (c) 1987 World Games */
	DRIVER( extrmatnj )	/* B06 (c) 1987 Taito Corporation (Japan)*/
	DRIVER( arknoid2 )	/* B08 (c) 1987 Taito Corporation Japan (World) */
	DRIVER( arknoid2u )	/* B08 (c) 1987 Taito America Corporation + Romstar license (US) */
	DRIVER( arknoid2j )	/* B08 (c) 1987 Taito Corporation (Japan) */
	DRIVER( drtoppel )	/* B19 (c) 1987 Taito Corporation Japan (World) */
	DRIVER( drtoppelu )	/* B19 (c) 1987 Taito Corporation (US) */
	DRIVER( drtoppelj )	/* B19 (c) 1987 Taito Corporation (Japan) */
	DRIVER( kageki )	/* B35 (c) 1988 Taito America Corporation + Romstar license (US) */
	DRIVER( kagekij )	/* B35 (c) 1988 Taito Corporation (Japan) */
	DRIVER( kagekih )	/* B35 (c) 1992 (hack) */
	DRIVER( chukatai )	/* B44 (c) 1988 Taito Corporation Japan (World) */
	DRIVER( chukataiu )	/* B44 (c) 1988 Taito Corporation (US) */
	DRIVER( chukataij )	/* B44 (c) 1988 Taito Corporation (Japan) */
	DRIVER( kabukiz )	/* B50 (c) 1988 Taito Corporation Japan (World) */
	DRIVER( kabukizj )	/* B50 (c) 1988 Taito Corporation (Japan) */
	DRIVER( tnzs )		/* B53 (c) 1988 Taito Corporation Japan (World) (new logo) */
	DRIVER( tnzsj )		/* B53 (c) 1988 Taito Corporation (Japan) (new logo) */
	DRIVER( tnzsjo )	/* B53 (c) 1988 Taito Corporation (Japan) (new logo) */
	DRIVER( tnzso )		/* B53 (c) 1988 Taito Corporation Japan (World) (old logo) */
	DRIVER( tnzsop )	/* B53?(c) 1988 Taito Corporation Japan (World) (old logo) */
	DRIVER( insectx )	/* B97 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( insectxj )	/* B97 (c) 1989 Taito Corporation (Japan) */
	DRIVER( cchance )	/* (c)1987 Taito? */
	DRIVER( champbwl )	/* (c)1989 Romstar.  not Taito, but the same Seta video chips */

	/* Taito L-System games */
	DRIVER( raimais )	/* B36 (c) 1988 Taito Corporation (Japan) */
	DRIVER( raimaisj )	/* B36 (c) 1988 Taito Corporation */
	DRIVER( raimaisjo )	/* B36 (c) 1988 Taito Corporation */
	DRIVER( kurikint )	/* B42 (c) 1988 Taito Corporation Japan (World) */
	DRIVER( kurikintu )	/* B42 (c) 1988 Taito America Corporation (US) */
	DRIVER( kurikintj )	/* B42 (c) 1988 Taito Corporation (Japan) */
	DRIVER( kurikinta )	/* B42 (c) 1988 Taito Corporation Japan (World) */
	DRIVER( evilston )	/* C67 (c) 1990 Spacy Industrial, Ltd */
	DRIVER( fhawk )		/* B70 (c) 1988 Taito Corporation Japan (World) */
	DRIVER( fhawkj )	/* B70 (c) 1988 Taito Corporation (Japan) */
	DRIVER( plotting )	/* B96 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( plottingu )	/* B96 (c) 1989 Taito Corporation Japan (US) */
	DRIVER( plottinga )	/* B96 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( plottingb )	/* B96 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( flipull )	/* B96 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( champwr )	/* C01 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( champwru )	/* C01 (c) 1989 Taito America Corporation (US) */
	DRIVER( champwrj )	/* C01 (c) 1989 Taito Corporation (Japan) */
	DRIVER( puzznic )	/* C20 (c) 1989 Taito Corporation (Japan) */
	DRIVER( puzznicj )	/* C20 (c) 1989 Taito Corporation (Japan) */
	DRIVER( puzznici )	/* C20 (c) 1989 Taito Corporation (Japan) */
	DRIVER( horshoes )	/* C47 (c) 1990 Taito America Corporation (US) */
	DRIVER( palamed )	/* C63 (c) 1990 Taito Corporation (Japan) */
	DRIVER( cachat )	/* ??? (c) 1993 Taito Corporation (Japan) */
	DRIVER( tubeit )	/* ??? no copyright message */
	DRIVER( cubybop )	/* ??? no copyright message */
	DRIVER( plgirls )	/* (c) 1992 Hot-B. */
	DRIVER( plgirls2 )	/* (c) 1993 Hot-B. */
	DRIVER( lagirl )	/* plgirls bootleg? */

	/* Taito H-System games */
	DRIVER( syvalion )	/* B51 (c) 1988 Taito Corporation (Japan) */
	DRIVER( recordbr )	/* B56 (c) 1988 Taito Corporation Japan (World) */
	DRIVER( gogold )	/* B56 (c) 1988 Taito Corporation (Japan) */
	DRIVER( dleague )	/* C02 (c) 1990 Taito Corporation (Japan) */

	/* Taito B-System games */
	DRIVER( masterw )	/* B72 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( masterwu )	/* B72 (c) 1989 Taito America Corporation (US) */
	DRIVER( masterwj )	/* B72 (c) 1989 Taito Corporation (Japan) */
	DRIVER( nastar )	/* B81 (c) 1988 Taito Corporation Japan (World) */
	DRIVER( nastarw )	/* B81 (c) 1988 Taito America Corporation (US) */
	DRIVER( rastsag2 )	/* B81 (c) 1988 Taito Corporation (Japan) */
	DRIVER( rambo3 )	/* B93 (c) 1989 Taito Europe Corporation (Europe) */
	DRIVER( rambo3ae )	/* B93 (c) 1989 Taito Europe Corporation (Europe) */
	DRIVER( rambo3a )	/* B93 (c) 1989 Taito America Corporation (US) */
	DRIVER( crimec )	/* B99 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( crimecu )	/* B99 (c) 1989 Taito America Corporation (US) */
	DRIVER( crimecj )	/* B99 (c) 1989 Taito Corporation (Japan) */
	DRIVER( tetrist )	/* C12 (c) 1989 Sega Enterprises,Ltd. (Japan) */
	DRIVER( tetrista )	/* C35 (c) 1989 Sega Enterprises,Ltd. (Japan) */
	DRIVER( viofight )	/* C16 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( viofightu )	/* C16 (c) 1989 Taito America Corporation (US) */
	DRIVER( viofightj )	/* C16 (c) 1989 Taito Corporation (Japan) */
	DRIVER( ashura )	/* C43 (c) 1990 Taito Corporation Japan (World) */
	DRIVER( ashuraj )	/* C43 (c) 1990 Taito Corporation (Japan) */
	DRIVER( ashurau )	/* C43 (c) 1990 Taito America Corporation (US) */
	DRIVER( hitice )	/* C59 (c) 1990 Williams (US) */
	DRIVER( hiticej )	/* C59 (c) 1990 Midway/Taito Corporation (Japan) */
	DRIVER( sbm )		/* C69 (c) 1990 Taito Corporation (Japan) */
	DRIVER( selfeena )	/* ??? (c) 1991 East Technology */
	DRIVER( silentd )	/* ??? (c) 1992 Taito Corporation Japan (World) */
	DRIVER( silentdu )	/* ??? (c) 1992 Taito Corporation Japan (World) */
	DRIVER( silentdj )	/* ??? (c) 1992 Taito Corporation (Japan) */
	DRIVER( ryujin )	/* ??? (c) 1993 Taito Corporation (Japan) */
	DRIVER( qzshowby )	/* D72 (c) 1993 Taito Corporation (Japan) */
	DRIVER( pbobble )	/* ??? (c) 1994 Taito Corporation (Japan) */
	DRIVER( spacedx )	/* D89 (c) 1994 Taito Corporation (US) */
	DRIVER( spacedxj )	/* D89 (c) 1994 Taito Corporation (Japan) */
	DRIVER( spacedxo )	/* D89 (c) 1994 Taito Corporation (Japan) */

	/* Taito Z-System games */
	DRIVER( contcirc )	/* B33 (c) 1987 Taito Corporation Japan (World) */
	DRIVER( contcircu )	/* B33 (c) 1987 Taito America Corporation (US) */
	DRIVER( contcircua )	/* B33 (c) 1987 Taito America Corporation (US) */
	DRIVER( chasehq )	/* B52 (c) 1988 Taito Corporation Japan (World) */
	DRIVER( chasehqj )	/* B52 (c) 1988 Taito Corporation (Japan) */
	DRIVER( chasehqu )	/* B52 (c) 1988 Taito America Corporation (US) */
	DRIVER( enforce )	/* B58 (c) 1988 Taito Corporation (Japan) */
	DRIVER( nightstr )	/* B91 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( nightstrj )	/* B91 (c) 1989 Taito Corporation (Japan) */
	DRIVER( nightstru )	/* B91 (c) 1989 Taito America Corporation (US) */
	DRIVER( sci )		/* C09 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( scia )		/* C09 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( scij )		/* C09 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( sciu )		/* C09 (c) 1989 Taito America Corporation (US) */
	DRIVER( scin )		/* C09 (c) 1991 Taito Corporation Japan (Negro Torino hack) */
	DRIVER( bshark )	/* C34 (c) 1989 Taito America Corporation (US) */
	DRIVER( bsharkj )	/* C34 (c) 1989 Taito Corporation (Japan) */
	DRIVER( aquajack )	/* B77 (c) 1990 Taito Corporation Japan (World) */
	DRIVER( aquajackj )	/* B77 (c) 1990 Taito Corporation (Japan) */
	DRIVER( spacegun )	/* C57 (c) 1990 Taito Corporation Japan (World) */
	DRIVER( dblaxle )	/* C78 (c) 1991 Taito America Corporation (US) */
	DRIVER( pwheelsj )	/* C78 (c) 1991 Taito Corporation (Japan) */
	DRIVER( racingb )	/* C84 (c) 1991 Taito Coropration .. */

	/* Taito Air System games */
	DRIVER( topland )	/* B62 (c) 1988 Taito Coporation Japan (World) */
	DRIVER( ainferno )	/* C45 (c) 1990 Taito America Corporation (US) */

	DRIVER( mlanding )	/* ??? (c) 1990 Taito America Corporation (US) */

	/* enhanced Z-System hardware games */
	DRIVER( gunbustr )	/* D27 (c) 1992 Taito Corporation (Japan) */
	DRIVER( superchs )	/* D46 (c) 1992 Taito America Corporation (US) */
	DRIVER( groundfx )	/* D51 (c) 1992 Taito Coporation */
	DRIVER( undrfire )	/* D67 (c) 1993 Taito Coporation Japan (World) */
	DRIVER( undrfireu )	/* D67 (c) 1993  Taito America Corporation (US) */
	DRIVER( undrfirej )	/* D67 (c) 1993 Taito Coporation (Japan) */
	DRIVER( cbombers )

	/* Taito F2 games */
	DRIVER( finalb )	/* 1989.?? B82 (c) 1988 Taito Corporation Japan (World) */
	DRIVER( finalbj )	/* 1989.04 B82 (c) 1988 Taito Corporation (Japan) */
	DRIVER( finalbu )	/* 1989.06 B82 (c) 1988 Taito America Corporation (US) */
	DRIVER( dondokod )	/* 1989.?? B95 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( dondokodu )	/* 1989.?? B95 (c) 1989 Taito America Corporation (US) */
	DRIVER( dondokodj )	/* 1989.07 B95 (c) 1989 Taito Corporation (Japan) */
	DRIVER( megablst )	/* 1989.?? C11 (c) 1989 Taito Corporation Japan (World) */
	DRIVER( megablstu )	/* 1989.?? C11 (c) 1989 Taito America Corporation (US) */
	DRIVER( megablstj )	/* 1989.11 C11 (c) 1989 Taito Corporation (Japan) */
	DRIVER( thundfox )	/* 1990.?? C28 (c) 1990 Taito Corporation Japan (World) */
	DRIVER( thundfoxu )	/* 1990.06 C28 (c) 1990 Taito America Corporation (US) */
	DRIVER( thundfoxj )	/* 1990.07 C28 (c) 1990 Taito Corporation (Japan) */
	DRIVER( cameltry )	/* 1990.?? C38 (c) 1989 Taito America Corporation (US) */
	DRIVER( cameltrya )	/* 1990.?? C38 (c) 1989 Taito America Corporation (US) */
	DRIVER( cameltryau )	/* 1990.?? C38 (c) 1989 Taito America Corporation (US) */
	DRIVER( cameltryj )	/* 1990.04 C38 (c) 1989 Taito Corporation (Japan) */
	DRIVER( qtorimon )	/* 1990.02 C41 (c) 1990 Taito Corporation (Japan) */
	DRIVER( liquidk )	/* 1990.?? C49 (c) 1990 Taito Corporation Japan (World) */
	DRIVER( liquidku )	/* 1990.?? C49 (c) 1990 Taito America Corporation (US) */
	DRIVER( mizubaku )	/* 1990.08 C49 (c) 1990 Taito Corporation (Japan) */
	DRIVER( quizhq )	/* 1990.07 C53 (c) 1990 Taito Corporation (Japan) */
	DRIVER( ssi )		/* 1990.?? C64 (c) 1990 Taito Corporation Japan (World) */
	DRIVER( majest12 )	/* 1990.11 C64 (c) 1990 Taito Corporation (Japan) */
						/* 1990.12 C64 (US) */
	DRIVER( gunfront )	/* 1991.?? C71 (c) 1990 Taito Corporation Japan (World) */
	DRIVER( gunfrontj )	/* 1991.01 C71 (c) 1990 Taito Corporation (Japan) */
	DRIVER( growl )		/* 1991.?? C74 (c) 1990 Taito Corporation Japan (World) */
	DRIVER( growlu )	/* 1991.02 C74 (c) 1990 Taito America Corporation (US) */
	DRIVER( runark )	/* 1991.02 C74 (c) 1990 Taito Corporation (Japan) */
	DRIVER( mjnquest )	/* 1991.05 C77 (c) 1990 Taito Corporation (Japan) */
	DRIVER( mjnquestb )	/* 1991.05 C77 (c) 1990 Taito Corporation (Japan) */
	DRIVER( footchmp )	/* 1991.?? C80 (c) 1990 Taito Corporation Japan (World) */
	DRIVER( hthero )	/* 1991.03 C80 (c) 1990 Taito Corporation (Japan) */
	DRIVER( euroch92 )	/* 1991.??     (c) 1992 Taito Corporation Japan (World) */
	DRIVER( koshien )	/* 1991.04 C81 (c) 1990 Taito Corporation (Japan) */
	DRIVER( yuyugogo )	/* 1991.03 C83 (c) 1990 Taito Corporation (Japan) */
	DRIVER( ninjak )	/* 1991.?? C85 (c) 1990 Taito Corporation Japan (World) */
	DRIVER( ninjakj )	/* 1991.04 C85 (c) 1990 Taito Corporation (Japan) */
	DRIVER( ninjaku )	/* 1991.03 C85 (c) 1990 Taito Corporation (Japan) */
	DRIVER( solfigtr )	/* 1991.?? C91 (c) 1991 Taito Corporation Japan (World) */
	DRIVER( qzquest )	/* 1991.07 C92 (c) 1991 Taito Corporation (Japan) */
	DRIVER( pulirula )	/* 1991.?? C98 (c) 1991 Taito Corporation Japan (World) */
	DRIVER( pulirulaj )	/* 1991.11 C98 (c) 1991 Taito Corporation (Japan) */
	DRIVER( metalb )	/* 1991.?? D16? (c) 1991 Taito Corporation Japan (World) */
	DRIVER( metalbj )	/* 1991.11 D12 (c) 1991 Taito Corporation (Japan) */
	DRIVER( qzchikyu )	/* 1992.02 D19 (c) 1991 Taito Corporation (Japan) */
	DRIVER( yesnoj )	/* 1992.03 D20 (c) 1992 Taito Corporation (Japan) */
	DRIVER( deadconx )	/* 1992.?? D28 (c) 1992 Taito Corporation Japan (World) */
	DRIVER( deadconxj )	/* 1992.08 D28 (c) 1992 Taito Corporation (Japan) */
	DRIVER( dinorex )	/* 1992.?? D39 (c) 1992 Taito Corporation Japan (World) */
	DRIVER( dinorexj )	/* 1992.11 D39 (c) 1992 Taito Corporation (Japan) */
	DRIVER( dinorexu )	/* 1992.?? D39 (c) 1992 Taito America Corporation (US) */
	DRIVER( qjinsei )	/* 1993.03 D48 (c) 1992 Taito Corporation (Japan) */
	DRIVER( qcrayon )	/* 1993.08 D55 (c) 1993 Taito Corporation (Japan) */
	DRIVER( qcrayon2 )	/* 1994.01 D63 (c) 1993 Taito Corporation (Japan) */
	DRIVER( driftout )	/* 1991.10 (c) 1991 Visco */
	DRIVER( driveout )	/* bootleg */

	/* Taito F3 games */
	DRIVER( ringrage )	/* 1992.?? D21 (c) 1992 Taito Corporation Japan (World) */
	DRIVER( ringragej )	/* 1992.09 D21 (c) 1992 Taito Corporation (Japan) */
	DRIVER( ringrageu )	/* 1992.02 D21 (c) 1992 Taito America Corporation (US) */
	DRIVER( arabianm )	/* 1992.?? D29 (c) 1992 Taito Corporation Japan (World) */
	DRIVER( arabianmj )	/* 1992.09 D29 (c) 1992 Taito Corporation (Japan) */
	DRIVER( arabianmu )	/* 1992.10 D29 (c) 1992 Taito America Corporation (US) */
	DRIVER( ridingf )	/* 1992.?? D34 (c) 1992 Taito Corporation Japan (World) */
	DRIVER( ridingfj )	/* 1992.12 D34 (c) 1992 Taito Corporation (Japan) */
	DRIVER( ridingfu )	/* 1992.?? D34 (c) 1992 Taito America Corporation (US) */
	DRIVER( gseeker )	/* 1992.?? D40 (c) 1992 Taito Corporation Japan (World) */
	DRIVER( gseekerj )	/* 1992.12 D40 (c) 1992 Taito Corporation (Japan) */
	DRIVER( gseekeru )	/* 1992.?? D40 (c) 1992 Taito America Corporation (US) */
	DRIVER( hthero93 )	/* 1993.03 D49 (c) 1992 Taito Corporation (Japan) */
						/* 1993.04 D49 (US) */
	DRIVER( cupfinal )	/* 1993.?? D49 (c) 1993 Taito Corporation Japan (World) */
	DRIVER( trstar )	/* 1993.?? D53 (c) 1993 Taito Corporation Japan (World) */
	DRIVER( trstarj )	/* 1993.07 D53 (c) 1993 Taito Corporation (Japan) */
	DRIVER( prmtmfgt )	/* 1993.08 D53 (c) 1993 Taito Corporation (US) */
	DRIVER( prmtmfgto )	/* 1993.08 D53 (c) 1993 Taito Corporation (US) */
	DRIVER( trstaro )	/* 1993.?? D53 (c) 1993 Taito Corporation (World) */
	DRIVER( trstaroj )	/* 1993.07 D53 (c) 1993 Taito Corporation (Japan) */
	DRIVER( gunlock )	/* 1994.?? D66 (c) 1993 Taito Corporation Japan (World) */
	DRIVER( rayforcej )	/* 1994.02 D66 (c) 1993 Taito Corporation (Japan) */
	DRIVER( rayforce )	/* 1994.?? D66 (c) 1993 Taito America Corporation (US) */
	DRIVER( scfinals )	/* 1994.?? D68 (c) 1993 Taito Corporation Japan (World) */
	DRIVER( intcup94 )	/* 1994.?? D78 (c) 1994 Taito (World) */
	DRIVER( hthero94 )	/* 1994.09 D78 (c) 1994 Taito (US) */
	DRIVER( lightbr )	/* 1994.03 D69 (c) 1993 Taito Corporation Japan (World) */
	DRIVER( lightbrj )	/* 1994.03 D69 (c) 1993 Taito Corporation (Japan) */
	DRIVER( dungeonm )	/* 1994.?? D69 (c) 1993 Taito Corporation Japan (World) */
	DRIVER( dungeonmu )	/* 1994.?? D69 (c) 1993 Taito America Corporation (US) */
	DRIVER( kaiserkn )	/* 1994.?? D84 (c) 1994 Taito Corporation Japan (World) */
	DRIVER( kaiserknj )	/* 1994.08 D84 (c) 1994 Taito Corporation (Japan) */
	DRIVER( gblchmp )	/* 1994.10 D84 (c) 1994 Taito America Corporation (US) */
	DRIVER( dankuga )	/* 1994.?? D84? (c) 1994 Taito Corporation (Japan) */
	DRIVER( dariusg )	/* 1994.?? D87 (c) 1994 Taito Corporation Japan (World) */
	DRIVER( dariusgj )	/* 1994.09 D87 (c) 1994 Taito Corporation (Japan) */
	DRIVER( dariusgu )	/* 1994.11 D87 (c) 1994 Taito America Corporation (US) */
	DRIVER( dariusgx )	/* 1994.?? D87 (c) 1994 Taito Corporation */
	DRIVER( bublbob2 )	/* 1994.?? D90 (c) 1994 Taito Corporation Japan (World) */
	DRIVER( bubsymphe )	/* 1994.?? D90 (c) 1994 Taito Corporation Japan (Europe) */
	DRIVER( bubsymphu )	/* 1994.10 D90 (c) 1994 Taito America Corporation (US) */
	DRIVER( bubsymphj )	/* 1994.10 D90 (c) 1994 Taito Corporation (Japan) */
	DRIVER( bubsymphb )	/* bootleg */
	DRIVER( spcinvdj )	/* 1994.09 D93 (c) 1994 Taito Corporation (Japan) */
	DRIVER( pwrgoal )	/* 1994.?? D94 (c) 1995 Taito Corporation Japan (World) */
	DRIVER( hthero95 )	/* 1994.11 D94 (c) 1995 Taito Corporation (Japan) */
	DRIVER( hthero95u )	/* 1995.05 D94 (c) 1995 Taito America Corporation (US) */
	DRIVER( qtheater )	/* 1995.01 D95 (c) 1994 Taito Corporation (Japan) */
	DRIVER( elvactr )	/* 1995.?? E02 (c) 1994 Taito Corporation Japan (World) */
	DRIVER( elvactrj )	/* 1995.03 E02 (c) 1994 Taito Corporation (Japan) */
	DRIVER( elvact2u )	/* 1995.05 E02 (c) 1994 Taito America Corporation (US) */
	DRIVER( spcinv95 )	/* 1995.?? E06 (c) 1995 Taito Corporation Japan (World) */
	DRIVER( spcinv95u )	/* 1995.05 E06 (c) 1995 Taito America Corporation (US) */
	DRIVER( akkanvdr )	/* 1995.07 E06 (c) 1995 Taito Corporation (Japan) */
	DRIVER( twinqix )	/* 1995.03 ??? (c) 1995 Taito America Corporation (US) */
	DRIVER( quizhuhu )	/* 1995.07 E08 (c) 1995 Taito Corporation (Japan) */
	DRIVER( pbobble2 )	/* 1995.?? E10 (c) 1995 Taito Corporation Japan (World) */
	DRIVER( pbobble2o )	/* 1995.?? E10 (c) 1995 Taito Corporation Japan (World) */
	DRIVER( pbobble2j )	/* 1995.09 E10 (c) 1995 Taito Corporation (Japan) */
	DRIVER( pbobble2u )	/* 1995.11 E10 (c) 1995 Taito America Corporation (US) */
	DRIVER( pbobble2x )	/* 1995.12 E10 (c) 1995 Taito Corporation (Japan) */
	DRIVER( gekirido )	/* 1995.11 E11 (c) 1995 Taito Corporation (Japan) */
	DRIVER( tcobra2 )	/* 1996.?? E15 (c) 1995 Taito Corporation (World) */
	DRIVER( tcobra2u )	/* 1996.?? E15 (c) 1995 Taito Corporation (US) */
	DRIVER( ktiger2 )	/* 1996.02 E15 (c) 1995 Taito Corporation (Japan) */
	DRIVER( bubblem )	/* 1996.?? E21 (c) 1995 Taito Corporation Japan (World) */
	DRIVER( bubblemj )	/* 1996.04 E21 (c) 1995 Taito Corporation (Japan) */
	DRIVER( cleopatr )	/* 1996.10 E28 (c) 1996 Taito Corporation (Japan) */
	DRIVER( pbobble3 )	/* 1996.?? E29 (c) 1996 Taito Corporation (World) */
	DRIVER( pbobble3u )	/* 1996.11 E29 (c) 1996 Taito Corporation (US) */
	DRIVER( pbobble3j )	/* 1996.11 E29 (c) 1996 Taito Corporation (Japan) */
	DRIVER( arkretrn )	/* 1997.03 E36 (c) 1997 Taito Corporation (Japan) */
	DRIVER( kirameki )	/* 1997.09 E44 (c) 1997 Taito Corporation (Japan) */
	DRIVER( puchicar )	/* 1997.?? E46 (c) 1997 Taito Corporation (World) */
	DRIVER( puchicarj )	/* 1997.12 E46 (c) 1997 Taito Corporation (Japan) */
	DRIVER( pbobble4 )	/* 1998.?? E49 (c) 1997 Taito Corporation (World) */
	DRIVER( pbobble4j )	/* 1998.02 E49 (c) 1997 Taito Corporation (Japan) */
	DRIVER( pbobble4u )	/* 1998.?? E49 (c) 1997 Taito Corporation (US) */
	DRIVER( popnpop )	/* 1998.?? E51 (c) 1997 Taito Corporation (World) */
	DRIVER( popnpopj )	/* 1998.03 E51 (c) 1997 Taito Corporation (Japan) */
	DRIVER( popnpopu )	/* 1998.?? E51 (c) 1997 Taito Corporation (US) */
	DRIVER( landmakr )	/* 1998.08 E61 (c) 1998 Taito Corporation (Japan) */
	DRIVER( landmakrp )	/* 1998.?? E61 (c) 1998 Taito Corporation (World, prototype) */
	DRIVER( recalh )	/* prototype */
	DRIVER( commandw )	/* prototype */

	/* Taito JC System */
	DRIVER( dangcurv )	/* 1995.?? E09 (c) 1995 Taito Corporation */
						/* 1995.07 (Japan) */
						/* 1995.10 (US) */
	DRIVER( landgear )	/* 1996.?? E17 (c) 1995 Taito Corporation Japan (World) */
						/* 1996.03 (Japan) */
	DRIVER( sidebs )	/* 1996.07 E23 (c) 1996 Taito Corporation (Japan) */
	DRIVER( dendeg )	/* 1997.03 E35 (c) 1996 Taito Corporation (Japan) */
	DRIVER( sidebs2j )	/* 1997.07 E38 (c) 1997 Taito Corporation (Japan) */
	DRIVER( sidebs2 )	/* 1997.?? E38 (c) 1997 Taito Corporation (Japan) */
	DRIVER( dendegx )	/* 1997.09 E35 (c) 1996 Taito Corporation (Japan) */
	DRIVER( dendeg2 )	/* 1998.03 E52 (c) 1998 Taito Corporation (Japan) */
	DRIVER( dendeg2x )	/* 1998.08 E52 (c) 1998 Taito Corporation (Japan) */

	/* Taito "Wolf" System */
	DRIVER( pf2012 )	/* E59 (c) 1997 Taito */

	/* Taito PPC JC System */
	DRIVER( optiger )	/* 1998.09 E63 (c) 1998 Taito */

	/* Taito Type-Zero System */
	DRIVER( taitotz )
	DRIVER( batlgear )	/* E68 (c) 1999 Taito */
	DRIVER( landhigh )	/* E82 (c) 1999 Taito */
	DRIVER( batlgr2 )	/* E87 (c) 2000 Taito */

	/* Toaplan games */
	DRIVER( perfrman )	/* (c) 1985 Data East Corporation (Japan) */
	DRIVER( perfrmanu )	/* (c) 1985 Data East USA (US) */
	DRIVER( tigerh )	/* A47 (c) 1985 Taito America  Corporation   GX-551 [not a Konami board!] */
	DRIVER( tigerhj )	/* A47 (c) 1985 Taito Corporation   GX-551 [not a Konami board!] */
	DRIVER( tigerhb1 )	/* bootleg but (c) 1985 Taito Corporation */
	DRIVER( tigerhb2 )	/* bootleg but (c) 1985 Taito Corporation */
	DRIVER( tigerhb3 )	/* bootleg but (c) 1985 Taito Corporation */
	DRIVER( alcon )		/* A77 / TP-??? */
	DRIVER( slapfigh )	/* A77 / TP-??? */
	DRIVER( slapfigha )	/* A76 / TP-??? */
	DRIVER( slapfighb1 )	/* bootleg but (c) 1986 Taito Corporation */
	DRIVER( slapfighb2 )	/* bootleg but (c) 1986 Taito Corporation */
	DRIVER( slapfighb3 )	/* bootleg but (c) 1986 Taito Corporation */
	DRIVER( getstar )	/* A68 (c) 1986 Taito Corporation */
	DRIVER( getstarj )	/* A68 (c) 1986 Taito Corporation */
	DRIVER( gtstarb1 )	/* GX-006 bootleg but (c) 1986 Taito Corporation */
	DRIVER( gtstarb2 )	/* GX-006 bootleg but (c) 1986 Taito Corporation */
	DRIVER( mjsister )	/* (c) 1986 Toaplan */

	DRIVER( fshark )	/* B02 / TP-007 (c) 1987 Taito Corporation (World) */
	DRIVER( skyshark )	/* B02 / TP-007 (c) 1987 Taito America Corporation + Romstar license (US) */
	DRIVER( hishouza )	/* B02 / TP-007 (c) 1987 Taito Corporation (Japan) */
	DRIVER( fsharkbt )	/* bootleg */
	DRIVER( wardner )	/* B25 / TP-009 (c) 1987 Taito Corporation Japan (World) */
	DRIVER( pyros )		/* B25 / TP-009 (c) 1987 Taito America Corporation (US) */
	DRIVER( wardnerj )	/* B25 / TP-009 (c) 1987 Taito Corporation (Japan) */
	DRIVER( twincobr )	/* B30 / TP-011 (c) 1987 Taito Corporation (World) */
	DRIVER( twincobru )	/* B30 / TP-011 (c) 1987 Taito America Corporation + Romstar license (US) */
	DRIVER( ktiger )	/* B30 / TP-011 (c) 1987 Taito Corporation (Japan) */
	DRIVER( gulfwar2 )	/* (c) 1991 Comad */

	DRIVER( rallybik )	/* B45 / TP-O12 (c) 1988 Taito */
	DRIVER( truxton )	/* B65 / TP-O13B (c) 1988 Taito */
	DRIVER( hellfire )	/* B90 / TP-??? (c) 1989 Toaplan + Taito license */
	DRIVER( hellfire1 )	/* B90 / TP-??? (c) 1989 Toaplan + Taito license */
	DRIVER( hellfire2 )	/* B90 / TP-??? (c) 1989 Toaplan + Taito license */
	DRIVER( hellfire3 )	/* B90 / TP-??? (c) 1989 Toaplan + Taito license */
	DRIVER( zerowing )	/* TP-O15 (c) 1989 Toaplan */
	DRIVER( zerowing2 )	/* TP-O15 (c) 1989 Toaplan */
	DRIVER( demonwld )	/* TP-O16 (c) 1990 Toaplan (+ Taito license when set to Japan) */
	DRIVER( demonwld1 )	/* TP-O16 (c) 1989 Toaplan + Taito license */
	DRIVER( demonwld2 )	/* TP-O16 (c) 1989 Toaplan */
	DRIVER( demonwld3 )	/* TP-O16 (c) 1989 Toaplan */
	DRIVER( demonwld4 )	/* TP-O16 (c) 1989 Toaplan */
	DRIVER( fireshrk )	/* TP-O17 (c) 1990 Toaplan */
	DRIVER( fireshrkd )	/* TP-O17 (c) 1990 Toaplan */
	DRIVER( fireshrkdh )	/* TP-O17 (c) 1990 Toaplan */
	DRIVER( samesame )	/* TP-O17 (c) 1989 Toaplan */
	DRIVER( samesame2 )	/* TP-O17 (c) 1989 Toaplan */
	DRIVER( outzone )	/* TP-O18 (c) 1990 Toaplan */
	DRIVER( outzonea )	/* TP-O18 (c) 1990 Toaplan */
	DRIVER( outzoneb )	/* TP-O18 (c) 1990 Toaplan */
	DRIVER( outzonec )	/* TP-O18 (c) 1990 Toaplan */
	DRIVER( outzoned )	/* TP-O18 (c) 1990 Toaplan */
	DRIVER( vimana )	/* TP-O19 (c) 1991 Toaplan (+ Tecmo license when set to Japan) */
	DRIVER( vimana1 )	/* TP-O19 (c) 1991 Toaplan (+ Tecmo license when set to Japan)  */
	DRIVER( vimanan )	/* TP-O19 (c) 1991 Toaplan (+ Nova Apparate GMBH & Co license) */
	DRIVER( snowbros )	/* MIN16-02 (c) 1990 Toaplan + Romstar license */
	DRIVER( snowbrosa )	/* MIN16-02 (c) 1990 Toaplan + Romstar license */
	DRIVER( snowbrosb )	/* MIN16-02 (c) 1990 Toaplan + Romstar license */
	DRIVER( snowbrosc )	/* MIN16-02 (c) 1990 Toaplan + Romstar license */
	DRIVER( snowbrosj )	/* MIN16-02 (c) 1990 Toaplan */
	DRIVER( snowbrosd )	/* MIN16-02 (c) 1990 Toaplan + Dooyong license */
	DRIVER( wintbob )	/* bootleg */
	DRIVER( honeydol )	/* (c) 1995 Barko Corp */
	DRIVER( twinadv )	/* (c) 1995 Barko Corp */
	DRIVER( twinadvk )	/* (c) 1995 Barko Corp */
		/* SemiCom games on snowbros like hardware */
	DRIVER( hyperpac )	/* (c) 1995 SemiCom */
	DRIVER( hyperpacb )	/* bootleg */
	DRIVER( toppyrap )	/* (c) 1996 SemiCom */
	DRIVER( cookbib2 )	/* (c) 1996 SemiCom */
	DRIVER( cookbib3 )	/* (c) 1997 SemiCom */
	DRIVER( twinkle )	/* (c) 1997 SemiCom */
	DRIVER( pzlbreak )	/* (c) 1997 SemiCom */
	DRIVER( 3in1semi )	/* (c) 1998 SemiCom */
	DRIVER( moremore )	/* (c) 1999 SemiCom + Exit */
	DRIVER( moremorp )	/* (c) 1999 SemiCom + Exit */
	DRIVER( 4in1boot )	/* (c) 2002 but bootleg of 1999 release? */
	DRIVER( snowbros3 )	/* (c) 2002 but hack / bootleg of snowbros? */
	DRIVER( finalttr )	/* (c) 1993 Jeil Computer System */

	/* More Toaplan Games */
	DRIVER( tekipaki )	/* TP-020 (c) 1991 Toaplan */
	DRIVER( ghox )		/* TP-021 (c) 1991 Toaplan */
	DRIVER( ghoxj )		/* TP-021 (c) 1991 Toaplan */
	DRIVER( dogyuun )	/* TP-022 (c) 1992 Toaplan */
	DRIVER( dogyuunk )	/* TP-022 (c) 1992 Toaplan */
	DRIVER( kbash )		/* TP-023 (c) 1993 Toaplan */
	DRIVER( kbash2 )	/* bootleg */
	DRIVER( truxton2 )	/* TP-024 (c) 1992 Toaplan */
	DRIVER( pipibibs )	/* TP-025 */
	DRIVER( pipibibsa )	/* TP-025 */
	DRIVER( whoopee )	/* TP-025 */
	DRIVER( pipibibi )	/* (c) 1991 Ryouta Kikaku (bootleg?) */
	DRIVER( fixeight )	/* TP-026 (c) 1992 + Taito license */
	DRIVER( fixeightb )	/* bootleg */
	DRIVER( vfive )		/* TP-027 (c) 1993 Toaplan (Japan) */
	DRIVER( grindstm )	/* TP-027 (c) 1993 Toaplan + Unite Trading license (Korea) */
	DRIVER( grindstma )	/* TP-027 (c) 1993 Toaplan + Unite Trading license (Korea) */
	DRIVER( batsugun )	/* TP-030 (c) 1993 Toaplan */
	DRIVER( batsuguna )	/* TP-030 (c) 1993 Toaplan */
	DRIVER( batsugunsp )/* TP-??? (c) 1993 Toaplan */
	DRIVER( snowbro2 )	/* TP-??? (c) 1994 Hanafram */
	DRIVER( sstriker )	/* (c) 1993 Raizing */
	DRIVER( sstrikera )	/* (c) 1993 Raizing */
	DRIVER( mahoudai )	/* (c) 1993 Raizing + Able license */
	DRIVER( shippumd )	/* (c) 1994 Raizing/8ing */
	DRIVER( kingdmgp )	/* (c) 1994 Raizing/8ing (hack?) */
	DRIVER( bgaregga )	/* (c) 1996 Raizing/8ing */
	DRIVER( bgareggahk )	/* (c) 1996 Raizing/8ing */
	DRIVER( bgareggatw )	/* (c) 1996 Raizing/8ing */
	DRIVER( bgaregganv )	/* (c) 1996 Raizing/8ing */
	DRIVER( bgareggat2 )	/* (c) 1996 Raizing/8ing */
	DRIVER( bgareggacn )	/* (c) 1996 Raizing/8ing */
	DRIVER( batrid )	/* (c) 1998 Raizing/8ing */
	DRIVER( batridu )	/* (c) 1998 Raizing/8ing */
	DRIVER( batridc )	/* (c) 1998 Raizing/8ing */
	DRIVER( batridj )	/* (c) 1998 Raizing/8ing */
	DRIVER( batridk )	/* (c) 1998 Raizing/8ing */
	DRIVER( batridja )	/* (c) 1998 Raizing/8ing */
	DRIVER( batridta )	/* (c) 1998 Raizing/8ing */
	DRIVER( bkraidu )	/* (c) 1999 8ing */
	DRIVER( bkraiduj )	/* (c) 1999 8ing */
	DRIVER( bkraidj )	/* (c) 1999 8ing */

/*
Toa Plan's board list
(translated from http://www.aianet.ne.jp/~eisetu/rom/rom_toha.html)

Title              ROMno.   Remark(1)   Remark(2)
--------------------------------------------------
Tiger Heli           A47      GX-551
Hishouzame           B02      TP-007
Kyukyoku Tiger       B30      TP-011
Dash Yarou           B45      TP-012
Tatsujin             B65      TP-013B   M6100649A
Zero Wing            O15      TP-015
Horror Story         O16      TP-016
Same!Same!Same!      O17      TP-017
Out Zone                      TP-018
Vimana                        TP-019
Teki Paki            O20      TP-020
Ghox               TP-21      TP-021
Dogyuun                       TP-022
Tatsujin Oh                   TP-024    *1
Fixeight                      TP-026
V-V                           TP-027

*1 There is a doubt this game uses TP-024 board and TP-025 romsets.

   86 Mahjong Sisters                                 Kit 2P 8W+2B     HC    Mahjong TP-
   88 Dash                                            Kit 2P 8W+2B                   TP-
   89 Fire Shark                                      Kit 2P 8W+2B     VC    Shooter TP-017
   89 Twin Hawk                                       Kit 2P 8W+2B     VC    Shooter TP-
   91 Whoopie                                         Kit 2P 8W+2B     HC    Action
   92 Teki Paki                                       Kit 2P                         TP-020
   92 Ghox                                            Kit 2P Paddle+1B VC    Action  TP-021
10/92 Dogyuun                                         Kit 2P 8W+2B     VC    Shooter TP-022
92/93 Knuckle Bash                 Atari Games        Kit 2P 8W+2B     HC    Action  TP-023
10/92 Tatsujin II/Truxton II       Taito              Kit 2P 8W+2B     VC    Shooter TP-024
10/92 Truxton II/Tatsujin II       Taito              Kit 2P 8W+2B     VC    Shooter TP-024
      Pipi & Bipi                                                                    TP-025
   92 Fix Eight                                       Kit 2P 8W+2B     VC    Action  TP-026
12/92 V  -  V (5)/Grind Stormer                       Kit 2P 8W+2B     VC    Shooter TP-027
 1/93 Grind Stormer/V - V (Five)                      Kit 2P 8W+2B     VC    Shooter TP-027
 2/94 Batsugun                                        Kit 2P 8W+2B     VC            TP-
 4/94 Snow Bros. 2                                    Kit 2P 8W+2B     HC    Action  TP-
*/

	/* Cave games */
	/* Cave was formed in 1994 from the ruins of Toaplan, like Raizing was. */
	DRIVER( pwrinst2 )	/* (c) 1994 Atlus */
	DRIVER( pwrinst2j )	/* (c) 1994 Atlus */
	DRIVER( plegends )	/* (c) 1994 Atlus */
	DRIVER( plegendsj )	/* (c) 1994 Atlus */
	DRIVER( mazinger )	/* (c) 1994 Banpresto (country is in EEPROM) */
	DRIVER( mazingerj )	/* (c) 1994 Banpresto (country is in EEPROM) */
	DRIVER( donpachi )	/* (c) 1995 Atlus/Cave */
	DRIVER( donpachij )	/* (c) 1995 Atlus/Cave */
	DRIVER( donpachikr )	/* (c) 1995 Atlus/Cave */
	DRIVER( donpachihk )	/* (c) 1995 Atlus/Cave */
	DRIVER( metmqstr )	/* (c) 1995 Banpresto / Pandorabox */
	DRIVER( nmaster )	/* (c) 1995 Banpresto / Pandorabox */
	DRIVER( sailormn )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnu )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnj )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnk )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnt )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnh )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormno )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnou )/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnoj )/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnok )/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnot )/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnoh )/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( agallet )	/* (c) 1996 Banpresto / Gazelle (country is in EEPROM) */
	DRIVER( agalletu )	/* (c) 1996 Banpresto / Gazelle (country is in EEPROM) */
	DRIVER( agalletj )	/* (c) 1996 Banpresto / Gazelle (country is in EEPROM) */
	DRIVER( agalletk )	/* (c) 1996 Banpresto / Gazelle (country is in EEPROM) */
	DRIVER( agallett )	/* (c) 1996 Banpresto / Gazelle (country is in EEPROM) */
	DRIVER( agalleth )	/* (c) 1996 Banpresto / Gazelle (country is in EEPROM) */
	DRIVER( hotdogst )	/* (c) 1996 Marble */
	DRIVER( ddonpach )	/* (c) 1997 Atlus/Cave */
	DRIVER( ddonpachj )	/* (c) 1997 Atlus/Cave */
	DRIVER( dfeveron )	/* (c) 1998 Cave + Nihon System license */
	DRIVER( feversos )	/* (c) 1998 Cave + Nihon System license */
	DRIVER( esprade )	/* (c) 1998 Atlus/Cave */
	DRIVER( espradej )	/* (c) 1998 Atlus/Cave (Japan) */
	DRIVER( espradejo )	/* (c) 1998 Atlus/Cave (Japan) */
	DRIVER( uopoko )	/* (c) 1998 Cave + Jaleco license */
	DRIVER( uopokoj )	/* (c) 1998 Cave + Jaleco license */
	DRIVER( guwange )	/* (c) 1999 Atlus/Cave */
	DRIVER( gaia )		/* (c) 1999 Noise Factory */
	DRIVER( theroes )	/* (c) 2001 Primetek Investments */
	DRIVER( korokoro )	/* (c) 1999 Takumi */
	DRIVER( crusherm )	/* (c) 1999 Takumi */
	DRIVER( tjumpman )	/* (c) 1999 Namco */

	/* Kyugo games */
	/* Kyugo only made four games: Repulse, Flash Gal, SRD Mission and Air Wolf. */
	/* Gyrodine was made by Crux. Crux was antecedent of Toa Plan, and spin-off from Orca. */
	DRIVER( gyrodine )	/* (c) 1984 Crux */
	DRIVER( gyrodinet )	/* (c) 1984 Crux (Taito Corporation license) */
	DRIVER( buzzard )	/* (c) 1984 Crux */
	DRIVER( repulse )	/* (c) 1985 Sega */
	DRIVER( 99lstwar )	/* (c) 1985 Proma */
	DRIVER( 99lstwara )	/* (c) 1985 Proma */
	DRIVER( 99lstwark )	/* (c) 1985 Kyugo */
	DRIVER( sonofphx )	/* bootleg */
	DRIVER( flashgal )	/* (c) 1985 Sega */
	DRIVER( flashgala )	/* (c) 1985 Sega */
	DRIVER( srdmissn )	/* (c) 1986 Taito Corporation */
	DRIVER( fx )		/* bootleg */
	DRIVER( legend )	/* no copyright notice [1986 Sega/Coreland] (Arcade TV Game List - P.104, Left, 14 from top) */
	DRIVER( legendb )	/* no copyright notice [1986 Sega/Coreland] (Arcade TV Game List - P.104, Left, 14 from top) */
	DRIVER( airwolf )	/* (c) 1987 Kyugo */
	DRIVER( airwolfa )	/* (c) 1987 Kyugo */
	DRIVER( skywolf )	/* bootleg */
	DRIVER( skywolf2 )	/* bootleg */
	DRIVER( skywolf3 )	/* bootleg */

	/* Williams games */
	DRIVER( defender )	/* (c) 1980 */
	DRIVER( defenderg )	/* (c) 1980 */
	DRIVER( defenderb )	/* (c) 1980 */
	DRIVER( defenderw )	/* (c) 1980 */
	DRIVER( defndjeu )	/* bootleg */
	DRIVER( tornado1 )	/* bootleg */
	DRIVER( tornado2 )	/* bootleg */
	DRIVER( zero )		/* bootleg */
	DRIVER( zero2 )		/* bootleg */
	DRIVER( defcmnd )	/* bootleg */
	DRIVER( defence )	/* bootleg */
	DRIVER( startrkd )	/* bootleg */
	DRIVER( mayday )	/* bootleg */
	DRIVER( maydaya )	/* bootleg */
	DRIVER( maydayb )	/* bootleg */
	DRIVER( colony7 )	/* (c) 1981 Taito */
	DRIVER( colony7a )	/* (c) 1981 Taito */
	DRIVER( jin )		/* Falcon bootleg/hack */
	DRIVER( stargate )	/* (c) 1981 */
	DRIVER( robotron )	/* (c) 1982 */
	DRIVER( robotronyo )/* (c) 1982 */
	DRIVER( joust )		/* (c) 1982 */
	DRIVER( joustr )	/* (c) 1982 */
	DRIVER( joustwr )	/* (c) 1982 */
	DRIVER( bubbles )	/* (c) 1982 */
	DRIVER( bubblesr )	/* (c) 1982 */
	DRIVER( bubblesp )	/* (c) 1982 */
	DRIVER( splat )		/* (c) 1982 */
	DRIVER( sinistar )	/* (c) 1982 */
	DRIVER( sinistar1 )	/* (c) 1982 */
	DRIVER( sinistar2 )	/* (c) 1982 */
	DRIVER( playball )	/* (c) 1983 */
	DRIVER( blaster )	/* (c) 1983 */
	DRIVER( blaster30 )	/* (c) 1983 */
	DRIVER( blasterkit )	/* (c) 1983 */
	DRIVER( spdball )	/* (c) 1985 */
	DRIVER( alienar )	/* (c) 1985 Duncan Brown */
	DRIVER( alienaru )	/* (c) 1985 Duncan Brown */
	DRIVER( mysticm )	/* (c) 1983 */
	DRIVER( tshoot )	/* (c) 1984 */
	DRIVER( inferno )	/* (c) 1984 */
	DRIVER( joust2 )	/* (c) 1986 */
	DRIVER( lottofun )	/* (c) 1987 H.A.R. Management */

	/* Capcom games */
	/* The following is a COMPLETE list of the Capcom games up to 1997, as shown on */
	/* their web site. The list is sorted by production date.                       */
	/* A comprehensive list of Capcom games with board info can be found here:      */
	/* http://www.arcadeflyers.com/strider/capcom_list.html                         */
	DRIVER( vulgus )	/*  5/1984 (c) 1984 */
	DRIVER( vulgus2 )	/*  5/1984 (c) 1984 */
	DRIVER( vulgusj )	/*  5/1984 (c) 1984 */
	DRIVER( sonson )	/*  7/1984 (c) 1984 */
	DRIVER( sonsonj )	/*  7/1984 (c) 1984 (Japan) */
	DRIVER( higemaru )	/*  9/1984 (c) 1984 */
	DRIVER( 1942 )		/* 12/1984 (c) 1984 */
	DRIVER( 1942a )		/* 12/1984 (c) 1984 */
	DRIVER( 1942abl )	/* bootleg */
	DRIVER( 1942b )		/* 12/1984 (c) 1984 */
	DRIVER( 1942w )		/* 12/1984 (c) 1984 + Williams Electronics license (c) 1985 */
	DRIVER( exedexes )	/*  2/1985 (c) 1985 */
	DRIVER( savgbees )	/*  2/1985 (c) 1985 + Memetron license */
	DRIVER( commando )	/*  5/1985 (c) 1985 (World) */
	DRIVER( commandou )	/*  5/1985 (c) 1985 + Data East license (US) */
	DRIVER( commandoj )	/*  5/1985 (c) 1985 (Japan) */
	DRIVER( commandob )	/* bootleg */
	DRIVER( sinvasn )	/* Europe original? */
	DRIVER( sinvasnb )	/* bootleg */
	DRIVER( gng )		/*  9/1985 (c) 1985 */
	DRIVER( gnga )		/*  9/1985 (c) 1985 */
	DRIVER( gngbl )		/* bootleg */
	DRIVER( gngblita )	/* bootleg */
	DRIVER( gngc )		/*  9/1985 (c) 1985 */
	DRIVER( gngt )		/*  9/1985 (c) 1985 */
	DRIVER( makaimur )	/*  9/1985 (c) 1985 */
	DRIVER( makaimurc )	/*  9/1985 (c) 1985 */
	DRIVER( makaimurg )	/*  9/1985 (c) 1985 */
	DRIVER( diamond )	/* (c) 1989 KH Video (NOT A CAPCOM GAME but runs on GnG hardware) */
	DRIVER( gunsmoke )	/* 11/1985 (c) 1985 (World) */
	DRIVER( gunsmokeu )	/* 11/1985 (c) 1985 + Romstar (US) */
	DRIVER( gunsmokeua )/* 11/1985 (c) 1985 (US) */
	DRIVER( gunsmokej )	/* 11/1985 (c) 1985 (Japan) */
	DRIVER( sectionz )	/* 12/1985 (c) 1985 */
	DRIVER( sectionza )	/* 12/1985 (c) 1985 */
	DRIVER( trojan )	/*  4/1986 (c) 1986 (US) */
	DRIVER( trojanr )	/*  4/1986 (c) 1986 + Romstar */
	DRIVER( trojanj )	/*  4/1986 (c) 1986 (Japan) */
	DRIVER( srumbler )	/*  9/1986 (c) 1986 */
	DRIVER( srumbler2 )	/*  9/1986 (c) 1986 */
	DRIVER( rushcrsh )	/*  9/1986 (c) 1986 */
	DRIVER( lwings )	/* 11/1986 (c) 1986 */
	DRIVER( lwings2 )	/* 11/1986 (c) 1986 */
	DRIVER( lwingsj )	/* 11/1986 (c) 1986 */
	DRIVER( lwingsb )	/* 11/1986 (c) 1986 (bootleg)*/
	DRIVER( sidearms )	/* 12/1986 (c) 1986 (World) */
	DRIVER( sidearmsr )	/* 12/1986 (c) 1986 + Romstar license (US) */
	DRIVER( sidearmsj )	/* 12/1986 (c) 1986 (Japan) */
	DRIVER( turtship )	/* (c) 1988 Philco (NOT A CAPCOM GAME but runs on modified Sidearms hardware) */
	DRIVER( turtshipj )	/* (c) 1988 Philco (NOT A CAPCOM GAME but runs on modified Sidearms hardware) */
	DRIVER( turtshipk )	/* (c) 1988 Philco (NOT A CAPCOM GAME but runs on modified Sidearms hardware) */
	DRIVER( dyger )		/* (c) 1989 Philco (NOT A CAPCOM GAME but runs on modified Sidearms hardware) */
	DRIVER( dygera )	/* (c) 1989 Philco (NOT A CAPCOM GAME but runs on modified Sidearms hardware) */
	DRIVER( twinfalc )	/* (c) 1989 Philko (Poara Enterprises license) (NOT A CAPCOM GAME but runs on modified Sidearms hardware) */
	DRIVER( whizz )		/* (c) 1989 Philco (NOT A CAPCOM GAME but runs on modified Sidearms hardware) */
	DRIVER( avengers )	/*  2/1987 (c) 1987 (US) */
	DRIVER( avengers2 )	/*  2/1987 (c) 1987 (US) */
	DRIVER( buraiken )	/*  2/1987 (c) 1987 (Japan) */
	DRIVER( bionicc )	/*  3/1987 (c) 1987 (Euro) */
	DRIVER( bionicc1 )	/*  3/1987 (c) 1987 (US) */
	DRIVER( bionicc2 )	/*  3/1987 (c) 1987 (US) */
	DRIVER( topsecrt )	/*  3/1987 (c) 1987 (Japan) */
	DRIVER( 1943 )		/*  6/1987 (c) 1987 (Euro) */
	DRIVER( 1943u )		/*  6/1987 (c) 1987 (US) */
	DRIVER( 1943j )		/*  6/1987 (c) 1987 (Japan) */
	DRIVER( blktiger )	/*  8/1987 (c) 1987 (US) */
	DRIVER( blktigerb1 )/* bootleg */
	DRIVER( blktigerb2 )/* bootleg */
	DRIVER( blkdrgon )	/*  8/1987 (c) 1987 (Japan) */
	DRIVER( blkdrgonb )	/* bootleg, hacked to say Black Tiger */
	DRIVER( sf )		/*  8/1987 (c) 1987 (World) */
	DRIVER( sfu )		/*  8/1987 (c) 1987 (US) */
	DRIVER( sfua )		/*  8/1987 (c) 1987 (US) */
	DRIVER( sfj )		/*  8/1987 (c) 1987 (Japan) */
	DRIVER( sfp )		/*  8/1987 (c) 1987 */
	DRIVER( tigeroad )	/* 11/1987 (c) 1987 + Romstar (US) */
	DRIVER( toramich )	/* 11/1987 (c) 1987 (Japan) */
	DRIVER( tigeroadb )	/* bootleg */
	DRIVER( f1dream )	/*  4/1988 (c) 1988 + Romstar */
	DRIVER( f1dreamb )	/* bootleg */
	DRIVER( 1943kai )	/*  6/1988 (c) 1987 (Japan) */
	DRIVER( lastduel )	/*  7/1988 (c) 1988 (US) */
	DRIVER( lastduelo )	/*  7/1988 (c) 1988 (US) */
	DRIVER( lastduelj )	/*  7/1988 (c) 1988 (Japan) */
	DRIVER( lastduelb )	/* bootleg */
	DRIVER( madgear )	/*  2/1989 (c) 1989 (US) */
	DRIVER( madgearj )	/*  2/1989 (c) 1989 (Japan) */
	DRIVER( ledstorm )	/*  1988 (c) 1988 (US) */
	DRIVER( ledstorm2 )	/*  1988 (c) 1988 (US) */
	/*  3/1989 Dokaben (baseball) - see below among "Mitchell" games */
	/*  8/1989 Dokaben 2 (baseball) - see below among "Mitchell" games */
	/* 10/1989 Capcom Baseball - see below among "Mitchell" games */
	/* 11/1989 Capcom World - see below among "Mitchell" games */
	/*  3/1990 Adventure Quiz 2 Hatena no Dai-Bouken - see below among "Mitchell" games */
	/*  1/1991 Quiz Tonosama no Yabou - see below among "Mitchell" games */
	/*  4/1991 Ashita Tenki ni Naare (golf) - see below among "Mitchell" games */
	/*  5/1991 Ataxx - see below among "Leland" games */
	/*  6/1991 Quiz Sangokushi - see below among "Mitchell" games */
	/* 10/1991 Block Block - see below among "Mitchell" games */
	/*  6/1995 Street Fighter - the Movie - see below among "Incredible Technologies" games */


	/* Capcom CPS1 games */
	DRIVER( forgottn )	/*  7/1988 (c) 1988 (World) */
	DRIVER( forgottnu )	/*  7/1988 (c) 1988 (US) */
	DRIVER( forgottnua )	/*  7/1988 (c) 1988 (US) */
	DRIVER( lostwrld )	/*  7/1988 (c) 1988 (Japan) */
	DRIVER( lostwrldo )	/*  7/1988 (c) 1988 (Japan) */
	DRIVER( ghouls )	/* 12/1988 (c) 1988 (World) */
	DRIVER( ghoulsu )	/* 12/1988 (c) 1988 (US) */
	DRIVER( daimakai )	/* 12/1988 (c) 1988 (Japan) */
	DRIVER( daimakair )	/* 12/1988 (c) 1988 (Japan) */
	DRIVER( strider )	/*  3/1989 (c) 1989 (not explicitly stated but should be US) */
	DRIVER( striderua )	/*  3/1989 (c) 1989 (not explicitly stated but should be US) */
	DRIVER( striderj )	/*  3/1989 (c) 1989 */
	DRIVER( striderjr )	/*  3/1989 (c) 1989 */
	DRIVER( dynwar )	/*  4/1989 (c) 1989 (US) */
	DRIVER( dynwara )	/*  4/1989 (c) 1989 (US) */
	DRIVER( dynwarj )	/*  4/1989 (c) 1989 (Japan) */
	DRIVER( willow )	/*  6/1989 (c) 1989 (US) */
	DRIVER( willowj )	/*  6/1989 (c) 1989 (Japan) */
	DRIVER( willowje )	/*  6/1989 (c) 1989 (Japan) */
	DRIVER( unsquad )	/*  8/1989 (c) 1989 */
	DRIVER( area88 )	/*  8/1989 (c) 1989 */
	DRIVER( area88r )	/*  8/1989 (c) 1989 */
	DRIVER( ffight )	/* 12/1989 (c) (World) */
	DRIVER( ffightu )	/* 12/1989 (c) (US)    */
	DRIVER( ffightua )	/* 12/1989 (c) (US)    */
	DRIVER( ffightub )	/* 12/1989 (c) (US)    */
	DRIVER( ffightj )	/* 12/1989 (c) (Japan) */
	DRIVER( ffightj1 )	/* 12/1989 (c) (Japan) */
	DRIVER( ffightj2 )	/* 12/1989 (c) (Japan) */
	DRIVER( ffightjh )	/* hack */
	DRIVER( 1941 )		/*  2/1990 (c) 1990 (World) */
	DRIVER( 1941j )		/*  2/1990 (c) 1990 (Japan) */
	DRIVER( mercs )		/* 02/03/1990 (c) 1990 (World) */
	DRIVER( mercsu )	/* 02/03/1990 (c) 1990 (US)    */
	DRIVER( mercsua )	/* 08/06/1990 (c) 1990 (US)    */
	DRIVER( mercsj )	/* 02/03/1990 (c) 1990 (Japan) */
	DRIVER( mtwins )	/* 19/06/1990 (c) 1990 (World) */
	DRIVER( chikij )	/* 19/06/1990 (c) 1990 (Japan) */
	DRIVER( msword )	/* 25/07/1990 (c) 1990 (World) */
	DRIVER( mswordr1 )	/* 23/06/1990 (c) 1990 (World) */
	DRIVER( mswordu )	/* 25/07/1990 (c) 1990 (US)    */
	DRIVER( mswordj )	/* 23/06/1990 (c) 1990 (Japan) */
	DRIVER( cawing )	/* 12/10/1990 (c) 1990 (World) */
	DRIVER( cawingr1 )	/* 09/10/1990 (c) 1990 (World) */
	DRIVER( cawingu )	/* 12/10/1990 (c) 1990 (US) */
	DRIVER( cawingj )	/* 12/10/1990 (c) 1990 (Japan) */
	DRIVER( nemo )		/* 30/11/1990 (c) 1990 (World) */
	DRIVER( nemoj )		/* 20/11/1990 (c) 1990 (Japan) */
	DRIVER( sf2 )		/* 22/05/1991 (c) 1991 (World) */
	DRIVER( sf2eb )		/* 14/02/1991 (c) 1991 (World) */
	DRIVER( sf2ebbl )	/* 14/02/1991 (c) 1991 (World, bootleg) */
	DRIVER( sf2ua )		/* 06/02/1991 (c) 1991 (US)    */
	DRIVER( sf2ub )		/* 14/02/1991 (c) 1991 (US)    */
	DRIVER( sf2ud )		/* 18/03/1991 (c) 1991 (US)    */
	DRIVER( sf2ue )		/* 28/02/1991 (c) 1991 (US)    */
	DRIVER( sf2uf )		/* 11/04/1991 (c) 1991 (US)    */
	DRIVER( sf2ui )		/* 22/05/1991 (c) 1991 (US)    */
	DRIVER( sf2uk )		/* 01/11/1991 (c) 1991 (US)    */
	DRIVER( sf2j )		/* 10/12/1991 (c) 1991 (Japan) */
	DRIVER( sf2ja )		/* 14/02/1991 (c) 1991 (Japan) */
	DRIVER( sf2jc )		/* 06/03/1991 (c) 1991 (Japan) */
	DRIVER( sf2qp1 )	/* hack */
	DRIVER( sf2thndr )	/* hack */
	DRIVER( 3wonders )	/* 20/05/1991 (c) 1991 (World) */
	DRIVER( 3wondersu )	/* 20/05/1991 (c) 1991 (US)    */
	DRIVER( wonder3 )	/* 20/05/1991 (c) 1991 (Japan) */
	DRIVER( 3wondersh )	/* hack */
	DRIVER( kod )		/* 11/07/1991 (c) 1991 (World) */
	DRIVER( kodu )		/* 10/09/1991 (c) 1991 (US)    */
	DRIVER( kodj )		/* 05/08/1991 (c) 1991 (Japan) */
	DRIVER( kodja )		/* 05/08/1991 (c) 1991 (Japan) */
	DRIVER( kodb )		/* bootleg */
	DRIVER( captcomm )	/* 02/12/1991 (c) 1991 (World) */
	DRIVER( captcommr1 )	/* 14/10/1991 (c) 1991 (World) */
	DRIVER( captcommu )	/* 28/ 9/1991 (c) 1991 (US)    */
	DRIVER( captcommj )	/* 02/12/1991 (c) 1991 (Japan) */
	DRIVER( captcommjr1 )	/* 28/ 9/1991 (c) 1991 (Japan) */
	DRIVER( captcommb )	/* bootleg */
	DRIVER( knights )	/* 27/11/1991 (c) 1991 (World) */
	DRIVER( knightsu )	/* 27/11/1991 (c) 1991 (US)    */
	DRIVER( knightsj )	/* 27/11/1991 (c) 1991 (Japan) */
	DRIVER( knightsja )	/* 27/11/1991 (c) 1991 (Japan) */
	DRIVER( knightsb )	/* bootleg */
	DRIVER( sf2ce )		/* 13/05/1992 (c) 1992 (World) */
	DRIVER( sf2ceea )	/* 13/03/1992 (c) 1992 (World) */
	DRIVER( sf2ceua )	/* 13/03/1992 (c) 1992 (US)    */
	DRIVER( sf2ceub )	/* 13/05/1992 (c) 1992 (US)    */
	DRIVER( sf2ceuc )	/* 03/08/1992 (c) 1992 (US)    */
	DRIVER( sf2cej )	/* 13/05/1992 (c) 1992 (Japan) */
	DRIVER( sf2rb )		/* hack */
	DRIVER( sf2rb2 )	/* hack */
	DRIVER( sf2rb3 )	/* hack */
	DRIVER( sf2red )	/* hack */
	DRIVER( sf2v004 )	/* hack */
	DRIVER( sf2acc )	/* hack */
	DRIVER( sf2accp2 )	/* hack */
	DRIVER( sf2dkot2 )	/* hack */
	DRIVER( sf2m1 )		/* hack */
	DRIVER( sf2m2 )		/* hack */
	DRIVER( sf2m3 )		/* hack */
	DRIVER( sf2m4 )		/* hack */
	DRIVER( sf2m5 )		/* hack */
	DRIVER( sf2m6 )		/* hack */
	DRIVER( sf2m7 )		/* hack */
	DRIVER( sf2yyc )	/* hack */
	DRIVER( sf2koryu )	/* hack */
	DRIVER( sf2mdt )	/* bootleg */
	DRIVER( varth )		/* 14/07/1992 (c) 1992 (World) */
	DRIVER( varthr1 )	/* 12/06/1992 (c) 1992 (World) */
	DRIVER( varthu )	/* 12/06/1992 (c) 1992 (US) */
	DRIVER( varthj )	/* 14/07/1992 (c) 1992 (Japan) */
	DRIVER( cworld2j )	/* 11/06/1992 (QUIZ 5) (c) 1992 (Japan) */
	DRIVER( wof )		/* 02/10/1992 (c) 1992 (World) (CPS1 + QSound) */
	DRIVER( wofa )		/* 05/10/1992 (c) 1992 (Asia)  (CPS1 + QSound) */
	DRIVER( wofu )		/* 31/10/1992 (c) 1992 (US) (CPS1 + QSound) */
	DRIVER( wofj )		/* 31/10/1992 (c) 1992 (Japan) (CPS1 + QSound) */
	DRIVER( wofhfh )	/* bootleg */
	DRIVER( sf2hf )		/* 09/12/1992 (c) 1992 (World) */
	DRIVER( sf2hfu )	/* 09/12/1992 (c) 1992 (US)    */
	DRIVER( sf2hfj )	/* 09/12/1992 (c) 1992 (Japan) */
	DRIVER( dino )		/* 01/02/1993 (c) 1993 (World) (CPS1 + QSound) */
	DRIVER( dinou )		/* 01/02/1993 (c) 1993 (US)    (CPS1 + QSound) */
	DRIVER( dinoj )		/* 01/02/1993 (c) 1993 (Japan) (CPS1 + QSound) */
	DRIVER( dinopic )	/* bootleg */
	DRIVER( dinopic2 )	/* bootleg */
	DRIVER( dinohunt )	/* bootleg */
	DRIVER( punisher )	/* 22/04/1993 (c) 1993 (World) (CPS1 + QSound) */
	DRIVER( punisheru )	/* 22/04/1993 (c) 1993 (US)    (CPS1 + QSound) */
	DRIVER( punisherj )	/* 22/04/1993 (c) 1993 (Japan) (CPS1 + QSound) */
	DRIVER( punipic )	/* bootleg */
	DRIVER( punipic2 )	/* bootleg */
	DRIVER( punipic3 )	/* bootleg */
	DRIVER( punisherbz )	/* bootleg */
	DRIVER( slammast )	/* 13/07/1993 (c) 1993 (World) (CPS1 + QSound) */
	DRIVER( slammastu )	/* 13/07/1993 (c) 1993 (US)    (CPS1 + QSound) */
	DRIVER( mbomberj )	/* 13/07/1993 (c) 1993 (Japan) (CPS1 + QSound) */
	DRIVER( mbombrd )	/* 06/12/1993 (c) 1993 (World) (CPS1 + QSound) */
	DRIVER( mbombrdj )	/* 06/12/1993 (c) 1993 (Japan) (CPS1 + QSound) */
	DRIVER( pnickj )	/* 08/06/1994 (c) 1994 Compile + Capcom license (Japan) not listed on Capcom's site */
	DRIVER( qad )		/* 01/07/1992 (c) 1992 (US)    */
	DRIVER( qadj )		/* 21/09/1994 (c) 1994 (Japan) */
	DRIVER( qtono2j )	/* 23/01/1995 (c) 1995 (Japan) */
	DRIVER( pang3 )		/* 01/06/1995 (c) 1995 Mitchell (Euro) not listed on Capcom's site */
	DRIVER( pang3n )	/* 11/05/1995 (c) 1995 Mitchell (Euro) not listed on Capcom's site */
	DRIVER( pang3j )	/* 11/05/1995 (c) 1995 Mitchell (Japan) not listed on Capcom's site */
	DRIVER( megaman )	/* 06/10/1995 (c) 1995 (Asia)  */
	DRIVER( rockmanj )	/* 22/09/1995 (c) 1995 (Japan) */

	DRIVER( fcrash )	/* bootleg of Final Fight */

	/* Capcom CPS2 games */
	/* list completed by CPS2Shock */
	/* http://cps2shock.retrogames.com */
	DRIVER( ssf2 )		/* 11/09/1993 (c) 1993 (World) */
	DRIVER( ssf2u )		/* 11/09/1993 (c) 1993 (US) */
	DRIVER( ssf2a )		/* 05/10/1993 (c) 1993 (Asia) */
	DRIVER( ssf2ar1 )	/* 14/09/1993 (c) 1993 (Asia) */
	DRIVER( ssf2j )		/* 05/10/1993 (c) 1993 (Japan) */
	DRIVER( ssf2jr1 )	/* 11/09/1993 (c) 1993 (Japan) */
	DRIVER( ssf2jr2 )	/* 10/09/1993 (c) 1993 (Japan) */
	DRIVER( ssf2h )		/* 11/09/1993 (c) 1993 (Hispanic) */
	DRIVER( ssf2tb )	/* 11/19/1993 (c) 1993 (World) */
	DRIVER( ssf2tbr1 )	/* 11/09/1993 (c) 1993 (World) */
	DRIVER( ssf2tbj )	/* 11/09/1993 (c) 1993 (Japan) */
	DRIVER( ecofghtr )	/* 03/12/1993 (c) 1993 (World) */
	DRIVER( ecofghtru )	/* 15/02/1994 (c) 1993 (US) */
	DRIVER( ecofghtru1 )/* 03/12/1993 (c) 1993 (US) */
	DRIVER( ecofghtra )	/* 03/12/1993 (c) 1993 (Asia) */
	DRIVER( ecofghtrh )	/* 03/12/1993 (c) 1993 (Hispanic) */
	DRIVER( uecology )	/* 03/12/1993 (c) 1993 (Japan) */
	DRIVER( ddtod )		/* 12/04/1994 (c) 1993 (Euro) */
	DRIVER( ddtodr1 )	/* 13/01/1994 (c) 1993 (Euro) */
	DRIVER( ddtodu )	/* 25/01/1994 (c) 1993 (US) */
	DRIVER( ddtodur1 )	/* 13/01/1994 (c) 1993 (US) */
	DRIVER( ddtodj )	/* 12/04/1994 (c) 1993 (Japan) */
	DRIVER( ddtodjr1 )	/* 25/01/1994 (c) 1993 (Japan) */
	DRIVER( ddtodjr2 )	/* 13/01/1994 (c) 1993 (Japan) */
	DRIVER( ddtoda )	/* 13/01/1994 (c) 1993 (Asia) */
	DRIVER( ddtodh )	/* 12/04/1994 (c) 1993 (Hispanic) */
	DRIVER( ddtodhr1 )	/* 25/01/1994 (c) 1993 (Hispanic) */
	DRIVER( ddtodhr2 )	/* 13/01/1994 (c) 1993 (Hispanic) */
	DRIVER( ssf2t )		/* 23/02/1994 (c) 1994 (World) */
	DRIVER( ssf2tu )	/* 23/03/1994 (c) 1994 (US) */
	DRIVER( ssf2tur1 )	/* 23/02/1994 (c) 1994 (US) */
	DRIVER( ssf2ta )	/* 23/02/1994 (c) 1994 (Asia) */
	DRIVER( ssf2xj )	/* 23/02/1994 (c) 1994 (Japan) */
	DRIVER( avsp )		/* 20/05/1994 (c) 1994 (Euro) */
	DRIVER( avspu )		/* 20/05/1994 (c) 1994 (US) */
	DRIVER( avspj )		/* 20/05/1994 (c) 1994 (Japan) */
	DRIVER( avspa )		/* 20/05/1994 (c) 1994 (Asia) */
	DRIVER( avsph )		/* 20/05/1994 (c) 1994 (Hispanic) */
						/*    06/1994? Galum Pa! (not listed on Capcom's site) */
	DRIVER( dstlk )		/* 05/07/1994 (c) 1994 (Euro) */
	DRIVER( dstlku )	/* 18/08/1994 (c) 1994 (US) */
	DRIVER( dstlkur1 )	/* 05/07/1994 (c) 1994 (US) */
	DRIVER( dstlka )	/* 05/07/1994 (c) 1994 (Asia) */
	DRIVER( dstlkh )	/* 18/08/1994 (c) 1994 (Hispanic) */
	DRIVER( vampj )		/* 05/07/1994 (c) 1994 (Japan) */
	DRIVER( vampja )	/* 05/07/1994 (c) 1994 (Japan) */
	DRIVER( vampjr1 )	/* 30/06/1994 (c) 1994 (Japan) */
	DRIVER( ringdest )	/* 02/09/1994 (c) 1994 (Euro) */
	DRIVER( smbomb )	/* 31/08/1994 (c) 1994 (Japan) */
	DRIVER( smbombr1 )	/* 08/08/1994 (c) 1994 (Japan) */
	DRIVER( armwar )	/* 24/10/1994 (c) 1994 (Euro) */
	DRIVER( armwarr1 )	/* 11/10/1994 (c) 1994 (Euro) */
	DRIVER( armwaru )	/* 24/10/1994 (c) 1994 (US) */
	DRIVER( armwaru1 )	/* 20/09/1994 (c) 1994 (US) */
	DRIVER( pgear )		/* 24/10/1994 (c) 1994 (Japan) */
	DRIVER( pgearr1 )	/* 16/09/1994 (c) 1994 (Japan) */
	DRIVER( armwara )	/* 20/09/1994 (c) 1994 (Asia) */
	DRIVER( xmcota )	/* 05/01/1995 (c) 1994 (Euro) */
	DRIVER( xmcotau )	/* 05/01/1995 (c) 1994 (US) */
	DRIVER( xmcotah )	/* 31/03/1995 (c) 1994 (Hispanic) */
	DRIVER( xmcotahr1 )	/* 05/01/1995 (c) 1994 (Hispanic) */
	DRIVER( xmcotaj )	/* 05/01/1995 (c) 1994 (Japan) */
	DRIVER( xmcotaj1 )	/* 22/12/1994 (c) 1994 (Japan) */
	DRIVER( xmcotaj2 )	/* 19/12/1994 (c) 1994 (Japan) */
	DRIVER( xmcotaj3 )	/* 17/12/1994 (c) 1994 (Japan) */
	DRIVER( xmcotaa )	/* 17/12/1994 (c) 1994 (Asia) */
	DRIVER( xmcotajr )	/* 08/12/1994 (c) 1994 (Japan Rent) */
	DRIVER( nwarr )		/* 16/03/1995 (c) 1995 (Euro) */
	DRIVER( nwarru )	/* 06/04/1995 (c) 1995 (US) */
	DRIVER( nwarrh )	/* 03/04/1995 (c) 1995 (Hispanic) */
	DRIVER( nwarrb )	/* 03/04/1995 (c) 1995 (Brazil) */
	DRIVER( nwarra )	/* 02/03/1995 (c) 1995 (Asia) */
	DRIVER( vhuntj )	/* 02/03/1995 (c) 1995 (Japan) */
	DRIVER( vhuntjr1 )	/* 07/03/1995 (c) 1995 (Japan) */
	DRIVER( vhuntjr2 )	/* 02/03/1995 (c) 1995 (Japan) */
	DRIVER( cybots )	/* 24/04/1995 (c) 1995 (Euro) */
	DRIVER( cybotsu )	/* 24/04/1995 (c) 1995 (US) */
	DRIVER( cybotsj )	/* 20/04/1995 (c) 1995 (Japan) */
	DRIVER( sfa )		/* 27/07/1995 (c) 1995 (Euro) */
	DRIVER( sfar1 )		/* 18/07/1995 (c) 1995 (Euro) */
	DRIVER( sfar2 )		/* 27/06/1995 (c) 1995 (Euro) */
	DRIVER( sfar3 )		/* 05/06/1995 (c) 1995 (Euro) */
	DRIVER( sfau )		/* 27/06/1995 (c) 1995 (US) */
	DRIVER( sfza )		/* 27/06/1995 (c) 1995 (Asia) */
	DRIVER( sfzj )		/* 27/07/1995 (c) 1995 (Japan) */
	DRIVER( sfzjr1 )	/* 27/06/1995 (c) 1995 (Japan) */
	DRIVER( sfzjr2 )	/* 05/06/1995 (c) 1995 (Japan) */
	DRIVER( sfzh )		/* 18/07/1995 (c) 1995 (Hispanic) */
	DRIVER( sfzhr1 )	/* 27/06/1995 (c) 1995 (Hispanic) */
	DRIVER( sfzb )		/* 09/11/1995 (c) 1995 (Brazil) */
	DRIVER( sfzbr1 )	/* 27/07/1995 (c) 1995 (Brazil) */
	DRIVER( mmancp2u )	/* 06/10/1995 (c) 1995 (Japan) */
	DRIVER( rmancp2j )	/* 22/09/1995 (c) 1995 (Japan) */
	DRIVER( msh )		/* 24/10/1995 (c) 1995 (Euro) */
	DRIVER( mshu )		/* 24/10/1995 (c) 1995 (US) */
	DRIVER( mshj )		/* 17/11/1995 (c) 1995 (Japan) */
	DRIVER( mshjr1 )	/* 24/10/1995 (c) 1995 (Japan) */
	DRIVER( msha )		/* 24/10/1995 (c) 1995 (Asia) */
	DRIVER( mshh )		/* 17/11/1995 (c) 1996 (Hispanic) */
	DRIVER( mshb )		/* 17/11/1995 (c) 1996 (Brazil) */
	DRIVER( 19xx )		/* 07/12/1995 (c) 1996 (US) */
	DRIVER( 19xxj )		/* 25/12/1995 (c) 1996 (Japan) */
	DRIVER( 19xxjr1 )	/* 07/12/1995 (c) 1996 (Japan) */
	DRIVER( 19xxa )		/* 07/12/1995 (c) 1996 (Asia) */
	DRIVER( 19xxh )		/* 18/12/1995 (c) 1996 (Hispanic) */
	DRIVER( 19xxb )		/* 18/12/1995 (c) 1996 (Brazil) */
	DRIVER( ddsom )		/* 19/06/1996 (c) 1996 (Euro) */
	DRIVER( ddsomr1 )	/* 23/02/1996 (c) 1996 (Euro) */
	DRIVER( ddsomr2 )	/* 09/02/1996 (c) 1996 (Euro) */
	DRIVER( ddsomr3 )	/* 08/02/1996 (c) 1996 (Euro) */
	DRIVER( ddsomu )	/* 19/06/1996 (c) 1996 (US) */
	DRIVER( ddsomur1 )	/* 09/02/1996 (c) 1996 (US) */
	DRIVER( ddsomjr1 )	/* 06/02/1996 (c) 1996 (Japan) */
	DRIVER( ddsomj )	/* 19/06/1996 (c) 1996 (Japan) */
	DRIVER( ddsoma )	/* 19/06/1996 (c) 1996 (Asia) */
	DRIVER( ddsomh )	/* 23/02/1996 (c) 1996 (Hispanic) */
	DRIVER( ddsomb )	/* 23/02/1996 (c) 1996 (Brazil) */
	DRIVER( sfa2 )		/* 06/03/1996 (c) 1996 (Euro) */
	DRIVER( sfa2u )		/* 06/03/1996 (c) 1996 (US) */
	DRIVER( sfz2j )		/* 27/02/1996 (c) 1996 (Japan) */
	DRIVER( sfz2a )		/* 27/02/1996 (c) 1996 (Asia) */
	DRIVER( sfz2b )		/* 31/05/1996 (c) 1996 (Brazil) */
	DRIVER( sfz2br1 )	/* 04/03/1996 (c) 1996 (Brazil) */
	DRIVER( sfz2h )		/* 04/03/1996 (c) 1996 (Hispanic) */
	DRIVER( sfz2n )		/* 29/02/1996 (c) 1996 (Oceania) */
	DRIVER( spf2t )		/* 20/06/1996 (c) 1996 (US) */
	DRIVER( spf2xj )	/* 31/05/1996 (c) 1996 (Japan) */
	DRIVER( spf2ta )	/* 29/05/1996 (c) 1996 (Asia) */
	DRIVER( spf2th )	/* 31/05/1996 (c) 1996 (Hispanic) */
	DRIVER( qndream )	/* 26/06/1996 (c) 1996 (Japan) */
	DRIVER( megaman2 )	/* 08/07/1996 (c) 1996 (US) */
	DRIVER( megaman2a )	/* 08/07/1996 (c) 1996 (Asia) */
	DRIVER( rockman2j )	/* 08/07/1996 (c) 1996 (Japan) */
	DRIVER( megaman2h )	/* 12/07/1996 (c) 1996 (Hispanic) */
	DRIVER( gigamn2 )	/* bootleg */
	DRIVER( sfz2al )	/* 26/08/1996 (c) 1996 (Asia) */
	DRIVER( sfz2alj )	/* 05/08/1996 (c) 1996 (Japan) */
	DRIVER( sfz2alh )	/* 13/08/1996 (c) 1996 (Hispanic) */
	DRIVER( sfz2alb )	/* 13/08/1996 (c) 1996 (Brazil) */
	DRIVER( xmvsf )		/* 04/10/1996 (c) 1996 (Euro) */
	DRIVER( xmvsfr1 )	/* 10/09/1996 (c) 1996 (Euro) */
	DRIVER( xmvsfu )	/* 23/10/1996 (c) 1996 (US) */
	DRIVER( xmvsfur1 )	/* 04/10/1996 (c) 1996 (US) */
	DRIVER( xmvsfj )	/* 04/10/1996 (c) 1996 (Japan) */
	DRIVER( xmvsfjr1 )	/* 10/09/1996 (c) 1996 (Japan) */
	DRIVER( xmvsfjr2 )	/* 09/09/1996 (c) 1996 (Japan) */
	DRIVER( xmvsfa )	/* 23/10/1996 (c) 1996 (Asia) */
	DRIVER( xmvsfar1 )	/* 19/09/1996 (c) 1996 (Asia) */
	DRIVER( xmvsfar2 )	/* 10/09/1996 (c) 1996 (Asia) */
	DRIVER( xmvsfh )	/* 04/10/1996 (c) 1996 (Hispanic) */
	DRIVER( xmvsfb )	/* 23/10/1996 (c) 1996 (Brazil) */
	DRIVER( batcir )	/* 19/03/1997 (c) 1997 (Euro) */
	DRIVER( batcirj )	/* 19/03/1997 (c) 1997 (Japan) */
	DRIVER( batcira )	/* 19/03/1997 (c) 1997 (Asia) */
	DRIVER( vsav )		/* 19/05/1997 (c) 1997 (Euro) */
	DRIVER( vsavu )		/* 19/05/1997 (c) 1997 (US) */
	DRIVER( vsavj )		/* 19/05/1997 (c) 1997 (Japan) */
	DRIVER( vsava )		/* 19/05/1997 (c) 1997 (Asia) */
	DRIVER( vsavh )		/* 19/05/1997 (c) 1997 (Hispanic) */
	DRIVER( mshvsf )	/* 25/06/1997 (c) 1997 (Euro) */
	DRIVER( mshvsfu )	/* 27/08/1997 (c) 1997 (US) */
	DRIVER( mshvsfu1 )	/* 25/06/1997 (c) 1997 (US) */
	DRIVER( mshvsfj )	/* 07/07/1997 (c) 1997 (Japan) */
	DRIVER( mshvsfj1 )	/* 02/07/1997 (c) 1997 (Japan) */
	DRIVER( mshvsfj2 )	/* 25/06/1997 (c) 1997 (Japan) */
	DRIVER( mshvsfh )	/* 25/06/1997 (c) 1997 (Hispanic) */
	DRIVER( mshvsfa )	/* 25/06/1997 (c) 1997 (Asia) */
	DRIVER( mshvsfa1 )	/* 20/06/1997 (c) 1997 (Asia) */
	DRIVER( mshvsfb )	/* 27/08/1997 (c) 1997 (Brazil) */
	DRIVER( mshvsfb1 )	/* 25/06/1997 (c) 1997 (Brazil) */
	DRIVER( csclub )	/* 17/10/1997 (c) 1997 (Euro) */
	DRIVER( csclub1 )	/* 22/07/1997 (c) 1997 (Euro) */
	DRIVER( csclubj )	/* 22/07/1997 (c) 1997 (Japan) */
	DRIVER( cscluba )	/* 22/07/1997 (c) 1997 (Asia) */
	DRIVER( csclubh )	/* 22/07/1997 (c) 1997 (Hispanic) */
	DRIVER( sgemf )		/* 04/09/1997 (c) 1997 (US) */
	DRIVER( pfghtj )	/* 04/09/1997 (c) 1997 (Japan) */
	DRIVER( sgemfa )	/* 04/09/1997 (c) 1997 (Asia) */
	DRIVER( sgemfh )	/* 04/09/1997 (c) 1997 (Hispanic) */
	DRIVER( vhunt2r1 )	/* 13/09/1997 (c) 1997 (Japan) */
	DRIVER( vhunt2 )	/* 29/09/1997 (c) 1997 (Japan) */
	DRIVER( vsav2 )		/* 13/09/1997 (c) 1997 (Japan) */
	DRIVER( mvsc )		/* 23/01/1998 (c) 1998 (Euro) */
	DRIVER( mvscr1 )	/* 12/01/1998 (c) 1998 (Euro) */
	DRIVER( mvscu )		/* 23/01/1998 (c) 1998 (US) */
	DRIVER( mvscur1 )	/* 23/01/1998 (c) 1998 (US) */
	DRIVER( mvscj )		/* 23/01/1998 (c) 1998 (Japan) */
	DRIVER( mvscjr1 )	/* 12/01/1998 (c) 1998 (Japan) */
	DRIVER( mvsca )		/* 23/01/1998 (c) 1998 (Asia) */
	DRIVER( mvscar1 )	/* 12/01/1998 (c) 1998 (Asia) */
	DRIVER( mvsch )		/* 23/01/1998 (c) 1998 (Hispanic) */
	DRIVER( mvscb )		/* 23/01/1998 (c) 1998 (Brazil) */
	DRIVER( sfa3 )		/* 04/09/1998 (c) 1998 (US) */
	DRIVER( sfa3u )		/* 04/09/1998 (c) 1998 (US) */
	DRIVER( sfa3ur1 )	/* 29/06/1998 (c) 1998 (US) */
	DRIVER( sfa3h )		/* 29/06/1998 (c) 1998 (Hispanic) */
	DRIVER( sfa3b )		/* 29/06/1998 (c) 1998 (Brazil) */
	DRIVER( sfz3j )		/* 04/09/1998 (c) 1998 (Japan) */
	DRIVER( sfz3jr1 )	/* 27/07/1998 (c) 1998 (Japan) */
	DRIVER( sfz3jr2 )	/* 29/06/1998 (c) 1998 (Japan) */
	DRIVER( sfz3a )		/* 04/09/1998 (c) 1998 (Asia) */
	DRIVER( sfz3ar1 )	/* 01/07/1998 (c) 1998 (Asia) */
	DRIVER( gigawing )	/* 22/02/1999 (c) 1999 Takumi (US) */
	DRIVER( gigawingj )	/* 23/02/1999 (c) 1999 Takumi (Japan) */
	DRIVER( gigawinga )	/* 22/02/1999 (c) 1999 Takumi (Asia) */
	DRIVER( gigawingh )	/* 22/02/1999 (c) 1999 Takumi (Hispanic) */
	DRIVER( gigawingb )	/* 22/02/1999 (c) 1999 Takumi (Brazil) */
	DRIVER( jyangoku )	/* 27/05/1999 (c) 1999 (Japan) */
	DRIVER( dimahoo )	/* 21/01/2000 (c) 2000 Eighting/Raizing (US) */
	DRIVER( dimahoou )	/* 21/01/2000 (c) 2000 Eighting/Raizing (US) */
	DRIVER( gmahou )	/* 21/01/2000 (c) 2000 Eighting/Raizing (Japan) */
	DRIVER( mmatrix )	/* 12/04/2000 (c) 2000 Takumi (US) */
	DRIVER( mmatrixj )	/* 12/04/2000 (c) 2000 Takumi (Japan) */
	DRIVER( 1944 )		/* 20/06/2000 (c) 2000 Eighting/Raizing (US) */
	DRIVER( 1944j )		/* 20/06/2000 (c) 2000 Eighting/Raizing (Japan) */
	DRIVER( mpang )		/* 10/10/2000 (c) 2000 Mitchell (Euro) */
	DRIVER( mpangr1 )	/* 25/09/2000 (c) 2000 Mitchell (Euro) */
	DRIVER( mpangu )	/* 10/10/2000 (c) 2000 Mitchell (US) */
	DRIVER( mpangj )	/* 11/10/2000 (c) 2000 Mitchell (Japan) */
	DRIVER( progear )	/* 17/01/2001 (c) 2001 Cave (US) */
	DRIVER( progearj )	/* 17/01/2001 (c) 2001 Cave (Japan) */
	DRIVER( progeara )	/* 17/01/2001 (c) 2001 Cave (Asia) */
	DRIVER( pzloop2 )	/* 02/03/2001 (c) 2001 Mitchell (Euro) */
	DRIVER( pzloop2j )	/* 05/02/2001 (c) 2001 Mitchell (Japan) */
	DRIVER( choko )		/* 20/08/2001 (c) 2001 Mitchell (Japan) */
	DRIVER( hsf2 )		/* 02/02/2004 (c) 2004 Hyper Street Fighter 2 (USA) */
	DRIVER( hsf2j )		/* 22/12/2003 (c) 2003 Hyper Street Fighter 2 (Japan) */
	DRIVER( hsf2a )		/* 02/02/2004 (c) 2004 Hyper Street Fighter 2 (Asia) */

	/* CPS2 'Phoenix' bootlegs */
	DRIVER( ssf2ud )
	DRIVER( ddtodd )
	DRIVER( ecofghtrd )
	DRIVER( avspd )
	DRIVER( dstlku1d )
	DRIVER( ringdstd )
	DRIVER( xmcotad )
	DRIVER( nwarrud )
	DRIVER( sfad )
	DRIVER( mshud )
	DRIVER( 19xxd )
	DRIVER( sfz2ad )
	DRIVER( sfz2jd )
	DRIVER( spf2td )
	DRIVER( spf2xjd )
	DRIVER( ddsomud )
	DRIVER( megamn2d )
	DRIVER( sfz2ald )
	DRIVER( xmvsfu1d )
	DRIVER( batcird )
	DRIVER( vsavd )
	DRIVER( mvscud )
	DRIVER( sfa3ud )
	DRIVER( gigawingd )
	DRIVER( gigawingjd )
	DRIVER( 1944d )
	DRIVER( progearud )
	DRIVER( progearjd )
	DRIVER( progearjbl )
	DRIVER( hsf2d )
	DRIVER( ssf2tbd )
	DRIVER( ssf2xjd )
	DRIVER( sgemfd )
	DRIVER( armwar1d )
	DRIVER( dimahoud )
	DRIVER( mmatrixd )
	DRIVER( vhunt2d )
	DRIVER( vsav2d )
	DRIVER( csclub1d )
	DRIVER( cybotsud )
	DRIVER( mshvsfu1d )
	DRIVER( sfz3jr2d )

	 /* Capcom CPS3 games */
	DRIVER( sfiii )		/* 04/02/1997 (c) 1997 Capcom */
	DRIVER( sfiiij )	/* 04/02/1997 (c) 1997 Capcom */
	DRIVER( sfiii2 )	/* 30/09/1997 (c) 1997 Capcom */
	DRIVER( sfiii2j )	/* 30/09/1997 (c) 1997 Capcom */
	DRIVER( sfiii3 )	/* 08/06/1999 (c) 1999 Capcom */
	DRIVER( sfiii3a )	/* 12/05/1999 (c) 1999 Capcom */
	DRIVER( jojo )		/* 08/01/1999 (c) 1998 Capcom */
	DRIVER( jojoa )		/* 02/12/1998 (c) 1998 Capcom */
	DRIVER( jojoba )	/* 13/09/1999 (c) 1999 Capcom */
	DRIVER( redearth )	/* 21/11/1996 (c) 1996 Capcom */
	DRIVER( warzard )	/* 21/11/1996 (c) 1996 Capcom */

	DRIVER( sfiiin )	/* 04/02/1997 (c) 1997 Capcom */
	DRIVER( sfiii2n )	/* 30/09/1997 (c) 1997 Capcom */
	DRIVER( sfiii3n )	/* 08/06/1999 (c) 1999 Capcom */
	DRIVER( sfiii3an )	/* 12/05/1999 (c) 1999 Capcom */
	DRIVER( jojon )		/* 08/01/1999 (c) 1998 Capcom */
	DRIVER( jojoan )	/* 02/12/1998 (c) 1998 Capcom */
	DRIVER( jojoban )	/* 13/09/1999 (c) 1999 Capcom */
	DRIVER( jojobane )	/* 13/09/1999 (c) 1999 Capcom */
	DRIVER( redeartn )	/* 21/11/1996 (c) 1996 Capcom */

	/* Capcom ZN1 */
	DRIVER( cpzn1 )
	DRIVER( ts2 )		/* Battle Arena Toshinden 2 (USA 951124) */
	DRIVER( ts2j )		/* Battle Arena Toshinden 2 (JAPAN 951124) */
	DRIVER( starglad )	/* Star Gladiator (USA 960627) */
	DRIVER( stargladj )	/* Star Gladiator (JAPAN 960627) */
	DRIVER( sfex )		/* Street Fighter EX (EURO 961219) */
	DRIVER( sfexu )		/* Street Fighter EX (USA 961219) */
	DRIVER( sfexa )		/* Street Fighter EX (ASIA 961219) */
	DRIVER( sfexj )		/* Street Fighter EX (JAPAN 961130) */
	DRIVER( glpracr )	/* Gallop Racer (JAPAN Ver 9.01.12) */
	DRIVER( sfexp )		/* Street Fighter EX Plus (USA 970407) */
	DRIVER( sfexpu1 )	/* Street Fighter EX Plus (USA 970311) */
	DRIVER( sfexpj )	/* Street Fighter EX Plus (JAPAN 970311) */

	/* Capcom ZN2 */
	DRIVER( cpzn2 )
	DRIVER( rvschool )	/* Rival Schools (USA 971117) */
	DRIVER( rvschoola )	/* Rival Schools (ASIA 971117) */
	DRIVER( jgakuen )	/* Justice Gakuen (JAPAN 971117) */
	DRIVER( sfex2 )		/* Street Fighter EX 2 (USA 980526) */
	DRIVER( sfex2j )	/* Street Fighter EX 2 (JAPAN 980312) */
	DRIVER( sfex2a )	/* Street Fighter EX 2 (ASIA 980312) */
	DRIVER( sfex2h )	/* Street Fighter EX 2 (HISPANIC 980312) */
	DRIVER( plsmaswd )	/* Plasma Sword (USA 980316) */
	DRIVER( stargld2 )	/* Star Gladiator 2 (JAPAN 980316) */
	DRIVER( plsmaswda )	/* Plasma Sword (ASIA 980316) */
	DRIVER( tgmj )		/* Tetris The Grand Master (JAPAN 980710) */
	DRIVER( techromn )	/* Tech Romancer (EURO 980914) */
	DRIVER( techromnu )	/* Tech Romancer (USA 980914) */
	DRIVER( kikaioh )	/* Kikaioh (JAPAN 980914) */
	DRIVER( sfex2p )	/* Street Fighter EX 2 Plus (USA 990611) */
	DRIVER( sfex2pa )	/* Street Fighter EX 2 Plus (ASIA 990611) */
	DRIVER( sfex2pj )	/* Street Fighter EX 2 Plus (JAPAN 990611) */
	DRIVER( strider2 )	/* Strider 2 (USA 991213) */
	DRIVER( strider2a )	/* Strider 2 (ASIA 991213) */
	DRIVER( shiryu2 )	/* Strider Hiryu 2 (JAPAN 991213) */

	/* Tecmo ZN1 */
	DRIVER( tps )
	DRIVER( glpracr2 )	/* Gallop Racer 2 (USA) */
	DRIVER( glpracr2j )	/* Gallop Racer 2 (JAPAN) */
	DRIVER( glpracr2l )	/* Gallop Racer 2 Link HW (JAPAN) */
	DRIVER( doapp )		/* Dead Or Alive ++ (JAPAN) */
	DRIVER( cbaj )		/* Cool Boaders Arcade Jam */
	DRIVER( shngmtkb )	/* Shanghai Matekibuyuu */
	DRIVER( tondemo )	/* Tondemo Crisis (JAPAN) */
	DRIVER( glpracr3 )	/* Gallop Racer 3 (JAPAN) */
	DRIVER( flamegun )	/* Flame Gunner (USA) */
	DRIVER( flamegunj )	/* Flame Gunner (JAPAN) */
	DRIVER( lpadv )		/* Logic Pro Adventure (JAPAN) */
	DRIVER( tblkkuzu )	/* The Block Kuzushi (JAPAN) */
	DRIVER( 1on1gov )	/* 1 on 1 Government (JAPAN) */
	DRIVER( tecmowcm )	/* Tecmo World Cup Millenium (JAPAN) */
	DRIVER( mfjump )	/* Monster Farm Jump (JAPAN) */

	/* Atlus ZN1 */
	DRIVER( atluspsx )
	DRIVER( hvnsgate )	/* Heaven's Gate (JAPAN) */

	/* PS Arcade 95 */
	DRIVER( psarc95 )
	DRIVER( beastrzr )	/* Beastorizer (USA) */
	DRIVER( bldyroar )	/* Bloody Roar (JAPAN) */
	DRIVER( beastrzb )	/* Beastorizer (USA Bootleg) */
	DRIVER( bldyror2 )	/* Bloody Roar 2 (WORLD) */
	DRIVER( bldyror2u )	/* Bloody Roar 2 (USA) */
	DRIVER( bldyror2a )	/* Bloody Roar 2 (ASIA) */
	DRIVER( bldyror2j )	/* Bloody Roar 2 (JAPAN) */
	DRIVER( brvblade )	/* Brave Blade (WORLD) */
	DRIVER( brvbladeu )	/* Brave Blade (USA) */
	DRIVER( brvbladea )	/* Brave Blade (ASIA) */
	DRIVER( brvbladej )	/* Brave Blade (JAPAN) */
	DRIVER( bam2 )		/* Bust a Move 2 (JAPANESE ROM ver. 1999/07/17 10:00:00) */

	/* Atari PSX */
	DRIVER( atpsx )
	DRIVER( primrag2 )	/* Primal Rage 2 */

	/* Acclaim PSX */
	DRIVER( acpsx )
	DRIVER( nbajamex )	/* NBA Jam Extreme */
	DRIVER( jdredd )	/* Judge Dredd (Rev C) */
	DRIVER( jdreddb )	/* Judge Dredd (Rev B) */

	/* Video System ZN1 */
	DRIVER( vspsx )
	DRIVER( aerofgts )	/* Aero Fighters Special (TAIWAN) */
	DRIVER( sncwgltd )	/* Sonic Wings Limited (JAPAN) */

	/* Taito FX1a */
	DRIVER( taitofx1 )
	DRIVER( sfchamp )	/* Super Football Champ (Ver 2.5O) */
	DRIVER( sfchamp24o )/* Super Football Champ (Ver 2.4O) */
	DRIVER( sfchampj )	/* Super Football Champ (Ver 2.4J) */
	DRIVER( psyforce )	/* Psychic Force (Ver 2.4O) */
	DRIVER( psyforcej )	/* Psychic Force (Ver 2.4J) */
	DRIVER( psyforcex )	/* Psychic Force EX (Ver 2.0J) */
	DRIVER( mgcldate )	/* Magical Date (JAPAN Ver 2.02J) */
	DRIVER( mgcldtex )	/* Magical Date EX (JAPAN Ver 2.01J) */

	/* Taito FX1b */
	DRIVER( raystorm )	/* Ray Storm (Ver 2.06A) */
	DRIVER( raystormj )	/* Ray Storm (Ver 2.05J) */
	DRIVER( ftimpact )	/* Fighters' Impact (Ver 2.02O) */
	DRIVER( ftimpactj )	/* Fighters' Impact (Ver 2.02J) */
	DRIVER( ftimpcta )	/* Fighters' Impact A (Ver 2.00J) */
	DRIVER( gdarius )	/* G-Darius (Ver 2.01J) */
	DRIVER( gdariusb )	/* G-Darius (Ver 2.02A) */
	DRIVER( gdarius2 )	/* G-Darius Ver.2 (Ver 2.03J) */

	/* Taito GNET */
	DRIVER( taitogn )
	DRIVER( chaoshea )	/* 1998.10 Chaos Heat (V2.09O) */
	DRIVER( chaosheaj )	/* 1998.10 Chaos Heat (V2.08J) */
	DRIVER( raycris )	/* 1998.12 Ray Crisis (V2.03J) */
	DRIVER( otenamih )	/* 1999.02 Otenami Haiken (Success) */
	DRIVER( otenamhf )	/* 2005.04 Otenami Haiken Final (V2.07JC) */
	DRIVER( spuzboblj )	/* 1999.03 Super Puzzle Bobble (Japan) */
	DRIVER( spuzbobl )	/* 1999.?? Super Puzzle Bobble (V2.05O) */
	DRIVER( gobyrc )	/* 1999.?? Go By RC (V2.03O) */
	DRIVER( rcdego )	/* 1999.06 RC De Go (V2.03J) */
	DRIVER( flipmaze )	/* 1999.09 Flip Maze (V2.04J) */
	DRIVER( mahjngoh )	/* 1999.12 Mahjong Oh (V2.06J) */
	DRIVER( psyvaria )	/* 2000.03 Psyvariar -Medium Unit- (V2.04J) */
	DRIVER( shanghss )	/* 2000.07 Shanghai Shoryu Sairin (V2.03J) */
	DRIVER( psyvarrv )	/* 2000.09 Psyvariar -Revision- (V2.04J) */
	DRIVER( zokuoten )	/* 2000.12 Zoku Otenamihaiken (V2.03J) */
						/* 2000.?? Taito Classic (Success, not released?) */
	DRIVER( soutenry )	/* 2001.02 Soutenryu (V2.07J) */
	DRIVER( nightrai )	/* 2001.05 Night Raid (V2.03J) */
	DRIVER( shikigam )	/* 2001.09 Shikigami no Shiro */
	DRIVER( otenki )	/* 2001.10 Otenki Kororin */
	DRIVER( usagi )		/* 2001.12 Usagi (V2.02J) */
	DRIVER( shangtou )	/* 2002.02 Shanghai Sangokuhaitougi */
	DRIVER( xiistag )	/* 2002.07 XII Stag (V2.01J) */
	DRIVER( sianniv )	/* 2003.10 Space Invaders Anniversary (V2.02J) */
	DRIVER( kollon )	/* 2003.11 Kollon (V2.04J) */
	DRIVER( kollonc )	/* 2003.11 Kollon (V2.04JC, Compact Flash version) */
	DRIVER( zooo )		/* 2004.05 Zooo */

	/* Namco System 11 */
	DRIVER( tekken )	/* 1994.?? Tekken (TE4/VER.C) */
	DRIVER( tekkena )	/* 1994.?? Tekken (TE2/VER.C) */
	DRIVER( tekkenb )	/* 1994.?? Tekken (TE2/VER.B) */
	DRIVER( tekkenc )	/* 1994.12 Tekken (TE1/VER.B, Japan) */
	DRIVER( tekken2 )	/* 1995.?? Tekken 2 Ver.B (TES3/VER.B) */
	DRIVER( tekken2a )	/* 1995.?? Tekken 2 Ver.B (TES2/VER.B) */
						/* 1995.10 Tekken 2 Ver.B (TES1/VER.B, Japan) */
	DRIVER( tekken2b )	/* 1995.?? Tekken 2 (TES2/VER.A) */
						/* 1995.08 Tekken 2 (TES1, Japan) */
	DRIVER( souledge1 )	/* 1996.?? Soul Edge (SO3/VER.A) */
	DRIVER( souledge1j )/* 1996.02 Soul Edge (SO1/VER.A, Japan) */
	DRIVER( primglex )	/* 1996.03 Prime Goal EX (PG1/VER.A, Japan) */
	DRIVER( xevi3dg )	/* 1996.05 Xevious 3D/G (XV31/VER.A, Japan) */
	DRIVER( souledge )	/* 1996.?? Soul Edge Ver. II (SO4/VER.C) */
	DRIVER( souledgea )	/* 1996.?? Soul Edge Ver. II (SO3/VER.C) */
						/* 1996.05 Soul Edge Ver. II (SO1, Japan) */
	DRIVER( dunkmnia )	/* 1996.?? Dunk Mania (DM1/VER.C) */
	DRIVER( dunkmniaj )	/* 1996.05 Dunk Mania (DM1/VER.C, Japan) */
	DRIVER( danceyes )	/* 1996.09 Dancing Eyes (DC1/VER.A, Japan) */
	DRIVER( pocketrc )	/* 1997.03 Pocket Racer (PKR1/VER.B, Japan) */
	DRIVER( starswep )	/* 1997.07 Star Sweep (STP1/VER.A, Japan) */
	DRIVER( myangel3 )	/* 1998.03 Kosodate Quiz My Angel 3 (KQT1/VER.A, Japan) */
	DRIVER( ptblnk2a )	/* 1999.?? Point Blank 2 (GNB3/VER.A) */
						/* 1999.02 Gunbaarl (GNB1, Japan) */

	/* Namco System 12 */
	DRIVER( tekken3 )	/* 1997.03 Tekken 3 (TET1/VER.E1, Japan) */
	DRIVER( tekken3a )	/* 1997.?? Tekken 3 (TET2/VER.B) */
	DRIVER( tekken3b )	/* 1997.?? Tekken 3 (TET3/VER.A) */
	DRIVER( tekken3c )	/* 1997.?? Tekken 3 (TET2/VER.A) */
	DRIVER( lbgrande )	/* 1997.?? Libero Grande (LG2/VER.A) */
						/* 1997.12 Libero Grande (LG1, Japan) */
	DRIVER( toukon3 )	/* 1997.12 Shin Nihon Pro Wrestling Toukon Retsuden 3 Arcade Edition (TR1/VER.A, Japan) */
	DRIVER( ehrgeiz )	/* 1998.?? Ehrgeiz (EG3/VER.A) */
	DRIVER( ehrgeiza )	/* 1998.?? Ehrgeiz (EG2/VER.A) */
	DRIVER( ehrgeizj )	/* 1998.05 Ehrgeiz (EG1, Japan) */
	DRIVER( sws98 )		/* 1998.06 Super World Stadium '98 (SS81/VER.A, Japan) */
	DRIVER( soulclbr )	/* 1998.?? Soul Calibur (SOC14/VER.C) */
	DRIVER( soulclbrb )	/* 1998.?? Soul Calibur (SOC14/VER.B) */
	DRIVER( soulclbrj )	/* 1998.07 Soul Calibur (SOC11/VER.C, Japan) */
	DRIVER( soulclbrb2 )	/* 1998.?? Soul Calibur (SOC13/VER.B) */
	DRIVER( soulclbrjb )/* 1998.?? Soul Calibur (SOC11/VER.B, Japan) */
	DRIVER( soulclbrja )/* 1998.?? Soul Calibur (SOC11/VER.A2, Japan) */
						/* 1998.07 Techno Drive */
	DRIVER( mdhorse )	/* 1998.11 Derby Quiz My Dream Horse (MDH1/VER.A2, Japan) */
						/* 1998.12 Attack Pla Rail */
	DRIVER( tenkomor )	/* 1998.?? Tenkomori Shooting (TKM2/VER.A1) */
	DRIVER( tenkomorj )	/* 1998.12 Tenkomori Shooting (TKM1/VER.A1, Japan) */
	DRIVER( pacapp )	/* 1998.12 Paca Paca Passion (PPP1/VER.A2, Japan) */
	DRIVER( fgtlayer )	/* 1998.12 Fighting Layer (FTL0/VER.A, Japan) */
						/* 1999.01 Bust A Move (Metro) */
	DRIVER( ptblank2 )	/* 1999.?? Point Blank 2 (GNB5/VER.A) */
	DRIVER( sws99 )		/* 1999.05 Super World Stadium '99 (SS91/VER.A3, Japan) */
	DRIVER( ghlpanic )	/* 1999.?? Ghoul Panic (OB2/VER.A) */
						/* 1999.07 Oh Bakyuun (OB1, Japan) */
	DRIVER( pacapp2 )	/* 1999.07 Paca Paca Passion 2 (PKS1/VER.A, Japan) */
						/* 1999.07 Bust A Move 2 (Metro) */
	DRIVER( tektagt )	/* 1999.?? Tekken Tag Tournament (TEG3/VER.C1) */
	DRIVER( tektagta )	/* 1999.?? Tekken Tag Tournament (TEG3/VER.B) */
	DRIVER( tektagtb )	/* 1999.07 Tekken Tag Tournament (TEG1/VER.B, Japan) */
	DRIVER( tektagtc )	/* 1999.07 Tekken Tag Tournament (TEG1/VER.A, Japan) */
	DRIVER( mrdrillr )	/* 1999.11 Mr Driller (DRI1/VER.A2, Japan) */
						/* 1999.12 UmJammer Lammy NOW! (Japan) */
						/* 1999.12 Quest for Fame (Japan) */
	DRIVER( pacappsp )	/* 1999.12 Paca Paca Passion Special (PSP1/VER.A, Japan) */
	DRIVER( kaiunqz )	/* 1999.12 Kaiun Quiz (KW1/VER.A, Japan) */
	DRIVER( aquarush )	/* 2000.02 Aqua Rush (AQ1/VER.A1, Japan) */
	DRIVER( golgo13 )	/* 2000.03 Golgo 13 (GLG1/VER.A, Japan) */
	DRIVER( sws2000 )	/* 2000.03 Super World Stadium 2000 (SS01/VER.A, Japan) */
						/* 2000.06 Truck Kyousoukyoku (Metro) */
						/* 2000.07 Kart Duel */
						/* 2000.08 Teknowerk */
	DRIVER( g13knd )	/* 2000.10 Golgo 13 Kiseki no Dandou (GLS1/VER.A, Japan) */
	DRIVER( sws2001 )	/* 2001.04 Super World Stadium 2001 (SS11/VER.A, Japan) */

	/* Namco System 10 */
	DRIVER( mrdrilr2 )	/* 2000.07 Mr Driller 2 (DR21 Ver.A) */
	DRIVER( mrdrlr2a )      /* 2000.?? Mr Driller 2 (DR22 Ver.A) */
						/* 2000.12 Gunbarina */
						/* 2001.02 Photo Battole */
						/* 2001.02 Taiko no Tatsujin */
						/* 2001.03 Gahaha Ippatsudou */
	DRIVER( mrdrilrg )	/* 2001.03 Mr. Driller G (DRG1 Ver.A) */
						/* 2001.07 Golgo 13 Juusei no Requiem */
						/* 2001.08 Taiko no Tatsujin 2 */
						/* 2001.09 Gahaha Ippatsudou 2 */
						/* 2001.11 Knock Down 2001 */
	DRIVER( knpuzzle )	/* 2001.12 Kotoba no Puzzle Mojipittan (KPM1 Ver.A) */
	DRIVER( gjspace )	/* 2001.12 Gekitoride-Jong Space (10011 Ver.A) */
						/* 2002.03 Panicuru Panekuru */
						/* 2002.03 Taiko no Tatsujin 3 */
	DRIVER( startrgn )	/* 2002.07 Star Trigon (STT1 Ver.A) */
						/* 2002.07 Tsukkomi Yousei Gips Nice Tsukkomi */
	DRIVER( gamshara )	/* 2002.08 Gamshara (10021 Ver.A) */
						/* 2002.10 Uchuu Daisakusen : Chocovader Contactee */
						/* 2002.12 Taiko no Tatsujin 4 */
						/* 2003.04 Seisyun Quiz Colorful High School */
						/* 2003.10 Taiko no Tatsujin 5 */
						/* 2004.07 Taiko no Tatsujin 6 */

	/* Konami GQ */
	DRIVER( cryptklr )	/* Crypt Killer (GQ420 UAA) */

	/* Konami GV */
	DRIVER( konamigv )
	DRIVER( kdeadeye )	/* GV054, UA01 */
	DRIVER( pbball96 )	/* GV017, Japan 1.03 */
	DRIVER( hyperath )	/* GV021, Japan 1.00 */
	DRIVER( susume )	/* GV027, Japan 1.20 */
	DRIVER( btchamp )	/* GV053, UAA01 */
	DRIVER( weddingr )	/* GX624, JAA */
	DRIVER( tokimosh )	/* GE755, JAA */
	DRIVER( tokimosp )	/* GE756, JAB */
	DRIVER( nagano98 )	/* GX720, EAA */
	DRIVER( simpbowl )	/* GQ829, UAA */

	/* Konami System 573 */
	DRIVER( sys573 )
	DRIVER( darkhleg )	/* 1998 - Dark Horse Legend (GX706 VER. JAA) */
	DRIVER( fbaitbc )	/* 1998 - Fisherman's Bait - A Bass Challenge (GE765 VER. UAB) */
	DRIVER( bassangl )	/* 1998 - Bass Angler (GE765 VER. JAA) */
	DRIVER( pbballex )	/* 1998 - Powerful Pro Baseball EX (GX802 VER. JAB) */
	DRIVER( konam80s )	/* 1998 - Konami 80's AC Special (GC826 VER. EAA) */
	DRIVER( konam80u )	/* 1998 - Konami 80's AC Special (GC826 VER. UAA) */
	DRIVER( konam80j )	/* 1998 - Konami 80's Gallery (GC826 VER. JAA) */
	DRIVER( konam80a )	/* 1998 - Konami 80's AC Special (GC826 VER. AAA) */
	DRIVER( konam80k )	/* 1998 - Konami 80's AC Special (GC826 VER. KAA) */
	DRIVER( dstage )	/* 1999 - Dancing Stage (GN845 VER. EAA) */
	DRIVER( ddru )		/* 1999 - Dance Dance Revolution (GN845 VER. UAA) */
	DRIVER( ddrj )		/* 1998 - Dance Dance Revolution - Internet Ranking Ver (GC845 VER. JBA) */
	DRIVER( ddrja )		/* 1998 - Dance Dance Revolution (GC845 VER. JAA) */
	DRIVER( ddrjb )		/* 1998 - Dance Dance Revolution (GC845 VER. JAB) */
	DRIVER( ddra )		/* 1999 - Dance Dance Revolution (GN845 VER. AAA) */
	DRIVER( fbait2bc )	/* 1998 - Fisherman's Bait 2 - A Bass Challenge (GE865 VER. UAB) */
	DRIVER( bassang2 )	/* 1998 - Bass Angler 2 (GE865 VER. JAA) */
	DRIVER( drmn )		/* 1999 - DrumMania (GQ881 VER. JAD) */
	DRIVER( gtrfrks )	/* 1999 - Guitar Freaks (GQ886 VER. EAC) */
	DRIVER( gtrfrksu )	/* 1999 - Guitar Freaks (GQ886 VER. UAC) */
	DRIVER( gtrfrksj )	/* 1999 - Guitar Freaks (GQ886 VER. JAC) */
	DRIVER( gtrfrksa )	/* 1999 - Guitar Freaks (GQ886 VER. AAC) */
	DRIVER( fbaitmc )	/* 1999 - Fisherman's Bait - Marlin Challenge (GX889 VER. EA) */
	DRIVER( fbaitmcu )	/* 1999 - Fisherman's Bait - Marlin Challenge (GX889 VER. UA) */
	DRIVER( fbaitmcj )	/* 1999 - Fisherman's Bait - Marlin Challenge (GX889 VER. JA) */
	DRIVER( fbaitmca )	/* 1999 - Fisherman's Bait - Marlin Challenge (GX889 VER. AA) */
	DRIVER( ddr2m )		/* 1999 - Dance Dance Revolution 2nd Mix (GN895 VER. JAA) */
	DRIVER( ddrbocd )	/* 1999 - Dance Dance Revolution Best of Cool Dancers (GE892 VER. JAA) */
	DRIVER( ddr2ml )	/* 1999 - Dance Dance Revolution 2nd Mix - Link Ver (GE885 VER. JAA) */
	DRIVER( ddr2mc )	/* 1999 - Dance Dance Revolution 2nd Mix with beatmaniaIIDX CLUB VERSiON (GE896 VER. JAA) */
	DRIVER( ddr2mc2 )	/* 1999 - Dance Dance Revolution 2nd Mix with beatmaniaIIDX substream CLUB VERSiON 2 (GE984 VER. JAA) */
	DRIVER( gtrfrk2m )	/* 1999 - Guitar Freaks 2nd Mix Ver 1.01 (GQ883 VER. JAD) */
	DRIVER( dsftkd )	/* 1999 - Dancing Stage featuring TRUE KiSS DESTiNATiON (G*884 VER. JAA) */
	DRIVER( cr589fw )	/* 1999 - CR-589 Firmware (700B04) */
	DRIVER( cr589fwa )	/* 1999 - CR-589 Firmware (700A04) */
	DRIVER( ddr3mk )	/* 2000 - Dance Dance Revolution 3rd Mix - Ver.Korea2 (GN887 VER. KBA) */
	DRIVER( ddr3mka )	/* 2000 - Dance Dance Revolution 3rd Mix - Ver.Korea (GN887 VER. KAA) */
	DRIVER( ddr3ma )	/* 1999 - Dance Dance Revolution 3rd Mix (GN887 VER. AAA) */
	DRIVER( ddr3mj )	/* 1999 - Dance Dance Revolution 3rd Mix (GN887 VER. JAA) */
	DRIVER( ddrsbm )	/* 1999 - Dance Dance Revolution Solo Bass Mix (GQ894 VER. JAA) */
	DRIVER( ddrs2k )	/* 1999 - Dance Dance Revolution Solo 2000 (GC905 VER. AAA) */
	DRIVER( ddrs2kj )	/* 1999 - Dance Dance Revolution Solo 2000 (GC905 VER. JAA) */
	DRIVER( hyperbbc )	/* 1999 - Hyper Bishi Bashi Champ (GX908 1999/08/24 VER. JAA) */
	DRIVER( hyperbbck )	/* 1999 - Hyper Bishi Bashi Champ (GX908 1999/08/24 VER. KAA) */
	DRIVER( dsfdct )	/* 1999 - Dancing Stage featuring Dreams Come True (GC910 VER. JCA) */
	DRIVER( dsfdcta )	/* 1999 - Dancing Stage featuring Dreams Come True (GC910 VER. JAA) */
	DRIVER( drmn2m )	/* 1999 - DrumMania 2nd Mix (GE912 VER. JAB) */
	DRIVER( drmn2mpu )	/* 1999 - DrumMania 2nd Mix Session Power Up Kit (GE912 VER. JAB) */
	DRIVER( dncfrks )	/* 2000 - Dance Freaks (G*874 VER. KAA) */
	DRIVER( dmx )		/* 2000 - Dance Maniax (G*874 VER. JAA) */
	DRIVER( dsem )		/* 2000 - Dancing Stage Euro Mix (G*936 VER. EAA) */
	DRIVER( gtrfrk3m )	/* 2000 - Guitar Freaks 3rd Mix (GE949 VER. JAC) */
	DRIVER( gtfrk3ma )	/* 2000 - Guitar Freaks 3rd Mix (GE949 VER. JAB) */
	DRIVER( gtfrk3mb )	/* 2000 - Guitar Freaks 3rd Mix - security cassette versionup (949JAZ02) */
	DRIVER( salarymc )	/* 2000 - Salary Man Champ (G*A18 VER. JAA) */
	DRIVER( ddr3mp )	/* 2000 - Dance Dance Revolution 3rd Mix Plus (G*A22 VER. JAA) */
	DRIVER( pcnfrk3m )	/* 2000 - Percussion Freaks 3rd Mix (G*A23 VER. KAA) */
	DRIVER( drmn3m )	/* 2000 - DrumMania 3rd Mix (G*A23 VER. JAA) */
	DRIVER( gtrfrk4m )	/* 2000 - Guitar Freaks 4th Mix (G*A24 VER. JAA) */
	DRIVER( ddr4m )		/* 2000 - Dance Dance Revolution 4th Mix (G*A33 VER. AAA) */
	DRIVER( ddr4mj )	/* 2000 - Dance Dance Revolution 4th Mix (G*A33 VER. JAA) */
	DRIVER( ddr4ms )	/* 2000 - Dance Dance Revolution Solo 4th Mix (G*A33 VER. ABA) */
	DRIVER( ddr4msj )	/* 2000 - Dance Dance Revolution Solo 4th Mix (G*A33 VER. JBA) */
	DRIVER( dsfdr )		/* 2000 - Dancing Stage Featuring Disney's Rave (GCA37JAA) */
	DRIVER( ddrusa )	/* 2000 - Dance Dance Revolution USA (G*A44 VER. UAA) */
	DRIVER( ddr4mp )	/* 2000 - Dance Dance Revolution 4th Mix Plus (G*A34 VER. JAA) */
	DRIVER( ddr4mps )	/* 2000 - Dance Dance Revolution 4th Mix Plus Solo (G*A34 VER. JAA) */
	DRIVER( dmx2m )		/* 2000 - Dance Maniax 2nd Mix (G*A39 VER. JAA) */
	DRIVER( fmania )	/* 2000 - Fighting Mania (918 xx B02) */
	DRIVER( gtrfrk5m )	/* 2001 - Guitar Freaks 5th Mix (G*A26 VER. JAA) */
	DRIVER( ddr5m )		/* 2001 - Dance Dance Revolution 5th Mix (G*A27 VER. JAA) */
	DRIVER( dmx2majp )	/* 2001 - Dance Maniax 2nd Mix Append J-Paradise (G*A38 VER. JAA ) */
	DRIVER( gtrfrk6m )	/* 2001 - Guitar Freaks 6th Mix (G*B06 VER. JAA) */
	DRIVER( gtrfrk7m )	/* 2001 - Guitar Freaks 6th Mix (G*B17 VER. JAA) */
	DRIVER( ddrmax )	/* 2001 - DDR Max - Dance Dance Revolution 6th Mix (G*B19 VER. JAA) */
	DRIVER( ddrmax2 )	/* 2002 - DDR Max 2 - Dance Dance Revolution 7th Mix (G*B20 VER. JAA) */
	DRIVER( dsem2 )		/* 2002 - Dancing Stage Euro Mix 2 (G*C23 VER. EAA) */
	DRIVER( ddrextrm )	/* 2002 - Dance Dance Revolution Extreme (G*C36 VER. JAA) */
	DRIVER( gtfrk11m )	/* 2004 - Guitar Freaks 11th Mix (G*D39 VER. JAA) */

	/* Konami Twinkle games */
	DRIVER( gq863 )
	DRIVER( bmiidx )
	DRIVER( bmiidxc )
	DRIVER( bmiidxca )
	DRIVER( bmiidxc2 )
	DRIVER( bmiidx3 )
	DRIVER( bmiidx4 )
	DRIVER( bmiidx6 )
	DRIVER( bmiidx7 )
	DRIVER( bmiidx8 )

	/* Konami Viper games */
	DRIVER( kviper )	/* 1999 */
	DRIVER( ppp2nd )	/* 2001 */
	DRIVER( boxingm )	/* 2001 */
	DRIVER( code1d )	/* 2000 */
	DRIVER( code1db )	/* 2000 */
	DRIVER( gticlub2 )	/* 2001 */
	DRIVER( gticlub2ea )/* 2001 */
	DRIVER( jpark3 )	/* 2001 */
	DRIVER( mocapglf )	/* 2001 */
	DRIVER( mocapb )	/* 2001 */
	DRIVER( mocapbj )	/* 2001 */
	DRIVER( p911 )		/* 2001 */
	DRIVER( p911uc )	/* 2001 */
	DRIVER( p911kc )	/* 2001 */
	DRIVER( p911e )		/* 2001 */
	DRIVER( p911j )		/* 2001 */
	DRIVER( p9112 )		/* 2001 */
	DRIVER( popn9 )		/* 2003 */
	DRIVER( sscopex )	/* 2001 */
	DRIVER( sogeki )	/* 2001 */
	DRIVER( thrild2 )	/* 2001 */
	DRIVER( thrild2a )	/* 2001 */
	DRIVER( thrild2c )	/* 2001 */
	DRIVER( tsurugi )	/* 2001 */
	DRIVER( tsurugij )	/* 2001 */
	DRIVER( wcombat )	/* 2002 */
	DRIVER( wcombatk )	/* 2002 */
	DRIVER( wcombatj )	/* 2002 */
	DRIVER( xtrial )	/* 2002 */
	DRIVER( mfightc )	/* 2002 */
	DRIVER( mfightcc )	/* 2002 */

	/* Mitchell games */
	DRIVER( mgakuen )	/* (c) 1988 Yuga */
	DRIVER( 7toitsu )	/* (c) 1988 Yuga */
	DRIVER( mgakuen2 )	/* (c) 1989 Face */
	DRIVER( pkladies )	/* (c) 1989 Mitchell */
	DRIVER( pkladiesl )	/* (c) 1989 Leprechaun */
	DRIVER( pkladiesla )/* (c) 1989 Leprechaun */
	DRIVER( pkladiesbl )/* bootleg */
	DRIVER( dokaben )	/*  3/1989 (c) 1989 Capcom (Japan) */
						/*  8/1989 Dokaben 2 (baseball) */
	DRIVER( pang )		/* (c) 1989 Mitchell (World) */
	DRIVER( pangb )		/* bootleg */
	DRIVER( pangbold )	/* bootleg */
	DRIVER( pangba )	/* bootleg */
	DRIVER( bbros )		/* (c) 1989 Capcom (US) not listed on Capcom's site */
	DRIVER( pompingw )	/* (c) 1989 Mitchell (Japan) */
	DRIVER( cbasebal )	/* 10/1989 (c) 1989 Capcom (Japan) (different hardware) */
	DRIVER( cworld )	/* 11/1989 (QUIZ 1) (c) 1989 Capcom */
	DRIVER( hatena )	/* 28/02/1990 (QUIZ 2) (c) 1990 Capcom (Japan) */
	DRIVER( spang )		/* 14/09/1990 (c) 1990 Mitchell (World) */
	DRIVER( spangj )	/* --/--/1990 (c) 1990 Mitchell (Japan) */
	DRIVER( spangbl )	/* bootleg */
	DRIVER( sbbros )	/* 01/10/1990 (c) 1990 Mitchell + Capcom (US) not listed on Capcom's site */
	DRIVER( marukin )	/* 17/10/1990 (c) 1990 Yuga (Japan) */
	DRIVER( qtono1 )	/* 25/12/1990 (QUIZ 3) (c) 1991 Capcom (Japan) */
						/*  4/1991 Ashita Tenki ni Naare (golf) */
	DRIVER( qsangoku )	/* 07/06/1991 (QUIZ 4) (c) 1991 Capcom (Japan) */
	DRIVER( block )		/* 10/09/1991 (c) 1991 Capcom (World) */
	DRIVER( blockj )	/* 10/09/1991 (c) 1991 Capcom (Japan) */
	DRIVER( blockjoy )	/* 06/11/1991 (c) 1991 Capcom (World) (Joystick version, bad dump?) */
	DRIVER( blockbl )	/* bootleg */

	/* Incredible Technologies games */
	/* http://www.itsgames.com */
	DRIVER( capbowl )	/* (c) 1988 Incredible Technologies */
	DRIVER( capbowl2 )	/* (c) 1988 Incredible Technologies */
	DRIVER( capbowl3 )	/* (c) 1988 Incredible Technologies */
	DRIVER( capbowl4 )	/* (c) 1989 Incredible Technologies */
	DRIVER( clbowl )	/* (c) 1989 Incredible Technologies */
	DRIVER( bowlrama )	/* (c) 1991 P & P Marketing */
	DRIVER( wfortune )	/* (c) 1989 GameTek */
	DRIVER( wfortunea )	/* (c) 1989 GameTek */
	DRIVER( grmatch )	/* (c) 1989 GameTek */
	DRIVER( stratab )	/* (c) 1990 Strata/Incredible Technologies */
	DRIVER( stratab1 )	/* (c) 1990 Strata/Incredible Technologies */
	DRIVER( sstrike )	/* (c) 1990 Strata/Incredible Technologies */
	DRIVER( gtg )		/* (c) 1990 Strata/Incredible Technologies */
	DRIVER( gtgt )		/* (c) 1990 Strata/Incredible Technologies */
	DRIVER( hstennis )	/* (c) 1990 Strata/Incredible Technologies */
	DRIVER( hstennis10 )/* (c) 1990 Strata/Incredible Technologies */
	DRIVER( slikshot )	/* (c) 1990 Grand Products/Incredible Technologies */
	DRIVER( slikshot17 )/* (c) 1990 Grand Products/Incredible Technologies */
	DRIVER( slikshot16 )/* (c) 1990 Grand Products/Incredible Technologies */
	DRIVER( dynobop )	/* (c) 1990 Grand Products/Incredible Technologies */
	DRIVER( arlingtn )	/* (c) 1991 Strata/Incredible Technologies */
	DRIVER( peggle )	/* (c) 1991 Strata/Incredible Technologies */
	DRIVER( pegglet )	/* (c) 1991 Strata/Incredible Technologies */
	DRIVER( pokrdice )	/* (c) 1991 Strata/Incredible Technologies */
	DRIVER( rimrockn )	/* (c) 1991 Strata/Incredible Technologies */
	DRIVER( rimrockn20 )/* (c) 1991 Strata/Incredible Technologies */
	DRIVER( rimrockn16 )/* (c) 1991 Strata/Incredible Technologies */
	DRIVER( rimrockn12 )/* (c) 1991 Strata/Incredible Technologies */
	DRIVER( ninclown )	/* (c) 1991 Strata/Incredible Technologies */
	DRIVER( gpgolf )	/* (c) 1992 Strata/Incredible Technologies */
	DRIVER( gtg2 )		/* (c) 1992 Strata/Incredible Technologies */
	DRIVER( gtg2t )		/* (c) 1989 Strata/Incredible Technologies */
	DRIVER( gtg2j )		/* (c) 1991 Strata/Incredible Technologies */
	DRIVER( neckneck )	/* (c) 1992 Bundra Games/Incredible Technologies */
	DRIVER( timekill )	/* (c) 1992 Strata/Incredible Technologies */
	DRIVER( timekill131 )	/* (c) 1992 Strata/Incredible Technologies */
	DRIVER( hardyard )	/* (c) 1993 Strata/Incredible Technologies */
	DRIVER( hardyard10 )	/* (c) 1993 Strata/Incredible Technologies */
	DRIVER( bloodstm )	/* (c) 1994 Strata/Incredible Technologies */
	DRIVER( bloodstm22 )	/* (c) 1994 Strata/Incredible Technologies */
	DRIVER( bloodstm21 )	/* (c) 1994 Strata/Incredible Technologies */
	DRIVER( bloodstm11 )	/* (c) 1994 Strata/Incredible Technologies */
	DRIVER( pairs )		/* (c) 1994 Strata/Incredible Technologies */
	DRIVER( pairsa )	/* (c) 1994 Strata/Incredible Technologies */
	DRIVER( hotmemry )	/* (c) 1994 Tuning/Incredible Technologies */
	DRIVER( drivedge )	/* (c) 1994 Strata/Incredible Technologies */
	DRIVER( wcbowl )	/* (c) 1995 Incredible Technologies */
	DRIVER( wcbowl165 )	/* (c) 1995 Incredible Technologies */
	DRIVER( wcbowl161 )	/* (c) 1995 Incredible Technologies */
	DRIVER( wcbowl16 )	/* (c) 1995 Incredible Technologies */
	DRIVER( wcbowl140 )	/* (c) 1995 Incredible Technologies */
	DRIVER( wcbowl15 )	/* (c) 1995 Incredible Technologies */
	DRIVER( wcbowl13 )	/* (c) 1995 Incredible Technologies */
	DRIVER( wcbowl12 )	/* (c) 1995 Incredible Technologies */
	DRIVER( sftm )		/* (c) 1995 Capcom/Incredible Technologies */
	DRIVER( sftm110 )	/* (c) 1995 Capcom/Incredible Technologies */
	DRIVER( sftm111 )	/* (c) 1995 Capcom/Incredible Technologies */
	DRIVER( sftmj )		/* (c) 1995 Capcom/Incredible Technologies */
	DRIVER( shufshot )	/* (c) Strata/Incredible Technologies */
	DRIVER( shufshot139 )	/* (c) Strata/Incredible Technologies */
	DRIVER( shufshot137 )	/* (c) Strata/Incredible Technologies */
	DRIVER( wcbowldx )	/* (c) 1999 Incredible Technologies */

	DRIVER( gt3d )		/* (c) 1995 Incredible Technologies */
	DRIVER( gt3dl192 )	/* (c) 1995 Incredible Technologies */
	DRIVER( gt3dl191 )	/* (c) 1995 Incredible Technologies */
	DRIVER( gt3ds192 )	/* (c) 1995 Incredible Technologies */
	DRIVER( gt3dv18 )	/* (c) 1995 Incredible Technologies */
	DRIVER( gt3dv17 )	/* (c) 1995 Incredible Technologies */
	DRIVER( gt3dv16 )	/* (c) 1995 Incredible Technologies */
	DRIVER( gt3dv15 )	/* (c) 1995 Incredible Technologies */
	DRIVER( gt3dv14 )	/* (c) 1995 Incredible Technologies */
	DRIVER( gt3dt231 )	/* (c) 1995 Incredible Technologies */
	DRIVER( gt3dt211 )	/* (c) 1995 Incredible Technologies */
	DRIVER( gt97 )		/* (c) 1997 Incredible Technologies */
	DRIVER( gt97v122 )	/* (c) 1997 Incredible Technologies */
	DRIVER( gt97v121 )	/* (c) 1997 Incredible Technologies */
	DRIVER( gt97s121 )	/* (c) 1997 Incredible Technologies */
	DRIVER( gt97v120 )	/* (c) 1997 Incredible Technologies */
	DRIVER( gt97t240 )	/* (c) 1997 Incredible Technologies */
	DRIVER( gt97t243 )	/* (c) 1997 Incredible Technologies */
	DRIVER( gt98 )		/* (c) 1998 Incredible Technologies */
	DRIVER( gt98v100 )	/* (c) 1998 Incredible Technologies */
	DRIVER( gt98s100 )	/* (c) 1998 Incredible Technologies */
	DRIVER( gt98t303 )	/* (c) 1998 Incredible Technologies */
	DRIVER( gt99 )		/* (c) 1999 Incredible Technologies */
	DRIVER( gt99s100 )	/* (c) 1999 Incredible Technologies */
	DRIVER( gt99t400 )	/* (c) 1999 Incredible Technologies */
	DRIVER( gtroyal )	/* (c) 1999 Incredible Technologies */
	DRIVER( gt2k )		/* (c) 2000 Incredible Technologies */
	DRIVER( gt2kp100 )	/* (c) 2000 Incredible Technologies */
	DRIVER( gt2ks100 )	/* (c) 2000 Incredible Technologies */
	DRIVER( gt2kt500 )	/* (c) 2000 Incredible Technologies */
	DRIVER( gtsuprem )	/* (c) 2000 Incredible Technologies */
	DRIVER( gtclassc )	/* (c) 2001 Incredible Technologies */
	DRIVER( gtclasscp )	/* (c) 2001 Incredible Technologies */
	DRIVER( gtclasscs )	/* (c) 2001 Incredible Technologies */

	/* Leland games */
	DRIVER( cerberus )	/* (c) 1985 Cinematronics */
	DRIVER( mayhem )	/* (c) 1985 Cinematronics */
	DRIVER( powrplay )	/* (c) 1985 Cinematronics */
	DRIVER( wseries )	/* (c) 1985 Cinematronics */
	DRIVER( alleymas )	/* (c) 1986 Cinematronics */
	DRIVER( upyoural )	/* (c) 1987 Cinematronics */
	DRIVER( dangerz )	/* (c) 1986 Cinematronics USA */
	DRIVER( basebal2 )	/* (c) 1987 Cinematronics */
	DRIVER( dblplay )	/* (c) 1987 Tradewest / Leland */
	DRIVER( strkzone )	/* (c) 1988 Leland */
	DRIVER( redlin2p )	/* (c) 1987 Cinematronics + Tradewest license */
	DRIVER( quarterb )	/* (c) 1987 Leland */
	DRIVER( quarterba )	/* (c) 1987 Leland */
	DRIVER( viper )		/* (c) 1988 Leland */
	DRIVER( teamqb )	/* (c) 1988 Leland */
	DRIVER( teamqb2 )	/* (c) 1988 Leland */
	DRIVER( aafb )		/* (c) 1989 Leland */
	DRIVER( aafbd2p )	/* (c) 1989 Leland */
	DRIVER( aafbc )		/* (c) 1989 Leland */
	DRIVER( aafbb )		/* (c) 1989 Leland */
	DRIVER( offroad )	/* (c) 1989 Leland */
	DRIVER( offroadt )	/* (c) 1989 Leland */
	DRIVER( offroadt2p )/* (c) 1989 Leland */
	DRIVER( pigout )	/* (c) 1990 Leland */
	DRIVER( pigouta )	/* (c) 1990 Leland */
	DRIVER( ataxx )		/* (c) 1990 Leland */
	DRIVER( ataxxa )	/* (c) 1990 Leland */
	DRIVER( ataxxj )	/* (c) 1990 Leland */
	DRIVER( wsf )		/* (c) 1990 Leland */
	DRIVER( indyheat )	/* (c) 1991 Leland */
	DRIVER( brutforc )	/* (c) 1991 Leland */
	DRIVER( asylum )	/* (c) 1991 Leland */

	/* Gremlin 8080 games */
	/* the numbers listed are the range of ROM part numbers */
	DRIVER( blockade )	/* 1-4 [1977 Gremlin] */
	DRIVER( comotion )	/* 5-7 [1977 Gremlin] */
	DRIVER( hustle )	/* 16-21 [1977 Gremlin] */
	DRIVER( blasto )	/* [1978 Gremlin] */
	DRIVER( mineswpr )	/* [1977 Amutech] */
	DRIVER( mineswpr4 )	/* [1977 Amutech] */

	/* Gremlin/Sega "VIC dual game board" games */
	/* the numbers listed are the range of ROM part numbers */
	DRIVER( depthch )	/* 50-55 [1977 Gremlin?] */
	DRIVER( depthcho )	/*   ?   [1977 Gremlin?] */
	DRIVER( subhunt )	/*   ?   [1977 Taito] */
	DRIVER( safari )	/* 57-66 [1977 Gremlin?] */
	DRIVER( safaria )	/* bootleg? */
	DRIVER( frogs )		/* 112-119 [1978 Gremlin?] */
	DRIVER( sspaceat )	/* 155-162 (c) */
	DRIVER( sspaceat2 )	/* 155-162 (c) 1979 Sega */
	DRIVER( sspaceat3 )	/* 155-162 (c) 1979 Sega */
	DRIVER( sspaceatc )	/* 139-146 (c) */
	DRIVER( sspacaho )	/* ? epr00001.bin - epr00008.bin */
	DRIVER( headon )	/* 163-167/192-193 (c) Gremlin */
	DRIVER( headonb )	/* 163-167/192-193 (c) Gremlin */
	DRIVER( headons )	/* Sidam bootleg */
	DRIVER( headonsa )	/* Sidam bootleg */
	DRIVER( supcrash )	/* bootleg */
	DRIVER( headon2 )	/* ???-??? (c) 1979 Sega */
	DRIVER( headon2s )	/* Sidam bootleg */
	DRIVER( car2 )		/* bootleg */
						/* ???-??? Fortress */
						/* ???-??? Gee Bee */
						/* 255-270  Head On 2 / Deep Scan */
	DRIVER( invho2 )	/* 271-286 (c) 1979 Sega */
	DRIVER( nsub )		/* 268-275 (c) 1980 Sega */
	DRIVER( samurai )	/* 289-302 + upgrades (c) 1980 Sega */
	DRIVER( invinco )	/* 310-318 (c) 1979 Sega */
	DRIVER( invds )		/* 367-382 (c) 1979 Sega */
	DRIVER( tranqgun )	/* 413-428 (c) 1980 Sega */
						/* 450-465  Tranquilizer Gun (different version?) */
						/* ???-??? Car Hunt / Deep Scan */
	DRIVER( spacetrk )	/* 630-645 (c) 1980 Sega */
	DRIVER( spacetrkc )	/* (c) 1980 Sega */
	DRIVER( carnival )	/* 651-666 (c) 1980 Sega */
	DRIVER( carnivalc )	/* 501-516 (c) 1980 Sega */
	DRIVER( carnivalh )
	DRIVER( carnivalha )
	DRIVER( brdrline )	/* (c) 1980 Sega */
	DRIVER( brdrlins )	/* Sidam bootleg */
	DRIVER( brdrlinb )	/* Karateco bootleg */
	DRIVER( digger )	/* 684-691 no copyright notice */
	DRIVER( pulsar )	/* 790-805 (c) 1981 Sega */
	DRIVER( heiankyo )	/* (c) [1979?] Denki Onkyo */
	DRIVER( alphaho )	/* Data East */

	/* Sega G-80 vector games */
	DRIVER( spacfury )	/* (c) 1981 */
	DRIVER( spacfurya )	/* no copyright notice */
	DRIVER( spacfuryb )	/* no copyright notice */
	DRIVER( zektor )	/* (c) 1982 */
	DRIVER( tacscan )	/* (c) 1982 */
	DRIVER( elim2 )		/* (c) 1981 Gremlin */
	DRIVER( elim2a )	/* (c) 1981 Gremlin */
	DRIVER( elim2c )	/* (c) 1981 Gremlin */
	DRIVER( elim4 )		/* (c) 1981 Gremlin */
	DRIVER( elim4p )	/* (c) 1981 Gremlin */
	DRIVER( startrek )	/* (c) 1982 */

	/* Sega G-80 raster games */
	DRIVER( astrob )	/* (c) 1981 */
	DRIVER( astrob2 )	/* (c) 1981 */
	DRIVER( astrob2a )	/* (c) 1981 */
	DRIVER( astrob1 )	/* (c) 1981 */
	DRIVER( astrobg )	/* (c) 1981 */
	DRIVER( 005 )		/* (c) 1981 */
	DRIVER( monsterb )	/* (c) 1982 */
	DRIVER( monsterb2 )	/* (c) 1982 */
	DRIVER( spaceod )	/* (c) 1981 */
	DRIVER( spaceod2 )	/* (c) 1981 */
	DRIVER( pignewt )	/* (c) 1983 */
	DRIVER( pignewta )	/* (c) 1983 */
	DRIVER( sindbadm )	/* 834-5244 (c) 1983 Sega */

	/* Sega "Zaxxon hardware" games */
	DRIVER( zaxxon )	/* (c) 1982 */
	DRIVER( zaxxonj )	/* (c) 1982 */
	DRIVER( zaxxon2 )	/* (c) 1982 */
	DRIVER( zaxxon3 )	/* (c) 1982 */
	DRIVER( zaxxonb )	/* bootleg */
	DRIVER( szaxxon )	/* (c) 1982 */
	DRIVER( futspy )	/* (c) 1984 */
	DRIVER( razmataz )	/* modified 834-0213, 834-0214 (c) 1983 */
	DRIVER( ixion )		/* (c) 1983 */
	DRIVER( congo )		/* 605-5167 (c) 1983 */
	DRIVER( tiptop )	/* 605-5167 (c) 1983 */

	/* Sega "LaserDisc hardware" games */
	DRIVER( astron )	/* (c) 1983 */
	DRIVER( astronp )	/* (c) 1983 */
	DRIVER( galaxyr )	/* (c) 1983 */
	DRIVER( galaxyrp )	/* (c) 1983 */
	DRIVER( sblazerp )	/* (c) 1983 */
	DRIVER( cobraseg )	/* (c) ???? */
	DRIVER( gpworld )	/* (c) 1984 */
	DRIVER( timetrv )	/* (c) 1991.10 Virtual Image Productions */

	/* Sega SG1000 based games */
	DRIVER( chboxing )	/* (c) 1984 */
	DRIVER( chwrestl )	/* (c) 1985 */
	DRIVER( dokidoki )	/* (c) 1985 */

	/* Sega System 1 / System 2 games */
	DRIVER( starjack )	/* 834-5191 (c) 1983 (S1) */
	DRIVER( starjacks )	/* (c) 1983 Stern (S1) */
	DRIVER( regulus )	/* 834-5328?(c) 1983 (S1) */
	DRIVER( reguluso )	/* 834-5328?(c) 1983 (S1) */
	DRIVER( regulusu )	/* 834-5328?(c) 1983 (S1) */
	DRIVER( upndown )	/* (c) 1983 (S1) */
	DRIVER( upndownu )	/* (c) 1983 (S1) */
	DRIVER( mrviking )	/* 834-5383 (c) 1984 (S1) */
	DRIVER( mrvikingj )	/* 834-5383 (c) 1984 (S1) */
	DRIVER( swat )		/* 834-5388 (c) 1984 Coreland / Sega (S1) */
	DRIVER( flicky )	/* (c) 1984 (S1) */
	DRIVER( flickys2 )	/* (c) 1984 (S1) */
	DRIVER( flickyo )	/* (c) 1984 (S1) */
	DRIVER( flickys1 )	/* (c) 1984 (S1) */
	DRIVER( wmatch )	/* (c) 1984 (S1) */
	DRIVER( bullfgt )	/* 834-5478 (c) 1984 Sega / Coreland (S1) */
	DRIVER( thetogyu )	/* 834-5478 (c) 1984 Sega / Coreland (S1) */
	DRIVER( spatter )	/* 834-5583 (c) 1984 (S1) */
	DRIVER( ssanchan )	/* 834-5583 (c) 1984 (S1) */
	DRIVER( pitfall2 )	/* 834-5627 [1985?] reprogrammed, (c) 1984 Activision (S1) */
	DRIVER( pitfall2a )	/* 834-5627 [1985?] reprogrammed, (c) 1984 Activision (S1) */
	DRIVER( pitfall2u )	/* 834-5627 [1985?] reprogrammed, (c) 1984 Activision (S1) */
	DRIVER( seganinj )	/* 834-5677 (c) 1985 (S1) */
	DRIVER( seganinju )	/* 834-5677 (c) 1985 (S1) */
	DRIVER( ninja )		/* 834-5677 (c) 1985 (S1) */
	DRIVER( nprinces )	/* 834-5677 (c) 1985 (S1) */
	DRIVER( nprinceso )	/* 834-5677 (c) 1985 (S1) */
	DRIVER( nprincesu )	/* 834-5677 (c) 1985 (S1) */
	DRIVER( nprincesb )	/* bootleg? (S1) */
	DRIVER( imsorry )	/* 834-5707 (c) 1985 Coreland / Sega (S1) */
	DRIVER( imsorryj )	/* 834-5707 (c) 1985 Coreland / Sega (S1) */
	DRIVER( teddybb )	/* 834-5712 (c) 1985 (S1) */
	DRIVER( teddybbo )	/* 834-5712 (c) 1985 (S1) */
	DRIVER( hvymetal )	/* 834-5745 (c) 1985 (S2?) */
	DRIVER( myhero )	/* 834-5755 (c) 1985 (S1) */
	DRIVER( sscandal )	/* 834-5755 (c) 1985 Coreland / Sega (S1) */
	DRIVER( myherok )	/* 834-5755 (c) 1985 Coreland / Sega (S1) */
	DRIVER( shtngmst )	/* 834-5719/5720 (c) 1985 (S2) */
	DRIVER( shtngmste )	/* 834-5719/5720 (c) 1985 (S2) */
	DRIVER( choplift )	/* 834-5795 (c) 1985, (c) 1982 Dan Gorlin (S2) */
	DRIVER( chopliftu )	/* 834-5795 (c) 1985, (c) 1982 Dan Gorlin (S2) */
	DRIVER( chopliftbl )/* bootleg (S2) */
	DRIVER( 4dwarrio )	/* 834-5918 (c) 1985 Coreland / Sega (S1) */
	DRIVER( brain )		/* (c) 1986 Coreland / Sega (S2?) */
	DRIVER( raflesia )	/* 834-5753 (c) 1985 Coreland / Sega (S1) */
	DRIVER( wboy )		/* 834-5984 (c) 1986 + Escape license (S1) */
	DRIVER( wboyo )		/* 834-5984 (c) 1986 + Escape license (S1) */
	DRIVER( wboy2 )		/* 834-5984 (c) 1986 + Escape license (S1) */
	DRIVER( wboy2u )	/* 834-5984 (c) 1986 + Escape license (S1) */
	DRIVER( wboy3 )		/* 834-5984 (c) 1986 + Escape license (S1) */
	DRIVER( wboy4 )		/* 834-5984 (c) 1986 + Escape license (S1) */
	DRIVER( wboyu )		/* 834-5753 (? maybe a conversion) (c) 1986 + Escape license (S1) */
	DRIVER( wbdeluxe )	/* (c) 1986 + Escape license (S1) */
	DRIVER( wboysys2 )	/* 834-5984  (c) 1986 + Escape license (S1) */
	DRIVER( gardia )	/* 834-6119 (S2?) */
	DRIVER( gardiab )	/* bootleg */
	DRIVER( nob )		/* (c) Data East */
	DRIVER( nobb )		/* bootleg */
	DRIVER( blockgal )	/* 834-6303 (S1) */
	DRIVER( blockgalb )	/* bootleg */
	DRIVER( tokisens )	/* (c) 1987 (from a bootleg board) (S2) */
	DRIVER( wbml )		/* bootleg (S2) */
	DRIVER( wbmljo )	/* (c) 1987 Sega/Westone (S2) */
	DRIVER( wbmljb )	/* (c) 1987 Sega/Westone (S2) */
	DRIVER( wbmlb )		/* bootleg? (S2) */
	DRIVER( wbmlbg )	/* bootleg (Galaxy Electronics) */
	DRIVER( dakkochn )	/* 836-6483? (S2) */
	DRIVER( ufosensi )	/* 834-6659 (S2) */
	DRIVER( ufosensib )	/* bootleg */
/*
other System 1 / System 2 games:

WarBall
DokiDoki Penguin Land *not confirmed
*/

	/* Sega System E games (Master System hardware) */
	DRIVER( hangonjr )	/* (c) 1985 */
	DRIVER( transfrm )	/* 834-5803 (c) 1986 */
	DRIVER( astrofl )	/* 834-5803 (c) 1986 */
	DRIVER( ridleofp )	/* (c) 1986 Sega / Nasco */
	DRIVER( fantzn2 )	/* (c) 1988 */
	DRIVER( opaopa )
	DRIVER( tetrisse )	/* (c) 1988 */

	/* other Sega 8-bit games */
	DRIVER( turbo )		/* (c) 1981 Sega */
	DRIVER( turboa )	/* (c) 1981 Sega */
	DRIVER( turbob )	/* (c) 1981 Sega */
	DRIVER( subroc3d )	/* (c) 1982 Sega */
	DRIVER( buckrog )	/* (c) 1982 Sega */
	DRIVER( buckrogn )	/* (c) 1982 Sega */
	DRIVER( zoom909 )	/* (c) 1982 Sega */
	DRIVER( kopunch )	/* 834-0103 (c) 1981 Sega */
	DRIVER( suprloco )	/* (c) 1982 Sega */
	DRIVER( suprlocoa )	/* (c) 1982 Sega */
	DRIVER( dotrikun )	/* cabinet test board */
	DRIVER( dotrikun2 )	/* cabinet test board */
	DRIVER( spcpostn )	/* (c) 1986 Sega / Nasco" */
	DRIVER( angelkds )	/* 833-6599 (c) 1988 Sega / Nasco? */
	DRIVER( calorie )	/* (c) 1986 Sega */
	DRIVER( calorieb )	/* (c) 1986 Sega (but bootleg) */

	/* Bingo-based games */
	DRIVER( bingoc )	/* (c) 1989 Sega */
	/* Bingo Fantasy  *//* (c) 19?? Sega */
	/* Bingo Star     *//* (c) 19?? Sega */
	/* Bingo Party    *//* (c) 1993 Sega */
	/* BP Phoenix     *//* (c) 1998 Sega */
	/* BP Special     *//* (c) 2000 Sega */
	/* BP Multicards  *//* (c) 2002 Sega */
	/* BP Splash      *//* (c) 2002 Sega */
	/* Bingo Planet   *//* (c) 1997 Sega */

	/* Enduro Racer / Space Harrier / Hang On */
	DRIVER( enduror )	/* (c) 1985 (FD1089B, decrypted) */
	DRIVER( enduror1 )	/* (c) 1985 (FD1089B, decrypted) */
	DRIVER( endurob2 )	/* (c) 1985 (Beta bootleg) */
	DRIVER( endurobl )	/* (c) 1985 (Herb bootleg) */
	DRIVER( hangon )	/* (c) 1985 (Rev A)*/
	DRIVER( hangon1 )	/* (c) 1985 */
	DRIVER( sharrier )	/* (c) 1985 */
	DRIVER( sharrier1 )	/* (c) 1985 */
	DRIVER( shangonro )	/* (c) 1987 (FD1094, decrypted) */
	DRIVER( shangonrb )	/* (c) 1992 (but bootleg, hangon hw?) */

	/* Pre-16 */
	DRIVER( aliensynjo )	/* (c) 1987 (FD1089A, decrypted) */
	DRIVER( aliensyn2 )	/* (c) 1987 (FD1089A, decrypted) */
	DRIVER( aliensyn5 )	/* (c) 1987 (FD1089B) */
	DRIVER( bodyslam )	/* (c) 1986 (8751) */
	DRIVER( dumpmtmt )	/* (c) 1986 (Japan) */
	DRIVER( mjleague )	/* (c) 1985 */
	DRIVER( quartet )	/* (c) 1986 (8751) */
	DRIVER( quarteta )	/* (c) 1986 (8751) */
	DRIVER( quartet2 )	/* (c) 1986 (8751) */
	DRIVER( quartet2a )	/* (c) 1986 */

	/* System16A */
	DRIVER( aceattaca )	/* (c) 1988 (FD1094, decrypted) */
	DRIVER( afighter )	/* (c) 1986 (FD1089A, decrypted) */
	DRIVER( alexkidd )	/* (c) 1986 (FD1089A, decrypted) */
	DRIVER( alexkidd1 )	/* (c) 1986 */
	DRIVER( fantzone )	/* (c) 1986 (Unprotected Rev A) */
	DRIVER( fantzone1 )	/* (c) 1986 (Unprotected) */
	DRIVER( fantzonep )	/* (c) 1986 (NEC 0317-5000, encrypted)*/
	DRIVER( sdi )		/* (c) 1987 (FD1089B, decrypted) */
	DRIVER( shinobi )	/* (c) 1987 */
	DRIVER( shinobi1 )	/* (c) 1987 (FD1094, decrypted) */
	DRIVER( shinobls )	/* (c) 1987 (Star bootleg) */
	DRIVER( sjryuko1 )	/* (c) 1988 White Board (FD1089B, decrypted) */
	DRIVER( tetris )	/* (c) 1988 (FD1094, decrypted) S16A */
	DRIVER( tetris3 )	/* (c) 1988 (FD1094, decrypted) S16A */
	DRIVER( timescan1 )	/* (c) 1987 (FD1089B, decrypted) */
	DRIVER( wb31 )		/* (c) 1988 Sega / Westone (FD1094, decrypted) */
	DRIVER( wb35 )		/* (c) 1988 Sega / Westone (FD1089A, decrypted) */
	DRIVER( wb35a )		/* (c) 1988 Sega / Westone (FD1089A, decrypted) */
	DRIVER( passsht16a )/* (c) 1988 */

	/* System 16B */
	DRIVER( aceattac )	/* (c) 1988 (protected) */
	DRIVER( aliensyn )	/* (c) 1987 */
	DRIVER( aliensyn3 )	/* (c) 1987 (FD1089A, decrypted) */
	DRIVER( aliensynj )	/* (c) 1987 (FD1089A, decrypted) */
	DRIVER( altbeast )	/* (c) 1988 (8751) */
	DRIVER( altbeastj )	/* (c) 1988 (8751) */
	DRIVER( altbeast5 )	/* (c) 1988 (8751) */
	DRIVER( altbeast4 )	/* (c) 1988 (MC-8123B) */
	DRIVER( altbeast2 )	/* (c) 1988 (MC-8123B) */
	DRIVER( altbeastj1 )	/* (c) 1988 (protected) */
	DRIVER( altbeastj2 )	/* (c) 1988 (protected) */
	DRIVER( altbeastj3 )	/* (c) 1988 (FD1094, decrypted) */
	DRIVER( aurail )	/* (c) 1990 Sega / Westone */
	DRIVER( aurail1 )	/* (c) 1990 Sega / Westone (FD1089B, decrypted) */
	DRIVER( aurailj )	/* (C) 1990 Sega / Westone (FD1089A, decrypted) */
	DRIVER( bayroute )	/* (c) 1989 Sunsoft / Sega (FD1094, decrypted) */
	DRIVER( bayroutej )	/* (c) 1989 Sunsoft / Sega (FD1094, decrypted) */
	DRIVER( bayroute1 )	/* (c) 1989 Sunsoft / Sega */
	DRIVER( bullet )	/* (c) 1987 (protected) */
	DRIVER( cotton )	/* (c) 1990 (FD1094, decrypted) */
	DRIVER( cottonu )	/* (c) 1990 (FD1094, decrypted) */
	DRIVER( cottonj )	/* (c) 1990 (FD1094, decrypted) */
	DRIVER( ddux )		/* (c) 1989 (FD1094, decrypted) */
	DRIVER( ddux1 )		/* (c) 1989 (8751) */
	DRIVER( dunkshot )	/* (c) 1986 (FD1094, decrypted) */
	DRIVER( eswat )		/* (c) 1989 (FD1094, decrypted) */
	DRIVER( eswatu )	/* (c) 1989 (FD1094, decrypted) */
	DRIVER( eswatj )	/* (c) 1989 (FD1094, decrypted) */
	DRIVER( exctleag )	/* (protected) */
	DRIVER( fpoint )	/* (c) 1989 (Japan, FD1094, decrypted) */
	DRIVER( fpoint1 )	/* (c) 1989 (Japan, FD1094, decrypted) */
	DRIVER( goldnaxe )	/* (c) 1989 (8751) */
	DRIVER( goldnaxeu )	/* (c) 1989 (protected) */
	DRIVER( goldnaxej )	/* (c) 1989 (FD1094, decrypted) */
	DRIVER( goldnaxe3 )	/* (c) 1989 (FD1094, decrypted) */
	DRIVER( goldnaxe2 )	/* (c) 1989 (8751) */
	DRIVER( goldnaxe1 )	/* (c) 1989 (FD1094, decrypted) */
	DRIVER( hwchamp )	/* (c) 1987 */
	DRIVER( hwchampj )	/* (c) 1987 (FD1094, decrypted) */
	DRIVER( mvp )		/* (c) 1989 (FD1094, decrypted) */
	DRIVER( mvpj )		/* (c) 1989 (FD1094, decrypted) */
	DRIVER( passsht )	/* (c) 1988 (FD1094, decrypted) */
	DRIVER( passshta )	/* (c) 1988 (FD1094, decrypted) */
	DRIVER( passshtj )	/* (c) 1988 (FD1094, decrypted) */
	DRIVER( riotcity )	/* (c) 1991 Sega / Westone */
	DRIVER( ryukyu )	/* (c) 1990 (FD1094, decrypted) */
	DRIVER( sdib )		/* (c) 1987 (FD1089A, decrypted) */
	DRIVER( sdibl )		/* (c) 1987 bootleg */
	DRIVER( defense )	/* (c) 1987 (FD1094, decrypted) */
	DRIVER( shinobi5 )	/* (c) 1987 */
	DRIVER( shinobi4 )	/* (c) 1987 (MC-8123B) */
	DRIVER( shinobi3 )	/* (c) 1987 (MC-8123B) */
	DRIVER( shinobi2 )	/* (c) 1987 (FD1094, decrypted) */
	DRIVER( sonicbom )	/* (c) 1987 (FD1094, decrypted) */
	DRIVER( sjryuko )	/* (c) 1988 White Board (FD1094, decrypted) */
	DRIVER( suprleag )	/* (c) 1987 (protected) */
	DRIVER( tetris2 )	/* (c) 1988 (FD1094, decrypted) S16B */
	DRIVER( tetris1 )	/* (c) 1988 (FD1094, decrypted) S16B */
	DRIVER( timescan )	/* (c) 1987 */
	DRIVER( toryumon )	/* (c) 1995 */
	DRIVER( tturf )		/* (c) 1989 Sega / Sunsoft (8751) */
	DRIVER( tturfu )	/* (c) 1989 Sega / Sunsoft (8751) */
	DRIVER( wb3 )		/* (c) 1988 Sega / Westone (8751) */
	DRIVER( wb34 )		/* (c) 1988 Sega / Westone (FD1094, decrypted) */
	DRIVER( wb33 )		/* (c) 1988 Sega / Westone (FD1094, decrypted) */
	DRIVER( wb32 )		/* (c) 1988 Sega / Westone (FD1094, decrypted) */
	DRIVER( wrestwar )	/* (c) 1989 (8751) */
	DRIVER( wrestwar2 )	/* (c) 1989 (FD1094, decrypted) */
	DRIVER( wrestwar1 )	/* (c) 1989 (FD1094, decrypted) */

	/* Philko Boards */
	DRIVER( atomicp )	/* (c) 1990 Philko - korean clone board */
	DRIVER( snapper )	/* (c) 1990 Philko - korean clone board */


	/* S16 Bootlegs */
	DRIVER( bayrouteb1 )/* (c) 1989 (protected) (bootleg) */
	DRIVER( bayrouteb2 )/* (c) 1989 (protected) (bootleg) */
	DRIVER( dduxbl )	/* (c) 1989 (Datsu bootleg) */
	DRIVER( eswatbl )	/* BOOTLEG */
	DRIVER( fpointbl )	/* (c) 1989 (Datsu bootleg) */
	DRIVER( fpointbj )	/* (c) 1989 (Datsu bootleg, Japan) */
	DRIVER( goldnaxeb1 )/* BOOTLEG  */
	DRIVER( goldnaxeb2 )/* BOOTLEG  */
	DRIVER( passht4b )	/* bootleg */
	DRIVER( passshtb )	/* bootleg */
	DRIVER( shinoblb )	/* (c) 1987 (but bootleg) */
	DRIVER( shinobld )	/* (c) 1987 (but bootleg) */
	DRIVER( tetrisbl )	/* (c) 1988 (but bootleg) */
	DRIVER( beautyb )	/* (c) 1991 AMT */
	DRIVER( iqpipe )	/* (c) 1991 AMT */
	DRIVER( tturfbl )	/* (c) 1989 (Datsu bootleg) */
	DRIVER( wb3bbl )	/* BOOTLEG */

	/* System 18 */
	DRIVER( shdancer )	/* 1989.05 Shadow Dancer (US) */
	DRIVER( shdancerj )	/* 1989.11 Shadow Dancer (Japan) */
	DRIVER( shdancer1 )	/* 1989.?? Shadow Dancer (World) */
	DRIVER( bloxeed )	/* 1989.12 Bloxeed (Japan, FD1094, decrypted) */
						/* 1990.04 Bloxeed (US) */
	DRIVER( astormu )	/* 1990.04 Alien Storm (US, FD1094, decrypted) */
	DRIVER( astormj )	/* 1990.05 Alien Storm (Japan, FD1094, decrypted) */
	DRIVER( astorm )	/* 1990.?? Alien Storm (World, protected 2 Players) */
	DRIVER( astorm3 )	/* 1990.?? Alien Storm (World, FD1094, decrypted) */
	DRIVER( mwalku )	/* 1990.08 Micheal Jackson's Moonwalker (US, FD1094+8751, decrypted) */
	DRIVER( mwalkj )	/* 1990.08 Micheal Jackson's Moonwalker (Japan, FD1094+8751, decrypted) */
	DRIVER( mwalk )		/* 1990.?? Micheal Jackson's Moonwalker (World, FD1094+8751, decrypted) */
	DRIVER( lghost )	/* 1990.?? Laser Ghost (World, FD1094, decrypted) */
						/* 1990.12 Laser Ghost (Japan) */
	DRIVER( lghostu )	/* 1991.01 Laser Ghost (US, FD1094, decrypted) */
	DRIVER( cltchitr )	/* 1991.02 Clutch Hitter (US, FD1094, decrypted) */
	DRIVER( cltchitrj )	/* 1991.05 Clutch Hitter (Japan, FD1094, decrypted) */
	DRIVER( ddcrewj )	/* 1991.07 D.D.Crew (Japan, FD1094, decrypted) */
	DRIVER( ddcrewu )	/* 1991.09 D.D.Crew (US, FD1094, decrypted) */
	DRIVER( ddcrew )	/* 1991.?? D.D.Crew (World, FD1094, decrypted) */
	DRIVER( ddcrew2 )	/* 1991.?? D.D.Crew (World, FD1094, decrypted) */
	DRIVER( ddcrew1 )	/* 1991.?? D.D.Crew (World, FD1094, decrypted) */
	DRIVER( wwallyj )	/* 1992.06 Wally wo Sagase! (Japan, FD1094, decrypted) */
	DRIVER( wwallyja )	/* 1992.06 Wally wo Sagase! (Japan, FD1094, decrypted) */
	DRIVER( desertbr )	/* 1993.?? Desert Breaker (World, FD1094, decrypted) */
	DRIVER( desertbrj )	/* 1992.10 Desert Breaker (Japan) */
	DRIVER( pontoon )

	/* System18 Bootlegs */
	DRIVER( astormbl )	/* BOOTLEG */
	DRIVER( astormb2 )	/* BOOTLEG */
	DRIVER( shdancbl )	/* BOOTLEG */
	DRIVER( mwalkbl )	/* BOOTLEG */


	/* Outrun Hardware */
						/* 1986.09 Outrun (Japan) */
						/* 1986.11 Outrun (US) */
						/* 1986.?? Outrun (World) */
	DRIVER( outrun )	/* Revision B */
	DRIVER( outrunra )	/* Revision A */
	DRIVER( outruno )	/* First version */
	DRIVER( outrundx )	/* Deluxe version */
	DRIVER( outrunb )	/* 1986 (protected beta bootleg) */
						/* 1987.04 Super Hang-On (Japan) */
						/* 1987.06 Super Hang-On (US) */
						/* 1987.?? Super Hang-On (World) */
	DRIVER( shangon )	/* 1987 (protected) */
	DRIVER( shangon3 )	/* 1987 (FD1089B, decrypted) */
	DRIVER( shangon2 )	/* 1987 (FD1089B, decrypted) */
	DRIVER( shangon1 )	/* 1987 (FD1089B, decrypted) */
	DRIVER( shangonle )	/* 1991 */
						/* 1989.02 Turbo Outrun (Japan) */
						/* 1989.04 Turbo Outrun (US) */
						/* 1989.?? Turbo Outrun (World) */
	DRIVER( toutrun )	/* 1987 (FD1094, decrypted) */
	DRIVER( toutrun3 )	/* 1987 (FD1094, decrypted) */
	DRIVER( toutrun2 )	/* 1987 (protected) */
	DRIVER( toutrun1 )	/* 1987 (FD1094, decrypted) */

	/* X Board */
	DRIVER( aburner )	/* 1987.07 After Burner (Japan) */
	DRIVER( aburner2 )	/* 1987.09 After Burner II (Japan) */
	DRIVER( thndrbld1 )	/* 1987.12 Thunder Blade (Japan, deluxe/standard) */
	DRIVER( thndrbld )	/* 1988.02 Thunder Blade (US?, upright, FD1094, decrypted) */
						/* 1989.01 Last Survivor (Japan) */
	DRIVER( smgpj )		/* 1989.06 Super Monaco GP (Japan, FD1094, decrypted) */
	DRIVER( smgpja )	/* 1989.06 Super Monaco GP (Japan, FD1094, decrypted) */
	DRIVER( smgp )		/* 1989.?? Super Monaco GP (World, FD1094, decrypted) */
	DRIVER( smgp6 )		/* 1989.?? Super Monaco GP (World, FD1094, decrypted) */
	DRIVER( smgp5 )		/* 1989.?? Super Monaco GP (World, FD1094, decrypted) */
	DRIVER( smgpu )		/* 1989.08 Super Monaco GP (US, FD1094, decrypted) */
	DRIVER( smgpu3 )	/* 1989.08 Super Monaco GP (US, FD1094, decrypted) */
	DRIVER( smgpu2 )	/* 1989.08 Super Monaco GP (US, FD1094, decrypted) */
	DRIVER( smgpu1 )	/* 1989.08 Super Monaco GP (US, FD1094, decrypted) */
	DRIVER( loffirej )	/* 1989.12 Line of Fire (Japan, FD1094, decrypted) */
	DRIVER( loffire )	/* 1989.?? Line of Fire (World, FD1094, decrypted) */
	DRIVER( loffireu )	/* 1990.04 Line of Fire (US, FD1094, decrypted) */
						/* 1990.03 Racing Hero (Japan) */
	DRIVER( rachero )	/* 1990.?? Racing Hero (World, FD1094, decrypted) */
	DRIVER( abcop )		/* 1990.?? A.B.Cop (World, FD1094, decrypted) */
						/* 1991.01 A.B.Cop (Japan) */
						/* 1990.10 GP Rider (Japan) */
	DRIVER( gprider )	/* 1990.?? GP Rider (World, protected) */
	DRIVER( gprider1 )	/* 1990.12 GP Rider (US, FD1094, decrypted) */

	/* Y-Board */
						/* 1988.05 Galaxy Force (Japan) */
						/* 1988.08 Galaxy Force (US) */
	DRIVER( gforce2j )	/* 1988.07 Galaxy Force II (Japan) */
	DRIVER( gforce2ja )	/* 1988.07 Galaxy Force II (Japan) */
	DRIVER( gforce2 )	/* 1988.?? Galaxy Force II (World) */
						/* 1988.12 Galaxy Force II (US) */
						/* 1988.08 Power Drift (US) */
	DRIVER( pdriftj )	/* 1988.09 Power Drift (Japan) */
	DRIVER( pdrift )	/* 1988.?? Power Drift (World) */
	DRIVER( pdrifta )
	DRIVER( pdrifte )
	DRIVER( gloc )		/* 1990.04 G-LOC Air Battle (US) */
						/* 1990.05 G-LOC Air Battle (Japan) */
	DRIVER( glocr360 )	/* 1990.?? G-LOC (R360, World?) */
	DRIVER( strkfgtr )	/* 1991.06 Strike Fighter (Japan) */
	DRIVER( rchase )	/* 1991.09 Rail Chase (World) */
	DRIVER( rchasej )	/* 1991.09 Rail Chase (Japan) */
						/* 1991.09 Rail Chase (US) */

	/* Sega System 24 games */
	/* disk based */
	DRIVER( hotrodj )	/* 1988.03 Hot Rod (Japan) */
	DRIVER( hotrod )	/* 1988.?? Hot Rod (World) */
	DRIVER( hotroda )
						/* 1988.07 Hot Rod (US) */
	DRIVER( sspiritj )	/* 1988.10 Scramble Spirits (Japan) */
	DRIVER( sspirits )	/* 1988.?? Scramble Spirits (World) */
	DRIVER( sspirtfc )	/* 1988.?? Scramble Spirits (protected) */
						/* 1988.11 Scramble Spirits (US) */
	DRIVER( ggroundj )	/* 1988.11 Gain Ground (Japan, FD1094, decrypted) */
	DRIVER( gground )	/* 1989.?? Gain Ground (World, FD1094, decrypted) */
						/* 1989.03 Gain Ground (US) */
	DRIVER( crkdownj )	/* 1989.03 Crack Down (Japan, FD1094, decrypted) */
	DRIVER( crkdown )	/* 1989.?? Crack Down (World, FD1094, decrypted) */
	DRIVER( crkdownu )	/* 1989.06 Crack Down (US, FD1094, decrypted) */
	DRIVER( sgmastj )	/* 1989.06 Jumbo Ozaki Super Masters (Japan, FD1094, decrypted) */
	DRIVER( sgmast )	/* 1989.?? Super Masters Golf (World, protected) */
	DRIVER( sgmastc )	/* 1989.?? Jumbo Ozaki Super Masters Golf (World, FD1094, decrypted) */
						/* 1989.09 Super Masters Golf (US) */
	DRIVER( roughrac )	/* 1990.03 Rough Racer (Japan, FD1094, decrypted) */
	DRIVER( bnzabrosj )	/* 1990.06 Bonanza Bros. (Japan) */
	DRIVER( bnzabros )	/* 1990.08 Bonanza Bros. (US) */
	DRIVER( qsww )		/* 1991.02 Quiz Syukudai wo Wasuremashita! (Japan) */
	DRIVER( dcclubfd )	/* 1991.07 Dynamic Country Club (US) */

	/* ROM based */
	DRIVER( dcclubj )	/* 1991.06 Dynamic Country Club (Japan) */
	DRIVER( dcclub )	/* 1991.?? Dynamic Country Club (World) */
	DRIVER( qrouka )	/* 1991.12 Quiz Rouka ni Tattenasai! (Japan) */
	DRIVER( mahmajn )	/* 1992.12 Tokorosan no Mahmahjong (Japan) */
	DRIVER( quizmeku )	/* 1993.05 Quiz Mekurumeku Story (Japan) */
	DRIVER( mahmajn2 )	/* 1994.05 Tokorosan no Mahmahjong 2 (Japan) */
	DRIVER( qgh )		/* 1994.09 Quiz Ghost Hunter */

	/* Sega System 32 games */
	DRIVER( radm )		/* 1991.?? Rad Mobile (World) */
	DRIVER( radmu )		/* 1991.03 Rad Mobile (US) */
						/* 1991.02 Rad Mobile (Japan) */
	DRIVER( radr )		/* 1991.?? Rad Rally (World) */
	DRIVER( radru )		/* 1991.09 Rad Rally (US) */
						/* 1991.07 Rad Rally (Japan) */
	DRIVER( spidman )	/* 1991.?? Spiderman (World) */
	DRIVER( spidmanu )	/* 1991.09 Spiderman (US) */
						/* 1991.09 Spiderman (Japan) */
	DRIVER( f1en )		/* 1991.?? F-1 Exhaust Note (World) */
						/* 1992.01 F-1 Exhaust Note (US) */
						/* 1991.11 F-1 Exhaust Note (Japan) */
	DRIVER( arabfgt )	/* 1992.?? Arabian Fight (World) */
	DRIVER( arabfgtu )	/* 1992.03 Arabian Fight (US) */
	DRIVER( arabfgtj )	/* 1992.03 Arabian Fight (Japan) */
	DRIVER( arescue )	/* 1992.03 Air Rescur (US) */
						/* 1992.04 Air Rescue (Japan) */
	DRIVER( ga2 )		/* 1992.?? Golden Axe II (World) */
	DRIVER( ga2u )		/* 1992.09 Golden Axe II (US) */
	DRIVER( ga2j )		/* 1992.10 Golden Axe Death Adder no Fukusyuu (Japan) */
	DRIVER( holo )		/* 1992.?? Hologram Holosseum (US) */
						/* 1992.11 Hologram Holosseum (Japan) */
	DRIVER( darkedge )	/* 1993.?? Dark Edge (World) */
						/* 1993.?? Dark Edge (US) */
	DRIVER( darkedgej )	/* 1993.03 Dark Edge (Japan) */
	DRIVER( brival )	/* 1993.?? Burning Rival (World) */
	DRIVER( brivalj )	/* 1993.08 Burning Rival (Japan) */
	DRIVER( f1lap )		/* 1993.?? F-1 Super Lap (World) */
						/* 1993.09 F-1 Super Lap (Japan) */
	DRIVER( alien3 )	/* 1993.?? Aliens 3 (World) */
	DRIVER( alien3u )	/* 1993.11 Aliens 3 (US) */
						/* 1993.09 Aliens 3 the Gun (Japan) */
	DRIVER( sonic )		/* 1993.09 Sonic the Hedgehog (Japan) */
	DRIVER( sonicp )	/* 1993.?? Sonic the Hedgehog (proto, Japan) */
	DRIVER( kokoroj2 )	/* 1993.12 Soreike! Kokoroji 2 (Japan) */
	DRIVER( jpark )		/* 1994.?? Jurassic Park (World)  */
						/* 1994.?? Jurassic Park (US) */
						/* 1994.02 Jurassic Park (Japan) */
	DRIVER( dbzvrvs )	/* 1994.03 Dragon Ball Z V.R.V.S. (Japan) */
	DRIVER( jleague )	/* 1994.07 The J League 1994 (Japan) */
	DRIVER( svf )		/* 1994.?? Super Visual Football */
	DRIVER( svs )		/* 1994.?? Super Visual Soccer */
	DRIVER( slipstrm )	/* 1995.?? Slip Stream (Capcom) */
	DRIVER( slipstrmh )

	/* Sega Multi System 32 games */
						/* 1993.05 Outrunners (Japan) */
	DRIVER( orunners )	/* 1993.?? Outrunners (World) */
	DRIVER( orunnersu )	/* 1993.06 Outrunners (US) */
	DRIVER( harddunkj )	/* 1994.04 Hard Dunk 3on3 (Japan) */
	DRIVER( harddunk )	/* 1994.?? Hard Dunk 3on3 (World) */
						/* 1993.03 Title Fight (Japan) */
	DRIVER( titlef )	/* 1993.?? Title Fight (World) */
	DRIVER( titlefu )	/* 1993.04 Title Fight (US) */
						/* 1992.07 Stadium Cross (Japan) */
	DRIVER( scross )	/* 1992.?? Stadium Cross (World) */
	DRIVER( scrossu )	/* 1992.09 Stadium Cross (US) */

	/* Model 1 Games */
	DRIVER( vr )		/* 1992.08 Virtua Racing */
	DRIVER( vf )		/* 1993.12 Virtua Fighter */
	DRIVER( vformula )	/* 1993.?? Virtua Formula */
	DRIVER( swa )		/* 1994.04 Star Wars Arcade */
	DRIVER( wingwar )	/* 1994.06 Wing War */
	DRIVER( wingwarj )	/* 1994.06 Wing War (Japan) */
	DRIVER( wingwaru )	/* 1994.09 Wing War (US) */

	/* Sega H1 games */
	DRIVER( coolridr )	/* 1995.04 Cool Riders */

	/* Sega ST-V games */
	DRIVER( stvbios )
						/* 1994.07 Sports Fishing */
	DRIVER( gaxeduel )	/* 1995.02 Golden Axe the Duel */
	DRIVER( suikoenb )	/* 1995.03 Suiko Enbu (Data East) */
	DRIVER( bakubaku )	/* 1995.04 Bakubaku Animal */
	DRIVER( vfremix )	/* 1995.06 Virtua Fighter Remix */
	DRIVER( ejihon )	/* 1995.07 Ejihon Tantei Jimusyo */
	DRIVER( finlarch )	/* 1995.07 Final Arch */
	DRIVER( sfish2j )	/* 1995.08 Sports Fishing 2 (Japan) */
	DRIVER( shanhigw )	/* 1995.08 Shanghai Banri no Choujou */
	DRIVER( sandor )	/* 1995.11 Nido Aru Koto wa Sando-R */
	DRIVER( kiwames )	/* 1995.11 Pro Mahjong Kiwame S (Athena) */
	DRIVER( sfish2 )	/* 1995.11 Sports Fishing 2 (Export) */
	DRIVER( critcrsh )	/* 1995.?? Critter Crusher */
						/* 1995.12 Tatakott */
	DRIVER( batmanfr )	/* 1996.02 Batman Forever (Acclaim) */
	DRIVER( fhboxers )	/* 1996.02 Funky Head Boxers */
	DRIVER( vfkids )	/* 1996.03 Virtua Fighter Kids */
	DRIVER( pblbeach )	/* 1996.03 Pebble Beach the Great Shot (T&E Soft) */
	DRIVER( introdon )	/* 1996.04 Karaoke Quiz Intro Dondon (Sunsoft / Success) */
	DRIVER( smleague )	/* 1996.04 Super Major League */
	DRIVER( decathlt )	/* 1996.05 Decathlete */
	DRIVER( decathlto )
	DRIVER( dnmtdeka )	/* 1996.07 Dynamite Deka */
	DRIVER( mausuke )	/* 1996.07 Mausuke no Ojama the World (Data East) */
	DRIVER( diehard )	/* 1996.08 Die Hard Arcade */
	DRIVER( sokyugrt )	/* 1996.09 Soukyuu Gurentai (Raizing / 8ing) */
	DRIVER( prikura )	/* 1996.10 Prikura Daisakusen (Atlus) */
	DRIVER( magzun )	/* 1996.11 Magical Zunou Power */
	DRIVER( puyosun )	/* 1996.12 Puyo Puyo SUN (Compile) */
	DRIVER( colmns97 )	/* 1997.01 Columns 97 */
	DRIVER( shienryu )	/* 1997.02 Shienryu (Warashi) */
	DRIVER( vmahjong )	/* 1997.02 Virtual Mahjong (Micronet) */
	DRIVER( groovef )	/* 1997.05 Groove on Fight (Atlus) */
	DRIVER( nclubv3 )	/* 1997.07 Name Club Ver. 3 */
	DRIVER( pclub2 )	/* 1997.09 Print Club 2 */
	DRIVER( thunt )		/* 1997.09 Puzzle & Action Treasure Hunt (Sega (Deniam License)) */
	DRIVER( thuntk )
	DRIVER( winterht )	/* 1997.10 Winter Heat (Data East) */
	DRIVER( cotton2 )	/* 1997.11 Cotton 2 (Success) */
	DRIVER( hanagumi )	/* 1997.11 Sakura Taisen Hanagumi Taisen Columns */
	DRIVER( findlove )	/* 1997.12 Find Love (Daiki / FCF) */
	DRIVER( maruchan )	/* 1997.12 Maruchan de Goo!!! */
	DRIVER( techbowl )	/* 1997.12 Technical Bowling */
	DRIVER( znpwfv )	/* 1997.12 Zen Nippon Pro Wrestling featuring Virtua */
	DRIVER( seabass )	/* 1998.02 Sea Bass Fishing (A wave Inc (Able License)) */
	DRIVER( sasissu )	/* 1998.02 Taisen Tanto-R Sasissu!! */
						/* 1998.03 Bass Master Challenge */
	DRIVER( twcup98 )	/* 1998.04 Tecmo World Cup '98 (Tecmo) */
	DRIVER( grdforce )	/* 1998.05 Guardian Force (Success) */
	DRIVER( rsgun )		/* 1998.05 Radient Silver Gun (Treasure) */
	DRIVER( astrass )	/* 1998.06 Astra Super Stars (Sunsoft) */
	DRIVER( myfairld )	/* 1998.07 My Fair Lady (Micronet) */
	DRIVER( othellos )	/* 1998.07 Othello Shiyouyo (Success) */
	DRIVER( cottonbm )	/* 1998.09 Cotton Boomerang (Success) */
	DRIVER( stress )	/* 1998.10 Stress Busters */
	DRIVER( elandore )	/* 1998.11 Touryuu Densetsu Elandore (Sai-Mate) */
	DRIVER( sss )		/* 1998.12 Steep Slope Sliders (Capcom / Cave / Victor) */
	DRIVER( micrombc )	/* 1999.03 Microman Battle Charge */
	DRIVER( pclub2v3 )	/* 1999.03 Print Club 2 Vol. 3 */
	DRIVER( sanjeon )	/* 1999.04 Sanjun Sujun (Sega / Deniam) */
	DRIVER( danchih )	/* 1999.06 Danchi de Hanafuda Okusan Komeya Desuyo! (Altron (Tecmo License)) */
	DRIVER( ffreveng )	/* 1999.09 Final Fight Revenge (Capcom) */
	DRIVER( pclubpok )  /* 1999.11 Print Club Pokemon */
	DRIVER( danchiq )	/* 2000.12 Danchi de Quiz Okusan Yontaku Desuyo! (Altron) */

	/* Sega Model 2 Games */

	/* Model 2 (TGPs, Model 1 sound board) */
	DRIVER( daytona )	/* 1994.03 Daytona USA */
	DRIVER( daytona93 )
	DRIVER( daytonas )
	DRIVER( daytonat )	/* 1993, hack */
	DRIVER( daytonam )	/* 1993, hack */
	DRIVER( desert )	/* 1994.07 Desert Tank */
	DRIVER( vcop )		/* 1994.09 Virtua Cop */

	/* Model 2A-CRX (TGPs, SCSP sound board) */
	DRIVER( vf2o )		/* 1994.11 Virtua Fighter 2 */
	DRIVER( vf2a )		/* 1995.?? */
	DRIVER( vf2b )		/* 1995.?? */
	DRIVER( srallyc )	/* 1995.02 Sega Rally Championship */
	DRIVER( vcop2 )		/* 1995.09 Virtua Cop 2 */
	DRIVER( vf2 )		/* 1995.09 Virtua Fighter 2.1*/
						/* 1995.10 Sky Target */
	DRIVER( manxtt )	/* 1995.11 Manx T.T. */
						/* 1996.08 Dynamite Baseball */
	DRIVER( doaa )		/* 1996.11 Dead or Alive */
						/* 1997.04 Dynamite Baseball '97 */
	DRIVER( motoraid )	/* 1997.10 Motor Raid */
	DRIVER( zeroguna )	/* 1997.12 Zero Gunner */
	DRIVER( dyndeka2 )	/* 1998.05 Dynamite Deka 2 */
	DRIVER( dynamcop )	/* 1998.09 Dynamic Cop */
	DRIVER( pltkidsa )	/* 1999.03 Pilot Kids */

	/* Model 2B-CRX (SHARC, SCSP sound board) */
	DRIVER( vstriker )	/* 1995.05 virtua Striker */
	DRIVER( vstrikero )
	DRIVER( rchase2 )	/* 1995.06 Rail Chase 2 (Arcade TV Game List - P.104, Left, 12 from top) */
	DRIVER( indy500 )	/* 1995.07 Indy 500 */
	DRIVER( indy500d )
	DRIVER( indy500to )
	DRIVER( skytargt )	/* 1995.10 Sky Target */
	DRIVER( fvipers )	/* 1995.11 Fighting Vipers */
	DRIVER( von )		/* 1996.01 Dennou Senki Virtual-On */
	DRIVER( vonj )
	DRIVER( gunblade )	/* 1996.04 Gunblade N.Y. */
	DRIVER( schamp )	/* 1996.05 Sonic Championship */
	DRIVER( sfight )	/* 1996.05 Sonic the Fighters */
	DRIVER( lastbrnx )	/* 1996.06 Last Bronx */
	DRIVER( lastbrnxj )
	DRIVER( sgt24h )	/* 1996.07 Super GT 24h */
	DRIVER( dynabb )	/* 1996.08 Dynamite Baseball */
	DRIVER( doa )		/* 1996.11 Dead or Alive */
						/* 1997.04 Dynamite Baseball '97 */
	DRIVER( zerogun )	/* 1997.12 Zero Gunner */
	DRIVER( zerogunj )
	DRIVER( dyndeka2b )	/* 1998.05 Dynamite Deka 2 */
	DRIVER( dynamcopb )	/* 1998.09 Dynamite Cop */
	DRIVER( pltkids )	/* 1999.03 Pilot Kids */

	/* Model 2C-CRX (TGPx4, SCSP sound board) */
	DRIVER( skisuprg )	/* 1996.12 Sega Ski Super G */
	DRIVER( stcc )		/* 1996.09 Sega Touring Car Championship */
	DRIVER( waverunr )	/* 1996.10 Wave Runner */
	DRIVER( hotd )		/* 1997.03 The House of the Dead */
	DRIVER( topskatr )	/* 1997.05 Top Skater */
	DRIVER( topskatru )
	DRIVER( overrev )	/* 1997.09 Overrev */
	DRIVER( segawski )	/* 1997.09 Sega Water Ski */
	DRIVER( dynamcopc )	/* 1998.09 Dynamite Cop */
	DRIVER( bel )		/* 1998.12 Behind Enemy Lines */

	/* Sega Model 3 Games */

	/* Step 1.0 */
	DRIVER( vf3 )		/* 1996.09 Virtua Fighter 3 */
	DRIVER( vf3a )
	DRIVER( vf3tb )		/* 1997.10 Virtua Fighter 3tb */
	DRIVER( bass )		/* 1997.12 Get Bass */
	DRIVER( getbass )
						/* 1998.03 Get Bass Brast */

	/* Step 1.5 */
	DRIVER( scud )		/* 1996.12 Scud Race */
	DRIVER( scudj )
	DRIVER( scuda )
	DRIVER( vs215 )		/* 1997.06 Virtua Striker 2 */
	DRIVER( lostwsga )	/* 1997.07 Lost World Jurassic Park */
	DRIVER( scudp )		/* 1997.09 Scud Race Plus */
	DRIVER( lemans24 )	/* 1997.09 Le Mans 24 */
	DRIVER( vs29815 )	/* 1998.05 Virtua Striker 2 ver.98 */

	/* Step 2.0 */
	DRIVER( vs2 )		/* 1997.06 Virtua Striker 2 */
	DRIVER( harley )	/* 1997.12 Harley Davidson & L.A.Riders */
	DRIVER( skichamp )	/* 1998.01 Ski Champ */
	DRIVER( srally2 )	/* 1998.02 Sega Rally 2 Championship */
	DRIVER( srally2x )
	DRIVER( von2 )		/* 1998.03 Dennou Senki Virtual-On Oratorio Tangram */
	DRIVER( fvipers2 )	/* 1998.04 Fighting Vipers 2 */
	DRIVER( vs298 )		/* 1998.05 Virtua Striker 2 ver.98*/
	DRIVER( vs299 )		/* 1998.12 Virtual Striker 2 ver.99 */
	DRIVER( vs299b )	/* 1999.?? Virtual Striker 2 ver.99 (rev B) */
	DRIVER( vs299a )	/* 1999.?? Virtual Striker 2 ver.99 (rev A) */
	DRIVER( vs2v991 )	/* 1999.?? Virtual Striker 2 ver.99.1 */
	DRIVER( von254g )	/* 1999.05 Dennou Senki Virtual-On Oratorio Tangram ver.5.4 */

	/* Step 2.1 */
	DRIVER( daytona2 )	/* 1998.06 Daytona USA 2 */
	DRIVER( oceanhun )	/* 1998.09 Ocean Hunter */
	DRIVER( spikeout )	/* 1998.09 Spikeout */
	DRIVER( dirtdvls )	/* 1998.10 Dirt Devils */
	DRIVER( dirtdvlsa )
	DRIVER( swtrilgy )	/* 1998.11 Star Wars Trilogy Arcade */
	DRIVER( swtrilgya )
						/* 1998.11 Magical Torokko Adventure (Japan) */
	DRIVER( lamachin )	/* 1998.12 L.A. Machine Guns */
	DRIVER( dayto2pe )	/* 1998.12 Daytone USA 2 Power Edition */
						/* 1998.12 Virtua Striker 2 ver.99 */
	DRIVER( spikeofe )	/* 1999.01 Spikeout Final Edition */
	DRIVER( magtruck )	/* 1999.03 Magical Truck Adventure (US) */
	DRIVER( eca )		/* 1999.10 Kyukyuusya Emergency Call Ambulance */
	DRIVER( ecax )


	/* Deniam games */
	/* they run on Sega System 16 video hardware */
	DRIVER( logicpro )	/* (c) 1996 Deniam */
	DRIVER( croquis )	/* (c) 1996 Deniam (Germany) */
	DRIVER( karianx )	/* (c) 1996 Deniam */
	DRIVER( logicpr2 )	/* (c) 1997 Deniam (Japan) */


/*
Deniam is a Korean company (http://deniam.co.kr).

Game list:
Title            System     Date
---------------- ---------- ----------
GO!GO!           deniam-16b 1995/10/11
Logic Pro        deniam-16b 1996/10/20
Karian Cross     deniam-16b 1997/04/17
LOTTERY GAME     deniam-16c 1997/05/21
Logic Pro 2      deniam-16c 1997/06/20
Propose          deniam-16c 1997/06/21
BOMULEUL CHAJARA SEGA ST-V  1997/04/11
*/

	/* System C games */
	DRIVER( bloxeedc )	/* (c) 1989 Sega / Elorg */
	DRIVER( bloxeedu )	/* (c) 1989 Sega / Elorg */
	DRIVER( columns )	/* (c) 1990 Sega */
	DRIVER( columnsu )	/* (c) 1990 Sega */
	DRIVER( columnsj )	/* (c) 1990 Sega */
	DRIVER( columns2 )	/* (c) 1990 Sega */
	DRIVER( column2j )	/* (c) 1990 Sega */

	/* System C-2 games */
	DRIVER( borench )	/* (c) 1990 Sega */
	DRIVER( tfrceac )	/* (c) 1990 Sega / Technosoft */
	DRIVER( tfrceacj )	/* (c) 1990 Sega / Technosoft */
	DRIVER( tfrceacb )	/* bootleg */
	DRIVER( twinsqua )	/* (c) 1991 Sega */
	DRIVER( ribbit )	/* (c) 1991 Sega */
	DRIVER( tantr )		/* (c) 1992 Sega */
	DRIVER( tantrkor )	/* (c) 1992 Sega */
	DRIVER( tantrbl )	/* bootleg */
	DRIVER( tantrbl2 )	/* bootleg */
	DRIVER( tantrbl3 )	/* bootleg */
	DRIVER( puyo )		/* (c) 1992 Sega / Compile */
	DRIVER( puyobl )	/* bootleg */
	DRIVER( puyoj )		/* (c) 1992 Sega / Compile */
	DRIVER( puyoja )	/* (c) 1992 Sega / Compile */
	DRIVER( ichir )		/* (c) 1994 Sega */
	DRIVER( ichirk )	/* (c) 1994 Sega */
	DRIVER( ichirj )	/* (c) 1994 Sega */
	DRIVER( ichirjbl )	/* bootleg */
	DRIVER( stkclmns )	/* (c) 1994 Sega */
	DRIVER( stkclmnsj )	/* (c) 1994 Sega */
	DRIVER( puyopuy2 )	/* (c) 1994 Compile + Sega license */
	DRIVER( potopoto )	/* (c) 1994 Sega */
	DRIVER( zunkyou )	/* (c) 1994 Sega */

	/* Atlus Print Club 'Games' C-2 board */
	DRIVER( pclubj )
	DRIVER( pclubjv2 )
	DRIVER( pclubjv4 )
	DRIVER( pclubjv5 )

	/* Genie Hardware (uses Genesis VDP) also has 'Sun Mixing Co' put into tile ram */
	DRIVER( puckpkmn )	/* (c) 2000 Genie */
	DRIVER( topshoot )	/* (c) 1995 Sun Mixing */

	/* Sega MegaTech, the number shown is on the label of the instruction rom */
	DRIVER( megatech )
	DRIVER( mt_beast )	/* 01 */
	DRIVER( mt_shar2 )	/* 02 */
	DRIVER( mt_stbld )	/* 03 */
	DRIVER( mt_ggolf )	/* 04 */
	DRIVER( mt_gsocr )	/* 05 */
	DRIVER( mt_orun )	/* 06 */
	DRIVER( mt_asyn )	/* 07 */
	DRIVER( mt_shnbi )	/* 08 */
						/* 09 */
	DRIVER( mt_aftrb )	/* 10 */
	DRIVER( mt_tfor2 )	/* 11 */
						/* 12 */
	DRIVER( mt_astro )	/* 13 */
						/* 14 */
						/* 15 */
						/* 16 */
						/* 17 */
						/* 18 */
	DRIVER( mt_gfoot )	/* 19 */
	DRIVER( mt_lastb )	/* 20 */
	DRIVER( mt_wcsoc )	/* 21 */
	DRIVER( mt_tetri )	/* 22 */
	DRIVER( mt_gng )	/* 23 */
	DRIVER( mt_shang )	/* 24 */
	DRIVER( mt_gaxe )	/* 25 */
	DRIVER( mt_fwrld )	/* 26 */
	DRIVER( mt_mystd )	/* 27 */
	DRIVER( mt_revsh )	/* 28 */
	DRIVER( mt_parlg )	/* 29 */
						/* 30 */
	DRIVER( mt_tgolf )	/* 31 */
	DRIVER( mt_srbb )	/* 32 */
						/* 33 */
						/* 34 */
	DRIVER( mt_tlbba )	/* 35 */
	DRIVER( mt_cols )	/* 36 */
						/* 37 */
	DRIVER( mt_eswat )	/* 38 */
	DRIVER( mt_smgp )	/* 39 */
	DRIVER( mt_mwalk )	/* 40 */
	DRIVER( mt_crack )	/* 41 */
						/* 42 */
	DRIVER( mt_shado )	/* 43 */
	DRIVER( mt_arrow )	/* 44 */
						/* 45 */
						/* 46 */
	DRIVER( mt_astrm )	/* 47 */
	DRIVER( mt_wwar )	/* 48 */
	DRIVER( mt_bbros )	/* 49 */
						/* 50 */
	DRIVER( mt_srage )	/* 51 */
	DRIVER( mt_sonic )	/* 52 */
	DRIVER( mt_sonia )	/* ^^ */
	DRIVER( mt_fshrk )	/* 53 */
	DRIVER( mt_spman )	/* 54 */
						/* 55 */
						/* 56 */
	DRIVER( mt_gaxe2 )	/* 57 */
	DRIVER( mt_stf )	/* 58 */
	DRIVER( mt_mlh )	/* 59 */
	DRIVER( mt_kcham )	/* 60 */
	DRIVER( mt_tout )	/* 61 */
	DRIVER( mt_soni2 )	/* 62 */
	//DRIVER( mt_comp1 )    /* ?? */
	//DRIVER( mt_comp2 )    /* ?? */

	/* Sega MegaPlay */
	DRIVER( megaplay )
	DRIVER( mp_sonic )	/* 01 */
	DRIVER( mp_gaxe2 )	/* 02 */
	DRIVER( mp_gslam )	/* 03 */
	DRIVER( mp_twc )	/* 04 */
	DRIVER( mp_sor2 )	/* 05 */
	DRIVER( mp_bio )	/* 06 */
	DRIVER( mp_soni2 )	/* 07 */
						/* 08 */
	DRIVER( mp_shnb3 )	/* 09 */
						/* 10 */
	DRIVER( mp_mazin )	/* 11 */
	DRIVER( mp_col3 )	/* ?? */

	DRIVER( ssf2ghw )	/* MegaDrive-based hack */
	DRIVER( aladbl )	/* MegaDrive-based hack */
	DRIVER( mk3ghw )	/* MegaDrive-based hack */
	DRIVER( ssgbl )		/* MegaDrive-based hack */

	/* Sega Naomi-based (Dreamcast) systems */
	DRIVER( naomi )
	DRIVER( naomigd )
	DRIVER( naomi2 )

	DRIVER( hod2bios )	/* 1998.11 The House of the Dead 2 */
	DRIVER( hotd2 )		/* 1998.11 The House of the Dead 2 */
	DRIVER( hotd2o )    	/* 1998.11 The House of the Dead 2 */
	DRIVER( dybbnao )	/* 1998.12 Dynamite Baseball NAOMI */
	DRIVER( smlg99 )        /* 1999.?? Super Major League '99 */
	DRIVER( pstone )	/* 1999.02 Power Stone */
	DRIVER( crzytaxi )	/* 1999.02 Crazy Taxi */
	DRIVER( zombrvn )	/* 1999.02 Zombie Revenge */
	DRIVER( airlbios )	/* 1999.03 Airline Pilots */
	DRIVER( alpiltdx )
	DRIVER( alpilota )
	DRIVER( ggram2 )	/* 1999.04 Giant Gram Zen Nippon Pro-wres 2 */
						/* 1999.05 Taisen Puzzle Kurutto Stone */
						/* 1999.06 Ringout 4x4 */
	DRIVER( f355bios )	/* 1999.07 F355 Challenge */
	DRIVER( f355 )
	DRIVER( f355twin )	/* 1999.07 F355 Challenge Twin */
	DRIVER( shangril )	/* 1999.08 Dengen Tenshi Taisen Janshi Shangri-la */
						/* 1999.08 Unou Nouryoku Check Machine Touch de UNO! */
	DRIVER( vs2_2k )	/* 1999.08 Virtua Striker 2 version 2000 */
	DRIVER( suchie3 )	/* 1999.09 Idol Janshi Su-Chi-Pi 3 */
	DRIVER( toyfight )	/* 1999.09 Toy Fighter */
	DRIVER( jambo )		/* 1999.09 Jambo! Safari */
						/* 1999.09 Super Major League 99 */
	DRIVER( sgtetris )	/* 1999.10 Sega Tetris */
	DRIVER( derbyoc )	/* 1999.10 Derby Owners Club */
	DRIVER( doa2 )		/* 1999.11 Dead or Alive 2 */
	DRIVER( otrigger )	/* 1999.11 Out Trigger */
	DRIVER( dybb99 )	/* 1999.11 Dynamite Baseball '99 */
						/* 1999.12 Mars TV */
	DRIVER( samba )		/* 1999.12 Samba de Amigo */
	DRIVER( vtennis )	/* 1999.12 Virtua Tennis / Power Smash (ROM) */
	DRIVER( vtennisg )	/* 1999.12 Virtua Tennis / Power Smash (GD-ROM) */
						/* 1999.12 Puyopuyo DA! */
	DRIVER( 18wheelr )	/* 2000.01 18 Wheeler */
	DRIVER( totd )		/* 2000.01 The Typing of the Dead */
	DRIVER( spawn )		/* 2000.01 Spawn */
	DRIVER( doa2m )		/* 2000.01 Dead or Alive 2 Millenium */
						/* 2000.02 Touch de UNO! 2 */
	DRIVER( virnba )	/* 2000.02 Virtua NBA */
	DRIVER( virnbao )
	DRIVER( mvsc2 )		/* 2000.03 Marvel vs. Capcom 2 */
	DRIVER( smarinef )	/* 2000.03 Sega Marine Fishing */
						/* 2000.03 World Kicks */
						/* 2000.04 Shin Nippon Pro-wres Toukon Retsuden 4 */
	DRIVER( pstone2 )	/* 2000.04 Power Stone 2 */
	DRIVER( qmegamis )	/* 2000.05 Quiz Ah Megamisama */
						/* 2000.06 Virtual-on Oratorio Tangram M.S.B.S. ver.5.66 2000 edition */
						/* 2000.06 Derby Owners Club 2000 */
	DRIVER( ggx )		/* 2000.07 Guilty Gear X */
	DRIVER( slasho )	/* 2000.07 Slashout */
	DRIVER( slashout )
	DRIVER( capsnka )	/* 2000.08 Capcom vs. SNK Millenium Fight 2000 */
						/* 2000.08 Tokyo Bus Annai */
	DRIVER( capsnk )	/* 2000.09 Capcom vs. SNK Millenium Fight 2000 */
	DRIVER( gram2000 )	/* 2000.09 Giant Gram 2000 Zen Nippon Pro-wres 3 */
						/* 2000.09 Nittere Shiki! Mirai Yosou Studio */
	DRIVER( crackndj )	/* 2000.10 Crackin' DJ */
	DRIVER( cspike )	/* 2000.10 Gun Spike */
	DRIVER( sstrkfgt )	/* 2000.10 Sega Strike Fighter */
	DRIVER( deathcox )	/* 2000.10 Death Crimson OX */
						/* 2000.11 Ninja Assault */
	DRIVER( confmiss )	/* 2000.11 Confidential Mission */											/* First GD-ROM game? */
						/* 2000.11 Shakatto Tambourine */
	DRIVER( wwfroyal )	/* 2000.11 WWF Royal Rumble */
	DRIVER( pjustic )	/* 2000.12 Moero Justice Gakuen */
						/* 2000.12 Samba de Amigo ver.2000 */
	DRIVER( sprtjam )	/* 2000.12 Sports Jam */
						/* 2000.12 Derby Owners Club 2000 ver.2 */
						/* 2000.?? World Kicks New ver. */
	DRIVER( derbyocw )	/* 2001.?? Derby Owners Club World Edition (Rev. C) */
	DRIVER( f355twn2 )	/* 2001.01 F355 Challenge 2 */
	DRIVER( gwing2 )	/* 2001.01 Giga Wing 2 */
	DRIVER( csmash )	/* 2001.01 Cosmic Smash */
	DRIVER( csmasho )
	DRIVER( sfz3ugd )	/* 2001.02 Street Fighter ZERO3 Upper */
	DRIVER( gundmgd )	/* 2001.03 Kidou Senshi Gundam Renpou vs. Zeon */
						/* 2001.04 Shakatto Tambourine Motto Norinori Shinkyoku Tsuika */
	DRIVER( dygolf )	/* 2001.04 Dynamic Golf */
						/* 2001.05 Zero Gunner 2 */
	DRIVER( monkeyba )	/* 2001.05 Monkey Ball */
	DRIVER( cvsgd )		/* 2001.06 Capcom vs. SNK Pro */
						/* 2001.06 Crackin' DJ Part2 */
	DRIVER( wsbbgd )	/* 2001.06 Super Major League */
	DRIVER( spkrbtl )	/* 2001.07 Spikers Battle */
	DRIVER( gunsur2 )	/* 2001.07 Gun Survivor 2: Bio Hazard Code Veronica */
	DRIVER( starseek )	/* 2001.07 Dokidoki Idol Star Seeker */
	DRIVER( cvs2gd )	/* 2001.08 Capcom vs. SNK 2 */
	DRIVER( gundmxgd )	/* 2001.09 Kidou Senshi Gundam Renpou vs. Zeon DX */
						/* 2001.09 Alien Front */
	DRIVER( hmgeo )		/* 2001.09 Heavy Metal Geo Matrix */
						/* 2001.10 Inu no Osanpo */
	DRIVER( derbyoc2 )	/* 2001.10 Derby Owners Club 2 */
	DRIVER( vtennis2 )	/* 2001.10 Power Smash 2 / Virtua Tennis 2 */
	DRIVER( vtenis2c )	/* Power Smash 2 / Virtua Tennis 2 (cartridge) */
						/* 2001.11 Shakatto Tambourine Cho Powerup Chu */
	DRIVER( keyboard )	/* 2001.11 La Keyboardxyu */
	DRIVER( ikaruga )	/* 2001.12 Ikaruga */
	DRIVER( lupinsho )	/* 2001.12 Lupin the Third: the Shooting */
	DRIVER( vathlete )	/* 2002.03 Virtua Athlete */
	DRIVER( luptype )	/* 2002.04 Lupin the Third: the Typing */
	DRIVER( ggxx )		/* 2002.05 Guilty Gear XX */
	DRIVER( mok )		/* 2002.05 The Maze of the Kings */
	DRIVER( cleoftp )	/* 2002.05 Cleopatra Fortune + */
	DRIVER( chocomk )	/* 2002.06 Musapey no Choco Marker */
						/* 2002.07 Mazan */
						/* 2002.09 Yonin Uchi Mahjong MJ */
	DRIVER( azumanga )	/* 2002.12 Azumanga Daiou Puzzle Bobble */
	DRIVER( moeru )		/* 2002.12 Moeru Casinyo */
	DRIVER( quizqgd )	/* 2003.01 Quiz Keitai Q mode */
						/* 2003.03 Yonin Uchi Mahjong MJ Network Taisen ver. */
	DRIVER( ggxxrl )	/* 2003.03 Guilty Gear XX # Reload */
	DRIVER( shikgam2 )	/* 2003.04 Shikigami no Shiro 2 */
	DRIVER( bdrdown )	/* 2003.04 Border Down */
	DRIVER( usagui )	/* 2003.06 Usagi Yamashiro Mahjong Hen */
						/* 2003.07 Yonin Uchi Mahjong MJ Network Taisen ver. NEW */
	DRIVER( psyvar2 )	/* 2003.11 Psyvariar 2 */
	DRIVER( puyofev )	/* 2003.11 Puyopuyo Fever */
	DRIVER( shootopl )	/* Shootout Pool */
	DRIVER( shootpl )	/* 2003.xx Shootout Pool / Shootout Pool Prize */
	DRIVER( kick4csh )	/* 2004.xx Kick '4' Cash */
	DRIVER( tetkiwam )	/* 2004.06 Tetris Kiwamemichi (Arcade TV Game List - P.88, Right, 11 from bottom) */
	DRIVER( cfield )	/* 2004.06 Chaos Field */
	DRIVER( trizeal )	/* 2004.09 Trizeal */
						/* 2005.03 Melty Blood Act Cadenza */
	DRIVER( ss2005 )	/* 2005.03 Super Shanghai 2005 */
	DRIVER( ss2005a )	/* 2005.04 Super Shanghai 2005 */
	DRIVER( senkoo )	/* 2005.04 Senkou no Rondo */
	DRIVER( senko )		/* 2005.08 Senkou no Rondo NEW ver. */
	DRIVER( meltybld )	/* 2005.08 Melty Blood Act Cadenza ver.A */
	DRIVER( ggxxsla  )	/* 2005.09 Guilty Gear XX Slash */
	DRIVER( radirgy )	/* 2005.10 Radirgy */
	DRIVER( undefeat )	/* 2005.10 Under Defeat */
						/* 2005.12 Radirgy (Rev. A) */
	DRIVER( kurucham )	/* 2006.03 Kurukuru Chameleon */
	DRIVER( trgheart )	/* 2006.05 Trigger Heart Exelica */
						/* 2006.07 Touch de Zunou */
	DRIVER( senkosp )	/* 2006.08 Senkou no Rondo SP */
	DRIVER( jingystm )	/* 2006.09 Jingi Storm */
	DRIVER( karous )	/* 2006.11 Karous */
	DRIVER( ggxxac  )	/* 2006.11 Guilty Gear XX Accent Core */
	DRIVER( meltyb )	/* 2006.12 Melty Blood Act Cadenza ver.B */
	DRIVER( meltyba )	/* 2006.12 Melty Blood Act Cadenza ver.B (Rev A) */
	DRIVER( takoron )	/* 2006.12.16 Noukone Puzzle Takoron */
						/* 2007.03 Melty Blood Act Cadenza ver.B Syuuseiban(ver.B2) */

	DRIVER( ngdup23a )	/* DIMM Firmware Updater */
	DRIVER( ngdup23c )

						/* Releases below use flash/mask ROMs like older NAOMI titles, not GD-ROM as GD-ROM Hardware (Drives and Discs) were no longer being produced */

						/* 2007.07 Pokasuka Ghost! */
						/* 2007.07 Shooting Love */
						/* 2007.07 Dynamite Deka EX - Asian dynamite */
						/* 2007.09 Rhythm Tengoku */
						/* 2008.02 Akatsuki Denkou Senki Ausf.Achse */
						/* 2008.06 Illvelo (Illmatic Envelope) */
						/* 2008.07 Mamoru Kun wa Norowarete Shimatta! */
						/* 2008.09 Melty Blood Actress Again */
						/* 2008.12 Melty Blood Actress Again ver.A */
						/* 2009.06 Radirgy Noah */
						/* 2009.?? Project Cerberus (planned to be released in 2009) */



	/* NAOMI based (System SP) */
						/* 2003.02 Kouchuu Ouja Mushiking */
						/* 2004.10 Oshare Majo Love & Berry */
						/* 2005.09 Kodai Ouja Kyouryuu King */
						/* 2007.06 Issyo ni Wanwan */
						/* 2007.06 Konchuu Dash!! */
						/* 2007.07 Marine Marine */
						/* 2007.11 Magical Dance on Dream Stage */
						/* 2008.02 Marine Marine ver.2 */
						/* 2008.04 Issyo ni Wanwan Waiwai Puppy */
						/* 2008.07 Issyo ni Turbo Drive */

	/* NAOMI 2 */
	DRIVER( vstrik3c )	/* 2001.04 Virtua Striker 3 */
	DRIVER( vstrik3 )
	DRIVER( wldrider )	/* 2001.05 Wild Riders */
						/* 2001.06 Club Kart */
	DRIVER( beachspi )	/* 2001.07 Beach Spikers */
						/* 2001.08 Wave Runner GP */
	DRIVER( vf4cart )	/* 2001.08.02 Virtua Fighter 4 (cartridge) */
	DRIVER( vf4 )		/* 2001.08.02 Virtua Fighter 4 (GD-ROM) */
	DRIVER( vf4b )		/* 2001.08.20 Virtua Fighter 4 ver.B */
	DRIVER( vf4c )		/* 2002.01.31 Virtua Fighter 4 ver.C */
						/* 2002.02 The King of Route 66 */
	DRIVER( initd )		/* 2002.03 Initial D Arcade Stage */
	DRIVER( initdexp )	/* 200?.?? Initial D Export Version */
	DRIVER( clubkrte )	/* 2002.?? Club Kart: European Session */
						/* 2002.06 WCCF SERIE A 2001-2002 */
						/* 2002.07 Soul Surfer */
	DRIVER( vf4evoct )	/* 2002.08 Virtua Fighter 4 Evolution (cartridge) */
	DRIVER( vf4evoa )	/* 2002.08 Virtua Fighter 4 Evolution */
						/* 2002.11 WCCF SERIE A 2001-2002 ver.1.2 */
	DRIVER( initdv2j )	/* 2002.12 Initial D Arcade Stage ver.2 */
	DRIVER( vf4evo )	/* 2002.12 Virtua Fighter 4 Evolution ver.B */
						/* 2003.03 WCCF SERIE A 2001-2002 ver.2.0 */
						/* 2003.09 WCCF 2002-2003 SERIE A */
						/* 2003.?? CYCRAFT Club Kart ver. */
	DRIVER( initdv3j )	/* 2004.01 Initial D Arcade Stage ver.3 */
						/* 2004.?? Initial D Arcade Stage ver.3 for CYCRAFT */
						/* 2004.06 WCCF 2002-2003 SERIE A ver.2.0 */
						/* 2004.07 Virtua Fighter 4 Final Tuned */
	DRIVER( vf4tuneda )	/* 2004.09 Virtua Fighter 4 Final Tuned ver.A */
						/* 2004.12 WCCF 2002-2003 ver.2.1 */
						/* 2005.01 Virtua Fighter 4 final Tuned ver.B */
						/* 2005.04 Menkyo no Tetsujin */
						/* 2005.06 WCCF European Clubs 2004-2005 */
						/* 2006.01 WCCF European Clubs 2004-2005 ver.1.1 */
						/* 2006.03 WCCF European Clubs 2004-2005 ver.1.2 */
						/* 2006.08 WCCF European Clubs 2005-2006 */
						/* 2006.10 WCCF European Clubs 2005-2006 bugfix ver. */
	DRIVER( vf4tunedd )
	DRIVER( vf4tuned )	/* 200?.?? Virtual fighter 4 Final Tuned ver.F */


	/* Atomiswave (Dreamcast-based) games */
	DRIVER( awbios )
						/* 2002.12 Maximum Speed */
	DRIVER( sprtshot )	/* 2002.12 Sports Shooting USA */
						/* 2003.03 Guilty Gear X ver.1.5 */
	DRIVER( demofist )	/* 2003.06 Demolish Fist */
	DRIVER( dolphin  )	/* 2003.08 Dolphin Blue */
	DRIVER( ggisuka )	/* 2003.12 Guilty Gear Isuka */
						/* 2003.12 Sangoku Senki Shichisei Tensei (IGS) */
	DRIVER( rumblef )	/* 2004.03 The Rumble Fish */
	DRIVER( rangrmsn )	/* 2004.03 Ranger Mission */
	DRIVER( salmankt )	/* 2004.06 Salary Man Kintarou Net Select */
	DRIVER( kofnw )		/* 2004.07 The King of Fighters Neo Wave */
	DRIVER( kofnwj )    	/* 2004.07 The King of Fighters Neo Wave (Japan) */
	DRIVER( xtrmhunt )	/* 2004.09 Extreme Hunting */
						/* 2004.09 Dirty Pig Skin Football */
						/* 2004.09 Faster than Speed */
	DRIVER( kov7sprt )	/* 2004.?? Knights of Valour - The Seven Sprits */
	DRIVER( vfurlong )	/* 2005.03 Net Select Keiba Victory Furlong */
						/* 2005.03 The Rumble Fish 2 */
	DRIVER( ngbc )		/* 2005.07 NeoGeo Battle Colloseum */
						/* 2005.09 Samurai Spirits Tenkaichi Kenkakuden */
						/* 2005.10 The King of Fighters XI */
	DRIVER( fotns )		/* 2005.12 Hokuto no Ken */
						/* 2006.02 Metal Slug 6 */
	DRIVER( xtrmhnt2 )			/* 2006.09 Extreme Hunting 2 Tournament Edition */
						/* 2008.?? Sega Clay Challenge */

	/* Hikaru */
	DRIVER( hikaru )	/* BIOS (c)2000 */
						/* 1999.06 Syouboushi Brave Fire Fighters */
	DRIVER( podrace )	/* 2000.06 Star Wars: Racer Arcade */
						/* 2000.09 Nascar Arcade */
	DRIVER( pharrier )	/* 2000.12 Planet Harriers */
	DRIVER( airtrix )	/* 2001.02 Air Trix */
						/* 2001.10 Dennou Senki Virtual-On Force */
						/* 2002.04 Dennou Senki Virtual-On Force ver.7.7 */

	/* Triforce (GameCube based) */
	DRIVER( triforce )
	DRIVER( vs2002j )	/* 2002.10 Virtua Striker 2002 */
	DRIVER( vs2002ex )
						/* 2003.06 F-ZERO AX */
	DRIVER( avalons )	/* 2003.07 Avalon no Kagi */
						/* 2003.08 Avalon no Kagi ver.1.00 */
						/* 2003.10 Avalon no Kagi ver.1.01 */
	DRIVER( gekpurya )	/* 2003.10 Gekitou Pro Yakyuu Mizushima Shinji All Stars vs. Pro Yakyuu */
						/* 2003.12 Avalon no Kagi ver.1.10 */
						/* 2003.?? CYCRAFT F-ZERO AX Monster Ride */
						/* 2004.04 Avalon no Kagi ver.1.20 Aratanaru Syoukan */
	DRIVER( avalon13 )	/* 2004.07 Avalon no Kagi ver.1.30 Konton no Utage */
	DRIVER( vs4 )		/* 2004.10 Virtua Striker 4 */
	DRIVER( vs4j )		/* 2004.10 Virtua Striker 4 */
	DRIVER( avalon20 )	/* 2004.11 Avalon no Kagi ver.2.0 Chitsujo to Kairitsu */
						/* 2005.04 Virtua Striker 4 ver.A */
						/* 2005.08 Avalon no Kagi ver.2.5 Kagi Seisen */
						/* 2005.12 Mario Kart Arcade Grand Prix */
	DRIVER( vs42006 )	/* 2006.05 Virtua Striker 4 ver.2006 */
						/* 2007.03 Mario Kart Arcade Grand Prix 2 */

	DRIVER( tfupdate )	/* Triforce GDROM Boot Update (GDT-0011) */

	/* Chihiro (X-Box based) */
	DRIVER( chihiro )
	DRIVER( hotd3 )		/* 2002.12 The House of the Dead III */
						/* 2003.03 Crazy Taxi High Roller */
	DRIVER( vcop3 )		/* 2003.04 Virtua Cop 3 */
	DRIVER( outr2 )		/* 2003.12 Outrun 2 */
						/* 2004.03 Sega Network Taisen Mahjong MJ 2 */
	DRIVER( ollie )		/* 2004.03 Ollie King */
						/* 2004.04 Sega Network Taisen Mahjong MJ 2 ver.B */
	DRIVER( wangmid )	/* 2004.07 Wangan Midnight Maximum Tune */
	DRIVER( mj2 )		/* 2004.07 Sega Network Taisen Mahjong MJ 2 ver.C */
						/* 2004.09 Quest of D */
						/* 2004.11 Sega Golf Club Network Pro Tour */
						/* 2004.12 Sega Network Taisen Mahjong MJ 2 ver.D */
						/* 2004.12 Quest of D ver.1.02 */
						/* 2004.12 Outrun 2 Special Tours */
						/* 2004.12 Quest of D ver.1.10 */
	DRIVER( ghostsqu )	/* 2004.12 Ghost Squad */
						/* 2004.12 Quest of D ver.1.10a */
						/* 2005.03 Sangokushi Taisen */
	DRIVER( gundamos )	/* 2005.03 Kidou Senshi Gundam Battle Operating Simulator */
						/* 2005.04 Quest of D ver.1.20 */
						/* 2005.04 Sega Network Taisen Mahjong MJ 2 ver.E */
	DRIVER( wangmid2 )	/* 2005.04 Wangan Midnight Maximum Tune 2 */
						/* 2005.06 Sangokushi Taisen ver.1.003 */
						/* 2005.06 Sega Golf Club Network Pro Tour 2005 */
						/* 2005.06 Quest of D ver.1.20a */
						/* 2005.09 Quest of D ver.1.21 */
						/* 2005.10 Sangokushi Taisen Ranse no Gunrou ver.1.100 */
						/* 2005.10 Wangan Midnight Maximum Tune 2 ver.B */
						/* 2005.11 Sangokushi Taisen Ranse no Gunrou ver.1.110 */
						/* 2005.12 Quest of D Gofu no Keisyousya ver.2.00 */
						/* 2005.12 Kidou Senshi Gundam 0079 Card Builder */
	DRIVER( mj3 )		/* 2005.12 Sega Network Taisen Mahjong MJ 3 */
						/* 2005.12 Quest of D Gofu no Keisyousya ver.2.01 */
	DRIVER( scg06nt )	/* 2005.12 Sega Golf Club 2006 Next Tours */
						/* 2006.02 Sangokushi Taisen Ranse no Gunrou ver.1.12 */
						/* 2006.03 Kidou Senshi Gundam 0079 Card Builder ver.1.007 */
						/* 2006.03 Quest of D Gofu no Keisyousya ver.2.02b */
						/* 2006.04 Sega Network Taisen Mahjong MJ 3 ver.A+ */
						/* 2006.05 Sangokushi Taisen 2 */
						/* 2006.07 Kidou Senshi Gundam 0079 Card Builder ver.2.00 */
						/* 2006.09 Sangokushi Taisen 2 ver.2.01 */
						/* 2006.09 Sega Network Taisen Mahjong MJ 3 ver.B */
						/* 2006.09 Kidou Senshi Gundam 0079 Card Builder ver.2.01 */
						/* 2006.10 Quest of D ver.3.00 Oukoku no Syugosya */
						/* 2006.11 Quest of D ver.3.01 Oukoku no Syugosya */
						/* 2006.11 Kidou Senshi Gundam 0079 Card Builder ver.2.02 */
						/* 2007.03 Kidou Senshi Gundam 0083 Card Builder */
						/* 2007.03 Sega Network Taisen Mahjong MJ 3 Evolution */
						/* 2007.10 Kidou Senshi Gundam 0083 Card Builder Ryouyuu Gekitotsu */
						/* 2007.11 Quest of D The Battle Kingdom ver.4.00 */
						/* 2008.01 Quest of D The Battle Kingdom ver.4.00b */
						/* 2008.02 Quest of D The Battle Kingdom ver.4.00c */
						/* 2008.06 Quest of D The Battle Kingdom ver.4.01 */

	DRIVER( hshavoc )	/* (c) 1993 Data East */

	/* Data East "Burger Time hardware" games */
	DRIVER( lnc )		/* (c) 1981 */
	DRIVER( zoar )		/* (c) 1982 */
	DRIVER( btime )		/* (c) 1982 */
	DRIVER( btime2 )	/* (c) 1982 */
	DRIVER( btimem )	/* (c) 1982 + Midway */
	DRIVER( tisland )	/* (c) 1982 */
	DRIVER( protennb )	/* bootleg */
	DRIVER( cookrace )	/* bootleg */
	DRIVER( wtennis )	/* bootleg 1982 */
	DRIVER( brubber )	/* (c) 1982 */
	DRIVER( bnj )		/* (c) 1982 + Midway */
	DRIVER( caractn )	/* bootleg */
	DRIVER( disco )		/* (c) 1982 */
	DRIVER( discof )	/* (c) 1982 */
	DRIVER( sdtennis )	/* (c) 1983 */
	DRIVER( mmonkey )	/* (c) 1982 Technos Japan + Roller Tron */

		/* cassette system, parent is decocass */
	DRIVER( decocass )
	DRIVER( ctsttape )	/* ? */
	DRIVER( chwy )		/* 01 1980.12 Highway Chase */
						/* 02 1980.12 Sengoku Ninjatai */
						/* 03 1981.01 Manhattan */
	DRIVER( cterrani )	/* 04 1981.02 Terranian */
						/* 05 1981.?? Missile Sprinter */
						/* 06 1980.12 Nebbler */
	DRIVER( castfant )	/* 07 1981.02 Astro Fantasia */
						/* 08 1981.03 The Tower */
	DRIVER( csuperas )	/* 09 1981.05 Super Astro Fighter */
						/* 10 1981.?? Ocean to Ocean (medal) */
	DRIVER( clocknch )	/* 11 1981.04 Rock'n Chase */
						/* 12 1981.08 Flash Boy/DECO Kid */
	DRIVER( cprogolf )	/* 13 1981.08 Pro Golf */
						/* 14 1981.06 DS Telejan */
	DRIVER( cluckypo )	/* 15 1981.?? Lucky Poker */
	DRIVER( ctisland )	/* 16 1982.02 Treasure Island */
	DRIVER( ctisland2 )	/* 16 */
	DRIVER( ctisland3 )	/* 16 */
						/* 17 1981.10 Bobbit */
	DRIVER( cexplore )	/* 18 1982.02 Explorer */
	DRIVER( cdiscon1 )	/* 19 1982.04 Disco No.1 */
	DRIVER( csweetht )	/* 19 */
	DRIVER( ctornado )	/* 20 1982.05 Tornado */
	DRIVER( cmissnx )	/* 21 1982.04 Mission X/Zoar */
	DRIVER( cptennis )	/* 22 1982.06 Pro Tennis */
						/* 23 1982.?? 18 Hole Pro Golf */
						/* 24 1982.07 Tsumego Kaisyou */
						/* 25 1982.10 Fishing */
	DRIVER( cbtime )	/* 26 1982.08 Hamburger/Burger Time */
	DRIVER( cburnrub )	/* 27 1982.11 Burnin' Rubber/Bump'n Jump */
	DRIVER( cburnrub2 )	/* 27 */
	DRIVER( cbnj )		/* 27 */
	DRIVER( cgraplop )	/* 28 1982.11 Grapelop */
	DRIVER( cgraplop2 )	/* 28 */
	DRIVER( clapapa )	/* 29 1982.11 Lapapa/Rootin' Tootin' */
	DRIVER( clapapa2 )	/* 29 */ /* this one doesn't display lapapa anyehere */
						/* 30 1983.03 Skater */
	DRIVER( cprobowl )	/* 31 1983.03 Pro Bowling */
	DRIVER( cnightst )	/* 32 1983.04 Night Star */
	DRIVER( cnightst2 )	/* 32 */
	DRIVER( cprosocc )	/* 33 1982.08 Pro Soccer */
						/* 34 1983.09 Super Doubles Tennis */
	DRIVER( cflyball )	/* 35 1983.?? Bampoline/Flying Ball */
						/* 36 1984.04 Genesis/Boomer Rang'r */
	DRIVER( czeroize )	/* 37 1983.10 Zeroize */ /* doesn't work at all */
	DRIVER( cscrtry )	/* 38 1984.03 Scrum Try */
	DRIVER( cscrtry2 )	/* 38 */
	DRIVER( cppicf )	/* 39 1984.02 Peter Pepper's Ice Cream Factory */
	DRIVER( cppicf2 )	/* 39 */
	DRIVER( cfghtice )	/* 40 1984.04 Fighting Ice Hockey */
						/* 41 1984.05 Oozumou */
						/* 42 1984.08 Hellow Gateball */ /* not a typo, this is official spelling */
						/* 43 1984.08 Yellow Cab */
	DRIVER( cbdash )	/* 44 1985.08 Boulder Dash */
						/* UX7 1984.12 Tokyo Mie Shinryoujo */
						/* UX8 1985.01 Tokyo Mie Shinryoujo Part 2 */
						/* UX9 1985.05 Geinoujin Shikaku Shiken */

	/* Data East LD games */
	DRIVER( begas )		/* (c) 1983 */
	DRIVER( begas1 )	/* (c) 1983 */
	DRIVER( cobra )		/* (c) 1984 */
	DRIVER( rblaster )	/* (c) 1985 */

	/* other Data East games */
	DRIVER( madalien )	/* (c) 1980 */
	DRIVER( madaliena )	/* (c) 1980 */
	DRIVER( astrof )	/* (c) [1980?] */
	DRIVER( astrof2 )	/* (c) [1980?] */
	DRIVER( astrof3 )	/* (c) [1980?] */
	DRIVER( abattle )	/* Sidam */
	DRIVER( abattle2 )	/* Sidam */
	DRIVER( afire )		/* Rene Pierre */
	DRIVER( acombat )	/* bootleg */
	DRIVER( acombato )	/* bootleg */
	DRIVER( sstarbtl )	/* bootleg */
	DRIVER( spfghmk2 )	/* (c) [1979] Data East Corporation */
	DRIVER( spfghmk22 )	/* (c) [1979] Data East Corporation */
	DRIVER( tomahawk )	/* (c) 1980 Data East, Inc. */
	DRIVER( tomahawk1 )	/* (c) 1980 Data East, Inc. */
	DRIVER( progolf )	/* (c) 1981 */
	DRIVER( progolfa )	/* (c) 1981 */
	DRIVER( deshoros )	/* (c) 1983 */
	DRIVER( prosoccr )	/* (c) 1983 */
	DRIVER( prosport )	/* (c) 1983 */
	DRIVER( prosporta )	/* (c) 1983 */
	DRIVER( boomrang )	/* (c) 1983 */
	DRIVER( boomranga )	/* (c) 1983 */
	DRIVER( kamikcab )	/* (c) 1984 */
	DRIVER( yellowcbj )	/* (c) 1984 */
	DRIVER( yellowcbb )	/* bootleg */
	DRIVER( liberate )	/* (c) 1984  Data East Corporation */
	DRIVER( dualaslt )	/* (c) 1984  Data East USA (US) */
	DRIVER( liberateb )	/* bootleg */
	DRIVER( bwings )	/* (c) 1984 */
	DRIVER( bwingso )	/* (c) 1984 */
	DRIVER( bwingsa )	/* (c) 1984 */
	DRIVER( zaviga )	/* (c) */
	DRIVER( zavigaj )	/* (c) */
	DRIVER( kchamp )	/* (c) 1984 Data East USA (US) */
	DRIVER( karatedo )	/* (c) 1984 Data East Corporation (Japan) */
	DRIVER( kchampvs )	/* (c) 1984 Data East USA (US) */
	DRIVER( kchampvs2 ) /* (c) 1984 Data East USA (US) */
	DRIVER( karatevs )	/* (c) 1984 Data East Corporation (Japan) */
	DRIVER( compgolf )	/* (c) 1985 Data East Corporation (Japan) */
	DRIVER( compgolfo )	/* (c) 1985 Data East Corporation (Japan) */
	DRIVER( cntsteer )	/* (c) 1985 Data East Corporation */
	DRIVER( zerotrgt )	/* (c) 1985 Data East Corporation */
	DRIVER( gekitsui )	/* (c) 1985 Data East Corporation (Japan) */
	DRIVER( tryout )	/* (c) 1985 Data East Corporation (Japan) */
	DRIVER( firetrap )	/* (c) 1986 Data East USA (US) */
	DRIVER( firetrapj )	/* (c) 1986 Wood Place Inc. (Japan) */
	DRIVER( firetrapbl )	/* bootleg */
	DRIVER( metlclsh )	/* (c) 1985 Data East */
	DRIVER( chanbara )	/* (c) 1985 Data East */
	DRIVER( brkthru )	/* (c) 1986 Data East USA (US) */
	DRIVER( brkthruj )	/* (c) 1986 Data East Corporation (Japan) */
	DRIVER( forcebrk )	/* (c) 1986 Data East Corporation (Japan) */
	DRIVER( darwin )	/* (c) 1986 Data East Corporation (Japan) */
	DRIVER( shootout )	/* (c) 1985 Data East USA (US) */
	DRIVER( shootoutj )	/* (c) 1985 Data East USA (Japan) */
	DRIVER( shootoutb )	/* bootleg */
	DRIVER( sidepckt )	/* (c) 1986 Data East Corporation */
	DRIVER( sidepcktj )	/* (c) 1986 Data East Corporation */
	DRIVER( sidepcktb )	/* bootleg */
	DRIVER( exprraid )	/* (c) 1986 Data East USA (US) */
	DRIVER( exprraida )	/* (c) 1986 Data East USA (US) */
	DRIVER( wexpress )	/* (c) 1986 Data East Corporation (World?) */
	DRIVER( wexpressb )	/* bootleg */
	DRIVER( wexpressb2 )	/* bootleg */
	DRIVER( pcktgal )	/* (c) 1987 Data East Corporation (Japan) */
	DRIVER( pcktgalb )	/* bootleg */
	DRIVER( pcktgal2 )	/* (c) 1989 Data East Corporation (World?) */
	DRIVER( pcktgal2j )	/* (c) 1989 Data East Corporation (World?) */
	DRIVER( pokechmp )	/* Korean hack of Pocket Gal */
	DRIVER( spool3 )	/* (c) 1989 Data East Corporation (World?) */
	DRIVER( spool3i )	/* (c) 1990 Data East Corporation + I-Vics license */
	DRIVER( battlera )	/* (c) 1988 Data East Corporation (World) */
	DRIVER( bldwolf )	/* (c) 1988 Data East USA (US) */
	DRIVER( bldwolfj )	/* (c) 1988 Data East Corporation (Japan) */
	DRIVER( actfancr )	/* (c) 1989 Data East Corporation (World) */
	DRIVER( actfancr1 )	/* (c) 1989 Data East Corporation (World) */
	DRIVER( actfancrj )	/* (c) 1989 Data East Corporation (Japan) */
	DRIVER( triothep )	/* (c) 1989 Data East Corporation (World) */
	DRIVER( triothepj )	/* (c) 1989 Data East Corporation (Japan) */

	/* Data East 8-bit games */
	DRIVER( lastmisn )	/* (c) 1986 Data East USA (US) */
	DRIVER( lastmisno )	/* (c) 1986 Data East USA (US) */
	DRIVER( lastmisnj )	/* (c) 1986 Data East Corporation (Japan) */
	DRIVER( shackled )	/* (c) 1986 Data East USA (US) */
	DRIVER( breywood )	/* (c) 1986 Data East Corporation (Japan) */
	DRIVER( csilver )	/* (c) 1987 Data East Corporation (Japan) */
	DRIVER( csilverj )	/* (c) 1987 Data East Corporation (Japan) */
	DRIVER( ghostb )	/* (c) 1987 Data East USA (US) */
	DRIVER( ghostb3 )	/* (c) 1987 Data East USA (US) */
	DRIVER( meikyuh )	/* (c) 1987 Data East Corporation (Japan) */
	DRIVER( meikyuha )	/* (c) 1987 Data East Corporation (Japan) */
	DRIVER( srdarwin )	/* (c) 1987 Data East Corporation (World) */
	DRIVER( srdarwinj )	/* (c) 1987 Data East Corporation (Japan) */
	DRIVER( gondo )		/* (c) 1987 Data East USA (US) */
	DRIVER( makyosen )	/* (c) 1987 Data East Corporation (Japan) */
	DRIVER( garyoret )	/* (c) 1987 Data East Corporation (Japan) */
	DRIVER( cobracom )	/* (c) 1988 Data East Corporation (World) */
	DRIVER( cobracomj )	/* (c) 1988 Data East Corporation (Japan) */
	DRIVER( oscar )		/* (c) 1988 Data East Corporation (World) */
	DRIVER( oscaru )	/* (c) 1987 Data East USA (US) */
	DRIVER( oscarj1 )	/* (c) 1987 Data East Corporation (Japan) */
	DRIVER( oscarj2 )	/* (c) 1987 Data East Corporation (Japan) */

	/* Data East 16-bit games */
	DRIVER( karnov )	/* (c) 1987 Data East USA (US) */
	DRIVER( karnovj )	/* (c) 1987 Data East Corporation (Japan) */
	DRIVER( wndrplnt )	/* (c) 1987 Data East Corporation (Japan) */
	DRIVER( chelnov )	/* (c) 1988 Data East USA (World) */
	DRIVER( chelnovu )	/* (c) 1988 Data East USA (US) */
	DRIVER( chelnovj )	/* (c) 1988 Data East Corporation (Japan) */
		/* the following ones all run on similar hardware */
	DRIVER( hbarrel )	/* (c) 1987 Data East USA (US) */
	DRIVER( hbarrelw )	/* (c) 1987 Data East Corporation (World) */
	DRIVER( baddudes )	/* EI (c) 1988 Data East USA (US) */
	DRIVER( drgninja )	/* EG (c) 1988 Data East Corporation (Japan) */
	DRIVER( birdtry )	/* (c) 1988 Data East Corporation (Japan) */
	DRIVER( robocop )	/* EP? (c) 1988 Data East Corporation (World) */
	DRIVER( robocopw )	/* EP? (c) 1988 Data East Corporation (World) */
	DRIVER( robocopj )	/* EM (c) 1988 Data East Corporation (Japan) */
	DRIVER( robocopu )	/* (c) 1988 Data East USA (US) */
	DRIVER( robocopu0 )	/* (c) 1988 Data East USA (US) */
	DRIVER( robocopb )	/* bootleg */
	DRIVER( automat )	/* bootleg */
	DRIVER( hippodrm )	/* EW (c) 1989 Data East USA (US) */
	DRIVER( ffantasy )	/* EV (c) 1989 Data East Corporation (Japan) */
	DRIVER( ffantasya )	/* EV (c) 1989 Data East Corporation (Japan) */
	DRIVER( ffantasybl )/* bootleg */
	DRIVER( slyspy )	/* (c) 1989 Data East USA (US) */
	DRIVER( slyspy2 )	/* (c) 1989 Data East USA (US) */
	DRIVER( secretag )	/* (c) 1989 Data East Corporation (World) */
	DRIVER( secretab )	/* bootleg */
	DRIVER( midres )	/* (c) 1989 Data East Corporation (World) */
	DRIVER( midresu )	/* (c) 1989 Data East USA (US) */
	DRIVER( midresj )	/* (c) 1989 Data East Corporation (Japan) */
	DRIVER( midresb )	/* (c) 1989 Data East Corporation (Japan) */
	DRIVER( bouldash )	/* (c) 1990 Data East Corporation (World) */
	DRIVER( bouldashj )	/* (c) 1990 Data East Corporation (Japan) */
		/* end of similar hardware */
	DRIVER( stadhero )	/* (c) 1988 Data East Corporation (Japan) */
	DRIVER( madmotor )	/* (c) [1989] Mitchell */
		/* All these games have a unique code stamped on the mask roms */
	DRIVER( vaportra )	/* MAA (c) 1989 Data East Corporation (World) */
	DRIVER( vaportra3 )	/* MAA (c) 1989 Data East Corporation (World) */
	DRIVER( vaportrau )	/* MAA (c) 1989 Data East Corporation (US) */
	DRIVER( kuhga )		/* MAA (c) 1989 Data East Corporation (Japan) */
	DRIVER( cbuster )	/* MAB (c) 1990 Data East Corporation (World) */
	DRIVER( cbusterw )	/* MAB (c) 1990 Data East Corporation (World) */
	DRIVER( cbusterj )	/* MAB (c) 1990 Data East Corporation (Japan) */
	DRIVER( twocrude )	/* MAB (c) 1990 Data East USA (US) */
	DRIVER( darkseal )	/* MAC (c) 1990 Data East Corporation (World) */
	DRIVER( darkseal1 )	/* MAC (c) 1990 Data East Corporation (World) */
	DRIVER( darksealj )	/* MAC (c) 1990 Data East Corporation (Japan) */
	DRIVER( gatedoom )	/* MAC (c) 1990 Data East Corporation (US) */
	DRIVER( gatedoom1 )	/* MAC (c) 1990 Data East Corporation (US) */
	DRIVER( edrandy )	/* MAD (c) 1990 Data East Corporation (World) */
	DRIVER( edrandy2 )	/* MAD (c) 1990 Data East Corporation (World) */
	DRIVER( edrandy1 )	/* MAD (c) 1990 Data East Corporation (World) */
	DRIVER( edrandyj )	/* MAD (c) 1990 Data East Corporation (Japan) */
	DRIVER( supbtime )	/* MAE (c) 1990 Data East Corporation (World) */
	DRIVER( supbtimea )	/* MAE (c) 1990 Data East Corporation (World) */
	DRIVER( supbtimej )	/* MAE (c) 1990 Data East Corporation (Japan) */
	DRIVER( mutantf )	/* MAF (c) 1992 Data East Corporation (World) */
	DRIVER( mutantf4 )	/* MAF (c) 1992 Data East Corporation (World) */
	DRIVER( mutantf3 )	/* MAF (c) 1992 Data East Corporation (World) */
	DRIVER( deathbrd )	/* MAF (c) 1992 Data East Corporation (Japan) */
	DRIVER( cninja )	/* MAG (c) 1991 Data East Corporation (World) */
	DRIVER( cninja1 )	/* MAG (c) 1991 Data East Corporation (World) */
	DRIVER( cninjau )	/* MAG (c) 1991 Data East Corporation (US) */
	DRIVER( joemac )	/* MAG (c) 1991 Data East Corporation (Japan) */
	DRIVER( cninjabl )	/* bootleg */
	DRIVER( stoneage )	/* bootleg */
	DRIVER( robocop2 )	/* MAH (c) 1991 Data East Corporation (World) */
	DRIVER( robocop2u )	/* MAH (c) 1991 Data East Corporation (US) */
	DRIVER( robocop2j )	/* MAH (c) 1991 Data East Corporation (Japan) */
	DRIVER( thndzone )	/* MAJ (c) 1991 Data East Corporation (World) */
	DRIVER( dassault )	/* MAJ (c) 1991 Data East Corporation (US) */
	DRIVER( dassault4 )	/* MAJ (c) 1991 Data East Corporation (US) */
	DRIVER( chinatwn )	/* MAK (c) 1991 Data East Corporation (Japan) */
	DRIVER( rohga )		/* MAM (c) 1991 Data East Corporation (Asia/Euro) */
	DRIVER( rohga1 )	/* MAM (c) 1991 Data East Corporation (Asia/Euro) */
	DRIVER( rohga2 )	/* MAM (c) 1991 Data East Corporation (Asia/Euro) */
	DRIVER( rohgah )	/* MAM (c) 1991 Data East Corporation (Hong Kong) */
	DRIVER( rohgau )	/* MAM (c) 1991 Data East Corporation (US) */
	DRIVER( wolffang )	/* MAM (c) 1991 Data East Corporation (Japan) */
	DRIVER( schmeisr )	/*     (c) 1993 Hot B */
	DRIVER( captaven )	/* MAN (c) 1991 Data East Corporation (Asia) */
	DRIVER( captavena )	/* MAN (c) 1991 Data East Corporation (Asia) */
	DRIVER( captavene )	/* MAN (c) 1991 Data East Corporation (UK) */
	DRIVER( captavenu )	/* MAN (c) 1991 Data East Corporation (US) */
	DRIVER( captavenuu )	/* MAN (c) 1991 Data East Corporation (US) */
	DRIVER( captavenua )	/* MAN (c) 1991 Data East Corporation (US) */
	DRIVER( captavenj )	/* MAN (c) 1991 Data East Corporation (Japan) */
						/* MAO ?? */
	DRIVER( tumblep )	/* MAP (c) 1991 Data East Corporation (World) */
	DRIVER( tumblepj )	/* MAP (c) 1991 Data East Corporation (Japan) */
	DRIVER( tumbleb )	/* bootleg */
	DRIVER( tumbleb2 )	/* bootleg */
	DRIVER( jumpkids )	/* (c) 1993 Comad */
	DRIVER( jumppop )	/* (c) 2001 ESD */
	DRIVER( pangpang )	/* (c) 1994 Dong Gue La Mi Ltd. */
	DRIVER( sdfight )	/* (c) 1996 SemiCom */
	DRIVER( wlstar )	/* (c) 1995 Mijin */
	DRIVER( wondl96 )	/* (c) 1996 SemiCom */
	DRIVER( fncywld )	/* (c) 1996 Unico */
	DRIVER( htchctch )	/* (c) 1995 SemiCom */
	DRIVER( cookbib )	/* (c) 1995 SemiCom */
	DRIVER( chokchok )	/* (c) 1995 SemiCom */
	DRIVER( metlsavr )	/* (c) 1994 */
	DRIVER( bcstry )	/* (c) 1997 SemiCom */
	DRIVER( bcstrya )	/* (c) 1997 SemiCom */
	DRIVER( semibase )	/* (c) 1997 SemiCom */
	DRIVER( dquizgo )	/* (c) 1998 SemiCom */
	DRIVER( suprtrio )	/* (c) 1994 GameAce */
	DRIVER( lemmings )	/* prototype (c) 1991 Data East USA (US) */
						/* MAQ ?? */
	DRIVER( dragngun )	/* MAR (c) 1992 Data East Corporation (US) */
	DRIVER( wizdfire )	/* MAS (c) 1992 Data East Corporation (Over Sea) */
	DRIVER( wizdfireu )	/* MAS (c) 1992 Data East Corporation (US) */
	DRIVER( darkseal2 )	/* MAS (c) 1992 Data East Corporation (Japan) */
	DRIVER( funkyjet )	/* MAT (c) 1992 Mitchell */
	DRIVER( funkyjetj )	/* MAT (c) 1992 Mitchell */
						/* MAU ?? */
	DRIVER( nitrobal )	/* MAV (c) 1992 Data East Corporation (US) */
	DRIVER( gunball )	/* MAV (c) 1992 Data East Corporation (Japan) */
						/* MAW ?? */
						/* MAX ?? */
	DRIVER( dietgo )	/* MAY (c) 1993 */
	DRIVER( dietgoe )	/* MAY (c) 1993 */
	DRIVER( dietgou )	/* MAY (c) 1993 */
	DRIVER( dietgoj )	/* MAY (c) 1993 */
	DRIVER( pktgaldx )	/* MAZ (c) 1993 Data East (Euro v3.00) */
	DRIVER( pktgaldxj )	/* MAZ (c) 1993 */
	DRIVER( pktgaldxb )	/* bootleg */
						/* MBA ?? */
						/* MBB ?? */
						/* MBC ?? */
	DRIVER( boogwing )	/* MBD (c) 1992 (Euro) */
	DRIVER( boogwinga )	/* MBD (c) 1992 (Asia) */
	DRIVER( ragtime )	/* MBD (c) 1992 (Japan) */
	DRIVER( ragtimea )	/* MBD (c) 1992 (Japan) */
	DRIVER( dblewing )	/* MBE (c) 1993 Mitchell */
	DRIVER( fghthist )	/* MBF (c) 1993 Data East Corporation (World) */
	DRIVER( fghthistu )	/* MBF (c) 1993 Data East Corporation (US) */
	DRIVER( fghthista )	/* MBF (c) 1993 Data East Corporation (US) */
	DRIVER( fghthistj )	/* MBF (c) 1993 Data East Corporation (Japan) */
	DRIVER( hvysmsh )	/* MBG (c) 1993 Data East Corporation (World) */
	DRIVER( hvysmsha )	/* MBG (c) 1993 Data East Corporation (Asia) */
	DRIVER( hvysmshj )	/* MBG (c) 1993 Data East Corporation (Japan) */
	DRIVER( nslasher )	/* MBH (c) 1993 Data East Corporation */
	DRIVER( nslasherj )	/* MBH (c) 1993 Data East Corporation (Japan)*/
	DRIVER( nslashers )	/* MBH (c) 1993 Data East Corporation (Japan)*/
						/* MBI ?? */
						/* MBJ ?? */
						/* MBK ?? */
						/* MBL ?? */
	DRIVER( lockload )	/* MBM (c) 1994 Data East Corporation (US) */
	DRIVER( lockloadu )	/* MBM (c) 1994 Data East Corporation (US) */
	DRIVER( joemacr )	/* MBN (c) 1994 */
	DRIVER( joemacra )	/* MBN (c) 1994 */
						/* MBO ?? */
						/* MBP ?? */
						/* MBQ ?? */
	DRIVER( tattass )	/* prototype (c) 1994 Data East Pinball (US) */
	DRIVER( tattassa )	/* prototype (c) 1994 Data East Pinball (Asia) */
	DRIVER( charlien )	/* MBR (c) 1994 Mitchell */
						/* MBS ?? */
						/* MBT ?? */
						/* MBU ?? */
						/* MBV ?? */
						/* MBW ?? */
	DRIVER( wcvol95 )	/* MBX (c) 1993 Data East */
						/* MBY ?? */
	DRIVER( backfire )	/* MBZ (c) 1995 */
	DRIVER( backfirea )	/* MBZ (c) 1995 */
						/* MCA ?? */
	DRIVER( prtytime )	/* MCB (c) 1995 Mitchell */
	DRIVER( gangonta )	/* MCB (c) 1995 Mitchell */
	DRIVER( chainrec )	/* MCC (c) 1994 */
	DRIVER( magdrop )	/* MCC (c) 1995 Data East */
	DRIVER( magdropp )	/* MCC (c) 1995 Data East */
						/* MCD ?? */
	/* MCE Dunk Dream 95/Hoops96 See MLC games below */
	DRIVER( osman )		/* MCF (c) 1996 Mitchell */
	DRIVER( candance )	/* MCF (c) 1996 Mitchell */
	DRIVER( sotsugyo )	/*     (c) 1995 Mitchell (Atlus license) */
	DRIVER( sshangha )	/*     (c) 1992 Hot-B */
	DRIVER( sshanghab )	/* bootleg */

	/* Data East MLC Games */
	DRIVER( hoops96 )	/* MCE (c) 1996 Data East Corporation */
	DRIVER( ddream95 )	/* MCE (c) 1996 Data East Corporation */
	DRIVER( hoops95 )	/* MCE (c) 1996 Data East Corporation */
	DRIVER( avengrgs )	/* MCG (c) 1995 Data East Corporation (World) */
	DRIVER( avengrgsj )	/* MCG (c) 1995 Data East Corporation (Japan) */
	DRIVER( skullfng )	/* MCH (c) 1996 Data East Corporation (World) */
	DRIVER( skullfngj )	/* MCH (c) 1996 Data East Corporation (Japan) */
	DRIVER( stadhr96 )	/* MCM (c) 1996 Data East */
	DRIVER( stadhr96j )	/* MCM (c) 1996 Data East */

	/* Tehkan / Tecmo games (Tehkan became Tecmo in 1986) */
	DRIVER( senjyo )	/* (c) 1983 Tehkan */
	DRIVER( starforc )	/* (c) 1984 Tehkan */
	DRIVER( starforce )	/* (c) 1984 Tehkan */
	DRIVER( starforcb )	/* (c) 1984 Tehkan */
	DRIVER( starforca )	/* (c) 1984 Tehkan */
	DRIVER( megaforc )	/* (c) 1985 Tehkan + Video Ware license */
	DRIVER( baluba )	/* (c) 1986 Able Corp. */
	DRIVER( bombjack )	/* (c) 1984 Tehkan */
	DRIVER( bombjack2 )	/* (c) 1984 Tehkan */
	DRIVER( pbaction )	/* (c) 1985 Tehkan */
	DRIVER( pbaction2 )	/* (c) 1985 Tehkan */
	DRIVER( pbaction3 )	/* (c) 1985 Tehkan */
	DRIVER( pbaction4 )	/* (c) 1985 Tehkan */
	DRIVER( pbaction5 )	/* (c) 1985 Tehkan */
						/* 6009 Tank Busters */
	DRIVER( ponttehk )	/* 6011 - (c) 1985 Tehkan */
	DRIVER( lvpoker )	/* ???? - (c) 1985 Tehkan */
	DRIVER( lvcards )	/* (c) 1985 Tehkan */
	DRIVER( tehkanwc )	/* (c) 1985 Tehkan */
	DRIVER( tehkanwcb )	/* bootleg */
	DRIVER( tehkanwcc )	/* bootleg */
	DRIVER( gridiron )	/* (c) 1985 Tehkan */
	DRIVER( teedoff )	/* 6102 - (c) 1986 Tecmo */
	DRIVER( solomon )	/* (c) 1986 Tecmo */
	DRIVER( solomonj )	/* (c) 1986 Tecmo */
	DRIVER( rygar )		/* 6002 - (c) 1986 Tecmo */
	DRIVER( rygar2 )	/* 6002 - (c) 1986 Tecmo */
	DRIVER( rygar3 )	/* 6002 - (c) 1986 Tecmo */
	DRIVER( rygarj )	/* 6002 - (c) 1986 Tecmo */
	DRIVER( gemini )	/* (c) 1987 Tecmo */
	DRIVER( silkworm )	/* 6217 - (c) 1988 Tecmo */
	DRIVER( silkworm2 )	/* 6217 - (c) 1988 Tecmo */
	DRIVER( backfirt )	/* bootleg [(c) 1988 Tecmo] */
	DRIVER( tbowl )		/* 6206 - (c) 1987 Tecmo */
	DRIVER( tbowlj )	/* 6206 - (c) 1987 Tecmo */
	DRIVER( shadoww )	/* 6215 - (c) 1988 Tecmo (World) */
	DRIVER( shadowwa )	/* 6215 - (c) 1988 Tecmo (World) */
	DRIVER( gaiden )	/* 6215 - (c) 1988 Tecmo (US) */
	DRIVER( ryukendn )	/* 6215 - (c) 1989 Tecmo (Japan) */
	DRIVER( ryukendna )	/* 6215 - (c) 1989 Tecmo (Japan) */
	DRIVER( mastninj )	/* bootleg */
	DRIVER( wildfang )	/* (c) 1989 Tecmo */
	DRIVER( wildfangs )	/* (c) 1989 Tecmo */
	DRIVER( tknight )	/* (c) 1989 Tecmo */
	DRIVER( stratof )	/* (c) 1991 Tecmo */
	DRIVER( raiga )		/* (c) 1991 Tecmo */
	DRIVER( drgnbowl )	/* (c) 1992 Nics */
	DRIVER( wc90 )		/* (c) 1989 Tecmo */
	DRIVER( wc90a )		/* (c) 1989 Tecmo */
	DRIVER( wc90b )		/* (c) 1989 Tecmo */
	DRIVER( wc90t )		/* (c) 1989 Tecmo */
	DRIVER( wc90b1 )	/* bootleg */
	DRIVER( wc90b2 )	/* bootleg */
	DRIVER( spbactn )	/* 9002 - (c) 1991 Tecmo */
	DRIVER( spbactnj )	/* 9002 - (c) 1991 Tecmo */
	DRIVER( fstarfrc )	/* (c) 1992 Tecmo */
	DRIVER( fstarfrcj )	/* (c) 1992 Tecmo */
	DRIVER( ginkun )	/* (c) 1995 Tecmo */
	DRIVER( deroon )	/* (c) 1996 Tecmo */
	DRIVER( tkdensho )	/* (c) 1996 Tecmo */
	DRIVER( tkdenshoa )	/* (c) 1996 Tecmo */

	/* Konami bitmap games */
	DRIVER( tutankhm )	/* GX350 (c) 1982 Konami */
	DRIVER( tutankhms )	/* GX350 (c) 1982 Stern */
	DRIVER( junofrst )	/* GX310 (c) 1983 Konami */
	DRIVER( junofrstg )	/* GX310 (c) 1983 Konami + Gottlieb license */

	/* Konami games */
	DRIVER( pooyan )	/* GX320 (c) 1982 */
	DRIVER( pooyans )	/* GX320 (c) 1982 Stern */
	DRIVER( pootan )	/* bootleg */
	DRIVER( timeplt )	/* GX393 (c) 1982 */
	DRIVER( timepltc )	/* GX393 (c) 1982 + Centuri license*/
	DRIVER( timeplta )	/* GX393 (c) 1982 + Atari license*/
	DRIVER( spaceplt )	/* bootleg */
	DRIVER( psurge )	/* (c) 1988 unknown (NOT Konami) */
	DRIVER( megazone )	/* GX319 (c) 1983 */
	DRIVER( megazonea )	/* GX319 (c) 1983 */
	DRIVER( megazoneb )	/* GX319 (c) 1983 + Kosuka */
	DRIVER( megazonec )	/* GX319 (c) 1983 + Kosuka */
	DRIVER( megazonei )	/* GX319 (c) 1983 + Interlogic / Kosuka */
	DRIVER( pandoras )	/* GX328 (c) 1984 + Interlogic */
	DRIVER( gyruss )	/* GX347 (c) 1983 */
	DRIVER( gyrussce )	/* GX347 (c) 1983 + Centuri license */
	DRIVER( venus )		/* bootleg */
	DRIVER( trackfld )	/* GX361 (c) 1983 */
	DRIVER( trackfldc )	/* GX361 (c) 1983 + Centuri license */
	DRIVER( trackfldnz )/* (c) 1982 Goldberg Enterprizes Inc. */
	DRIVER( hyprolym )	/* GX361 (c) 1983 */
	DRIVER( hyprolymb )	/* bootleg */
	DRIVER( atlantol )	/* bootleg */
	DRIVER( wizzquiz )	/* (c) 1985 Konami */
	DRIVER( wizzquiza )	/* (c) 1985 Zilec-Zenitone */
	DRIVER( reaktor )	/* (c) 1985 Zilec */
	DRIVER( mastkin )	/* (c) 1988 Du Tech */
	DRIVER( rocnrope )	/* GX364 (c) 1983 */
	DRIVER( rocnropek )	/* GX364 (c) 1983 + Kosuka */
	DRIVER( circusc )	/* GX380 (c) 1984 */
	DRIVER( circusc2 )	/* GX380 (c) 1984 */
	DRIVER( circusc3 )	/* GX380 (c) 1984 */
	DRIVER( circuscc )	/* GX380 (c) 1984 + Centuri license */
	DRIVER( circusce )	/* GX380 (c) 1984 + Centuri license */
	DRIVER( tp84 )		/* GX388 (c) 1984 */
	DRIVER( tp84a )		/* GX388 (c) 1984 */
	DRIVER( tp84b )		/* GX388 (c) 1984 */
	DRIVER( hyperspt )	/* GX330 (c) 1984 + Centuri */
	DRIVER( hypersptb ) /* bootleg */
	DRIVER( hpolym84 )	/* GX330 (c) 1984 */
	DRIVER( sbasketb )	/* GX405 (c) 1984 (Version I, Encrypted) */
	DRIVER( sbasketh )	/* GX405 (c) 1984 (Version H, Unprotected) */
	DRIVER( sbasketg )	/* GX405 (c) 1984 (Version G, Encrypted) */
	DRIVER( sbaskete )	/* GX405 (c) 1984 (Version E, Encrypted) */
	DRIVER( mikie )		/* GX469 (c) 1984 */
	DRIVER( mikiej )	/* GX469 (c) 1984 */
	DRIVER( mikiehs )	/* GX469 (c) 1984 */
	DRIVER( roadf )		/* GX461 (c) 1984 */
	DRIVER( roadf2 )	/* GX461 (c) 1984 */
	DRIVER( yiear )		/* GX407 (c) 1985 */
	DRIVER( yiear2 )	/* GX407 (c) 1985 */
	DRIVER( yieartf )	/* GX361 (c) 1985 */
	DRIVER( kicker )	/* GX477 (c) 1985 */
	DRIVER( shaolins )	/* GX477 (c) 1985 */
	DRIVER( shaolinb )	/* bootleg */
	DRIVER( pingpong )	/* GX555 (c) 1985 */
	DRIVER( merlinmm )	/* (c) 1986 Zilec-Zenitone */
	DRIVER( cashquiz )	/* (c) 1986 Zilec-Zenitone */
	DRIVER( gberet )	/* GX577 (c) 1985 */
	DRIVER( rushatck )	/* GX577 (c) 1985 */
	DRIVER( gberetb )	/* bootleg on different hardware */
	DRIVER( mrgoemon )	/* GX621 (c) 1986 (Japan) */
	DRIVER( jailbrek )	/* GX507 (c) 1986 */
	DRIVER( manhatan )	/* GX507 (c) 1986 (Japan) */
	DRIVER( jailbrekb )	/* bootleg */
	DRIVER( scotrsht )	/* GX545 (c) 1985 */
	DRIVER( finalizr )	/* GX523 (c) 1985 */
	DRIVER( finalizrb )	/* bootleg */
	DRIVER( ironhors )	/* GX560 (c) 1986 */
	DRIVER( dairesya )	/* GX560 (c) 1986 (Japan) */
	DRIVER( farwest )	/* bootleg */
	DRIVER( jackal )	/* GX631 (c) 1986 (World) */
	DRIVER( topgunr )	/* GX631 (c) 1986 (US) */
	DRIVER( jackalj )	/* GX631 (c) 1986 (Japan) */
	DRIVER( topgunbl )	/* bootleg */
	DRIVER( ddribble )	/* GX690 (c) 1986 */
	DRIVER( contra )	/* GX633 (c) 1987 */
	DRIVER( contra1 )	/* GX633 (c) 1987 */
	DRIVER( contrab )	/* bootleg */
	DRIVER( contraj )	/* GX633 (c) 1987 (Japan) */
	DRIVER( contrajb )	/* bootleg */
	DRIVER( gryzor )	/* GX633 (c) 1987 */
	DRIVER( gryzora )	/* GX633 (c) 1987 */
	DRIVER( combatsc )	/* GX611 (c) 1988 */
	DRIVER( combatsct )	/* GX611 (c) 1987 */
	DRIVER( combatscj )	/* GX611 (c) 1987 (Japan) */
	DRIVER( bootcamp )	/* GX611 (c) 1987 */
	DRIVER( combatscb )	/* bootleg */
	DRIVER( rockrage )	/* GX620 (c) 1986 (World?) */
	DRIVER( rockragea )	/* GX620 (c) 1986 (Prototype?) */
	DRIVER( rockragej )	/* GX620 (c) 1986 (Japan) */
	DRIVER( mx5000 )	/* GX669 (c) 1987 */
	DRIVER( flkatck )	/* GX669 (c) 1987 (Japan) */
	DRIVER( fastlane )	/* GX752 (c) 1987 */
	DRIVER( tricktrp )	/* GX771 (c) 1987 */
	DRIVER( labyrunr )	/* GX771 (c) 1987 (Japan) */
	DRIVER( labyrunrk )	/* GX771 (c) 1987 (Japan) */
	DRIVER( thehustl )	/* GX765 (c) 1987 (Japan) */
	DRIVER( thehustlj )	/* GX765 (c) 1987 (Japan) */
	DRIVER( rackemup )	/* GX765 (c) 1987 */
	DRIVER( battlnts )	/* GX777 (c) 1987 */
	DRIVER( battlntsj )	/* GX777 (c) 1987 (Japan) */
	DRIVER( bladestl )	/* GX797 (c) 1987 */
	DRIVER( bladestll )	/* GX797 (c) 1987 */
	DRIVER( bladestle )	/* GX797 (c) 1987 */
	DRIVER( hcastle )	/* GX768 (c) 1988 */
	DRIVER( hcastleo )	/* GX768 (c) 1988 */
	DRIVER( hcastlej )	/* GX768 (c) 1988 (Japan) */
	DRIVER( hcastljo )	/* GX768 (c) 1988 (Japan) */
	DRIVER( ajax )		/* GX770 (c) 1987 */
	DRIVER( typhoon )	/* GX770 (c) 1987 */
	DRIVER( ajaxj )		/* GX770 (c) 1987 (Japan) */
	DRIVER( scontra )	/* GX775 (c) 1988 */
	DRIVER( scontraj )	/* GX775 (c) 1988 (Japan) */
	DRIVER( thunderx )	/* GX873 (c) 1988 */
	DRIVER( thunderxa )	/* GX873 (c) 1988 */
	DRIVER( thunderxb )	/* GX873 (c) 1988 */
	DRIVER( thunderxj )	/* GX873 (c) 1988 (Japan) */
	DRIVER( mainevt )	/* GX799 (c) 1988 */
	DRIVER( mainevto )	/* GX799 (c) 1988 */
	DRIVER( mainevt2p )	/* GX799 (c) 1988 */
	DRIVER( ringohja )	/* GX799 (c) 1988 (Japan) */
	DRIVER( devstors )	/* GX890 (c) 1988 */
	DRIVER( devstors2 )	/* GX890 (c) 1988 */
	DRIVER( devstors3 )	/* GX890 (c) 1988 */
	DRIVER( garuka )	/* GX890 (c) 1988 (Japan) */
	DRIVER( 88games )	/* GX861 (c) 1988 */
	DRIVER( konami88 )	/* GX861 (c) 1988 */
	DRIVER( hypsptsp )	/* GX861 (c) 1988 (Japan) */
	DRIVER( gbusters )	/* GX878 (c) 1988 */
	DRIVER( gbustersa )	/* GX878 (c) 1988 */
	DRIVER( crazycop )	/* GX878 (c) 1988 (Japan) */
	DRIVER( crimfght )	/* GX821 (c) 1989 (US) */
	DRIVER( crimfght2 )	/* GX821 (c) 1989 (World) */
	DRIVER( crimfghtj )	/* GX821 (c) 1989 (Japan) */
	DRIVER( spy )		/* GX857 (c) 1989 (World) */
	DRIVER( spyu )		/* GX857 (c) 1989 (US) */
	DRIVER( bottom9 )	/* GX891 (c) 1989 */
	DRIVER( bottom9n )	/* GX891 (c) 1989 */
	DRIVER( mstadium )	/* GX891 (c) 1989 (Japan) */
	DRIVER( blockhl )	/* GX973 (c) 1989 */
	DRIVER( quarth )	/* GX973 (c) 1989 (Japan) */
	DRIVER( aliens )	/* GX875 (c) 1990 (World) */
	DRIVER( aliens2 )	/* GX875 (c) 1990 (World) */
	DRIVER( aliens3 )	/* GX875 (c) 1990 (World) */
	DRIVER( aliensu )	/* GX875 (c) 1990 (US) */
	DRIVER( aliensj )	/* GX875 (c) 1990 (Japan) */
	DRIVER( aliensj2 )	/* GX875 (c) 1990 (Japan) */
	DRIVER( aliensa )	/* GX875 (c) 1990 (Asia) */
	DRIVER( suratk )	/* GX911 (c) 1990 (World) */
	DRIVER( suratka )	/* GX911 (c) 1990 (Asia) */
	DRIVER( suratkj )	/* GX911 (c) 1990 (Japan) */
	DRIVER( parodius )	/* GX955 (c) 1990 (World) */
	DRIVER( parodiusj )	/* GX955 (c) 1990 (Japan) */
	DRIVER( rollerg )	/* GX999 (c) 1991 (US) */
	DRIVER( rollergj )	/* GX999 (c) 1991 (Japan) */
	DRIVER( simpsons )	/* GX072 (c) 1991 */
	DRIVER( simpsons4pa )	/* GX072 (c) 1991 */
	DRIVER( simpsons2p )	/* GX072 (c) 1991 */
	DRIVER( simpsons2p2 )	/* GX072 (c) 1991 */
	DRIVER( simpsons2pa )	/* GX072 (c) 1991 (Asia) */
	DRIVER( simpsons2pj )	/* GX072 (c) 1991 (Japan) */
	DRIVER( esckids )	/* GX975 (c) 1991 (Asia) */
	DRIVER( esckidsj )	/* GX975 (c) 1991 (Japan) */
	DRIVER( vendetta )	/* GX081 (c) 1991 (World) */
	DRIVER( vendettar )	/* GX081 (c) 1991 (World) */
	DRIVER( vendetta2p )/* GX081 (c) 1991 (World) */
	DRIVER( vendetta2pu )/* GX081 (c) 1991 (Asia) */
	DRIVER( vendetta2pd )/* GX081 (c) 1991 (Asia) */
	DRIVER( vendettaj )	/* GX081 (c) 1991 (Japan) */
	DRIVER( mogura )	/* GX141 (c) 1991 */
	DRIVER( wecleman )	/* GX602 (c) 1986 */
	DRIVER( hotchase )	/* GX763 (c) 1988 */
	DRIVER( chqflag )	/* GX717 (c) 1988 */
	DRIVER( chqflagj )	/* GX717 (c) 1988 (Japan) */
	DRIVER( ultraman )	/* GX910 (c) 1991 Banpresto/Bandai */
	DRIVER( hexion )	/* GX122 (c) 1992 */
	DRIVER( lethalen )	/* GX191 (c) 1992 */
	DRIVER( lethalenua )	/* GX191 (c) 1992 */
	DRIVER( lethalenux )	/* GX191 (c) 1992 */
	DRIVER( lethaleneab )	/* GX191 (c) 1992 */
	DRIVER( lethaleneae )	/* GX191 (c) 1992 */
	DRIVER( lethalenj )	/* GX191 (c) 1992 */
	DRIVER( tgtpanic )	/* GX601 (c) 1996 */

	/* Konami "Nemesis hardware" games */
	DRIVER( nemesis )	/* GX456 (c) 1985 */
	DRIVER( nemesisuk )	/* GX456 (c) 1985 */
	DRIVER( konamigt )	/* GX561 (c) 1985 */
	DRIVER( salamand )	/* GX587 (c) 1986 */
	DRIVER( salamandj )	/* GX587 (c) 1986 */
	DRIVER( lifefrce )	/* GX587 (c) 1986 (US) */
	DRIVER( lifefrcej )	/* GX587 (c) 1986 (Japan) */
	DRIVER( blkpnthr )	/* GX604 (c) 1987 (Japan) */
	DRIVER( citybomb )	/* GX787 (c) 1987 (World) */
	DRIVER( citybombj )	/* GX787 (c) 1987 (Japan) */
	DRIVER( kittenk )	/* GX712 (c) 1988 */
	DRIVER( nyanpani )	/* GX712 (c) 1988 (Japan) */
	DRIVER( hcrash )	/* GX790 (c) 1987 */
	DRIVER( hcrashc )	/* GX790 (c) 1987 */

	/* GX400 BIOS based games */
	DRIVER( rf2 )		/* GX561 (c) 1985 */
	DRIVER( twinbee )	/* GX412 (c) 1985 */
	DRIVER( gradius )	/* GX456 (c) 1985 */
	DRIVER( gwarrior )	/* GX578 (c) 1985 */

	/* Konami "Twin 16" games */
	DRIVER( devilw )	/* GX687 (c) 1987 */
	DRIVER( darkadv )	/* GX687 (c) 1987 */
	DRIVER( majuu )		/* GX687 (c) 1987 (Japan) */
	DRIVER( vulcan )	/* GX785 (c) 1988 */
	DRIVER( vulcana )	/* GX785 (c) 1988 */
	DRIVER( vulcanb )	/* GX785 (c) 1988 */
	DRIVER( gradius2 )	/* GX785 (c) 1988 (Japan) */
	DRIVER( gradius2a )	/* GX785 (c) 1988 (Japan) */
	DRIVER( gradius2b )	/* GX785 (c) 1988 (Japan) */
	DRIVER( cuebrickj )	/* GX903 (c) 1989 (Japan) */
	DRIVER( fround )	/* GX870 (c) 1988 */
	DRIVER( froundl )	/* GX870 (c) 1988 */
	DRIVER( hpuncher )	/* GX870 (c) 1988 (Japan) */
	DRIVER( miaj )		/* GX808 (c) 1989 (Japan) */

	/* (some) Konami 68000 games */
	DRIVER( cuebrick )	/* GX903 (c) 1989 */
	DRIVER( mia )		/* GX808 (c) 1989 */
	DRIVER( mia2 )		/* GX808 (c) 1989 */
	DRIVER( tmnt )		/* GX963 (c) 1989 (World) */
	DRIVER( tmntu )		/* GX963 (c) 1989 (US) */
	DRIVER( tmntua )	/* GX963 (c) 1989 (US) */
	DRIVER( tmht )		/* GX963 (c) 1989 (UK) */
	DRIVER( tmhta )		/* GX963 (c) 1989 (UK) */
	DRIVER( tmntj )		/* GX963 (c) 1990 (Japan) */
	DRIVER( tmht2p )	/* GX963 (c) 1989 (UK) */
	DRIVER( tmht2pa )	/* GX963 (c) 1989 (UK) */
	DRIVER( tmnt2pj )	/* GX963 (c) 1990 (Japan) */
	DRIVER( tmnt2po )	/* GX963 (c) 1989 (Oceania) */
	DRIVER( punkshot )	/* GX907 (c) 1990 (US) */
	DRIVER( punkshot2 )	/* GX907 (c) 1990 (US) */
	DRIVER( punkshotj )	/* GX907 (c) 1990 (Japan) */
	DRIVER( lgtnfght )	/* GX939 (c) 1990 (World) */
	DRIVER( lgtnfghta )	/* GX939 (c) 1990 (Asia) */
	DRIVER( lgtnfghtu )	/* GX939 (c) 1990 (US) */
	DRIVER( trigon )	/* GX939 (c) 1990 (Japan) */
	DRIVER( blswhstl )	/* GX060 (c) 1991 */
	DRIVER( detatwin )	/* GX060 (c) 1991 (Japan) */
	DRIVER( glfgreat )	/* GX061 (c) 1991 */
	DRIVER( glfgreatj )	/* GX061 (c) 1991 (Japan) */
	DRIVER( tmnt2 )		/* GX063 (c) 1991 (US) */
	DRIVER( tmnt2a )	/* GX063 (c) 1991 (Asia) */
	DRIVER( tmht22pe )	/* GX063 (c) 1991 (UK) */
	DRIVER( tmnt22pu )	/* GX063 (c) 1991 (US) */
	DRIVER( ssriders )	/* GX064 (c) 1991 (World) */
	DRIVER( ssridersebd )	/* GX064 (c) 1991 (World) */
	DRIVER( ssridersebc )	/* GX064 (c) 1991 (World) */
	DRIVER( ssridersuda )	/* GX064 (c) 1991 (US) */
	DRIVER( ssriderseaa )	/* GX064 (c) 1991 (US) */
	DRIVER( ssridersuac )	/* GX064 (c) 1991 (US) */
	DRIVER( ssridersubc )	/* GX064 (c) 1991 (US) */
	DRIVER( ssridersabd )	/* GX064 (c) 1991 (Asia) */
	DRIVER( ssridersadd )	/* GX064 (c) 1991 (Asia) */
	DRIVER( ssridersjbd )	/* GX064 (c) 1991 (Japan) */
	DRIVER( ssridersb )	/* bootleg */
	DRIVER( xmen )		/* GX065 (c) 1992 (US) */
	DRIVER( xmenj )		/* GX065 (c) 1992 (Japan) */
	DRIVER( xmene )		/* GX065 (c) 1992 (Europe) */
	DRIVER( xmen2pe )	/* GX065 (c) 1992 (World) */
	DRIVER( xmen2pa )	/* GX065 (c) 1992 (Asia) */
	DRIVER( xmen2pj )	/* GX065 (c) 1992 (Japan) */
	DRIVER( xmen6p )	/* GX065 (c) 1992 */
	DRIVER( xmen6pu )	/* GX065 (c) 1992 */
	DRIVER( xexex )		/* GX067 (c) 1991 (World) */
	DRIVER( xexexa )	/* GX067 (c) 1991 (AAA) */
	DRIVER( xexexj )	/* GX067 (c) 1991 (Japan) */
	DRIVER( asterix )	/* GX068 (c) 1992 (World) */
	DRIVER( asterixeac )	/* GX068 (c) 1992 (World) */
	DRIVER( asterixeaa )	/* GX068 (c) 1992 (World) */
	DRIVER( asterixaad )	/* GX068 (c) 1992 (Asia) */
	DRIVER( asterixj )	/* GX068 (c) 1992 (Japan) */
	DRIVER( gijoe )		/* GX069 (c) 1991 (World) */
	DRIVER( gijoeu )	/* GX069 (c) 1991 (US) */
	DRIVER( gijoej )	/* GX069 (c) 1991 (Japan) */
	DRIVER( thndrx2 )	/* GX073 (c) 1991 (World) */
	DRIVER( thndrx2a )	/* GX073 (c) 1991 (Asia) */
	DRIVER( thndrx2j )	/* GX073 (c) 1991 (Japan) */
	DRIVER( prmrsocr )	/* GX101 (c) 1993 (Europe) */
	DRIVER( prmrsocrj )	/* GX101 (c) 1993 (Japan) */
	DRIVER( qgakumon )	/* GX248 (c) 1993 (Japan) */
	DRIVER( moo )		/* GX151 (c) 1992 (World) */
	DRIVER( mooua )		/* GX151 (c) 1992 (US) */
	DRIVER( mooaa )		/* GX151 (c) 1992 (US) */
	DRIVER( moobl )		/* bootleg */
	DRIVER( bucky )		/* GX173 (c) 1992 (World) */
	DRIVER( buckyua )	/* GX173 (c) 1992 (US) */
	DRIVER( buckyaa )	/* GX173 (c) 1992 (Asia) */
	DRIVER( gaiapols )	/* GX123 (c) 1993 (Europe) */
	DRIVER( gaiapolsu )	/* GX123 (c) 1993 (US) */
	DRIVER( gaiapolsj )	/* GX123 (c) 1993 (Japan) */
	DRIVER( mystwarr )	/* GX128 (c) 1993 (World) */
	DRIVER( mystwarru )	/* GX128 (c) 1993 (US) */
	DRIVER( mystwarrj )	/* GX128 (c) 1993 (Japan) */
	DRIVER( mystwarra )	/* GX128 (c) 1993 (Asia) */
	DRIVER( viostorm )	/* GX168 (c) 1993 (Europe) */
	DRIVER( viostormu )	/* GX168 (c) 1993 (US) */
	DRIVER( viostormub )	/* GX168 (c) 1993 (US) */
	DRIVER( viostormj )	/* GX168 (c) 1993 (Japan) */
	DRIVER( viostorma )	/* GX168 (c) 1993 (Asia) */
	DRIVER( mmaulers )	/* GX170 (c) 1993 (Europe) */
	DRIVER( dadandrn )	/* GX170 (c) 1993 (Japan) */
	DRIVER( metamrph )	/* GX224 (c) 1993 (Europe) */
	DRIVER( metamrphu )	/* GX224 (c) 1993 (US) */
	DRIVER( metamrphj )	/* GX224 (c) 1993 (Japan) */
	DRIVER( mtlchamp )	/* GX234 (c) 1993 (World) */
	DRIVER( mtlchamp1 )	/* GX234 (c) 1993 (World) */
	DRIVER( mtlchampu )	/* GX234 (c) 1993 (US) */
	DRIVER( mtlchampj )	/* GX234 (c) 1993 (Japan) */
	DRIVER( mtlchampa )	/* GX234 (c) 1993 (Japan) */
	DRIVER( rungun )	/* GX247 (c) 1993 (World) */
	DRIVER( runguna )	/* GX247 (c) 1993 (World) */
	DRIVER( rungunu )	/* GX247 (c) 1993 (US) */
	DRIVER( rungunua )	/* GX247 (c) 1993 (US) */
	DRIVER( slmdunkj )	/* GX247 (c) 1993 (Japan) */
	DRIVER( dbz )		/* (c) 1993 Banpresto */
	DRIVER( dbz2 )		/* (c) 1994 Banpresto */
	DRIVER( bishi )		/* GX575 (c) 1996 (Japan) */
	DRIVER( sbishi )	/* GX675 (c) 1998 (Japan) */
	DRIVER( sbishik )	/* GX675 (c) 1998 (Korea) */

	/* Konami dual 68000 games */
	DRIVER( overdriv )	/* GX789 (c) 1990 */
	DRIVER( gradius3 )	/* GX945 (c) 1989 (Japan) */
	DRIVER( gradius3a )	/* GX945 (c) 1989 (Asia) */
	DRIVER( gradius3e )	/* GX945 (c) 1989 (World?) */

	/* Konami 68020 games */
	DRIVER( plygonet )	/* GX305 (c) 1993 */
	DRIVER( polynetw )	/* GX305 too? (c) 1993 */

	/* Konami System GX games */
	DRIVER( konamigx )

	/* GX Type 1 */
	DRIVER( racinfrc )	/* 1994.?? GX250 (Europe) */
	DRIVER( racinfrcu )	/* 1994.03 GX250 (US) */
						/* 1994.02 (Japan) */
	DRIVER( opengolf )	/* 1994.?? GX218 (Europe) */
	DRIVER( opengolf2 )	/* 1994.?? GX218 (Europe) */
	DRIVER( ggreats2 )	/* 1994.03 GX218 (Japan) */
						/* 1995.03 GX218 (US) */

	/* GX Type 2 */
	DRIVER( le2 )		/* 1994.?? GX312 (Europe) */
	DRIVER( le2u )		/* 1994.04 GX312 (US) */
	DRIVER( le2j )		/* 1994.06 GX312 (Japan) */
	DRIVER( puzldama )	/* 1994.07 GX315 (Japan) */
	DRIVER( gokuparo )	/* 1994.04 GX321 (Japan) */
	DRIVER( fantjour )	/* 1994.?? GX321 (Euro) */
	DRIVER( fantjoura )	/* 1994.?? GX321 (Asia) */
	DRIVER( dragoona )	/* 1995.?? GX417 (Asia) */
	DRIVER( dragoonj )	/* 1995.08 GX417 (Japan) */
	DRIVER( tbyahhoo )	/* 1995.04 GX424 (Japan) */
	DRIVER( tkmmpzdm )	/* 1995.12 GX515 (Japan) */
	DRIVER( salmndr2 )	/* 1996.01 GX521 (Japan) */
	DRIVER( salmndr2a )	/* 1996.?? GX521 (Asia) */
	DRIVER( sexyparo )	/* 1996.02 GX533 (Japan) */
	DRIVER( daiskiss )	/* 1996.03 GX535 (Japan) */
	DRIVER( tokkae )	/* 1996.10 GX615 (Japan) */
	DRIVER( winspike )	/* 1997.?? GX705 (Europe) */
	DRIVER( winspikej )	/* 1997.07 GX705 (Japan) */

	/* GX Type 3 */
	DRIVER( soccerss )	/* 1994.?? GX427 (Europe) */
	DRIVER( soccerssj )	/* 1994.12 GX427 (Japan) */
	DRIVER( soccerssja )/* 1994.12 GX427 (Japan) */
	DRIVER( soccerssa )	/* 1994.?? GX427 (Asia) */
						/* 1995.04 GX427 (US) */

	/* GX Type 4 */
	DRIVER( vsnetscr )	/* 1996.?? GX627 (Europe) */
	DRIVER( vsnetscreb )/* 1996.?? GX627 (Europe) */
	DRIVER( vsnetscru )	/* 1996.?? GX627 (US) */
	DRIVER( vsnetscra )	/* 1996.?? GX627 (Asia) */
	DRIVER( vsnetscrj )	/* 1996.12 GX627 (Japan) */
	DRIVER( rungun2 )	/* 1996.03 GX505 (US) */
	DRIVER( slamdnk2 )	/* 1996.03 GX505 (Japan) */
	DRIVER( rushhero )	/* 1997.10 GX605 (US) */

	/* 68k + HD */
	DRIVER( qdrmfgp )	/* 1994.12 GQ460 (Japan) */
	DRIVER( qdrmfgp2 )	/* 1995.09 GE557 (Japan) */

	/* 68000-based gambling games */
	DRIVER( kingtut )
	DRIVER( moneybnk )

	/* DJ Main */
	DRIVER( bm1stmix )	/* GQ753 (c) 1997 (Japan) */
	DRIVER( bm2ndmix )	/* GX853 (c) 1998 (Japan) */
	DRIVER( bm2ndmxa )	/* GX853 (c) 1998 (Japan) */
	DRIVER( bm3rdmix )	/* GX853 (c) 1998 (Japan) */
	//DRIVER( bm3rdmxb )    /* GX853 (c) 1998 (Japan) */
	DRIVER( bmcompmx )	/* GX858 (c) 1999 (Japan) */
	DRIVER( hmcompmx )	/* GX858 (c) 1999 */
	DRIVER( bm4thmix )	/* GX847 (c) 1999 (Japan) */
	DRIVER( bm5thmix )	/* GX981 (c) 1999 (Japan) */
	DRIVER( bmclubmx )	/* GX993 (c) 2000 (Japan) */
	DRIVER( bmcorerm )	/* GXA05 (c) 1999 (Japan) */
	DRIVER( bmdct )		/* GX995 (c) 2000 (Japan) */
	DRIVER( bmcompm2 )	/* GX988 (c) 2000 (Japan) */
	DRIVER( hmcompm2 )	/* GX988 (c) 2000 */
	DRIVER( bm6thmix )	/* GXA21 (c) 2001 (Japan) */
	DRIVER( bm7thmix )	/* GXB07 (c) 2001 (Japan) */
	DRIVER( bmfinal )	/* GXC01 (c) 2002 (Japan) */

	//DRIVER( popn1 )       /* GX803 (c) 1998 (Japan) */
	DRIVER( popn2 )       /* GX831 (c) 1998 (Japan) */
	//DRIVER( popn3 )       /* GX980 (c) 1999 (Japan) */
	//DRIVER( popnstex )    /* GX970 (c) 1999 (Japan) */

	/* Firebeat games */
	DRIVER( ppp )		/* GQ977 (c) 2000 */
	DRIVER( ppd )		/* GQ977 (c) 2000 (Korea) */
	DRIVER( ppp11 )		/* GQ977 (c) 2000 */
	DRIVER( kbm )		/* GQ974 (c) 2000 */
	DRIVER( kbm2nd )	/* GCA01 (c) 2000 */
	DRIVER( kbm3rd )	/* GCA12 (c) 2001 */
	DRIVER( popn5 )		/* GCA04 (c) 2000 */
	DRIVER( popn7 )		/* not sure, doesn't boot yet (c) 2001 */

	/* PowerPC based Konami games */
	DRIVER( fiveside )	/* GX479 (c)1995 */
	DRIVER( thunderh )	/* GX680 (c)1996 Euro EAA */
	DRIVER( thunderhu )	/* GX680 (c)1996 USA UAA */
	DRIVER( slrasslt )	/* GX792 (c)1996 */
	DRIVER( gticlub )	/* GX688 (c)1996 Euro EAA */
	DRIVER( gticluba )	/* GX688 (c)1996 Asia AAA */
	DRIVER( gticlubj )	/* GX688 (c)1996 Japan JAA */
	DRIVER( hangplt )	/* GX685 (c)1997 */
	DRIVER( nbapbp )	/* GX778 (c)1998 */
	DRIVER( terabrst )	/* GX715 (c)1998 USA UEL 1998/07/17*/
	DRIVER( terabrsta )	/* GX715 (c)1998 Asia AAA 1998/02/25 */
	DRIVER( gradius4 )	/* GX837 (c)1999 */
	DRIVER( sscope )	/* GX830 (c)1999 */
	DRIVER( sscopea )	/* GX830 (c)1999 */
	DRIVER( sscopeb )	/* GX830 (c)1999 */
	DRIVER( sscope2 )	/* GX931 (c)2000 */
	DRIVER( racingj )	/* GX676 (c)1998 */
	DRIVER( racingj2 )	/* GX888 (c)1999 */
	DRIVER( thrilld )	/* GX713 (c)1998 Japan JAE */
	DRIVER( thrilldb )	/* GX713 (c)1998 Japan JAB */
	DRIVER( midnrun )	/* GX476 (c)1995 */
	DRIVER( windheat )	/* GX677 (c)1996 */
	DRIVER( windheatu )	/* GX677 (c)1996 */
	DRIVER( windheatj )	/* GX677 (c)1996 */
	DRIVER( waveshrk )	/* GX678 (c)1996 */

	/* Konami M2 games */
	DRIVER( polystar )	/* GX623 (c)1997 */
	DRIVER( totlvice )	/* GX639 (c)1997 */
	DRIVER( totlvicj )	/* GX639 (c)1997 */
	DRIVER( btltryst )	/* GX636 (c)1998 */
	DRIVER( heatof11 )	/* GX703 (c)1998 */
	DRIVER( evilngt )	/* GX810 (c)1998 */
	DRIVER( hellngt )	/* GX810 (c)1998 */

	/* Exidy games */
	DRIVER( carpolo )	/* (c) 1977 */
	DRIVER( sidetrac )	/* (c) 1979 */
	DRIVER( targ )		/* (c) 1980 */
	DRIVER( targc )		/* (c) 1980 */
	DRIVER( spectar )	/* (c) 1980 */
	DRIVER( spectar1 )	/* (c) 1980 */
	DRIVER( rallys )	/* (c) 1980 Novar (bootleg) */
	DRIVER( panzer )	/* (c) 1980 Proel (bootleg) */
	DRIVER( phantoma )	/* (c) 1980 Jeutel (bootleg) */
	DRIVER( phantom )	/* (c) 1980 Proel (bootleg) */
	DRIVER( venture )	/* (c) 1981 */
	DRIVER( venture2 )	/* (c) 1981 */
	DRIVER( venture4 )	/* (c) 1981 */
	DRIVER( teetert )	/* (c) 1981 */
	DRIVER( mtrap )		/* (c) 1981 */
	DRIVER( mtrap3 )	/* (c) 1981 */
	DRIVER( mtrap4 )	/* (c) 1981 */
	DRIVER( pepper2 )	/* (c) 1982 */
	DRIVER( hardhat )	/* (c) 1982 */
	DRIVER( fax )		/* (c) 1983 */
	DRIVER( fax2 )		/* (c) 1983 */
	DRIVER( circus )	/* no copyright notice [1977?] */
	DRIVER( circusse )	/* bootleg */
	DRIVER( robotbwl )	/* no copyright notice */
	DRIVER( crash )		/* Exidy [1979?] */
	DRIVER( smash )		/* bootleg */
	DRIVER( ripcord )	/* Exidy [1977?] */
	DRIVER( starfire )	/* Exidy [1979?] */
	DRIVER( starfirea )	/* Exidy [1979?] */
	DRIVER( fireone )	/* (c) 1979 Exidy */
	DRIVER( starfir2 )	/* (c) 1979 Exidy */
	DRIVER( victory )	/* (c) 1982 */
	DRIVER( victorba )	/* (c) 1982 */
	DRIVER( topgunnr )	/* (c) 1986 */

	/* Exidy 440 games */
	DRIVER( crossbow )	/* (c) 1983 */
	DRIVER( cheyenne )	/* (c) 1984 */
	DRIVER( combat )	/* (c) 1985 */
	DRIVER( catch22 )	/* (c) 1985 */
	DRIVER( cracksht )	/* (c) 1985 */
	DRIVER( claypign )	/* (c) 1986 */
	DRIVER( chiller )	/* (c) 1986 */
	DRIVER( topsecex )	/* (c) 1986 */
	DRIVER( hitnmiss )	/* (c) 1987 */
	DRIVER( hitnmiss2 )	/* (c) 1987 */
	DRIVER( whodunit )	/* (c) 1988 */
	DRIVER( showdown )	/* (c) 1988 */
	DRIVER( yukon )		/* (c) 1989 */
	DRIVER( yukon1 )	/* (c) 1989 */

	/* Exidy Max-A-Flex */
	DRIVER( maxaflex )
	DRIVER( mf_achas )	/* (c) 1982 Exidy / First Star Software */
	DRIVER( mf_brist )	/* (c) 1983 Exidy / First Star Software */
	DRIVER( mf_flip )	/* (c) 1983 Exidy / First Star Software */
	DRIVER( mf_bdash )	/* (c) 1984 Exidy / First Star Software */

	/* Atari b/w games */
	DRIVER( tank8 )		/* ??????           1976/04 [6800] */
	DRIVER( tank8a )	/* ??????           1976/04 [6800] */
	DRIVER( tank8b )	/* ??????           1976/04 [6800] */
	DRIVER( tank8c )	/* ??????           1976/04 [6800] */
	DRIVER( tank8d )	/* ??????           1976/04 [6800] */
	DRIVER( cball )		/* ?????? prototype 1976/?? [6800] */
	DRIVER( copsnrob )	/* 005625           1976/07 [6502] */
	DRIVER( flyball )	/* 005629           1976/07 [6502] */
	DRIVER( sprint2 )	/* 005922           1976/11 [6502] */
	DRIVER( sprint2a )	/* 005922           1976/11 [6502] */
	DRIVER( sprint4 )	/* 008716           1977/12 [6502] */
	DRIVER( sprint4a )	/* 008716           1977/12 [6502] */
	DRIVER( nitedrvr )	/* 006321           1976/10 [6502] */
	DRIVER( dominos )	/* 007305           1977/01 [6502] */
	DRIVER( triplhnt )	/* 008422-008791    1977/04 [6800] */
	DRIVER( sprint8 )	/* ??????           1977/05 [6800] */
	DRIVER( sprint8a )	/* ??????           1977/05 [6800] */
	DRIVER( dragrace )	/* 008505-008521    1977/06 [6800] */
	DRIVER( poolshrk )	/* 006281           1977/06 [6800] */
	DRIVER( starshp1 )	/* 007513-007531    1977/07 [6502] */
	DRIVER( starshpp )	/* 007513-007531    1977/07 [6502] */
	DRIVER( superbug )	/* 009115-009467    1977/09 [6800] */
	DRIVER( canyon )	/* 009493-009504    1977/10 [6502] */
	DRIVER( canyonp )	/* 009493-009504    1977/10 [6502] */
	DRIVER( destroyr )	/* 030131-030136    1977/10 [6800] */
	DRIVER( sprint1 )	/* 006443           1978/01 [6502] */
	DRIVER( ultratnk )	/* 009801           1978/02 [6502] */
	DRIVER( skyraid )	/* 009709           1978/03 [6502] */
	DRIVER( tourtabl )	/* 030170           1978/03 [6507] */
	DRIVER( tourtab2 )	/* 030170           1978/03 [6507] */
	DRIVER( avalnche )	/* 030574           1978/04 [6502] */
	DRIVER( cascade )	/* bootleg                         */
	DRIVER( firetrk )	/* 030926           1978/06 [6808] */
	DRIVER( skydiver )	/* 009787           1978/06 [6800] */
	/* Smokey Joe */	/* 030926           1978/07 [6502] */
	DRIVER( sbrkout3 )	/* 033442-033452    1978/09 [6502] */
	DRIVER( sbrkout )	/* 033453-033455    1978/09 [6502] */
	DRIVER( atarifb )	/* 033xxx           1978/10 [6502] */
	DRIVER( atarifb1 )	/* 033xxx           1978/10 [6502] */
	DRIVER( orbit )		/* 033689-033702    1978/11 [6800] */
	DRIVER( boxer )		/* ?????? prototype 1978    [6502] */
	DRIVER( wolfpack )	/* ?????? prototype         [6502] */
	DRIVER( mgolf )		/* ?????? prototype 1978    [6502] */
	DRIVER( videopin )	/* 034253-034267    1979/02 [6502] */
	DRIVER( atarifb4 )	/* 034754           1979/04 [6502] */
	DRIVER( subs )		/* 033714           1979/05 [6502] */
	DRIVER( bsktball )	/* 034756-034766    1979/05 [6502] */
	DRIVER( abaseb )	/* 034711-034738    1979/06 [6502] */
	DRIVER( abaseb2 )	/* 034711-034738    1979/06 [6502] */
	DRIVER( montecar )	/* 035763-035780    1980/04 [6502] */
	DRIVER( soccer )	/* 035222-035260    1980/04 [6502] */

	/* Atari "Missile Command hardware" games */
	DRIVER( missile )	/* 035820-035825    (c) 1980 */
	DRIVER( missile2 )	/* 035820-035825    (c) 1980 */
	DRIVER( suprmatk )	/*                  (c) 1980 + (c) 1981 Gencomp */
	DRIVER( suprmatkd )	/*                  (c) 1980 + (c) 1981 Gencomp */
	DRIVER( mcombat )
	DRIVER( mcombata )

	/* Atari vector games */
	DRIVER( llander )	/* 0345xx           no copyright notice */
	DRIVER( llander1 )	/* 0345xx           no copyright notice */
	DRIVER( asteroid )	/* 035127-035145    (c) 1979 */
	DRIVER( asteroid2 )	/* 035127-035145    (c) 1979 */
	DRIVER( asteroid1 )	/* 035127-035145    no copyright notice */
	DRIVER( asteroidb )	/* (bootleg) */
	DRIVER( asterock )	/* Sidam bootleg    (c) 1979 */
	DRIVER( meteorts )	/* VCC bootleg      (c) 1979 */
	DRIVER( meteorho )	/* Hoei? bootleg    (c) 1980 */
	DRIVER( astdelux )	/* 0351xx           (c) 1980 */
	DRIVER( astdelux2 )	/* 0351xx           (c) 1980 */
	DRIVER( astdelux1 )	/* 0351xx           (c) 1980 */
	DRIVER( bzone )		/* 0364xx           (c) 1980 */
	DRIVER( bzone2 )	/* 0364xx           (c) 1980 */
	DRIVER( bzonec )	/* 0364xx           (c) 1980 */
	DRIVER( bradley )	/*     ??           (c) 1980 */
	DRIVER( redbaron )	/* 036995-037007    (c) 1980 */
	DRIVER( tempest )	/* 136002           (c) 1980 */
	DRIVER( tempest1 )	/* 136002           (c) 1980 */
	DRIVER( tempest2 )	/* 136002           (c) 1980 */
	DRIVER( tempest3 )	/* 136002           (c) 1980 */
	DRIVER( temptube )	/* (hack) */
	DRIVER( spacduel )	/* 136006           (c) 1980 */
	DRIVER( gravitar )	/* 136010           (c) 1982 */
	DRIVER( gravitar2 )	/* 136010           (c) 1982 */
	DRIVER( gravp )		/* (proto)          (c) 1982 */
	DRIVER( lunarbat )	/* (proto)          (c) 1982 */
	DRIVER( lunarba1 )	/* (proto)          (c) 1982 */
	DRIVER( quantum )	/* 136016           (c) 1982 */	/* made by Gencomp */
	DRIVER( quantum1 )	/* 136016           (c) 1982 */	/* made by Gencomp */
	DRIVER( quantump )	/* 136016           (c) 1982 */	/* made by Gencomp */
	DRIVER( bwidow )	/* 136017           (c) 1982 */
	DRIVER( starwars )	/* 136021           (c) 1983 */
	DRIVER( starwars1 )	/* 136021           (c) 1983 */
	DRIVER( tomcatsw )	/* (proto)          (c) 1983 */
	DRIVER( mhavoc )	/* 136025           (c) 1983 */
	DRIVER( mhavoc2 )	/* 136025           (c) 1983 */
	DRIVER( mhavocp )	/* 136025           (c) 1983 */
	DRIVER( mhavocrv )	/* (hack) */
	DRIVER( alphaone )	/* (proto)          (c) 1983 */
	DRIVER( alphaonea )	/* (proto)          (c) 1983 */
	DRIVER( esb )		/* 136031           (c) 1985 */
	DRIVER( tomcat )	/* (proto)          (c) 1985 */

	/* Atari "Centipede hardware" games */
	DRIVER( warlords )	/* 037153-037159    (c) 1980 */
	DRIVER( centiped )	/* 136001           (c) 1980 */
	DRIVER( centiped2 )	/* 136001           (c) 1980 */
	DRIVER( centtime )	/* 136001           (c) 1980 */
	DRIVER( centipdb )	/* (bootleg) */
	DRIVER( centipdd )	/* hack by Two Bit Score */
	DRIVER( caterplr )	/* (bootleg) */
	DRIVER( millpac )	/* Valadon */
	DRIVER( magworm )	/* (bootleg) */
	DRIVER( mazeinv )	/* (proto)          (c) 1982 */
	DRIVER( milliped )	/* 136013           (c) 1982 */
	DRIVER( millipdd )	/* hack by Two Bit Score */
	DRIVER( qwak )		/* (proto)          (c) 1982 */
	DRIVER( runaway )	/* (proto)          (c) 1982 */
	DRIVER( bullsdrt )	/* (c) 1985 Shinkai */
	DRIVER( cmmb162 )

	/* misc Atari games */
	DRIVER( tunhunt )	/* 136000           (c) 1979 */
	DRIVER( tunhuntc )	/* 136000           (c) 1981 */ /* licensed to / distributed by Centuri */
	DRIVER( liberatr )	/* 136012           (c) 1982 */
	DRIVER( liberatr2 )	/* 136012           (c) 1982 */
	DRIVER( foodf )		/* 136020           (c) 1982 */	/* made by Gencomp */
	DRIVER( foodf2 )	/* 136020           (c) 1982 */	/* made by Gencomp */
	DRIVER( foodfc )	/* 136020           (c) 1982 */	/* made by Gencomp */
	DRIVER( ccastles )	/* 136022           (c) 1983 */
	DRIVER( ccastlesg )	/* 136022           (c) 1983 */
	DRIVER( ccastlesp )	/* 136022           (c) 1983 */
	DRIVER( ccastlesf )	/* 136022           (c) 1983 */
	DRIVER( ccastles3 )	/* 136022           (c) 1983 */
	DRIVER( ccastles2 )	/* 136022           (c) 1983 */
	DRIVER( ccastles1 )	/* 136022           (c) 1983 */
	DRIVER( ccastlesj )	/* 136022           (c) 1983 */
	DRIVER( cloak )		/* 136023           (c) 1983 */
	DRIVER( cloaksp )	/* 136023           (c) 1983 */
	DRIVER( cloakfr )	/* 136023           (c) 1983 */
	DRIVER( cloakgr )	/* 136023           (c) 1983 */
	DRIVER( agentx4 )	/* 136023           (c) 1983 */
	DRIVER( agentx3 )	/* 136023           (c) 1983 */
	DRIVER( agentx2 )	/* 136023           (c) 1983 */
	DRIVER( agentx1 )	/* 136023           (c) 1983 */
	DRIVER( cloud9 )	/* (proto)          (c) 1983 */
	DRIVER( firebeas )	/* (proto)          (c) 1983 */
	DRIVER( firefox )	/* 136026           (c) 1984 */
	DRIVER( firefoxa )	/* 136026           (c) 1984 */
	DRIVER( jedi )		/* 136030           (c) 1984 */

	/* Atari System 1 games */
	DRIVER( atarisy1 )	/* 136033           (c) 1984 */
	DRIVER( peterpak )	/* 136028           (c) 1984 */
	/* Marble Madness */
	DRIVER( marble )	/* 136033           (c) 1984 */
	DRIVER( marble2 )	/* 136033           (c) 1984 */
	DRIVER( marble3 )	/* 136033           (c) 1984 */
	DRIVER( marble4 )	/* 136033           (c) 1984 */
	DRIVER( marble5 )	/* 136033           (c) 1984   (LSI Cartridge) */
	/* Indiana Jones and the Temple of Doom */
	DRIVER( indytemp )	/* 136036           (c) 1985 */
	DRIVER( indytemp2 )	/* 136036           (c) 1985 */
	DRIVER( indytemp3 )	/* 136036           (c) 1985 */
	DRIVER( indytemp4 )	/* 136036           (c) 1985 */
	DRIVER( indytempd )	/* 136036           (c) 1985 */
	DRIVER( indytempc )	/* 136036           (c) 1985 */
	/* Road Runner */
	DRIVER( roadrunn )	/* 136040           (c) 1985 */
	DRIVER( roadrunn2 )	/* 136040           (c) 1985 */
	DRIVER( roadrunn1 )	/* 136040           (c) 1985 */
	/* Road Blasters */
	DRIVER( roadblst )	/* 136048           (c) 1986, 1987 */
	DRIVER( roadblstg )	/* 136048           (c) 1986, 1987 */
	DRIVER( roadblst3 )	/* 136048           (c) 1986, 1987 */
	DRIVER( roadblstg2 )/* 136048           (c) 1986, 1987 */
	DRIVER( roadblst2 )	/* 136048           (c) 1986, 1987 */
	DRIVER( roadblstg1 )/* 136048           (c) 1986, 1987 */
	DRIVER( roadblst1 )	/* 136048           (c) 1986, 1987 */
	DRIVER( roadblstc )	/* 136048           (c) 1986, 1987 */
	DRIVER( roadblstcg )/* 136048           (c) 1986, 1987 */
	DRIVER( roadblstc1 )/* 136048           (c) 1986, 1987 */

	/* Atari System 2 games */
	/* Paperboy */
	DRIVER( paperboy )	/* 136034           (c) 1984 */
	DRIVER( paperboyr2 )/* 136034           (c) 1984 */
	DRIVER( paperboyr1 )/* 136034           (c) 1984 */
	/* Super Sprint */
	DRIVER( ssprint )	/* 136042           (c) 1986 */
	DRIVER( ssprint3 )	/* 136042           (c) 1986 */
	DRIVER( ssprint1 )	/* 136042           (c) 1986 */
	DRIVER( ssprintg )	/* 136042           (c) 1986 */
	DRIVER( ssprintg1 )	/* 136042           (c) 1986 */
	DRIVER( ssprintf )	/* 136042           (c) 1986 */
	DRIVER( ssprints )	/* 136042           (c) 1986 */
	/* Championship Sprint */
	DRIVER( csprint )	/* 136045           (c) 1986 */
	DRIVER( csprint2 )	/* 136045           (c) 1986 */
	DRIVER( csprint1 )	/* 136045           (c) 1986 */
	DRIVER( csprintg )	/* 136045           (c) 1986 */
	DRIVER( csprintg1 )	/* 136045           (c) 1986 */
	DRIVER( csprintf )	/* 136045           (c) 1986 */
	DRIVER( csprints )	/* 136045           (c) 1986 */
	DRIVER( csprints1 )	/* 136045           (c) 1986 */
	/* 720 Degrees */
	DRIVER( 720 )		/* 136047           (c) 1986 */
	DRIVER( 720r3 )		/* 136047           (c) 1986 */
	DRIVER( 720r2 )		/* 136047           (c) 1986 */
	DRIVER( 720r1 )		/* 136047           (c) 1986 */
	DRIVER( 720g )		/* 136047           (c) 1986 */
	DRIVER( 720gr1 )	/* 136047           (c) 1986 */
	/* APB. */
	DRIVER( apb )		/* 136051           (c) 1987 */
	DRIVER( apb6 )		/* 136051           (c) 1987 */
	DRIVER( apb5 )		/* 136051           (c) 1987 */
	DRIVER( apb4 )		/* 136051           (c) 1987 */
	DRIVER( apb3 )		/* 136051           (c) 1987 */
	DRIVER( apb2 )		/* 136051           (c) 1987 */
	DRIVER( apb1 )		/* 136051           (c) 1987 */
	DRIVER( apbg )		/* 136051           (c) 1987 */
	DRIVER( apbf )		/* 136051           (c) 1987 */

	/* Atari polygon games */
	DRIVER( irobot )	/* 136029           (c) 1983 */
	/* Atari System IV */
	DRIVER( laststar )	/* (proto)          (c) 1984 */
	DRIVER( airrace )	/* (proto)          (c) 1985 */
	/* Hard Drivin' */
	DRIVER( harddriv )	/* 136052           (c) 1988 */
	DRIVER( harddrivb )	/* 136052           (c) 1988 */
	DRIVER( harddrivg )	/* 136052           (c) 1988 */
	DRIVER( harddrivj )	/* 136052           (c) 1988 */
	DRIVER( harddrivb6 )/* 136052           (c) 1988 */
	DRIVER( harddrivj6 )/* 136052           (c) 1988 */
	DRIVER( harddrivb5 )/* 136052           (c) 1988 */
	DRIVER( harddrivg4 )/* 136052           (c) 1988 */
	DRIVER( harddriv3 )	/* 136052           (c) 1988 */
	DRIVER( harddriv2 )	/* 136052           (c) 1988 */
	DRIVER( harddriv1 )	/* 136052           (c) 1988 */
	/* Hard Drivin' Compact */
	DRIVER( harddrivc )	/* 136068           (c) 1990 */
	DRIVER( harddrivcg )/* 136068           (c) 1990 */
	DRIVER( harddrivcb )/* 136068           (c) 1990 */
	DRIVER( harddrivc1 )/* 136068           (c) 1990 */
	/* Stun Runner */
	DRIVER( stunrun )	/* 136070           (c) 1989 */
	DRIVER( stunrunj )	/* 136070           (c) 1989 */
	DRIVER( stunrun5 )	/* 136070           (c) 1989 */
	DRIVER( stunrune )	/* 136070           (c) 1989 */
	DRIVER( stunrun4 )	/* 136070           (c) 1989 */
	DRIVER( stunrun3 )	/* 136070           (c) 1989 */
	DRIVER( stunrun3e )	/* 136070           (c) 1989 */
	DRIVER( stunrun2 )	/* 136070           (c) 1989 */
	DRIVER( stunrun2e )	/* 136070           (c) 1989 */
	DRIVER( stunrun0 )	/* 136070           (c) 1989 */
	DRIVER( stunrunp )	/* (proto)          (c) 1989 */
	/* Race Drivin' */
	DRIVER( racedriv )	/* 136077           (c) 1990 */
	DRIVER( racedrivb )	/* 136077           (c) 1990 */
	DRIVER( racedrivg )	/* 136077           (c) 1990 */
	DRIVER( racedriv4 )	/* 136077           (c) 1990 */
	DRIVER( racedrivb4 )/* 136077           (c) 1990 */
	DRIVER( racedrivg4 )/* 136077           (c) 1990 */
	DRIVER( racedriv3 )	/* 136077           (c) 1990 */
	DRIVER( racedriv2 )	/* 136077           (c) 1990 */
	DRIVER( racedriv1 )	/* 136077           (c) 1990 */
	DRIVER( racedrivb1 )/* 136077           (c) 1990 */
	DRIVER( racedrivg1 )/* 136077           (c) 1990 */
	/* Race Drivin' Compact */
	DRIVER( racedrivc )		/* 136077           (c) 1990 */
	DRIVER( racedrivcb )	/* 136077           (c) 1990 */
	DRIVER( racedrivcg )	/* 136077           (c) 1990 */
	DRIVER( racedrivc4 )	/* 136077           (c) 1990 */
	DRIVER( racedrivcb4 )	/* 136077           (c) 1990 */
	DRIVER( racedrivcg4 )	/* 136077           (c) 1990 */
	DRIVER( racedrivc2 )	/* 136077           (c) 1990 */
	DRIVER( racedrivc1 )	/* 136077           (c) 1990 */
	/* Race Drivin' Panorama */
	DRIVER( racedrivpan )	/* 136077           (c) 1990 */
				/* 136086 - BMX Heat (proto) */
	/* Steel Talons */
	DRIVER( steeltal )	/* 136087           (c) 1990 */
	DRIVER( steeltalg )	/* 136087           (c) 1990 */
	DRIVER( steeltal1 )	/* 136087           (c) 1990 */
	DRIVER( steeltalp )	/* 136087           (c) 1990 */
				/* 136088 - Race Drivin' Panorama */
	/* Street Drivin' */
	DRIVER( strtdriv )	/* 136091 (proto) */
	/* Hard Drivin' Airbourne */
	DRIVER( hdrivair )	/* 136098 (proto) */
	DRIVER( hdrivairp )	/* 136098 (proto) */

	/* later Atari games */

	/* Gauntlet Hardware */
	/* Gauntlet */
	DRIVER( gauntlet )		/* 136037           (c) 1985 */
	DRIVER( gauntlets )		/* 136037           (c) 1985 */
	DRIVER( gauntletj )		/* 136037           (c) 1985 */
	DRIVER( gauntletg )		/* 136037           (c) 1985 */
	DRIVER( gauntletj12 )	/* 136037           (c) 1985 */
	DRIVER( gauntletr9 )	/* 136037           (c) 1985 */
	DRIVER( gauntletgr8 )	/* 136037           (c) 1985 */
	DRIVER( gauntletr7 )	/* 136037           (c) 1985 */
	DRIVER( gauntletgr6 )	/* 136037           (c) 1985 */
	DRIVER( gauntletr5 )	/* 136037           (c) 1985 */
	DRIVER( gauntletr4 )	/* 136037           (c) 1985 */
	DRIVER( gauntletgr3 )	/* 136037           (c) 1985 */
	DRIVER( gauntletr2 )	/* 136037           (c) 1985 */
	DRIVER( gauntletr1 )	/* 136037           (c) 1985 */
	/* Gauntlet - 2 Player */
	DRIVER( gauntlet2p )	/* 136037           (c) 1985 */
	DRIVER( gauntlet2pj )	/* 136037           (c) 1985 */
	DRIVER( gauntlet2pg )	/* 136037           (c) 1985 */
	DRIVER( gauntlet2pr3 )	/* 136037           (c) 1985 */
	DRIVER( gauntlet2pj2 )	/* 136037           (c) 1985 */
	DRIVER( gauntlet2pg1 )	/* 136037           (c) 1985 */
	/* Gauntlet 2 */
	DRIVER( gaunt2 )		/* 136043           (c) 1986 */
	DRIVER( gaunt2g )	/* 136043           (c) 1986 */
	/* Gauntlet 2 - 2 Player */
	DRIVER( gaunt22p )	/* 136043           (c) 1986 */
	DRIVER( gaunt22p1 )	/* 136043           (c) 1986 */
	DRIVER( gaunt22pg )	/* 136043           (c) 1986 */
	/* Vindicators Part II */
	DRIVER( vindctr2 )	/* 136059           (c) 1988 */
	DRIVER( vindctr2r2 )	/* 136059           (c) 1988 */
	DRIVER( vindctr2r1 )	/* 136059           (c) 1988 */

	/* Other Hardware */

	/* Xybots */
	DRIVER( xybots )	/* 136054           (c) 1987 */
	DRIVER( xybotsg )	/* 136054           (c) 1987 */
	DRIVER( xybotsf )	/* 136054           (c) 1987 */
	DRIVER( xybots1 )	/* 136054           (c) 1987 */
	DRIVER( xybots0 )	/* 136054           (c) 1987 */
	/* Blasteroids */
	DRIVER( blstroid )	/* 136057           (c) 1987 */
	DRIVER( blstroid3 )	/* 136057           (c) 1987 */
	DRIVER( blstroid2 )	/* 136057           (c) 1987 */
	DRIVER( blstroidg )	/* 136057           (c) 1987 */
	DRIVER( blstroidh )	/* (proto)          (c) 1987 */
	/* Vindicators */
	DRIVER( vindictr )	/* 136059           (c) 1988 */
	DRIVER( vindictre )	/* 136059           (c) 1988 */
	DRIVER( vindictrg )	/* 136059           (c) 1988 */
	DRIVER( vindictre4 )/* 136059           (c) 1988 */
	DRIVER( vindictr4 )	/* 136059           (c) 1988 */
	DRIVER( vindictre3 )/* 136059           (c) 1988 */
	DRIVER( vindictr2 )	/* 136059           (c) 1988 */
	DRIVER( vindictr1 )	/* 136059           (c) 1988 */
	/* Toobin */
	DRIVER( toobin )	/* 136061           (c) 1988 */
	DRIVER( toobine )	/* 136061           (c) 1988 */
	DRIVER( toobing )	/* 136061           (c) 1988 */
	DRIVER( toobin2 )	/* 136061           (c) 1988 */
	DRIVER( toobin2e )	/* 136061           (c) 1988 */
	DRIVER( toobin1 )	/* 136061           (c) 1988 */
	/* Cyberball */
	DRIVER( cyberbal )	/* 136064           (c) 1989 */
	DRIVER( cyberbal2 )	/* 136064           (c) 1989 */
	DRIVER( cyberbalp )	/* 136064           (c) 1989 */
	/* Atari Tetris */
	DRIVER( atetrisc )	/* 136066           (c) 1989 */
	DRIVER( atetrisc2 )	/* 136066           (c) 1989 */
	DRIVER( atetris )	/* 136066           (c) 1988 */
	DRIVER( atetrisa )	/* 136066           (c) 1988 */
	DRIVER( atetrisb )	/* (bootleg) */
	DRIVER( atetrisb2 )	/* (bootleg) */
	/* Escape from the Planet of Robot Monsters */
	DRIVER( eprom )		/* 136069           (c) 1989 */
	DRIVER( eprom2 )	/* 136069           (c) 1989 */
	DRIVER( guts )		/* prototype */
	/* Cyberball 2 Player */
	DRIVER( cyberbal2p )	/* 136071           (c) 1989 */
	DRIVER( cyberbal2p3 )	/* 136071           (c) 1989 */
	DRIVER( cyberbal2p2 )	/* 136071           (c) 1989 */
	DRIVER( cyberbal2p1 )	/* 136071           (c) 1989 */
	/* Skull and Crossbones */
	DRIVER( skullxbo )	/* 136072           (c) 1989 */
	DRIVER( skullxbo4 )	/* 136072           (c) 1989 */
	DRIVER( skullxbo3 )	/* 136072           (c) 1989 */
	DRIVER( skullxbo2 )	/* 136072           (c) 1989 */
	DRIVER( skullxbo1 )	/* 136072           (c) 1989 */
	/* Cyberball Tournament */
	DRIVER( cyberbalt )	/* 136073           (c) 1989 */
	DRIVER( cyberbalt1 )/* 136073           (c) 1989 */

	DRIVER( badlands )	/* 136074           (c) 1989 */
	DRIVER( badlandsb )	/* bootleg */
	DRIVER( klax )		/* 136075           (c) 1989 */
	DRIVER( klax2 )		/* 136075           (c) 1989 */
	DRIVER( klax3 )		/* 136075           (c) 1989 */
	DRIVER( klaxj )		/* 136075           (c) 1989 (Japan) */
	DRIVER( klaxd )		/* 136075           (c) 1989 (Germany) */
	DRIVER( klaxp1 )	/* prototype */
	DRIVER( klaxp2 )	/* prototype */
	DRIVER( thunderj )	/* 136076           (c) 1990 */

	DRIVER( hydra )		/* 136079           (c) 1990 */
	DRIVER( hydrap )	/* (proto)          (c) 1990 */
	DRIVER( hydrap2 )	/* (proto)          (c) 1990 */
	DRIVER( pitfight )	/* 136081           (c) 1990 */
	DRIVER( pitfight7 )	/* 136081           (c) 1990 */
	DRIVER( pitfight6 )	/* 136081           (c) 1990 */
	DRIVER( pitfight5 )	/* 136081           (c) 1990 */
	DRIVER( pitfight4 )	/* 136081           (c) 1990 */
	DRIVER( pitfight3 )	/* 136081           (c) 1990 */
	DRIVER( pitfightj )	/* Japan */
	DRIVER( pitfightb )	/* bootleg */
	DRIVER( rampart )	/* 136082           (c) 1990 */
	DRIVER( rampart2p )	/* 136082           (c) 1990 */
	DRIVER( rampartj )	/* 136082           (c) 1990 (Japan) */
	DRIVER( shuuz )		/* 136083           (c) 1990 */
	DRIVER( shuuz2 )	/* 136083           (c) 1990 */
				/* 136084 - Marble Man (proto) */
	DRIVER( batman )	/* 136085           (c) 1991 */
	DRIVER( roadriot )	/* 136089           (c) 1991 */
	DRIVER( offtwall )	/* 136090           (c) 1991 */
	DRIVER( offtwallc )	/* 136090           (c) 1991 */
	DRIVER( guardian )	/* 136092           (c) 1992 */
	DRIVER( relief )	/* 136093           (c) 1992 */
	DRIVER( relief2 )	/* 136093           (c) 1992 */
	DRIVER( arcadecl )	/* (proto)          (c) 1992 */
	DRIVER( sparkz )	/* (proto)          (c) 1992 */
	DRIVER( motofren )	/* 136094           (c) 1992 */
	DRIVER( motofrenmd )	/* 136094           (c) 1992 */
	DRIVER( motofrenft )	/* 136094           (c) 1992 */
	DRIVER( motofrenmf )	/* 136094           (c) 1992 */
	DRIVER( spclords )	/* 136095           (c) 1992 */
	DRIVER( spclordsb )	/* 136095           (c) 1992 */
	DRIVER( spclordsa )	/* 136095           (c) 1992 */
	DRIVER( spclordsg )	/* 136095           (c) 1992 */
	DRIVER( rrreveng )	/*     ??           (c) 1993 */
	DRIVER( rrrevenga )	/*     ??           (c) 1993 */
	DRIVER( rrrevengb )	/*     ??           (c) 1993 */
	DRIVER( beathead )	/* (proto)          (c) 1993 */
	DRIVER( tmek )		/* 136100           (c) 1994 */
	DRIVER( tmek51p )	/* 136100           (c) 1994 */
	DRIVER( tmek45 )	/* 136100           (c) 1994 */
	DRIVER( tmek44 )	/* 136100           (c) 1994 */
	DRIVER( tmek20 )	/* 136100           (c) 1994 */
	DRIVER( primrage )	/* 136102           (c) 1994 */
	DRIVER( primrage20 )/* 136102           (c) 1994 */
	DRIVER( metalmx )	/* 136103?          (c) 1994 */
	DRIVER( area51 )	/* 136105           (c) 1995 */
	DRIVER( area51t )	/* 136105           (c) 1995 */
	DRIVER( area51a )	/* 136105           (c) 1995 */
	DRIVER( fishfren )	/* (proto)          (c) 1995 */
	DRIVER( freezeat )	/* (proto)          (c) 1996 */
	DRIVER( freezeatjp )	/* (proto)          (c) 1996 */
	DRIVER( freezeat2 )	/* (proto)          (c) 1996 */
	DRIVER( freezeat3 )	/* (proto)          (c) 1996 */
	DRIVER( freezeat4 )	/* (proto)          (c) 1996 */
	DRIVER( freezeat5 )	/* (proto)          (c) 1996 */
	DRIVER( freezeat6 )	/* (proto)          (c) 1996 */
	DRIVER( area51mx )	/* 136105           (c) 1998 */
	DRIVER( a51mxr3k )	/*     ??           (c) 1998 */
	DRIVER( maxforce )	/*     ??           (c) 1996 */
	DRIVER( maxf_102 )	/*     ??           (c) 1996 */
	DRIVER( maxf_ng )	/*     ??           (c) 1996 */
	DRIVER( vcircle )	/* (proto)          (c) 1996 */
	DRIVER( a51site4 )	/*     ??           (c) 1998 */

	/* SNK / Rock-ola games */
	DRIVER( sasuke )	/* [1980] Shin Nihon Kikaku (SNK) */
	DRIVER( satansat )	/* (c) 1981 SNK */
	DRIVER( satansata )	/* (c) 1981 SNK */
	DRIVER( zarzon )	/* (c) 1981 Taito, gameplay says SNK */
	DRIVER( vanguard )	/* (c) 1981 SNK */
	DRIVER( vanguardc )	/* (c) 1981 SNK + Centuri */
	DRIVER( vanguardj )	/* (c) 1981 SNK */
	DRIVER( fantasy )	/* (c) 1981 SNK */
	DRIVER( fantasyu )	/* (c) 1981 Rock-Ola */
	DRIVER( fantasyj )	/* (c) 1981 SNK */
	DRIVER( pballoon )	/* (c) 1982 SNK */
	DRIVER( pballoonr )	/* (c) 1982 SNK (Rock-Ola license) */
	DRIVER( nibbler )	/* (c) 1982 Rock-ola */
	DRIVER( nibblera )	/* (c) 1982 Rock-ola */
	DRIVER( nibblerb )	/* (c) 1982 Rock-ola */
	DRIVER( nibblero )	/* (c) 1982 Olympia */

	/* later SNK games, each game can be identified by PCB code and ROM
    code, the ROM code is the same between versions, and usually based
    upon the Japanese title. */
	DRIVER( lasso )		/*       'WM' (c) 1982 */
	DRIVER( chameleo )	/* (c) 1983 Jaleco */
	DRIVER( wwjgtin )	/* (c) 1984 Jaleco / Casio */
	DRIVER( photof )	/* (c) 1984 Jaleco / Casio */
	DRIVER( pinbo )		/* (c) 1984 Jaleco */
	DRIVER( pinboa )	/* (c) 1984 Jaleco */
	DRIVER( pinbos )	/* (c) 1985 Strike */
	DRIVER( joyfulr )	/* A2001      (c) 1983 */
	DRIVER( mnchmobl )	/* A2001      (c) 1983 + Centuri license */
	DRIVER( marvins )	/* A2003      (c) 1983 */
	DRIVER( madcrash )	/* A2005      (c) 1984 */
	DRIVER( madcrush )	/* A2005      (c) 1984 */
	DRIVER( vangrd2 )	/* A2005      (c) 1984 */
	DRIVER( mainsnk )	/*            (c) 1984 */
	DRIVER( canvas )	/* A3008      (c) 1985 */
	DRIVER( jcross )	/* A3006      (c) 1984 */
	DRIVER( sgladiat )	/* A3006      (c) 1984 */
	DRIVER( hal21 )		/* A4031      (c) 1985 */
	DRIVER( hal21j )	/*            (c) 1985 (Japan) */
	DRIVER( aso )		/*            (c) 1985 */
	DRIVER( alphamis )	/*            (c) 1985 */
	DRIVER( arian )		/*            (c) 1985 */
	DRIVER( tnk3 )		/* A5001      (c) 1985 */
	DRIVER( tnk3j )		/* A5001      (c) 1985 */
	DRIVER( athena )	/*       'UP' (c) 1986 */
	DRIVER( fitegolf )	/*       'GU' (c) 1988 */
	DRIVER( fitegolfu )	/*       'GU' (c) 1988 */
	DRIVER( countryc )	/* A7004 'CC' (c) 1988 */
	DRIVER( ikari )		/* A5004 'IW' (c) 1986 */
	DRIVER( ikaria )	/* A5004 'IW' (c) 1986 */
	DRIVER( ikarinc )	/* A5004 'IW' (c) 1986 */
	DRIVER( ikarijp )	/* A5004 'IW' (c) 1986 (Japan) */
	DRIVER( ikarijpb )	/* bootleg */
	DRIVER( victroad )	/* A6002      (c) 1986 */
	DRIVER( dogosoke )	/* A6002      (c) 1986 */
	DRIVER( dogosokb )	/* bootleg */
	DRIVER( gwar )		/* A7003 'GV' (c) 1987 */
	DRIVER( gwarj )		/* A7003 'GV' (c) 1987 (Japan) */
	DRIVER( gwara )		/* A7003 'GV' (c) 1987 */
	DRIVER( gwarb )		/* bootleg */
	DRIVER( bermudat )	/*            (c) 1987 */
	DRIVER( bermudatj )	/*            (c) 1987 */
	DRIVER( bermudata )	/* A6004 'WW' (c) 1987 */
	DRIVER( worldwar )	/* A6004 'WW' (c) 1987 */
	DRIVER( psychos )	/*       'PS' (c) 1987 */
	DRIVER( psychosj )	/*       'PS' (c) 1987 (Japan) */
	DRIVER( chopper )	/* A7003 'KK' (c) 1988 */
	DRIVER( choppera )	/* A7003 'KK' (c) 1988 */
	DRIVER( chopperb )	/* A7003 'KK' (c) 1988 */
	DRIVER( legofair )	/* A7003 'KK' (c) 1988 */
	DRIVER( fsoccer )	/* A6006 'FS' (c) 1988 */
	DRIVER( fsoccerj )	/* A6006 'FS' (c) 1988 */
	DRIVER( fsoccerb )	/*            (c) 1988 */
	DRIVER( tdfever )	/* A6006 'TD' (c) 1987 */
	DRIVER( tdfeverj )	/* A6006 'TD' (c) 1987 */
	DRIVER( tdfever2 )	/* A6006 'TD'?(c) 1988 */
	DRIVER( ikari3 )	/* A7007 'IK3'(c) 1989 */
	DRIVER( ikari3nr )	/* A7007 'IK3'(c) 1989 */
	DRIVER( pow )		/* A7008 'DG' (c) 1988 */
	DRIVER( powj )		/* A7008 'DG' (c) 1988 */
	DRIVER( searchar )	/* A8007 'BH' (c) 1989 */
	DRIVER( searcharu )	/* A8007 'BH' (c) 1989 */
	DRIVER( searcharj )	/* A8007 'BH' (c) 1989 */
	DRIVER( streetsm )	/* A8007 'S2' (c) 1989 */
	DRIVER( streetsm1 )	/* A7008 'S2' (c) 1989 */
	DRIVER( streetsmw )	/*            (c) 1989 */
	DRIVER( streetsmj )	/* A8007 'S2' (c) 1989 */
	DRIVER( prehisle )	/* A8003 'GT' (c) 1989 */
	DRIVER( prehisleu )	/* A8003 'GT' (c) 1989 */
	DRIVER( gensitou )	/* A8003 'GT' (c) 1989 */
	DRIVER( mechatt )	/* A8002 'MA' (c) 1989 */
	DRIVER( mechattu )	/* A8002 'MA' (c) 1989 */
	DRIVER( bbusters )	/* A9003 'BB' (c) 1989 */
	DRIVER( bbustersu )	/* A9003 'BB' (c) 1989 */

	/* Electrocoin on early SNK hardware - gambling game remove once it works */
	DRIVER( dmndrby )	/* G4001 'DD' (c) 1986 */
	DRIVER( dmndrbya )	/* G4001 'DD' (c) 1986 */

	/* Neo Geo games */
	/* the four digits number is the game ID stored at address 0x0108 of the program ROM */
	/* info on prototypes taken from http://www.members.tripod.com/fresa/proto/puzzle.htm */
	DRIVER( neogeo )
	DRIVER( nam1975 )	/* 0001 (c) 1990 SNK */
	DRIVER( bstars )	/* 0002 (c) 1990 SNK */
	DRIVER( bstarsh )	/* 0002 (c) 1990 SNK */
	DRIVER( tpgolf )	/* 0003 (c) 1990 SNK */
	DRIVER( mahretsu )	/* 0004 (c) 1990 SNK */
	DRIVER( maglord )	/* 0005 (c) 1990 Alpha Denshi Co. */
	DRIVER( maglordh )	/* 0005 (c) 1990 Alpha Denshi Co. */
	DRIVER( ridhero )	/* 0006 (c) 1990 SNK */
	DRIVER( ridheroh )	/* 0006 (c) 1990 SNK */
	DRIVER( alpham2 )	/* 0007 (c) 1991 SNK */
						/* 0008 Sunshine (prototype) 1990 SNK */
	DRIVER( ncombat )	/* 0009 (c) 1990 Alpha Denshi Co. */
	DRIVER( ncombath )	/* 0009 (c) 1990 Alpha Denshi Co. */
	DRIVER( cyberlip )	/* 0010 (c) 1990 SNK */
	DRIVER( superspy )	/* 0011 (c) 1990 SNK */
						/* 0012 */
						/* 0013 */
	DRIVER( mutnat )	/* 0014 (c) 1992 SNK */
						/* 0015 */
	DRIVER( kotm )		/* 0016 (c) 1991 SNK */
	DRIVER( kotmh )		/* 0016 (c) 1991 SNK */
	DRIVER( sengoku )	/* 0017 (c) 1991 SNK */
	DRIVER( sengokuh )	/* 0017 (c) 1991 SNK */
	DRIVER( burningf )	/* 0018 (c) 1991 SNK */
	DRIVER( burningfh )	/* 0018 (c) 1991 SNK */
	DRIVER( lbowling )	/* 0019 (c) 1990 SNK */
	DRIVER( gpilots )	/* 0020 (c) 1991 SNK */
	DRIVER( gpilotsh )	/* 0020 (c) 1991 SNK */
	DRIVER( joyjoy )	/* 0021 (c) 1990 SNK */
	DRIVER( bjourney )	/* 0022 (c) 1990 Alpha Denshi Co. */
	DRIVER( quizdais )	/* 0023 (c) 1991 SNK */
	DRIVER( quizdaisk )	/* 0123 (c) 1991 SNK */
	DRIVER( lresort )	/* 0024 (c) 1992 SNK */
	DRIVER( eightman )	/* 0025 (c) 1991 SNK / Pallas */
						/* 0026 Fun Fun Brothers (prototype) 1991 Alpha */
	DRIVER( minasan )	/* 0027 (c) 1990 Monolith Corp. */
						/* 0028 Dunk Star (prototype) Sammy */
	DRIVER( legendos )	/* 0029 (c) 1991 SNK */
	DRIVER( 2020bb )	/* 0030 (c) 1991 SNK / Pallas */
	DRIVER( 2020bba )	/* 0030 (c) 1991 SNK / Pallas */
	DRIVER( 2020bbh )	/* 0030 (c) 1991 SNK / Pallas */
	DRIVER( socbrawl )	/* 0031 (c) 1991 SNK */
	DRIVER( socbrawlh )	/* 0031 (c) 1991 SNK */
	DRIVER( roboarmy )	/* 0032 (c) 1991 SNK */
	DRIVER( fatfury1 )	/* 0033 (c) 1991 SNK */
	DRIVER( fbfrenzy )	/* 0034 (c) 1992 SNK */
						/* 0035 Mystic Wand (prototype) 1991 Alpha */
	DRIVER( bakatono )	/* 0036 (c) 1991 Monolith Corp. */
	DRIVER( crsword )	/* 0037 (c) 1991 Alpha Denshi Co. */
	DRIVER( trally )	/* 0038 (c) 1991 Alpha Denshi Co. */
	DRIVER( kotm2 )		/* 0039 (c) 1992 SNK */
	DRIVER( sengoku2 )	/* 0040 (c) 1993 SNK */
	DRIVER( bstars2 )	/* 0041 (c) 1992 SNK */
	DRIVER( quizdai2 )	/* 0042 (c) 1992 SNK */
	DRIVER( 3countb )	/* 0043 (c) 1993 SNK */
	DRIVER( aof )		/* 0044 (c) 1992 SNK */
	DRIVER( samsho )	/* 0045 (c) 1993 SNK */
	DRIVER( samshoh )	/* 0045 (c) 1993 SNK */
	DRIVER( tophuntr )	/* 0046 (c) 1994 SNK */
	DRIVER( tophuntrh )	/* 0046 (c) 1994 SNK */
	DRIVER( fatfury2 )	/* 0047 (c) 1992 SNK */
	DRIVER( janshin )	/* 0048 (c) 1994 Aicom */
	DRIVER( androdun )	/* 0049 (c) 1992 Visco */
	DRIVER( ncommand )	/* 0050 (c) 1992 Alpha Denshi Co. */
	DRIVER( viewpoin )	/* 0051 (c) 1992 Sammy */
	DRIVER( ssideki )	/* 0052 (c) 1992 SNK */
	DRIVER( wh1 )		/* 0053 (c) 1992 Alpha Denshi Co. */
	DRIVER( wh1h )		/* 0053 (c) 1992 Alpha Denshi Co. */
	DRIVER( wh1ha )		/* 0053 (c) 1992 Alpha Denshi Co. */
						/* 0054 Crossed Swords 2  (CD only? not confirmed, MVS might exist) */
	DRIVER( kof94 )		/* 0055 (c) 1994 SNK */
	DRIVER( aof2 )		/* 0056 (c) 1994 SNK */
	DRIVER( aof2a )		/* 0056 (c) 1994 SNK */
	DRIVER( wh2 )		/* 0057 (c) 1993 ADK */
	DRIVER( fatfursp )	/* 0058 (c) 1993 SNK */
	DRIVER( fatfurspa )	/* 0058 (c) 1993 SNK */
	DRIVER( savagere )	/* 0059 (c) 1995 SNK */
	DRIVER( fightfev )	/* 0060 (c) 1994 Viccom */
	DRIVER( fightfeva )	/* 0060 (c) 1994 Viccom */
	DRIVER( ssideki2 )	/* 0061 (c) 1994 SNK */
	DRIVER( spinmast )	/* 0062 (c) 1993 Data East Corporation */
	DRIVER( samsho2 )	/* 0063 (c) 1994 SNK */
	DRIVER( samsho2k )	/* 0063 (c) 1994 SNK (Korean hack) */
	DRIVER( wh2j )		/* 0064 (c) 1994 ADK / SNK */
	DRIVER( wjammers )	/* 0065 (c) 1994 Data East Corporation */
	DRIVER( karnovr )	/* 0066 (c) 1994 Data East Corporation */
	DRIVER( gururin )	/* 0067 (c) 1994 Face */
	DRIVER( pspikes2 )	/* 0068 (c) 1994 Video System Co. */
						/* Super Volley '94 was once released in Mar.1994, and recalled. Then released as Power Spikes 2 (with some tweaks). */
	DRIVER( fatfury3 )	/* 0069 (c) 1995 SNK */
	DRIVER( zupapa )	/* 0070 Zupapa - released in 2001, 1994 prototype probably exists */
						/* 0071 Bang Bang Busters (prototype) 1994 Visco */
						/* 0072 Last Odyssey Pinball Fantasia (prototype) 1995 Monolith */
	DRIVER( panicbom )	/* 0073 (c) 1994 Eighting / Hudson */
	DRIVER( aodk )		/* 0074 (c) 1994 ADK / SNK */
	DRIVER( sonicwi2 )	/* 0075 (c) 1994 Video System Co. */
	DRIVER( zedblade )	/* 0076 (c) 1994 NMK */
						/* 0077 The Warlocks of the Fates (prototype) 1995 Astec */
	DRIVER( galaxyfg )	/* 0078 (c) 1995 Sunsoft */
	DRIVER( strhoop )	/* 0079 (c) 1994 Data East Corporation */
	DRIVER( quizkof )	/* 0080 (c) 1995 Saurus */
	DRIVER( quizkofk )	/* 0080 (c) 1995 Saurus */
	DRIVER( ssideki3 )	/* 0081 (c) 1995 SNK */
	DRIVER( doubledr )	/* 0082 (c) 1995 Technos */
	DRIVER( pbobblen )	/* 0083 (c) 1994 Taito */
	DRIVER( pbobblenb )	/* bootleg */
	DRIVER( kof95 )		/* 0084 (c) 1995 SNK */
	DRIVER( kof95h )	/* 0084 (c) 1995 SNK */
						/* 0085 Shinsetsu Samurai Spirits Bushidoretsuden / Samurai Shodown RPG (CD only) */
	DRIVER( tws96 )		/* 0086 (c) 1996 Tecmo */
	DRIVER( samsho3 )	/* 0087 (c) 1995 SNK */
	DRIVER( samsho3h )	/* 0087 (c) 1995 SNK */
	DRIVER( fswords )	/* 0187 Korean hack of samsho3 */
	DRIVER( stakwin )	/* 0088 (c) 1995 Saurus */
	DRIVER( pulstar )	/* 0089 (c) 1995 Aicom */
	DRIVER( whp )		/* 0090 (c) 1995 ADK / SNK */
						/* 0091 */
	DRIVER( kabukikl )	/* 0092 (c) 1995 Hudson */
	DRIVER( neobombe )	/* 0093 (c) 1997 Hudson */
	DRIVER( gowcaizr )	/* 0094 (c) 1995 Technos */
	DRIVER( rbff1 )		/* 0095 (c) 1995 SNK */
	DRIVER( rbff1a )	/* 0095 (c) 1995 SNK */
	DRIVER( aof3 )		/* 0096 (c) 1996 SNK */
	DRIVER( aof3k )		/* 0196 Censored Korean release of aof3 */
	DRIVER( sonicwi3 )	/* 0097 (c) 1995 Video System Co. */
						/* 0098 Idol Mahjong - final romance 2 (CD only? not confirmed, MVS might exist) */
						/* 0099 Neo Pool Masters */
	DRIVER( turfmast )	/* 0200 (c) 1996 Nazca */
	DRIVER( mslug )		/* 0201 (c) 1996 Nazca */
	DRIVER( puzzledp )	/* 0202 (c) 1995 Taito (Visco license) */
	DRIVER( mosyougi )	/* 0203 (c) 1995 ADK / SNK */
						/* 0204 QP (prototype) */
						/* 0205 Neo-Geo CD Special (CD only) */
	DRIVER( marukodq )	/* 0206 (c) 1995 Takara */
	DRIVER( neomrdo )	/* 0207 (c) 1996 Visco */
	DRIVER( sdodgeb )	/* 0208 (c) 1996 Technos */
	DRIVER( goalx3 )	/* 0209 (c) 1995 Visco */
						/* 0210 Karate Ninja Sho (prototype) 1995 Yumekobo */
						/* 0211 Oshidashi Zintrick (CD only? not confirmed, MVS might exist) 1996 SNK/ADK */
	DRIVER( zintrckb )	/* 0211 hack - this is not a genuine MVS proto, its a bootleg made from the CD version */
	DRIVER( overtop )	/* 0212 (c) 1996 ADK */
	DRIVER( neodrift )	/* 0213 (c) 1996 Visco */
	DRIVER( kof96 )		/* 0214 (c) 1996 SNK */
	DRIVER( kof96h )	/* 0214 (c) 1996 SNK */
	DRIVER( ssideki4 )	/* 0215 (c) 1996 SNK */
	DRIVER( kizuna )	/* 0216 (c) 1996 SNK */
						/*  Fu-un Super Tag Battle Special Version (4-player battle available) exists */
	DRIVER( ninjamas )	/* 0217 (c) 1996 ADK / SNK */
	DRIVER( ragnagrd )	/* 0218 (c) 1996 Saurus */
	DRIVER( pgoal )		/* 0219 (c) 1996 Saurus */
						/* 0220 (c) 1996 Saurus - Choutetsu Brikin'ger - Iron clad (undumped protoype) */
	DRIVER( magdrop2 )	/* 0221 (c) 1996 Data East Corporation */
	DRIVER( samsho4 )	/* 0222 (c) 1996 SNK */
	DRIVER( samsho4k )	/* Censored Korean release of samsho4 */
	DRIVER( rbffspec )	/* 0223 (c) 1996 SNK */
	DRIVER( rbffspeck )	/* 0124 (c) 1996 SNK */
	DRIVER( twinspri )	/* 0224 (c) 1996 ADK */
	DRIVER( wakuwak7 )	/* 0225 (c) 1996 Sunsoft */
						/* 0226 Pair Pair Wars (prototype) 1996 Sunsoft? */
	DRIVER( stakwin2 )	/* 0227 (c) 1996 Saurus */
	DRIVER( ghostlop )	/* 0228 GhostLop (prototype) 1996? Data East */
						/* 0229 King of Fighters '96 CD Collection (CD only) */
	DRIVER( breakers )	/* 0230 (c) 1996 Visco */
	DRIVER( miexchng )	/* 0231 (c) 1997 Face */
	DRIVER( kof97 )		/* 0232 (c) 1997 SNK */
	DRIVER( kof97h )	/* 0232 (c) 1997 SNK */
	DRIVER( kof97pls )	/* bootleg of kof97 */
	DRIVER( kog )		/* bootleg of kof97 */
	DRIVER( magdrop3 )	/* 0233 (c) 1997 Data East Corporation */
	DRIVER( lastblad )	/* 0234 (c) 1997 SNK */
	DRIVER( lastbladh )	/* 0234 (c) 1997 SNK */
	DRIVER( lastsold )	/* 0196 Censored Korean release of lastblad */
	DRIVER( puzzldpr )	/* 0235 (c) 1997 Taito (Visco license) */
	DRIVER( irrmaze )	/* 0236 (c) 1997 SNK / Saurus */
	DRIVER( popbounc )	/* 0237 (c) 1997 Video System Co. */
	DRIVER( shocktro )	/* 0238 (c) 1997 Saurus */
	DRIVER( shocktroa )	/* 0238 (c) 1997 Saurus */
	DRIVER( blazstar )	/* 0239 (c) 1998 Yumekobo */
	DRIVER( rbff2 )		/* 0240 (c) 1998 SNK */
	DRIVER( rbff2h )	/* 0240 (c) 1998 SNK */
	DRIVER( rbff2k )	/* 0140 Censored Korean release of rbff2 */
	DRIVER( mslug2 )	/* 0241 (c) 1998 SNK */
	DRIVER( kof98 )		/* 0242 (c) 1998 SNK */
	DRIVER( kof98k )	/* 0242 (c) 1998 SNK */
	DRIVER( kof98ka )	/* 0242 (c) 1998 SNK */
	DRIVER( kof98h )	/* 0242 (c) 1998 SNK */
	DRIVER( lastbld2 )	/* 0243 (c) 1998 SNK */
	DRIVER( neocup98 )	/* 0244 (c) 1998 SNK */
	DRIVER( breakrev )	/* 0245 (c) 1998 Visco */
	DRIVER( shocktr2 )	/* 0246 (c) 1998 Saurus */
	DRIVER( lans2004 )	/* bootleg of shocktr2 */
	DRIVER( flipshot )	/* 0247 (c) 1998 Visco */
	DRIVER( pbobbl2n )	/* 0248 (c) 1999 Taito (SNK license) */
	DRIVER( ctomaday )	/* 0249 (c) 1999 Visco */
	DRIVER( mslugx )	/* 0250 (c) 1999 SNK */
	DRIVER( kof99 )		/* 0251 (c) 1999 SNK */
	DRIVER( kof99h )	/* 0251 (c) 1999 SNK */
	DRIVER( kof99e )	/* 0251 (c) 1999 SNK */
	DRIVER( kof99k )	/* 0152 (c) 1999 SNK */
	DRIVER( kof99p )	/* 0251 (c) 1999 SNK */
	DRIVER( ganryu )	/* 0252 (c) 1999 Visco */
	DRIVER( garou )		/* 0253 (c) 1999 SNK */
	DRIVER( garouo )	/* 0253 (c) 1999 SNK */
	DRIVER( garoup )	/* 0253 (c) 1999 SNK */
	DRIVER( garoubl )	/* bootleg */
	DRIVER( s1945p )	/* 0254 (c) 1999 Psikyo */
	DRIVER( preisle2 )	/* 0255 (c) 1999 Yumekobo */
	DRIVER( mslug3 )	/* 0256 (c) 2000 SNK */
	DRIVER( mslug3h )	/* 0256 (c) 2000 SNK */
	DRIVER( mslug3b6 )	/* bootleg */
	DRIVER( kof2000 )	/* 0257 (c) 2000 SNK */
	DRIVER( kof2000n )	/* 0257 (c) 2000 SNK */
						/* 0258 SNK vs. Capcom? */
	DRIVER( bangbead )	/* 0259 (c) 2000 Visco */
	DRIVER( nitd )		/* 0260 (c) 2000 Eleven / Gavaking */
	DRIVER( nitdbl )	/* bootleg */
	DRIVER( sengoku3 )	/* 0261 (c) 2001 Noise Factory / SNK */
	DRIVER( kof2001 )	/* 0262 (c) 2001 Eolith / SNK */
	DRIVER( kof2001h )	/* 0262 (c) 2001 Eolith / SNK */
	DRIVER( cthd2003 )	/* bootleg of kof2001 */
	DRIVER( ct2k3sp )	/* bootleg of kof2001 */
	DRIVER( ct2k3sa )	/* bootleg of kof2001 */
	DRIVER( mslug4 )	/* 0263 (c) 2002 Mega Enterprise */
	DRIVER( mslug4h )	/* 0263 (c) 2002 Mega Enterprise */
	DRIVER( ms4plus )	/* bootleg */
	DRIVER( rotd )		/* 0264 (c) 2002 Evoga */
	DRIVER( kof2002 )	/* 0265 (c) 2002 Eolith / Playmore */
	DRIVER( kof2002b )	/* bootleg */
	DRIVER( kf2k2pls )	/* bootleg */
	DRIVER( kf2k2pla )	/* bootleg */
	DRIVER( kf2k2mp )	/* bootleg */
	DRIVER( kf2k2mp2 )	/* bootleg */
	DRIVER( kof10th )	/* bootleg of kof2002 */
	DRIVER( kf2k5uni )	/* bootleg of kof2002 */
	DRIVER( kf10thep )	/* bootleg of kof2002 */
	DRIVER( kof2k4se )	/* bootleg of kof2002 */
	DRIVER( matrim )	/* 0266 (c) 2002 Atlus */
	DRIVER( matrimbl )	/* bootleg */
	DRIVER( pnyaa )		/* 0267 (c) 2003 Aiky / Taito */
	DRIVER( ms5pcb )	/* 0268 (c) 2003 Playmore */
	DRIVER( mslug5 )	/* 0268 (c) 2003 Playmore */
	DRIVER( mslug5h )	/* 0268 (c) 2003 Playmore */
	DRIVER( ms5plus )	/* bootleg */
	DRIVER( svcpcb )	/* 0269 (c) 2003 Playmore / Capcom - JAMMA PCB */
	DRIVER( svcpcba )	/* 0269 (c) 2003 Playmore / Capcom - JAMMA PCB */
	DRIVER( svc )		/* 0269 (c) 2003 Playmore / Capcom */
	DRIVER( svcboot )	/* bootleg */
	DRIVER( svcplus )	/* bootleg */
	DRIVER( svcplusa )	/* bootleg */
	DRIVER( svcsplus )	/* bootleg */
	DRIVER( samsho5 )	/* 0270 (c) 2003 Playmore */
	DRIVER( samsho5h )	/* 0270 (c) 2003 Playmore */
	DRIVER( samsho5b )	/* bootleg */
	DRIVER( kf2k3pcb )	/* 0271 (c) 2003 Playmore - JAMMA PCB */
	DRIVER( kof2003 )	/* 0271 (c) 2003 Playmore */
	DRIVER( kof2003h )	/* 0271 (c) 2003 Playmore */
	DRIVER( kf2k3bl )	/* bootleg */
	DRIVER( kf2k3bla )	/* bootleg */
	DRIVER( kf2k3pl )	/* bootleg */
	DRIVER( kf2k3upl )	/* bootleg */
	DRIVER( samsh5sp )	/* 0272 (c) 2004 Playmore */
	DRIVER( samsh5sph )	/* 0272 (c) 2004 Playmore */
	DRIVER( samsh5spn )	/* 0272 (c) 2004 Playmore */

	/* The BrezzaSoft games don't have proper ID codes */
	DRIVER( jockeygp )
	DRIVER( vliner )
	DRIVER( vlinero )

	/* Nor does Digger Man */
	DRIVER( diggerma )  /* No Game ID (unlicensed), (c) 2000 Kyle Hodgetts, prototype */

	/* Neo Print */
	DRIVER( 98best44 )

	/* Hyper NeoGeo 64 uses a 3 digit rom code? */

	DRIVER( hng64 )
	DRIVER( roadedge )	/* 001 1997/09/10 Roads Edge / Round Trip RV */
	DRIVER( sams64 )	/* 002 1997/12/19 Samurai Shodown 64 / Samurai Spirits 64 */
	DRIVER( xrally )	/* 003 1998/05/13 Xtreme Rally / Off Beat Racer! */
	DRIVER( bbust2 )	/* 004 1998/09/11 Beast Busters: Second Nightmare */
	DRIVER( sams64_2 )	/* 005 1998/10/16 Samurai Shodown: Warrior's Rage / Samurai Spirits: Asura Zanmaden */
	DRIVER( fatfurwa )	/* 006 1999/01/28 Fatal Fury: Wild Ambition / Garou Densetsu: Wild Ambition */
	DRIVER( buriki )	/* 007 1999/05/21 Buriki One */

	/* Alpha Denshi games */
	DRIVER( shougi )	/* (c) 1982 (Arcade TV Game List - P.80, Left, 9 from top) */
	DRIVER( shougi2 )	/* (c) 1982 (Arcade TV Game List - P.80, Left, 10 from top) */
	DRIVER( champbas )	/* (c) 1983 Sega */
	DRIVER( champbasj )	/* (c) 1983 Alpha Denshi Co. */
	DRIVER( champbasja )/* (c) 1983 Alpha Denshi Co. */
	DRIVER( champbb2 )	/* (c) 1983 Sega */
	DRIVER( champbb2a )	/* (c) 1983 ? */
	DRIVER( champbb2j )	/* (c) 1983 ? */
	DRIVER( exctsccr )	/* (c) 1983 Alpha Denshi Co. */
	DRIVER( exctsccra )	/* (c) 1983 Alpha Denshi Co. */
	DRIVER( exctsccrj )	/* (c) 1983 Alpha Denshi Co. */
	DRIVER( exctsccrj2 )/* (c) 1983 Alpha Denshi Co. */
	DRIVER( exctsccrb )	/* bootleg */
	DRIVER( exctscc2 )	/* (c) 1984 Alpha Denshi Co. */
	DRIVER( talbot )	/* Volt Electronics */
	DRIVER( equites )	/* (c) 1984 Alpha Denshi Co. */
	DRIVER( equitess )	/* (c) 1984 Alpha Denshi Co./Sega */
	DRIVER( gekisou )	/* (c) 1985 Eastern */
	DRIVER( bullfgtr )	/* (c) 1984 Alpha Denshi Co. */
	DRIVER( bullfgtrs )	/* (c) 1984 Alpha Denshi Co./Sega */
	DRIVER( kouyakyu )	/* (c) 1985 Alpha Denshi Co. */
	DRIVER( splndrbt )	/* (c) 1985 Alpha Denshi Co. */
	DRIVER( hvoltage )	/* (c) 1985 Alpha Denshi Co. */

	/* SNK / Alpha 68K games */
	DRIVER( sstingry )	/* (c) 1986 Alpha Denshi Co. */
	DRIVER( kyros )		/* (c) 1987 World Games */
	DRIVER( kyrosj )	/* (c) 1986 Alpha Denshi Co. */
	DRIVER( jongbou )	/* (c) 1987 SNK */
	DRIVER( paddlema )	/* Alpha-68K96I  'PM' (c) 1988 SNK */
	DRIVER( timesold )	/* Alpha-68K96II 'BT' (c) 1987 SNK / Romstar */
	DRIVER( timesold1 )	/* Alpha-68K96II 'BT' (c) 1987 */
	DRIVER( btlfield )	/* Alpha-68K96II 'BT' (c) 1987 */
	DRIVER( btlfieldb )	/* bootleg */
	DRIVER( skysoldr )	/* Alpha-68K96II 'SS' (c) 1988 SNK (Romstar with dip switch) */
	DRIVER( goldmedl )	/* Alpha-68K96II 'GM' (c) 1988 SNK */
	DRIVER( goldmedla )	/* Alpha-68K96II 'GM' (c) 1988 SNK */
	DRIVER( goldmedlb )	/* Alpha-68K96II bootleg */
	DRIVER( skyadvnt )	/* Alpha-68K96V  'SA' (c) 1989 Alpha Denshi Co. */
	DRIVER( skyadvntu )	/* Alpha-68K96V  'SA' (c) 1989 SNK of America licensed from Alpha */
	DRIVER( skyadvntj )	/* Alpha-68K96V  'SA' (c) 1989 Alpha Denshi Co. */
	DRIVER( gangwars )	/* Alpha-68K96V       (c) 1989 Alpha Denshi Co. */
	DRIVER( gangwarsu )	/* Alpha-68K96V       (c) 1989 Alpha Denshi Co. */
	DRIVER( sbasebal )	/* Alpha-68K96V       (c) 1989 SNK of America licensed from Alpha */
	DRIVER( tnextspc )	/* A8003 'NS' (c) 1989 */
	DRIVER( tnextspcj )	/* A8003 'NS' (c) 1989 */

	DRIVER( meijinsn )	/* (c) 1986 SNK */

	/* Technos games */
	DRIVER( scregg )	/* TA-0001 (c) 1983 */
	DRIVER( eggs )		/* TA-0002 (c) 1983 Universal USA */
	DRIVER( dommy )		/* TA-00?? (c) 1983 (Arcade TV Game List - P.90, Left, 13 from top) */
	DRIVER( bigprowr )	/* TA-0007 (c) 1983 */
	DRIVER( tagteam )	/* TA-0007 (c) 1983 + Data East license */
	DRIVER( ssozumo )	/* TA-0008 (c) 1984 */
	DRIVER( mystston )	/* TA-0010 (c) 1984 */
	DRIVER( myststono )	/* TA-0010 (c) 1984 */
	DRIVER( dogfgt )	/* TA-0011 (c) 1984 */
	DRIVER( dogfgtj )	/* TA-0011 (c) 1984 */
	DRIVER( bogeyman )	/* -0204-0 (Data East part number) (c) 1985 (Arcade TV Game List - P.96, Right, 8 from top) */
	DRIVER( matmania )	/* TA-0015 (c) 1985 + Taito America license */
	DRIVER( excthour )	/* TA-0015 (c) 1985 + Taito license */
	DRIVER( maniach )	/* TA-0017 (c) 1986 + Taito America license */
	DRIVER( maniach2 )	/* TA-0017 (c) 1986 + Taito America license */
	DRIVER( renegade )	/* TA-0018 (c) 1986 + Taito America license */
	DRIVER( kuniokun )	/* TA-0018 (c) 1986 */
	DRIVER( kuniokunb )	/* bootleg */
	DRIVER( xsleena )	/* TA-0019 (c) 1986 */
	DRIVER( xsleenab )	/* bootleg */
	DRIVER( solarwar )	/* TA-0019 (c) 1986 Taito + Memetron license */
	DRIVER( battlane )	/* -0215, -0216 (Data East part number) (c) 1986 + Taito license */
	DRIVER( battlane2 )	/* -0215, -0216 (Data East part number) (c) 1986 + Taito license */
	DRIVER( battlane3 )	/* -0215, -0216 (Data East part number) (c) 1986 + Taito license */
	DRIVER( ddragon )	/* TA-0021 (c) 1987 */
	DRIVER( ddragonu )	/* TA-0021 (c) 1987 Taito America */
	DRIVER( ddragonua )	/* TA-0021 (c) 1987 Taito America */
	DRIVER( ddragonub )	/* TA-0021 (c) 1987 Taito America */
	DRIVER( ddragonw )	/* TA-0021 (c) 1987 Taito */
	DRIVER( ddragonw1 )	/* TA-0021 (c) 1987 Taito */
	DRIVER( ddragonb )	/* bootleg */
	DRIVER( ddragonba )	/* bootleg */
	DRIVER( ddragon6809 )	/* bootleg */
	DRIVER( ddragon6809a )	/* bootleg */
	DRIVER( ddragonb2 )	/* bootleg */
	DRIVER( spdodgeb )	/* TA-0022 (c) 1987 */
	DRIVER( nkdodge )	/* TA-0022 (c) 1987 (Japan) */
	DRIVER( nkdodgeb )	/* TA-0022 (c) 1987 (Japan) */
	DRIVER( chinagat )	/* TA-0023 (c) 1988 Taito + Romstar license (US) */
	DRIVER( saiyugou )	/* TA-0023 (c) 1988 (Japan) */
	DRIVER( saiyugoub1 )	/* bootleg */
	DRIVER( saiyugoub2 )	/* bootleg */
	DRIVER( wwfsstar )	/* TA-0024 (c) 1989 (Europe) */
	DRIVER( wwfsstara )	/* TA-0024 (c) 1989 (US, newer) */
	DRIVER( wwfsstaru )	/* TA-0024 (c) 1989 (US) */
	DRIVER( wwfsstarj )	/* TA-0024 (c) 1989 (Japan) */
	DRIVER( vball )		/* TA-0025 (c) 1988 (US) */
	DRIVER( vball2pj )	/* TA-0025 (c) 1988 (Japan) */
	DRIVER( vballb )	/* TA-0025 (c) 1988 */
	DRIVER( ddragon2 )	/* TA-0026 (c) 1988 (World) */
	DRIVER( ddragon2u )	/* TA-0026 (c) 1988 (US) */
	DRIVER( toffy )		/* (c) 1993 Midas */
	DRIVER( stoffy )	/* (c) 1994 Midas + Unico */
	DRIVER( ddungeon )	/* Game Room */
	DRIVER( ddungeone )	/* East Coast Coin Company (Melbourne) */
	DRIVER( darktowr )	/* Game Room */
	DRIVER( tstrike )	/* East Coast Coin Company (Melbourne) */
	DRIVER( tstrikea )	/* Game Room  */
	DRIVER( ctribe )	/* TA-0028 (c) 1990 (US) */
	DRIVER( ctribe1 )	/* TA-0028 (c) 1990 (US) - Set 1? */
	DRIVER( ctribej )	/* TA-0028 (c) 1990 (Japan) */
	DRIVER( ctribeb )	/* bootleg */
	DRIVER( ctribeb2 )	/* bootleg */
	DRIVER( blockout )	/* TA-0029 (c) 1989 + California Dreams */
	DRIVER( blockout2 )	/* TA-0029 (c) 1989 + California Dreams */
	DRIVER( blockoutj )	/* TA-0029 (c) 1989 + California Dreams (Japan) */
	DRIVER( agress )	/* TA-0029 (c) 1991 Palco */
	DRIVER( agressb )	/* TA-0029 (c) 1991 Palco */
	DRIVER( ddragon3 )	/* TA-0030 (c) 1990 */
	DRIVER( ddragon3p )	/* TA-0030 (c) 1990 */
	DRIVER( ddragon3j )	/* TA-0030 (c) 1990 (Japan) */
	DRIVER( ddragon3b )	/* bootleg */
	DRIVER( wwfwfest )	/* TA-0031 (c) 1991 (US) */
	DRIVER( wwfwfesta )	/* TA-0031 (c) 1991 + Tecmo license (US) */
	DRIVER( wwfwfestb )	/* bootleg */
	DRIVER( wwfwfestj )	/* TA-0031 (c) 1991 (Japan) */
	DRIVER( shadfrce )	/* TA-0032 (c) 1993 (US) */
	DRIVER( shadfrcej )	/* TA-0032 (c) 1993 (Japan) */
	DRIVER( shadfrcejv2 )/* TA-0032 (c) 1993 (Japan) */

	/* Stern "Berzerk hardware" games */
	DRIVER( berzerk )	/* (c) 1980 */
	DRIVER( berzerk1 )	/* (c) 1980 */
	DRIVER( berzerkg )	/* (c) 1980 */
	DRIVER( frenzy )	/* (c) 1982 */
	DRIVER( moonwarp )	/* (c) 1982 - prototype */

	/* Stern 'Mazer Blazer hardware' games */
	DRIVER( mazerbla )	/* (c) 1983 */
	DRIVER( mazerblaa )	/* (c) 1983 */
	DRIVER( greatgun )	/* (c) 1983 */

	/* Other Stern */
	DRIVER( cliffhgr )	/* (c) 1983 */
	DRIVER( cliffhgra )	/* (c) 1983 */
	DRIVER( goaltogo )	/* (c) 1983 */

	/* GamePlan games */
	DRIVER( toratora )	/* (c) 1980 Game Plan */
	DRIVER( megatack )	/* (c) 1980 Centuri */
	DRIVER( killcom )	/* (c) 1980 Centuri */
	DRIVER( challeng )	/* (c) 1981 Centuri */
	DRIVER( kaos )		/* (c) 1981 */

	/* Zaccaria games */
	DRIVER( sia2650 )	/* (c) 1978 */
	DRIVER( tinv2650 )	/* (c) 1978 */
	DRIVER( dodgem )	/* (c) 1979 */
	DRIVER( galaxia )	/* (c) 1979 */
	DRIVER( astrowar )	/* (c) 1980 */
	DRIVER( laserbat )	/* (c) 1981 */
	DRIVER( lazarian )	/* (c) 1981 */
	DRIVER( catnmous )	/* (c) 1982 */
	DRIVER( catnmousa )	/* (c) 1982 */
	DRIVER( monymony )	/* (c) 1983 */
	DRIVER( jackrabt )	/* (c) 1984 */
	DRIVER( jackrabt2 )	/* (c) 1984 */
	DRIVER( jackrabts )	/* (c) 1984 */

	/* UPL games */
	DRIVER( mouser )	/* UPL-83001 (c) 1983 */
	DRIVER( mouserc )	/* UPL-83001 (c) 1983 */
	DRIVER( nova2001 )	/* UPL-83005 (c) 1983 */
	DRIVER( nova2001u )	/* UPL-83005 (c) [1983] + Universal license */
	DRIVER( ninjakun )	/* UPL-84003 (c) 1984 Taito Corporation */
	DRIVER( raiders5 )	/* UPL-85004 (c) 1985 */
	DRIVER( raiders5t )	/* UPL-85004 (c) 1985 Taito license */
	DRIVER( pkunwar )	/* UPL-????? (c) 1985 (Arcade Game TV List - P.98, Left, 22 from top) */
	DRIVER( pkunwarj )	/* UPL-????? (c) 1985 (Arcade Game TV List - P.98, Left, 22 from top) */
	DRIVER( xxmissio )	/* UPL-86001 [1986] */
	DRIVER( ninjakd2 )	/* UPL-????? (c) 1987 */
	DRIVER( ninjakd2a )	/* UPL-????? (c) 1987 */
	DRIVER( ninjakd2b )	/* UPL-????? (c) 1987 */
	DRIVER( rdaction )	/* UPL-87003?(c) 1987 + World Games license */
	DRIVER( mnight )	/* UPL-????? (c) 1987 distributed by Kawakus */
	DRIVER( arkarea )	/* UPL-87007 (c) 1988 (Arcade TV Game List - P.67, Right, 2 from top)*/
	DRIVER( robokid )	/* UPL-88013 (c) 1988 */
	DRIVER( robokidj )	/* UPL-88013 (c) 1988 */
	DRIVER( robokidj2 )	/* UPL-88013 (c) 1988 */
	DRIVER( omegaf )	/* UPL-89016 (c) 1989 */
	DRIVER( omegafs )	/* UPL-89016 (c) 1989 */

	/* UPL/NMK/Banpresto games */
	DRIVER( urashima )	/* UPL-89052 */
	DRIVER( tharrier )	/* UPL-89053 (c) 1989 UPL + American Sammy license */
	DRIVER( tharrierj )	/* UPL-89053 (c) 1989 UPL (Japan) */
	DRIVER( mustang )	/* UPL-90058 (c) 1990 UPL */
	DRIVER( mustangs )	/* UPL-90058 (c) 1990 UPL + Seoul Trading */
	DRIVER( bioship )	/* UPL-90062 (c) 1990 UPL + American Sammy license */
	DRIVER( sbsgomo )	/* UPL-90062 (c) 1990 UPL */
	DRIVER( vandyke )	/* UPL-90064 (c) UPL */
	DRIVER( vandykejal )	/* UPL-90064 (c) Jaleco */
	DRIVER( vandykejal2 )	/* UPL-90064 (c) Jaleco */
	DRIVER( vandykeb )	/* bootleg */
	DRIVER( blkheart )	/* UPL-91069 */
	DRIVER( blkheartj )	/* UPL-91069 */
	DRIVER( acrobatm )	/* UPL-91073 (c) 1991 UPL + Taito license */
	DRIVER( strahl )	/* UPL-91074 (c) 1992 UPL (Japan) */
	DRIVER( strahla )	/* UPL-91074 (c) 1992 UPL (Japan) */
	DRIVER( bjtwin )	/* UPL-93087 (c) 1993 NMK */
	DRIVER( bjtwina )	/* UPL-93087 (c) 1993 NMK */
	DRIVER( tdragon2 )	/* UPL-93091 (c) 1993 NMK */
	DRIVER( tdragon2a )	/* UPL-93091 (c) 1993 NMK */
	DRIVER( bigbang )	/* UPL-93091 (c) 1993 NMK */
	DRIVER( tdragon )	/* (c) 1991 NMK / Tecmo */
	DRIVER( tdragon1 )	/* (c) 1991 NMK / Tecmo */
	DRIVER( hachamf )	/* (c) 1991 NMK */
	DRIVER( macross )	/* (c) 1992 Banpresto */
	DRIVER( riot )		/* (c) 1992 NMK */
	DRIVER( gunnail )	/* (c) 1993 NMK / Tecmo */
	DRIVER( macross2 )	/* (c) 1993 Banpresto */
	DRIVER( sabotenb )	/* (c) 1992 NMK / Tecmo */
	DRIVER( sabotenba )	/* (c) 1992 NMK / Tecmo */
	DRIVER( cactus )	/* bootleg */
	DRIVER( nouryoku )	/* (c) 1995 Tecmo */
	DRIVER( raphero )	/* (c) 1994 Media Trading Corp */

	/* nmk16.c bootlegs using the Seibu Sound System */
	DRIVER( mustangb )	/* bootleg */
	DRIVER( mustangb2 )	/* TAB Austria bootleg */
	DRIVER( tdragonb )	/* bootleg */

	/* Comad games on unofficial hardware based on nmk16.c */
	DRIVER( ssmissin )	/* (c) 1992 Comad */
	DRIVER( airattck )	/* (c) 1996 Comad */
	DRIVER( airattcka )	/* (c) 1996 Comad */

	/* Bee-Oh games on unofficial hardware based on nmk16.c */
	DRIVER( manybloc )	/* (c) 1991 Bee-Oh */

	/* Jaleco Mahjong Games, Similar Hardware to the NMK ones above? */
	DRIVER( daireika )	/* (c) 1989 Jaleco/NMK */
	DRIVER( mjzoomin )	/* (c) 1990 */
	DRIVER( kakumei )	/* (c) 1990 */
	DRIVER( kakumei2 )	/* (c) 1992 */
	DRIVER( suchipi )	/* (c) 1993 */

	/* NMK Double Dealer, Similar Hardware to Jaleco Mahjong games above */
	DRIVER( ddealer )	/* (c) 1991 NMK */

	/* Based on nmk16? */
	DRIVER( quizpani )	/* (c) 1993 NMK */
	DRIVER( acommand )	/* (c) 1990 */

	/* don't know what hardare Banpresto used for these games */
	DRIVER( macrossp )	/* (c) 1996 Banpresto */
	DRIVER( quizmoon )	/* (c) 1997 Banpresto */

	/* Face/NMK games */
	DRIVER( gakupara )	/* (c) 1991 NMK */
	DRIVER( quizdna )	/* (c) 1992 Face */
	DRIVER( gekiretu )	/* (c) 1992 Face */
	DRIVER( cultures )	/* (c) 1994 Face */

	/* Williams/Midway TMS games */
	DRIVER( narc )		/* (c) 1988 Williams */
	DRIVER( narc3 )		/* (c) 1988 Williams */
	DRIVER( narc2 )		/* (c) 1988 Williams */
	DRIVER( trog )		/* (c) 1990 Midway */
	DRIVER( trog4 )		/* (c) 1990 Midway */
	DRIVER( trog3 )		/* (c) 1990 Midway */
	DRIVER( trogpa6 )	/* (c) 1990 Midway */
	DRIVER( trogpa4 )	/* (c) 1990 Midway */
	DRIVER( smashtv )	/* (c) 1990 Williams */
	DRIVER( smashtv6 )	/* (c) 1990 Williams */
	DRIVER( smashtv5 )	/* (c) 1990 Williams */
	DRIVER( smashtv4 )	/* (c) 1990 Williams */
	DRIVER( smashtv3 )	/* (c) 1990 Williams */
	DRIVER( hiimpact )	/* (c) 1990 Williams */
	DRIVER( hiimpact4 )	/* (c) 1990 Williams */
	DRIVER( hiimpact3 )	/* (c) 1990 Williams */
	DRIVER( hiimpact2 )	/* (c) 1990 Williams */
	DRIVER( hiimpact1 )	/* (c) 1990 Williams */
	DRIVER( hiimpactp )	/* (c) 1990 Williams */
	DRIVER( shimpact )	/* (c) 1991 Midway */
	DRIVER( shimpactp6 )	/* (c) 1991 Midway */
	DRIVER( shimpactp5 )	/* (c) 1991 Midway */
	DRIVER( shimpactp4 )	/* (c) 1991 Midway */
	DRIVER( strkforc )	/* (c) 1991 Midway */
	DRIVER( mk )		/* (c) 1992 Midway */
	DRIVER( mkr4 )		/* (c) 1992 Midway */
	DRIVER( mktturbo )	/* hack */
	DRIVER( mkyturbo )	/* hack */
	DRIVER( mkprot8 )	/* (c) 1992 Midway */
	DRIVER( mkprot9 )	/* (c) 1992 Midway */
	DRIVER( mkla1 )		/* (c) 1992 Midway */
	DRIVER( mkla2 )		/* (c) 1992 Midway */
	DRIVER( mkla3 )		/* (c) 1992 Midway */
	DRIVER( mkla4 )		/* (c) 1992 Midway */
	DRIVER( mkyawdim )	/* bootleg */
	DRIVER( term2 )		/* (c) 1992 Midway */
	DRIVER( term2la3 )	/* (c) 1992 Midway */
	DRIVER( term2la2 )	/* (c) 1992 Midway */
	DRIVER( term2la1 )	/* (c) 1992 Midway */
	DRIVER( totcarn )	/* (c) 1992 Midway */
	DRIVER( totcarnp )	/* (c) 1992 Midway */
	DRIVER( mk2 )		/* (c) 1993 Midway */
	DRIVER( mk2r31e )	/* (c) 1993 Midway */
	DRIVER( mk2r32 )	/* (c) 1993 Midway */
	DRIVER( mk2r30 )	/* (c) 1993 Midway */
	DRIVER( mk2r21 )	/* (c) 1993 Midway */
	DRIVER( mk2r14 )	/* (c) 1993 Midway */
	DRIVER( mk2r42 )	/* hack */
	DRIVER( mk2r91 )	/* hack */
	DRIVER( mk2chal )	/* hack */
	DRIVER( jdreddp )	/* (c) 1993 Midway */
	DRIVER( nbajam )	/* (c) 1993 Midway */
	DRIVER( nbajamr2 )	/* (c) 1993 Midway */
	DRIVER( nbajamte )	/* (c) 1994 Midway */
	DRIVER( nbajamt1 )	/* (c) 1994 Midway */
	DRIVER( nbajamt2 )	/* (c) 1994 Midway */
	DRIVER( nbajamt3 )	/* (c) 1994 Midway */
	DRIVER( revx )		/* (c) 1994 Midway */
	DRIVER( mk3 )		/* (c) 1994 Midway */
	DRIVER( mk3r20 )	/* (c) 1994 Midway */
	DRIVER( mk3r10 )	/* (c) 1994 Midway */
	DRIVER( mk3p40 )	/* (c) 1994 Midway */
	DRIVER( umk3 )		/* (c) 1994 Midway */
	DRIVER( umk3r11 )	/* (c) 1994 Midway */
	DRIVER( umk3r10 )	/* (c) 1994 Midway */
	DRIVER( wwfmania )	/* (c) 1995 Midway */
	DRIVER( wwfmaniab )	/* (c) 1995 Midway */
	DRIVER( openice )	/* (c) 1995 Midway */
	DRIVER( nbahangt )	/* (c) 1996 Midway */
	DRIVER( nbamht )	/* (c) 1996 Midway */
	DRIVER( nbamht1 )	/* (c) 1996 Midway */
	DRIVER( rmpgwt )	/* (c) 1997 Midway */
	DRIVER( rmpgwt11 )	/* (c) 1997 Midway */

	/* Midway V-unit games */
	DRIVER( crusnusa )	/* (c) 1994 Midway */
	DRIVER( crusnusa40 )	/* (c) 1994 Midway */
	DRIVER( crusnusa21 )	/* (c) 1994 Midway */
	DRIVER( crusnwld )	/* (c) 1996 Midway */
	DRIVER( crusnwld23 )	/* (c) 1996 Midway */
	DRIVER( crusnwld20 )	/* (c) 1996 Midway */
	DRIVER( crusnwld17 )	/* (c) 1996 Midway */
	DRIVER( crusnwld13 )	/* (c) 1996 Midway */
	DRIVER( offroadc )	/* (c) 1997 Midway (v1.63) */
	DRIVER( offroadc4 )	/* (c) 1997 Midway (v1.40) */
	DRIVER( offroadc3 )	/* (c) 1997 Midway (v1.30) */
	DRIVER( offroadc1 )	/* (c) 1997 Midway (v1.10) */
	DRIVER( wargods )	/* (c) 1996 Midway */

	/* Midway Zeus games */
	DRIVER( mk4 )		/* (c) 1997 Midway (v3.0) Latest offical release */
	DRIVER( mk4a )		/* (c) 1997 Midway (v2.1) */
	DRIVER( mk4b )		/* (c) 1997 Midway (v1.0) */
	DRIVER( invasnab )	/* (c) 1999 Midway (v5.0) Latest offical release */
	DRIVER( invasnv4 )	/* (c) 1999 Midway (v4.0) */
	DRIVER( crusnexo )	/* (c) 1999 Midway (v2.4) Latest offical release */
	DRIVER( crusnexoa )	/* (c) 1999 Midway (v2.0) */
	DRIVER( crusnexob )	/* (c) 1999 Midway (v1.6) */
	DRIVER( thegrid )	/* (c) 2001 Midway (v1.2) Latest offical release */
	DRIVER( thegrida )	/* (c) 2001 Midway (v1.1) */

	/* Midway / Atari  Voodoo based Hardware (Seattle, Flagstaff) */
	DRIVER( wg3dh )		/* (c) 1996 Atari Games */
	DRIVER( mace )		/* (c) 1996 Atari Games */
	DRIVER( macea )		/* (c) 1996 Atari Games */
	DRIVER( sfrush )	/* (c) 1996 Atari Games */
	DRIVER( sfrushrk )	/* (c) 1996 Atari Games */
	DRIVER( calspeed )	/* (c) 1996 Atari Games */
	DRIVER( calspeeda )	/* (c) 1996 Atari Games */
	DRIVER( vaportrx )	/* (c) 1998 Atari Games */
	DRIVER( vaportrxp )	/* (c) 1998 Atari Games */
	DRIVER( carnevil )	/* (c) 1998 Midway Games (v1.0.3) */
	DRIVER( carnevil1 )	/* (c) 1998 Midway Games (v1.0.1) */
	DRIVER( biofreak )	/* (c) 1997 Midway Games (prototype) */
	DRIVER( blitz )		/* (c) 1997 Midway Games */
	DRIVER( blitz11 )	/* (c) 1997 Midway Games */
	DRIVER( blitz99 )	/* (c) 1998 Midway Games */
	DRIVER( blitz2k )	/* (c) 1999 Midway Games */
	DRIVER( hyprdriv )	/* (c) 1998 Midway Games */

	/* Midway / Atari  Voodoo 2 & 3 - based Hardware (Vegas, Denver, Durango) */
	DRIVER( gauntleg )	/* (c) 1998 Atari Games */
	DRIVER( gauntleg12 )	/* (c) 1998 Atari Games */
	DRIVER( tenthdeg )	/* (c) 1998 Atari Games (prototype) */
	DRIVER( roadburn )	/* (c) 1999 Atari Games */
	DRIVER( sf2049 )	/* (c) 1999 Atari Games */
	DRIVER( sf2049se )	/* (c) 1999 Atari Games */
	DRIVER( sf2049te )	/* (c) 1999 Atari Games */
	DRIVER( warfa )		/* (c) 1999 Atari Games */
	DRIVER( nbashowt )	/* (c) 1998 Midway Games */
	DRIVER( nbanfl )	/* (c) 1999 Midway Games */
	DRIVER( gauntdl )	/* (c) 1999 Midway Games */
	DRIVER( gauntdl24 )	/* (c) 1999 Midway Games */
	DRIVER( cartfury )	/* (c) 2000 Midway Games */

	/* Midway Touchmaster / Galaxy Games */
	DRIVER( tm )		/* (c) 1996 Midway Games */
	DRIVER( tmdo )		/* (c) 1996 Midway Games */
	DRIVER( tm2k )		/* (c) 1996 Midway Games */
	DRIVER( tm2ka )		/* (c) 1996 Midway Games */
	DRIVER( tm3k )		/* (c) 1997 Midway Games */
	DRIVER( tm3ka )		/* (c) 1997 Midway Games */
	DRIVER( tm4k )		/* (c) 1998 Midway Games */
	DRIVER( tm4ka )		/* (c) 1998 Midway Games */
	DRIVER( tm5k )		/* (c) 1998 Midway Games */
	DRIVER( tm5kca )	/* (c) 1998 Midway Games */
	DRIVER( tm5ka )		/* (c) 1998 Midway Games */
	DRIVER( tm7k )		/* (c) 1999 Midway Games */
	DRIVER( tm7ka )		/* (c) 1999 Midway Games */
	DRIVER( tm7keval )	/* (c) 1999 Midway Games */
	DRIVER( tm8k )		/* (c) 2000 Midway Games */
	DRIVER( tm8k902 )	/* (c) 2000 Midway Games */
	DRIVER( galgbios )	/* (c) 1998 Creative Electronics & Software (CES) */
	DRIVER( galgame2 )	/* (c) 1998 Creative Electronics & Software (CES) / Namco */

	/* Cinematronics raster games */
	DRIVER( embargo )
	DRIVER( tripool )	/* (c) 1981 Noma (Casino Tech license) */
	DRIVER( tripoola )	/* (c) 1981 Noma (Costal Games license) */
	DRIVER( unclepoo )	/* (c) 1983 Diatec */
	DRIVER( joinem )	/* (c) 1986 Global */
	DRIVER( loverboy )	/* (c) 1983 Global */
	DRIVER( striv )		/* (c) 1985 Hara Industries */
	DRIVER( jack )		/* (c) 1982 Cinematronics */
	DRIVER( jack2 )		/* (c) 1982 Cinematronics */
	DRIVER( jack3 )		/* (c) 1982 Cinematronics */
	DRIVER( treahunt )	/* (c) 1982 Hara Ind. */
	DRIVER( zzyzzyxx )	/* (c) 1982 Cinematronics + Advanced Microcomputer Systems */
	DRIVER( zzyzzyxx2 )	/* (c) 1982 Cinematronics + Advanced Microcomputer Systems */
	DRIVER( brix )		/* (c) 1982 Cinematronics + Advanced Microcomputer Systems */
	DRIVER( freeze )	/* Cinematronics */
	DRIVER( sucasino )	/* (c) 1982 Data Amusement */

	/* Cinematronics vector games */
	DRIVER( spacewar )	/* (c) 1977 */
	DRIVER( spaceshp )	/* (c) 1978 Cinematronics (Sega licence) */
	DRIVER( barrier )	/* (c) 1979 Vectorbeam */
	DRIVER( starcas )	/* (c) 1980 */
	DRIVER( starcas1 )	/* (c) 1980 */
	DRIVER( starcasp )	/* (c) 1980 prototype */
	DRIVER( starcase )	/* (c) 1980 Cinematronics (Mottoeis license) */
	DRIVER( stellcas )	/* bootleg */
	DRIVER( spaceftr )	/* (c) 1981 Zaccaria */
	DRIVER( tailg )		/* (c) 1979 */
	DRIVER( ripoff )	/* (c) 1980 */
	DRIVER( armora )	/* (c) 1980 */
	DRIVER( armorap )	/* (c) 1980 */
	DRIVER( armorar )	/* (c) 1980 Cinematronics (Rock-ola license) */
	DRIVER( wotw )		/* (c) 1981 */
	DRIVER( wotwc )		/* (c) 1981 */
	DRIVER( warrior )	/* (c) 1979 Vectorbeam */
	DRIVER( starhawk )	/* (c) 1979 */
	DRIVER( solarq )	/* (c) 1981 */
	DRIVER( boxingb )	/* (c) 1981 */
	DRIVER( speedfrk )	/* (c) 1979 Vectorbeam */
	DRIVER( sundance )	/* (c) 1979 */
	DRIVER( demon )		/* (c) 1982 Rock-ola */
	DRIVER( qb3 )		/* (c) 1982 Rock-ola - prototype */
	/* this one uses 68000+Z80 instead of the Cinematronics CPU */
	DRIVER( cchasm )
	DRIVER( cchasm1 )	/* (c) 1983 Cinematronics / GCE */

	/* "The Pit hardware" games */
	DRIVER( roundup )	/* (c) 1981 Amenip/Centuri */
	DRIVER( fitter )	/* (c) 1981 Taito */
	DRIVER( fitterbl )	/* bootleg */
	DRIVER( thepit )	/* (c) 1982 Taito */
	DRIVER( thepitc )	/* (c) 1982 Centuri */
	DRIVER( thepitb )	/* bootleg */
	DRIVER( thepitm )	/* bootleg */
	DRIVER( dockman )	/* (c) 1982 Taito Corp. */
	DRIVER( portman )	/* (c) 1982 Nova Games Ltd. */
	DRIVER( suprmous )	/* (c) 1982 Taito */
	DRIVER( funnymou )	/* (c) 1982 Chuo Co. Ltd */
	DRIVER( machomou )	/* (c) 1982 Techstar */
	DRIVER( intrepid )	/* (c) 1983 Nova Games Ltd. */
	DRIVER( intrepid2 )	/* (c) 1983 Nova Games Ltd. */
	DRIVER( zaryavos )	/* (c) 1983 Nova Games of Canada (prototype) */
	DRIVER( timelimt )	/* (c) 1983 Chuo Co. Ltd */
	DRIVER( progress )	/* (c) 1984 Chuo Co. Ltd */
	DRIVER( koroleva )	/* (c) 1988 Terminal */
	DRIVER( rtriv )		/* (c) 198? Romar */

	/* TIA-MC1 hardware games */
	DRIVER( konek )		/* (c) 1988 Terminal */
	DRIVER( sosterm )	/* (c) 1988 Terminal */
	DRIVER( bilyard )	/* (c) 1988 Terminal */

	/* Photon hardware games */
	DRIVER( phtetris )
	DRIVER( phpython )
	DRIVER( phklad )

	/* Photon IK2 hardware */
	DRIVER( kok )
	DRIVER( black )
	DRIVER( brod )

	/* Valadon Automation games */
	DRIVER( bagman )	/* (c) 1982 */
	DRIVER( bagnard )	/* (c) 1982 */
	DRIVER( bagnarda )	/* (c) 1982 */
	DRIVER( bagmans )	/* (c) 1982 + Stern license */
	DRIVER( bagmans2 )	/* (c) 1982 + Stern license */
	DRIVER( pickin )	/* (c) 1983 */
	DRIVER( supdrapo )	/* (c) 1983 + Stern license */
	DRIVER( supdrapoa )	/* (c) 1983 + Jeutel license? */
	DRIVER( supdrapob )	/* bootleg */
	DRIVER( sbagman )	/* (c) 1984 */
	DRIVER( sbagmans )	/* (c) 1984 + Stern license */
	DRIVER( botanic )	/* (c) 1985 */
	DRIVER( squaitsa )	/* (c) 1985 */
	DRIVER( tankbust )	/* (c) 1985 */

	/* Seibu Denshi / Seibu Kaihatsu games */
	DRIVER( stinger )	/* (c) 1983 Seibu Denshi */
	DRIVER( stinger2 )	/* (c) 1983 Seibu Denshi */
	DRIVER( scion )		/* (c) 1984 Seibu Denshi */
	DRIVER( scionc )	/* (c) 1984 Seibu Denshi + Cinematronics license */
	DRIVER( kungfut )	/* (c) 1984 Seibu Kaihatsu */
	DRIVER( kungfuta )	/* (c) 1984 Seibu Kaihatsu */
	DRIVER( wiz )		/* (c) 1985 Seibu Kaihatsu */
	DRIVER( wizt )		/* (c) 1985 Taito Corporation */
	DRIVER( wizta )		/* (c) 1985 Taito Corporation */
	DRIVER( kncljoe )	/* (c) 1985 Taito Corporation */
	DRIVER( kncljoea )	/* (c) 1985 Taito Corporation */
	DRIVER( bcrusher )	/* bootleg */
	DRIVER( empcity )	/* (c) 1986 Seibu Kaihatsu (bootleg?) */
	DRIVER( empcityu )	/* (c) 1986 Taito (Romstar License) */
	DRIVER( empcityj )	/* (c) 1986 Taito Corporation (Japan) */
	DRIVER( stfight )	/* (c) 1986 Seibu Kaihatsu (Germany) (bootleg?) */
	DRIVER( stfighta )	/* (c) 1986 Seibu Kaihatsu (bootleg?) */
	DRIVER( darkmist )	/* (c) 1986 Taito */
	DRIVER( mustache )	/* (c) 1987 March */
	DRIVER( cshooter )	/* (c) 1987 Taito */
	DRIVER( cshootere )	/* (c) 1987 JKH (bootleg) */
	DRIVER( airraid )	/* (c) 1987 Seibu Kaihatsu */
	DRIVER( panicr )	/* Seibu / Taito */
	DRIVER( deadang )	/* (c) 1988 Seibu Kaihatsu */
	DRIVER( leadang )	/* (c) 1988 Seibu Kaihatsu */
	DRIVER( ghunter )	/* (c) 1988 Seibu Kaihatsu + Segasa/Sonic license */
	DRIVER( dynduke )	/* (c) 1989 Seibu Kaihatsu + Fabtek license */
	DRIVER( dyndukef )	/* (c) 1989 Seibu Kaihatsu + Fabtek license */
	DRIVER( dbldyn )	/* (c) 1989 Seibu Kaihatsu + Fabtek license */
	DRIVER( dbldynf )	/* (c) 1989 Seibu Kaihatsu + Fabtek license */
	DRIVER( raiden )	/* (c) 1990 Seibu Kaihatsu */
	DRIVER( raidena )	/* (c) 1990 Seibu Kaihatsu */
	DRIVER( raidenk )	/* (c) 1990 Seibu Kaihatsu + IBL Corporation license */
	DRIVER( raident )	/* (c) 1990 Seibu Kaihatsu + Liang HWA Electronics license */
	DRIVER( raidenu )	/* (c) 1990 Seibu Kaihatsu + Fabtek license */
	DRIVER( sdgndmps )	/* (c) Banpresto / Bandai (Japan) */
	DRIVER( dcon )		/* (c) 1992 Success */
	DRIVER( sengokmj )	/* (c) 1991 Sigma */
	DRIVER( totmejan )	/* (c) 1991 Seibu/Tecmo */
	DRIVER( goodejan )	/* (c) 1991 Seibu/Tecmo */
	DRIVER( goodejana )	/* (c) 1991 Seibu/Tecmo */


	DRIVER( raiden2 )	/* (c) 1993 Seibu Kaihatsu + Fabtek license */
	DRIVER( raiden2a )	/* (c) 1993 Seibu Kaihatsu + Metrotainment license */
	DRIVER( raiden2b )	/* (c) 1993 Seibu Kaihatsu */
	DRIVER( raiden2c )	/* (c) 1993 Seibu Kaihatsu */
	DRIVER( raiden2d )	/* (c) 1993 Seibu Kaihatsu */
	DRIVER( raiden2e )	/* (c) 1993 Seibu Kaihatsu */
	DRIVER( raidndx )	/* (c) 1994 Seibu Kaihatsu */
	DRIVER( raidndxj )	/* (c) 1994 Seibu Kaihatsu */
	DRIVER( raidndxu )	/* (c) 1994 Seibu Kaihatsu + Fabtek license */
	DRIVER( raidndxa1 )	/* (c) 1994 Seibu Kaihatsu + Metrotainment license */
	DRIVER( raidndxa2 )	/* (c) 1994 Seibu Kaihatsu + Metrotainment license */
	DRIVER( raidndxg )	/* (c) 1994 Seibu Kaihatsu + Tuning license */
	DRIVER( zeroteam )	/* (c) 1993 Seibu Kaihatsu */
	DRIVER( zeroteama )	/* (c) 1993 Seibu Kaihatsu */
	DRIVER( zeroteamb )	/* (c) 1993 Seibu Kaihatsu */
	DRIVER( zeroteamc )	/* (c) 1993 Seibu Kaihatsu */
	DRIVER( zeroteams )	/* (c) 1993 Seibu Kaihatsu */
	DRIVER( nzerotea )	/* (c) 1993 Seibu Kaihatsu */
	DRIVER( xsedae )	/* (c) 1995 Dream Island */

	/* Newer V33 PCB with COP3 protection */
	DRIVER( r2dx_v33 )	/* (c) 1996 Seibu Kaihatsu */

	/* Seibu SPI System games */

	DRIVER( senkyu )	/* (c) 1995 Seibu Kaihatsu */
	DRIVER( senkyua )	/* (c) 1995 Seibu Kaihatsu */
	DRIVER( batlball )	/* (c) 1995 Seibu Kaihatsu (Tuning License) */
	DRIVER( batlballa )	/* (c) 1995 Seibu Kaihatsu (Metrotainment License) */
	DRIVER( batlballu )	/* (c) 1995 Seibu Kaihatsu (Metrotainment License) */

	DRIVER( viprp1 )	/* (c) 1995 Seibu Kaihatsu */
	DRIVER( viprp1u )	/* (c) 1995 Seibu Kaihatsu (Fabtek license) */
	DRIVER( viprp1j )	/* (c) 1995 Seibu Kaihatsu */
	DRIVER( viprp1s )	/* (c) 1995 Seibu Kaihatsu */
	DRIVER( viprp1oj )	/* (c) 1995 Seibu Kaihatsu */
	DRIVER( viprp1ot )	/* (c) 1995 Seibu Kaihatsu (Tuning License) */
	DRIVER( viprp1hk )	/* (c) 1995 Seibu Kaihatsu (Metrotainment License) */

	DRIVER( ejanhs )	/* (c) 1996 Seibu Kaihatsu */

	DRIVER( rdft )		/* (c) 1996 Seibu Kaihatsu */
	DRIVER( rdftu )		/* (c) 1996 Seibu Kaihatsu (Fabtek license) */
	DRIVER( rdftau )	/* (c) 1996 Seibu Kaihatsu */
	DRIVER( rdftj )		/* (c) 1996 Seibu Kaihatsu */
	DRIVER( rdftadi )	/* (c) 1996 Seibu Kaihatsu (Dream Island license) */
	DRIVER( rdftam )	/* (c) 1996 Seibu Kaihatsu (Metrotainment license) */
	DRIVER( rdftit )	/* (c) 1996 Seibu Kaihatsu */
	DRIVER( rdfta )		/* (c) 1996 Seibu Kaihatsu */

	DRIVER( rdft2 )		/* (c) 1997 Seibu Kaihatsu (Tuning License) */
	DRIVER( rdft2a )	/* (c) 1997 Seibu Kaihatsu (Metrotainment license) */
	DRIVER( rdft2a2 )	/* (c) 1997 Seibu Kaihatsu (Dream Island license) */
	DRIVER( rdft2j )	/* (c) 1997 Seibu Kaihatsu */
	DRIVER( rdft2j2 )	/* (c) 1997 Seibu Kaihatsu */
	DRIVER( rdft2t )	/* (c) 1997 Seibu Kaihatsu */
	DRIVER( rdft2u )	/* (c) 1997 Seibu Kaihatsu (Fabtek license) */
	DRIVER( rdft2us )	/* (c) 1997 Seibu Kaihatsu (Fabtek license) */
	DRIVER( rdft22kc )	/* (c) 2000 Seibu Kaihatsu */

	DRIVER( rfjet )		/* (c) 1998 Seibu Kaihatsu (Tuning License) */
	DRIVER( rfjetu )	/* (c) 1998 Seibu Kaihatsu (Fabtek license) */
	DRIVER( rfjeta )	/* (c) 1998 Seibu Kaihatsu (Dream Island license) */
	DRIVER( rfjetj )	/* (c) 1998 Seibu Kaihatsu */
	DRIVER( rfjets )	/* (c) 1998 Seibu Kaihatsu */
	DRIVER( rfjet2kc )	/* (c) 2000 Seibu Kaihatsu */

	DRIVER( ejsakura )	/* (c) 1999 Seibu Kaihatsu */
	DRIVER( ejsakura12 )	/* (c) 1999 Seibu Kaihatsu */

	/* Seibu System SH2B games */
	DRIVER( feversoc )	/* (c) 2000 */

	/* Tad games (Tad games run on Seibu hardware) */
	DRIVER( cabal )		/* (c) 1988 Tad + Fabtek license */
	DRIVER( cabala )	/* (c) 1988 Tad + Fabtek license */
	DRIVER( cabalbl )	/* bootleg */
	DRIVER( cabalbl2 )	/* bootleg */
	DRIVER( cabalus )	/* (c) 1988 Tad + Fabtek license */
	DRIVER( cabalus2 )	/* (c) 1988 Tad + Fabtek license */
	DRIVER( toki )		/* (c) 1989 Tad (World) */
	DRIVER( tokia )		/* (c) 1989 Tad (World) */
	DRIVER( tokiu )		/* (c) 1989 Tad + Fabtek license (US) */
	DRIVER( tokiua )	/* (c) 1989 Tad + Fabtek license (US) */
	DRIVER( tokib )		/* bootleg */
	DRIVER( juju )		/* (c) 1989 Tad (Japan) */
	DRIVER( jujub )		/* (c) 1989 Tad (Japan) */
	DRIVER( bloodbro )	/* (c) 1990 Tad */
	DRIVER( bloodbroa )	/* (c) 1990 Tad */
	DRIVER( bloodbrob )	/* (c) 1990 Tad */
	DRIVER( weststry )	/* bootleg */
	DRIVER( skysmash )	/* (c) 1990 Nihon System Inc. */
	DRIVER( legionna )	/* (c) 1992 Tad (World) */
	DRIVER( legionnau )	/* (c) 1992 Tad + Fabtek license (US) */
	DRIVER( heatbrl )	/* (c) 1992 Tad (World version 3) */
	DRIVER( heatbrl2 )	/* (c) 1992 Tad (World version 2) */
	DRIVER( heatbrlo )	/* (c) 1992 Tad (World) */
	DRIVER( heatbrlu )	/* (c) 1992 Tad (US) */
	DRIVER( godzilla )	/* (c) 1993 Banpresto */
	DRIVER( denjinmk )	/* (c) 199? Banpresto */
	DRIVER( grainbow )	/* (c) 1993 Banpresto */
	DRIVER( cupsoc )	/* (c) 1992 Seibu */
	DRIVER( cupsoca )	/* (c) 1992 Seibu */
	DRIVER( cupsocs )	/* (c) 1992 Seibu */
	DRIVER( cupsocs2 )	/* (c) 1992 Seibu */
	DRIVER( cupsocsb )	/* bootleg */
	DRIVER( cupsocsb2 )	/* bootleg */
	DRIVER( olysoc92 )	/* (c) 1992 Seibu */
	DRIVER( goal92 )	/* bootleg */

	/* Jaleco games */
	DRIVER( exerion )	/* (c) 1983 Jaleco */
	DRIVER( exeriont )	/* (c) 1983 Jaleco + Taito America license */
	DRIVER( exerionb )	/* bootleg */
	DRIVER( fcombat )	/* (c) 1985 Jaleco */
	DRIVER( pturn )		/* (c) 1984 Jaleco */
	DRIVER( ddayjlc )	/* (c) 1984 Jaleco */
	DRIVER( ddayjlca )	/* (c) 1984 Jaleco */
	DRIVER( formatz )	/* (c) 1984 Jaleco */
	DRIVER( aeroboto )	/* (c) 1984 Williams */
	DRIVER( citycon )	/* (c) 1985 Jaleco */
	DRIVER( citycona )	/* (c) 1985 Jaleco */
	DRIVER( cruisin )	/* (c) 1985 Jaleco/Kitkorp */
	DRIVER( momoko )	/* (c) 1986 Jaleco */
	DRIVER( argus )		/* (c) 1986 Jaleco */
	DRIVER( valtric )	/* (c) 1986 Jaleco */
	DRIVER( butasan )	/* (c) 1987 Jaleco */
	DRIVER( psychic5 )	/* (c) 1987 Jaleco */
	DRIVER( psychic5a )	/* (c) 1987 Jaleco */
	DRIVER( bombsa )	/* (c) 1988 Jaleco */
	DRIVER( ginganin )	/* (c) 1987 Jaleco */
	DRIVER( ginganina )	/* (c) 1987 Jaleco */
	DRIVER( skyfox )	/* (c) 1987 Jaleco + Nichibutsu USA license */
	DRIVER( exerizer )	/* (c) 1987? Jaleco */
	DRIVER( exerizerb )	/* bootleg */
	DRIVER( homerun )	/* (c) 1988 Jaleco */
	DRIVER( dynashot )	/* (c) 1988 Jaleco */
	DRIVER( bigrun )	/* (c) 1989 Jaleco */
	DRIVER( cischeat )	/* (c) 1990 Jaleco */
	DRIVER( f1gpstar )	/* (c) 1991 Jaleco */
	DRIVER( armchmp2 )	/* (c) 1992 Jaleco */
	DRIVER( armchmp2o )	/* (c) 1992 Jaleco */
	DRIVER( wildplt )	/* (c) 1992 Jaleco */
	DRIVER( f1gpstr2 )	/* (c) 1993 Jaleco */
	DRIVER( scudhamm )	/* (c) 1994 Jaleco */
	DRIVER( tetrisp2 )	/* (c) 1997 Jaleco */
	DRIVER( tetrisp2j )	/* (c) 1997 Jaleco */
	DRIVER( nndmseal )	/* (c) 1997 I'Max/Jaleco */
	DRIVER( stepstag )	/* (c) 1999 Jaleco */
	DRIVER( step3 )		/* (c) 1999 Jaleco */
	DRIVER( rockn )		/* (c) 1999 Jaleco */
	DRIVER( rockna )	/* (c) 1999 Jaleco */
	DRIVER( rockn2 )	/* (c) 1999 Jaleco */
	DRIVER( rocknms )	/* (c) 1999 Jaleco */
	DRIVER( rockn3 )	/* (c) 1999 Jaleco */
	DRIVER( rockn4 )	/* (c) 2000 Jaleco */

	/* Jaleco Mega System 1 games */
	DRIVER( lomakai )	/* (c) 1988 (World) */
	DRIVER( makaiden )	/* (c) 1988 (Japan) */
	DRIVER( p47 )		/* (c) 1988 */
	DRIVER( p47j )		/* (c) 1988 (Japan) */
	DRIVER( kickoff )	/* (c) 1988 (Japan) */
	DRIVER( tshingen )	/* (c) 1988 (Japan) */
	DRIVER( tshingena )	/* (c) 1988 (Japan) */
	DRIVER( kazan )		/* (c) 1988 (World) */
	DRIVER( iganinju )	/* (c) 1988 (Japan) */
	DRIVER( astyanax )	/* (c) 1989 */
	DRIVER( lordofk )	/* (c) 1989 (Japan) */
	DRIVER( hachoo )	/* (c) 1989 */
	DRIVER( jitsupro )	/* (c) 1989 (Japan) */
	DRIVER( plusalph )	/* (c) 1989 */
	DRIVER( stdragon )	/* (c) 1989 */
	DRIVER( rodland )	/* (c) 1990 */
	DRIVER( rodlandj )	/* (c) 1990 (Japan) */
	DRIVER( rodlandjb )	/* bootleg */
	DRIVER( avspirit )	/* (c) 1991 */
	DRIVER( phantasm )	/* (c) 1991 (Japan) */
	DRIVER( monkelf )	/* bootleg */
	DRIVER( edf )		/* (c) 1991 */
	DRIVER( edfu )		/* (c) 1991 */
	DRIVER( edfbl )		/* (c) 1991 */
	DRIVER( 64street )	/* (c) 1991 */
	DRIVER( 64streetj )	/* (c) 1991 (Japan) */
	DRIVER( soldam )	/* (c) 1992 */
	DRIVER( soldamj )	/* (c) 1992 (Japan) */
	DRIVER( bigstrik )	/* (c) 1992 */
	DRIVER( bigstrkb )	/* bootleg on different hardware */
	DRIVER( bestleag )	/* bootleg, by Playmark? */
	DRIVER( bestleaw )	/* bootleg */
	DRIVER( chimerab )	/* (c) 1993 */
	DRIVER( cybattlr )	/* (c) 1993 */
	DRIVER( hayaosi1 )	/* (c) 1993 */
	DRIVER( peekaboo )	/* (c) 1993 */

	/* Jaleco Mega System 32 games */
	DRIVER( hayaosi2 )	/* (c) 1993 */
	DRIVER( hayaosi3 )	/* (c) 1994 */
	DRIVER( bbbxing )	/* (c) 1994 */
	DRIVER( 47pie2 )	/* (c) 1994 */
	DRIVER( 47pie2o )	/* (c) 1994 */
	DRIVER( desertwr )	/* (c) 1995 */
	DRIVER( gametngk )	/* (c) 1995 */
	DRIVER( tetrisp )	/* (c) 1995 Jaleco / BPS */
	DRIVER( p47aces )	/* (c) 1995 */
	DRIVER( akiss )		/* (c) 1995 */
	DRIVER( gratia )	/* (c) 1996 */
	DRIVER( gratiaa )	/* (c) 1996 */
	DRIVER( kirarast )	/* (c) 1996 */
	DRIVER( tp2m32 )	/* (c) 1997 */
	DRIVER( bnstars )	/* (c) 1997 */
	DRIVER( wpksocv2 )	/* (c) 1997 */

	DRIVER( f1superb )	/* (c) 1994 */

	DRIVER( bnstars1 )	/* (c) 1994 */

	/* Video System Co. games */
	DRIVER( rabiolep )	/* (c) 1987 V-System Co. (Japan) */
	DRIVER( rpunch )	/* (c) 1987 V-System Co. + Bally/Midway/Sente license (US) */
	DRIVER( svolley )	/* (c) 1989 V-System Co. (Japan) */
	DRIVER( svolleyk )	/* (c) 1989 V-System Co. (Korea) */
	DRIVER( svolleyu )	/* (c) 1989 V-System Co. + Data East license (US) */
	DRIVER( tail2nos )	/* [1989] V-System Co. */
	DRIVER( sformula )	/* [1989] V-System Co. (Japan) */
	DRIVER( ojankoc )	/* [1986] V-System Co. (Japan) */
	DRIVER( ojankoy )	/* [1986] V-System Co. (Japan) */
	DRIVER( ojanko2 )	/* [1987] V-System Co. (Japan) */
	DRIVER( ccasino )	/* [1987] V-System Co. (Japan) */
	DRIVER( ojankohs )	/* [1988] V-System Co. (Japan) */
	DRIVER( nekkyoku )	/* [1988] Video System Co. (Japan) */
	DRIVER( idolmj )	/* [1988] (c) System Service (Japan) */
	DRIVER( mjnatsu )	/* [1989] Video System presents (Japan) */
	DRIVER( natsuiro )	/* [1989] Video System presents (Japan) */
	DRIVER( mfunclub )	/* [1989] V-System (Japan) */
	DRIVER( daiyogen )	/* [1990] Video System Co. (Japan) */
	DRIVER( nmsengen )	/* (c) 1991 Video System (Japan) */
	DRIVER( fromance )	/* (c) 1991 Video System Co. (Japan) */
	DRIVER( pipedrm )	/* (c) 1990 Video System Co. (World) */
	DRIVER( pipedrmu )	/* (c) 1990 Video System Co. (US) */
	DRIVER( pipedrmj )	/* (c) 1990 Video System Co. (Japan) */
	DRIVER( hatris )	/* (c) 1990 Video System Co. (US) */
	DRIVER( hatrisj )	/* (c) 1990 Video System Co. (Japan) */
	DRIVER( spinlbrk )	/* (c) 1990 V-System Co. (World) */
	DRIVER( spinlbrku )	/* (c) 1990 V-System Co. (US) */
	DRIVER( spinlbrkj )	/* (c) 1990 V-System Co. (Japan) */
	DRIVER( pspikes )	/* (c) 1991 Video System Co. (World) */
	DRIVER( pspikesk )	/* (c) 1991 Video System Co. (Korea) */
	DRIVER( svolly91 )	/* (c) 1991 Video System Co. (Japan) */
	DRIVER( pspikesb )	/* bootleg */
	DRIVER( spikes91 )	/* bootleg */
	DRIVER( pspikesc )	/* bootleg */
	DRIVER( karatblz )	/* (c) 1991 Video System Co. */
	DRIVER( karatblzu )	/* (c) 1991 Video System Co. (US) */
	DRIVER( karatblzj )	/* (c) 1991 Video System / Tecmo (Japan) */
	DRIVER( turbofrc )	/* (c) 1991 Video System Co. */
	DRIVER( aerofgt )	/* (c) 1992 Video System Co. */
	DRIVER( aerofgtb )	/* (c) 1992 Video System Co. */
	DRIVER( aerofgtc )	/* (c) 1992 Video System Co. */
	DRIVER( sonicwi )	/* (c) 1992 Video System Co. (Japan) */
	DRIVER( aerfboot )	/* bootleg */
	DRIVER( aerfboo2 )	/* bootleg */
	DRIVER( wbbc97 )	/* (c) 1997 Comad */
	DRIVER( welltris )	/* (c) 1991 Video System Co. (Japan) */
	DRIVER( welltrisj )	/* (c) 1991 Video System Co. (Japan) */
	DRIVER( quiz18k )	/* (c) 1992 EIM (Welltris hardware) */
	DRIVER( f1gp )		/* (c) 1991 Video System Co. */
	DRIVER( f1gpb )		/* bootleg */
	DRIVER( f1gp2 )		/* (c) 1992 Video System Co. */
	DRIVER( crshrace )	/* (c) 1993 Video System Co. */
	DRIVER( crshrace2 )	/* (c) 1993 Video System Co. */
	DRIVER( taotaido )	/* (c) 1993 Video System Co. */
	DRIVER( taotaidoa )	/* (c) 1993 Video System Co. */
	DRIVER( gstriker )	/* (c) [1993] Human */
	DRIVER( gstrikera )	/* (c) [1993] Human */
	DRIVER( vgoalsoc )	/* (c) 199? Temco */
	DRIVER( vgoalsca )	/* (c) 199? Temco */
	DRIVER( twrldc94 )	/* (c) 1994 Tecmo */
	DRIVER( twrldc94a )	/* (c) 1994 Tecmo */
	DRIVER( suprslam )	/* (c) 1995 Banpresto */
	DRIVER( fromanc2 )	/* (c) 1995 Video System Co. (Japan) */
	DRIVER( fromancr )	/* (c) 1995 Video System Co. (Japan) */
	DRIVER( fromanc4 )	/* (c) 1998 Video System Co. (Japan) */
	DRIVER( inufuku )	/* (c) 1998 Video System Co. (Japan) */

	/* Psikyo games */
	DRIVER( samuraia )	/* (c) 1993 (World) */
	DRIVER( sngkace )	/* (c) 1993 (Japan) */
	DRIVER( gunbird )	/* (c) 1994 */
	DRIVER( gunbirdk )	/* (c) 1994 */
	DRIVER( gunbirdj )	/* (c) 1994 */
	DRIVER( btlkroad )	/* (c) 1994 */
	DRIVER( s1945 )		/* (c) 1995 */
	DRIVER( s1945a )	/* (c) 1995 */
	DRIVER( s1945j )	/* (c) 1995 */
	DRIVER( s1945jn )	/* (c) 1995 */
	DRIVER( s1945bl )	/* (c) 1995 (Hong Kong bootleg) */
	DRIVER( s1945k )	/* (c) 1995 */
	DRIVER( tengai )	/* (c) 1996 */
	DRIVER( tengaij )	/* (c) 1996 */
	DRIVER( s1945ii )	/* (c) 1997 */
	DRIVER( soldivid )	/* (c) 1997 */
	DRIVER( sbomberb )	/* (c) 1998 */
	DRIVER( daraku )	/* (c) 1998 */
	DRIVER( gunbird2 )	/* (c) 1998 */
	DRIVER( s1945iii )	/* (c) 1999 */
	DRIVER( dragnblz )	/* (c) 2000 */
	DRIVER( tgm2 )		/* (c) 2000 */
	DRIVER( tgm2p )		/* (c) 2000 */
	DRIVER( gnbarich )	/* (c) 2001 */
	DRIVER( mjgtaste )	/* (c) 2002 */
	DRIVER( hotgmck )	/* (c) 1997 */
	DRIVER( hgkairak )	/* (c) 1998 */
	DRIVER( hotgmck3 )	/* (c) 1999 */
	DRIVER( hotgm4ev )	/* (c) 2000 */
	DRIVER( hotgmcki )	/* (c) 2001 */
	DRIVER( loderndf )	/* (c) 2000 */
	DRIVER( loderndfa )	/* (c) 2000 */
	DRIVER( hotdebut )	/* (c) 2000 */

	/* Orca games */
	DRIVER( marineb )	/* (c) 1982 Orca */
	DRIVER( changes )	/* (c) 1982 Orca */
	DRIVER( changesa )	/* (c) 1982 Orca + Eastern Micro Electronics license */
	DRIVER( looper )	/* (c) 1982 Orca */
	DRIVER( springer )	/* (c) 1982 Orca */
	DRIVER( hoccer )	/* (c) 1983 Eastern Micro Electronics, Inc. */
	DRIVER( hoccer2 )	/* (c) 1983 Eastern Micro Electronics, Inc. */
	DRIVER( bcruzm12 )	/* (c) 1983 Sigma Ent. Inc. */
	DRIVER( hopprobo )	/* (c) 1983 Sega */
	DRIVER( wanted )	/* (c) 1984 Sigma Ent. Inc. */
	DRIVER( funkybee )	/* (c) 1982 Orca */
	DRIVER( funkybeeb )	/* (c) 1982 Orca */
	DRIVER( skylancr )	/* (c) 1983 Orca */
	DRIVER( skylancre )	/* (c) 1983 Orca + Esco Trading Co license */
	DRIVER( zodiack )	/* (c) 1983 Orca + Esco Trading Co license */
	DRIVER( dogfight )	/* (c) 1983 Thunderbolt */
	DRIVER( moguchan )	/* (c) 1982 Orca + Eastern Commerce Inc. license (doesn't appear on screen) */
	DRIVER( percuss )	/* (c) 1981 Orca */
	DRIVER( bounty )	/* (c) 1982 Orca */
	DRIVER( espial )	/* (c) 1983 Thunderbolt, Orca logo is hidden in title screen */
	DRIVER( espialu )	/* (c) 1983 Thunderbolt, Orca logo is hidden in title screen */
	DRIVER( netwars )	/* (c) 1983 Orca + Esco Trading Co license */
	/* Vastar was made by Orca, but when it was finished, Orca had already bankrupted. */
	/* So they sold this game as "Made by Sesame Japan" because they couldn't use */
	/* the name "Orca" */
	DRIVER( vastar )	/* (c) 1983 Sesame Japan */
	DRIVER( vastar2 )	/* (c) 1983 Sesame Japan */
/*
   other Orca games:
   82 Battle Cross                         Kit 2P
   82 River Patrol Empire Mfg/Kerstens Ind Ded 2P        HC Action
   82 Slalom                               Kit 2P        HC Action
   83 Net Wars                                 2P
   83 Super Crush                          Kit 2P           Action
*/

	/* Gaelco 2D games */
	DRIVER( mastboy )	/* (c) 1987 - No Ref on the PCB */
	DRIVER( mastboyi )	/* (c) 1987 - No Ref on the PCB */
	DRIVER( xorworld )	/* (c) 1990 - prototype */
	DRIVER( bigkarnk )	/* (c) 1991 - Ref 901112-1 */
	/* Master Boy 2 */	/* (c) 1991 - Ref ??? */
	DRIVER( splash )	/* (c) 1992 - Ref 922804 */
	DRIVER( splash10 )	/* (c) 1992 - Ref 922804 */
	DRIVER( paintlad )	/* (c) 1992 - ver 1.3 */
	DRIVER( roldfrog )	/* not a gaelco game but based on splash code */
	DRIVER( roldfroga )	/* not a gaelco game but based on splash code */
	DRIVER( rebus )		/* not a gaelco game but on similar hw to splash */
	DRIVER( funystrp )	/* not a gaelco game but on similar hw to splash */
	DRIVER( puckpepl )	/* not a gaelco game but on similar hw to splash */
	DRIVER( thoop )		/* (c) 1992 - Ref 922804/1 */
	DRIVER( squash )	/* (c) 1992 - Ref 922804/2 */
	DRIVER( wrally )	/* (c) 1993 - Ref 930705 */
	DRIVER( wrallya )	/* (c) 1993 - Ref 930705 */
	DRIVER( wrallyb )	/* (c) 1993 - Ref 930217 */
	DRIVER( glass )		/* (c) 1993 - Ref 931021 */
	DRIVER( glass10 )	/* (c) 1993 - Ref 931021 */
	DRIVER( glassbrk )	/* (c) 1993 - Ref 931021 shows "Break Edition" on a real PCB */
	DRIVER( targeth )	/* (c) 1994 - Ref 940531 */
	DRIVER( targetha )	/* (c) 1994 - Ref 940531 */
	DRIVER( thoop2 )	/* (c) 1994 - Ref ??? */
	DRIVER( aligator )	/* (c) 1994 - Ref 940411 */
	DRIVER( aligatorun )	/* (c) 1994 - Ref 940411 (unprotected) */
	DRIVER( biomtoy )	/* (c) 1995 - Ref 922804/2 - (unprotected) */
	DRIVER( touchgo )	/* (c) 1995 - Ref 950906 */
	DRIVER( touchgon )	/* (c) 1995 - Ref 950906 */
	DRIVER( touchgoe )	/* (c) 1995 - Ref 950510-1 */
	DRIVER( wrally2 )	/* (c) 1995 - Ref 950510 */
	DRIVER( maniacsp )	/* (c) 1996 - Ref 922804/2 - (prototype) */
	DRIVER( maniacsq )	/* (c) 1996 - Ref ??? - (unprotected) */
	DRIVER( snowboar )	/* (c) 1996 - Ref 960419/1 */
	DRIVER( snowboara )	/* (c) 1996 - Ref 960419/1 */
	DRIVER( bang )		/* (c) 1998 - Ref ??? */
	DRIVER( bangj )		/* (c) 1999 - Ref ??? */
	DRIVER( grtesoro )	/* (c) 1999 - Ref ??? */

	/* Gaelco 3D games */
	DRIVER( speedup )	/* (c) 1996 - Ref ??? */
	DRIVER( surfplnt )	/* (c) 1997 - Ref 971223 */
	DRIVER( surfplnt40 )	/* (c) 1997 - Ref 970514 */
	DRIVER( radikalb )	/* (c) 1998 - Ref ??? */

	/*
    Remaining Gaelco Games:
    =======================
    1999: Rolling Extreme
    2000: Football Power
    2001: Smashing Drive
    2002: ATV Track
    */

	/* Galeco PC based games */
	DRIVER( tokyocop )	/* (c) 2003 (Arcade TV Game List - P.168, Right, 19 from bottom) */

	/* Kaneko games */
	DRIVER( hvyunit )	/* (c) 1988 Kaneko */
	DRIVER( hvyunitj )	/* (c) 1988 Kaneko */
	DRIVER( hvyunito )	/* (c) 1988 Kaneko */
	DRIVER( djboy )		/* (c) 1989 Kaneko */
	DRIVER( djboyj )	/* (c) 1989 Kaneko */
	DRIVER( djboya )	/* (c) 1989 Kaneko */
	DRIVER( airbustr )	/* (c) 1990 Kaneko + Namco */
	DRIVER( airbustrj )	/* (c) 1990 Kaneko + Namco (Japan) */
	DRIVER( airbustrb )	/* bootleg */
	DRIVER( galpanic )	/* (c) 1990 Kaneko */
	DRIVER( galpanica )	/* (c) 1990 Kaneko */
	DRIVER( galsnew )	/* (c) 1990 Kaneko */
	DRIVER( galsnewa )	/* (c) 1990 Kaneko */
	DRIVER( galsnewj )	/* (c) 1990 Kaneko */
	DRIVER( galsnewk )	/* (c) 1990 Kaneko */
	DRIVER( galpani2 )	/* (c) 1993 Kaneko */
	DRIVER( galpani2g )	/* (c) 1993 Kaneko */
	DRIVER( galpani2t )	/* (c) 1993 Kaneko */
	DRIVER( galpani2j )	/* (c) 1993 Kaneko */
	DRIVER( gp2quiz )	/* (c) 1993 Kaneko */
	DRIVER( gp2se )		/* (c) 1994 Kaneko */
	DRIVER( jchan )		/* (c) 1995 Kaneko */
	DRIVER( jchan2 )	/* (c) 1995 Kaneko */
	DRIVER( galpani3 )	/* (c) 1995 Kaneko */

	/* Kaneko "AX System" games */
	DRIVER( berlwall )	/* (c) 1991 Kaneko */
	DRIVER( berlwallt )	/* (c) 1991 Kaneko */
	DRIVER( mgcrystl )	/* (c) 1991 Kaneko (World) */
	DRIVER( mgcrystlo )	/* (c) 1991 Kaneko (World) */
	DRIVER( mgcrystlj )	/* (c) 1991 Kaneko + distributed by Atlus (Japan) */
	DRIVER( blazeon )	/* (c) 1992 Atlus */
	DRIVER( sandscrp )	/* (c) 1992 Face */
	DRIVER( sandscrpa )	/* (c) 1992 Face */
	DRIVER( sandscrpb )	/* (c) 1992 Face */
	DRIVER( explbrkr )	/* (c) 1992 Kaneko */
	DRIVER( bakubrkr )	/* (c) 1992 Kaneko */
	DRIVER( shogwarr )	/* (c) 1992 Kaneko */
	DRIVER( shogwarru )	/* (c) 1992 Kaneko */
	DRIVER( fjbuster )	/* (c) 1992 Kaneko */
	DRIVER( brapboys )	/* (c) 1992 Kaneko */
	DRIVER( brapboysj )	/* (c) 1992 Kaneko */
	DRIVER( bloodwar )	/* (c) 1994 Kaneko */
	DRIVER( oedfight )	/* (c) 1994 Kaneko */
	DRIVER( bonkadv )	/* (c) 1994 Kaneko */
	DRIVER( gtmr )		/* (c) 1994 Kaneko */
	DRIVER( gtmra )		/* (c) 1994 Kaneko */
	DRIVER( gtmre )		/* (c) 1994 Kaneko */
	DRIVER( gtmrusa )	/* (c) 1994 Kaneko (US) */
	DRIVER( gtmr2 )		/* (c) 1995 Kaneko */
	DRIVER( gtmr2a )	/* (c) 1995 Kaneko */
	DRIVER( gtmr2u )	/* (c) 1995 Kaneko */

	/* Kaneko "Super Nova System" games */
	DRIVER( skns )
	DRIVER( galpani4 )	/* 1996.09 (c) 1996 Kaneko (Japan) */
	DRIVER( galpani4k )	/* 1996.09 (c) 1996 Kaneko (Korea) */
	DRIVER( jjparads )	/* 1996.12 (c) 1996 Electro Design Co. (Japan) */
	DRIVER( sarukani )	/* 1997.01 (c) 1997 Kaneko / Mediaworks (Japan) */
	DRIVER( vblokbrk )	/* 1997.?? (c) 1997 Kaneko / Mediaworks (Asia) */
	DRIVER( galpanis )	/* 1997.04 (c) 1997 Kaneko (Japan) */
	DRIVER( galpanisk )	/* 1997.04 (c) 1997 Kaneko (Korea) */
	DRIVER( sengekis )	/* 1997.?? (c) 1997 Kaneko / Warashi (Asia) */
	DRIVER( sengekisj )	/* 1997.07 (c) 1997 Kaneko / Warashi (Japan) */
	DRIVER( jjparad2 )	/* 1997.12 (c) 1997 Electro Design Co. (Japan) */
	DRIVER( cyvern )	/* 1998.02 (c) 1998 Kaneko (Japan) */
	DRIVER( ryouran )	/* 1998.07 (c) 1998 Electro Design Co. (Japan) */
	DRIVER( puzzloop )	/* 1998.?? (c) 1998 Mitchell (Europe) */
	DRIVER( puzzloopj )	/* 1998.12 (c) 1998 Mitchell (Japan) */
	DRIVER( puzzloopa )	/* 1998.?? (c) 1998 Mitchell (Asia) */
	DRIVER( puzzloopk )	/* 1998.?? (c) 1998 Mitchell (Korea) */
	DRIVER( puzzloopu )	/* 1999.05 (c) 1998 Mitchell (USA) */
	DRIVER( galpans2 )	/* 1999.02 (c) 1999 Kaneko (Japan) */
	DRIVER( galpans2a )	/* 1999.?? (c) 1999 Kaneko (Asia) */
	DRIVER( galpansu )	/* 1999.?? (c) 1999 Kaneko (Korea) */
	DRIVER( senknow )	/* 1999.04 (c) 1999 Kaneko / Kouyousha (Japan) */
	DRIVER( panicstr )	/* 1999.09 (c) 1999 Kaneko (Japan) */
	DRIVER( teljan )	/* 1999.11 (c) 1999 Electro Design Co. (Japan) */
	DRIVER( gutsn )		/* 2000.06 (c) 2000 Kaneko / Kouyousha (Japan) */
	DRIVER( galpans3 )	/* 2002.07 (c) 2002 Kaneko (Japan) */

	/* Seta games (seta.c) */
	DRIVER( hanaawas )	/* (c) SetaKikaku */
	DRIVER( speedatk )	/* CB-0 (c) SetaKikaku */
	DRIVER( srmp1 )		/* UB or UC?? (c) 1987 */
	DRIVER( srmp2 )		/* UB or UC?? (c) 1987 */
	DRIVER( srmp3 )		/* ZA-0? (c) 1988 */
	DRIVER( mjyuugi )	/* (c) 1990 Visco */
	DRIVER( mjyuugia )	/* (c) 1990 Visco */
	DRIVER( ponchin )	/* (c) 1991 Visco */
	DRIVER( ponchina )	/* (c) 1991 Visco */
	DRIVER( tndrcade )	/* UA-0 (c) 1987 Taito */
	DRIVER( tndrcadej )	/* UA-0 (c) 1987 Taito */
	DRIVER( twineagl )	/* UA-2 (c) 1988 + Taito license */
	DRIVER( downtown )	/* UD-2 (c) 1989 + Romstar or Taito license (DSW) */
	DRIVER( downtown2 )	/* UD-2 (c) 1989 + Romstar or Taito license (DSW) */
	DRIVER( downtownj )	/* UD-2 (c) 1989 + Romstar or Taito license (DSW) */
	DRIVER( downtownp )	/* UD-2 (c) 1989 + Romstar or Taito license (DSW) */
	DRIVER( usclssic )	/* UE   (c) 1989 + Romstar or Taito license (DSW) */
	DRIVER( calibr50 )	/* UH   (c) 1989 + Romstar or Taito license (DSW) */
	DRIVER( arbalest )	/* UK   (c) 1989 + Jordan, Romstar or Taito license (DSW) */
	DRIVER( metafox )	/* UP   (c) 1989 + Jordan, Romstar or Taito license (DSW) */
	DRIVER( setaroul )	/* UF   (c) 19?? Seta / Visco */
	DRIVER( drgnunit )	/* (c) 1989 Athena / Seta + Romstar or Taito license (DSW) */
	DRIVER( wits )		/* (c) 1989 Athena (Visco license) */
	DRIVER( thunderl )	/* (c) 1990 Seta + Romstar or Visco license (DSW) */
	DRIVER( wiggie )	/* (c) 1994 Promat (not genuine seta hardware, but based on thunderl code) */
	DRIVER( superbar )	/* (c) 1994 Promat (not genuine seta hardware, but based on thunderl code) */
	DRIVER( jockeyc )	/* (c) 1990 [Seta] (Visco license) */
	DRIVER( rezon )		/* (c) 1991 Allumer */
	DRIVER( rezont )	/* (c) 1991 Allumer / Taito */
	DRIVER( stg )		/* (c) 1991 Athena / Tecmo */
	DRIVER( pairlove )	/* (c) 1991 Athena */
	DRIVER( blandia )	/* (c) 1992 Allumer */
	DRIVER( blandiap )	/* (c) 1992 Allumer */
	DRIVER( blockcar )	/* (c) 1992 Visco */
	DRIVER( qzkklogy )	/* (c) 1992 Tecmo */
	DRIVER( neobattl )	/* (c) 1992 Banpresto / Sotsu Agency. Sunrise */
	DRIVER( umanclub )	/* (c) 1992 Tsuburaya Prod. / Banpresto */
	DRIVER( zingzip )	/* UY   (c) 1992 Allumer + Tecmo */
	DRIVER( atehate )	/* (C) 1993 Athena */
	DRIVER( jjsquawk )	/* (c) 1993 Athena / Able */
	DRIVER( jjsquawkb )	/* bootleg */
	DRIVER( kamenrid )	/* (c) 1993 Toei / Banpresto */
	DRIVER( madshark )	/* (c) 1993 Allumer */
	DRIVER( msgundam )	/* (c) 1993 Banpresto */
	DRIVER( msgundam1 )	/* (c) 1993 Banpresto */
	DRIVER( daioh )		/* (C) 1993 Athena */
	DRIVER( oisipuzl )	/* (c) 1993 SunSoft / Atlus */
	DRIVER( triplfun )	/* bootleg */
	DRIVER( utoukond )	/* (c) 1993 Banpresto + Tsuburaya Prod. */
	DRIVER( qzkklgy2 )	/* (c) 1993 Tecmo */
	DRIVER( wrofaero )	/* (c) 1993 Yang Cheng */
	DRIVER( eightfrc )	/* (c) 1994 Tecmo */
	DRIVER( kiwame )	/* (c) 1994 Athena */
	DRIVER( krzybowl )	/* (c) 1994 American Sammy */
	DRIVER( orbs )		/* (c) 1994 American Sammy */
	DRIVER( keroppi )	/* (c) 1993 Sammy */
	DRIVER( extdwnhl )	/* (c) 1995 Sammy Japan */
	DRIVER( gundhara )	/* (c) 1995 Banpresto */
	DRIVER( sokonuke )	/* (c) 1995 Sammy Industries */
	DRIVER( zombraid )	/* (c) 1995 American Sammy */
	DRIVER( crazyfgt )	/* (c) 1998 Subsino */
	DRIVER( inttoote )	/* (c) 1998 Coinmaster */
	DRIVER( inttootea )	/* (c) 1998 Coinmaster */

	/* Seta games (seta2.c) */
	DRIVER( gundamex )	/* (c) 1994 Banpresto */
	DRIVER( grdians )	/* (c) 1995 Banpresto */
	DRIVER( mj4simai )	/* (c) 1996 Maboroshi Ware */
	DRIVER( myangel )	/* (c) 1996 Namco */
	DRIVER( myangel2 )	/* (c) 1997 Namco */
	DRIVER( pzlbowl )	/* (c) 1999 Nihon System / Moss */
	DRIVER( penbros )	/* (c) 2000 Subsino */
	DRIVER( deerhunt )	/* (c) 2000 Sammy USA Corporation */
	DRIVER( deerhunta )	/* (c) 2000 Sammy USA Corporation */
	DRIVER( deerhuntb )	/* (c) 2000 Sammy USA Corporation */
	DRIVER( deerhuntc )	/* (c) 2000 Sammy USA Corporation */
	DRIVER( turkhunt )	/* (c) 2001 Sammy USA Corporation */
	DRIVER( wschamp )	/* (c) 2001 Sammy USA Corporation */
	DRIVER( wschampa )	/* (c) 2001 Sammy USA Corporation */
	DRIVER( funcube2 )	/* (c) 2001 Namco */
	DRIVER( funcube4 )	/* (c) 2001 Namco */
	DRIVER( trophyh )	/* (c) 2002 Sammy USA Corporation */

	/* SSV System (Seta, Sammy, Visco) games */
	DRIVER( srmp4 )		/* (c) 1993 Seta */
	DRIVER( srmp4o )	/* (c) 1993 Seta */
	DRIVER( twineag2 )	/* (c) 1994 Seta */
	DRIVER( srmp7 )		/* (c) 1997 Seta */
	DRIVER( dynagear )	/* (c) 1993 Sammy */
	DRIVER( survarts )	/* (c) 1993 Sammy */
	DRIVER( survartsu )	/* (c) 1993 Sammy (American) */
	DRIVER( eaglshot )	/* (c) 1994 Sammy */
	DRIVER( hypreact )	/* (c) 1995 Sammy */
	DRIVER( meosism )	/* (c) 1996?Sammy */
	DRIVER( hypreac2 )	/* (c) 1997 Sammy */
	DRIVER( sxyreact )	/* (c) 1998 Sammy */
	DRIVER( sxyreac2 )	/* (c) 1999 Sammy */
	DRIVER( cairblad )	/* (c) 1999 Sammy */
	DRIVER( keithlcy )	/* (c) 1993 Visco */
	DRIVER( drifto94 )	/* (c) 1994 Visco */
	DRIVER( janjans1 )	/* (c) 1996 Visco */
	DRIVER( stmblade )	/* (c) 1996 Visco */
	DRIVER( jsk )		/* (c) 1997 Visco */
	DRIVER( koikois2 )	/* (c) 1997 Visco */
	DRIVER( mslider )	/* (c) 1997 Visco / Datt Japan */
	DRIVER( ryorioh )	/* (c) 1998 Visco */
	DRIVER( janjans2 )	/* (c) 2000 Visco */
	DRIVER( vasara )	/* (c) 2000 Visco */
	DRIVER( vasara2 )	/* (c) 2001 Visco */
	DRIVER( vasara2a )	/* (c) 2001 Visco */

	DRIVER( ultrax )	/* (c) 1995 Banpresto */
	DRIVER( gdfs )		/* (c) 1995 Banpresto */

	DRIVER( jclub2 )	/* (c) 199? Seta */
	DRIVER( jclub2o )	/* (c) 199? Seta */
	DRIVER( darkhors )	/* bootleg of Jockey Club II */

	/* ST-0016 based games (ST-0016 is a z80 / gfx / sound all in one) */
	/* plain ST-0016 */
	DRIVER( renju )		/* (c) 1994 Visco */
	DRIVER( nratechu )	/* (c) 1996 Seta */
	/* R3000 + ST-0016 */
	DRIVER( srmp5 )		/* (c) 199? Seta */
	DRIVER( speglsht )	/* (c) 1994 Seta */
	/* V810 + ST-0016 */
	DRIVER( mayjinsn )	/* (c) 1994 Seta */
	DRIVER( mayjisn2 )	/* (c) 1994 Seta */
	DRIVER( koikois )	/* (c) 1995 Seta */

	/* Seta System S12 */
	DRIVER( srmp6 )		/* (c) 1996 Seta (Japan) */

	/* Seta Aleck64 and similar (N64 based) */
	DRIVER( aleck64 )	/* (c) 1998 Nintendo/Seta */
	DRIVER( 11beat )	/* (c) 1998 Hudson */
	DRIVER( mtetrisc )	/* (c) 1998 Capcom */
	DRIVER( starsldr )	/* (c) 1998 Hudson/Seta */
	DRIVER( vivdolls )	/* (c) 1998 Visco */
	DRIVER( srmvs )		/* (c) 1999 Seta */
	DRIVER( twrshaft )	/* (c) 2003 Aruze */
	DRIVER( hipai )		/* (c) 2003 Seta/Aruze (Arcade TV Game List - P.92, Right, 16 from top) */
	DRIVER( doncdoon )	/* (c) 2003 Aruze (Arcade TV Game List - P.90, Right, 8 from bottom) */
	DRIVER( kurufev )	/* (c) 2003 Aruze (Arcade TV Game List - P.75, Right, 1st entry on top) */
	DRIVER( mayjin3 )	/* (c) 2000 Seta (Arcade TV Game List - P.101, Right, 9 from top) */


	/* Atlus games */
	DRIVER( powerins )	/* (c) 1993 Atlus (USA) */
	DRIVER( powerinsj )	/* (c) 1993 Atlus (Japan) */
	DRIVER( powerinsa )	/* (c) 1993 Atlus (bootleg of USA version) */
	DRIVER( powerinsb )	/* (c) 1993 Atlus (bootleg of USA version) */
	DRIVER( ohmygod )	/* (c) 1993 Atlus (Japan) */
	DRIVER( naname )	/* (c) 1994 Atlus (Japan) */
	DRIVER( blmbycar )	/* (c) 1994 ABM & Gecas - uses same gfx chip as powerins? */
	DRIVER( blmbycaru )	/* (c) 1994 ABM & Gecas - uses same gfx chip as powerins? */
	DRIVER( watrball )	/* (c) 1996 ABM */

	/* Sun Electronics / SunSoft games */
	DRIVER( speakres )	/* [Sun Electronics] */
	DRIVER( stratvox )	/* [1980 Sun Electronics] Taito */
	DRIVER( stratvoxb )	/* bootleg */
	DRIVER( spacecho )	/* bootleg */
	DRIVER( spacecho2 )	/* bootleg */
	DRIVER( route16 )	/* (c) 1981 Tehkan/Sun + Centuri license */
	DRIVER( route16a )	/* (c) 1981 Tehkan/Sun + Centuri license */
	DRIVER( route16b )	/* bootleg */
	DRIVER( routex )	/* bootleg */
	DRIVER( ttmahjng )	/* Taito */
	DRIVER( fnkyfish )	/* (c) 1981 Sun Electronics */
	DRIVER( kangaroo )	/* (c) 1982 Sun Electronics */
	DRIVER( kangarooa )	/* 136008           (c) 1982 Atari */
	DRIVER( kangaroob )	/* (bootleg) */
	DRIVER( arabian )	/* TVG13 (c) 1983 Sun Electronics */
	DRIVER( arabiana )	/* 136019           (c) 1983 Atari */
	DRIVER( markham )	/* TVG14 (c) 1983 Sun Electronics */
	DRIVER( strnskil )	/* TVG15 (c) 1984 Sun Electronics */
	DRIVER( guiness )	/* TVG15 (c) 1984 Sun Electronics */
	DRIVER( pettanp )	/* TVG16 (c) 1984 Sun Electronics (Japan) */
	DRIVER( banbam )	/* TVG16 (c) 1984 Sun Electronics */
	DRIVER( ikki )		/* TVG17 (c) 1985 Sun Electronics (Japan) */
	DRIVER( farmer )	/* TVG17 (c) 1985 Sun Electronics */
	DRIVER( shanghai )	/* (c) 1988 Sunsoft (Sun Electronics) */
	DRIVER( shangha2 )	/* (c) 1989 Sunsoft (Sun Electronics) */
	DRIVER( kothello )	/* (c) 1990 Success */
	DRIVER( shangha3 )	/* (c) 1993 Sunsoft */
	DRIVER( heberpop )	/* (c) 1994 Sunsoft / Atlus */
	DRIVER( blocken )	/* (c) 1994 KID / Visco */
/*
Other Sun games
1978.11 (GT)Block Perfect
1978.12 (GT)Block Challenger
1979.04 Galaxy Force
1979.07 Run Away
1979.09 Dai San Wakusei (The Third Planet)
1979.12 Warp 1
1980.10 Cosmo Police (Cosmopolis?)
*/

	/* SunA games */
	DRIVER( goindol )	/* (c) 1987 Sun a Electronics */
	DRIVER( goindolu )	/* (c) 1987 Sun a Electronics */
	DRIVER( goindolk )	/* (c) 1987 Sun a Electronics */
	DRIVER( rranger )	/* (c) 1988 SunA + Sharp Image license */
	DRIVER( sranger )	/* (c) 1988 */
	DRIVER( srangerb )	/* bootleg */
	DRIVER( srangerw )	/* (c) 1988 SunA (WDK License) */
	DRIVER( hardhead )	/* (c) 1988 SunA */
	DRIVER( hardheadb )	/* bootleg */
	DRIVER( pop_hh )	/* bootleg */
	DRIVER( sparkman )	/* (c) 1989 SunA */
	DRIVER( sparkmana )	/* (c) 1989 SunA */
	DRIVER( starfigh )	/* (c) 1990 SunA */
	DRIVER( hardhea2 )	/* (c) 1991 SunA */
	DRIVER( brickzn )	/* (c) 1992 SunA */
	DRIVER( brickzn3 )	/* (c) 1992 SunA */
	DRIVER( bestbest )	/* (c) 1994 SunA */
	DRIVER( sunaq )		/* (c) 1994 SunA */
	DRIVER( bssoccer )	/* (c) 1996 SunA */
	DRIVER( uballoon )	/* (c) 1996 SunA */

	/* Dooyong games */
	DRIVER( gundealr )	/* (c) 1990 Dooyong */
	DRIVER( gundealra )	/* (c) 1990 Dooyong */
	DRIVER( gundealrt )	/* (c) 1990 Tecmo */
	DRIVER( yamyam )	/* (c) 1990 Dooyong */
	DRIVER( wiseguy )	/* (c) 1990 Dooyong */
	DRIVER( lastday )	/* (c) 1990 Dooyong */
	DRIVER( lastdaya )	/* (c) 1990 Dooyong */
	DRIVER( gulfstrm )	/* (c) 1991 Dooyong */
	DRIVER( gulfstrmm )	/* (c) 1991 Dooyong + distributed by Media Shoji */
	DRIVER( pollux )	/* (c) 1991 Dooyong */
	DRIVER( polluxa )	/* (c) 1991 Dooyong */
	DRIVER( polluxa2 )	/* (c) 1991 Dooyong */
	DRIVER( flytiger )	/* (c) 1992 Dooyong */
	DRIVER( bluehawk )	/* (c) 1993 Dooyong */
	DRIVER( bluehawkn )	/* (c) 1993 NTC */
	DRIVER( sadari )	/* (c) 1993 NTC */
	DRIVER( gundl94 )	/* (c) 1994 Dooyong */
	DRIVER( primella )	/* (c) 1994 NTC */
	DRIVER( superx )	/* (c) 1994 NTC */
	DRIVER( superxm )	/* (c) 1994 Mitchell */
	DRIVER( rshark )	/* (c) 1995 Dooyong */
	DRIVER( popbingo )	/* (c) 1996 Dooyong */

	/* Tong Electronic games */
	DRIVER( leprechn )	/* (c) 1982 */
	DRIVER( potogold )	/* (c) 1982 */
	DRIVER( leprechp )	/* (c) 1982 */
	DRIVER( piratetr )	/* (c) 1982 */
	DRIVER( beezer )	/* (c) 1982 */
	DRIVER( beezer1 )	/* (c) 1982 */

	/* Comad games */
	DRIVER( pushman )	/* (c) 1990 Comad */
	DRIVER( pushmana )	/* (c) 1990 Comad */
	DRIVER( pushmans )	/* (c) 1990 Comad + American Sammy license */
	DRIVER( pushmant )	/* (c) 1990 Comad + Top Tronic license */
	DRIVER( bballs )	/* (c) 1991 Comad */
	DRIVER( zerozone )	/* (c) 1993 Comad */
	DRIVER( lvgirl94 )	/* (c) 1994 Comad */
	DRIVER( hotpinbl )	/* (c) 1995 Comad & New Japan System */
	DRIVER( galspnbl )	/* (c) 1996 Comad */
		/* the following ones run on modified Gals Panic hardware */
	DRIVER( fantasia )	/* (c) 1994 Comad & New Japan System */
	DRIVER( supmodel )	/* (c) 1994 Comad & New Japan System */
	DRIVER( newfant )	/* (c) 1995 Comad & New Japan System */
	DRIVER( fantsy95 )	/* (c) 1995 Hi-max Technology Inc. */
	DRIVER( zipzap )	/* (c) 1995 Barko Corp */
	DRIVER( missw96 )	/* (c) 1996 Comad */
	DRIVER( missmw96 )	/* (c) 1996 Comad */
	DRIVER( fantsia2 )	/* (c) 1997 Comad */
	DRIVER( fantsia2a )	/* (c) 1997 Comad */
	DRIVER( wownfant )	/* (c) 2002 Comad */
	DRIVER( galhustl )	/* (c) 1997 ACE International */

	/* Playmark games */
	DRIVER( sslam )		/* (c) 1993 - hack of Namco's Super World Court */
	DRIVER( sslama )	/* (c) 1993 - hack of Namco's Super World Court */
	DRIVER( powerbal )	/* (c) 1994 */
	DRIVER( powerbals )	/* (c) 1994 */
	DRIVER( magicstk )	/* (c) 1995 */
	DRIVER( hotminda )	/* (c) 1995 */
	DRIVER( bigtwin )	/* (c) 1995 */
	DRIVER( wbeachvl )	/* (c) 1995 */
	DRIVER( wbeachvl2 )	/* (c) 1995 */
	DRIVER( excelsr )	/* (c) 1995 */
	DRIVER( hotmind )	/* (c) 1995 */
	DRIVER( hrdtimes )	/* (c) 1994 */
	DRIVER( hrdtimesa )	/* (c) 1994 */
	DRIVER( drtomy )	/* (c) 1993 no copyright message (1993 on pcb), some code taken from 'Big Karnak' */
	DRIVER( sderby )	/* (c) 1996 */
	DRIVER( pmroulet )
	DRIVER( spacewin )

	/* Pacific Novelty games */
	DRIVER( sharkatt )	/* (c) 1980 */
	DRIVER( thief )		/* (c) 1981 */
	DRIVER( natodef )	/* (c) 1982 */
	DRIVER( natodefa )	/* (c) 1982 */
	DRIVER( mrflea )	/* (c) 1982 */

	/* Tecfri games */
	DRIVER( ambush )	/* (c) 1983 */
	DRIVER( ambushj )	/* (c) 1983 Nippon Amuse license */
	DRIVER( ambushv )	/* (c) 1983 Volt Electronics license */
	DRIVER( holeland )	/* (c) 1984 */
	DRIVER( crzrally )	/* (c) 1985 */
	DRIVER( crzrallya )	/* (c) 1985 */
	DRIVER( crzrallyg )	/* (c) 1985 */
	DRIVER( speedbal )	/* (c) 1987 */
	DRIVER( sauro )		/* (c) 1987 */
	DRIVER( trckydoc )	/* (c) 1987 */
	DRIVER( trckydoca )	/* (c) 1987 */

	/* Metro games */
	DRIVER( karatour )	/* (c) 1992 Mitchell */
	DRIVER( ladykill )	/* (c) 1993 Yanyaka + Mitchell license - (Arcade TV Game List - P.101, Right, 2 from Bottom) */
	DRIVER( moegonta )	/* (c) 1993 Yanyaka (Japan) (Arcade TV Game List - P.101, Right, 2 from Bottom) */
	DRIVER( pangpoms )	/* (c) 1992 */
	DRIVER( pangpomsm )	/* (c) 1992 Mitchell / Metro */
	DRIVER( skyalert )	/* (c) 1992 */
	DRIVER( poitto )	/* (c) 1993 Metro / Able Corp. */
	DRIVER( dharma )	/* (c) 1994 */
	DRIVER( dharmak )	/* (c) 1994 */
	DRIVER( lastfort )	/* (c) 1994 */
	DRIVER( lastforte )	/* (c) 1994 */
	DRIVER( lastfortk )	/* (c) 1994 */
	DRIVER( lastfortg )	/* (c) 1994 */
	DRIVER( toride2g )	/* (c) 1994 */
	DRIVER( toride2gg )	/* (c) 1994 */
	DRIVER( toride2j )	/* (c) 1994 */
	DRIVER( gunmast )	/* (c) 1994 */
	DRIVER( daitorid )	/* (c) 1995 */
	DRIVER( daitoa )	/* (c) 1995 */
	DRIVER( dokyusei )	/* (c) 1995 Make Software / Elf / Media Trading */
	DRIVER( dokyusp )	/* (c) 1995 Make Software / Elf / Media Trading */
	DRIVER( msgogo )	/* (c) 1995 Metro */
	DRIVER( puzzli )	/* (c) 1995 Metro / Banpresto */
	DRIVER( 3kokushi )	/* (c) 1996 Mitchell */
	DRIVER( pururun )	/* (c) 1995 Metro / Banpresto */
	DRIVER( balcube )	/* (c) 1996 */
	DRIVER( mouja )		/* (c) 1996 Etona (Japan) */
	DRIVER( bangball )	/* (c) 1996 Banpresto / Kunihiko Tashiro+Goodhouse */
	DRIVER( batlbubl )	/* (c) 1996 Banpresto / Kunihiko Tashiro+Goodhouse */
	DRIVER( gakusai )	/* (c) 1997 MakeSoft */
	DRIVER( gakusai2 )	/* (c) 1998 MakeSoft */
	DRIVER( blzntrnd )	/* (c) 1994 Human Amusement */
	DRIVER( gstrik2 )	/* (c) 1996 Human Amusement */
	DRIVER( gstrik2j )
	DRIVER( hyprduel )	/* (c) 1993 Technosoft (Japan) */
	DRIVER( hyprduel2 )	/* (c) 1993 Technosoft (Japan) */
	DRIVER( magerror )	/* (c) 1994 Technosoft (Japan) (Arcade TV Game List - P.100, Left, 13 from Bottom) */
	DRIVER( puzzlet )	/* (c) 2000 Yunizu Corporation (Japan) */

	/* Venture Line games */
	DRIVER( spcforce )	/* (c) 1980 Venture Line */
	DRIVER( spcforc2 )	/* bootleg */
	DRIVER( meteor )	/* (c) 1981 Venture Line */
	DRIVER( looping )	/* (c) 1982 Video Games GMBH */
	DRIVER( loopingv )	/* (c) 1982 Video Games GMBH (Venture Line license) */
	DRIVER( loopingva )	/* (c) 1982 Video Games GMBH (Venture Line license) */
	DRIVER( skybump )	/* (c) 1982 Venture Line */
	DRIVER( suprridr )	/* (c) 1983 Venture Line + Taito license */

	/* Yun Sung games */
	DRIVER( paradise )	/* (c) >1994 Yun Sung */
	DRIVER( paradlx )	/* (c) >1994 Yun Sung */
	DRIVER( para2dx )	/* (c) >1994 Yun Sung */
	DRIVER( tgtball )	/* (c) 1995 Yun Sung */
	DRIVER( tgtballa )	/* (c) 1995 Yun Sung */
	DRIVER( torus )		/* (c) 1996 Yun Sung */
	DRIVER( paprazzi )	/* (c) 1996 Yun Sung */
	DRIVER( madball )	/* (c) 1998 Yun Sung */
	DRIVER( madballn )	/* (c) 1998 Yun Sung */
	DRIVER( cannball )	/* (c) 1995 Yun Sung / Soft Visio */
	DRIVER( cannballv )	/* (c) 1995 Yun Sung / T&K */
	DRIVER( magix )		/* (c) 1995 Yun Sung */
	DRIVER( rocktris )	/* (c) 1994? Yun Sung */
	DRIVER( magicbub )	/* (c) Yun Sung */
	DRIVER( magicbuba )	/* (c) Yun Sung */
	DRIVER( shocking )	/* (c) 1997 Yun Sung */
	DRIVER( bombkick )	/* (c) 1998 Yun Sung */
	DRIVER( nmg5 )		/* (c) 1998 Yun Sung */
	DRIVER( nmg5e )		/* (c) 1998 Yun Sung */
	DRIVER( searchey )	/* (c) 1999 Yun Sung */
	DRIVER( searchp2 )	/* (c) 1999 Yun Sung */
	DRIVER( pclubys )	/* (c) 2000 Yun Sung */
	DRIVER( pclubysa )	/* (c) 2000 Yun Sung */
	DRIVER( garogun )	/* (c) 2000 Yun Sung */
	DRIVER( 7ordi )		/* (c) 2002 Yun Sung */
	DRIVER( wondstck )	/* (c) ???? Yun Sung */

	/* Zilec games */
	DRIVER( blueprnt )	/* (c) 1982 Bally Midway (Zilec in ROM 3U, and the programmer names) */
	DRIVER( blueprntj )	/* (c) 1982 Jaleco (Zilec in ROM 3U, and the programmer names) */
	DRIVER( saturn )	/* (c) 1983 Jaleco (Zilec in ROM R6, and the programmer names) */

	/* Fuuki FG-2 games */
	DRIVER( gogomile )	/* (c) 1995 */
	DRIVER( gogomilej )	/* (c) 1995 (Japan) */
	DRIVER( pbancho )	/* (c) 1996 (Japan) */

	/* Fuuki FG-3 games */
	DRIVER( asurabld )	/* (c) 1998 (Japan) */
	DRIVER( asurabus )	/* (c) 2000 (Japan) */

	/* Unico games */
	DRIVER( drgnmst )	/* (c) 1994 */
	DRIVER( burglarx )	/* (c) 1997 */
	DRIVER( zeropnt )	/* (c) 1998 */
	DRIVER( zeropnta )	/* (c) 1998 */
	DRIVER( zeropnt2 )	/* (c) 1999 */
	DRIVER( silkroad )	/* (c) 1999 */
	DRIVER( aoh )		/* (c) 2001 */

	/* Afega games */
	DRIVER( twinactn )	/* (c) 1996 - bootleg of US AFF Mustang */
	DRIVER( stagger1 )	/* (c) 1998 */
	DRIVER( redhawk )	/* (c) 1997 */
	DRIVER( redhawke )	/* (c) 1997 Excellent Co., Ltd license */
	DRIVER( redhawki )	/* (c) 1997 Haedong Corp. license */
	DRIVER( redhawkb )	/* (c) 1997 */
	DRIVER( grdnstrm )	/* (c) 1998 */
	DRIVER( grdnstrmk )	/* (c) 1998 */
	DRIVER( bubl2000 )	/* (c) 1998 Tuning */
	DRIVER( hotbubl )	/* (c) 1998 Pandora */
	DRIVER( popspops )	/* (c) 1999 */
	DRIVER( firehawk )	/* (c) 2001 ESD */
	DRIVER( mangchi )	/* (c) 2000 */
	DRIVER( spec2k )	/* (c) 2000 Yonatech */

	/* ESD games */
	/* http://www.esdgame.co.kr/english/ */
	DRIVER( multchmp )	/* (c) 1998 (World) */
	DRIVER( multchmpk )	/* (c) 1998 (Korea) */
	DRIVER( mchampdx )	/* (c) 1999 ESD */
	DRIVER( mchampdxa )	/* (c) 1999 ESD */
	DRIVER( hedpanic )	/* (c) 2000 ESD */
	DRIVER( hedpanicf )	/* (c) 2000 ESD + Fuuki, Co. Ltd. */
	DRIVER( hedpanico )	/* (c) 1999 ESD */
	DRIVER( tangtang )	/* (c) 2000 ESD */
	DRIVER( swatpolc )	/* (c) 2001 ESD */

	/* Dyna Electronics / Dynax / Nakanihon games */
	DRIVER( royalmj )	/* (c) 1982 Nichibutsu */
	DRIVER( openmj )	/* (c) 1982 Nichibutsu */
	DRIVER( royalmah )	/* Falcon bootleg */
	DRIVER( janyoup2 )	/* Falcon bootleg */
	DRIVER( janputer )	/* (c) 1982 Taito */
	DRIVER( janoh )		/* (c) 1984 Toaplan */
	DRIVER( janoha )	/* (c) 1984 Toaplan */
	DRIVER( jansou )	/* (c) 1985 Dyna */
	DRIVER( jansoua )	/* (c) 1985 Dyna */
	DRIVER( mjsiyoub )	/* (c) 1986 Visco */
	DRIVER( mjsenka )	/* (c) 1986 Visco */
	DRIVER( mjyarou )	/* (c) 1986 Visco / Video System */
	DRIVER( mjclub )	/* (c) 1986 Xex */
	DRIVER( daisyari )	/* (c) 1989 Best System */
	DRIVER( suzume )	/*  ??  (c) 1986 Dyna Electronics */
	DRIVER( hnayayoi )	/* "02" (c) 1987 Dyna Electronics */
	DRIVER( dondenmj )	/* "03" (c) 1986 Dyna Electronics */
	DRIVER( ippatsu )	/*  ??  (c) 1986 Dyna Electronics */
	DRIVER( hnfubuki )	/* "06" (c) 1987 Dynax */
	DRIVER( mjdiplob )	/* "07" (c) 1987 Dynax */
	DRIVER( untoucha )	/* "08" (c) 1987 Dynax */
	DRIVER( tontonb )	/* "09" (c) 1987 Dynax */
	DRIVER( makaijan )	/*  ??  (c) 1987 Dynax */
	DRIVER( hanamai )	/* "16" (c) 1988 Dynax */
	DRIVER( majs101b )	/* "17" (c) [1988] Dynax */
	DRIVER( mjapinky )	/* 1988 Dynax */
	DRIVER( mjdejavu )	/* 1989 Dynax */
	DRIVER( mjdejav2 )	/* 1989 Dynax */
	DRIVER( hnkochou )	/* "20" (c) 1989 Dynax */
	DRIVER( mjderngr )	/* "22" (c) 1989 Dynax */
	DRIVER( hnoridur )	/* "23" (c) 1989 Dynax */
	DRIVER( drgpunch )	/* "24" (c) 1989 Dynax */
	DRIVER( mjfriday )	/* "26" (c) [1989] Dynax */
	DRIVER( jantouki )	/* "27" Jantouki 1989 Dynax */
	DRIVER( gekisha )	/* "28" 1989 Dynax */
	DRIVER( mjifb )		/* "29" 1990 Dynax */
	DRIVER( mjifb2 )	/* "29" 1990 Dynax */
	DRIVER( mjifb3 )	/* "29" 1990 Dynax */
	DRIVER( mjvegas )	/* 1991 Dynax */
	DRIVER( mjvegasa )	/* 1991 Dynax */
	DRIVER( cafetime )	/* 1992 Dynax */
	DRIVER( cafedoll )	/* 1993 Dynax */
	DRIVER( janptr96 )	/* 1996 Dynax */
	DRIVER( sprtmtch )	/* "31" (c) 1989 Dynax + Fabtek license */
	DRIVER( maya )		/* (c) 1994 Promat */
	DRIVER( mayaa )		/* (c) 1994 Promat */
	DRIVER( inca )		/* (c) 199? ???? */
	DRIVER( blktouch )	/* (c) Yang Gi Co Ltd. */
	DRIVER( mcnpshnt )	/* "33" Mahjong Campus Hunting 1990 Dynax */
	DRIVER( 7jigen )	/* "37" 7jigen no Youseitachi 1990 Dynax */
	DRIVER( neruton )	/* "45" Neruton Haikujiradan 1990 Dynax */
	DRIVER( mjcomv1 )	/* 1991 Dynax */
	DRIVER( shpeng )	/* (c) 1995 WSAC Systems */
	DRIVER( tenkai )	/* 1991 Dynax */
	DRIVER( tenkai2b )	/* 1991 Dynax */
	DRIVER( tenkaibb )	/* 1991 Dynax */
	DRIVER( tenkaicb )	/* 1991 Dynax */
	DRIVER( tenkaid )	/* 1991 Dynax */
	DRIVER( tenkaie )	/* 1991 Dynax */
	DRIVER( htengoku )	/* 1992 Dynax */
	DRIVER( mjreach )	/* 1994 Dynax */
	DRIVER( mjdialq2 )	/* "52" (c) 1991 Dynax */
	DRIVER( yarunara )	/* "55" Mahjong Yarunara 1991 Dynax */
	DRIVER( mjangels )	/* "61" Mahjong Angels 1991 Dynax */
	DRIVER( quiztvqq )	/* "64" Quiz TV Gassyuukoku Q&Q 1992 Dynax */
	DRIVER( mmpanic )	/* "70" 1992 Nakanihon/Taito */
	DRIVER( quizchq )	/* "73" (c) 1993 Nakanihon */
	DRIVER( quizchql )	/* "73" (c) 1993 Laxan */
	DRIVER( funkyfig )	/* "74" (c) 1993 Nakanihon + East Technology */
	DRIVER( animaljr )	/* "75" 1993 Nakanihon/Taito (USA) */
	DRIVER( animaljrs )	/* "75" 1993 Nakanihon/Taito (Spanish version) */
	DRIVER( animaljrj )	/* "75" 1993 Nakanihon/Taito (Japan) */
	DRIVER( hginga )	/*      1994 Dynax */
	DRIVER( mjmyster )	/* "?7" 1994 Dynax */
	DRIVER( mjmywrld )	/* "?7" 1994 Dynax */
	DRIVER( mjmyornt )	/*      1994 Dynax */
	DRIVER( mjmyuniv )	/*      1994 Dynax */
	DRIVER( quiz365 )	/* "78" (c) 1994 Nakanihon */
	DRIVER( quiz365t )	/* "78" (c) 1994 Nakanihon (Taiwan) */
	DRIVER( rongrong )	/* "80" (c) 1994 Nakanihon */
	DRIVER( rongrongj )	/* "80" (c) 1994 Nakanihon */
	DRIVER( rongrongg )	/* "80" (c) 1994 Nakanihon */
	DRIVER( hparadis )	/*      (c) 1994 Nakanihon */
	DRIVER( hgokou )	/*      (c) 1995 Nakanihon */
	DRIVER( mjdchuka )	/*      (c) 1995 Nakanihon */
	DRIVER( mjtensin )	/* "100" 1995 Dynax */
	DRIVER( nettoqc )	/* "103" (c) 1995 Nakanihon */
	DRIVER( majrjhdx )	/* "105" 1996 Dynax */
	DRIVER( ddenlovj )	/* "113" (c) 1995 Dynax (Japan) */
	DRIVER( ddenlovrk )	/* "113" (c) 1995 Dynax (Korea) */
	DRIVER( ddenlovr )	/* "113" (c) 1996 Dynax (Hong Kong) */
	DRIVER( ddenlovrb )	/* bootleg */
	DRIVER( hanakanz )	/* "507" 1996 Dynax */
	DRIVER( akamaru )	/* "501" 1996 Dynax */
	DRIVER( sryudens )	/* "502" 1996 Dynax / Face */
	DRIVER( mjflove )	/*       1996 Dynax */
	DRIVER( hkagerou )	/* "510" Hana Kagerou 1996 Nakanihon */
	DRIVER( realbrk )	/* "600" Billiard Academy Real Break 1998 (Europe)*/
	DRIVER( realbrkk )	/* "600" Billiard Academy Real Break 1998 (Korea)*/
	DRIVER( realbrkj )	/* "523" Billiard Academy Real Break 1998 (Japan)*/
	DRIVER( dai2kaku )	/* "522" DaiDaiKakumei (Japan)*/
	DRIVER( mjchuuka )	/* "???" (c) 1998 Dynax */
	DRIVER( mjreach1 )	/* "526" Mahjong Reach Ippatsu 1998 Nihon System/Dynax */
	DRIVER( cafebrk )	/* "528" Mahjong Cafe Break (C) 1999 Nakanihon / Dynax */
	DRIVER( jongtei )	/* "532" Mahjong Jong-Tei (C) 1999 Dynax */
	DRIVER( daimyojn )	/* 2002 Dynax / Techno-Top */

	DRIVER( pkgnsh )	/* "505" Pachinko Gindama Shoubu 1998 (Japan) */
	DRIVER( pkgnshdx )	/* "522" Pachinko Gindama Shoubu DX 1998 (Japan) */

	DRIVER( hjingi )	/* "???" 1990 Dynax */
	DRIVER( hanayara )	/* "???" */
	DRIVER( mjelctrn )	/* "???" Mahjong Electron Base parts 2 & 4 */
	DRIVER( mjelct3 )	/* "???" Mahjong Electron Base parts 2 & 3 */
	DRIVER( mjelct3a )	/* "???" Mahjong Electron Base parts 2 & 3 */
	DRIVER( mjelctrb )	/* (c) 1993 Dynax Mahjong Electron Base parts  4 */
	DRIVER( majxtal7 )	/* "???" */

	/* Sigma games */
	DRIVER( nyny )		/* (c) 1980 Sigma Ent. Inc. */
	DRIVER( nynyg )		/* (c) 1980 Sigma Ent. Inc. + Gottlieb */
	DRIVER( arcadia )	/* (c) 1982 Sigma Ent. Inc. */
	DRIVER( r2dtank )	/* (c) 1980 Sigma Ent. Inc. */
	DRIVER( spiders )	/* (c) 1981 Sigma Ent. Inc. */
	DRIVER( spiders2 )	/* (c) 1981 Sigma Ent. Inc. */
	DRIVER( spiders3 )	/* (c) 1981 Sigma Ent. Inc. */
	DRIVER( spinner )	/* bootleg */
	DRIVER( sub )		/* (c) 1985 Sigma Ent. Inc. */

	/* Sigma B52 */
	DRIVER( jwildb52 )	/* (c) 199? Sigma */
	DRIVER( jwildb52a )	/* (c) 199? Sigma */
	DRIVER( jwildb52h )	/* (c) 199? Sigma */

	/* Sigma B98 */
	DRIVER( gegege )	/* (c) 1997 Banpresto / Sigma */

	/* IGS games ( www.igs.com.tw ) */
	DRIVER( goldstar )	/* (c) 198? IGS */
	DRIVER( goldstbl )	/* (c) 198? IGS */
	DRIVER( moonlght )	/* bootleg */
	DRIVER( chry10 )	/* bootleg */
	DRIVER( chrygld )	/* bootleg */
	DRIVER( cabaret )	/* (c) 1992 AMT */
	DRIVER( cpoker )	/* (c) 1993? IGS */
	DRIVER( cpokert )	/* (c) 1993? Tuning */
	DRIVER( csk227it )	/* (c) 198? IGS */
	DRIVER( csk234it )	/* (c) 198? IGS */
	DRIVER( igs_ncs )	/* (c) 198? IGS */
	DRIVER( igs_ncs2 )	/* (c) 2000 IGS */
	DRIVER( spk115it )	/* (c) 199? IGS */
	DRIVER( spk116it )	/* (c) 199? IGS */
	DRIVER( jackie )	/* (c) 1993 IGS */
	DRIVER( pktet346 )	/* (c) 199? IGS */
	DRIVER( chleague )	/* (c) 199? IGS */
	DRIVER( chleagul )	/* (c) 199? IGS */
	DRIVER( number10 )	/* (c) IGS (Board labeled Playmark Srl) */
	DRIVER( numbr10l )	/* (c) IGS (Board labeled Playmark Srl) */
	DRIVER( cpokerpk )	/* (c) SGS (bootleg blue board) */
	DRIVER( cpokerpkg )	/* (c) SGS (bootleg green board) */
	DRIVER( citalcup )	/* (c) SGS (bootleg of IGS v220it) */
	DRIVER( stellecu )	/* (c) 1998 */
	DRIVER( iqblock )	/* (c) 1993 */
	DRIVER( grndtour )	/* (c) 1993 */
	DRIVER( 5clown )	/* (c) IGS 1993 */
	DRIVER( 5clowna )	/* (c) IGS 1993 */
	DRIVER( 5clownsp )	/* (c) IGS 1993 */

	/* IGS005 + IGS006 + IGS007 + IGS008 Games */
	DRIVER( lordgun )	/* (c) 1994 */
	DRIVER( aliencha )	/* (c) 1994 */
	DRIVER( alienchac )	/* (c) 1994 */

	/* IGS009 Games */
	DRIVER( jingbell )	/* (c) 1995? */
	DRIVER( gp98 )		/* (c) 1998 */

	/* IGS011 Games */
	DRIVER( lhb )		/* (c) 1995 */
	DRIVER( lhbv33c )	/* (c) 1995 */
	DRIVER( ryukobou )	/* (c) 1995 */
	DRIVER( dbc )		/* (c) 199? */
	DRIVER( lhb2 )		/* (c) 1996 */
	DRIVER( xymg )		/* (c) 1996 */
	DRIVER( wlcc )		/* (c) 1996 */
	DRIVER( vbowl )		/* (c) 1996 */
	DRIVER( vbowlj )	/* (c) 1996 */
	DRIVER( drgnwrld )	/* (c) 1997 */
	DRIVER( drgnwrldv30 )	/* (c) 1995 */
	DRIVER( drgnwrldv21 )	/* (c) 1995 */
	DRIVER( drgnwrldv21j )	/* (c) 1995 */
	DRIVER( drgnwrldv20j )	/* (c) 1995 */
	DRIVER( drgnwrldv10c )	/* (c) 1995 */
	DRIVER( drgnwrldv11h )	/* (c) 1995 */
	DRIVER( nkishusp )	/* (c) 1998 */

	/* IGS017 / IGS031 Games */
	DRIVER( iqblocka )	/* (c) 1996 */
	DRIVER( iqblockf )	/* (c) 1996 */
	DRIVER( tjsb )		/* (c) 1997 */
	DRIVER( sdmg2 )		/* (c) 1997 */
	DRIVER( mgdh )		/* (c) 1997 */
	DRIVER( mgcs )		/* (c) 1998 */
	DRIVER( lhzb2 )		/* (c) 1998 */
	DRIVER( lhzb2a )	/* (c) 1998 */
	DRIVER( slqz2 )		/* (c) 1998 */
	DRIVER( tarzan )	/* (c) 1999? */
	DRIVER( tarzana )	/* (c) 1999? */
	DRIVER( starzan )	/* (c) 2000? */

	/* IGS027A Games */
	DRIVER( bigd2 )		/* (c) 2000 */
	DRIVER( sdwx )		/* (c) 2002 */
	DRIVER( sddz )		/* (c) 200? */
	DRIVER( lhzb3 )		/* (c) 200? */
	DRIVER( lhzb4 )		/* (c) 200? */
	DRIVER( klxyj )		/* (c) 200? */
	DRIVER( mgfx )		/* (c) 200? */

	/* IGS PGM System Games */
	DRIVER( pgm )
	DRIVER( orlegend )		/* (c) 1997 */
	DRIVER( orlegende )		/* (c) 1997 */
	DRIVER( orlegendc )		/* (c) 1997 */
	DRIVER( orlegendca )	/* (c) 1997 */
	DRIVER( orlegend111c )	/* (c) 1997 */
	DRIVER( orlegend105k )	/* (c) 1997 */
	DRIVER( drgw2 )			/* (c) 1997 */
	DRIVER( drgw2c )		/* (c) 1997 */
	DRIVER( drgw2j )		/* (c) 1997 */
	DRIVER( drgw3 )			/* (c) 1998 */
	DRIVER( drgw3105 )		/* (c) 1998 */
	DRIVER( drgw3100 )		/* (c) 1998 */
	DRIVER( dwex )			/* (c) 1998 */
	DRIVER( killbld )		/* (c) 1998 */
	DRIVER( killbld104 )	/* (c) 1998 */
	DRIVER( kov )			/* (c) 1999 */
	DRIVER( kovplus )		/* (c) 1999 */
	DRIVER( kovplusa )		/* (c) 1999 */
	DRIVER( kov115 )		/* (c) 1999 */
	DRIVER( kov100 )		/* (c) 1999 */
	DRIVER( kovsh )			/* (c) 1999 */
	DRIVER( kovsh103 )		/* (c) 1999 */
	DRIVER( photoy2k )		/* (c) 1999 */
	DRIVER( photoy2k102 )	/* (c) 1999 */
	DRIVER( photoy2k104 )	/* (c) 1999 */
	DRIVER( puzlstar )		/* (c) 1999 */
	DRIVER( olds )			/* (c) 1999 */
	DRIVER( olds100 )		/* (c) 1999 */
	DRIVER( olds100a )		/* (c) 1999 */
	DRIVER( kov2 )			/* (c) 2000 */
	DRIVER( kov2106 )		/* (c) 2000 */
	DRIVER( kov2103 )		/* (c) 2000 */
	DRIVER( kov2102 )		/* (c) 2000 */
	DRIVER( kov2100 )		/* (c) 2000 */
	DRIVER( kov2p )			/* (c) 2000 */
	DRIVER( kov2p205 )		/* (c) 2000 */
	DRIVER( ddp2 )			/* (c) 2001 */
	DRIVER( ddp2a )			/* (c) 2001 */
	DRIVER( puzzli2 )		/* (c) 2001 */
	DRIVER( martmast )		/* (c) 2001 */
	DRIVER( martmastc )		/* (c) 2001 */
	DRIVER( martmastc102 )	/* (c) 2001 */
	DRIVER( theglad )		/* (c) 2001 */
	DRIVER( theglada )		/* (c) 2001 */
	DRIVER( dw2001 )		/* (c) 2001 */
	DRIVER( py2k2 )			/* (c) 2001 */
	DRIVER( dmnfrnt )		/* (c) 2002 */
	DRIVER( dmnfrnta )		/* (c) 2002 */
	DRIVER( kovshp )
	DRIVER( oldsplus )
	DRIVER( killbldp )
	DRIVER( happy6 )
	DRIVER( svg )

	/* Spirit Tech. Corp. */
	DRIVER( dunhuang )	/* (c) 1995 */

	/* RamTek games */
	DRIVER( hitme )		/* (c) 1976 Ramtek */
	DRIVER( m21 )		/* (c) 1976 Mirco */
	DRIVER( barricad )	/* (c) 1976 Ramtek */
	DRIVER( brickyrd )	/* (c) 1976 Ramtek */
	DRIVER( starcrus )	/* (c) 1977 Ramtek */

	/* Omori games */
	DRIVER( shuttlei )	/* (c) 1979 Omori (Arcade Tv Game List - P.79, Left, 6 from bottom) */
	DRIVER( skylove )	/* (c) 1979 Omori */
	DRIVER( battlex )	/* (c) 1982 Omori E. Co., Ltd. */
	DRIVER( carjmbre )	/* (c) 1983 Omori Electric Co., Ltd. */
	DRIVER( popper )	/* (c) 1983 Omori Electric Co., Ltd. */

	/* TCH games */
	DRIVER( speedspn )	/* (c) 1994 */
	DRIVER( mstworld )	/* (c) 1994 */

	DRIVER( kickgoal )	/* (c) 1995 */
	DRIVER( actionhw )	/* (c) 1995 */

	/* U.S. Games games */
	DRIVER( usg32 )		/* (c) 1987 */
	DRIVER( usg82 )		/* (c) 1988 */
	DRIVER( usg83 )		/* (c) 1988 */
	DRIVER( usg83x )	/* (c) 1988 */
	DRIVER( usg182 )	/* (c) 1989 */
	DRIVER( usg185 )	/* (c) 1991 */
	DRIVER( usg252 )	/* (c) 1992 */

	/* Sanritsu games */
	DRIVER( mermaid )	/* (c) 1982 Rock-ola,  made by Sanritsu */
	DRIVER( yachtmn )	/* (c) 1982 [Sanritsu] Esco */
	DRIVER( rougien )	/* (c) 1982 Sanritsu */
	DRIVER( drmicro )	/* (c) 1983 Sanritsu */
	DRIVER( jantotsu )	/* (c) 1983 Sanritsu */
	DRIVER( appoooh )	/* (c) 1984 Sega,  made by Sanritsu */
	DRIVER( robowres )	/* (c) 1986 Sega */
	DRIVER( robowresb )	/* bootleg */
	DRIVER( bankp )		/* (c) 1984 Sega,  made by Sanritsu */
	DRIVER( combh )		/* (c) 1987 Sega,  made by Sanritsu */
	DRIVER( mjkjidai )	/* (c) 1986 Sanritsu */
	DRIVER( chinsan )	/* (c) 1987 Sanritsu */
	DRIVER( mayumi )	/* (c) 1988 Victory L.L.C.,  made by Sanritsu */

	/* Rare games */
	DRIVER( xtheball )	/* (c) 1991 Rare */
	DRIVER( btoads )	/* (c) 1994 Rare */
	DRIVER( kinst )		/* (c) 1994 Rare */
	DRIVER( kinst14 )	/* (c) 1994 Rare */
	DRIVER( kinst13 )	/* (c) 1994 Rare */
	DRIVER( kinstp )	/* (c) 1994 Rare */
	DRIVER( kinst2 )	/* (c) 1995 Rare */
	DRIVER( kinst2k4 )	/* (c) 1995 Rare */
	DRIVER( kinst213 )	/* (c) 1995 Rare */
	DRIVER( kinst2k3 )	/* (c) 1995 Rare */
	DRIVER( kinst211 )	/* (c) 1995 Rare */
	DRIVER( kinst210 )	/* (c) 1995 Rare */

	/* Nihon System games */
	DRIVER( gigas )		/* (c) 1986 Sega */
	DRIVER( gigasb )	/* (c) 1986 bootleg */
	DRIVER( gigasm2b )	/* (c) 1986 bootleg */
	DRIVER( oigas )		/* (c) 1986 bootleg */
	DRIVER( pbillrd )	/* (c) 1987 Nihon System */
	DRIVER( pbillrds )	/* (c) 1988 Nihon System (+ optional Sega) */
	DRIVER( freekick )	/* (c) 1988 Nihon System (+ optional Sega) */
	DRIVER( freekickb )	/* (c) 1987 bootleg */
	DRIVER( freekickb2 )	/* (c) 1987 bootleg */
	DRIVER( countrun )	/* (c) 1988 Nihon System (+ optional Sega) */
	DRIVER( countrunb )	/* (c) 1988 bootleg */
	DRIVER( countrunb2 )	/* (c) 1988 bootleg */

	/* Alba games */
	DRIVER( rmhaihai )	/* (c) 1985 Alba */
	DRIVER( rmhaihib )	/* (c) 1985 Alba */
	DRIVER( rmhaijin )	/* (c) 1986 Alba */
	DRIVER( rmhaisei )	/* (c) 1986 Visco */
	DRIVER( themj )		/* (c) 1987 Visco */
	DRIVER( hanaroku )	/* (c) 1988 Alba */
	DRIVER( yumefuda )	/* (c) 198? Alba */

	/* Home Data games */
	DRIVER( hourouki )	/* (c) 1987 Home Data */
	DRIVER( mhgaiden )	/* (c) 1987 Home Data */
	DRIVER( mirderby )  /* (c) 1988 Home Data? */
	DRIVER( mjhokite )	/* (c) 1988 Home Data */
	DRIVER( mjclinic )	/* (c) 1988 Home Data */
	DRIVER( mrokumei )	/* (c) 1988 Home Data */
	DRIVER( reikaids )	/* (c) 1988 Home Data */
	DRIVER( mjkojink )	/* (c) 1989 Home Data */
	DRIVER( vitaminc )	/* (c) 1989 Home Data */
	DRIVER( mjyougo )	/* (c) 1989 Home Data */
	DRIVER( lemnangl )	/* (c) 1990 Home Data */
	DRIVER( mjkinjas )	/* (c) 1991 Home Data */
	DRIVER( battlcry )	/* (c) 1991 Home Data */
	DRIVER( jogakuen )	/* (c) 1992 Windom corporation */
	DRIVER( mjikaga )	/* (c) 1991 Mitchell */
	DRIVER( mjprivat )	/* (c) 1991 Matoba */

	/* Art & Magic games */
	DRIVER( ultennis )	/* (c) 1993 */
	DRIVER( ultennisj )	/* (c) 1993 */
	DRIVER( cheesech )	/* (c) 1994 */
	DRIVER( stonebal )	/* (c) 1994 */
	DRIVER( stonebal2 )	/* (c) 1994 */

	/* Dynamo games */
	DRIVER( skeetsht )	/* (c) 1991 */
	DRIVER( toursol )	/* (c) 1995 Dynamo */
	DRIVER( toursol1 )	/* (c) 1995 Dynamo */

	/* Taiyo games */
	DRIVER( dynamski )	/* (c) 1984 Taiyo */
	DRIVER( chinhero )	/* (c) 1984 Taiyo */
	DRIVER( chinhero2 )	/* (c) 1984 Taiyo */
	DRIVER( chinherot )	/* (c) 1984 Taiyo */
	DRIVER( shangkid )	/* (c) 1985 Taiyo + Data East license */
	DRIVER( hiryuken )	/* (c) 1985 Taito */

	/* Tatsumi Games */
	DRIVER( kingdrby )  /* (c) 1981 Tatsumi */
	DRIVER( tx1 )		/* (c) 1983 Tatsumi */
	DRIVER( tx1a )		/* (c) 1983 Tatsumi */
	/* TX-1 V8 */		/* (c) 1984 Tatsumi */
	DRIVER( buggyboy )	/* (c) 1985 Tatsumi */
	DRIVER( buggyboyjr )/* (c) 1986 Tatsumi */
	DRIVER( lockon )	/* (c) 1986 Tatsumi */
	DRIVER( lockonc )	/* (c) 1986 Tatsumi */
	/* Gray-Out */		/* (c) 1987 Tatsumi */
	DRIVER( apache3 )	/* (c) 1988 Tatsumi */
	DRIVER( apache3a )	/* (c) 1988 Tatsumi + Kana Corporation license */
	DRIVER( roundup5 )	/* (c) 1989 Tatsumi */
	/* Round Up 5 cockpit */
	DRIVER( cyclwarr )	/* (c) 1991 Tatsumi */
	DRIVER( bigfight )	/* (c) 1992 Tatsumi */
	/* Life */          /* (c) 1997 Tatsumi */

	/* Electronic Devices */
	DRIVER( mugsmash )	/* (c) Electronic Devices (Italy) / 3D Games (England) */
	DRIVER( stlforce )	/* (c) 1994 Electronic Devices (Italy) / Ecogames S.L. (Spain) */
	DRIVER( fantland )	/* (c) 198? Electronic Devices Italy */
	DRIVER( fantlanda )	/* (c) 198? Electronic Devices Italy */
	DRIVER( wheelrun )	/* (c) 1988? International Games */
	DRIVER( galaxygn )	/* (c) 1989 Electronic Devices Italy */
	DRIVER( borntofi )	/* (c) International Games? */
	DRIVER( diverboy )	/* (c) 1992 Electronic Devices */
	DRIVER( twinbrat )	/* (c) 1995 */
	DRIVER( twinbrata )	/* (c) 1995 */
	DRIVER( ppmast93 )	/* (c) 1993 Electronic Devices S.R.L. */
	DRIVER( twins )		/* (c) 1994 */
	DRIVER( twinsa )	/* (c) 1994 */
	DRIVER( mwarr )
	DRIVER( pzletime )

	/* Arcadia System */
	DRIVER( ar_bios )
	DRIVER( ar_airh )	/* (c) 1988 */
	DRIVER( ar_airh2 )	/* (c) 1988 */
	DRIVER( ar_bowl )	/* (c) 1988 */
	DRIVER( ar_dart )	/* (c) 1987 */
	DRIVER( ar_dart2 )	/* (c) 1987 */
	DRIVER( ar_fast )	/* (c) 1988 */
	DRIVER( ar_ldrb )	/* (c) 1988 */
	DRIVER( ar_ldrba )	/* (c) 1988 */
	DRIVER( ar_ninj )	/* (c) 1987 */
	DRIVER( ar_ninj2 )	/* (c) 1987 */
	DRIVER( ar_rdwr )	/* (c) 1988 */
	DRIVER( ar_sdwr )	/* (c) 1988 */
	DRIVER( ar_sdwr2 )	/* (c) 1988 */
	DRIVER( ar_socc )	/* (c) 1989 */
	DRIVER( ar_spot )	/* (c) 1990 */
	DRIVER( ar_sprg )	/* (c) 1987 */
	DRIVER( ar_xeon )	/* (c) 1988 */

	/* Sente Super System */
	DRIVER( mquake )	/* (c) 1987 */

	/* Other Amiga-based games */
	DRIVER( upscope )	/* (c) 1986 Grand products */

	DRIVER( cd32 )		/* (c) 1993 */
	DRIVER( cndypuzl )	/* (c) 1995 */
	DRIVER( haremchl )	/* (c) 1995 */
	DRIVER( lsrquiz )	/* (c) 1995 */
	DRIVER( lsrquiz2 )	/* (c) 1995 */
	DRIVER( mgprem11 )	/* (c) 1995 */
	DRIVER( lasstixx )	/* (c) 1995 */
	DRIVER( mgnumber )	/* (c) 1995 */

	/* American Laser Games */
	DRIVER( alg_bios )
	DRIVER( maddoga )	/* (c) 1990 */
	DRIVER( wsjr )		/* (c) 1991 */
	DRIVER( wsjr15 )	/* (c) 1991 */
	DRIVER( maddog )	/* (c) 1990 */
	DRIVER( maddog2 )	/* (c) 1992 */
	DRIVER( maddog22 )	/* (c) 1992 */
	DRIVER( maddog21 )	/* (c) 1992 */
	DRIVER( spacepir )	/* (c) 1992 */
	DRIVER( gallgall )	/* (c) 1992 */
	DRIVER( crimepat )	/* (c) 1993 */
	DRIVER( crimep211 )	/* (c) 1993 */
	DRIVER( crimep2 )	/* (c) 1993 */
	DRIVER( lastbh )	/* (c) 1994 */
	DRIVER( fastdraw )	/* (c) 1995 */
	DRIVER( aplatoon )	/* (c) 199? Nova? */
	DRIVER( zortonbr )	/* (c) 1993 Web Picmatic */

	/* Crystal System */
	DRIVER( crysbios )
	DRIVER( crysking )	/* 2001 Brezzasoft. Crystal of the kings */
	DRIVER( evosocc )	/* 2001 Evoga. Evolution Soccer */
	DRIVER( topbladv )	/* 2002 Sonokong. Top Blade V */
	DRIVER( officeye )

	DRIVER( ddz )		/* (c) 200? */

	DRIVER( psattack )	/* 2004 Uniana */

	/* Bell Fruit Cobra Hardware */
	DRIVER( inquiztr )	/* 1989 BFM */
	DRIVER( escounts )	/* 1990 BFM */
	DRIVER( trebltop )	/* 1991 BFM */
	DRIVER( beeline )	/* 1991 BFM */
	DRIVER( quizvadr )	/* 1991 BFM */
	DRIVER( qos )		/* 1992 BFM */
	DRIVER( qosa )		/* 1992 BFM */
	DRIVER( qosb )		/* 1992 BFM */

	/* Scorpion 2+Video Board Games */
	DRIVER( qntoondo )	/* 1993 */
	DRIVER( quintoon )	/* 1993 */
	DRIVER( quintond )	/* 1993 */
	DRIVER( quintono )	/* 1993 */
	DRIVER( qntoond )	/* 1993 */
	DRIVER( pokio )		/* 1994 */
	DRIVER( slotsnl )	/* 1995 */
	DRIVER( paradice )	/* 1995 */
	DRIVER( pyramid )	/* 1996 */
	DRIVER( sltblgtk )	/* 1996 */
	DRIVER( sltblgpo )	/* 1996 */
	DRIVER( sltblgp1 )	/* 1996 */
	DRIVER( gldncrwn )	/* 1997 */

	/* MPU4 Diagnostic ROMs */
	DRIVER( mpu4utst )
	DRIVER( mpu4tst2 )
	DRIVER( mpu4met0 )

	/* MPU4 Non-Video SWPs */
	DRIVER( connect4 )	/* Dolbeck Systems */

	/* MPU4+Video Board Games */
	DRIVER( bctvidbs )
	DRIVER( dealem )	/* 1987 */
	DRIVER( adders )	/* 1989 */
	DRIVER( timemchn )	/* 1990 */
	DRIVER( turnover )	/* 199? */
	DRIVER( skiltrek )	/* 1990 */
	DRIVER( crmaze )	/* 1994 */
	DRIVER( crmazed )	/* 1993 */
	DRIVER( crmazea )	/* 1993 */
	DRIVER( crmaze2 )	/* 1993 */
	DRIVER( crmaze2d )	/* 1993 */
	DRIVER( crmaze2a )	/* 1993 */
	DRIVER( crmaze3 )	/* 1993 */
	DRIVER( crmaze3d )	/* 1993 */
	DRIVER( crmaze3a )	/* 1993 */

	DRIVER( mating )	/* 199? */
	DRIVER( matingd )	/* 199? */

	DRIVER( strikeit )	/* 199? */
	DRIVER( strikeitd )	/* 199? */
	DRIVER( strikeit2 )	/* 199? */
	DRIVER( strikeit2d )/* 199? */

	DRIVER( quidgrid )	/* 199? */
	DRIVER( quidgridd )	/* 199? */
	DRIVER( quidgrid2 )	/* 199? */
	DRIVER( quidgrid2d )/* 199? */

	DRIVER( eyesdown )	/* 199? */
	DRIVER( eyesdownd )	/* 199? */

	DRIVER( blox )		/* 199? */
	DRIVER( bloxd )		/* 199? */
	DRIVER( prizeinv )	/* 199? */
	DRIVER( vgpoker )	/* 199? */

	/* ADP games */
	DRIVER( backgamn )	/* (c) 1990 */
	DRIVER( quickjac )	/* (c) 1993 */
	DRIVER( skattv )	/* (c) 1994 */
	DRIVER( skattva )	/* (c) 1995 */
	DRIVER( fashiong )	/* (c) 1997 */
	DRIVER( fstation )	/* (c) 2000 */

	/* Other games on adp hardware */
	DRIVER( funlddlx )	/* (c) 1999 */

	/* JPM Give us a Break hardware */
	DRIVER( guab )		/* (c) 1986 */
	DRIVER( guab3a )	/* (c) 1986 */
	DRIVER( guab4 )		/* (c) 1986 */
	DRIVER( guab6 )		/* (c) 1986 */
	DRIVER( guab6a )	/* (c) 1986 */
	DRIVER( guab7 )		/* (c) 1986 */
	DRIVER( guab21 )	/* (c) 1986 */
	DRIVER( guab43 )	/* (c) 1986 */
	DRIVER( crisscrs )	/* (c) 1986 */
	DRIVER( tenup )		/* (c) 1988 */
	DRIVER( tenup3 )	/* (c) 1988 */

	/* JPM System 5 + video expansion 2 Games */
	DRIVER( monopoly )	/* (c) 1994 */
	DRIVER( monoplcl )	/* (c) 1995 */
	DRIVER( monopldx )	/* (c) 1995 */

	/* JPM Impact Games */
	DRIVER( cluedo )	/* (c) 1995 */
	DRIVER( cluedo2c )	/* (c) 1995 */
	DRIVER( cluedo2 )	/* (c) 1995 */
	DRIVER( trivialp )	/* (c) 1996 */
	DRIVER( scrabble )	/* (c) 1997 */
	DRIVER( hngmnjpm )	/* (c) 1998 */
	DRIVER( coronatn )	/* (c) 1999 */

	/* Other Games */
	DRIVER( vega )		/* (c) 19?? Olympia? */
	DRIVER( astinvad )	/* (c) 1980 Stern */
	DRIVER( kosmokil )	/* bootleg */
	DRIVER( kamikaze )	/* Leijac Corporation */
	DRIVER( spcking2 )	/* (c) 1979 Konami */
	DRIVER( spaceint )	/* [1980] Shoei */
	DRIVER( spaceintj )	/* [1980] Shoei */
	DRIVER( spacefb )	/* (c) [1980?] Nintendo */
	DRIVER( spacefbg )	/* 834-0031 (c) 1980 Gremlin */
	DRIVER( spacefbb )	/* bootleg */
	DRIVER( spacebrd )	/* bootleg */
	DRIVER( spacedem )	/* (c) 1980 Fortrek + made by Nintendo */
	DRIVER( omegrace )	/* (c) 1981 Midway */
	DRIVER( deltrace )	/* bootleg */
	DRIVER( monzagp )	/* (c) 1981 Olympia */
	DRIVER( dday )		/* (c) 1982 Olympia */
	DRIVER( ddayc )		/* (c) 1982 Olympia + Centuri license */
	DRIVER( stactics )	/* [1981 Sega] */
	DRIVER( exterm )	/* (c) 1989 Premier Technology - a Gottlieb game */
	DRIVER( flower )	/* (c) 1986 Komax license */
	DRIVER( flowerj )	/* 834-5998 (c) 1986 Sega / Alpha license? */
	DRIVER( kingofb )	/* (c) 1985 Wood Place Inc. */
	DRIVER( ringking )	/* (c) 1985 Data East USA */
	DRIVER( ringking2 )	/* (c) 1985 Data East USA */
	DRIVER( ringking3 )	/* (c) 1985 Data East USA */
	DRIVER( ringkingw )	/* (c) 1985 Wood Place Inc. */
	DRIVER( homo )		/* bootleg */
	DRIVER( dlair )		/* (c) 1983 Cinematronics */
	DRIVER( dlairf )	/* (c) 1983 Cinematronics */
	DRIVER( dlaire )	/* (c) 1983 Cinematronics */
	DRIVER( dlaird )	/* (c) 1983 Cinematronics */
	DRIVER( dlairc )	/* (c) 1983 Cinematronics */
	DRIVER( dlairb )	/* (c) 1983 Cinematronics */
	DRIVER( dlaira )	/* (c) 1983 Cinematronics */
	DRIVER( dleuro )	/* (c) 1983 Atari */
	DRIVER( dlital )	/* (c) 1983 Sidam */
	DRIVER( spaceace )	/* (c) 1983 Cinematronics */
	DRIVER( spaceaa2 )	/* (c) 1983 Cinematronics */
	DRIVER( spaceaa )	/* (c) 1983 Cinematronics */
	DRIVER( saeuro )	/* (c) 1983 Atari */
	DRIVER( aztarac )	/* (c) 1983 Centuri (vector game) */
	DRIVER( mole )		/* (c) 1982 Yachiyo Electronics, Ltd. */
	DRIVER( thehand )	/* (c) 1981 T.I.C. */
	DRIVER( mrjong )	/* (c) 1983 Kiwako */
	DRIVER( crazyblk )	/* (c) 1983 Kiwako + ECI license */
	DRIVER( blkbustr )	/* (c) 1983 Kiwako + ECI license */
	DRIVER( polyplay )
	DRIVER( amspdwy )	/* no copyright notice, but (c) 1987 Enerdyne Technologies, Inc. */
	DRIVER( amspdwya )	/* no copyright notice, but (c) 1987 Enerdyne Technologies, Inc. */
	DRIVER( othldrby )	/* (c) 1995 Sunwise */
	DRIVER( quizpun2 )	/* (c) 1989 Space Computer System of Korea */
	DRIVER( mosaic )	/* (c) 1990 Space */
	DRIVER( mosaica )	/* (c) 1990 Space + Fuuki license */
	DRIVER( gfire2 )	/* (c) 1992 Topis Corp */
	DRIVER( sprcros2 )	/* (c) 1986 GM Shoji */
	DRIVER( sprcros2a )	/* (c) 1986 GM Shoji */
	DRIVER( d9final )	/* (c) 1992 Excellent System */
	DRIVER( gcpinbal )	/* (c) 1994 Excellent System */
	DRIVER( vmetal )	/* (c) 1995 Excellent System? */
	DRIVER( vmetaln )	/* (c) 1995 Excellent System? */
	DRIVER( aquarium )	/* (c) 1996 Excellent System */
	DRIVER( policetr )	/* (c) 1996 P&P Marketing */
	DRIVER( policetr11 )/* (c) 1996 P&P Marketing */
	DRIVER( policetr10 )/* (c) 1996 P&P Marketing */
	DRIVER( policetr13a )/* (c) 1996 P&P Marketing */
	DRIVER( policetr13b )/* (c) 1996 P&P Marketing */
	DRIVER( sshooter )	/* (c) 1998 P&P Marketing */
	DRIVER( sshooter12 )/* (c) 1998 P&P Marketing */
	DRIVER( sshooter11 )/* (c) 1998 P&P Marketing */
	DRIVER( pass )		/* (c) 1992 Oksan */
	DRIVER( news )		/* "Virus"??? ((c) 1993 Poby in ROM VIRUS.4) */
	DRIVER( newsa )		/* "Jeansole / Poby */
	DRIVER( taxidrvr )	/* [1984 Graphic Techno] */
	DRIVER( xyonix )	/* [1989 Philko] */
	DRIVER( gt507uk )	/* (c) 1986 Grayhound Electronics */
	DRIVER( gt5 )		/* (c) 1984 Grayhound Electronics */
	DRIVER( gtsers8 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gtsers9 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gtsers10 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gtsers11 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gt103a1 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gt103a2 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gt103a3 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gt103aa )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gt103asx )	/* (c) 1984 Greyhound Electronics */
	DRIVER( reelfun )	/* (c) 1986 Grayhound Electronics */
	DRIVER( reelfun1 )	/* (c) 1986 Grayhound Electronics */
	DRIVER( findout )	/* (c) 1987 [Elettronolo] */
	DRIVER( quizvid )   /* (c) 1986 Greyhound Electronics */
	DRIVER( quiz )		/* (c) 1986 Italian bootleg */
	DRIVER( quiz211 )	/* (c) 1986 Italian bootleg */
	DRIVER( jokpoker )	/* (c) 1982 Greyhound Electronics */
	DRIVER( jokpokera )	/* (c) 1982 Greyhound Electronics */
	DRIVER( jokpokerb )
	DRIVER( jokpokerc )
	DRIVER( superbwl )	/* (c) 1982 Greyhound Electronics */
	DRIVER( amuse )		/* (c) 1982 Greyhound Electronics */
	DRIVER( amuse1 )	/* (c) 1982 Greyhound Electronics */
	DRIVER( gs4002 )	/* (c) 1982 G.E.I. */
	DRIVER( gs4002a )	/* (c) 1982 G.E.I. */
	DRIVER( gepoker )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gepoker1 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gepoker2 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gtsers1 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gtsers2 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gtsers3 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gtsers4 )	/* (c) 1984 Greyhound Electronics */
	DRIVER( gtsers5 )	/* (c) 1985 Greyhound Electronics */
	DRIVER( gtsers7 )	/* (c) 1985 Greyhound Electronics */
	DRIVER( gtsersa )	/* (c) 1985 Greyhound Electronics */
	DRIVER( gtsersb )	/* (c) 1985 Greyhound Electronics */
	DRIVER( sextriv1 )	/* (c) 1985 Kinky Kit and Game Co. */
	DRIVER( sextriv2 )	/* (c) 1985 Kinky Kit and Game Co. */
	DRIVER( geimulti )	/* (c) 1992 Grayhound Electronics */
	DRIVER( sprtauth )	/* (c) 1982 Classic Games */
	DRIVER( suprpokr )  /* (c) 1986 Grayhound Electronics */
	DRIVER( suprpkr1 )  /* (c) 1986 Grayhound Electronics */
	DRIVER( dribling )	/* (c) 1983 Model Racing */
	DRIVER( driblingo )	/* (c) 1983 Olympia */
	DRIVER( ace )		/* [1976 Allied Leisure] */
	DRIVER( clayshoo )	/* [1979 Allied Leisure] */
	DRIVER( pirates )	/* (c) 1994 NIX */
	DRIVER( genix )		/* (c) 199? NIX */
	DRIVER( fitfight )	/* bootleg of Art of Fighting */
	DRIVER( histryma )	/* bootleg of Fighter's History */
	DRIVER( bbprot )	/* unfinished prototype */
	DRIVER( beaminv )	/* (c) 1979 Tekunon Kougyou (Arcade TV Game List - P.95, Left, 9 from top) */
	DRIVER( beaminva )	/* (c) 1979 Tekunon Kougyou (Arcade TV Game List - P.95, Left, 9 from top) */
	DRIVER( mcatadv )	/* (c) 1993 Wintechno */
	DRIVER( mcatadvj )	/* (c) 1993 Wintechno */
	DRIVER( catt )		/* (c) 1993 Wintechno */
	DRIVER( nost )		/* (c) 1993 Face */
	DRIVER( nostj )		/* (c) 1993 Face */
	DRIVER( nostk )		/* (c) 1993 Face */
	DRIVER( 4enraya )	/* (c) 1990 IDSA */
	DRIVER( oneshot )	/* no copyright notice */
	DRIVER( maddonna )	/* (c) 1995 Tuning */
	DRIVER( maddonnb )	/* (c) 1995 Tuning */
	DRIVER( tugboat )	/* (c) 1982 ETM */
	DRIVER( noahsark )	/* (c) 1983 Enter-Tech */
	DRIVER( berenstn )	/* (c) 1984 Enter-Tech */
	DRIVER( gotcha )	/* (c) 1997 Dongsung + "presented by Para" */
	DRIVER( ppchamp )	/* (c) 1997 Dongsung + "presented by Para" */
	DRIVER( amerdart )	/* (c) 1989 Ameri Corporation */
	DRIVER( amerdart2 )	/* (c) 1989 Ameri Corporation */
	DRIVER( amerdart3 )	/* (c) 1989 Ameri Corporation */
	DRIVER( coolpool )	/* (c) 1992 Catalina Games */
	DRIVER( 9ballsht )	/* (c) 1993 E-Scape EnterMedia + "marketed by Bundra Games" */
	DRIVER( 9ballsht2 )	/* (c) 1993 E-Scape EnterMedia + "marketed by Bundra Games" */
	DRIVER( 9ballsht3 )	/* (c) 1993 E-Scape EnterMedia + "marketed by Bundra Games" */
	DRIVER( 9ballshtc )	/* (c) 1993 E-Scape EnterMedia + "marketed by Bundra Games" */
	DRIVER( gumbo )		/* (c) 1994 Min Corp. */
	DRIVER( mspuzzleg )	/* (c) 1994 Min Corp. */
	DRIVER( mspuzzle )	/* (c) 1994 Min Corp. */
	DRIVER( mspuzzlen )	/* (c) 1994 Min Corp. */
	DRIVER( msbingo )	/* (C) 1994 Min Corp. */
	DRIVER( dblpoint )	/* (c) 1995 Min Corp. */
	DRIVER( dblpointd )	/* (c) 1995 Dong Bang Electron */
	DRIVER( statusbj )	/* (c) 1981 Status Games */
	DRIVER( tripdraw )	/* (c) 1981 Status Games */
	DRIVER( funcsino )	/* (c) 1981 Status Games */
	DRIVER( hangman )	/* (c) 1984 Status Games */
	DRIVER( trivquiz )	/* (c) 1984 Status Games */
	DRIVER( statriv2 )	/* (c) 1984 Status Games */
	DRIVER( statriv2v )	/* (c) 1984 Status Games */
	DRIVER( statriv4 )	/* (c) 1985 Status Games */
	DRIVER( sextriv )	/* (c) 1985 Status Games */
	DRIVER( quaquiz2 )	/* (c) 1985 Status Games */
	DRIVER( supertr2 )	/* (c) 1986 Status Games */
	DRIVER( supertr3 )	/* (c) 1986 Status Games */
	DRIVER( cstripxi )	/* (c) 1981 Status Games */
	DRIVER( tickee )	/* (c) 1994 Raster Elite */
	DRIVER( ghoshunt )	/* (c) 1995? Hanaho Games */
	DRIVER( tutstomb )	/* (c) 199?? Island Design */
	DRIVER( mouseatk )	/* (c) 1996 ICE */
	DRIVER( rapidfir )	/* (c) 1998 Hanaho Games */
	DRIVER( rapidfire )	/* (c) 1998 Hanaho Games */
	DRIVER( crgolf )	/* (c) 1984 Nasco Japan */
	DRIVER( crgolfa )	/* (c) 1984 Nasco Japan */
	DRIVER( crgolfb )	/* (c) 1984 Nasco Japan */
	DRIVER( crgolfc )	/* (c) 1984 Nasco Japan */
	DRIVER( crgolfbt )	/* bootleg */
	DRIVER( crgolfhi )	/* (c) 1984 Nasco Japan */
	DRIVER( truco )		/* (c) 198? Playtronic SRL */
	DRIVER( thedeep )	/* (c) 1987 Wood Place */
	DRIVER( rundeep )	/* (c) 1988 Cream (bootleg?) */
	DRIVER( wallc )		/* (c) 1984 Midcoin */
	DRIVER( wallca )	/* (c) 1984 Midcoin */
	DRIVER( wink )		/* (c) 1985 Midcoin */
	DRIVER( winka )		/* (c) 1985 Midcoin */
	DRIVER( skyarmy )	/* (c) 1982 Shoei */
	DRIVER( lethalj )	/* (c) 1996 The Game Room */
	DRIVER( eggventr )	/* (c) 1997 The Game Room */
	DRIVER( eggventr8 )	/* (c) 1997 The Game Room */
	DRIVER( eggventr7 )	/* (c) 1997 The Game Room */
	DRIVER( eggventra )	/* (c) 1997 The Game Room (Allied Leisure license) */
	DRIVER( eggventrd )	/* (c) 1997 The Game Room */
	DRIVER( calchase )	/* (c) 1999 The Game Room */
	DRIVER( ripribit )	/* (c) 1997 LAI Games */
	DRIVER( cfarm )		/* (c) 1999 LAI Games */
	DRIVER( cclownz )	/* (c) 1999 LAI Games */
	DRIVER( rotaryf )
	DRIVER( portrait )	/* (c) 1983 Olympia */
	DRIVER( portraita )	/* (c) 1983 Olympia */
	DRIVER( enigma2 )	/* (c) 1981 Game Plan (Zilec Electronics license) */
	DRIVER( enigma2a )	/* (c) 1984 Zilec Electronics (bootleg?) */
	DRIVER( enigma2b )	/* (c) 1981 Zilec Electronics */
	DRIVER( ltcasino )	/* (c) 1982 Digital Controls Inc */
	DRIVER( mv4in1 )	/* (c) 1983 Entertainment Enterprises */
	DRIVER( ltcasinn )	/* (c) 1984 Digital Controls Inc */
	DRIVER( coolmini )	/* (c) 1999 SemiCom */
	DRIVER( luplup )	/* (c) 1999 Omega System */
	DRIVER( luplup29 )	/* (c) 1999 Omega System */
	DRIVER( puzlbang )	/* (c) 1999 Omega System */
	DRIVER( suplup )	/* (c) 1999 Omega System */
	DRIVER( jmpbreak )	/* (c) 1999 F2 System */
	DRIVER( vamphalf )	/* (c) 1999 DanBi & F2 System */
	DRIVER( vamphalfk )	/* (c) 1999 DanBi & F2 System */
	DRIVER( dquizgo2 )	/* (c) 2000 Semicom */
	DRIVER( misncrft )	/* (c) 2000 Sun */
	DRIVER( mrdig )		/* (c) 2000 Sun */
	DRIVER( hidnctch )	/* (c) 1998 Eolith */
	DRIVER( ironfort )	/* (c) 1998 Eolith */
	DRIVER( ironfortj )	/* (c) 1998 Eolith */
	DRIVER( puzzlekg )	/* (c) 1998 Eolith */
	DRIVER( hidctch2 )	/* (c) 1999 Eolith */
	DRIVER( nhidctch )	/* (c) 1999 Eolith */
	DRIVER( landbrk )	/* (c) 1999 Eolith */
	DRIVER( landbrka )	/* (c) 1999 Eolith */
	DRIVER( raccoon )	/* (c) 1999 Eolith */
	DRIVER( fort2b )	/* (c) 2001 Eolith */
	DRIVER( fort2ba )	/* (c) 2001 Eolith */
	DRIVER( klondkp )	/* (c) 1999 Eolith */
	DRIVER( hidctch3 )	/* (c) 2000 Eolith */
	DRIVER( crazywar )	/* (c) 2002 Eolith */
	DRIVER( bballoon )	/* (c) 2003 Eolith */
	DRIVER( hapytour )	/* (c) 2005 GAV Company (on Eolith's Ghost hardware) */
	DRIVER( xfiles )	/* (c) 1999 dgPIX Entertainment Inc */
	DRIVER( kdynastg )	/* (c) 1999 EZ Graphics */
	DRIVER( fmaniac3 )	/* (c) 2002 Saero Entertainment */
	DRIVER( promutrv )	/* (c) 1985 Enerdyne Technologies Inc */
	DRIVER( promutrva )	/* (c) 1985 Enerdyne Technologies Inc */
	DRIVER( promutrvb )	/* (c) 1985 Enerdyne Technologies Inc */
	DRIVER( promutrvc )	/* (c) 1985 Enerdyne Technologies Inc */
	DRIVER( strvmstr )	/* (c) 1986 Enerdyne Technologies Inc */
	DRIVER( dorachan )	/* (c) 1980 Craul Denshi */
	DRIVER( ladyfrog )	/* (c) 1990 Mondial Games */
	DRIVER( toucheme )
	DRIVER( rabbit )	/* (c) 1997 Electronic Arts */
	DRIVER( tmmjprd )	/* (c) 1997 Media / Sonnet */
	DRIVER( tmpdoki )	/* (c) 1998 Media Syouji */
	DRIVER( malzak )	/* (c) 1980? Kitronix */
	DRIVER( malzak2 )	/* (c) 1980? Kitronix */
	DRIVER( supertnk )	/* (c) 1981 VIDEO GAMES GmbH, W.-GERMANY */
	DRIVER( crospang )	/* (c) 1998 F2 System */
	DRIVER( heuksun )	/* (c) 1998 Oksan / F2 System */
	DRIVER( bestri )	/* (c) F2 System */
	DRIVER( funybubl )	/* (c) 1999 In Chang Electronic Co */
	DRIVER( funybublc )	/* (c) 1999 Comad Industries */
	DRIVER( dcheese )	/* (c) 1993 HAR */
	DRIVER( lottof2 )	/* (c) 1993 HAR */
	DRIVER( fredmem )	/* (c) 1994 Coastal Amusements */
	DRIVER( fredmemus )	/* (c) 1994 Coastal Amusements */
	DRIVER( fredmemuk )	/* (c) 1994 Coastal Amusements */
	DRIVER( fredmemj )	/* (c) 1994 Coastal Amusements */
	DRIVER( fredmemc )	/* (c) 1994 Coastal Amusements */
	DRIVER( fredmesp )	/* (c) 1994 Coastal Amusements */
	DRIVER( cecmatch )	/* (c) 1993 Coastal Amusements */
	DRIVER( f15se )		/* (c) 1991 Microprose Games Inc. */
	DRIVER( f15se21 )	/* (c) 1991 Microprose Games Inc. */
	DRIVER( botss )		/* (c) 1992 Microprose Games Inc. */
	DRIVER( botssa )	/* (c) 1992 Microprose Games Inc. */
	DRIVER( tankbatl )	/* (c) 1992 Microprose Games Inc. */
	DRIVER( dynadice )	/* ? */
	DRIVER( atamanot )	/* (c) 1983 Yachiyo Denki / Uni Enterprize */
	DRIVER( ssingles )	/* Yachiyo? */
	DRIVER( tcl )		/* (c) 1995 Uniwang */
	DRIVER( othello )	/* (c) 1984 Success */
	DRIVER( sothello )	/* (c) 1986 Success / Fujiwara */
	DRIVER( quake )		/* (c) 19?? Lazer-Tron / iD Software  */

	/* Multi Amenity Cassette System */
	DRIVER( macsbios )
	DRIVER( mac2bios )
	DRIVER( kisekaem )	/* (c) 1995 I'Max */
	DRIVER( kisekaeh )	/* (c) 1995 I'Max */
	DRIVER( cultname )	/* (c) 1995 I'Max */
	DRIVER( yuka )		/* (c) 1999 Yubis */
	DRIVER( yujan )		/* (C) 1999 Yubis */

	DRIVER( onetwo )	/* (c) 1997 Barko */
	DRIVER( onetwoe )	/* (c) 1997 Barko */
	DRIVER( 1945kiii )	/* (c) 2000 Oriental */
	DRIVER( witch )		/* (c) 1992 Sega / Vic Tokai / Excellent Systems */
	DRIVER( pbchmp95 )	/* (c) 1995 Veltmeijer Automaten */
	DRIVER( cardline )	/* (c) 199? Veltmeijer */
	DRIVER( bmcbowl )	/* (c) 1994 BMC */
	DRIVER( koftball )	/* (c) 1995 BMC */
	DRIVER( mpoker )	/* (c) 1983 Merit */
	DRIVER( pitboss )	/* (c) 1983 Merit */
	DRIVER( pitbossa )	/* (c) 1983 Merit */
	DRIVER( pitbossb )	/* (c) 1983 Merit */
	DRIVER( pitbossc )	/* (c) 1983 Merit */
	DRIVER( casino5 )	/* (c) 1983 Merit */
	DRIVER( trvwzh )	/* (c) 1985 Merit */
	DRIVER( trvwzha )	/* (c) 1985 Merit */
	DRIVER( trvwzhb )	/* (c) 1985 Merit */
	DRIVER( trvwzv )	/* (c) 1985 Merit */
	DRIVER( trvwzva )	/* (c) 1985 Merit */
	DRIVER( trvwz2 )	/* (c) 1985 Merit */
	DRIVER( trvwz2a )	/* (c) 1985 Merit */
	DRIVER( trvwz3h )	/* (c) 1985 Merit */
	DRIVER( trvwz3ha )	/* (c) 1985 Merit */
	DRIVER( trvwz3v )	/* (c) 1985 Merit */
	DRIVER( trvwz4 )	/* (c) 1985 Merit */
	DRIVER( trvwz4a )	/* (c) 1985 Merit */
	DRIVER( tictac )	/* (c) 1985 Merit */
	DRIVER( phrcraze )	/* (c) 1986 Merit */
	DRIVER( phrcrazea )	/* (c) 1986 Merit */
	DRIVER( phrcrazeb )	/* (c) 1986 Merit */
	DRIVER( phrcrazec )	/* (c) 1986 Merit */
	DRIVER( phrcrazev )	/* (c) 1986 Merit */
	DRIVER( bigappg )	/* (c) 1986 Merit */
	DRIVER( dodge )		/* (c) 1986 Merit */
	DRIVER( couple )	/* (c) 1988 Merit */
	DRIVER( couplep )	/* (c) 1988 Merit */
	DRIVER( couplei )	/* (c) 1988 Merit */
	DRIVER( pitboss2 )	/* (c) 1988 Merit */
	DRIVER( spitboss )	/* (c) 1988 Merit */
	DRIVER( pitbosss )	/* (c) 1990 Merit */
	DRIVER( pitbossm )	/* (c) 1994 Merit */
	DRIVER( pitbossma )	/* (c) 1994 Merit */
	DRIVER( pbst30 )	/* (c) 1994 Merit */
	DRIVER( pbst30b )	/* (c) 1993 Merit */
	DRIVER( megat3 )	/* (c) 1996 Merit */
	DRIVER( megat3a )	/* (c) 1996 Merit */
	DRIVER( megat3ca )	/* (c) 1996 Merit */
	DRIVER( megat3nj )	/* (c) 1995 Merit */
	DRIVER( megat3te )	/* (c) 1996 Merit */
	DRIVER( megat4 )	/* (c) 1996 Merit */
	DRIVER( megat4a )	/* (c) 1996 Merit */
	DRIVER( megat4sn )	/* (c) 1996 Merit */
	DRIVER( megat4te )	/* (c) 1996 Merit */
	DRIVER( megat4tea )	/* (c) 1996 Merit */
	DRIVER( megat4st )	/* (c) 1996 Merit */
	DRIVER( megat4stg )	/* (c) 1996 Merit */
	DRIVER( megat5 )	/* (c) 1997 Merit */
	DRIVER( megat5a )	/* (c) 1997 Merit */
	DRIVER( megat5nj )	/* (c) 1997 Merit */
	DRIVER( megat5tg )	/* (c) 1997 Merit */
	DRIVER( megat6 )	/* (c) 1998 Merit */
	DRIVER( suprgolf )	/* (c) 1989 Nasco */
	DRIVER( hotblock )	/* (c) 1993 Nics? / Nix? */
	DRIVER( pntnpuzl )	/* Century? */
	DRIVER( trucocl )	/* (c) 1991 Miky SRL */
	DRIVER( ertictac )	/* (c) 1992 Sisteme */
	DRIVER( poizone )	/* (c) 1990 Sisteme */
	DRIVER( ssfindo )	/* (c) 1999 Icarus */
	DRIVER( ppcar )		/* (c) 1999 Icarus */
	DRIVER( imolagp )	/* (c) 1981 Leante Games */
	DRIVER( dambustr )	/* (c) 1981 South West Research */
	DRIVER( dambustra )	/* (c) 1981 South West Research */
	DRIVER( dambustruk )/* (c) 1981 South West Research */
	DRIVER( gunpey )	/* (c) 2000 Banpresto */
	DRIVER( spaceg )	/* (c) 19?? */
	DRIVER( wldarrow )	/* (c) 1982 */
	DRIVER( unkmeyco )	/* (c) 198? */
	DRIVER( mdrawpkr )	/* (c) 19?? */
	DRIVER( mdrawpkra )	/* (c) 19?? */
	DRIVER( laserbas )	/* (c) 1981 Amstar/HOEI */
	DRIVER( laserbasa )	/* (c) 1981 Amstar/HOEI */
	DRIVER( futflash )	/* (c) 1981 HOEI (Arcade TV Game List - P.96, Right, 18 from top) */
	DRIVER( turbosub )	/* (c) 1985 Entertainment Sciences */
	DRIVER( turbosub7 )	/* (c) 1985 Entertainment Sciences */
	DRIVER( turbosub6 )	/* (c) 1985 Entertainment Sciences */
	DRIVER( pipeline )	/* (c) 1990 Daehyun Electronics */
	DRIVER( ttchamp )
	DRIVER( ttchampa )
	DRIVER( dreamwld )	/* (c) 2000 SemiCom */
	DRIVER( cybertnk )	/* (c) 1990 Coreland */
	DRIVER( cubeqst )	/* (c) 1983 Simutrek Inc. */
	DRIVER( cubeqsta )	/* (c) 1983 Simutrek Inc. */
	DRIVER( pkscram )	/* (c) 1993 Cosmo Electronics Corporation */

	/* Impera CD-I based */
	DRIVER( magicard )	/* (c) 199? Impera */
	DRIVER( magicarda )	/* (c) 199? Impera */
	DRIVER( magicardb )	/* (c) 199? Impera */
	DRIVER( magicardj )	/* (c) 1998 Impera */
	DRIVER( magicle )	/* (c) 1998 Impera */

	/* Funworld */
	DRIVER( jollycrd )	/* (c) 1985 TAB-Austria */
	DRIVER( jolyc3x3 )	/* (c) 1985 TAB-Austria */
	DRIVER( jolycmzs )	/* (c) 1993 MZS Tech */
	DRIVER( jolyc980 )	/* (c) 2000 Spale-Soft */
	DRIVER( jolycdev )	/* (c) 1998 TAB/Evona */
	DRIVER( jolyccra )	/* (c) 1985 TAB-Austria */
	DRIVER( jolyccrb )	/* (c) 1993 Soft Design */
	DRIVER( jolycdit )	/* 199? bootleg */
	DRIVER( jolycdib )	/* 1990 bootleg */
	DRIVER( sjcd2kx3 )	/* (c) 1985 M.P.*/
	DRIVER( jolycdab )	/* (c) 1986 Inter Games */
	DRIVER( bigdeal )	/* (c) 1986 Funworld */
	DRIVER( bigdealb )	/* (c) 1986 Funworld */
	DRIVER( jolycdat )	/* (c) 1986 Funworld */
	DRIVER( cuoreuno )	/* (c) 1996 C.M.C. */
	DRIVER( elephfam )	/* (c) 1997 C.M.C. */
	DRIVER( elephfmb )	/* (c) 1996 C.M.C. */
	DRIVER( pool10 )	/* (c) 1996 C.M.C. */
	DRIVER( pool10b )	/* (c) 1996 C.M.C. */
	DRIVER( pool10c )	/* (c) 1996 C.M.C. */
	DRIVER( pool10d )	/* (c) 1997 C.M.C. */
	DRIVER( tortufam )	/* (c) 1997 C.M.C. */
	DRIVER( potgame )	/* (c) 1996 C.M.C. */
	DRIVER( bottle10 )	/* (c) 1996 C.M.C. */
	DRIVER( bottl10b )	/* (c) 1996 C.M.C. */
	DRIVER( royalcrd )	/* (c) 1991 TAB-Austria */
	DRIVER( royalcrda )	/* (c) 1991 TAB-Austria */
	DRIVER( royalcrdb )	/* (c) 1991 TAB-Austria */
	DRIVER( royalcrdc )	/* (c) 1991 TAB-Austria */
	DRIVER( royalcrdd )	/* (c) 1991 TAB-Austria */
	DRIVER( royalcrde )	/* (c) 1991 TAB-Austria */
	DRIVER( royalcrdf )	/* (c) 1991 Evona Electronic */
	DRIVER( royalcrdp )	/* (c) 1993 Digital Dreams */
	DRIVER( lluck3x3 )	/* (c) 1991 TAB-Austria */
	DRIVER( lluck4x1 )	/* (c) 1991 TAB-Austria */
	DRIVER( magicrd2 )	/* (c) 1996 Impera */
	DRIVER( magicd2a )	/* (c) 1996 Impera */
	DRIVER( magicd2b )	/* (c) 1996 Impera */
	DRIVER( vegasslw )	/* (c) 1993 Funworld */
	DRIVER( vegasfst )	/* (c) 1993 Soft Design */
	DRIVER( vegasfte )	/* (c) 1993 Soft Design */
	DRIVER( vegasmil )	/* (c) 1993 Mile */
	DRIVER( jolyjokr )	/* (c) 198? Impera */
	DRIVER( jolyjokra )	/* (c) 198? Impera */
	DRIVER( jolyjokrb )	/* (c) 198? Impera */
	DRIVER( multiwin )	/* (c) 1992 Funworld */
	DRIVER( jokercrd )	/* (c) 1993 Vesely Svet */
	DRIVER( mongolnw )	/* 199? bootleg */
	DRIVER( soccernw )	/* 199? bootleg */
	DRIVER( saloon )	/* 199? unknown */
	DRIVER( funquiz )	/* (c) 198? Funworld */
	DRIVER( photoply )

	/* Sandii' */
	DRIVER( snookr10 )	/* (c) 1998 Sandii' */
	DRIVER( apple10 )	/* (c) 1998 Sandii' */
	DRIVER( tenballs )	/* 1997 unknown */

	/* Game-A-Tron */
	DRIVER( gotya )		/* (c) 1981 Game-A-Tron */
	DRIVER( sbugger )	/* (c) 1981 Game-A-Tron */
	DRIVER( sbuggera )	/* (c) 1981 Game-A-Tron */
	DRIVER( poker41 )	/* (c) 1983 Game-A-Tron */
	DRIVER( pulltabs )	/* (c) 1983 Game-A-Tron */

	DRIVER( vroulet )	/* (c) 1989 World Game */
	DRIVER( rcasino )	/* (c) 1984 Dyna Electronics */
	DRIVER( ampoker2 )	/* (c) 1990 Novomatic */
	DRIVER( ampkr2b1 )	/* 1990 bootleg */
	DRIVER( ampkr2b2 )	/* 1990 bootleg */
	DRIVER( ampkr2b3 )	/* 1994 bootleg */
	DRIVER( ampkr2b4 )	/* 1994 bootleg */
	DRIVER( ampkr228 )	/* 1990 bootleg */
	DRIVER( pkrdewin )	/* 1990 bootleg */
	DRIVER( ampkr95 )	/* 1995 bootleg */
	DRIVER( videomat )	/* 1990 bootleg */
	DRIVER( rabbitpk )	/* 1990 bootleg */
	DRIVER( sigmapkr )	/* (c) 1995 Sigma */
	DRIVER( sigma2k )	/* (c) 2000 Sigma */
	DRIVER( sidewndr )	/* (c) 1981? ACE */
	DRIVER( spellbnd )	/* (c) 1981? ACE */
	DRIVER( starspnr )	/* (c) 1982? ACE */
	DRIVER( acefruit )	/* (c) 1982? ACE */
	DRIVER( attckufo )	/* (c) 1980 Ryoto Electric Co. */
	DRIVER( fortecar )
	DRIVER( jackpool )	/* (c) 1997 Electronic Projects */
	DRIVER( murogem )	/* ??? */
	DRIVER( murogema )	/* ??? */
	DRIVER( murogmbl )	/* ??? */
	DRIVER( lasvegas )	/* hack */
	DRIVER( quizmstr )	/* (c) 1985 Coinmaster */
	DRIVER( trailblz )	/* (c) 1987 Coinmaster */
	DRIVER( supnudg2 )	/* (c) 1989 Coinmaster */
	DRIVER( pokeroul )	/* (c) 1990 Coinmaster */
	DRIVER( egghunt )	/* (c) 1995 Invi Image */
	DRIVER( magic10 )	/* (c) 1995 A.W.P. Games */
	DRIVER( magic10a )	/* (c) 1995 A.W.P. Games */
	DRIVER( magic10b )	/* (c) 1995 A.W.P. Games */
	DRIVER( magic102 )	/* (c) 1997 ABM Games */
	DRIVER( suprpool )	/* (c) 1997 ABM Games */
	DRIVER( hotslot )	/* (c) 1996 ABM Electronics */
	DRIVER( mcolors )	/* 1999 unknown */
	DRIVER( sgsafari )	/* (c) 1996 New Impeuropex Corp. */
	DRIVER( galaxi )	/* (c) 2000 BRL S.R.L. Bologna */
	DRIVER( magjoker )	/* (c) 2000 BRL S.R.L. Bologna */
	DRIVER( dwarfd )	/* (c) 198? Electro-Sports */
	DRIVER( quarterh )	/* (c) 1983 Electro-Sports */
	DRIVER( quarterha )	/* (c) 1983 Electro-Sports */
	DRIVER( quarterhb )	/* (c) 1983 Electro-Sports */
	DRIVER( qc )        /* (c) 1995 ArJay Exports/Prestige Games */
	DRIVER( trvquest )	/* (c) 1984 Sunn / Techstar */
	DRIVER( wheelfir )	/* (c) 199? TCH */
	DRIVER( littlerb )	/* (c) 1993 TCH */
	DRIVER( tattack )	/* (c) 198? Shonan */
	DRIVER( mosaicf2 )	/* (c) 1999 F2 System */
	DRIVER( finalgdr )	/* (c) 2001 Semicom */
	DRIVER( mrkicker )	/* (c) 2001 Semicom */
	DRIVER( wyvernwg )	/* (c) 2001 Semicom / Game Vision */
	DRIVER( rbmk )		/* (c) 1995 GMS */
	DRIVER( go2000 )	/* (c) 2000 SA */
	DRIVER( mirax )		/* (c) 1985 Current Technologies */
	DRIVER( miraxa )	/* (c) 1985 Current Technologies */

	DRIVER( victor5 )	/* (c) 1990 Subsino */
	DRIVER( victor21 )	/* (c) 1990 Subsino */
	DRIVER( crsbingo )	/* (c) 1991 Subsino */
	DRIVER( tisub )		/* (c) 1992 Subsino */
	DRIVER( tisuba )	/* (c) 1992 Subsino */
	DRIVER( stisub )	/* (c) 1995 Subsino (Alpha license) */
	DRIVER( sharkpy )	/* (c) 1996 Subsino */
	DRIVER( sharkpya )	/* (c) 1996 Subsino */
	DRIVER( sharkpye )	/* (c) 1996 Subsino (Alpha license) */
	DRIVER( smoto20 )	/* (c) 1996 Subsino */
	DRIVER( smoto16 )	/* (c) 1996 Subsino */
	DRIVER( mtrain )	/* (c) 1997 Subsino */
	DRIVER( saklove )	/* (c) 1998 Subsino */
	DRIVER( bishjan )	/* (c) 1999 Subsino */
	DRIVER( lastfght )	/* (c) 2000 Subsino */

	DRIVER( sliver )	/* (c) 1996 Hollow Corp */
	DRIVER( intrscti )	/* ???? */
	DRIVER( pasha2 )	/* (c) 1998 Dong Sung */
	DRIVER( trvmadns )	/* (c) 1985 Thunderhead Inc. */
	DRIVER( rcorsair )	/* (c) 1984 Nakasawa */
	DRIVER( pzlestar )	/* (c) 199? Sang Ho Soft */
	DRIVER( sexyboom )	/* (c) 1992 Sang Ho Soft */
	DRIVER( sfkick )	/* (c) 1988 Haesung/HJ Corp */
	DRIVER( sfkicka )	/* (c) 198? Haesung */
	DRIVER( spinkick )	/* (c) 1988 Haesung/Seojin */
	DRIVER( 7mezzo )	/* (c) 198? Unknown */
	DRIVER( magicfly )	/* (c) 198? P&A GAMES */
	DRIVER( dynabomb )	/* (c) 2000 Limenko */
	DRIVER( legendoh )	/* (c) 2000 Limenko */
	DRIVER( sb2003 )	/* (c) 2003 Limenko */
	DRIVER( sb2003a )	/* (c) 2003 Limenko */
	DRIVER( spotty )	/* (c) 2001 Prince Co. */
	DRIVER( carrera )	/* (c) 19?? BS Electronics */

	/* Golden Poker / Potten's Poker / Witch Card hardware */
	DRIVER( pmpoker )	/* (c) 1981 PlayMan */
	DRIVER( goldnpkr )	/* (c) 1981 Bonanza */
	DRIVER( goldnpkb )	/* (c) 1981 Bonanza */
	DRIVER( pottnpkr )	/* Bootleg */
	DRIVER( potnpkra )	/* Bootleg in Coinmaster H/W */
	DRIVER( potnpkrb )	/* Bootleg */
	DRIVER( potnpkrc )	/* Bootleg */
	DRIVER( potnpkrd )	/* Bootleg */
	DRIVER( potnpkre )	/* Bootleg */
	DRIVER( goodluck )	/* Unknown */
	DRIVER( royale )	/* Unknown */
	DRIVER( royalea )	/* Unknown */
	DRIVER( witchcrd )	/* 1991, Video Klein */
	DRIVER( witchcda )	/* 1991, Unknown */
	DRIVER( witchcdb )	/* 1991, Unknown */
	DRIVER( witchcdc )	/* 1991, Unknown */
	DRIVER( witchcdd )	/* 1991, Unknown */
	DRIVER( witchcde )	/* 1994, Unknown */
	DRIVER( witchcdf )	/* (c) 1991 PlayMan */
	DRIVER( sloco93 )	/* 1993, Unknown */
	DRIVER( sloco93a )	/* 1993, Unknown */
	DRIVER( maverik )	/* 198?, Unknown */
	DRIVER( brasil89 )	/* 1989, Unknown */
	DRIVER( poker91 )	/* 1991, Unknown */
	DRIVER( bsuerte )	/* 1990, Unknown */
	DRIVER( bsuertea )	/* 1991, Unknown */
	DRIVER( bsuerteb )	/* 1991, Unknown */
	DRIVER( bsuertec )	/* 1991, Unknown */
	DRIVER( bsuerted )	/* 1991, Unknown */
	DRIVER( bsuertee )	/* 1991, Unknown */
	DRIVER( bsuertef )	/* 1991, Unknown */
	DRIVER( bsuerteg )	/* 1991, Unknown */
	DRIVER( bsuerteh )	/* 1991, Unknown */
	DRIVER( bsuertei )	/* 1991, Unknown */
	DRIVER( bsuertej )	/* 1991, Unknown */
	DRIVER( bsuertek )	/* 1991, Unknown */
	DRIVER( bsuertel )	/* 1991, Unknown */
	DRIVER( bsuertem )	/* 1991, Unknown */
	DRIVER( bsuerten )	/* 1991, Unknown */
	DRIVER( bsuerteo )	/* 1991, Unknown */
	DRIVER( bsuertep )	/* 1991, Unknown */
	DRIVER( bsuerteq )	/* 1991, Unknown */
	DRIVER( bsuerter )	/* 1991, Unknown */
	DRIVER( bsuertes )	/* 1991, Unknown */
	DRIVER( bsuertet )	/* 1991, Unknown */
	DRIVER( bsuerteu )	/* 1991, Unknown */

	/* Cal Omega */
	DRIVER( comg074 )	/* (c) 1981 Cal Omega Inc. */
	DRIVER( comg076 )	/* (c) 1981 Cal Omega Inc. */
	DRIVER( comg079 )	/* (c) 1981 Cal Omega Inc. */
	DRIVER( comg080 )	/* (c) 1981 Cal Omega Inc. */
	DRIVER( comg094 )	/* (c) 1981 Cal Omega Inc. */
	DRIVER( comg107 )	/* (c) 1982 Cal Omega Inc. */
	DRIVER( comg123 )	/* (c) 1982 Cal Omega Inc. */
	DRIVER( comg125 )	/* (c) 1982 Cal Omega Inc. */
	DRIVER( comg127 )	/* (c) 1982 Cal Omega Inc. */
	DRIVER( comg128 )	/* (c) 1982 Cal Omega Inc. */
	DRIVER( comg134 )	/* (c) 1982 Cal Omega Inc. */
	DRIVER( comg145 )	/* (c) 1982 Cal Omega Inc. */
	DRIVER( comg157 )	/* (c) 1983 Cal Omega Inc. */
	DRIVER( comg159 )	/* (c) 1983 Cal Omega Inc. */
	DRIVER( comg164 )	/* (c) 1983 Cal Omega Inc. */
	DRIVER( comg168 )	/* (c) 1983 Cal Omega Inc. */
	DRIVER( comg172 )	/* (c) 1983 Cal Omega Inc. */
	DRIVER( comg175 )	/* (c) 1984 Cal Omega / Casino Electronics Inc. */
	DRIVER( comg176 )	/* (c) 1982 Cal Omega Inc. */
	DRIVER( comg181 )	/* (c) 1983 Cal Omega Inc. */
	DRIVER( comg183 )	/* (c) 1983 Cal Omega Inc. */
	DRIVER( comg185 )	/* (c) 1983 Cal Omega Inc. */
	DRIVER( comg186 )	/* (c) 1983 Cal Omega Inc. */
	DRIVER( comg187 )	/* (c) 1983 Cal Omega Inc. */
	DRIVER( comg204 )	/* (c) 1984 Cal Omega Inc. */
	DRIVER( comg208 )	/* (c) 1984 Cal Omega Inc. */
	DRIVER( comg227 )	/* (c) 1984 Cal Omega Inc. */
	DRIVER( comg230 )	/* (c) 1984 Cal Omega Inc. */
	DRIVER( comg236 )	/* (c) 1984 Cal Omega Inc. */
	DRIVER( comg239 )	/* (c) 1985 Cal Omega / Casino Electronics Inc. */
	DRIVER( comg240 )	/* (c) 1985 Cal Omega / Casino Electronics Inc. */
	DRIVER( comg246 )	/* (c) 1985 Cal Omega Inc. */
	DRIVER( comg272a )	/* (c) 1985 Cal Omega Inc. */
	DRIVER( comg272b )	/* (c) 1985 Cal Omega Inc. */
	DRIVER( comg5108 )	/* (c) 1985 Cal Omega Inc. */
	DRIVER( comg903d )	/* (c) 198? Cal Omega Inc. */
	DRIVER( comg905d )	/* (c) 198? Cal Omega Inc. */
	DRIVER( elgrande )	/* (c) 1982 Tuni Electro Service / E.T. Marketing */
	DRIVER( jjpoker )	/* (c) 1983 Enter-Tech */
	DRIVER( jjpokerb )	/* (c) 1983 Enter-Tech */
	DRIVER( ssipkr24 )	/* (c) 1988 SSI */
	DRIVER( ssipkr30 )	/* (c) 1988 SSI */
	DRIVER( ssipkr40 )	/* (c) 1990 SSI */

	DRIVER( altair )	/* (c) 1981 Cidelsa */
	DRIVER( destryer )	/* (c) 1981 Cidelsa */
	DRIVER( destryera )	/* (c) 1981 Cidelsa */
	DRIVER( draco )		/* (c) 1981 Cidelsa */
	DRIVER( rockduck )	/* (c) 1983 Datel SAS */
	DRIVER( tetriskr )	/* (c) 1988? bootleg */
	DRIVER( trvhang )	/* (c) 1984 SMS MFG CORP */
	DRIVER( trvhanga )	/* (c) 1984 SMS MFG CORP */
	DRIVER( sureshot )	/* (c) 1985 SMS MFG CORP */
	DRIVER( secondch )  /* (c) 1985 SMS MFG CORP */
	DRIVER( skylncr )	/* (c) 1995 Bordun International */
	DRIVER( butrfly )	/* (c) 1995 Bordun International */
	DRIVER( madzoo )	/* (c) 1995 Bordun International */
	DRIVER( leader )	/* Bootleg? */
	DRIVER( livequiz )	/* (c) 1999 Andamiro */
	DRIVER( hammer )	/* (c) 2000 Andamiro */
	DRIVER( cowrace )	/* (c) 20?? hack */
	DRIVER( kingdrbb )  /* (c) 199? hack */
	DRIVER( discoboy )	/* (c) 1993 Soft Art Co. */
	DRIVER( pangofun )	/* (c) 1995 InfoCube */
	DRIVER( 2mindril )	/* (c) 1993 Taito */
	DRIVER( koikoi )	/* (c) 1983 Kiwako */
	DRIVER( good )		/* (c) 1983 Kiwako */
	DRIVER( filetto )	/* (c) 1990 Novamatic */
	DRIVER( quizo )		/* (c) 1985 Seoul Coin Corp. */
	DRIVER( quizoa )	/* (c) 1985 Seoul Coin Corp. */
	DRIVER( gstream )	/* (c) 2002, Oriental Soft Japan */
	DRIVER( miniboy7 )	/* (c) 1983, Bonanza Enterprises */
	DRIVER( miniboy7a )	/* (c) 1983, Bonanza Enterprises */
	DRIVER( 39in1 )		/* MAME based bootleg */
	DRIVER( 48in1 )		/* MAME based bootleg */
	DRIVER( 48in1a )    	/* MAME based bootleg */
	DRIVER( kingpin )	/* (c) 1983 American Communications Laboratories Inc. */
	DRIVER( kingpinm )	/* (c) 1983 American Communications Laboratories Inc. */
	DRIVER( thayers )	/* (c) 1983 RDI Video Systems */
	DRIVER( thayersa )	/* (c) 1983 RDI Video Systems */
	DRIVER( istellar )	/* (c) 1983 Funai / Gakken */
	DRIVER( esh )		/* (c) 1984 Funai / Gakken */
	DRIVER( esha )		/* (c) 1984 Funai / Gakken */
	DRIVER( eshb )		/* (c) 1984 Funai / Gakken */
	DRIVER( comebaby )	/* (c) 2000 ExPotato */
	DRIVER( peset038 )	/* (c) 1987 IGT - International Gaming Technology */
	DRIVER( pepp0043 )	/* (c) 1987 IGT - International Gaming Technology */
	DRIVER( pepp0065 )	/* (c) 1987 IGT - International Gaming Technology */
	DRIVER( pepp0158 )	/* (c) 1987 IGT - International Gaming Technology */
	DRIVER( pepp0188 )	/* (c) 1987 IGT - International Gaming Technology */
	DRIVER( pepp0250 )	/* (c) 1987 IGT - International Gaming Technology */
	DRIVER( pepp0447 )	/* (c) 1987 IGT - International Gaming Technology */
	DRIVER( pepp0516 )	/* (c) 1987 IGT - International Gaming Technology */
	DRIVER( pebe0014 )	/* (c) 1994 IGT - International Gaming Technology */
	DRIVER( peke1012 )	/* (c) 1994 IGT - International Gaming Technology */
	DRIVER( peps0014 )	/* (c) 1996 IGT - International Gaming Technology */
	DRIVER( peps0022 )	/* (c) 1996 IGT - International Gaming Technology */
	DRIVER( peps0043 )	/* (c) 1996 IGT - International Gaming Technology */
	DRIVER( peps0045 )	/* (c) 1996 IGT - International Gaming Technology */
	DRIVER( peps0308 )	/* (c) 1996 IGT - International Gaming Technology */
	DRIVER( peps0615 )	/* (c) 1996 IGT - International Gaming Technology */
	DRIVER( peps0716 )	/* (c) 1996 IGT - International Gaming Technology */
	DRIVER( pex2069p )	/* (c) 1995 IGT - International Gaming Technology */
	DRIVER( pexp0019 )	/* (c) 1995 IGT - International Gaming Technology */
	DRIVER( pexp0112 )	/* (c) 1995 IGT - International Gaming Technology */
	DRIVER( pexs0006 )	/* (c) 1997 IGT - International Gaming Technology */
	DRIVER( pexmp006 )	/* (c) 1997 IGT - International Gaming Technology */
	DRIVER( pexmp017 )	/* (c) 1997 IGT - International Gaming Technology */
	DRIVER( pexmp024 )	/* (c) 1997 IGT - International Gaming Technology */
	DRIVER( mirage )	/* (c) 1994 Mitchell */
	DRIVER( dominob )	/* (c) 1996 Wonwoo Systems */
	DRIVER( dominobv2 )	/* (c) 1996 Wonwoo Systems */
	DRIVER( jongkyo )	/* (c) 1985 Kiwako */
	DRIVER( gamecstl )	/* MAME based bootleg */
	DRIVER( gamecst2 )	/* MAME based bootleg, version 2.613 */
	DRIVER( drw80pkr )	/* (c) 1982 IGT */
	DRIVER( drw80pk2 )	/* (c) 1983 IGT */
	DRIVER( fortune1 )	/* (c) 1984 IGT */
	DRIVER( blazlaz )	/* (c) 1989 Hudson Soft */
	DRIVER( ggconnie )	/* (c) ???? Capcom */
	DRIVER( paranoia )	/* (c) 1990 Naxat Soft */
	DRIVER( vcombat )	/* (c) 1993 VR8 Inc. */
	DRIVER( shadfgtr )	/* (c) 1993 DUTECH Inc. */
	DRIVER( skimaxx )	/* (c) 1996 Kyle Hodgetts/ICE */
	DRIVER( blackt96 )	/* (c) 1996 D.G.R.M. of Korea */
	DRIVER( magictg )	/* (c) 199? Acclaim */
	DRIVER( magictga )	/* (c) 199? Acclaim */
	DRIVER( re900 )		/* (c) 1993 Entretenimientos GEMINIS */
	DRIVER( bs94 )		/* (c) 1994 Entretenimientos GEMINIS */
	DRIVER( jokrwild )	/* (c) 1988 Sigma */
	DRIVER( pesadelo )	/* (c) 1989 Forte II Games */
	DRIVER( pengadvb )	/* (c) 1988 Screen */
	DRIVER( spool99 )	/* (c) 1998 Electronic Projects */
	DRIVER( spool99a )	/* (c) 1998 Electronic Projects */
	DRIVER( vcarn )		/* (c) 1999 Electronic Projects */
	DRIVER( 4roses )	/* (c) 1999 Unknown */
	DRIVER( 4rosesa )	/* (c) 1999 Unknown */
	DRIVER( big10 )		/* (c) 198? Unknown */
	DRIVER( bingor1 )	/* (c) 2002 Unknown */
	DRIVER( bingor2 )	/* (c) 2002 Unknown */
	DRIVER( bingor3 )	/* (c) 2002 Unknown */
	DRIVER( bingor4 )	/* (c) 2002 Unknown */
	DRIVER( bingor5 )	/* (c) 2002 Unknown */

	/* InterFlip / Recreativos Franco */
	DRIVER( videopkr )	/* (c) 1984 InterFlip */
	DRIVER( blckjack )	/* (c) 1984 InterFlip */
	DRIVER( videodad )	/* (c) 1987 InterFlip */
	DRIVER( videocba )	/* (c) 1987 InterFlip */
	DRIVER( babypkr )	/* (c) 1987 Recreativos Franco */
	DRIVER( babydad )	/* (c) 1987 Recreativos Franco */

	/* H8/3048 italian gambling games */
	DRIVER( laperla )	/* (c) 2002 Nazionale Elettronica */
	DRIVER( laperlag )	/* (c) 2001 Nazionale Elettronica */
	DRIVER( capunc )	/* (c) 2000 Nazionale Elettronica */
	DRIVER( capcor )	/* (c) 2001 Nazionale Elettronica */
	DRIVER( euro2k2 )	/* (c) 2001 Nazionale Elettronica */
	DRIVER( euro2k2a )	/* (c) 2001 Nazionale Elettronica */
	DRIVER( euro2k2s )	/* (c) 2002 Nazionale Elettronica */
	DRIVER( mnumber )	/* (c) 200? M.M. - B.R.L. */
	DRIVER( abacus )	/* (c) 200? unknown */
	DRIVER( bookthr )	/* (c) 200? unknown */

	/* H8/3337 italian gambling games */
	DRIVER( ntcash )	/* (c) 1999 unknown */
	DRIVER( wizard )	/* (c) 1999 A.A. */
	DRIVER( trstar2k )	/* (c) 1999 A.M. */
	DRIVER( laser2k1 )	/* (c) 2001 unknown */
	DRIVER( mdrink )	/* (c) 2001 unknown */
	DRIVER( te0144 )	/* (c) 2001 unknown */
	DRIVER( cmagica )	/* (c) 200? unknown */
	DRIVER( millsun )	/* (c) 200? unknown */
	DRIVER( sspac2k1 )	/* (c) 200? unknown */
	DRIVER( elvis )		/* (c) 200? unknown */
	DRIVER( sstar )		/* (c) 200? unknown */
	DRIVER( pirati )	/* (c) 2001 Cin */
	DRIVER( mnumitg )	/* (c) 200? unknown */
	DRIVER( mclass )	/* (c) 200? unknown */
	DRIVER( europass )	/* (c) 200? unknown */

	/* Intel 8086-based italian gambling games */
	DRIVER( newmcard )	/* (c) 19?? unknown */
	DRIVER( brasil )	/* (c) 2000 unknown */
	DRIVER( fashion )	/* (c) 2000 unknown */
	DRIVER( ciclone )   /* (c) 2000 unknown */

	/* V30-based italian gambling games */
	DRIVER( tour4000 )	/* (c) 2000 High Video */
	DRIVER( cfever40 )	/* (c) 2000 High Video */
	DRIVER( cfever50 )	/* (c) 2000 High Video */
	DRIVER( tour4010 )	/* (c) 2000 High Video */
	DRIVER( cfever51 )	/* (c) 2000 High Video */
	DRIVER( cfever61 )	/* (c) 2000 High Video */
	DRIVER( cfever1k )	/* (c) 2000 High Video */
	DRIVER( girotutt )	/* (c) 2000 High Video */

	/* M68k-based gambling games */
	DRIVER( steaser )	/* (c) 1993 unknown */
	DRIVER( ilpag )		/* (c) 199? unknown */

	/* Sure games */
	DRIVER( mil4000 )	/* (c) 2000 Sure Milano */
	DRIVER( mil4000a )	/* (c) 2000 Sure Milano */
	DRIVER( mil4000b )	/* (c) 2000 Sure Milano */
	DRIVER( mil4000c )	/* (c) 2000 Sure Milano */

	/* Mitsubishi/Renesas M16/62A italian gambling games */
	DRIVER( ejollyx5 )	/* (c) 200? Solar Games */
	DRIVER( grandprx )	/* (c) 200? 4fun */
	DRIVER( supjolly )	/* (c) 200? unknown */
	DRIVER( x5jokers )	/* (c) 200? Electronic Projects */

	/* Astro Corp. */
	DRIVER( showhand )	/* (c) 2000  Astro Corp. */
	DRIVER( showhanc )	/* (c) 2000  Astro Corp. */
	DRIVER( skilldrp )	/* (c) 2002  Astro Corp. */
	DRIVER( speeddrp )	/* (c) 2003  Astro Corp. */
	DRIVER( winbingo )	/* (c) 2005? Astro Corp. */
	DRIVER( winbingoa )	/* (c) 2005? Astro Corp. */
	DRIVER( zoo )		/* (c) 2005? Astro Corp. */

	/* Dyna */
	DRIVER( ncb3 )		/* (c) 199? Dyna Electronics */
	DRIVER( cb3a )		/* (c) 199? Dyna Electronics */
	DRIVER( cb3 )		/* (c) 199? Dyna Electronics */
	DRIVER( cb3b )		/* (c) 199? Dyna Electronics */
	DRIVER( cb3c )		/* (c) 199? Dyna Electronics */
	DRIVER( cmv801 )	/* (c) 198? Corsica */
	DRIVER( cmv4 )		/* (c) 1992 Dyna Electronics */
	DRIVER( cmv4a )		/* (c) 1992 Dyna Electronics */
	DRIVER( cmwm )		/* (c) 199? Dyna Electronics */
	DRIVER( cmfun )		/* (c) 1995 Dyna Electronics */
	DRIVER( cmaster )	/* (c) 1991 Dyna Electronics */
	DRIVER( cmasterb )	/* (c) 1991 Dyna Electronics */
	DRIVER( cmezspin )	/* (c) 1991 Dyna Electronics */
	DRIVER( cmasterc )	/* (c) 1991 Dyna Electronics */
	DRIVER( cmasterbv )	/* (c) 1991 Dyna Electronics / Gerald Duhamel?*/
	DRIVER( cmasterd )	/* (c) 1991 Dyna Electronics */
	DRIVER( cmastere )	/* (c) 1991 Dyna Electronics */
	DRIVER( cmasterf )	/* (c) 1991 Dyna Electronics */
	DRIVER( cmast91 )	/* (c) 1991 Dyna Electronics */
	DRIVER( cmast92 )	/* (c) 1992 Dyna Electronics */

	DRIVER( jkrmast )	/* (c) 199? unknown */
	DRIVER( pkrmast )	/* (c) 199? unknown */
	DRIVER( pkrmasta )	/* (c) 199? unknown */

	DRIVER( cb2001 )	/* (c) 2001 Dyna Electronics */
	DRIVER( scherrym )	/* (c) 2001? Dyna Electronics */

	/* STG? */
	DRIVER( queen )

	/* Aristocrat */
	DRIVER( 86lions )	/* (c) 1985? */
	DRIVER( caswin )	/* (c) 1985 */

	/* Aristocrat Mk4 hardware */
	DRIVER( 3bagflvt )	/* (c) 1994 */
	DRIVER( 3bagflnz )	/* (c) 1994 */
	DRIVER( blkrhino )	/* (c) 1996 */
	DRIVER( coralr2 )	/* (c) 2002 */
	DRIVER( eforest )	/* (c) 1994 */
	DRIVER( eforesta )	/* (c) 1995 */
	DRIVER( eforestb )	/* (c) 1996 */
	DRIVER( goldenc )	/* (c) 1996 */
	DRIVER( kgbird )	/* (c) 1996 */
	DRIVER( kgbirda )	/* (c) 1996 */
	DRIVER( phantomp )	/* (c) 1998 */
	DRIVER( swtht2nz )	/* (c) 1998 */
	DRIVER( topgear )	/* (c) 1996 */
	DRIVER( wtigernz )	/* (c) 1996 */
	DRIVER( autmoon )	/* (c) 1999 */
	DRIVER( ffortune )	/* (c) 1998 */

	/* Aristocrat Mk5 hardware */
	DRIVER( aristmk5 )	/* (c) 1995 */

	DRIVER( swthrt2v )	/* (c) 1995 */
	DRIVER( enchfrst )	/* (c) 1995 */
	DRIVER( dolphntr )	/* (c) 1996 */
	DRIVER( dolphtra )	/* (c) 1996 */
	DRIVER( dmdtouch )	/* (c) 1997 */
	DRIVER( goldprmd )	/* (c) 1997 */
	DRIVER( qotn )		/* (c) 1997 */
	DRIVER( adonis )	/* (c) 1998 */
	DRIVER( reelrock )	/* (c) 1998 */
	DRIVER( indiandr )	/* (c) 1998 */
	DRIVER( magicmsk )	/* (c) 2000 */
	DRIVER( margmgc )	/* (c) 2000 */
	DRIVER( geishanz )	/* (c) 2001 */

	/* Eagle */
	DRIVER( janshi )
	DRIVER( ronjan )
	DRIVER( pinkiri8 )

	/* Wing */
	DRIVER( slotcarn )	/* (c) 1985 Wing Co. Ltd */
	DRIVER( lucky74 )	/* (c) 1988 Wing Co. Ltd */
	DRIVER( lucky74a )	/* (c) 1988 Wing Co. Ltd */
	DRIVER( lucky8 )	/* (c) 1989 Wing Co. Ltd */
	DRIVER( lucky8a )	/* (c) 1989 Wing Co. Ltd */
	DRIVER( lucky8b )	/* (c) 1989 Wing Co. Ltd */
	DRIVER( luckgrln )	/* (c) 1991 Wing Co. Ltd */

	DRIVER( skill98 )	/* (c) 1998 Amcoe */
	DRIVER( match98 )	/* (c) 1998 Amcoe */
	DRIVER( schery97 )	/* (c) 1998 Amcoe */
	DRIVER( schery97a )	/* (c) 1998 Amcoe */
	DRIVER( roypok96 )	/* (c) 1996 Amcoe */
	DRIVER( roypok96a )	/* (c) 1996 Amcoe */
	DRIVER( roypok96b )	/* (c) 1996 Amcoe */
	DRIVER( pokonl97 )	/* (c) 1997 Amcoe */
	DRIVER( nc96 )		/* (c) 1996 Amcoe */
	DRIVER( nc96a )		/* (c) 1996 Amcoe */
	DRIVER( nc96b )		/* (c) 1996 Amcoe */
	DRIVER( nc96c )		/* (c) 1996 Amcoe */
	DRIVER( nc96txt )	/* (c) 2000 Amcoe */
	DRIVER( nfb96 )		/* (c) 1996 Amcoe */
	DRIVER( nfb96a )	/* (c) 1996 Amcoe */
	DRIVER( nfb96b )	/* (c) 1996 Amcoe */
	DRIVER( nfb96c )	/* (c) 1996 Amcoe */
	DRIVER( nfb96txt )	/* (c) 2000 Amcoe */
	DRIVER( nfb96se )	/* bootleg */
	DRIVER( nfb96sea )	/* bootleg */
	DRIVER( nfb96seb )	/* bootleg */
	DRIVER( carb2002 )	/* bootleg */
	DRIVER( carb2003 )	/* bootleg */
	DRIVER( nfm )		/* bootleg */
	DRIVER( unkch1 )	/* bootleg */
	DRIVER( unkch2 )	/* bootleg */
	DRIVER( unkch3 )	/* bootleg */
	DRIVER( unkch4 )	/* bootleg */

	DRIVER( ns8lines )	/* unknown */
	DRIVER( ns8linew )	/* unknown */
	DRIVER( ladylinr )	/* (c) 198? TAB Austria */
	DRIVER( kkojnoli )	/* 198? south korean hack */
	DRIVER( wcat3 )
	DRIVER( mtonic )	/* 198? (c) Tonic */
	DRIVER( magical )	/* 198? (c) Micro Manufacturing Ltd. */
	DRIVER( magicala )	/* 198? (c) Micro Manufacturing Ltd. */

	DRIVER( sfbonus )	/* (c) 2003 Amcoe - Skill Fruit Bonus */
	DRIVER( sfbonusd1 )	/* (c) 2003 Amcoe */
	DRIVER( sfbonusv1 )	/* (c) 2003 Amcoe */
	DRIVER( sfbonuso )	/* (c) 2003 Amcoe */
	DRIVER( sfbonuso2 )	/* (c) 2003 Amcoe */
	DRIVER( parrot3 )	/* (c) 2003 Amcoe - Parrot Poker III */
	DRIVER( parrot3b1 )	/* (c) 2004 Amcoe */
	DRIVER( parrot3d1 )	/* (c) 2004 Amcoe */
	DRIVER( parrot3v1 )	/* (c) 2004 Amcoe */
	DRIVER( parrot3o )	/* (c) 2004 Amcoe */
	DRIVER( hldspin1 )	/* (c) 2000 Amcoe - Hold & Spin I */
	DRIVER( hldspin1dt )	/* (c) 2000 Amcoe */
	DRIVER( hldspin1vt )	/* (c) 2000 Amcoe */
	DRIVER( hldspin1o )	/* (c) 2000 Amcoe */
	DRIVER( hldspin2 )	/* (c) 2000 Amcoe - Hold & Spin II */
	DRIVER( hldspin2d1 )	/* (c) 2000 Amcoe */
	DRIVER( hldspin2v1 )	/* (c) 2000 Amcoe */
	DRIVER( hldspin2o )	/* (c) 2000 Amcoe */
	DRIVER( fcnudge )	/* (c) 2003 Amcoe - Fruit Carnival Nudge */
	DRIVER( fcnudgeo )	/* (c) 2003 Amcoe */
	DRIVER( fcnudgeo2 )	/* (c) 2003 Amcoe */
	DRIVER( fcnudgeo3 )	/* (c) 2003 Amcoe */
	DRIVER( pickwin )	/* (c) 2001 Amcoe - Pick 'n Win */
	DRIVER( pickwinb1 )	/* (c) 2001 Amcoe */
	DRIVER( pickwind1 )	/* (c) 2001 Amcoe */
	DRIVER( pickwinv1 )	/* (c) 2001 Amcoe */
	DRIVER( pickwinbt )	/* (c) 2001 Amcoe */
	DRIVER( pickwindt )	/* (c) 2001 Amcoe */
	DRIVER( pickwinvt )	/* (c) 2001 Amcoe */
	DRIVER( pickwino )	/* (c) 2001 Amcoe */
	DRIVER( pickwino2 )	/* (c) 2001 Amcoe */
	DRIVER( tighook )	/* (c) 2004 Amcoe - Tiger Hook */
	DRIVER( tighookc1 )	/* (c) 2004 Amcoe */
	DRIVER( tighookd1 )	/* (c) 2004 Amcoe */
	DRIVER( tighookv1 )	/* (c) 2004 Amcoe */
	DRIVER( tighookc2 )	/* (c) 2004 Amcoe */
	DRIVER( tighookd2 )	/* (c) 2004 Amcoe */
	DRIVER( tighookv2 )	/* (c) 2004 Amcoe */
	DRIVER( tighooko )	/* (c) 2004 Amcoe */
	DRIVER( tighooko2 )	/* (c) 2004 Amcoe */
	DRIVER( robadv )	/* (c) 2004 Amcoe - Robin's Adventure */
	DRIVER( robadvc1 )	/* (c) 2004 Amcoe */
	DRIVER( robadvd1 )	/* (c) 2004 Amcoe */
	DRIVER( robadvv1 )	/* (c) 2004 Amcoe */
	DRIVER( robadvo )	/* (c) 2004 Amcoe */
	DRIVER( robadv2 )	/* (c) 2004 Amcoe - Robin's Adventure 2 */
	DRIVER( robadv2c1 )	/* (c) 2004 Amcoe */
	DRIVER( robadv2d1 )	/* (c) 2004 Amcoe */
	DRIVER( robadv2v1 )	/* (c) 2004 Amcoe */
	DRIVER( robadv2c2 )	/* (c) 2004 Amcoe */
	DRIVER( robadv2d2 )	/* (c) 2004 Amcoe */
	DRIVER( robadv2v2 )	/* (c) 2004 Amcoe */
	DRIVER( robadv2c3 )	/* (c) 2004 Amcoe */
	DRIVER( robadv2d3 )	/* (c) 2004 Amcoe */
	DRIVER( robadv2v3 )	/* (c) 2004 Amcoe */
	DRIVER( robadv2o )	/* (c) 2004 Amcoe */
	DRIVER( robadv2o2 )	/* (c) 2004 Amcoe */
	DRIVER( pirpok2 )	/* (c) 2003 Amcoe - Pirate Poker II */
	DRIVER( pirpok2b1 )	/* (c) 2003 Amcoe */
	DRIVER( pirpok2d1 )	/* (c) 2003 Amcoe */
	DRIVER( pirpok2v1 )	/* (c) 2003 Amcoe */
	DRIVER( pirpok2o )	/* (c) 2003 Amcoe */
	DRIVER( anibonus )	/* (c) 2003 Amcoe - Animal Bonus */
	DRIVER( anibonusv1 )	/* (c) 2003 Amcoe */
	DRIVER( anibonusv2 )	/* (c) 2003 Amcoe */
	DRIVER( anibonusb1 )	/* (c) 2003 Amcoe */
	DRIVER( anibonusd1 )	/* (c) 2003 Amcoe */
	DRIVER( anibonusb2 )	/* (c) 2003 Amcoe */
	DRIVER( anibonusd2 )	/* (c) 2003 Amcoe */
	DRIVER( anibonuso )	/* (c) 2003 Amcoe */
	DRIVER( anibonuso2 )	/* (c) 2003 Amcoe */
	DRIVER( anibonuso3 )	/* (c) 2003 Amcoe */
	DRIVER( anibonusxo )	/* (c) 2003 Amcoe */
	DRIVER( anibonusxo2 )	/* (c) 2003 Amcoe */
	DRIVER( anibonusxo3 )	/* (c) 2003 Amcoe */
	DRIVER( abnudge )	/* (c) 2003 Amcoe - Animal Bonus Nudge */
	DRIVER( abnudgeb )	/* (c) 2003 Amcoe */
	DRIVER( abnudged )	/* (c) 2003 Amcoe */
	DRIVER( abnudgeo )	/* (c) 2003 Amcoe */
	DRIVER( dblchal )	/* (c) 2003 Amcoe - Double Challenge */
	DRIVER( dblchalc1 )	/* (c) 2003 Amcoe */
	DRIVER( dblchald1 )	/* (c) 2003 Amcoe */
	DRIVER( dblchalv1 )	/* (c) 2003 Amcoe */
	DRIVER( dblchalo )	/* (c) 2003 Amcoe */
	DRIVER( anithunt )	/* (c) 2003 Amcoe - Animal Treasure Hunt */
	DRIVER( anithuntd1 )	/* (c) 2003 Amcoe */
	DRIVER( anithuntv1 )	/* (c) 2003 Amcoe */
	DRIVER( anithunto )	/* (c) 2003 Amcoe */
	DRIVER( anithunto2 )	/* (c) 2003 Amcoe */
	DRIVER( sfruitb )	/* (c) 2002 Amcoe - Super Fruit Bonus */
	DRIVER( sfruitbb1 )	/* (c) 2002 Amcoe */
	DRIVER( sfruitbd1 )	/* (c) 2002 Amcoe */
	DRIVER( sfruitbv1 )	/* (c) 2002 Amcoe */
	DRIVER( sfruitbb2 )	/* (c) 2002 Amcoe */
	DRIVER( sfruitbd2 )	/* (c) 2002 Amcoe */
	DRIVER( sfruitbv2 )	/* (c) 2002 Amcoe */
	DRIVER( sfruitbo )	/* (c) 2002 Amcoe */
	DRIVER( sfruitbo2 )	/* (c) 2002 Amcoe */
	DRIVER( sfruitbh )	/* (c) 2002 Amcoe */
	DRIVER( sfruitbbh )	/* (c) 2002 Amcoe */
	DRIVER( sfruitbdh )	/* (c) 2002 Amcoe */
	DRIVER( sfruitbvh )	/* (c) 2002 Amcoe */
	DRIVER( sfruitboh )	/* (c) 2002 Amcoe */
	DRIVER( fb2gen )	/* (c) 2004 Amcoe - Fruit Bonus 2nd Generation */
	DRIVER( fb2genc1 )	/* (c) 2004 Amcoe */
	DRIVER( fb2gend1 )	/* (c) 2004 Amcoe */
	DRIVER( fb2genv1 )	/* (c) 2004 Amcoe */
	DRIVER( fb2genc2 )	/* (c) 2004 Amcoe */
	DRIVER( fb2gend2 )	/* (c) 2004 Amcoe */
	DRIVER( fb2genv2 )	/* (c) 2004 Amcoe */
	DRIVER( fb2geno )	/* (c) 2004 Amcoe */
	DRIVER( fb2geno2 )	/* (c) 2004 Amcoe */
	DRIVER( fb2nd )		/* (c) 2004 Amcoe - Fruit Bonus 2nd Edition */
	DRIVER( fb2ndd1 )	/* (c) 2004 Amcoe */
	DRIVER( fb2ndv1 )	/* (c) 2004 Amcoe */
	DRIVER( fb2ndc2 )	/* (c) 2004 Amcoe */
	DRIVER( fb2ndd2 )	/* (c) 2004 Amcoe */
	DRIVER( fb2ndv2 )	/* (c) 2004 Amcoe */
	DRIVER( fb2ndo )	/* (c) 2004 Amcoe */
	DRIVER( fb4 )		/* (c) 2004 Amcoe - Fruit Bonus 2004 */
	DRIVER( fb4c1 )		/* (c) 2004 Amcoe */
	DRIVER( fb4d1 )		/* (c) 2004 Amcoe */
	DRIVER( fb4v1 )		/* (c) 2004 Amcoe */
	DRIVER( fb4exp )	/* (c) 2004 Amcoe */
	DRIVER( fb4b2 )		/* (c) 2004 Amcoe */
	DRIVER( fb4c2 )		/* (c) 2004 Amcoe */
	DRIVER( fb4d2 )		/* (c) 2004 Amcoe */
	DRIVER( fb4v2 )		/* (c) 2004 Amcoe */
	DRIVER( fb4o )		/* (c) 2004 Amcoe */
	DRIVER( fb4o2 )		/* (c) 2004 Amcoe */
	DRIVER( fb5 )		/* (c) 2005 Amcoe - Fruit Bonus 2005 */
	DRIVER( fb5c )		/* (c) 2005 Amcoe */
	DRIVER( fb5d )		/* (c) 2005 Amcoe */
	DRIVER( fb5v )		/* (c) 2005 Amcoe */
	DRIVER( fb6 )		/* (c) 2006 Amcoe - Fruit Bonus '06 - 10th anniversary */
	DRIVER( fb6v )		/* (c) 2006 Amcoe */
	DRIVER( fb6d1 )		/* (c) 2006 Amcoe */
	DRIVER( fb6s1 )		/* (c) 2006 Amcoe */
	DRIVER( fb6v1 )		/* (c) 2006 Amcoe */
	DRIVER( fb6d2 )		/* (c) 2006 Amcoe */
	DRIVER( fb6s2 )		/* (c) 2006 Amcoe */
	DRIVER( fb6v2 )		/* (c) 2006 Amcoe */
	DRIVER( fb6s3 )		/* (c) 2006 Amcoe */
	DRIVER( fb6se )		/* (c) 2006 Amcoe - Fruit Bonus 2006 Special Edition */
	DRIVER( fb6sev )	/* (c) 2006 Amcoe */
	DRIVER( fb6sed1 )	/* (c) 2006 Amcoe */
	DRIVER( fb6sev1 )	/* (c) 2006 Amcoe */
	DRIVER( fb6sed2 )	/* (c) 2006 Amcoe */
	DRIVER( fb6sev2 )	/* (c) 2006 Amcoe */
	DRIVER( version4 )	/* (c) 2006 Amcoe - Version 4 ? */
	DRIVER( bugfever )	/* (c) 2006 Amcoe - Bug Fever */
	DRIVER( bugfevero )	/* (c) 2006 Amcoe */
	DRIVER( bugfeverd )	/* (c) 2006 Amcoe */
	DRIVER( bugfeverv )	/* (c) 2006 Amcoe */
	DRIVER( bugfeverv2 )	/* (c) 2006 Amcoe */
	DRIVER( funriver )	/* (c) 2005 Amcoe - Fun River */
	DRIVER( funriverv )	/* (c) 2005 Amcoe */
	DRIVER( dvisland )	/* (c) 2006 Amcoe - Devil's Island */
	DRIVER( atworld )	/* (c) 2007 Amcoe - Around The World */
	DRIVER( atworldd1 )	/* (c) 2007 Amcoe */
	DRIVER( amclink )	/* (c) 200? Amcoe - Amcoe Link Control Box? */

	DRIVER( act2000 )	/* (c) 1999 Amcoe - Action 2000 */
	DRIVER( act2000v1 )	/* (c) 1999 Amcoe */
	DRIVER( act2000d1 )	/* (c) 1999 Amcoe */
	DRIVER( act2000b1 )	/* (c) 1999 Amcoe */
	DRIVER( act2000vx )	/* (c) 1999 Amcoe */
	DRIVER( act2000dx )	/* (c) 1999 Amcoe */
	DRIVER( act2000bx )	/* (c) 1999 Amcoe */
	DRIVER( act2000o )	/* (c) 1999 Amcoe */
	DRIVER( act2000o2 )	/* (c) 1999 Amcoe */
	DRIVER( act2000o3 )	/* (c) 1999 Amcoe */
	DRIVER( ch2000 )	/* (c) 2000 Amcoe - Fruit Bonus 2000 / New Cherry 2000 */
	DRIVER( ch2000b1 )	/* (c) 2000 Amcoe */
	DRIVER( ch2000c1 )	/* (c) 2000 Amcoe */
	DRIVER( ch2000d1 )	/* (c) 2000 Amcoe */
	DRIVER( ch2000v1 )	/* (c) 2000 Amcoe */
	DRIVER( ch2000b2 )	/* (c) 2000 Amcoe */
	DRIVER( ch2000c2 )	/* (c) 2000 Amcoe */
	DRIVER( ch2000d2 )	/* (c) 2000 Amcoe */
	DRIVER( ch2000v2 )	/* (c) 2000 Amcoe */
	DRIVER( ch2000o )	/* (c) 2000 Amcoe */
	DRIVER( ch2000o2 )	/* (c) 2000 Amcoe */
	DRIVER( ch2000o3 )	/* (c) 2000 Amcoe */
	DRIVER( pir2001 )	/* (c) 2001 Amcoe - Pirate 2001 */
	DRIVER( pir2001b1 )	/* (c) 2001 Amcoe */
	DRIVER( pir2001d1 )	/* (c) 2001 Amcoe */
	DRIVER( pir2001v1 )	/* (c) 2001 Amcoe */
	DRIVER( pir2001bx )	/* (c) 2001 Amcoe */
	DRIVER( pir2001dx )	/* (c) 2001 Amcoe */
	DRIVER( pir2001vx )	/* (c) 2001 Amcoe */
	DRIVER( pir2001o )	/* (c) 2001 Amcoe */
	DRIVER( pir2001o2 )	/* (c) 2001 Amcoe */
	DRIVER( pir2001o3 )	/* (c) 2001 Amcoe */
	DRIVER( pir2002 )	/* (c) 2001 Amcoe - Pirate 2002 */
	DRIVER( pir2002b1 )	/* (c) 2001 Amcoe */
	DRIVER( pir2002d1 )	/* (c) 2001 Amcoe */
	DRIVER( pir2002v1 )	/* (c) 2001 Amcoe */
	DRIVER( pir2002bx )	/* (c) 2001 Amcoe */
	DRIVER( pir2002dx )	/* (c) 2001 Amcoe */
	DRIVER( pir2002vx )	/* (c) 2001 Amcoe */
	DRIVER( pir2002o )	/* (c) 2001 Amcoe */
	DRIVER( pir2002o2 )	/* (c) 2001 Amcoe */
	DRIVER( pir2002o3 )	/* (c) 2001 Amcoe */
	DRIVER( classice )	/* (c) 2004 Amcoe - Classic Edition */
	DRIVER( classicev )	/* (c) 2004 Amcoe */
	DRIVER( classice1 )	/* (c) 2004 Amcoe */
	DRIVER( classiced1 )/* (c) 2004 Amcoe */
	DRIVER( classicev1 )/* (c) 2004 Amcoe */
	DRIVER( classice2 )	/* (c) 2004 Amcoe */
	DRIVER( classiced2 )/* (c) 2004 Amcoe */
	DRIVER( classicev2 )/* (c) 2004 Amcoe */
	DRIVER( seawld )	/* (c) 200? Amcoe - Sea World */
	DRIVER( seawldd1 )	/* (c) 200? Amcoe */
	DRIVER( moneymac )	/* (c) 200? Amcoe - Money Machine */
	DRIVER( moneymacd1 )/* (c) 200? Amcoe */
	DRIVER( moneymacv1 )/* (c) 200? Amcoe */
	DRIVER( moneymacd2 )/* (c) 200? Amcoe */
	DRIVER( moneymacv2 )/* (c) 200? Amcoe */
	//DRIVER( atworld ) /* (c) 2007 Amcoe - Around The World (year listed on Amcoe web site) */
	//DRIVER( atworldd1 )   /* (c) 2007 Amcoe (year listed on Amcoe web site) */


	DRIVER( chsuper3 )	/* unknown */
	DRIVER( chsuper2 )	/* unknown */
	DRIVER( chmpnum )	/* unknown */
	DRIVER( 3super8 )	/* unknown */
	DRIVER( buster )	/* (c) 1987 Marian Electronics Ltd. */
	DRIVER( spielbud )	/* (c) 1985 ADP */

	/* Hi-Soft */
	DRIVER( himesiki )	/* (c) 1989 Hi-Soft */

	/* Draw Poker HI-LO based (z80) */
	DRIVER( norautp )	/* (c) 1988 Noraut Ltd. */
	DRIVER( norautdx )	/* (c) 198? Noraut Ltd. */
	DRIVER( norautpn )	/* (c) 199? unknown */
	DRIVER( norautjo )	/* (c) 198? Noraut Ltd. */
	DRIVER( norautpl )	/* (c) 198? Prologic / Noraut Ltd. */
	DRIVER( norautjp )	/* (c) 198? Noraut Ltd. */
	DRIVER( norautrh )	/* (c) 198? Noraut Ltd. */
	DRIVER( norautra )	/* (c) 198? Noraut Ltd. */
	DRIVER( norautu )	/* (c) 198? Noraut Ltd. */
	DRIVER( noraut3a )	/* (c) 2002 Noraut Ltd. */
	DRIVER( noraut3b )	/* (c) 2003 Noraut Ltd. */
	DRIVER( norautua )	/* (c) 198? Noraut Ltd. */
	DRIVER( norautub )	/* (c) 198? Noraut Ltd. */
	DRIVER( mainline )	/* (c) Mainline London */
	DRIVER( df_djpkr )	/* (c) DellFern Ltd. */
	DRIVER( ndxron10 )	/* 2005, unknown */
	DRIVER( cgip30cs )	/* (c) 1999, CGI */
	DRIVER( kimblz80 )	/* (c) 198?, Kimble Ireland */
	DRIVER( pma )		/* 1983, PMA */
	DRIVER( bjpoker )	/* (c) M. Kramer Inc. */

	/* Draw Poker HI-LO based (8080) */
	DRIVER( dphl )		/* (c) 1983 M. Kramer Inc. */
	DRIVER( dphla )		/* unknown */
	DRIVER( dphljp )	/* unknown */
	DRIVER( kimbldhl )	/* (c) 198?, Kimble Ireland */
	DRIVER( gtipoker )	/* (c) 1983 GTI Inc */
	DRIVER( smshilo )	/* (c) 1983 SMS Manufacturing Corp. */
	DRIVER( drhl )		/* (c) Drew, Inc. */
	DRIVER( tpoker2 )	/* (c) 1993 Micro Manufacturing, Inc. */
	DRIVER( ssjkrpkr )	/* (c) 1982 Southern Systems & Assembly, Ltd. */
	DRIVER( fastdrwp )	/* Stern? */
	DRIVER( dphlunka )	/* SMS Manufacturing Corp? */
	DRIVER( dphlunkb )	/* SMS Manufacturing Corp? */


	/* Sanki Denshi Kogyo */
	DRIVER( pachifev )	/* (c) 1983? */

	/* New Image Technologies */
	DRIVER( bonanza )	/* (c) 1993 */
	DRIVER( bonanzar2 )	/* (c) 1993 */
	DRIVER( streetg )	/* (c) 1993 */
	DRIVER( streetgr3 )	/* (c) 1993 */
	DRIVER( streetg2 )	/* (c) 1993 */
	DRIVER( streetg2r5 )	/* (c) 1993 */

	/* Coinmaster-Gaming VGA based */
	DRIVER( colorama )	/* (c) 2001 Coinmaster-Gaming, Ltd. */
	DRIVER( cmrltv75 )	/* (c) 2001 Coinmaster-Gaming, Ltd. */
	DRIVER( cmkenosp )	/* (c) 2000 Coinmaster-Gaming, Ltd. */
	DRIVER( cmkenospa )	/* (c) 2000 Coinmaster-Gaming, Ltd. */

	/* Igrosoft */
	/* Multifish */
	//DRIVER( mfish )   /* (c) 2002 */
	//DRIVER( mfish_2 ) /* (c) 2002 */
	DRIVER( mfish_3 )	/* (c) 2002 */
	DRIVER( mfish_3a )	/* (c) 2002 */
	//DRIVER( mfish_4 ) /* (c) 2002 */
	//DRIVER( mfish_5 ) /* (c) 2002 */
	DRIVER( mfish_6 ) /* (c) 2002 */
	//DRIVER( mfish_7 ) /* (c) 2002 */
	//DRIVER( mfish_8 ) /* (c) 2002 */
	//DRIVER( mfish_9 ) /* (c) 2002 */
	//DRIVER( mfish_10 )    /* (c) 2002 */
	DRIVER( mfish_11 )	/* (c) 2002 */
	DRIVER( mfish_12 )	/* (c) 2002 */
	DRIVER( mfish_12a )	/* (c) 2002 */
	//DRIVER( mfish_13 )    /* (c) 2002 */

	/* Crazy Monkey */
	//DRIVER( crzmon )  /* (c) 2003 */
	//DRIVER( crzmon_2 )    /* (c) 2003 */
	//DRIVER( crzmon_3 )    /* (c) 2003 */
	//DRIVER( crzmon_4 )    /* (c) 2003 */
	//DRIVER( crzmon_5 )    /* (c) 2003 */
	//DRIVER( crzmon_6 )
	DRIVER( crzmon_7 )	/* (c) 2003 */
	DRIVER( crzmon_7a )	/* (c) 2003 */
	DRIVER( crzmon_7b )	/* (c) 2003 */
	DRIVER( crzmon_8 )	/* (c) 2003 */
	DRIVER( crzmon_8a )	/* (c) 2003 */

	/* Fruit Cocktail */
	//DRIVER( fcockt )  /* (c) 2003 */
	//DRIVER( fcockt_2 )    /* (c) 2003 */
	//DRIVER( fcockt_3 )    /* (c) 2003 */
	//DRIVER( fcockt_4 )    /* (c) 2003 */
	//DRIVER( fcockt_5 )    /* (c) 2003 */
	DRIVER( fcockt_6 )	/* (c) 2003 */
	DRIVER( fcockt_6a )	/* (c) 2003 */
	DRIVER( fcockt_6b )	/* (c) 2003 */
	DRIVER( fcockt_7 )	/* (c) 2003 */
	DRIVER( fcockt_7a )	/* (c) 2003 */
	DRIVER( fcockt_8 )	/* (c) 2003 */

	/* Lucky Haunter */
	//DRIVER( lhaunt )  /* (c) 2003 */
	//DRIVER( lhaunt_2 )    /* (c) 2003 */
	//DRIVER( lhaunt_3 )    /* (c) 2003 */
	DRIVER( lhaunt_4 )	/* (c) 2003 */
	DRIVER( lhaunt_4a )	/* (c) 2003 */
	DRIVER( lhaunt_5 )	/* (c) 2003 */
	DRIVER( lhaunt_5a )	/* (c) 2003 */
	DRIVER( lhaunt_6 )	/* (c) 2003 */
	DRIVER( lhaunt_6a )	/* (c) 2003 */

	/* Rollfruit */
	//DRIVER( rollfr )  /* (c) 2003 */
	DRIVER( rollfr_2 )	/* (c) 2003 */

	/* Garage */
	//DRIVER( garage )  /* (c) 2004 */
	//DRIVER( garage_2 )    /* (c) 2004 */
	//DRIVER( garage_3 )    /* (c) 2004 */
	DRIVER( garage_4 )	/* (c) 2004 */
	DRIVER( garage_4a )	/* (c) 2004 */
	DRIVER( garage_4b )	/* (c) 2004 */
	DRIVER( garage_5 )	/* (c) 2004 */
	DRIVER( garage_5a )	/* (c) 2004 */

	/* Rock Climber */
	//DRIVER( rclimb )  /* (c) 2004 */
	//DRIVER( rclimb_2 )    /* (c) 2004 */
	DRIVER( rclimb_3 )	/* (c) 2004 */
	DRIVER( rclimb_3a )	/* (c) 2004 */
	DRIVER( rclimb_3b )	/* (c) 2004 */

	/* Sweet Life */
	DRIVER( sweetl )	/* (c) 2004 */
	DRIVER( sweetla )	/* (c) 2004 */
	DRIVER( sweetlb )	/* (c) 2004 */

	/* Resident */
	//DRIVER( resdnt )  /* (c) 2004 */
	DRIVER( resdnt_2 )	/* (c) 2004 */
	DRIVER( resdnt_2a )	/* (c) 2004 */

	/* Island */
	DRIVER( island )	/* (c) 2005 */
	DRIVER( islanda )	/* (c) 2005 */

	/* Pirate */
	//DRIVER( pirate )  /* (c) 2005 */
	DRIVER( pirate_2 )	/* (c) 2005 */
	DRIVER( pirate_3 )	/* (c) 2005 */

	/* Pirate 2 */
	DRIVER( pirate2 )	/* (c) 2006 */
	DRIVER( pirate2a )	/* (c) 2006 */
	DRIVER( pirate2b )	/* (c) 2006 */
	DRIVER( pirate2c )	/* (c) 2006 */
	DRIVER( pirate2d )	/* (c) 2006 */
	DRIVER( pirate2e )	/* (c) 2006 */

	/* Island 2 */
	DRIVER( island2 )	/* (c) 2006 */
	DRIVER( island2a )	/* (c) 2006 */

	/* Keks */
	DRIVER( keks )		/* (c) 2006 */
	DRIVER( keksa )		/* (c) 2006 */
	DRIVER( keksb )		/* (c) 2006 */
	DRIVER( keksc )		/* (c) 2006 */
	DRIVER( keks_2 )	/* (c) 2006 */
	DRIVER( keks_2a )	/* (c) 2006 */
	DRIVER( keks_2b )	/* (c) 2006 */

	/* Disney */
	DRIVER( cdi )		/* Base unit */
	DRIVER( quizard )	/* (c) Disney 1996 */
	DRIVER( quizrd22 )	/* (c) Disney 1995 */
	DRIVER( quizrd17 )	/* (c) Disney 1996 */
	DRIVER( quizrd12 )	/* (c) Disney 1996 */
	DRIVER( quizrr42 )	/* (c) Disney 1998 */
	DRIVER( quizrr41 )	/* (c) Disney 1998 */

/* Drivers below are mechanical games, usually with no video display, requiring external artwork to function.
       These are currently mostly 'fruit machine' style games, although this may later be extended to system logic
       emulation for pinball machiens.

       the m_ prefix is used to identify them as many have very generic names.

    */

	/* Scorpion 2 */
	DRIVER( m_brkfst )
	DRIVER( m_brkfs1 )
	DRIVER( m_brkfs2 )
	DRIVER( m_brkfs3 )
	DRIVER( m_brkfs4 )
	DRIVER( m_brkfs5 )

	DRIVER( m_bdrwho )
	DRIVER( m_bdrwh1 )
	DRIVER( m_bdrwh2 )
	DRIVER( m_bdrwh3 )
	DRIVER( m_bdrwh4 )
	DRIVER( m_bdrwh5 )
	DRIVER( m_bdrwh6 )
	DRIVER( m_bdrwh7 )
	DRIVER( m_bdrwh8 )
	DRIVER( m_bdrwh9 )
	DRIVER( m_bdrw10 )
	DRIVER( m_bdrw11 )
	DRIVER( m_bdrw12 )
	DRIVER( m_bdrw13 )
	DRIVER( m_bdrw14 )
	DRIVER( m_bdrw15 )
	DRIVER( m_bdrw16 )
	DRIVER( m_bdrw17 )

	DRIVER( m_bfocus )
	DRIVER( m_bcgslm )
	DRIVER( m_luvjub )
	DRIVER( m_cpeno1 )

	/* System85 games */
	DRIVER( m_supcrd )	/* (c) 198? BFM */

	/* JPM Impact games */
	DRIVER( m_tbirds )

	/* Scorpion1 games */
	DRIVER( m_lotsse )	/* (c) 198? BFM Dutch ROMS, prelim*/
	DRIVER( m_roulet )	/* (c) 198? BFM Dutch ROMS, prelim*/
	DRIVER( m_clattr )	/* (c) 1990 BFM,     Game Card 39-370-196*/
	DRIVER( m_tppokr )	/* (c) 1996 BFM/ELAM,Game Card 95-750-899, uses Adder board for feature gfx*/

	/* Maygay Machines Ltd. */
	DRIVER( m_sptlgt )

	DRIVER( screenpl )
	DRIVER( screenp1 )
	DRIVER( screenp2 )

	/* MPU4 */
	DRIVER( m_oldtmr )	/* Barcrest - highly prelim*/
	DRIVER( m_ccelbr )
	DRIVER( m_gmball )

	/* MPU5 */
	DRIVER( m_honmon )

	/* Leisure Ent. */
	DRIVER( roul )		/* (c) 1990 */

	/* Accept LTD. */
	DRIVER( hitpoker )	/* (c) 1997 */

	/* Videotronics */
	DRIVER( vpoker )	/* (c) 198? */

	/* Play Mechanix */
	DRIVER( jnero )		/* (c) 2004 */

	/* Amatic Trading GMBH */
	DRIVER( am_uslot )	/* (c) Amatic 1996 */
	DRIVER( am_mg24 )	/* (c) Amatic 2000 */
	DRIVER( am_mg3 )	/* (c) Amatic 2000 */

	DRIVER( neptunp2 )
	DRIVER( rgum )

	/* Extrema Systems International Ltd. */
	DRIVER( poker72 )

	DRIVER( chkun )

	/* Blitz System Inc. */
	DRIVER( megadpkr )	/* (c) 1990 Blitz System Inc */

	/* Video Fun Games Ltd. */
	DRIVER( genie )		/* (c) 198? Video Fun Games Ltd. */

	/* Olympic Video Gaming */
	DRIVER( hotstuff )

	/* Interactive Light */
	DRIVER( savquest )

#endif	/* DRIVER_RECURSIVE */
