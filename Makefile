CXX = g++
CXXFLAGS = -Wall -Werror -std=c++17 -O2
SANFLAGS = -fsanitize=address -g

SRCS = main.cc
HDRS = main.h camera.h interval.h vec3.h color.h ray.h hittable.h hittable_list.h material.h sphere.h
OBJS = $(SRCS:.cc=.o)

TARGET = tracer.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cc $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

debug: CXXFLAGS += $(SANFLAGS)
debug: clean $(TARGET)

clean:
	del /f $(OBJS) $(TARGET)

.PHONY: all clean debug