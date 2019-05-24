file = open('rosout.log')
listx = []
for line in file.readlines():
    listx.append(line)

print(listx)