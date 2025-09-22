#include "../../../backend/backend.h"
#include "c74_min.h"

#include <stdio.h>
#include <iostream>

#include <chrono>
#include <semaphore>
#include <string>
#include <thread>
#include <vector>

#include "../shared/min_path.h"
#include "../shared/min_dictionary.h"

#include "../shared/utils.h"

#include <math.h>

#ifndef VERSION
#define VERSION "UNDEFINED"
#endif

#define None at::indexing::None
#define Slice torch::indexing::Slice

using namespace c74::min;
using namespace c74::min::ui;

class nn_terrain : public object<nn_terrain>{
    
public:
    MIN_DESCRIPTION{"Encoding audio buffers to latent vectors with neural audio autoencoder"};
    MIN_AUTHOR{"Jasper Shuoyang Zheng"};
    MIN_RELATED        { "nn.terrain~, nn.terrain.record, nn~, nn.terrain.gui" };

    inlet<>    append_inlet      { this, "(append) Append buffers to the buffer pool", "message"};
    
    outlet<>    log_output          { this, "(dictionary) List of buffers in the pool", "dictionary"};
    outlet<>    latent_len_output   { this, "(list) Number of latents along each trajectory", "list"};
//    outlet<>    latent_len_all_output   { this, "(int) Total number of latents along all trajectories", "int"};

    outlet<>    dataset_output      { this, "(dictionary) Latent vectors encoded from buffers, to be sent to nn.terrain", "dictionary"};

    nn_terrain(const atoms &args = {});
    ~nn_terrain();
    
    std::vector<std::unique_ptr<buffer_reference>> buffers;
    
    std::unique_ptr<Backend> m_model;
    
    bool m_is_backend_init = false;
    std::string d_method{"decode"};
    std::string e_method{"encode"};
    
    c74::min::path m_path;
    int m_in_dim{1}, m_in_ratio{1}, m_out_dim{8}, m_out_ratio{2048}, m_higher_ratio{2048};

    int load_encoder(string path_str);
    
    argument<symbol> path_arg{this, "model_path", "Path to the pretrained model (encoder).", true};
//    argument<int> latent_dim_arg {this, "latent_dim", "(Optional) Dimensionality of the autoencoder's latent space."};
    
    
  // ENABLE / DISABLE ATTRIBUTE
    attribute<bool> gpu{this, "gpu", true,
        description{"Enable / disable gpu usage when available"},
        setter{
            [this](const c74::min::atoms &args, const int inlet) -> c74::min::atoms {
                if (m_is_backend_init)
                    m_model->use_gpu(bool(args[0]));
                // TODO: t_model->use_gpu(bool(args[0]));
                return args;
            }
        }
    };
    // TODO: add this:
//    attribute<atoms> buffer_list_attr {this, "buffer_list", "",
//        description{""},
//        setter{
//            [this](const c74::min::atoms &args, const int inlet) -> c74::min::atoms {
//                return args;
//            }
//        }
//    };

    message<> bang { this, "bang", "Post the encoded latents.",
        MIN_FUNCTION {
            dataset_output.send("dictionary", training_data.name());
            return {};
        }
    };


    int latent_dim = 8;
    
    attribute<int,threadsafe::undefined,limit::clamp> batch_size{ this, "encoder_batch_size", 16, description{"batch size used when encoding audio buffers"}, range{{8, 256}} };
    
  // BOOT STAMP
  message<> maxclass_setup{
      this, "maxclass_setup",
      [this](const c74::min::atoms &args, const int inlet) -> c74::min::atoms {
        cout << "torch version: " << TORCH_VERSION << endl;
        return {};
      }};
    
    min_dict training_data = {symbol(true)};
    min_dict buffer_dict = {symbol(true)};
    min_dict latent_dict = {symbol(true)};
    atoms latent_lengths;
    numbers dataset_counts = {0,0,0};
    
    int latent_lengths_all = 0;
    int buffer_count = 0;
    message<> m_append_bufferlist {this, "append", "Add a buffer to the buffer pool",
        MIN_FUNCTION {
            if (args.size() != 1){
                cerr << "args size " << args.size() << " error, args size should be 1" << endl;
                return {};
            }
            atom buffer_name = std::string(args[0]);
            buffers.push_back(std::make_unique<buffer_reference>(this));
            buffers[buffer_count]->set(buffer_name);
            
            buffer_lock<false> b {*buffers[buffer_count]};
            
            if (!b.valid()){
                cerr << "buffer is not valid" << endl;
                buffers.pop_back();
                return {};
            }
            
            buffer_dict[buffer_count] = std::string(args[0]);
            buffer_dict.touch();
            
            int latents_count = static_cast<int>(std::ceil(b.frame_count() / static_cast<float>(m_out_ratio)));
            latent_lengths.push_back(latents_count);
            latent_lengths_all += latents_count;
            
            buffer_count++;
            log_output.send("dictionary", buffer_dict.name());
            latent_len_output.send(latent_lengths);
            return {};
        }
    };
    
