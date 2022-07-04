// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes
/*
* font_dwrite.cpp
*
*/

#include "font_module.h"
#include "modules/osdmodule.h"
#include "modules/lib/osdlib.h"

#if defined(OSD_WINDOWS)

#include <windows.h>

#include <cmath>
#include <memory>
#include <stdexcept>

// Windows Imaging Components
#include <wincodec.h>

// Load InitGuid after WIC to work around missing format problem
#include <initguid.h>

// Direct2D
#include <d2d1_1.h>
#include <dwrite_1.h>

// WIC GUIDs
DEFINE_GUID(CLSID_WICImagingFactory, 0xcacaf262, 0x9370, 0x4615, 0xa1, 0x3b, 0x9f, 0x55, 0x39, 0xda, 0x4c, 0xa);
DEFINE_GUID(GUID_WICPixelFormat8bppAlpha, 0xe6cd0116, 0xeeba, 0x4161, 0xaa, 0x85, 0x27, 0xdd, 0x9f, 0xb3, 0xa8, 0x95);

#include <wrl/client.h>
#undef interface

#include "strconv.h"
#include "corestr.h"
#include "winutil.h"
#include "osdcore.h"

using namespace Microsoft::WRL;

//-------------------------------------------------
//  Some formulas and constants
//-------------------------------------------------

//#define DEFAULT_CELL_HEIGHT (200)
#define DEFAULT_EM_HEIGHT (166.5f)

// 1DIP = 1/96in vs 1pt = 1/72in (or 4 / 3)
static const float DIPS_PER_POINT = (4.0f / 3.0f);
static const float POINTS_PER_DIP = (3.0f / 4.0f);

//-------------------------------------------------
//  Error handling macros
//-------------------------------------------------

// Macro to check for a failed HRESULT and if failed, return 0
#define HR_RET( CALL, ret ) do { \
	result = CALL; \
	if (FAILED(result)) { \
		osd_printf_error(#CALL " failed with error 0x%X\n", (unsigned int)result); \
		return ret; } \
} while (0)

#define HR_RETHR( CALL ) HR_RET(CALL, result)
#define HR_RET0( CALL ) HR_RET(CALL, 0)
#define HR_RET1( CALL ) HR_RET(CALL, 1)

// Debugging functions
#ifdef DWRITE_DEBUGGING

//-------------------------------------------------
//  Save image to file
//-------------------------------------------------

HRESULT SaveBitmap(IWICBitmap* bitmap, GUID pixelFormat, const WCHAR *filename)
{
	HRESULT result = S_OK;
	ComPtr<IWICStream> stream;
	ComPtr<ID2D1Factory1> d2dfactory;
	ComPtr<IDWriteFactory> dwriteFactory;
	ComPtr<IWICImagingFactory> wicFactory;

	OSD_DYNAMIC_API(dwrite, "dwrite.dll");
	OSD_DYNAMIC_API(d2d1, "d2d1.dll");
	OSD_DYNAMIC_API_FN(dwrite, HRESULT, WINAPI, DWriteCreateFactory, DWRITE_FACTORY_TYPE, REFIID, IUnknown **);
	OSD_DYNAMIC_API_FN(d2d1, HRESULT, WINAPI, D2D1CreateFactory, D2D1_FACTORY_TYPE, REFIID, const D2D1_FACTORY_OPTIONS *, void **);

	if (!OSD_DYNAMIC_API_TEST(D2D1CreateFactory) || !OSD_DYNAMIC_API_TEST(DWriteCreateFactory))
		return ERROR_DLL_NOT_FOUND;

	// Create a Direct2D factory
	HR_RETHR(OSD_DYNAMIC_CALL(D2D1CreateFactory,
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory1),
		nullptr,
		reinterpret_cast<void**>(d2dfactory.GetAddressOf())));

	// Initialize COM - ignore failure
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	// Create a DirectWrite factory.
	HR_RETHR(OSD_DYNAMIC_CALL(DWriteCreateFactory,
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown **>(dwriteFactory.GetAddressOf())));

	HR_RETHR(CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWICImagingFactory),
		(void**)&wicFactory));

	HR_RETHR(wicFactory->CreateStream(&stream));
	HR_RETHR(stream->InitializeFromFilename(filename, GENERIC_WRITE));

	ComPtr<IWICBitmapEncoder> encoder;
	HR_RETHR(wicFactory->CreateEncoder(GUID_ContainerFormatBmp, nullptr, &encoder));
	HR_RETHR(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache));

	ComPtr<IWICBitmapFrameEncode> frameEncode;
	HR_RETHR(encoder->CreateNewFrame(&frameEncode, nullptr));
	HR_RETHR(frameEncode->Initialize(nullptr));

	UINT width, height;
	HR_RETHR(bitmap->GetSize(&width, &height));
	HR_RETHR(frameEncode->SetSize(width, height));
	HR_RETHR(frameEncode->SetPixelFormat(&pixelFormat));

	HR_RETHR(frameEncode->WriteSource(bitmap, nullptr));

	HR_RETHR(frameEncode->Commit());
	HR_RETHR(encoder->Commit());

	return S_OK;
}

