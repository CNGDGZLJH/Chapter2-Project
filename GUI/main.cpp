/* ============================================================================
 *  main.cpp  ——  图书管理系统 Windows 图形界面版
 *
 *  设计要点：
 *    - 业务逻辑复用 func.cpp / List.h（单链表增删查改手写实现，未做改动）
 *    - 本文件只负责界面与交互
 *    - 用 Unicode API + L"..." 宽字符串，中文不会乱码
 *    - 布局分左右两栏：左侧图书列表，右侧表单 + 按钮，WM_SIZE 里统一重排
 *      （关键：列表不能覆盖右侧栏，否则按钮收不到鼠标点击）
 *
 *  编译： g++ -std=c++17 -O2 -municode -mwindows *.cpp -o book_gui.exe
 *         -lcomctl32 -lgdi32
 * ==========================================================================*/

#define WIN32_LEAN_AND_MEAN
#define _WIN32_IE 0x0600

#include <windows.h>
#include <windowsx.h>       /* GET_X_LPARAM / GET_Y_LPARAM */
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include "List.h"



/* ---------- 控件 ID ---------- */
#define ID_LIST        1001
#define ID_EDIT_ISBN   1002
#define ID_EDIT_TITLE  1003
#define ID_EDIT_AUTHOR 1004
#define ID_EDIT_PRICE  1005
#define ID_EDIT_STOCK  1006
#define ID_BTN_ADD     1100
#define ID_BTN_DEL     1101
#define ID_BTN_QUERY   1102
#define ID_BTN_MODIFY  1103
#define ID_BTN_SEARCH  1104
#define ID_BTN_SORT    1105
#define ID_BTN_STAT    1106
#define ID_BTN_SAVE    1107
#define ID_BTN_LOAD    1108
#define ID_BTN_CLEAR   1109
#define ID_BTN_SHOWALL 1110
#define ID_STATUS      1200

/* ---------- 控件句柄（供布局使用） ---------- */
typedef struct { HWND h; int id; } Ctl;
static Ctl g_edits[5];      /* ISBN, 书名, 作者, 价格, 库存 */
static Ctl g_labels[5];
static Ctl g_btns[11];
static HWND g_lblFormTitle = NULL;
static HWND g_lblListTitle = NULL;

/* ---------- 全局状态 ---------- */
static HINSTANCE g_hInst = NULL;
static HWND      g_hMain = NULL;
static HWND      g_hList = NULL;
static HWND      g_hStatus = NULL;
static BookList  g_list;
static HFONT     g_hFont  = NULL;
static HFONT     g_hFontTitle = NULL;

/* ================= 深色科技风主题 ================= */
#define CLR_WINDOW     RGB(0x14, 0x19, 0x26)
#define CLR_FIELD      RGB(0x0E, 0x14, 0x20)
#define CLR_TEXT       RGB(0xE4, 0xEC, 0xF8)
#define CLR_TEXT_DIM   RGB(0x8A, 0x9B, 0xB5)
#define CLR_ACCENT     RGB(0x22, 0xD3, 0xEE)
#define CLR_ACCENT_DK  RGB(0x1B, 0xA8, 0xC8)
#define CLR_BTN        RGB(0x23, 0x2C, 0x40)
#define CLR_BTN_HOT    RGB(0x2E, 0x3A, 0x54)
#define CLR_BTN_DOWN   RGB(0x18, 0x20, 0x30)
#define CLR_BORDER     RGB(0x2C, 0x38, 0x50)

#define ID_BTN_FIRST   1100
#define ID_BTN_LAST    1110

#ifndef HDM_SETBKCOLOR
#define HDM_SETBKCOLOR   (HDM_FIRST + 1)
#endif
#ifndef HDM_SETTEXTCOLOR
#define HDM_SETTEXTCOLOR (HDM_FIRST + 2)
#endif

static HBRUSH g_brWindow = NULL;
static HBRUSH g_brField  = NULL;
static int g_hotBtn     = -1;
static int g_pressedBtn = -1;



typedef HRESULT (WINAPI *PFN_SetWindowTheme)(HWND, LPCWSTR, LPCWSTR);
static PFN_SetWindowTheme pSetWindowTheme = NULL;

static int GetBtnRectForId(int id, RECT *rc)
{
    HWND h = GetDlgItem(g_hMain, id);
    if (h == NULL || rc == NULL) return 0;
    GetWindowRect(h, rc);
    MapWindowPoints(NULL, g_hMain, (LPPOINT)rc, 2);
    return 1;
}

