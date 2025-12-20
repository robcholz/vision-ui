//
// Created by Finn Sheng (Ziheng Sheng) on 11/12/25.
//
/*
  u8x8_d_sdl_128x64.c
*/
#include "u8g2_240x240_display.h"

#include "u8g2.h"
#ifndef NO_SDL
#include "SDL.h"
#endif
#include <assert.h>
#include <stdio.h>

// #define HEIGHT (64)
// #define WIDTH 128

#define W(x, w) (((x) * (w)) / 100)

#ifndef NO_SDL
SDL_Window* U8G_SDL_WINDOW;
SDL_Surface* U8G_SDL_SCREEN;
#endif

int U8G_SDL_MULTIPLE = 3;
uint32_t U8G_SDL_COLOR[256];
int U8G_SDL_HEIGHT, U8G_SDL_WIDTH;


static void u8g_sdl_set_pixel(const int x, const int y, const int idx) {
    uint32_t* ptr;
    uint32_t offset;

    if (y >= U8G_SDL_HEIGHT) {
        return;
    }
    if (y < 0) {
        return;
    }
    if (x >= U8G_SDL_WIDTH) {
        return;
    }
    if (x < 0) {
        return;
    }

    for (int i = 0; i < U8G_SDL_MULTIPLE; i++) {
        for (int j = 0; j < U8G_SDL_MULTIPLE; j++) {
#ifndef NO_SDL
            offset =
                    (((y * U8G_SDL_MULTIPLE) + i) * (U8G_SDL_WIDTH * U8G_SDL_MULTIPLE) + ((x * U8G_SDL_MULTIPLE) + j)) *
                    U8G_SDL_SCREEN->format->BytesPerPixel;

            assert(offset < (Uint32) (U8G_SDL_WIDTH * U8G_SDL_MULTIPLE * U8G_SDL_HEIGHT * U8G_SDL_MULTIPLE *
                                      U8G_SDL_SCREEN->format->BytesPerPixel));

            ptr = (uint32_t*) (((uint8_t*) (U8G_SDL_SCREEN->pixels)) + offset);
            *ptr = U8G_SDL_COLOR[idx];
#endif
        }
    }
}

static void u8g_sdl_set_8pixel(const int x, int y, uint8_t pixel) {
    int cnt = 8;
    int bg = 0;
    if ((x / 8 + y / 8) & 1) {
        bg = 4;
    }
    while (cnt > 0) {
        if ((pixel & 1) == 0) {
            u8g_sdl_set_pixel(x, y, bg);
        } else {
            u8g_sdl_set_pixel(x, y, 3);
        }
        pixel >>= 1;
        y++;
        cnt--;
    }
}

static void u8g_sdl_set_multiple_8pixel(int x, const int y, int cnt, const uint8_t* pixel) {
    while (cnt > 0) {
        const uint8_t b = *pixel;
        u8g_sdl_set_8pixel(x, y, b);
        x++;
        pixel++;
        cnt--;
    }
}

static void u8g_sdl_init(int width, int height) {
    U8G_SDL_HEIGHT = height;
    U8G_SDL_WIDTH = width;

#ifndef NO_SDL

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Unable to initialize SDL:  %s\n", SDL_GetError());
        exit(1);
    }

    U8G_SDL_WINDOW = SDL_CreateWindow(
            "vision_ui-simulator",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            U8G_SDL_WIDTH * U8G_SDL_MULTIPLE,
            U8G_SDL_HEIGHT * U8G_SDL_MULTIPLE,
            0
    );

    if (U8G_SDL_WINDOW == NULL) {
        printf("Couldn't create window: %s\n", SDL_GetError());
        exit(1);
    }

    U8G_SDL_SCREEN = SDL_GetWindowSurface(U8G_SDL_WINDOW);

    if (U8G_SDL_SCREEN == NULL) {
        printf("Couldn't create screen: %s\n", SDL_GetError());
        exit(1);
    }

    printf("%d bits-per-pixel mode\n", U8G_SDL_SCREEN->format->BitsPerPixel);
    printf("%d bytes-per-pixel mode\n", U8G_SDL_SCREEN->format->BytesPerPixel);

    U8G_SDL_COLOR[0] = SDL_MapRGB(U8G_SDL_SCREEN->format, 0, 0, 0);
    U8G_SDL_COLOR[1] = SDL_MapRGB(U8G_SDL_SCREEN->format, W(100, 50), W(255, 50), 0);
    U8G_SDL_COLOR[2] = SDL_MapRGB(U8G_SDL_SCREEN->format, W(100, 80), W(255, 80), 0);
    U8G_SDL_COLOR[3] = SDL_MapRGB(U8G_SDL_SCREEN->format, 255, 255, 255);
    U8G_SDL_COLOR[4] = SDL_MapRGB(U8G_SDL_SCREEN->format, 30, 30, 30);

    /*
    u8g_sdl_set_pixel(0,0);
    u8g_sdl_set_pixel(1,1);
    u8g_sdl_set_pixel(2,2);
    */

    /* update all */
    SDL_UpdateWindowSurface(U8G_SDL_WINDOW);

    atexit(SDL_Quit);
#endif
    return;
}


