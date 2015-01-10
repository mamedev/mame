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

#define MAX_PRINT 80
#define MAX_LINE 16


void usage();


int main(int argc, char **argv)
{
	pcap_t *fp;
	char errbuf[PCAP_ERRBUF_SIZE];
	char *source = NULL;
	char *ofilename = NULL;
	char *filter = NULL;
	int i;
	pcap_dumper_t *dumpfile;
	struct bpf_program fcode;
	bpf_u_int32 NetMask;
	int res;
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	
	if (argc == 1)
	{
		usage();
		return -1;
	}

	/* Parse parameters */
	for(i=1;i < argc; i+= 2)
	{
		switch (argv[i] [1])
		{
			case 's':
			{
				source=argv[i+1];
			};
			break;
			
			case 'o':
			{
				ofilename=argv[i+1];
			};
			break;

			case 'f':
			{
				filter=argv[i+1];
			};
			break;
		}
	}
	
	// open a capture from the network
	if (source != NULL)
	{
		if ((fp = pcap_open_live(source,		// name of the device
			65536,								// portion of the packet to capture. 
												// 65536 grants that the whole packet will be captured on all the MACs.
			1,									// promiscuous mode (nonzero means promiscuous)
			1000,								// read timeout
			errbuf								// error buffer
			)) == NULL)
		{
			fprintf(stderr,"\nUnable to open the adapter.\n");
			return -2;
		}
	}
	else usage();

	if (filter != NULL)
	{
		// We should loop through the adapters returned by the pcap_findalldevs_ex()
		// in order to locate the correct one.
		//
		// Let's do things simpler: we suppose to be in a C class network ;-)
		NetMask=0xffffff;

		//compile the filter
		if(pcap_compile(fp, &fcode, filter, 1, NetMask) < 0)
		{
			fprintf(stderr,"\nError compiling filter: wrong syntax.\n");

			pcap_close(fp);
			return -3;
		}

		//set the filter
		if(pcap_setfilter(fp, &fcode)<0)
		{
			fprintf(stderr,"\nError setting the filter\n");

			pcap_close(fp);
			return -4;
		}

	}

	//open the dump file
	if (ofilename != NULL)
	{
		dumpfile= pcap_dump_open(fp, ofilename);

		if (dumpfile == NULL)
		{
			fprintf(stderr,"\nError opening output file\n");

			pcap_close(fp);
			return -5;
		}
	}
	else usage();

	//start the capture
 	while((res = pcap_next_ex( fp, &header, &pkt_data)) >= 0)
	{

		if(res == 0)
		/* Timeout elapsed */
		continue;

		//save the packet on the dump file
		pcap_dump((unsigned char *) dumpfile, header, pkt_data);

	}

	pcap_close(fp);
	pcap_dump_close(dumpfile);

	return 0;
}


void usage()
{

	printf("\npf - Generic Packet Filter.\n");
	printf("\nUsage:\npf -s source -o output_file_name [-f filter_string]\n\n");
	exit(0);
}
