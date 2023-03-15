import scipy.io as sio
import matplotlib.pyplot as plt # load plot tools
from scipy.interpolate import interp1d
import sys
import os
#opencv module
import cv2
import numpy as np


# values are store in dictionary with key 'values'
# timestamps are stored in dictionary with key 'time'
# sort by time
def sort_by_time(mat):
    values = mat['values']
    time = mat['time']
    values = values[time[:,0].argsort()]
    time = time[time[:,0].argsort()]
    return values, time


# function getting the path id to the data folder and applying the preprocessing
def preproc_data(path, texture, id):
    # load data
    mat_kistler = sio.loadmat('{}/{}/{}_{}/data_Kistler_{}_{}.mat'.format(path, texture, texture, id, texture, id))
    mat_forces = sio.loadmat('{}/{}/{}_{}/data_forces_{}_{}.mat'.format(path, texture, texture, id, texture, id))
    mat_positions = sio.loadmat('{}/{}/{}_{}/data_positions_{}_{}.mat'.format(path, texture, texture, id, texture, id))

    # sort by time
    forces, forces_time = sort_by_time(mat_forces)
    positions, positions_time = sort_by_time(mat_positions)
    kistler, kistler_time = sort_by_time(mat_kistler)

    # get the latest initial time
    init_time = max(forces_time[0,0], positions_time[0,0])
    # get the earliest final time
    final_time = min(forces_time[-1,0], positions_time[-1,0])
    
    # ensure kistler and kistler_time are within the range
    kistler = kistler[(kistler_time[:,0] >= init_time) & (kistler_time[:,0] <= final_time)]
    kistler_time = kistler_time[(kistler_time[:,0] >= init_time) & (kistler_time[:,0] <= final_time)]

    # get the mean during the first 20 samples of each of the 8 force channels
    mean_forces = np.mean(forces[:20,:], axis=0)
    #for each of the 8 channels, if a value is greater or lower than 10000 from the mean, set it to the previous value
    for i in range(forces.shape[1]):
        if( abs(forces[0,i] - mean_forces[i]) > 10000):
            forces[0,i] = mean_forces[i]
        for j in range(1,forces.shape[0]):
            if abs(forces[j,i]-mean_forces[i]) > 10000:
                forces[j,i] = forces[j-1,i]
    #remove the mean from the force channels
    forces = forces - mean_forces

    
    #regression from the 8 force channels to the 6 F/T channels, using the calibration matrix A
    #A is a 42x6 matrix, 42 = (f1,f2,f3,f4,f5,f6,f7,f8, f1*f1, f1*f2, f2*f2, f1*f3, f2*f3, f3*f3, f1*f4, f2*f4, f3*f4, f4*f4, f1*f5, f2*f5, f3*f5, f4*f5, f5*f5, f1*f6, f2*f6, f3*f6, f4*f6, f5*f6, f6*f6, f1*f7, f2*f7, f3*f7, f4*f7, f5*f7, f6*f7, f7*f7, f1*f8, f2*f8, f3*f8, f4*f8, f5*f8, f6*f8, f7*f8, f8*f8)
    #and 6 = (Fx, Fy, Fz, Tx, Ty, Tz)
    # creation of the forces signal
    ft = np.zeros((forces.shape[0], 6))
    forces_extend = np.zeros((forces.shape[0], 44))
    forces_extend[:,:8] = forces
    k=0
    for j in range(8):
        for i in range(j+1):
            forces_extend[:,8+k] = forces[:,i]*forces[:,j]
            k+=1
    ft = np.matmul(forces_extend, A)
    # decinewton to newton
    ft[:,0:3] = ft[:,0:3]/10
    # milinewton.m to newton.m
    ft[:,3:6] = ft[:,3:6]/1000

    # create a subplot, one for the forces and one for the torques
    fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)
    # plot the forces
    ax1.plot(forces_time[:,0], ft[:,0], label='Fx')
    ax1.plot(forces_time[:,0], ft[:,1], label='Fy')
    ax1.plot(forces_time[:,0], ft[:,2], label='Fz')
    ax1.legend()
    ax1.set_ylabel('Force (N)')
    # plot the torques
    ax2.plot(forces_time[:,0], ft[:,3], label='Tx')
    ax2.plot(forces_time[:,0], ft[:,4], label='Ty')
    ax2.plot(forces_time[:,0], ft[:,5], label='Tz')
    ax2.legend()
    ax2.set_ylabel('Torque (Nm)')
    ax2.set_xlabel('Time (s)')
    
    # save the plots
    plt.savefig('{}_{}/{}/plot_forces_{}_{}.svg'.format(path, folder_suffix, texture, texture, id), format='svg', dpi=1200)
    #close the plot
    plt.close()
    
    # upsample forces to match kistler timestamps
    f = interp1d(forces_time[:,0], forces, axis=0, kind='linear')
    forces = f(kistler_time[:,0])

    # upsample ft to match kistler timestamps
    f = interp1d(forces_time[:,0], ft, axis=0, kind='linear')
    ft = f(kistler_time[:,0])


    #position is composed of(x1,y1) and (x2,y2)
    x1 = positions[:,0]
    y1 = positions[:,1]
    x2 = positions[:,2]
    y2 = positions[:,3]
    
    #get the mean offset between x1 and x2
    x_offset = np.mean(x1-x2)
    #get the mean offset between y1 and y2
    y_offset = np.mean(y1-y2)
    
    # if a value is negative, it means the position was not detected.
    # we need to interpolate the missing values from the other signal
    # we use the mean offset to interpolate the missing values if the second signal is positive
    # if the second signal is negative, we use the first signal to interpolate the missing values
    # if both signals are negative, we use the previous value
    for j in range(positions.shape[0]):
        if x1[j] < 0 and x2[j] > 0:
            x1[j] = x2[j] + x_offset
        elif x2[j] < 0 and x1[j] > 0:
            x2[j] = x1[j] + x_offset
        elif x1[j] < 0 and x2[j] < 0:
            x1[j] = x1[j-1]
            x2[j] = x2[j-1]

        if y1[j] < 0 and y2[j] > 0:
            y1[j] = y2[j] + y_offset
        elif y1[j] > 0 and y2[j] < 0:
            y2[j] = y1[j] + y_offset
        elif y1[j] < 0 and y2[j] < 0:
            y1[j] = y1[j-1]
            y2[j] = y2[j-1]
    
    #to ensure there is no peak in the signal
    #we check if no value is higher or lower than 600 form the previous value
    #we replace the peaks by the previous value
    for i in range(positions.shape[1]):
        for j in range(1,positions.shape[0]):
            if abs(positions[j,i] - positions[j-1,i]) > 600:
                positions[j,i] = positions[j-1,i]

    origin_x1 = 932
    origin_y1 = 884
    origin_x2 = 939
    origin_y2 = 40
    # remove from positions the origin
    x1 = x1 - origin_x1
    y1 = y1 - origin_y1
    x2 = x2 - origin_x2
    y2 = y2 - origin_y2
    
    #plot
    plt.figure()
    for i in range(positions.shape[1]):
        plt.plot(positions_time, positions[:,i])
    #legend
    plt.legend(['x1','y1','x2','y2'])
    # save the plots
    plt.savefig('{}_{}/{}/plot_positions_{}_{}.svg'.format(path, folder_suffix, texture, texture, id), format='svg', dpi=1200)
    #close the plot
    plt.close()
    
    # upsample positions to match kistler timestamps
    f = interp1d(positions_time[:,0], positions, axis=0, kind='linear')
    positions = f(kistler_time[:,0])

    # Microphone data
    audio = kistler[:,0]
    # 3axis accelerometer data
    acceleration = kistler[:,1:4]

    # save all data in a new file
    sio.savemat('{}_{}/{}/data_all_{}_{}.mat'.format(path, folder_suffix, texture, texture, id, texture, id), {'time': kistler_time, 'audio': audio, 'accelerations': acceleration, 'forces': forces, "ft":ft , 'positions': positions})
    

