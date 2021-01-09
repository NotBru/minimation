#ifndef MINIM_SVG_H
#define MINIM_SVG_H

#include <vector>
#include <iostream>
#include <fstream>
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
    class bad_linecap{};
    class bad_linejoin{};
    class bad_opacity{};
    class bad_size{};

    enum class Linecap { butt, square, round };
    enum class Linejoin { miter, round, bevel };

    struct RGB
    {
      unsigned char r, g, b;
    };

    struct RGBA
    {
      unsigned char r, g, b, a;
    };

    class Formatter
    {
      virtual void insert(std::map<std::string, std::string>&) const = 0;
    public:
      friend class Shape;
      friend class Surface;
    };

    class Shape
    {
      virtual std::string get_content() const = 0;
      virtual Shape* clone() const = 0;
      std::map<std::string, std::string> format;

    protected:

      std::string default_format() const
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
      
      void write(std::string filename)
      {
        std::ofstream outf;
        outf.open(filename);
        outf << "<svg version=\"1.1\"\n";
        if(viewBox.size())
          outf << "     viewBox=\"" << viewBox[0] << " " << viewBox[1] << " " << viewBox[2] << " " << viewBox[3] << "\"\n";
        outf << "     baseProfile=\"full\"\n"
                "     width=\"" << width << "\" height=\"" << height << "\"\n"
                "     xmlns=\"http://www.w3.org/2000/svg\">\n\n";

        for(std::unique_ptr<Shape> &shape: shapes)
          outf << "  " << shape->get_content() << "\n\n";

        outf << "</svg>";
      }

      // friend operator<<(std::ostream&, const Surface&); // TODO?

      Surface &operator<<(const Shape& shape)
      {
        shapes.push_back(std::unique_ptr<Shape>(shape.clone()));
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
      RGB rgb;

      std::string get_id() const
      {
        return "fill";
      }

      void insert(std::map<std::string, std::string>& format) const
      {
        format["fill"]=(std::string)"rgb("+std::to_string(rgb.r)+","+std::to_string(rgb.g)+","+std::to_string(rgb.b)+")";
      }

    public:
      SetFillRGB(const RGB &rgb):
        rgb{ rgb }
      {}
    };

    class SetFillOpacity: public Formatter
    {
      double opacity;

      std::string get_id() const
      {
        return "fill-opacity";
      }

      void insert(std::map<std::string, std::string>& format) const
      {
        format["fill-opacity"]=std::to_string(opacity);
      }

    public:
      SetFillOpacity(const double opacity):
        opacity{ opacity }
      {
        // QUESTION: Perform this check?
        if(opacity<0 || opacity>1) throw bad_opacity{};
      }
    };

    class SetFillRGBA: public Formatter
    {
      RGBA rgba;

      std::string get_id() const
      {
        return "fill";
      }

      void insert(std::map<std::string, std::string>& format) const
      {
        format["fill"]=(std::string)"rgb("+std::to_string(rgba.r)+","+std::to_string(rgba.g)+","+std::to_string(rgba.b)+")";
        format["fill-opacity"]=std::to_string(rgba.a/255.);
      }

    public:
      SetFillRGBA(const RGBA &rgba):
        rgba{ rgba }
      {}
    };

    class SetStrokeRGB: public Formatter
    {
      RGB rgb;

      std::string get_id() const
      {
        return "stroke";
      }

      void insert(std::map<std::string, std::string>& format) const
      {
        format["stroke"]=(std::string)"rgb("+std::to_string(rgb.r)+","+std::to_string(rgb.g)+","+std::to_string(rgb.b)+")";
      }

    public:
      SetStrokeRGB(const RGB &rgb):
        rgb{ rgb }
      {}
    };

    class SetStrokeOpacity: public Formatter
    {
      double opacity;

      std::string get_id() const
      {
        return "stroke-opacity";
      }

      void insert(std::map<std::string, std::string>& format) const
      {
        format["stroke-opacity"]=std::to_string(opacity);
      }

    public:
      SetStrokeOpacity(const double opacity):
        opacity{ opacity }
      {
        // QUESTION: Perform this check?
        if(opacity<0 || opacity>1) throw bad_opacity{};
      }
    };

    class SetStrokeRGBA: public Formatter
    {
      RGBA rgba;

      std::string get_id() const
      {
        return "stroke";
      }

      void insert(std::map<std::string, std::string>& format) const
      {
        format["stroke"]=(std::string)"rgb("+std::to_string(rgba.r)+","+std::to_string(rgba.g)+","+std::to_string(rgba.b)+")";
        format["stroke-opacity"]=std::to_string(rgba.a/255.);
      }

    public:
      SetStrokeRGBA(const RGBA &rgba):
        rgba{ rgba }
      {}
    };

    class SetStrokeWidth: public Formatter
    {
      double width;

      std::string get_id() const
      {
        return "stroke-width";
      }

      void insert(std::map<std::string, std::string>& format) const
      {
        format["stroke-width"]=std::to_string(width);
      }

    public:
      SetStrokeWidth(const double width):
        width{ width }
      {}
    };

    class SetStrokeLinecap: public Formatter
    {
      Linecap linecap;

      std::string get_id() const
      {
        return "stroke-linecap";
      }

      void insert(std::map<std::string, std::string>& format) const
      {
        switch(linecap)
        {
          case Linecap::butt:
            format["stroke-linecap"]="butt";
            break;
          case Linecap::square:
            format["stroke-linecap"]="square";
            break;
          case Linecap::round:
            format["stroke-linecap"]="round";
            break;
        }
      }

    public:
      SetStrokeLinecap(const std::string &linecap)
      {
        if     (linecap=="butt")
          this->linecap=Linecap::butt;
        else if(linecap=="square")
          this->linecap=Linecap::square;
        else if(linecap=="round")
          this->linecap=Linecap::round;
        else throw bad_linecap{};
      }

      SetStrokeLinecap(const Linecap linecap):
        linecap{ linecap }
      {}
    };

    class SetStrokeLinejoin: public Formatter
    {
      Linejoin linejoin;

      std::string get_id() const
      {
        return "stroke-linejoin";
      }

      void insert(std::map<std::string, std::string>& format) const
      {
        switch(linejoin)
        {
          case Linejoin::miter:
            format["stroke-linejoin"]="miter";
            break;
          case Linejoin::bevel:
            format["stroke-linejoin"]="bevel";
            break;
          case Linejoin::round:
            format["stroke-linejoin"]="round";
            break;
        }
      }

    public:
      SetStrokeLinejoin(const std::string &linejoin)
      {
        if     (linejoin=="miter")
          this->linejoin=Linejoin::miter;
        else if(linejoin=="bevel")
          this->linejoin=Linejoin::bevel;
        else if(linejoin=="round")
          this->linejoin=Linejoin::round;
        else throw bad_linejoin{};
      }

      SetStrokeLinejoin(const Linejoin linejoin):
        linejoin{ linejoin }
      {}
    };

    class SetStrokeDasharray: public Formatter
    {
      std::list<double> dasharray;

      std::string get_id() const
      {
        return "stroke-dasharray";
      }

      void insert(std::map<std::string, std::string>& format) const
      {
        std::string temp;
        for(double dash: dasharray)
          temp+=std::to_string(dash)+",";
        format["stroke-dasharray"]=temp.substr(0, temp.size()-1);
      }

    public:
      SetStrokeDasharray(std::list<double> dasharray):
        dasharray{ dasharray }
      {
        for(double dash: dasharray) if(dash<0) throw negative_measure{}; 
      }
    };

    class Path: public Shape
    {

      std::string d;
      std::string get_content() const
      {
        return (std::string)("<path d=\"")+d+"\""+default_format()+"/>";
      }

      Shape* clone() const
      {
        return new Path{*this};
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

    class Rect: public Shape
    {
      double x, y, width, height, rx, ry;
      bool br=0;

      std::string get_content() const
      {
        return (std::string)("<rect x=\"")+std::to_string(x)+"\" y=\""+std::to_string(y)
                            +"\" width=\""+std::to_string(width)+"\" height=\""+std::to_string(height)+"\""
                            +(br&1?(std::string)" rx=\""+std::to_string(rx)+"\"":"")
                            +(br&2?(std::string)" ry=\""+std::to_string(ry)+"\"":"")
                            +default_format()+"/>";
      }

      Rect* clone() const
      {
        return new Rect{*this};
      }

    public:
      Rect(const std::pair<double, double> origin, const std::pair<double, double> size):
        x{ origin.first }, y{ origin.second }, width{ size.first }, height{ size.second }
      {
        if(size.first<0 || size.second<0) throw negative_measure{};
      }

      void set_rx(const double rx)
      {
        br|=1;
        this->rx=rx;
        if(rx<0) throw negative_measure{};
      }

      void unset_rx()
      {
        br&=2;
      }

      void set_ry(const double ry)
      {
        br|=2;
        this->ry=ry;
        if(ry<0) throw negative_measure{};
      }

      void unset_ry()
      {
        br&=1;
      }

      // QUESTION: TODO? transformations or whatever?
    };
  }
}

#endif
