SDL_DIR = S:/Repertoire/_Global/SDK/SDL3/x86_64-w64-mingw32

CXX = g++
CXXFLAGS = -Wall -Werror -std=c++17 -O2 -Isrc/math -Isrc/scene -Isrc/render -Isrc -I$(SDL_DIR)/include
LDFLAGS = -L"$(SDL_DIR)/lib" -lSDL3
SANFLAGS = -fsanitize=address -g

SRCS = src/main.cc
HDRS = src/render/main.h src/render/camera.h src/math/interval.h src/math/vec3.h src/math/color.h src/math/ray.h src/scene/hittable.h src/scene/hittable_list.h src/scene/material.h src/scene/sphere.h
OBJS = $(SRCS:.cc=.o)

TARGET = tracer.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cc $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

debug: CXXFLAGS += $(SANFLAGS)
debug: clean $(TARGET)

clean:
	del /f $(subst /,\,$(OBJS) $(TARGET))

.PHONY: all clean debug