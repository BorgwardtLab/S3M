########################################################################
# Regular
########################################################################

./s3m -p -m 5 -M 8   ../data/Blood_pressure.csv   > /tmp/Blood_pressure.json    # 241
./s3m -p -m 5 -M 12  ../data/Heart_rate.csv       > /tmp/Heart_rate.json        #  42
./s3m -p -m 5 -M 12  ../data/Respiratory_rate.csv > /tmp/Respiratory_rate.json  #  68

########################################################################
# Standardized
########################################################################

./s3m --standardize -p -m 5 -M 8  ../data/Blood_pressure.csv   > /tmp/Blood_pressure.json   # 242 ( 56725)
./s3m --standardize -p -m 5 -M 12 ../data/Heart_rate.csv       > /tmp/Heart_rate.json       #  42 (295838)
./s3m --standardize -p -m 5 -M 12 ../data/Respiratory_rate.csv > /tmp/Respiratory_rate.json #  72 (285250)

########################################################################
# Shapelets
########################################################################

111,66,117,50    # Heart rate
18,26,18,6,27    # Respiratory rate
99,93,101,149,90 # Blood pressure


# Heart rate
return t.isClose( localCandidate, 0.1, 0.42 );
standardized!
2--5, plus duplicate contingency table removal
2--8, plus duplicate contingency table removal  470

# Respiratory rate
return t.isClose( localCandidate, 0.1, 0.60 );
standardized!
2--5, plus duplicate contingency table removal  386
2--8, plus duplicate contingency table removal  589

# Blood pressure
return t.isClose( localCandidate, 0.1, 0.20 );
standardized!
2--5, plus duplicate contingency table removal 
2--8, plus duplicate contingency table removal 

102,145,99,87,96

########################################################################
# Creating cases and controls from the CSV file
########################################################################

grep    "^1" Blood_pressure.csv > Blood_pressure_cases.csv
grep -v "^1" Blood_pressure.csv > Blood_pressure_controls.csv

########################################################################
# Creating a subsample
########################################################################

sort -R Blood_pressure_cases.csv    | head -n 50 >  Blood_pressure_random_sample.csv
sort -R Blood_pressure_controls.csv | head -n 50 >> Blood_pressure_random_sample.csv
gshuf -o Blood_pressure_random_sample.csv < Blood_pressure_random_sample.csv

# New
./s3m -n -t -m 4 -M 6 -p ../data/Heart_rate.csv       > /tmp/Heart_rate_4_6.json       # 375 | 295
./s3m -n -t -m 4 -M 6 -p ../data/Respiratory_rate.csv > /tmp/Respiratory_rate_4_6.json
./s3m -n -t -m 4 -M 6 -p ../data/Respiratory_rate.csv > /tmp/Respiratory_rate_4_6.json