HRESULT SaveBitmap2(bitmap_argb32 &bitmap, const WCHAR *filename)
{
	HRESULT result;

	// Convert the bitmap into a form we understand and save it
	std::unique_ptr<uint32_t> pBitmap(new uint32_t[bitmap.width() * bitmap.height()]);
	for (int y = 0; y < bitmap.height(); y++)
	{
		uint32_t* pRow = pBitmap.get() + (y * bitmap.width());
		for (int x = 0; x < bitmap.width(); x++)
		{
			uint32_t pixel = bitmap.pix(y, x);
			pRow[x] = (pixel == 0xFFFFFFFF) ? rgb_t(0xFF, 0x00, 0x00, 0x00) : rgb_t(0xFF, 0xFF, 0xFF, 0xFF);
		}
	}

	ComPtr<IWICImagingFactory> wicFactory;
	HR_RETHR(CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWICImagingFactory),
		(void**)&wicFactory));

	// Save bitmap
	ComPtr<IWICBitmap> bmp2 = nullptr;
	wicFactory->CreateBitmapFromMemory(
		bitmap.width(),
		bitmap.height(),
		GUID_WICPixelFormat32bppRGBA,
		bitmap.width() * sizeof(uint32_t),
		bitmap.width() * bitmap.height() * sizeof(uint32_t),
		(BYTE*)pBitmap.get(),
		&bmp2);

	SaveBitmap(bmp2.Get(), GUID_WICPixelFormat32bppRGBA, filename);

	return S_OK;
}

#endif

//-------------------------------------------------
//  FontDimension class - holds a font dimension
//  and makes it availabile in different formats
//-------------------------------------------------

class FontDimension
{
private:
	uint16_t  m_designUnitsPerEm;
	float   m_emSizeInDip;
	float   m_designUnits;

public:
	FontDimension(uint16_t designUnitsPerEm, float emSizeInDip, float designUnits)
	{
		m_designUnitsPerEm = designUnitsPerEm;
		m_emSizeInDip = emSizeInDip;
		m_designUnits = designUnits;
	}

	uint16_t DesignUnitsPerEm() const
	{
		return m_designUnitsPerEm;
	}

	float EmSizeInDip() const
	{
		return m_emSizeInDip;
	}

	float DesignUnits() const
	{
		return m_designUnits;
	}

	int Dips() const
	{
		return static_cast<int>(floor((m_designUnits * m_emSizeInDip) / m_designUnitsPerEm));
	}

	float Points() const
	{
		return Dips() * POINTS_PER_DIP;
	}

	FontDimension operator-(const FontDimension &other) const
	{
		if (m_designUnitsPerEm != other.m_designUnitsPerEm || m_emSizeInDip != other.m_emSizeInDip)
		{
			throw std::invalid_argument("Attempted subtraction of FontDimension with different scale.");
		}

		return FontDimension(m_designUnitsPerEm, m_emSizeInDip, m_designUnits - other.m_designUnits);
	}

	FontDimension operator+(const FontDimension &other) const
	{
		if (m_designUnitsPerEm != other.m_designUnitsPerEm || m_emSizeInDip != other.m_emSizeInDip)
		{
			throw std::invalid_argument("Attempted addition of FontDimension with different scale.");
		}

		return FontDimension(m_designUnitsPerEm, m_emSizeInDip, m_designUnits + other.m_designUnits);
	}
};

