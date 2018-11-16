#pragma once
#include <fstream>
#include <list>

struct Stsc
{
    long size;
    char box_type[4];
    char version;
    char flags[3];
    uint32_t entry_count;
    struct Sample
    {
        uint32_t first_chunk;
        uint32_t samples_per_chunk;
        uint32_t sample_description_index;
    };
    std::list<Sample> samples;
};

struct Stco
{
    long size;
    char box_type[4];
    char version;
    char flags[3];
    std::list<uint32_t> chunk_offset;
    uint32_t entry_count;
};

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
    uint32_t create_time;    // 创建时间
    uint32_t modification_time;  // 修改时间
    uint32_t time_scale; // 文件媒体在1秒时间内的刻度值
    uint32_t duration;   // track时间长度 = time_scale*duration;
    double rate;       // 推荐播放速率 [16.16]
    float volume;     // 音量 [8.8]
    char reserved[10];
    char matrix[36];    // 视频变换矩阵
    char pre_defined[24];   
    uint32_t next_track_id;  // 下一个track id
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
    char version;
    char flags[3];
    char entry_count[4];    // “url”或“urn”表的元素个数
    char* url_or_urn;  // 不定
    Dref dref;
};

struct Co64
{
    long size;
    char box_type[4];
    char version;
    char flags[3];
    uint32_t entry_count;
    uint64_t chunk_offset;
};

struct Stbl
{
    long size;
    char box_type[4];
    //char bin_data[256];
    Stsc stsc;
    Stco stco;
    //Co64 co64;
};

struct Vmhd
{
    long size;
    char box_type[4];
    char version;
    char flags[3];
    char graphics_mode[4];  // 视频合成模式
    char opcolor[6];  // ｛red，green，blue｝
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
    uint32_t create_time;    // 创建时间
    uint32_t modification_time;  // 修改时间
    uint32_t time_scale; // 文件媒体在1秒时间内的刻度值
    uint32_t duration;   // track时间长度 = time_scale*duration;
    char language[2];       
    char pre_defined[2];
};

class Hdlr
{
public:
    Hdlr() :
        name(nullptr)
    {
    }
    ~Hdlr()
    {
        if (name)
        {
            free(name);
            name = nullptr;
        }
    }

public:
    long size;
    char box_type[4];
    char version;
    char flags[3];
    char pre_defined[4];
    char handler_type[4];  // “vide”— video track | “soun”— audio track | “hint”— hint track
    char reserved[4];  
    char* name;  // 长度不定， track type name，以‘\0’结尾的字符串 
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
    uint32_t create_time;    // 创建时间
    uint32_t modification_time;  // 修改时间
    uint32_t track_id;
    char reserved[4];
    uint32_t duration;   // track时间长度 = time_scale*duration;
    char reserved2[8];
    uint16_t layer;
    uint16_t alternate_group;  // track分组信息
    float voluem;
    char reserved3[2];
    char matrix[36];    // 视频变换矩阵
    uint32_t width;
    uint32_t height;
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
    std::list<Trak> tracks;
};

class Mdat
{
public:
    Mdat(uint32_t size) :
        max_size_(size),
        bin_data_(nullptr)
    {
        bin_data_ = (char*)malloc(size);
    }
    ~Mdat()
    {
        if (bin_data_)
        {
            free(bin_data_);
            bin_data_ = nullptr;
        }
    }

    uint32_t Append(char* data, uint32_t size)
    {
        if (!bin_data_)
            return 0;

        if (free_size_ > size)  // 能够放下
        {
            memcpy(bin_data_ + real_size_, data, size);
            real_size_ += size;
            free_size_ = max_size_ - real_size_;
        }
        else  // 扩大内存
        {
            size_t expand_size = size * 2;
            char* tmp = (char*)malloc(max_size_ + expand_size);
            if (tmp)
            {
                memcpy(tmp, bin_data_, real_size_);
                free(bin_data_);
                bin_data_ = tmp;
                max_size_ += expand_size;
                memcpy(bin_data_ + real_size_, data, size);
                real_size_ += size;
                free_size_ = max_size_ - real_size_;
            }
            else
            {
                return 0;
            }
        }

        return size;
    }

private:
    uint32_t max_size_;
    uint32_t real_size_;
    uint32_t free_size_;
    char* bin_data_;
};

class Mp4Parser
{
public:
    Mp4Parser();
    ~Mp4Parser();

    void Parser();
    uint32_t CalcSize(uint8_t size[]);
    uint32_t ReadUInt32();
    uint16_t ReadUInt16();
    uint32_t Mp4Parser::readBigEndianUnsignedInteger(void);
    uint16_t Mp4Parser::readBigEndianUnsignedShort(void);
    float Mp4Parser::readBigEndianFixedPoint(unsigned int integerLength, unsigned int fractionalLength);
    void ParserMoov(long sz);
    void ParserMvhd(Mvhd& mvhd);
    void ParserTkhd(Tkhd& tkhd);
    void ParserTrak(Trak& trak);
    void ParserMedi(Mdia& trak);
    void ParserMdhd(Mdhd& mdhd);
    void ParserHdrl(Hdlr& hdlr);
    void ParserVmhd(Vmhd& vmhd);
    void ParserMinf(Minf& trak);
    void ParserStbl(Stbl& trak);
    void ParserDinf(Dinf& dinf);
    void ParserMdat(Mdat* mdat);
    void ParserStsc(Stsc& stsc);
    void ParserStco(Stco& stco);

private:
    uint32_t file_size_;
    std::fstream file_;
    Ftyp ftyp_;
    Moov moov_;
    Mdat* mdat_;
};

