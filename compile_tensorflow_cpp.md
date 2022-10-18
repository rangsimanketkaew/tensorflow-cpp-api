# Compile TensorFlow C++ from source code <!-- omit in toc -->

Building TensorFlow C++ API is very tricky and can be a pain as there is not much information you can find about it even on TensorFlow's official documentation. Following you will find a step-by-step instruction showing how to build TensorFlow C++ v2 on Linux. It works well for my Ubuntu 20.04 running on AMD Ryzen processors.

On this page, I will walk you through the steps to install **TensorFlow C++ API version 2.7**.

- [Dependencies](#dependencies)
- [Install package dependencies](#install-package-dependencies)
  - [1. Environment setup](#1-environment-setup)
  - [2. Install Bazel](#2-install-bazel)
  - [3. Install Protobuf](#3-install-protobuf)
- [Compile TensorFlow C++ and install libraries](#compile-tensorflow-c-and-install-libraries)
  - [1. Compile TensorFlow C++ shared library (with optimization)](#1-compile-tensorflow-c-shared-library-with-optimization)
  - [2. Copy required files into a single path for C++ linkage](#2-copy-required-files-into-a-single-path-for-c-linkage)
  - [3. Compiling the op library and example code](#3-compiling-the-op-library-and-example-code)
- [Optional: Compile TensorFlow via pip (wheel) builder](#optional-compile-tensorflow-via-pip-wheel-builder)
- [References](#references)

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
wget https://github.com/bazelbuild/bazel/releases/download/3.7.2/bazel-3.7.2-installer-linux-x86_64.sh
chmod +x bazel-3.7.2-installer-linux-x86_64.sh
./bazel-3.7.2-installer-linux-x86_64.sh --user
```

### 3. Install Protobuf

1. Check the version of protobuf that is supported by TensorFlow in the file `tensorflow/workspace2.bzl`,
   e.g., TF 2.7.0 supports Protobuf 3.9.2
2. Download protobuf source code from its GitHub release https://github.com/protocolbuffers/protobuf/tags
   - E.g. v3.9.2 https://github.com/protocolbuffers/protobuf/releases/tag/v3.9.2
3. Compile and link
   ```bash
   ./configure --prefix=/home/rangsiman/protobuf-3.9.2/ CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0"
   make
   make check
   make install
   ```

---

## Compile TensorFlow C++ and install libraries

### 1. Compile TensorFlow C++ shared library (with optimization)

Download a tarball of TensorFlow (I strongly prefer v.2.7 to other versions), 
e.g., from https://github.com/tensorflow/tensorflow/releases/tag/v2.7.4
```bash
wget https://github.com/tensorflow/tensorflow/archive/refs/tags/v2.7.4.tar.gz
tar -xzvf v2.7.4.tar.gz
cd tensorflow-2.7.4
```
or clone github repo to your system:
```bash
git clone https://github.com/tensorflow/tensorflow
cd tensorflow
git checkout r2.7
```

Configure the build:
```bash
./configure

You have bazel 3.7.2 installed.
Please specify the location of python. [Default is /home/rketka/miniconda3/envs/deepcv/bin/python3]:


Found possible Python library paths:
  /home/rketka/miniconda3/envs/deepcv/lib/python3.9/site-packages
Please input the desired Python library path to use.  Default is [/home/rketka/miniconda3/envs/deepcv/lib/python3.9/site-packages]

Do you wish to build TensorFlow with ROCm support? [y/N]: N
No ROCm support will be enabled for TensorFlow.

Do you wish to build TensorFlow with CUDA support? [y/N]: N
No CUDA support will be enabled for TensorFlow.

Do you wish to download a fresh release of clang? (Experimental) [y/N]: N
Clang will not be downloaded.

Please specify optimization flags to use during compilation when bazel option "--config=opt" is specified [Default is -Wno-sign-compare]: n


Would you like to interactively configure ./WORKSPACE for Android builds? [y/N]: N
Not configuring the WORKSPACE for Android builds.

Preconfigured Bazel build configs. You can use any of the below by adding "--config=<>" to your build command. See .bazelrc for more details.
        --config=mkl            # Build with MKL support.
        --config=mkl_aarch64    # Build with oneDNN and Compute Library for the Arm Architecture (ACL).
        --config=monolithic     # Config for mostly static monolithic build.
        --config=numa           # Build with NUMA support.
        --config=dynamic_kernels        # (Experimental) Build kernels into separate shared objects.
        --config=v1             # Build with TensorFlow 1 API instead of TF 2 API.
Preconfigured Bazel build configs to DISABLE default on features:
        --config=nogcp          # Disable GCP support.
        --config=nonccl         # Disable NVIDIA NCCL support.
Configuration finished
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
            //tensorflow:install_headers
```

You can use the following command to check all available rules in a specific top directory:
```bash
bazel query ...
```

Note:

1. Building TensorFlow can consume a lot of memory. So I prefer a small number of CPUs (`--jobs`), e.g. 4 CPUs use `--jobs=4`.
2. Limit RAM requested by bazel with `--local_ram_resources`. The value is either integer, .e.g., 2048 use `--local_ram_resources=2048` or % of total memory, e.g., 50% use `"HOST_RAM*.50"`.
3. The whole process can take up to 1 hour.
4. If you don't want Bazel creates cache files in your local space, add [`--output_user_root`](https://docs.bazel.build/versions/main/user-manual.html#flag--output_user_root) to change the directory where output and base files will be created, e.g., 
   ```bash
   bazel --output_user_root=/scratch/bazel/ build ...
   ```
5. Add `-D_GLIBCXX_USE_CXX11_ABI=0` if you use GCC 5 or higher version.
6. Flags for optimization: `--copt="-O3"`.
7. Flasg for both AMD and Intel chips: `--copt=-mfma --copt=-msse4.1 --copt=-msse4.2 --copt=-mfpmath=both`.
8. Flags for Intel: `--copt=-mavx --copt=-mavx2`.
9. Rebuild with `--config=monolithic` if you want to compile all TensorFlow C++ code into a single shared object.

**Optional 1:** Test
```bash
bazel test --jobs=4 --cxxopt="-D_GLIBCXX_USE_CXX11_ABI=0" -c opt \
           //tensorflow/tools/lib_package:libtensorflow_test
```

**Optional 2:**
Additionally, you can also build TensorFlow wheel file (.whl) which can then be used to install TensorFlow by pip:

```bash
# Build process
bazel build --config=opt -c opt //tensorflow/tools/pip_package:build_pip_package

# Create a wheel file
./bazel-bin/tensorflow/tools/pip_package/build_pip_package /tmp/tensorflow_pkg

# Install TensorFlow using the wheel file
pip install /tmp/tensorflow_pkg/tensorflow-2.3.0-cp38-cp38-linux_x86_64.whl

# Try importing TensorFlow and test it
python -c "import tensorflow as tf;print(tf.reduce_sum(tf.random.normal([1000, 1000])))"
```

### 2. Copy required files into a single path for C++ linkage

```bash
sudo mkdir /usr/local/tensorflow
sudo cp -r bazel-bin/tensorflow/include/ /usr/local/tensorflow/
sudo cp -r /home/rangsiman/protobuf-3.9.2/include/google/ /usr/local/tensorflow/include/
sudo mkdir /usr/local/tensorflow/lib
sudo cp -r bazel-bin/tensorflow/*.so* /usr/local/tensorflow/lib
sudo cp -r /home/rangsiman/protobuf-3.9.2/lib/*.so* /usr/local/tensorflow/lib
```

### 3. Compiling the op library and example code

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
