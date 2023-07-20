import os
import torch
import numpy as np
import argparse
import cv2
from tqdm import tqdm
from utils.preprocess import Rubost_OpticalFlow_KL
import json
import random

parser = argparse.ArgumentParser()
parser.add_argument("--dataset", default="H:\Datasets\pip_370k", type = str)
parser.add_argument("--num-frames", default= 16, type=int)
parser.add_argument("--export-dir", default="H:\Datasets\pip_370k\optic", type=str)

args = parser.parse_args()

class Preprocess():
    def __init__(self, dataset=str, num_frames=int, transform=str, resize=tuple):
        if(transform not in {'BGR_to_RGB','BGR_to_GRAYSCALE'}):
            raise ValueError('unexpected transform type')
        self.resize = resize
        self.transform = transform
        self.dataset = dataset
        self.stablized = os.path.join(dataset, "stablized")
        self.class_list = os.listdir(self.stablized)
        self.num_frames = num_frames
        self.skip = 0
        #print(self.class_list)
        
    def OpticalFlow(self, video):
        #if os.path.exists(export_path) == False:
        #    os.makedirs(export_path)

        optic_vec = Rubost_OpticalFlow_KL(video)
        return optic_vec

    def StaticFlow(self, video):
        cap = cv2.VideoCapture(video)                         
        self.frame_count = cap.get(cv2.CAP_PROP_FRAME_COUNT)
        self.frame_rate = cap.get(cv2.CAP_PROP_FPS)
        self.stride = int(self.frame_count // self.num_frames)
        #print(self.stride)
        if (self.stride == 0):
            self.stride = 1
            


        frames = []
        transform = {"BGR_to_RGB" : cv2.COLOR_BGR2RGB,  "BGR_to_GRAYSCALE" : cv2.COLOR_BGR2GRAY }
        count = 0
        i = 0
        while cap.isOpened():
            ret, frame = cap.read()
            i += 1
            if i % self.stride == 0:
                frame = cv2.cvtColor(frame, transform[self.transform])
                if transform == "COLOR_BGR2RGB":
                    frame = frame.transpose(2, 0, 1)

                frames.append(frame)
                count = count + 1
            if count == self.num_frames or i == self.frame_count:
                cap.release()
                break
        #print(self.frame_count, self.frame_rate)
        if (self.frame_count < self.frame_rate*5):
            #print(len(frames))
            return frames[0], frames[0]
        else:
            return frames[0], frames[-1]
            #if(len(frames) > self.num_frames*0.5 and len(frames) < self.num_frames):
            #    frames.append(torch.zeros([self.num_frames - len(frames), self.resize[0], self.resize[1]]).to(torch.float32))
        
        
    def Generate(self):
        video_list = []
        labels = []
        for label in tqdm(self.class_list):
            vids =  os.listdir(os.path.join(self.stablized, label))
            video_list += vids
            labels += [label]*len(vids)
        print(len(video_list), len(labels))
        videos_labels_dict = dict(zip(video_list, labels))
        label = list(range(len(self.class_list)))
        label_LUT = dict(zip(label, self.class_list))
        with open("label_LUT.jason", "w") as f:
            json.dump(label_LUT, f)
        f.close()
        print('label loaded!')
        print(f"sample count :{len(video_list)}, sample format: {video_list[0]}")
        
        random.shuffle(video_list)
        for item in tqdm(video_list):
            try:
                vid_class = os.path.join(self.stablized, videos_labels_dict[item])
                optic_export_dir = os.path.join(args.export_dir, videos_labels_dict[item])
                static_first_export_dir = optic_export_dir.replace("optic", "static_first")
                static_last_export_dir = optic_export_dir.replace("optic", "static_last")
                if os.path.exists(optic_export_dir) is False:
                    os.makedirs(optic_export_dir)
                
                if os.path.exists(static_first_export_dir) is False:
                    os.makedirs(static_first_export_dir)
                
                if os.path.exists(static_last_export_dir) is False:
                    os.makedirs(static_last_export_dir)
                
            except:
                exit()
            video = os.path.join(vid_class, item)
            item = item.replace(".mp4", ".jpg")
            #args.export_dir = os.path.join(args.export_dir, item.replace("mp4", "jpg"))
            #print(args.export_dir+"\n")
            optic_export_dir = os.path.join(optic_export_dir, item)
            if os.path.exists(optic_export_dir) is True:
                continue
            static_first_export_dir = optic_export_dir.replace("optic", "static_first")
            static_last_export_dir = optic_export_dir.replace("optic", "static_last")
            first_frame, last_frame = self.StaticFlow(video)
            if self.frame_count < self.frame_rate*5:
                continue
            else:
                optical_graph = self.OpticalFlow(video)
                cv2.imwrite(optic_export_dir, optical_graph)
                cv2.imwrite(static_first_export_dir, first_frame)
                cv2.imwrite(static_last_export_dir, last_frame)







Preprocess(args.dataset,
           16,
           "BGR_to_RGB",
           (224, 224)).Generate()