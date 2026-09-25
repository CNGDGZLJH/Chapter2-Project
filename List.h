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

void InitList(BookList *L);                             // 初始化链表
int AddBook(BookList *L, Book b);                       // 添加图书
Book* FindByIsbn(BookList *L, const char *isbn);        // 根据ISBN查找图书
int DeleteBook(BookList *L, const char *isbn);          // 根据ISBN删除图书
void ShowAll(BookList *L);                              // 显示所有图书
void DestroyList(BookList *L);                          // 销毁链表
void SortByPrice(BookList *L);                          // 按价格排序

#endif