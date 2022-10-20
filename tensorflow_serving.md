# TensorFlow Serving

## SavedModel

**Save your (keras/tf) model:**
```python
model.compile(...)
model.fit(...)
model.save("/path/to/your/model/folder/")
```

of you can use [`tf.saved_model.save`](https://www.tensorflow.org/api_docs/python/tf/saved_model/save) method to save object in SavedModel format at low-level:

```python
tf.saved_model.save(model, export_dir)
```

**Get model input:**
```bash
$ saved_model_cli show --tag_set serve --signature_def serving_default --dir model/
```

output:
```bash
The given SavedModel SignatureDef contains the following input(s):
  inputs['dense_input'] tensor_info:
      dtype: DT_FLOAT
      shape: (-1, 4)
      name: serving_default_dense_input:0
The given SavedModel SignatureDef contains the following output(s):
  outputs['dense_1'] tensor_info:
      dtype: DT_FLOAT
      shape: (-1, 1)
      name: StatefulPartitionedCall:0
Method name is: tensorflow/serving/predict
```

The name of input and output tensors are `serving_default_dense_input` and `StatefulPartitionedCall`, respectively

You can also use `--all` argument to get all information, like this:

```bash
MetaGraphDef with tag-set: 'serve' contains the following SignatureDefs:

signature_def['__saved_model_init_op']:
  The given SavedModel SignatureDef contains the following input(s):
  The given SavedModel SignatureDef contains the following output(s):
    outputs['__saved_model_init_op'] tensor_info:
        dtype: DT_INVALID
        shape: unknown_rank
        name: NoOp
  Method name is: 

signature_def['serving_default']:
  The given SavedModel SignatureDef contains the following input(s):
    inputs['args_0'] tensor_info:
        dtype: DT_FLOAT
        shape: (-1, 394)
        name: serving_default_args_0:0
    inputs['args_0_1'] tensor_info:
        dtype: DT_FLOAT
        shape: (-1, 99)
        name: serving_default_args_0_1:0
  The given SavedModel SignatureDef contains the following output(s):
    outputs['out_1'] tensor_info:
        dtype: DT_FLOAT
        shape: (-1, 394)
        name: StatefulPartitionedCall:0
    outputs['out_2'] tensor_info:
        dtype: DT_FLOAT
        shape: (-1, 99)
        name: StatefulPartitionedCall:1
  Method name is: tensorflow/serving/predict

...
```

## Run model

**Load model**
```cpp
const std::string export_dir = "/path/to/your/model/folder/";

tensorflow::SavedModelBundle model_bundle;
tensorflow::SessionOptions session_options = tensorflow::SessionOptions();
tensorflow::RunOptions run_options = tensorflow::RunOptions();
tensorflow::Status status = tensorflow::LoadSavedModel(session_options, run_options, export_dir, {tensorflow::kSavedModelTagServe},
                            &model_bundle);

if (status.ok()) {
    std::cout << "Session successfully loaded: " << status;
}
else {
    std::cerr << "Failed: " << status;
}

auto sig_map = model_bundle.GetSignatures();
auto model_def = sig_map.at("serving_default");

printf("\nModel Signature\n");
for (auto const& p : sig_map) {
    printf("key: %s\n", p.first.c_str());
}

printf("Model Input Nodes\n");
for (auto const& p : model_def.inputs()) {
    printf("key: %s value: %s\n", p.first.c_str(), p.second.name().c_str());
}

printf("Model Output Nodes\n");
for (auto const& p : model_def.outputs()) {
    printf("key: %s value: %s\n", p.first.c_str(), p.second.name().c_str());
}
```
