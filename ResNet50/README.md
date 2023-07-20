# 本项目利用ResNet50与新颖预处理方式进行行为识别

```
--checkpoint: 每轮训练保存模型结构与权重，给出第一个checkpoint作为样例

--save: 保存对模型权重，参数量，结构的分析文件

--utils: 具有稳健性的LK光流计算。

--dataset.py: 针对Pytorch Dataloader的数据集类

--main.py: 训练流程，如optimizer, scheduler

--label_LUT.json: 事件文字与对应id的查找表

--resnet.caffemodel: caffe模型参数

--resnet.prototxt: caffe模型架构

--distribution.py 学习数据集样本分布

--preprocess.py 视频文件预处理，得到--pip370k：数据集主文件夹
					|--optic：光流图文件夹
					|--static_first：起始帧文件夹
					|--static_last: 结束帧文件夹


```
