#include <nusys.h>
#include <string.h>
#include <malloc.h>
#include "file.h"
#include "hash.h"

#define READ_BUF_SIZE 1024
#define LZ_WINDOW_SIZE 1024
#define LZ_WINDOW_START 958
#define LZ_MIN_MATCH_LEN 3

struct decode {
	u16 read_pos;
	u32 src;
	char *dst;
	u32 len;
};

typedef struct file_entry {
    u32 name_hash;
    u32 offset;
    u32 size;
    u32 compress_flag;
} FileEntry;

extern u8 _file_packSegmentRomStart[];

static u32 num_files __attribute__((aligned(8)));
static u32 file_pack_base;
static FileEntry *file_entries;

void FilePackInit()
{
	int i;
	file_pack_base = (u32)_file_packSegmentRomStart;
	nuPiReadRom(file_pack_base, &num_files, sizeof(u32));
	file_entries = malloc(num_files*sizeof(FileEntry));
	nuPiReadRom(file_pack_base+sizeof(u32), file_entries, num_files*sizeof(FileEntry));
}

static int GetHashFileIdx(u32 hash)
{
	int i;
	for(i=0; i<num_files; i++) {
		if(file_entries[i].name_hash == hash) {
			return i;
		}
	}
	return -1;
}

static void DecodeRaw(struct decode *decode)
{
	u32 read_size;
	while(decode->len) {
		if(decode->len < READ_BUF_SIZE) {
			read_size = (decode->len+1)&0xFFFFFFFE;
			decode->len = 0;
		} else {
			read_size = READ_BUF_SIZE;
			decode->len -= READ_BUF_SIZE;
		}
		nuPiReadRom(decode->src, decode->dst, read_size);
		decode->src += read_size;
		decode->dst += read_size;
	}
}

static u8 GetLZByte(struct decode *decode)
{
	static u8 read_buf[READ_BUF_SIZE];
	if(decode->read_pos == READ_BUF_SIZE) {
		nuPiReadRom(decode->src, &read_buf, READ_BUF_SIZE);
		decode->src += READ_BUF_SIZE;
		decode->read_pos = 0;
	}
	return read_buf[decode->read_pos++];
}

static void DecodeLZ(struct decode *decode)
{
	char window[LZ_WINDOW_SIZE];
    int window_ofs = LZ_WINDOW_START;
    u16 flag = 0;
    memset(window, 0, LZ_WINDOW_SIZE);
    while (decode->len) {
		flag >>= 1;
        if (!(flag & 0x100)) {
            flag = 0xFF00 | GetLZByte(decode);
        }
        if (flag & 0x1) {
            window[window_ofs++] = *decode->dst++ = GetLZByte(decode);
            window_ofs %= LZ_WINDOW_SIZE;
            decode->len--;
        } else {
			int i;
            u8 byte1 = GetLZByte(decode);
            u8 byte2 = GetLZByte(decode);
            int ofs = ((byte2 & 0xC0) << 2) | byte1;
            int copy_len = (byte2 & 0x3F) + LZ_MIN_MATCH_LEN;
            for (i = 0; i < copy_len; i++) {
                window[window_ofs++] = *decode->dst++ = window[(ofs+i)%LZ_WINDOW_SIZE];
                window_ofs %= LZ_WINDOW_SIZE;
            }
			decode->len -= i;
        }
    }
}

void *FileRead(const char *name)
{
	int file_idx;
	if(name[0] == '\\') {
		name++;
	}
	file_idx = GetHashFileIdx(HashGetPath(name));
	if(file_idx != -1) {
		struct decode decode;
		int raw_len = (file_entries[file_idx].size+1) & 0xFFFFFFFE;
		void *dst = malloc(raw_len);
		decode.read_pos = READ_BUF_SIZE;
		decode.src = file_pack_base+file_entries[file_idx].offset;
		decode.dst = dst;
		decode.len = raw_len;
		if(file_entries[file_idx].compress_flag) {
			DecodeLZ(&decode);
		} else {
			DecodeRaw(&decode);
		}
		return dst;
	} else {
		return NULL;
	}
}

void FileFree(void *ptr)
{
	if(ptr) {
		free(ptr);
	}
}