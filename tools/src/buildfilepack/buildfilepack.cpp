#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <dirent.h>
#include <ctype.h>

#define N 1024   /* size of ring buffer */   
#define F 66   /* upper limit for match_length */   
#define THRESHOLD 2 /* encode string into position and length  if match_length is greater than this */
#define NIL  N /* index for root of binary search trees */   

uint8_t text_buf[N + F - 1];    /* ring buffer of size N,
        with extra F-1 bytes to facilitate string comparison */
int match_position, match_length,  /* of longest match.  These are
                        set by the InsertNode() procedure. */
    lson[N + 1], rson[N + 257], dad[N + 1];  /* left & right children &
            parents -- These constitute binary search trees. */

void InitTree(void)  /* initialize trees */
{
    int  i;

    /* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
       left children of node i.  These nodes need not be initialized.
       Also, dad[i] is the parent of node i.  These are initialized to
       NIL (= N), which stands for 'not used.'
       For i = 0 to 255, rson[N + i + 1] is the root of the tree
       for strings that begin with character i.  These are initialized
       to NIL.  Note there are 256 trees. */

    for (i = N + 1; i <= N + 256; i++) rson[i] = NIL;
    for (i = 0; i < N; i++) dad[i] = NIL;
}

void InsertNode(int r)
/* Inserts string of length F, text_buf[r..r+F-1], into one of the
   trees (text_buf[r]'th tree) and returns the longest-match position
   and length via the global variables match_position and match_length.
   If match_length = F, then removes the old node in favor of the new
   one, because the old one will be deleted sooner.
   Note r plays double role, as tree node and position in buffer. */
{
    int  i, p, cmp;
    uint8_t *key;

    cmp = 1;  key = &text_buf[r];  p = N + 1 + key[0];
    rson[r] = lson[r] = NIL;  match_length = 0;
    for (; ; ) {
        if (cmp >= 0) {
            if (rson[p] != NIL) p = rson[p];
            else { rson[p] = r;  dad[r] = p;  return; }
        }
        else {
            if (lson[p] != NIL) p = lson[p];
            else { lson[p] = r;  dad[r] = p;  return; }
        }
        for (i = 1; i < F; i++)
            if ((cmp = key[i] - text_buf[p + i]) != 0)  break;
        if (i > match_length) {
            match_position = p;
            if ((match_length = i) >= F)  break;
        }
    }
    dad[r] = dad[p];  lson[r] = lson[p];  rson[r] = rson[p];
    dad[lson[p]] = r;  dad[rson[p]] = r;
    if (rson[dad[p]] == p) rson[dad[p]] = r;
    else                   lson[dad[p]] = r;
    dad[p] = NIL;  /* remove p */
}

void DeleteNode(int p)  /* deletes node p from tree */
{
    int  q;

    if (dad[p] == NIL) return;  /* not in tree */
    if (rson[p] == NIL) q = lson[p];
    else if (lson[p] == NIL) q = rson[p];
    else {
        q = lson[p];
        if (rson[q] != NIL) {
            do { q = rson[q]; } while (rson[q] != NIL);
            rson[dad[q]] = lson[q];  dad[lson[q]] = dad[q];
            lson[q] = lson[p];  dad[lson[p]] = q;
        }
        rson[q] = rson[p];  dad[rson[p]] = q;
    }
    dad[q] = dad[p];
    if (rson[dad[p]] == p) rson[dad[p]] = q;  else lson[dad[p]] = q;
    dad[p] = NIL;
}

