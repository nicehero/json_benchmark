import numpy as np
import matplotlib.pyplot as plt

fp = open("result.txt","r")
ll = fp.readlines()
fp.close()

fig,_ = plt.subplots(figsize=(18, 25))
axs = []

def on_move(event):
	# get the x and y pixel coords
	x, y = event.x, event.y
	if event.inaxes:
		ax = event.inaxes  # the axes instance
		for a in axs:
			if a[0] == ax:
				lit = 'k' if a[2] else ''
				a[1].set_position((event.xdata, event.ydata))
				a[1].set_text(str(int(event.ydata)) + lit)
			else:
				a[1].set_text("")
		#print('data coords %f %f %f %f' % (x, y,event.xdata, event.ydata))
		fig.canvas.draw_idle()


opts = [
	("bson2json","(70k bson)"),
	("json2bson","(70k bson)"),
	("insert","(on empty insert 500 object)"),
	("insert&erase","(on 70k bson)"),
	("find&replace","(on 70k bson)"),
	("copy","(70k bson)"),
	("parse","(70k bson)"),
	("dump","(70k bson)"),
]

tools = ["nlohmann","jsoncons","python"]
i = 1
for opt,lit in opts:
	ax = plt.subplot(2,4,i)
	plt.title(opt + lit)
	plt.ylabel("QPS")
	maxQPS = 0
	isW = False
	for l in ll:
		l = l.strip()
		if l[l.find(' ')+1:l.find(' ',l.find(' ')+1)] == opt:
			tool = l[:l.find(' ')]
			qps = int(l[l.find('qps:') + 4:])
			if qps > 10000 or isW == True:
				qps /= 1000
				isW = True
				plt.ylabel("QPS(k)")
			maxQPS = qps if qps > maxQPS else maxQPS
			plt.bar(tool, qps)
	ys = np.linspace(0, maxQPS, num=10)
	if isW:
		plt.yticks(ys,list(map(lambda x: str(int(x)) + "k", ys)))
	t = plt.text(0,0,"",fontsize = 10)
	axs.append([ax,t,isW])
	i += 1

plt.connect('motion_notify_event', on_move)
plt.show()
