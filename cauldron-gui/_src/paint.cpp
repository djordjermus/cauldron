#if defined(_WIN32) || defined(_WIN64)
#include "../paint.h"
#include "../control.h"
#include <Windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus")


namespace cauldron::gui {
	using namespace cauldron::common;



	static void initGdi();
	static paint::font::core_t* makeFont(
		cwstr family, 
		f32 size, 
		paint::font::style style);
	static paint::brush::core_t* makeSolidBrush(
		rgba8 color);
	static paint::image::core_t* loadImage(
		cwstr path);
	static paint::pen::core_t* makePen(
		const paint::brush& brush,
		f32 width);
	static paint::image::core_t* makeImage(
		u64 w,
		u64 h);
	static paint::brush::core_t* makeTextureBrush(
		const paint::image* image,
		const paint::bounds_t* bounds);
	static paint::core_t* makeGfxClient(
		control* window);
	static paint::core_t* makeGfxBitmap(
		paint::bitmap* bitmap);
	static Gdiplus::RectF makeRect(
		const paint::bounds_t& bounds);
	static Gdiplus::FontStyle convert(paint::font::style style);
	static paint::font::style convert(Gdiplus::FontStyle style);

#define FONT(x) reinterpret_cast<Gdiplus::Font*>((x).getCore())
#define IMAGE(x) reinterpret_cast<Gdiplus::Image*>((x).getCore())
#define BITMAP(x) reinterpret_cast<Gdiplus::Bitmap*>((x).getCore())
#define BRUSH(x) reinterpret_cast<Gdiplus::Brush*>((x).getCore())
#define SBRUSH(x) reinterpret_cast<Gdiplus::SolidBrush*>((x).getCore())
#define GBRUSH(x) reinterpret_cast<Gdiplus::LinearGradientBrush*>((x).getCore())
#define TBRUSH(x) reinterpret_cast<Gdiplus::TextureBrush*>((x).getCore())
#define PEN(x) reinterpret_cast<Gdiplus::Pen*>((x).getCore())
#define GFX(x) reinterpret_cast<Gdiplus::Graphics*>((x).getCore())
#undef near;
#undef far;



	//
	// FONT
	using font = paint::font;

	font::font() : _core(nullptr) {}
	font::font(cwstr family, f32 size, style style)
		: _core(makeFont(family, size, style)) {}
	font::font(core_t* core) : _core(core) {}
	font::~font() {
		if (FONT(*this))
			delete FONT(*this);
	}

	bool font::isValid() const {
		return _core != nullptr;
	}
	f32 font::getSize() const {
		return FONT(*this)->GetSize();
	}
	font::style font::getStyle() const {
		return convert((Gdiplus::FontStyle)FONT(*this)->GetStyle());
	}



	//
	// IMAGE
	using image = paint::image;
	image::image(cwstr path) : _core(loadImage(path)) {}
	image::image(core_t* core) : _core(core) { }
	image::~image() {
		if (IMAGE(*this)) delete IMAGE(*this);
	}
	image::image(image&& move) noexcept : _core(move._core) {
		move._core = nullptr;
	}
	void image::swap(image&& swap) noexcept {
		core_t* temp = _core;
		_core = swap._core;
		swap._core = temp;
	}

	u64 paint::image::getWidth() const {
		return IMAGE(*this)->GetWidth();
	}
	u64 paint::image::getHeight() const {
		return IMAGE(*this)->GetHeight();
	}

	using bitmap = paint::bitmap;
	bitmap::bitmap(cwstr path) : image(path) {}
	bitmap::bitmap(u64 width, u64 height) :
		image(makeImage(width, height)) {
	}
	bitmap::bitmap(bitmap&& move) noexcept :
		image(static_cast<image&&>(move)) {}
	void bitmap::swap(bitmap&& swap) noexcept {
		image::swap((image&&)swap);
	}



	//
	// BRUSH

	paint::brush::brush(core_t* core) : _core(core) {}
	paint::brush::~brush() {
		if (BRUSH(*this)) {
			delete BRUSH(*this);
			_core = nullptr;
		}
	}

	paint::solidBrush::solidBrush(rgba8 color) :
		brush(makeSolidBrush(color)) {}

