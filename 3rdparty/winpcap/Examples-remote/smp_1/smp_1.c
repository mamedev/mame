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
#include <conio.h>

#include <pcap.h>


int main()
{	
pcap_if_t *alldevs, *d;
pcap_t *fp;
u_int inum, i=0;
char errbuf[PCAP_ERRBUF_SIZE];
int res;
struct pcap_pkthdr *header;
const u_char *pkt_data;
struct pcap_pkthdr old;

	printf("SMP_1\n");
	printf("\nThis program tests the WinPcap kernel driver on SMP machines.\n");
	printf("The program tests that timestamps on the captured packets are consistent,\n");
	printf("and that the caplen is equal to the packet length.\n");
	printf("If there is an error, it will print out a message saying \"Inconsistent XXX\"\n");

	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
	{
		fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}
		
	/* Print the list */
	for(d=alldevs; d; d=d->next)
	{
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}
		
	if(i==0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
		return -1;
	}
		
	printf("Enter the interface number (1-%d):",i);
	scanf_s("%d", &inum);
		
	if(inum < 1 || inum > i)
	{
		printf("\nInterface number out of range.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}
		
	/* Jump to the selected adapter */
	for(d=alldevs, i=0; i< inum-1 ;d=d->next, i++);
	
	/* Open the device */
	if ( (fp= pcap_open(d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf) ) == NULL)
	{
		fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	old.ts.tv_sec=0;
	old.ts.tv_usec=0;


	/* Read the packets */
	while((res = pcap_next_ex( fp, &header, &pkt_data)) >= 0){

		if(res == 0)
			continue;

		//check that caplen is equal to packet length
		if (header->caplen!=header->len)
			printf("Inconsistent header: CapLen %d\t Len %d\n",header->caplen,header->len);

		//check that timestamps always grow
		if ( old.ts.tv_sec > header->ts.tv_sec || (old.ts.tv_sec == header->ts.tv_sec  && old.ts.tv_usec > header->ts.tv_usec))
			printf("Inconsistent Timestamps! Old was %d.%.06d - New is %d.%.06d\n",old.ts.tv_sec,old.ts.tv_usec, header->ts.tv_sec,header->ts.tv_usec);

		old=*header;

	}

	if(res == -1){
		printf("Error reading the packets: %s\n", pcap_geterr(fp));
		return -1;
	}

	_getch();

	return 0;
}
