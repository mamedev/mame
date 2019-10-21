// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    #defines related to the video emulation of Midway T-unit, W-unit,
    and X-unit games.

**************************************************************************/

#define INIT_TEMPLATED_DMA_DRAW(dest, i, xflip, skip, scale, zero, nonzero) \
	dest[i+0] = &midtunit_video_device::dma_draw<8, xflip, skip, scale, zero, nonzero>; \
	dest[i+1] = &midtunit_video_device::dma_draw<1, xflip, skip, scale, zero, nonzero>; \
	dest[i+2] = &midtunit_video_device::dma_draw<2, xflip, skip, scale, zero, nonzero>; \
	dest[i+3] = &midtunit_video_device::dma_draw<3, xflip, skip, scale, zero, nonzero>; \
	dest[i+4] = &midtunit_video_device::dma_draw<4, xflip, skip, scale, zero, nonzero>; \
	dest[i+5] = &midtunit_video_device::dma_draw<5, xflip, skip, scale, zero, nonzero>; \
	dest[i+6] = &midtunit_video_device::dma_draw<6, xflip, skip, scale, zero, nonzero>; \
	dest[i+7] = &midtunit_video_device::dma_draw<7, xflip, skip, scale, zero, nonzero>;

#define TEMPLATED_DMA_DRAW_NONE(dest, i) \
	dest[i+0] = &midtunit_video_device::dma_draw_none;  \
	dest[i+1] = &midtunit_video_device::dma_draw_none;  \
	dest[i+2] = &midtunit_video_device::dma_draw_none;  \
	dest[i+3] = &midtunit_video_device::dma_draw_none;  \
	dest[i+4] = &midtunit_video_device::dma_draw_none;  \
	dest[i+5] = &midtunit_video_device::dma_draw_none;  \
	dest[i+6] = &midtunit_video_device::dma_draw_none;  \
	dest[i+7] = &midtunit_video_device::dma_draw_none;

#define TEMPLATED_DMA_DRAW_P0(dest, i, skip, scale)   INIT_TEMPLATED_DMA_DRAW(dest, i, false, skip, scale, PIXEL_COPY,  PIXEL_SKIP)
#define TEMPLATED_DMA_DRAW_P1(dest, i, skip, scale)   INIT_TEMPLATED_DMA_DRAW(dest, i, false, skip, scale, PIXEL_SKIP,  PIXEL_COPY)
#define TEMPLATED_DMA_DRAW_C0(dest, i, skip, scale)   INIT_TEMPLATED_DMA_DRAW(dest, i, false, skip, scale, PIXEL_COLOR, PIXEL_SKIP)
#define TEMPLATED_DMA_DRAW_C1(dest, i, skip, scale)   INIT_TEMPLATED_DMA_DRAW(dest, i, false, skip, scale, PIXEL_SKIP,  PIXEL_COLOR)
#define TEMPLATED_DMA_DRAW_P0P1(dest, i, skip, scale) INIT_TEMPLATED_DMA_DRAW(dest, i, false, skip, scale, PIXEL_COPY,  PIXEL_COPY)
#define TEMPLATED_DMA_DRAW_C0C1(dest, i, skip, scale) INIT_TEMPLATED_DMA_DRAW(dest, i, false, skip, scale, PIXEL_COLOR, PIXEL_COLOR)
#define TEMPLATED_DMA_DRAW_C0P1(dest, i, skip, scale) INIT_TEMPLATED_DMA_DRAW(dest, i, false, skip, scale, PIXEL_COLOR, PIXEL_COPY)
#define TEMPLATED_DMA_DRAW_P0C1(dest, i, skip, scale) INIT_TEMPLATED_DMA_DRAW(dest, i, false, skip, scale, PIXEL_COPY,  PIXEL_COLOR)

#define TEMPLATED_DMA_DRAW_P0_XF(dest, i, skip, scale)   INIT_TEMPLATED_DMA_DRAW(dest, i, true, skip, scale, PIXEL_COPY,  PIXEL_SKIP)
#define TEMPLATED_DMA_DRAW_P1_XF(dest, i, skip, scale)   INIT_TEMPLATED_DMA_DRAW(dest, i, true, skip, scale, PIXEL_SKIP,  PIXEL_COPY)
#define TEMPLATED_DMA_DRAW_C0_XF(dest, i, skip, scale)   INIT_TEMPLATED_DMA_DRAW(dest, i, true, skip, scale, PIXEL_COLOR, PIXEL_SKIP)
#define TEMPLATED_DMA_DRAW_C1_XF(dest, i, skip, scale)   INIT_TEMPLATED_DMA_DRAW(dest, i, true, skip, scale, PIXEL_SKIP,  PIXEL_COLOR)
#define TEMPLATED_DMA_DRAW_P0P1_XF(dest, i, skip, scale) INIT_TEMPLATED_DMA_DRAW(dest, i, true, skip, scale, PIXEL_COPY,  PIXEL_COPY)
#define TEMPLATED_DMA_DRAW_C0C1_XF(dest, i, skip, scale) INIT_TEMPLATED_DMA_DRAW(dest, i, true, skip, scale, PIXEL_COLOR, PIXEL_COLOR)
#define TEMPLATED_DMA_DRAW_C0P1_XF(dest, i, skip, scale) INIT_TEMPLATED_DMA_DRAW(dest, i, true, skip, scale, PIXEL_COLOR, PIXEL_COPY)
#define TEMPLATED_DMA_DRAW_P0C1_XF(dest, i, skip, scale) INIT_TEMPLATED_DMA_DRAW(dest, i, true, skip, scale, PIXEL_COPY,  PIXEL_COLOR)

