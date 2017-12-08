for i in `seq 1 5`; do
  echo 'this is battle'$i 1>&2
  ./official/official course/random_course_$i.smrjky player/beamZ/player me player/sample/player sample > results/generated_result$i.out
done
