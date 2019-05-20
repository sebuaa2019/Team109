from PIL import Image
import os, glob
 
im = Image.open('map.pgm')
im = im.crop((400,400,600,600))
im.save('map2.jpg')
        
 
