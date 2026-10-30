// 学生成绩管理系统
// 面向对象程序设计课程实践
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>

using namespace std;

// 角色：0学生 1老师 2管理员
enum RoleType { ROLE_STU, ROLE_TEA, ROLE_ADMIN };

// 一条成绩
struct Grade {
    string sid;     // 学号
    string course;  // 科目
    double score;   // 分数
    string term;    // 学期
};

// 人员基类
class Person {
public:
    string account;   // 登录账号
    string pwd;       // 密码
    string realName;  // 姓名
    RoleType role;
    bool valid;       // 是否启用
    string sex;
    string grade;     // 年级
    string cls;       // 班级
    string tel;       // 电话

    Person(string acc, string pw, string nm, RoleType r)
        : account(acc), pwd(pw), realName(nm), role(r), valid(true) {}
    virtual ~Person() {}

    virtual void showMenu() = 0;
    RoleType getRole() { return role; }
};

class Student : public Person {
public:
    Student(string acc, string pw, string nm) : Person(acc, pw, nm, ROLE_STU) {}
    void showMenu() {
        cout << "\n------ 学生端 ------\n";
        cout << "1. 看自己信息\n";
        cout << "2. 查成绩和排名\n";
        cout << "3. 导出成绩单\n";
        cout << "4. 改密码\n";
        cout << "0. 退出\n";
    }
};

class Teacher : public Person {
public:
    Teacher(string acc, string pw, string nm) : Person(acc, pw, nm, ROLE_TEA) {}
    void showMenu() {
        cout << "\n------ 老师端 ------\n";
        cout << "1. 查学生名单\n";
        cout << "2. 登成绩(单条/批量)\n";
        cout << "3. 班级成绩分析\n";
        cout << "4. 导出全班成绩单\n";
        cout << "0. 退出\n";
    }
};

class Admin : public Person {
public:
    Admin(string acc, string pw, string nm) : Person(acc, pw, nm, ROLE_ADMIN) {}
    void showMenu() {
        cout << "\n------ 管理员 ------\n";
        cout << "1. 账号管理\n";
        cout << "2. 成绩数据维护\n";
        cout << "3. 看操作日志\n";
        cout << "0. 退出\n";
    }
};

// ---------- 文件读写 ----------
// 人员文件每行：账号,密码,姓名,角色,是否启用,性别,年级,班级,电话
// 角色用 s/t/a 三个字母存，省得写中文
static vector<Person*> g_users;
static vector<Grade> g_grades;

static string nowStr() {
    time_t t = time(0);
    tm lt;
    localtime_s(&lt, &t);
    char buf[24];
    strftime(buf, 24, "%m-%d %H:%M:%S", &lt);
    return buf;
}

static void writeLog(string who, string what) {
    ofstream f("log.txt", ios::app);
    if (!f) return;
    f << nowStr() << "  " << who << "  " << what << "\n";
}

static RoleType parseRole(string s) {
    if (s == "t") return ROLE_TEA;
    if (s == "a") return ROLE_ADMIN;
    return ROLE_STU;
}

static string roleCh(RoleType r) {
    if (r == ROLE_TEA) return "t";
    if (r == ROLE_ADMIN) return "a";
    return "s";
}

static void loadUsers() {
    ifstream f("stu_users.txt");
    if (!f) { cout << "没找到 stu_users.txt\n"; return; }
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string acc, pw, nm, rch, en, sx, gd, cl, ph;
        getline(ss, acc, ',');
        getline(ss, pw, ',');
        getline(ss, nm, ',');
        getline(ss, rch, ',');
        getline(ss, en, ',');
        getline(ss, sx, ',');
        getline(ss, gd, ',');
        getline(ss, cl, ',');
        getline(ss, ph, ',');
        RoleType r = parseRole(rch);
        Person* p;
        if (r == ROLE_STU) p = new Student(acc, pw, nm);
        else if (r == ROLE_TEA) p = new Teacher(acc, pw, nm);
        else p = new Admin(acc, pw, nm);
        p->valid = (en == "1");
        p->sex = sx; p->grade = gd; p->cls = cl; p->tel = ph;
        g_users.push_back(p);
    }
}

