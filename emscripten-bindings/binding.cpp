#include <emscripten/bind.h>
#include "dlib/dlib/image_processing.h"

using namespace emscripten;
using namespace dlib;

void array2d_set(
  array2d<rgb_pixel> &arr,
  const unsigned &y,
  const unsigned &x,
  const rgb_pixel &value
) {
  arr[y][x] = value;
}

void start_track(
  correlation_tracker &correlation_tracker,
  const array2d<rgb_pixel> &img,
  const drectangle &bounding_box
) {
  return correlation_tracker.start_track(img, bounding_box);
}

array2d<rgb_pixel> read_image_data(
  uintptr_t image_data_ptr_r,
  const unsigned &width,
  const unsigned &height
) {
  const unsigned char *image_data_ptr = reinterpret_cast<unsigned char *>(image_data_ptr_r);
  array2d<rgb_pixel> output_arr(height, width);
  int x, y;
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      unsigned int i = ((y * width) + x) * 4;
      output_arr[y][x] = rgb_pixel(
        image_data_ptr[i],
        image_data_ptr[i + 1],
        image_data_ptr[i + 2]
      );
    }
  }
  return output_arr;
}

drectangle update(
  correlation_tracker &correlation_tracker,
  const array2d<rgb_pixel> &img
) {
  correlation_tracker.update(img);
  return correlation_tracker.get_position();
}

drectangle update_guess(
  correlation_tracker &correlation_tracker,
  const array2d<rgb_pixel> &img,
  const drectangle &bounding_box
) {
  correlation_tracker.update(img, bounding_box);
  return correlation_tracker.get_position();
}

EMSCRIPTEN_BINDINGS(my_module) {
  value_array<rgb_pixel>("rgb_pixel")
    .element(&rgb_pixel::red)
    .element(&rgb_pixel::green)
    .element(&rgb_pixel::blue)
    ;

  function("readImageData", &read_image_data);

  class_<array2d<rgb_pixel>>("Array2D")
    .constructor<>()
    .constructor<long, long>()
    .function("setSize", &array2d<rgb_pixel>::set_size)
    .function("set", &array2d_set)
    .property("width", &array2d<rgb_pixel>::nc)
    .property("height", &array2d<rgb_pixel>::nr)
    ;

  class_<drectangle>("Rectangle")
    .constructor<double, double, double, double>()
    .property("left", select_const(&drectangle::left))
    .property("top", select_const(&drectangle::top))
    .property("right", select_const(&drectangle::right))
    .property("bottom", select_const(&drectangle::bottom))
    .property("width", &drectangle::width)
    .property("height", &drectangle::height)
    ;

  class_<correlation_tracker>("CorrelationTracker")
    .constructor<>()
    .constructor<unsigned long>()
    .constructor<unsigned long, unsigned long>()
    .constructor<unsigned long, unsigned long, unsigned long>()
    .constructor<unsigned long, unsigned long, unsigned long, double>()
    .constructor<unsigned long, unsigned long, unsigned long, double, double>()
    .constructor<unsigned long, unsigned long, unsigned long, double, double, double>()
    .constructor<unsigned long, unsigned long, unsigned long, double, double, double, double>()
    .constructor<unsigned long, unsigned long, unsigned long, double, double, double, double, double>()
    .function("startTrack", &start_track)
    .function("predict", &update)
    .function("update", &update_guess)
    .function("getPosition", &correlation_tracker::get_position)
    ;
}
