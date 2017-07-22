// encoder_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../ddraw/EDirectDraw.h"
#include "../ddraw/ESurface.h"
#include "../ddraw/ECPUSurface.h"
#include "../ddraw/Recorder.h"

void RunRecordingThread()
{
    while (true)
    {
        global_recording_buffer.Save();
    }
}

Encoder gEx{ global_recording_buffer };

void RunEncodingThread()
{
    while (true)
    {
        global_encoding_buffer.Encode(gEx);
    }
}

int main()
{
    std::thread recorder_thread(RunRecordingThread);
    recorder_thread.detach();

    std::thread encorder_thread(RunEncodingThread);
    encorder_thread.detach();

    global_out_file_stream.open("E:\\records\\test.ars", std::ios_base::binary);
    global_is_recording = true;

    std::vector<WORD> v_red(1920 * 2 * 1080 * 2, 0xF800);
    std::vector<WORD> v_green(1920 * 2 * 1080 * 2, 0x03E0);
    std::vector<WORD> v_blue(1920 * 2 * 1080 * 2, 0x001F);

    for (int i = 0; i < 2000; i++)
    {
        record_stream(v_red.data(), 1920 * 2, 1080 * 2, 1920 * 2);
        record_stream(v_green.data(), 1920 * 2, 1080 * 2, 1920 * 2);
        record_stream(v_blue.data(), 1920 * 2, 1080 * 2, 1920 * 2);
    }

    return 0;
}

