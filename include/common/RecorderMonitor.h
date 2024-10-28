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

class RecorderMonitor {
public:
    RecorderMonitor();
    ~RecorderMonitor();

    // Start the metrics collection
    void start();
    // Stop the metrics collection
    void stop();

    // Function to update the metrics
    void updateMetrics(double runTimeSeconds, double timeUntilStopSeconds, int subscribedInstruments, const std::string& instrument);

private:
    void metricsThreadFunction();

    std::shared_ptr<prometheus::Registry> registry_;

    prometheus::Gauge& runTimeGauge_;
    prometheus::Gauge& timeUntilStopGauge_;
    prometheus::Gauge& subscribedInstrumentsGauge_;

    std::unordered_map<std::string, prometheus::Counter*> updatesCounterMap_;

    std::thread metricsThread_;
    std::atomic<bool> stopMetricsThread_;
};