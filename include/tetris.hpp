#pragma once

#include <cstdint>
#include <cstring>

// should return a number between 0 and top - 1
uint32_t random(uint32_t top) { return 0; }

template <size_t WIDTH, size_t HEIGHT> struct Tetris {
  typedef enum { RED = 1, GREEN, BLUE, YELLOW, FUCHSIA, AQUA, BLINK } Color;

  struct Status {
    bool reset : 1;
    bool pause : 1;
    bool rotCCW : 1;
    bool rotCW : 1;
    bool left : 1;
    bool right : 1;
    bool down : 1;
    bool ending : 1;

  public:
    Status() {
      memset(this, 0, sizeof(Status));
      this->reset = true;
    }
  };

  const int8_t tiles[19][3][2] = {
      {{0, -2}, {0, -1}, {0, 1}},  // long piece vert
      {{-1, 0}, {1, 0}, {2, 0}},   // long piece hori
      {{1, 0}, {0, 1}, {1, 1}},    // block
      {{-1, 0}, {0, 1}, {1, 1}},   // stairs down hori
      {{0, -1}, {-1, 0}, {-1, 1}}, // stairs down vert
      {{1, 0}, {0, 1}, {-1, 1}},   // stairs up hori
      {{-1, -1}, {-1, 0}, {0, 1}}, // stairs up vert
      {{0, -1}, {0, 1}, {-1, 1}},  // L left 000
      {{-1, -1}, {-1, 0}, {1, 0}}, // L left 090
      {{0, -1}, {1, -1}, {0, 1}},  // L left 180
      {{-1, 0}, {1, 0}, {1, 1}},   // L left 270
      {{0, -1}, {0, 1}, {1, 1}},   // L right 000
      {{-1, 1}, {-1, 0}, {1, 0}},  // L right 090
      {{0, -1}, {-1, -1}, {0, 1}}, // L right 180
      {{-1, 0}, {1, 0}, {1, -1}},  // L right 270
      {{-1, 0}, {0, -1}, {1, 0}},  // pedestral 000
      {{0, -1}, {1, 0}, {0, 1}},   // pedestral 090
      {{-1, 0}, {0, 1}, {1, 0}},   // pedestral 180
      {{-1, 0}, {0, -1}, {0, 1}}   // pedestral 270
  };
  uint8_t playground[WIDTH][HEIGHT] = {0}; // playing array
  int8_t tile[4][2] = {0}; // storage for 4 block coordinates of current tile
  int8_t pos[2] = {0};     // position of pivot point of current tile
  uint8_t tile_nr = 0;     // type of current tile  (index)
  uint8_t rotation = 0;    // rotation of current tile (sub_index)
  Status status;           // game status
  Color color = RED;       // color value of the current tile
  uint8_t timer = 0;       // timer for slow ticks

  // update tile offsets
  void update_tile(const uint8_t tile_rot, const int8_t x, const int8_t y) {
    size_t _index = 0;
    switch (tile_nr) {
    case 0:
      _index = tile_rot % 2;
      break; // long piece
    case 1:
      _index = 2;
      break; // block
    case 2:
      _index = 3 + tile_rot % 2;
      break; // stairs up
    case 3:
      _index = 5 + tile_rot % 2;
      break; // stairs down
    case 4:
      _index = 7 + tile_rot % 4;
      break; // L left
    case 5:
      _index = 11 + tile_rot % 4;
      break; // L right
    case 6:
      _index = 15 + tile_rot % 4;
      break; // pedestral
    default:
      break;
    }
    this->tile[0][0] = x;
    tile[0][1] = y;
    for (size_t i = 0; i < 3; ++i) {
      this->tile[i + 1][0] = x + this->tiles[_index][i][0];
      this->tile[i + 1][1] = y + this->tiles[_index][i][1];
    }
  }

  bool check_collision() {
    for (size_t i = 0; i < 4; ++i) {
      int8_t _x = this->tile[i][0];
      int8_t _y = this->tile[i][1];

      // tiles are coming from above the screen, so don't check that
      const bool outside_bounds =
          (_x < 0 || _x >= (signed)WIDTH || _y >= (signed)HEIGHT);
      // we can only access dots inside the playground, so do the missing check
      // here
      if (outside_bounds || (_y >= 0 && this->playground[_x][_y] != 0)) {
        return true;
      }
    }
    return false;
  }

  // tries to rotate clockwise (1) or CCW (-1)
  void rotate(const int8_t dir) {
    update_tile((this->rotation + dir + 4) % 4, this->pos[0], this->pos[1]);
    if (check_collision()) {
      update_tile(this->rotation, this->pos[0], this->pos[1]);
    } else {
      this->rotation = (this->rotation + 4 + dir) % 4;
    }
  }

  // tries to move one block left (-1) or right (1)
  // resets to previous state if failed
  void move_x(const int8_t dir) {
    for (size_t i = 0; i < 4; ++i) {
      this->tile[i][0] += dir;
    }
    if (check_collision()) {
      for (size_t i = 0; i < 4; ++i) {
        this->tile[i][0] -= dir;
      }
    } else {
      this->pos[0] += dir;
    }
  }

  // tries to move one block down
  // returns false if it fails
  bool fall_down() {
    for (size_t i = 0; i < 4; ++i) {
      this->tile[i][1]++;
    }
    if (check_collision()) {
      for (size_t i = 0; i < 4; ++i) {
        this->tile[i][1]--;
      }
      return false;
    } else {
      this->pos[1]++;
      return true;
    }
  }

  // creates new tile
  void reset_tile() {
    this->pos[0] = WIDTH / 2;
    this->pos[1] = -1;
    this->tile_nr = static_cast<Color>(random(7));
    this->rotation = static_cast<Color>(random(4));
    this->color = static_cast<Color>(random(6) + 1);
    update_tile(rotation, this->pos[0], this->pos[1]);
  }

  // draw the tile to the screen
  // returns false, if tile sticks out of the screen
  bool apply_tile() {
    bool result = true;
    for (size_t i = 0; i < 4; ++i) {
      int8_t _x = this->tile[i][0];
      int8_t _y = this->tile[i][1];
      if (_y >= 0) {
        this->playground[_x][_y] = static_cast<uint8_t>(this->color);
      } else
        result = false;
    }
    return result;
  }

  // erases tile from the screen
  void unapply_tile() {
    for (size_t i = 0; i < 4; ++i) {
      int8_t _x = this->tile[i][0];
      int8_t _y = this->tile[i][1];
      if (_y >= 0) {
        this->playground[_x][_y] = 0;
      }
    }
  }

  // finds completed lines and marks them
  void check_lines() {
    for (size_t j = 0; j < HEIGHT; j++) {
      size_t i = 0;
      while (i < WIDTH - 1) {
        if (this->playground[i][j] == 0) {
          break;
        }
        ++i;
      }
      if (i == WIDTH - 1 && this->playground[i][j] != 0) {
        for (i = 0; i < WIDTH; ++i) {
          this->playground[i][j] = static_cast<uint8_t>(BLINK);
        }
      }
    }
  }

  // collapses marked lines
  void remove_lines() {
    for (size_t k = 0; k < HEIGHT; k++) {
      if (this->playground[0][k] == static_cast<uint8_t>(BLINK)) {
        for (size_t i = 0; i < WIDTH; ++i) {
          for (size_t j = k; j > 0; j--) {
            this->playground[i][j] = this->playground[i][j - 1];
          }
        }
        for (size_t i = 0; i < WIDTH; ++i) {
          this->playground[i][0] = 0;
        }
      }
    }
  }

public:
  // performs one game logic tick
  void tick() {
    if (this->status.pause) {
      return;
    }
    if (this->status.reset) {
      reset_tile();
      memset(&this->status, 0, sizeof(Status));
      memset(this->playground, 0, WIDTH * HEIGHT);
      this->timer = 0;
      return;
    }
    if (this->status.ending) {
      for (size_t j = 0; j < HEIGHT - 1; ++j) {
        if (this->playground[0][j] != static_cast<uint8_t>(BLINK) &&
            this->playground[0][j + 1] == static_cast<uint8_t>(BLINK)) {
          for (size_t i = 0; i < WIDTH; ++i) {
            this->playground[i][j] = static_cast<uint8_t>(BLINK);
          }
          break;
        }
      }
      return;
    }
    unapply_tile();
    if (this->status.rotCCW) {
      this->status.rotCCW = false;
      rotate(-1);
    }
    if (this->status.rotCW) {
      this->status.rotCW = false;
      rotate(1);
    }
    if (this->status.left) {
      this->status.left = false;
      move_x(-1);
    }
    if (this->status.right) {
      this->status.right = false;
      move_x(1);
    }
    const bool slow_tick = this->timer > 4;
    if (slow_tick) {
      this->timer = 0;
      remove_lines();
    }
    if (slow_tick || this->status.down) {
      this->status.down = false;
      if (!fall_down()) {             // cannot move tile down
        if (!apply_tile()) {          // tile sticks out of the top
          this->status.ending = true; // you lost!
          // fill bottom line (start animation)
          for (size_t i = 0; i < WIDTH; ++i) {
            this->playground[i][HEIGHT - 1] = static_cast<uint8_t>(BLINK);
          }
        } else { // tile placed - proceed with next one
          reset_tile();
          check_lines();
        }
      }
    }
    apply_tile();
    ++this->timer;
  }

  auto *get_playground() { return this->playground; }
  auto &get_status() { return this->status; }
};