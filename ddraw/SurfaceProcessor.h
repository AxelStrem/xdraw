#pragma once

#include <vector>
#include "unordered_vector.hpp"
#include "cyclic_buf.hpp"
#include <queue>

#include "Recorder.h"

#include <mutex>

class CircularBuffer;

struct SPSurface
{
    int xs;
    int ys;
    std::vector<WORD> data;

    SPSurface(int xs_ = 0, int ys_ = 0) : xs(xs_), ys(ys_), data(xs*ys) {}
};

struct ModRect
{
    int handle;
    RecRect r;
};

class SurfaceProcessor
{
    unordered_vector<SPSurface> surfaces;

    CircularBuffer& pSource;
    CircularBuffer& pDest;

    std::queue<ModRect, cyclic_buffer<ModRect>> orders;
    std::mutex orders_access;
public:
    SurfaceProcessor(CircularBuffer& pD, CircularBuffer& pS) : pSource(pS), pDest(pD) {}
    ~SurfaceProcessor();

    void CreateTexture(int h, int xs, int ys);
    void DestroyTexture(int h);
    void ModifyRect(int h, RecRect r);
    void Process();


    void ClearQueue()
    {
        std::unique_lock<std::mutex> ul{ orders_access };
        while(!orders.empty())
            orders.pop();
    }
};

