use std::cmp::Ordering;
use std::collections::{BinaryHeap, HashSet};
use bumpalo::Bump;
use crate::puzzle::Puzzle;

pub struct Node<'a, const N: usize> {
    puzzle: Puzzle<N>,
    prev: Option<&'a Node<'a, N>>,
    g: i32,
    f: i32,
}

impl<'a, const N: usize> PartialEq<Self> for Node<'a, N> {
    fn eq(&self, other: &Self) -> bool {
        self.f == other.f
    }
}

impl<'a, const N: usize> Eq for Node<'a, N> {}

impl<'a, const N: usize> PartialOrd for Node<'a, N> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        other.f.partial_cmp(&self.f)
    }
}

impl<'a, const N: usize> Ord for Node<'a, N> {
    fn cmp(&self, other: &Self) -> Ordering {
        other.f.cmp(&self.f)
    }
}

impl<'a, const N: usize> Node<'a, N> {
    fn new_root(puzzle: Puzzle<N>) -> Self {
        Self { puzzle, prev: None, g: 0, f: 0 }
    }

    fn new_child(puzzle: Puzzle<N>, prev: &'a Node<'a, N>) -> Self {
        let h = puzzle.heuristic();
        let g = prev.g + 1;
        Self {
            puzzle,
            prev: Some(prev),
            g,
            f: g + h,
        }
    }
}

pub fn reconstruct_path<const N: usize>(root: &Node<N>) -> Vec<Puzzle<N>> {
    let mut path = vec![];
    let mut curr = Some(root);
    while let Some(n) = curr {
        path.push(n.puzzle.clone());
        curr = n.prev;
    }
    path.reverse();
    path
}

pub fn find_path<const N: usize>(initial: Puzzle<N>) -> Vec<Puzzle<N>> {
    let arena = Bump::new();

    let mut visited: HashSet<&Puzzle<N>> = HashSet::new();
    let mut frontier: BinaryHeap<&Node<N>> = BinaryHeap::new();

    let root = Node::new_root(initial);
    frontier.push(&root);

    let goal = Puzzle::goal();

    while let Some(n) = frontier.pop() {
        if &n.puzzle == &goal {
            return reconstruct_path(n);
        }

        visited.insert(&n.puzzle);

        n.puzzle.on_neighbors(|puzzle: Puzzle<N>| {
            if !visited.contains(&puzzle) {
                let child = arena.alloc(Node::new_child(puzzle, n));
                frontier.push(child);
            }
        })
    }

    vec![]
}