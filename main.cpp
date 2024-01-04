#include <deque>
#include <iostream>
#include <raylib.h>
#include <raymath.h>

using namespace std;
// 1. creating a blank canvas & game loop -> done
// 2. creating a food -> done
// 3. creating a snake -> done
// 4. moving the snake -> done
// 5. making the snake eat the food -> done
// 6. making the snake grow longer -> done
// 7. checking for collision with edges & tail -> done
// 8. adding title & frame -> on progress
// 9. keeping score
// 10. adding sounds

Color green = {172, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};
Color Dark_Blue = Color{0, 82, 172, 255};

// sell dalam sebuah vertex
int cellSize = 30;
int cellCount = 25;
int offset = 75;

// melacak waktu terjadinya pembaruan terakhir
double lastUpdateTime = 0;

// mengembalikan nilai true jika element tertentu berada dalam deque ular
// fungsi ini agar makan ular tidak muncul didalam cell ular.
// fungsi ini juga berguna untuk mendeteksi apakah kepala ular berada dalam cell
// ekor
bool ElementInDeque(Vector2 element, deque<Vector2> deque) {
  for (unsigned int i = 0; i < deque.size(); i++) {
    cout << "deque[i].x " << deque[i].x << "element.x: " << element.x << endl;
    cout << "deque[i].y " << deque[i].y << "element.y: " << element.y << endl;
    if (Vector2Equals(deque[i], element)) {
      cout << " ===========true=============" << endl;
      cout << "deque[i].x " << deque[i].x << "element.x: " << element.x << endl;
      cout << "deque[i].y " << deque[i].y << "element.y: " << element.y << endl;
      return true;
    }
  }
  return false;
}

// check apakah suatu interval telah terlewati, dan akan mengembalikan
// true/false
bool eventTriggered(double interval) {
  double currenttTime = GetTime();

  // jika selisihnya besar atau sama dengan interval, maka interval tersebut
  // telah berlalu sejak pembaruan terkahir lalu lakukan update kepada variabel
  // lastUpdateTime ke waktu saat ini, dan mengembalikan nilai true
  if (currenttTime - lastUpdateTime >= interval) {
    lastUpdateTime = currenttTime;
    return true;
  }
  return false;
}

class Food {
public:
  Vector2 position;
  Texture2D texture;

  // konstruktor
  Food(deque<Vector2> snakeBody) {
    Image image = LoadImage("./Graphics/food.png");
    texture = LoadTextureFromImage(image);
    // kosong kan memori
    UnloadImage(image);

    position = GenerateRandomPos(snakeBody);
  }

  // Destructor, berfungsi untuk menyusun kembali texture yang memorinya telah
  // dikosongkan
  ~Food() { UnloadTexture(texture); }

  void Draw() {
    DrawTexture(texture, offset + position.x * cellSize,
                offset + position.y * cellSize, WHITE);
  }

  Vector2 GenerateRandomCell() {
    float x = GetRandomValue(0, cellCount - 1);
    float y = GetRandomValue(0, cellCount - 1);
    Vector2 position = {x, y};
    return Vector2{x, y};
  }

  // membuat posisi makanan selalu berubah saat di load
  Vector2 GenerateRandomPos(deque<Vector2> snakeBody) {
    // jika posisi makan ulara berada dalam cell ular, maka kita harus ubah
    // posisi makan tersebut
    Vector2 position = GenerateRandomCell();
    while (ElementInDeque(position, snakeBody)) {
      position = GenerateRandomCell();
    }
    return position;
  }
};

class Snake {
  // membuat object untuk badan ular menggunakan deque
  // deque ini flexibel, karena kita bisa push dan remove element didalamnya.
public:
  deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};

  // direction ini digunakan untuk penjumlahan vector, agar ular bisa bergerak
  // misalkan:
  // [4,9][5,9][6,9] + direction =  [5,9][6,9][7,9] -> maka ular bergerak
  // kekanan
  Vector2 direction = {1, 0};
  bool addSegment = false;

  void Draw() {
    for (unsigned i = 0; i < body.size(); i++) {
      int x = body[i].x;
      int y = body[i].y;

      Rectangle segment =
          Rectangle{(float)offset + x * cellSize, (float)offset + y * cellSize,
                    (float)cellSize, (float)cellSize};
      DrawRectangleRounded(segment, 0.5, 6, darkGreen);
    }
  }

  void update() {
    // tambahkan vector baru untuk kepala ular
    //   // direction ini digunakan untuk penjumlahan vector, agar ular bisa
    //   bergerak
    // misalkan:
    // [4,9][5,9][6,9] + direction =  [5,9][6,9][7,9]
    body.push_front(Vector2Add(body[0], direction));

    if (addSegment == true) {
      addSegment = false;
    } else {
      // pertama harus kita hapus vektor terkhir, dan tambahakn vektor baru ke
      // awal(kepala ular)
      body.pop_back();
    }
  }

  void Reset() {
    body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    direction = {1, 0};
  }
};

