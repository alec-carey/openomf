#include "video/video.h"
#include "video/image.h"
#include "video/tcache.h"
#include "utils/log.h"
#include "utils/list.h"
#include "resources/palette.h"
#include <SDL2/SDL.h>
#include <stdlib.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int w;
    int h;
    int fs;
    int vsync;

    // Palettes
    palette *base_palette;
    screen_palette *cur_palette;
    SDL_Texture *target;
} video_state;

static video_state state;

int video_init(int window_w, int window_h, int fullscreen, int vsync) {
    state.w = window_w;
    state.h = window_h;
    state.fs = fullscreen;
    state.vsync = vsync;

    // Clear palettes
    state.cur_palette = malloc(sizeof(screen_palette));
    state.base_palette = malloc(sizeof(palette));
    state.cur_palette->version = 1;
    memset(state.cur_palette->data, 0, 768);

    // Open window
    state.window = SDL_CreateWindow(
        "OpenOMF",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_w,
        window_h,
        SDL_WINDOW_SHOWN);
    if(state.window == NULL) {
        PERROR("Could not create window: %s", SDL_GetError());
        return 1;
    }

    // Form flags
    int renderer_flags = SDL_RENDERER_ACCELERATED;
    if(state.vsync) {
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
    }

    // Create renderer
    state.renderer = SDL_CreateRenderer(
        state.window, 
        -1, 
        renderer_flags);
    if(state.renderer == NULL) {
        PERROR("Could not create renderer: %s", SDL_GetError());
        return 1;
    }

    // Default resolution for renderer. This will them get scaled up to screen size.
    SDL_RenderSetLogicalSize(state.renderer, NATIVE_W, NATIVE_H);

    // Disable screensaver :/
    SDL_DisableScreenSaver();
    
    // Set fullscreen if needed
    if(state.fs) {
        if(SDL_SetWindowFullscreen(state.window, 1) != 0) {
            PERROR("Could not set fullscreen mode!");
        } else {
            DEBUG("Fullscreen enabled!");
        }
    }

    // Set up surfaces etc.
    state.target = SDL_CreateTexture(
        state.renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_TARGET,
        NATIVE_W, NATIVE_H);

    // Get renderer data
    SDL_RendererInfo rinfo;
    SDL_GetRendererInfo(state.renderer, &rinfo);

    // Show some info
    INFO("Video Init OK");
    INFO(" * Driver: %s", SDL_GetCurrentVideoDriver());
    INFO(" * Renderer: %s", rinfo.name);
    INFO(" * Accelerated: %s", (rinfo.flags & SDL_RENDERER_ACCELERATED) ? "Yes" : "No");
    INFO(" * VSync: %s", (rinfo.flags & SDL_RENDERER_PRESENTVSYNC) ? "Yes" : "No");
    return 0;
}

int video_reinit(int window_w, int window_h, int fullscreen, int vsync) {
    // Set fullscreen if necessary
    if(fullscreen != state.fs) {
        if(SDL_SetWindowFullscreen(state.window, fullscreen ? SDL_TRUE : SDL_FALSE) != 0) {
            PERROR("Could not set fullscreen mode!");
        } else {
            DEBUG("Fullscreen changed!");
        }
    }

    // Set window size if necessary
    if(window_w != state.w || window_h != state.h || fullscreen != state.fs) {
        SDL_SetWindowSize(state.window, window_w, window_h);
    }
    
    // VSync change; reset renderer.
    if(vsync != state.vsync) {
        int renderer_flags = SDL_RENDERER_ACCELERATED;
        if(vsync) {
            renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
        }
        SDL_DestroyRenderer(state.renderer);
        tcache_clear(); // Clear cache, because renderer changed.
        state.renderer = SDL_CreateRenderer(state.window, -1, renderer_flags);
        SDL_RenderSetLogicalSize(state.renderer, NATIVE_W, NATIVE_H);
    }

    // Set video state
    state.vsync = vsync;
    state.fs = fullscreen;
    state.w = window_w;
    state.h = window_h;
    return 0;
}

void video_screenshot(image *img) {
    image_create(img, state.w, state.h);
    int ret = SDL_RenderReadPixels(state.renderer, NULL, SDL_PIXELFORMAT_ABGR8888, img->data, img->w * 4);
    if(ret != 0) {
        PERROR("Unable to read pixels from rendertarget: %s", SDL_GetError());
    }
}

void video_force_pal_refresh() {
    memcpy(state.cur_palette->data, state.base_palette->data, 768);
    state.cur_palette->version++;
}

void video_set_base_palette(const palette *src) {
    memcpy(state.base_palette, src, sizeof(palette));
    memcpy(state.cur_palette->data, state.base_palette->data, 768);
    state.cur_palette->version++;
}

palette *video_get_base_palette() {
    return state.base_palette;
}

void video_copy_pal_range(const palette *src, int src_start, int dst_start, int amount) {
    memcpy(state.cur_palette->data + dst_start * 3, 
           src->data + src_start * 3, 
           amount * 3);
    state.cur_palette->version++;
}

screen_palette* video_get_pal_ref() {
    return state.cur_palette;
}

