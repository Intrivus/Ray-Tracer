CXX      := g++
CXXFLAGS := -Wall -Werror -std=c++17 -O2
SANFLAGS := -fsanitize=address -g

SDL_DIR   := S:/Repertoire/_Global/SDK/SDL3/x86_64-w64-mingw32
IMGUI_DIR := third_party/imgui
BUILD_DIR := build
SRC_DIR   := src

INCLUDES  := -I$(SRC_DIR) \
             -I$(SRC_DIR)/app \
             -I$(SRC_DIR)/math \
             -I$(SRC_DIR)/scene \
             -I$(SRC_DIR)/render \
             -I$(SDL_DIR)/include \
             -I$(IMGUI_DIR) \
             -I$(IMGUI_DIR)/backends

LIBS      := -L"$(SDL_DIR)/lib" -lSDL3

# APP_SRCS  := $(SRC_DIR)/main.cc \
#              $(SRC_DIR)/app/app.cpp

IMGUI_SRCS := $(IMGUI_DIR)/imgui.cpp \
              $(IMGUI_DIR)/imgui_draw.cpp \
              $(IMGUI_DIR)/imgui_tables.cpp \
              $(IMGUI_DIR)/imgui_widgets.cpp \
              $(IMGUI_DIR)/imgui_demo.cpp

IMGUI_BACKENDS := $(IMGUI_DIR)/backends/imgui_impl_sdl3.cpp \
                  $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer3.cpp

OBJS := $(BUILD_DIR)/src/main.o \
        $(BUILD_DIR)/src/app/app.o \
        $(IMGUI_SRCS:%.cpp=$(BUILD_DIR)/%.o) \
        $(IMGUI_BACKENDS:%.cpp=$(BUILD_DIR)/%.o)

DEPS := $(OBJS:.o=.d)

TARGET := tracer.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(BUILD_DIR)/%.o: %.cc
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

-include $(DEPS)

debug:
	@$(MAKE) clean
	@$(MAKE) $(TARGET) CXXFLAGS="$(CXXFLAGS) $(SANFLAGS)"

clean:
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	@if exist "$(TARGET)" del /f /q "$(TARGET)"

.PHONY: all clean debug