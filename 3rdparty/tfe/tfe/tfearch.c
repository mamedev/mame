/*
 * tfearch.c - TFE ("The final ethernet") emulation, 
 *                 architecture-dependant stuff
 *
 * Written by
 *  Spiro Trikaliotis <Spiro.Trikaliotis@gmx.de>
 * 
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */
 

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../atbridge/pcap_delay.h"
#include "tfesupp.h" 
#include "../defc.h"
#include "protos_tfe.h"

/** #define TFE_DEBUG_ARCH 1 **/
/** #define TFE_DEBUG_PKTDUMP 1 **/

/* #define TFE_DEBUG_FRAMES - might be defined in TFE.H! */

#define TFE_DEBUG_WARN 1 /* this should not be deactivated */


/* ------------------------------------------------------------------------- */
/*    variables needed                                                       */


//static log_t tfe_arch_log = LOG_ERR;

static pcap_if_t *TfePcapNextDev = NULL;
static pcap_if_t *TfePcapAlldevs = NULL;
static pcap_t *TfePcapFP = NULL;

static char TfePcapErrbuf[PCAP_ERRBUF_SIZE];

#ifdef TFE_DEBUG_PKTDUMP

static
void debug_output( const char *text, unsigned char *what, int count )
{
    char buffer[256];
    char *p = buffer;
    char *pbuffer1 = what;
    int len1 = count;
    int i;

    sprintf(buffer, "\n%s: length = %u\n", text, len1);
    OutputDebugString(buffer);
    do {
        p = buffer;
        for (i=0; (i<8) && len1>0; len1--, i++) {
            sprintf( p, "%02x ", (unsigned int)(unsigned char)*pbuffer1++);
            p += 3;
        }
        *(p-1) = '\n'; *p = 0;
        OutputDebugString(buffer);
    } while (len1>0);
}
#endif // #ifdef TFE_DEBUG_PKTDUMP


/*
 These functions let the UI enumerate the available interfaces.

 First, TfeEnumAdapterOpen() is used to start enumeration.

 TfeEnumAdapter is then used to gather information for each adapter present
 on the system, where:

   ppname points to a pointer which will hold the name of the interface
   ppdescription points to a pointer which will hold the description of the interface

   For each of these parameters, new memory is allocated, so it has to be
   freed with lib_free().

 TfeEnumAdapterClose() must be used to stop processing.

 Each function returns 1 on success, and 0 on failure.
 TfeEnumAdapter() only fails if there is no more adpater; in this case, 
   *ppname and *ppdescription are not altered.
*/
int tfe_arch_enumadapter_open(void)
{
    if (pcapdelay_findalldevs(&TfePcapAlldevs, TfePcapErrbuf) == -1)
    {
#ifdef TFE_DEBUG_ARCH
        log_message(tfe_arch_log, "ERROR in TfeEnumAdapterOpen: pcap_findalldevs: '%s'", TfePcapErrbuf);
#endif
        return 0;
    }

	if (!TfePcapAlldevs) {
#ifdef TFE_DEBUG_ARCH
        log_message(tfe_arch_log, "ERROR in TfeEnumAdapterOpen, finding all pcap devices - "
			"Do we have the necessary privilege rights?");
#endif
		return 0;
	}

    TfePcapNextDev = TfePcapAlldevs;

    return 1;
}

int tfe_arch_enumadapter(char **ppname, char **ppdescription)
{
    if (!TfePcapNextDev || (TfePcapNextDev->name == NULL))
        return 0;

    *ppname = lib_stralloc(TfePcapNextDev->name);
    if (TfePcapNextDev->description)
	*ppdescription = lib_stralloc(TfePcapNextDev->description);
    else
	*ppdescription = lib_stralloc(TfePcapNextDev->name);
    TfePcapNextDev = TfePcapNextDev->next;

    return 1;
}

int tfe_arch_enumadapter_close(void)
{
    if (TfePcapAlldevs) {
        pcapdelay_freealldevs(TfePcapAlldevs);
        TfePcapAlldevs = NULL;
    }
    return 1;
}

