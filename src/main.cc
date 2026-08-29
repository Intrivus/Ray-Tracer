#include "main.h"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

#include <SDL3/SDL.h>

int main()
{
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_unique<sphere>(point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
            
            if ((center - point3(4, 0.2, 0)).length() > 0.9)
            {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8)
                {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_unique<sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95)
                {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_unique<sphere>(center, 0.2, sphere_material));
                }
                else
                {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_unique<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }
    
    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_unique<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_unique<sphere>(point3(-4, 1, 0), 1.0, material2));
    
    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_unique<sphere>(point3(4, 1, 0), 1.0, material3));
    
    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 400;
    cam.samples_per_pixel = 50;
    cam.max_depth = 10;
    
    cam.vfov = 20;
    cam.lookfrom = point3(13, 2, 3);
    cam.lookat = point3(0, 0, 0);
    cam.vup = vec3(0, 1, 0);
    
    cam.defocus_angle = 0.6;
    cam.focus_dist = 10.0;



    int image_height = (int)cam.image_width / cam.aspect_ratio;
    image_height = (image_height < 1) ? 1 : image_height;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Window", cam.image_width, image_height, 0);
    if (window == NULL)
    {
        std::cerr << "Window could not be created\n";
        SDL_Quit();
        return -1;
    }
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == nullptr)
    {
        std::cerr << "Cannot create renderer\n";
        SDL_Quit();
        return -1;
    }
    
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, cam.image_width, image_height);
    if (texture == nullptr)
    {
        std::cerr << "Cannot create texture\n";
        SDL_Quit();
        return -1;
    }

    bool done = false;

    
    void * pixels;
    int pitch = 0;
    if (SDL_LockTexture(texture, NULL, &pixels, &pitch))
    {
        uint32_t* pixel_buffer = static_cast<uint32_t*>(pixels);

        cam.render(world, pixel_buffer, pitch);

        SDL_UnlockTexture(texture);
    }
    

    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                done = true;
        }
        SDL_SetRenderDrawColor(renderer, 0, 60, 60, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}