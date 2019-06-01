#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_ERRORS_H

#include <emscripten/bind.h>

#define FUNCTION(RET, ARGS, CODE...) \
  emscripten::optional_override([] ARGS -> RET { CODE })

#include <malloc.h>

emscripten::val get_mallinfo() {
  const auto& i = mallinfo();
  emscripten::val rv(emscripten::val::object());
  rv.set("arena", emscripten::val(i.arena));
  rv.set("ordblks", emscripten::val(i.ordblks));
  rv.set("smblks", emscripten::val(i.smblks));
  rv.set("hblks", emscripten::val(i.hblks));
  rv.set("hblkhd", emscripten::val(i.hblkhd));
  rv.set("usmblks", emscripten::val(i.usmblks));
  rv.set("fsmblks", emscripten::val(i.fsmblks));
  rv.set("uordblks", emscripten::val(i.uordblks));
  rv.set("fordblks", emscripten::val(i.fordblks));
  rv.set("keepcost", emscripten::val(i.keepcost));
  return rv;
}

EMSCRIPTEN_BINDINGS(mallinfo) {
  emscripten::function("mallinfo", &get_mallinfo);
}

EMSCRIPTEN_BINDINGS(FT_VERSION) {
  emscripten::constant("FREETYPE_MAJOR", FREETYPE_MAJOR);
  emscripten::constant("FREETYPE_MINOR", FREETYPE_MINOR);
  emscripten::constant("FREETYPE_PATCH", FREETYPE_PATCH);
}

// fterrors.h

static const char* _FT_Error_String(FT_Error error_code) {
#undef FTERRORS_H_
#define FT_ERROR_START_LIST     switch ( FT_ERROR_BASE( error_code ) ) {
#define FT_ERRORDEF( e, v, s )    case v: return s;
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
  return NULL;
}

EMSCRIPTEN_BINDINGS(fterrors) {
  emscripten::function("FT_Error_String", FUNCTION(std::string, (FT_Error error_code), {
    return std::string(_FT_Error_String(error_code));
  }));
}

// ftimage.h

// typedef signed long FT_Pos;

// typedef struct FT_Vector_
EMSCRIPTEN_BINDINGS(FT_Vector) {
  emscripten::value_object<FT_Vector>("FT_Vector")
    // FT_Pos  x;
    .field("x", &FT_Vector::x)
    // FT_Pos  y;
    .field("y", &FT_Vector::y)
  ;
}

// typedef struct FT_BBox_
EMSCRIPTEN_BINDINGS(FT_BBox) {
  emscripten::value_object<FT_BBox>("FT_BBox")
    // FT_Pos  xMin, yMin;
    .field("xMin", &FT_BBox::xMin)
    .field("yMin", &FT_BBox::yMin)
    // FT_Pos  xMax, yMax;
    .field("xMax", &FT_BBox::xMax)
    .field("yMax", &FT_BBox::yMax)
  ;
}

// typedef enum  FT_Pixel_Mode_
//     FT_PIXEL_MODE_NONE = 0,
//     FT_PIXEL_MODE_MONO,
//     FT_PIXEL_MODE_GRAY,
//     FT_PIXEL_MODE_GRAY2,
//     FT_PIXEL_MODE_GRAY4,
//     FT_PIXEL_MODE_LCD,
//     FT_PIXEL_MODE_LCD_V,
//     FT_PIXEL_MODE_BGRA,
//     FT_PIXEL_MODE_MAX      /* do not remove */
//   } FT_Pixel_Mode;

// /* these constants are deprecated; use the corresponding `FT_Pixel_Mode` */
// #define ft_pixel_mode_none   FT_PIXEL_MODE_NONE
// #define ft_pixel_mode_mono   FT_PIXEL_MODE_MONO
// #define ft_pixel_mode_grays  FT_PIXEL_MODE_GRAY
// #define ft_pixel_mode_pal2   FT_PIXEL_MODE_GRAY2
// #define ft_pixel_mode_pal4   FT_PIXEL_MODE_GRAY4

// typedef struct FT_Bitmap_
EMSCRIPTEN_BINDINGS(FT_Bitmap) {
  emscripten::class_<FT_Bitmap>("FT_Bitmap")
    // unsigned int    rows;
    .property("rows", &FT_Bitmap::rows)
    // unsigned int    width;
    .property("width", &FT_Bitmap::width)
    // int             pitch;
    .property("pitch", &FT_Bitmap::pitch)
    // unsigned char*  buffer;
    .property("buffer", FUNCTION(emscripten::val, (const FT_Bitmap& that), {
      if (that.buffer == NULL) { return emscripten::val::null(); }
      size_t length = that.rows * (that.pitch < 0 ? -that.pitch : that.pitch);
      return emscripten::val(emscripten::typed_memory_view<unsigned char>(length, that.buffer));
    }))
    // unsigned short  num_grays;
    .property("num_grays", &FT_Bitmap::num_grays)
    // unsigned char   pixel_mode;
    .property("pixel_mode", &FT_Bitmap::pixel_mode)
    // unsigned char   palette_mode;
    .property("palette_mode", &FT_Bitmap::palette_mode)
    // void*           palette;
    .property("palette", FUNCTION(emscripten::val, (const FT_Bitmap& that), {
      return emscripten::val::null();
    }))
  ;
}

// typedef struct FT_Outline_
EMSCRIPTEN_BINDINGS(FT_Outline) {
  emscripten::class_<FT_Outline>("FT_Outline")
    // short       n_contours;      /* number of contours in glyph        */
    // short       n_points;        /* number of points in the glyph      */

    // FT_Vector*  points;          /* the outline's points               */
    // char*       tags;            /* the points flags                   */
    // short*      contours;        /* the contour end points             */

    // int         flags;           /* outline masks                      */
  ;
}

// #define FT_OUTLINE_CONTOURS_MAX  SHRT_MAX
// #define FT_OUTLINE_POINTS_MAX    SHRT_MAX
// #define FT_OUTLINE_NONE             0x0
// #define FT_OUTLINE_OWNER            0x1
// #define FT_OUTLINE_EVEN_ODD_FILL    0x2
// #define FT_OUTLINE_REVERSE_FILL     0x4
// #define FT_OUTLINE_IGNORE_DROPOUTS  0x8
// #define FT_OUTLINE_SMART_DROPOUTS   0x10
// #define FT_OUTLINE_INCLUDE_STUBS    0x20
// #define FT_OUTLINE_HIGH_PRECISION   0x100
// #define FT_OUTLINE_SINGLE_PASS      0x200

