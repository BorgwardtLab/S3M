#!/usr/bin/env python3

n  = 476
n1 = 237
n0 = n - n1

tables = []

for a in range(0,n1+1):
  b = n1 - a
  for c in range(0,n0+1):
    d = n0 - c

    tables.append( [a,b,d,c] )

print("{")
print("\"shapelets\": [")
for table in tables:
  print("{")
  print("\"table\":")
  print("[")
  a,b,d,c = table
  print("{},{},{},{}".format(a,b,d,c))
  print("]")
  print("},")
print("]")
print("}")
