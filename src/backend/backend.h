// Adapted from https://github.com/acids-ircam/nn_tilde/blob/master/src/backend/backend.h
//


#pragma once
#include <mutex>
#include <string>
#include <torch/script.h>
#include <torch/torch.h>
#include <vector>


class LatentDataset : public torch::data::datasets::Dataset<LatentDataset> {
private:
    torch::Tensor data, targets;
    
public:
    // Constructor
    LatentDataset(torch::Tensor data, torch::Tensor targets)
        : data(data), targets(targets) {}
    
    // Get size of dataset
    torch::optional<size_t> size() const override {
        return data.size(0);
    }

    // Get a single data sample
    torch::data::Example<> get(size_t index) override {
        return {data[index], targets[index]};
    }
};

struct FourierCPPNImpl : torch::nn::Module {
    
    FourierCPPNImpl(int in_dim, int out_dim, int c_max, float gauss_scale, int mapping_size) :
        fc_in_linear(torch::nn::Linear(in_dim, c_max)),
        fc_in_gau(torch::nn::Linear(mapping_size*2, c_max)),
        fc2(torch::nn::Linear(c_max, c_max)),
        fc3(torch::nn::Linear(c_max, c_max)),
        fc4(torch::nn::Linear(c_max, c_max)),
        fc_out(torch::nn::Linear(c_max, out_dim)),
        prelu1(torch::nn::PReLU()),
        prelu2(torch::nn::PReLU()),
        prelu3(torch::nn::PReLU()),
        prelu4(torch::nn::PReLU()),
        two_pi(torch::tensor(2 * M_PI)),
        B_T(torch::normal(0, gauss_scale, {mapping_size, in_dim}).transpose(0, 1))
    {
        // Construct and register submodules.
        gauss = gauss_scale;
        c_in_dim = in_dim;
        c_out_dim = out_dim;
        
        if (gauss == 0.0f) {
            register_module("fc_in_linear", fc_in_linear);
        } else {
            register_module("fc_in_gau", fc_in_gau);
            register_buffer("two_pi", two_pi);
            register_buffer("B_T", B_T);
        }
        register_module("prelu1", prelu1);
        register_module("fc2", fc2);
        register_module("prelu2", prelu2);
        register_module("fc3", fc3);
        register_module("prelu3", prelu3);
        register_module("fc4", fc4);
        register_module("prelu4", prelu4);
        register_module("fc_out", fc_out);
    }

    torch::Tensor forward(torch::Tensor x) {
        if (gauss > 0.0f) {
            x = torch::matmul(two_pi * x, B_T);
            x = torch::cat({x.sin(), x.cos()}, 1);
            x = prelu1(fc_in_gau(x));
        } else {
            x = prelu1(fc_in_linear(x));
        }
        x = prelu2(fc2(x));
        x = prelu3(fc3(x));
        x = prelu4(fc4(x));
        x = fc_out(x);
        
        return x;
    }
    
    float gauss;
    int c_in_dim, c_out_dim;
    c10::DeviceType m_device;
    torch::nn::Linear fc_in_linear{nullptr}, fc_in_gau{nullptr}, fc2{nullptr}, fc3{nullptr}, fc4{nullptr}, fc_out{nullptr};
    torch::nn::PReLU prelu1{nullptr}, prelu2{nullptr}, prelu3{nullptr}, prelu4{nullptr};
    torch::Tensor two_pi{nullptr}, B_T{nullptr};
};
TORCH_MODULE(FourierCPPN);



class Backend {
public:
  torch::jit::script::Module m_model;
  int m_loaded;
  std::string m_path;
  std::mutex m_model_mutex;
  std::vector<std::string> m_available_methods;
  c10::DeviceType m_device;
  bool m_use_gpu;
    
    
  Backend();
  void perform(std::vector<float *> in_buffer, std::vector<float *> out_buffer,
               int n_vec, std::string method, int n_batches);
//    void freeze(std::vector<float *> in_buffer, std::vector<float *> out_buffer,
//                 int n_vec, std::string method, int n_batches);
  bool has_method(std::string method_name);
  bool has_settable_attribute(std::string attribute);
  std::vector<std::string> get_available_methods();
  std::vector<std::string> get_available_attributes();
  std::vector<std::string> get_settable_attributes();
  std::vector<c10::IValue> get_attribute(std::string attribute_name);
  std::string get_attribute_as_string(std::string attribute_name);
  void set_attribute(std::string attribute_name,
                     std::vector<std::string> attribute_args);

  std::vector<int> get_method_params(std::string method);
  int get_higher_ratio();
  int load(std::string path);
  int reload();
  bool is_loaded();
  torch::jit::script::Module get_model() { return m_model; }
  void use_gpu(bool value);
};

class FCPPN{
public:
    FourierCPPN m_model{nullptr};
    int m_loaded;
    std::mutex m_model_mutex;
    c10::DeviceType m_device;
    bool m_use_gpu;
    bool optimizer_init = false;
    std::unique_ptr<torch::optim::Adam> optimizer;
    at::Tensor sample_tensor;
    
    FCPPN();
    
    std::unique_ptr<torch::data::StatelessDataLoader<torch::data::datasets::MapDataset<LatentDataset, torch::data::transforms::Stack<torch::data::Example<>>>, torch::data::samplers::RandomSampler>> data_loader;
    
    bool create(int in_dim, int out_dim, int c_max, float gauss_scale, int mapping_size){
        try{
            m_model = FourierCPPN(in_dim, out_dim, c_max, gauss_scale, mapping_size);
            m_loaded = 1;
            m_model->eval();
            return true;
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            return false;
        }
    }
    bool is_loaded() { return m_loaded; }
    void use_gpu(bool value);
    bool save(std::string save_path, std::string save_name, int m_in_dim, int m_out_dim, int m_cmax, float m_gauss_scale, int m_mapping_size);
    void init_optimizer(float lr);
    float train_for(int epochs);
};




