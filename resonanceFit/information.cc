///////////////////////////////////////////////////////////////////////////
//
//    Copyright 2016 Sebastian Uhl (TUM)
//
//    This file is part of ROOTPWA
//
//    ROOTPWA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    ROOTPWA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with ROOTPWA.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------
//
// Description:
//      implementation of the storage for the general information required
//      by the resonance fit
//
//-------------------------------------------------------------------------


#include "information.h"


rpwa::resonanceFit::information::bin::bin(const std::string& fileName,
                                          const double tPrimeMean,
                                          const double rescaleErrors,
                                          const std::vector<std::string>& sysFileNames)
	: _fileName(fileName),
	  _tPrimeMean(tPrimeMean),
	  _rescaleErrors(rescaleErrors),
	  _sysFileNames(sysFileNames)
{
}


std::ostream&
rpwa::resonanceFit::information::bin::print(std::ostream& out, const bool newLine) const
{
	out << "fit result at '" << _fileName << "' with mean tPrime = " << _tPrimeMean;

	if(_rescaleErrors != 1.0) {
		out << ", errors are rescaled by " << _rescaleErrors;
	}

	if(_sysFileNames.size() > 0) {
		out << ", systematics bands are obtained from [ ";
		for(size_t idx = 0; idx < _sysFileNames.size(); ++idx) {
			out << ((idx != 0) ? ", " : "") << "'" << _sysFileNames[idx] << "'";
		}
		out << " ]";
	}

	out << ".";

	if(newLine) {
		out << std::endl;
	}

	return out;
}


rpwa::resonanceFit::information::wave::wave(const std::string& waveName,
                                            const std::pair<double, double>& massLimits,
                                            const std::vector<std::string>& waveNameAlternatives)
	: _waveName(waveName),
	  _massLimits(massLimits),
	  _waveNameAlternatives(waveNameAlternatives)
{
}


std::ostream&
rpwa::resonanceFit::information::wave::print(std::ostream& out, const bool newLine) const
{
	out << "wave '" << _waveName << "'";

	if(_massLimits.first >= 0.0 and _massLimits.second >= 0.0) {
		out << " in mass range " << _massLimits.first << " to " << _massLimits.second << " GeV/c^2";
	} else if(_massLimits.first >= 0.0) {
		out << " for masses above " << _massLimits.first << " GeV/c^2";
	} else if(_massLimits.second >= 0.0) {
		out << " for masses below " << _massLimits.second << " GeV/c^2";
	}

	if(_waveNameAlternatives.size() > 0) {
		out << " with alternative wave names: [ ";
		for(size_t idx = 0; idx < _waveNameAlternatives.size(); ++idx) {
			out << ((idx != 0) ? ", " : "") << "'" << _waveNameAlternatives[idx] << "'";
		}
		out << " ]";
	}

	out << ".";

	if(newLine) {
		out << std::endl;
	}

	return out;
}


rpwa::resonanceFit::information::information(const std::vector<rpwa::resonanceFit::information::bin>& bins,
                                             const std::vector<rpwa::resonanceFit::information::wave>& waves)
	: _bins(bins),
	  _waves(waves)
{
}


std::ostream&
rpwa::resonanceFit::information::print(std::ostream& out, const bool newLine) const
{
	const size_t nrBins = this->nrBins();
	const size_t nrWaves = this->nrWaves();

	out << "fit information for " << nrBins << " bin" << ((nrBins != 1) ? "s" : "") << " and " << nrWaves << " wave" << ((nrWaves != 1) ? "s" : "") << ":";

	for(size_t idxBin = 0; idxBin < nrBins; ++idxBin) {
		out << std::endl;
		out << getBin(idxBin);
	}

	for(size_t idxWave = 0; idxWave < nrWaves; ++idxWave) {
		out << std::endl;
		out << getWave(idxWave);
	}

	if(newLine) {
		out << std::endl;
	}

	return out;
}