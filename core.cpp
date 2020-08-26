#include <list>
#include <valarray>
#include <array>
#include <cmath>

//#include <cairo.h>

typedef std::valarray<float> Point;

float dot(Point x, Point y)
{
	return (x*y).sum();
}

Point cross(Point x, Point y)
{
	return Point{ x[1]*y[2]-y[1]*x[2], x[2]*y[0]-y[2]*x[0], x[0]*y[1]-y[0]*x[1] };
}

float norm(Point x)
{
	return sqrt(dot(x, x));
}


struct ReferenceFrame
{
	std::array<Point, 3> axes;
	Point origin;
};

class Path
{
private:
	std::list< Point > nodes;

public:
	Path()
	{}

	Path(std::list< Point > nodes): nodes{ nodes }
	{}

	void add_node(Point node, bool back=true)
	{
		if(back) nodes.push_back(node);
		else     nodes.push_front(node);
	}

	void erase_node(bool back=true)
	{
		if(back) nodes.pop_back();
		else     nodes.pop_front();
	}

	std::list< Point > as_list()
	{
		return nodes;
	}
};

class Contour
{
public:

	virtual Path get_path();

	friend class Path;
};

class Ellipse : public Contour
{
private:
	Point center, first_vertex, second_vertex;
	int path_nodes;

public:
	Ellipse(const Point center, const std::array< Point, 2 > vertexes,
		const int path_nodes=32): center{ center }, first_vertex{ vertexes[0] }, second_vertex{ vertexes[1] }, path_nodes{ path_nodes }
	{}

	Ellipse(const Point center, const Point radius_vector, const int path_nodes=32): center{ center }, path_nodes{ path_nodes }
	{
		double radius=norm(radius_vector);
		if( fabs(radius_vector[1])>0.01*radius || fabs(radius_vector[2])>0.01*radius )
			first_vertex=Point{ 1, 0, 0 };
		else
			first_vertex=Point{ 0, 1, 0 };

		first_vertex=radius_vector-dot(radius_vector, first_vertex)*first_vertex;
		first_vertex=1/norm(first_vertex)*first_vertex;
		second_vertex=cross(radius_vector, first_vertex);
		first_vertex=radius*first_vertex;
	}
	
	Path get_path()
	{
		std::list< Point > nodes;
		const double dtheta=2*M_PI/path_nodes;
		for(int i=0; i<path_nodes; i++) nodes.push_back( center + cos(i*dtheta)*first_vertex + sin(i*dtheta)*second_vertex );
		nodes.push_back( nodes.front() );
		return Path{ nodes };
	}
};

class Shape
{
public:
	
	virtual Contour contour(Point point_of_view);
};

class Sphere : public Shape
{
private:
	Point center;
	float radius;

public:
	Sphere(Point center, float radius): center{ center }, radius{ radius }
	{}

	Contour contour(Point point_of_view)
	{
		Point normal_vector = point_of_view-center;
		normal_vector = 1/norm(normal_vector)*normal_vector;
		return Ellipse(center, radius*normal_vector);
	}
};

std::list< Point > project(std::list< Point > points, float field_of_view=M_PI/2)
{
	std::list< Point > ret;
	float scale=1/tan(0.5*field_of_view);
	for(auto point: points)
		ret.push_back({ -scale*point[0]/point[2], -scale*point[1]/point[2], -point[2] });
	return ret;
}

int main()
{
	Sphere my_sphere({0, 0, -3}, 1);
	my_sphere.contour({0, 0, 0});

	return 0;
}
