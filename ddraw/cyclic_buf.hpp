#pragma once
#include <vector>
#include <algorithm>

template<class T, class C = std::vector<T>> class cyclic_buffer
{
    C data;
    typename C::iterator ibeg;
    typename C::iterator iend;
public:
    typedef T value_type;
    typedef typename C::size_type size_type;
    typedef typename C::reference reference;
    typedef typename C::const_reference const_reference;

    cyclic_buffer() : data{1}, ibeg(begin(data)), iend(begin(data))
    {
    }
    cyclic_buffer(size_t sz) : data{ sz }, ibeg(begin(data)), iend(begin(data)) {}

    T& front()
    {
        return *ibeg;
    }

    T& back()
    {
        if (iend == begin(data))
            return data.back();
        return *(iend-1);
    }

    void pop_front()
    {
        ibeg++;
        if (ibeg == end(data))
            ibeg = begin(data);
    }

    void force_back(const T& val)
    {
        *(iend++) = val;
        if (iend == end(data))
            iend = begin(data);
    }

    void push_back(const T& val)
    {
        force_back(val);
        if (iend == ibeg)
        {
            int s = data.size();
            
            std::rotate(begin(data), ibeg, end(data));
            data.push_back(T{});

            ibeg = begin(data);
            iend = ibeg + s;
        }
    }

    size_t size() const
    {
        if (iend < ibeg)
            return data.size() - (ibeg - iend);
        return iend - ibeg;
    }

    bool empty() const
    {
        return iend == ibeg;
    }


};