    message<> m_set_bufferlist {this, "anything", "Use a list of buffer names to set buffers in the buffer pool (it will overwrite previous buffers)",
        MIN_FUNCTION {
            buffer_dict.clear();
            buffer_dict.touch();
            buffer_count = 0;
            
            buffers.erase(std::remove_if(buffers.begin(), buffers.end(), [](auto const& pi){ return *pi % 2 == 0; }), buffers.end());
            buffers.clear();
            
            latent_lengths_all = 0;
            latent_lengths.clear();
            
            for (int i = 0; i < args.size(); i++){
                atom buffer_name = std::string(args[i]);
                
                buffers.push_back(std::make_unique<buffer_reference>(this));
                buffers[buffer_count]->set(buffer_name);
                
                buffer_lock<false> b {*buffers[buffer_count]};
                
                if (!b.valid()){
                    cerr << "buffer " << buffer_name << " is not valid" << endl;
                    buffers.pop_back();
                    continue;
                }
                
                buffer_dict[buffer_count] = std::string(args[i]);
                
                int latents_count = static_cast<int>(std::ceil(b.frame_count() / static_cast<float>(m_out_ratio)));
                latent_lengths.push_back(latents_count);
                latent_lengths_all += latents_count;
                
                buffer_count++;
            }
            buffer_dict.touch();
            log_output.send("dictionary", buffer_dict.name());
            latent_len_output.send(latent_lengths);
            return {};
        }
    };
    
    message<> dictionary { this, "dictionary",
        "A dictionary of buffer names, key name should be 'buffers'",
        MIN_FUNCTION {
            c74::max::t_symbol    **keys = NULL;
            long        numkeys = 0;
            
            min_dict d = {args[0]};
            c74::max::dictionary_getkeys(d.m_instance, &numkeys, &keys);
            
            for (int i = 0; i < numkeys; i++){
                string key_str = std::string(keys[i]->s_name);
                atom d_data = d[key_str];
                min_dict d_data_dict = {d_data};
                if (key_str == "coordinates"){
                    cout << "please send coordinates directly to nn.terrain" << endl;
                } else if (key_str == "buffers"){
                    buffer_dict.clear();
                    buffer_dict.copyunique(d_data_dict);
                    latent_lengths.clear();
                    latent_lengths_all = 0;
                    buffers.erase(std::remove_if(buffers.begin(), buffers.end(), [](auto const& pi){ return *pi % 2 == 0; }), buffers.end());
                    buffers.clear();
                    
                    int numkeys_b = static_cast<int>(c74::max::dictionary_getentrycount(buffer_dict.m_instance));
                    for (int i = 1; i <= numkeys_b; i++){
                        buffers.push_back(std::make_unique<buffer_reference>(this));
                        atom buffer_name = buffer_dict[i];
                        buffers[i-1]->set(buffer_name);
                        
                        buffer_lock<false> b {*buffers[i-1]};
                        int latents_count = static_cast<int>(std::ceil(b.frame_count()/static_cast<float>(m_out_ratio)));
                        latent_lengths.push_back(latents_count);
                        latent_lengths_all += latents_count;
                    }
                } else {
                    cerr << "unknown key: " << key_str << endl;
                }
            }
            if (keys){
                c74::max::dictionary_freekeys(d.m_instance, numkeys, keys);
            }
            latent_len_output.send(latent_lengths);
            return {};
        }
    };

