#ifndef INPUT_HPP
#define INPUT_HPP

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <unordered_map>
#include <string>
#include <cstdint>

using std::unordered_map;

enum class Action
{
    Reload,
    Pan,
    Orbit,
    Zoom
};

class Input
{
    public:
    inline static unordered_map<SDL_Scancode, Action> key_bindings = 
    {
        {SDL_SCANCODE_R, Action::Reload},
        {SDL_SCANCODE_LSHIFT, Action::Pan}
    };
    inline static unordered_map<uint8_t, Action> mouse_bindings = 
    {
        {SDL_BUTTON_MIDDLE, Action::Orbit}
    };

    inline static unordered_map<Action, bool> pressed;
    inline static unordered_map<Action, bool> held;
    inline static unordered_map<Action, bool> released;

    static constexpr float ORBIT_SENS = 50; // What unit?
    static constexpr float PAN_SENS = 25;

    static constexpr float SCROLL_RELEASED_MS = 250;
    inline static float last_scroll = 0;
    inline static float scroll_delta = 0;
    inline static bool scrolling = false;
    inline static bool scrolling_released = false;
    
    static void check_all(SDL_Event* event);
    static void reset();
    
    private:
};

#endif