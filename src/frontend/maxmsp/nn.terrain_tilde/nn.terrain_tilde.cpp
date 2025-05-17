#include "../../../backend/backend.h"
#include "../shared/circular_buffer.h"

#include "c74_min.h"

#include <stdio.h>
#include <iostream>
#include <format>

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

#define CPU torch::kCPU
#define CUDA torch::kCUDA
#define MPS torch::kMPS

#define None at::indexing::None
#define Slice torch::indexing::Slice

using namespace c74::min;
using namespace c74::min::ui;

std::string min_devkit_path() {
#ifdef WIN_VERSION
    char    pathstr[4096];
    HMODULE hm = nullptr;
    if (!GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&min_devkit_path, &hm)) {
        int ret = GetLastError();
        fprintf(stderr, "GetModuleHandle() returned %d\n", ret);
    }
    GetModuleFileNameA(hm, pathstr, sizeof(pathstr));
    // path now is the path to this external's binary, including the binary filename.
    auto filename = strrchr(pathstr, '\\');
    if (filename)
        *filename = 0;
    auto externals = strrchr(pathstr, '\\');
    if (externals)
        *externals = 0;
    path p{ pathstr };    // convert to Max path
    return p;
#endif    // WIN_VERSION

#ifdef MAC_VERSION
    CFBundleRef this_bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.jasperzheng.nn-terrain-"));
    CFURLRef    this_url = CFBundleCopyExecutableURL(this_bundle);
    char        this_path[4096];
    CFURLGetFileSystemRepresentation(this_url, true, reinterpret_cast<UInt8*>(this_path), 4096);
    string this_path_str{ this_path };
    CFRelease(this_url);
    // we now have a path like this:
    // /Users/tim/Materials/min-devkit/externals/min.project.mxo/Contents/MacOS/min.project"
    // so we need to chop off 5 slashes from the end
    auto iter = this_path_str.find("/externals/nn.terrain~.mxo/Contents/MacOS/nn.terrain~");
    this_path_str.erase(iter, strlen("/externals/nn.terrain~.mxo/Contents/MacOS/nn.terrain~"));
    return this_path_str;
#endif    // MAC_VERSION
}


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

class nn_terrain : public object<nn_terrain>, public vector_operator<> {
    
public:
    MIN_DESCRIPTION{"Latent terrain priors for neural audio autoencoders, for generating latent vectors"};
    MIN_TAGS        { "" };
    MIN_AUTHOR{"Jasper Shuoyang Zheng"};
    MIN_RELATED        { "nn~, nn.terrain.gui, nn.terrain.encode, nn.terrain.record" };


    // INLETS OUTLETS
    std::vector<std::unique_ptr<outlet<>>> m_outlets;
    std::vector<std::unique_ptr<inlet<>>> m_inlets;

    nn_terrain(const atoms &args = {});
    ~nn_terrain();
    
    std::vector<std::unique_ptr<buffer_reference>> buffers;
    
    
    // BACKEND
    std::unique_ptr<FCPPN> cppn_model;
    
    bool cppn_init = false;
    c74::min::path t_path;
    
    std::string d_method{"decode"};
    std::string e_method{"encode"};
    
//    std::vector<std::string> settable_attributes;
//    bool has_settable_attribute(std::string attribute);
    
//    c74::min::path m_path;
    int m_in_dim{2}, m_in_ratio{2048}, m_out_dim{8}, m_out_ratio{1}, m_cmax{128}, m_mapping_size{256};
    float m_gauss_scale{0.2};
    
    
    // BUFFER RELATED MEMBERS
    int m_buffer_size{2048};
    std::unique_ptr<circular_buffer<double, float>[]> m_in_buffer;
    std::unique_ptr<circular_buffer<float, double>[]> m_out_buffer;
    std::vector<std::unique_ptr<float[]>> m_in_model, m_out_model;
    std::vector<std::unique_ptr<float[]>> m_latent_out;

    // AUDIO PERFORM
    bool m_use_thread{true}, m_should_stop_perform_thread{false};
    std::unique_ptr<std::thread> m_compute_thread{nullptr};
    std::binary_semaphore m_data_available_lock{0};
    std::binary_semaphore m_result_available_lock{1};

//    void display_args();
    atoms create_dataloader();
    void operator()(audio_bundle input, audio_bundle output);
    void perform(audio_bundle input, audio_bundle output);
    void cppn_infer(float *, std::vector<float *>);
//    atoms freeze_terrain(int width_x, int height_y, string terrain_name, int stride);
    
    at::Tensor sampled_tensor_in;
    min_dict sampled_dict{symbol(true)};
    min_dict terrain_dict{symbol(true)};
    string sampled_str;
    float x_lo_prev{0.0}, x_hi_prev{0.0}, y_lo_prev{0.0}, y_hi_prev{0.0}, stride_prev{0.0};
    int x_res_prev{0}, y_res_prev{0}, plot_resolution_prev{0}, c_prev{0};
    void sample_interval(float x_lo, float x_hi, int x_res,
                         float y_lo, float y_hi, int y_res, int c);
    void create_sampled_tensor_in();
    
//    void freeze_and_compile_terrain(int width_x, int height_y, string terrain_name);
    
//    float x{0.0f}, y{0.0f}, p{0.0f};
    