static
int TfePcapOpenAdapter(const char *interface_name) 
{
    pcap_if_t *TfePcapDevice = NULL;

    if (!tfe_enumadapter_open()) {
        return FALSE;
    }
    else {
        /* look if we can find the specified adapter */
        char *pname;
        char *pdescription;
        int  found = FALSE;

        if (interface_name) {
            /* we have an interface name, try it */
            TfePcapDevice = TfePcapAlldevs;

            while (tfe_enumadapter(&pname, &pdescription)) {
                if (strcmp(pname, interface_name)==0) {
                    found = TRUE;
                }
                lib_free(pname);
                lib_free(pdescription);
                if (found) break;
                TfePcapDevice = TfePcapNextDev;
            }
        }

        if (!found) {
            /* just take the first adapter */
            TfePcapDevice = TfePcapAlldevs;
        }
    }

    TfePcapFP = pcapdelay_open_live(TfePcapDevice->name, 1700, 1, 20, TfePcapErrbuf);
    if ( TfePcapFP == NULL)
    {
#ifdef TFE_DEBUG_ARCH
        log_message(tfe_arch_log, "ERROR opening adapter: '%s'", TfePcapErrbuf);
#endif
        tfe_enumadapter_close();
        return FALSE;
    }

    if (pcapdelay_setnonblock(TfePcapFP, 1, TfePcapErrbuf)<0)
    {
#ifdef TFE_DEBUG_ARCH
        log_message(tfe_arch_log, "WARNING: Setting PCAP to non-blocking failed: '%s'", TfePcapErrbuf);
#endif
    }

	/* Check the link layer. We support only Ethernet for simplicity. */
	if(pcapdelay_datalink(TfePcapFP) != DLT_EN10MB)
	{
#ifdef TFE_DEBUG_ARCH
		log_message(tfe_arch_log, "ERROR: TFE works only on Ethernet networks.");
#endif
		tfe_enumadapter_close();
        return FALSE;
	}
	
    tfe_enumadapter_close();
    return TRUE;
}


/* ------------------------------------------------------------------------- */
/*    the architecture-dependend functions                                   */


int tfe_arch_init(void)
{
    //tfe_arch_log = log_open("TFEARCH");

    return 1;
}

void tfe_arch_pre_reset( void )
{
#ifdef TFE_DEBUG_ARCH
    log_message( tfe_arch_log, "tfe_arch_pre_reset()." );
#endif
}

void tfe_arch_post_reset( void )
{
#ifdef TFE_DEBUG_ARCH
    log_message( tfe_arch_log, "tfe_arch_post_reset()." );
#endif
}

int tfe_arch_activate(const char *interface_name)
{
#ifdef TFE_DEBUG_ARCH
    log_message( tfe_arch_log, "tfe_arch_activate()." );
#endif
    if (!TfePcapOpenAdapter(interface_name)) {
        return 0;
    }
    return 1;
}

void tfe_arch_deactivate( void )
{
#ifdef TFE_DEBUG_ARCH
    log_message( tfe_arch_log, "tfe_arch_deactivate()." );
#endif
}

