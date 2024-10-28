#include "RLStrategy.h"
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/protobuf/meta_graph.pb.h>
#include <tensorflow/core/graph/default_device.h>
#include <tensorflow/core/framework/tensor.h>
#include <tensorflow/core/platform/env.h>
#include <iostream>
#include <deque>
#include <random>
#include <tuple>
#include <algorithm>

RLStrategy::RLStrategy()
    : session_(nullptr), learning_rate_(0.001), gamma_(0.99), epsilon_(1.0), replay_memory_size_(10000), batch_size_(32) {
    initialize_model();
}

RLStrategy::~RLStrategy() {
    if (session_) {
        session_->Close();
        delete session_;
    }
}

void RLStrategy::initialize_model() {
    tensorflow::SessionOptions options;
    tensorflow::Status status = tensorflow::NewSession(options, &session_);
    if (!status.ok()) {
        std::cerr << "Error creating TensorFlow session: " << status.ToString() << std::endl;
        return;
    }
    // Load the pre-trained model
    load_model("model_path"); // Replace with actual model path
}

void RLStrategy::load_model(const std::string& model_path) {
    tensorflow::Status status = tensorflow::ReadBinaryProto(tensorflow::Env::Default(), model_path, &graph_def_);
    if (!status.ok()) {
        std::cerr << "Error loading TensorFlow model: " << status.ToString() << std::endl;
        return;
    }
    status = session_->Create(graph_def_);
    if (!status.ok()) {
        std::cerr << "Error creating TensorFlow graph: " << status.ToString() << std::endl;
    }
}

std::tuple<std::string, double> RLStrategy::on_data(const TickData& tick) {
    // Choose action based on epsilon-greedy policy
    std::string action;
    double quantity = 0.0;
    
    if ((double)rand() / RAND_MAX < epsilon_) {
        // Exploration: Random action
        int action_index = rand() % 3;
        quantity = (action_index == 0 || action_index == 1) ? (rand() % 100) + 1 : 0; // Random quantity for buy/sell
        action = (action_index == 0) ? "buy" : (action_index == 1) ? "sell" : "hold";
    } else {
        // Exploitation: Action based on model
        tensorflow::Tensor input_tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({1, 4}));
        auto input = input_tensor.matrix<float>();
        input(0, 0) = tick.bid_price;
        input(0, 1) = tick.bid_size;
        input(0, 2) = tick.ask_price;
        input(0, 3) = tick.ask_size;

        std::vector<tensorflow::Tensor> outputs;
        tensorflow::Status status = session_->Run({{"input", input_tensor}}, {"output"}, {}, &outputs);
        if (!status.ok()) {
            std::cerr << "Error running TensorFlow session: " << status.ToString() << std::endl;
            return {"hold", 0.0}; // Fallback action
        }

        auto output = outputs[0].flat<float>();
        int action_index = std::distance(output.data(), std::max_element(output.data(), output.data() + output.size()));
        quantity = output(1); // Assuming second output is quantity
        action = (action_index == 0) ? "buy" : (action_index == 1) ? "sell" : "hold";
    }

    // Store experience
    store_experience(tick, action, quantity, 0.0, tick); // Placeholder for reward and next tick

    return {action, quantity};
}

void RLStrategy::store_experience(const TickData& tick, const std::string& action, double quantity, double reward, const TickData& next_tick) {
    if (replay_memory_.size() >= replay_memory_size_) {
        replay_memory_.pop_front(); // Remove oldest experience
    }
    replay_memory_.emplace_back(tick, action, quantity, next_tick, reward);
}

void RLStrategy::preprocess_data() {
    // This is where you convert experience to tensors for training
    // Implement data conversion based on your DQN model
}

void RLStrategy::update_model(const std::vector<std::tuple<TickData, std::string, double, TickData, double>>& batch) {
    // Prepare tensors for the model update
    tensorflow::Tensor inputs(tensorflow::DT_FLOAT, tensorflow::TensorShape({batch_size_, 4}));
    tensorflow::Tensor actions(tensorflow::DT_FLOAT, tensorflow::TensorShape({batch_size_, 3}));
    tensorflow::Tensor quantities(tensorflow::DT_FLOAT, tensorflow::TensorShape({batch_size_, 1}));
    tensorflow::Tensor targets(tensorflow::DT_FLOAT, tensorflow::TensorShape({batch_size_, 3}));

    auto input_matrix = inputs.matrix<float>();
    auto action_matrix = actions.matrix<float>();
    auto quantity_matrix = quantities.matrix<float>();
    auto target_matrix = targets.matrix<float>();

    for (size_t i = 0; i < batch.size(); ++i) {
        const auto& [tick, action, quantity, next_tick, reward] = batch[i];
        input_matrix(i, 0) = tick.bid_price;
        input_matrix(i, 1) = tick.bid_size;
        input_matrix(i, 2) = tick.ask_price;
        input_matrix(i, 3) = tick.ask_size;

        // Prepare action tensor
        action_matrix(i, 0) = (action == "buy") ? 1.0 : 0.0;
        action_matrix(i, 1) = (action == "sell") ? 1.0 : 0.0;
        action_matrix(i, 2) = (action == "hold") ? 1.0 : 0.0;

        quantity_matrix(i, 0) = quantity;

        // Compute target value (Q-learning update)
        double next_max_q = 0.0; // Compute max Q-value for next state
        target_matrix(i, 0) = reward + gamma_ * next_max_q; // For "buy"
        target_matrix(i, 1) = reward + gamma_ * next_max_q; // For "sell"
        target_matrix(i, 2) = reward + gamma_ * next_max_q; // For "hold"
    }

    // Run training operation
    tensorflow::Status status = session_->Run({{"inputs", inputs}, {"actions", actions}, {"quantities", quantities}, {"targets", targets}}, {}, {"train_op"}, nullptr);
    if (!status.ok()) {
        std::cerr << "Error running TensorFlow session: " << status.ToString() << std::endl;
    }
}

void RLStrategy::train() {
    if (replay_memory_.size() < batch_size_) {
        return; // Not enough data to train
    }

    // Sample a batch from the replay memory
    std::vector<std::tuple<TickData, std::string, double, TickData, double>> batch;
    std::sample(replay_memory_.begin(), replay_memory_.end(), std::back_inserter(batch), batch_size_,
                std::mt19937{std::random_device{}()});

    // Update the model with the sampled batch
    update_model(batch);

    // Decay epsilon to gradually reduce exploration
    epsilon_ = std::max(0.1, epsilon_ * 0.995); // Ensure epsilon doesn't go below 0.1
}