#define INIT_TEMPLATED_DMA_DRAW_GROUP(dest, skip, scale) \
	TEMPLATED_DMA_DRAW_NONE(dest, 0);                   \
	TEMPLATED_DMA_DRAW_P0(dest, 8, skip, scale);        \
	TEMPLATED_DMA_DRAW_P1(dest, 16, skip, scale);       \
	TEMPLATED_DMA_DRAW_P0P1(dest, 24, skip, scale);     \
	TEMPLATED_DMA_DRAW_C0(dest, 32, skip, scale);       \
	TEMPLATED_DMA_DRAW_C0(dest, 40, skip, scale);       \
	TEMPLATED_DMA_DRAW_C0P1(dest, 48, skip, scale);     \
	TEMPLATED_DMA_DRAW_C0P1(dest, 56, skip, scale);     \
	TEMPLATED_DMA_DRAW_C1(dest, 64, skip, scale);       \
	TEMPLATED_DMA_DRAW_P0C1(dest, 72, skip, scale);     \
	TEMPLATED_DMA_DRAW_C1(dest, 80, skip, scale);       \
	TEMPLATED_DMA_DRAW_P0C1(dest, 88, skip, scale);     \
	TEMPLATED_DMA_DRAW_C0C1(dest, 96, skip, scale);     \
	TEMPLATED_DMA_DRAW_C0C1(dest, 104, skip, scale);    \
	TEMPLATED_DMA_DRAW_C0C1(dest, 112, skip, scale);    \
	TEMPLATED_DMA_DRAW_C0C1(dest, 120, skip, scale);    \
	TEMPLATED_DMA_DRAW_NONE(dest, 128);                 \
	TEMPLATED_DMA_DRAW_P0_XF(dest, 136, skip, scale);   \
	TEMPLATED_DMA_DRAW_P1_XF(dest, 144, skip, scale);   \
	TEMPLATED_DMA_DRAW_P0P1_XF(dest, 152, skip, scale); \
	TEMPLATED_DMA_DRAW_C0_XF(dest, 160, skip, scale);   \
	TEMPLATED_DMA_DRAW_C0_XF(dest, 168, skip, scale);   \
	TEMPLATED_DMA_DRAW_C0P1_XF(dest, 176, skip, scale); \
	TEMPLATED_DMA_DRAW_C0P1_XF(dest, 184, skip, scale); \
	TEMPLATED_DMA_DRAW_C1_XF(dest, 192, skip, scale);   \
	TEMPLATED_DMA_DRAW_P0C1_XF(dest, 200, skip, scale); \
	TEMPLATED_DMA_DRAW_C1_XF(dest, 208, skip, scale);   \
	TEMPLATED_DMA_DRAW_P0C1_XF(dest, 216, skip, scale); \
	TEMPLATED_DMA_DRAW_C0C1_XF(dest, 224, skip, scale); \
	TEMPLATED_DMA_DRAW_C0C1_XF(dest, 232, skip, scale); \
	TEMPLATED_DMA_DRAW_C0C1_XF(dest, 240, skip, scale); \
	TEMPLATED_DMA_DRAW_C0C1_XF(dest, 248, skip, scale);

#define DEFINE_TEMPLATED_DMA_DRAW(xflip, skip, scale, zero, nonzero) \
	template void midtunit_video_device::dma_draw<8, xflip, skip, scale, zero, nonzero>();  \
	template void midtunit_video_device::dma_draw<1, xflip, skip, scale, zero, nonzero>();  \
	template void midtunit_video_device::dma_draw<2, xflip, skip, scale, zero, nonzero>();  \
	template void midtunit_video_device::dma_draw<3, xflip, skip, scale, zero, nonzero>();  \
	template void midtunit_video_device::dma_draw<4, xflip, skip, scale, zero, nonzero>();  \
	template void midtunit_video_device::dma_draw<5, xflip, skip, scale, zero, nonzero>();  \
	template void midtunit_video_device::dma_draw<6, xflip, skip, scale, zero, nonzero>();  \
	template void midtunit_video_device::dma_draw<7, xflip, skip, scale, zero, nonzero>();

