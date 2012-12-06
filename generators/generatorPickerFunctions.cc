
#include<assert.h>
#include<map>

#include<boost/assign/std/vector.hpp>

#include "randomNumberGenerator.h"
#include "libConfigUtils.hpp"
#include "generatorPickerFunctions.h"


using namespace boost::assign;
using namespace libconfig;
using namespace std;
using namespace rpwa;


void massAndTPrimePicker::overrideMassRange(double lowerLimit, double upperLimit) {
	if(not _initialized) {
		printErr << "cannot override massRange on uninitialized massAndTPrimePicker." << endl;
		throw;
	}
	_massRange.first = lowerLimit;
	_massRange.second = upperLimit;
	printInfo << "mass range overriden: new range ]" << _massRange.first << ", " << _massRange.second << "]." << endl;
}


std::pair<double, double> massAndTPrimePicker::massRange() {
	if(not _initialized) {
		printErr << "cannot call massRange on uninitialized massAndTPrimePicker." << endl;
		throw;
	}
	return _massRange;
}


bool massAndTPrimePicker::initTPrimeAndMassRanges(const libconfig::Setting& setting) {
	if(_initialized) {
		printErr << "trying to initialize a massAndTPrimePicker class twice." << endl;
		return false;
	}
	map < string, Setting::Type > mandatoryArguments;
	insert (mandatoryArguments)
		("mass_min", Setting::TypeFloat)
		("mass_max", Setting::TypeFloat);
	if(not checkIfAllVariablesAreThere(&setting, mandatoryArguments)) {
		printErr << "found invalid settings for the mass range of a mass and t' picker." << endl;
		return false;
	}
	_massRange.first = setting["mass_min"];
	_massRange.second = setting["mass_max"];
	if(_massRange.second < _massRange.first) {
		printErr << "'mass_max' must not be smaller than 'mass_min'." << endl;
		return false;
	}
	if(setting.exists("t_prime_min")) {
		if(not setting.lookupValue("t_prime_min", _tPrimeRange.first)) {
			printWarn << "'t_prime_min' setting is invalid. Setting 't_prime_min' to "
			          << _tPrimeRange.first << "." << endl;
		}
	} else {
		printInfo << "'t_prime_min' not specified. Setting it to "
		          << _tPrimeRange.first << "." << endl;
	}
	if(setting.exists("t_prime_max")) {
		if(not setting.lookupValue("t_prime_max", _tPrimeRange.second)) {
			printWarn << "'t_prime_max' setting is invalid. Setting 't_prime_max' to "
			          << _tPrimeRange.second << "." << endl;
		}
	} else {
		printInfo << "'t_prime_max' not specified. Setting it to "
		          << _tPrimeRange.second << "." << endl;
	}
	if(_tPrimeRange.second < _tPrimeRange.first) {
		printErr << "'t_prime_max' must not be smaller than 't_prime_min'."
		         << endl;
		return false;
	}
	return true;
}


uniformMassExponentialTPicker::uniformMassExponentialTPicker()
	: massAndTPrimePicker() { }


uniformMassExponentialTPicker::uniformMassExponentialTPicker(const uniformMassExponentialTPicker& picker)
	: massAndTPrimePicker(picker),
	  _tSlopesForMassBins(picker._tSlopesForMassBins) { }


massAndTPrimePicker* uniformMassExponentialTPicker::clone() const {
	massAndTPrimePicker* retval = new uniformMassExponentialTPicker(*this);
	return retval;
}


