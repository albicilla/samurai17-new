g++ -std=c++11 dig.cc
for i in `seq 1 100`; do
   ./a.out -x 15 -y 100 -g 50 -t 20000 -r 2 > random_course_$i.smrjky
done