uint32_t CompressData(FILE *dst_file, FILE *src_file)
{
    int  i, c, len, r, s, last_match_length, code_buf_ptr;
    uint8_t code_buf[17], mask;
    uint32_t codesize = 0;

    InitTree();  /* initialize trees */
    code_buf[0] = 0;  /* code_buf[1..16] saves eight units of code, and
            code_buf[0] works as eight flags, "1" representing that the unit
            is an unencoded letter (1 byte), "0" a position-and-length pair
            (2 bytes).  Thus, eight units require at most 16 bytes of code. */
    code_buf_ptr = mask = 1;
    s = 0;  r = N - F;
    for (i = s; i < r; i++) text_buf[i] = '\0';  /* Clear the buffer with
            any character that will appear often. */
    for (len = 0; len < F && (c = getc(src_file)) != EOF; len++)
        text_buf[r + len] = c;  /* Read F bytes into the last F bytes of
                the buffer */
    if (len == 0) return 0;  /* text of size zero */
    for (i = 1; i <= F; i++) InsertNode(r - i);  /* Insert the F strings,
            each of which begins with one or more 'space' characters.  Note
            the order in which these strings are inserted.  This way,
            degenerate trees will be less likely to occur. */
    InsertNode(r);  /* Finally, insert the whole string just read.  The
            global variables match_length and match_position are set. */
    do {
        if (match_length > len) match_length = len;  /* match_length
                may be spuriously long near the end of text. */
        if (match_length <= THRESHOLD) {
            match_length = 1;  /* Not long enough match.  Send one byte. */
            code_buf[0] |= mask;  /* 'send one byte' flag */
            code_buf[code_buf_ptr++] = text_buf[r];  /* Send uncoded. */
        }
        else {
            code_buf[code_buf_ptr++] = (uint8_t)match_position;
            code_buf[code_buf_ptr++] = (uint8_t)
                (((match_position >> 2) & 0xC0)
                    | (match_length - (THRESHOLD + 1)));  /* Send position and
                                  length pair. Note match_length > THRESHOLD. */
        }
        if ((mask <<= 1) == 0) {  /* Shift mask left one bit. */
            for (i = 0; i < code_buf_ptr; i++)  /* Send at most 8 units of */
                putc(code_buf[i], dst_file);     /* code together */
            codesize += code_buf_ptr;
            code_buf[0] = 0;  code_buf_ptr = mask = 1;
        }
        last_match_length = match_length;
        for (i = 0; i < last_match_length &&
            (c = getc(src_file)) != EOF; i++) {
            DeleteNode(s);          /* Delete old strings and */
            text_buf[s] = c;        /* read new bytes */
            if (s < F - 1) text_buf[s + N] = c;  /* If the position is
                    near the end of buffer, extend the buffer to make
                    string comparison easier. */
            s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
            /* Since this is a ring buffer, increment the position
               modulo N. */
            InsertNode(r);  /* Register the string in text_buf[r..r+F-1] */
        }
        while (i++ < last_match_length) {       /* After the end of text, */
            DeleteNode(s);                                  /* no need to read, but */
            s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
            if (--len) InsertNode(r);               /* buffer may not be empty. */
        }
    } while (len > 0);      /* until length of string to be processed is zero */
    if (code_buf_ptr > 1) {         /* Send remaining code. */
        for (i = 0; i < code_buf_ptr; i++) putc(code_buf[i], dst_file);
        codesize += code_buf_ptr;
    }
    return codesize;
}

struct FileEntry {
    uint32_t name_hash;
    uint32_t offset;
    uint32_t size;
    uint32_t compress_flag;
};

std::vector<std::string> filenames;

void WriteDirFilenames(std::string base_dir, std::string dir_name)
{
    struct dirent *dir_entry;
    DIR *dir_ptr;
    std::string dir = base_dir + "\\" + dir_name + "\\";
    dir_ptr = opendir(dir.c_str());
    if (!dir_ptr) {
        printf("Failed to Open Directory %s.\n", dir.c_str());
        exit(1);
    }
    while (dir_entry = readdir(dir_ptr)) {
        std::string name = dir_entry->d_name;
        if (name != "." && name != "..") {
            if (dir_entry->d_type != DT_DIR) {
                std::string file_name;
                if (dir_name != "") {
                    file_name = dir_name + "\\" + name;
                } else {
                    file_name = name;
                }
                filenames.push_back(file_name);
            } else {
                WriteDirFilenames(base_dir, dir_name + name);
            }
        }
    }
    closedir(dir_ptr);
}

