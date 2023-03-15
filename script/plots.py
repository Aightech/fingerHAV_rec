# \begin{table}[ht!]
# \begin{center}
# \caption{Comparison of audio data classifiers}
# \label{tab:audio_classifiers}
# \begin{tabular}{lllll}
# Classifiers    & Accuracy(/\%) & Precision & Recall & F1 score\\
# SVC            & 58.82  & 0.59  & 0.58 & 0.58         \\          
# Decision Tree  & 52.94  & 0.54  &0.51  & 0.52          \\
# Random Forest  & 70.59  & 0.74 & 0.7 & 0.71 \\ 
# \end{tabular}
# \end{center}
# \end{table}

# \begin{table}[ht!]
# \begin{center}
# \caption{Comparison of acceleration data classifiers}
# \label{tab:acc_classifiers}
# \begin{tabular}{lllll}
# Classifiers    & Accuracy(/\%) & Precision & Recall & F1 score\\
# SVC            & 61.20   & 0.61 & 0.61 & 0.60      \\
# Decision Tree  & 72.23   & 0.72 & 0.72 & 0.72      \\
# Random Forest  & 76.47   & 0.89 & 0.76 & 0.79       \\
# \end{tabular}
# \end{center}
# \end{table}

# \begin{table}[ht!]
# \begin{center}
# \caption{Comparison of acceleration and force data classifiers}
# \label{tab:acc_force_classifier}
# \begin{tabular}{lllll}
# Classifiers    & Accuracy(/\%) & Precision & Recall & F1 score\\
# SVC            & 60   & 0.61 & 0.60 & 0.60        \\
# Decision Tree  & 67.20   & 0.69 & 0.67 & 0.66        \\
# Random Forest  & 82    & 0.85 & 0.82 & 0.82        \\
# \end{tabular}
# \end{center}
# \end{table}


# \begin{table}[ht!]
# \begin{center}
# \caption{Classification of acceleration and audio model}
# \label{tab:acc_audio_model}
# \begin{tabular}{lllll}
# Classifiers    & Accuracy(/\%) & Precision & Recall & F1 score\\
# SVC            & 80.00 & 0.84 & 0.81 & 0.80           \\
# Decision Tree  & 86.60 & 0.86 & 0.86 & 0.86           \\
# Random Forest  & 96.67 & 0.97 & 0.96 & 0.96   \\
# \end{tabular}
# \end{center}
# \end{table}



# \begin{table}[ht!]
# \begin{center}
# \caption{Classification of accleration audio and force model}
# \label{tab:acc_audio_force_model}
# \begin{tabular}{lllll}
# Classifiers    & Accuracy(/\%) & Precision & Recall & F1 score\\
# SVC            & 82 & 0.84 & 0.82 & 0.82           \\
# Decision Tree  & 86.6 & 0.89 & 0.86 & 0.86           \\
# Random Forest  & 97.3 & 0.97 & 0.96 & 0.96   \\
# \end{tabular}
# \end{center}
# \end{table}

#aud=0.16,acc=0.10,acc+force=0.18, aud+acc=0.12,acc+forc+aud=0.13


import matplotlib.pyplot as plt
import numpy as np

# data to plot
n_classifiers = 3
classifiers = ('SVC', 'Decision Tree', 'Random Forest')
types = ('Audio', 'Acceleration', 'Acceleration+Load', 'Audio+Acceleration', 'Acceleration+Load+Audio')

accuracy_SVC = (58.82, 61.20, 60, 80, 82)
accuracy_DT = (52.94, 72.23, 67.20, 86.60, 86.60)
accuracy_RF = (70.59, 76.47, 82, 96.67, 97.30)
std_RF = (0.16, 0.10, 0.18, 0.12, 0.13)

# create plot size 20x10
fig, ax = plt.subplots(figsize=(10,5))
index = np.arange(len(types))
bar_width = 0.2
opacity = 0.8

rects1 = plt.bar(index, accuracy_SVC, bar_width,
alpha=opacity,
color='b',
label='SVC')

rects2 = plt.bar(index + bar_width, accuracy_DT, bar_width,
alpha=opacity,
color='g',
label='Decision Tree')

rects3 = plt.bar(index + 2*bar_width, accuracy_RF, bar_width,
alpha=opacity,
color='r',
label='Random Forest')



# # #create a array with the mean value of accuracy for each type of features
# # mean_accuracy = []
# # for i in range(len(types)):
# #     mean_accuracy.append((accuracy_SVC[i] + accuracy_DT[i] + accuracy_RF[i])/3)

# # #add the mean value of accuracy for each type of features as a line plot
# # #ensure the line plot is centered on the bar plot
# # plt.plot(index + bar_width, mean_accuracy, color='black', linestyle='dashed', marker='o',
# #             markerfacecolor='black', markersize=5,label="Mean Accuracy of the 3 classifiers")

# #add legend for the bar and the line plotting
# plt.legend(loc='upper left')
# plt.xlabel('Features')
# plt.ylabel('Accuracy')
# plt.title('Comparison of different features')
# plt.xticks(index + bar_width, types)
# plt.ylim(0, 100)

# plt.legend()

# plt.tight_layout()

# # create a new line plot with only the random forest accuracy and std
# # line plot for the mean accuracy of the random forest classifier
# # add error bars for the std
# plt.figure()

#plt.plot(index + 2*bar_width, accuracy_RF, color='grey', marker='o', markersize=5,label="Mean Accuracy of the Random Forest classifier")
#plt.errorbar(index + bar_width, accuracy_RF, yerr=std_RF, fmt='_', color='black', ecolor='black', elinewidth=3, capsize=0)
#for each accuracy value, add a text with the mean and std
for i in range(len(accuracy_RF)):
    plt.text(index[i] + 2*bar_width, accuracy_RF[i]+2, str(round(accuracy_RF[i],2)), fontsize=10)
    
plt.xlabel('Features')
plt.ylabel('Accuracy')
plt.title('Comparison of different features')
plt.xticks(index + bar_width, types)
plt.ylim(min([min(accuracy_SVC), min(accuracy_DT), min(accuracy_RF)])-5, max([max(accuracy_SVC), max(accuracy_DT), max(accuracy_RF)])+5)
plt.legend()


    
    
    
    

plt.tight_layout()


plt.show()
