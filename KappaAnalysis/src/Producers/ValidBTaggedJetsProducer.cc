
#include "Artus/KappaAnalysis/interface/Producers/ValidBTaggedJetsProducer.h"


std::string ValidBTaggedJetsProducer::GetProducerId() const {
	return "ValidBTaggedJetsProducer";
}

void ValidBTaggedJetsProducer::Init(KappaSettings const& settings)
{
	KappaProducerBase::Init(settings);
	// is std::map<std::string,std::vector<float>
	std::map<std::string, std::vector<float>> bTagWorkingPointsTmp = Utility::ParseMapTypes<std::string,float>(Utility::ParseVectorToMap(settings.GetBTaggerWorkingPoints()));

	for (auto bTagWorkingPoint : bTagWorkingPointsTmp)
	{
		m_bTagWorkingPoints[bTagWorkingPoint.first] = bTagWorkingPoint.second.at(0);
		if (settings.GetApplyBTagSF() && !settings.GetInputIsData())
		{
			m_bTagSFMethod = ToBTagScaleFactorMethod(boost::algorithm::to_lower_copy(boost::algorithm::trim_copy(settings.GetBTagSFMethod())));
			m_bTagSfMap[bTagWorkingPoint.first] = new BTagSF(settings.GetBTagScaleFactorFile(), settings.GetBTagEfficiencyFile(), bTagWorkingPoint.first);
		}
		// define lambda expression for nbtag per working point
		std::string btagQuantity = std::string("n")+bTagWorkingPoint.first+std::string("btag");
		LambdaNtupleConsumer<KappaTypes>::AddIntQuantity(btagQuantity, [bTagWorkingPoint](KappaEvent const& event, KappaProduct const& product) {
			auto it = product.m_bTaggedJetsByWp.find(bTagWorkingPoint.first);
			return it != product.m_bTaggedJetsByWp.end() ? product.m_bTaggedJetsByWp.at(bTagWorkingPoint.first).size() : 0;
		});
	}
	
	// add possible quantities for the lambda ntuples consumers
	LambdaNtupleConsumer<KappaTypes>::AddIntQuantity("nBJets", [](KappaEvent const& event, KappaProduct const& product) {
		return product.m_bTaggedJets.size();
	});
	LambdaNtupleConsumer<KappaTypes>::AddIntQuantity("nBJets20", [this](KappaEvent const& event, KappaProduct const& product) {
		return KappaProduct::GetNJetsAbovePtThreshold(product.m_bTaggedJets, 20.0);
	});
	LambdaNtupleConsumer<KappaTypes>::AddIntQuantity("nBJets30", [this](KappaEvent const& event, KappaProduct const& product) {
		return KappaProduct::GetNJetsAbovePtThreshold(product.m_bTaggedJets, 30.0);
	});
	LambdaNtupleConsumer<KappaTypes>::AddFloatQuantity("bJetPt", [](KappaEvent const& event, KappaProduct const& product) {
		return product.m_bTaggedJets.size() >= 1 ? product.m_bTaggedJets.at(0)->p4.Pt() : DefaultValues::UndefinedFloat;
	});
	LambdaNtupleConsumer<KappaTypes>::AddFloatQuantity("bJetEta", [](KappaEvent const& event, KappaProduct const& product) {
		return product.m_bTaggedJets.size() >= 1 ? product.m_bTaggedJets.at(0)->p4.Eta() : DefaultValues::UndefinedFloat;
	});
	LambdaNtupleConsumer<KappaTypes>::AddFloatQuantity("bJetPhi", [](KappaEvent const& event, KappaProduct const& product) {
		return product.m_bTaggedJets.size() >= 1 ? product.m_bTaggedJets.at(0)->p4.Phi() : DefaultValues::UndefinedFloat;
	});
	LambdaNtupleConsumer<KappaTypes>::AddFloatQuantity("bJet2Pt", [](KappaEvent const& event, KappaProduct const& product) {
		return product.m_bTaggedJets.size() >= 2 ? product.m_bTaggedJets.at(1)->p4.Pt() : DefaultValues::UndefinedFloat;
	});
	LambdaNtupleConsumer<KappaTypes>::AddFloatQuantity("bJet2Eta", [](KappaEvent const& event, KappaProduct const& product) {
		return product.m_bTaggedJets.size() >= 2 ? product.m_bTaggedJets.at(1)->p4.Eta() : DefaultValues::UndefinedFloat;
	});
	LambdaNtupleConsumer<KappaTypes>::AddFloatQuantity("bJet2Phi", [](KappaEvent const& event, KappaProduct const& product) {
		return product.m_bTaggedJets.size() >= 2 ? product.m_bTaggedJets.at(1)->p4.Phi() : DefaultValues::UndefinedFloat;
	});

	std::string bTaggedJetCSVName = settings.GetBTaggedJetCombinedSecondaryVertexName();
	std::string jetPuJetIDName = settings.GetPuJetIDFullDiscrName();

	LambdaNtupleConsumer<KappaTypes>::AddFloatQuantity("leadingBJetCSV",[bTaggedJetCSVName](KappaEvent const& event, KappaProduct const& product) {
		return product.m_bTaggedJets.size() >= 1 ? static_cast<KJet*>(product.m_bTaggedJets.at(0))->getTag(bTaggedJetCSVName, event.m_jetMetadata) : DefaultValues::UndefinedFloat;
	});
	LambdaNtupleConsumer<KappaTypes>::AddFloatQuantity("leadingBJetPuID",[jetPuJetIDName](KappaEvent const& event, KappaProduct const& product) {
		return product.m_bTaggedJets.size() >= 1 ? static_cast<KJet*>(product.m_bTaggedJets.at(0))->getTag(jetPuJetIDName, event.m_jetMetadata) : DefaultValues::UndefinedFloat;
	});
	LambdaNtupleConsumer<KappaTypes>::AddFloatQuantity("trailingBJetCSV",[bTaggedJetCSVName](KappaEvent const& event, KappaProduct const& product) {
		return product.m_bTaggedJets.size() >= 2 ? static_cast<KJet*>(product.m_bTaggedJets.at(1))->getTag(bTaggedJetCSVName, event.m_jetMetadata) : DefaultValues::UndefinedFloat;
	});
	LambdaNtupleConsumer<KappaTypes>::AddFloatQuantity("trailingBJetPuID",[jetPuJetIDName](KappaEvent const& event, KappaProduct const& product) {
		return product.m_bTaggedJets.size() >= 2 ? static_cast<KJet*>(product.m_bTaggedJets.at(1))->getTag(jetPuJetIDName, event.m_jetMetadata) : DefaultValues::UndefinedFloat;
	});
}

