const std = @import("std");
const puzzle = @import("puzzle.zig");
const mem = std.mem;
const debug = std.debug;
const time = std.time;
const heap = std.heap;
const process = std.process;

pub fn main() !void {
    var arena = heap.ArenaAllocator.init(heap.page_allocator);
    defer arena.deinit();
    var alloc = arena.allocator();

    var args = try process.argsWithAllocator(alloc);
    defer args.deinit();

    var args_list = std.ArrayList([]const u8).init(alloc);
    while (args.next()) |arg| {
        try args_list.append(arg);
    }

    if (args_list.items.len < 2) {
        debug.panic("Needs at least one argument for the input file\n", .{});
    }

    const inputFile = args_list.items[1];

    var flag: []const u8 = "seq";
    if (args_list.items.len >= 3) {
        flag = args_list.items[2];
    }

    const puzzles = try puzzle.readPuzzles(alloc, inputFile);

    const start_time = time.nanoTimestamp();

    var solutions: []puzzle.Solution =
        if (mem.eql(u8, flag, "seq"))
        puzzle.findPaths(alloc, puzzles)
    else if (mem.eql(u8, flag, "par"))
        puzzle.findPaths(alloc, puzzles)
    else
        debug.panic("Parallelism flag must be 'par' or 'seq', got {s}\n", .{flag});

    const end_time = time.nanoTimestamp();
    const ete_time = @divFloor(end_time - start_time, @as(i128, 1_000_000.0)); // convert to milliseconds

    for (0.., solutions) |i, s| {
        debug.print("Solution for puzzle {d}\n", .{i + 1});
        s.print();
        const l = if (s.len() > 0) s.len() - 1 else 0;
        debug.print("Solved in {d} steps\n\n", .{l});
    }

    var total_time: f64 = 0;
    var total_nodes: u32 = 0;
    for (0.., solutions) |i, s| {
        debug.print("Puzzle {d}: {d} ms, {d} nodes\n", .{ i + 1, s.time, s.nodes });
        total_time += s.time;
        total_nodes += s.nodes;
    }
    debug.print("\nTotal: {d} ms, {d} nodes\n", .{ total_time, total_nodes });

    debug.print("End-to-end: {d} ms\n", .{ete_time});
}
