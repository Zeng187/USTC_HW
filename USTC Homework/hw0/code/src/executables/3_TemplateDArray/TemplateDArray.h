#pragma once

#include "assert.h"
#include <iostream>

template <class datatype>
class DArray
{
public:
	DArray();//default constructor
	DArray(int nSize, const datatype& dValue = static_cast<datatype>(0)); //constructor
	DArray(const DArray& arr); // copy constructor
	~DArray(); // deconstructor

	void Reserve(int nSize);//allow enough memory

	void Print() const;

	int GetSize();//get the size
	void SetSize(int nSize);//set the size

	const datatype& GetAt(int nIndex) const; // get the value of an element
	void SetAt(int nIndex, const datatype& dValue); // set the value of an element

	datatype& operator[](int nIndex); // overload operator '[]'
	const datatype & operator[](int nIndex) const; // overload operator '[]'


	void PushBack(const datatype& dValue); // add a new element at the end of the array
	void DeleteAt(int nIndex); // delete an element at some index
	void InsertAt(int nIndex, const datatype& dValue); // insert a new element at some index

	DArray& operator = (const DArray& arr); //overload operator '='

private:
	datatype* m_pData; // the pointer to the array memory
	int m_nSize; // the size of the array
	int m_nMax; //the Maximum size now


	void Init(); // initilize the array
	void Free(); // free the array


};

// default constructor
template <class datatype>
DArray<datatype>::DArray()
{
	Init();
}

// set an array with default values
template <class datatype>
DArray<datatype>::DArray(int nSize, const datatype& dValue)
{
	m_nSize = nSize;

	m_nMax = nSize;

	m_pData = new datatype[m_nSize];

	for (int i = 0; i < nSize; i++)
	{
		m_pData[i] = dValue;
	}

}

template <class datatype>
DArray<datatype>::DArray(const DArray& arr)
{
	m_nSize = arr.m_nSize;

	m_nMax = m_nSize;

	m_pData = new double[m_nMax];
	for (int i = 0; i < m_nSize; i++)
	{
		m_pData[i] = arr[i];
	}

}

// deconstructor
template <class datatype>
DArray<datatype>::~DArray()
{
	Free();
}

// display the elements of the array
template <class datatype>
void DArray<datatype>::Print() const
{
	std::cout << "size= " << m_nSize << '\t' << "data:";
	for (int i = 0; i < m_nSize; i++)
		std::cout << " " << m_pData[i];

	std::cout << std::endl;
}

// initilize the array
template <class datatype>
void DArray<datatype>::Init()
{
	m_pData = nullptr;

	m_nSize = 0;

	m_nMax = 0;
}

// free the array
template <class datatype>
void DArray<datatype>::Free()
{
	delete[] m_pData;
	m_pData = nullptr;

	m_nMax = 0;

	m_nSize = 0;
}

// get the size of the array
template <class datatype>
int DArray<datatype>::GetSize()
{
	return m_nSize;
}

// set the size of the array
template <class datatype>
void DArray<datatype>::SetSize(int nSize)
{
	if (m_nSize == nSize)
		return;

	Reserve(nSize);

	for (int i = m_nSize; i < nSize; i++)
		m_pData[i] = 0.;

	m_nSize = nSize;
}

// get an element at an index
template <class datatype>
const datatype& DArray<datatype>::GetAt(int nIndex) const
{
	assert(nIndex >= 0 && nIndex < m_nSize);

	return m_pData[nIndex];
}

// set the value of an element 
template <class datatype>
void DArray<datatype>::SetAt(int nIndex, const datatype& dValue)
{
	assert(nIndex >= 0 && nIndex < m_nSize);

	m_pData[nIndex] = dValue;
}

// overload operator '[]'
template <class datatype>
datatype& DArray<datatype>::operator[](int nIndex)
{
	assert(nIndex >= 0 && nIndex < m_nSize);

	return m_pData[nIndex];
}

// overload operator '[]'
template <class datatype>
const datatype& DArray<datatype>::operator[](int nIndex) const
{
	assert(nIndex >= 0 && nIndex < m_nSize);

	return m_pData[nIndex];
}

//allocate enough memory
template <class datatype>
void DArray<datatype>::Reserve(int nSize)
{
	if (m_nMax >= nSize)
		return;

	while (m_nMax < nSize)
		m_nMax = m_nMax == 0 ? 1 : 2 * m_nMax;

	datatype* pData = new datatype[m_nMax];
	memcpy(pData, m_pData, m_nSize * sizeof(datatype));//ÄÚ´æ¿½±´º¯Êý

	delete[] m_pData;
	m_pData = pData;
}

// add a new element at the end of the array
template <class datatype>
void DArray<datatype>::PushBack(const datatype& dValue)
{
	Reserve(m_nSize + 1);

	m_pData[m_nSize] = dValue;
	m_nSize++;

}

// delete an element at some index
template <class datatype>
void DArray<datatype>::DeleteAt(int nIndex)
{
	assert(nIndex >= 0 && nIndex < m_nSize);

	for (int i = nIndex + 1; i < m_nSize; i++)
		m_pData[i - 1] = m_pData[i];

	m_nSize--;
}

// insert a new element at some index
template <class datatype>
void DArray<datatype>::InsertAt(int nIndex, const datatype& dValue)
{
	assert(nIndex >= 0 && nIndex <= m_nSize);

	Reserve(m_nSize + 1);

	for (int i = m_nSize; i > nIndex; i--)
		m_pData[i] = m_pData[i - 1];

	m_pData[nIndex] = dValue;

	m_nSize++;
}

// overload operator '='
template <class datatype>
DArray<datatype>& DArray<datatype>::operator = (const DArray<datatype>& arr)
{
	Reserve(arr.m_nSize);

	m_nSize = arr.m_nSize;
	memcpy(m_pData, arr.m_pData, m_nSize * sizeof(datatype));

	return *this;
}
