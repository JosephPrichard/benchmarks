use std::env;
use std::fs::{File, OpenOptions};
use std::io::{BufRead, BufReader, BufWriter, Write};
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

fn run_puzzle<const N: usize>(puzzle: Puzzle<N>, times: &mut Vec<f64>, w: &mut Option<BufWriter<File>>, i: usize) -> u32 {
    let start = Instant::now();
    let (solution, nodes) = solver::find_path_arena(puzzle);
    let elapsed = start.elapsed();

    times.push((elapsed.as_micros() as f64) / 1000f64);

    println!("Solution for puzzle {}", (i + 1));
    for puzzle in &solution {
        print!("{}", puzzle);
    }

    if let Some(w) = w {
        w.write(format!("{} steps\n", solution.len() - 1).as_bytes()).unwrap();
    }
    println!("Solved in {} steps, expanded {} nodes\n", solution.len() - 1, nodes);
    nodes
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        panic!("Need at least 1 program argument")
    }

    let file = File::open(&args[1]).unwrap();

    let outb_writer = &mut if args.len() >= 3 {
        let file = OpenOptions::new()
            .write(true)
            .create(true)
            .truncate(true)
            .open(&args[2])
            .unwrap();
        Some(BufWriter::new(file))
    } else {
        None
    };

    let out_writer = &mut if args.len() >= 4 {
        let file = OpenOptions::new()
            .write(true)
            .create(true)
            .truncate(true)
            .open(&args[3])
            .unwrap();
        Some(BufWriter::new(file))
    } else {
        None
    };

    let puzzles = read_puzzles(file);

    let mut total_nodes = 0;
    let times = &mut vec![];

    println!("Running solution for {} puzzle input(s)...\n", puzzles.len());

    // puzzle sizes are all known at compile time for the purpose of efficiency
    for (i, puzzle) in puzzles.iter().enumerate() {
        println!("Solution for puzzle {}", i+1);
        total_nodes += match puzzle {
            EightPuzzle(puzzle) => run_puzzle(puzzle.to_owned(), times, out_writer, i),
            FifteenPuzzle(puzzle) => run_puzzle(puzzle.to_owned(), times, out_writer, i)
        };
    }

    let mut total_time = 0f64;
    for (i, time) in times.iter().enumerate() {
        println!("Puzzle {} took {} ms to solve", i + 1, time);
        total_time += time;
        if let Some(w) = outb_writer {
            w.write(format!("{}, {}\n", (i+1), time).as_bytes()).unwrap();
        }
    }

    println!("Took {} ms in total, expanded {} nodes in total", total_time, total_nodes);
    if let Some(w) = outb_writer {
        w.write(format!("total, {}", total_time).as_bytes()).unwrap();
    }
}
