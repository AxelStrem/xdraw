#include "stdafx.h"
#include "Recorder.h"
#include <atomic>
#include <thread>
#include <utility>
#include <algorithm>

std::ofstream global_out_file;
std::ofstream global_out_file_stream;

bool          global_is_recording = false;
bool          global_is_recording_ps = false;

void CircularBuffer::Notify()
{
   // cvar.notify_all();
}

CircularBuffer::CircularBuffer() : data(CBSIZE) {}

	int CircularBuffer::Space()
	{
		int be = bend;
		int bs = bstart;
		if (be < bs)
			be += data.size();
		return data.size() - (be - bs);
	}

    int CircularBuffer::Size()
    {
        int be = bend;
        int bs = bstart;
        if (be < bs)
            be += data.size();
        return (be - bs);
    }

    void CircularBuffer::WaitForData()
    {
        std::unique_lock<std::mutex> ul(buf_mutex);
        while(!Size())
            cvar.wait(ul);
    }

    void CircularBuffer::Clear()
    {
        bstart.store(bend);
    }

    void CircularBuffer::Pop(int size)
    {
        int bs = bstart + size;
        bs %= data.size();
        bstart.store(bs);
    }

    void CircularBuffer::Reserve(int sz)
    {
        while (Space() < sz)
            SwitchToThread();
    }

    void CircularBuffer::ForceInt(int val)
    {
        *((int*)(data.data() + bend.load())) = val;
        int nend = (bend + sizeof(int));
        if (nend >= static_cast<int>(data.size())) nend -= data.size();
        bend.store(nend);
    }

	void CircularBuffer::Write(const char* d, int sz)
	{
        Reserve(sz);

        int nend = bend.load();

		int s1 = std::min<int>(sz,data.size() - nend);
		int s2 = sz - s1;
		if (s1)
		{
			memcpy(data.data() + nend, d, s1);
		}
		if (s2)
		{
			memcpy(data.data(), d + s1, s2);
			nend = s2;
		}
		else
			nend += s1;

        bend.store(nend);
        Notify();
	}

	bool CircularBuffer::ReadFromStream(std::istream & is, int sz)
	{
		if (Space() < sz)
		{
			return false;
		}

        int nend = bend.load();
		
		int s1 = std::min<int>(sz, data.size() - nend);
		int s2 = sz - s1;
		if (s1)
		{
			is.read((char*)(data.data() + nend), s1);
		}
		if (s2)
		{
			is.read((char*)(data.data()), s2);
			nend = s2;
		}
		else
			nend += s1;

        bend.store(nend);
        Notify();

        return true;
	}

    CBIter<int> CircularBuffer::begin()
    {
        CBIter<int> i;
        int be = bend.load();
        int bs = bstart.load();
        
        be = (be / sizeof(int)) * sizeof(int);
        bs = (bs / sizeof(int)) * sizeof(int);

        i.pos = data.data() + bs;
        i.end = data.data() + be;
        i.alt = data.data() + data.size();
        if (be < bs)
            std::swap(i.end, i.alt);
        return i;
    }

	void CircularBuffer::Encode(Encoder& dst)
	{
        auto b = begin();
        auto e = end();
        int tb = bstart;
        int te = bend;

        te = (te / sizeof(int)) * sizeof(int);

       if(tb <= te)
            dst.EncodeRange(reinterpret_cast<int*>(data.data() + tb), reinterpret_cast<int*>(data.data() + te));
        else
        {
            dst.EncodeRange(reinterpret_cast<int*>(data.data() + tb), reinterpret_cast<int*>(data.data() + data.size()));
            dst.EncodeRange(reinterpret_cast<int*>(data.data()), reinterpret_cast<int*>(data.data() + te));
        }

        Pop(te - tb);
		/*int be = bend;
		int bs = bstart;
		if (be < bs)
			be += data.size();
		int sz = be - bs;

		int s1 = std::min<int>(sz, data.size() - bs);
		int s2 = sz - s1;

		for (int i = 0; i < s1; i+=2)
		{
			dst.PushWord(*((WORD*)(data.data() + bs + i)));
		}

		for (int i = 0; i < s2; i+=2)
		{
			dst.PushWord(*((WORD*)(data.data() + i)));
		}

		dst.Finalize();

		bstart = be;*/
	}

	bool CircularBuffer::ReadWord(WORD& w)
	{
		if ((bstart == bend)||((bstart+1) == bend)) return false;
		w = *((WORD*)(data.data() + bstart));
		bstart = (bstart + 2) % data.size();
		return true;
	}

    bool CircularBuffer::ReadInt(int& i)
    {
        if (Size() < sizeof(int))
            return false;
        i = *((int*)(data.data() + bstart));
        bstart = (bstart + sizeof(int)) % data.size();
        return true;
    }

    bool CircularBuffer::Read(char* s, int sz)
    {
        if (Size() < sz) return false;

        int bs = bstart;

        int s1 = std::min<int>(bs + sz, data.size()) - bs;
        int s2 = sz - s1;

        if (s1 > 0)
            memcpy(s, data.data() + bs, s1);
       
        if (s2 > 0)
        {
            memcpy(s + s1, data.data(), s2);
            bstart = s2;
        }
        else
            bstart += s1;

        return true;
    }

    


    void CircularBuffer::Save()
	{
		int be = bend;
		int bs = bstart;
		if (be < bs)
			be += data.size();
		int sz = be - bs;

		int s1 = std::min<int>(sz, data.size() - bs);
		int s2 = sz - s1;
		if (s1)
		{
			global_out_file_stream.write((char*)(data.data()) + bs, s1);
		}
		if (s2)
		{
			global_out_file_stream.write((char*)(data.data()), s2);
			bstart = s2;
		}
		else
			bstart += s1;
	}

