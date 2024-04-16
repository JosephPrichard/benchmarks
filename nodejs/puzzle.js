const fs = require('fs');
const Heap = require('heap');

const ACTIONS = ["Start", "Up", "Down", "Left", "Right"]

const DIRECTIONS = [
    { row: 1, col: 0 },
    { row: -1, col: 0 },
    { row: 0, col: 1 },
    { row: 0, col: -1 }
];

function newPuzzle(tiles) {
    return { 
        g: 0,
        f: 0,
        prev: null,
        tiles: tiles,
        action: 0
    };
}

function tilesEquals(tiles, other) {
    for (let i = 0; i < tiles.length; i++) {
        if (tiles[i] !== other[i]) {
            return false;
        }
    }
    return true;
}

function heuristic(tiles, dim) {
    let h = 0;
    for (let i = 0; i < tiles.length; i++) {
        const t = tiles[i];
        if (t != 0) {
            const row1 = Math.floor(i / dim);
            const col1 = i % dim;
            const row2 = Math.floor(t / dim);
            const col2 = t % dim;
            h += Math.abs(row2 - row1) + Math.abs(col2 - col1);
        }
    }
    return h;
}

function findZero(tiles) {
    for (let i = 0; i < tiles.length; i++) {
        if (tiles[i] === 0) {
            return i;
        }
    }
    throw new Error("Puzzles should contain a 0 tile");
}

function tilesToString(tiles) {
    return tiles.join('');
}

function printString(dim) {
    let s = "";
    for (let i = 0; i < tiles.length; i++) {
        if (tiles[i] === 0) {
            s += "  ";
        } else {
            s += tiles[i] + " ";
        }
        if ((i + 1) % dim === 0) {
            s += "\n";
        }
    }
    return s
}

function inBounds(row, col, dim) {
    return row >= 0 && row < dim && col >= 0 && col < dim;
}

function createGoal(size) {
    const tiles = [];
    for (let i = 0; i < size; i++) {
        tiles.push(i);
    }
    return tiles;
}

function createGoal(size) {
    const tiles = [];
    for (let i = 0; i < size; i++) {
        tiles.push(i);
    }
    return tiles;
}


function reconstructPath(curr) {
    const path = [];
    while (curr !== null) {
        path.push(curr);
        curr = curr.prev;
    }
    path.reverse();
    return path;
}

function findPath(initial) {
    const dim = Math.floor(Math.sqrt(initial.tiles.length));

    const visited = new Map();
    const frontier = new Heap((p1, p2) => p1.f - p2.f);
    frontier.push(initial);

    const goal = createGoal(initial.tiles.length);

    let nodes = 0;
    while (!frontier.empty()) {
        const currPuzzle = frontier.pop();
        nodes++;

        if (tilesEquals(currPuzzle.tiles, goal)) {
            return { path: reconstructPath(currPuzzle), nodes };
        }

        const tilesStr = tilesToString(currPuzzle.tiles);
        visited.set(tilesStr, true);

        const zeroIndex = findZero(currPuzzle.tiles);
        const zeroRow = Math.floor(zeroIndex / dim);
        const zeroCol = zeroIndex % dim;

        for (let i = 0; i < DIRECTIONS.length; i++) {
            const {row, col} = DIRECTIONS[i];

            const nextRow = zeroRow + row;
            const nextCol = zeroCol + col;

            if (!inBounds(nextRow, nextCol, dim)) {
                continue;
            }

            const nextIndex = nextRow * dim + nextCol;
            const nextPuzzle = newPuzzle([...currPuzzle.tiles]);

            const temp = nextPuzzle.tiles[zeroIndex];
            nextPuzzle.tiles[zeroIndex] = nextPuzzle.tiles[nextIndex];
            nextPuzzle.tiles[nextIndex] = temp;

            nextPuzzle.prev = currPuzzle;
            nextPuzzle.action = i + 1;
            nextPuzzle.g = currPuzzle.g + 1;
            nextPuzzle.f = nextPuzzle.g + heuristic(nextPuzzle.tiles, dim);

            const tilesStr = tilesToString(nextPuzzle.tiles);
            if (!visited.get(tilesStr)) {
                frontier.push(nextPuzzle);
            }
        }
    }

    return { path: [], nodes };
}

function readPuzzles(s) {
    const puzzles = [];
    let currTiles = [];

    const lines = s.split("\n");
    for (const line of lines) {
        const tokens = line.split(" ").map(t => t.trim()).filter((t) => t != "");

        if (tokens.length > 1) {
            for (const token of tokens) {
                currTiles.push(parseInt(token));
            }
        } else {
            if (currTiles.length > 0) {
                puzzles.push(newPuzzle(currTiles));
                currTiles = [];
            }
        }
    }
    return puzzles;
}

function runPuzzles(puzzles) {
    const solutions = [];
    for (const puzzle of puzzles) {
        const start = process.hrtime.bigint();

        const { path, nodes } = findPath(puzzle);

        const end = process.hrtime.bigint();
        const time = Number(end - start) / 1e6;

        solutions.push({path, time, nodes});
    }
    return solutions;
}

function main() {
    if (process.argv.length < 3) {
        console.log("Need at least 1 program argument");
        process.exit(1);
    }

    const path = process.argv[2];

    let fileContents = "";
    try {
        fileContents = fs.readFileSync(path, "utf8");
    } catch (err) {
        console.log("Failed to read input file " + path)
        process.exit(1);
    }

    const puzzles = readPuzzles(fileContents);
    const solutions = runPuzzles(puzzles);

    for (const sol of solutions) {
        for (const p of sol.path) {
            console.log(ACTIONS[p.action]);
        }
        console.log(`Solved in ${sol.path.length - 1} steps\n`);
    }

    let totalTime = 0;
    let totalNodes = 0;
    for (let i = 0; i < solutions.length; i++) {
        console.log(`Puzzle ${i + 1}: ${solutions[i].time} ms, ${solutions[i].nodes} nodes`);
        totalTime += solutions[i].time;
        totalNodes += solutions[i].nodes;
    }

    console.log(`\nTotal: ${totalTime} ms, ${totalNodes} nodes`);
    console.log(`End-to-end: ${totalTime} ms`);
}

main();
