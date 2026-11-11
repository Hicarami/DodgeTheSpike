#include "raylib.h"
#include <vector>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <cmath>


// world size
const int cellSize = 32;
const int cellCountX = 15;
const int cellCountY = 10;


typedef struct Player {
    Vector2 position;
    float speed;

    int currentFrame;
    int framesCounter;
    int framesSpeed;

    float invulnerabilityTimer = 0.0f;

} Player;

struct Projectile {
    Vector2 position;
    Vector2 velocity;

    float radius = 7.0f;
    bool active = true;

}; 
std::vector<Projectile> projectiles;

struct SpikeAttack
{
    float survivalTimer = 0.0f;
    float spawnTimer = 0.0f;

    float spawnInterval = 0.7f;
    float projectileSpeed = 180.0f;

    float difficultyTimer = 0.0f;
    float difficultyInterval = 10.0f;

    float spawnIntervalDecrease = 0.08f;
    float speedIncrease = 20.0f;

    float minSpawnInterval = 0.15f;
    float maxProjectileSpeed = 300.0f;

};

void drawCell(Texture2D texture, int posX, int posY, Color tint) {
    DrawTexture(
        texture,
        posX * cellSize,
        posY * cellSize,
        tint
    );
}

void drawBorder(Texture2D platformTexture) {

    ClearBackground({ 50, 50, 50, 255 });

    for (int x = 0; x < cellCountX; x++) {

        int y = 0;
        drawCell(platformTexture, x, y, { WHITE });
    }

    for (int y = 1; y < cellCountY; y++) {

        int x = 0;
        drawCell(platformTexture, x, y, { WHITE });
    }

    for (int y = 1; y < cellCountY; y++) {

        int x = cellCountX - 1;
        drawCell(platformTexture, x, y, { WHITE });
    }

    for (int x = 1; x < cellCountX; x++) {
        int y = cellCountY - 1;
        drawCell(platformTexture, x, y, { WHITE });
    }
}

void UpdateGame(Player& player, float speed, float deltaTime) {
    // player input
    if (IsKeyDown(KEY_A)) player.position.x -= speed * deltaTime;

    if (IsKeyDown(KEY_D)) player.position.x += speed * deltaTime;

    if (IsKeyDown(KEY_W)) player.position.y -= speed * deltaTime;

    if (IsKeyDown(KEY_S)) player.position.y += speed * deltaTime;


    if (player.position.y + 20 >= (cellCountY - 1) * cellSize) {
        player.position.y = (cellCountY - 1) * cellSize - 20;
    }

    if (player.position.y <= cellSize)
        player.position.y = cellSize;

    if (player.position.x + 32 >= (cellCountX - 1) * cellSize) {
        player.position.x = (cellCountX - 1) * cellSize - 32;
    }

    if (player.position.x <= cellSize) {
        player.position.x = cellSize;
    }

}

void SpawnProjectile(Vector2 position, Vector2 target, float speed)
{
    Vector2 direction = {
        target.x - position.x,
        target.y - position.y
    };

    float length = sqrtf(
        direction.x * direction.x +
        direction.y * direction.y
    );

    if (length > 0.0f)
    {
        direction.x /= length;
        direction.y /= length;
    }

    Projectile p;

    p.position = position;
    p.velocity = {
        direction.x * speed,
        direction.y * speed
    };

    projectiles.push_back(p);
}

void UpdateProjectiles(float deltaTime)
{
    for (auto& p : projectiles)
    {
        p.position.x += p.velocity.x * deltaTime;
        p.position.y += p.velocity.y * deltaTime;

        if (p.position.x < 0 ||
            p.position.x > GetScreenWidth() ||
            p.position.y < 0 ||
            p.position.y > GetScreenHeight())
        {
            p.active = false;
        }
    }

    projectiles.erase(
        std::remove_if(
            projectiles.begin(),
            projectiles.end(),
            [](const Projectile& p)
            {
                return !p.active;
            }
        ),
        projectiles.end()
    );
}

void UpdateSpikeAttack(
    Player& player,
    SpikeAttack& attack,
    float deltaTime)
{

    attack.survivalTimer += deltaTime;
    attack.difficultyTimer += deltaTime;

    while (attack.difficultyTimer >= attack.difficultyInterval)
    {
        attack.difficultyTimer -= attack.difficultyInterval;


        attack.spawnInterval = std::max(
            attack.minSpawnInterval,
            attack.spawnInterval - attack.spawnIntervalDecrease
        );

        attack.projectileSpeed = std::min(
            attack.maxProjectileSpeed,
            attack.projectileSpeed + attack.speedIncrease
        );
    }

    attack.spawnTimer += deltaTime;

    if (attack.spawnTimer >= attack.spawnInterval)
    {
        attack.spawnTimer -= attack.spawnInterval;

        Vector2 spawnPosition;


        int side = GetRandomValue(0, 3);

        switch (side)
        {
        case 0: 
            spawnPosition = {
                (float)GetRandomValue(48, 432),
                16.0f
            };
            break;

        case 1: 
            spawnPosition = {
                (float)GetRandomValue(48, 432),
                304.0f
            };
            break;

        case 2: 
            spawnPosition = {
                16.0f,
                (float)GetRandomValue(48, 272)
            };
            break;

        case 3:
            spawnPosition = {
                464.0f,
                (float)GetRandomValue(48, 272)
            };
            break;
        }

        Vector2 playerCenter = {
            player.position.x + 16.0f,
            player.position.y + 10.0f
        };

        SpawnProjectile(
            spawnPosition,
            playerCenter,
            attack.projectileSpeed
        );
    }
}

