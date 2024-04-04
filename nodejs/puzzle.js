const fs = require('fs');

function index_of_pos(pos, dim) {
    return pos.row * dim + pos.col;
}

function pos_of_index(i, dim) {
    return { row: Math.floor(i / dim), col: i % dim };
}

function in_bounds(pos, dim) {
    return pos.row >= 0 && pos.row < dim && pos.col >= 0 && pos.col < dim;
}

function add_positions(pos1, pos2) {
    return { row: pos1.row + pos2.row, col: pos1.col + pos2.col };
}

const directions = [
    { dir: { row: 1, col: 0 }, action: "Up" },
    { dir: { row: -1, col: 0 }, action: "Down" },
    { dir: { row: 0, col: 1 }, action: "Left" },
    { dir: { row: 0, col: -1 }, action: "Right" }
];

class Puzzle {

    constructor(tiles) {
        this.g = 0;
        this.f = 0;
        this.prev = null;
        this.tiles = tiles;
        this.action = "Start";
        this.dim = Math.floor(Math.sqrt(tiles.length));
    }

    equals(other) {
        for (let i = 0; i < this.tiles.length; i++) {
            if (this.tiles[i] !== other[i]) {
                return false;
            }
        }
        return true;
    }

    heuristic() {
        let h = 0;
        for (let i = 0; i < this.tiles.length; i++) {
            const t = this.tiles[i];
            if (t != 0) {
                const pos1 = pos_of_index(i, this.dim);
                const pos2 = pos_of_index(t, this.dim);
                h += Math.abs(pos2.row - pos1.row) + Math.abs(pos2.col - pos1.col);
            }
        }
        return h;
    }

    find_zero() {
        for (let i = 0; i < this.tiles.length; i++) {
            if (this.tiles[i] === 0) {
                return pos_of_index(i, this.dim);
            }
        }
        throw new Error("Puzzles should contain a 0 tile");
    }

    hash() {
        return this.tiles.join('');
    }

    printString() {
        let s = "";
        s += this.action.toString() + "\n";
        for (let i = 0; i < this.tiles.length; i++) {
            if (this.tiles[i] === 0) {
                s += "  ";
            } else {
                s += this.tiles[i] + " ";
            }
            if ((i + 1) % this.dim === 0) {
                s += "\n";
            }
        }
        return s
    }

    neighbors(callback) {
        const zero_pos = this.find_zero();
        const zero_index = index_of_pos(zero_pos, this.dim);

        for (const { dir, action } of directions) {
            const next_pos = add_positions(zero_pos, dir);
            const next_index = index_of_pos(next_pos, this.dim);

            if (!in_bounds(next_pos, this.dim)) {
                continue;
            }

            const next_puzzle = new Puzzle([...this.tiles]);

            const temp = next_puzzle.tiles[zero_index];
            next_puzzle.tiles[zero_index] = next_puzzle.tiles[next_index];
            next_puzzle.tiles[next_index] = temp;

            next_puzzle.prev = this;
            next_puzzle.action = action;
            next_puzzle.g = this.g + 1;
            next_puzzle.f = next_puzzle.g + next_puzzle.heuristic();

            callback(next_puzzle);
        }
    }
}

class Heap {

    constructor(compare) {
        this.elements = [];
        this.compare = compare;
    }

    getSize() {
        return this.elements.length;
    }

    isEmpty() {
        return this.elements.length === 0;
    }

    push(e) {
        this.elements.push(e);
        this.siftUp(this.elements.length - 1); //last element
    }

    peek() {
        return this.elements[0];
    }

    pop() {
        const val = this.peek();
        this.move(this.elements.length - 1, 0);
        this.elements.pop();
        this.siftDown(0);
        return val;
    }

    clear() {
        this.elements = [];
    }

    siftUp(pos) {
        let parent = ((pos - 1) / 2) >> 0; //integer division
        while (parent >= 0) {
            if (this.compare(this.elements[pos], this.elements[parent])) {
                this.swap(pos, parent);
                pos = parent;
                parent = ((pos - 1) / 2) >> 0;
            } else {
                parent = -1;
            }
        }
    }

    siftDown(pos) {
        const left = 2 * pos + 1;
        const right = 2 * pos + 2;
        if (left >= this.elements.length) {
            return;
        }
        const child = (right >= this.elements.length || this.compare(this.elements[left], this.elements[right]))
            ? left : right;
        if (this.compare(this.elements[child], this.elements[pos])) {
            this.swap(child, pos);
            this.siftDown(child);
        }
    }

    move(from, to) {
        this.elements[to] = this.elements[from];
    }

    swap(a, b) {
        let val = this.elements[a];
        this.elements[a] = this.elements[b];
        this.elements[b] = val;
    }
}

function create_goal(size) {
    const tiles = [];
    for (let i = 0; i < size; i++) {
        tiles.push(i);
    }
    return tiles;
}

function reconstruct_path(curr) {
    const path = [];
    while (curr !== null) {
        path.push(curr);
        curr = curr.prev;
    }
    path.reverse();
    return path;
}

function find_path(initial) {
    const visited = {};
    const frontier = new Heap((p1, p2) => p1.f < p2.f);
    frontier.push(initial);

    const goal = create_goal(initial.tiles.length);

    let nodes = 0;
    while (!frontier.isEmpty()) {
        const curr_puzzle = frontier.pop();
        nodes++;

        if (curr_puzzle.equals(goal)) {
            return { path: reconstruct_path(curr_puzzle), nodes };
        }

        visited[curr_puzzle.hash()] = true;

        const on_neighbor = (neighbor) => {
            if (!visited[neighbor.hash()]) {
                frontier.push(neighbor);
            }
        };

        curr_puzzle.neighbors(on_neighbor);
    }

    return { path: [], nodes };
}

function read_puzzles(s) {
    const puzzles = [];
    let curr_tiles = [];

    const lines = s.split("\n");
    for (const line of lines) {
        const tokens = line.split(" ").map(t => t.trim()).filter((t) => t != "");

        if (tokens.length > 1) {
            for (const token of tokens) {
                curr_tiles.push(parseInt(token));
            }
        } else {
            if (curr_tiles.length > 0) {
                puzzles.push(new Puzzle(curr_tiles));
                curr_tiles = [];
            }
        }
    }
    return puzzles;
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

    const puzzles = read_puzzles(fileContents);
    const results = [];

    for (const puzzle of puzzles) {
        const start = process.hrtime.bigint();

        const { path, nodes } = find_path(puzzle);

        const end = process.hrtime.bigint();
        const time = Number(end - start) / 1e6;

        results.push({time, nodes});

        for (const p of path) {
            console.log(p.printString());
        }
        console.log(`Solved in ${path.length - 1} steps\n`);
    }

    let totalTime = 0;
    let totalNodes = 0;
    for (let i = 0; i < results.length; i++) {
        console.log(`Puzzle ${i + 1}: ${results[i].time} ms, ${results[i].nodes} nodes`);
        totalTime += results[i].time;
        totalNodes += results[i].nodes;
    }

    console.log(`Total: ${totalTime} ms, ${totalNodes} nodes`);
}

main();