// #define ft_outline_none             FT_OUTLINE_NONE
// #define ft_outline_owner            FT_OUTLINE_OWNER
// #define ft_outline_even_odd_fill    FT_OUTLINE_EVEN_ODD_FILL
// #define ft_outline_reverse_fill     FT_OUTLINE_REVERSE_FILL
// #define ft_outline_ignore_dropouts  FT_OUTLINE_IGNORE_DROPOUTS
// #define ft_outline_high_precision   FT_OUTLINE_HIGH_PRECISION
// #define ft_outline_single_pass      FT_OUTLINE_SINGLE_PASS

// #define FT_CURVE_TAG( flag )  ( flag & 0x03 )
// #define FT_CURVE_TAG_ON            0x01
// #define FT_CURVE_TAG_CONIC         0x00
// #define FT_CURVE_TAG_CUBIC         0x02
// #define FT_CURVE_TAG_HAS_SCANMODE  0x04
// #define FT_CURVE_TAG_TOUCH_X       0x08  /* reserved for TrueType hinter */
// #define FT_CURVE_TAG_TOUCH_Y       0x10  /* reserved for TrueType hinter */
// #define FT_CURVE_TAG_TOUCH_BOTH    ( FT_CURVE_TAG_TOUCH_X | FT_CURVE_TAG_TOUCH_Y )

// #define FT_Curve_Tag_On       FT_CURVE_TAG_ON
// #define FT_Curve_Tag_Conic    FT_CURVE_TAG_CONIC
// #define FT_Curve_Tag_Cubic    FT_CURVE_TAG_CUBIC
// #define FT_Curve_Tag_Touch_X  FT_CURVE_TAG_TOUCH_X
// #define FT_Curve_Tag_Touch_Y  FT_CURVE_TAG_TOUCH_Y

//   (*FT_Outline_MoveToFunc)( const FT_Vector*  to,
// #define FT_Outline_MoveTo_Func  FT_Outline_MoveToFunc

//   (*FT_Outline_LineToFunc)( const FT_Vector*  to,
// #define FT_Outline_LineTo_Func  FT_Outline_LineToFunc

//   (*FT_Outline_ConicToFunc)( const FT_Vector*  control,
//                              const FT_Vector*  to,
// #define FT_Outline_ConicTo_Func  FT_Outline_ConicToFunc

//   (*FT_Outline_CubicToFunc)( const FT_Vector*  control1,
//                              const FT_Vector*  control2,
//                              const FT_Vector*  to,
// #define FT_Outline_CubicTo_Func  FT_Outline_CubicToFunc

// typedef struct FT_Outline_Funcs_
//     FT_Outline_MoveToFunc   move_to;
//     FT_Outline_LineToFunc   line_to;
//     FT_Outline_ConicToFunc  conic_to;
//     FT_Outline_CubicToFunc  cubic_to;
//     FT_Pos                  delta;
//   } FT_Outline_Funcs;

// #ifndef FT_IMAGE_TAG
// #define FT_IMAGE_TAG( value, _x1, _x2, _x3, _x4 )  \
// #endif /* FT_IMAGE_TAG */

// typedef enum  FT_Glyph_Format_
//     FT_IMAGE_TAG( FT_GLYPH_FORMAT_NONE, 0, 0, 0, 0 ),
//     FT_IMAGE_TAG( FT_GLYPH_FORMAT_COMPOSITE, 'c', 'o', 'm', 'p' ),
//     FT_IMAGE_TAG( FT_GLYPH_FORMAT_BITMAP,    'b', 'i', 't', 's' ),
//     FT_IMAGE_TAG( FT_GLYPH_FORMAT_OUTLINE,   'o', 'u', 't', 'l' ),
//     FT_IMAGE_TAG( FT_GLYPH_FORMAT_PLOTTER,   'p', 'l', 'o', 't' )
//   } FT_Glyph_Format;

// /* `FT_Glyph_Format` values instead.                     */
// #define ft_glyph_format_none       FT_GLYPH_FORMAT_NONE
// #define ft_glyph_format_composite  FT_GLYPH_FORMAT_COMPOSITE
// #define ft_glyph_format_bitmap     FT_GLYPH_FORMAT_BITMAP
// #define ft_glyph_format_outline    FT_GLYPH_FORMAT_OUTLINE
// #define ft_glyph_format_plotter    FT_GLYPH_FORMAT_PLOTTER

// typedef struct FT_RasterRec_*  FT_Raster;
// typedef struct FT_Span_
//     short           x;
//     unsigned short  len;
//     unsigned char   coverage;
//   } FT_Span;

//   (*FT_SpanFunc)( int             y,
//                   const FT_Span*  spans,
// #define FT_Raster_Span_Func  FT_SpanFunc

//   (*FT_Raster_BitTest_Func)( int    y,

//   (*FT_Raster_BitSet_Func)( int    y,

// #define FT_RASTER_FLAG_DEFAULT  0x0
// #define FT_RASTER_FLAG_AA       0x1
// #define FT_RASTER_FLAG_DIRECT   0x2
// #define FT_RASTER_FLAG_CLIP     0x4

// /* `FT_RASTER_FLAG_XXX` values instead                   */
// #define ft_raster_flag_default  FT_RASTER_FLAG_DEFAULT
// #define ft_raster_flag_aa       FT_RASTER_FLAG_AA
// #define ft_raster_flag_direct   FT_RASTER_FLAG_DIRECT
// #define ft_raster_flag_clip     FT_RASTER_FLAG_CLIP

// typedef struct FT_Raster_Params_
//     const FT_Bitmap*        target;
//     FT_SpanFunc             gray_spans;
//     FT_SpanFunc             black_spans;  /* unused */
//     FT_Raster_BitTest_Func  bit_test;     /* unused */
//     FT_Raster_BitSet_Func   bit_set;      /* unused */
//     FT_BBox                 clip_box;
//   } FT_Raster_Params;

//   (*FT_Raster_NewFunc)( void*       memory,
//                         FT_Raster*  raster );
// #define FT_Raster_New_Func  FT_Raster_NewFunc

//   (*FT_Raster_DoneFunc)( FT_Raster  raster );
// #define FT_Raster_Done_Func  FT_Raster_DoneFunc

//   (*FT_Raster_ResetFunc)( FT_Raster       raster,
// #define FT_Raster_Reset_Func  FT_Raster_ResetFunc

//   (*FT_Raster_SetModeFunc)( FT_Raster      raster,
// #define FT_Raster_Set_Mode_Func  FT_Raster_SetModeFunc

