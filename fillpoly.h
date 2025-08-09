#include <vector>

struct color{
    float r;
    float g;
    float b;
    float a;

    color(): r(0), g(0), b(0), a(0){}

    color(float r, float g, float b, float a): r(r), g(g), b(b), a(a){}

    color operator +(const color &other){
        return color(r + other.r, g + other.g, b + other.b, a + other.a);
    }

    color &operator +=(const color &other){
        this->r += other.r;
        this->g += other.g;
        this->b += other.b;
        this->a += other.a;
        return *this;
    }

    color operator*(const color &other){
        return color(r * other.r, g * other.g, b * other.b, a * other.a);
    }

    color operator*(float other){
        return color(r * other, g * other, b * other, a * other);
    }
};

struct in_point {
    //algumas vezes relativo ao canvas
    int x; 
    int y;

    //a entrada eh int, nao aceitamos entradas float
    color col;

    in_point(int x, int y, float incol[4]): x(x), y(y){
        col.r = incol[0];
        col.g = incol[1];
        col.b = incol[2];
        col.a = incol[3];
    }
};

struct point{
    float x;
    float y;
    
    color col;

    point(float x, float y): x(x), y(y){}

    point(float x, float y, color col): x(x), y(y), col(col){}

    point(float x, float y, float incol[4]): x(x), y(y){
        col.r = incol[0];
        col.g = incol[1];
        col.b = incol[2];
        col.a = incol[3];
    }

    point(const in_point& ip): x(ip.x), y(ip.y), col(ip.col) {}
};

std::vector<point> fillpoly(const std::vector<in_point> &vertexes);
