#include <armadillo>
#include <list>
#include <cmath>

#include <iomanip>
#include <iostream>

namespace minim
{
  namespace svg
  {
    class Path;

    class Surface
    {
      int width, height;
      std::string content;

    public:
      Surface(int width=512, int height=512):
        width{ width },
        height{ height }
      {}

      friend Surface& operator<<(Surface&, const Path&);

      friend std::ostream& operator<<(std::ostream &, const Surface);
    };

    std::ostream& operator<<(std::ostream &out, const Surface surface)
    {
      out << "<svg version=\"1.1\"\n"
             "     baseProfile=\"full\"\n"
             "     width=\"" << surface.width << "\" height=\"" << surface.height << "\"\n"
             "     xmlns=\"http://www.w3.org/2000/svg\">\n";
      out << surface.content;
      out << "</svg>";
      return out;
    }

    class Path
    {
    public:
      enum class Cap { butt, square, round };

    private:
      std::list<std::string> d;
      arma::Col<double> current;
      std::vector<double> stroke,
                          fill,
                          global_opacity;
      Cap linecap=Cap::butt, linejoin=Cap::butt;
      int padding=2;
      double width=1;
      std::list<double> dasharray;

      void command_insertion(std::string command, std::list< arma::Col<double> > params)
      {
        std::string push=command;
        for(auto param: params) push+=(std::string)(" ")+std::to_string(param[0])+","+std::to_string(param[1]);
        d.push_back(push);
        current = params.back();
      }

      void set_rgba(std::vector<double> &prop, const std::vector<double> rgba)
      {
        if(rgba.size()<3 || rgba.size()>4) throw bad_rgba{};
        for(int i=0; i<rgba.size(); i++) if(rgba[i]<0 || rgba[i]>1) throw bad_rgba{};
        prop=rgba;
      }

      void set_opacity(std::vector<double> &prop, const double opacity)
      {
        if(opacity<0 || opacity>1) throw bad_rgba{};
        if(prop.size()==0 || prop.size()==3) prop.push_back(opacity);
        else prop.back()=opacity;
      }

      double get_opacity(std::vector<double> &prop)
      {
        if(prop.size()==1 || prop.size()==3) return 1;
        return prop.back();
      }

    public:
      class bad_rgba{};
      class bad_length{};

      Path(const arma::Col<double> origin):
        d{ (std::string)"M "+std::to_string(origin[0])+","+std::to_string(origin[1]) },
        current{ origin },
        stroke{ 0, 0, 0 },
        fill{ 0, 0, 0, 0 }
      {}

      void L(const arma::Col<double> target)
      {
        command_insertion(" L", {target});
      }

      void l(const arma::Col<double> rtarget)
      {
        command_insertion(" l", {rtarget});
      }

      void Z()
      {
        d.push_back(" Z");
      }

      void C(const arma::Col<double> first_handle, const arma::Col<double> second_handle, const arma::Col<double> target)
      {
        command_insertion(" C", {first_handle, second_handle, target});
      }

      void C(const arma::Col<double> first_handle, const int dummy, const arma::Col<double> target)
      {
        command_insertion(" C", {first_handle, current, target});
      }

      void C(const int dummy, const arma::Col<double> second_handle, const arma::Col<double> target)
      {
        command_insertion(" C", {current, second_handle, target});
      }

      void c(const arma::Col<double> first_handle, const arma::Col<double> second_handle, const arma::Col<double> target)
      {
        command_insertion(" c", {first_handle, second_handle, target});
      }

      void c(const arma::Col<double> first_handle, const int dummy, const arma::Col<double> target)
      {
        command_insertion(" c", {first_handle, current, target});
      }

      void c(const int dummy, const arma::Col<double> second_handle, const arma::Col<double> target)
      {
        command_insertion(" c", {current, second_handle, target});
      }

      void S(const arma::Col<double> second_handle, const arma::Col<double> target)
      {
        command_insertion(" S", {second_handle, target});
      }

      void s(const arma::Col<double> second_handle, const arma::Col<double> target)
      {
        command_insertion(" s", {second_handle, target});
      }

