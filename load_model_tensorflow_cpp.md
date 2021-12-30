# Load model with TensorFlow C++ API

## 1. Build neural network and save model

```
from sklearn.datasets import make_classification
from tensorflow.keras import Sequential
from tensorflow.keras.layers import Dense
from tensorflow.keras.optimizers import SGD
# create the dataset
X, y = make_classification(n_samples=1000, n_features=4, n_classes=2, random_state=1)
# determine the number of input features
n_features = X.shape[1]
# define model
model = Sequential()
model.add(Dense(10, activation='relu', kernel_initializer='he_normal', input_shape=(n_features,)))
model.add(Dense(1, activation='sigmoid'))
# compile the model
sgd = SGD(learning_rate=0.001, momentum=0.8)
model.compile(optimizer=sgd, loss='binary_crossentropy')
# fit the model
model.fit(X, y, epochs=100, batch_size=32, verbose=1, validation_split=0.3)
# save model to file
model.save('model')
```

This will create a folder `model` and save model as protobuf files: 
```
ls model/
assets  keras_metadata.pb  saved_model.pb  variables
```

## 2. Load model with C++ API

Create a new file, e.g., `load_model.cpp`
```
#include <tensorflow/cc/saved_model/loader.h>
#include <tensorflow/cc/saved_model/tag_constants.h>

using namespace tensorflow;

int main() {

const std::string export_dir = "./model/";

// Load
SavedModelBundle model_bundle;
SessionOptions session_options = SessionOptions();
RunOptions run_options = RunOptions();
Status status = LoadSavedModel(session_options, run_options, export_dir, {kSavedModelTagServe}, &model_bundle);

if (!status.ok()) {
   std::cerr << "Failed: " << status;
}
   return 0;
}
```

Compile source code
```
g++ -Wall -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 \
    load_model.cpp -o load_model.o \
    -I/usr/local/tensorflow/include/ -L/usr/local/tensorflow/lib -ltensorflow_cc -ltensorflow_framework 
```

Add TensorFlow lib into lib env var
```
export LD_LIBRARY_PATH=/usr/local/tensorflow/lib/:$LD_LIBRARY_PATH
```

Run the executable
```
./load_model.o

2021-12-30 22:14:10.621434: I tensorflow/cc/saved_model/reader.cc:38] Reading SavedModel from: ./model/
2021-12-30 22:14:10.630099: I tensorflow/cc/saved_model/reader.cc:90] Reading meta graph with tags { serve }
2021-12-30 22:14:10.630299: I tensorflow/cc/saved_model/reader.cc:132] Reading SavedModel debug info (if present) from: ./model/
2021-12-30 22:14:10.714968: I tensorflow/cc/saved_model/loader.cc:211] Restoring SavedModel bundle.
2021-12-30 22:14:10.760375: I tensorflow/cc/saved_model/loader.cc:195] Running initialization op on SavedModel bundle at path: ./model/
2021-12-30 22:14:10.765069: I tensorflow/cc/saved_model/loader.cc:283] SavedModel load for tags { serve }; Status: success: OK. Took 143842 microseconds.
```
