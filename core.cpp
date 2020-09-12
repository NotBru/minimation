#include <armadillo>
#include <list>
#include <cmath>
#include <cairo.h>

#include <iomanip>
#include <iostream>

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
        traslation{ -origin }, rotscale{ arma::inv(arma::join_rows( axes[0], axes[1], axes[2] ) ) }
    {}

    static arma::Mat<double> rotation_matrix(arma::Col<double> versor, double angle);

    FrameTransform &rotate(const arma::Col<double> &);

    FrameTransform &traslate(const arma::Col<double> &);

    arma::Col<double> origin() const
    {
        return -traslation;
    }

    friend FrameTransform operator*(const FrameTransform &, const FrameTransform &);

    friend arma::Col<double> operator*(const FrameTransform &, const arma::Col<double> &);

};

class Camera
{
private:
    FrameTransform frame;
    double focal_distance, half_width, half_height;

public:
    Camera(const FrameTransform &frame=FrameTransform{}, double focal_distance=1, double width=2, double height=2):
        frame{ frame },
        focal_distance{ focal_distance },
        half_width{ width/2 },
        half_height{ height/2 }
    {}

    arma::Col<double> origin() const
    {
        return frame.origin();
    }

    void move_projection_plane(double move)
    {
        focal_distance+=move;
    }

    Camera &traslate(const arma::Col<double> &displacement)
    {
        frame.traslate(displacement);
        return *this;
    }

    arma::Col<double> project(arma::Col<double> point) const;

    std::list< arma::Col<double> > project(std::list< arma::Col<double> > point) const;

    arma::Col<double> project_NDC(arma::Col<double>) const;

    std::list< arma::Col<double> > project_NDC(std::list< arma::Col<double> >) const;

    static Camera from_polars(const double rho=1, const double phi=0, const double theta=M_PI/2, const double radial=0);
};

class Ellipse;

class Circle
{
private:
    arma::Col<double> center, radius;

public:
    Circle(const arma::Col<double> &center, const arma::Col<double> &radius): center{ center }, radius{ radius }
    {}

    Circle(const arma::Col<double> &center, const arma::Col<double> &normal, const double radius): center{ center }, radius{ radius*normal/arma::norm(normal) }
    {}

    std::list< arma::Col<double> > as_path(const int points=180);
    
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

    std::list< arma::Col<double> > as_path(const int points=180);
};

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

    std::list< arma::Col<double> > contour(const Camera &);
};

namespace tests
{
    void normal_circle()
    {
        Circle circle({0, 0, -5}, {0, 0, 1});
        auto path = circle.as_path(4);
        for(auto point: path) {
            if(fabs(arma::norm(point-arma::Col<double>{0, 0, -5})-1)>0.000001)
            {
                std::cout << "Circle thingy not working.\n";
            }
        }
    }

    void draw_normal_circle()
    {
        //TODO: Hide all cairo details in Scene
        auto path = Camera{}.project_NDC(Circle({0, 0, -5}, {0, 0, 1}, 0.5).as_path(128));
        cairo_surface_t *surface;
        cairo_t *cr;

        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 512, 512);
        cr = cairo_create(surface);
        cairo_surface_set_device_scale(surface, 512, 512);
        cairo_set_line_width(cr, 0.01);

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_paint(cr);

        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_move_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
        for(auto point: path)
            cairo_line_to(cr, point[0]+0.5, point[1]+0.5);
        cairo_line_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
        cairo_stroke(cr);

