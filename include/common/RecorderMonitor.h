#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <string>
#include <memory>
#include "common/logger.hpp"

class RecorderMonitor: public prometheus::Exposer{
public:
    RecorderMonitor(const std::string& date);
    ~RecorderMonitor();

    void updateMetrics(double runTimeSeconds, double timeUntilStopSeconds, int subscribedInstruments, const std::string& instrument);
    int getUpdatesCount();

private:
    std::shared_ptr<prometheus::Registry> registry_;

    prometheus::Gauge& runTimeGauge_;
    prometheus::Gauge& timeUntilStopGauge_;
    prometheus::Gauge& subscribedInstrumentsGauge_;

    std::unordered_map<std::string, prometheus::Counter*> updatesCounterMap_;
};