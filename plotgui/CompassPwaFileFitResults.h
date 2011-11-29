///////////////////////////////////////////////////////////////////////////
//
//    Copyright 2010
//
//    This file is part of rootpwa
//
//    rootpwa is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    rootpwa is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with rootpwa.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------
// File and Version Information:
// $Rev::                             $: revision of last commit
// $Author::                          $: author of last commit
// $Date::                            $: date of last commit
//
// Description:
//      Header file for the CompassPwaFileFitResults class that provides
//		functionality to read in fit results from
//		txt files generated by Compass pwa and store them
//
//
// Author List:
//      Stephan Schmeing          TUM            (original author)
//
//
//-------------------------------------------------------------------------

#ifndef CompassPwaFileFitResults_H
#define CompassPwaFileFitResults_H


#include <fstream>
#include <string>
#include <vector>

#include "TCMatrix.h"

#include "CompassPwaFileBase.h"

namespace rpwa{

	class CompassPwaFileFitResults : public CompassPwaFileBase {
	private:
		// Variables
		unsigned int _NumEvents;
		double _LogLikelihood;
		unsigned int _Rank;
		std::vector<std::string> _WaveNames;
		TCMatrix _FitResults;

		static bool _Debug; ///< if set to true, debug messages are printed

		// Functions

	public:
		// Constructors + Destructors
		CompassPwaFileFitResults(); ///< Constructor
		~CompassPwaFileFitResults(); ///< Destructor

		// Get && Set
		unsigned int NumEvents() const; ///< Returns _NumEvents
		double LogLikelihood() const; ///< Returns _LogLikelihood
		unsigned int Rank() const; ///< Returns _Rank
		const std::vector<std::string>& WaveNames() const; ///< Returns _WaveNames
		const TCMatrix& FitResults() const; ///< Returns _FitResults

		static bool Debug() { return _Debug; } ///< returns debug flag
		static void SetDebug(const bool Debug = true) { _Debug = Debug; } ///< sets debug flag

		// Functions
		bool ReadIn( std::istream& File ); ///< Reads the rest of the information from a fit result file stream and returns 0 if no error occurred or a negative number as the error code
		std::ostream& Print(std::ostream& Out) const; ///< Prints all important variables of class
	};

	inline std::ostream& operator <<( std::ostream& Out, const CompassPwaFileFitResults& FitResult ){
		return FitResult.Print(Out);
	}

} // namespace rpwa

#endif /* CompassPwaFileFitResults_H */
