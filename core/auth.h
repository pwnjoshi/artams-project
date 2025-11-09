#ifndef AUTH_H
#define AUTH_H

int teacherLogin();
int studentLogin();
int getCurrentLoggedInStudent(); // Returns roll number of logged-in student
void clearLoggedInStudent();     // Clear login session

#endif