void video_render_prepare() {
    // Set default render target, and clear it up as transparent
    SDL_SetRenderTarget(state.renderer, state.target);
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0);
    SDL_RenderClear(state.renderer);

    // Reset palette
    memcpy(state.cur_palette->data, state.base_palette->data, 768);
}

void video_render_background(surface *sur) {
    SDL_Texture *tex = tcache_get(sur, state.renderer, state.cur_palette, NULL, 0);
    SDL_SetTextureColorMod(tex, 0xFF, 0xFF, 0xFF);
    SDL_SetTextureAlphaMod(tex, 0xFF);
    SDL_RenderCopy(state.renderer, tex, NULL, NULL);
}

void video_render_helper(
            SDL_Texture *tex, 
            int sx,
            int sy,
            int w,
            int h,
            unsigned int blend_mode,
            unsigned int flip_mode,
            float scale_y) {

    // Size & pois
    SDL_Rect dst;
    dst.w = w;
    dst.h = h * scale_y;
    dst.x = sx;
    dst.y = sy + (h - dst.h) / 2;
    
    if(blend_mode == BLEND_ADDITIVE) {
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
    } else {
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    }

    // Flipping
    SDL_RendererFlip flip = 0;
    if(flip_mode & FLIP_HORIZONTAL) flip |= SDL_FLIP_HORIZONTAL;
    if(flip_mode & FLIP_VERTICAL) flip |= SDL_FLIP_VERTICAL;

    // Okay, render now.
    SDL_RenderCopyEx(state.renderer, tex, NULL, &dst, 0, NULL, flip);
}

void video_render_sprite_shadow(surface *sur, int sx, int pal_offset, unsigned int flip_mode) {
    SDL_Texture *tex = tcache_get(sur, state.renderer, state.cur_palette, NULL, pal_offset);

    // Set rendering mode for shadow. We want sprite pixels black, with some opacity.
    SDL_SetTextureColorMod(tex, 0, 0, 0);
    SDL_SetTextureAlphaMod(tex, 64);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    // Position & correct height
    float scale_y = 0.25f;
    SDL_Rect dst;
    dst.w = sur->w;
    dst.h = sur->h * scale_y;
    dst.x = sx;
    dst.y = (190 - sur->h) + (sur->h - dst.h);

    SDL_RendererFlip flip = 0;
    if(flip_mode & FLIP_HORIZONTAL) flip |= SDL_FLIP_HORIZONTAL;
    if(flip_mode & FLIP_VERTICAL) flip |= SDL_FLIP_VERTICAL;
    SDL_RenderCopyEx(state.renderer, tex, NULL, &dst, 0, NULL, flip);
}

void video_render_sprite_tint(
            surface *sur, 
            int sx, 
            int sy, 
            color c, 
            int pal_offset) {

    SDL_Texture *tex = tcache_get(sur, state.renderer, state.cur_palette, NULL, pal_offset);
    SDL_SetTextureColorMod(tex, c.r, c.g, c.b);
    SDL_SetTextureAlphaMod(tex, 0xFF);
    video_render_helper(tex, sx, sy, sur->w, sur->h, BLEND_ALPHA, FLIP_NONE, 1.0);
}

void video_render_sprite(surface *sur, int sx, int sy, unsigned int rendering_mode, int pal_offset) {
    video_render_sprite_flip_scale(sur, sx, sy, rendering_mode, pal_offset, FLIP_NONE, 1.0f);
}

void video_render_sprite_flip_scale(
            surface *sur, 
            int sx, 
            int sy, 
            unsigned int rendering_mode, 
            int pal_offset, 
            unsigned int flip_mode, 
            float y_percent) {

    SDL_Texture *tex = tcache_get(sur, state.renderer, state.cur_palette, NULL, pal_offset);
    SDL_SetTextureColorMod(tex, 0xFF, 0xFF, 0xFF);
    SDL_SetTextureAlphaMod(tex, 0xFF);
    video_render_helper(tex, sx, sy, sur->w, sur->h, rendering_mode, flip_mode, y_percent);
}

void video_render_sprite_flip_scale_opacity(
            surface *sur, 
            int sx, 
            int sy, 
            unsigned int rendering_mode, 
            int pal_offset, 
            unsigned int flip_mode, 
            float y_percent, 
            uint8_t opacity) {

    SDL_Texture *tex = tcache_get(sur, state.renderer, state.cur_palette, NULL, pal_offset);
    SDL_SetTextureAlphaMod(tex, opacity);
    SDL_SetTextureColorMod(tex, 0xFF, 0xFF, 0xFF);
    video_render_helper(tex, sx, sy, sur->w, sur->h, rendering_mode, flip_mode, y_percent);
}

void video_render_finish() {
    // Copy the default target texture over
    SDL_SetRenderTarget(state.renderer, NULL);
    SDL_RenderCopy(state.renderer, state.target, NULL, NULL);

    // Flip
    SDL_RenderPresent(state.renderer);
    if(!state.vsync) {
        SDL_Delay(1);
    }
}

void video_close() {
    SDL_DestroyTexture(state.target);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    free(state.cur_palette);
    free(state.base_palette);
    INFO("Video deinit.");
}
