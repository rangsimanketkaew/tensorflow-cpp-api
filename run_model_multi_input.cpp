/*
Load and run a saved TensorFlow's multiple-input model

g++ -Wall -fPIC -D_GLIBCXX_USE_CXX11_ABI=1 run_model.cpp -o run_model.o \
    -I/usr/local/tensorflow-2.11/include/ -L/usr/local/tensorflow-2.11/lib \ 
    -ltensorflow_cc -ltensorflow_framework 

Rangsiman Ketkaew
*/

#include <tensorflow/cc/saved_model/loader.h>
#include <tensorflow/cc/saved_model/tag_constants.h>

using namespace tensorflow;

int main()
{

    // load the whole folder
    std::string export_dir = "./model/";

    // Load
    SavedModelBundle model_bundle;
    SessionOptions session_options = SessionOptions();
    RunOptions run_options = RunOptions();
    Status status = LoadSavedModel(session_options, run_options, export_dir, {kSavedModelTagServe}, &model_bundle);

    if (!status.ok())
    {
        std::cerr << "Failed: " << status;
    }

    auto sig_map = model_bundle.GetSignatures();
    auto model_def = sig_map.at("serving_default");

    for (auto const &p : sig_map)
    {
        std::cout << "key: " << p.first.c_str() << std::endl;
    }
    for (auto const &p : model_def.inputs())
    {
        std::cout << "key: " << p.first.c_str() << " " << p.second.name().c_str() << std::endl;
    }
    for (auto const &p : model_def.outputs())
    {
        std::cout << "key: " << p.first.c_str() << " " << p.second.name().c_str() << std::endl;
    }

    auto input_tensor_1_name = "serving_default_args_0:0";
    auto input_tensor_2_name = "serving_default_args_0_1:0";
    auto output_tensor_1_name = "StatefulPartitionedCall:0";
    auto output_tensor_2_name = "StatefulPartitionedCall:1";

    // Create output placeholder tensors
    // I use the size of input that exactly matchs the size of input from Python model
    // Use saved_model_cli to check
    tensorflow::Tensor input_1(tensorflow::DT_FLOAT, tensorflow::TensorShape({1, 394}));
    tensorflow::Tensor input_2(tensorflow::DT_FLOAT, tensorflow::TensorShape({1, 99}));

    auto input_mat_1 = input_1.matrix<float>();
    // assign random value
    for (unsigned i = 0; i < 394; ++i)
        input_mat_1(0, i) = 1.0;
    auto input_mat_2 = input_2.matrix<float>();
    // assign random value
    for (unsigned i = 0; i < 99; ++i)
        input_mat_2(0, i) = 2.0;

    typedef std::vector<std::pair<std::string, tensorflow::Tensor>> tensor_dict;
    tensor_dict feed_dict = {
        {input_tensor_1_name, input_1},
        {input_tensor_2_name, input_2}};

    // Create output placeholder tensors for results
    std::vector<tensorflow::Tensor> outputs;
    std::vector<std::string> output_names = {output_tensor_1_name, output_tensor_2_name};
    // Running inference
    tensorflow::Status status_run = model_bundle.session->Run(feed_dict,
                                                              output_names,
                                                              {},
                                                              &outputs);
    // Check if session is successfully loaded
    if (!status_run.ok())
    {
        std::cerr << "Failed: " << status_run;
    }
    else
    {
        std::cout << "Passed: " << status_run << std::endl;
    }

    std::cout << "input 1         " << input_1.DebugString() << std::endl;
    std::cout << "input 2         " << input_2.DebugString() << std::endl;
    std::cout << "output          " << outputs[0].DebugString() << std::endl;
    std::cout << "dense/kernel:0  " << outputs[1].DebugString() << std::endl;
    // std::cout << "dense/bias:0    " << outputs[2].DebugString() << std::endl;

    return 0;
}
