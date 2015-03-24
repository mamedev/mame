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


#include <stdio.h>
#include <conio.h>


#include "..\..\..\Include\packet32.h"
#include <ntddndis.h>

#define Max_Num_Adapter 10

// Prototypes

void PrintPackets(LPPACKET lpPacket);

char        AdapterList[Max_Num_Adapter][1024];

int main()
{
//define a pointer to an ADAPTER structure

LPADAPTER  lpAdapter = 0;

//define a pointer to a PACKET structure

LPPACKET   lpPacket;

int        i;
DWORD      dwErrorCode;

//ascii strings
char		AdapterName[8192]; // string that contains a list of the network adapters
char		*temp,*temp1;


int			AdapterNum=0,Open;
ULONG		AdapterLength;

char buffer[256000];  // buffer to hold the data coming from the driver

struct bpf_stat stat;
	
	//
	// Obtain the name of the adapters installed on this machine
	//
	printf("Packet.dll test application. Library version:%s\n", PacketGetVersion());

	printf("Adapters installed:\n");
	i=0;	

	AdapterLength = sizeof(AdapterName);

	if(PacketGetAdapterNames(AdapterName,&AdapterLength)==FALSE){
		printf("Unable to retrieve the list of the adapters!\n");
		return -1;
	}
	temp=AdapterName;
	temp1=AdapterName;

	while ((*temp!='\0')||(*(temp-1)!='\0'))
	{
		if (*temp=='\0') 
		{
			memcpy(AdapterList[i],temp1,temp-temp1);
			temp1=temp+1;
			i++;
		}
		temp++;
	}
		  
	AdapterNum=i;
	for (i=0;i<AdapterNum;i++)
		printf("\n%d- %s\n",i+1,AdapterList[i]);
	printf("\n");


	do 
	{
		printf("Select the number of the adapter to open : ");
		scanf_s("%d",&Open);
		if (Open>AdapterNum) printf("\nThe number must be smaller than %d",AdapterNum); 
	} while (Open>AdapterNum);
	

	
	
	lpAdapter =   PacketOpenAdapter(AdapterList[Open-1]);
	
	if (!lpAdapter || (lpAdapter->hFile == INVALID_HANDLE_VALUE))
	{
		dwErrorCode=GetLastError();
		printf("Unable to open the adapter, Error Code : %lx\n",dwErrorCode); 

		return -1;
	}	

	// set the network adapter in promiscuous mode
	
	if(PacketSetHwFilter(lpAdapter,NDIS_PACKET_TYPE_PROMISCUOUS)==FALSE){
			printf("Warning: unable to set promiscuous mode!\n");
	}

	// set a 512K buffer in the driver
	if(PacketSetBuff(lpAdapter,512000)==FALSE){
			printf("Unable to set the kernel buffer!\n");
			return -1;
	}

	// set a 1 second read timeout
	if(PacketSetReadTimeout(lpAdapter,1000)==FALSE){
			printf("Warning: unable to set the read tiemout!\n");
	}

	//allocate and initialize a packet structure that will be used to
	//receive the packets.
	if((lpPacket = PacketAllocatePacket())==NULL){
		printf("\nError: failed to allocate the LPPACKET structure.");
		return (-1);
	}
	PacketInitPacket(lpPacket,(char*)buffer,256000);
	
	//main capture loop
	while(!_kbhit())
	{
	    // capture the packets
		if(PacketReceivePacket(lpAdapter,lpPacket,TRUE)==FALSE){
			printf("Error: PacketReceivePacket failed");
			return (-1);
		}

		PrintPackets(lpPacket);
	}


	//print the capture statistics
	if(PacketGetStats(lpAdapter,&stat)==FALSE){
			printf("Warning: unable to get stats from the kernel!\n");
	}
	else
		printf("\n\n%d packets received.\n%d Packets lost",stat.bs_recv,stat.bs_drop);

	PacketFreePacket(lpPacket);
	
	// close the adapter and exit

	PacketCloseAdapter(lpAdapter);
	return (0);
}

// this function prints the content of a block of packets received from the driver

void PrintPackets(LPPACKET lpPacket)
{

	ULONG	i, j, ulLines, ulen, ulBytesReceived;
	char	*pChar, *pLine, *base;
	char	*buf;
	u_int off=0;
	u_int tlen,tlen1;
	struct bpf_hdr *hdr;
	
		ulBytesReceived = lpPacket->ulBytesReceived;


		buf = lpPacket->Buffer;

		off=0;

		while(off<ulBytesReceived){	
			if(_kbhit())return;
		hdr=(struct bpf_hdr *)(buf+off);
		tlen1=hdr->bh_datalen;
		tlen=hdr->bh_caplen;
		printf("Packet length, captured portion: %ld, %ld\n", tlen1, tlen);
		off+=hdr->bh_hdrlen;

		ulLines = (tlen + 15) / 16;

		pChar =(char*)(buf+off);
		base=pChar;
		off=Packet_WORDALIGN(off+tlen);
		
		for ( i=0; i<ulLines; i++ )
		{

			pLine =pChar;

			printf( "%08lx : ", pChar-base );

			ulen=tlen;
			ulen = ( ulen > 16 ) ? 16 : ulen;
			tlen -= ulen;

			for ( j=0; j<ulen; j++ )
				printf( "%02x ", *(BYTE *)pChar++ );

			if ( ulen < 16 )
				printf( "%*s", (16-ulen)*3, " " );

			pChar = pLine;

			for ( j=0; j<ulen; j++, pChar++ )
				printf( "%c", isprint( (unsigned char)*pChar ) ? *pChar : '.' );

			printf( "\n" );
		} 

		printf( "\n" );
		}
} 

 