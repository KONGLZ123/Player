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
    char bin_data[256];
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
    char bin_data[256];
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
    char bin_data[256];
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
    void ParserMvhd(long sz);
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

