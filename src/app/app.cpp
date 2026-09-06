#include "app.hpp"

App::App(double aspect_ratio, int window_width)
{
    this->aspect_ratio = aspect_ratio;
    this->window_width = window_width;
    window_height = int(window_width / aspect_ratio);
    window_height = (window_height < 1) ? 1 : window_height;
}

void App::scene_setup()
{
    // World
    // auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    // world.add(make_unique<sphere>(point3(0, -1000, 0), 1000, ground_material));

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

    // Camera
    cam.aspect_ratio = aspect_ratio;
    cam.image_width = window_width;
    // cam.samples_per_pixel = 50;
    // cam.max_depth = 10;

    cam.vfov = 20;
    cam.lookat = point3(0, 0, 0);
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0.6;
    cam.focus_dist = 10.0;
}

void App::init()
{
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Window", window_width, window_height, SDL_WINDOW_RESIZABLE);
    if (window == nullptr)
    {
        std::cerr << "Window could not be created\n";
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    SDL_SetWindowAspectRatio(window, static_cast<float>(aspect_ratio), static_cast<float>(aspect_ratio));

    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == nullptr)
    {
        std::cerr << "Cannot create renderer\n";
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    ui_context = ImGui::CreateContext();
    if (ui_context == nullptr)
    {
        std::cerr << "Cannot create GUI context\n";
        ImGui::DestroyContext(ui_context);
        exit(EXIT_FAILURE);
    }
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
    if (texture == nullptr)
    {
        std::cerr << "Cannot create texture\n";
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    scene_setup();
}

void App::update()
{
    render_scene();

    while (!finished)
    {
        SDL_Event event;
        handle_event(event);

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        draw_ui();
        
        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 0, 60, 60, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

        SDL_RenderPresent(renderer);
    }
}

void App::handle_event(SDL_Event event)
{
    Input::reset();
    Input::update();

    // Event Handling
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT) finished = true;
        Input::check_all(&event);
    }

    // Input Handling
    if (Input::pressed[Action::Reload]) render_scene();

    if (Input::held[Action::Orbit])
    {
        float mouse_dx = ImGui::GetIO().MouseDelta.x / window_width;
        float mouse_dy = ImGui::GetIO().MouseDelta.y / window_height;
        if (Input::held[Action::Pan])
        {
            cam.lookat += (-mouse_dx * cam.u + mouse_dy * cam.v) * cam.orbit_radius * Input::PAN_SENS;
            render_scene(true);
        }
        else if (Input::released[Action::Pan])
        {
            render_scene(false);
        }
        else
        {
            cam.horizontal_angle += mouse_dx * Input::ORBIT_SENS;
            cam.vertical_angle -= mouse_dy * Input::ORBIT_SENS;
            render_scene(true);
        }
    }
    else if (Input::released[Action::Orbit]) render_scene(false);

    if (Input::scroll_delta != 0)
    {
        double new_radius = cam.orbit_radius - Input::scroll_delta * Input::ZOOM_SENS;
        cam.orbit_radius = cam.proximity.clamp(new_radius);
        render_scene(true);
    }
    else if (Input::scrolling_released)
    {
        render_scene(false);
    }
}

void App::draw_ui()
{
    float pos[3] = {(float)cam.lookat.x(), (float)cam.lookat.y(), (float)cam.lookat.z()};

    float orbit[3] = {
        (float)cam.vertical_angle,
        (float)cam.horizontal_angle,
        (float)cam.orbit_radius};

    float focus_distance = (float)cam.focus_dist;

    ImGui::Begin("Control");

    if (ImGui::SliderFloat3("Position", pos, -20.0, 20.0))
    {
        cam.lookat = vec3(pos[0], pos[1], pos[2]);
    }
    if (ImGui::IsItemActive())
    {
        render_scene(true);
    }
    else if (ImGui::IsItemDeactivatedAfterEdit())
    {
        render_scene(false);
    }

    if (ImGui::SliderFloat3("Orbit", orbit, -180.0, 360.0))
    {
        cam.vertical_angle = orbit[0];
        cam.horizontal_angle = orbit[1];
        cam.orbit_radius = orbit[2];
    }
    if (ImGui::IsItemActive())
    {
        render_scene(true);
    }
    else if (ImGui::IsItemDeactivatedAfterEdit())
    {
        render_scene(false);
    }

    if (ImGui::SliderFloat("Focus Distance", &focus_distance, 0.0, 100.0))
    {
        cam.focus_dist = focus_distance;
    }
    if (ImGui::IsItemActive())
    {
        render_scene(true);
    }
    else if (ImGui::IsItemDeactivatedAfterEdit())
    {
        render_scene(false);
    }
    
    ImGui::End();
}

void App::render_scene(bool preview)
{
    if (preview)
    {
        cam.samples_per_pixel = 1;
        cam.max_depth = 5;
    }
    else 
    {
        cam.samples_per_pixel = 50;
        cam.max_depth = 10;
    }

    void *pixels;
    int pitch = 0;
    if (SDL_LockTexture(texture, NULL, &pixels, &pitch))
    {
        uint32_t *pixel_buffer = static_cast<uint32_t *>(pixels);
        cam.render(world, pixel_buffer, pitch);
        SDL_UnlockTexture(texture);
    }
}

void App::shutdown()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext(ui_context);
}