#include <fstream>
#include <array>

struct Position {
    int row;
    int col;
};

Position operator+(Position lhs, Position rhs) {
    return Position{lhs.row + rhs.row, lhs.col + rhs.col};
}

bool in_bounds(Position pos, int n) {
    return pos.row >= 0 && pos.row < n && pos.col >= 0 && pos.col < n;
}

Position pos_of_index(int index, int n) {
    return Position{index / (int) n, index % (int) n};
}

int pos_to_index(Position pos, int n) {
    return pos.row * n + pos.col;
}

enum Action {
    NONE, LEFT, DOWN, UP, RIGHT
};

void print_action(Action a) {
    switch (a) {
        case LEFT:
            printf("Left\n");
            break;
        case RIGHT:
            printf("Right\n");
            break;
        case DOWN:
            printf("Down\n");
            break;
        case UP:
            printf("Up\n");
            break;
        default:
            printf("Start\n");
            break;
    }
}
struct Direction {
    Position vector;
    Action action;
};

const std::array<Direction, 4>& get_directions() {
    const static std::array<Direction, 4> DIRECTIONS = {
        Direction{Position{0, -1}, LEFT},
        Direction{Position{-1, 0}, UP},
        Direction{Position{1, 0}, DOWN},
        Direction{Position{0, 1}, RIGHT}
    };
    return DIRECTIONS;
}