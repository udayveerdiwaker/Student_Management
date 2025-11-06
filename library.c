/* library.c
   Simple Library Management System in C
   - Console UI
   - File-based storage (binary files)
   - Books, Members, Transactions (issue/return)
   Compile: gcc library.c -o library
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
  #define CLEAR "cls"
#else
  #define CLEAR "clear"
#endif

#define BOOKS_FILE "books.dat"
#define MEMBERS_FILE "members.dat"
#define TRANS_FILE "transactions.dat"

#define ADMIN_PASS "admin123"  // change if you want

typedef struct {
    int id;
    char title[100];
    char author[60];
    int year;
    int total_copies;
    int available;
} Book;

typedef struct {
    int id;
    char name[60];
    char email[60];
    char phone[20];
} Member;

typedef struct {
    int trans_id;
    int book_id;
    int member_id;
    char issue_date[11];   // YYYY-MM-DD
    char return_date[11];  // YYYY-MM-DD or empty if not returned
    int returned; // 0 = not returned, 1 = returned
} Transaction;

/* Utility: read integer safely */
int read_int() {
    char buf[32];
    if (!fgets(buf, sizeof(buf), stdin)) return 0;
    return atoi(buf);
}

void press_enter() {
    printf("\nPress Enter to continue...");
    getchar();
}

/* Date helper: current date as YYYY-MM-DD */
void today_str(char *out, size_t n) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    snprintf(out, n, "%04d-%02d-%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
}

/* ===== File helpers ===== */
int next_book_id() {
    FILE *f = fopen(BOOKS_FILE, "rb");
    if (!f) return 1;
    Book b;
    int max = 0;
    while (fread(&b, sizeof(Book), 1, f)) {
        if (b.id > max) max = b.id;
    }
    fclose(f);
    return max + 1;
}

int next_member_id() {
    FILE *f = fopen(MEMBERS_FILE, "rb");
    if (!f) return 1;
    Member m;
    int max = 0;
    while (fread(&m, sizeof(Member), 1, f)) {
        if (m.id > max) max = m.id;
    }
    fclose(f);
    return max + 1;
}

int next_trans_id() {
    FILE *f = fopen(TRANS_FILE, "rb");
    if (!f) return 1;
    Transaction t;
    int max = 0;
    while (fread(&t, sizeof(Transaction), 1, f)) {
        if (t.trans_id > max) max = t.trans_id;
    }
    fclose(f);
    return max + 1;
}

/* ===== Book operations ===== */
void add_book() {
    system(CLEAR);
    printf("=== Add New Book ===\n");
    Book b;
    b.id = next_book_id();
    printf("Title: "); fgets(b.title, sizeof(b.title), stdin); b.title[strcspn(b.title, "\n")] = 0;
    printf("Author: "); fgets(b.author, sizeof(b.author), stdin); b.author[strcspn(b.author, "\n")] = 0;
    printf("Year: "); b.year = read_int();
    printf("Total Copies: "); b.total_copies = read_int();
    if (b.total_copies < 0) b.total_copies = 0;
    b.available = b.total_copies;

    FILE *f = fopen(BOOKS_FILE, "ab");
    if (!f) { perror("Cannot open books file"); return; }
    fwrite(&b, sizeof(Book), 1, f);
    fclose(f);
    printf("\nBook added with ID: %d\n", b.id);
    press_enter();
}

void list_books() {
    system(CLEAR);
    printf("=== Books List ===\n");
    FILE *f = fopen(BOOKS_FILE, "rb");
    if (!f) { printf("No books found.\n"); press_enter(); return; }
    Book b;
    printf("%-4s %-30s %-18s %-6s %-6s\n", "ID", "Title", "Author", "Year", "Avail");
    printf("----------------------------------------------------------------------------\n");
    while (fread(&b, sizeof(Book), 1, f)) {
        printf("%-4d %-30s %-18s %-6d %-6d\n", b.id, b.title, b.author, b.year, b.available);
    }
    fclose(f);
    press_enter();
}