	rgba8 paint::solidBrush::getColor() const {
		Gdiplus::Color col;
		SBRUSH(*this)->GetColor(&col);
		return rgba8(
			(col.GetValue() >> 16) & 0xFF,
			(col.GetValue() >> 8) & 0xFF,
			(col.GetValue() >> 0) & 0xFF,
			(col.GetValue() >> 24) & 0xFF);
	}
	void paint::solidBrush::setColor(rgba8 color) {
		SBRUSH(*this)->SetColor(color.value);
	}

	paint::textureBrush::textureBrush(const image& image, const bounds_t& bounds) :
		brush(makeTextureBrush(&image, &bounds)) {}

	

	//
	// PEN

	paint::pen::pen(const brush& brush, f32 width) :
		_core(makePen(brush, width)) {}
	paint::pen::~pen() {
		if (PEN(*this))
			delete PEN(*this);
	}

	void paint::pen::setBrush(const brush& brush) {
		PEN(*this)->SetBrush(BRUSH(brush));
	}
	f32 paint::pen::getWidth() const {
		return PEN(*this)->GetWidth();
	}

	void paint::pen::setWidth(f32 width) {
		PEN(*this)->SetWidth(width);
	}



	//
	// PAINT

	paint::paint() : _core(nullptr) {
		initGdi();
	}
	paint::paint(control& ctrl) :
		_core(makeGfxClient(&ctrl)) {}
	paint::paint(bitmap& bmp) :
		_core(makeGfxBitmap(&bmp)) {}
	paint::paint(core_t* core) :
		_core(core) {}
	paint::~paint() {
		if (isInitialized())
			terminate();
	}

	void paint::initialize(control& window) {
		if (isInitialized())
			throw;
		if (!window.isValid())
			throw;
		_core = makeGfxClient(&window);
	}
	void paint::initialize(bitmap& bitmap) {
		if (isInitialized())
			throw;
		_core = makeGfxBitmap(&bitmap);
	}
	void paint::terminate() {
		if (!isInitialized())
			throw;
		
		delete GFX(*this);
		_core = nullptr;
	}
	bool paint::isInitialized() const {
		return _core != nullptr;
	}

	void paint::setSmoothing(bool smoothing) {
		GFX(*this)->SetSmoothingMode(smoothing ? 
			Gdiplus::SmoothingModeAntiAlias : Gdiplus::SmoothingModeNone);
	}
	void paint::clear(rgba8 color) {
		GFX(*this)->Clear(color.value);
	}
	void paint::setTransform(f32 m11, f32 m12, f32 m21, f32 m22, f32 m31, f32 m32) {
		Gdiplus::Matrix mx(m11, m12, m21, m22, m31, m32);
		GFX(*this)->SetTransform(&mx);
	}
	void paint::clearTransform() {
		Gdiplus::Matrix mx(1, 0, 0, 1, 0, 0);
		GFX(*this)->SetTransform(&mx);
	}
	void paint::setClip(const bounds_t& clip) {
		GFX(*this)->ResetClip();
		GFX(*this)->SetClip(makeRect(clip), Gdiplus::CombineMode::CombineModeReplace);
	}
	void paint::clearClip() {
		GFX(*this)->ResetClip();
	}

