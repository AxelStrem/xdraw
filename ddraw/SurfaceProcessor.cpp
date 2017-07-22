#include "stdafx.h"
#include "SurfaceProcessor.h"

SurfaceProcessor::~SurfaceProcessor()
{
}

void SurfaceProcessor::CreateTexture(int h, int xs, int ys)
{
    if (!surfaces.emplace_at(h, xs, ys))
    {
        surfaces[h] = SPSurface(xs, ys);
    }
}

void SurfaceProcessor::DestroyTexture(int h)
{
    surfaces.erase(h);
}

void SurfaceProcessor::ModifyRect(int h, RecRect r)
{
    std::unique_lock<std::mutex> ul{ orders_access };
    orders.push(ModRect{ h, r });
}

void SurfaceProcessor::Process()
{
    std::unique_lock<std::mutex> ul{ orders_access };
    while (!orders.empty())
    {
        ModRect mr = orders.front();
        int sz = mr.r.getSize()*sizeof(WORD);
        if (pSource.Size() < sz)
            return;

        if (pDest.Space() < sz)
            return;

        orders.pop();

        ul.unlock();

        auto in = pSource.naked_begin<WORD>();
        auto out = pDest.naked_begin_out<WORD>();

        for (int i = 0; i < mr.r.sy; i++)
        {
            WORD* pImg = surfaces[mr.handle].data.data() + i*surfaces[mr.handle].xs;
            for (int j = 0; j < mr.r.sx; j++)
            {
                WORD w = *in;
                *out = w;// w ^ (*pImg);
                //*pImg = w;

                in = pSource.next(in);
                out = pDest.next(out);
                pImg++;
            }
        }
        pDest.push_naked(out);
        pSource.pop_naked(in);

        ul.lock();
    }
}

SurfaceProcessor gSP{ global_encoding_buffer, global_processing_buffer };