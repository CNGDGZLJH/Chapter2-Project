#include <iostream>
#include <cstring>
#include <ctime>
#include "List.h"
using namespace std;
/*
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
*/
static void SwapData(Book *a, Book *b)                 // 交换两个图书结点的数据
{
    Book temp ;
    for(int i = 0 ; i < MAX_ISBN ; i++)
        temp.isbn[i] = a->isbn[i] ;
    for(int i = 0 ; i < MAX_TITLE ; i++)
        temp.title[i] = a->title[i] ;
    for(int i = 0 ; i < MAX_AUTHOR ; i++)
        temp.author[i] = a->author[i] ;
    temp.price = a->price ;
    temp.stock = a->stock ;

    for(int i = 0 ; i < MAX_ISBN ; i++)
        a->isbn[i] = b->isbn[i] ;
    for(int i = 0 ; i < MAX_TITLE ; i++)
        a->title[i] = b->title[i] ;
    for(int i = 0 ; i < MAX_AUTHOR ; i++)
        a->author[i] = b->author[i] ;
    a->price = b->price ;
    a->stock = b->stock ;

    for(int i = 0 ; i < MAX_ISBN ; i++)
        b->isbn[i] = temp.isbn[i] ;
    for(int i = 0 ; i < MAX_TITLE ; i++)
        b->title[i] = temp.title[i] ;
    for(int i = 0 ; i < MAX_AUTHOR ; i++)
        b->author[i] = temp.author[i] ;
    b->price = temp.price ;
    b->stock = temp.stock ;
}

void InitList(BookList *L)                             // 初始化链表
{
    L->head = new Book ;
    L->head->next = NULL ;
    L->count = 0 ;
}

int AddBook(BookList *L, Book b)                       // 添加图书
{
    Book *q = L->head->next ;
    while(q != NULL)
    {
        if(strcmp(q->isbn, b.isbn) == 0)
        {
            cout << "该图书已存在！" << endl ;
            return 0 ;
        }
        q = q->next ;
    }
    Book *s = new Book ;
    for(int i = 0 ; i < MAX_ISBN ; i++)
        s->isbn[i] = b.isbn[i] ;
    for(int i = 0 ; i < MAX_TITLE ; i++)
        s->title[i] = b.title[i] ;
    for(int i = 0 ; i < MAX_AUTHOR ; i++)
        s->author[i] = b.author[i] ;
    s->price = b.price ;
    s->stock = b.stock ;
    Book *p = L->head ;
    while(p->next != NULL)
        p = p->next ;
    p->next = s ;
    s->next = NULL ;
    L->count++ ;
    return 1 ;
}

static Book* FindBook(BookList *L, const char *isbn)    // 查找图书，供其他函数使用
{
    Book *p = L->head->next ;
    while(p != NULL)
    {
        if(strcmp(p->isbn, isbn) == 0)
            return p ;
        p = p->next ;
    }
    return NULL ;
}

Book* FindByIsbn(BookList *L, const char *isbn)        // 根据ISBN查找图书
{
    Book *p = FindBook(L, isbn) ;
    if(p != NULL)
        cout << "找到该图书！" << endl ;
    return p ;
}

int DeleteBook(BookList *L, const char *isbn)          // 根据ISBN删除图书
{
    Book *p = L->head->next ;
    while(p != NULL)
    {
        if(strcmp(p->isbn, isbn) == 0)
        {
            Book *q = L->head ;
            while(q->next != p)
                q = q->next ;
            q->next = p->next ;
            delete p ;
            L->count-- ;
            return 1 ;
        }
        p = p->next ;
    }
    return 0 ;
}

void ShowAll(BookList *L)                              // 显示所有图书
{
    Book *p = L->head->next ;
    if(p == NULL)
    {
        cout << "图书列表为空！" << endl ;
        return ;
    }
    cout << "图书列表如下：" << endl ;
    cout << "------------------------" << endl ;
    while(p != NULL)
    {
        cout << "ISBN: " << p->isbn << " " ;
        cout << "书名: " << p->title << " " ;
        cout << "作者: " << p->author << " " ;
        cout << "价格: " << p->price << " " ;
        cout << "库存: " << p->stock ;
        cout << endl ;
        p = p->next ;
    }
}

void DestroyList(BookList *L)                          // 销毁链表
{
    Book *p = L->head ;
    while(p != NULL)
    {
        Book *q = p ;
        p = p->next ;
        delete q ;
    }
}

void SortByPrice(BookList *L)                          // 按价格排序
{
    if(L->count <= 1)
        return ;
    for(int i = 0 ; i < L->count - 1 ; i++)
    {
        Book *p = L->head->next ;
        for(int j = 0 ; j < L->count - 1 ; j++)
        {
            if(p->next == NULL)
                break ;
            if(p->price > p->next->price)
            {
                SwapData(p , p->next) ;
            }
            p = p->next ;
        }
    }
}

