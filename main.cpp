#include <iostream>
#include <cstdio>
#include <cstring>
#include <windows.h>    // 用于设置控制台输出为UTF-8编码
#include "List.h"

using namespace std;

int main() {
    SetConsoleOutputCP(CP_UTF8) ; // 设置控制台输出为UTF-8编码
    SetConsoleCP(CP_UTF8) ;       // 设置控制台输入为UTF-8编码

    BookList L ;
    InitList(&L) ;
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
                cout << "库存: " << p->stock << " " ;
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
            if(DeleteBook(&L, isbn))
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
            // 待实现
        }
        else if(op == 6)    // 按书名模糊查找
        {
            // 待实现
        }
        else if(op == 7)    // 按价格排序
        {
            SortByPrice(&L) ;
        }
        else if(op == 8)    // 统计信息
        {
            cout << "图书总数：" << L.count << endl ;
            Book *p = L.head->next ;
            float totalPrice = 0 ;
            int totalStock = 0 ;
            while(p != NULL)
            {
                totalPrice += p->price ;
                totalStock += p->stock ;
                p = p->next ;
            }
            cout << "图书总价格：" << totalPrice << endl ;
            cout << "图书总库存：" << totalStock << endl ;
        }
        else if(op == 9)    // 保存到文件
        {
            FILE *f = fopen("books.txt", "w") ;
            Book *p = L.head->next ;
            while(p != NULL)
            {
                fprintf(f, "%s %s %s %.2f %d\n", p->isbn, p->title, p->author, p->price, p->stock) ;
                p = p->next ;
            }
            fclose(f) ;
        }
        else
        {
            cout << "无效的选择，请重新输入！" << endl ;
        }

        cout << "请输入你的选择：" << endl ;
        cin >> op ;
    }

    DestroyList(&L) ;

    return 0 ;    
}
