# coding: utf-8
import json
from pyecharts import options as opts
from pyecharts.charts import Tree

ifile_name ="./test.json"
ofile_name = "./test.html"
with open(ifile_name,'r',encoding="gbk") as fr: #同上
	data = json.loads(fr.read())

if ofile_name.split('.')[-1]!='html':
    print("文件名需要以html结尾")
else:
    c = (
        Tree()
        .add("",
		data,
		orient='LR', # 从上至下绘制TB
		# layout="radial", # 发散树图
		# collapse_interval=2, # 折叠枝点
		) 
		.set_global_opts(title_opts=opts.TitleOpts(title="语法树"))
        .render(ofile_name)
    )