static void saveUsers() {
    ofstream f("stu_users.txt");
    for (Person* p : g_users) {
        f << p->account << "," << p->pwd << "," << p->realName << ","
          << roleCh(p->role) << "," << (p->valid ? "1" : "0") << ","
          << p->sex << "," << p->grade << "," << p->cls << "," << p->tel << "\n";
    }
}

static void loadGrades() {
    ifstream f("grade_data.txt");
    if (!f) { cout << "没找到 grade_data.txt\n"; return; }
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Grade g; string sv;
        getline(ss, g.sid, ',');
        getline(ss, g.course, ',');
        getline(ss, sv, ',');
        getline(ss, g.term, ',');
        try { g.score = stod(sv); } catch (...) { continue; }
        g_grades.push_back(g);
    }
}

static void saveGrades() {
    ofstream f("grade_data.txt");
    for (Grade& g : g_grades)
        f << g.sid << "," << g.course << "," << g.score << "," << g.term << "\n";
}

// 登录，返回 Person 指针，失败返回 NULL
static Person* doLogin(string acc, string pw) {
    for (Person* p : g_users) {
        if (p->account == acc && p->pwd == pw) {
            if (!p->valid) { cout << "账号被禁用了\n"; return NULL; }
            return p;
        }
    }
    return NULL;
}

// 绩点换算
static double toGpa(double s) {
    if (s >= 90) return 4.0;
    if (s >= 85) return 3.7;
    if (s >= 82) return 3.3;
    if (s >= 78) return 3.0;
    if (s >= 75) return 2.7;
    if (s >= 72) return 2.3;
    if (s >= 68) return 2.0;
    if (s >= 64) return 1.5;
    if (s >= 60) return 1.0;
    return 0;
}

// ================= 学生端 =================
static void stuInfo(Person* me) {
    cout << "\n学号:" << me->account << " 姓名:" << me->realName
         << " 性别:" << me->sex << " 年级:" << me->grade
         << " 班级:" << me->cls << " 电话:" << me->tel << "\n";
}

static void stuQuery(Person* me) {
    cout << "\n--- 成绩单 ---\n";
    double sum = 0; int n = 0; double gpaSum = 0;
    for (Grade& g : g_grades) {
        if (g.sid == me->account) {
            cout << g.course << "  " << g.score << "  (" << g.term << ")\n";
            sum += g.score; n++;
            gpaSum += toGpa(g.score);
        }
    }
    if (n == 0) { cout << "没成绩\n"; return; }
    cout << "------------\n";
    cout << "总分" << sum << "  平均" << sum / n << "  绩点" << gpaSum / n << "\n";

    // 总分排名
    map<string, double> tot;
    for (Grade& g : g_grades) tot[g.sid] += g.score;
    vector<pair<string, double>> v(tot.begin(), tot.end());
    sort(v.begin(), v.end(), [](pair<string, double>& a, pair<string, double>& b) {
        return a.second > b.second;
    });
    cout << "\n[总分榜]\n";
    for (int i = 0; i < (int)v.size(); i++)
        if (v[i].first == me->account)
            cout << "你排第" << (i + 1) << " / " << v.size() << "人\n";

    // 每科排名
    cout << "\n[单科排名]\n";
    map<string, vector<pair<string, double>>> byCourse;
    for (Grade& g : g_grades)
        byCourse[g.course].push_back({ g.sid, g.score });
    for (auto& kv : byCourse) {
        auto& lst = kv.second;
        sort(lst.begin(), lst.end(), [](auto& a, auto& b) { return a.second > b.second; });
        for (int i = 0; i < (int)lst.size(); i++)
            if (lst[i].first == me->account)
                cout << kv.first << ": 第" << (i + 1) << "名\n";
    }
}

