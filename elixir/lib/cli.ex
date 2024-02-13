defmodule NPuzzle.CLI do
  def main(args \\ []) do
    IO.puts("Start...")

    {opts, _, _} = OptionParser.parse(args, strict: [path: :string])

    case File.read(opts[:path]) do
      {:ok, binary} -> handle_file_data(binary)
      _ -> IO.puts("Unable to read from the file")
    end
  end

  @spec handle_file_data(String.t()) :: any()
  def handle_file_data(data) do
    {tiles, n} = PuzzleSolver.string_to_tiles(data)

    {timeUSecs, solution} = :timer.tc(PuzzleSolver, :search, [tiles, n])

    solution
    |> Enum.map(&(PuzzleSolver.state_to_string(&1) <> "\n"))
    |> IO.puts()

    IO.puts("Took " <> Float.to_string(timeUSecs / 1_000) <> " microseconds\n")
  end
end
