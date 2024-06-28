#pragma once
#include <vector>

using namespace std;

template <class T>
class CircularBuffer {
public:
	CircularBuffer(){
		curI = 0;
		size = 0;
	}
	void resize(int n) {
		a.clear();
		a.reserve(n);
		size = n;
		for (int i = 0; i < size; ++i) {
			a.push_back(0);
		}
	}
	double getSample(double sample) {
		double r = sample - ((int)sample);
		double x1 = get((int)sample), x2 = get((int)sample+1);
		return x2 * r + x1 * (1 - r);
	}
	double get(int i) {
		if (i >= 0 && i < size) {
			int index = curI - i;
			if (index < 0)
				index += size;
			return a[index];
		}
		return 0;
	}
	void newSample(const T& v) {
		a[curI++] = v;
		if (curI == size) {
			curI = 0;
		}
	}
	T& operator[](int i) {
		if (curI + i > size)
			return a[curI + i-size];
		return a[i];
	}

	const T& operator[](int i) const{
		if (curI + i > size)
			return a[curI + i - size];
		return a[i];
	}

	vector<T> a;
	int curI, size;
};