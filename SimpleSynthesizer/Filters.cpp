#include <vector>
#include <iostream>
#include "Filters.h"
#include "Tone.h"

LowPassFilter_1Order::LowPassFilter_1Order()
{
    prevVout = 0;
    Cof1 = 1.0;
    Cof2 = 0;
}

void LowPassFilter_1Order::UpdateParam(double cutOffFreq, double sampleRate)
{
    prevVout = 0;
    double RC = cutOffFreq == 0 ? 0 : 0.5 / pi / cutOffFreq;
    Cof1 = 1 / (1 + RC * sampleRate);
    Cof2 = 1 - Cof1;
}

double LowPassFilter_1Order::TriggerPulse(const double& vIn)
{
    double vOut = Cof1 * vIn + Cof2 * prevVout;
    prevVout = vOut;

    return vOut;
}

HighPassFilter_1Order::HighPassFilter_1Order()
{
    prevVin = prevVout = 0;
    Coff = 1.0;
}

void HighPassFilter_1Order::UpdateParam(double cutOffFreq, double sampleRate)
{
    prevVin = 0;
    prevVout = 0;

    double RC = cutOffFreq == 0 ? 0 : 0.5 / pi / cutOffFreq;
    Coff = RC / (RC + 1 / sampleRate);
}

double HighPassFilter_1Order::TriggerPulse(const double& vIn)
{
    double vOut = (vIn - prevVin + prevVout) * Coff;

    prevVout = vOut;
    prevVin = vIn;

    return vOut;
}

BandPassFilter::BandPassFilter()
{
    a = 1;
    b = 0;
    c = 0;
    y0 = y1 = 0;
}

void BandPassFilter::UpdateParam(double centerFreq, double bandWidth, int scaleFactor, double sampleRate)
{
    c = exp(-2 * pi * bandWidth / sampleRate);
    b = -4 * c / (1 + c) * cos(2 * pi * centerFreq / sampleRate);
    if (scaleFactor == 1)
        a = sqrt(1 - b * b / (4 * c)) * (1 - c);
    else
        a = sqrt(((1 + c) * (1 + c) - b * b) * (1 - c) / (1 + c));

    y0 = y1 = 0;
}

double BandPassFilter::TriggerPulse(const double& vIn)
{
    double y;
    y = a * vIn - b * y1 - c * y0;
    y0 = y1;
    y1 = y;
    return y;
}