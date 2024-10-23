// implementation of class DArray
#include "DArray.h"
#include <iostream>
#include <cassert>

// default constructor
DArray::DArray() {
	Init();
}

// set an array with default values
DArray::DArray(int nSize, double dValue) 
{
	Init();

	m_pData = new double[nSize];

	for (int i = 0; i < nSize; i++)
	{
		m_pData[i] = dValue;
	}

	m_nSize = nSize;
}

DArray::DArray(const DArray& arr) 
{
	Init();

	SetSize(arr.GetSize());

	m_pData = new double[m_nSize];

	for (int i = 0; i < m_nSize; i++)
	{
		m_pData[i] = arr[i];
	}
	

}

// deconstructor
DArray::~DArray() {
	Free();
}

// display the elements of the array
void DArray::Print() const {
	
	std::cout << "count:" << m_nSize<<'\t'<<"data:";
	for (int i = 0; i < m_nSize; i++)
	{
		std::cout << ' '<< m_pData[i] ;
	}
	std::cout << std::endl;

}

// initilize the array
void DArray::Init() 
{
	m_nSize = 0;

	m_pData = nullptr;
}

// free the array
void DArray::Free() 
{
	delete[] m_pData;
	m_pData = nullptr;

	m_nSize = 0;
}

// get the size of the array
int DArray::GetSize() const 
{
	return m_nSize;
}

// set the size of the array
void DArray::SetSize(int nSize) 
{
	double* m_pData1 = new double[nSize];

	int count= nSize < m_nSize ? nSize : m_nSize;
	for (int i = 0; i < count; i++)
		m_pData1[i] = m_pData[i];
	for (int i = count; i < nSize; i++)
		m_pData1[i] = 0.;

	delete[] m_pData;
	m_pData = m_pData1;

	m_nSize = nSize;

}

// get an element at an index
const double& DArray::GetAt(int nIndex) const 
{
	assert(nIndex >= 0 && nIndex < m_nSize);//add assert to avoid error

	return m_pData[nIndex];
}

// set the value of an element 
void DArray::SetAt(int nIndex, double dValue) 
{
	assert(nIndex >= 0 && nIndex < m_nSize);//add assert to avoid error

	m_pData[nIndex] = dValue;
}

// overload operator '[]'
const double& DArray::operator[](int nIndex) const 
{

	assert(nIndex >= 0 && nIndex < m_nSize);

	return m_pData[nIndex];
}

// add a new element at the end of the array
void DArray::PushBack(double dValue) 
{
	double* m_pData1 = new double[m_nSize + 1];

	for (int i = 0; i < m_nSize; i++)
		m_pData1[i] = m_pData[i];

	m_pData1[m_nSize] = dValue;

	delete[] m_pData;
	m_pData = m_pData1;
	m_nSize++;
}

// delete an element at some index
void DArray::DeleteAt(int nIndex) 
{
	assert(nIndex >= 0 && nIndex < m_nSize);

	double* m_pData1 = new double[m_nSize - 1];

	for (int i = 0; i < nIndex; i++)
		m_pData1[i] = m_pData[i];

	for (int i = nIndex; i < m_nSize - 1; i++)
		m_pData1[i] = m_pData[i + 1];

	delete[] m_pData;
	m_pData = m_pData1;
	m_nSize--;

}

// insert a new element at some index
void DArray::InsertAt(int nIndex, double dValue) 
{
	assert(nIndex >= 0 && nIndex <= m_nSize);

	double* m_pData1 = new double[m_nSize + 1];

	for (int i = 0; i < nIndex; i++)
		m_pData1[i] = m_pData[i];

	m_pData1[nIndex] = dValue;

	for (int i = nIndex+1; i < m_nSize + 1; i++)
		m_pData1[i] = m_pData[i - 1];

	delete[] m_pData;
	m_pData = m_pData1;
	m_nSize++;


}

// overload operator '='
DArray& DArray::operator = (const DArray& arr) {
	delete[] m_pData;

	m_nSize = arr.GetSize();
	m_pData = new double[m_nSize];

	for (int i = 0; i < m_nSize; i++)
		m_pData[i] = arr[i];

	return *this;
}
