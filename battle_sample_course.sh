#jはグローバル変数
export j=0
# 
for i in `seq 1 9`; do
  echo 'this is battle'$(($i/2)) 1>&2
  j=$(($j+1))
  ./official/official course/course_$j.smrjky player/alpha/player me player/sample/player sample > results/generated_result$i.out
  ./official/official course/course_$j.smrjky player/sample/player sample player/alpha/player me > results/generated_result$(($i+1)).out
done
j=$(($j+1))
./official/official course/course_$j.smrjky player/alpha/player me player/sample/player sample > results/generated_result20.out
./official/official course/course_$j.smrjky player/sample/player sample player/alpha/player me > results/generated_result21.out
