use std::cmp::Ordering;
use std::collections::{BinaryHeap, HashSet};
use std::rc::Rc;
use bumpalo::Bump;
use gc::{Finalize, Gc, Trace};
use crate::puzzle::Puzzle;

pub struct Node<'a, const N: usize> {
    prev: Option<&'a Node<'a, N>>,
    puzzle: Puzzle<N>,
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

pub fn find_path_arena<const N: usize>(initial: Puzzle<N>) -> (Vec<Puzzle<N>>, u32) {
    let arena = Bump::new();

    let mut visited: HashSet<&Puzzle<N>> = HashSet::new();
    let mut frontier: BinaryHeap<&Node<N>> = BinaryHeap::new();

    let root = Node::new_root(initial);
    frontier.push(&root);

    let goal = Puzzle::goal();

    let mut nodes = 0;
    while let Some(n) = frontier.pop() {
        nodes += 1;

        if &n.puzzle == &goal {
            return (reconstruct_path(n), nodes);
        }

        visited.insert(&n.puzzle);

        n.puzzle.on_neighbors(|puzzle: Puzzle<N>| {
            if !visited.contains(&puzzle) {
                let child = arena.alloc(Node::new_child(puzzle, n));
                frontier.push(child);
            }
        })
    }

    (vec![], nodes)
}

pub struct CountedNode<const N: usize> {
    prev: Option<Rc<CountedNode<N>>>,
    puzzle: Puzzle<N>,
    g: i32,
    f: i32,
}

impl <const N: usize> PartialEq<Self> for CountedNode<N> {
    fn eq(&self, other: &Self) -> bool {
        self.f == other.f
    }
}

impl <const N: usize> Eq for CountedNode<N> {}

impl <const N: usize> PartialOrd for CountedNode<N> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        other.f.partial_cmp(&self.f)
    }
}

impl <const N: usize> Ord for CountedNode<N> {
    fn cmp(&self, other: &Self) -> Ordering {
        other.f.cmp(&self.f)
    }
}

impl<const N: usize> CountedNode<N> {
    fn new_root(puzzle: Puzzle<N>) -> Self {
        Self { puzzle, prev: None, g: 0, f: 0 }
    }

    fn new_child(puzzle: Puzzle<N>, prev: Rc<CountedNode<N>>) -> Self {
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

pub fn reconstruct_path_rc<const N: usize>(root: Rc<CountedNode<N>>) -> Vec<Puzzle<N>> {
    let mut path = vec![];
    let mut curr = Some(root);
    while let Some(n) = curr {
        path.push(n.puzzle.clone());
        curr = n.prev.clone();
    }
    path.reverse();
    path
}

pub fn find_path_rc<const N: usize>(initial: Puzzle<N>) -> (Vec<Puzzle<N>>, u32) {
    let mut visited: HashSet<u64> = HashSet::new();
    let mut frontier: BinaryHeap<Rc<CountedNode<N>>> = BinaryHeap::new();

    let root = CountedNode::new_root(initial);
    frontier.push(Rc::new(root));

    let goal = Puzzle::goal();

    let mut nodes = 0;
    while let Some(n) = frontier.pop() {
        nodes += 1;

        if &n.puzzle == &goal {
            return (reconstruct_path_rc(n), nodes);
        }

        visited.insert(n.puzzle.hash_u64());

        n.puzzle.on_neighbors(|puzzle: Puzzle<N>| {
            let hash = puzzle.hash_u64();
            if !visited.contains(&hash) {
                let child = CountedNode::new_child(puzzle, n.clone());
                frontier.push(Rc::new(child));
            }
        })
    }

    (vec![], nodes)
}

#[derive(Trace, Finalize)]
pub struct CollNode<const N: usize> {
    prev: Option<Gc<CollNode<N>>>,
    puzzle: Puzzle<N>,
    g: i32,
    f: i32,
}

impl <const N: usize> PartialEq<Self> for CollNode<N> {
    fn eq(&self, other: &Self) -> bool {
        self.f == other.f
    }
}

impl <const N: usize> Eq for CollNode<N> {}

impl <const N: usize> PartialOrd for CollNode<N> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        other.f.partial_cmp(&self.f)
    }
}

impl <const N: usize> Ord for CollNode<N> {
    fn cmp(&self, other: &Self) -> Ordering {
        other.f.cmp(&self.f)
    }
}

impl<const N: usize> CollNode<N> {
    fn new_root(puzzle: Puzzle<N>) -> Self {
        Self { puzzle, prev: None, g: 0, f: 0 }
    }

    fn new_child(puzzle: Puzzle<N>, prev: Gc<CollNode<N>>) -> Self {
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

pub fn reconstruct_path_gc<const N: usize>(root: Gc<CollNode<N>>) -> Vec<Puzzle<N>> {
    let mut path = vec![];
    let mut curr = Some(root);
    while let Some(n) = curr {
        path.push(n.puzzle.clone());
        curr = n.prev.clone();
    }
    path.reverse();
    path
}

pub fn find_path_gc<const N: usize>(initial: Puzzle<N>) -> (Vec<Puzzle<N>>, u32) {
    let mut visited: HashSet<u64> = HashSet::new();
    let mut frontier: BinaryHeap<Gc<CollNode<N>>> = BinaryHeap::new();

    let root = CollNode::new_root(initial);
    frontier.push(Gc::new(root));

    let goal = Puzzle::goal();

    let mut nodes = 0;
    while let Some(n) = frontier.pop() {
        nodes += 1;

        if &n.puzzle == &goal {
            return (reconstruct_path_gc(n), nodes);
        }

        visited.insert(n.puzzle.hash_u64());

        n.puzzle.on_neighbors(|puzzle: Puzzle<N>| {
            let hash = puzzle.hash_u64();
            if !visited.contains(&hash) {
                let child = CollNode::new_child(puzzle, n.clone());
                frontier.push(Gc::new(child));
            }
        })
    }

    (vec![], nodes)
}
