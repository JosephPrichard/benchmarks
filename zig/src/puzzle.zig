const std = @import("std");
const mem = std.mem;
const debug = std.debug;
const time = std.time;
const math = std.math;
const heap = std.heap;

pub const Action = enum { none, up, down, left, right };

pub const Position = struct {
    row: i32,
    col: i32,

    fn toIndex(self: Position, comptime n: u32) usize {
        if (self.row < 0 or self.col < 0) {
            debug.panic("Out of bounds for position: {d}, {d}", .{ self.row, self.col });
        }

        const dim = math.sqrt(n);
        const r: u32 = @intCast(self.row);
        const c: u32 = @intCast(self.col);
        const i: usize = @intCast(r * dim + c);

        if (i < 0 or i > n) {
            debug.panic("Index {d} is out of bounds for position: {d}, {d}", .{ i, r, c });
        }
        return i;
    }

    fn inBounds(self: Position, comptime n: u32) bool {
        const dim = math.sqrt(n);
        return self.row >= 0 and self.row < dim and self.col >= 0 and self.col < dim;
    }

    fn add(self: Position, other: Position) Position {
        return Position{ .row = self.row + other.row, .col = self.col + other.col };
    }
};

fn indexToPos(i: usize, comptime n: u32) Position {
    const dim = math.sqrt(n);
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
        pub var dim: u32 = math.sqrt(n);

        tiles: [n]u8,
        parent: ?*const Puzzle(n),
        f: u32,
        g: u32,
        action: Action,

        const Self = @This();

        fn init(parent: ?*const Puzzle(n), tiles: []const u8) Self {
            debug.assert(tiles.len >= n);
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
                    var tile: u32 = @truncate(i);
                    return indexToPos(tile, n);
                }
            }
            debug.panic("no zero contained within tiles", .{});
        }

        fn heuristic(self: Self) u32 {
            var h: u32 = 0;
            for (0.., self.tiles) |i, tile| {
                const pos1 = indexToPos(i, n);
                const pos2 = indexToPos(tile, n);
                if (tile != 0) {
                    h += math.absCast(pos2.row - pos1.row) + math.absCast(pos2.col - pos1.col);
                }
            }
            return h;
        }

        fn hash(self: Self) u64 {
            return hashTiles(&self.tiles);
        }

        fn print(self: Self) void {
            switch (self.action) {
                Action.down => debug.print("Down\n", .{}),
                Action.up => debug.print("Up\n", .{}),
                Action.left => debug.print("Left\n", .{}),
                Action.right => debug.print("Right\n", .{}),
                else => {},
            }

            for (0.., self.tiles) |i, tile| {
                if (tile == 0) {
                    debug.print("  ", .{});
                } else {
                    debug.print("{d} ", .{tile});
                }
                if ((i + 1) % dim == 0) {
                    debug.print("\n", .{});
                }
            }
            debug.print("\n", .{});
        }

        fn compare(_: void, self: *const Self, other: *const Self) math.Order {
            return if (self.f > other.f) math.Order.gt else math.Order.lt;
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
            else => debug.panic("an any_puzzle must be size 9 or 16, got {d}", .{n}),
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
    time: f64,
    nodes: u32,
    path: union(enum) { eight: []Puzzle(9), fifteen: []Puzzle(16) },

    fn init(comptime n: u32, path: []Puzzle(n), time_ms: f64, nodes: u32) Solution {
        return switch (n) {
            9 => Solution{ .path = .{ .eight = path }, .time = time_ms, .nodes = nodes },
            16 => Solution{ .path = .{ .fifteen = path }, .time = time_ms, .nodes = nodes },
            else => debug.panic("a solution must be size 9 or 16, got {d}", .{n}),
        };
    }

    pub fn print(self: Solution) void {
        switch (self.path) {
            .eight => |p| {
                for (p) |puz| puz.print();
            },
            .fifteen => |p| {
                for (p) |puz| puz.print();
            },
        }
    }

    pub fn len(self: Solution) usize {
        return switch (self.path) {
            .eight => |p| p.len,
            .fifteen => |p| p.len,
        };
    }
};

