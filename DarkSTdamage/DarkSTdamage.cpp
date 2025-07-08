#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cctype>
#include <cmath>
#include <algorithm> 

using namespace std;

struct EnemyStatus
{
    string name;
    double defense;
    double normal;
    double strike;
    double slash;
    double stab;
};

struct WeaponRow { unordered_map<string, double> motions; };

class DamageTable
{
    unordered_map<string, EnemyStatus> table;

public:
    explicit DamageTable(const string& csvPath)
    {
        ifstream file(csvPath);
        if (!file) throw runtime_error("damagelist.csv を開けません: " + csvPath);

        string line;
        getline(file, line);

        while (getline(file, line))
        {
            if (line.empty()) continue;

            stringstream ss(line);
            string field;
            EnemyStatus s;

            getline(ss, field, ',');  s.defense = stod(field) / 100;
            getline(ss, s.name, ',');
            getline(ss, field, ',');  s.normal = stod(field) / 100;
            getline(ss, field, ',');  s.strike = stod(field) / 100;
            getline(ss, field, ',');  s.slash = stod(field) / 100;
            getline(ss, field, ',');  s.stab = stod(field) / 100;

            table.emplace(s.name, move(s));
        }
    }

    const EnemyStatus& get(const string& enemyName) const
    {
        auto it = table.find(enemyName);
        if (it == table.end()) throw invalid_argument("未登録の敵名です: " + enemyName);
        return it->second;
    }
};

class MotionTable {
    unordered_map<string, WeaponRow> table;
public:
    explicit MotionTable(const string& csvPath) {
        ifstream file(csvPath);
        if (!file) throw runtime_error("motionlist.csv を開けない: " + csvPath);

        string headerLine;
        getline(file, headerLine);
        vector<string> headers;
        for (stringstream ss(headerLine); ss.good();) {
            string cell; getline(ss, cell, ',');
            headers.push_back(cell);
        }

        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string cell;

            getline(ss, cell, ',');
            string weaponName = cell;

            WeaponRow row;
            for (size_t col = 1; col < headers.size() && getline(ss, cell, ','); ++col) {
                if (cell.empty()) continue;
                row.motions[headers[col]] = stod(cell) / 100;
            }
            table.emplace(move(weaponName), move(row));
        }
    }

    double get(const string& weapon, const string& motion) const {
        auto wIt = table.find(weapon);
        if (wIt == table.end())
            throw out_of_range("武器 '" + weapon + "' がない");
        const auto& motions = wIt->second.motions;
        auto mIt = motions.find(motion);
        if (mIt == motions.end())
            throw out_of_range("モーション '" + motion + "' がない");
        return mIt->second;
    }
};

double getCutRate(const EnemyStatus& st, string attr)
{

    for (char& ch : attr) ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));

    if (attr == "normal")  return st.normal;
    if (attr == "strike")  return st.strike;
    if (attr == "slash")   return st.slash;
    if (attr == "stab")    return st.stab;

    throw invalid_argument("攻撃属性がない: " + attr);
}

double computePenetrationPercent(double A, double D)
{

    double x = A / D;
    if (x == 0.0)        return 0.0;
    else if (x < 0.125)  return 10.0;
    else if (x > 8.0)    return 90.0;
    else if (x < 1.0)    return 1920.0 / 49.0 * pow(x - 0.125, 2) + 10.0;
    else if (x < 2.5)    return -40.0 / 3.0 * pow(x - 2.5, 2) + 70.0;
    else                 return -80.0 / 121.0 * pow(x - 8.0, 2) + 90.0;
}

int main()
{
    try {
        MotionTable motionTbl(R"(C:\Users\tanim\OneDrive\ドキュメント\AtCoder\DamageCalcApp\motionlistEn.csv)");
        DamageTable enemyTbl(R"(C:\Users\tanim\OneDrive\ドキュメント\AtCoder\DamageCalcApp\damagelistEn.csv)");

        string enemy, weapon, motion, attr;
        double atk;
        cout << "Enemy Name          ? "; getline(cin, enemy);
        cout << "Weapon Name         ? "; getline(cin, weapon);
        cout << "Motion Type         ? "; getline(cin, motion);
        cout << "Attack Attribute    ? (Normal / Strike / Slash / Stab): "; getline(cin, attr);
        cout << "Weapon Attack Power ? "; cin >> atk;

        const EnemyStatus& st = enemyTbl.get(enemy);
        double motionVal = motionTbl.get(weapon, motion);
        double cutRatePercent = getCutRate(st, attr);
        double A = atk * motionVal;
        double penPercent = computePenetrationPercent(A, st.defense);
        double reduction = max(0.0,min(1.0,1.0 - cutRatePercent / 100.0));

        int damage = atk * motionVal * (penPercent / 100.0) * reduction; 

        cout << "\n=== Calculation Result ===\n";
        cout << "Enemy            : " << st.name << '\n';
        cout << "Defence          : " << st.defense << '\n';
        cout << "Motion Value     : " << motionVal << '\n';
        cout << "Cut-Rate (%)     : " << cutRatePercent << '\n';
        cout << "Penetration (%)  : " << penPercent << '\n';
        cout << "---------------------------------\n";
        cout << "=> Damage        : " << damage << '\n';
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