class Game {
public:
  Snake snake = Snake();
  Food food = Food(snake.body);
  bool running = true;
  int score = 0;
  Sound eatSound;
  Sound wallSound;

  // load sound kedalam memori
  Game() {
    InitAudioDevice();
    eatSound = LoadSound("./sounds/eat.mp3");
    wallSound = LoadSound("./sounds/wall.mp3");
  }

  // remove dari memori
  ~Game() {
    UnloadSound(eatSound);
    UnloadSound(wallSound);
    CloseAudioDevice();
  }

  void Draw() {
    food.Draw();
    snake.Draw();
  }

  void Update() {
    if (running) {
      snake.update();
      CheckCollisionWithFood();
      ChecekCollisionWithEdges();
      CheckCollisionWithTail();
    }
  }
  // check apakah posisi cell makan ==  kepala ular
  void CheckCollisionWithFood() {
    // body[0] adalah kepala ular
    if (Vector2Equals(snake.body[0], food.position)) {
      food.position = food.GenerateRandomPos(snake.body);

      // tambahkan ukuran ular setiap dia makan
      snake.addSegment = true;
      // tambahkan score setiap ular makan
      score++;
      PlaySound(eatSound);
    }
  }

  // check apakah kepala ular melewati cell
  void ChecekCollisionWithEdges() {
    // posisi kepala.ular.x ==  jumlahcell(25), artinya kepala ular sudah
    // melewati tepi kanan bingkai.
    // atau kepala.ular.x == -1, ini sama dengan, apakah ular tersebut sudah
    // melewati tepi kiri bingkai.
    if (snake.body[0].x == cellCount || snake.body[0].x == -1) {
      GameOver();
      PlaySound(wallSound);
    }
    if (snake.body[0].y == cellCount || snake.body[0].y == -1) {
      GameOver();
      PlaySound(wallSound);
    }
  }

  // akhiri game
  void GameOver() {
    // reset posisi ular
    snake.Reset();

    // acak kembali posisi makanan
    food.position = food.GenerateRandomPos(snake.body);

    // state untuk memulai kembali permainan.
    running = false;
    score = 0;
  }

  // check apakah kepala ular menabrak badan ular
  void CheckCollisionWithTail() {
    // membuat salinan tubuh ular
    deque<Vector2> headlessBody = snake.body;
    // dari salinan ini kita akan menghapus element pertama
    headlessBody.pop_front();

    // sekarang check apakah kepala ular yang asli berada di headlessBody
    if (ElementInDeque(snake.body[0], headlessBody)) {
      GameOver();
      PlaySound(wallSound);
    }
  }
};

int main() {
  InitWindow(2 * offset + cellSize * cellCount,
             2 * offset + cellSize * cellCount, "SNAKE GAME WITH RAYLIB");
  SetTargetFPS(60);

  Game game = Game();

  while (!WindowShouldClose()) {
    BeginDrawing();

    // perbaharui ular setiap 0.2 detik, agar ular tidak ngebut
    if (eventTriggered(0.2)) {
      game.Update();
    }

    if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) {
      game.snake.direction = {0, -1};
      game.running = true;
    }

    if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {
      game.snake.direction = {0, 1};
      game.running = true;
    }

    if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {
      game.snake.direction = {-1, 0};
      game.running = true;
    }

    if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {
      game.snake.direction = {1, 0};
      game.running = true;
    }

    ClearBackground(Dark_Blue);
    DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5,
                                   (float)cellSize * cellCount + 10,
                                   (float)cellSize * cellCount + 10},
                         5, darkGreen);

    DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
    DrawText(TextFormat("%i", game.score), offset - 5,
             offset + cellSize * cellCount + 10, 40, darkGreen);
    game.Draw();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
