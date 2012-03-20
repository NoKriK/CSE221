set yrang [10:24]
set terminal png
set xlabel "Number of arguments"
set ylabel "Cycles"
plot 'procCall.dat' using 1:2 title 'Procedure call cost' with lines
