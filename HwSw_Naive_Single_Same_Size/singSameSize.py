#!/usr/bin/env python
# coding: utf-8

# <h2>EDA Project</h2>
# <br>
# This is the Jupyter notebook template where you should insert your Python code.

# Load the image and convert it to numpy arrays.

# In[1]:


from PIL import Image as PIL_Image
import numpy

import time

from pynq import Overlay
from pynq.mmio import MMIO


# In[6]:


# load and display the image
# get image height and width
imgOrig = PIL_Image.open("../images/image0.jpg")
imgWidth, imgHeight = imgOrig.size
display(imgOrig)

BLK_HEIGHT=128
BLK_WIDTH=128

# convert the image to a numpy array with dimensions (imgHeight x imgWidth x 3)
# each entry is an array with the three RGB intensity values, from 0 to 255
imgArr = numpy.asarray(imgOrig)
print(imgOrig.size) # REMOVE!!!!!!
print(imgArr.shape) # REMOVE!!!!!!

display(PIL_Image.fromarray(imgArr))
print(imgArr.shape)


# In[7]:


padHeight = 0
padWidth = 0
# Verify necessity of padding on the height parameter
if imgHeight % BLK_HEIGHT != 0:
    padHeight = max(0, BLK_HEIGHT - imgHeight % BLK_HEIGHT)

# Verify necessity of padding on the width parameter
if imgWidth % BLK_WIDTH != 0:
    padWidth = max(0, BLK_WIDTH - imgWidth % BLK_WIDTH)
    
#print(imgHeight + padHeight, imgWidth + padWidth) ## REMOVE!!!!
#print(padHeight, padWidth) ## REMOVE!!!!

padding = ((0, padHeight), (0, padWidth), (0,0))
print(padding) ## REMOVE!!!!

imgPadArr = numpy.pad(imgArr, padding, mode='edge')
imgPadHeight, imgPadWidth, imgPadChannels = imgPadArr.shape
print(imgPadHeight, imgPadWidth)
display(PIL_Image.fromarray(imgPadArr))


# Implement your code below.

# In[13]:


print('Filtering image using hardware acceleration ...')

# start counting time
start_time = time.time()

# load overlay
overlay = Overlay('/home/xilinx/jupyter_notebooks/project/singSameSize/singSameSize.bit')

# address configuration
ADDR_BASE_IN=0x40000000
ADDR_RANGE_IN=0x10000
ADDR_BASE_OUT=0x40020000
ADDR_RANGE_OUT=0x10000

# get MMIO handlers
inMMIO = MMIO(ADDR_BASE_IN, ADDR_RANGE_IN)
outMMIO = MMIO(ADDR_BASE_OUT, ADDR_RANGE_OUT)

# view linear memory areas as 128x128 matrices
# inMat and outMat are numpy.ndarray elements with dimensions 128x128
inMat = numpy.reshape(inMMIO.array,(BLK_HEIGHT,BLK_WIDTH))
outMat = numpy.reshape(outMMIO.array,(BLK_HEIGHT,BLK_WIDTH))
#imgBlockTest = numpy.full([BLK_HEIGHT, BLK_WIDTH, 3], dtype=numpy.uint8, fill_value=0x00)
imgArrAccel=numpy.full([imgPadHeight, imgPadWidth, 3], dtype=numpy.uint8, fill_value=0x00)

# run BLK_HEIGHT x BLK_WIDTH slices of the image through the accelerator
for offsetRow in range(0, imgPadHeight, BLK_HEIGHT):
    #if offsetRow + BLK_HEIGHT > imgPadHeight:
    #    offsetRow = imgPadHeight - BLK_HEIGHT
    for offsetCol in range (0, imgPadWidth, BLK_WIDTH):
    #    if offsetCol + BLK_WIDTH > imgPadWidth:
    #        offsetCol = imgPadWidth - BLK_WIDTH

        print(offsetRow, offsetCol) ## REMOVE!!!!
        # write the BLK_HEIGHT x BLK_WIDTH slice to inMat
        for row in range(0,BLK_HEIGHT,1):
            for col in range(0,BLK_WIDTH,1):
                i = offsetRow + row
                j = offsetCol + col
                inMat[row,col] = (imgPadArr[i,j,0] << 16) | (imgPadArr[i,j,1] << 8) | imgPadArr[i,j,2]
                #imgBlockTest[row, col] = [imgArr[i,j,0], imgArr[i,j,1], imgArr[i,j,2]]

        # start the hardware kernel and wait for it to complete
        overlay.filter_Controller_0.register_map.CTRL.AP_START=1
        while overlay.filter_Controller_0.register_map.CTRL.AP_DONE == 0:
            pass
        # read the BLK_HEIGHT x BLK_WIDTH slice from outMat
        for row in range(0,BLK_HEIGHT,1):
            for col in range(0,BLK_WIDTH,1):
                i = offsetRow + row
                j = offsetCol + col
                imgArrAccel[i,j] = [0xff & (outMat[row,col] >> 16), 0xff & (outMat[row,col] >> 8), 0xff & outMat[row,col]]

# end counting time and print execution time
end_time = time.time()
execution_time = end_time - start_time
print(f'Done. Run time: {execution_time}s.')
display(PIL_Image.fromarray(imgArrAccel))


# In[14]:


imgArrAccel = imgArrAccel[:-padHeight, :-padWidth]
imgAccel = PIL_Image.fromarray(imgArrAccel)
display(imgAccel)


# In[ ]:


#### REMOVE WHAT IS NOT RELEVANT ###

#
# OPTION #1
#
imgMod = PIL_Image.fromarray(imgArrMod)
display(imgMod)