    message<> create_dataset_from_buffers {this, "encode", "Encode buffers in the pool to a sequence of latent vectors",
        MIN_FUNCTION {
            
            if (!m_model->is_loaded()){
                cerr << "encoder not loaded" << endl;
                return {};
            }
            if (buffers.empty()){
                cerr << "no buffers loaded" << endl;
                return {};
            }
            std::vector<at::Tensor> tensor_in;
            std::vector<int> latent_lens;
            int valid_buffers = 0;
            try{
                // TODO: need a better way to convert buffers to tensors
                for (int i(0); i < buffers.size(); i++) {
                    buffer_lock<false> b {*buffers[i]};
                    if (!b.valid()){
                        cerr << "buffer " << i << " is not valid" << endl;
                        continue;
                    }
                    if (b.frame_count() == 0){
                        cerr << "buffer " << i << " is empty" << endl;
                        continue;
                    }
                    if (b.channel_count() < m_in_dim){
                        cerr << "buffer " << i << " has less channels than model input dimension, skipping" << endl;
                        continue;
                    }
                    vector<float> buffer_data;
                    for (int c(0); c < m_in_dim; c++){
                        for (int j(0); j < b.frame_count(); j++){
                            float buffer_data_p = b.lookup(j, c);
                            buffer_data.push_back(buffer_data_p);
                        }
                    }
                    
                    at::Tensor buffer_tensor = torch::from_blob(buffer_data.data(), {1, m_in_dim, static_cast<long long>(b.frame_count())}, torch::kFloat);
                    buffer_tensor = buffer_tensor.clone();
                    
                    // if buffer size is not divisible by m_out_ratio (2048), pad with zeros at the end
                    int zeros = m_out_ratio - b.frame_count() % m_out_ratio;
                    latent_lens.push_back(
                        std::ceil(static_cast<float>(b.frame_count()) / static_cast<float>(m_out_ratio))
                    );
                    at::Tensor zero_fill = torch::zeros({1,m_in_dim,zeros});
                    buffer_tensor = torch::cat({buffer_tensor, zero_fill}, 2);
                    tensor_in.push_back(buffer_tensor);
                    valid_buffers+= 1;
                }
            } catch (const std::exception &e) {
                cerr << e.what() << endl;
                return {};
            }
            if (valid_buffers == 0){
                cerr << "no valid buffers found" << endl;
                return {};
            }
            auto cat_tensor_in = torch::cat(tensor_in, 2); // -> [1, 2, num_samples]
            
            // batching, in case the buffers are too long:
            at::Tensor cat_tensor_in_trim, cat_tensor_in_left;
            try{
                int chunk_size = m_out_ratio*batch_size;
                float leftover = cat_tensor_in.size(2) % (chunk_size);
                float trim = cat_tensor_in.size(2) - leftover;
                cat_tensor_in_trim = cat_tensor_in.index({Slice(None), Slice(None), Slice(None, trim) });
                cat_tensor_in_trim = cat_tensor_in_trim.squeeze(0).unfold(1, chunk_size, chunk_size).permute({1,0,2}); // -> [num_chunks, m_in_dim, chunk_size]
                
//                cat_tensor_in_trim = cat_tensor_in_trim.reshape({-1, m_in_dim, (m_out_ratio*batch_size)});
                cat_tensor_in_left = cat_tensor_in.index({Slice(None), Slice(None), Slice(trim, None)});
            } catch (const std::exception &e) {
                cerr << e.what() << endl;
                return {};
            }
            
//            cout << cat_tensor_in_trim.sizes() << endl;
//            cout << cat_tensor_in_left.sizes() << endl;
//            return {};
            std::vector<at::Tensor> tensor_out_trim;
            at::Tensor cat_tensor_out;
//            return {};
            std::unique_lock<std::mutex> model_lock(m_model->m_model_mutex);
//            torch::NoGradGuard no_grad;
            try {
                torch::NoGradGuard no_grad;
                for (int i(0); i < cat_tensor_in_trim.size(0); i++){
                    at::Tensor input_tensor = cat_tensor_in_trim.index({i}).unsqueeze(0).to(m_model->m_device);
                    std::vector<torch::jit::IValue> inputs = {input_tensor};
                    
                    at::Tensor tensor_out = m_model->m_model.get_method(e_method)(inputs).toTensor();
                    
                    tensor_out = tensor_out.index({0}).permute({1,0}).to(torch::kCPU);
                    tensor_out_trim.push_back(tensor_out);
                }
                if (cat_tensor_in_left.size(2) != 0){
                    cat_tensor_in_left = cat_tensor_in_left.to(m_model->m_device);
                    std::vector<torch::jit::IValue> inputs_left = {cat_tensor_in_left};
                    at::Tensor tensor_out_left = m_model->m_model.get_method(e_method)(inputs_left).toTensor();
                    tensor_out_left = tensor_out_left.index({0}).permute({1,0}).to(torch::kCPU);
                    tensor_out_trim.push_back(tensor_out_left);
                }
                cat_tensor_out = torch::cat(tensor_out_trim, 0);
                
                model_lock.unlock();
            } catch (const std::exception &e) {
                std::string str = e.what();
                int batches = std::ceil(str.size()/1000.0);
                for (int i(0); i < batches; i++){
                    if (str.size() > 1000) {
                        std::string display = str.substr(0, 1000);
                        str.erase(0, 1000);
                        cerr << display << endl;
                    } else {
                        cerr << str << endl;
                    }
                }
                cerr << "It's likely that encoder_batch_size is too large, please try changing to a smaller buffer size" << endl;
                return {};
            }
            
            
            latent_dict.clear();
            
            torch::Tensor cat_tensor_out_clone = cat_tensor_out.permute({1,0}).contiguous();
            const float* latents_ptr = cat_tensor_out_clone.data_ptr<float>();
            
            int c = 0;
            for (int i(0); i < latent_lens.size(); i++){
                min_dict traj_dict = {};
                
                for (int z(0); z < cat_tensor_out.size(1); z++){ // latent_dim
                    atoms result(latents_ptr + c + z*cat_tensor_out.size(0), latents_ptr + c + z*cat_tensor_out.size(0)+ latent_lens[i]);
                    symbol skey{z};
                    c74::max::dictionary_appendatoms(traj_dict.m_instance, skey, result.size(), &result[0]);
                }
                c += latent_lens[i];
                latent_dict[std::to_string(i+1)] = traj_dict;
            }
            
            training_data.touch();
            dataset_output.send("dictionary", training_data.name());
            return {};
        }
    };

