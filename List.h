#ifndef LIST_H
#define LIST_H

const int MAX_ISBN = 20; // ISBN最大长度
const int MAX_TITLE = 100; // 书名最大长度
const int MAX_AUTHOR = 50; // 作者名最大长度

typedef struct node{    // 单个图书结点
    char isbn[MAX_ISBN];
    char title[MAX_TITLE];
    char author[MAX_AUTHOR];
    float price;
    int stock;
    struct node *next;
}Book;

typedef struct {    // 图书链表
    Book *head;     // 头结点，不存数据
    int count;      // 图书总数
} BookList;

const int MAX_BORROWER = 50; // 借阅人姓名最大长度
const int MAX_DATE = 20;     // 借出日期字符串最大长度（YYYY-MM-DD）

typedef struct borrowNode{  // 单条借阅记录结点
    char isbn[MAX_ISBN];            // 所借图书的ISBN
    char borrower[MAX_BORROWER];    // 借阅人姓名
    char borrowDate[MAX_DATE];      // 借出日期
    struct borrowNode *next;
}BorrowRecord;

typedef struct {        // 借阅记录链表
    BorrowRecord *head; // 头结点
    int count;          // 尚未归还的借阅记录总数
} BorrowList;

void InitList(BookList *L);                             // 初始化链表
int AddBook(BookList *L, Book b);                       // 添加图书
Book* FindByIsbn(BookList *L, const char *isbn);        // 根据ISBN查找图书
int DeleteBook(BookList *L, const char *isbn);          // 根据ISBN删除图书
void ShowAll(BookList *L);                              // 显示所有图书
void DestroyList(BookList *L);                          // 销毁链表
void SortByPrice(BookList *L);                          // 按价格排序
void FindByTitle(BookList *L, const char *title);       // 根据书名模糊查找图书

void InitBorrowList(BorrowList *B);                     // 初始化借阅记录链表
int AddBorrowRecord(BorrowList *B, const char *isbn,    // 尾插法记录借阅记录
                    const char *borrower, const char *date);
BorrowRecord* FindBorrowRecord(BorrowList *B,           // 按ISBN+借阅人查找记录
                               const char *isbn, const char *borrower);
int BorrowBook(BookList *L, BorrowList *B,              // 借书：1成功 0图书不存在
               const char *isbn, const char *borrower); //       -1库存不足 -2重复借阅
int ReturnBook(BookList *L, BorrowList *B,              // 还书：1成功 0图书不存在
               const char *isbn, const char *borrower); //       -1无借阅记录
int CountBorrowByIsbn(BorrowList *B, const char *isbn); // 统计某本书的借出册数
void ShowBorrowed(BorrowList *B, BookList *L);          // 显示全部借出记录
void DestroyBorrowList(BorrowList *B);                  // 销毁借阅记录链表

#endif