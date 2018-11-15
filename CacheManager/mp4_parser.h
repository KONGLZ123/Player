#pragma once
#include <fstream>
#include <vector>

//struct BoxType
//{
//    char* FtypBox = "ftyp";
//    char* MoovBox = "moov";
//    char* MoovBox = "moov";
//    char* MoovBox = "moov";
//};

struct Ftyp
{
    long size;
    char box_type[4];
    char major_brand[4];
    int minor_version;
    char compatible_brands[8];
};

struct Mvhd
{
    long size;
    char box_type[4];
    char version;
    char flags[3];
    char create_time[4];    // 创建时间
    char modification_time[4];  // 修改时间
    char time_scale[4]; // 文件媒体在1秒时间内的刻度值
    char duration[4];   // track时间长度 = time_scale*duration;
    char rate[4];       // 推荐播放速率 [16.16]
    char volume[2];     // 音量 [8.8]
    char reserved[10];
    char matrix[36];    // 视频变换矩阵
    char pre_defined[24];   
    char next_track_id[4];  // 下一个track id
};

struct Url
{
    long size;
    char box_type[4];
    char bin_data[256];
};

struct Dref
{
    long size;
    char box_type[4];
    char bin_data[256];
    Url url;
};

struct Dinf
{
    long size;
    char box_type[4];
    char bin_data[256];
    Dref dref;
};

struct Stbl
{
    long size;
    char box_type[4];
    char bin_data[256];
};

struct Vmhd
{
    long size;
    char box_type[4];
    char bin_data[256];
};

struct Minf
{
    long size;
    char box_type[4];
    Stbl stbl;
    Dinf dinf;
    Vmhd vmhd;
    char bin_data[256];
};

struct Mdhd
{
    long size;
    char box_type[4];
    char version;
    char flags[3];
    char create_time[4];    // 创建时间
    char modification_time[4];  // 修改时间
    char time_scale[4]; // 文件媒体在1秒时间内的刻度值
    char duration[4];   // track时间长度 = time_scale*duration;
    char language[2];       
    char pre_defined[2];

};

struct Hdlr
{
    long size;
    char box_type[4];
    char bin_data[256];
};

struct Mdia
{
    long size;
    char box_type[4];
    Minf minf;
    Mdhd mdhd;
    Hdlr hdrl;
    char bin_data[256];
};

struct Tkhd
{
    long size;
    char box_type[4];
    char version;
    char flags[3];
    char create_time[4];    // 创建时间
    char modification_time[4];  // 修改时间
    char track_id[4];
    char reserved[4];
    char duration[4];   // track时间长度 = time_scale*duration;
    char reserved2[8];
    char layer[2];
    char alternate_group[2];
    char voluem[2];
    char reserved3[2];
    char matrix[36];    // 视频变换矩阵
    char width[4];
    char height[4];
};

struct Trak
{
    long size;
    char box_type[4];
    Tkhd tkhd;
    char hdlr[4];
    Mdia mdia;
};

struct Moov
{
    long size;
    char box_type[4];
    Mvhd mvhd;
    std::vector<Trak> tracks;
};

class Mp4Parser
{
public:
    Mp4Parser();
    ~Mp4Parser();

    void Parser();
    uint32_t CalcSize(uint8_t size[]);
    void ParserMoov(long sz);
    void ParserMvhd(Mvhd& mvhd);
    void ParserTkhd(Tkhd& tkhd);
    void ParserTrak(Trak& trak);
    void ParserMedi(Mdia& trak);
    void ParserMinf(Minf& trak);
    void ParserStbl(Stbl& trak);
    void ParserDinf(Dinf& dinf);

private:
    std::fstream file_;
    Ftyp ftyp_;
    Moov moov_;
};

