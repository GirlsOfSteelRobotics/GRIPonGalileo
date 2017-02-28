GRIPPROJ=GearLift

NT=/usr/local
#NTLIBS=-L$(NT)/lib -lntcore -lwpiutil
NTLIBS=-L$(NT)/lib -lntcore
LDLIBS=$(NTLIBS) -lstdc++ -lopencv_highgui -lopencv_imgproc -lopencv_core -lm
CPPFLAGS=-I$(GRIPPROJ) -I$(NT)/include
CXXFLAGS=-std=c++11

SRCS=GRIPonGalileo.cpp $(GRIPPROJ)/GripPipeline.cpp LifecamCapture.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: GRIPonGalileo

GRIPonGalileo: $(OBJS)

GRIPonGalileo.o: GRIPonGalileo.cpp LifecamCapture.h $(GRIPPROJ)/GripPipeline.h

LifecamCapture.o: LifecamCapture.cpp LifecamCapture.h

$(GRIPPROJ)/GripPipeline.o: $(GRIPPROJ)/GripPipeline.cpp $(GRIPPROJ)/GripPipeline.h

LifecamCaptureTest: LifecamCaptureTest.o LifecamCapture.o 

LifecamCaptureTest.o: LifecamCaptureTest.cpp LifecamCapture.h

clean:
	$(RM) $(OBJS)