    //caution
//    std::unique_ptr<float[]> latent_codes = std::make_unique<float[]>(8);
    
    float twoPi = 4 * acos(0.0);
    
    int inputPhase{0};
    int lastIdx{0};
    
    bool time_to_refresh = false;
    
    
    argument<int> control_dim_arg {this, "control_dim", "Dimensionality of the input space.", true};
    argument<int> latent_dim_arg {this, "latent_dim", "Dimensionality of the autoencoder's latent space.", true};
    argument<float> t_gauss_scale_arg {this, "gauss_scale", "(Optional) Gaussian scale of the neural network. A float value between 0-1 is suggested, by default is 0.2. A higher Gaussian scale leads to a noisy terrain. If the Gaussian scale is 0, the Fourier feature mapping layer will be removed, resulting in a smooth (low frequency) terrain."};
    argument<int> t_cmax_arg {this, "network_channel", "(Optional) Number of channels in the neural network's fully connected layers, by default is 128"};
    argument<int> mapping_size_arg {this, "feature_size", "(Optional) Size of the random Fourier feature mapping, higher feature size is suggested when using high dimensional control space, by default is 256."};
    argument<int> buffer_arg {this, "buffer_size", "(Optional) Size of the internal buffer (can't be lower than the decoder method's ratio)."};
    
    argument<symbol> model_path_arg {this, "checkpoint_path", "If only one argument is given, it should be the path to a model checkpoint."};
    
    attribute<bool> enable_cppn {this, "enable_terrain", false, title{": Enable Terrain"},
        description{"Enable / disable cppn computation"},
        setter{
            [this](const c74::min::atoms &args, const int inlet) -> c74::min::atoms {
                if (cppn_init && args[0]){
                    return {true};
                } else {
                    return {false};
                }
                return args;
            }
        }
    };
    
