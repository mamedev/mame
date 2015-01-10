#include <stdlib.h>
#include <stdio.h>

#include <pcap.h>


void main(int argc, char **argv)
{
pcap_t *fp;
char errbuf[PCAP_ERRBUF_SIZE];
u_char packet[100];
int i;

	/* Check the validity of the command line */
	if (argc != 2)
	{
		printf("usage: %s interface (e.g. 'rpcap://eth0')", argv[0]);
		return;
	}
    
	/* Open the output device */
	if ( (fp= pcap_open(argv[1],			// name of the device
						100,				// portion of the packet to capture (only the first 100 bytes)
						PCAP_OPENFLAG_PROMISCUOUS, 	// promiscuous mode
						1000,				// read timeout
						NULL,				// authentication on the remote machine
						errbuf				// error buffer
						) ) == NULL)
	{
		fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", argv[1]);
		return;
	}

	/* Supposing to be on ethernet, set mac destination to 1:1:1:1:1:1 */
	packet[0]=1;
	packet[1]=1;
	packet[2]=1;
	packet[3]=1;
	packet[4]=1;
	packet[5]=1;
	
	/* set mac source to 2:2:2:2:2:2 */
	packet[6]=2;
	packet[7]=2;
	packet[8]=2;
	packet[9]=2;
	packet[10]=2;
	packet[11]=2;
	
	/* Fill the rest of the packet */
	for(i=12;i<100;i++)
	{
		packet[i]=(u_char)i;
	}

	/* Send down the packet */
	if (pcap_sendpacket(fp, packet, 100 /* size */) != 0)
	{
		fprintf(stderr,"\nError sending the packet: %s\n", pcap_geterr(fp));
		return;
	}

	return;
}