Book* find_book_by_id(int id) {
    FILE *f = fopen(BOOKS_FILE, "rb");
    if (!f) return NULL;
    Book *res = NULL;
    Book b;
    while (fread(&b, sizeof(Book), 1, f)) {
        if (b.id == id) {
            res = malloc(sizeof(Book));
            *res = b;
            break;
        }
    }
    fclose(f);
    return res;
}

void save_book(Book *book) {
    // read all books, replace matching id, rewrite
    FILE *f = fopen(BOOKS_FILE, "rb");
    FILE *tmp = fopen("tmp.dat", "wb");
    if (!tmp) { perror("tmp file"); if(f) fclose(f); return; }
    Book b;
    if (!f) { // no books yet
        fwrite(book, sizeof(Book), 1, tmp);
    } else {
        while (fread(&b, sizeof(Book), 1, f)) {
            if (b.id == book->id) fwrite(book, sizeof(Book), 1, tmp);
            else fwrite(&b, sizeof(Book), 1, tmp);
        }
        fclose(f);
    }
    fclose(tmp);
    remove(BOOKS_FILE);
    rename("tmp.dat", BOOKS_FILE);
}

void edit_book() {
    system(CLEAR);
    printf("=== Edit Book ===\n");
    printf("Enter Book ID to edit: ");
    int id = read_int();
    Book *b = find_book_by_id(id);
    if (!b) { printf("Book not found.\n"); press_enter(); return; }
    printf("Editing Book ID %d\n", b->id);
    printf("Current Title: %s\nNew Title (leave blank to keep): ", b->title);
    char buf[200]; fgets(buf, sizeof(buf), stdin);
    if (buf[0] != '\n') { buf[strcspn(buf, "\n")] = 0; strncpy(b->title, buf, sizeof(b->title)); }
    printf("Current Author: %s\nNew Author (leave blank to keep): ", b->author);
    fgets(buf, sizeof(buf), stdin);
    if (buf[0] != '\n') { buf[strcspn(buf, "\n")] = 0; strncpy(b->author, buf, sizeof(b->author)); }
    printf("Current Year: %d\nNew Year (0 to keep): ", b->year);
    int year = read_int();
    if (year > 0) b->year = year;
    printf("Current Total Copies: %d\nNew Total Copies (0 to keep): ", b->total_copies);
    int tc = read_int();
    if (tc > 0) {
        int diff = tc - b->total_copies;
        b->total_copies = tc;
        b->available += diff;
        if (b->available < 0) b->available = 0;
    }
    save_book(b);
    free(b);
    printf("Book updated.\n");
    press_enter();
}

void delete_book() {
    system(CLEAR);
    printf("=== Delete Book ===\n");
    printf("Enter Book ID to delete: ");
    int id = read_int();
    FILE *f = fopen(BOOKS_FILE, "rb");
    if (!f) { printf("No books.\n"); press_enter(); return; }
    FILE *tmp = fopen("tmp.dat", "wb");
    Book b;
    int found = 0;
    while (fread(&b, sizeof(Book), 1, f)) {
        if (b.id == id) { found = 1; continue; }
        fwrite(&b, sizeof(Book), 1, tmp);
    }
    fclose(f); fclose(tmp);
    if (!found) {
        remove("tmp.dat");
        printf("Book not found.\n");
    } else {
        remove(BOOKS_FILE);
        rename("tmp.dat", BOOKS_FILE);
        printf("Book deleted.\n");
    }
    press_enter();
}

/* ===== Member operations ===== */
void add_member() {
    system(CLEAR);
    printf("=== Add New Member ===\n");
    Member m;
    m.id = next_member_id();
    printf("Name: "); fgets(m.name, sizeof(m.name), stdin); m.name[strcspn(m.name, "\n")] = 0;
    printf("Email: "); fgets(m.email, sizeof(m.email), stdin); m.email[strcspn(m.email, "\n")] = 0;
    printf("Phone: "); fgets(m.phone, sizeof(m.phone), stdin); m.phone[strcspn(m.phone, "\n")] = 0;
    FILE *f = fopen(MEMBERS_FILE, "ab");
    if (!f) { perror("Cannot open members file"); return; }
    fwrite(&m, sizeof(Member), 1, f);
    fclose(f);
    printf("\nMember added with ID: %d\n", m.id);
    press_enter();
}