//-------------------------------------------------
//  FontABCWidths class - hold width-related
//  font dimensions for a glyph
//-------------------------------------------------

class FontABCWidths
{
private:
	FontDimension m_advanceWidth;
	FontDimension m_a;
	FontDimension m_c;

public:
	FontABCWidths(const FontDimension &advanceWidth, const FontDimension &leftSideBearing, const FontDimension &rightsideBearing) :
		m_advanceWidth(advanceWidth),
		m_a(leftSideBearing),
		m_c(rightsideBearing)
	{
	}

	FontDimension advanceWidth() const { return m_advanceWidth; }
	FontDimension abcA() const { return m_a; }

	// Relationship between advanceWidth and B is ADV = A + B + C so B = ADV - A - C
	FontDimension abcB() const { return advanceWidth() - abcA() - abcC(); }
	FontDimension abcC() const { return m_c; }
};

//-------------------------------------------------
//  FontDimensionFactory class - simple way of
//  creating font dimensions
//-------------------------------------------------

class FontDimensionFactory
{
private:
	uint16_t m_designUnitsPerEm;
	float m_emSizeInDip;

public:
	float EmSizeInDip() const
	{
		return m_emSizeInDip;
	}

	FontDimensionFactory(uint16_t designUnitsPerEm, float emSizeInDip)
	{
		m_designUnitsPerEm = designUnitsPerEm;
		m_emSizeInDip = emSizeInDip;
	}

	FontDimension FromDip(float dip) const
	{
		float sizeInDesignUnits = (dip / m_emSizeInDip) * m_designUnitsPerEm;
		return FontDimension(m_designUnitsPerEm, m_emSizeInDip, sizeInDesignUnits);
	}

	FontDimension FromDesignUnit(float designUnits) const
	{
		return FontDimension(m_designUnitsPerEm, m_emSizeInDip, designUnits);
	}

	FontDimension FromPoint(float pointSize) const
	{
		float sizeInDip = pointSize * (4.0f / 3.0f);
		float sizeInDesignUnits = (sizeInDip / m_emSizeInDip) * m_designUnitsPerEm;
		return FontDimension(m_designUnitsPerEm, m_emSizeInDip, sizeInDesignUnits);
	}

	FontABCWidths CreateAbcWidths(float advanceWidth, float leftSideBearing, float rightSideBearing) const
	{
		return FontABCWidths(
			FromDesignUnit(advanceWidth),
			FromDesignUnit(leftSideBearing),
			FromDesignUnit(rightSideBearing));
	}
};

//-------------------------------------------------
//  font_open - attempt to "open" a handle to the
//  font with the given name
//-------------------------------------------------

class osd_font_dwrite : public osd_font
{
private:
	ComPtr<ID2D1Factory>            m_d2dfactory;
	ComPtr<IDWriteFactory>          m_dwriteFactory;
	ComPtr<IWICImagingFactory>      m_wicFactory;
	ComPtr<IDWriteFont>             m_font;
	float                           m_fontEmHeightInDips;

public:
	osd_font_dwrite(ComPtr<ID2D1Factory> d2dfactory, ComPtr<IDWriteFactory> dwriteFactory, ComPtr<IWICImagingFactory> wicFactory)
		: m_d2dfactory(d2dfactory), m_dwriteFactory(dwriteFactory), m_wicFactory(wicFactory), m_fontEmHeightInDips(0)
	{
	}

	virtual ~osd_font_dwrite() { osd_font_dwrite::close(); }

	virtual bool open(std::string const &font_path, std::string const &_name, int &height) override
	{
		if (m_d2dfactory == nullptr || m_dwriteFactory == nullptr || m_wicFactory == nullptr)
			return false;

		HRESULT result;

		// accept qualifiers from the name
		std::string name(_name);
		if (name.compare("default") == 0)
			name = "Segoe UI";
		bool bold = (strreplace(name, "[B]", "") + strreplace(name, "[b]", "") > 0);
		bool italic = (strreplace(name, "[I]", "") + strreplace(name, "[i]", "") > 0);

		// convert the face name
		std::wstring familyName = osd::text::to_wstring(name.c_str());

		// find the font
		HR_RET0(find_font(
			familyName,
			bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
			m_font.GetAddressOf()));

		DWRITE_FONT_METRICS metrics;
		m_font->GetMetrics(&metrics);

		m_fontEmHeightInDips = DEFAULT_EM_HEIGHT;
		height = static_cast<int>(round(m_fontEmHeightInDips * DIPS_PER_POINT));

		return true;
	}

