"""
Implements the model. The model is loaded from pickle file
"""
# Some of the imports are there to support the pickle file and not directly used
import pickle

import numpy as np
import pandas as pd
from sklearn.preprocessing import MinMaxScaler
from sklearn.metrics import accuracy_score, mean_squared_error, mean_absolute_error
from sklearn.svm import SVR

def data_prep(real_sm_input):
    # For data prep
    # Random rain data over next 100 points, if flag is true it will use 
    an_array = np.zeros([1,5])
    rain_data = np.zeros([88])
    rain_random = np.array([0.05,0.01,0.19,0.02,0.61,0.038,0.4,0.06,0.5,0.31,0.09,0.07])
    rain_sample = np.hstack((rain_data, rain_random))
    np.random.shuffle(rain_sample)
    #otherwise will use zero
    rain_input = np.zeros([100])
    for key, value in real_sm_input.items():
        if value == None :
            value = 0
        if key == 'temp' :
            fernht = (value * 9/5) + 32 
            an_array[0,2] = fernht
            an_array[0,1] = fernht - 8
        if key == 'light' :
            an_array[0,4] = value
        if key == 'moisture' :
            an_array[0,0] = value
        if key == 'humidity' :
            an_array[0,3] = value
        if key == 'mock_rain' :
            if value :
                rain_input = rain_sample.copy()    
    np.reshape(rain_input,(-1,1))
    test_input = np.zeros([100,6])
    test_input[:,5] = rain_input
    test_input[:,1] =  fernht - 8
    test_input[:,2] =  fernht
    test_input[:,3] =  an_array[0,3]
    test_input[:,4] = an_array[0,4]
    test_input[0,0:5] = an_array
    return test_input

def load_model():
    filename = 'smart-ag-iotc/finalized_model.sav'
    scaler = "smart-ag-iotc/scaler.pkl"
    loaded_model = pickle.load(open(filename, 'rb'))
    scaler = pickle.load(open(scaler, 'rb'))
    return(loaded_model, scaler)

def predict_sm_next_100(test_input):
    loaded_model,scaler = load_model()
    for idx in range(99):
        input = test_input[idx,:]
        new_input = np.reshape(input, (-1, 6))
        x_test_scaled = scaler.transform(new_input)
        result = loaded_model.predict(x_test_scaled)
        #print(result,Test_input[idx,0])
        test_input[idx+1, 0] = result
    return result

def predict_sm(real_sm_inp):
    test_input = data_prep(real_sm_inp)
    return predict_sm_next_100(test_input)[0]
    
