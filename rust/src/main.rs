use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use crate::puzzle::{find_path, Puzzle};

mod puzzle;

fn read_puzzles(file: File) -> Vec<Puzzle> {
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
            let puzzle = Puzzle::from_u8_slice(&curr_tiles);
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

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        panic!("Needs at least 1 program argument - the input file containing puzzles")
    }

    let file = File::open(&args[1]).unwrap();
    let puzzles = read_puzzles(file);

    for puzzle in puzzles {
        let solution = find_path(puzzle);
        println!("{:?}", solution)
        // for puzzle in solution {
        //     println!("{}", puzzle);
        // }
        // println!()
    }
}
