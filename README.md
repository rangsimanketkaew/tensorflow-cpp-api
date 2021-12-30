# TensorFlow C++ API

Building TensorFlow C++ API can be a pain as there is not much information you can find about it even on TensorFlow's officiel documentation. The following you can find a step-by-step instruction showing how to build TensorFlow C++ v2 on Linux. It works well for my Ubuntu 20.04 & AMD Ryzen processors. 

## Dependencies 

- Conda environment
- Python 3.9.0
- TensorFlow 2.7
- Bazel 3.7.2
- Protobuf 3.9.2 (must be compatible with the version of TensorFlow-built protobuf or protoc)

## Environment setup & install Python
```
conda create -n tfcc
conda activate tfcc
conda install python
conda update --all -y
```

## Install bazel
```
sudo apt install bazel-3.7.2
```

## Install TensorFlow CC
```
git clone https://github.com/tensorflow/tensorflow
cd tensorflow
git checkout r2.7
```

---

## 1. Compile TF shared library (with optimization)
```
export CC=gcc
export CXX=g++
bazel build --jobs=4 --cxxopt="-D_GLIBCXX_USE_CXX11_ABI=0" -c opt \
            //tensorflow:libtensorflow.so \
            //tensorflow:libtensorflow_cc.so \
            //tensorflow:libtensorflow_framework.so \
            //tensorflow:install_headers \
            //tensorflow/tools/pip_package:build_pip_package
```

Note:

1. Building TF uses a lot of memory, I prefer a small number of CPUs (`--jobs`)
2. The whole process can take several hours
3. Add `-D_GLIBCXX_USE_CXX11_ABI=0` if you use GCC 5 or higher
4. Flags for optimization: `--copt="-O3"`
5. Flasg for both AMD and Intel chips: `--copt=-mfma --copt=-msse4.1 --copt=-msse4.2 --copt=-mfpmath=both`
6. Flags for Intel: `--copt=-mavx --copt=-mavx2`
7. Rebuild with `--config=monolithic` if you want compile all TF C++ code into a single shared object

**optional**
```
bazel test --jobs=4 --cxxopt="-D_GLIBCXX_USE_CXX11_ABI=0" -c opt \
           //tensorflow/tools/lib_package:libtensorflow_test
```

## 2. Install protobuf

1. Check the version of protobuf that TF is built with
   ```
   bazel-bin/external/com_google_protobuf/protoc --version
   libprotoc 3.9.2
   ```
2. Download protobuf source code from its GitHub release https://github.com/protocolbuffers/protobuf/tags
3. Compile and link
   ```
   ./configure --prefix=/home/rangsiman/protobuf-3.9.2/
   make
   make check
   make install
   ```

## 3. Copy required files into a single path for C++ linkage
```
sudo mkdir /usr/local/tensorflow
sudo cp -r bazel-bin/tensorflow/include/ /usr/local/tensorflow/
sudo cp -r /home/rangsiman/protobuf-3.9.2/include/google/ /usr/local/tensorflow/include/
sudo mkdir /usr/local/tensorflow/lib
sudo cp -r bazel-bin/tensorflow/*.so* /usr/local/tensorflow/lib
sudo cp -r /home/rangsiman/protobuf-3.9.2/lib/*.so* /usr/local/tensorflow/lib
```

## 4. Compiling the op library and example code

**Example-1**: Zero out

Create `zero_out.cpp`
```
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"

using namespace tensorflow;

REGISTER_OP("ZeroOut")
    .Input("to_zero: int32")
    .Output("zeroed: int32")
    .SetShapeFn([](::tensorflow::shape_inference::InferenceContext* c) {
      c->set_output(0, c->input(0));
      return Status::OK();
    });
```

Run the following
```
g++ -Wall -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 \
    -shared zero_out.cpp -o zero_out.so \
    -I/usr/local/tensorflow/include/ -L/usr/local/tensorflow/lib \
    -ltensorflow_cc -ltensorflow_framework
```

**Example-2**: Call TF session

Create `session.cpp`
```
#include <tensorflow/core/platform/env.h>
#include <tensorflow/core/public/session.h>

#include <iostream>

using namespace std;
using namespace tensorflow;

int main()
{
    Session* session;
    Status status = NewSession(SessionOptions(), &session);
    if (!status.ok()) {
        cout << status.ToString() << "\n";
        return 1;
    }
    cout << "Session successfully created.\n";
}
```
            
Run the following
```
g++ -Wall -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 \
    session.cpp -o session \
    -I/usr/local/tensorflow/include/ -L/usr/local/tensorflow/lib \
    -ltensorflow_cc -ltensorflow_framework
```

To run the executable, you also need to add `/usr/local/tensorflow/lib/` into `LD_LIBRARY_PATH` env var.

---

## Optional: Compile TF via pip (wheel) builder
```
## create a wheel package
./bazel-bin/tensorflow/tools/pip_package/build_pip_package /tmp/tensorflow_pkg

## install TF using a created wheel
pip install /tmp/tensorflow_pkg/tensorflow-*.whl
```

## References
1. https://www.tensorflow.org/guide/create_op#compile_the_op_using_your_system_compiler_tensorflow_binary_installation
2. https://www.tensorflow.org/install/source#bazel_build_options