	//-------------------------------------------------
	//  font_close - release resources associated with
	//  a given OSD font
	//-------------------------------------------------

	virtual void close() override
	{
		m_font = nullptr;
		m_fontEmHeightInDips = 0;
	}

	//-------------------------------------------------
	//  font_get_bitmap - allocate and populate a
	//  BITMAP_FORMAT_ARGB32 bitmap containing the
	//  pixel values rgb_t(0xff,0xff,0xff,0xff)
	//  or rgb_t(0x00,0xff,0xff,0xff) for each
	//  pixel of a black & white font
	//-------------------------------------------------

	virtual bool get_bitmap(char32_t chnum, bitmap_argb32 &bitmap, std::int32_t &width, std::int32_t &xoffs, std::int32_t &yoffs) override
	{
		const int MEM_ALIGN_CONST = 31;
		const int BITMAP_PAD = 50;

		HRESULT result;
		UINT cbData;
		BYTE* pixels = nullptr;

		ComPtr<ID2D1BitmapRenderTarget>     target;
		ComPtr<ID2D1SolidColorBrush>        pWhiteBrush;
		ComPtr<IWICBitmap>                  wicBitmap;
		ComPtr<IWICBitmapLock>              lock;

		ComPtr<IDWriteFontFace> face;
		HR_RET0(m_font->CreateFontFace(face.GetAddressOf()));

		// get the GDI metrics
		DWRITE_FONT_METRICS gdi_metrics;
		HR_RET0(face->GetGdiCompatibleMetrics(
			m_fontEmHeightInDips,
			1.0f,
			nullptr,
			&gdi_metrics));

		FontDimensionFactory fdf(gdi_metrics.designUnitsPerEm, m_fontEmHeightInDips);

		uint32_t tempChar = chnum;
		uint16_t glyphIndex;
		HR_RET0(face->GetGlyphIndices(&tempChar, 1, &glyphIndex));

		// get the width of this character
		DWRITE_GLYPH_METRICS glyph_metrics = { 0 };
		HR_RET0(face->GetGdiCompatibleGlyphMetrics(
			m_fontEmHeightInDips,
			1.0f,
			nullptr,
			FALSE,
			&glyphIndex,
			1,
			&glyph_metrics));

		// The height is the ascent added to the descent
		// By definition, the Em is equal to Cell Height minus Internal Leading (topSide bearing).
		//auto cellheight = fdf.FromDesignUnit(gdi_metrics.ascent + gdi_metrics.descent + gdi_metrics.);
		auto ascent = fdf.FromDesignUnit(gdi_metrics.ascent);
		auto descent = fdf.FromDesignUnit(gdi_metrics.descent);
		auto charHeight = ascent + descent;

		auto abc = fdf.CreateAbcWidths(
			glyph_metrics.advanceWidth,
			glyph_metrics.leftSideBearing,
			glyph_metrics.rightSideBearing);

		width = abc.abcA().Dips() + abc.abcB().Dips() + abc.abcC().Dips();

		// determine desired bitmap size
		int bmwidth = (BITMAP_PAD + abc.abcA().Dips() + abc.abcB().Dips() + abc.abcC().Dips() + BITMAP_PAD + MEM_ALIGN_CONST) & ~MEM_ALIGN_CONST;
		int bmheight = BITMAP_PAD + charHeight.Dips() + BITMAP_PAD;

		// GUID_WICPixelFormat8bppAlpha is 8 bits per pixel
		const REFWICPixelFormatGUID     source_bitmap_wic_format = GUID_WICPixelFormat8bppAlpha;
		const DXGI_FORMAT               source_bitmap_dxgi_format = DXGI_FORMAT_A8_UNORM;
		const D2D1_ALPHA_MODE           source_bitmap_d2d_alpha_mode = D2D1_ALPHA_MODE_STRAIGHT;

		// describe the bitmap we want
		HR_RET0(m_wicFactory->CreateBitmap(
			bmwidth,
			bmheight,
			source_bitmap_wic_format,
			WICBitmapCacheOnLoad,
			wicBitmap.GetAddressOf()));

		D2D1_RENDER_TARGET_PROPERTIES targetProps;
		targetProps = D2D1::RenderTargetProperties();
		targetProps.pixelFormat = D2D1::PixelFormat(source_bitmap_dxgi_format, source_bitmap_d2d_alpha_mode);

		// create a DIB to render to
		HR_RET0(this->m_d2dfactory->CreateWicBitmapRenderTarget(
			wicBitmap.Get(),
			&targetProps,
			reinterpret_cast<ID2D1RenderTarget**>(target.GetAddressOf())));

		target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);

