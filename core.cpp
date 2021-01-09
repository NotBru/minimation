#include "minim_svg.hpp"

int main()
{
  // Canvas for drawing, size 512x512
  minim::svg::Surface surface(512, 512);

  // Path starting at coordinates (256+128, 256)
  minim::svg::Path path({256-128, 256});
  // Line to (256, 256+128)
  path.L({256, 256+128});
  // ...
  path.L({256+128, 256});
  path.L({256, 256-128});
  // Close path
  path.Z();

  // Fill with color #008b8b
  path << minim::svg::SetFillRGB({0, 128, 128});

  // Paint into surface
  surface << path;

  // Should be equivalent to
  /*
   * surface << path << minim::svg::SetFillRGB({0, 128, 128});
   */

  // Or even
  /*
   * surface << path;
   * surface << minim::svg::SetFillRGB({0, 128, 128});
   */

  // Function pending, but output should be
  /*
   * surface.write("minim_svg_output.svg");
   */

  surface.write("test.svg");

  return 0;
}
