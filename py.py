# coding=utf-8
import bson
import json
import time
import base58
import copy

repeatCount = 1000
keyNum = 500
jdata = open("data.txt", encoding='UTF-8').read()
j = json.loads(jdata)
bs = bson.encode(j)
j = bson.decode(bs)
print("len(bson):" + str(len(bs)))
ks = []
for i in range(keyNum):
	h = hash(str(i + 1))
	ks.append(base58.b58encode(h.to_bytes(8,'little',signed=True)))	
#print(ks)
print("len(ks):" + str(len(ks)))

s = time.time()
for i in range(repeatCount):
	j2 = bson.decode(bs)
dt = time.time() - s
print("python bson2json times:" + str(repeatCount) + " cost:" + str(dt) + " qps:"+str(int(repeatCount/dt)))

s = time.time()
for i in range(repeatCount):
	bs2 = bson.encode(j)
dt = time.time() - s
print("python json2bson times:" + str(repeatCount) + " cost:" + str(dt) + " qps:"+str(int(repeatCount/dt)))

repeatCount = 1000

m = {}
for i in range(repeatCount):
	m[i] = {}

s = time.time()
for i in range(repeatCount):
	for jj in range(len(ks)):
		m[i][ks[jj]] = i + jj
dt = time.time() - s
print("python insert times:"+ str(keyNum) + "x" + str(repeatCount) + " cost:" + str(dt) + " qps:"+str(int(repeatCount*keyNum/dt)))

repeatCount = 100000
s = time.time()
for i in range(repeatCount):
	st = ks[i % len(ks)]
	j[st] = i + 100
	del j[st]
dt = time.time() - s
print("python insert&erase times:" + str(repeatCount) + " cost:" + str(dt) + " qps:"+str(int(repeatCount/dt)))

repeatCount = 100000
mm = {}
miss = 10
missValue = 0
for v in ks:
	mm[v] = 0
s = time.time()
for i in range(repeatCount):
	st = ks[i % len(ks)]
	if i % 100 < miss:
		missValue = mm.get("miss_key")
	else:
		mm[st] = i + 200
dt = time.time() - s
print("python find&replace times:" + str(repeatCount) + " cost:" + str(dt) + " qps:"+str(int(repeatCount/dt)))

repeatCount = 100

mm = {}
for i in range(repeatCount):
	mm[i] = {}
s = time.time()
for i in range(repeatCount):
	mm[i] = copy.deepcopy(j)
dt = time.time() - s
print("python copy times:" + str(repeatCount) + " cost:" + str(dt) + " qps:"+str(int(repeatCount/dt)))

repeatCount = 1000

s = time.time()
for i in range(repeatCount):
	json.loads(jdata)
dt = time.time() - s
print("python parse times:" + str(repeatCount) + " cost:" + str(dt) + " qps:"+str(int(repeatCount/dt)))

s = time.time()
for i in range(repeatCount):
	json.dumps(j)
dt = time.time() - s
print("python dump times:" + str(repeatCount) + " cost:" + str(dt) + " qps:"+str(int(repeatCount/dt)))
