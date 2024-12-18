// visit-struct-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#

#include "visit_struct.hpp"
#include "visit_struct_intrusive.hpp"

struct Vec3
{
	float x, y, z;
};

struct Z
{
	int z0;
	float z1;
	std::string z2;
};

struct A
{
	int m;
	float n;
	Z o;
};

struct B
{
	int a;
	float b;
	std::string c;
	Vec3 d;
	A e;
};

VISITABLE_STRUCT(Vec3, x, y, z);
VISITABLE_STRUCT(Z, z0, z1, z2);
VISITABLE_STRUCT(A, m, n, o);
VISITABLE_STRUCT(B, a, b, c, d, e);

struct printer {
	void operator()(const char* field, Vec3 vec)
	{
		std::cout << field << " = (" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;
	}

	// Primitive types
	template <typename T, std::enable_if_t<std::is_arithmetic<T>::value>* = nullptr>
	void operator()(const char* field, const T& value)
	{
		for (int i = 0; i < depth; i++) std::cout << "  ";
		std::cout << field << " = " << value << std::endl;
	}

	// Strings
	void operator()(const char* field, const std::string& value)
	{
		for (int i = 0; i < depth; i++) std::cout << "  ";
		std::cout << field << " = \"" << value << "\"" << std::endl;
	}
	
	// Structs
	template <typename T, typename = std::enable_if_t<visit_struct::traits::is_visitable<T>::value>>
	void operator()(const char* field, const T& value)
	{
		for (int i = 0; i < depth; i++) std::cout << "  ";
		std::cout << field << " = {" << std::endl;
		depth++;
		visit_struct::for_each(value, *this);
		depth--;
		for (int i = 0; i < depth; i++) std::cout << "  ";
		std::cout << "}" << std::endl;
	}

	int depth = 0;
};

int main()
{
	B b{ 1, 2.0f, "hello", {3.0f, 4.0f, 5.0f}, {6, 7.0f, {8, 9.0f, "world"}} };
	printer p;
	visit_struct::for_each(b, p);
}