/*
void main(void)
{
  u8g_sdl_init();
  u8g_sdl_set_pixel(0,0,3);
  u8g_sdl_set_pixel(0,1,3);
  u8g_sdl_set_pixel(0,2,3);
  u8g_sdl_set_pixel(1,1,3);
  u8g_sdl_set_pixel(2,2,3);
  while( u8g_sdl_get_key() < 0 )
    ;
}
*/


static uint8_t u8x8_d_sdl_gpio(
        u8x8_t* u8x8,
        const uint8_t msg,
        U8X8_UNUSED uint8_t arg_int,
        U8X8_UNUSED void* arg_ptr
) {
    static int debounce_cnt = 0;
    static int curr_msg = 0;
    static int db_cnt = 10;

    if (curr_msg > 0) {
        if (msg == curr_msg) {
            u8x8_SetGPIOResult(u8x8, 0);
            if (debounce_cnt == 0) {
                curr_msg = 0;
            } else {
                debounce_cnt--;
            }
            return 1;
        }

    } else {
        const int event = u8g_sdl_get_key();

        switch (event) {
            case 273:
                curr_msg = U8X8_MSG_GPIO_MENU_UP;
                debounce_cnt = db_cnt;
                break;
            case 274:
                curr_msg = U8X8_MSG_GPIO_MENU_DOWN;
                debounce_cnt = db_cnt;
                break;
            case 275:
                curr_msg = U8X8_MSG_GPIO_MENU_NEXT;
                debounce_cnt = db_cnt;
                break;
            case 276:
                curr_msg = U8X8_MSG_GPIO_MENU_PREV;
                debounce_cnt = db_cnt;
                break;
            case 's':
                curr_msg = U8X8_MSG_GPIO_MENU_SELECT;
                debounce_cnt = db_cnt;
                break;
            case 'q':
                curr_msg = U8X8_MSG_GPIO_MENU_HOME;
                debounce_cnt = db_cnt;
                break;
        }
    }
    u8x8_SetGPIOResult(u8x8, 1);
    return 1;
}

/*========================================*/
/* 240x240 */

static const u8x8_display_info_t U8X8_SDL_240X240_INFO = {
        /* chip_enable_level = */ 0,
        /* chip_disable_level = */ 1,

        /* post_chip_enable_wait_ns = */ 0,
        /* pre_chip_disable_wait_ns = */ 0,
        /* reset_pulse_width_ms = */ 0,
        /* post_reset_wait_ms = */ 0,
        /* sda_setup_time_ns = */ 0,
        /* sck_pulse_width_ns = */ 0,
        /* sck_clock_hz = */ 4000000UL, /* since Arduino 1.6.0, the SPI bus speed in Hz. Should be
                                           1000000000/sck_pulse_width_ns */
        /* spi_mode = */ 1,
        /* i2c_bus_clock_100kHz = */ 0,
        /* data_setup_time_ns = */ 0,
        /* write_pulse_width_ns = */ 0,
        /* tile_width = */ 30, /* width of 30*8=240 pixel */
        /* tile_hight = */ 30, /* height: 240 pixel */
        /* default_x_offset = */ 0,
        /* flipmode_x_offset = */ 0,
        /* pixel_width = */ 240,
        /* pixel_height = */ 240
};


uint8_t u8x8_d_sdl_240x240(u8x8_t* u8g2, const uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    uint8_t x, y;
    switch (msg) {
        case U8X8_MSG_DISPLAY_SETUP_MEMORY:
            u8x8_d_helper_display_setup_memory(u8g2, &U8X8_SDL_240X240_INFO);
            u8g_sdl_init(240, 240);
            break;
        case U8X8_MSG_DISPLAY_INIT:
            u8x8_d_helper_display_init(u8g2);
            break;
        case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
            break;
        case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
            break;
        case U8X8_MSG_DISPLAY_SET_CONTRAST:
            break;
        case U8X8_MSG_DISPLAY_DRAW_TILE:
            x = ((u8x8_tile_t*) arg_ptr)->x_pos;
            x *= 8;
            x += u8g2->x_offset;

            y = ((u8x8_tile_t*) arg_ptr)->y_pos;
            y *= 8;

            do {
                const uint8_t c = ((u8x8_tile_t*) arg_ptr)->cnt;
                const uint8_t* ptr = ((u8x8_tile_t*) arg_ptr)->tile_ptr;
                u8g_sdl_set_multiple_8pixel(x, y, c * 8, ptr);
                arg_int--;
            } while (arg_int > 0);
            // note: completely ignore the stupid draw tile, this drag down the fps to ~70 from ~400
            break;
        case U8X8_MSG_DISPLAY_REFRESH:
#ifndef NO_SDL
            /* update all */
            SDL_UpdateWindowSurface(U8G_SDL_WINDOW);
#endif
            break;
        default:
            return 0;
    }
    return 1;
}


void u8x8_setup_sdl_240x240(u8x8_t* u8x8) {
    /* setup defaults */
    u8x8_SetupDefaults(u8x8);

    /* setup specific callbacks */
    u8x8->display_cb = u8x8_d_sdl_240x240;

    u8x8->gpio_and_delay_cb = u8x8_d_sdl_gpio;

    /* setup display info */
    u8x8_SetupMemory(u8x8);
}
