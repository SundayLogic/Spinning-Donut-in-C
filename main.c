#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

// Constants to define screen size, rotation speed, and mathematical constants.
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1600
#define TRANSITION_TIME 5000
#define PI 3.1415926535

#define FRAME_RATE 60
#define FRAME_DELAY (1000 / FRAME_RATE)

// Constants to define the shape and size of the donut.
#define THETA_SPACING 0.07
#define PHI_SPACING 0.02
#define R1 1
#define R2 2
#define K2 5

// Define a Color structure to hold the color values for drawing.
typedef struct Color
{
    Uint8 r, g, b, a;
} Color;

// Define background and donut colors using the Color structure.
Color backgroundColor = {0, 0, 0, 0};    // Blue background
Color donutColor = {255, 255, 255, 255}; // White donut

// Function to initialize an SDL_Window object.
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

// Function to initialize an SDL_Renderer object.
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

// Function to draw a donut shape, with rotations based on A and B.
void drawDonut(SDL_Renderer *renderer, float A, float B)
{
    // Pre-compute the cosine and sine of A and B to optimize the loop below.
    float cosA = cos(A), sinA = sin(A);
    float cosB = cos(B), sinB = sin(B);
    
    // Constant to adjust the projected size of the donut.
    const float K1 = SCREEN_WIDTH * K2 * 3 / (8 * (R1 + R2));

    // Double loop over theta and phi to cover the entire surface of the donut.
    for (float theta = 0; theta < 2 * PI; theta += THETA_SPACING)
    {
        float costheta = cos(theta), sintheta = sin(theta);

        for (float phi = 0; phi < 2 * PI; phi += PHI_SPACING)
        {
            float cosphi = cos(phi), sinphi = sin(phi);

            // Calculating the 3D coordinates of the points on the surface of the donut.
            float circlex = R2 + R1 * costheta;
            float circley = R1 * sintheta;

            // Projecting those coordinates into 2D space using a perspective projection.
            float x = circlex * (cosB * cosphi + sinA * sinB * sinphi) - circley * cosA * sinB;
            float y = circlex * (sinB * cosphi - sinA * cosB * sinphi) + circley * cosA * cosB;
            float z = K2 + cosA * circlex * sinphi + circley * sinA;
            float ooz = 1 / z;

            // Converting the 2D points into screen coordinates.
            int xp = (int)(SCREEN_WIDTH / 2 + K1 * ooz * x);
            int yp = (int)(SCREEN_HEIGHT / 2 - K1 * ooz * y);

            // Calculating the light effect on the donut.
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

// Main function, where the program starts.
int main(int argc, char *argv[])
{
    // Initialize SDL video mode.
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize window and renderer.
    SDL_Window *window = initWindow();
    SDL_Renderer *renderer = initRenderer(window);

    // Set up event handling, rotation parameters, and timing.
    SDL_Event event;
    int running = 1;
    Uint32 startTime = SDL_GetTicks();
    float A = 0, B = 0;
    Uint32 frameStart, frameTime;

    // Main game loop: handle events, draw the donut, wait for next frame.
    while (running)
    {
        frameStart = SDL_GetTicks();

        // Time since the program started, in seconds.
        float t = (SDL_GetTicks() - startTime) / (float)TRANSITION_TIME;
        if (t > 1.0f)
        {
            t = 1.0f;
        }

        // Clear the screen with the background color.
        SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
        SDL_RenderClear(renderer);

        // Draw the donut with the current rotation angles.
        drawDonut(renderer, A, B);

        // Update the rotation angles for the next frame.
        A += 0.01;
        B += 0.01;

        // Poll for events, and handle the case where the user closes the window.
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
        }

        // Present the rendered frame to the user.
        SDL_RenderPresent(renderer);

        // Ensure the program doesn't run too fast by delaying the next frame if necessary.
        frameTime = SDL_GetTicks() - frameStart;
        if (FRAME_DELAY > frameTime)
        {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    // Clean up SDL and exit the program.
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