static void DrawFlatButton(LPDRAWITEMSTRUCT dis)
{
    int id;
    int isHot, isPressed, isFocus;
    COLORREF fill;
    HDC hdc, mem;
    RECT rc, r0;
    int w, h;
    HBITMAP bmp, oldBmp;
    HBRUSH br;
    HPEN pen, oldPen;
    HBRUSH oldBr;
    wchar_t text[128];
    HFONT oldFont;
    RECT rt;

    if (g_hFont == NULL || dis == NULL) return;
    hdc = dis->hDC;
    if (hdc == NULL) return;

    id        = (int)dis->CtlID;
    isHot     = (id == g_hotBtn);
    isPressed = (id == g_pressedBtn) || ((dis->itemState & ODS_SELECTED) != 0);
    isFocus   = (dis->itemState & ODS_FOCUS) != 0;

    fill = CLR_BTN;
    if (isPressed)     fill = CLR_BTN_DOWN;
    else if (isHot)    fill = CLR_BTN_HOT;

    rc = dis->rcItem;
    w = rc.right - rc.left;
    h = rc.bottom - rc.top;
    if (w <= 0 || h <= 0) return;

    mem = CreateCompatibleDC(hdc);
    if (mem == NULL) return;
    bmp = CreateCompatibleBitmap(hdc, w, h);
    if (bmp == NULL) { DeleteDC(mem); return; }
    oldBmp = (HBITMAP)SelectObject(mem, bmp);

    r0.left = 0; r0.top = 0; r0.right = w; r0.bottom = h;
    br = CreateSolidBrush(fill);
    FillRect(mem, &r0, br);
    DeleteObject(br);

    rt.left = 0; rt.top = 0; rt.right = w; rt.bottom = 1;
    br = CreateSolidBrush(isPressed ? CLR_BORDER : CLR_BTN_HOT);
    FillRect(mem, &rt, br);
    DeleteObject(br);

    pen = CreatePen(PS_SOLID, 1, (isHot || isFocus) ? CLR_ACCENT : CLR_BORDER);
    oldPen = (HPEN)SelectObject(mem, pen);
    oldBr = (HBRUSH)SelectObject(mem, GetStockObject(NULL_BRUSH));
    Rectangle(mem, 0, 0, w, h);
    SelectObject(mem, oldPen);
    SelectObject(mem, oldBr);
    DeleteObject(pen);

    text[0] = L'\0';
    GetWindowTextW(dis->hwndItem, text, 128);
    SetBkMode(mem, TRANSPARENT);
    SetTextColor(mem, (isHot || isPressed) ? CLR_ACCENT : CLR_TEXT);

    oldFont = (HFONT)SelectObject(mem, g_hFont);
    rt = r0;
    if (isPressed) OffsetRect(&rt, 0, 1);
    DrawTextW(mem, text, -1, &rt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(mem, oldFont);

    BitBlt(hdc, 0, 0, w, h, mem, 0, 0, SRCCOPY);

    SelectObject(mem, oldBmp);
    DeleteObject(bmp);
    DeleteDC(mem);
}

static void ApplyDarkTheme(void)
{
    if (g_hList == NULL) return;

    /* 注意：绝不要在此对表头发送 HDM_SETBKCOLOR / HDM_SETTEXTCOLOR。
       实测在本机 MinGW + ComCtl32 组合下发送这两个消息会让表头控件内部崩溃
       （0xC0000005）。表头保持系统样式，改用深色列表主体 + 深色窗口背景，
       整体观感已足够统一。 */
    if (pSetWindowTheme != NULL)
        pSetWindowTheme(g_hList, L"DarkMode_Explorer", NULL);
}

static void ApplyDarkTitleBar(HWND hwnd)
{
    typedef HRESULT (WINAPI *PFN_Dwm)(HWND, DWORD, LPCVOID, DWORD);
    HMODULE hDwm = LoadLibraryW(L"dwmapi.dll");
    PFN_Dwm pDwm;
    BOOL dark = TRUE;

    if (hDwm == NULL) return;
    pDwm = (PFN_Dwm)(void *)GetProcAddress(hDwm, "DwmSetWindowAttribute");
    if (pDwm == NULL) return;

    if (FAILED(pDwm(hwnd, 20, &dark, sizeof(dark))))
        pDwm(hwnd, 19, &dark, sizeof(dark));
}





/* ---------- 控件取值 ---------- */
static void SetEditText(HWND h, const wchar_t *s)
{
    SetWindowTextW(h, (s != NULL) ? s : L"");
}

static void GetEditText(HWND h, char *out, int cap)
{
    if (out == NULL || cap <= 0) return;
    out[0] = '\0';
    if (h == NULL) return;

    /* 用 Unicode 读取再转 UTF-8：比 GetWindowTextA 可靠。
       后者依赖本地 ANSI 代码页，遇到无法映射的字符可能整体转换失败而返回空串 */
    wchar_t w[512];
    w[0] = L'\0';
    int n = (int)SendMessageW(h, WM_GETTEXT, (WPARAM)512, (LPARAM)w);
    if (n <= 0) return;
    w[511] = L'\0';
    WideCharToMultiByte(CP_UTF8, 0, w, -1, out, cap, NULL, NULL);
    out[cap - 1] = '\0';
}

static float GetEditFloat(HWND h, float def)
{
    wchar_t buf[64];
    buf[0] = L'\0';
    GetWindowTextW(h, buf, 64);
    if (buf[0] == L'\0') return def;
    return (float)_wtof(buf);
}

static int GetEditInt(HWND h, int def)
{
    wchar_t buf[64];
    buf[0] = L'\0';
    GetWindowTextW(h, buf, 64);
    if (buf[0] == L'\0') return def;
    return (int)_wtoi(buf);
}

static HWND EditByIndex(int i)
{
    return (i >= 0 && i < 5) ? g_edits[i].h : NULL;
}
/* ---------- 状态栏提示（替代模态对话框，不阻塞操作）---------- */
static void SetStatus(const wchar_t *fmt, ...)
{
    wchar_t buf[512];
    va_list ap;
    va_start(ap, fmt);
    _vsnwprintf(buf, 512, fmt, ap);
    va_end(ap);
    buf[511] = L'\0';
    if (g_hStatus != NULL) SetWindowTextW(g_hStatus, buf);
}

/* ---------- 布局 ---------- */

/* 取得程序所在目录，拼出 books.txt 完整路径 */
static void GetBooksPath(wchar_t *full, int cap)
{
    wchar_t dir[MAX_PATH];
    dir[0] = L'\0';
    GetModuleFileNameW(NULL, dir, MAX_PATH);
    wchar_t *slash = wcsrchr(dir, L'\\');
    if (slash != NULL) *(slash + 1) = L'\0';
    _snwprintf(full, (size_t)cap, L"%lsbooks.txt", dir);
    full[cap - 1] = L'\0';
}

/* ---------- 文件读写 ---------- */
static int SaveToFile(const wchar_t *path)
{
    /* 注意：不要用 _wfopen(path, L"w, ccs=UTF-8")。
       本机 MinGW UCRT64 下该模式有缺陷：只写出 BOM，
       之后所有 fprintf 都返回 -1，数据全部丢失（已实测验证）。
       改为直接写 UTF-8 字节，效果相同且可靠。 */
    FILE *f = _wfopen(path, L"w");
    if (f == NULL) return 0;

    Book *p = g_list.head->next;
    while (p != NULL)
    {
        fprintf(f, "%s %s %s %.2f %d\n",
                p->isbn, p->title, p->author, p->price, p->stock);
        p = p->next;
    }
    fclose(f);
    return 1;
}
static int LoadFromFile(const wchar_t *path)
{
    /* 同样避开 ccs=UTF-8；直接读字节，若带 UTF-8 BOM 则跳过 */
    FILE *f = _wfopen(path, L"rb");
    if (f == NULL) return 0;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) { fclose(f); return 0; }

    char *buf = (char *)malloc((size_t)size + 1);
    if (buf == NULL) { fclose(f); return 0; }

    int n = (int)fread(buf, 1, (size_t)size, f);
    fclose(f);
    if (n < 0) n = 0;
    buf[n] = '\0';

    /* 跳过可能存在的 UTF-8 BOM */
    char *p0 = buf;
    if (n >= 3 && (unsigned char)p0[0] == 0xEF &&
                  (unsigned char)p0[1] == 0xBB &&
                  (unsigned char)p0[2] == 0xBF)
        p0 += 3;

    int loaded = 0;
    char *line = strtok(p0, "\r\n");
    while (line != NULL)
    {
        Book b;
        memset(&b, 0, sizeof(Book));
        if (sscanf(line, "%19s %99s %49s %f %d",
                   b.isbn, b.title, b.author, &b.price, &b.stock) == 5)
        {
            if (AddBook(&g_list, b) == 1) loaded++;
        }
        line = strtok(NULL, "\r\n");
    }

    free(buf);
    return loaded;
}
/* ---------- 列表 ---------- */
static void SetSubItem(int row, int col, const wchar_t *text)
{
    LVITEMW it;
    ZeroMemory(&it, sizeof(it));
    it.mask     = LVIF_TEXT;
    it.iItem    = row;
    it.iSubItem = col;
    it.pszText  = (LPWSTR)text;
    SendMessageW(g_hList, LVM_SETITEMTEXTW, (WPARAM)row, (LPARAM)&it);
}

