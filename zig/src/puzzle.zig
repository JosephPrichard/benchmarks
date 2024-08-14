const std = @import("std");

pub const Action = enum { none, up, down, left, right };

pub const Position = struct {
    row: i32,
    col: i32,

    fn toIndex(self: Position, comptime n: u32) usize {
        const dim = std.math.sqrt(n);
        const r: u32 = @intCast(self.row);
        const c: u32 = @intCast(self.col);
        const i: usize = @intCast(r * dim + c);
        return i;
    }

    fn inBounds(self: Position, comptime n: u32) bool {
        const dim = std.math.sqrt(n);
        return self.row >= 0 and self.row < dim and self.col >= 0 and self.col < dim;
    }

    fn add(self: Position, other: Position) Position {
        return Position{ .row = self.row + other.row, .col = self.col + other.col };
    }
};

fn indexToPos(i: usize, comptime n: u32) Position {
    const dim = std.math.sqrt(n);
    return Position{ .row = @intCast(i / dim), .col = @intCast(i % dim) };
}

const Direction = struct {
    pos: Position,
    action: Action,
};

var directions = [_]Direction{
    Direction{ .pos = Position{ .row = 0, .col = 1 }, .action = Action.right },
    Direction{ .pos = Position{ .row = 0, .col = -1 }, .action = Action.left },
    Direction{ .pos = Position{ .row = 1, .col = 0 }, .action = Action.down },
    Direction{ .pos = Position{ .row = -1, .col = 0 }, .action = Action.up },
};

pub fn Puzzle(comptime n: u32) type {
    return struct {
        pub var dim: u32 = std.math.sqrt(n);

        tiles: [n]u8,
        parent: ?*const Puzzle(n),
        f: u32,
        g: u32,
        action: Action,

        const Self = @This();

        fn init(parent: ?*const Puzzle(n), tiles: []const u8) Self {
            std.debug.assert(tiles.len >= n);
            var self = Self{ .tiles = undefined, .parent = parent, .f = 0, .g = 0, .action = Action.none };
            @memcpy(&self.tiles, tiles[0..n]);
            return self;
        }

        fn initTiles(tiles: []const u8) Self {
            return init(null, tiles);
        }

        fn initChild(self: *const Self) Self {
            return init(self, &self.tiles);
        }

        fn findZero(self: Self) Position {
            for (0..self.tiles.len) |i| {
                if (self.tiles[i] == 0) {
                    return indexToPos(i, n);
                }
            }
            std.debug.panic("no zero contained within tiles", .{});
        }

        fn heuristic(self: Self) u32 {
            var h: u32 = 0;
            for (0.., self.tiles) |i, tile| {
                const pos1 = indexToPos(i, n);
                const pos2 = indexToPos(tile, n);
                if (tile != 0) {
                    h += std.math.absCast(pos2.row - pos1.row) + std.math.absCast(pos2.col - pos1.col);
                }
            }
            return h;
        }

        fn hash(self: Self) u64 {
            return hashTiles(&self.tiles);
        }

        fn print(self: Self, bufw: *std.io.BufferedWriter(4096, std.fs.File.Writer)) !void {
            var w = bufw.writer();

            switch (self.action) {
                Action.down => try w.print("Down\n", .{}),
                Action.up => try w.print("Up\n", .{}),
                Action.left => try w.print("Left\n", .{}),
                Action.right => try w.print("Right\n", .{}),
                else => {},
            }

            for (0.., self.tiles) |i, tile| {
                if (tile == 0) {
                    try w.print("  ", .{});
                } else {
                    try w.print("{d} ", .{tile});
                }
                if ((i + 1) % dim == 0) {
                    try w.print("\n", .{});
                }
            }
            try w.print("\n", .{});
        }

        fn compare(_: void, self: *const Self, other: *const Self) std.math.Order {
            return if (self.f > other.f) std.math.Order.gt else std.math.Order.lt;
        }
    };
}

fn hashTiles(tiles: []const u8) u64 {
    var h: u64 = 0;
    for (0.., tiles) |i, tile| {
        const shift: u6 = @truncate(i * 4);
        const large_tile: u64 = @as(u64, tile);
        const mask: u64 = large_tile << shift;
        h = (h | mask);
    }
    return h;
}

