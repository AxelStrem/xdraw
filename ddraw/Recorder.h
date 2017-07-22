#pragma once
#include <queue>
#include "cyclic_buf.hpp"
#include <mutex>
#include <condition_variable>

class SurfaceProcessor;

struct RecStructHeader
{
	char opcode;
};

struct RecVect
{
	int x;
	int y;
};

struct RecRect
{
	int x;
	int y;
	int sx;
	int sy;

    RecRect(int x_=0, int y_=0, int sx_=0, int sy_=0) : x{x_}, y{y_}, sx{sx_}, sy{sy_} {}

	RecRect(RECT* pR)
	{
		if (pR)
		{
			x = pR->left;
			y = pR->top;
			sx = pR->right - x;
			sy = pR->bottom - y;
		}
		else
		{
			x = -1;
			y = -1;
			sx = -1;
			sy = -1;
		}
	}

    int getSize() const
    {
        return sx*sy;
    }

	RECT toRECT() const
	{
		RECT r;
		r.left = x;
		r.top = y;
		r.right = x + sx;
		r.bottom = y + sy;
		return r;
	}
};

#define RS_CREATE_SURFACE 1
#define RS_FILL_SURFACE 2
#define RS_BLIT 3
#define RS_BLIT_MIRRORED 4
#define RS_BLIT_KEYED 5
#define RS_MODIFY_SUB 6
#define RS_MODIFY 7
#define RS_TIMESTAMP 8

struct RecStructCreateSurface : public RecStructHeader
{
	int     handle;
	int     flags;
	RecVect size;

	RecStructCreateSurface(int handle_, int flags_, RecVect size_)
		: RecStructHeader{ RS_CREATE_SURFACE }, handle(handle_), flags(flags_), size(size_) {}
};


struct RecStructFillSurface : public RecStructHeader
{
	int     handle;
	int     flags;
	RecRect rect;
	int     color;

	RecStructFillSurface(int handle_, int flags_, RecRect rect_, int color_)
		: RecStructHeader{ RS_FILL_SURFACE }, handle(handle_), flags(flags_), rect(rect_), color(color_) {}
};

struct RecStructBlit : public RecStructHeader
{
	int     d_handle;
	int     s_handle;
	int     flags;

	RecRect d_rect;
	RecRect s_rect;

	RecStructBlit(int d_handle_, int s_handle_, int flags_, RecRect d_rect_, RecRect s_rect_)
		: RecStructHeader{ RS_BLIT }, d_handle(d_handle_), s_handle(s_handle_),
		flags(flags_), d_rect(d_rect_), s_rect(s_rect_) {}
};

struct RecStructMirrorBlit : public RecStructHeader
{
	int     d_handle;
	int     s_handle;
	int     flags;

	RecRect d_rect;
	RecRect s_rect;

	int color;

	RecStructMirrorBlit(int d_handle_, int s_handle_, int flags_, RecRect d_rect_, RecRect s_rect_, int color_)
		: RecStructHeader{ RS_BLIT_MIRRORED }, d_handle(d_handle_), s_handle(s_handle_),
		flags(flags_), d_rect(d_rect_), s_rect(s_rect_), color(color_) {}
};


struct RecStructKeyedBlit : public RecStructHeader
{
	int     d_handle;
	int     s_handle;
	int     flags;

	RecRect d_rect;
	RecRect s_rect;

	int color;

	RecStructKeyedBlit(int d_handle_, int s_handle_, int flags_, RecRect d_rect_, RecRect s_rect_, int color_)
		: RecStructHeader{ RS_BLIT_KEYED }, d_handle(d_handle_), s_handle(s_handle_),
		flags(flags_), d_rect(d_rect_), s_rect(s_rect_), color(color_) {}
};

struct RecStructModifySubSurface : public RecStructHeader
{
	int     handle;
    long long data_rec;
	RecRect rect;
	
	RecStructModifySubSurface(int handle_, RecRect rect_) :
		RecStructHeader{ RS_MODIFY_SUB }, handle(handle_),rect(rect_) {}
};


struct RecStructModifySurface : public RecStructHeader
{
	int     handle;
    long long data_rec;

	RecStructModifySurface(int handle_) :
		RecStructHeader{ RS_MODIFY }, handle(handle_){}
};

struct RecStructFrameTimestamp : public RecStructHeader
{
	int     timestamp;

	RecStructFrameTimestamp(int ts_) :
		RecStructHeader{ RS_TIMESTAMP }, timestamp(ts_){}
};


