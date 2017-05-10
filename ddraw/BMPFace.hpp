#pragma once
#include <windows.h>

#include "stdafx.h"
#include <iterator>

#include <vector> 
#include <string>
#include <memory>

#include <fstream>
#include <array>

#include <windows.h>
#include <conio.h>

#include <set>

COLORREF inline BGR(COLORREF c)
{
	return RGB(GetBValue(c), GetGValue(c), GetRValue(c));
}

struct COLOR_3B
{
	BYTE b;
	BYTE g;
	BYTE r;
};

struct COLOR_4B : public COLOR_3B
{
	BYTE a;
};

bool inline in_palette(COLOR_4B col)
{
	return (col.a | col.b | col.g | col.r) != 0;
}

template<class Pixel, class Color = COLOR_3B> class Bitmap
{
	int size_x, size_y;
	std::vector<Pixel> data;
	std::vector<COLORREF> palette;

	BITMAPFILEHEADER hdr;
	BITMAPINFOHEADER ihdr;
	BITMAPCOREHEADER chdr;

public:
	Bitmap()
	{
		if (sizeof(Pixel) == 1) palette.resize(256);
	}

	Bitmap(const Bitmap& b) : data(b.data), size_x(b.size_x), size_y(b.size_y), palette(b.palette)
	{}

	int GetWidth() { return size_x; }
	int GetHeight() { return size_y; }

	void SetSize(int width, int height)
	{
		size_x = width;
		size_y = height;
		data.resize(size_x*size_y);
	}

	Pixel& At(int x, int y)
	{
		return data[size_x*y + x];
	}

	Pixel& At(std::pair<int, int> coord)
	{
		return At(coord.first, coord.second);
	}

	bool Check(int x, int y)
	{
		return ((x < size_x) && (x >= 0) && (y < size_y) && (y >= 0));
	}

	bool Check(std::pair<int, int> coord)
	{
		return Check(coord.first, coord.second);
	}

	COLORREF &Palette(Pixel value)
	{
		return palette[value];
	}

	void Clear(Pixel c)
	{
		for (auto& p : data)
			p = c;
	}

	template<class F> void Fill(int x, int y, F& func)
	{
		std::vector<std::pair<int, int>> border;

		std::vector<char> visited(data.size(),0);

		Pixel col = At(x, y);

		border.push_back(std::make_pair(x, y));
		while (!border.empty())
		{
			std::vector<std::pair<int, int>> new_border;
			for (auto p : border)
			{
				auto apt = Neighbours(p.first, p.second);
				for (auto& pp : apt)
				{
					int index = size_x*pp.y + pp.x;
					if ((pp.value == col) && (!visited[index]))
					{
						visited[index] = 1;
						new_border.push_back(std::make_pair(pp.x, pp.y));
						BitmapPoint bp{ pp.x,pp.y,data[index] };
						func(bp);
					}
				}
			}
			border = new_border;
		}
	}

	bool Load(std::string filename)
	{
		std::ifstream src;
		src.open(filename.c_str(), std::ios_base::binary);
		src.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
		src.read(reinterpret_cast<char*>(&ihdr), sizeof(ihdr));

		if (ihdr.biBitCount != (8 * sizeof(Pixel))) 
			return false;

		int offset = sizeof(hdr) + sizeof(ihdr);

		if (sizeof(Pixel) == 1)
		{
			palette.resize(256);
			for (COLORREF& c : palette)
			{
				src.read(reinterpret_cast<char*>(&c), sizeof(COLORREF));
				c = BGR(c);
			}
			offset += 256 * sizeof(COLORREF);
		}

		unsigned char ch;

		int height = ihdr.biHeight;
		int width = ihdr.biWidth;
		int offbits = hdr.bfOffBits;

		int stride = ((width*sizeof(Pixel) + 3) / 4) * 4;

		while (offset < hdr.bfOffBits)
		{
			offset++;
			src.read(reinterpret_cast<char*>(&ch), 1);
		}

		SetSize(width, height);
		Pixel p;

		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				src.read(reinterpret_cast<char*>(&p), sizeof(Pixel));
				At(j, height-1-i) = p;
			}
			offset = width*sizeof(Pixel);
			while (offset < stride)
			{
				offset++;
				src.read(reinterpret_cast<char*>(&ch), 1);
			}
		}

		src.close();
		return true;
	}

	void Save(std::string filename)
	{
		ZeroMemory(&hdr, sizeof(hdr));
		ZeroMemory(&ihdr, sizeof(ihdr));
		
		ihdr.biBitCount = 8 * sizeof(Pixel);

		ihdr.biHeight = size_y;
		ihdr.biWidth = size_x;
		hdr.bfOffBits = sizeof(hdr) + sizeof(ihdr) + (sizeof(Pixel)==1)?(256*sizeof(COLORREF)):0;

		int height = ihdr.biHeight;
		int width = ihdr.biWidth;
		int offbits = hdr.bfOffBits;

		int stride = ((width * sizeof(Pixel) + 3) / 4) * 4;
		int datasize = stride*height;

		hdr.bfType = ('M' << 8) | 'B';
		hdr.bfSize = hdr.bfOffBits+datasize;

		ihdr.biSize = sizeof(ihdr);
		ihdr.biPlanes = 1;
		ihdr.biCompression = BI_RGB;
		
		std::ofstream dst;
		dst.open(filename.c_str(), std::ios_base::binary);
		dst.write(reinterpret_cast<char*>(&hdr), sizeof(hdr));
		dst.write(reinterpret_cast<char*>(&ihdr), sizeof(ihdr));
				
		int offset = sizeof(hdr) + sizeof(ihdr);

		if (sizeof(Pixel) == 1)
		{
			palette.resize(256);
			for (COLORREF& c : palette)
			{
				COLORREF cc = BGR(c);
				dst.write(reinterpret_cast<char*>(&cc), sizeof(COLORREF));
			}
			offset += 256 * sizeof(COLORREF);
		}

		unsigned char ch = 0;

		while (offset < hdr.bfOffBits)
		{
			offset++;
			dst.write(reinterpret_cast<char*>(&ch), 1);
		}

		Pixel p;

		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				p = At(j, height - 1 - i);
				dst.write(reinterpret_cast<char*>(&p), sizeof(Pixel));
			}
			offset = width * sizeof(Pixel);
			while (offset < stride)
			{
				offset++;
				dst.write(reinterpret_cast<char*>(&ch), 1);
			}
		}

		dst.close();
	}

	struct BitmapPoint
	{
		int x;
		int y;
		Pixel &value;
	};

	class Iterator
	{
		Bitmap& host;
		int x;
		int y;
	public:
		Iterator(Bitmap& h) : host(h) {}
		Iterator(Bitmap& h, int x_, int y_) : host(h), x(x_), y(y_) {}
		Iterator operator ++()
		{
			x++;
			if (x >= host.GetWidth())
			{
				x = 0;
				y++;
			}
			return *this;
		}

		bool operator==(const Iterator& b) const
		{
			return (x == b.x) && (y == b.y);
	    }

		bool operator!=(const Iterator& b) const
		{
			return !(*this==b);
		}

		BitmapPoint operator*()
		{
			return BitmapPoint{ x,y,host.At(x,y) };
		}
	};

	Iterator begin()
	{
		return Iterator(*this,0,0);
	}

	Iterator end()
	{
		return Iterator(*this, 0, size_y);
	}

	std::vector<BitmapPoint> Aperture(int x, int y)
	{
		std::vector<BitmapPoint> result;
		for(int i=-1;i<=1;i++)
			for (int j = -1; j <= 1; j++)
			{
				if ((i == 0) && (j==0)) continue;
				if (!Check(x + i, y + j)) continue;
				result.push_back(BitmapPoint{x+i,y+j,At(x+i,y+j)});
			}

		return result;
	}

	std::vector<BitmapPoint> Neighbours(int x, int y)
	{
		std::vector<BitmapPoint> result;

		if (Check(x - 1, y)) result.push_back(BitmapPoint{ x -1,y ,At(x - 1,y) });
		if (Check(x + 1, y)) result.push_back(BitmapPoint{ x + 1,y ,At(x + 1,y) });
		if (Check(x, y-1)) result.push_back(BitmapPoint{ x,y-1 ,At(x,y-1) });
		if (Check(x, y + 1)) result.push_back(BitmapPoint{ x,y + 1 ,At(x,y + 1) });
		return result;
	}
};

