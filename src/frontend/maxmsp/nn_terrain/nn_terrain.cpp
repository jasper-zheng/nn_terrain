#include "../../../backend/backend.h"
#include "circular_buffer.h"

#include "c74_min.h"

#include <stdio.h>
#include <iostream>

#include <chrono>
#include <semaphore>
#include <string>
#include <thread>
#include <vector>

#include "terrain.h"
#include "min_path.h"
#include "pen.h"

#include "utils.h"

#include <math.h>

#ifndef VERSION
#define VERSION "UNDEFINED"
#endif

using namespace c74::min;
using namespace c74::min::ui;

class CustomDataset : public torch::data::datasets::Dataset<CustomDataset> {
private:
    torch::Tensor data, targets;
    
public:
    // Constructor
    CustomDataset(torch::Tensor data, torch::Tensor targets)
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

//struct Net : torch::nn::Module {
//  Net() {
//    // Construct and register two Linear submodules.
//    fc1 = register_module("fc1", torch::nn::Linear(784, 64));
//    fc2 = register_module("fc2", torch::nn::Linear(64, 32));
//    fc3 = register_module("fc3", torch::nn::Linear(32, 10));
//  }
//
//  // Implement the Net's algorithm.
//  torch::Tensor forward(torch::Tensor x) {
//    // Use one of many tensor manipulation functions.
//    x = torch::relu(fc1->forward(x.reshape({x.size(0), 784})));
//    x = torch::dropout(x, /*p=*/0.5, /*train=*/is_training());
//    x = torch::relu(fc2->forward(x));
//    x = torch::log_softmax(fc3->forward(x), /*dim=*/1);
//    return x;
//  }
//
//  // Use one of many "standard library" modules.
//  torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr};
//};


class nn_terrain : public object<nn_terrain>, public vector_operator<> {
    
public:
    MIN_DESCRIPTION{"Interface for deep learning models, allow for -234ms pre-delay compensation"};
    MIN_TAGS        { "ui, multitouch" };
    MIN_AUTHOR{"Jasper Shuoyang Zheng built on top the work of Antoine Caillon & Axel Chemla--Romeu-Santos"};
    MIN_RELATED        { "mousestate" };


    // INLETS OUTLETS
    //  std::vector<std::unique_ptr<outlet<>>> m_outlets;
//    inlet<>    input1            { this, "(signal) Sample value at index", "float" };
//    inlet<>    input2            { this, "(signal) Sample value at index", "float" };
//    inlet<>    input3            { this, "(signal) Sample value at index", "float" };
    std::vector<std::unique_ptr<inlet<>>> m_inlets;
    outlet<>    output            { this, "(signal) Sample value at index", "signal" };
    outlet<>    path_output       { this, "(message) Path to the plotted terrain (.txt)", "message"};
    outlet<>    log_output       { this, "(message) logging information", "message"};

    nn_terrain(const atoms &args = {});
    ~nn_terrain();
    
    std::vector<std::unique_ptr<buffer_reference>> buffers;
    
//    vector<vector<float *> *> vector_buffers;
//    vector<std::unique_ptr<float[]>> vector_buffers;
    
    // BACKEND RELATED MEMBERS
    std::unique_ptr<CPPN> t_model;
    bool t_is_backend_init = false;
    c74::min::path t_path;
    
    std::unique_ptr<Backend> m_model;
    
    at::Tensor training_trajectories;
    at::Tensor training_latents;
    bool dataloader_ready = false;
    
    bool m_is_backend_init = false;
    std::string d_method{"decode"};
    std::string e_method{"encode"};
    std::vector<std::string> settable_attributes;
    bool has_settable_attribute(std::string attribute);
    c74::min::path m_path;
    int m_in_dim{1}, m_in_ratio{2048}, m_out_dim{1}, m_out_ratio{1}, m_higher_ratio{2048};

