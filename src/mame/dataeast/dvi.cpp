// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"

// This just attempts to parse some of the header structures in the DVI data Dragon Gun uses...
// see http://www.fileformat.info/format/dvi/egff.htm
// I don't know if there are any software decoders for this format, I think it was a hardware solution?
// The Dragon Gun data doesn't seem to have all fields filled in..
//
// Hopefully somebody who actually understands video (de)compression can actually flesh this out...


//#define dviprintf printf
#define dviprintf device->logerror


struct DVI_Header
{
	uint32_t dv1;     // header ID (VDVI)
	uint16_t dv2;     // header size (sometimes just 1)
	uint16_t dv3;     // header version
	uint32_t dv4;     // offset of annotation data
};


struct AVSS_Header
{
	uint32_t av1;       // header ID (AVSS)
	uint16_t av2;     // header size
	uint16_t av3;     // header version
	uint16_t av4;       // number of stream groups
	uint16_t av5;       // size of each stream group
	uint32_t av6;       // offset to first stream group
	uint16_t av7;       // format of stream groups
	uint16_t av8;       // size of stream header
	uint16_t av9;       // format of stream header
	uint16_t av10;      // number of stream headers
	uint32_t av11;      // offset to stream structures array
	uint32_t av12;      // offset to substream headers array
	uint32_t av13;      // number of labels
	uint32_t av14;      // offset to first label
	uint16_t av15;      // label size
	uint16_t av16;      // label format
	uint32_t av17;      // offset to video sequence header
	uint16_t av18;      // size of video sequence header
	uint16_t av19;      // version of frame headers
	uint32_t av20;      // number of frame headers
	uint32_t av21;      // size of frame headers
	uint32_t av22;      // offset to first frame
	uint32_t av23;      // offset of last frame
	uint16_t av24;      // size of frame header
	uint16_t av25;      // size of frame dictionary
	uint32_t av26;      // offset of frame dictionary
	uint16_t av27;      // format of frame dictionary
	uint16_t av28;      // framerate
	uint32_t av29;      // streaming data?
	uint32_t av30;      // unused
	uint8_t  av31[32];  // unused
};

struct STRM_Header
{
	uint32_t st1;       // header ID (STRM)
	uint16_t st2;       // header size
	uint16_t st3;       // stream type
	uint16_t st4;       // stream subtype
	uint16_t st5;       // number of substream headers
	uint16_t st6;       // next stream ID
	uint16_t st7;       // group IP
	uint16_t st8;       // unused (Pad)
	uint16_t st9;       // frame size flag..
	uint32_t st10;      // max data per frame
	uint32_t st11;      // offset to first substream header
	uint8_t  st12[16];  // name
};

#if 0
struct AUDI_Header
{
	uint32_t au1;      // header iID (AUDI)
	uint16_t au2;      // header size
	uint16_t au3;      // header format
	uint8_t  au4[80];  // original media filename
	uint32_t au5;      // original media frame ID
	uint16_t au6;    // original media frame ID
	uint16_t au7;      // unused (Pad)
	uint32_t au8;      // number of frames
	uint32_t au9;      // offset of next header
	uint8_t  au10[16]; // library stream name
	uint8_t  au11[16]; // compression algorithm name
	uint32_t au12;     // sample freq
	uint16_t au13;     // filter cutoff
	uint16_t au14;     // unused
	uint16_t au15;     // left volume
	uint16_t au16;     // right volume
	uint32_t au17;     // unused
	uint32_t au18;     // first frame ID
	uint32_t au19;     // mono / stereo
	uint16_t au20;     // playback rate
	uint16_t au21;     // unused (Pad)
	uint32_t au22;     // Digital Compression ID
};
#endif

struct CIMG_Header
{
	uint32_t ci1;     // header ID (CIMG)
	uint16_t ci2;     // header size
	uint16_t ci3;     // header format
	uint8_t  ci4[80]; // source media filename
	uint32_t ci5;     // source media frame ID
	uint16_t ci6;     // source media stream ID
	uint16_t ci7;     // Unused (Pad value)
	uint32_t ci8;     // Frames before next header
	uint32_t ci9;     // Next Header Pos
	uint16_t ci10;    // X Pos
	uint16_t ci11;    // Y Pos
	uint16_t ci12;    // Width
	uint16_t ci13;    // Height
	uint16_t ci14;    // X Crop
	uint16_t ci15;    // Y Crop
	uint16_t ci16;    // Unused
	uint16_t ci17;    // Unused
	uint32_t ci18;    // Interframe Frequency
	uint16_t ci19;    // Min Buffer Size
	uint16_t ci20;    // Max Buffer Size
	uint16_t ci21;    // Decompression ID
	uint16_t ci22;    // Unused (Pad Value)
	uint32_t ci23;    // Digitial Compression ID
};

