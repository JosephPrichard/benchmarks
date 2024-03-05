use std::hash::{Hash, Hasher};
use std::{fmt, ops};
use std::fmt::Formatter;

type Tile = u8;

#[derive(Copy, Clone)]
pub struct Position {
    row: i32,
    col: i32,
}

impl Position {
    fn of_index(i: usize, dim: i32) -> Self {
        Self { row: (i as i32) / dim, col: (i as i32) % dim }
    }

    fn to_index(&self, dim: i32) -> usize {
        (self.row * dim + self.col) as usize
    }

    fn in_bounds(&self) -> bool {
        self.row >= 0 && self.row < 3
            && self.col >= 0 && self.col < 3
    }
}

impl ops::Add<Position> for Position {
    type Output = Position;

    fn add(self, rhs: Position) -> Self::Output {
        Position { row: self.row + rhs.row, col: self.col + rhs.col }
    }
}

#[derive(Debug, Clone)]
pub struct Puzzle<const N: usize> {
    tiles: [Tile; N],
    action: &'static str,
}

static DIRECTIONS: [(Position, &'static str); 4] = [
    (Position {row: 0, col: 1}, "Right"),
    (Position {row: 0, col: -1}, "Left"),
    (Position {row: 1, col: 0}, "Down"),
    (Position {row: -1, col: 0}, "Up"),
];

const fn int_sqrt(n: i32) -> i32 {
    if n <= 1 {
        return n;
    }
    let mut x = n;
    let mut y = (x + 1) / 2;
    while y < x {
        x = y;
        y = (x + n / x) / 2;
    }
    x
}

impl<const N: usize> Puzzle<N> {

    const DIM: i32 = int_sqrt(N as i32);

    pub fn from_tiles(tiles: [Tile; N]) -> Self {
        Self { tiles, action: ""}
    }

    pub fn from_u8_slice(slice: &[u8]) -> Self {
        let mut tiles = [0; N];
        for (i, t) in slice.iter().enumerate() {
            if i >= tiles.len() {
                return Self::from_tiles(tiles)
            }
            tiles[i] = *t;
        }
        Self::from_tiles(tiles)
    }

    pub fn goal() -> Self {
        let mut tiles = [0; N];
        for i in 0..tiles.len() {
            tiles[i] = i as Tile;
        }
        Self::from_tiles(tiles)
    }

    fn find_zero(&self) -> Position {
        for i in 0..self.tiles.len() {
            if self.tiles[i] == 0 {
                return Position::of_index(i, Self::DIM);
            }
        }
        panic!("Puzzle doesn't contain a zero: this shouldn't happen")
    }

    pub fn heuristic(&self) -> i32 {
        let mut h = 0i32;
        for i in 0..self.tiles.len() {
            let pos1 = Position::of_index(i, Self::DIM);
            let pos2 = Position::of_index(self.tiles[i] as usize, Self::DIM);
            h += (pos2.row - pos1.row).abs() + (pos2.col - pos1.col).abs()
        }
        h
    }

    pub fn on_neighbors<F: FnMut(Self) -> ()>(&self, mut f: F) {
        let zero_pos = self.find_zero();
        for (d_pos, action) in DIRECTIONS {
            let new_pos = zero_pos + d_pos;
            if new_pos.in_bounds() {
                let mut new_puzzle = Self::from_tiles(self.tiles.clone());

                let temp = new_puzzle[new_pos];
                new_puzzle[new_pos] = new_puzzle[zero_pos];
                new_puzzle[zero_pos] = temp;
    
                new_puzzle.action = action;
    
                f(new_puzzle);
            }
        }
    }
}

impl<const N: usize> fmt::Display for Puzzle<N> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        let action = if self.action.is_empty() { "None" } else { self.action };
        writeln!(f, "{}", action)?;
        for i in 0..9 {
            write!(f, "{} ", self.tiles[i])?;
            if (i + 1) % 3 == 0 {
                writeln!(f)?;
            }
        }
        Ok(())
    }
}

impl<const N: usize> PartialEq<Self> for Puzzle<N> {
    fn eq(&self, other: &Self) -> bool {
        self.tiles == other.tiles
    }
}

impl<const N: usize> Eq for Puzzle<N> {}

impl<const N: usize> ops::Index<Position> for Puzzle<N> {
    type Output = Tile;

    fn index(&self, pos: Position) -> &Self::Output {
        &self.tiles[pos.to_index(Self::DIM)]
    }
}

impl<const N: usize> ops::IndexMut<Position> for Puzzle<N> {
    fn index_mut(&mut self, pos: Position) -> &mut Self::Output {
        &mut self.tiles[pos.to_index(Self::DIM)]
    }
}

impl<const N: usize> Hash for Puzzle<N> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.tiles.hash(state)
    }
}