bool uniformMassExponentialTPicker::init(const Setting& setting) {
	if(not massAndTPrimePicker::initTPrimeAndMassRanges(setting)) {
		printErr << "could not initialize t' or mass range settings in 'uniformMassExponentialT'." << endl;
		return false;
	}
	map < string, Setting::Type > mandatoryArguments;
	insert (mandatoryArguments)
		("t_slope", Setting::TypeArray)
		("inv_m", Setting::TypeArray);
	if(not checkIfAllVariablesAreThere(&setting, mandatoryArguments)) {
		printErr << "found an invalid settings for function 'uniformMassExponentialT'." << endl;
		return false;
	}
	if(setting["inv_m"].getLength() != setting["t_slope"].getLength()) {
		printErr << "'inv_m' and 't_slope' have to be arrays of the same length." << endl;
		return false;
	}
	if(not (setting["inv_m"][0].isNumber())
			&& (setting["t_slope"][0].isNumber())) {
		printErr << "'inv_m' and 't_slope' have to be array of numbers." << endl;
		return false;
	}
	for(int i = 0; i < setting["inv_m"].getLength(); ++i) {
		_tSlopesForMassBins.push_back(pair<double, double>(setting["inv_m"][i], setting["t_slope"][i]));
	}
	_initialized = true;
	return true;
}


bool uniformMassExponentialTPicker::operator()(double& invariantMass, double& tPrime) {
	if(not _initialized) {
		printErr << "trying to use an uninitialized massAndTPrimePicker." << endl;
		return false;
	}
	assert(not _tSlopesForMassBins.empty());
	TRandom3* randomNumbers = randomNumberGenerator::instance()->getGenerator();
	invariantMass = randomNumbers->Uniform(_massRange.first, _massRange.second);
	double tPrimeSlope = -1.;
	if(_tSlopesForMassBins.size() == 1) {
		tPrimeSlope = _tSlopesForMassBins[0].second;
	} else {
		if(invariantMass > _tSlopesForMassBins[_tSlopesForMassBins.size() - 1].first) {
			tPrimeSlope = _tSlopesForMassBins[_tSlopesForMassBins.size() - 1].second;
		} else {
			unsigned int i = 0;
			for(; (invariantMass > _tSlopesForMassBins[i].first); ++i);
			tPrimeSlope = _tSlopesForMassBins[i].second;
		}
	}
	if(tPrimeSlope < 0) {
		printErr << "error when calculating the t'-slope." << endl;
		return false;
	}
	do {
		tPrime = randomNumbers->Exp(1. / tPrimeSlope);
	} while(tPrime < _tPrimeRange.first || tPrime > _tPrimeRange.second);
	return true;
}


ostream& uniformMassExponentialTPicker::print(ostream& out) {
	out << "'uniformMassExponentialT' weighter parameters:" << endl;
	out << "    minimum Mass ... " << _massRange.first << endl;
	out << "    maximum Mass ... " << _massRange.second << endl;
	out << "    minimum t' ..... " << _tPrimeRange.first << endl;
	out << "    maximum t' ..... " << _tPrimeRange.second << endl;
	if(_tSlopesForMassBins.size() == 1) {
		out << "    t' slope ....... " << _tSlopesForMassBins[0].second	<< endl;
	} else {
		unsigned int nSlots = _tSlopesForMassBins.size();
		out << "    t' slopes:" << endl;
		out << "        mass < " << _tSlopesForMassBins[0].first
		    << " -> t' slope = " << _tSlopesForMassBins[0].second
		    << endl;
		for(unsigned int i = 0; i < nSlots - 1; ++i) {
			out << "        mass in [" << _tSlopesForMassBins[i].first << ", "
			    << _tSlopesForMassBins[i + 1].first << "] -> t' slope = "
			    << _tSlopesForMassBins[i + 1].second << endl;
		}
		out << "        mass >= " << _tSlopesForMassBins[nSlots - 1].first
		    << " -> t' slope = " << _tSlopesForMassBins[nSlots - 1].second
		    << endl;
	}
	return out;
}


polynomialMassAndTPrimeSlopePicker::polynomialMassAndTPrimeSlopePicker()
	: massAndTPrimePicker() { }


polynomialMassAndTPrimeSlopePicker::polynomialMassAndTPrimeSlopePicker(const polynomialMassAndTPrimeSlopePicker& picker)
	: massAndTPrimePicker(picker),
	  _massPolynomial(picker._massPolynomial),
	  _tPrimeSlopePolynomial(picker._tPrimeSlopePolynomial) { }


