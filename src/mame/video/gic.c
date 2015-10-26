// license:BSD-3-Clause
// copyright-holders:David Viens
/***************************************************************************

   gic.c

   GI AY-3-8800-1 (Datasheet exists as AY-3-8500-1 Graphics Interface Chip)
   For the GIMINI "Challenger" programmable game system.
   
   Really only ever used in the Unisonic Champion 2711

   More LA tests made by plgDavid on hardware pretty much confirmed what is found
   in the AY-3-8950-1 datasheet, but with more fine grained detail.

   the GIC does not have internal ram of any sort apart from shift registers, 
   instead it relies on the external shared ram, (see page 7-85) Appendix AY-3-8950-1
   
   Unverified on LA (since the video pins are all connected into a composite mix):
   at line 46 it lowers GIC_BUSY, until line 240

   Verified using LA:
   It will read the external ram areas continuously while GIC_BUSY is low (for 12.36ms)

   (NOTE: OCTAL)
   
   000,001,002,003,004,005,  110,111,112,113,114,115,116,117,120,121,122,123,124,125 (15 times - No first bg line?)
   006,007,010,011,012,013,  110,111,112,113,114,115,116,117,120,121,122,123,124,125 (16 times)
   
   014,015,016,017,020,021,  125,126,127,130,131,132,133,134,135,136,137,140,141,142 (16 times)
   022,023,024,025,026,027,  125,126,127,130,131,132,133,134,135,136,137,140,141,142 (16 times)

   030,031,032,033,034,035,  142,143,144,145,146,147,150,151,152,153,154,155,156,157 (16 times)
   036,037,040,041,042,043,  142,143,144,145,146,147,150,151,152,153,154,155,156,157 (16 times)

   044,045,046,047,050,051,  157,160,161,162,163,164,165,166,167,170,171,172,173,174 (16 times)
   052,053,054,055,056,057,  157,160,161,162,163,164,165,166,167,170,171,172,173,174 (16 times)

   060,061,062,063,064,065,  174,175,176,177,200,201,202,203,204,205,206,207,210,211 (16 times)
   066,067,070,071,072,073,  174,175,176,177,200,201,202,203,204,205,206,207,210,211 (16 times)

   074,075,076,077,100,101,  211,212,213,214,215,216,217,220,221,222,223,224,225,226 (16 times)
   102,103,104,105,106,107,  211,212,213,214,215,216,217,220,221,222,223,224,225,226 (16 times)

   000,001,002,003,004,005,  000,001,002,003,004,005,006,007,010,011,012,013,014,015 (once! padding?)

   for a total of (12*20*16) = 3840 RAM reads (3 clocks per read at 1.79MHz)

   Then it relingishes control to the CPU by raising BUSREQ.
   
   Cloking in more detail: (in 1.79MHz clocks)
   boot:
	busy:1  5360 clocks
	busy:0 22116 clocks
	busy:1  7752 clocks
	busy:0 22116 clocks
	busy:1  7752 clocks	
	(...)

   There are NO IRQ handshakes, just BUSREQ sync shared RAM

***************************************************************************/

#include "emu.h"
#include "gic.h"

// device type definition
const device_type GIC = &device_creator<gic_device>;


//Font data taken from Paul Robson's simulator
//http://worstconsole.blogspot.ca/2012/12/the-worstconsoleever.html
//A real AY-3-8800-1 (dead) is going to decap for a good dump
ROM_START( gic_font )
	ROM_REGION( 0x200, "cgrom", 0 )
	ROM_LOAD( "ay-3-8800-1.bin", 0x0000, 0x200,  BAD_DUMP CRC(d9f11d2b) SHA1(60ef45d51d102cd3af78787008d9aed848137bee)) 
ROM_END


//-------------------------------------------------
//  gic_device - constructor
//-------------------------------------------------

