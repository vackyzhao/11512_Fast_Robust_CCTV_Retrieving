import torch
print(torch.__version__)
from torch.utils.data import DataLoader
import argparse
from dataset import PIP_370K
import multiprocessing as mp
import torch.nn as nn
import torchvision
import torch.optim as optim
import os
import json
from tqdm import tqdm 
from pytorch_to_caffe_master.Pytorch.models import TwoStream, Three_Stream

parser = argparse.ArgumentParser()
parser.add_argument("--num-epochs", default=100, type=int)
parser.add_argument("--batch-size", default=32, type=int)
parser.add_argument("--lr", default=0.01, type=float)
parser.add_argument('--optic-dir', type= str, default= "E:\pip370k\optic_17")
parser.add_argument("--static-first-dir", type = str, default="E:\pip370k\static_first_17")
parser.add_argument("--static-last-dir", type=str, default="E:\pip370k\static_last_17")
parser.add_argument("--num-classes", type=str, default=16)
parser.add_argument("--checkpoint", type=str, default="checkpoint\ResNet50_16_0.pth")
args = parser.parse_args()


    


if __name__ == '__main__':
    mp.set_start_method('spawn')
    #class_names = []
    #class_num = []
    #with open("/data/pip_370k/SOC_CHINA_BACKUP/label_LUT.json") as f:
    #    class_dict = json.load(f)
    #    f.close()
    #for class_name in os.listdir(args.optic_dir):
    #    class_names.append(class_name)
    #    class_num.append(len(os.listdir(os.path.join(args.optic_dir, class_name))))
    #class_dist = dict(zip(class_names, class_num))
    #print(class_num)
    #weights = []
    #max_sample_num = max(class_num)
    #print(max_sample_num)
    #for sample_num in class_num:
    #    if sample_num < 500:
    #        weight = 1.0
    #    if sample_num < 2000 and sample_num > 500 :
    #        weight = 2.0
    #    else:
    #        weight = max_sample_num/sample_num
            #print(sample_num, weight)
    #    weights.append(weight)
        
    #weights = torch.tensor(weights)
    #print(weights)
    device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')
    pip370k = PIP_370K(args.optic_dir,
                    {
                        "first-frame" : args.static_first_dir,
                        "last-frame" : args.static_last_dir
                    })
    train_loader = DataLoader(pip370k, args.batch_size, True, pin_memory=True, num_workers=0)
    val_loader = DataLoader(pip370k, 1, True, pin_memory=True, num_workers=0)
    acc_loader = DataLoader(pip370k, 1, True, pin_memory=True, num_workers=0)

    criterion = nn.CrossEntropyLoss().cuda()
    if os.path.exists(args.checkpoint):
        print("existing checkpoint found!")
        checkpoint = torch.load(args.checkpoint)
        model = torchvision.models.resnet.resnet50(num_classes = args.num_classes)
        new_sd = {}
        for k, v in dict(checkpoint.state_dict()).items():
            if k.startswith("module."):
                k_new = k.replace("module.", "")
                new_sd[k_new] = v
        model.load_state_dict(checkpoint.state_dict())
        model = nn.DataParallel(model.cuda(),device_ids= ("cuda:0", "cuda:1"), output_device="cuda:0")
        model = model.cuda()
        print("model loaded! continue training")
    else:
       print("no existing checkpoint, creat new model")
       #model = nn.DataParallel(torchvision.models.resnet.resnet34(num_classes = args.num_classes).cuda(),device_ids= ("cuda:0", "cuda:1"), output_device="cuda:0")
       model = torchvision.models.resnet.resnet50(num_classes = args.num_classes).cuda()
       print("new model created! start training")
    
    
    
    optimizer = optim.Adam(model.parameters(), lr=args.lr)
    scheduler = optim.lr_scheduler.ExponentialLR(optimizer, 0.1)
    
    for epoch in range(args.num_epochs):
        torch.save(model, f'checkpoint\ResNet50_16_{epoch}.pth')
        for i, (features,labels) in tqdm(enumerate(train_loader)):
            #print(labels)
            labels = labels.cuda()
            features = features.cuda()
            features_copy = features.clone().detach()
            pred = model(features_copy)
            loss = criterion(pred, labels)
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
            if i %5 == 0:			
                #print(f'for epoch {epoch + 1}/{args.num_epochs}, iteration {i}  loss = {loss:.8f}')
                with open("log.txt", "a") as f:
                        f.writelines(f'for epoch {epoch + 1}/{args.num_epochs}, iteration {i}  loss = {loss:.8f}\n')
                        f.close()
            
            if i % 50 == 0:
                with torch.no_grad():
                    #print("one epoch finished, running validation")
                    n_correct = 0
                    n_sample = 0
                    val = []
                    for j, (features, labels) in enumerate(val_loader):
                        features = features.cuda()
                        labels = labels.cuda()
                        prediction = model(features)
                        val_loss = criterion(prediction,labels)
                        val.append(val_loss)
                        if j == 10:
                            break
                        
                    with open("val_log.txt", "a") as f:
                        f.writelines(f"for epoch {epoch + 1}, val_loss = {val}\n")
                        f.close()
            else:
                continue
        
        with torch.no_grad():
           #print("calculate acc")
           n_correct = 0
           n_sample = 0
           for feature, label in acc_loader:
               feature = feature.cuda()
               label = label.cuda()
               output = model(feature)
               _, prediction = torch.max(output, 1)
               n_sample += label.size(0)
               n_correct += (prediction == label).sum().item()
               if n_sample == 1000:
                   break
           with open("acc_log.txt", "a") as f:
               f.writelines(f"for {epoch + 1}, acc = {100*n_correct/n_sample}\n")
               f.close()
           print(f"for {epoch + 1}, acc = {100*n_correct/n_sample}")
         
                         
        scheduler.step()
        
            

          