fn reconstructPath(out_alloc: mem.Allocator, comptime n: u32, puzzle: ?*const Puzzle(n)) []Puzzle(n) {
    var path = std.ArrayList(Puzzle(n)).init(out_alloc);
    errdefer path.deinit();

    var curr: ?*const Puzzle(n) = @constCast(puzzle);
    while (curr != null) {
        path.append(curr.?.*) catch unreachable;
        curr = @constCast(curr.?.parent);
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

    return path.items;
}

fn findPath(out_alloc: mem.Allocator, comptime n: u32, initial: Puzzle(n)) Solution {
    const start_time = time.nanoTimestamp();

    // we use an arena allocator to allocate within the search function - anything here has a lifetime til end of function
    var arena = heap.ArenaAllocator.init(heap.page_allocator);
    defer arena.deinit();
    var allocator = arena.allocator();

    var visited = std.AutoHashMap(u64, void).init(allocator);
    defer visited.deinit();

    var frontier = std.PriorityQueue(*const Puzzle(n), void, Puzzle(n).compare).init(allocator, {});
    defer frontier.deinit();

    frontier.add(&initial) catch unreachable;

    const goal = newGoal(n);
    const goal_hash = hashTiles(&goal);

    var path: []Puzzle(n) = &.{};

    var nodes: u32 = 0;
    while (frontier.len > 0) {
        const puzzle: *const Puzzle(n) = frontier.remove();
        nodes += 1;

        const curr_hash = puzzle.hash();
        visited.put(curr_hash, {}) catch unreachable;

        if (curr_hash == goal_hash) {
            path = reconstructPath(out_alloc, n, puzzle);
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
            np.* = np_temp;
            frontier.add(np) catch unreachable;
        }
    }

    const end_time = time.nanoTimestamp();
    const diff_time: f64 = @floatFromInt(end_time - start_time);
    const elapsed_time: f64 = @divFloor(diff_time, 1_000_000.0); // convert to milliseconds

    return Solution.init(n, path, elapsed_time, nodes);
}

pub fn readPuzzles(out_alloc: mem.Allocator, file_name: []const u8) ![]AnyPuzzle {
    var puzzles = std.ArrayList(AnyPuzzle).init(out_alloc);
    errdefer puzzles.deinit();

    const file = try std.fs.cwd().openFile(file_name, .{ .mode = std.fs.File.OpenMode.read_only });
    defer file.close();

    var curr_tiles: [16]u8 = undefined;

    var tile: u8 = 0;
    var tile_idx: u32 = 0;
    var tiles_idx: u32 = 0;

    var buffer: [4096]u8 = undefined;
    var i: u32 = 0;
    var bytes_read: usize = 0;
    var nl: bool = false;

    while (true) {
        if (i >= bytes_read) {
            // when we exceed the maximum length of a buffer, we need to read the next buffer
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
                nl = false;
                // we have finished reading a tile - lets do something with it
                if (tiles_idx >= 16) {
                    // but we can't have any tiles arrays that are too long!
                    return error.TilesLength;
                }
                curr_tiles[tiles_idx] = tile;
                tiles_idx += 1;

                // reset the tile to write once again to the start
                tile = 0;
                tile_idx = 0;
            }

            if (c == '\n') {
                if (nl) {
                    // we have finished reading the tiles - lets copy them into a puzzl
                    const puzzle = AnyPuzzle.init(tiles_idx, &curr_tiles);
                    try puzzles.append(puzzle);
                    tiles_idx = 0;
                    nl = false;
                } else {
                    nl = true;
                }
            }
        } else if (std.ascii.isWhitespace(c)) {} // skip the /r on windows or any other whitespace like tabs
        else {
            nl = false;
            debug.assert(c >= '0');
            const digit: u8 = @truncate(math.pow(u32, 10, tile_idx));
            tile += digit * (c - '0');
            tile_idx += 1;
        }

        i += 1;
    }

    return puzzles.items;
}

pub fn findPaths(out_alloc: mem.Allocator, puzzles: []AnyPuzzle) []Solution {
    var solutions = std.ArrayList(Solution).init(out_alloc);
    for (puzzles) |any_puzzle| {
        const solution = switch (any_puzzle) {
            .eight => |puzzle| findPath(out_alloc, 9, puzzle),
            .fifteen => |puzzle| findPath(out_alloc, 16, puzzle),
        };
        solutions.append(solution) catch unreachable;
    }
    return solutions.items;
}
