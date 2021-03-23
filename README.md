# CATAIN
Image capture and image processing scripts for the Camera To Analyze Invertebrates

CATAIN development team: <br>
Kirstin Meyer-Kaiser, marine biologist and PI (kmeyer@whoi.edu) <br>
Stefano Suman, software engineer (ssuman@whoi.edu) <br>
John Bailey, electrical engineer (jbailey@whoi.edu) <br>
Yogesh Girdhar, imagery and autonomy (ygirdhar@whoi.edu) <br>
Kharis Schrage, MIT-WHOI Joint Program PhD student (kharis@mit.edu) <br>

This repository has the following directories: <br>
Image capture software - codes in C++ and Python by Stefano Suman <br>
Disk image - a full image of CATAIN's SD card by Stefano Suman <br>
Image analysis pipeline - codes in Python by Kharis Schrage and Yogesh Girdhar <br>

Disk image: <br>
CATAIN's disk image includes all image capture software as well as the Linux operating system and writable data folders. When offloading data from CATAIN, users should copy files from the 'image' and 'log' folders in the 'writable' directory without deleting the folders themselves. We highly encourage CATAIN users to reach out to the development team for assistance when assembling their system. 

Image analysis pipeline: <br>
1. imageprocessing_eqalize.py can be used for Contrast Limited Adaptive Histogram Equalization (CLAHE) processing of CATAIN images. This step makes the organisms in the images easier to visualize.
2. crop_multiple_images.py can be used to crop CATAIN images to a set size, thus eliminating the distorted edges of the images.
3. Organisms in CATAIN images can be annotated using the program Labelme, which is available elsewhere on Github (wkentaro/labelme). This program exports the pixel coordinates of polygons and points used to mark organisms as JSON files.
4. multiple_JSON_to_csv.py can be used to extract data from all JSON files in a directory and place them into a CSV spreadsheet. This step allows all CATAIN data to be collated into a single table showing the size (in pixels) of each organism at each time-point.