      void Q(const arma::Col<double> handle, const arma::Col<double> target)
      {
        command_insertion(" Q", {handle, target});
      }

      void q(const arma::Col<double> handle, const arma::Col<double> target)
      {
        command_insertion(" q", {handle, target});
      }

      void T(const arma::Col<double> target)
      {
        command_insertion(" T", {target});
      }

      void t(const arma::Col<double> target)
      {
        command_insertion(" t", {target});
      }

      void set_stroke(const std::vector<double> stroke)
      {
        set_rgba(this->stroke, stroke);
      }

      void set_stroke_opacity(const double opacity)
      {
        set_opacity(this->stroke, opacity);
      }

      double get_stroke_opacity()
      {
        return get_opacity(stroke);
      }

      std::vector<double> get_stroke()
      {
        return stroke;
      }

      void set_fill(std::vector<double> fill)
      {
        set_rgba(this->fill, fill);
      }

      void set_fill_opacity(const double opacity)
      {
        set_opacity(fill, opacity);
      }

      void set_global_opacity(const double opacity)
      {
        set_opacity(global_opacity, opacity);
      }

      void reset_global_opacity()
      {
        global_opacity.resize(0);
      }

      void set_linecap(Cap linecap)
      {
        this->linecap=linecap;
      }

      Cap get_linecap()
      {
        return linecap;
      }

      void set_linejoin(Cap linejoin)
      {
        this->linejoin=linejoin;
      }

      Cap get_linejoin()
      {
        return linejoin;
      }

      void set_width(double width)
      {
        if(width<0) throw bad_length{};
        this->width=width;
      }

      double get_width()
      {
        return width;
      }

      void set_dasharray(std::list<double> dasharray)
      {
        for(auto dash: dasharray) if(dash<0) throw bad_length{};
        this->dasharray=dasharray;
      }

      std::list<double> get_dasharray()
      {
        return dasharray;
      }

      friend Surface& operator<<(Surface&, const Path&);
    };

    Surface& operator<<(Surface& surface, const Path& path)
    {
      std::string d{""};
      for(auto command: path.d) d+=command;

      std::string padding = "\n";
      padding += std::string(path.padding, ' ');

      surface.content += padding+"<path d=\""+d+"\"";

      padding += std::string(6, ' ');

      if(path.stroke.size()>=3)
        surface.content += padding+"stroke=\"rgb("
                          +std::to_string((int)(255.9999*path.stroke[0]))+","
                          +std::to_string((int)(255.9999*path.stroke[1]))+","
                          +std::to_string((int)(255.9999*path.stroke[2]))+")\"";
      // Maybe reduce duplicated code?
      if(path.fill.size()>=3)
        surface.content += padding+"fill=\"rgb("
                          +std::to_string((int)(255.9999*path.fill[0]))+","
                          +std::to_string((int)(255.9999*path.fill[1]))+","
                          +std::to_string((int)(255.9999*path.fill[2]))+")\"";
      if(path.global_opacity.size())
        surface.content += padding+"opacity=\""
                          +std::to_string(path.global_opacity[0])+"\"";
      else
      {
        if(path.stroke.size()==1 || path.stroke.size()==4)
          surface.content += padding+"stroke-opacity=\""
                            +std::to_string(path.stroke.back())+"\"";
        if(path.fill.size()==1 || path.fill.size()==4)
          surface.content += padding+"fill-opacity=\""
                            +std::to_string(path.fill.back())+"\"";
      }
      if(path.linecap!=Path::Cap::butt)
        surface.content += padding+"stroke-linecap=\""+(path.linecap==Path::Cap::square?"square":"round")+"\"";
      if(path.linejoin!=Path::Cap::butt)
        surface.content += padding+"stroke-linejoin=\""+(path.linejoin==Path::Cap::square?"square":"round")+"\"";
      if(path.width!=1)
        surface.content += padding+"stroke-width=\""+std::to_string(path.width)+"\"";
      surface.content += "/>\n";

      return surface;
    }

