import scipy.io as sio
import matplotlib.pyplot as plt # load plot tools
from scipy.interpolate import interp1d
import sys
import os
#opencv module
import cv2
import numpy as np

#import signal
from scipy import signal

import os
import matplotlib.pyplot as plt
import librosa
import librosa.display
#import IPython.display as ipd

from matplotlib.widgets import Slider, Button

from scipy.interpolate import interp2d
# 1d gaussian filter
from scipy.ndimage import gaussian_filter1d

#PCA library
from sklearn.decomposition import PCA


# get the path to the data folder from first argument
path = sys.argv[1]


#the angular resolution is 0.1 rad
angular_resolution = 2*np.pi/2
#the number of bins for the angular axis
n_angle = int(2*np.pi/angular_resolution)+1
histograms_angle = np.zeros((n_angle,))

speed_orientation_array = np.zeros([])


#find all textures in the folder
textures = [f for f in os.listdir(path) if os.path.isdir(os.path.join(path, f))]
#create a polar subploat for each texture
nb_texture = len(textures)
i_texture = 0
#for each texture
for texture in textures:
    print('Processing texture {}'.format(texture))

    #get all mat files in the folder
    files = [f for f in os.listdir('{}/{}'.format(path, texture)) if f.endswith('.mat')]

    i_file = 0
    # for each file
    for file in files:
        print('Processing file {}'.format(file))
        #open mat file
        mat = sio.loadmat('{}/{}/{}'.format(path, texture, file))
        #print(mat.keys())
        #get time, accelerations, audio , forces and positions
        freq=12500
        time = mat['time']-mat['time'][0]
        accelerations = mat['accelerations']
        magnitude = np.sqrt(np.sum(accelerations**2, axis=1))
        audio = mat['audio']
        forces = mat['forces']
        positions = mat['positions']
        ft = mat['ft'] #force and torque Fx,Fy,Fz,Tx,Ty,Tz
        
        origin_x1 = 932
        origin_y1 = 884
        origin_x2 = 939
        origin_y2 = 40
        positions[:,0] = positions[:,0] - origin_x1
        positions[:,1] = positions[:,1] - origin_y1
        positions[:,2] = positions[:,2] - origin_x2
        positions[:,3] = positions[:,3] - origin_y2

        # compute the absolute speed of the fingertip between each sample.
        # compute the dt between each sample with the time vector
        dt = np.diff(time, axis=0)
        
        # compute the speed between each sample
        speed = np.sqrt(np.sum(np.diff(positions, axis=0)**2, axis=1))
        # divide by the dt to get the speed per second
        speed = speed/dt[:,0]
        # compute the speed orientation (angle between the speed vector and the y axis) in radians
        # no modulo 2pi because we want to keep the sign
        speed_orientation = np.arctan2(np.diff(positions[:,1]), np.diff(positions[:,0]))
        # when orientation goes from -pi to pi, we want to keep the sign
        speed_orientation = np.unwrap(speed_orientation)
        
        #apply a low pass filter to the orientation with a cutoff frequency of 10Hz
        b, a = signal.butter(4, 10/(freq/2), 'low')
        #speed_orientation = signal.filtfilt(b, a, speed_orientation)
        #apply same filter to the speed
        #speed = signal.filtfilt(b, a, speed)

        #append the speed orientation to the array
        speed_orientation_array = np.append(speed_orientation_array, speed_orientation)
        print(speed_orientation_array.shape)

        start = 0*freq
        end = -1

        hist, bin_edges = np.histogram(speed_orientation[start:end], bins=n_angle, range=(-np.pi, np.pi))

        #plot the speed and orientation
        plt.figure(figsize=(20,10))
        plt.subplot(2,1,1)
        plt.plot(time[1:], speed)
        plt.xlabel('time (s)')
        plt.ylabel('speed (mm/s)')
        plt.subplot(2,1,2)
        plt.plot(time[1:], speed_orientation%(2*np.pi), 'r',marker='.')
        plt.xlabel('time (s)')
        plt.ylabel('speed orientation (rad)')

        #plot histogram of the speed orientation on polar plot
        plt.figure(figsize=(20,10))
        plt.subplot(1,1,1, projection='polar')
        plt.hist((speed_orientation%(2*np.pi))[start:end], bins=n_angle, range=(0,2*np.pi), density=True)
        
        plt.show()
        

        # add the speed orientation to the histogram
        histograms_angle += np.histogram(speed_orientation[start:end], bins=n_angle, range=(0,2*np.pi))[0]

       
    

        i_file += 1
    i_texture += 1

#save the histogram
np.save('histograms_angle.npy', histograms_angle)
#save the speed orientation array
np.save('speed_orientation_array.npy', speed_orientation_array)

##load the histogram
#histograms_angle = np.load('histograms_angle.npy')
#load the speed orientation array
speed_orientation_array = np.load('speed_orientation_array.npy')

# #plot the orientation array
# plt.figure(figsize=(20,10))
# plt.plot(speed_orientation_array)
# plt.show()

histograms_angle = np.histogram(speed_orientation_array, bins=n_angle, range=(0,2*np.pi))[0]




#create a polar plot of the histogram
fig = plt.figure()
ax = fig.add_subplot(111, projection='polar')
ax.set_theta_zero_location("N")
ax.set_theta_direction(-1)
ax.set_rmax(1000)
ax.set_rticks([100, 200, 300, 400, 500, 600, 700, 800, 900, 1000])  # less radial ticks
ax.set_rlabel_position(-22.5)  # get radial labels away from plotted line
ax.set_title("Speed orientation", va='bottom')
ax.grid(True)

#plot the histogram
ax.bar(np.linspace(0, 2*np.pi, n_angle), histograms_angle, width=angular_resolution, bottom=0.0, color='r', alpha=0.5)

plt.show()
