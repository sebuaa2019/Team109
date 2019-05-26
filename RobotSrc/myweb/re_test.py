import re
import datetime
file = open("C:/Users/72927/Desktop/rosout.log")
list = []
log_list = [[] for i in range(3)]
for line in file.readlines():
    list.append(line)

pattern = r'(\d+\.\d+) ([A-Z]+)? (.*)'

for x in list:
    obj = re.match(pattern, x)
    #print(obj.group(1))
    timestr = obj.group(1)
    timeStamp = float(timestr)
    timeArray = datetime.datetime.utcfromtimestamp(timeStamp)
    stdTime = timeArray.strftime("%Y-%m-%d:%H:%M:%S")
    #print(stdTime)
    log_list[0].append(stdTime)
    if obj.group(2) is None:
        log_list[1].append("BASE")
    else:
        log_list[1].append(obj.group(2))
    log_list[2].append(obj.group(3))

for x in log_list[0]:
    print(x)
for x in log_list[1]:
    print(x)
for x in log_list[2]:
    print(x)


