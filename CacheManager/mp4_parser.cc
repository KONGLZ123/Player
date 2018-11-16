#include "mp4_parser.h"
#include <iostream>
#include <queue>
#include <vector>

#pragma warning(disable:4996)

Mp4Parser::Mp4Parser() :
    mdat_(nullptr),
    file_size_(0)
{
    file_.open("E:\\project_code\\CacheManager\\CacheManager\\cache\\e9ef49324bdb4f239bd03604c8d36952", std::ios::binary | std::ios::in);
    if (file_.is_open())
    {
        file_.seekg(0, std::ios::end);
        file_size_ = file_.tellg();
        file_.seekg(0, std::ios::beg);
    }
}


Mp4Parser::~Mp4Parser()
{
    file_.close();
}


void Mp4Parser::Parser()
{
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
            // TODO: 需要流式解析，数据可能不完全，解析过程中需要剔除free box
            else if (strncmp(box_type, "mdat", 4) == 0)  
            {
                int pos = file_.tellg();
                int sz = CalcSize((uint8_t*)size);
                mdat_ = new Mdat(sz);
                if (mdat_)
                {
                    ParserMdat(mdat_);
                }
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

uint32_t Mp4Parser::ReadUInt32()
{
    char tmp[4];
    file_.read(tmp, 4);
    return CalcSize((uint8_t*)tmp);
}

uint16_t Mp4Parser::ReadUInt16()
{
    char tmp[2];
    file_.read(tmp, 2);
    uint16_t n;

    return ((uint16_t)tmp[0]<<8 | (uint16_t)tmp[1]);
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
            ParserMvhd(moov_.mvhd);
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

void Mp4Parser::ParserMvhd(Mvhd& mvhd)
{
    file_.read(&mvhd.version, 1);
    file_.read(mvhd.flags, 3);
    mvhd.create_time = ReadUInt32();
    mvhd.modification_time = ReadUInt32();
    mvhd.time_scale = ReadUInt32();
    mvhd.duration = ReadUInt32();
    mvhd.rate = readBigEndianFixedPoint(16, 16);
    mvhd.volume = readBigEndianFixedPoint(8, 8);
    file_.read(mvhd.reserved, 10);
    file_.read(mvhd.matrix, 36);
    file_.read(mvhd.pre_defined, 24);
    mvhd.next_track_id = ReadUInt32();
}

void Mp4Parser::ParserTkhd(Tkhd & tkhd)
{
    file_.read(&tkhd.version, 1);
    file_.read(tkhd.flags, 3);
    tkhd.create_time = ReadUInt32();
    tkhd.modification_time = ReadUInt32();
    tkhd.track_id = ReadUInt32();
    file_.read(tkhd.reserved, 4);
    tkhd.duration = ReadUInt32();
    file_.read(tkhd.reserved2, 8);
    tkhd.layer = ReadUInt16();
    tkhd.alternate_group = ReadUInt16();
    tkhd.voluem = readBigEndianFixedPoint(8, 8);
    file_.read(tkhd.reserved3, 2);
    file_.read(tkhd.matrix, 36);
    tkhd.width = ReadUInt32();
    tkhd.height = ReadUInt32();
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
            ParserTkhd(trak.tkhd);
        }
        else if (strncmp(box_type, "tref", 4) == 0)
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
        else if (strncmp(box_type, "mdia", 4) == 0)
        {
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
            ParserMdhd(mdia.mdhd);
            //file_.read(mdia.mdhd.bin_data, mdia.mdhd.size - 8);
        }
        else if (strncmp(box_type, "hdlr", 4) == 0)
        {
            mdia.hdrl.size = CalcSize((uint8_t*)size);
            strcpy(mdia.hdrl.box_type, box_type);
            ParserHdrl(mdia.hdrl);
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

void Mp4Parser::ParserMdhd(Mdhd & mdhd)
{

    file_.read(&mdhd.version, 1);
    file_.read(mdhd.flags, 3);
    mdhd.create_time = ReadUInt32();
    mdhd.modification_time = ReadUInt32();
    mdhd.time_scale = ReadUInt32();
    mdhd.duration = ReadUInt32();
    file_.read(mdhd.language, 2);
    file_.read(mdhd.pre_defined, 2);
}

void Mp4Parser::ParserHdrl(Hdlr & hdlr)
{
    file_.read(&hdlr.version, 1);
    file_.read(hdlr.flags, 3);
    file_.read(hdlr.pre_defined, 4);
    file_.read(hdlr.handler_type, 4);
    file_.read(hdlr.reserved, 4);
    hdlr.name = (char*)malloc(hdlr.size - 8 - 16);
    file_.read(hdlr.name, hdlr.size - 8 - 16);
}

void Mp4Parser::ParserVmhd(Vmhd & vmhd)
{
    file_.read(&vmhd.version, 1);
    file_.read(vmhd.flags, 3);
    file_.read(vmhd.graphics_mode, 4);
    file_.read(vmhd.opcolor, vmhd.size - 8 - 8);
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
            //file_.read(minf.vmhd.bin_data, minf.vmhd.size - 8);
            ParserVmhd(minf.vmhd);
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
            ParserVmhd(minf.vmhd);
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
            //uint32_t len = CalcSize((uint8_t*)size);
            //char temp[256];
            //file_.read(temp, len - 8);
            stbl.stsc.size = CalcSize((uint8_t*)size);
            strcpy(stbl.stsc.box_type, box_type);
            ParserStsc(stbl.stsc);
        }
        else if (strncmp(box_type, "stco", 4) == 0)
        {
            stbl.stco.size = CalcSize((uint8_t*)size);
            strcpy(stbl.stco.box_type, box_type);
            ParserStco(stbl.stco);
            //uint32_t len = CalcSize((uint8_t*)size);
            //char temp[256];
            //file_.read(temp, len - 8);
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
            file_.read(&dinf.version, 1);
            file_.read(dinf.flags, 3);
            file_.read(dinf.entry_count, 4);
            uint32_t len = CalcSize((uint8_t*)dinf.entry_count);
            //dinf.url_or_urn = (char*)malloc(len);
            //file_.read(dinf.url_or_urn, len);

            file_.read(size, 4);
            file_.read(box_type, 4);
            if (strncmp(box_type, "url ", 4) == 0)
            {
                dinf.dref.url.size = CalcSize((uint8_t*)size);
                strcpy(dinf.dref.url.box_type, box_type);
                file_.read(dinf.dref.url.bin_data, dinf.dref.url.size - 8);
            }
            else
            {
                uint32_t len = CalcSize((uint8_t*)size);
                char temp[256];
                file_.read(temp, len - 8);
                //finish = true;
            }
        }
        else
        {
            uint32_t len = CalcSize((uint8_t*)size);
            char temp[256];
            file_.read(temp, len - 8);
        }
    }
}

void Mp4Parser::ParserMdat(Mdat* mdat)
{
    char ch;
    char tmp[8];
    char free_type[4];
    int off = 0;
    while (!file_.eof())
    {
        file_.read(&ch, 1);     // 每次读一个，因为不知道什么时候结束
        tmp[off++] = ch;
        if (off == 7)
        {
            off = 0;
            for (int i = 0; i < 4; ++i)
                free_type[i] = tmp[4 + i];
            if (strcmp(free_type, "free") == 0)
            {
                int size = CalcSize((uint8_t*)tmp);
                char *f = (char*)malloc(size);
                file_.read(f, size - 8);
                free(f);
            }
            else
            {
                mdat->Append(tmp, 8);
            }
        }
    }

    if (off != 7)
    {
        mdat->Append(tmp, off);
    }
}

void Mp4Parser::ParserStsc(Stsc& stsc)
{
    file_.read(&stsc.version, 1);
    file_.read(stsc.flags, 3);
    stsc.entry_count = ReadUInt32();
    for (int i = 0; i < stsc.entry_count; ++i)
    {
        Stsc::Sample sample;
        sample.first_chunk = ReadUInt32();
        sample.samples_per_chunk = ReadUInt32();
        sample.sample_description_index = ReadUInt32();
        stsc.samples.push_back(sample);
    }
}

void Mp4Parser::ParserStco(Stco & stco)
{
    file_.read(&stco.version, 1);
    file_.read(stco.flags, 3);
    stco.entry_count = ReadUInt32();
    for (int i = 0; i < stco.entry_count; ++i)
    {
        uint32_t off = ReadUInt32();
        stco.chunk_offset.push_back(off);
    }
}

uint32_t Mp4Parser::readBigEndianUnsignedInteger(void)
{
    uint8_t  c[4];
    uint32_t n;

    file_.read((char *)c, 4);

    n = (uint32_t)c[0] << 24
        | (uint32_t)c[1] << 16
        | (uint32_t)c[2] << 8
        | (uint32_t)c[3];

    return n;
}

uint16_t Mp4Parser::readBigEndianUnsignedShort(void)
{
    uint8_t  c[2];
    uint16_t n;

    file_.read((char *)c, 2);

    n = (uint16_t)c[0] << 8
        | (uint16_t)c[1];

    return n;
}

float Mp4Parser::readBigEndianFixedPoint(unsigned int integerLength, unsigned int fractionalLength)
{
    uint32_t n;
    unsigned int integer;
    unsigned int fractionalMask;
    unsigned int fractional;

    if (integerLength + fractionalLength == 16)
    {
        n = this->readBigEndianUnsignedShort();
    }
    else
    {
        n = this->readBigEndianUnsignedInteger();
    }

    integer = n >> fractionalLength;
    fractionalMask = pow(2, fractionalLength) - 1;
    fractional = (n & fractionalMask) / (1 << fractionalLength);

    return integer + fractional;
}