//   (*FT_Raster_RenderFunc)( FT_Raster                raster,
//                            const FT_Raster_Params*  params );
// #define FT_Raster_Render_Func  FT_Raster_RenderFunc

// typedef struct FT_Raster_Funcs_
//     FT_Glyph_Format        glyph_format;
//     FT_Raster_NewFunc      raster_new;
//     FT_Raster_ResetFunc    raster_reset;
//     FT_Raster_SetModeFunc  raster_set_mode;
//     FT_Raster_RenderFunc   raster_render;
//     FT_Raster_DoneFunc     raster_done;
//   } FT_Raster_Funcs;

// freetype.h

// typedef struct FT_Glyph_Metrics_
EMSCRIPTEN_BINDINGS(FT_Glyph_Metrics) {
  emscripten::value_object<FT_Glyph_Metrics>("FT_Glyph_Metrics")
    // FT_Pos  width;
    // FT_Pos  height;
    .field("width", &FT_Glyph_Metrics::width)
    .field("height", &FT_Glyph_Metrics::height)

    // FT_Pos  horiBearingX;
    // FT_Pos  horiBearingY;
    // FT_Pos  horiAdvance;
    .field("horiBearingX", &FT_Glyph_Metrics::horiBearingX)
    .field("horiBearingY", &FT_Glyph_Metrics::horiBearingY)
    .field("horiAdvance", &FT_Glyph_Metrics::horiAdvance)

    // FT_Pos  vertBearingX;
    // FT_Pos  vertBearingY;
    // FT_Pos  vertAdvance;
    .field("vertBearingX", &FT_Glyph_Metrics::vertBearingX)
    .field("vertBearingY", &FT_Glyph_Metrics::vertBearingY)
    .field("vertAdvance", &FT_Glyph_Metrics::vertAdvance)
  ;
}

// typedef struct FT_Bitmap_Size_
//   FT_Short  height;
//   FT_Short  width;
//   FT_Pos    size;
//   FT_Pos    x_ppem;
//   FT_Pos    y_ppem;
// } FT_Bitmap_Size;

// typedef struct FT_LibraryRec_*  FT_Library;
// typedef struct FT_ModuleRec_*  FT_Module;
// typedef struct FT_DriverRec_*  FT_Driver;
// typedef struct FT_RendererRec_*  FT_Renderer;
// typedef struct FT_FaceRec_*  FT_Face;
// typedef struct FT_SizeRec_*  FT_Size;
// typedef struct FT_GlyphSlotRec_*  FT_GlyphSlot;
// typedef struct FT_CharMapRec_*  FT_CharMap;

// #ifndef FT_ENC_TAG
// #define FT_ENC_TAG( value, a, b, c, d )         \
//         value = ( ( (FT_UInt32)(a) << 24 ) |  \
//                   ( (FT_UInt32)(b) << 16 ) |  \
//                   ( (FT_UInt32)(c) <<  8 ) |  \
//                     (FT_UInt32)(d)         )
// #endif /* FT_ENC_TAG */

// typedef enum  FT_Encoding_
//   FT_ENC_TAG( FT_ENCODING_NONE, 0, 0, 0, 0 ),
//   FT_ENC_TAG( FT_ENCODING_MS_SYMBOL, 's', 'y', 'm', 'b' ),
//   FT_ENC_TAG( FT_ENCODING_UNICODE,   'u', 'n', 'i', 'c' ),
//   FT_ENC_TAG( FT_ENCODING_SJIS,    's', 'j', 'i', 's' ),
//   FT_ENC_TAG( FT_ENCODING_PRC,     'g', 'b', ' ', ' ' ),
//   FT_ENC_TAG( FT_ENCODING_BIG5,    'b', 'i', 'g', '5' ),
//   FT_ENC_TAG( FT_ENCODING_WANSUNG, 'w', 'a', 'n', 's' ),
//   FT_ENC_TAG( FT_ENCODING_JOHAB,   'j', 'o', 'h', 'a' ),
//   FT_ENCODING_GB2312     = FT_ENCODING_PRC,
//   FT_ENCODING_MS_SJIS    = FT_ENCODING_SJIS,
//   FT_ENCODING_MS_GB2312  = FT_ENCODING_PRC,
//   FT_ENCODING_MS_BIG5    = FT_ENCODING_BIG5,
//   FT_ENCODING_MS_WANSUNG = FT_ENCODING_WANSUNG,
//   FT_ENCODING_MS_JOHAB   = FT_ENCODING_JOHAB,
//   FT_ENC_TAG( FT_ENCODING_ADOBE_STANDARD, 'A', 'D', 'O', 'B' ),
//   FT_ENC_TAG( FT_ENCODING_ADOBE_EXPERT,   'A', 'D', 'B', 'E' ),
//   FT_ENC_TAG( FT_ENCODING_ADOBE_CUSTOM,   'A', 'D', 'B', 'C' ),
//   FT_ENC_TAG( FT_ENCODING_ADOBE_LATIN_1,  'l', 'a', 't', '1' ),
//   FT_ENC_TAG( FT_ENCODING_OLD_LATIN_2, 'l', 'a', 't', '2' ),
//   FT_ENC_TAG( FT_ENCODING_APPLE_ROMAN, 'a', 'r', 'm', 'n' )
// } FT_Encoding;

// /* these constants are deprecated; use the corresponding `FT_Encoding` */
// #define ft_encoding_none            FT_ENCODING_NONE
// #define ft_encoding_unicode         FT_ENCODING_UNICODE
// #define ft_encoding_symbol          FT_ENCODING_MS_SYMBOL
// #define ft_encoding_latin_1         FT_ENCODING_ADOBE_LATIN_1
// #define ft_encoding_latin_2         FT_ENCODING_OLD_LATIN_2
// #define ft_encoding_sjis            FT_ENCODING_SJIS
// #define ft_encoding_gb2312          FT_ENCODING_PRC
// #define ft_encoding_big5            FT_ENCODING_BIG5
// #define ft_encoding_wansung         FT_ENCODING_WANSUNG
// #define ft_encoding_johab           FT_ENCODING_JOHAB
// #define ft_encoding_adobe_standard  FT_ENCODING_ADOBE_STANDARD
// #define ft_encoding_adobe_expert    FT_ENCODING_ADOBE_EXPERT
// #define ft_encoding_adobe_custom    FT_ENCODING_ADOBE_CUSTOM
// #define ft_encoding_apple_roman     FT_ENCODING_APPLE_ROMAN

