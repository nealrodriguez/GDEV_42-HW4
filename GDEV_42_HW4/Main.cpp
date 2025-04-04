#include <raylib.h>
#include <raymath.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

#include "PlayerStateMachine.cpp"
#include "EnemyStateMachine.cpp"

struct Tile {
    int x, y, width, height;
    bool is_colliding;
};

const int WINDOW_WIDTH(1280);
const int WINDOW_HEIGHT(720);
const float FPS(60.0f);

Rectangle camera_window = {(WINDOW_WIDTH / 2) - 150, (WINDOW_HEIGHT / 2) - 150, 300.0f, 300.0f};
float cam_drift = 0.5f;
float cam_zoom;
bool zoom_in;

void MoveCamera(Camera2D* cam, Player* player, float delta_time) {
    float cam_push_x = 0.0f;
    float cam_push_y = 0.0f;
    float drift_x = 0.0f;
    float drift_y = 0.0f;

    if (player->position.x > camera_window.x + camera_window.width) {
        cam_push_x = player->position.x - (camera_window.x + camera_window.width);
        cam->target.x += cam_push_x;
        camera_window.x += cam_push_x;
    }
    else if (player->position.x < camera_window.x) {
        cam_push_x = player->position.x - camera_window.x;
        cam->target.x += cam_push_x;
        camera_window.x += cam_push_x;
    }
    else {
        cam->target.x += drift_x;
        camera_window.x += drift_x;
    }

    if (player->position.y > camera_window.y + camera_window.height) {
        cam_push_y = player->position.y - (camera_window.y + camera_window.height);
        cam->target.y += cam_push_y;
        camera_window.y += cam_push_y;
    }
    else if (player->position.y < camera_window.y) {
        cam_push_y = player->position.y - camera_window.y;
        cam->target.y += cam_push_y;
        camera_window.y += cam_push_y;
    }
    else {
        cam->target.y += drift_y;
        camera_window.y += drift_y;
    }
}

int main() {
    Player player({WINDOW_WIDTH/2, WINDOW_HEIGHT/2}, 15.0f, 150.0f, 5);
    std::vector<Enemy> enemies;
    Enemy enemy01({500,400}, 100.0f, 15.0f, 100.0f, 250.0f, 50.0f, 2);
    Enemy enemy02({200,100}, 100.0f, 15.0f, 100.0f, 250.0f, 50.0f, 2);

    bool game_ongoing = true;
    bool enemy_lose = false;

    enemies.push_back(enemy01);
    enemies.push_back(enemy02);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "AmadorRodriguezTatoy_HW4");
    SetTargetFPS(FPS);

    Camera2D camera_view = {0};
    camera_view.target = player.position;
    camera_view.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
    camera_view.zoom = 1.0f;

    ifstream file("leveldeets.txt");
    vector<Tile> tiles;
    string IMAGE_NAME;
    int TILE_COUNT, GRID_WIDTH = 0, GRID_HEIGHT = 0;
    getline(file, IMAGE_NAME);
    file >> TILE_COUNT;
    for (int i = 0; i < TILE_COUNT; ++i) {
        Tile tile;
        file >> tile.x >> tile.y >> tile.width >> tile.height;
        file >> tile.is_colliding;
        tiles.push_back(tile);
    }
    file >> GRID_WIDTH >> GRID_HEIGHT;
    vector<vector<int>> GRID(GRID_HEIGHT, vector<int>(GRID_WIDTH));
    for (int i = 0; i < GRID_HEIGHT; ++i) {
        for (int j = 0; j < GRID_WIDTH; ++j) {
            file >> GRID[i][j];
        }
    }
    file.close();

    Texture texture = LoadTexture(IMAGE_NAME.c_str());

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        if (game_ongoing) {
            enemy_lose = true;
            player.Update(delta_time);
            for (size_t i = 0; i < enemies.size(); i++) {
                if(enemies[i].active) {
                    enemy_lose = false;
                    enemies[i].Update(delta_time);
                    enemies[i].HandleCollision(&player);
                    player.HandleCollision(&enemies[i]);
                }
            }
            
            if(enemy_lose) {
                game_ongoing = false;
            }

            if(player.health <= 0) {
                game_ongoing = false;
            }

            std::cout << player.health << std::endl;
        }
        
        MoveCamera(&camera_view, &player, delta_time);
        
        BeginDrawing();
        BeginMode2D(camera_view);
        ClearBackground(BLACK);
        
        for (int i = 0; i < GRID_HEIGHT; ++i) {
            for (int j = 0; j < GRID_WIDTH; ++j) {
                int TILE_INDEX = GRID[i][j];
                if (TILE_INDEX >= 0 && TILE_INDEX < tiles.size()) {
                    Tile tile = tiles[TILE_INDEX];
                    DrawTextureRec(texture, 
                        { (float)tile.x, (float)tile.y, (float)tile.width, (float)tile.height }, 
                        { (float)(j * tile.width), (float)(i * tile.height) }, 
                        WHITE);
                }
            }
        }

        if (game_ongoing) {
            player.Draw();
            for (size_t i = 0; i < enemies.size(); i++) {
                if (enemies[i].active) {
                    enemies[i].Draw();
                } 
            }
        }
        
        EndMode2D();
        
        if (game_ongoing) {
            DrawText(std::to_string(player.health).c_str(), 10, 10, 50, WHITE);
        }
        else {
            if (enemy_lose) {
                DrawText("YOU WIN", WINDOW_WIDTH / 4, WINDOW_HEIGHT / 2 - 25, 100, WHITE);
            }
            else if (!enemy_lose) {
                DrawText("YOU LOSE", WINDOW_WIDTH / 4, WINDOW_HEIGHT / 2 - 25, 100, WHITE);
            }
        }
        EndDrawing();
    }
    UnloadTexture(texture);
    CloseWindow();
    
    return 0;
}
