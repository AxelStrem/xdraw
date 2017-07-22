#pragma once
#include <algorithm>
#include <utility>
#include <vector>

#include <windef.h>

namespace RM
{

	struct Point
	{
		int x;
		int y;
	};

	struct Vector
	{
		int x;
		int y;
	};

	struct Size
	{ 
		Vector v;
		int area;

		Size() = default;
		Size(Vector v_) : v(v_), area(v_.x*v_.y) {}
	};

	struct Rect
	{
		Point ul;
		Point dr;
		Size s;

		Rect() = default;
		Rect(Point a, Point b) : ul(a), dr(b), s(Vector{ b.x - a.x,b.y - a.y })
		{}
		Rect(int x, int y, int sx, int sy) : ul{ x,y }, dr{ x + sx, y + sy }, s(Vector{ sx, sy })
		{}
		Rect(RECT r) : Rect(RM::Point{ r.left, r.top }, RM::Point{ r.right, r.bottom })
		{}

		int Area() const { return s.area; }
		int Top() const { return ul.y; }
		int Bottom() const { return dr.y; }
		int Left() const { return ul.x; }
		int Right() const { return dr.x; }
		int Width() const { return s.v.x; }
		int Height() const { return s.v.y; }

		RECT toRECT() const
		{
			RECT r;
			r.left = Left();
			r.right = Right();
			r.top = Top();
			r.bottom = Bottom();
			return r;
		}
	};

	Point MinPoint(Point a, Point b);

	Point MaxPoint(Point a, Point b);

	Rect CombineRects(Rect a, Rect b);

	bool Intersects(Rect a, Rect b);

	Rect Intersection(Rect a, Rect b);

	std::vector<Rect> Difference(Rect a, Rect b);

	class RectSet
	{
		std::vector<Rect> mRects;
	public:
		void Add(Rect r);

		void Intersect(Rect r);
		void Intersect(RectSet r);

		void Subtract(Rect r);
		void Subtract(RectSet r);

		std::vector<Rect>::iterator begin();
		std::vector<Rect>::iterator end();
	};

	class RectMerger
	{
		std::vector<Rect> rlist;
		int total_area;
	public:
		RectMerger();
		~RectMerger();

		void AddNotIntersecting(Rect r);
		void AddRect(Rect r);
		bool IsEmpty() const;
		void Clear();

		bool Intersects(Rect r) const;

		int GetArea() const;

		std::vector<Rect>::const_iterator begin() const;
		std::vector<Rect>::const_iterator end() const;

		RectMerger ExtractRect(Rect r);
	};

};