#pragma once


class Wall
{
    public:
    float x;
    float y;
    float length;
    float angle;
    float maxy;
    float maxx;
    float minx;
    float miny;
    Wall(float,float,float,float);
};

void Set(Wall);
