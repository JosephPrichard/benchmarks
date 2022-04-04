package src;

import java.util.Comparator;

public class FScoreComparator implements Comparator<PuzzleState>
{
    /**
     * Implementation of comparator used for PriorityQueue
     *
     * @param o1, the first node to compare
     * @param o2, the second node to compare
     * @return -1 if o1 is less, 1 if o1 is more, 0 if equal
     */
    @Override
    public int compare(PuzzleState o1, PuzzleState o2) {
        if(o1.getFScore() < o2.getFScore()) {
            return -1;
        } else if(o1.getFScore() > o2.getFScore()){
            return 1;
        }
        return 0;
    }

}