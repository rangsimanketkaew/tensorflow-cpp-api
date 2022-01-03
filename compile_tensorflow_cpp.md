# Compile TensorFlow C++ from source code

Building TensorFlow C++ API is very tricky and can be a pain as there is not much information you can find about it even on TensorFlow's official documentation. Following you will find a step-by-step instruction showing how to build TensorFlow C++ v2 on Linux. It works well for my Ubuntu 20.04 running on AMD Ryzen processors.

In this page I will walk you through the steps to install TensorFlow C++ API version 2.7.

## Dependencies

- Conda
- Python 3.9.0
- GCC 5 or newer
- Bazel 3.7.2
- Protobuf 3.9.2 (must be compatible with the version of TensorFlow-built protobuf or protoc)

Check a list of supported Python version, compiler, and bazel at https://www.tensorflow.org/install/source#tested_build_configurations.

---

## Install package dependencies

### 1. Environment setup & install Python
```bash
conda create -n tfcc
conda activate tfcc
conda install python==3.9
conda update --all -y
```

### 2. Install bazel

Add bazel repository:
```bash
sudo apt install apt-transport-https curl gnupg
curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor > bazel.gpg
sudo mv bazel.gpg /etc/apt/trusted.gpg.d/
echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
```

Install:
```bash
sudo apt install bazel
# or a specific version
sudo apt install bazel-3.7.2
```

### 3. Install Protobuf

I suggest installing protobuf after building TensorFlow so that we are able to check that which version of protobuf we should use.

---

## Compile TensorFlow C++ and install libraries

### 1. Compile TensorFlow C++ shared library (with optimization)

Download or clone github repo to your system:
```bash
git clone https://github.com/tensorflow/tensorflow
cd tensorflow
git checkout r2.7
```

Let's compile using bazel `build` rule:
```bash
export CC=gcc
export CXX=g++
bazel build --jobs=4 --cxxopt="-D_GLIBCXX_USE_CXX11_ABI=0" -c opt \
            //tensorflow:libtensorflow.so \
            //tensorflow:libtensorflow_cc.so \
            //tensorflow:libtensorflow_framework.so \
            //tensorflow:install_headers \
            //tensorflow/tools/pip_package:build_pip_package
```

You can use the following command to check all available rules in each folder:
```bash
bazel query ...
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
```bash
bazel test --jobs=4 --cxxopt="-D_GLIBCXX_USE_CXX11_ABI=0" -c opt \
           //tensorflow/tools/lib_package:libtensorflow_test
```

### 2. Install protobuf

1. Check the version of protobuf that TF is built with
   ```bash
   bazel-bin/external/com_google_protobuf/protoc --version
   libprotoc 3.9.2
   ```
2. Download protobuf source code from its GitHub release https://github.com/protocolbuffers/protobuf/tags
3. Compile and link
   ```bash
   ./configure --prefix=/home/rangsiman/protobuf-3.9.2/
   make
   make check
   make install
   ```

### 3. Copy required files into a single path for C++ linkage

```bash
sudo mkdir /usr/local/tensorflow
sudo cp -r bazel-bin/tensorflow/include/ /usr/local/tensorflow/
sudo cp -r /home/rangsiman/protobuf-3.9.2/include/google/ /usr/local/tensorflow/include/
sudo mkdir /usr/local/tensorflow/lib
sudo cp -r bazel-bin/tensorflow/*.so* /usr/local/tensorflow/lib
sudo cp -r /home/rangsiman/protobuf-3.9.2/lib/*.so* /usr/local/tensorflow/lib
```

### 4. Compiling the op library and example code

**Example-1**: Zero out

Create `zero_out.cpp`
```cpp
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
```bash
g++ -Wall -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 \
    -shared zero_out.cpp -o zero_out.so \
    -I/usr/local/tensorflow/include/ -L/usr/local/tensorflow/lib \
    -ltensorflow_cc -ltensorflow_framework
```

**Example-2**: Call TF session

Create `session.cpp`
```cpp
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
```bash
g++ -Wall -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 \
    session.cpp -o session \
    -I/usr/local/tensorflow/include/ -L/usr/local/tensorflow/lib \
    -ltensorflow_cc -ltensorflow_framework
```

To run the executable, you also need to add `/usr/local/tensorflow/lib/` into `LD_LIBRARY_PATH` env var.

---

## Optional: Compile TensorFlow via pip (wheel) builder

Once you have built your TensorFlow, you can then make a wheel file and install the TensorFlow Python library:

Create a wheel package:
```bash
./bazel-bin/tensorflow/tools/pip_package/build_pip_package /tmp/tensorflow_pkg
```

Install TensorFlow library using a created wheel:
```bash
pip install /tmp/tensorflow_pkg/tensorflow-*.whl
```

## References
1. https://www.tensorflow.org/guide/create_op#compile_the_op_using_your_system_compiler_tensorflow_binary_installation
2. https://www.tensorflow.org/install/source#bazel_build_options
