#include <iostream>
#include <cstring>
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
Book* FindByIsbn(BookList *L, const char *isbn)        // 根据ISBN查找图书
{
    Book *p = L->head->next ;
    while(p != NULL)
    {
        if(strcmp(p->isbn, isbn) == 0)
        {
            cout << "找到该图书！" << endl ;
            return p ;
        }
        p = p->next ;
    }
    return NULL ;
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
        cout << "库存: " << p->stock << " " ;
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
        Book *q = p->next ;
        for(int j = i + 1 ; j < L->count - 1 ; j++)
        {
            if(p->price > q->price)
            {
                Book temp = *p ;
                *p = *q ;
                *q = temp ;
            }
            p = q ;
            q = q->next ;
        }
    }
}