	paint::bounds_t paint::measureWrite(
		cwstr text,
		i32 length,
		const bounds_t& bounds,
		const font& font,
		alignment horizontal,
		alignment vertical) {

		static Gdiplus::Bitmap g_bmp(1, 1);
		static Gdiplus::Graphics g_gfx(&g_bmp);

		Gdiplus::StringFormat f = Gdiplus::StringFormat::GenericTypographic();
		f.SetAlignment((Gdiplus::StringAlignment)(u64)horizontal);
		f.SetLineAlignment((Gdiplus::StringAlignment)(u64)vertical);

		Gdiplus::RectF output = {};
		g_gfx.MeasureString(
			text,
			length,
			FONT(font),
			makeRect(bounds), 
			&f,
			&output);
		return bounds_t({ output.X, output.Y }, { output.Width, output.Height });
	}
	i32 paint::pointToCaretIndex(
		cwstr text,
		i32 length,
		const bounds_t& bounds,
		const font& font,
		alignment horizontal,
		alignment vertical,
		const vector_t& point) {

		// Ready region and rect
		Gdiplus::Region region;
		Gdiplus::RectF rect(point.x, point.y, 2, 2);

		// Compose format
		Gdiplus::StringFormat f = Gdiplus::StringFormat::GenericTypographic();
		f.SetAlignment((Gdiplus::StringAlignment)(u64)horizontal);
		f.SetLineAlignment((Gdiplus::StringAlignment)(u64)vertical);

		Gdiplus::SolidBrush sb(Gdiplus::Color::Red);
		for (i32 i = 0; i <= length; i++) {
			Gdiplus::CharacterRange range(i, 1);
			f.SetMeasurableCharacterRanges(1, &range);

			GFX(*this)->MeasureCharacterRanges(
				text, length, FONT(font), makeRect(bounds), &f, length, &region);
			
			region.Intersect(rect);

			if (region.IsEmpty(GFX(*this)))
				continue;
			else 
				return i;
		}

		Gdiplus::RectF str_rect;
		GFX(*this)->MeasureString(
			text, length, FONT(font), makeRect(bounds), &f, &str_rect);

		if (point.x < str_rect.X)
			return 0;
		else if (point.x > str_rect.X + str_rect.Width)
			return length;
	}
	void paint::write(
		cwstr text,
		i32 length,
		const bounds_t& bounds,
		const font& font,
		const brush& brush,
		alignment horizontal,
		alignment vertical) {
		Gdiplus::StringFormat f = Gdiplus::StringFormat::GenericTypographic();
		f.SetAlignment((Gdiplus::StringAlignment)(u64)horizontal);
		f.SetLineAlignment((Gdiplus::StringAlignment)(u64)vertical);
		GFX(*this)->DrawString(
			text, 
			length, 
			FONT(font),
			makeRect(bounds),
			&f,
			BRUSH(brush));
	}


	
	void paint::drawImage(
		const bounds_t& bounds,
		const image& image) {
		GFX(*this)->DrawImage(IMAGE(image), makeRect(bounds));
	}



	void paint::drawRect(
		const bounds_t& bounds,
		const pen& pen) {
		GFX(*this)->DrawRectangle(
			PEN(pen),
			makeRect(bounds));
	}
	void paint::drawEllipse(
		const bounds_t& bounds,
		const pen& pen) {
		GFX(*this)->DrawEllipse(
			PEN(pen),
			makeRect(bounds));
	}
	void paint::drawLine(
		const vector_t& from,
		const vector_t& to,
		const pen& pen) {
		GFX(*this)->DrawLine(
			PEN(pen),
			(Gdiplus::PointF&)from,
			(Gdiplus::PointF&)to);
	}
	void paint::drawArc(
		const bounds_t& bounds,
		const vector_t angleSweep,
		const pen& pen) {
		GFX(*this)->DrawArc(
			PEN(pen),
			makeRect(bounds), 
			(angleSweep.x / 3.14159F) * 180.0F,
			(angleSweep.y / 3.14159F) * 180.0F);
	}
	void paint::drawPoly(
		const vector_t* points,
		u64 count,
		const pen& pen) {
		GFX(*this)->DrawPolygon(
			PEN(pen),
			(Gdiplus::PointF*)points,
			count);
	}
	void paint::drawCurve(
		const vector_t* points,
		u64 count,
		const pen& pen) {
		GFX(*this)->DrawCurve(PEN(pen), (Gdiplus::PointF*)points, count);

	}
	void paint::drawBezier(
		const vector_t& from,
		const vector_t& control1,
		const vector_t& control2,
		const vector_t& to,
		const pen& pen) {
		GFX(*this)->DrawBezier(
			PEN(pen),
			(Gdiplus::PointF&)from,
			(Gdiplus::PointF&)control1,
			(Gdiplus::PointF&)control2,
			(Gdiplus::PointF&)to);
	}
	void paint::fillRect(
		const bounds_t& bounds,
		const brush& brush) {
		GFX(*this)->FillRectangle(
			BRUSH(brush),
			makeRect(bounds));
	}
	void paint::fillEllipse(
		const bounds_t& bounds,
		const brush& brush) {
		GFX(*this)->FillEllipse(
			BRUSH(brush),
			makeRect(bounds));
	}
	void paint::fillArc(
		const bounds_t& bounds,
		const vector_t angleSweep,
		const brush& brush) {
		GFX(*this)->FillPie(
			BRUSH(brush),
			makeRect(bounds),
			(angleSweep.x / 3.14159F) * 180.0F,
			(angleSweep.y / 3.14159F) * 180.0F);
	}
	void paint::fillPoly(
		const vector_t* points,
		u64 count,
		const brush& brush) {
		GFX(*this)->FillPolygon(
			BRUSH(brush), 
			(Gdiplus::PointF*)points, 
			count, 
			Gdiplus::FillMode::FillModeAlternate);
	}



