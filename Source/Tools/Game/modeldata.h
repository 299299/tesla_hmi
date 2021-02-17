#pragma once

#include <Urho3D/Urho3DAll.h>

#define MODEL_PATH_DISTANCE 60
#define POLYFIT_DEGREE 4
#define SPEED_PERCENTILES 10
#define DESIRE_PRED_SIZE 32
#define OTHER_META_SIZE 4

#define TRAJECTORY_SIZE 33
#define MIN_DRAW_DISTANCE 10.0
#define MAX_DRAW_DISTANCE 100.0

struct LeadData
{
    float dRel = MAX_DRAW_DISTANCE;
    float yRel;
    float vRel;
    float aRel;
    float vLead;
    int status = 0;
};

struct LineData
{
    float prob = 0.0F;
    float std = 1.0F;
};

struct ModelData
{
    float max_distance;
    LeadData leads[2];
    LineData lanes[4];
    LineData edges[2];
};