// typedef struct FT_CharMapRec_
//   FT_Face      face;
//   FT_Encoding  encoding;
//   FT_UShort    platform_id;
//   FT_UShort    encoding_id;
// } FT_CharMapRec;

// typedef struct FT_Face_InternalRec_*  FT_Face_Internal;

// typedef struct FT_FaceRec_
EMSCRIPTEN_BINDINGS(FT_FaceRec) {
  emscripten::class_<FT_FaceRec>("FT_FaceRec")
    // FT_Long           num_faces;
    .property("num_faces", &FT_FaceRec::num_faces)
    // FT_Long           face_index;
    .property("face_index", &FT_FaceRec::face_index)

    // FT_Long           face_flags;
    .property("face_flags", &FT_FaceRec::face_flags)
    // FT_Long           style_flags;
    .property("style_flags", &FT_FaceRec::style_flags)

    // FT_Long           num_glyphs;
    .property("num_glyphs", &FT_FaceRec::num_glyphs)

    // FT_String*        family_name;
    .property("family_name", FUNCTION(std::string, (const FT_FaceRec& that), { return std::string(that.family_name); }))
    // FT_String*        style_name;
    .property("style_name", FUNCTION(std::string, (const FT_FaceRec& that), { return std::string(that.style_name); }))

    // FT_Int            num_fixed_sizes;
    // FT_Bitmap_Size*   available_sizes;

    // FT_Int            num_charmaps;
    // FT_CharMap*       charmaps;

    // FT_Generic        generic;

    // /*# The following member variables (down to `underline_thickness`) */
    // /*# are only relevant to scalable outlines; cf. @FT_Bitmap_Size    */
    // /*# for bitmap fonts.                                              */
    // FT_BBox           bbox;
    .property("bbox", FUNCTION(FT_BBox, (const FT_FaceRec& that), { return that.bbox; }))

    // FT_UShort         units_per_EM;
    .property("units_per_EM", &FT_FaceRec::units_per_EM)
    // FT_Short          ascender;
    .property("ascender", &FT_FaceRec::ascender)
    // FT_Short          descender;
    .property("descender", &FT_FaceRec::descender)
    // FT_Short          height;
    .property("height", &FT_FaceRec::height)

    // FT_Short          max_advance_width;
    .property("max_advance_width", &FT_FaceRec::max_advance_width)
    // FT_Short          max_advance_height;
    .property("max_advance_height", &FT_FaceRec::max_advance_height)

    // FT_Short          underline_position;
    .property("underline_position", &FT_FaceRec::underline_position)
    // FT_Short          underline_thickness;
    .property("underline_thickness", &FT_FaceRec::underline_thickness)

    // FT_GlyphSlot      glyph;
    .property("glyph", FUNCTION(emscripten::val, (const FT_FaceRec& that), {
      FT_GlyphSlotRec* p = (FT_GlyphSlotRec*) (intptr_t) that.glyph; return emscripten::val(p);
    }))
    // FT_Size           size;
    // FT_CharMap        charmap;

    // /*@private begin */

    // FT_Driver         driver;
    // FT_Memory         memory;
    // FT_Stream         stream;

    // FT_ListRec        sizes_list;

    // FT_Generic        autohint;   /* face-specific auto-hinter data */
    // void*             extensions; /* unused                         */

    // FT_Face_Internal  internal;

    // /*@private end */
  ;
}

// #define FT_FACE_FLAG_SCALABLE          ( 1L <<  0 )
// #define FT_FACE_FLAG_FIXED_SIZES       ( 1L <<  1 )
// #define FT_FACE_FLAG_FIXED_WIDTH       ( 1L <<  2 )
// #define FT_FACE_FLAG_SFNT              ( 1L <<  3 )
// #define FT_FACE_FLAG_HORIZONTAL        ( 1L <<  4 )
// #define FT_FACE_FLAG_VERTICAL          ( 1L <<  5 )
// #define FT_FACE_FLAG_KERNING           ( 1L <<  6 )
// #define FT_FACE_FLAG_FAST_GLYPHS       ( 1L <<  7 )
// #define FT_FACE_FLAG_MULTIPLE_MASTERS  ( 1L <<  8 )
// #define FT_FACE_FLAG_GLYPH_NAMES       ( 1L <<  9 )
// #define FT_FACE_FLAG_EXTERNAL_STREAM   ( 1L << 10 )
// #define FT_FACE_FLAG_HINTER            ( 1L << 11 )
// #define FT_FACE_FLAG_CID_KEYED         ( 1L << 12 )
// #define FT_FACE_FLAG_TRICKY            ( 1L << 13 )
// #define FT_FACE_FLAG_COLOR             ( 1L << 14 )
// #define FT_FACE_FLAG_VARIATION         ( 1L << 15 )

// #define FT_HAS_HORIZONTAL( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_HORIZONTAL )
// #define FT_HAS_VERTICAL( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_VERTICAL )
// #define FT_HAS_KERNING( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_KERNING )
// #define FT_IS_SCALABLE( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_SCALABLE )
// #define FT_IS_SFNT( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_SFNT )
// #define FT_IS_FIXED_WIDTH( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_FIXED_WIDTH )
// #define FT_HAS_FIXED_SIZES( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_FIXED_SIZES )
// #define FT_HAS_FAST_GLYPHS( face )  0
// #define FT_HAS_GLYPH_NAMES( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_GLYPH_NAMES )
// #define FT_HAS_MULTIPLE_MASTERS( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS )
// #define FT_IS_NAMED_INSTANCE( face ) \
// #define FT_IS_VARIATION( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_VARIATION )
// #define FT_IS_CID_KEYED( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_CID_KEYED )
// #define FT_IS_TRICKY( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_TRICKY )
// #define FT_HAS_COLOR( face ) \
//         ( (face)->face_flags & FT_FACE_FLAG_COLOR )

// #define FT_STYLE_FLAG_ITALIC  ( 1 << 0 )
// #define FT_STYLE_FLAG_BOLD    ( 1 << 1 )

// typedef struct FT_Size_InternalRec_*  FT_Size_Internal;

// typedef struct FT_Size_Metrics_
//   FT_UShort  x_ppem;      /* horizontal pixels per EM               */
//   FT_UShort  y_ppem;      /* vertical pixels per EM                 */
//   FT_Fixed   x_scale;     /* scaling values used to convert font    */
//   FT_Fixed   y_scale;     /* units to 26.6 fractional pixels        */
//   FT_Pos     ascender;    /* ascender in 26.6 frac. pixels          */
//   FT_Pos     descender;   /* descender in 26.6 frac. pixels         */
//   FT_Pos     height;      /* text height in 26.6 frac. pixels       */
//   FT_Pos     max_advance; /* max horizontal advance, in 26.6 pixels */
// } FT_Size_Metrics;