        cairo_destroy(cr);
        cairo_surface_destroy(surface);
    }

    void draw_spinning_circle()
    {
        //TODO: Hide all cairo details in Scene
        double dtheta=2*M_PI/180;
        for(int i=0; i<720; i++)
        {
            auto path = Camera{}.project_NDC(Circle({0, 0, -5+2*sin(i*dtheta)}, {cos(i*dtheta), 0, sin(i*dtheta)}, 0.5).as_path(128));
            cairo_surface_t *surface;
            cairo_t *cr;
    
            surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 512, 512);
            cr = cairo_create(surface);
            cairo_surface_set_device_scale(surface, 512, 512);
            cairo_set_line_width(cr, 0.01);
    
            cairo_set_source_rgb(cr, 0, 0, 0);
            cairo_paint(cr);
    
            cairo_set_source_rgb(cr, 1, 1, 1);
            cairo_move_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
            for(auto point: path)
                cairo_line_to(cr, point[0]+0.5, point[1]+0.5);
            cairo_line_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
            cairo_stroke(cr);
    

            std::ostringstream stringStream;
            stringStream << "test_draw_spinning_circle_" << std::setfill('0') << std::setw(3) << i << ".png";
            cairo_surface_write_to_png(surface, stringStream.str().c_str()); // Memory leak, too lazy to fix
    
            // ffmpeg -framerate $FRAMERATE -i "pic%03d.png" output.mp4
            // ffmpeg -framerate $FRAMERATE -i "pic%03d.png" -c:v libx264 -profile:v baseline -level 3.0 -pix_fmt yuv420p output.mp4
            cairo_destroy(cr);
            cairo_surface_destroy(surface);
        }
    }

    void rotating_frame_circle()
    {
        //TODO: Fix this
        double dtheta=2*M_PI/180;
        std::list<Circle> circles;
        std::list< std::vector< double > > colors;
        circles.push_back(Circle({0, 0, 0}, {0, 0, 1}));
        circles.push_back(Circle({0, 0, 0}, {0, 1, 0}));
        circles.push_back(Circle({0, 0, 0}, {1, 0, 0}));
        colors.push_back({ 0, 0, 1 });
        colors.push_back({ 0, 1, 0 });
        colors.push_back({ 1, 0, 0 });
        for(int i=0; i<720; i++)
        {
            auto camera = Camera::from_polars(5.0, i*dtheta, M_PI/2-i*dtheta/2);
            cairo_surface_t *surface;
            cairo_t *cr;
    
            surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 512, 512);
            cr = cairo_create(surface);
            cairo_surface_set_device_scale(surface, 512, 512);
            cairo_set_line_width(cr, 0.01);
    
            cairo_paint(cr);
    
            cairo_set_source_rgb(cr, 1, 1, 1);
            auto it_circles=circles.begin();
            auto it_colors=colors.begin();
            while(it_circles!=circles.end())
            {
                cairo_set_source_rgb(cr, (*it_colors)[0], (*it_colors)[1], (*it_colors)[2]);
                auto path = camera.project_NDC(it_circles->as_path(128));
                cairo_move_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
                for(auto point: path)
                    cairo_line_to(cr, point[0]+0.5, point[1]+0.5);
                cairo_line_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
                cairo_stroke(cr);
                ++it_circles;
                ++it_colors;
            }

            std::ostringstream stringStream;
            stringStream << "test_rotating_frame_circle_" << std::setfill('0') << std::setw(3) << i << ".png";
            cairo_surface_write_to_png(surface, stringStream.str().c_str()); // Memory leak, too lazy to fix
    
            // ffmpeg -framerate $FRAMERATE -i "pic%03d.png" output.mp4
            // ffmpeg -framerate $FRAMERATE -i "pic%03d.png" -c:v libx264 -profile:v baseline -level 3.0 -pix_fmt yuv420p output.mp4
            cairo_destroy(cr);
            cairo_surface_destroy(surface);
        }
    }

    void rotating_frame_axis()
    {
        //TODO: Fix this
        double dtheta=2*M_PI/180;
        std::list< std::list< arma::Col<double> > > axes;
        std::list< std::vector< double > > colors;
        axes.push_back({ arma::Col<double>{0, 0, 0}, arma::Col<double>{0, 0, 1} });
        axes.push_back({ arma::Col<double>{0, 0, 0}, arma::Col<double>{0, 1, 0} });
        axes.push_back({ arma::Col<double>{0, 0, 0}, arma::Col<double>{1, 0, 0} });
        colors.push_back({ 0, 0, 1 });
        colors.push_back({ 0, 1, 0 });
        colors.push_back({ 1, 0, 0 });
        for(int i=0; i<720; i++)
        {
            auto camera = Camera::from_polars(5.0, i*dtheta, M_PI/2-i*dtheta/2);
            cairo_surface_t *surface;
            cairo_t *cr;
    
            surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 512, 512);
            cr = cairo_create(surface);
            cairo_surface_set_device_scale(surface, 512, 512);
            cairo_set_line_width(cr, 0.01);
    
            cairo_paint(cr);
    
            cairo_set_source_rgb(cr, 1, 1, 1);
            auto it_axes=axes.begin();
            auto it_colors=colors.begin();
            while(it_axes!=axes.end())
            {
                cairo_set_source_rgb(cr, (*it_colors)[0], (*it_colors)[1], (*it_colors)[2]);
                auto path = camera.project_NDC(*it_axes);
                cairo_move_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
                for(auto point: path)
                    cairo_line_to(cr, point[0]+0.5, point[1]+0.5);
                cairo_line_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
                cairo_stroke(cr);
                ++it_axes;
                ++it_colors;
            }

            std::ostringstream stringStream;
            stringStream << "test_rotating_frame_axes" << std::setfill('0') << std::setw(3) << i << ".png";
            cairo_surface_write_to_png(surface, stringStream.str().c_str()); // Memory leak, too lazy to fix
    
            // ffmpeg -framerate $FRAMERATE -i "pic%03d.png" output.mp4
            // ffmpeg -framerate $FRAMERATE -i "pic%03d.png" -c:v libx264 -profile:v baseline -level 3.0 -pix_fmt yuv420p output.mp4
            cairo_destroy(cr);
            cairo_surface_destroy(surface);
        }
    }

    void rotating_frame_axis_b()
    {
        //TODO: Fix this
        double dtheta=2*M_PI/180;
        std::list< std::list< arma::Col<double> > > axes;
        std::list< std::vector< double > > colors;
        axes.push_back({ arma::Col<double>{0, 0, 0}, arma::Col<double>{0, 0, 1} });
        axes.push_back({ arma::Col<double>{0, 0, 0}, arma::Col<double>{0, 1, 0} });
        axes.push_back({ arma::Col<double>{0, 0, 0}, arma::Col<double>{1, 0, 0} });
        colors.push_back({ 0, 0, 1 });
        colors.push_back({ 0, 1, 0 });
        colors.push_back({ 1, 0, 0 });
        for(int i=0; i<720; i++)
        {
            auto camera = Camera::from_polars(5.0, 0, M_PI/2-M_PI/8, i*dtheta/4);
            cairo_surface_t *surface;
            cairo_t *cr;
    
            surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 512, 512);
            cr = cairo_create(surface);
            cairo_surface_set_device_scale(surface, 512, 512);
            cairo_set_line_width(cr, 0.01);
    
            cairo_paint(cr);
    
            cairo_set_source_rgb(cr, 1, 1, 1);
            auto it_axes=axes.begin();
            auto it_colors=colors.begin();
            while(it_axes!=axes.end())
            {
                cairo_set_source_rgb(cr, (*it_colors)[0], (*it_colors)[1], (*it_colors)[2]);
                auto path = camera.project_NDC(*it_axes);
                cairo_move_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
                for(auto point: path)
                    cairo_line_to(cr, point[0]+0.5, point[1]+0.5);
                cairo_line_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
                cairo_stroke(cr);
                ++it_axes;
                ++it_colors;
            }

            std::ostringstream stringStream;
            stringStream << "test_rotating_frame_axes_b" << std::setfill('0') << std::setw(3) << i << ".png";
            cairo_surface_write_to_png(surface, stringStream.str().c_str()); // Memory leak, too lazy to fix
    
            // ffmpeg -framerate $FRAMERATE -i "pic%03d.png" output.mp4
            // ffmpeg -framerate $FRAMERATE -i "pic%03d.png" -c:v libx264 -profile:v baseline -level 3.0 -pix_fmt yuv420p output.mp4
            cairo_destroy(cr);
            cairo_surface_destroy(surface);
        }
    }

};

