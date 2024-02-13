defmodule State do
  defstruct action: "",
            g: 0,
            f: 0,
            n: 3,
            tiles: [],
            prev: nil
end

defmodule PuzzleSolver do
  @type state :: %State{}
  @type position :: {integer(), integer()}
  @type tiles :: list(integer())

  @directions [{{-1, 0}, "Up"}, {{1, 0}, "Down"}, {{0, -1}, "Left"}, {{0, 1}, "Right"}]

  @spec index_to_pos(integer(), integer()) :: position()
  def index_to_pos(i, n), do: {floor(i / n), rem(i, n)}

  @spec pos_to_index(position(), integer()) :: integer()
  def pos_to_index({row, col}, n), do: row * n + col

  @spec add_pos(position(), position()) :: position()
  def add_pos({row1, col1}, {row2, col2}), do: {row1 + row2, col1 + col2}

  @spec string_to_tiles(String.t()) :: {tiles(), integer()}
  def string_to_tiles(string) do
    tiles =
      string
      |> String.split("\n")
      |> Enum.map(&String.split(&1, " "))
      |> List.flatten()
      |> Enum.map(&String.to_integer(&1))

    n =
      length(tiles)
      |> :math.sqrt()
      |> floor()

    {tiles, n}
  end

  @spec tiles_to_string(tiles(), integer()) :: String.t()
  def tiles_to_string(tiles, n) do
    f = fn x, {str, i} ->
      x = Integer.to_string(x)

      if rem(i + 1, n) == 0 do
        {str <> x <> "\n", i + 1}
      else
        {str <> x <> " ", i + 1}
      end
    end

    tiles
    |> Enum.reduce({"", 0}, f)
    |> elem(0)
  end

  @spec state_to_string(state()) :: String.t()
  def state_to_string(state), do: state.action <> "\n" <> tiles_to_string(state.tiles, state.n)

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
    |> List.replace_at(index1, Enum.at(tiles, index2))
    |> List.replace_at(index2, Enum.at(tiles, index1))
  end

  @spec neighbor(state(), {position(), any()}, position()) :: state()
  def neighbor(state, direction, target_pos) do
    {dpos, action} = direction

    tiles = swap_tiles(state.tiles, state.n, target_pos, add_pos(target_pos, dpos))

    %State{
      action: action,
      g: state.g + 1,
      f: state.g + 1 + heuristic(tiles, state.n),
      n: state.n,
      tiles: tiles,
      prev: state
    }
  end

  @spec neighbors(state()) :: list(state())
  def neighbors(state) do
    zero_pos = find_zero(state.tiles, state.n)

    @directions
    |> Enum.map(&neighbor(state, &1, zero_pos))
  end

  @spec heuristic(tiles(), integer()) :: number()
  def heuristic(tiles, n) do
    f = fn x, {h, i} ->
      {row1, col1} = index_to_pos(i, n)
      {row2, col2} = index_to_pos(x, n)
      {h + abs(row2 - row1) + abs(col2 - col1), i + 1}
    end

    tiles
    |> Enum.reduce({0, 0}, f)
    |> elem(0)
  end

  @spec add_neighbor(state(), :psq.psq()) :: :psq.psq()
  def add_neighbor(state, frontier) do
    tiles_to_key(state.tiles)
    |> :psq.insert(state.f, state, frontier)
  end

  @spec is_visited(state(), map()) :: boolean()
  def is_visited(state, visited) do
    key = tiles_to_key(state.tiles)
    Map.has_key?(visited, key)
  end

  @spec add_neighbors(:psq.psq(), map(), state()) :: :psq.psq()
  def add_neighbors(frontier, visited, state) do
    neighbors(state)
    |> Enum.filter(&(not is_visited(&1, visited)))
    |> Enum.reduce(frontier, &add_neighbor/2)
  end

  @spec tiles_to_key(tiles()) :: integer()
  def tiles_to_key(tiles) do
    f = fn x, {i, key} -> {i + 1, key + 10 ** i * x} end

    Enum.reduce(tiles, {0, 0}, f)
    |> elem(1)
  end

  @spec reconstruct_path(state()) :: list(state())
  def reconstruct_path(state), do: reconstruct_path(state, [])

  @spec reconstruct_path(state(), list(state())) :: list(state())
  def reconstruct_path(nil, acc), do: acc

  def reconstruct_path(state, acc), do: reconstruct_path(state.prev, [state | acc])

  @spec search(tiles(), integer()) :: list(state())
  def search(tiles, n) do
    state = %State{tiles: tiles, n: n}
    visited = %{}
    frontier = :psq.insert(0, state.f, state, :psq.new())
    search(frontier, visited, [0, 1, 2, 3, 4, 5, 6, 7, 8])
  end

  @spec search(:psq.psq(), map(), tiles()) :: list(state())
  def search(frontier, visited, goal) do
    case :psq.find_min(frontier) do
      :nothing ->
        []

      {:just, {_, _, state}} ->
        IO.puts("Curr state\n" <> tiles_to_string(state.tiles, state.n))

        if state.tiles == goal do
          reconstruct_path(state)
        else
          new_visited = Map.put(visited, tiles_to_key(state.tiles), true)

          :psq.delete_min(frontier)
          |> add_neighbors(new_visited, state)
          |> search(new_visited, goal)
        end
    end
  end
end
