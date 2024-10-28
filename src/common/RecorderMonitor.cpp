#include "common/RecorderMonitor.h"
#include <iostream>

RecorderMonitor::RecorderMonitor() : 
    stopMetricsThread_(false),
    registry_(std::make_shared<prometheus::Registry>()),
    runTimeGauge_(prometheus::BuildGauge()
            .Name("RecorderRunTimeSeconds")
            .Help("Total runtime of the recorder")
            .Register(*registry_)
            .Add({})),
    timeUntilStopGauge_(prometheus::BuildGauge()
            .Name("RecorderTimeUntilStopSeconds")
            .Help("Time left until the recorder stops")
            .Register(*registry_)
            .Add({})),
    subscribedInstrumentsGauge_(prometheus::BuildGauge()
            .Name("RecorderSubscribedInstruments")
            .Help("Number of subscribed instruments")
            .Register(*registry_)
            .Add({}))
{
    
}

RecorderMonitor::~RecorderMonitor() {
    stop();
}
void RecorderMonitor::start() {
    LOG_INFO("start");
    std::cout << "start" << std::endl;
    metricsThread_ = std::thread(&RecorderMonitor::metricsThreadFunction, this);
}

void RecorderMonitor::stop() {
    stopMetricsThread_ = true;
    if (metricsThread_.joinable()) {
        metricsThread_.join();
    }
}

void RecorderMonitor::updateMetrics(double runTimeSeconds, double timeUntilStopSeconds, int subscribedInstruments, const std::string& instrument) {

    runTimeGauge_.Set(runTimeSeconds);
    timeUntilStopGauge_.Set(timeUntilStopSeconds);
    subscribedInstrumentsGauge_.Set(subscribedInstruments);

    if (updatesCounterMap_.find(instrument) == updatesCounterMap_.end()) {
        updatesCounterMap_[instrument] = &prometheus::BuildCounter()
                                            .Name("RecorderInstrumentUpdatesTotal")
                                            .Help("Total number of updates received for each instrument")
                                            .Register(*registry_)
                                            .Add({{"instrument", instrument}});
    }
    updatesCounterMap_[instrument]->Increment();
}

void RecorderMonitor::metricsThreadFunction() {
    LOG_INFO("metricsThreadFunction");
    prometheus::Exposer exposer{"0.0.0.0:8080"};
    exposer.RegisterCollectable(registry_);

    while (!stopMetricsThread_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}