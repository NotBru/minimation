#ifndef MINIM_SVG_H
#define MINIM_SVG_H

#include <vector>
#include <iostream>
#include <string>
#include <list>
#include <memory>
#include <map>

namespace minim
{
  namespace svg
  {
    // QUESTION: Inherit from std::runtime_error?
    class negative_measure{};
    class bad_rgb{};
    class bad_size{};

    class Formatter
    {
      virtual void insert(std::map<std::string, std::string>) const;
    public:
      friend class Shape;
      friend class Surface;
    };

    class Shape
    {
      virtual std::string get_content();
      std::map<std::string, std::string> format;

    protected:

      std::string default_format()
      {
        std::string ret;
        for(std::pair<std::string, std::string> formatter: format)
          ret+=(std::string)" "+formatter.first+"=\""+formatter.second+"\"";
        return ret;
      }

    public:

      Shape& operator<<(const Formatter& formatter)
      {
        formatter.insert(format);
        return *this;
      }

      friend class Surface;
    };

    class Surface
    {

      int width, height;
      std::vector<double> viewBox;
      std::list< std::unique_ptr<Shape> > shapes;
      // QUESTION: Any more relevant properties?

    public:

      Surface():
        width{ 512 },
        height{ 512 }
      {}

      Surface(const int width, const int height):
        width{ width },
        height{ height }
      {}

      void set_width(const int width)
      {
        this->width=width;
      }

      int get_width() const
      {
        return width;
      }

      void set_height(const int height)
      {
        this->height=height;
      }

      int get_height() const
      {
        return height;
      }

      void set_size(const int width, const int height)
      {
        this->width=width;
        this->height=height;
      }

      std::pair<int, int> get_size() const
      {
        return {width, height};
      }

      void set_viewbox(const std::pair<double, double> origin, const std::pair<double, double> size)
      {
        if(size.first<0 || size.second<0) throw negative_measure{};
        viewBox=std::vector<double>{origin.first, origin.second, size.first, size.second};
      }

      std::pair<double, double> get_viewbox_origin() const
      {
        return {viewBox[0], viewBox[1]};
      }

      std::pair<double, double> get_viewbox_end() const
      {
        return {viewBox[0]+viewBox[2], viewBox[1]+viewBox[3]};
      }

      std::pair<double, double> get_viewbox_size() const
      {
        return {viewBox[2], viewBox[3]};
      }

      void set_viewbox_coordinates(const std::pair<double, double> origin, const std::pair<double, double> end)
      {
        if(end.first<origin.first || end.second<origin.second) throw negative_measure{};
        viewBox=std::vector<double>{origin.first, origin.second, end.first-origin.first, end.second-origin.second};
      }
      
      // void write(std::string filename); // TODO

      // friend operator<<(std::ostream&, const Surface&); // TODO?

      Surface &operator<<(const Shape& shape)
      {
        // TAPIANINI: ¿Cómo evito object slicing?
        //            ¿Se puede declarar virtual al operator new?
        shapes.push_back(std::unique_ptr<Shape>(new Shape{shape}));
        return *this;
      }

      Surface& operator<<(const Formatter& formatter)
      {
        formatter.insert(shapes.back()->format);
        return *this;
      }

    };

    // Formatters sleeping
    // Do not disturb

    class SetFillRGB: public Formatter
    {
      int r, g, b;

      void check_and_assign(const std::vector<int>& rgb)
      {
        if(rgb.size()!=3) throw bad_rgb{};
        for(int i=0; i<3; i++) if(rgb[i]<0 || rgb[i]>=256) throw bad_rgb{};
        r=rgb[0];
        g=rgb[1];
        b=rgb[2];
      }

      std::string get_id() const
      {
        return "fill";
      }

      // QUESTION: Should it modify opacity?
      void insert(std::map<std::string, std::string>& format) const
      {
        format["fill"]=(std::string)"rgb("+std::to_string(r)+","+std::to_string(g)+","+std::to_string(b);
      }

    public:
      SetFillRGB(const std::vector<int>& rgb)
      {
        check_and_assign(rgb);
      }

      SetFillRGB(const std::vector<double>& rgb)
      {
        check_and_assign({(int)(rgb[0]*255.999), (int)(rgb[1]*255.999), (int)(rgb[2]*255.999)});
      }
    };

    class Path: public Shape
    {
      std::string d;
      std::string get_content()
      {
        return (std::string)("<path d=\"")+d+"\""+default_format()+"/>";
      }

      void commandu(const std::string com, std::vector<double> params)
      {
        d+=com;
        for(const double param: params)
        {
          d+=(std::string)" "+std::to_string(param);
        }
      }

      void command(const std::string com, std::vector< std::vector<double> > params)
      {
        d+=com;
        for(const std::vector<double> &param: params)
        {
          d+=(std::string)" "+std::to_string(param[0])+" "+std::to_string(param[1]);
        }
      }

    public:
      Path(const std::vector<double> &origin, bool rel=false):
        d{ (rel?"m":"M")+std::to_string(origin[0])+" "+std::to_string(origin[1]) }
      {}

      void L(const std::vector<double> &target)
      {
        command("L", {target});
      }

      void l(const std::vector<double> &target)
      {
        command("l", {target});
      }

      void H(const double target)
      {
        commandu("H", {target});
      }

      void h(const double target)
      {
        commandu("h", {target});
      }

      void V(const double target)
      {
        commandu("V", {target});
      }

      void v(const double target)
      {
        commandu("v", {target});
      }

      void Z()
      {
        d+="Z";
      }

      void C(const std::vector<double> origin_handle, const std::vector<double> target_handle, const std::vector<double> target)
      {
        command("C", {origin_handle, target_handle, target});
      }

      void c(const std::vector<double> origin_handle, const std::vector<double> target_handle, const std::vector<double> target)
      {
        command("c", {origin_handle, target_handle, target});
      }

      void S(const std::vector<double> target_handle, const std::vector<double> target)
      {
        command("S", {target_handle, target});
      }

      void s(const std::vector<double> target_handle, const std::vector<double> target)
      {
        command("s", {target_handle, target});
      }

      void Q(const std::vector<double> handle, const std::vector<double> target)
      {
        command("Q", {handle, target});
      }

      void q(const std::vector<double> handle, const std::vector<double> target)
      {
        command("q", {handle, target});
      }

      void T(const std::vector<double> target)
      {
        command("T", {target});
      }

      void t(const std::vector<double> target)
      {
        command("t", {target});
      }

      // Arc is missing but... whatever.
    };
  }
}

#endif
