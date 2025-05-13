//#include "../../../backend/backend.h"
#include "../shared/circular_buffer.h"

#include "c74_min.h"

#include <stdio.h>
#include <iostream>

#include <string>
#include <vector>

#include "../shared/min_path.h"
#include "../shared/min_dictionary.h"

#include "../shared/utils.h"

#include <math.h>

#ifndef VERSION
#define VERSION "UNDEFINED"
#endif

using namespace c74::min;
using namespace c74::min::ui;

typedef struct traj_info_type {
    vector<vector<float>> traj;
    int length;
} traj_info;

class nn_terrain : public object<nn_terrain>, public vector_operator<> {
    
public:
    MIN_DESCRIPTION{"Recording a multichannel signal as latent vectors, for nn.terrain."};
    MIN_TAGS        { "" };
    MIN_AUTHOR{"Jasper Shuoyang Zheng"};
    MIN_RELATED        { "nn.terrain, nn.terrain.encode, nn.terrain.record, nn~" };

    std::vector<std::unique_ptr<inlet<>>> m_inlets;
    
    outlet<>    dataset_output      { this, "(dictionary) Recorded latent vectors, to be sent to nn.terrain", "dictionary"};
    
    outlet<>    latent_len_output   { this, "(list) Number of latents along each trajectory", "list"};
    outlet<>    latent_len_all_output   { this, "(int) Total number of latents along all trajectories", "int"};
    outlet<>    traj_num_output   { this, "(int) Number of trajectories", "int"};

    nn_terrain(const atoms &args = {});
    ~nn_terrain();
    
    
    // BUFFER RELATED MEMBERS
    int m_buffer_size{2048};
    std::unique_ptr<circular_buffer<double, float>[]> m_in_buffer;
    std::unique_ptr<circular_buffer<float, double>[]> m_out_buffer;
    std::vector<std::unique_ptr<float[]>> m_in_model, m_out_model;

    vector<traj_info *> m_trajs;
    
    void operator()(audio_bundle input, audio_bundle output);
    void perform(audio_bundle input, audio_bundle output);
    
    
    argument<int> latent_dim_arg {this, "latent_dim", "(int) Dimensionality of the latent vector.", true};
    argument<int> buffer_arg {this, "buffer_size", "(Optional) Record the signal once per buffer, 2048 by default."};

    
    int latent_dim = 8;
    
    bool recorder_initialised = false;
  // BOOT STAMP
  message<> maxclass_setup{
      this, "maxclass_setup",
      [this](const c74::min::atoms &args, const int inlet) -> c74::min::atoms {
//        cout << "nn_terrain~ " << VERSION << " - torch " << TORCH_VERSION
//             << " - built on 2023 - Antoine Caillon & Axel Chemla--Romeu-Santos" << endl;
        return {};
      }};
    message<> dspsetup {this, "dspsetup",
        MIN_FUNCTION {
           m_one_over_samplerate = 1.0 / samplerate();
           return {};
       }
    };
    
    min_dict training_data = {symbol(true)};
//    min_dict coord_dict = {symbol(true)};
//    min_dict buffer_dict = {symbol(true)};
    min_dict latent_dict = {symbol(true)};
    
    atoms latent_lengths;
    
    int record_count_batch = 0;
    int record_count = 0;
    
    
    attribute<bool> record_latents {this, "record", false,
        description {"Record input signal as latents vectors, once per buffer_size."},
        setter{[this](const c74::min::atoms &args, const int inlet) -> c74::min::atoms {
            if (args[0] && !record_latents){
                // start a new trajectory
                m_in_model.clear();
                m_in_buffer = std::make_unique<circular_buffer<double, float>[]>(latent_dim);
                
                vector<vector<float>> traj;
                for (int i(0); i < latent_dim; i++){
                    m_in_buffer[i].initialize(m_buffer_size); //4096
                    m_in_model.push_back(std::make_unique<float[]>(m_buffer_size));
                    
                    vector<float> latents;
                    traj.push_back(latents);
                }
                m_trajs.push_back(new traj_info{traj, 0});
            } else if (!args[0] && record_latents){
                if (record_count_batch > 0 && !m_trajs.empty()){
//                    latent_lengths.push_back(record_count_batch);
                    record_count_batch = 0;
                    send_recordings();
                }
            }
            return args;
        }
    }};
    
    attribute<int,threadsafe::undefined,limit::clamp> record_limit {this, "record_limit", 2048,
        description {"Maximum number of latents to record; once reached, recording stops."},range{{1, 16384}}};
  
    message<> clear_latents {this, "clear", "Clear recorded latents",
        MIN_FUNCTION {
            latent_dict.clear();
            m_trajs.clear();
            training_data.touch();
            record_count_batch = 0;
            
//            latent_lengths.clear();
            
            send_recordings();
            return {};
        }
    };
    
    message<> bang {this, "bang", "Resend the latents dataset",
        MIN_FUNCTION {
            send_recordings();
            return {};
        }
    };
    