void follow_no_join(cairo_t *cr, std::list< arma::Col<double> > path)
{
    cairo_move_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
    for(auto point: path) cairo_line_to(cr, point[0]+0.5, point[1]+0.5);
}

void follow(cairo_t *cr, std::list< arma::Col<double> > path)
{
    cairo_move_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
    for(auto point: path) cairo_line_to(cr, point[0]+0.5, point[1]+0.5);
    cairo_line_to(cr, path.front()[0]+0.5, path.front()[1]+0.5);
}

int main()
{
    cairo_surface_t *surface;
    cairo_t *cr;
    
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 512, 512);
    cr = cairo_create(surface);
    cairo_surface_set_device_scale(surface, 512, 512);
    cairo_set_line_width(cr, 0.01);
    
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    
    auto camera = Camera::from_polars(3.5, M_PI/2/2, M_PI/2-M_PI/6);
    camera.move_projection_plane(1);
    camera.traslate(arma::Col<double>{0, -0.4, 0.2});

    // Axes behind sphere
    cairo_set_source_rgb(cr, 0.4, 0, 0.4);
    cairo_set_line_width(cr, 0.007);

    follow(cr, camera.project({ arma::Col<double>{0, 0, 0}, arma::Col<double>{1, 0, 0} }));
    cairo_stroke(cr);
    follow(cr, camera.project({ arma::Col<double>{0, 0, 0}, arma::Col<double>{0, 1, 0} }));
    cairo_stroke(cr);
    follow(cr, camera.project({ arma::Col<double>{0, 0, 0}, arma::Col<double>{0, 0, 1} }));
    cairo_stroke(cr);

    // omega vector
    double omega_x=0.4, omega_z=2*omega_x,
           q=1.2;
    cairo_set_line_width(cr, 0.007);
    cairo_set_source_rgb(cr, 0, 0.8, 0.4);
    follow(cr, camera.project({ arma::Col<double>{0, 0, 0}, arma::Col<double>{omega_x, 0, omega_z} }));
    cairo_stroke(cr);
    cairo_set_line_width(cr, 0.004);
    follow(cr, camera.project({ arma::Col<double>{0, 0, 0}, arma::Col<double>{omega_x, 0, 0} }));
    cairo_stroke(cr);
    follow(cr, camera.project({ arma::Col<double>{0, 0, 0}, arma::Col<double>{0, 0, omega_z} }));
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0.3, 1, 0.7);
    follow(cr, camera.project({ arma::Col<double>{omega_x, 0, omega_z}, arma::Col<double>{q*omega_x, 0, omega_z} }));
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0, 1, 0.5);
    follow(cr, camera.project({ arma::Col<double>{0, 0, 0}, arma::Col<double>{q*omega_x, 0, omega_z} }));
    cairo_stroke(cr);

    // Sphere fill
    cairo_set_line_width(cr, 0.01);
    cairo_set_source_rgba(cr, 0, 0, 0, 0.4);
    follow(cr, Sphere(arma::Col<double>{0, 0, 0}, 1).contour(camera));
    cairo_fill(cr);

    // Axes in front of sphere
    cairo_set_source_rgb(cr, 0.4, 0, 0.4);
    cairo_set_line_width(cr, 0.007);

    follow(cr, camera.project({ arma::Col<double>{1, 0, 0}, arma::Col<double>{1.5, 0, 0} }));
    cairo_stroke(cr);
    follow(cr, camera.project({ arma::Col<double>{0, 1, 0}, arma::Col<double>{0, 1.5, 0} }));
    cairo_stroke(cr);
    follow(cr, camera.project({ arma::Col<double>{0, 0, 1}, arma::Col<double>{0, 0, 1.5} }));
    cairo_stroke(cr);

    // Trajectory
    cairo_set_line_width(cr, 0.004);
    cairo_set_source_rgb(cr, 0, 0.8, 0.8);
    {
        int points=128; 
        arma::Col<double> omega{ omega_x, 0, omega_z };
        double dtheta=0.8*2*M_PI/points;
        omega = omega/arma::norm(omega);
        std::list< arma::Col<double> > traj;
        for(int i=0; i<points; i++) traj.push_back(FrameTransform::rotation_matrix(omega, i*dtheta)*arma::Col<double>{0, 0, 1});
        std::cout << traj.back() << "\n";
        traj = camera.project(traj);
        follow_no_join(cr, traj);
        cairo_stroke(cr);
    }

    // Delta trajectory

    cairo_set_line_width(cr, 0.004);
    cairo_set_source_rgb(cr, 0, 0.9, 0.9);
    {
        int points=128; 
        arma::Col<double> omega{ q*omega_x, 0, omega_z };
        double dtheta=0.8*2*M_PI*arma::norm(omega)/arma::norm(arma::Col<double>{ omega_x, 0, omega_z })/points;
        omega=omega/arma::norm(omega);
        std::list< arma::Col<double> > traj;
        for(int i=0; i<points; i++) traj.push_back(FrameTransform::rotation_matrix(omega, i*dtheta)*arma::Col<double>{0, 0, 1});
        std::cout << traj.back() << "\n";
        traj = camera.project(traj);
        follow_no_join(cr, traj);
        cairo_stroke(cr);
    }
    
    cairo_surface_write_to_png(surface, "test.png");
    
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return 0;
}