static void stuExport(Person* me) {
    string fn = "my_" + me->account + ".txt";
    ofstream out(fn);
    if (!out) { cout << "导出失败\n"; return; }
    out << me->realName << "(" << me->account << ") 的成绩\n";
    out << "--------------------\n";
    for (Grade& g : g_grades)
        if (g.sid == me->account)
            out << g.course << "\t" << g.score << "\t" << g.term << "\n";
    cout << "已经写到 " << fn << "\n";
    writeLog(me->account, "导出自己的成绩单");
}

static void stuChangePwd(Person* me) {
    string oldp, p1, p2;
    cout << "原密码:"; cin >> oldp;
    if (oldp != me->pwd) { cout << "原密码不对\n"; return; }
    cout << "新密码:"; cin >> p1;
    cout << "再输一遍:"; cin >> p2;
    if (p1 != p2) { cout << "两次不一样\n"; return; }
    me->pwd = p1;
    saveUsers();
    cout << "改好了\n";
    writeLog(me->account, "改密码");
}

// ================= 老师端 =================
static void teaListStudents() {
    cout << "\n学号      姓名  性别 班级      电话\n";
    for (Person* p : g_users) {
        if (p->role != ROLE_STU) continue;
        cout << p->account << "  " << p->realName << "  " << p->sex
             << "  " << p->cls << "  " << p->tel << "\n";
    }
}

static void teaSearchStudent() {
    string kw;
    cout << "搜学号或姓名:"; cin >> kw;
    bool any = false;
    for (Person* p : g_users) {
        if (p->role != ROLE_STU) continue;
        if (p->account.find(kw) != string::npos || p->realName.find(kw) != string::npos) {
            cout << p->account << " " << p->realName << " " << p->cls << "\n";
            any = true;
        }
    }
    if (!any) cout << "没搜到\n";
}

static void teaEditPhone() {
    string sid, ph;
    cout << "学号:"; cin >> sid;
    for (Person* p : g_users) {
        if (p->account == sid && p->role == ROLE_STU) {
            cout << "原电话:" << p->tel << " 新电话:"; cin >> ph;
            p->tel = ph;
            saveUsers();
            cout << "改好了\n";
            return;
        }
    }
    cout << "没这个学生\n";
}

static bool isStudent(string sid) {
    for (Person* p : g_users)
        if (p->account == sid && p->role == ROLE_STU) return true;
    return false;
}

static void teaAddOne(Person* me) {
    string sid, course, term; double sc;
    cout << "学号:"; cin >> sid;
    if (!isStudent(sid)) { cout << "没这学生\n"; return; }
    cout << "科目:"; cin >> course;
    cout << "分数(0-100):"; cin >> sc;
    if (sc < 0 || sc > 100) { cout << "分数超范围\n"; return; }
    bool upd = false;
    for (Grade& g : g_grades) {
        if (g.sid == sid && g.course == course) {
            g.score = sc; upd = true;
            cout << "原来有,改成" << sc << "\n";
            break;
        }
    }
    if (!upd) {
        cout << "学期:"; cin >> term;
        g_grades.push_back({ sid, course, sc, term });
        cout << "新录了一条\n";
    }
    saveGrades();
    writeLog(me->account, "录成绩 " + sid + " " + course);
}

static void teaBatch(Person* me) {
    ifstream f("batch_in.txt");
    if (!f) { cout << "没有 batch_in.txt\n"; return; }
    int cnt = 0;
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Grade g; string sv;
        getline(ss, g.sid, ',');
        getline(ss, g.course, ',');
        getline(ss, sv, ',');
        getline(ss, g.term, ',');
        try { g.score = stod(sv); } catch (...) { continue; }
        if (g.score < 0 || g.score > 100) {
            cout << "跳过非法:" << line << "\n"; continue;
        }
        g_grades.push_back(g);
        cnt++;
    }
    saveGrades();
    cout << "批量进了 " << cnt << " 条\n";
    writeLog(me->account, "批量导入" + to_string(cnt) + "条");
}

