/* Copyright (c) 2013 - All Rights Reserved
 *   Thomas Hauth  <Thomas.Hauth@cern.ch>
 *   Joram Berger  <Joram.Berger@cern.ch>
 *   Dominik Haitz <Dominik.Haitz@kit.edu>
 */

#pragma once

#include "Artus/Configuration/interface/SettingsBase.h"

class TestSettings : public SettingsBase {
public:

	TestSettings() : SettingsBase(), m_Level( 1), m_Offset(23) {
	}

	explicit TestSettings(std::string lineName) : SettingsBase(lineName), m_Level(1) {
	}

	std::string ToString() const {
		return "Test setting";
	}

	IMPL_PROPERTY(unsigned int, Level)

	// needs to be overwritte here, because the test
	// cases don't have a json file loaded and
	// the code would fail if a lookup to the json file
	// would happen
	stringvector GetFilters () const override
	{
		return stringvector();
	}
	stringvector & GetTaggingFilters () const override
	{
		return m_taggingFilters;
	}
	mutable stringvector m_taggingFilters;

	long long GetProcessNEvents () const
	{
		return -1;
	}

	long long GetFirstEvent () const
	{
		return 0;
	}

	IMPL_PROPERTY(unsigned int, Offset)
};

