# Result Management System (C)

Console-based Result Management System built in C with role-based login for Teachers and Students.

- **Teacher**: Upload, edit, view, sort, search student marks; view basic statistics (avg/min/max).
- **Student**: View *only* their own marks.

Data is persisted using plain-text files (File I/O).

## Project Structure
```
src/        # C source code
data/       # sample credentials + sample student records
docs/       # assignment requirements / notes
```

## Getting Started

### Prerequisites
- GCC (or any C compiler)

### Build
```bash
gcc -std=c11 -Wall -Wextra -O2 -o rms src/main.c
```

### Run
```bash
./rms
```

## Data Files

### `data/credentials.txt`
Format per line:
```
username password role
```
Where `role` is:
- `T` = Teacher
- `S` = Student

### `data/student_data.txt`
Format per line:
```
student_username marks
```

> Note: In this implementation, students are identified by `student_username` (matching `credentials.txt`).
> This makes it easy to enforce that students can only view their own marks.

## Demo Credentials
- Teacher: `teacher1 / pass123`
- Student: `student1 / pass123`
- Student: `student2 / pass123`

## Roadmap (Optional Enhancements)
- Support multiple subjects per student
- Hash passwords instead of storing plain text (for learning purposes only)
- Export results to CSV
- Add input validation and better error messages