struct FHEAD_Header
{
	uint32_t fh1;              // frame number
	uint32_t fh2;              // offset of previous frame
	uint32_t fh3;              // checksum
	//uint32_t framesizearray? // just the frame data?
};

struct FDICT_Header
{
	uint32_t fd1; // offset of frame header
};

static uint32_t R32(uint8_t **currptr)
{
	uint32_t ret = 0;
	ret |= *(*currptr)++; ret <<=8;
	ret |= *(*currptr)++; ret <<=8;
	ret |= *(*currptr)++; ret <<=8;
	ret |= *(*currptr)++;

	return ret;
}

static uint16_t R16(uint8_t **currptr)
{
	uint16_t ret = 0;
	ret |= *(*currptr)++; ret <<=8;
	ret |= *(*currptr)++;

	return ret;
}

static uint8_t R8(uint8_t **currptr)
{
	uint8_t ret = 0;
	ret |= *(*currptr)++;
	return ret;
}

void process_dvi_data(device_t *device,uint8_t* dvi_data, int baseoffset, int regionsize)
{
	DVI_Header DVI;

	uint8_t* currptr = dvi_data + baseoffset;

	DVI.dv1 = R32(&currptr);
	DVI.dv2 = R16(&currptr);
	DVI.dv3 = R16(&currptr);
	DVI.dv4 = R32(&currptr);

	dviprintf("DVI Header\n");
	dviprintf("Header ID          %08x %c%c%c%c\n", DVI.dv1, (DVI.dv1>>24)&0xff,  (DVI.dv1>>16)&0xff,  (DVI.dv1>>8)&0xff,  (DVI.dv1>>0)&0xff);
	dviprintf("Header Size (or 1) %04x\n", DVI.dv2);
	dviprintf("Header Version     %04x\n", DVI.dv3);
	dviprintf("Annotation Offset  %08x\n", DVI.dv4);
	if (DVI.dv1 != 0x56445649) { dviprintf("Header Error\n"); return; }
	dviprintf("\n");

	AVSS_Header AVSS;

	AVSS.av1 = R32(&currptr);
	AVSS.av2 = R16(&currptr);
	AVSS.av3 = R16(&currptr);
	AVSS.av4 = R16(&currptr);
	AVSS.av5 = R16(&currptr);
	AVSS.av6 = R32(&currptr);
	AVSS.av7 = R16(&currptr);
	AVSS.av8 = R16(&currptr);
	AVSS.av9 = R16(&currptr);
	AVSS.av10 = R16(&currptr);
	AVSS.av11 = R32(&currptr);
	AVSS.av12 = R32(&currptr);
	AVSS.av13 = R32(&currptr);
	AVSS.av14 = R32(&currptr);
	AVSS.av15 = R16(&currptr);
	AVSS.av16 = R16(&currptr);
	AVSS.av17 = R32(&currptr);
	AVSS.av18 = R16(&currptr);
	AVSS.av19 = R16(&currptr);
	AVSS.av20 = R32(&currptr);
	AVSS.av21 = R32(&currptr);
	AVSS.av22 = R32(&currptr);
	AVSS.av23 = R32(&currptr);
	AVSS.av24 = R16(&currptr);
	AVSS.av25 = R16(&currptr);
	AVSS.av26 = R32(&currptr);
	AVSS.av27 = R16(&currptr);
	AVSS.av28 = R16(&currptr);
	AVSS.av29 = R32(&currptr);
	AVSS.av30 = R32(&currptr);
	for (int i=0;i<32;i++) { AVSS.av31[i] = R8(&currptr); }

	dviprintf(" AVSS Header\n");
	dviprintf(" Header ID          %08x %c%c%c%c\n", AVSS.av1, (AVSS.av1>>24)&0xff,  (AVSS.av1>>16)&0xff,  (AVSS.av1>>8)&0xff,  (AVSS.av1>>0)&0xff);
	dviprintf(" Header Size        %04x\n", AVSS.av2);
	dviprintf(" Header Version     %04x\n", AVSS.av3);
	dviprintf(" Stream Group Count %04x\n", AVSS.av4);
	dviprintf(" Stream Group Size  %04x\n", AVSS.av5);
	dviprintf(" Stream Group Offs  %08x\n", AVSS.av6);
	dviprintf(" Stream Group Vers  %04x\n", AVSS.av7);
	dviprintf(" Stream Size        %04x\n", AVSS.av8);
	dviprintf(" Stream Version     %04x\n", AVSS.av9);
	dviprintf(" Stream Count       %04x\n", AVSS.av10);
	dviprintf(" Stream Offset      %08x\n", AVSS.av11);
	dviprintf(" Header Pool Offset %08x\n", AVSS.av12);
	dviprintf(" Label Count        %08x\n", AVSS.av13);
	dviprintf(" Label Offset       %08x\n", AVSS.av14);
	dviprintf(" Label Size         %04x\n", AVSS.av15);
	dviprintf(" Label Version      %04x\n", AVSS.av16);
	dviprintf(" Vid Seq Hdr Offset %08x\n", AVSS.av17);
	dviprintf(" Vid Seq Hdr Size   %04x\n", AVSS.av18);
	dviprintf(" Frame Version      %04x\n", AVSS.av19);
	dviprintf(" Frame Count        %08x\n", AVSS.av20);
	dviprintf(" Frame Size         %08x\n", AVSS.av21);
	dviprintf(" First Frame Offset %08x\n", AVSS.av22);
	dviprintf(" EO Frame Offset    %08x\n", AVSS.av23);
	dviprintf(" Frame Header Size  %04x\n", AVSS.av24);
	dviprintf(" Frame Dir Size     %04x\n", AVSS.av25);
	dviprintf(" Frame Dir Offset   %08x\n", AVSS.av26);
	dviprintf(" Frame Dir Vers     %04x\n", AVSS.av27);
	dviprintf(" Frame PerSec       %04x\n", AVSS.av28);
	dviprintf(" UpdateFlag         %08x\n", AVSS.av29);
	dviprintf(" FreeBlock          %08x\n", AVSS.av30);
	dviprintf(" Patch              ");  for (int i=0;i<32;i++) { dviprintf("%02x", AVSS.av31[i]); }; dviprintf("\n");
	dviprintf("\n");

	for (int s=0;s<AVSS.av10;s++)
	{
		STRM_Header STRM;

		STRM.st1 = R32(&currptr);
		STRM.st2 = R16(&currptr);
		STRM.st3 = R16(&currptr);
		STRM.st4 = R16(&currptr);
		STRM.st5 = R16(&currptr);
		STRM.st6 = R16(&currptr);
		STRM.st7 = R16(&currptr);
		STRM.st8 = R16(&currptr);
		STRM.st9 = R16(&currptr);
		STRM.st10 = R32(&currptr);
		STRM.st11 = R32(&currptr);
		for (int i=0;i<16;i++) { STRM.st12[i] = R8(&currptr); }

		dviprintf("     STRM Header\n");
		dviprintf("     Header ID          %08x %c%c%c%c\n", STRM.st1, (STRM.st1>>24)&0xff,  (STRM.st1>>16)&0xff,  (STRM.st1>>8)&0xff,  (STRM.st1>>0)&0xff);
		dviprintf("     Header Size        %04x\n", STRM.st2);
		dviprintf("     Type               %04x\n", STRM.st3);
		dviprintf("     SubType            %04x\n", STRM.st4);
		dviprintf("     Header Count       %04x\n", STRM.st5);
		dviprintf("     Next Stream Number %04x\n", STRM.st6);
		dviprintf("     Stream Group Numbr %04x\n", STRM.st7);
		dviprintf("     Padding            %04x\n", STRM.st8);
		dviprintf("     Flag               %04x\n", STRM.st9);
		dviprintf("     FrameSize          %08x\n", STRM.st10);
		dviprintf("     FirstHeaderOffset  %08x\n", STRM.st11);
		dviprintf("     Name               ");  for (int i=0;i<16;i++) { dviprintf("%02x", STRM.st12[i]); }; dviprintf("\n");
		dviprintf("\n");

		uint8_t* subptr = dvi_data+STRM.st11+baseoffset;

		for (int h=0;h<STRM.st5;h++)
		{
			// I believe in these can be either CIMG or AUDI blocks, but we don't have any of the latter
			CIMG_Header CIMG;


			CIMG.ci1 = R32(&subptr);
			CIMG.ci2 = R16(&subptr);
			CIMG.ci3 = R16(&subptr);
			for (int i=0;i<80;i++) { CIMG.ci4[i] = R8(&subptr); }
			CIMG.ci5 = R32(&subptr);
			CIMG.ci6 = R16(&subptr);
			CIMG.ci7 = R16(&subptr);
			CIMG.ci8 = R32(&subptr);
			CIMG.ci9 = R32(&subptr);
			CIMG.ci10 = R16(&subptr);
			CIMG.ci11 = R16(&subptr);
			CIMG.ci12 = R16(&subptr);
			CIMG.ci13 = R16(&subptr);
			CIMG.ci14 = R16(&subptr);
			CIMG.ci15 = R16(&subptr);
			CIMG.ci16 = R16(&subptr);
			CIMG.ci17 = R16(&subptr);
			CIMG.ci18 = R32(&subptr);
			CIMG.ci19 = R16(&subptr);
			CIMG.ci20 = R16(&subptr);
			CIMG.ci21 = R16(&subptr);
			CIMG.ci22 = R16(&subptr);
			CIMG.ci23 = R32(&subptr);

			dviprintf("         CIMG Header\n");
			dviprintf("         Header ID          %08x %c%c%c%c\n", CIMG.ci1, (CIMG.ci1>>24)&0xff,  (CIMG.ci1>>16)&0xff,  (CIMG.ci1>>8)&0xff,  (CIMG.ci1>>0)&0xff);
			dviprintf("         Header Size        %04x\n", CIMG.ci2);
			dviprintf("         Version            %04x\n", CIMG.ci3);
			dviprintf("         Original Name      ");  for (int i=0;i<80;i++) { dviprintf("%02x", CIMG.ci4[i]); }; dviprintf("\n");
			dviprintf("         Original Frame     %08x\n", CIMG.ci5);
			dviprintf("         Original Stream    %04x\n", CIMG.ci6);
			dviprintf("         Padding            %04x\n", CIMG.ci7);
			dviprintf("         Frame Count        %08x\n", CIMG.ci8);
			dviprintf("         Next Header Offset %08x\n", CIMG.ci9);
			dviprintf("         X Position         %04x\n", CIMG.ci10);
			dviprintf("         Y Position         %04x\n", CIMG.ci11);
			dviprintf("         X Length           %04x\n", CIMG.ci12);
			dviprintf("         Y Length           %04x\n", CIMG.ci13);
			dviprintf("         X Crop             %04x\n", CIMG.ci14);
			dviprintf("         Y Crop             %04x\n", CIMG.ci15);
			dviprintf("         Drop Frame         %04x\n", CIMG.ci16);
			dviprintf("         Drop Phrase        %04x\n", CIMG.ci17);
			dviprintf("         Still Period       %08x\n", CIMG.ci18);
			dviprintf("         Buffer Minimum     %04x\n", CIMG.ci19);
			dviprintf("         Buffer Maximum     %04x\n", CIMG.ci20);
			dviprintf("         Decode Algorithm   %04x\n", CIMG.ci21);
			dviprintf("         Padding(2)         %04x\n", CIMG.ci22);
			dviprintf("         DCFID              %08x\n", CIMG.ci23);
			dviprintf("\n");

			subptr = dvi_data+CIMG.ci9+baseoffset;
		}

	}

	/* Frame Dictionaries etc. */
	/* is this just a seek table of sorts? the first doesn't have an entry (pointed to by First Frame Offset - AVSS.av22) the first two here are actually bad values, and the last one points to nothing */
	/* some entries also seem to point to the wrong places?? I'm guessing this isn't used when playing back the data ... it seems like the upper and lower words are out of sync... */
	dviprintf("Frame Dictionaries\n\n");

	uint8_t* frameptr = dvi_data + AVSS.av26 + 2 + baseoffset; // +2 ??
//  uint8_t* frameptr = dvi_data + AVSS.av26 + baseoffset;

	for (int f=0;f<AVSS.av20;f++)
	{
		FDICT_Header FDICT;
		FDICT.fd1 = R32(&frameptr);

		FDICT.fd1 = ((FDICT.fd1 & 0xffff0000)>>16) |  ((FDICT.fd1 & 0x0000ffff)<<16);

		dviprintf(" %04d Frame Offset %08x\n", f, FDICT.fd1);

			uint8_t* frameptr2 = dvi_data + ((FDICT.fd1 + baseoffset)&(regionsize-1)) ;
		FHEAD_Header FHEAD;
		FHEAD.fh1 = R32(&frameptr2);
		FHEAD.fh2 = R32(&frameptr2);
		FHEAD.fh3 = R32(&frameptr2);

		dviprintf("     Frame Num         %08x\n", FHEAD.fh1);
		dviprintf("     Previous Offset   %08x\n", FHEAD.fh2);
		dviprintf("     Frame Checksum    %08x\n", FHEAD.fh3);


	}

	dviprintf("\n");



}
