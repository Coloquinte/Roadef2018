for i in $(cat move_list.txt); do
  cnt=`grep "	$i\$" -R results_180s/ | wc -l` 
  printf "%3d $i\n" $cnt
done
