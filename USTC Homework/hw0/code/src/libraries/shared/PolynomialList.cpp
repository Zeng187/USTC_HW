#include "PolynomialList.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <assert.h>

#define EPSILON 1.0e-10	

using namespace std;

PolynomialList::PolynomialList(const PolynomialList& other) 
{
    m_Polynomial = other.m_Polynomial;
}

PolynomialList::PolynomialList(const string& file) 
{
    ReadFromFile(file);
}

PolynomialList::PolynomialList(const double* cof, const int* deg, int n) 
{
    for (int i = 0; i < n; i++)
        AddOneTerm(Term(deg[i], cof[i]));
}

PolynomialList::PolynomialList(const vector<int>& deg, const vector<double>& cof) 
{
    for (int i = 0; i < deg.size(); i++)
        AddOneTerm(Term(deg[i], cof[i]));
}

double PolynomialList::coff(int i) const 
{
    for (const auto& term : m_Polynomial)  //迭代器，只读取m_Polynomial中内容，不可修改
    {
        if (term.deg < i)
            break;
        if (term.deg == i)
            return term.cof;
    }

    return 0.;
}

double& PolynomialList::coff(int i) 
{
    
    for (auto& term : m_Polynomial)  //迭代器，可修改
    {
        if (term.deg < i)
            break;
        if (term.deg == i)
            return term.cof;
    }

    //如果不存在此项
    return AddOneTerm(Term(i,0)).cof;


}

void PolynomialList::compress() 
{
    auto term = m_Polynomial.begin();
    while (term != m_Polynomial.end())
    {
        if (abs((*term).cof) < EPSILON)
            term = m_Polynomial.erase(term);
        else
            term++;
    }
}

PolynomialList PolynomialList::operator+(const PolynomialList& right) const 
{
    PolynomialList poly(*this);

    for (const auto& term : right.m_Polynomial)
    {
        poly.AddOneTerm(Term(term.deg,term.cof));
    }

    poly.compress();

    return poly;
}

PolynomialList PolynomialList::operator-(const PolynomialList& right) const 
{
    PolynomialList poly(*this);

    for (const auto& term : right.m_Polynomial)
    {
        poly.AddOneTerm(Term(term.deg, -term.cof));
    }

    poly.compress();

    return poly;
}

PolynomialList PolynomialList::operator*(const PolynomialList& right) const 
{
    PolynomialList poly;

    for (const auto& term1 : this->m_Polynomial)
    {
        for (const auto& term2 : right.m_Polynomial)
        {
            poly.AddOneTerm(Term(term1.deg + term2.deg, term1.cof * term2.cof));
        }
    }

    poly.compress();

    return poly;
}

PolynomialList& PolynomialList::operator=(const PolynomialList& right) 
{
    m_Polynomial = right.m_Polynomial;
    return *this;
}

void PolynomialList::Print() const 
{
    if (m_Polynomial.begin() == m_Polynomial.end())
    {
        std::cout << "0" << std::endl;
    }
    
    for (auto itr = m_Polynomial.begin(); itr != m_Polynomial.end(); itr++) 
    {
        if (itr != m_Polynomial.begin()) {
            if (itr->cof > 0)
                std::cout << "+";
        }

        std::cout << itr->cof;

        if (itr->deg > 0)
            std::cout << "x^" << itr->deg;
    }
    std::cout << std::endl;

}

bool PolynomialList::ReadFromFile(const string& file) 
{
    m_Polynomial.clear();

    ifstream inp;
    inp.open(file.c_str());
    if (!inp.is_open()) {
        cout << "ERROR::Open file fail:" << endl;
        return false;
    }

    char ch;
    int n;
    inp >> ch;
    inp >> n;
    for (int i = 0; i < n; i++) {
        Term nd;
        inp >> nd.deg;
        inp >> nd.cof;

        AddOneTerm(nd);
    }

    inp.close();

    compress();

    return true;
}

PolynomialList::Term& PolynomialList::AddOneTerm(const Term& term) 
{
    /*auto itr = m_Polynomial.begin();

    if (m_Polynomial.begin() == m_Polynomial.end())
    {
        return *m_Polynomial.insert(itr, term);
    }

    for (; itr != m_Polynomial.end(); itr++)
    {
        if (itr->deg > term.deg)
            continue;
        else if (itr->deg == term.deg)
        {
            itr->cof += term.cof;
            return *itr;
        }
        else
        {
            return *m_Polynomial.insert(itr, term);
        }
    }
    return *m_Polynomial.insert(itr, term);*/

    auto itr = m_Polynomial.begin();
    for (; itr != m_Polynomial.end(); itr++) {
        if (itr->deg == term.deg) {
            itr->cof += term.cof;
            return *itr;
        }

        if (itr->deg > term.deg)
            break;
    }
    return *m_Polynomial.insert(itr, term);
    
}
