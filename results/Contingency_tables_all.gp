set terminal qt

set xrange [-1.05:1.05]
set yrange [-1.05:1.05]
set zeroaxis

set size square

set title "Contingency Tables (Heart Rate)" 
set xlabel "Case distinction" 
set ylabel "Control distinction" 

set pointsize 0.5

plot "Heart_rate_4_6_contingency_tables_best.txt" pt 7 lc rgb '#EF888888' title "All shapelets",\
     "Heart_rate_contingency_tables.txt"          pt 7 lc rgb '#99C41E3A' title "Significant shapelets"

pause -1
