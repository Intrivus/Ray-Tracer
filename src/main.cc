#include "app.hpp"
#include <SDL3/SDL_vulkan.h>

struct Vertex {
    float x,y,z;
    float r,g,b;
};

Vertex vertices[4] = {
    {-0.5f, -0.5f, 0.0f,  1, 0, 0},
    { 0.5f, -0.5f, 0.0f,  0, 1, 0},
    { 0.5f,  0.5f, 0.0f,  0, 0, 1},
    {-0.5f,  0.5f, 0.0f,  1, 1, 0},
};

Uint16 indices[6] = { 0, 1, 2,  2, 3, 0 };

// Create a new texture, meant to be used for depth testing
static SDL_GPUTexture* create_depth_texture(SDL_GPUDevice* device, Uint32 w, Uint32 h) {
    SDL_GPUTextureCreateInfo info;
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET; // Setting this flag = used for depth testing
    info.width = w;
    info.height = h;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    return SDL_CreateGPUTexture(device, &info);
}

static SDL_GPUShader* load_shader(SDL_GPUDevice* device, const char* path, SDL_GPUShaderStage stage) {
    size_t size = 0;
    void* code = SDL_LoadFile(path, &size);
    if (!code) {
        SDL_Log("Failed to load %s: %s", path, SDL_GetError());
        return nullptr;
    }

    SDL_GPUShaderCreateInfo info = {};
    info.code = (const Uint8*)code;
    info.code_size = size;
    info.entrypoint = "main";
    info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    info.stage = stage;
    info.num_samplers = 0;
    info.num_storage_buffers = 0;
    info.num_storage_textures = 0;
    info.num_uniform_buffers = 0;

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &info);
    SDL_free(code);   // SDL copies what it needs
    if (!shader) SDL_Log("CreateGPUShader failed for %s: %s", path, SDL_GetError());
    return shader;
}

