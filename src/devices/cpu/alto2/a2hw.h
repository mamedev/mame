// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII memory mapped i/o stuff (HW)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef MAME_CPU_ALTO2_A2HW_H
#define MAME_CPU_ALTO2_A2HW_H
//! miscellaneous hardware registers in the memory mapped I/O range
struct {
	uint16_t eia;             //!< the EIA port at 0177001
	uint16_t utilout;         //!< the UTILOUT port at 0177016 (active-low outputs)
	uint16_t xbus[4];         //!< the XBUS port at 0177020 to 0177023
	uint16_t utilin;          //!< the UTILIN port at 0177030 to 0177033 (same value on all addresses)
}   m_hw;

uint16_t pprdy_r();                           //!< read UTILIN[0] printer paper ready bit
uint16_t pcheck_r();                          //!< read UTILIN[1] printer check bit
uint16_t unused2_r();                         //!< read UTILIN[2] unused bit
uint16_t pchrdy_r();                          //!< read UTILIN[3] printer daisy ready bit
uint16_t parrdy_r();                          //!< read UTILIN[4] printer carriage ready bit
uint16_t pready_r();                          //!< read UTILIN[5] printer ready bit
uint16_t memconfig_r();                       //!< read UTILIN[6] memory config switch
uint16_t unused7_r();                         //!< read UTILIN[7] unused bit
uint16_t keyset_key0_r();                     //!< read UTILIN[8] keyset key #0
uint16_t keyset_key1_r();                     //!< read UTILIN[9] keyset key #1
uint16_t keyset_key2_r();                     //!< read UTILIN[10] keyset key #2
uint16_t keyset_key3_r();                     //!< read UTILIN[11] keyset key #3
uint16_t keyset_key4_r();                     //!< read UTILIN[12] keyset key #4
uint16_t mouse_red_r();                       //!< read UTILIN[13] mouse red button bit
uint16_t mouse_blue_r();                      //!< read UTILIN[14] mouse blue button bit
uint16_t mouse_yellow_r();                    //!< read UTILIN[15] mouse yellow button bit

void pprdy_w(uint16_t data);                  //!< write UTILIN[0] printer paper ready bit
void pcheck_w(uint16_t data);                 //!< write UTILIN[1] printer check bit
void unused2_w(uint16_t data);                //!< write UTILIN[2] unused bit
void pchrdy_w(uint16_t data);                 //!< write UTILIN[3] printer daisy ready bit
void parrdy_w(uint16_t data);                 //!< write UTILIN[4] carriage ready bit
void pready_w(uint16_t data);                 //!< write UTILIN[5] printer ready bit
void memconfig_w(uint16_t data);              //!< write UTILIN[6] memory config switch
void unused7_w(uint16_t data);                //!< write UTILIN[7] unused bit
void keyset_key0_w(uint16_t data);            //!< write UTILIN[8] keyset key #0
void keyset_key1_w(uint16_t data);            //!< write UTILIN[9] keyset key #1
void keyset_key2_w(uint16_t data);            //!< write UTILIN[10] keyset key #2
void keyset_key3_w(uint16_t data);            //!< write UTILIN[11] keyset key #3
void keyset_key4_w(uint16_t data);            //!< write UTILIN[12] keyset key #4
void mouse_red_w(uint16_t data);              //!< write UTILIN[13] mouse red button bit
void mouse_blue_w(uint16_t data);             //!< write UTILIN[14] mouse blue button bit
void mouse_yellow_w(uint16_t data);           //!< write UTILIN[15] mouse yellow button bit
void mouse_buttons_w(uint16_t data);          //!< write UTILIN[13-15] mouse buttons bits

uint16_t utilin_r(offs_t offset);             //!< read an UTILIN address
uint16_t utilout_r(offs_t offset);            //!< read the UTILOUT address
void utilout_w(offs_t offset, uint16_t data); //!< write the UTILOUT address
uint16_t xbus_r(offs_t offset);               //!< read an XBUS address
void xbus_w(offs_t offset, uint16_t data);    //!< write an XBUS address (?)

void init_hw();                               //!< initialize miscellaneous hardware
void exit_hw();                               //!< deinitialize miscellaneous hardware
void reset_hw();                              //!< reset miscellaneous hardware
#endif  // MAME_CPU_ALTO2_A2HW_H
#endif  // ALTO2_DEFINE_CONSTANTS
