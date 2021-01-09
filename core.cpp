#include "minim_svg.hpp"

int main()
{
  // Canvas for drawing, size 512x512
  minim::svg::Surface surface(512, 512);
  // Set view box from (116, 116) with size (512-2*116, 512-2*116)
  surface.set_viewbox({116, 116}, {512-2*116, 512-2*116});

  // Rectangle from (256-96, 256-96) with size (194, 194)
  minim::svg::Rect rect({256-96, 256-96}, {194, 194});

  // Set corner radiuses
  // (I have no idea why there's 2 of them)
  rect.set_rx(15);
  rect.set_ry(3);

  // Stack up a bunch of formatters
  rect << minim::svg::SetStrokeRGB({255, 255, 255})
       << minim::svg::SetStrokeWidth(3);

  surface << rect;

  // Path starting at coordinates (256+128, 256)
  minim::svg::Path path({256-128, 256});
  // Line to (256, 256+128)
  path.L({256, 256+128});
  // ...
  path.L({256+128, 256});
  path.L({256, 256-128});
  // Close path
  path.Z();

  // Stack up another bunch of formatters
  path << minim::svg::SetFillRGBA({0, 128, 128, 128})
       << minim::svg::SetStrokeRGB({0, 0, 0})
       << minim::svg::SetStrokeWidth(3)
       << minim::svg::SetStrokeLinejoin("round")
       << minim::svg::SetStrokeLinecap("round")
       << minim::svg::SetStrokeDasharray({16, 8, 4});

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
