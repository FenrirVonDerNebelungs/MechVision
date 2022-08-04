
import numpy as np
import tensorflow as tf

#sparce_categorical_crossentropy does not work since requires [0,1]
def setupModel(w_preTrainHi, w_preTrainLo,x_train,y_train):
    inputs = tf.keras.Input(shape=(56,))
    layer1=tf.keras.layers.Dense(7,activation=tf.nn.tanh,name="layer1")
    layer2=tf.keras.layers.Dense(1,activation=tf.nn.tanh,name="layer2")
    model=tf.keras.Sequential()
    model.add(inputs)
    model.add(layer1)
    model.add(layer2)

    #compile 
    model.compile(optimizer='sgd',loss=tf.keras.losses.LogCosh(),metrics=['accuracy'])

    #set the weights biases are left alone
    bias_Hi=np.zeros(1)
    bias_Lo=np.zeros(7)
    model.layers[0].set_weights([w_preTrainLo,bias_Lo])
    model.layers[1].set_weights([w_preTrainHi,bias_Hi])

    #run
    model.fit(x_train,y_train,epochs=5)

    #to get the weights
    w_Lo=model.layers[0].get_weights()[0]
    b_Lo=model.layers[0].get_weights()[1]
    w_Hi=model.layers[1].get_weights()[0]
    b_Hi=model.layers[1].get_weights()[1]
    return w_Lo,b_Lo,w_Hi,b_Hi

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


def setY(sel_stamp_i, stamp_i, ang, rad):
    cur_i=int(stamp_i)
    retval=-1
    if sel_stamp_i==cur_i:
        retval=1
    return retval

def readInDat(sel_stamp_i):
    weightsSet=False
    w_Hi=np.zeros((7,1))#number of internal nodes, number of output nodes will have shape (7,1)
    w_Lo=np.zeros((56,7))#number of Xs, number of interal Nodes will have shape(56,7)
    #the 1 is actually the data dim after the array it is filled it will be above 400
    Xs=np.empty((0,56)) #this will be filled in the 1st dim (now zero) up to the total number of data substamps sig and bac dim(dataDim,56)
    Ys=np.empty(0)#filled has dim dataDim
    fin=open("TFstamps.csv")
    line=fin.readline()
    while line:
        fieldindx=0
        commaloc=line.find(",")
        fline=np.empty(0)
        while commaloc>=0:
            fieldstr=line[:commaloc]
            fline1=np.append(fline,float(fieldstr))
            fline=fline1
            fieldindx+=1
            commaloc+=1
            line=line[commaloc:]
            commaloc=line.find(",")
        #get hanging values for lunas
        X_line=fline[1:57]
        Xs0=np.append(Xs,[X_line],axis=0)
        Xs=Xs0
        tail_line = fline[120:123]
        stamp_y=setY(sel_stamp_i,fline[0],tail_line[1],tail_line[2])
        Ys0=np.append(Ys,stamp_y)
        Ys=Ys0
        if int(fline[0])==sel_stamp_i and not weightsSet:
            w_Hi_line=fline[57:64]
            w_Lo_line=fline[64:120]
            for i_node in range(7):
                w_Hi[i_node]=w_Hi_line[i_node]
            for i_node in range(7):
                w_index=i_node*8
                line_start=w_index
                line_end=w_index+8
                w_node_seg=w_Lo_line[line_start:line_end]
                for i_hanging in range(8):
                    i_w=i_hanging+w_index
                    w_Lo[i_w,i_node]=w_node_seg[i_hanging]
            weightsSet=True
        line=fin.readline()
    fin.close()
    return w_Hi,w_Lo,Xs,Ys
