#include "common/RecorderMonitor.h"
#include <iostream>

RecorderMonitor::RecorderMonitor(const std::string& date) : 
    prometheus::Exposer{"0.0.0.0:8080"},
    registry_(std::make_shared<prometheus::Registry>()),
    runTimeGauge_(prometheus::BuildGauge()
            .Name("RecorderRunTimeSeconds")
            .Help("Total runtime of the recorder")
            .Register(*registry_)
            .Add({{"record_date", date}})),
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
    RegisterCollectable(registry_);
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

int RecorderMonitor::getUpdatesCount()
{
    int updatesCount = 0;
    for (auto const& it: updatesCounterMap_)
    {
        updatesCount+= it.second->Value();
    }
    return updatesCount;
}