#define DEFINE_TEMPLATED_DMA_DRAW_P0(skip, scale)   \
	DEFINE_TEMPLATED_DMA_DRAW(false, skip, scale, midtunit_video_device::PIXEL_COPY,  midtunit_video_device::PIXEL_SKIP)
#define DEFINE_TEMPLATED_DMA_DRAW_P1(skip, scale)   \
	DEFINE_TEMPLATED_DMA_DRAW(false, skip, scale, midtunit_video_device::PIXEL_SKIP,  midtunit_video_device::PIXEL_COPY)
#define DEFINE_TEMPLATED_DMA_DRAW_C0(skip, scale)   \
	DEFINE_TEMPLATED_DMA_DRAW(false, skip, scale, midtunit_video_device::PIXEL_COLOR, midtunit_video_device::PIXEL_SKIP)
#define DEFINE_TEMPLATED_DMA_DRAW_C1(skip, scale)   \
	DEFINE_TEMPLATED_DMA_DRAW(false, skip, scale, midtunit_video_device::PIXEL_SKIP,  midtunit_video_device::PIXEL_COLOR)
#define DEFINE_TEMPLATED_DMA_DRAW_P0P1(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW(false, skip, scale, midtunit_video_device::PIXEL_COPY,  midtunit_video_device::PIXEL_COPY)
#define DEFINE_TEMPLATED_DMA_DRAW_C0C1(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW(false, skip, scale, midtunit_video_device::PIXEL_COLOR, midtunit_video_device::PIXEL_COLOR)
#define DEFINE_TEMPLATED_DMA_DRAW_C0P1(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW(false, skip, scale, midtunit_video_device::PIXEL_COLOR, midtunit_video_device::PIXEL_COPY)
#define DEFINE_TEMPLATED_DMA_DRAW_P0C1(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW(false, skip, scale, midtunit_video_device::PIXEL_COPY,  midtunit_video_device::PIXEL_COLOR)

#define DEFINE_TEMPLATED_DMA_DRAW_P0_XF(skip, scale)    \
	DEFINE_TEMPLATED_DMA_DRAW(true, skip, scale, midtunit_video_device::PIXEL_COPY,  midtunit_video_device::PIXEL_SKIP)
#define DEFINE_TEMPLATED_DMA_DRAW_P1_XF(skip, scale)    \
	DEFINE_TEMPLATED_DMA_DRAW(true, skip, scale, midtunit_video_device::PIXEL_SKIP,  midtunit_video_device::PIXEL_COPY)
#define DEFINE_TEMPLATED_DMA_DRAW_C0_XF(skip, scale)    \
	DEFINE_TEMPLATED_DMA_DRAW(true, skip, scale, midtunit_video_device::PIXEL_COLOR, midtunit_video_device::PIXEL_SKIP)
#define DEFINE_TEMPLATED_DMA_DRAW_C1_XF(skip, scale)    \
	DEFINE_TEMPLATED_DMA_DRAW(true, skip, scale, midtunit_video_device::PIXEL_SKIP,  midtunit_video_device::PIXEL_COLOR)
#define DEFINE_TEMPLATED_DMA_DRAW_P0P1_XF(skip, scale)  \
	DEFINE_TEMPLATED_DMA_DRAW(true, skip, scale, midtunit_video_device::PIXEL_COPY,  midtunit_video_device::PIXEL_COPY)
#define DEFINE_TEMPLATED_DMA_DRAW_C0C1_XF(skip, scale)  \
	DEFINE_TEMPLATED_DMA_DRAW(true, skip, scale, midtunit_video_device::PIXEL_COLOR, midtunit_video_device::PIXEL_COLOR)
#define DEFINE_TEMPLATED_DMA_DRAW_C0P1_XF(skip, scale)  \
	DEFINE_TEMPLATED_DMA_DRAW(true, skip, scale, midtunit_video_device::PIXEL_COLOR, midtunit_video_device::PIXEL_COPY)
#define DEFINE_TEMPLATED_DMA_DRAW_P0C1_XF(skip, scale)  \
	DEFINE_TEMPLATED_DMA_DRAW(true, skip, scale, midtunit_video_device::PIXEL_COPY,  midtunit_video_device::PIXEL_COLOR)

#define DEFINE_TEMPLATED_DMA_DRAW_GROUP(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_P0(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_P1(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_C0(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_C1(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_P0P1(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_C0C1(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_C0P1(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_P0C1(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_P0_XF(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_P1_XF(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_C0_XF(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_C1_XF(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_P0P1_XF(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_C0C1_XF(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_C0P1_XF(skip, scale) \
	DEFINE_TEMPLATED_DMA_DRAW_P0C1_XF(skip, scale)
