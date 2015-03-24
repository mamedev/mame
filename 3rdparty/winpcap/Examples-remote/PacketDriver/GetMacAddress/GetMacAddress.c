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
char		AdapterList[Max_Num_Adapter][1024];

int main()
{
	LPADAPTER	lpAdapter = 0;
	int			i;
	DWORD		dwErrorCode;
	char		AdapterName[8192];
	char		*temp,*temp1;
	int			AdapterNum=0,Open;
	ULONG		AdapterLength;
	PPACKET_OID_DATA  OidData;
	BOOLEAN		Status;
	
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
	

	//
	// Open the selected adapter
	//

	lpAdapter =   PacketOpenAdapter(AdapterList[Open-1]);
	
	if (!lpAdapter || (lpAdapter->hFile == INVALID_HANDLE_VALUE))
	{
		dwErrorCode=GetLastError();
		printf("Unable to open the adapter, Error Code : %lx\n",dwErrorCode); 

		return -1;
	}	

	// 
	// Allocate a buffer to get the MAC adress
	//

	OidData = malloc(6 + sizeof(PACKET_OID_DATA));
	if (OidData == NULL) 
	{
		printf("error allocating memory!\n");
		PacketCloseAdapter(lpAdapter);
		return -1;
	}

	// 
	// Retrieve the adapter MAC querying the NIC driver
	//

	OidData->Oid = OID_802_3_CURRENT_ADDRESS;

	OidData->Length = 6;
	ZeroMemory(OidData->Data, 6);
	
	Status = PacketRequest(lpAdapter, FALSE, OidData);
	if(Status)
	{
		printf("The MAC address of the adapter is %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
			(OidData->Data)[0],
			(OidData->Data)[1],
			(OidData->Data)[2],
			(OidData->Data)[3],
			(OidData->Data)[4],
			(OidData->Data)[5]);
	}
	else
	{
		printf("error retrieving the MAC address of the adapter!\n");
	}

	free(OidData);
	PacketCloseAdapter(lpAdapter);
	return (0);
}

