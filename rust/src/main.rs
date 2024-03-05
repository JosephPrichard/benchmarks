use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::time::Instant;
use crate::AnyPuzzle::{EightPuzzle, FifteenPuzzle};
use crate::puzzle::{Puzzle};

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

fn run_puzzle<const N: usize>(puzzle: Puzzle<N>, times: &mut Vec<f64>) {
    let start = Instant::now();
    let solution = solver::find_path(puzzle);
    let elapsed = start.elapsed();

    times.push((elapsed.as_micros() as f64) / 1000f64);

    let mut steps = 0;
    for puzzle in solution {
        print!("{}", puzzle);
        steps += 1;
    }
    println!("Solved in {} steps\n", steps)
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        panic!("Needs at least 1 program argument - the input file containing puzzles")
    }

    let file = File::open(&args[1]).unwrap();
    let puzzles = read_puzzles(file);

    let times = &mut vec![];

    println!("Running solution for {} puzzle input(s)...\n\n", puzzles.len());

    // puzzle sizes are all known at compile time for the purpose of efficiency
    for (i, puzzle) in puzzles.iter().enumerate() {
        println!("Solution for puzzle {}", i+1);
        match puzzle {
            EightPuzzle(puzzle) => run_puzzle(puzzle.to_owned(), times),
            FifteenPuzzle(puzzle) => run_puzzle(puzzle.to_owned(), times)
        };
    }

    let mut total_time = 0f64;
    for (i, time) in times.iter().enumerate() {
        println!("Puzzle {} took {} ms to solve", i + 1, time);
        total_time += time
    }

    println!("Took {} ms in total", total_time)
}
