#include "stdafx.h"
#include "RectMerger.h"

namespace RM
{

	RectMerger::RectMerger()
	{
	}


	RectMerger::~RectMerger()
	{
	}

	void RM::RectMerger::AddNotIntersecting(Rect r)
	{
		for (int i = 0; i < static_cast<int>(rlist.size()); i++)
		{
			Rect tmp = CombineRects(rlist[i], r);
			if (tmp.Area() <= (r.Area() + rlist[i].Area()))
			{
				rlist[i] = rlist.back();
				rlist.pop_back();
				return AddNotIntersecting(tmp);
			}
		}

		total_area += r.Area();
		rlist.push_back(r);
	}

	void RM::RectMerger::AddRect(Rect r)
	{
		std::vector<Rect> c_rects = {r};
		std::vector<Rect> n_rects;
		for (int i = 0; i < static_cast<int>(rlist.size()); i++)
		{
			n_rects.clear();
			for (Rect l : c_rects)
			{
				if (RM::Intersects(rlist[i], l))
				{
					auto d = RM::Difference(l, rlist[i]);
					n_rects.insert(n_rects.end(), d.begin(), d.end());
				}
				else
					n_rects.push_back(l);
			}

			std::swap(c_rects, n_rects);
			if (c_rects.empty())
				break;
		}

		for (Rect l : c_rects)
			AddNotIntersecting(l);

		/*int index = -1;
		float area_ratio = 2048.f;
		Rect combined_rect;
		for (int i = 0; i < static_cast<int>(rlist.size()); i++)
		{
			Rect tmp = CombineRects(rlist[i], r);
			float current_ratio = static_cast<float>(tmp.Area()) / (r.Area() + rlist[i].Area());

			if ((current_ratio<=1.01f)&&(current_ratio < area_ratio))
			{
				area_ratio = current_ratio;
				index = i;
				combined_rect = tmp;
			}
		}

		if (index == -1)
		{
			rlist.push_back(r);
			total_area += r.Area();
			return;
		}

		total_area -= rlist[index].Area();
		rlist[index] = rlist.back();
		rlist.pop_back();
		
		AddRect(combined_rect);*/
	}

	bool RM::RectMerger::IsEmpty() const
	{
		return rlist.empty();
	}

	void RectMerger::Clear()
	{
		rlist.clear();
		total_area = 0;
	}

	bool RectMerger::Intersects(Rect r) const
	{
		return std::any_of(rlist.begin(), rlist.end(), [&r](Rect t) { return RM::Intersects(r, t); });
	}

	int RectMerger::GetArea() const
	{
		return total_area;
	}

	std::vector<Rect>::const_iterator RectMerger::begin() const
	{
		return rlist.cbegin();
	}

	std::vector<Rect>::const_iterator RectMerger::end() const
	{
		return rlist.cend();
	}

	RectMerger RectMerger::ExtractRect(Rect r)
	{
		RectMerger result;

		/*rlist = std::vector<Rect>{ RM::Rect{0,0,500,500},
			RM::Rect{ 500,0,500,500 },
			RM::Rect{ 0,500,500,500 },
			RM::Rect{ 500,500,500,500 } };

		r = RM::Rect{ 250,250,500,500 };*/

		std::vector<Rect> old;
		std::swap(old, rlist);
		total_area = 0;

		std::vector<Rect> n_rects;
		std::vector<Rect> c_rects{ r };
		
		for (Rect l : old)
		{
			n_rects.clear();
			RectSet current_area;
			current_area.Add(l);
			for (Rect q : c_rects)
			{
				if (RM::Intersects(q, l))
				{
					result.AddNotIntersecting(Intersection(q, l));
					auto d1 = Difference(q, l);
					n_rects.insert(n_rects.end(), d1.begin(), d1.end());
					
					current_area.Subtract(q);
				}
				else
				{
					n_rects.push_back(q);
				}
			}

			for (Rect q : current_area)
				AddNotIntersecting(q);
			std::swap(c_rects, n_rects);
		}

		//for (Rect l : c_rects)
		//	result.AddNotIntersecting(l);

		/*result.AddRect(r);
		return result;

		std::vector<Rect> old = std::move(rlist);
		total_area = 0;
		rlist.clear();

		for (Rect t : old)
		{
			if (RM::Intersects(t, r))
			{
				Rect i = RM::Intersection(t, r);
				auto diff = RM::Difference(t, r);
				result.AddRect(i);
				for (auto d : diff)
					AddRect(d);
			}
			else
			{
				AddRect(t);
			}
		}
		*/



		return result;
	}

