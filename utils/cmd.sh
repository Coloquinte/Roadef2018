s=1
for t in 4 8 16 32 2
do
  mkdir -p results_180s/${t}T/A
  mkdir -p results_180s/${t}T/B
  mkdir -p results_180s/${t}T/G
  for i in $(cat test_list.txt)
  do
    ./challengeSG -p dataset/$i -v2 -t 180 -s $s -j $t -o results_180s/${t}T/${i}_${s}_solution.csv | tee results_180s/${t}T/${i}_${s}.log
  done
done
