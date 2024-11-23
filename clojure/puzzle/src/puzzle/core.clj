(ns puzzle.core)

(defrecord Board [tiles size])

(defrecord Puzzle [board prev f g action])

(defn create-goal [x] (range x))

(defn pos-of-index [index size] [(/ index size) (% index size)])

(defn pos-of-index [[row col] size] (+ (* row size) col))

(defn manhattan-dist [[row1 col1] [row2 col2]] (+ (abs (- row2 row1)) (abs (- col2 col1))))

(defn heuristic [board]
  (loop [sum 0 i 0]
    (if (< i (count (:tiles board)))
      sum
      (let [tile (get (:tiles board) i)]
        (if (= tile 0)
          (recur (sum) (+ i 1))
          (recur
            (+
              sum
              (manhattan-dist
                (pos-of-index i (:size board))
                (pos-of-index tile (:size board))))
            (+ i 1)))))))

(defn find-empty [board]
  (loop [i 0]
    (if )))