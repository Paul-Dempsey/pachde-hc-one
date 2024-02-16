// Copyright (C) Paul Chase Dempsey
#include "components.hpp"

namespace pachde {

// draw #d logo
void DrawLogo(NVGcontext * vg, float x, float y, NVGcolor fill, float scale)
{
    bool scaling = scale != 1.0f;
    nvgSave(vg);
    nvgTranslate(vg, x, y);
    if (scaling) {
        nvgScale(vg, scale, scale);
    }
    nvgFillColor(vg, fill);

    // #
    nvgBeginPath(vg);
    nvgMoveTo(vg, 3.0225791, 5.3090902);
    nvgLineTo(vg, 3.9164512, 3.5427485);
    nvgLineTo(vg, 5.0031212, 3.8339209);
    nvgLineTo(vg, 4.1042182, 5.5989145);
    nvgLineTo(vg, 5.7593776, 6.0424131);
    nvgLineTo(vg, 6.6582806, 4.2774195);
    nvgLineTo(vg, 7.7399197, 4.5672439);
    nvgLineTo(vg, 6.8460476, 6.3335855);
    nvgLineTo(vg, 7.6308648, 6.5438766);
    nvgLineTo(vg, 7.3396924, 7.6305466);
    nvgLineTo(vg, 6.3284856, 7.3595946);
    nvgLineTo(vg, 5.6659891, 8.6649013);
    nvgLineTo(vg, 6.9689869, 9.0140385);
    nvgLineTo(vg, 6.6778145, 10.100708);
    nvgLineTo(vg, 5.1484271, 9.6909104);
    nvgLineTo(vg, 4.254555, 11.457252);
    nvgLineTo(vg, 3.1678851, 11.16608);
    nvgLineTo(vg, 4.0617572, 9.399738);
    nvgLineTo(vg, 2.4065978, 8.9562394);
    nvgLineTo(vg, 1.5127257, 10.722581);
    nvgLineTo(vg, 0.4260557, 10.431409);
    nvgLineTo(vg, 1.3199278, 8.665067);
    nvgLineTo(vg, 0.54014147, 8.4561239);
    nvgLineTo(vg, 0.83131382, 7.3694539);
    nvgLineTo(vg, 1.8425206, 7.640406);
    nvgLineTo(vg, 2.4999863, 6.3337512);
    nvgLineTo(vg, 1.2020194, 5.985962);
    nvgLineTo(vg, 1.4931917, 4.899292);
    nvgClosePath(vg);
    nvgPathWinding(vg, NVGsolidity::NVG_HOLE);
    nvgMoveTo(vg, 5.2418157, 7.0684222);
    nvgLineTo(vg, 3.5866563, 6.6249236);
    nvgLineTo(vg, 2.9241597, 7.9302303);
    nvgLineTo(vg, 4.5793191, 8.3737289);
    nvgClosePath(vg);
    nvgFill(vg);

    //d
    nvgBeginPath(vg);
    nvgMoveTo(vg, 13.386835,10.647075);
    nvgLineTo(vg, 13.086226,11.768961);
    nvgLineTo(vg, 10.731774,11.138087);
    nvgLineTo(vg, 10.886797,10.559536);

    nvgQuadTo(vg, 10.063919,11.034625, 9.3042558,10.831074);
    nvgQuadTo(vg, 8.3936666,10.587083, 7.9931509,9.6871288);
    nvgQuadTo(vg, 7.5926352,8.7871751, 7.8743714,7.7357213);
    nvgQuadTo(vg, 8.1601517,6.6691748, 8.9693819,6.1041558);
    nvgQuadTo(vg, 9.7799602,5.5341058, 10.665395,5.7713573);
    nvgQuadTo(vg, 11.515614,5.9991727, 11.89242,6.8065002);
    nvgLineTo(vg, 12.306262,5.2620202);
    nvgLineTo(vg, 11.546599,5.0584691);
    nvgLineTo(vg, 11.847208,3.936583);
    nvgLineTo(vg, 14.161412,4.5566722);
    nvgLineTo(vg, 12.586925,10.43274);
    nvgClosePath(vg);

    nvgPathWinding(vg, NVGsolidity::NVG_HOLE);
    nvgMoveTo(vg, 11.402005,8.6971241);
    nvgQuadTo(vg, 11.58938,7.9978318, 11.36706,7.6201287);
    nvgQuadTo(vg, 11.149771,7.2437736, 10.767424,7.1413241);
    nvgQuadTo(vg, 10.309614,7.0186543, 9.962361,7.3084457);
    nvgQuadTo(vg, 9.6214865,7.5945543, 9.4664642,8.1731054);
    nvgQuadTo(vg, 9.31953,8.7214713, 9.4649416,9.1648403);
    nvgQuadTo(vg, 9.612,9.603, 10.08,9.729);
    nvgQuadTo(vg, 10.467,9.833, 10.84,9.609);
    nvgQuadTo(vg, 11.219,9.382, 11.402,8.697);
    nvgClosePath(vg);
    nvgFill(vg);

    nvgRestore(vg);
}

void heart_path(NVGcontext* vg)
{
    nvgBeginPath(vg);
    nvgMoveTo(vg, 50.f, 14.5f);
    nvgQuadTo(vg, 51.88014,11.675824, 54.28907,9.0659343);
    nvgQuadTo(vg, 56.698,6.456044, 59.518215,4.4299451 );
    nvgQuadTo(vg, 62.338425,2.4038461, 65.481785,1.2019231 );
    nvgQuadTo(vg, 68.625145,0, 71.97415,0 );
    nvgQuadTo(vg, 77.790836,0, 82.873093,2.5755494 );
    nvgQuadTo(vg, 87.955343,5.151099, 91.774386,9.6153843 );
    nvgQuadTo(vg, 95.593421,14.079671, 97.796707,20.020604 );
    nvgQuadTo(vg, 100,25.961539, 100,32.760989 );
    nvgQuadTo(vg, 100,35.302198, 99.383071,39.010989);
    nvgQuadTo(vg, 98.766143,42.71978, 96.915386,47.458791 );
    nvgQuadTo(vg, 95.064621,52.1978, 91.656871,57.967035 );
    nvgQuadTo(vg, 88.249114,63.736265, 82.667443,70.32967 );
    nvgQuadTo(vg, 77.085779,76.923071, 69.00705,84.375 );
    nvgQuadTo(vg, 60.92832,91.826921, 49.706228,100 );
    nvgQuadTo(vg, 38.366627,91.620879, 30.287896,84.203293 );
    nvgQuadTo(vg, 22.209166,76.785714, 16.686251,70.26099 );
    nvgQuadTo(vg, 11.163337,63.736265, 7.8437136,58.138735 );
    nvgQuadTo(vg, 4.5240893,52.54121, 2.7908343,47.870879 );
    nvgQuadTo(vg, 1.0575793,43.200549, 0.52878964,39.423077 );
    nvgQuadTo(vg, 0,35.645604, 0,32.760989 );
    nvgQuadTo(vg, 0,25.961539, 2.2326674,20.020604 );
    nvgQuadTo(vg, 4.4653349,14.079671, 8.2843714,9.6153843 );
    nvgQuadTo(vg, 12.103408,5.151099, 17.244419,2.5755494 );
    nvgQuadTo(vg, 22.385429,0, 28.202115,0);
    nvgQuadTo(vg, 31.492362,0, 34.606346,1.2019231 );
    nvgQuadTo(vg, 37.720329,2.4038461, 40.511164,4.4299451 );
    nvgQuadTo(vg, 43.301997,6.456044, 45.710929,9.0659343 );
    nvgQuadTo(vg, 48.119859,11.675824, 50,14.491758);
    nvgClosePath(vg);
}

void FillHeart(NVGcontext* vg, float x, float y, float square, NVGcolor fill)
{
    nvgSave(vg);
    nvgTranslate(vg, x, y);
    float scale = square / 100.f;
    nvgScale(vg, scale, scale);
    nvgFillColor(vg, fill);
    heart_path(vg);
    nvgFill(vg);
    nvgRestore(vg);
}

void StrokeHeart(NVGcontext* vg, float x, float y, float square, NVGcolor stroke, float stroke_width)
{
    nvgSave(vg);
    nvgTranslate(vg, x, y);
    float scale = square / 100.f;
    nvgScale(vg, scale, scale);
    nvgStrokeColor(vg, stroke);
    nvgStrokeWidth(vg, stroke_width * 10.f);
    heart_path(vg);
    nvgStroke(vg);
    nvgRestore(vg);
}

#if defined IMPLEMENT_DRAW_PERSON
void person_path(NVGcontext* vg)
{
    nvgBeginPath(vg);
    nvgMoveTo(vg, 36.050185f,11.378003f);
    nvgBezierTo(vg, 36.050185f,12.93721f, 35.755198f,14.412137f, 35.165229f,15.802781f);
    nvgBezierTo(vg, 34.575261f,17.193426f, 33.764047f,18.404973f, 32.731601f,19.437421f);
    nvgBezierTo(vg, 31.699156f,20.469869f, 30.498144f,21.281079f, 29.128568f,21.871049f);
    nvgBezierTo(vg, 27.758991f,22.46102f, 26.294604f,22.756005f, 24.735394f,22.756005f);
    nvgBezierTo(vg, 23.176185f,22.756005f, 21.70126f,22.46102f, 20.310616f,21.871049f);
    nvgBezierTo(vg, 18.91997f,21.281079f, 17.708422f,20.469869f, 16.675975f,19.437421f);
    nvgBezierTo(vg, 15.643529f,18.404973f, 14.832315f,17.193426f, 14.242347f,15.802781f);
    nvgBezierTo(vg, 13.652379f,14.412137f, 13.357391f,12.93721f, 13.357391f,11.378003f);
    nvgBezierTo(vg, 13.357391f,9.8187946f, 13.652379f,8.3438685f, 14.242347f,6.9532238f);
    nvgBezierTo(vg, 14.832315f,5.562579f, 15.643529f,4.3510322f, 16.675975f,3.3185841f);
    nvgBezierTo(vg, 17.708422f,2.2861359f, 18.91997f,1.474926f, 20.310616f,0.88495575f);
    nvgBezierTo(vg, 21.70126f,0.29498546f, 23.176185f,0.f, 24.735394f,0.f);
    nvgBezierTo(vg, 26.294604f,0.f, 27.758991f,0.29498546f, 29.128568f,0.88495575f);
    nvgBezierTo(vg, 30.498144f,1.474926f, 31.699156f,2.2861359f, 32.731601f,3.3185841f);
    nvgBezierTo(vg, 33.764047f,4.3510322f, 34.575261f,5.562579f, 35.165229f,6.9532238f);
    nvgBezierTo(vg, 35.755198f,8.3438685f, 36.050185f,9.8187946f, 36.050185f,11.378003f);
    nvgClosePath(vg);
    nvgMoveTo(vg, 37.112018f,27.876827f);
    nvgBezierTo(vg, 38.50265f,27.872527f, 39.809028f,28.140207f, 41.031108f,28.666966f);
    nvgBezierTo(vg, 38.334098f,28.403586f, 39.408687f,29.130514f, 40.335785f,30.057611f);
    nvgBezierTo(vg, 41.262884f,30.984707f, 41.989812f,32.059297f, 42.516569f,33.281378f);
    nvgBezierTo(vg, 43.043327f,34.50346f, 43.139635f,35.819896f, 43.306708f,37.200468f);
    nvgLineTo(vg, 46.037543f,59.766119f);
    nvgBezierTo(vg, 46.108423f,60.351814f,	45.932189f,60.924987f,	45.721487f,61.472819f);
    nvgBezierTo(vg, 45.510785f,62.020651f,	45.215798f,62.515803f,	44.836531f,62.958281f);
    nvgBezierTo(vg, 44.457265f,63.400759f,	44.004249f,63.758957f,	43.477492f,64.03287f);
    nvgBezierTo(vg, 42.950735f,64.306783f,	42.371297f,64.443742f,	41.739186f,64.443742f);
    nvgBezierTo(vg, 40.390684f,64.443742f,	39.263415f,64.022333f,	38.357391f,63.17952f);
    nvgBezierTo(vg, 37.451367f,62.336707f,	37.198091f,61.226169f,	36.998352f,59.892541f);
    nvgLineTo(vg, 37.669964f,38.999822f);
    nvgBezierTo(vg, 37.582574f,38.41636f, 37.438188f,37.893628f, 36.974642f,37.45115f);
    nvgBezierTo(vg, 36.511095f,37.008672f, 35.984332f,36.787433f, 35.394363f,36.787433f);
    nvgBezierTo(vg, 34.804395f,36.787433f, 34.277631f,37.008672f, 33.814085f,37.45115f);
    nvgBezierTo(vg, 33.350539f,37.893628f, 33.024161f,38.417486f, 33.118763f,38.999822f);
    nvgLineTo(vg, 36.87009f,62.091568f);
    nvgLineTo(vg, 36.366241f,95.448799f);
    nvgBezierTo(vg, 36.345871f,96.797149f, 35.902695f,97.892965f, 34.975596f,98.735777);
    nvgBezierTo(vg, 34.048497f,99.57859f, 32.910697f,100.f, 31.562196f,100.f);
    nvgBezierTo(vg, 30.972228f,100.f, 30.413858f,99.86304f, 29.887101f,99.589128f);
    nvgBezierTo(vg, 29.360344f,99.315215f, 28.907329f,98.957016f, 28.528062f,98.514539f);
    nvgBezierTo(vg, 28.148795f,98.072061f, 27.843277f,97.576909f, 27.611501f,97.029077f);
    nvgBezierTo(vg, 27.379724f,96.481245f, 27.263839f,95.912345f, 27.263839f,95.322377f);
    nvgLineTo(vg, 27.263839f,68.836915f);
    nvgBezierTo(vg, 27.263839f,68.204804f, 27.042601f,67.667509f, 26.600123f,67.225032f);
    nvgBezierTo(vg, 26.157645f,66.782554f, 25.62035f,66.561315f, 24.988239f,66.561315f);
    nvgBezierTo(vg, 24.356128f,66.561315f, 23.818833f,66.782554f, 23.376355f,67.225032f);
    nvgBezierTo(vg, 22.933877f,67.667509f, 22.712638f,68.204804f, 22.712638f,68.836915f);
    nvgLineTo(vg, 22.712638f,95.448799f);
    nvgBezierTo(vg, 22.712638f,96.797301f, 22.259623f,97.892965f, 21.353599f,98.735777f);
    nvgBezierTo(vg, 20.447575f,99.57859f, 19.320306f,100.f, 17.971803f,100.f);
    nvgBezierTo(vg, 17.339692f,100.f, 16.760255f,99.86304f, 16.233497f,99.589128f);
    nvgBezierTo(vg, 15.70674f,99.315215f, 15.253725f,98.957016f, 14.874458f,98.514539f);
    nvgBezierTo(vg, 14.495192f,98.072061f, 14.189673f,97.576909f, 13.957897f,97.029077f);
    nvgBezierTo(vg, 13.726121f,96.481245f, 13.623924f,95.912187f, 13.610236f,95.322377f);
    nvgLineTo(vg, 12.849224f,62.529671f);
    nvgLineTo(vg, 16.931525f,39.284453f);
    nvgBezierTo(vg, 17.040863f,38.66187f, 16.710286f,38.115047f, 16.267808f,37.67257f);
    nvgBezierTo(vg, 15.825331f,37.230092f, 15.288036f,37.008853f, 14.655925f,37.008853f);
    nvgBezierTo(vg, 14.06595f,37.008853f, 13.539193f,37.240626f, 13.075647f,37.704175f);
    nvgBezierTo(vg, 12.612094f,38.167723f, 12.470812f,38.701463f, 12.380324f,39.284453f);
    nvgLineTo(vg, 9.0590348f,60.68268f);
    nvgBezierTo(vg, 8.8522061f,62.015227f, 8.6060196f,63.126846f, 7.6999956f,63.969659f);
    nvgBezierTo(vg, 6.7939716f,64.812472f, 5.6667026f,65.233881f, 4.3182007f,65.233881f);
    nvgBezierTo(vg, 3.6860894f,65.233881f, 3.106652f,65.096922f, 2.5798948f,64.823009f);
    nvgBezierTo(vg, 2.0531375f,64.549096f, 1.6001223f,64.190898f, 1.2208556f,63.74842f);
    nvgBezierTo(vg, 0.84158881f,63.305942f, 0.54660144f,62.81079f, 0.3358998f,62.262958f);
    nvgBezierTo(vg, 0.12519816f,61.715126f, -0.06258629f,61.140442f, 0.01984417f,60.556258f);
    nvgLineTo(vg, 3.1935168f,38.064413f);
    nvgBezierTo(vg, 3.3878164f,36.687409f, 3.4568985f,35.367405f, 3.9836558f,34.145324f);
    nvgBezierTo(vg, 4.5104131f,32.923242f, 5.237341f,31.848653f, 6.1644396f,30.921556f);
    nvgBezierTo(vg, 7.0915379f,29.99446f, 8.1661271f,29.267532f, 9.3882067f,28.740772f);
    nvgBezierTo(vg, 10.610286f,28.214013f, 11.916658f,27.954945f, 13.307296f,27.950633f);
    nvgClosePath(vg);
}

void StrokePerson(NVGcontext* vg, float x, float y, float square, NVGcolor stroke, float stroke_width)
{
    nvgSave(vg);
    nvgTranslate(vg, x, y);
    float scale = square / 100.f;
    nvgScale(vg, scale, scale);
    nvgStrokeColor(vg, stroke);
    nvgStrokeWidth(vg, stroke_width * 10.f);
    person_path(vg);
    nvgStroke(vg);
    nvgRestore(vg);
}

void FillPerson(NVGcontext* vg, float x, float y, float square, NVGcolor fill)
{
    nvgSave(vg);
    nvgTranslate(vg, x, y);
    float scale = square / 100.f;
    nvgScale(vg, scale, scale);
    nvgFillColor(vg, fill);
    person_path(vg);
    nvgFill(vg);
    nvgRestore(vg);
}
#endif

void DrawKnobTrack(NVGcontext* vg, const Knob * w, float track_radius, float track_width, const NVGcolor& color)
{
    nvgBeginPath(vg);
    nvgArc(vg, w->box.size.x *.5f, w->box.size.y *.5f, track_radius, w->minAngle - M_PI/2.f, w->maxAngle - M_PI/2.f, NVG_CW);
    nvgStrokeWidth(vg, track_width);
    nvgStrokeColor(vg, color);
    nvgLineCap(vg, NVG_ROUND);
    nvgStroke(vg);
}

}