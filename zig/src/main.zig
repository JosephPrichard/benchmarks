const std = @import("std");
const puzzle = @import("puzzle.zig");

fn millisFromNanos(nanos: u64) f64 {
    const nanos_float: f64 = @floatFromInt(nanos);
    return nanos_float / 1_000_000;
}

fn showResults(bufw: *std.io.BufferedWriter(4096, std.fs.File.Writer), tasks: []puzzle.Task, ete_time: u64) !void {
    var w = bufw.writer();

    for (0.., tasks) |i, t| {
        const s = t.solution_out;
        try w.print("Solution for puzzle {d}\n", .{i + 1});
        try s.print(bufw);
        const l = if (s.len() > 0) s.len() - 1 else 0;
        try w.print("Solved in {d} steps\n\n", .{l});
    }

    var total_time: u64 = 0;
    var total_nodes: u32 = 0;
    for (0.., tasks) |i, t| {
        const s = t.solution_out;
        try w.print("Puzzle {d}: {d} ms, {d} nodes\n", .{ i + 1, millisFromNanos(s.time), s.nodes });
        total_time += s.time;
        total_nodes += s.nodes;
    }
    try w.print("\nTotal: {d} ms, {d} nodes\n", .{ millisFromNanos(total_time), total_nodes });

    try w.print("End-to-end: {d} ms\n", .{millisFromNanos(ete_time)});
}

pub fn main() !void {
    const out = std.io.getStdOut();
    var bufw = std.io.bufferedWriter(out.writer());

    // we use the general purpose allocator for allocating things that are not performance sensitive - with added benefit of thread safety
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const alloc = gpa.allocator();

    var args_it = try std.process.argsWithAllocator(alloc);
    defer args_it.deinit();

    var args: [2][]const u8 = undefined;

    var args_count: u32 = 0;
    while (args_it.next()) |arg| {
        if (args_count >= args.len) {
            std.debug.panic("Needs at least one argument for the input file\n", .{});
        }
        args[args_count] = arg;
        args_count += 1;
    }

    if (args_count < 1) {
        std.debug.panic("Needs at least one argument for the input file\n", .{});
    }
    const in_file = args[1];

    const tasks = try puzzle.readTasks(alloc, in_file);

    const start_time = try std.time.Instant.now();

    try puzzle.completeTasks(alloc, tasks);

    const end_time = try std.time.Instant.now();
    const ete_time = end_time.since(start_time);

    defer {
        for (tasks) |task| task.solution_out.free(alloc);
        defer alloc.free(tasks);
    }

    try showResults(&bufw, tasks, ete_time);
    try bufw.flush();
}
