#include "Tuples.h"

Tup3::Tup3() :x(0.f), y(0.f), z(0.f) {
	;
}
Tup3::~Tup3() {
	;
}
Tup3::Tup3(float _x, float _y, float _z) :x(_x), y(_y), z(_z) {
	;
}
Tup3::Tup3(const Tup3& other) {
	this->x = other.x;
	this->y = other.y;
	this->z = other.z;
}
Tup3& Tup3::operator=(const Tup3& other) {
	if (this != &other) {
		this->x = other.x;
		this->y = other.y;
		this->z = other.z;
	}
	return *this;
}
Tup3 Tup3::operator+(const Tup3& other) {
	Tup3 res;
	res.x = this->x + other.x;
	res.y = this->y + other.y;
	res.z = this->z + other.z;
	return res;
}
Tup3 Tup3::operator-(const Tup3& other) {
	Tup3 res;
	res.x = this->x - other.x;
	res.y = this->y - other.y;
	res.z = this->z - other.z;
	return res;
}
Tup3& Tup3::operator+=(const Tup3& other) {
	this->x += other.x;
	this->y += other.y;
	this->z += other.z;
	return *this;
}
void Tup3::fill_xyz(Tup3& tup) {
	this->x = tup.x;
	this->y = tup.y;
	this->z = tup.z;
}
