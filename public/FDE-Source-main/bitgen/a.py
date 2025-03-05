import os
a = []
b = []
for root, dirs, files in os.walk("."):
	if files != []:
		for file in files:
			if file.find(".cpp") != -1:
				a.append( root + os.sep + file )
			if file.find(".h") != -1 and root not in b:
				b.append( root )
print("SET HEADER", "\n\t".join(a).replace("\\", "/"))
print("SET DIR_SRCS", "\n\t".join(b).replace("\\", "/"))
  