    // BUFFER RELATED MEMBERS
    int m_buffer_size{2048};
    std::unique_ptr<circular_buffer<double, float>[]> m_in_buffer;
    std::unique_ptr<circular_buffer<float, double>[]> m_out_buffer;
    std::vector<std::unique_ptr<float[]>> m_in_model, m_out_model;

    // AUDIO PERFORM
    bool m_use_thread{true}, m_should_stop_perform_thread{false};
    std::unique_ptr<std::thread> m_compute_thread{nullptr};
    std::binary_semaphore m_data_available_lock{0};
    std::binary_semaphore m_result_available_lock{1};

    void operator()(audio_bundle input, audio_bundle output);
    void perform(audio_bundle input, audio_bundle output);
    void cppn_infer();
    atoms freeze_terrain(int width_x, int height_y, string terrain_name, int stride);
    void freeze_and_compile_terrain(int width_x, int height_y, string terrain_name);
    
    float x{0.0f}, y{0.0f}, p{0.0f};
    //caution
    std::unique_ptr<float[]> latent_codes = std::make_unique<float[]>(8);
    
    float twoPi = 4 * acos(0.0);
    
    int inputPhase{0};
    int lastIdx{0};
    
    bool time_to_refresh = false;
    
  // ONLY FOR DOCUMENTATION
    timer<timer_options::defer_delivery> m_timer{ this,
        MIN_FUNCTION {
            time_to_refresh = true;
//            m_timer.delay(5);
            return {};
        }
    };
    
  argument<symbol> path_arg{this, "model path",
                            "Absolute path to the pretrained model."};
  argument<symbol> method_arg{this, "method",
                              "Name of the method to call during synthesis."};
//  argument<int> buffer_arg{
//      this, "buffer size",
//      "Size of the internal buffer (can't be lower than the method's ratio)."};

    message<threadsafe::yes> m_ints{
        this, "float", "control params", MIN_FUNCTION {
            switch (inlet) {
                case 0:
                    x = args[0];
//                    if (time_to_refresh){
//                        cppn_infer();
//                        if (m_model->is_loaded() && enable) {
//                            cout << "cppn infer" << endl;
//                        }
//                        time_to_refresh = false;
//                    }
                        
                    log_output.send("latents", *(latent_codes.get()+0), *(latent_codes.get()+1), *(latent_codes.get()+2),*(latent_codes.get()+3),*(latent_codes.get()+4),*(latent_codes.get()+5),*(latent_codes.get()+6),*(latent_codes.get()+7));
                    break;
                case 1:
                    y = args[0];
                    break;
                case 2:
                    p = args[0];
                    break;
                default:
                    assert(false);
            }
            return {};
        }
    };
    
    attribute<bool> enable {this, "enable_model", false,
        description{"Enable / disable tensor computation"},
        setter{
            [this](const c74::min::atoms &args, const int inlet) -> c74::min::atoms {
                if (m_is_backend_init && m_model->is_loaded() && args[0]){
                    return {true};
                } else {
                    return {false};
                }
                return args;
            }
        }
    };
    attribute<bool> enable_cppn {this, "enable_cppn", false,
        description{"Enable / disable cppn computation"},
        setter{
            [this](const c74::min::atoms &args, const int inlet) -> c74::min::atoms {
                if (m_is_backend_init && t_model->is_loaded() && args[0]){
                    return {true};
                } else {
                    return {false};
                }
                return args;
            }
        }
    };

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
    
