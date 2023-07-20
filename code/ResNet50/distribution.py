import json
import os
import random
import shutil
from tqdm import tqdm


class_names = []
class_num_1 = []
class_num_2 = []
class_num_3 = []

for class_name in os.listdir("optic_17"):
    class_names.append(class_name)
    class_num_1.append(len(os.listdir(os.path.join("optic_17", class_name))))
    class_num_2.append(len(os.listdir(os.path.join("static_first_17", class_name))))
    class_num_3.append(len(os.listdir(os.path.join("static_last_17", class_name))))
    #if len(os.listdir(os.path.join("optic_1k", class_name))) == 0:
    #    os.remove(os.path.join("static_first_1k", class_name))
    #    os.remove(os.path.join("static_last_1k", class_name))
    #    os.remove(os.path.join("optic_1k", class_name))
class_dist = dict(zip(class_names, class_num_1))
with open("LUT17.json", "w") as f:
    json.dump(dict(zip(list(range(len(class_names))), class_names)), f)
    f.close()
              

total = sum(class_num_1)  
print(class_dist)
print(class_num_1, class_num_2, class_num_3)







with open("distribution.json", "w") as f:
    json.dump(class_dist, f)
    f.close()
'''
for label in tqdm(class_names):
    #print(label)
    optic_label = os.path.join("optic", label)
    first_label = os.path.join("static_first", label)
    last_label = os.path.join("static_last", label)
    optic_vids = os.listdir(optic_label)
    first_vids = os.listdir(first_label)
    last_vids = os.listdir(last_label)
    if os.path.exists("static_first_1k") is False:
        os.makedirs(os.path.join("static_first_1k", label))
        os.makedirs(os.path.join("static_last_1k", label))
        os.makedirs(os.path.join("optic_1k", label))
    i = 0
    distribution_1k = []
    if len(optic_vids) >= 1000 and len(first_vids) >= 1000 and len(last_vids) >= 1000:
        random.shuffle(optic_vids)
        for image in optic_vids:
            optic_path = os.path.join(optic_label, image)
            first_path = os.path.join(first_label, image)
            last_path = os.path.join(last_label,image)
            #print(optic_path, first_path, last_path)
            if os.path.exists(optic_path) and os.path.exists(first_path) and os.path.exists(last_path):
                shutil.copy(optic_path, optic_path.replace("optic", "optic_1k"))
                shutil.copy(first_path, first_path.replace("static_first", "static_first_1k"))
                shutil.copy(last_path, last_path.replace("static_last", "static_last_1k"))
                i += 1
            else:
                continue

            if i == 1000:
                break
    else:
        continue
'''