/* 刷新列表；filter 非空时只显示书名包含 filter 的图书（模糊查找用） */
static void RefreshListFiltered(const char *filter)
{
    SendMessageW(g_hList, LVM_DELETEALLITEMS, 0, 0);

    Book *p = g_list.head->next;
    int row = 0;
    while (p != NULL)
    {
        if (filter != NULL && filter[0] != '\0' && strstr(p->title, filter) == NULL)
        {
            p = p->next;
            continue;
        }

        wchar_t buf[128];

        /* ISBN 也是 char[] 存 UTF-8，必须转成宽字符再放进列表；
           直接 (LPWSTR)p->isbn 会把 UTF-8 字节当 UTF-16 用，显示成乱码 */
        MultiByteToWideChar(CP_UTF8, 0, p->isbn, -1, buf, 128);

        LVITEMW item;
        ZeroMemory(&item, sizeof(item));
        item.mask     = LVIF_TEXT;
        item.iItem    = row;
        item.iSubItem = 0;
        item.pszText  = buf;
        SendMessageW(g_hList, LVM_INSERTITEMW, 0, (LPARAM)&item);

        MultiByteToWideChar(CP_UTF8, 0, p->title,  -1, buf, 128);
        SetSubItem(row, 1, buf);

        MultiByteToWideChar(CP_UTF8, 0, p->author, -1, buf, 128);
        SetSubItem(row, 2, buf);

        _snwprintf(buf, 128, L"%.2f", p->price);
        SetSubItem(row, 3, buf);

        _snwprintf(buf, 128, L"%d", p->stock);
        SetSubItem(row, 4, buf);

        p = p->next;
        row++;
    }

    /* 更新左上角标题，显示总数 */
    wchar_t t[128];
    _snwprintf(t, 128, L"图书列表（共 %d 本）", g_list.count);
    SetWindowTextW(g_lblListTitle, t);
}

