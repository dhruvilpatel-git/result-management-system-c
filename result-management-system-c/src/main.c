#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CREDENTIALS 100
#define MAX_STUDENTS 100
#define MAX_NAME_LENGTH 50

// Files are resolved relative to the working directory.
// For convenience, we default to the `data/` folder.
#define FILENAME_CREDENTIALS "data/credentials.txt"
#define FILENAME_STUDENTS "data/student_data.txt"

typedef struct {
    char username[MAX_NAME_LENGTH];
    char password[MAX_NAME_LENGTH];
    char role;  // 'T' for Teacher, 'S' for Student
} User;

typedef struct {
    char studentUsername[MAX_NAME_LENGTH];  // must match a Student user's username
    int marks;                              // single overall mark for simplicity
} Student;

// ---------- Utility ----------
static void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == '\n') s[n - 1] = '\0';
}

static void read_line(const char *prompt, char *buf, size_t bufSize) {
    if (prompt) {
        printf("%s", prompt);
        fflush(stdout);
    }
    if (!fgets(buf, (int)bufSize, stdin)) {
        // EOF or error
        buf[0] = '\0';
        return;
    }
    trim_newline(buf);
}

static int read_int(const char *prompt, int min, int max) {
    char line[128];
    while (1) {
        read_line(prompt, line, sizeof(line));
        if (line[0] == '\0') continue;
        char *end = NULL;
        long v = strtol(line, &end, 10);
        if (end == line || *end != '\0') {
            printf("Please enter a valid number.\n");
            continue;
        }
        if (v < min || v > max) {
            printf("Please enter a number between %d and %d.\n", min, max);
            continue;
        }
        return (int)v;
    }
}

static int find_student_index(Student *students, int numStudents, const char *studentUsername) {
    for (int i = 0; i < numStudents; i++) {
        if (strcmp(students[i].studentUsername, studentUsername) == 0) return i;
    }
    return -1;
}

// ---------- I/O ----------
static void loadCredentials(User *users, int *numUsers) {
    FILE *file = fopen(FILENAME_CREDENTIALS, "r");
    if (file == NULL) {
        perror("Error opening credentials file");
        printf("Expected path: %s\n", FILENAME_CREDENTIALS);
        exit(1);
    }

    *numUsers = 0;
    while (*numUsers < MAX_CREDENTIALS) {
        User u;
        if (fscanf(file, "%49s %49s %c", u.username, u.password, &u.role) != 3) break;
        users[*numUsers] = u;
        (*numUsers)++;
    }
    fclose(file);
}

static void loadStudentData(Student *students, int *numStudents) {
    FILE *file = fopen(FILENAME_STUDENTS, "r");
    if (file == NULL) {
        // If missing, start empty (more user-friendly than exiting)
        *numStudents = 0;
        return;
    }

    *numStudents = 0;
    while (*numStudents < MAX_STUDENTS) {
        Student s;
        if (fscanf(file, "%49s %d", s.studentUsername, &s.marks) != 2) break;
        students[*numStudents] = s;
        (*numStudents)++;
    }
    fclose(file);
}

static void saveStudentData(Student *students, int numStudents) {
    FILE *file = fopen(FILENAME_STUDENTS, "w");
    if (file == NULL) {
        perror("Error opening student data file for writing");
        printf("Expected path: %s\n", FILENAME_STUDENTS);
        exit(1);
    }

    for (int i = 0; i < numStudents; i++) {
        fprintf(file, "%s %d\n", students[i].studentUsername, students[i].marks);
    }
    fclose(file);
}

// ---------- Auth ----------
static int login(User *users, int numUsers, User *currentUser) {
    char username[MAX_NAME_LENGTH];
    char password[MAX_NAME_LENGTH];

    // Consume any leftover newline from earlier input
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* no-op */ }

    read_line("Enter username: ", username, sizeof(username));
    read_line("Enter password: ", password, sizeof(password));

    for (int i = 0; i < numUsers; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0) {
            *currentUser = users[i];
            return 1;
        }
    }
    return 0;
}

// ---------- Features ----------
static void uploadGrades(Student *students, int *numStudents) {
    if (*numStudents >= MAX_STUDENTS) {
        printf("Student capacity reached.\n");
        return;
    }

    char studentUsername[MAX_NAME_LENGTH];
    read_line("Student username to add/update: ", studentUsername, sizeof(studentUsername));

    int marks = read_int("Enter marks (0-100): ", 0, 100);

    int idx = find_student_index(students, *numStudents, studentUsername);
    if (idx >= 0) {
        students[idx].marks = marks;
        printf("Updated %s => %d\n", studentUsername, marks);
    } else {
        Student s;
        strncpy(s.studentUsername, studentUsername, MAX_NAME_LENGTH);
        s.studentUsername[MAX_NAME_LENGTH - 1] = '\0';
        s.marks = marks;
        students[*numStudents] = s;
        (*numStudents)++;
        printf("Added %s => %d\n", studentUsername, marks);
    }

    saveStudentData(students, *numStudents);
}

static void editGrades(Student *students, int numStudents) {
    if (numStudents == 0) {
        printf("No student records found.\n");
        return;
    }

    char studentUsername[MAX_NAME_LENGTH];
    read_line("Student username to edit: ", studentUsername, sizeof(studentUsername));

    int idx = find_student_index(students, numStudents, studentUsername);
    if (idx < 0) {
        printf("Student not found.\n");
        return;
    }

    int marks = read_int("Enter new marks (0-100): ", 0, 100);
    students[idx].marks = marks;

    saveStudentData(students, numStudents);
    printf("Updated %s => %d\n", studentUsername, marks);
}

