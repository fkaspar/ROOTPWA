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
//      Header file for the CompassPwaFileObject class that provides
//		functionality to read in fit results and integrals from
//		txt files generated by Compass pwa
//
//
// Author List:
//      Stephan Schmeing          TUM            (original author)
//
//
//-------------------------------------------------------------------------

#ifndef CompassPwaFileObject_H
#define CompassPwaFileObject_H

#include <fstream>
#include <string>

#include "CompassPwaFileBase.h"
#include "CompassPwaFileFitResults.h"
#include "CompassPwaFileNormIntegrals.h"
#include "CompassPwaFilePhaseSpaceIntegrals.h"

namespace rpwa{

	enum CompassPwaFileObjectStatus{
		Error = -1,							///< An error has occurred during read in process
		NotLoaded,							///< ReadFromFile has not been run yet
		///< Types
		FitResult,							///< PWA fit results
		PhaseSpaceIntegral,					///< PWA phase space integrals
		AcceptanceCorrectedNormIntegral,	///< PWA acceptance corrected norm integrals
		NotAcceptanceCorrectedNormIntegral	///< PWA not acceptance corrected norm integrals
	};

	class CompassPwaFileObject{
	private:
		// Variables
		CompassPwaFileObjectStatus _Status; ///< Status of the Data
		const CompassPwaFileBase *_DataObject;
		///< (const CompassPwaFileFitResults *) for fit results
		///< (const CompassPwaFilePhaseSpaceIntegrals *) for phase space integrals
		///< (const CompassPwaFileNormIntegrals *) for norm integrals ( acceptance corrected and not acceptance corrected)
		double _MassBinStart;
		double _MassBinEnd;
		double _tBinStart;
		double _tBinEnd;

		static bool _Debug; ///< if set to true, debug messages are printed

		// Functions
		bool ReadBin( std::istream& File ); ///< Reads the mass and t' bin from file stream
		template<class T, CompassPwaFileObjectStatus ReturnValue> CompassPwaFileObjectStatus Read( std::istream& File ); ///< Reads the rest of the information a file specified in the template and returns -1 if an error occurred or ReturnValue if no error occurred

	public:
		// Constructors + Destructors
		CompassPwaFileObject(); ///< Constructor
		~CompassPwaFileObject(); ///< Destructor

		// Get && Set
		CompassPwaFileObjectStatus Status() const; ///< Returns _Status;
		std::string StatusMessage() const; ///< Returns _Status as a string containing the corresponding message
		double MassBinStart() const; ///< Returns _MassBinStart;
		double MassBinEnd() const; ///< Returns _MassBinEnd;
		double tBinStart() const; ///< Returns _tBinStart;
		double tBinEnd() const; ///< Returns _tBinEnd;
		const CompassPwaFileBase *DataObject() const; ///< Returns _DataObject;

		static bool Debug() { return _Debug; } ///< returns debug flag
		static void SetDebug(const bool Debug = true) { _Debug = Debug; } ///< sets debug flag

		// Functions
		CompassPwaFileObjectStatus ReadFromFile( std::string FileString ); ///< Reading in the given file by determining it's type and calling the appropriate function to read in this type
		void Clear(); ///< Clears the file object and calls destructor of the _DataObject
		std::ostream& Print( std::ostream& Out ) const; ///< Prints all important variables of class
	};

	inline std::ostream& operator <<( std::ostream& Out, const CompassPwaFileObject& FileObject ){
		return FileObject.Print(Out);
	}

} // namespace rpwa

#endif /* CompassPwaFileObject_H */
