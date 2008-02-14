/* sys16spr.c
**
**  This module maps spriteram for various Sega System16/System18 games to
**  a shared abstraction represented by the sys16_sprite_attributes
**  structure.
**
**  Each function takes a pointer to a sprite attribute structure to fill
**  out.  This function is initialized to zero automatically prior to the
**  function call.
**
**  The next parameter, source points to the spriteram
**  associated with a particular sprite.
**
**  The final parameter bJustGetColor is set when this function is called while
**  marking colors.  During this pass, only the palette field needs to be
**  populated.
**
**  This function must return 1 if the sprite was flagged as the last sprite,
**  or 0 if the sprite was not flagged as the last sprite.
**
**  We should probably unpack sprites into an array once, rather than processing
**  sprites one at a time, once when marking colors and once when rendering them.
**  Note that we need to draw sprites from front to back to achieve proper sprite-tilemap
**  orthogonality, so would make it easier to know how many sprites there are (and
**  thus which is the 'last sprite' with which we need to start.
*/

/*
Changes:
03/12/04  Charles MacDonald
- Fixed end of list and sprite hide flag processing in sys16_sprite_shinobi, hwchamp output test works properly.
- Fixed size of zoom fields, fixes sprite alignment issues in hwchamp
*/

#include "driver.h"
#include "system16.h"

int sys16_sprite_shinobi( struct sys16_sprite_attributes *sprite, const UINT16 *source, int bJustGetColor ){
/* standard sprite hardware (Shinobi, Altered Beast, Golden Axe
    0   YYYYYYYY    YYYYYYYY    top, bottom (screen coordinates)
    1   -------X    XXXXXXXX    left (screen coordinate)
    2   EH------F   WWWWWWWW    end list flag, hide sprite flag, flipx, signed pitch
    3   TTTTTTTT    TTTTTTTT    word offset for start of sprite data; each word is 16 bits (4 pixels)
    4   ----BBBB    PPCCCCCC    attributes: bank, priority, color
    5   ------YY    YYYXXXXX    zoomy, zoomx
    6   --------    --------
    7   --------    --------
*/
	UINT16 ypos = source[0];
	UINT16 width = source[2];
	int top = ypos&0xff;
	int bottom = ypos>>8;
	if(width & 0x8000) return 1; // End of list bit set
	if(width & 0x4000) return 0; // Hide this sprite bit set
	if(top >= bottom || top >= 0xe0) return 0; // Invalid height or off-screen
	if(bottom >= 0xe0) bottom = 0xe0; // Clip bottom of sprite

	{
		UINT16 attributes = source[4];

		UINT16 zoomx = source[5] & 0x1F;
		UINT16 zoomy = (source[5] >> 5) & 0x1F;

		// Rest of rendering code expects zoom to be 10 bits
		zoomx <<= 5;
		zoomy <<= 5;

		// Having zoomy set to zoomx when zoomy is zero heavily distorts the multi-sprite opponents in hwchamp
		//if( zoomy==0 || source[6]==0xffff ) zoomy = zoomx; /* if zoomy is 0, use zoomx instead */

		sprite->x = source[1] + sys16_sprxoffset;
		sprite->y = top;
		sprite->priority = (attributes>>6)&0x3;
		sprite->color = 1024/16 + (attributes&0x3f);
		sprite->screen_height = bottom-top;
		sprite->flags = SYS16_SPR_VISIBLE;
		if( width&0x100 ) sprite->flags |= SYS16_SPR_FLIPX;
		if ((attributes&0x3f)==0x3f) sprite->flags|= SYS16_SPR_SHADOW;
		sprite->zoomx = zoomx;
		sprite->zoomy = zoomy;
		sprite->pitch = source[2]&0xff;
		sprite->gfx = ( source[3] + (sys16_obj_bank[(attributes>>8)&0xf]<<16) ) << 1; //*
	}
	return 0;
}

