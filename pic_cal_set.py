#-*- coding:utf-8 -*-
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.pyplot import MultipleLocator

file = open("lbm_omnetpp_access", 'r')
result = []
while True:
    line = file.readline()
    if not line:
        break
    line = line.strip('\n')
    result.append(int(line))

x = range(len(result))
plt.bar(x, result, color='red', width=1.0)
plt.ylabel('miss')
# plt.ylim(0.2E7, 1E7)
plt.savefig("cal_set_slice_access.png")