void list_members() {
    system(CLEAR);
    printf("=== Members List ===\n");
    FILE *f = fopen(MEMBERS_FILE, "rb");
    if (!f) { printf("No members found.\n"); press_enter(); return; }
    Member m;
    printf("%-4s %-20s %-25s %-12s\n", "ID", "Name", "Email", "Phone");
    printf("---------------------------------------------------------------\n");
    while (fread(&m, sizeof(Member), 1, f)) {
        printf("%-4d %-20s %-25s %-12s\n", m.id, m.name, m.email, m.phone);
    }
    fclose(f);
    press_enter();
}

Member* find_member_by_id(int id) {
    FILE *f = fopen(MEMBERS_FILE, "rb");
    if (!f) return NULL;
    Member *res = NULL;
    Member m;
    while (fread(&m, sizeof(Member), 1, f)) {
        if (m.id == id) {
            res = malloc(sizeof(Member));
            *res = m;
            break;
        }
    }
    fclose(f);
    return res;
}

void save_member(Member *member) {
    FILE *f = fopen(MEMBERS_FILE, "rb");
    FILE *tmp = fopen("tmpm.dat", "wb");
    if (!tmp) { perror("tmp file"); if(f) fclose(f); return; }
    Member m;
    if (!f) {
        fwrite(member, sizeof(Member), 1, tmp);
    } else {
        while (fread(&m, sizeof(Member), 1, f)) {
            if (m.id == member->id) fwrite(member, sizeof(Member), 1, tmp);
            else fwrite(&m, sizeof(Member), 1, tmp);
        }
        fclose(f);
    }
    fclose(tmp);
    remove(MEMBERS_FILE);
    rename("tmpm.dat", MEMBERS_FILE);
}

void edit_member() {
    system(CLEAR);
    printf("=== Edit Member ===\n");
    printf("Enter Member ID to edit: ");
    int id = read_int();
    Member *m = find_member_by_id(id);
    if (!m) { printf("Member not found.\n"); press_enter(); return; }
    printf("Editing Member ID %d\n", m->id);
    printf("Current Name: %s\nNew Name (blank keep): ", m->name);
    char buf[200]; fgets(buf, sizeof(buf), stdin);
    if (buf[0] != '\n') { buf[strcspn(buf, "\n")] = 0; strncpy(m->name, buf, sizeof(m->name)); }
    printf("Current Email: %s\nNew Email (blank keep): ", m->email);
    fgets(buf, sizeof(buf), stdin);
    if (buf[0] != '\n') { buf[strcspn(buf, "\n")] = 0; strncpy(m->email, buf, sizeof(m->email)); }
    printf("Current Phone: %s\nNew Phone (blank keep): ", m->phone);
    fgets(buf, sizeof(buf), stdin);
    if (buf[0] != '\n') { buf[strcspn(buf, "\n")] = 0; strncpy(m->phone, buf, sizeof(m->phone)); }
    save_member(m);
    free(m);
    printf("Member updated.\n");
    press_enter();
}

void delete_member() {
    system(CLEAR);
    printf("=== Delete Member ===\n");
    printf("Enter Member ID to delete: ");
    int id = read_int();
    FILE *f = fopen(MEMBERS_FILE, "rb");
    if (!f) { printf("No members.\n"); press_enter(); return; }
    FILE *tmp = fopen("tmpm.dat", "wb");
    Member m;
    int found = 0;
    while (fread(&m, sizeof(Member), 1, f)) {
        if (m.id == id) { found = 1; continue; }
        fwrite(&m, sizeof(Member), 1, tmp);
    }
    fclose(f); fclose(tmp);
    if (!found) {
        remove("tmpm.dat");
        printf("Member not found.\n");
    } else {
        remove(MEMBERS_FILE);
        rename("tmpm.dat", MEMBERS_FILE);
        printf("Member deleted.\n");
    }
    press_enter();
}