// typedef struct FT_SizeRec_
//   FT_Face           face;      /* parent face object              */
//   FT_Generic        generic;   /* generic pointer for client uses */
//   FT_Size_Metrics   metrics;   /* size metrics                    */
//   FT_Size_Internal  internal;
// } FT_SizeRec;

// typedef struct FT_SubGlyphRec_*  FT_SubGlyph;

// typedef struct FT_Slot_InternalRec_*  FT_Slot_Internal;

// typedef struct FT_GlyphSlotRec_
EMSCRIPTEN_BINDINGS(FT_GlyphSlotRec) {
  emscripten::class_<FT_GlyphSlotRec>("FT_GlyphSlotRec")
    // FT_Library        library;
    // FT_Face           face;
    // FT_GlyphSlot      next;
    // FT_UInt           glyph_index; /* new in 2.10; was reserved previously */
    // FT_Generic        generic;

    // FT_Glyph_Metrics  metrics;
    .property("metrics", &FT_GlyphSlotRec::metrics)
    // FT_Fixed          linearHoriAdvance;
    .property("linearHoriAdvance", &FT_GlyphSlotRec::linearHoriAdvance)
    // FT_Fixed          linearVertAdvance;
    .property("linearVertAdvance", &FT_GlyphSlotRec::linearVertAdvance)
    // FT_Vector         advance;
    .property("advance", &FT_GlyphSlotRec::advance)

    // FT_Glyph_Format   format;
    .property("format", &FT_GlyphSlotRec::format)

    // FT_Bitmap         bitmap;
    .property("bitmap", FUNCTION(emscripten::val, (const FT_GlyphSlotRec& that), {
      FT_Bitmap* p = (FT_Bitmap*) (intptr_t) &that.bitmap; return emscripten::val(p);
    }))
    // FT_Int            bitmap_left;
    .property("bitmap_left", &FT_GlyphSlotRec::bitmap_left)
    // FT_Int            bitmap_top;
    .property("bitmap_top", &FT_GlyphSlotRec::bitmap_top)

    // FT_Outline        outline;
    .property("outline", FUNCTION(emscripten::val, (const FT_GlyphSlotRec& that), {
      FT_Outline* p = (FT_Outline*) (intptr_t) &that.outline; return emscripten::val(p);
    }))

    // FT_UInt           num_subglyphs;
    // FT_SubGlyph       subglyphs;

    // void*             control_data;
    // long              control_len;

    // FT_Pos            lsb_delta;
    // FT_Pos            rsb_delta;

    // void*             other;

    // FT_Slot_Internal  internal;
  ;
}

