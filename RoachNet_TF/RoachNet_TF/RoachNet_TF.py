
import numpy as np
import tensorflow as tf

#x_train has dimensions?
def runTF(w_preTrainHi, w_preTrainLo, x_train, y_train):
    inputs = tf.keras.Input(shape=(56,))
    x = tf.keras.layers.Dense(7, activation=tf.nn.tanh)(inputs)
    outputs=tf.keras.layers.Dense(1, activation=tf.nn.tanh)(x)
    x.set_weights(w_preTrainLo)
    outputs.set_weights(w_preTrainHi)
    model=tf.keras.Model(inputs=inputs, outputs=outputs)
    model.compile(optimizer=tf.keras.optimizers.Adam(learing_rate=1e-3),loss=tf.keras.losses.MeanSquaredError(),metrics=['accuracy'])
    model.fit(x_train,y_train,epochs=5)
    model.evaluate(x_train,y_train)#better if x_test, y_test is used instead of running eval on same data as training
    w_Lo=model.layers[1].get_weights()
    w_Hi=model.layers[2].get_weights()
    return w_Hi,w_Lo

def getWeightsIn():
    w_Hi=np.zeros(7,1)#number of internal nodes, number of output nodes
    w_Lo=np.zeros(56,7)#number of Xs, number of interal Nodes
    return w_Hi, w_Lo

def getXYIn():
    #the 1 is actually the data dim after the array it is filled it will be above 400
    Xs=np.zeros(1,56)
    Ys=np.zeros(1)
    return Xs,Ys

