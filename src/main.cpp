#include <raylib.h>
#include <math.h>
#include <vector>
#include <random>
#include <fstream>

class Ball {
public:
    float x, y;
    float speed_x, speed_y;
    int radius;
    float speed;

    Ball() {
        radius = 10;
        speed = 5.0f;
        Reset();
    }

    void Reset() {
        x = GetScreenWidth() / 2;
        y = GetScreenHeight() / 2;
        
        float angle = GetRandomValue(0, 359) * DEG2RAD;
        speed_x = cosf(angle) * speed;
        speed_y = sinf(angle) * speed;
    }

    void Draw() {
        DrawCircle(x, y, radius, WHITE);
    }

    void Update(std::vector<Vector2>& circleVertices, float paddleAngle, float paddleSize, int& score, int& highScore) {
        x += speed_x;
        y += speed_y;

        Vector2 center = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        float distanceToCenter = sqrtf((x - center.x) * (x - center.x) + (y - center.y) * (y - center.y));
        float circleRadius = 300;

        if (distanceToCenter + radius > circleRadius) {
            float ballAngle = atan2f(y - center.y, x - center.x) * RAD2DEG;
            if (ballAngle < 0) ballAngle += 360;
            
            bool hitPaddle = (ballAngle >= paddleAngle - paddleSize/2 && ballAngle <= paddleAngle + paddleSize/2);
            
            if (hitPaddle) {
                Vector2 normal = {x - center.x, y - center.y};
                float normalLength = sqrtf(normal.x * normal.x + normal.y * normal.y);
                normal.x /= normalLength;
                normal.y /= normalLength;
                
                float dotProduct = speed_x * normal.x + speed_y * normal.y;
                speed_x = speed_x - 2 * dotProduct * normal.x;
                speed_y = speed_y - 2 * dotProduct * normal.y;
                
                float randomAngle = GetRandomValue(-10, 10) * DEG2RAD;
                float newSpeedX = speed_x * cosf(randomAngle) - speed_y * sinf(randomAngle);
                float newSpeedY = speed_x * sinf(randomAngle) + speed_y * cosf(randomAngle);
                speed_x = newSpeedX;
                speed_y = newSpeedY;
                
                float currentSpeed = sqrtf(speed_x * speed_x + speed_y * speed_y);
                speed_x = (speed_x / currentSpeed) * speed;
                speed_y = (speed_y / currentSpeed) * speed;
                
                speed += 0.2f;
                
                score++;
                if (score > highScore) {
                    highScore = score;
                    std::ofstream file("highscore.txt");
                    if (file.is_open()) {
                        file << highScore;
                        file.close();
                    }
                }
            } else {
                score = 0;
                Reset();
            }
        }
    }
};

class Paddle {
public:
    float angle;
    float size;
    
    Paddle(float startAngle) : angle(startAngle), size(30) {}
    
    void Draw(Vector2 center, float circleRadius) {
        float startAngle = angle - size/2;
        float endAngle = angle + size/2;
        
        for (float a = startAngle; a <= endAngle; a += 1.0f) {
            Vector2 point = {
                center.x + (circleRadius + 5) * cosf(a * DEG2RAD),
                center.y + (circleRadius + 5) * sinf(a * DEG2RAD)
            };
            DrawCircleV(point, 8, WHITE);
        }
    }
    
    void Update(float circleRadius) {
        if (IsKeyDown(KEY_LEFT)) angle -= 6.0f;
        if (IsKeyDown(KEY_RIGHT)) angle += 6.0f;
        
        if (angle < 0) angle += 360;
        if (angle > 360) angle -= 360;
    }
};

int LoadHighScore() {
    int highScore = 0;
    std::ifstream file("highscore.txt");
    if (file.is_open()) {
        file >> highScore;
        file.close();
    }
    return highScore;
}

int main() {
    const int screen_height = 800;
    const int screen_width = 800;

    InitWindow(screen_height, screen_width, "Circular Pong");
    SetTargetFPS(60);

    Vector2 center = {screen_width / 2.0f, screen_height / 2.0f};
    float circleRadius = 300;
    float rotation = 0.0f;
    int sides = 100;
    float thickness = 6;

    std::vector<Vector2> circleVertices;
    for (int i = 0; i < sides; i++) {
        float angle = rotation + (float)i * (360.0f/sides) * DEG2RAD;
        Vector2 vertex = {
            center.x + circleRadius * cosf(angle),
            center.y + circleRadius * sinf(angle)
        };
        circleVertices.push_back(vertex);
    }

    Ball ball;
    Paddle paddle(270);
    
    int score = 0;
    int highScore = LoadHighScore();
    
    bool gameStarted = false;

    while (!WindowShouldClose()) {
        if (!gameStarted) {
            if (IsKeyPressed(KEY_SPACE)) {
                gameStarted = true;
                score = 0;
                ball.Reset();
            }
        } else {
            paddle.Update(circleRadius);
            ball.Update(circleVertices, paddle.angle, paddle.size, score, highScore);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawPolyLinesEx(center, sides, circleRadius, rotation, thickness, WHITE);
        
        paddle.Draw(center, circleRadius);
        
        ball.Draw();
        
        char scoreText[50];
        sprintf(scoreText, "%d", score);
        int scoreWidth = MeasureText(scoreText, 60);
        DrawText(scoreText, screen_width/2 - scoreWidth/2, screen_height/2 - 30, 60, WHITE);
        
        sprintf(scoreText, "High: %d", highScore);
        int highScoreWidth = MeasureText(scoreText, 20);
        DrawText(scoreText, screen_width/2 - highScoreWidth/2, screen_height/2 + 40, 20, GRAY);
        
        if (!gameStarted) {
            const char* message = "Press SPACE to start";
            int msgWidth = MeasureText(message, 30);
            DrawText(message, screen_width/2 - msgWidth/2, screen_height/3 - 15, 30, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}