# get the path to the data folder from first argument
path = sys.argv[1]

folder_suffix = 'processed2'

A = sio.loadmat('f82ft_1_2_comb.mat')["A"]

#create an other folder to save the processed data
if not os.path.exists('{}_{}'.format(path, folder_suffix)):
    os.makedirs('{}_{}'.format(path, folder_suffix))

#find all textures in the folder
textures = [f for f in os.listdir(path) if os.path.isdir(os.path.join(path, f))]

# for each texture
for texture in textures:
    print('Processing texture {}'.format(texture))
    
    # find all id in the folder with format texture_id
    ids = [f.split('_')[1] for f in os.listdir('{}/{}'.format(path, texture)) if os.path.isdir(os.path.join(path, texture, f))]
    # create a folder to save this texture_id
    if not os.path.exists('{}_{}/{}'.format(path, folder_suffix, texture)):
        os.makedirs('{}_{}/{}'.format(path, folder_suffix, texture))
    # for each id
    for id in ids:
        print('Processing id {}'.format(id))
        #check if the 3 mat files exist
        if(os.path.isfile('{}/{}/{}_{}/data_Kistler_{}_{}.mat'.format(path, texture, texture, id, texture, id)) and
           os.path.isfile('{}/{}/{}_{}/data_forces_{}_{}.mat'.format(path, texture, texture, id, texture, id)) and
           os.path.isfile('{}/{}/{}_{}/data_positions_{}_{}.mat'.format(path, texture, texture, id, texture, id))):
            #apply preprocessing
            preproc_data(path, texture, id)
        else:
            print('Missing data for texture {} id {}'.format(texture, id))
        
    # # get all the images
    # images0 = [f for f in os.listdir('{}/{}/'.format(path, texture, texture, id)) if f.endswith('0.png')]
    # images1 = [f for f in os.listdir('{}/{}/'.format(path, texture, texture, id)) if f.endswith('1.png')]
    # images = np.array([images0, images1])
    
    # crop_rect_0 = np.array([(2492, 902), (2479, 2436), (932, 2405), (952, 884)], dtype = "float32")
    # crop_rect_1 = np.array([(939, 41), (2489, 40), (2475, 1588), (939, 1567)], dtype = "float32")
    # crop_rect = np.array([crop_rect_0, crop_rect_1])

    # for cam in ['0', '1']:
    #     for img in images[int(cam)]:
    #         print('{}/{}/{}'.format(path, texture, img))
    #         #load image
    #         image = cv2.imread('{}/{}/{}'.format(path, texture, img))
    #         # crop image
    #         pts = np.array(crop_rect[int(cam)], dtype = "float32")
    #         rect = cv2.boundingRect(pts)
    #         x, y, w, h = rect
    #         croped = image[y:y+h, x:x+w].copy()
    #         #save the image
    #         cv2.imwrite('{}/{}/{}'.format(path, texture, img), croped)
        



# plot
#mat = mat_kistler
# plt.plot(mat['counter'], mat['values'])
# plt.show()