static void RefreshList(void)
{
    RefreshListFiltered(NULL);
}

/* 取列表中选中行的 ISBN；没有选中行返回空串 */
static void GetSelectedIsbn(char *out, int cap)
{
    out[0] = '\0';
    int sel = (int)SendMessageW(g_hList, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
    if (sel < 0) return;

    LVITEMW it;
    ZeroMemory(&it, sizeof(it));
    it.mask      = LVIF_TEXT;
    it.iItem     = sel;
    it.iSubItem  = 0;
    it.pszText   = (LPWSTR)out;
    it.cchTextMax = cap;
    SendMessageW(g_hList, LVM_GETITEMTEXTW, (WPARAM)sel, (LPARAM)&it);

    /* LVITEMW 的 cchTextMax 字段位置与 LVITEMW 一致，这里用 TEXT 缓冲接收 */
    out[cap - 1] = '\0';
}


/* ---------- 表单 ---------- */
static void ReadFields(Book *b)
{
    memset(b, 0, sizeof(Book));
    GetEditText(EditByIndex(0), b->isbn,   MAX_ISBN);
    GetEditText(EditByIndex(1), b->title,  MAX_TITLE);
    GetEditText(EditByIndex(2), b->author, MAX_AUTHOR);
    b->price = GetEditFloat(EditByIndex(3), 0.0f);
    b->stock = GetEditInt(EditByIndex(4), 0);
}

static void ClearFields(void)
{
    int i;
    for (i = 0; i < 5; i++) SetEditText(EditByIndex(i), L"");
    SetFocus(EditByIndex(0));
}
/* 把某个结点内容填进表单，方便修改 */
static void FillFieldsFrom(const Book *p)
{
    wchar_t buf[128];
    MultiByteToWideChar(CP_UTF8, 0, p->isbn,   -1, buf, 128);
    SetEditText(EditByIndex(0), buf);
    MultiByteToWideChar(CP_UTF8, 0, p->title,  -1, buf, 128);
    SetEditText(EditByIndex(1), buf);
    MultiByteToWideChar(CP_UTF8, 0, p->author, -1, buf, 128);
    SetEditText(EditByIndex(2), buf);
    _snwprintf(buf, 128, L"%.2f", p->price);
    SetEditText(EditByIndex(3), buf);
    _snwprintf(buf, 128, L"%d", p->stock);
    SetEditText(EditByIndex(4), buf);
}

/* 操作目标 ISBN：优先取列表中选中的行，其次取 ISBN 输入框 */
static void GetTargetIsbn(char *out, int cap)
{
    GetSelectedIsbn(out, cap);
    if (out[0] == '\0')
        GetEditText(EditByIndex(0), out, cap);
}

/* ---------- 按钮功能 ---------- */

static void OnAdd(void)
{
    Book b;
    ReadFields(&b);

    if (b.isbn[0] == '\0')
    {
        SetStatus(L"请先填写 ISBN！");
        SetFocus(EditByIndex(0));
        return;
    }

    if (AddBook(&g_list, b) == 0)
    {
        SetStatus(L"该图书已存在，未重复添加。");
        return;
    }

    RefreshList();
    ClearFields();
    SetStatus(L"添加成功：《%ls》已入库。", b.title);
}
static void OnDelete(void)
{
    char isbn[MAX_ISBN];
    GetTargetIsbn(isbn, MAX_ISBN);

    if (isbn[0] == '\0')
    {
        SetStatus(L"请在列表中选中一行，或在 ISBN 框填写要删除的编号！");
        return;
    }

    if (DeleteBook(&g_list, isbn) == 1)
    {
        RefreshList();
        ClearFields();
        SetStatus(L"删除成功！");
    }
    else
    {
        SetStatus(L"未找到该图书，删除失败！");
    }
}

static void OnQuery(void)
{
    char isbn[MAX_ISBN];
    GetTargetIsbn(isbn, MAX_ISBN);

    if (isbn[0] == '\0')
    {
        SetStatus(L"请在列表中选中一行，或在 ISBN 框填写要查询的编号！");
        return;
    }

    Book *p = FindByIsbn(&g_list, isbn);
    if (p == NULL)
    {
        SetStatus(L"未找到该图书。");
        return;
    }

    wchar_t isbnW[MAX_ISBN], title[MAX_TITLE], author[MAX_AUTHOR];
    MultiByteToWideChar(CP_UTF8, 0, p->isbn,   -1, isbnW,  MAX_ISBN);
    MultiByteToWideChar(CP_UTF8, 0, p->title,  -1, title,  MAX_TITLE);
    MultiByteToWideChar(CP_UTF8, 0, p->author, -1, author, MAX_AUTHOR);

    SetStatus(L"查询结果 —— ISBN：%ls  书名：%ls  作者：%ls  价格：%.2f  库存：%d",
              isbnW, title, author, p->price, p->stock);
}

static void OnModify(void)
{
    char isbn[MAX_ISBN];
    GetTargetIsbn(isbn, MAX_ISBN);

    if (isbn[0] == '\0')
    {
        SetStatus(L"请在列表中选中一行，或在 ISBN 框填写要修改的编号！");
        return;
    }

    Book *p = FindByIsbn(&g_list, isbn);
    if (p == NULL)
    {
        SetStatus(L"未找到该图书。");
        return;
    }

    /* 若表单里 ISBN 为空，先把该条数据填进表单，方便直接改 */
    char formIsbn[MAX_ISBN];
    GetEditText(EditByIndex(0), formIsbn, MAX_ISBN);
    if (formIsbn[0] == '\0')
    {
        FillFieldsFrom(p);
        SetStatus(L"已把《%ls》填入表单，改好后请再点一次【修改选中图书】保存。", p->title);
        return;
    }

    /* 否则把表单中非空字段写回结点 */
    char buf[MAX_TITLE];
    char tmp[MAX_TITLE];

    GetEditText(EditByIndex(1), buf, MAX_TITLE);
    if (buf[0] != '\0')
    {
        memset(p->title, 0, MAX_TITLE);
        _snprintf(tmp, MAX_TITLE, "%s", buf);
        memcpy(p->title, tmp, MAX_TITLE);
    }

    GetEditText(EditByIndex(2), buf, MAX_AUTHOR);
    if (buf[0] != '\0')
    {
        memset(p->author, 0, MAX_AUTHOR);
        _snprintf(tmp, MAX_AUTHOR, "%s", buf);
        memcpy(p->author, tmp, MAX_AUTHOR);
    }

    GetEditText(EditByIndex(3), buf, 64);
    if (buf[0] != '\0') p->price = GetEditFloat(EditByIndex(3), p->price);

    GetEditText(EditByIndex(4), buf, 64);
    if (buf[0] != '\0') p->stock = GetEditInt(EditByIndex(4), p->stock);

    RefreshList();
    SetStatus(L"修改成功！");
}

static void OnSearchTitle(void)
{
    char key[MAX_TITLE];
    GetEditText(EditByIndex(1), key, MAX_TITLE);
    if (key[0] == '\0')
    {
        SetStatus(L"请先在【书名】输入框填写关键字！");
        SetFocus(EditByIndex(1));
        return;
    }

    /* 统计匹配数量 */
    int hits = 0;
    Book *p = g_list.head->next;
    while (p != NULL)
    {
        if (strstr(p->title, key) != NULL) hits++;
        p = p->next;
    }

    RefreshListFiltered(key);

    wchar_t keyW[MAX_TITLE];
    MultiByteToWideChar(CP_UTF8, 0, key, -1, keyW, MAX_TITLE);
    if (hits == 0)
        SetStatus(L"没有书名包含【%ls】的图书。", keyW);
    else
        SetStatus(L"模糊查找：找到 %d 本含【%ls】的书，点【显示全部】恢复完整列表。", hits, keyW);
}

static void OnShowAll(void)
{
    RefreshList();
    SetStatus(L"已显示全部图书，共 %d 本。", g_list.count);
}

static void OnSort(void)
{
    if (g_list.count <= 1)
    {
        SetStatus(L"图书不足 2 本，无需排序。");
        return;
    }
    SortByPrice(&g_list);
    RefreshList();
    SetStatus(L"已按价格升序排列，共 %d 本。", g_list.count);
}

static void OnStat(void)
{
    Book *p = g_list.head->next;
    float totalValue = 0.0f;
    int   totalStock = 0;

    while (p != NULL)
    {
        totalValue += p->price * (float)p->stock;
        totalStock += p->stock;
        p = p->next;
    }

    SetStatus(L"统计 —— 图书总数：%d 本  库存总量：%d 本  库存总价值：%.2f",
              g_list.count, totalStock, totalValue);
}

static void OnSave(void)
{
    wchar_t path[MAX_PATH];
    GetBooksPath(path, MAX_PATH);
    if (SaveToFile(path) == 1)
        SetStatus(L"已保存 %d 条记录到 books.txt", g_list.count);
    else
        SetStatus(L"保存失败：无法写入 books.txt");
}

static void OnLoad(void)
{
    wchar_t path[MAX_PATH];
    GetBooksPath(path, MAX_PATH);

    int n = LoadFromFile(path);
    RefreshList();

    if (n > 0)
        SetStatus(L"已从 books.txt 加载 %d 条图书记录，当前共 %d 本。", n, g_list.count);
    else
        SetStatus(L"books.txt 不存在，或没有新的可加载记录。");
}

/* ---------- 布局 ---------- */
static void LayoutChildren(int cw, int ch)
{
    if (g_hList == NULL) return;

    const int MARGIN   = 12;
    const int RIGHT_W  = 388;   /* 右侧栏总宽 */
    const int GAP      = 10;
    const int LBL_W    = 62;
    const int EDIT_W   = RIGHT_W - LBL_W - 6;
    const int BTN_Y0   = 258;   /* 按钮区起始 Y（相对客户区） */
    const int BTN_H    = 34;
    const int BTN_VGAP = 8;
    const int ROW_H    = 36;

    int rpX = cw - RIGHT_W - MARGIN;          /* 右侧栏左边界 */
    if (rpX < 200) rpX = 200;
    int listW = rpX - MARGIN - GAP;
    if (listW < 120) listW = 120;

    /* 底部状态栏 */
    const int STAT_H = 26;
    if (g_hStatus != NULL)
        MoveWindow(g_hStatus, MARGIN, ch - MARGIN - STAT_H, cw - MARGIN * 2, STAT_H, TRUE);

    /* 左侧列表（底部让出状态栏高度） */
    MoveWindow(g_hList, MARGIN, MARGIN + 24, listW,
               ch - MARGIN * 2 - STAT_H - 8 - 24, TRUE);

    /* 右侧栏标题 */
    MoveWindow(g_lblFormTitle, rpX, MARGIN + 4, RIGHT_W, 20, TRUE);

    /* 表单 5 行 */
    int i;
    for (i = 0; i < 5; i++)
    {
        int y = MARGIN + 30 + i * ROW_H;
        MoveWindow(g_labels[i].h, rpX, y + 4, LBL_W, 22, TRUE);
        MoveWindow(g_edits[i].h,  rpX + LBL_W + 6, y, EDIT_W, 26, TRUE);
    }

    /* 按钮 4 行 × 3 列 */
    int colW = (RIGHT_W - 2 * 8) / 3;
    int k;
    for (k = 0; k < 11; k++)
    {
        if (g_btns[k].h == NULL) continue;
        int col = k % 3;
        int row = k / 3;
        int x = rpX + col * (colW + 8);
        int y = BTN_Y0 + row * (BTN_H + BTN_VGAP);
        MoveWindow(g_btns[k].h, x, y, colW, BTN_H, TRUE);
    }
}

/* ---------- 界面创建 ---------- */
static HWND MakeLabel(const wchar_t *text, int x, int y, int w, int h, HFONT f)
{
    HWND c = CreateWindowExW(0, L"STATIC", text,
                             WS_CHILD | WS_VISIBLE | SS_LEFT,
                             x, y, w, h, g_hMain, NULL, g_hInst, NULL);
    SendMessageW(c, WM_SETFONT, (WPARAM)(f ? f : g_hFont), TRUE);
    return c;
}

static HWND MakeEdit(int id, int x, int y, int w, int h)
{
    HWND c = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                             x, y, w, h, g_hMain, (HMENU)(INT_PTR)id, g_hInst, NULL);
    SendMessageW(c, WM_SETFONT, (WPARAM)g_hFont, TRUE);
    return c;
}

