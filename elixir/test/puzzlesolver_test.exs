defmodule PuzzleSolverTest do
  use ExUnit.Case
  doctest PuzzleSolver

  test "find_zero finds the zero" do
    tiles = [1, 2, 3, 4, 0, 5, 6, 7, 8]
    assert PuzzleSolver.find_zero(tiles, 3) == {1, 1}
  end

  test "swap_tiles swaps the tiles" do
    tiles = [1, 2, 3, 4, 0, 5, 6, 7, 8]
    assert PuzzleSolver.swap_tiles(tiles, 3, {1, 1}, {0, 0}) == [0, 2, 3, 4, 1, 5, 6, 7, 8]
  end

  test "neighbors gets the right neighbors" do
    state = %State{tiles: [1, 2, 3, 4, 0, 5, 6, 7, 8], n: 3}

    ntiles_expected = [
      [1, 0, 3, 4, 2, 5, 6, 7, 8],
      [1, 2, 3, 4, 7, 5, 6, 0, 8],
      [1, 2, 3, 0, 4, 5, 6, 7, 8],
      [1, 2, 3, 4, 5, 0, 6, 7, 8]
    ]

    ntiles =
      PuzzleSolver.neighbors(state)
      |> Enum.map(& &1.tiles)

    assert ntiles_expected == ntiles
  end

  test "tiless_to_key hash function works properly" do
    assert PuzzleSolver.tiles_to_key([1, 2, 3, 4, 0, 5, 6, 7, 8]) == 876_504_321
  end

  test "string_to_tiles converts string to valid tiles" do
    assert PuzzleSolver.string_to_tiles("1 2 3\n4 5 6\n7 8 0") == {[1, 2, 3, 4, 5, 6, 7, 8, 0], 3}
  end

  test "tiles_to_string converts tiles to valid string" do
    assert PuzzleSolver.tiles_to_string([1, 2, 3, 4, 5, 6, 7, 8, 0], 3) == "1 2 3\n4 5 6\n7 8 0\n"
  end

  test "add_neighbors function adds nonvisited neighbors to frontier" do
    state = %State{tiles: [1, 2, 3, 4, 0, 5, 6, 7, 8], n: 3}
    neighbor_key = PuzzleSolver.tiles_to_key([1, 2, 3, 0, 4, 5, 6, 7, 8])

    added_states =
      :psq.new()
      |> PuzzleSolver.add_neighbors(%{neighbor_key => true}, state)
      |> :psq.to_list()
      |> Enum.map(&elem(&1, 2).action)

    assert Enum.sort(added_states) == Enum.sort(["Right", "Up", "Down"])
  end

  test "heuristic function retrieves correct value" do
    assert PuzzleSolver.heuristic([1, 2, 3, 4, 0, 5, 6, 7, 8], 3) == 8
  end

  test "search finds the correct solution for the base case" do
    actions =
      [0, 1, 2, 3, 4, 5, 6, 7, 8]
      |> PuzzleSolver.search(3)
      |> Enum.map(&(&1.action))
    assert actions == [""]
  end

  test "search finds the correct solution for a 1 step case" do
    actions =
      [1, 2, 0, 3, 4, 5, 6, 7, 8]
      |> PuzzleSolver.search(3)
      |> Enum.map(&(&1.action))
    assert actions == ["", "Left", "Left"]
  end
end
