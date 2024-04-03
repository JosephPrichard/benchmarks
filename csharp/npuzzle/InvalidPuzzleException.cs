using System;

namespace npuzzle
{
    public class InvalidPuzzleException : Exception
    {
        public InvalidPuzzleException(string message) : base(message)
        {
        }
    }
}