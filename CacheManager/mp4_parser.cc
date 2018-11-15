#include "mp4_parser.h"
#include <iostream>

#pragma warning(disable:4996)

Mp4Parser::Mp4Parser()
{
    file_.open("E:\\project_code\\CacheManager\\CacheManager\\cache\\2cd73f68510b4786a11a77e06ae254ac", std::ios::binary | std::ios::in);
}


Mp4Parser::~Mp4Parser()
{
    file_.close();
}



void Mp4Parser::Parser()
{
    int offset = 0;
    if (file_.is_open())
    {
        char size[4];
        char box_type[4];
        char tmp[4];

        while (!file_.eof())
        {
            file_.read(size, 4);
            file_.read(box_type, 4);
            if (strncmp(box_type, "ftyp", 4) == 0)
            {
                ftyp_.size = CalcSize((uint8_t*)size);
                strcpy(ftyp_.box_type, box_type);
                file_.read(ftyp_.major_brand, 4);
                file_.read(tmp, 4);
                ftyp_.minor_version = static_cast<int>(tmp[3]);
                file_.read(ftyp_.compatible_brands, ftyp_.size - 16);
            }
            else if (strncmp(box_type, "moov", 4) == 0)
            {
                moov_.size = CalcSize((uint8_t*)size);
                strcpy(moov_.box_type, box_type);
                ParserMoov(moov_.size);
            }
            else if (strncmp(box_type, "mdat", 4) == 0)
            {
                int sz = CalcSize((uint8_t*)size);
                char* tmp = (char*)malloc(sz);
                file_.read(tmp, sz - 8);
                free(tmp);
            }
            else if (strncmp(box_type, "free", 4) == 0)
            {
                int sz = CalcSize((uint8_t*)size);
                char* tmp = (char*)malloc(sz);
                file_.read(tmp, sz - 8);
                free(tmp);
            }
            else
            {
                uint32_t len = CalcSize((uint8_t*)size);
                char temp[256];
                file_.read(temp, len);
            }
        }
    }

    std::cout << "main thread quit" << std::endl;
}

uint32_t Mp4Parser::CalcSize(uint8_t size[])
{
    //return size[3] + size[2] * 100 + size[1] * 10000 + size[0] * 1000000;

    uint32_t n;

    n = (uint32_t)size[0] << 24;
    n |= (uint32_t)size[1] << 16;
    n |= (uint32_t)size[2] << 8;
    n |= (uint32_t)size[3];

    return n;
}

void Mp4Parser::ParserMoov(long sz)
{
    char size[4];
    char box_type[4];
    int finish_pos = moov_.size + file_.tellg() - 8;

    while (true)
    {
        if (static_cast<int>(file_.tellg()) == finish_pos)
            break;

        file_.read(size, 4);
        file_.read(box_type, 4);
        if (strncmp(box_type, "mvhd", 4) == 0)
        {
            moov_.mvhd.size = CalcSize((uint8_t*)size);
            strcpy(moov_.mvhd.box_type, box_type);
            ParserMvhd(moov_.mvhd.size);
        }
        else if (strncmp(box_type, "trak", 4) == 0)
        {
            Trak trak;
            trak.size = CalcSize((uint8_t*)size);
            strcpy(trak.box_type, box_type);
            ParserTrak(trak);
            moov_.tracks.push_back(trak);
        }
        else if (strncmp(box_type, "iods", 4) == 0)
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
        else
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
    }
}

void Mp4Parser::ParserMvhd(long sz)
{
    file_.read(moov_.mvhd.bin_data, sz - 8);
}

void Mp4Parser::ParserTrak(Trak& trak)
{
    char size[4];
    char box_type[4];
    int finish_pos = trak.size + file_.tellg() - 8;

    while (true)
    {
        if (static_cast<int>(file_.tellg()) == finish_pos)
            break;

        file_.read(size, 4);
        file_.read(box_type, 4);
        if (strncmp(box_type, "tkhd", 4) == 0)
        {
            trak.tkhd.size = CalcSize((uint8_t*)size);
            strcpy(trak.tkhd.box_type, box_type);
            file_.read(trak.tkhd.bin_data, trak.tkhd.size - 8);
        }
        else if (strncmp(box_type, "tref", 4) == 0)
        {
            Trak trak;
            trak.size = CalcSize((uint8_t*)size);
            strcpy(trak.box_type, box_type);

        }
        else if (strncmp(box_type, "mdia", 4) == 0)
        {
            Trak trak;
            trak.mdia.size = CalcSize((uint8_t*)size);
            strcpy(trak.mdia.box_type, box_type);
            ParserMedi(trak.mdia);
        }
        else
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
    }
}

