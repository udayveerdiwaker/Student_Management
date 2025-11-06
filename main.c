// main.c
#include <stdio.h>
#include <stdlib.h>
#include "student.h"

int main() {
    int choice;
    do {
        header();
        showMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1: addStudent(); break;
            case 2: viewStudents(); break;
            case 3: searchStudent(); break;
            case 4: deleteStudent(); break;
            case 5: 
                printf("\nExiting... Goodbye!\n");
                exit(0);
            default: 
                printf("\nInvalid choice! Try again.\n");
        }
        printf("\nPress Enter to continue...");
        getchar(); getchar();
    } while (1);

    return 0;
}
