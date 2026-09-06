#ifndef APP_HPP
#define APP_HPP

#include "input.hpp"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "utils.h"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

class App
{
    public:
    inline static double aspect_ratio = 1.0;
    inline static int window_width = 100;
    inline static int window_height = 100;
    inline static bool finished = false;

    App(double aspect_ratio, int window_width);
    
    void init();
    void update();
    void shutdown();
    
    private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    
    ImGuiContext* ui_context;
    hittable_list world;
    camera cam;

    void draw_ui();
    void handle_event(SDL_Event event);
    void scene_setup();
    void render_scene(bool preview = false);
};

#endif