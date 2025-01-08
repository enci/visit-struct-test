// visit-struct-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <vector>

#include "visit_struct.hpp"
#include "visit_struct_intrusive.hpp"
#include "json.hpp"

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
	std::vector<int> v;
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
VISITABLE_STRUCT(A, m, n, o, v);
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
	

	// Vectors of primitive types
	template <typename T, std::enable_if_t<std::is_arithmetic<T>::value>* = nullptr>
	void operator()(const char* field, const std::vector<T>& value)
	{
		for (int i = 0; i < depth; i++) std::cout << "  ";
		std::cout << field << " = [";

		for (const auto& v : value)
			std::cout << v << ",";

		std::cout << "]" << std::endl;
	}

	// Vectors of structs
	template <typename T, typename = std::enable_if_t<visit_struct::traits::is_visitable<T>::value>>
	void operator()(const char* field, const std::vector<T>& value)
	{
		for (int i = 0; i < depth; i++) std::cout << "  ";
		std::cout << field << " = [" << std::endl;
		depth++;
		for (const auto& v : value)
			visit_struct::for_each(v, *this);
		depth--;
		for (int i = 0; i < depth; i++) std::cout << "  ";
		std::cout << "]" << std::endl;
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

struct to_json
{
	to_json() : json(nlohmann::json::object()) {}

	void operator()(const char* field, Vec3 vec)
	{
		json[field] = { vec.x, vec.y, vec.z };
	}

	// Primitive types
	template <typename T, std::enable_if_t<std::is_arithmetic<T>::value>* = nullptr>
	void operator()(const char* field, const T& value)
	{
		json[field] = value;
	}

	// Strings
	void operator()(const char* field, const std::string& value)
	{
		json[field] = value;
	}

	// Vectors of primitive types
	template <typename T, std::enable_if_t<std::is_arithmetic<T>::value>* = nullptr>
	void operator()(const char* field, const std::vector<T>& value)
	{
		json[field] = value;
	}

	// Vectors of structs
	template <typename T, typename = std::enable_if_t<visit_struct::traits::is_visitable<T>::value>>
	void operator()(const char* field, const std::vector<T>& value)
	{
		std::vector<nlohmann::json> j;
		for (const auto& v : value)
		{
			to_json tj;
			visit_struct::for_each(v, tj);
			j.push_back(tj.json);
		}
		json[field] = j;
	}

	// Structs
	template <typename T, typename = std::enable_if_t<visit_struct::traits::is_visitable<T>::value>>
	void operator()(const char* field, const T& value)
	{
		to_json tj;
		visit_struct::for_each(value, tj);
		json[field] = tj.json;
	}

	nlohmann::json json;
};

struct from_json
{
	from_json(const nlohmann::json& json) : json(json) {}

	void operator()(const char* field, Vec3& vec)
	{
		vec.x = json[field][0];
		vec.y = json[field][1];
		vec.z = json[field][2];
	}

	// Primitive types
	template <typename T, std::enable_if_t<std::is_arithmetic<T>::value>* = nullptr>
	void operator()(const char* field, T& value)
	{
		value = json[field];
	}

	// Strings
	void operator()(const char* field, std::string& value)
	{
		value = json[field];
	}

	// Vectors of primitive types
	template <typename T, std::enable_if_t<std::is_arithmetic<T>::value>* = nullptr>
	void operator()(const char* field, std::vector<T>& value)
	{
		value = json[field].get<std::vector<T>>();
	}

	// Vectors of structs
	template <typename T, typename = std::enable_if_t<visit_struct::traits::is_visitable<T>::value>>
	void operator()(const char* field, std::vector<T>& value)
	{
		for (const auto& j : json[field])
		{
			T t;
			from_json fj(j);
			visit_struct::for_each(t, fj);
			value.push_back(t);
		}
	}

	// Structs
	template <typename T, typename = std::enable_if_t<visit_struct::traits::is_visitable<T>::value>>
	void operator()(const char* field, T& value)
	{
		from_json fj(json[field]);
		visit_struct::for_each(value, fj);
	}

private:
	const nlohmann::json& json;
};

int main()
{
	std::cout << "=========================================================" << std::endl;
	B b{ 1, 2.0f, "hello", {3.0f, 4.0f, 5.0f}, {6, 7.0f, {8, 9.0f, "world"}, { -1, -2, -3 }} };
	printer p;
	visit_struct::for_each(b, p);

	to_json tj;
	visit_struct::for_each(b, tj);

	std::ofstream out("output.json");
	out << tj.json.dump(4) << std::endl;

	std::cout << "=========================================================" << std::endl;
	nlohmann::json json;
	std::ifstream in("output.json");
	in >> json;

	B b2;
	from_json fj(json);
	visit_struct::for_each(b2, fj);


	printer p2;
	visit_struct::for_each(b2, p2);
}