    namespace
    {
      inline
      arma::Row<double> orthogonal(arma::Col<double> vect)
      {
        if(arma::size(vect)[0]==3) return {-vect[1], vect[0], 0};
        return {-vect[1], vect[0]};
      }
    }

    Path quad_approx(std::function<arma::Col<double>(double)> gamma, const int points=16, const int dts=128)
    {
      // TODO
      // assert(points>=2 && dts>=points);
      Path ret(gamma(0));
      std::list< std::pair< arma::Row<double>, arma::Col<double> > > lines; // normal, point
      lines.push_back({orthogonal(gamma(1/(double)dts)-gamma(0)), gamma(0)});
      double dl=1/(double)points,
             dt=1/(double)dts;
      std::list<double> ts;
      int j=1;
      double accum=0, tot=0;
      for(int i=1; i<dts; i++) tot+=arma::norm(gamma(i*dt)-gamma((i-1)*dt));
      dl*=tot;
      for(int i=1; j<points; i++)
      {
        accum+=arma::norm(gamma(i*dt)-gamma((i-1)*dt));
        if(accum>j*dl)
        {
          j++;
          ts.push_back(i*dt);
        }
      }
      for(auto t: ts)
        lines.push_back({orthogonal(gamma(t+1/(double)dts)-gamma(t-1/(double)dts)), gamma(t)});
      lines.push_back({orthogonal(gamma(1)-gamma(1/(double)dts)), gamma(1)});

      auto curr=lines.begin(),
           next=lines.begin();
      ++next;
      for(; next!=lines.end(); curr=(next++))
      {
        ret.Q(arma::solve(arma::join_cols(curr->first, next->first),
              arma::Col<double>{ arma::as_scalar(arma::dot(curr->first, curr->second)),
                                 arma::as_scalar(arma::dot(next->first, next->second)) }),
              next->second);
      }
      return ret;
    }
  }

  class FrameTransform
  {
  private:
    arma::Col<double> traslation;
    arma::Mat<double> rotscale;

  public:
    FrameTransform(arma::Col<double> origin={0, 0, 0},
                   std::array<arma::Col<double>, 3> axes={ arma::Col<double>{1, 0, 0},
                                                           arma::Col<double>{0, 1, 0},
                                                           arma::Col<double>{0, 0, 1} }):
      traslation{ -origin },
      rotscale{ arma::inv(arma::join_rows(axes[0], axes[1], axes[2])) }
    {}

    static arma::Mat<double> rotation_matrix(arma::Col<double> versor, double theta)
    {
      double x{versor[0]}, y{versor[1]}, z{versor[2]};
      return arma::Mat<double>{ { cos(theta)+x*x*(1-cos(theta)), x*y*(1-cos(theta))-z*sin(theta), x*z*(1-cos(theta))+y*sin(theta) },
                                { x*y*(1-cos(theta))+z*sin(theta), cos(theta)+y*y*(1-cos(theta)), y*z*(1-cos(theta))-x*sin(theta) },
                                { x*z*(1-cos(theta))-y*sin(theta), y*z*(1-cos(theta))+x*sin(theta), cos(theta)+z*z*(1-cos(theta)) } };
    }

    FrameTransform copy()
    {
      return *this;
    }

    FrameTransform& traslate(const arma::Col<double> &displacement)
    {
      traslation = traslation - displacement;
      return *this;
    }

    FrameTransform& rotate(const arma::Col<double> &rotation)
    {
      double theta=arma::norm(rotation);
      rotscale = rotscale*arma::inv(rotation_matrix(1/theta*rotation, theta));
      return *this;
    }

    arma::Col<double> origin() const
    {
        return -traslation;
    }

    friend FrameTransform operator*(const FrameTransform &, const FrameTransform &);

    friend arma::Col<double> operator*(const FrameTransform &, const arma::Col<double> &);
  };

  FrameTransform operator*(const FrameTransform &left, const FrameTransform &right)
  {
    FrameTransform ret;
    ret.traslation = right.rotscale*right.traslation+left.traslation ;
    ret.rotscale = left.rotscale*right.rotscale;
    return ret;
  }
  
