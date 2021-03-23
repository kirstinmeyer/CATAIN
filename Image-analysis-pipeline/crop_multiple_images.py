#Code to crop all CATAIN images to the same dimensions and remove the blurry edges

#put all photos to be cropped in a single directory with nothing else in it 
#replace the path, crop coordinates, and image name and run code 

def mylistdir(directory):
    """A specialized version of os.listdir() that ignores files that
    start with a leading period.
    """
    filelist = os.listdir(directory)
    return [x for x in filelist
            if not (x.startswith('.'))]

from PIL import Image
import os.path
path = "/path/name" #path to directory containing all the images to be cropped
dirs = mylistdir(path)

def crop():
    for item in dirs:
        fullpath = os.path.join(path,item)
        if os.path.isfile(fullpath):
            im = Image.open(fullpath)
            f, e = os.path.splitext(fullpath)
            imCrop = im.crop((736, 216, 4752, 3416)) #coordinates (in pixels) of four corners to be cropped
            imCrop.save(f + 'Cropped736.png') #adds name to end of the origional file names (but saves it as a new image)
crop()
print ("done")