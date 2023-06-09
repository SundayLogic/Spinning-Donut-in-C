#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1600
#define TRANSITION_TIME 5000
#define PI 3.1415926535

#define FRAME_RATE 60
#define FRAME_DELAY (1000 / FRAME_RATE)

// Donut parameters
#define THETA_SPACING 0.07
#define PHI_SPACING 0.02
#define R1 1
#define R2 2
#define K2 5

typedef struct Color
{
    Uint8 r, g, b, a;
} Color;

Color backgroundColor = {0, 0, 0, 0};    // Blue background
Color donutColor = {255, 255, 255, 255}; // White donut

SDL_Window *initWindow()
{
    SDL_Window *window = SDL_CreateWindow("Spinning Donut", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    return window;
}

SDL_Renderer *initRenderer(SDL_Window *window)
{
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
    return renderer;
}

void drawDonut(SDL_Renderer *renderer, float A, float B)
{
    float cosA = cos(A), sinA = sin(A);
    float cosB = cos(B), sinB = sin(B);
    const float K1 = SCREEN_WIDTH * K2 * 3 / (8 * (R1 + R2));

    for (float theta = 0; theta < 2 * PI; theta += THETA_SPACING)
    {
        float costheta = cos(theta), sintheta = sin(theta);

        for (float phi = 0; phi < 2 * PI; phi += PHI_SPACING)
        {
            float cosphi = cos(phi), sinphi = sin(phi);

            float circlex = R2 + R1 * costheta;
            float circley = R1 * sintheta;

            float x = circlex * (cosB * cosphi + sinA * sinB * sinphi) - circley * cosA * sinB;
            float y = circlex * (sinB * cosphi - sinA * cosB * sinphi) + circley * cosA * cosB;
            float z = K2 + cosA * circlex * sinphi + circley * sinA;
            float ooz = 1 / z;

            int xp = (int)(SCREEN_WIDTH / 2 + K1 * ooz * x);
            int yp = (int)(SCREEN_HEIGHT / 2 - K1 * ooz * y);

            float L = cosphi * costheta * sinB - cosA * costheta * sinphi - sinA * sintheta + cosB * (cosA * sintheta - costheta * sinA * sinphi);
            if (L > 0)
            {
                Uint8 luminance = (Uint8)(L * 255);
                SDL_SetRenderDrawColor(renderer, donutColor.r, donutColor.g, donutColor.b, donutColor.a);
                SDL_RenderDrawPoint(renderer, xp, yp);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = initWindow();
    SDL_Renderer *renderer = initRenderer(window);

    SDL_Event event;
    int running = 1;
    Uint32 startTime = SDL_GetTicks();
    float A = 0, B = 0;
    Uint32 frameStart, frameTime;

    while (running)
    {
        frameStart = SDL_GetTicks();

        float t = (SDL_GetTicks() - startTime) / (float)TRANSITION_TIME;
        if (t > 1.0f)
        {
            t = 1.0f;
        }

        SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
        SDL_RenderClear(renderer);

        drawDonut(renderer, A, B);

        A += 0.01;
        B += 0.01;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
        }

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;

        if (FRAME_DELAY > frameTime)
        {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
