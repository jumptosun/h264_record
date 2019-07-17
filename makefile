CC=clang
CXX=clang++
CXXFLAGS=-g -fPIC -Wall
LD=clang++

.PHONY: all clean format

all: format lib264record.so

avc2mp4: avc2mp4.cpp lib264record.so
	$(CXX) -Wall -g avc2mp4.cpp -Wl,-rpath="./" -L./ -l264record -lavformat -lavcodec -lavutil -pthread -o avc2mp4

lib264record.so: h264_record.o h264_reader.o utils.o sps_parser.o
	$(LD) -shared -lavformat -lavcodec -lavutil -pthread -o lib264record.so h264_record.o h264_reader.o utils.o sps_parser.o

h264_record.o: h264_record.cpp  bit_stream.hpp byte_stream.hpp

h264_reader.o: h264_reader.cpp

utils.o : utils.cpp

sps_parser.o: sps_parser.cpp

format:
	./format.sh

clean:
	rm avc2mp4 *.o *.so

