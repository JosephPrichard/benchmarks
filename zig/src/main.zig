const std = @import("std");
const puzzle = @import("puzzle.zig");
const mem = std.mem;
const debug = std.debug;
const time = std.time;
const heap = std.heap;
const process = std.process;

const Args = struct {
    in_file: []const u8,
    flag: []const u8,

    fn fromIt(args_it: *process.ArgIterator) Args {
        var args: [3][]const u8 = undefined;

        var args_count: u32 = 0;
        while (args_it.next()) |arg| {
            if (args_count >= args.len) {
                debug.panic("Needs at least one argument for the input file\n", .{});
            }
            args[args_count] = arg;
            args_count += 1;
        }

        if (args_count < 1) {
            debug.panic("Needs at least one argument for the input file\n", .{});
        }
        const in_file = args[1];

        var flag: []const u8 = "seq";
        if (args_count >= 3) {
            flag = args[2];
        }

        // this struct just contains references to the string data which lives inside the allocator the iterator use
        return Args{ .in_file = in_file, .flag = flag };
    }

    fn isSeqMode(self: Args) bool {
        return mem.eql(u8, self.flag, "seq");
    }

    fn isParMode(self: Args) bool {
        return mem.eql(u8, self.flag, "par");
    }
};

fn showResults(tasks: []puzzle.Task, ete_time: f64) void {
    for (0.., tasks) |i, t| {
        const s = t.solution_out;
        debug.print("Solution for puzzle {d}\n", .{i + 1});
        s.print();
        const l = if (s.len() > 0) s.len() - 1 else 0;
        debug.print("Solved in {d} steps\n\n", .{l});
    }

    var total_time: f64 = 0;
    var total_nodes: u32 = 0;
    for (0.., tasks) |i, t| {
        const s = t.solution_out;
        debug.print("Puzzle {d}: {d} ms, {d} nodes\n", .{ i + 1, s.time, s.nodes });
        total_time += s.time;
        total_nodes += s.nodes;
    }
    debug.print("\nTotal: {d} ms, {d} nodes\n", .{ total_time, total_nodes });

    debug.print("End-to-end: {d} ms\n", .{ete_time});
}

pub fn main() !void {
    // we use the general purpose allocator for allocating things that are not performance sensitive - with added benefit of thread safety
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const alloc = gpa.allocator();

    var args_it = try process.argsWithAllocator(alloc);
    defer args_it.deinit();

    const args = Args.fromIt(&args_it);

    const tasks = try puzzle.readTasks(alloc, args.in_file);

    const start_time = time.nanoTimestamp();

    if (args.isSeqMode()) {
        puzzle.completeTasks(alloc, tasks);
    } else if (args.isParMode()) {
        try puzzle.completeTasksParallel(alloc, tasks);
    } else {
        debug.panic("Parallelism flag must be 'par' or 'seq', got {s}\n", .{args.flag});
    }

    defer {
        for (tasks) |task| task.solution_out.free(alloc);
        defer alloc.free(tasks);
    }

    const end_time = time.nanoTimestamp();
    const diff_time: f64 = @floatFromInt(end_time - start_time);
    const ete_time: f64 = @divFloor(diff_time, 1_000_000.0); // convert to milliseconds

    showResults(tasks, ete_time);
}
