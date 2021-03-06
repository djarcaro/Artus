
#include "Artus/KappaAnalysis/interface/Producers/NumberGeneratedEventsWeightProducer.h"


std::string NumberGeneratedEventsWeightProducer::GetProducerId() const {
	return "NumberGeneratedEventsWeightProducer";
}

void NumberGeneratedEventsWeightProducer::Produce(KappaEvent const& event,
                     KappaProduct & product,
                     KappaSettings const& settings) const
{
	product.m_weights["numberGeneratedEventsWeight"] = (1.0 / settings.GetNumberGeneratedEvents());
}

