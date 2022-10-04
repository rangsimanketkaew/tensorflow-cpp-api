# Compile TensorFlow C++ from source code

Building TensorFlow C++ API is very tricky and can be a pain as there is not much information you can find about it even on TensorFlow's official documentation. Following you will find a step-by-step instruction showing how to build TensorFlow C++ v2 on Linux. It works well for my Ubuntu 20.04 running on AMD Ryzen processors.

On this page, I will walk you through the steps to install **TensorFlow C++ API version 2.7**.

## Dependencies

- Conda
- Python + NumPy
- GCC 5 or higher
- Bazel*
- Protobuf*

*a supported version depends on the version of TensorFlow. For TensorFlow v2.7, Python 3.9.9, GCC 10.3.0, Bazel 3.7.2, and Protobuf 3.9.2 work for me.

A list of supported Python, compiler and Bazel can be found [here](https://www.tensorflow.org/install/source#tested_build_configurations).

---

## Install package dependencies

### 1. Environment setup
```bash
conda create -n tfcc
conda activate tfcc # or `source activate tfcc`
conda update --all -y
conda install python==3.9.9
conda install numpy
```

### 2. Install Bazel

Check a supported version at `tensorflow/.bazelversion`.

Add bazel repository:
```bash
sudo apt install curl gnupg
curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
```

Install:
```bash
sudo apt update
# Install the latest version
sudo apt install bazel
# or a specific version, e.g.,
sudo apt install bazel-3.7.2
```

You can also run the installer. Download from Bazel GitHub release page and run the installer in your home:
```bash
https://github.com/bazelbuild/bazel/releases/download/3.7.2/bazel-3.7.2-installer-linux-x86_64.sh
chmod +x bazel-3.7.2-installer-linux-x86_64.sh
./bazel-3.7.2-installer-linux-x86_64.sh --user
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

Configure the build:
```bash
./configure
# I leave all answers empty, just Enter
```

Let's compile using bazel `build` rule:
```bash
export CC=gcc
export CXX=g++
bazel build --jobs=4 --local_ram_resources="HOST_RAM*.50" \
            --cxxopt="-D_GLIBCXX_USE_CXX11_ABI=0" -c opt \
            --config=noaws --config=nogcp --config=nohdfs --config=nonccl \
            //tensorflow:libtensorflow.so \
            //tensorflow:libtensorflow_cc.so \
            //tensorflow:libtensorflow_framework.so \
            //tensorflow:install_headers \
            //tensorflow/tools/pip_package:build_pip_package
```

You can use the following command to check all available rules in a specific top directory:
```bash
bazel query ...
```

Note:

1. Building TensorFlow uses a lot of memory, I prefer a small number of CPUs (`--jobs`)
2. Limit RAM requested by bazel with `--local_ram_resources`. The value is either integer, .e.g., `4096` or % of total memory, e.g., 50% use `"HOST_RAM*.50"`
3. The whole process can take several hours
4. Add `-D_GLIBCXX_USE_CXX11_ABI=0` if you use GCC 5 or higher
5. Flags for optimization: `--copt="-O3"`
6. Flasg for both AMD and Intel chips: `--copt=-mfma --copt=-msse4.1 --copt=-msse4.2 --copt=-mfpmath=both`
7. Flags for Intel: `--copt=-mavx --copt=-mavx2`
8. Rebuild with `--config=monolithic` if you want compile all TensorFlow C++ code into a single shared object

**optional**
```bash
bazel test --jobs=4 --cxxopt="-D_GLIBCXX_USE_CXX11_ABI=0" -c opt \
           //tensorflow/tools/lib_package:libtensorflow_test
```

### 2. Install protobuf

1. Check the version of protobuf that is supported by TensorFlow
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

**Example-2**: Call TensorFlow session

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
