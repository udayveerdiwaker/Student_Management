// frontend.c
#include <stdio.h>
#include <stdlib.h>
#include "student.h"

void header() {
    system("cls"); // For Linux, use system("clear");
    printf("\n=========================================\n");
    printf("        STUDENT MANAGEMENT SYSTEM        \n");
    printf("=========================================\n\n");
}

void showMenu() {
    printf("1. Add Student\n");
    printf("2. View Students\n");
    printf("3. Search Student\n");
    printf("4. Delete Student\n");
    printf("5. Exit\n");
    printf("-----------------------------------------\n");
    printf("Enter your choice: ");
}