void Mp4Parser::ParserMedi(Mdia& mdia)
{
    char size[4];
    char box_type[4];
    int finish_pos = mdia.size + file_.tellg() - 8;

    while (true)
    {
        if (static_cast<int>(file_.tellg()) == finish_pos)
            break;

        file_.read(size, 4);
        file_.read(box_type, 4);
        if (strncmp(box_type, "mdhd", 4) == 0)
        {
            mdia.mdhd.size = CalcSize((uint8_t*)size);
            strcpy(mdia.mdhd.box_type, box_type);
            file_.read(mdia.mdhd.bin_data, mdia.mdhd.size - 8);
        }
        else if (strncmp(box_type, "hdlr", 4) == 0)
        {
            mdia.hdrl.size = CalcSize((uint8_t*)size);
            strcpy(mdia.hdrl.box_type, box_type);
            file_.read(mdia.hdrl.bin_data, mdia.hdrl.size - 8);
        }
        else if (strncmp(box_type, "minf", 4) == 0)
        {
            mdia.minf.size = CalcSize((uint8_t*)size);
            strcpy(mdia.minf.box_type, box_type);
            ParserMinf(mdia.minf);
        }
        else
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
    }
}

void Mp4Parser::ParserMinf(Minf& minf)
{
    char size[4];
    char box_type[4];
    int finish_pos = minf.size + file_.tellg() - 8;

    while (true)
    {
        if (static_cast<int>(file_.tellg()) == finish_pos)
            break;

        file_.read(size, 4);
        file_.read(box_type, 4);

        if (strncmp(box_type, "vmhd", 4) == 0)
        {
            minf.vmhd.size = CalcSize((uint8_t*)size);
            strcpy(minf.vmhd.box_type, box_type);
            file_.read(minf.vmhd.bin_data, minf.vmhd.size - 8);
        }
        else if (strncmp(box_type, "dinf", 4) == 0)
        {
            minf.dinf.size = CalcSize((uint8_t*)size);
            strcpy(minf.dinf.box_type, box_type);
            ParserDinf(minf.dinf);
        }
        else if (strncmp(box_type, "stbl", 4) == 0)
        {
            minf.stbl.size = CalcSize((uint8_t*)size);
            strcpy(minf.stbl.box_type, box_type);
            ParserStbl(minf.stbl);
        }
        else if (strncmp(box_type, "smhd", 4) == 0)
        {
            minf.vmhd.size = CalcSize((uint8_t*)size);
            strcpy(minf.vmhd.box_type, box_type);
            file_.read(minf.vmhd.bin_data, minf.vmhd.size - 8);
        }
        else
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
    }
}

void Mp4Parser::ParserStbl(Stbl& stbl)
{
    char size[4];
    char box_type[4];
    int finish_pos = stbl.size + file_.tellg() - 8;

    while (true)
    {
        if (static_cast<int>(file_.tellg()) == finish_pos)
            break;

        file_.read(size, 4);
        file_.read(box_type, 4);
        if (strncmp(box_type, "stsd", 4) == 0)
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
        else if (strncmp(box_type, "avc1", 4) == 0)
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
        else if (strncmp(box_type, "btrt", 4) == 0)
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
        else if (strncmp(box_type, "stts", 4) == 0)
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
        else if (strncmp(box_type, "ctts", 4) == 0)
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[5000];
            file_.read(temp, len - 8);
        }
        else if (strncmp(box_type, "stss", 4) == 0)
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
        else if (strncmp(box_type, "stsc", 4) == 0)
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
        else if (strncmp(box_type, "stco", 4) == 0)
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
        else if (strncmp(box_type, "stsz", 4) == 0)
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[2560];
            file_.read(temp, len - 8);
        }
        else
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
    }
}

void Mp4Parser::ParserDinf(Dinf & dinf)
{
    char size[4];
    char box_type[4];
    int finish_pos = dinf.size + file_.tellg() - 8;

    while (true)
    {
        if (static_cast<int>(file_.tellg()) == finish_pos)
            break;

        file_.read(size, 4);
        file_.read(box_type, 4);
        if (strncmp(box_type, "dref", 4) == 0)
        {
            dinf.dref.size = CalcSize((uint8_t*)size);
            strcpy(dinf.dref.box_type, box_type);
            file_.read(dinf.dref.bin_data, dinf.dref.size - 8);

            //file_.read(size, 4);
            //file_.read(box_type, 4);
            //if (strncmp(box_type, "url", 4) == 0)
            //{
            //    dinf.dref.url.size = CalcSize((uint8_t*)size);
            //    strcpy(dinf.dref.url.box_type, box_type);
            //    file_.read(dinf.dref.url.bin_data, dinf.dref.url.size - 8);
            //}
            //else
            //{
            //    uint32_t len = CalcSize((uint8_t*)size);
            //    char temp[256];
            //    file_.read(temp, len - 8);
            //    //finish = true;
            //}
        }
        else
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
    }
}
