package src;

import java.util.List;

/**
 *
 * @author Joseph Prichard
 */
public class Utils
{
    public static int[][] cloneArray(int[][] src) {
        int length = src.length;
        int[][] target = new int[length][src[0].length];
        for (int i = 0; i < length; i++) {
            System.arraycopy(src[i], 0, target[i], 0, src[i].length);
        }
        return target;
    }

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

    public static int[][] listMatrixToArray(List<List<Integer>> listMatrix) {
        int[][] arrMatrix = new int[listMatrix.size()][listMatrix.size()];
        for (int i = 0; i < listMatrix.size(); i++) {
            for (int j = 0; j < listMatrix.size(); j++) {
                arrMatrix[i][j] = listMatrix.get(i).get(j);
            }
        }
        return arrMatrix;
    }

    public static int[] flattenArray(int[][] arr) {
        int[] flat = new int[arr.length * arr.length];
        int i = 0;
        for (int[] row : arr) {
            for (int e : row) {
                flat[i] = e;
                i++;
            }
        }
        return flat;
    }

    public static int rand(int min, int max) {
        return (int) (Math.random() * (max + 1 - min)) + min;
    }
}
