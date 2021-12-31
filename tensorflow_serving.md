# TensorFlow Serving

## SavedModel

**Save your (keras/tf) model:**
```
model.compile(...)
model.fit(...)
model.save("/path/to/your/model/folder/")
```

**Get model input:**
```
$ saved_model_cli show --tag_set serve --signature_def serving_default --dir model/
```

output:
```
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

The name of input and output tensors are `serving_default_dense_input:0` and `StatefulPartitionedCall:0`, respectively

## Run model

**Load model**
```
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
