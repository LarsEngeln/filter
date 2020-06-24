/*

	Filter - threadsafe, templated, ease-of-use
	Lars Engeln 2019-20
	mail@lars-engeln.de

	MIT License

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.

*/

#pragma once#

#include <deque>
#include <mutex>

#define _withOneEuro
#ifdef _withOneEuro
#include "OneEuroFilter.hpp" // by Nicolas Roussel (BSD 3)
#endif

#define _withGLM
#ifdef _withGLM
 #include "cinder\CinderGlm.h"  // I use cinder here, but you can refer directly to glm
#endif
#include <memory>

		
template<class T>
class FilterBase {
  public:
	virtual T getValue() = 0;
	virtual void insertValue(T value) = 0;
};
template<class T> inline FilterBase<T>& operator<<(FilterBase<T>& filter, const T value){
	filter.insertValue(value);
	return filter;
}
template<class T> inline FilterBase<T>& operator>>(FilterBase<T>& filter, T& value){
	value = filter.getValue();
	return filter;
}

template<class T>
class Filter : public FilterBase<T>
{
  public:
	Filter(unsigned maxWindowSize = 10) : windowSize(maxWindowSize), currentWindowSize(maxWindowSize), currentValue(0) {};
	virtual ~Filter() {};

	void setMaxWindowSize(unsigned maxWindowSize, bool force = false) {
		windowSize = maxWindowSize;
		if (force)
			currentWindowSize = maxWindowSize;
	};
	unsigned getCurrentWindowSize() {
		return currentWindowSize;
	};
	void increaseWindow() {
		if (currentWindowSize < windowSize)
			currentWindowSize++;
	};
	void decreaseWindow() {
		if (currentWindowSize > 1) {
			currentWindowSize--;
			std::lock_guard<std::mutex> lock(mutex);
			computeCurrentValue();
		}
	};

	T getValue() override {
		std::unique_lock<std::mutex> lock(mutex);
		return currentValue;
	};
	void insertValue(T value) override {
		std::lock_guard<std::mutex> lock(mutex);
		values.push_back(value);
		computeCurrentValue(true);
	};
  private:
	std::deque<T> values;
	T currentValue; // speedUp
	unsigned windowSize;
	unsigned currentWindowSize;
	mutable std::mutex mutex;

	void computeCurrentValue(bool force = false) {
		if (values.size() > currentWindowSize) {
			values.pop_front();
			force = true;
		}

		if (force) {
			T temp = T(0);
			for (auto n : values) {
				temp += n;
			}
			temp /= values.size();
			currentValue = temp;
		}
	}
};
template<class T> inline Filter<T>& operator++(Filter<T>& filter){
	filter.increaseWindow();
	return filter;
}
template<class T> inline Filter<T>& operator--(Filter<T>& filter){
	filter.decreaseWindow();
	return filter;
}

#ifdef _withOneEuro
template<class T = float>
class OneEuroFilter : public FilterBase<T> 
{
  public:
	OneEuroFilter(double mincutoff = 1, double beta = 0.007, double dcutoff = 1, double frequency = 120) : mincutoff(mincutoff), beta(beta), dcutoff(dcutoff), frequency(frequency) {
		filter = std::make_shared<eurofilter::OneEuroFilter>(frequency, mincutoff, beta, dcutoff);
	};
	T getValue() override {
		return currentValue;
	};
	void insertValue(T value) override {
		currentValue = filter->filter((value);
	}
  private:
	double frequency;
	double mincutoff;
	double beta;
	double dcutoff;
	T currentValue;
	std::shared_ptr<eurofilter::OneEuroFilter> filter;
};

#ifdef _withGLM
class OneEuroFilterV2 : public FilterBase<glm::vec2>
{
  public:
	OneEuroFilterV2(double mincutoff = 1, double beta = 0.007, double dcutoff = 1, double frequency = 120) : mincutoff(mincutoff), beta(beta), dcutoff(dcutoff), frequency(frequency) {
		filterX = std::make_shared<eurofilter::OneEuroFilter>(frequency, mincutoff, beta, dcutoff);
		filterY = std::make_shared<eurofilter::OneEuroFilter>(frequency, mincutoff, beta, dcutoff);
	};
	glm::vec2 getValue() override {
		return currentValue;
	};
	void insertValue(glm::vec2 value) override {
		currentValue.x = filterX->filter(value.x);
		currentValue.y = filterY->filter(value.y);
	}
  private:
	double frequency;
	double mincutoff;
	double beta;
	double dcutoff;
	glm::vec2 currentValue;
	std::shared_ptr<eurofilter::OneEuroFilter> filterX;
	std::shared_ptr<eurofilter::OneEuroFilter> filterY;
};

class OneEuroFilterV3 : public FilterBase<glm::vec3>
{
  public:
	OneEuroFilterV3(double mincutoff = 1, double beta = 0.007, double dcutoff = 1, double frequency = 120) : mincutoff(mincutoff), beta(beta), dcutoff(dcutoff), frequency(frequency) {
		filterX = std::make_shared<eurofilter::OneEuroFilter>(frequency, mincutoff, beta, dcutoff);
		filterY = std::make_shared<eurofilter::OneEuroFilter>(frequency, mincutoff, beta, dcutoff);
		filterZ = std::make_shared<eurofilter::OneEuroFilter>(frequency, mincutoff, beta, dcutoff);
	};
	glm::vec3 getValue() override {
		return currentValue;
	};
	void insertValue(glm::vec3 value) override {
		currentValue.x = filterX->filter(value.x);
		currentValue.y = filterY->filter(value.y);
		currentValue.z = filterZ->filter(value.z);
	}
  private:
	double frequency;
	double mincutoff;
	double beta;
	double dcutoff;
	glm::vec3 currentValue;
	std::shared_ptr<eurofilter::OneEuroFilter> filterX;
	std::shared_ptr<eurofilter::OneEuroFilter> filterY;
	std::shared_ptr<eurofilter::OneEuroFilter> filterZ;
};
#endif
#endif

