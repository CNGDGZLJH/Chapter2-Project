#include <iostream>
#include <cstdio>
#include <cstring>
#include <windows.h>    // 用于设置控制台输出为UTF-8编码
#include "List.h"

using namespace std;

/* 与 FindByIsbn 功能相同，但不打印"找到该图书！"，供菜单输出友好提示使用 */
static Book* FindBookQuiet(BookList *L, const char *isbn)
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

int main() {
    SetConsoleOutputCP(CP_UTF8) ; // 设置控制台输出为UTF-8编码
    SetConsoleCP(CP_UTF8) ;       // 设置控制台输入为UTF-8编码

    BookList L ;
    InitList(&L) ;

    BorrowList B ;
    InitBorrowList(&B) ;

    FILE *f = fopen("books.txt", "r") ;     // 从目前图书数据库中加载文件
    if(f != NULL)
    {
        
        Book *p = new Book ;
        while(fscanf(f, "%s %s %s %f %d", p->isbn, p->title, p->author, &p->price, &p->stock) == 5)
        {
            AddBook(&L, *p) ;
        }
        delete p ;        
        fclose(f) ;
        cout << "已从文件中加载图书数据！" << endl << endl ;
    }

    f = fopen("borrow.txt", "r") ;          // 加载尚未归还的借阅记录
    if(f != NULL)                           // 宽度与 MAX_ISBN/MAX_BORROWER/MAX_DATE 对应
    {
        char isbn[MAX_ISBN] ;
        char borrower[MAX_BORROWER] ;
        char date[MAX_DATE] ;
        int n = 0 ;
        while(fscanf(f, "%19s %49s %19s", isbn, borrower, date) == 3)
        {
            if(AddBorrowRecord(&B, isbn, borrower, date))
                n++ ;
        }
        fclose(f) ;
        if(n > 0)
            cout << "已从文件中加载 " << n << " 条借阅记录！" << endl << endl ;
    }

    cout << "========== 图书管理系统 ==========" << endl 
        << "1. 添加图书" << endl 
        << "2. 查找图书（按ISBN）" << endl 
        << "3. 删除图书" << endl 
        << "4. 显示全部图书" << endl 
        << "5. 修改图书信息" << endl 
        << "6. 按书名模糊查找" << endl 
        << "7. 按价格排序" << endl 
        << "8. 统计信息" << endl 
        << "9. 保存到文件" << endl 
        << "10. 借书" << endl 
        << "11. 还书" << endl 
        << "12. 查看借出记录" << endl 
        << "0. 退出系统" << endl 
        << "===============================" << endl 
        << "请输入你的选择：" << endl ;

    int op ;
    cin >> op ;

    while(op != 0)
    {
        if(op == 1)         // 添加图书
        {
            Book *p = new Book ;
            cin >> p->isbn >> p->title >> p->author >> p->price >> p->stock ;
            AddBook(&L, *p) ;
            delete p ;
        }
        else if(op == 2)    // 查找图书（按ISBN）
        {
            char isbn[MAX_ISBN] ;
            cout << "请输入ISBN：" ;
            cin >> isbn ;
            Book *p = FindByIsbn(&L, isbn) ;
            if(p != NULL)
            {
                cout << "ISBN: " << p->isbn << " " ;
                cout << "书名: " << p->title << " " ;
                cout << "作者: " << p->author << " " ;
                cout << "价格: " << p->price << " " ;
                cout << "库存: " << p->stock ;
                cout << endl ;
            }
            else
                cout << "未找到该图书！" << endl ;
        }
        else if(op == 3)    // 删除图书
        {
            char isbn[MAX_ISBN] ;
            cout << "请输入ISBN：" ;
            cin >> isbn ;
            int borrowed = CountBorrowByIsbn(&B, isbn) ;   // 还有外借就不能删除
            if(borrowed > 0)
                cout << "该书尚有 " << borrowed << " 本未归还，请先办理归还后再删除！" << endl ;
            else if(DeleteBook(&L, isbn))
                cout << "删除成功！" << endl ;
            else
                cout << "未找到该图书，删除失败！" << endl ;
        }
        else if(op == 4)    // 显示全部图书
        {
            ShowAll(&L) ;
        }
        else if(op == 5)    // 修改图书信息
        {
            int choice ;
            cout << "请输入要修改的图书的ISBN：" ;
            char isbn[MAX_ISBN] ;
            cin >> isbn ;
            cout << "请选择要修改的信息的序号：" << endl 
                << "1. 书名" << endl 
                << "2. 作者" << endl 
                << "3. 价格" << endl 
                << "4. 库存" << endl ;
            cin >> choice ;
            Book *p = FindByIsbn(&L, isbn) ;
            if(p != NULL)
            {
                if(choice == 1)
                {
                    cout << "请输入新的书名：" ;
                    cin >> p->title ;
                }
                else if(choice == 2)
                {
                    cout << "请输入新的作者：" ;
                    cin >> p->author ;
                }
                else if(choice == 3)
                {
                    cout << "请输入新的价格：" ;
                    cin >> p->price ;
                }
                else if(choice == 4)
                {
                    cout << "请输入新的库存：" ;
                    cin >> p->stock ;
                }
                else
                {
                    cout << "无效的选择！" << endl ;
                }
            }
            else
            {
                cout << "未找到该图书！" << endl ;
            }
        }
        else if(op == 6)    // 按书名模糊查找
        {
            char title[MAX_TITLE] ;
            cout << "请输入书名关键字：" ;
            cin >> title ;
            FindByTitle(&L, title) ;
        }
        else if(op == 7)    // 按价格排序
        {
            SortByPrice(&L) ;
            cout << "按价格排序后的图书列表：" << endl ;
            ShowAll(&L) ;
        }
        else if(op == 8)    // 统计信息
        {
            cout << "图书总数：" << L.count << endl ;
            Book *p = L.head->next ;
            float totalPrice = 0 ;
            int totalStock = 0 ;
            while(p != NULL)
            {
                totalPrice += p->price * p->stock ;
                totalStock += p->stock ;
                p = p->next ;
            }
            cout << "图书总价值：" << totalPrice << endl ;
            cout << "图书总库存：" << totalStock << endl ;
        }
        else if(op == 9)    // 保存到文件
        {
            FILE *f = fopen("books.txt", "w") ;
            if(f == NULL)
            {
                cout << "保存失败：无法写入 books.txt！" << endl ;
            }
            else
            {
                Book *p = L.head->next ;
                while(p != NULL)
                {
                    fprintf(f, "%s %s %s %.2f %d\n", p->isbn, p->title, p->author, p->price, p->stock) ;
                    p = p->next ;
                }
                fclose(f) ;
                cout << "图书数据已保存到 books.txt（共 " << L.count << " 条）。" << endl ;
            }

            f = fopen("borrow.txt", "w") ;      // 保存借阅记录
            if(f == NULL)
            {
                cout << "保存失败：无法写入 borrow.txt！" << endl ;
            }
            else
            {
                BorrowRecord *q = B.head->next ;
                while(q != NULL)
                {
                    fprintf(f, "%s %s %s\n", q->isbn, q->borrower, q->borrowDate) ;
                    q = q->next ;
                }
                fclose(f) ;
                cout << "借阅记录已保存到 borrow.txt（共 " << B.count << " 条）。" << endl ;
            }
        }
        else if(op == 10)   // 借书
        {
            char isbn[MAX_ISBN] ;
            char borrower[MAX_BORROWER] ;
            cout << "请输入要借阅的图书ISBN：" ;
            cin >> isbn ;
            cout << "请输入借阅人姓名：" ;
            cin >> borrower ;

            int r = BorrowBook(&L, &B, isbn, borrower) ;
            if(r == 1)
            {
                Book *p = FindBookQuiet(&L, isbn) ;
                cout << "借阅成功！《" << (p != NULL ? p->title : isbn) << "》已借给 "
                     << borrower << "，该书当前在馆 " << (p != NULL ? p->stock : 0) << " 本。" << endl ;
            }
            else if(r == 0)
                cout << "借阅失败：未找到该图书！" << endl ;
            else if(r == -1)
                cout << "借阅失败：该书已全部借出，在馆数量为 0！" << endl ;
            else
                cout << "借阅失败：" << borrower << " 已借该书且尚未归还，不能重复借阅！" << endl ;
        }
        else if(op == 11)   // 还书
        {
            char isbn[MAX_ISBN] ;
            char borrower[MAX_BORROWER] ;
            cout << "请输入要归还的图书ISBN：" ;
            cin >> isbn ;
            cout << "请输入借阅人姓名：" ;
            cin >> borrower ;

            int r = ReturnBook(&L, &B, isbn, borrower) ;
            if(r == 1)
            {
                Book *p = FindBookQuiet(&L, isbn) ;
                cout << "归还成功！《" << (p != NULL ? p->title : isbn) << "》已入库，该书当前在馆 "
                     << (p != NULL ? p->stock : 0) << " 本。" << endl ;
            }
            else if(r == 0)
                cout << "归还失败：未找到该图书！" << endl ;
            else
                cout << "归还失败：未找到 " << borrower << " 借阅 ISBN " << isbn << " 的记录！" << endl ;
        }
        else if(op == 12)   // 查看借出记录
        {
            ShowBorrowed(&B, &L) ;
        }
        else
        {
            cout << "无效的选择，请重新输入！" << endl ;
        }

        cout << "请输入你的选择：" << endl ;
        cin >> op ;
    }

    DestroyList(&L) ;
    DestroyBorrowList(&B) ;

    return 0 ;    
}
