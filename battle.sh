#jはグローバル変数
export j=0
# for int i i=1 i<=100 i+=2
for i in `seq 1 2 100`; do
  echo 'this is battle'$(($i/2)) 1>&2
  j=$(($j+1))
  ./official/official course/random_course_$j.smrjky player/sample/player me player/greedy/player sample > results/generated_result$i.out
  ./official/official course/random_course_$j.smrjky player/greedy/player sample player/sample/player me > results/generated_result$(($i+1)).out
done
