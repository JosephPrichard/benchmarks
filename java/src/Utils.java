package src;

import java.util.Random;

/**
 *
 * @author Joseph Prichard
 */
public class Utils
{
    /**
     * Clones the provided array of integers
     *
     * @param src, the array to be cloned
     * @return a new clone of the provided array
     */
    public static int[][] cloneArray(int[][] src) {
        int length = src.length;
        int[][] target = new int[length][src[0].length];
        for (int i = 0; i < length; i++) {
            System.arraycopy(src[i], 0, target[i], 0, src[i].length);
        }
        return target;
    }

    /**
     * Checks if an array is a square array at least size NxN
     *
     * @param array, to be checked
     * @return size of square array, -1 if it isn't a square array
     */
    public static int checkSquare(int[][] array, int n) {
        if (array.length < n || array.length < 1) {
            return -1;
        }

        int rows = 0;
        while(rows < array.length) {
            rows++;
        }

        for (int i = 0; i < array.length; i++) {
            int cols = 0;
            while(cols < array.length) {
                cols++;
            }
            if (rows != cols) {
                return -1;
            }
        }

        return array.length;
    }

    public static int rand(int min, int max) {
        return (int) (Math.random() * (max + 1 - min)) + min;
    }
}