		// Create our brush
		HR_RET0(target->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), pWhiteBrush.GetAddressOf()));

		// Signal the start of the frame
		target->BeginDraw();

		// clear the bitmap
		// In the alpha mask, it will look like 0x00 per pixel
		target->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));

		// now draw the character
		DWRITE_GLYPH_RUN run = { nullptr };
		DWRITE_GLYPH_OFFSET offsets;
		offsets.advanceOffset = 0;
		offsets.ascenderOffset = 0;
		float advanceWidth = abc.advanceWidth().Dips();

		run.fontEmSize = m_fontEmHeightInDips;
		run.fontFace = face.Get();
		run.glyphCount = 1;
		run.glyphIndices = &glyphIndex;
		run.glyphAdvances = &advanceWidth;
		run.glyphOffsets = &offsets;

		auto baseline_origin = D2D1::Point2F(BITMAP_PAD + abc.abcA().Dips() + 1, BITMAP_PAD + ascent.Dips());
		target->DrawGlyphRun(
			baseline_origin,
			&run,
			pWhiteBrush.Get(),
			DWRITE_MEASURING_MODE_GDI_CLASSIC);

		HR_RET0(target->EndDraw());

#ifdef DWRITE_DEBUGGING
		// Save to file for debugging
		SaveBitmap(wicBitmap.Get(), GUID_WICPixelFormatBlackWhite, L"C:\\temp\\ddraw_step1.bmp");
#endif

		// characters are expected to be full-height
		rectangle actbounds;
		actbounds.min_y = BITMAP_PAD;
		actbounds.max_y = BITMAP_PAD + charHeight.Dips() - 1;

		// Lock the bitmap and get the data pointer
		WICRect rect = { 0, 0, bmwidth, bmheight };
		HR_RET0(wicBitmap->Lock(&rect, WICBitmapLockRead, lock.GetAddressOf()));
		HR_RET0(lock->GetDataPointer(&cbData, static_cast<BYTE**>(&pixels)));

		// determine the actual left of the character
		for (actbounds.min_x = 0; actbounds.min_x < bmwidth; actbounds.min_x++)
		{
			BYTE *offs = pixels + actbounds.min_x;
			uint8_t summary = 0;
			for (int y = 0; y < bmheight; y++)
				summary |= offs[y * bmwidth];
			if (summary != 0)
			{
				break;
			}
		}

		// determine the actual right of the character
		// Start from the right edge, and move in until we find a pixel
		for (actbounds.max_x = bmwidth - 1; actbounds.max_x >= 0; actbounds.max_x--)
		{
			BYTE *offs = pixels + actbounds.max_x;
			uint8_t summary = 0;

			// Go through the entire column and build a summary
			for (int y = 0; y < bmheight; y++)
				summary |= offs[y * bmwidth];
			if (summary != 0)
			{
				break;
			}
		}

		// allocate a new bitmap
		if (actbounds.max_x >= actbounds.min_x && actbounds.max_y >= actbounds.min_y)
		{
			bitmap.allocate(actbounds.max_x + 1 - actbounds.min_x, actbounds.max_y + 1 - actbounds.min_y);

			// copy the bits into it
			for (int y = 0; y < bitmap.height(); y++)
			{
				uint32_t *dstrow = &bitmap.pix(y);
				uint8_t *srcrow = &pixels[(y + actbounds.min_y) * bmwidth];
				for (int x = 0; x < bitmap.width(); x++)
				{
					int effx = x + actbounds.min_x;
					dstrow[x] = rgb_t(srcrow[effx], 0xff, 0xff, 0xff);
				}
			}

			// set the final offset values
			xoffs = actbounds.min_x - (BITMAP_PAD + abc.abcA().Dips());
			yoffs = actbounds.max_y - (BITMAP_PAD + ascent.Dips());

#ifdef DWRITE_DEBUGGING
			SaveBitmap2(bitmap, L"C:\\temp\\dwrite_final.bmp");
#endif
		}

		BOOL success = bitmap.valid();