arma::Mat<double> FrameTransform::rotation_matrix(arma::Col<double> versor, double theta)
{
    double x{versor[0]}, y{versor[1]}, z{versor[2]};
    return arma::Mat<double>{ { cos(theta)+x*x*(1-cos(theta)), x*y*(1-cos(theta))-z*sin(theta), x*z*(1-cos(theta))+y*sin(theta) },
                              { x*y*(1-cos(theta))+z*sin(theta), cos(theta)+y*y*(1-cos(theta)), y*z*(1-cos(theta))-x*sin(theta) },
                              { x*z*(1-cos(theta))-y*sin(theta), y*z*(1-cos(theta))+x*sin(theta), cos(theta)+z*z*(1-cos(theta)) } };
}

FrameTransform &FrameTransform::traslate(const arma::Col<double> &displacement)
{
    traslation = traslation - displacement;
    return *this;
}

FrameTransform &FrameTransform::rotate(const arma::Col<double> &rotation)
{
    double theta=arma::norm(rotation);
    rotscale = rotscale*arma::inv(rotation_matrix(1/theta*rotation, theta));
    return *this;
}

FrameTransform operator*(const FrameTransform &left, const FrameTransform &right)
{
    FrameTransform ret;
    ret.traslation = right.rotscale*right.traslation+left.rotscale ;
    ret.rotscale = left.rotscale*right.rotscale;
    return ret;
}