bool CheckProjectileCollisions(const Player& player)
{

    Rectangle playerHitbox = {
        player.position.x,
        player.position.y,
        32.0f,
        20.0f
    };

    for (const auto& p : projectiles)
    {
        if (!p.active)
            continue;

        if (CheckCollisionCircleRec(
            p.position,
            p.radius,
            playerHitbox))
        {
            return true;
        }
    }

    return false;
}

void DrawProjectiles(Texture2D spikeTexture)
{
    for (const auto& p : projectiles)
    {
        Vector2 drawPosition = {
            p.position.x - spikeTexture.width / 2.0f,
            p.position.y - spikeTexture.height / 2.0f
        };

        DrawTextureV(spikeTexture, drawPosition, WHITE);
    }
}


void ResetGame(Player& player, SpikeAttack& attack)
{

    player.position = { 160.0f, 256.0f };


    player.currentFrame = 0;
    player.framesCounter = 0;

    projectiles.clear();

    attack = SpikeAttack{};
}

int main()
{

    InitWindow(
        cellSize * cellCountX,
        cellSize * cellCountY,
        "DODGE THE SPIKES!"
    );

    // Loading Textures
    Texture2D platformTexture = LoadTexture(RESOURCES_PATH "wall.png");
    Texture2D playerTexture = LoadTexture(RESOURCES_PATH "eye-Sheet.png");
    Texture2D spikeTexture = LoadTexture(RESOURCES_PATH "spike.png");

    // Player data
    Player player = { 0 };
    player.position = { 160, 256 };
    player.speed = 0;
    player.framesCounter = 0;
    player.currentFrame = 0;
    player.framesSpeed = 6;

    // Camera
    Camera2D camera = { 0 };
    camera.target = { player.position.x + 20.0f, player.position.y + 20.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.2f;

    float worldWidth = cellCountX * cellSize;
    float worldHeight = cellCountY * cellSize;

    float halfViewWidth = GetScreenWidth() / (2.0f * camera.zoom);
    float halfViewHeight = GetScreenHeight() / (2.0f * camera.zoom);

    SetTargetFPS(60);

    Rectangle playerframeRec = {
        32 * player.currentFrame, 0.0f, 32.0f, 20.0f
    };

    SpikeAttack spikeAttack;

    float bestTime = 0.0f;
    float lastSurvivedTime = 0.0f;


    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        player.framesCounter++;

        if (player.framesCounter >= (60 / player.framesSpeed))
        {
            player.framesCounter = 0;
            player.currentFrame++;

            if (player.currentFrame > 11) player.currentFrame = 0;

            playerframeRec.x = (float)player.currentFrame * (float)playerTexture.width / 11;
        }

        camera.target.x = player.position.x + 16.0f;
        camera.target.y = player.position.y + 10.0f;

        camera.zoom = 1.3f;

        camera.offset = {
            GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f
        };

        float halfViewWidth = GetScreenWidth() / (2.0f * camera.zoom);
        float halfViewHeight = GetScreenHeight() / (2.0f * camera.zoom);


        if (camera.target.x < halfViewWidth)
            camera.target.x = halfViewWidth;

        if (camera.target.x > worldWidth - halfViewWidth)
            camera.target.x = worldWidth - halfViewWidth;


        if (camera.target.y < halfViewHeight)
            camera.target.y = halfViewHeight;

        if (camera.target.y > worldHeight - halfViewHeight)
            camera.target.y = worldHeight - halfViewHeight;

        BeginDrawing();
        BeginMode2D(camera);

        drawBorder(platformTexture);
        DrawProjectiles(spikeTexture);
        DrawTextureRec(playerTexture, playerframeRec, player.position, WHITE);


        UpdateGame(player, 200, deltaTime);


        UpdateSpikeAttack(player, spikeAttack, deltaTime);

        UpdateProjectiles(deltaTime);


        if (CheckProjectileCollisions(player))
        {

            lastSurvivedTime = spikeAttack.survivalTimer;


            if (lastSurvivedTime > bestTime)
            {
                bestTime = lastSurvivedTime;
            }

            ResetGame(player, spikeAttack);
        }


        EndMode2D();


        DrawText(
            TextFormat("TIME: %.1f s", spikeAttack.survivalTimer),
            10, 10, 20, WHITE
        );

        DrawText(
            TextFormat("LAST: %.1f s", lastSurvivedTime),
            10, 35, 20, LIGHTGRAY
        );


        DrawText(
            TextFormat("BEST: %.1f s", bestTime),
            10, 60, 20, GOLD
        );


        EndDrawing();
    }
    UnloadTexture(spikeTexture);
    UnloadTexture(playerTexture);
    UnloadTexture(platformTexture);
    CloseWindow();

    return 0;
}


