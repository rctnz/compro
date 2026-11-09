/*
 * library.c - ระบบจัดการห้องสมุด (Library Management System) กลุ่มที่ 1
 * ===== VERSION 3: ต่อยอดจาก v2 + ฟังก์ชันพิเศษ (unique) + ASCII art + cls =====
 *
 * ฟังก์ชันพิเศษที่เพิ่ม (section 10-11)
 *   Book recommendation 16 หมวด
 *     Non-Fiction: Business & Economics, Food & Drink, Health/Lifestyle & Sports, Self Development,
 *                  History, Biography, Science & Technology
 *     Fiction    : Romance, Action & Adventure, Crime & Thriller, Fantasy, Horror, Sci-Fi,
 *                  Literary, Historical, Comedy
 *   1) เลือกตามหมวด    : เลือก 1 ใน 16 หมวด -> แสดงหนังสือหมวดนั้นเรียงตามคะแนน
 *   2) แนะนำตามวิชา     : เลือกวิชา -> แนะนำหนังสือในหมวดที่ตรงกับวิชานั้น
 *   3) ให้คะแนน 1-5     : ถามตอนคืนหนังสือ -> ตาราง Top Rated + "แนะนำต่อ" (หนังสือหมวดเดียวกัน)
 *   + ล้างหน้าจอ (cls) ทุกครั้งที่เปลี่ยนหน้า และเมนูแบบ ASCII art
 *
 * ไฟล์ข้อมูลเพิ่ม 2 ไฟล์ (ไฟล์เดิม 4 ไฟล์และ struct เดิมไม่เปลี่ยน เพื่อไม่ให้งานเพื่อนพัง)
 *   bookcats.txt  bookId|categoryId        หมวดของหนังสือแต่ละเล่ม
 *   ratings.txt   studentId|bookId|score   คะแนนที่นักศึกษาให้ (1-5)
 *
 * stdlib.h ใช้แค่ system("cls") กับ system("chcp 65001") เท่านั้น
 * time.h ใช้แค่หาวันนี้ ตอนกด Enter ในช่องปีของการกรอกวันที่
 * ส่วนอื่นยังไม่ใช้ string.h / ctype.h เลย
 * ฟังก์ชันที่ปกติต้องใช้จาก library พวกนั้น (strcmp, strcpy, strlen, atoi, isspace ...)
 * เขียนเองทั้งหมดในส่วน "ฟังก์ชันช่วยกลาง" ด้านล่าง
 * (sscanf / fgets / fprintf / fopen อยู่ใน stdio.h จึงใช้ได้)
 *
 * โค้ดของเพื่อนแต่ละคนถูกนำมารวมไว้ในไฟล์นี้ แยกเป็น section ตามชื่อคนทำ
 * logic เหมือนของเดิม แต่เปลี่ยนส่วนที่เรียก string.h / ctype.h / stdlib.h
 * ให้ไปเรียกฟังก์ชันช่วยกลางแทน และเปลี่ยนการรับข้อมูลให้เป็นแบบอ่านทีละบรรทัดเหมือนกันหมด
 *
 *   section 1  struct + ตัวแปรกลาง                    (ทุกคนตกลงร่วมกัน)
 *   section 2  ฟังก์ชันช่วยกลาง (string / input / date) (@rctnz)
 *   section 3  อ่าน/บันทึกไฟล์                          (@rctnz)
 *   section 4  เพิ่ม / ลบหนังสือ                         (@MaiDaiJarRuu)
 *   section 5  ค้นหาหนังสือ / แสดงทั้งหมด + สรุป         (@pryntz_.)
 *   section 6  ยืม / คืนหนังสือ                          (@หวัดดีฮาฟพี่)
 *   section 7  ตรวจห้องว่าง / จองห้อง                    (@GG)
 *   section 8  ค้นหายืม-คืน / ค้นหาการจอง / ยกเลิกจอง    (@NightMare)
 *   section 9  หมวดหนังสือ + คะแนน (ข้อมูล/ไฟล์)        (unique)
 *   section 10 เลือกตามหมวด / ตามวิชา / Top Rated       (unique)
 *   section 11 หน้าจอ: cls, pause, ASCII art            (unique)
 *   section 12 เมนู + main                              (@rctnz)
 *
 * คอมไพล์:  gcc -std=c99 -Wall library.c -o library   (Dev-C++ เปิดไฟล์แล้วกด F11 ได้เลย)
 * โปรแกรมสั่ง chcp 65001 ให้เองตอนเริ่ม จึงแสดงภาษาไทยได้โดยไม่ต้องตั้งค่าเพิ่ม
 */
#include <stdio.h>
#include <stdlib.h>   /* system() สำหรับ cls / chcp เท่านั้น */
#include <time.h>     /* หาวันนี้ สำหรับกด Enter ตอนกรอกวันที่ */

/* =====================================================================
 * SECTION 1: struct + ตัวแปรกลาง (ทุกคนตกลงร่วมกัน)
 * ===================================================================== */

#define MAX_BOOKS 1000
#define MAX_LOANS 5000
#define MAX_ROOMS 50
#define MAX_BOOKINGS 5000
#define LOAN_DAYS 15        /* ยืมได้สูงสุด 15 วัน นับจากวันที่ยืม */
#define OPEN_HOUR 9         /* ช่วงแรกที่จองได้ 09.00-10.00 */
#define LAST_HOUR 16        /* ช่วงสุดท้ายที่จองได้ 16.00-17.00 */

typedef struct {
    int year;
    int month;
    int day;
} Date;

/* ข้อมูลหนังสือ */
typedef struct {
    char bookId[20];
    char title[256];
    char author[128];
    int isBorrowed; /* 0 = พร้อมให้ยืม, 1 = ถูกยืม */
} Book;

/* รายการยืม-คืน */
typedef struct {
    char studentId[20];
    char bookId[20];
    char bookTitle[256]; /* เก็บชื่อหนังสือ ณ วันที่ยืม */
    Date borrowDate;
    Date dueDate;
    Date returnDate;     /* ยังไม่คืน = {0,0,0} (ในไฟล์เป็น "-") */
    int isReturned;      /* 0 = ยังไม่คืน, 1 = คืนแล้ว */
} Loan;

/* ข้อมูลห้องอ่านหนังสือ */
typedef struct {
    char roomId[20];
    char roomName[100];
} Room;

/* รายการจองห้อง */
typedef struct {
    int bookingId;
    char studentId[20];
    char roomId[20];
    Date bookingDate;
    int startHour; /* 9 ถึง 16 */
    int isActive;  /* 0 = ยกเลิกแล้ว, 1 = ยังไม่ยกเลิก */
} Booking;

Book books[MAX_BOOKS];          /* หนังสือสูงสุด 1000 เล่ม */
Loan loans[MAX_LOANS];          /* เก็บประวัติยืมคืนได้ 5000 รายการ */
Room rooms[MAX_ROOMS];          /* เก็บห้องได้สูงสุด 50 ห้อง */
Booking bookings[MAX_BOOKINGS]; /* เก็บรายการจองได้สูงสุด 5000 รายการ */
int bookCount = 0, loanCount = 0, roomCount = 0, bookingCount = 0;

/* ----- ข้อมูลเพิ่มสำหรับฟังก์ชันพิเศษ (unique) ----- */

#define MAX_RATINGS 5000
#define CATEGORY_COUNT 20   /* หมวดหนังสือมี 20 หมวด (รหัส 1-20, 0 = ยังไม่ระบุ) */

/*
 * รหัสหมวดแบ่งเป็น 3 กลุ่ม
 *   1-4   Textbook     ตำราเรียน (ใช้กับ "แนะนำตามวิชา")
 *   5-11  Non-Fiction  สารคดี/ความรู้ 7 หมวด
 *   12-20 Fiction      นิยาย 9 หมวด
 */
#define TEXTBOOK_FIRST    1
#define TEXTBOOK_LAST     4
#define NONFICTION_FIRST  5
#define NONFICTION_LAST   11
#define FICTION_FIRST     12
#define FICTION_LAST      20

/* ชื่อหมวดแบบสั้น (ไม่เกิน 16 ตัว) ใช้ในตาราง เพื่อให้คอลัมน์ตรงกัน */
const char *categoryNames[CATEGORY_COUNT + 1] = {
    "(none)",
    "Programming",          /* 1 */
    "Data & AI",            /* 2 */
    "Mathematics",          /* 3 */
    "Electronics",          /* 4 */
    "Business & Econ.",     /* 5 */
    "Food & Drink",         /* 6 */
    "Health & Sports",      /* 7 */
    "Self Development",     /* 8 */
    "History",              /* 9 */
    "Biography",            /* 10 */
    "Science & Tech",       /* 11 */
    "Romance",              /* 12 */
    "Action/Adventure",     /* 13 */
    "Crime & Thriller",     /* 14 */
    "Fantasy",              /* 15 */
    "Horror",               /* 16 */
    "Sci-Fi",               /* 17 */
    "Literary",             /* 18 */
    "Historical",           /* 19 */
    "Comedy"                /* 20 */
};
/* ชื่อหมวดเต็ม ใช้ในหน้าเลือกหมวด และหน้ารายละเอียดหนังสือ */
const char *categoryFull[CATEGORY_COUNT + 1] = {
    "(none)",
    "Programming",
    "Data & AI",
    "Mathematics",
    "Electronics",
    "Business & Economics",
    "Food & Drink",
    "Health, Lifestyle & Sports",
    "Self Development",
    "History",
    "Biography",
    "Science & Technology",
    "Romance",
    "Action & Adventure",
    "Crime & Thriller",
    "Fantasy",
    "Horror",
    "Sci-Fi",
    "Literary",
    "Historical",
    "Comedy"
};
/* ชื่อหมวดภาษาไทย (แสดงในหน้าเลือกหมวดเท่านั้น เพราะภาษาไทยจัดความกว้างในตาราง/กรอบไม่ตรง) */
const char *categoryThai[CATEGORY_COUNT + 1] = {
    "-",
    "การเขียนโปรแกรม",
    "ข้อมูลและ AI",
    "คณิตศาสตร์",
    "อิเล็กทรอนิกส์",
    "ธุรกิจและเศรษฐศาสตร์",
    "อาหารและเครื่องดื่ม",
    "สุขภาพ ไลฟ์สไตล์ และกีฬา",
    "พัฒนาตัวเอง",
    "ประวัติศาสตร์",
    "ชีวประวัติ",
    "วิทยาศาสตร์และเทคโนโลยี",
    "โรแมนติก",
    "แอ็กชันและผจญภัย",
    "อาชญากรรมและระทึกขวัญ",
    "แฟนตาซี",
    "สยองขวัญ",
    "ไซไฟ",
    "วรรณกรรม",
    "นิยายอิงประวัติศาสตร์",
    "ตลกขบขัน"
};

