#include "pcap.h"

/* prototype of the packet handler */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

int main(int argc, char **argv)
{
pcap_if_t *alldevs;
pcap_if_t *d;
int inum;
int i=0;
pcap_t *adhandle;
char errbuf[PCAP_ERRBUF_SIZE];
pcap_dumper_t *dumpfile;


	
    /* Check command line */
	if(argc != 2)
	{
        printf("usage: %s filename", argv[0]);
        return -1;
    }
    
	/* Retrieve the device list on the local machine */
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
	if ( (adhandle= pcap_open(d->name,			// name of the device
							  65536,			// portion of the packet to capture
												// 65536 guarantees that the whole packet will be captured on all the link layers
							  PCAP_OPENFLAG_PROMISCUOUS, 	// promiscuous mode
							  1000,				// read timeout
							  NULL,				// authentication on the remote machine
							  errbuf			// error buffer
							  ) ) == NULL)
	{
		fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* Open the dump file */
	dumpfile = pcap_dump_open(adhandle, argv[1]);

	if(dumpfile==NULL)
	{
		fprintf(stderr,"\nError opening output file\n");
		return -1;
	}
    
    printf("\nlistening on %s... Press Ctrl+C to stop...\n", d->description);
	
    /* At this point, we no longer need the device list. Free it */
    pcap_freealldevs(alldevs);
    
    /* start the capture */
    pcap_loop(adhandle, 0, packet_handler, (unsigned char *)dumpfile);

    return 0;
}

/* Callback function invoked by libpcap for every incoming packet */
void packet_handler(u_char *dumpfile, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	/* save the packet on the dump file */
	pcap_dump(dumpfile, header, pkt_data);
}
