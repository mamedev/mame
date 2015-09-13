// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII memory mapped i/o stuff (HW)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2HW_H_
#define _A2HW_H_
//! miscellaneous hardware registers in the memory mapped I/O range
struct {
	UINT16 eia;             //!< the EIA port at 0177001
	UINT16 utilout;         //!< the UTILOUT port at 0177016 (active-low outputs)
	UINT16 xbus[4];         //!< the XBUS port at 0177020 to 0177023
	UINT16 utilin;          //!< the UTILIN port at 0177030 to 0177033 (same value on all addresses)
}   m_hw;

DECLARE_READ16_MEMBER ( pprdy_r );          //!< read UTILIN[0] printer paper ready bit
DECLARE_READ16_MEMBER ( pcheck_r );         //!< read UTILIN[1] printer check bit
DECLARE_READ16_MEMBER ( unused2_r );        //!< read UTILIN[2] unused bit
DECLARE_READ16_MEMBER ( pchrdy_r );         //!< read UTILIN[3] printer daisy ready bit
DECLARE_READ16_MEMBER ( parrdy_r );         //!< read UTILIN[4] printer carriage ready bit
DECLARE_READ16_MEMBER ( pready_r );         //!< read UTILIN[5] printer ready bit
DECLARE_READ16_MEMBER ( memconfig_r );      //!< read UTILIN[6] memory config switch
DECLARE_READ16_MEMBER ( unused7_r );        //!< read UTILIN[7] unused bit
DECLARE_READ16_MEMBER ( keyset_key0_r );    //!< read UTILIN[8] keyset key #0
DECLARE_READ16_MEMBER ( keyset_key1_r );    //!< read UTILIN[9] keyset key #1
DECLARE_READ16_MEMBER ( keyset_key2_r );    //!< read UTILIN[10] keyset key #2
DECLARE_READ16_MEMBER ( keyset_key3_r );    //!< read UTILIN[11] keyset key #3
DECLARE_READ16_MEMBER ( keyset_key4_r );    //!< read UTILIN[12] keyset key #4
DECLARE_READ16_MEMBER ( mouse_red_r );      //!< read UTILIN[13] mouse red button bit
DECLARE_READ16_MEMBER ( mouse_blue_r );     //!< read UTILIN[14] mouse blue button bit
DECLARE_READ16_MEMBER ( mouse_yellow_r );   //!< read UTILIN[15] mouse yellow button bit

DECLARE_WRITE16_MEMBER( pprdy_w );          //!< write UTILIN[0] printer paper ready bit
DECLARE_WRITE16_MEMBER( pcheck_w );         //!< write UTILIN[1] printer check bit
DECLARE_WRITE16_MEMBER( unused2_w );        //!< write UTILIN[2] unused bit
DECLARE_WRITE16_MEMBER( pchrdy_w );         //!< write UTILIN[3] printer daisy ready bit
DECLARE_WRITE16_MEMBER( parrdy_w );         //!< write UTILIN[4] carriage ready bit
DECLARE_WRITE16_MEMBER( pready_w );         //!< write UTILIN[5] printer ready bit
DECLARE_WRITE16_MEMBER( memconfig_w );      //!< write UTILIN[6] memory config switch
DECLARE_WRITE16_MEMBER( unused7_w );        //!< write UTILIN[7] unused bit
DECLARE_WRITE16_MEMBER( keyset_key0_w );    //!< write UTILIN[8] keyset key #0
DECLARE_WRITE16_MEMBER( keyset_key1_w );    //!< write UTILIN[9] keyset key #1
DECLARE_WRITE16_MEMBER( keyset_key2_w );    //!< write UTILIN[10] keyset key #2
DECLARE_WRITE16_MEMBER( keyset_key3_w );    //!< write UTILIN[11] keyset key #3
DECLARE_WRITE16_MEMBER( keyset_key4_w );    //!< write UTILIN[12] keyset key #4
DECLARE_WRITE16_MEMBER( mouse_red_w );      //!< write UTILIN[13] mouse red button bit
DECLARE_WRITE16_MEMBER( mouse_blue_w );     //!< write UTILIN[14] mouse blue button bit
DECLARE_WRITE16_MEMBER( mouse_yellow_w );   //!< write UTILIN[15] mouse yellow button bit
DECLARE_WRITE16_MEMBER( mouse_buttons_w );  //!< write UTILIN[13-15] mouse buttons bits

DECLARE_READ16_MEMBER ( utilin_r );         //!< read an UTILIN address
DECLARE_READ16_MEMBER ( utilout_r );        //!< read the UTILOUT address
DECLARE_WRITE16_MEMBER( utilout_w );        //!< write the UTILOUT address
DECLARE_READ16_MEMBER ( xbus_r );           //!< read an XBUS address
DECLARE_WRITE16_MEMBER( xbus_w );           //!< write an XBUS address (?)

void init_hw();                             //!< initialize miscellaneous hardware
void exit_hw();                             //!< deinitialize miscellaneous hardware
void reset_hw();                            //!< reset miscellaneous hardware
#endif  // _A2HW_H_
#endif  // ALTO2_DEFINE_CONSTANTS