EMSCRIPTEN_BINDINGS(FreeType) {

// FT_EXPORT( void )
// FT_Library_Version( FT_Library   library,
//                     FT_Int      *amajor,
//                     FT_Int      *aminor,
//                     FT_Int      *apatch );
emscripten::function("FT_Library_Version", FUNCTION(void, (emscripten::val library, emscripten::val amajor, emscripten::val aminor, emscripten::val apatch), {
  if (library[0].isNull()) { /* throw */ }
  FT_Library _library = (FT_Library) library[0].as<intptr_t>(emscripten::allow_raw_pointers());
  FT_Int major = 0;
  FT_Int minor = 0;
  FT_Int patch = 0;
  FT_Library_Version(_library, &major, &minor, &patch);
  amajor.set(0, emscripten::val(major));
  aminor.set(0, emscripten::val(minor));
  apatch.set(0, emscripten::val(patch));
}));

// FT_EXPORT( FT_Error )
// FT_Init_FreeType( FT_Library  *alibrary );
emscripten::function("FT_Init_FreeType", FUNCTION(FT_Error, (emscripten::val library), {
  if (!library[0].isNull()) { /* throw */ }
  FT_Library _library;
  FT_Error error = FT_Init_FreeType(&_library);
  intptr_t p = (intptr_t) _library; library.set(0, emscripten::val(p));
  return error;
}));

// FT_EXPORT( FT_Error )
// FT_Done_FreeType( FT_Library  library );
emscripten::function("FT_Done_FreeType", FUNCTION(FT_Error, (emscripten::val library), {
  if (library[0].isNull()) { /* throw */ }
  FT_Library _library = (FT_Library) library[0].as<intptr_t>(emscripten::allow_raw_pointers());
  library.set(0, emscripten::val::null());
  return FT_Done_FreeType(_library);
}));

// #define FT_OPEN_MEMORY    0x1
// #define FT_OPEN_STREAM    0x2
// #define FT_OPEN_PATHNAME  0x4
// #define FT_OPEN_DRIVER    0x8
// #define FT_OPEN_PARAMS    0x10

// /* these constants are deprecated; use the corresponding `FT_OPEN_XXX` */
// #define ft_open_memory    FT_OPEN_MEMORY
// #define ft_open_stream    FT_OPEN_STREAM
// #define ft_open_pathname  FT_OPEN_PATHNAME
// #define ft_open_driver    FT_OPEN_DRIVER
// #define ft_open_params    FT_OPEN_PARAMS

// typedef struct FT_Parameter_
//   FT_ULong    tag;
//   FT_Pointer  data;
// } FT_Parameter;

// typedef struct FT_Open_Args_
//   FT_UInt         flags;
//   const FT_Byte*  memory_base;
//   FT_Long         memory_size;
//   FT_String*      pathname;
//   FT_Stream       stream;
//   FT_Module       driver;
//   FT_Int          num_params;
//   FT_Parameter*   params;
// } FT_Open_Args;

// FT_EXPORT( FT_Error )
// FT_New_Face( FT_Library   library,
//              FT_Long      face_index,
//              FT_Face     *aface );

// FT_EXPORT( FT_Error )
// FT_New_Memory_Face( FT_Library      library,
//                     const FT_Byte*  file_base,
//                     FT_Long         file_size,
//                     FT_Long         face_index,
//                     FT_Face        *aface );
emscripten::function("FT_New_Memory_Face", FUNCTION(FT_Error, (emscripten::val library, emscripten::val file, FT_Long face_index, emscripten::val face), {
  if (library[0].isNull()) { /* throw */ }
  FT_Library _library = (FT_Library) library[0].as<intptr_t>(emscripten::allow_raw_pointers());
  std::vector<FT_Byte>* _file = new std::vector<FT_Byte>;
  _file->resize(file["length"].as<size_t>());
  emscripten::val(emscripten::typed_memory_view<FT_Byte>(_file->size(), _file->data())).call<void>("set", file);
  if (!face[0].isNull()) { /* throw */ }
  FT_Face _face;
  FT_Error error = FT_New_Memory_Face(_library, _file->data(), _file->size(), face_index, &_face);
  _face->generic.data = _file;
  _face->generic.finalizer = FUNCTION(void, (void* object), {
    FT_Face _face = (FT_Face) object;
    std::vector<FT_Byte>* _file = (std::vector<FT_Byte>*) _face->generic.data;
    delete _file;
  });
  FT_Face p = (FT_Face) (intptr_t) _face; face.set(0, emscripten::val(p));
  return error;
}));

// FT_EXPORT( FT_Error )
// FT_Open_Face( FT_Library           library,
//               const FT_Open_Args*  args,
//               FT_Long              face_index,
//               FT_Face             *aface );

// FT_EXPORT( FT_Error )
// FT_Attach_File( FT_Face      face,

// FT_EXPORT( FT_Error )
// FT_Attach_Stream( FT_Face        face,
//                   FT_Open_Args*  parameters );

// FT_EXPORT( FT_Error )
// FT_Reference_Face( FT_Face  face );

// FT_EXPORT( FT_Error )
// FT_Done_Face( FT_Face  face );
emscripten::function("FT_Done_Face", FUNCTION(FT_Error, (emscripten::val face), {
  if (face[0].isNull()) { /* throw */ }
  FT_Face _face = face[0].as<FT_Face>(emscripten::allow_raw_pointers());
  face.set(0, emscripten::val::null());
  FT_Error error = FT_Done_Face(_face);
  return error;
}));

// FT_EXPORT( FT_Error )
// FT_Select_Size( FT_Face  face,
//                 FT_Int   strike_index );

// typedef enum  FT_Size_Request_Type_
//   FT_SIZE_REQUEST_TYPE_NOMINAL,
//   FT_SIZE_REQUEST_TYPE_REAL_DIM,
//   FT_SIZE_REQUEST_TYPE_BBOX,
//   FT_SIZE_REQUEST_TYPE_CELL,
//   FT_SIZE_REQUEST_TYPE_SCALES,
//   FT_SIZE_REQUEST_TYPE_MAX
// } FT_Size_Request_Type;

// typedef struct FT_Size_RequestRec_
//   FT_Size_Request_Type  type;
//   FT_Long               width;
//   FT_Long               height;
//   FT_UInt               horiResolution;
//   FT_UInt               vertResolution;
// } FT_Size_RequestRec;
// typedef struct FT_Size_RequestRec_  *FT_Size_Request;

// FT_EXPORT( FT_Error )
// FT_Request_Size( FT_Face          face,
//                  FT_Size_Request  req );

// FT_EXPORT( FT_Error )
// FT_Set_Char_Size( FT_Face     face,
//                   FT_F26Dot6  char_width,
//                   FT_F26Dot6  char_height,
//                   FT_UInt     horz_resolution,
//                   FT_UInt     vert_resolution );
emscripten::function("FT_Set_Char_Size", FUNCTION(FT_Error, (emscripten::val face, FT_F26Dot6 char_width, FT_F26Dot6 char_height, FT_UInt horz_resolution, FT_UInt vert_resolution), {
  FT_Face _face = face[0].as<FT_Face>(emscripten::allow_raw_pointers());
  return FT_Set_Char_Size(_face, char_width, char_height, horz_resolution, vert_resolution);
}));

// FT_EXPORT( FT_Error )
// FT_Set_Pixel_Sizes( FT_Face  face,
//                     FT_UInt  pixel_width,
//                     FT_UInt  pixel_height );

// FT_EXPORT( FT_Error )
// FT_Load_Glyph( FT_Face   face,
//                FT_UInt   glyph_index,
//                FT_Int32  load_flags );
emscripten::function("FT_Load_Glyph", FUNCTION(FT_Error, (emscripten::val face, FT_UInt glyph_index, FT_Int32 load_flags), {
  FT_Face _face = face[0].as<FT_Face>(emscripten::allow_raw_pointers());
  return FT_Load_Glyph(_face, glyph_index, load_flags);
}));

// FT_EXPORT( FT_Error )
// FT_Load_Char( FT_Face   face,
//               FT_ULong  char_code,
//               FT_Int32  load_flags );
emscripten::function("FT_Load_Char", FUNCTION(FT_Error, (emscripten::val face, FT_ULong char_code, FT_Int32 load_flags), {
  FT_Face _face = face[0].as<FT_Face>(emscripten::allow_raw_pointers());
  return FT_Load_Char(_face, char_code, load_flags);
}));

// #define FT_LOAD_DEFAULT                      0x0
// #define FT_LOAD_NO_SCALE                     ( 1L << 0 )
// #define FT_LOAD_NO_HINTING                   ( 1L << 1 )
// #define FT_LOAD_RENDER                       ( 1L << 2 )
// #define FT_LOAD_NO_BITMAP                    ( 1L << 3 )
// #define FT_LOAD_VERTICAL_LAYOUT              ( 1L << 4 )
// #define FT_LOAD_FORCE_AUTOHINT               ( 1L << 5 )
// #define FT_LOAD_CROP_BITMAP                  ( 1L << 6 )
// #define FT_LOAD_PEDANTIC                     ( 1L << 7 )
// #define FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH  ( 1L << 9 )
// #define FT_LOAD_NO_RECURSE                   ( 1L << 10 )
// #define FT_LOAD_IGNORE_TRANSFORM             ( 1L << 11 )
// #define FT_LOAD_MONOCHROME                   ( 1L << 12 )
// #define FT_LOAD_LINEAR_DESIGN                ( 1L << 13 )
// #define FT_LOAD_NO_AUTOHINT                  ( 1L << 15 )
// /* Bits 16-19 are used by `FT_LOAD_TARGET_` */
// #define FT_LOAD_COLOR                        ( 1L << 20 )
// #define FT_LOAD_COMPUTE_METRICS              ( 1L << 21 )
// #define FT_LOAD_BITMAP_METRICS_ONLY          ( 1L << 22 )
// #define FT_LOAD_ADVANCE_ONLY                 ( 1L << 8 )
// #define FT_LOAD_SBITS_ONLY                   ( 1L << 14 )
// #define FT_LOAD_TARGET_( x )   ( (FT_Int32)( (x) & 15 ) << 16 )
// #define FT_LOAD_TARGET_NORMAL  FT_LOAD_TARGET_( FT_RENDER_MODE_NORMAL )
// #define FT_LOAD_TARGET_LIGHT   FT_LOAD_TARGET_( FT_RENDER_MODE_LIGHT  )
// #define FT_LOAD_TARGET_MONO    FT_LOAD_TARGET_( FT_RENDER_MODE_MONO   )
// #define FT_LOAD_TARGET_LCD     FT_LOAD_TARGET_( FT_RENDER_MODE_LCD    )
// #define FT_LOAD_TARGET_LCD_V   FT_LOAD_TARGET_( FT_RENDER_MODE_LCD_V  )
// #define FT_LOAD_TARGET_MODE( x )  ( (FT_Render_Mode)( ( (x) >> 16 ) & 15 ) )

// FT_EXPORT( void )
// FT_Set_Transform( FT_Face     face,
//                   FT_Matrix*  matrix,
//                   FT_Vector*  delta );

// typedef enum  FT_Render_Mode_
//   FT_RENDER_MODE_NORMAL = 0,
//   FT_RENDER_MODE_LIGHT,
//   FT_RENDER_MODE_MONO,
//   FT_RENDER_MODE_LCD,
//   FT_RENDER_MODE_LCD_V,
//   FT_RENDER_MODE_MAX
// } FT_Render_Mode;
emscripten::enum_<FT_Render_Mode>("FT_Render_Mode")
  .value("FT_RENDER_MODE_NORMAL", FT_RENDER_MODE_NORMAL)
  .value("FT_RENDER_MODE_LIGHT", FT_RENDER_MODE_LIGHT)
  .value("FT_RENDER_MODE_MONO", FT_RENDER_MODE_MONO)
  .value("FT_RENDER_MODE_LCD", FT_RENDER_MODE_LCD)
  .value("FT_RENDER_MODE_LCD_V", FT_RENDER_MODE_LCD_V)
  .value("FT_RENDER_MODE_MAX", FT_RENDER_MODE_MAX)
;

// /* `FT_Render_Mode` values instead                       */
// #define ft_render_mode_normal  FT_RENDER_MODE_NORMAL
// #define ft_render_mode_mono    FT_RENDER_MODE_MONO

// FT_EXPORT( FT_Error )
// FT_Render_Glyph( FT_GlyphSlot    slot,
//                  FT_Render_Mode  render_mode );
// FT_EXPORT( FT_Error )
// FT_Render_Glyph( FT_GlyphSlot    slot,
//                  FT_Render_Mode  render_mode );
emscripten::function("FT_Render_Glyph", FUNCTION(FT_Error, (emscripten::val slot, FT_Render_Mode render_mode), {
  FT_GlyphSlotRec* _slot = slot.as<FT_GlyphSlotRec*>(emscripten::allow_raw_pointers());
  return FT_Render_Glyph(_slot, render_mode);
}));

// typedef enum  FT_Kerning_Mode_
//   FT_KERNING_DEFAULT = 0,
//   FT_KERNING_UNFITTED,
//   FT_KERNING_UNSCALED
// } FT_Kerning_Mode;

// /* `FT_Kerning_Mode` values instead                      */
// #define ft_kerning_default   FT_KERNING_DEFAULT
// #define ft_kerning_unfitted  FT_KERNING_UNFITTED
// #define ft_kerning_unscaled  FT_KERNING_UNSCALED

// FT_EXPORT( FT_Error )
// FT_Get_Kerning( FT_Face     face,
//                 FT_UInt     left_glyph,
//                 FT_UInt     right_glyph,
//                 FT_UInt     kern_mode,
//                 FT_Vector  *akerning );
emscripten::function("FT_Get_Kerning", FUNCTION(FT_Error, (emscripten::val face, FT_UInt left_glyph, FT_UInt right_glyph, FT_UInt kern_mode, emscripten::val akerning), {
  FT_Face _face = face[0].as<FT_Face>(emscripten::allow_raw_pointers());
  FT_Vector _akerning;
  FT_Error error = FT_Get_Kerning(_face, left_glyph, right_glyph, kern_mode, &_akerning);
  akerning.set("x", _akerning.x);
  akerning.set("y", _akerning.y);
  return error;
}));

// FT_EXPORT( FT_Error )
// FT_Get_Track_Kerning( FT_Face    face,
//                       FT_Fixed   point_size,
//                       FT_Int     degree,
//                       FT_Fixed*  akerning );

// FT_EXPORT( FT_Error )
// FT_Get_Glyph_Name( FT_Face     face,
//                    FT_UInt     glyph_index,
//                    FT_Pointer  buffer,
//                    FT_UInt     buffer_max );

// FT_EXPORT( const char* )
// FT_Get_Postscript_Name( FT_Face  face );

// FT_EXPORT( FT_Error )
// FT_Select_Charmap( FT_Face      face,
//                    FT_Encoding  encoding );

// FT_EXPORT( FT_Error )
// FT_Set_Charmap( FT_Face     face,
//                 FT_CharMap  charmap );

// FT_EXPORT( FT_Int )
// FT_Get_Charmap_Index( FT_CharMap  charmap );

// FT_EXPORT( FT_UInt )
// FT_Get_Char_Index( FT_Face   face,
//                    FT_ULong  charcode );
// FT_EXPORT( FT_UInt )
// FT_Get_Char_Index( FT_Face   face,
//                  FT_ULong  char_code );
emscripten::function("FT_Get_Char_Index", FUNCTION(FT_UInt, (emscripten::val face, FT_ULong char_code), {
  FT_Face _face = face[0].as<FT_Face>(emscripten::allow_raw_pointers());
  return FT_Get_Char_Index(_face, char_code);
}));

// FT_EXPORT( FT_ULong )
// FT_Get_First_Char( FT_Face   face,
//                    FT_UInt  *agindex );

// FT_EXPORT( FT_ULong )
// FT_Get_Next_Char( FT_Face    face,
//                   FT_ULong   char_code,
//                   FT_UInt   *agindex );

// FT_EXPORT( FT_Error )
// FT_Face_Properties( FT_Face        face,
//                     FT_UInt        num_properties,
//                     FT_Parameter*  properties );

// FT_EXPORT( FT_UInt )
// FT_Get_Name_Index( FT_Face     face,
//                    FT_String*  glyph_name );

// #define FT_SUBGLYPH_FLAG_ARGS_ARE_WORDS          1
// #define FT_SUBGLYPH_FLAG_ARGS_ARE_XY_VALUES      2
// #define FT_SUBGLYPH_FLAG_ROUND_XY_TO_GRID        4
// #define FT_SUBGLYPH_FLAG_SCALE                   8
// #define FT_SUBGLYPH_FLAG_XY_SCALE             0x40
// #define FT_SUBGLYPH_FLAG_2X2                  0x80
// #define FT_SUBGLYPH_FLAG_USE_MY_METRICS      0x200

// FT_EXPORT( FT_Error )
// FT_Get_SubGlyph_Info( FT_GlyphSlot  glyph,
//                       FT_UInt       sub_index,
//                       FT_Int       *p_index,
//                       FT_UInt      *p_flags,
//                       FT_Int       *p_arg1,
//                       FT_Int       *p_arg2,
//                       FT_Matrix    *p_transform );

// typedef struct FT_LayerIterator_
//   FT_UInt   num_layers;
//   FT_UInt   layer;
//   FT_Byte*  p;
// } FT_LayerIterator;

// FT_EXPORT( FT_Bool )
// FT_Get_Color_Glyph_Layer( FT_Face            face,
//                           FT_UInt            base_glyph,
//                           FT_UInt           *aglyph_index,
//                           FT_UInt           *acolor_index,
//                           FT_LayerIterator*  iterator );

// #define FT_FSTYPE_INSTALLABLE_EMBEDDING         0x0000
// #define FT_FSTYPE_RESTRICTED_LICENSE_EMBEDDING  0x0002
// #define FT_FSTYPE_PREVIEW_AND_PRINT_EMBEDDING   0x0004
// #define FT_FSTYPE_EDITABLE_EMBEDDING            0x0008
// #define FT_FSTYPE_NO_SUBSETTING                 0x0100
// #define FT_FSTYPE_BITMAP_EMBEDDING_ONLY         0x0200

// FT_EXPORT( FT_UShort )
// FT_Get_FSType_Flags( FT_Face  face );

// FT_EXPORT( FT_UInt )
// FT_Face_GetCharVariantIndex( FT_Face   face,
//                              FT_ULong  charcode,
//                              FT_ULong  variantSelector );

// FT_EXPORT( FT_Int )
// FT_Face_GetCharVariantIsDefault( FT_Face   face,
//                                  FT_ULong  charcode,
//                                  FT_ULong  variantSelector );

// FT_EXPORT( FT_UInt32* )
// FT_Face_GetVariantSelectors( FT_Face  face );

// FT_EXPORT( FT_UInt32* )
// FT_Face_GetVariantsOfChar( FT_Face   face,
//                            FT_ULong  charcode );

// FT_EXPORT( FT_UInt32* )
// FT_Face_GetCharsOfVariant( FT_Face   face,
//                            FT_ULong  variantSelector );

// FT_EXPORT( FT_Long )
// FT_MulDiv( FT_Long  a,
//            FT_Long  b,
//            FT_Long  c );

// FT_EXPORT( FT_Long )
// FT_MulFix( FT_Long  a,
//            FT_Long  b );

// FT_EXPORT( FT_Long )
// FT_DivFix( FT_Long  a,
//            FT_Long  b );

// FT_EXPORT( FT_Fixed )
// FT_RoundFix( FT_Fixed  a );

// FT_EXPORT( FT_Fixed )
// FT_CeilFix( FT_Fixed  a );

// FT_EXPORT( FT_Fixed )
// FT_FloorFix( FT_Fixed  a );

// FT_EXPORT( void )
// FT_Vector_Transform( FT_Vector*        vector,
//                      const FT_Matrix*  matrix );

// FT_EXPORT( void )
// FT_Library_Version( FT_Library   library,
//                     FT_Int      *amajor,
//                     FT_Int      *aminor,
//                     FT_Int      *apatch );

// FT_EXPORT( FT_Bool )
// FT_Face_CheckTrueTypePatents( FT_Face  face );

// FT_EXPORT( FT_Bool )
// FT_Face_SetUnpatentedHinting( FT_Face  face,
//                               FT_Bool  value );

}

