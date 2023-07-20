


--checkpoint：存储模型架构和每个时期的权重。为了简化，只提供了1个检查点。<br />
--save：state_dict()写入analysis.csv <br />
--utils：具有稳健特性的LK光流算法 <br />
--dataset.py：专门用于PyTorch DataLoader的类 <br />
--main.py：训练流程 <br />
--label_LUT.json：标签-ID查找表 <br />
--resnet.caffemodel：训练后的Torch模型转换为Caffe框架 <br />
--resnet.prototxt：适用于Caffe框架的模型架构。 <br />
