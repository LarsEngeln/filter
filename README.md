# filter
templated header-only thread-safe temporal filter

At the moment a moving-average and the 1€-Filter is implemented.
The 1€-Filter is used from [Nicolas Roussel's C++-implementation](http://www.lifl.fr/~casiez/1euro/OneEuroFilter.cc)  

## usage
```
filter::FilterBase<float> flt;
flt = filter::Filter();         // for moving-average
flt = filter::OneEuroFilter();  // for 1€-Filter

// add some values
flt << 0.3f;
flt << 0.1f;
flt << 0.6f;
//  ... etc. 

// get the filtered value
float  result;
flt >> result;
```
