#ifndef RLSTRATEGY_H
#define RLSTRATEGY_H

#include "Strategy.h"
#include <vector>
#include <tuple>
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/protobuf/meta_graph.pb.h>

class RLStrategy : public Strategy {
public:
    RLStrategy();
    ~RLStrategy();
    std::tuple<std::string, double> on_data(const TickData& tick) override;
    void train();

private:
    tensorflow::Session* session_;
    tensorflow::GraphDef graph_def_;
    std::deque<std::tuple<TickData, std::string, double, TickData, double>> replay_memory_;
    double learning_rate_;
    double gamma_;
    double epsilon_;
    int replay_memory_size_;
    int batch_size_;
    void preprocess_data();
    void initialize_model();
    void load_model(const std::string& model_path);
    void update_model(const std::vector<std::tuple<TickData, std::string, double, TickData, double>>& batch);
    void store_experience(const TickData& tick, const std::string& action, double quantity, double reward, const TickData& next_tick);
};

#endif // RLSTRATEGY_H

