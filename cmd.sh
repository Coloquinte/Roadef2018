for t in 2 4 8 16 32
do
  for i in $(cat test_list.txt)
  do
    ./release/gcut_opt -p dataset/$i -v2 -t 180 -s 0 -j $t -o results_180s/${t}T/${i}_${s}_solution.csv | tee results_180s/${t}T/${i}_${s}.log
  done
done
