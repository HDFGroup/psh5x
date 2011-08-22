import h5py
from h5py import *

import numpy
from numpy import zeros, float64

h5file = h5f.create('sample1.h5', h5f.ACC_TRUNC)
dspace = h5s.create_simple((256,512), (h5s.UNLIMITED, h5s.UNLIMITED))
dtype = h5t.IEEE_F64LE.copy()

plist = h5p.create(h5p.DATASET_CREATE)
plist.set_chunk((32,64))
plist.set_deflate(7)

dset = h5d.create(h5file, 'floats', dtype, dspace, plist)

data = zeros((256,512), dtype=float64)
for i in range(256):
    data[i,i] = i
    data[i,256+i] = i

dset.write(dspace, dspace, data)

h5file.close()