	static void initGdi() {
		static bool initialized = false;
		static ULONG_PTR ptr = 0;
		static Gdiplus::GdiplusStartupInput gdiplusStartupInput = nullptr;

		if (initialized) return;
		initialized = true;
		auto status = Gdiplus::GdiplusStartup(&ptr, &gdiplusStartupInput, nullptr);
	}
	static paint::font::core_t* makeFont(cwstr family, f32 size, paint::font::style style) {
		initGdi();
		auto* ret = new Gdiplus::Font(
			family,
			size,
			convert(style),
			Gdiplus::Unit::UnitPixel);
		return (paint::font::core_t*)ret;
	}
	static paint::image::core_t* loadImage(cwstr path) {
		initGdi();
		auto* ret = new Gdiplus::Image(path);
		return (paint::image::core_t*)ret;
	}
	static paint::image::core_t* makeImage(u64 w, u64 h) {
		initGdi();
		auto* ret = new Gdiplus::Bitmap(w, h);
		return (paint::image::core_t*)ret;
	}
	static paint::brush::core_t* makeSolidBrush(rgba8 color) {
		initGdi();
		auto* ret = new Gdiplus::SolidBrush(color.value);
		return (paint::brush::core_t*)ret;
	}
	static paint::brush::core_t* makeTextureBrush(
		const paint::image* image,
		const paint::bounds_t* bounds) {
		initGdi();
		auto* ret = new Gdiplus::TextureBrush((Gdiplus::Image*)image->getCore(), makeRect(*bounds));
		return (paint::brush::core_t*)ret;
	}
	static paint::pen::core_t* makePen(const paint::brush& brush, f32 width) {
		initGdi();
		auto* ret = new Gdiplus::Pen(BRUSH(brush), width);
		return (paint::pen::core_t*)ret;
	}
	static paint::core_t* makeGfxClient(control* window) {
		initGdi();
		auto* ret = new Gdiplus::Graphics(GetDC((HWND)window->getCore()));
		return (paint::core_t*)ret;
	}
	static paint::core_t* makeGfxBitmap(paint::bitmap* bitmap) {
		initGdi();
		auto* ret = new Gdiplus::Graphics(reinterpret_cast<Gdiplus::Bitmap*>(bitmap->getCore()));
		return (paint::core_t*)ret;
	}
	static Gdiplus::RectF makeRect(const paint::bounds_t& bounds) {
		return Gdiplus::RectF(
			bounds.from.x,
			bounds.from.y,
			bounds.to.x - bounds.from.x,
			bounds.to.y - bounds.from.y);
	}
	static Gdiplus::FontStyle convert(paint::font::style style) {
		Gdiplus::FontStyle ret = Gdiplus::FontStyleRegular;

		if ((style & paint::font::style::bold) == paint::font::style::bold)
			ret = (Gdiplus::FontStyle)(ret | Gdiplus::FontStyleBold);
		if ((style & paint::font::style::italic) == paint::font::style::italic)
			ret = (Gdiplus::FontStyle)(ret | Gdiplus::FontStyleItalic);
		if ((style & paint::font::style::strikeout) == paint::font::style::strikeout)
			ret = (Gdiplus::FontStyle)(ret | Gdiplus::FontStyleStrikeout);
		if ((style & paint::font::style::underline) == paint::font::style::underline)
			ret = (Gdiplus::FontStyle)(ret | Gdiplus::FontStyleUnderline);

		return ret;
	}
	static paint::font::style convert(Gdiplus::FontStyle style) {
		paint::font::style ret = paint::font::style::normal;

		if ((style & Gdiplus::FontStyleBold) == Gdiplus::FontStyleBold)
			ret = ret | paint::font::style::bold;
		if ((style & Gdiplus::FontStyleItalic) == Gdiplus::FontStyleItalic)
			ret = ret | paint::font::style::italic;
		if ((style & Gdiplus::FontStyleStrikeout) == Gdiplus::FontStyleStrikeout)
			ret = ret | paint::font::style::strikeout;
		if ((style & Gdiplus::FontStyleUnderline) == Gdiplus::FontStyleUnderline)
			ret = ret | paint::font::style::underline;

		return ret;
	}
}

#endif