	Point MinPoint(Point a, Point b)
	{
		return Point{ std::min(a.x,b.x),std::min(a.y,b.y) };
	}

	Point MaxPoint(Point a, Point b)
	{
		return Point{ std::max(a.x,b.x),std::max(a.y,b.y) };
	}

	Rect CombineRects(Rect a, Rect b)
	{
		return Rect(MinPoint(a.ul, b.ul), MaxPoint(a.dr, b.dr));
	}
	bool Intersects(Rect a, Rect b)
	{
		return (std::max(a.Left(),b.Left()) < std::min(a.Right(),b.Right())) && (std::max(a.Top(), b.Top()) < std::min(a.Bottom(), b.Bottom()));
	}
	Rect Intersection(Rect a, Rect b)
	{
		Point tl{ std::max(a.Left(),b.Left()), std::max(a.Top(),b.Top())};
		Point br{ std::min(a.Right(),b.Right()), std::min(a.Bottom(),b.Bottom()) };

		return Rect{ tl,br };
	}
	std::vector<Rect> Difference(Rect a, Rect b)
	{
		std::vector<Rect> result;
		if (a.Top() < b.Top())
		{
			result.push_back(RM::Rect{ RM::Point{a.Left(),a.Top()}, RM::Point{a.Right(),b.Top()} });
		}
		if (a.Bottom() > b.Bottom())
		{
			result.push_back(RM::Rect{ RM::Point{ a.Left(),b.Bottom() }, RM::Point{ a.Right(),a.Bottom() } });
		}
		if (a.Left() < b.Left())
		{
			result.push_back(RM::Rect{ RM::Point{ a.Left(),std::max<int>(a.Top(),b.Top()) }, RM::Point{ b.Left(),std::min<int>(a.Bottom(),b.Bottom()) } });
		}
		if (a.Right() > b.Right())
		{
			result.push_back(RM::Rect{ RM::Point{ b.Right(),std::max<int>(a.Top(),b.Top()) }, RM::Point{ a.Right(),std::min<int>(a.Bottom(),b.Bottom()) } });
		}
		return result;
	}
	void RectSet::Add(Rect r)
	{
		mRects.push_back(r);
	}
	void RectSet::Intersect(Rect r)
	{
		std::vector<Rect> mR;
		for(Rect l : mRects)
		{
			if (Intersects(l, r))
			{
				mR.push_back(Intersection(l, r));
			}
		}
		std::swap(mR, mRects);
	}

	void RectSet::Intersect(RectSet r)
	{
		std::vector<Rect> mR;

		for (Rect l : mRects)
		{
			for (Rect t : r.mRects)
			{
				if (Intersects(l, t))
				{
					mR.push_back(Intersection(l, t));
				}
			}
		}

		std::swap(mR, mRects);
	}
	void RectSet::Subtract(Rect r)
	{
		std::vector<Rect> mR;
		for (Rect l : mRects)
		{
			auto d = Difference(l, r);
			mR.insert(mR.end(), d.begin(), d.end());
		}
		std::swap(mR, mRects);
	}

	void RectSet::Subtract(RectSet r)
	{
		for (Rect l : r.mRects)
		{
			Subtract(l);
		}
	}
	std::vector<Rect>::iterator RectSet::begin()
	{
		return mRects.begin();
	}
	std::vector<Rect>::iterator RectSet::end()
	{
		return mRects.end();
	}
}