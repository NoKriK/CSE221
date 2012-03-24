
proc = 1
avg = 0
with open('fileContention-block-ms.dat', 'r') as f:
	for line in f:
		split = line.split(' ')
		if int(split[0]) != proc:
			print proc, avg / proc
			proc = int(split[0])
			avg = 0
		avg += float(split[1])
print proc, avg / proc
