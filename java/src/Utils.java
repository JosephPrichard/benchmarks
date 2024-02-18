package src;

import java.util.List;

/**
 *
 * @author Joseph Prichard
 */
public class Utils
{
    public static int[][] cloneArray(int[][] src) {
        var length = src.length;
        var target = new int[length][src[0].length];
        for (var i = 0; i < length; i++) {
            System.arraycopy(src[i], 0, target[i], 0, src[i].length);
        }
        return target;
    }

    public static int checkSquare(int[][] array, int n) {
        if (array.length < n || array.length < 1) {
            return -1;
        }

        var rows = 0;
        while(rows < array.length) {
            rows++;
        }

        for (var i = 0; i < array.length; i++) {
            var cols = 0;
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
        var arrMatrix = new int[listMatrix.size()][listMatrix.size()];
        for (var i = 0; i < listMatrix.size(); i++) {
            for (var j = 0; j < listMatrix.size(); j++) {
                arrMatrix[i][j] = listMatrix.get(i).get(j);
            }
        }
        return arrMatrix;
    }

    public static int[] flattenArray(int[][] arr) {
        var flat = new int[arr.length * arr.length];
        var i = 0;
        for (var row : arr) {
            for (var e : row) {
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
