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
angular_resolution = 2*np.pi/80
#the number of bins for the frequency axis
n_freq = 625+1
#the number of bins for the angular axis
n_angle = int(2*np.pi/angular_resolution)+1
histograms_angle = np.zeros((n_angle,))
#the matrix of frequencies energies
frequencies_acc = []
#the matrix of frequencies energies
frequencies_audio = []


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
        speed_orientation = signal.filtfilt(b, a, speed_orientation)
        #apply same filter to the speed
        #speed = signal.filtfilt(b, a, speed)

        #transform the audio signal to a spectrogram with a window of 0.1s and a step of 0.01s
        #the spectrogram is a matrix of size (n_freq, n_time)
        #the frequency axis is in Hz
        #the time axis is in seconds
        #the spectrogram is in dB

        start = 21*freq
        end = 38*freq

        audio = audio[0,start:end]
        magnitude = magnitude[start:end]
        time = time[start:end]-time[start]
        
        hop = int(0.01*freq)
        #compute spectrogram of the audio signal
        spectrogram_audio = np.abs(librosa.stft(audio, n_fft=hop*10, hop_length=hop, win_length=hop, window='hann', center=True, pad_mode='reflect'))
        #convert the spectrogram to dB
        spectrogram_audio = librosa.power_to_db(spectrogram_audio, ref=np.max)

        
        #compute spectrogram of the magnitude signal
        spectrogram_acc = np.abs(librosa.stft(magnitude, n_fft=hop*10, hop_length=hop, win_length=hop*10, window='hann', center=True, pad_mode='reflect'))
        spectrogram_acc = librosa.power_to_db(spectrogram_acc, ref=np.max)


        sheer_force = np.sqrt(ft[:,0]**2 + ft[:,1]**2)
        normal_force = ft[:,2]

        
        # #plot the two spectrogram just one above the other
        # plt.figure(figsize=(14, 5))
        # plt.subplot(2,1,1)
        # #librosa.display.specshow(spectrogram_audio, sr=freq, x_axis='time', y_axis='hz')
        # plt.pcolormesh(np.arange(spectrogram_audio.shape[1])*hop/freq, np.arange(spectrogram_audio.shape[0])*freq/spectrogram_audio.shape[0], spectrogram_audio)
        # #set limits for the frequency axis
        # plt.ylim(0, 3000)
        # #plt.pcolormesh(np.arange(spectrogram_audio.shape[1])*hop/freq, np.arange(spectrogram_audio.shape[0])*freq/spectrogram_audio.shape[0], spectrogram_audio)
        # plt.colorbar()
        # #plot the audio on the same figure with a different y axis
        # ax2 = plt.twinx()
        # ax2.plot(time, audio, color='r')
        
        # plt.subplot(2,1,2)
        # #librosa.display.specshow(spectrogram_acc, sr=freq, x_axis='time', y_axis='hz')
        # plt.pcolormesh(np.arange(spectrogram_acc.shape[1])*hop/freq, np.arange(spectrogram_acc.shape[0])*freq/n_freq, spectrogram_acc)
        # plt.colorbar()
        # ax2 = plt.twinx()
        # ax2.plot(time, magnitude, color='r')

        # #plot speed orientation
        # plt.show()
        
        # print(spectrogram.shape)
        # print(speed_orientation.shape)
        
        #plot the spectrogram
        # plt.figure(figsize=(10, 4))
        # librosa.display.specshow(spectrogram, x_axis='time', y_axis='mel', sr=freq, fmax=8000)
        # plt.colorbar(format='%+2.0f dB')
        # plt.title('Mel-frequency spectrogram')
        # plt.tight_layout()
        # plt.show()

        #compute the frequencies enegries depending on the speed orientation
        #we compute the mean of the spectrogram for each speed orientation
        
        #for each angular bin
        for i in range(spectrogram_acc.shape[1]//2):
            #get the mean speed orientation of the sample
            angle = np.mean(speed_orientation[i*hop:i*hop+hop])
            mean_speed = np.mean(speed[i*hop:i*hop+hop])
            #get the index of the angular bin
            #if angle is not a NaN
            if( (not np.isnan(angle) )):
                index = int(angle%(2*np.pi)/angular_resolution)
                #append the spectrogram sample and the angle to the list
                frequencies_acc.append(np.concatenate((spectrogram_acc[:,i], [angle],[index], [i_texture], [i_file], [mean_speed])))
                #frequencies_audio.append(np.concatenate((spectrogram_audio[:,i], [angle], [i_texture], [i_file], [texture], [mean_speed])))
        
                



        # #get the images from the current texture
        # images = [f for f in os.listdir('{}/{}/'.format(path, texture)) if f.endswith('.png')]

        # init_time = 12500

        # #open the first image
        # im = cv2.imread('{}/{}/{}'.format(path, texture, images[0]))
        # print('Processing image {}'.format(images[0]))
        # #get the size of the image
        # height, width, channels = im.shape
        # print(height, width, channels)

        #free memory
        del mat
        del audio
        del magnitude
        del speed
        del speed_orientation
        del spectrogram_audio
        del spectrogram_acc
        del time
        del ft
        del sheer_force
        del normal_force
    

        i_file += 1
    i_texture += 1


def process_polar(arr,x,y):
    # since the data is polar,
    # we create a new grid a quater of the last column in front of the first column
    # and a quater of the first column behind the last column
    # this is done to avoid the discontinuity at the end of the polar plot
    # the new grid is then interpolated
    size_angle = arr.shape[1]
    new_arr = np.zeros((arr.shape[0], size_angle+size_angle//2))
    new_arr[:,:size_angle//4] = arr[:,-size_angle//4:]
    new_arr[:,size_angle//4:size_angle+size_angle//4] = arr
    new_arr[:,size_angle+size_angle//4:] = arr[:,:size_angle//4]

    # size of x
    size_x = len(x)
    # new x is the old x with 1/4 of the last old x in front and 1/4 of the first old x behind
    new_x = np.zeros(size_x+size_x//2)
    new_x[:size_x//4] = x[-size_x//4:]
    new_x[size_x//4:size_x+size_x//4] = x
    new_x[size_x+size_x//4:] = x[:size_x//4]
        
    #apply a gaussian filter to smooth the data in the angular direction
    new_arr = gaussian_filter1d(new_arr, 1, axis=1)
        
    #new array goes from -3pi/2 to 3pi/2
    #we want it to go from -pi to pi
    size_angle = new_arr.shape[1]
    new_arr = new_arr[:,size_angle//(3*2):-size_angle//(3*2)]
    new_x = new_x[size_angle//(3*2):-size_angle//(3*2)]
    new_y = y
        
    return new_arr, new_x, new_y

                                            
frequencies_acc = np.array(frequencies_acc)
#frequencies_audio = np.array(frequencies_audio)

#save the data
np.save('frequencies_acc.npy', frequencies_acc)
#np.save('frequencies_audio.npy', frequencies_audio)

#load the data as float32
frequencies_acc = np.load('frequencies_acc.npy').astype(np.float32)
print(frequencies_acc.shape)



def compute_distance(X, y):
    # each row of X contains a samples, and each rows of Y contains the corresponding class
    # compute the distance inter class and intra class
    # return the mean distance inter class and the mean distance intra class
    # compute the mean distance between each sample of a class
    classes = np.unique(y)
    distance_intra = []
    means = []
    # for each class
    for i in range(len(classes)):
        # get the samples of the class
        #print("class {}".format(i))
        samples = X[y==classes[i]]
        mean = np.mean(samples, axis=0)
        #print(mean.shape)
        #print(mean)
        #print(samples.shape)
        means.append(mean)
        # compute the distance between each sample and the mean of the class
        #print((samples-mean))
        distance_intra.append(np.mean(np.linalg.norm(samples-mean, axis=1)))

    # from the array of means, compute the distance between each mean
    distance_inter = np.zeros((len(classes), len(classes)))
    for i in range(len(classes)):
        for j in range(i+1, len(classes)):
            distance_inter[i,j] = np.linalg.norm(means[i]-means[j])
            distance_inter[j,i] = distance_inter[i,j]
    #plot heatmap of the distance inter class
    #angle in degrees
    angle = np.arange(0, 360, 360/len(classes))
    plt.figure()
    plt.imshow(distance_inter)
    
    #plot angle 0 90 180 270
    plt.xticks([0, len(classes)//4, len(classes)//2, 3*len(classes)//4], [0, 90, 180, 270])
    plt.yticks([0, len(classes)//4, len(classes)//2, 3*len(classes)//4], [0, 90, 180, 270])
    plt.colorbar()
    plt.title('Distance inter class')
    plt.show()

    # compute the distance between the mean each class and the mean of the other class
    distance_inter = []
    for i in range(len(classes)):
        for j in range(i+1, len(classes)):
            distance_inter.append(np.linalg.norm(means[i]-means[j]))

    

    return np.mean(distance_inter), np.mean(distance_intra)
        


for freq in [frequencies_acc]:
    type = 'acceleration' if freq is frequencies_acc else 'audio'
    print(freq[:,n_freq])

    # the frequency for each sample is from column 0 to n_freq
    # then the orientation angle in radian is in column n_freq (needs modulo 2pi)
    # then the texture index is in column n_freq+1
    # then the file index is in column n_freq+2
    # then the texture name is in column n_freq+3

    # n_angle is the number of angular bins
    # n_freq is the number of frequency bins
    # for each texture id,
    # compute the distance inter and intra class with the frequency as features and the angle as a class
    for i_texture in range(len(textures)):
        #extract the samples for the current texture
        
        samples = freq[freq[:,n_freq+2]==i_texture]
        print(samples.shape)
        #extract the frequency for each sample
        X = samples[:,:n_freq]
        #extract the angle for each sample
        y = samples[:,n_freq+1]
        #compute the distance inter and intra class with the frequency as features and the angle as a class
        dist_inter, dist_intra = compute_distance(X, y)
        print('Distance inter class for texture {} with {}: {}'.format(textures[i_texture], type, dist_inter))
        print('Distance intra class for texture {} with {}: {}'.format(textures[i_texture], type, dist_intra))

        # #compute a pca on the frequency
        # pca = PCA(n_components=2)
        # pca.fit(X)
        # X_pca = pca.transform(X)
        # #plot the pca
        # plt.figure()
        # plt.scatter(X_pca[:,0], X_pca[:,1], c=y, cmap='hsv')
        # plt.colorbar()
        # plt.title('PCA for texture {} with {}'.format(textures[i_texture], type))
        # plt.savefig('pca_{}_{}.png'.format(textures[i_texture], type))
        # plt.close()

        # # compute a kmeans on frequency and compare the result with the angle class
        # kmeans = KMeans(n_clusters=n_angle)
        # kmeans.fit(X)
        # y_kmeans = kmeans.predict(X)
        # #plot the kmeans
        # plt.figure()
        # plt.scatter(y_kmeans, y, c=y, cmap='hsv')
        # plt.colorbar()
        # plt.title('Kmeans for texture {} with {}'.format(textures[i_texture], type))
        # plt.savefig('kmeans_{}_{}.png'.format(textures[i_texture], type))
        # plt.close()
        
        
        
    
    # energy = np.zeros((freq.shape[0], n_freq))
    
    # #process the frequency energies for the acceleration
    # # angle from -pi to pi
    # angle = np.linspace(-np.pi, np.pi, frequencies_energies_acc.shape[1])
    # # frequency from 0 to 12500/2
    # freq = np.linspace(0, freq/2, frequencies_energies_acc.shape[0])
    
    # #apply a gaussian filter to smooth the data in the frequency direction
    # processed_energy, new_angle, new_freq = process_polar(energy, angle, freq)
    
#     #new figure, polar plot
#     fig = plt.figure()
#     ax = fig.add_subplot(111, projection='polar')
#     #label orientation of the plot in degrees 0  90  180  270 with the symbol °
#     ax.set_xticks(np.linspace(0, 2*np.pi, 4, endpoint=False))
#     ax.set_xticklabels(['0°', '90°', '180°', '270°'])
#     # y ticks from 0, 2, 4 kHz in white on a 45° angle
#     ax.set_yticks(np.linspace(0, 4, 3)*1000)
#     ax.set_yticklabels(['0', '2kHz', '4kHz'], color='w', rotation=45)
#     plt.grid(False)
#     plt.pcolormesh(new_angle, new_freq, processed_energy, edgecolors='face')
#     plt.xlim(-np.pi, np.pi)
#     plt.ylim(0, 6000)
#     #display the scale of the heatmap on right offset by 0.1
#     plt.colorbar(format='%+2.0f dB', pad=0.1)
#     #title of the plot
#     #set the title below the plot
#     plt.title(texture, y=-0.1)    
#     #save the figure
#     #plt.savefig("spectrogram_{}_{}.png".format(type,texture),dpi=300)
#     plt.close()

#     aa = axs1 if energy is frequencies_energies_acc else axs2
#     ax = aa[i_ax%2,i_ax//2]
        
#     #label orientation of the plot in degrees -135 -45  45  135 with the symbol °
#     ax.set_xticks(np.linspace(-np.pi+np.pi/4, np.pi+np.pi/4, 4, endpoint=False))
#     ax.set_xticklabels(['225°', '315°', '45°', '135°'],fontsize=5, rotation=45, y=0.3)
    
#     # y ticks from 0, 2, 4 kHz in white on a 45° angle
#     ax.set_yticks(np.linspace(0, 5, 4)*1000)
#     ax.set_yticklabels(['0', '1', '3', '5'], color='w', rotation=45, fontsize=4)
#     ax.grid(False)
#     ax.pcolormesh(new_angle, new_freq, processed_energy, edgecolors='face')
#     ax.set_xlim(-np.pi, np.pi)
#     ax.set_ylim(0, 6000)
#     # position the title below the plot in bold
#     ax.set_title(texture, y=-0.5, fontsize=8, fontweight='bold')





# # 5 rows and 2 columns
# fig1, axs1 = plt.subplots(2,5, sharex=True, sharey=True, subplot_kw={'projection': "polar"})
# fig2, axs2 = plt.subplots(2,5, sharex=True, sharey=True, subplot_kw={'projection': "polar"})

# #offset of 0.1 between the first and the second row
# offset = 0.1
# fig1.subplots_adjust(hspace=-0.5, wspace=0.5)
# fig2.subplots_adjust(hspace=-0.5, wspace=0.5)
# # index to access the subplots
# i_ax = 0
#     i_ax += 1
# #the frequencies are in the range [0, 5000]Hz#add a general color bar for the figure with the scale of the heatmap of the first plot axs1[0,0]
# #set the color bar on the right of the figure
# fig1.colorbar(axs1[0,0].collections[0], ax=axs1, orientation='vertical', pad=0.05, label='dB',shrink=0.55)
# fig2.colorbar(axs2[0,0].collections[0], ax=axs2, orientation='vertical', pad=0.05, label='dB',shrink=0.55)

# #set title of the fig1 and fig2
# fig1.suptitle('Acceleration spectrogram of the textures', y=0.1)
# fig2.suptitle('Audio spectrogram of the textures', y=0.1)

# #save fig1 and fig2
# fig1.savefig("spectrogram_acc.png", dpi=300)
# fig2.savefig("spectrogram_audio.png", dpi=300)





# print(mean_forces)


# # get a list of unique label from the third column
# unique_labels = list(set([x[2] for x in mean_forces]))

# #get the number of unique labels and create a list of colors
# num_unique_labels = len(unique_labels)
# colors = plt.cm.rainbow(np.linspace(0, 1, num_unique_labels))

# list_ratios = []
# list_means_std = []
# arr = []
# for i, u in enumerate(unique_labels):
#     list_ratios.append([x[0]/x[1] for x in mean_forces if x[2] == u])
#     list_means_std.append([np.mean(list_ratios[i]), np.std(list_ratios[i])])
#     arr.append([list_ratios[i], list_means_std[i], colors[i], u])

# #sort the list of ratios from the smallest to the largest mean
# arr.sort(key=lambda x: x[1][0])

# #plot the data
# for i in range(len(arr)):
#     plt.plot(i, arr[i][1][0], '*' ,color=arr[i][2], label=arr[i][3])
#     plt.errorbar(i, arr[i][1][0], yerr=arr[i][1][1], color=arr[i][2],alpha=0.5)
#     #plot the individual data points with a slight random x position
#     # for j in range(len(arr[i][0])):
#     #     plt.plot(i+0.1*np.random.randn(), arr[i][0][j], 'o', color=arr[i][2], alpha=0.5)

# plt.legend()

    
    
# # # get the mean and standard deviation value of the ratio of the sheer force and normal force for each label
# # mean_std = []
# # for label in unique_labels:
# #     mean_std.append([np.mean([x[0]/x[1] for x in mean_forces if x[2] == label]), np.std([x[0]/x[1] for x in mean_forces if x[2] == label]), label])

# # # sort the list by the mean value
# # mean_std.sort(key=lambda x: x[0])

# # # plot the mean and standard deviation value of the ratio of the sheer force and normal force for each label
# # plt.figure()
        

# # # plot the ratio of the first and second column
# # for i, u in enumerate(unique_labels):
# #     x = [i for x in mean_forces if x[2] == u]
# #     y = [x[1]/x[0] for x in mean_forces if x[2] == u]
# #     plt.scatter(x, y, c=colors[i],label=str(u))
# #     #plot the mean and standard deviation value of the current label, use the i value for the x axis
# #     #plt.errorbar(i, mean_std[i][0], yerr=0, fmt='o', c=colors[i], alpha=0.5)
    
    
# plt.show()
        