void ValidBTaggedJetsProducer::Produce(KappaEvent const& event, KappaProduct& product,
                                       KappaSettings const& settings) const
{
	assert(event.m_jetMetadata);
	assert(settings.GetBTagWPs().size() > 0);

	for (std::vector<std::string>::const_iterator workingPoint = settings.GetBTagWPs().begin();
	     workingPoint != settings.GetBTagWPs().end(); ++workingPoint)
	{
		for (std::vector<KBasicJet*>::iterator jet = product.m_validJets.begin();
			jet != product.m_validJets.end(); ++jet)
		{
			bool validBJet = true;
			KJet* tjet = static_cast<KJet*>(*jet);

			float combinedSecondaryVertex = tjet->getTag(settings.GetBTaggedJetCombinedSecondaryVertexName(), event.m_jetMetadata);
			float bTagWorkingPoint = m_bTagWorkingPoints.at(*workingPoint);

			if (combinedSecondaryVertex < bTagWorkingPoint ||
				std::abs(tjet->p4.eta()) > settings.GetBTaggedJetAbsEtaCut()) {
				validBJet = false;
			}

			validBJet = validBJet && AdditionalCriteria(tjet, event, product, settings);
			
			//entry point for Scale Factor (SF) of btagged jets
			if (settings.GetApplyBTagSF() && !settings.GetInputIsData())
			{
				//https://twiki.cern.ch/twiki/bin/view/CMS/BTagSFMethods#2a_Jet_by_jet_updating_of_the_b
				if (m_bTagSFMethod == BTagScaleFactorMethod::PROMOTIONDEMOTION) {
				
					int jetflavor = tjet->flavour;
					unsigned int btagSys = BTagSF::kNo;
					unsigned int bmistagSys = BTagSF::kNo;
					
					if (settings.GetBTagShift()<0)
						btagSys = BTagSF::kDown;
					if (settings.GetBTagShift()>0)
						btagSys = BTagSF::kUp;
					if (settings.GetBMistagShift()<0)
						bmistagSys = BTagSF::kDown;
					if (settings.GetBMistagShift()>0)
						bmistagSys = BTagSF::kUp;

					LOG(DEBUG) << "Btagging shifts tag/mistag : " << settings.GetBTagShift() << " " << settings.GetBMistagShift(); 
					
					bool taggedBefore = validBJet;
					validBJet = m_bTagSfMap.at(*workingPoint)->isbtagged(tjet->p4.pt(), tjet->p4.eta(), combinedSecondaryVertex,
									jetflavor, btagSys, bmistagSys, settings.GetYear(), bTagWorkingPoint); 
					
					if (taggedBefore != validBJet)
						LOG(DEBUG) << "Promoted/demoted : " << validBJet;
				}
				
				else if (m_bTagSFMethod == BTagScaleFactorMethod::OTHER) {
					//todo
				}
			}

			if (validBJet)
				product.m_bTaggedJetsByWp[*workingPoint].push_back(tjet);
			else
				product.m_nonBTaggedJetsByWp[*workingPoint].push_back(tjet);
		}
	}
	product.m_bTaggedJets = product.m_bTaggedJetsByWp[settings.GetBTagWPs().at(0)];
	product.m_nonBTaggedJets = product.m_nonBTaggedJetsByWp[settings.GetBTagWPs().at(0)];
}

bool ValidBTaggedJetsProducer::AdditionalCriteria(KJet* jet, KappaEvent const& event,
                                KappaProduct& product, KappaSettings const& settings) const
{
	bool validBJet = true;
	return validBJet;
}