  arma::Col<double> operator*(const FrameTransform &left, const arma::Col<double> &right)
  {
    return left.rotscale*(right+left.traslation);
  }
  
  class Camera
  {
  private:
    FrameTransform frame;
    double focal_distance, half_width, half_height;
  
  public:
    Camera(const FrameTransform &frame=FrameTransform{}, double focal_distance=1, double width=2, double height=2):
      frame{ FrameTransform(arma::Col<double>{0, 0, -focal_distance})*frame },
      focal_distance{ focal_distance },
      half_width{ width/2 },
      half_height{ height/2 }
    {}
  
    arma::Col<double> origin() const
    {
      return frame.origin();
    }
  
    double get_focal_distance() const
    {
      return focal_distance;
    }
  
    void set_focal_distance(double new_focal_distance)
    {
      frame=FrameTransform{arma::Col<double>{0, 0, focal_distance-new_focal_distance}};
      focal_distance=new_focal_distance;
    }
  
    double get_xfov() const
    {
      return atan(half_width/focal_distance);
    }
  
    void set_xfov(const double xfov)
    {
      set_focal_distance(half_width/tan(xfov));
    }
  
    double get_yfov() const
    {
      return atan(half_height/focal_distance);
    }
  
    void set_yfov(const double yfov)
    {
      set_focal_distance(half_height/tan(yfov));
    }
  
    double get_width() const
    {
      return 2*half_width;
    }
  
    void set_width(const double width)
    {
      half_width=width/2;
    }
  
    double get_height() const
    {
      return 2*half_height;
    }
  
    void set_height(const double height)
    {
      half_height=height/2;
    }
  
    Camera copy()
    {
      return *this;
    }
  
    Camera &traslate(const arma::Col<double> &displacement)
    {
      frame.traslate(displacement);
      return *this;
    }
  
    arma::Col<double> project(arma::Col<double> point) const
    {
      point = frame*point;
      point[2] = -point[2]/focal_distance;
      return { point[0]/point[2], point[1]/point[2], point[2] };
    }

    std::list< arma::Col<double> > project(std::list< arma::Col<double> > points) const
    {
      std::list< arma::Col<double> > ret;
      for(auto point: points) ret.push_back(project(point));
      return ret;
    }

    std::function<arma::Col<double>(double)> project(std::function<arma::Col<double>(double)> path) const
    {
      FrameTransform frame=this->frame;
      double focal_distance=this->focal_distance;
      return [frame, focal_distance, path](double t)->arma::Col<double>
      {
        arma::Col<double> ret = frame*path(t);
        ret[2] = -ret[2]/focal_distance;
        return { ret[0]/ret[2], ret[1]/ret[2], ret[2] };
      };
    }

    arma::Col<double> project_NDC(arma::Col<double> point) const
    {
      point = frame*point;
      point[2] = -point[2]/focal_distance;
      return { point[0]/point[2]/half_width, point[1]/point[2]/half_height, point[2] };
    }

    std::list< arma::Col<double> > project_NDC(std::list< arma::Col<double> > points) const
    {
      std::list< arma::Col<double> > ret;
      for(auto point: points) ret.push_back(project_NDC(point));
      return ret;
    }

    std::function<arma::Col<double>(double)> project_NDC(std::function<arma::Col<double>(double)> path) const
    {
      FrameTransform frame=this->frame;
      double focal_distance=this->focal_distance,
             half_width=this->half_width,
             half_height=this->half_height;
      return [frame, focal_distance, half_width, half_height, path](double t)->arma::Col<double>
      {
        arma::Col<double> ret = frame*path(t);
        ret[2] = -ret[2]/focal_distance;
        return { ret[0]/ret[2]/half_width, ret[1]/ret[2]/half_height, ret[2] };
      };
    }

    static Camera from_polars(const double rho=1, const double phi=0, const double theta=M_PI/2, const double radial=0);
  };