void record_stream(WORD* pData, int xs, int ys, int pitch);

class Encoder;
class Decoder;

#define CBSIZE 33554432

template<class T> class CBIter
{
public:
    BYTE* pos;
    BYTE* end;
    BYTE* alt;
public:
    bool operator!=(const CBIter&)
    {
        if (pos != end)
            return true;
        if (end > alt)
        {
            std::swap(end, alt);
            pos -= CBSIZE;
            return true;
        }
        return false;
    }
    CBIter& operator++()
    {
        pos += sizeof(T);
        return *this;
    }
    T* operator->()
    {
        return reinterpret_cast<T*>(pos);
    }
    T& operator*()
    {
        return *(reinterpret_cast<T*>(pos));
    }
    int getSize() const
    {
        int r = end - pos;
        if (end > alt)
            r += alt - (pos - CBSIZE);
        return r / sizeof(T);
    }
    int operator-(const CBIter& b)
    {
        return b.getSize();
    }
};

class CircularBuffer
{
	std::vector<BYTE> data;
	std::atomic<int>  bstart = 0;
	std::atomic<int>  bend = 0;

    std::mutex              buf_mutex;
    std::condition_variable cvar;

    void Notify();
public:
	CircularBuffer();

	int Space();
    int Size();

    void WaitForData();
    
    void Clear();
    void Pop(int size);

	void Write(const char* d, int sz);
    void ForceInt(int val);
    void Reserve(int sz);
	bool ReadFromStream(std::istream& is, int sz);
   
    CBIter<int> begin();
    CBIter<int> end() const { return{}; }

    template<class T> T* naked_begin()
    {
        return reinterpret_cast<T*>(data.data() + (bstart/sizeof(T))*sizeof(T));
    }

    template<class T> T* naked_end()
    {
        return reinterpret_cast<T*>(data.data() + (bend / sizeof(T)) * sizeof(T));
    }

    template<class T> T* naked_begin_out()
    {
        return reinterpret_cast<T*>(data.data() + (bend / sizeof(T)) * sizeof(T));
    }

    template<class T> T* next(T* pc)
    {
        pc ++;
        if (reinterpret_cast<BYTE*>(pc) >= (data.data() + data.size()))
            pc -= data.size()/sizeof(T);
        return pc;
    }

    template<class T> void push_naked(T* pc)
    {
        bend = reinterpret_cast<BYTE*>(pc) - data.data();
        Notify();
    }

    template<class T> void pop_naked(T* pc)
    {
        bstart = reinterpret_cast<BYTE*>(pc) - data.data();
    }
 
	void Save();
	void Encode(Encoder& dst);
	bool ReadWord(WORD& w);
    bool ReadInt(int& w);
    bool Read(char* d, int sz);
};

struct IntChunk
{
    int value;
    int counter;
};

class Encoder
{
	CircularBuffer& mTarget;

	WORD last_word;
	WORD counter;
	bool bCleanState = true;
	bool bRepeated = false;
	std::vector<WORD> mdat;

    int last_int = 0;
    int int_counter = 0;

    std::queue < IntChunk, cyclic_buffer<IntChunk> > compress_queue;
    std::queue<WORD> decode_queue;
public:
	Encoder(CircularBuffer& cb) : mTarget(cb) {}
	void PushWord(WORD w);
	void Finalize();

    void DecodeSequence();
	void Decode(WORD* pDst, int size);

    void PushDecodedDword(int i);
    void PushInt(int i, int c);

    void PopIntBatch();
    void PopSequence();

    template<class Iter> void EncodeRange(Iter a, Iter b)
    {
       // mTarget.Reserve(sizeof(int)*(b - a));
        for (; a != b; ++a)
        {
          //  mTarget.Write((const char*)(&(*a)), sizeof(*a));
            mTarget.ForceInt(*a);
        }
        return;

        for (; a != b; ++a)
        {
            const int& i = *a;
            if (i == last_int)
            {
                int_counter++;
                if (int_counter > 0x7FFFFFFF)
                {
                    PushInt(last_int, 0x7FFFFFFF);
                    int_counter = 1;
                }
            }
            else
            {
                if(int_counter)
                    PushInt(last_int, int_counter);
                last_int = i;
                int_counter = 1;
            }
        }
    }
};

extern CircularBuffer global_recording_buffer;
extern CircularBuffer global_encoding_buffer;
extern CircularBuffer global_processing_buffer;

extern SurfaceProcessor gSP;