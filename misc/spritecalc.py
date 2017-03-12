import array

print "moi"

barrels = [0,0,0,0,0,0,0,0,0,0,0,0,0]

barrel_visi = [0,0,0,0,0,0,0,0,0,0,0,0,0]
barrel_visi_d = [0,0,0,0,0,0,0,0,0,0,0,0,0]

print barrels

full_vis = 3

curspr = -1

for x in range(0, 200):
	if (x % 16 == 0):
		if (curspr >= 0):
			barrel_visi_d[curspr] = -1
		curspr = curspr + 1
		barrel_visi[curspr] = full_vis
		barrel_visi_d[curspr] = 0

	for i, barrel in enumerate(barrels):
		barrels[i] = (barrels[i] + 1) % 21
		barrel_visi[i] += barrel_visi_d[i]
		barrel_visi[i] = min(full_vis, barrel_visi[i])
		barrel_visi[i] = max(0, barrel_visi[i])

	print x, ":", barrels, " - ", barrel_visi