arma::Col<double> operator*(const FrameTransform &left, const arma::Col<double> &right)
{
    return left.rotscale*(right+left.traslation);
}

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

arma::Col<double> Camera::project(arma::Col<double> point) const
{
    point = frame*point;
    point[2] = -point[2]/focal_distance;
    return { point[0]/point[2], point[1]/point[2], point[2] };
}

std::list< arma::Col<double> > Camera::project(std::list< arma::Col<double> > points) const
{
    std::list< arma::Col<double> > ret;
    for(auto point: points) ret.push_back(project(point));
    return ret;
}

arma::Col<double> Camera::project_NDC(arma::Col<double> point) const
{
    point = frame*point;
    point[2] = -point[2]/focal_distance;
    return { half_width*point[0]/point[2], half_height*point[1]/point[2], point[2] }; //WARNING: no es exactamente NDC, ya que z debería ir de 0 a 1 tmb.
}

std::list< arma::Col<double> > Camera::project_NDC(std::list< arma::Col<double> > points) const
{
    std::list< arma::Col<double> > ret;
    for(auto point: points) ret.push_back(project_NDC(point));
    return ret;
}

std::list< arma::Col<double> > Circle::as_path(const int points)
{
    return Ellipse(*this).as_path(points); //Ah re
}

std::list< arma::Col<double> > Ellipse::as_path(const int points)
{
    double dtheta=2*M_PI/points;
    std::list< arma::Col<double> > ret;
    for(int i=0; i<points; i++) ret.push_back(arma::Col<double>{ center+axis1*cos(i*dtheta)+axis2*sin(i*dtheta) });
    return ret;
}

std::list< arma::Col<double> > Sphere::contour(const Camera &camera)
{
    arma::Col<double> rel=center-camera.origin();
    double norm=arma::norm(rel);
    double q=radius*radius/arma::norm(rel)/arma::norm(rel);
    return camera.project(Circle((1-q)*rel+camera.origin(), rel, sqrt(radius*radius-q*q*norm*norm)).as_path());
}