    attribute<symbol> cppn_path { this, "cppn_path", "none",
        description {
            "Model to be loaded."
            "The path to a torchscript file."
        },
        setter { MIN_FUNCTION{
            //target t { args };
            if (args[0] == "none"){
                cout << "waiting to load cppn" << endl;
            } else {
                auto model_path = std::string(args[0]);
                if (model_path.substr(model_path.length() - 3) != ".ts"){
                    model_path = model_path + ".ts";
                }
                t_path = path(model_path);
                // TRY TO LOAD MODEL
                if (t_model->load(std::string(t_path))) {
                    cerr << "error during loading" << endl;
                    error();
                    return { -1 };
                }
//                t_model->use_gpu(gpu);
                cout << "loaded model from " << std::string(t_path) << endl;
                cout << t_model->output_tensor << endl;
                auto params = m_model->get_method_params("decode");
                cout << "m_in_dim: " << params[0] << "\n m_in_ratio: " << params[1] << "\n m_out_dim: " << params[2] << "\n m_out_ratio: " << params[3] << endl;
//                atoms saved_path = freeze_terrain(canvas_width,canvas_height, args[0], 1);
            }
            return {args[0]};
        }}
    };
    attribute<int> plot_resolution {this, "plot_resolution", 5, description {"(int) 1~16"}};
    message<> plot_terrain {this, "plot_terrain",
        MIN_FUNCTION {
            if (!m_model->is_loaded()) {
                return {};
            }
            if (!t_model->is_loaded()) {
                return {};
            }
//            cout << "plot terrain" << endl;
            atoms saved_path = freeze_terrain(canvas_width,canvas_height, "", plot_resolution);
//            cout << "saved plot path: " << saved_path[0] << endl;
            path_output.send(saved_path[0]);
            return {};
       }
    };
    
    
    attribute<number> x_range_f {this, "x_range_f", 16.0f, description {"(float) (-x, x)"}};
    attribute<number> y_range_f {this, "y_range_f", 4.0f, description {"(float) (-y, y)"}};
    attribute<number> canvas_width {this, "width", 600, description {"(int) width"}};
    attribute<number> canvas_height {this, "height", 150, description {"(int) height"}};
//    attribute<number> start_pos {this, "start_pos", 0.0, description {"start_pos"}};
  // BOOT STAMP

  message<> maxclass_setup{
      this, "maxclass_setup",
      [this](const c74::min::atoms &args, const int inlet) -> c74::min::atoms {
        cout << "nn_terrain~ " << VERSION << " - torch " << TORCH_VERSION
             << " - built on 2023 - Antoine Caillon & Axel Chemla--Romeu-Santos" << endl;
        return {};
      }};
    message<> dspsetup {this, "dspsetup",
        MIN_FUNCTION {
           m_one_over_samplerate = 1.0 / samplerate();
           return {};
       }
    };
//    message<> load_audio_snippets {this, "load_audio_snippets", "Load audio snippets as buffers",
//        MIN_FUNCTION {
//            cout << "Load audio snippets as buffers" << endl;
//            buffers.erase(std::remove_if(buffers.begin(), buffers.end(), [](auto const& pi){ return *pi % 2 == 0; }), buffers.end());
//            buffers.clear();
//            
//            buffers.push_back(std::make_unique<buffer_reference>(this));
//            buffers[0]->set("c03");
//            
//            buffers.push_back(std::make_unique<buffer_reference>(this));
//            buffers[1]->set("c02");
//            
//            buffers.push_back(std::make_unique<buffer_reference>(this));
//            buffers[2]->set("c01");
//            
////            for (int i(0); i < buffers.size(); i++) {
////                buffer_lock<false> b {*buffers[i]};
////                cout << b.length_in_seconds() << endl;
////            }
//            return {};
//        }
//    };
    
