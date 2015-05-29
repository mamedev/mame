// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/* This program is based on DIS68k by Aaron Giles */

#include "emu.h"

static UINT8 *filebuf;
static UINT32 offset;

#define STANDALONE
#include "34010dsm.c"


static const char *const Options[]=
{
	"begin","end","offset",0
};

static void usage (void)
{
	printf ("Usage: DIS34010 [options] <filename>\n"
			"Available options are:\n"
			" -begin  - Specify begin offset in file to disassemble in bits [0]\n"
			" -end    - Specify end offset in file to disassemble in bits [none]\n"
			" -offset - Specify address to load program in bits [0]\n"
			"All values should be entered in hexadecimal\n");
	exit (1);
}

int main (int argc,char *argv[])
{
		UINT8 i,j,n;
		char *filename=0,buf[80];
	FILE *f;
		UINT32 begin=0,end=(UINT32)-1,filelen,len,pc;
	printf ("DIS34010\n"
						"Copyright Zsolt Vasvari/Aaron Giles\n");

	for (i=1,n=0;i<argc;++i)
	{
		if (argv[i][0]!='-')
		{
			switch (++n)
			{
			case 1:  filename=argv[i]; break;
			default: usage();
			}
		}
		else
		{
			for (j=0;Options[j];++j)
				if (!strcmp(argv[i]+1,Options[j])) break;

			switch (j)
			{
			case 0:  ++i; if (i>argc) usage();
								begin=strtoul(argv[i],0,16) >> 3;
				break;
			case 1:  ++i; if (i>argc) usage();
								end=strtoul(argv[i],0,16) >> 3;
				break;
			case 2:  ++i; if (i>argc) usage();
								offset=strtoul(argv[i],0,16) >> 3;
				break;
			default: usage();
			}
		}
	}

	if (!filename)
	{
		usage();
		return 1;
	}
	f=fopen (filename,"rb");
	if (!f)
	{
		printf ("Unable to open %s\n",filename);
		return 2;
	}
	fseek (f,0,SEEK_END);
	filelen=ftell (f);
	fseek (f,begin,SEEK_SET);
	len=(filelen>end)? (end-begin+1):(filelen-begin);
	filebuf=malloc(len+16);
	if (!filebuf)
	{
		printf ("Memory allocation error\n");
		fclose (f);
		return 3;
	}
	memset (filebuf,0,len+16);
	if (fread(filebuf,1,len,f)!=len)
	{
		printf ("Read error\n");
		fclose (f);
		free (filebuf);
		return 4;
	}
	fclose (f);
	pc=0;
	while (pc<len-1)
	{
		i=(Dasm34010 (buf,pc<<3))>>3;

		printf ("%08X: ",(pc+offset) << 3);
		for (j=0;j<i ;++j) printf("%02X ",filebuf[pc+j]);
		for (   ;j<10;++j) printf("   ");
		printf(buf);
		printf ("\n");
		pc+=i;
	}
	free (filebuf);
	return 0;
}