int main()
{
    double aspect_ratio = 16.0 / 9.0;
    int window_width = 800;
    
    // Temporary commenting out app to do real time renderer
    /*
    App app(aspect_ratio, window_width);
    app.init();
    app.update();
    app.shutdown();
    */

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Ray Tracer", window_width, window_width * (1 /aspect_ratio), SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return 1;
    }

    SDL_GPUDevice* device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
        true,  
        "vulkan"
    );

    if (!device) {
        SDL_Log("SDL_CreateGPUDevice failed: %s", SDL_GetError());
        return 1;
    }
    SDL_Log("GPU backend: %s", SDL_GetGPUDeviceDriver(device));

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("ClaimWindow failed: %s", SDL_GetError());
        return 1;
    }

    // Drawing triangle pipeline

    // Triangle shaders
    SDL_GPUShader* vs = load_shader(device, "src/render/shaders/triangle.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX);
    SDL_GPUShader* fs = load_shader(device, "src/render/shaders/triangle.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT);
    if (!vs || !fs) return 1;

    SDL_GPUColorTargetDescription colorDesc = {};
    colorDesc.format = SDL_GetGPUSwapchainTextureFormat(device, window);

    // Create pipeline info
    SDL_GPUGraphicsPipelineCreateInfo pi = {};
    pi.vertex_shader = vs;
    pi.fragment_shader = fs;
    pi.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pi.target_info.color_target_descriptions = &colorDesc;
    pi.target_info.num_color_targets = 1;

    // Vertex buffer
    SDL_GPUVertexBufferDescription vbDesc = {};
    vbDesc.slot = 0;
    vbDesc.pitch = sizeof(Vertex);
    vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

    SDL_GPUVertexAttribute attrs[2] = {};
    
    // Pos 0: position
    attrs[0].location = 0;
    attrs[0].buffer_slot = 0;
    attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attrs[0].offset = offsetof(Vertex, x);

    // Pos 1: color
    attrs[1].location = 1;
    attrs[1].buffer_slot = 0;
    attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attrs[1].offset = offsetof(Vertex, r);

    // Assign buffer to pipeline
    pi.vertex_input_state.vertex_buffer_descriptions = &vbDesc;
    pi.vertex_input_state.num_vertex_buffers = 1;
    pi.vertex_input_state.vertex_attributes = attrs;
    pi.vertex_input_state.num_vertex_attributes = 2;

    // Create pipeline
    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &pi);
    if (!pipeline) {
        SDL_Log("CreateGPUGraphicsPipeline failed: %s", SDL_GetError());
        return 1;
    }
    // Free unused shader object (alr registered, can be free)
    SDL_ReleaseGPUShader(device, vs);
    SDL_ReleaseGPUShader(device, fs);

    // Create buffers, upload data to buffer
    const Uint32 vSize = sizeof(vertices);
    const Uint32 iSize = sizeof(indices);

    // Vertex Buffer
    SDL_GPUBufferCreateInfo vbInfo = {};
    vbInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vbInfo.size = vSize;
    SDL_GPUBuffer* vertexBuffer = SDL_CreateGPUBuffer(device, &vbInfo);

    // Indices buffer
    SDL_GPUBufferCreateInfo ibInfo = {};
    ibInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    ibInfo.size = iSize;
    SDL_GPUBuffer* indexBuffer = SDL_CreateGPUBuffer(device, &ibInfo);

    // Buffer holding both buffer to transfer data
    SDL_GPUTransferBufferCreateInfo tbInfo = {};
    tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbInfo.size = vSize + iSize;
    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(device, &tbInfo);

    Uint8* mapped = (Uint8*)SDL_MapGPUTransferBuffer(device, transfer, false);
    memcpy(mapped, vertices, vSize);
    memcpy(mapped + vSize, indices, iSize);
    SDL_UnmapGPUTransferBuffer(device, transfer);

    // Copy to GPU
    SDL_GPUCommandBuffer* uploadCmd = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(uploadCmd);

    SDL_GPUTransferBufferLocation src = {};
    src.transfer_buffer = transfer;
    src.offset = 0;
    SDL_GPUBufferRegion dst = {};
    dst.buffer = vertexBuffer;
    dst.offset = 0;
    dst.size = vSize;
    SDL_UploadToGPUBuffer(copy, &src, &dst, false);

    src.offset = vSize;
    dst.buffer = indexBuffer;
    dst.size = iSize;
    SDL_UploadToGPUBuffer(copy, &src, &dst, false);

    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(uploadCmd);
    SDL_ReleaseGPUTransferBuffer(device, transfer);

    // Depth testing initialize
    SDL_GPUTexture* depthTex = NULL;
    Uint32 depthW = 0, depthH = 0;
    
    bool running = true;
    while(running) {

        // Inputs = PollEvent
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
        }

        // All GPU command goes into command buffer
        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
        if (!cmd) continue;

        // Swap between back and front buffer
        SDL_GPUTexture* swapchain = NULL;
        Uint32 w, h;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapchain, &w, &h)) {
            SDL_Log("Acquire swapchain failed: %s", SDL_GetError());
            SDL_SubmitGPUCommandBuffer(cmd);
            continue;
        }

        if (swapchain) {
            if (!depthTex || w != depthW || h != depthH) {
                if (depthTex) SDL_ReleaseGPUTexture(device, depthTex);
                depthTex = create_depth_texture(device, w, h);
                depthW = w;
                depthH = h;
            }

            // Background color
            SDL_GPUColorTargetInfo color = {0};
            color.texture = swapchain;
            color.clear_color = (SDL_FColor){1.0f, 1.0f, 1.0f, 1.0f};
            color.load_op = SDL_GPU_LOADOP_CLEAR;
            color.store_op = SDL_GPU_STOREOP_STORE;

            // Depth testing
            SDL_GPUDepthStencilTargetInfo depth = {0};
            depth.texture = depthTex;
            depth.clear_depth = 1.0f;
            depth.load_op = SDL_GPU_LOADOP_CLEAR;
            depth.store_op = SDL_GPU_STOREOP_DONT_CARE;
            depth.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
            depth.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;

            SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &color, 1, &depth);

            SDL_BindGPUGraphicsPipeline(pass, pipeline);

            SDL_GPUBufferBinding vbBind = {};
            vbBind.buffer = vertexBuffer;
            vbBind.offset = 0;
            SDL_BindGPUVertexBuffers(pass, 0, &vbBind, 1);

            SDL_GPUBufferBinding ibBind = {};
            ibBind.buffer = indexBuffer;
            ibBind.offset = 0;
            SDL_BindGPUIndexBuffer(pass, &ibBind, SDL_GPU_INDEXELEMENTSIZE_16BIT);

            SDL_DrawGPUIndexedPrimitives(pass, 6, 1, 0, 0, 0);

            SDL_EndGPURenderPass(pass);
        }

        SDL_SubmitGPUCommandBuffer(cmd);
    }

    SDL_WaitForGPUIdle(device);
    if (depthTex) SDL_ReleaseGPUTexture(device, depthTex);
    SDL_ReleaseGPUBuffer(device, vertexBuffer);
    SDL_ReleaseGPUBuffer(device, indexBuffer);
    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    SDL_ReleaseWindowFromGPUDevice(device, window);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}