/*
#include <algorithm>
#include <iostream>
#include <numeric>
#include <functional>
#include <array>
#include <map>
#include <string>
#include <Windows.h>
#include <string>
#include <deque>
#include <iterator>
#include <iostream>
#include <memory>
#include <array>*/
/*
#include <opencv/cv.h>
#include <opencv2/core.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/stitching.hpp>*/
//#include <experimental/dynarray>
/*
#include "local_ptr.hpp"
#include <atomic>

class Foo {
public:
Foo() {}
Foo(int j) : i(j) {}
virtual ~Foo() {}
virtual int at(int index) const { return i[index]; }
virtual int size() const { return i.size(); }
Foo& operator=(const Foo& b)
{
i.resize(b.size());
for (int p = 0; p < i.size(); p++)
i[p] = b.at(p);
return *this;
}
private:
std::vector<int> i;
};

class Bar : public Foo {
public:
Bar(int j) : i(j) {}
~Bar() {}
virtual int at(int index) const { return i[index]; }
virtual int size() const { return i.size(); }
private:
std::vector<char> i;
};

class Sequence
{
std::atomic<int> val;
public:
Sequence() : val(2) {}
int next()
{
return val.fetch_add(3,std::memory_order_relaxed);
}
};


std::mutex gm;
std::vector<int> gvals;

void run(Sequence* s)
{
std::vector<int> vals;
for (int i = 0; i < 100; i++)
{
vals.push_back(s->next());
}
gm.lock();
std::copy(vals.begin(), vals.end(), std::back_inserter(gvals));
gm.unlock();
}

using namespace lptr;

class TestIntAllocator
{
static int value;
public:
static char* allocate(size_t size)
{
return (char*)(&value);
}
static void free(char* ptr)
{

}
};

int TestIntAllocator::value = 0;

void g()
{
int i = 0;
throw 5;
i += 10;
}

void f()
{
g();
}

class Frame
{
static const int frame_width = 79;
std::array<int, frame_width> bg_color;
std::array<int, frame_width> fg_color;
std::array<char, frame_width> text;

void SetBGColor(int color)
{
switch (color)
{
case 0: std::cout << rang::bg::black; return;
case 1: std::cout << rang::bg::blue; return;
case 2: std::cout << rang::bg::cyan; return;
case 3: std::cout << rang::bg::gray; return;
case 4: std::cout << rang::bg::green; return;
case 5: std::cout << rang::bg::magenta; return;
case 6: std::cout << rang::bg::red; return;
case 7: std::cout << rang::bg::yellow; return;
case 8: std::cout << rang::bgB::black; return;
case 9: std::cout << rang::bgB::blue; return;
case 10: std::cout << rang::bgB::cyan; return;
case 11: std::cout << rang::bgB::gray; return;
case 12: std::cout << rang::bgB::green; return;
case 13: std::cout << rang::bgB::magenta; return;
case 14: std::cout << rang::bgB::red; return;
case 15: std::cout << rang::bgB::yellow; return;
}
}
void SetFGColor(int color)
{
switch (color)
{
case 0: std::cout << rang::fg::black; return;
case 1: std::cout << rang::fg::blue; return;
case 2: std::cout << rang::fg::cyan; return;
case 3: std::cout << rang::fg::gray; return;
case 4: std::cout << rang::fg::green; return;
case 5: std::cout << rang::fg::magenta; return;
case 6: std::cout << rang::fg::red; return;
case 7: std::cout << rang::fg::yellow; return;
case 8: std::cout << rang::fgB::black; return;
case 9: std::cout << rang::fgB::blue; return;
case 10: std::cout << rang::fgB::cyan; return;
case 11: std::cout << rang::fgB::gray; return;
case 12: std::cout << rang::fgB::green; return;
case 13: std::cout << rang::fgB::magenta; return;
case 14: std::cout << rang::fgB::red; return;
case 15: std::cout << rang::fgB::yellow; return;
}
}
public:
Frame()
{
ResetFrame();
}

void ResetFrame()
{
for (int i = 0; i < frame_width; i++)
{
SetCharacter(i);
}
}

void Draw()
{
std::cout << '\r';

SetBGColor(bg_color[0]);
SetFGColor(fg_color[0]);

std::cout << text[0];

for (int i = 1; i < frame_width; i++)
{
if (bg_color[i] != bg_color[i - 1])
SetBGColor(bg_color[i]);
if (fg_color[i] != fg_color[i - 1])
SetFGColor(fg_color[i]);
std::cout << text[i];
}

std::cout.flush();
}

void SetCharacter(int pos, int bg_col = 0, int fg_col = 3, char c = ' ')
{
bg_color[pos] = bg_col;
fg_color[pos] = fg_col;
text[pos] = c;
}

int SetCharacterBC(int pos, int bg_col = 0, int fg_col = 3, char c = ' ')
{
while (pos < 0) pos += frame_width;
pos %= frame_width;
SetCharacter(pos, bg_col, fg_col, c);
return pos;
}

int GetFrameWidth()
{
return frame_width;
}
};

void RandomShower(Frame& f)
{
for (int i = 0; i<f.GetFrameWidth(); i++)
{
f.SetCharacter(i, rand() % 16);
}
}

void TS(Frame& f, int pos, int col)
{
f.SetCharacterBC(pos, col);
f.SetCharacterBC(pos-1, col-8);
f.SetCharacterBC(pos+1, col-8);
}

void TreeStructure(Frame& f)
{
static std::vector<int> pos({40});
static std::vector<int> col({10});
static std::vector<int> pos_target({ 40 });
static std::vector<int> pos_delay({ 10 });
f.ResetFrame();
for (int i = 0; i < pos.size(); i++)
{
if (pos[i] != pos_target[i])
{
if (pos[i] < pos_target[i])
pos[i]++;
else
pos[i]--;

for (int j = 0; j < pos.size(); j++)
{
if ((j != i) && (pos[j] == pos[i]))
{
if (!(rand() % 4))
{
pos.erase(pos.begin() + j);
col.erase(col.begin() + j);
pos_target.erase(pos_target.begin() + j);
pos_delay.erase(pos_delay.begin() + j);

if (i > j) i--;
}
}
}

TS(f, pos[i], col[i]);
}
else
{
if (pos_delay[i] > 0)
{
pos_delay[i]--;
}
else
{
pos_target[i] = rand() % 78;
pos_delay[i] = 10 + rand() % 32;
pos.push_back(pos[i]);
pos_target.push_back(rand() % 78);
col.push_back(9 + rand() % 7);
pos_delay.push_back(10+rand() % 32);
}
TS(f, pos[i], col[i]);
}
}
}

void Starfall(Frame& f)
{
static std::array<int, 78> stars;
static std::array<int, 78> colors;


int ns = rand() % 256;

if (ns < 78)
{
stars[ns] = 3 + rand() % 8;
colors[ns] = rand() % 7 + 1;
}

for (int i = 0; i < 78; i++)
{
if (stars[i])
{
f.SetCharacterBC(i, 0, colors[i], '|');
stars[i]--;
}
else
f.SetCharacterBC(i);
}

if (ns < 78)
{
f.SetCharacter(ns, 0,colors[ns]+8,'*');
}
}

void RANG(Frame& f)
{
static std::ifstream text("rang.txt");
char str[45];
text.read(str, 41);

if (text.eof())
{
text.close();
text.clear();
text.open("rang.txt");
text.read(str, 41);
}

for (int i = 0; i < 20; i++)
f.SetCharacter(i, 0);
for (int i = 0; i < 40; i++)
{
int color = 0;
switch (str[i])
{
//case '@': color = 0; break;
case ' ': color = 0; break;
//case '#':
//case ',': color = 3; break;
default: color = rand() % 16; break;
}
f.SetCharacter(20 + i, color);
}
for (int i = 60; i < 78; i++)
f.SetCharacter(i, 0);
}

void Toothpaste(Frame& f)
{
static std::vector<int> colors({ 11 });
static std::vector<int> intervals({ 78 });

for (int i = 0; i < colors.size(); i++)
{
if (intervals[i] >= 10)
{
if (!(rand() % 4))
{
int os = intervals[i];
int col = colors[i];
int p = rand() % (os-6) + 3;
intervals[i] = p;
intervals.insert(intervals.begin() + i, 1);
intervals.insert(intervals.begin() + i, os - 1 - p);
colors.insert(colors.begin() + i, rand() % 15 + 1);
colors.insert(colors.begin() + i, col);
}
}
}

int inter = rand() % intervals.size();
int v = intervals[inter];
if (rand() % 2)
{
intervals[inter]++;
}
else
{
if ((intervals[inter] > 5) || !(rand() % 4))
{
intervals[inter]--;
if (intervals[inter] == 0)
{
intervals.erase(intervals.begin() + inter);
colors.erase(colors.begin() + inter);
}
}
}

int c = 0;
for (int i = 0; i < colors.size(); i++)
{
if (c >= 78)
{
colors.erase(colors.begin() + i,colors.end());
intervals.erase(intervals.begin() + i, intervals.end());
return;
}
for (int j = 0; j < intervals[i]; j++)
{
if (c < 78)
{
f.SetCharacter(c, colors[i]);
c++;
}
}
}

}

int rang_game(Frame& f)
{
int player = 39;
int l = 2, r = 76, c = 39, w = 74;
int nc = c;
float fc = c;
int nw = 74;
int del = 10;
float s_spd = 0.1f;
int score = 0;
bool redsd = false;
while (true)
{
if (del)
{
del--;
if (del == 0)
{
if (w > 10)
w -= 2;
else
{
w = 74;
if(s_spd<1.f)
s_spd += 0.1f;
}
nc = rand() % (78 - w) + (w / 2);
}
}
else
{
if (c == nc)
{
del = 4+rand()%32;
}
else
{
if (nc > c)
{
fc += s_spd;
}
else
fc -= s_spd;

c = static_cast<int>(fc);

l = c - (w / 2);
r = c + (w / 2);
}
}

score++;
if (_kbhit())
{
int ch = _getch();
if ((ch==228)||(ch==148))//((ch == 'a')||(ch=='A'))
player--;
if ((ch==162)||(ch==130))//((ch == 'd') || (ch == 'D'))
player++;
}

if ((player <= l)||(player>=r))
return score;

for (int i = 0; i < 78; i++)
{
if ((i <= l) || (i >= r))
f.SetCharacterBC(i, 5);
else
f.SetCharacterBC(i, 0);

if (redsd)
{
if ((i == l+1) || (i == r-1))
f.SetCharacterBC(i, 6);
}
}

f.SetCharacterBC(player, 0, 9, 2);

f.Draw();
redsd = !redsd;
Sleep(20);
std::cout << std::endl;
}
}

class Dick
{
public:
int a, b;
public:
Dick(int aa, int bb) : a(aa), b(bb) {}
};

class LOL
{
public:
static int dd(Dick l)
{
return 2 * l.a;
}
};

template<class T, class U> constexpr auto&& minim(T&& t, U&& u)
{
return ((t < u) ? std::forward<T&&>(t): std::forward<U&&>(u));
}

template <typename fn_t>
DWORD fn_to_addr(fn_t fn) { // convert function to DWORD for printing
union U {
fn_t fn;
DWORD addr;
};
U u;
u.fn = fn;
return u.addr;
}*/