gic_device::gic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GIC, "GIC", tag, owner, clock, "gic", __FILE__)
	, device_sound_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_cgrom(0)
	, m_audiocnt(0)
	, m_audioval(0)	
    , m_audioreset(0)
    , m_ram(0)	
{
}


gic_device::gic_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int lines, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, device_sound_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_cgrom(0)	
	, m_audiocnt(0)
	, m_audioval(0)
    , m_audioreset(0)
    , m_ram(0)	
{
}

const rom_entry *gic_device::device_rom_region() const
{
	//there is only one... how do I get rid of this?
	return ROM_NAME( gic_font );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gic_device::device_start()
{
	m_cgrom = memregion("cgrom")->base();

	// Let the screen create our temporary bitmap with the screen's dimensions
	m_screen->register_screen_bitmap(m_bitmap);

	m_vblank_timer = timer_alloc(TIMER_VBLANK);
	m_vblank_timer->adjust( m_screen->time_until_pos(1, END_ACTIVE_SCAN + 18 ), 0, m_screen->scan_period() );

	// allocate the audio stream
	m_stream = stream_alloc( 0, 1, clock()/(2*228) ); 
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void gic_device::device_reset()
{
	m_audiocnt=0;
	m_audioval=0;	
	m_audioreset=0;
}

#define GIC_CLUB    28
#define GIC_SPACE    0
	
void gic_device::draw_char_left(int startx, int starty, UINT8 code, bitmap_ind16 &bitmap){
	
	UINT8*ptr = &m_cgrom[code*GIC_CHAR_H];
		
	for (size_t y=0;y<GIC_CHAR_H;y++){
		UINT8 current = *ptr++;	
		UINT8 nextx=0;
		UINT8 curry= starty+y;		
		for(UINT8 x=0x20;x!=0;x=x/2){	
			if (current&x)
				m_bitmap.pix16(curry,startx+nextx) = GIC_WHITE;
			nextx++;
		}
	}
}

void gic_device::draw_char_right(int startx, int starty, UINT8 code, bitmap_ind16 &bitmap, int bg_col){
	
	UINT8*ptr = &m_cgrom[code*GIC_CHAR_H];
	
	for (size_t y=0;y<GIC_CHAR_H;y++){
		UINT8 current = *ptr++;	
		UINT8 nextx=0;
		UINT8 curry= starty+y;

		m_bitmap.pix16(curry,startx+nextx) = bg_col;
		nextx++;	
		for(UINT8 x=0x20;x!=0;x=x/2){			
			m_bitmap.pix16(curry,startx+nextx) = (current&x)?GIC_WHITE:bg_col;	
			nextx++;
		}
		m_bitmap.pix16(curry,startx+nextx) = bg_col;
		nextx++;		
		m_bitmap.pix16(curry,startx+nextx) = bg_col;		
	}
}

UINT32 gic_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bitmap.fill(GIC_GREEN);

	size_t XSTART = BORDER_SIZE;
	size_t YSTART = START_ACTIVE_SCAN;
	
	//left hand side first
	UINT8 current=0;
	for(UINT8 cy=0;cy<GIC_LEFT_H;cy++){
		for(UINT8 cx=0;cx<GIC_LEFT_W;cx++){
			draw_char_left(XSTART+(cx*GIC_CHAR_W),
			               YSTART+(cy*GIC_CHAR_H),
			               m_ram[current],
				           m_bitmap);
			current++;
		}
	}

	//right hand side is next
	current=0x48;//110 octal
	XSTART+=(GIC_LEFT_W*GIC_CHAR_W)+1;
	
	for(UINT8 cy=0;cy<GIC_RIGHT_H;cy++){
		for(UINT8 cx=0;cx<GIC_RIGHT_W;cx++){
			//complex case
			UINT8 data = m_ram[current++];
			
			size_t currX   = (XSTART+           (cx*(3+GIC_CHAR_W)));
			size_t currUP  = (YSTART+           (cy*(2*GIC_CHAR_H)));
			size_t currLOW = (YSTART+GIC_CHAR_H+(cy*(2*GIC_CHAR_H)));
			
			switch(data&0xC0){
				case 0x00:{
					//lower rectangle only, normal char
					draw_char_right(currX,currLOW,data,m_bitmap,GIC_GREEN);
				}break;
				
				//White block
				case 0xC0:{
					//upper rectangle
					draw_char_right(currX,currUP, GIC_SPACE,m_bitmap,GIC_WHITE);	
					//lower rectangle						  
					draw_char_right(currX,currLOW,GIC_SPACE,m_bitmap,GIC_WHITE);
				}break;
				
				//Draw a card
				case 0x40:{
					int bgColor = (data&0x10)?GIC_RED:GIC_BLACK;	
					//upper rectangle
					draw_char_right(currX,currUP,           (data&0xF)+0x30,m_bitmap,bgColor);					
					//lower rectangle		
					draw_char_right(currX,currLOW,GIC_CLUB+((data&0x30)>>4),m_bitmap,bgColor);	
				}break;
				
				default:printf("gic unknown char! %02X\n",data); break;
			}
		}
	}

	copybitmap( bitmap, m_bitmap, 0, 0, 0, 0, cliprect );
	return 0;
}

/* AUDIO SECTION */

void gic_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch ( id )
	{
		case TIMER_VBLANK:
			//flag the audio to reset
			m_audioreset = 1;//phase need to reset! on next clock/228
		break;	
	}
}