/* หมวดของหนังสือ 1 เล่ม (แยกจาก Book เพื่อไม่ต้องแก้ struct ที่ทีมตกลงกันไว้) */
typedef struct {
    char bookId[20];
    int categoryId;     /* 1-10 */
} BookCategory;

/* คะแนนที่นักศึกษาให้หนังสือ 1 ครั้ง */
typedef struct {
    char studentId[20];
    char bookId[20];
    int score;          /* 1-5 */
} Rating;

BookCategory bookCats[MAX_BOOKS];
Rating ratings[MAX_RATINGS];
int bookCatCount = 0, ratingCount = 0;

/*
 * prototype ของฟังก์ชันใน section 9-10 ที่ section 4-6 เรียกใช้ก่อน
 * (C ต้องรู้จักชื่อฟังก์ชันก่อนเรียก ตัวฟังก์ชันจริงอยู่ด้านล่าง)
 */
int getBookCategory(const char bookId[]);
void setBookCategory(const char bookId[], int categoryId);
void removeBookCategory(const char bookId[]);
int chooseCategory(void);
void printCategoryRange(const char *groupName, int first, int last, int offset);
double averageRating(const char bookId[], int *count);
void printStars(double average);
void loadBookCategories(void);
void loadRatings(void);
int saveBookCategories(void);
int saveRatings(void);
void askRatingAfterReturn(int loanIndex);

/* =====================================================================
 * SECTION 2: ฟังก์ชันช่วยกลาง (@rctnz)
 * ใช้แทน string.h / ctype.h / stdlib.h ทุกส่วนเรียกใช้ร่วมกัน
 * ===================================================================== */

/* ---------- ข้อความ (แทน string.h) ---------- */

/* textEquals(): แทน strcmp(a, b) == 0 ; คืน 1 ถ้าเหมือนกันทุกตัวอักษรและยาวเท่ากัน */
int textEquals(const char a[], const char b[])
{
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) return 0;   /* เจอตัวที่ไม่ตรงกัน */
        i++;
    }
    return a[i] == '\0' && b[i] == '\0';  /* ต้องจบพร้อมกันด้วย */
}

/* textContains(): คืน 1 ถ้า keyword อยู่ในส่วนใดส่วนหนึ่งของ text (แทน strstr) */
int textContains(const char text[], const char keyword[])
{
    int i, j;
    if (keyword[0] == '\0') return 0;
    /* ลองเริ่มเทียบ keyword จากทุกตำแหน่งใน text */
    for (i = 0; text[i] != '\0'; i++) {
        j = 0;
        while (text[i + j] != '\0' && keyword[j] != '\0' && text[i + j] == keyword[j]) {
            j++;
        }
        if (keyword[j] == '\0') return 1;   /* เทียบจนจบ keyword แล้ว = เจอ */
    }
    return 0;
}

/* textCopy(): แทน strcpy แต่ไม่ล้น buffer (size = ขนาด dest รวม '\0') */
void textCopy(char dest[], const char src[], int size)
{
    int i = 0;
    while (src[i] != '\0' && i < size - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

/* isBlank(): แทน isspace สำหรับช่องว่าง / tab / \r */
int isBlank(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

/* ---------- รับข้อมูลจากคีย์บอร์ด ---------- */

/*
 * readBookInput(): (ต้นฉบับของ @pryntz_.) อ่านข้อความ 1 บรรทัดด้วย getchar
 * ตัดช่องว่างหน้า-หลัง ; size ต้องเป็นขนาด array
 * คืน 1 = อ่านได้ (อาจเป็นข้อความว่าง), -1 = ยาวเกิน, 0 = EOF
 */
int readBookInput(char text[], int size)
{
    int ch;
    int i = 0;
    int tooLong = 0;

    ch = getchar();
    while (ch != '\n' && ch != EOF) {
        /* ข้ามช่องว่างด้านหน้า (ยังไม่ได้เก็บตัวอักษรเลย) */
        if (i == 0 && isBlank((char)ch)) {
            ch = getchar();
            continue;
        }
        if (i < size - 1) {        /* เว้นช่องสุดท้ายไว้ใส่ '\0' */
            text[i] = (char)ch;
            i++;
        } else {
            tooLong = 1;           /* เต็มแล้ว อ่านทิ้งต่อจนจบบรรทัด */
        }
        ch = getchar();
    }

    /* ถอยข้ามช่องว่างด้านหลัง (รวม \r ของไฟล์ Windows) */
    while (i > 0 && isBlank(text[i - 1])) i--;
    text[i] = '\0';

    if (tooLong == 1) {
        printf("Input is too long.\n");
        return -1;
    }
    if (ch == EOF && i == 0) return 0;
    return 1;
}

/*
 * readRequiredLine(): แสดง prompt แล้วรับข้อความที่ "ห้ามว่าง"
 * ถ้าว่างหรือยาวเกินจะถามใหม่ ; คืน 1 = ได้ข้อความ, 0 = EOF (ยกเลิก)
 */
int readRequiredLine(const char prompt[], char buffer[], int size)
{
    int result;
    for (;;) {
        printf("%s", prompt);
        result = readBookInput(buffer, size);
        if (result == 0) return 0;                 /* EOF */
        if (result == -1) continue;                /* ยาวเกิน ถามใหม่ */
        if (buffer[0] == '\0') {
            printf("Input cannot be empty. Please try again.\n");
            continue;
        }
        return 1;
    }
}

/*
 * parseNumber(): แปลงข้อความเป็นจำนวนเต็ม >= 0 (แทน atoi / strtol)
 * ใช้ sscanf จาก stdio.h ; %c ตัวท้ายใช้จับตัวอักษรที่เกินมา เช่น "12abc"
 * คืน 1 = สำเร็จ, 0 = ไม่ใช่ตัวเลขล้วน
 */
int parseNumber(const char text[], int *result)
{
    int value;
    char extra;
    if (sscanf(text, "%d %c", &value, &extra) != 1 || value < 0) return 0;
    *result = value;
    return 1;
}

/* readNumber(): แสดง prompt แล้วรับตัวเลข >= 0 จนกว่าจะถูก ; คืน 0 = EOF */
int readNumber(const char prompt[], int *result)
{
    char text[50];
    for (;;) {
        if (!readRequiredLine(prompt, text, sizeof(text))) return 0;
        if (parseNumber(text, result)) return 1;
        printf("Please type a whole number.\n");
    }
}

/*
 * readYesNo(): ถาม yes/no จนกว่าจะตอบถูก (รับ yes, y, no, n ตัวพิมพ์เล็กหรือใหญ่)
 * คืน 1 = yes, 0 = no หรือ EOF
 */
int readYesNo(const char question[])
{
    char answer[20];
    int i, result;
    for (;;) {
        printf("%s (yes/no): ", question);
        result = readBookInput(answer, sizeof(answer));
        if (result == 0) return 0;
        if (result == 1) {
            for (i = 0; answer[i] != '\0'; i++) {
                if (answer[i] >= 'A' && answer[i] <= 'Z') answer[i] += 'a' - 'A';
            }
            if (textEquals(answer, "yes") || textEquals(answer, "y")) return 1;
            if (textEquals(answer, "no") || textEquals(answer, "n")) return 0;
        }
        printf("Please type yes or no.\n");
    }
}

/* ---------- รูปแบบรหัส ---------- */

#define STUDENT_ID_DIGITS 11
#define STUDENT_ID_EXAMPLE "69000000001"
#define BOOK_ID_EXAMPLE "B001"
#define BOOK_ID_PROMPT "Book ID (e.g. " BOOK_ID_EXAMPLE "): "
#define STUDENT_ID_PROMPT "Student ID (11 digits, e.g. " STUDENT_ID_EXAMPLE "): "

/* ตัวเลข 0-9 ทั้งข้อความหรือไม่ และยาวอย่างน้อย minDigits ตัว ; คืนจำนวนหลัก (ไม่ผ่านคืน 0) */
int countDigits(const char text[], int minDigits)
{
    int i = 0;
    while (text[i] != '\0') {
        if (text[i] < '0' || text[i] > '9') return 0;
        i++;
    }
    return i >= minDigits ? i : 0;
}

/* readStudentId(): รับรหัสนักศึกษาจนกว่าจะเป็นตัวเลข 11 หลัก ; คืน 0 = EOF */
int readStudentId(const char prompt[], char buffer[], int size)
{
    for (;;) {
        if (!readRequiredLine(prompt, buffer, size)) return 0;
        if (countDigits(buffer, STUDENT_ID_DIGITS) == STUDENT_ID_DIGITS) return 1;
        printf("Invalid student ID. Correct form: %d digits, e.g. %s\n",
               STUDENT_ID_DIGITS, STUDENT_ID_EXAMPLE);
    }
}

/* readBookId(): รับรหัสหนังสือจนกว่าจะเป็น B ตามด้วยตัวเลข 3 หลักขึ้นไป ; คืน 0 = EOF */
int readBookId(char buffer[], int size)
{
    for (;;) {
        if (!readRequiredLine(BOOK_ID_PROMPT, buffer, size)) return 0;
        if (buffer[0] == 'B' && countDigits(&buffer[1], 3)) return 1;
        printf("Invalid book ID. Correct form: B + 3 digits, e.g. %s\n", BOOK_ID_EXAMPLE);
    }
}

/* ---------- วันที่ ---------- */

/* ปีอธิกสุรทิน: หาร 4 ลงตัว แต่ไม่ลงตัวด้วย 100 ยกเว้นหาร 400 ลงตัว */
int isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/* จำนวนวันในเดือน (ก.พ. ปีอธิกสุรทินมี 29 วัน) */
int daysInMonth(int year, int month)
{
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) return 29;
    return days[month - 1];
}

#define MIN_YEAR 2000       /* ปี ค.ศ. ที่รับได้ */
#define MAX_YEAR 2100
#define BE_OFFSET 543       /* พ.ศ. = ค.ศ. + 543 */

/* วันที่สองวันตรงกันหรือไม่ (ของ @GG) */
int isSameDate(Date a, Date b)
{
    return a.year == b.year && a.month == b.month && a.day == b.day;
}

/*
 * dateToDays(): แปลงวันที่เป็นลำดับวันแบบนับต่อเนื่อง เอาไว้ลบกันหาจำนวนวันห่าง
 * ย้าย ม.ค./ก.พ. ไปเป็นเดือน 13/14 ของปีก่อน ให้ 29 ก.พ. อยู่ท้ายปีพอดี
 */
long dateToDays(Date d)
{
    long y = d.year;
    long m = d.month;
    if (m < 3) {
        y--;
        m += 12;
    }
    return 365 * y + y / 4 - y / 100 + y / 400 + (153 * (m - 3) + 2) / 5 + d.day;
}

/* daysBetween(): b ห่างจาก a กี่วัน (บวก = b อยู่หลัง a) */
long daysBetween(Date a, Date b)
{
    return dateToDays(b) - dateToDays(a);
}

/* addDays(): บวกวันทีละวัน เกินสิ้นเดือนขึ้นเดือนใหม่ เกินเดือน 12 ขึ้นปีใหม่ */
Date addDays(Date d, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        d.day++;
        if (d.day > daysInMonth(d.year, d.month)) {
            d.day = 1;
            d.month++;
            if (d.month > 12) {
                d.month = 1;
                d.year++;
            }
        }
    }
    return d;
}

/* todayDate(): วันนี้ตามนาฬิกาเครื่อง */
Date todayDate(void)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    Date d;
    d.year = t->tm_year + 1900;
    d.month = t->tm_mon + 1;
    d.day = t->tm_mday;
    return d;
}