static int cmp_by_username_asc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    return strcmp(sa->studentUsername, sb->studentUsername);
}

static int cmp_by_marks_desc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    return sb->marks - sa->marks;
}

static void sortGrades(Student *students, int numStudents) {
    if (numStudents == 0) {
        printf("No student records found.\n");
        return;
    }

    printf("\nSort By:\n");
    printf("1. Student Username (A-Z)\n");
    printf("2. Marks (High to Low)\n");
    int choice = read_int("Choose: ", 1, 2);

    if (choice == 1) qsort(students, (size_t)numStudents, sizeof(Student), cmp_by_username_asc);
    else qsort(students, (size_t)numStudents, sizeof(Student), cmp_by_marks_desc);

    saveStudentData(students, numStudents);
    printf("Sorted successfully.\n");
}

static void viewGrades_all(Student *students, int numStudents) {
    if (numStudents == 0) {
        printf("No student records found.\n");
        return;
    }

    printf("\n%-20s | %-5s\n", "Student Username", "Marks");
    printf("---------------------+------\n");
    for (int i = 0; i < numStudents; i++) {
        printf("%-20s | %-5d\n", students[i].studentUsername, students[i].marks);
    }
}

static void viewGrades_student(Student *students, int numStudents, const char *studentUsername) {
    int idx = find_student_index(students, numStudents, studentUsername);
    if (idx < 0) {
        printf("No marks found for %s.\n", studentUsername);
        return;
    }

    printf("\nStudent: %s\n", studentUsername);
    printf("Marks: %d\n", students[idx].marks);
}

static void viewStatistics(Student *students, int numStudents) {
    if (numStudents == 0) {
        printf("No student records found.\n");
        return;
    }

    int min = students[0].marks;
    int max = students[0].marks;
    long sum = 0;

    for (int i = 0; i < numStudents; i++) {
        int m = students[i].marks;
        if (m < min) min = m;
        if (m > max) max = m;
        sum += m;
    }

    double avg = (double)sum / (double)numStudents;

    printf("\n===== Statistics =====\n");
    printf("Students: %d\n", numStudents);
    printf("Average:  %.2f\n", avg);
    printf("Minimum:  %d\n", min);
    printf("Maximum:  %d\n", max);
}

static void searchStudent(Student *students, int numStudents) {
    if (numStudents == 0) {
        printf("No student records found.\n");
        return;
    }

    char query[MAX_NAME_LENGTH];
    read_line("Enter student username to search: ", query, sizeof(query));

    int idx = find_student_index(students, numStudents, query);
    if (idx < 0) {
        printf("Student not found.\n");
        return;
    }

    printf("\nFound:\n");
    printf("Student: %s\n", students[idx].studentUsername);
    printf("Marks:   %d\n", students[idx].marks);
}

// ---------- Menus ----------
static void teacherMenu(Student *students, int *numStudents) {
    while (1) {
        printf("\n===== Teacher Menu =====\n");
        printf("1. Upload Grades (Add/Update)\n");
        printf("2. Edit Grades\n");
        printf("3. Sort Grades\n");
        printf("4. View All Grades\n");
        printf("5. View Statistics\n");
        printf("6. Student Search\n");
        printf("7. Logout\n");

        int choice = read_int("Enter choice: ", 1, 7);
        switch (choice) {
            case 1: uploadGrades(students, numStudents); break;
            case 2: editGrades(students, *numStudents); break;
            case 3: sortGrades(students, *numStudents); break;
            case 4: viewGrades_all(students, *numStudents); break;
            case 5: viewStatistics(students, *numStudents); break;
            case 6: searchStudent(students, *numStudents); break;
            case 7: printf("Logging out...\n"); return;
        }
    }
}

static void studentMenu(Student *students, int numStudents, const char *studentUsername) {
    while (1) {
        printf("\n===== Student Menu =====\n");
        printf("1. View My Grades\n");
        printf("2. Logout\n");

        int choice = read_int("Enter choice: ", 1, 2);
        switch (choice) {
            case 1: viewGrades_student(students, numStudents, studentUsername); break;
            case 2: printf("Logging out...\n"); return;
        }
    }
}

int main(void) {
    User users[MAX_CREDENTIALS];
    Student students[MAX_STUDENTS];
    User currentUser;

    int numUsers = 0;
    int numStudents = 0;

    loadCredentials(users, &numUsers);
    loadStudentData(students, &numStudents);

    printf("Welcome to the Result Management System\n");

    while (1) {
        printf("\n===== Main Menu =====\n");
        printf("1. Login\n");
        printf("2. Exit\n");

        int mainChoice = read_int("Enter choice: ", 1, 2);
        if (mainChoice == 2) {
            printf("Goodbye.\n");
            break;
        }

        if (!login(users, numUsers, &currentUser)) {
            printf("Invalid login. Try again.\n");
            continue;
        }

        printf("Login successful! Role: %s\n", (currentUser.role == 'T') ? "Teacher" : "Student");

        // Reload data each session to reflect latest changes
        loadStudentData(students, &numStudents);

        if (currentUser.role == 'T') {
            teacherMenu(students, &numStudents);
        } else {
            studentMenu(students, numStudents, currentUser.username);
        }
    }

    return 0;
}
