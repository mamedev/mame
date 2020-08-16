/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2010 by GSport contributors
 
 Based on the KEGS emulator written by and Copyright (C) 2003 Kent Dickey

 This program is free software; you can redistribute it and/or modify it 
 under the terms of the GNU General Public License as published by the 
 Free Software Foundation; either version 2 of the License, or (at your 
 option) any later version.

 This program is distributed in the hope that it will be useful, but 
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 for more details.

 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*tfe.c*/
#ifndef _PROTOS_TFE_H
#define _PROTOS_TFE_H

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int tfe_enabled;

void tfe_init(void);
int tfe_resources_init(void);
int tfe_cmdline_options_init(void);
int set_tfe_interface(const char* name);
void get_disabled_state(int * param);

void tfe_reset(void);
void tfe_shutdown(void);
byte tfe_read(word16 addr);
void tfe_store(word16 addr, byte var);
/* Unused in this version of TFE, and generates complaints
int tfe_read_snapshot_module(struct snapshot_s *s);
int tfe_write_snapshot_module(struct snapshot_s *s);
*/
int tfe_enumadapter_open(void);
int tfe_enumadapter(char **ppname, char **ppdescription);
int tfe_enumadapter_close(void);

/*tfearch.c*/
int  tfe_arch_init(void);
void tfe_arch_pre_reset(void);
void tfe_arch_post_reset(void);
int  tfe_arch_activate(const char *interface_name);
void tfe_arch_deactivate(void);
void tfe_arch_set_mac(const byte mac[6]);
void tfe_arch_set_hashfilter(const int hash_mask[2]);

void tfe_arch_recv_ctl( int bBroadcast,   /* broadcast */
                        int bIA,          /* individual address (IA) */
                        int bMulticast,   /* multicast if address passes the hash filter */
                        int bCorrect,     /* accept correct frames */
                        int bPromiscuous, /* promiscuous mode */
                        int bIAHash       /* accept if IA passes the hash filter */
                      );

void tfe_arch_line_ctl(int bEnableTransmitter, int bEnableReceiver);

void tfe_arch_transmit(int force,       /* FORCE: Delete waiting frames in transmit buffer */
                       int onecoll,     /* ONECOLL: Terminate after just one collision */
                       int inhibit_crc, /* INHIBITCRC: Do not append CRC to the transmission */
                       int tx_pad_dis,  /* TXPADDIS: Disable padding to 60 Bytes */
                       int txlength,    /* Frame length */
                       byte *txframe    /* Pointer to the frame to be transmitted */
                      );

int tfe_arch_receive(byte *pbuffer  ,    /* where to store a frame */
                     int  *plen,         /* IN: maximum length of frame to copy; 
                                            OUT: length of received frame 
                                            OUT can be bigger than IN if received frame was
                                                longer than supplied buffer */
                     int  *phashed,      /* set if the dest. address is accepted by the hash filter */
                     int  *phash_index,  /* hash table index if hashed == TRUE */   
                     int  *prx_ok,       /* set if good CRC and valid length */
                     int  *pcorrect_mac, /* set if dest. address is exactly our IA */
                     int  *pbroadcast,   /* set if dest. address is a broadcast address */
                     int  *pcrc_error    /* set if received frame had a CRC error */
                     );

/*
 This is a helper for tfe_receive() to determine if the received frame should be accepted
 according to the settings.

 This function is even allowed to be called in tfearch.c from tfe_arch_receive() if 
 necessary, which is the reason why its prototype is included here in tfearch.h.
*/
int tfe_should_accept(unsigned char *buffer, int length, int *phashed, int *phash_index, 
                      int *pcorrect_mac, int *pbroadcast, int *pmulticast);

int tfe_arch_enumadapter_open(void);
int tfe_arch_enumadapter(char **ppname, char **ppdescription);
int tfe_arch_enumadapter_close(void);

#ifdef __cplusplus
}
#endif

#endif