  Camera Camera::from_polars(const double rho, const double phi, const double theta, const double radial)
  {
    arma::Col<double> x_{           -sin(phi),            cos(phi),          0 },
                      y_{ cos(phi)*cos(theta), sin(phi)*cos(theta), -sin(theta) },
                       z{ cos(phi)*sin(theta), sin(phi)*sin(theta),  cos(theta) };
    arma::Col<double> x = cos(radial)*x_-sin(radial)*y_,
                      y = sin(radial)*x_+cos(radial)*y_;
    FrameTransform frame(rho*z, {x, y, z});
    return Camera(frame);
  }

  class Ellipse;

  class Circle
  {
  private:
    arma::Col<double> center, radius;

  public:
    Circle(const arma::Col<double> &center, const arma::Col<double> &radius):
      center{ center },
      radius{ radius }
    {}

    Circle(const arma::Col<double> &center, const arma::Col<double> &normal, const double radius):
      center{ center },
      radius{ radius*normal/arma::norm(normal) }
    {}
  
    std::list< arma::Col<double> > as_path(const int points) const;

    std::function<arma::Col<double>(double)> as_function() const;
    
    friend Ellipse;
  };

  class Ellipse
  {
  private:
    arma::Col<double> center, axis1, axis2;

    Ellipse() {}
  public:
    Ellipse(const Circle &circle): center{ circle.center }
    {
      double radius = arma::norm(circle.radius);
      arma::Col<double> versor = 1/radius*circle.radius;

      if(fabs(versor[0])>1-0.1) axis1 = arma::Col<double>{0, 1, 0}; // Pick the perpendicularest
      else                      axis1 = arma::Col<double>{1, 0, 0}; //

      axis1 = arma::cross(versor, axis1);
      axis1 = radius/arma::norm(axis1)*axis1;
      axis2 = arma::cross(versor, axis1);
    }

    std::list< arma::Col<double> > as_path(const int points) const
    {
      double dtheta=2*M_PI/points;
      std::list< arma::Col<double> > ret;
      for(int i=0; i<points; i++) ret.push_back(arma::Col<double>{ center+axis1*cos(i*dtheta)+axis2*sin(i*dtheta) });
      return ret;
    }
    
    std::function<arma::Col<double>(double)> as_function() const
    {
      arma::Col<double> center=this->center,
                        axis1=this->axis1,
                        axis2=this->axis2;
      return [center, axis1, axis2](double t)->arma::Col<double>
      {
        return center+axis1*cos(2*M_PI*t)+axis2*sin(2*M_PI*t);
      };
    }
  };

  // ¿Hay forma de meter esto en Circle directamente?
  std::list< arma::Col<double> > Circle::as_path(const int points=128) const
  {
    return Ellipse(*this).as_path(points); //Ah re
  }

  std::function<arma::Col<double>(double)> Circle::as_function() const
  {
    return Ellipse(*this).as_function();
  }

  class Sphere
  {
  private:
    arma::Col<double> center;
    double radius;

    Sphere() {}
  public:
    Sphere(const arma::Col<double> &center, const double radius):
      center{ center },
      radius{ radius }
    {}

    /*
     * TODO: get rid of this?
     *
    std::list< arma::Col<double> > contour(const Camera &camera)
    {
      arma::Col<double> rel=center-camera.origin();
      double norm=arma::norm(rel);
      double q=radius*radius/arma::norm(rel)/arma::norm(rel);
      return camera.project(Circle((1-q)*rel+camera.origin(), rel, sqrt(radius*radius-q*q*norm*norm)).as_path());
    }
    */

    std::function<arma::Col<double>(double)> contour(const Camera &camera)
    {
      arma::Col<double> rel=center-camera.origin();
      double norm=arma::norm(rel);
      double q=radius*radius/arma::norm(rel)/arma::norm(rel);
      return camera.project(Circle((1-q)*rel+camera.origin(), rel, sqrt(radius*radius-q*q*norm*norm)).as_function());
    }

    std::function<arma::Col<double>(double)> contour_NDC(const Camera &camera) const
    {
      arma::Col<double> rel=center-camera.origin();
      double norm=arma::norm(rel);
      double q=radius*radius/arma::norm(rel)/arma::norm(rel);
      return camera.project_NDC(Circle((1-q)*rel+camera.origin(), rel, sqrt(radius*radius-q*q*norm*norm)).as_function());
    }
  };

