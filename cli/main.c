#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/student_db.h"
#include "../core/token_manager.h"
#include "../core/log_manager.h"
#include "../core/location_validator.h"
#include "../core/utils.h"
#include "../core/auth.h"  

void teacherMenu();
void studentMenu();
void mainMenu();

int main() {
    inithashTable();
    loadstudents("data/students.txt");
    
    initAttendanceLog();
    loadAttendanceFromFile("data/attendance_log.txt");
    
    loadClassroomLocation();
        
    printf("=== ARTAMS - Hamara Attendance Management System ===\n");

    while (1) {
        mainMenu();
        
        if (confirmExit()) {
            freeAttendanceList();
            break;
        }
    }
    
    return 0;
}

void mainMenu() {
    int mode;

    printf("\n Select mode:\n"
        "    (1) Teacher Login\n"
        "    (2) Student Login\n"
        "    (3) Exit\n");
    
    printf("> ");  
    fflush(stdout);
    scanf("%d", &mode);

    if (mode == 1) {
        if (teacherLogin()) {   // Teacher authentication
            teacherMenu();
        } else {
            printf("Returning to main menu...\n");
        }
    }
    else if (mode == 2) {
        if (studentLogin()) {   // Student authentication
            studentMenu();
            clearLoggedInStudent(); // Clear session when returning to main menu
        } else {
            printf("Returning to main menu...\n");
        }
    }
    else if (mode == 3) {
        return;
    }
    else {
        printf("Invalid choice!\n");
    }
}

void teacherMenu() {
    int choice;
    double lat, lon;
    int rollNo;
    char name[50];
    const char *student_file = "data/students.txt";

    while (1) {
        printf("\n----Teacher Menu----\n");
        printf("1. Start Attendance Session\n");
        printf("2. View Attendance Log\n");
        printf("3. Add Student\n");
        printf("4. Remove Student\n");
        printf("5. View Students\n");
        printf("6. Return to Main Menu\n");
        printf("> ");
        fflush(stdout);
        scanf("%d", &choice);

        if (choice == 1) {
            printf("Enter classroom latitude: ");
            scanf("%lf", &lat);
            printf("Enter classroom longitude: ");
            scanf("%lf", &lon);

            setClassroomLocation(lat, lon);
            printf("Classroom location set to (%.6f, %.6f) successfully\n", lat, lon);
            
            displayToken(30);
            waitForUserInput(); 
        }
        else if (choice == 2) {
            printf("\n--- Attendance Log ---\n");
            showAttendance(); 
            waitForUserInput(); 
        } 
        else if (choice == 3) { // Add student
            printf("\nEnter Roll Number: ");
            scanf("%d", &rollNo);
            getchar(); // clear newline
            printf("Enter Student Name: ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = 0;

            add_student_teacher(student_file, rollNo, name);
            waitForUserInput();
        }
        else if (choice == 4) { // Remove student
            printf("\nEnter Roll Number to Remove: ");
            scanf("%d", &rollNo);
            remove_student_teacher(student_file, rollNo);
            waitForUserInput();
        }
        else if (choice == 5) { // View students
            displaystudents();
            waitForUserInput();
        }
        else if (choice == 6) {
            break;
        } 
        else {
            printf("Invalid choice!\n");
        }
    }
}

void studentMenu() {
    int rollNo;
    char token[16];
    double lat, lon;
    double classroom_lat, classroom_lon;
    Student *s;

    // Get the logged-in student's roll number
    rollNo = getCurrentLoggedInStudent();
    
    if (rollNo == -1) {
        printf("Error: No student logged in!\n");
        return;
    }

    // Get student details
    s = searchStudent(rollNo);
    if (!s) {
        printf("Error: Student record not found!\n");
        return;
    }

    printf("\n=== Student Attendance Marking ===\n");
    printf("Logged in as: %s (Roll No: %d)\n", s->name, rollNo);
    
    getCurrentClassroomLocation(&classroom_lat, &classroom_lon);

    // Token validation loop
    while (1) {
        printf("\nEnter Token: ");
        scanf("%s", token);

        if (!validateToken("data/sessions.txt", token)) {
            char choice;
            printf("Invalid or expired token!\n");
            printf("Do you want to try again? (y/n): ");
            fflush(stdout);
            clearInputBuffer();
            scanf(" %c", &choice);
            if (choice == 'y' || choice == 'Y') {
                continue;
            } else {
                printf("Returning to main menu...\n");
                return;
            }
        } else {
            printf("Token validated successfully!\n");
            break;
        }
    }
    
    // Location validation loop
    while (1) {
        printf("\nEnter Location (latitude longitude): ");
        scanf("%lf %lf", &lat, &lon);

        if (!validateLocation(lat, lon)) {
            char choice;
            double distance = getDistanceFromClassroom(lat, lon);
            printf("\nERROR: Location outside classroom range!\n");
            printf("Your coordinates: (%.6f, %.6f)\n", lat, lon);
            printf("Classroom location: (%.6f, %.6f)\n", classroom_lat, classroom_lon);
            printf("Distance: %.2f km (max allowed: 0.1 km or 100 meters)\n", distance);
            printf("\nDo you want to try again? (y/n): ");
            fflush(stdout);
            clearInputBuffer();
            scanf(" %c", &choice);
            if (choice == 'y' || choice == 'Y') {
                continue;
            } else {
                printf("Returning to main menu...\n");
                return;
            }
        } else {
            printf("\nLocation validated successfully!\n");
            break;
        }
    }

    // Mark attendance using the logged-in student's roll number
    markAttendance(rollNo, lat, lon, "Present");

    printf("Attendance marked successfully for %s!\n", s->name);
    printf("Your location: (%.6f, %.6f)\n", lat, lon);
}