#define GIC_AUDIO_BYTE 0x96

void gic_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];

    //Audio is basic and badly implemented (doubt that was the intent)
	//The datasheet list the 3 different frequencies the GIC can generate: 500,1000 and 2000Hz
	//but it is clear (for an audio guy at least) that the resulting spectrum 
	//is not a pure square wav. In fact, the counter is reset on vertical sync!
	//http://twitter.com/plgDavid/status/527269086016077825
	//...thus creating a buzzing sound. 

	//Dumping the audio pin value each time 
	// either (PHI2 made a 0->1 transition (1.789MHz)
	//     or (PHI1 made a 1->1 transition (1.789MHz)
	//I found that the granularity of audio transitions 
	//(including phase resets and silences) was 228 clocks
	//The audio subsystem thus runs at 1.789MHz/228 = 7849.88Hz
	
	//when 1
	//normal period:912 clocks (228*4)
	//hi for 456 clocks
	//lo for 456 clocks
	//reset period: (each frame)
	//hi for 228 clocks
	//lo for 456 clocks
	//when 2
	//normal period lasts 1824 clocks (228*8)
	//hi for 912 clocks
	//lo for 912 clocks
	//reset period: (each frame)
	//hi for 912   (228*4)
	//lo for 1596  (228*7)
	//hi for 912   (228*4)
	//when 4
	//normal period lasts 3648 clocks (228*16)
	//hi for 1824(228*8) 
	//lo for 1824(228*8) 	
	//Reset period:
	//lo for 1824(228*8) 
	//hi for 2508(228*11)
	//lo for 1824(228*8)
	//hi for 1824(228*8)
	
	if(!m_ram) return;

	UINT8 audioByte = m_ram[GIC_AUDIO_BYTE]*2;
	
	if(!audioByte){
		for(size_t i = 0; i < samples; i++)
			*buffer++ = 0;	

   		m_audioval   = 0;
		m_audiocnt   = 0;     
        m_audioreset = 0;
		return;//early	
	}
	
    //forced resynch @ 59.95Hz
    if(m_audioreset){
   		m_audioval   = 0;//forced low
		m_audiocnt   = 0;     
        m_audioreset = 0;
    }

	for(size_t i=0; i < samples; i++){
        m_audiocnt++;
		if(m_audiocnt >= audioByte){
			m_audioval = !m_audioval;
			m_audiocnt=0;
		}
		*buffer++ = m_audioval<<13;
    }
}
