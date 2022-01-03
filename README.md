# TensorFlow C++ API

TensorFlow provides API to call its modules implemented at a low-level paradigm in C++ class and function.
https://www.tensorflow.org/api_docs/cc

## What is it exactly?

The functionality of the TensorFlow (TF) is implemented in many languages and one of them is C++, 
but Python is used as an interface (front-end) between the backend system and the users. 
The TF C++ API his built on TensorFlow Session (in the old version) and on TensorFlow ClientSession (in the new version). 
One can make use either of these to execute TensorFlow graphs that have been built using the Python API and serialized to a `GraphDef`.

## C++ API for TensorFlow

The runtime of TensorFlow is written in C++, and mostly, C++ is connected to TensorFlow through header files in `tensorflow/cc`. 
The C++ API is still in experimental stages of development, and also the documentation is being updated, 
meaning that it lacks of information and tutorial about how to use TensorFlow API.

## Content

1. [Compile TensorFlow C++ from source code](./compile_tensorflow_cpp.md)
2. [TensorFlow Serving](./tensorflow_serving.md)
3. [Tutorial: Introduction to TensorFlow Tensor](./tensorflow_tensor.md)
4. [Tutorial: load saved model](./load_model_tensorflow_cpp.md)