/* ===== Transactions: Issue / Return ===== */
void issue_book() {
    system(CLEAR);
    printf("=== Issue Book ===\n");
    printf("Enter Book ID: "); int bid = read_int();
    Book *b = find_book_by_id(bid);
    if (!b) { printf("Book not found.\n"); press_enter(); return; }
    if (b->available <= 0) { printf("No copies available.\n"); free(b); press_enter(); return; }

    printf("Enter Member ID: "); int mid = read_int();
    Member *m = find_member_by_id(mid);
    if (!m) { printf("Member not found.\n"); free(b); press_enter(); return; }

    Transaction t;
    t.trans_id = next_trans_id();
    t.book_id = bid;
    t.member_id = mid;
    today_str(t.issue_date, sizeof(t.issue_date));
    strcpy(t.return_date, "");
    t.returned = 0;

    FILE *f = fopen(TRANS_FILE, "ab");
    if (!f) { perror("Cannot open transactions file"); free(b); free(m); return; }
    fwrite(&t, sizeof(Transaction), 1, f);
    fclose(f);

    b->available -= 1;
    save_book(b);

    printf("Book issued. Transaction ID: %d\n", t.trans_id);
    free(b); free(m);
    press_enter();
}

Transaction* find_transaction_by_id(int id) {
    FILE *f = fopen(TRANS_FILE, "rb");
    if (!f) return NULL;
    Transaction *res = NULL;
    Transaction t;
    while (fread(&t, sizeof(Transaction), 1, f)) {
        if (t.trans_id == id) {
            res = malloc(sizeof(Transaction));
            *res = t;
            break;
        }
    }
    fclose(f);
    return res;
}

void save_transaction(Transaction *tr) {
    FILE *f = fopen(TRANS_FILE, "rb");
    FILE *tmp = fopen("tmpt.dat", "wb");
    if (!tmp) { perror("tmp file"); if(f) fclose(f); return; }
    Transaction t;
    if (!f) {
        fwrite(tr, sizeof(Transaction), 1, tmp);
    } else {
        while (fread(&t, sizeof(Transaction), 1, f)) {
            if (t.trans_id == tr->trans_id) fwrite(tr, sizeof(Transaction), 1, tmp);
            else fwrite(&t, sizeof(Transaction), 1, tmp);
        }
        fclose(f);
    }
    fclose(tmp);
    remove(TRANS_FILE);
    rename("tmpt.dat", TRANS_FILE);
}

void return_book() {
    system(CLEAR);
    printf("=== Return Book ===\n");
    printf("Enter Transaction ID: "); int tid = read_int();
    Transaction *tr = find_transaction_by_id(tid);
    if (!tr) { printf("Transaction not found.\n"); press_enter(); return; }
    if (tr->returned) { printf("Already returned on %s\n", tr->return_date); free(tr); press_enter(); return; }

    today_str(tr->return_date, sizeof(tr->return_date));
    tr->returned = 1;
    save_transaction(tr);

    Book *b = find_book_by_id(tr->book_id);
    if (b) {
        b->available += 1;
        save_book(b);
        free(b);
    }
    printf("Book returned. Transaction updated.\n");
    free(tr);
    press_enter();
}

void list_transactions() {
    system(CLEAR);
    printf("=== Transactions ===\n");
    FILE *f = fopen(TRANS_FILE, "rb");
    if (!f) { printf("No transactions.\n"); press_enter(); return; }
    Transaction t;
    printf("%-4s %-6s %-6s %-12s %-12s %-8s\n", "TID", "Book", "Mem", "IssueDate", "ReturnDate", "Status");
    printf("----------------------------------------------------------------\n");
    while (fread(&t, sizeof(Transaction), 1, f)) {
        printf("%-4d %-6d %-6d %-12s %-12s %-8s\n", t.trans_id, t.book_id, t.member_id, t.issue_date,
               t.returned ? t.return_date : "N/A", t.returned ? "Returned" : "Issued");
    }
    fclose(f);
    press_enter();
}

/* ===== Search utilities ===== */
void search_books_by_title() {
    system(CLEAR);
    printf("=== Search Books (by title) ===\n");
    char key[100];
    printf("Enter keyword: "); fgets(key, sizeof(key), stdin); key[strcspn(key, "\n")] = 0;
    FILE *f = fopen(BOOKS_FILE, "rb");
    if (!f) { printf("No books.\n"); press_enter(); return; }
    Book b; int found = 0;
    while (fread(&b, sizeof(Book), 1, f)) {
        if (strcasestr(b.title, key)) {
            if(!found) printf("%-4s %-30s %-18s %-6s %-6s\n", "ID", "Title", "Author", "Year", "Avail");
            printf("%-4d %-30s %-18s %-6d %-6d\n", b.id, b.title, b.author, b.year, b.available);
            found = 1;
        }
    }
    fclose(f);
    if (!found) printf("No matching books.\n");
    press_enter();
}