    message<> clear_buffers {this, "clear", "Clear recorded latents",
        MIN_FUNCTION {
            buffer_dict.clear();
            buffer_dict.touch();
            buffer_count = 0;
            
            buffers.erase(std::remove_if(buffers.begin(), buffers.end(), [](auto const& pi){ return *pi % 2 == 0; }), buffers.end());
            buffers.clear();
            
            latent_lengths_all = 0;
            latent_lengths.clear();
            
            log_output.send("dictionary", buffer_dict.name());
            return {};
        }
    };
    
    message<> reload_m {this, "reload", "Reload a model",
        MIN_FUNCTION {
            
            if (args.size() != 1) {
                cerr << "wrong number of argments" << endl;
                return {};
            }
            if (args.size() == 1) {
                if (args[0].a_type == 3){
                    
                    try {
                        m_model.reset();
                        m_model = std::make_unique<Backend>();
                    } catch (...) {
                        }
                    auto model_path = std::string(args[0]);
                    if (model_path.substr(model_path.length() - 3) != ".ts")
                      model_path = model_path + ".ts";
                    m_path = path(model_path);
                    try {
                        if (m_model->load(std::string(m_path))) {
                            cerr << "error during loading" << endl;
                            error();
                            return {};
                        }
                    }
                        catch (...) {
                        }
                    
                    try {
                        m_model->use_gpu(gpu);
                        m_higher_ratio = m_model->get_higher_ratio();
                    } catch (...) {
                        
                    }
                    
                } else {
                    cerr << "argument should be a path" << endl;
                    return {};
                }
            }
            cout << "nn setup finished" << endl;
            return {};
        }
    };
};


int nn_terrain::load_encoder(string path_str){
    
    if (path_str.length()>3){
        if (path_str.substr(path_str.length() - 3) != ".ts"){
            path_str = path_str + ".ts";
        }
    }
    m_path = path(path_str);
    
    if (m_model->load(std::string(m_path))) {
        cerr << "error during loading" << endl;
        error();
        return 0;
    }
    m_model->use_gpu(gpu);
    

    m_higher_ratio = m_model->get_higher_ratio();

    // GET MODEL'S METHOD PARAMETERS
    auto params = m_model->get_method_params(e_method);


    if (!params.size()) {
      error("method " + e_method + " not found !");
    }

    m_in_dim = params[0];
    m_in_ratio = params[1];
    m_out_dim = params[2];
    m_out_ratio = params[3];
    cout << "m_in_dim: " << m_in_dim << "\n m_in_ratio: " << m_in_ratio << "\n m_out_dim: " << m_out_dim << "\n m_out_ratio: " << m_out_ratio << endl;
    
    return m_out_dim;
}
    

nn_terrain::nn_terrain(const atoms &args){
          
  m_model = std::make_unique<Backend>();
  m_is_backend_init = true;

    training_data["latents"] = latent_dict;
  // CHECK ARGUMENTS
  if (!args.size()) {
    return;
  } else if (args.size() == 1) { // ONE ARGUMENT (symbols) ARE GIVEN
      if (args[0].a_type == 3){
          latent_dim = load_encoder(std::string(args[0]));
      } else {
            cerr << "error: if one argument is given, it should be symbol" << endl;
        }
  } else { // THREE ARGUMENTS ARE GIVEN
      cerr << "error: maximum two arguments" << endl;
  }
          
//    external_path = min_devkit_path();
//    cout << external_path << endl;
    
    return;
}

nn_terrain::~nn_terrain() {

}
MIN_EXTERNAL(nn_terrain);




