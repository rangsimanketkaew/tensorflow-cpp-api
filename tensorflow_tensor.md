# TensorFlow Tensor

https://www.tensorflow.org/api_docs/cc/class/tensorflow/tensor

## Create a tensor

**Create a vector tensor**
```cpp
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/platform/env.h>

using namespace std;
using namespace tensorflow;

int main(int argc, char* argv[]) {

    Tensor a(DT_FLOAT, TensorShape({2}));
    auto a_vec = a.vec<float>();
    a_vec(0) = 1.0f;
    a_vec(1) = 2.0f;

    cout << "Vector tensor" << endl;

    return 0;
}
```

**Create a matrix tensor**
```cpp
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/platform/env.h>

using namespace std;
using namespace tensorflow;

int main(int argc, char* argv[]) {

    Tensor b(DT_FLOAT, TensorShape({2,2}));
    auto b_mat = b.matrix<float>();
    b_mat(0,0) = 1.0f;
    b_mat(0,1) = 2.0f;
    b_mat(1,0) = 3.0f;
    b_mat(1,1) = 4.0f;

    return 0;
}
```

**Print tensor components**

Method 1:
```cpp
...
    cout << "First element: " << a_vec(0) << endl; // output: 1
    // or
    cout << "First element: " << b_mat(0) << endl; // output: 1
...
```

Method 2: pointer
```cpp
...
    // get pointer to memory for the tensor
    float * p = b.vec<float>().data();
    cout << typeid(p).name() << endl;
    cout << p[0] << endl;
    cout << p[1] << endl;
    cout << p[2] << endl;
    cout << p[3] << endl;
...
```

**Print shape info**
```cpp
...
    cout << b.shape().dims() << endl;
    // output: 2
...
```

## Convert vector to tensor

**Use `std::copy`
```cpp
...
    vector<float> myvec = {5,6,7};
    Tensor a(DT_FLOAT, TensorShape({3}));
    copy(myvec.begin(), myvec.end(), a.flat<float>().data() );
...
```
