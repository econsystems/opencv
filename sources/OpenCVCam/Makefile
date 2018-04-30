CC = g++
CFLAGS = -ggdb -std=c++0x 
RM = rm -f
LDFLAGS = `pkg-config --cflags --libs opencv` -lpthread -ludev

default:	
	@$(CC) $(CFLAGS) OpenCVCam.cpp -o OpenCVCam $(LDFLAGS)

all:	OpenCVCam

OpenCVCam: OpenCVCam.cpp
	@$(CC) $(CFLAGS) OpenCVCam.cpp -o OpenCVCam $(LDFLAGS)

clean veryclean:
	@$(RM) OpenCVCam
