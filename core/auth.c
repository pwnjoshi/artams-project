#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "auth.h"
#include "student_db.h"

#define TEACHER_FILE "data/teachers.txt"
#define STUDENT_ACCOUNT_FILE "data/student_accounts.txt"

// Global variable to store currently logged-in student's roll number
static int currentLoggedInStudent = -1;

// TEACHER LOGIN

int teacherLogin() {
    char username[50], password[50], stored_user[50], stored_pass[50];
    FILE *file = fopen(TEACHER_FILE, "r");
    if (!file) {
        printf("Error: Could not open %s\n", TEACHER_FILE);
        return 0;
    }

    printf("\n=== Teacher Login ===\n");
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);

    int success = 0;
    while (fscanf(file, "%49[^,],%49[^\n]\n", stored_user, stored_pass) == 2) {
        if (strcmp(username, stored_user) == 0 && strcmp(password, stored_pass) == 0) {
            success = 1;
            break;
        }
    }

    fclose(file);

    if (success) {
        printf("Login successful! Welcome, %s\n", username);
        return 1;
    } else {
        printf("Invalid credentials. Access denied.\n");
        return 0;
    }
}

// STUDENT LOGIN
int studentLogin() {
    int rollno;
    char password[50];
    char stored_pass[50];
    int stored_roll;
    Student *s;

    FILE *file = fopen(STUDENT_ACCOUNT_FILE, "r");
    if (!file) {
        printf("Error: Could not open %s\n", STUDENT_ACCOUNT_FILE);
        return 0;
    }

    printf("\n=== Student Login ===\n");
    printf("Enter Roll Number: ");
    scanf("%d", &rollno);
    printf("Enter Password: ");
    scanf("%s", password);

    int authenticated = 0;
    while (fscanf(file, "%d,%49[^\n]\n", &stored_roll, stored_pass) == 2) {
        if (rollno == stored_roll && strcmp(password, stored_pass) == 0) {
            authenticated = 1;
            break;
        }
    }
    fclose(file);

    if (!authenticated) {
        printf("❌ Invalid roll number or password.\n");
        return 0;
    }

    // Also verify rollno exists in student database
    s = searchStudent(rollno);
    if (!s) {
        printf("Student record not found in database.\n");
        return 0;
    }

    printf("Welcome %s!\n", s->name);
    
    // Store the logged-in student's roll number
    currentLoggedInStudent = rollno;
    
    return 1;
}

// Function to get current logged-in student's roll number
int getCurrentLoggedInStudent() {
    return currentLoggedInStudent;
}

// Function to clear logged-in student (for logout)
void clearLoggedInStudent() {
    currentLoggedInStudent = -1;
}
