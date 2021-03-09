#include"ExpandableHashMap.h"
#include<string>
#include<iostream>
#include<cassert>
using namespace std;

int main() {
	ExpandableHashMap<string, double> s;
	s.associate("David", 2.01);
	s.associate("Frank", 3.99);
	s.associate("A", 200);
	s.associate("B", 300);
	s.associate("C", 2.34);
	s.associate("A", 100);
	s.associate("Z", 20);
	s.associate("adw", 3.3);
	assert(s.size() == 7);
	assert(s.find("sdwad") == nullptr);
	double* res=s.find("Frank");
	assert(*res == 3.99);
	(*res) = 10;
	double* res2 = s.find("Frank");
	assert(*res == 10);
	s.reset();
	assert(s.size() == 0);
	cout << "passed all tests" << endl;
	int* a=new int[10];

}