void FindByTitle(BookList *L, const char *title)       // 根据书名模糊查找图书
{
    Book *p = L->head->next ;
    while(p != NULL)
    {
        if(strstr(p->title, title) != NULL)
        {
            cout << "找到包含该书名的图书！" << endl ;
            cout << "ISBN: " << p->isbn << " " ;
            cout << "书名: " << p->title << " " ;
            cout << "作者: " << p->author << " " ;
            cout << "价格: " << p->price << " " ;
            cout << "库存: " << p->stock ;
            cout << endl ;
        }
        p = p->next ;
    }
}

// 下面是借阅记录相关函数的实现

static void GetToday(char *out , int cap)                                                               // 取当天日期，格式 YYYY-MM-DD
{
    out[0] = '\0' ;
    time_t t = time(NULL) ;
    struct tm *lt = localtime(&t) ;
    if(lt == NULL || strftime(out , (size_t)cap , "%Y-%m-%d" , lt) == 0)
    {
        strncpy(out , "0000-00-00" , (size_t)cap - 1) ;
        out[cap - 1] = '\0' ;
    }
}

void InitBorrowList(BorrowList *B)                                                                      // 初始化借阅记录链表
{
    B->head = new BorrowRecord ;
    B->head->next = NULL ;
    B->count = 0 ;
}

BorrowRecord* FindBorrowRecord(BorrowList *B , const char *isbn , const char *borrower)                 // 按ISBN+借阅人查找借阅记录
{
    BorrowRecord *q = B->head->next ;
    while(q != NULL)
    {
        if(strcmp(q->isbn, isbn) == 0 && strcmp(q->borrower, borrower) == 0)
            return q ;
        q = q->next ;
    }
    return NULL ;
}

int AddBorrowRecord(BorrowList *B , const char *isbn , const char *borrower , const char *date)         // 尾插法记录借阅记录
{
    if(FindBorrowRecord(B, isbn, borrower) != NULL)
        return 0 ;                                     
    BorrowRecord *s = new BorrowRecord ;
    memset(s , 0 , sizeof(BorrowRecord)) ;              
    strncpy(s->isbn , isbn , MAX_ISBN - 1) ;
    strncpy(s->borrower , borrower , MAX_BORROWER - 1) ;
    strncpy(s->borrowDate , date , MAX_DATE - 1) ;
    s->next = NULL ;
    BorrowRecord *q = B->head ;
    while(q->next != NULL)                             
        q = q->next ;
    q->next = s ;
    B->count++ ;
    return 1 ;
}

int BorrowBook(BookList *L , BorrowList *B , const char *isbn , const char *borrower)                   // 借书：1成功 0图书不存在 -1库存不足 -2重复借阅
{
    Book *p = FindBook(L, isbn) ;
    if(p == NULL)
        return 0 ;                                     
    if(FindBorrowRecord(B, isbn, borrower) != NULL)
        return -2 ;                                    // 同一人已借且未还
    if(p->stock <= 0)
        return -1 ;                                    // 在馆数量为 0，已全部借出

    char today[MAX_DATE] ;
    GetToday(today, MAX_DATE) ;
    if(AddBorrowRecord(B, isbn, borrower, today) == 0)
        return -2 ;

    p->stock-- ;                                       // 借出成功：在馆数量减一
    return 1 ;
}

int ReturnBook(BookList *L , BorrowList *B , const char *isbn , const char *borrower)                   // 还书：1成功 0图书不存在 -1无借阅记录
{
    Book *p = FindBook(L, isbn) ;
    if(p == NULL)
        return 0 ;                                    
    BorrowRecord *q = B->head ;                        
    while(q->next != NULL &&
          !(strcmp(q->next->isbn, isbn) == 0 && strcmp(q->next->borrower, borrower) == 0))
        q = q->next ;
    if(q->next == NULL)
        return -1 ;                                    
    BorrowRecord *r = q->next ;
    q->next = r->next ;
    delete r ;                                         
    B->count-- ;
    p->stock++ ;                                       
    return 1 ;
}

int CountBorrowByIsbn(BorrowList *B , const char *isbn)                                                 // 统计某本书的借出册数
{
    int n = 0 ;
    BorrowRecord *q = B->head->next ;
    while(q != NULL)
    {
        if(strcmp(q->isbn, isbn) == 0)
            n++ ;
        q = q->next ;
    }
    return n ;
}

void ShowBorrowed(BorrowList *B , BookList *L)                                                          // 显示全部借出记录
{
    BorrowRecord *q = B->head->next ;
    if(q == NULL)
    {
        cout << "当前没有未归还的借阅记录！" << endl ;
        return ;
    }
    cout << "当前借出记录如下（共 " << B->count << " 条）：" << endl ;
    cout << "------------------------" << endl ;
    while(q != NULL)
    {
        Book *p = FindBook(L, q->isbn) ;              
        cout << "ISBN: " << q->isbn << " " ;
        cout << "书名: " << (p != NULL ? p->title : "（图书已不存在）") << " " ;
        cout << "借阅人: " << q->borrower << " " ;
        cout << "借出日期: " << q->borrowDate ;
        cout << endl ;
        q = q->next ;
    }
}

void DestroyBorrowList(BorrowList *B)                                                                   // 销毁借阅记录链表
{
    BorrowRecord *q = B->head ;
    while(q != NULL)
    {
        BorrowRecord *r = q ;
        q = q->next ;
        delete r ;
    }
}