    message<> m_sample_interval {this, "plot_interval", "Sample the terrain across a closed interval to plot into the GUI, only 2D interval dimension is supported at the moment.   <br /><br />Args:  <br />1D [not implemented]: lo, hi, resolution;  <br />2D: x_lo, x_hi, x_resolution, y_lo, y_hi, y_resolution, color_channel (optional, default 1)",
        MIN_FUNCTION {
            //args:
            // x_lo, x_hi, y_lo, y_hi, stride, latent_clamp_min, latent_clamp_max
            // lo, hi, stride, latent_clamp_min, latent_clamp_max
            if (args.size() == 3) {
//                cout << args[0] << " " << args[1] << " " << args[2] << " " << args[3] << " " << args[4] << endl;
                cerr << "plot_interval: 1D sampling not implemented" << endl;
                return {};
            } else if (args.size() == 6) {
                sample_interval(args[0], args[1], args[2], args[3], args[4], args[5], 1);
                return {};
            } else if (args.size() == 7) {
                sample_interval(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
                return {};
            }
            cerr << "plot_interval: wrong number of arguments" << endl;
            return {};
        }
    };

    attribute<int,threadsafe::undefined,limit::clamp> plot_resolution {this, "plot_resolution", 1, title{": Plot Resolution"}, description {"(int) 1~16"},range{{1, 16}}};
    
    attribute<number,threadsafe::undefined,limit::clamp> lr{ this, "training_lr", 0.001, title{": Training Learning Rate"}, description{"learning rate when training the nn"}, range{{0.0001, 0.1}} };
    attribute<int,threadsafe::undefined,limit::clamp> batch_size{ this, "training_batchsize", 32, title{": Training Batch Size"}, description{"batch size when training the nn"}, range{{4, 128}} };
    attribute<int,threadsafe::undefined,limit::clamp> worker{ this, "training_worker", 2, title{": Training Workers"},  description{"number of worker in dataloader"}, range{{0, 4}} };

    attribute<int> epoch_per_cycle {this, "training_epoch", 10, title{": Training Epoch"}, description {"(int) train the model for this many epochs when one train message is received"}};
    
    message<> maxclass_setup{
        this, "maxclass_setup", [this](const c74::min::atoms &args, const int inlet) -> c74::min::atoms {
            cout << "torch version: " << TORCH_VERSION << endl;
            return {};
        }
    };
    
    message<> dspsetup {this, "dspsetup",
        MIN_FUNCTION {
           m_one_over_samplerate = 1.0 / samplerate();
           return {};
       }
    };
    
    at::Tensor training_coords;
    at::Tensor training_latents;
    
    min_dict training_data = {symbol(true)};
    min_dict coord_dict = {symbol(true)};
    min_dict latent_dict = {symbol(true)};
    int coord_len = 0;
    int latent_len = 0;
    
    bool dataset_updated = true;
    bool dataloader_ready = false;
    
    message<> dictionary { this, "dictionary",
        "Use a dictionary to gather training data for the terrain",
        MIN_FUNCTION {
            c74::max::t_symbol    **keys = NULL;
            long        numkeys = 0;
            
            min_dict d = {args[0]};
            c74::max::dictionary_getkeys(d.m_instance, &numkeys, &keys);
            
            for (int i = 0; i < numkeys; i++){
                string key_str = std::string(keys[i]->s_name);
                atom d_data = d[key_str];
                min_dict d_data_dict = {d_data};
                atom count = c74::max::dictionary_getentrycount(d_data_dict.m_instance);
                
                if (key_str == "coordinates"){
                    coord_len = count;
                    if (coord_len == 0) {
                        coord_dict.clear();
                    } else {
                        coord_dict = d_data_dict;
                    }
                    dataset_updated = true;
                } else if (key_str == "latents"){
                    latent_len = count;
                    if (latent_len==0){
                        latent_dict.clear();
                    } else {
                        latent_dict = d_data_dict;
                    }
                    dataset_updated = true;
                } else {
                    cerr << "unknown key, or some dictionaries are empty " << key_str << endl;
                }
            }
            // freeing memory for keys
            if (keys){
                c74::max::dictionary_freekeys(d.m_instance, numkeys, keys);
            }
            training_data.touch();
            m_outlets[m_outlets.size()-1]->send("dataset","dictionary", training_data.name());
            return {};
        }
    };
    

    std::unique_ptr<torch::data::StatelessDataLoader<torch::data::datasets::MapDataset<CustomDataset, torch::data::transforms::Stack<torch::data::Example<>>>, torch::data::samplers::RandomSampler>> data_loader;
    
    std::unique_ptr<torch::optim::Adam> optimizer;
    
    message<> model_summary {this, "model_summary", "Model summary",
        MIN_FUNCTION {
            min_dict net_args_dict {symbol(true)};
            
            net_args_dict["input_dimension"] = m_in_dim;
            net_args_dict["output_dimension"] = m_out_dim;
            net_args_dict["gaussian_scale"] = m_gauss_scale;
            net_args_dict["number_of_channels"] = m_cmax;
            net_args_dict["features_size"] = m_mapping_size;
            net_args_dict["number_of_parameters"] = static_cast<int>(cppn_model->m_model->parameters().size());
            net_args_dict["saving_path"] = (std::filesystem::path(std::to_string(external_path)) / (std::to_string(saving_name)+".pt")).string();
            
            m_outlets[m_outlets.size()-1]->send("summary", "dictionary", net_args_dict.name());
            return {};
        }
    };
    
    min_dict dataset_info_dict {symbol(true)};
    min_dict dataset_co_dict {symbol(true)};
    min_dict dataset_lt_dict {symbol(true)};
    atoms dataset_summary;
    message<> dataset_summary_m {this, "dataset_summary", "Training pairs summary",
        MIN_FUNCTION {
            try{
                if (dataset_updated){
                    dataset_summary = create_dataloader();
                }
                
                if (dataset_summary[0] == 0){
                    cerr << "no data loaded" << endl;
                    return {};
                }
                symbol shape_key{"shape"};
                symbol range_key{"range"};
                
                dataset_info_dict["dataset_len"] = static_cast<int>(dataset_summary[0]);
                
                atoms shape_co = {dataset_summary[3], dataset_summary[4]};
                c74::max::dictionary_appendatoms(dataset_co_dict.m_instance, shape_key, shape_co.size(), &shape_co[0]);
                atoms range_co = {dataset_summary[5], dataset_summary[6]};
                c74::max::dictionary_appendatoms(dataset_co_dict.m_instance, range_key, range_co.size(), &range_co[0]);
                
                atoms shape_lt = {dataset_summary[7], dataset_summary[8]};
                c74::max::dictionary_appendatoms(dataset_lt_dict.m_instance, shape_key, shape_lt.size(), &shape_lt[0]);
                atoms range_lt = {dataset_summary[9], dataset_summary[10]};
                c74::max::dictionary_appendatoms(dataset_lt_dict.m_instance, range_key, range_lt.size(), &range_lt[0]);
                
                dataset_info_dict.touch();
                m_outlets[m_outlets.size()-1]->send("summary", "dictionary", dataset_info_dict.name());
            } catch (const std::exception &e) {
                cerr << e.what() << endl;
            }
            return {};
        }
    };
    
    attribute<bool> cppn_gpu{this, "gpu", false,
        description{"Enable / disable gpu usage when available"},
        setter{
            [this](const c74::min::atoms &args, const int inlet) -> c74::min::atoms {
                if (cppn_init){
                    if (cppn_model->is_loaded()){
                        cppn_model->use_gpu(bool(args[0]));
                        optimizer = std::make_unique<torch::optim::Adam>(cppn_model->m_model->parameters(), static_cast<float>(lr));
                        
                        if (total_epochs>0){
                            cwarn << "Changing device after any training will re-initialise the optimizer, be careful." << endl;
                        }
                        if (bool(args[0])){
                            cwarn << "The terrain model is suggested to be operated on the CPU because it's a very small neural network without any convolutional layer." << endl;
                        }
//                        cout << "cppn gpu: " << bool(args[0]) << endl;
                    }
                }
                return args;
            }
        }
    };
    
    
    
    int total_epochs = 0;
    message<> train_cycle {this, "train", "Train the terrain model with one batch",
        MIN_FUNCTION {
            if (dataset_updated){
                try{
                    dataset_summary = create_dataloader();
                    if (dataset_summary[0]==0){
                        cerr << "no data loaded" << endl;
                        return {};
                    }
                } catch (const std::exception &e) {
                    cerr << e.what() << endl;
                    return {};
                }
            }
//            return {};
//            cout << "Training the terrain model with " << epoch_per_cycle <<" batch" << endl;
            
            cppn_model->m_model->train();
            
            size_t batch_count = 0;
            float loss_log = 0.0;
            try {
                for (int i(0); i < epoch_per_cycle; i++){
                    for (auto& batch : *data_loader) {
                        
                            optimizer->zero_grad();
                            torch::Tensor batch_data = batch.data.to(cppn_model->m_device);
                            torch::Tensor prediction = cppn_model->m_model->forward(batch_data);
                            
                            torch::Tensor batch_target = batch.target.to(cppn_model->m_device);
                            
                            torch::Tensor loss = torch::mse_loss(prediction, batch_target).to(cppn_model->m_device);
                            loss.backward();
                            
                            optimizer->step();
                            
                            loss_log+=loss.to(torch::kCPU).item<float>();
                        
                        batch_count++;
                    }
                }
            } catch (const std::exception &e) {
                cerr << e.what() << endl;
            }
            if (batch_count == 0){
                cerr << "error" << endl;
                cppn_model->m_model->eval();
                return {};
            }
            total_epochs += epoch_per_cycle;
            loss_log /= batch_count;
            cppn_model->m_model->eval();
            
            m_outlets[m_outlets.size()-1]->send("loss", loss_log);
            m_outlets[m_outlets.size()-1]->send("epoch", total_epochs);
            return {};
        }
    };
    attribute<symbol> external_path{ this, "saving_path", "none", title{": Saving Path"}, setter{ MIN_FUNCTION{
        if (args[0] == "none") {
            symbol min_path_this = min_devkit_path();
            return { min_path_this };
        } else {
            try {
                min_path min_path_this = min_path(static_cast<string>(args[0]));
                return { args[0] };
            } catch (...) {
                symbol min_path_this = min_devkit_path();
                cwarn << "Error reading folder, set to default" << endl;
                return { min_path_this };
            }
        }
        } }
    };
    attribute<symbol> saving_name{ this, "saving_name", "UntitledTerrain", title{": Saving Name"}, setter{ MIN_FUNCTION{
        std::string str = std::string(args[0]);
        str.erase(std::remove(str.begin(), str.end(), ' '), str.end()); // remove spaces
        if (str.empty()) {
            return { "UntitledTerrain" };
        } else {
            return { args[0] };
        }
        } }
    };
    
    message<> save_cppn {this, "checkpoint", "Checkpoint and save the terrain model",
        MIN_FUNCTION {
            if (!cppn_init){
                return {};
            }
            std::string saved_name = std::to_string(saving_name);
            if (saved_name.substr(saved_name.length() - 3) != ".pt"){
                saved_name = saved_name + ".pt";
            }
            try {
                if(cppn_model->save(std::to_string(external_path), saved_name, m_in_dim, m_out_dim, m_cmax, m_gauss_scale, m_mapping_size)){
                    cout << saved_name << " saved to: " << std::to_string(external_path) << endl;
                }
            } catch (const std::exception &e) {
                cerr << "error saving the model:" << e.what() << endl;
            }
            return {};
        }
    };
    
private:
//    double m_playback_position        { 0.0 };    // normalized range
//    size_t m_record_position        { 0 };        // native range
    double m_one_over_samplerate    { 1.0 };
    
//    string external_path;
    
};

atoms nn_terrain::create_dataloader() {
    int coord_count = static_cast<int>(c74::max::dictionary_getentrycount(coord_dict.m_instance));
    int latent_count = static_cast<int>(c74::max::dictionary_getentrycount(latent_dict.m_instance));
    
    if (coord_count==0 || latent_count==0){
        cerr << "coordinates or latents not set" << endl;
        return {{0}};
    }
    
    int dataset_len = 0;
    int traj_len = 0;
    
    if (latent_count > coord_count){
        cwarn << "latent trejectories "<<latent_count<<" > audio trejectories "<<coord_count<<", using all audio with " << latent_count - coord_count << " latent trejectories remained" << endl;
        traj_len = coord_count;
    } else if (latent_count < coord_count){
        cwarn << "audio trejectories "<<coord_count<<" > latent trejectories"<<latent_count<<", using all latent trejectories with " << coord_count - latent_count << "audio trejectories remained" << endl;
        traj_len = latent_count;
    } else {
        traj_len = latent_count;
    }
    
    std::vector<at::Tensor> coord_trim;
    std::vector<at::Tensor> latent_trim;
    
    for (int i(0); i < traj_len; i++){
        
        std::vector<float> coords;
        std::vector<float> latents;
        
        int point_count = 0;
        atom c_data = coord_dict[std::to_string(i+1)];
        min_dict c_seg_dict = {c_data};
        atoms seg_c_atoms_tp = c_seg_dict["0"];
        
        atom l_data = latent_dict[std::to_string(i+1)];
        min_dict l_seg_dict = {l_data};
        atoms seg_l_atoms_tp = l_seg_dict["0"];
        
        point_count = seg_c_atoms_tp.size() > seg_l_atoms_tp.size() ? seg_l_atoms_tp.size() : seg_c_atoms_tp.size();
        
        int coord_dim = static_cast<int>(c74::max::dictionary_getentrycount(c_seg_dict.m_instance));
        int latent_dim_here = static_cast<int>(c74::max::dictionary_getentrycount(l_seg_dict.m_instance));
        
        if (m_out_dim != latent_dim_here){
            cerr << "latents dimension " << latent_dim_here << " doesn't match latent_dim" << endl;
            return {{0}};
        }
        for (int k(0); k < m_in_dim; k++){
            atoms seg_c_atoms;
            try {
                seg_c_atoms = c_seg_dict[std::to_string(k)];
            } catch (const std::exception &e) {
                cerr << "error getting coordinates from dictionary: " << e.what() << endl;
                return {{0}};
            }
            for (int j(0); j < point_count; j++){
                coords.push_back(static_cast<float>(seg_c_atoms[j]));
            }
        }
        for (int k(0); k < m_out_dim; k++){
            atoms seg_l_atoms;
            try {
                seg_l_atoms = l_seg_dict[std::to_string(k)];
            } catch (const std::exception &e) {
                cerr << "error getting latents from dictionary: " << e.what() << endl;
                return {{0}};
            }
            for (int j(0); j < point_count; j++){
                latents.push_back(static_cast<float>(seg_l_atoms[j]));
            }
        }
        coord_trim.push_back(torch::from_blob(coords.data(), {m_in_dim, point_count}, torch::kFloat).clone());
        latent_trim.push_back(torch::from_blob(latents.data(), {m_out_dim, point_count}, torch::kFloat).clone());
        dataset_len += point_count;
    }

    
    if (coord_trim.size() == 0 || latent_trim.size() == 0){
        cerr << "error converting to tensor" << endl;
        return {{0}};
    }
    training_coords = torch::cat(coord_trim, 1).permute({1,0});
    training_latents = torch::cat(latent_trim, 1).permute({1,0});
    
//    for (int i(0); i < training_coords.ndimension(); i++){
//        cout << "coords dim " << i << ": " << training_coords.size(i) << endl;
//    }
//    for (int i(0); i < training_latents.ndimension(); i++){
//        cout << "latents dim " << i << ": " << training_latents.size(i) << endl;
//    }
        
    
    std::vector<int> traj_info = {
        static_cast<int>(training_coords.size(0)),
        static_cast<int>(training_coords.size(1))
    };
    std::vector<float> traj_range_info = {
        training_coords.min().item<float>(),
        training_coords.max().item<float>()
    };
    std::vector<int> lat_info = {
        static_cast<int>(training_latents.size(0)),
        static_cast<int>(training_latents.size(1))
    };
    std::vector<float> lat_range_info = {
        training_latents.min().item<float>(),
        training_latents.max().item<float>()
    };
    
    auto dataset = CustomDataset(training_coords,training_latents).map(torch::data::transforms::Stack<>());
    
    m_outlets[m_outlets.size()-1]->send("dataset_length", static_cast<float>(dataset.size().value()));
    data_loader = torch::data::make_data_loader(std::move(dataset),
                                                torch::data::DataLoaderOptions().batch_size(batch_size).workers(worker).drop_last(false));
    dataloader_ready = true;
    dataset_updated = false;
    
    return {{
        dataset_len,
        coord_count,
        latent_count,
        traj_info[0], // trajs.size(0)
        traj_info[1], // trajs.size(1)
        traj_range_info[0], // traj_range_info.min()
        traj_range_info[1], // traj_range_info.max()
        lat_info[0],  // latents.size(0)
        lat_info[1],  // latents.size(1)
        lat_range_info[0],  // lat_range_info.min()
        lat_range_info[1]   // lat_range_info.max()
    }};
}

void model_perform_loop(nn_terrain *nn_instance) {
  std::vector<float *> in_model, out_model;

  for (auto &ptr : nn_instance->m_in_model)
    in_model.push_back(ptr.get());

  for (auto &ptr : nn_instance->m_out_model)
    out_model.push_back(ptr.get());

  while (!nn_instance->m_should_stop_perform_thread) {
      if (nn_instance->m_data_available_lock.try_acquire_for(std::chrono::milliseconds(200))) {
//          nn_instance->m_model->perform(in_model, out_model,
//                                        nn_instance->m_buffer_size,
//                                        nn_instance->d_method, 1);
          nn_instance->m_result_available_lock.release();
      }
  }
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
//  CHECK IF MODEL IS LOADED AND ENABLED
    if (!cppn_init || !enable_cppn) {
        fill_with_zero(output);
        return;
    }
    perform(input, output);
}

void nn_terrain::sample_interval(float x_lo, float x_hi, int x_res, float y_lo, float y_hi, int y_res, int c){
    if (x_lo == x_lo_prev && x_hi == x_hi_prev && y_lo == y_lo_prev && y_hi == y_hi_prev && y_res == y_res_prev && x_res == x_res_prev && plot_resolution == plot_resolution_prev && c == c_prev){
        // do nothing
    } else {
        x_lo_prev = x_lo;
        x_hi_prev = x_hi;
        y_lo_prev = y_lo;
        y_hi_prev = y_hi;
        x_res_prev = x_res;
        y_res_prev = y_res;
        plot_resolution_prev = plot_resolution;
        c_prev = c;
        create_sampled_tensor_in();
    }
    if (sampled_tensor_in.numel() == 0){
        return;
    }
    
    terrain_dict.clear();
    std::unique_lock<std::mutex> model_lock(cppn_model->m_model_mutex);
    if (c == 1){
        try {
            for (int i(0); i < sampled_tensor_in.size(0); i++){ // y
                at::Tensor tensor_in = sampled_tensor_in.index({i}).to(cppn_model->m_device);
                at::Tensor tensor_out = cppn_model->m_model->forward(tensor_in).to(torch::kFloat).to(torch::kCPU);
                tensor_out = tensor_out.index({Slice(None), 0});
                auto ten_out_ptr = tensor_out.contiguous().data_ptr<float>();
                atoms result(ten_out_ptr, ten_out_ptr + sampled_tensor_in.size(1));
                
                symbol skey{i};
                c74::max::dictionary_appendatoms(terrain_dict.m_instance, skey, result.size(), &result[0]);
            }
        } catch (const std::exception &e) {
            cerr << e.what() << endl;
        }
    } else if (c == 4) {
        try {
            
            for (int i(0); i < sampled_tensor_in.size(0); i++){ // y
                min_dict row_dict = {};
                at::Tensor tensor_in = sampled_tensor_in.index({i}).to(cppn_model->m_device);
                at::Tensor tensor_out = cppn_model->m_model->forward(tensor_in).to(torch::kFloat).to(torch::kCPU);
                
                for (int j(0); j < 4; j++){
                    
                    at::Tensor a_out = tensor_out.index({Slice(None), j}).clone().contiguous();
                    
                    auto ten_out_ptr = a_out.data_ptr<float>();
                    atoms result(ten_out_ptr, ten_out_ptr + sampled_tensor_in.size(1));
                    
                    symbol skey{j};
                    c74::max::dictionary_appendatoms(row_dict.m_instance, skey, result.size(), &result[0]);
                }
                terrain_dict[std::to_string(i)] = row_dict;
            }
        } catch (const std::exception &e) {
            cerr << e.what() << endl;
        }
    }
    model_lock.unlock();
    
    atom count = c74::max::dictionary_getentrycount(terrain_dict.m_instance);
    if (count == 0){
        return;
    }
    sampled_dict.touch();
    m_outlets[m_outlets.size()-2]->send("dictionary", sampled_dict.name());
}

void nn_terrain::create_sampled_tensor_in(){
    vector<float> tensor_in_data;
    
    float x_stride = (x_hi_prev-x_lo_prev)/x_res_prev;
    float y_stride = (y_hi_prev-y_lo_prev)/y_res_prev;
    
    for (int i(0); i < y_res_prev; i+=plot_resolution){
        for (int j(0); j < x_res_prev; j+=plot_resolution){
            tensor_in_data.push_back(x_lo_prev + j * x_stride);
            tensor_in_data.push_back(y_lo_prev + i * y_stride);
        }
    }
    int canvas_width = static_cast<int>(std::ceil(x_res_prev / static_cast<float>(plot_resolution)));
    int canvas_height = static_cast<int>(std::ceil(y_res_prev / static_cast<float>(plot_resolution)));
    int canvas_size = canvas_width * canvas_height * 2;
    
    if (tensor_in_data.size() != canvas_size){
        cerr << "tensor_in_data size mismatch: " << tensor_in_data.size() << ", " << canvas_size << endl;
        return;
    }
    sampled_tensor_in = torch::from_blob(tensor_in_data.data(), {canvas_height, canvas_width, 2}, torch::kFloat);
    sampled_tensor_in = sampled_tensor_in.clone();
}

void nn_terrain::perform(audio_bundle input, audio_bundle output) {
    auto vec_size = input.frame_count();// 128
  // COPY INPUT TO CIRCULAR BUFFER
    for (int c(0); c < input.channel_count(); c++) { // 2
        auto in = input.samples(c);
        m_in_buffer[c].put(in, vec_size);
    }
    
    if (m_in_buffer[0].full()) { // BUFFER IS FULL // 2048
        
        std::vector<float *> out_model;
        for (int c(0); c < m_out_dim; c++){
            out_model.push_back(m_latent_out[c].get());
        }
        vector<float> float_in;
        for (int c(0); c < input.channel_count(); c++){
            float_in.push_back(*input.samples(c));
            m_in_buffer[c].reset();
        }
        
        cppn_infer(float_in.data(), out_model);
        
        for (int c(0); c < m_out_dim; c++){
            m_out_buffer[c].put(m_latent_out[c].get(), m_buffer_size);
        }
    }

    // COPY CIRCULAR BUFFER TO OUTPUT
    for (int c(0); c < output.channel_count(); c++) { // 8
        auto out = output.samples(c);
        if (enable_cppn){
            m_out_buffer[c].get(out, vec_size);
//            memcpy(out, m_latent_out[c].get(), vec_size * sizeof(float));
        }
    }
}

void nn_terrain::cppn_infer(float* float_in, std::vector<float *> out_buffer){
    if (!cppn_init){
        return;
    }
    at::Tensor tensor_in = torch::from_blob(float_in, {1, 2}, torch::kFloat);
    
    std::unique_lock<std::mutex> model_lock(cppn_model->m_model_mutex);
    tensor_in = tensor_in.to(cppn_model->m_device);
    at::Tensor tensor_out;
    try {
        
        tensor_out = cppn_model->m_model->forward(tensor_in);
        
        tensor_out = tensor_out.clamp_min({-100.0f}).clamp_max({100.0f});
        
        // -----------------------------
        
        tensor_out = tensor_out.repeat_interleave(m_buffer_size).reshape({1, m_out_dim, -1}); //2048 is buffersize
        
        // -----------------------------
        
    } catch (const std::exception &e) {
        cout << e.what() << endl;
    }
    model_lock.unlock();
    
    tensor_out = tensor_out.to(torch::kCPU);
    tensor_out = tensor_out.reshape({1 * m_out_dim, -1}); // -> [8, 2048]
    auto out_ptr = tensor_out.contiguous().data_ptr<float>();
    
    for (int i(0); i < out_buffer.size(); i++) {
      memcpy(out_buffer[i], out_ptr + i * m_buffer_size, m_buffer_size * sizeof(float));
    }
}



nn_terrain::nn_terrain(const atoms &args){
    
//    t_model = std::make_unique<CPPN>();

    // arguments:
    // (path)
    // (in_dim, out_dim)
    // (in_dim, out_dim, gauss_scale)
    // (in_dim, out_dim, gauss_scale, c_max)
    // (in_dim, out_dim, gauss_scale, c_max, buffer_size)
    // (in_dim, out_dim, gauss_scale, c_max, mapping_size, buffer_size)
    torch::serialize::InputArchive archive;
    
    // CHECK ARGUMENTS
    if (!args.size()) {
        return;
    }
    if (args.size() == 1) { // ONE ARGUMENT IS GIVEN
        if (args[0].a_type == 3){
            std::string model_path = std::string(args[0]);
            if (model_path.substr(model_path.length() - 3) != ".pt")
                model_path = model_path + ".pt";
            min_path m_path = min_path(model_path);
            
            try {
                // Load the archive from the specified file path
                archive.load_from(std::string(m_path));
                
                // Load initialization arguments
                torch::Tensor in_dim_tensor, out_dim_tensor, c_max_tensor, gauss_scale_tensor, mapping_size_tensor;
                archive.read("m_in_dim", in_dim_tensor);
                archive.read("m_out_dim", out_dim_tensor);
                archive.read("m_cmax", c_max_tensor);
                archive.read("m_gauss_scale", gauss_scale_tensor);
                archive.read("m_mapping_size", mapping_size_tensor);
                
                m_in_dim = in_dim_tensor.item<int>();
                m_out_dim = out_dim_tensor.item<int>();
                m_cmax = c_max_tensor.item<int>();
                m_gauss_scale = gauss_scale_tensor.item<float>();
                m_mapping_size = mapping_size_tensor.item<int>();
            } catch (const std::exception& e) {
                cerr << "Error loading from checkpoint: " << e.what() << endl;
                return;
            }
        } else {
            cerr << "argument should be a path" << endl;
            return;
        }
    }
    if (args.size() >= 2) { // TWO ARGUMENTS ARE GIVEN
        if (args[0].a_type == 1 && args[1].a_type == 1) {
            if (int(args[0]) > 0 && int(args[1]) > 0 && int(args[0]) <= 6 && int(args[1]) <= 1024) {
                m_in_dim = int(args[0]);
                m_out_dim = int(args[1]);
            } else {
                cerr << "control_dim should be positive integers <= 6, latent_dim should be positive integers <= 1024" << endl;
                return;
            }
        } else {
            cerr << "first and second args should be integers" << endl;
            return;
        }
    }
    if (args.size() >= 3) { // THREE ARGUMENTS ARE GIVEN
        if (args[2].a_type == 2) {
            if (float(args[2]) >= 0 && float(args[2]) <= 10) {
                m_gauss_scale = float(args[2]);
            } else {
                cerr << "third arg should be non-negative float <= 10" << endl;
                return;
            }
        } else {
            cerr << "third arg should be a float" << endl;
            return;
        }
    }
    if (args.size() >= 4) { // Four ARGUMENTS ARE GIVEN
        if (args[3].a_type == 1) {
            if (int(args[3]) <= 1024) {
                m_cmax = int(args[3]);
            } else {
                cerr << "fourth arg should be positive integers <= 1024" << endl;
                return;
            }
        } else {
            cerr << "fourth arg be an integer" << endl;
            return;
        }
    }
    if (args.size() >= 5) { // Five ARGUMENTS ARE GIVEN
        if (args[4].a_type == 1) {
            if (int(args[4]) <= 2048 && int(args[4]) >= 16) {
                m_cmax = int(args[4]);
            } else {
                cerr << "fifth arg should be positive integers between [16, 2048]" << endl;
                return;
            }
        } else {
            cerr << "fifth arg be an integer" << endl;
            return;
        }
    }
    if (args.size() >= 6) {
        m_buffer_size = power_ceil(int(args[5]));
    }
    if (args.size() >= 7) {
        cerr << "too many arguments" << endl;
        return;
    }
    
    cppn_model = std::make_unique<FCPPN>();
    
    if (cppn_model->create(m_in_dim, m_out_dim, m_cmax, m_gauss_scale, m_mapping_size)) {
        cerr << "error during creating model" << endl;
        error();
        return;
    }
    
    if (args.size() == 1) { // if loading from a checkpoint
        try{
            cppn_model->m_model->load(archive);
        } catch (const std::exception& e) {
            cerr << "Error loading weights: " << e.what() << endl;
            return;
        }
    }
    
    // TODO: do this:
    cppn_model->m_model->eval();
    
    torch::Tensor test_inputs = torch::ones({1, m_in_dim}, torch::kFloat);
    
    
    auto output_tensor = cppn_model->m_model->forward(test_inputs);
//    cout << "cppn model test output: " << output_tensor << endl;
    
    optimizer = std::make_unique<torch::optim::Adam>(cppn_model->m_model->parameters(), static_cast<float>(lr));
    
    cppn_init = true;

  // Calling forward in a thread causes memory leak in windows.
  // See https://github.com/pytorch/pytorch/issues/24237
//#ifdef _WIN32
  m_use_thread = false;
//#endif

  // CREATE INLETS, OUTLETS and BUFFERS
  m_in_buffer = std::make_unique<circular_buffer<double, float>[]>(m_in_dim);
  for (int i(0); i < m_in_dim; i++) {
    std::string input_label = "(signal) input from control space dimension: " + std::to_string(i);
    m_inlets.push_back(std::make_unique<inlet<>>(this, input_label, "float"));
    m_in_buffer[i].initialize(m_buffer_size); //2048
    m_in_model.push_back(std::make_unique<float[]>(m_buffer_size));
  }
    
  m_out_buffer = std::make_unique<circular_buffer<float, double>[]>(m_out_dim);
  for (int i(0); i < m_out_dim; i++) {
      std::string output_label = "(signal) output value at latent space dimension: " + std::to_string(i);
      m_outlets.push_back(std::make_unique<outlet<>>(this, output_label, "signal"));
      m_out_buffer[i].initialize(m_buffer_size);
      m_out_model.push_back(std::make_unique<float[]>(m_buffer_size));
      m_latent_out.push_back(std::make_unique<float[]>(m_buffer_size));
  }
    m_outlets.push_back(std::make_unique<outlet<>>(this, "(dictionary) sampled interval", "dictionary"));
    m_outlets.push_back(std::make_unique<outlet<>>(this, "(message) logging information, route option: summary, dataset, dataset_length, epoch, loss", "message"));
    

//  if (m_use_thread) {
////      m_compute_thread = std::make_unique<std::thread>(model_perform_loop, this);
//  }
          
    external_path = min_devkit_path();
    
    training_data["coordinates"] = coord_dict;
    training_data["latents"] = latent_dict;
    
    sampled_dict["terrain"] = terrain_dict;
    
    dataset_info_dict["coordinates"] = dataset_co_dict;
    dataset_info_dict["latents"] = dataset_lt_dict;
//    cout << external_path << endl;

//    m_timer.delay(1000);
//    torch::OrderedDict<std::string, torch::Tensor> nb = cppn_model->m_model->named_buffers();
//    for (auto it = nb.begin(); it != nb.end(); ++it) {
//        cout << "name: " << it->key() << endl;
//        cout << "tensor: " << it->value() << endl;
//        cout << "size: " << it->value().sizes() << endl;
//    }
//    min_path model_path;
//    model_path = min_path("cppn_model.pt");
//    torch::Tensor loaded_tensor;
//    try {
//            // Load the tensor from the .pt file
//            torch::load(loaded_tensor, std::string(model_path));
//            cout << "Tensor loaded successfully:\n" << loaded_tensor << endl;
//        } catch (const std::exception& e) {
//            cerr << "Error loading tensor: " << e.what() << endl;
//        }
    cout << "terrain setup finished" << endl;
}

nn_terrain::~nn_terrain() {
    
    terrain_dict.clear();
    latent_dict.clear();
    coord_dict.clear();
    
  m_should_stop_perform_thread = true;
  if (m_compute_thread)
    m_compute_thread->join();
}
MIN_EXTERNAL(nn_terrain);
