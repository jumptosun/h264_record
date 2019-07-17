#ifndef _H264_FILE_H_
#define _H264_FILE_H_

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct H264File {
    int fd;

    uint8_t* fileBuf_;
    int fileSize_;

    uint8_t *pos, *pend;

    H264File()
        : fd(-1)
        , fileBuf_(NULL)
        , fileSize_(-1)
    {
    }
    ~H264File() {}

    int open(char* filename)
    {
        if ((fd = ::open(filename, O_RDONLY)) == -1) {
            fprintf(stderr, "open file=%s faild, err=%s\n", filename,
                strerror(errno));
            return -1;
        }

        fileSize_ = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        fileBuf_ = (uint8_t*)malloc(fileSize_);
        int nread = 0, ret;

        while (nread < fileSize_) {
            ret = read(fd, fileBuf_ + nread, fileSize_ - nread);

            if (ret == -1) {
                fprintf(stderr, "read file failed, err=%s\n", strerror(errno));
                close(fd);
                return -1;
            }

            nread += ret;
        }

        pos = fileBuf_;
        pend = pos + fileSize_;

        return 0;
    }

    int read_nalu(uint8_t*& buf, int& count)
    {
        uint32_t prefix;
        uint8_t* last_pos = pos;

        if (pos >= (pend - 3)) {
            return -1;
        }

        // find prefix
        for (pos += 3; pos <= (pend - 3); pos++) {
            prefix = *((uint32_t*)pos);

            if ((prefix & 0x00FFFFFF) == 0x010000) {
                break;
            }

            if (prefix == 0x01000000) {
                break;
            }
        }

        if (pos >= (pend - 3)) {
            pos = pend;
        }

        buf = last_pos;
        count = pos - last_pos;

        return count;
    }

    int read_nalu5(uint8_t*& buf, int& count)
    {
        uint8_t* nalu;
        int nalu_len;

        buf = NULL;
        count = 0;

        if (pos >= (pend - 3)) {
            return -1;
        }

        for (int i = 0; i < 5; i++) {
            if (read_nalu(nalu, nalu_len) < 0)
                break;

            if (buf == NULL)
                buf = nalu;

            count += nalu_len;
        }

        return 0;
    }

    void reset()
    {
        pos = fileBuf_;
    }
};

#endif