  namespace samples
  {
    void svg_square()
    {
      svg::Surface surface{512, 512};
        // 512x512 px surface

      svg::Path square({128, 128});
        // Path starting at (128, 128)
      square.l({256, 0});
        // Line with relative displacement (256, 0)
      square.l({0, 256});
      square.l({-256, 0});
      square.Z();
        // Close path
      square.set_fill({1, 1, 1});
        // White fill
      square.set_stroke({0, 0, 0});
        // Black stroke
      square.set_width(2);
        // 2px stroke width

      surface << square;
        // Send square to surface

      std::ofstream outf("svg_square.svg");
      outf << surface;
        // Print surface to file
    }

    void svg_quadratic_approximation()
    {
      svg::Surface surface{512, 512};

      svg::Path square({0, 0});
      square.L({0, 512});
      square.L({512, 512});
      square.L({512, 0});
      square.Z();

      square.set_fill({1, 1, 1});

      surface << square;


      std::function<arma::Col<double>(double)> circle_parametrization=
        [](double t)->arma::Col<double>{ return {256+128*cos(2*M_PI*t), 256+128*sin(2*M_PI*t)}; };

      svg::Path precise_enough_circle = svg::quad_approx(circle_parametrization, 8);
        // Quadratic approximation from the circle's parametrization, taking 8+1 points.

      precise_enough_circle.set_stroke_opacity(0);
      precise_enough_circle.set_fill({0, 0.5, 0.5});
      
      surface << precise_enough_circle;


      svg::Path not_so_precise_circle = svg::quad_approx(circle_parametrization, 3);

      not_so_precise_circle.set_width(2);
      not_so_precise_circle.set_stroke({ 0.8, 0.2, 0.2 });

      surface << not_so_precise_circle;

      std::ofstream outf("svg_quadratic_approximation.svg");
      outf << surface;
    }

    void rotating_circles()
    {
      Camera default_camera{FrameTransform{}, 10, 4, 4};

      // Relative position of unrotated spheres to center
      std::list< arma::Col<double> > sphere_rels;
      for(int i=0; i<27; i++) {
        if(i%3!=1 || (i/3)%3!=1 || (i/9)%3!=1 )
        {
          sphere_rels.push_back({ (double)(i%3-1), (double)((i/3)%3-1), (double)((i/9)%3-1) });
          sphere_rels.back()=1/arma::norm(sphere_rels.back())*sphere_rels.back();
        }
      }

      for(int i=0; i<360; i++)
      {
        svg::Surface surface({512, 512});
        std::list< arma::Col<double> > sphere_centers;
        for(auto sphere_rel: sphere_rels)
          sphere_centers.push_back( arma::Col<double>{ 0, 0, -3 }
                                   +FrameTransform::rotation_matrix({ 0, sin(0.3*2*M_PI*0), cos(0.3*2*M_PI*0) }, 2*M_PI/(double)360*2*i)*sphere_rel);
        sphere_centers.sort([](arma::Col<double> left, arma::Col<double> right)->bool{ return arma::norm(left)<arma::norm(right); });
        for(auto sphere_center: sphere_centers)
        {
          Sphere sphere(sphere_center, 0.2);
          svg::Path sphere_path = svg::quad_approx(
              [sphere, default_camera](double t)->arma::Col<double>
              {
                return 512*(sphere.contour_NDC(default_camera)(t)+arma::Col<double>{1, 1, 0})/2;
              },
              8);
          sphere_path.set_fill({1, 1, 1, 0.5});
          sphere_path.set_width(3);
          surface << sphere_path;
        }

        // output
        std::ofstream outf(std::string("rotating_circles_")+std::to_string(i)+".svg");
        outf << surface;
      }
    }
  }
};

int main()
{
  // minim::samples::svg_square();
  // minim::samples::svg_quadratic_approximation();
  // minim::samples::rotating_circles();
  return 0;
}
