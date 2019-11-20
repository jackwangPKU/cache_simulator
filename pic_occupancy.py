#-*- coding:utf-8 -*-
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.pyplot import MultipleLocator

file1 = open("lbm_parest_0_0_0_1", 'r')
file2 = open("lbm_parest_0_0_0_2", 'r')

occupancy1 = []
occupancy2 = []

while True:
    line = file1.readline()
    if not line:
        break
    line = line.strip('\n')
    occupancy1.append(int(line))

while True:
    line = file2.readline()
    if not line:
        break
    line = line.strip('\n')
    occupancy2.append(int(line))


x = range(len(occupancy1))

plt.scatter(list(x), occupancy1, c='red', marker='.', label='lbm', linewidths=0.01)
plt.scatter(list(x), occupancy2, c='green', marker='.', label='parest', linewidths=0.01)#, alpha=0.1)
# plt.plot(list(x), occupancy1, color='red', label='lbm', linewidth=0.05)
# plt.plot(list(x), occupancy2, color='green', label='parest', linewidth=0.05)
plt.ylabel('occupancy')
#x_major_locator=MultipleLocator()
y_major_locator=MultipleLocator(1)
ax=plt.gca()
#ax.xaxis.set_major_locator(x_major_locator)
ax.yaxis.set_major_locator(y_major_locator)
plt.ylim(0,12)
plt.legend(loc=1)
plt.title("lbm_parest_0_0_0")
plt.savefig("lbm_parest_0_0_0.png")