    message<> record_latent {this, "poke", "Record just one latent vector from the signal",
        MIN_FUNCTION {
            if(record_latents){
                cerr << "cannot poke while recording" << endl;
                return {};
            }
            vector<vector<float>> traj;
            for (int i(0); i < latent_dim; i++){
                vector<float> latents;
                float latent = m_in_buffer[i].get_no_pop();
                latents.push_back(latent);
                traj.push_back(latents);
            }
            m_trajs.push_back(new traj_info{traj, 1});
            
            training_data.touch();
            
            send_recordings();
            return {};
        }
    };
    
    void send_recordings() {
        cout << "sending latents" << endl;
        
        if (m_trajs.empty()){
            traj_num_output.send(m_trajs.size());
            latent_len_all_output.send(0);
            return;
        }
        latent_lengths.clear();
        record_count = 0;
        int c = 0;
        for (int i(0); i < m_trajs.size(); i++){
            if (!m_trajs[i]->traj[0].empty()){
                min_dict traj_dict = {};
                for (int z(0); z < latent_dim; z++){
                    
                    atoms result(m_trajs[i]->traj[z].begin(), m_trajs[i]->traj[z].end());
                    symbol skey{z};
                    c74::max::dictionary_appendatoms(traj_dict.m_instance, skey, result.size(), &result[0]);
                    
                }
                latent_dict[std::to_string(c+1)] = traj_dict;
                latent_lengths.push_back(m_trajs[i]->length);
                record_count += m_trajs[i]->length;
                c+= 1;
            }
        }
        training_data.touch();
        
        traj_num_output.send(m_trajs.size());
        latent_len_all_output.send(record_count);
        latent_len_output.send(latent_lengths);
        
        dataset_output.send("dictionary", training_data.name());
    }
    
private:
    double m_one_over_samplerate    { 1.0 };
    
};

void nn_terrain::operator()(audio_bundle input, audio_bundle output) {
    perform(input, output);
}



void nn_terrain::perform(audio_bundle input, audio_bundle output) {
    if (!recorder_initialised){
        return;
    }
        
    auto vec_size = input.frame_count();// 128
  // COPY INPUT TO CIRCULAR BUFFER
    for (int c(0); c < input.channel_count(); c++) { // 8
        auto in = input.samples(c);
        m_in_buffer[c].put(in, vec_size);
    }
    
    if (m_in_buffer[0].full()) { // BUFFER IS FULL //2048
            // TRANSFER MEMORY BETWEEN INPUT CIRCULAR BUFFER AND MODEL BUFFER
        if (record_latents){
            auto& traj {m_trajs.back()};
            for (int c(0); c < latent_dim; c++){
                auto& traj_dim {traj->traj[c]};
                float latent = m_in_buffer[c].get_no_pop();
                traj_dim.push_back(latent);
            }
            traj->length += 1;
            record_count_batch += 1;
            if (record_count_batch >= record_limit){
                record_count_batch = 0;
                record_latents = false;
                send_recordings();
                cwarn << "recording latents stopped because maximum length reached" << endl;
            }
        }
        for (int c(0); c < input.channel_count(); c++) { // 8
            m_in_buffer[c].get(m_in_model[c].get(), m_buffer_size);
        }
        
    }
}
    

nn_terrain::nn_terrain(const atoms &args){

    training_data["latents"] = latent_dict;
  // CHECK ARGUMENTS
    if (!args.size()) {
        return;
    }
    if (args.size() >= 1) { // TWO ARGUMENT IS GIVEN:
      // a_type 1: int, a_type 2: float, a_type 3: symbol
      if (args[0].a_type == 1) { // (int)
          if (int(args[0]) < 1 || int(args[0]) > 1024) {
              cerr << "error: the latent dimension should be in the range [1, 1024]" << endl;
              return;
          }
          latent_dim = int(args[0]);
      } else {
          cerr << "error: the argument should be int (number of latent dimensions) in the range [1, 1024]" << endl;
      }
  }
    if (args.size() >= 2) { // TWO ARGUMENTS ARE GIVEN:
      m_buffer_size = power_ceil(int(args[1]));
  }
    if (args.size() >= 3) { // THREE ARGUMENTS ARE GIVEN
      cerr << "error: maximum one argument" << endl;
        return;
  }
    
    m_in_buffer = std::make_unique<circular_buffer<double, float>[]>(latent_dim);
    for (int i(0); i < latent_dim; i++) {
        std::string input_label = "(signal) latent codes input " + std::to_string(i);
        
        m_inlets.push_back(std::make_unique<inlet<>>(this, input_label, "float"));
        m_in_buffer[i].initialize(m_buffer_size); //4096
        m_in_model.push_back(std::make_unique<float[]>(m_buffer_size));
    }

    recorder_initialised = true;
    return;
}

nn_terrain::~nn_terrain() {

}
MIN_EXTERNAL(nn_terrain);




