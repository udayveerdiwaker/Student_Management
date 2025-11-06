// student.h
#ifndef STUDENT_H
#define STUDENT_H

typedef struct {
    int id;
    char name[50];
    int age;
    float marks;
} Student;

// Function declarations
void addStudent();
void viewStudents();
void searchStudent();
void deleteStudent();
void showMenu();
void header();

#endif
