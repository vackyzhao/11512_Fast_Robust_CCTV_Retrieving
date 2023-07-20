import torch
from torch.utils.data import Dataset
from torchvision.transforms import transforms
import argparse
import os
from PIL import Image, ImageOps
import json
import numpy as np
import operator
import cv2

parser = argparse.ArgumentParser()
parser.add_argument('--optic-dir', type= str, default= "H:\Datasets\pip_370k\optic_17")
parser.add_argument("--static-first-dir", type = str, default="H:\Datasets\pip_370k\static_first_17")
parser.add_argument("--static-last-dir", type=str, default="H:\Datasets\pip_370k\static_last_17")
args = parser.parse_args()

class PIP_370K(Dataset):
    def __init__(self, optic_dir=str, static_dir=dict, transform=True):
        super(PIP_370K, self).__init__()
        self.optic_dir = optic_dir
        self.first_dir = static_dir['first-frame']
        self.last_dir = static_dir['last-frame']
        self.class_list = os.listdir(optic_dir)
        self.image_list = []
        self.label_list = []
        for class_name in self.class_list:
            images = os.listdir(os.path.join((self.optic_dir), class_name))
            self.image_list += images
            self.label_list += [class_name]*len(images)
        self.image_label_LUT = dict(zip(self.image_list, self.label_list))
        with open("E:\pip370k\LUT17.json") as f:
            self.label_LUT = json.load(f)
        f.close()
        self.label_LUT = dict(zip(self.label_LUT.values(), self.label_LUT.keys()))
        self.transform = transform
        #print(self.label_LUT)


    def __len__(self):
        return len(self.image_list)

    def __getitem__(self, idx):
        image_id = self.image_list[idx]
        label_name = self.image_label_LUT[image_id]
        #print(label_name)
        path = os.path.join(label_name, image_id)
        optic_path = os.path.join(self.optic_dir, path)
        first_path = os.path.join(self.first_dir, path)
        last_path = os.path.join(self.last_dir, path)
        label_value = int(self.label_LUT[label_name])
        if os.path.exists(first_path) and os.path.exists(last_path) and os.path.exists(optic_path):
            optic_graph = np.asarray(ImageOps.grayscale(Image.open(optic_path)))
            img = ImageOps.grayscale(Image.open(optic_path))
            first_frame = np.asarray(ImageOps.grayscale(Image.open(first_path)))
            last_frame = np.asarray(ImageOps.grayscale(Image.open(last_path)))
            feature = np.stack([first_frame, optic_graph+np.divide(first_frame +last_frame, 3.0), last_frame], axis = 0)
            feature = torch.from_numpy(feature).to(torch.float32)
            if self.transform is not None:
                mean = torch.mean(feature)
                std = torch.std(feature)
                self.transform = transforms.Normalize(mean, std)
                feature = self.transform(feature)
            return feature, label_value
        
        #error handeling 
        elif os.path.exists(first_path) is False and os.path.exists(last_path) is False:
            feature = np.asarray(Image.open(optic_path))
            feature = torch.from_numpy(feature).to(torch.float32).permute(2, 0, 1)
            if self.transform is not None:
                mean = torch.mean(feature) * torch.ones((3, 224, 224),dtype=torch.float32)
                #self.transform = transforms.Normalize(mean, std)
                feature = feature - mean
                #feature = torch.unsqueeze(feature, 0)
            #print("WARNING: NO FRAME: FIRST & LAST")
            return feature, label_value
        
        elif os.path.exists(optic_path) is False and os.path.exists(first_path) is False:
            last_frame = np.asarray(ImageOps.open(last_frame))
            feature = torch.from_numpy(feature).to(torch.float32).permute(2, 0, 1)
            if self.transform is not None:
                mean = torch.mean(feature) * torch.ones((3, 224, 224),dtype=torch.float32)
               # self.transform = transforms.Normalize(mean, std)
                feature = feature - mean
                #feature = torch.unsqueeze(feature, 0)
            #print("WARNING: NO FRAME: OPTIC & LAST")
            return last_frame, label_value
        
        elif os.path.exists(optic_path) is False and os.path.exists(last_path) is False:
            feature = np.asarray(ImageOps.open(first_frame))
            feature = torch.from_numpy(feature).to(torch.float32).permute(2, 0, 1)
            if self.transform is not None:
                mean = torch.mean(feature) * torch.ones((3, 224, 224),dtype=torch.float32)
                if std == 0:
                    std = 1e-5
                #self.transform = transforms.Normalize(mean, std)
                feature = feature - mean
                #feature = torch.unsqueeze(feature, 0)
            #print("WARNING: NO FRAME: OPTIC & LAST")
            return feature, label_value
        
        elif os.path.exists(optic_path) is False and os.path.exists(first_path) is True and os.path.exists(last_path) is True:
            first_frame = np.asarray(ImageOps.grayscale(Image.open(first_path)))
            last_frame = np.asarray(ImageOps.grayscale(Image.open(last_path)))
            pad = np.zeros(first_frame.shape)
            feature = np.stack([first_frame, pad, last_frame], axis=0)
            feature = torch.from_numpy(feature).to(torch.float32)
            if self.transform is not None:
                mean = torch.mean(feature) * torch.ones((3, 224, 224),dtype=torch.float32)
                #self.transform = transforms.Normalize(mean, std)
                feature = feature - mean 
                #feature = torch.unsqueeze(feature, 0)
            #print("WARNING: NO FRAME: OPTIC")
            return feature, label_value
        
        elif os.path.exists(optic_path) is True and os.path.exists(first_path) is True and os.path.exists(last_path) is False:
            first_frame = np.asarray(ImageOps.grayscale(Image.open(first_path)))
            optic_graph = np.asarray(ImageOps.grayscale(Image.open(optic_path)))
            pad = np.zeros(first_frame.shape)
            feature = np.stack([first_frame, optic_graph, pad], axis=0)
            feature = torch.from_numpy(feature).to(torch.float32)
            if self.transform is not None:
                mean = torch.mean(feature) * torch.ones((3, 224, 224),dtype=torch.float32)
                #self.transform = transforms.Normalize(mean, std)
                feature = feature - mean
                #feature = torch.unsqueeze(feature, 0)
            #print("WARNING: NO FRAME: LAST")
            return  feature, label_value
        
        elif os.path.exists(optic_path) is True and os.path.exists(first_path) is False and os.path.exists(last_path) is True:
            optic_graph = np.asarray(ImageOps.grayscale(Image.open(optic_path)))
            last_frame = np.asarray(ImageOps.grayscale(Image.open(last_path)))
            pad = np.zeros(last_frame.shape)
            feature = np.stack([pad, optic_graph, last_frame], axis=0)
            feature = torch.from_numpy(feature).to(torch.float32)
            if self.transform is not None:
                mean = torch.mean(feature) * torch.ones((3, 224, 224),dtype=torch.float32)
                feature = feature - mean
                #feature = torch.unsqueeze(feature, 0)
            #print("WARNING: NO FRAME: FIRST")
            return feature, label_value
'''
pip370k = PIP_370K(args.optic_dir, 
                   {
                       "first-frame" : args.static_first_dir,
                       "last-frame" : args.static_last_dir
                   })

for i in tqdm(range(pip370k.__len__())):
    feature, label = pip370k.__getitem__(i)
    size = list(feature.shape)
    if operator.eq(size, [3, 224, 224]) is False:
        print("wrong", feature.shape)

'''
        
