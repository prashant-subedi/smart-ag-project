import logging
import numpy as np
import pandas as pd
from sklearn.preprocessing import MinMaxScaler
from sklearn.metrics import accuracy_score, mean_squared_error, mean_absolute_error
from sklearn.svm import SVR
import pickle
import json

def data_prep(Real_Sm_inp):
    # For data prep
    # Random rain data over next 100 points, if flag is true it will use 
    an_array = np.zeros([1,5])
    rain_data = np.zeros([88])
    rain_random = np.array([0.05,0.01,0.19,0.02,0.61,0.038,0.4,0.06,0.5,0.31,0.09,0.07])
    rain_sample = np.hstack((rain_data,rain_random))
    np.random.shuffle(rain_sample)
    #otherwise will use zero
    rain_input = np.zeros([100])
    for key, value in Real_Sm_inp.items():
        if value == None :
            value = 0
        if key == 'temp' :
            Fernht = (value * 9/5) + 32 
            an_array[0,2] = Fernht
            an_array[0,1] = Fernht - 8
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
    Test_input = np.zeros([100,6])
    Test_input[:,5] = rain_input
    Test_input[:,1] =  Fernht - 8
    Test_input[:,2] =  Fernht
    Test_input[:,3] =  an_array[0,3]
    Test_input[:,4] = an_array[0,4]
    Test_input[0,0:5] = an_array
    return Test_input

def load_model():
    import os, logging
    logging.info(os.getcwd())
    filename = 'smart-ag-iotc/finalized_model.sav'
    loaded_model = pickle.load(open(filename, 'rb'))
    scaler = pickle.load(open('smart-ag-iotc/scaler.pkl', 'rb'))
    return(loaded_model,scaler)

def predict_SM_next_100(Test_input):
    loaded_model,scaler = load_model()
    for idx in range(99):
        Input = Test_input[idx,:]
        new_Input = np.reshape(Input, (-1, 6))
        X_test_scaled = scaler.transform(new_Input)
        result = loaded_model.predict(X_test_scaled)
        #print(result,Test_input[idx,0])
        Test_input[idx+1,0] = result
    return result

def predict_SM(Real_Sm_inp):
    Test_Input = data_prep(Real_Sm_inp)
    return predict_SM_next_100(Test_Input)[0]
    
