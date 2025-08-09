#include "fillpoly.h"

#include <limits>
#include <algorithm>
#include <cmath>
#include <cassert>

using block = std::vector<std::vector<point>>;

int n_scanlines(const std::vector<in_point> &vertexes, float &y_min, float &y_max){
    for(size_t i = 0; i < vertexes.size(); ++i){
        if(vertexes[i].y > y_max){
            y_max = vertexes[i].y;
        }

        if(vertexes[i].y < y_min){
            y_min = vertexes[i].y;
        }
    }    

    return y_max - y_min;
}

void process_points_y(block &intersections, float y_min, point p1, point p2){
    if(p1.y > p2.y){
        std::swap(p1, p2);
    }

    float yv = p2.y - p1.y;

    if(yv == 0){ //horizontal
        return;
    }

    float tx = (p2.x - p1.x) / yv; //1/m
    
    color tcolor = {
        (p2.col.r - p1.col.r) / yv,
        (p2.col.g - p1.col.g) / yv,
        (p2.col.b - p1.col.b) / yv,
        (p2.col.a - p1.col.a) / yv
    };

    int scanline = p1.y - y_min;

    float cur_x = p1.x;

    color cur_color = p1.col;

    for(int cur_y = p1.y; cur_y < (int)p2.y; ++cur_y, ++scanline){
        point pnt{cur_x, (float)cur_y, cur_color};

        intersections[scanline].push_back(pnt);

        cur_x += tx;
        cur_color += tcolor;
    }
}

std::vector<point> process_points_x(block& intersections){
    std::vector<point> out;

    for(size_t scanline = 0; scanline < intersections.size(); ++scanline) {
        auto cur = intersections[scanline];

        for(size_t i = 0; i + 1 < cur.size(); i += 2){
            auto p1 = cur[i];
            auto p2 = cur[i+1];

            float xv = (p2.x - p1.x);

            if(xv == 0){
                continue;
            }

            color tcolor = {
                (p2.col.r - p1.col.r) / xv,
                (p2.col.g - p1.col.g) / xv,
                (p2.col.b - p1.col.b) / xv,
                (p2.col.a - p1.col.a) / xv
            };

            float walked_inbetween_point = (ceil(p1.x) - p1.x);
            //imagine x = 8.5, ceil(x) = 9, (9 - 8.5) = 0.5, aplicamos 0.5 da interpol

            color cur_color = p1.col;

            cur_color += tcolor * walked_inbetween_point;

            for(float cur_x = ceil(p1.x); cur_x < floor(p2.x); ++cur_x){
                out.push_back({(float)cur_x, p1.y, cur_color});
                
                cur_color += tcolor;
            }
        }
    }

    return out;
}

std::vector<point> fillpoly(const std::vector<in_point> &vertexes){
    float y_max = -1, y_min = std::numeric_limits<float>::max();

    int scanlines = n_scanlines(vertexes, y_min, y_max);

    block intersections;
    intersections.reserve(scanlines);

    for(int i = 0; i < scanlines; ++i){
        intersections.push_back({});
    }

    for(size_t i = 0; i + 1 < vertexes.size(); ++i){
        auto p1 = vertexes[i];
        auto p2 = vertexes[i+1];

        process_points_y(intersections, y_min, p1, p2);
    }

    process_points_y(intersections, y_min, vertexes[vertexes.size()-1], vertexes[0]);

    for(int i = 0; i < scanlines; ++i){
        auto &cur = intersections[i];

        std::sort(cur.begin(), cur.end(), [](auto &a, auto &b){
            return a.x < b.x;
        });
    }

    return process_points_x(intersections);
}

/*int main(){
    std::vector<point> rect = {
        {20, 20},
        {80, 10},
        {140, 40},
        {110, 80},
        {60, 100},
        {20, 70}
    };

    auto result = fillpoly(rect);

    for(int i = 0; i < result.size(); ++i){
        std::cout << result[i].x << ' ' << result[i].y << '\n';
    }
}*/
