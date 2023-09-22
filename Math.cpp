#pragma once
#include <math.h>

namespace math
{
    const float INCHES_TO_METER_RATE = 39.3701;

    double distanceToMeters(float distance)
    {
        return distance / INCHES_TO_METER_RATE;
    }

    double calculateDistance(float x1, float y1, float z1, float x2, float y2, float z2)
    {
        float dx = (x1 - x2);
        float dy = (y1 - y2);
        float dz = (z1 - z2);
        float distance = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
        return distance;
    }

    double calculateDistanceInMeters(float x1, float y1, float z1, float x2, float y2, float z2)
    {
        return distanceToMeters(calculateDistance(x1, y1, z1, x2, y2, z2));
    }

    double calculateDistance2D(float x1, float y1, float x2, float y2)
    {
        float dx = (x1 - x2);
        float dy = (y1 - y2);
        float distance = sqrt(pow(dx, 2) + pow(dy, 2));
        return distance;
    }

    double calculateDesiredYaw(
        double localPlayerLocationX,
        double localPlayerLocationY,
        double enemyPlayerLocationX,
        double enemyPlayerLocationY)
    {
        const double locationDeltaX = enemyPlayerLocationX - localPlayerLocationX;
        const double locationDeltaY = enemyPlayerLocationY - localPlayerLocationY;
        const double yawInRadians = atan2(locationDeltaY, locationDeltaX);
        const double yawInDegrees = yawInRadians * (180 / M_PI);
        return yawInDegrees;
    }

    double calculateDesiredPitch(
        double localPlayerLocationX,
        double localPlayerLocationY,
        double localPlayerLocationZ,
        double enemyPlayerLocationX,
        double enemyPlayerLocationY,
        double enemyPlayerLocationZ)
    {
        const double locationDeltaZ = (enemyPlayerLocationZ -
            localPlayerLocationZ - INCHES_TO_METER_RATE * 0.5);
        const double distanceBetweenPlayers = math::calculateDistance2D(enemyPlayerLocationX, enemyPlayerLocationY, localPlayerLocationX, localPlayerLocationY);
        const double pitchInRadians = atan2(-locationDeltaZ, distanceBetweenPlayers);
        const double pitchInDegrees = pitchInRadians * (180 / M_PI);
        return pitchInDegrees;
    }

    double calculatePitchAngleDelta(double oldAngle, double newAngle)
    {
        double wayA = newAngle - oldAngle;
        return wayA;
    }

    double calculateAngleDelta(double oldAngle, double newAngle)
    {
        double wayA = newAngle - oldAngle;
        double wayB = 360 - abs(wayA);
        if (wayA > 0 && wayB > 0)
            wayB *= -1;
        if (abs(wayA) < abs(wayB))
            return wayA;
        return wayB;
    }

    void convertPointByYaw(double yaw, double x, double y, double& xOut, double& yOut) {
        double rad = (90 - yaw) / 180 * M_PI;
        double cos = std::cos(rad);
        double sin = std::sin(rad);
        xOut = x * cos - y * sin;
        yOut = y * cos + x * sin;
    }
}