/*

class ArtFrame
{
	ARTFrameHeader header;
	char*                  data;
	vector<vector<BYTE>> pixels;

	int px, py;
	bool Inc()
	{
		px++;
		if (px >= header.width)
		{
			px = 0;
			py++;
		}
		if (py >= header.height) return false;
		if (py < 0) return false;
		return true;
	}

	void Dec()
	{
		px--;
		if (px < 0)
		{
			px = header.width - 1;
			py--;
		}
	}

	void Reset()
	{
		px = py = 0;
	}

	bool EOD()
	{
		if (py < header.height) return false;
		return true;
	}

public:
	ARTFrameHeader& GetHeader() { return header; }
	void LoadHeader(ifstream& source)
	{
		source.read(reinterpret_cast<char*>(&header), sizeof(header));
	}

	void SaveHeader(ofstream& dest)
	{
		dest.write(reinterpret_cast<char*>(&header), sizeof(header));
	}

	void Load(ifstream& source)
	{
		data = new char[header.size];
		source.read(data, header.size);
	}

	void Save(ofstream& source)
	{
		source.write(data, header.size);
	}


	BYTE GetValue(int x, int y)
	{
		return pixels[y][x];
	}

	BYTE GetValueI(int x, int y)
	{
		return pixels[pixels.size() - 1 - y][x];
	}

	void SetValue(int x, int y, BYTE ch)
	{
		pixels[y][x] = ch;
	}

	void SetSize(int w, int h)
	{
		header.width = w;
		header.height = h;
		pixels = vector<vector<BYTE>>(header.height, vector<BYTE>(header.width));
	}

	void Encode()
	{
		string data_compressed;
		string data_raw;

		Reset();
		do
		{
			char clones = 0;
			char val = GetValueI(px, py);
			if (!Inc())
			{
				data_compressed += 0x81;
				data_compressed += val;
			}
			else
			{
				if (val == GetValueI(px, py))
				{
					clones = 2;
					while (Inc() && (val == GetValueI(px, py)) && (clones<0x7F))
					{
						clones++;
					}
					data_compressed += clones;
					data_compressed += val;
				}
				else
				{
					clones = 2;
					data_compressed += '\0';
					data_compressed += val;
					data_compressed += GetValueI(px, py);
					while (Inc() && (GetValueI(px, py) != data_compressed.back()) && (clones < 0x7F))
					{
						data_compressed += GetValueI(px, py);
						clones++;
					}
					if ((!EOD()) && (GetValueI(px, py) == data_compressed.back()))
					{
						clones--;
						data_compressed.resize(data_compressed.size() - 1);
						Dec();
					}
					data_compressed[data_compressed.size() - clones - 1] = (0x80 + clones);
				}
			}
		} while (!EOD());

		Reset();
		while (!EOD())
		{
			data_raw += GetValueI(px, py);
			Inc();
		}
		Reset();

		if (data_raw.size() <= data_compressed.size())
		{
			data = new char[data_raw.size()];
			memcpy(data, data_raw.c_str(), data_raw.size());
			header.size = data_raw.size();
		}
		else
		{
			data = new char[data_compressed.size()];
			memcpy(data, data_compressed.c_str(), data_compressed.size());
			header.size = data_compressed.size();
		}
	}

	void Decode()
	{
		pixels = vector<vector<BYTE>>(header.height, vector<BYTE>(header.width));
		Reset();
		if (header.size != (header.height*header.width))
		{
			for (int p = 0; p < header.size; p++)
			{
				BYTE ch = static_cast<BYTE>(data[p]);
				if (ch & 0x80)
				{
					int to_copy = ch&(0x7F);
					while (to_copy--)
					{
						p++;
						pixels[py][px] = data[p];
						Inc();
					}
				}
				else
				{
					int to_clone = ch&(0x7F);
					p++;
					BYTE src = static_cast<BYTE>(data[p]);
					while (to_clone--)
					{
						pixels[py][px] = src;
						Inc();
					}
				}
			}
		}
		else
		{
			for (int p = 0; p < header.size; p++)
			{
				pixels[py][px] = data[p];
				Inc();
			}
		}
	}
};

class ArtFile
{
	ARTheader header;
	std::vector<ArtFrame> frame_data;
	std::vector<CTABLE_255> palette_data;

	int palettes;
	int frames;
	int key_frame;
	bool animated;

	BITMAPFILEHEADER hdr;
	BITMAPINFOHEADER ihdr;
	BITMAPCOREHEADER chdr;
	BITMAPV4HEADER v5;
public:
	void LoadArt(string fname)
	{
		ifstream source;
		source.open(fname, ios_base::binary);
		source.read(reinterpret_cast<char*>(&header), sizeof(header));

		animated = ((header.h0[0] & 0x1) == 0);

		palettes = 0;
		for (auto col : header.stupid_color)
		{
			if (in_palette(col)) palettes++;
		}
		frames = header.frame_num;
		key_frame = header.frame_num_low;

		if (animated) frames *= 8;

		for (int i = 0; i < palettes; i++)
		{
			palette_data.push_back(CTABLE_255());
			source.read(reinterpret_cast<char*>(&palette_data.back()), sizeof(CTABLE_255));
		}

		for (int i = 0; i < frames; i++)
		{
			frame_data.push_back(ArtFrame());
			frame_data.back().LoadHeader(source);
		}

		for (auto &af : frame_data)
		{
			af.Load(source);
			af.Decode();
		}
	}

	void SaveArt(string fname)
	{
		ofstream source;
		source.open(fname, ios_base::binary);
		source.write(reinterpret_cast<char*>(&header), sizeof(header));

		for (int i = 0; i < palettes; i++)
		{
			source.write(reinterpret_cast<char*>(&palette_data[i]), sizeof(CTABLE_255));
		}

		for (auto &af : frame_data)
		{
			af.Encode();
			af.SaveHeader(source);
		}

		for (auto &af : frame_data)
		{
			af.Save(source);
		}

		source.close();
	}

	void LoadBMPS(string fname)
	{
		ifstream ctrl;
		ctrl.open(fname);

		if (ctrl)
		{
			string str;
			ctrl >> str;
			ctrl >> frames;
			header.frame_num = frames;
			ctrl >> str;
			ctrl >> key_frame;
			header.frame_num_low = key_frame;
			ctrl >> str;
			ctrl >> palettes;

			ctrl >> str;//header:

			LoadHeader(ctrl, header);

			animated = ((header.h0[0] & 0x1) == 0);
			if (animated)
				header.frame_num /= 8;

			palette_data.resize(palettes);

			for (int i = 0; i < palettes; i++)
			{
				ctrl >> str; //palette
				ctrl >> str; //palette number
				for (auto &c : palette_data[i].colors)
				{
					LoadCOLOR(ctrl, c);
				}
			}

			frame_data.resize(frames);
			for (int i = 0; i < frames; i++)
			{
				ctrl >> str; //frame
				ctrl >> str; //frame number
				ctrl >> str; //center_x
				ctrl >> frame_data[i].GetHeader().c_x;
				ctrl >> str; //center_y
				ctrl >> frame_data[i].GetHeader().c_y;

				ctrl >> str; //offset_x
				ctrl >> frame_data[i].GetHeader().d_x;
				ctrl >> str; //offset_y
				ctrl >> frame_data[i].GetHeader().d_y;
			}

			ctrl.close();

			string base_name = fname.substr(0, fname.size() - 4);
			int frame_num = 0;
			for (auto &af : frame_data)
			{
				ostringstream oss;
				if (!animated)
					oss << base_name << "_" << frame_num << ".bmp";
				else
					oss << base_name << "_" << (frame_num / 8) << (frame_num % 8) << ".bmp";
				frame_num++;

				std::ifstream src;
				src.open(oss.str(), std::ios_base::binary);
				src.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
				src.read(reinterpret_cast<char*>(&ihdr), sizeof(ihdr));

				CTABLE_255 ignore;
				src.read(reinterpret_cast<char*>(&ignore), sizeof(ignore));

				int offset = sizeof(hdr) + sizeof(ihdr) + sizeof(ignore);
				unsigned char ch;

				int height = ihdr.biHeight;
				int width = ihdr.biWidth;
				int offbits = hdr.bfOffBits;

				int stride = ((width + 3) / 4) * 4;

				while (offset < hdr.bfOffBits)
				{
					offset++;
					src.read(reinterpret_cast<char*>(&ch), 1);
				}

				af.SetSize(width, height);

				for (int i = 0; i < height; i++)
				{
					for (int j = 0; j < width; j++)
					{
						src.read(reinterpret_cast<char*>(&ch), 1);
						af.SetValue(j, i, ch);
					}
					offset = width;
					while (offset < stride)
					{
						offset++;
						src.read(reinterpret_cast<char*>(&ch), 1);
					}
				}

				src.close();
			}
		}
	}

	void SaveDWORD(ofstream& dst, DWORD data)
	{
		for (int i = 0; i < 8; i++)
		{
			BYTE ch = (data >> (i * 4)) & 0xF;
			ch = "0123456789ABCDEF"[ch];
			dst << ch;
		}
		dst << " ";
	}

	void LoadDWORD(ifstream& dst, DWORD &data)
	{
		string str;
		dst >> str;

		std::reverse(str.begin(), str.end());

		DWORD res = 0;

		for (int i = 0; i < 8; i++)
		{
			res <<= 4;
			BYTE ch = str[i];
			switch (ch)
			{
			case '0': res += 0; break;
			case '1': res += 1; break;
			case '2': res += 2; break;
			case '3': res += 3; break;
			case '4': res += 4; break;
			case '5': res += 5; break;
			case '6': res += 6; break;
			case '7': res += 7; break;
			case '8': res += 8; break;
			case '9': res += 9; break;
			case 'A': res += 10; break;
			case 'B': res += 11; break;
			case 'C': res += 12; break;
			case 'D': res += 13; break;
			case 'E': res += 14; break;
			case 'F': res += 15; break;
			default:return;
			}
		}
		data = res;
	}

	void LoadCOLOR(ifstream& dst, COLOR_4B &data)
	{
		DWORD d;
		LoadDWORD(dst, d);
		data.a = (d & 0xFF000000) >> 24;
		data.b = (d & 0x00FF0000) >> 16;
		data.g = (d & 0x0000FF00) >> 8;
		data.r = (d & 0x000000FF);
	}

	void SaveCOLOR(ofstream& dst, COLOR_4B col)
	{
		DWORD d = (col.a << 24) | (col.b << 16) | (col.g << 8) | col.r;
		SaveDWORD(dst, d);
	}

	void SaveHeader(ofstream& dst, ARTheader& h)
	{
		for (int i = 0; i < 3; i++)
			SaveDWORD(dst, h.h0[i]);
		for (int i = 0; i < 4; i++)
			SaveCOLOR(dst, h.stupid_color[i]);
		for (int i = 0; i < 8; i++)
			SaveCOLOR(dst, h.palette_data1[i]);
		for (int i = 0; i < 8; i++)
			SaveCOLOR(dst, h.palette_data2[i]);
		for (int i = 0; i < 8; i++)
			SaveCOLOR(dst, h.palette_data3[i]);
	}

	void LoadHeader(ifstream& dst, ARTheader& h)
	{
		for (int i = 0; i < 3; i++)
			LoadDWORD(dst, h.h0[i]);
		for (int i = 0; i < 4; i++)
			LoadCOLOR(dst, h.stupid_color[i]);
		for (int i = 0; i < 8; i++)
			LoadCOLOR(dst, h.palette_data1[i]);
		for (int i = 0; i < 8; i++)
			LoadCOLOR(dst, h.palette_data2[i]);
		for (int i = 0; i < 8; i++)
			LoadCOLOR(dst, h.palette_data3[i]);
	}

	void SaveBMPS(string fname)
	{
		ostringstream gss;
		gss << fname << ".ini";
		ofstream ctrl;
		ctrl.open(gss.str());
		ctrl << "frames: " << frames << "\r\n";
		ctrl << "key_frame: " << key_frame << "\r\n";
		ctrl << "palettes: " << palettes << "\r\n";

		ctrl << "header: \r\n";
		SaveHeader(ctrl, header);

		ctrl << "\r\n";
		for (int i = 0; i < palettes; i++)
		{
			ctrl << "palette " << i << ":\r\n";
			for (auto c : palette_data[i].colors)
			{
				SaveCOLOR(ctrl, c);
				ctrl << "\r\n";
			}
		}

		for (int i = 0; i < frames; i++)
		{
			if (animated)
				ctrl << "frame " << (i / 8) << "_" << (i % 8) << ":\r\n";
			else
				ctrl << "frame " << i << ":\r\n";
			ctrl << "center_x: " << frame_data[i].GetHeader().c_x << "\r\n";
			ctrl << "center_y: " << frame_data[i].GetHeader().c_y << "\r\n";
			ctrl << "offset_x: " << frame_data[i].GetHeader().d_x << "\r\n";
			ctrl << "offset_y: " << frame_data[i].GetHeader().d_y << "\r\n";
		}

		ctrl.close();

		int frame_num = 0;
		for (auto& af : frame_data)
		{
			ostringstream oss;
			if (!animated)
				oss << fname << "_" << frame_num << ".bmp";
			else
				oss << fname << "_" << (frame_num / 8) << (frame_num % 8) << ".bmp";
			frame_num++;

			hdr.bfOffBits = sizeof(hdr) + sizeof(ihdr) + sizeof(CTABLE_255);
			hdr.bfReserved1 = 28020;
			hdr.bfReserved2 = 115;

			int stride = ((af.GetHeader().width + 3) / 4) * 4;

			hdr.bfSize = sizeof(hdr) + sizeof(ihdr) + sizeof(CTABLE_255) + af.GetHeader().height*stride - 40;
			hdr.bfType = 19778;

			ihdr.biBitCount = 8;
			ihdr.biClrImportant = 0;
			ihdr.biClrUsed = 0;
			ihdr.biCompression = 0;
			ihdr.biHeight = af.GetHeader().height;
			ihdr.biPlanes = 1;
			ihdr.biSize = 40;
			ihdr.biWidth = af.GetHeader().width;

			ofstream dst;
			dst.open(oss.str(), ios_base::binary);
			dst.write(reinterpret_cast<char*>(&hdr), sizeof(hdr));
			dst.write(reinterpret_cast<char*>(&ihdr), sizeof(ihdr));

			dst.write(reinterpret_cast<char*>(&palette_data[0]), sizeof(CTABLE_255));

			int offset = sizeof(hdr) + sizeof(ihdr) + sizeof(CTABLE_255);

			char ch = 0;
			while (offset < hdr.bfOffBits)
			{
				offset++;
				dst.write(&ch, 1);
			}

			for (int y = af.GetHeader().height - 1; y >= 0; y--)
			{
				for (int x = 0; x < af.GetHeader().width; x++)
				{
					ch = af.GetValue(x, y);
					dst.write(&ch, 1);
				}
				ch = 0;
				offset = af.GetHeader().width;
				while (offset < stride)
				{
					offset++;
					dst.write(&ch, 1);
				}
			}

			dst.close();
		}
	}

	void Draw(int p_num, int f_num)
	{
		HDC hdc = GetDC(NULL);
		ArtFrame& af = frame_data[f_num];
		CTABLE_255& ap = palette_data[p_num];
		for (int y = 0; y<af.GetHeader().height; y++)
			for (int x = 0; x < af.GetHeader().width; x++)
			{
				BYTE b = af.GetValue(x, y);
				SetPixelV(hdc, x, y, RGB(ap.colors[b].r, ap.colors[b].g, ap.colors[b].b));
			}

	}
};
*/