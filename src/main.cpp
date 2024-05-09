#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
using namespace std;

// Structure to hold tile coordinates
struct Vector2i {
    int x, y;
    Vector2i() = default;
    Vector2i(int _x, int _y) : x(_x), y(_y) {}
};

// Structure to hold tile coordinates in 3D
struct Vector3i {
    int x, y, z;
    Vector3i() = default;
    Vector3i(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
};

const int FIELD_X = 50;
const int FIELD_Y = 50;
const int FIELD_Z = 50;

int field[FIELD_X + 4][FIELD_Y + 4][FIELD_Z] = {0};

int& f(int x, int y, int z) { return field[y + 2][x + 2][z]; }
int& f(Vector3i v) { return f(v.x, v.y, v.z); }

// Function to check if a tile is open and can be selected
bool isOpen(int x, int y, int z)
{
    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++)
            if (f(x + 2, y + i, z) > 0 && f(x - 2, y + j, z) > 0) return false;

    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++)
            if (f(x + i, y + j, z + 1) > 0) return false;

    return true;
}

int main(int argc, char* argv[])
{
    srand(time(0));

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        cout << "SDL_Init Error: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Majinso!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 740, 570, SDL_WINDOW_SHOWN);
    if (!window)
    {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* t1 = IMG_LoadTexture(renderer, "files/tiles.png");
    SDL_Texture* t2 = IMG_LoadTexture(renderer, "files/background.png");
    if (!t1 || !t2)
    {
        cout << "IMG_LoadTexture Error: " << SDL_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Rect backgroundRect = { 0, 0, 740, 570 };
    SDL_Rect tileRect = { 0, 0, 48, 66 };

    Vector3i v1, v2; // Variables to store selected tile coordinates
    std::vector<Vector3i> moves; // Vector to store selected moves

    // Load map from file
    std::fstream myfile("files/map.txt");
    for (int y = 0; y < 18; y++)
        for (int x = 0; x < 30; x++)
        {
            char a;
            myfile >> a;
            int n = a - '0';
            for (int z = 0; z < n; z++)
                if (f(x - 1, y - 1, z)) f(x - 1, y, z) = f(x, y - 1, z) = 0;
                else f(x, y, z) = 1;
        }

    // Create map
    for (int k = 1;; k++)
    {
        std::vector<Vector3i> opens;
        for (int z = 0; z < 10; z++)
            for (int y = 0; y < 18; y++)
                for (int x = 0; x < 30; x++)
                    if (f(x, y, z) > 0 && isOpen(x, y, z)) opens.push_back(Vector3i(x, y, z));

        int n = opens.size();
        if (n < 2) break;
        int a = 0, b = 0;
        while (a == b) { a = rand() % n; b = rand() % n; }
        f(opens[a]) = -k;  if (k > 34) k++;
        f(opens[b]) = -k;
        k %= 42;
    }

    // Reverse the tiles to hide them
    for (int z = 0; z < 10; z++)
        for (int y = 0; y < 18; y++)
            for (int x = 0; x < 30; x++) f(x, y, z) *= -1;

    SDL_Event event;
    bool isRunning = true;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                isRunning = false;

            // Handle mouse button press
            if ((event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_FINGERDOWN) && event.button.button == SDL_BUTTON_LEFT)
            {
                for (int z = 0; z < 10; z++)
                {
                    Vector2i pos = { event.button.x - 30, event.button.y }; // 30 - desk offset
                    int x = (pos.x - z * 4.6) / 22;
                    int y = (pos.y + z * 7.1) / 33;

                    for (int i = 0; i < 2; i++)
                        for (int j = 0; j < 2; j++)
                            if (f(x - i, y - j, z) > 0 && isOpen(x - i, y - j, z))
                                v1 = Vector3i(x - i, y - j, z);

                    if (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z)
                        continue;

                    int a = f(v1), b = f(v2);
                    if (a == b || (a > 34 && a < 39 && b > 34 && b < 39) || (a >= 39 && b >= 39))
                    {
                        f(v1) *= -1; moves.push_back(v1);
                        f(v2) *= -1; moves.push_back(v2);
                    }
                    v2 = v1;
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, t2, nullptr, &backgroundRect);
        for (int z = 0; z < 10; z++)
            for (int x = 30; x >= 0; x--)
                for (int y = 0; y < 18; y++)
                {
                    int k = f(x, y, z) - 1;
                    if (k < 0) continue;
                    tileRect.x = k * 48;
                    if (isOpen(x, y, z)) tileRect.y = 66;
                    SDL_Rect destRect = { static_cast<int>(x * 22 + z * 4.6), static_cast<int>(y * 33 - z * 7.1), 48, 66 };
                    SDL_RenderCopy(renderer, t1, &tileRect, &destRect);
                }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(t1);
    SDL_DestroyTexture(t2);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