#ifdef DWRITE_DEBUGGING
		osd_printf_debug(
			"dwr: %s, c'%S' w%i x%i y%i asc%i dsc%i a%ib%ic%i\n",
			success ? "Success" : "Error",
			(WCHAR*)&chnum,
			width,
			xoffs,
			yoffs,
			ascent.Dips(),
			descent.Dips(),
			abc.abcA().Dips(),
			abc.abcB().Dips(),
			abc.abcC().Dips());
#endif
		return success;
	}

private:

	//-------------------------------------------------
	//  find_font - finds a font, given attributes
	//-------------------------------------------------

	HRESULT find_font(std::wstring familyName, DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STRETCH stretch, DWRITE_FONT_STYLE style, IDWriteFont ** ppfont) const
	{
		HRESULT result;

		ComPtr<IDWriteFontCollection> fonts;
		HR_RETHR(m_dwriteFactory->GetSystemFontCollection(fonts.GetAddressOf()));

		UINT family_index; BOOL exists;
		HR_RETHR(fonts->FindFamilyName(familyName.c_str(), &family_index, &exists));
		if (!exists)
		{
			osd_printf_error("Font with family name %s does not exist.\n", osd::text::from_wstring(familyName));
			return E_FAIL;
		}

		ComPtr<IDWriteFontFamily> family;
		HR_RETHR(fonts->GetFontFamily(family_index, family.GetAddressOf()));

		ComPtr<IDWriteFont> font;
		HR_RETHR(family->GetFirstMatchingFont(weight, stretch, style, font.GetAddressOf()));

		*ppfont = font.Detach();
		return result;
	}
};

//-------------------------------------------------
//  font_dwrite - the DirectWrite font module
//-------------------------------------------------

class font_dwrite : public osd_module, public font_module
{
private:
	OSD_DYNAMIC_API(dwrite, "dwrite.dll");
	OSD_DYNAMIC_API(d2d1, "d2d1.dll");
	OSD_DYNAMIC_API(locale, "kernel32.dll");
	OSD_DYNAMIC_API_FN(dwrite, HRESULT, WINAPI, DWriteCreateFactory, DWRITE_FACTORY_TYPE, REFIID, IUnknown **);
	OSD_DYNAMIC_API_FN(d2d1, HRESULT, WINAPI, D2D1CreateFactory, D2D1_FACTORY_TYPE, REFIID, const D2D1_FACTORY_OPTIONS *, void **);
	OSD_DYNAMIC_API_FN(locale, int, WINAPI, GetUserDefaultLocaleName, LPWSTR, int);
	ComPtr<ID2D1Factory>         m_d2dfactory;
	ComPtr<IDWriteFactory>       m_dwriteFactory;
	ComPtr<IWICImagingFactory>   m_wicFactory;

public:
	font_dwrite() :
		osd_module(OSD_FONT_PROVIDER, "dwrite"),
		font_module(),
		m_d2dfactory(nullptr),
		m_dwriteFactory(nullptr),
		m_wicFactory(nullptr)
	{
	}

	virtual bool probe() override
	{
		// This module is available if it can load the expected API Functions
		if (!OSD_DYNAMIC_API_TEST(D2D1CreateFactory) || !OSD_DYNAMIC_API_TEST(DWriteCreateFactory))
			return false;

		return true;
	}

	virtual int init(const osd_options &options) override
	{
		HRESULT result;

		osd_printf_verbose("FontProvider: Initializing DirectWrite\n");

		// Make sure we can initialize our api functions
		if (!OSD_DYNAMIC_API_TEST(D2D1CreateFactory) || !OSD_DYNAMIC_API_TEST(DWriteCreateFactory))
		{
			osd_printf_error("ERROR: FontProvider: Failed to load DirectWrite functions.\n");
			return -1;
		}

		assert(OSD_DYNAMIC_API_TEST(GetUserDefaultLocaleName));

		// Create a Direct2D factory.
		HR_RET1(OSD_DYNAMIC_CALL(D2D1CreateFactory,
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory),
			nullptr,
			reinterpret_cast<void**>(this->m_d2dfactory.GetAddressOf())));