fn newGoal(comptime n: u32) [n]u8 {
    var goal: [n]u8 = undefined;
    for (0..n) |i| {
        var tile: u8 = @truncate(i);
        goal[i] = tile;
    }
    return goal;
}

pub const AnyPuzzle = union(enum) {
    eight: Puzzle(9),
    fifteen: Puzzle(16),

    fn init(n: u32, curr_tiles: []u8) AnyPuzzle {
        return switch (n) {
            9 => AnyPuzzle{ .eight = Puzzle(9).initTiles(curr_tiles) },
            16 => AnyPuzzle{ .fifteen = Puzzle(16).initTiles(curr_tiles) },
            else => std.debug.panic("an any_puzzle must be size 9 or 16, got {d}", .{n}),
        };
    }

    pub fn print(self: AnyPuzzle) void {
        switch (self) {
            .eight => |p| p.print(),
            .fifteen => |p| p.print(),
        }
    }
};

pub const Solution = struct {
    time: u64,
    nodes: u32,
    path: union(enum) { eight: []Puzzle(9), fifteen: []Puzzle(16) },

    // takes ownership of the array the path slice points to
    fn setPath(self: *Solution, comptime n: u32, path: []Puzzle(n)) void {
        switch (n) {
            9 => self.path = .{ .eight = path },
            16 => self.path = .{ .fifteen = path },
            else => std.debug.panic("a solution must be size 9 or 16, got {d}", .{n}),
        }
    }

    pub fn print(self: Solution, bufw: *std.io.BufferedWriter(4096, std.fs.File.Writer)) !void {
        switch (self.path) {
            .eight => |path| {
                for (path) |puzzle| try puzzle.print(bufw);
            },
            .fifteen => |path| {
                for (path) |puzzle| try puzzle.print(bufw);
            },
        }
    }

    pub fn len(self: Solution) usize {
        return switch (self.path) {
            .eight => |path| path.len,
            .fifteen => |path| path.len,
        };
    }

    pub fn free(self: Solution, alloc: std.mem.Allocator) void {
        return switch (self.path) {
            .eight => |slice| alloc.free(slice),
            .fifteen => |slice| alloc.free(slice),
        };
    }
};

fn reconstructPath(alloc: std.mem.Allocator, comptime n: u32, puzzle: ?*const Puzzle(n)) []Puzzle(n) {
    var path = std.ArrayList(Puzzle(n)).init(alloc);
    errdefer path.deinit();

    var curr: ?*const Puzzle(n) = puzzle;
    while (curr != null) {
        path.append(curr.?.*) catch unreachable;
        curr = curr.?.parent;
    }

    // reverse the path by swapping all element and its opposite up until the midpoint
    var i: u32 = 0;
    const mp = path.items.len / 2;
    while (i < mp) {
        const temp = path.items[i];
        path.items[i] = path.items[path.items.len - 1 - i];
        path.items[path.items.len - 1 - i] = temp;
        i += 1;
    }

    return path.toOwnedSlice() catch unreachable;
}

fn findPath(alloc:std.mem.Allocator, comptime n: u32, initial: Puzzle(n), solution: *Solution) void {
    // we use an arena allocator to allocate puzzle within the search function - anything here has a lifetime til end of function
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    var allocator = arena.allocator();

    var visited = std.AutoHashMap(u64, void).init(allocator);
    var frontier = std.PriorityQueue(*const Puzzle(n), void, Puzzle(n).compare).init(allocator, {});
    defer visited.deinit();
    defer frontier.deinit();

    frontier.add(&initial) catch unreachable;

    const goal = newGoal(n);
    const goal_hash = hashTiles(&goal);

    solution.setPath(n, &.{});
    solution.nodes = 0;

    while (frontier.len > 0) {
        const puzzle: *const Puzzle(n) = frontier.remove();
        solution.nodes += 1;

        const curr_hash = puzzle.hash();
        visited.put(curr_hash, {}) catch unreachable;

        if (curr_hash == goal_hash) {
            solution.setPath(n, reconstructPath(alloc, n, puzzle));
            break;
        }

        const zero_pos = puzzle.findZero();
        for (directions) |direction| {
            const new_pos = zero_pos.add(direction.pos);
            if (!new_pos.inBounds(n)) {
                continue;
            }

            // we need to allocate the next puzzle to search (which we may or may not need)
            var np_temp = puzzle.initChild();

            const new_idx = new_pos.toIndex(n);
            const zero_idx = zero_pos.toIndex(n);
            const temp = np_temp.tiles[new_idx];
            np_temp.tiles[new_idx] = np_temp.tiles[zero_idx];
            np_temp.tiles[zero_idx] = temp;

            // if the puzzle is already visited, we don't want to check it again - np_temp is stack allocated
            const np_hash = np_temp.hash();

            const value = visited.get(np_hash);
            if (value != null) {
                continue;
            }

            np_temp.g = puzzle.g + 1;
            np_temp.f = np_temp.g + np_temp.heuristic();
            np_temp.action = direction.action;

            // we need to allocate on the arena and copy the puzzle from stack
            const np = allocator.create(Puzzle(n)) catch unreachable;
            np.* = np_temp; // we must write the data before we insert into the frontier, since the frontier uses the f score
            frontier.add(np) catch unreachable;
        }
    }
}