    attribute<symbol> buffer_list { this, "buffer_list", "none",
        description {
            "Buffer names in a list"
        },
        setter { MIN_FUNCTION{
            if (args[0] == "none"){
                cout << "no buffer" << endl;
            } else {
                buffers.erase(std::remove_if(buffers.begin(), buffers.end(), [](auto const& pi){ return *pi % 2 == 0; }), buffers.end());
                buffers.clear();
                
                std::stringstream sub_string((std::string(args[0])));
                string token;
                int buffer_count = 0;
                float length_count = 0.0;
                while (std::getline(sub_string, token, ' ')) {
                    buffers.push_back(std::make_unique<buffer_reference>(this));
                    buffers[buffer_count]->set(token);
                    length_count += buffer_lock<false> {*buffers[buffer_count]}.length_in_seconds();
                    buffer_count++;
                }
                cout << "loaded " << buffer_count << " buffers, length in sec: " << length_count << endl;
            }
            return {args[0]};
        }}
    };
    
    
    attribute<symbol> load_trajectories { this, "load_trajectories", "none",
        description {
            "Trajectories (XYs) to be loaded."
            "The path to a txt file with all trajectories (XYs) as CPPN's training data."
        },
        setter { MIN_FUNCTION{
            if (args[0] == "none"){
                cout << "waiting to load cppn" << endl;
                return {args[0]};
            }
            cout << "loading training data from: " << std::string(args[0]) << endl;
            auto log_path = std::string(args[0]);

            min_path m_log_path = min_path(log_path);

            std::fstream logFile;
            logFile.open(m_log_path, std::ios::in);
            if (!logFile.is_open()) {
                cout << "error opening txt file" << endl;
                return {args[0]};
            }
            
            string line;
            std::vector<float> trajs;
            int line_count = 0;
            
            while (std::getline(logFile, line)) {
                std::stringstream sub_string(line);
                string token;
                int point_count = 0;
                while (std::getline(sub_string, token, ',')) {
                    if (point_count%2==0){
                        trajs.push_back(itof(std::stof(token), canvas_width, x_range_f));
                    } else {
                        trajs.push_back(itof(std::stof(token), canvas_height, y_range_f));
                    }
                    point_count ++;
                }
                line_count += 1;
            }

            training_trajectories = torch::from_blob(trajs.data(), {line_count, 2}, torch::kFloat);
            training_trajectories = training_trajectories.clone();
//            cout << "min: " << training_trajectories.min() << " max: " << training_trajectories.max() << endl;
//            cout << training_trajectories << endl;
            
            return {args[0]};
        }}
    };
    

    message<> convert_audio_to_latents {this, "convert_audio_to_latents", "Conver buffers of audio snippet to latents",
        MIN_FUNCTION {
            cout << "Conver buffer to latent" << endl;
            if (!m_model->is_loaded()){
                return {};
            }
            
            std::vector<at::Tensor> tensor_in;
            
            for (int i(0); i < buffers.size(); i++) {
                buffer_lock<false> b {*buffers[i]};
//                cout << b.frame_count() << endl;
                for (int j(0); j < b.frame_count(); j++){
                    tensor_in.push_back(torch::from_blob(&b.lookup(j, 1), {1, 1, 1}, torch::kFloat));
                }
            }
            auto cat_tensor_in = torch::cat(tensor_in, 2);

//            for (int i(0); i < cat_tensor_in.ndimension(); i++){
//                cout << "dim " << i << ": " << cat_tensor_in.size(i) << endl;
//            }
            
            std::unique_lock<std::mutex> model_lock(m_model->m_model_mutex);
            cat_tensor_in = cat_tensor_in.to(m_model->m_device);
            std::vector<torch::jit::IValue> inputs = {cat_tensor_in};
            
            try {
                training_latents = m_model->m_model.get_method("encode")(inputs).toTensor();
                training_latents = training_latents.reshape({-1, m_in_dim});
            } catch (const std::exception &e) {
              std::cerr << e.what() << '\n';
            }
            model_lock.unlock();
            
            for (int i(0); i < training_latents.ndimension(); i++){
                cout << "latents dim " << i << ": " << training_latents.size(i) << endl;
            }
//            cout << "min: " << training_latents.min() << " max: " << training_latents.max() << endl;
            return {};
        }
    };
    
    std::vector<float> recorded_latents;
    int latent_count = 0;
    
    message<> record_latents {this, "record_latents", "Save the current latents as training examples",
        MIN_FUNCTION {
            if (!m_is_backend_init){
                return {};
            }
            for (int i(0); i < m_in_dim; i++){
                float latent = m_in_buffer[i].get_no_pop();
                recorded_latents.push_back(latent);
            }
            latent_count += 1;
            training_latents = torch::from_blob(recorded_latents.data(), {latent_count, m_in_dim}, torch::kFloat);
            training_latents = training_latents.clone();
            for (int i(0); i < training_latents.ndimension(); i++){
                cout << "latents dim " << i << ": " << training_latents.size(i) << endl;
            }
            log_output.send("latent_len", static_cast<int>(training_latents.size(0)));
            return {};
        }
    };
    
    

