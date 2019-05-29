#include <dseed.h>

dseed::point2i::point2i (int32_t x, int32_t y) : x (x), y (y) { }
dseed::point2f::point2f (float x, float y) : x (x), y (y) { }
dseed::point3i::point3i (int32_t x, int32_t y, int32_t z) : x (x), y (y), z (z) { }
dseed::point3f::point3f (float x, float y, float z) : x (x), y (y), z (z) { }
dseed::size2i::size2i (int32_t width, int32_t height) : width (width), height (height) { }
dseed::size2f::size2f (float width, float height) : width (width), height (height) { }
dseed::size3i::size3i (int32_t width, int32_t height, int32_t depth) : width (width), height (height), depth (depth) { }
dseed::size3f::size3f (float width, float height, float depth) : width (width), height (height), depth (depth) { }

dseed::rectangle::rectangle (int32_t x, int32_t y, int32_t width, int32_t height)
	: x (x), y (y), width (width), height (height)
{ }
dseed::rectangle::rectangle (const point2i& p, const size2i& s)
	: x (p.x), y (p.y), width (s.width), height (s.height)
{ }
bool dseed::rectangle::intersect (const rectangle& rect)
{
	return x <= rect.x + rect.width && rect.x <= x + width &&
		y <= rect.y + rect.height && rect.y <= y + height;
}

dseed::rectanglef::rectanglef (float x, float y, float width, float height)
	: x (x), y (y), width (width), height (height)
{ }
dseed::rectanglef::rectanglef (const point2f& p, const size2f& s)
	: x (p.x), y (p.y), width (s.width), height (s.height)
{ }
bool dseed::rectanglef::intersect (const rectanglef& rect)
{
	return x <= rect.x + rect.width && rect.x <= x + width &&
		y <= rect.y + rect.height && rect.y <= y + height;
}

dseed::circle::circle (int32_t x, int32_t y, int32_t radius)
	: x (x), y (y), radius (radius)
{ }
bool dseed::circle::intersect (const circle& circle)
{
	return sqrt (pow (x - circle.x, 2) + pow (y - circle.y, 2))
		<= (radius + (double)circle.radius);
}

dseed::circlef::circlef (float x, float y, float radius)
	: x (x), y (y), radius (radius)
{ }
bool dseed::circlef::intersect (const circlef& circle)
{
	return sqrt (pow (x - circle.x, 2) + pow (y - circle.y, 2))
		<= radius + circle.radius;
}