		// Initialize COM
		CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

		// Create a DirectWrite factory.
		HR_RET1(OSD_DYNAMIC_CALL(DWriteCreateFactory,
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown **>(m_dwriteFactory.GetAddressOf())));

		HR_RET1(CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWICImagingFactory),
			static_cast<void**>(&m_wicFactory)));

		osd_printf_verbose("FontProvider: DirectWrite initialized successfully.\n");
		return 0;
	}

	virtual osd_font::ptr font_alloc() override
	{
		return std::make_unique<osd_font_dwrite>(m_d2dfactory, m_dwriteFactory, m_wicFactory);
	}

	virtual bool get_font_families(std::string const &font_path, std::vector<std::pair<std::string, std::string> > &fontresult) override
	{
		HRESULT result;
		ComPtr<IDWriteFontFamily> family;
		ComPtr<IDWriteLocalizedStrings> names;

		// For now, we're just enumerating system fonts, if we want to support custom font
		// collections, there's more work that neeeds to be done
		ComPtr<IDWriteFontCollection> fonts;
		HR_RET0(m_dwriteFactory->GetSystemFontCollection(fonts.GetAddressOf()));

		int family_count = fonts->GetFontFamilyCount();
		for (int i = 0; i < family_count; i++)
		{
			HR_RET0(fonts->GetFontFamily(i, family.ReleaseAndGetAddressOf()));

			HR_RET0(family->GetFamilyNames(names.ReleaseAndGetAddressOf()));

			std::unique_ptr<WCHAR[]> name = nullptr;
			HR_RET0(get_localized_familyname(names, name));

			std::string utf8_name = osd::text::from_wstring(name.get());
			name.reset();

			// Review: should the config name, be unlocalized?
			// maybe the english name?
			fontresult.emplace_back(make_pair(utf8_name, utf8_name));
		}

		std::stable_sort(fontresult.begin(), fontresult.end());
		return true;
	}

private:
	HRESULT get_family_for_locale(ComPtr<IDWriteLocalizedStrings> family_names, const std::wstring &locale, std::unique_ptr<WCHAR[]> &family_name) const
	{
		HRESULT result;
		uint32_t index;
		BOOL exists = false;

		result = family_names->FindLocaleName(locale.c_str(), &index, &exists);

		// if the above find did not find a match, retry with US English
		if (SUCCEEDED(result) && !exists)
			family_names->FindLocaleName(L"en-us", &index, &exists);

		// If the specified locale doesn't exist, select the first on the list.
		if (!exists)
			index = 0;

		// Get the length and allocate our buffer
		uint32_t name_length = 0;
		HR_RETHR(family_names->GetStringLength(index, &name_length));
		auto name_buffer = std::make_unique<WCHAR[]>(name_length + 1);

		// Get the name
		HR_RETHR(family_names->GetString(index, name_buffer.get(), name_length + 1));

		family_name = std::move(name_buffer);
		return S_OK;
	}

	HRESULT get_localized_familyname(ComPtr<IDWriteLocalizedStrings> family_names, std::unique_ptr<WCHAR[]> &family_name)
	{
		std::wstring locale_name;

		// Get the default locale for this user if possible.
		// GetUserDefaultLocaleName doesn't exist on XP, so don't assume.
		if (OSD_DYNAMIC_API_TEST(GetUserDefaultLocaleName))
		{
			wchar_t name_buffer[LOCALE_NAME_MAX_LENGTH];
			int len = OSD_DYNAMIC_CALL(GetUserDefaultLocaleName, name_buffer, LOCALE_NAME_MAX_LENGTH);
			if (len != 0)
				locale_name = name_buffer;
		}

		// If the default locale is returned, find that locale name
		if (!locale_name.empty())
			return get_family_for_locale(family_names, locale_name, family_name);

		// If locale can't be determined, fall back to US English
		return get_family_for_locale(family_names, L"en-us", family_name);
	}
};

#else
MODULE_NOT_SUPPORTED(font_dwrite, OSD_FONT_PROVIDER, "dwrite")
#endif

MODULE_DEFINITION(FONT_DWRITE, font_dwrite)
