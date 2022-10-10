CC = g++
CFLAGS = -ggdb -std=c++0x 
RM = rm -f
LDFLAGS = `pkg-config --cflags --libs opencv4` -lpthread -ludev -lopencv_world
default:	
	@$(CC) $(CFLAGS) OpenCVCam.cpp -o OpenCVCam $(LDFLAGS)

all:	OpenCVCam

OpenCVCam: OpenCVCam1.cpp
	@$(CC) $(CFLAGS) OpenCVCam.cpp -o OpenCVCam $(LDFLAGS)

clean veryclean:
	@$(RM) OpenCVCam
