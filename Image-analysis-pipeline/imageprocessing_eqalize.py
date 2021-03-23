import cv2
import numpy as np
import os
import glob
img = cv2.imread('image/2020-07-15-12-31-42_512.00ms.png')
fname='image/2020-07-15-12-31-42_512.00ms.png'
outdir='processed/equalized-hist'
#outdir="scratch"
try:
    os.makedirs(outdir)
except FileExistsError:
    print("out dir exists")

clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))

for fname in glob.glob("./image/*.png"):
    img = cv2.imread(fname)
    (indir, inname) = os.path.split(fname)
    for c in range(3):
        #img[:,:,c] = clahe.apply(img[:,:,c])
        img[:,:,c] = cv2.equalizeHist(img[:,:,c])
    
    cv2.imwrite(outdir+"/"+inname, img)
    print(outdir+"/"+inname)
