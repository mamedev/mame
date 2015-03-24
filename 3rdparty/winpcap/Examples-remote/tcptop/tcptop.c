/*
 * Copyright (c) 1999 - 2005 NetGroup, Politecnico di Torino (Italy)
 * Copyright (c) 2005 - 2006 CACE Technologies, Davis (California)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Politecnico di Torino, CACE Technologies 
 * nor the names of its contributors may be used to endorse or promote 
 * products derived from this software without specific prior written 
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include <pcap.h>

void usage();

void dispatcher_handler(u_char *, const struct pcap_pkthdr *, const u_char *);


void main(int argc, char **argv)
{
pcap_t *fp;
char errbuf[PCAP_ERRBUF_SIZE];
struct timeval st_ts;
u_int netmask;
struct bpf_program fcode;
  
	/* Check the validity of the command line */
	if (argc != 2)
	{
		usage();
		return;
	}
		
	/* Open the output adapter */
	if ( (fp= pcap_open(argv[1], 100, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf) ) == NULL)
	{
		fprintf(stderr,"\nUnable to open adapter %s.\n", errbuf);
		return;
	}

    /* Don't care about netmask, it won't be used for this filter */
    netmask=0xffffff; 

    //compile the filter
    if (pcap_compile(fp, &fcode, "tcp", 1, netmask) <0 )
	{
        fprintf(stderr,"\nUnable to compile the packet filter. Check the syntax.\n");
        /* Free the device list */
        return;
    }
    
    //set the filter
    if (pcap_setfilter(fp, &fcode)<0)
	{
        fprintf(stderr,"\nError setting the filter.\n");
		pcap_close(fp);
        /* Free the device list */
        return;
    }

	/* Put the interface in statstics mode */
	if (pcap_setmode(fp, MODE_STAT)<0)
	{
        fprintf(stderr,"\nError setting the mode.\n");
		pcap_close(fp);
        /* Free the device list */
        return;
    }


	printf("TCP traffic summary:\n");

	/* Start the main loop */
	pcap_loop(fp, 0, dispatcher_handler, (PUCHAR)&st_ts);

	pcap_close(fp);
	return;
}

void dispatcher_handler(u_char *state, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	struct timeval *old_ts = (struct timeval *)state;
	u_int delay;
	LARGE_INTEGER Bps,Pps;
	struct tm ltime;
	char timestr[16];
	time_t local_tv_sec;

	/* Calculate the delay in microseconds from the last sample. */
	/* This value is obtained from the timestamp that the associated with the sample. */
	delay=(header->ts.tv_sec - old_ts->tv_sec) * 1000000 - old_ts->tv_usec + header->ts.tv_usec;
	/* Get the number of Bits per second */
	Bps.QuadPart=(((*(LONGLONG*)(pkt_data + 8)) * 8 * 1000000) / (delay));
	/*                                            ^      ^
                                                  |      |
                                                  |      | 
                                                  |      |
                         converts bytes in bits --       |
                                                         |
                    delay is expressed in microseconds --
	*/

	/* Get the number of Packets per second */
	Pps.QuadPart=(((*(LONGLONG*)(pkt_data)) * 1000000) / (delay));

	/* Convert the timestamp to readable format */
	local_tv_sec = header->ts.tv_sec;
	localtime_s(&ltime, &local_tv_sec);
	strftime( timestr, sizeof timestr, "%H:%M:%S", &ltime);

	/* Print timestamp*/
	printf("%s ", timestr);

	/* Print the samples */
	printf("BPS=%I64u ", Bps.QuadPart);
	printf("PPS=%I64u\n", Pps.QuadPart);

	//store current timestamp
	old_ts->tv_sec=header->ts.tv_sec;
	old_ts->tv_usec=header->ts.tv_usec;
}


void usage()
{
	
	printf("\nShows the TCP traffic load, in bits per second and packets per second.\nCopyright (C) 2002 Loris Degioanni.\n");
	printf("\nUsage:\n");
	printf("\t tcptop adapter\n");
	printf("\t You can use \"WinDump -D\" if you don't know the name of your adapters.\n");

	exit(0);
}