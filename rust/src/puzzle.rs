use std::cmp::Ordering;
use std::collections::{BinaryHeap, HashSet};
use std::hash::{Hash, Hasher};
use std::{fmt, ops};
use std::fmt::Formatter;
use bumpalo::Bump;

type Tile = u8;

#[derive(Copy, Clone)]
pub struct Position {
    row: i32,
    col: i32,
}

impl Position {
    fn of_index(i: usize) -> Self {
        Self { row: (i as i32) / 3, col: (i as i32) % 3 }
    }

    fn to_index(&self) -> usize {
        (self.row * 3 + self.col) as usize
    }

    fn in_bounds(&self) -> bool {
        self.row >= 0 && self.row < 3
            && self.col >= 0 && self.col < 3
    }
}

impl ops::Add<Position> for Position {
    type Output = Position;

    fn add(self, rhs: Position) -> Self::Output {
        Position { row: self.row + rhs.row, col: self.col + self.col }
    }
}

pub type Tiles = [Tile; 9];

#[derive(Debug, Copy, Clone)]
pub struct Puzzle {
    tiles: Tiles,
    action: &'static str,
}

static DIRECTIONS: [(Position, &'static str); 4] = [
    (Position {row: 0, col: 1}, "Left"),
    (Position {row: 0, col: -1}, "Right"),
    (Position {row: 1, col: 0}, "Up"),
    (Position {row: -1, col: 0}, "Down"),
];

impl Puzzle {
    pub fn new() -> Self {
        Self { tiles: [0; 9], action: "" }
    }

    pub fn from_tiles(tiles: Tiles) -> Self {
        Self { tiles, action: "" }
    }

    pub fn from_u8_slice(slice: &[u8]) -> Self {
        let mut tiles = [0; 9];
        for (i, t) in slice.iter().enumerate() {
            if i >= 9 {
                return Self::from_tiles(tiles)
            }
            tiles[i] = *t;
        }
        Self::from_tiles(tiles)
    }

    fn goal() -> Self {
        let mut tiles: Tiles = [0; 9];
        for i in 0..9 {
            tiles[i] = i as Tile;
        }
        Self::from_tiles(tiles)
    }

    fn find_zero(&self) -> Position {
        for i in 0..9 {
            if self.tiles[i] == 0 {
                return Position::of_index(i);
            }
        }
        panic!("Puzzle doesn't contain a zero: this shouldn't happen")
    }

    fn heuristic(&self) -> i32 {
        let mut h = 0i32;
        for i in 0..9 {
            let pos1 = Position::of_index(i);
            let pos2 = Position::of_index(self.tiles[i] as usize);
            h += (pos2.row - pos1.row).abs() + (pos2.col - pos1.col).abs()
        }
        h
    }

    fn on_neighbors<F: FnMut(Self) -> ()>(&self, mut f: F) {
        let zero_pos = self.find_zero();
        for (d_pos, action) in DIRECTIONS {
            let new_pos = zero_pos + d_pos;
            if new_pos.in_bounds() {
                let mut new_puzzle = Self::from_tiles(self.tiles);

                let temp = new_puzzle[new_pos];
                new_puzzle[new_pos] = new_puzzle[zero_pos];
                new_puzzle[zero_pos] = temp;
    
                new_puzzle.action = action;
    
                f(new_puzzle);
            }
        }
    }
}

impl fmt::Display for Puzzle {
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

impl PartialEq<Self> for Puzzle {
    fn eq(&self, other: &Self) -> bool {
        self.tiles == other.tiles
    }
}

impl Eq for Puzzle {}

impl ops::Index<Position> for Puzzle {
    type Output = Tile;

    fn index(&self, pos: Position) -> &Self::Output {
        &self.tiles[pos.to_index()]
    }
}

impl ops::IndexMut<Position> for Puzzle {
    fn index_mut(&mut self, pos: Position) -> &mut Self::Output {
        &mut self.tiles[pos.to_index()]
    }
}

impl Hash for Puzzle {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.tiles.hash(state)
    }
}

pub struct Node<'a> {
    puzzle: Puzzle,
    prev: Option<&'a Node<'a>>,
    g: i32,
    f: i32,
}

impl<'a> PartialEq<Self> for Node<'a> {
    fn eq(&self, other: &Self) -> bool {
        self.f == other.f
    }
}

impl<'a> Eq for Node<'a> {}

impl<'a> PartialOrd for Node<'a> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        self.f.partial_cmp(&other.f)
    }
}

impl<'a> Ord for Node<'a> {
    fn cmp(&self, other: &Self) -> Ordering {
        self.f.cmp(&other.f)
    }
}

impl<'a> Node<'a> {
    fn new_root(puzzle: Puzzle) -> Self {
        Self { puzzle, prev: None, g: 0, f: 0 }
    }

    fn new_child(puzzle: Puzzle, prev: &'a Node<'a>) -> Self {
        Self {
            puzzle,
            prev: Some(prev),
            g: prev.g,
            f: prev.g + puzzle.heuristic(),
        }
    }
}

pub fn reconstruct_path(root: &Node) -> Vec<Puzzle> {
    let mut path = vec![];
    let mut curr = Some(root);
    while let Some(n) = curr {
        path.push(n.puzzle);
        curr = n.prev;
    }
    path.reverse();
    path
}

pub fn find_path(initial: Puzzle) -> Vec<Puzzle> {
    let arena = Bump::new();

    let mut visited: HashSet<Puzzle> = HashSet::new();
    let mut frontier: BinaryHeap<&Node> = BinaryHeap::new();

    let root = Node::new_root(initial);
    frontier.push(&root);

    let goal = Puzzle::goal();

    while let Some(n) = frontier.pop() {
        if &n.puzzle == &goal {
            return reconstruct_path(n);
        }

        visited.insert(n.puzzle);

        n.puzzle.on_neighbors(|puzzle: Puzzle| {
            if !visited.contains(&puzzle) {
                let child = arena.alloc(Node::new_child(puzzle, n));
                frontier.push(child);
            }
        })
    }

    vec![]
}
