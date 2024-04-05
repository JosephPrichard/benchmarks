use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::time::Instant;
use crate::AnyPuzzle::{EightPuzzle, FifteenPuzzle};
use crate::puzzle::Puzzle;

mod puzzle;
mod solver;

enum AnyPuzzle {
    EightPuzzle(Puzzle<9>),
    FifteenPuzzle(Puzzle<16>),
}

fn read_puzzles(file: File) -> Vec<AnyPuzzle> {
    let mut puzzles = vec![];
    let mut curr_tiles = Vec::new();

    let reader = BufReader::new(file);
    for line in reader.lines() {
        let line = line.unwrap();

        let mut tokens = vec![];
        for token in line.split(" ") {
            match token.parse::<u8>() {
                Ok(t) => tokens.push(t),
                _ => ()
            }
        }

        if tokens.is_empty() {
            let puzzle = match curr_tiles.len() {
                16 => FifteenPuzzle(Puzzle::from_u8_slice(&curr_tiles)),
                9 => EightPuzzle(Puzzle::from_u8_slice(&curr_tiles)),
                _ => panic!("A puzzle must be size 16 or 9")
            };
            puzzles.push(puzzle);
            curr_tiles.clear()
        } else {
            for token in tokens {
                curr_tiles.push(token);
            }
        }
    }

    puzzles
}

fn run_puzzle<const N: usize>(puzzle: Puzzle<N>, flag: &str, i: usize) -> (f64, u32) {
    let find_path = match flag {
        "arena" => solver::SolverArena::find_path::<N>,
        "rc" => solver::SolverRc::find_path::<N>,
        flag => panic!("Invalid flag: {} - must be 'arena' or 'rc'", flag)
    };

    let start = Instant::now();
    let (solution, nodes) = find_path(puzzle);
    let elapsed = start.elapsed();

    let time = (elapsed.as_micros() as f64) / 1000f64;

    println!("Solution for puzzle {}", (i + 1));
    for puzzle in &solution {
        print!("{}", puzzle);
    }

    println!("Solved in {} steps, expanded {} nodes\n", solution.len() - 1, nodes);
    
    (time, nodes)
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        panic!("Need at least 1 program argument")
    }

    let mut flag = "arena";
    if args.len() >= 3 {
        flag = &args[2];
    }

    let file = File::open(&args[1]).unwrap();

    let puzzles = read_puzzles(file);
    let results = &mut vec![];
    // puzzle sizes are all known at compile time for the purpose of efficiency
    for (i, puzzle) in puzzles.iter().enumerate() {
        println!("Solution for puzzle {}", i+1);
        match puzzle {
            EightPuzzle(puzzle) => {
                let result = run_puzzle(puzzle.to_owned(), flag, i);
                results.push(result)
            }
            FifteenPuzzle(puzzle) => {
                let result = run_puzzle(puzzle.to_owned(), flag, i);
                results.push(result)
            }
        };
    }

    let mut total_time = 0f64;
    let mut total_nodes = 0;
    for (i, (time, nodes)) in results.iter().enumerate() {
        println!("Puzzle {}: {} ms, {} nodes", i + 1, time, nodes);
        total_time += time;
        total_nodes += nodes;
    }

    println!("Total: {} ms, {} nodes", total_time, total_nodes);
}