static HWND MakeButton(const wchar_t *text, int id, int x, int y, int w, int h)
{
    HWND c = CreateWindowExW(0, L"BUTTON", text,
                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                             x, y, w, h, g_hMain, (HMENU)(INT_PTR)id, g_hInst, NULL);
    SendMessageW(c, WM_SETFONT, (WPARAM)g_hFont, TRUE);
    return c;
}

static void CreateListColumns(void)
{
    const wchar_t *names[5] = { L"ISBN", L"书名", L"作者", L"价格", L"库存" };
    int widths[5] = { 140, 200, 120, 80, 70 };

    int i;
    for (i = 0; i < 5; i++)
    {
        LVCOLUMNW col;
        ZeroMemory(&col, sizeof(col));
        col.mask     = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.iSubItem = i;
        col.cx       = widths[i];
        col.pszText  = (LPWSTR)names[i];
        SendMessageW(g_hList, LVM_INSERTCOLUMNW, (WPARAM)i, (LPARAM)&col);
    }
}

/* ---------- 窗口过程 ---------- */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        /* 关键：先赋值 g_hMain，后面所有 Make* 都以它作父窗口。
           若等 CreateWindowExW 返回后再赋值，WM_CREATE 期间它是 NULL，
           所有子控件都会创建失败。 */
        g_hMain = hwnd;

        g_hList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                  WS_CHILD | WS_VISIBLE | LVS_REPORT |
                                  LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                  12, 36, 400, 400,
                                  hwnd, (HMENU)(INT_PTR)ID_LIST, g_hInst, NULL);
        SendMessageW(g_hList, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        /* 去掉网格线更干净；DOUBLEBUFFER 减少重绘闪烁 */
        SendMessageW(g_hList, LVM_SETEXTENDEDLISTVIEWSTYLE,
                     LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER,
                     LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

        ListView_SetBkColor(g_hList, CLR_FIELD);
        ListView_SetTextBkColor(g_hList, CLR_FIELD);
        ListView_SetTextColor(g_hList, CLR_TEXT);
        ApplyDarkTheme();

        CreateListColumns();

        g_hStatus = CreateWindowExW(WS_EX_CLIENTEDGE, L"STATIC", L"就绪",
                                 WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
                                 12, 12, 400, 24, g_hMain, (HMENU)(INT_PTR)ID_STATUS, g_hInst, NULL);
        SendMessageW(g_hStatus, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        g_lblListTitle = MakeLabel(L"图书列表（共 0 本）", 12, 12, 300, 22, g_hFontTitle);
        g_lblFormTitle = MakeLabel(L"图书信息录入 / 修改", 0, 0, 300, 20, g_hFontTitle);

        const wchar_t *lbl[5] = { L"ISBN：", L"书名：", L"作者：", L"价格：", L"库存：" };
        int ids[5] = { ID_EDIT_ISBN, ID_EDIT_TITLE, ID_EDIT_AUTHOR,
                       ID_EDIT_PRICE, ID_EDIT_STOCK };
        int i;
        for (i = 0; i < 5; i++)
        {
            g_labels[i].h  = MakeLabel(lbl[i], 0, 0, 60, 22, g_hFont);
            g_labels[i].id = i;
            g_edits[i].h   = MakeEdit(ids[i], 0, 0, 200, 26);
            g_edits[i].id  = ids[i];
        }

        struct { const wchar_t *text; int id; } bs[11] = {
            { L"添加图书",       ID_BTN_ADD     },
            { L"删除选中图书",   ID_BTN_DEL     },
            { L"查询选中图书",   ID_BTN_QUERY   },
            { L"修改选中图书",   ID_BTN_MODIFY  },
            { L"按书名模糊查找", ID_BTN_SEARCH  },
            { L"显示全部",       ID_BTN_SHOWALL },
            { L"按价格排序",     ID_BTN_SORT    },
            { L"统计信息",       ID_BTN_STAT    },
            { L"保存到文件",     ID_BTN_SAVE    },
            { L"从文件导入",     ID_BTN_LOAD    },
            { L"清空输入框",     ID_BTN_CLEAR   },
        };
        int k;
        for (k = 0; k < 11; k++)
        {
            g_btns[k].h  = MakeButton(bs[k].text, bs[k].id, 0, 0, 100, 30);
            g_btns[k].id = bs[k].id;
        }

        /* 初次布局 */
        RECT cr;
        GetClientRect(hwnd, &cr);
        LayoutChildren(cr.right, cr.bottom);


        SetFocus(EditByIndex(0));
        return 0;
    }

    /* ===== 自绘按钮 ===== */
    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
        if (dis != NULL && dis->CtlType == ODT_BUTTON)
        {
            DrawFlatButton(dis);
            return TRUE;
        }
        break;
    }

    /* ===== 输入框：深色背景 + 浅色文字 ===== */
    case WM_CTLCOLOREDIT:
    {
        HDC dc;
        if (g_brField == NULL) break;
        dc = (HDC)wParam;
        SetTextColor(dc, CLR_TEXT);
        SetBkColor(dc, CLR_FIELD);
        return (LRESULT)g_brField;
    }

    /* ===== 静态文本（标签 / 区块标题 / 状态栏）===== */
    case WM_CTLCOLORSTATIC:
    {
        HDC dc;
        HWND h;
        if (g_brWindow == NULL) break;
        dc = (HDC)wParam;
        h  = (HWND)lParam;

        if (h == g_hStatus)
            SetTextColor(dc, CLR_ACCENT);
        else if (h == g_lblListTitle || h == g_lblFormTitle)
            SetTextColor(dc, CLR_ACCENT_DK);
        else
            SetTextColor(dc, CLR_TEXT_DIM);

        SetBkColor(dc, CLR_WINDOW);
        SetBkMode(dc, OPAQUE);
        return (LRESULT)g_brWindow;
    }

    /* ===== 按钮悬停高亮 ===== */
    case WM_MOUSEMOVE:
    {
        POINT pt;
        int hot = -1;
        int i;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        for (i = ID_BTN_FIRST; i <= ID_BTN_LAST; i++)
        {
            RECT rc;
            if (GetBtnRectForId(i, &rc) && PtInRect(&rc, pt)) { hot = i; break; }
        }

        if (hot != g_hotBtn)
        {
            HWND hOld = GetDlgItem(g_hMain, g_hotBtn);
            HWND hNew = (hot >= 0) ? GetDlgItem(g_hMain, hot) : NULL;
            g_hotBtn = hot;
            if (hOld != NULL) InvalidateRect(hOld, NULL, FALSE);
            if (hNew != NULL) InvalidateRect(hNew, NULL, FALSE);

            {
                TRACKMOUSEEVENT tme;
                tme.cbSize    = sizeof(tme);
                tme.dwFlags   = TME_LEAVE;
                tme.hwndTrack = hwnd;
                TrackMouseEvent(&tme);
            }
        }
        break;
    }

    case WM_MOUSELEAVE:
    {
        if (g_hotBtn >= 0)
        {
            HWND hOld = GetDlgItem(g_hMain, g_hotBtn);
            g_hotBtn = -1;
            if (hOld != NULL) InvalidateRect(hOld, NULL, FALSE);
        }
        return 0;
    }

    /* ===== 背景填深色，减少闪烁 ===== */
    case WM_ERASEBKGND:
    {
        RECT rc;
        if (g_brWindow == NULL) break;
        GetClientRect(hwnd, &rc);
        FillRect((HDC)wParam, &rc, g_brWindow);
        return 1;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_BTN_ADD:     OnAdd();         break;
        case ID_BTN_DEL:     OnDelete();      break;
        case ID_BTN_QUERY:   OnQuery();       break;
        case ID_BTN_MODIFY:  OnModify();      break;
        case ID_BTN_SEARCH:  OnSearchTitle(); break;
        case ID_BTN_SHOWALL: OnShowAll();     break;
        case ID_BTN_SORT:    OnSort();        break;
        case ID_BTN_STAT:    OnStat();        break;
        case ID_BTN_SAVE:    OnSave();        break;
        case ID_BTN_LOAD:    OnLoad();        break;
        case ID_BTN_CLEAR:   ClearFields();   break;
        default: break;
        }
        return 0;

    /* 列表双击：把该行内容填入表单，方便修改 */
    case WM_NOTIFY:
    {
        NMHDR *nh = (NMHDR *)lParam;
        if (nh == NULL) break;

        /* 列表行自绘在 MinGW 下不稳定（列矩形取值异常导致文字重叠），
           故不使用；列表主体由原生渲染，已通过 ListView_SetBkColor 设为深色。
           这里仅处理双击。 */

        if (nh->idFrom == ID_LIST && nh->code == NM_DBLCLK)
        {
            char isbn[MAX_ISBN];
            GetSelectedIsbn(isbn, MAX_ISBN);
            if (isbn[0] != '\0')
            {
                Book *p = FindByIsbn(&g_list, isbn);
                if (p != NULL) FillFieldsFrom(p);
            }
        }
        return 0;
    }

    case WM_SIZE:
        LayoutChildren(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO *mmi = (MINMAXINFO *)lParam;
        mmi->ptMinTrackSize.x = 880;
        mmi->ptMinTrackSize.y = 560;
        return 0;
    }

    case WM_CLOSE:
        if (MessageBoxW(hwnd, L"退出前是否把数据保存到 books.txt？",
                        L"退出确认", MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES)
        {
            wchar_t path[MAX_PATH];
            GetBooksPath(path, MAX_PATH);
            SaveToFile(path);
        }
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        DestroyList(&g_list);
        if (g_brWindow != NULL) { DeleteObject(g_brWindow); g_brWindow = NULL; }
        if (g_brField  != NULL) { DeleteObject(g_brField);  g_brField  = NULL; }
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

/* ---------- 入口 ---------- */
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;

    g_hInst = hInstance;

    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    /* 加载 uxtheme 以启用深色滚动条（失败不影响其他功能） */
    {
        HMODULE hUx = LoadLibraryW(L"uxtheme.dll");
        if (hUx != NULL)
            pSetWindowTheme = (PFN_SetWindowTheme)(void *)GetProcAddress(hUx, "SetWindowTheme");
    }

    /* 主题画刷（必须在建窗口前创建，WM_CTLCOLOR* 会用到） */
    g_brWindow = CreateSolidBrush(CLR_WINDOW);
    g_brField  = CreateSolidBrush(CLR_FIELD);

    g_hFont = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                          L"Microsoft YaHei UI");
    g_hFontTitle = CreateFontW(-18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                               L"Microsoft YaHei UI");

    WNDCLASSEXW wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = g_brWindow;   /* 深色背景，避免启动闪白 */
    wc.lpszClassName = L"BookManagerGUI";
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);

    if (RegisterClassExW(&wc) == 0)
    {
        MessageBoxW(NULL, L"注册窗口类失败！", L"错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    InitList(&g_list);

    g_hMain = CreateWindowExW(0, L"BookManagerGUI",
                              L"图书管理系统 —— 单链表实现（图形界面版）",
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 1020, 680,
                              NULL, NULL, hInstance, NULL);
    if (g_hMain == NULL)
    {
        MessageBoxW(NULL, L"创建窗口失败！", L"错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    ApplyDarkTitleBar(g_hMain);

    {
        wchar_t path[MAX_PATH];
        GetBooksPath(path, MAX_PATH);
        if (LoadFromFile(path) > 0)
            RefreshList();
    }

    ShowWindow(g_hMain, nCmdShow);
    UpdateWindow(g_hMain);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_hFont != NULL)       DeleteObject(g_hFont);
    if (g_hFontTitle != NULL)  DeleteObject(g_hFontTitle);
    return (int)msg.wParam;
}