static void teaStats() {
    cout << "\n--- 各科分析 ---\n";
    map<string, vector<double>> m;
    for (Grade& g : g_grades) m[g.course].push_back(g.score);
    for (auto& kv : m) {
        auto& v = kv.second;
        double mx = v[0], mn = v[0], sum = 0; int pass = 0;
        for (double x : v) {
            if (x > mx) mx = x;
            if (x < mn) mn = x;
            sum += x;
            if (x >= 60) pass++;
        }
        cout << "\n[" << kv.first << "] 人数" << v.size()
             << " 平均" << fixed << setprecision(1) << sum / v.size()
             << " 最高" << mx << " 最低" << mn
             << " 及格率" << 100.0 * pass / v.size() << "%\n";
        int b[5] = {0,0,0,0,0};
        for (double x : v) {
            if (x < 60) b[0]++;
            else if (x < 70) b[1]++;
            else if (x < 80) b[2]++;
            else if (x < 90) b[3]++;
            else b[4]++;
        }
        const char* lab[5] = {"<60 ","60-69","70-79","80-89","90+ "};
        for (int i = 0; i < 5; i++) {
            cout << "  " << lab[i] << "|";
            for (int j = 0; j < b[i]; j++) cout << "*";
            cout << " " << b[i] << "人\n";
        }
    }
}

static void teaExport(Person* me) {
    ofstream out("whole_class.txt");
    out << "学号\t姓名\t科目\t分数\t学期\n";
    for (Grade& g : g_grades) {
        string nm = "?";
        for (Person* p : g_users)
            if (p->account == g.sid) { nm = p->realName; break; }
        out << g.sid << "\t" << nm << "\t" << g.course << "\t" << g.score << "\t" << g.term << "\n";
    }
    cout << "全班成绩单写到 whole_class.txt\n";
    writeLog(me->account, "导出全班成绩单");
}

// ================= 管理员 =================
static void admListUsers() {
    cout << "\n账号      姓名    角色  状态\n";
    for (Person* p : g_users) {
        cout << p->account << "  " << p->realName << "  "
             << (p->role == ROLE_STU ? "学生" : p->role == ROLE_TEA ? "老师" : "管理员")
             << "  " << (p->valid ? "正常" : "停用") << "\n";
    }
}

static void admAddUser() {
    string acc, pw, nm, r;
    cout << "账号:"; cin >> acc;
    cout << "密码:"; cin >> pw;
    cout << "姓名:"; cin >> nm;
    cout << "角色(s/t):"; cin >> r;
    for (Person* p : g_users)
        if (p->account == acc) { cout << "账号重复\n"; return; }
    if (r == "s") g_users.push_back(new Student(acc, pw, nm));
    else if (r == "t") g_users.push_back(new Teacher(acc, pw, nm));
    else { cout << "角色只能 s 或 t\n"; return; }
    saveUsers();
    cout << "加好了\n";
    writeLog("admin", "加账号 " + acc);
}

static void admDelUser() {
    string acc;
    cout << "要删的账号:"; cin >> acc;
    if (acc == "admin") { cout << "不能删管理员\n"; return; }
    for (int i = 0; i < (int)g_users.size(); i++) {
        if (g_users[i]->account == acc) {
            delete g_users[i];
            g_users.erase(g_users.begin() + i);
            saveUsers();
            cout << "删了\n";
            writeLog("admin", "删账号 " + acc);
            return;
        }
    }
    cout << "没这账号\n";
}