    std::unique_ptr<torch::data::StatelessDataLoader<torch::data::datasets::MapDataset<CustomDataset, torch::data::transforms::Stack<torch::data::Example<>>>, torch::data::samplers::RandomSampler>> data_loader;
    
    std::unique_ptr<torch::optim::Adam> optimizer;
    
    message<> print_training_examples {this, "print_training_examples", "Print some training pairs",
        MIN_FUNCTION {
            for (torch::data::Example<>& batch : *data_loader) {
                cout << "data: " << endl << batch.data[0] << endl;
                cout << "target: " << endl << batch.target[0] << endl;
            }
            cout << "latent min: " << training_latents.min() << " max: " << training_latents.max() << endl;
            cout << "trajectory min: " << training_trajectories.min() << " max: " << training_trajectories.max() << endl;
            return {};
        }
    };
    
    message<> create_dataloader {this, "create_dataloader", "Create dataloader to train the terrain model",
        MIN_FUNCTION {
            if (!t_model->is_loaded()){
                return {};
            }
//            cout << "Create dataloader" << endl;
            
            for (int i(0); i < training_latents.ndimension(); i++){
                cout << "latents dim " << i << ": " << training_latents.size(i) << endl;
            }
            for (int i(0); i < training_trajectories.ndimension(); i++){
                cout << "trajectory dim " << i << ": " << training_trajectories.size(i) << endl;
            }
            at::Tensor d_latents;
            at::Tensor d_trajs;
            
            if (training_latents.size(0) > training_trajectories.size(0)){
                cout << "Latents size > trajectories size, using all trajectories with extra latents" << endl;
                d_latents = training_latents.index({torch::indexing::Slice(torch::indexing::None, training_trajectories.size(0))});
                d_trajs = training_trajectories;
            } else {
                cout << "Trajectories size > latents size, using all latents with extra trajectories" << endl;
                d_latents = training_latents;
                d_trajs = training_trajectories.index({torch::indexing::Slice(torch::indexing::None, training_latents.size(0))});
            }
                
            t_model->m_model.train();

            std::vector<at::Tensor> parameters;
            for (const auto& x : t_model->m_model.parameters()) {
                parameters.push_back(x);
            }
//            cout << "parameters: " << parameters.size() << endl;
            optimizer = std::make_unique<torch::optim::Adam>(parameters, /*lr=*/0.001);
            
            auto dataset = CustomDataset(d_trajs,d_latents).map(torch::data::transforms::Stack<>());
            
//            cout << "dataset size: " << dataset.size().value() << endl;
            log_output.send("dataset_len", static_cast<float>(dataset.size().value()));
            data_loader = torch::data::make_data_loader(std::move(dataset),
                                                             torch::data::DataLoaderOptions().batch_size(32).workers(2)
                                                             );
            dataloader_ready = true;
            return {};
        }
    };
    
    attribute<bool> cppn_gpu{this, "cppn_gpu", false,
        description{"Enable / disable cppn gpu usage when available"},
        setter{
            [this](const c74::min::atoms &args, const int inlet) -> c74::min::atoms {
                if (m_is_backend_init){
                    if (t_model->is_loaded()){
                        t_model->use_gpu(bool(args[0]));
//                        cout << "cppn gpu: " << bool(args[0]) << endl;
                    }
                }
                return args;
            }
        }
    };
    
