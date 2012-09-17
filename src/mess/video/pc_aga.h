/*
  pc cga/mda combi adapters

  one type hardware switchable between cga and mda/hercules
  another type software switchable between cga and mda/hercules

  some support additional modes like
  commodore pc10 320x200 in 16 colors


    // aga
    // 256 8x8 thick chars
    // 256 8x8 thin chars
    // 256 9x14 in 8x16 chars, line 3 is connected to a10
    ROM_LOAD("aga.chr",     0x00000, 0x02000, CRC(aca81498))
    // hercules font of above
    ROM_LOAD("hercules.chr", 0x00000, 0x1000, CRC(7e8c9d76))

*/

#define AGA_SCREEN_NAME "screen"
#define AGA_MC6845_NAME "mc6845_aga"

MACHINE_CONFIG_EXTERN( pcvideo_aga );
MACHINE_CONFIG_EXTERN( pcvideo_pc200 );


enum AGA_MODE  { AGA_OFF, AGA_COLOR, AGA_MONO };
void pc_aga_set_mode(running_machine &machine, AGA_MODE mode);

DECLARE_READ8_HANDLER( pc_aga_videoram_r );
DECLARE_WRITE8_HANDLER( pc_aga_videoram_w );

DECLARE_READ8_HANDLER( pc200_videoram_r );
DECLARE_WRITE8_HANDLER( pc200_videoram_w );
DECLARE_READ16_HANDLER( pc200_videoram16le_r );
DECLARE_WRITE16_HANDLER( pc200_videoram16le_w );

DECLARE_READ8_HANDLER( pc200_cga_r );
DECLARE_WRITE8_HANDLER( pc200_cga_w );
DECLARE_READ16_HANDLER( pc200_cga16le_r );
DECLARE_WRITE16_HANDLER( pc200_cga16le_w );

