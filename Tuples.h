#pragma once
#ifndef TUPLES_H
#define TUPLES_H
#include <iostream>
#include <thread>
#include <ctgmath>
#ifndef MECVISPI_WIN
#include <pthread.h>
#endif
#define PI 3.14159265359f
//generic 3 tuple used when saving points in 3D space
class Tup3 {
public:
	Tup3();
	Tup3(float _x, float _y, float _z);
	Tup3(const Tup3& other);
	~Tup3();
	Tup3& operator=(const Tup3& other);
	Tup3 operator+(const Tup3& other);
	Tup3 operator-(const Tup3& other);
	Tup3& operator+=(const Tup3& other);
	float x;
	float y;
	float z;

	void fill_xyz(Tup3& tup);
};
#endif

