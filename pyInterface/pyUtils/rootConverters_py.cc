#include "rootConverters_py.h"

#include<TClonesArray.h>
#include<TFile.h>
#include<TLorentzRotation.h>
#include<TPython.h>
#include<TRandom3.h>
#include<TVector3.h>
#include<TTree.h>

#include<ampIntegralMatrix.h>
#include<fitResult.h>

namespace bp = boost::python;

template<typename T>
PyObject* rpwa::py::convertToPy(const T& cxxObj) {
	T* newCxxObj = new T(cxxObj);
	return TPython::ObjectProxy_FromVoidPtr(newCxxObj, newCxxObj->ClassName(), true);
}

template<typename T>
T rpwa::py::convertFromPy(PyObject* pyObj) {
	TObject* TObj = (TObject*)(TPython::ObjectProxy_AsVoidPtr(pyObj));
	T cxxObj = dynamic_cast<T>(TObj);
	return cxxObj;
}

template<typename T>
int rpwa::py::setBranchAddress(T objectPtr, PyObject* pyTree, const std::string& name)
{
		TTree* tree = rpwa::py::convertFromPy<TTree*>(pyTree);
		if(not tree) {
			PyErr_SetString(PyExc_TypeError, "Got invalid input for tree when executing rpwa::py::setBranchAddress()");
			bp::throw_error_already_set();
		}
		static std::map<T, T*> pointerMap;
		if(pointerMap.find(objectPtr) == pointerMap.end())
		{
			pointerMap[objectPtr] = new T(objectPtr);
		}
		return tree->SetBranchAddress(name.c_str(), pointerMap[objectPtr]);
}

void rpwa::py::exportRootConverters() {

	bp::def("__RootConverters_convertToPy_TVector3", &rpwa::py::convertToPy<TVector3>);
	bp::def(
		"__RootConverters_convertFromPy_TVector3", &rpwa::py::convertFromPy<TVector3*>
		, bp::return_internal_reference<1>()
	);

	bp::def("__RootConverters_convertToPy_TLorentzRotation", &rpwa::py::convertToPy<TLorentzRotation>);
	bp::def(
		"__RootConverters_convertFromPy_TLorentzRotation", &rpwa::py::convertFromPy<TLorentzRotation*>
		, bp::return_internal_reference<1>()
	);

	bp::def("__RootConverters_convertToPy_TLorentzVector", &rpwa::py::convertToPy<TLorentzVector>);
	bp::def(
		"__RootConverters_convertFromPy_TLorentzVector", &rpwa::py::convertFromPy<TLorentzVector*>
		, bp::return_internal_reference<1>()
	);

	bp::def("__RootConverters_convertToPy_TClonesArray", &rpwa::py::convertToPy<TClonesArray>);
	bp::def(
		"__RootConverters_convertFromPy_TClonesArray", &rpwa::py::convertFromPy<TClonesArray*>
		, bp::return_internal_reference<1>()
	);

	bp::def("__RootConverters_convertToPy_TRandom3", &rpwa::py::convertToPy<TRandom3>);
	bp::def(
		"__RootConverters_convertFromPy_TRandom3", &rpwa::py::convertFromPy<TRandom3*>
		, bp::return_internal_reference<1>()
	);

	bp::def(
		"__RootConverters_convertFromPy_TTree", &rpwa::py::convertFromPy<TTree*>
		, bp::return_internal_reference<1>()
	);

	bp::def("__RootConverters_convertToPy_TMatrixD", &rpwa::py::convertToPy<TMatrixT<double> >);
	bp::def(
		"__RootConverters_convertFromPy_TMatrixD", &rpwa::py::convertFromPy<TMatrixT<double>*>
		, bp::return_internal_reference<1>()
	);

	bp::def(
		"__RootConverters_convertFromPy_TFile", &rpwa::py::convertFromPy<TFile*>
		, bp::return_internal_reference<1>()
	);

	rpwa::py::setBranchAddress<rpwa::fitResult*>(0, 0, "");
	rpwa::py::setBranchAddress<rpwa::ampIntegralMatrix*>(0, 0, "");

}
