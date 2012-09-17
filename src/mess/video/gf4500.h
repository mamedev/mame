/*

    NVIDIA GoForce 4500

    (c) 2010 Tim Schuerewegen

*/

#ifndef __GF4500_H__
#define __GF4500_H__

DECLARE_READ32_HANDLER( gf4500_r );
DECLARE_WRITE32_HANDLER( gf4500_w );

VIDEO_START( gf4500 );
SCREEN_UPDATE_RGB32( gf4500 );

#endif /* __GF4500_H__ */