void search_members_by_name() {
    system(CLEAR);
    printf("=== Search Members (by name) ===\n");
    char key[100];
    printf("Enter keyword: "); fgets(key, sizeof(key), stdin); key[strcspn(key, "\n")] = 0;
    FILE *f = fopen(MEMBERS_FILE, "rb");
    if (!f) { printf("No members.\n"); press_enter(); return; }
    Member m; int found = 0;
    while (fread(&m, sizeof(Member), 1, f)) {
        if (strcasestr(m.name, key)) {
            if(!found) printf("%-4s %-20s %-25s %-12s\n", "ID", "Name", "Email", "Phone");
            printf("%-4d %-20s %-25s %-12s\n", m.id, m.name, m.email, m.phone);
            found = 1;
        }
    }
    fclose(f);
    if (!found) printf("No matching members.\n");
    press_enter();
}

/* ===== Menus ===== */
void books_menu() {
    while (1) {
        system(CLEAR);
        printf("=== Books Menu ===\n");
        printf("1. Add Book\n2. List Books\n3. Edit Book\n4. Delete Book\n5. Search Books by Title\n0. Back\nChoose: ");
        int c = read_int();
        getchar(); // consume leftover newline
        if (c == 1) add_book();
        else if (c == 2) list_books();
        else if (c == 3) edit_book();
        else if (c == 4) delete_book();
        else if (c == 5) search_books_by_title();
        else if (c == 0) break;
    }
}

void members_menu() {
    while (1) {
        system(CLEAR);
        printf("=== Members Menu ===\n");
        printf("1. Add Member\n2. List Members\n3. Edit Member\n4. Delete Member\n5. Search Members by Name\n0. Back\nChoose: ");
        int c = read_int();
        getchar();
        if (c == 1) add_member();
        else if (c == 2) list_members();
        else if (c == 3) edit_member();
        else if (c == 4) delete_member();
        else if (c == 5) search_members_by_name();
        else if (c == 0) break;
    }
}

void transactions_menu() {
    while (1) {
        system(CLEAR);
        printf("=== Transactions Menu ===\n");
        printf("1. Issue Book\n2. Return Book\n3. List Transactions\n0. Back\nChoose: ");
        int c = read_int();
        getchar();
        if (c == 1) issue_book();
        else if (c == 2) return_book();
        else if (c == 3) list_transactions();
        else if (c == 0) break;
    }
}

int admin_login() {
    system(CLEAR);
    printf("=== Library Management System ===\n");
    printf("Admin Login\nPassword: ");
    char pass[100];
    fgets(pass, sizeof(pass), stdin);
    pass[strcspn(pass, "\n")] = 0;
    if (strcmp(pass, ADMIN_PASS) == 0) return 1;
    return 0;
}

int main() {
    if (!admin_login()) {
        printf("Incorrect password. Exiting.\n");
        return 0;
    }
    while (1) {
        system(CLEAR);
        printf("=== Main Menu ===\n");
        printf("1. Books\n2. Members\n3. Transactions (Issue/Return)\n4. Search\n5. Exit\nChoose: ");
        int c = read_int();
        getchar();
        if (c == 1) books_menu();
        else if (c == 2) members_menu();
        else if (c == 3) transactions_menu();
        else if (c == 4) {
            while (1) {
                system(CLEAR);
                printf("=== Search Menu ===\n1. Search Books by Title\n2. Search Members by Name\n0. Back\nChoose: ");
                int s = read_int(); getchar();
                if (s == 1) search_books_by_title();
                else if (s == 2) search_members_by_name();
                else if (s == 0) break;
            }
        }
        else if (c == 5) {
            printf("Goodbye!\n");
            break;
        }
    }
    return 0;
}