    attribute<number> batch_per_epoch {this, "batch_per_epoch", 3, description {"(int) number of batch per training epoch"}};
    message<> train_batch {this, "train_batch", "Train the terrain model with one batch",
        MIN_FUNCTION {
            if (!dataloader_ready){
                return {};
            }
            cout << "Training the terrain model with one batch" << endl;
            t_model->m_model.train();
            
            size_t batch_index = 0;
            float loss_log = 0.0;
            for (int i(0); i < batch_per_epoch; i++){
                for (auto& batch : *data_loader) {
                    optimizer->zero_grad();
                    std::vector<torch::jit::IValue> inputs;
                    inputs.push_back(batch.data.to(t_model->m_device));
                    torch::Tensor prediction = t_model->m_model.forward(inputs).toTensor();
                    
                    torch::Tensor target = batch.target;
                    
                    // TODO: gpu?
                    torch::Tensor loss = torch::mse_loss(prediction, target.to(t_model->m_device));
                    loss.backward();
                    optimizer->step();
                    
//                    cout << "Batch: " << batch_index << " | Loss: " << loss.to(torch::kCPU).item<float>() << endl;
                    loss_log+=loss.to(torch::kCPU).item<float>();
                    
                    batch_index++;
                }
            }
            loss_log /= batch_index;
            log_output.send("loss", loss_log);
            t_model->m_model.eval();
            return {};
        }
    };
    
    message<> save_cppn {this, "save_cppn", "Save the CPPN model",
        MIN_FUNCTION {
            if (!t_model->is_loaded()){
                return {};
            }
            cout << "Save the CPPN model" << endl;
            t_model->save("cppn_model.pt");
            return {};
        }
    };
    
private:
//    double m_playback_position        { 0.0 };    // normalized range
//    size_t m_record_position        { 0 };        // native range
    double m_one_over_samplerate    { 1.0 };
    
