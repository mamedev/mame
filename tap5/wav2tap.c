/*
	wav2tap tool by F.Frances
	version 1.1: fixed a difference between wav2tap's behavior and
	the real Oric: the Oric accepts non-$16 values after the three
	$16 bytes (four when counting the first synchronized byte) and
	before the starting $24 byte. Thanks to Simon for spotting this.
*/


#include <stdio.h>

struct {
	char sig[4];
	int riff_size;
	char datasig[4];
	char fmtsig[4];
	int fmtsize;
	short tag;
	short channels;
	int freq;
	int bytes_per_sec;
	short byte_per_sample;
	short bits_per_sample;
	char samplesig[4];
	int length;
} sample_riff;

int sync_ok=0;
int offset,pos;
FILE *in, *out;

main(int argc,char **argv)
{	
	unsigned start,end,byte;
	if (argc!=3) { printf("Wav2tap special Simon\nUsage: wav2tap file.wav file.tap\n",argv[0]); exit(1);}
	in=fopen(argv[1],"rb");
	if (in==NULL) { printf("Unable to open WAV file\n"); exit(1);}
	fread(&sample_riff,sizeof(sample_riff),1,in);
//	if (sample_riff.channels!=1 || sample_riff.freq!=4800 || sample_riff.byte_per_sample!=1) {
//		printf("Invalid WAV format: should be 4800 Hz, 8-bit, mono\n");
//		exit(1);
//	}
	printf("Channels:%d, Freq=%d, Sampling=%d\n", sample_riff.channels, sample_riff.freq, sample_riff.byte_per_sample);	

	out=fopen(argv[2],"wb");
	if (out==NULL) { printf("Unable to create TAP file\n"); exit(1);}

	for (;;) {
//		putc(getbit());
		fprintf(out,"%d", getbit());
	}
}

int getc2(FILE *f)
{
	int val;
	val = getc(f);
	if (sample_riff.byte_per_sample == 2)
		getc(f);
	else if (sample_riff.byte_per_sample == 4)
	{
		val = getc(f);
		getc(f);
		val = getc(f);
	}
//	printf("%c", val);
	return val;
}

getbit()
{
	int val,length;
skip:
	length=1;
	val=getc2(in); pos++;
	if (val==EOF) exit(0);
	while (val<=0x80) {
		length++;
		val=getc2(in); pos++;
		if (val==EOF) exit(0);
	}
	length++;
	val=getc2(in); pos++;
	if (val==EOF) exit(0);
	while (val>=0x80) {
		length++;
		val=getc2(in); pos++;
		if (val==EOF) exit(0);
	}
//	printf("len=%d\n", length);
	if (length>40) return 1;
	else return 0;
}