void tfe_arch_set_mac( const unsigned char mac[6] )
{
#if defined(TFE_DEBUG_ARCH) || defined(TFE_DEBUG_FRAMES)
    log_message( tfe_arch_log, "New MAC address set: %02X:%02X:%02X:%02X:%02X:%02X.",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
#endif
}


void tfe_arch_recv_ctl( int bBroadcast,   /* broadcast */
                        int bIA,          /* individual address (IA) */
                        int bMulticast,   /* multicast if address passes the hash filter */
                        int bCorrect,     /* accept correct frames */
                        int bPromiscuous, /* promiscuous mode */
                        int bIAHash       /* accept if IA passes the hash filter */
                      )
{
#if defined(TFE_DEBUG_ARCH) || defined(TFE_DEBUG_FRAMES)
    log_message( tfe_arch_log, "tfe_arch_recv_ctl() called with the following parameters:" );
    log_message( tfe_arch_log, "\tbBroadcast   = %s", bBroadcast   ? "TRUE" : "FALSE" );
    log_message( tfe_arch_log, "\tbIA          = %s", bIA          ? "TRUE" : "FALSE" );
    log_message( tfe_arch_log, "\tbMulticast   = %s", bMulticast   ? "TRUE" : "FALSE" );
    log_message( tfe_arch_log, "\tbCorrect     = %s", bCorrect     ? "TRUE" : "FALSE" );
    log_message( tfe_arch_log, "\tbPromiscuous = %s", bPromiscuous ? "TRUE" : "FALSE" );
    log_message( tfe_arch_log, "\tbIAHash      = %s", bIAHash      ? "TRUE" : "FALSE" );
#endif
}

void tfe_arch_line_ctl(int bEnableTransmitter, int bEnableReceiver )
{
#if defined(TFE_DEBUG_ARCH) || defined(TFE_DEBUG_FRAMES)
    log_message( tfe_arch_log, "tfe_arch_line_ctl() called with the following parameters:" );
    log_message( tfe_arch_log, "\tbEnableTransmitter = %s", bEnableTransmitter ? "TRUE" : "FALSE" );
    log_message( tfe_arch_log, "\tbEnableReceiver    = %s", bEnableReceiver    ? "TRUE" : "FALSE" );
#endif
}


typedef struct TFE_PCAP_INTERNAL_tag {

    unsigned int len;
    unsigned char *buffer;

} TFE_PCAP_INTERNAL;

/* Callback function invoked by libpcap for every incoming packet */
static
void TfePcapPacketHandler(unsigned char *param, const struct pcap_pkthdr *header, const unsigned char *pkt_data)
{
    TFE_PCAP_INTERNAL *pinternal = (TFE_PCAP_INTERNAL*)param;

    /* determine the count of bytes which has been returned, 
     * but make sure not to overrun the buffer 
     */
    if (header->caplen < pinternal->len)
        pinternal->len = header->caplen;

    memcpy(pinternal->buffer, pkt_data, pinternal->len);
}

/* the following function receives a frame.

   If there's none, it returns a -1.
   If there is one, it returns the length of the frame in bytes.

   It copies the frame to *buffer and returns the number of copied 
   bytes as return value.

   At most 'len' bytes are copied.
*/
static 
int tfe_arch_receive_frame(TFE_PCAP_INTERNAL *pinternal)
{
    int ret = -1;

    /* check if there is something to receive */
    if (pcapdelay_dispatch(TfePcapFP, 1, (pcap_handler)TfePcapPacketHandler, (unsigned char*)pinternal)!=0) {
        /* Something has been received */
        ret = pinternal->len;
    }

#ifdef TFE_DEBUG_ARCH
    log_message( tfe_arch_log, "tfe_arch_receive_frame() called, returns %d.", ret );
#endif

    return ret;
}

void tfe_arch_transmit(int force,       /* FORCE: Delete waiting frames in transmit buffer */
                       int onecoll,     /* ONECOLL: Terminate after just one collision */
                       int inhibit_crc, /* INHIBITCRC: Do not append CRC to the transmission */
                       int tx_pad_dis,  /* TXPADDIS: Disable padding to 60 Bytes */
                       int txlength,    /* Frame length */
                       unsigned char *txframe    /* Pointer to the frame to be transmitted */
                      )
{
#ifdef TFE_DEBUG_ARCH
    log_message( tfe_arch_log, "tfe_arch_transmit() called, with: "
        "force = %s, onecoll = %s, inhibit_crc=%s, tx_pad_dis=%s, txlength=%u",
        force ?       "TRUE" : "FALSE", 
        onecoll ?     "TRUE" : "FALSE", 
        inhibit_crc ? "TRUE" : "FALSE", 
        tx_pad_dis ?  "TRUE" : "FALSE", 
        txlength
        );
#endif

#ifdef TFE_DEBUG_PKTDUMP
    debug_output( "Transmit frame: ", txframe, txlength);
#endif // #ifdef TFE_DEBUG_PKTDUMP

    if (pcapdelay_sendpacket(TfePcapFP, txframe, txlength) == -1) {
        //log_message(tfe_arch_log, "WARNING! Could not send packet!");
    }
}

/*
  tfe_arch_receive()

  This function checks if there was a frame received.
  If so, it returns 1, else 0.

  If there was no frame, none of the parameters is changed!

  If there was a frame, the following actions are done:

  - at maximum *plen byte are transferred into the buffer given by pbuffer
  - *plen gets the length of the received frame, EVEN if this is more
    than has been copied to pbuffer!
  - if the dest. address was accepted by the hash filter, *phashed is set, else
    cleared.
  - if the dest. address was accepted by the hash filter, *phash_index is
    set to the number of the rule leading to the acceptance
  - if the receive was ok (good CRC and valid length), *prx_ok is set, 
    else cleared.
  - if the dest. address was accepted because it's exactly our MAC address
    (set by tfe_arch_set_mac()), *pcorrect_mac is set, else cleared.
  - if the dest. address was accepted since it was a broadcast address,
    *pbroadcast is set, else cleared.
  - if the received frame had a crc error, *pcrc_error is set, else cleared
*/
int tfe_arch_receive(unsigned char *pbuffer  ,    /* where to store a frame */
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
                    )
{
    int len;

    TFE_PCAP_INTERNAL internal = { *plen, pbuffer };


#ifdef TFE_DEBUG_ARCH
    log_message( tfe_arch_log, "tfe_arch_receive() called, with *plen=%u.", *plen );
#endif

    assert((*plen&1)==0);

    len = tfe_arch_receive_frame(&internal);

    if (len!=-1) {

#ifdef TFE_DEBUG_PKTDUMP
        debug_output( "Received frame: ", internal.buffer, internal.len );
#endif // #ifdef TFE_DEBUG_PKTDUMP

        if (len&1)
            ++len;

        *plen = len;

        /* we don't decide if this frame fits the needs;
         * by setting all zero, we let tfe.c do the work
         * for us
         */
        *phashed =
        *phash_index =
        *pbroadcast = 
        *pcorrect_mac =
        *pcrc_error = 0;

        /* this frame has been received correctly */
        *prx_ok = 1;

        return 1;
    }

    return 0;
}