/*
 * readDate(): ถามปี เดือน วัน ทีละคำถาม ตอบผิดถามใหม่เฉพาะข้อนั้น
 * ปีรับได้ทั้ง ค.ศ. และ พ.ศ. (พ.ศ. แปลงเป็น ค.ศ. ให้อัตโนมัติ)
 * กด Enter ที่ช่องปี = ใช้วันนี้ทั้งวันที่ (ไม่ถามเดือนกับวันต่อ)
 * คืน 1 = สำเร็จ, 0 = EOF
 */
int readDate(const char label[], Date *out)
{
    Date d;
    Date today = todayDate();
    char prompt[100];
    char text[50];
    int maxDay, result;

    printf("%s\n", label);
    sprintf(prompt, "  Year (e.g. 2026 or 2569, Enter = today %04d-%02d-%02d): ",
            today.year, today.month, today.day);
    for (;;) {
        printf("%s", prompt);
        result = readBookInput(text, sizeof(text));
        if (result == 0) return 0;
        if (result == 1 && text[0] == '\0') {
            printf("  Using today: %04d-%02d-%02d\n", today.year, today.month, today.day);
            *out = today;
            return 1;
        }
        if (result == 1 && parseNumber(text, &d.year)) {
            if (d.year >= MIN_YEAR + BE_OFFSET && d.year <= MAX_YEAR + BE_OFFSET) {
                printf("  B.E. %d -> C.E. %d\n", d.year, d.year - BE_OFFSET);
                d.year -= BE_OFFSET;
            }
            if (d.year >= MIN_YEAR && d.year <= MAX_YEAR) break;
        }
        printf("  Invalid year. Correct form: 4 digits, e.g. 2026 (C.E.) or 2569 (B.E.)\n");
    }
    for (;;) {
        if (!readNumber("  Month (1-12, e.g. 11): ", &d.month)) return 0;
        if (d.month >= 1 && d.month <= 12) break;
        printf("  Invalid month. Correct form: a number from 1 to 12, e.g. 11\n");
    }
    maxDay = daysInMonth(d.year, d.month);
    sprintf(prompt, "  Day (1-%d, e.g. 8): ", maxDay);
    for (;;) {
        if (!readNumber(prompt, &d.day)) return 0;
        if (d.day >= 1 && d.day <= maxDay) break;
        printf("  Invalid day. Correct form: a number from 1 to %d, e.g. 8\n", maxDay);
    }
    *out = d;
    return 1;
}

/* printDate(): แสดง YYYY-MM-DD ; ถ้าไม่มีวันที่ (ปี = 0) แสดง "-" */
void printDate(Date d)
{
    if (d.year == 0) printf("-");
    else printf("%04d-%02d-%02d", d.year, d.month, d.day);
}

/* =====================================================================
 * SECTION 3: อ่าน/บันทึกไฟล์ (@rctnz)
 * books.txt    bookId|title|author|isBorrowed
 * loans.txt    studentId|bookId|bookTitle|borrowDate|dueDate|returnDate|isReturned
 * rooms.txt    roomId|roomName
 * bookings.txt bookingId|studentId|roomId|bookingDate|startHour|isActive
 * ===================================================================== */

#define LINE_SIZE 1024
#define MAX_FIELDS 10

/*
 * splitLine(): ตัด \r\n และ BOM ออก แล้วแยกบรรทัดด้วย '|'
 * โดยเปลี่ยน '|' เป็น '\0' และเก็บตำแหน่งเริ่มของแต่ละช่องไว้ใน fields
 * คืนจำนวนช่อง (บรรทัดว่างคืน 0)
 */