static void admToggle() {
    string acc;
    cout << "账号:"; cin >> acc;
    for (Person* p : g_users) {
        if (p->account == acc) {
            p->valid = !p->valid;
            saveUsers();
            cout << "现在" << (p->valid ? "正常" : "停用") << "\n";
            return;
        }
    }
    cout << "没这账号\n";
}

static void admReset() {
    string acc;
    cout << "账号:"; cin >> acc;
    for (Person* p : g_users) {
        if (p->account == acc) {
            p->pwd = "123456";
            saveUsers();
            cout << "密码已重置成123456\n";
            writeLog("admin", "重置密码 " + acc);
            return;
        }
    }
    cout << "没这账号\n";
}

static void admAllGrades() {
    for (Grade& g : g_grades)
        cout << g.sid << " " << g.course << " " << g.score << " " << g.term << "\n";
}

static void admDelGrade() {
    string sid, c;
    cout << "学号:"; cin >> sid;
    cout << "科目:"; cin >> c;
    for (int i = 0; i < (int)g_grades.size(); i++) {
        if (g_grades[i].sid == sid && g_grades[i].course == c) {
            g_grades.erase(g_grades.begin() + i);
            saveGrades();
            cout << "删了\n";
            return;
        }
    }
    cout << "没这条\n";
}

static void admShowLog() {
    ifstream f("log.txt");
    if (!f) { cout << "还没日志\n"; return; }
    cout << "\n--- 最近日志 ---\n";
    string line;
    vector<string> all;
    while (getline(f, line)) all.push_back(line);
    int from = max(0, (int)all.size() - 15);
    for (int i = from; i < (int)all.size(); i++) cout << all[i] << "\n";
}

// ================= main =================
int main() {
    loadUsers();
    loadGrades();
    cout << "=== 学生成绩管理系统 ===" << endl;
    cout << "载入 " << g_users.size() << " 个账号, " << g_grades.size() << " 条成绩\n";

    Person* me = NULL;
    while (!me) {
        string acc, pw;
        cout << "\n账号:"; cin >> acc;
        cout << "密码:"; cin >> pw;
        me = doLogin(acc, pw);
        if (!me) cout << "账号密码不对\n";
        else {
            cout << "登录成功 " << me->realName << "\n";
            writeLog(me->account, "登录");
        }
    }

    int op = -1;
    while (op != 0) {
        me->showMenu();
        cout << ">"; cin >> op;
        if (op == 0) break;

        if (me->role == ROLE_STU) {
            if (op == 1) stuInfo(me);
            else if (op == 2) stuQuery(me);
            else if (op == 3) stuExport(me);
            else if (op == 4) stuChangePwd(me);
            else cout << "没这个选项\n";
        }
        else if (me->role == ROLE_TEA) {
            if (op == 1) {
                cout << "1名单 2搜人 3改电话>";
                int s; cin >> s;
                if (s == 1) teaListStudents();
                else if (s == 2) teaSearchStudent();
                else teaEditPhone();
            }
            else if (op == 2) {
                cout << "1单条 2批量>";
                int s; cin >> s;
                if (s == 1) teaAddOne(me);
                else teaBatch(me);
            }
            else if (op == 3) teaStats();
            else if (op == 4) teaExport(me);
            else cout << "没这个选项\n";
        }
        else {
            if (op == 1) {
                cout << "1看名单 2加人 3删人 4停启用 5重置密码>";
                int s; cin >> s;
                if (s == 1) admListUsers();
                else if (s == 2) admAddUser();
                else if (s == 3) admDelUser();
                else if (s == 4) admToggle();
                else admReset();
            }
            else if (op == 2) {
                cout << "1看全部 2删一条>";
                int s; cin >> s;
                if (s == 1) admAllGrades();
                else admDelGrade();
            }
            else if (op == 3) admShowLog();
            else cout << "没这个选项\n";
        }
    }

    writeLog(me->account, "退出");
    for (Person* p : g_users) delete p;
    cout << "再见\n";
    return 0;
}
