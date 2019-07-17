#include <stdio.h>

#include "h264_file.hpp"
#include "h264_record.hpp"

void usage()
{
    printf("usage:\n"
           "av2mp4 input.264 output.format_you_want\n");
}

int main(int argc, char** argv)
{
    int ret;
    uint8_t* nalu;
    int nalu_len;

    H264File reader;
    H264Record recorder;

    if (argc < 3) {
        fprintf(stderr, "not enough args\n");

        usage();
        exit(0);
    }

    if (reader.open(argv[1]) == -1) {
        fprintf(stderr, "open h264 failed\n");

        usage();
        exit(0);
    }

    recorder.init(argv[2]);

    while ((ret = reader.read_nalu5(nalu, nalu_len)) != -1) {
        // fprintf(stdout, "frame: ");
        // for(int i = 0; i < 30 && i < nalu_len; i++){
        //     fprintf(stdout, "%d ", nalu[i]);
        // }
        // fprintf(stdout, "\n");

        recorder.write_data(nalu, nalu_len);
    }

    recorder.uninit();

    return 0;
}
