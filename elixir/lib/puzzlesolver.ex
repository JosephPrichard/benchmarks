defmodule Puzzle do
  defstruct action: "",
            g: 0,
            f: 0,
            n: 3,
            tiles: [],
            prev: nil
end

defmodule PuzzleSolver do
  @type puzzle :: %Puzzle{}
  @type position :: {integer(), integer()}
  @type tiles :: list(integer())

  @directions [{{-1, 0}, "Up"}, {{1, 0}, "Down"}, {{0, -1}, "Left"}, {{0, 1}, "Right"}]

  @spec index_to_pos(integer(), integer()) :: position()
  def index_to_pos(i, n), do: {floor(i / n), rem(i, n)}

  @spec pos_to_index(position(), integer()) :: integer()
  def pos_to_index({row, col}, n), do: row * n + col

  @spec add_pos(position(), position()) :: position()
  def add_pos({row1, col1}, {row2, col2}), do: {row1 + row2, col1 + col2}

  @spec find_zero(tiles(), integer()) :: position()
  def find_zero(tiles, n) do
    Enum.find_index(tiles, &(&1 == 0))
    |> index_to_pos(n)
  end

  @spec swap_tiles(tiles(), integer(), position(), position()) :: tiles()
  def swap_tiles(tiles, n, pos1, pos2) do
    index1 = pos_to_index(pos1, n)
    index2 = pos_to_index(pos2, n)

    tiles
    |> List.update_at(index1, fn _ -> Enum.at(tiles, index2) end)
    |> List.update_at(index2, fn _ -> Enum.at(tiles, index1) end)
  end

  @spec neighbor(puzzle(), {position(), any()}, position()) :: puzzle()
  def neighbor(puzzle, direction, target_pos) do
    {dpos, action} = direction

    tiles = swap_tiles(puzzle.tiles, puzzle.n, target_pos, add_pos(target_pos, dpos))

    %Puzzle{
      action: action,
      g: puzzle.g + 1,
      f: puzzle.g + 1 + heuristic(tiles, puzzle.n),
      n: puzzle.n,
      tiles: tiles,
      prev: puzzle
    }
  end

  @spec neighbors(puzzle()) :: list(puzzle())
  def neighbors(puzzle) do
    zero_pos = find_zero(puzzle.tiles, puzzle.n)

    @directions
    |> Enum.map(&neighbor(puzzle, &1, zero_pos))
  end

  @spec heuristic(tiles(), integer()) :: number()
  def heuristic(tiles, n) do
    f = fn x, acc ->
      {h, i} = acc
      {row1, col1} = index_to_pos(i, n)
      {row2, col2} = index_to_pos(x, n)
      {h + abs(row2 - row1) + abs(col2 - col1), i + 1}
    end

    tiles
    |> Enum.reduce({0, 0}, f)
    |> elem(0)
  end

  def add_neighbor(puzzle, frontier), do: :psq.insert(0, puzzle.f, puzzle, frontier)

  def is_visited(puzzle, visited) do
    key = tiles_to_key(puzzle.tiles)
    Map.has_key?(visited, key)
  end

  @spec tiles_to_key(tiles()) :: integer()
  def tiles_to_key(tiles) do
    Enum.reduce(tiles, {0, 0}, fn x, acc ->
      {i, key} = acc
      {i + 1, key + 10 ** i * x}
    end)
    |> elem(1)
  end

  def reconstruct_path(nil), do: []

  def reconstruct_path(puzzle), do: [puzzle | reconstruct_path(puzzle.prev)]

  @spec search({tiles(), integer()}) :: list(puzzle())
  def search({tiles, n}) do
    puzzle = %Puzzle{tiles: tiles, n: n}
    visited = %{}
    frontier = :psq.insert(0, puzzle.f, puzzle, :psq.new())
    search(visited, frontier, [0, 1, 2, 3, 4, 5, 7, 8])
  end

  def search(visited, frontier, goal) do
    case :psq.find_min(frontier) do
      :nothing ->
        []

      {:just, {_, _, puzzle}} ->
        if puzzle.tiles == goal do
          reconstruct_path(puzzle)
        else
          new_visited = Map.put(visited, tiles_to_key(puzzle.tiles), true)

          new_frontier =
            neighbors(puzzle)
            |> Enum.filter(&is_visited(&1, new_visited))
            |> Enum.reduce(frontier, &add_neighbor/2)

          search(new_visited, new_frontier, goal)
        end
    end
  end
end