int sys16_sprite_passshot( struct sys16_sprite_attributes *sprite, const UINT16 *source, int bJustGetColor ){
/* Passing shot 4p needs:   passshot_y=-0x23; passshot_width=1;
    0   -------X    XXXXXXXX    left (screen coordinate)
    1   YYYYYYYY    YYYYYYYY    bottom, top (screen coordinates)
    2   XTTTTTTT    TTTTTTTT    (pen data, flipx, flipy)
    3   --------    FWWWWWWW    pitch: flipy, logical width
    4   ------ZZ    ZZZZZZZZ    zoom
    5   PPCCCCCC    BBBB----    attributes: priority, color, bank
    6   --------    --------
    7   --------    --------
*/
	int passshot_y=0;
	int passshot_width=0;
	UINT16 attributes = source[5];
	UINT16 ypos = source[1];
	int bottom = (ypos>>8)+passshot_y;
	int top = (ypos&0xff)+passshot_y;
	if( bottom>top && ypos!=0xffff ){
		int bank = (attributes>>4)&0xf;
		UINT16 number = source[2];
		UINT16 width = source[3];
		int zoom = source[4]&0x3ff;
		int xpos = source[0] + sys16_sprxoffset;
		sprite->screen_height = bottom - top;
		sprite->priority = attributes>>14;
		sprite->color = 1024/16+ ((attributes>>8)&0x3f);
		/* hack */
		if( passshot_width) { /* 4 player bootleg version */
			width = -width;
			number -= width*(bottom-top-1)-1;
		}
		sprite->flags = SYS16_SPR_VISIBLE;
		if( number & 0x8000 ) sprite->flags |= SYS16_SPR_FLIPX;
		if (((attributes>>8)&0x3f)==0x3f)	// shadow sprite
			sprite->flags|= SYS16_SPR_SHADOW;
		sprite->pitch = width&0xff;
		if( sprite->flags&SYS16_SPR_FLIPX ){
			bank = (bank-1) & 0xf; /* ? */
		}
		sprite->gfx = ((number-(short)width)*4 + (sys16_obj_bank[bank] << 17))/2; //*
		sprite->x = xpos;
		sprite->y = top+2; //*
		sprite->zoomx = sprite->zoomy = zoom;
	}
	return 0;
}


int sys16_sprite_quartet2( struct sys16_sprite_attributes *sprite, const UINT16 *source, int bJustGetColor ){
/* Quartet2, Alexkidd, Bodyslam, mjleague, shinobibl
    0   YYYYYYYY YYYYYYYY   bottom, top
    1   -------X XXXXXXXX   xpos
    2   -------- WWWWWWWW   pitch
    3   FTTTTTTT TTTTTTTT   flipx, gfx
    4   --CCCCCC BBBBPPPP   color, bank, priority
    5   -------- --------
    6   -------- --------
    7   -------- --------
*/
	UINT16 ypos = source[0];
	int top = ypos&0xff;
	int bottom = ypos>>8;
	if( bottom == 0xff ) return 1;
	if(bottom !=0 && bottom > top){
		UINT16 spr_pri=(source[4])&0xf; /* ?? */
		UINT16 bank=(source[4]>>4) &0xf;
		UINT16 pal=(source[4]>>8)&0x3f;
		UINT16 tsource[4];
		UINT16 width;
		int gfx;

		if (spr_pri) { /* MASH - ?? */
			tsource[2]=source[2];
			tsource[3]=source[3];
			if((tsource[3] & 0x7f80) == 0x7f80){
				bank=(bank-1)&0xf;
				tsource[3]^=0x8000;
			}
			tsource[2] &= 0x00ff;
			if (tsource[3]&0x8000){ // reverse
				tsource[2] |= 0x0100;
				tsource[3] &= 0x7fff;
			}
			gfx = tsource[3]*4;
			width = tsource[2];
			//top++;
			//bottom++;
			sprite->x = source[1] + sys16_sprxoffset;
			if(sprite->x > 0x140) sprite->x-=0x200;
			sprite->y = top;
			sprite->priority = spr_pri;
			sprite->color = 1024/16 + pal;
			sprite->screen_height = bottom-top;
			sprite->pitch = width&0xff;
			sprite->flags = SYS16_SPR_VISIBLE;
			if( width&0x100 ) sprite->flags |= SYS16_SPR_FLIPX;
			if( pal==0x3f ) sprite->flags|= SYS16_SPR_SHADOW; // shadow sprite
			sprite->gfx = ((gfx &0x3ffff) + (sys16_obj_bank[bank] << 17))/2;
		}
	}
	return 0;
}