// ftoutln.h

static emscripten::val __func_interface = emscripten::val::undefined();
static emscripten::val __user = emscripten::val::undefined();
EMSCRIPTEN_BINDINGS(FT_Outline_Decompose) {
  // FT_EXPORT( FT_Error )
  // FT_Outline_Decompose( FT_Outline*              outline,
  //                     const FT_Outline_Funcs*  func_interface,
  //                     void*                    user );
  emscripten::function("FT_Outline_Decompose", FUNCTION(FT_Error, (emscripten::val outline, emscripten::val func_interface, emscripten::val user), {
    __func_interface = func_interface;
    __user = user;
    FT_Outline_Funcs _func_interface;
    _func_interface.move_to = FUNCTION(int, (const FT_Vector* to, void* user), {
      // printf("move_to %ld %ld %p\n", to->x, to->y, user);
      return __func_interface["move_to"](emscripten::val(*to), __user).as<int>();
    });
    _func_interface.line_to = FUNCTION(int, (const FT_Vector* to, void* user), {
      // printf("line_to %ld %ld %p\n", to->x, to->y, user);
      return __func_interface["line_to"](emscripten::val(*to), __user).as<int>();
    });
    _func_interface.conic_to = FUNCTION(int, (const FT_Vector* control, const FT_Vector* to, void* user), {
      // printf("conic_to %ld %ld %ld %ld %p\n", control->x, control->y, to->x, to->y, user);
      return __func_interface["conic_to"](emscripten::val(*control), emscripten::val(*to), __user).as<int>();
    });
    _func_interface.cubic_to = FUNCTION(int, (const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user), {
      // printf("cubic_to %ld %ld %ld %ld %ld %ld %p\n", control1->x, control1->y, control2->x, control2->y, to->x, to->y, user);
      return __func_interface["cubic_to"](emscripten::val(*control1), emscripten::val(*control2), emscripten::val(*to), __user).as<int>();
    });
    emscripten::val shift = func_interface["shift"];
    emscripten::val delta = func_interface["delta"];
    _func_interface.shift = shift.isUndefined() ? 0 : shift.as<int>();
    _func_interface.delta = delta.isUndefined() ? 0 : delta.as<FT_Pos>();
    FT_Outline* _outline = outline.as<FT_Outline*>(emscripten::allow_raw_pointers());
    return FT_Outline_Decompose(_outline, &_func_interface, NULL);
  }));
}