void WriteU32(FILE *file, uint32_t value)
{
    uint32_t temp = (value & 0x000000FF) << 24;
    temp |= (value & 0x0000FF00) << 8;
    temp |= (value & 0x00FF0000) >> 8;
    temp |= (value & 0xFF000000) >> 24;
    fwrite(&temp, 4, 1, file);
}

uint32_t GetHash(std::string string)
{
    uint32_t hash = 16777619;
    for (size_t i = 0; i < string.length(); i++) {
        if (string.at(i) == '\\') {
			hash ^= '/';
		} else {
			hash ^= string.at(i);
		}
        hash *= 2166136261;
    }
    return hash;
}

void WriteFilesystem(std::string base_dir, std::string out_name)
{
    uint32_t num_files = filenames.size();
    FILE *out_fs = fopen(out_name.c_str(), "wb");
    if (!out_fs) {
        printf("Failed to Open %s for Writing.\n", out_name.c_str());
        exit(1);
    }
    FileEntry *file_entries = new FileEntry[num_files]();
    WriteU32(out_fs, num_files);
    fwrite(file_entries, 1, sizeof(FileEntry) * num_files, out_fs);
    uint32_t file_ofs = ftell(out_fs);
    for (int i = 0; i < num_files; i++) {
        std::string file_name = base_dir + "\\" + filenames[i];
        FILE *file;
        FILE *temp_file;
        uint32_t packed_size;
        file_entries[i].name_hash = GetHash(filenames[i]);
        file_entries[i].offset = file_ofs;
        file = fopen(file_name.c_str(), "rb");
        if (!file) {
            printf("Failed to Open %s for Reading.\n", file_name.c_str());
            exit(1);
        }
        fseek(file, 0, SEEK_END);
        file_entries[i].size = ftell(file);
        fseek(file, 0, SEEK_SET);
        temp_file = tmpfile();
        if (!temp_file) {
            printf("Failed to Open Temporary File.\n");
            exit(1);
        }
        packed_size = CompressData(temp_file, file);
        fseek(temp_file, 0, SEEK_SET);
        fseek(file, 0, SEEK_SET);
        if (packed_size < file_entries[i].size) {
            file_entries[i].compress_flag = 1;
            uint8_t *file_buf = new uint8_t[packed_size];
            fread(file_buf, 1, packed_size, temp_file);
            fwrite(file_buf, 1, packed_size, out_fs);
            file_ofs += (packed_size + 1) & 0xFFFFFFFE;
            fclose(temp_file);
            fclose(file);
            delete[] file_buf;
        } else {
            file_entries[i].compress_flag = 0;
            uint8_t *file_buf = new uint8_t[file_entries[i].size];
            fread(file_buf, 1, file_entries[i].size, file);
            fwrite(file_buf, 1, file_entries[i].size, out_fs);
            file_ofs += (file_entries[i].size + 1) & 0xFFFFFFFE;
            fclose(temp_file);
            fclose(file);
            delete[] file_buf;
        }
        if (ftell(out_fs) % 2) {
            uint8_t zero = 0;
            fwrite(&zero, 1, 1, out_fs);
        }
    }
    fseek(out_fs, 4, SEEK_SET);
    for (int i = 0; i < num_files; i++) {
        WriteU32(out_fs, file_entries[i].name_hash);
        WriteU32(out_fs, file_entries[i].offset);
        WriteU32(out_fs, file_entries[i].size);
        WriteU32(out_fs, file_entries[i].compress_flag);
    }
    fclose(out_fs);
    delete[] file_entries;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <dir> <out>\n", argv[0]);
        return 1;
    }
    WriteDirFilenames(argv[1], "");
    WriteFilesystem(argv[1], argv[2]);
    return 0;
}