CircularBuffer global_recording_buffer;
CircularBuffer global_encoding_buffer;
CircularBuffer global_processing_buffer;

/*class Compressor
{
	WORD lw;
	int tp = 0;
	bool ts = false;
public:
	Compressor(int sz) { cdata.reserve(sz); cdata.clear(); }
	void InitWord(WORD w)
	{
		lw = w;
		tp = 0;
	}
	void PushWord(WORD w)
	{
		if (w == lw)
		{
			tp++;
		}
		else
		{
			cdata.push_back(tp);
			cdata.push_back(lw);
			tp = 1;
			lw = w;
		}
	}
	void Save()
	{
		global_out_file.write((char*)cdata.data(), cdata.size()*2);
	}
};*/

void record_stream(WORD * pData, int xs, int ys, int pitch)
{
	//Compressor comp(xs*ys*2);
	//cdata.resize(xs*ys);

	if ((xs <= 0) || (ys <= 0))
		return;
	//comp.InitWord(*pData);
	for (int i = 0; i < ys; i++)
	{
		global_processing_buffer.Write((char*)pData, xs * 2);
		//for (int j = 0; j < xs; j++)
		//	comp.PushWord(pData[j]);
		pData += pitch;
	}
	//comp.Save();
}

void Encoder::PushWord(WORD w)
{
	if (bCleanState)
	{
		last_word = w;
		counter = 1;
		bCleanState = false;
		bRepeated = false;
		return;
	}

	if ((w == last_word)&&(counter<0x7FFF))
	{
		if (!bRepeated)
		{
			if (counter == 1)
			{
				bRepeated = true;
				counter++;
			}
			else
			{
				mdat[0] = counter | 0x8000;
				mTarget.Write((char*)(mdat.data()), sizeof(WORD) * mdat.size());
				counter = 1;
			}
		}
		else
			counter++;
	}
	else
	{
		if (bRepeated)
		{
			WORD dat[2] = { counter, last_word };
			mTarget.Write((char*)(dat), sizeof(WORD) * 2);
			counter = 1;
			last_word = w;
			bRepeated = false;
		}
		else
		{
			if (counter == 1)
			{
				mdat.clear();
				mdat.push_back(0);
				mdat.push_back(last_word);
				mdat.push_back(w);
				counter++;
			}
			else
			{
				if (counter < 0x7FFF)
				{
					counter++;
					mdat.push_back(w);
				}
				else
				{
					mdat[0] = counter | 0x8000;
					mTarget.Write((char*)(mdat.data()), sizeof(WORD) * mdat.size());
					counter = 1;
				}
			}
		}
		last_word = w;
	}	
}

void Encoder::Finalize()
{
	if (bCleanState)
		return;

	if (counter == 1)
	{
		WORD dat[2] = { counter, last_word };
		mTarget.Write((char*)(dat), sizeof(WORD) * 2);
		counter = 0;
		bRepeated = false;
		bCleanState = true;
		return;
	}

	if (bRepeated)
	{
		WORD dat[2] = { counter, last_word };
		mTarget.Write((char*)(dat), sizeof(WORD) * 2);
		counter = 0;
		bRepeated = false;
		bCleanState = true;
	}
	else
	{
		mdat[0] = counter | 0x8000;
		mTarget.Write((char*)(mdat.data()), sizeof(WORD) * mdat.size());
		counter = 0;
		bRepeated = false;
		bCleanState = true;
	}
}

void Encoder::DecodeSequence()
{
    int c;
    while (!mTarget.ReadInt(c)) {}
    if (c & 0x80000000)
    {
        int i;
        while (!mTarget.ReadInt(i)) {}
        c &= 0x7FFFFFFF;
        for(int t=0;t<c;t++)
            PushDecodedDword(i);
    }
    else
    {
        int i;
        for (int t = 0; t < c; t++)
        {
            while (!mTarget.ReadInt(i)) {}
            PushDecodedDword(i);
        }
    }
}

void Encoder::Decode(WORD * pDst, int size)
{
    while (static_cast<int>(decode_queue.size()) < size)
        DecodeSequence();

    for (int i = 0; i < size; ++i)
    {
        pDst[i] = decode_queue.front();
        decode_queue.pop();
    }
}

void Encoder::PushDecodedDword(int i)
{
    decode_queue.push(LOWORD(i));
    decode_queue.push(HIWORD(i));
}

void Encoder::PushInt(int i, int c)
{
    if (compress_queue.size())
    {
        if (compress_queue.back().value == i)
        {
            int nc = compress_queue.back().counter = c;
            if (nc <= 0x7FFFFFFF)
            {
                compress_queue.back().counter = nc;
                return;
            }
        }
        compress_queue.push({ i,c });
        if (compress_queue.front().counter > 1)
            PopIntBatch();
        else if (compress_queue.back().counter > 1)
            PopSequence();
    }
    else
    compress_queue.push({ i,c });
}

void Encoder::PopIntBatch()
{
    int c = compress_queue.front().counter | 0x80000000;
    mTarget.Write(reinterpret_cast<char*>(&c), sizeof(c));
    c = compress_queue.front().value;
    mTarget.Write(reinterpret_cast<char*>(&c), sizeof(c));
    compress_queue.pop();
}

void Encoder::PopSequence()
{
    std::vector<int> ldata;
    while ((compress_queue.size() > 0) && (compress_queue.front().counter == 1))
    {
        ldata.push_back(compress_queue.front().value);
        compress_queue.pop();
    }

    if (ldata.empty())
        return;

    int c = ldata.size();
     mTarget.Write(reinterpret_cast<char*>(&c), sizeof(c));
    mTarget.Write(reinterpret_cast<char*>(ldata.data()), sizeof(int)*ldata.size());
}
