#pragma once

#include <vector>
#include <new>

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

    template <class ... Args> bool emplace_at(int pos, Args&& ... args)
    {
        if (static_cast<int>(data.size()) <= pos)
        {
            int s = pos - data.size();
            int i = data.size();
            data.insert(data.end(), s, T{});
            free_pos.reserve(free_pos.size() + s);
            for (; i < s; i++)
                free_pos.push_back(i);

            data.emplace_back(args...);
            return true;
        }
        auto it = std::find(free_pos.begin(), free_pos.end(), pos);
        if (it == free_pos.end())
            return false;

        *it = free_pos.back();
        free_pos.pop_back();

        T* pObj = &data[pos];
        pObj->~T();
        new (pObj) T{ args... };
        return true;
    }

	bool has(int pos)
	{
		if (static_cast<int>(data.size()) <= pos)
			return false;
		return std::find(free_pos.begin(), 
			free_pos.end(), pos) == free_pos.end();
	}

	int size() const { return data.size(); }
};