// location_validator.c  (patched)

#include "location_validator.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static double class_lat = 0.0;
static double class_lon = 0.0;
static int classroom_set = 0;
static double custom_radius_km = 10.0;  // 

#define DEFAULT_RADIUS_KM 10.0  // Default h ye
#define PI 3.14159265358979323846
#define EARTH_RADIUS_KM 6372.0
#define CLASSROOM_FILE "data/classroom.txt"

static double toRadians(double degree) {
    return degree * (PI / 180.0);
}

static double haversineDistance(double lat1, double lon1, double lat2, double lon2) {
    lat1 = toRadians(lat1);
    lon1 = toRadians(lon1);
    lat2 = toRadians(lat2);
    lon2 = toRadians(lon2);

    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;

    double a = sin(dLat / 2) * sin(dLat / 2) + cos(lat1) * cos(lat2) *sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    
    return EARTH_RADIUS_KM * c;
}

void setClassroomLocation(double lat, double lon) {
    class_lat = lat;
    class_lon = lon;
    classroom_set = 1;
    
    FILE* file = fopen(CLASSROOM_FILE, "w");
    if (file) {
        fprintf(file, "%.6f %.6f %.2f\n", lat, lon, custom_radius_km);
        fclose(file);
    }
}

void setValidationRadius(double radius_km) {
    if (radius_km > 0 && radius_km <= 50.0) {  // ye new condition h
        custom_radius_km = radius_km;
    } else {
        custom_radius_km = DEFAULT_RADIUS_KM;
    }
}

// modify stuff
double getValidationRadius() {
    return custom_radius_km;
}

void loadClassroomLocation() {
    FILE* file = fopen(CLASSROOM_FILE, "r");
    if (file) {
        double radius;
        int items = fscanf(file, "%lf %lf %lf", &class_lat, &class_lon, &radius);
        if (items >= 2) {
            classroom_set = 1;
            if (items == 3) {
                custom_radius_km = radius;  // Load custom radius if available
            } else {
                custom_radius_km = DEFAULT_RADIUS_KM;
            }
        } else {
            class_lat = 0.0;
            class_lon = 0.0;
            classroom_set = 0;
            custom_radius_km = DEFAULT_RADIUS_KM;
        }
        fclose(file);
    } else {
        classroom_set = 0;
        custom_radius_km = DEFAULT_RADIUS_KM;
    }
}

void getCurrentClassroomLocation(double* lat, double* lon) {
    if (!classroom_set) {
        loadClassroomLocation();
    }

    *lat = class_lat;
    *lon = class_lon;
}

int validateLocation(double lat, double lon) {
    if (!classroom_set) {
        loadClassroomLocation();
    }

    if (!classroom_set) {
        printf("ERROR: Classroom location not configured. Please ask teacher to set classroom location first.\n");
        return 0;
    }
    
    double distance = haversineDistance(class_lat, class_lon, lat, lon);
    return (distance <= custom_radius_km);  // Use dynamic radius
}

// modify 2 h: Get validation status with detailed info
int validateLocationDetailed(double lat, double lon, char* status_msg, int msg_size) {
    if (!classroom_set) {
        loadClassroomLocation();
    }

    if (!classroom_set) {
        snprintf(status_msg, msg_size, "ERROR: Classroom not configured");
        return 0;
    }
    
    double distance = haversineDistance(class_lat, class_lon, lat, lon);
    double distance_meters = distance * 1000;
    double radius_meters = custom_radius_km * 1000;
    
    if (distance <= custom_radius_km) {
        snprintf(status_msg, msg_size, 
                "VALID: %.0fm from classroom (within %.0fm radius)", 
                distance_meters, radius_meters);
        return 1;
    } else {
        snprintf(status_msg, msg_size, 
                "INVALID: %.0fm from classroom (exceeds %.0fm radius by %.0fm)", 
                distance_meters, radius_meters, distance_meters - radius_meters);
        return 0;
    }
}

// ENHANCEMENT 3: Get location accuracy category
const char* getLocationAccuracy(double distance_km) {
    double distance_m = distance_km * 1000;
    
    if (distance_m <= 10) {
        return "EXCELLENT (±10m)";
    } else if (distance_m <= 50) {
        return "VERY GOOD (±50m)";
    } else if (distance_m <= 100) {
        return "GOOD (±100m)";
    } else if (distance_m <= 500) {
        return "MODERATE (±500m)";
    } else if (distance_m <= 1000) {
        return "LOW (±1km)";
    } else {
        return "VERY LOW (>1km)";
    }
}

double getDistanceFromClassroom(double lat, double lon) {
    // Ensure classroom location is loaded from file
    if (!classroom_set) {
        loadClassroomLocation();
    }
    if (!classroom_set) {
        // Indicate invalid krega ye
        return -1.0;
    }
    return haversineDistance(class_lat, class_lon, lat, lon);
}
