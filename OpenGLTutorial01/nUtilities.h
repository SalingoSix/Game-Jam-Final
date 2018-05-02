#ifndef _UTILITIES_HG_
#define _UTILTIES_HG_

namespace nUtility
{
	template <class T>
	T getRandInRange(T min, T max)
	{
		double value =
			min + static_cast <double> (rand())
			/ (static_cast <double> (RAND_MAX / (static_cast<double>(max - min))));
		return static_cast<T>(value);
	}
}

#endif