int splitLine(char line[], char *fields[], int maxFields)
{
    int i = 0, count = 0;

    /* ข้าม BOM (EF BB BF) ที่ Notepad ชอบใส่หน้าไฟล์ UTF-8 */
    if ((unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB &&
        (unsigned char)line[2] == 0xBF) {
        i = 3;
    }
    if (line[i] == '\0' || line[i] == '\n' || line[i] == '\r') return 0;

    fields[count++] = &line[i];
    for (; line[i] != '\0'; i++) {
        if (line[i] == '\n' || line[i] == '\r') {   /* ท้ายบรรทัด จบตรงนี้ */
            line[i] = '\0';
            break;
        }
        if (line[i] == '|') {
            line[i] = '\0';
            if (count < maxFields) fields[count++] = &line[i + 1];
        }
    }
    return count;
}

/* parseDateField(): "YYYY-MM-DD" -> Date ; "-" หรืออ่านไม่ได้ -> {0,0,0} */
Date parseDateField(const char text[])
{
    Date d = {0, 0, 0};
    if (sscanf(text, "%d-%d-%d", &d.year, &d.month, &d.day) != 3) {
        d.year = d.month = d.day = 0;
    }
    return d;
}

/* toInt(): แปลงข้อความในไฟล์เป็นตัวเลข (อ่านไม่ได้ = 0) */
int toInt(const char text[])
{
    int value = 0;
    sscanf(text, "%d", &value);
    return value;
}

void loadBooks(void)
{
    char line[LINE_SIZE];
    char *f[MAX_FIELDS];
    FILE *fp = fopen("books.txt", "r");
    bookCount = 0;
    if (fp == NULL) {
        printf("Warning: books.txt not found. Starting with no books.\n");
        return;
    }
    while (bookCount < MAX_BOOKS && fgets(line, sizeof(line), fp) != NULL) {
        if (splitLine(line, f, MAX_FIELDS) < 4) continue;   /* ข้ามบรรทัดว่าง/ไม่ครบ */
        textCopy(books[bookCount].bookId, f[0], sizeof(books[bookCount].bookId));
        textCopy(books[bookCount].title, f[1], sizeof(books[bookCount].title));
        textCopy(books[bookCount].author, f[2], sizeof(books[bookCount].author));
        books[bookCount].isBorrowed = toInt(f[3]);
        bookCount++;
    }
    fclose(fp);
}

void loadLoans(void)
{
    char line[LINE_SIZE];
    char *f[MAX_FIELDS];
    FILE *fp = fopen("loans.txt", "r");
    loanCount = 0;
    if (fp == NULL) {
        printf("Warning: loans.txt not found. Starting with no loans.\n");
        return;
    }
    while (loanCount < MAX_LOANS && fgets(line, sizeof(line), fp) != NULL) {
        Loan *l = &loans[loanCount];
        if (splitLine(line, f, MAX_FIELDS) < 7) continue;
        textCopy(l->studentId, f[0], sizeof(l->studentId));
        textCopy(l->bookId, f[1], sizeof(l->bookId));
        textCopy(l->bookTitle, f[2], sizeof(l->bookTitle));
        l->borrowDate = parseDateField(f[3]);
        l->dueDate = parseDateField(f[4]);
        l->returnDate = parseDateField(f[5]);
        l->isReturned = toInt(f[6]);
        loanCount++;
    }
    fclose(fp);
}

void loadRooms(void)
{
    char line[LINE_SIZE];
    char *f[MAX_FIELDS];
    FILE *fp = fopen("rooms.txt", "r");
    roomCount = 0;
    if (fp == NULL) {
        printf("Warning: rooms.txt not found. There are no rooms.\n");
        return;
    }
    while (roomCount < MAX_ROOMS && fgets(line, sizeof(line), fp) != NULL) {
        if (splitLine(line, f, MAX_FIELDS) < 2) continue;
        textCopy(rooms[roomCount].roomId, f[0], sizeof(rooms[roomCount].roomId));
        textCopy(rooms[roomCount].roomName, f[1], sizeof(rooms[roomCount].roomName));
        roomCount++;
    }
    fclose(fp);
}

void loadBookings(void)
{
    char line[LINE_SIZE];
    char *f[MAX_FIELDS];
    FILE *fp = fopen("bookings.txt", "r");
    bookingCount = 0;
    if (fp == NULL) {
        printf("Warning: bookings.txt not found. Starting with no bookings.\n");
        return;
    }
    while (bookingCount < MAX_BOOKINGS && fgets(line, sizeof(line), fp) != NULL) {
        Booking *b = &bookings[bookingCount];
        if (splitLine(line, f, MAX_FIELDS) < 6) continue;
        b->bookingId = toInt(f[0]);
        textCopy(b->studentId, f[1], sizeof(b->studentId));
        textCopy(b->roomId, f[2], sizeof(b->roomId));
        b->bookingDate = parseDateField(f[3]);
        b->startHour = toInt(f[4]);
        b->isActive = toInt(f[5]);
        bookingCount++;
    }
    fclose(fp);
}

void loadAllData(void)
{
    loadBooks();
    loadLoans();
    loadRooms();
    loadBookings();
    loadBookCategories();   /* unique */
    loadRatings();          /* unique */
    printf("Loaded: %d books, %d loans, %d rooms, %d bookings, %d ratings\n",
           bookCount, loanCount, roomCount, bookingCount, ratingCount);
}

/* writeDateField(): เขียนวันที่ลงไฟล์ ; ไม่มีวันที่เขียน "-" */
void writeDateField(FILE *fp, Date d)
{
    if (d.year == 0) fprintf(fp, "-");
    else fprintf(fp, "%04d-%02d-%02d", d.year, d.month, d.day);
}

/* saveBooks(): เขียน books ทั้งหมดทับ books.txt ; คืน 1 = สำเร็จ */
int saveBooks(void)
{
    int i;
    FILE *fp = fopen("books.txt", "w");
    if (fp == NULL) {
        printf("Error: cannot write books.txt\n");
        return 0;
    }
    for (i = 0; i < bookCount; i++) {
        fprintf(fp, "%s|%s|%s|%d\n", books[i].bookId, books[i].title,
                books[i].author, books[i].isBorrowed);
    }
    fclose(fp);
    return 1;
}

/* saveLoans(): เขียน loans ทั้งหมดทับ loans.txt ; คืน 1 = สำเร็จ */
int saveLoans(void)
{
    int i;
    FILE *fp = fopen("loans.txt", "w");
    if (fp == NULL) {
        printf("Error: cannot write loans.txt\n");
        return 0;
    }
    for (i = 0; i < loanCount; i++) {
        fprintf(fp, "%s|%s|%s|", loans[i].studentId, loans[i].bookId, loans[i].bookTitle);
        writeDateField(fp, loans[i].borrowDate);
        fprintf(fp, "|");
        writeDateField(fp, loans[i].dueDate);
        fprintf(fp, "|");
        if (loans[i].isReturned) writeDateField(fp, loans[i].returnDate);
        else fprintf(fp, "-");                     /* ยังไม่คืน */
        fprintf(fp, "|%d\n", loans[i].isReturned);
    }
    fclose(fp);
    return 1;
}

/* saveBookings(): เขียน bookings ทั้งหมดทับ bookings.txt (รวมที่ยกเลิกแล้วด้วย) */
int saveBookings(void)
{
    int i;
    FILE *fp = fopen("bookings.txt", "w");
    if (fp == NULL) {
        printf("Error: cannot write bookings.txt\n");
        return 0;
    }
    for (i = 0; i < bookingCount; i++) {
        fprintf(fp, "%d|%s|%s|%04d-%02d-%02d|%d|%d\n",
                bookings[i].bookingId, bookings[i].studentId, bookings[i].roomId,
                bookings[i].bookingDate.year, bookings[i].bookingDate.month,
                bookings[i].bookingDate.day, bookings[i].startHour, bookings[i].isActive);
    }
    fclose(fp);
    return 1;
}

/* =====================================================================
 * SECTION 4: เพิ่ม / ลบหนังสือ (@MaiDaiJarRuu)
 * ===================================================================== */

/* คืน index ของหนังสือที่รหัสตรงกัน ; ไม่พบคืน -1 */
int findBookIndexById(const char bookId[])
{
    int i;
    for (i = 0; i < bookCount; i++) {
        if (textEquals(books[i].bookId, bookId)) return i;
    }
    return -1;
}

/* เลขในรหัสหนังสือ เช่น "B012" -> 12 ; รูปแบบอื่นคืน 0 */
int bookIdNumber(const char bookId[])
{
    int number = 0;
    sscanf(bookId, "B%d", &number);
    return number;
}

/*
 * generateNewBookId(): รหัสใหม่ = เลขสูงสุดที่เคยใช้ + 1
 * ดูประวัติยืมและคะแนนด้วย เล่มใหม่จะได้ไม่ใช้รหัสซ้ำกับเล่มที่ลบไปแล้ว
 */
void generateNewBookId(char bookId[])
{
    int i, maxNumber = 0;
    for (i = 0; i < bookCount; i++) {
        if (bookIdNumber(books[i].bookId) > maxNumber) maxNumber = bookIdNumber(books[i].bookId);
    }
    for (i = 0; i < loanCount; i++) {
        if (bookIdNumber(loans[i].bookId) > maxNumber) maxNumber = bookIdNumber(loans[i].bookId);
    }
    for (i = 0; i < ratingCount; i++) {
        if (bookIdNumber(ratings[i].bookId) > maxNumber) maxNumber = bookIdNumber(ratings[i].bookId);
    }
    sprintf(bookId, "B%03d", maxNumber + 1);
}

/* เพิ่มหนังสือ: สร้างรหัสให้อัตโนมัติ, หนังสือใหม่พร้อมให้ยืม (isBorrowed = 0) */
void addBook(void)
{
    Book newBook;   /* เก็บชั่วคราวก่อน กรอกครบแล้วค่อยใส่ array */

    if (bookCount >= MAX_BOOKS) {
        printf("Cannot add a book: the book storage is full.\n");
        return;
    }

    generateNewBookId(newBook.bookId);
    printf("New Book ID (created automatically): %s\n\n", newBook.bookId);

    if (!readRequiredLine("Title (e.g. Clean Code): ", newBook.title, sizeof(newBook.title)) ||
        !readRequiredLine("Author (e.g. Robert C. Martin): ", newBook.author, sizeof(newBook.author))) {
        printf("Add book cancelled because input could not be read.\n");
        return;
    }

    newBook.isBorrowed = 0;           /* หนังสือใหม่ = พร้อมให้ยืม */
    books[bookCount] = newBook;       /* ช่องว่างแรกคือ index bookCount */
    bookCount++;
    saveBooks();                      /* บันทึกทันทีที่ข้อมูลเปลี่ยน */

    /* [unique] เลือกหมวดหนังสือ เพื่อใช้ในระบบแนะนำ */
    setBookCategory(newBook.bookId, chooseCategory());
    saveBookCategories();

    printf("\nBook added successfully.\n");
    printf("----------------------------------------\n");
    printf("Book ID  : %s\n", newBook.bookId);
    printf("Title    : %s\n", newBook.title);
    printf("Author   : %s\n", newBook.author);
    printf("Category : %s\n", categoryFull[getBookCategory(newBook.bookId)]);
    printf("Status   : Available\n");
    printf("----------------------------------------\n");
}

/* ลบหนังสือตามรหัส ; หนังสือที่กำลังถูกยืมลบไม่ได้ */
void deleteBook(void)
{
    char bookId[20];
    char question[320];
    int index, i;

    if (bookCount == 0) {
        printf("Cannot delete a book: there are no books in the library.\n");
        return;
    }

    if (!readBookId(bookId, sizeof(bookId))) {
        printf("Delete book cancelled because input could not be read.\n");
        return;
    }

    index = findBookIndexById(bookId);
    if (index == -1) {
        printf("Book ID '%s' was not found.\n", bookId);
        return;
    }
    if (books[index].isBorrowed) {
        printf("Cannot delete '%s' because it is currently borrowed.\n", books[index].title);
        return;
    }

    sprintf(question, "You want to delete '%s' (%s)?", books[index].title, books[index].bookId);
    if (!readYesNo(question)) {
        printf("Delete cancelled. Nothing was changed.\n");
        return;
    }

    printf("Book '%s' was deleted successfully.\n", books[index].title);

    /* เลื่อนเล่มถัดไปมาทางซ้าย 1 ช่อง ไม่ให้มีช่องว่างกลาง array */
    /* [unique] ลบหมวดของเล่มนี้ด้วย (ต้องทำก่อนเลื่อน array เพราะยังใช้ bookId อยู่) */
    removeBookCategory(books[index].bookId);
    saveBookCategories();

    for (i = index; i < bookCount - 1; i++) {
        books[i] = books[i + 1];
    }
    bookCount--;
    saveBooks();
}

/* =====================================================================
 * SECTION 5: ค้นหาหนังสือ / แสดงทั้งหมด + สรุป (@pryntz_.)
 * (โค้ดต้นฉบับใช้แค่ stdio.h อยู่แล้ว แค่เปลี่ยนไปใช้ฟังก์ชันช่วยกลาง)
 * ===================================================================== */

/* แสดงข้อมูลหนังสือ 1 เล่มตาม index */
void printBookDetails(int index)
{
    printf("Book ID : %s\n", books[index].bookId);
    printf("Title   : %s\n", books[index].title);
    printf("Author  : %s\n", books[index].author);
    {
        /* [unique] แสดงหมวด + คะแนนเฉลี่ย */
        int count;
        double avg = averageRating(books[index].bookId, &count);
        printf("Category: %s\n", categoryFull[getBookCategory(books[index].bookId)]);
        printf("Rating  : ");
        printStars(avg);
        if (count > 0) printf(" %.1f (%d votes)\n", avg, count);
        else printf(" no ratings yet\n");
    }
    if (books[index].isBorrowed == 0) {
        printf("Status  : Available\n");
    } else if (books[index].isBorrowed == 1) {
        printf("Status  : Borrowed\n");
    } else {
        printf("Status  : Invalid status\n");   /* ข้อมูลในไฟล์ผิด */
    }
    printf("----------------------------------------\n");
}

/* ค้นหา 1 = จากรหัส (ตรงตัว), 2 = จากบางส่วนของชื่อ ; คืนจำนวนที่พบ */
int searchBooks(int searchType, const char keyword[])
{
    int i, matched, found = 0;

    if (searchType != 1 && searchType != 2) {
        printf("Invalid search type. Use 1 or 2.\n");
        return -1;
    }
    if (keyword[0] == '\0') {
        printf("Please enter a search keyword.\n");
        return -1;
    }
    if (bookCount == 0) {
        printf("No books in the library.\n");
        return 0;
    }

    for (i = 0; i < bookCount; i++) {
        if (searchType == 1) matched = textEquals(books[i].bookId, keyword);
        else matched = textContains(books[i].title, keyword);

        if (matched) {
            printBookDetails(i);
            found++;
        }
    }

    if (found == 0) printf("No matching books found.\n");
    else printf("Found: %d book(s)\n", found);
    return found;
}

/* แสดงหนังสือทุกเล่ม + สรุปจำนวนทั้งหมด / พร้อมให้ยืม / ถูกยืม */
void displayAllBooksAndSummary(void)
{
    int i, available = 0, borrowed = 0, invalid = 0;

    if (bookCount == 0) printf("No books in the library.\n");

    for (i = 0; i < bookCount; i++) {
        printBookDetails(i);
        if (books[i].isBorrowed == 0) available++;
        else if (books[i].isBorrowed == 1) borrowed++;
        else invalid++;
    }

    printf("Total books : %d\n", bookCount);
    printf("Available   : %d\n", available);
    printf("Borrowed    : %d\n", borrowed);
    if (invalid > 0) printf("Invalid status: %d (please check book data)\n", invalid);
}

/* เมนูค้นหา: ถามประเภทการค้นหา แล้วถามคำค้น */
void searchBooksMenu(void)
{
    char choice[20];
    char keyword[256];
    int searchType = 0;

    while (searchType == 0) {
        printf("1. Search by book ID\n");
        printf("2. Search by title\n");
        if (!readRequiredLine("Choose (1 or 2): ", choice, sizeof(choice))) {
            printf("Search cancelled.\n");
            return;
        }
        if (textEquals(choice, "1")) searchType = 1;
        else if (textEquals(choice, "2")) searchType = 2;
        else printf("Please choose 1 or 2.\n");
    }

    if (!readRequiredLine(searchType == 1 ? "Enter book ID (e.g. B001): "
                                          : "Enter part of the title (e.g. Python): ",
                          keyword, sizeof(keyword))) {
        printf("Search cancelled.\n");
        return;
    }
    searchBooks(searchType, keyword);
}

/* =====================================================================
 * SECTION 6: ยืม / คืนหนังสือ (@หวัดดีฮาฟพี่)
 * ยืมได้สูงสุด 15 วัน นับจากวันที่ยืม
 * ===================================================================== */

/* หารายการยืมที่ยังไม่คืนของเล่มนี้ (ค้นจากท้าย = รายการล่าสุด) ; ไม่พบคืน -1 */
int findActiveLoanIndex(const char bookId[])
{
    int i;
    for (i = loanCount - 1; i >= 0; i--) {
        if (textEquals(loans[i].bookId, bookId) && loans[i].isReturned == 0) return i;
    }
    return -1;
}

/*
 * borrowBook(): รับรหัสนักศึกษา รหัสหนังสือ วันที่ยืม
 * -> ตรวจว่าหนังสือพร้อมให้ยืม -> วันที่ต้องคืน = วันที่ยืม + 15
 * -> เพิ่มรายการยืม -> หนังสือเป็นถูกยืม -> บันทึกไฟล์
 */
void borrowBook(void)
{
    char studentId[20];
    char bookId[20];
    Date borrowDate;
    Date noDate = {0, 0, 0};
    int index;
    Loan *l;


    if (loanCount >= MAX_LOANS) {
        printf("Cannot borrow: loan history is full.\n");
        return;
    }

    if (!readStudentId(STUDENT_ID_PROMPT, studentId, sizeof(studentId)) ||
        !readBookId(bookId, sizeof(bookId))) {
        printf("Borrow cancelled.\n");
        return;
    }

    /* ตรวจว่ามีหนังสือ และพร้อมให้ยืม */
    index = findBookIndexById(bookId);
    if (index == -1) {
        printf("Book ID '%s' was not found.\n", bookId);
        return;
    }
    if (books[index].isBorrowed != 0) {
        int activeLoan = findActiveLoanIndex(bookId);
        printf("Cannot borrow '%s' because it is already borrowed.\n", books[index].title);
        if (activeLoan != -1) {
            printf("Due date    : ");
            printDate(loans[activeLoan].dueDate);
            printf("\n");
        }
        return;
    }

    if (!readDate("Borrow date:", &borrowDate)) {
        printf("Borrow cancelled.\n");
        return;
    }

    /* เพิ่มรายการยืมใหม่ที่ช่อง loanCount */
    l = &loans[loanCount];
    textCopy(l->studentId, studentId, sizeof(l->studentId));
    textCopy(l->bookId, bookId, sizeof(l->bookId));
    textCopy(l->bookTitle, books[index].title, sizeof(l->bookTitle)); /* ชื่อ ณ วันที่ยืม */
    l->borrowDate = borrowDate;
    l->dueDate = addDays(borrowDate, LOAN_DAYS);   /* คำนวณวันที่ต้องคืนอัตโนมัติ */
    l->returnDate = noDate;                        /* ยังไม่คืน */
    l->isReturned = 0;
    loanCount++;

    books[index].isBorrowed = 1;                   /* เปลี่ยนสถานะเป็นถูกยืม */

    if (!saveBooks() || !saveLoans()) {
        printf("Warning: data was changed but could not be saved to file.\n");
    }

    printf("\nBorrow recorded successfully.\n");
    printf("Student ID  : %s\n", l->studentId);
    printf("Book        : %s (%s)\n", l->bookTitle, l->bookId);
    printf("Borrow date : "); printDate(l->borrowDate); printf("\n");
    printf("Due date    : "); printDate(l->dueDate); printf("  (%d days)\n", LOAN_DAYS);
}

/*
 * returnBook(): รับรหัสหนังสือ และวันที่คืนจริง
 * -> หารายการยืมที่ยังไม่คืน -> เทียบกับวันที่ต้องคืน
 * -> แจ้งตรงเวลา / เกินกำหนดกี่วัน -> หนังสือพร้อมให้ยืม -> บันทึกไฟล์
 */
void returnBook(void)
{
    char bookId[20];
    Date returnDate;
    int loanIndex, bookIndex;
    long lateDays;

    if (!readBookId(bookId, sizeof(bookId))) {
        printf("Return cancelled.\n");
        return;
    }

    loanIndex = findActiveLoanIndex(bookId);
    if (loanIndex == -1) {
        printf("Book ID '%s' is not currently borrowed.\n", bookId);
        return;
    }

    printf("Borrowed by : %s\n", loans[loanIndex].studentId);
    printf("Book        : %s\n", loans[loanIndex].bookTitle);
    printf("Borrow date : "); printDate(loans[loanIndex].borrowDate); printf("\n");
    printf("Due date    : "); printDate(loans[loanIndex].dueDate); printf("\n");

    /* วันที่คืนต้องไม่ก่อนวันที่ยืม */
    for (;;) {
        if (!readDate("Return date:", &returnDate)) {
            printf("Return cancelled.\n");
            return;
        }
        if (daysBetween(loans[loanIndex].borrowDate, returnDate) >= 0) break;
        printf("Return date cannot be before the borrow date.\n");
    }

    /* บวก = เกินกำหนด, 0 หรือลบ = ตรงเวลา */
    lateDays = daysBetween(loans[loanIndex].dueDate, returnDate);

    loans[loanIndex].returnDate = returnDate;
    loans[loanIndex].isReturned = 1;

    bookIndex = findBookIndexById(bookId);
    if (bookIndex != -1) books[bookIndex].isBorrowed = 0;   /* พร้อมให้ยืมอีกครั้ง */

    if (!saveBooks() || !saveLoans()) {
        printf("Warning: data was changed but could not be saved to file.\n");
    }

    printf("\nReturn recorded successfully.\n");
    if (lateDays > 0) printf("Result: LATE by %ld day(s).\n", lateDays);
    else printf("Result: Returned on time.\n");

    /* [unique] ให้คะแนนหนังสือ 1-5 แล้วแนะนำเล่มต่อไป */
    askRatingAfterReturn(loanIndex);
}

/* =====================================================================
 * SECTION 7: ตรวจห้องว่าง / จองห้อง (@GG)
 * เปิด 09.00-17.00 ช่วงละ 1 ชม. ช่วงสุดท้าย 16.00-17.00 (startHour 9..16)
 * ===================================================================== */

/* คืน index ของห้องที่รหัสตรงกัน ; ไม่พบคืน -1 */
int findRoomIndex(const char roomId[])
{
    int i;
    for (i = 0; i < roomCount; i++) {
        if (textEquals(rooms[i].roomId, roomId)) return i;
    }
    return -1;
}

/* สร้างรหัสการจองใหม่ = รหัสสูงสุดที่มี + 1 */
int generateNewBookingId(void)
{
    int i, maxId = 0;
    for (i = 0; i < bookingCount; i++) {
        if (bookings[i].bookingId > maxId) maxId = bookings[i].bookingId;
    }
    return maxId + 1;
}

/* readHour(): รับเวลาเริ่ม 9-16 ; คืน 1 = สำเร็จ, 0 = EOF */
int readHour(const char prompt[], int *hour)
{
    for (;;) {
        if (!readNumber(prompt, hour)) return 0;
        if (*hour >= OPEN_HOUR && *hour <= LAST_HOUR) return 1;
        printf("ข้อผิดพลาด: เปิดให้จองเฉพาะเวลา 09.00 - 17.00 น. (กรอกได้เฉพาะ 9 ถึง 16)\n");
    }
}

/* ตรวจสอบห้องว่างตามวันที่และช่วงเวลา */
void checkRoomAvailability(void)
{
    Date targetDate;
    int targetHour, i, j, availableCount = 0;

    /* [v3] หัวข้อใหญ่ใช้กรอบของ runScreen แทน */
    printf("  (ตรวจสอบห้องอ่านหนังสือว่างตามวันและเวลา)\n\n");

    if (!readDate("กรอกวันที่ต้องการตรวจสอบ:", &targetDate) ||
        !readHour("กรอกช่วงเวลาเริ่มต้นที่ต้องการ (9 ถึง 16 น. เช่น 13): ", &targetHour)) {
        return;
    }

    printf("\n--- สถานะห้องวันที่ %04d-%02d-%02d เวลา %02d:00 - %02d:00 น. ---\n",
           targetDate.year, targetDate.month, targetDate.day, targetHour, targetHour + 1);

    for (i = 0; i < roomCount; i++) {
        int isOccupied = 0;
        /* ห้องไม่ว่าง = มีการจองที่ยังไม่ยกเลิก ห้องเดียวกัน วันเดียวกัน เวลาเดียวกัน */
        for (j = 0; j < bookingCount; j++) {
            if (bookings[j].isActive == 1 &&
                textEquals(bookings[j].roomId, rooms[i].roomId) &&
                isSameDate(bookings[j].bookingDate, targetDate) &&
                bookings[j].startHour == targetHour) {
                isOccupied = 1;
                break;
            }
        }
        if (!isOccupied) {
            printf("[ ว่าง ] รหัสห้อง: %-8s | ชื่อห้อง: %s\n", rooms[i].roomId, rooms[i].roomName);
            availableCount++;
        } else {
            printf("[ไม่ว่าง] รหัสห้อง: %-8s | ชื่อห้อง: %s\n", rooms[i].roomId, rooms[i].roomName);
        }
    }

    printf("------------------------------------------------------------\n");
    printf("สรุป: มีห้องว่างพร้อมใช้งาน %d ห้อง (จากทั้งหมด %d ห้อง)\n", availableCount, roomCount);
}

/* บันทึกการจองห้อง: ห้ามห้องซ้อนเวลา และห้ามนักศึกษาคนเดียวจองหลายห้องเวลาเดียวกัน */
void bookRoom(void)
{
    char studentId[20];
    char roomId[20];
    Date targetDate;
    int startHour, roomIdx, newBookingId, i;
    Booking *b;

    if (bookingCount >= MAX_BOOKINGS) {
        printf("ข้อผิดพลาด: ข้อมูลการจองเต็มขีดจำกัดของระบบแล้ว (5000 รายการ)\n");
        return;
    }
    /* [v3] หัวข้อใหญ่ใช้กรอบของ runScreen แทน */
    printf("  (บันทึกการจองห้องอ่านหนังสือ)\n\n");

    if (!readStudentId("กรอกรหัสนักศึกษา (ตัวเลข 11 หลัก เช่น 69000000001): ", studentId, sizeof(studentId)) ||
        !readRequiredLine("กรอกรหัสห้องที่ต้องการจอง (เช่น R001): ", roomId, sizeof(roomId))) {
        return;
    }

    /* 1. รหัสห้องต้องมีอยู่จริง */
    roomIdx = findRoomIndex(roomId);
    if (roomIdx == -1) {
        printf("ข้อผิดพลาด: ไม่พบรหัสห้อง '%s' ในระบบ!\n", roomId);
        return;
    }

    /* 2. วันที่ + เวลา (readHour ตรวจช่วง 9-16 ให้แล้ว) */
    if (!readDate("กรอกวันที่ต้องการจอง:", &targetDate) ||
        !readHour("กรอกเวลาเริ่มต้นการจอง (9 ถึง 16 น. เช่น 13): ", &startHour)) {
        return;
    }

    /* 3. ตรวจการจองซ้อน (นับเฉพาะรายการที่ยังไม่ยกเลิก) */
    for (i = 0; i < bookingCount; i++) {
        if (bookings[i].isActive == 1 &&
            isSameDate(bookings[i].bookingDate, targetDate) &&
            bookings[i].startHour == startHour) {

            if (textEquals(bookings[i].roomId, roomId)) {
                printf("ข้อผิดพลาด: ห้อง %s ถูกจองแล้วในวันที่ %04d-%02d-%02d ช่วงเวลา %02d:00 - %02d:00 น.!\n",
                       roomId, targetDate.year, targetDate.month, targetDate.day,
                       startHour, startHour + 1);
                return;
            }
            if (textEquals(bookings[i].studentId, studentId)) {
                printf("ข้อผิดพลาด: นักศึกษารหัส %s ได้จองห้องอื่น (%s) ในช่วงเวลาเดียวกันไว้แล้ว!\n",
                       studentId, bookings[i].roomId);
                return;
            }
        }
    }

    /* 4. สร้างรหัสการจองอัตโนมัติ แล้วเพิ่มลง array */
    newBookingId = generateNewBookingId();
    b = &bookings[bookingCount];
    b->bookingId = newBookingId;
    textCopy(b->studentId, studentId, sizeof(b->studentId));
    textCopy(b->roomId, roomId, sizeof(b->roomId));
    b->bookingDate = targetDate;
    b->startHour = startHour;
    b->isActive = 1;
    bookingCount++;

    /* 5. บันทึกลงไฟล์ */
    if (saveBookings()) {
        printf("\n[สำเร็จ] บันทึกการจองห้องเรียบร้อยแล้ว!\n");
        printf("----------------------------------------\n");
        printf("รหัสการจอง (Booking ID) : %d\n", newBookingId);
        printf("รหัสนักศึกษา            : %s\n", studentId);
        printf("ห้องที่จอง               : %s (%s)\n", roomId, rooms[roomIdx].roomName);
        printf("วันที่จอง               : %04d-%02d-%02d\n",
               targetDate.year, targetDate.month, targetDate.day);
        printf("ช่วงเวลา                : %02d:00 - %02d:00 น.\n", startHour, startHour + 1);
        printf("สถานะ                  : ปกติ (Active)\n");
        printf("----------------------------------------\n");
    }
}

/* =====================================================================
 * SECTION 8: ค้นหายืม-คืน / ค้นหาการจอง / ยกเลิกการจอง (@NightMare)
 * ===================================================================== */

/* ค้นหารายการยืม-คืนจากรหัสนักศึกษาหรือรหัสหนังสือ (อ่านอย่างเดียว) */
void searchLoans(void)
{
    char key[50];
    int i, found = 0;

    if (!readRequiredLine("Enter student ID (e.g. 69000000001) or book ID (e.g. B001): ", key, sizeof(key))) {
        printf("Search cancelled.\n");
        return;
    }

    for (i = 0; i < loanCount; i++) {
        Loan *l = &loans[i];
        if (textEquals(l->studentId, key) || textEquals(l->bookId, key)) {
            found = 1;
            printf("\n--------------------------\n");
            printf("Student ID : %s\n", l->studentId);
            printf("Book       : %s (%s)\n", l->bookTitle, l->bookId);
            printf("Borrow date: "); printDate(l->borrowDate); printf("\n");
            printf("Due date   : "); printDate(l->dueDate); printf("\n");
            printf("Return date: ");
            if (l->isReturned) printDate(l->returnDate);
            else printf("-");                       /* ยังไม่คืน */
            printf("\n");
            printf("Status     : %s\n", l->isReturned ? "Returned" : "Not returned");
        }
    }
    if (!found) printf("Not found.\n");
}

/* ค้นหารายการจองจากรหัสนักศึกษาหรือรหัสการจอง (ที่ยกเลิกแล้วก็แสดง) */
void searchBookings(void)
{
    char key[50];
    int keyNumber = 0, keyIsNumber, i, found = 0;

    if (!readRequiredLine("Enter student ID (e.g. 69000000001) or booking ID (e.g. 1): ", key, sizeof(key))) {
        printf("Search cancelled.\n");
        return;
    }
    /* bookingId เป็น int จึงลองแปลงคำค้นเป็นตัวเลขไว้เทียบด้วย */
    keyIsNumber = parseNumber(key, &keyNumber);

    for (i = 0; i < bookingCount; i++) {
        Booking *b = &bookings[i];
        if (textEquals(b->studentId, key) || (keyIsNumber && b->bookingId == keyNumber)) {
            found = 1;
            printf("\n--------------------------\n");
            printf("Booking ID : %d\n", b->bookingId);
            printf("Student ID : %s\n", b->studentId);
            printf("Room ID    : %s\n", b->roomId);
            printf("Date       : "); printDate(b->bookingDate); printf("\n");
            printf("Time       : %02d.00-%02d.00\n", b->startHour, b->startHour + 1);
            printf("Status     : %s\n", b->isActive ? "Active" : "Cancelled");
        }
    }
    if (!found) printf("Not found.\n");
}

/* ยกเลิกการจองโดยตั้ง isActive = 0 (เก็บประวัติไว้) ช่วงเวลานั้นจึงจองใหม่ได้ */
void cancelBooking(void)
{
    int id = 0, i;

    if (bookingCount == 0) {
        printf("There are no bookings in the system.\n");
        return;
    }

    if (!readNumber("Enter booking ID to cancel (e.g. 1, 0 = go back): ", &id) || id == 0) {
        printf("Cancel stopped.\n");
        return;
    }

    for (i = 0; i < bookingCount; i++) {
        if (bookings[i].bookingId == id) {
            if (bookings[i].isActive == 0) {
                printf("This booking is already cancelled.\n");
                return;
            }
            bookings[i].isActive = 0;
            if (saveBookings()) {
                printf("Booking %d cancelled. That time slot is free now.\n", id);
            } else {
                bookings[i].isActive = 1;   /* บันทึกไม่ได้ คืนค่าเดิมให้ตรงกับไฟล์ */
                printf("Cancel failed. Nothing was changed.\n");
            }
            return;
        }
    }
    printf("Booking ID not found.\n");
}


/* =====================================================================
 * SECTION 9: หมวดหนังสือ + คะแนน (unique) — ข้อมูลและไฟล์
 * bookcats.txt  bookId|categoryId
 * ratings.txt   studentId|bookId|score
 * ===================================================================== */

/* คืนรหัสหมวดของหนังสือ ; ยังไม่ระบุหมวดคืน 0 */
int getBookCategory(const char bookId[])
{
    int i;
    for (i = 0; i < bookCatCount; i++) {
        if (textEquals(bookCats[i].bookId, bookId)) return bookCats[i].categoryId;
    }
    return 0;
}

/* ตั้งหมวดให้หนังสือ: มีอยู่แล้วให้แก้ ไม่มีให้เพิ่มต่อท้าย */
void setBookCategory(const char bookId[], int categoryId)
{
    int i;
    if (categoryId < 1 || categoryId > CATEGORY_COUNT) return;
    for (i = 0; i < bookCatCount; i++) {
        if (textEquals(bookCats[i].bookId, bookId)) {
            bookCats[i].categoryId = categoryId;
            return;
        }
    }
    if (bookCatCount < MAX_BOOKS) {
        textCopy(bookCats[bookCatCount].bookId, bookId, sizeof(bookCats[bookCatCount].bookId));
        bookCats[bookCatCount].categoryId = categoryId;
        bookCatCount++;
    }
}

/* ลบหมวดของหนังสือ (ตอนลบหนังสือ) แล้วเลื่อน array ปิดช่อง */
void removeBookCategory(const char bookId[])
{
    int i, j;
    for (i = 0; i < bookCatCount; i++) {
        if (textEquals(bookCats[i].bookId, bookId)) {
            for (j = i; j < bookCatCount - 1; j++) bookCats[j] = bookCats[j + 1];
            bookCatCount--;
            return;
        }
    }
}

/* ให้ผู้ใช้เลือกหมวด 1-20 ; อ่านไม่ได้ (EOF) คืน 0 = ไม่ระบุหมวด */
int chooseCategory(void)
{
    int choice;
    char prompt[40];
    sprintf(prompt, "  Category (1-%d, e.g. 20): ", CATEGORY_COUNT);
    printf("\n  Choose a category for this book:\n\n");
    printCategoryRange("TEXTBOOK", TEXTBOOK_FIRST, TEXTBOOK_LAST, 0);
    printCategoryRange("NON-FICTION", NONFICTION_FIRST, NONFICTION_LAST, 0);
    printCategoryRange("FICTION", FICTION_FIRST, FICTION_LAST, 0);
    for (;;) {
        if (!readNumber(prompt, &choice)) return 0;
        if (choice >= 1 && choice <= CATEGORY_COUNT) return choice;
        printf("  Please choose 1-%d.\n", CATEGORY_COUNT);
    }
}

/* คะแนนเฉลี่ยของหนังสือ ; *count = จำนวนคนที่ให้คะแนน (0 = ยังไม่มีใครให้) */
double averageRating(const char bookId[], int *count)
{
    int i, sum = 0;
    *count = 0;
    for (i = 0; i < ratingCount; i++) {
        if (textEquals(ratings[i].bookId, bookId)) {
            sum += ratings[i].score;
            (*count)++;
        }
    }
    if (*count == 0) return 0.0;
    return (double)sum / *count;
}

/* แสดงดาวแบบ ASCII เช่น 4.4 -> [****.] (ปัดเศษเป็นจำนวนเต็ม) */
void printStars(double average)
{
    int i, full = (int)(average + 0.5);
    printf("[");
    for (i = 1; i <= 5; i++) printf("%c", i <= full ? '*' : '.');
    printf("]");
}

void loadBookCategories(void)
{
    char line[LINE_SIZE];
    char *f[MAX_FIELDS];
    FILE *fp = fopen("bookcats.txt", "r");
    bookCatCount = 0;
    if (fp == NULL) return;              /* ไม่มีไฟล์ = ทุกเล่มยังไม่ระบุหมวด */
    while (bookCatCount < MAX_BOOKS && fgets(line, sizeof(line), fp) != NULL) {
        if (splitLine(line, f, MAX_FIELDS) < 2) continue;
        setBookCategory(f[0], toInt(f[1]));
    }
    fclose(fp);
}

void loadRatings(void)
{
    char line[LINE_SIZE];
    char *f[MAX_FIELDS];
    FILE *fp = fopen("ratings.txt", "r");
    ratingCount = 0;
    if (fp == NULL) return;
    while (ratingCount < MAX_RATINGS && fgets(line, sizeof(line), fp) != NULL) {
        Rating *r = &ratings[ratingCount];
        if (splitLine(line, f, MAX_FIELDS) < 3) continue;
        textCopy(r->studentId, f[0], sizeof(r->studentId));
        textCopy(r->bookId, f[1], sizeof(r->bookId));
        r->score = toInt(f[2]);
        if (r->score < 1 || r->score > 5) continue;   /* คะแนนผิดช่วง ข้าม */
        ratingCount++;
    }
    fclose(fp);
}

int saveBookCategories(void)
{
    int i;
    FILE *fp = fopen("bookcats.txt", "w");
    if (fp == NULL) {
        printf("Error: cannot write bookcats.txt\n");
        return 0;
    }
    for (i = 0; i < bookCatCount; i++) {
        fprintf(fp, "%s|%d\n", bookCats[i].bookId, bookCats[i].categoryId);
    }
    fclose(fp);
    return 1;
}

int saveRatings(void)
{
    int i;
    FILE *fp = fopen("ratings.txt", "w");
    if (fp == NULL) {
        printf("Error: cannot write ratings.txt\n");
        return 0;
    }
    for (i = 0; i < ratingCount; i++) {
        fprintf(fp, "%s|%s|%d\n", ratings[i].studentId, ratings[i].bookId, ratings[i].score);
    }
    fclose(fp);
    return 1;
}

/*
 * addOrUpdateRating(): นักศึกษาคนเดิมให้คะแนนเล่มเดิมซ้ำ = แก้คะแนนเดิม (ไม่นับ 2 เสียง)
 * ไม่เคยให้ = เพิ่มรายการใหม่
 */
void addOrUpdateRating(const char studentId[], const char bookId[], int score)
{
    int i;
    for (i = 0; i < ratingCount; i++) {
        if (textEquals(ratings[i].studentId, studentId) && textEquals(ratings[i].bookId, bookId)) {
            ratings[i].score = score;
            return;
        }
    }
    if (ratingCount < MAX_RATINGS) {
        textCopy(ratings[ratingCount].studentId, studentId, sizeof(ratings[ratingCount].studentId));
        textCopy(ratings[ratingCount].bookId, bookId, sizeof(ratings[ratingCount].bookId));
        ratings[ratingCount].score = score;
        ratingCount++;
    }
}

/* =====================================================================
 * SECTION 10: แนะนำหนังสือ (unique)
 * ===================================================================== */

/*
 * isBetterBook(): ใช้ตอนเรียงลำดับ ; คืน 1 ถ้าเล่ม a ควรอยู่ก่อนเล่ม b
 * เกณฑ์: คะแนนเฉลี่ยมากกว่า -> คนโหวตมากกว่า -> ว่างให้ยืมก่อน
 */
int isBetterBook(int a, int b)
{
    int countA, countB;
    double avgA = averageRating(books[a].bookId, &countA);
    double avgB = averageRating(books[b].bookId, &countB);
    if (avgA != avgB) return avgA > avgB;
    if (countA != countB) return countA > countB;
    return books[a].isBorrowed < books[b].isBorrowed;
}

/* sortBooks(): เรียง index หนังสือใน list จากดีที่สุดไปน้อยสุด (selection sort) */
void sortBooks(int list[], int n)
{
    int i, j, best, temp;
    for (i = 0; i < n - 1; i++) {
        best = i;
        for (j = i + 1; j < n; j++) {
            if (isBetterBook(list[j], list[best])) best = j;
        }
        temp = list[i];
        list[i] = list[best];
        list[best] = temp;
    }
}

/*
 * collectBooks(): เก็บ index หนังสือที่อยู่ในหมวด cat1 หรือ cat2 ลง list (ไม่เอา excludeId)
 * cat = 0 คือไม่ใช้ช่องนั้น ; คืนจำนวนที่เก็บได้
 */
int collectBooks(int cat1, int cat2, const char excludeId[], int list[])
{
    int i, n = 0, cat;
    for (i = 0; i < bookCount; i++) {
        cat = getBookCategory(books[i].bookId);
        if (cat == 0 || textEquals(books[i].bookId, excludeId)) continue;
        if (cat == cat1 || cat == cat2) list[n++] = i;
    }
    return n;
}

/* หัวตาราง + แถวตาราง (ชื่อหนังสือไว้คอลัมน์สุดท้าย เพราะภาษาไทยจัดความกว้างให้ตรงไม่ได้) */
void printBookTableHeader(void)
{
    printf("  No  ID     Rating            Status     Category          Title\n");
    printf("  --  -----  ----------------  ---------  ----------------  ------------------------\n");
}

void printBookRow(int rank, int index)
{
    int count;
    double avg = averageRating(books[index].bookId, &count);
    printf("  %2d  %-5s  ", rank, books[index].bookId);
    printStars(avg);
    if (count > 0) printf(" %.1f (%2d)", avg, count);
    else printf("  -  ( 0)");
    printf("  %-9s  %-16s  %s\n",
           books[index].isBorrowed ? "Borrowed" : "Available",
           categoryNames[getBookCategory(books[index].bookId)],
           books[index].title);
}

/* แสดงรายการหนังสือ (เรียงตามคะแนนแล้ว) ไม่เกิน limit เล่ม */
void printBookList(int list[], int n, int limit)
{
    int i;
    if (n == 0) {
        printf("  (no books in this category yet)\n");
        return;
    }
    printBookTableHeader();
    for (i = 0; i < n && i < limit; i++) printBookRow(i + 1, list[i]);
}

/*
 * printCategoryRange(): แสดงรายการหมวด first..last ให้เลือก
 * เลขที่แสดง = รหัสหมวด - offset (เช่น หน้า Browse ให้ Non-Fiction เริ่มที่ [1])
 */
void printCategoryRange(const char *groupName, int first, int last, int offset)
{
    int i;
    printf("  %s\n", groupName);
    for (i = first; i <= last; i++) {
        printf("    [%2d] %-27s %s\n", i - offset, categoryFull[i], categoryThai[i]);
    }
}

/* ---------- 2) แนะนำตามวิชา ---------- */

#define SUBJECT_COUNT 8

const char *subjectNames[SUBJECT_COUNT] = {
    "CPE101 Computer Programming",
    "Data Structures & Algorithms",
    "Calculus / Engineering Math",
    "Physics",
    "Digital Logic & Circuits",
    "Intro to AI / Machine Learning",
    "English & Reading",
    "Entrepreneurship"
};
/* วิชา -> หมวดหนังสือที่เกี่ยวข้อง {หมวด 1, หมวด 2} ; 0 = ไม่มี */
int subjectMap[SUBJECT_COUNT][2] = {
    {1, 0}, {1, 2}, {3, 0}, {11, 3}, {4, 0}, {2, 3}, {18, 12}, {5, 8}
};

void recommendBySubject(void)
{
    int i, choice, n;
    int list[MAX_BOOKS];

    printf("  Which subject are you studying?\n\n");
    for (i = 0; i < SUBJECT_COUNT; i++) {
        printf("    [%d] %s\n", i + 1, subjectNames[i]);
    }
    for (;;) {
        if (!readNumber("\n  Subject (1-8): ", &choice)) return;
        if (choice >= 1 && choice <= SUBJECT_COUNT) break;
        printf("  Please choose 1-%d.\n", SUBJECT_COUNT);
    }
    choice--;   /* เปลี่ยนเป็น index เริ่มที่ 0 */

    printf("\n  Books for \"%s\"", subjectNames[choice]);
    printf("  (%s", categoryFull[subjectMap[choice][0]]);
    if (subjectMap[choice][1] != 0) printf(" + %s", categoryFull[subjectMap[choice][1]]);
    printf(")\n\n");

    n = collectBooks(subjectMap[choice][0], subjectMap[choice][1], "", list);
    sortBooks(list, n);
    printBookList(list, n, 10);
}

/* ---------- 1) เลือกตามหมวด: Non-Fiction 7 หมวด + Fiction 9 หมวด ---------- */

void recommendByGenre(void)
{
    int choice, n;
    int offset = NONFICTION_FIRST - 1;                  /* รหัส 5 แสดงเป็น [1] */
    int count = FICTION_LAST - NONFICTION_FIRST + 1;    /* 16 หมวด */
    int list[MAX_BOOKS];
    char prompt[40];

    printf("  Which kind of book do you feel like reading?\n\n");
    printCategoryRange("NON-FICTION", NONFICTION_FIRST, NONFICTION_LAST, offset);
    printf("\n");
    printCategoryRange("FICTION", FICTION_FIRST, FICTION_LAST, offset);
    for (;;) {
        sprintf(prompt, "\n  Genre (1-%d, e.g. 1): ", count);
        if (!readNumber(prompt, &choice)) return;
        if (choice >= 1 && choice <= count) break;
        printf("  Please choose 1-%d.\n", count);
    }
    choice += offset;   /* 1-16 -> รหัสหมวด 5-20 */

    printf("\n  Best %s books in our library\n\n", categoryFull[choice]);
    n = collectBooks(choice, 0, "", list);
    sortBooks(list, n);
    printBookList(list, n, 10);
}

/* ---------- 3) ตาราง Top Rated + แนะนำต่อ ---------- */

void topRatedTable(void)
{
    int list[MAX_BOOKS], n = 0, i, count, shown = 0;

    /* เก็บเฉพาะเล่มที่มีคนให้คะแนนแล้ว */
    for (i = 0; i < bookCount; i++) {
        averageRating(books[i].bookId, &count);
        if (count > 0) list[n++] = i;
    }
    sortBooks(list, n);

    printf("  TOP 10 HIGHEST RATED BOOKS (from %d ratings)\n\n", ratingCount);
    if (n == 0) {
        printf("  No ratings yet. Rate a book when you return it!\n");
        return;
    }
    printBookList(list, n, 10);

    /* แนะนำต่อ: เล่มคะแนนสูงที่ว่างให้ยืมได้ตอนนี้ */
    printf("\n  >> READY TO BORROW NOW (top rated & available)\n");
    for (i = 0; i < n && shown < 3; i++) {
        if (books[list[i]].isBorrowed == 0) {
            printf("     %d. %s  %s\n", shown + 1, books[list[i]].bookId, books[list[i]].title);
            shown++;
        }
    }
    if (shown == 0) printf("     (all top rated books are borrowed right now)\n");
}

/*
 * askRatingAfterReturn(): เรียกตอนคืนหนังสือเสร็จ
 * ให้คะแนน 1-5 (0 = ข้าม) -> บันทึก ratings.txt -> แนะนำหนังสือหมวดเดียวกัน 3 เล่ม
 */
void askRatingAfterReturn(int loanIndex)
{
    int score, n, cat;
    int list[MAX_BOOKS];
    Loan *l = &loans[loanIndex];

    printf("\n  +------------------ RATE THIS BOOK ------------------+\n");
    printf("  |  How did you like it?   1 = bad ... 5 = excellent  |\n");
    printf("  +----------------------------------------------------+\n");
    for (;;) {
        if (!readNumber("  Your rating (1-5, 0 = skip): ", &score)) return;
        if (score >= 0 && score <= 5) break;
        printf("  Please type 0-5.\n");
    }

    if (score > 0) {
        addOrUpdateRating(l->studentId, l->bookId, score);
        saveRatings();
        printf("  Thanks! You gave ");
        printStars(score);
        printf(" to \"%s\"\n", l->bookTitle);
    }

    /* แนะนำต่อ: หนังสือหมวดเดียวกัน เรียงตามคะแนน */
    cat = getBookCategory(l->bookId);
    if (cat == 0) return;
    n = collectBooks(cat, 0, l->bookId, list);
    sortBooks(list, n);
    printf("\n  >> YOU MAY ALSO LIKE (%s)\n", categoryNames[cat]);
    printBookList(list, n, 3);
}

/* =====================================================================
 * SECTION 11: หน้าจอ — cls, pause, ASCII art (unique)
 * ===================================================================== */

#define BOX_WIDTH 52   /* ความกว้างด้านในของกรอบเมนู */

/* ล้างหน้าจอ: Windows ใช้ cls, Mac/Linux ใช้ clear */
void clearScreen(void)
{
    fflush(stdout);   /* ส่งข้อความที่ค้างใน buffer ออกไปก่อน ไม่งั้นอาจโผล่หลัง cls */
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* หยุดรอให้กด Enter ก่อนกลับเมนู (ไม่งั้น cls จะลบผลลัพธ์ทิ้งทันที) */
void pauseScreen(void)
{
    char dummy[100];
    printf("\n  Press Enter to continue...");
    while (readBookInput(dummy, sizeof(dummy)) == -1) { }
}

/* พิมพ์ตัวอักษร c ซ้ำ n ครั้ง (ใช้วาดเส้นกรอบ) */
void repeatChar(char c, int n)
{
    int i;
    for (i = 0; i < n; i++) printf("%c", c);
}

/* เส้นขอบกรอบ  +=====...=====+ */
void printBoxLine(void)
{
    printf("  +");
    repeatChar('=', BOX_WIDTH);
    printf("+\n");
}

/* หัวกรอบ ข้อความอยู่ตรงกลาง (ต้องเป็นภาษาอังกฤษ เพื่อให้นับความกว้างได้ถูก) */
void printBoxTitle(const char title[])
{
    int len = 0, left;
    while (title[len] != '\0') len++;
    left = (BOX_WIDTH - len) / 2;
    printBoxLine();
    printf("  |");
    repeatChar(' ', left);
    printf("%s", title);
    repeatChar(' ', BOX_WIDTH - len - left);
    printf("|\n");
    printBoxLine();
}

/* 1 บรรทัดในเมนู  |   [1]  Add book                  | */
void printMenuItem(int key, const char text[])
{
    /* "   [1]  " ยาว 8 ตัว ที่เหลือ BOX_WIDTH - 8 ช่องเป็นข้อความ (%-*s = ชิดซ้าย เติมช่องว่าง) */
    printf("  |   [%d]  %-*s|\n", key, BOX_WIDTH - 8, text);
}

/* บรรทัดว่างในกรอบ */
void printBoxEmpty(void)
{
    printf("  |");
    repeatChar(' ', BOX_WIDTH);
    printf("|\n");
}

/* โลโก้ตัวใหญ่ + ชั้นหนังสือ (หน้าเมนูหลัก) ; \\ = ตัว \ หนึ่งตัวใน string ของ C */
void printBanner(void)
{
    printf("\n");
    printf("      _     ___ ____  ____      _    ______   __\n");
    printf("     | |   |_ _| __ )|  _ \\    / \\  |  _ \\ \\ / /\n");
    printf("     | |    | ||  _ \\| |_) |  / _ \\ | |_) \\ V / \n");
    printf("     | |___ | || |_) |  _ <  / ___ \\|  _ < | |  \n");
    printf("     |_____|___|____/|_| \\_\\/_/   \\_\\_| \\_\\|_|  \n");
    printf("\n");
    printf("      .--. .---. .--.--. .----. .--. .---. .--.\n");
    printf("      |  | |   | |  |  | |    | |  | |   | |  |\n");
    printf("      |CS| |MTH| |AI|PH| |NOVL| |EE| |BIZ| |ME|\n");
    printf("      |  | |   | |  |  | |    | |  | |   | |  |\n");
    printf("      '--' '---' '--'--' '----' '--' '---' '--'\n");
    printf("    ===============================================\n");
    printf("         L I B R A R Y   M A N A G E M E N T\n\n");
}

/* =====================================================================
 * SECTION 12: เมนู + main (@rctnz)
 * ทุกหน้า: cls -> วาดเมนู -> รับเลือก -> cls -> ทำงาน -> รอ Enter -> กลับมาวาดเมนูใหม่
 * กด 0 ในเมนูย่อย = กลับเมนูหลัก ; กด 0 ในเมนูหลัก = บันทึกแล้วออก
 * ===================================================================== */

#define INPUT_EOF -1   /* อ่าน input ต่อไม่ได้แล้ว (Ctrl+Z / Ctrl+D) */

/* readChoice(): รับเลขเมนู 0-max ; คืนเลขที่เลือก, -2 = ไม่ใช่ตัวเลข, INPUT_EOF = อ่านไม่ได้ */
int readChoice(int max)
{
    char line[20];
    int result, choice;

    printf("\n  Choose (0-%d) >> ", max);
    result = readBookInput(line, sizeof(line));
    if (result == 0) return INPUT_EOF;
    if (result == -1 || !parseNumber(line, &choice)) return -2;
    return choice;
}

/*
 * runScreen(): ล้างจอ -> แสดงหัวข้อ -> เรียกฟังก์ชัน -> รอ Enter
 * รับ "ฟังก์ชัน" เป็น parameter (function pointer) จะได้ไม่ต้องเขียน 4 บรรทัดนี้ซ้ำทุกเมนู
 */
void runScreen(const char title[], void (*action)(void))
{
    clearScreen();
    printBoxTitle(title);
    printf("\n");
    action();
    pauseScreen();
}

/* แจ้งเลือกผิด แล้วรอ Enter */
void invalidChoice(int max)
{
    printf("  Invalid choice. Please choose 0-%d.\n", max);
    pauseScreen();
}

/* เมนูย่อยคืน 1 = กลับเมนูหลัก, 0 = input หมด ให้ออกโปรแกรม */

int bookMenu(void)
{
    for (;;) {
        clearScreen();
        printBoxTitle("BOOK MANAGEMENT");
        printBoxEmpty();
        printMenuItem(1, "Add book");
        printMenuItem(2, "Delete book");
        printMenuItem(3, "Search books (ID / title)");
        printMenuItem(4, "Show all books + summary");
        printBoxEmpty();
        printMenuItem(0, "Back to main menu");
        printBoxLine();
        switch (readChoice(4)) {
        case 1: runScreen("ADD BOOK", addBook); break;
        case 2: runScreen("DELETE BOOK", deleteBook); break;
        case 3: runScreen("SEARCH BOOKS", searchBooksMenu); break;
        case 4: runScreen("ALL BOOKS", displayAllBooksAndSummary); break;
        case 0: return 1;
        case INPUT_EOF: return 0;
        default: invalidChoice(4);
        }
    }
}

int loanMenu(void)
{
    for (;;) {
        clearScreen();
        printBoxTitle("BORROW / RETURN");
        printBoxEmpty();
        printMenuItem(1, "Borrow book");
        printMenuItem(2, "Return book (+ rate it 1-5)");
        printMenuItem(3, "Search loan records");
        printBoxEmpty();
        printMenuItem(0, "Back to main menu");
        printBoxLine();
        switch (readChoice(3)) {
        case 1: runScreen("BORROW BOOK", borrowBook); break;
        case 2: runScreen("RETURN BOOK", returnBook); break;
        case 3: runScreen("SEARCH LOAN RECORDS", searchLoans); break;
        case 0: return 1;
        case INPUT_EOF: return 0;
        default: invalidChoice(3);
        }
    }
}

int roomMenu(void)
{
    for (;;) {
        clearScreen();
        printBoxTitle("READING ROOMS");
        printf("  |   Open 09.00-17.00  |  1 hour/slot  |  last 16-17 |\n");
        printBoxLine();
        printBoxEmpty();
        printMenuItem(1, "Check available rooms");
        printMenuItem(2, "Book a room");
        printMenuItem(3, "Search bookings");
        printMenuItem(4, "Cancel booking");
        printBoxEmpty();
        printMenuItem(0, "Back to main menu");
        printBoxLine();
        switch (readChoice(4)) {
        case 1: runScreen("CHECK AVAILABLE ROOMS", checkRoomAvailability); break;
        case 2: runScreen("BOOK A ROOM", bookRoom); break;
        case 3: runScreen("SEARCH BOOKINGS", searchBookings); break;
        case 4: runScreen("CANCEL BOOKING", cancelBooking); break;
        case 0: return 1;
        case INPUT_EOF: return 0;
        default: invalidChoice(4);
        }
    }
}

/* [unique] เมนูแนะนำหนังสือ */
int recommendMenu(void)
{
    for (;;) {
        clearScreen();
        printBoxTitle("BOOK RECOMMENDATIONS");
        printBoxEmpty();
        printMenuItem(1, "Browse by genre");
        printMenuItem(2, "Find books by subject");
        printMenuItem(3, "Top rated books");
        printBoxEmpty();
        printMenuItem(0, "Back to main menu");
        printBoxLine();
        switch (readChoice(3)) {
        case 1: runScreen("BROWSE BY GENRE", recommendByGenre); break;
        case 2: runScreen("BOOKS BY SUBJECT", recommendBySubject); break;
        case 3: runScreen("TOP RATED BOOKS", topRatedTable); break;
        case 0: return 1;
        case INPUT_EOF: return 0;
        default: invalidChoice(3);
        }
    }
}

int main(void)
{
    int running = 1;   /* 1 = ทำงานต่อ, 0 = ออกจากโปรแกรม */
    int i, available;

#ifdef _WIN32
    system("chcp 65001 > nul");   /* ให้ console แสดงภาษาไทย (UTF-8) ได้ */
#endif
    loadAllData();     /* โหลดไฟล์ทั้งหมดเข้า array ก่อนเข้าเมนู */

    while (running) {
        /* นับหนังสือที่ว่างอยู่ เอาไว้โชว์บนหน้าเมนูหลัก */
        available = 0;
        for (i = 0; i < bookCount; i++) {
            if (books[i].isBorrowed == 0) available++;
        }

        clearScreen();
        printBanner();
        printBoxTitle("MAIN MENU");
        printBoxEmpty();
        printMenuItem(1, "Book management");
        printMenuItem(2, "Borrow / Return books");
        printMenuItem(3, "Reading room booking");
        printMenuItem(4, "Book recommendations");
        printBoxEmpty();
        printMenuItem(0, "Exit");
        printBoxLine();
        printf("    Books: %d (available %d) | Loans: %d | Ratings: %d\n",
               bookCount, available, loanCount, ratingCount);

        switch (readChoice(4)) {
        case 1: running = bookMenu(); break;
        case 2: running = loanMenu(); break;
        case 3: running = roomMenu(); break;
        case 4: running = recommendMenu(); break;
        case 0:
        case INPUT_EOF: running = 0; break;
        default: invalidChoice(4);
        }
    }

    /* ออกจากโปรแกรม: บันทึกทุกไฟล์อีกครั้งเพื่อความชัวร์ */
    saveBooks();
    saveLoans();
    saveBookings();
    saveBookCategories();
    saveRatings();

    clearScreen();
    printBanner();
    printf("        All data saved. Thank you & happy reading!\n\n");
    return 0;
}