fn findSolution(alloc: std.mem.Allocator, comptime n: u32, initial: Puzzle(n)) !Solution {
    var solution: Solution = undefined;

    const start_time = try std.time.Instant.now();

    findPath(alloc, n, initial, &solution);

    const end_time = try std.time.Instant.now();
    solution.time = end_time.since(start_time);

    return solution;
}

pub const Task = struct {
    puzzle_in: AnyPuzzle,
    solution_out: Solution,

    // creates a new task with the input but without the output (will be set once complete)
    fn init(puzzle_in: AnyPuzzle) Task {
        return Task{ .puzzle_in = puzzle_in, .solution_out = undefined };
    }
};

// parses a puzzle files into an array of tasks that can be completed by the complete task functions
pub fn readTasks(alloc: std.mem.Allocator, file_name: []const u8) ![]Task {
    var tasks = std.ArrayList(Task).init(alloc);
    defer tasks.deinit();

    const file = try std.fs.cwd().openFile(file_name, .{ .mode = std.fs.File.OpenMode.read_only });
    defer file.close();

    var curr_tiles: [16]u8 = undefined;

    var tile: u8 = 0;
    var tile_idx: u32 = 0;
    var tiles_idx: u32 = 0;

    var buffer: [4096]u8 = undefined;
    var i: u32 = 0;
    var bytes_read: usize = 0;
    var prev_nl: bool = false;

    while (true) {
        if (i >= bytes_read) {
            bytes_read = try file.readAll(&buffer);
            i = 0;
            if (bytes_read == 0) {
                break;
            }
        }

        const c = buffer[i];
        if (c == ' ' or c == '\n') {
            // only write the tile if it actually contains any changes
            if (tile_idx != 0) {
                prev_nl = false;
                // we have finished reading a tile - lets do something with it
                if (tiles_idx >= 16) return error.TilesLength;
                curr_tiles[tiles_idx] = tile;
                tiles_idx += 1;
                tile = 0;
                tile_idx = 0;
            }
            if (c == '\n') {
                if (prev_nl) {
                    // we have finished reading the tiles, lets copy them into a puzzle
                    const puzzle = AnyPuzzle.init(tiles_idx, &curr_tiles);
                    try tasks.append(Task.init(puzzle));
                    tiles_idx = 0;
                    prev_nl = false;
                } else {
                    prev_nl = true;
                }
            }
        } else if (std.ascii.isWhitespace(c)) {} // skip any other whitespace - nothing to do!
        else {
            prev_nl = false;
            std.debug.assert(c >= '0');
            const digit: u8 = @truncate(std.math.pow(u32, 10, tile_idx));
            tile += digit * (c - '0');
            tile_idx += 1;
        }

        i += 1;
    }

    return try tasks.toOwnedSlice();
}

pub fn completeTasks(alloc: std.mem.Allocator, tasks: []Task) !void {
    for (0.., tasks) |i, task| {
        const solution = switch (task.puzzle_in) {
            .eight => |puzzle| try findSolution(alloc, 9, puzzle),
            .fifteen => |puzzle| try findSolution(alloc, 16, puzzle),
        };
        tasks[i].solution_out = solution;
    }
}