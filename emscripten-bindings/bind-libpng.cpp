#include <png.h>

#include <emscripten/bind.h>

#define FUNCTION(RET, ARGS, CODE...) \
  emscripten::optional_override([] ARGS -> RET { CODE })

#include <malloc.h>

emscripten::val get_mallinfo() {
  const auto &i = mallinfo();
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

EMSCRIPTEN_BINDINGS(LIBPNG) {
  emscripten::constant("PNG_LIBPNG_VER_STRING", emscripten::val(PNG_LIBPNG_VER_STRING));
  // emscripten::constant("PNG_LIBPNG_VER_MAJOR", emscripten::val(PNG_LIBPNG_VER_MAJOR));
  // emscripten::constant("PNG_LIBPNG_VER_MINOR", emscripten::val(PNG_LIBPNG_VER_MINOR));
  // emscripten::constant("PNG_LIBPNG_VER_RELEASE", emscripten::val(PNG_LIBPNG_VER_RELEASE));
  // emscripten::constant("PNG_LIBPNG_VER", emscripten::val(PNG_LIBPNG_VER));
  emscripten::function("png_access_version_number", &png_access_version_number);
}

static std::string _png_error;
static void _error_fn(png_structp png_ptr, png_const_charp error) {
  _png_error = error;
   png_longjmp(png_ptr, 1);
}
static void _warn_fn(png_structp png_ptr, png_const_charp error) {
}
static emscripten::val _get_error() {
  emscripten::val error = emscripten::val::global("Error").new_(_png_error);
  _png_error.clear();
  return error;
}

static std::vector<unsigned char> _png_file;
static unsigned long _png_cursor = 0;

static void _write_fn(png_structp png_ptr, unsigned char *data, unsigned long size) {
  // printf("write %p %lu\n", data, size);
  _png_file.insert(_png_file.end(), data, data + size);
  _png_cursor += size;
}

static void _read_fn(png_structp png_ptr, unsigned char *data, unsigned long size) {
  // printf("read %p %lu\n", data, size);
  memcpy(data, &_png_file[_png_cursor], size);
  _png_cursor += size;
}

EMSCRIPTEN_BINDINGS(encode) {
  emscripten::function("encode", FUNCTION(emscripten::val, (emscripten::val image_data), {
    _png_error.clear();
    _png_file.clear();
    _png_cursor = 0;

    png_uint_32 width = image_data["width"].as<png_uint_32>();
    png_uint_32 height = image_data["height"].as<png_uint_32>();
    int bytes_per_pixel = 4;
    std::vector<unsigned char> data;
    data.resize(width * height * bytes_per_pixel);
    emscripten::val(emscripten::typed_memory_view<unsigned char>(data.size(), data.data())).call<void>("set", image_data["data"]);

    int bit_depth = 8;
    int color_type = PNG_COLOR_TYPE_RGBA;
    int interlace_type = PNG_INTERLACE_NONE;
    int compression_method = PNG_COMPRESSION_TYPE_DEFAULT;
    int filter_method = PNG_FILTER_TYPE_DEFAULT;

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, _error_fn, _warn_fn);
    if (png_ptr == NULL) {
      return _get_error();
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
      png_destroy_write_struct(&png_ptr, NULL);
      return _get_error();
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
      _png_file.clear();
      _png_cursor = 0;
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return _get_error();
    }
    png_set_write_fn(png_ptr, NULL, _write_fn, NULL);
    png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, interlace_type, compression_method, filter_method);
    png_write_info(png_ptr, info_ptr);
    png_bytep row_pointers[height];
    for (int row = 0; row < height; row++) {
      row_pointers[row] = (png_bytep) data.data() + row * png_get_rowbytes(png_ptr, info_ptr);
    }
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    emscripten::val png_file = emscripten::val::global("Uint8Array").new_(emscripten::typed_memory_view<unsigned char>(_png_file.size(), _png_file.data()));

    _png_file.clear();
    _png_cursor = 0;
    _png_error.clear();

    return png_file;
  }));
}

EMSCRIPTEN_BINDINGS(decode) {
  emscripten::function("decode", FUNCTION(emscripten::val, (emscripten::val png_file), {
    _png_error.clear();
    _png_file.clear();
    _png_cursor = 0;

    _png_file.resize(png_file["length"].as<size_t>());
    emscripten::val(emscripten::typed_memory_view<unsigned char>(_png_file.size(), _png_file.data())).call<void>("set", png_file);
    _png_cursor = 0;

    png_uint_32 width = 0;
    png_uint_32 height = 0;
    int bytes_per_pixel = 4;
    std::vector<unsigned char> data;

    int bit_depth = 0;
    int color_type = 0;
    int interlace_type = 0;
    int compression_method = 0;
    int filter_method = 0;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, _error_fn, _warn_fn);
    if (png_ptr == NULL) {
      return _get_error();
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      return _get_error();
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      _png_file.clear();
      _png_cursor = 0;
      return _get_error();
    }
    png_set_read_fn(png_ptr, NULL, _read_fn);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_method, &filter_method);
    #ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
    png_set_scale_16(png_ptr);
    #else
    png_set_strip_16(png_ptr);
    #endif
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
      png_set_palette_to_rgb(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
      png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) != 0) {
      png_set_tRNS_to_alpha(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_RGB) {
      png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
    }
    data.resize(height * png_get_rowbytes(png_ptr, info_ptr));
    png_bytep row_pointers[height];
    for (int row = 0; row < height; row++) {
      row_pointers[row] = (png_bytep) data.data() + row * png_get_rowbytes(png_ptr, info_ptr);
    }
    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    _png_file.clear();
    _png_cursor = 0;
    _png_error.clear();

    emscripten::val ImageData = emscripten::val::global("ImageData");
    if (!ImageData.isUndefined()) {
      return ImageData.new_(
        emscripten::val::global("Uint8ClampedArray").new_(emscripten::typed_memory_view<unsigned char>(data.size(), data.data())),
        emscripten::val(width),
        emscripten::val(height));
    } else {
      emscripten::val image_data = emscripten::val::object();
      image_data.set("width", emscripten::val(width));
      image_data.set("height", emscripten::val(height));
      image_data.set("data", emscripten::val::global("Uint8ClampedArray").new_(emscripten::typed_memory_view<unsigned char>(data.size(), data.data())));
      return image_data;
    }
  }));
}
