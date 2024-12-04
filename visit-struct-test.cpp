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

// template <typename T, std::enable_if_t(!std::is_base_of<visit_struct::visitable_base, T>::value = 0)

struct printer {

	printer


	void operator()(const char* field, Vec3 vec)
	{
		std::cout << field << " = (" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;
	}

	// Primitive types
	template <typename T, std::enable_if_t<std::is_arithmetic<T>::value>* = nullptr>
	void operator()(const char* field, const T& value)
	{
		std::cout << field << " = " << value << std::endl;
	}

	// Strings
	void operator()(const char* field, const std::string& value)
	{
		std::cout << field << " = \"" << value << "\"" << std::endl;
	}
	
	// Structs
	template <typename T, typename = std::enable_if_t<visit_struct::traits::is_visitable<T>::value>>
	void operator()(const char* field, const T& value)
	{
		std::cout << field << " = {" << std::endl;
		visit_struct::for_each(value, *this);
		std::cout << "}" << std::endl;
	}

	/*
	template <typename T, typename = std::enable_if_t<visit_struct::traits::is_visitable<T>>::vallue = 0>
	void operator()(const char* field, const T& value)
	{
		printer p;
		visit_struct::for_each(value, p);
	}
	*/

	int depth = 0;
};

 

int main()
{
	B b{ 1, 2.0f, "hello", {3.0f, 4.0f, 5.0f}, {6, 7.0f, {8, 9.0f, "world"}} };
	printer p;
	visit_struct::for_each(b, p);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
