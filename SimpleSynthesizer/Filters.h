#pragma once

#include <math.h>

//1 order low pass filter
class LowPassFilter_1Order
{
protected:
    double prevVout;
    double Cof1;
    double Cof2;

public:
    LowPassFilter_1Order();
    void UpdateParam(double cutOffFreq, double sampleRate);
    double TriggerPulse(const double& vIn);
};


class HighPassFilter_1Order
{
protected:
    double prevVin;
    double prevVout;
    double Coff;
public:
    HighPassFilter_1Order();
    void UpdateParam(double _cutOffFreq, double _sampleRate);

    double TriggerPulse(const double& vIn);
};



//    This is a 2nd order recursive band pass filter of the form.
//      y(n)= a * x(n) - b * y(n-1) - c * y(n-2)
//    where :
//         x(n) = "IN"
//         "OUT" = y(n)
//         c = EXP(-2 * pi * bandWidth / SAMPLE_RATE)
//         b = -4 * c / (1 + c) * COS(2 * pi * centerFreq / SAMPLE_RATE)
//    if scaleFactor == 2 (i.e. noise input)
//         a = sqrt(((1+c)*(1+c)-b*b)*(1-c)/(1+c))
//    else
//         a = sqrt(1-b*b/(4*c))*(1-c)

//    note :     
//         centerFreq is the center frequency in Hertz
//         bandWidth is the band width in Hertz
//         scaleFactor is a scale factor, use 1 for pitched sounds, 2 for noise.
class BandPassFilter
{
protected:
    double c;
    double b;
    double a;
    double y0;
    double y1;
public:
    BandPassFilter();
    void UpdateParam(double centerFreq, double bandWidth, int scaleFactor, double sampleRate);
    double TriggerPulse(const double& vIn);
};