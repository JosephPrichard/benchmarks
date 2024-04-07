use std::cmp::Ordering;
use std::collections::{BinaryHeap, HashSet};
use std::rc::Rc;
use bumpalo::Bump;
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

pub struct SolverArena {}

impl SolverArena {
    fn reconstruct_path<const N: usize>(root: &Node<N>) -> Vec<Puzzle<N>> {
        let mut path = vec![];
        let mut curr = Some(root);
        while let Some(n) = curr {
            path.push(n.puzzle.clone());
            curr = n.prev;
        }
        path.reverse();
        path
    }

    pub fn find_path<const N: usize>(initial: Puzzle<N>) -> (Vec<Puzzle<N>>, u32) {
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
                return (Self::reconstruct_path(n), nodes);
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
}

pub struct NodeRc<const N: usize> {
    prev: Option<Rc<NodeRc<N>>>,
    puzzle: Puzzle<N>,
    g: i32,
    f: i32,
}

impl <const N: usize> PartialEq<Self> for NodeRc<N> {
    fn eq(&self, other: &Self) -> bool {
        self.f == other.f
    }
}

impl <const N: usize> Eq for NodeRc<N> {}

impl <const N: usize> PartialOrd for NodeRc<N> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        other.f.partial_cmp(&self.f)
    }
}

impl <const N: usize> Ord for NodeRc<N> {
    fn cmp(&self, other: &Self) -> Ordering {
        other.f.cmp(&self.f)
    }
}

impl<const N: usize> NodeRc<N> {
    fn new_root(puzzle: Puzzle<N>) -> Self {
        Self { puzzle, prev: None, g: 0, f: 0 }
    }

    fn new_child(puzzle: Puzzle<N>, prev: Rc<NodeRc<N>>) -> Self {
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

pub struct SolverRc {}

impl SolverRc {
    fn reconstruct_path<const N: usize>(root: Rc<NodeRc<N>>) -> Vec<Puzzle<N>> {
        let mut path = vec![];
        let mut curr = Some(root);
        while let Some(n) = curr {
            path.push(n.puzzle.clone());
            curr = n.prev.clone();
        }
        path.reverse();
        path
    }

    pub fn find_path<const N: usize>(initial: Puzzle<N>) -> (Vec<Puzzle<N>>, u32) {
        let mut visited: HashSet<u64> = HashSet::new();
        let mut frontier: BinaryHeap<Rc<NodeRc<N>>> = BinaryHeap::new();
    
        let root = NodeRc::new_root(initial);
        frontier.push(Rc::new(root));
    
        let goal = Puzzle::goal();
    
        let mut nodes = 0;
        while let Some(n) = frontier.pop() {
            nodes += 1;
    
            if &n.puzzle == &goal {
                return (Self::reconstruct_path(n), nodes);
            }
    
            visited.insert(n.puzzle.hash_u64());
    
            n.puzzle.on_neighbors(|puzzle: Puzzle<N>| {
                let hash = puzzle.hash_u64();
                if !visited.contains(&hash) {
                    let child = NodeRc::new_child(puzzle, n.clone());
                    frontier.push(Rc::new(child));
                }
            })
        }
    
        (vec![], nodes)
    }
}

pub struct SolverSlow {}

// a "dumb" implementation to show how useful our fancy data structures are...
impl SolverSlow {
    pub fn find_path<const N: usize>(initial: Puzzle<N>) -> (Vec<Puzzle<N>>, u32) {
        let mut visited: Vec<u64> = vec![];
        let mut frontier: Vec<Rc<NodeRc<N>>> = vec![];

        let root = NodeRc::new_root(initial);
        frontier.push(Rc::new(root));

        let goal = Puzzle::goal();

        let mut nodes = 0;
        while let Some(n) = frontier.pop() {
            nodes += 1;

            if &n.puzzle == &goal {
                return (SolverRc::reconstruct_path(n), nodes);
            }

            visited.push(n.puzzle.hash_u64());

            n.puzzle.on_neighbors(|puzzle: Puzzle<N>| {
                let mut is_visited = false;
                let hash = puzzle.hash_u64();
                for h in &visited {
                    if *h == hash {
                        is_visited = true;
                    }
                }
                if !is_visited {
                    let child = NodeRc::new_child(puzzle, n.clone());
                    let mut ins_index = None;
                    for (i, n) in frontier.iter().enumerate().rev() {
                        if child.f < n.f {
                            ins_index = Some(i + 1);
                            break
                        }
                    }
                    match ins_index {
                        None => frontier.insert(0, Rc::new(child)),
                        Some(i) => frontier.insert(i, Rc::new(child))
                    }
                }
            })
        }

        (vec![], nodes)
    }
}