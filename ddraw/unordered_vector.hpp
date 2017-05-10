#pragma once

#include <vector>

template <class T> class unordered_vector
{
	std::vector<T> data;
	std::vector<int> free_pos;
public:
	T& operator[](int i)   { return data[i]; }
	template <class T2> int insert(T2&& v) 
	{
		if (free_pos.empty())
		{
			int r = data.size();
			data.push_back(std::forward<T2&&>(v));
			return r;
		}
		int p = free_pos.back();
		free_pos.pop_back();

		data[p] = std::forward<T2&&>(v);

		return p;
	}

	template <class ... Args> int emplace(Args&& ... args)
	{
		if (free_pos.empty())
		{
			int r = data.size()
				data.emplace_back(std::forward<Args&& ...>(args...));
			return r;
		}
		int p = free_pos.back();
		free_pos.pop_back();

		T* pT = &data[p];
		pT->(~T());
		new (pT) T(std::forward<Args&& ...>(args...));
		return p;
	}

	void erase(int pos)
	{
		free_pos.push_back(pos);
	}

	bool has(int pos)
	{
		if (data.size() <= pos)
			return false;
		return std::find(free_pos.begin(), 
			free_pos.end(), pos) == free_pos.end();
	}

	int size() const { return data.size(); }
};