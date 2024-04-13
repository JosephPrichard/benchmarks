use std::sync::Arc;
use std::{env, thread};
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::time::Instant;
use semaphore::Semaphore;

use crate::AnyPuzzle::{EightPuzzle, FifteenPuzzle};
use crate::AnySolution::{EightSolution, FifteenSolution};
use crate::puzzle::Puzzle;

mod puzzle;
mod solver;
mod semaphore;

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

fn run_puzzle<const N: usize>(puzzle: Puzzle<N>, mem_flag: MemFlag) -> (f64, u32, Vec<Puzzle<N>>) {
    let find_path = match mem_flag {
        MemFlag::Arena => solver::SolverArena::find_path::<N>,
        MemFlag::Rc => solver::SolverRc::find_path::<N>
    };

    let start = Instant::now();
    let (solution, nodes) = find_path(puzzle);
    let elapsed = start.elapsed();

    let time = (elapsed.as_micros() as f64) / 1000f64;
    
    (time, nodes, solution)
}

enum AnySolution {
    EightSolution(Vec<Puzzle<9>>),
    FifteenSolution(Vec<Puzzle<16>>),
}

fn run_any_puzzle(puzzle: AnyPuzzle, flag: MemFlag) -> (f64, u32, AnySolution) {
    match puzzle {
        EightPuzzle(puzzle) => {
            let (time, nodes, solution) = run_puzzle(puzzle.to_owned(), flag);
            (time, nodes, EightSolution(solution))
        }
        FifteenPuzzle(puzzle) => {
            let (time, nodes, solution) = run_puzzle(puzzle.to_owned(), flag);
            (time, nodes, FifteenSolution(solution))
        }
    }
}

fn run_puzzles(puzzles: Vec<AnyPuzzle>, mem_flag: MemFlag) -> Vec<(f64, u32, AnySolution)> {
    let mut solutions = vec![];
    for puzzle in puzzles {
        solutions.push(run_any_puzzle(puzzle, mem_flag));
    }
    return solutions;
}

fn run_puzzles_parallel(puzzles: Vec<AnyPuzzle>, mem_flag: MemFlag) -> Vec<(f64, u32, AnySolution)> {
    let mut threads = vec![];
    let mut solutions = vec![];

    let sem = Arc::new(Semaphore::with_core_count());

    for puzzle in puzzles {
        let sem = sem.clone();
        let thread = thread::spawn(move || {
            sem.down_one();
            let solution = run_any_puzzle(puzzle, mem_flag);
            sem.up_one();
            solution
        });
        threads.push(thread);
    }

    for thread in threads {
        solutions.push(thread.join().unwrap());
    }

    return solutions;
}

fn print_solution<const N: usize>(solution: &Vec<Puzzle<N>>, nodes: u32) {
    for puzzle in solution {
        println!("{}", puzzle.action);
    }
    println!("Solved in {} steps, expanded {} nodes\n", solution.len() - 1, nodes);
}

#[derive(Copy, Clone)]
enum MemFlag {
    Arena, Rc,
}

#[derive(Copy, Clone)]
enum ParFlag {
    Sequential, Parallel
}

fn read_flags(args: &Vec<String>) -> (MemFlag, ParFlag) {
    let mem_flag = if args.len() < 3 { 
        MemFlag::Arena
    } else {
        match args[2].as_str() {
            "arena" => MemFlag::Arena,
            "rc" => MemFlag::Rc,
            flag => panic!("Invalid flag: {} - must be 'arena' or 'rc'", flag)
        }
    };

    let par_flag = if args.len() < 4 { 
        ParFlag::Sequential
    } else {
        match args[3].as_str() {
            "par" => ParFlag::Parallel,
            "seq" => ParFlag::Sequential,
            flag => panic!("Invalid flag: {} - must be 'par' or 'seq'", flag)
        }
    };

    return (mem_flag, par_flag)
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        panic!("Need at least 1 program argument")
    }

    let (mem_flag, par_flag) = read_flags(&args);
    let file = File::open(&args[1]).unwrap();

    let puzzles = read_puzzles(file);

    let start = Instant::now();

    let solutions = match par_flag {
        ParFlag::Sequential => run_puzzles(puzzles, mem_flag),
        ParFlag::Parallel => run_puzzles_parallel(puzzles, mem_flag),
    };

    let elapsed = start.elapsed();
    let ete_time = (elapsed.as_micros() as f64) / 1000f64;
    
    for (i, (_, nodes, solution)) in solutions.iter().enumerate() {
        println!("Solution for puzzle {}", (i + 1));
        match solution {
            EightSolution(solution) => print_solution(solution, *nodes),
            FifteenSolution(solution) => print_solution(solution, *nodes),
        }
    }

    let mut total_time = 0f64;
    let mut total_nodes = 0;
    for (i, (time, nodes, _)) in solutions.iter().enumerate() {
        println!("Puzzle {}: {} ms, {} nodes", i + 1, time, nodes);
        total_time += time;
        total_nodes += nodes;
    }

    println!("\nTotal: {} ms, {} nodes", total_time, total_nodes);

    println!("End-to-end time: {} ms", ete_time)
}