massAndTPrimePicker* polynomialMassAndTPrimeSlopePicker::clone() const {
	massAndTPrimePicker* retval = new polynomialMassAndTPrimeSlopePicker(*this);
	return retval;
}


bool polynomialMassAndTPrimeSlopePicker::init(const Setting& setting) {
	if(not massAndTPrimePicker::initTPrimeAndMassRanges(setting)) {
		printErr << "could not initialize t' or mass range settings in 'polynomialMassAndTPrime'." << endl;
		return false;
	}
	map < string, Setting::Type > mandatoryArguments;
	insert (mandatoryArguments)
		("coeffs_mass", Setting::TypeArray)
		("coeffs_tslopes", Setting::TypeArray);
	if(not checkIfAllVariablesAreThere(&setting, mandatoryArguments)) {
		printErr << "found an invalid settings for function 'polynomialMassAndTPrime'." << endl;
		return false;
	}
	const Setting& configCoeffsMass = setting["coeffs_mass"];
	unsigned int numberOfMassCoeffs = configCoeffsMass.getLength();
	if(numberOfMassCoeffs < 1) {
		printErr << "'coeffs_mass' array must not be empty." << endl;
		return false;
	}
	if(not configCoeffsMass[0].isNumber()) {
		printErr << "'coeffs_mass' array has to be made up of numbers." << endl;
		return false;
	}
	stringstream strStr;
	strStr << "pol" << (numberOfMassCoeffs - 1);
	_massPolynomial = TF1("generatorPickerMassPolynomial", strStr.str().c_str(), _massRange.first, _massRange.second);
	for(unsigned int i = 0; i < numberOfMassCoeffs; ++i) {
		_massPolynomial.SetParameter(i, configCoeffsMass[i]);
	}
	const Setting& configCoeffsTSlopes = setting["coeffs_tslopes"];
	unsigned int numberOfTSlopeCoeffs = configCoeffsTSlopes.getLength();
	if(numberOfTSlopeCoeffs < 1) {
		printErr << "'coeffs_tslopes' array must not be empty." << endl;
		return false;
	}
	if(not configCoeffsTSlopes[0].isNumber()) {
		printErr << "'coeffs_tslopes' array has to be made up of numbers." << endl;
		return false;
	}
	strStr.str("");
	strStr << "pol" << (numberOfTSlopeCoeffs - 1);
	_tPrimeSlopePolynomial = TF1("generatorPickerMassPolynomial", strStr.str().c_str());
	for(unsigned int i = 0; i < numberOfTSlopeCoeffs; ++i) {
		_tPrimeSlopePolynomial.SetParameter(i, configCoeffsTSlopes[i]);
	}
	_initialized = true;
	return true;
}


bool polynomialMassAndTPrimeSlopePicker::operator()(double& invariantMass, double& tPrime) {
	if(not _initialized) {
		printErr << "trying to use an uninitialized massAndTPrimePicker." << endl;
		return false;
	}
	TRandom3* randomNumbers = randomNumberGenerator::instance()->getGenerator();
	invariantMass = _massPolynomial.GetRandom(_massRange.first, _massRange.second);
	double tPrimeSlope = _tPrimeSlopePolynomial.Eval(invariantMass);
	do {
		tPrime = randomNumbers->Exp(1. / tPrimeSlope);
	} while(tPrime < _tPrimeRange.first || tPrime > _tPrimeRange.second);
	return true;
}


ostream& polynomialMassAndTPrimeSlopePicker::print(ostream& out) {
	out << "'polynomialMassAndTPrime' weighter parameters:" << endl;
	out << "    minimum Mass ........... " << _massRange.first << endl;
	out << "    maximum Mass ........... " << _massRange.second << endl;
	out << "    mass polynomial ........ " << _massPolynomial.GetExpFormula() <<endl;
	out << "    t' slopes polynomial ... " << _tPrimeSlopePolynomial.GetExpFormula() <<endl;
	return out;
}
