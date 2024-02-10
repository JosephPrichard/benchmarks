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
    puzzle = %Puzzle{tiles: [1, 2, 3, 4, 0, 5, 6, 7, 8], n: 3}

    ntiles_expected = [
      [1, 0, 3, 4, 2, 5, 6, 7, 8],
      [1, 2, 3, 4, 7, 5, 6, 0, 8],
      [1, 2, 3, 0, 4, 5, 6, 7, 8],
      [1, 2, 3, 4, 5, 0, 6, 7, 8]
    ]

    ntiles =
      PuzzleSolver.neighbors(puzzle)
      |> Enum.map(& &1.tiles)

    assert ntiles_expected == ntiles
  end

  test "puzzle_to_key hash function works properly" do
    assert PuzzleSolver.tiles_to_key([1, 2, 3, 4, 0, 5, 6, 7, 8]) == 876_504_321
  end
end
