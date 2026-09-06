#include "input.hpp"

void Input::check_all(SDL_Event* event)
{
    // Check Key Inputs
    if (event->type == SDL_EVENT_KEY_DOWN && event->key.repeat == 0)
    {
        SDL_Scancode code = event->key.scancode;

        if (key_bindings.count(code))
        {
            Action action = key_bindings[code];
            if (!held[action]) pressed[action] = true;
            held[action] = true;
        }
    }
    else if (event->type == SDL_EVENT_KEY_UP)
    {
        SDL_Scancode code = event->key.scancode;

        if (key_bindings.count(code))
        {
            Action action = key_bindings[code];
            held[action] = false;
            released[action] = true;
        }
    }

    // Check Mouse Input
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        uint8_t code = event->button.button;
        
        if (mouse_bindings.count(code))
        {
            Action action = mouse_bindings[code];
            if (!held[action]) pressed[action] = true;
            held[action] = true;
        }
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        uint8_t code = event->button.button;
        
        if (mouse_bindings.count(code))
        {
            Action action = mouse_bindings[code];
            held[action] = false;
            released[action] = true;
        }
    }

    // Check Scroll Input
    if (event->type == SDL_EVENT_MOUSE_WHEEL)
    {
        scrolling = true;
        scroll_delta = event->wheel.y;
        last_scroll = SDL_GetTicks();
    }
}

void Input::update()
{
    if (scrolling && SDL_GetTicks() - last_scroll > SCROLL_RELEASED_MS)
    {
        scrolling = false;
        scrolling_released = true;
    }
}

void Input::reset()
{
    pressed.clear();
    released.clear();

    scrolling_released = false;
    scroll_delta = 0;
}