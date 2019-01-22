for s in {1..10}
do
  for t in 1 2 4 8 16 32
  do
    for i in $(cat test_list.txt)
    do
      ./release/gcut_opt -p dataset/$i -v2 -t 180 -s $s -j $t -o results_180_${t}/${i}_${s}_solution.csv | tee results_180_${t}/${i}_${s}.log
    done
  done
done
