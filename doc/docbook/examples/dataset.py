import h5py
from h5py import *

import numpy
from numpy import zeros, float64

h5file = h5py.File('sample1.h5','w')

dset = h5file.create_dataset('floats', shape=(256,512), dtype='float64',
    maxshape=(None,None), chunks(32,64), compression='gzip',
    compression_opts=7)

data = zeros((256,512), dtype=float64)

for i in range(256):
    data[i,i] = i
    data[i,256+i] = i

dset = data

h5file.close()