    string external_path;
    
};

void model_perform_loop(nn_terrain *nn_instance) {
  std::vector<float *> in_model, out_model;

  for (auto &ptr : nn_instance->m_in_model)
    in_model.push_back(ptr.get());

  for (auto &ptr : nn_instance->m_out_model)
    out_model.push_back(ptr.get());

  while (!nn_instance->m_should_stop_perform_thread) {
      if (nn_instance->m_data_available_lock.try_acquire_for(std::chrono::milliseconds(200))) {
          nn_instance->cppn_infer();
          nn_instance->m_model->perform(in_model, out_model,
                                        nn_instance->m_buffer_size,
                                        nn_instance->d_method, 1);
          nn_instance->m_result_available_lock.release();
      }
  }
}


bool nn_terrain::has_settable_attribute(std::string attribute) {
    for (std::string candidate : settable_attributes) {
        if (candidate == attribute)
            return true;
    }
    return false;
}

void fill_with_zero(audio_bundle output) {
    for (int c(0); c < output.channel_count(); c++) {
        auto out = output.samples(c);
        for (int i(0); i < output.frame_count(); i++) {
            out[i] = 0.;
        }
    }
}

void nn_terrain::operator()(audio_bundle input, audio_bundle output) {
//    auto out = output.samples(0);
//    auto in_1 = input.samples(0);
//    auto in_2 = input.samples(1);
//    auto in_3 = input.samples(2);
//    auto dsp_vec_size = output.frame_count(); //128

//  CHECK IF MODEL IS LOADED AND ENABLED
    if (!m_model->is_loaded() || !enable) {
        fill_with_zero(output);
        return;
    }
    perform(input, output);
}



void nn_terrain::perform(audio_bundle input, audio_bundle output) {
    auto vec_size = input.frame_count();// 128
  // COPY INPUT TO CIRCULAR BUFFER
    for (int c(0); c < input.channel_count(); c++) { // 8
        auto in = input.samples(c);
        // TODO: now we're intervening on the input buffer to replace it with the latent codes
        // but we should be creating the latent codes here
        // TODO: bypass this if cppn is not enabled
        if (enable_cppn){
            for (int i(0); i < vec_size; i++){
                in[i] = *(latent_codes.get()+c);
            }
        }
        m_in_buffer[c].put(in, vec_size);
    }
    
    if (m_in_buffer[0].full()) { // BUFFER IS FULL //2048
        if (m_result_available_lock.try_acquire()) {
            // TRANSFER MEMORY BETWEEN INPUT CIRCULAR BUFFER AND MODEL BUFFER
            for (int c(0); c < m_in_dim; c++){
                m_in_buffer[c].get(m_in_model[c].get(), m_buffer_size);
            }
            // TRANSFER MEMORY BETWEEN OUTPUT CIRCULAR BUFFER AND MODEL BUFFER
            for (int c(0); c < m_out_dim; c++){
                m_out_buffer[c].put(m_out_model[c].get(), m_buffer_size);
            }
            // SIGNAL PERFORM THREAD THAT DATA IS AVAILABLE
            m_data_available_lock.release();
        }
    }

    // COPY CIRCULAR BUFFER TO OUTPUT
    for (int c(0); c < output.channel_count(); c++) {
        auto out = output.samples(c);
        m_out_buffer[c].get(out, vec_size);
    }
}

void nn_terrain::cppn_infer(){
    if (!t_model->is_loaded()){
        return;
    }
    float xx = itof(x, canvas_width, x_range_f);
    float yy = itof(y, canvas_height, y_range_f);
    vector<float> float_in{xx, yy};
    at::Tensor tensor_in = torch::from_blob(float_in.data(), {1, 2}, torch::kFloat);
    
    std::unique_lock<std::mutex> model_lock(t_model->m_model_mutex);
    tensor_in = tensor_in.to(t_model->m_device);
    vector<torch::jit::IValue> inputs = {tensor_in};
    at::Tensor tensor_out;
    try {
        tensor_out = t_model->m_model.forward(inputs).toTensor();
    } catch (const std::exception &e) {
        cout << e.what() << '\n';
    }
    model_lock.unlock();
    
    tensor_out = tensor_out.to(torch::kCPU);
    tensor_out = tensor_out.reshape({1, -1});
    auto out_ptr = tensor_out.contiguous().data_ptr<float>();
    float first_idx = *out_ptr * std::max(p,0.005f);
    *out_ptr = first_idx;
//    cout << *out_ptr << endl;
    memcpy(latent_codes.get(), out_ptr, m_in_dim*sizeof(float));
}

// TODO: move this to backend
atoms nn_terrain::freeze_terrain(int width_x, int height_y, string terrain_name, int stride){
    cout << "freeze terrain" << endl;
    
    // TODO: try locking model to use InferenceMode
//    c10::InferenceMode guard;
    
    vector<vector<float>> terrain_canvas;
    string src_content = "";
    
    for (int i = 0; i < height_y; i+=stride){
        vector<float> line;
        float y = itof(i, canvas_height, y_range_f);
        for (int j = 0; j < width_x; j+=stride){
            vector<torch::jit::IValue> test_inputs;
            float x = itof(j, canvas_width, x_range_f);
            test_inputs.push_back(torch::tensor({ {x, y} }).to(t_model->m_device));
            
            at::Tensor output_tensor = t_model->m_model.forward(test_inputs).toTensor();
            torch::Tensor ten = output_tensor.to(torch::kFloat).to(torch::kCPU);
            
            atoms result(ten.data_ptr<float>(), ten.data_ptr<float>() + 1);
            number latent_code = scale(clamp(static_cast<float>(result[0]), -1.5f, 4.0f), -1.5f, 4.0f, 0.0f, 1.0f);
            line.push_back(latent_code);
            
            src_content += std::to_string(static_cast<float>(latent_code));
            if ((j+1) < width_x){
                src_content += ",";
            }
        }
        terrain_canvas.push_back(line);
        src_content += "\n";
    }
    cout << "terrain compiled: " << terrain_canvas.size() << ", " << terrain_canvas[0].size() << endl;
    std::stringstream s;
    s << stride;
    atoms results = create_log_and_save(terrain_name + "stride_" + s.str() + "_terrain_", external_path, src_content);
    return results;
}

nn_terrain::nn_terrain(const atoms &args){
          
          t_model = std::make_unique<CPPN>();
          t_is_backend_init = true;

  m_model = std::make_unique<Backend>();
  m_is_backend_init = true;

  // CHECK ARGUMENTS
  if (!args.size()) {
    return;
  }
  if (args.size() > 0) { // ONE ARGUMENT IS GIVEN
    auto model_path = std::string(args[0]);
    if (model_path.substr(model_path.length() - 3) != ".ts")
      model_path = model_path + ".ts";
    m_path = path(model_path);
  }
  if (args.size() > 1) { // TWO ARGUMENTS ARE GIVEN
//    d_method = std::string(args[1]);
//      d_method = "forward";
      d_method = "decode";
  }
  if (args.size() > 2) { // THREE ARGUMENTS ARE GIVEN
    m_buffer_size = int(args[2]);
  }

  // TRY TO LOAD MODEL
  if (m_model->load(std::string(m_path))) {
    cerr << "error during loading" << endl;
    error();
    return;
  }
          

  m_model->use_gpu(gpu);
  

  m_higher_ratio = m_model->get_higher_ratio();

  // GET MODEL'S METHOD PARAMETERS
  auto params = m_model->get_method_params(d_method);

  // GET MODEL'S SETTABLE ATTRIBUTES
  try {
    settable_attributes = m_model->get_settable_attributes();
  } catch (...) {
  }

  if (!params.size()) {
    error("method " + d_method + " not found !");
  }

  m_in_dim = params[0];
//  m_in_ratio = params[1];
//  m_out_dim = params[2];
//  m_out_ratio = params[3];
          cout << "m_in_dim: " << m_in_dim << "\n m_in_ratio: " << m_in_ratio << "\n m_out_dim: " << m_out_dim << "\n m_out_ratio: " << m_out_ratio << endl;

  if (!m_buffer_size) {
    // NO THREAD MODE
    m_use_thread = false;
    m_buffer_size = m_higher_ratio;
  } else if (m_buffer_size < m_higher_ratio) {
    m_buffer_size = m_higher_ratio;
    cerr << "buffer size too small, switching to " << m_buffer_size << endl;
  } else {
    m_buffer_size = power_ceil(m_buffer_size);
  }

  // Calling forward in a thread causes memory leak in windows.
  // See https://github.com/pytorch/pytorch/issues/24237
#ifdef _WIN32
  m_use_thread = false;
#endif

  // CREATE INLETS, OUTLETS and BUFFERS
  m_in_buffer = std::make_unique<circular_buffer<double, float>[]>(m_in_dim);
  for (int i(0); i < m_in_dim; i++) {
    std::string input_label = "";
    try {
      input_label = m_model->get_model()
                        .attr(d_method + "_input_labels")
                        .toList()
                        .get(i)
                        .toStringRef();
    } catch (...) {
      input_label = "(signal) model input " + std::to_string(i);
    }
    m_inlets.push_back(std::make_unique<inlet<>>(this, input_label, "float"));
    m_in_buffer[i].initialize(m_buffer_size); //4096
    m_in_model.push_back(std::make_unique<float[]>(m_buffer_size));
  }
   
  m_out_buffer = std::make_unique<circular_buffer<float, double>[]>(m_out_dim);
  for (int i(0); i < m_out_dim; i++) {
      std::string output_label = "";
      try {
          output_label = m_model->get_model()
              .attr(d_method + "_output_labels")
              .toList()
              .get(i)
              .toStringRef();
      } catch (...) {
          output_label = "(signal) model output " + std::to_string(i);
      }
      //    m_outlets.push_back(std::make_unique<outlet<>>(this, output_label, "signal"));
      m_out_buffer[i].initialize(m_buffer_size);
      m_out_model.push_back(std::make_unique<float[]>(m_buffer_size));
  }
    

  if (m_use_thread) {
      m_compute_thread = std::make_unique<std::thread>(model_perform_loop, this);
  }
          
    external_path = min_devkit_path();
    cout << external_path << endl;

    m_timer.delay(1000);
}

nn_terrain::~nn_terrain() {

  m_should_stop_perform_thread = true;
  if (m_compute_thread)
    m_compute_thread->join();
}
MIN_EXTERNAL(nn_terrain);
